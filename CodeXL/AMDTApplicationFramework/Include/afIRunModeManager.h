//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afIRunModeManager.h
///
//==================================================================================

#ifndef __AFIRUNMODEMANAGER_H
#define __AFIRUNMODEMANAGER_H

// Forward declarations:
class apExceptionEvent;
class osCallStack;

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>



// ----------------------------------------------------------------------------------
// Class Name:          afRunMode
// General Description: A bit mask of modes describing whether there is a session running
//                      in the implementing plugin.
// Author:              Uri Shomroni
// Creation Date:       3/5/2012
// ----------------------------------------------------------------------------------
enum afRunMode
{
    // 0x0001 - 0x0080 reserved for debugger plugin:
    AF_DEBUGGED_PROCESS_EXISTS = 0x0001,
    AF_DEBUGGED_PROCESS_RUNNING = 0x0002,
    AF_DEBUGGED_PROCESS_SUSPENDED = 0x0004,
    AF_DEBUGGED_PROCESS_IN_KERNEL_DEBUGGING = 0x0008,
    AF_DEBUGGED_PROCESS_PAUSED = 0x0010,
    AF_DEBUGGED_PROCESS_DATA_TRANSLATING = 0x0020,

    // 0x0100 - 0x0800 reserved for analyzer plugin:

    // 0x1000 - 0x80c00 reserved for analyzer plugin:
    AF_ANALYZE_CURRENTLY_BUILDING = 0x1000,

    // Frame Analyzer import \ export
    AF_FRAME_ANALYZE_CURRENTLY_EXPORTING = 0x2000,
    AF_FRAME_ANALYZE_CURRENTLY_IMPORTING = 0x4000,
    AF_FRAME_ANALYZE_CONNECTING = 0x8000
};

typedef int afRunModes; // Bitwise OR of afRunMode values

// ----------------------------------------------------------------------------------
// Class Name:          AF_API afIRunModeManager
// General Description: An interface that should be implemented by plugins to allow the
//                      afPluginConnectionManager to query and control run modes
// Author:              Uri Shomroni
// Creation Date:       3/5/2012
// ----------------------------------------------------------------------------------
class AF_API afIRunModeManager
{
public:
    afIRunModeManager() : m_stopAndExit(false) {};
    virtual ~afIRunModeManager() {};

public:
    virtual afRunModes getCurrentRunModeMask() = 0;

    virtual bool canStopCurrentRun() = 0;
    virtual bool stopCurrentRun() = 0;

    virtual bool getExceptionEventDetails(const apExceptionEvent& exceptionEve, osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce) = 0;

    void setStopAndExit(bool value) { m_stopAndExit = value; }
protected:
    bool m_stopAndExit;
};

#endif //__AFIRUNMODEMANAGER_H

