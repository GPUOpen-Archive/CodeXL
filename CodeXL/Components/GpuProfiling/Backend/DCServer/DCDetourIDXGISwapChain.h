//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Currently not used
//==============================================================================

#ifndef _DC_DETOUR_IDXGISWAPCHAIN_H_
#define _DC_DETOUR_IDXGISWAPCHAIN_H_

#include <windows.h>
#include <d3d11.h>

/// \addtogroup DCDetour
// @{

/// Detour IDXGISwapChain member functions
/// \param pSwapChain Pointer to IDXGISwapChain
/// \return true if detoured
bool DetourAttachIDXGISwapChain(IDXGISwapChain* pSwapChain);

/// Restore detoured member functions
/// \return true if detached
bool DetourDetachIDXGISwapChain();

// @}

#endif // _DC_DETOUR_IDXGISWAPCHAIN_H_