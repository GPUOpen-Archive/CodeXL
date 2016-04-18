//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csForcedModesManager.cpp
///
//==================================================================================

//------------------------------ csForcedModesManager.cpp ------------------------------

// Local:
#include <src/csForcedModesManager.h>


// ---------------------------------------------------------------------------
// Name:        csForcedModesManager::csForcedModesManager
// Description: Constructor.
// Author:      Sigal Algranaty
// Date:        6/5/2010
// ---------------------------------------------------------------------------
csForcedModesManager::csForcedModesManager()
{
    // Initialize force modes to false:
    for (int i = 0; i < AP_OPENCL_AMOUNT_OF_EXECUTIONS; i++)
    {
        _isModeForced[i] = true;
    }
}


// ---------------------------------------------------------------------------
// Name:        csForcedModesManager:~csForcedModesManager
// Description: Destructor.
// Author:      Sigal Algranaty
// Date:        6/5/2010
// ---------------------------------------------------------------------------
csForcedModesManager::~csForcedModesManager()
{
}

