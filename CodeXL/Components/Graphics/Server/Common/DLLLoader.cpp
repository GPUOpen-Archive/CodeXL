//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Responsible for loading and unloading dlls into the currently runnning exe.
//==============================================================================

#include "DLLLoader.h"
#include "defines.h"
#include "Logger.h"

DLLLoader::DLLLoader()
{
}

DLLLoader::~DLLLoader()
{
}

unsigned int DLLLoader::Load(const char* directory, const char* filenameWithWildcards, HMODULE* pModule)
{
    ScopeLock mutex(m_DLLMutex);
    unsigned int numLoadedDlls = 0;

    char strSearch[ PS_MAX_PATH ];
    sprintf_s(strSearch, PS_MAX_PATH, "%s\\%s", directory, filenameWithWildcards);

    // Search all files that match template
    WIN32_FIND_DATA findData;
    HANDLE hHandle = FindFirstFile(strSearch, &findData);

    if (hHandle == INVALID_HANDLE_VALUE)
    {
        return numLoadedDlls;
    }

    BOOL bContinue = TRUE;

    while (bContinue)
    {
        // call load library on the file
        std::string strPathToDLL = directory;
        strPathToDLL.append(findData.cFileName);

        // make sure this dll has not already been loaded
        if (m_loadedDLLMap.find(strPathToDLL) == m_loadedDLLMap.end())
        {
            HMODULE hModule = LoadLibrary(strPathToDLL.c_str());
            *pModule = hModule;

            if (hModule != NULL)
            {
                m_loadedDLLMap[ strPathToDLL ] = hModule;
                ++numLoadedDlls;
            }
        }
        else
        {
            // log a message to let us know that we didn't reload a dll.
            // I don't expect that this will ever happen, so log will help confirm that
            Log(logWARNING, "DLL will not be loaded because it has already been loaded: %s\n", findData.cFileName);
        }

        bContinue = FindNextFile(hHandle, &findData);
    } // End while

    FindClose(hHandle);

    return numLoadedDlls;
}

void DLLLoader::Unload()
{
    ScopeLock mutex(m_DLLMutex);
    PathToHandleMapIterator iter = m_loadedDLLMap.begin();

    while (iter != m_loadedDLLMap.end())
    {
        HMODULE module = iter->second;
        FreeLibrary(module);
        ++iter;
    }

    m_loadedDLLMap.clear();
}