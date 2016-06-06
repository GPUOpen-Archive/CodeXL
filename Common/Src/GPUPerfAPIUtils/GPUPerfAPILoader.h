//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  class to load the GPA at run-time
//==============================================================================

#ifndef _GPU_PERFAPI_LOADER_H_
#define _GPU_PERFAPI_LOADER_H_

#ifdef _WIN32
    #include <windows.h>
#endif

#if defined(_LINUX) || defined(LINUX)
    #include <stdio.h>
    #include <stdlib.h>
    #include <dlfcn.h>
#endif

#include <string>
#include <assert.h>

#ifdef GDT_INTERNAL
    #include "GPUPerfAPITypes-Private.h"
    #include "GPUPerfAPIFunctionTypes-Private.h"
#else
    #include "GPUPerfAPITypes.h"
    #include "GPUPerfAPIFunctionTypes.h"
#endif

// class to load the GPA at run-time
// To call a function create an instance of GPUPerfAPILoader and use . notation to access the function as normal
class GPUPerfAPILoader
{
public:

    /// constructor
    GPUPerfAPILoader();
    // destructor
    ~GPUPerfAPILoader();

    /// Loads the GPA Dll for the specified API and initializes all the GPA function pointers
    /// \param[in] pDllPath path to load GPA Dlls from
    /// \param[in] api the API to load GPA for
    /// \param[out] ppErrorMessage the error message if loading fails
    /// \return true if the GPA Dll is successfully loaded, false otherwise
    bool Load(const char* pDllPath, GPA_API_Type api, const char** ppErrorMessage);

    /// Unload may safely be called multiple times
    void Unload();

    /// Determine if Load has been successfully called.
    /// \return true if the GPA Dll is loaded
    bool Loaded();

    /// Define all GPA public functions
#define GPA_FUNCTION_PREFIX( f )                  \
    f##PtrType f;
#include "GPAFunctions.h"
#undef GPA_FUNCTION_PREFIX

protected:
#ifdef _WIN32
    HMODULE m_hMod; ///< DLL module handle
#else
    void* pHandle;  ///< shared library handle
#endif

private:
    /// Get the GPA DLL Name for the specified api
    /// \param[in] dllPath the path to load GPA Dlls from
    /// \param[in] api the API to load GPA for
    /// \return the full path to the dll to load
    std::string GetGPADllName(const std::string& dllPath, GPA_API_Type api);
};

#endif //_GPU_PERFAPI_LOADER_H_
