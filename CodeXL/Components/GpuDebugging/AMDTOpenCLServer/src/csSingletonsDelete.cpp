//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csSingletonsDelete.cpp
///
//==================================================================================

//------------------------------ gaSingletonsDelete.cpp ------------------------------

// Local:
#include <src/csAMDKernelDebuggingManager.h>
#include <src/csOpenCLMonitor.h>
#include <src/csSingletonsDelete.h>

// This class's instance, which will destroy all singleton objects when the module is destroyed:
static csSingletonsDelete instance;

// ---------------------------------------------------------------------------
// Name:        csSingletonsDelete::csSingletonsDelete
// Description: Constructor
// Author:      Uri Shomroni
// Date:        16/11/2010
// ---------------------------------------------------------------------------
csSingletonsDelete::csSingletonsDelete()
{

}

// ---------------------------------------------------------------------------
// Name:        gaSingletonsDelete::~gaSingletonsDelete
// Description: Destructor - release all the existing singleton objects.
// Author:      Uri Shomroni
// Date:        16/11/2010
// ---------------------------------------------------------------------------
csSingletonsDelete::~csSingletonsDelete()
{
    // Delete the csAMDKernelDebuggingManager single instance:
    delete csAMDKernelDebuggingManager::_pMySingleInstance;
    csAMDKernelDebuggingManager::_pMySingleInstance = NULL;

    // Delete the OpenCL monitor:
    delete csOpenCLMonitor::_pMySingleInstance;
    csOpenCLMonitor::_pMySingleInstance = NULL;
}

