//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaSingletonsDelete.cpp
///
//==================================================================================

//------------------------------ gaSingletonsDelete.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <src/gaAPIToSpyConnector.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <src/gaPersistentDataManager.h>
#include <src/gaSingletonsDelete.h>


// ---------------------------------------------------------------------------
// Name:        gaSingletonsDelete::~gaSingletonsDelete
// Description: Destructor - release all the existing singleton objects.
// Author:      Yaki Tebeka
// Date:        7/12/2003
// ---------------------------------------------------------------------------
gaSingletonsDelete::~gaSingletonsDelete()
{
    // Delete the gaPersistentDataManager single instance:
    gaPersistentDataManager* pBreakpointsManagerInstance = gaPersistentDataManager::_pMySingleInstance;
    delete pBreakpointsManagerInstance;
    gaPersistentDataManager::_pMySingleInstance = NULL;

    // Delete the pdProcessDebugger single instance:
    gaAPIToSpyConnector* pSpyConnectorInstance = gaAPIToSpyConnector::_pMySingleInstance;
    delete pSpyConnectorInstance;
    gaAPIToSpyConnector::_pMySingleInstance = NULL;

    // Delete the gaGRApiFunctions single instance:
    gaGRApiFunctions::deleteInstance();
}

