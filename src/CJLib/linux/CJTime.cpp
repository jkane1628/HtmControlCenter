// CJTime.h : Implementation file
//

#include <unistd.h>
#include <sys/time.h>

#include "CJTime.h"

static CJTime gTime;
CJTime* gpTime = &gTime;

CJTime::CJTime() : CJObject("LIB::TIME::", "stack")
{
   dHighResClockRateMHz=0.0;            
   dHighResClockPeriodMsec=0.0;
   CalibrateHighResTimestamp();
}

CJTime::CJTime( char* pTimeName) : CJObject("LIB::TIME::", pTimeName)
{
   dHighResClockRateMHz=0.0;            
   dHighResClockPeriodMsec=0.0;
   CalibrateHighResTimestamp();
}

//CJTime::~CJTime()


void   
CJTime::CalibrateHighResTimestamp()
{
   timeval startTime, endTime;
   HighResTicks_t start, stop, cycles;

   gettimeofday( &startTime, NULL);
   start = GetHighResTimestamp();

   usleep(500000);

   gettimeofday( &endTime, NULL);
   stop = GetHighResTimestamp();

   cycles = GetTicksElapsed(stop,start);

   double totalSeconds = (double)(endTime.tv_sec - startTime.tv_sec) + 
                         (double)(endTime.tv_usec - startTime.tv_usec)/1000000.0;


   dHighResClockPeriodMsec = (totalSeconds / (double) cycles) * 1000.0;
   dHighResClockRateMHz = 1.0 / (1000.0 * dHighResClockPeriodMsec);          
}


#if defined(__x86_64__)
HighResTicks_t 
CJTime::GetHighResTimestamp()
{
   U32 hi, lo;
   U64 ret;

   __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));

   ret = ((U64) hi) << 32;
   ret |= lo;
   return (ret);
}
#else
HighResTicks_t 
CJTime::GetHighResTimestamp()
{
   U32 ts;

   // NEED TO GET TIME ON RASP PI 2
   ts = 0;
   //__asm__ volatile (".byte 0x0f,0x31" : "=A" (ts));

   return ts;
}
#endif



HighResTicks_t CJTime::GetTicksElapsed(HighResTicks_t end, HighResTicks_t beg)
{
   if (end > beg)
      return (end - beg);
   else // rollover condition
#if defined(__x86_64__)
      return ((((U64)(0xFFFFFFFFFFFFFFFF)) - beg) + end);
#else
      return ((((U64)(0xFFFFFFFF)) - beg) + end);
#endif
}



double 
CJTime::TicksToMicroSeconds( HighResTicks_t ticks)
{
   return (TicksToMilliSeconds(ticks) * 1000.0);
}
double 
CJTime::TicksToMilliSeconds( HighResTicks_t ticks)
{
   return (dHighResClockPeriodMsec * ((double) ticks));
}
double 
CJTime::TicksToSeconds( HighResTicks_t ticks)
{
   return (TicksToMilliSeconds(ticks) / 1000.0);
}



