//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsEAGLWrappers.h
///
//==================================================================================

//------------------------------ gsEAGLWrappers.h ------------------------------

#ifndef __GSEAGLWRAPPERS_H
#define __GSEAGLWRAPPERS_H


bool gsInitializeEAGLWrappers();
void gsGetEAGLFunctionPointers(void*& pEAGLContext_initWithAPI, void*& pEAGLContext_initWithAPI_sharegroup, void*& pEAGLContext_dealloc, void*& pEAGLContext_setCurrentContext, void*& pEAGLContext_currentContext, void*& pEAGLContext_API, void*& pEAGLContext_sharegroup, void*& pEAGLContext_renderbufferStorage_fromDrawable, void*& pEAGLContext_presentRenderbuffer);
void gsGetEAGLWrapperFunctionPointers(void*& pgsEAGLContext_initWithAPI, void*& pgsEAGLContext_initWithAPI_sharegroup, void*& pgsEAGLContext_dealloc, void*& pgsEAGLContext_setCurrentContext, void*& pgsEAGLContext_currentContext, void*& pgsEAGLContext_API, void*& pgsEAGLContext_sharegroup, void*& pgsEAGLContext_renderbufferStorage_fromDrawable, void*& pgsEAGLContext_presentRenderbuffer);
bool gsMakeEAGLContextCurrent(osOpenGLRenderContextHandle hRC);
bool gsSwapEAGLContextBuffers(osOpenGLRenderContextHandle hRC);

#endif //__GSEAGLWRAPPERS_H

