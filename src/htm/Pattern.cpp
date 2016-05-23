
#include "CJTrace.h"
#include "CJAssert.h"
#include "vInputSpace.h"
#include "Pattern.h"



Pattern::Pattern(vInputSpace* pInputSpace, ePatternType type)
   :
   dPatternType(type),
   dpInputSpace(pInputSpace)
{}
Pattern::~Pattern(void) {}

void Pattern::ResetPattern()
{
   dPatternTimestep = 0;
}

void Pattern::ApplyPattern(int time)
{
   float nextValue;

   dPatternTimestep = time;
   switch (dPatternType)
   {
   case PATTERN_NONE:
      // Do nothing
      dpInputSpace->DeactivateAll();
      break;

   case PATTERN_ENCODED_SEQUENCE:
   case PATTERN_ENCODED_FILE:
      dpInputSpace->DeactivateAll();
      nextValue = dValues[dPatternTimestep % dValues.size()];
      dpInputSpace->EncodeSDR(nextValue);
      CJTRACE(TRACE_HIGH_LEVEL, "Input  val=%.3lf,res=%.3lf,max=%d", nextValue, dpInputSpace->GetEncoderResolutionMin(), dpInputSpace->GetEncoderResolutionSteps());
      break;

   case PATTERN_ENCODED_RANDOM:
      // Start by clearing all activity.
      dpInputSpace->DeactivateAll();
      nextValue = abs(rand() * (dpInputSpace->GetEncoderMaxValue() - dpInputSpace->GetEncoderMinValue()) / 32768) + dpInputSpace->GetEncoderMinValue();
      dpInputSpace->EncodeSDR(nextValue);
      CJTRACE(TRACE_HIGH_LEVEL, "Input  val=%.3lf,res=%.3lf,max=%d", nextValue, dpInputSpace->GetEncoderResolutionMin(), dpInputSpace->GetEncoderResolutionSteps());
      break;

   default:
      CJASSERT("ERROR: Unknown pattern type configured (type=%d)", dPatternType);
   }
}

void Pattern::InitSequence_Sin()
{

}
//void InitSequence_Cos();
//void InitSequence_Tan();
//void InitSequence_Square();


void Pattern::InitSequence_ClearAll()
{
   dValues.clear();
}
void Pattern::InitSequence_AddValue(float value)
{
   dValues.push_back(value);
}
