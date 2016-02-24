// CJTrace.h : Header file
//

#ifndef __CJTRACE_H_
#define __CJTRACE_H_

#include <stdarg.h>

#include "CJConfig.h"
#include "CJTypes.h"

enum eCJTraceId
{
   eTraceId_Default,
   eTraceId_CJ_Cli,
   

   eTraceId_MainWindow,
   eTraceId_NetworkManager,
   eTraceId_BasicGraph,


   eTraceId_LAST_ENTRY
};

#define TRACE_OFF   0x00U
#define TRACE_ERROR_LEVEL 0x01U
#define TRACE_HIGH_LEVEL  0x02U
#define TRACE_LOW_LEVEL   0x04U
#define TRACE_DBG_LEVEL   0x08U

// Trace entry macro for submitting entry to global trace mechanism (entry format= TIMESTAMP  ID:<formatted trace string>)
#define CJTRACE(level,...)   if(level<=CJTrace::GetTraceLevel(CJTRACE_MODULE_ID)) CJTrace::Trace(CJTRACE_MODULE_ID,__VA_ARGS__)
#define CJPRINTF(level,...)  if(level<=CJTrace::GetTraceLevel(CJTRACE_MODULE_ID)) CJTrace::TracePrintf(__VA_ARGS__)
#define CJTRACE_REGISTER_CONSOLE(pConsole) CJTrace::RegisterConsole(pConsole);
#define CJTRACE_SET_TRACEID_STRING(id,str) CJTrace::SetTraceIdString(id,str);



#ifndef CJTRACE_MODULE_ID
#define CJTRACE_MODULE_ID eTraceId_Default  // Sets the default MODULE ID, this will be a blank string and ERROR_LEVEL tracing unless overridden in the local module
#endif




class CJConsole;
class CJTime;
class CJSem;

class CJTrace //: public CJObject
{
public:
   CJTrace( char const* pTraceNameStr, U32 traceBufferSizeBytes=CJTRACE_DEFAULT_TRACE_BUFFER_SIZE_BYTES);
   ~CJTrace( );

   static BOOL  RegisterConsole( CJConsole* pConsole);
   static BOOL  UnregisterConsole(CJConsole* pConsole);
   static void  SetCliTraceState(BOOL newState) { dConsoleTraceOn = newState; }

   static void  DisplayRegisteredObjects(CJConsole* pConsole);
   static void  SetTraceIdString(eCJTraceId traceID, char const* prefixString);

   static U32   GetTraceLevel(eCJTraceId traceID);
   static BOOL  SetTraceLevel(eCJTraceId traceID, U32 level);
   static void  SetTraceGlobalLevel(U32 level);

   static void  Trace(eCJTraceId traceID, char const* fmt, ...);
   static void  TracePrintf(char const* fmt, ...);
   static void  TraceVPrintf(char const* fmt, va_list argList);
   static void  TraceFlush();

   static void  DisplayTraceBuffer(CJConsole* pConsole, U32 numBytesToDisplay);


private:

   static char       dTraceIdStringArray[eTraceId_LAST_ENTRY][CJTRACE_PREFIX_STRING_MAX_LENGTH + 1];
   static U32        dTraceIdLevelArray[eTraceId_LAST_ENTRY];
   static CJConsole* dpConsoleArray[MAX_NUM_REGISTERED_CONSOLE + 1];
   static U32        dNumRegisteredConsole;
   
   static char*  dTraceBuffer;
   static char*  dTraceBufferNext;
   static U32    dTraceBufferNumBytes;
   static BOOL   dConsoleTraceOn;
   static CJSem* dpLockSem;

};

#endif // __CJTRACE_H_
