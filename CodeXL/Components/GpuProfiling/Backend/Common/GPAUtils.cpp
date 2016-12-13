//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief GPUPerfAPI Utilities
//==============================================================================

#include "GPAUtils.h"
#include "StringUtils.h"
#include "FileUtils.h"
#include "DeviceInfoUtils.h"
#include "Logger.h"
#include "GlobalSettings.h"
#include <string.h> //for strcmp
#include <iostream>
#include <algorithm>
#include <functional>
#include "AMDTMutex.h"
#include "OSUtils.h"

using namespace std;
using namespace GPULogger;

AMDTMutex mtx("GPAMutex");

GPAUtils::GPAUtils()
{
    m_nMaxNumCounter = 1000;
    m_pGetAvailableCountersByGen = NULL;
    m_pGetAvailableCountersForDevice = NULL;
    m_API = GPA_API__LAST;
    m_bInit = false;
    m_nMaxPass = GPA_INFINITE_PASS;
}

bool GPAUtils::Open(void* context)
{
    // Remove it when Peter make GPA threadsafe
    AMDTScopeLock lock(mtx);

    if (!m_GPALoader.Loaded())
    {
        return false;
    }

    if (StatusCheck(m_GPALoader.GPA_OpenContext(context)) != GPA_STATUS_OK)
    {
        return false;
    }

    return true;
}

bool GPAUtils::Close()
{
    if (!m_GPALoader.Loaded())
    {
        return false;
    }

    if (StatusCheck(m_GPALoader.GPA_CloseContext()) != GPA_STATUS_OK)
    {
        return false;
    }

    return true;
}

void GPAUtils::FilterNonComputeCounters(GPA_HW_GENERATION gen, CounterList& counterList, bool counterListWithDescription)
{
    SP_UNREFERENCED_PARAMETER(gen);

    // unfortunately, we still need to hard code the list of valid DC performance counters because
    // asking GPADX11 for the list of available counters returns all counters, not just the compute
    // counters, so we have to filter that list based on what we know are compute counters

    // this isn't very efficient, however it's only used during intialization, or when CodeXLGpuProfiler requests the list of counters
    for (CounterList::iterator it = counterList.begin(); it != counterList.end();)
    {
        string strCounterName = *it;

#ifdef AMDT_INTERNAL

        // check for internal counters (all counters with an underscore)
        if (strCounterName.find("_") != string::npos)
        {
            // except for the D3D query counters that have an underscore
            if (strCounterName.find("PrimsWritten_") == string::npos &&
                strCounterName.find("PrimsStorageNeed_") == string::npos &&
                strCounterName.find("OverflowPred_") == string::npos)
            {
                ++it;
                continue;
            }
        }
        else
#endif
            if (strCounterName.compare("GPUTime") == 0)
            {
                ++it;

                if (counterListWithDescription)
                {
                    ++it;
                }

                continue;
            }
            else if (strCounterName.find("CS") == 0)
            {
                // all counters starting with "CS" except CSBusy and CSTime are considered compute counters
                if (0 != strCounterName.compare("CSBusy") && 0 != strCounterName.compare("CSTime") && 0 != strCounterName.compare("CSInvocations"))
                {
                    ++it;

                    if (counterListWithDescription)
                    {
                        ++it;
                    }

                    continue;
                }
            }

        it = counterList.erase(it);

        if (counterListWithDescription)
        {
            it = counterList.erase(it);
        }
    }
}

void GPAUtils::FilterNonComputeCountersGdt(GDT_HW_GENERATION gen, CounterList& counterList)
{
    GPA_HW_GENERATION gpaHwGen = GdtHwGenToGpaHwGen(gen);
    return FilterNonComputeCounters(gpaHwGen, counterList);
}

bool GPAUtils::EnableCounters()
{
    if (!m_GPALoader.Loaded())
    {
        return false;
    }

    if (m_selectedCounters.size() == 0)
    {
#ifndef AMDT_INTERNAL
        // enable all counters
        return m_GPALoader.GPA_EnableAllCounters() == GPA_STATUS_OK;
#else
        cout << "Unable to enable all counters in internal build." << endl;
        return false;
#endif
    }
    else
    {
        // enable counters from counter file
        if (m_selectedCounters.size() > m_nMaxNumCounter)
        {
            m_selectedCounters.resize(m_nMaxNumCounter);
            Log(logWARNING, "Number of selected counters exceeds the limit(%d), Only the first %d counters are enabled.\n", m_nMaxNumCounter, m_nMaxNumCounter);
        }

        // enable the counter set
        if (!EnableCounterSet(m_selectedCounters))
        {
            return false;
        }
    }

    return true;
}


bool GPAUtils::InitGPA(GPA_API_Type api,
                       const gtString& strDLLPath,
                       std::string& strError,
                       const char* pszCounterFile,
                       CounterList* pSelectedCounters,
                       size_t nMaxPass)
{
    m_API = api;
    m_nMaxPass = nMaxPass;

    const char* pszErrorMessage = NULL;

    SP_TODO("Solve API Loader Issues for installed libraries in unicode directory");
    string utf8DllPath;
    StringUtils::WideStringToUtf8String(strDLLPath.asCharArray(), utf8DllPath);
    bool bGPALoaded = m_GPALoader.Load(utf8DllPath.c_str(), api, &pszErrorMessage);

    if (bGPALoaded)
    {
        if (GPA_STATUS_OK != m_GPALoader.GPA_RegisterLoggingCallback(GPA_LOGGING_ERROR_AND_MESSAGE, GPALogCallback))
        {
            Log(logERROR, "Failed to register GPA logging callback\n");
        }
    }

    if (pszErrorMessage != NULL)
    {
        strError = pszErrorMessage;
    }

    if (!GPUPerfAPICounterLoader::Instance()->IsLoaded())
    {
        GPUPerfAPICounterLoader::Instance()->LoadPerfAPICounterDll(strDLLPath);
    }

    m_pGetAvailableCountersForDevice = GPUPerfAPICounterLoader::Instance()->GetGPAAvailableCountersForDeviceProc();
    m_pGetAvailableCountersByGen = GPUPerfAPICounterLoader::Instance()->GetGPAAvailableCountersByGenerationProc();

    if (pszCounterFile != NULL)
    {
        CounterList tmpCounterList;
        bool bRet = FileUtils::ReadFile(pszCounterFile, tmpCounterList, true) && bGPALoaded;

        for (CounterList::iterator it = tmpCounterList.begin(); it != tmpCounterList.end(); ++it)
        {
            std::string counterName = StringUtils::Trim(*it); // trim off any whitespace in counter name
            GPA_HW_GENERATION gen;
            VerifyCounter(counterName, gen);

            if (gen == GPA_HW_GENERATION_NONE)
            {
                cout << "Unknown counter " << *it << " encountered in the counter file." << endl;
                Log(logTRACE, "Unknown counter %s.\n", it->c_str());
            }
            else
            {
                m_selectedCounters.push_back(*it);
            }
        }

        if (pSelectedCounters != NULL)
        {
            // copy selected counters
            *pSelectedCounters = m_selectedCounters;
        }

        m_bInit = bRet;
        return bRet;
    }
    else
    {
        m_bInit = bGPALoaded;
        return bGPALoaded;
    }
}

#ifndef _DEBUG
GPA_Status GPAUtils::StatusCheck(GPA_Status status)
{
    return status;
}
#else
GPA_Status GPAUtils::StatusCheck(GPA_Status status)
{
    switch (status)
    {
        case GPA_STATUS_ERROR_NULL_POINTER :
            Log(logERROR, "Null pointer\n");
            break;

        case GPA_STATUS_ERROR_COUNTERS_NOT_OPEN :
            Log(logERROR, "Counters not opened\n");
            break;

        case GPA_STATUS_ERROR_COUNTERS_ALREADY_OPEN :
            Log(logERROR, "Counters already opened\n");
            break;

        case GPA_STATUS_ERROR_INDEX_OUT_OF_RANGE :
            Log(logERROR, "Index out of range\n");
            break;

        case GPA_STATUS_ERROR_NOT_FOUND :
            Log(logERROR, "Not found\n");
            break;

        case GPA_STATUS_ERROR_ALREADY_ENABLED :
            Log(logERROR, "Already enabled\n");
            break;

        case GPA_STATUS_ERROR_NO_COUNTERS_ENABLED :
            Log(logERROR, "No counters enabled\n");
            break;

        case GPA_STATUS_ERROR_NOT_ENABLED :
            Log(logERROR, "Not enabled\n");
            break;

        case GPA_STATUS_ERROR_SAMPLING_NOT_STARTED :
            Log(logERROR, "Sampling not started\n");
            break;

        case GPA_STATUS_ERROR_SAMPLING_ALREADY_STARTED :
            Log(logERROR, "Sampling already started\n");
            break;

        case GPA_STATUS_ERROR_SAMPLING_NOT_ENDED :
            Log(logERROR, "Sampling not ended\n");
            break;

        case GPA_STATUS_ERROR_NOT_ENOUGH_PASSES :
            Log(logERROR, "Not enough passes\n");
            break;

        case GPA_STATUS_ERROR_PASS_NOT_ENDED :
            Log(logERROR, "Pass not ended\n");
            break;

        case GPA_STATUS_ERROR_PASS_NOT_STARTED :
            Log(logERROR, "Pass not started\n");
            break;

        case GPA_STATUS_ERROR_PASS_ALREADY_STARTED :
            Log(logERROR, "Pass already started\n");
            break;

        case GPA_STATUS_ERROR_SAMPLE_NOT_STARTED :
            Log(logERROR, "Sample not found\n");
            break;

        case GPA_STATUS_ERROR_SAMPLE_ALREADY_STARTED :
            Log(logERROR, "Sample already started\n");
            break;

        case GPA_STATUS_ERROR_SAMPLE_NOT_ENDED :
            Log(logERROR, "sample not ended\n");
            break;

        case GPA_STATUS_ERROR_CANNOT_CHANGE_COUNTERS_WHEN_SAMPLING :
            Log(logERROR, "Cannot change counters when sampling\n");
            break;

        case GPA_STATUS_ERROR_SESSION_NOT_FOUND :
            Log(logERROR, "Session not found\n");
            break;

        case GPA_STATUS_ERROR_SAMPLE_NOT_FOUND :
            Log(logERROR, "Sample not found\n");
            break;

        case GPA_STATUS_ERROR_SAMPLE_NOT_FOUND_IN_ALL_PASSES :
            Log(logERROR, "Sample not found in all passes\n");
            break;

        case GPA_STATUS_ERROR_COUNTER_NOT_OF_SPECIFIED_TYPE :
            Log(logERROR, "Counter not of specified type\n");
            break;

        case GPA_STATUS_ERROR_READING_COUNTER_RESULT :
            Log(logERROR, "Error reading counter result\n");
            break;

        case GPA_STATUS_ERROR_VARIABLE_NUMBER_OF_SAMPLES_IN_PASSES :
            Log(logERROR, "Variable number of samples in passes\n");
            break;

        case GPA_STATUS_ERROR_FAILED :
            Log(logERROR, "Failed\n");
            break;

        case GPA_STATUS_ERROR_HARDWARE_NOT_SUPPORTED :
            Log(logERROR, "Hardware not supported\n");
            break;

        case GPA_STATUS_ERROR_DRIVER_NOT_SUPPORTED:
            Log(logERROR, "Driver version not supported\n");
            break;


        default:

            if (status != GPA_STATUS_OK)
            {
                Log(logERROR, "Unknown error\n");
            }

            break;
    }

    return status;
}
#endif


bool GPAUtils::EnableCounterSet(const CounterList& selectedCounterNames)
{
    // if the list of selected counters is the full set of counters, and we don't have to worry about max passes,
    // then just call EnableAllCounters.  It is a little faster than using EnableCounterStr, which itself has to loop
    // through the counter list, requesting the counter index and then calling EnableCounter with the index. This gave
    // a minor perf improvement when profiling an application that dispatches many kernels (since for each dispatch,
    // the counters are re-enabled)
    gpa_uint32 numAvailableCounters;

    if (StatusCheck(m_GPALoader.GPA_GetNumCounters(&numAvailableCounters)) == GPA_STATUS_OK)
    {
        if (selectedCounterNames.size() == numAvailableCounters && GPA_INFINITE_PASS == m_nMaxPass)
        {
            if (StatusCheck(m_GPALoader.GPA_EnableAllCounters()) == GPA_STATUS_OK)
            {
                return true;
            }
        }
    }

    for (size_t i = 0; i < selectedCounterNames.size(); i++)
    {
        if (StatusCheck(m_GPALoader.GPA_EnableCounterStr(selectedCounterNames[i].c_str())) != GPA_STATUS_OK)
        {
            Log(logMESSAGE, "Can't enable counter : %s\n", selectedCounterNames[i].c_str());
        }

        if (m_nMaxPass != GPA_INFINITE_PASS)
        {
            gpa_uint32 nPassCount;
            m_GPALoader.GPA_GetPassCount(&nPassCount);

            if (nPassCount > m_nMaxPass)
            {
                m_GPALoader.GPA_DisableCounterStr(selectedCounterNames[i].c_str());
                cout << "Max number of enabled counters reached. Counter \"" << selectedCounterNames[i] << "\" ignored." << endl;
            }
        }
    }

    return true;
}

GPA_HW_GENERATION GPAUtils::GdtHwGenToGpaHwGen(const GDT_HW_GENERATION gdtHwGen)
{
    GPA_HW_GENERATION gpaHwGen = GPA_HW_GENERATION_NONE;

    switch (gdtHwGen)
    {
        case GDT_HW_GENERATION_SOUTHERNISLAND:
            gpaHwGen = GPA_HW_GENERATION_SOUTHERNISLAND;
            break;

        case GDT_HW_GENERATION_SEAISLAND:
            gpaHwGen = GPA_HW_GENERATION_SEAISLAND;
            break;

        case GDT_HW_GENERATION_VOLCANICISLAND:
            gpaHwGen = GPA_HW_GENERATION_VOLCANICISLAND;
            break;

        default:
            break;
    }

    return gpaHwGen;
}

void GPAUtils::GetCounterValues(gpa_uint32 uSessionID, CounterList& counterDataTable, gpa_uint32& sampleCount, gpa_uint32& count)
{
    bool readyResult = false;

    if (uSessionID > 0)
    {
        while (!readyResult)
        {
            m_GPALoader.GPA_IsSessionReady(&readyResult, uSessionID);
        }
    }

    m_GPALoader.GPA_GetEnabledCount(&count);

    string str;

    m_GPALoader.GPA_GetSampleCount(uSessionID, &sampleCount);

    for (gpa_uint32 sample = 0 ; sample < sampleCount ; sample++)
    {
        for (gpa_uint32 counter = 0 ; counter < count ; counter++)
        {
            gpa_uint32 enabledCounterIndex;
            m_GPALoader.GPA_GetEnabledIndex(counter, &enabledCounterIndex);
            GPA_Type type;
            m_GPALoader.GPA_GetCounterDataType(enabledCounterIndex, &type);

            const char* name;
            m_GPALoader.GPA_GetCounterName(enabledCounterIndex, &name);
            string strName = name;
            int precision = 2;

            if (strName.find("GPUTime") != string::npos)
            {
                precision = 5;
            }

            bool bConvertUnit = false;

            /// In GPADX11, TexMemBytesRead is the equivalent coutner for FetchSize
            /// execept it's unit is in byte instead of kilobyte
            /// Convert it to kilobyte so that it's consistent in client side
            if (strName.find("TexMemBytesRead") != string::npos)
            {
                bConvertUnit = true;
            }

            /// In GPADX11, FastPath is reported in byte instead of kilobyte
            /// Convert it to kilobyte so that it's consistent in client side
            if (strName.find("CSFastPath") != string::npos)
            {
                bConvertUnit = true;
            }

            // Same for CSCompletePath
            if (strName.find("CSCompletePath") != string::npos)
            {
                bConvertUnit = true;
            }

            if (type == GPA_TYPE_UINT32)
            {
                gpa_uint32 value;
                m_GPALoader.GPA_GetSampleUInt32(uSessionID, sample, enabledCounterIndex, &value);

                str = StringUtils::ToString(value);
                counterDataTable.push_back(str);
            }
            else if (type == GPA_TYPE_UINT64)
            {
                gpa_uint64 value;
                m_GPALoader.GPA_GetSampleUInt64(uSessionID, sample, enabledCounterIndex, &value);

                str = StringUtils::ToString(value);
                counterDataTable.push_back(str);
            }
            else if (type == GPA_TYPE_FLOAT32)
            {
                gpa_float32 value;
                m_GPALoader.GPA_GetSampleFloat32(uSessionID, sample, enabledCounterIndex, &value);

                if (bConvertUnit)
                {
                    value /= 1024.0f;
                }

                str = StringUtils::ToStringPrecision(value, precision);
                counterDataTable.push_back(str);
            }
            else if (type == GPA_TYPE_FLOAT64)
            {
                gpa_float64 value;
                m_GPALoader.GPA_GetSampleFloat64(uSessionID, sample, enabledCounterIndex, &value);

                if (bConvertUnit)
                {
                    value /= 1024.0f;
                }

                str = StringUtils::ToStringPrecision(value, precision);
                counterDataTable.push_back(str);
            }
            else
            {
                Log(logERROR, "Unknown counter type\n");
            }
        }
    }

}

gpa_uint32 GPAUtils::GetEnabledCounterNames(CounterList& enabledCounters)
{
    gpa_uint32 count;
    const char* name;
    m_GPALoader.GPA_GetEnabledCount(&count);

    for (gpa_uint32 counter = 0 ; counter < count ; counter++)
    {
        gpa_uint32 enabledCounterIndex;
        m_GPALoader.GPA_GetEnabledIndex(counter, &enabledCounterIndex);
        m_GPALoader.GPA_GetCounterName(enabledCounterIndex, &name);
        string counterName(name);
        enabledCounters.push_back(counterName);
    }

    return count;
}

CounterList& GPAUtils::GetCounters(GPA_HW_GENERATION generation, const bool shouldIncludeCounterDescriptions)
{
    CounterList& list = m_HWCounterMap[generation];

    if (list.empty())
    {
        GPA_ICounterAccessor* pAccessor = NULL;
        SpAssertRet(m_pGetAvailableCountersByGen != NULL) list;
        SpAssertRet(m_API != GPA_API__LAST) list;
        m_pGetAvailableCountersByGen(m_API, generation, &pAccessor);

        if (pAccessor == NULL)
        {
            return list;
        }

        gpa_uint32 nCounters = pAccessor->GetNumCounters();
        list.resize(shouldIncludeCounterDescriptions ? nCounters * 2 : nCounters);

        unsigned int counterListIndex = 0;

        for (gpa_uint32 i = 0; i < nCounters; ++i)
        {
            list[counterListIndex++] = pAccessor->GetCounterName(i);

            if (shouldIncludeCounterDescriptions)
            {
                list[counterListIndex++] = pAccessor->GetCounterDescription(i);
            }
        }
    }

    return list;
}

CounterList& GPAUtils::GetCountersForDevice(gpa_uint32 uDeviceid, gpa_uint32 uRevisionid, size_t nMaxPass)
{
    CounterList& list = m_HWCounterDeviceMap[uDeviceid];

    if (list.empty())
    {
        GPA_ICounterAccessor* pAccessor = NULL;
        GPA_ICounterScheduler* pScheduler = NULL;
        SpAssertRet(m_pGetAvailableCountersForDevice != NULL) list;
        SpAssertRet(m_API != GPA_API__LAST) list;
        static const int AMD_VENDOR_ID = 0x1002;
        m_pGetAvailableCountersForDevice(m_API, AMD_VENDOR_ID, uDeviceid, uRevisionid, &pAccessor, &pScheduler);
        SpAssertRet(pAccessor != NULL) list;
        SpAssertRet(pScheduler != NULL) list;

        pScheduler->SetCounterAccessor(pAccessor, AMD_VENDOR_ID, uDeviceid, uRevisionid);
        pScheduler->DisableAllCounters();

        gpa_uint32 nCounters = pAccessor->GetNumCounters();
        list.clear();

        gpa_uint32 uRequiredPass;

        for (gpa_uint32 i = 0; i < nCounters; ++i)
        {
            pScheduler->EnableCounter(i);
            pScheduler->GetNumRequiredPasses(&uRequiredPass);

            if (uRequiredPass <= nMaxPass)
            {
                list.push_back(pAccessor->GetCounterName(i));
            }
            else
            {
                pScheduler->DisableCounter(i);
                continue;
            }
        }
    }

    return list;
}

bool GPAUtils::SetEnabledCounters(const CounterList& countersToEnable)
{
    m_selectedCounters.assign(countersToEnable.begin(), countersToEnable.end());
    return true;
}

bool GPAUtils::GetAvailableCounters(GPA_HW_GENERATION generation, CounterList& availableCounters, const bool shouldIncludeCounterDescriptions)
{
    bool retVal = true;

    switch (generation)
    {
        case GPA_HW_GENERATION_SOUTHERNISLAND:
        case GPA_HW_GENERATION_SEAISLAND:
        case GPA_HW_GENERATION_VOLCANICISLAND:
            retVal = true;
            break;

        default:
            retVal = false;
            break;
    }

    if (retVal)
    {
        availableCounters = GetCounters(generation, shouldIncludeCounterDescriptions);
    }

    return retVal;
}

bool GPAUtils::GetAvailableCountersGdt(GDT_HW_GENERATION generation, CounterList& availableCounters)
{
    GPA_HW_GENERATION gpaHwGen = GdtHwGenToGpaHwGen(generation);
    bool retVal = true;

    if (GPA_HW_GENERATION_NONE == gpaHwGen)
    {
        retVal = false;
    }
    else
    {
        retVal = GetAvailableCounters(gpaHwGen, availableCounters);
    }

    return retVal;
}

bool GPAUtils::GetAvailableCountersForDevice(gpa_uint32 deviceId, gpa_uint32 revisionId, size_t nMaxPass, CounterList& availableCounters)
{
    availableCounters = GetCountersForDevice(deviceId, revisionId, nMaxPass);
    return true;
}

void GPAUtils::VerifyCounter(const std::string& strCounter, GPA_HW_GENERATION& generation)
{
    CounterList& list = GetCounters(GPA_HW_GENERATION_VOLCANICISLAND);
    CounterList::iterator it = std::find(list.begin(), list.end(), strCounter);

    if (it != list.end())
    {
        generation = GPA_HW_GENERATION_VOLCANICISLAND;
        return;
    }

    list = GetCounters(GPA_HW_GENERATION_SEAISLAND);
    it = std::find(list.begin(), list.end(), strCounter);

    if (it != list.end())
    {
        generation = GPA_HW_GENERATION_SEAISLAND;
        return;
    }

    list = GetCounters(GPA_HW_GENERATION_SOUTHERNISLAND);
    it = std::find(list.begin(), list.end(), strCounter);

    if (it != list.end())
    {
        generation = GPA_HW_GENERATION_SOUTHERNISLAND;
        return;
    }

    generation = GPA_HW_GENERATION_NONE;
}

void GPAUtils::GPALogCallback(GPA_Logging_Type messageType, const char* message)
{
    LogType logType = logMESSAGE;

    if (messageType == GPA_LOGGING_ERROR)
    {
        logType = logERROR;
    }
    else if (messageType == GPA_LOGGING_TRACE)
    {
        logType = logTRACE;
    }

    Log(logType, "GPA: %s\n", message);
}
