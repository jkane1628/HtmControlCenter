// CJPoolBlocking.cpp : Implementation file
//

#include "stdio.h"

#include "CJTrace.h"
#include "CJSem.h"

#include "CJPoolBlocking.h"



CJPoolBlocking::CJPoolBlocking( char const* pPoolNameStr, U32 maxNumObjects) : CJObject("LIB::POOL::", pPoolNameStr)
{
   char tempname[256];
   dppFreeArray = new void*[maxNumObjects];

   sprintf(tempname, "%s_Fre", pPoolNameStr);
   dpFreeCountSem = new CJSem(0, tempname);

   sprintf(tempname, "%s_Lck", pPoolNameStr);
   dpLockSem = new CJSem(1, tempname);     

   dNumTotalElements = 0;
   //dNumReservedElements = 0;
   dNumFreeElements = 0;
   dMaxNumObjects = maxNumObjects;
}


CJPoolBlocking::~CJPoolBlocking( )
{
   delete dpLockSem;
   delete dpFreeCountSem;
   //delete dpReservedList;
   delete []dppFreeArray;
}



BOOL CJPoolBlocking::AddElementToPool( void* pNewPoolObject)
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


   dpLockSem->Lock();
   dppFreeArray[dNumFreeElements] = pNewPoolObject;
   dNumTotalElements++;
   dNumFreeElements++;
   dpFreeCountSem->Post();
   dpLockSem->Unlock();


   //CJTRACE(TRACE_ERROR_LEVEL, "Adding object to pool (%p, total/resv/free=%u/%u/%u)", pNewPoolObject, GetTotalCount(), GetTotalCount() - GetFreeCount(), GetFreeCount());

   return TRUE;
}



//BOOL CJPoolBlocking::RemoveObjectFromPool();

void*
CJPoolBlocking::ReserveElement()
{
   void* pReturn;

   dpFreeCountSem->Wait();

   dpLockSem->Lock();

   //if( dNumFreeElements == 0)
   //{
   //   dpLockSem->Unlock();
   //   return NULL;
   //}
   //dNumReservedElements++;
   dNumFreeElements--;
   pReturn = dppFreeArray[dNumFreeElements];
   //CJTRACE(TRACE_ERROR_LEVEL, "Reserving from pool (%p, total/resv/free=%u/%u/%u, sem=%u)", pReturn, GetTotalCount(), GetTotalCount() - GetFreeCount(), GetFreeCount(), dpLockSem->GetCount());
   dpLockSem->Unlock();

   return pReturn;
} 


BOOL
CJPoolBlocking::ReleaseElement( void* pObjectToRelease)
{
   dpLockSem->Lock();

   //CJTRACE(TRACE_ERROR_LEVEL, "Releasing to pool (%p, total/resv/free=%u/%u/%u, sem=%u)", pObjectToRelease, GetTotalCount(), GetReservedCount(), GetFreeCount(), dpLockSem->GetCount());

   if( dpFreeCountSem->GetCount() >= dNumTotalElements)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to release to pool, there would be more free objects than total (total/resv/free=%u/%u/%u)", GetTotalCount(), GetTotalCount() - GetFreeCount(), GetFreeCount());
      dpLockSem->Unlock();
      return FALSE;
   }

   dppFreeArray[dNumFreeElements] = pObjectToRelease;
   dNumFreeElements++;
   dpFreeCountSem->Post();
   dpLockSem->Unlock();

   return TRUE;
}

inline
U32 CJPoolBlocking::GetFreeCount()
{
   return dpFreeCountSem->GetCount();
}

U32 CJPoolBlocking::GetReservedCount()
{
   return (GetTotalCount() - dpFreeCountSem->GetCount());
}



//void
//CJPoolBlocking::ReleaseAllElements()
//{
//   CJPoolBlockingElement* pElement = (CJPoolBlockingElement*)dpReservedList->GetTail();
//   while (pElement)
//   {
//      //ReleaseElement(pElement);
//      if( ReleaseElement(pElement) == FALSE)
//      {
//         CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Pool is corrupted");
//      }
//      pElement = (CJPoolBlockingElement*)dpReservedList->GetNext(pElement);
//   }
//}


//CJPoolBlockingElement* 
//CJPoolBlocking::GetFirstReserved()
//{
//   return (CJPoolBlockingElement*)dpReservedList->GetHead();
//}
//
//CJPoolBlockingElement* 
//CJPoolBlocking::GetNextReserved( CJPoolBlockingElement* pPoolElement)
//{
//   if( pPoolElement == NULL)
//   {
//      return GetFirstReserved();
//   }
//   return (CJPoolBlockingElement*)dpReservedList->GetNext(pPoolElement);
//}
//
//CJPoolBlockingElement* 
//CJPoolBlocking::GetLastReserved()
//{
//   return (CJPoolBlockingElement*)dpReservedList->GetTail();
//}
//
//CJPoolBlockingElement* 
//CJPoolBlocking::GetPrevReserved( CJPoolBlockingElement* pPoolElement)
//{
//   if( pPoolElement == NULL)
//   {
//      return GetLastReserved();
//   }
//   return (CJPoolBlockingElement*)dpReservedList->GetPrev(pPoolElement);
//}


