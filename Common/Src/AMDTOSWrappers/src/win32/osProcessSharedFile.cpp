//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osProcessSharedFile.cpp
///
//=====================================================================

//------------------------------ osProcessSharedFile.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osProcessSharedFile.h>


// ---------------------------------------------------------------------------
// Name:        osProcessSharedFile::osProcessSharedFile
// Description: default constructor
// Author:      AMD Developer Tools Team
// Date:        20/6/2013
// ---------------------------------------------------------------------------
osProcessSharedFile::osProcessSharedFile() : m_fileHandle(NULL)
{

}

// ---------------------------------------------------------------------------
// Name:        osProcessSharedFile::~osProcessSharedFile
// Description: default destructor
// Author:      AMD Developer Tools Team
// Date:        20/6/2013
// ---------------------------------------------------------------------------
osProcessSharedFile::~osProcessSharedFile()
{
    closeFile();
}

// ---------------------------------------------------------------------------
// Name:        afWin32RedirectionManager::openFile
// Description: open the file with the needed access and create flags
// Author:      AMD Developer Tools Team
// Date:        20/6/2013
// ---------------------------------------------------------------------------
bool osProcessSharedFile::openFile(gtString& fileName, bool openForWrite, bool openForAppend)
{
    bool retVal = false;

    DWORD desiredAccess = GENERIC_READ;
    DWORD desiredCreate = OPEN_ALWAYS;

    if (openForWrite)
    {
        // Open the file according to the directive type:
        desiredAccess = openForAppend ? FILE_APPEND_DATA : GENERIC_WRITE;
        desiredCreate = openForAppend ? OPEN_ALWAYS : CREATE_ALWAYS;
    }

    SECURITY_ATTRIBUTES securityAtributes;
    securityAtributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    securityAtributes.lpSecurityDescriptor = NULL;
    securityAtributes.bInheritHandle = TRUE;
    HANDLE tempHandle = CreateFile(fileName.asCharArray(), desiredAccess , FILE_SHARE_WRITE, &securityAtributes, desiredCreate, FILE_ATTRIBUTE_NORMAL, 0);

    if (tempHandle != INVALID_HANDLE_VALUE)
    {
        retVal = true;
        m_fileHandle = tempHandle;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osProcessSharedFile::closeFile
// Description: close the file
// Return Val:  void
// Author:      AMD Developer Tools Team
// Date:        20/6/2013
// ---------------------------------------------------------------------------
void osProcessSharedFile::closeFile()
{
    if (m_fileHandle != NULL)
    {
        CloseHandle(m_fileHandle);
        m_fileHandle = NULL;
    }
}
