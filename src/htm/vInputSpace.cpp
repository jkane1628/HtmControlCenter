
#include "crtdbg.h"
#include "malloc.h"
#include "CJTypes.h"
#include "vInputSpace.h"

#define VHTM_MEMORY_ALIGNMENT_VALUE 32  // TODO: SHOULD BE COMMON WITH VALUE IN vRegion.h

vInputSpace::vInputSpace(int sizeX, int sizeY)
:
dSizeX(sizeX),
dSizeY(sizeY)
{
   dNumCells = sizeX * sizeY;
   dpBuffer = (uint8_t*)_aligned_malloc(sizeX*sizeY, VHTM_MEMORY_ALIGNMENT_VALUE);
}

vInputSpace::~vInputSpace(void)
{
   free(dpBuffer);
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

bool vInputSpace::GetIsActive(int x, int y)
{
   _ASSERT((x >= 0) && (x < dSizeX));
   _ASSERT((y >= 0) && (y < dSizeY));
   return dpBuffer[y*dSizeX + x];
}

bool vInputSpace::GetIsActive(int index)
{
   _ASSERT(index < dNumCells);
   return dpBuffer[index];
}

void vInputSpace::SetIsActive(int x, int y, bool active)
{
   _ASSERT((x >= 0) && (x < dSizeX));
   _ASSERT((y >= 0) && (y < dSizeY));
   dpBuffer[y*dSizeX + x] = active;
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
