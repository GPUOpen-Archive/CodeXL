//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file appQtApplication.h
///
//==================================================================================

#ifndef __APPQTAPPLICATION_H
#define __APPQTAPPLICATION_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <AMDTApplication/inc/appApplicationDLLBuild.h>

// Declare osExceptionCode as a meta types for Qt to be able to queue its instances:
Q_DECLARE_METATYPE(osExceptionCode);

// ----------------------------------------------------------------------------------
// Class Name:          appQtApplication : public QApplication
// General Description: inherits the QApplication to enable overwriting virtual functions
//
// Author:                Gilad Yarnitzky
// Creation Date:        30/5/2012
// ----------------------------------------------------------------------------------
class APP_API appQtApplication : public QApplication
{
    Q_OBJECT

public:
    appQtApplication(int& argc, char** argv);
    virtual ~appQtApplication();

    virtual bool notify(QObject* pReceiver, QEvent* pEvent);

    /// Static function for handling the out of memory. the set_new_handler handler
    static void AppMemAllocFailureHandler();

    /// Static function for handling client the out of memory. the set_new_handler handler
    static void ClientMemAllocFailureHandler();

public slots:
    void UnhandledExceptionHandler(osExceptionCode exceptionCode, void* pExceptionContext);

    // Handle the signal for AppMemAllocFailureSignal
    void OnAppMemAllocFailureSignal();

    // Handle the client ClientMemAllocFailureSignal
    void OnClientMemAllocFailureSignal();

private:
    void updateMenuUIbyKeyEvent(QMenu* pMenu, QKeyEvent* pKeyEvent);

protected slots:
    void deleteSingletonDialog();

signals:
    /// App memory allocation failure signal:
    void AppMemAllocFailureSignal();

    /// client memory allocation failure signal:
    void ClientMemAllocFailureSignal();
};

#endif // __APPQTAPPLICATION_H
