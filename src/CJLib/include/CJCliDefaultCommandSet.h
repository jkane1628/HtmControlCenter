// CJCliDefaultCommandSet.h : Header file
//

#ifndef __CJCLIDEFAULTCOMMANDSET_H_
#define __CJCLIDEFAULTCOMMANDSET_H_

#include "CJConfig.h"

class CJCli;
struct CliParams;

class CliEmptyCommandSet
{
public:
   static CliCommand EmptyCommandSet[];
};

class CliDefaultCommandSet
{
public:
   CliDefaultCommandSet( CJCli* pCli);

   static CliCommand DefaultCommandSet[];

   // Default CLI Command Set
   //   NOTE: This can be used as a template for a user defined set of CLI Commands
   // 

   static CliReturnCode CliCommand_History( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);
   static CliReturnCode CliCommand_Help( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);
#ifdef USE_DEFAULT_GLOBAL_THREAD_MANAGER
   static CliReturnCode CliCommand_Threads( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);
#endif
   static CliReturnCode CliCommand_Trace( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);
   static CliReturnCode CliCommand_TraceLevel( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);

private:

};


#endif //__CJCLIDEFAULTCOMMANDSET_H_
