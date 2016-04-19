//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsGlobalVariables.h
///
//==================================================================================

//------------------------------ gsGlobalVariables.h ------------------------------

#ifndef __GSGLOBALVARIABLES_H
#define __GSGLOBALVARIABLES_H

// ----------------------------------------------------------------------------------
// File Name:   gsGlobalVariables
// General Description:
//   This file (and its .cpp) contains global variables used by various spy files.
//   All global variables are grouped in one file to ensure proper variables
//   initialization sequence; variables that should be initialized first should
//   appear first in the gsGlobalVariables.cpp file (C++ ensures right initialization
//   sequence within the same file, but not across files).
//
//   When applicable (performance wise, etc), prefer using global variables access functions.
//   This will enable locating the call stack that led to a global variable change.
//
// Author:               Yaki Tebeka
// Creation Date:        12/4/2005
// ----------------------------------------------------------------------------------

// Forward decelerations:
class apMonitoredFunctionsManager;
struct gsMonitoredFunctionPointers;
class gsOpenGLMonitor;
class gsExtensionsManager;
class gsSpyPerformanceCountersManager;
class gsATIPerformanceCountersManager;

// iPhone device-only forward declarations, see cpp files
#ifdef _GR_IPHONE_DEVICE_BUILD
    class gsOSPerformanceCountersManager;
    class gsiPhoneGPUPerformanceCountersReader;
#endif

// OpenGL-only (not ES) forward declarations:
#ifdef _AMDT_OPENGLSERVER_EXPORTS
    class gsDeprecationAnalyzer;
#endif

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>


// -----------------------------------------------------------
//           Global variables access functions
// -----------------------------------------------------------

int gsMaxLoggedArraySize();
void gsSetMaxLoggedArraySize(int maxLoggedArraySize);
bool gsIsNullOpenGLImplementation();
void gsSetNULLOpenGLImplementationMode(bool isNullOpenGLImplementation);
osModuleHandle gsSystemsOpenGLModuleHandle();
void gsSetSystemsOpenGLModuleHandle(osModuleHandle systemsOpenGLModuleHandle);

// Mac OSX only:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    osModuleHandle gsSystemsOpenGLFrameworkModuleHandle();
    void gsSetSystemsOpenGLFrameworkModuleHandle(osModuleHandle systemsOpenGLFrameworkModuleHandle);
#endif

// -----------------------------------------------------------
//            Global variables direct access
// -----------------------------------------------------------

// Contains pointers to the real implementation of the wrapped "base" functions:
extern gsMonitoredFunctionPointers gs_stat_realFunctionPointers;

// References to singletons instances:
extern gsOpenGLMonitor& gs_stat_openGLMonitorInstance;
extern gsExtensionsManager& gs_stat_extensionsManager;

#ifdef _GR_IPHONE_DEVICE_BUILD
    extern gsOSPerformanceCountersManager gs_stat_osPerformanceManager;
    extern gsiPhoneGPUPerformanceCountersReader gs_stat_iPhoneGPUPerformanceCountersReader;
#endif

#ifdef _AMDT_OPENGLSERVER_EXPORTS
    extern gsDeprecationAnalyzer& gs_stat_deprecationAnalizer;
#endif

// The maximal size of arrays that will be logged:
extern unsigned int gs_stat_maxLoggedArraySize;

// Is in NULL OpenGL implementation mode:
extern bool gs_stat_isInNullOpenGLImplementationMode;


#endif //__GSGLOBALVARIABLES_H

