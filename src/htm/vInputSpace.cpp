
#include "crtdbg.h"
#include "malloc.h"
#include "CJTypes.h"
#include "CJTrace.h"
#include "Pattern.h"
#include "vInputSpace.h"

#define VHTM_MEMORY_ALIGNMENT_VALUE 32  // TODO: SHOULD BE COMMON WITH VALUE IN vRegion.h

vInputSpace::vInputSpace(QString id, int sizeX, int sizeY, int sizeZ)
:
vDataSpace(id),
dpActivePattern(NULL),
dpBuffer(NULL)
{
   dMaxPosition.x = sizeX;
   dMaxPosition.y = sizeY;
   dMaxPosition.z = sizeZ;
}

vInputSpace::~vInputSpace(void)
{
   if(dpBuffer) _aligned_free(dpBuffer);
   if(dpActivePattern) delete dpActivePattern;
}

uint8_t* vInputSpace::CreateBuffer()
{
   int memSize = dMaxPosition.x * dMaxPosition.y * dMaxPosition.z;
   if (memSize > 0)
   {
      dpBuffer = (uint8_t*)_aligned_malloc(memSize, VHTM_MEMORY_ALIGNMENT_VALUE);
      if (dpBuffer == NULL)
      {
         CJTRACE(TRACE_ERROR_LEVEL, "Failed to alloc input buffer memory.  %d bytes, id=%s", memSize, this->GetID().toStdString().c_str());
         return NULL;
      }      
   }
   memset(dpBuffer, 0, memSize);
   CJTRACE(TRACE_HIGH_LEVEL, "Input buffer created.  %d bytes, id=%s", memSize, this->GetID().toStdString().c_str());
   return dpBuffer;
}

void vInputSpace::Step(int time)
{
   if(dpActivePattern)
      dpActivePattern->ApplyPattern(time);
}

void vInputSpace::SetupEncoder(int active_cells, int active_range, float min_value, float max_value)
{
   dActiveCells = active_cells;
   dActiveRange = active_range;
   dMinValue = min_value;
   dMaxValue = max_value;

   dResolutionSteps = CalcEncoderResolutionSteps();
   dResolutionMin = (dMaxValue - dMinValue) / dResolutionSteps;   // TODO: I think if this was (dMaxValue - dMinValue) / dResolution_steps;, then the line if (value == dMaxValue) adjusted_value--; could be removed in encode()
}

int vInputSpace::CalcEncoderResolutionSteps()
{
   int starting_adjusted_value = dNumCells - dActiveRange;

   for (int adjusted_value = starting_adjusted_value; adjusted_value < dNumCells; adjusted_value++)
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
         if (temp_active_index >= dNumCells)
            return adjusted_value - 1;
      }
   }
   return -1; // ERROR
}


float vInputSpace::EncodeSDR(float value)
{
   _ASSERT((value >= dMinValue) && (value <= dMaxValue));

   dCurrentInputValue = value;

   int adjusted_value = ((value - dMinValue) / dResolutionMin);
   
   // Check to make sure this isn't the max value TODO: COULD REMOVE THIS IF RESOLUTION WAS LOWER
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
      SetIsActive(temp_active_index, true);
   }
   return adjusted_value * dResolutionMin;
}

bool vInputSpace::GetIsActive(int x, int y, int z)
{
   _ASSERT((x >= 0) && (x < GetSizeX()));
   _ASSERT((y >= 0) && (y < GetSizeY()));
   _ASSERT((z >= 0) && (z < GetSizeZ()));
   return dpBuffer[GetIndex(x,y,z)];
}

bool vInputSpace::GetIsActive(int index)
{
   _ASSERT(index < dNumCells);
   return dpBuffer[index];
}

void vInputSpace::SetIsActive(int x, int y, int z, bool active)
{
   _ASSERT((x >= 0) && (x < GetSizeX()));
   _ASSERT((y >= 0) && (y < GetSizeY()));
   _ASSERT((z >= 0) && (z < GetSizeZ()));
   dpBuffer[GetIndex(x, y, z)] = active;
}

void vInputSpace::SetIsActive(int index, bool active)
{
   _ASSERT(index < dNumCells);
   dpBuffer[index] = active;
}

void vInputSpace::DeactivateAll()
{
   memset(dpBuffer, 0, dNumCells);
}
