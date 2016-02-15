// CJLinuxSysCmdMgr.h : Header file
//

#ifndef __CJLINUXSYSCMDMGR_H_
#define __CJLINUXSYSCMDMGR_H_

//#include "CJTypes.h"
#include "CJObject.h"

#define SYSCMD_MAGIC_CMD_VALUE 0xABCDABCD

class CJSem;

class CJLinuxSysCmdMgr : public CJObject
{
public:
   CJLinuxSysCmdMgr( char const* name);
   ~CJLinuxSysCmdMgr();

   U32 GetUniqueOutputId();
   int SendShellCommand( char* commandStr);

   void ShutdownChildProcess();

private:

   void MainChildProcess();
   void AddToLog(char const* logStr, ...);

   pid_t  dpid;
   int    dCommandPipeFd[2];
   int    dDataPipeFd[2];
   int    dResponsePipeFd[2];

   CJSem* dpOutputIdSem;
   U32    dOutputId;
};

extern CJLinuxSysCmdMgr* gpLinuxSysCmdMgr;

#endif // __CJLINUXSYSCMD_H_
