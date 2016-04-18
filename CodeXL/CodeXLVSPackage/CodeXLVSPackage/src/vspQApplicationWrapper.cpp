//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspQApplicationWrapper.cpp
///
//==================================================================================

//------------------------------ vsQApplicationWrapper.cpp ------------------------------

#include "StdAfx.h"

#include <QtGui>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osCallsStackReader.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTApplicationComponents/Include/acSendErrorReportDialog.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afUnhandledExceptionHandler.h>

// Local:
#include <src/vspQApplicationWrapper.h>
#include <src/vspWindowsManager.h>
#include <src/vspQTWindowPaneImpl.h>
#include <Include/Public/vscWindowsManager.h>
#include <Include/Public/vscToolWindow.h>
#include <src/vspProgressBarWrapper.h>


// ---------------------------------------------------------------------------
// Name:        vspQApplicationWrapper::vspQApplicationWrapper
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        1/12/2011
// ---------------------------------------------------------------------------
vspQApplicationWrapper::vspQApplicationWrapper(int argc, char* argv[])
    : QApplication(argc, argv), m_pActiveWidget(nullptr), m_inWheelEvent(false)
{
    // Register osExceptionCode as a meta-type for Qt. This allows Qt to queue instances
    //  of that type, which will be required to handle the exceptions across threads.
    qRegisterMetaType<osExceptionCode>("osExceptionCode");

    // Register this object which runs in the main thread as the top-level handler of exception events in Qt.
    afUnhandledExceptionHandler& unhandledExceptionsHandler = afUnhandledExceptionHandler::instance();
    bool rc = connect(&unhandledExceptionsHandler, SIGNAL(UnhandledExceptionSignal(osExceptionCode, void*)),
                      this, SLOT(UnhandledExceptionHandler(osExceptionCode, void*)), Qt::AutoConnection);
    GT_ASSERT(rc);
}


// ---------------------------------------------------------------------------
// Name:        vspQApplicationWrapper::notify
// Description:
// Arguments:   QObject *receiver
//              QEvent *e
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        19/12/2011
// ---------------------------------------------------------------------------
bool vspQApplicationWrapper::notify(QObject* receiver, QEvent* e)
{
    bool notifyResult = false;

    // Explanation for capturing the mouse events in the derived Qt application:
    // When Qt creates the mouse events it takes the global coordinates and transfer them into local
    // using its mapFromParent function and not the win32 API. In case that the final parent is not a Qt widget
    // this mechanism fails to find the correct local coordinates. In this case the global coordinates are taken
    // and converted locally using the win32 API, a new event is created (it is not possible to update the passed event)
    // send the new event, and delete it when done
    // In all other cases the event is simply forwarded to the parent class
    if (e->type() ==  QEvent::MouseButtonPress || e->type() ==  QEvent::MouseButtonRelease ||
        e->type() ==  QEvent::MouseButtonDblClick || e->type() ==  QEvent::MouseMove)
    {
        QMouseEvent* pMouseEvent = dynamic_cast<QMouseEvent*>(e);
        QWindow* pTargetWindow = qobject_cast<QWindow*>(receiver);
        QWidget* pTargetWidget = qobject_cast<QWidget*>(receiver);

        if (pMouseEvent != NULL && (pTargetWidget != NULL || pTargetWindow != NULL))
        {
            // Get the new correct position:
            // Get the global position:

            POINT globalPos;
            globalPos.x = pMouseEvent->globalX();
            globalPos.y = pMouseEvent->globalY();

            // Get target HWND
            HWND targetWin32;

            if (pTargetWindow != NULL)
            {
                targetWin32 = (HWND)pTargetWindow->winId();
            }
            else
            {
                targetWin32 = (HWND)pTargetWidget->winId();
            }

            // convert to local position
            ::ScreenToClient(targetWin32, &globalPos);
            QPoint  correctLocalPos(globalPos.x, globalPos.y);

            // Create a new corrected mouse event
            QMouseEvent* pCorrectedMouseEvent = new QMouseEvent(e->type(), correctLocalPos, pMouseEvent->globalPos(),
                                                                pMouseEvent->button(), pMouseEvent->buttons(), pMouseEvent->modifiers());

            notifyResult = QApplication::notify(receiver, pCorrectedMouseEvent);

            delete pCorrectedMouseEvent;

            // Check if needs to send focus event (simulate Qt mechanism defined in QApplicationPrivate::giveFocusAccordingToFocusPolicy && QApplicationPrivate::shouldSetFocus:
            if (pTargetWidget != NULL)
            {
                if (pMouseEvent->spontaneous() && pTargetWidget->isEnabled() && e->type() == QEvent::MouseButtonPress)
                {
                    QWidget* pFocusWidget = pTargetWidget;

                    while (pFocusWidget)
                    {
                        if (pFocusWidget->isEnabled() && shouldSetFocus(pFocusWidget, Qt::ClickFocus))
                        {
                            pFocusWidget->setFocus(Qt::MouseFocusReason);
                            break;
                        }

                        if (pFocusWidget->isWindow())
                        {
                            break;
                        }

                        pFocusWidget = pFocusWidget->parentWidget();
                    }
                }

                if (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonDblClick)
                {
                    vscToolWindow::CallToolShowFunction(pTargetWidget);
                }
            }
        }
    }
    else if ((e->type() == QEvent::ApplicationActivate) || (e->type() == QEvent::ApplicationDeactivated) || (e->type() == QEvent::WindowActivate))
    {
        if ((e->type() == QEvent::ApplicationActivate) || (e->type() == QEvent::ApplicationDeactivated))
        {
            vspQTWindowPaneImpl::s_isApplicationActive = (e->type() == QEvent::ApplicationActivate);
        }

        if ((e->type() == QEvent::ApplicationActivate) || (e->type() == QEvent::WindowActivate))
        {
            // bring progress dialog to front
            bool progressDlg = vscProgressBarWrapper::instance().IsDlgShown();

            if (progressDlg)
            {
                vscProgressBarWrapper::instance().BringToFront();
            }
        }
    }
    else if (e->type() == QEvent::Wheel && m_pActiveWidget != nullptr && !m_inWheelEvent)
    {
        // the first wheel event we need to transfer to our qt control, after that we let qt take control
        m_inWheelEvent = true;
        QWheelEvent* pWheelEvent = dynamic_cast<QWheelEvent*>(e);

        // if it is a wheel event and we have an active window check with it which widget should get the event
        if (pWheelEvent != nullptr)
        {
            QPoint eventPosLocal = m_pActiveWidget->mapFromGlobal(pWheelEvent->globalPos());
            // get the lowest child
            QWidget* pTargetObject = m_pActiveWidget->childAt(eventPosLocal);
            bool foundLowest = (pTargetObject == nullptr);

            while (!foundLowest)
            {
                eventPosLocal = pTargetObject->mapFromGlobal(pWheelEvent->globalPos());
                QWidget* pChild = pTargetObject->childAt(eventPosLocal);

                if (pChild != nullptr)
                {
                    pTargetObject = pChild;
                }

                foundLowest = (pChild == nullptr);
            }

            if (pTargetObject != nullptr)
            {
                // Create a new corrected wheel event
                QWheelEvent* pCorrectedWheelEvent = new QWheelEvent(eventPosLocal, pWheelEvent->globalPos(),
                                                                    pWheelEvent->pixelDelta(), pWheelEvent->angleDelta(), pWheelEvent->angleDelta().ry(), pWheelEvent->orientation(), pWheelEvent->buttons(), Qt::NoModifier, pWheelEvent->phase());

                notifyResult = QApplication::notify(pTargetObject, pCorrectedWheelEvent);

                delete pCorrectedWheelEvent;

            }
        }

        m_inWheelEvent = false;
    }
    else
    {
        notifyResult = QApplication::notify(receiver, e);
    }

    return notifyResult;
}

bool vspQApplicationWrapper::eventFilter(QObject* pObject, QEvent* pEvent)
{
    // by default do not filter anything
    bool retVal = false;

    if (pEvent != nullptr && ((pEvent->type() == QEvent::ToolTip || pEvent->type() == QEvent::WhatsThis)) && !vspQTWindowPaneImpl::s_isApplicationActive == true)
    {
        retVal = true;
    }
    else
    {
        retVal = QApplication::eventFilter(pObject, pEvent);
    }

    return retVal;

}

// ---------------------------------------------------------------------------
// Name:        vspQApplicationWrapper::UnhandledExceptionHandler
// Description: This handler will be the application's top-level exception handler
// Arguments:   osExceptionCode exceptionCode - the unhandled exception's code
//              void* pExceptionContext - the unhandled exception's context
// Return Val:  void
// Author:      Amit Ben-Moshe
// Date:        19/10/2014
// ---------------------------------------------------------------------------
void vspQApplicationWrapper::UnhandledExceptionHandler(osExceptionCode exceptionCode, void* pExceptionContext)
{
    // Get the exception's associated call stack:
    osCallStack exceptionCallStack;
    osCallsStackReader callStackReader;

    bool gotExceptionCallStack = callStackReader.getCallStack(exceptionCallStack, pExceptionContext, false);
    GT_ASSERT(gotExceptionCallStack);

    bool isAMDRelatedCallStack = false;
    gtString stackString;
    gtString ignoredStackBriefString;
    exceptionCallStack.asString(ignoredStackBriefString, stackString, isAMDRelatedCallStack, false);

    if (!isAMDRelatedCallStack)
    {
        stackString.toLowerCase();
        isAMDRelatedCallStack = (-1 != stackString.find(L"\\amdt")) || (-1 != stackString.find(L"\\codexl"));
    }

    if (isAMDRelatedCallStack)
    {
        // Get the application commands object. Note that an assertion here means we
        // got a crash before the class got registered on loading or after it got killed on exit.
        afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
        GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
        {
            // Check if this needs to be reported:
            if (pApplicationCommands->shouldReportClientApplicationCrash(exceptionCallStack))
            {
                // Display the exception to the user:
                QPixmap iconPixMap;
                acSetIconInPixmap(iconPixMap, afGlobalVariablesManager::ProductIconID(), AC_64x64_ICON);
                acSendErrorReportDialog sendErrorReportDialog(nullptr, afGlobalVariablesManager::ProductNameA(), iconPixMap);

                bool allowDifferentSystemPath = false;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                // On windows, the remote process debugger is used for 64-bit debugging. We should allow the different system file path in this case:
                allowDifferentSystemPath = afCanAllowDifferentSystemPath();
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

                // Display the dialog to the user:
                sendErrorReportDialog.onUnhandledException(exceptionCode, pExceptionContext, allowDifferentSystemPath);

                // Exit the application (this prevents the OS error report dialog from appearing):
                osExitCurrentProcess(0);
            }
        }
    }

    // Inform the global exceptions handler to stop waiting.
    afUnhandledExceptionHandler::instance().StopWaiting();
}


// ---------------------------------------------------------------------------
// Name:        vspQApplicationWrapper::shouldSetFocus
// Description: Check the focus policy of the widget
// Author:      Gilad Yarnitzky
// Date:        28/6/2012
// ---------------------------------------------------------------------------
bool vspQApplicationWrapper::shouldSetFocus(QWidget* pWidget, Qt::FocusPolicy policy)
{
    bool retVal = true;

    QWidget* pProxyWidget = pWidget;

    // Get the top focus proxy:
    while (pProxyWidget->focusProxy())
    {
        pProxyWidget = pProxyWidget->focusProxy();
    }

    // If the widget policy is not valid or focus proxy policy is not valid
    if ((pWidget->focusPolicy() & policy) != policy)
    {
        retVal = false;
    }

    if (pWidget != pProxyWidget && (pProxyWidget->focusPolicy() & policy) != policy)
    {
        retVal = false;
    }

    return retVal;
}

void vspQApplicationWrapper::SetMDIActiveWindow(HWND activeWindow)
{
    if (activeWindow != nullptr)
    {
        m_pActiveWidget = QWidget::find((WId)activeWindow);
        HWND childWindow = ::GetWindow(activeWindow, GW_CHILD);

        while (nullptr == m_pActiveWidget && childWindow != nullptr)
        {
            m_pActiveWidget = QWidget::find((WId)childWindow);
            childWindow = ::GetWindow(childWindow, GW_HWNDNEXT);
        }
    }
    else
    {
        m_pActiveWidget = nullptr;
    }
}