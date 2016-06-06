//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apInterceptionMethod.h
///
//==================================================================================

//------------------------------ apInterceptionMethod.h ------------------------------

#ifndef __APINTERCEPTIONMETHOD_H
#define __APINTERCEPTIONMETHOD_H

// Defines modules and functions interception methods.
enum apInterceptionMethod
{
    AP_SET_DLL_DIRECTORY_INTERCEPTION   = 0,    // Intercept using the Win32 SetDllDirectory function.
    AP_PRELOAD_INTERCEPTION             = 1,    // Intercept using "pre-load" environment variable.
    AP_LIBRARY_PATH_INTERCEPTION        = 2     // Intercept using "library path" environment variable.
};


// Defines the default interception method (per platform):
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define AP_DEFAULT_INTERCEPTION_METHOD AP_SET_DLL_DIRECTORY_INTERCEPTION
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
        #define AP_DEFAULT_INTERCEPTION_METHOD AP_LIBRARY_PATH_INTERCEPTION
    #elif AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
        #define AP_DEFAULT_INTERCEPTION_METHOD AP_PRELOAD_INTERCEPTION
    #else
        #error Error: Unknown Linux variant
    #endif
#endif

#endif //__APINTERCEPTIONMETHOD_H

