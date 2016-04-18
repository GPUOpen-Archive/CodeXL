//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file Wrapped_ID3D12DeviceChild.h
/// \brief A class used to wrap D3D12's ID3D12DeviceChild interface.
//=============================================================================

#ifndef WRAPPED_ID3D12DEVICECHILD_H
#define WRAPPED_ID3D12DEVICECHILD_H

#include "DX12Defines.h"

//-----------------------------------------------------------------------------
/// A class used to wrap D3D12's ID3D12DeviceChild interface.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12DeviceChild : public ID3D12DeviceChild
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor accepts the real runtime instance.
    /// \param inRealDeviceChild The real runtime instance being wrapped.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12DeviceChild(ID3D12DeviceChild* inRealDeviceChild) { mRealDeviceChild = inRealDeviceChild; }

    //-----------------------------------------------------------------------------
    /// Default destructor.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12DeviceChild() {}

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

    ID3D12DeviceChild* mRealDeviceChild;        ///< The real runtime instance being wrapped.
};

#endif // WRAPPED_ID3D12DEVICECHILD_H