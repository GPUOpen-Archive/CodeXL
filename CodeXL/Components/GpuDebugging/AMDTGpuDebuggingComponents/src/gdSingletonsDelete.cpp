//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdSingletonsDelete.cpp
///
//==================================================================================

//------------------------------ gdSingletonsDelete.cpp ------------------------------

// Qt:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeHandler.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdImagesAndBuffersManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdPropertiesEventObserver.h>
#include <AMDTGpuDebuggingComponents/Include/gdSingletonsDelete.h>
#include <AMDTGpuDebuggingComponents/Include/gdThreadsEventObserver.h>

// The static instance of this class. When the process that contains this file exits,
// the destructor of this single instance will be called. This destructor releases all
// the existing singleton objects.
static gdSingletonsDelete instance;


// ---------------------------------------------------------------------------
// Name:        gdSingletonsDelete::~gdSingletonsDelete
// Description: Destructor - release all the existing singleton objects.
// Author:      Yaki Tebeka
// Date:        7/12/2003
// ---------------------------------------------------------------------------
gdSingletonsDelete::~gdSingletonsDelete()
{

    // Delete the gdGDebuggerGlobalVariablesManager instance:
    OS_OUTPUT_DEBUG_LOG(L"Before deleting gdGDebuggerGlobalVariablesManager", OS_DEBUG_LOG_DEBUG);
    delete gdGDebuggerGlobalVariablesManager::_pMySingleInstance;
    gdGDebuggerGlobalVariablesManager::_pMySingleInstance = nullptr;
    OS_OUTPUT_DEBUG_LOG(L"After deleting gdGDebuggerGlobalVariablesManager", OS_DEBUG_LOG_DEBUG);

    // Delete the gdApplicationCommands instance:
    OS_OUTPUT_DEBUG_LOG(L"Before deleting gdApplicationCommands", OS_DEBUG_LOG_DEBUG);
    delete gdApplicationCommands::_pMySingleInstance;
    gdApplicationCommands::_pMySingleInstance = nullptr;
    OS_OUTPUT_DEBUG_LOG(L"After deleting gdApplicationCommands", OS_DEBUG_LOG_DEBUG);

    // Delete the gdDebugApplicationTreeHandler instance:
    OS_OUTPUT_DEBUG_LOG(L"Before deleting gdDebugApplicationTreeHandler", OS_DEBUG_LOG_DEBUG);
    delete gdDebugApplicationTreeHandler::m_pMySingleInstance;
    gdDebugApplicationTreeHandler::m_pMySingleInstance = nullptr;
    OS_OUTPUT_DEBUG_LOG(L"After deleting gdDebugApplicationTreeHandler", OS_DEBUG_LOG_DEBUG);

    // Delete the gdImagesAndBuffersManager instance:
    OS_OUTPUT_DEBUG_LOG(L"Before deleting gdImagesAndBuffersManager", OS_DEBUG_LOG_DEBUG);
    delete gdImagesAndBuffersManager::_pMySingleInstance;
    gdImagesAndBuffersManager::_pMySingleInstance = nullptr;
    OS_OUTPUT_DEBUG_LOG(L"After deleting gdImagesAndBuffersManager", OS_DEBUG_LOG_DEBUG);

    // Delete the gdPropertiesEventObserver instance:
    OS_OUTPUT_DEBUG_LOG(L"Before deleting gdPropertiesEventObserver", OS_DEBUG_LOG_DEBUG);
    delete gdPropertiesEventObserver::m_spMySingleInstance;
    gdPropertiesEventObserver::m_spMySingleInstance = nullptr;
    OS_OUTPUT_DEBUG_LOG(L"After deleting gdPropertiesEventObserver", OS_DEBUG_LOG_DEBUG);

    // Delete the gdThreadsEventObserver instance:
    OS_OUTPUT_DEBUG_LOG(L"Before deleting gdThreadsEventObserver", OS_DEBUG_LOG_DEBUG);
    delete gdThreadsEventObserver::m_spMySingleInstance;
    gdThreadsEventObserver::m_spMySingleInstance = nullptr;
    OS_OUTPUT_DEBUG_LOG(L"After deleting gdThreadsEventObserver", OS_DEBUG_LOG_DEBUG);
}
