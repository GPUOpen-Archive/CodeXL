//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osSingleApplicationInstance.h
///
//=====================================================================

//------------------------------ osSingleApplicationInstance.h ------------------------------

#ifndef __OSSINGLEAPPLICATIONINSTANCE_H
#define __OSSINGLEAPPLICATIONINSTANCE_H

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>


// ----------------------------------------------------------------------------------
// Class Name:           OS_API osSingleApplicationInstance
// General Description:
//   This class enables checking if another instance of this application is already
//   running.
//
// Author:      AMD Developer Tools Team
// Creation Date:        12/10/2006
// ----------------------------------------------------------------------------------
class OS_API osSingleApplicationInstance
{
public:
    osSingleApplicationInstance(const gtString& applicationUniqueIdentifier);
    ~osSingleApplicationInstance();

    bool isAnotherInstanceRunning() const { return _isAnotherInstanceRunning; };

private:
    // Contains true iff this is the first instance of the current application:
    bool _isAnotherInstanceRunning;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // The handle of the mutex we use on Windows:
    osMutexHandle _mutexHandle;
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    // The handle of the file we use on Linux / Mac:
    int _fileDescriptor;
#endif
};


#endif //__OSSINGLEAPPLICATIONINSTANCE_H

