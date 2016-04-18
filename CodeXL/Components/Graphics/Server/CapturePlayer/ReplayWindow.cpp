//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file ReplayWindow.cpp
/// \brief An empty window used for replaying API Frame Capture files.
//==============================================================================

#include "ReplayWindow.h"

const wchar_t* kWindowClassName = L"APIReplayWindow";
const wchar_t* kWindowTitle = L"Capture Player - [PLACEHOLDER].ACR";

//--------------------------------------------------------------------------
/// Default constructor for ReplayWindow.
//--------------------------------------------------------------------------
ReplayWindow::ReplayWindow(UINT32 inWidth, UINT32 inHeight)
    : mWidth(inWidth)
    , mHeight(inHeight)
    , mWindowHandle(NULL)
    , mhInstance(NULL)
{
    mWindowClass = {};
}

//--------------------------------------------------------------------------
/// Default destructor for ReplayWindow.
//--------------------------------------------------------------------------
ReplayWindow::~ReplayWindow()
{
}

//--------------------------------------------------------------------------
/// Create a new window and prepare it for use.
/// \param inHInstance The HINSTANCE for the running application.
/// \param innShowCmd The nShowCmd we get from WinMain.
/// \returns True if initialization is successful.
//--------------------------------------------------------------------------
bool ReplayWindow::Initialize(HINSTANCE inHinstance, int innShowCmd, WNDPROC inWndProc)
{
    PS_UNREFERENCED_PARAMETER(innShowCmd);

    bool bInitializedSuccessfully = false;

    mWindowClass = {};
    mWindowClass.cbSize = sizeof(WNDCLASSEX);

    mWindowClass.style = CS_HREDRAW | CS_VREDRAW;
    mWindowClass.lpfnWndProc = inWndProc;
    mWindowClass.cbClsExtra = 0;
    mWindowClass.cbWndExtra = 0;
    mWindowClass.hInstance = inHinstance;
    mWindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    mWindowClass.lpszClassName = kWindowClassName;

    if (RegisterClassEx(&mWindowClass) != 0)
    {
        mWindowHandle = CreateWindow(kWindowClassName, kWindowTitle, WS_OVERLAPPEDWINDOW,
                                     CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, inHinstance, NULL);

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

//--------------------------------------------------------------------------
/// Shut down and clean up resources associated with a ReplayWindow instance.
/// \returns True if cleanup and shutdown was successful.
//--------------------------------------------------------------------------
bool ReplayWindow::Shutdown()
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

//--------------------------------------------------------------------------
/// Open an initialized window in the system UI.
//--------------------------------------------------------------------------
bool ReplayWindow::OpenAndUpdate(int inNCmdShow)
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