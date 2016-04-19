//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#include <new>
#include "DCCommandRecorder.h"
#include "DCStatusKeeper.h"
#include "DCUtils.h"
#include "..\Common\Logger.h"

using std::queue;
using std::map;
using std::nothrow;
using namespace GPULogger;

DCCommandBuffer::DCCommandBuffer()
{
    m_pDynMem = NULL;
}

DCCommandBuffer::~DCCommandBuffer()
{
    // clear command list just in case
    ClearCommandList();

    while (!m_qBackupBufferQueue.empty())
    {
        // dealloc remaining mem just  in case
        if (m_qBackupBufferQueue.front().m_pBuffer)
        {
            free(m_qBackupBufferQueue.front().m_pBuffer);
        }

        m_qBackupBufferQueue.pop();
    }
}

void DCCommandBuffer::ClearCommandList()
{
    while (!m_CMDList.empty())
    {
        DC_CMDBase* cmd = m_CMDList.front();
        assert(cmd != NULL);
        m_CMDList.pop();
        delete cmd;
    }
}

void DCCommandBuffer::AddToCommandList(DC_CMDBase* cmd)
{
    assert(cmd != NULL);
#if defined(_DEBUG)||defined(AUTO_TEST)
    DWORD dwTID;
    dwTID = GetCurrentThreadId();
    cmd->m_dwThreadId = dwTID;
#endif
    m_CMDList.push(cmd);
}

void DCCommandBuffer::BeginEnqueueBackBuffer(void* pBuf)
{
    BackupBuffer bb;
    bb.m_pTmpBuffer = pBuf;
    m_qBackupBufferQueue.push(bb);
}

void DCCommandBuffer::EndEnqueueBackBuffer(ID3D11Resource* pRes)
{
    BackupBuffer* bb = &m_qBackupBufferQueue.back();
    UINT size;
    void* pBuf = NULL;
    DCUtils::CopyBuffer(pRes, bb->m_pTmpBuffer, &pBuf, size);
    assert(pBuf != NULL);
    bb->m_pBuffer = pBuf;
    bb->m_uiSize = size;
}

void DCCommandBuffer::DequeueBackBuffer(void** ppBuf, UINT& size)
{
    BackupBuffer bb = m_qBackupBufferQueue.front();
    *ppBuf = bb.m_pBuffer;
    size = bb.m_uiSize;
    m_qBackupBufferQueue.pop();
}

void DCCommandBuffer::ExecuteCommandList(ID3D11DeviceContext* pImmediateContext)
{
    while (!m_CMDList.empty())
    {
        DC_CMDBase* cmd = m_CMDList.front();
        assert(cmd != NULL);
#if 0
        LogTrace(traceMESSAGE, "[%lu]Command: %s", cmd->m_dwThreadId, cmd->ToString().c_str());
#endif
        cmd->Play(pImmediateContext);
        m_CMDList.pop();
        delete cmd;
    }

    ClearCommandList();
}

DCCommandList* DCCommandBuffer::DebugCommandList()
{
    return &m_CMDList;
}

CommandRecorder::CommandRecorder(void)
{
    m_pmutex = new AMDTMutex("CommandRecorder Mutex");
    m_pImmediateDeviceContext = NULL;
    m_pID3D11Device = NULL;
}

CommandRecorder::~CommandRecorder(void)
{
    for (ContextMap::iterator it = m_DeferredContextMap.begin(); it != m_DeferredContextMap.end(); it++)
    {
        if (it->second)
        {
            delete it->second;
        }
    }

    m_DeferredContextMap.clear();

    if (m_pmutex)
    {
        delete m_pmutex;
    }
}

void CommandRecorder::AddDeferredDeviceContext(const ID3D11DeviceContext* pDeferredDeviceContext)
{
    AMDTScopeLock lock(m_pmutex);
    ContextMap::iterator it = m_DeferredContextMap.find(pDeferredDeviceContext);

    if (it == m_DeferredContextMap.end())
    {
        DCCommandBuffer* pContext = new DCCommandBuffer();
        m_DeferredContextMap.insert(ContextMapPair(pDeferredDeviceContext, pContext));
    }
    else
    {
        //shouldn't happen
        assert(!"Existing deferred context found");
    }
}

void CommandRecorder::CreateFlattenedCommandList(const ID3D11DeviceContext* pDeferredDeviceContext, ID3D11CommandList* pCommandList)
{
    AMDTScopeLock lock(m_pmutex);
    ContextMap::iterator it = m_DeferredContextMap.find(pDeferredDeviceContext);

    if (it == m_DeferredContextMap.end())
    {
        assert(!"Deferred context missing");
    }
    else
    {
        // Create an new empty command list for deferred context, pair its current command list with ID3D11CommandList that just created
        // Remark:  deferred context device maintains commands called on it, ID3D11DeviceContext::FinishCommandList creates a ID3D11CommandList
        //          that contains the commands called before, deferred context's own command buffer init to empty
        //          When ID3D11DeviceContext::ExecuteCommandList finished, it->second must be deleted!
        m_CommandListMap.insert(CommandListMapPair(pCommandList, it->second));
        it->second = new DCCommandBuffer();
    }
}

void CommandRecorder::AddToCommandList(const ID3D11DeviceContext* pDeferredDeviceContext, DC_CMDBase* cmd)
{
    ContextMap::iterator it = m_DeferredContextMap.find(pDeferredDeviceContext);

    if (it == m_DeferredContextMap.end())
    {
        assert(!"Deferred context missing");
    }
    else
    {
        it->second->AddToCommandList(cmd);
    }
}

void CommandRecorder::ExecuteCommands(ID3D11CommandList* pCommandList, BOOL RestoreContextState)
{
    //CommandListDeviceContextMap::iterator it = m_CommandListDeviceContextMap.find(pCommandList);
    CommandListMap::iterator it = m_CommandListMap.find(pCommandList);

    if (it == m_CommandListMap.end())
    {
        SpAssert(!"CommandListMapPair missing");
    }
    else
    {
        DX11Status* pDXState = NULL;

        if (RestoreContextState)
        {
            // Save context
            pDXState = new(nothrow)DX11Status();

            if (pDXState != NULL)
            {
                pDXState->Capture(m_pID3D11Device);
            }
            else
            {
                SpAssert(!"Failed to initialize DX11Status");
            }
        }

        // clear state before execution
        m_pImmediateDeviceContext->ClearState();

        it->second->ExecuteCommandList(m_pImmediateDeviceContext);

        if (RestoreContextState)
        {
            // Restore to saved context
            if (pDXState != NULL)
            {
                pDXState->RestoreAndRelease();
                delete pDXState;
            }
            else
            {
                Log(logERROR, "Failed to restore current context(pDXState == NULL)\n");
            }
        }
        else
        {
            // clear state after execution
            m_pImmediateDeviceContext->ClearState();
        }

        // remove CommandListMap pair
        // destroy DCCommandBuffer
        delete it->second;
        m_CommandListMap.erase(it);
    }
}

DCCommandList* CommandRecorder::DebugCommandList(ID3D11CommandList* pCommandList)
{
    CommandListMap::iterator it = m_CommandListMap.find(pCommandList);

    if (it == m_CommandListMap.end())
    {
        SpAssert(!"CommandListMapPair missing");
    }
    else
    {
        return it->second->DebugCommandList();
    }

    return NULL;
}

void CommandRecorder::BeginEnqueueBackupBuffer(const ID3D11DeviceContext* pDeferredDeviceContext, D3D11_MAPPED_SUBRESOURCE& MappedResource)
{
    AMDTScopeLock lock(m_pmutex);
    ContextMap::iterator it = m_DeferredContextMap.find(pDeferredDeviceContext);

    if (it == m_DeferredContextMap.end())
    {
        assert(!"Deferred context missing");
    }
    else
    {
        it->second->BeginEnqueueBackBuffer(MappedResource.pData);
    }
}

void CommandRecorder::EndEnqueueBackupBuffer(const ID3D11DeviceContext* pDeferredDeviceContext, ID3D11Resource* pRes)
{
    AMDTScopeLock lock(m_pmutex);
    ContextMap::iterator it = m_DeferredContextMap.find(pDeferredDeviceContext);

    if (it == m_DeferredContextMap.end())
    {
        assert(!"Deferred context missing");
        // TODOs: add deferred context to the map here
    }
    else
    {
        it->second->EndEnqueueBackBuffer(pRes);
    }
}

DCCommandBuffer* CommandRecorder::GetContext(const ID3D11DeviceContext* pDeferredDeviceContext) const
{
    ContextMap::const_iterator it = m_DeferredContextMap.find(pDeferredDeviceContext);

    if (it == m_DeferredContextMap.end())
    {
        assert(!"Deferred context missing");
        // TODOs: add deferred context to the map here
    }
    else
    {
        return it->second;
    }

    return NULL;
}