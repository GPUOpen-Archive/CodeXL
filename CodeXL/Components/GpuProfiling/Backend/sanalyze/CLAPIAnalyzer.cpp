//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file defines the CLAPIAnalyzer and CLAPIAnalyzerManager specialization classes
//==============================================================================

#include <vector>
#include <string>
#include "CLAPIAnalyzer.h"
#include "../Common/StringUtils.h"
#include "../Common/Logger.h"

using namespace GPULogger;


CLAPIAnalyzer::CLAPIAnalyzer(CLAPIAnalyzerManager* pManager) : m_pParent(pManager)
{
}

CLAPIAnalyzerManager::CLAPIAnalyzerManager(void) : m_bEnableKernelArgsAnalyzer(false)
{
}

const CLKernelArgsSetupMap& CLAPIAnalyzerManager::GetKernelArgsSetupMap() const
{
    return m_KernelArgsSetupMap;
}

void CLAPIAnalyzerManager::EnableKernelArgsSetupAnalyzer()
{
    m_bEnableKernelArgsAnalyzer = true;
}

void CLAPIAnalyzerManager::AnalyzeKernelArgSetup(CLAPIInfo* pAPIInfo)
{
    if (!m_bEnableKernelArgsAnalyzer)
    {
        return;
    }

    if (pAPIInfo->m_uiAPIID == CL_FUNC_TYPE_clCreateKernel)
    {
        if (pAPIInfo->m_strRet != "NULL")
        {
            CLKernelInfo ki;
            ki.uiRefCount = 1;
            m_KernelArgsSetupMap[pAPIInfo->m_strRet] = ki;
        }
    }

    if (pAPIInfo->m_uiAPIID == CL_FUNC_TYPE_clCreateKernelsInProgram)
    {
        if (pAPIInfo->m_strRet == "CL_SUCCESS")
        {
            std::vector<std::string> output;
            StringUtils::Split(output, pAPIInfo->m_ArgList, std::string(";"));
            SpAssert(output.size() == NUM_ARG_CL_CREATE_KERNELS_IN_PROGRAM);

            if (output.size() == NUM_ARG_CL_CREATE_KERNELS_IN_PROGRAM)
            {
                if (output[2] == "NULL" || output[2].length() <= 2)
                {
                    return;
                }

                std::string strNumKernels = output[1];
                size_t numKernels = 0;
                bool ret = StringUtils::Parse(strNumKernels, numKernels);
                SpAssert(ret);

                if (!ret || numKernels == 0)
                {
                    return;
                }

                // strip out square brackets
                std::string strKernelHandleList = output[2].substr(1, output[2].length() - 2);
                std::vector<std::string> kernelHandles;
                StringUtils::Split(kernelHandles, strKernelHandleList, std::string(","));
                // User could pass in bigger number than actual number of kernels that are created
                SpAssert(kernelHandles.size() <= numKernels);

                if (kernelHandles.size() <= numKernels)
                {
                    for (size_t i = 0; i < kernelHandles.size(); ++i)
                    {
                        CLKernelInfo ki;
                        ki.uiRefCount = 1;
                        m_KernelArgsSetupMap[kernelHandles[i]] = ki;
                    }
                }
            }
        }
    }
    else if (pAPIInfo->m_uiAPIID == CL_FUNC_TYPE_clRetainKernel)
    {
        if (pAPIInfo->m_strRet == "CL_SUCCESS")
        {
            CLKernelArgsSetupMap::iterator it = m_KernelArgsSetupMap.find(pAPIInfo->m_ArgList);
            SpAssert(it != m_KernelArgsSetupMap.end());

            if (it != m_KernelArgsSetupMap.end())
            {
                it->second.uiRefCount++;
            }
        }
    }
    else if (pAPIInfo->m_uiAPIID == CL_FUNC_TYPE_clReleaseKernel)
    {
        if (pAPIInfo->m_strRet == "CL_SUCCESS")
        {
            CLKernelArgsSetupMap::iterator it = m_KernelArgsSetupMap.find(pAPIInfo->m_ArgList);
            SpAssert(it != m_KernelArgsSetupMap.end());

            if (it != m_KernelArgsSetupMap.end())
            {
                unsigned int ref = it->second.uiRefCount;

                if (ref == 1)
                {
                    // released
                    m_KernelArgsSetupMap.erase(it);
                }
                else
                {
                    it->second.uiRefCount--;
                }
            }
        }
    }

    if (pAPIInfo->m_uiAPIID == CL_FUNC_TYPE_clSetKernelArg)
    {
        // keep track of all setKernelArg
        std::vector<std::string> output;
        StringUtils::Split(output, pAPIInfo->m_ArgList, std::string(";"));
        SpAssert(output.size() == NUM_ARG_CL_SET_KERNEL_ARG);

        if (output.size() == NUM_ARG_CL_SET_KERNEL_ARG)
        {
            // output[0] is kernel handle
            CLKernelArgsSetupMap::iterator it = m_KernelArgsSetupMap.find(output[0]);
            SpAssert(it != m_KernelArgsSetupMap.end());

            if (it != m_KernelArgsSetupMap.end())
            {
                unsigned int idx = 0;

                // output[1] is index
                if (StringUtils::Parse(output[1], idx))
                {
                    it->second.kernelArgsSetup[idx] = output[3];
                }
                else
                {
                    Log(logERROR, "Failed to parse clSetKernelArg parameter. %s\n", output[1].c_str());
                }
            }
        }
    }
}

void CLAPIAnalyzerManager::OnParse(CLAPIInfo* pAPIInfo, bool& stopParsing)
{
    stopParsing = false;

    // On-the-fly analysis
    for (AnalyzerList::iterator it = m_analyzers.begin(); it != m_analyzers.end(); it++)
    {
        if ((*it)->IsEnabled())
        {
            (*it)->Analyze(pAPIInfo);
        }
    }

    // Build flattened APIs list for post-parsing analysis
    if (!m_apisToFlatten.empty())
    {
        if (m_apisToFlatten.find((CL_FUNC_TYPE)pAPIInfo->m_uiAPIID) != m_apisToFlatten.end())
        {
            m_flattenedAPIs.insert(pAPIInfo);
        }
    }
}

bool CLAPIAnalyzerManager::DoEnableAnalyzer()
{
    EnableKernelArgsSetupAnalyzer();
    return true;
}

bool CLAPIAnalyzerManager::DoEndAnalyze(APIInfo* pAPIInfo)
{
    CLAPIInfo* pCLApiInfo = dynamic_cast<CLAPIInfo*>(pAPIInfo);

    if (nullptr != pCLApiInfo)
    {
        AnalyzeKernelArgSetup(pCLApiInfo);
    }

    return true;
}
