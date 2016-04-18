//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdSingletonsDelete.cpp
///
//==================================================================================

//------------------------------ pdSingletonsDelete.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <src/pdSingletonsDelete.h>
#include <AMDTProcessDebugger/Include/pdProcessDebuggersManager.h>

static pdSingletonsDelete singletonDeleterInstance;

// ---------------------------------------------------------------------------
// Name:        pdSingletonsDelete::~pdSingletonsDelete
// Description: Destructor - release all the singleton objects
// Author:      Uri Shomroni
// Date:        3/6/2009
// ---------------------------------------------------------------------------
pdSingletonsDelete::~pdSingletonsDelete()
{
    OS_OUTPUT_DEBUG_LOG(L"Before deleting pdProcessDebuggersManager", OS_DEBUG_LOG_DEBUG);

    pdProcessDebuggersManager* pProcessDebuggersManagerInstance = pdProcessDebuggersManager::_pMySingleInstance;
    delete pProcessDebuggersManagerInstance;
    pdProcessDebuggersManager::_pMySingleInstance = NULL;

    OS_OUTPUT_DEBUG_LOG(L"After deleting pdProcessDebuggersManager", OS_DEBUG_LOG_DEBUG);
}

