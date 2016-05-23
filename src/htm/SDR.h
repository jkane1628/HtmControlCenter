#pragma once

class vInputSpace;


/// A data structure representing a single context sensitive cell.
class SDR
{
public:
   SDR(int xSize, int ySize, int active_cells, int active_range, float min_value, float max_value);
   SDR(vInputSpace* pCompatibleInputSpace);
   ~SDR();

   void  GenerateSDR(int* pData, float value);
   float GetSDRResolution();
   int   GetRepresentableValueCount();

   bool  GetOverlapCounts(int* pDataSpace1, int* pDataSpace2, int* pOverlapCount = NULL, int* pNotOverlapCount = NULL);
   void  GetOverlapAmounts(float* pDataSpace1, int* pDataSpace2, float* pOverlapAmount = NULL, float* pNotOverlapAmount = NULL);

   int   GetStrongestOverlapCounts(int* pDataSpace, float* pMaxOverlapValue, int maxOutputEntries, float* pStrongestValueArray = NULL, int* pStrongestOverlapCountArray = NULL, int* pStrongestNotOverlapCountArray = NULL);
   int   GetStrongestOverlapAmounts(float* pDataSpace, float* pMaxOverlapValue, int maxOutputEntries, float* pStrongestValueArray = NULL, float* pStrongestOverlapAmountArray = NULL, float* pStrongestNotOverlapAmountArray = NULL);
   
   bool  GetIsActive(int* pData, int _x, int _y, int _index);
   void  SetIsActive(int* pData, int _x, int _y, int _index, bool _active);
   void  DeactivateAll(int* pData);

   void  ZeroAllValues(float* pData);
   int   GetValue(int* pData, int _x, int _y, int _index);
   float GetValue(float* pData, int _x, int _y, int _index);
   void  SetValue(float* pData, int _x, int _y, int _index, bool _active);
   void  SetValue(float* pData, int _x, int _y, int _index, float _value);
   void  IncrementValue(float* pData, int _x, int _y, int _index, float _value);
   

   int   GetIndexFromXY(int x, int y);
   void  GetXYFromIndex(int index, int* pX, int* pY);

   bool  GetFirstActivePostion(int* pData, int* pX, int* pY);
   bool  GetLastActivePostion(int* pData, int* pX, int* pY);
   bool  GetFirstActiveIndex(int* pData, int* outputIndex);
   bool  GetLastActiveIndex(int* pData, int* outputIndex);

   int   dxSize;
   int   dySize;
   int   dActiveCells;
   int   dActiveRange; 
   float dMinValue;
   float dMaxValue;

private:
   int CalcSDRResolutionSteps();
   int dResolution_steps;
   
	
};

class SDR_Int : public SDR
{
public:
   SDR_Int(int xSize, int ySize, int active_cells, int active_range, float min_value, float max_value)
   : SDR(xSize, ySize, active_cells, active_range, min_value, max_value)
   {
      dpData = new int[dxSize*dySize];
   }
   SDR_Int(SDR* pSDR)
      : SDR(pSDR->dxSize, pSDR->dySize, pSDR->dActiveCells, pSDR->dActiveRange, pSDR->dMinValue, pSDR->dMaxValue)
   {
      dpData = new int[dxSize*dySize];
   }

   ~SDR_Int()
   {
      delete[] dpData;
   }
   int* dpData;
};

class SDR_Float : public SDR
{
public:
   SDR_Float(int xSize, int ySize, int active_cells, int active_range, float min_value, float max_value)
      : SDR(xSize, ySize, active_cells, active_range, min_value, max_value)
   {
      dpData = new float[dxSize*dySize];
   }
   SDR_Float(SDR* pSDR)
      : SDR(pSDR->dxSize, pSDR->dySize, pSDR->dActiveCells, pSDR->dActiveRange, pSDR->dMinValue, pSDR->dMaxValue)
   {
      dpData = new float[dxSize*dySize];
   }
   SDR_Float(vInputSpace* pCompatibleInputSpace)
      : SDR(pCompatibleInputSpace)
   {
      dpData = new float[dxSize*dySize];
   }

   ~SDR_Float()
   {
      delete[] dpData;
   }
   float* dpData;
};

