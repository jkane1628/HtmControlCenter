// CJCliDefaultCommandSet.cpp : Implementation file
//

#include <string.h>
#include <stdlib.h>

#include "CJThreadMgr.h"
#include "CJThread.h"
#include "CJTrace.h"
#include "CJCli.h"
#include "CJConsole.h"
#include "CJCliDefaultCommandSet.h"

CliCommand CliEmptyCommandSet::EmptyCommandSet[] =
{
   {"","","",eCliAccess_Hidden} // This must be the last command
};

CliCommand CliDefaultCommandSet::DefaultCommandSet[] =
{
   {"history",
    "Displays the list of commands recently executed on this CLI session",
    "history [num_commands]\n"\
    "   PARAMS:"\
    "     num_command - Number of previous commands to display",
    eCliAccess_Guest,
    CliDefaultCommandSet::CliCommand_History},

   {"help",
    "Displays the list of available CLI commands",
    "help",
    eCliAccess_Guest,
    CliDefaultCommandSet::CliCommand_Help},


#ifdef USE_DEFAULT_GLOBAL_THREAD_MANAGER
   {"threads",
    "Display all threads and the associated information.",
    "threads"\
      "   PARAMS:"\
      "     none",
    eCliAccess_Guest,
    CliDefaultCommandSet::CliCommand_Threads},
#endif

   {"trace",
    "Display the trace buffer.  This can also redirect the trace buffer to the CLI",
    "trace [num_KB_of_trace_text] | [-on|-off]"\
      "   PARAMS:"\
      "     num_KB_of_trace_text = Number of KB of trace buffer to display (default=5KB)"\
      "     -on/-off - Turns on/off the CLI trace buffer display in real time",
    eCliAccess_Guest,
    CliDefaultCommandSet::CliCommand_Trace},

   {"tracelevel",
    "Set the tracelevel globally or for a specific object",
    "tracelevel [newlevel] | [-o objectID] | [-l] "\
      "   PARAMS:"\
      "     newlevel    - The new level assigned to the given object"\
      "     -o objectID - Specifies a certain object to adjust the trace level"\
      "     -l          - Displays all traceable objects and their current trace level",
    eCliAccess_Admin,
    CliDefaultCommandSet::CliCommand_TraceLevel},


   {"","","",eCliAccess_Hidden} // This must be the last command
};


CliDefaultCommandSet::CliDefaultCommandSet( CJCli* pCli)
{

}

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
      gpTrace->DisplayTraceBuffer(pConsole, 5 * 1024);
   }
   else if(pParams->numParams == 2)
   {
      if(strcmp(pParams->str[1], "-on") == 0)
      {
         gpTrace->SetCliTraceState(TRUE);
         pConsole->Printf("Setting CLI trace printing ON\n");
         return eCliReturn_Success;
      }
      else if(strcmp(pParams->str[1], "-off") == 0)
      {
         pConsole->Printf("Setting CLI trace printing OFF\n");
         gpTrace->SetCliTraceState(FALSE);
         return eCliReturn_Success;
      }
      else
      {
         kb_trace_buffer = atol(pParams->str[1]);
         if( kb_trace_buffer > (CJTRACE_DEFAULT_TRACE_BUFFER_SIZE_BYTES / 1024))
         {
            pConsole->Printf("ERROR: Invalid trace view size in KB (size=%u)\n", kb_trace_buffer);
            pConsole->Printf("%s\n", pCmd->usage);
            return eCliReturn_InvalidParam;
         }
         gpTrace->DisplayTraceBuffer(pConsole, kb_trace_buffer * 1024);
      }
   }
   else
   {
      pConsole->Printf("%s\n", pCmd->usage);
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
      gpTrace->DisplayRegisteredObjects(pConsole);
   }
   else if(pParams->numParams == 3)
   {
      if(strcmp(pParams->str[1], "-global") == 0)
      {
         level = atol(pParams->str[paramIndex]);
         if( gpTrace->SetTraceObjectLevel( 0, level) == FALSE)
         {
            pConsole->Printf("ERROR: Failed to set new trace level\n");
            return eCliReturn_Failed;
         }
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

      if( gpTrace->SetTraceObjectLevel( objectID, level) == FALSE)
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



