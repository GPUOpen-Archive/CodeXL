//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspQTWindowPaneImpl.h
///
//==================================================================================

//------------------------------ vspQTWindowPaneImpl.h ------------------------------
#ifndef __VSPQTWINDOWPANEIMPL_H
#define __VSPQTWINDOWPANEIMPL_H

// Qt:
#include <QtWidgets>
// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtMap.h>

// Local:
#include <src/vspWindowPaneImpl.h>

// ----------------------------------------------------------------------------------
// Class Name:          vspQTWindowPaneImpl : public vspWindowPaneImpl
// General Description: Implements vspQTWindowPaneImpl by creating and managing contact with
//                      the (QWidget) facet of a tool window's contents.
// Author:              Sigal Algranaty
// Creation Date:       1/12/2011
// ----------------------------------------------------------------------------------
class vspTooltipData
{
public:
    /// HWND that receives the timer
    HWND m_tooltipWindow;

    /// client point:
    QPoint m_clientPoint;

    /// screen point:
    QPoint m_screenPoint;
};

class vspQTWindowPaneImpl : public vspWindowPaneImpl
{
public:
    vspQTWindowPaneImpl(HWND hwndParent, int x, int y, int cx, int cy, bool usesTimer = false);
    ~vspQTWindowPaneImpl();

    // Events:
    virtual void OnResize(int x, int y, int w, int h);
    virtual void OnClick(bool& handled);

    // Accessors:
    QWidget* widget() { return m_pBackgroundWidget; };

    void setQTCreateWindow(QWidget* pPaneQTWindow);
    QWidget* createdQTWidget() {return m_pCreatedQTWidget;};

    // Accessors:
    virtual HWND GetHWND();

protected:

    static LRESULT windowPaneProc(HWND hWND, UINT message, WPARAM wParam, LPARAM lParam);

    friend class vspQApplicationWrapper;

protected:


    // This widget serves as the "background" widget. It's parent is VS window handle, and it's child is the created widget:
    QWidget* m_pBackgroundWidget;

    // The tool window Qt contents:
    QWidget* m_pCreatedQTWidget;

    // Message Handling:
    static gtMap<HWND, vspQTWindowPaneImpl*> m_hwndToPaneImpl;

    // holds the status of the application if it is active or not:
    static bool s_isApplicationActive;

    /// Holds the tooltip data
    static vspTooltipData m_tooltipData;
};

#endif //__VSPQTWINDOWPANEIMPL_H

