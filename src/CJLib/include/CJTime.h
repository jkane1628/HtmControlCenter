// CJTime.h : Header file
//

#ifndef __CJTIME_H_
#define __CJTIME_H_

#include <time.h>

#include "CJTypes.h"
#include "CJObject.h"


#if defined(__x86_64__)
   typedef U64 HighResTicks_t;
#else
   typedef U32 HighResTicks_t;
#endif


class CJTime : public CJObject
{
public:
    CJTime( char* pTimeName);
    CJTime();
    //~CJTime( );


    // High Resolution Timer
    void   CalibrateHighResTimestamp();
    HighResTicks_t GetHighResTimestamp();

    double TicksToMicroSeconds( HighResTicks_t ticks);
    double TicksToMilliSeconds( HighResTicks_t ticks);
    double TicksToSeconds( HighResTicks_t ticks);

    double GetHighResClockRateMHz() {return dHighResClockRateMHz;}
    double GetHighResPeriodMSec() {return dHighResClockPeriodMsec;}

private:

   HighResTicks_t GetTicksElapsed(HighResTicks_t end, HighResTicks_t beg);


   double dHighResClockRateMHz;
   double dHighResClockPeriodMsec;

};

extern CJTime* gpTime;

#endif // __CJTIME_H_
