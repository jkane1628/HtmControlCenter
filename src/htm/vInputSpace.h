#pragma once
#include "vDataSpace.h"

class Pattern;

class vInputSpace : public vDataSpace
{
public:
   vInputSpace(QString id, int sizeX=0, int sizeY=0, int sizeZ=1);
   ~vInputSpace(void);

   uint8_t* CreateBuffer();
   vDataSpaceType GetDataSpaceType() { return VDATASPACE_TYPE_INPUTSPACE; }
   
   void  Step(int time);

   void  SetupEncoder(int active_cells, int active_range, float min_value, float max_value);
   float EncodeSDR(float value);
   float GetEncoderResolutionMin() {return dResolutionMin;}
   int   GetEncoderActiveCells() { return dActiveCells; }
   int   GetEncoderActiveRange() { return dActiveRange; }
   int   GetEncoderResolutionSteps() { return dResolutionSteps; }
   float GetEncoderMinValue() { return dMinValue; }
   float GetEncoderMaxValue() { return dMaxValue;}
   
   uint8_t* GetBuffer() { return dpBuffer; }

   bool GetIsActive(int x, int y, int z=0);
   bool GetIsActive(int index);
   void SetIsActive(int x, int y, int z=1, bool active=true);
   void SetIsActive(int index, bool active);
   void DeactivateAll();

   void AddPattern(Pattern* pPattern) {dpActivePattern = pPattern;}

   float dCurrentInputValue;

private:

   Pattern* dpActivePattern;
   
   uint8_t *dpBuffer;

   // Encoder Config
   int   dActiveCells;
   int   dActiveRange;
   float dMinValue;
   float dMaxValue;
   float dResolutionMin;
   int   dResolutionSteps;

   int   CalcEncoderResolutionSteps();
};
