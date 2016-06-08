//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file AMDTActivityLogger.cpp
/// \brief  Implementation of the AMDTActivityLogger lib
//==============================================================================

#include <fstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <sstream>

#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osThread.h>

#include "AMDTActivityLogger.h"
#include "AMDTActivityLoggerProfileControl.h"
#include "AMDTCpuProfileControl.h"
#include "Common/OSUtils.h"
#include "Common/FileUtils.h"
#include "Common/StringUtils.h"
#include "Common/Logger.h"
#include "AMDTMutex.h"

using namespace std;
using namespace GPULogger;

#define INDENT "   "
#define DEFAULT_GROUP "Default"

Parameters g_params;
AMDTMutex g_mtx;
bool g_bInit = false;
bool g_bFinalized = false;

class PerfMarkerItem
{
public:
    PerfMarkerItem()
    {
        os = NULL;
        depth = 0;
    }

    ~PerfMarkerItem()
    {
        SAFE_DELETE(os);
    }

    ostream* os;
    int depth;

private:
    PerfMarkerItem(const PerfMarkerItem& obj);
    PerfMarkerItem& operator = (const PerfMarkerItem& obj);
};

class ofstream_with_filename : public ofstream
{
public:
    ofstream_with_filename(const char* file)
        : ofstream(file)
    {
        fileName = file;
    }

    string fileName;
};

map<osThreadId, PerfMarkerItem*> g_perfMarkerItemMap;
string g_strPerfFileName;

int GetPerfMarkerItem(PerfMarkerItem** ppItem)
{
    if (ppItem == NULL)
    {
        return AL_INTERNAL_ERROR;
    }

    stringstream ss;
    ostream* os = NULL;
    osThreadId tid = osGetUniqueCurrentThreadId();
    map<osThreadId, PerfMarkerItem*>::const_iterator it;
    it = g_perfMarkerItemMap.find(tid);

    if (it != g_perfMarkerItemMap.end())
    {
        *ppItem = it->second;
        return AL_SUCCESS;
    }
    else
    {
        if (g_params.m_bTimeOutBasedOutput)
        {
            // Timeout mode, create a tmp file
            string path;
            osProcessId pid = osGetCurrentProcessId();

            if (g_params.m_strOutputFile.empty())
            {
                path = FileUtils::GetDefaultOutputPath();
            }
            else
            {
                path = FileUtils::GetTempFragFilePath();
            }

            ss << path << pid << "_" << tid << PERFMARKER_EXT;
            os = new(nothrow) ofstream_with_filename(ss.str().c_str());
        }
        else
        {
            os = new(nothrow) stringstream();
        }

        if (os == NULL)
        {
            return AL_OUT_OF_MEMORY;
        }

        PerfMarkerItem* pItem = new(nothrow) PerfMarkerItem();

        if (pItem == NULL)
        {
            delete os;
            return AL_OUT_OF_MEMORY;
        }

        pItem->depth = 0;
        pItem->os = os;
        g_perfMarkerItemMap.insert(pair<osThreadId, PerfMarkerItem*>(tid, pItem));

        *ppItem = pItem;
        return AL_SUCCESS;
    }
}

extern "C"
int AL_API_CALL amdtInitializeActivityLogger()
{
    AMDTScopeLock lock(&g_mtx);

    if (g_bInit)
    {
        return AL_SUCCESS;
    }

    if (g_bFinalized)
    {
        return AL_FINALIZED_ACTIVITY_LOGGER;
    }

    // NOTE: the following code will only work with an internal build if you're
    //       also using an internal version of the ActivityLogger library
    bool isProfileAgentFound = false;
    string agent = OSUtils::Instance()->GetEnvVar("CL_AGENT");

    if (!agent.empty())
    {
        isProfileAgentFound = agent.find(CL_TRACE_AGENT_DLL) != string::npos ||
                              agent.find(CL_PROFILE_AGENT_DLL) != string::npos;
    }

    if (!isProfileAgentFound)
    {
        agent = OSUtils::Instance()->GetEnvVar("HSA_TOOLS_LIB");

        if (!agent.empty())
        {
            isProfileAgentFound = agent.find(HSA_TRACE_AGENT_DLL) != string::npos ||
                                  agent.find(HSA_PROFILE_AGENT_DLL) != string::npos;
        }
    }

    if (!isProfileAgentFound)
    {
        return AL_CODEXL_GPU_PROFILER_NOT_DETECTED;
    }

    FileUtils::GetParametersFromFile(g_params);

    g_bInit = true;

    if (g_params.m_strOutputFile.empty())
    {
        g_strPerfFileName = FileUtils::GetDefaultPerfMarkerOutputFile();
    }
    else
    {
        g_strPerfFileName = FileUtils::GetBaseFileName(g_params.m_strOutputFile) + PERFMARKER_EXT;
    }

#ifdef _DEBUG
    cout << "AMDTActivityLogger initialized.\n";
    cout << "AMDTActivityLogger output: " << g_strPerfFileName << endl;
#endif

    return AL_SUCCESS;
}

extern "C"
int AL_API_CALL amdtBeginMarker(const char* szMarkerName, const char* szGroupName, const char* szUserString)
{
    // TODO: szUserString is currently unused. Need to use it.
    (void)(szUserString);

    AMDTScopeLock lock(&g_mtx);

    if (!g_bInit)
    {
        return AL_UNINITIALIZED_ACTIVITY_LOGGER;
    }

    if (g_bFinalized)
    {
        return AL_FINALIZED_ACTIVITY_LOGGER;
    }

    if (szMarkerName == NULL)
    {
        return AL_NULL_MARKER_NAME;
    }

    string strGroupName;

    if (szGroupName == NULL)
    {
        strGroupName = DEFAULT_GROUP;
    }
    else
    {
        strGroupName = szGroupName;
    }

    if (strGroupName.empty())
    {
        strGroupName = DEFAULT_GROUP;
    }

    string strMarkerName(szMarkerName);

    PerfMarkerItem* pItem;
    int ret = GetPerfMarkerItem(&pItem);
    SpAssert(ret == AL_SUCCESS);

    if (ret != AL_SUCCESS)
    {
        return ret;
    }

    strMarkerName = StringUtils::Replace(strMarkerName, " ", string(SPACE));
    strGroupName = StringUtils::Replace(strGroupName, " ", string(SPACE));

    const size_t DEFAULT_MARKER_NAME_WIDTH = 50;

    bool fit = true;

    if (strMarkerName.length() >= DEFAULT_MARKER_NAME_WIDTH)
    {
        fit = false;
    }

    if (fit)
    {
        (*pItem->os) << left << setw(20) << "clBeginPerfMarker" << left << setw(DEFAULT_MARKER_NAME_WIDTH) << strMarkerName << setw(20) << OSUtils::Instance()->GetTimeNanos() << "   " << strGroupName << endl;
    }
    else
    {
        // super long marker name
        (*pItem->os) << "clBeginPerfMarker   " << strMarkerName << "   " << OSUtils::Instance()->GetTimeNanos() << "   " << strGroupName << endl;
    }

    pItem->depth++;

    return AL_SUCCESS;
}

extern "C"
int AL_API_CALL amdtEndMarker()
{
    AMDTScopeLock lock(&g_mtx);

    if (!g_bInit)
    {
        return AL_UNINITIALIZED_ACTIVITY_LOGGER;
    }

    if (g_bFinalized)
    {
        return AL_FINALIZED_ACTIVITY_LOGGER;
    }

    PerfMarkerItem* pItem;
    int ret = GetPerfMarkerItem(&pItem);
    SpAssert(ret == AL_SUCCESS);

    if (ret != AL_SUCCESS)
    {
        return ret;
    }

    if (pItem->depth <= 0)
    {
        return AL_UNBALANCED_MARKER;
    }

    (*pItem->os) << left << setw(20) << "clEndPerfMarker" << left << setw(20) << OSUtils::Instance()->GetTimeNanos() << endl;
    pItem->depth--;

    return AL_SUCCESS;
}

extern "C"
int AL_API_CALL amdtFinalizeActivityLogger()
{
    AMDTScopeLock lock(&g_mtx);

    if (g_bFinalized)
    {
        return AL_SUCCESS;
    }

    if (g_bInit)
    {
        ofstream fout;
        fout.open(g_strPerfFileName.c_str());

        if (!fout.fail())
        {
            // write header
            fout << "=====CodeXL Perfmarker Output=====\n";

            if (g_params.m_bCompatibilityMode)
            {
                fout << "ProfilerVersion=" << GPUPROFILER_BACKEND_MAJOR_VERSION << "." << GPUPROFILER_BACKEND_MINOR_VERSION << "." << GPUPROFILER_BACKEND_BUILD_NUMBER << endl;
            }

            for (map<osThreadId, PerfMarkerItem*>::iterator it = g_perfMarkerItemMap.begin(); it != g_perfMarkerItemMap.end(); ++it)
            {
                string content;
                // thread ID
                fout << it->first << endl;

                if (it->second->depth != 0)
                {
                    cout << "[Thread " << it->first << "] Unbalanced PerfMarker detected.\n";
                }

                if (g_params.m_bTimeOutBasedOutput)
                {
                    ofstream_with_filename* pOfstream = dynamic_cast<ofstream_with_filename*>(it->second->os);
                    pOfstream->close();
                    FileUtils::ReadFile(pOfstream->fileName, content);
                    remove(pOfstream->fileName.c_str());
                }
                else
                {
                    content = dynamic_cast<stringstream*>(it->second->os)->str();
                }

                // num of markers
                fout << StringUtils::GetNumLines(content) << endl;
                fout << content;
                delete it->second;
            }

            g_perfMarkerItemMap.clear();
            fout.close();

            g_bFinalized = true;

#ifdef _DEBUG
            cout << "AMDTActivityLogger finalized.\n";
#endif
            return AL_SUCCESS;
        }
        else
        {
            return AL_FAILED_TO_OPEN_OUTPUT_FILE;
        }
    }
    else
    {
        return AL_UNINITIALIZED_ACTIVITY_LOGGER;
    }
}

extern "C"
int AL_API_CALL amdtStopProfiling(amdtProfilingControlMode profilingControlMode)
{
    int result = AL_SUCCESS;

    if (profilingControlMode & AMDT_CPU_PROFILING)
    {
        result = AMDTCpuProfilePause();
    }
    else
    {
        result = AMDTActivityLoggerProfileControl::Instance()->StopProfiling(profilingControlMode);
    }

    return result;
}

extern "C"
int AL_API_CALL amdtResumeProfiling(amdtProfilingControlMode profilingControlMode)
{
    int result = AL_SUCCESS;

    if (profilingControlMode & AMDT_CPU_PROFILING)
    {
        result = AMDTCpuProfileResume();
    }
    else
    {
        result = AMDTActivityLoggerProfileControl::Instance()->ResumeProfiling(profilingControlMode);
    }

    return result;
}

extern "C"
int AL_API_CALL amdtStopProfilingEx()
{
    return amdtStopProfiling(AMDT_CPU_PROFILING);
}

extern "C"
int AL_API_CALL amdtResumeProfilingEx()
{
    return amdtResumeProfiling(AMDT_CPU_PROFILING);
}
