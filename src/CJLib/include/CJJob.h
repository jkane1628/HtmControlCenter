// CJJob.h : Header file
//
//    The CJJob class is for background jobs that each have there own task.  The job manager holds information about
//  status of the jobs and some display functions for tracking the state of each job.

#ifndef __CJJOB_H_
#define __CJJOB_H_

#include <stdio.h>

#include "CJTypes.h"
#include "CJObject.h"
#include "CJTime.h"
#include "CJSem.h"

enum JobState
{
   eCJJOB_STATE_NOINIT    = 0x01,
   eCJJOB_STATE_ACQUIRED  = 0x02,
   eCJJOB_STATE_QUEUED    = 0x04,
   eCJJOB_STATE_RUNNING   = 0x08,
   eCJJOB_STATE_ABORTING  = 0x10,
   eCJJOB_STATE_DONE      = 0x20,

   eCJJOB_STATE_FREE_MASK = eCJJOB_STATE_NOINIT | eCJJOB_STATE_DONE,
   eCJJOB_STATE_ALL_MASK  = 0x7FFFFFFF
};


class CJJob : public CJListObject
{
public:
   CJJob( char const* objName);
   virtual ~CJJob();

   double       GetElaspedSeconds();
   char*        GetJobDescription() {return dJobDescription;}
   char*        GetJobName() {return dJobName;}
   JobState     GetJobState() { return dState;}
   void         SetJobState(JobState newState) { dState = newState;}
   void         StartJobUpdate();
   void         EndJobUpdate();


   virtual U32  GetPercentComplete() = 0;
   virtual void MainFunction() = 0;


private:
   JobState  dState;
   char      dJobDescription[64];
   char      dJobName[16];
   BOOL      dAbortFlag;

   HighResTicks_t dStartTimeStamp;
   HighResTicks_t dEndTimeStamp;

};


class CJJobMgr;

struct JobThreadArgs
{  
   CJJobMgr* pJobManager;
   CJJob*    pActiveJob;
   CJThread* pAssignedThread;
   CJSem     runSem;
   BOOL      shutdownFlag;
};

class CJJobQueue;
class CJPool; 

class CJJobMgr : public CJObject
{
public:
   CJJobMgr(char const* pJobMgrNameStr, U32 numSimultaneousJobs, U32 maxNumQueuedJobs);
   ~CJJobMgr( );


   BOOL      ShutdownAllThreads( int timeout_ms=-1);

   BOOL      QueueJob( CJJob* pJob);           

   // To use the job mananger, the caller must create the job object, then add them to the mananger.  
   // At shutdown time, the caller can get each job object and delete them.  Another option for the caller is
   // just to manage the pool of jobs themselves.
   BOOL      AddNewJobToManager( CJJob* pNewJob);
   CJJob*    GetNextJob( CJJob* pJob, JobState stateMask=eCJJOB_STATE_ALL_MASK);
   CJJob*    AcquireFreeJob();
   void      UnacquireJob( CJJob* pJob);

   U32       GetJobCount( JobState stateMask);
   U32       GetJobCount() { return dTotalNumJobsAdded;}

   // NOTE: The user of this class should never call this function.  It is used internally, but cannot be made private
   BOOL      ContinueProcessingJobs( CJThread* pCompletedThread);

private:

   CJList*        dpThreadList;
   CJPool*        dpFreeThreadPool;
   JobThreadArgs* dpThreadArgsArray;
   U32            dMaxNumActiveJobs;

   CJJobQueue* dpJobQueue;

   // Array of jobs that have been added to the manager, this could be a pool or list, but keep in mind 
   // that the jobs can only reside on one list or Q at at time.
   CJJob**   dppJobArray;
   CJJob*    dpLastAcquiredJob;
   U32       dTotalNumJobsAdded;
   U32       dMaxNumQueuedJobs;

};



#endif // __CJJOB_H_
