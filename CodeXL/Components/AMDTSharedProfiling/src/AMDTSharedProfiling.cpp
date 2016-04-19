//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTSharedProfiling.cpp
///
//==================================================================================

// AMDTSharedProfiling.cpp : Defines the exported functions for the DLL application.
//

#ifdef _WIN32
    #include <Windows.h>
#endif

#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

//Application Framework
#include <AMDTApplicationFramework/Include/afQtCreatorsManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>

// Local:
#include <AMDTSharedProfiling/inc/AMDTSharedProfiling.h>
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>
#include <AMDTSharedProfiling/inc/SharedMenuActions.h>
#include <AMDTSharedProfiling/inc/SharedProfileManager.h>


// Static members:
AmdtSharedProfiling* AmdtSharedProfiling::m_pMySingleInstance = NULL;
SharedMenuActions* AmdtSharedProfiling::m_pSharedMenuActions = NULL;
SharedProfileManager* AmdtSharedProfiling::m_pSharedProfileManager = NULL;

AmdtSharedProfiling::AmdtSharedProfiling()
{
}

AmdtSharedProfiling& AmdtSharedProfiling::instance()
{
    // If this class single instance was not already created:
    if (NULL == m_pMySingleInstance)
    {
        // Create it:
        m_pMySingleInstance = new AmdtSharedProfiling;
        GT_ASSERT(m_pMySingleInstance);
    }

    return *m_pMySingleInstance;
}

void AmdtSharedProfiling::initialize()
{
    //Initialize the profile manager
    m_pSharedMenuActions = &SharedMenuActions::instance();


    m_pSharedProfileManager = &SharedProfileManager::instance();


    afExecutionModeManager::instance().registerExecutionMode(m_pSharedProfileManager);
    afPluginConnectionManager::instance().registerRunModeManager(m_pSharedProfileManager);
}

SharedMenuActions* AmdtSharedProfiling::profileManager()
{
    return m_pSharedMenuActions;
}

void AmdtSharedProfiling::initializeIndependentWidgets()
{
    //Not currently used
}

void AmdtSharedProfiling::resetGuiLayout()
{
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();

    GT_IF_WITH_ASSERT(pMainWindow != NULL)
    {
        pMainWindow->resetInitialLayout();
    }
}



extern "C"
{
    // 0 = no error
    // != 0 error value and error string contains the error
    int AMDTSHAREDPROFILING_API CheckValidity(gtString& errString)
    {
        int retVal = 0;

        errString = L"";

        return retVal;
    }
    void AMDTSHAREDPROFILING_API initialize()
    {
        AmdtSharedProfiling::instance().initialize();
    }
    void AMDTSHAREDPROFILING_API initializeIndependentWidgets()
    {
        AmdtSharedProfiling::instance().initializeIndependentWidgets();
    }

}; // extern "C"

