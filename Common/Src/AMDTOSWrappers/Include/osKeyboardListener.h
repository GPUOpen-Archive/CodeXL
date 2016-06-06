//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osKeyboardListener.h
///
//=====================================================================

//------------------------------ osKeyboardListener.h ------------------------------

#ifndef __OSKEYBOARDLISTENER
#define __OSKEYBOARDLISTENER

#include <functional>
#include <memory>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>

#define OS_KEYBOARD_LISTENER osKeyboardListener::Instance()

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#include <windows.h>

//singleton class that intercepts keyboard events from current(main) thread main window
class OS_API osKeyboardListener
{
public:
    //receives keyboard virtual code https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
    //Other parameters , read : https://msdn.microsoft.com/en-us/library/windows/desktop/ms644984(v=vs.85).aspx
    //function must be executed immediately, preferably in separate thread
    typedef class OS_API std::function<void(_In_ int code, _In_ WPARAM wParam, _In_ LPARAM lParam)> OnKeyboardPressed;

    virtual ~osKeyboardListener();

    static osKeyboardListener& Instance();

    //Set callback function,which is to be called from keyboard pressed Hook
    void                SetOnKbPressedCallback(OnKeyboardPressed callback);
    //Getter for callback function
    OnKeyboardPressed   GetOnKbPressedCallback() {return  m_callback;}
    //Getter for hook pointer
    HHOOK               GetKbHook();

private:
    osKeyboardListener();

    //Members
private:
    //hook pointer, to be used when calling original function
    HHOOK m_KeyboardProcHook;
    //call back to be used on keyboard event
    OnKeyboardPressed m_callback;


};
//Li
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
#include <thread>

class OS_API osKeyboardListener
{
public:
    //read for possible input values for the callback:
    //https://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/tree/include/uapi/linux/input-event-codes.h
    typedef class OS_API std::function<void(const unsigned int code)> OnKeyboardPressed;
public:
    virtual ~osKeyboardListener();

    static osKeyboardListener& Instance();
    //Set callback function,which is to be called from keyboard pressed Hook
    void   SetOnKbPressedCallback(OnKeyboardPressed callback);
    //Getter for callback function
    OnKeyboardPressed   GetOnKbPressedCallback() {return  m_callback;}
private:
    osKeyboardListener();
    void     Listen(int fd);
    int      GetKeyboardDescriptor();
private:
    bool m_Stop = false;
    //call back to be used on keyboard event
    OnKeyboardPressed m_callback = nullptr;
    std::unique_ptr<std::thread> m_pListener = nullptr;
};

#endif
#endif

