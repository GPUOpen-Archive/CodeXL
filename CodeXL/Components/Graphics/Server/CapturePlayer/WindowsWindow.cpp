//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file WindowsWindow.cpp
/// \brief An empty window used for replaying API Frame Capture files.
//==============================================================================

#include "WindowsWindow.h"

const wchar_t* kWindowClassName = L"APIReplayWindow"; ///< Windows class name
const wchar_t* kWindowTitle = L"Capture Player - [PLACEHOLDER].ACR"; ///< Window title

/// Constructor.
/// \param inWidth The width of the player window
/// \param inHeight The height of the player window
/// \param inWndProc The application - defined function that processes messages sent to a window.Main message handler
WindowsWindow::WindowsWindow(UINT windowWidth, UINT windowHeight, HINSTANCE hInstance, WNDPROC inWndProc)
    : WindowBase(windowWidth, windowHeight)
    , mWindowHandle(NULL)
    , mhInstance(hInstance)
    , mWndProc(inWndProc)
{
    mWindowClass = {};
}

/// Destructor
WindowsWindow::~WindowsWindow()
{
}

/// Create a new window and prepare it for use.
/// \param inHinstance The HINSTANCE for the running application.
/// \returns True if initialization is successful.
bool WindowsWindow::Initialize()
{
    bool bInitializedSuccessfully = false;

    mWindowClass = {};
    mWindowClass.cbSize = sizeof(WNDCLASSEX);

    mWindowClass.style = CS_HREDRAW | CS_VREDRAW;
    mWindowClass.lpfnWndProc = mWndProc;
    mWindowClass.cbClsExtra = 0;
    mWindowClass.cbWndExtra = 0;
    mWindowClass.hInstance = mhInstance;
    mWindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    mWindowClass.lpszClassName = kWindowClassName;

    if (RegisterClassEx(&mWindowClass) != 0)
    {
        mWindowHandle = CreateWindow(kWindowClassName, kWindowTitle, WS_OVERLAPPEDWINDOW,
                                     CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, mhInstance, NULL);

        if (mWindowHandle != NULL)
        {
            bInitializedSuccessfully = true;
        }
        else
        {
            // @TODO: Log this as an error. Need to fix inclusion os OSWrappers within Player project to get this working.
            //Log(logERROR, "Failed to create a window for replaying capture.\n");
        }

    }
    else
    {
        // @TODO: Log this as an error. Need to fix inclusion os OSWrappers within Player project to get this working.
        //Log(logERROR, "Failed to register ReplayWindow's WindowClass.\n");
    }

    return bInitializedSuccessfully;
}

/// Shut down and clean up resources associated with a ReplayWindow instance.
/// \returns True if cleanup and shutdown was successful.
bool WindowsWindow::Shutdown()
{
    bool bSuccess = false;

    BOOL windowDestroyed = DestroyWindow(mWindowHandle);

    if (windowDestroyed != 0)
    {
        BOOL unregisterSuccessul = UnregisterClass(kWindowClassName, mhInstance);

        if (unregisterSuccessul != 0)
        {
            bSuccess = true;
        }
    }

    return bSuccess;
}

/// Open an initialized window in the system UI.
/// \param inNCmdShow Controls how the window is to be shown.
/// \return True if success, false if fail.
bool WindowsWindow::OpenAndUpdate(int inNCmdShow)
{
    bool bSuccessful = false;

    if (mWindowHandle != NULL)
    {
        // The window was created successfully. open
        ShowWindow(mWindowHandle, inNCmdShow);

        BOOL bUpdated = UpdateWindow(mWindowHandle);

        if (bUpdated == TRUE)
        {
            bSuccessful = true;
        }
    }

    return bSuccessful;
}

/// Update the window. This is the OS-dependent message loop
/// implementation so should be called periodically.
/// \return false if the message loop is to be terminated, true
/// otherwise.
bool WindowsWindow::Update()
{
    MSG msg = {};

    // Process any messages in the queue.
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_QUIT)
        {
            return false;
        }
    }
    return true;
}
