//==============================================================================
// Copyright (c) 2011-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools
/// \file
/// \brief   Base class for dynamic loading of .so/.dll.
//==============================================================================

#include "DynamicLibraryModule.h"

DynamicLibraryModule::DynamicLibraryModule(void)
    : m_Module(NULL)
{
}

DynamicLibraryModule::DynamicLibraryModule(const std::string& name)
    : m_Module(NULL)
{
    LoadModule(name);
}

DynamicLibraryModule::~DynamicLibraryModule(void)
{
    UnloadModule();
}

bool DynamicLibraryModule::LoadModule(const std::string& name)
{
#ifdef _WIN32
    m_Module = LoadLibraryA(name.c_str());
#else
    m_Module = dlopen(name.c_str(), RTLD_LAZY);
#endif
    return (m_Module != NULL);
}

// start loading based on the vector of library names.  First success
// terminates the loading attempts.  Needed for CentOS6 and OpenCL.
// The Catalyst installer (at least in Catalyst 12.1) creates libOpenCL.so.1
// but no symbolic link to libOpenCL.so.  This symbolic link exists on other
// distributions where multi-step repackaging is required (build a .deb, run it)
bool DynamicLibraryModule::LoadModule(const std::vector<std::string>& names)
{
    bool    loaded = false;

    for (std::vector<std::string>::const_iterator it = names.begin(); it != names.end(); it++)
    {
        loaded = LoadModule(*it);

        if (loaded)
        {
            return loaded;
        }
    }

    return loaded;
}

void DynamicLibraryModule::UnloadModule()
{
    if (m_Module != NULL)
    {
#ifdef _WIN32
        FreeLibrary(m_Module);
#else
        dlclose(m_Module);
#endif
        m_Module = NULL;
    }
}

void* DynamicLibraryModule::GetProcAddress(const std::string& name)
{
    if (m_Module != NULL)
    {
#ifdef _WIN32
        return ::GetProcAddress(m_Module, name.c_str());
#else
        return dlsym(m_Module, name.c_str());
#endif
    }
    else
    {
        return NULL;
    }
}
