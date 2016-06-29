//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsGlobalVariables.cpp
///
//==================================================================================

//------------------------------ gsGlobalVariables.cpp ------------------------------

// Infra:
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTAPIClasses/Include/apCounterID.h>

// Local:
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsExtensionsManager.h>
#include <src/gsGlobalVariables.h>

// iPhone only:
#ifdef _GR_IPHONE_DEVICE_BUILD
    #include <AMDTOSWrappers/Include/osMacSystemResourcesSampler.h>
    #include <src/gsOSPerformanceCountersManager.h>
    #include <src/gsiPhoneGPUPerformanceCountersReader.h>
#endif

// Desktop OpenGL Servers only:
#ifdef _AMDT_OPENGLSERVER_EXPORTS
    #include <src/gsDeprecationAnalyzer.h>
#endif

// The handle of the system's OpenGL module:
osModuleHandle gs_stat_systemsOpenGLModuleHandle = OS_NO_MODULE_HANDLE;

// Mac OSX only:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    // The handle of the system's OpenGL framework module:
    osModuleHandle gs_stat_systemsOpenGLFrameworkModuleHandle = OS_NO_MODULE_HANDLE;

    #ifdef _GR_IPHONE_DEVICE_BUILD
        osMacSystemResourcesSampler gs_stat_osSystemResourcesSampler;
        gsOSPerformanceCountersManager gs_stat_osPerformanceManager(gs_stat_osSystemResourcesSampler);
        gsiPhoneGPUPerformanceCountersReader gs_stat_iPhoneGPUPerformanceCountersReader;
    #endif
#endif

// Contains pointers to the real implementation of the wrapped "base" functions:
gsMonitoredFunctionPointers gs_stat_realFunctionPointers;

// Contains pointers to the real implementation of driver-internal functions:
gsDriverInternalFunctionPointers gs_stat_realDriverInternalFunctionPointers;

// Reference to the singleton instances:
gsOpenGLMonitor& gs_stat_openGLMonitorInstance = gsOpenGLMonitor::instance();
gsExtensionsManager& gs_stat_extensionsManager = gsExtensionsManager::instance();

#ifdef _AMDT_OPENGLSERVER_EXPORTS
    gsDeprecationAnalyzer& gs_stat_deprecationAnalizer = gsDeprecationAnalyzer::instance();
#endif


// The maximal size of arrays that will be logged:
unsigned int gs_stat_maxLoggedArraySize = 15;

// Is in NULL OpenGL implementation mode:
bool gs_stat_isInNullOpenGLImplementationMode = false;


// ---------------------------------------------------------------------------
// Name:        gsMaxLoggedArraySize
// Description: Returns the maximal size of arrays that will be logged.
// Author:      Yaki Tebeka
// Date:        9/9/2007
// ---------------------------------------------------------------------------
int gsMaxLoggedArraySize()
{
    return gs_stat_maxLoggedArraySize;
}


// ---------------------------------------------------------------------------
// Name:        gsSetMaxLoggedArraySize
// Description: Sets the maximal size of arrays that will be logged.
// Arguments: maxLoggedArraySize - The maximal logged array size.
// Author:      Yaki Tebeka
// Date:        9/9/2007
// ---------------------------------------------------------------------------
void gsSetMaxLoggedArraySize(int maxLoggedArraySize)
{
    gs_stat_maxLoggedArraySize = maxLoggedArraySize;
}


// ---------------------------------------------------------------------------
// Name:        gsIsNullOpenGLImplementation
// Description: Returns true iff we are in NULL OpenGL implementation mode.
// Author:      Yaki Tebeka
// Date:        9/9/2007
// ---------------------------------------------------------------------------
bool gsIsNullOpenGLImplementation()
{
    return gs_stat_isInNullOpenGLImplementationMode;
}


// ---------------------------------------------------------------------------
// Name:        gsSetNULLOpenGLImplementationMode
// Description: Turns on / off NULL OpenGL implementation mode.
// Arguments: isNullOpenGLImplementation - true - turn on NULL OpenGL implementation mode.
//                                       - false - turn off NULL OpenGL implementation mode.
// Author:      Yaki Tebeka
// Date:        9/9/2007
// ---------------------------------------------------------------------------
void gsSetNULLOpenGLImplementationMode(bool isNullOpenGLImplementation)
{
    gs_stat_isInNullOpenGLImplementationMode = isNullOpenGLImplementation;
}


// ---------------------------------------------------------------------------
// Name:        gsSystemsOpenGLModuleHandle
// Description: Returns the loaded system's OpenGL module handle.
// Author:      Yaki Tebeka
// Date:        9/9/2007
// ---------------------------------------------------------------------------
osModuleHandle gsSystemsOpenGLModuleHandle()
{
    return gs_stat_systemsOpenGLModuleHandle;
}


// ---------------------------------------------------------------------------
// Name:        gsSetSystemsOpenGLModuleHandle
// Description: Stores the loaded system's OpenGL module handle.
// Arguments: systemsOpenGLModuleHandle - The loaded system's OpenGL module handle.
// Author:      Yaki Tebeka
// Date:        9/9/2007
// ---------------------------------------------------------------------------
void gsSetSystemsOpenGLModuleHandle(osModuleHandle systemsOpenGLModuleHandle)
{
    gs_stat_systemsOpenGLModuleHandle = systemsOpenGLModuleHandle;
}


// Mac OSX only:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

// ---------------------------------------------------------------------------
// Name:        gsSystemsOpenGLFrameworkModuleHandle
// Description: Returns the system's loaded OpenGL framework handle.
// Author:      Yaki Tebeka
// Date:        15/12/2008
// ---------------------------------------------------------------------------
osModuleHandle gsSystemsOpenGLFrameworkModuleHandle()
{
    return gs_stat_systemsOpenGLFrameworkModuleHandle;
}


// ---------------------------------------------------------------------------
// Name:        gsSetSystemsOpenGLFrameworkModuleHandle
// Description: Stores the loaded system's OpenGL framework module handle.
// Arguments: systemsOpenGLFrameworkModuleHandle - The loaded system's OpenGL framework module handle.
// Author:      Yaki Tebeka
// Date:        15/12/2008
// ---------------------------------------------------------------------------
void gsSetSystemsOpenGLFrameworkModuleHandle(osModuleHandle systemsOpenGLFrameworkModuleHandle)
{
    gs_stat_systemsOpenGLFrameworkModuleHandle = systemsOpenGLFrameworkModuleHandle;
}

#endif // Mac OS X code only

