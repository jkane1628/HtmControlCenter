// CJJob.cpp : Implementation file
//

#include <string.h>

#include "CJPool.h"
#include "CJQueue.h"
#include "CJSem.h"
#include "CJThread.h"
#include "CJTrace.h"
#include "CJAssert.h"

#include "CJJob.h"


U32 gJobThreadFunction( void* pArgs)
{
   JobThreadArgs* pJobThreadArgs = (JobThreadArgs*)pArgs;

   while(1)
   {
      pJobThreadArgs->runSem.Wait();
      if( pJobThreadArgs->shutdownFlag == TRUE)
      {
         break;
      }
      pJobThreadArgs->pActiveJob->StartJobUpdate();
      pJobThreadArgs->pActiveJob->MainFunction();
      pJobThreadArgs->pActiveJob->EndJobUpdate();
      pJobThreadArgs->pActiveJob = NULL;
      pJobThreadArgs->pJobManager->ContinueProcessingJobs( pJobThreadArgs->pAssignedThread);
   }
   return 0;
}


class CJJobQueue : public CJQueue
{
public:
   CJJobQueue( char const* name, U32 maxNumIO) : CJQueue(name,maxNumIO,FALSE) {}
   BOOL Queue( CJJob* pJob) { return CJQueue::Queue(pJob);}
   CJJob* Dequeue( int timeout_ms=-1) { return (CJJob*)CJQueue::Dequeue(timeout_ms);}
};


CJJobMgr::CJJobMgr( char const* pJobMgrNameStr, U32 maxNumActiveJobs, U32 maxNumQueuedJobs) : CJObject("LIB::JOBMGR::", pJobMgrNameStr),
   dMaxNumActiveJobs(maxNumActiveJobs),
   dMaxNumQueuedJobs(maxNumQueuedJobs)
{
   char tempName[32];
   CJThread* pTempThread;

   // Create the run queue for the job manager
   snprintf( tempName, 32, "%s_runQ", pJobMgrNameStr);
   dpJobQueue = new CJJobQueue(tempName, maxNumQueuedJobs);

   // Create a array job ptrs that is basically static, these will hold the job objects added to this mgr
   dppJobArray = new CJJob*[dMaxNumQueuedJobs];               
   dpLastAcquiredJob = NULL;     
   dTotalNumJobsAdded = 0;      

   // Create a list of threads that will execute these jobs
   snprintf( tempName, 32, "%s_threads", pJobMgrNameStr);
   snprintf( tempName, 32, "%s_freethr", pJobMgrNameStr);
   dpFreeThreadPool = new CJPool(pJobMgrNameStr, maxNumActiveJobs);
   dpThreadArgsArray = new JobThreadArgs[maxNumActiveJobs];

   for( U32 i=0; i < dMaxNumActiveJobs; i++)
   {
      snprintf( tempName, 32, "%s_%u", pJobMgrNameStr, i);
      pTempThread = new CJThread(tempName);

      // Setup the thread arguments
      dpThreadArgsArray[i].pJobManager = this;
      dpThreadArgsArray[i].pActiveJob = NULL;
      dpThreadArgsArray[i].pAssignedThread = pTempThread;
      dpThreadArgsArray[i].shutdownFlag = FALSE;
      pTempThread->SetThreadArgs( &dpThreadArgsArray[i]);

      // Start the actual thread
      if( !pTempThread->InitThread( NULL, gJobThreadFunction, &dpThreadArgsArray[i]))
      {
         CJASSERT("Failed to start job manager thread (index=%u)", i);
      }

      dpFreeThreadPool->AddElementToPool(pTempThread);
   }
}


CJJobMgr::~CJJobMgr( )
{

   ShutdownAllThreads(1000); // Give the threads 1 sec to shutdown

   for( U32 i=0; i < dMaxNumActiveJobs; i++)
   {
      delete dpThreadArgsArray[i].pAssignedThread;
   }
   delete []dpThreadArgsArray;
   delete dpFreeThreadPool;
   delete dpJobQueue;
   delete []dppJobArray;
}


BOOL CJJobMgr::ContinueProcessingJobs( CJThread* pCompletedThread)
{
   BOOL rc = FALSE; // This rc indicates if a job was started
   CJJob* pTempJob;
   CJThread* pTempThread;
   JobThreadArgs* pThreadArgs;

   if( pCompletedThread != NULL)
   {
      dpFreeThreadPool->ReleaseElement(pCompletedThread);
   }

   CJTRACE(TRACE_LOW_LEVEL, "ContinueProcessingJobs: FreeThreadPool=%u, JobQueue=%u", dpFreeThreadPool->GetFreeCount(), dpJobQueue->GetCount());

   // Get an available thread
   pTempThread = (CJThread*)dpFreeThreadPool->ReserveElement();
   while( pTempThread != NULL)
   {
      // Check to see if any jobs are queued
      pTempJob = dpJobQueue->Dequeue(0);
      if( pTempJob == NULL)
      {
         dpFreeThreadPool->ReleaseElement(pTempThread);
         CJTRACE(TRACE_LOW_LEVEL, "ContinueProcessingJobs: JobMgr Q Empty", dpFreeThreadPool->GetFreeCount(), dpJobQueue->GetCount());
         return rc;
      }

      // Assign the job to the thread and then trigger it to run
      pThreadArgs = (JobThreadArgs*)pTempThread->GetThreadArgs();
      pThreadArgs->pActiveJob = pTempJob;

      // Trigger the thread to run
      pThreadArgs->runSem.Post();

      rc = TRUE;

      pTempThread = (CJThread*)dpFreeThreadPool->ReserveElement();
   }
   return rc;
}


BOOL CJJobMgr::ShutdownAllThreads(int timeout_ms)
{
   U32 i;

   // Post to all the run sems
   for( i=0; i < dMaxNumActiveJobs; i++)
   {
      dpThreadArgsArray[i].shutdownFlag = TRUE;
      dpThreadArgsArray[i].runSem.Post();
   }

   // Wait on the thread to complete
   if( timeout_ms != 0)
   {
      for( i=0; i < dMaxNumActiveJobs; i++)
      {
         if( dpThreadArgsArray[i].pAssignedThread->WaitForThreadExit(timeout_ms) == FALSE)
         {
            return FALSE;
         }
      }
   }
   return TRUE;
}



BOOL CJJobMgr::QueueJob( CJJob* pJob)
{
   // Check to make sure the job has been properly acquired
   if( pJob->GetJobState() != eCJJOB_STATE_ACQUIRED)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: A job must be in the aquired state before it can be queued (name=%s)", pJob->GetJobName());
      return FALSE;
   }

   // Add the job to the queue
   dpJobQueue->Queue(pJob);

   // Attempt to start the job
   ContinueProcessingJobs(NULL);

   return TRUE;
}


BOOL CJJobMgr::AddNewJobToManager( CJJob* pNewJob)
{
   if( pNewJob == NULL)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to add job because it is NULL");
      return FALSE;
   }

   if( dTotalNumJobsAdded >= dMaxNumQueuedJobs)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to add job because the max number of queued jobs have already been added (max=%u)", dTotalNumJobsAdded);
      return FALSE;
   }

   if((pNewJob->GetJobState() & eCJJOB_STATE_FREE_MASK) == 0)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to add job because it is not in one of the eCJJOB_STATE_FREE_MASK states (state=%u)", pNewJob->GetJobState());
      return FALSE;
   }

   for( U32 i=0; i < dTotalNumJobsAdded; i++)
   {
      if( dppJobArray[i] == pNewJob)
      {
         CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to add job because it has already been added to this job manager");
         return FALSE;
      }
   }
   dppJobArray[dTotalNumJobsAdded] = pNewJob;
   dTotalNumJobsAdded++;
   return TRUE;
}


CJJob* CJJobMgr::GetNextJob( CJJob* pJob, JobState stateMask)
{
   BOOL currentJobFound = (pJob == NULL);

   for( U32 i=0; i < dTotalNumJobsAdded; i++)
   {
      // Skip all jobs until the passed in one is found
      if( currentJobFound == FALSE)
      {
         if( pJob == dppJobArray[i])
         {
            currentJobFound = TRUE;
         } 
         continue;
      }
      // Does this job match the mask
      if( dppJobArray[i]->GetJobState() & stateMask)
      {
         return dppJobArray[i];
      }
   }
   return NULL;
}


CJJob* CJJobMgr::AcquireFreeJob()
{
   CJJob* rcJob = GetNextJob( dpLastAcquiredJob, eCJJOB_STATE_FREE_MASK);
   if((rcJob == NULL) && (dpLastAcquiredJob != NULL))
   {
      // We still need to check the 'dpLastAcquiredJob'
      if((dpLastAcquiredJob->GetJobState() & eCJJOB_STATE_FREE_MASK) == 0)
      {
         return NULL;
      }
   }
   else
   {
      dpLastAcquiredJob = rcJob;
      dpLastAcquiredJob->SetJobState(eCJJOB_STATE_ACQUIRED);
   }
   dpLastAcquiredJob->SetJobState(eCJJOB_STATE_ACQUIRED);
   return dpLastAcquiredJob;
}

void CJJobMgr::UnacquireJob( CJJob* pJob)
{
   pJob->SetJobState(eCJJOB_STATE_NOINIT);
}


U32 CJJobMgr::GetJobCount( JobState stateMask)
{
   U32 jobCount=0;
   for( U32 i=0; i < dTotalNumJobsAdded; i++)
   {
      // Does this job match the mask
      if( dppJobArray[i]->GetJobState() & stateMask)
      {
         jobCount++;
      }
   }
   return jobCount;
}









CJJob::CJJob( char const* pName) : CJListObject("LIB::JOB::", pName)
{
   dState = eCJJOB_STATE_NOINIT;
   memset( dJobDescription, 0, 64);
   strncpy( dJobName, pName, 16);
   dAbortFlag = FALSE;
   dStartTimeStamp = 0;
   dEndTimeStamp = 0;
}


CJJob::~CJJob()
{}


void CJJob::StartJobUpdate()
{
   CJTRACE(TRACE_ERROR_LEVEL, "Starting job (name=%s)", GetJobName());
   dState = eCJJOB_STATE_RUNNING;
   dStartTimeStamp = gpTime->GetHighResTimestamp();
}

void CJJob::EndJobUpdate()
{
   dEndTimeStamp = gpTime->GetHighResTimestamp();
   dState = eCJJOB_STATE_DONE;
   CJTRACE(TRACE_ERROR_LEVEL, "Ending job (name=%s, time=%.2lf sec)", GetJobName(), GetElaspedSeconds());
}

double CJJob::GetElaspedSeconds()
{
   if( dEndTimeStamp == 0)
   {
      return gpTime->TicksToSeconds( gpTime->GetHighResTimestamp() - dStartTimeStamp); 
   }
   return gpTime->TicksToSeconds( dEndTimeStamp - dStartTimeStamp); 
}









