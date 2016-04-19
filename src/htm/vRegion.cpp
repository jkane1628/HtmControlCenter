
#include "stdint.h"

#include "NetworkManager.h"
#include "vInputSpace.h"
#include "vRegion.h"

vDistalSegment::vDistalSegment(int8_t* permanenceArray, int8_t* permanenceUpdateArray)
:
dpPermanenceArray(permanenceArray),
dpPermanenceUpdate(permanenceUpdateArray)
{}


#define MAX_NUM_SEGMENT_GROUPS 4

vCell::vCell()
:
dNumSegments(0)
{
   for (int i = 0; i < MAX_NUM_SEGMENT_GROUPS; i++)
   {
      dpSegmentGroupArray[i] = NULL;
   }
}


vColumn::vColumn()
:
dBoost(1)
{}


vRegion::vRegion(vInputSpace* pInputSpace, eRegionSize dRegionSize)
:
dpInputSpace(pInputSpace),
dpCellStateArray0(NULL),
dpDistalSegmentPermanenceArray(NULL),
dpColumnArray(NULL),
dpCellArray(NULL),
dDistalSegmentGroupsTotal(0),
dDistalSegmentGroupsRemaining(0),
dDistalSegmentGroupsSize(0),
dpDistalSegmentGroupsNext(0)
{
   switch (dRegionSize)
   {
   case eRegionSize_4x4x4:
      dXSize = 4;
      dYSize = 4;
      dZSize = 4;
      break;
   case eRegionSize_16x16x8:
      dXSize = 16;
      dYSize = 16;
      dZSize = 8;
      break;
   case eRegionSize_32x32x4:
      dXSize = 32;
      dYSize = 32;
      dZSize = 4;
      break; 
   default:
      dXSize = -1;
      dYSize = -1;
      dZSize = -1;
      break;
   }
   dNumCells = dXSize * dYSize * dZSize;
   dNumColumns = dXSize * dYSize;
}

vRegion::~vRegion()
{
   FreeRegion();
}

#include <malloc.h>


bool vRegion::CreateRegion()
{
   dDistalSegmentSize = dNumCells / VHTM_RECEPTIVE_RANGE_DIVISOR;

   // Memories
   dpCellStateArray0 = (uint8_t*)_aligned_malloc(dNumCells, VHTM_MEMORY_ALIGNMENT_VALUE);
   if (dpCellStateArray0 == NULL) return false;
   memset(dpCellStateArray0, 0, dNumCells);
   
   dpCellStateArray1 = (uint8_t*)_aligned_malloc(dNumCells, VHTM_MEMORY_ALIGNMENT_VALUE);
   if (dpCellStateArray1 == NULL) return false;
   memset(dpCellStateArray1, 0, dNumCells);


   int dpProximalSegmentPermanenceArraySize = dpInputSpace->GetSizeX() * dpInputSpace->GetSizeY() * dNumColumns;
   dpProximalSegmentPermanenceArray = (int8_t*)_aligned_malloc(dpProximalSegmentPermanenceArraySize, VHTM_MEMORY_ALIGNMENT_VALUE);
   if (dpProximalSegmentPermanenceArray == NULL) return false;
   memset(dpProximalSegmentPermanenceArray, VHTM_CONNECTED_PERMANENCE, dpProximalSegmentPermanenceArraySize);

   int dpDistalSegmentPermanenceArraySize = dNumCells * VHTM_TARGET_SEGMENTS_PER_CELL * 2 * dNumCells / VHTM_RECEPTIVE_RANGE_DIVISOR;
   dpDistalSegmentPermanenceArray = (int8_t*)_aligned_malloc(dpDistalSegmentPermanenceArraySize, VHTM_MEMORY_ALIGNMENT_VALUE);
   if (dpDistalSegmentPermanenceArray == NULL) return false;
   memset(dpDistalSegmentPermanenceArray, VHTM_CONNECTED_PERMANENCE, dpDistalSegmentPermanenceArraySize);
      
   dpCellArray = new vCell[dNumCells];
   dpColumnArray = new vColumn[dNumColumns];

   dDistalSegmentGroupsTotal = dNumCells * VHTM_MAX_NUM_SEGMENT_GROUPS;
   dDistalSegmentGroupsRemaining = dDistalSegmentGroupsTotal;
   dDistalSegmentGroupsSize = dDistalSegmentSize * VHTM_TARGET_SEGMENTS_PER_CELL / VHTM_MAX_NUM_SEGMENT_GROUPS;
   dpDistalSegmentGroupsNext = dpDistalSegmentPermanenceArray;

   return true;
}

void vRegion::FreeRegion()
{
   if (dpCellStateArray0) free(dpCellStateArray0);
   if (dpCellStateArray1) free(dpCellStateArray1);
   if (dpProximalSegmentPermanenceArray) free(dpProximalSegmentPermanenceArray);
   if (dpDistalSegmentPermanenceArray) free(dpDistalSegmentPermanenceArray);
   if (dpCellArray) free(dpCellArray);
   if (dpColumnArray) free(dpColumnArray);
}

void vRegion::Step()
{
   bool allowSpacialLearning = true;
   bool allowTemporalLearning = true;
   PerformSpacialPooling(allowSpacialLearning);
   PerformTemporalPooling(allowTemporalLearning);
}


#include "immintrin.h"

void vRegion::PerformSpacialPooling(bool allowLearning)
{
   // SP1 - Overlap

   __m256i* pCurrentInputPtr = (__m256i*)dpInputSpace->GetBuffer();
   

   for (int columnIndex = 0; columnIndex < dNumColumns; columnIndex++)
   { 
      __m256i* pCurrentProximalPermPtr = (__m256i*)GetProximalSegmentPermanenceArray(columnIndex);
      uint8_t boost = GetColumnObject(columnIndex)->dBoost;

      // int overlap = Calculate Overlap( uint8* pBuf1, uint8* pBuf1, int range, uint8 boost);

      int receptiveRange = dNumColumns;
      int subtotal = 255;
      __m256i ymm0_threshold = _mm256_set1_epi8((char)VHTM_CONNECTED_PERMANENCE);     // Load threshold value ;
      __m256i ymm6_overlap_total8 = _mm256_set1_epi8(0);
      __m256i ymm9_boost = _mm256_set1_epi8((char)boost); // Load column boost value 
      __m256i ymm10_overlap_total32 = _mm256_set1_epi32(0);
      __m256i ymm4_overlap;

      for (int synIndex = 0; synIndex < receptiveRange; synIndex++)
      {
         __m256i ymm1_perm = _mm256_load_si256(pCurrentProximalPermPtr);         // Load the proximal synaspe permanence values

         __m256i ymm2_connected = _mm256_cmpgt_epi8(ymm0_threshold, ymm1_perm);  // Compare to get a vector of connected synapses

         __m256i ymm3_input = _mm256_load_si256(pCurrentInputPtr);               // Load the input data

         ymm4_overlap = _mm256_and_si256(ymm2_connected, ymm3_input);            // bitwise-AND of input and connected synapses to get overlap

         // Only need to mask off active bit if buffer has values other than 0 and 1 in it
         //uint8_t activeMask[] = { 0x01, 0x01, 0x01, 0x01 }
         //__m256 ymm6_activemask = _mm256_broadcast_ss((float const *)activeMask); 
         //__m256i ymm6i_activemask = _mm256_castps_si256(ymm6_activemask);
         //__m256i ymm5_overlap = _mm256_and_si256(ymm5_overlap, ymm6i_activemask);

         ymm6_overlap_total8 = _mm256_adds_epi8(ymm6_overlap_total8, ymm4_overlap);

         if (subtotal == 0)
         {
            __m256i ymm7_one8 = _mm256_set1_epi8(1);  
            __m256i ymm8_overlap_interm16 = _mm256_maddubs_epi16(ymm4_overlap, ymm7_one8);  // (uint8)overlap * (int8)column boost to a uint16 value, then SUM each uint16      
            __m256i ymm9_boost = _mm256_set1_epi8((char)boost);                  // Load column boost value 
            __m256i ymm10_overlap_interm32 = _mm256_madd_epi16(ymm8_overlap_interm16, ymm9_boost);
            ymm10_overlap_total32 = _mm256_add_epi32(ymm10_overlap_total32, ymm10_overlap_interm32);
            ymm6_overlap_total8 = _mm256_set1_epi8(0);
            subtotal = 256;
         }
         subtotal--;
         pCurrentProximalPermPtr += sizeof(__m256i);
      }

      if (subtotal != 255)
      {
         __m256i ymm7_one8 = _mm256_set1_epi8(1);
         __m256i ymm8_overlap_interm16 = _mm256_maddubs_epi16(ymm4_overlap, ymm7_one8);  // (uint8)overlap * (int8)column boost to a uint16 value, then SUM each uint16      
         
         __m256i ymm10_overlap_interm32 = _mm256_madd_epi16(ymm8_overlap_interm16, ymm9_boost);
         ymm10_overlap_total32 = _mm256_add_epi32(ymm10_overlap_total32, ymm10_overlap_interm32);
      }

      int result[256 / 32]; // TODO: May need to align
      _mm256_store_si256((__m256i*)result, ymm10_overlap_total32);
      int totalResult = result[0];
      for (int i = 1; i < sizeof(__m256i); i++)
      {
         totalResult += result[i];
      }      
   }

   





   // SP2 - Inhibition

   // SP3 - Learning
}

void vRegion::PerformTemporalPooling(bool allowLearning)
{

}

vColumn* vRegion::GetColumnObject(int columnIndex)
{
   _ASSERT(columnIndex <= dNumColumns);
   return &dpColumnArray[columnIndex];
}

vCell* vRegion::GetCellObject(int cellIndex)
{
   _ASSERT(cellIndex <= dNumCells);
   return &dpCellArray[cellIndex];
}

int vRegion::GetTopCellIndex(int columnIndex) 
{
   return columnIndex*dZSize; 
}

uint8_t* vRegion::GetCurrentTopCellStateArrayPtr(int columnIndex)
{
   _ASSERT(columnIndex <= dNumColumns);
   int index = GetTopCellIndex(columnIndex);
   if (dTimestep % 2)
      return &dpCellStateArray0[index];
   else
      return &dpCellStateArray1[index];
}

uint8_t* vRegion::GetPreviousTopCellStateArrayPtr(int columnIndex)
{
   _ASSERT(columnIndex <= dNumColumns);
   int index = GetTopCellIndex(columnIndex);
   if (dTimestep % 2)
      return &dpCellStateArray1[index];
   else
      return &dpCellStateArray0[index];
}

int8_t* vRegion::GetProximalSegmentPermanenceArray(int columnIndex)
{
   _ASSERT(columnIndex < dNumColumns);
   return &dpProximalSegmentPermanenceArray[dpInputSpace->GetSizeX() * dpInputSpace->GetSizeY() * columnIndex];
}

int8_t* vRegion::GetDistalSegmentPermanenceArray(int cellIndex, int segmentIndex)
{
   _ASSERT((cellIndex < dNumCells) && (segmentIndex < VHTM_MAX_SEGMENTS_PER_CELL));
   int groupIndex = segmentIndex / VHTM_SEGMENTS_PER_GROUP;
   int groupSegmentIndex = segmentIndex % VHTM_SEGMENTS_PER_GROUP;
   vCell* pCell = GetCellObject(cellIndex);
   return &pCell->dpSegmentGroupArray[groupIndex][groupSegmentIndex*dDistalSegmentSize];
}


