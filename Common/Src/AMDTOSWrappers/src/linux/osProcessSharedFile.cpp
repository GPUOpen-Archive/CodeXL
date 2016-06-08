//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osProcessSharedFile.cpp
///
//=====================================================================

//------------------------------ osProcessSharedFile.cpp ------------------------------

// system:
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

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
osProcessSharedFile::osProcessSharedFile() : m_fileHandle(0)
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

    int flags = openForWrite ? O_WRONLY | O_CREAT : O_RDONLY;
    int mode = S_IRUSR | S_IWUSR;

    if (openForAppend)
    {
        flags |= O_APPEND;
    }

    m_fileHandle = open(fileName.asUTF8CharArray(), flags, mode);

    if (m_fileHandle != -1)
    {
        retVal = true;
    }
    else
    {
        m_fileHandle = 0;
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
    if (m_fileHandle != 0)
    {
        close(m_fileHandle);
        m_fileHandle = 0;
    }
}
