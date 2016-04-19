//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#include "DCCommandDefs.h"
#include "DCCommandRecorder.h"
#include <assert.h>


void DC_CMD_Map::Play(ID3D11DeviceContext* pImmediateContext)
{
    HRESULT hr = pImmediateContext->Map(m_pResource, m_Subresource, m_MapType, m_MapFlags, m_pMappedResource);

    if (SUCCEEDED(hr))
    {
        m_pOwner->m_pDynMem = m_pMappedResource->pData;
    }
    else
    {
        assert(!"Map() failed:shouldn't happen");
    }
}

void DC_CMD_Map::OnCreate(ID3D11DeviceContext* pObj, ID3D11Resource* pResource, UINT Subresource, D3D11_MAP MapType, UINT MapFlags, D3D11_MAPPED_SUBRESOURCE* pMappedResource, DCCommandBuffer* pOwner)
{
    m_pObj = pObj;
    m_pResource = pResource;
    m_Subresource = Subresource;
    m_MapType = MapType;
    m_MapFlags = MapFlags;
    m_MappedResource = *pMappedResource;
    m_pMappedResource = &m_MappedResource;
    m_CMDType = DC_CMD_Type_Map;
    m_pOwner = pOwner;
}

void DC_CMD_Unmap::OnCreate(ID3D11DeviceContext* pObj, ID3D11Resource* pResource, UINT Subresource, DCCommandBuffer* pOwner)
{
    m_pObj = pObj;
    m_pResource = pResource;
    m_Subresource = Subresource;
    m_CMDType = DC_CMD_Type_Unmap;
    m_pOwner = pOwner;
}

/// This is called when we flatten command list and execute commands on immediate context
void DC_CMD_Unmap::Play(ID3D11DeviceContext* pImmediateContext)
{
    void* pBuf = NULL;
    UINT size;
    m_pOwner->DequeueBackBuffer(&pBuf, size);
    assert(pBuf != NULL);
    assert(m_pOwner->m_pDynMem != NULL);
    // write to pDynMem
    memcpy(m_pOwner->m_pDynMem, pBuf, size * sizeof(BYTE));
    pImmediateContext->Unmap(m_pResource, m_Subresource);
    free(pBuf);
}