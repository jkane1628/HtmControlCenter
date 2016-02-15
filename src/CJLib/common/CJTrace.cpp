// CJTrace.cpp : Implementation file
//

#include <stdio.h>
#include <time.h>

#include "CJConsole.h"
#include "CJTrace.h"
//#include "CJTime.h"
#include "CJSem.h"

CJTrace gTrace("GLOBAL");
CJTrace* gpTrace = &gTrace;


CJTrace::CJTrace( char const* pTraceNameStr, U32 traceBufferSizeBytes) : CJObject("LIB::TRACE::", pTraceNameStr)
{
   dNumRegisteredConsole = 0;
   for( int i=0; i<MAX_NUM_REGISTERED_CONSOLE+1; i++)
   {
      dpConsoleArray[i] = NULL;
   }
   for( int j=0; j<MAX_NUM_REGISTERED_TRACE_OBJECTS; j++)
   {
      dpObject[j] = NULL;
   }

   dTraceBuffer = new char[CJTRACE_DEFAULT_TRACE_BUFFER_SIZE_BYTES + 256];
   dTraceBufferNext = dTraceBuffer;
   dTraceBufferNumBytes = 0;
   dConsoleTraceOn = TRUE;

   //dpTime = new CJTime();
   dpLockSem = new CJSem(1, "TraceLock");
}

CJTrace::~CJTrace( )
{
   delete []dTraceBuffer;
   //delete dpTime;
}



void CJTrace::Trace( U32 level, char const* objStr, char const* fmt,...)
{
   va_list argList;
   char traceFmtStr[256];

#ifdef CJLINUX
   // High res time stamp (num microseconds since last entry)
   static struct timespec gLastTimestamp;
   struct timespec t;
   clock_gettime(CLOCK_MONOTONIC, &t);
   U32 current_microseconds = t.tv_nsec/1000 + (t.tv_sec%10*1000000); // 10 second wrap
   U32 diff_microseconds =  (t.tv_nsec - gLastTimestamp.tv_nsec)/1000 + (t.tv_sec- gLastTimestamp.tv_sec)*1000000;   
   gLastTimestamp = t;
#else
   U32 current_microseconds = 0;
   U32 diff_microseconds = 0;
#endif

   // Date and Time  // TODO: REPLACE WITH CJ TIME
   //char time[20];
   //time[0] = 0;
   
   /*  struct timeval tv;
   time_t curtime;  
   gettimeofday(&tv, NULL); 
   curtime=tv.tv_sec;
   strftime(time,20,"%m/%d %H:%M",localtime(&curtime));
   */

   // Format Final String
#ifdef CJWINDOWS 
   sprintf_s(traceFmtStr, 256, "(%s) %s\n", objStr, fmt);
#else
   sprintf(traceFmtStr, "(%u/%-7u)(%s) %s\n", diff_microseconds, current_microseconds, objStr, fmt);
#endif
   va_start(argList, fmt);
   TraceVPrintf(level, traceFmtStr, argList);
   va_end(argList);
}

void CJTrace::TracePrintf( U32 level, char const* fmt,...)
{
   va_list argList;
   va_start(argList, fmt);
   TraceVPrintf( level, fmt, argList);
   va_end(argList);
}


void CJTrace::TraceVPrintf( U32 level, char const* fmt, va_list argList)
{
   char* pThisEntry = dTraceBufferNext;
   int string_length;
   int i=0;


   dpLockSem->Lock();

   // Write to the trace buffer
   string_length = vsnprintf( pThisEntry, 255, fmt, argList);

   if( dConsoleTraceOn)
   {
      // Print to all CJConsoles
      while (dpConsoleArray[i])
      {
         dpConsoleArray[i]->PrintString(pThisEntry);
         i++;
      }
   }

   if( string_length >= 255)
      dTraceBufferNext += 255;
   else
      dTraceBufferNext += string_length;

   // Add on an 'esc' char to indicate the end of an entry
   *dTraceBufferNext = 27;
   dTraceBufferNext++;


   if((dTraceBufferNumBytes + string_length) <= CJTRACE_DEFAULT_TRACE_BUFFER_SIZE_BYTES)
   {
      dTraceBufferNumBytes += string_length;
   }

   // Wrap the buffer
   if( dTraceBufferNext >= (dTraceBuffer + CJTRACE_DEFAULT_TRACE_BUFFER_SIZE_BYTES))
   {

      printf("WARNING: Wrapping trace buffer\n");
      dTraceBufferNext = dTraceBuffer;
   }

   dpLockSem->Unlock();

}


BOOL CJTrace::RegisterObject( CJObject* pObject)
{
   // Find an empty slot
   for( int j=0; j<MAX_NUM_REGISTERED_TRACE_OBJECTS; j++)
   {
      if( dpObject[j] == NULL)
      {
         dpObject[j] = pObject;
         return TRUE;
      }
   }
   return FALSE;
}

BOOL CJTrace::UnregisterObject( CJObject* pObject)
{
   // Find the object and remove it
   for( int j=0; j<MAX_NUM_REGISTERED_TRACE_OBJECTS; j++)
   {
      if( dpObject[j] == pObject)
      {
         dpObject[j] = NULL;
         return TRUE;
      }
   }
   return FALSE;
}


BOOL CJTrace::SetTraceObjectLevel( U32 traceID, U32 newLevel)
{
   for( int j=0; j<MAX_NUM_REGISTERED_TRACE_OBJECTS; j++)
   {
      if( dpObject[j] != NULL)
      {
         if ((dpObject[j]->GetTraceId() == traceID) || (traceID == 0))
         {
            dpObject[j]->SetTraceLevel(newLevel);
            if (traceID != 0)
            {
               // No need to continue
               return TRUE;
            }
         }
      }
   }
   return (traceID == 0) ? TRUE : FALSE;
}


void CJTrace::DisplayRegisteredObjects( CJConsole* pConsole)
{
   char levelStr[24];
   pConsole->Printf("REGISTERED TRACE OBJECTS:\n");
   pConsole->Printf("ID    NAME                                              LEVEL\n");
   pConsole->Printf("----  ------------------------------------------------  -----------\n");

   for( int j=0; j<MAX_NUM_REGISTERED_TRACE_OBJECTS; j++)
   {
      if( dpObject[j] != NULL)
      {
         switch(dpObject[j]->GetTraceLevel())
         {
         case TRACE_OFF:   sprintf( levelStr, "OFF(%u)", dpObject[j]->GetTraceLevel()); break;
         case TRACE_ERROR_LEVEL: sprintf(levelStr, "ERROR(%u)", dpObject[j]->GetTraceLevel()); break;
         case TRACE_HIGH_LEVEL:  sprintf(levelStr, "HIGH(%u)", dpObject[j]->GetTraceLevel()); break;
         case TRACE_LOW_LEVEL:   sprintf(levelStr, "LOW(%u)", dpObject[j]->GetTraceLevel()); break;
         case TRACE_DBG_LEVEL:   sprintf(levelStr, "DEBUG(%u)", dpObject[j]->GetTraceLevel()); break;
         default:          sprintf( levelStr, "UNKNOWN(%u)", dpObject[j]->GetTraceLevel()); break;
         }

         pConsole->Printf("%-4u  %-48s  %-11s\n", dpObject[j]->GetTraceId(), dpObject[j]->GetObjectStr(), levelStr);
      }
   }
   pConsole->Printf("\n");
}


BOOL CJTrace::RegisterConsole( CJConsole* pConsole)
{
   if( dNumRegisteredConsole >= MAX_NUM_REGISTERED_CONSOLE)
   {
      return FALSE;
   }
   dpConsoleArray[dNumRegisteredConsole] = pConsole;
   dNumRegisteredConsole++;
   return TRUE;
}

BOOL  CJTrace::UnregisterConsole(CJConsole* pConsole)
{
   BOOL foundConsole = FALSE;
   for (U32 i = 0; i < dNumRegisteredConsole; i++)
   {
      if (dpConsoleArray[i] == pConsole)
      {
         foundConsole = TRUE;
      } 
      if (foundConsole)
      {
         dpConsoleArray[i] = dpConsoleArray[i + 1];
      }
   }
   if (foundConsole == FALSE)
   {
      return FALSE;
   }
   dNumRegisteredConsole--;
   return TRUE;
}


void CJTrace::DisplayTraceBuffer(CJConsole* pConsole, U32 numBytesToDisplay)
{
   char* pBuffer = dTraceBufferNext;
   BOOL foundNextEntry = FALSE;

   dpLockSem->Lock();

   if(numBytesToDisplay > dTraceBufferNumBytes)
   {
      numBytesToDisplay = dTraceBufferNumBytes;
   }

   // Back up pBuffer
   if((pBuffer - numBytesToDisplay) >= dTraceBuffer)
   {
      // No wrap back
      pBuffer -= numBytesToDisplay;
   }
   else
   {
      // Wrap back (this calc is a little tricky)
      pBuffer = CJTRACE_DEFAULT_TRACE_BUFFER_SIZE_BYTES - numBytesToDisplay + dTraceBufferNext;
   }

   if( numBytesToDisplay == 0)
   {
      pConsole->Printf("  <The trace buffer is empty>\n");
   }

   // Find the start of the next entry
   while( pBuffer != dTraceBufferNext)
   {
      if((pBuffer[0] == 27))
      {
         if(foundNextEntry == FALSE)
            foundNextEntry = TRUE;  
         if(pBuffer > (dTraceBuffer + CJTRACE_DEFAULT_TRACE_BUFFER_SIZE_BYTES))
         {
            pBuffer = dTraceBuffer;
         }
      }
      else if(foundNextEntry)
      {
         pConsole->PrintChar(pBuffer[0], FALSE);
      }

      pBuffer++;
   }

   pConsole->PrintChar('\n');
   pConsole->Flush();

   dpLockSem->Unlock();
}



