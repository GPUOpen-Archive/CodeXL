//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osKeyboardListener.cpp
///
//=====================================================================

//------------------------------ osKeyboardListener.cpp ------------------------------

// Local:
#include <AMDTOSWrappers/Include/osKeyboardListener.h>
#include <AMDTOSWrappers/Include/osProcess.h>

//Hook implementation
LRESULT CALLBACK KeyboardProc(_In_ int code, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    osKeyboardListener::OnKeyboardPressed callback = OS_KEYBOARD_LISTENER.GetOnKbPressedCallback();

    //if callback set
    if (callback)
    {
        callback(code, wParam, lParam);
    }

    return ::CallNextHookEx(OS_KEYBOARD_LISTENER.GetKbHook(), code, wParam, lParam);
}

osKeyboardListener::osKeyboardListener() : m_callback(nullptr)
{
    // Find the main thread for the current process.
    osThreadId mainThreadId;
    bool bGotMainThreadId = osGetMainThreadId(osGetCurrentProcessId(), mainThreadId);

    if (bGotMainThreadId)
    {
        // Hook the keypress listener on the main thread- it won't die until the application exits.
        m_KeyboardProcHook = ::SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, GetModuleHandle(NULL), mainThreadId);
    }
}

osKeyboardListener::~osKeyboardListener()
{
    ::UnhookWindowsHookEx(m_KeyboardProcHook);
}

//singleton implementation
osKeyboardListener& osKeyboardListener::Instance()
{
    static osKeyboardListener instance;
    return instance;
}

void osKeyboardListener::SetOnKbPressedCallback(OnKeyboardPressed callback)
{
    m_callback = callback;
}

HHOOK osKeyboardListener::GetKbHook()
{
    return m_KeyboardProcHook;
}
