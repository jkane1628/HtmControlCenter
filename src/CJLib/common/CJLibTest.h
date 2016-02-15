// CJLibTest_Main.cpp : Implementation file
//

#include "CJTypes.h"

class CJLibTest;
class CJConsole;
class CJTrace;
class CJList;
class CJPool;
class CJPoolBlocking;

class CJLibTestCliCommandSet
{
public:
   static CJLibTest* dpLibTest;
   static CliCommand CJLibTestCommandSet[];

   static CliReturnCode CliCommand_ListTest( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);
   static CliReturnCode CliCommand_PoolTest( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);
   static CliReturnCode CliCommand_BlockingPoolTest( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);
   static CliReturnCode CliCommand_ThreadTest( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);
   static CliReturnCode CliCommand_PFileTest( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);
   static CliReturnCode CliCommand_JobTest( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);
   static CliReturnCode CliCommand_SocketTest( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);
   

};

class CJLibTest : public CJObject
{
public:
   CJLibTest();
   ~CJLibTest();

   int Main(int argc, char *argv[]);

   CliReturnCode RunLinkedListTest();
   CliReturnCode RunPoolTest();
   CliReturnCode RunBlockingPoolTest();
   CliReturnCode RunThreadTest(CJConsole* pConsole);
   CliReturnCode RunPFileTest();
   CliReturnCode RunJobTest();
   CliReturnCode RunSocketTest();

private:

   CJConsole*               dpSocketConsole;
   CJCli*                   dpTelnetCli;

   CJConsole*               dpConsole;
   CJCli*                   dpCli;
   CJLibTestCliCommandSet*  dpCliCommandSet;
   CJTrace*                 dpTrace;
   CJList*                  dpLinkedList;
   CJPool*                  dpPool;
   CJPoolBlocking*          dpBlockingPool;
   CJSocket*                dpClientSocket;
   CJSocket*                dpServerSocket;

};

