//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaChannelOperators.cpp
///
//=====================================================================

//------------------------------ oaChannelOperators.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osRawMemoryStream.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaChannelOperators.h>
#include <AMDTOSAPIWrappers/Include/oaDeviceContext.h>

// Data types sizes:
static unsigned long statDeviceContextHandleSize = sizeof(oaDeviceContextHandle);
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    #ifndef _GR_IPHONE_BUILD
        static unsigned long statCGLPixelFormatAttributeSize = sizeof(CGLPixelFormatAttribute);
        static unsigned long statCGLContextEnableSize = sizeof(CGLContextEnable);
        static unsigned long statCGLContextParameterSize = sizeof(CGLContextParameter);
        static unsigned long statCGLGlobalOptionSize = sizeof(CGLGlobalOption);
        static unsigned long statCGLRendererPropertySize = sizeof(CGLRendererProperty);
    #endif
#elif (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    static unsigned long statPBufferHandlerSize = sizeof(oaPBufferHandle);
#endif
// static unsigned long statPtrSize = sizeof(void*);

// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes a device context handle value into an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        26/1/2008
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, const oaDeviceContextHandle& deviceContextHandleToBeSent)
{
    const gtByte* pDataBuffer = (const gtByte*)&deviceContextHandleToBeSent;
    bool rc = ipcChannel.write(pDataBuffer, statDeviceContextHandleSize);

    // Sanity test:
    GT_ASSERT(rc);

    return ipcChannel;
}

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
// In Linux, osPbufferHandler = XID = unsigned long, so we do not include this operator
// To avoid ambiguity.
#else
// ---------------------------------------------------------------------------
// Name:        operator<<
// Description: Writes a pBufferHandler value into an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        21/11/2007
// ---------------------------------------------------------------------------
osChannel& operator<<(osChannel& ipcChannel, const oaPBufferHandle& pBufferHandlerToBeSent)
{
    const gtByte* pDataBuffer = (const gtByte*)&pBufferHandlerToBeSent;
    bool rc = ipcChannel.write(pDataBuffer, statPBufferHandlerSize);

    // Sanity test:
    GT_ASSERT(rc);

    return ipcChannel;
}
#endif

// Mac OS X:
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)

// CGL parameters are irrelevant on the iPhone:
#ifndef _GR_IPHONE_BUILD
osChannel& operator<<(osChannel& ipcChannel, CGLPixelFormatAttribute cglPixelFormat)
{
    const gtByte* pDataBuffer = (const gtByte*)&cglPixelFormat;
    bool rc = ipcChannel.write(pDataBuffer, statCGLPixelFormatAttributeSize);

    return ipcChannel;
}

osChannel& operator<<(osChannel& ipcChannel, CGLContextEnable cglContextEnable)
{
    const gtByte* pDataBuffer = (const gtByte*)&cglContextEnable;
    bool rc = ipcChannel.write(pDataBuffer, statCGLContextEnableSize);

    return ipcChannel;
}

osChannel& operator<<(osChannel& ipcChannel, CGLContextParameter cglContextParameter)
{
    const gtByte* pDataBuffer = (const gtByte*)&cglContextParameter;
    bool rc = ipcChannel.write(pDataBuffer, statCGLContextParameterSize);

    return ipcChannel;
}

osChannel& operator<<(osChannel& ipcChannel, CGLGlobalOption cglGlobalOption)
{
    const gtByte* pDataBuffer = (const gtByte*)&cglGlobalOption;
    bool rc = ipcChannel.write(pDataBuffer, statCGLGlobalOptionSize);

    return ipcChannel;
}

osChannel& operator<<(osChannel& ipcChannel, CGLRendererProperty cglRendererProperty)
{
    const gtByte* pDataBuffer = (const gtByte*)&cglRendererProperty;
    bool rc = ipcChannel.write(pDataBuffer, statCGLRendererPropertySize);

    return ipcChannel;
}
#endif //_GR_IPHONE_BUILD

#endif

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
// In Linux, osPbufferHandler = XID = unsigned long, so we do not include this operator
// To avoid ambiguity.
#else
// ---------------------------------------------------------------------------
// Name:        operator>>
// Description: Reads a pBufferHandler value from an IPC channel.
// Author:      AMD Developer Tools Team
// Date:        21/11/2007
// ---------------------------------------------------------------------------
osChannel& operator>>(osChannel& ipcChannel, oaPBufferHandle& pBufferHandlerToBeRecieved)
{
    gtByte* pDataBuffer = (gtByte*)&pBufferHandlerToBeRecieved;
    bool rc = ipcChannel.read(pDataBuffer, statPBufferHandlerSize);
    GT_ASSERT(rc);

    return ipcChannel;
}
#endif

// Mac OS X
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)

// CGL parameters are irrelevant on the iPhone:
#ifndef _GR_IPHONE_BUILD

osChannel& operator>>(osChannel& ipcChannel, CGLPixelFormatAttribute& cglPixelFormat)
{
    gtByte* pDataBuffer = (gtByte*)&cglPixelFormat;
    bool rc = ipcChannel.read(pDataBuffer, statCGLPixelFormatAttributeSize);
    GT_ASSERT(rc);

    return ipcChannel;
}
osChannel& operator>>(osChannel& ipcChannel, CGLContextEnable& cglContextEnable)
{
    gtByte* pDataBuffer = (gtByte*)&cglContextEnable;
    bool rc = ipcChannel.read(pDataBuffer, statCGLContextEnableSize);
    GT_ASSERT(rc);

    return ipcChannel;
}
osChannel& operator>>(osChannel& ipcChannel, CGLContextParameter& cglContextParameter)
{
    gtByte* pDataBuffer = (gtByte*)&cglContextParameter;
    bool rc = ipcChannel.read(pDataBuffer, statCGLContextParameterSize);
    GT_ASSERT(rc);

    return ipcChannel;
}
osChannel& operator>>(osChannel& ipcChannel, CGLGlobalOption& cglGlobalOption)
{
    gtByte* pDataBuffer = (gtByte*)&cglGlobalOption;
    bool rc = ipcChannel.read(pDataBuffer, statCGLGlobalOptionSize);
    GT_ASSERT(rc);

    return ipcChannel;
}
osChannel& operator>>(osChannel& ipcChannel, CGLRendererProperty& cglRendererProprty)
{
    gtByte* pDataBuffer = (gtByte*)&cglRendererProprty;
    bool rc = ipcChannel.read(pDataBuffer, statCGLRendererPropertySize);
    GT_ASSERT(rc);

    return ipcChannel;
}
#endif //_GR_IPHONE_BUILD

#endif



