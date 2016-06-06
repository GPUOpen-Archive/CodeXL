//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaOpenCLIncludes.h
///
//=====================================================================

//------------------------------ oaOpenCLIncludes.h ------------------------------

#ifndef __OAOPENCLINCLUDES_H
#define __OAOPENCLINCLUDES_H

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// OpenCL header files:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    #define WIN32_LEAN_AND_MEAN 1
    #include <Windows.h>
    #include <CL/cl_platform.h>
    #include <CL/cl.h>
    #include <CL/cl_ext.h>
    #include <CL/cl_gl_ext.h>
    #include <CL/cl_gl.h>
    #include <CL/cl_dx9_media_sharing.h>
    #include <d3d10_1.h>
    #include <CL/cl_d3d10.h>
    #include <CL/cl_d3d11.h>
    #include <AMDTOSAPIWrappers/Include/CL/AMDCLExtensions.h>
    #include <AMDTOSAPIWrappers/Include/CL/cl_additions.h>

#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS

    // Yaki 16/11/06:
    // This definition tells gl.h (included in opencl.h and cl_gl.h) to don't include glext.h, enabling us to include
    // our glext.h version instead of the /usr/include/GL file:
    #define GL_GLEXT_LEGACY

    #if AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT

        // cl.h uses the AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER macro. Since we want
        // to cheat the compiler to work even on Snow leopard (since we don't link to OpenCL,
        // this shouldn't cause any problems)
        #ifdef AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER
            #define AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER_EXISTED
        #else
            #define AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER
        #endif

        //  Add prototypes for CL extension functions:
        #define CL_EXT_PROTOTYPES 1

        #include <CL/mac/OpenCL/opencl.h>
        #include <CL/mac/OpenCL/cl_gl_additions.h>
        #include <AMDTOSAPIWrappers/Include/CL/AMDCLExtensions.h>

        // undefine the macro only if it didn't exist before
        #ifndef AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER_EXISTED
            #undef AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER
        #endif

        // An OpenCL error that does not yet exist on Mac:
        #define CL_INVALID_GLOBAL_WORK_SIZE -63

    #elif AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT

        #include <CL/cl_platform.h>
        #include <CL/cl.h>
        #include <CL/cl_ext.h>
        #include <CL/cl_gl_ext.h>
        #include <CL/cl_gl.h>
        #include <AMDTOSAPIWrappers/Include/CL/AMDCLExtensions.h>
        #include <AMDTOSAPIWrappers/Include/CL/cl_additions.h>

    #else
        #error Unknown Linux variant!
    #endif // AMDT_LINUX_VARIANT
#else
    #error Unknown build target!
#endif // AMDT_BUILD_TARGET

#endif //__OAOPENCLINCLUDES_H

