//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsSingletonsDelete.cpp
///
//==================================================================================

//------------------------------ gsSingletonsDelete.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <src/gsSingletonsDelete.h>
#include <src/gsCallsHistoryLogger.h>
#include <src/gsDeprecationAnalyzer.h>
#include <src/gsExtensionsManager.h>
#include <src/gsGlobalVariables.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsMemoryMonitor.h>

// ---------------------------------------------------------------------------
// Name:        gsSingletonsDelete::~gsSingletonsDelete
// Description: deleteSingeltonObjects - Delete the OpenGL32.dll singelton
//              objects.
//              Notice: Use this class only once at the process exit.
// Author:      Yaki Tebeka
// Date:        11/5/2004
// ---------------------------------------------------------------------------
void gsSingletonsDelete::deleteSingeltonObjects()
{
    // Delete the gsOpenGLMonitor single instance:
    delete gsOpenGLMonitor::_pMySingleInstance;
    gsOpenGLMonitor::_pMySingleInstance = NULL;

    // Delete the gsExtensionsManager single instance:
    delete gsExtensionsManager::_pMySingleInstance;
    gsExtensionsManager::_pMySingleInstance = NULL;

    // Delete the gsDeprecationAnalyzer single instance:
    delete gsDeprecationAnalyzer::_pMySingleInstance;
    gsDeprecationAnalyzer::_pMySingleInstance = NULL;

    // Delete the gsMemoryMonitor single instance:
    delete gsMemoryMonitor::_pMySingleInstance;
    gsMemoryMonitor::_pMySingleInstance = NULL;
}


