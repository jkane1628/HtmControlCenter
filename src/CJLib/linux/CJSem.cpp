// CJSem.h : Implementation file
//


#include "CJTrace.h"
#include "CJSem.h"

#define INVALID_TRACKER_HANDLE 0xFFFFFFFF


CJSem::CJSem( ) : CJObject(NULL, "")
{
   sem_init(&d_linux_semaphore, 0, 0);
}

CJSem::CJSem(U32 starting_count, char const* pSemName) : CJObject("LIB::SEM::%s", pSemName)
{
   sem_init(&d_linux_semaphore, 0, starting_count);
}

void CJSem::Post()
{
   sem_post(&d_linux_semaphore);
}

void CJSem::Wait()
{
   sem_wait(&d_linux_semaphore);
}

BOOL CJSem::TryWait()
{
   return (sem_trywait(&d_linux_semaphore) == 0);
}

U32 CJSem::GetCount()
{
   int value;
   sem_getvalue(&d_linux_semaphore, &value);
   return value;
}

void CJSem::SetCount(U32 newCount)
{
   sem_init(&d_linux_semaphore, 0, newCount);
}

void CJSem::Lock()
{
   sem_wait(&d_linux_semaphore);
}
void CJSem::Unlock()
{
   sem_post(&d_linux_semaphore);
}



#ifdef TRACE_RWLOCK_COUNTS
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
#endif


CJRWLock::CJRWLock( char const* pRWLockName) : CJObject("LIB::RWLCK::", pRWLockName) 
{
   // This lock is initialized as UNLOCKED for reads and writes
   if( pthread_rwlock_init( &d_rwlock, NULL) != 0)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to init pthread_rwlock");
   }

#ifdef TRACE_RWLOCK_COUNTS
   d_read_count = 1;
   d_write_count = 1;
#endif
}

CJRWLock::~CJRWLock()
{
   pthread_rwlock_destroy( &d_rwlock);
}

void CJRWLock::ReadLock()
{
   int rc = pthread_rwlock_rdlock( &d_rwlock);
   if( rc != 0)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to lock for read (rc=%d)", rc);
   }

#ifdef TRACE_RWLOCK_COUNTS
   pthread_mutex_lock( &mutex1 );
   d_read_count--;
   CJTRACE(TRACE_ERROR_LEVEL, "Read Lock (read=%d, write=%d)", d_read_count, d_write_count);
   pthread_mutex_unlock( &mutex1 );
#endif

}   
void CJRWLock::ReadUnlock()
{
   int rc = pthread_rwlock_unlock( &d_rwlock);
   if( rc != 0)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to unlock for read (rc=%d)", rc);
   }

#ifdef TRACE_RWLOCK_COUNTS
   pthread_mutex_lock( &mutex1 );
   d_read_count++;
   CJTRACE(ERROR_LEVEL, "Read Unlock (read=%d, write=%d)", d_read_count, d_write_count);
   pthread_mutex_unlock( &mutex1 );
#endif
} 
void CJRWLock::WriteLock()
{
   int rc = pthread_rwlock_wrlock( &d_rwlock);
   if( rc != 0)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to lock for write (rc=%d)", rc);
   }

#ifdef TRACE_RWLOCK_COUNTS
   pthread_mutex_lock( &mutex1 );
   d_write_count--;
   CJTRACE(TRACE_ERROR_LEVEL, "Write Lock (read=%d, write=%d)", d_read_count, d_write_count);
   pthread_mutex_unlock( &mutex1 );
#endif
}   
void CJRWLock::WriteUnlock()
{
   int rc = pthread_rwlock_unlock( &d_rwlock);
   if( rc != 0)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to unlock for write (rc=%d)", rc);
   }

#ifdef TRACE_RWLOCK_COUNTS
   pthread_mutex_lock( &mutex1 );
   d_write_count++;
   CJTRACE(TRACE_ERROR_LEVEL, "Write Unlock (read=%d, write=%d)", d_read_count, d_write_count);
   pthread_mutex_unlock( &mutex1 );
#endif
}
