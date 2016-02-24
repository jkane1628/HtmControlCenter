// CJSem.h : Windows Implementation file
//

#include "CJTrace.h"
#include "CJSem.h"


CJSem::CJSem()
{
   DWORD err;
   d_win_semaphore = CreateSemaphore(NULL, 0, 0xFFFF, NULL);
   if (d_win_semaphore == NULL)
   {
      err = GetLastError();
   }
}

CJSem::CJSem( U32 starting_count)
{
   DWORD err;
   d_win_semaphore = CreateSemaphore(NULL, starting_count, 0xFFFF, NULL);
   if (d_win_semaphore == NULL)
   {
      err = GetLastError();
   }
}

void CJSem::Post()
{
   ReleaseSemaphore(d_win_semaphore, 1, NULL);
}

void CJSem::Wait()
{
   if( WaitForSingleObject(d_win_semaphore, INFINITE) != 0)
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Bad rc from CJSem::Wait");
}

BOOL CJSem::TryWait()
{
   return (WaitForSingleObject(d_win_semaphore, 0) == 0);
}

U32 CJSem::GetCount()
{
   LONG value;
   ReleaseSemaphore(d_win_semaphore, 0, &value);
   return value;
}

void CJSem::SetCount(U32 newCount)
{
   CloseHandle(d_win_semaphore);
   d_win_semaphore = CreateSemaphore(NULL, newCount, 0xFFFFFFFF, NULL);
}

void CJSem::Lock()
{
   Wait();
}
void CJSem::Unlock()
{
   Post();
}


/*
#ifdef TRACE_RWLOCK_COUNTS
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
#endif


CJRWLock::CJRWLock( char* pRWLockName) : CJObject("LIB::RWLCK::", pRWLockName) 
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
   CJTRACE(TRACE_ERROR_LEVEL, "Read Unlock (read=%d, write=%d)", d_read_count, d_write_count);
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
*/