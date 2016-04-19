//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdDebuggingCommands.cpp
///
//==================================================================================

//------------------------------ gdDebuggingCommands.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdDebuggingCommands.h>

// Static member initializations
gdDebuggingCommands* gdDebuggingCommands::m_pMySingleInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        gdDebuggingCommands::instance
// Description: Returns my instance if it was already registered
// Author:      Uri Shomroni
// Date:        3/5/2012
// ---------------------------------------------------------------------------
gdDebuggingCommands* gdDebuggingCommands::instance()
{
    GT_ASSERT(NULL != m_pMySingleInstance);

    return m_pMySingleInstance;
}

void gdDebuggingCommands::registerInstance(gdDebuggingCommands* pDebuggingCommandsInstance)
{
    // Verify only one instance exists:
    GT_IF_WITH_ASSERT(NULL == m_pMySingleInstance)
    {
        m_pMySingleInstance = pDebuggingCommandsInstance;
    }
}

