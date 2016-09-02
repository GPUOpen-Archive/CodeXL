//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Class for managing the kernel occupancy objects
//==============================================================================

#include <sstream>
#include <string>
#include <fstream>
#include <iostream>

#include <AMDTOSWrappers/Include/osProcess.h>

#include "CLOccupancyInfoManager.h"
#include "CLOccupancyFile.h"
#include "../Common/GlobalSettings.h"
#include "../Common/FileUtils.h"
#include "../Common/Logger.h"
#include "../Common/Defs.h"

using namespace std;
using namespace GPULogger;

char OccupancyInfoEntry::m_cListSeparator = ';';

OccupancyInfoEntry::~OccupancyInfoEntry()
{
    SAFE_DELETE(m_pCLCUInfo);
}

std::string OccupancyInfoEntry::ToString()
{
    stringstream sout;

    SpAssertRet(m_pCLCUInfo != NULL) "";

    // print out the thread ID
    sout << left << m_tid << m_cListSeparator;

    // print out kernel name
    sout << left << m_strKernelName << m_cListSeparator;

    // print out the device name
    sout << left << m_strDeviceName << m_cListSeparator;

    size_t nCU = 0;
    size_t nMaxNumWavePerCU = 0;
    size_t nMaxVGPR = 0;
    size_t nMaxSGPR = 0;
    size_t nMaxLDS = 0;
    size_t nUsedVGPR = 0;
    size_t nUsedSGPR = 0;
    size_t nUsedLDS = 0;
    size_t nWaveSize = 0;
    size_t nFlattenedLocalSize = 0;
    size_t nFlattenedGlobalSize = 0;
    size_t nMaxGlobalSize = 0;
    size_t nActiveWaveLimitedByVGPR = 0;
    size_t nActiveWaveLimtiedBySGPR = 0;
    size_t nActiveWaveLimtiedByLDS = 0;
    size_t nActiveWaveLimtiedByWG = 0;
    float fOccupancy = 0;

    m_pCLCUInfo->ReadCUParam(CU_PARAMS_KERNEL_OCCUPANCY, fOccupancy);

    // print out work group size (flattened)
    m_pCLCUInfo->ReadCUParam(CU_PARAMS_KERNEL_WG_SIZE, nFlattenedLocalSize);
    size_t nWorkItemsInWG = nFlattenedLocalSize;

    if (fOccupancy < 0.0f)
    {
        nWorkItemsInWG = 0;
    }

    // print out the number of compute units
    m_pCLCUInfo->ReadCUParam(CU_PARAMS_NBR_COMPUTE_UNITS, nCU);
    sout << left << nCU << m_cListSeparator;

    // print out the max. number of wavefronts per compute unit
    m_pCLCUInfo->ReadCUParam(CU_PARAMS_WAVEFRONT_PER_COMPUTE_UNIT, nMaxNumWavePerCU);
    sout << left << nMaxNumWavePerCU << m_cListSeparator;

    // print out the max. number of workgroup per compute unit
    sout << left << m_pCLCUInfo->GetMaxWorkgroupPerCU(nWorkItemsInWG) << m_cListSeparator;

    // print out the max number of vector GPR on a compute unit
    m_pCLCUInfo->ReadCUParam(CU_PARAMS_VECTOR_GPRS_MAX, nMaxVGPR);
    sout << left << nMaxVGPR << m_cListSeparator;

    // print out the max number of scalar GPR on a compute unit
    m_pCLCUInfo->ReadCUParam(CU_PARAMS_SCALAR_GPRS_MAX, nMaxSGPR);
    sout << left << nMaxSGPR << m_cListSeparator;

    // print the max amount of available LDS
    m_pCLCUInfo->ReadCUParam(CU_PARAMS_LDS_MAX, nMaxLDS);
    sout << left << nMaxLDS << m_cListSeparator;

    // print the number of vector GPR used
    m_pCLCUInfo->ReadCUParam(CU_PARAMS_VECTOR_GPRS_USED, nUsedVGPR);
    sout << left << nUsedVGPR << m_cListSeparator;

    // print the number of scalar GPR used
    m_pCLCUInfo->ReadCUParam(CU_PARAMS_SCALAR_GPRS_USED, nUsedSGPR);
    sout << left << nUsedSGPR << m_cListSeparator;

    // print the amount of LDS used
    m_pCLCUInfo->ReadCUParam(CU_PARAMS_LDS_USED, nUsedLDS);
    sout << left << nUsedLDS << m_cListSeparator;

    // print the wavefront size
    m_pCLCUInfo->ReadCUParam(CU_PARAMS_WAVEFRONT_SIZE, nWaveSize);
    sout << left << nWaveSize << m_cListSeparator;

    sout << left << nWorkItemsInWG << m_cListSeparator;


    //print out the number of wavefronts in work-group
    if (nWorkItemsInWG == 0)
    {
        sout << left << 0 << m_cListSeparator;
    }
    else
    {
        if (nWaveSize != 0)
        {
            sout << left << (unsigned int)ceil((double)nWorkItemsInWG / (double)nWaveSize) << m_cListSeparator;
        }
        else
        {
            sout << left << 0 << m_cListSeparator;
        }
    }

    // print out the max work-group size
    size_t nMaxWGSize = 0;
    m_pCLCUInfo->ReadCUParam(CU_PARAMS_WG_SIZE_MAX, nMaxWGSize);
    sout << left << nMaxWGSize << m_cListSeparator;

    //print out the max number of wavefronts in a work-group
    if (nWaveSize == 0)
    {
        sout << left << 0 << m_cListSeparator;
    }
    else
    {
        sout << left << (unsigned int)ceil((double)nMaxWGSize / (double)nWaveSize) << m_cListSeparator;
    }

    // print out the global work size
    sout << std::dec;
    m_pCLCUInfo->ReadCUParam(CU_PARAMS_KERNEL_GLOBAL_SIZE, nFlattenedGlobalSize);

    if (fOccupancy < 0.0f)
    {
        nFlattenedGlobalSize = 0;
    }

    sout << left << nFlattenedGlobalSize << m_cListSeparator;

    // print the maximum global size
    m_pCLCUInfo->ReadCUParam(CU_PARAMS_GLOBAL_SIZE_MAX, nMaxGlobalSize);
    sout << left << nMaxGlobalSize << m_cListSeparator;

    // print the number of vector GPR-limited waves
    m_pCLCUInfo->ReadCUParam(CU_PARAMS_WF_MAX_VECTOR_GPRS, nActiveWaveLimitedByVGPR);
    sout << left << nActiveWaveLimitedByVGPR << m_cListSeparator;

    // print the number of scalar GPR-limited waves
    m_pCLCUInfo->ReadCUParam(CU_PARAMS_WF_MAX_SCALAR_GPRS, nActiveWaveLimtiedBySGPR);
    sout << left << nActiveWaveLimtiedBySGPR << m_cListSeparator;

    // print the number of LDS-limited waves
    m_pCLCUInfo->ReadCUParam(CU_PARAMS_WF_MAX_LDS, nActiveWaveLimtiedByLDS);
    sout << left << nActiveWaveLimtiedByLDS << m_cListSeparator;

    // print the number of wavefronts limited by the work-group size (other factors not limiting
    // the number of wavefronts)
    m_pCLCUInfo->ReadCUParam(CU_PARAMS_WF_MAX_WG, nActiveWaveLimtiedByWG);
    sout << left << nActiveWaveLimtiedByWG << m_cListSeparator;

    //print out the kernel occupancy
    sout << left << fOccupancy << std::endl;

    return sout.str();
}

OccupancyInfoManager::OccupancyInfoManager() :
    TraceInfoManager(),
    m_bIsProfilingEnabled(true),
    m_bDelayStartEnabled(false),
    m_bProfilerDurationEnabled(false),
    m_delayInMilliseconds(0ul),
    m_durationInMilliseconds(0ul)
{
    m_delayTimer = nullptr;
    m_durationTimer = nullptr;
}

OccupancyInfoManager::~OccupancyInfoManager()
{
    if (m_delayTimer)
    {
        m_delayTimer->stopTimer();
        delete m_delayTimer;
        m_delayTimer = nullptr;
    }

    if (m_durationTimer)
    {
        m_durationTimer->stopTimer();
        delete m_durationTimer;
        m_durationTimer = nullptr;
    }
}

void OccupancyInfoManager::FlushTraceData(bool bForceFlush)
{
    SP_UNREFERENCED_PARAMETER(bForceFlush);
    m_mtxFlush.Lock();
    osProcessId pid = osGetCurrentProcessId();
    TraceInfoMap& nonActiveMap = m_TraceInfoMap[ 1 - m_iActiveMap ];

    for (TraceInfoMap::iterator mapIt = nonActiveMap.begin(); mapIt != nonActiveMap.end(); ++mapIt)
    {
        osThreadId tid = mapIt->first;
        stringstream ss;
        string path;

        if (GlobalSettings::GetInstance()->m_params.m_strOutputFile.empty())
        {
            path = FileUtils::GetDefaultOutputPath();
        }
        else
        {
            path = FileUtils::GetTempFragFilePath();
        }

        // File name: pid_tid.TMP_OCCUPANCY_EXT

        ss << path << pid << "_" << tid << TMP_OCCUPANCY_EXT;
        string tmpTraceFile = ss.str();

        // Open file for append
        ofstream foutTrace(tmpTraceFile.c_str(), fstream::out | fstream::app);

        if (foutTrace.fail())
        {
            continue;
        }

        while (!mapIt->second.empty())
        {
            ITraceEntry* item = mapIt->second.front();
            foutTrace << item->ToString();
            mapIt->second.pop_front();

            delete item;
        }

        foutTrace.close();
    }

    m_mtxFlush.Unlock();
}

void OccupancyInfoManager::SetOutputFile(const string& strFileName)
{
    std::string strExtension("");

    if (strFileName.empty())
    {
        m_strOutputFile = FileUtils::GetDefaultOutputPath() + FileUtils::GetExeName() + OCCUPANCY_EXT;
    }
    else
    {
        strExtension = FileUtils::GetFileExtension(strFileName);

        if (strExtension != OCCUPANCY_EXT)
        {
            if ((strExtension == TRACE_EXT) || (strExtension == PERF_COUNTER_EXT))
            {
                string strBaseFileName = FileUtils::GetBaseFileName(strFileName);
                m_strOutputFile = strBaseFileName + "." + OCCUPANCY_EXT;
            }
            else
            {
                m_strOutputFile = strFileName + "." + OCCUPANCY_EXT;
            }
        }
        else
        {
            m_strOutputFile = strFileName;
        }
    }

    if (FileUtils::FileExist(m_strOutputFile))
    {
        cout << "Specified output file " << m_strOutputFile << " already exists. It will be overwritten.\n";
        remove(m_strOutputFile.c_str());
    }

}

void OccupancyInfoManager::SaveToOccupancyFile()
{
    TraceInfoMap& activeMap = m_TraceInfoMap[ 0 ];

    if (!activeMap.empty())
    {
        ofstream fout(m_strOutputFile.c_str());

        if (fout.fail())
        {
            Log(logWARNING, "Failed to open file: %s.\n", m_strOutputFile.c_str());
            cout << "Failed to generate " OCCUPANCY_EXT " file: " << m_strOutputFile << ". Make sure you have permission to write to the path you specified." << endl;
            return;
        }

        CLOccupancyHdr header;
        header.m_strAppArgs = GlobalSettings::GetInstance()->m_params.m_strCmdArgs;
        header.m_strAppName = FileUtils::GetExeFullPathAsUnicode();
        header.m_listSeparator = GlobalSettings::GetInstance()->m_params.m_cOutputSeparator;
        WriteOccupancyHeader(fout, header, m_cListSeparator);
        fout << endl;

        for (TraceInfoMap::iterator mapIt = activeMap.begin(); mapIt != activeMap.end(); mapIt++)
        {
            for (list<ITraceEntry*>::iterator listIt = mapIt->second.begin(); listIt != mapIt->second.end(); listIt++)
            {
                ITraceEntry* en = *listIt;
                fout << en->ToString().c_str();
            }
        }
    }
}


void CLOccupancyAgentTimerEndResponse(ProfilerTimerType timerType)
{
    switch (timerType)
    {
        case PROFILEDELAYTIMER:
            OccupancyInfoManager::Instance()->ResumeTracing();
            OccupancyInfoManager::Instance()->EnableProfiling(true);
            unsigned long profilerDuration;

            if (OccupancyInfoManager::Instance()->IsProfilerDurationEnabled(profilerDuration))
            {
                OccupancyInfoManager::Instance()->CreateTimer(PROFILEDURATIONTIMER, profilerDuration);
                OccupancyInfoManager::Instance()->SetTimerFinishHandler(PROFILEDURATIONTIMER, CLOccupancyAgentTimerEndResponse);
                OccupancyInfoManager::Instance()->startTimer(PROFILEDURATIONTIMER);
            }

            break;

        case PROFILEDURATIONTIMER:
            OccupancyInfoManager::Instance()->StopTracing();
            OccupancyInfoManager::Instance()->EnableProfiling(false);
            break;

        default:
            break;
    }
}


bool OccupancyInfoManager::IsProfilerDelayEnabled(unsigned long& delayInMilliseconds)
{
    delayInMilliseconds = m_delayInMilliseconds;
    return m_bDelayStartEnabled;
}

bool OccupancyInfoManager::IsProfilerDurationEnabled(unsigned long& durationInMilliseconds)
{
    durationInMilliseconds = m_durationInMilliseconds;
    return m_bProfilerDurationEnabled;
}

void OccupancyInfoManager::SetTimerFinishHandler(ProfilerTimerType timerType, TimerEndHandler timerEndHandler)
{
    switch (timerType)
    {
        case PROFILEDELAYTIMER:
            m_delayTimer->SetTimerFinishHandler(timerEndHandler);
            break;

        case PROFILEDURATIONTIMER:
            m_durationTimer->SetTimerFinishHandler(timerEndHandler);
            break;

        default:
            break;
    }
}

void OccupancyInfoManager::EnableProfileDelayStart(bool doEnable, unsigned long delayInSeconds)
{
    m_bDelayStartEnabled = doEnable;
    m_delayInMilliseconds = doEnable ? delayInSeconds : 0;
}

void OccupancyInfoManager::EnableProfileDuration(bool doEnable, unsigned long durationInMilliseconds)
{
    m_bProfilerDurationEnabled = doEnable;
    m_durationInMilliseconds = doEnable ? durationInMilliseconds : 0;
}


void OccupancyInfoManager::CreateTimer(ProfilerTimerType timerType, unsigned long timeIntervalInMilliseconds)
{
    switch (timerType)
    {
        case PROFILEDELAYTIMER:
            if (m_delayTimer == nullptr && timeIntervalInMilliseconds > 0)
            {
                m_delayTimer = new ProfilerTimer(timeIntervalInMilliseconds);
                m_delayTimer->SetTimerType(PROFILEDELAYTIMER);
                m_bDelayStartEnabled = true;
                m_delayInMilliseconds = timeIntervalInMilliseconds;
            }

            break;

        case PROFILEDURATIONTIMER:
            if (m_durationTimer == nullptr && timeIntervalInMilliseconds > 0)
            {
                m_durationTimer = new ProfilerTimer(timeIntervalInMilliseconds);
                m_durationTimer->SetTimerType(PROFILEDURATIONTIMER);
                m_bProfilerDurationEnabled = true;
                m_durationInMilliseconds = timeIntervalInMilliseconds;
            }

            break;

        default:
            break;
    }
}



void OccupancyInfoManager::startTimer(ProfilerTimerType timerType)
{
    if (m_delayTimer || m_durationTimer)
    {
        switch (timerType)
        {
            case PROFILEDELAYTIMER:
                m_delayTimer->startTimer(true);
                break;

            case PROFILEDURATIONTIMER:
                m_durationTimer->startTimer(true);
                break;

            default:
                break;
        }
    }
}

