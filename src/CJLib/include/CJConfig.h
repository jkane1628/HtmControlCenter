// CJConfig.h : Header file
//

#ifndef __CJCONFIG_H_
#define __CJCONFIG_H_




//////////////////////////////////////////////////////////
// 
// Build Environment Configuration
// 
//////////////////////////////////////////////////////////
#define CJWINDOWS 1
//#define CJLINUX 1
//#define CJRASPBERRYPI 1


//////////////////////////////////////////////////////////
// 
// Build Environment Configuration
// 
//////////////////////////////////////////////////////////
#define CJLIB_USE_MODULE_TRACE 1
#define CJLIB_USE_MODULE_POOL 1
#define CJLIB_USE_MODULE_QUEUE 1
#define CJLIB_USE_MODULE_LIST 1
#define CJLIB_USE_MODULE_SEMAPHORE 1
#define CJLIB_USE_MODULE_POOLBLOCKING 1
#define CJLIB_USE_MODULE_ASSERT 1
#define CJLIB_USE_MODULE_THREAD 1
#define CJLIB_USE_MODULE_THREADMGR 1
#define CJLIB_USE_MODULE_TIME 1
#define CJLIB_USE_MODULE_CLI 1
#define CJLIB_USE_MODULE_CONSOLE 1
#define CJLIB_USE_MODULE_JOB 1
#define CJLIB_USE_MODULE_LINUXSYSCMD 1
#define CJLIB_USE_MODULE_MSGTASK 1
#define CJLIB_USE_MODULE_PERSISTCONFIG 1
#define CJLIB_USE_MODULE_PERSISTFILE 1






//////////////////////////////////////////////////////////
// 
// CJObject
// 
//////////////////////////////////////////////////////////
#define MAX_OBJ_STR_LEN 32

//////////////////////////////////////////////////////////
// 
// CJTrace
// 
//////////////////////////////////////////////////////////
#define CJTRACE_DEFAULT_TRACE_BUFFER_SIZE_BYTES 1024*1024
#define CJTRACE_DEFAULT_LEVEL TRACE_ERROR_LEVEL
#define MAX_NUM_REGISTERED_TRACE_OBJECTS 128
#define MAX_NUM_REGISTERED_CONSOLE 1

//////////////////////////////////////////////////////////
// 
// CJThread
// 
//////////////////////////////////////////////////////////
#define CJTHREAD_DEFAULT_STACK_SIZE (64*1024)

//////////////////////////////////////////////////////////
// 
// CJThreadMgr
// 
//////////////////////////////////////////////////////////
//#define USE_DEFAULT_GLOBAL_THREAD_MANAGER 1
#define MAX_NUM_CPUS_FOR_THREAD_AFFINITY 1




//////////////////////////////////////////////////////////
// 
// CJCli
// 
//////////////////////////////////////////////////////////
#define MAX_LINE_BUFFER_SIZE 256
#define MAX_NUMBER_PARAMS 16
#define MAX_HISTORY_ENTRIES 32
#define CLI_DEFAULT_PROMPT "JOS>"
#define MAX_NUM_USER_CLI_COMMAND_SETS 64


//////////////////////////////////////////////////////////
// 
// CJPersistFile
//  
//////////////////////////////////////////////////////////
#define MAX_NUM_TOTAL_TAGS_PER_FILE 16


#endif //__CJCONFIG_H_
