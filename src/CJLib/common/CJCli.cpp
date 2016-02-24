// CJCli.cpp : Implmentation file
//

#include <string.h>
#include <stdio.h>

#include "CJTrace.h"
#include "CJConsole.h"
#include "CJThread.h"
#include "CJCli.h"
#include "CJCliDefaultCommandSet.h"

#undef CJTRACE_MODULE_ID
#define CJTRACE_MODULE_ID eTraceId_CJ_Cli

CJCli* CJCli::spActiveCli = NULL;
CJCli* CJCli::spGlobalCli = NULL;


CJCli::CJCli(char const* pCliNameStr, CJConsole* pConsole)
{
   CJTRACE_SET_TRACEID_STRING(eTraceId_CJ_Cli,"CLI");

   // Register the first CLI as the global
   if (spGlobalCli == NULL)
      spGlobalCli = this;

   dpConsole = pConsole;
   dpSessionThread = new CJThread("MAINTHREAD");
   dState = eCLI_STATE_READY_FOR_CONNECTION;

   dLineBufferCount = 0;
   dLineBufferPosition = 0;
   for( int i=0; i < MAX_NUM_USER_CLI_COMMAND_SETS; i++)
   {
      dpUserCliCommandSetAry[i] = NULL;
   }

   // History Buffer Init
   memset(dHistoryBufferSizes, 0, sizeof(int) * MAX_HISTORY_ENTRIES);
   dHistoryBufferNextIndex = 0;

   CJTRACE_REGISTER_CLI_COMMAND_OBJ(CliDefaultCommandSet)
}


CJCli::~CJCli( )
{
   delete dpSessionThread;
}


U32 gCliThreadFunction( void* cliPtr)
{
   CJCli* pCli = (CJCli*)cliPtr;
   pCli->RunCli();
   return 0;
}


BOOL CJCli::StartCliSession( BOOL blockTillShutdown)
{
   BOOL rc = dpSessionThread->InitThread( NULL, gCliThreadFunction, this);
   if( rc == FALSE)
   {
      return FALSE;
   }
   dState = eCLI_STATE_RUNNING;

   if( blockTillShutdown)
   {
      return BlockTillCliShutdown();
   }
   return TRUE;
}


BOOL CJCli::InitiateCliShutdown()
{
   if( dState != eCLI_STATE_RUNNING)
   {
      return FALSE;
   }

   // Set the flag to shutdown the CLI
   dState = eCLI_STATE_SHUTTING_DOWN;
   return TRUE;
}


BOOL CJCli::BlockTillCliShutdown()
{
   if( dpSessionThread->WaitForThreadExit() != TRUE)
   {
      // CJTrace(ERROR, "Failed to shutdown CLI thread\n");
      return FALSE;
   }
   dState = eCLI_STATE_SHUTDOWN;
   return TRUE;
}

BOOL CJCli::RegisterCliCommandSet(CliCommand* pCommandSet)
{
   for( int i=0; i < MAX_NUM_USER_CLI_COMMAND_SETS; i++)
   {
      if( dpUserCliCommandSetAry[i] == NULL)
      {
         dpUserCliCommandSetAry[i] = pCommandSet;
         return TRUE;
      }
   }
   //CJASSERT("Too many CLI command sets registered");
   return FALSE;  // TODO: SHOULD BE A FAULT OR ASSERT
}

void CJCli::RunCli()
{
   char inputChars[16];
   int  maxChars=16;
   int  numChars;
   int  fail_count = 0;
   
   while (1) 
   {
       // Get console connection
       if( dpConsole->InitializeConsoleConnection())
       {
           PrintLineFeed();
           PrintPrompt();

           while (1) {
               // Get data from console
               numChars = dpConsole->WaitAndGetChars( inputChars, maxChars);
               
               // Check to make sure console is still operational
               if ( numChars == 0)
               {
                   break;
               }
               // Process data through CLI
               ProcessCharacter( inputChars, numChars);

               // Check CLI state
               if( dState != eCLI_STATE_RUNNING)
               {
                  break;
               }
           }
       }
       else
       {
           fail_count++;
           if ( fail_count > 3) 
           {
               CJTRACE( TRACE_ERROR_LEVEL, "ERROR: Failed to init console connect, shutting down this CLI task\n");
               break;
           }
           dpConsole->ResetConsole();
       }

       // Check CLI state
       if( dState != eCLI_STATE_RUNNING)
       {
          break;
       }
   }
}



void CJCli::PrintLineFeed()
{
   dpConsole->PrintChar('\n');
}
void CJCli::PrintPrompt()
{
   dpConsole->Printf(CLI_DEFAULT_PROMPT);
   dpConsole->Flush();
}


BOOL CJCli::IsStandardASCIIChar(char ch)
{
   return (ch >= 32 && ch <= 126) ? TRUE : FALSE;
}
BOOL CJCli::IsReturnChar(char ch)
{
   return (ch == '\r')? TRUE:FALSE;
}
BOOL CJCli::IsLineFeedChar(char ch)
{
   return (ch == '\n')? TRUE:FALSE;
}
BOOL CJCli::IsSpecialChar(char ch)
{
   return (ch == 28) ? TRUE : FALSE;
}

BOOL CJCli::IsBackspaceKey(char ch)
{
   return (ch == 0x7F)? TRUE:FALSE;
}

BOOL CJCli::IsEscChar(char ch)
{
   return (ch == 27)? TRUE:FALSE;
}

BOOL CJCli::IsCtrlChar(char ch)
{
   return (ch >= 1 && ch <= 26)? TRUE:FALSE;
}
BOOL CJCli::IsDeleteKey(CliSpecialKeys specialKey)
{
   return (specialKey == eCLI_KEY_DELETE)? TRUE:FALSE;
}
BOOL CJCli::IsArrowKey(CliSpecialKeys specialKey)
{
   return ((specialKey == eCLI_KEY_UP_ARROW) ||
    (specialKey == eCLI_KEY_DOWN_ARROW) ||
    (specialKey == eCLI_KEY_RIGHT_ARROW) ||
    (specialKey == eCLI_KEY_LEFT_ARROW))? TRUE:FALSE;
}

char CJCli::ConvertCtlrCharToStandard(char ctrlChar)
{
   return ctrlChar + 64;
}
int  CJCli::ConvertEscCharToSpecial(char* pMultiKeyArray, int numBytes, CliSpecialKeys* pSpecialKey)
{

   //dpConsole->Printf("Multibyte: ");
   //for( int i=0; i < numBytes; i++)
   //{
   //   dpConsole->Printf("%d  ", pMultiKeyArray[i]);
   //
   //}
   //dpConsole->Printf("\n");


   if((numBytes == 1) && (pMultiKeyArray[0] == 27))
   {
      *pSpecialKey = eCLI_KEY_ESC;
      return numBytes;
   }
   else if((numBytes == 3) && (pMultiKeyArray[0] == 27))
   {
           if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 68)) *pSpecialKey = eCLI_KEY_LEFT_ARROW;
      else if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 65)) *pSpecialKey = eCLI_KEY_UP_ARROW;
      else if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 66)) *pSpecialKey = eCLI_KEY_DOWN_ARROW;
      else if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 67)) *pSpecialKey = eCLI_KEY_RIGHT_ARROW;
      else *pSpecialKey = eCLI_KEY_UNKNOWN;
      return numBytes;
   }
   else if((numBytes == 4) && (pMultiKeyArray[0] == 27))
   {
           if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 50) && (pMultiKeyArray[3] == 126)) *pSpecialKey = eCLI_KEY_INSERT;
      else if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 51) && (pMultiKeyArray[3] == 126)) *pSpecialKey = eCLI_KEY_DELETE;
      else if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 49) && (pMultiKeyArray[3] == 126)) *pSpecialKey = eCLI_KEY_HOME;
      else if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 52) && (pMultiKeyArray[3] == 126)) *pSpecialKey = eCLI_KEY_END;
      else if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 53) && (pMultiKeyArray[3] == 126)) *pSpecialKey = eCLI_KEY_PAGE_UP;
      else if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 54) && (pMultiKeyArray[3] == 126)) *pSpecialKey = eCLI_KEY_PAGE_DOWN;
      else *pSpecialKey = eCLI_KEY_UNKNOWN;
      return numBytes;
   }
   else if((numBytes == 5) && (pMultiKeyArray[0] == 27))
   {
           if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 49) && (pMultiKeyArray[3] == 49) && (pMultiKeyArray[4] == 126)) *pSpecialKey = eCLI_KEY_F1;
      else if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 49) && (pMultiKeyArray[3] == 50) && (pMultiKeyArray[4] == 126)) *pSpecialKey = eCLI_KEY_F2;
      else if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 49) && (pMultiKeyArray[3] == 51) && (pMultiKeyArray[4] == 126)) *pSpecialKey = eCLI_KEY_F3;
      else if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 49) && (pMultiKeyArray[3] == 52) && (pMultiKeyArray[4] == 126)) *pSpecialKey = eCLI_KEY_F4;
      else if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 49) && (pMultiKeyArray[3] == 53) && (pMultiKeyArray[4] == 126)) *pSpecialKey = eCLI_KEY_F5;
      else if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 49) && (pMultiKeyArray[3] == 55) && (pMultiKeyArray[4] == 126)) *pSpecialKey = eCLI_KEY_F6;
      else if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 49) && (pMultiKeyArray[3] == 56) && (pMultiKeyArray[4] == 126)) *pSpecialKey = eCLI_KEY_F7;
      else if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 49) && (pMultiKeyArray[3] == 57) && (pMultiKeyArray[4] == 126)) *pSpecialKey = eCLI_KEY_F8;
      else if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 50) && (pMultiKeyArray[3] == 48) && (pMultiKeyArray[4] == 126)) *pSpecialKey = eCLI_KEY_F9;
      else if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 50) && (pMultiKeyArray[3] == 49) && (pMultiKeyArray[4] == 126)) *pSpecialKey = eCLI_KEY_F10;
      else if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 50) && (pMultiKeyArray[3] == 51) && (pMultiKeyArray[4] == 126)) *pSpecialKey = eCLI_KEY_F11;
      else if((pMultiKeyArray[1] == 91) && (pMultiKeyArray[2] == 50) && (pMultiKeyArray[3] == 52) && (pMultiKeyArray[4] == 126)) *pSpecialKey = eCLI_KEY_F12;
      else *pSpecialKey = eCLI_KEY_UNKNOWN;
      return numBytes;
   }

   *pSpecialKey = eCLI_KEY_UNKNOWN;
   return numBytes;
}



void CJCli::ProcessCharacter( char* pInputChars, int numChars)
{
   int charsProcessed=0;

   if( numChars == 0)
   {
      dpConsole->Printf("ERROR: Procssing char when numChars=0\n");
      return;
   }

   while (charsProcessed < numChars)
   {
      if ( IsReturnChar(pInputChars[charsProcessed])) 
      {
         charsProcessed++;
      }
      else if ( IsLineFeedChar(pInputChars[charsProcessed]))
      {
         // Complete the line buffer, add to history, and parse the command line
         if ( dpConsole->GetConsoleType() == eConsoleType_LinuxTerminal)
         {
             // Echo the line feed
             PrintLineFeed();
         }
         charsProcessed++;
         dLineBuffer[dLineBufferCount] = '\0';
         ParseCommandLine( dLineBuffer, dLineBufferCount);
         StartNewLine();
     }
     else if( IsStandardASCIIChar(pInputChars[charsProcessed]))
     {
         if( dLineBufferCount < MAX_LINE_BUFFER_SIZE)
         {
            if( dLineBufferPosition < dLineBufferCount)
            {
               // Clear the display line
               MovePositionToStartOfLine(FALSE);
   
               // Adjust the line buffer (NOTE: memcpy will not work, it overwrites itself as it's copying, use a manual copy starting at the back of the buffer)
               //memcpy(&dLineBuffer[dLineBufferPosition+1], &dLineBuffer[dLineBufferPosition], dLineBufferCount-dLineBufferPosition);
               for(int i=0; i<dLineBufferCount-dLineBufferPosition; i++)
               {
                  dLineBuffer[dLineBufferCount-i] = dLineBuffer[dLineBufferCount-i-1];
               }

               dLineBuffer[dLineBufferPosition] = pInputChars[charsProcessed];
               dLineBufferPosition++;
               dLineBufferCount++;
   
               // Print the line buffer
               PrintLineBuffer();
   
               // Move the cursor back to the correct position
               for( int i=0; i < dLineBufferCount - dLineBufferPosition; i++)
               {
                  dpConsole->PrintChar(0x08);
               }
            }
            else
            {
               // Add the char to the line buf and echo it to the terminal
               if ( dpConsole->GetConsoleType() == eConsoleType_LinuxTerminal)
               {
                   // Echo the char
                   dpConsole->PrintChar(pInputChars[charsProcessed]);
               }
               dLineBuffer[dLineBufferPosition] = pInputChars[charsProcessed];
               dLineBufferPosition++;
               if( dLineBufferPosition > dLineBufferCount)
               {
                  dLineBufferCount++;
               }
            }
         }
         charsProcessed++;
      }
	  else if (IsBackspaceKey(pInputChars[charsProcessed]))
	  {
		   HandleBackspaceKey();
		   charsProcessed++;
	  }
      else if (IsSpecialChar(pInputChars[charsProcessed]))
      {
         charsProcessed++;
         HandleSpecialKeyPress((CliSpecialKeys) pInputChars[charsProcessed]);
         charsProcessed++;
      }      
      else if( IsCtrlChar(pInputChars[charsProcessed]))
      {
         HandleCtrlKeyPress( ConvertCtlrCharToStandard(pInputChars[charsProcessed]));
         charsProcessed++;
         //StartNewLine();
      }
      else if( IsEscChar(pInputChars[charsProcessed]))
      {
         CliSpecialKeys specialKey;
         charsProcessed += ConvertEscCharToSpecial( &pInputChars[charsProcessed], numChars-charsProcessed, &specialKey);
         if( IsDeleteKey(specialKey))
         {
            HandleDeleteKey();
         }
         else if( IsArrowKey(specialKey))
         {
            HandleArrowKey(specialKey);
         }
         else
         {
            HandleSpecialKeyPress(specialKey);
            StartNewLine();
         }
      }
      
      else
      {
         // Unknown Character Value
         dpConsole->Printf("###0x%X", pInputChars[charsProcessed]);
         if( dLineBufferCount < MAX_LINE_BUFFER_SIZE)
         {
            dLineBuffer[dLineBufferCount] = pInputChars[charsProcessed];
            dLineBufferCount++;
            charsProcessed++;
         }
      }
   }   
}

void CJCli::HandleCtrlKeyPress( char ctrlChar)
{
   dpConsole->Printf("Ctrl-%c", ctrlChar);
   dpConsole->Flush();
}

void CJCli::HandleSpecialKeyPress( CliSpecialKeys value)
{
   switch(value)
   {
#ifdef CJLINUX
   case eCLI_KEY_UNKNOWN:           dpConsole->Printf("ESC Char: eCLI_KEY_UNKNOWN"); break;
#else
   case eCLI_KEY_UNKNOWN:           HandleBackspaceKey(); break;
#endif
   case eCLI_KEY_ESC:               dpConsole->Printf("ESC Char: eCLI_KEY_ESC"); break;
   case eCLI_KEY_F1:                dpConsole->Printf("ESC Char: eCLI_KEY_F1"); break;
   case eCLI_KEY_F2:                dpConsole->Printf("ESC Char: eCLI_KEY_F2"); break;
   case eCLI_KEY_F3:                dpConsole->Printf("ESC Char: eCLI_KEY_F3"); break;
   case eCLI_KEY_F4:                dpConsole->Printf("ESC Char: eCLI_KEY_F4"); break;
   case eCLI_KEY_F5:                dpConsole->Printf("ESC Char: eCLI_KEY_F5"); break;
   case eCLI_KEY_F6:                dpConsole->Printf("ESC Char: eCLI_KEY_F6"); break;
   case eCLI_KEY_F7:                dpConsole->Printf("ESC Char: eCLI_KEY_F7"); break;
   case eCLI_KEY_F8:                dpConsole->Printf("ESC Char: eCLI_KEY_F8"); break;
   case eCLI_KEY_F9:                dpConsole->Printf("ESC Char: eCLI_KEY_F9"); break;
   case eCLI_KEY_F10:               dpConsole->Printf("ESC Char: eCLI_KEY_F10"); break;
   case eCLI_KEY_F11:               dpConsole->Printf("ESC Char: eCLI_KEY_F11"); break;
   case eCLI_KEY_F12:               dpConsole->Printf("ESC Char: eCLI_KEY_F12"); break;

   case eCLI_KEY_UP_ARROW:   
   case eCLI_KEY_DOWN_ARROW:    
   case eCLI_KEY_RIGHT_ARROW:   
   case eCLI_KEY_LEFT_ARROW:    
      HandleArrowKey(value);
      break;

   case eCLI_KEY_PAGE_UP:           dpConsole->Printf("ESC Char: eCLI_KEY_PAGE_UP"); break;
   case eCLI_KEY_PAGE_DOWN:         dpConsole->Printf("ESC Char: eCLI_KEY_PAGE_DOWN"); break;
   case eCLI_KEY_HOME:              dpConsole->Printf("ESC Char: eCLI_KEY_HOME"); break;
   case eCLI_KEY_END:               dpConsole->Printf("ESC Char: eCLI_KEY_END"); break;
   case eCLI_KEY_INSERT:            dpConsole->Printf("ESC Char: eCLI_KEY_INSERT"); break;
   case eCLI_KEY_DELETE:            dpConsole->Printf("ESC Char: eCLI_KEY_DELETE"); break;
   default:                         dpConsole->Printf("ERROR: GOT UNKNOWN SPECIAL KEY ENUM (value=%d)", value); break;
   }
   dpConsole->Flush();
}  


void CJCli::HandleBackspaceKey()
{
   if( dLineBufferPosition > 0)
   {
      // Clear the display line
      MovePositionToStartOfLine(TRUE);

      // Adjust the line buffer
      memcpy(&dLineBuffer[dLineBufferPosition-1], &dLineBuffer[dLineBufferPosition], dLineBufferCount-dLineBufferPosition);
      dLineBufferPosition--;
      dLineBufferCount--;

      // Print the line buffer
      PrintLineBuffer();

      // Move the cursor back to the correct position
      dpConsole->MoveCursorBack(dLineBufferCount - dLineBufferPosition);
   }
}

void CJCli::HandleDeleteKey()
{
   if( dLineBufferPosition < dLineBufferCount)
   {
      // Clear the display line
      MovePositionToStartOfLine(TRUE);

      // Adjust the line buffer
      memcpy(&dLineBuffer[dLineBufferPosition], &dLineBuffer[dLineBufferPosition+1], dLineBufferCount-dLineBufferPosition);
      dLineBufferCount--;

      // Print the line buffer
      PrintLineBuffer();

      // Move the cursor back to the correct position
      dpConsole->MoveCursorBack(dLineBufferCount - dLineBufferPosition);
    
   }
}

void CJCli::HandleArrowKey( CliSpecialKeys value)
{
   switch(value)
   {
   case eCLI_KEY_UP_ARROW:    
      // Decrement the History Buffer Temp Index
      if((dHistoryBufferTempIndex - 1) < 0)
         dHistoryBufferTempIndex = MAX_HISTORY_ENTRIES - 1;
      else
         dHistoryBufferTempIndex--;
      LoadHistoryBufferToLineBuffer( dHistoryBufferTempIndex);
      break;
   case eCLI_KEY_DOWN_ARROW:  
      // Increment the History Buffer Temp Index
      if((dHistoryBufferTempIndex + 1) >= MAX_HISTORY_ENTRIES)
         dHistoryBufferTempIndex = 0;
      else
         dHistoryBufferTempIndex++;
      LoadHistoryBufferToLineBuffer( dHistoryBufferTempIndex);
      break;
   case eCLI_KEY_RIGHT_ARROW: 
      if( dLineBufferPosition < dLineBufferCount)
      {
         dpConsole->PrintChar(dLineBuffer[dLineBufferPosition]);
         dLineBufferPosition++;
      }
      break;
   case eCLI_KEY_LEFT_ARROW:
      if( dLineBufferPosition > 0)
      {
         dLineBufferPosition--;
         dpConsole->PrintChar(0x08);
      }
      break;
   default:
      dpConsole->Printf("\nERROR: Got an Unknown Arrow Key\n");
      StartNewLine();
      break;
   }
}


void CJCli::ParseCommandLine( char* pLineBuffer, int numChars)
{
   //dpConsole->Printf("PARSING LINE (cnt=%d): %s", numChars, pLineBuffer);
   CliReturnCode cliRc = eCliReturn_UnknownCommand;
   CliCommand* pUserCommand;
   int cliCommandSetIndex = 0;
   CliParams paramstruct;
   enum ParsingState
   {
      eSearching,
      eCollectingParam
   } state = eSearching;

   paramstruct.numParams = 0;

   // Copy the raw buffer to the param struct format
   for( int i=0; i < numChars; i++)
   {
      paramstruct.rawBuffer[i] = pLineBuffer[i];
      if((pLineBuffer[i] == ' ') && (state == eSearching))
      {
         paramstruct.parsedBuffer[i] = '\0';
      }
      else if((pLineBuffer[i] == ' ') && (state == eCollectingParam))
      {
         state = eSearching;
         paramstruct.parsedBuffer[i] = '\0';
      }
      else if(state == eSearching)
      {
         state = eCollectingParam;
         paramstruct.str[paramstruct.numParams] = &paramstruct.parsedBuffer[i];
         paramstruct.numParams++;
         paramstruct.parsedBuffer[i] = pLineBuffer[i];
      }
      else //if(state == eCollectingParam )
      {
         paramstruct.parsedBuffer[i] = pLineBuffer[i];
      }   
   }
   // Null terminate the end
   paramstruct.rawBuffer[numChars] = '\0';
   paramstruct.parsedBuffer[numChars] = '\0';
   paramstruct.str[paramstruct.numParams] = NULL;

   if( paramstruct.numParams == 0)
   {
      //dpConsole->Printf("ERROR: Failed to parse any parameters from command line\n");
      return;
   }

   while (dpUserCliCommandSetAry[cliCommandSetIndex] != NULL)
   {
      pUserCommand = dpUserCliCommandSetAry[cliCommandSetIndex];
      while (pUserCommand->command_name[0] != 0)
      {
         if( strcmp( paramstruct.str[0], pUserCommand->command_name) == 0)
         {
            spActiveCli = this;  // Set this up, in the cases where the static CLI command handlers need to know which CLI is calling the command
            cliRc = pUserCommand->cbCliFunc(dpConsole, pUserCommand, &paramstruct);
            spActiveCli = NULL;
            break;
         }
         pUserCommand++;
      }
      cliCommandSetIndex++;
   }

   switch(cliRc)
   {
   case eCliReturn_Success:
      dpConsole->Printf("Command Complete: SUCCESS\n\n");
      break;
   case eCliReturn_InvalidParam:
      dpConsole->Printf("Command Complete: Invalid Paramters\n\n");
      break;
   case eCliReturn_UnknownCommand:
      dpConsole->Printf("Error: Unknown Command\n\n");
      break;
   case eCliReturn_Failed:
      dpConsole->Printf("Command Complete: FAILED\n\n");
      break;
   default:
      dpConsole->Printf("Command Complete: UNKNOWN RETURN CODE\n\n");
      break;   
   }
}


void CJCli::StartNewLine()
{
   AddLineBufferToHistoryBuffer();
   dHistoryBufferTempIndex = dHistoryBufferNextIndex;
   dLineBufferCount=0;
   dLineBufferPosition=0;
   //PrintLineFeed();
   PrintPrompt();
}


void CJCli::PrintLineBuffer( BOOL doFlush)
{
   for(int i=0; i < dLineBufferCount; i++)
   {
      dpConsole->PrintChar(dLineBuffer[i], FALSE);
   }
   if( doFlush)
   {
      dpConsole->Flush();
   }
}


void CJCli::MovePositionToStartOfLine(BOOL clearLineDisplay)
{
   

   int i;
   /*for( i=dLineBufferPosition; i > 0; i--)
   {
	   
      dpConsole->PrintChar(0x08, FALSE);
   }*/
   dpConsole->MoveCursorBack(dLineBufferPosition);

   if( clearLineDisplay)
   {
      for(i=0; i < dLineBufferCount; i++)
      {
         dpConsole->PrintChar(' ', FALSE);
      }
      /*for(i=0; i < dLineBufferCount; i++)
      {
         dpConsole->PrintChar(0x08, FALSE);
      }*/
	   dpConsole->MoveCursorBack(dLineBufferCount);
   }
   
   dpConsole->Flush();
}


void CJCli::AddLineBufferToHistoryBuffer() 
{
   if( dLineBufferCount > 0)
   {
      memcpy(dHistoryBuffer[dHistoryBufferNextIndex], dLineBuffer, dLineBufferCount);
      dHistoryBuffer[dHistoryBufferNextIndex][dLineBufferCount] = '\0';
      dHistoryBufferSizes[dHistoryBufferNextIndex] = dLineBufferCount;
      dHistoryBufferNextIndex++;
      if( dHistoryBufferNextIndex >= MAX_HISTORY_ENTRIES)
         dHistoryBufferNextIndex = 0;
      
      if(dHistoryBufferCount < MAX_HISTORY_ENTRIES)
         dHistoryBufferCount++;
   }
}

void CJCli::LoadHistoryBufferToLineBuffer( int index) 
{
   MovePositionToStartOfLine(TRUE);
   memcpy( dLineBuffer, dHistoryBuffer[index], dHistoryBufferSizes[index]);
   dLineBufferCount = dHistoryBufferSizes[index];
   dLineBufferPosition = dLineBufferCount;
   PrintLineBuffer( TRUE);
}


void CJCli::DisplayHistoryBuffer(CJConsole* pLocalConsole)
{
   int i;
   int tempIndex = dHistoryBufferNextIndex;
   pLocalConsole->Printf("  CLI History:\n");
   

   for( i=0; i < dHistoryBufferCount ; i++)
   {
      // Decrement the index
      if( tempIndex == 0) 
         tempIndex = MAX_HISTORY_ENTRIES - 1;
      else
         tempIndex--;
   }
   for( i=0; i < dHistoryBufferCount ; i++)
   {
      // Print each entry
      pLocalConsole->Printf("    %s\n", dHistoryBuffer[i]);
      // Increment the index
      tempIndex++;
      if( tempIndex == MAX_HISTORY_ENTRIES) 
         tempIndex = 0;
   }

} 

void CJCli::DisplayRegisteredCliCommands(CJConsole* pLocalConsole)
{
   CliCommand* pUserCommand;
   int cliCommandSetIndex = 0;

   pLocalConsole->Printf("  CLI Commands:\n");
   while (dpUserCliCommandSetAry[cliCommandSetIndex] != NULL)
   {
      pUserCommand = dpUserCliCommandSetAry[cliCommandSetIndex];   
      while (pUserCommand->command_name[0] != 0)
      {
         pLocalConsole->Printf("    %-24s  %s\n", pUserCommand->command_name, pUserCommand->short_desc);
         pUserCommand++;
      }
      cliCommandSetIndex++;
   }
}



