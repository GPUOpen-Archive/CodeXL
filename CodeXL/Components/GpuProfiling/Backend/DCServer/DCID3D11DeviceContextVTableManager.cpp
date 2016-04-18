//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#include "DCID3D11DeviceContextVTableManager.h"
#include "DCID3D11DeviceContext_wrapper.h"

ID3D11DeviceContextVTableManager::ID3D11DeviceContextVTableManager(ptrdiff_t* pFnMine) : VTableManager("ID3D11DeviceContextVTMgr", 2, pFnMine)
{
    m_pMine_ID3D11DeviceContext_VSSetConstantBuffersFn = NULL;
    m_pMine_ID3D11DeviceContext_PSSetShaderResourcesFn = NULL;
    m_pMine_ID3D11DeviceContext_PSSetShaderFn = NULL;
    m_pMine_ID3D11DeviceContext_PSSetSamplersFn = NULL;
    m_pMine_ID3D11DeviceContext_VSSetShaderFn = NULL;
    m_pMine_ID3D11DeviceContext_DrawIndexedFn = NULL;
    m_pMine_ID3D11DeviceContext_DrawFn = NULL;
    m_pMine_ID3D11DeviceContext_MapFn = NULL;
    m_pMine_ID3D11DeviceContext_UnmapFn = NULL;
    m_pMine_ID3D11DeviceContext_PSSetConstantBuffersFn = NULL;
    m_pMine_ID3D11DeviceContext_IASetInputLayoutFn = NULL;
    m_pMine_ID3D11DeviceContext_IASetVertexBuffersFn = NULL;
    m_pMine_ID3D11DeviceContext_IASetIndexBufferFn = NULL;
    m_pMine_ID3D11DeviceContext_DrawIndexedInstancedFn = NULL;
    m_pMine_ID3D11DeviceContext_DrawInstancedFn = NULL;
    m_pMine_ID3D11DeviceContext_GSSetConstantBuffersFn = NULL;
    m_pMine_ID3D11DeviceContext_GSSetShaderFn = NULL;
    m_pMine_ID3D11DeviceContext_IASetPrimitiveTopologyFn = NULL;
    m_pMine_ID3D11DeviceContext_VSSetShaderResourcesFn = NULL;
    m_pMine_ID3D11DeviceContext_VSSetSamplersFn = NULL;
    m_pMine_ID3D11DeviceContext_BeginFn = NULL;
    m_pMine_ID3D11DeviceContext_EndFn = NULL;
    m_pMine_ID3D11DeviceContext_GetDataFn = NULL;
    m_pMine_ID3D11DeviceContext_SetPredicationFn = NULL;
    m_pMine_ID3D11DeviceContext_GSSetShaderResourcesFn = NULL;
    m_pMine_ID3D11DeviceContext_GSSetSamplersFn = NULL;
    m_pMine_ID3D11DeviceContext_OMSetRenderTargetsFn = NULL;
    m_pMine_ID3D11DeviceContext_OMSetRenderTargetsAndUnorderedAccessViewsFn = NULL;
    m_pMine_ID3D11DeviceContext_OMSetBlendStateFn = NULL;
    m_pMine_ID3D11DeviceContext_OMSetDepthStencilStateFn = NULL;
    m_pMine_ID3D11DeviceContext_SOSetTargetsFn = NULL;
    m_pMine_ID3D11DeviceContext_DrawAutoFn = NULL;
    m_pMine_ID3D11DeviceContext_DrawIndexedInstancedIndirectFn = NULL;
    m_pMine_ID3D11DeviceContext_DrawInstancedIndirectFn = NULL;
    m_pMine_ID3D11DeviceContext_DispatchFn = NULL;
    m_pMine_ID3D11DeviceContext_DispatchIndirectFn = NULL;
    m_pMine_ID3D11DeviceContext_RSSetStateFn = NULL;
    m_pMine_ID3D11DeviceContext_RSSetViewportsFn = NULL;
    m_pMine_ID3D11DeviceContext_RSSetScissorRectsFn = NULL;
    m_pMine_ID3D11DeviceContext_CopySubresourceRegionFn = NULL;
    m_pMine_ID3D11DeviceContext_CopyResourceFn = NULL;
    m_pMine_ID3D11DeviceContext_UpdateSubresourceFn = NULL;
    m_pMine_ID3D11DeviceContext_CopyStructureCountFn = NULL;
    m_pMine_ID3D11DeviceContext_ClearRenderTargetViewFn = NULL;
    m_pMine_ID3D11DeviceContext_ClearUnorderedAccessViewUintFn = NULL;
    m_pMine_ID3D11DeviceContext_ClearUnorderedAccessViewFloatFn = NULL;
    m_pMine_ID3D11DeviceContext_ClearDepthStencilViewFn = NULL;
    m_pMine_ID3D11DeviceContext_GenerateMipsFn = NULL;
    m_pMine_ID3D11DeviceContext_SetResourceMinLODFn = NULL;
    m_pMine_ID3D11DeviceContext_GetResourceMinLODFn = NULL;
    m_pMine_ID3D11DeviceContext_ResolveSubresourceFn = NULL;
    m_pMine_ID3D11DeviceContext_ExecuteCommandListFn = NULL;
    m_pMine_ID3D11DeviceContext_HSSetShaderResourcesFn = NULL;
    m_pMine_ID3D11DeviceContext_HSSetShaderFn = NULL;
    m_pMine_ID3D11DeviceContext_HSSetSamplersFn = NULL;
    m_pMine_ID3D11DeviceContext_HSSetConstantBuffersFn = NULL;
    m_pMine_ID3D11DeviceContext_DSSetShaderResourcesFn = NULL;
    m_pMine_ID3D11DeviceContext_DSSetShaderFn = NULL;
    m_pMine_ID3D11DeviceContext_DSSetSamplersFn = NULL;
    m_pMine_ID3D11DeviceContext_DSSetConstantBuffersFn = NULL;
    m_pMine_ID3D11DeviceContext_CSSetShaderResourcesFn = NULL;
    m_pMine_ID3D11DeviceContext_CSSetUnorderedAccessViewsFn = NULL;
    m_pMine_ID3D11DeviceContext_CSSetShaderFn = NULL;
    m_pMine_ID3D11DeviceContext_CSSetSamplersFn = NULL;
    m_pMine_ID3D11DeviceContext_CSSetConstantBuffersFn = NULL;

    m_pMine_ID3D11DeviceContext_ClearStateFn = NULL;
    m_pMine_ID3D11DeviceContext_FlushFn = NULL;
    m_pMine_ID3D11DeviceContext_GetContextFlagsFn = NULL;
    m_pMine_ID3D11DeviceContext_FinishCommandListFn = NULL;
}

ID3D11DeviceContextVTableManager::~ID3D11DeviceContextVTableManager(void)
{
}

void ID3D11DeviceContextVTableManager::Patch(ID3D11DeviceContext** ppImmediateContext, bool bAppCreated)
{
    *ppImmediateContext = WrapDeviceContext(*ppImmediateContext);
    ID3D11DeviceContext* pDev = *ppImmediateContext;
    // NOTE: We can only do this if all DeviceContext are wrapped - Wrapped Device Context always share the same vtable
    UnpatchAndRemoveAll();

    if (AddAndDetourIfUnique(pDev, bAppCreated))
    {
        AddVtableElementToPatch(pDev, 7, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_VSSetConstantBuffersFn);
        AddVtableElementToPatch(pDev, 8, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_PSSetShaderResourcesFn);
        AddVtableElementToPatch(pDev, 9, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_PSSetShaderFn);
        AddVtableElementToPatch(pDev, 10, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_PSSetSamplersFn);
        AddVtableElementToPatch(pDev, 11, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_VSSetShaderFn);
        AddVtableElementToPatch(pDev, 12, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_DrawIndexedFn);
        AddVtableElementToPatch(pDev, 13, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_DrawFn);
        AddVtableElementToPatch(pDev, 14, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_MapFn);
        AddVtableElementToPatch(pDev, 15, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_UnmapFn);
        AddVtableElementToPatch(pDev, 16, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_PSSetConstantBuffersFn);
        AddVtableElementToPatch(pDev, 17, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_IASetInputLayoutFn);
        AddVtableElementToPatch(pDev, 18, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_IASetVertexBuffersFn);
        AddVtableElementToPatch(pDev, 19, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_IASetIndexBufferFn);
        AddVtableElementToPatch(pDev, 20, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_DrawIndexedInstancedFn);
        AddVtableElementToPatch(pDev, 21, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_DrawInstancedFn);
        AddVtableElementToPatch(pDev, 22, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_GSSetConstantBuffersFn);
        AddVtableElementToPatch(pDev, 23, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_GSSetShaderFn);
        AddVtableElementToPatch(pDev, 24, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_IASetPrimitiveTopologyFn);
        AddVtableElementToPatch(pDev, 25, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_VSSetShaderResourcesFn);
        AddVtableElementToPatch(pDev, 26, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_VSSetSamplersFn);
        AddVtableElementToPatch(pDev, 27, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_BeginFn);
        AddVtableElementToPatch(pDev, 28, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_EndFn);
        AddVtableElementToPatch(pDev, 29, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_GetDataFn);
        AddVtableElementToPatch(pDev, 30, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_SetPredicationFn);
        AddVtableElementToPatch(pDev, 31, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_GSSetShaderResourcesFn);
        AddVtableElementToPatch(pDev, 32, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_GSSetSamplersFn);
        AddVtableElementToPatch(pDev, 33, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_OMSetRenderTargetsFn);
        AddVtableElementToPatch(pDev, 34, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_OMSetRenderTargetsAndUnorderedAccessViewsFn);
        AddVtableElementToPatch(pDev, 35, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_OMSetBlendStateFn);
        AddVtableElementToPatch(pDev, 36, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_OMSetDepthStencilStateFn);
        AddVtableElementToPatch(pDev, 37, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_SOSetTargetsFn);
        AddVtableElementToPatch(pDev, 38, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_DrawAutoFn);
        AddVtableElementToPatch(pDev, 39, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_DrawIndexedInstancedIndirectFn);
        AddVtableElementToPatch(pDev, 40, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_DrawInstancedIndirectFn);
        AddVtableElementToPatch(pDev, 41, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_DispatchFn);
        AddVtableElementToPatch(pDev, 42, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_DispatchIndirectFn);
        AddVtableElementToPatch(pDev, 43, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_RSSetStateFn);
        AddVtableElementToPatch(pDev, 44, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_RSSetViewportsFn);
        AddVtableElementToPatch(pDev, 45, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_RSSetScissorRectsFn);
        AddVtableElementToPatch(pDev, 46, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_CopySubresourceRegionFn);
        AddVtableElementToPatch(pDev, 47, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_CopyResourceFn);
        AddVtableElementToPatch(pDev, 48, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_UpdateSubresourceFn);
        AddVtableElementToPatch(pDev, 49, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_CopyStructureCountFn);
        AddVtableElementToPatch(pDev, 50, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_ClearRenderTargetViewFn);
        AddVtableElementToPatch(pDev, 51, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_ClearUnorderedAccessViewUintFn);
        AddVtableElementToPatch(pDev, 52, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_ClearUnorderedAccessViewFloatFn);
        AddVtableElementToPatch(pDev, 53, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_ClearDepthStencilViewFn);
        AddVtableElementToPatch(pDev, 54, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_GenerateMipsFn);
        AddVtableElementToPatch(pDev, 55, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_SetResourceMinLODFn);
        AddVtableElementToPatch(pDev, 56, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_GetResourceMinLODFn);
        AddVtableElementToPatch(pDev, 57, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_ResolveSubresourceFn);
        AddVtableElementToPatch(pDev, 58, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_ExecuteCommandListFn);
        AddVtableElementToPatch(pDev, 59, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_HSSetShaderResourcesFn);
        AddVtableElementToPatch(pDev, 60, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_HSSetShaderFn);
        AddVtableElementToPatch(pDev, 61, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_HSSetSamplersFn);
        AddVtableElementToPatch(pDev, 62, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_HSSetConstantBuffersFn);
        AddVtableElementToPatch(pDev, 63, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_DSSetShaderResourcesFn);
        AddVtableElementToPatch(pDev, 64, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_DSSetShaderFn);
        AddVtableElementToPatch(pDev, 65, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_DSSetSamplersFn);
        AddVtableElementToPatch(pDev, 66, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_DSSetConstantBuffersFn);
        AddVtableElementToPatch(pDev, 67, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_CSSetShaderResourcesFn);
        AddVtableElementToPatch(pDev, 68, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_CSSetUnorderedAccessViewsFn);
        AddVtableElementToPatch(pDev, 69, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_CSSetShaderFn);
        AddVtableElementToPatch(pDev, 70, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_CSSetSamplersFn);
        AddVtableElementToPatch(pDev, 71, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_CSSetConstantBuffersFn);

        AddVtableElementToPatch(pDev, 110, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_ClearStateFn);
        AddVtableElementToPatch(pDev, 111, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_FlushFn);
        AddVtableElementToPatch(pDev, 113, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_GetContextFlagsFn);
        AddVtableElementToPatch(pDev, 114, (ptrdiff_t*)m_pMine_ID3D11DeviceContext_FinishCommandListFn);
        Attach();
    }
}