//    
//  Workfile: XltCallbacks.h
//
//  Description:
//      ILTextParserEnv class definition.
//
//  Trade secret of ATI Technologies, Inc.
//  Copyright 2002, ATI Technologies, Inc., (unpublished)
//
//  All rights reserved.  This notice is intended as a precaution against
//  inadvertent publication and does not imply publication or any waiver
//  of confidentiality.  The year included in the foregoing notice is the
//  year of creation of the work.
//
//

#ifndef XLTCALLBACKS_H
#define XLTCALLBACKS_H

#include <stdarg.h>

// callback accessor functions
void* xlt_malloc( unsigned int nSizeInBytes );
void  xlt_free( void* pBuffer );
int   xlt_printf( const char* pszBuffer, ... );
int   xlt_error( const char* pszBuffer, ... );
int   xlt_debug( const char* pszBuffer, ... );
void  xlt_outputBuffer( const void* pBuffer, unsigned int nBufferSize );
void  xlt_assert(void);
int   xlt_FloatToString(char *pszBuffer, float Value);
bool  xlt_isLineMode(void);

#endif // XLTCALLBACKS_H