//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLRenderContextGraphicsInfo.h
///
//==================================================================================

//------------------------------ apGLRenderContextGraphicsInfo.h ------------------------------

#ifndef __APGLRENDERCONTEXTGRAPHICSINFO_H
#define __APGLRENDERCONTEXTGRAPHICSINFO_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>

class AP_API apGLRenderContextGraphicsInfo : public osTransferableObject
{
public:
    apGLRenderContextGraphicsInfo();
    ~apGLRenderContextGraphicsInfo();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const { return OS_TOBJ_ID_GL_RENDER_CONTEXT_GRAPHICS_INFO; };
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    enum hardwareAcceleration
    {
        AP_FULL_HARDWARE_ACCELERATED_CONTEXT,
        AP_PARTIAL_HARDWARE_ACCELERATED_CONTEXT,
        AP_NOT_HARDWARE_ACCELERATED_CONTEXT,
        AP_UNKNOWN_HARDWARE_ACCELERATED_CONTEXT
    };

    // Accessors:
    void setGeneralGraphicsInfo(int pixelFormatIndex, bool isDoubleBuffered, hardwareAcceleration acceleration, bool stereo, bool supportsNative);
    void getGeneralGraphicsInfo(int& pixelFormatIndex, bool& isDoubleBuffered, hardwareAcceleration& acceleration, bool& stereo, bool& supportsNative) const;
    const hardwareAcceleration& hardwareAccelerationLevel() const;
    void setChannels(unsigned int red, unsigned int green, unsigned int blue, unsigned int alpha,
                     unsigned int index, unsigned int depth, unsigned int stencil, unsigned int accum);
    void getChannels(unsigned int& red, unsigned int& green, unsigned int& blue, unsigned int& alpha,
                     unsigned int& index, unsigned int& depth, unsigned int& stencil, unsigned int& accum) const;
    void setOpenGLVersion(int major, int minor);
    void getOpenGLVersion(int& major, int& minor) const;
    void setShadingLanguageVersionString(const gtString& version);
    const gtString& getShadingLanguageVersionString() const;
    void addSharingContext(int contextId);
    const gtVector<int>& getSharingContexts() const;
    void addGPUAffinity(const intptr_t& hGPU);
    const gtVector<intptr_t>& getGPUAffinities() const;
    void setRendererInformation(const gtString& rendererVendor, const gtString& rendererName, const gtString& rendererVersion);
    void getRendererInformation(gtString& rendererVendor, gtString& rendererName, gtString& rendererVersion) const;

    // Context compatibility:
    bool isComaptibilityContext() const {return _isCompatibiltyContext;};
    bool isForwardCompatible() const {return _isForwardCompatibleContext;};

    void setComaptibilityContext(bool isCompatibiltyContext) {_isCompatibiltyContext = isCompatibiltyContext;};
    void setForwardCompatible(bool isForwardCompatible) {_isForwardCompatibleContext = isForwardCompatible;};

    // Debug context:
    bool isDebugContext() const {return _isDebugContext;};
    bool isDebugContextFlagForced() const {return _isDebugContextFlagForced;};

    void setDebugFlagDetails(bool isDebugContext, bool isDebugContextFlagForced) {_isDebugContext = isDebugContext; _isDebugContextFlagForced = isDebugContextFlagForced;};

    // Debug context bit force utility:
    static void forceDebugContext(apExecutionMode currentExecutionMode, const int* pOriginalAttributes, const int*& pForcedAttributes, bool& isDebugFlagForced);
    static void releaseAttribListCreatedForDebugContextForcing(const int*& pForcedAttributes);

private:
    // We do not use oaPixelFormatId here as it is ambiguous in Linux and temporary in Mac:
    int _pixelFormatIndex;

    // General graphics information:
    bool _isDoubleBuffered;
    hardwareAcceleration _hardwareAccleration;
    bool _isStereo;
    bool _supportsNativeRendering;

    // Which contexts use this context for sharing:
    gtVector<int> _sharingContexts;

    // Channel sizes:
    unsigned int _redBits;
    unsigned int _greenBits;
    unsigned int _blueBits;
    unsigned int _alphaBits;
    unsigned int _indexBits;
    unsigned int _depthBits;
    unsigned int _stencilBits;
    unsigned int _accumulationBits;

    // OpenGL version:
    int _openGLMajorVersion;
    int _openGLMinorVersion;

    // GLSL version:
    gtString _shadingLanguageVersionString;

    // Context compatibility:
    bool _isCompatibiltyContext;
    bool _isForwardCompatibleContext;

    // Debug context:
    bool _isDebugContext;
    bool _isDebugContextFlagForced;

    // GPU affinities (a vector of handles to GPUs this context has an affinity for, or a single 0 if
    // it is not an affinity GPU)
    gtVector<intptr_t> _gpuAffinities;

    // OpenGL renderer information:
    gtString m_rendererVendor;
    gtString m_rendererName;
    gtString m_rendererVersion;
};

#endif //__APGLRENDERCONTEXTGRAPHICSINFO_H

