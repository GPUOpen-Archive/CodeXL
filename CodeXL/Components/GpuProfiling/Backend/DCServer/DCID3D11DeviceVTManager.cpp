//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#include "DCID3D11DeviceVTManager.h"
#include "DCID3D11Device_wrapper.h"
#include "DCVtableOffsets.h"

DCID3D11DeviceVTManager::DCID3D11DeviceVTManager(ptrdiff_t* pFnMine) : VTableManager("ID3D11DeviceVTableManager", 2, pFnMine, 2)
{
    m_pMine_ID3D11Device_CreateBufferFn = NULL;
    m_pMine_ID3D11Device_CreateTexture1DFn = NULL;
    m_pMine_ID3D11Device_CreateTexture2DFn = NULL;
    m_pMine_ID3D11Device_CreateTexture3DFn = NULL;
    m_pMine_ID3D11Device_CreateUnorderedAccessViewFn = NULL;
    m_pMine_ID3D11Device_CreateDeferredContextFn = NULL;
    m_pMine_ID3D11Device_GetImmediateContextFn = NULL;
    m_pMine_ID3D11Device_CreateComputeShaderFn = NULL;
}

DCID3D11DeviceVTManager::~DCID3D11DeviceVTManager(void)
{

}

void DCID3D11DeviceVTManager::Patch(ID3D11Device** ppDevice, bool bAppCreated)
{
    if (ppDevice && *ppDevice)
    {
        *ppDevice = WrapDevice(*ppDevice);
        ID3D11Device* pDevice = *ppDevice;
        // This is added to work around a mem leak in DXUT June 2010
        // Assumption : Only one ID3D11Device can exist
        // See DXUTDevice11.cpp line 471, ID3D11Device is created but never gets released
        UnpatchAndRemoveAll();

        if (AddAndDetourIfUnique(pDevice, bAppCreated))
        {
            AddVtableElementToPatch(pDevice, DX11_VTABLE_OFFSET_ID3D11Device_CreateUnorderedAccessView, (ptrdiff_t*)m_pMine_ID3D11Device_CreateUnorderedAccessViewFn);
            AddVtableElementToPatch(pDevice, DX11_VTABLE_OFFSET_ID3D11Device_CreateBuffer, (ptrdiff_t*)m_pMine_ID3D11Device_CreateBufferFn);
            AddVtableElementToPatch(pDevice, DX11_VTABLE_OFFSET_ID3D11Device_CreateTexture1D, (ptrdiff_t*)m_pMine_ID3D11Device_CreateTexture1DFn);
            AddVtableElementToPatch(pDevice, DX11_VTABLE_OFFSET_ID3D11Device_CreateTexture2D, (ptrdiff_t*)m_pMine_ID3D11Device_CreateTexture2DFn);
            AddVtableElementToPatch(pDevice, DX11_VTABLE_OFFSET_ID3D11Device_CreateTexture3D, (ptrdiff_t*)m_pMine_ID3D11Device_CreateTexture3DFn);
            AddVtableElementToPatch(pDevice, DX11_VTABLE_OFFSET_ID3D11Device_CreateComputeShader, (ptrdiff_t*)m_pMine_ID3D11Device_CreateComputeShaderFn);
            AddVtableElementToPatch(pDevice, DX11_VTABLE_OFFSET_ID3D11Device_CreateDeferredContext, (ptrdiff_t*)m_pMine_ID3D11Device_CreateDeferredContextFn);
            AddVtableElementToPatch(pDevice, DX11_VTABLE_OFFSET_ID3D11Device_GetImmediateContext, (ptrdiff_t*)m_pMine_ID3D11Device_GetImmediateContextFn);
            Attach();
        }
    }
    else
    {
        Log(logWARNING, "DCID3D11DeviceVTManager::Patch() Device == NULL\n");
        return;
    }
}
