//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief   Not to be confused with the Windows file of the same name. Contains
///          equivalent definitions for Windows-specific data types.
//==============================================================================

#ifndef GPS_WINDEFS
#define GPS_WINDEFS

#include <stdarg.h>
#include <stdlib.h>
#include <cstdint>
#include "Types.h"
#include "SafeCRT.h"

// Windows definition equivalents for Linux

#define NO_ERROR  0

// windows message box flags
#define MB_OK              0x00000000
#define MB_ICONSTOP        0x00000010
#define MB_ICONERROR       0x00000010
#define MB_ICONWARNING     0x00000030
#define MB_ICONINFORMATION 0x00000040
#define MB_TASKMODAL       0x00002000
#define MB_SETFOREGROUND   0x00010000

#define _TRUNCATE ((size_t)-1)
#define SOCKET_ERROR -1

#define HAVE_BOOLEAN 1
#ifndef TRUE
    #define TRUE 1
#endif
#ifndef FALSE
    #define FALSE 0
#endif
typedef int boolean;

#ifndef FLT_MIN
    #define FLT_MIN __FLT_MIN__
#endif
#ifndef FLT_MAX
    #define FLT_MAX __FLT_MAX__
#endif

// taken from a WinBase.h/WinNT.h
#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))

// Data types
typedef unsigned short int  WORD;
typedef unsigned int        DWORD;
typedef int                 LONG;
typedef void*               HANDLE;
typedef const char*         LPCTSTR;
typedef unsigned int        LRESULT;
typedef int                 INT;
typedef unsigned int        UINT;
typedef unsigned short int  USHORT;
typedef unsigned char       BYTE;
typedef void*               PVOID;
typedef bool                BOOL;
typedef char                _TCHAR;

typedef int                 INT32;
typedef unsigned int        UINT32;
typedef long long           LONGLONG;

typedef union
{
    uint64 QuadPart;
} LARGE_INTEGER;

typedef struct timeval      TIMEVAL;

typedef WORD                WPARAM;
typedef DWORD               LPARAM;

#endif // def GPS_WINDEFS

