//==================================================================================
// Copyright (c) 2013-2017, Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JitTaskInfo.h
/// \brief JIT task information interface.
///
//==================================================================================

#pragma once

#include <tuple>
#include <inc/JitTaskInfoMapper.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <AMDTOSWrappers/Include/osCriticalSection.h>
    #define TI_MULTITHREADED
#endif

class JitTaskInfo
{
public:
    // constructor
    JitTaskInfo();

    // destructor
    ~JitTaskInfo();

    // clean up the maps and time marks
    void Cleanup();

    HRESULT GetUserModInfo(TiModuleInfo* pModInfo, TiTimeType systemTimeTick);
    HRESULT GetModuleInfo(TiModuleInfo* pModInfo);

    TiTimeType CalculateDeltaTick(gtUInt64 rawTick) const;

    //read the JIT info written by the agents
    HRESULT ReadJavaJitInformation(/* [in] */ const wchar_t* pDirectory, const wchar_t* pSessionDir);
    HRESULT WriteJavaJncFiles(/*[in]*/ const wchar_t* pDirectory);

    //read the JIT info after processing
    HRESULT ReadOldJitInfo(/* [in] */ const wchar_t* pDirectory);

    bool IsJitProcess32Bit(gtUInt64 jitProcID) const;

    void GetJavaJitBlockInfo(gtVector<std::tuple<gtUInt32, gtString, gtUInt32, gtUInt64, gtUInt64, gtUInt64>>& jitBlockInfo);
    void GetJavaJncInfo(gtVector<std::tuple<gtUInt32, gtString, gtString>>& jncInfoList);

    // clean up maps
    void CleanupMaps() { Cleanup(); }

protected:
    bool GetUserJitModInfo(TiModuleInfo* pModInfo, TiTimeType systemTimeTick, ModuleMap::value_type& item);

    // processor affinity
    int m_affinity = 0;

    // module map
    ModuleMap m_tiModMap;

    // JIT data map
    int m_jnc_counter = 0;
    int m_JitModCount = 0;
    JitBlockInfoMap m_JitInfoMap;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // JIT CLR data map
    JitBlockInfoMap m_JitClrMap;

    gtUInt64 m_hrFreq = 0;
    gtUInt64 m_startHr = 0;
    gtUInt64 m_EndHr = 0;

    BOOL m_is32on64Sys = FALSE;
#endif

    BitnessMap m_bitnessMap;

#if defined(TI_MULTITHREADED)
    osCriticalSection m_TIMutexJIT;
#endif

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    // This id is assigned each instance of Java function
    // This id is treated as function id by CaPerfTranslator
    gtUInt32 m_nextModInstanceId = 1;
#endif
};
