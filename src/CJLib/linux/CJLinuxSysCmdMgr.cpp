// CJLinuxSysCmdMgr.cpp : Implementation file
//
#include <sys/wait.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/prctl.h>

#include "CJTrace.h"
#include "CJSem.h"
#include "CJLinuxSysCmd.h"
#include "CJLinuxSysCmdMgr.h"

#define USE_DEFAULT_LINUX_SYSTEM_COMMAND_MANAGER

#ifdef USE_DEFAULT_LINUX_SYSTEM_COMMAND_MANAGER
CJLinuxSysCmdMgr gLinuxSysCmdMgr("GLOBAL");
CJLinuxSysCmdMgr* gpLinuxSysCmdMgr = &gLinuxSysCmdMgr;
#else
CJLinuxSysCmdMgr* gpLinuxSysCmdMgr = NULL;
#endif


enum SysCmdMgrOpCode
{
   eSysCmdMgrOp_Exit,
   eSysCmdMgrOp_ShellCommand,  // dataLen = num chars to read in for shell command
};

struct SysCmdMgrOperation
{
   U32             magicValue;  // Always 0xABCDABCD
   SysCmdMgrOpCode opcode;
   U32             dataLen;
};

struct SysCmdMgrResponse
{
   U32             magicValue;  // Always 0xABCDABCD
   U32             sysCmdMgrError;
   int             shellRc;
};


CJLinuxSysCmdMgr::CJLinuxSysCmdMgr(char const* name) : CJObject("SYSCMDMGR::", name)
{
   dpOutputIdSem = new CJSem(1);
   dOutputId = 1;

   if (pipe(dCommandPipeFd) == -1) 
   { 
      printf("ERROR: Failed to create command pipe");
      exit(EXIT_FAILURE); 
   }
   if (pipe(dDataPipeFd) == -1) 
   { 
      printf("ERROR: Failed to create data pipe");
      exit(EXIT_FAILURE); 
   }
   if (pipe(dResponsePipeFd) == -1) 
   { 
      printf("ERROR: Failed to create response pipe");
      exit(EXIT_FAILURE); 
   }

   dpid = fork();
   if (dpid == -1) 
   { 
      printf("ERROR: Failed to fork");
      exit(EXIT_FAILURE); 
   }
   if (dpid == 0) 
   {  
      MainChildProcess();
      close(dCommandPipeFd[1]);
      close(dCommandPipeFd[0]);
      close(dDataPipeFd[1]);
      close(dDataPipeFd[0]);
      close(dResponsePipeFd[1]);
      close(dResponsePipeFd[0]);
      _exit(EXIT_SUCCESS);
   }
   // Main process continues to execute normally

}


CJLinuxSysCmdMgr::~CJLinuxSysCmdMgr()
{
}


void CJLinuxSysCmdMgr::ShutdownChildProcess()
{
   SysCmdMgrOperation op;

   // Build up an exit command
   if( dpid != 0)
   {
      printf("Shutting down child process\n");
      op.magicValue = SYSCMD_MAGIC_CMD_VALUE;
      op.opcode = eSysCmdMgrOp_Exit;
      op.dataLen = 0;
      write( dCommandPipeFd[1], &op, sizeof(SysCmdMgrOperation));
      wait(NULL);
   }

   close(dCommandPipeFd[1]);
   close(dCommandPipeFd[0]);
   close(dDataPipeFd[1]);
   close(dDataPipeFd[0]);
   close(dResponsePipeFd[1]);
   close(dResponsePipeFd[0]);
}


void CJLinuxSysCmdMgr::MainChildProcess()
{
   SysCmdMgrOperation cmdOp;
   SysCmdMgrResponse  rsp;
   char inputBuffer[MAX_INPUT_COMMAND_STR_LEN+1];
   int rc0, rc1;
   U32 errorValue;

   system("touch sysCmdErrorLog.txt");

   // Change the process name so it can be easily identified
   //char processName[16];
   //rc0 = prctl( PR_GET_NAME, processName, 0,0,0);
   //if( strlen(processName) >= 11)
   //{
   //   processName[10] = 0;
   //}
   //strcat(processName, "_trc");
   //rc0 = prctl( PR_SET_NAME, processName, 0,0,0);




   // Read command from the command pipe
   while (1)
   {
      errorValue = 0;
      rc1 = 0;
      if( read(dCommandPipeFd[0], &cmdOp, sizeof(SysCmdMgrOperation)) > 0)
      {

         //AddToLog("Got cmd (magic=0x%x, op=%u, len=%u)\n", cmdOp.magicValue, cmdOp.opcode, cmdOp.dataLen);


         if( cmdOp.magicValue != SYSCMD_MAGIC_CMD_VALUE)
         {
            AddToLog("ERROR: Failed to read command, invalid magic value (val=0x%X)\n", cmdOp.magicValue);
            errorValue = 10;
         }
         else if( cmdOp.opcode == eSysCmdMgrOp_Exit)
         {        
            AddToLog("Shutting down linux sys command child process\n");
            return;
         }
         else if( cmdOp.opcode == eSysCmdMgrOp_ShellCommand)
         {
            if( cmdOp.dataLen > MAX_INPUT_COMMAND_STR_LEN)
            {
               AddToLog("ERROR: Shell command is too long (dataLen=%u, max=%u)\n", cmdOp.dataLen, MAX_INPUT_COMMAND_STR_LEN);
               errorValue = 20;
            }
            else
            {
               // Read in the shell command line
               AddToLog("Reading %d bytes of data\n", cmdOp.dataLen);
               rc0 = read(dDataPipeFd[0], &inputBuffer, cmdOp.dataLen);
               if( rc0 > 0)
               {
                  inputBuffer[cmdOp.dataLen] = 0;
   
                  rc1 = system(inputBuffer);
   
                  //AddToLog("Executed sh: %s : rc=%d\n", inputBuffer, rc1);
                  
               }
               else
               {
                  AddToLog("ERROR: Failed to read data string (read_rc=%d)\n", rc0);
                  errorValue = 30;
               }
            }
         }
      }
      else
      {
         AddToLog("ERROR: Failed to read command, pipe read failed\n");
         errorValue = 40;
      }

      rsp.magicValue = SYSCMD_MAGIC_CMD_VALUE;
      rsp.sysCmdMgrError = errorValue;
      rsp.shellRc = rc1;

      rc0 = write( dResponsePipeFd[1], &rsp, sizeof(SysCmdMgrResponse));
      if( rc0 <= 0)
      {
         AddToLog("ERROR: Failed to write response, pipe write failed\n");
         errorValue = 50;
      }
   }
}


int CJLinuxSysCmdMgr::SendShellCommand( char* commandStr)
{
   SysCmdMgrResponse  rsp;
   SysCmdMgrOperation op;
   int rc;

   // Build up an operation command
   op.magicValue = SYSCMD_MAGIC_CMD_VALUE;
   op.opcode = eSysCmdMgrOp_ShellCommand;
   op.dataLen = strlen(commandStr);

   if(op.dataLen > MAX_INPUT_COMMAND_STR_LEN)
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: command string is too long (len=%u)", op.dataLen);
      return FALSE;
   }

   // Write the command to the pipe
   rc = write( dCommandPipeFd[1], &op, sizeof(SysCmdMgrOperation));
   CJTRACE(TRACE_DBG_LEVEL, "wrote to command pipe (len=%u, rc=%d)", sizeof(SysCmdMgrOperation), rc);

   // Write the shell command to the data pipe
   rc = write( dDataPipeFd[1], commandStr, op.dataLen);
   CJTRACE(TRACE_DBG_LEVEL, "wrote to data pipe (len=%u, rc=%d)", op.dataLen, rc);

   // Wait for the response
   rc = read( dResponsePipeFd[0], &rsp, sizeof(SysCmdMgrResponse));
   CJTRACE(TRACE_DBG_LEVEL, "read from command pipe (len=%u, rc=%d)", sizeof(SysCmdMgrResponse), rc);
   if( rc > 0)
   {
      if( rsp.magicValue != SYSCMD_MAGIC_CMD_VALUE)
      {
         CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Command response did not have valid magic value (val=0x%X)\n", rsp.magicValue);
         return -1111;
      }
      else if( rsp.sysCmdMgrError != 0)
      {
         CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Command response had a sysCmd error (error=%d)\n", rsp.sysCmdMgrError);
         return -2222;
      }
      else if( rsp.shellRc != 0)
      {
         CJTRACE(TRACE_ERROR_LEVEL, "WARN: shell return was not zero (rc=%d, cmd=%s)\n", rsp.shellRc, commandStr);
      }
   }
   else
   {
      CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to read response data (read_rc=%d)\n", rc);
      return -3333;
   }
   return rsp.shellRc;

}


void CJLinuxSysCmdMgr::AddToLog(char const* logStr, ...)
{
   FILE* log = fopen("sysCmdErrorLog.txt", "a"); 

   char tracebuf[1024];
   va_list a_list;
   va_start( a_list, logStr );

   sprintf(tracebuf, logStr, a_list);
   fwrite( tracebuf, strlen(tracebuf), 1, log);

   va_end ( a_list );
   fclose(log);
}


U32 CJLinuxSysCmdMgr::GetUniqueOutputId()
{
   U32 rc;
   dpOutputIdSem->Lock();
   rc = dOutputId;
   dOutputId++;
   if( dOutputId >= 32)
   {
      dOutputId = 1;
   }
   dpOutputIdSem->Unlock();
   return rc;
}

