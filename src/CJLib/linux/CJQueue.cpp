// CJQueue.cpp : Implementation file
//

#include <stdio.h>
#include <time.h>

#include "CJTrace.h"
#include "CJQueue.h"

CJQueue::CJQueue( char const* pQueueNameStr, U32 maxQdepth, BOOL blockOnMaxQueued) : CJList( "LIB::QUEUE::", pQueueNameStr)
{
   dElementCount = 0;
   dMaxQdepth = maxQdepth;
   dBlockOnMaxQueued = blockOnMaxQueued;

   if( dBlockOnMaxQueued == TRUE)
   {
      sem_init(&d_max_limit_sem, 0, maxQdepth);
   }
   else
   {
      sem_init(&d_max_limit_sem, 0, 0x7FFFFFFF);
   }
   sem_init(&d_count_sem, 0, 0);
   sem_init(&d_lock_sem, 0, 1);
}

//CJQueue::~CJQueue( );



BOOL CJQueue::Queue( CJListElement* pListElement)
{
   sem_wait(&d_max_limit_sem);
   sem_wait(&d_lock_sem);
   if( dElementCount >= dMaxQdepth)
   {
      sem_post(&d_lock_sem);
      sem_post(&d_max_limit_sem);
      return FALSE;
   }
   dElementCount++;
   this->CJList::AddTail(pListElement);
   sem_post(&d_count_sem);
   sem_post(&d_lock_sem);
   return TRUE;
}


BOOL CJQueue::Dequeue( CJListElement** ppListElement, int timeout_ms)
{
   timespec timeout_val;

   if( timeout_ms == -1)
   {
      sem_wait(&d_count_sem);
   }
   else if( timeout_ms == 0)
   {
      if( sem_trywait(&d_count_sem) != 0)
      {
         return FALSE;
      }
   }
   else
   {
      if (clock_gettime(CLOCK_REALTIME, &timeout_val) != 0)
         return FALSE;

      timeout_val.tv_sec += timeout_ms / 1000;
      timeout_val.tv_nsec += 1000000 * (timeout_ms % 1000); 

      if( sem_timedwait(&d_count_sem, &timeout_val) != 0)
      {
         return FALSE;
      }
   }

   sem_wait(&d_lock_sem);
   dElementCount--;
   *ppListElement = RemoveHead();
   sem_post(&d_lock_sem);
   sem_post(&d_max_limit_sem);
   return TRUE;
}



CJListElement* CJQueue::Dequeue( int timeout_ms)
{
   CJListElement* pRetElement;
   timespec timeout_val;

   if( timeout_ms == -1)
   {
      sem_wait(&d_count_sem);
   }
   else if( timeout_ms == 0)
   {
      if( sem_trywait(&d_count_sem) != 0)
      {
         return NULL;
      }
   }
   else
   {
      if (clock_gettime(CLOCK_REALTIME, &timeout_val) != 0)
      {
         CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to call clock_gettime when calculating timeout");
         return NULL;
      }

      timeout_val.tv_sec += timeout_ms / 1000;
      timeout_val.tv_nsec += 1000000 * (timeout_ms % 1000); 

      if( sem_timedwait(&d_count_sem, &timeout_val) != 0)
      {
         return NULL;
      }
   }

   sem_wait(&d_lock_sem);
   dElementCount--;
   pRetElement = RemoveHead();
   sem_post(&d_lock_sem);
   sem_post(&d_max_limit_sem);
   return pRetElement;
}



void CJQueue::AddTail( CJListElement* pListElement)
{
   CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Cannot use AddTail() on a CJQueue obj, use Queue() instead");
}
void CJQueue::AddTail( CJListObject* pListObject)
{
   CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Cannot use AddTail() on a CJQueue obj, use Queue() instead");
}
void CJQueue::AddHead( CJListElement* pListElement)
{
   CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Cannot use AddHead() on a CJQueue obj, use Queue() instead");
}
void CJQueue::AddHead( CJListObject* pListObject)
{
   CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Cannot use AddHead() on a CJQueue obj, use Queue() instead");
}

