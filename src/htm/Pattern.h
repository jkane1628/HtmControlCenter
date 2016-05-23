#pragma once
#include <vector>


enum ePatternType
{
   PATTERN_NONE,
   /*PATTERN_STRIPE,
   PATTERN_BOUNCING_STRIPE,
   PATTERN_BAR,
   PATTERN_BOUNCING_BAR,
   PATTERN_TEXT,
   PATTERN_BITMAP,
   PATTERN_IMAGE,*/
   PATTERN_ENCODED_SEQUENCE,
   PATTERN_ENCODED_FILE,
   PATTERN_ENCODED_RANDOM

};

class vInputSpace;

class Pattern 
{
public:
   Pattern(vInputSpace* pInputSpace, ePatternType type);
   ~Pattern(void);

   void ApplyPattern(int time);
   void ResetPattern();

   void InitSequence_Sin();
   //void InitSequence_Cos();
   //void InitSequence_Tan();
   //void InitSequence_Square();

   
   void InitSequence_ClearAll();
   void InitSequence_AddValue(float value);
   //void InitSequence_AddFile();

private:
   vInputSpace* dpInputSpace;
   ePatternType dPatternType;
   uint32_t dPatternTimestep;
   std::vector<float> dValues;   
};
