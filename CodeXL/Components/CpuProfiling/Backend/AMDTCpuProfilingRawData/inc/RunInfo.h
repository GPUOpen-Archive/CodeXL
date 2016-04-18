//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file RunInfo.h
///
//==================================================================================

#ifndef _RUNINFO_H_
#define _RUNINFO_H_

#include <AMDTBaseTools/Include/gtString.h>
#include "CpuProfilingRawDataDLLBuild.h"
#include <AMDTCpuProfilingBackendUtils/Include/CpuProfileDefinitions.h>

// The record types written to the RI file
enum RiRecordType
{
    RI_REC_TARGET_PATH = 0x01,
    RI_REC_WRK_DIR = 0x02,
    RI_REC_CMD_ARG = 0x03,
    RI_REC_ENV_VAR = 0x04,
    RI_REC_PROF_TYPE = 0x05,
    RI_REC_PROF_DIR = 0x06,
    RI_REC_PROF_START_TIME = 0x07,
    RI_REC_PROF_END_TIME = 0x08,
    RI_REC_CSS_ENABLED = 0x09,
    RI_REC_CSS_UNWIND_DEPTH = 0x0A,
    RI_REC_PROFILING_CLU = 0x0B,
    RI_REC_PROFILING_IBS_OP = 0x0C,
    RI_REC_CPU_AFFINITY = 0x0D,
    RI_REC_CSS_SCOPE = 0x0E,
    RI_REC_CSS_SUPPORT_FPO = 0x0F,
    RI_REC_OS_NAME = 0x10,
    RI_REC_PROF_SCOPE = 0x11,
    RI_REC_EXECUTED_PROCESS_ID = 0x12,
    RI_REC_INVALID = 0xFF
};


// The definition of a record written into the RI file
struct RiRecord
{
    RiRecord() : type(RI_REC_INVALID), length(0UL), pValue(NULL) {}

    gtUByte type;
    gtSize_t length;
    wchar_t* pValue;
};


// This class contain the data written to the RI file
class RunInfo
{
public:
    RunInfo()
    {
        m_cssUnwindDepth = CP_CSS_DEFAULT_UNWIND_DEPTH;
        m_cssScope = CP_CSS_SCOPE_UNKNOWN;
        m_isCssSupportFpo = false;
        m_isCSSEnabled = true;
        m_isProfilingClu = false;
        m_isProfilingIbsOp = false;
        m_cpuAffinity = 0U;
        m_executedPID = 0;
    }

public:
    gtString            m_targetPath;
    gtString            m_wrkDirectory;
    gtString            m_cmdArguments;
    gtString            m_envVariables;
    gtString            m_profType;
    gtString            m_profDirectory;
    gtString            m_profStartTime;
    gtString            m_profEndTime;
    unsigned int        m_cssUnwindDepth;
    CpuProfileCssScope  m_cssScope;
    bool                m_isCssSupportFpo;
    bool                m_isCSSEnabled;
    bool                m_isProfilingClu;
    bool                m_isProfilingIbsOp;
    unsigned long long  m_cpuAffinity;
    gtString            m_osName;
    gtString            m_profScope;
    unsigned int        m_executedPID;
};


/** Writes the RI file with the provided data
    @param[in] pRIFilePath The file name to write to
    @param[in] pRIInfo The data need to be written into the file pRIFilePath
    \return The success of writing the data into the RI file
    \retval S_OK Success
    \retval E_INVALIDARG if pRIFilePath or pRunInfo are not a valid pointer
    \retVal E_FAIL if failed to create or write the file pRIFilePath
*/
CP_RAWDATA_API HRESULT fnWriteRIFile(const wchar_t* pRIFilePath, const RunInfo* pRIInfo);


/** Provides the data read from the RI file
    @param[in] pRIFilePath The file name to read from
    @param[out] pRIInfo The data read from the file pRIFilePath
    \return The success of reading the data into the RI file
    \retval S_OK Success
    \retval E_INVALIDARG if pRIFilePath or pRunInfo are not a valid pointer
    \retVal E_FAIL if failed to open the file handle
*/
CP_RAWDATA_API HRESULT fnReadRIFile(const wchar_t* pRIFilePath, RunInfo* pRIInfo);

#endif //_RUNINFO_H_
