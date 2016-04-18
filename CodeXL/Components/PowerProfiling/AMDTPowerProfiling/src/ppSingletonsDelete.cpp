//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppSingletonsDelete.cpp
///
//==================================================================================

//------------------------------ ppSingletonsDelete.cpp ------------------------------

// Qt:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTPowerProfiling/src/ppAppController.h>
#include <AMDTPowerProfiling/src/ppSingletonsDelete.h>

// The static instance of this class. When the process that contains this file exits,
// the destructor of this single instance will be called. This destructor releases all
// the existing singleton objects.
static ppSingletonsDelete instance;


// ---------------------------------------------------------------------------
// Name:        ppSingletonsDelete::~ppSingletonsDelete
// Description: Destructor - release all the existing singleton objects.
// Author:      Yaki Tebeka
// Date:        7/12/2003
// ---------------------------------------------------------------------------
ppSingletonsDelete::~ppSingletonsDelete()
{

    // Delete the kaGlobalVariableManager instance:
    OS_OUTPUT_DEBUG_LOG(L"Before deleting ppAppController", OS_DEBUG_LOG_DEBUG);
    delete ppAppController::m_spMySingleInstance;
    ppAppController::m_spMySingleInstance = nullptr;
    OS_OUTPUT_DEBUG_LOG(L"After deleting ppAppController", OS_DEBUG_LOG_DEBUG);
}
