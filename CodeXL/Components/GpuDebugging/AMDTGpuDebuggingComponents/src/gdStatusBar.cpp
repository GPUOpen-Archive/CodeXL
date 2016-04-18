//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStatusBar.cpp
///
//==================================================================================

//------------------------------ gdStatusBar.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdStatusBar.h>

// Static member initializations:
gdStatusBar* gdStatusBar::_pMySingleInstance = NULL;


// ---------------------------------------------------------------------------
// Name:        gdStatusBar::instance
// Description: Returns the single instance of this class (if exists).
// Author:      Yaki Tebeka
// Date:        20/6/2007
// ---------------------------------------------------------------------------
gdStatusBar* gdStatusBar::instance()
{
    return _pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        gdStatusBar::gdStatusBar
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        19/6/2007
// ---------------------------------------------------------------------------
gdStatusBar::gdStatusBar()
{
}


// ---------------------------------------------------------------------------
// Name:        gdStatusBar::~gdStatusBar
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        19/6/2007
// ---------------------------------------------------------------------------
gdStatusBar::~gdStatusBar()
{
}


// ---------------------------------------------------------------------------
// Name:        gdStatusBar::~gdStatusBar
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        19/6/2007
// ---------------------------------------------------------------------------
void gdStatusBar::setSingleInstance(gdStatusBar& instance)
{
    // Sanity check:
    GT_ASSERT(_pMySingleInstance == NULL);

    _pMySingleInstance = &instance;
}

