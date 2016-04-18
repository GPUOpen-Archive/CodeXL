//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file provides the CL Deprecated Function Analyzer
//==============================================================================

#include <sstream>
#include "CLDeprecatedFunctionAnalyzer.h"

using namespace std;

CLDeprecatedFunctionAnalyzer::CLDeprecatedFunctionAnalyzer(CLAPIAnalyzerManager* p) : CLAPIAnalyzer(p)
{
    m_strName = "DeprecatedFunctionAnalyzer";
    SP_TODO("Add dependent APIs");

    m_deprecatedFunctionMap[CL_FUNC_TYPE_clSetCommandQueueProperty]     = "It is suggested that the command queue properties are set when creating the queue.";
    m_deprecatedFunctionMap[CL_FUNC_TYPE_clCreateImage2D]               = "It is suggested that \"clCreateImage\" is used instead, since it now supports creating 2D images.";
    m_deprecatedFunctionMap[CL_FUNC_TYPE_clCreateImage3D]               = "It is suggested that \"clCreateImage\" is used instead, since it now supports creating 3D images.";
    m_deprecatedFunctionMap[CL_FUNC_TYPE_clEnqueueMarker]               = "It is suggested that \"clEnqueueMarkerWithWaitList\" is used instead.";
    m_deprecatedFunctionMap[CL_FUNC_TYPE_clEnqueueWaitForEvents]        = "It is suggested that \"clEnqueueMarkerWithWaitList\" or \"clEnqueueBarrierWithWaitList\" is used instead.";
    m_deprecatedFunctionMap[CL_FUNC_TYPE_clEnqueueBarrier]              = "It is suggested that \"clEnqueueBarrierWithWaitList\" is used instead.";
    m_deprecatedFunctionMap[CL_FUNC_TYPE_clUnloadCompiler]              = "It is suggested that \"clUnloadCompilerForPlatform\" is used instead.";
    m_deprecatedFunctionMap[CL_FUNC_TYPE_clGetExtensionFunctionAddress] = "It is suggested that \"clGetExtensionFunctionAddressForPlatform\" is used instead.";
    m_deprecatedFunctionMap[CL_FUNC_TYPE_clCreateFromGLTexture2D]       = "It is suggested that \"clCreateFromGLTexture\" is used instead, since it now supports creating 2D images.";
    m_deprecatedFunctionMap[CL_FUNC_TYPE_clCreateFromGLTexture3D]       = "It is suggested that \"clCreateFromGLTexture\" is used instead, since it now supports creating 3D images.";
    m_deprecatedFunctionMap[CL_FUNC_TYPE_clEnqueueTask]                 = "It is suggested that \"clEnqueueNDRangeKernel\" is used instead.";
    m_deprecatedFunctionMap[CL_FUNC_TYPE_clCreateCommandQueue]          = "It is suggested that \"clCreateCommandQueueWithProperties\" is used instead.";
    m_deprecatedFunctionMap[CL_FUNC_TYPE_clCreateSampler]               = "It is suggested that \"clCreateSamplerWithProperties\" is used instead.";
}

CLDeprecatedFunctionAnalyzer::~CLDeprecatedFunctionAnalyzer(void)
{
}

void CLDeprecatedFunctionAnalyzer::FlattenedAPIAnalyze(APIInfo* pAPIInfo)
{
    SP_UNREFERENCED_PARAMETER(pAPIInfo);
    return;
}

void CLDeprecatedFunctionAnalyzer::Clear()
{
    m_msgList.clear();
}

void CLDeprecatedFunctionAnalyzer::Analyze(APIInfo* pAPIInfo)
{
    CLAPIInfo* pCLApiInfo = dynamic_cast<CLAPIInfo*>(pAPIInfo);

    if (nullptr != pCLApiInfo)
    {
        std::map<int, std::string>::const_iterator it = m_deprecatedFunctionMap.find(pCLApiInfo->m_uiAPIID);

        if (it != m_deprecatedFunctionMap.end())
        {
            // deprecated API found
            stringstream ss;
            ss << pCLApiInfo->m_strName << " is deprecated. " << it->second;
            APIAnalyzerMessage msg;
            msg.type = MSGTYPE_Warning;
            msg.uiSeqID = pCLApiInfo->m_uiSeqID;
            msg.uiDisplaySeqID = pCLApiInfo->m_uiDisplaySeqID;
            msg.bHasDisplayableSeqId = pCLApiInfo->m_bHasDisplayableSeqId;
            msg.uiTID = pCLApiInfo->m_tid;
            msg.strMsg = ss.str();
            m_msgList.push_back(msg);
            return;
        }
    }
}

void CLDeprecatedFunctionAnalyzer::EndAnalyze()
{
}
