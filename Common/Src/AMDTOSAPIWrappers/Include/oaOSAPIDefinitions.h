//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaOSAPIDefinitions.h
///
//=====================================================================

//------------------------------ oaOSAPIDefinitions.h ------------------------------

#ifndef __OAOSAPIDEFINITIONS
#define __OAOSAPIDEFINITIONS

//////////////////////////////////////////////////////////////////////////
// A few notes about some of the types defined in this file:            //
//////////////////////////////////////////////////////////////////////////
// oaPixelFormatId - a (preferably unique) identifier of a pixel format.
//      Ideally, this would be a numeric identifier (which is how it
//      works in Windows and Linux). However, in Mac OS X, pixel formats
//      cannot be enumerated, thus we use the system pixel format (which
//      is a opaque struct pointer) as the ID. Because of this, one
//      should not assume that oaPixelFormatId is numeric (eg do not set
//      it as equal to an int). The appropriate "zero" value is defined
//      by OA_NO_PIXEL_FORMAT_ID and is also platform dependent. It is
//      even more important to use this value, as setting a pixel format
//      to 0 will compile in all implementations, but the "n/a" value in
//      Windows is -1 and not 0. Also, in Linux this type is unsigned.
// oaPixelFormatHandle - a handle to a pixel format. This is currently
//      used only in Linux, where we need to use the same visual info in
//      creating windows and contexts so they can be used together.
//      The "zero" value is OA_NO_PIXEL_FORMAT_HANDLE.
// oaVendorType - an enumeration for the graphic card vendor
//////////////////////////////////////////////////////////////////////////

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// ------------------------ Common definitions ------------------------

typedef enum
{
    OA_VENDOR_ATI,
    OA_VENDOR_NVIDIA,
    OA_VENDOR_INTEL,
    OA_VENDOR_S3,
    OA_VENDOR_MICROSOFT,
    OA_VENDOR_MESA,
    OA_VENDOR_UNKNOWN
} oaVendorType;

//////////////////////////////////////////////////////////////////////////
// Yaki, 17/1/10: Leaving the option to disable OpenCL and iPhone
// on-device, until we will be certain that we would like to release them
//////////////////////////////////////////////////////////////////////////
#define OA_ENABLE_IPHONE_DEVICE_FUNCTIONALITY

// Version 5.8: Removing OpenGL ES support. The first two #defines are platform-specific,
// comment them out to restore support. The other #define (OA_REMOVE_OPENGL_ES_SUPPORT)
// considers the current build environment, it is the only one that should be used in the code
#define OA_REMOVE_CodeXL_ES_FUNCTIONALITY
#define OA_REMOVE_CodeXL_IPHONE_FUNCTIONALITY

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    #ifdef OA_REMOVE_CodeXL_ES_FUNCTIONALITY
        #define OA_REMOVE_OPENGL_ES_SUPPORT
    #endif
#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    #ifdef OA_REMOVE_CodeXL_IPHONE_FUNCTIONALITY
        #define OA_REMOVE_OPENGL_ES_SUPPORT
    #endif
#endif // AMDT_BUILD_TARGET

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

// ------------------------ Win32 definitions ------------------------

// Window handle:
typedef HWND oaWindowHandle;

// Device context handle:
typedef HDC oaDeviceContextHandle;

// Drawable handle:
typedef HWND oaDrawableHandle;

// The id of a pixel format (in its device context):
typedef int oaPixelFormatId;
#define OA_NO_PIXEL_FORMAT_ID -1

// Pixel format handle:
typedef void* oaPixelFormatHandle;
#define OA_NO_PIXEL_FORMAT_HANDLE NULL

// OpenGL render context handle:
typedef HGLRC oaOpenGLRenderContextHandle;

// PBuffer handler:
typedef HANDLE oaPBufferHandle;

// Handles for OpenGL objects in the Spy (we do not know if the spy has the same address space as we do):
typedef gtUInt64 oaGLSyncHandle;
typedef gtUInt64 oaGLHandle;
#define OA_GL_NULL_HANDLE ((oaGLHandle)NULL)

// Handles for OpenCL objects in the Spy (we do not know if the spy has the same address space as we do):
typedef gtUInt64 oaCLPlatformID;
typedef gtUInt64 oaCLDeviceID;
typedef gtUInt64 oaCLContextHandle;
typedef gtUInt64 oaCLContextProperty;
typedef gtUInt64 oaCLCommandQueueHandle;
typedef gtUInt64 oaCLMemHandle;
typedef gtUInt64 oaCLProgramHandle;
typedef gtUInt64 oaCLKernelHandle;
typedef gtUInt64 oaCLEventHandle;
typedef gtUInt64 oaCLSamplerHandle;
typedef gtUInt64 oaCLHandle; // Generic handle for all the above
#define OA_CL_NULL_HANDLE ((oaCLHandle)NULL)

// Specific function names in the spies:
#define OA_SPIES_BREAKPOINT_FUNCTION_NAME L"suBreakpointsManager::triggerBreakpointException"

// If this is an OpenGL ES implementation DLL:
#ifdef AEE_SIMULATOR
    #define OA_OGL_ES_IMPLEMENTATION_DLL_BUILD
#endif
//////////////////////////////////////////////////////////////////////////
// Uri, 2014-04-27: #if-ing out all code related to GPA from the        //
// debugger and debugger servers. The code is left mostly intact but    //
// commented out, to avoid having to recreate all the infrastructure    //
// for it, should we to use it again.                                   //
//////////////////////////////////////////////////////////////////////////
// #define OA_DEBUGGER_USE_AMD_GPA 1

#ifdef OA_DEBUGGER_USE_AMD_GPA
    #if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
        #define OA_ATI_PERF_OPENGL_MODULE_NAME L"GPUPerfAPIGL"
        #define OA_ATI_PERF_OPENCL_MODULE_NAME L"GPUPerfAPICL"
    #elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
        #define OA_ATI_PERF_OPENGL_MODULE_NAME L"GPUPerfAPIGL-x64"
        #define OA_ATI_PERF_OPENCL_MODULE_NAME L"GPUPerfAPICL-x64"
    #endif
#endif

// Bitmap Red, Green, Blue and Alpha channels array index:
#define OA_BITMAP_RED_CHANNEL_INDEX 0
#define OA_BITMAP_GREEN_CHANNEL_INDEX 1
#define OA_BITMAP_BLUE_CHANNEL_INDEX 2
#define OA_BITMAP_ALPHA_CHANNEL_INDEX 3


#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS

// ------------------------ Linux definitions ------------------------

// GL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Handles for OpenGL objects in the Spy (we do not know if the spy has the same address space as we do):
typedef gtUInt64 oaGLSyncHandle;
typedef gtUInt64 oaGLHandle;
#define OA_GL_NULL_HANDLE ((oaGLHandle)NULL)

// Handles for OpenCL objects in the Spy (we do not know if the spy has the same address space as we do):
typedef gtUInt64 oaCLPlatformID;
typedef gtUInt64 oaCLDeviceID;
typedef gtUInt64 oaCLContextHandle;
typedef gtUInt64 oaCLContextProperty;
typedef gtUInt64 oaCLCommandQueueHandle;
typedef gtUInt64 oaCLMemHandle;
typedef gtUInt64 oaCLProgramHandle;
typedef gtUInt64 oaCLKernelHandle;
typedef gtUInt64 oaCLEventHandle;
typedef gtUInt64 oaCLSamplerHandle;
typedef gtUInt64 oaCLHandle; // Generic handle for all the above
#define OA_CL_NULL_HANDLE ((oaCLHandle)0)

// Bitmap Red, Green, Blue and Alpha channels array index:
#define OA_BITMAP_RED_CHANNEL_INDEX 2
#define OA_BITMAP_GREEN_CHANNEL_INDEX 1
#define OA_BITMAP_BLUE_CHANNEL_INDEX 0
#define OA_BITMAP_ALPHA_CHANNEL_INDEX 3

#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT || AMDT_LINUX_VARIANT == AMDT_ANDROID_LINUX_VARIANT

// ----------------- Generic Linux variants only -----------------

// X11:
#include <X11/Xlib.h>

#if AMDT_LINUX_VARIANT == AMDT_ANDROID_LINUX_VARIANT
    // Drawable handle:
    typedef EGLSurface oaDrawableHandle;

    // Device context handle:
    typedef EGLDisplay oaDeviceContextHandle;

    // OpenGL render context handle:
    typedef EGLContext oaOpenGLRenderContextHandle;

    // Pixel format handle:
    typedef void* oaPixelFormatHandle;
#else
    // Drawable handle:
    typedef GLXDrawable oaDrawableHandle;

    // Device context handle:
    typedef Display* oaDeviceContextHandle;

    // OpenGL render context handle:
    typedef GLXContext oaOpenGLRenderContextHandle;

    // Pixel format handle:
    typedef XVisualInfo* oaPixelFormatHandle;

#endif // AMDT_LINUX_VARIANT

#define OA_NO_PIXEL_FORMAT_HANDLE ((oaPixelFormatHandle)NULL)

// Pixel format id:
typedef VisualID oaPixelFormatId;
#define OA_NO_PIXEL_FORMAT_ID 0

// PBuffer handler:
typedef XID oaPBufferHandle;

// Window handle:
typedef Window oaWindowHandle;

#elif AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT

// ----------------- Mac OS X only  -----------------

// Do not include X11 headers, since they sometimes collide with Mac OS X headers

#ifndef _GR_IPHONE_BUILD
    // Forward declarations of CGL types:
    typedef struct _CGLPixelFormatObject* CGLPixelFormatObj;
    typedef struct _CGLContextObject* CGLContextObj;
    typedef struct _CGLPBufferObject* CGLPBufferObj;

    // OpenGL render context handle:
    // was unsigned long - Uri is researching for the right type
    typedef CGLContextObj oaOpenGLRenderContextHandle;

    // PBuffer handler:
    typedef CGLPBufferObj oaPBufferHandle;

    // Pixel format id:
    typedef CGLPixelFormatObj oaPixelFormatId;
    #define OA_NO_PIXEL_FORMAT_ID NULL

    // Pixel format handle:
    typedef void* oaPixelFormatHandle;
    #define OA_NO_PIXEL_FORMAT_HANDLE NULL

#else // _GR_IPHONE_BUILD
    // TO_DO: iPhone
    typedef void* oaOpenGLRenderContextHandle;
    typedef void** oaPBufferHandle;
    typedef int oaPixelFormatId;
    #define OA_NO_PIXEL_FORMAT_ID 0
    typedef void* oaPixelFormatHandle;
    #define OA_NO_PIXEL_FORMAT_HANDLE NULL

#endif

// Drawable handle:
typedef unsigned long oaDrawableHandle;

// Device context handle:
typedef void* oaDeviceContextHandle;

// Window handle:
typedef void* oaWindowHandle;

// EAGL types:
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
    typedef unsigned int NSUInteger;
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    typedef unsigned long NSUInteger;
#else
    #error Unknown address space size
#endif
#ifndef _EAGL_H_
enum
{
    kEAGLRenderingAPIOpenGLES1 = 1,
    kEAGLRenderingAPIOpenGLES2 = 2
};
#endif
typedef NSUInteger EAGLRenderingAPI;

#endif

#endif // AMDT_BUILD_TARGET == AMDT_LINUX_OS



#endif  // __OAOSAPIDEFINITIONS
