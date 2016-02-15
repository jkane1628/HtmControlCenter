// CJThread.cpp : Windows Implementation file
//
/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
*/







#ifdef USE_DEFAULT_GLOBAL_THREAD_MANAGER
#include "CJThreadMgr.h"
#endif

#include "CJTrace.h"
#include "CJThread.h"

U32 gAssertCounter;  // TODO: REMOVE!!


CJThread::CJThread( char const* pThreadNameStr) : CJListObject("LIB::THREAD::", pThreadNameStr)
{
   dThreadState = eThreadState_Uninitialized;
   dpReturnParams = NULL;

   //dThreadParams.sched = eCJSCHEDULER_DEFAULT;
   //dThreadParams.cpuAffinityMask = CPU_AFFINITY_ALL_CPUS;

   dWinThreadId = 0;
   dWinThreadHandle = NULL;

#ifdef USE_DEFAULT_GLOBAL_THREAD_MANAGER
   gpThreadMgr->AddThread(this);
#endif
}

CJThread::~CJThread( )
{
   if( dThreadState == eThreadState_Running)
   {
      //Wait 5 seconds for thread to exit
      if( WaitForThreadExit(5000) == FALSE)
      {
         CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Thread is being deleted before it has stopped");
      }
      //if(dThreadState == eThreadState_Running)
      //{
      //   pthread_detach(dThreadHandle);
      //}
   }

#ifdef USE_DEFAULT_GLOBAL_THREAD_MANAGER
   if( gpThreadMgr->RemoveThread(this) == FALSE)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to remove the thread from the thread mgr");
   }
#endif

   if (dWinThreadHandle != NULL)
      CloseHandle(dWinThreadHandle);
}

static DWORD MainThreadFunction(void* pArg)
{
   CJThread* pThreadObject = (CJThread*)pArg;

   // Call the user function
   return pThreadObject->dpCallerThreadEntryFunction(pThreadObject->dpCallerThreadArgs);
}  


// This version uses the args that have been previously set using SetThreadArgs()
BOOL CJThread::InitThread( CJThreadConfig* pThreadConfig, U32(threadEntryFunction)(void*))
{
   return InitThread(pThreadConfig, threadEntryFunction, dpCallerThreadArgs);
}


BOOL CJThread::InitThread(CJThreadConfig* pThreadConfig, U32(threadEntryFunction)(void*), void* pArgs)
{
   if(dThreadState != eThreadState_Uninitialized)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "The thread state must be in the Uninitialized state (state=%d)", dThreadState);
      return FALSE;
   }


   dpCallerThreadEntryFunction = threadEntryFunction;
   dpCallerThreadArgs = pArgs;

   // Create the thread
   dWinThreadHandle = CreateThread(
      NULL,
      CJTHREAD_DEFAULT_STACK_SIZE,
      MainThreadFunction,
      this,
      0,  // Could create suspended here
      &dWinThreadId
      );


   if (dWinThreadHandle == NULL)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to create thread");
      return FALSE;
   }
   dThreadState = eThreadState_Running;
   return TRUE;
}


/*BOOL CJThread::SetThreadPriority( int priorityLevel)
{
   // TODO: NEED TO IMPLEMENT ON WINDOWS
   return FALSE;
   
   struct sched_param param;
   struct sched_param old_param;
   int old_policy;
   pthread_getschedparam( dThreadHandle, &old_policy, &old_param);
   CJTRACE(TRACE_ERROR_LEVEL, "Changing thread priority (old=%d, new=%d, policy=%d)", old_param, priorityLevel, old_policy);

   // sched_priority will be the priority of the thread
   param.sched_priority = priorityLevel;
   // only supported policy, others will result in ENOTSUP 
   // scheduling parameters of target thread
   int ret = pthread_setschedparam( dThreadHandle, SCHED_OTHER, &param);

   if( ret != 0)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to set thread priority (level=%d)", priorityLevel);
      return FALSE;
   }
   return TRUE;
   
}*/


BOOL CJThread::InitThreadAffinity() { return FALSE; }

/*BOOL CJThread::SetThreadAffinity()
{
   // TODO: NEED TO IMPLEMENT ON WINDOWS
   return FALSE;
   
   cpu_set_t cpu_affinity;
   //U32 tempTID = syscall(SYS_gettid);

   // Set the affinity
   if( dThreadParams.cpuAffinityMask != 0xFFFFFFFF)
   {
      CPU_ZERO(&cpu_affinity);
      U32 tempMask = 1;
      for( U32 i=0; i<32; i++)
      {
         if((dThreadParams.cpuAffinityMask & tempMask) != 0)
         {
            CPU_SET(i, &cpu_affinity);
         }
         tempMask = tempMask << 1;
      }
   
      CJTRACE(LOW_LEVEL, "Setting thread affinity (cpu_affinity=0x%x, handle=%u)", cpu_affinity.__bits[0], dThreadHandle);
   
      int aff_rc = sched_setaffinity(0, sizeof(cpu_set_t), &cpu_affinity);
      if( aff_rc != 0)
      {
         switch(aff_rc)
         {
         case EFAULT:
            CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to set thread affinity (rc=%d (EFAULT), cpu_affinity=0x%x, TID=%u)", aff_rc, cpu_affinity.__bits[0], syscall(SYS_gettid));
            return FALSE;
         case EINVAL:
            CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to set thread affinity (rc=%d (EINVAL), cpu_affinity=0x%x, TID=%u)", aff_rc, cpu_affinity.__bits[0], syscall(SYS_gettid));
            return FALSE;
         case EPERM:
            CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to set thread affinity (rc=%d (EPERM), cpu_affinity=0x%x, TID=%u)", aff_rc, cpu_affinity.__bits[0], syscall(SYS_gettid));
            return FALSE;
         case ESRCH:
            CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to set thread affinity (rc=%d (ESRCH), cpu_affinity=0x%x, TID=%u)", aff_rc, cpu_affinity.__bits[0], syscall(SYS_gettid));
            return FALSE;
         default:
            CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to set thread affinity (rc=%d (UNKONWN), cpu_affinity=0x%x, TID=%u)", aff_rc, cpu_affinity.__bits[0], syscall(SYS_gettid));
            return FALSE;
         }
      }
   }
   return TRUE;
   
}*/


BOOL CJThread::WaitForThreadExit(int timeout_ms)
{
   DWORD rc;

   if (dThreadState != eThreadState_Running)
   {
      return TRUE;
   }

   rc = WaitForSingleObject(dWinThreadHandle, timeout_ms);
   switch (rc)
   {
   case WAIT_ABANDONED:
      return FALSE;
   case WAIT_OBJECT_0:
      break;
   case WAIT_TIMEOUT:
      return FALSE;
   case WAIT_FAILED:
   default:
      return FALSE;
   }

   dThreadState = eThreadState_Exited;
   return TRUE;
}


void CJThread::UpdateThreadState()
{
   int rc;
   if( dThreadState == eThreadState_Running)
   {
      if( WaitForSingleObject(dWinThreadHandle, 0) == 0)
      {
         dThreadState = eThreadState_Exited;
      }
   }
}

