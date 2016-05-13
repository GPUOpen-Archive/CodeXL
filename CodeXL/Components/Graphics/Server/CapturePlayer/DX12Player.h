//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief DX12 specialization of the base player functionality
//==============================================================================

#ifndef DX12_PLAYER_H
#define DX12_PLAYER_H

#include <dxgi1_4.h>
#include "WindowsWindow.h"
#include "BasePlayer.h"

/// This class implements the features required for a DX12 capture player
class DX12Player: public BasePlayer
{
    /// Stores the current swapchain pointer
    IDXGISwapChain3* m_swapchain;

public:

    /// Constructor
    DX12Player(): m_swapchain(NULL)
    {
    }

    /// Destructor
    ~DX12Player()
    {
    }

    bool InitializeWindow(HINSTANCE hInstance, UINT windowWidth, UINT windowHeight);

    bool InitializeGraphics();

    void RenderLoop();

    void Destroy();
};


#endif //DX12_PLAYER_H