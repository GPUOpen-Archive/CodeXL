/*****************************************************************************
 *
 *  Trade secret of ATI Technologies, Inc.
 *  Copyright 2003, ATI Technologies, Inc., (unpublished)
 *
 *  All rights reserved.  This notice is intended as a precaution against
 *  inadvertent publication and does not imply publication or any waiver
 *  of confidentiality.  The year included in the foregoing notice is the
 *  year of creation of the work.
 *
 ****************************************************************************
 */

#ifndef XLTINTERFACE_H
#define XLTINTERFACE_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/*                                                                           */
/*   Xlt Types                                                               */
/*                                                                           */
/*****************************************************************************/
#define XLT_TRUE 1
#define XLT_FALSE 0

typedef int XLT_BOOL;
typedef void* XLT_PVOID;

#if defined(__CONFIGURATION3__)
	#define XLT_STDCALL   // not needed on mac os x
	#define _vsnprintf vsnprintf
#else
  // win32, win64, other platforms
	#define XLT_FASTCALL __fastcall
	#define XLT_STDCALL __stdcall
    #define XLT_CDECL __cdecl
#endif // #if defined(__CONFIGURATION3__)

#ifndef __cplusplus
typedef int XLT_BOOL;
#endif


/*****************************************************************************/
/*                                                                           */
/*   Xlt Callback Declarations                                               */
/*                                                                           */
/*****************************************************************************/

typedef void* (XLT_STDCALL *XLTCLIENT_ALLOCSYSMEM ) 
    (
        void* pHandle,
        unsigned int dwSizeInBytes
    );

typedef void (XLT_STDCALL *XLTCLIENT_FREESYSMEM)
    (
        void* pHandle,
        void* lpAddress
    );

typedef int ( XLT_CDECL *XLTCLIENT_OUTPUTSTRING)
    (
        void* pHandle,
        const char* pszBuffer,
        ...
    );

typedef int (XLT_STDCALL  *XLTCLIENT_OUTPUTBINARY)
    (
        void* pHandle,
        const void* pTranslatedProgram,
        unsigned int nTranslatedProgramSizeInBytes
    );

typedef void (XLT_STDCALL  *XLTCLIENT_ASSERT)
    (
        void* pHandle
    );

typedef int (XLT_STDCALL *XLTCLIENT_FLOATTOSTRING)
    (
    	void* pHandle,
        char *pszBuffer,
        float Value
    );

/*****************************************************************************/
/*                                                                           */
/*   Xlt Enumerations                                                        */
/*                                                                           */
/*****************************************************************************/

typedef enum _E_XLT_MODE
{
    E_XLT_NORMAL = 0,
    E_XLT_CHUNK  = 1,
} E_XLT_MODE;


/*****************************************************************************/
/*                                                                           */
/*   Xlt Structures                                                          */
/*                                                                           */
/*****************************************************************************/

typedef struct _XLT_PROGINFO
{
    char* pBuffer;
    int   nBufferSize;
} XLT_PROGINFO, *LPXLT_PROGINFO;


typedef struct _XLT_CALLBACKS 
{
    E_XLT_MODE eXltMode;
    XLT_PVOID                   pHandle;         // callback handle
    XLTCLIENT_ALLOCSYSMEM       AllocateSysMem;  // sys mem alloc callback
    XLTCLIENT_FREESYSMEM        FreeSysMem;      // sys mem free callback
    XLTCLIENT_OUTPUTSTRING      OutputString;    // output translated IL Text
    XLTCLIENT_OUTPUTBINARY      OutputBinary;    // output translated IL binary
    XLTCLIENT_OUTPUTBINARY      OutputBinaryFull;// output full DX10 shader object
    XLTCLIENT_ASSERT            Assert;          // assert callback
    XLTCLIENT_FLOATTOSTRING     FloatToString;   // FLoat to string conversion.
    int flags;
    // flag == 0 means shader follows the  ms dissembler format
    // flag == 1 means shader follows the hlsl format
    // flag == 2 means shader follows the testasm format
    // 
} XLT_CALLBACKS, *LPXLT_CALLBACKS;

// details of how the setting of flag effects outputs
// (1) dcl_immedconstantbuffer  { { 1, 2, 3, 4}, { 5, 6, 7, 8} }
//   hlls ms dis generates 1,2,3,4, 5,6,7,8
//   testasm generates     5,6,7,8, 1,2,3,4 (list reversed)


/*****************************************************************************/
/*                                                                           */
/*   Translator Interface Routines                                           */
/*                                                                           */
/*****************************************************************************/
XLT_BOOL XLT_FASTCALL DX10AsmText2Stream( LPXLT_PROGINFO lpInput,
                                          LPXLT_CALLBACKS xltCallbacks, char*& pTheBinBuffer,  unsigned int& nBufferSize );

XLT_BOOL XLT_FASTCALL DX10AsmStream2Text( LPXLT_PROGINFO lpInput,
                                          LPXLT_CALLBACKS xltCallbacks );

#ifdef __cplusplus
}
#endif

#endif
