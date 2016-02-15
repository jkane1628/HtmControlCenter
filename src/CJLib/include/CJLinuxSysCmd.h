// CJLinuxSysCmd.h : Header file
//

#ifndef __CJLINUXSYSCMD_H_
#define __CJLINUXSYSCMD_H_

#include "CJTypes.h"
#include "CJObject.h"

#define MAX_INPUT_COMMAND_STR_LEN  256
#define MAX_OUTPUT_COMMAND_STR_LEN 1024
#define MAX_ERROR_COMMAND_STR_LEN  1024

class CJLinuxSysCmd : public CJObject
{
public:
   CJLinuxSysCmd();
   //~CJLinuxSysCmd();

   BOOL ExecCommand(char* command, BOOL collectStdErrAndStdOut=TRUE);

   char* GetInputCommand() {return dInputCommandStr;}
   char* GetOutputStr() {return dOutputStr;}
   int   GetErrno() { return dLocalErrno;}

private:

   int  dLocalErrno;
   char dInputCommandStr[MAX_INPUT_COMMAND_STR_LEN+32];
   char dOutputStr[MAX_OUTPUT_COMMAND_STR_LEN];
   char dErrorStr[MAX_ERROR_COMMAND_STR_LEN];

};


#endif // __CJLINUXSYSCMD_H_
