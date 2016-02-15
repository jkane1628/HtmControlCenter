// CJAssert.cpp : Implementation file
//

#include <stdarg.h>
#include "CJConsole.h"

U32 gAssertCounter = 0;
void gASSERT(CJConsole* pConsole, char* fmt, ...) 
{
   va_list argList;
   if( gAssertCounter < 5)                            
   { 
      if( pConsole != NULL)
      {
         pConsole->Printf( "**** APPLICATION FAULT ****\n");     
         pConsole->Printf( "FILE: %s:%s\n", __FILE__, __LINE__); 
         pConsole->Printf( "REASON: ");                          
         va_start(argList, fmt);
         pConsole->Printf( fmt, argList);                    
         va_end(argList);
      }
      gAssertCounter++;                               
   }                                                  
   else if( gAssertCounter == 5)                            
   {                                                  
      if( pConsole != NULL)
      {
         pConsole->Printf( "TOO MANY ASSERTS DETECTED, SENDING KILL MESSAGE (gAssertCounter=%u)\n", gAssertCounter); 
      }
   }
}

