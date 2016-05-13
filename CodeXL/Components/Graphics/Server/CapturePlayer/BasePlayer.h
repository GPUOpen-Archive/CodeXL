//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Base class functionality for all derived players
//==============================================================================

#ifndef BASE_PLAYER_H
#define BASE_PLAYER_H

#ifdef WIN32
#include <Windows.h>
#else
#include <WinDefs.h>
typedef void* HINSTANCE;
#endif
#include "WindowBase.h"

/// This class specifies the functionality required by all derived players
class BasePlayer
{

public:

    /// Constructor
    BasePlayer() : m_pPlayerWindow(NULL)
    {
    }

    /// Destructor
    ~BasePlayer()
    {
    }

    /// Overriden in derived class to initialize a window for a specific OS.
    /// \param hInstance Application instance
    /// \param windowWidth The width of the player window
    /// \param windowHeight The height of the player window
    virtual bool InitializeWindow(HINSTANCE hInstance, UINT windowWidth, UINT windowHeight) = 0;

    /// Overriden in derived class to initialize the graphics required for a render loop. The render loop acts as a message pump to the user clients.
    virtual bool InitializeGraphics() = 0;

    /// Overriden in derived class to implement the render loop.
    virtual void RenderLoop() = 0;

    /// Overriden in derived class to cleanup after use.
    virtual void Destroy() = 0;

protected:

    /// Stores the instance of the player window
    WindowBase* m_pPlayerWindow;

    /// Static function to be called from the OS mechanism for triggering a rendering.
    static void Present();

};

#endif //BASE_PLAYER_H