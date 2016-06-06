//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLRenderContextGraphicsInfo.cpp
///
//==================================================================================

//------------------------------ apGLRenderContextGraphicsInfo.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/apGLenumParameter.h>
#include <AMDTAPIClasses/Include/apGLRenderContextGraphicsInfo.h>

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    #define AP_CONTEXT_FLAGS_ARB WGL_CONTEXT_FLAGS_ARB
    #define AP_CONTEXT_DEBUG_BIT_ARB WGL_CONTEXT_DEBUG_BIT_ARB
#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    #if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
        #define AP_CONTEXT_FLAGS_ARB GLX_CONTEXT_FLAGS_ARB
        #define AP_CONTEXT_DEBUG_BIT_ARB GLX_CONTEXT_DEBUG_BIT_ARB
    #elif AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
    #endif
#else
    #error Unknown build configuration!
#endif

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::apGLRenderContextGraphicsInfo
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        19/3/2009
// ---------------------------------------------------------------------------
apGLRenderContextGraphicsInfo::apGLRenderContextGraphicsInfo():
    _pixelFormatIndex(-1), _isDoubleBuffered(false), _hardwareAccleration(AP_UNKNOWN_HARDWARE_ACCELERATED_CONTEXT), _isStereo(false), _supportsNativeRendering(false),
    _redBits(0), _greenBits(0), _blueBits(0), _alphaBits(0), _indexBits(0), _depthBits(0), _stencilBits(0), _accumulationBits(0),
    _openGLMajorVersion(-1), _openGLMinorVersion(-1), _isCompatibiltyContext(false), _isForwardCompatibleContext(false), _isDebugContext(false), _isDebugContextFlagForced(false)
{

}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::~apGLRenderContextGraphicsInfo
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        19/3/2009
// ---------------------------------------------------------------------------
apGLRenderContextGraphicsInfo::~apGLRenderContextGraphicsInfo()
{

}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::writeSelfIntoChannel
// Description: Writes this class data into a channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        19/3/2009
// ---------------------------------------------------------------------------
bool apGLRenderContextGraphicsInfo::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    ipcChannel << (gtInt32)_pixelFormatIndex;
    ipcChannel << _isDoubleBuffered;
    ipcChannel << (gtInt32)_hardwareAccleration;
    ipcChannel << _isStereo;
    ipcChannel << _supportsNativeRendering;

    gtInt64 numOfSharingContexts = (gtInt64)_sharingContexts.size();
    ipcChannel << numOfSharingContexts;

    for (int i = 0; i < numOfSharingContexts; i++)
    {
        ipcChannel << (gtInt32)_sharingContexts[i];
    }

    ipcChannel << (gtUInt32)_redBits;
    ipcChannel << (gtUInt32)_greenBits;
    ipcChannel << (gtUInt32)_blueBits;
    ipcChannel << (gtUInt32)_alphaBits;
    ipcChannel << (gtUInt32)_indexBits;
    ipcChannel << (gtUInt32)_depthBits;
    ipcChannel << (gtUInt32)_stencilBits;
    ipcChannel << (gtUInt32)_accumulationBits;

    ipcChannel << (gtInt32)_openGLMajorVersion;
    ipcChannel << (gtInt32)_openGLMinorVersion;
    ipcChannel << _shadingLanguageVersionString;
    ipcChannel << _isCompatibiltyContext;
    ipcChannel << _isForwardCompatibleContext;

    ipcChannel << _isDebugContext;
    ipcChannel << _isDebugContextFlagForced;

    gtInt64 numOfGPUs = (gtInt64)_gpuAffinities.size();
    ipcChannel << numOfGPUs;

    for (int i = 0; i < numOfGPUs; i++)
    {
        ipcChannel << (gtUInt64)_gpuAffinities[i];
    }

    ipcChannel << m_rendererVendor;
    ipcChannel << m_rendererName;
    ipcChannel << m_rendererVersion;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::readSelfFromChannel
// Description: Reads this class data from a channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        19/3/2009
// ---------------------------------------------------------------------------
bool apGLRenderContextGraphicsInfo::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    gtInt32 pixelFormatIndexAsInt32 = 0;
    ipcChannel >> pixelFormatIndexAsInt32;
    _pixelFormatIndex = (int)pixelFormatIndexAsInt32;

    ipcChannel >> _isDoubleBuffered;

    gtInt32 accelerationAsInt32 = 0;
    ipcChannel >> accelerationAsInt32;
    _hardwareAccleration = (hardwareAcceleration)accelerationAsInt32;

    ipcChannel >> _isStereo;
    ipcChannel >> _supportsNativeRendering;

    gtInt64 numOfSharingContexts = 0;
    ipcChannel >> numOfSharingContexts;

    for (int i = 0; i < numOfSharingContexts; i++)
    {
        gtInt32 currentContext = -1;
        ipcChannel >> currentContext;
        _sharingContexts.push_back((int)currentContext);
    }

    gtUInt32 redBitsAsUInt32 = 0;
    ipcChannel >> redBitsAsUInt32;
    _redBits = (unsigned int)redBitsAsUInt32;

    gtUInt32 greenBitsAsUInt32 = 0;
    ipcChannel >> greenBitsAsUInt32;
    _greenBits = (unsigned int)greenBitsAsUInt32;

    gtUInt32 blueBitsAsUInt32 = 0;
    ipcChannel >> blueBitsAsUInt32;
    _blueBits = (unsigned int)blueBitsAsUInt32;

    gtUInt32 alphaBitsAsUInt32 = 0;
    ipcChannel >> alphaBitsAsUInt32;
    _alphaBits = (unsigned int)alphaBitsAsUInt32;

    gtUInt32 indexBitsAsUInt32 = 0;
    ipcChannel >> indexBitsAsUInt32;
    _indexBits = (unsigned int)indexBitsAsUInt32;

    gtUInt32 depthBitsAsUInt32 = 0;
    ipcChannel >> depthBitsAsUInt32;
    _depthBits = (unsigned int)depthBitsAsUInt32;

    gtUInt32 stencilBitsAsUInt32 = 0;
    ipcChannel >> stencilBitsAsUInt32;
    _stencilBits = (unsigned int)stencilBitsAsUInt32;

    gtUInt32 accumulationBitsAsUInt32 = 0;
    ipcChannel >> accumulationBitsAsUInt32;
    _accumulationBits = (unsigned int)accumulationBitsAsUInt32;

    gtInt32 openGLMajorVersionAsInt32 = 0;
    ipcChannel >> openGLMajorVersionAsInt32;
    _openGLMajorVersion = (int)openGLMajorVersionAsInt32;

    gtInt32 openGLMinorVersionAsInt32 = 0;
    ipcChannel >> openGLMinorVersionAsInt32;
    _openGLMinorVersion = (int)openGLMinorVersionAsInt32;

    ipcChannel >> _shadingLanguageVersionString;

    ipcChannel >> _isCompatibiltyContext;
    ipcChannel >> _isForwardCompatibleContext;

    ipcChannel >> _isDebugContext;
    ipcChannel >> _isDebugContextFlagForced;

    gtInt64 numOfGPUs = 0;
    ipcChannel >> numOfGPUs;

    for (int i = 0; i < numOfGPUs; i++)
    {
        gtUInt64 currentGPU = 0;
        ipcChannel >> currentGPU;
        _gpuAffinities.push_back((intptr_t)currentGPU);
    }

    ipcChannel >> m_rendererVendor;
    ipcChannel >> m_rendererName;
    ipcChannel >> m_rendererVersion;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::setGeneralGraphicsInfo
// Description: Sets the general information
// Author:  AMD Developer Tools Team
// Date:        19/3/2009
// ---------------------------------------------------------------------------
void apGLRenderContextGraphicsInfo::setGeneralGraphicsInfo(int pixelFormatIndex, bool isDoubleBuffered, hardwareAcceleration acceleration, bool stereo, bool supportsNative)
{
    _pixelFormatIndex = pixelFormatIndex;
    _isDoubleBuffered = isDoubleBuffered;
    _hardwareAccleration = acceleration;
    _isStereo = stereo;
    _supportsNativeRendering = supportsNative;
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::getGeneralGraphicsInfo
// Description: Returns the general information
// Author:  AMD Developer Tools Team
// Date:        19/3/2009
// ---------------------------------------------------------------------------
void apGLRenderContextGraphicsInfo::getGeneralGraphicsInfo(int& pixelFormatIndex, bool& isDoubleBuffered, hardwareAcceleration& acceleration, bool& stereo, bool& supportsNative) const
{
    pixelFormatIndex = _pixelFormatIndex;
    isDoubleBuffered = _isDoubleBuffered;
    acceleration = _hardwareAccleration;
    stereo = _isStereo;
    supportsNative = _supportsNativeRendering;
}
// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::hardwareAccelerationLevel
// Description: returns the hardware acceleration level
// Author:  AMD Developer Tools Team
// Date:        10/2/2014
// ---------------------------------------------------------------------------
const apGLRenderContextGraphicsInfo::hardwareAcceleration& apGLRenderContextGraphicsInfo::hardwareAccelerationLevel() const
{
    return _hardwareAccleration;
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::setChannels
// Description: Sets the size in bits of all channels
// Author:  AMD Developer Tools Team
// Date:        19/3/2009
// ---------------------------------------------------------------------------
void apGLRenderContextGraphicsInfo::setChannels(unsigned int red, unsigned int green, unsigned int blue, unsigned int alpha, unsigned int index, unsigned int depth, unsigned int stencil, unsigned int accum)
{
    _redBits = red;
    _greenBits = green;
    _blueBits = blue;
    _alphaBits = alpha;
    _indexBits = index;
    _depthBits = depth;
    _stencilBits = stencil;
    _accumulationBits = accum;
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::getChannels
// Description: Returns the size in bits of all channels
// Author:  AMD Developer Tools Team
// Date:        19/3/2009
// ---------------------------------------------------------------------------
void apGLRenderContextGraphicsInfo::getChannels(unsigned int& red, unsigned int& green, unsigned int& blue, unsigned int& alpha, unsigned int& index, unsigned int& depth, unsigned int& stencil, unsigned int& accum) const
{
    red = _redBits;
    green = _greenBits;
    blue = _blueBits;
    alpha = _alphaBits;
    index = _indexBits;
    depth = _depthBits;
    stencil = _stencilBits;
    accum = _accumulationBits;
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::setOpenGLVersion
// Description: Sets the context's OpenGL version
// Author:  AMD Developer Tools Team
// Date:        22/3/2009
// ---------------------------------------------------------------------------
void apGLRenderContextGraphicsInfo::setOpenGLVersion(int major, int minor)
{
    _openGLMajorVersion = major;
    _openGLMinorVersion = minor;
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::getOpenGLVersion
// Description: Returns the context's OpenGL version
// Author:  AMD Developer Tools Team
// Date:        22/3/2009
// ---------------------------------------------------------------------------
void apGLRenderContextGraphicsInfo::getOpenGLVersion(int& major, int& minor) const
{
    major = _openGLMajorVersion;
    minor = _openGLMinorVersion;
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::setShadingLanguageVersionString
// Description: Sets the GLSL version string
// Author:  AMD Developer Tools Team
// Date:        22/3/2009
// ---------------------------------------------------------------------------
void apGLRenderContextGraphicsInfo::setShadingLanguageVersionString(const gtString& version)
{
    _shadingLanguageVersionString = version;
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::getShadingLanguageVersionString
// Description: Returns the GLSL version string
// Author:  AMD Developer Tools Team
// Date:        22/3/2009
// ---------------------------------------------------------------------------
const gtString& apGLRenderContextGraphicsInfo::getShadingLanguageVersionString() const
{
    return _shadingLanguageVersionString;
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::addSharingContext
// Description: Adds a context as sharing this context
// Author:  AMD Developer Tools Team
// Date:        19/3/2009
// ---------------------------------------------------------------------------
void apGLRenderContextGraphicsInfo::addSharingContext(int contextId)
{
    _sharingContexts.push_back(contextId);
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::getSharingContexts
// Description: Returns the sharing contexts vector.
// Author:  AMD Developer Tools Team
// Date:        19/3/2009
// ---------------------------------------------------------------------------
const gtVector<int>& apGLRenderContextGraphicsInfo::getSharingContexts() const
{
    return _sharingContexts;
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::addGPUAffinity
// Description: Adds hGPU as a GPU this context has an affinity for:
// Author:  AMD Developer Tools Team
// Date:        23/3/2009
// ---------------------------------------------------------------------------
void apGLRenderContextGraphicsInfo::addGPUAffinity(const intptr_t& hGPU)
{
    _gpuAffinities.push_back(hGPU);
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::getGPUAffinities
// Description:
// Return Val: const gtVector<intptr_t>&
// Author:  AMD Developer Tools Team
// Date:        23/3/2009
// ---------------------------------------------------------------------------
const gtVector<intptr_t>& apGLRenderContextGraphicsInfo::getGPUAffinities() const
{
    return _gpuAffinities;
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::setRendererInformation
// Description: Set the renderer information
// Author:  AMD Developer Tools Team
// Date:        10/2/2014
// ---------------------------------------------------------------------------
void apGLRenderContextGraphicsInfo::setRendererInformation(const gtString& rendererVendor, const gtString& rendererName, const gtString& rendererVersion)
{
    m_rendererVendor = rendererVendor;
    m_rendererName = rendererName;
    m_rendererVersion = rendererVersion;
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::getRendererInformation
// Description: Get the renderer information
// Author:  AMD Developer Tools Team
// Date:        10/2/2014
// ---------------------------------------------------------------------------
void apGLRenderContextGraphicsInfo::getRendererInformation(gtString& rendererVendor, gtString& rendererName, gtString& rendererVersion) const
{
    rendererVendor = m_rendererVendor;
    rendererName = m_rendererName;
    rendererVersion = m_rendererVersion;
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::forceDebugContext
// Description: Utility: checks the input context creation attributes, and add / remove
//              debug bit according to the execution mode
// Arguments:   executionMode - the current execution mode, will force the debug flag
//              on or off according to this.
//              pOriginalAttributes - the attributes as specified by the user.
//              pForcedAttributes - will be set to point to a new array of ints,
//              containing the modified (or unmodified) attributes. It is the caller's
//              responsibility to release this array - as non-const int array.
//              isDebugFlagForced - will contain true iff the debug flag was changed.
// Author:  AMD Developer Tools Team
// Date:        4/7/2010
// ---------------------------------------------------------------------------
void apGLRenderContextGraphicsInfo::forceDebugContext(apExecutionMode executionMode, const int* pOriginalAttributes, const int*& pForcedAttributes, bool& isDebugFlagForced)
{
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    // Iterate the attributes list, and search for the debug flag:
    int originalAttribsCount = 0;
    bool contextFlagsExist = false;
    bool flagsLeft = (pOriginalAttributes != NULL);
    isDebugFlagForced = false;

    gtString attribsForLog;
    bool collectAttribs = (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity());

    // Iterate the flags, count the flags, and check if there is a context flag already:
    while (flagsLeft)
    {
        // Check the current attribute:
        int currentAttribute = pOriginalAttributes[2 * originalAttribsCount];

        if (currentAttribute != 0)
        {
            if (collectAttribs)
            {
                int currentAttrVal = pOriginalAttributes[2 * originalAttribsCount + 1];
                attribsForLog.appendFormattedString(L"%d: Name: %#10x; Value: %#10x | ", originalAttribsCount, currentAttribute, currentAttrVal);
                apGLenumParameter paramNm((GLenum)currentAttribute);
                gtString enumVal;
                paramNm.valueAsString(enumVal);
                attribsForLog.append(enumVal).append('=');
                apGLenumParameter paramVl((GLenum)currentAttrVal);
                paramVl.valueAsString(enumVal);
                attribsForLog.append(enumVal).append('\n');
            }

            if (currentAttribute == AP_CONTEXT_FLAGS_ARB)
            {
                contextFlagsExist = true;
            }

            // Increment the amount of attributes:
            originalAttribsCount++;
        }
        else
        {
            flagsLeft = false;
        }
    }

    // Check if debug bit should be on:
    bool debugFlagShouldBeOn = false;

    if (executionMode != AP_PROFILING_MODE)
    {
        debugFlagShouldBeOn = true;
    }

    if (collectAttribs)
    {
        if (attribsForLog.isEmpty())
        {
            attribsForLog = L"None";
        }

        attribsForLog.prepend(L"Context creation properties:\n").append(L"AP_CONTEXT_FLAGS value has AP_CONTEXT_DEBUG_BIT ").append(debugFlagShouldBeOn ? L"forced on." : L"unchanged.");
        OS_OUTPUT_DEBUG_LOG(attribsForLog.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }

    // Check how many attributes should be allocated (same as original, but add another one if
    // there are no context flags):
    int amountOfNeededAttribs = originalAttribsCount;

    if (!contextFlagsExist)
    {
        amountOfNeededAttribs++;
    }

    // Allocate a new attributes pointer (each attribute is a name-value pair, plus 1 for the terminating 0):
    pForcedAttributes = (const int*)(new int[(amountOfNeededAttribs * 2) + 1]);
    int* pForcedAttributesMutable = (int*)pForcedAttributes;

    // Copy the original attributes:
    int currentAttrib = 0;

    for (currentAttrib = 0; currentAttrib < originalAttribsCount; currentAttrib ++)
    {
        // Check the current flag:
        int currentAttribute = pOriginalAttributes[2 * currentAttrib];
        int currentFlagValue = pOriginalAttributes[2 * currentAttrib + 1];

        if (currentAttribute != 0)
        {
            // Check for context flags:
            if (currentAttribute == AP_CONTEXT_FLAGS_ARB)
            {
                // Check if the debug flag is currently on:
                bool debugFlagIsOn = ((currentFlagValue & AP_CONTEXT_DEBUG_BIT_ARB) != 0);

                // Check if the debug flag is forced:
                isDebugFlagForced = (debugFlagIsOn != debugFlagShouldBeOn);

                if (debugFlagShouldBeOn)
                {
                    currentFlagValue |= AP_CONTEXT_DEBUG_BIT_ARB;
                }
                else
                {
                    currentFlagValue &= ~AP_CONTEXT_DEBUG_BIT_ARB;
                }
            }

            // Set the attribute value:
            pForcedAttributesMutable[2 * currentAttrib] = currentAttribute;
            pForcedAttributesMutable[2 * currentAttrib + 1] = currentFlagValue;
        }
    }

    // Add the debug flag attribute if not added:
    if (currentAttrib < amountOfNeededAttribs)
    {
        pForcedAttributesMutable[currentAttrib * 2] = AP_CONTEXT_FLAGS_ARB;
        pForcedAttributesMutable[currentAttrib * 2 + 1] = AP_CONTEXT_DEBUG_BIT_ARB;
        currentAttrib++;
    }

    pForcedAttributesMutable[currentAttrib * 2] = 0;
#endif
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextGraphicsInfo::releaseAttribListCreatedForDebugContextForcing
// Description: Release the pForcedAttributes array allocated in apGLRenderContextGraphicsInfo::forceDebugContext
// Author:  AMD Developer Tools Team
// Date:        17/10/2013
// ---------------------------------------------------------------------------
void apGLRenderContextGraphicsInfo::releaseAttribListCreatedForDebugContextForcing(const int*& pForcedAttributes)
{
    delete[] pForcedAttributes;
    pForcedAttributes = NULL;
}

