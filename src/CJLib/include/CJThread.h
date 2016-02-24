// CJThread.h : Header file
//

#ifndef __CJTHREAD_H_
#define __CJTHREAD_H_

#include <string>

#include "CJTypes.h"
#include "CJList.h"



enum CJTHREAD_SCHED
{
   eCJSCHEDULER_DEFAULT,
   eCJSCHEDULER_ROUND_ROBIN,
   eCJSCHEDULER_FIFO
};
#define CPU_AFFINITY_ALL_CPUS 0xFFFFFFFF
#define CPU_THREAD_PRIORITY_DEFAULT -1
#define CPU_THREAD_PRIORITY_REALTIME 49
#define PREFAULT_SAFE_STACK_SIZE (8*1024)

struct CJThreadConfig
{                                                     
   BOOL           setupAsRealTimeThread;    // If this is set, then stack pre-fault, FIFO scheduling, and priority=49 is set automatically.  To truly run preemtively, the kernel RT_PREEMPT features must be built in
   CJTHREAD_SCHED sched;                    // Sets the scheduling policy         
   int            priorityLevel;            // Can be set from 0-50, 50 is highest priority for kernel threads, 49 is for Real Time threads
   U32            cpuAffinityMask;          // 0x0000000F would indicate that the thread can execute on CPUs 0,1,2,3
};


class CJThread : public CJListElement
{
public:
   CJThread( char const* pThreadNameStr);
   ~CJThread( );

   BOOL InitThread(CJThreadConfig* pThreadConfig, U32(threadEntryFunction)(void*), void* pArgs);
   BOOL InitThread(CJThreadConfig* pThreadConfig, U32(threadEntryFunction)(void*));  // This version uses the args that have been previously set using SetThreadArgs()
   BOOL WaitForThreadExit(int timeout_ms=-1);  
   //void StopThread();

   void SetThreadArgs( void* pArgs) { dpCallerThreadArgs = pArgs;}
   void* GetThreadArgs() { return dpCallerThreadArgs;}

   enum ThreadState
   {
      eThreadState_Uninitialized,
      eThreadState_Exited,
      eThreadState_Running
   };
   ThreadState GetThreadState() {UpdateThreadState(); return dThreadState;}
   const char* GetThreadStateStr() 
   {
      UpdateThreadState();
      switch(dThreadState)
      {
      case eThreadState_Uninitialized: return "NoInit";
      case eThreadState_Exited:        return "Exited";
      case eThreadState_Running:       return "Running";
      default:                         return "Unknown";
      }
   }

   // Private Functions Used Internally (could use an interface class to clean these functions up and make them private)
   U32 GetAffinity() {return dThreadConfig.cpuAffinityMask;}
   CJThreadConfig* GetThreadConfig() { return &dThreadConfig;}
   void* GetReturnParamsPtr() {return dpReturnParams;}
   BOOL InitThreadAffinity();
   //BOOL InitThreadPriority( int priorityLevel);  // Not support for runtime priority changing now.  use raw linux calls.
   void PreFaultStackForRTThread();

   U32(*dpCallerThreadEntryFunction)(void*);
   void* dpCallerThreadArgs;

private:

   void UpdateThreadState();

   ThreadState     dThreadState;
   CJThreadConfig  dThreadConfig;
   void*           dpReturnParams;

#ifdef CJWINDOWS 
   HANDLE dWinThreadHandle;
   ULONG  dWinThreadId;
#else
   #include <pthread.h>
   pthread_attr_t  dLinuxThreadAttr;
   pthread_t       dLinuxThreadHandle;
#endif 



};


#endif // __CJTHREAD_H_
