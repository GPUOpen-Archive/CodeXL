//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Basic types definitions
//==============================================================================

#ifndef _TYPES_H
#define _TYPES_H

// DEFINES ===============================================================================================================================================
#define SU_DLL

// TYPEDEFS ==============================================================================================================================================
#if defined( _WINDOWS ) || defined( WIN32 )
    //======================================================
    // 32-bit Windows using the Visual Studio 2005 compiler
    //======================================================

    // for TCHAR type
    #include <tchar.h>

    // tolua_begin

    //Signed
    typedef char char8;
    typedef wchar_t char16;

    typedef char    int8;
    typedef short   int16;
    typedef int     int32;
    typedef __int64 int64;

    typedef float       float32;
    typedef double      float64;

    //Unsigned
    typedef unsigned char    uint8;
    typedef unsigned short   uint16;
    typedef unsigned int     uint32;
    typedef unsigned __int64 uint64;

    //Handle
    // this used to be unsigned __int64, but I changed it because tolua was generating bad code for it -- JDB
    typedef __int64 SuHandle;

    //Time
    typedef double           SuTime;

    // tolua_end
#elif defined ( MACOSX )
    //=====================
    // MAC using GCC 4
    //=====================

    //#define SU_USE_NATIVE_STL

    //Signed
    typedef char char8;
    typedef wchar_t char16;
    typedef char TCHAR;  // no unicode

    typedef char  int8;
    typedef short int16;
    typedef int  int32;
    typedef long long int64;

    typedef float       float32;
    typedef double      float64;

    //Unsigned
    typedef unsigned char  uint8;
    typedef unsigned short uint16;
    typedef unsigned int  uint32;
    typedef unsigned long long  uint64;
    typedef unsigned long  LARGE_INTEGER;

    typedef unsigned int   UINT;

    // we need this definition for lua
    #define __int64 long long

    //Handle
    typedef __int64 SuHandle;

    //Time
    typedef double           SuTime;

    #if defined (_T)
        #error "_T already defined. I'm confused."
    #else
        #define _T(x) (x)
    #endif

#elif defined(_LINUX)
    //=====================
    // Linux using GCC
    //=====================

    //Signed
    typedef char char8;
    typedef short char16;
    typedef char TCHAR;  // no unicode

    typedef char      int8;
    typedef short     int16;
    typedef int       int32;
    typedef long      int64;

    typedef float     float32;
    typedef double    float64;

    //Unsigned
    typedef unsigned char      uint8;
    typedef unsigned short     uint16;
    typedef unsigned int       uint32;
    typedef unsigned long      uint64;

    //Handle
    typedef unsigned long long SuHandle;

    //Time
    typedef double           SuTime;

    #if defined (_T)
        #error "_T already defined. I'm confused."
    #else
        #define _T(x) L##x
    #endif

#elif defined (__SYMBIAN32__)

    //======================
    // SYMBIAN using GCC 2.9
    //======================
    #include <stddef.h>

    //Signed
    typedef char char8;
    typedef wchar_t char16;
    #ifdef _UNICODE
        typedef wchar_t TCHAR;
    #else
        typedef char TCHAR;
    #endif

    typedef char      int8;
    typedef short     int16;
    typedef int       int32;
    typedef long long int64;

    typedef float     float32;
    typedef double    float64;

    //Unsigned
    typedef unsigned char      uint8;
    typedef unsigned short     uint16;
    typedef unsigned int       uint32;
    typedef unsigned long long uint64;

    //Handle
    typedef unsigned long long SuHandle;

    //Time
    typedef double           SuTime;

    #if defined (_T)
        #error "_T already defined. I'm confused."
    #else
        #define _T(x) (x)
    #endif

#endif


#endif // _TYPES_H
