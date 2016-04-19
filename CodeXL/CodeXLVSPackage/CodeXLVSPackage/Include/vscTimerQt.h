//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscTimerQt.h
///
//==================================================================================

#ifndef vscTimerQt_h__
#define vscTimerQt_h__

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Core:
#include <Include/Public/vscWindowsManager.h>
#include <Include/Public/CoreInterfaces/IVscTimerOwner.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afDocUpdateManager.h>

// KA
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>

// We have reports from users of Visual Studio 2010 and 2012 that CodeXL VS Extension was slowing it down, so use a
// long enough interval that will not affect VS performance
#define VSP_TIMER_TICK_LENGTH_MS 2500

class vscTimer : public QObject
{
    Q_OBJECT
public:
    vscTimer();
    ~vscTimer();

    gtString m_commandPathUnedited;
    gtString m_previousExecPath;
    gtString m_previousWorkDir;
    gtString m_previousCmdArgs;
    gtString m_previousExecEnv;
    gtString m_newExecPath;
    gtString m_newWorkDir;
    gtString m_newCmdArgs;
    gtString m_newExecEnv;
    gtString m_activeDocPath;

    IVscTimerOwner* m_pOwner;
    QTimer* m_pQtTimer;

    void SetOwner(IVscTimerOwner* pOwner) { m_pOwner = pOwner; }

private:
    // Need to update the acMessagebox parent window. This is done only once.
    bool m_isUpdatedMessageBoxHWND;
    HWND m_lastActiveWindowHwnd;

public slots:
    void vscOnClockTick();
};
#endif // vscTimerQt_h__
