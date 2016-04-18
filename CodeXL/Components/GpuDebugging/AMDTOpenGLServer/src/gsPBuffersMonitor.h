//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsPBuffersMonitor.h
///
//==================================================================================

//------------------------------ gsPBuffersMonitor.h ------------------------------

#ifndef __GSPBUFFERSMONITOR
#define __GSPBUFFERSMONITOR

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <src/gsPBuffer.h>

// ----------------------------------------------------------------------------------
// Class Name:           gsPBuffersMonitor
// General Description:  Monitors PBuffer objects in the debugged application.
// Author:               Eran Zinman
// Creation Date:        24/08/2007
// ----------------------------------------------------------------------------------
class gsPBuffersMonitor
{
public:
    // Constructor:
    gsPBuffersMonitor();

    // Destructor:
    ~gsPBuffersMonitor();

public:
    // On OpenGL events functions:
    void onPBufferCreation(const oaPBufferHandle& pbufferHandler, oaDeviceContextHandle hDC, int iPixelFormat, int iWidth, int iHeight, const int* piAttribList,
                           GLenum target = GL_NONE, GLenum format = GL_NONE, GLint level = -1);
    void onPBufferDeletion(const oaPBufferHandle& pbufferHandler);
    void onPBufferhDCGeneration(const oaPBufferHandle& pbufferHandler, const oaDeviceContextHandle& pbufferhDC);
    void onPBufferhDCRelease(const oaPBufferHandle& pbufferHandler, const oaDeviceContextHandle& pbufferhDC);

    // Different onMakeCurrent implementations for Linux, Mac and Windows
    void onWGLMakeCurrent(oaDeviceContextHandle hDC, oaOpenGLRenderContextHandle hRC);
    void onGLXMakeCurrent(oaDeviceContextHandle hDC, oaPBufferHandle draw, oaPBufferHandle read, oaOpenGLRenderContextHandle hRC);
    void onCGLSetPBuffer(oaOpenGLRenderContextHandle hRC, oaPBufferHandle hPBuffer, GLenum cubeMapFace, GLint mipLevel);
    void onCGLTexImagePBuffer(int contextId, oaPBufferHandle hPBuffer, GLenum source) const;

    // Returns amount of PBuffers in the debugged application:
    int amountOfPBuffersObjects() const { return (int)_pbuffers.size(); };

public:
    // Get PBuffer object associated with the pbufferId
    gsPBuffer* getPBufferObjectDetails(int pbufferId) const;

    // Functions to handle a specific PBuffer:
    int amountOfPBufferContentBuffers(int pbufferId) const;

    // Return a specific buffer from a PBuffer object:
    apStaticBuffer* getPBufferStaticBufferObjectDetails(int pbufferId, apDisplayBuffer bufferType) const;

    // Return the static buffer type, given a pbufferId and a static buffer type.
    bool getPBufferStaticBufferType(int pbufferId, int staticBufferIter, apDisplayBuffer& bufferType) const;

    // Update the PBuffer static buffer raw data
    bool updatePBufferStaticBufferRawData(int pbufferId, apDisplayBuffer bufferType);

    // Update the PBuffer's dimensions according to the current HDC dimensions:
    bool updatePBuffersDimensions();

private:
    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead
    gsPBuffersMonitor& operator=(const gsPBuffersMonitor& otherMonitor);
    gsPBuffersMonitor(const gsPBuffersMonitor& otherMonitor);

    // Called when a PBuffer becomes current
    void pbufferMadeCurrent(gsPBuffer* pPBufferItem, oaOpenGLRenderContextHandle pbufferhRC);

    // Returns the vector index of the gsPBuffer object that is linked with the osPbufferHandle handle
    int getPBufferObject(const oaPBufferHandle& pbufferHandler) const;

    // Add the new PBuffer object into the PBuffer vector
    void addPBufferItem(gsPBuffer* pPBufferItem);

    // Transforms a render context to it's equivalent spyID render context
    int getRenderContextSpyId(const oaOpenGLRenderContextHandle& pbufferhRC);

private:
    // Hold the parameters of PBuffers that reside in this render context:
    gtVector<gsPBuffer*> _pbuffers;
};

#endif  // __GSPBUFFERSMONITOR
