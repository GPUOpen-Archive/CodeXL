//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief GPUPerfAPI Utilities
//==============================================================================

#ifndef _GPU_PERFAPI_COUNTER_LOADER_H_
#define _GPU_PERFAPI_COUNTER_LOADER_H_

#include "GPUPerfAPI.h"
#include "GPACounterGenerator.h"
#include "GPUPerfAPICounters.h"
#include <TSingleton.h>
#include "StringUtils.h"
#include "OSUtils.h"

typedef decltype(GPA_GetAvailableCounters)* GPA_GetAvailableCountersForDeviceProc;
typedef decltype(GPA_GetAvailableCountersByGeneration)* GPA_GetAvailableCountersByGenerationProc;

class GPUPerfAPICounterLoader: public TSingleton<GPUPerfAPICounterLoader>
{
    friend class TSingleton<GPUPerfAPICounterLoader>;
public:

    /// Loads the dll for the GPU Perf API counter
    /// \param strDLLPath GPA DLL Path
    bool LoadPerfAPICounterDll(const gtString& strDLLPath);

    /// Accessor to whether or not GPA Perf API Counter has been loaded
    /// \return true if GPA dll has been loaded; false otherwise
    bool IsLoaded();

    /// unloads the currently loaded GPA Perf API counter dll
    void UnLoadPerfAPICounterDll();

    /// Accessor to the function pointer of the GPA Perf API Counters exposed function
    /// \return if dll is loaded return function pointer to the GPA_GetAvailableCounters function otherwise null pointer
    GPA_GetAvailableCountersForDeviceProc GetGPAAvailableCountersForDeviceProc();

    /// Accessor to the function pointer of the GPA Perf API Counters exposed function
    /// \return if dll is loaded return function pointer to the GPA_GetAvailableCountersByGeneration function otherwise null pointer
    GPA_GetAvailableCountersByGenerationProc GetGPAAvailableCountersByGenerationProc();

    ~GPUPerfAPICounterLoader();
private:

    /// Constructor
    GPUPerfAPICounterLoader();

    GPA_GetAvailableCountersByGenerationProc m_pGetAvailableCountersByGen;     ///< Function to retrieve list of counters for a particular hw generation
    GPA_GetAvailableCountersForDeviceProc    m_pGetAvailableCountersForDevice; ///< Function to retrieve list of counters for a particular device
    bool                                     m_bGPAPerfAPICounterLoaded;       ///< Flag indicating whther the GPA perf api counter dll is loaded or not
    LIB_HANDLE                               m_DllModuleHandle;                ///< Handle to the GPA Perf Counter dll
};

#endif
