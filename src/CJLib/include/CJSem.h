// CJSem.h : Header file
//

#ifndef __CJSEM_H_
#define __CJSEM_H_

#ifdef CJLINUX
#include "semaphore.h"
#include "pthread.h"
#endif

#include "CJTypes.h"
#include "CJObject.h"

//#define TRACE_RWLOCK_COUNTS 1

class CJSem : public CJObject
{
public:
   CJSem();  // The count defaults to 0
   CJSem(U32 starting_count, char const* pSemName = NULL);
   //~CJSem( );

   void Post();
   void Wait();
   BOOL TryWait();
   U32  GetCount();
   void SetCount(U32 newCount);

   void Lock();
   void Unlock();

private:

#ifdef CJWINDOWS 
   HANDLE d_win_semaphore;
#else
   sem_t d_linux_semaphore;
#endif 
};



#ifdef CJLINUX

class CJRWLock : public CJObject
{
public:
   // This lock is initialized as UNLOCKED for reads and writes
   CJRWLock( char const* pRWLockName);
   ~CJRWLock();

   void ReadLock();    // Locks so that only readers can access
   void ReadUnlock();  
   void WriteLock();   // Locks both readers and writers
   void WriteUnlock();

private:
#ifdef TRACE_RWLOCK_COUNTS
   int d_read_count;
   int d_write_count;
#endif

   pthread_rwlock_t d_rwlock;

};
#endif // CJLINUX

#endif // __CJSEM_H_
