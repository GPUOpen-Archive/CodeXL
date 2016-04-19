//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/AMDTGpuProfilerPlugin.cpp $
/// \version $Revision: #26 $
/// \brief  This file implements the main plugin for the GPU Profiler
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/AMDTGpuProfilerPlugin.cpp#26 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

#include <qtIgnoreCompilerWarnings.h>


#include <stddef.h>
#include <new>

#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTOSWrappers/Include/osProcess.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afQtCreatorsManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/commands/afSystemInformationCommand.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/StringConstants.h>

// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerPlugin.h>
#include <AMDTGpuProfiling/gpViewsCreator.h>
#include <AMDTGpuProfiling/ProfileManager.h>
#include <AMDTGpuProfiling/CounterSelectionSettingPage.h>
#include <AMDTGpuProfiling/GeneralSettingWindow.h>
#include <AMDTGpuProfiling/OpenCLTraceSettingPage.h>
#include <AMDTGpuProfiling/ProjectSettings.h>
#include <AMDTGpuProfiling/APIColorMap.h>
#include <AMDTGpuProfiling/CLAPIFilterManager.h>
#include <AMDTGpuProfiling/gpMenuActionsExecutor.h>
#include <AMDTGpuProfiling/gpProjectSettingsExtension.h>

bool GpuProfilerPlugin::s_loadEnabled = true;

// ---------------------------------------------------------------------------
// Name:        CheckValidity
// Description: check validity of the plug in
// Return Val:  int - error type
//              0 = no error
//              != 0 error value and error string contains the error
// Author:      Gilad Yarnitzky
// Date:        1/7/2014
// ---------------------------------------------------------------------------
int CheckValidity(gtString& errString)
{
    int retVal = GpuProfilerPlugin::Instance()->CheckValidity(errString);

    GpuProfilerPlugin::s_loadEnabled = (0 == retVal);

    return retVal;
}

void initialize()
{
    GpuProfilerPlugin::Instance()->Initialize();
}


/// A static instance of the singleton deleter class. Its destructor will delete all singletons used by the AMDTGpuProfiling library
static GpuProfilerSingletonsDelete singeltonDeleter;

int GpuProfilerPlugin::CheckValidity(gtString& errString)
{
    GT_UNREFERENCED_PARAMETER(errString);

    int retVal = 0;
    // check if we are remotely connected
    gtString sshVarEnvName(L"SSH_TTY");
    gtString sshVarEnvVal;
    osGetCurrentProcessEnvVariableValue(sshVarEnvName, sshVarEnvVal);

    if (!sshVarEnvVal.isEmpty())
    {
        retVal = 1;
    }

    return retVal;
}

void GpuProfilerPlugin::Initialize()
{
    // Register the dynamic MDI views creator:
    afQtCreatorsManager::instance().registerViewCreator(gpViewsCreator::Instance());

    // Register the Profile menu items, and tree handlers
    ProfileManager::Instance()->SetupGPUProfiling();

    // Do not register Global setting page -- the option to auto delete sessions when a project closes isn't fully supported
    //  1) No reliable "project closed" notification that always gets triggered at the right time (project closing, startup project changing, VS exiting, CXL exiting)
    //  2) Extra work needed in the CPU profiler to support this.
    // afGlobalVariablesManager::instance().registerGlobalSettingsPage(GeneralSettingWindow::Instance());

    // Register Profiler setting pages
    afProjectManager::instance().registerProjectSettingsExtension(OpenCLTraceOptions::Instance());

    afProjectManager::instance().registerProjectSettingsExtension(CounterSelectionSettingWindow::Instance());

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS    // Register the objects for the DX profile, if it is enabled
    // Create and register the project settings object:
    gpProjectSettingsExtension* pProjectSettingsExtension = new gpProjectSettingsExtension;

    afProjectManager::instance().registerProjectSettingsExtension(pProjectSettingsExtension);


    // Create the main menu actions creator:
    gpMenuActionsExecutor* pActionsCreator = new gpMenuActionsExecutor;

    // Register the actions creator:
    afQtCreatorsManager::instance().registerActionExecutor(pActionsCreator);
#endif
}

GpuProfilerSingletonsDelete::~GpuProfilerSingletonsDelete()
{
    GpuProfilerPlugin::DeleteInstance();
    gpViewsCreator::DeleteInstance();
    APIColorMap::DeleteInstance();
    GeneralSettingWindow::DeleteInstance();
    GlobalSettings::DeleteInstance();
    ProfileManager::DeleteInstance();
    SessionManager::DeleteInstance();
    CLAPIDefs::DeleteInstance();
    CounterManager::DeleteInstance();
}

