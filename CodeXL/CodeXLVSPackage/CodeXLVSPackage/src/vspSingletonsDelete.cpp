//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspSingletonsDelete.cpp
///
//==================================================================================

//------------------------------ vspSingletonsDelete.cpp ------------------------------

#include "stdafx.h"

// Infra
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// Local:
#include <src/vscBreakpointsManager.h>
#include <src/vspDTEConnector.h>
#include <src/vspExpressionEvaluator.h>
#include <src/vspImagesAndBuffersManager.h>
#include <src/vspPackageWrapper.h>
#include <src/vspSingletonsDelete.h>
#include <src/vspSourceCodeViewer.h>
#include <src/vspWindowsManager.h>

// This class's instance, which will destroy all singleton objects when the module is destroyed:
static vspSingletonsDelete instance;


// ---------------------------------------------------------------------------
// Name:        vspSingletonsDelete::vspSingletonsDelete
// Description:
// Return Val:
// Author:      Uri Shomroni
// Date:        12/10/2010
// ---------------------------------------------------------------------------
vspSingletonsDelete::vspSingletonsDelete()
{

}

// ---------------------------------------------------------------------------
// Name:        vspSingletonsDelete::~vspSingletonsDelete
// Description: Destructor. Called when the DLL unloads and destroys all singleton
//              objects.
// Author:      Uri Shomroni
// Date:        12/10/2010
// ---------------------------------------------------------------------------
vspSingletonsDelete::~vspSingletonsDelete()
{
    // The single instance of gaGRApiFunctions should be the vspGRApiFunctions, so delete it:
    gaGRApiFunctions::deleteInstance();

    // Delete the breakpoints manager:
    delete vscBreakpointsManager::_pMySingleInstance;
    vscBreakpointsManager::_pMySingleInstance = NULL;

    // Delete the source code manager:
    delete vspSourceCodeViewer::_pMySingleInstance;
    vspSourceCodeViewer::_pMySingleInstance = NULL;


    // Pass this to the VSP's mirror object - Begin.

    //// Delete the DTE connector:
    //delete vspDTEConnector::_pMySingleInstance;
    //vspDTEConnector::_pMySingleInstance = NULL;

    // Pass this to the VSP's mirror object - End.

    // Delete the windows manager:
    delete vspWindowsManager::_pMySingleInstance;
    vspWindowsManager::_pMySingleInstance = NULL;


    //// Delete the DTE connector:

    //// Delete the package wrapper:
    //delete vspPackageWrapper::_pMySingleInstance;
    //vspPackageWrapper::_pMySingleInstance = NULL;

    // Pass this to the VSP's mirror object - End.


    // Delete the expression evaluator:
    delete vspExpressionEvaluator::_pMySingleInstance;
    vspExpressionEvaluator::_pMySingleInstance = NULL;

    // Delete the images and buffers manager:
    delete vspImagesAndBuffersManager::_pMySingleInstance;
    vspImagesAndBuffersManager::_pMySingleInstance = NULL;
}

