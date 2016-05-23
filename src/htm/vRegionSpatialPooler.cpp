
#include "stdint.h"
#include "immintrin.h"
#include "malloc.h"
#include "CJAssert.h"

#include "vInputSpace.h"
#include "vRegion.h"


void vRegion::PerformSpatialPooling(bool allowLearning)
{
   return;


   // SP1 - Compute Input Overlap
   int insertTotalMask = 0x80;
   __m256i ymm12_output_total32 = _mm256_set1_epi32(0);

   __m256i  ymm15_column_states;
   __m256i* pCurrentInputPtr = (__m256i*)dpInputSpace->GetBuffer();
   
   for (int columnIndex = 0; columnIndex < dNumColumns; columnIndex++)
   { 
      __m256i* pCurrentProximalPermPtr = (__m256i*)GetProximalSegmentPermanenceArray(columnIndex);
      int receptiveRange = dpInputSpace->GetSizeX() * dpInputSpace->GetSizeY();       // Each column will be able to see the full input space
      int subtotal = 255;

      __m256i ymm0_threshold = _mm256_set1_epi8((char)VHTM_CONNECTED_PERMANENCE);     // Load threshold value
      __m256i ymm6_overlap_total8 = _mm256_set1_epi8(0);
      __m256i ymm10_overlap_total32 = _mm256_set1_epi32(0);
      

      for (int synIndex = 0; synIndex < receptiveRange; synIndex++)
      {
         __m256i ymm1_perm = _mm256_load_si256(pCurrentProximalPermPtr);         // Load the proximal synaspe permanence values

         __m256i ymm2_connected = _mm256_cmpgt_epi8(ymm1_perm, ymm0_threshold);  // Compare to get a vector of connected synapses

         __m256i ymm3_input = _mm256_load_si256(pCurrentInputPtr);               // Load the input data

         __m256i ymm4_overlap = _mm256_and_si256(ymm2_connected, ymm3_input);    // bitwise-AND of input and connected synapses to get overlap

         ymm4_overlap = _mm256_and_si256(ymm4_overlap, _mm256_set1_epi8(1));     // Only need to mask off active bit if buffer has values other than 0 and 1 in it
         
         ymm6_overlap_total8 = _mm256_adds_epu8(ymm6_overlap_total8, ymm4_overlap); // Sum up all the overlapping columns in to 8-bit totals

         // Move the 8-bit totals to 32-bit totals every 255 times through loop, or if this is the last loop
         if (subtotal == 0 || (synIndex == (receptiveRange - 1)))
         {              
            __m256i ymm8_overlap_interm16 = _mm256_maddubs_epi16(ymm4_overlap, _mm256_set1_epi8(1));           // (uint8)overlap * 1 to a uint16 value, then SUM each uint16      
            __m256i ymm10_overlap_interm32 = _mm256_madd_epi16(ymm8_overlap_interm16, _mm256_set1_epi8(1));    // Optionally multiply in the boost and hadd u16s to u32s (not using boost here because overlap stats want unboosted values)
            ymm10_overlap_total32 = _mm256_add_epi32(ymm10_overlap_total32, ymm10_overlap_interm32);           // Add the u32 intermediate totals to overall totals
            ymm6_overlap_total8 = _mm256_set1_epi8(0);                                                         // Clear the intermediate 8bit totals
            subtotal = 256;
         }
         subtotal--;
         pCurrentProximalPermPtr += sizeof(__m256i);
         pCurrentInputPtr += sizeof(__m256i);
      }

      // Sum up the 32-bit counts in the total32 vector
      __m256i ymm11_zero = _mm256_set1_epi32(0);
      ymm10_overlap_total32 = _mm256_hadd_epi32(ymm10_overlap_total32, ymm11_zero);
	   ymm10_overlap_total32 = _mm256_hadd_epi32(ymm10_overlap_total32, ymm11_zero);
	   ymm10_overlap_total32 = _mm256_hadd_epi32(ymm10_overlap_total32, ymm11_zero);
	   ymm10_overlap_total32 = _mm256_hadd_epi32(ymm10_overlap_total32, ymm11_zero);
      
      // Broadcast the final total to all values in the vector, now it can be used for blend
      __m128i ymm10_overlap_total32_casted = _mm256_castsi256_si128(ymm10_overlap_total32);
      ymm10_overlap_total32 = _mm256_broadcastd_epi32(ymm10_overlap_total32_casted);
      switch (columnIndex % 8)
      {
      case 0:  ymm12_output_total32 = _mm256_blend_epi32(_mm256_set1_epi32(0), ymm10_overlap_total32, 0x80); break;
      case 1:  ymm12_output_total32 = _mm256_blend_epi32(_mm256_set1_epi32(0), ymm10_overlap_total32, 0x40); break;
      case 2:  ymm12_output_total32 = _mm256_blend_epi32(_mm256_set1_epi32(0), ymm10_overlap_total32, 0x20); break;
      case 3:  ymm12_output_total32 = _mm256_blend_epi32(_mm256_set1_epi32(0), ymm10_overlap_total32, 0x10); break;
      case 4:  ymm12_output_total32 = _mm256_blend_epi32(_mm256_set1_epi32(0), ymm10_overlap_total32, 0x08); break;
      case 5:  ymm12_output_total32 = _mm256_blend_epi32(_mm256_set1_epi32(0), ymm10_overlap_total32, 0x04); break;
      case 6:  ymm12_output_total32 = _mm256_blend_epi32(_mm256_set1_epi32(0), ymm10_overlap_total32, 0x02); break;
      case 7:
      {
         ymm12_output_total32 = _mm256_blend_epi32(_mm256_set1_epi32(0), ymm10_overlap_total32, 0x01);

         // Loop assumes that dNumCols always a multiple of 32 (since 32 8bit values fit in a 256 and none spill over to a non-filled vector)

         // Filter out the values below the overlap minimum
         __m256i ymm13_overlap_min = _mm256_set1_epi32(dSpatialPoolerConfig.noOverlapThresholdMin);
         __m256i ymm14_overlap_min_mask = _mm256_cmpgt_epi32(ymm12_output_total32, ymm13_overlap_min);
         ymm12_output_total32 = _mm256_and_si256(ymm12_output_total32, ymm14_overlap_min_mask);

         // Update the column overlap duty cycle totals
         __m256i ymm17_overlap_dc_totals = _mm256_load_si256((__m256i*)&dpColumnOverlapDutyCycleTotals[columnIndex]);
         ymm17_overlap_dc_totals = _mm256_add_epi32(ymm17_overlap_dc_totals, _mm256_mullo_epi32(ymm14_overlap_min_mask, _mm256_set1_epi32(-1)));

         // Subtract the oldest duty cycle entries from the column state history
         uint8_t* pOldestColumnStateArray = GetOldestColumnStateArrayPtr(columnIndex);
         __m256i ymm18_overlap_dc_oldest = _mm256_set_epi32(pOldestColumnStateArray[0] && eColumnState_Overlapped, pOldestColumnStateArray[1] && eColumnState_Overlapped,
            pOldestColumnStateArray[2] && eColumnState_Overlapped, pOldestColumnStateArray[3] && eColumnState_Overlapped,
            pOldestColumnStateArray[4] && eColumnState_Overlapped, pOldestColumnStateArray[5] && eColumnState_Overlapped,
            pOldestColumnStateArray[6] && eColumnState_Overlapped, pOldestColumnStateArray[7] && eColumnState_Overlapped);
         ymm17_overlap_dc_totals = _mm256_sub_epi32(ymm17_overlap_dc_totals, ymm18_overlap_dc_oldest);

         // Store off the new overlap duty cycle values
         _mm256_stream_si256((__m256i*)&dpColumnOverlapDutyCycleTotals[columnIndex], ymm17_overlap_dc_totals);

         // Press the 32bit mask values to 8 bit values so they can be stored in the column state flags
         switch (columnIndex % 32)
         {
         case 7:  ymm15_column_states = _mm256_shuffle_epi8(ymm14_overlap_min_mask, _mm256_set_epi32(0x00000000, 0x01010101, 0x02020202, 0x03030303, 0x00000000, 0x01010101, 0x02020202, 0x03030303)); break;
         case 15: ymm15_column_states = _mm256_shuffle_epi8(ymm14_overlap_min_mask, _mm256_set_epi32(0x04040404, 0x05050505, 0x06060606, 0x07070707, 0x04040404, 0x05050505, 0x06060606, 0x07070707)); break;
         case 23: ymm15_column_states = _mm256_shuffle_epi8(ymm14_overlap_min_mask, _mm256_set_epi32(0x08080808, 0x09090909, 0x0A0A0A0A, 0x0B0B0B0B, 0x08080808, 0x09090909, 0x0A0A0A0A, 0x0B0B0B0B)); break;
         case 31: ymm15_column_states = _mm256_shuffle_epi8(ymm14_overlap_min_mask, _mm256_set_epi32(0x0C0C0C0C, 0x0D0D0D0D, 0x0E0E0E0E, 0x0F0F0F0F, 0x0C0C0C0C, 0x0D0D0D0D, 0x0E0E0E0E, 0x0F0F0F0F));

            // Store the column state flags for columns that had activation higher than the current minimum
            ymm15_column_states = _mm256_permutevar8x32_epi32(ymm15_column_states, _mm256_set_epi32(0, 2, 4, 6, 1, 3, 5, 7));
            ymm15_column_states = _mm256_mullo_epi32(ymm15_column_states, _mm256_set1_epi32(-eColumnState_Overlapped));
            _mm256_stream_si256((__m256i*)GetCurrentColumnStateArrayPtr(columnIndex - 31), ymm15_column_states);
            break;

         default:
            // Do nothing in these cases
            break;
         }

         // Multiply in the boost values, store the results in the totals array for SP2
         __m256i ymm16_boosted = _mm256_load_si256((__m256i*)&dpColumnBoostArray[columnIndex]);
         ymm16_boosted = _mm256_mullo_epi32(ymm16_boosted, ymm12_output_total32);
         _mm256_stream_si256((__m256i*)&dpColumnActivationTotalArray[columnIndex], ymm16_boosted);

      }
      default: break;
      }
   }


   // SP2 - Compute Active Columns After Inhibition (Is column activation within kth highest across inhibition distance)
   
   __m256i ymm0_desired_active = _mm256_set1_epi32(dSpatialPoolerConfig.desiredNumActiveColumns);
   __m256i ymm5_column_states;

   // Shift total activations across each other while comparing to get maximum activation
   int startInhibitionRangeColumnIndex = dNumColumns - dSpatialPoolerConfig.inhibitionDistance/2;
   int currentInhibitionRangeColumnIndex;   
   
   for (int columnIndex = 0; columnIndex < dNumColumns; columnIndex += 8)
   {      
      __m256i ymm1_greater_counts = _mm256_set1_epi32(0);                        // Initialize counters to see how many other cols have larger activation values
      __m256i ymm2_column = _mm256_stream_load_si256((__m256i const*) &dpColumnActivationTotalArray[columnIndex]);  // Load a vector of column activation totals

      for (int inhibitionCount = 0; inhibitionCount < dSpatialPoolerConfig.inhibitionDistance; inhibitionCount += 8)
      {
         currentInhibitionRangeColumnIndex = startInhibitionRangeColumnIndex + columnIndex;
         if (currentInhibitionRangeColumnIndex > dNumColumns)
            currentInhibitionRangeColumnIndex -= dNumColumns;

         __m256i ymm3_column_moving = _mm256_stream_load_si256((__m256i const*) &dpColumnActivationTotalArray[currentInhibitionRangeColumnIndex]);

         // Count the number of values larger than the column total in the 'moving' vector
         __m256i ymm3_greater_mask;
         //ymm3_column_moving = _mm256_shuffle_epi32(ymm3_column_moving, 0x1B);  // Not needed, does nothing
         ymm3_greater_mask = _mm256_cmpgt_epi32(ymm2_column, ymm3_column_moving);
         ymm1_greater_counts = _mm256_add_epi32(ymm1_greater_counts, ymm3_greater_mask);  // By adding the masks, we are basically adding -1s together

         ymm3_column_moving = _mm256_shuffle_epi32(ymm3_column_moving, 0xC6);
         ymm3_greater_mask = _mm256_cmpgt_epi32(ymm2_column, ymm3_column_moving);
         ymm1_greater_counts = _mm256_add_epi32(ymm1_greater_counts, ymm3_greater_mask);

         ymm3_column_moving = _mm256_shuffle_epi32(ymm3_column_moving, 0xB1);
         ymm3_greater_mask = _mm256_cmpgt_epi32(ymm2_column, ymm3_column_moving);
         ymm1_greater_counts = _mm256_add_epi32(ymm1_greater_counts, ymm3_greater_mask);

         ymm3_column_moving = _mm256_shuffle_epi32(ymm3_column_moving, 0x6C);
         ymm3_greater_mask = _mm256_cmpgt_epi32(ymm2_column, ymm3_column_moving);
         ymm1_greater_counts = _mm256_add_epi32(ymm1_greater_counts, ymm3_greater_mask);

         ymm3_column_moving = _mm256_permute4x64_epi64(ymm3_column_moving, 0xB1);  // Switch 2 128 bit halves
         ymm3_greater_mask = _mm256_cmpgt_epi32(ymm2_column, ymm3_column_moving);
         ymm1_greater_counts = _mm256_add_epi32(ymm1_greater_counts, ymm3_greater_mask);

         ymm3_column_moving = _mm256_shuffle_epi32(ymm3_column_moving, 0xC6);
         ymm3_greater_mask = _mm256_cmpgt_epi32(ymm2_column, ymm3_column_moving);
         ymm1_greater_counts = _mm256_add_epi32(ymm1_greater_counts, ymm3_greater_mask);

         ymm3_column_moving = _mm256_shuffle_epi32(ymm3_column_moving, 0xB1);
         ymm3_greater_mask = _mm256_cmpgt_epi32(ymm2_column, ymm3_column_moving);
         ymm1_greater_counts = _mm256_add_epi32(ymm1_greater_counts, ymm3_greater_mask);

         ymm3_column_moving = _mm256_shuffle_epi32(ymm3_column_moving, 0x6C);
         ymm3_greater_mask = _mm256_cmpgt_epi32(ymm2_column, ymm3_column_moving);
         ymm1_greater_counts = _mm256_add_epi32(ymm1_greater_counts, ymm3_greater_mask);
      }

      // Check to see if any columns should be active
      __m256i ymm4_active_mask = _mm256_cmpgt_epi32(ymm0_desired_active, ymm1_greater_counts);  // Mask out the cols that have less than the desired count of active columns (these are active)
            
      if (_mm256_movemask_epi8(ymm4_active_mask))                                               // Optimization to skip building indexes for vectors with no active cols
      {
         // Press the 32bit mask values to 8 bit values so they can be stored in the column state flags
         switch (columnIndex % 32)
         {
         case 7:  ymm5_column_states = _mm256_shuffle_epi8(ymm4_active_mask, _mm256_set_epi32(0x00000000, 0x01010101, 0x02020202, 0x03030303, 0x00000000, 0x01010101, 0x02020202, 0x03030303)); break;
         case 15: ymm5_column_states = _mm256_shuffle_epi8(ymm4_active_mask, _mm256_set_epi32(0x04040404, 0x05050505, 0x06060606, 0x07070707, 0x04040404, 0x05050505, 0x06060606, 0x07070707)); break;
         case 23: ymm5_column_states = _mm256_shuffle_epi8(ymm4_active_mask, _mm256_set_epi32(0x08080808, 0x09090909, 0x0A0A0A0A, 0x0B0B0B0B, 0x08080808, 0x09090909, 0x0A0A0A0A, 0x0B0B0B0B)); break;
         case 31: ymm5_column_states = _mm256_shuffle_epi8(ymm4_active_mask, _mm256_set_epi32(0x0C0C0C0C, 0x0D0D0D0D, 0x0E0E0E0E, 0x0F0F0F0F, 0x0C0C0C0C, 0x0D0D0D0D, 0x0E0E0E0E, 0x0F0F0F0F));

            // Store the column state flags for columns that had activation higher than the current minimum
            ymm5_column_states = _mm256_permutevar8x32_epi32(ymm5_column_states, _mm256_set_epi32(0, 2, 4, 6, 1, 3, 5, 7));
            ymm5_column_states = _mm256_mullo_epi32(ymm5_column_states, _mm256_set1_epi32(-eColumnState_Active));

            __m256i ymm6_new_column_states = _mm256_load_si256((__m256i*)GetCurrentColumnStateArrayPtr(columnIndex - 31));  // Load the old flags
            ymm6_new_column_states = _mm256_or_si256(ymm6_new_column_states, ymm5_column_states);                           // OR in the new active flags
            _mm256_stream_si256((__m256i*)GetCurrentColumnStateArrayPtr(columnIndex - 31), ymm6_new_column_states);         // Store the new flags
            break;

         default:
            // Do nothing in these cases
            break;
         }

         // Walk the values in the active_mask, this isn't AVX, but there shouldn't be too many active columns and not much room for parallel speedup
         int activeStateBitMask = _mm256_movemask_epi8(ymm4_active_mask);

         // Create a list of the active state indexes for easy traversal of the active columns in later TP states
         if (activeStateBitMask & 0xF) { dpActiveColumnIndexArray[dCurrentNumActiveColumns] = columnIndex + 0; dCurrentNumActiveColumns++; dpColumnActiveDutyCycleTotals[columnIndex + 0]++; }
         if (activeStateBitMask & 0xF0) { dpActiveColumnIndexArray[dCurrentNumActiveColumns] = columnIndex + 1; dCurrentNumActiveColumns++; dpColumnActiveDutyCycleTotals[columnIndex + 1]++; }
         if (activeStateBitMask & 0xF00) { dpActiveColumnIndexArray[dCurrentNumActiveColumns] = columnIndex + 2; dCurrentNumActiveColumns++; dpColumnActiveDutyCycleTotals[columnIndex + 2]++; }
         if (activeStateBitMask & 0xF000) { dpActiveColumnIndexArray[dCurrentNumActiveColumns] = columnIndex + 3; dCurrentNumActiveColumns++; dpColumnActiveDutyCycleTotals[columnIndex + 3]++; }
         if (activeStateBitMask & 0xF0000) { dpActiveColumnIndexArray[dCurrentNumActiveColumns] = columnIndex + 4; dCurrentNumActiveColumns++; dpColumnActiveDutyCycleTotals[columnIndex + 4]++; }
         if (activeStateBitMask & 0xF00000) { dpActiveColumnIndexArray[dCurrentNumActiveColumns] = columnIndex + 5; dCurrentNumActiveColumns++; dpColumnActiveDutyCycleTotals[columnIndex + 5]++; }
         if (activeStateBitMask & 0xF000000) { dpActiveColumnIndexArray[dCurrentNumActiveColumns] = columnIndex + 6; dCurrentNumActiveColumns++; dpColumnActiveDutyCycleTotals[columnIndex + 6]++; }
         if (activeStateBitMask & 0xF0000000) { dpActiveColumnIndexArray[dCurrentNumActiveColumns] = columnIndex + 7; dCurrentNumActiveColumns++; dpColumnActiveDutyCycleTotals[columnIndex + 7]++; }
      }

      // Subtract the oldest active state columns from the duty cycle totals
      uint8_t* pOldestColumnStateArrayPtr = GetOldestColumnStateArrayPtr(0);
      for (int i = 0; i < 8; i++)
      {
         if (pOldestColumnStateArrayPtr[i] && eColumnState_Active)
         {
            dpColumnActiveDutyCycleTotals[columnIndex + i]--;
         }
      }
      
   }


   // SP3 - Spatial Learning

   // Strengthen/Weaken the synaspe permanences on the active columns for this input pattern
   pCurrentInputPtr = (__m256i*)dpInputSpace->GetBuffer();

   for (int i = 0; i < dCurrentNumActiveColumns; i++)
   {
      int activeColumnIndex = dpActiveColumnIndexArray[i];
      int receptiveRange = dpInputSpace->GetSizeX() * dpInputSpace->GetSizeY();

      __m256i* pCurrentProximalPermPtr = (__m256i*)GetProximalSegmentPermanenceArray(activeColumnIndex);
      for (int synIndex = 0; synIndex < receptiveRange; synIndex++)
      {
         __m256i ymm0_increase = _mm256_set1_epi8((char)VHTM_PERMANENCE_INCREASE);     // Load the value we will increase the correct synaspes
         __m256i ymm1_decrease = _mm256_set1_epi8((char)VHTM_PERMANENCE_DECREASE);     // Load the value we will decrease the correct synaspes
         __m256i ymm2_input = _mm256_load_si256(pCurrentInputPtr);                     // Load the input data to use as mask
         __m256i ymm3_update = _mm256_blendv_epi8(ymm0_increase, ymm1_decrease, ymm2_input);
         __m256i ymm4_perm2 = _mm256_load_si256(pCurrentProximalPermPtr);               // Load the proximal synaspe permanence values
         ymm4_perm2 = _mm256_adds_epi8(ymm4_perm2, ymm3_update);
         _mm256_stream_si256(pCurrentProximalPermPtr, ymm4_perm2);                      // Store the new permanences
         pCurrentProximalPermPtr += sizeof(__m256i);
         pCurrentInputPtr += sizeof(__m256i);
      }
   }

   // Process the statistics for calculating receptive range - Column Active Duty Rate
   for (int columnIndex = 0; columnIndex < dNumColumns; columnIndex += 8)
   {
      // Modify the Column Boost Array using the updated ACTIVE duty cycle
      __m256i ymm5_active_duty_cycle = _mm256_load_si256((__m256i*)&dpColumnActiveDutyCycleTotals[currentInhibitionRangeColumnIndex]);

      // Find the max duty cycle of the column neighbors, then take ~1% for the min
      __m256i ymm6_max_value;
      __m256i ymm7_shuffled_max;
      for (int inhibitionIndex = 0; inhibitionIndex < dSpatialPoolerConfig.inhibitionDistance; inhibitionIndex += 8)
      {
         currentInhibitionRangeColumnIndex = startInhibitionRangeColumnIndex + columnIndex;
         if (currentInhibitionRangeColumnIndex > dNumColumns)
            currentInhibitionRangeColumnIndex -= dNumColumns;

         ymm7_shuffled_max = _mm256_load_si256((__m256i*)&dpColumnActiveDutyCycleTotals[currentInhibitionRangeColumnIndex]);
         ymm6_max_value = _mm256_max_epi32(ymm7_shuffled_max, ymm6_max_value);
      }

      // Find the max in the vector (horizontally)
      ymm7_shuffled_max = _mm256_shuffle_epi32(ymm6_max_value, 0x4E);
      ymm6_max_value = _mm256_max_epi32(ymm7_shuffled_max, ymm6_max_value);
      ymm7_shuffled_max = _mm256_shuffle_epi32(ymm6_max_value, 0xE4);
      ymm6_max_value = _mm256_max_epi32(ymm7_shuffled_max, ymm6_max_value);
      ymm7_shuffled_max = _mm256_permute4x64_epi64(ymm6_max_value, 0xB1);       // Switch 2 128 bit halves
      ymm6_max_value = _mm256_max_epi32(ymm7_shuffled_max, ymm6_max_value);
      ymm6_max_value = _mm256_srli_epi32(ymm6_max_value, 7);                   // Divide by 128 to get about ~1% of max (max becomes the min)
      
      __m256i ymm8_boost_mask = _mm256_cmpgt_epi32(ymm6_max_value, ymm5_active_duty_cycle); // Compare to see which columns are below min, and then we will raise their boost

      if (_mm256_testz_si256(ymm8_boost_mask, _mm256_set1_epi32(0xFF)) > 0)                 // Optimization to skip boost adjustment if all columns are above the minimum
      {
         // Adjust the boost for columns that are above and below the min
         __m256i ymm9_boosted = _mm256_load_si256((__m256i*)&dpColumnBoostArray[columnIndex]);
         __m256i ymm10_boost_delta = _mm256_blendv_epi8(_mm256_set1_epi32(dSpatialPoolerConfig.boostIncrement), _mm256_set1_epi32(-dSpatialPoolerConfig.boostIncrement), ymm8_boost_mask);

         // Make sure we have not gone above the max or below the min
         ymm8_boost_mask = _mm256_cmpgt_epi32(_mm256_set1_epi32(dSpatialPoolerConfig.boostMax), ymm9_boosted);
         ymm10_boost_delta = _mm256_and_si256(ymm10_boost_delta, ymm8_boost_mask);

         ymm8_boost_mask = _mm256_cmpgt_epi32(ymm9_boosted, _mm256_set1_epi32(dSpatialPoolerConfig.boostMin));
         ymm10_boost_delta = _mm256_and_si256(ymm10_boost_delta, ymm8_boost_mask);

         // Store off the modified boost values
         ymm9_boosted = _mm256_add_epi32(ymm9_boosted, ymm10_boost_delta);
         _mm256_stream_si256((__m256i*)&dpColumnBoostArray[columnIndex], ymm9_boosted);   // Save the newly adjusted boost values
      }

      // Walk the values in the mask that represent the cols that the OVERLAP is below the ACTIVE duty cycle min (not sure why this isn't overlap duty cycle), increase all the perms of those cols
      __m256i ymm8_overlap_duty_cycle = _mm256_load_si256((__m256i*)&dpColumnOverlapDutyCycleTotals[columnIndex]); 
      __m256i ymm9_overlap_mask = _mm256_cmpgt_epi32(ymm6_max_value, ymm5_active_duty_cycle); // Compare to see which columns are below min active duty cycle
      int activeDutyBitMask = _mm256_movemask_epi8(ymm9_overlap_mask);
      if (activeDutyBitMask)
      {
         if (activeDutyBitMask & 0xF) IncreaseProximalPermanences(columnIndex + 0);
         if (activeDutyBitMask & 0xF0) IncreaseProximalPermanences(columnIndex + 1);
         if (activeDutyBitMask & 0xF00) IncreaseProximalPermanences(columnIndex + 2);
         if (activeDutyBitMask & 0xF000) IncreaseProximalPermanences(columnIndex + 3);
         if (activeDutyBitMask & 0xF0000) IncreaseProximalPermanences(columnIndex + 4);
         if (activeDutyBitMask & 0xF00000) IncreaseProximalPermanences(columnIndex + 5);
         if (activeDutyBitMask & 0xF000000) IncreaseProximalPermanences(columnIndex + 6);
         if (activeDutyBitMask & 0xF0000000) IncreaseProximalPermanences(columnIndex + 7);
      }
   }

   // TODO: CURRENTLY WE ASSUME dInhibitionDistance is constant because the input pattern will always cover all the cells equally, this
   // means the average distance of connected synaspes is 1/2 of the dNumCols.  If this assumption changes, then add AverageReceptiveField()

}


void vRegion::IncreaseProximalPermanences(int columnIndex)
{
   int8_t* pPermanences = GetProximalSegmentPermanenceArray(columnIndex);
   int numPermanences = dpInputSpace->GetSizeX() * dpInputSpace->GetSizeY();

   for (int i = 0; i < numPermanences; i += 32)
   {
      __m256i ymm0_perm = _mm256_load_si256((__m256i*)&pPermanences[i]);
      ymm0_perm = _mm256_adds_epi8(ymm0_perm, _mm256_set1_epi8(dSpatialPoolerConfig.noOverlapPermanenceIncrement));
      _mm256_stream_si256((__m256i*)&pPermanences[i], ymm0_perm);
   }
}
