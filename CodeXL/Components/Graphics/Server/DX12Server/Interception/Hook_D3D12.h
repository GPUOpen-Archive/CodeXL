//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   Hook_D3D12.h
/// \brief  Responsible for hooking D3D12 entrypoint functions.
//=============================================================================

#ifndef HOOK_D3D12_H
#define HOOK_D3D12_H

#include "../DX12Defines.h"     // For D3D12 symbols
#include "../../Common/misc.h"  // For RealAndMineHook

//-----------------------------------------------------------------------------
/// This class is responsible for hooking D3D12 module entrypoints. It contains
/// a "Mine" implementation for each hooked function.
//-----------------------------------------------------------------------------
class Hook_D3D12
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for Hook_D3D12.
    //-----------------------------------------------------------------------------
    Hook_D3D12();

    //-----------------------------------------------------------------------------
    /// Default destructor for Hook_D3D12.
    //-----------------------------------------------------------------------------
    virtual ~Hook_D3D12();

    //-----------------------------------------------------------------------------
    /// Attach API entrypoints for hooking.
    /// \returns True if entrypoints were successfully hooked.
    //-----------------------------------------------------------------------------
    bool HookInterceptor();

    //-----------------------------------------------------------------------------
    /// Detach all hooked API entrypoints.
    /// \returns True if entrypoints were successfully detached.
    //-----------------------------------------------------------------------------
    bool UnhookInterceptor();

#ifdef DLL_REPLACEMENT
    //-----------------------------------------------------------------------------
    /// Set the handle to the real D3D12.DLL module.
    /// \param hRealD3D12 The HINSTANCE handle to the real D3D12.DLL module.
    //-----------------------------------------------------------------------------
    void SetRealDllHandle(HINSTANCE hRealD3D12) { mRealD3D12 = hRealD3D12; }
#endif

    /// A hook used to intercept D3D12GetDebugInterface.
    RealAndMineHook<PFN_D3D12_GET_DEBUG_INTERFACE> mHook_GetDebugInterface;

    /// A hook used to intercept D3D12CreateDevice.
    RealAndMineHook<PFN_D3D12_CREATE_DEVICE> mHook_CreateDevice;

    /// A hook used to intercept D3D12SerializeRootSignature.
    RealAndMineHook<PFN_D3D12_SERIALIZE_ROOT_SIGNATURE> mHook_SerializeRootSignature;

    /// A hook used to intercept D3D12CreateRootSignatureDeserializer.
    RealAndMineHook<PFN_D3D12_CREATE_ROOT_SIGNATURE_DESERIALIZER> mHook_CreateRootSignatureDeserializer;

    /// Handle to real D3D12.dll module, when operating in DLL_REPLACEMENT mode.
    HINSTANCE mRealD3D12;
};

#endif // HOOK_D3D12_H