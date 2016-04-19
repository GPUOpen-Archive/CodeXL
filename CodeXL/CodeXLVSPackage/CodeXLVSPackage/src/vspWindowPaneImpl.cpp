//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspWindowPaneImpl.cpp
///
//==================================================================================

//------------------------------ vspWXWindowPaneImpl.cpp ------------------------------

#include "StdAfx.h"


// Local:
#include <src/vspWindowPaneImpl.h>

#include <AMDTBaseTools/Include/AMDTDefinitions.h>


// ---------------------------------------------------------------------------
// Name:        vspWindowPaneImpl::vspWindowPaneImpl
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        1/12/2011
// ---------------------------------------------------------------------------
vspWindowPaneImpl::vspWindowPaneImpl(HWND hwndParent, int x, int y, int cx, int cy, bool usesTimer) : _pPaneBaseView(NULL)
{
    GT_UNREFERENCED_PARAMETER(hwndParent);
    GT_UNREFERENCED_PARAMETER(x);
    GT_UNREFERENCED_PARAMETER(y);
    GT_UNREFERENCED_PARAMETER(cx);
    GT_UNREFERENCED_PARAMETER(cy);
    GT_UNREFERENCED_PARAMETER(usesTimer);
}

// ---------------------------------------------------------------------------
// Name:        vspWindowPaneImpl::~vspWindowPaneImpl
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        1/12/2011
// ---------------------------------------------------------------------------
vspWindowPaneImpl::~vspWindowPaneImpl()
{
}

void vspWindowPaneImpl::OnResize(int x, int y, int w, int h)
{
    GT_UNREFERENCED_PARAMETER(x);
    GT_UNREFERENCED_PARAMETER(y);
    GT_UNREFERENCED_PARAMETER(w);
    GT_UNREFERENCED_PARAMETER(h);
}

void vspWindowPaneImpl::OnClick(bool& handled)
{
    GT_UNREFERENCED_PARAMETER(handled);
}
