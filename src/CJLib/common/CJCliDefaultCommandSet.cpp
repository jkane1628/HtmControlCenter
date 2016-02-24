// CJCliDefaultCommandSet.cpp : Implementation file
//

#include <string.h>
#include <stdlib.h>

#include "CJConfig.h"
#include "CJThreadMgr.h"
#include "CJThread.h"
#include "CJTrace.h"
#include "CJCli.h"
#include "CJConsole.h"
#include "CJCliDefaultCommandSet.h"


// Setup an empty command set for initialization
CJCLI_COMMAND_DEFINTION_START(CliEmptyCommandSet)
CJCLI_COMMAND_DEFINTION_END

// Map CLI commands to associated functions for the default command set
CJCLI_CREATE_FCNMAPPER(CliDefaultCommandSet)
CJCLI_MAP_CMD_TO_FCN(CliDefaultCommandSet, history, CliCommand_History)
CJCLI_MAP_CMD_TO_FCN(CliDefaultCommandSet, help, CliCommand_Help)
CJCLI_MAP_CMD_TO_FCN(CliDefaultCommandSet, trace, CliCommand_Trace)
CJCLI_MAP_CMD_TO_FCN(CliDefaultCommandSet, tracelevel, CliCommand_TraceLevel)

// Define CLI commands for the default command set
CJCLI_COMMAND_DEFINTION_START(CliDefaultCommandSet)
CJCLI_COMMAND_DESCRIPTOR( history,     eCliAccess_Guest, "Displays the list of commands recently executed on this CLI session")
CJCLI_COMMAND_DESCRIPTOR( help,        eCliAccess_Guest, "Displays the list of available CLI commands")
CJCLI_COMMAND_DESCRIPTOR( trace,       eCliAccess_Guest, "Display the trace buffer, or redirect the trace buffer to CLI")
CJCLI_COMMAND_DESCRIPTOR( tracelevel,  eCliAccess_Admin, "Set the tracelevel of a specific modules")
#ifdef USE_DEFAULT_GLOBAL_THREAD_MANAGER
CJCLI_COMMAND_DESCRIPTOR( threads,     eCliAccess_Hidden, Display all threads and thread information)
#endif
CJCLI_COMMAND_DEFINTION_END




CliReturnCode CliDefaultCommandSet::CliCommand_History( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams)
{
   CJCli::spActiveCli->DisplayHistoryBuffer(pConsole);
   return eCliReturn_Success;
}
CliReturnCode CliDefaultCommandSet::CliCommand_Help( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams)
{
   CJCli::spActiveCli->DisplayRegisteredCliCommands(pConsole);
   return eCliReturn_Success;
}

#ifdef USE_DEFAULT_GLOBAL_THREAD_MANAGER
CliReturnCode CliDefaultCommandSet::CliCommand_Threads( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams)
{
   CJThread* pTempThread;
   else if ((pParams->numParams == 2) && (strcmp(pParams->str[1], "-help") == 0))
   {
      pConsole->Printf( "threads"
                        "   PARAMS:"
                        "     none");
      return eCliReturn_Success;
   }

   pConsole->Printf("System Threads:\n");
   pConsole->Printf("Name                              State     Affinity\n");
   pConsole->Printf("--------------------------------  --------  ----------\n");

   if(gpThreadMgr->GetThreadCount() == 0)
   {
      pConsole->Printf("<No Threads In Use>\n");
   }
   else
   {
      pTempThread = gpThreadMgr->GetNextThread(NULL);
      while(pTempThread)
      {
         pConsole->Printf("%-32s  %-8s  0x%-08X\n", pTempThread->GetObjectStr(), 
                                             pTempThread->GetThreadStateStr(), 
                                             pTempThread->GetAffinity());

         pTempThread = gpThreadMgr->GetNextThread(pTempThread);
      }
   }
   pConsole->Printf("\n");
   return eCliReturn_Success;
}
#endif

CliReturnCode CliDefaultCommandSet::CliCommand_Trace( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams)
{
   U32 kb_trace_buffer = 0;

   if(pParams->numParams == 1)
   {
      CJTrace::DisplayTraceBuffer(pConsole, 5 * 1024);
   }
   else if(pParams->numParams == 2)
   {
      if (strcmp(pParams->str[1], "-help") == 0)
      {
         pConsole->Printf( "trace [num_KB_of_trace_text] | [-on|-off]"
                           "   PARAMS:"
                           "     num_KB_of_trace_text = Number of KB of trace buffer to display (default=5KB)"
                           "     -on/-off - Turns on/off the CLI trace buffer display in real time");
      }
      if(strcmp(pParams->str[1], "-on") == 0)
      {
         CJTrace::SetCliTraceState(TRUE);
         pConsole->Printf("Setting CLI trace printing ON\n");
         return eCliReturn_Success;
      }
      else if(strcmp(pParams->str[1], "-off") == 0)
      {
         pConsole->Printf("Setting CLI trace printing OFF\n");
         CJTrace::SetCliTraceState(FALSE);
         return eCliReturn_Success;
      }
      else
      {
         kb_trace_buffer = atol(pParams->str[1]);
         if( kb_trace_buffer > (CJTRACE_DEFAULT_TRACE_BUFFER_SIZE_BYTES / 1024))
         {
            pConsole->Printf("ERROR: Invalid trace view size in KB (size=%u)\n", kb_trace_buffer);
            return eCliReturn_InvalidParam;
         }
         CJTrace::DisplayTraceBuffer(pConsole, kb_trace_buffer * 1024);
      }
   }
   else
   {
      return eCliReturn_InvalidParam;
   }

   return eCliReturn_Success;
}

CliReturnCode CliDefaultCommandSet::CliCommand_TraceLevel( CJConsole* pConsole, CliCommand* pCmd, CliParams* pParams)
{
   U32 objectID = 0xFFFFFFFF;
   U32 level = 0xFFFFFFFF;
   U32 paramIndex = 0;

   if(pParams->numParams == 1)
   {
      pConsole->Printf("TRACE LEVELS: OFF=%d, ERROR=%d, HIGH=%d, LOW=%d, DEBUG=%d\n", TRACE_OFF, TRACE_ERROR_LEVEL, TRACE_HIGH_LEVEL, TRACE_LOW_LEVEL, TRACE_DBG_LEVEL);
      CJTrace::DisplayRegisteredObjects(pConsole);
   }
   else if ((pParams->numParams == 2) && (strcmp(pParams->str[1], "-help") == 0))
   {
      pConsole->Printf( "tracelevel [newlevel] | [-o objectID] | [-l] "
                        "   PARAMS:"
                        "     newlevel    - The new level assigned to the given object"
                        "     -o objectID - Specifies a certain object to adjust the trace level"
                        "     -l          - Displays all traceable objects and their current trace level");
   }
   else if(pParams->numParams == 3)
   {
      if(strcmp(pParams->str[1], "-global") == 0) // TODO: FIX THIS TO SET ALL TRACE LEVELS
      {
         level = atol(pParams->str[paramIndex]);
         CJTrace::SetTraceGlobalLevel(level);
      }
      else
      {
         pConsole->Printf("ERROR: Unknown command switch (%s)\n",pParams->str[1]);
         return eCliReturn_InvalidParam;
      }
   }
   else if(pParams->numParams == 5)
   {
      paramIndex = 1;
      while( paramIndex < pParams->numParams)
      {
         if(strcmp(pParams->str[paramIndex], "-id") == 0)
         {
            pConsole->Printf("got an id...\n");
            paramIndex++;
            objectID = atol(pParams->str[paramIndex]);
            paramIndex++;
         }
         else if(strcmp(pParams->str[paramIndex], "-level") == 0)
         {
            pConsole->Printf("got a level...\n");
            paramIndex++;
            level = atol(pParams->str[paramIndex]);
            paramIndex++;
         }
         else
         {
            pConsole->Printf("ERROR: Unknown command switch (%s)\n",pParams->str[paramIndex]);
            return eCliReturn_InvalidParam;
         }
      }

      if((objectID == 0xFFFFFFFF) || (level == 0xFFFFFFFF))
      {
         pConsole->Printf("ERROR: An -id and -level must be specified in the command\n");
         return eCliReturn_InvalidParam;
      }

      if (CJTrace::SetTraceLevel((eCJTraceId)objectID, level) == FALSE)
      {
         pConsole->Printf("ERROR: Failed to find object ID\n");
         return eCliReturn_Failed;
      }
      pConsole->Printf("Trace Object ID %u has been set to trace level %u\n", objectID, level);
      
   }
   else
   {
      return eCliReturn_InvalidParam;
   }

   return eCliReturn_Success;
}



