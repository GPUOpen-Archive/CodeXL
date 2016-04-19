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
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdSingletonsDelete.h>

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
    gdGDebuggerGlobalVariablesManager::_pMySingleInstance = NULL;
    OS_OUTPUT_DEBUG_LOG(L"After deleting gdGDebuggerGlobalVariablesManager", OS_DEBUG_LOG_DEBUG);

    // Delete the gdApplicationCommands instance:
    OS_OUTPUT_DEBUG_LOG(L"Before deleting gdApplicationCommands", OS_DEBUG_LOG_DEBUG);
    delete gdApplicationCommands::_pMySingleInstance;
    gdApplicationCommands::_pMySingleInstance = NULL;
    OS_OUTPUT_DEBUG_LOG(L"After deleting gdGDebuggerGlobalVariablesManager", OS_DEBUG_LOG_DEBUG);
}
