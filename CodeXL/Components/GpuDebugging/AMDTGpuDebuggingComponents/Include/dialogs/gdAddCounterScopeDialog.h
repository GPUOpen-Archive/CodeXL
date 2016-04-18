//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdAddCounterScopeDialog.h
///
//==================================================================================

//------------------------------ gdAddCounterScopeDialog.h ------------------------------

#ifndef __GDADDCOUNTERSCOPEDIALOG_H
#define __GDADDCOUNTERSCOPEDIALOG_H


// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTAPIClasses/Include/apCounterScope.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTApplicationComponents/Include/acDialog.h>

// wxWindows
#include "wx/dialog.h"
#include "wx/button.h"
#include "wx/spinctrl.h"



// Local:
#include <AMDTGpuDebuggingComponents/Include/gdCodeXLAppDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           gdAddCounterScopeDialog
// General Description: Is used for adding new counter scope.
//                      The type of the scope that the user is requested to select, depends
//                      on the input apCounterScope object. This object is also used for setting
//                      the counter scope details according to user selection.
// Author:               Sigal Algranaty
// Creation Date:        10/3/2008
// ----------------------------------------------------------------------------------
class GD_API gdAddCounterScopeDialog : public acDialog
{
public:
    gdAddCounterScopeDialog(wxWindow* parent, apCounterScope* pCounterScope, const gtVector<apCounterScope>& existingScopes);

    // Overrides wxDialog:
    virtual int ShowModal();


private:
    // Dialogs controls and Sizers
    wxSizer* _pSizer;
    wxSizer* _pContextTextAndSpinSizer;
    wxSizer* _pQueueTextAndSpinSizer;
    wxSizer* _pButtonsSizer;
    wxButton* _pOKButton;
    wxButton* _pCancelButton;
    wxSpinCtrl* _pContextIDSpinCtrl;
    wxSpinCtrl* _pQueueIDSpinCtrl;

    // The currently existing counter scope:
    gtVector<apCounterScope> _existingScopes;

    // The counter minimal scope index:
    int _counterMinScope;

private:

    // The counter scope type (context / queue / global):
    apCounterScope* _pCounterScope;

    // Events:
    void onOK(wxCommandEvent& eve);
    void onCancel(wxCommandEvent& eve);
    void onContextSpinCtrlTextChanged(wxSpinEvent& event);
    void onQueueSpinCtrlTextChanged(wxSpinEvent& event);
    void onContextTextChanged(wxCommandEvent& textChangedEvent);
    void onQueueTextChanged(wxCommandEvent& textChangedEvent);

    // Utilities:
    void setDialogLayout();
    void findDefaultScopeValues(int& defaultContextIndex, int& defaultQueueIndex);

    // wxWidgets events table:
    DECLARE_EVENT_TABLE()
};

#endif //__gdAddCounterScopeDialog_H

