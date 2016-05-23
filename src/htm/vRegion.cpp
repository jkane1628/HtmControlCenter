
#include <stdint.h>
#include <malloc.h>
#include "CJTrace.h"

#include "NetworkManager.h"
#include "vInputSpace.h"
#include "vRegion.h"

vDistalSegment::vDistalSegment(int8_t* permanenceArray, int8_t* permanenceUpdateArray)
:
dpPermanenceArray(permanenceArray),
dpPermanenceUpdate(permanenceUpdateArray)
{}


vRegion::vRegion(QString id)
:
vDataSpace(id),
dRegionSize(eRegionSize_INVALID),
dpInputSpace(NULL),
dpColumnStateArray(NULL),
dpCellStateArray(NULL),
dpProximalSegmentPermanenceArray(NULL),
dpDistalSegmentPermanenceArray(NULL),
dpDistalSegmentPermanenceUpdateArray(NULL),
dpColumnBoostArray(NULL),
dpColumnActivationTotalArray(NULL),
dpActiveColumnIndexArray(NULL),
dpColumnActiveDutyCycleTotals(NULL),
dpColumnOverlapDutyCycleTotals(NULL),
dpCurrentTimestepColumnStateArray(NULL),
dpCurrentTimestepCellStateArray(NULL),
dNumColumnStateTimestepsInHistory(VHTM_NUM_COLUMN_STATE_HISTORY_ENTRIES_DEFAULT),
dNumCellStateTimestepsInHistory(VHTM_NUM_CELL_STATE_HISTORY_ENTRIES_DEFAULT),
dDistalSegmentGroupsTotal(0),
dDistalSegmentGroupsRemaining(0),
dDistalSegmentGroupsSize(0),
dpDistalSegmentGroupsNext(0)
{
}

vRegion::~vRegion()
{
   FreeRegion();
}

bool vRegion::AttachInputDataSpace(vDataSpace* pInputSpace)
{
   // TODO: Only supporting a single input space for now...no region connection.
   //      In the future, a derived class from vInputspace can be created to shim a connection from a region output to a vInputSpace object... something like vInputSpace* createCompatibleInputSpace(Region*)
   dpInputSpace = (vInputSpace*)pInputSpace;
   return true;
}


bool vRegion::CreateRegion()
{
   bool allocError = false;

   switch (dRegionSize)
   {
   case eRegionSize_4x4x4:
      SetSize(vPosition(4, 4, 4));
      break;
   case eRegionSize_16x16x8:
      SetSize(vPosition(16, 16, 8));
      break;
   case eRegionSize_32x32x4:
      SetSize(vPosition(32, 32, 4));
      break;
   default:
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Cannot create region, invalid regionSize (dRegionSize=%d)", dRegionSize);
      return false;
   }


   dDistalSegmentSize = dNumCells / VHTM_DISTAL_SEGMENT_RECEPTIVE_RANGE_DIVISOR;

   CJTRACE(TRACE_ERROR_LEVEL, "Creating memory for vRegion (id=%s)", GetID());

   if (dpInputSpace == NULL)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Cannot create a region without the input spaces attached");
      return false;
   }

   // Memories
   int dpColumnStateArraySize = dNumColumns * dNumColumnStateTimestepsInHistory;
   dpColumnStateArray = (uint8_t*)_aligned_malloc(dpColumnStateArraySize, VHTM_MEMORY_ALIGNMENT_VALUE);
   if (dpColumnStateArray == NULL) allocError = true;
   memset(dpColumnStateArray, 0, dpColumnStateArraySize);
   dpCurrentTimestepColumnStateArray = dpColumnStateArray;
   CJTRACE(TRACE_ERROR_LEVEL, "  dpColumnStateArray = 0x%x (%d B)", dpColumnStateArray, dpColumnStateArraySize);

   int dpCellStateArraySize = dNumCells * dNumCellStateTimestepsInHistory;
   dpCellStateArray = (uint8_t*)_aligned_malloc(dpCellStateArraySize, VHTM_MEMORY_ALIGNMENT_VALUE);
   if (dpCellStateArray == NULL) allocError = true;
   memset(dpCellStateArray, 0, dpCellStateArraySize);
   dpCurrentTimestepCellStateArray = dpCellStateArray;
   CJTRACE(TRACE_ERROR_LEVEL, "  dpCellStateArray = 0x%x (%d B)", dpColumnStateArray, dpCellStateArraySize);

   int dpProximalSegmentPermanenceArraySize = dpInputSpace->GetSizeX() * dpInputSpace->GetSizeY() * dNumColumns;
   dpProximalSegmentPermanenceArray = (int8_t*)_aligned_malloc(dpProximalSegmentPermanenceArraySize, VHTM_MEMORY_ALIGNMENT_VALUE);
   if (dpProximalSegmentPermanenceArray == NULL) allocError = true;
   memset(dpProximalSegmentPermanenceArray, VHTM_CONNECTED_PERMANENCE, dpProximalSegmentPermanenceArraySize);
   CJTRACE(TRACE_ERROR_LEVEL, "  dpProximalSegmentPermanenceArray = 0x%x (%d B)", dpProximalSegmentPermanenceArray, dpProximalSegmentPermanenceArraySize);

   int dpDistalSegmentPermanenceArraySize = dNumCells * VHTM_NOMINAL_DISTAL_SEGMENTS_PER_CELL * dDistalSegmentSize;
   dpDistalSegmentPermanenceArray = (int8_t*)_aligned_malloc(dpDistalSegmentPermanenceArraySize, VHTM_MEMORY_ALIGNMENT_VALUE);
   if (dpDistalSegmentPermanenceArray == NULL) allocError = true;
   memset(dpDistalSegmentPermanenceArray, VHTM_CONNECTED_PERMANENCE, dpDistalSegmentPermanenceArraySize);
   CJTRACE(TRACE_ERROR_LEVEL, "  dpDistalSegmentPermanenceArray = 0x%x (%d B)", dpDistalSegmentPermanenceArray, dpDistalSegmentPermanenceArraySize);

   int dpDistalSegmentPermanenceUpdateArraySize = dNumCells * VHTM_NOMINAL_DISTAL_SEGMENTS_PER_CELL * dDistalSegmentSize;
   dpDistalSegmentPermanenceUpdateArray = (int8_t*)_aligned_malloc(dpDistalSegmentPermanenceUpdateArraySize, VHTM_MEMORY_ALIGNMENT_VALUE);
   if (dpDistalSegmentPermanenceUpdateArray == NULL) allocError = true;
   memset(dpDistalSegmentPermanenceUpdateArray, VHTM_CONNECTED_PERMANENCE, dpDistalSegmentPermanenceUpdateArraySize);
   CJTRACE(TRACE_ERROR_LEVEL, "  dpDistalSegmentPermanenceUpdateArray = 0x%x (%d B)", dpDistalSegmentPermanenceUpdateArray, dpDistalSegmentPermanenceUpdateArraySize);

   uint32_t dpColumnBoostArraySize = dNumColumns * sizeof(int);
   dpColumnBoostArray = (int*)_aligned_malloc(dpColumnBoostArraySize, VHTM_MEMORY_ALIGNMENT_VALUE);
   if (dpColumnBoostArray == NULL)allocError = true;
   memset(dpColumnBoostArray, 1, dpColumnBoostArraySize);
   CJTRACE(TRACE_ERROR_LEVEL, "  dpColumnBoostArray = 0x%x (%d B)", dpColumnBoostArray, dpColumnBoostArraySize);

   uint32_t dpColumnActivationTotalArraySize = dNumColumns * sizeof(int);
   dpColumnActivationTotalArray = (int*)_aligned_malloc(dpColumnActivationTotalArraySize, VHTM_MEMORY_ALIGNMENT_VALUE);
   if (dpColumnActivationTotalArray == NULL) allocError = true;
   memset(dpColumnActivationTotalArray, 0, dpColumnActivationTotalArraySize);
   CJTRACE(TRACE_ERROR_LEVEL, "  dpColumnActivationTotalArray = 0x%x (%d B)", dpColumnActivationTotalArray, dpColumnActivationTotalArraySize);
   
   dDistalSegmentGroupsTotal = dNumCells * VHTM_MAX_DISTAL_SEGMENTS_PER_CELL;
   dDistalSegmentGroupsRemaining = dDistalSegmentGroupsTotal;
   dDistalSegmentGroupsSize = dDistalSegmentSize * VHTM_SEGMENTS_PER_DISTAL_SEGMENT_GROUP;
   dpDistalSegmentGroupsNext = dpDistalSegmentPermanenceArray;

   uint32_t dActiveColumnIndexArraySize = dNumColumns * sizeof(int);  // Allocate for all the cols to be active, this is way more than what's needed, but makes it simple
   dpActiveColumnIndexArray = (int*)_aligned_malloc(dActiveColumnIndexArraySize, VHTM_MEMORY_ALIGNMENT_VALUE);
   if (dpActiveColumnIndexArray == NULL) allocError = true;
   memset(dpActiveColumnIndexArray, 0, dActiveColumnIndexArraySize);
   CJTRACE(TRACE_ERROR_LEVEL, "  dpActiveColumnIndexArray = 0x%x (%d B)", dpActiveColumnIndexArray, dActiveColumnIndexArraySize);

   uint32_t dColumnActiveDutyCycleTotalsSize = dNumColumns * sizeof(int);
   dpColumnActiveDutyCycleTotals = (int*)_aligned_malloc(dColumnActiveDutyCycleTotalsSize, VHTM_MEMORY_ALIGNMENT_VALUE); 
   if (dpColumnActiveDutyCycleTotals == NULL) allocError = true;
   memset(dpColumnActiveDutyCycleTotals, 0, dColumnActiveDutyCycleTotalsSize);
   CJTRACE(TRACE_ERROR_LEVEL, "  dpColumnActiveDutyCycleTotals = 0x%x (%d B)", dpColumnActiveDutyCycleTotals, dColumnActiveDutyCycleTotalsSize);

   uint32_t dColumnOverlapDutyCycleTotalsSize = dNumColumns * sizeof(int);
   dpColumnOverlapDutyCycleTotals = (int*)_aligned_malloc(dColumnOverlapDutyCycleTotalsSize, VHTM_MEMORY_ALIGNMENT_VALUE);
   if (dpColumnOverlapDutyCycleTotals == NULL) allocError = true;
   memset(dpColumnOverlapDutyCycleTotals, 0, dColumnOverlapDutyCycleTotalsSize);
   CJTRACE(TRACE_ERROR_LEVEL, "  dpColumnOverlapDutyCycleTotals = 0x%x (%d B)", dpColumnOverlapDutyCycleTotals, dColumnOverlapDutyCycleTotalsSize);

   if (allocError)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Memory allocation failure.");
      FreeRegion();
      return false;
   }

   uint32_t totalMemorySize = dpColumnStateArraySize + dpCellStateArraySize +
                         dpProximalSegmentPermanenceArraySize + dpDistalSegmentPermanenceArraySize +  
                         dpDistalSegmentPermanenceUpdateArraySize + dpColumnBoostArraySize +
                         dpColumnActivationTotalArraySize + dActiveColumnIndexArraySize +
                         dColumnActiveDutyCycleTotalsSize + dColumnOverlapDutyCycleTotalsSize;
   CJTRACE(TRACE_ERROR_LEVEL, "Memory alloc complete, %d bytes allocated (%d MB)", totalMemorySize, totalMemorySize / 1024 / 1024);
   return true;
}

void vRegion::FreeRegion()
{
   if (dpColumnStateArray) _aligned_free(dpColumnStateArray); dpColumnStateArray = NULL;
   if (dpCellStateArray) _aligned_free(dpCellStateArray); dpCellStateArray = NULL;
   if (dpProximalSegmentPermanenceArray) _aligned_free(dpProximalSegmentPermanenceArray); dpProximalSegmentPermanenceArray = NULL;
   if (dpDistalSegmentPermanenceArray) _aligned_free(dpDistalSegmentPermanenceArray); dpDistalSegmentPermanenceArray = NULL;
   if (dpDistalSegmentPermanenceUpdateArray) _aligned_free(dpDistalSegmentPermanenceUpdateArray); dpDistalSegmentPermanenceUpdateArray = NULL;
   if (dpColumnBoostArray) _aligned_free(dpColumnBoostArray); dpColumnBoostArray = NULL;
   if (dpColumnActivationTotalArray) _aligned_free(dpColumnActivationTotalArray); dpColumnActivationTotalArray = NULL;
   if (dpActiveColumnIndexArray) _aligned_free(dpActiveColumnIndexArray); dpActiveColumnIndexArray = NULL;
   if (dpColumnActiveDutyCycleTotals) _aligned_free(dpColumnActiveDutyCycleTotals); dpColumnActiveDutyCycleTotals = NULL;
   if (dpColumnOverlapDutyCycleTotals) _aligned_free(dpColumnOverlapDutyCycleTotals); dpColumnOverlapDutyCycleTotals = NULL;

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

int vRegion::GetProximalSegmentReceptiveRange() 
{ 
   return dpInputSpace->GetNumCells(); 
}

int vRegion::GetTopCellIndex(int columnIndex) 
{
   return columnIndex*GetSizeZ();
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
   _ASSERT((cellIndex < dNumCells) && (segmentIndex < VHTM_MAX_DISTAL_SEGMENTS_PER_CELL));
   int groupIndex = segmentIndex / VHTM_SEGMENTS_PER_DISTAL_SEGMENT_GROUP;
   int groupSegmentIndex = segmentIndex % VHTM_SEGMENTS_PER_DISTAL_SEGMENT_GROUP;
   CellSegment* pCellSegment = &dpCellSegmentGroups[cellIndex];
   int pSegGroupIndex = pCellSegment->pSegmentGroupIndexArray[groupIndex];
   return &dpDistalSegmentPermanenceArray[pSegGroupIndex*dDistalSegmentSize];
}

int8_t* vRegion::GetDistalSegmentPermanenceUpdateArray(int cellIndex, int segmentIndex)
{
   _ASSERT((cellIndex < dNumCells) && (segmentIndex < VHTM_MAX_DISTAL_SEGMENTS_PER_CELL));
   int groupIndex = segmentIndex / VHTM_SEGMENTS_PER_DISTAL_SEGMENT_GROUP;
   int groupSegmentIndex = segmentIndex % VHTM_SEGMENTS_PER_DISTAL_SEGMENT_GROUP;
   CellSegment* pCellSegment = &dpCellSegmentGroups[cellIndex];
   int pSegGroupIndex = pCellSegment->pSegmentGroupIndexArray[groupIndex];
   return &dpDistalSegmentPermanenceUpdateArray[pSegGroupIndex*dDistalSegmentSize];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// vCell Class
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vCell::vCell(vRegion* pRegion, int x, int y, int z)
   :
   dpRegion(pRegion)
{
   dIndex = pRegion->GetIndex(x, y, z);
}
vCell::vCell(vRegion* pRegion, int index)
   :
   dpRegion(pRegion),
   dIndex(index)
{
   _ASSERT(dIndex < dpRegion->GetNumCells());
}

int  vCell::GetCellIndex()    { return dIndex; }
vPosition vCell::GetCellPosition() { return dpRegion->GetPosition(dIndex); }
bool vCell::IsCellActive()    { return dpRegion->dpCellStateArray[dIndex] && eCellState_Active; }
bool vCell::IsCellPredicted() { return dpRegion->dpCellStateArray[dIndex] && eCellState_Predicting; }
int  vCell::GetNumPredictionSteps() { return -1; } // TODO: IMPLEMENT THIS...
bool vCell::IsCellLearning()  { return dpRegion->dpCellStateArray[dIndex] && eCellState_Learning; }

int vCell::GetNumDistalSegments() { return dpRegion->dpCellSegmentGroups[dIndex].numSegments;}

int8_t* vCell::GetDistalSegmentPermanenceArray(int segmentIndex)
{
   _ASSERT(segmentIndex < GetNumDistalSegments());
   return dpRegion->GetDistalSegmentPermanenceArray(dIndex, segmentIndex);
}

int8_t* vCell::GetDistalSegmentPermanenceUpdateArray(int segmentIndex)
{
   _ASSERT(segmentIndex < GetNumDistalSegments());
   return dpRegion->GetDistalSegmentPermanenceUpdateArray(dIndex, segmentIndex);
}

int vCell::GetGetDistalSegmentReceptiveRange()
{
   return dpRegion->GetNumCells() * VHTM_DISTAL_SEGMENT_RECEPTIVE_RANGE_DIVISOR;
}

int vCell::GetDistalSynaspeRemoteConnectionIndex(int synapseIndex)
{
   int connectedCellIndex = dIndex + synapseIndex;
   if (connectedCellIndex >= dpRegion->GetNumCells())
   {
      connectedCellIndex -= dpRegion->GetNumCells();
   }
   return connectedCellIndex;
}

vPosition vCell::GetDistalSynaspeRemoteConnectionPosition(int synapseIndex)
{
   return dpRegion->GetPosition(GetDistalSynaspeRemoteConnectionIndex(synapseIndex));
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// vColumn Class
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


vColumn::vColumn(vRegion* pRegion, int x, int y)
{
   int index = x * pRegion->GetSizeY() + y;
   vColumn(pRegion, index);
}

vColumn::vColumn(vRegion* pRegion, int index)
:
dIndex(index),
dpRegion(pRegion)
{
   _ASSERT(dIndex < dpRegion->GetNumColumns());
}


int   vColumn::GetColumnIndex() {return dIndex;}
float vColumn::GetBoostValue() { return (float)dpRegion->dpColumnBoostArray[dIndex];}
bool  vColumn::GetIsActive() { return dpRegion->dpColumnStateArray[dIndex] && eColumnState_Active; }
bool  vColumn::GetIsOverlapped() { return dpRegion->dpColumnStateArray[dIndex] && eColumnState_Overlapped; }

bool  vColumn::GetIsAnyCellPredicting(int numSteps)
{
   for (int i = 0; i < dpRegion->GetSizeZ(); i++)
   {
      vCell cell = GetCellByIndex(i);
      if (cell.IsCellPredicted())
         return true;
   }
   return false;
}

vCell vColumn::GetCellByIndex(int cellIndex)
{
   int regionCellIndex = dpRegion->GetSizeZ() * dIndex + cellIndex;
   return vCell(dpRegion, regionCellIndex);
}

int8_t* vColumn::GetProximalSegmentPermanenceArray()
{
   return dpRegion->GetProximalSegmentPermanenceArray(dIndex);
}

int vColumn::GetProximalSegmentReceptiveRange()
{
   return dpRegion->dpInputSpace->GetNumCells();
}

int vColumn::GetProximalSynapseConnectionInputIndex(int synaspeIndex)
{
   return synaspeIndex;
}

vPosition vColumn::GetProximalSynapseConnectionInputPosition(int synaspeIndex)
{
   return dpRegion->dpInputSpace->GetPosition(synaspeIndex);
}
