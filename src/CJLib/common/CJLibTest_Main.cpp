// CJLibTest_Main.cpp : Implementation file
//

#include <unistd.h>
#include <stdio.h>
#include <string.h>



#include "CJTrace.h"
#include "CJThread.h"
#include "CJList.h"
#include "CJPool.h"
#include "CJPoolBlocking.h"
#include "CJConsole.h"
#include "CJCli.h"
#include "CJPersistFile.h"
#include "CJJob.h"
#include "CJSocket.h"

#include "CJLibTest.h"

CJLibTest* CJLibTestCliCommandSet::dpLibTest = NULL;

CliCommand CJLibTestCliCommandSet::CJLibTestCommandSet[] =
{
   {"list_test",
    "Tests the linked list object",
    "list_test\n"\
    "   PARAMS:"\
    "     none",
    eCliAccess_Guest,
    CJLibTestCliCommandSet::CliCommand_ListTest},

   {"pool_test",
    "Tests the pool object",
    "pool_test\n"\
    "   PARAMS:"\
    "     none",
    eCliAccess_Guest,
    CJLibTestCliCommandSet::CliCommand_PoolTest},

   {"blockingpool_test",
    "Tests the Blocking pool object",
    "pool_test\n"\
    "   PARAMS:"\
    "     none",
    eCliAccess_Guest,
    CJLibTestCliCommandSet::CliCommand_BlockingPoolTest},

   {"thread_test",
    "Tests the thread and threadMgr objects",
    "list_test\n"\
    "   PARAMS:"\
    "     none",
    eCliAccess_Guest,
    CJLibTestCliCommandSet::CliCommand_ThreadTest},

   {"pfile_test", 
    "Tests the persitFile objects",
    "list_test\n"\
    "   PARAMS:"\
    "     none",
    eCliAccess_Guest,
    CJLibTestCliCommandSet::CliCommand_PFileTest},

   {"job_test", 
    "Tests the Job and JobMgr objects",
    "list_test\n"\
    "   PARAMS:"\
    "     none",
    eCliAccess_Guest,
    CJLibTestCliCommandSet::CliCommand_JobTest},

   {"socket_test", 
    "Tests the Socket object",
    "list_test\n"\
    "   PARAMS:"\
    "     none",
    eCliAccess_Guest,
    CJLibTestCliCommandSet::CliCommand_SocketTest},
   


   {"","","",eCliAccess_Hidden} // This must be the last command
};


int main(int argc, char *argv[])
{
   CJLibTest app;
   return app.Main(argc, argv);
}

CJLibTest::CJLibTest() : CJObject("APP","")
{
   CJTRACE_REGISTER_TRACE_OBJ(this);

   // Create Socket Console
   dpSocketConsole = new CJSocketConsole(("SkConsole"));

   // Create the Test Console
   dpConsole = new CJTerminalConsole("Console");

   // Create the Test CLI Command Set
   dpCliCommandSet = new CJLibTestCliCommandSet;
   dpCliCommandSet->dpLibTest = this;

   // Create the Test CLI and register the command set
   dpCli = new CJCli("Cli", dpConsole);
   dpCli->RegisterCliCommandSet( dpCliCommandSet->CJLibTestCommandSet);

   // Create the Telnet CLI and register the command set
   dpTelnetCli = new CJCli("TelnetCli", dpSocketConsole);
   dpTelnetCli->RegisterCliCommandSet( dpCliCommandSet->CJLibTestCommandSet);

   // Register the console
   //CJTRACE_REGISTER_CONSOLE(dpConsole);
   //CJTRACE_REGISTER_CONSOLE(dpSocketConsole);

   // Register the CLI object so debug tracing can be displayed
   //CJTRACE_REGISTER_TRACE_OBJ(dpCli);

   // Initialize the private test variables    
   dpLinkedList = NULL;
   dpPool = NULL;
   dpBlockingPool = NULL;
   dpClientSocket = NULL;
   dpServerSocket = NULL;
}

CJLibTest::~CJLibTest()
{
   delete dpTelnetCli;
   delete dpCli;
}

int
CJLibTest::Main(int argc, char *argv[])
{
   // TEST #1 - CLI, threads, and tracing (and sockets with telnet active)
   if( !dpTelnetCli->StartCliSession(FALSE))
   {
      printf("Failed to start console CLI\n");
      return 1;
   } 

   if( !dpCli->StartCliSession(FALSE))
   {
      printf("Failed to start console CLI\n");
      return 1;
   }   

   dpConsole->Printf("\n");
   dpConsole->Printf("*******************************************************************************\n");
   dpConsole->Printf("*  CJ Library Test Application\n");
   dpConsole->Printf("*******************************************************************************\n");
   dpConsole->Printf("Starting CJLib Test App...\n");
   dpConsole->Flush();

   while(TRUE) {usleep(10000000);}
   return 0;
}


///////////////////////////////////////////////////////////////////////////////////
// List Test
///////////////////////////////////////////////////////////////////////////////////

CliReturnCode
CJLibTestCliCommandSet::CliCommand_ListTest( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams)
{
   return dpLibTest->RunLinkedListTest();
}
CliReturnCode
CJLibTest::RunLinkedListTest()
{
   class TestObj : public CJListElement
   {
   public:
      char testStr[5];
   } element0, element1, element2, element3, element4, element5, element6;

   TestObj* pTempObj;

   strcpy( element0.testStr, "obj0");
   strcpy( element1.testStr, "obj1");
   strcpy( element2.testStr, "obj2");
   strcpy( element3.testStr, "obj3");
   strcpy( element4.testStr, "obj4");
   strcpy( element5.testStr, "obj5");
   strcpy( element6.testStr, "obj6");

   CJTRACE(TRACE_ERROR_LEVEL, "Starting a Linked List Test");

   // Create the list
   if( dpLinkedList != NULL)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "Deleting old list");
      delete dpLinkedList;
   }
   dpLinkedList = new CJList("test_list");
   CJTRACE_REGISTER_TRACE_OBJ(dpLinkedList);
   //dpLinkedList->SetTraceLevel(TRACE_DBG_LEVEL);
   
   // Add some objects
   CJTRACE(TRACE_ERROR_LEVEL, "Adding 5 Objects to List");
   dpLinkedList->AddHead(&element2);
   dpLinkedList->AddTail(&element3);
   dpLinkedList->AddHead(&element1);
   dpLinkedList->AddTail(&element4);
   dpLinkedList->AddTail(&element5);

   // Display list
   CJTRACE(TRACE_ERROR_LEVEL, "Display list from Tail to Head (num_objects=%u)", dpLinkedList->GetNumElements());
   pTempObj = (TestObj*)dpLinkedList->GetTail();
   while(pTempObj)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "OBJECT: %s (%p)", pTempObj->testStr, pTempObj);
      pTempObj = (TestObj*)dpLinkedList->GetNext(pTempObj);
   }

   // Remove some objects
   CJTRACE(TRACE_ERROR_LEVEL, "Removing obj4 and Head(obj1)");
   if( dpLinkedList->Remove(&element4) == FALSE) { CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Remove() failed (element4)");}
   if( dpLinkedList->RemoveHead() == FALSE) { CJTRACE(TRACE_ERROR_LEVEL, "ERROR: RemoveTail() failed");}

   // Display list
   CJTRACE(TRACE_ERROR_LEVEL, "Display list from Tail to Head (num_objects=%u)", dpLinkedList->GetNumElements());
   pTempObj = (TestObj*)dpLinkedList->GetTail();
   while(pTempObj)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "OBJECT: %s (%p)", pTempObj->testStr, pTempObj);
      pTempObj = (TestObj*)dpLinkedList->GetNext(pTempObj);
   }

   // Insert objects back
   CJTRACE(TRACE_ERROR_LEVEL, "Inserting obj4 and obj1 back to list");
   if( dpLinkedList->InsertAfter( &element2, &element1) == FALSE) { CJTRACE(TRACE_ERROR_LEVEL, "ERROR: InsertAfter() failed (element2 after element1)");}
   if( dpLinkedList->InsertBefore( &element3, &element4) == FALSE) { CJTRACE(TRACE_ERROR_LEVEL, "ERROR: InsertBefore() failed (element4 before element3)");}

   CJTRACE(TRACE_ERROR_LEVEL, "Inserting obj0 and obj6 to list");
   if( dpLinkedList->InsertAfter( &element1, &element0) == FALSE) { CJTRACE(TRACE_ERROR_LEVEL, "ERROR: InsertAfter() failed (element0 after element1)");}
   if( dpLinkedList->InsertBefore( &element5, &element6) == FALSE) { CJTRACE(TRACE_ERROR_LEVEL, "ERROR: InsertBefore() failed (element6 before element5)");}

   // Display list
   CJTRACE(TRACE_ERROR_LEVEL, "Display list from Head to Tail (num_objects=%u)", dpLinkedList->GetNumElements());
   pTempObj = (TestObj*)dpLinkedList->GetHead();
   while(pTempObj)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "OBJECT: %s (%p)", pTempObj->testStr, pTempObj);
      pTempObj = (TestObj*)dpLinkedList->GetPrev(pTempObj);
   }
   CJTRACE(TRACE_ERROR_LEVEL, "Display list from Tail to Head (num_objects=%u)", dpLinkedList->GetNumElements());
   pTempObj = (TestObj*)dpLinkedList->GetTail();
   while(pTempObj)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "OBJECT: %s (%p)", pTempObj->testStr, pTempObj);
      pTempObj = (TestObj*)dpLinkedList->GetNext(pTempObj);
   }

   return eCliReturn_Success;

}




///////////////////////////////////////////////////////////////////////////////////
// Pool Test
///////////////////////////////////////////////////////////////////////////////////


CliReturnCode
CJLibTestCliCommandSet::CliCommand_PoolTest( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams)
{
   return dpLibTest->RunPoolTest();
}
CliReturnCode
CJLibTest::RunPoolTest() 
{
   class PoolTestObj
   {
   public:
      char testStr[5];
   } element0, element1, element2, element3, element4, element5;

   PoolTestObj *pReservedObj1=NULL,*pReservedObj2=NULL,*pReservedObj3=NULL, *pReservedObj4=NULL,*pReservedObj5=NULL,*pReservedObj6=NULL;

   strcpy( element0.testStr, "Obj0");
   strcpy( element1.testStr, "Obj1");
   strcpy( element2.testStr, "Obj2");
   strcpy( element3.testStr, "Obj3");
   strcpy( element4.testStr, "Obj4");
   strcpy( element5.testStr, "Obj5");

   CJTRACE(TRACE_ERROR_LEVEL, "Starting a Pool Test");

   // Create the pool
   if( dpPool != NULL)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "Deleting old pool");
      delete dpPool;
   }
   dpPool = new CJPool("test_pool", 5);  // 5 max object
   CJTRACE_REGISTER_TRACE_OBJ(dpPool);
   dpPool->SetTraceLevel(TRACE_DBG_LEVEL);

   // Add some objects
   CJTRACE(TRACE_ERROR_LEVEL, "Adding 5 objects to pool");
   if( dpPool->AddElementToPool(&element0) == FALSE)
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to add object0 to pool");
   if( dpPool->AddElementToPool(&element1) == FALSE)
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to add object1 to pool");
   if( dpPool->AddElementToPool(&element2) == FALSE)
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to add object2 to pool");
   if( dpPool->AddElementToPool(&element3) == FALSE)
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to add object3 to pool");
   if( dpPool->AddElementToPool(&element4) == FALSE)
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to add object4 to pool");


   CJTRACE(TRACE_ERROR_LEVEL, "Adding 1 objects to pool, this should fail because max objects is 5");
   if( dpPool->AddElementToPool(&element5) == FALSE)
      CJTRACE(TRACE_ERROR_LEVEL, "SUCCESS: Failed to add object4 to pool");


   CJTRACE(TRACE_ERROR_LEVEL, "Reserving 3 objects (total/resv/free=%u/%u/%u)", dpPool->GetTotalCount(), dpPool->GetReservedCount(), dpPool->GetFreeCount());
   pReservedObj1 = (PoolTestObj*)dpPool->ReserveElement();
   pReservedObj2 = (PoolTestObj*)dpPool->ReserveElement();
   pReservedObj3 = (PoolTestObj*)dpPool->ReserveElement();
   CJTRACE(TRACE_ERROR_LEVEL, "Reserved (1=%s, 2=%s, 3=%s)", pReservedObj1->testStr,pReservedObj2->testStr,pReservedObj3->testStr);

   CJTRACE(TRACE_ERROR_LEVEL, "Reserving 2 more objects (total/resv/free=%u/%u/%u)", dpPool->GetTotalCount(), dpPool->GetReservedCount(), dpPool->GetFreeCount());
   pReservedObj4 = (PoolTestObj*)dpPool->ReserveElement();
   pReservedObj5 = (PoolTestObj*)dpPool->ReserveElement();
   CJTRACE(TRACE_ERROR_LEVEL, "Reserved (4=%s, 5=%s)", pReservedObj4->testStr,pReservedObj5->testStr);

   CJTRACE(TRACE_ERROR_LEVEL, "Reserving 1 more objects that is not available(total/resv/free=%u/%u/%u)", dpPool->GetTotalCount(), dpPool->GetReservedCount(), dpPool->GetFreeCount());
   pReservedObj6 = (PoolTestObj*)dpPool->ReserveElement();
   if( pReservedObj6 == NULL)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "SUCCESS: Reserved couldn't reserve from the pool");
   }

   CJTRACE(TRACE_ERROR_LEVEL, "Release object1 (total/resv/free=%u/%u/%u)", dpPool->GetTotalCount(), dpPool->GetReservedCount(), dpPool->GetFreeCount());
   if( dpPool->ReleaseElement(pReservedObj1) == FALSE)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to release obj1");
   }


   //CJTRACE(TRACE_ERROR_LEVEL, "Release all the objects (total/resv/free=%u/%u/%u)", dpPool->GetTotalCount(), dpPool->GetReservedCount(), dpPool->GetFreeCount());
   //dpPool->ReleaseAllElements();
   CJTRACE(TRACE_ERROR_LEVEL, "Done (total/resv/free=%u/%u/%u)", dpPool->GetTotalCount(), dpPool->GetReservedCount(), dpPool->GetFreeCount());

   return eCliReturn_Success;
}

///////////////////////////////////////////////////////////////////////////////////
// Blocking Pool Test
///////////////////////////////////////////////////////////////////////////////////


CliReturnCode
CJLibTestCliCommandSet::CliCommand_BlockingPoolTest( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams)
{
   return dpLibTest->RunBlockingPoolTest();
}
CliReturnCode
CJLibTest::RunBlockingPoolTest() 
{
   class BlockingPoolTestObj
   {
   public:
      char testStr[5];
   } element0, element1, element2, element3, element4, element5;

   BlockingPoolTestObj *pReservedObj1,*pReservedObj2,*pReservedObj3, *pReservedObj4,*pReservedObj5;//,*pReservedObj6;

   strcpy( element0.testStr, "Obj0");
   strcpy( element1.testStr, "Obj1");
   strcpy( element2.testStr, "Obj2");
   strcpy( element3.testStr, "Obj3");
   strcpy( element4.testStr, "Obj4");
   strcpy( element5.testStr, "Obj5");

   CJTRACE(TRACE_ERROR_LEVEL, "Starting a Pool Test");

   // Create the pool
   if( dpBlockingPool != NULL)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "Deleting old Blocking pool");
      delete dpBlockingPool;
   }
   dpBlockingPool = new CJPoolBlocking("test_blockingpool", 5);  // 5 max object
   CJTRACE_REGISTER_TRACE_OBJ(dpBlockingPool);
   dpBlockingPool->SetTraceLevel(TRACE_DBG_LEVEL);

   // Add some objects
   CJTRACE(TRACE_ERROR_LEVEL, "Adding 5 objects to pool");
   if( dpBlockingPool->AddElementToPool(&element0) == FALSE)
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to add object0 to pool");
   if( dpBlockingPool->AddElementToPool(&element1) == FALSE)
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to add object1 to pool");
   if( dpBlockingPool->AddElementToPool(&element2) == FALSE)
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to add object2 to pool");
   if( dpBlockingPool->AddElementToPool(&element3) == FALSE)
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to add object3 to pool");
   if( dpBlockingPool->AddElementToPool(&element4) == FALSE)
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to add object4 to pool");


   CJTRACE(TRACE_ERROR_LEVEL, "Adding 1 objects to pool, this should fail because max objects is 5");
   if( dpBlockingPool->AddElementToPool(&element5) == FALSE)
      CJTRACE(TRACE_ERROR_LEVEL, "SUCCESS: Failed to add object4 to pool");


   CJTRACE(TRACE_ERROR_LEVEL, "Reserving 3 objects (total/resv/free=%u/%u/%u)", dpBlockingPool->GetTotalCount(), dpBlockingPool->GetReservedCount(), dpBlockingPool->GetFreeCount());
   pReservedObj1 = (BlockingPoolTestObj*)dpBlockingPool->ReserveElement();
   pReservedObj2 = (BlockingPoolTestObj*)dpBlockingPool->ReserveElement();
   pReservedObj3 = (BlockingPoolTestObj*)dpBlockingPool->ReserveElement();
   CJTRACE(TRACE_ERROR_LEVEL, "Reserved (1=%s, 2=%s, 3=%s)", pReservedObj1->testStr,pReservedObj2->testStr,pReservedObj3->testStr);

   CJTRACE(TRACE_ERROR_LEVEL, "Reserving 2 more objects (total/resv/free=%u/%u/%u)", dpBlockingPool->GetTotalCount(), dpBlockingPool->GetReservedCount(), dpBlockingPool->GetFreeCount());
   pReservedObj4 = (BlockingPoolTestObj*)dpBlockingPool->ReserveElement();
   pReservedObj5 = (BlockingPoolTestObj*)dpBlockingPool->ReserveElement();
   CJTRACE(TRACE_ERROR_LEVEL, "Reserved (4=%s, 5=%s)", pReservedObj4->testStr,pReservedObj5->testStr);

   //CJTRACE(TRACE_ERROR_LEVEL, "Reserving 1 more objects that is not available(total/resv/free=%u/%u/%u)", dpBlockingPool->GetTotalCount(), dpBlockingPool->GetReservedCount(), dpBlockingPool->GetFreeCount());
   //pReservedObj6 = (BlockingPoolTestObj*)dpBlockingPool->ReserveElement();
   //if( pReservedObj6 == NULL)
   //{
   //   CJTRACE(TRACE_ERROR_LEVEL, "SUCCESS: Reserve couldn't reserve from the pool");
   //}

   CJTRACE(TRACE_ERROR_LEVEL, "Release object1 (total/resv/free=%u/%u/%u)", dpBlockingPool->GetTotalCount(), dpBlockingPool->GetReservedCount(), dpBlockingPool->GetFreeCount());
   if( dpBlockingPool->ReleaseElement(pReservedObj1) == FALSE)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to release obj1");
   }


   //CJTRACE(TRACE_ERROR_LEVEL, "Release all the objects (total/resv/free=%u/%u/%u)", dpBlockingPool->GetTotalCount(), dpBlockingPool->GetReservedCount(), dpBlockingPool->GetFreeCount());
   //dpBlockingPool->ReleaseAllElements();
   CJTRACE(TRACE_ERROR_LEVEL, "Done (total/resv/free=%u/%u/%u)", dpBlockingPool->GetTotalCount(), dpBlockingPool->GetReservedCount(), dpBlockingPool->GetFreeCount());

   return eCliReturn_Success;
}




///////////////////////////////////////////////////////////////////////////////////
// Thread Test
///////////////////////////////////////////////////////////////////////////////////


struct TestThreadParams
{
   CJConsole* pConsole;
   BOOL runState;
};

U32 gTestThreadFunction0( void* pParam)
{
   TestThreadParams* pParams = (TestThreadParams*)pParam;
   while(pParams->runState)
   {
      sleep(1); pParams->pConsole->Printf("Mesage from Thread 0...\n");
   }
   return 0;
}
U32 gTestThreadFunction1( void* pParam)
{
   TestThreadParams* pParams = (TestThreadParams*)pParam;
   while(pParams->runState)
   {
      sleep(1); pParams->pConsole->Printf("Mesage from Thread 1...\n");
   }
   return 0;
}
U32 gTestThreadFunction2( void* pParam)
{
   TestThreadParams* pParams = (TestThreadParams*)pParam;
   while(pParams->runState)
   {
      sleep(1); pParams->pConsole->Printf("Mesage from Thread 2...\n");
   }
   return 0;
}


CliReturnCode
CJLibTestCliCommandSet::CliCommand_ThreadTest( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams)
{
   return dpLibTest->RunThreadTest(pConsole);
}
CliReturnCode
CJLibTest::RunThreadTest(CJConsole* pConsole)
{
   int i;
   CJThread* pThreadArray[5];
   char tempThreadNameStr[32];
   TestThreadParams paramArray[5];

   // Create 5 Threads
   CJTRACE(TRACE_ERROR_LEVEL, "Creating 5 Threads that will simply enter a trace msg every second");
   for( i=0; i<5; i++)
   {
      sprintf( tempThreadNameStr, "TestThread_%d", i); 
      pThreadArray[i] = new CJThread(tempThreadNameStr);   
   }

   // Init and run 3 Threads
   CJTRACE(TRACE_ERROR_LEVEL, "Init and start thread 0");
   paramArray[0].pConsole = pConsole;
   paramArray[0].runState = TRUE;
   if( pThreadArray[0]->InitThread( NULL, gTestThreadFunction0, &paramArray[0]) == FALSE)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to init thread 0");
   }
   CJTRACE(TRACE_ERROR_LEVEL, "Init and start thread 1");
   paramArray[1].pConsole = pConsole;
   paramArray[1].runState = TRUE;
   if( pThreadArray[1]->InitThread( NULL, gTestThreadFunction1, &paramArray[1]) == FALSE)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to init thread 1");
   }
   CJTRACE(TRACE_ERROR_LEVEL, "Init and start thread 2");
   paramArray[2].pConsole = pConsole;
   paramArray[2].runState = TRUE;
   if( pThreadArray[2]->InitThread( NULL, gTestThreadFunction2, &paramArray[2]) == FALSE)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to init thread 2");
   }

   CJTRACE(TRACE_ERROR_LEVEL, "Print Thread States:");
   for( i=0; i<5; i++)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "Thread %d = %s", i, pThreadArray[i]->GetThreadStateStr());
   }

   CJTRACE(TRACE_ERROR_LEVEL, "Let the three thread run for 3 seconds");
   sleep(3);

   CJTRACE(TRACE_ERROR_LEVEL, "Stop threads 0 and 2");
   paramArray[0].runState = FALSE;
   paramArray[2].runState = FALSE;

   CJTRACE(TRACE_ERROR_LEVEL, "Let the last thread run for 5 seconds");
   sleep(5);

   CJTRACE(TRACE_ERROR_LEVEL, "Stop thread 1");
   paramArray[1].runState = FALSE;
   
   CJTRACE(TRACE_ERROR_LEVEL, "Wait for all thread 0 to exit");
   if( pThreadArray[0]->WaitForThreadExit() == FALSE)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Thread 0 failed to exit");
   }
   CJTRACE(TRACE_ERROR_LEVEL, "Wait for all thread 1 to exit");
   if( pThreadArray[1]->WaitForThreadExit() == FALSE)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Thread 0 failed to exit");
   }
   CJTRACE(TRACE_ERROR_LEVEL, "Wait for all thread 2 to exit");
   if( pThreadArray[2]->WaitForThreadExit() == FALSE)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Thread 0 failed to exit");
   }

   CJTRACE(TRACE_ERROR_LEVEL, "Print Thread States:");
   for( i=0; i<5; i++)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "Thread %d = %s", i, pThreadArray[i]->GetThreadStateStr());
   }

   // Delete the threads
   CJTRACE(TRACE_ERROR_LEVEL, "Deleting all threads");
   for( i=0; i<5; i++)
   {
      delete pThreadArray[i];   
   }


   return eCliReturn_Success;
}





///////////////////////////////////////////////////////////////////////////////////
// Persistent File Test
///////////////////////////////////////////////////////////////////////////////////



CliReturnCode
CJLibTestCliCommandSet::CliCommand_PFileTest( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams)
{
   return dpLibTest->RunPFileTest();
}
CliReturnCode
CJLibTest::RunPFileTest() 
{
   int tempParentIndex, tempChildIndex;

   CJTRACE(TRACE_ERROR_LEVEL, "Creating a persistent file, add params, save, then load");

   enum ExampleTagIdEnum
   {
      eEX_NONE,  // The Zero tag id value must be reserved for top level tags
   
      eEX_MULTI_TAGID,
      eEX_32CHAR_STRING,

      eEX_PARENTTAGID,
      eEX_CHILD_MULTI_U64_TAGID,
      eEX_CHILD_CHAR1_TAGID,

      eEX_MULTI_PARENTTAGID,
      eEX_CHILD_U8_TAGID,
      eEX_CHILD_CHAR2_TAGID,

      eEX_BLANK_TAGID

   };

   PersistentTableElement ExampleParentTagDescriptorTable[] = {
   
      // DATA TYPE                   TAG STRING               TAG ID,                           MAX NUM PER FILE    PARENT TAG ID
      // -----------------------     -----------------------  -------------------------------   ----------------    ------------------        
      {  ePERSISTTYPE_S64,           "exampleMultiTag",       eEX_MULTI_TAGID,                  3,                  eEX_NONE     },
      {  ePERSISTTYPE_32CHAR_STRING, "example32String",       eEX_32CHAR_STRING,                1,                  eEX_NONE     },
                                                                                                                                         
      {  ePERSISTTYPE_PARENT_TAG,    "exampleParentTag",      eEX_PARENTTAGID,                  1,                  eEX_NONE     },
      {    ePERSISTTYPE_S64,         "someChildU64Tag",       eEX_CHILD_MULTI_U64_TAGID,        5,/*per parent tag*/eEX_PARENTTAGID  },
      {    ePERSISTTYPE_CHAR,        "someChildCharTag",      eEX_CHILD_CHAR1_TAGID,            1,                  eEX_PARENTTAGID  },
                                                                                                 
      {  ePERSISTTYPE_PARENT_TAG,    "multiParentTag",        eEX_MULTI_PARENTTAGID,            1,                  eEX_NONE               },
      {    ePERSISTTYPE_S64,         "someChildU8Tag",        eEX_CHILD_U8_TAGID,               3,/*per parent tag*/eEX_MULTI_PARENTTAGID  },
      {    ePERSISTTYPE_CHAR,        "someChildCharTag",      eEX_CHILD_CHAR2_TAGID,            1,                  eEX_MULTI_PARENTTAGID  },
                                                                                                 
      {  ePERSISTTYPE_U16,           "BlankTag",              eEX_BLANK_TAGID,                  1,                  eEX_NONE               },
                                     
  //    {  ePERSISTTYPE_PARENT_TAG,    "invalidParentTag",      eEX_PARENTTAGID,                  1,                  eEX_MULTI_PARENTTAGID  },

      {  ePERSISTTYPE_LAST_ENTRY,    "", 0, 0, 0} 
   };

   CJPersistFile* pPFile = new CJPersistFile("TestFile");

   if( pPFile->Initialize( "TestFile", ExampleParentTagDescriptorTable) == FALSE)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to initialize TestFile.xml");
      delete pPFile;
      return eCliReturn_Failed;
   }

   // Set 3 "exampleMultiTag"
   for( tempParentIndex=0; tempParentIndex < 3; tempParentIndex++)
   {
      if( pPFile->SetS64Param( 0x1234123499998888LL, eEX_MULTI_TAGID, tempParentIndex) == FALSE)
      {
         CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to set eEX_MULTI_TAGID");
      }   
   }

   // Set the 32char string
   if( pPFile->Set32CharStrParam( "Sample 32 Char String", eEX_32CHAR_STRING) == FALSE)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to set eEX_32CHAR_STRING");
   }   

   // Set 4 someChildU64Tag
   for( tempChildIndex=0; tempChildIndex < 4; tempChildIndex++)
   {
      if( pPFile->SetU64Param( tempChildIndex*10000, eEX_PARENTTAGID, 0, eEX_CHILD_MULTI_U64_TAGID, tempChildIndex) == FALSE)
      {
         CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to set eEX_CHILD_MULTI_U64_TAGID");
      }   
   }

   // Set "someChildCharTag"
   if( pPFile->SetCharParam( 'a', eEX_PARENTTAGID, 0, eEX_CHILD_CHAR1_TAGID, 0) == FALSE)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to set eEX_CHILD_CHAR_TAGID");
   }   


   // Set multi parent tag values
  // for( int tempParentIndex=0; tempParentIndex < 3; tempParentIndex++)
  // {
  //    for( tempChildIndex=0; tempChildIndex < 4; tempChildIndex++)
  //    {
  //    }
  // }


   // Save the file
   if( pPFile->SaveFile() == FALSE)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to save Persistent File");
   }


   delete pPFile;
   return eCliReturn_Success;


}


///////////////////////////////////////////////////////////////////////////////////
// Job Manager Test
///////////////////////////////////////////////////////////////////////////////////



class TestJob1 : public CJJob
{
public:
   TestJob1() : CJJob("3SecTestJob1"){}
   U32  GetPercentComplete() {return 50;}
   void MainFunction() {sleep(3);}
};
class TestJob2 : public CJJob
{
public:
   TestJob2() : CJJob("4SecTestJob2"){}
   U32  GetPercentComplete() {return 25;}
   void MainFunction() {sleep(4);}
};
class TestJob3 : public CJJob
{
public:
   TestJob3() : CJJob("5SecTestJob3"){}
   U32  GetPercentComplete() {return 26;}
   void MainFunction() {sleep(5);}
};
class TestJob4 : public CJJob
{
public:
   TestJob4() : CJJob("1SecTestJob4"){}
   U32  GetPercentComplete() {return 27;}
   void MainFunction() {sleep(1);}
};
class TestJob5 : public CJJob
{
public:
   TestJob5() : CJJob("1SecTestJob5"){}
   U32  GetPercentComplete() {return 28;}
   void MainFunction() {sleep(1);}
};


CliReturnCode
CJLibTestCliCommandSet::CliCommand_JobTest( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams)
{
   return dpLibTest->RunJobTest();
}
CliReturnCode
CJLibTest::RunJobTest() 
{

   // Create 5 Jobs and a job manager
   CJJobMgr* pJobManager = new CJJobMgr( "TestJobManager", 2, 12);
   TestJob1 job1, job1b;
   TestJob2 job2, job2b;
   TestJob3 job3, job3b;
   TestJob4 job4, job4b;
   TestJob5 job5, job5b;

   pJobManager->SetTraceLevel(TRACE_LOW_LEVEL);

   // Start 5 Jobs and register them with the job manager
   if( !pJobManager->AddNewJobToManager(&job1) || 
       !pJobManager->AddNewJobToManager(&job2) || 
       !pJobManager->AddNewJobToManager(&job3) || 
       !pJobManager->AddNewJobToManager(&job4) || 
       !pJobManager->AddNewJobToManager(&job5))
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to AddNewJobToManager()");
      delete pJobManager;
      return eCliReturn_Failed;
   }   

   CJTRACE(TRACE_ERROR_LEVEL, "Queuing 5 jobs...");
   if( !pJobManager->QueueJob(&job1)) CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to Queue job1");
   if( !pJobManager->QueueJob(&job2)) CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to Queue job2");
   if( !pJobManager->QueueJob(&job3)) CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to Queue job3");
   if( !pJobManager->QueueJob(&job4)) CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to Queue job4");
   if( !pJobManager->QueueJob(&job5)) CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to Queue job5");

   CJTRACE(TRACE_ERROR_LEVEL, "Waiting for jobs to finish (6sec)");
   sleep(6);

   if( !pJobManager->AddNewJobToManager(&job1b) || 
       !pJobManager->AddNewJobToManager(&job2b) || 
       !pJobManager->AddNewJobToManager(&job3b) || 
       !pJobManager->AddNewJobToManager(&job4b) || 
       !pJobManager->AddNewJobToManager(&job5b))
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to AddNewJobToManager()");
      delete pJobManager;
      return eCliReturn_Failed;
   }  

   CJTRACE(TRACE_ERROR_LEVEL, "Queuing 5 jobs again...");
   if( !pJobManager->QueueJob(&job1b)) CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to Queue job1b");
   if( !pJobManager->QueueJob(&job2b)) CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to Queue job2b");
   if( !pJobManager->QueueJob(&job3b)) CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to Queue job3b");
   if( !pJobManager->QueueJob(&job4b)) CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to Queue job4b");
   if( !pJobManager->QueueJob(&job5b)) CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to Queue job5b");

   CJTRACE(TRACE_ERROR_LEVEL, "Waiting for jobs to finish (25 sec)");
   sleep(25);

   if( !pJobManager->ShutdownAllThreads())
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: ShutdownAllThreads() failed");
   }
   delete pJobManager;
   return eCliReturn_Success;
}


///////////////////////////////////////////////////////////////////////////////////
// Socket Test
///////////////////////////////////////////////////////////////////////////////////


struct ServerThreadParams
{
   CJConsole* pConsole;
   CJSocket*  pServerSocket;
};

ServerThreadParams gServerThreadParams;

U32 gServerThreadFunction( void* pParam)
{
    char dataBuffer[256];
    int  numBytes;

    ServerThreadParams* pParams = (ServerThreadParams*)pParam;
    pParams->pServerSocket->AcceptConnection();

    numBytes = pParams->pServerSocket->ReceiveData( dataBuffer, 256);
    pParams->pConsole->Printf("Message received on server socket (%d bytes): %s\n", numBytes, dataBuffer);
    if( numBytes != sizeof("Hello Server"))
    {
        pParams->pConsole->Printf("ERROR: Wrong size of data received from client\n");
    }
    
    pParams->pConsole->Printf("Sending message to client using server socket\n");
    sprintf(dataBuffer, "Hello Client");
    if( pParams->pServerSocket->SendData(dataBuffer, sizeof("Hello Client")) != sizeof("Hello Client"))
    {
        pParams->pConsole->Printf("ERROR: Failed to send data from server socket\n");
    }
    return 0;
}

CliReturnCode
CJLibTestCliCommandSet::CliCommand_SocketTest( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams)
{
   return dpLibTest->RunSocketTest();
}
CliReturnCode
CJLibTest::RunSocketTest() 
{
    CliReturnCode rc = eCliReturn_Success;
    char dataBuffer[256];
    int  numBytes;
    int  portNumber = 41000;

    dpClientSocket = new CJSocket("TstClient");
    dpServerSocket = new CJSocket("TstServer");

    if( dpClientSocket->InitializeClientSocket() != eSocket_Success)
    {
        delete dpClientSocket;
        delete dpServerSocket;
        return eCliReturn_Failed;
    }
    if( dpServerSocket->InitializeServerSocket(portNumber) != eSocket_Success)
    {
        delete dpClientSocket;
        delete dpServerSocket;
        return eCliReturn_Failed;
    }

   CJThread* pSvrThread = new CJThread("ServerSocketThread");   

   // Init Server Thread
   dpConsole->Printf("Init server socket test thread");
   gServerThreadParams.pConsole = dpConsole;
   gServerThreadParams.pServerSocket = dpServerSocket;
   if( pSvrThread->InitThread( NULL, gServerThreadFunction, &gServerThreadParams) == FALSE)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to init thread 0");
   }

   if( dpClientSocket->Connect((char*)"localhost", portNumber) != eSocket_Success)
   {
       CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to connect to server socket");
       delete dpClientSocket;
       delete dpServerSocket;
       return eCliReturn_Failed;
   }

   dpConsole->Printf("Sending message to server using client socket\n");
   sprintf(dataBuffer, "Hello Server");
   if( dpClientSocket->SendData(dataBuffer, sizeof("Hello Server")) != sizeof("Hello Server"))
   {
       CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to send data from client socket");
       rc = eCliReturn_Failed;
   }

   numBytes = dpClientSocket->ReceiveData( dataBuffer, 256);
    
   dpConsole->Printf("Message received on client socket (%d bytes): %s\n", numBytes, dataBuffer);

   if( numBytes != sizeof("Hello Client"))
   {
       CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Wrong size of data received from server");
       rc = eCliReturn_Failed;
   }

   delete dpClientSocket;
   delete dpServerSocket;
   return rc;
}
