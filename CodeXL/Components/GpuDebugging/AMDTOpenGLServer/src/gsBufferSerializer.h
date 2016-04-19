//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsBufferSerializer.h
///
//==================================================================================

//------------------------------ gsBufferSerializer.h ------------------------------

#ifndef __GSBUFFERSERIALIZER
#define __GSBUFFERSERIALIZER

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTAPIClasses/Include/apTextureType.h>
#include <AMDTAPIClasses/Include/apFileType.h>

// Local:
#include <src/gsBufferReader.h>

// ----------------------------------------------------------------------------------
// Class Name:           gsBufferSerializer
// General Description:  Stores a buffer into a file.
// Author:               Eran Zinman
// Creation Date:        7/8/2007
// ----------------------------------------------------------------------------------
class gsBufferSerializer
{
public:
    // Constructor:
    gsBufferSerializer();

    // Destructor:
    virtual ~gsBufferSerializer();

    // Save buffer data to a file:
    bool saveBufferToFile(apStaticBuffer& staticBuffer, const osFilePath& bufferFilePath, bool isDoubleBuffer);

    // Save render buffer data to a file:
    bool saveBufferToFile(apGLRenderBuffer& renderBuffer, const osFilePath& bufferFilePath);

    // Save VBO data to a file:
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    bool saveBufferToFile(apGLVBO& vboDetails, GLenum target, GLintptr offset, const osFilePath& bufferFilePath, PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData);
#else
    bool saveBufferToFile(apGLVBO& vboDetails, GLenum target, GLintptr offset, const osFilePath& bufferFilePath);
#endif

};


#endif  // __GSBUFFERSERIALIZER
