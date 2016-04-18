//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsBufferReader.h
///
//==================================================================================

//------------------------------ gsBufferReader.h ------------------------------

#ifndef __GSBUFFERREADER_H
#define __GSBUFFERREADER_H

// Forward decelerations:
class osFilePath;

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaDataType.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>
#include <AMDTAPIClasses/Include/apFileType.h>
#include <AMDTAPIClasses/Include/apDisplayBuffer.h>

// Spy Utilities:
#include <AMDTServerUtilities/Include/suBufferReader.h>

// Local:
#include <src/gsImageWriter.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsBufferReader : public gsImageWriter
// General Description:
//   Reads OpenGL buffers data (back and front buffers, auxiliary buffers, PBuffers,
//   FBOs, etc).
//
// Author:               Yaki Tebeka
// Creation Date:        5/7/2006
// ----------------------------------------------------------------------------------
class gsBufferReader : public suBufferReader, gsImageWriter
{
public:
    // Constructor:
    gsBufferReader();

    // Destructor:
    virtual ~gsBufferReader();

public:
    // Reads the VBO content into an internal raw data buffer:
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    bool readVBOContent(GLenum target, GLintptr offset, GLsizeiptr bufferSize, PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData);
#else
    bool readVBOContent(GLenum target, GLintptr offset, GLsizeiptr bufferSize);
#endif

    // Reads the buffer content into an internal raw data buffer:
    bool readBufferContent(apDisplayBuffer readBuffer, int bufferWidth, int bufferHeight, oaTexelDataFormat bufferDataFormat, oaDataType componentDataType, bool isDoubleBuffer);

protected:
    // Reads buffer content to memory
    virtual bool readBufferContentFromAPI();

    // Sets the buffer to be read as active
    bool setActiveReadBuffer(bool& wasModeChanged);

    // Check if interactive break mode is on
    bool isInteractiveBreakOn() const;

    // Calculate the amount of memory required for the buffer size
    unsigned long calculateBufferMemorySize();

    // To read static buffers, we need to unbind framebuffer objects (other the GL_DEPTH gives the FBO's
    // depth attachment, and the other static buffers cannot be read.
    void unbindFBO();
    void restoreFBO();

private:
    // The buffer to be read:
    apDisplayBuffer _readBuffer;

    // Is the application double buffered:
    bool _isDoubleBuffer;
};


#endif //__GSBUFFERREADER_H
