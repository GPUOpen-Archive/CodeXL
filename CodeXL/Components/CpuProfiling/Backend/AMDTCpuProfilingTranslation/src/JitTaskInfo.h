//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JitTaskInfo.h
/// \brief JIT task information interface.
///
//==================================================================================

#ifndef _JITTASKINFO_H_
#define _JITTASKINFO_H_

#include <JitTaskInfoMapper.h>

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
    int m_affinity;

    // module map
    ModuleMap m_tiModMap;

    // JIT data map
    int m_jnc_counter;
    int m_JitModCount;
    JitBlockInfoMap m_JitInfoMap;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // JIT CLR data map
    JitBlockInfoMap m_JitClrMap;

    gtUInt64 m_hrFreq;
    gtUInt64 m_startHr;
    gtUInt64 m_EndHr;

    BOOL m_is32on64Sys;
#endif

    BitnessMap m_bitnessMap;

#if defined(TI_MULTITHREADED)
    osCriticalSection m_TIMutexJIT;
#endif
};

#endif // _JITTASKINFO_H_
