// CJTypes.h : Header file
//

#ifndef __CJTYPES_H_
#define __CJTYPES_H_


#include "CJConfig.h"
#include "stdint.h"


#ifdef CJWINDOWS
#include <windows.h>
#endif 

#ifndef FALSE
   #define FALSE 0
#endif
#ifndef TRUE
   #define TRUE 1
#endif

#ifndef NULL
   #define NULL 0
#endif

/* -------------------------------------------------
 * Basic Sized Types
 * -----------------------------------------------*/
typedef int                BOOL;
typedef char               S8;
typedef unsigned char      U8;
typedef short              S16;
typedef unsigned short     U16;
typedef int                S32;
typedef unsigned int       U32;
typedef int64_t            S64;
typedef uint64_t           U64;

typedef  signed char        int8_t;
typedef  signed short       int16_t;
typedef  signed int         int32_t;
//typedef  signed long long   int64_t;

#define SWAP_DATA16(x)  ((((x) & 0xFF) << 8) | ((x) >> 8))
#define SWAP_DATA32(x)  (((x & 0xFF) << 24) | ((x & 0xFF00) << 8) | ((x & 0xFF0000) >> 8) | ((x & 0xFF000000) >> 24))

#define DIM_OF(arr)  (sizeof(arr)/sizeof(arr[0]))
#define MIN(a,b)  ( ((a) < (b)) ? (a) : (b) )
#define MAX(a,b)  ( ((a) > (b)) ? (a) : (b) )


#endif
