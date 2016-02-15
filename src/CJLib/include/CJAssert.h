// CJAssert.h : Header file
//

#ifndef __CJASSERT_H_
#define __CJASSERT_H_

#include "CJTypes.h"
#include "CJCli.h"


#ifdef CJWINDOWS
#include <assert.h>
#else
#include <pthread.h>
#endif





extern U32 gAssertCounter;

#define CJASSERT(...) {                                      \
   if( gAssertCounter < 5)                                        \
   {                                                              \
      CJPRINTF(TRACE_ERROR_LEVEL, "\n\n\n*************************************************\n");\
      CJPRINTF(TRACE_ERROR_LEVEL, "*************** APPLICATION ASSERT **************\n");\
      CJPRINTF(TRACE_ERROR_LEVEL, "*************************************************\n");\
      CJPRINTF(TRACE_ERROR_LEVEL, "FILE: %s:%u\n", __FILE__, __LINE__); \
      CJPRINTF(TRACE_ERROR_LEVEL, "REASON: ");                          \
      CJPRINTF(TRACE_ERROR_LEVEL, __VA_ARGS__);                    \
      CJPRINTF(TRACE_ERROR_LEVEL, "\n\n\n");                            \
   }                                                              \
   else if( gAssertCounter == 5)                                  \
   {                                                              \
      CJPRINTF(TRACE_ERROR_LEVEL, "TOO MANY ASSERTS DETECTED, STOPPING THREAD (gAssertCounter=%u)\n", gAssertCounter); \
   }                                                              \
   gAssertCounter++;  }
   //pthread_exit(NULL);  }



void gASSERT(CJConsole* pConsole, char* fmt, ...);


#endif // __CJASSERT_H_
