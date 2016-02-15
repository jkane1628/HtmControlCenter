// CJTrace.cpp : Implementation file
//


#include "CJList.h"

#include "CJThreadMgr.h"

#ifdef USE_DEFAULT_GLOBAL_THREAD_MANAGER
CJThreadMgr gThreadMgr("GLOBAL");
CJThreadMgr* gpThreadMgr = &gThreadMgr;
#else
CJThreadMgr* gpThreadMgr = NULL;
#endif

CJThreadMgr::CJThreadMgr( char* pThreadMgrNameStr) : CJObject("LIB::THREADMGR::", pThreadMgrNameStr)
{
   dpThreadList = new CJList("ThreadMgrList");
}

CJThreadMgr::~CJThreadMgr( )
{
   delete dpThreadList;
}

BOOL CJThreadMgr::AddThread( CJThread* pThread)
{
   dpThreadList->AddHead((CJListObject*)pThread);
   return TRUE;
}

BOOL CJThreadMgr::RemoveThread( CJThread* pThread)
{
   return dpThreadList->Remove((CJListObject*)pThread);
}

CJThread* CJThreadMgr::GetNextThread( CJThread* pThread)
{
   return (CJThread*)dpThreadList->GetNext((CJListObject*)pThread);
}

U32 CJThreadMgr::GetThreadCount()
{
   return dpThreadList->GetNumElements();
}



