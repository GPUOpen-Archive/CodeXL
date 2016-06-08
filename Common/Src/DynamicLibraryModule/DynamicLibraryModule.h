//==============================================================================
// Copyright (c) 2011-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools
/// \file
/// \brief   Interface for loading/unloading shared images and retrieving symbol addresses.
//==============================================================================

#ifndef _DYNAMIC_LIBRARY_MODULE_H_
#define _DYNAMIC_LIBRARY_MODULE_H_

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

#include <string>
#include <vector>

/// Interface for loading/unloading shared images and retrieving symbol addresses.
class DynamicLibraryModule
{
public:

    /// ctor
    DynamicLibraryModule(void);

    /// ctor
    /// \param name name of .so/.dll file which ctor will load.
    DynamicLibraryModule(const std::string& name);

    /// dtor
    ~DynamicLibraryModule(void);

    /// Free/close the .dll/.so image.
    void UnloadModule();

    /// Load the module.
    /// \param   name of .so/.dll file.
    /// \returns      success.
    bool LoadModule(const std::string& name);

    /// Attempt to load the module, based upon the vector of names.
    /// Iterate over them and quit after the first success
    /// \param   name of .so/.dll file.
    /// \returns      success.
    bool LoadModule(const std::vector<std::string>& names);

    /// Look up a symbol.
    /// \param   name of symbol to lookup
    /// \returns      pointer to the symbol.
    void* GetProcAddress(const std::string& name);

    /// Do we have a shared object sucessfully loaded?
    /// \returns an image is loaded.
    bool IsLoaded() const { return m_Module != NULL; };

private:
    /// typedef for our OS connection to a shared image.
#ifdef _WIN32
    typedef HMODULE ImageHandle_t;
#else
    typedef void* ImageHandle_t;
#endif

    /// Handle/pointer to .so/.dll.
    ImageHandle_t m_Module;
};

#endif
