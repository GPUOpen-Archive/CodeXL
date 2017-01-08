//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AmdtCpuProfiling.h
/// \brief  The CodeAnalyst component class for initial interfacing with the framework
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/AmdtCpuProfiling.h#16 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================
#ifndef _AmdtCpuProfiling_H
#define _AmdtCpuProfiling_H

// Local:
#include <inc/DllExport.h>

class ActionsExecutor;
class SessionViewCreator;
class CpuProjectSettings;
class CustomProfileProjectSettingsExtension;
class CpuGlobalSettings;
class CpuProjectHandler;
class tpProjectSettingsExtension;

/// This class is responsible for controlling the framework interaction
class AMDT_CPU_PROF_API AmdtCpuProfiling
{
public:
    /// Get the singleton instance
    static AmdtCpuProfiling& instance();
    /// Initialize the and all static items class
    void initialize();
    /// Initialize all widgets that aren't registered with a creator
    void initializeIndependentWidgets();
    ///Initialize the items in a static menu environment (no actions registered, like Visual Studio)
    void initializeStatic();

    void resetGuiLayout();

    /// Access the session list creator
    static SessionViewCreator* sessionViewCreator();

    // Access the project settings extension directly
    static CpuProjectSettings* cpuProjectSettings();

protected:

    /// Do not allow the use of the constructor:
    AmdtCpuProfiling();

    /// The singleton instance
    static AmdtCpuProfiling* m_pMySingleInstance;

    /// Contains the CPU profile actions creator:
    static ActionsExecutor* m_pCpuProfileActionExecutor;

    /// Contains the Session MDI view creator:
    static SessionViewCreator* m_pSessionViewCreator;

    ///Contains the project settings extension
    static CpuProjectSettings* m_pProjectSettingsExtension;

    ///Contains the project settings extension
    static CustomProfileProjectSettingsExtension* m_pCustomProjectSettingsExtension;

    // Contains the application event observer:
    static CpuProjectHandler* m_pProjectEventObserver;

    // Contains the project settings extension:
    static tpProjectSettingsExtension* m_psProjectSettingsExtension;

    ///Contains the options/global settings
    CpuGlobalSettings* m_pOptions;

};

extern "C"
{
    // check validity of the plugin:
    int AMDT_CPU_PROF_API CheckValidity(gtString& errString);

    /// The function called by the framework to initialize this component
    void AMDT_CPU_PROF_API initialize();

    /// Initializes other items after main window creation:
    void AMDT_CPU_PROF_API initializeIndependentWidgets();
};

#endif //_AmdtCpuProfiling_H
