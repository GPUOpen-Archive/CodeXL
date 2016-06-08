//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtSingeltonsDelete.cpp
///
//=====================================================================

//------------------------------ gtSingeltonsDelete.cpp ------------------------------

// Forward decelerations:
void gtDeleteAssertionFailureHandlersArray();

// Local:
#include <gtSingeltonsDelete.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// A static instance of the singleton deleter class. Its destructor will delete all
// this ApiClasses library singletons.
static gtSingeltonsDelete singeltonDeleter;


// ---------------------------------------------------------------------------
// Name:        gtSingeltonsDelete::~gtSingeltonsDelete
// Description: Destructor - deletes all the singleton instances.
// Author:      AMD Developer Tools Team
// Date:        20/8/2007
// ---------------------------------------------------------------------------
gtSingeltonsDelete::~gtSingeltonsDelete()
{
    // Delete the assertion failure handlers array:
    gtDeleteAssertionFailureHandlersArray();
}


