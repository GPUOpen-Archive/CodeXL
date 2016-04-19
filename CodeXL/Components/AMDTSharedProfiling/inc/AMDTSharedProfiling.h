//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTSharedProfiling.h
///
//==================================================================================

#ifndef _AMDTSHAREDPROFILING_H
#define _AMDTSHAREDPROFILING_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include "LibExport.h"

class SharedMenuActions;
class SharedProfileManager;
class ProfilerSessionExplorerViewCreator;

/// This class is responsible for controlling the framework interaction
class AMDTSHAREDPROFILING_API AmdtSharedProfiling
{
public:
    /// Get the singleton instance
    static AmdtSharedProfiling& instance();
    /// Initialize the and all static items class
    void initialize();
    /// Initialize all widgets that aren't registered with a creator
    void initializeIndependentWidgets();

    void resetGuiLayout();

    /// Access the profile manager action creator
    static SharedMenuActions* profileManager();

protected:

    /// Do not allow the use of the constructor:
    AmdtSharedProfiling();

    /// The singleton instance
    static AmdtSharedProfiling* m_pMySingleInstance;

    /// Contains the Shared profile manager
    static SharedProfileManager* m_pSharedProfileManager;
    static SharedMenuActions* m_pSharedMenuActions;
    ///Contains the shared session view
    static ProfilerSessionExplorerViewCreator* m_pSharedSessionExplorer;
};

extern "C"
{
    // check validity of the plugin:
    int AMDTSHAREDPROFILING_API CheckValidity(gtString& errString);

    /// The function called by the framework to initialize this component
    void AMDTSHAREDPROFILING_API initialize();

    /// Initializes other items after main window creation:
    void AMDTSHAREDPROFILING_API initializeIndependentWidgets();
};

#endif //_AMDTSHAREDPROFILING_H
