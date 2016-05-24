//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppReporter.cpp
///
//==================================================================================

// Project:
#include <ppReporter.h>
#include <string>

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define U_FORMAT "%lu"
    #ifndef _GLIBCXX_TR1_INTTYPES_H
        #define _GLIBCXX_TR1_INTTYPES_H 1
        #include <tr1/cinttypes>
    #endif // _GLIBCXX_TR1_INTTYPES_H
#else
    #define U_FORMAT "%llu"
#endif // Windows

#include <inttypes.h>

#define MICROSEC_IN_SEC  1000000

//
//  Reporter
//
// FIXME: Need to be removed when IPC implemented for Linux
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS // Linux
    #define PROCESS_INFO_TXT_FORMAT        "%d\t%d\t%d\t%3.2f\t\t%3.2f\t\t%-45.45s\t%s\n"
    #define PROCESS_INFO_TXT_HDR_FORMAT    "\nSNo\tPID\tSamples\tPower(Joules)\tPower(%)\tName\t\t\t\t\t\tPath\n\n"
    #define PROCESS_INFO_CSV_FORMAT        "%d,%d,%d,%3.2f,%3.2f,%s,%s\n"
    #define PROCESS_INFO_CSV_HDR_FORMAT    "\nSNo,PID,Samples,Power(Joules),Power(%),Name,Path\n\n"
#else // Windows
    #define PROCESS_INFO_TXT_FORMAT        "%d\t%d\t%d\t%3.2f\t%3.2f\t\t%3.2f\t\t%-45.45s\t%s\n"
    #define PROCESS_INFO_TXT_HDR_FORMAT    "\nSNo\tPID\tSamples\tIPC\tPower(Joules)\tPower(%)\tName\t\t\t\t\t\tPath\n\n"
    #define PROCESS_INFO_CSV_FORMAT        "%d,%d,%d,%3.2f,%3.2f,%3.2f,%s,%s\n"
    #define PROCESS_INFO_CSV_HDR_FORMAT    "\nSNo,PID,Samples,IPC,Power(Joules),Power(%),Name,Path\n\n"
    #define MODULE_INFO_TXT_FORMAT        "%d\t%d\t%d\t%d\t%3.2f\t\t%3.2f\t\t0x%-8.8llx\t\t%-1.8lld\t\t%-45.45s\t%s\n"
    #define MODULE_INFO_TXT_HDR_FORMAT    "\nSNo\tPID\tSamples\tKernel\tPower(Joules)\tPower(%)\tLoad Addr\t\tsize\t\t\tName\t\t\t\t\t\tPath\n\n"
    #define MODULE_INFO_CSV_FORMAT        "%d,%d,%d,%d,%3.2f,%3.2f,0x%llx,%lld,%s,%s\n"
    #define MODULE_INFO_CSV_HDR_FORMAT    "\nSNo,PID,Samples,Kernel,Power(Joules),Power(%),Load Addr,size,Name,Path\n\n"
#endif

void ppReporter::ReportHeader()
{
    m_dataStr.clear();
    memset(m_pDataStr, 0, m_DataStrSize);

    ConstructHeader();

    m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());

    ReportProfiledCounterDesc();

    return;
} // ReportHeader


bool ppReporter::ReportProfiledCounterDesc()
{
    // Write Counter Descriptor Header
    m_dataStr.clear();
    memset(m_pDataStr, 0, m_DataStrSize);

    if (m_profiledCounterIdVec.size() > 0)
    {
        ConstructProfiledCounterDescHdr();
    }

    m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());

    m_dataStr.clear();

    // Write Counter descriptors
    if (m_profiledCounterIdVec.size() > 0)
    {
        for (AMDTUInt32 i = 0; i < m_profiledCounterIdVec.size(); i++)
        {
            AMDTUInt32 counterId = m_profiledCounterIdVec[i];
            const AMDTPwrCounterDesc* pDesc = m_supportedCounterIdDescVec[counterId];
            gtString counterName = m_supportedCounterIdNameVec[counterId];

            ConstructProfiledCounterDesc(counterName, pDesc);

            m_dataStr.append(m_pDataStr);
        }
    }

    m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());

    return true;
} // ReportProfiledCounterDesc


// Report counter header for profile data
bool ppReporter::ReportProfiledCounterDataHdr(AMDTPwrSample*& sample)
{
    m_dataStr.clear();
    memset(m_pDataStr, 0, m_DataStrSize);

    m_dataStr.append("\n");
    m_dataStr.append(PP_REPORT_PROFILE_RECORDS_SECTION_HDR);
    m_dataStr.append("\n");

    ConstructProfiledCounterDataHdr(sample);

    m_dataStr.append("\n");

    m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());

    m_isCounterDataHdrPrinted = true;
    return m_isCounterDataHdrPrinted;
} // ReportProfiledCounterDataHdr

bool ppReporter::ReportProfiledCounterData(AMDTUInt32& nbrSamples, AMDTPwrSample*& sample)
{
    if (!m_isCounterDataHdrPrinted && nbrSamples)
    {
        ReportProfiledCounterDataHdr(sample);
    }

    ConstructProfiledCounterData(nbrSamples, sample);

    m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());

    return true;
} // ReportProfiledCounterData

AMDTUInt64 ppReporter::ConvertTimeStampToStr(AMDTPwrSystemTime& sampleTime, AMDTUInt64 elapsedMs, char*& pTimeStr)
{

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    ULARGE_INTEGER time;

    // Convert sample time to 100-nanosec
    time.QuadPart = (sampleTime.m_second * PP_WINDOWS_TICK_PER_SEC) + (sampleTime.m_microSecond * 10);

    // adjust the absolute profile start TS with elapsed time (in ms)
    time.QuadPart += elapsedMs * 10000;

    // Rebase to Windows EPOC - currently the API uses OS specific EPOCH
    // time.QuadPart = time.QuadPart + (PP_SEC_TO_UNIX_EPOCH * PP_WINDOWS_TICK_PER_SEC);

    // Copy value into FILETIME
    FILETIME fileTime;
    fileTime.dwHighDateTime = (DWORD)(time.HighPart);
    fileTime.dwLowDateTime = (DWORD)(time.LowPart);

    SYSTEMTIME sysTime;

    if (FileTimeToSystemTime(&fileTime, &sysTime))
    {
        sprintf(pTimeStr, "%d:%d:%d:%03d", sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
    }

#else
    struct timeval ts;
    struct tm time;
    AMDTUInt64 tmp = 0;

    ts.tv_sec = sampleTime.m_second;
    ts.tv_usec = sampleTime.m_microSecond;

    tmp = ts.tv_usec + (elapsedMs * 1000);

    // when tmp > 1000000 usec add to seconds
    ts.tv_sec += tmp / MICROSEC_IN_SEC;
    ts.tv_usec = tmp % MICROSEC_IN_SEC;

    //printf(" start-sec %lu, start-us %lu, elapsedMs %lu, sec %lu, usec %lu \n", sampleTime.m_second, sampleTime.m_microSecond, elapsedMs, ts.tv_sec, ts.tv_usec);
    tzset();
    localtime_r(&(ts.tv_sec), &time);

    sprintf(pTimeStr, "%d:%d:%d:%03lu", time.tm_hour, time.tm_min, time.tm_sec, ts.tv_usec / (1000));
#endif

    return S_OK;
} // ConvertTimeStampToStr

void ppReporterText::ConstructHeader()
{
    m_dataStr.append(PP_REPORT_HDR);
    m_dataStr.append("\n\n");

    if (!m_args.GetLaunchApp().isEmpty())
    {
        m_dataStr.append(PP_REPORT_EXECUTION_SECTION_HDR);
        m_dataStr.append("\n");

        sprintf(m_pDataStr, "    %-30.30s %s\n", PP_REPORT_EXECUTION_TARGET_PATH, m_args.GetLaunchApp().asASCIICharArray());
        m_dataStr.append(m_pDataStr);

        sprintf(m_pDataStr, "    %-30.30s %s\n", PP_REPORT_EXECUTION_TARGET_ARGS, m_args.GetLaunchAppArgs().asASCIICharArray());
        m_dataStr.append(m_pDataStr);

        sprintf(m_pDataStr, "    %-30.30s %s\n", PP_REPORT_EXECUTION_WORK_DIR, m_args.GetWorkingDir().asASCIICharArray());
        m_dataStr.append(m_pDataStr);

        sprintf(m_pDataStr, "    %-30.30s 0x%" PRIx64 "\n", PP_REPORT_PROFILE_AFFINITY_MASK, m_args.GetCoreAffinityMask());
        m_dataStr.append(m_pDataStr);
    }

    // Profile details
    m_dataStr.append("\n");
    m_dataStr.append(PP_REPORT_PROFILE_DETAILS_SECTION_HDR);
    m_dataStr.append("\n");

    // FIXME use API
    osCpuid cpuid;
    sprintf(m_pDataStr, "    %-30.30s Family(0x%x) Model(0x%x)\n", PP_REPORT_PROFILE_CPU_DETAILS, cpuid.getFamily(), cpuid.getModel());
    m_dataStr.append(m_pDataStr);

    sprintf(m_pDataStr, "    %-30.30s 0x%" PRIx64 "\n", PP_REPORT_PROFILE_CPU_MASK, m_args.GetCoreMask());
    m_dataStr.append(m_pDataStr);

    sprintf(m_pDataStr, "    %-30.30s %u milli-seconds\n", PP_REPORT_PROFILE_SAMPLING_INTERVAL, m_args.GetSamplingInterval());
    m_dataStr.append(m_pDataStr);

    sprintf(m_pDataStr, "    %-30.30s %s\n", PP_REPORT_PROFILE_START_TIME, m_profStartTime.c_str());
    m_dataStr.append(m_pDataStr);

    sprintf(m_pDataStr, "    %-30.30s %u seconds\n", PP_REPORT_PROFILE_DURATION, m_args.GetProfileDuration());
    m_dataStr.append(m_pDataStr);

    m_dataStr.append("\n");
    return;
} // ConstructHeader


void ppReporterText::ConstructProfiledCounterDescHdr()
{
    m_dataStr.append(PP_REPORT_PROFILE_COUNTERS);
    m_dataStr.append("\n");

    //sprintf(m_pDataStr, "    %-15.15s %-15.15s %-15.15s %-15.15s %-15.15s %-15.15s %-15.15s\n",
    //    PP_REPORT_COUNTER_ID,
    //    PP_REPORT_COUNTER_NAME,
    //    PP_REPORT_COUNTER_CATEGORY,
    //    PP_REPORT_COUNTER_MIN_VALUE,
    //    PP_REPORT_COUNTER_MAX_VALUE,
    //    PP_REPORT_COUNTER_UNIT,
    //    PP_REPORT_COUNTER_DESCRIPTION);

    sprintf(m_pDataStr, "    %-15.15s %-25.25s %-15.15s %-15.15s %-15.15s\n",
            PP_REPORT_COUNTER_ID,
            PP_REPORT_COUNTER_NAME,
            PP_REPORT_COUNTER_CATEGORY,
            PP_REPORT_COUNTER_UNIT,
            PP_REPORT_COUNTER_DESCRIPTION);

    m_dataStr.append(m_pDataStr);

    return;
} // ConstructProfiledCounterDescHdr


void ppReporterText::ConstructProfiledCounterDesc(gtString& counterName, const AMDTPwrCounterDesc*& counterDesc)
{
    gtString categoryStr;
    ppCliUtils::GetCategoryString(counterDesc->m_category, categoryStr);

    gtString unitStr;
    ppCliUtils::GetUnitString(counterDesc->m_units, unitStr);

    memset(m_pDataStr, 0, m_DataStrSize);
    //sprintf(m_pDataStr, "    %6d.         %-15.15s %-15.15s %7.2f         %7.2f         %-15.15s %s\n",
    //        counterDesc->m_counterID,
    //        counterName.asASCIICharArray(),
    //        categoryStr,
    //        counterDesc->m_minValue,
    //        counterDesc->m_maxValue,
    //        unitStr,
    //        counterDesc->m_description);

    sprintf(m_pDataStr, "    %6d.         %-25.25s %-15.15s %-15.15s %s\n",
            counterDesc->m_counterID,
            counterName.asASCIICharArray(),
            categoryStr.asASCIICharArray(),
            unitStr.asASCIICharArray(),
            counterDesc->m_description);

    return;
} // ConstructProfiledCounterDesc


void ppReporterText::ConstructProfiledCounterDataHdr(AMDTPwrSample*& sample)
{
    sprintf(m_pDataStr, "%-8.8s  ", PP_REPORT_PROFILE_RECORD_ID);
    m_dataStr.append(m_pDataStr);
    sprintf(m_pDataStr, "%-15.15s ", PP_REPORT_PROFILE_RECORD_TS);
    m_dataStr.append(m_pDataStr);

    AMDTPwrSample* pSample = sample;

    for (AMDTUInt32 j = 0; j < pSample->m_numOfValues; j++)
    {
        AMDTUInt32 counterId = pSample->m_counterValues[j].m_counterID;
        gtString counterName = m_supportedCounterIdNameVec[counterId];

        sprintf(m_pDataStr, "%-17.17s ", counterName.asASCIICharArray());
        m_dataStr.append(m_pDataStr);
    }

    m_dataStr.append("\n");

    return;
} // ConstructProfiledCounterDataHdr


void ppReporterText::ConstructProfiledCounterData(AMDTUInt32& nbrSamples, AMDTPwrSample*& sample)
{
    m_dataStr.clear();

    for (AMDTUInt32 i = 0; i < nbrSamples; i++)
    {
        AMDTPwrSample* pSample = sample + i;

        memset(m_pDataStr, 0, m_DataStrSize);

        char ts[64] = { "\0" };
        char* pTimeStamp = ts;
        ConvertTimeStampToStr(pSample->m_systemTime, pSample->m_elapsedTimeMs, pTimeStamp);

        sprintf(m_pDataStr, U_FORMAT"    ", pSample->m_recordId);
        m_dataStr.append(m_pDataStr);
        sprintf(m_pDataStr, "%-15.15s ", ts);
        m_dataStr.append(m_pDataStr);

        for (AMDTUInt32 j = 0; j < pSample->m_numOfValues; j++)
        {
            switch (m_supportedCounterIdDescVec[pSample->m_counterValues[j].m_counterID]->m_units)
            {
                case AMDT_PWR_UNIT_TYPE_COUNT:
                case AMDT_PWR_UNIT_TYPE_MILLI_SECOND:
                    sprintf(m_pDataStr, "%6u            ", static_cast<AMDTUInt32>(pSample->m_counterValues[j].m_counterValue));
                    break;

                case AMDT_PWR_UNIT_TYPE_PERCENT:
                case AMDT_PWR_UNIT_TYPE_RATIO:
                case AMDT_PWR_UNIT_TYPE_JOULE:
                case AMDT_PWR_UNIT_TYPE_WATT:
                case AMDT_PWR_UNIT_TYPE_MILLI_AMPERE:
                case AMDT_PWR_UNIT_TYPE_MEGA_HERTZ:
                case AMDT_PWR_UNIT_TYPE_CENTIGRADE:
                    sprintf(m_pDataStr, "%8.3f          ", pSample->m_counterValues[j].m_counterValue);
                    break;

                case AMDT_PWR_UNIT_TYPE_VOLT:
                    sprintf(m_pDataStr, "%7.4f           ", pSample->m_counterValues[j].m_counterValue);
                    break;

                default:
                    break;
            }

            m_dataStr.append(m_pDataStr);
        }

        m_dataStr.append("\n");
    }

    return;
} // ConstructProfiledCounterData

void ppReporterText::ReportHistogramCounters(AMDTUInt32 num, AMDTPwrHistogram* pHist)
{
    m_dataStr.clear();
    m_dataStr.append("\n");
    m_dataStr.append("\nHISTOGRAMS OF COUNTERS\n");
    m_dataStr.append("\n");
    AMDTPwrHistogram* tmp = NULL;

    for (AMDTUInt32 i = 0; i < num; i++)
    {
        tmp = &pHist[i];
        sprintf(m_pDataStr, "    %-20.20s ", "COUNTER");
        m_dataStr.append(m_pDataStr);
        gtString counterName = m_supportedCounterIdNameVec[tmp->m_counterId];
        sprintf(m_pDataStr, "    %-25.25s ", counterName.asASCIICharArray());
        m_dataStr.append(m_pDataStr);
        m_dataStr.append("\n");
        sprintf(m_pDataStr, "      %-20.20s ", "HISTOGRAM");
        m_dataStr.append(m_pDataStr);
        m_dataStr.append("\n");
        sprintf(m_pDataStr, "      %4s", "low");
        m_dataStr.append(m_pDataStr);
        sprintf(m_pDataStr, "      %4s", "high");
        m_dataStr.append(m_pDataStr);
        sprintf(m_pDataStr, "      %4s", "count");
        m_dataStr.append(m_pDataStr);
        m_dataStr.append("\n");

        for (AMDTUInt32 j = 0; j < (tmp->m_numOfBins); j++)
        {
            sprintf(m_pDataStr, "      %4d", static_cast<AMDTUInt32>(tmp->m_pRange[j]));
            m_dataStr.append(m_pDataStr);
            sprintf(m_pDataStr, "      %4d", static_cast<AMDTUInt32>(tmp->m_pRange[j + 1] - 1));
            m_dataStr.append(m_pDataStr);
            sprintf(m_pDataStr, "      %4d", static_cast<AMDTUInt32>(tmp->m_pBins[j]));
            m_dataStr.append(m_pDataStr);
            m_dataStr.append("\n");
        }

        m_dataStr.append("\n");
    }

    m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());
}

void ppReporterText::ReportCumulativeCounters(AMDTUInt32 num, AMDTFloat32* pHist, AMDTUInt32* pCounterId)
{
    m_dataStr.clear();
    m_dataStr.append("\n");
    m_dataStr.append("\nCUMULATIVE COUNTERS\n");
    sprintf(m_pDataStr, "    %-25.25s ", "COUNTER");
    m_dataStr.append(m_pDataStr);
    sprintf(m_pDataStr, "%-17.17s ", "CUMULATIVE VALUE");
    m_dataStr.append(m_pDataStr);
    m_dataStr.append("\n");

    for (AMDTUInt32 i = 0; i < num; i++)
    {
        gtString counterName = m_supportedCounterIdNameVec[pCounterId[i]];
        sprintf(m_pDataStr, "    %-25.25s ", counterName.asASCIICharArray());
        m_dataStr.append(m_pDataStr);
        sprintf(m_pDataStr, "  %8.2f         ", pHist[i]);
        m_dataStr.append(m_pDataStr);
        m_dataStr.append("\n");
    }

    m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());
}

void ppReporterText::WriteProcessData(AMDTUInt32 recCnt, AMDTPwrProcessInfo*& pInfo)
{
    AMDTUInt32 cnt = 0;
    AMDTFloat32 totalPower = 0;
    AMDTUInt32 totalRecords = 0;
    AMDTPwrProcessInfo* recInfo = nullptr;
    m_dataStr.clear();
    m_dataStr.append("\n");
    m_dataStr.append("PROCESS PROFILING DATA");
    m_dataStr.append(PROCESS_INFO_TXT_HDR_FORMAT);
    m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());

    for (cnt = 0; cnt < recCnt; cnt++)
    {
        recInfo = &pInfo[cnt];

        if (nullptr != recInfo)
        {
            totalRecords += recInfo->m_sampleCnt;
            totalPower += recInfo->m_power;
        }
    }

    for (cnt = 0; cnt < recCnt; cnt++)
    {
        recInfo = &pInfo[cnt];

        if (nullptr != recInfo)
        {
            m_dataStr.clear();

            sprintf(m_pDataStr, PROCESS_INFO_TXT_FORMAT,
                    cnt,
                    recInfo->m_pid,
                    recInfo->m_sampleCnt,
#if AMDT_BUILD_TARGET != AMDT_LINUX_OS // Windows
                    recInfo->m_ipc,
#endif
                    recInfo->m_power,
                    (recInfo->m_power * 100) / totalPower,
                    recInfo->m_name,
                    recInfo->m_path);

            m_dataStr.append(m_pDataStr);
            m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());

        }
    }

    m_dataStr.clear();
    sprintf(m_pDataStr,
            "\nProfile Sesssion Power Consumption:\t%3.2f\nTotal PID record collected %d",
            totalPower,
            totalRecords);

    m_dataStr.append(m_pDataStr);
    m_dataStr.append("\n");
    m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());
}

void ppReporterText::WriteModuleData(AMDTUInt32 recCnt, AMDTPwrModuleData*& pInfo, AMDTFloat32 totalPower)
{
    AMDTUInt32 cnt = 0;
    AMDTPwrModuleData* recInfo = nullptr;
    m_dataStr.clear();
    m_dataStr.append("\n");
    m_dataStr.append("MODULE PROFILING DATA");
    m_dataStr.append(MODULE_INFO_TXT_HDR_FORMAT);
    m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());

    for (cnt = 0; cnt < recCnt; cnt++)
    {
        recInfo = &pInfo[cnt];

        if (nullptr != recInfo)
        {
            m_dataStr.clear();
            sprintf(m_pDataStr,  MODULE_INFO_TXT_FORMAT,
                    cnt,
                    recInfo->m_processId,
                    recInfo->m_sampleCnt,
                    recInfo->m_isKernel,
                    recInfo->m_power,
                    (recInfo->m_power * 100) / totalPower,
                    recInfo->m_loadAddr,
                    recInfo->m_size,
                    recInfo->m_name,
                    recInfo->m_path);

            m_dataStr.append(m_pDataStr);
            m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());

        }
    }

    m_dataStr.clear();
    sprintf(m_pDataStr,
            "\nProfile Sesssion Power Consumption:\t%3.2f\nTotal modules collected %d",
            totalPower,
            recCnt);

    m_dataStr.append(m_pDataStr);
    m_dataStr.append("\n");
    m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());
}

void ppReporterCsv::ConstructHeader()
{
    m_dataStr.append(PP_REPORT_HDR);
    m_dataStr.append("\n\n");

    if (!m_args.GetLaunchApp().isEmpty())
    {
        m_dataStr.append(PP_REPORT_EXECUTION_SECTION_HDR);
        m_dataStr.append("\n");

        sprintf(m_pDataStr, "%s,%s\n", PP_REPORT_EXECUTION_TARGET_PATH, m_args.GetLaunchApp().asASCIICharArray());
        m_dataStr.append(m_pDataStr);

        sprintf(m_pDataStr, "%s,%s\n", PP_REPORT_EXECUTION_TARGET_ARGS, m_args.GetLaunchAppArgs().asASCIICharArray());
        m_dataStr.append(m_pDataStr);

        sprintf(m_pDataStr, "%s,%s\n", PP_REPORT_EXECUTION_WORK_DIR, m_args.GetWorkingDir().asASCIICharArray());
        m_dataStr.append(m_pDataStr);

        sprintf(m_pDataStr, "%s,0x%" PRIx64 "\n", PP_REPORT_PROFILE_AFFINITY_MASK, m_args.GetCoreAffinityMask());
        m_dataStr.append(m_pDataStr);
    }

    // Profile details
    m_dataStr.append("\n");
    m_dataStr.append(PP_REPORT_PROFILE_DETAILS_SECTION_HDR);
    m_dataStr.append("\n");

    // FIXME use API
    osCpuid cpuid;
    sprintf(m_pDataStr, "%s,Family(%xh) Model(%xh)\n", PP_REPORT_PROFILE_CPU_DETAILS, cpuid.getFamily(), cpuid.getModel());
    m_dataStr.append(m_pDataStr);

    sprintf(m_pDataStr, "%s,0x%" PRIx64 "\n", PP_REPORT_PROFILE_CPU_MASK, m_args.GetCoreMask());
    m_dataStr.append(m_pDataStr);

    sprintf(m_pDataStr, "%s,%u milli-seconds\n", PP_REPORT_PROFILE_SAMPLING_INTERVAL, m_args.GetSamplingInterval());
    m_dataStr.append(m_pDataStr);

    sprintf(m_pDataStr, "%s,%s\n", PP_REPORT_PROFILE_START_TIME, m_profStartTime.c_str());
    m_dataStr.append(m_pDataStr);

    sprintf(m_pDataStr, "%s,%u seconds\n", PP_REPORT_PROFILE_DURATION, m_args.GetProfileDuration());
    m_dataStr.append(m_pDataStr);

    m_dataStr.append("\n");
    return;
} // ConstructHeader


void ppReporterCsv::ConstructProfiledCounterDescHdr()
{
    m_dataStr.append(PP_REPORT_PROFILE_COUNTERS);
    m_dataStr.append("\n");

    sprintf(m_pDataStr, "%s,%s,%s,%s,%s\n",
            PP_REPORT_COUNTER_ID,
            PP_REPORT_COUNTER_NAME,
            PP_REPORT_COUNTER_CATEGORY,
            PP_REPORT_COUNTER_UNIT,
            PP_REPORT_COUNTER_DESCRIPTION);

    m_dataStr.append(m_pDataStr);

    return;
} // ConstructCounterDescHdr


void ppReporterCsv::ConstructProfiledCounterDesc(gtString& counterName, const AMDTPwrCounterDesc*& counterDesc)
{
    gtString categoryStr;
    ppCliUtils::GetCategoryString(counterDesc->m_category, categoryStr);

    gtString unitStr;
    ppCliUtils::GetUnitString(counterDesc->m_units, unitStr);

    memset(m_pDataStr, 0, m_DataStrSize);
    sprintf(m_pDataStr, "%d.,%s,%s,%s,%s\n",
            counterDesc->m_counterID,
            counterName.asASCIICharArray(),
            categoryStr.asASCIICharArray(),
            unitStr.asASCIICharArray(),
            counterDesc->m_description);

    return;
} // ConstructProfiledCounterDescHdr


void ppReporterCsv::ConstructProfiledCounterDataHdr(AMDTPwrSample*& sample)
{
    sprintf(m_pDataStr, "%s", PP_REPORT_PROFILE_RECORD_ID);
    m_dataStr.append(m_pDataStr);
    sprintf(m_pDataStr, ",%s", PP_REPORT_PROFILE_RECORD_TS);
    m_dataStr.append(m_pDataStr);

    AMDTPwrSample* pSample = sample;

    for (AMDTUInt32 j = 0; j < pSample->m_numOfValues; j++)
    {
        AMDTUInt32 counterId = pSample->m_counterValues[j].m_counterID;
        gtString counterName = m_supportedCounterIdNameVec[counterId];

        sprintf(m_pDataStr, ",%s", counterName.asASCIICharArray());
        m_dataStr.append(m_pDataStr);
    }

    m_dataStr.append("\n");

    return;
} // ConstructProfiledCounterDataHdr


void ppReporterCsv::ConstructProfiledCounterData(AMDTUInt32& nbrSamples, AMDTPwrSample*& sample)
{
    m_dataStr.clear();

    for (AMDTUInt32 i = 0; i < nbrSamples; i++)
    {
        AMDTPwrSample* pSample = sample + i;

        memset(m_pDataStr, 0, m_DataStrSize);

        sprintf(m_pDataStr, U_FORMAT, pSample->m_recordId);
        m_dataStr.append(m_pDataStr);

        char ts[64] = { "\0" };
        char* pTimeStamp = ts;
        ConvertTimeStampToStr(pSample->m_systemTime, pSample->m_elapsedTimeMs, pTimeStamp);

        sprintf(m_pDataStr, ",%s", ts);
        m_dataStr.append(m_pDataStr);

        for (AMDTUInt32 j = 0; j < pSample->m_numOfValues; j++)
        {
            switch (m_supportedCounterIdDescVec[pSample->m_counterValues[j].m_counterID]->m_units)
            {
                case AMDT_PWR_UNIT_TYPE_COUNT:
                case AMDT_PWR_UNIT_TYPE_MILLI_SECOND:
                    sprintf(m_pDataStr, ",%6u", static_cast<AMDTUInt32>(pSample->m_counterValues[j].m_counterValue));
                    break;

                case AMDT_PWR_UNIT_TYPE_PERCENT:
                case AMDT_PWR_UNIT_TYPE_RATIO:
                case AMDT_PWR_UNIT_TYPE_JOULE:
                case AMDT_PWR_UNIT_TYPE_WATT:
                case AMDT_PWR_UNIT_TYPE_VOLT:
                case AMDT_PWR_UNIT_TYPE_MILLI_AMPERE:
                case AMDT_PWR_UNIT_TYPE_CENTIGRADE:
                case AMDT_PWR_UNIT_TYPE_MEGA_HERTZ:
                    sprintf(m_pDataStr, ",%8.3f", pSample->m_counterValues[j].m_counterValue);
                    break;

                default:
                    break;
            }

            m_dataStr.append(m_pDataStr);
        }

        m_dataStr.append("\n");
    }

    return;
} // ConstructProfiledCounterData

void ppReporterCsv::ReportHistogramCounters(AMDTUInt32 num, AMDTPwrHistogram* pHist)
{
    m_dataStr.clear();
    m_dataStr.append("\n");
    m_dataStr.append("\nHISTOGRAMS OF COUNTERS\n");
    m_dataStr.append("\n");
    AMDTPwrHistogram* tmp = NULL;

    for (AMDTUInt32 i = 0; i < num; i++)
    {
        tmp = &pHist[i];
        sprintf(m_pDataStr, "%s", "COUNTER");
        m_dataStr.append(m_pDataStr);
        gtString counterName = m_supportedCounterIdNameVec[tmp->m_counterId];
        sprintf(m_pDataStr, ",%s", counterName.asASCIICharArray());
        m_dataStr.append(m_pDataStr);
        m_dataStr.append("\n");
        sprintf(m_pDataStr, "%s", "HISTOGRAM");
        m_dataStr.append(m_pDataStr);
        m_dataStr.append("\n");
        sprintf(m_pDataStr, "%s", "low");
        m_dataStr.append(m_pDataStr);
        sprintf(m_pDataStr, ",%s", "high");
        m_dataStr.append(m_pDataStr);
        sprintf(m_pDataStr, ",%s", "count");
        m_dataStr.append(m_pDataStr);
        m_dataStr.append("\n");

        for (AMDTUInt32 j = 0; j < (tmp->m_numOfBins); j++)
        {
            sprintf(m_pDataStr, "%4d", static_cast<AMDTUInt32>(tmp->m_pRange[j]));
            m_dataStr.append(m_pDataStr);
            sprintf(m_pDataStr, ",%4d", static_cast<AMDTUInt32>(tmp->m_pRange[j + 1] - 1));
            m_dataStr.append(m_pDataStr);
            sprintf(m_pDataStr, ",%4d", static_cast<AMDTUInt32>(tmp->m_pBins[j]));
            m_dataStr.append(m_pDataStr);
            m_dataStr.append("\n");
        }

        m_dataStr.append("\n");
    }

    m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());
} // ReportHistogramCounters

void ppReporterCsv::ReportCumulativeCounters(AMDTUInt32 num, AMDTFloat32* pHist, AMDTUInt32* pCounterId)
{
    m_dataStr.clear();
    m_dataStr.append("\n");
    m_dataStr.append("\nCUMULATIVE COUNTERS\n");
    sprintf(m_pDataStr, "%s", "COUNTER");
    m_dataStr.append(m_pDataStr);
    sprintf(m_pDataStr, ",%s", "CUMULATIVE VALUE");
    m_dataStr.append(m_pDataStr);
    m_dataStr.append("\n");

    for (AMDTUInt32 i = 0; i < num; i++)
    {
        gtString counterName = m_supportedCounterIdNameVec[pCounterId[i]];
        sprintf(m_pDataStr, "%s", counterName.asASCIICharArray());
        m_dataStr.append(m_pDataStr);
        sprintf(m_pDataStr, ",%8.2f", pHist[i]);
        m_dataStr.append(m_pDataStr);
        m_dataStr.append("\n");
    }

    m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());
} // ReportCumulativeCounters

void ppReporterCsv::WriteProcessData(AMDTUInt32 recCnt, AMDTPwrProcessInfo*& pInfo)
{
    AMDTUInt32 cnt = 0;
    AMDTFloat32 totalPower = 0;
    AMDTUInt32 totalRecords = 0;
    AMDTPwrProcessInfo* recInfo = nullptr;
    m_dataStr.clear();
    m_dataStr.append("\n");
    m_dataStr.append("PROCESS PROFILING DATA");
    m_dataStr.append(PROCESS_INFO_CSV_HDR_FORMAT);
    m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());

    for (cnt = 0; cnt < recCnt; cnt++)
    {
        recInfo = &pInfo[cnt];

        if (nullptr != recInfo)
        {
            totalRecords += recInfo->m_sampleCnt;
            totalPower += recInfo->m_power;
        }
    }

    for (cnt = 0; cnt < recCnt; cnt++)
    {
        recInfo = &pInfo[cnt];

        if (nullptr != recInfo)
        {
            m_dataStr.clear();

            sprintf(m_pDataStr, PROCESS_INFO_CSV_FORMAT,
                    cnt,
                    recInfo->m_pid,
                    recInfo->m_sampleCnt,
#if AMDT_BUILD_TARGET != AMDT_LINUX_OS // Windows
                    recInfo->m_ipc,
#endif
                    recInfo->m_power,
                    (recInfo->m_power * 100) / totalPower,
                    recInfo->m_name,
                    recInfo->m_path);

            m_dataStr.append(m_pDataStr);
            m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());

        }
    }

    m_dataStr.clear();
    sprintf(m_pDataStr,
            "\nProfile Sesssion Power Consumption:\t%3.2f\nTotal PID record collected %d",
            totalPower,
            totalRecords);

    m_dataStr.append(m_pDataStr);
    m_dataStr.append("\n");
    m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());
}
void ppReporterCsv::WriteModuleData(AMDTUInt32 recCnt, AMDTPwrModuleData*& pInfo, AMDTFloat32 totalPower)
{
    AMDTUInt32 cnt = 0;
    AMDTPwrModuleData* recInfo = nullptr;
    m_dataStr.clear();
    m_dataStr.append("\n");
    m_dataStr.append("MODULE PROFILING DATA");
    m_dataStr.append(MODULE_INFO_CSV_HDR_FORMAT);
    m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());

    for (cnt = 0; cnt < recCnt; cnt++)
    {
        recInfo = &pInfo[cnt];

        if (nullptr != recInfo)
        {
            m_dataStr.clear();
            sprintf(m_pDataStr,  MODULE_INFO_CSV_FORMAT,
                    cnt,
                    recInfo->m_processId,
                    recInfo->m_sampleCnt,
                    recInfo->m_isKernel,
                    recInfo->m_power,
                    (recInfo->m_power * 100) / totalPower,
                    recInfo->m_loadAddr,
                    recInfo->m_size,
                    recInfo->m_name,
                    recInfo->m_path);

            m_dataStr.append(m_pDataStr);
            m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());

        }
    }

    m_dataStr.clear();
    sprintf(m_pDataStr,
            "\nProfile Sesssion Power Consumption:\t%3.2f\nTotal modules collected %d",
            totalPower,
            recCnt);

    m_dataStr.append(m_pDataStr);
    m_dataStr.append("\n");
    m_reportFile.write(m_dataStr.c_str(), m_dataStr.length());
}

