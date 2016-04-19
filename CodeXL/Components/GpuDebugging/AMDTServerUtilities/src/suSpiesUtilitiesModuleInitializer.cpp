//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suSpiesUtilitiesModuleInitializer.cpp
///
//==================================================================================

//------------------------------ suSpiesUtilitiesModuleInitializer.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <src/suSpiesUtilitiesDLLInitializationFunctions.h>
#include <src/suSpiesUtilitiesModuleInitializer.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Static members initializations:
suSpiesUtilitiesModuleInitializer* suSpiesUtilitiesModuleInitializer::_pMySingleInstance = NULL;


// ---------------------------------------------------------------------------
// Name:        suSpiesUtilitiesModuleInitializer::instance
// Description: Returns the single instance of the suSpiesUtilitiesModuleInitializer class
// Author:      Yaki Tebeka
// Date:        5/1/2010
// ---------------------------------------------------------------------------
suSpiesUtilitiesModuleInitializer& suSpiesUtilitiesModuleInitializer::instance()
{
    // If this class single instance was not already created:
    if (_pMySingleInstance == NULL)
    {
        // Create it:
        _pMySingleInstance = new suSpiesUtilitiesModuleInitializer;
        GT_ASSERT(_pMySingleInstance);
    }

    return *_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        suSpiesUtilitiesModuleInitializer::suSpiesUtilitiesModuleInitializer
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        5/1/2010
// ---------------------------------------------------------------------------
suSpiesUtilitiesModuleInitializer::suSpiesUtilitiesModuleInitializer()
{
    // Initialize the spies utilities module:
    suInitializeSpiesUtilitiesModule();
}


// ---------------------------------------------------------------------------
// Name:        suSpiesUtilitiesModuleInitializer::~suSpiesUtilitiesModuleInitializer
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        5/1/2010
// ---------------------------------------------------------------------------
suSpiesUtilitiesModuleInitializer::~suSpiesUtilitiesModuleInitializer()
{
}


