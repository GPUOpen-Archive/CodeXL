//=====================================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \file
/// \brief DXGI Factory function hooking functions
//=====================================================================================

#ifndef DXGI_HOOKDXGI_FACTORY_H
#define DXGI_HOOKDXGI_FACTORY_H

#include <windows.h>
#include <DXGI.h>
#include "../Common/LayerManager.h"

void HookCreateDXGIFactoryFunction(HMODULE hDXGI);
void UnhookCreateDXGIFactoryFunction(HMODULE hDXGI);

DWORD HookIDXGIFactory(IDXGIFactory* pFac);
DWORD UnhookIDXGIFactory();

#ifdef DLL_REPLACEMENT

    //-----------------------------------------------------------------------------
    /// ReplaceCreateDXGIFactoryFunction
    /// Setup the real CreateDXGIFactory function
    ///
    /// \param hDXGI The handle to the real DXGI dll
    //-----------------------------------------------------------------------------
    void ReplaceCreateDXGIFactoryFunction(HMODULE hDXGI);
#endif

#endif // DXGI_HOOKDXGI_FACTORY_H
