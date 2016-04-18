//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdShadersSourceCodeBuildLogDialog.cpp
///
//==================================================================================

//------------------------------ gdShadersSourceCodeBuildLogDialog.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// For compilers that support precompilation, includes "wx/wx.h".


// wxWindows
#include "wx/statline.h"
#include "wx/listctrl.h"
#include "wx/textctrl.h"
#include "wx/button.h"
#include "wx/colour.h"
#include "wx/stattext.h"


// Infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTAPIClasses/Include/apOpenGLStateVariablesManager.h>
#include <AMDTAPIClasses/Include/apOpenGLAPIType.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acWXHTMLWindow.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAppWindowsManger.h>
#include <AMDTGpuDebuggingComponents/Include/gdCodeXLGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/dialogs/gdShadersSourceCodeBuildLogDialog.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStateVariablesCompareFunctor.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdShadersSourceCodeViewer.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdSaveVarStateCommand.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>

// wxWidgets events table:
BEGIN_EVENT_TABLE(gdShadersSourceCodeBuildLogDialog, wxFrame)
    EVT_SHOW(gdShadersSourceCodeBuildLogDialog::onShow)
    EVT_CLOSE(gdShadersSourceCodeBuildLogDialog::onClose)
    EVT_BUTTON(ID_SHADERS_SOURCE_CODE_BUILD_LOG_CLOSE_BUTTON,   gdShadersSourceCodeBuildLogDialog::onCloseButton)
    EVT_AC_HTML_LINK_CLICKED_EVENT(gdShadersSourceCodeBuildLogDialog::OnHTMLLinkClicked)
END_EVENT_TABLE()


// ----------------------------------------------------------------------------------
// Class Name:           gdShadersSourceCodeBuildLogDialog::gdShadersSourceCodeBuildLogDialog(wxWindow *parent)
// General Description:
// Author:               Avi Shapira
// Creation Date:        19/9/2005
// ----------------------------------------------------------------------------------
gdShadersSourceCodeBuildLogDialog::gdShadersSourceCodeBuildLogDialog()
    : wxFrame((wxFrame*)NULL, wxID_ANY, GD_STR_ShadersSourceCodeBuildLogDialogTitle, wxDefaultPosition, wxDefaultSize,
              wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE),
      _pSizer(NULL), _pCloseButton(NULL)
{
    // Get the product title:
    gtString title = afGlobalVariablesManager::instance().productTitleAsString();

    // Add the specific dialog caption
    title.append(GD_STR_ShadersSourceCodeBuildLogDialogTitle);

    // Set the title:
    this->SetTitle(title.asCharArray());

    // Add the Icon to the dialog
    afLoadCodeXLTitleBarIcon(*this);

    // Set the dialog layout
    setDialogLayout();
    doLayout();
}

// ---------------------------------------------------------------------------
// Name:        ~gdShadersSourceCodeBuildLogDialog
// Description:
// Return Val:
// Author:      Avi Shapira
// Date:        19/9/2005
// ---------------------------------------------------------------------------
gdShadersSourceCodeBuildLogDialog::~gdShadersSourceCodeBuildLogDialog()
{
}

// ---------------------------------------------------------------------------
// Name:        SetDialogLayout
// Description:
// Author:      Avi Shapira
// Date:        25/9/2005
// ---------------------------------------------------------------------------
void gdShadersSourceCodeBuildLogDialog::setDialogLayout()
{
    _pHtmlWindow = new acWXHTMLWindow(this, ID_SHADERS_SOURCE_CODE_BUILD_LOG_HTML_CTRL, wxDefaultPosition, wxSize(600, 350), wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER, AF_STR_Empty);
    GT_ASSERT_ALLOCATION(_pHtmlWindow);
}


// ---------------------------------------------------------------------------
// Name:        gdShadersSourceCodeBuildLogDialog::DoLayout
// Description: Building the dialog
// Author:      Avi Shapira
// Date:        19/9/2005
// ---------------------------------------------------------------------------
void gdShadersSourceCodeBuildLogDialog::doLayout()
{
    _pCloseButton = new wxButton(this, ID_SHADERS_SOURCE_CODE_BUILD_LOG_CLOSE_BUTTON, AF_STR_Close_Button);

    _pSizer = new wxBoxSizer(wxVERTICAL);

    // Add the Tool bar:
    _pSizer->Add(_pHtmlWindow, 0, wxALL, 5);
    _pSizer->Add(_pCloseButton, 0, wxALL | wxCENTRE, 5);
    _pCloseButton->SetDefault();
    _pCloseButton->SetFocus();

    // Add the dialog and the Ok/Cancel buttons to the final sizer
    //  _pSizer->Add(_pStateVarsSizer, 0, wxALL, 5);

    GT_ASSERT(_pSizer);

    // activate
    SetSizer(_pSizer);
    SetAutoLayout(true);
    _pSizer->SetSizeHints(this);
    _pSizer->Fit(this);
    Centre(wxBOTH | wxCENTRE_ON_SCREEN);
}

// ---------------------------------------------------------------------------
// Name:        gdShadersSourceCodeBuildLogDialog::OnClose
// Description: Hide the Dialog.
// Author:      Avi Shapira
// Date:        20/11/2005
// ---------------------------------------------------------------------------
void gdShadersSourceCodeBuildLogDialog::onClose(wxCloseEvent& event)
{
    // Hide the dialog:
    Show(false);
}

// ---------------------------------------------------------------------------
// Name:        gdShadersSourceCodeBuildLogDialog::onCloseButton
// Description: Hide the Dialog.
// Author:      Avi Shapira
// Date:        20/11/2005
// ---------------------------------------------------------------------------
void gdShadersSourceCodeBuildLogDialog::onCloseButton(wxCommandEvent& event)
{
    // Hide the dialog:
    Show(false);
}

// ---------------------------------------------------------------------------
// Name:        gdShadersSourceCodeBuildLogDialog::onShow
// Description: Is called when the viewer is shown / hidden.
// Arguments: eve - Contains the event information.
// Author:      Yaki Tebeka
// Date:        11/10/2005
// ---------------------------------------------------------------------------
void gdShadersSourceCodeBuildLogDialog::onShow(wxShowEvent& eve)
{
    // Set the close button to be the default:
    _pCloseButton->SetDefault();
    _pCloseButton->SetFocus();
}

// ---------------------------------------------------------------------------
// Name:        gdShadersSourceCodeBuildLogDialog::setHtmlStringIntoDialog
// Description: Set html string into the output dialog.
// Arguments:   htmlString - the string to be sets.
// Author:      Avi Shapira
// Date:        20/11/2005
// ---------------------------------------------------------------------------
void gdShadersSourceCodeBuildLogDialog::setHtmlStringIntoDialog(gtString& htmlString)
{
    // Get the shaders source code viewer:
    gdAppWindowsManger& theAppWindowsMgr = gdAppWindowsManger::instance();
    gdShadersSourceCodeViewer* pShadersSourceCodeViewer = (gdShadersSourceCodeViewer*)(theAppWindowsMgr.getAppWindow(ID_SHADERS_SOURCE_CODE_VIEWER));
    GT_IF_WITH_ASSERT(pShadersSourceCodeViewer != NULL)
    {
        pShadersSourceCodeViewer->deleteAllMarkers();
    }

    // Add the strings into the dialog:
    wxString wxHtmlString = htmlString.asCharArray();

    // Replace the \n with <br> to show in HTMLCtrl
    wxHtmlString.Replace(AF_STR_NewLine, "<br>", true);

    // Build an HTML string:
    gtString htmlPage;
    afHTMLContent::buildHTMLHeader(htmlPage);
    htmlPage.append(wxHtmlString);
    afHTMLContent::endHTML(htmlPage);

    _pHtmlWindow->SetPage(htmlPage.asCharArray());
}

// ---------------------------------------------------------------------------
// Name:        gdShadersSourceCodeBuildLogDialog::OnHTMLLinkClicked
// Description: Is called when the user clicks on a html link.
// Arguments:   imageInfo - A string that contains the clicked info.
// Author:      Avi Shapira
// Date:        21/11/2005
// ---------------------------------------------------------------------------
void gdShadersSourceCodeBuildLogDialog::OnHTMLLinkClicked(acWXHTMLLinkClickedEvent& event)
{
    // Get the pressed link url:
    const wxHtmlLinkInfo& linkInfo = event.pressedLinkInfo();

    // TO_DO: VS GLSL: make sure that the link has the right format:
    gtString linkHref(linkInfo.GetHref().GetData());
    gdMonitoredObjectTreeItemData objectID;
    int lineNumber = -1;
    bool rc = gdHTMLProperties::htmlLinkToObjectDetails(linkHref, objectID, lineNumber);
    GT_IF_WITH_ASSERT(rc)
    {

        // Get the shaders source code viewer:
        gdAppWindowsManger& theAppWindowsMgr = gdAppWindowsManger::instance();
        gdShadersSourceCodeViewer* pShadersSourceCodeViewer = (gdShadersSourceCodeViewer*)(theAppWindowsMgr.getAppWindow(ID_SHADERS_SOURCE_CODE_VIEWER));
        GT_IF_WITH_ASSERT(pShadersSourceCodeViewer != NULL)
        {
            // Go to (highlight) the error line location:
            pShadersSourceCodeViewer->gotoErrorLocation(objectID, lineNumber);
        }
    }
}
