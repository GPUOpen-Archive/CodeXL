//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afUnhandledExceptionHandler.h
///
//==================================================================================

#ifndef __AFUNHANDLEDEXCEPTIONHANDLER_H
#define __AFUNHANDLEDEXCEPTIONHANDLER_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osUnhandledExceptionHandler.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           GD_API afUnhandledExceptionHandler : public osUnhandledExceptionHandler
// General Description:
//  Catches unhandles exceptions that should of cause the CodeXL application to crash
//  and enables the user to report them to the CodeXL development team.
//
// Author:               Yaki Tebeka
// Creation Date:        21/4/2009
// ----------------------------------------------------------------------------------
class AF_API afUnhandledExceptionHandler : public QObject, public osUnhandledExceptionHandler
{
    Q_OBJECT

public:
    virtual ~afUnhandledExceptionHandler();
    static afUnhandledExceptionHandler& instance();

    // After this object emits the UnhandledExceptionSignal, it waits until another object
    // tells it to stop waiting. This mechanism allows us to handle scenarios where the uncaught
    // exception is thrown in a thread other than the main thread. In such scenarios, the main
    // thread (which handles the UnhandledExceptionSignal) opens up an error report dialog and
    // interacts with the user. This object, which might be running on a thread other than the
    // main thread, must allow the main thread to handle the queued event, and must not exit
    // before the main thread has finished interacting with the user.This function allows the
    // main thread to inform the other thread that it can and should now exit.
    void StopWaiting();

signals:
    void UnhandledExceptionSignal(osExceptionCode exceptionCode, void* pExceptionContext);

protected:
    virtual void onUnhandledException(osExceptionCode exceptionCode, void* pExceptionContext);

private:

    // Only my instance method should create me:
    afUnhandledExceptionHandler();
    bool m_shouldStopWaiting;

};

#endif //__afUnhandledExceptionHandler_H

