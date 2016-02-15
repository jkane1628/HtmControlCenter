// CJThread.cpp : Implementation file
//
#include <string.h>  // for memcpy
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>

#ifdef USE_DEFAULT_GLOBAL_THREAD_MANAGER
#include "CJThreadMgr.h"
#endif

#include "CJTrace.h"
#include "CJThread.h"


CJThread::CJThread( char const* pThreadNameStr) : CJListObject("LIB::THREAD::", pThreadNameStr)
{
   dThreadState = eThreadState_Uninitialized;
   dpReturnParams = NULL;

   dLinuxThreadHandle = 0;
   dThreadConfig.sched = eCJSCHEDULER_DEFAULT;
   dThreadConfig.cpuAffinityMask = CPU_AFFINITY_ALL_CPUS;

   if( pthread_attr_init(&dLinuxThreadAttr) != 0)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "Failed to init thread param object");
   }

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
         CJTRACE(TRACE_ERROR_LEVEL, "Thread is being deleted before it has stopped");
      }
      //if(dThreadState == eThreadState_Running)
      //{
      //   pthread_detach(dLinuxThreadHandle);
      //}
   }

#ifdef USE_DEFAULT_GLOBAL_THREAD_MANAGER
   if( gpThreadMgr->RemoveThread(this) == FALSE)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "Failed to remove the thread from the thread mgr");
   }
#endif

   pthread_attr_destroy(&dLinuxThreadAttr);
}

void* MainThreadFunction( void* pArg)
{
   CJThread* pThreadObject = (CJThread*)pArg;

   // Setup real time thread if needed
   CJThreadConfig* pConfig = pThreadObject->GetThreadConfig();

   if( pConfig->setupAsRealTimeThread)
   {
        // Lock memory
        if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) 
        {
            return (void*)-2;
        }

        // Pre-fault our stack
        pThreadObject->PreFaultStackForRTThread();
   }

   // Setup the affinity
   pThreadObject->InitThreadAffinity();

   // Call the user function
   return (void*)pThreadObject->dpCallerThreadEntryFunction(pThreadObject->dpCallerThreadArgs);
}  


// This version uses the args that have been previously set using SetThreadArgs()
BOOL CJThread::InitThread( CJThreadConfig* pThreadConfig, U32(threadEntryFunction)(void*))
{
   return InitThread( pThreadConfig, threadEntryFunction, dpCallerThreadArgs);
}

void CJThread::PreFaultStackForRTThread()
{
    unsigned char dummy[PREFAULT_SAFE_STACK_SIZE];
    memset(dummy, 0, PREFAULT_SAFE_STACK_SIZE);
}

BOOL CJThread::InitThread( CJThreadConfig* pThreadConfig, U32(threadEntryFunction)(void*), void* pArgs)
{
   if(dThreadState != eThreadState_Uninitialized)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "The thread state must be in the Uninitialized state (state=%d)", dThreadState);
      return FALSE;
   }

   // Pre-thread creation setup (scheduler algo, stack).  Affiniity and priority are setup in the thread itself.
   if( pThreadConfig != NULL)
   {
       memcpy(&dThreadConfig, pThreadConfig, sizeof(CJThreadConfig));
       if( pThreadConfig->setupAsRealTimeThread)
       {
           if ((dThreadConfig.priorityLevel != CPU_THREAD_PRIORITY_DEFAULT) && 
               (dThreadConfig.priorityLevel != CPU_THREAD_PRIORITY_DEFAULT))
           {
               CJTRACE(TRACE_ERROR_LEVEL, "RT thread setup requires priority 49, overiding thread config priority(49)\n");
           }
           if ((dThreadConfig.sched != eCJSCHEDULER_FIFO))
           {
               CJTRACE(TRACE_ERROR_LEVEL, "RT thread setup requires FIFO scheduling, overiding thread config scheduler setting\n");
           }
           dThreadConfig.priorityLevel = CPU_THREAD_PRIORITY_REALTIME;
           dThreadConfig.sched = eCJSCHEDULER_FIFO;
       }

       switch(dThreadConfig.sched)
       {
       case eCJSCHEDULER_ROUND_ROBIN:
          pthread_attr_setinheritsched(&dLinuxThreadAttr, PTHREAD_EXPLICIT_SCHED);
          pthread_attr_setschedpolicy(&dLinuxThreadAttr, SCHED_RR);
          break;
       case eCJSCHEDULER_FIFO:
          pthread_attr_setinheritsched(&dLinuxThreadAttr, PTHREAD_EXPLICIT_SCHED);
          pthread_attr_setschedpolicy(&dLinuxThreadAttr, SCHED_FIFO);
          break;
       default:
          break;
       }
   }

   dpCallerThreadEntryFunction = threadEntryFunction;
   dpCallerThreadArgs = pArgs;

   int rc = pthread_create( &dLinuxThreadHandle, &dLinuxThreadAttr, MainThreadFunction, this);

   switch(rc)
   {
   case 0:
      break;
   case EAGAIN: 
      CJTRACE(TRACE_ERROR_LEVEL, "pthread_create() returned an error, no linux resources (err=EAGAIN)");
      break;
   case EINVAL:
      CJTRACE(TRACE_ERROR_LEVEL, "pthread_create() returned an error, attributes were invalid (err=EINVAL)");
      break;
   case EPERM:
      CJTRACE(TRACE_ERROR_LEVEL, "pthread_create() returned an error, bad permissions (err=EPERM)");
      break;
   default:
      CJTRACE(TRACE_ERROR_LEVEL, "pthread_create() returned an unknown error (err=%d)", rc);
      break;
   }

   if( rc != 0)
   {
      return FALSE;
   }
   dThreadState = eThreadState_Running;
   return TRUE;
}


//BOOL CJThread::InitThreadPriority( int priorityLevel)
//{
//   struct sched_param param;
//   struct sched_param old_param;
//   int old_policy;
//   pthread_getschedparam( dLinuxThreadHandle, &old_policy, &old_param);
//   CJTRACE(TRACE_ERROR_LEVEL, "Changing thread priority (old=%d, new=%d, policy=%d)", old_param, priorityLevel, old_policy);
//
//   /* sched_priority will be the priority of the thread */
//   param.sched_priority = priorityLevel;
//   /* only supported policy, others will result in ENOTSUP */
//   /* scheduling parameters of target thread */
//   int ret = pthread_setschedparam( dLinuxThreadHandle, SCHED_OTHER, &param);
//
//   if( ret != 0)
//   {
//      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to set thread priority (level=%d)", priorityLevel);
//      return FALSE;
//   }
//   return TRUE;
//}


BOOL CJThread::InitThreadAffinity()
{
   cpu_set_t cpu_affinity;
   //U32 tempTID = syscall(SYS_gettid);

   // Set the affinity
   if( dThreadConfig.cpuAffinityMask != 0xFFFFFFFF)
   {
      CPU_ZERO(&cpu_affinity);
      U32 tempMask = 1;
      for( U32 i=0; i<32; i++)
      {
         if((dThreadConfig.cpuAffinityMask & tempMask) != 0)
         {
            CPU_SET(i, &cpu_affinity);
         }
         tempMask = tempMask << 1;
      }
   
      CJTRACE(TRACE_LOW_LEVEL, "Setting thread affinity (cpu_affinity=0x%x, handle=%u)", cpu_affinity.__bits[0], dLinuxThreadHandle);
   
      int aff_rc = sched_setaffinity(0, sizeof(cpu_set_t), &cpu_affinity);
      if( aff_rc != 0)
      {
         switch(aff_rc)
         {
         case EFAULT:
            CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to set thread affinity (rc=%d (EFAULT), cpu_affinity=0x%x, TID=%u)", aff_rc, cpu_affinity.__bits[0], 0);//syscall(SYS_gettid));
            return FALSE;
         case EINVAL:
            CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to set thread affinity (rc=%d (EINVAL), cpu_affinity=0x%x, TID=%u)", aff_rc, cpu_affinity.__bits[0], 0);// syscall(SYS_gettid));
            return FALSE;
         case EPERM:
            CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to set thread affinity (rc=%d (EPERM), cpu_affinity=0x%x, TID=%u)", aff_rc, cpu_affinity.__bits[0], 0);// syscall(SYS_gettid));
            return FALSE;
         case ESRCH:
            CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to set thread affinity (rc=%d (ESRCH), cpu_affinity=0x%x, TID=%u)", aff_rc, cpu_affinity.__bits[0], 0);// syscall(SYS_gettid));
            return FALSE;
         default:
            CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to set thread affinity (rc=%d (UNKONWN), cpu_affinity=0x%x, TID=%u)", aff_rc, cpu_affinity.__bits[0], 0);// syscall(SYS_gettid));
            return FALSE;
         }
      }
   }
   return TRUE;
}


BOOL CJThread::WaitForThreadExit(int timeout_ms)
{
   int rc;

   if( dThreadState != eThreadState_Running)
   {
      return TRUE;
   }

   if( timeout_ms == 0)
   {
      rc = pthread_tryjoin_np(dLinuxThreadHandle, &dpReturnParams);
      if( rc != 0)
      {
         return FALSE;
      }
   }
   else if(timeout_ms < 0)
   {
      rc = pthread_join( dLinuxThreadHandle, &dpReturnParams);
      if( rc != 0)
      {
         return FALSE;
      }
   }
   else
   {
      timespec timeout_val;
      if (clock_gettime(CLOCK_REALTIME, &timeout_val) != 0)
      {
         CJTRACE(TRACE_ERROR_LEVEL, "clock_gettime() failed when setting up timeout in WaitForThreadExit");
         return FALSE;
      }

      timeout_val.tv_sec += timeout_ms / 1000;
      timeout_val.tv_nsec += 1000000 * (timeout_ms % 1000); 

      rc = pthread_timedjoin_np(dLinuxThreadHandle, &dpReturnParams, &timeout_val);
      if( rc != 0)
      {
         return FALSE;
      }
   }

   dThreadState = eThreadState_Exited;
   return TRUE;
}


void CJThread::UpdateThreadState()
{
   int rc;
   if( dThreadState == eThreadState_Running)
   {
      rc = pthread_tryjoin_np(dLinuxThreadHandle, NULL);
      if( rc == 0)
      {
         dThreadState = eThreadState_Exited;
      }
   }
}

