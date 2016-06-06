//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osProcessSharedFile.h
///
//=====================================================================

//------------------------------ osProcessSharedFile.h ------------------------------

#ifndef __OSPROCESSSHAREDFILE
#define __OSPROCESSSHAREDFILE

// Windows:
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    #define WIN32_LEAN_AND_MEAN 1
    #include <Windows.h>
#endif

// Local:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// ----------------------------------------------------------------------------------
// Class Name:          OS_API osProcessSharedFile
// General Description: File to be used when need to open a file to be shared between process
//                      in windows only
// Author:      AMD Developer Tools Team
// Creation Date:       20/6/2013
// ----------------------------------------------------------------------------------
class OS_API osProcessSharedFile
{
public:
    osProcessSharedFile();
    ~osProcessSharedFile();

    bool openFile(gtString& fileName, bool openForWrite, bool openForAppend);
    void closeFile();

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    int handle() { return m_fileHandle; }
#elif (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    HANDLE handle() { return m_fileHandle; }
#endif

private:
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    int m_fileHandle;
#elif (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    HANDLE m_fileHandle;
#endif
};
#endif //__OSPROCESSSHAREDFILE

