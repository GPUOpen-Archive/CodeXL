//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspWindowPaneImpl.h
///
//==================================================================================

//------------------------------ vspWindowPaneImpl.h ------------------------------

#ifndef __VSPWINDOWPANEIMPL_H
#define __VSPWINDOWPANEIMPL_H
#include <QtGui>
#include <WinUser.h>

class afBaseView;
// ----------------------------------------------------------------------------------
// Class Name:          vspWindowPaneImpl
// General Description: Base class for Qt and WX windows implementations
// Author:              Sigal Algranaty
// Creation Date:       1/12/2011
// ----------------------------------------------------------------------------------
class vspWindowPaneImpl
{
public:
    vspWindowPaneImpl(HWND hwndParent, int x, int y, int cx, int cy, bool usesTimer = false);
    ~vspWindowPaneImpl();

    // Events:
    virtual void OnResize(int x, int y, int w, int h);
    virtual void OnClick(bool& handled);

    // Accessors:
    virtual HWND GetHWND() { return NULL; }

    afBaseView* baseView() { return _pPaneBaseView; }

protected:

    // The window base extra behavior
    afBaseView* _pPaneBaseView;

    WNDPROC _originalMessageHandler;
};
#endif //__vspWindowPaneImpl_H

