
#include "CJTypes.h"
#include "CJTrace.h"

#include "SDR.h"
#include "vRegion.h"
#include "vInputSpace.h"


#include "vClassifier.h"


vClassifier::vClassifier(QString &_id, QString _regionID, QString _inputspaceID)
: vDataSpace(_id), dRegionID(_regionID), dInputspaceID(_inputspaceID),
dpProjectionSdr(NULL),
dPreviousMaxOverlapValue(-1000000000)
{
   
}

vClassifier::~vClassifier(void)
{
   if (dpProjectionSdr != NULL)
      delete dpProjectionSdr;
}


void vClassifier::Classify() 
{
   // Create an SDR from the projection of the prediction cells projected through the proximal synaspes
   CreateProjectionSdr(1);

   // Find the best overlap from the input values
   // NOTE : In the case of no overlap (no predcition), dMaxOverlapValue will not be updated and the last value will still be there.
   dPreviousMaxOverlapValue = dMaxOverlapValue;
   dNumValuesWithOverlap = dpProjectionSdr->GetStrongestOverlapAmounts(dpProjectionSdr->dpData, &dMaxOverlapValue, dOverlapArraySize, dOverlapValueArray, dOverlapAmountArray, dNotOverlapAmountArray);

   if (dNumValuesWithOverlap == 0)
   { 
      CJTRACE(TRACE_HIGH_LEVEL, "No Overlap to classify");
   }
   else
   {
      CJTRACE(TRACE_HIGH_LEVEL, "Best val=%.3lf", dMaxOverlapValue);
      for (int i = 0; i < MIN(dOverlapArraySize, dNumValuesWithOverlap); i++)
      {
         CJTRACE(TRACE_HIGH_LEVEL, "Match val=%.3lf, overlap=%.3lf, not=%.3lf", dOverlapValueArray[i], dOverlapAmountArray[i], dNotOverlapAmountArray[i]);
      }
   }
}


bool vClassifier::CreateProjectionSdr(int numSteps)
{
   vRegion* curRegion = dpRegion;
   vColumn* curCol;
   vCell* curCell;
   bool projectCol;
   float maxValue=0;

   if (dpProjectionSdr == NULL)
      dpProjectionSdr = new SDR_Float( dpInputspace);
   dpProjectionSdr->ZeroAllValues(dpProjectionSdr->dpData);

   for (int colY = 0; colY < curRegion->GetSizeY(); colY++)
   {
      for (int colX = 0; colX < curRegion->GetSizeX(); colX++)
      {
         // Get a pointer to the current column.
         vColumn curCol(curRegion, colX, colY);
         //curCol = curRegion->GetColumn(colX, colY);

         projectCol = false;

         if (numSteps == 0)
         {
            // Project this column if it is active.
            projectCol = curCol.GetIsActive();
         }
         else
         {
            // Set projectCol to true if any cell in this column is predicting for the next time step (a sequence prediction).
            for (int cellIndex = 0; cellIndex < curRegion->GetSizeZ(); cellIndex++)
            {
               vCell curCell(curRegion, colX, colY, cellIndex);
               //curCell = curCol->Cells[cellIndex];

               if (curCell.IsCellPredicted() && (curCell.GetNumPredictionSteps() == numSteps))
               {
                  projectCol = true;
                  break;
               }
            }
         }

         // If the current proximal synapse connects from the DataSpace being displayed in this view...
         if (projectCol && (dpInputspace == dpRegion->GetInputSpace()))
         {
            int8_t* pPermanenceArray = curCol.GetProximalSegmentPermanenceArray();

            // Iterate through all proximal synapses of this column that's being projected...
            for (int synapseIndex = 0; synapseIndex < curCol.GetProximalSegmentReceptiveRange(); synapseIndex++)
            {
               // Add the current synapse's permanence to the imageVal corresponding to the cell in the DataSpace being
               // displayed by this view, that the Synapse connects from.
               vPosition pos = curCol.GetProximalSynapseConnectionInputPosition(synapseIndex);
               dpProjectionSdr->IncrementValue(dpProjectionSdr->dpData, pos.x, pos.y, 0, (float)pPermanenceArray[synapseIndex]);

               // Record the maximum imageVal among all cells, to use later for normalization.
               maxValue = MAX(maxValue, dpProjectionSdr->GetValue(dpProjectionSdr->dpData, pos.x, pos.y, 0));

            }
         }
      }
   }
   return true;
}


