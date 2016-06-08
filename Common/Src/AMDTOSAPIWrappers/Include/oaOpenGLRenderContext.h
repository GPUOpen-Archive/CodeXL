//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaOpenGLRenderContext.h
///
//=====================================================================

//------------------------------ oaOpenGLRenderContext.h ------------------------------

#ifndef __OAOPENGLRENDERCONTEXT
#define __OAOPENGLRENDERCONTEXT

// Forward decelerations:
class oaHiddenWindow;
class oaDeviceContext;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIWrappersDLLBuild.h>



// ----------------------------------------------------------------------------------
// Class Name:           oaOpenGLRenderContext
// General Description:
//   Describes an OpenGL render context.
//   This context holds the OpenGL state variables (current color, light, model view matrix,
//   current vertex attributes, etc)
//
//   Serve as a wrapper class for:
//   - Win32 - OpenGL Render context (HGLRC).
//   - UNIX - glX Render context (GLXContext).
//
// Author:      AMD Developer Tools Team
// Creation Date:        6/12/2003
// ----------------------------------------------------------------------------------
class OA_API oaOpenGLRenderContext
{
public:
    oaOpenGLRenderContext(const oaDeviceContext& deviceContext);
    virtual ~oaOpenGLRenderContext();
    // This function is public to allow the main thread to create the window before any other thread attempts to access
    // the render context. This is because on Windows only the thread that created a window is allowed to destroy it.
    static bool InitHiddenWindow();

    bool isValid() const;
    bool makeCurrent();
    bool doneCurrent();
    bool isExtensionSupported(const gtString& extensionName);

    const oaDeviceContext& getContainingDeviceContext() const { return _myDeviceContext; };
    oaOpenGLRenderContextHandle nativeRenderContextHandle() const { return _hOpenGLRC; };
    bool extensionsString(gtString& extensions);
    bool platformSpecificExtensionsString(gtString& extensions);

    static oaOpenGLRenderContext* getDefaultRenderContext();
    static bool getOpenGLString(/*GLenum*/ unsigned int name, gtString& stringValue);
    static void* getOpenGLProcAddress(const char* pFuncName);

    bool getGPUInformation(unsigned long gpuIndex, gtString& gpuName, intptr_t& hGPU);

    // Vendor type:
    bool extractVendorType();
    oaVendorType vendorType() {return _vendorType;};
    gtString vendorString() {return _vendorString;};

private:
    // Disallow use of default constructor, copy constructor and assignment operator:
    oaOpenGLRenderContext() = delete;
    oaOpenGLRenderContext(const oaOpenGLRenderContext&) = delete;
    oaOpenGLRenderContext& operator=(const oaOpenGLRenderContext&) = delete;

private:
    // The device context in which this render context exists:
    const oaDeviceContext& _myDeviceContext;

    // Handle to the native OpenGL render context:
    oaOpenGLRenderContextHandle _hOpenGLRC;

    // The "default" render context:
    static oaOpenGLRenderContext* _pDefaultRenderContext;

    // The "default" default render context window:
    static oaHiddenWindow* _pDefaultRenderContextWindow;

    // oaSingeltonsDelete will delete the above static members:
    friend class oaSingeltonsDelete;

    // Will contain the results of glGetString(GL_EXTENSIONS), if we need them.
    gtString _extensionsString;

    // Will contain the results of wglGetExtensionsStringARB / glXQueryExtensionsString, if we need them.
    gtString _platformSpecificExtensionsString;

    // Contain the renderer type:
    oaVendorType _vendorType;
    gtString _vendorString;
};


#endif  // __OAOPENGLRENDERCONTEXT
