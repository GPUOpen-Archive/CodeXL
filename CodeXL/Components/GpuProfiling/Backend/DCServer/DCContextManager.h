//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#ifndef _DC_CONTEXT_MANAGER_H_
#define _DC_CONTEXT_MANAGER_H_

#include <windows.h>
#include <d3d11.h>
#include <vector>
#include <map>
#include "AMDTMutex.h"

/// \defgroup DCContextManager DCContextManager
/// This module handles keeping track of resources and creating
/// backup resources to be used when dispatching kernel multiple times.
///
/// \ingroup DCServer
// @{

typedef std::map<ID3D11UnorderedAccessView*, ID3D11UnorderedAccessView*> UAVMap;
typedef std::map<ID3D11Resource*, ID3D11Resource*> ResourceMap;

//------------------------------------------------------------------------------------
/// This class manages context on ImmediateContext
/// It pushes and pops context before and after Dispatch() call
//------------------------------------------------------------------------------------
class DCContextManager
{
public:
    /// Constructor
    DCContextManager(void);
    /// Destructor
    ~DCContextManager(void);

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

    /// For active UAV in m_vecActiveUAVs, replace client UAV with backup UAV
    /// \return true if operation is successful
    bool PushContext();

    /// For active UAV in m_vecActiveUAVs, set as active UAVs
    /// \return true if operation is successful
    bool PopContext();

    /// For each UAV in m_vecActiveUAVs, find it backup UAV, restore its value to client UAV
    /// \return true if operation is successful
    bool RestoreContext();

    /// Get the reference count, real ref count * 2 because AddRef is called for original resources and UAVs when we create backup ones
    /// \return reference count
    UINT GetNumBackupResourcesCreated()
    {
        return m_uiRefCounter * 2;
    }

    /// Create backup texture1D, increase reference counter by 1
    /// \param pDesc tex description
    /// \param pInitialData Initial data
    /// \return backup tex1D
    ID3D11Texture1D* CreateBackupTextur1D(const D3D11_TEXTURE1D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData);

    /// Create backup texture2D, increase reference counter by 1
    /// \param pDesc tex description
    /// \param pInitialData Initial data
    /// \return backup tex2D
    ID3D11Texture2D* CreateBackupTextur2D(const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData);

    /// Create backup texture1D, increase reference counter by 1
    /// \param pDesc tex description
    /// \param pInitialData Initial data
    /// \return backup tex3D
    ID3D11Texture3D* CreateBackupTextur3D(const D3D11_TEXTURE3D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData);

    /// Create backup buffer, increase reference counter by 1
    /// \param pDesc buffer description
    /// \param pInitialData Initial data
    /// \return backup buffer
    ID3D11Buffer* CreateBackupBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData);

    /// Create backup UAV, increase reference counter by 1
    /// \param pResource original resource that is used to create the UAV
    /// \param pDesc UAV description
    /// \return backup UAV
    ID3D11UnorderedAccessView* CreateBackupUAV(ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc);

    /// Save active UAVs when client calls CSSetUAV (on Immediate device context)
    /// \param StartSlot Start slot
    /// \param NumUAVs Number of UAVs
    /// \param ppUnorderedAccessViews UAV array
    /// \param pUAVInitialCounts Offset array
    void SaveUAV(UINT StartSlot, UINT NumUAVs, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews, const UINT* pUAVInitialCounts);

    /// Create a back up resource
    /// \param pOriginal original resource
    /// \param pCopy backup resource
    void AddBackupResource(ID3D11Resource* pOriginal, ID3D11Resource* pCopy);

    /// Get backup resource
    /// \param pOriginal Original resource
    /// \return backup resource
    ID3D11Resource* GetBackupResource(ID3D11Resource* pOriginal);

    /// Add both original UAV and backup UAV
    /// \param pOriginal UAV
    /// \param pCopy UAV
    void AddtoUAVTable(ID3D11UnorderedAccessView* pOriginal, ID3D11UnorderedAccessView* pCopy);

    /// Return the number of release
    /// Can be added to render loop
    /// it checks the ref counter of orignal resources and UAVs, if it equals to 1, releave it, as well as backup resources
    /// \return the number of release
    ULONG Cleanup();

    /// Set current(immediate context only) compute shader
    /// \param pCS pointer to compute shader
    void SetCurrentComputeShader(ID3D11ComputeShader* pCS);

    /// Get current active compute shader set on immediate context
    /// \return current compute shader set on immediate context
    ID3D11ComputeShader* GetCurrentComputeShader() const;

private:
    /// Copy original UAV to backup UAV
    /// \param pOriginalUAV Original/Client UAV
    bool UpdateBackupResource(ID3D11UnorderedAccessView* pOriginalUAV);

private:
    ID3D11ComputeShader* m_pCurrentCS;                                  ///< active compute shader set on immediate context
    ID3D11UnorderedAccessView* m_arrActiveUAVs[D3D11_PS_CS_UAV_REGISTER_COUNT];                       ///< active original UAVs list set on immediate context
    UINT m_arrUAVOffset[D3D11_PS_CS_UAV_REGISTER_COUNT];                                              ///< Saved UAV offset array on immediate context
    bool m_arrOffsetEnable[D3D11_PS_CS_UAV_REGISTER_COUNT];                                           ///< Array of bool that indicating whether UAV offset is enable or not

    ID3D11Device*        m_pID3D11Device;              ///< ID3D11Device
    ID3D11DeviceContext* m_pImmediateDeviceContext;    ///< Immediate device context
    UAVMap               m_UAVTable;                   ///< map original UAV to backup UAV
    ResourceMap          m_ResxMap;                    ///< map original resource to backup resource
    UINT                 m_uiRefCounter;               ///< Number of backup resources created
    AMDTMutex*           m_pmutex;                     ///< mutex
};

// @}

#endif // _DC_CONTEXT_MANAGER_H_