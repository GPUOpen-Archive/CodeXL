//==============================================================================
// Copyright (c) 2011-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools
/// \file
/// \brief This class manages the dynamic loading of atidxx{32,64}.dll
//==============================================================================

#include <string>
#include "DXXModule.h"

#if _WIN64
    const char* AMDDXXModule::s_DefaultModuleName = "atidxx64.dll";
#else
    const char* AMDDXXModule::s_DefaultModuleName = "atidxx32.dll";
#endif

AMDDXXModule::AMDDXXModule(const std::string& moduleName)
{
    Initialize();
    LoadModule(moduleName);
}

AMDDXXModule::AMDDXXModule()
{

}

AMDDXXModule::~AMDDXXModule()
{
    UnloadModule();
}

void AMDDXXModule::Initialize()
{
#define X(SYM) SYM = NULL;
    AMDDXX_INTERFACE_TABLE;
#undef X

    m_ModuleLoaded = false;
}

void AMDDXXModule::UnloadModule()
{
    m_DynamicLibraryHelper.UnloadModule();
    Initialize();
}

bool
AMDDXXModule::LoadModule(
    const std::string& moduleName)
{
    if (m_ModuleLoaded)
    {
        UnloadModule();
    }

    if (m_DynamicLibraryHelper.LoadModule(moduleName))
    {

#define MAKE_STRING(s) #s
#define X(SYM) SYM = (Pfn##SYM) m_DynamicLibraryHelper.GetProcAddress(MAKE_STRING(SYM));
        AMDDXX_INTERFACE_TABLE;
#undef X
#undef MAKE_STRING

        // Decide if we got everything
#define X(SYM) && SYM != NULL
        m_ModuleLoaded = 1 AMDDXX_INTERFACE_TABLE;
#undef X

        // If we failed, clean up.
        if (!m_ModuleLoaded)
        {
            UnloadModule();
        }
    }

    return m_ModuleLoaded;
}

// This will be something like: "d3dcompiler_43.dll".
#if _WIN64
    const char* D3DCompileModule::s_DefaultModuleName = "x64\\d3dcompiler_47.dll";
#else
    const char* D3DCompileModule::s_DefaultModuleName = "x86\\d3dcompiler_47.dll";
#endif


D3DCompileModule::D3DCompileModule(const std::string& moduleName)
{
    Initialize();
    LoadModule(moduleName);
}

D3DCompileModule::~D3DCompileModule()
{
    UnloadModule();
}

void D3DCompileModule::Initialize()
{
#define X(SYM) SYM = NULL;
    D3DCOMPILE_INTERFACE_TABLE;
#undef X

    m_isModuleLoaded = false;
}

void D3DCompileModule::UnloadModule()
{
    m_DynamicLibraryHelper.UnloadModule();
    Initialize();
}

bool D3DCompileModule::LoadModule(const std::string& name /*= s_DefaultModuleName*/, int* pErrorCode /* = NULL*/)
{
    if (m_isModuleLoaded)
    {
        UnloadModule();
    }

    if (pErrorCode)
    {
        *pErrorCode = 0;
    }

    if (m_DynamicLibraryHelper.LoadModule(name))
    {

#define MAKE_STRING(s) #s
#define X(SYM) SYM = (p##SYM) m_DynamicLibraryHelper.GetProcAddress(MAKE_STRING(SYM));
        D3DCOMPILE_INTERFACE_TABLE;
#undef X
#undef MAKE_STRING

        m_isModuleLoaded = (D3DCompile != NULL)      &&
                           (D3DDisassemble != NULL)  &&
                           (D3DCompressShaders != NULL);

        // If we failed, clean up.
        if (!m_isModuleLoaded)
        {
            if (pErrorCode)
            {
                *pErrorCode = GetLastError();
            }

            UnloadModule();
        }
    }
    else
    {
        if (pErrorCode)
        {
            *pErrorCode = GetLastError();
        }
    }

    return m_isModuleLoaded;
}
