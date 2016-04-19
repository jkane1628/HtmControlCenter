#pragma once


class vInputSpace
{
public:
   vInputSpace( int sizeX, int sizeY);
   ~vInputSpace(void);
   
   void SetupEncoder(int active_cells, int active_range, float min_value, float max_value);
   float EncodeSDR(float value);

   int GetSizeX() { return dSizeX; }
   int GetSizeY() { return dSizeY; }
   uint8_t* GetBuffer() { return dpBuffer; }
   //int GetHypercolumnDiameter();

   bool GetIsActive(int x, int y);
   bool GetIsActive(int index);
   void SetIsActive(int x, int y, bool active);
   void SetIsActive(int index, bool active);
   void DeactivateAll();

private:
   // Properties
   int dSizeX, dSizeY, dNumCells;
   uint8_t *dpBuffer;

   // Encoder Config
   int   dActiveCells;
   int   dActiveRange;
   float dMinValue;
   float dMaxValue;
   float dResolutionMin;
   int   dResolutionSteps;

   int CalcEncoderResolutionSteps();
};
