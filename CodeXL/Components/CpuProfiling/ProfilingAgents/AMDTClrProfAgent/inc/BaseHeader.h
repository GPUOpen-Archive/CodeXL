//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file BaseHeader.h
///
//==================================================================================

#ifndef _BASEHEADER_H_
#define _BASEHEADER_H_

/***************************************************************************************
 ********************                                               ********************
 ********************             common includes                   ********************
 ********************                                               ********************
 ***************************************************************************************/
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>
#include <windows.h>
#include <winreg.h>
#include <wincrypt.h>
#include <winbase.h>
#include <objbase.h>
#pragma warning( push )
#pragma warning( disable : 4091)
#pragma warning( disable : 4458)
#include <cor.h>
#include <corhdr.h>
#include <corhlpr.h>
#include <corerror.h>
#include <corsym.h>
#include <corpub.h>
#include <corprof.h>
#include <cordebug.h>
#pragma warning( pop )

/***************************************************************************************
 ********************                                               ********************
 ********************            compiler warnings                  ********************
 ********************                                               ********************
 ***************************************************************************************/
// the compiler complains about the exception not being
// used in the exception handler---it is being rethrown.
// ---turn off warning!
#pragma warning ( disable: 4101 )

// the compiler complains about not having an implementation for
// a base class where the derived class exports everything and
// the base class is a template---turn off warning!
#pragma warning ( disable: 4275 )

// the compiler complains about "a unary minus operator applied
// to unsigned type ...", when importing mscorlib.tlb for the
// debugger service test---turn off warning!
#pragma warning ( disable: 4146 )

#pragma warning ( disable: 4996 )


/***************************************************************************************
 ********************                                               ********************
 ********************              basic macros                     ********************
 ********************                                               ********************
 ***************************************************************************************/

//
// max length for arrays
//
#define MAX_LENGTH 256


//
// useful environment/registry macros
//
#define EE_REGISTRY_ROOT         "Software\\Microsoft\\.NETFramework"

//
// char to wchar conversion HEAP
//
#define MAKE_WIDE_PTRHEAP_FROMUTF8( widestr, utf8str ) \
    widestr = new WCHAR[strlen( utf8str ) + 1]; \
    swprintf( widestr, L"%S", utf8str ); \


    //
    // char to wchar conversion ALLOCA
    //
#define MAKE_WIDE_PTRSTACK_FROMUTF8( widestr, utf8str ) \
    widestr = (WCHAR *)_alloca( (strlen( utf8str ) + 1) * sizeof ( WCHAR ) ); \
    swprintf( widestr, L"%S", utf8str ); \


    //
    // wchar to char conversion HEAP
    //
#define MAKE_UTF8_PTRHEAP_FROMWIDE( utf8str, widestr ) \
    utf8str = new char[wcslen( widestr ) + 1]; \
    sprintf( utf8str, "%S", widestr ); \


    //
    // wchar to char conversion ALLOCA
    //
#define MAKE_UTF8_PTRSTACK_FROMWIDE( utf8str, widestr ) \
    utf8str = (char *)_alloca( (wcslen( widestr ) + 1) * sizeof ( char ) ); \
    sprintf( utf8str, "%S", widestr ); \

#endif // _BASEHEADER_H_
