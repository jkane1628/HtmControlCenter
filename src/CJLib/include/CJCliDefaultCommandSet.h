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
   CliEmptyCommandSet() {}
   static CliCommand CommandDescriptorArray[];
};

// Default CLI Command Set
//   NOTE: This can be used as a template for a user defined set of CLI Commands
// 
class CliDefaultCommandSet
{
public:
   CliDefaultCommandSet() {  }
   static CliCommand CommandDescriptorArray[];

   CliReturnCode CliCommand_History( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);
   CliReturnCode CliCommand_Help( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);
   CliReturnCode CliCommand_Trace( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);
   CliReturnCode CliCommand_TraceLevel( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);
#ifdef USE_DEFAULT_GLOBAL_THREAD_MANAGER
   CliReturnCode CliCommand_Threads( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams);
#endif

};


#endif //__CJCLIDEFAULTCOMMANDSET_H_
