//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdRegisterProcessDebuggersManagerInstance.cpp
///
//==================================================================================

//------------------------------ pdRegisterProcessDebuggersManagerInstance.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTProcessDebugger/Include/pdProcessDebuggersManager.h>
#include <src/pdRegisterProcessDebuggersManagerInstance.h>

// This static instance of this class actually does the registration job -
// It's constructor creates and registered a pdProcessDebuggersManager instance as the
// single instance of the class.
static pdRegisterProcessDebuggersManagerInstance registererInstance;

// ---------------------------------------------------------------------------
// Name:        pdRegisterProcessDebuggersManagerInstance::pdRegisterProcessDebuggersManagerInstance
// Description: Constructor: create the process debuggers manager instance and
//              registers it.
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
pdRegisterProcessDebuggersManagerInstance::pdRegisterProcessDebuggersManagerInstance()
{
    // If we somehow have an instance already created, delete it:
    if (pdProcessDebuggersManager::_pMySingleInstance != NULL)
    {
        delete pdProcessDebuggersManager::_pMySingleInstance;
        pdProcessDebuggersManager::_pMySingleInstance = NULL;
    }

    pdProcessDebuggersManager::_pMySingleInstance = new pdProcessDebuggersManager();


}

