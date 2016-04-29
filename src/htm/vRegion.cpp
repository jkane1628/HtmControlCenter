
#include <stdint.h>
#include <malloc.h>

#include "NetworkManager.h"
#include "vInputSpace.h"
#include "vRegion.h"

vDistalSegment::vDistalSegment(int8_t* permanenceArray, int8_t* permanenceUpdateArray)
:
dpPermanenceArray(permanenceArray),
dpPermanenceUpdate(permanenceUpdateArray)
{}


vCell::vCell()
:
dNumSegments(0)
{
   for (int i = 0; i < VHTM_MAX_NUM_SEGMENT_GROUPS; i++)
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
dpCellStateArray(NULL),
dpDistalSegmentPermanenceArray(NULL),
dpColumnArray(NULL),
dpCellArray(NULL),
dpCurrentTimestepColumnStateArray(NULL),
dNumColumnStateTimestepsInHistory(VHTM_NUM_COLUMN_STATE_HISTORY_ENTRIES_DEFAULT),
dpCurrentTimestepCellStateArray(NULL),
dNumCellStateTimestepsInHistory(VHTM_NUM_CELL_STATE_HISTORY_ENTRIES_DEFAULT),
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




bool vRegion::CreateRegion()
{
   dDistalSegmentSize = dNumCells / VHTM_RECEPTIVE_RANGE_DIVISOR;

   // Memories
   dpColumnStateArray = (uint8_t*)_aligned_malloc(dNumColumns * dNumColumnStateTimestepsInHistory, VHTM_MEMORY_ALIGNMENT_VALUE);
   if (dpColumnStateArray == NULL) return false;
   memset(dpColumnStateArray, 0, dNumColumns);
   dpCurrentTimestepColumnStateArray = dpColumnStateArray;

   dpCellStateArray = (uint8_t*)_aligned_malloc(dNumCells * dNumCellStateTimestepsInHistory, VHTM_MEMORY_ALIGNMENT_VALUE);
   if (dpCellStateArray == NULL) return false;
   memset(dpCellStateArray, 0, dNumCells);
   dpCurrentTimestepCellStateArray = dpCellStateArray;
   
   int dpProximalSegmentPermanenceArraySize = dpInputSpace->GetSizeX() * dpInputSpace->GetSizeY() * dNumColumns;
   dpProximalSegmentPermanenceArray = (int8_t*)_aligned_malloc(dpProximalSegmentPermanenceArraySize, VHTM_MEMORY_ALIGNMENT_VALUE);
   if (dpProximalSegmentPermanenceArray == NULL) return false;
   memset(dpProximalSegmentPermanenceArray, VHTM_CONNECTED_PERMANENCE, dpProximalSegmentPermanenceArraySize);

   int dpDistalSegmentPermanenceArraySize = dNumCells * VHTM_TARGET_SEGMENTS_PER_CELL * 2 * dNumCells / VHTM_RECEPTIVE_RANGE_DIVISOR;
   dpDistalSegmentPermanenceArray = (int8_t*)_aligned_malloc(dpDistalSegmentPermanenceArraySize, VHTM_MEMORY_ALIGNMENT_VALUE);
   if (dpDistalSegmentPermanenceArray == NULL) return false;
   memset(dpDistalSegmentPermanenceArray, VHTM_CONNECTED_PERMANENCE, dpDistalSegmentPermanenceArraySize);

   dpColumnBoostArray = (int*)_aligned_malloc(dNumColumns * sizeof(int), VHTM_MEMORY_ALIGNMENT_VALUE);
   if (dpColumnBoostArray == NULL) return false;
   memset(dpColumnBoostArray, 1, dNumColumns);

   dpCellArray = new vCell[dNumCells];
   dpColumnArray = new vColumn[dNumColumns];

   dDistalSegmentGroupsTotal = dNumCells * VHTM_MAX_NUM_SEGMENT_GROUPS;
   dDistalSegmentGroupsRemaining = dDistalSegmentGroupsTotal;
   dDistalSegmentGroupsSize = dDistalSegmentSize * VHTM_TARGET_SEGMENTS_PER_CELL / VHTM_MAX_NUM_SEGMENT_GROUPS;
   dpDistalSegmentGroupsNext = dpDistalSegmentPermanenceArray;

   int dActiveColumnIndexArraySize = dNumColumns * sizeof(int);  // Allocate for all the cols to be active, this is way more than what's needed, but makes it simple
   dpActiveColumnIndexArray = (int*)_aligned_malloc(dActiveColumnIndexArraySize, VHTM_MEMORY_ALIGNMENT_VALUE);

   int dColumnActiveDutyCycleTotalsSize = dNumColumns * sizeof(int);
   dpColumnActiveDutyCycleTotals = (int*)_aligned_malloc(dColumnActiveDutyCycleTotalsSize, VHTM_MEMORY_ALIGNMENT_VALUE); 

   int dColumnOverlapDutyCycleTotalsSize = dNumColumns * sizeof(int);
   dpColumnOverlapDutyCycleTotals = (int*)_aligned_malloc(dColumnOverlapDutyCycleTotalsSize, VHTM_MEMORY_ALIGNMENT_VALUE);

   return true;
}

void vRegion::FreeRegion()
{
   if (dpCellStateArray) free(dpCellStateArray);
   dpCurrentTimestepColumnStateArray = NULL;
   if (dpProximalSegmentPermanenceArray) free(dpProximalSegmentPermanenceArray);
   if (dpDistalSegmentPermanenceArray) free(dpDistalSegmentPermanenceArray);
   if (dpColumnBoostArray) free(dpColumnBoostArray);
   if (dpCellArray) free(dpCellArray);
   if (dpColumnArray) free(dpColumnArray);
   if (dpActiveColumnIndexArray) free(dpActiveColumnIndexArray);
   if (dpColumnActiveDutyCycleTotals) free(dpColumnActiveDutyCycleTotals);
   if (dpColumnOverlapDutyCycleTotals) free(dpColumnOverlapDutyCycleTotals);

}

void vRegion::Step()
{
   bool allowSpacialLearning = true;
   bool allowTemporalLearning = true;

   IncrementCurrentColumnStateArrayPtr();
   IncrementCurrentCellStateArrayPtr();

   PerformSpatialPooling(allowSpacialLearning);
   PerformTemporalPooling(allowTemporalLearning);
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

void vRegion::IncrementCurrentColumnStateArrayPtr()
{
   dpCurrentTimestepColumnStateArray += dNumColumns;
   if (dpCurrentTimestepColumnStateArray > (dpCurrentTimestepColumnStateArray + (dNumCells*dNumColumnStateTimestepsInHistory)))
   {
      dpCurrentTimestepColumnStateArray = dpColumnStateArray;
   }
}

uint8_t* vRegion::GetCurrentColumnStateArrayPtr(int columnIndex)
{
   _ASSERT(columnIndex <= dNumColumns);
   return &dpCurrentTimestepColumnStateArray[columnIndex];
}

uint8_t* vRegion::GetOldestColumnStateArrayPtr(int columnIndex)
{
   _ASSERT(columnIndex <= dNumColumns);
   uint8_t* pOldestTimestepColumnStateArray = dpCurrentTimestepColumnStateArray + dNumColumns;
   if (pOldestTimestepColumnStateArray > (dpColumnStateArray + dNumColumns * dNumColumnStateTimestepsInHistory))
   {
      pOldestTimestepColumnStateArray = dpColumnStateArray;
   }
   return &pOldestTimestepColumnStateArray[columnIndex];
}

uint8_t* vRegion::GetHistoricalColumnStateArrayPtr(int columnIndex, int stepsBack)
{
   _ASSERT(columnIndex <= dNumColumns);
   _ASSERT(stepsBack <= dNumColumnStateTimestepsInHistory);

   uint8_t* pHistoricalTimestepColumnStateArray = dpCurrentTimestepColumnStateArray - (dNumColumns * stepsBack);
   if (pHistoricalTimestepColumnStateArray < dpColumnStateArray)
   {
      pHistoricalTimestepColumnStateArray = (dpColumnStateArray + dNumColumns * dNumColumnStateTimestepsInHistory) - (dpColumnStateArray - pHistoricalTimestepColumnStateArray);
   }
   return &pHistoricalTimestepColumnStateArray[columnIndex];
}


void vRegion::IncrementCurrentCellStateArrayPtr()
{
   dpCurrentTimestepCellStateArray += dNumCells;
   if (dpCurrentTimestepCellStateArray > (dpCurrentTimestepCellStateArray + (dNumCells*dNumCellStateTimestepsInHistory)))
   {
      dpCurrentTimestepCellStateArray = dpCellStateArray;
   }
}


uint8_t* vRegion::GetCurrentCellStateArrayPtr(int cellIndex)
{
   _ASSERT(cellIndex <= dNumCells);
   uint8_t* pCellStateArray = dpCurrentTimestepCellStateArray;
   return &pCellStateArray[cellIndex];
}

uint8_t* vRegion::GetOldestCellStateArrayPtr(int cellIndex)
{
   _ASSERT(cellIndex <= dNumCells);
   uint8_t* pOldestTimestepCellStateArray = dpCurrentTimestepCellStateArray + dNumCells;
   if (pOldestTimestepCellStateArray > (dpCellStateArray + dNumCells * dNumCellStateTimestepsInHistory))
   {
      pOldestTimestepCellStateArray = dpCellStateArray;
   }
   return &pOldestTimestepCellStateArray[cellIndex];
}


uint8_t* vRegion::GetHistoricalCellStateArrayPtr(int cellIndex, int stepsBack)
{
   _ASSERT(cellIndex <= dNumCells);
   _ASSERT(stepsBack <= dNumCellStateTimestepsInHistory);

   uint8_t* pHistoricalTimestepCellStateArray = dpCurrentTimestepCellStateArray - (dNumCells * stepsBack);
   if (pHistoricalTimestepCellStateArray < dpCellStateArray)
   {
      pHistoricalTimestepCellStateArray = (dpCellStateArray + dNumCells * dNumCellStateTimestepsInHistory) - (dpCellStateArray - pHistoricalTimestepCellStateArray);
   }
   return &pHistoricalTimestepCellStateArray[cellIndex];
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


