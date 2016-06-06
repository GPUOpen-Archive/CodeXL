//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apSingeltonsDelete.cpp
///
//==================================================================================

//------------------------------ apSingeltonsDelete.cpp ------------------------------

// Local:
#include <inc/apSingeltonsDelete.h>
#include <AMDTAPIClasses/Include/apOpenGLStateVariablesManager.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>

// A static instance of the singleton deleter class. Its destructor will delete all
// this ApiClasses library singletons.
static apSingeltonsDelete singeltonDeleter;


// ---------------------------------------------------------------------------
// Name:        apSingeltonsDelete::~apSingeltonsDelete
// Description: Destructor - deletes all the singleton instances.
// Author:  AMD Developer Tools Team
// Date:        24/4/2004
// ---------------------------------------------------------------------------
apSingeltonsDelete::~apSingeltonsDelete()
{
    // Delete the apMonitoredFunctionsManager single instance:
    delete apMonitoredFunctionsManager::_pMySingleInstance;
    apMonitoredFunctionsManager::_pMySingleInstance = NULL;

    // Delete the apOpenGLStateVariablesManager single instance:
    delete apOpenGLStateVariablesManager::_pMySingleInstance;
    apOpenGLStateVariablesManager::_pMySingleInstance = NULL;

    // Delete the apEventsHandler single instance:
    delete apEventsHandler::_pMySingleInstance;
    apEventsHandler::_pMySingleInstance = NULL;
}


