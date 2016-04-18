//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#ifndef _DC_ID3D11DEVICECONTEXT_VTABLE_MANAGER_H_
#define _DC_ID3D11DEVICECONTEXT_VTABLE_MANAGER_H_

#include "dcdetourhelper.h"
#include "DCID3D11DeviceContext_typedefs.h"

/// \addtogroup DCVirtualTablePatching
// @{

//------------------------------------------------------------------------------------
/// ID3D11DeviceContext VTable manager. It contains mine_* member function pointers that derived class should assign to and encapsulates Patch()
/// We shouldn't create an instance of it but inherit from it.
//------------------------------------------------------------------------------------
class ID3D11DeviceContextVTableManager :
    public VTableManager
{
public:
    /// Destructor
    virtual ~ID3D11DeviceContextVTableManager(void);

    /// Patch vtable
    /// \param pDev pointer to ID3D11DeviceContext pointer
    /// \param bAppCreated Is it created by client app?
    void Patch(ID3D11DeviceContext** pDev, bool bAppCreated);

protected:
    /// Constructor - We shouldn't create an instance of it.
    /// \param pFnMine Release Function pointer
    ID3D11DeviceContextVTableManager(ptrdiff_t* pFnMine);

private:
    /// Copy constructor - disabled
    /// \param obj source
    ID3D11DeviceContextVTableManager(const ID3D11DeviceContextVTableManager& obj);

    /// Assignment operator - disabled
    /// \param obj left
    /// \return right
    ID3D11DeviceContextVTableManager& operator = (const ID3D11DeviceContextVTableManager& obj);

protected:
    ID3D11DeviceContext_VSSetConstantBuffers_type m_pMine_ID3D11DeviceContext_VSSetConstantBuffersFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_PSSetShaderResources_type m_pMine_ID3D11DeviceContext_PSSetShaderResourcesFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_PSSetShader_type m_pMine_ID3D11DeviceContext_PSSetShaderFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_PSSetSamplers_type m_pMine_ID3D11DeviceContext_PSSetSamplersFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_VSSetShader_type m_pMine_ID3D11DeviceContext_VSSetShaderFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_DrawIndexed_type m_pMine_ID3D11DeviceContext_DrawIndexedFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_Draw_type m_pMine_ID3D11DeviceContext_DrawFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_Map_type m_pMine_ID3D11DeviceContext_MapFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_Unmap_type m_pMine_ID3D11DeviceContext_UnmapFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_PSSetConstantBuffers_type m_pMine_ID3D11DeviceContext_PSSetConstantBuffersFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_IASetInputLayout_type m_pMine_ID3D11DeviceContext_IASetInputLayoutFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_IASetVertexBuffers_type m_pMine_ID3D11DeviceContext_IASetVertexBuffersFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_IASetIndexBuffer_type m_pMine_ID3D11DeviceContext_IASetIndexBufferFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_DrawIndexedInstanced_type m_pMine_ID3D11DeviceContext_DrawIndexedInstancedFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_DrawInstanced_type m_pMine_ID3D11DeviceContext_DrawInstancedFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_GSSetConstantBuffers_type m_pMine_ID3D11DeviceContext_GSSetConstantBuffersFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_GSSetShader_type m_pMine_ID3D11DeviceContext_GSSetShaderFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_IASetPrimitiveTopology_type m_pMine_ID3D11DeviceContext_IASetPrimitiveTopologyFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_VSSetShaderResources_type m_pMine_ID3D11DeviceContext_VSSetShaderResourcesFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_VSSetSamplers_type m_pMine_ID3D11DeviceContext_VSSetSamplersFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_Begin_type m_pMine_ID3D11DeviceContext_BeginFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_End_type m_pMine_ID3D11DeviceContext_EndFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_GetData_type m_pMine_ID3D11DeviceContext_GetDataFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_SetPredication_type m_pMine_ID3D11DeviceContext_SetPredicationFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_GSSetShaderResources_type m_pMine_ID3D11DeviceContext_GSSetShaderResourcesFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_GSSetSamplers_type m_pMine_ID3D11DeviceContext_GSSetSamplersFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_OMSetRenderTargets_type m_pMine_ID3D11DeviceContext_OMSetRenderTargetsFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_OMSetRenderTargetsAndUnorderedAccessViews_type m_pMine_ID3D11DeviceContext_OMSetRenderTargetsAndUnorderedAccessViewsFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_OMSetBlendState_type m_pMine_ID3D11DeviceContext_OMSetBlendStateFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_OMSetDepthStencilState_type m_pMine_ID3D11DeviceContext_OMSetDepthStencilStateFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_SOSetTargets_type m_pMine_ID3D11DeviceContext_SOSetTargetsFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_DrawAuto_type m_pMine_ID3D11DeviceContext_DrawAutoFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_DrawIndexedInstancedIndirect_type m_pMine_ID3D11DeviceContext_DrawIndexedInstancedIndirectFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_DrawInstancedIndirect_type m_pMine_ID3D11DeviceContext_DrawInstancedIndirectFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_Dispatch_type m_pMine_ID3D11DeviceContext_DispatchFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_DispatchIndirect_type m_pMine_ID3D11DeviceContext_DispatchIndirectFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_RSSetState_type m_pMine_ID3D11DeviceContext_RSSetStateFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_RSSetViewports_type m_pMine_ID3D11DeviceContext_RSSetViewportsFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_RSSetScissorRects_type m_pMine_ID3D11DeviceContext_RSSetScissorRectsFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_CopySubresourceRegion_type m_pMine_ID3D11DeviceContext_CopySubresourceRegionFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_CopyResource_type m_pMine_ID3D11DeviceContext_CopyResourceFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_UpdateSubresource_type m_pMine_ID3D11DeviceContext_UpdateSubresourceFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_CopyStructureCount_type m_pMine_ID3D11DeviceContext_CopyStructureCountFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_ClearRenderTargetView_type m_pMine_ID3D11DeviceContext_ClearRenderTargetViewFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_ClearUnorderedAccessViewUint_type m_pMine_ID3D11DeviceContext_ClearUnorderedAccessViewUintFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_ClearUnorderedAccessViewFloat_type m_pMine_ID3D11DeviceContext_ClearUnorderedAccessViewFloatFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_ClearDepthStencilView_type m_pMine_ID3D11DeviceContext_ClearDepthStencilViewFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_GenerateMips_type m_pMine_ID3D11DeviceContext_GenerateMipsFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_SetResourceMinLOD_type m_pMine_ID3D11DeviceContext_SetResourceMinLODFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_GetResourceMinLOD_type m_pMine_ID3D11DeviceContext_GetResourceMinLODFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_ResolveSubresource_type m_pMine_ID3D11DeviceContext_ResolveSubresourceFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_ExecuteCommandList_type m_pMine_ID3D11DeviceContext_ExecuteCommandListFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_HSSetShaderResources_type m_pMine_ID3D11DeviceContext_HSSetShaderResourcesFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_HSSetShader_type m_pMine_ID3D11DeviceContext_HSSetShaderFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_HSSetSamplers_type m_pMine_ID3D11DeviceContext_HSSetSamplersFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_HSSetConstantBuffers_type m_pMine_ID3D11DeviceContext_HSSetConstantBuffersFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_DSSetShaderResources_type m_pMine_ID3D11DeviceContext_DSSetShaderResourcesFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_DSSetShader_type m_pMine_ID3D11DeviceContext_DSSetShaderFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_DSSetSamplers_type m_pMine_ID3D11DeviceContext_DSSetSamplersFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_DSSetConstantBuffers_type m_pMine_ID3D11DeviceContext_DSSetConstantBuffersFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_CSSetShaderResources_type m_pMine_ID3D11DeviceContext_CSSetShaderResourcesFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_CSSetUnorderedAccessViews_type m_pMine_ID3D11DeviceContext_CSSetUnorderedAccessViewsFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_CSSetShader_type m_pMine_ID3D11DeviceContext_CSSetShaderFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_CSSetSamplers_type m_pMine_ID3D11DeviceContext_CSSetSamplersFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_CSSetConstantBuffers_type m_pMine_ID3D11DeviceContext_CSSetConstantBuffersFn;      ///< Detoured member function pointer for ID3D11DeviceContext

    ID3D11DeviceContext_ClearState_type m_pMine_ID3D11DeviceContext_ClearStateFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_Flush_type m_pMine_ID3D11DeviceContext_FlushFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_GetContextFlags_type m_pMine_ID3D11DeviceContext_GetContextFlagsFn;      ///< Detoured member function pointer for ID3D11DeviceContext
    ID3D11DeviceContext_FinishCommandList_type m_pMine_ID3D11DeviceContext_FinishCommandListFn;      ///< Detoured member function pointer for ID3D11DeviceContext
};

// @}

#endif // _DC_ID3D11DEVICECONTEXT_VTABLE_MANAGER_H_