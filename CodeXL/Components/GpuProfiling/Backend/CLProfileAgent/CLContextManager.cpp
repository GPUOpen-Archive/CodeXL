//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages the CL contexts stored.
//==============================================================================

#include <CL/opencl.h>

#include <vector>
#include "CLContextManager.h"
#include "Logger.h"

using namespace GPULogger;


CLContextManager::~CLContextManager()
{
    for (unsigned int i = 0; i < m_contextList.size(); ++i)
    {
        CLContext* tmp = m_contextList[i];

        if (tmp != NULL)
        {
            delete tmp;
        }
    }

    m_contextList.clear();
}

bool CLContextManager::LoadArena(const cl_context& context, const cl_command_queue& commandQueue, const cl_kernel& kernel)
{
    int nIndex = FindContextIndex(context);

    if (nIndex < 0)
    {
        return false;
    }

    return m_contextList[ nIndex ]->LoadArena(commandQueue, kernel);
}

bool CLContextManager::SaveArena(const cl_context& context, const cl_command_queue& commandQueue, const cl_kernel& kernel)
{
    int nIndex = FindContextIndex(context);

    if (nIndex < 0)
    {
        return false;
    }

    return m_contextList[ nIndex ]->SaveArena(commandQueue, kernel);
}

bool CLContextManager::ClearArena(const cl_context& context, const cl_kernel& kernel)
{
    int nIndex = FindContextIndex(context);

    if (nIndex < 0)
    {
        return false;
    }

    return m_contextList[ nIndex ]->ClearArena(kernel);
}

bool CLContextManager::AddContext(const cl_context& context)
{
    int nIndex = FindContextIndex(context);

    if (nIndex >= 0)
    {
        // context exists
        return false;
    }

    CLContext* pContext = new(std::nothrow) CLContext(context);

    SpAssertRet(pContext != NULL) false;

    if (pContext != NULL)
    {
        m_contextList.push_back(pContext);
    }

    return true;
}

bool CLContextManager::RemoveContext(const cl_context& context)
{
    std::vector< CLContext* >::iterator it = FindContext(context);

    if (it == m_contextList.end())
    {
        // context exists
        return false;
    }

    CLContext* pCtx = *it;
    m_contextList.erase(it);

    if (pCtx != NULL)
    {
        delete pCtx;
    }

    return true;
}

int CLContextManager::FindContextIndex(const cl_context& context)
{
    for (unsigned int i = 0; i < m_contextList.size(); ++i)
    {
        if (m_contextList[ i ]->IsEqual(context))
        {
            return i;
        }
    }

    // can't find the context
    return -1;
}

std::vector< CLContext* >::iterator CLContextManager::FindContext(const cl_context& context)
{
    std::vector< CLContext* >::iterator it;

    for (it = m_contextList.begin(); it != m_contextList.end(); ++it)
    {
        if ((*it)->IsEqual(context))
        {
            return it;
        }
    }

    // can't find the context
    return m_contextList.end();
}

bool CLContextManager::AddKernelToContext(const cl_context& context, const cl_kernel& kernel)
{
    int nIndex = FindContextIndex(context);

    if (nIndex < 0)
    {
        return false;
    }

    m_contextList[ nIndex ]->AddKernel(kernel);

    return true;
}

bool CLContextManager::RemoveKernelFromContext(const cl_context& context, const cl_kernel& kernel)
{
    int nIndex = FindContextIndex(context);

    if (nIndex < 0)
    {
        return false;
    }

    m_contextList[ nIndex ]->RemoveKernel(kernel);

    return true;
}

bool CLContextManager::AddKernelArg(const cl_context& context, const cl_kernel& kernel, cl_uint argIdx, const void* pArgValue)
{
    int nIndex = FindContextIndex(context);

    if (nIndex < 0)
    {
        return false;
    }

    return m_contextList[ nIndex ]->AddKernelArg(kernel, argIdx, pArgValue);
}

bool CLContextManager::AddKernelArgSVMPointer(const cl_context& context, const cl_kernel kernel, cl_uint argIdx)
{
    bool retVal = false;
    int nIndex = FindContextIndex(context);

    if (nIndex >= 0)
    {
        retVal = m_contextList[ nIndex ]->AddKernelArgSVMPointer(kernel, argIdx);
    }

    return retVal;
}

bool CLContextManager::HasKernelArgSVMPointer(const cl_context& context, const cl_kernel kernel)
{
    bool retVal = false;
    int nIndex = FindContextIndex(context);

    if (nIndex >= 0)
    {
        retVal = m_contextList[ nIndex ]->HasKernelArgSVMPointer(kernel);
    }

    return retVal;
}

bool CLContextManager::AddKernelArgPipe(const cl_context& context, const cl_kernel kernel, cl_uint argIdx)
{
    bool retVal = false;
    int nIndex = FindContextIndex(context);

    if (nIndex >= 0)
    {
        retVal = m_contextList[nIndex]->AddKernelArgPipe(kernel, argIdx);
    }

    return retVal;
}

bool CLContextManager::HasKernelArgPipe(const cl_context& context, const cl_kernel kernel)
{
    bool retVal = false;
    int nIndex = FindContextIndex(context);

    if (nIndex >= 0)
    {
        retVal = m_contextList[nIndex]->HasKernelArgPipe(kernel);
    }

    return retVal;
}

bool CLContextManager::AddBufferToContext(const cl_context& context,
                                          const cl_mem&     buffer,
                                          cl_mem_flags      flags,
                                          size_t            bufferSize,
                                          void*             pHost)
{
    int nIndex = FindContextIndex(context);

    if (nIndex < 0)
    {
        return false;
    }

    m_contextList[ nIndex ]->AddBuffer(buffer, flags, bufferSize, pHost);

    return true;
}

bool CLContextManager::AddSubBuffer(const cl_mem& parentBuffer,
                                    const cl_mem& subBuffer,
                                    cl_mem_flags  flags,
                                    size_t        bufferSize)
{
    bool retVal = false;

    // find the context that created the parent buffer, and then add the sub buffer to it
    for (std::vector<CLContext*>::iterator it = m_contextList.begin(); it != m_contextList.end(); ++it)
    {
        if ((*it)->HasBuffer(parentBuffer))
        {
            (*it)->AddBuffer(subBuffer, flags, bufferSize, NULL);
            retVal = true;
            break;
        }
    }

    return retVal;
}

bool CLContextManager::AddPipeToContext(const cl_context context, const cl_mem pipe)
{
    int nIndex = FindContextIndex(context);

    if (nIndex < 0)
    {
        return false;
    }

    m_contextList[nIndex]->AddPipe(pipe);

    return true;
}

bool CLContextManager::RemoveBuffer(const cl_context& context,
                                    const cl_mem&     buffer)
{
    int nIndex = FindContextIndex(context);

    if (nIndex < 0)
    {
        return false;
    }

    return m_contextList[ nIndex ]->RemoveBuffer(buffer);
}
