//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file Wrapped_ID3D12Heap.h
/// \brief A class used to wrap D3D12's ID3D12Heap interface.
//=============================================================================

#ifndef WRAPPED_ID3D12HEAP_H
#define WRAPPED_ID3D12HEAP_H

#include "DX12Defines.h"

class Wrapped_ID3D12Device;
class Wrapped_ID3D12HeapCreateInfo;

//-----------------------------------------------------------------------------
/// Utility function used to wrap the D3D12 interface.
/// \param inParentDevice The parent device for the interface.
/// \param inRealHeap The real runtime instance of the D3D12 interface.
/// \param inCreateInfo A structure containing the interface creation info.
/// \returns True if the interface was wrapped successfully.
//-----------------------------------------------------------------------------
bool WrapD3D12Heap(Wrapped_ID3D12Device* inParentDevice, ID3D12Heap** inRealHeap, Wrapped_ID3D12HeapCreateInfo* inCreateInfo = nullptr);

//-----------------------------------------------------------------------------
/// A class used to wrap D3D12's ID3D12Heap interface.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12Heap : public ID3D12Heap
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor accepts the real runtime instance.
    /// \param inRealHeap The real runtime instance being wrapped.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12Heap(ID3D12Heap* inRealHeap) { mRealHeap = inRealHeap; }

    //-----------------------------------------------------------------------------
    /// Default destructor.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12Heap() {}

    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // ID3D12Object
    virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData);
    virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID guid, UINT DataSize, const void* pData);
    virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID guid, const IUnknown* pData);
    virtual HRESULT STDMETHODCALLTYPE SetName(LPCWSTR Name);

    // ID3D12DeviceChild
    virtual HRESULT STDMETHODCALLTYPE GetDevice(REFIID riid, void** ppvDevice);

    // ID3D12Heap
    virtual D3D12_HEAP_DESC STDMETHODCALLTYPE GetDesc();

    ID3D12Heap* mRealHeap;      ///< The real runtime instance being wrapped.
};

#endif // WRAPPED_ID3D12HEAP_H