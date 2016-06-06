//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaHiddenWindow.cpp
///
//=====================================================================

//------------------------------ oaHiddenWindow.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaDeviceContext.h>
#include <AMDTOSAPIWrappers/Include/oaHiddenWindow.h>

// Is used to give a different id to each window class:
static int stat_windowClassNameCounter = 0;


// ---------------------------------------------------------------------------
// Name:        oaWindowDefaultWinMsgHandlingProcedure
// Description:
//  Receives and handles window messages.
//
// Arguments:
//  hWnd - A handle to the window to which the message was sent.
//  message - The id of the sent message.
//  wParam, lParam - Message parameters (depends on the message sent).
//
// Return Val:
//  The result of the message processing (depends on the message sent).
// ---------------------------------------------------------------------------
LRESULT APIENTRY oaWindowDefaultWinMsgHandlingProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Will get the return value of this function:
    LRESULT retVal = 0;

    // Contains true iff we should call the default window procedure for the input message:
    bool shouldCallDefaultWinProc = false;

    switch (message)
    {
        case WM_CREATE: // Window is created:
        {
        }
        break;

        case WM_DESTROY: // Window is destroyed:
        {
            // Notify the system that this thread is about to terminate:
            PostQuitMessage(0);
        }
        break;

        case WM_SIZE: // Window size is changed:
        {
        }
        break;

        case WM_PAINT: // Window needs to be repainted:
        {
            // Prepare the window for painting:
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);

            // TO_DO: Repaint the window ...

            // Window painting was ended:
            EndPaint(hWnd, &ps);
        }
        break;

        case WM_CHAR: // Keyboard key was pressed:
        {
            // Get the pressed key (as a char)
            int pressedKey = (int)wParam;

            // Handle the key event:
            // TO_DO: ...
            GT_UNREFERENCED_PARAMETER(pressedKey);
        }
        break;

        case WM_LBUTTONDOWN: // Mouse left button was pressed:
        {
            // TO_DO ...
        }
        break;

        case WM_RBUTTONDOWN: // Mouse right button was pressed:
        {
            // TO_DO ...
        }
        break;

        case WM_LBUTTONUP: // Mouse left button was released:
        {
            // TO_DO ...
        }
        break;

        case WM_RBUTTONUP: // Mouse right button was released:
        {
            // TO_DO ...
        }
        break;

        case WM_MOUSEMOVE: // The mouse was moved:
        {
            // Get the mouse current position (after the movement):
            int mouseCurrentPosX = ((int) LOWORD(lParam) << 16) >> 16;
            int mouseCurrentPosY = ((int) HIWORD(lParam) << 16) >> 16;

            // TO_DO ...
            GT_UNREFERENCED_PARAMETER(mouseCurrentPosX);
            GT_UNREFERENCED_PARAMETER(mouseCurrentPosY);
        }
        break;

        default:
        {
            shouldCallDefaultWinProc = true;
        }
        break;
    }

    if (shouldCallDefaultWinProc)
    {
        // Call the default window procedure:
        retVal = DefWindowProc(hWnd, message, wParam, lParam);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::oaHiddenWindow
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        6/12/2003
// ---------------------------------------------------------------------------
oaHiddenWindow::oaHiddenWindow()
    : _hWindow(NULL), _pDeviceContext(NULL), _hPixelFormat(OA_NO_PIXEL_FORMAT_HANDLE)
{
}


// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::~oaHiddenWindow
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        6/12/2003
// ---------------------------------------------------------------------------
oaHiddenWindow::~oaHiddenWindow()
{
    destroy();
}


// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::create
//
// Description: Creates the Win32 window that this class wraps.
//
// Arguments:   title - The window title.
//              originX - The window origin X coordinate.
//              originY - The window origin Y coordinate.
//              width - The window width.
//              height - The window height.
//
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        6/12/2003
// ---------------------------------------------------------------------------
bool oaHiddenWindow::create(const gtString& title, int originX, int originY, int width, int height)
{
    bool retVal = false;

    // Get the the handle of the module (dll / exe) that contains this code:
    HINSTANCE hThisModule = GetModuleHandle(NULL);

    // Generate a unique name for this window class:
    _windowClassName = L"oaHiddenWindow window class name ";
    _windowClassName.appendFormattedString(L"%d", stat_windowClassNameCounter++);

    // Register the window class:
    WNDCLASSEX wndClass;
    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;            // Redraw OnSize, And OwnDC For Window.
    wndClass.lpfnWndProc = DefWindowProc;                           // Windows message handling function
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hThisModule;
    wndClass.hIcon = NULL;                                          // TO_DO: LoadIcon(hAppModuleInstance, (LPCTSTR)IDI_TP_APPLICATION_ICON_32);
    wndClass.hIconSm = NULL;                                        // TO_DO: LoadIcon(hAppModuleInstance, (LPCTSTR)IDI_TP_APPLICATION_ICON);
    wndClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);               // Load The default arrow Pointer;
    wndClass.hbrBackground = NULL;                                  // No Background Required (OpenGL fills it).
    wndClass.lpszMenuName = NULL;                                   // No menu please.
    wndClass.lpszClassName = _windowClassName.asCharArray();        // The window class Name
    ::RegisterClassEx(&wndClass);

    // Create the window itself:
    _hWindow = ::CreateWindow(_windowClassName.asCharArray(), title.asCharArray(),
                              WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                              originX, originY, width, height, NULL, NULL, hThisModule, NULL);

    GT_IF_WITH_ASSERT(_hWindow != NULL)
    {
        // Build a wrapper class to this window device context:
        _pDeviceContext = new oaDeviceContext(_hWindow, this);
        GT_IF_WITH_ASSERT(_pDeviceContext != NULL)
        {
            retVal = true;
        }
    }
    else
    {
        // Failure clean up - unregister the window class:
        ::UnregisterClass(_windowClassName.asCharArray(), hThisModule);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::destroy
// Description: Destroys the Win32 window.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        2/1/2006
// ---------------------------------------------------------------------------
bool oaHiddenWindow::destroy()
{
    bool retVal = false;

    BOOL rc1 = TRUE;
    BOOL rc2 = TRUE;

    // If we have a device context:
    if (_pDeviceContext != NULL)
    {
        // Delete the device context wrapper class and release the OS device context:
        delete _pDeviceContext;
        _pDeviceContext = NULL;
    }

    // If the Win32 window was created:
    if (_hWindow)
    {
        // Destroy it:
        rc1 = ::DestroyWindow(_hWindow);
        GT_ASSERT(rc1 == TRUE);
        _hWindow = NULL;
    }

    // If we registered this window class:
    if (!_windowClassName.isEmpty())
    {
        // Get the the handle of the module (dll / etc) that contains this code:
        HINSTANCE hThisModule = GetModuleHandle(NULL);

        // Unregister the window class:
        rc2 = ::UnregisterClass(_windowClassName.asCharArray(), hThisModule);
        GT_ASSERT(rc2 == TRUE);
        _windowClassName.makeEmpty();
    }

    retVal = ((rc1 != FALSE) && (rc2 != FALSE));
    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::showWindow
// Description: Shows the window.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        15/5/2006
// ---------------------------------------------------------------------------
bool oaHiddenWindow::showWindow()
{
    ::ShowWindow(_hWindow, SW_SHOW);
    return true;
}


// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::hideWindow
// Description: Hides the window.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        15/5/2006
// ---------------------------------------------------------------------------
bool oaHiddenWindow::hideWindow()
{
    ::ShowWindow(_hWindow, SW_HIDE);
    return true;
}

// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::setActivePixelFormat
// Description: Sets the active pixel format on this graphic device context.
// Arguments:   pixelFormatIndex - The index of the pixel format that should become
//                                 the active pixel format.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        6/12/2003
// ---------------------------------------------------------------------------
bool oaHiddenWindow::setActivePixelFormat(oaPixelFormatId pixelFormatIndex)
{
    bool retVal = false;

    // Sanity check
    if (_pDeviceContext != NULL)
    {
        oaDeviceContextHandle hDC = _pDeviceContext->nativeDeviceContextHandle();

        // Get the pixel format description:
        PIXELFORMATDESCRIPTOR pixelFormatDescriptior;
        int rc = ::DescribePixelFormat(hDC, pixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &pixelFormatDescriptior);
        GT_IF_WITH_ASSERT(rc != 0)
        {
            // Set it to be the active pixel format:
            BOOL rc2 = ::SetPixelFormat(hDC, pixelFormatIndex, &pixelFormatDescriptior);
            GT_IF_WITH_ASSERT(rc2 == TRUE)
            {
                retVal = true;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        oaHiddenWindow::getActivePixelFormat
// Description: Returns the index of the active pixel format.
// Arguments:   pixelFormatIndex - Will get the index of the active pixel format.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        6/12/2003
// ---------------------------------------------------------------------------
bool oaHiddenWindow::getActivePixelFormat(oaPixelFormatId& pixelFormatIndex) const
{
    bool retVal = false;

    // Sanity check
    if (_pDeviceContext != NULL)
    {
        oaDeviceContextHandle hDC = _pDeviceContext->nativeDeviceContextHandle();

        // Get the active pixel format index:
        pixelFormatIndex = ::GetPixelFormat(hDC);

        if (pixelFormatIndex != 0)
        {
            retVal = true;
        }
        else
        {
            pixelFormatIndex = OA_NO_PIXEL_FORMAT_ID;
        }
    }

    return retVal;
}
