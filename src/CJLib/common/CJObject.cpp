// CJObject.cpp : Implmentation file
//
#include <string.h>

#include "CJTrace.h"
#include "CJObject.h"


U32 CJObject::gNextTraceId = 0; 

CJObject::CJObject()
{
   CJObject("","");
}

CJObject::CJObject( char const* pPoolNameBaseStr, char const* pPoolNameUserStr)
{
   memset( dObjectNameStr, 0, MAX_OBJ_STR_LEN);
   if( pPoolNameBaseStr != NULL)
   {
      strncpy( dObjectNameStr, pPoolNameBaseStr, MAX_OBJ_STR_LEN-1);
   }

   if( pPoolNameUserStr != NULL)
   {
      strncat( dObjectNameStr, pPoolNameUserStr, MAX_OBJ_STR_LEN-1-strlen(dObjectNameStr));
      dObjectNameStr[MAX_OBJ_STR_LEN-1] = 0;
   }
      
   dTraceId = gNextTraceId;
   gNextTraceId++;
   dTraceLevel = CJTRACE_DEFAULT_LEVEL;
}

void  CJObject::SetObjectStr( char const* pObjString) 
{
   strncpy( dObjectNameStr, pObjString, MAX_OBJ_STR_LEN);
}





