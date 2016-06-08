//==============================================================================
// Copyright (c) 2012-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools
/// \file
/// \brief This class manages the dynamic loading of OpenCL.
//==============================================================================

#include <string>
#include <vector>
#include <iostream>
#include "OpenCLModule.h"

#if _WIN32
    const char* OpenCLModule::s_DefaultModuleName = "OpenCL.dll";
#else
    const char* OpenCLModule::s_DefaultModuleName = "libOpenCL.so";
#endif

OpenCLModule::OpenCLModule(const std::string& moduleName)
{
    Initialize();
    m_openCLVersion = LoadModule(moduleName);

#ifndef _WIN32

    // Bug 8385 - The automatic installer of Catalyst 12.1 on CentOS6
    // installs libOpenCL.so.1, but does not install a symbolic link to libOpenCL.so
    // It does not appear to be possible to dynamically construct a vector
    // inside the namespace declaration where the dispatch table is constructed
    // in the DebugAPI code, so we have to do that here, and I was not sure how to
    // modify that code accordingly
    if ((moduleName == OpenCLModule::s_DefaultModuleName)
        && (OpenCLLoaded() == OpenCL_None))
    {
        std::vector<std::string>    vMods;

        // Generic first, followed in descending order by most-recent
        // a .2 version does not yet exist
        vMods.push_back("libOpenCL.so");
        vMods.push_back("libOpenCL.so.2");
        vMods.push_back("libOpenCL.so.1");

        m_openCLVersion = LoadModule(vMods);

        // But if this fails, we have no fallback...
    }

#endif
}

OpenCLModule::OpenCLModule(const std::vector<std::string>& names)
{
    Initialize();
    LoadModule(names);
}

OpenCLModule::~OpenCLModule()
{
    UnloadModule();
}

OpenCLModule::OpenCLVersion
OpenCLModule::LoadModule(const std::string& moduleName)
{
    if (m_DynamicLibraryHelper.LoadModule(moduleName))
    {

#define MAKE_STRING(s) #s
#define X(SYM) SYM = (cl##SYM##_fn) m_DynamicLibraryHelper.GetProcAddress("cl" MAKE_STRING(SYM));
        OPENCL10_API_TABLE;
        // Even though I never expect to find this one, try to load it anyway.
        OPENCL10_API_TABLE_SPECIAL;
        D3D10_KHR_RESERVED_TABLE;
        OPENCL11_API_TABLE;
        DEVICE_FISSION_EXT_RESERVE_DTABLE;
        CREATE_EVENT_FROM_GL_SYNC_RESERVED_TABLE;
        OPENCL12_API_TABLE;
        // Skip _reservedD3DExtensions
        // Skip _reservedEGLExtensions
        OPENCL20_API_TABLE;
#undef X
#undef MAKE_STRING

    }

    return OpenCLLoaded();
}

// The vector version
OpenCLModule::OpenCLVersion
OpenCLModule::LoadModule(const std::vector<std::string>& names)
{
    OpenCLVersion loadEnum = OpenCL_None;

    for (std::vector<std::string>::const_iterator it = names.begin(); it != names.end(); it++)
    {
        loadEnum = LoadModule(*it);

        if (loadEnum != OpenCL_None)
        {
            return loadEnum;
        }
    }

    return loadEnum;
}

void
OpenCLModule::UnloadModule()
{
    m_DynamicLibraryHelper.UnloadModule();
    Initialize();
}

OpenCLModule::OpenCLVersion
OpenCLModule::OpenCLLoaded()
{
    OpenCLVersion moduleLoaded = OpenCL_None;

#define X(SYM) && SYM != NULL

    // Note how I don't check OPENCL10_API_TABLE_SPECIAL.
    if (moduleLoaded == OpenCL_None OPENCL10_API_TABLE)
    {
        moduleLoaded = OpenCL_1_0;
    }

    if (moduleLoaded == OpenCL_1_0 OPENCL11_API_TABLE)
    {
        moduleLoaded = OpenCL_1_1;
    }

    if (moduleLoaded == OpenCL_1_1 OPENCL12_API_TABLE)
    {
        moduleLoaded = OpenCL_1_2;
    }

    if (moduleLoaded == OpenCL_1_2 OPENCL20_API_TABLE)
    {
        moduleLoaded = OpenCL_2_0;
    }

#undef X

    return moduleLoaded;
}


bool OpenCLModule::LoadOpenCLExtensions(
    cl_platform_id platform)
{
    if ((GetExtensionFunctionAddressForPlatform == NULL) &&
        (GetExtensionFunctionAddress == NULL))
    {
        return false;
    }

#define MAKE_STRING(s) #s
#define X(SYM)                                                                                                 \
    if (GetExtensionFunctionAddressForPlatform != NULL) {                                                      \
        SYM = (cl##SYM##_fn)(*GetExtensionFunctionAddressForPlatform)(platform, "cl" MAKE_STRING(SYM));        \
    } else {                                                                                                   \
        SYM = (cl##SYM##_fn)(*GetExtensionFunctionAddress)("cl" MAKE_STRING(SYM));                             \
    }

    EXTENSIONS_TABLE;
#ifdef _WIN32
    WINDOWS_ONLY_EXTENSIONS_TABLE;
#endif
#undef MAKE_STRING
#undef X
    return true;
}


bool
OpenCLModule::IsExtensionSupported(
    OpenCLModule::OpenCLExt ext)
{
#define X(SYM) && SYM != NULL

    switch (ext)
    {
        case OpenCL_khr_d3d10_sharing:
#if D3D10_KHR_RESERVED_TABLE_AVAILABLE
            return true D3D10_KHR_RESERVED_TABLE;
#else
            return false;
#endif

        case OpenCL_ext_device_fission:
            return true DEVICE_FISSION_EXT_RESERVE_DTABLE;

        case OpenCL_khr_gl_event:
            return true CREATE_EVENT_FROM_GL_SYNC_RESERVED_TABLE;

        default:
            return false;
    }

#undef X
}

void
OpenCLModule::GetAsCLDispatchTable(cl_icd_dispatch_table& dispatchTable)
{
#define X(SYM) dispatchTable.SYM = SYM;
#define X2(RES, SYM) dispatchTable.RES = (void *) SYM;
    OPENCL10_API_TABLE;
    OPENCL10_API_TABLE_SPECIAL;
    D3D10_KHR_RESERVED_TABLE_2;
    OPENCL11_API_TABLE;
    DEVICE_FISSION_EXT_RESERVE_DTABLE_2;
    CREATE_EVENT_FROM_GL_SYNC_RESERVED_TABLE;
    OPENCL12_API_TABLE;
    // Skip _reservedD3DExtensions
    // Skip _reservedEGLExtensions
    OPENCL20_API_TABLE;
#undef X2
#undef X
}


void
OpenCLModule::SetFromCLDispatchTable(const cl_icd_dispatch_table& dispatchTable)
{
#define X(SYM) SYM = dispatchTable.SYM;
#define X2(RES, SYM) SYM = (cl##SYM##_fn) dispatchTable.RES;
    OPENCL10_API_TABLE;
    OPENCL10_API_TABLE_SPECIAL;
    D3D10_KHR_RESERVED_TABLE_2;
    OPENCL11_API_TABLE;
    DEVICE_FISSION_EXT_RESERVE_DTABLE_2;
    CREATE_EVENT_FROM_GL_SYNC_RESERVED_TABLE;
    OPENCL12_API_TABLE;
    // Skip _reservedD3DExtensions
    // Skip _reservedEGLExtensions
    OPENCL20_API_TABLE;
#undef X
#undef X2
}

void OpenCLModule::Initialize()
{
#define X(SYM) SYM = NULL;
    OPENCL10_API_TABLE;
    OPENCL10_API_TABLE_SPECIAL;
    D3D10_KHR_RESERVED_TABLE;
    OPENCL11_API_TABLE;
    DEVICE_FISSION_EXT_RESERVE_DTABLE;
    CREATE_EVENT_FROM_GL_SYNC_RESERVED_TABLE;
    OPENCL12_API_TABLE;
    _reservedD3DExtensions[0] = NULL;
    _reservedD3DExtensions[1] = NULL;
    _reservedD3DExtensions[2] = NULL;
    _reservedD3DExtensions[3] = NULL;
    _reservedD3DExtensions[4] = NULL;
    _reservedD3DExtensions[5] = NULL;
    _reservedD3DExtensions[6] = NULL;
    _reservedD3DExtensions[7] = NULL;
    _reservedD3DExtensions[8] = NULL;
    _reservedD3DExtensions[9] = NULL;
    _reservedEGLExtensions[0] = NULL;
    _reservedEGLExtensions[1] = NULL;
    _reservedEGLExtensions[2] = NULL;
    _reservedEGLExtensions[3] = NULL;
    OPENCL20_API_TABLE;
    EXTENSIONS_TABLE;
#ifdef _WIN32
    WINDOWS_ONLY_EXTENSIONS_TABLE;
#endif
#undef X
}

std::ostream& operator<<(std::ostream& output, const OpenCLModule& module)
{
#define MAKE_STRING(s) #s
#define X(SYM) output << MAKE_STRING( SYM ) << " = " << module.SYM << "\n";
    OPENCL10_API_TABLE;
    OPENCL10_API_TABLE_SPECIAL;
    D3D10_KHR_RESERVED_TABLE;
    OPENCL11_API_TABLE;
    DEVICE_FISSION_EXT_RESERVE_DTABLE;
    CREATE_EVENT_FROM_GL_SYNC_RESERVED_TABLE;
    OPENCL12_API_TABLE;
    // Skip _reservedD3DExtensions
    // Skip _reservedEGLExtensions
    OPENCL20_API_TABLE;
    EXTENSIONS_TABLE;
#ifdef _WIN32
    WINDOWS_ONLY_EXTENSIONS_TABLE;
#endif
#undef X
#undef MAKE_STRING

    return output;
}
