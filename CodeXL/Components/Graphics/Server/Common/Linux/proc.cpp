//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Process Implementation class. Handles all implementation details
///         for the external Proc class.
//==============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "proc.h"

/// On Linux, all processes are mapped to the file system and are found in the
/// /proc folder. The numbered folders are valid processes and refer to the
/// process ID. Each process folder contains a number of files containing
/// properties of the process. This class uses the 'cmdline' file, which
/// contains the command line of the process, including its full path and
/// command line arguments. The 'stat' file can also be used; this has a number
/// of different parameters and contains the process ID and name, but the
/// process name is limited to 15 characters. This could be an issue for
/// PerfStudio, since the processes GPUPerfServer32 and GPUPerfServer-Internal
/// will appear the same,

static const char* PROC_DIRECTORY = "/proc";
static const int BUFFER_SIZE = 1024;

class ProcImpl
{
private:
    /// Is the directory name passed in a valid process name
    /// Valid process names consist entirely of digits
    /// \param dirName the directory to analyse
    /// \return true if the directory is a process, false if not
    bool IsProcessDir(const char* dirName)
    {
        int count = strlen(dirName);

        for (int loop = 0; loop < count; loop++)
        {
            if (dirName[loop] < '0' && dirName[loop] > '9')
            {
                return false;
            }
        }

        return true;
    }

    /// read the cmdline file. This gives the full path and any command
    /// line arguments the program
    bool ReadCommandLine(const char* dirName)
    {
        char cmdFileName[256];

        m_FullPath[0] = '\0';
        m_ProcName = &m_FullPath[0];
        snprintf(cmdFileName, 256, "%s/cmdline", dirName);
        int fd = open(cmdFileName, O_RDONLY);

        if (fd != -1)
        {
            int r = read(fd, m_FullPath, BUFFER_SIZE);
            close(fd);
            int stringLength = strlen(m_FullPath);

            if (r != -1 && stringLength > 0)
            {
                // find the first occurrence of '/' working backwards from the end of the string
                int index = stringLength;
                char c;

                do
                {
                    index--;
                    c = m_FullPath[index];
                }
                while (index > 0 && c != '/');

                if (c == '/')
                {
                    index++;
                }

                m_ProcName = &m_FullPath[index];
                return true;
            }
        }

        return false;
    }

    DIR*             m_Dir;                    ///< Structure to hold directory information for opendir
    struct dirent*   m_Dirent;                 ///< Structure to hold the current directory entry

    int              m_ProcessId;              ///< The current process ID
    char             m_FullPath[BUFFER_SIZE];  ///< The full path of the process
    char*            m_ProcName;               ///< pointer to the full process name

public:
    /// Constructor
    ProcImpl()
        : m_Dir(NULL)
        , m_Dirent(NULL)
        , m_ProcessId(0)
    {
    }

    /// Destructor
    ~ProcImpl()
    {
    }

    /// Open a Proc object. Needs to be called before reading.
    /// \return true if opened successfully, false if error
    bool Open()
    {
        if ((m_Dir = opendir(PROC_DIRECTORY)) == NULL)
        {
            return false;
        }

        return true;
    }

    /// Read from a Proc object. Sets the data for the next process
    /// in the list
    /// \return true if the data for the next process has been set up
    /// correctly, false if there is an error or there are no more
    /// processes to retrieve.
    bool Read()
    {
        while ((m_Dirent = readdir(m_Dir)) != NULL)
        {
            char pathName[256];
            struct stat buf;
            char* dirName = m_Dirent->d_name;
            snprintf(pathName, 256, "%s/%s", PROC_DIRECTORY, dirName);

            stat(pathName, &buf);

            if (S_ISDIR(buf.st_mode) && IsProcessDir(dirName))
            {
                // read command line and if successful, store the process ID
                if (ReadCommandLine(pathName) == true)
                {
                    m_ProcessId = strtol(dirName, NULL, 10);
                    return true;
                }
            }
        }

        return false;
    }

    /// Accessor method to get the name of the current process
    /// \return full name of current process
    const char* GetProcName()
    {
        return m_ProcName;
    }

    /// Accessor method to get the ID of the current process
    /// \return ID of the current process
    int GetProcessId()
    {
        return m_ProcessId;
    }

    /// Close a Proc object.
    /// \return true if closed successfully, false if error
    bool Close()
    {
        if (closedir(m_Dir) == -1)
        {
            return false;
        }

        return true;
    }
};

///////////////////////////////////////////////////////
// Function definitions for external Proc class
Proc::Proc()
{
    m_pProcImpl = new ProcImpl;
}

Proc::~Proc()
{
    delete m_pProcImpl;
}

bool Proc::Open()
{
    return m_pProcImpl->Open();
}

bool Proc::Read()
{
    return m_pProcImpl->Read();
}

const char* Proc::GetProcName()
{
    return m_pProcImpl->GetProcName();
}

int Proc::GetProcessId()
{
    return m_pProcImpl->GetProcessId();
}

bool Proc::Close()
{
    return m_pProcImpl->Close();
}

