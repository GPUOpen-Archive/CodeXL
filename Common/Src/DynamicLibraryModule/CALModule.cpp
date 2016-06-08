//==============================================================================
// Copyright (c) 2011-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools
/// \file
/// \brief This class manages the dynamic loading of amdcalcl, amdcalrt, amdcaldd.
//==============================================================================

#include <string>
#include "CALModule.h"


#if _WIN32
    #if _WIN64
        const char* CALCLModule::s_DefaultModuleName = "aticalcl64.dll";
    #else
        const char* CALCLModule::s_DefaultModuleName = "aticalcl.dll";
    #endif
#else
    const char* CALCLModule::s_DefaultModuleName = "libaticalcl.so";
#endif


CALCLModule::CALCLModule(const std::string& moduleName)
{
    Initialize();
    LoadModule(moduleName);
}

CALCLModule::~CALCLModule()
{
    UnloadModule();
}

void CALCLModule::Initialize()
{
#define X(SYM) SYM = NULL;
    CALCL_INTERFACE_TABLE;
#undef X

    m_ModuleLoaded = false;
}

void CALCLModule::UnloadModule()
{
    m_DynamicLibraryHelper.UnloadModule();
    Initialize();
}

bool
CALCLModule::LoadModule(
    const std::string& moduleName)
{
    if (m_ModuleLoaded)
    {
        UnloadModule();
    }

    if (m_DynamicLibraryHelper.LoadModule(moduleName))
    {

#define MAKE_STRING(s) #s
#define X(SYM) SYM = (myddi::ddiif##SYM) m_DynamicLibraryHelper.GetProcAddress("calcl" MAKE_STRING(SYM));
        CALCL_INTERFACE_TABLE;
#undef X
#undef MAKE_STRING

        // Decide if we got everything
#define X(SYM) && SYM != NULL
        m_ModuleLoaded = 1 CALCL_INTERFACE_TABLE;
#undef X

        // If we failed, clean up.
        if (!m_ModuleLoaded)
        {
            UnloadModule();
        }
    }

    return m_ModuleLoaded;
}

#if _WIN32
    #if _WIN64
        const char* CALRTModule::s_DefaultModuleName = "aticalrt64.dll";
    #else
        const char* CALRTModule::s_DefaultModuleName = "aticalrt.dll";
    #endif
#else
    const char* CALRTModule::s_DefaultModuleName = "libaticalrt.so";
#endif


CALRTModule::CALRTModule(const std::string& moduleName)
{
    Initialize();
    LoadModule(moduleName);
}

CALRTModule::~CALRTModule()
{
    UnloadModule();
}

void CALRTModule::Initialize()
{
#define X(SYM) SYM = NULL;
    CALRT_INTERFACE_TABLE;
#undef X

    m_ModuleLoaded = false;
}

void CALRTModule::UnloadModule()
{
    m_DynamicLibraryHelper.UnloadModule();
    Initialize();
}

bool
CALRTModule::LoadModule(
    const std::string& moduleName)
{
    if (m_ModuleLoaded)
    {
        UnloadModule();
    }

    if (m_DynamicLibraryHelper.LoadModule(moduleName))
    {

#define MAKE_STRING(s) #s
#define X(SYM) SYM = (myddi::ddiif##SYM) m_DynamicLibraryHelper.GetProcAddress("cal" MAKE_STRING(SYM));
        CALRT_INTERFACE_TABLE;
#undef X
#undef MAKE_STRING

        // Decide if we got everything
#define X(SYM) && SYM != NULL
        m_ModuleLoaded = true CALRT_INTERFACE_TABLE;
#undef X

        // If we failed, clean up.
        if (!m_ModuleLoaded)
        {
            UnloadModule();
        }
    }

    return m_ModuleLoaded;
}

#if _WIN32
    #if _WIN64
        const char* CALDDModule::s_DefaultModuleName = "aticaldd64.dll";
    #else
        const char* CALDDModule::s_DefaultModuleName = "aticaldd.dll";
    #endif
#else
    const char* CALDDModule::s_DefaultModuleName = "libaticaldd.so";
#endif


CALDDModule::CALDDModule(const std::string& moduleName)
{
    Initialize();
    LoadModule(moduleName);
}

CALDDModule::~CALDDModule()
{
    UnloadModule();
}

void CALDDModule::Initialize()
{
#define X(SYM) SYM = NULL;
    CALDD_INTERFACE_TABLE;
#undef X

    m_ModuleLoaded = false;
}

void CALDDModule::UnloadModule()
{
    m_DynamicLibraryHelper.UnloadModule();
    Initialize();
}

bool
CALDDModule::LoadModule(
    const std::string& moduleName)
{
    if (m_ModuleLoaded)
    {
        UnloadModule();
    }

    if (m_DynamicLibraryHelper.LoadModule(moduleName))
    {

#define MAKE_STRING(s) #s
#define X(SYM) SYM = (myddi::ddiif##SYM) m_DynamicLibraryHelper.GetProcAddress("calddi" MAKE_STRING(SYM));
        CALDD_INTERFACE_TABLE;
#undef X
#undef MAKE_STRING

        // Decide if we got everything
#define X(SYM) && SYM != NULL
        m_ModuleLoaded = true CALDD_INTERFACE_TABLE;
#undef X

        // If we failed, clean up.
        if (!m_ModuleLoaded)
        {
            UnloadModule();
        }
    }

    return m_ModuleLoaded;
}
