//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file Wrapped_ID3D12RootSignatureDeserializer.h
/// \brief A class used to wrap D3D12's ID3D12RootSignatureDeserializer interface.
//=============================================================================

#ifndef WRAPPED_ID3D12ROOTSIGNATUREDESERIALIZER_H
#define WRAPPED_ID3D12ROOTSIGNATUREDESERIALIZER_H

#include "DX12Defines.h"

class Wrapped_ID3D12RootSignatureDeserializerCreateInfo;

//-----------------------------------------------------------------------------
/// Utility function used to wrap the D3D12 interface.
/// \param inRealRootSignatureDeserializer The real runtime instance of the D3D12 interface.
/// \param inCreateInfo A structure containing the interface creation info.
/// \returns True if the interface was wrapped successfully.
//-----------------------------------------------------------------------------
bool WrapD3D12RootSignatureDeserializer(ID3D12RootSignatureDeserializer** inRealRootSignatureDeserializer, Wrapped_ID3D12RootSignatureDeserializerCreateInfo* inCreateInfo = nullptr);

//-----------------------------------------------------------------------------
/// A class used to wrap D3D12's ID3D12RootSignatureDeserializer interface.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12RootSignatureDeserializer : public ID3D12RootSignatureDeserializer
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor accepts the real runtime instance.
    /// \param inRealRootSignatureDeserializer The real runtime instance being wrapped.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12RootSignatureDeserializer(ID3D12RootSignatureDeserializer* inRealRootSignatureDeserializer) { mRealRootSignatureDeserializer = inRealRootSignatureDeserializer; }

    //-----------------------------------------------------------------------------
    /// Default destructor.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12RootSignatureDeserializer() {}

    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    //-----------------------------------------------------------------------------
    /// Retrieve the Root Signature description structure.
    /// \returns The Root Signature description structure.
    //-----------------------------------------------------------------------------
    virtual const D3D12_ROOT_SIGNATURE_DESC* STDMETHODCALLTYPE GetRootSignatureDesc() { return mRealRootSignatureDeserializer->GetRootSignatureDesc(); }

    ID3D12RootSignatureDeserializer* mRealRootSignatureDeserializer;        ///< The real runtime instance being wrapped.
};

#endif // WRAPPED_ID3D12ROOTSIGNATUREDESERIALIZER_H