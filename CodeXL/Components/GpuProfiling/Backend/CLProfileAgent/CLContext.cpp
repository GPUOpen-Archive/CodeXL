//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages all the information in a CL context.
//==============================================================================

#include <CL/opencl.h>
#include <stdlib.h>

#include "CLContext.h"

CLContext::~CLContext()
{
    m_context = NULL;

    for (unsigned int i = 0; i < m_kernelList.size(); ++i)
    {
        CLKernel* pTmpKernel = m_kernelList[i];

        if (pTmpKernel != NULL)
        {
            delete pTmpKernel;
        }
    }

    m_kernelList.clear();

    for (unsigned int i = 0; i < m_bufferList.size(); ++i)
    {
        CLBuffer* pTmpBuffer = m_bufferList[ i ];

        if (pTmpBuffer != NULL)
        {
            delete pTmpBuffer;
        }
    }

    m_bufferList.clear();
}

bool CLContext::IsEqual(const cl_context& context) const
{
    return (m_context == context);
}

int CLContext::FindKernelIndex(const cl_kernel& kernel) const
{
    for (unsigned int i = 0; i < m_kernelList.size(); ++i)
    {
        if (m_kernelList[ i ]->IsKernel(kernel))
        {
            return i;
        }
    }

    // can't find the context
    return -1;
}

int CLContext::FindBufferIndex(const cl_mem& buffer) const
{
    for (unsigned int i = 0; i < m_bufferList.size(); ++i)
    {
        if (m_bufferList[ i ]->IsEqual(buffer))
        {
            return i;
        }
    }

    // can't find the context
    return -1;
}

int CLContext::FindPipeIndex(const cl_mem& pipe) const
{
    for (unsigned int i = 0; i < m_pipeList.size(); ++i)
    {
        if (m_pipeList[i] == pipe)
        {
            return i;
        }
    }

    // can't find the context
    return -1;
}

void CLContext::AddKernel(const cl_kernel& kernel)
{
    CLKernel* pClkernel = new(std::nothrow) CLKernel(kernel);

    if (pClkernel != NULL)
    {
        m_kernelList.push_back(pClkernel);
    }
}

void CLContext::RemoveKernel(const cl_kernel& kernel)
{
    int kernelIndex = FindKernelIndex(kernel);

    if (kernelIndex != -1)
    {
        CLKernel* pClKernel = m_kernelList[kernelIndex];
        m_kernelList.erase(m_kernelList.begin() + kernelIndex);

        if (pClKernel != NULL)
        {
            delete pClKernel;
        }
    }
}

bool CLContext::AddKernelArg(const cl_kernel& kernel, cl_uint argIdx, const void* pArgValue)
{
    bool retVal = false;
    int nKernelIndex = FindKernelIndex(kernel);

    if (nKernelIndex >= 0)
    {
        // tell the kernel to remove an SVM kernel arg at this index (if one exists),
        // in case this index was previously flagged as an SVM Pointer
        m_kernelList[ nKernelIndex ]->RemoveKernelArgSVMPointerOrPipe(argIdx);

        const cl_mem* pBuffer = (cl_mem*)pArgValue;

        if (NULL != pBuffer)
        {
            int nBufferIndex = FindBufferIndex(*pBuffer);

            if (nBufferIndex >= 0)
            {
                m_kernelList[ nKernelIndex ]->AddKernelBufferArg(argIdx, m_bufferList[ nBufferIndex ]);
                retVal = true;
            }
            else
            {
                nBufferIndex = FindPipeIndex(*pBuffer);

                if (nBufferIndex >= 0)
                {
                    m_kernelList[nKernelIndex]->AddKernelArgPipe(argIdx);
                    retVal = true;
                }
            }
        }
    }

    return retVal;
}

bool CLContext::AddKernelArgSVMPointer(const cl_kernel& kernel, cl_uint argIdx)
{
    bool retVal = false;

    int nKernelIndex = FindKernelIndex(kernel);

    if (nKernelIndex >= 0)
    {
        m_kernelList[ nKernelIndex ]->AddKernelArgSVMPointer(argIdx);
        retVal = true;
    }

    return retVal;
}

bool CLContext::HasKernelArgSVMPointer(const cl_kernel kernel) const
{
    bool retVal = false;

    int nKernelIndex = FindKernelIndex(kernel);

    if (nKernelIndex >= 0)
    {
        retVal = m_kernelList[ nKernelIndex ]->HasKernelArgSVMPointer();
    }

    return retVal;
}

bool CLContext::AddKernelArgPipe(const cl_kernel& kernel, cl_uint argIdx)
{
    bool retVal = false;

    int nKernelIndex = FindKernelIndex(kernel);

    if (nKernelIndex >= 0)
    {
        m_kernelList[nKernelIndex]->AddKernelArgPipe(argIdx);
        retVal = true;
    }

    return retVal;
}

bool CLContext::HasKernelArgPipe(const cl_kernel kernel) const
{
    bool retVal = false;

    int nKernelIndex = FindKernelIndex(kernel);

    if (nKernelIndex >= 0)
    {
        retVal = m_kernelList[nKernelIndex]->HasKernelArgPipe();
    }

    return retVal;
}

void CLContext::AddBuffer(const cl_mem& buffer,
                          cl_mem_flags  flags,
                          size_t        bufferSize,
                          void*         pHost)
{
    CLBuffer* pBuffer = new(std::nothrow) CLBuffer(buffer, bufferSize, flags, pHost);

    if (pBuffer != NULL)
    {
        m_bufferList.push_back(pBuffer);
    }
}

void CLContext::AddPipe(const cl_mem& pipe)
{
    m_pipeList.push_back(pipe);
}

bool CLContext::RemoveBuffer(const cl_mem& buffer)
{
    bool found = false;
    std::vector< CLBuffer* >::iterator it;

    for (it = m_bufferList.begin(); it != m_bufferList.end(); ++it)
    {
        if ((*it)->IsEqual(buffer))
        {
            found = true;
            break;
        }
    }

    if (found)
    {
        CLBuffer* pBuffer = *it;
        m_bufferList.erase(it);

        // Clean up kernel arg setup
        for (unsigned int i = 0; i < m_kernelList.size(); ++i)
        {
            m_kernelList[i]->RemoveKernelArg(buffer);
        }

        if (pBuffer != NULL)
        {
            delete pBuffer;
        }

        return true;
    }
    else
    {
        std::vector< cl_mem >::iterator pipeIt;
        found = false;

        for (pipeIt = m_pipeList.begin(); pipeIt != m_pipeList.end(); ++pipeIt)
        {
            if (*pipeIt == buffer)
            {
                found = true;
                break;
            }
        }

        if (found)
        {
            m_pipeList.erase(pipeIt);
        }

        return false;
    }
}

bool CLContext::HasBuffer(const cl_mem& buffer)
{
    bool found = false;

    for (std::vector<CLBuffer*>::iterator it = m_bufferList.begin(); it != m_bufferList.end(); ++it)
    {
        if ((*it)->IsEqual(buffer))
        {
            found = true;
            break;
        }
    }

    return found;
}

bool CLContext::LoadArena(const cl_command_queue& commandQueue, const cl_kernel& kernel)
{
    int nKernelIndex = FindKernelIndex(kernel);

    if (nKernelIndex < 0)
    {
        return false;
    }

    return m_kernelList[ nKernelIndex ]->LoadArena(commandQueue);
}

bool CLContext::SaveArena(const cl_command_queue& commandQueue, const cl_kernel& kernel)
{
    int nKernelIndex = FindKernelIndex(kernel);

    if (nKernelIndex < 0)
    {
        return false;
    }

    return m_kernelList[ nKernelIndex ]->SaveArena(commandQueue);
}

bool CLContext::ClearArena(const cl_kernel& kernel)
{
    int nKernelIndex = FindKernelIndex(kernel);

    if (nKernelIndex < 0)
    {
        return false;
    }

    return m_kernelList[ nKernelIndex ]->ClearArena();
}
