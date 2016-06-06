//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ osWin32Window.h ------------------------------

#ifndef __OSWIN32WINDOW
#define __OSWIN32WINDOW

// Win32:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// GRBaseTools:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTOSWrappers/Include/osWindow.h>


// ----------------------------------------------------------------------------------
// Class Name:           osWin32Window : public osWindow
// General Description:
//   Win32 implementation of osWindow
//
// Author:      AMD Developer Tools Team
// Creation Date:        6/12/2003
// ----------------------------------------------------------------------------------
class OS_API osWin32Window : public osWindow
{
public:
    osWin32Window(const gtString& title, int originX, int originY, int width, int height);
    virtual ~osWin32Window();

    // Overrides osWindow:
    virtual osGraphicDeviceContext* createDeviceContext() const;

private:
    bool createWin32Window(const gtString& title, int originX, int originY,
                           int width, int height);
    bool destroyWin32Window();

private:
    // The window win32 class name:
    gtString _windowClassName;

    // The window instance handle:
    HINSTANCE _hWindowInstance;

    // The window win32 handle:
    HWND _hWnd;

    // A static counter that is used to create uniqe windows class names:
    static int _windowClassNameCounter;
};


#endif  // __OSWIN32WINDOW
