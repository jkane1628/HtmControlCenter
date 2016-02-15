// CJConsole.cpp : Implmentation file
//

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>

#include "CJTrace.h"
#include "CJSocket.h"
#include "CJConsole.h"


#define CLI_CONSOLE_STDIN stdin
#define CLI_CONSOLE_STDOUT stdout





void CJConsole::Printf(char const* msg, ...)
{
   va_list argList;
   va_start(argList, msg);
   vPrintf((char*)msg, argList);
   va_end(argList);
}

void CJConsole::Printf(char* msg, ...)
{
   va_list argList;
   va_start(argList, msg);
   vPrintf(msg, argList);
   va_end(argList);
}

void CJConsole::MoveCursorBack( int numSpaceToMoveBack)
{
    for ( int i=0; i<numSpaceToMoveBack; i++)
        Printf("\033[2D");
}


CJTerminalConsole::CJTerminalConsole(char const* pConsoleNameStr) : CJConsole(pConsoleNameStr)
{
   CJTRACE_REGISTER_CONSOLE(this);
   dType = eConsoleType_LinuxTerminal;

   // Setup for STDIN and STDOUT
   dInputFileDescriptor = fileno(CLI_CONSOLE_STDIN);
   dOutputFileDescriptor = fileno(CLI_CONSOLE_STDOUT);

   // Setup STDIN to capture every keyboard button press (no line buffering)
   tcgetattr(STDIN_FILENO, &dOldTermAttr);
   dNewTermAttr = dOldTermAttr;
   dNewTermAttr.c_lflag &= ~(ICANON | ECHO);
   tcsetattr(STDIN_FILENO, TCSANOW, &dNewTermAttr);

}

CJTerminalConsole::~CJTerminalConsole()
{
   // Re-enable echoing and other settting that were changed in the constructor
   tcgetattr(STDIN_FILENO, &dOldTermAttr);
   dNewTermAttr = dOldTermAttr;
   dNewTermAttr.c_lflag |= (ICANON | ECHO);
   tcsetattr(STDIN_FILENO, TCSANOW, &dNewTermAttr);
}

BOOL CJTerminalConsole::InitializeConsoleConnection() {return TRUE;}

void CJTerminalConsole::vPrintf(char* msg, va_list ap)
{
   vfprintf(CLI_CONSOLE_STDOUT, msg, ap);
}

void CJTerminalConsole::PrintChar(char val, BOOL doFlush)
{
   fprintf(CLI_CONSOLE_STDOUT, "%c", val);
   if (doFlush)
   {
      Flush();
   }
}

void CJTerminalConsole::Flush()
{
   fflush(CLI_CONSOLE_STDOUT);
}

int CJTerminalConsole::WaitAndGetChars(char* pInputChars, int maxNumChars)
{
   // Poll on the input file, then read a single char
   struct pollfd fileDesc;
   int nfds;
   int retval;
   int timeout_ms = 1000;
   //char inputChar[256];
   int byteCount;
   int timeoutCount = 0;

   while (1) //dState == eCLI_STATE_RUNNING)
   {
      nfds = 1;
      fileDesc.fd = dInputFileDescriptor;
      fileDesc.events = POLLIN;
      fileDesc.revents = 0;

      retval = poll(&fileDesc, nfds, timeout_ms);
      if (retval == 0)
      {
         // Timed out, just continue...could use this as a CLI idle timer
         timeoutCount++;
      }
      else if (retval < 0 && errno == EINTR)
      {
         //Got an interrupt, this could be expected
         Printf((char*)"WARNING: poll() ERRNO=EINTR\n");
      }
      else if (retval < 0)
      {
         // Poll() failed??
         if (errno == EBADF || errno == EFAULT || errno == EINVAL || errno == ENOMEM)
         {
            Printf((char*)"ERROR: poll(): rc=%d, errno=(%d)%s", retval, errno, strerror(errno));
         }
      }
      else
      {
         // Everything is fine, read the data
         byteCount = read(dInputFileDescriptor, pInputChars, maxNumChars);
         return byteCount;
      }
   }
   return 0;
}





CJSocketConsole::CJSocketConsole(char const* pConsoleNameStr) : CJConsole(pConsoleNameStr)
{
    CJTRACE_REGISTER_CONSOLE(this);
    dType = eConsoleType_Telnet;
    dpSocket = new CJSocket(pConsoleNameStr);
}

CJSocketConsole::~CJSocketConsole()
{
    delete dpSocket;
}

BOOL CJSocketConsole::InitializeConsoleConnection() 
{
    if( dpSocket->InitializeServerSocket(40000) != eSocket_Success)
        return FALSE;

    if( dpSocket->AcceptConnection() != eSocket_Success)
        return FALSE;

    return TRUE;
}

void CJSocketConsole::ResetConsole()
{
    delete dpSocket;
    dpSocket = new CJSocket(GetObjectStr());
}

void CJSocketConsole::vPrintf(char* msg, va_list ap)
{
    char strbuf[1024];
    int dataSent;
    int totalDataSent = 0;
    int strlength = vsnprintf( strbuf, 1024, msg, ap);

    while ( totalDataSent != strlength) 
    {
        dataSent = dpSocket->SendData(strbuf+totalDataSent, strlength-totalDataSent);
        if ( dataSent == -1)
            break;
        totalDataSent += dataSent;
    }
}

void CJSocketConsole::PrintChar(char val, BOOL doFlush)
{
    dpSocket->SendData( &val, 1);
}

void CJSocketConsole::Flush()
{

}

int CJSocketConsole::WaitAndGetChars(char* pInputChars, int maxNumChars)
{
    return dpSocket->ReceiveData( pInputChars, maxNumChars);
}


