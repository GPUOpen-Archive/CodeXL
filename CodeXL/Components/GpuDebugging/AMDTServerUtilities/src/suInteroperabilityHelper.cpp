//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suInteroperabilityHelper.cpp
///
//==================================================================================

//------------------------------ suInteroperabilityHelper.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTServerUtilities/Include/suInteroperabilityHelper.h>

// Static memebers initializations:
suInteroperabilityHelper* suInteroperabilityHelper::_pMySingleInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        suInteroperabilityHelper::instance
// Description: Returns the single instance of the suInteroperabilityHelper class.
//              Creates it the first time this function is called.
// Author:      Uri Shomroni
// Date:        9/11/2011
// ---------------------------------------------------------------------------
suInteroperabilityHelper& suInteroperabilityHelper::instance()
{
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new suInteroperabilityHelper;

    }

    return *_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        suInteroperabilityHelper::suInteroperabilityHelper
// Description: Constructor
// Author:      Uri Shomroni
// Date:        9/11/2011
// ---------------------------------------------------------------------------
suInteroperabilityHelper::suInteroperabilityHelper()
    : _nestedFunctionCount(0)
{

}

// ---------------------------------------------------------------------------
// Name:        suInteroperabilityHelper::~suInteroperabilityHelper
// Description: Destructor
// Author:      Uri Shomroni
// Date:        9/11/2011
// ---------------------------------------------------------------------------
suInteroperabilityHelper::~suInteroperabilityHelper()
{

}

// ---------------------------------------------------------------------------
// Name:        suInteroperabilityHelper::onNestedFunctionExited
// Description: Called when a function which calls or is called by other API
//              functions is exited, decrease the counter for the nested depth.
// Author:      Uri Shomroni
// Date:        9/11/2011
// ---------------------------------------------------------------------------
void suInteroperabilityHelper::onNestedFunctionExited()
{
    GT_IF_WITH_ASSERT(_nestedFunctionCount > 0)
    {
        _nestedFunctionCount--;
    }
}

