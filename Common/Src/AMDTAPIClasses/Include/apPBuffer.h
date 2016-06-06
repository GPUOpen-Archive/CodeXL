//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apPBuffer.h
///
//==================================================================================

//------------------------------ apPBuffer.h ------------------------------

#ifndef __APPBUFFER
#define __APPBUFFER

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apAllocatedObject.h>

// ----------------------------------------------------------------------------------
// Class Name:          apPBuffer : public osTransferableObject
// General Description:
//   Represents an OpenGL PBuffer
//
// Author:  AMD Developer Tools Team
// Creation Date:       25/08/2007
// ----------------------------------------------------------------------------------
class AP_API apPBuffer : public apAllocatedObject
{
public:
    // Constructor:
    apPBuffer();
    apPBuffer(const oaPBufferHandle& pbufferHandler);

    // Destructor:
    ~apPBuffer();

    // Self functions:
    apPBuffer(const apPBuffer& other);
    apPBuffer& operator=(const apPBuffer& other);

public:
    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Was PBuffer deleted?
    bool isDeleted() { return _isDeleted; };

    // CGL PBuffers additional information:
    enum pbufferBindTarget
    {
        AP_UNDEFINED_PBUFFER,
        AP_2D_PBUFFER,
        AP_CUBE_MAP_PBUFFER,
        AP_RECTANGLE_PBUFFER
    };

    enum pbufferInternalFormat
    {
        AP_UNDEFINED_FORMAT_PBUFFER,
        AP_RGB_PBUFFER,
        AP_RGBA_PBUFFER
    };

public:
    // Set the PBuffer dimensions
    bool setDimensions(GLint pbufferWidth, GLint pbufferHeight);

    // Set the PBuffer device context
    void setPBufferHDC(oaDeviceContextHandle pbufferhDC) { _pbufferhDC = pbufferhDC; };

    // Set the PBuffer render context (Spy ID):
    void setPBufferRenderContextSpyId(int renderContextSpyId) { _pbufferRenderContextSpyId = renderContextSpyId; };

    // Release the PBuffer device context
    void releasePBufferHDC(oaDeviceContextHandle pbufferhDC);

    // Set the CGL PBuffer properties:
    void setBindTarget(pbufferBindTarget target) {_bindTarget = target;};
    void setInternalFormat(pbufferInternalFormat format) {_internalFormat = format;};
    void setCubeMapFace(GLenum face) {_cubemapFace = face;};
    void setMaxMipmapLevel(GLint level) {_maxMipmapLevel = level;};
    void setMipmapLevel(GLint level) {_mipmapLevel = level;};

public:
    // Return the PBuffer openGL handler:
    oaPBufferHandle pbufferHandler() const { return _pbufferHandler; };

    // Return the PBuffer hDC:
    oaDeviceContextHandle deviceContextOSHandle() const { return _pbufferhDC; };

    // Return the PBuffer context Id:
    int pbufferContextId() const { return _pbufferRenderContextSpyId; };

    // Get the buffer's dimesions:
    GLint width() const {return _width;};
    GLint height() const {return _height;};

    // Get the buffer's CGL properties:
    pbufferBindTarget bindTarget() {return _bindTarget;};
    pbufferInternalFormat internalFormat() {return _internalFormat;};
    GLenum cubeMapFace() {return _cubemapFace;};
    GLint maxMipmapLevel() {return _maxMipmapLevel;};
    GLint mipmapLevel() {return _mipmapLevel;};

protected:
    // The PBuffer render context spy internal id
    int _pbufferRenderContextSpyId;

    // The PBuffer width and height:
    GLint _width, _height;

    // The PBuffer handler:
    oaPBufferHandle _pbufferHandler;

    // The PBuffer device context:
    oaDeviceContextHandle _pbufferhDC;

    // Flags if the PBuffer was deleted or not
    bool _isDeleted;

    // Used for CGL PBuffers:
    pbufferBindTarget _bindTarget;
    pbufferInternalFormat _internalFormat;
    GLenum _cubemapFace;
    GLint _maxMipmapLevel;
    GLint _mipmapLevel;
};


#endif  // __apPBuffer
