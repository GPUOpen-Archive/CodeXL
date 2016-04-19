//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This analyzer class analyzes opencl return codes; reports errors for failures
//==============================================================================

#include <sstream>
#include "CLRetCodeAnalyzer.h"
#include "../Common/StringUtils.h"

static const char* s_szSUCCESS = "CL_SUCCESS";
static const char* s_szNULL = "NULL";

using namespace GPULogger;

CLRetCodeAnalyzer::CLRetCodeAnalyzer(CLAPIAnalyzerManager* pManager) : CLAPIAnalyzer(pManager)
{
    m_strName = "RetCodeAnalyzer";
    SP_TODO("Add dependent APIs")

    // the following are the OCL APIs that return errors in a parameter, rather than the return value
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateContext] = 5;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateContextFromType] = 4;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateCommandQueue] = 3;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateBuffer] = 4;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateSubBuffer] = 4;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateImage2D] = 7;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateImage3D] = 9;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateSampler] = 4;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateProgramWithSource] = 4;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateProgramWithBinary] = 6;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateKernel] = 2;
    m_retCodeArgMap[CL_FUNC_TYPE_clEnqueueMapBuffer] = 9;
    m_retCodeArgMap[CL_FUNC_TYPE_clEnqueueMapImage] = 11;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateFromGLBuffer] = 3;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateFromGLTexture2D] = 5;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateFromGLTexture3D] = 5;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateFromGLRenderbuffer] = 3;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateEventFromGLsyncKHR] = 2;
#ifdef _WIN32
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateFromD3D10BufferKHR] = 3;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateFromD3D10Texture2DKHR] = 4;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateFromD3D10Texture3DKHR] = 4;
#endif
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateImage] = 5;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateProgramWithBuiltInKernels] = 4;
    m_retCodeArgMap[CL_FUNC_TYPE_clLinkProgram] = 8;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateFromGLTexture] = 5;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateCommandQueueWithProperties] = 3;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreateSamplerWithProperties] = 2;
    m_retCodeArgMap[CL_FUNC_TYPE_clCreatePipe] = 5;
}

CLRetCodeAnalyzer::~CLRetCodeAnalyzer(void)
{
}

void CLRetCodeAnalyzer::FlattenedAPIAnalyze(APIInfo* pAPIInfo)
{
    SP_UNREFERENCED_PARAMETER(pAPIInfo);
    return;
}

void CLRetCodeAnalyzer::Clear()
{
    m_msgList.clear();
}

void CLRetCodeAnalyzer::Analyze(APIInfo* pAPIInfo)
{
    CLAPIInfo* pCLApiInfo = dynamic_cast<CLAPIInfo*>(pAPIInfo);

    if (nullptr != pCLApiInfo)
    {
        std::string retCode = pCLApiInfo->m_strRet;
        CL_FUNC_TYPE apiType = static_cast<CL_FUNC_TYPE>(pCLApiInfo->m_uiAPIID);

        bool noRetCode = false;

        if (m_retCodeArgMap.count(apiType) > 0)
        {
            std::vector<std::string> apiArgs;
            StringUtils::Split(apiArgs, pCLApiInfo->m_ArgList, ";", true);
            unsigned int argIndex = m_retCodeArgMap[apiType];

            if (apiArgs.size() < argIndex)
            {
                Log(logERROR, "CLRetCodeAnalyzer: Unable to check ret code of API: ", pCLApiInfo->m_uiAPIID);
                return;
            }

            retCode = apiArgs[argIndex];

            if (0 == retCode.compare(s_szNULL))
            {
                noRetCode = true;
            }
        }

        if (!noRetCode && (retCode.find(s_szSUCCESS) == std::string::npos))
        {
            // error found
            std::stringstream ss;
            ss << pCLApiInfo->m_strName << " returns " << retCode;
            APIAnalyzerMessage msg;
            msg.type = MSGTYPE_Error;
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

void CLRetCodeAnalyzer::EndAnalyze()
{
}
