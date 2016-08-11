//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file RunInfoWriter.cpp
///
//==================================================================================

#include "RunInfoWriter.h"
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

RunInfoWriter::RunInfoWriter()
{
}

RunInfoWriter::~RunInfoWriter()
{
}

bool RunInfoWriter::WriteNextRIRecord(CrtFile& file, RiRecord* pRIRec)
{
    bool bRet = false;

    if (NULL != pRIRec && file.isOpened())
    {
        if (file.write(pRIRec->type))
        {
            if (file.write(pRIRec->length))
            {
                if (0UL == pRIRec->length || file.write(pRIRec->pValue, sizeof(wchar_t) * pRIRec->length))
                {
                    bRet = true;
                }
            }
        }
    }

    return bRet;
}

HRESULT RunInfoWriter::Write(const wchar_t* pfilePath, const RunInfo* pRunInfo)
{
    HRESULT hr = S_OK;

    // create the file and get the file handle
    CrtFile file;

    if (!file.open(pfilePath, WINDOWS_SWITCH(FMODE_TEXT("w+b"), FMODE_TEXT("w+b, ccs=UTF-8"))))
    {
        hr = E_FAIL;
        return hr;
    }

    RiRecord riRecord;
    bool bRet;

    riRecord.type = RI_REC_TARGET_PATH;
    riRecord.length = pRunInfo->m_targetPath.length();
    riRecord.pValue = const_cast<wchar_t*>(pRunInfo->m_targetPath.asCharArray());
    bRet = WriteNextRIRecord(file, &riRecord);

    if (bRet == true)
    {
        riRecord.type = RI_REC_WRK_DIR;
        riRecord.length = pRunInfo->m_wrkDirectory.length();
        riRecord.pValue = const_cast<wchar_t*>(pRunInfo->m_wrkDirectory.asCharArray());
        bRet = WriteNextRIRecord(file, &riRecord);
    }

    if (bRet == true)
    {
        riRecord.type = RI_REC_CMD_ARG;
        riRecord.length = pRunInfo->m_cmdArguments.length();
        riRecord.pValue = const_cast<wchar_t*>(pRunInfo->m_cmdArguments.asCharArray());
        bRet = WriteNextRIRecord(file, &riRecord);
    }

    if (bRet == true)
    {
        riRecord.type = RI_REC_ENV_VAR;
        riRecord.length = pRunInfo->m_envVariables.length();
        riRecord.pValue = const_cast<wchar_t*>(pRunInfo->m_envVariables.asCharArray());
        bRet = WriteNextRIRecord(file, &riRecord);
    }

    if (bRet == true)
    {
        riRecord.type = RI_REC_PROF_TYPE;
        riRecord.length = pRunInfo->m_profType.length();
        riRecord.pValue = const_cast<wchar_t*>(pRunInfo->m_profType.asCharArray());
        bRet = WriteNextRIRecord(file, &riRecord);
    }

    if (bRet == true)
    {
        riRecord.type = RI_REC_PROF_DIR;
        riRecord.length = pRunInfo->m_profDirectory.length();
        riRecord.pValue = const_cast<wchar_t*>(pRunInfo->m_profDirectory.asCharArray());
        bRet = WriteNextRIRecord(file, &riRecord);
    }

    if (bRet == true)
    {
        riRecord.type = RI_REC_PROF_START_TIME;
        riRecord.length = pRunInfo->m_profStartTime.length();
        riRecord.pValue = const_cast<wchar_t*>(pRunInfo->m_profStartTime.asCharArray());
        bRet = WriteNextRIRecord(file, &riRecord);
    }

    if (bRet == true)
    {
        riRecord.type = RI_REC_PROF_END_TIME;
        riRecord.length = pRunInfo->m_profEndTime.length();
        riRecord.pValue = const_cast<wchar_t*>(pRunInfo->m_profEndTime.asCharArray());
        bRet = WriteNextRIRecord(file, &riRecord);
    }

    if (bRet == true)
    {
        riRecord.type = RI_REC_CSS_ENABLED;

        if (pRunInfo->m_isCSSEnabled)
        {
            riRecord.length = wcslen(L"ENABLED");
            riRecord.pValue = const_cast<wchar_t*>(L"ENABLED");
        }
        else
        {
            riRecord.length = wcslen(L"DISABLED");
            riRecord.pValue = const_cast<wchar_t*>(L"DISABLED");
        }

        bRet = WriteNextRIRecord(file, &riRecord);
    }

    if (bRet == true)
    {
        riRecord.type = RI_REC_CSS_UNWIND_DEPTH;
        wchar_t buff[50] = { 0 };
        (void) swprintf(buff, 50, L"%u", pRunInfo->m_cssUnwindDepth);
        riRecord.length = wcslen(buff);
        riRecord.pValue = buff;
        bRet = WriteNextRIRecord(file, &riRecord);
    }

    if (bRet == true && CP_CSS_SCOPE_UNKNOWN != pRunInfo->m_cssScope)
    {
        riRecord.type = RI_REC_CSS_SCOPE;
        wchar_t buff[50] = { 0 };
        (void) swprintf(buff, 50, L"0x%X", static_cast<unsigned int>(pRunInfo->m_cssScope));
        riRecord.length = wcslen(buff);
        riRecord.pValue = buff;
        bRet = WriteNextRIRecord(file, &riRecord);
    }

    if (bRet == true)
    {
        riRecord.type = RI_REC_CSS_SUPPORT_FPO;

        if (pRunInfo->m_isCssSupportFpo)
        {
            riRecord.length = wcslen(L"YES");
            riRecord.pValue = const_cast<wchar_t*>(L"YES");
        }
        else
        {
            riRecord.length = wcslen(L"NO");
            riRecord.pValue = const_cast<wchar_t*>(L"NO");
        }

        bRet = WriteNextRIRecord(file, &riRecord);
    }

    if (bRet == true)
    {
        riRecord.type = RI_REC_PROFILING_CLU;

        if (pRunInfo->m_isProfilingClu)
        {
            riRecord.length = wcslen(L"YES");
            riRecord.pValue = const_cast<wchar_t*>(L"YES");
        }
        else
        {
            riRecord.length = wcslen(L"NO");
            riRecord.pValue = const_cast<wchar_t*>(L"NO");
        }

        bRet = WriteNextRIRecord(file, &riRecord);
    }

    if (bRet == true)
    {
        riRecord.type = RI_REC_PROFILING_IBS_OP;

        if (pRunInfo->m_isProfilingIbsOp)
        {
            riRecord.length = wcslen(L"YES");
            riRecord.pValue = const_cast<wchar_t*>(L"YES");
        }
        else
        {
            riRecord.length = wcslen(L"NO");
            riRecord.pValue = const_cast<wchar_t*>(L"NO");
        }

        bRet = WriteNextRIRecord(file, &riRecord);
    }

    if (bRet == true)
    {
        riRecord.type = RI_REC_CPU_AFFINITY;
        wchar_t buff[50] = { 0 };
        (void) swprintf(buff, 50, L"%llu", pRunInfo->m_cpuAffinity);
        riRecord.length = wcslen(buff);
        riRecord.pValue = buff;
        bRet = WriteNextRIRecord(file, &riRecord);
    }

    if (bRet == true)
    {
        riRecord.type = RI_REC_OS_NAME;
        riRecord.length = pRunInfo->m_osName.length();
        riRecord.pValue = const_cast<wchar_t*>(pRunInfo->m_osName.asCharArray());
        bRet = WriteNextRIRecord(file, &riRecord);
    }

    if (bRet == true)
    {
        riRecord.type = RI_REC_PROF_SCOPE;
        riRecord.length = pRunInfo->m_profScope.length();
        riRecord.pValue = const_cast<wchar_t*>(pRunInfo->m_profScope.asCharArray());
        bRet = WriteNextRIRecord(file, &riRecord);
    }

    if (bRet == true)
    {
        riRecord.type = RI_REC_EXECUTED_PROCESS_ID;
        wchar_t buff[50] = { 0 };
        (void)swprintf(buff, 50, L"%u", pRunInfo->m_executedPID);
        riRecord.length = wcslen(buff);
        riRecord.pValue = buff;
        bRet = WriteNextRIRecord(file, &riRecord);
    }

    if (bRet == true)
    {
        riRecord.type = RI_REC_CSS_INTERVAL;
        wchar_t buff[50] = { 0 };
        (void)swprintf(buff, 50, L"%u", pRunInfo->m_cssInterval);
        riRecord.length = wcslen(buff);
        riRecord.pValue = buff;
        bRet = WriteNextRIRecord(file, &riRecord);
    }

    if (bRet == true)
    {
        riRecord.type = RI_REC_CPU_COUNT;
        wchar_t buff[50] = { 0 };
        (void)swprintf(buff, 50, L"%u", pRunInfo->m_cpuCount);
        riRecord.length = wcslen(buff);
        riRecord.pValue = buff;
        bRet = WriteNextRIRecord(file, &riRecord);
    }

    if (bRet == true)
    {
        riRecord.type = RI_REC_CXL_VERSION;
        riRecord.length = pRunInfo->m_codexlVersion.length();
        riRecord.pValue = const_cast<wchar_t*>(pRunInfo->m_codexlVersion.asCharArray());
        bRet = WriteNextRIRecord(file, &riRecord);
    }

    if (bRet == false)
    {
        hr = E_FAIL;
    }

    // close the file handle
    file.close();

    return hr;
}
