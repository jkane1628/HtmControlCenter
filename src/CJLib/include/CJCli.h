// CJCli.h : Header file
//

#ifndef __CJCLI_H_
#define __CJCLI_H_

#include "CJTypes.h"
#include "CJObject.h"
#include "CJConfig.h"

class CJCli;
class CJThread;
class CJConsole;
class CliDefaultCommandSet;

struct CliParams  
{
   char  rawBuffer[MAX_LINE_BUFFER_SIZE+1];
   char  parsedBuffer[MAX_LINE_BUFFER_SIZE+1];
   char* str[MAX_NUMBER_PARAMS];
   U32   numParams;
};

enum CliAccessLevel
{
   eCliAccess_Hidden,
   eCliAccess_Guest,
   eCliAccess_Admin,
   eCliAccess_Devel
};

enum CliReturnCode
{
   eCliReturn_Success,
   eCliReturn_InvalidParam,
   eCliReturn_UnknownCommand,
   eCliReturn_Failed
};

struct CliCommand;

typedef CliReturnCode (*CliFunction)(CJConsole*, CliCommand*, CliParams*);

struct CliCommand
{
   char const* command_name;
   char const* short_desc;
   char const* usage;
   CliAccessLevel access_level;
   CliFunction cbCliFunc;
};

class CliCommandSetBase
{
public:
   CliCommand* dpCommandSet;
};

// NOTE: DO NOT USE THIS MACRO IF MULTIPLE CLIs ARE BEING USED, THIS ONLY REGISTERS WITH THE GLOBAL CLI INSTANCE
#define CJTRACE_REGISTER_CLI_COMMAND_OBJ(pObj) CJCli::spGlobalCli->RegisterCliCommandSet(pObj);

enum CliSessionState
{
   eCLI_STATE_UNINITIALIZED,
   eCLI_STATE_READY_FOR_CONNECTION,
   eCLI_STATE_RUNNING,
   eCLI_STATE_SHUTTING_DOWN,
   eCLI_STATE_SHUTDOWN
};

#define CLI_SPECIAL_KEY_IDENTIFIER 28

enum CliSpecialKeys
{
   eCLI_KEY_UNKNOWN,
   eCLI_KEY_ESC,
   eCLI_KEY_F1,
   eCLI_KEY_F2,
   eCLI_KEY_F3,
   eCLI_KEY_F4,
   eCLI_KEY_F5,
   eCLI_KEY_F6,
   eCLI_KEY_F7,
   eCLI_KEY_F8,
   eCLI_KEY_F9,
   eCLI_KEY_F10,
   eCLI_KEY_F11,
   eCLI_KEY_F12,
   eCLI_KEY_UP_ARROW,
   eCLI_KEY_DOWN_ARROW,
   eCLI_KEY_RIGHT_ARROW,
   eCLI_KEY_LEFT_ARROW,
   eCLI_KEY_PAGE_UP,
   eCLI_KEY_PAGE_DOWN,
   eCLI_KEY_HOME,
   eCLI_KEY_END,
   eCLI_KEY_INSERT,
   eCLI_KEY_DELETE
};

class CJCli : public CJObject
{
public:
   CJCli(char const* pCliNameStr, CJConsole* pConsole);
   virtual ~CJCli( );

   BOOL StartCliSession( BOOL blockTillShutdown = FALSE);
   BOOL InitiateCliShutdown();
   BOOL BlockTillCliShutdown();

   BOOL RegisterCliCommandSet( CliCommandSetBase* pCliCommandSetBase);

   void RunCli();

   void DisplayHistoryBuffer(CJConsole* pLocalConsole);
   void DisplayRegisteredCliCommands(CJConsole* pLocalConsole);

   CJConsole* GetConsole() { return dpConsole;}

   static CJCli* spActiveCli;
   static CJCli* spGlobalCli; // The first created CLI object becomes the "global" one


protected:
   virtual void HandleCtrlKeyPress( char ctrlChar);
   virtual void HandleSpecialKeyPress( CliSpecialKeys value);
   virtual void HandleArrowKey( CliSpecialKeys value);
   virtual void HandleBackspaceKey();
   virtual void HandleDeleteKey();

   virtual void ParseCommandLine( char* pLineBuffer, int numChars);

   void PrintLineFeed();
   void PrintPrompt();
   void PrintLineBuffer( BOOL doFlush=TRUE);
   void MovePositionToStartOfLine(BOOL clearLineDisplay);
   void StartNewLine();

private:

   
    
   CJConsole* dpConsole;
   CJThread* dpSessionThread;
   CliSessionState dState;
   CliDefaultCommandSet* dpDefaultCliCommandSet;
   CliCommand* dpUserCliCommandSetAry[MAX_NUM_USER_CLI_COMMAND_SETS];
   
   int  dInputFileDescriptor;
   int  dOutputFileDescriptor;

   char dLineBuffer[MAX_LINE_BUFFER_SIZE+1];
   int  dLineBufferCount;
   int  dLineBufferPosition;

   char dHistoryBuffer[MAX_HISTORY_ENTRIES][MAX_LINE_BUFFER_SIZE];
   int  dHistoryBufferSizes[MAX_HISTORY_ENTRIES];
   int  dHistoryBufferCount;
   int  dHistoryBufferNextIndex;
   int  dHistoryBufferTempIndex;

   void ProcessCharacter( char* pInputChars, int numChars);

   BOOL IsStandardASCIIChar(char ch);
   BOOL IsReturnChar(char ch);
   BOOL IsLineFeedChar(char ch);
   BOOL IsSpecialChar(char ch);
   BOOL IsEscChar(char ch);
   
   BOOL IsCtrlChar(char ch);
   BOOL IsBackspaceKey(char ch);
   BOOL IsDeleteKey(CliSpecialKeys specialKey);
   BOOL IsArrowKey(CliSpecialKeys specialKey);

   char ConvertCtlrCharToStandard(char ctrlChar);
   int  ConvertEscCharToSpecial(char* pMultiKeyArray, int numBytes, CliSpecialKeys* pSpecialKey);

   void AddLineBufferToHistoryBuffer();
   void LoadHistoryBufferToLineBuffer( int index);


};
extern CJCli* gpCli;

#endif // __CJCLI_H_
