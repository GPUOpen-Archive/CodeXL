//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ osUnhandledExceptionHandler.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osUnhandledExceptionHandler.h>

// Static members initialization:
osUnhandledExceptionHandler* osUnhandledExceptionHandler::_pMySingleInstance = 0;

// ---------------------------------------------------------------------------
// Name:        osUnhandledExceptionHandler::osUnhandledExceptionHandler
// Description: Constructor.
// Author:      AMD Developer Tools Team
// Date:        21/4/2009
// ---------------------------------------------------------------------------
osUnhandledExceptionHandler::osUnhandledExceptionHandler()
{
}


// ---------------------------------------------------------------------------
// Name:        osUnhandledExceptionHandler::~osUnhandledExceptionHandler
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        21/4/2009
// ---------------------------------------------------------------------------
osUnhandledExceptionHandler::~osUnhandledExceptionHandler()
{
}


// ---------------------------------------------------------------------------
// Name:        osUnhandledExceptionHandler::registerSingleInstance
// Description:
//  Logs the single instance of this class and registeres it as the unhandles
//  exceptions hanlder for the calling process.
// Arguments: singleInstance - The single instance of this class to be registered.
// Author:      AMD Developer Tools Team
// Date:        22/4/2009
// ---------------------------------------------------------------------------
void osUnhandledExceptionHandler::registerSingleInstance(osUnhandledExceptionHandler& singleInstance)
{
    // Log the single instance of this class:
    _pMySingleInstance = &singleInstance;

    // Do not register the Linux version until everything is implemented
#pragma message ("TODO: register handler after implementation:")

}


// ---------------------------------------------------------------------------
// Name:        osUnhandledExceptionHandler::instance
// Description: Returns the single instance of this, which was registered by
//              registerSingleInstance.
// Author:      AMD Developer Tools Team
// Date:        22/4/2009
// ---------------------------------------------------------------------------
osUnhandledExceptionHandler* osUnhandledExceptionHandler::instance()
{
    return _pMySingleInstance;
};


