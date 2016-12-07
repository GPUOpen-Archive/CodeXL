//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Utils.h
///
//==================================================================================

#ifndef __CPUPROFILE_CLI_UTILS_H
#define __CPUPROFILE_CLI_UTILS_H

#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtHashMap.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtString.h>

#define ALL_PROCESS_IDS    (-1)
#define ALL_THREAD_IDS     (-1)

// FLAGS
#define SAMPLE_IGNORE_SYSTEM_MODULES    0x1
#define SAMPLE_GROUP_BY_THREAD          0x2
#define SAMPLE_GROUP_BY_MODULE          0x4
#define SAMPLE_SEPARATE_BY_CORE         0x8

// Instruction and samples count map
using InstructionSamplesMap = gtHashMap<std::string, gtUInt64>;
struct ModuleImixInfo
{
    InstructionSamplesMap m_InstMap;
    //const CpuProfileModule*  m_pModule;
    gtUInt64 m_samplesCount;
};

using ModuleImixInfoList = gtVector<ModuleImixInfo>;
using ImixSummaryMap = gtHashMap<std::string, gtUInt64>;

#if AMDT_CPCLI_ENABLE_IMIX
bool GetImixInfoList(CpuProfileReader&    profileReader,
                     ProcessIdType        pid,
                     ThreadIdType         tid,
                     gtUInt64             flags,
                     gtUInt64             coreMask,
                     ModuleImixInfoList&  modImixInfoList,
                     gtUInt64&            totalSamples);

bool GetImixInfoList(CpuProfileReader&    profileReader,
                     ProcessIdType        pid,
                     ThreadIdType         tid,
                     CpuProfileModule&    module,
                     gtUInt64             flags,
                     gtUInt64             coreMask,
                     ModuleImixInfoList&  modImixInfoList,
                     gtUInt64&            totalSamples);

bool GetImixSummaryMap(CpuProfileReader&    profileReader,
                       ModuleImixInfoList&  modImixInfoList,
                       ImixSummaryMap&      imixSummaryMap,
                       gtUInt64&            totalSamples);
#endif //AMDT_CPCLI_ENABLE_IMIX

#endif //__CPUPROFILE_CLI_UTILS_H