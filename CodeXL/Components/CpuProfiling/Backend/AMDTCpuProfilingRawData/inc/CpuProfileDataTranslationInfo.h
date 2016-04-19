//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileDataTranslationInfo.h
///
//==================================================================================

#ifndef _CPUPROFILEDATATRANSLATIONINFO_H_
#define _CPUPROFILEDATATRANSLATIONINFO_H_

#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Current TBP Version
// Increased version to 14 after including
// OS name & Profile scope to RUNINFO
#define TBPVER_DEFAULT      14

// TRB version before including [RUNINFO] section
#define TBPVER_BEFORE_RI    12

// Earliest backward-compatible TBP Version
#define TBPVER_MIN          6

// Unknown TBP Version
#define TBPVER_UNKNOWN      0


#define RUNINFO_BEGIN       L"[RUNINFO]"
#define RUNINFO_END         L"[END]"
#define ENV_BEGIN           L"[ENV]"
#define ENV_END             L"[END]"
#define PROCESSDATA_BEGIN   L"[PROCESSDATA]"
#define PROCESSDATA_END     L"[END]"
#define MODDATA_BEGIN       L"[MODDATA]"
#define MODDATA_END         L"[END]"
#define SUB_BEGIN           L"[SUB]"
#define SUB_END             L"[SUB_END]"
#define JIT_BEGIN           L"[JIT_BEGIN]"
#define JIT_END             L"[JIT_END]"
#define IMD_END             L"[END]"

typedef gtUInt32 ProcessIdType;
typedef gtUInt32 ThreadIdType;

#endif //_CPUPROFILEDATATRANSLATIONINFO_H_
