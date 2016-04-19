//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspQTWindowPaneImpl.cpp
///
//==================================================================================

//------------------------------ vspQTWindowPaneImpl.cpp ------------------------------

#include "StdAfx.h"

// Qt:
#include <QtWidgets>

// Windows:
#include <Windows.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationFramework/Include/views/afBaseView.h>

// Local:
#include <src/vspQTWindowPaneImpl.h>

// Static members initializations:
gtMap<HWND, vspQTWindowPaneImpl*> vspQTWindowPaneImpl::m_hwndToPaneImpl;
vspTooltipData vspQTWindowPaneImpl::m_tooltipData;

bool vspQTWindowPaneImpl::s_isApplicationActive = false;

#define MAIN_TIMER_ID 1
#define MAIN_TIMER_INTERVAL 100

#define TOOL_TIP_TIMER_ID 1000
#define TOOL_TIP_TIMER_INTERVAL 300

// ---------------------------------------------------------------------------
// Name:        vspQTWindowPaneImpl::vspQTWindowPaneImpl
// Description: Constructor
// Arguments:   HWND hwndParent - the Visual Studio tool window handle
//              int x, y, cx, cy - the input coordinated for the window
// Author:      Sigal Algranaty
// Date:        1/12/2011
// ---------------------------------------------------------------------------
vspQTWindowPaneImpl::vspQTWindowPaneImpl(HWND hwndParent, int x, int y, int cx, int cy, bool usesTimer):
    vspWindowPaneImpl(hwndParent, x, y, cx, cy, usesTimer), m_pBackgroundWidget(nullptr), m_pCreatedQTWidget(nullptr)
{
    // Create the QWidget:
    m_pBackgroundWidget = new QWidget;

    // Make the window frameless so that the client area will be the same as the window area (no NC area)
    m_pBackgroundWidget->setWindowFlags(m_pBackgroundWidget->windowFlags() | Qt::FramelessWindowHint);

    // Set the HWND handle as the parent for this widget:
    ::SetParent((HWND)m_pBackgroundWidget->winId(), hwndParent);

    // Show the background window:
    m_pBackgroundWidget->show();

    // Set the size and position according to the parameters got from VS when the tool window was created:
    QSize size = QSize(cx - x, cy - y);
    m_pBackgroundWidget->move(x, y);
    m_pBackgroundWidget->resize(size);

    // Take control of this window's events:
    HWND hWindow = (HWND)m_pBackgroundWidget->winId();
    GT_IF_WITH_ASSERT(hWindow != nullptr)
    {
        // Store the original proc function:
        _originalMessageHandler = (WNDPROC)::SetWindowLongPtr(hWindow, GWLP_WNDPROC, (LONG_PTR)&vspQTWindowPaneImpl::windowPaneProc);
        m_hwndToPaneImpl[hWindow] = this;

        if (usesTimer)
        {
            // Set a timer to call us to refresh the window:
            ::SetTimer(hWindow, MAIN_TIMER_ID, MAIN_TIMER_INTERVAL, nullptr);
        }
    }

    // Show the background widget:
    m_pBackgroundWidget->show();
    m_pBackgroundWidget->raise();
}

// ---------------------------------------------------------------------------
// Name:        vspQTWindowPaneImpl::~vspQTWindowPaneImpl
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        1/12/2011
// ---------------------------------------------------------------------------
vspQTWindowPaneImpl::~vspQTWindowPaneImpl()
{
    if (m_pBackgroundWidget != nullptr)
    {
        // Unregister us from the window messages map:
        HWND hWindow = (HWND)m_pBackgroundWidget->winId();

        // restore original proc and delete the window so we'll stop getting its events
        ::SetWindowLongPtr(hWindow, GWLP_WNDPROC, (LONG_PTR)_originalMessageHandler);
        delete m_pBackgroundWidget;
        m_pBackgroundWidget = nullptr;

        GT_IF_WITH_ASSERT(hWindow != nullptr)
        {
            m_hwndToPaneImpl[hWindow] = nullptr;

            // Kill the timer:
            ::KillTimer(hWindow, MAIN_TIMER_ID);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspQTWindowPaneImpl::OnResize
// Description: Resizes the window
// Arguments:   x, y - coordinates
//              w, h - size
// Author:      Sigal Algranaty
// Date:        1/12/2011
// ---------------------------------------------------------------------------
void vspQTWindowPaneImpl::OnResize(int x, int y, int w, int h)
{
    GT_UNREFERENCED_PARAMETER(x);
    GT_UNREFERENCED_PARAMETER(y);

    // Resize both the created widget and the background widget:
    if (m_pBackgroundWidget != nullptr)
    {
        m_pBackgroundWidget->resize(w, h);
        m_pBackgroundWidget->move(0, 0);
    }

    if (m_pCreatedQTWidget != nullptr)
    {
        m_pCreatedQTWidget->resize(w, h);
        m_pCreatedQTWidget->move(0, 0);
    }
}

// ---------------------------------------------------------------------------
// Name:        vspQTWindowPaneImpl::OnClick
// Description:
// Arguments: bool& handled
// Return Val: void
// Author:      Sigal Algranaty
// Date:        1/12/2011
// ---------------------------------------------------------------------------
void vspQTWindowPaneImpl::OnClick(bool& handled)
{
    GT_UNREFERENCED_PARAMETER(handled);

    // This event works without this implementation. If we would want to add some
    // VS-Related actions to onclick, this is where it should be done.
}

// ---------------------------------------------------------------------------
// Name:        vspQTWindowPaneImpl::GetHWND
// Description: Return the HWND handle for this WX window implementation
// Return Val:  HWND
// Author:      Sigal Algranaty
// Date:        1/12/2011
// ---------------------------------------------------------------------------
HWND vspQTWindowPaneImpl::GetHWND()
{
    HWND retVal = nullptr;

    if (m_pCreatedQTWidget != nullptr)
    {
        retVal = (HWND)(m_pCreatedQTWidget->winId());
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        setWXCreateWindow
// Description: Store the create widget and the base view
// Arguments:   QWidget* pCreateQWidget
// Author:      Sigal Algranaty
// Date:        1/12/2011
// ---------------------------------------------------------------------------
void vspQTWindowPaneImpl::setQTCreateWindow(QWidget* pCreateQWidget)
{
    m_pCreatedQTWidget = pCreateQWidget;
    GT_IF_WITH_ASSERT(m_pCreatedQTWidget != nullptr)
    {
        // Down cast the widget to a baseview:
        _pPaneBaseView = dynamic_cast<afBaseView*>(pCreateQWidget);
        GT_ASSERT(_pPaneBaseView != nullptr);


        // Create a layout for the background widget, that will contain the created widget:
        QBoxLayout* pLayout = new QBoxLayout(QBoxLayout::TopToBottom, m_pBackgroundWidget);


        // Add the create widget to the layout:
        pLayout->addWidget(m_pCreatedQTWidget, 1);

        // Set empty margins;
        QMargins margins(0, 0, 0, 0);
        pLayout->setContentsMargins(margins);

        // Set the layout:
        m_pBackgroundWidget->setLayout(pLayout);

        // Show & raise the content widget:
        m_pCreatedQTWidget->show();
        m_pCreatedQTWidget->raise();
    }
};

// ---------------------------------------------------------------------------
// Name:        vspWXWindowPaneImpl::windowPaneProc
// Description: Called on
// Author:      Gilad Yarnitzky
// Date:        3/2/2011
// ---------------------------------------------------------------------------
LRESULT vspQTWindowPaneImpl::windowPaneProc(HWND hWND, UINT message, WPARAM wParam, LPARAM lParam)
{
    // This member is static since this is a static function and the variable is used locally.
    // So it is not a static member of the class.
    static QPoint lastTooltipPosition;

    LRESULT retVal = 0;

    // Get the window.
    gtMap<HWND, vspQTWindowPaneImpl*>::const_iterator findIter = m_hwndToPaneImpl.find(hWND);
    gtMap<HWND, vspQTWindowPaneImpl*>::const_iterator endIter = m_hwndToPaneImpl.end();

    if (findIter != endIter)
    {
        // Verify that this window was not yet deleted.
        vspQTWindowPaneImpl* pWindowPaneImpl = (*findIter).second;
        GT_IF_WITH_ASSERT(pWindowPaneImpl != nullptr)
        {
            // Verify that the widget is active.
            QWidget* pChild = pWindowPaneImpl->m_pCreatedQTWidget;
            QWidget* pCurrentWidget = pWindowPaneImpl->m_pCreatedQTWidget;

            // if the created window if nullptr take the background window
            // this is needed because of the new windows layers creation that was changed a few months ago
            // some times m_pCreatedQTWidget is not used and only m_pBackgroundWidget is used
            if (pWindowPaneImpl->m_pCreatedQTWidget == nullptr)
            {
                pChild = pWindowPaneImpl->m_pBackgroundWidget;
                pCurrentWidget = pWindowPaneImpl->m_pBackgroundWidget;
            }

            if (pCurrentWidget != nullptr && pCurrentWidget->isActiveWindow())
            {
                // If this is an idle message, send idle events to the Qt application, since we do not have a main loop.
                if ((message == 0x036A /*WM_KICKIDLE*/) || (message == WM_ENTERIDLE) || (message == WM_TIMER))
                {
                    // Get the client position and the global position of the mouse.
                    POINT screenPos;
                    POINT clientPos;
                    HWND currentWinId = reinterpret_cast<HWND>(pCurrentWidget->winId());
                    GetCursorPos(&screenPos);
                    clientPos = screenPos;
                    ScreenToClient(currentWinId, &clientPos);

                    // Find the lowest level child and move the clientPos to that child.
                    // This done by looking at the first wrapper Qt widget that was created and then looking
                    // At each child that contains the mouse point.
                    // Continue until we reached the most inner child window. for each child we do find move the client point
                    // to the inner coords of the child (relative to the upper left corner of the widget)
                    // The final window is stored in pCurrentWidget, and the initial child to start the process is defined as the outer wrapper.
                    QPoint qtClientPos(clientPos.x, clientPos.y);
                    QPoint qtScreenPos(screenPos.x, screenPos.y);

                    while (pChild)
                    {
                        pChild = pCurrentWidget->childAt(qtClientPos);

                        if (pChild)
                        {
                            pCurrentWidget = pChild;

                            // Move client pos to the current qwidget.
                            qtClientPos = pCurrentWidget->mapFromGlobal(qtScreenPos);
                        }
                    }

                    // If this is the timer we created for the tooltip handle it:
                    if (hWND == m_tooltipData.m_tooltipWindow && message == WM_TIMER && TOOL_TIP_TIMER_ID == wParam)
                    {
                        // first kill the new timer:
                        ::KillTimer(m_tooltipData.m_tooltipWindow, TOOL_TIP_TIMER_ID);
                        // restore the original timer
                        ::SetTimer(m_tooltipData.m_tooltipWindow, MAIN_TIMER_ID, MAIN_TIMER_INTERVAL, nullptr);

                        // if this the timer we posted then send the tooltip event only if the mouse is in the same position
                        POINT screenPosCursorPos;
                        GetCursorPos(&screenPosCursorPos);
                        QPoint qtScreenPosCursorPos(screenPosCursorPos.x, screenPosCursorPos.y);

                        if (qtScreenPosCursorPos == m_tooltipData.m_screenPoint)
                        {
                            // Create the tooltip event and send it to Qt
                            QHelpEvent helpEvent(QEvent::ToolTip, m_tooltipData.m_clientPoint, m_tooltipData.m_screenPoint);
                            QApplication::sendEvent(pCurrentWidget, &helpEvent);
                            lastTooltipPosition = m_tooltipData.m_clientPoint;
                        }
                        else
                        {
                            // No tooltip at all so allow placing a new tooltip anywhere on the screen
                            lastTooltipPosition = QPoint(-1, -1);
                        }
                    }

                    // only if the current mouse is not in the same place as the current tooltip and the window is visible and the application is active
                    // only then place a tooltip event
                    HWND windowRealHwnd = reinterpret_cast<HWND>(pCurrentWidget->winId());

                    if (lastTooltipPosition != qtClientPos && ::IsWindowVisible(windowRealHwnd) && s_isApplicationActive)
                    {
                        // store the tooltip event data
                        m_tooltipData.m_tooltipWindow = hWND;
                        m_tooltipData.m_clientPoint = qtClientPos;
                        m_tooltipData.m_screenPoint = qtScreenPos;
                        // Kill the old timer first (for some reason two timers for the same window do not work correctly here)
                        ::KillTimer(hWND, MAIN_TIMER_ID);
                        // activate the new timer timer
                        ::SetTimer(hWND, TOOL_TIP_TIMER_ID, TOOL_TIP_TIMER_INTERVAL, nullptr);
                    }

                }
            }

            // Call the real message handler.
            if (pWindowPaneImpl->_originalMessageHandler != nullptr)
            {
                retVal = pWindowPaneImpl->_originalMessageHandler(hWND, message, wParam, lParam);
            }
        }
    }

    return retVal;
}
