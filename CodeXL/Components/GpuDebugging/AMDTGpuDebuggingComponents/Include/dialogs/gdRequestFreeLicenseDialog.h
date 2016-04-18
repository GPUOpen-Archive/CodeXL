//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdRequestFreeLicenseDialog.h
///
//==================================================================================

//------------------------------ gdRequestFreeLicenseDialog.h ------------------------------

#ifndef __GDREQUESTFREELICENSEDIALOG_H
#define __GDREQUESTFREELICENSEDIALOG_H

// Forward decelerations:
class wxHyperlinkEvent;
class wxSizer;
class acHTMLLinkClickedEvent;
class acHTMLWindow;
class acTextCtrl;

// Infra:
#include <AMDTApplicationComponents/Include/acDialog.h>

// Local:
#include <CodeXLAppCode/gdCodeXLAppDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           gdRequestFreeLicenseDialog : public wxDialog
// General Description:
//   A dialog that asks the user if he would like to request a free license over the web.
// Author:               Yaki Tebeka
// Creation Date:        19/5/2008
// ----------------------------------------------------------------------------------
class gdRequestFreeLicenseDialog : public acDialog
{
public:
    gdRequestFreeLicenseDialog(wxWindow* pParentWindow);
    bool wasFreeLicenseInstalled() const { return _wasFreeLicenseInstalled; };

private:
    void setDialogLayout();
    void onClose(wxCloseEvent& event);
    void onSendRequestButton(wxCommandEvent& event);
    void onCancelButton(wxCommandEvent& event);
    void onAdvancedOptionsButton(wxCommandEvent& event);
    void onPrivacyPolicyButton(wxCommandEvent& eve);
    void onClickHerePressed(wxHyperlinkEvent& eve);
    void onUsingAProxyPressed(wxHyperlinkEvent& eve);
    void onLicenseByWebFormPressed(wxHyperlinkEvent& eve);
    void getFreeRequestDetails(gtString& freeRequestDetails);
    void getLicenseByWebFormDetails(gtString& licenseByWebFormDetails);
    bool validateFreeLicenseRequestInfo();

private:
    // Dialog input fields:
    acTextCtrl* _pNameInputField;
    acTextCtrl* _pCompanyInputField;
    acTextCtrl* _pEmailInputField;
    acTextCtrl* _pTelNumberInputField;

    // "Send me information" checkbox:
    wxCheckBox* _pSendMeInformationCheckBox;
    wxCheckBox* _pSendMePartnerInformationCheckBox;

    // Dialog buttons:
    wxButton* _pSendRequestButton;
    wxButton* _pCancelButton;
    wxButton* _pAdvancedOptionsButton;
    wxButton* _pPrivacyPolicyButton;

    // Are the advanced options (Request by email, proxy settings) shown?
    bool _areAdvancedOptionsShown;
    wxSizer* _pTextItemsSizer;
    wxSizer* _pAdvancedOptionsSizer;

    // Will get true iff a free license was successfully installed:
    bool _wasFreeLicenseInstalled;

private:
    // wxWidgets events table:
    DECLARE_EVENT_TABLE()
};


#endif //__GDREQUESTFREELICENSEDIALOG_H

