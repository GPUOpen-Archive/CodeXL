//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief CL Atp File writer and parser
//==============================================================================

//std
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>

#include "CLAtpFile.h"
//Local Common
#include "../Common/OSUtils.h"
#include "../Common/FileUtils.h"
#include "../Common/StringUtils.h"
#include "../Common/Logger.h"
#include "../Common/Version.h"
#include "../Common/Defs.h"
#include "../Common/ATPFileUtils.h"
#include "../CLCommon/CLFunctionEnumDefs.h"
#include "../CLCommon/CLUtils.h"
#include "../CLCommon/CLFunctionDefs.h"

//Infra
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osProcess.h>

#define OUT_OF_MEMORY_ERROR              "Session load is NN% complete. The complete session data size exceeds the available memory. Only the data loaded so far will be displayed."

using namespace std;

const int BUFSIZE = 2048; ///< Max buffer size of single line in atp file.

using namespace GPULogger;
#define MAX_WORKING_SET_SIZE 1300000000 //just above 1 GB we are usually get hit with memory bad allocation in QT
CLAPIType GetType(const std::string& strAPIName)
{
    if ((strAPIName.size() < 2) || (strAPIName[0] != 'c' || strAPIName[1] != 'l'))
    {
        return CL_UNKNOWN_API;
    }

    if (strAPIName.find("Enqueue") != string::npos)
    {
        if (strAPIName.find("Kernel") != string::npos || strAPIName.find("Task") != string::npos)
        {
            return CL_ENQUEUE_KERNEL;
        }
        else if ((strAPIName.find("Read") != string::npos || strAPIName.find("Write") != string::npos || strAPIName.find("Map") != string::npos || strAPIName.find("Copy") != string::npos) && strAPIName.find("SVM") == string::npos)
        {
            return CL_ENQUEUE_MEM;
        }
        else if (strAPIName.find("UnmapMemObject") != string::npos || strAPIName.find("ReleaseD3D10ObjectsKHR") != string::npos ||
                 strAPIName.find("AcquireGLObjects") != string::npos || strAPIName.find("ReleaseGLObjects") != string::npos || strAPIName.find("Marker") != string::npos || strAPIName.find("MigrateMemObjects") != string::npos ||
                 strAPIName.find("MarkerWithWaitList") != string::npos || strAPIName.find("BarrierWithWaitList") != string::npos || strAPIName.find("SVMFree") != string::npos ||
                 strAPIName.find("SVMMemFill") != string::npos || strAPIName.find("SVMMap") != string::npos)

        {
            return CL_ENQUEUE_OTHER_OPERATIONS;
        }
        else if (strAPIName.find("FillBuffer") != string::npos || strAPIName.find("FillImage") != string::npos || strAPIName.find("AcquireD3D10ObjectsKHR") != string::npos ||
                 strAPIName.find("SVMMemcpy") != string::npos || strAPIName.find("SVMUnmap") != string::npos)
        {
            return CL_ENQUEUE_DATA_OPERATIONS;
        }
        else if (strAPIName == "clEnqueueWaitForEvents" || strAPIName == "clEnqueueBarrier")
        {
            // clEnqueueWaitForEvents and clEnqueueBarrier are not true "enqueue" APIs in that they don't have an event they can query timestamps from
            return CL_API;
        }
        else // rest of enqueue command, e.g. EnqueueUnmap, GL/CL Interop and etc.
        {
            return CL_ENQUEUE_BASE_API;
        }
    }
    else
    {
        return CL_API;
    }
}

CLAPIInfo* CreateAPIInfo(const std::string& strAPIName)
{
    CLAPIInfo* retObj = NULL;
    CLAPIType apiType = GetType(strAPIName);

    switch (apiType)
    {
        case CL_API:
        {
            retObj = new CLAPIInfo();
        }
        break;

        case CL_ENQUEUE_MEM:
        {
            retObj = new CLMemAPIInfo();
        }
        break;

        case CL_ENQUEUE_OTHER_OPERATIONS:
        {
            retObj = new CLOtherEnqueueAPIInfo();
        }

        case CL_ENQUEUE_DATA_OPERATIONS:
        {
            retObj = new CLDataEnqueueAPIInfo();
        }
        break;

        case CL_ENQUEUE_KERNEL:
        {
            retObj = new CLKernelAPIInfo();
        }
        break;

        case CL_ENQUEUE_BASE_API:
        {
            retObj = new CLEnqueueAPI();
        }
        break;

        default:
            Log(logWARNING, "Unknown API %s\n", strAPIName.c_str());
            break;
    }

    if (retObj)
    {
        retObj->m_strName = strAPIName;
        retObj->m_Type = apiType;
    }

    return retObj;
}


bool ParseTimestamp(const char* buf, CLAPIInfo* pAPIInfo, bool bTimeoutMode)
{
    stringstream ss(buf);
    int apiTypeID;
    string apiName;
    ULONGLONG ullStart;
    ULONGLONG ullEnd;

    ss >> apiTypeID;
    CHECK_SS_ERROR(ss)
    ss >> apiName;
    CHECK_SS_ERROR(ss)

    if (!pAPIInfo)
    {
        return false;
    }

    pAPIInfo->m_uiAPIID = apiTypeID;

    if (apiName != pAPIInfo->m_strName)
    {
        // unmatched num of API trace items and num of timestamp items
        // in this case, number of api could greater than number of timestamp item,
        // it's harder to recover from this problem.
        Log(logWARNING, "Unexpected data in input file. Inconsistent API trace item and timestamp item.\n");
        //m_strWarningMsg = "[Warning]Unexpected data in input file. Incomplete summary pages may be generated.";
        return false;
    }

    ss >> ullStart;
    CHECK_SS_ERROR(ss)
    ss >> ullEnd;
    CHECK_SS_ERROR(ss)

    pAPIInfo->m_ullStart = ullStart;
    pAPIInfo->m_ullEnd = ullEnd;

    // enqueue api
    if (IsEnqueueAPI(apiTypeID))
    {
        CLEnqueueAPI* pEnAPI = (CLEnqueueAPI*)pAPIInfo;
        pEnAPI->m_bInfoMissing = false;
        unsigned int uiCmdType;
        string strCmdType;
        ULONGLONG ullQueue;
        ULONGLONG ullSubmit;
        ULONGLONG ullStartDevice;
        ULONGLONG ullEndDevice;
        unsigned int uiQueueID;
        unsigned int uiContextID;

        string strCmdQHandle;
        string strDeviceName;
        string strCntxHandle;

        ss >> uiCmdType;
        ss >> strCmdType;
        ss >> ullQueue;
        ss >> ullSubmit;
        ss >> ullStartDevice;
        ss >> ullEndDevice;
        ss >> uiQueueID;
        ss >> strCmdQHandle;
        ss >> uiContextID;
        ss >> strCntxHandle;
        ss >> strDeviceName;

        if (ss.fail())
        {
            // API failure, incomplete information
            pEnAPI->m_bInfoMissing = true;
            return true;
        }

        SpAssert(uiCmdType >= CL_COMMAND_NDRANGE_KERNEL);

        pEnAPI->m_strCMDType = strCmdType;
        pEnAPI->m_uiCMDType = uiCmdType;
        pEnAPI->m_strDevice = strDeviceName;
        pEnAPI->m_uiContextID = uiContextID;
        pEnAPI->m_uiQueueID = uiQueueID;
        pEnAPI->m_ullComplete = ullEndDevice;
        pEnAPI->m_ullQueue = ullQueue;
        pEnAPI->m_ullRunning = ullStartDevice;
        pEnAPI->m_ullSubmit = ullSubmit;
        pEnAPI->m_strCntxHandle = strCntxHandle;
        pEnAPI->m_strCmdQHandle = strCmdQHandle;

        // CL_COMMAND_NDRANGE_KERNEL, CL_COMMAND_TASK, CL_COMMAND_NATIVE_KERNEL
        if (uiCmdType <= CL_COMMAND_NATIVE_KERNEL)
        {
            CLKernelAPIInfo* pKAPI = (CLKernelAPIInfo*)pAPIInfo;

            string strKernelName;
            string strGlobal;
            string strLocal;

            ss >> pKAPI->m_strKernelHandle;

            if (uiCmdType == CL_COMMAND_NATIVE_KERNEL)
            {
                // assigning pKAPI->m_strKernelName allows a native kernel to have a name in the kernel summary page
                pKAPI->m_strKernelName = "NATIVE_KERNEL (" + pKAPI->m_strKernelHandle + ")";
                pKAPI->m_strGlobalWorkSize.clear();
                pKAPI->m_strGroupWorkSize.clear();
            }
            else
            {
                ss >> pKAPI->m_strKernelName;
                ss >> pKAPI->m_strGlobalWorkSize;
                ss >> pKAPI->m_strGroupWorkSize;
            }

            if (ss.fail())
            {
                // API failure, incomplete information
                pKAPI->m_bInfoMissing = true;
            }
            else
            {
                pKAPI->m_bInfoMissing = false;
            }
        }
        // CL_COMMAND_READ_BUFFER, CL_COMMAND_WRITE_BUFFER, CL_COMMAND_COPY_BUFFER
        // CL_COMMAND_READ_IMAGE, CL_COMMAND_WRITE_IMAGE, CL_COMMAND_COPY_IMAGE
        // CL_COMMAND_COPY_IMAGE_TO_BUFFER, CL_COMMAND_COPY_BUFFER_TO_IMAGE
        // CL_COMMAND_MAP_BUFFER, CL_COMMAND_MAP_IMAGE
        // CL_COMMAND_READ_BUFFER_RECT, CL_COMMAND_WRITE_BUFFER_RECT, CL_COMMAND_COPY_BUFFER_RECT,
        else if ((uiCmdType >= CL_COMMAND_READ_BUFFER && uiCmdType <= CL_COMMAND_MAP_IMAGE) ||
                 (uiCmdType >= CL_COMMAND_READ_BUFFER_RECT && uiCmdType <= CL_COMMAND_COPY_BUFFER_RECT))
        {
            CLMemAPIInfo* pMAPI = (CLMemAPIInfo*)pAPIInfo;
            ss >> pMAPI->m_uiTransferSize;

            if (ss.fail())
            {
                // API failure, incomplete information
                pMAPI->m_bInfoMissing = true;
            }
            else
            {
                pMAPI->m_bInfoMissing = false;
            }
        }
        // CL_COMMAND_FILL_BUFFER, CL_COMMAND_FILL_IMAGE, CL_COMMAND_SVM_MEMCPY, CL_COMMAND_SVM_MAP
        else if (uiCmdType == CL_COMMAND_FILL_BUFFER || uiCmdType == CL_COMMAND_FILL_IMAGE ||
                 uiCmdType == CL_COMMAND_SVM_MEMCPY || uiCmdType == CL_COMMAND_SVM_MAP)
        {
            CLDataEnqueueAPIInfo* pMAPI = (CLDataEnqueueAPIInfo*)pAPIInfo;
            ss >> pMAPI->m_uiDataSize;

            if (ss.fail())
            {
                // API failure, incomplete information
                pMAPI->m_bInfoMissing = true;
            }
            else
            {
                pMAPI->m_bInfoMissing = false;
            }
        }

        if (bTimeoutMode)
        {
            ss >> pEnAPI->m_strEventHandle;
        }
    }

    return true;
}

//
bool WriteTimestampEntry(std::ostream& sout, CLAPIInfo* pInfo)
{
    SP_TODO("write a unit test for this function.");
    // APIType
    sout << left << setw(5) << pInfo->m_uiAPIID;

    // APIName
    sout << left << setw(45) << pInfo->m_strName;

    // start time
    sout << left << setw(21) << pInfo->m_ullStart;
    // end time
    sout << left << setw(21) << pInfo->m_ullEnd;

    if (IsEnqueueAPI(pInfo->m_uiAPIID))
    {
        // dump GPU data
        CLEnqueueAPI* pEnAPI = (CLEnqueueAPI*)pInfo;

        if (!pEnAPI->m_bInfoMissing)
        {
            sout << left << setw(8) << pEnAPI->m_uiCMDType;
            sout << left << setw(40) << pEnAPI->m_strCMDType;

            sout << left << setw(21) << pEnAPI->m_ullQueue;
            sout << left << setw(21) << pEnAPI->m_ullSubmit;
            sout << left << setw(21) << pEnAPI->m_ullRunning;
            sout << left << setw(21) << pEnAPI->m_ullComplete;

            // command queue handle
            sout << std::dec << setw(10) << pEnAPI->m_uiQueueID;
            sout << setw(25) << pEnAPI->m_strCmdQHandle;
            // context handle
            sout << std::dec << setw(10) << pEnAPI->m_uiContextID;
            sout << setw(25) << pEnAPI->m_strCntxHandle;

            // device name
            sout << setw(30) << pEnAPI->m_strDevice;

            SpAssert(pEnAPI->m_uiCMDType >= CL_COMMAND_NDRANGE_KERNEL);

            // CL_COMMAND_NDRANGE_KERNEL, CL_COMMAND_TASK
            if (pEnAPI->m_uiCMDType <= CL_COMMAND_TASK)
            {
                // print out kernel name and kernel handle
                CLKernelAPIInfo* pKAPI = (CLKernelAPIInfo*)pInfo;
                sout << setw(25) << pKAPI->m_strKernelHandle;
                sout << pKAPI->m_strKernelName;

                // print out work group size, global size
                sout << "      " << pKAPI->m_strGlobalWorkSize;
                sout << "     " << pKAPI->m_strGroupWorkSize;
            }
            else if (pEnAPI->m_uiCMDType == CL_COMMAND_NATIVE_KERNEL)
            {
                // print out kernel function address
                CLKernelAPIInfo* pKAPI = (CLKernelAPIInfo*)pInfo;
                sout << setw(25) << pKAPI->m_strKernelHandle;
            }
            // CL_COMMAND_READ_BUFFER, CL_COMMAND_WRITE_BUFFER, CL_COMMAND_COPY_BUFFER
            // CL_COMMAND_READ_IMAGE, CL_COMMAND_WRITE_IMAGE, CL_COMMAND_COPY_IMAGE
            // CL_COMMAND_COPY_IMAGE_TO_BUFFER, CL_COMMAND_COPY_BUFFER_TO_IMAGE
            // CL_COMMAND_MAP_BUFFER, CL_COMMAND_MAP_IMAGE
            // CL_COMMAND_READ_BUFFER_RECT, CL_COMMAND_WRITE_BUFFER_RECT, CL_COMMAND_COPY_BUFFER_RECT
            // CL_COMMAND_FILL_BUFFER, CL_COMMAND_FILL_IMAGE
            else if ((pEnAPI->m_uiCMDType >= CL_COMMAND_READ_BUFFER && pEnAPI->m_uiCMDType <= CL_COMMAND_MAP_IMAGE) ||
                     (pEnAPI->m_uiCMDType >= CL_COMMAND_READ_BUFFER_RECT && pEnAPI->m_uiCMDType <= CL_COMMAND_COPY_BUFFER_RECT))
            {
                CLMemAPIInfo* pMAPI = (CLMemAPIInfo*)pInfo;
                sout << std::dec << setw(20) << pMAPI->m_uiTransferSize;
            }

            // CL_COMMAND_FILL_IMAGE, CL_COMMAND_FILL_BUFFER, CL_COMMAND_SVM_MAP, CL_COMMAND_SVM_UNMAP
            if (pEnAPI->m_uiCMDType == CL_COMMAND_FILL_IMAGE || pEnAPI->m_uiCMDType == CL_COMMAND_FILL_BUFFER ||
                pEnAPI->m_uiCMDType == CL_COMMAND_SVM_MAP || pEnAPI->m_uiCMDType == CL_COMMAND_SVM_UNMAP)
            {
                CLDataEnqueueAPIInfo* pMAPI = (CLDataEnqueueAPIInfo*)pInfo;
                sout << std::dec << setw(20) << pMAPI->m_uiDataSize;
            }
        }

        // switch back the dec mode
        sout << std::dec << std::endl;
    }
    else
    {
        sout << endl;
    }

    return true;
}

///
bool CLAtpFilePart::LoadGPUTimestampRaw(const string& strFile, EventMap& vTimestamps)
{
    char buf[BUFSIZE];

    ifstream fin(strFile.c_str());

    if (!fin.is_open())
    {
        return false;
    }

    while (!fin.eof())
    {
        memset(buf, 0, BUFSIZE * sizeof(char));
        fin.getline(buf, BUFSIZE);
        istringstream ss(buf);

        // check empty line.
        if (ss.str().empty())
        {
            continue;
        }

        std::string strEventHandle;
        unsigned int uiStatus;
        ULONGLONG ullTimestamp;

        ss >> strEventHandle;
        ss >> uiStatus;
        ss >> ullTimestamp;

        if (!ss.fail())
        {
            GPUTimestamp gpuTs;

            // first build a GPUTimestamp with this single timestamp filled in.
            switch (uiStatus)
            {
                case CL_COMPLETE:
                    gpuTs.m_ullCompleteTimestamp = ullTimestamp;
                    break;

                case CL_RUNNING:
                    gpuTs.m_ullRunningTimestamp = ullTimestamp;
                    break;

                case CL_SUBMITTED:
                    gpuTs.m_ullSubmitTimestamp = ullTimestamp;
                    break;

                case CL_QUEUED:
                    gpuTs.m_ullQueuedTimestamp = ullTimestamp;
                    break;

                default:
                    Log(logERROR, "Unknown event type\n");
            }

            // this is the algorithm used here:
            // check to see if we've already seen an event with this handle (this is check 1)
            //   if we have (check 1), then check whether we've seen this particular timestamp (Queue, Submit, Running, or Complete) (this is check 2)
            //     if we have (check 2), then this is a timestamp for a new event that is using a recycled event handle
            //     if not (check 2), then this is another timestamp for the already existing event handle
            //   if not (check 1), then this is an entirely new event
            EventMapIt it = vTimestamps.find(strEventHandle);

            if (it != vTimestamps.end())
            {
                bool found = false;

                for (auto itt = (it->second).begin(); itt != (it->second).end() && !found; ++itt)
                {
                    switch (uiStatus)
                    {
                        case CL_COMPLETE:
                            if ((*itt).m_ullCompleteTimestamp == 0)
                            {
                                (*itt).m_ullCompleteTimestamp = ullTimestamp;
                                found = true;
                            }

                            break;

                        case CL_RUNNING:
                            if ((*itt).m_ullRunningTimestamp == 0)
                            {
                                (*itt).m_ullRunningTimestamp = ullTimestamp;
                                found = true;
                            }

                            break;

                        case CL_SUBMITTED:
                            if ((*itt).m_ullSubmitTimestamp == 0)
                            {
                                (*itt).m_ullSubmitTimestamp = ullTimestamp;
                                found = true;
                            }

                            break;

                        case CL_QUEUED:
                            if ((*itt).m_ullQueuedTimestamp == 0)
                            {
                                (*itt).m_ullQueuedTimestamp = ullTimestamp;
                                found = true;
                            }

                            break;

                        default:
                            Log(logERROR, "Unknown event type\n");
                    }
                }

                // at this point "found" is false if this is a new event using a recycled handle
                if (!found)
                {
                    it->second.push_back(gpuTs);
                }
            }
            else
            {
                // this is an entirely new event that we haven't seen before
                GPUTimestampList list;
                list.push_back(gpuTs);
                vTimestamps.insert(EventMapPair(strEventHandle, list));
            }
        }
        else
        {
            Log(logERROR, "Unable to parse raw timestamp data\n");
        }
    }

    fin.close();

    return true;
}

CLAtpFilePart::CLAtpFilePart(const Config& config, bool shouldReleaseMemory) : IAtpFilePart(config, shouldReleaseMemory)
{
#define PART_NAME "ocl"
    m_strPartName = PART_NAME;
    m_sections.push_back("CodeXL " PART_NAME " API Trace Output");
    m_sections.push_back("CodeXL " PART_NAME " Timestamp Output");
#undef PART_NAME
    InitRealCLFunctions();
}

CLAtpFilePart::~CLAtpFilePart(void)
{
    if (m_shouldReleaseMemory)
    {
        // clean up all API object
        for (CLAPIInfoMap::iterator it = m_CLAPIInfoMap.begin(); it != m_CLAPIInfoMap.end(); it++)
        {
            std::vector<CLAPIInfo*>& apiList = it->second;

            for (std::vector<CLAPIInfo*>::iterator listIt = apiList.begin(); listIt != apiList.end(); listIt++)
            {
                if ((*listIt) != NULL)
                {
                    delete *listIt;
                }
            }

            apiList.clear();
        }

        m_CLAPIInfoMap.clear();
    }
}


bool CLAtpFilePart::MergeTimestamp(const string& strFile, EventMap& vTimestamps, vector<CLAPIInfo*>& apis)
{
    char buf[BUFSIZE];
    ifstream fin(strFile.c_str());

    if (!fin.is_open())
    {
        return false;
    }

    while (!fin.eof())
    {
        memset(buf, 0, BUFSIZE * sizeof(char));
        fin.getline(buf, BUFSIZE);

        istringstream ss(buf);

        if (ss.str().empty())
        {
            continue;
        }

        string strTmp;
        ss >> strTmp;  // API ID
        ss >> strTmp;  // API name

        if (ss.fail())
        {
            Log(logERROR, "Failed to parse tmp timestamp entry, val = %s\n", buf);
            continue;
        }

        CLAPIInfo* pApiInfo = CreateAPIInfo(strTmp);

        bool ret = ParseTimestamp(buf, pApiInfo, true);

        if (!ret)
        {
            Log(logERROR, "Failed to parse tmp timestamp entry, val = %s.\n", buf);
        }
        else
        {
            // Update GPU timestamps
            if (IsEnqueueAPI(pApiInfo->m_uiAPIID))
            {
                CLEnqueueAPI* pEnAPI = (CLEnqueueAPI*)pApiInfo;

                EventMapIt it = vTimestamps.find(pEnAPI->m_strEventHandle);

                if (it == vTimestamps.end())
                {
                    Log(logWARNING, "Timestamps are missing for command %s.\n", pEnAPI->m_strName.c_str());
                }
                else
                {
                    pEnAPI->m_ullComplete = it->second.front().m_ullCompleteTimestamp;
                    pEnAPI->m_ullRunning = it->second.front().m_ullRunningTimestamp;
                    pEnAPI->m_ullSubmit = it->second.front().m_ullSubmitTimestamp;
                    pEnAPI->m_ullQueue = it->second.front().m_ullQueuedTimestamp;

                    it->second.pop_front();

                    if (it->second.empty())
                    {
                        vTimestamps.erase(it);
                    }
                }

                apis.push_back(pApiInfo);
            }
            else
            {
                apis.push_back(pApiInfo);
            }
        }
    }

    fin.close();

    return true;
}

bool CLAtpFilePart::UpdateTmpTimestampFiles(const string& strTmpFilePath, const string& strFilePrefix)
{
    vector<string> files;
    string strRawFile;

    string strPartName;
    stringstream ss;
    ss << '.' << m_strPartName;
    strPartName = ss.str();

    if (!FileUtils::GetFilesUnderDir(strTmpFilePath , files, strFilePrefix))
    {
        return false;
    }
    else
    {
        EventMap eventMap;

        for (vector<string>::iterator it = files.begin(); it != files.end(); ++it)
        {
            size_t part_found;
            part_found = it->find(strPartName);

            if (part_found == string::npos)
            {
                Log(logWARNING, "Skipping file: %s. It does not contain: %s.\n", it->c_str(), strPartName.c_str());
                continue;
            }

            size_t found;
            found = it->find_last_of(".");

            if (found != string::npos)
            {
                string strExt = it->substr(found);

                if (strExt == TMP_GPU_TIME_STAMP_RAW_EXT)
                {
                    strRawFile = strTmpFilePath + '/' + *it;

                    if (!LoadGPUTimestampRaw(strRawFile, eventMap))
                    {
                        return false;
                    }

                    break;
                }
            }
            else
            {
                // wrong file name? ignore this one
                Log(logWARNING, "Incorrect file name : %s.\n", it->c_str());
                continue;
            }

        }

        for (vector<string>::iterator it = files.begin(); it != files.end(); ++it)
        {

            size_t part_found;
            part_found = it->find(strPartName);

            if (part_found == string::npos)
            {
                Log(logWARNING, "Skipping file: %s. It does not contain: %s.\n", it->c_str(), strPartName.c_str());
                continue;
            }

            size_t found;
            found = it->find_last_of(".");

            if (found != string::npos)
            {
                string strExt = it->substr(found);

                if (strExt == TMP_TIME_STAMP_EXT)
                {
                    string strFullname = strTmpFilePath + '/' + *it;
                    vector<CLAPIInfo*> apis;
                    MergeTimestamp(strFullname, eventMap, apis);
                    // delete original file
                    remove(strFullname.c_str());
                    // write new files.

                    ofstream fout(strFullname.c_str());

                    if (!fout.is_open())
                    {
                        Log(logERROR, "Failed to open file %s\n", it->c_str());
                        continue;
                    }

                    for (vector<CLAPIInfo*>::const_iterator infoIt = apis.begin(); infoIt != apis.end(); ++infoIt)
                    {
                        WriteTimestampEntry(fout, *infoIt);
                        delete(*infoIt);
                    }

                    apis.clear();
                    fout.close();

                }
            }
        }

        if (!strRawFile.empty())
        {
            // delete raw file.
            remove(strRawFile.c_str());
        }

        return true;
    }
}

void CLAtpFilePart::WriteHeaderSection(SP_fileStream& sout)
{
    set<std::string> excludedAPIs;
    ReadExcludedAPIs(m_config.strAPIFilterFile, excludedAPIs);
    WriteExcludedAPIs(sout, "CL", excludedAPIs);

    // Write platforms
    CLPlatformSet platformInfo;

    if (CLUtils::GetPlatformInfo(platformInfo))
    {
        CLPlatformSet::const_iterator itPlatformInfo;

        for (itPlatformInfo = platformInfo.begin(); itPlatformInfo != platformInfo.end(); itPlatformInfo++)
        {
            sout << "Device " << (*itPlatformInfo).strDeviceName.c_str() << " Platform Vendor = " << (*itPlatformInfo).strPlatformVendor.c_str() << endl;
            sout << "Device " << (*itPlatformInfo).strDeviceName.c_str() << " Platform Name = " << (*itPlatformInfo).strPlatformName.c_str() << endl;
            sout << "Device " << (*itPlatformInfo).strDeviceName.c_str() << " Platform Version = " << (*itPlatformInfo).strPlatformVersion.c_str() << endl;
            sout << "Device " << (*itPlatformInfo).strDeviceName.c_str() << " CLDriver Version = " << (*itPlatformInfo).strDriverVersion.c_str() << endl;
            sout << "Device " << (*itPlatformInfo).strDeviceName.c_str() << " CLRuntime Version = " << (*itPlatformInfo).strCLRuntime.c_str() << endl;
            sout << "Device " << (*itPlatformInfo).strDeviceName.c_str() << " NumberAppAddressBits = " << (*itPlatformInfo).uiNbrAddressBits << endl;
        }
    }
}

bool CLAtpFilePart::WriteContentSection(SP_fileStream& sout, const std::string& strTmpFilePath, const string& strPID)
{
    bool ret = false;
    SpAssertRet(m_sections.size() == 2) ret;

    if (m_config.bTimeOut || m_config.bMergeMode)
    {
#ifdef NON_BLOCKING_TIMEOUT
        UpdateTmpTimestampFiles(strTmpFilePath, strPID);
#endif
        stringstream ss;
        ss << "." << m_strPartName << TMP_TRACE_EXT;
        // Merge frags
        ret = FileUtils::MergeTmpTraceFiles(sout, strTmpFilePath, strPID, ss.str().c_str(), GetSectionHeader(m_sections[0]).c_str());

        ss.str("");
        ss << "." << m_strPartName << TMP_TIME_STAMP_EXT;
        ret |= FileUtils::MergeTmpTraceFiles(sout, strTmpFilePath, strPID, ss.str().c_str(), GetSectionHeader(m_sections[1]).c_str());
    }
    else
    {
        // read .$(ModName).atp and write to sout
        stringstream ss;

        std::string strExtension = FileUtils::GetFileExtension(m_config.strOutputFile);

        if (strExtension == TRACE_EXT || strExtension == OCCUPANCY_EXT || strExtension == PERF_COUNTER_EXT)
        {
            // strip .atp, .csv or .occupancy and append .$ModName.atp
            string strBaseFileName = FileUtils::GetBaseFileName(m_config.strOutputFile);
            ss << strBaseFileName << "." << m_strPartName << "." << TRACE_EXT;
        }
        else
        {
            // append $ModName.atp
            ss << m_config.strOutputFile << "." << m_strPartName << "." << TRACE_EXT;
        }

        string fileContent;
        ret = FileUtils::ReadFile(ss.str(), fileContent, false);
        sout << fileContent.c_str();
        remove(ss.str().c_str());
    }

    return ret;
}

bool CLAtpFilePart::Parse(std::istream& in, std::string& outErrorMsg)
{
    // Assumption: CL API trace section first then CL timestamp
    bool bError = false;
    bool bTSStart = false;
    std::string strProgressMessage = "Parsing trace data...";
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    size_t iterationCounter = 0;
#endif
    ErrorMessageUpdater errorMessageUpdater(outErrorMsg, this);

    do
    {
        if (m_shouldStopParsing)
        {
            break;
        }

        string line;
        READLINE(line)

        if (line.length() == 0)
        {
            continue;
        }

        if (line[0] == '=')
        {
            if (!bTSStart)
            {
                bTSStart = true;

                // read thread id
                ReadLine(in, line);
            }
            else
            {
                // finished reading CL trace sections
                RewindToPreviousPos(in);
                return true;
            }
        }

        //else it's thread id
        osThreadId tid = 0;
        bool ret = StringUtils::Parse(line, tid);

        if (!ret)
        {
            Log(logERROR, "Failed to parse thread ID, Unexpected data in input file.\n");
            return false;
        }

        READLINE(line)
        unsigned int apiNum = 0;
        ret = StringUtils::Parse(line, apiNum);

        if (!ret)
        {
            Log(logERROR, "Failed to parse thread number, Unexpected data in input file.\n");
            return false;
        }

        int apiIndex = -1;

        for (std::vector<IParserListener<CLAPIInfo>*>::iterator it = m_listenerList.begin(); it != m_listenerList.end(); it++)
        {
            if ((*it) != NULL)
            {
                (*it)->SetAPINum(tid, apiNum);
            }
        }

        // read all apis for this thread
        for (unsigned int i = 0; i < apiNum && !m_shouldStopParsing; i++)
        {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

            if (iterationCounter % 10000 == 0)
            {
                unsigned int processID = GetCurrentProcessId();
                unsigned int pageFaultCount = 0;
                size_t workingSetSize = 0;
                size_t peakWorkingSetSize = 0;
                size_t quotaPeakPagedPoolUsage = 0;
                size_t quotaPagedPoolUsage = 0;
                size_t quotaPeakNonPagedPoolUsage = 0;
                size_t quotaNonPagedPoolUsage = 0;
                size_t pagefileUsage = 0;
                size_t peakPagefileUsage = 0;
                size_t privateUsage = 0;

                if (osGetMemoryUsage(processID, pageFaultCount, workingSetSize, peakWorkingSetSize, quotaPeakPagedPoolUsage,
                                     quotaPagedPoolUsage, quotaPeakNonPagedPoolUsage, quotaNonPagedPoolUsage, pagefileUsage,
                                     peakPagefileUsage, privateUsage)
                    && workingSetSize > MAX_WORKING_SET_SIZE)
                {
                    m_shouldStopParsing = true;
                    double progressPersentage = (apiNum > 0 ? double(i) / double(apiNum) : 0.0) * 100;

                    m_strWarningMsg = StringUtils::Replace(OUT_OF_MEMORY_ERROR, "NN", StringUtils::ToStringPrecision(progressPersentage, 2));
                    return false;
                }
            }

            ++iterationCounter;
#endif
            apiIndex++;
            READLINE(line)

            ReportProgress(strProgressMessage, i, apiNum);

            if (!bTSStart)
            {
                // api trace
                string apiTraceStr = line;

                if (apiTraceStr.empty())
                {
                    continue;
                }

                string name;

                // ATP API line example:
                // CL_SUCCESS = clGetPlatformInfo ( 0x0B69F8C0;CL_PLATFORM_ICD_SUFFIX_KHR;0;NULL;[4] )

                // First find the API name (required for the API info object allocation):
                size_t equalSignPos = apiTraceStr.find_first_of("=");
                size_t nameEndIndex = apiTraceStr.find_first_of("(");

                if ((equalSignPos != string::npos) && (nameEndIndex != string::npos))
                {
                    // Increment the start index (we expect a space before the name starts):
                    size_t nameStartIndex = equalSignPos + 1;

                    // Get the name:
                    size_t nameLength = nameEndIndex - nameStartIndex - 1;

                    if ((nameStartIndex < apiTraceStr.size()) && (nameStartIndex + nameLength <= apiTraceStr.size()))
                    {
                        name = apiTraceStr.substr(nameStartIndex + 1, nameEndIndex - nameStartIndex - 2);

                        // Create the API info object:
                        CLAPIInfo* pAPIInfo = CreateAPIInfo(name);

                        // Parse the comment:
                        size_t commentStart = apiTraceStr.find_first_of("/*");

                        if (commentStart != string::npos)
                        {
                            size_t commentEnd = apiTraceStr.find_last_of("*/");

                            if (commentEnd != string::npos && commentEnd > commentStart + 3)
                            {
                                pAPIInfo->m_strComment = apiTraceStr.substr(commentStart + 2, commentEnd - commentStart - 3);
                                // Here is an assumption:
                                // There is one space between the last argument and comment
                                SP_TODO("When Qt client is in place, revisit this issue.");
                                pAPIInfo->m_strComment = pAPIInfo->m_strComment.substr(0, commentStart) + ")";

                                // Remove the comment from the API string:
                                apiTraceStr = apiTraceStr.substr(0, commentStart) + ")";
                            }
                        }

                        // Parse the return value:
                        pAPIInfo->m_strRet = apiTraceStr.substr(0, equalSignPos - 1);

                        // Parse the arg list:
                        size_t argListEndIndex = apiTraceStr.find_first_of(")");

                        if (argListEndIndex != string::npos)
                        {
                            size_t argListLength = argListEndIndex - nameEndIndex - 2;

                            if (nameEndIndex + argListLength < apiTraceStr.size())
                            {
                                pAPIInfo->m_ArgList = StringUtils::Trim(apiTraceStr.substr(nameEndIndex + 2, argListLength));
                            }
                        }
                        else
                        {
                            Log(logERROR, "Failed to parse API trace (%s), Unexpected data in input file.\n", line.c_str());
                            return false;
                        }

                        pAPIInfo->m_tid = tid;
                        pAPIInfo->m_uiSeqID = i;

                        if (name == "clGetEventInfo")
                        {
                            apiIndex--;
                            pAPIInfo->m_bHasDisplayableSeqId = false;
                        }
                        else
                        {
                            pAPIInfo->m_bHasDisplayableSeqId = true;
                        }

                        pAPIInfo->m_uiDisplaySeqID = apiIndex;

                        m_CLAPIInfoMap[tid].push_back(pAPIInfo);

                    }
                }

                else
                {
                    Log(logERROR, "Failed to parse API trace (%s), Unexpected data in input file.\n", line.c_str());
                    return false;
                }

            }
            else
            {
                // timestamp
                // find APIInfo object from InfoMap
                CLAPIInfo* pAPIInfo = NULL;
                std::vector<CLAPIInfo*>& apiList = m_CLAPIInfoMap[ tid ];

                if (apiList.size() <= i)
                {
                    // unmatched num of API trace items and num of timestamp items
                    // skip this timestamp
                    // This could happen in timeout mode
                    Log(logWARNING, "Unexpected data in input file. Inconsistent number of API trace items and timestamp items. Number of timestamp items is greater than number of api trace items.\n");
                    m_bWarning = true;
                    m_strWarningMsg = "[Warning]Unexpected data in input file. Incomplete summary pages may be generated.";
                    continue;
                }
                else
                {
                    pAPIInfo = apiList[i];

                    if (!ParseTimestamp(line.c_str(), pAPIInfo))
                    {
                        Log(logERROR, "Unexpected data in input file. Failed to parse timestamp entry.\n");
                        return false;
                    }
                }

                for (std::vector<IParserListener<CLAPIInfo>*>::iterator it = m_listenerList.begin(); it != m_listenerList.end() && !m_shouldStopParsing; it++)
                {
                    if (((*it) != NULL) && (pAPIInfo != NULL))
                    {
                        (*it)->OnParse(pAPIInfo, m_shouldStopParsing);
                    }
                    else
                    {
                        SpBreak("pAPIInfo == NULL");
                    }
                }
            }
        }

    }
    while (!in.eof());

    return true;
}

bool CLAtpFilePart::ParseHeader(const std::string& strKey, const std::string& strVal)
{
    SP_TODO("For each summarizer, add a dependent API list. Only if dependent APIs are filtered out should a summarizer be disabled.")

    if (strKey == "ExcludedAPIs")
    {
        m_excludedAPIs.clear();
        StringUtils::Split(m_excludedAPIs, strVal, string(","), true, true);
    }

    return true;
}

void CLAtpFilePart::SaveToFile(const std::string& strTmpFilePath, const std::string& strPID)
{
    stringstream ss;

    std::string strExtension = FileUtils::GetFileExtension(m_config.strOutputFile);

    if (strExtension != TRACE_EXT)
    {
        if (strExtension == OCCUPANCY_EXT || strExtension == PERF_COUNTER_EXT)
        {
            // strip .csv or .occupancy and append .atp
            string strBaseFileName = FileUtils::GetBaseFileName(m_config.strOutputFile);
            ss << strBaseFileName << "." << TRACE_EXT;
        }
        else
        {
            // append .atp
            ss << m_config.strOutputFile << "." << TRACE_EXT;
        }
    }
    else
    {
        // use original name
        ss << m_config.strOutputFile;
    }

    string strOutputFile = ss.str();

    SP_fileStream fout(strOutputFile.c_str());

    if (fout.fail())
    {
        cout << "Failed to write to file " << strOutputFile << endl;
        return;
    }

    fout << "TraceFileVersion=" << GPUPROFILER_BACKEND_MAJOR_VERSION << "." << GPUPROFILER_BACKEND_MINOR_VERSION << endl;
    fout << "ProfilerVersion=" << GPUPROFILER_BACKEND_MAJOR_VERSION << "." << GPUPROFILER_BACKEND_MINOR_VERSION << "." << GPUPROFILER_BACKEND_BUILD_NUMBER << endl;
    fout << "Application=" << m_config.strInjectedApp.asUTF8CharArray() << endl;
    fout << "ApplicationArgs=" << m_config.strInjectedAppArgs.asUTF8CharArray() << endl;
    fout << "WorkingDirectory=" << m_config.strWorkingDirectory.asUTF8CharArray() << endl;

    if (m_config.mapEnvVars.size() > 0)
    {
        fout << "FullEnvironment=" << (m_config.bFullEnvBlock  ? "True" : "False") << endl;

        for (EnvVarMap::const_iterator it = m_config.mapEnvVars.begin(); it != m_config.mapEnvVars.end(); ++it)
        {
            fout << "EnvVar=" << (it->first).asUTF8CharArray() << "=" << (it->second).asUTF8CharArray() << endl;
        }
    }

    fout << "UserTimer=" << (m_config.bUserTimer ? "True" : "False") << endl;

    fout << "OS Version=" << OSUtils::Instance()->GetOSInfo().c_str() << endl;
    fout << "DisplayName=" << m_config.strSessionName.c_str() << endl;

    WriteHeaderSection(fout);
    WriteContentSection(fout, strTmpFilePath, strPID);
    fout.close();
}
