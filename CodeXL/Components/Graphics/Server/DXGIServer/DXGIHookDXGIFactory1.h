//=====================================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \file
/// \brief DXGIFactory1 interception
//=====================================================================================

#ifndef DXGI_HOOKDXGI_FACTORY1_H
#define DXGI_HOOKDXGI_FACTORY1_H

#include <windows.h>
#include <dxgi1_4.h>
#include "../Common/LayerManager.h"

/// Helper function to hook the CreateFactory1 function
void HookCreateDXGIFactory1Function(HMODULE hDXGI);

/// Helper function to unhook the CreateFactory1 function
void UnhookCreateDXGIFactory1Function(HMODULE hDXGI);

#ifdef DLL_REPLACEMENT
    //-----------------------------------------------------------------------------
    /// ReplaceCreateDXGIFactory1Function
    /// Setup the real CreateDXGIFactory1 function
    ///
    /// \param hDXGI The handle to the real DXGI dll
    //-----------------------------------------------------------------------------
    void ReplaceCreateDXGIFactory1Function(HMODULE hDXGI);
#endif

#endif // DXGI_HOOKDXGI_FACTORY1_H