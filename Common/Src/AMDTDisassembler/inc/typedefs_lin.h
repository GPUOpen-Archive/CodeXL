//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file typedefs_lin.h
/// \brief This file contains all the simple type definitions, and some cross-compiler definitions.
///
//==================================================================================

#ifndef _TYPEDEFS_LIN_H_
#define _TYPEDEFS_LIN_H_

#include <stdint.h>


#define MAX_UINT64      0xffffffffffffffffULL
#define MAX_INT64       0x7fffffffffffffffLL
#define MIN_UINT64      0x0000000000000000ULL
#define MIN_INT64       0x8000000000000000LL

#define FMT64           "ll"

#define sign64(num64)       num64##ll
#define usign64(num64)      num64##ull

typedef long long unsigned int  UINT64;
typedef int64_t         INT64;
typedef unsigned int    DWORD;
typedef unsigned char   UCHAR;
typedef uint16_t        USHORT;
typedef uint16_t        UINT16;
typedef unsigned char   UINT8;
typedef unsigned int    UINT;
typedef unsigned int    UINT32;

typedef bool            BOOL;
#define TRUE            true
#define FALSE           false
typedef unsigned char   BYTE;

#ifndef S_OK
    typedef unsigned int  HRESULT;

    #define S_OK            ((HRESULT)(0))
    #define S_FALSE         ((HRESULT)(1))
    #define E_FAIL          ((HRESULT)(0x80004005))
#endif

#define strncpy_s(strDest,numberOfElements,strSource,count) strncpy(strDest, strSource, count)
#define strcpy_s(strDest,numberOfElements,strSource) strcpy(strDest, strSource)

#endif //_TYPEDEFS_LIN_H_
