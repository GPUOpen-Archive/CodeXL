//=====================================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \file
/// \brief DXGIFactory2 interception
//=====================================================================================

#ifndef DXGI_HOOKDXGI_FACTORY2_H
#define DXGI_HOOKDXGI_FACTORY2_H

#include <windows.h>
#include <dxgi1_4.h>
#include "../Common/LayerManager.h"

void HookCreateDXGIFactory2Function(HMODULE hDXGI);
void UnhookCreateDXGIFactory2Function(HMODULE hDXGI);

#ifdef DLL_REPLACEMENT
    //-----------------------------------------------------------------------------
    /// ReplaceCreateDXGIFactory2Function
    /// Setup the real CreateDXGIFactory2 function
    ///
    /// \param hDXGI The handle to the real DXGI dll
    //-----------------------------------------------------------------------------
    void ReplaceCreateDXGIFactory2Function(HMODULE hDXGI);
#endif

#endif // DXGI_HOOKDXGI_FACTORY2_H