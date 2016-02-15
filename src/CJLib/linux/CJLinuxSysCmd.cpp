// CJLinuxSysCmd.cpp : Implementation File
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "CJTrace.h"
#include "CJLinuxSysCmdMgr.h"
#include "CJLinuxSysCmd.h"




//static int popenRWE(int *rwepipe, const char *exe, const char *const argv[]);
//static int pcloseRWE(int pid, int *rwepipe);

CJLinuxSysCmd::CJLinuxSysCmd() : CJObject("LIB::SYSCMD","") 
{
   memset(dInputCommandStr, 0, MAX_INPUT_COMMAND_STR_LEN);
   memset(dOutputStr, 0, MAX_OUTPUT_COMMAND_STR_LEN);
}


//CJLinuxSysCmd::~CJLinuxSysCmd();

//BOOL CJLinuxSysCmd::ExecCommand(char* command)
//{
//   //FILE* process_fp;
//   int output_size, error_size;
//
//   strncpy( dInputCommandStr, command, MAX_INPUT_COMMAND_STR_LEN);
//
//   //CJTRACE(DBG_LEVEL, "Executing cmd: %s", dInputCommandStr);
//   //fflush(NULL);
//
//
//
//
//
//
//   int rc, status;
//   int pipe[3];
//   int pid;
//   //const char *const args[] = {
//   //        "cat",
//   //        "-n",
//   //        NULL
//   //};
//
//   pid = popenRWE(pipe, dInputCommandStr, NULL);
//
//   CJTRACE(ERROR_LEVEL, "Cmd opened: (pid=%d) %s", pid, dInputCommandStr);
//
//   if( pid != -1)
//   {
//      rc = waitpid(pid, &status, 0);
//   
//      // write to pipe[0] - input to cat
//      // read from pipe[1] - output from cat
//      // read from pipe[2] - errors from cat
//       
//
//
//      output_size = read( pipe[1], dOutputStr, MAX_OUTPUT_COMMAND_STR_LEN);
//      error_size = read( pipe[2], dErrorStr, MAX_ERROR_COMMAND_STR_LEN);
//
//       
//      CJTRACE(ERROR_LEVEL, "wait rc:       %d", pid);
//      CJTRACE(ERROR_LEVEL, "PID:       %d", pid);
//      CJTRACE(ERROR_LEVEL, "Status:    %d", status);
//      CJTRACE(ERROR_LEVEL, "Input cmd: %s", dInputCommandStr);
//      CJTRACE(ERROR_LEVEL, "Output:    (%d)%s", output_size, dOutputStr);
//      CJTRACE(ERROR_LEVEL, "Error:     (%d)%s", error_size, dErrorStr);
//
//      
//   }
//
//   pcloseRWE(pid, pipe);
//
//
//
//
//   //process_fp = popen( dInputCommandStr, "r");
//   //if (process_fp != NULL)
//   //{
//   //   dLocalErrno = errno;
//   //   output_size = fread(dOutputStr, sizeof(char), sizeof(dOutputStr), process_fp);
//   //   pclose(process_fp);
//   //   dOutputStr[output_size-1] = 0;
//   //   CJTRACE(DBG_LEVEL, "Cmd Returned %u chars: %s", output_size, dOutputStr);
//   //   return TRUE;
//   //}
//   return FALSE;
//
//
//
//
//
//}



extern CJLinuxSysCmdMgr* gpLinuxSysCmdMgr;

BOOL CJLinuxSysCmd::ExecCommand(char* command, BOOL collectStdErrAndStdOut)
{
   //FILE* process_fp;
   //int output_size;
   FILE* stdout_file;
   FILE* stderr_file;
   U32   bytesWritten;
   int   rc;
   BOOL  success=TRUE;
   U32   outputId=0;
   char  stdoutFilename[32];
   char  stderrFilename[32];



   if( collectStdErrAndStdOut)
   {
      // Make stderr and stdout go to seperate files( ./aio-stress -o 2 -s 2 /dev/sdcm > stdout.out ) >& stderr.err
      outputId = gpLinuxSysCmdMgr->GetUniqueOutputId();
      sprintf( stdoutFilename, "/tmp/stdout_%u.txt", outputId);
      sprintf( stderrFilename, "/tmp/stderr_%u.txt", outputId);
      snprintf( dInputCommandStr, MAX_INPUT_COMMAND_STR_LEN + 31, "( %s > %s ) >& %s", command, stdoutFilename, stderrFilename);
   }
   else
   {
      strncpy( dInputCommandStr, command, MAX_INPUT_COMMAND_STR_LEN);
   }

   CJTRACE(TRACE_DBG_LEVEL, "Executing cmd(%u): %s ", collectStdErrAndStdOut, dInputCommandStr);

   //fflush(NULL);
   //process_fp = popen( dInputCommandStr, "r");
   //if (process_fp != NULL)
   //{
   //   dLocalErrno = errno;
   //   output_size = fread(dOutputStr, sizeof(char), sizeof(dOutputStr), process_fp);
   //   pclose(process_fp);
   //   dOutputStr[output_size-1] = 0;
   //   CJTRACE(DBG_LEVEL, "Cmd Returned %u chars: %s", output_size, dOutputStr);
   //   return TRUE;
   //}
    
   dLocalErrno = gpLinuxSysCmdMgr->SendShellCommand(dInputCommandStr);

   if( collectStdErrAndStdOut)
   {

      stdout_file = fopen(stdoutFilename, "r"); 
      if( stdout_file != NULL)
      {
         bytesWritten = 0;
         rc = fread( &dOutputStr[bytesWritten], 1, 1, stdout_file);
         while(rc > 0)
         {
            bytesWritten += rc;
            if( bytesWritten >= (MAX_OUTPUT_COMMAND_STR_LEN-1))
            {
               CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Stdout of sys command was truncated to %u bytes", bytesWritten);
               break;
            }
            rc = fread( &dOutputStr[bytesWritten], 1, 1, stdout_file);
         }
         dOutputStr[bytesWritten] = 0; // Null terminate the string
         fclose(stdout_file);

         while((bytesWritten > 0) && (dOutputStr[bytesWritten-1] == 10))
         {
            bytesWritten--;
            dOutputStr[bytesWritten] = 0; // Trim off the line feed
         }

         CJTRACE(TRACE_DBG_LEVEL, "Executing complete: stdout_%u=%s", outputId, dOutputStr);
      }
      else
      {
         CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to retrieve stdout_%u.txt for sys command", outputId);
         success = FALSE;
      }
   
      stderr_file = fopen(stderrFilename, "r"); 
      if( stderr_file != NULL)
      {
         bytesWritten = 0;
         rc = fread( &dErrorStr[bytesWritten], 1, 1, stderr_file);
         while(rc > 0)
         {
            bytesWritten += rc;
            if( bytesWritten >= (MAX_ERROR_COMMAND_STR_LEN-1))
            {
               CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Stderr of sys command was truncated to %u bytes", bytesWritten);
               break;
            }
            rc = fread( &dErrorStr[bytesWritten], 1, 1, stderr_file);
         }
         dErrorStr[bytesWritten] = 0; // Null terminate the string
         fclose(stderr_file);

         while((bytesWritten > 0) && (dOutputStr[bytesWritten-1] == 10))
         {
            bytesWritten--;
            dErrorStr[bytesWritten] = 0; // Trim off the line feed(s)
         }
      }
      else
      {
         CJTRACE(TRACE_ERROR_LEVEL, "ERROR: Failed to retrieve stderr.txt for sys command");
         success = FALSE;
      }
   }

   return success;
}







/**
 * Copyright 2009-2010 Bart Trojanowski <bart@jukie.net>
 * Licensed under GPLv2, or later, at your choosing.
 *
 * bidirectional popen() call
 *
 * @param rwepipe - int array of size three
 * @param exe - program to run
 * @param argv - argument list
 * @return pid or -1 on error
 *
 * The caller passes in an array of three integers (rwepipe), on successful
 * execution it can then write to element 0 (stdin of exe), and read from
 * element 1 (stdout) and 2 (stderr).
 */
//static int popenRWE(int *rwepipe, const char *exe, const char *const argv[])
//{
//	int in[2];
//	int out[2];
//	int err[2];
//	int pid;
//	int rc;
//
//	rc = pipe(in);
//	if (rc<0)
//		goto error_in;
//
//	rc = pipe(out);
//	if (rc<0)
//		goto error_out;
//
//	rc = pipe(err);
//	if (rc<0)
//		goto error_err;
//
//	pid = fork();
//	if (pid > 0) { // parent
//		close(in[0]);
//		close(out[1]);
//		close(err[1]);
//		rwepipe[0] = in[1];
//		rwepipe[1] = out[0];
//		rwepipe[2] = err[0];
//		return pid;
//	} else if (pid == 0) { // child
//		close(in[1]);
//		close(out[0]);
//		close(err[0]);
//		close(0);
//		dup(in[0]);
//		close(1);
//		dup(out[1]);
//		close(2);
//		dup(err[1]);
//
//		execvp(exe, (char**)argv);
//		exit(1);
//	} else
//		goto error_fork;
//
//	return pid;
//
//error_fork:
//	close(err[0]);
//	close(err[1]);
//error_err:
//	close(out[0]);
//	close(out[1]);
//error_out:
//	close(in[0]);
//	close(in[1]);
//error_in:
//	return -1;
//}
//
//static int pcloseRWE(int pid, int *rwepipe)
//{
//	int rc, status;
//	close(rwepipe[0]);
//	close(rwepipe[1]);
//	close(rwepipe[2]);
//	rc = waitpid(pid, &status, 0);
//	return status;
//}
