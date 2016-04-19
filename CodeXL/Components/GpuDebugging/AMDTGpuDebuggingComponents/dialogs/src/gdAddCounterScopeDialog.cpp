//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdAddCounterScopeDialog.cpp
///
//==================================================================================

//------------------------------ gdAddCounterScopeDialog.cpp ------------------------------





// wxWindows
#include "wx/sizer.h"

// Infra
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

//Local
#include <AMDTGpuDebuggingComponents/Include/dialogs/gdAddCounterScopeDialog.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>

// The context Id spin control width - is different in Linux / Mac since spin control width is different
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define GD_CONTEXT_ID_SPIN_WIDTH 55
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define GD_CONTEXT_ID_SPIN_WIDTH 65
#else
    #error Unsupported platform
#endif

// Defines minimum and maximum values for the spin control
#define COUNTER_SCOPE_ID_CONTEXT_MIN 2
#define COUNTER_SCOPE_ID_QUQUE_MIN 1
#define COUNTER_SCOPE_ID_MAX 100

// wxWidgets Events table:
BEGIN_EVENT_TABLE(gdAddCounterScopeDialog, acDialog)
    EVT_BUTTON(ID_ADD_CONTEXT_OK, gdAddCounterScopeDialog::onOK)
    EVT_BUTTON(ID_ADD_CONTEXT_CANCEL, gdAddCounterScopeDialog::onCancel)

    // On Linux - the event EVT_TEXT doesn't work, so we use EVT_SPINCTRL.
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    EVT_TEXT(ID_CONTEXT_CONTEXT_ID_CHANGED, gdAddCounterScopeDialog::onContextTextChanged)
    EVT_TEXT(ID_QUEUE_CONTEXT_ID_CHANGED, gdAddCounterScopeDialog::onQueueTextChanged)
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    EVT_SPINCTRL(ID_CONTEXT_CONTEXT_ID_CHANGED, gdAddCounterScopeDialog::onContextSpinCtrlTextChanged)
    EVT_SPINCTRL(ID_CONTEXT_CONTEXT_ID_CHANGED, gdAddCounterScopeDialog::onQueueSpinCtrlTextChanged)
#else
    #error Unsupported platform
#endif

END_EVENT_TABLE()


// ---------------------------------------------------------------------------
// Name:        gdAddCounterScopeDialog::gdAddCounterScopeDialog
// Description: Definition of the Add Render Context Dialog.
// Arguments:   wxWindow *parent - the parent window
//              existingContexts - allready existing contexts - is used for
//              setting the displayed context id - the first non-existing one
// Author:      Sigal Algranaty
// Date:        3/3/2008
// -------------------------------------------------------------------------
gdAddCounterScopeDialog::gdAddCounterScopeDialog(wxWindow* parent, apCounterScope* pCounterScope, const gtVector<apCounterScope>& existingScopes)
    : acDialog(parent, -1, _(GD_STR_PerformanceCountersAddRenderContextDialogTitle), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
      _pSizer(NULL), _pContextTextAndSpinSizer(NULL), _pQueueTextAndSpinSizer(NULL),
      _pButtonsSizer(NULL), _pOKButton(NULL), _pCancelButton(NULL), _pContextIDSpinCtrl(NULL), _pQueueIDSpinCtrl(NULL),
      _counterMinScope(COUNTER_SCOPE_ID_CONTEXT_MIN), _pCounterScope(pCounterScope)
{
    // Copy the existing context IDs to the class member vector
    for (size_t pos = 0; pos < existingScopes.size(); pos++)
    {
        _existingScopes.push_back(existingScopes[pos]);
    }

    // Sanity check:
    GT_IF_WITH_ASSERT(_pCounterScope != NULL)
    {
        // Set the title according to the type of scope being added:
        if (_pCounterScope->_counterScopeType == apCounterScope::AP_CONTEXT_COUNTER)
        {
            // Context counter scope:
            this->SetTitle(GD_STR_PerformanceCountersAddRenderContextDialogTitle);
            _pCounterScope->_contextID._contextType = AP_OPENGL_CONTEXT;
            _counterMinScope = COUNTER_SCOPE_ID_CONTEXT_MIN;
        }
        else if (_pCounterScope->_counterScopeType == apCounterScope::AP_QUEUE_COUNTER)
        {
            // Context counter scope:
            this->SetTitle(GD_STR_PerformanceCountersAddOpenCLQueueDialogTitle);
            _pCounterScope->_contextID._contextType = AP_OPENCL_CONTEXT;
            _counterMinScope = COUNTER_SCOPE_ID_QUQUE_MIN;
        }
        else
        {
            GT_ASSERT_EX(false, L"Should not get here");
        }
    }

    // Add the Icon to the dialog:
    afLoadCodeXLTitleBarIcon(*this);

    // Set the dialog layout:
    setDialogLayout();
}

// ---------------------------------------------------------------------------
// Name:        gdAddCounterScopeDialog::ShowModal
// Description: Overrides the wxDialog ShowModal
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        3/3/2008
// ---------------------------------------------------------------------------
int gdAddCounterScopeDialog::ShowModal()
{
    int retVal = wxDialog::ShowModal();
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdAddCounterScopeDialog::OnOK
// Description: OK button event handler
// Arguments: wxCommandEvent &eve
// Return Val: void
// Author:      Sigal Algranaty
// Date:        4/3/2008
// ---------------------------------------------------------------------------
void gdAddCounterScopeDialog::onOK(wxCommandEvent& eve)
{
    GT_IF_WITH_ASSERT(_pCounterScope != NULL)
    {
        // Set the context and queue ids:
        _pCounterScope->_contextID._contextId = _pContextIDSpinCtrl->GetValue();

        // For queue counters, set the queue id:
        if (_pCounterScope->_counterScopeType == apCounterScope::AP_QUEUE_COUNTER)
        {
            GT_IF_WITH_ASSERT(_pQueueIDSpinCtrl != NULL)
            {
                _pCounterScope->_queueId = _pQueueIDSpinCtrl->GetValue();
            }
        }
    }
    EndModal(wxID_OK);
}

// ---------------------------------------------------------------------------
// Name:        gdAddCounterScopeDialog::OnCancel
// Description: Cancel button event handler
// Arguments: wxCommandEvent &eve
// Return Val: void
// Author:      Sigal Algranaty
// Date:        4/3/2008
// ---------------------------------------------------------------------------
void gdAddCounterScopeDialog::onCancel(wxCommandEvent& eve)
{
    EndModal(wxID_CANCEL);
}

// ---------------------------------------------------------------------------
// Name:        gdAddCounterScopeDialog::onContextTextChanged
// Description: TextChanged spin button event handler
//              it is important to use this event handler and not the specific
//              SpinButton event handler, since this event responses to each
//              keyboard click
// Arguments: wxCommandEvent& textChangedEvent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        4/3/2008
// ---------------------------------------------------------------------------
void gdAddCounterScopeDialog::onContextTextChanged(wxCommandEvent& textChangedEvent)
{
    // Check the string and only if it is numeric, set the context id,
    // otherwise - the context id would be the last integer set
    gtString testValidate(textChangedEvent.GetString().c_str());
    bool valueValid = false;

    if (testValidate.isIntegerNumber())
    {
        // Do not allow the user set values from out of range
        int editedValue = 0;

        if (testValidate.toIntNumber(editedValue))
        {
            valueValid  = ((editedValue >= _pContextIDSpinCtrl->GetMin()) && (editedValue <= _pContextIDSpinCtrl->GetMax()));

            if (valueValid)
            {
                GT_IF_WITH_ASSERT(_pCounterScope != NULL)
                {
                    if (_pCounterScope->_counterScopeType == apCounterScope::AP_CONTEXT_COUNTER)
                    {
                        _pCounterScope->_contextID._contextId = editedValue;
                    }
                    else if (_pCounterScope->_counterScopeType == apCounterScope::AP_QUEUE_COUNTER)
                    {
                        _pCounterScope->_contextID._contextId = editedValue;
                    }

                }
            }
        }
    }

    if (_pOKButton != NULL)
    {
        _pOKButton->Enable(valueValid);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdAddCounterScopeDialog::onQueueTextChanged
// Description: TextChanged spin button event handler
//              it is important to use this event handler and not the specific
//              SpinButton event handler, since this event responses to each
//              keyboard click
// Arguments:   wxCommandEvent& textChangedEvent
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        8/3/2010
// ---------------------------------------------------------------------------
void gdAddCounterScopeDialog::onQueueTextChanged(wxCommandEvent& textChangedEvent)
{
    // Check the string and only if it is numeric, set the context id,
    // otherwise - the context id would be the last integer set
    gtString testValidate(textChangedEvent.GetString().c_str());
    bool valueValid = false;

    if (testValidate.isIntegerNumber())
    {
        // Do not allow the user set values from out of range
        int editedValue = 0;

        if (testValidate.toIntNumber(editedValue))
        {
            valueValid  = ((editedValue >= _pQueueIDSpinCtrl->GetMin()) && (editedValue <= _pQueueIDSpinCtrl->GetMax()));

            if (valueValid)
            {
                GT_IF_WITH_ASSERT(_pCounterScope != NULL)
                {
                    _pCounterScope->_queueId = editedValue - 1;
                }
            }
        }
    }

    if (_pOKButton != NULL)
    {
        _pOKButton->Enable(valueValid);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdAddCounterScopeDialog::onContextSpinCtrlTextChanged
// Description: Event handler for Linux  text changed. Doesn't react to each key
//              down, but we get there after OK button click.
// Arguments: wxSpinEvent& event
// Return Val: void
// Author:      Sigal Algranaty
// Date:        11/3/2008
// ---------------------------------------------------------------------------
void gdAddCounterScopeDialog::onContextSpinCtrlTextChanged(wxSpinEvent& event)
{
    // Set context id to the spin control value
    // This value would be set to Min, or Max if the user used edit mode,
    // and entered an illegal string:
    GT_IF_WITH_ASSERT(_pCounterScope != NULL)
    {
        if (_pCounterScope->_counterScopeType == apCounterScope::AP_CONTEXT_COUNTER)
        {
            _pCounterScope->_contextID._contextId = _pContextIDSpinCtrl->GetValue();
        }
        else if (_pCounterScope->_counterScopeType == apCounterScope::AP_QUEUE_COUNTER)
        {
            _pCounterScope->_contextID._contextId = _pContextIDSpinCtrl->GetValue();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdAddCounterScopeDialog::onQueueSpinCtrlTextChanged
// Description: Event handler for Linux  text changed. Doesn't react to each key
//              down, but we get there after OK button click.
// Arguments:   wxSpinEvent& event
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        8/3/2010
// ---------------------------------------------------------------------------
void gdAddCounterScopeDialog::onQueueSpinCtrlTextChanged(wxSpinEvent& event)
{
    // Set queue id to the spin control value
    // This value would be set to Min, or Max if the user used edit mode,
    // and entered an illegal string:
    GT_IF_WITH_ASSERT(_pCounterScope != NULL)
    {
        _pCounterScope->_queueId = _pQueueIDSpinCtrl->GetValue() - 1;
    }
}



// ---------------------------------------------------------------------------
// Name:        gdAddCounterScopeDialog::setDialogLayout
// Description: Sets the dialog layout
// Author:      Sigal Algranaty
// Date:        8/3/2010
// ---------------------------------------------------------------------------
void gdAddCounterScopeDialog::setDialogLayout()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pCounterScope != NULL)
    {
        // Create a StaticText for the description of the context add:
        gtString description = GD_STR_PerformanceCountersAddGLContextDialogDescription;

        if (_pCounterScope->_counterScopeType == apCounterScope::AP_QUEUE_COUNTER)
        {
            description = GD_STR_PerformanceCountersAddCLContextDialogDescription;
        }

        wxStaticText* pAddContextDialogDescription = new wxStaticText;
        GT_ASSERT_ALLOCATION(pAddContextDialogDescription);

        // Create the static text:
        pAddContextDialogDescription->Create(this, -1, AF_STR_Empty, wxDefaultPosition, wxDefaultSize, wxTE_RICH | wxTE_MULTILINE);
        pAddContextDialogDescription->SetLabel(description.asCharArray());

        // Find out the displayed values according to the existing scopes:
        int defaultContextIndex = 0;
        int defaultQueueIndex = 0;
        findDefaultScopeValues(defaultContextIndex, defaultQueueIndex);

        // Create the spin control for the context ID:
        _pSizer = new wxBoxSizer(wxVERTICAL);
        GT_ASSERT_ALLOCATION(_pSizer);

        _pContextIDSpinCtrl = new wxSpinCtrl;
        GT_ASSERT_ALLOCATION(_pContextIDSpinCtrl);

        _pContextIDSpinCtrl->Create(this, ID_CONTEXT_CONTEXT_ID_CHANGED, wxEmptyString, wxDefaultPosition, wxSize(GD_CONTEXT_ID_SPIN_WIDTH, -1), wxSP_ARROW_KEYS);
        _pContextIDSpinCtrl->SetRange(_counterMinScope, COUNTER_SCOPE_ID_MAX);
        _pContextIDSpinCtrl->SetValue(defaultContextIndex);

        _pContextTextAndSpinSizer = new wxBoxSizer(wxHORIZONTAL);
        GT_ASSERT_ALLOCATION(_pContextTextAndSpinSizer);

        _pContextTextAndSpinSizer->Add(pAddContextDialogDescription, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 0);
        _pContextTextAndSpinSizer->AddStretchSpacer(1);
        _pContextTextAndSpinSizer->Add(_pContextIDSpinCtrl, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

        // Add the top controls sizer to the main sizer:
        _pSizer->Add(_pContextTextAndSpinSizer, 0, wxALL | wxGROW, 10);

        if (_pCounterScope->_counterScopeType == apCounterScope::AP_QUEUE_COUNTER)
        {
            // StaticText for the description of the dialog
            wxStaticText* pAddQueueDialogDescription = new wxStaticText;
            GT_ASSERT_ALLOCATION(pAddQueueDialogDescription);

            // Create the queue add description:
            pAddQueueDialogDescription->Create(this, -1, AF_STR_Empty, wxDefaultPosition, wxDefaultSize, wxTE_RICH | wxTE_MULTILINE);
            pAddQueueDialogDescription->SetLabel(GD_STR_PerformanceCountersAddOpenCLQueueDialogDescription);

            _pQueueIDSpinCtrl = new wxSpinCtrl;
            GT_ASSERT_ALLOCATION(_pQueueIDSpinCtrl);

            _pQueueIDSpinCtrl->Create(this, ID_QUEUE_CONTEXT_ID_CHANGED, wxEmptyString, wxDefaultPosition, wxSize(GD_CONTEXT_ID_SPIN_WIDTH, -1), wxSP_ARROW_KEYS);
            _pQueueIDSpinCtrl->SetRange(_counterMinScope, COUNTER_SCOPE_ID_MAX);
            _pQueueIDSpinCtrl->SetValue(defaultQueueIndex);

            _pQueueTextAndSpinSizer = new wxBoxSizer(wxHORIZONTAL);
            GT_ASSERT_ALLOCATION(_pQueueTextAndSpinSizer);

            _pQueueTextAndSpinSizer->Add(pAddQueueDialogDescription, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 0);
            _pQueueTextAndSpinSizer->AddStretchSpacer(1);
            _pQueueTextAndSpinSizer->Add(_pQueueIDSpinCtrl, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

            _pSizer->Add(_pQueueTextAndSpinSizer, 0, wxALL | wxGROW, 10);

        }

        // OK/Cancel Buttons
        _pOKButton = new wxButton(this, ID_ADD_CONTEXT_OK, AF_STR_OK_Button);
        GT_ASSERT_ALLOCATION(_pOKButton);

        _pCancelButton = new wxButton(this, ID_ADD_CONTEXT_CANCEL, AF_STR_Cancel_Button);
        GT_ASSERT_ALLOCATION(_pCancelButton);

        _pButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
        GT_ASSERT_ALLOCATION(_pButtonsSizer);

        _pButtonsSizer->Add(_pOKButton, 0, wxRIGHT, 10);
        _pButtonsSizer->Add(_pCancelButton, 0);

        _pSizer->Add(_pButtonsSizer, 0, wxALIGN_CENTER | wxBOTTOM, 10);
        _pOKButton->SetDefault();

        SetAutoLayout(true);
        SetSizer(_pSizer);
        _pSizer->Fit(this);
        _pSizer->SetSizeHints(this);
        Layout();
    }
}


// ---------------------------------------------------------------------------
// Name:        gdAddCounterScopeDialog::findDefaultScopeValues
// Description: Find the default scope attributes
// Arguments:   int& defaultContextIndex
//            int& defaultQueueIndex
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        8/3/2010
// ---------------------------------------------------------------------------
void gdAddCounterScopeDialog::findDefaultScopeValues(int& defaultContextIndex, int& defaultQueueIndex)
{
    GT_IF_WITH_ASSERT(_pCounterScope != NULL)
    {
        // Search for the first free context index:
        if (_pCounterScope->_counterScopeType == apCounterScope::AP_CONTEXT_COUNTER)
        {
            // Search for the default context:
            defaultContextIndex = _counterMinScope;
            defaultQueueIndex = -1;
            gtVector<apCounterScope>::const_iterator iterBegin = _existingScopes.begin();
            gtVector<apCounterScope>::const_iterator iterEnd = _existingScopes.end();

            for (size_t i = _counterMinScope; i < COUNTER_SCOPE_ID_MAX; i++)
            {
                apCounterScope scope(apContextID(AP_OPENGL_CONTEXT, i));
                gtVector<apCounterScope>::const_iterator iter = gtFind(iterBegin, iterEnd, scope);

                if (iter == iterEnd)
                {
                    defaultContextIndex = i;
                    break;
                }
            }
        }

        // Queue scope:
        else if (_pCounterScope->_counterScopeType == apCounterScope::AP_QUEUE_COUNTER)
        {
            // Search for the first free combination of context and queue:
            defaultContextIndex = _counterMinScope;
            defaultQueueIndex = _counterMinScope;
            gtVector<apCounterScope>::const_iterator iterBegin = _existingScopes.begin();
            gtVector<apCounterScope>::const_iterator iterEnd = _existingScopes.end();

            for (size_t contextIndex = _counterMinScope; contextIndex < COUNTER_SCOPE_ID_MAX; contextIndex++)
            {
                bool wasQueueFound = true;

                for (size_t queueIndex = _counterMinScope; queueIndex < COUNTER_SCOPE_ID_MAX; queueIndex++)
                {
                    // Define a counter scope for this queue / context combination:
                    apCounterScope counterScope(apCounterScope::AP_QUEUE_COUNTER);
                    counterScope._contextID._contextId = contextIndex;
                    counterScope._contextID._contextType = AP_OPENCL_CONTEXT;
                    counterScope._queueId = queueIndex;

                    // Check if this scope already exist:
                    gtVector<apCounterScope>::const_iterator iter = gtFind(iterBegin, iterEnd, counterScope);

                    if (iter == iterEnd)
                    {
                        defaultContextIndex = contextIndex;
                        defaultQueueIndex = queueIndex;
                        wasQueueFound = true;
                        break;
                    }
                }

                if (wasQueueFound)
                {
                    break;
                }
            }
        }

        else
        {
            GT_ASSERT(false);
        }
    }
}

