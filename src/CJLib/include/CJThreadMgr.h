// CJThreadMgr.h : Header file
//

#ifndef __CJTHREADMGR_H_
#define __CJTHREADMGR_H_

//#include <stdarg.h>

#include "CJConfig.h"
#include "CJTypes.h"
#include "CJObject.h"

class CJThread;
class CJList;

class CJThreadMgr : public CJObject
{
public:
   CJThreadMgr( char* pThreadMgrNameStr);
   ~CJThreadMgr( );

   BOOL AddThread( CJThread* pThread);
   BOOL RemoveThread( CJThread* pThread);
   CJThread* GetNextThread( CJThread* pThread);
   U32 GetThreadCount();
   
private:

   CJList*  dpThreadList;
};

extern CJThreadMgr* gpThreadMgr;

#endif //__CJTHREADMGR_H_
