//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file Wrapped_ID3D12Object.h
/// \brief A class used to wrap D3D12's ID3D12Object interface.
//=============================================================================

#ifndef WRAPPED_ID3D12OBJECT_H
#define WRAPPED_ID3D12OBJECT_H

#include "DX12Defines.h"

//-----------------------------------------------------------------------------
/// A class used to wrap D3D12's ID3D12Object interface.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12Object : public ID3D12Object
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor accepts the real runtime instance.
    /// \param inRealObject The real runtime instance being wrapped.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12Object(ID3D12Object* inRealObject) { mRealObject = inRealObject; }

    //-----------------------------------------------------------------------------
    /// Default destructor.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12Object() {}

    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // ID3D12Object
    virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData);
    virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID guid, UINT DataSize, const void* pData);
    virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID guid, const IUnknown* pData);
    virtual HRESULT STDMETHODCALLTYPE SetName(LPCWSTR Name);

    ID3D12Object* mRealObject;      ///< The real runtime instance being wrapped.
};

#endif // WRAPPED_ID3D12OBJECT_H