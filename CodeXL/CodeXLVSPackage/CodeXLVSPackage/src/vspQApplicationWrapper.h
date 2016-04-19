//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspQApplicationWrapper.h
///
//==================================================================================

//------------------------------ vspQApplicationWrapper.h ------------------------------

#ifndef __VSPQAPPLICATIONWRAPPER_H
#define __VSPQAPPLICATIONWRAPPER_H

// Qt:
#include <QApplication>

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// ----------------------------------------------------------------------------------
// Class Name:          vspQApplicationWrapper : public QApplication
// General Description: Servers as an application wrapper for the Qt GUI system
// Author:              Sigal Algranaty
// Creation Date:       1/12/2011
// ----------------------------------------------------------------------------------
class vspQApplicationWrapper : public QApplication
{
    Q_OBJECT

public:
    vspQApplicationWrapper(int argc, char* argv[]);

    bool notify(QObject* receiver, QEvent* e);

    // add event filter to prevent tooltips when not in focus
    bool eventFilter(QObject* pObject, QEvent* pEvent);

    /// Set the active mdi window using the timer
    void SetMDIActiveWindow(HWND activeWindow);

public slots:
    void UnhandledExceptionHandler(osExceptionCode exceptionCode, void* pExceptionContext);

protected:
    bool shouldSetFocus(QWidget* pWidget, Qt::FocusPolicy policy);

    /// last active window from mdi
    QWidget* m_pActiveWidget;

    /// handle only one wheel event since we are sending it directly to the control under the cursor
    bool m_inWheelEvent;
};


#endif //__VSPQAPPLICATIONWRAPPER_H

