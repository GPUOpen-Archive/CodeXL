//==============================================================================
// Copyright (c) 2015-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools
/// \file
/// \brief  This class manages the dynamic loading of HSA debugger RT entry points in hsa-runtime-tools{32,64}.dll and libhsa-runtime-tools{32,64}.so
//==============================================================================

#include <string>
#include "HSADebuggerRTModule.h"

#if defined (_WIN64) || defined(__LP64__)
    #if defined _WIN32 || defined __CYGWIN__
        const char* HSADebuggerRTModule::s_defaultModuleName = "hsa-runtime-tools64.dll";
    #else // LINUX
        const char* HSADebuggerRTModule::s_defaultModuleName = "libhsa-runtime-tools64.so.1";
    #endif
#else
    #error HSA Foundation runtime does not support 32-bit builds
    #if defined _WIN32 || defined __CYGWIN__
        const char* HSADebuggerRTModule::s_defaultModuleName = "hsa-runtime-tools.dll";
    #else // LINUX
        const char* HSADebuggerRTModule::s_defaultModuleName = "libhsa-runtime-tools.so.1";
    #endif
#endif

HSADebuggerRTModule::HSADebuggerRTModule(void) : m_isModuleLoaded(false)
{
    Initialize();
    LoadModule();
}

HSADebuggerRTModule::~HSADebuggerRTModule(void)
{
    UnloadModule();
}

void HSADebuggerRTModule::Initialize()
{
#define X(SYM) SYM = NULL;
    HSA_TOOLS_CALLBACK_API_TABLE;
#undef X

    m_isModuleLoaded = false;
}

void HSADebuggerRTModule::UnloadModule()
{
    m_dynamicLibraryHelper.UnloadModule();
    Initialize();
}

bool HSADebuggerRTModule::LoadModule(const std::string& moduleName)
{
    // Load from specified module
    bool bLoaded = m_dynamicLibraryHelper.LoadModule(moduleName);

    if (!bLoaded)
    {
        // Load from deafult module
        bLoaded = m_dynamicLibraryHelper.LoadModule(s_defaultModuleName);
    }

    if (bLoaded)
    {

#define MAKE_STRING(s) "hsa_"#s
#define X(SYM) SYM = reinterpret_cast<hsa_##SYM##_t>(m_dynamicLibraryHelper.GetProcAddress(MAKE_STRING(SYM)));
        HSA_TOOLS_CALLBACK_API_TABLE;
#undef X
#undef MAKE_STRING

        // Check if we initialized all the function pointers
#define X(SYM) && SYM != NULL
        m_isModuleLoaded = true HSA_TOOLS_CALLBACK_API_TABLE;

#undef X

    }

    return m_isModuleLoaded;
}
