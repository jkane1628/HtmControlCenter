// CJConsole.cpp : Implmentation file for Windows shim to QT widget
//

#include "CJSem.h"
#include "CJTrace.h"
#include "consoledock.h"
#include "CJConsole.h"




void CJConsole::Printf(char const* msg, ...)
{
   va_list argList;
   va_start(argList, msg);
   vPrintf(msg, argList);
   va_end(argList);
}

void CJConsole::Printf(char * msg, ...)
{
   va_list argList;
   va_start(argList, msg);
   vPrintf(msg, argList);
   va_end(argList);
}

void CJConsole::MoveCursorBack(int numSpaceToMoveBack)
{
   for (int i = 0; i < numSpaceToMoveBack; i++)
      PrintChar(0x08);
}

CJTerminalConsole::CJTerminalConsole(QConsoleWidget* pConsoleWidget, char const* pConsoleNameStr) : CJConsole(pConsoleNameStr)
{
	CJTRACE_REGISTER_CONSOLE(this);
	dType = eConsoleType_LinuxTerminal;
   dpConsoleWidget = pConsoleWidget;
}

CJTerminalConsole::~CJTerminalConsole() {}


BOOL CJTerminalConsole::InitializeConsoleConnection() { return TRUE; }
//void CJTerminalConsole::ResetConsole() {}
//void CJTerminalConsole::ShutdownConsole() {}

void CJTerminalConsole::vPrintf(char const *msg, va_list ap)
{
	QString outputStr;
	outputStr.vsprintf(msg, ap);

   dpConsoleWidget->ExternalThreadInsertString(outputStr);
}

void CJTerminalConsole::PrintChar(char val, BOOL doFlush)
{
	QString outputChar(val);
   dpConsoleWidget->ExternalThreadInsertString(outputChar);
}

void CJTerminalConsole::PrintString(char const* msg)
{
   dpConsoleWidget->ExternalThreadInsertString(QString::fromLatin1(msg));
}

void CJTerminalConsole::Flush()
{}

int CJTerminalConsole::WaitAndGetChars(char* pInputChars, int maxNumChars)
{
   int copybuffer[256];

   if (dpConsoleWidget->mCJConsoleInputBufferCount == 0)
   {	// Wait until there is data in the input buffer
      dpConsoleWidget->mpCJSemInputBufferNotEmpty->Wait();
   }

   dpConsoleWidget->mpCJSemInputBufferLock->Wait();

   int numToProcess = MIN(maxNumChars, dpConsoleWidget->mCJConsoleInputBufferCount);

	// Copy the input buffer into the CLI
   for (int i = 0; i < numToProcess; i++)
	{
      pInputChars[i] = dpConsoleWidget->mCJConsoleInputBuffer[i];
      dpConsoleWidget->mCJConsoleInputBufferCount--;
	}

   if (dpConsoleWidget->mCJConsoleInputBufferCount > 0)
   {
      // Copy the remaining data forward
      memcpy(copybuffer, &dpConsoleWidget->mCJConsoleInputBuffer[numToProcess], dpConsoleWidget->mCJConsoleInputBufferCount*sizeof(int));
      memcpy(&dpConsoleWidget->mCJConsoleInputBuffer[0], &dpConsoleWidget->mCJConsoleInputBuffer[numToProcess], dpConsoleWidget->mCJConsoleInputBufferCount*sizeof(int));
   }

   dpConsoleWidget->mpCJSemInputBufferLock->Post();
   return numToProcess;
}