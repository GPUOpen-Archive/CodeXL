//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file Wrapped_ID3D12Fence.h
/// \brief A class used to wrap D3D12's ID3D12Fence interface.
//=============================================================================

#ifndef WRAPPED_ID3D12FENCE_H
#define WRAPPED_ID3D12FENCE_H

#include "DX12Defines.h"

class Wrapped_ID3D12Device;
class Wrapped_ID3D12FenceCreateInfo;

//-----------------------------------------------------------------------------
/// Utility function used to wrap the D3D12 interface.
/// \param inParentDevice The parent device for the interface.
/// \param inRealFence The real runtime instance of the D3D12 interface.
/// \param inCreateInfo A structure containing the interface creation info.
/// \returns True if the interface was wrapped successfully.
//-----------------------------------------------------------------------------
bool WrapD3D12Fence(Wrapped_ID3D12Device* inParentDevice, ID3D12Fence** inRealFence, Wrapped_ID3D12FenceCreateInfo* inCreateInfo = nullptr);

//-----------------------------------------------------------------------------
/// A class used to wrap D3D12's ID3D12Fence interface.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12Fence : public ID3D12Fence
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor accepts the real runtime instance.
    /// \param inRealFence The real runtime instance being wrapped.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12Fence(ID3D12Fence* inRealFence) { mRealFence = inRealFence; }

    //-----------------------------------------------------------------------------
    /// Default destructor.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12Fence() {}

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

    // ID3D12Fence
    virtual UINT64 STDMETHODCALLTYPE GetCompletedValue();
    virtual HRESULT STDMETHODCALLTYPE SetEventOnCompletion(UINT64 Value, HANDLE hEvent);
    virtual HRESULT STDMETHODCALLTYPE Signal(UINT64 Value);

    ID3D12Fence* mRealFence;        ///< The real runtime instance being wrapped.
};

#endif // WRAPPED_ID3D12FENCE_H