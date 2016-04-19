//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages all the information in a CL kernel.
//==============================================================================

#include <CL/opencl.h>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>
#include <string.h>  // memcpy
#include "CLKernel.h"
#include "CLFunctionDefs.h"
#include "../Common/Logger.h"

using namespace GPULogger;

CLKernel::~CLKernel()
{
    ClearArgBufferHostList();
    m_kernel = NULL;
}


bool CLKernel::IsKernel(const cl_kernel& kernel) const
{
    return (m_kernel == kernel);
}

void CLKernel::AddKernelBufferArg(cl_uint argIdx, const CLBuffer* buffer)
{
    CLKernelArgMap::iterator it = m_kernelArgBufferMap.find(argIdx);

    if (it != m_kernelArgBufferMap.end())
    {
        // check mem binding
        if (it->second->GetBuffer() != buffer->GetBuffer())
        {
            // replace entry
            m_kernelArgBufferMap[ argIdx ] = buffer;
        }
    }
    else
    {
        // new binding
        m_kernelArgBufferMap[ argIdx ] = buffer;
    }
}

void CLKernel::AddKernelArgSVMPointer(cl_uint argIdx)
{
    if (std::find(m_svmPointerArgIndices.begin(), m_svmPointerArgIndices.end(), argIdx) == m_svmPointerArgIndices.end())
    {
        m_svmPointerArgIndices.push_back(argIdx);
    }
}

void CLKernel::RemoveKernelArgSVMPointerOrPipe(cl_uint argIdx)
{
    std::vector<cl_uint>::iterator svmIt = std::find(m_svmPointerArgIndices.begin(), m_svmPointerArgIndices.end(), argIdx);

    if (svmIt != m_svmPointerArgIndices.end())
    {
        m_svmPointerArgIndices.erase(svmIt);
    }

    std::vector<cl_uint>::iterator pipeIt = std::find(m_pipeArgIndices.begin(), m_pipeArgIndices.end(), argIdx);

    if (pipeIt != m_pipeArgIndices.end())
    {
        m_pipeArgIndices.erase(pipeIt);
    }
}

bool CLKernel::HasKernelArgSVMPointer() const
{
    return m_svmPointerArgIndices.size() > 0;
}

void CLKernel::AddKernelArgPipe(cl_uint argIdx)
{
    if (std::find(m_pipeArgIndices.begin(), m_pipeArgIndices.end(), argIdx) == m_pipeArgIndices.end())
    {
        m_pipeArgIndices.push_back(argIdx);
    }
}

bool CLKernel::HasKernelArgPipe() const
{
    return m_pipeArgIndices.size() > 0;
}

bool CLKernel::RemoveKernelArg(const cl_mem memobj)
{
    CLKernelArgMap::iterator toFind = m_kernelArgBufferMap.end();

    for (CLKernelArgMap::iterator it = m_kernelArgBufferMap.begin(); it != m_kernelArgBufferMap.end(); it++)
    {
        if (it->second->GetBuffer() == memobj)
        {
            toFind = it;
            break;
        }
    }

    if (toFind != m_kernelArgBufferMap.end())
    {
        m_kernelArgBufferMap.erase(toFind->first);
        return true;
    }
    else
    {
        return false;
    }
}

bool CLKernel::LoadArena(const cl_command_queue& commandQueue)
{
    cl_int status = 0;

    for (CLKernelArgMap::iterator it = m_kernelArgBufferMap.begin(); it != m_kernelArgBufferMap.end(); it++)
    {
        const CLBuffer* buffer = it->second;

        char* pHost = NULL;
        CLKernelArgBufferHostMap::iterator hostIt = m_kernelArgBufferHostMap.find(it->first);

        if (hostIt != m_kernelArgBufferHostMap.end())
        {
            pHost = hostIt->second;
        }
        else
        {
            // we didn't find host mem(created in SaveArena) for this cl_mem object, shouldn't happen unless there is a bug
            Log(logERROR, "CLKernel::LoadArena - Backup buffer missing.\n");
            continue;
        }

        status |= g_realDispatchTable.EnqueueWriteBuffer(commandQueue,
                                                         buffer->GetBuffer(),
                                                         CL_TRUE,
                                                         0,
                                                         buffer->GetBufferSize(),
                                                         pHost,
                                                         0,
                                                         NULL,
                                                         NULL);
    }

    if (CL_SUCCESS != status)
    {
        return false;
    }

    return true;
}

bool CLKernel::SaveArena(const cl_command_queue& commandQueue)
{
    cl_int status = 0;

    // Delete existing backup buffers which are created in last kernel dispatch
    ClearArgBufferHostList();

    for (CLKernelArgMap::iterator it = m_kernelArgBufferMap.begin(); it != m_kernelArgBufferMap.end(); it++)
    {
        const CLBuffer* buffer = it->second;

        char* pHost = new(std::nothrow) char[ buffer->GetBufferSize() ];

        if (NULL != pHost)
        {
            status |= g_realDispatchTable.EnqueueReadBuffer(commandQueue,
                                                            buffer->GetBuffer(),
                                                            CL_TRUE,
                                                            0,
                                                            buffer->GetBufferSize(),
                                                            pHost,
                                                            0,
                                                            NULL,
                                                            NULL);
        }

        if (CL_SUCCESS == status)
        {
            // add or replace backup buffer
            m_kernelArgBufferHostMap[ it->first ] = pHost;
        }
    }

    if (CL_SUCCESS != status)
    {
        return false;
    }

    return true;
}

void CLKernel::ClearArgBufferHostList()
{
    // Delete all buffers
    for (CLKernelArgBufferHostMap::iterator it = m_kernelArgBufferHostMap.begin(); it != m_kernelArgBufferHostMap.end(); it++)
    {
        if (NULL != it->second)
        {
            delete[] it->second;
            it->second = NULL;
        }
    }
}
