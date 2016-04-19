//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspTimer.h
///
//==================================================================================

//------------------------------ vspTimer.h ------------------------------

#ifndef __VSPTIMER_H
#define __VSPTIMER_H
#include <Include/Public/CoreInterfaces/IVscTimerOwner.h>

// ----------------------------------------------------------------------------------
// Class Name:          vspTimer : public QObject
// General Description: This class is used for operation that needs to be performed
//                      on timer ticks
// Author:              Sigal Algranaty
// Creation Date:       24/5/2012
// ----------------------------------------------------------------------------------
class vspTimer : public IVscTimerOwner
{
public:
    vspTimer();
    ~vspTimer();
    void onClockTick();

private:
    // Region: IVcsTimerOwner - Begin.

    virtual bool vscTimerOwner_GetStartupProjectDebugInfo(wchar_t*& pExecutableFilePathBuffer, wchar_t*& pWorkingDirectoryPathBuffer,
                                                          wchar_t*& pCommandLineArgumentsBuffer, wchar_t*& pEnvironmentBuffer, bool& isProjectOpenBuffer, bool& isProjectTypeValidBuffer,
                                                          bool& isNonNativeProjectBuffer);

    virtual void vscTimerOwner_UpdateCommandShellUi()  const;

    virtual void vscTimerOwner_GetParentWindowHandle(HWND& winHandle)  const;

    virtual void vscTimerOwner_DeleteWCharStr(wchar_t*& pStr)  const;

    virtual void vscTimerOwner_UpdateProjectSettingsFromStartupInfo() const;

    virtual void vscTimerOwner_GetActiveDocumentFileFullPath(wchar_t*& pStrBuffer, bool& hasActiveDocBuffer) const;

    virtual bool vscTimerOwner_GetActiveWindowHandle(HWND& winHandle) const;

    // Region: IVcsTimerOwner - End.

    // Pointer to the vscTimer object
    void* m_pTimer;

    // Cached result of the queried executable path from the VS project settings
    std::wstring m_executableFilePath;

};

#endif //__VSPTIIMER_H

