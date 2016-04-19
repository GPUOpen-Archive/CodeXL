//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains DirectCompute Utilities
//==============================================================================

#ifndef _DC_UTILS_H_
#define _DC_UTILS_H_

#include <windows.h>
#include <d3d11.h>

#include "DCStringifyD3d11Enums.h"
#include "DCStringifyDxgiFormatEnums.h"
#include "DCStringifyDxgiTypeEnums.h"

/// \addtogroup DCServer
// @{

/// Utility functions related to DirectX
namespace DCUtils
{
/// Make a copy of ID3D11Resource
/// \param pDevCntx Device Context
/// \param pObj resource to be copied
/// \return a copy of ID3DResource
ID3D11Resource* CloneResource(ID3D11DeviceContext* pDevCntx, ID3D11Resource* pObj);

/// Allocate memory and copy data from pSrc(Usually a pointer openned by Map that points to a dynamic memory) to pDst(target buffer), buffer type/size are defined by ID3D11Resource
/// \param pObj ID3D11Resource type e.g. Tex1D, Tex2D, Tex3D, Buffer
/// \param pSrc A pointer openned by Map() that points to a dynamic memory
/// \param pDst Output pointer points to target memory
/// \param size Output size of original buffer
void CopyBuffer(ID3D11Resource* pObj, void* pSrc, void** pDst, UINT& size);

/// Helper function that takes ID3D11Resource as input, output size in byte
/// \param pResource resource
/// \return size in byte
UINT GetResourceSize(ID3D11Resource* pResource);

/// Take DXGI_FORMAT as input, output size in byte
/// \param format DXGI_FORMAT
/// \return size in byte
UINT GetFormatSizeInByte(DXGI_FORMAT format);

/// Helper function that does simple allocation and array copy
/// \param NumBuffers size of array
/// \param pArray array
/// \return target array
template< class T > T* CopyArrays(UINT NumBuffers, const T* pArray)
{
    if (NumBuffers == 0)
    {
        return NULL;
    }

    T* pCB = (T*)malloc(NumBuffers * sizeof(T));

    for (UINT i = 0; i < NumBuffers; i++)
    {
        pCB[i] = pArray[i];
    }

    return pCB;
}

/// Helper function that copy array of pointer
/// \param NumBuffers size of array
/// \param ppArray array of T*
/// \return target array
template< class T > T** CopyArrayOfPointers(UINT NumBuffers, T* const* ppArray)
{
    if (NumBuffers == 0)
    {
        return NULL;
    }

    T** ppCB = (T**)malloc(NumBuffers * sizeof(T*));

    for (UINT i = 0; i < NumBuffers; i++)
    {
        ppCB[i] = ppArray[i];
    }

    return ppCB;
}

/// Helper function that copy array of IUnknown*, add reference to original IUnknown
/// \param NumBuffers size of array
/// \param ppIUnknown array of IUnknown*
/// \return target array
template< class T > T** CopyArrayOfInterfaces(UINT NumBuffers, T* const* ppIUnknown)
{
    T** ppCB = CopyArrayOfPointers(NumBuffers, ppIUnknown);

    //inc its ref count
    for (UINT i = 0; i < NumBuffers; i++)
    {
        if (ppCB[i] != NULL)
        {
            ppCB[i]->AddRef();
        }
    }

    return ppCB;
}

/// Helper function that releases array of IUnknown*, decrease reference counter, deallocate array
/// \param NumBuffers size of array
/// \param ppIUnknown array of T*
template< class T > void ReleaseArrayOfInterfaces(UINT NumBuffers, T** ppIUnknown)
{
    //inc its ref count
    for (UINT i = 0; i < NumBuffers; i++)
    {
        if (ppIUnknown[i])
        {
            ppIUnknown[i]->Release();
        }
    }

    if (ppIUnknown)
    {
        free(ppIUnknown);
    }
}
}

// @}

#endif //_DC_UTILS_H_