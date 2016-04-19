//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppAppWrapper.h
///
//==================================================================================

//------------------------------ ppAppWrapper.h ------------------------------

#ifndef __PPAPPWRAPPER_H
#define __PPAPPWRAPPER_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>
#include <AMDTPowerProfiling/src/ppMDIViewCreator.h>

// Forward declarations:
class ppProjectSettingsExtension;
class ppMDIViewCreator;
class ppMenuActionsExecutor;

class PP_API ppAppWrapper
{
public:

    static ppAppWrapper& instance();
    int CheckValidity(gtString& errString);
    void initialize();
    void initializeIndependentWidgets();

    ppMDIViewCreator* MDIViewCreator() const { return m_psMDIViewCreator; };


    // marks if the prerequisite of this plugin were met needs access
    static bool s_loadEnabled;

protected:

    // Do not allow the use of my default constructor:
    ppAppWrapper();

    static ppAppWrapper* m_spMySingleInstance;

    // Contains the project settings extension
    static ppProjectSettingsExtension* m_psProjectSettingsExtension;

    // Contains the views creator
    static ppMDIViewCreator* m_psMDIViewCreator;

    // Contains the main menu actions creator
    static ppMenuActionsExecutor* m_spActionsCreator;
};

extern "C"
{
    // check validity of the plugin:
    int PP_API CheckValidity(gtString& errString);

    // initialize function for this wrapper:
    void PP_API initialize();

    // Initialize other items after main window creation:
    void PP_API initializeIndependentWidgets();
};


#endif //__PPAPPWRAPPER_H

