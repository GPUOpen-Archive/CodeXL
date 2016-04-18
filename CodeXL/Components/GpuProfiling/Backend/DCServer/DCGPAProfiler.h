//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================


#ifndef _DC_GPA_PROFILER_H_
#define _DC_GPA_PROFILER_H_

#include <d3d11.h>
#include <vector>
#include <string>
#include "DCContextManager.h"
#include "DCCommandRecorder.h"
#include "DCKernelAssembly.h"
#include "..\Common\GPAUtils.h"
#include "..\Common\Timer.h"
#include "DeviceInfoUtils.h"

/// \addtogroup DCServer
// @{

//------------------------------------------------------------------------------------
/// This class encapsulates GPUPerfAPI
//------------------------------------------------------------------------------------
class DCGPAProfiler
{
public:
    /// Constructor
    DCGPAProfiler(void);

    /// Destructor
    ~DCGPAProfiler(void);

    /// Accessor to the GPA Loader
    /// \return GPUPerfAPILoader
    GPUPerfAPILoader& GetGPALoader()
    {
        //return m_GPALoader;
        return m_GPAUtils.GetGPALoader();
    }

    /// Accessor to the Context Manager
    /// \return DCContextManager
    DCContextManager& GetContextManager()
    {
        return m_DCContextMgr;
    }

    /// Accessor to the CommandRecorder
    /// \return CommandRecorder
    CommandRecorder& GetCommandRecorder()
    {
        return m_CommandRecorder;
    }

    /// Initialize GPA with a context (command queue)
    /// \return True if succeed
    bool Open();

    /// Close the current context in GPA
    /// \return True if succeed
    bool Close();

    /// Enable GPA counters
    /// \return true if succesfull, false otherwise (counters have been enabled)
    bool EnableCounters();

    /// Output the counter result for the specified session to a csv file
    /// \param uSessionID the GPA profiling session ID
    /// \param threadGroupX Thread Group X
    /// \param threadGroupY Thread Group Y
    /// \param threadGroupZ Thread Group Z
    /// \return false if GPA is not loaded, true if successful
    bool DumpSession(gpa_uint32 uSessionID, UINT threadGroupX, UINT threadGroupY, UINT threadGroupZ);

    /// Output memory instruction profile result
    /// \param pResource Pointer to the resource
    /// \param elapsedTime time spent
    /// \param MapType D3D11_MAP map type
    /// \param MapFlags blocking or non-blocking
    void DumpMemoryStats(ID3D11Resource* pResource, float elapsedTime, D3D11_MAP MapType, UINT MapFlags);

    /// Profile a DirectCompute kernel call with the full set of public counters
    /// \param pObj Immediate context
    /// \param ThreadGroupCountX Thread group X
    /// \param ThreadGroupCountY Thread group Y
    /// \param ThreadGroupCountZ Thread group Z
    /// \param uSessionIDOut output session ID
    /// \return True if succeed
    bool FullProfile(
        ID3D11DeviceContext* pObj,
        UINT ThreadGroupCountX,
        UINT ThreadGroupCountY,
        UINT ThreadGroupCountZ,
        gpa_uint32&      uSessionIDOut
    );

    /// Accessor to whether or not GPA has been loaded
    /// \return true if GPA dll has been loaded; false otherwise
    bool Loaded()
    {
        return m_GPAUtils.Loaded();
    }

    /// unloads the currently loaded GPA dll
    void Unload()
    {
        m_GPAUtils.Unload();
    }

    /// Initialize PMC profiler
    /// \param params Parameters pass from CodeXLGpuProfiler
    /// \param strErrorOut Error message if init failed
    /// \return true if succeeded
    bool Init(const Parameters& params, std::string& strErrorOut);

    /// Set ID3D11Device
    /// \param pDevice Current device
    void SetID3D11Device(ID3D11Device* pDevice)
    {
        m_pID3D11Device = pDevice;
    }

    /// Accessor to KernelAssembly
    /// \return KernelAssembly
    KernelAssembly& GetKernelAssemblyManager();

    /// Get Outputfile
    /// \return output file string
    std::string GetOutputFile() const
    {
        return m_profilerParams.m_strOutputFile;
    }

    /// Timer accssor
    Timer* GetTimer()
    {
        return &m_timer;
    }

    /// sets the flag indicating if the ASM file should be written out
    /// \param bOutputASM flag indicating if the ASM file should be written out
    void SetOutputASM(bool bOutputASM)
    {
        m_KernelAssembly.SetOutputASM(bOutputASM);
    }

private:

    /// Map counter name from Client convention to GPUPerfAPI convention
    /// \param selectedCounters List of selected counters
    static void MapCounterNames(CounterList& selectedCounters);

    /// Map counter name from GPUPerfAPI convention to Client convention
    /// \param prefix prefix string
    /// \param[in,out] str counter name to be unmapped
    void UnmapCounterName(const std::string& prefix, std::string& str);

    /// Write header : ExecutionOrder, Thread Group, GroupWorkSize, DataTransferSize, Time, Counter1, Counter2, ...
    void DumpHeader();

    /// Disable copy constructor
    /// \param[in] obj  the input object
    DCGPAProfiler(const DCGPAProfiler& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return  a reference of the object
    DCGPAProfiler& operator= (const DCGPAProfiler& obj);

private:
    ID3D11Device*            m_pID3D11Device;      ///< ID3D11Device
    DCContextManager         m_DCContextMgr;       ///< Context Manager
    CommandRecorder          m_CommandRecorder;    ///< Command Recorder
    Parameters               m_profilerParams;     ///< CodeXLGpuProfiler parameters passed to the server
    UINT                     m_uiCurDispatchCount; ///< number of kernels that have been profiled.
    UINT                     m_uiMaxDispatchCount; ///< max number of kernels to profile.
    KernelAssembly           m_KernelAssembly;     ///< Kernel Manager
    GPAUtils                 m_GPAUtils;           ///< GPA Utility class
    bool                     m_bDoneHeadings;      ///< Make sure we only write header once
    std::vector<std::string> m_NonCounterHeader;   ///< Non counter header list
    CounterList              m_enabledCounters;    ///< Enabled counters list
    UINT                     m_uiSequenceNum;      ///< Used in execution order
    Timer                    m_timer;              ///< Timer
    std::wstring             m_wstrDeviceName;     ///< Device name
    bool                     m_deferLoadCounters;  ///< Flag to load default counters when enable is called

    /// Check if the GPUTime counter should be enabled
    ///
    /// \return True if GPUTime counter should be enabled, false if not
    bool IsGpuTimeEnabled();

    /// Disable GPU time counter
    void DisableGpuTime();

    /// Get all compute counters available on all GPUs in the system
    ///
    /// \param[out] counterList The list of counter on all GPUs on the system
    /// \return True if retrieving the counter list succeeded, false if not
    bool GetAllComputeCounters(CounterList& counterList);
};

// @}

#endif // _DC_GPA_PROFILER_H_
