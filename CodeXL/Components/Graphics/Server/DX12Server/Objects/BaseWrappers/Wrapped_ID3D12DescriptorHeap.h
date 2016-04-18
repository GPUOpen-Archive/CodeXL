//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file Wrapped_ID3D12DescriptorHeap.h
/// \brief A class used to wrap D3D12's ID3D12DescriptorHeap interface.
//=============================================================================

#ifndef WRAPPED_ID3D12DESCRIPTORHEAP_H
#define WRAPPED_ID3D12DESCRIPTORHEAP_H

#include "DX12Defines.h"

class Wrapped_ID3D12Device;
class Wrapped_ID3D12DescriptorHeapCreateInfo;

//-----------------------------------------------------------------------------
/// Utility function used to wrap the D3D12 interface.
/// \param inParentDevice The parent device for the interface.
/// \param inRealDescriptorHeap The real runtime instance of the D3D12 interface.
/// \param inCreateInfo A structure containing the interface creation info.
/// \returns True if the interface was wrapped successfully.
//-----------------------------------------------------------------------------
bool WrapD3D12DescriptorHeap(Wrapped_ID3D12Device* inParentDevice, ID3D12DescriptorHeap** inRealDescriptorHeap, Wrapped_ID3D12DescriptorHeapCreateInfo* inCreateInfo = nullptr);

//-----------------------------------------------------------------------------
/// A class used to wrap D3D12's ID3D12DescriptorHeap interface.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12DescriptorHeap : public ID3D12DescriptorHeap
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor accepts the real runtime instance.
    /// \param inRealDescriptorHeap The real runtime instance being wrapped.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12DescriptorHeap(ID3D12DescriptorHeap* inRealDescriptorHeap) { mRealDescriptorHeap = inRealDescriptorHeap; }

    //-----------------------------------------------------------------------------
    /// Default destructor.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12DescriptorHeap() {}

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

    // ID3D12DescriptorHeap
    virtual D3D12_DESCRIPTOR_HEAP_DESC STDMETHODCALLTYPE GetDesc();
    virtual D3D12_CPU_DESCRIPTOR_HANDLE STDMETHODCALLTYPE GetCPUDescriptorHandleForHeapStart();
    virtual D3D12_GPU_DESCRIPTOR_HANDLE STDMETHODCALLTYPE GetGPUDescriptorHandleForHeapStart();

    ID3D12DescriptorHeap* mRealDescriptorHeap;      ///< The real runtime instance being wrapped.
};

#endif // WRAPPED_ID3D12DESCRIPTORHEAP_H