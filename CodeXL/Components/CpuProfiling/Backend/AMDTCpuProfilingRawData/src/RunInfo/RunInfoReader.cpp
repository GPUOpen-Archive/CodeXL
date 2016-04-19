//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file RunInfoReader.cpp
///
//==================================================================================

#include "RunInfoReader.h"
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

RunInfoReader::RunInfoReader()
{
}

RunInfoReader::~RunInfoReader()
{
}

HRESULT RunInfoReader::GetRunInfoData(RunInfo* pInfo)
{
    if (pInfo == NULL)
    {
        return E_INVALIDARG;
    }

    *pInfo = m_Info;

    return S_OK;
}

HRESULT RunInfoReader::ReadNextRiRecord(CrtFile& file, RiRecord* pRIRec, long& fileSize) const
{
    HRESULT hr = S_OK;

    if (NULL != pRIRec && file.isOpened())
    {
        if (file.read(pRIRec->type))
        {
            fileSize -= sizeof(pRIRec->type);

            if (file.read(pRIRec->length))
            {
                fileSize -= sizeof(pRIRec->length);

                if (static_cast<unsigned long>(fileSize) >= static_cast<unsigned long>(pRIRec->length))
                {
                    if (pRIRec->pValue != NULL)
                    {
                        delete[] pRIRec->pValue;
                        pRIRec->pValue = NULL;
                    }

                    pRIRec->pValue = new wchar_t[pRIRec->length + 1];

                    if (0 == pRIRec->length || file.read(pRIRec->pValue, sizeof(wchar_t) * pRIRec->length))
                    {
#if defined(RI_FILE_NOT_STRICT)

                        if (0 != pRIRec->length && L'\0' == pRIRec->pValue[0] && L'\0' == pRIRec->pValue[1])
                        {
                            gtUInt32* pValueUtf32 = new gtUInt32[pRIRec->length];
                            gtSize_t bytesRead = sizeof(wchar_t) * pRIRec->length;
                            gtSize_t bytesLeft = bytesRead + sizeof(gtUInt32);
                            bytesRead -= sizeof(gtUInt32);
                            memcpy(pValueUtf32, reinterpret_cast<gtByte*>(pRIRec->pValue) + sizeof(gtUInt32), bytesRead);

                            if (file.read(reinterpret_cast<gtByte*>(pValueUtf32) + bytesRead, bytesLeft))
                            {
                                for (gtSize_t i = 0; i < pRIRec->length; ++i)
                                {
                                    pRIRec->pValue[i] = static_cast<wchar_t>(pValueUtf32[i]);
                                }

                                fileSize -= static_cast<long>(bytesLeft);
                            }

                            delete [] pValueUtf32;
                        }

#endif

                        if (0 != pRIRec->length)
                        {
                            fileSize -= static_cast<long>(sizeof(wchar_t) * pRIRec->length);
                        }

                        pRIRec->pValue[pRIRec->length] = L'\0';
                    }
                    else
                    {
                        delete[] pRIRec->pValue;
                        pRIRec->pValue = NULL;
                        hr = E_INVALIDDATA;
                    }
                }
                else
                {
                    hr = E_INVALIDDATA;
                }
            }
            else
            {
                hr = E_FAIL;
            }
        }
        else
        {
            hr = S_FALSE;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

HRESULT RunInfoReader::Read(const wchar_t* pfilePath)
{
    HRESULT hr = S_OK;

    CrtFile file;

    if (file.open(pfilePath, WINDOWS_SWITCH(FMODE_TEXT("r+b"), FMODE_TEXT("r+b, ccs=UTF-8"))))
    {
        long fileSize = 0;

        file.seekCurrentPosition(CrtFile::ORIGIN_END, 0);
        file.currentPosition(fileSize);
        file.seekCurrentPosition(CrtFile::ORIGIN_BEGIN, 0);

        if (0 < fileSize)
        {
            RiRecord riRecord;

            while (S_OK == (hr = ReadNextRiRecord(file, &riRecord, fileSize)))
            {
                switch (riRecord.type)
                {
                    case RI_REC_TARGET_PATH:
                        m_Info.m_targetPath = riRecord.pValue;
                        break;

                    case RI_REC_WRK_DIR:
                        m_Info.m_wrkDirectory = riRecord.pValue;
                        break;

                    case RI_REC_CMD_ARG:
                        m_Info.m_cmdArguments = riRecord.pValue;
                        break;

                    case RI_REC_ENV_VAR:
                        m_Info.m_envVariables = riRecord.pValue;
                        break;

                    case RI_REC_PROF_TYPE:
                        m_Info.m_profType = riRecord.pValue;
                        break;

                    case RI_REC_PROF_DIR:
                        m_Info.m_profDirectory = riRecord.pValue;
                        break;

                    case RI_REC_PROF_START_TIME:
                        m_Info.m_profStartTime = riRecord.pValue;
                        break;

                    case RI_REC_PROF_END_TIME:
                        m_Info.m_profEndTime = riRecord.pValue;
                        break;

                    case RI_REC_CSS_ENABLED:
                        if (wcscmp(riRecord.pValue, L"ENABLED") == 0)
                        {
                            m_Info.m_isCSSEnabled = true;
                        }
                        else if (wcscmp(riRecord.pValue, L"DISABLED") == 0)
                        {
                            m_Info.m_isCSSEnabled = false;
                        }

                        break;

                    case RI_REC_CSS_UNWIND_DEPTH:
                    {
                        int retVal = 0;
                        unsigned int value = 0;
                        retVal = swscanf(riRecord.pValue, L"%u", &value);

                        if (retVal == 1)
                        {
                            m_Info.m_cssUnwindDepth = value;
                        }

                        break;
                    }

                    case RI_REC_CSS_SCOPE:
                    {
                        int retVal = 0;
                        int value = -1;
                        retVal = swscanf(riRecord.pValue, L"%X", &value);

                        if (retVal == 1)
                        {
                            m_Info.m_cssScope = static_cast<CpuProfileCssScope>(value);
                        }

                        break;
                    }

                    case RI_REC_CSS_SUPPORT_FPO:
                        if (wcscmp(riRecord.pValue, L"YES") == 0)
                        {
                            m_Info.m_isCssSupportFpo = true;
                        }
                        else if (wcscmp(riRecord.pValue, L"NO") == 0)
                        {
                            m_Info.m_isCssSupportFpo = false;
                        }

                        break;

                    case RI_REC_PROFILING_CLU:
                        if (wcscmp(riRecord.pValue, L"YES") == 0)
                        {
                            m_Info.m_isProfilingClu = true;
                        }
                        else if (wcscmp(riRecord.pValue, L"NO") == 0)
                        {
                            m_Info.m_isProfilingClu = false;
                        }

                        break;

                    case RI_REC_PROFILING_IBS_OP:
                        if (wcscmp(riRecord.pValue, L"YES") == 0)
                        {
                            m_Info.m_isProfilingIbsOp = true;
                        }
                        else if (wcscmp(riRecord.pValue, L"NO") == 0)
                        {
                            m_Info.m_isProfilingIbsOp = false;
                        }

                        break;

                    case RI_REC_CPU_AFFINITY:
                    {
                        int retVal = 0;
                        unsigned long long value = 0;
                        retVal = swscanf(riRecord.pValue, L"%llu", &value);

                        if (retVal == 1)
                        {
                            m_Info.m_cpuAffinity = value;
                        }

                        break;
                    }

                    case RI_REC_OS_NAME:
                        m_Info.m_osName = riRecord.pValue;
                        break;

                    case RI_REC_PROF_SCOPE:
                        m_Info.m_profScope = riRecord.pValue;
                        break;

                    case RI_REC_EXECUTED_PROCESS_ID:
                    {
                        int retVal = 0;
                        unsigned int value = 0;
                        retVal = swscanf(riRecord.pValue, L"%u", &value);

                        if (retVal == 1)
                        {
                            m_Info.m_executedPID = value;
                        }

                        break;
                    }


                    default:
                        break;
                }

                delete [] riRecord.pValue;
                riRecord.pValue = NULL;
            }

            // Make sure we do not return S_FALSE, as the caller of this function does not care about such results.
            if (SUCCEEDED(hr))
            {
                hr = S_OK;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        // close the file handle
        file.close();
    }
    else
    {
        hr = E_NOFILE;
    }




    return hr;
}
