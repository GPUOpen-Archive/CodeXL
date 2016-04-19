//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suDebugLogInitializer.cpp
///
//==================================================================================

//------------------------------ suDebugLogInitializer.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <src/suDebugLogInitializer.h>
#include <src/suSpiesUtilitiesDLLInitializationFunctions.h>


// ---------------------------------------------------------------------------
// Name:        suDebugLogInitializer::suDebugLogInitializer
// Description: Constructor - Initializes the debug log file.
// Author:      Yaki Tebeka
// Date:        7/1/2010
// ---------------------------------------------------------------------------
suDebugLogInitializer::suDebugLogInitializer()
{
    // Initialize the debug log file:
    bool rcDebugLog = suInitializeDebugLogFile();
    GT_ASSERT(rcDebugLog);
}


// ---------------------------------------------------------------------------
// Name:        suDebugLogInitializer::~suDebugLogInitializer
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        7/1/2010
// ---------------------------------------------------------------------------
suDebugLogInitializer::~suDebugLogInitializer()
{
}

