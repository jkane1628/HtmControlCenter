
#include "CJTypes.h"
#include "DataSpace.h"
#include "InputSpace.h"

#include "SDR.h"


SDR::SDR(int xSize, int ySize, int active_cells, int active_range, float min_value, float max_value)
:
dxSize(xSize),
dySize(ySize),
dActiveCells(active_cells),
dActiveRange(active_range),
dMinValue(min_value),
dMaxValue(max_value)
{
   dResolution_steps = CalcSDRResolutionSteps();

   if (dMinValue == 0 && dMaxValue == 0)
   {
      dMaxValue = dResolution_steps;
   }
}

SDR::~SDR() {}

void  SDR::GenerateSDR(int* pData, float value)
{
   _ASSERT((value <= dMaxValue) && (value >= dMinValue));

   int total_cells = dySize * dxSize;
   float resolution_min = (dMaxValue - dMinValue) / dResolution_steps;
   int adjusted_value = ((value - dMinValue) / resolution_min);

   // Check to make sure this isn't the max value
   if (value == dMaxValue) adjusted_value--;
   
   int major_bin_index = adjusted_value / dActiveRange;
   int major_bin_remainder = adjusted_value % dActiveRange;
   int major_bin_remainder_remaining = major_bin_remainder;
   int minor_bin_size = dActiveRange / dActiveCells;
   int minor_bin_count = dActiveCells;
   int temp_active_index;
   int temp_active_index_offset;
   int temp_active_index_total;

   for (int minor_bin_index = 0; minor_bin_index < minor_bin_count; minor_bin_index++)
   {
      temp_active_index_offset = (major_bin_remainder + dActiveCells - 1 - minor_bin_index) / dActiveCells;
      temp_active_index_total = (temp_active_index_offset / minor_bin_size) * dActiveRange + (temp_active_index_offset % minor_bin_size);
      temp_active_index = (major_bin_index * dActiveRange) + (minor_bin_index * minor_bin_size) + temp_active_index_total;
      SetIsActive(pData, temp_active_index / dySize, temp_active_index % dySize, 0, true);
   }
}

int SDR::CalcSDRResolutionSteps()
{
   int total_cells = dySize * dxSize;
   int starting_adjusted_value = total_cells - dActiveRange;

   for (int adjusted_value = starting_adjusted_value; adjusted_value < total_cells; adjusted_value++)
   {
      int major_bin_index = adjusted_value / dActiveRange;
      int major_bin_remainder = adjusted_value % dActiveRange;
      int major_bin_remainder_remaining = major_bin_remainder;
      int minor_bin_size = dActiveRange / dActiveCells;
      int minor_bin_count = dActiveCells;
      int temp_active_index;
      int temp_active_index_offset;
      int temp_active_index_total;
      for (int minor_bin_index = 0; minor_bin_index < minor_bin_count; minor_bin_index++)
      {
         temp_active_index_offset = (major_bin_remainder + dActiveCells - 1 - minor_bin_index) / dActiveCells;
         temp_active_index_total = (temp_active_index_offset / minor_bin_size) * dActiveRange + (temp_active_index_offset % minor_bin_size);
         temp_active_index = (major_bin_index * dActiveRange) + (minor_bin_index * minor_bin_size) + temp_active_index_total;
         if (temp_active_index >= total_cells)
            return adjusted_value-1;
      }
   }
   return -1; // ERROR
}


float SDR::GetSDRResolution()
{
   return (dMaxValue - dMinValue) / dResolution_steps;
}

int SDR::GetRepresentableValueCount()
{
   int total_cells = dySize * dxSize;
   return total_cells - dActiveCells + 1;
}

bool SDR::GetOverlapCounts(int* pDataSpace1, int* pDataSpace2, int* pOverlapCount, int* pNotOverlapCount)
{
   int x, y;
   int overlap = 0;
   int notOverlap = 0;

   for (y = 0; y < dySize; y++)
   {
      for (x = 0; x < dxSize; x++)
      {
         if ((pDataSpace1[y*dxSize + x] == 1) && (pDataSpace2[y*dxSize + x] == 1))
            overlap++;
         else if ((pDataSpace1[y*dxSize + x]) != (pDataSpace2[y*dxSize + x]))
         {
            notOverlap++;
            if ((pOverlapCount == NULL) && (pNotOverlapCount == NULL))
               break;  // Cut out early, they are not an exact match
         }
         //else if ((pDataSpace1[y*dxSize + x] == 0) && (pDataSpace2[y*dxSize + x] == 0))
            
      }
   }

   if (pOverlapCount) *pOverlapCount = overlap;
   if (pNotOverlapCount) *pNotOverlapCount = notOverlap;
   return (notOverlap==0);
}


void SDR::GetOverlapAmounts(float* pDataSpace1, int* pDataSpace2, float* pOverlapAmount, float* pNotOverlapAmount)
{
   int x, y;
   float overlap = 0;
   float notOverlap = 0;

   for (y = 0; y < dySize; y++)
   {
      for (x = 0; x < dxSize; x++)
      {
         if (pDataSpace2[y*dxSize + x])
         {
            overlap += pDataSpace1[y*dxSize + x];
         }
         else
         {
            notOverlap += pDataSpace1[y*dxSize + x];
         }
      }
   }
   if (pOverlapAmount) *pOverlapAmount = overlap;
   if (pNotOverlapAmount) *pNotOverlapAmount = notOverlap;
}


int SDR::GetStrongestOverlapCounts(int* pDataSpace, float* pMaxOverlapValue, int maxOutputEntries, float* pStrongestValueArray, int* pStrongestOverlapCountArray, int* pStrongestNotOverlapCountArray)
{
   int numValuesWithAnyOverlap = 0;
   // Zero out output arrays
   int i;
   for (i = 0; i < maxOutputEntries; i++)
   {
      pStrongestValueArray[i] = 0;
      pStrongestOverlapCountArray[i] = 0;
      pStrongestNotOverlapCountArray[i] = dActiveCells;
   }
   *pMaxOverlapValue = 0;

   // Get start and end positions to start scan from    //////////////// TODO: BIG OPTIMIZATION COULD BE MADE HERE.  NEED TO REDO HOW DATA IS LAYED OUT IN DATA BUFFER TO MAKE THIS WORK
/*   int startX,startY,endX,endY;
   int maxIndex = dySize * dxSize - 1;
   if (!GetFirstActivePostion(pDataSpace, &startX, &startY)) return;
   if (!GetLastActivePostion(pDataSpace, &endX, &endY)) return;
   */
   // Change start and end values to scan range start and end
   float start_scan_value = dMinValue; // MAX(GetIndexFromXY(startX, startY) - dActiveCells, 0) * GetSDRResolution();
   float end_scan_value = dMaxValue; // MIN(GetIndexFromXY(endX, endY) + dActiveCells, maxIndex) * GetSDRResolution();
   float current_scan_value = start_scan_value;
   SDR_Int* pScanSdr = new SDR_Int(this);
   //float valueCandidate;
   int   overlapCountCandidate;
   int   notOverlapCountCandidate;
   int   lowestIndex = 0;
   int   maxOverlapCount = 0;


   // Scan overlaps over range and record best values
   while (current_scan_value <= end_scan_value)
   {
      DeactivateAll(pScanSdr->dpData);
      GenerateSDR(pScanSdr->dpData, current_scan_value);
      GetOverlapCounts(pDataSpace, pScanSdr->dpData, &overlapCountCandidate, &notOverlapCountCandidate);

      if ((overlapCountCandidate > pStrongestOverlapCountArray[lowestIndex]) ||
         ((overlapCountCandidate == pStrongestOverlapCountArray[lowestIndex]) && (notOverlapCountCandidate < pStrongestNotOverlapCountArray[lowestIndex])))
      {
         // Replace this entry
         pStrongestValueArray[lowestIndex] = current_scan_value;
         pStrongestOverlapCountArray[lowestIndex] = overlapCountCandidate;
         pStrongestNotOverlapCountArray[lowestIndex] = notOverlapCountCandidate;
      }
      

      // Find the new lowest value index
      for (i = 0; i < maxOutputEntries; i++)
      {
         if ((pStrongestOverlapCountArray[i] < pStrongestOverlapCountArray[lowestIndex]) ||
            ((pStrongestOverlapCountArray[i] == pStrongestOverlapCountArray[lowestIndex]) && (pStrongestNotOverlapCountArray[i] > pStrongestNotOverlapCountArray[lowestIndex])))
         {
            lowestIndex = i;
         }
      }

      // Count any values that have overlap
      if (overlapCountCandidate > 0)
         numValuesWithAnyOverlap++;

      // Record the best match
      if (overlapCountCandidate > maxOverlapCount)
      {
         maxOverlapCount = overlapCountCandidate;
         *pMaxOverlapValue = current_scan_value;
      }

      current_scan_value += GetSDRResolution();
   }
   return numValuesWithAnyOverlap;
}


int SDR::GetStrongestOverlapAmounts(float* pDataSpace, float* pMaxOverlapValue, int maxOutputEntries, float* pStrongestValueArray, float* pStrongestOverlapAmountArray, float* pStrongestNotOverlapAmountArray)
{
   int numValuesWithAnyOverlap = 0;

   // Zero out output arrays
   int i;
   for (i = 0; i < maxOutputEntries; i++)
   {
      pStrongestValueArray[i] = 0;
      pStrongestOverlapAmountArray[i] = 0;
      pStrongestNotOverlapAmountArray[i] = dActiveCells;
   }

   // Change start and end values to scan range start and end
   float start_scan_value = dMinValue; // MAX(GetIndexFromXY(startX, startY) - dActiveCells, 0) * GetSDRResolution();
   float end_scan_value = dMaxValue; // MIN(GetIndexFromXY(endX, endY) + dActiveCells, maxIndex) * GetSDRResolution();
   float current_scan_value = start_scan_value;
   SDR_Int* pScanSdr = new SDR_Int(this);
   float overlapCountCandidate;
   float notOverlapCountCandidate;
   int   lowestIndex = 0;
   int   maxOverlapAmount = 0;

   // Scan overlaps over range and record best values
   while (current_scan_value <= end_scan_value)
   {
      DeactivateAll(pScanSdr->dpData);
      GenerateSDR(pScanSdr->dpData, current_scan_value);
      GetOverlapAmounts(pDataSpace, pScanSdr->dpData, &overlapCountCandidate, &notOverlapCountCandidate);

      if ((overlapCountCandidate > pStrongestOverlapAmountArray[lowestIndex]) ||
         ((overlapCountCandidate == pStrongestOverlapAmountArray[lowestIndex]) && (notOverlapCountCandidate < pStrongestNotOverlapAmountArray[lowestIndex])))
      {
         // Replace this entry
         pStrongestValueArray[lowestIndex] = current_scan_value;
         pStrongestOverlapAmountArray[lowestIndex] = overlapCountCandidate;
         pStrongestNotOverlapAmountArray[lowestIndex] = notOverlapCountCandidate;
      }
      
      // Find the lowest index
      for (i = 0; i < maxOutputEntries; i++)
      {
         if ((pStrongestOverlapAmountArray[i] < pStrongestOverlapAmountArray[lowestIndex]) ||
            ((pStrongestOverlapAmountArray[i] == pStrongestOverlapAmountArray[lowestIndex]) && (pStrongestNotOverlapAmountArray[i] > pStrongestNotOverlapAmountArray[lowestIndex])))
         {
            lowestIndex = i;
         }
      }

      // Count any values that have overlap
      if (overlapCountCandidate > 0)
         numValuesWithAnyOverlap++;
      
      // Record the best match
      if (overlapCountCandidate > maxOverlapAmount)
      {
         maxOverlapAmount = overlapCountCandidate;
         *pMaxOverlapValue = current_scan_value;
      }

      current_scan_value += GetSDRResolution();
   }
   return numValuesWithAnyOverlap;
}


int SDR::GetIndexFromXY(int x, int y)
{
   return (y * dxSize) + x;
}

void SDR::GetXYFromIndex(int index, int* pX, int* pY)
{
   *pX = index / dxSize;
   *pY = index % dxSize;
}

bool SDR::GetIsActive(int* pData, int _x, int _y, int _index)
{
   _ASSERT((_x >= 0) && (_x < dxSize));
   _ASSERT((_y >= 0) && (_y < dySize));
   _ASSERT((_index >= 0) && (_index < 1));
   return (pData[(_y * dxSize) + (_x * 1) + _index] != 0);
}


void  SDR::SetIsActive(int* pData, int _x, int _y, int _index, bool _active)
{
   _ASSERT((_x >= 0) && (_x < dxSize));
   _ASSERT((_y >= 0) && (_y < dySize));
   _ASSERT((_index >= 0) && (_index < 1));
   pData[(_y * dxSize) + (_x * 1) + _index] = (_active ? 1 : 0);
}


void  SDR::DeactivateAll(int* pData)
{
   memset(pData, 0, dySize * dxSize * sizeof(int));
}

int SDR::GetValue(int* pData, int _x, int _y, int _index)
{
   _ASSERT((_x >= 0) && (_x < dxSize));
   _ASSERT((_y >= 0) && (_y < dySize));
   _ASSERT((_index >= 0) && (_index < 1));
   return pData[(_y * dxSize) + (_x * 1) + _index];
}
    
float SDR::GetValue(float* pData, int _x, int _y, int _index)
{
   _ASSERT((_x >= 0) && (_x < dxSize));
   _ASSERT((_y >= 0) && (_y < dySize));
   _ASSERT((_index >= 0) && (_index < 1));
   return pData[(_y * dxSize) + (_x * 1) + _index];
}


void SDR::SetValue(float* pData, int _x, int _y, int _index, bool _value)
{
   _ASSERT((_x >= 0) && (_x < dySize));
   _ASSERT((_y >= 0) && (_y < dySize));
   _ASSERT((_index >= 0) && (_index < 1));
   pData[(_y * dxSize) + (_x * 1) + _index] = _value;
}


void SDR::SetValue(float* pData, int _x, int _y, int _index, float _value)
{
   _ASSERT((_x >= 0) && (_x < dySize));
   _ASSERT((_y >= 0) && (_y < dySize));
   _ASSERT((_index >= 0) && (_index < 1));
   pData[(_y * dxSize) + (_x * 1) + _index] = _value;
}


void SDR::IncrementValue(float* pData, int _x, int _y, int _index, float _value)
{
   _ASSERT((_x >= 0) && (_x < dySize));
   _ASSERT((_y >= 0) && (_y < dySize));
   _ASSERT((_index >= 0) && (_index < 1));
   pData[(_y * dxSize) + (_x * 1) + _index] += _value;
}


void SDR::ZeroAllValues(float* pData)
{
   memset(pData, 0, dySize * dxSize * sizeof(float));
}

bool SDR::GetFirstActiveIndex(int* pData, int* outputIndex)
{
 /*  for (int i = 0; i < (dxSize*dySize); i++)
   {
      if (pData[i])
      {
         *outputIndex = i;
         return true;
      }
   }*/
   return false;
}

bool SDR::GetLastActiveIndex(int* pData, int* outputIndex)
{
 /*  for (int i = (dxSize*dySize)-1; i != 0; i--)
   {
      if (pData[i])
      {
         *outputIndex = i;
         return true;
      }
   }*/
   return false;
}


bool SDR::GetFirstActivePostion(int* pData, int* pX, int* pY)
{
   int x, y;
   for (x = 0; x < dxSize; x++)
   {
      for (y = 0; y < dySize; y++)
      {
         if (GetValue(pData, x, y, 0) > 0)
         {
            *pX = x;
            *pY = y;
            return true;
         }
      }
   }
   return false;
}

bool SDR::GetLastActivePostion(int* pData, int* pX, int* pY)
{
   int x, y;
   for (x = dxSize-1; x >= 0; x--)
   {
      for (y = dySize - 1; y >= 0; y--)
      {
         if (GetValue(pData,x,y,0) > 0)
         {
            *pX = x;
            *pY = y;
            return true;
         }
      }
   }
   return false;
}