// CJTrace.h : Header file
//

#ifndef __CJTRACE_H_
#define __CJTRACE_H_

#include <stdarg.h>

#include "CJConfig.h"
#include "CJTypes.h"
#include "CJObject.h"

// These macros use the default global trace mechanism
#define CJTRACE(level,...)  if(level<=this->GetTraceLevel()) gpTrace->Trace(level,this->GetObjectStr(),__VA_ARGS__)
#define CJPRINTF(level,...)  if(level<=this->GetTraceLevel()) gpTrace->TracePrintf(level,__VA_ARGS__)
#define CJTRACE_REGISTER_CONSOLE(pConsole) gpTrace->RegisterConsole(pConsole);
#define CJTRACE_REGISTER_TRACE_OBJ(pObj) gpTrace->RegisterObject(pObj);
#define CJTRACE_UNREGISTER_TRACE_OBJ(pObj) gpTrace->UnregisterObject(pObj);

#define TRACE_OFF   0x00
#define TRACE_ERROR_LEVEL 0x01
#define TRACE_HIGH_LEVEL  0x02
#define TRACE_LOW_LEVEL   0x04
#define TRACE_DBG_LEVEL   0x08



class CJConsole;
class CJTime;
class CJSem;

class CJTrace : public CJObject
{
public:
   CJTrace( char const* pTraceNameStr, U32 traceBufferSizeBytes=CJTRACE_DEFAULT_TRACE_BUFFER_SIZE_BYTES);
   ~CJTrace( );

   BOOL  RegisterConsole( CJConsole* pConsole);
   BOOL  UnregisterConsole(CJConsole* pConsole);
   void  SetCliTraceState(BOOL newState) { dConsoleTraceOn = newState; }

   BOOL  RegisterObject( CJObject* pObject);
   BOOL  UnregisterObject( CJObject* pObject);
   void  DisplayRegisteredObjects(CJConsole* pConsole);
   BOOL  SetTraceObjectLevel( U32 objectID, U32 newLevel);

   void  Trace( U32 level, char const* objStr, char const* fmt,...);
   void  TracePrintf( U32 level, char const* fmt,...);
   void  TraceVPrintf( U32 level, char const* fmt, va_list argList);
   void  TraceFlush();

   void  DisplayTraceBuffer(CJConsole* pConsole, U32 numBytesToDisplay);

   //static U32 dGlobalTraceLevel;

private:

   CJObject* dpObject[MAX_NUM_REGISTERED_TRACE_OBJECTS];
   CJConsole* dpConsoleArray[MAX_NUM_REGISTERED_CONSOLE + 1];
   U32    dNumRegisteredConsole;
   char*  dTraceBuffer;
   char*  dTraceBufferNext;
   U32    dTraceBufferNumBytes;
   BOOL   dConsoleTraceOn;

   CJTime* dpTime;
   CJSem*  dpLockSem;

};

extern CJTrace* gpTrace;

#endif // __CJTRACE_H_
