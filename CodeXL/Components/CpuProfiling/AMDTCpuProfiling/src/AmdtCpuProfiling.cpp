//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AmdtCpuProfiling.cpp
/// \brief  Implements the initial framework interface for the CodeAnalyst component
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/AmdtCpuProfiling.cpp#32 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================

#if defined (_WIN32)
    #include <Windows.h>
#endif

//QT
#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

//Application Framework
#include <AMDTApplicationFramework/Include/afQtCreatorsManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osProcess.h>

// AmdtSharedProfiling:
#include <ProfileApplicationTreeHandler.h>
#include <SharedProfileSettingPage.h>

// Local:
#include <inc/ActionsExecutor.h>
#include <inc/AmdtCpuProfiling.h>
#include <inc/CommandsHandler.h>
#include <inc/SessionViewCreator.h>
#include <inc/CustomProfileProjectSettingsExtension.h>
#include <inc/CpuProjectSettings.h>
#include <inc/CpuProjectHandler.h>
#include <inc/CpuGlobalSettings.h>
#include <inc/StringConstants.h>
#include <inc/tpMenuActionsExecutor.h>
#include <inc/tpProjectSettingsExtension.h>
#include <inc/tpAppController.h>
#include <inc/tpMDIViewCreator.h>
#include <inc/tpTreeHandler.h>

// Static members:
AmdtCpuProfiling* AmdtCpuProfiling::m_pMySingleInstance = nullptr;
SessionViewCreator* AmdtCpuProfiling::m_pSessionViewCreator = nullptr;
ActionsExecutor* AmdtCpuProfiling::m_pCpuProfileActionExecutor = nullptr;
CpuProjectSettings* AmdtCpuProfiling::m_pProjectSettingsExtension = nullptr;
CustomProfileProjectSettingsExtension* AmdtCpuProfiling::m_pCustomProjectSettingsExtension = nullptr;
CpuProjectHandler* AmdtCpuProfiling::m_pProjectEventObserver = nullptr;
tpProjectSettingsExtension* AmdtCpuProfiling::m_psProjectSettingsExtension = nullptr;

AmdtCpuProfiling::AmdtCpuProfiling()
{
}

AmdtCpuProfiling& AmdtCpuProfiling::instance()
{
    // If this class single instance was not already created:
    if (nullptr == m_pMySingleInstance)
    {
        // Create it:
        m_pMySingleInstance = new AmdtCpuProfiling;
        GT_ASSERT(m_pMySingleInstance);
    }

    return *m_pMySingleInstance;
}

void AmdtCpuProfiling::initialize()
{
    initializeStatic();
}

void AmdtCpuProfiling::initializeStatic()
{
    //Set up the environmental variable so the event files can be found.
    osEnvironmentVariable eventDataPath;
    eventDataPath._name = L"CPUPerfAPIDataPath";
    osFilePath basePath(osFilePath::OS_CODEXL_DATA_PATH);
    basePath.appendSubDirectory(L"Events");
    eventDataPath._value.appendFormattedString(L"%ls/", basePath.asString().asCharArray());
    osSetCurrentProcessEnvVariable(eventDataPath);

    // Create the single instance
    CommandsHandler* pCommandsHandler = new CommandsHandler;

    bool rcRegsister = CommandsHandler::registerInstance(pCommandsHandler);
    GT_ASSERT_EX(rcRegsister, L"Only a single instance of CommandsHandler should be registered");

    // Create a views creator:
    m_pSessionViewCreator = new SessionViewCreator;


    // Create an action executor:
    m_pCpuProfileActionExecutor = new ActionsExecutor;


    // Register the actions executor:
    afQtCreatorsManager::instance().registerActionExecutor(m_pCpuProfileActionExecutor);

    // Initialize:
    m_pSessionViewCreator->initialize();

    // Register the views creator:
    afQtCreatorsManager::instance().registerViewCreator(m_pSessionViewCreator);

    // Create and register the project settings object:
    m_pProjectSettingsExtension = new CpuProjectSettings;

    afProjectManager::instance().registerProjectSettingsExtension(m_pProjectSettingsExtension);
    afProjectManager::instance().registerToListenExeChanged(m_pProjectSettingsExtension);

    m_pCustomProjectSettingsExtension = new CustomProfileProjectSettingsExtension;

    afProjectManager::instance().registerProjectSettingsExtension(m_pCustomProjectSettingsExtension);


    //create and register the options object:
    m_pOptions = new CpuGlobalSettings;

    afGlobalVariablesManager::instance().registerGlobalSettingsPage(m_pOptions);

    // Create an event observer:
    m_pProjectEventObserver = &(CpuProjectHandler::instance());

    // Initialize the session explorer window:
    ProfileApplicationTreeHandler::instance();

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    gtString envValue;
    bool rc = osGetCurrentProcessEnvVariableValue(L"CODEXL_ENABLE_THREAD_PROFILE", envValue);

    if (rc && (envValue == L"1"))
    {
        // Create and register the project settings object:
        m_psProjectSettingsExtension = new tpProjectSettingsExtension;

        afProjectManager::instance().registerProjectSettingsExtension(m_psProjectSettingsExtension);

        // Create the main menu actions creator:
        tpMenuActionsExecutor* pActionsCreator = new tpMenuActionsExecutor;

        // Register the actions creator:
        afQtCreatorsManager::instance().registerActionExecutor(pActionsCreator);

        // Register the tree handler:
        ProfileApplicationTreeHandler::instance()->registerSessionTypeTreeHandler(CP_STR_ThreadProfileFileExtensionCaption, &tpTreeHandler::Instance());

        tpAppController::Instance();
        tpMDIViewCreator::Instance();

        // Register the views creator:
        afQtCreatorsManager::instance().registerViewCreator(&tpMDIViewCreator::Instance());
    }

#endif
}

SessionViewCreator* AmdtCpuProfiling::sessionViewCreator()
{
    return m_pSessionViewCreator;
}

CpuProjectSettings* AmdtCpuProfiling::cpuProjectSettings()
{
    return m_pProjectSettingsExtension;
}

void AmdtCpuProfiling::initializeIndependentWidgets()
{
}

void AmdtCpuProfiling::resetGuiLayout()
{
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();

    GT_IF_WITH_ASSERT(pMainWindow != nullptr)
    {
        pMainWindow->resetInitialLayout();
    }
}

extern "C"
{
    // 0 = no error
    // != 0 error value and error string contains the error
    int AMDT_CPU_PROF_API CheckValidity(gtString& errString)
    {
        int retVal = 0;

        errString = L"";

        return retVal;
    }
    void AMDT_CPU_PROF_API initialize()
    {
        AmdtCpuProfiling::instance().initialize();
    }
    void AMDT_CPU_PROF_API initializeIndependentWidgets()
    {
        AmdtCpuProfiling::instance().initializeIndependentWidgets();
    }

}; // extern "C"

