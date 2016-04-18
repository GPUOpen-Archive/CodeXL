//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#ifndef _DC_COMMAND_RECORDER_H_
#define _DC_COMMAND_RECORDER_H_

#include <windows.h>
#include <d3d11.h>
#include <vector>
#include <queue>
#include <map>
#include "DCCommandDefs.h"
#include "AMDTMutex.h"

/// \defgroup DCCommandRecorder DCCommandRecorder
/// This module captures commands executed on deferred context, flatten them and
/// execute them on the immediate context.
///
/// \ingroup DCServer
// @{

typedef std::queue<DC_CMDBase*> DCCommandList;

//------------------------------------------------------------------------------------
/// This class captures commands executed on deferred context
/// Flatten and execute them on immediate context
/// One DCCommandBuffer Per ID3D11CommandList
/// Remark: Main thread and deferred context thread could run in parallel
///         Once FinishCommandList is called, a new DCCommandBuffer should be created
///         for deferred context to continue to record commands while the old one wait
///         to be executed when ExecuteCommandList is called. It prevents consuming from
///         and producing to the same command list from happening
//------------------------------------------------------------------------------------
class DCCommandBuffer
{
    friend class DCContextManager;
    friend class DC_CMD_Map;
    friend class DC_CMD_Unmap;

public:

    /// A structure to hold pointers for the backup buffer
    struct BackupBuffer
    {
        void* m_pBuffer;     /// < Pointer to the mapped memory
        UINT  m_uiSize;      /// < Size of the mapped memory
        void* m_pTmpBuffer;  /// < Temporary buffer pointer which points to mapped memory(usually dynamic memory read by GPU and write by CPU) when map() is called

        BackupBuffer()
        {
            m_pBuffer = m_pTmpBuffer = NULL;
            m_uiSize = 0;
        }
    };
public:
    /// Constructor
    DCCommandBuffer();

    /// Destructor
    ~DCCommandBuffer();

    /// Release command list
    void ClearCommandList();

    /// Add command to command list
    /// \param pCMD Command called on deferred context
    void AddToCommandList(DC_CMDBase* pCMD);

    /// Execute deferred commands on immediate context Cleanup commands after execution
    /// \param pImmediateContext Immediate context
    void ExecuteCommandList(ID3D11DeviceContext* pImmediateContext);

    /// Return the recorded command list
    /// \return pointer to recorded command list
    DCCommandList* DebugCommandList();

    /// Create backup buffer structure, enqueue, wait to be updated when unmap() is called
    /// \param pBuf pointer opened by Map(), points to mapped(dynamic) memory
    void BeginEnqueueBackBuffer(void* pBuf);

    /// Update backup buffer at back of the queue
    /// \param pRes resource that to be updated
    void EndEnqueueBackBuffer(ID3D11Resource* pRes);

    /// Dequeue backup
    /// \param ppBuf pointer to backup buffer
    /// \param size size of backup buffer
    void DequeueBackBuffer(void** ppBuf, UINT& size);

private:
    /// Disable copy constructor
    /// \param dcc  the input object
    DCCommandBuffer(const DCCommandBuffer& dcc);

    /// Disable assignment operator
    /// \param dcc  the input object
    /// \return a reference to the object
    DCCommandBuffer& operator= (const DCCommandBuffer& dcc);

private:
    DCCommandList m_CMDList;                       ///< List of commands deferred to be execute
    void* m_pDynMem;                               ///< current pointer openned by map() from immediate context for writting to GPU
    std::queue<BackupBuffer> m_qBackupBufferQueue; ///< Queue that maintains all backup buffers
};

typedef std::map<const ID3D11DeviceContext*, DCCommandBuffer*> ContextMap;
typedef std::pair<const ID3D11DeviceContext*, DCCommandBuffer*> ContextMapPair;

typedef std::map<ID3D11CommandList*, DCCommandBuffer*> CommandListMap;
typedef std::pair<ID3D11CommandList*, DCCommandBuffer*> CommandListMapPair;

//------------------------------------------------------------------------------------
/// This class manages the deferred contexts
/// It records all commands executed and executes them on immediate context
//------------------------------------------------------------------------------------
class CommandRecorder
{
public:
    /// Constructor
    CommandRecorder(void);

    /// Destructor
    ~CommandRecorder(void);

    /// Set ID3D11Device
    /// \param pDevice current Device created by DXGIFactory1
    void SetID3D11Device(ID3D11Device* pDevice)
    {
        m_pID3D11Device = pDevice;
    }

    /// Set ID3D11Device
    /// \param pDeviceContext Immediate context
    void SetID3D11DeviceContext(ID3D11DeviceContext* pDeviceContext)
    {
        m_pImmediateDeviceContext = pDeviceContext;
    }

    /// Add defered device context
    /// \param pDeferredDeviceContext Deferred Device Context
    void AddDeferredDeviceContext(const ID3D11DeviceContext* pDeferredDeviceContext);

    /// Create an new empty command list for deferred context, pair its current command list with ID3D11CommandList that just created
    /// \param pDeferredDeviceContext deferred Device Context
    /// \param pCommandList New command list
    void CreateFlattenedCommandList(const ID3D11DeviceContext* pDeferredDeviceContext, ID3D11CommandList* pCommandList);

    /// Add to coresponding deferred context
    /// \param pDeferredDeviceContext deferred device context
    /// \param pCmd command executed on deferred context
    void AddToCommandList(const ID3D11DeviceContext* pDeferredDeviceContext, DC_CMDBase* pCmd);

    /// Flatten deferred command list and run command on immediate command list
    /// \param pCommandList key to lookup ID3D11DeviceContext which can be used to lookup DCContext
    /// \param RestoreContextState TRUE: Restore context on immediate context after execute, FALSE: Restore context on immeidate context to default context
    void ExecuteCommands(ID3D11CommandList* pCommandList, BOOL RestoreContextState);

    /// Return the recorded command list
    /// \param [in] pCommandList DXCommandList, map key to DCCommandBuffer
    /// \return pointer to recorded command list
    DCCommandList* DebugCommandList(ID3D11CommandList* pCommandList);

    /// Begin backup the buffer updated on deferred context
    /// \param pDeferredDeviceContext deferred device context
    /// \param MappedResource Structure that contains pointer to mapped memory
    void BeginEnqueueBackupBuffer(const ID3D11DeviceContext* pDeferredDeviceContext, D3D11_MAPPED_SUBRESOURCE& MappedResource);

    /// Begin backup the buffer updated on deferred context
    /// \param pDeferredDeviceContext deferred device context
    /// \param pRes pointer to buffer resource
    void EndEnqueueBackupBuffer(const ID3D11DeviceContext* pDeferredDeviceContext, ID3D11Resource* pRes);

    /// Get DCCommandBuffer that currently paired with deferred context
    /// \param pDeferredDeviceContext pointer to deferred context
    /// \return DCCommandBuffer that currently paired with deferred context
    DCCommandBuffer* GetContext(const ID3D11DeviceContext* pDeferredDeviceContext) const;

private:
    /// Disable copy constructor
    /// \param [in] obj source
    CommandRecorder(const CommandRecorder& obj);

    /// Disable assignment operator
    /// \param [in] obj left
    /// \return value
    CommandRecorder& operator = (const CommandRecorder& obj);

private:
    ContextMap           m_DeferredContextMap;         ///< Deferred Context
    CommandListMap       m_CommandListMap;             ///< Map from ID3D11CommandList to our own command list
    AMDTMutex*           m_pmutex;                     ///< mutex
    ID3D11DeviceContext* m_pImmediateDeviceContext;    ///< Immediate device context
    ID3D11Device*        m_pID3D11Device;              ///< ID3D11Device
};

// @}

#endif // _DC_COMMAND_RECORDER_H_