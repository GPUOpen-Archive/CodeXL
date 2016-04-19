//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdShadersSourceCodeBuildLogDialog.h
///
//==================================================================================

//------------------------------ gdShadersSourceCodeBuildLogDialog.h ------------------------------

#ifndef __GDSHADERSSOURCECODEBUILDLOGDIALOG
#define __GDSHADERSSOURCECODEBUILDLOGDIALOG

// wxWindows:
#include <wx/html/htmlwin.h>
#include <wx/html/htmlcell.h>


// ----------------------------------------------------------------------------------
// Class Name:
// General Description:
// Author:               Avi Shapira
// Creation Date:        9/4/2004
// ----------------------------------------------------------------------------------
class GD_API gdShadersSourceCodeBuildLogDialog : public wxFrame
{
public:
    gdShadersSourceCodeBuildLogDialog();
    ~gdShadersSourceCodeBuildLogDialog();
    void setHtmlStringIntoDialog(gtString& htmlString);

    void OnHTMLLinkClicked(acWXHTMLLinkClickedEvent& event);

private:
    DECLARE_EVENT_TABLE()


private:
    wxSizer* _pSizer;
    wxButton* _pCloseButton;

    acWXHTMLWindow* _pHtmlWindow;

private:
    void doLayout();
    void setDialogLayout();
    void setDialogInitialValues();


    void onClose(wxCloseEvent& event);
    void onCloseButton(wxCommandEvent& event);
    void onShow(wxShowEvent& eve);
};


#endif  // __GDSHADERSSOURCECODEBUILDLOGDIALOG
