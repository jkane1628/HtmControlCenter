
#include "CJTypes.h"
#include "CJTrace.h"

#include "Region.h"
#include "SDR.h"
#include "InputSpace.h"
#include "Cell.h"
#include "Column.h"
#include "ProximalSynapse.h"

#include "Classifier.h"


Classifier::Classifier(QString &_id, int _numItems, QString _regionID, QString _inputspaceID, QStringList &_labels)
	: DataSpace(_id), labels(_labels), regionID(_regionID), inputspaceID(_inputspaceID), numItems(_numItems),
   dpProjectionSdr(NULL)
{
   
}

Classifier::~Classifier(void)
{
   if (dpProjectionSdr != NULL)
      delete dpProjectionSdr;
}

int Classifier::GetSizeX()
{
   return inputspace->GetSizeX();
}

int Classifier::GetSizeY()
{
   return inputspace->GetSizeY();
}

int Classifier::GetNumValues()
{
   return inputspace->GetNumValues();
}

int Classifier::GetHypercolumnDiameter() 
{
	return 1;
}

bool Classifier::GetIsActive(int _x, int _y, int _index)
{
   return inputspace->GetIsActive(_x, _y, _index);
}


void Classifier::Classify() 
{

   // Create an SDR from the projection of the prediction cells throught the proximal synaspes
   
   CreateProjectionSdr(1);

   // Find the best overlap from the input values
   const int arraySize = 1;
   float strongestValueArray[arraySize];
   float strongestOverlapAmountArray[arraySize];
   float strongestNotOverlapAmountArray[arraySize];

   dpProjectionSdr->GetStrongestOverlapAmounts(dpProjectionSdr->dpData, arraySize, strongestValueArray, strongestOverlapAmountArray, strongestNotOverlapAmountArray);

   for (int i = 0; i < arraySize; i++)
   {
      CJTRACE(TRACE_HIGH_LEVEL, "Output val=%.3lf, overlap=%.3lf, not=%.3lf", strongestValueArray[i], strongestNotOverlapAmountArray[i]);
   }
}


bool Classifier::CreateProjectionSdr(int numSteps)
{


   Region* curRegion = region;
   Column* curCol;
   Cell* curCell;
   ProximalSynapse* pSyn;

   bool projectCol;
   int cellIndex;
   float maxValue=0;

   if (dpProjectionSdr == NULL)
      dpProjectionSdr = new SDR_Float(inputspace->dpSdr);
   dpProjectionSdr->ZeroAllValues(dpProjectionSdr->dpData);

   for (int colY = 0; colY < curRegion->GetSizeY(); colY++)
   {
      for (int colX = 0; colX < curRegion->GetSizeX(); colX++)
      {
         // Get a pointer to the current column.
         curCol = curRegion->GetColumn(colX, colY);

         projectCol = false;

         if (numSteps == 0)
         {
            // Project this column if it is active.
            projectCol = curCol->GetIsActive();
         }
         else
         {
            // Set projectCol to true if any cell in this column is predicting for the next time step (a sequence prediction).
            for (cellIndex = 0; cellIndex < curRegion->GetCellsPerCol(); cellIndex++)
            {
               curCell = curCol->Cells[cellIndex];

               if (curCell->GetIsPredicting() && (curCell->GetNumPredictionSteps() == numSteps))
               {
                  projectCol = true;
                  break;
               }
            }
         }

         if (projectCol)
         {
            // Iterate through all proximal synapses of this column that's being projected...
            FastListIter synapses_iter(curCol->ProximalSegment->Synapses);
            for (Synapse *syn = (Synapse*)(synapses_iter.Reset()); syn != NULL; syn = (Synapse*)(synapses_iter.Advance()))
            {
               pSyn = (ProximalSynapse*)syn;

               // If the current proximal synapse connects from the DataSpace being displayed in this view...
               if (pSyn->InputSource == inputspace)
               {
                  // Add the current synapse's permanence to the imageVal corresponding to the cell in the DataSpace being
                  // displayed by this view, that the Synapse connects from.
                  //columnDisps[pSyn->InputPoint.X + (pSyn->InputPoint.Y * sceneWidth)]->imageVals[pSyn->InputPoint.Index] += pSyn->GetPermanence();
                  dpProjectionSdr->IncrementValue(dpProjectionSdr->dpData, pSyn->InputPoint.X, pSyn->InputPoint.Y, 0, pSyn->GetPermanence());


                  // Record the maximum imageVal among all cells, to use later for normalization.
                  //maxImageVal = Max(maxImageVal, columnDisps[pSyn->InputPoint.X + (pSyn->InputPoint.Y * sceneWidth)]->imageVals[pSyn->InputPoint.Index]);
                  maxValue = MAX(maxValue, dpProjectionSdr->GetValue(dpProjectionSdr->dpData, pSyn->InputPoint.X, pSyn->InputPoint.Y, 0));
               }
            }
         }
      }
   }
   return true;
}


