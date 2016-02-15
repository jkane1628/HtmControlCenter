// CJPool.cpp : Implementation file
//

#include "stdio.h"

#include "CJTrace.h"
#include "CJSem.h"

#include "CJPool.h"



CJPool::CJPool( char const* pPoolNameStr, U32 maxNumObjects) : CJObject("LIB::POOL::", pPoolNameStr)
{
   char tempname[256];
   dppFreeArray = new void*[maxNumObjects];

   //sprintf(tempname, "%s_Rsv", pPoolNameStr);
   //dpReservedList = new CJList(tempname);
   //
   //sprintf(tempname, "%s_Fre", pPoolNameStr);
   //dpFreeCountSem = new CJSem(tempname, 0);

   sprintf(tempname, "%s_Lck", pPoolNameStr);
   dpLockSem = new CJSem(1, tempname);     

   dNumTotalElements = 0;
   dNumReservedElements = 0;
   dNumFreeElements = 0;
   dMaxNumObjects = maxNumObjects;
}


CJPool::~CJPool( )
{
   delete dpLockSem;
   //delete dpFreeCountSem;
   //delete dpReservedList;
   delete []dppFreeArray;
}



BOOL CJPool::AddElementToPool( void* pNewPoolObject)
{
   if( pNewPoolObject == NULL)
   {
      CJTRACE(TRACE_DBG_LEVEL, "Cannot add to pool, object is NULL");
      return FALSE;
   }


   if(dNumTotalElements >= dMaxNumObjects)
   {
      CJTRACE(TRACE_DBG_LEVEL, "Cannot add to pool, already at max (max=%u)", dMaxNumObjects);
      return FALSE;
   }


   CJTRACE(TRACE_DBG_LEVEL, "Adding object to pool (%p, total/resv/free=%u/%u/%u)", pNewPoolObject, GetTotalCount(), GetReservedCount(), GetFreeCount());

   dpLockSem->Lock();
   dppFreeArray[dNumFreeElements] = pNewPoolObject;
   dNumTotalElements++;
   dNumFreeElements++;
   dpLockSem->Unlock();
   return TRUE;
}



//BOOL CJPool::RemoveObjectFromPool();

void*
CJPool::ReserveElement()
{
   void* pReturn;
   dpLockSem->Lock();

   if( dNumFreeElements == 0)
   {
      dpLockSem->Unlock();
      return NULL;
   }

   dNumReservedElements++;
   dNumFreeElements--;
   pReturn = dppFreeArray[dNumFreeElements];

   CJTRACE(TRACE_DBG_LEVEL, "Reserving from pool (%p, total/resv/free=%u/%u/%u, sem=%u)", pReturn, GetTotalCount(), GetReservedCount(), GetFreeCount(), dpLockSem->GetCount());
   dpLockSem->Unlock();

   return pReturn;
} 


BOOL
CJPool::ReleaseElement( void* pObjectToRelease)
{
   dpLockSem->Lock();

   CJTRACE(TRACE_DBG_LEVEL, "Releasing to pool (%p, total/resv/free=%u/%u/%u, sem=%u)", pObjectToRelease, GetTotalCount(), GetReservedCount(), GetFreeCount(), dpLockSem->GetCount());


   if( dNumFreeElements >= dNumTotalElements)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to release to pool, there would be more free objects than total (total/resv/free=%u/%u/%u)", GetTotalCount(), GetReservedCount(), GetFreeCount());
      dpLockSem->Unlock();
      return FALSE;
   }

   dppFreeArray[dNumFreeElements] = pObjectToRelease;
   dNumReservedElements--;
   dNumFreeElements++;
   dpLockSem->Unlock();
   return TRUE;
}

//void
//CJPool::ReleaseAllElements()
//{
//   CJPoolElement* pElement = (CJPoolElement*)dpReservedList->GetTail();
//   while (pElement)
//   {
//      //ReleaseElement(pElement);
//      if( ReleaseElement(pElement) == FALSE)
//      {
//         CJTRACE(ERROR_LEVEL, "ERROR: Pool is corrupted");
//      }
//      pElement = (CJPoolElement*)dpReservedList->GetNext(pElement);
//   }
//}


//CJPoolElement* 
//CJPool::GetFirstReserved()
//{
//   return (CJPoolElement*)dpReservedList->GetHead();
//}
//
//CJPoolElement* 
//CJPool::GetNextReserved( CJPoolElement* pPoolElement)
//{
//   if( pPoolElement == NULL)
//   {
//      return GetFirstReserved();
//   }
//   return (CJPoolElement*)dpReservedList->GetNext(pPoolElement);
//}
//
//CJPoolElement* 
//CJPool::GetLastReserved()
//{
//   return (CJPoolElement*)dpReservedList->GetTail();
//}
//
//CJPoolElement* 
//CJPool::GetPrevReserved( CJPoolElement* pPoolElement)
//{
//   if( pPoolElement == NULL)
//   {
//      return GetLastReserved();
//   }
//   return (CJPoolElement*)dpReservedList->GetPrev(pPoolElement);
//}


