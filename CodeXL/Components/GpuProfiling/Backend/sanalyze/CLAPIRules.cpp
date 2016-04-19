//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file defines the callbacks for the simple rules
//==============================================================================

#include <string>
#include <vector>
#include <sstream>
#include "CLAPIRules.h"
#include "../Common/StringUtils.h"
#include "../Common/Logger.h"
#include "../CLCommon/CLFunctionEnumDefs.h"

using namespace std;
using namespace GPULogger;

#define PREFERRED_GLOBAL_WORK_SIZE 2560 //40 waves
#define PREFERRED_GROUP_WORK_SIZE 64
#define PREFERRED_GROUP_WORK_SIZE_STR "64"

static SimpleCLAPIRuleManager ruleMgr(NULL);

// Blocking write
bool CALLBACK CLAPIRule_BlockingWrite(CLAPIInfo* info, std::string& strMsg, APIAnalyzerMessageType& type)
{
    vector<string> args;
    StringUtils::Split(args, info->m_ArgList, string(";"), true, true);

    if (args.size() == 9 || args.size() == 14 || args.size() == 11)   // write buffer || write bufferRect || writeImage
    {
        CreateCmdQueueInfo* pCmdQInfoList = ruleMgr.GetCmdQueueInfoList()->FindInCmdQInfoList(args[0]);

        if ((args[2] == "CL_TRUE") &&
            (pCmdQInfoList != NULL) &&
            (std::string::npos == pCmdQInfoList->strProp.find("CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE")))
        {
            type = MSGTYPE_BestPractices;
            strMsg = info->m_strName + ": Unnecessary synchronization: Blocking write.";
            return true;
        }
    }
    else
    {
        Log(logERROR, "Failed to parse api list - %s\n", info->m_ArgList.c_str());
    }

    return false;
}

// Bad work size
bool CALLBACK CLAPIRule_BadWorkSize(CLAPIInfo* info, std::string& strMsg, APIAnalyzerMessageType& type)
{
    CLKernelAPIInfo* kInfo = dynamic_cast<CLKernelAPIInfo*>(info);
    SpAssertRet(NULL != kInfo) false;

    if (kInfo->m_bInfoMissing)
    {
        return false;
    }

    if (kInfo->m_strDevice.find("CPU") != string::npos)
    {
        // this doesn't apply to CPU
        return false;
    }

    stringstream ss;

    vector<string> workDims;
    StringUtils::Split(workDims, kInfo->m_strGlobalWorkSize, " ,{}", true, true);

    bool bParseFlag = true;
    int product = 1;
    int tmp = 0;

    for (size_t i = 0; i < workDims.size() && bParseFlag; i++)
    {
        if (!StringUtils::Parse(workDims[i], tmp))
        {
            bParseFlag = false;
        }
        else
        {
            product *= tmp;
        }
    }

    if (bParseFlag)
    {
        if (product < PREFERRED_GLOBAL_WORK_SIZE)
        {
            type = MSGTYPE_BestPractices;
            ss << info->m_strName;
            ss << ": Global Work size is too small - ";
            ss << kInfo->m_strGlobalWorkSize;
            ss << ", resulting in low GPU utilization.";
            strMsg = ss.str();
            return true;
        }

        // else, global is good, take a look at Work-group size
    }
    else
    {
        Log(logERROR, "Failed to parse global work size - %s\n", kInfo->m_strGlobalWorkSize.c_str());
        return false;
    }

    workDims.clear();
    StringUtils::Split(workDims, kInfo->m_strGroupWorkSize, " ,{}", true, true);

    product = 1;

    for (size_t i = 0; i < workDims.size() && bParseFlag; i++)
    {
        if (!StringUtils::Parse(workDims[i], tmp))
        {
            bParseFlag = false;
        }
        else
        {
            product *= tmp;
        }
    }

    if (bParseFlag)
    {
        if (product < PREFERRED_GROUP_WORK_SIZE)
        {
            type = MSGTYPE_BestPractices;
            ss << info->m_strName;
            ss << ": Work-group size is too small - ";
            ss << kInfo->m_strGroupWorkSize;
            ss << ". Recommended Work-group size is a multiple of " PREFERRED_GROUP_WORK_SIZE_STR ".";
            strMsg = ss.str();
            return true;
        }
        else if (product % PREFERRED_GROUP_WORK_SIZE != 0)
        {
            type = MSGTYPE_BestPractices;
            ss << info->m_strName;
            ss << ": Work-group size(";
            ss << kInfo->m_strGroupWorkSize;
            ss << ") is not a multiple of " PREFERRED_GROUP_WORK_SIZE_STR ". Recommended Work-group size is a multiple of" PREFERRED_GROUP_WORK_SIZE_STR ".";
            strMsg = ss.str();
            return true;
        }
        else
        {
            // all good
            return false;
        }
    }

    Log(logERROR, "Failed to parse work group size - %s\n", kInfo->m_strGroupWorkSize.c_str());

    return false;
}

SimpleCLAPIRuleManager* GetSimpleCLAPIRuleManager()
{
    ruleMgr.AddRule("BlockingWrite", CL_FUNC_TYPE_clEnqueueWriteBuffer, CLAPIRule_BlockingWrite);
    ruleMgr.AddRule("BlockingWrite", CL_FUNC_TYPE_clEnqueueWriteBufferRect, CLAPIRule_BlockingWrite);
    ruleMgr.AddRule("BlockingWrite", CL_FUNC_TYPE_clEnqueueWriteImage, CLAPIRule_BlockingWrite);
    ruleMgr.AddRule("BadWorkGroupSize", CL_FUNC_TYPE_clEnqueueNDRangeKernel, CLAPIRule_BadWorkSize);
    ruleMgr.AddRule("BadWorkGroupSize", CL_FUNC_TYPE_clEnqueueTask, CLAPIRule_BadWorkSize);

    return &ruleMgr;
}
