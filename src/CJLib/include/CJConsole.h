// CJConsole.h : Header file
//

#ifndef __CJCONSOLE_H_
#define __CJCONSOLE_H_

#include <stdarg.h>


#include "CJTypes.h"
#include "CJConfig.h"
#include "CJObject.h"

enum eConsoleType
{
    eConsoleType_LinuxTerminal,
    eConsoleType_Telnet
};

class CJConsole : public CJObject
{
public:
   CJConsole(char const* pConsoleNameStr) : CJObject("LIB::CN::", pConsoleNameStr) {};

   
   void Printf(char const*msg, ...);
   void Printf(char *msg, ...);
   void MoveCursorBack( int numSpaceToMoveBack);
   eConsoleType GetConsoleType() { return dType;}

   virtual BOOL InitializeConsoleConnection() =0;
   virtual void ResetConsole() =0;
   virtual void ShutdownConsole() =0;
   virtual void PrintString(char const*msg) =0;
   virtual void vPrintf(char const *msg, va_list ap) =0;
   virtual void PrintChar(char val, BOOL doFlush=TRUE) =0;
   virtual void Flush() =0;  
   virtual int  WaitAndGetChars( char* pInputChars, int maxNumChars) =0;
   
protected:
    eConsoleType dType;
private:
    

};

class QConsoleWidget;

class CJTerminalConsole : public CJConsole
{
public:
    CJTerminalConsole( QConsoleWidget* pConsoleWidget, char const* pConsoleNameStr);
    virtual ~CJTerminalConsole( );

    BOOL InitializeConsoleConnection();
    void ResetConsole() {};
    void ShutdownConsole() {};
    void PrintString(char const*msg);
    void vPrintf(char const *msg, va_list ap);
    void PrintChar(char val, BOOL doFlush=TRUE);
    void Flush(); 
    int  WaitAndGetChars( char* pInputChars, int maxNumChars);

private:
    int dInputFileDescriptor;
    int dOutputFileDescriptor;
    
    QConsoleWidget* dpConsoleWidget;
};

class CJSocket;

class CJSocketConsole : public CJConsole
{
public:
   CJSocketConsole(char const* pConsoleNameStr);
   virtual ~CJSocketConsole( );

   BOOL InitializeConsoleConnection();
   void ResetConsole();
   void ShutdownConsole() {};
   void PrintString(char const*msg);
   void vPrintf(char *msg, va_list ap);
   void PrintChar(char val, BOOL doFlush=TRUE);
   void Flush(); 
   int  WaitAndGetChars( char* pInputChars, int maxNumChars);

private:

    CJSocket* dpSocket;
};

#endif // __CJCONSOLE_H_
