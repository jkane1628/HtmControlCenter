// CJTrace.cpp : Implementation file
//

#include <stdio.h>

#include "CJSem.h"
#include "CJConsole.h"
//#include "CJTime.h"
#include "CJTrace.h"


char       CJTrace::dTraceIdStringArray[eTraceId_LAST_ENTRY][CJTRACE_PREFIX_STRING_MAX_LENGTH + 1];
U32        CJTrace::dTraceIdLevelArray[eTraceId_LAST_ENTRY];
CJConsole* CJTrace::dpConsoleArray[MAX_NUM_REGISTERED_CONSOLE + 1];
U32        CJTrace::dNumRegisteredConsole;

char*  CJTrace::dTraceBuffer;
char*  CJTrace::dTraceBufferNext;
U32    CJTrace::dTraceBufferNumBytes;
BOOL   CJTrace::dConsoleTraceOn;
CJSem* CJTrace::dpLockSem;

CJTrace gTrace("GLB", CJTRACE_DEFAULT_TRACE_BUFFER_SIZE_BYTES); //Created to call constructor

CJTrace::CJTrace( char const* pTraceNameStr, U32 traceBufferSizeBytes)
{
   dNumRegisteredConsole = 0;
   for( int i=0; i<MAX_NUM_REGISTERED_CONSOLE+1; i++)
   {
      dpConsoleArray[i] = NULL;
   }
   for (int j = 0; j<eTraceId_LAST_ENTRY; j++)
   {
      dTraceIdStringArray[j][0] = 0;
      dTraceIdLevelArray[j] = CJTRACE_DEFAULT_LEVEL;
   }

   dTraceBuffer = new char[CJTRACE_DEFAULT_TRACE_BUFFER_SIZE_BYTES + 256];  // TODO: DECLARE STATIC
   dTraceBufferNext = dTraceBuffer;
   dTraceBufferNumBytes = 0;
   dConsoleTraceOn = TRUE;

   dpLockSem = new CJSem(1);   // TODO: DECLARE STATIC
}

CJTrace::~CJTrace( )
{
   delete []dTraceBuffer;
   //delete dpTime;
}



void CJTrace::Trace(eCJTraceId traceID, char const* fmt, ...)
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
   //U32 current_microseconds = 0;
   //U32 diff_microseconds = 0;
   LARGE_INTEGER current_raw_count;
   QueryPerformanceCounter(&current_raw_count);
   //BOOL WINAPI QueryPerformanceFrequency(_Out_ LARGE_INTEGER *lpFrequency);

#endif

   // Format Final String
#ifdef CJWINDOWS 
   sprintf_s(traceFmtStr, 256, "%-08u %s: %s\n", current_raw_count.LowPart, dTraceIdStringArray[traceID], fmt);
#else
   sprintf(traceFmtStr, "(%u/%-7u)(%s) %s\n", diff_microseconds, current_microseconds, dTraceIdStringArray[traceID], fmt);
#endif
   va_start(argList, fmt);
   TraceVPrintf(traceFmtStr, argList);
   va_end(argList);
}

void CJTrace::TracePrintf( char const* fmt,...)
{
   va_list argList;
   va_start(argList, fmt);
   TraceVPrintf( fmt, argList);
   va_end(argList);
}


void CJTrace::TraceVPrintf( char const* fmt, va_list argList)
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


void CJTrace::SetTraceIdString(eCJTraceId traceID, char const* prefixString)
{
   strncpy(dTraceIdStringArray[traceID], prefixString, CJTRACE_PREFIX_STRING_MAX_LENGTH);
}


U32 CJTrace::GetTraceLevel(eCJTraceId traceID) { return dTraceIdLevelArray[traceID]; }

BOOL  CJTrace::SetTraceLevel(eCJTraceId traceID, U32 level)
{ 
   if (traceID >= eTraceId_LAST_ENTRY)
      return FALSE;
   dTraceIdLevelArray[traceID] = level; 
   return TRUE;
}


void  CJTrace::SetTraceGlobalLevel(U32 level) 
{ 
   for (int j = 0; j<eTraceId_LAST_ENTRY; j++)
   {
      dTraceIdLevelArray[j] = level;
   }
}


void CJTrace::DisplayRegisteredObjects( CJConsole* pConsole)
{
   char levelStr[24];
   pConsole->Printf("REGISTERED TRACE OBJECTS:\n");
   pConsole->Printf("ID    NAME                                    LEVEL\n");
   pConsole->Printf("----  --------------------------------------  -----------\n");

   for (U32 j = 0; j<eTraceId_LAST_ENTRY; j++)
   {
      if (dTraceIdStringArray[j][0] != 0)
      {
         switch (GetTraceLevel((eCJTraceId)j))
         {
         case TRACE_OFF:   sprintf( levelStr, "OFF"); break;
         case TRACE_ERROR_LEVEL: sprintf(levelStr, "ERROR"); break;
         case TRACE_HIGH_LEVEL:  sprintf(levelStr, "HIGH"); break;
         case TRACE_LOW_LEVEL:   sprintf(levelStr, "LOW"); break;
         case TRACE_DBG_LEVEL:   sprintf(levelStr, "DEBUG"); break;
         default:                sprintf(levelStr, "UNKNOWN(%u)", GetTraceLevel((eCJTraceId)j)); break;
         }
         pConsole->Printf("%-4u  %-38s  %-11s\n", j, dTraceIdStringArray[j], levelStr);
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



