//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ oaChannelOperators.h ------------------------------

#ifndef __OACHANNELOPERATORS
#define __OACHANNELOPERATORS

// Forward declaration:
class osChannel;
class osTime;
class osRawMemoryStream;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIWrappersDLLBuild.h>


// ----  Write into IPC channel operators ---- :
OA_API osChannel& operator<<(osChannel& ipcChannel, const oaDeviceContextHandle& deviceContextHandleToBeSent);

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    // In Linux, osPbufferHandler = XID = unsigned long, so we do not include this operator
    // To avoid ambiguity.
#else
    OA_API osChannel& operator<<(osChannel& ipcChannel, const oaPBufferHandle& pBufferHandlerToBeSent);
#endif


// Mac OS X
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    // CGL is not relevant to OpenGL ES
    #ifndef _GR_IPHONE_BUILD
        OA_API osChannel& operator<<(osChannel& ipcChannel, CGLPixelFormatAttribute cglPixelFormat);
        OA_API osChannel& operator<<(osChannel& ipcChannel, CGLContextEnable cglContextEnable);
        OA_API osChannel& operator<<(osChannel& ipcChannel, CGLContextParameter cglContextParameter);
        OA_API osChannel& operator<<(osChannel& ipcChannel, CGLGlobalOption cglGlobalOption);
        OA_API osChannel& operator<<(osChannel& ipcChannel, CGLRendererProperty cglRendererProperty);
    #endif
#endif

// ----  Read from IPC channel operators ---- :
OA_API osChannel& operator>>(osChannel& ipcChannel, oaDeviceContextHandle& deviceContextHandleToBeRecieved);

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    // In Linux, osPbufferHandler = XID = unsigned long, so we do not include this operator
    // To avoid ambiguity.
#else
    OA_API osChannel& operator>>(osChannel& ipcChannel, oaPBufferHandle& pBufferHandlerToBeRecieved);
#endif

// Mac OS X
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    // CGL is not relevant to OpenGL ES
    #ifndef _GR_IPHONE_BUILD
        OA_API osChannel& operator>>(osChannel& ipcChannel, CGLPixelFormatAttribute& cglPixelFormat);
        OA_API osChannel& operator>>(osChannel& ipcChannel, CGLContextEnable& cglContextEnable);
        OA_API osChannel& operator>>(osChannel& ipcChannel, CGLContextParameter& cglContextParameter);
        OA_API osChannel& operator>>(osChannel& ipcChannel, CGLGlobalOption& cglGlobalOption);
        OA_API osChannel& operator>>(osChannel& ipcChannel, CGLRendererProperty& cglRendererProprty);
    #endif
#endif


#endif  // __OACHANNELOPERATORS
