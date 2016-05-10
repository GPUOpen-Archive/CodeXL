//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Windows specialization of the base player functionality
//==============================================================================

#ifndef WINDOWS_PLAYER_H
#define WINDOWS_PLAYER_H

#include <d3d12.h>
#include <dxgi1_4.h>
#include "BasePlayer.h"
#include "WindowsWindow.h"

/// Windows specialization of the base player functionality
class WindowsPlayer: public BasePlayer
{

protected:

    /// Width of the player window
    UINT m_windowWidth;

    /// Height of the player window
    UINT m_windowHeight;

    /// Stores the instance of the player window
    WindowsWindow* m_pPlayerWindow;

public:

    /// Constructor
    WindowsPlayer(): m_windowWidth(0), m_windowHeight(0), m_pPlayerWindow(NULL)
    {
    }

    /// Destructor
    ~WindowsPlayer()
    {
    }
};


#endif //WINDOWS_PLAYER_H