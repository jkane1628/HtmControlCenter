
#include "stdint.h"
#include "immintrin.h"
#include "malloc.h"

#include "vInputSpace.h"
#include "vRegion.h"


void vRegion::PerformSpatialPooling(bool allowLearning)
{
   // SP1 - Compute Input Overlap

   __m256i* pCurrentInputPtr = (__m256i*)dpInputSpace->GetBuffer();
   int* pColumnActivationTotalArray = (int*)_aligned_malloc(dNumColumns*sizeof(int), VHTM_MEMORY_ALIGNMENT_VALUE);

   for (int columnIndex = 0; columnIndex < dNumColumns; columnIndex++)
   { 
      __m256i* pCurrentProximalPermPtr = (__m256i*)GetProximalSegmentPermanenceArray(columnIndex);
      int receptiveRange = dpInputSpace->GetSizeX() * dpInputSpace->GetSizeY();

      // int overlap = Calculate Overlap( uint8* pBuf1, uint8* pBuf1, int range, uint8 boost);

      int subtotal = 255;
      __m256i ymm0_threshold = _mm256_set1_epi8((char)VHTM_CONNECTED_PERMANENCE);     // Load threshold value ;
      __m256i ymm6_overlap_total8 = _mm256_set1_epi8(0);
      __m256i ymm7_one8 = _mm256_set1_epi8(1);
      __m256i ymm9_boost = _mm256_set1_epi8((char)0);// GetColumnObject(columnIndex)->dBoost); // Load column boost value (optional)
      __m256i ymm10_overlap_total32 = _mm256_set1_epi32(0);
      __m256i ymm4_overlap;

      for (int synIndex = 0; synIndex < receptiveRange; synIndex++)
      {
         __m256i ymm1_perm = _mm256_load_si256(pCurrentProximalPermPtr);         // Load the proximal synaspe permanence values

         __m256i ymm2_connected = _mm256_cmpgt_epi8(ymm0_threshold, ymm1_perm);  // Compare to get a vector of connected synapses

         __m256i ymm3_input = _mm256_load_si256(pCurrentInputPtr);               // Load the input data

         ymm4_overlap = _mm256_and_si256(ymm2_connected, ymm3_input);            // bitwise-AND of input and connected synapses to get overlap

         // Only need to mask off active bit if buffer has values other than 0 and 1 in it

         ymm6_overlap_total8 = _mm256_adds_epu8(ymm6_overlap_total8, ymm4_overlap);

         if (subtotal == 0)
         {              
            __m256i ymm8_overlap_interm16 = _mm256_maddubs_epi16(ymm4_overlap, ymm7_one8);            // (uint8)overlap * (int8)column boost to a uint16 value, then SUM each uint16      
            __m256i ymm10_overlap_interm32 = _mm256_madd_epi16(ymm8_overlap_interm16, ymm9_boost);    // Mult the boost in and hadd u16s to u32s
            ymm10_overlap_total32 = _mm256_add_epi32(ymm10_overlap_total32, ymm10_overlap_interm32);  // Add the u32 intermediate totals to overall totals
            ymm6_overlap_total8 = _mm256_set1_epi8(0);                                                // Clear the intermediate 8bit totals
            subtotal = 256;
         }
         subtotal--;
         pCurrentProximalPermPtr += sizeof(__m256i);
      }


      if (subtotal != 255)
      {
         __m256i ymm8_overlap_interm16 = _mm256_maddubs_epi16(ymm4_overlap, ymm7_one8);  // (uint8)overlap * (int8)column boost to a uint16 value, then SUM each uint16      
         __m256i ymm10_overlap_interm32 = _mm256_madd_epi16(ymm8_overlap_interm16, ymm9_boost);
         ymm10_overlap_total32 = _mm256_add_epi32(ymm10_overlap_total32, ymm10_overlap_interm32);
      }

      __m256i ymm11_zero = _mm256_set1_epi32(0);
      ymm10_overlap_total32 = _mm256_hadd_epi32(ymm10_overlap_total32, ymm11_zero);
	   ymm10_overlap_total32 = _mm256_hadd_epi32(ymm10_overlap_total32, ymm11_zero);
	   ymm10_overlap_total32 = _mm256_hadd_epi32(ymm10_overlap_total32, ymm11_zero);
	   ymm10_overlap_total32 = _mm256_hadd_epi32(ymm10_overlap_total32, ymm11_zero);
	  
      __m128i ymm10_overlap_total32_casted = _mm256_castsi256_si128(ymm10_overlap_total32);
      ymm10_overlap_total32 = _mm256_broadcastd_epi32(ymm10_overlap_total32_casted);

      int insertTotalMask = 0x80;

      __m256i ymm12_output_total32 = _mm256_blend_epi32(ymm12_output_total32, ymm10_overlap_total32, insertTotalMask);
      insertTotalMask = insertTotalMask >> 1;
      
      if (insertTotalMask == 0)  // Loop assumes that dNumCols always a multiple of 8 (since 8 values fit in a 256 and none spill over to a non-filled vector)
      {
         // Filter out the values below the overlap minimum, increment the overlap statistic totals with the values that are higher than the min
         __m256i ymm13_overlap_min = _mm256_set1_epi32(VHTM_MIN_OVERLAP_THRESHOLD_DEFAULT);
         __m256i ymm14_overlap_min_mask = _mm256_cmpgt_epi32(ymm12_output_total32, ymm13_overlap_min);
         ymm12_output_total32 = _mm256_and_si256(ymm12_output_total32, ymm14_overlap_min_mask);

         // Multiply in the boost values, store the results in the totals array for SP2
         __m256i ymm16_boosted = _mm256_load_si256((__m256i*)&dpColumnBoostArray[columnIndex]);
         ymm16_boosted = _mm256_mul_epu32(ymm16_boosted, ymm12_output_total32);
         _mm256_stream_si256((__m256i*)&pColumnActivationTotalArray[columnIndex], ymm16_boosted);

         // Update the overlap duty cycle counts
         __m256i ymm15_overlap_dc_totals = _mm256_load_si256((__m256i*)&dpColumnOverlapDutyCycleTotals[columnIndex]);

         // TODO: NEED TO ADD THE MIN_MASK, THEN SUBTRACT THE OLDEST FROM THE TOTALS USING STATE FLAGS...THEN DO SAME FOR ACTIVE DUTY CYCLE

         ymm15_overlap_dc_totals = _mm256_sub_epi32(ymm15_overlap_dc_totals, ymm14_overlap_min_mask);
         _mm256_stream_si256((__m256i*)&dpColumnOverlapDutyCycleTotals[columnIndex], ymm15_overlap_dc_totals);

         insertTotalMask = 0x80;
      }
   }


   // SP2 - Compute Active Columns After Inhibition (Is column activation within kth highest across inhibition distance)
   
   __m256i ymm0_desired_active = _mm256_set1_epi32(dDesiredNumActiveColumns);
   __m256i ymm4_32_to_8 = _mm256_set_epi8(0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
   __m256i ymm5_64_mask = _mm256_set_epi32(0xFF, 0xFF, 0, 0, 0, 0, 0, 0);


   // Shift total activations across each other while comparing
   int startInhibitionRangeColumnIndex = dNumColumns - dInhibitionDistance/2;
   int currentInhibitionRangeColumnIndex;
   uint8_t* pCurrentColumnStateArrayPtr = GetCurrentColumnStateArrayPtr(0);
   
   for (int columnIndex = 0; columnIndex < dNumColumns; columnIndex += 8)
   {      
      __m256i ymm1_greater_total = _mm256_set1_epi32(0);
      __m256i ymm2_column = _mm256_stream_load_si256((__m256i const*) &pColumnActivationTotalArray[columnIndex]);

      for (int inhibitionCount = 0; inhibitionCount < dInhibitionDistance; inhibitionCount += 8)
      {
         currentInhibitionRangeColumnIndex = startInhibitionRangeColumnIndex + columnIndex;
         if (currentInhibitionRangeColumnIndex > dNumColumns)
            currentInhibitionRangeColumnIndex -= dNumColumns;

         __m256i ymm3_column_moving = _mm256_stream_load_si256((__m256i const*) &pColumnActivationTotalArray[currentInhibitionRangeColumnIndex]);

         __m256i ymm3_greater_mask;
         //ymm3_column_moving = _mm256_shuffle_epi32(ymm3_column_moving, 0x1B);  // Not needed, does nothing
         ymm3_greater_mask = _mm256_cmpgt_epi32(ymm2_column, ymm3_column_moving);
         ymm1_greater_total = _mm256_add_epi32(ymm1_greater_total, ymm3_greater_mask);

         ymm3_column_moving = _mm256_shuffle_epi32(ymm3_column_moving, 0xC6);
         ymm3_greater_mask = _mm256_cmpgt_epi32(ymm2_column, ymm3_column_moving);
         ymm1_greater_total = _mm256_add_epi32(ymm1_greater_total, ymm3_greater_mask);

         ymm3_column_moving = _mm256_shuffle_epi32(ymm3_column_moving, 0xB1);
         ymm3_greater_mask = _mm256_cmpgt_epi32(ymm2_column, ymm3_column_moving);
         ymm1_greater_total = _mm256_add_epi32(ymm1_greater_total, ymm3_greater_mask);

         ymm3_column_moving = _mm256_shuffle_epi32(ymm3_column_moving, 0x6C);
         ymm3_greater_mask = _mm256_cmpgt_epi32(ymm2_column, ymm3_column_moving);
         ymm1_greater_total = _mm256_add_epi32(ymm1_greater_total, ymm3_greater_mask);

         ymm3_column_moving = _mm256_permute4x64_epi64(ymm3_column_moving, 0xB1);  // Switch 2 128 bit halves
         ymm3_greater_mask = _mm256_cmpgt_epi32(ymm2_column, ymm3_column_moving);
         ymm1_greater_total = _mm256_add_epi32(ymm1_greater_total, ymm3_greater_mask);

         ymm3_column_moving = _mm256_shuffle_epi32(ymm3_column_moving, 0xC6);
         ymm3_greater_mask = _mm256_cmpgt_epi32(ymm2_column, ymm3_column_moving);
         ymm1_greater_total = _mm256_add_epi32(ymm1_greater_total, ymm3_greater_mask);

         ymm3_column_moving = _mm256_shuffle_epi32(ymm3_column_moving, 0xB1);
         ymm3_greater_mask = _mm256_cmpgt_epi32(ymm2_column, ymm3_column_moving);
         ymm1_greater_total = _mm256_add_epi32(ymm1_greater_total, ymm3_greater_mask);

         ymm3_column_moving = _mm256_shuffle_epi32(ymm3_column_moving, 0x6C);
         ymm3_greater_mask = _mm256_cmpgt_epi32(ymm2_column, ymm3_column_moving);
         ymm1_greater_total = _mm256_add_epi32(ymm1_greater_total, ymm3_greater_mask);
      }

      // Check to see if any columns should be active
      __m256i ymm4_active_mask = _mm256_cmpgt_epi32(ymm1_greater_total, ymm0_desired_active);  // Mask out the cols that are above the desired threshold
      

      int no_active_col_flag = _mm256_testz_si256(ymm4_active_mask, _mm256_set1_epi32(0xFF));  // Optimization to skip building indexes for vectors with no active cols
      
      // Prep the active cols to be stored in column state array (TODO: THESE NEXT INSTRUCTIONS COULD BE REDUCED/REMOVED IF ACTIVE STATE FLAG WAS 0XFF, WOULD MAKE TP2 EASIER TOO)
      ymm4_active_mask = _mm256_blendv_epi8(_mm256_set1_epi32(0), _mm256_set1_epi32(eColumnState_Active), ymm4_active_mask);  // Change the 0xFF mask values to the active state flag
      ymm4_active_mask = _mm256_shuffle_epi8(ymm4_active_mask, ymm4_32_to_8);  // Convert the 32 bit mask values to 8 bit values that are used in the storeage (256b vector to 64b vector)
      
      __m256i ymm5_new_states = _mm256_stream_load_si256((__m256i const*) pCurrentColumnStateArrayPtr);
      ymm5_new_states = _mm256_add_epi8(ymm5_new_states, ymm4_active_mask);

      // Store off the active states in the column state array
      _mm256_maskstore_epi32((int*)pCurrentColumnStateArrayPtr, ymm5_64_mask, ymm5_new_states);  

      // Create a list of the active state indexes for easy traversal of the active columns in later TP states
      if (!no_active_col_flag)
      {
         for (int i = 0; i < 8; i++)
         {
            if (pCurrentColumnStateArrayPtr[i] && eColumnState_Active)
            {
               dpActiveColumnIndexArray[dCurrentNumActiveColumns] = columnIndex + i;
               dCurrentNumActiveColumns++;
            }
         }
      }
      pCurrentColumnStateArrayPtr += 8;      
   }


   // SP3 - Spatial Learning

   // Strengthen/Weaken the synaspe permanences on the active columns for this input pattern
   for (int i = 0; i < dCurrentNumActiveColumns; i++)
   {
      int activeColumnIndex = dpActiveColumnIndexArray[i];
      int receptiveRange = dpInputSpace->GetSizeX() * dpInputSpace->GetSizeY();

      __m256i* pCurrentProximalPermPtr = (__m256i*)GetProximalSegmentPermanenceArray(activeColumnIndex);
      for (int synIndex = 0; synIndex < receptiveRange; synIndex++)
      {
         __m256i ymm0_increase = _mm256_set1_epi8((char)VHTM_PERMANENCE_INCREASE);     // Load the value we will increase the correct synaspes
         __m256i ymm1_decrease = _mm256_set1_epi8((char)VHTM_PERMANENCE_DECREASE);     // Load the value we will decrease the correct synaspes
         __m256i ymm2_input = _mm256_load_si256(pCurrentInputPtr);                  // Load the input data to use as mask
         __m256i ymm3_update = _mm256_blendv_epi8(ymm0_increase, ymm1_decrease, ymm2_input);
         __m256i ymm4_perm = _mm256_load_si256(pCurrentProximalPermPtr);         // Load the proximal synaspe permanence values
         __m256i ymm4_perm = _mm256_adds_epi8(ymm4_perm, ymm3_update);
         _mm256_stream_si256(pCurrentProximalPermPtr, ymm4_perm);
         pCurrentProximalPermPtr += sizeof(__m256i);
      }
   }

   // Update the statistics for calculating receptive range - Max Duty Rate
   for (int columnIndex = 0; columnIndex < dNumColumns; columnIndex += 8)
   {
      // Modify the Column Boost Array using the updated ACTIVE duty cycle
      __m256i ymm5_overlap_totals = _mm256_load_si256((__m256i*)&dpColumnActiveDutyCycleTotals[columnIndex]);

      __m256i ymm6_max_value;
      __m256i ymm7_shuffled_max;
      for (int inhibitionCount = 0; inhibitionCount < dInhibitionDistance; inhibitionCount += 8)
      {
         currentInhibitionRangeColumnIndex = startInhibitionRangeColumnIndex + columnIndex;
         if (currentInhibitionRangeColumnIndex > dNumColumns)
            currentInhibitionRangeColumnIndex -= dNumColumns;

         ymm6_max_value = _mm256_max_epi32(ymm5_overlap_totals, ymm6_max_value);
      }

      // Find the max in the vector (horizontally)
      ymm7_shuffled_max = _mm256_shuffle_epi32(ymm6_max_value, 0x4E);
      ymm6_max_value = _mm256_max_epi32(ymm5_overlap_totals, ymm6_max_value);
      ymm7_shuffled_max = _mm256_shuffle_epi32(ymm6_max_value, 0xE4);
      ymm6_max_value = _mm256_max_epi32(ymm5_overlap_totals, ymm6_max_value);
      ymm7_shuffled_max = _mm256_permute4x64_epi64(ymm6_max_value, 0xB1);              // Switch 2 128 bit halves
      ymm6_max_value = _mm256_max_epi32(ymm5_overlap_totals, ymm6_max_value);
      __m256i ymm6_max_value = _mm256_srli_epi32(ymm6_max_value, 7);                   // Divide by 128 to get about ~1% of max
      __m256i ymm7_boost_mask = _mm256_cmpgt_epi32(ymm6_max_value, ymm5_overlap_totals); // Compare to see which are below min

      // Adjust the boost for columns that are above and below the min
      __m256i ymm8_boosted = _mm256_load_si256((__m256i*)&dpColumnBoostArray[columnIndex]);
      __m256i ymm9_boost_delta = _mm256_blendv_epi8(_mm256_set1_epi32(dColumnBoostIncrement), _mm256_set1_epi32(-dColumnBoostIncrement), ymm7_boost_mask);

      // Make sure we have not gone above the max or below the min
      ymm7_boost_mask = _mm256_cmpgt_epi32(_mm256_set1_epi32(dColumnBoostMax), ymm8_boosted);
      ymm9_boost_delta = _mm256_and_si256(ymm9_boost_delta, ymm7_boost_mask);

      ymm7_boost_mask = _mm256_cmpgt_epi32(ymm8_boosted, _mm256_set1_epi32(dColumnBoostMin));
      ymm9_boost_delta = _mm256_and_si256(ymm9_boost_delta, ymm7_boost_mask);

      // Store off the modified boost values
      ymm8_boosted = _mm256_add_epi32(ymm8_boosted, ymm9_boost_delta);
      _mm256_stream_si256((__m256i*)&dpColumnBoostArray[columnIndex], ymm8_boosted);
      


      // Modify the Column Boost Array using the updated OVERLAP duty cycle
      __m256i ymm5_overlap_totals = _mm256_load_si256((__m256i*)&dpColumnOverlapDutyCycleTotals[columnIndex]);
      for (int inhibitionCount = 0; inhibitionCount < dInhibitionDistance; inhibitionCount += 8)
      {
         currentInhibitionRangeColumnIndex = startInhibitionRangeColumnIndex + columnIndex;
         if (currentInhibitionRangeColumnIndex > dNumColumns)
            currentInhibitionRangeColumnIndex -= dNumColumns;

         ymm6_max_value = _mm256_max_epi32(ymm5_overlap_totals, ymm6_max_value);
      }

      // Find the max in the vector (horizontally)
      ymm7_shuffled_max = _mm256_shuffle_epi32(ymm6_max_value, 0x4E);
      ymm6_max_value = _mm256_max_epi32(ymm5_overlap_totals, ymm6_max_value);
      ymm7_shuffled_max = _mm256_shuffle_epi32(ymm6_max_value, 0xE4);
      ymm6_max_value = _mm256_max_epi32(ymm5_overlap_totals, ymm6_max_value);
      ymm7_shuffled_max = _mm256_permute4x64_epi64(ymm6_max_value, 0xB1);              // Switch 2 128 bit halves
      ymm6_max_value = _mm256_max_epi32(ymm5_overlap_totals, ymm6_max_value);
      __m256i ymm6_max_value = _mm256_srli_epi32(ymm6_max_value, 7);                   // Divide by 128 to get about ~1% of max
      __m256i ymm7_overlap_mask = _mm256_cmpgt_epi32(ymm5_overlap_totals, ymm6_max_value); // Compare to see which are ABOVE min

      // Walk the values in the mask that represent the cols that are below the overlap duty cycle min, then increase the perms of those cols
      __m256i ymm8_test_mask = _mm256_set_epi32(0,0,0,0,0,0,0,0xFFFFFFFFF);
      for (int i = 0; i < 4; i++)
      {
         if (_mm256_testz_si256(ymm7_overlap_mask, ymm8_test_mask))
         {
            IncreaseProximalPermanences(columnIndex + i);
         }
         ymm8_test_mask = _mm256_slli_si256(ymm8_test_mask, 4);
      }
      ymm8_test_mask = _mm256_set_epi32(0, 0, 0, 0xFFFFFFFFF, 0, 0, 0, 0);
      for (int i = 0; i < 4; i++)
      {
         if (_mm256_testz_si256(ymm7_overlap_mask, ymm8_test_mask))
         {
            IncreaseProximalPermanences(columnIndex + i);
         }
         ymm8_test_mask = _mm256_slli_si256(ymm8_test_mask, 4);
      }
   }

   // TODO: CURRENTLY WE ASSUME dInhibitionDistance is constant because the input pattern will always cover all the cells equally, this
   // means the average distance of connected synaspes is 1/2 of the dNumCols.  If this assumption changes, then add AverageReceptiveField()

   free(pColumnActivationTotalArray);
}


void vRegion::IncreaseProximalPermanences(int columnIndex)
{
   int8_t* pPermanences = GetProximalSegmentPermanenceArray(columnIndex);
   int numPermanences = dpInputSpace->GetSizeX() * dpInputSpace->GetSizeY();

   for (int i = 0; i < numPermanences; i += 32)
   {
      __m256i ymm0_perm = _mm256_load_si256((__m256i*)&pPermanences[i]);
      ymm0_perm = _mm256_adds_epi8(ymm0_perm, _mm256_set1_epi8(dColumnPermanenceIncrement));
      _mm256_stream_si256((__m256i*)&pPermanences[i], ymm0_perm);
   }
}
