//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Responsible for loading and unloading dlls into the currently runnning exe.
//==============================================================================

#ifndef GPS_DLLLOADER_H_
#define GPS_DLLLOADER_H_

#include "TSingleton.h"
#include <map>
#include <string>
#include <windows.h>

#include"mymutex.h"

/// Path to handle map
typedef std::map< std::string, HMODULE > PathToHandleMap;

/// Path to handle map iterator
typedef PathToHandleMap::const_iterator PathToHandleMapIterator;

/// class to help simplify the loading and unloading of DLLs
class DLLLoader : public TSingleton<DLLLoader>
{
    /// TSingleton needs to be able to use our constructor.
    friend TSingleton<DLLLoader>;

public:

    /// destructor
    ~DLLLoader();

    /// Loads DLLs in a given directory which match a filename that may or may
    /// not contain wildcard characters.
    /// \param directory the path to find the DLLs in
    /// \param filenameWithWildcards the filename which is matched against
    /// \param pModule Output module that was loaded.
    /// existing DLLs
    /// return the number of DLLs loaded
    unsigned int Load(const char* directory, const char* filenameWithWildcards, HMODULE* pModule);

    /// Unloads the DLLs that have been loaded
    void Unload();

private:

    /// private constructor to adhere to singleton pattern
    DLLLoader();

    /// map to store the dll names and associated handles
    PathToHandleMap m_loadedDLLMap;

    mutex m_DLLMutex; ///< Mutex to serialize the loading of dlls
};

#endif //GPS_DLLLOADER_H_