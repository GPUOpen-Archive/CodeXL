//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages pointers to each saved API object for
///        API tracing.
//==============================================================================

#include "APIInfoManagerBase.h"
#include <fstream>
#include <ostream>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <math.h>

#include <AMDTOSWrappers/Include/osProcess.h>

#include "../Common/Defs.h"
#include "../Common/FileUtils.h"
#include "../Common/GlobalSettings.h"
#include "../Common/Logger.h"
#include "../Common/Version.h"
#include "../Common/StringUtils.h"
#include "../Common/OSUtils.h"
#include "../Common/StackTracer.h"
#include "AMDTMutex.h"

using namespace std;
using namespace GPULogger;

void APIBase::WriteAPIEntry(std::ostream& sout)
{
    sout << GetRetString() << " = ";
    sout << m_strName << " ( ";
    sout << ToString() << " )";
}

bool APIBase::WriteTimestampEntry(std::ostream& sout, bool bTimeout)
{
    SP_UNREFERENCED_PARAMETER(bTimeout);
    // APIType APITypeName StartTime  EndTime
    // APIName
    sout << std::left << std::setw(45) << m_strName;

    // start time
    sout << std::left << std::setw(21) << m_ullStart;
    // end time
    sout << std::left << std::setw(21) << m_ullEnd;

    return true;
}

void APIBase::WriteStackEntry(std::ostream& sout)
{
    if (m_pStackEntry == NULL)
    {
        // place holder
        sout << m_strName;
        return;
    }

    sout << m_strName << "\t";

    if (m_pStackEntry->m_strSymName.empty())
    {
        sout << m_pStackEntry->m_strSymAddr << "+";
        sout << StringUtils::ToHexString(m_pStackEntry->m_dwDisplacement);
    }
    else
    {
        string newSymName = StringUtils::Replace(m_pStackEntry->m_strSymName, " ", string(SPACE));
        sout << newSymName << "\t";
        sout << m_pStackEntry->m_dwLineNum << "\t";
        string newFileName = StringUtils::Replace(m_pStackEntry->m_strFile, " ", string(SPACE));
        sout << newFileName;
    }
}

APIInfoManagerBase::APIInfoManagerBase(void) :
    TraceInfoManager()
{
    m_strTraceModuleName.clear();
}

APIInfoManagerBase::~APIInfoManagerBase(void)
{
}

std::string APIInfoManagerBase::GetTempFileName(osProcessId pid, osThreadId tid, const std::string& strExtension)
{
    stringstream ss;
    std::string path;

    if (GlobalSettings::GetInstance()->m_params.m_strOutputFile.empty())
    {
        path = FileUtils::GetDefaultOutputPath();
    }
    else
    {
        path = FileUtils::GetTempFragFilePath();
    }

    // File name: pid_tid.[modname.]<ext>
    string fileName;

    if (m_strTraceModuleName.empty())
    {
        ss << path << pid << "_" << tid;
    }
    else
    {
        ss << path << pid << "_" << tid << "." << m_strTraceModuleName;
    }

    ss << strExtension;

    return ss.str();
}

void APIInfoManagerBase::FlushTraceData(bool bForceFlush)
{
    SP_UNREFERENCED_PARAMETER(bForceFlush);
    m_mtxFlush.Lock();
    osProcessId pid = osGetCurrentProcessId();
    TraceInfoMap& nonActiveMap = m_TraceInfoMap[ 1 - m_iActiveMap ];

    for (TraceInfoMap::iterator mapIt = nonActiveMap.begin(); mapIt != nonActiveMap.end(); mapIt++)
    {
        osThreadId tid = mapIt->first;

        string tmpTraceFile = GetTempFileName(pid, tid, TMP_TRACE_EXT);
        string tmpTimestampFile = GetTempFileName(pid, tid, TMP_TIME_STAMP_EXT);

        bool bEnableStackTrace = GlobalSettings::GetInstance()->m_params.m_bStackTrace && StackTracer::Instance()->IsInitialized();
        string tmpStackTraceFile;
        ofstream foutST;

        if (bEnableStackTrace)
        {
            tmpStackTraceFile = GetTempFileName(pid, tid, TMP_TRACE_STACK_EXT);
            foutST.open(tmpStackTraceFile.c_str(), fstream::out | fstream::app);
            bEnableStackTrace = !foutST.fail();
        }

        // Open file for append
        ofstream foutTrace(tmpTraceFile.c_str(), fstream::out | fstream::app);
        ofstream foutTS(tmpTimestampFile.c_str(), fstream::out | fstream::app);

        if (foutTrace.fail() || foutTS.fail())
        {
            continue;
        }

        while (!mapIt->second.empty())
        {
            APIBase* item =  dynamic_cast<APIBase*>(mapIt->second.front());

            //WriteTimestampEntry(foutTS, item, !bForceFlush);
            item->WriteTimestampEntry(foutTS, m_bTimeOutMode);
            foutTS << endl;

            // If isReady, write API entry as well
            //WriteAPIEntry(foutTrace, item);
            item->WriteAPIEntry(foutTrace);
            foutTrace << endl;

            if (bEnableStackTrace)
            {
                //WriteStackEntry(foutST, item);
                item->WriteStackEntry(foutST);
                foutST << endl;
            }

            mapIt->second.pop_front();

            delete item;
        }

        foutTrace.close();
        foutTS.close();

        if (bEnableStackTrace)
        {
            foutST.close();
        }
    }

    string tmpKernelTimestampFile = GetTempFileName(pid, 0, TMP_KERNEL_TIME_STAMP_EXT);
    ofstream foutKTS(tmpKernelTimestampFile.c_str(), fstream::out | fstream::app);
    FlushNonAPITimestampData(foutKTS);
    foutKTS.close();

    m_mtxFlush.Unlock();
}

void APIInfoManagerBase::SaveToOutputFile()
{
    //*********************Atp file format*************************
    // TraceFileVersion=*.*
    // Application=
    // ApplicationArgs=
    // =====AMD APP Profiler Trace Output=====
    // [Thread ID]
    // [Num of Items]
    // [API trace item 0]
    // ...
    // [Another Thread block]
    // =====AMD APP Profiler Timestamp Output=====
    // [Thread ID]
    // [Num of Items]
    // [Timestamp item 0]
    // ...
    // [Another Thread block]
    //*********************End Atp file format**********************
    ofstream fout(m_strOutputFile.c_str());

    if (fout.fail())
    {
        Log(logWARNING, "Failed to open file: %s.\n", m_strOutputFile.c_str());
        cout << "Failed to generate .atp file: " << m_strOutputFile << ". Make sure you have permission to write to the path you specified." << endl;
        return;
    }
    else
    {
        WriteAPITraceDataToStream(fout);
        WriteTimestampToStream(fout);
        fout.close();
    }

    if (GlobalSettings::GetInstance()->m_params.m_bStackTrace)
    {
        string stackFile = FileUtils::GetBaseFileName(m_strOutputFile) + TRACE_STACK_EXT;

        ofstream fout1(stackFile.c_str());

        if (fout1.fail())
        {
            Log(logWARNING, "Failed to open file: %s.\n", stackFile.c_str());
            cout << "Failed to generate .atp file: " << stackFile << ". Make sure you have permission to write to the path you specified." << endl;
            return;
        }
        else
        {
            WriteStackTraceDataToStream(fout1);
            fout1.close();
        }
    }
}

void APIInfoManagerBase::LoadAPIFilterFile(const std::string& strFileName)
{
    vector<string> tmpArr;
    FileUtils::ReadFile(strFileName, tmpArr, true);

    for (vector<string>::iterator it = tmpArr.begin(); it != tmpArr.end(); ++it)
    {
        string name = StringUtils::Trim(*it);
        AddAPIToFilter(name);
    }
}

void APIInfoManagerBase::WriteTimestampToStream(std::ostream& sout)
{
    // format:
    // =====AMD APP Profiler Timestamp Output=====
    // [Thread ????]
    // {Number of APIs}
    // APIType APITypeName StartTime  EndTime
    // ...
    // [Thread ????]
    // ...
    //
    TraceInfoMap& activeMap = m_TraceInfoMap[ 0 ];
    sout << "=====CodeXL " << m_strTraceModuleName << " Timestamp Output=====" << endl;

    for (TraceInfoMap::iterator mapIt = activeMap.begin(); mapIt != activeMap.end(); mapIt++)
    {
        // Thread ID
        sout << mapIt->first << endl;
        // Number of APIs
        sout << mapIt->second.size() << endl;

        for (list<ITraceEntry*>::iterator listIt = mapIt->second.begin(); listIt != mapIt->second.end(); listIt++)
        {
            APIBase* en = dynamic_cast<APIBase*>(*listIt);
            en->WriteTimestampEntry(sout, m_bTimeOutMode);
            sout << endl;
        }
    }
}

void APIInfoManagerBase::WriteAPITraceDataToStream(std::ostream& sout)
{
    // Double buffering is used in Timeout mode, for normal mode, only m_APIInfoMap[ 0 ] is used
    TraceInfoMap& activeMap = m_TraceInfoMap[ 0 ];
    sout << "=====CodeXL " << m_strTraceModuleName << " API Trace Output=====" << endl;

    for (TraceInfoMap::iterator mapIt = activeMap.begin(); mapIt != activeMap.end(); mapIt++)
    {
        // Thread ID
        sout << mapIt->first << endl;
        // Number of APIs
        sout << mapIt->second.size() << endl;

        for (list<ITraceEntry*>::iterator listIt = mapIt->second.begin(); listIt != mapIt->second.end(); listIt++)
        {
            APIBase* en = dynamic_cast<APIBase*>(*listIt);
            en->WriteAPIEntry(sout);
            sout << endl;
        }
    }
}

void APIInfoManagerBase::WriteStackTraceDataToStream(std::ostream& sout)
{
    // Double buffering is used in Timeout mode, for normal mode, only m_APIInfoMap[ 0 ] is used
    TraceInfoMap& activeMap = m_TraceInfoMap[ 0 ];

    sout << "=====CodeXL " << m_strTraceModuleName << " Stack Trace Output=====" << endl;

    if (GlobalSettings::GetInstance()->m_params.m_bCompatibilityMode)
    {
        if (!GlobalSettings::GetInstance()->m_params.m_bTestMode)
        {
            sout << "ProfilerVersion=" << GPUPROFILER_BACKEND_MAJOR_VERSION << "." << GPUPROFILER_BACKEND_MINOR_VERSION << "." << GPUPROFILER_BACKEND_BUILD_NUMBER << endl;
        }
        else
        {
            sout << "ProfilerVersion=" << 0 << "." << 0 << "." << 0 << endl;
        }
    }

    int fakeTID = 0;

    for (TraceInfoMap::iterator mapIt = activeMap.begin(); mapIt != activeMap.end(); ++mapIt)
    {

        if (!GlobalSettings::GetInstance()->m_params.m_bTestMode)
        {
            // Thread ID
            sout << mapIt->first << endl;
        }
        else
        {
            sout << fakeTID << endl;
            fakeTID++;
        }

        // Number of APIs
        sout << mapIt->second.size() << endl;

        for (list<ITraceEntry*>::iterator listIt = mapIt->second.begin(); listIt != mapIt->second.end(); listIt++)
        {
            APIBase* en = dynamic_cast<APIBase*>(*listIt);
            en->WriteStackEntry(sout);
            sout << endl;
        }
    }
}

void APIInfoManagerBase::SetOutputFile(const std::string& strFileName)
{
    stringstream ss;

    if (strFileName.empty())
    {
        // $defaultPath/$exeName
        ss << FileUtils::GetDefaultOutputPath() << FileUtils::GetExeName();
    }
    else
    {
        std::string strExtension = FileUtils::GetFileExtension(strFileName);

        if (strExtension == TRACE_EXT || strExtension == OCCUPANCY_EXT || strExtension == PERF_COUNTER_EXT)
        {
            // strip .atp, .csv or .occupancy
            string strBaseFileName = FileUtils::GetBaseFileName(strFileName);
            ss << strBaseFileName;
        }
        else
        {
            ss << strFileName;
        }
    }

    // Append .$modName.atp
    if (m_strTraceModuleName.empty())
    {
        ss << "." << TRACE_EXT;
    }
    else
    {
        ss << "." << m_strTraceModuleName << "." << TRACE_EXT;
    }

    m_strOutputFile = ss.str();
}

void APIInfoManagerBase::FlushNonAPITimestampData(std::ostream& sout)
{
    // do nothing in base class
    SP_UNREFERENCED_PARAMETER(sout);
}

void APIInfoManagerBase::AddAPIToFilter(const std::string& strAPIName)
{
    // do nothing in base class
    SP_UNREFERENCED_PARAMETER(strAPIName);
}

