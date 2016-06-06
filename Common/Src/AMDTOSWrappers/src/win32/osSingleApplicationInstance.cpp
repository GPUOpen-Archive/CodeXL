//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osSingleApplicationInstance.cpp
///
//=====================================================================

//------------------------------ osSingleApplicationInstance.cpp ------------------------------

// Win32:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Local:
#include <AMDTOSWrappers/Include/osSingleApplicationInstance.h>


// ---------------------------------------------------------------------------
// Name:        osSingleApplicationInstance::osSingleApplicationInstance
// Description: Constructor
// Arguments:
//   applicationUniqueIdentifier - A unique string that identifies the application.
//                                 All application instances should use the same string.
//                                 This string should be unique for this application.
// Author:      AMD Developer Tools Team
// Date:        12/10/2006
// Implementation notes:
//   This idea for using a mutex is taken from the MSDN Knowledge base article Q243953:
//   "How to limit 32-bit applications to one instance in Visual C++".
// ---------------------------------------------------------------------------
osSingleApplicationInstance::osSingleApplicationInstance(const gtString& applicationUniqueIdentifier)
    : _mutexHandle(NULL), _isAnotherInstanceRunning(false)
{
    // Try to create a mutex that has the application unique string name:
    _mutexHandle = ::CreateMutex(NULL, FALSE, applicationUniqueIdentifier.asCharArray());
    DWORD lastError = GetLastError();

    // If a mutex of this name already exists, it means that there is another
    // running instance of the application:
    if (lastError == ERROR_ALREADY_EXISTS)
    {
        _isAnotherInstanceRunning = true;
    }
}


// ---------------------------------------------------------------------------
// Name:        osSingleApplicationInstance::~osSingleApplicationInstance
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        12/10/2006
// ---------------------------------------------------------------------------
osSingleApplicationInstance::~osSingleApplicationInstance()
{
    if (_mutexHandle)
    {
        // Release the mutex:
        ::CloseHandle(_mutexHandle);
        _mutexHandle = NULL;
    }
}

