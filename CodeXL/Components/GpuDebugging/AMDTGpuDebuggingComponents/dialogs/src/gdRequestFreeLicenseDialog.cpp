//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdRequestFreeLicenseDialog.cpp
///
//==================================================================================

//------------------------------ gdRequestFreeLicenseDialog.cpp ------------------------------

// wxWidgets precompilation mechanism:

#include "wx/hyperlink.h"

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <GRLicenseManager/lmLicenseManagerFunctions.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFileLauncher.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osMachine.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acHTMLWindow.h>
#include <AMDTApplicationComponents/Include/acMessageDialog.h>
#include <AMDTApplicationComponents/Include/acTextCtrl.h>
#include <AMDTApplicationComponents/Include/acTextCtrlWithInlineDescription.h>

// Local:
#include <CodeXLAppCode/gdCommandIDs.h>
#include <CodeXLAppCode/gdStringConstants.h>
#include <CodeXLAppCode/gdAidFunctions.h>
#include <CodeXLAppCode/gdCodeXLGlobalVariablesManager.h>
#include <CodeXLAppCode/dialogs/gdOptionsDialog.h>
#include <CodeXLAppCode/dialogs/gdRequestFreeLicenseDialog.h>

// wxWidgets events table:
BEGIN_EVENT_TABLE(gdRequestFreeLicenseDialog, acDialog)
    EVT_BUTTON(ID_REQUEST_FREE_LICENSE_ADVANCED_BUTTON,     gdRequestFreeLicenseDialog::onAdvancedOptionsButton)
    EVT_BUTTON(ID_REQUEST_FREE_LICENSE_SEND_BUTTON,         gdRequestFreeLicenseDialog::onSendRequestButton)
    EVT_BUTTON(ID_REQUEST_FREE_LICENSE_CANCEL_BUTTON,       gdRequestFreeLicenseDialog::onCancelButton)
    EVT_BUTTON(ID_REQUEST_FREE_LICENSE_PRIVACY_POLICY,      gdRequestFreeLicenseDialog::onPrivacyPolicyButton)
    EVT_HYPERLINK(ID_REQUEST_FREE_LICENSE_CLICK_HERE,       gdRequestFreeLicenseDialog::onClickHerePressed)
    EVT_HYPERLINK(ID_REQUEST_FREE_LICENSE_USING_PROXY,      gdRequestFreeLicenseDialog::onUsingAProxyPressed)
    EVT_HYPERLINK(ID_REQUEST_FREE_LICENSE_LICENSE_BY_EMAIL, gdRequestFreeLicenseDialog::onLicenseByWebFormPressed)
END_EVENT_TABLE()


// ---------------------------------------------------------------------------
// Name:        gdRequestFreeLicenseDialog::gdRequestFreeLicenseDialog
// Description: Definition of the Evaluation Dialog.
// Author:      Yaki Tebeka
// Date:        19/5/2008
// ---------------------------------------------------------------------------
gdRequestFreeLicenseDialog::gdRequestFreeLicenseDialog(wxWindow* pParentWindow)
    : acDialog(pParentWindow, ID_REQUEST_FREE_LICENSE_DIALOG, GD_STR_RequestFreeDialogTitle,
               wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
      _pNameInputField(NULL), _pCompanyInputField(NULL), _pEmailInputField(NULL), _pTelNumberInputField(NULL), _pSendMeInformationCheckBox(NULL), _pSendMePartnerInformationCheckBox(NULL),
      _pSendRequestButton(NULL), _pCancelButton(NULL), _pAdvancedOptionsButton(NULL), _pPrivacyPolicyButton(NULL), _areAdvancedOptionsShown(false),
      _pTextItemsSizer(NULL), _pAdvancedOptionsSizer(NULL), _wasFreeLicenseInstalled(false)
{
    // Get the dialog icon to be the CodeXL icon:
    gdLoadCodeXLTitleBarIcon(*this);
    // Set the dialogs layout:
    setDialogLayout();
}


// ---------------------------------------------------------------------------
// Name:        gdRequestFreeLicenseDialog::setDialogLayout
// Description: Populates the dialog with its internal items and sets the
//              dialog layout.
// Author:      Yaki Tebeka
// Date:        19/5/2008
// ---------------------------------------------------------------------------
void gdRequestFreeLicenseDialog::setDialogLayout()
{
    // Get the system default background color:
    wxColor defaultBackgroundcolor = acGetSystemDefaultBackgroundColor();
    this->SetBackgroundColour(defaultBackgroundcolor);
    // Create the main dialog sizer:
    wxBoxSizer* pDialogMainSizer = new wxBoxSizer(wxHORIZONTAL);
    GT_ASSERT_ALLOCATION(pDialogMainSizer);
    wxBoxSizer* pDialogVerticalSizer = new wxBoxSizer(wxVERTICAL);
    GT_ASSERT_ALLOCATION(pDialogVerticalSizer);

    // Create a box sizer that will hold the static text items and input fields:
    _pTextItemsSizer = new wxStaticBoxSizer(wxVERTICAL, this, GD_STR_RequestFreeDialogTitle);
    GT_ASSERT_ALLOCATION(_pTextItemsSizer);

    // Create sizers for the input fields and their text items:
    wxBoxSizer* pNameFieldSizer = new wxBoxSizer(wxHORIZONTAL);
    GT_ASSERT_ALLOCATION(pNameFieldSizer);

    wxBoxSizer* pCompanyFieldSizer = new wxBoxSizer(wxHORIZONTAL);
    GT_ASSERT_ALLOCATION(pCompanyFieldSizer);

    wxBoxSizer* pEmailFieldSizer = new wxBoxSizer(wxHORIZONTAL);
    GT_ASSERT_ALLOCATION(pEmailFieldSizer);

    wxBoxSizer* pTelNumFieldSizer = new wxBoxSizer(wxHORIZONTAL);
    GT_ASSERT_ALLOCATION(pTelNumFieldSizer);

    // A sizer that will hold the advanced options hyperlinks:
    _pAdvancedOptionsSizer = new wxBoxSizer(wxVERTICAL);
    GT_ASSERT_ALLOCATION(_pAdvancedOptionsSizer);

    // Create a sizer for the buttons:
    wxBoxSizer* pButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
    GT_ASSERT_ALLOCATION(pButtonsSizer);

    // Company logo
    // Build the path in which the Images reside:
    gtString companyLogoBitmapString;

    // Get the CodeXL images path:
    bool rc = gdGetApplicationImagesPath(companyLogoBitmapString);
    GT_IF_WITH_ASSERT(rc)
    {
        // Company logo
        companyLogoBitmapString.append(osFilePath::osPathSeparator);

        // We use a different, wider logo in Linux / Mac, because of larger fonts
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        companyLogoBitmapString.append(GD_STR_CodeXLLogoFileName);
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
        companyLogoBitmapString.append(GD_STR_CodeXLLogoWideFileName);
#else
#error Error: unknown build configuration.
#endif
    }

    wxBitmap companyLogoBitmap(companyLogoBitmapString.asCharArray(), wxBITMAP_TYPE_PNG);

    wxStaticBitmap* pCompanyLogo = new wxStaticBitmap();
    pCompanyLogo->Create(this, -1, companyLogoBitmap);

    // Create static text items:
    wxStaticText* pStaticText = new wxStaticText(this, wxID_ANY, GD_STR_RequestFreeDialogStaticText1 GD_STR_RequestFreeDialogStaticText2);
    GT_ASSERT_ALLOCATION(pStaticText);

    wxHyperlinkCtrl* pClickHereHyperLink = new wxHyperlinkCtrl(this, ID_REQUEST_FREE_LICENSE_CLICK_HERE, GD_STR_RequestFreeDialogViewSentData, GD_STR_RequestFreeDialogViewSentData);
    pClickHereHyperLink->SetBackgroundColour(defaultBackgroundcolor);
    GT_ASSERT_ALLOCATION(pClickHereHyperLink);

    wxHyperlinkCtrl* pUsingAProxyHyperLink = new wxHyperlinkCtrl(this, ID_REQUEST_FREE_LICENSE_USING_PROXY, GD_STR_RequestFreeDialogUsingProxy, GD_STR_RequestFreeDialogUsingProxy);
    pUsingAProxyHyperLink->SetBackgroundColour(defaultBackgroundcolor);
    GT_ASSERT_ALLOCATION(pUsingAProxyHyperLink);

    /* // Uri, 22/6/11 - removing this until we have a free license form in the developer.amd.com website.
    wxHyperlinkCtrl* pGetLicenseByWebFormHyperLink = new wxHyperlinkCtrl(this, ID_REQUEST_FREE_LICENSE_LICENSE_BY_EMAIL, GD_STR_RequestFreeDialogLicenseByForm, GD_STR_RequestFreeDialogLicenseByForm);
    pGetLicenseByWebFormHyperLink->SetBackgroundColour(defaultBackgroundcolor);
    GT_ASSERT_ALLOCATION(pGetLicenseByWebFormHyperLink);
    */

    _pSendMeInformationCheckBox = new wxCheckBox(this, ID_REQUEST_FREE_LICENSE_SEND_ME_INFORMATION_CHECKBOX, GD_STR_RequestFreeDialogSendMeInformation);
    _pSendMeInformationCheckBox->SetBackgroundColour(defaultBackgroundcolor);
    GT_ASSERT_ALLOCATION(_pSendMeInformationCheckBox);
    _pSendMeInformationCheckBox->SetValue(false);

    _pSendMePartnerInformationCheckBox = new wxCheckBox(this, ID_REQUEST_FREE_LICENSE_SEND_ME_PARTNER_INFORMATION_CHECKBOX, GD_STR_RequestFreeDialogSendMePartnerInformation);
    _pSendMePartnerInformationCheckBox->SetBackgroundColour(defaultBackgroundcolor);
    GT_ASSERT_ALLOCATION(_pSendMePartnerInformationCheckBox);
    _pSendMePartnerInformationCheckBox->SetValue(false);

    wxStaticText* pStaticText1 = new wxStaticText(this, wxID_ANY, GD_STR_RequestFreeDialogStaticText3, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
    GT_ASSERT_ALLOCATION(pStaticText1);

    wxStaticText* pNameFieldStaticText = new wxStaticText(this, wxID_ANY, GD_STR_Name);
    GT_ASSERT_ALLOCATION(pNameFieldStaticText);

    wxStaticText* pCompanyFieldStaticText = new wxStaticText(this, wxID_ANY, GD_STR_RequestFreeDialogCompanyFieldCaption);
    GT_ASSERT_ALLOCATION(pCompanyFieldStaticText);

    wxStaticText* pEmailFieldStaticText = new wxStaticText(this, wxID_ANY, GD_STR_RequestFreeDialogEmailFieldCaption);
    GT_ASSERT_ALLOCATION(pEmailFieldStaticText);

    wxStaticText* pTelNumFieldStaticText = new wxStaticText(this, wxID_ANY, GD_STR_RequestFreeDialogTelNumFieldCaption);
    GT_ASSERT_ALLOCATION(pTelNumFieldStaticText);

    // Set the pNameFieldStaticText and pEmailFieldStaticText to be the same width
    int fieldStaticTextsWidth = pNameFieldStaticText->GetSize().GetWidth();
    fieldStaticTextsWidth = max(fieldStaticTextsWidth, pCompanyFieldStaticText->GetSize().GetWidth());
    wxSize fieldStaticTextsMinSize(fieldStaticTextsWidth, -1);
    pNameFieldStaticText->SetMinSize(fieldStaticTextsMinSize);
    pCompanyFieldStaticText->SetMinSize(fieldStaticTextsMinSize);
    pEmailFieldStaticText->SetMinSize(fieldStaticTextsMinSize);
    pTelNumFieldStaticText->SetMinSize(fieldStaticTextsMinSize);

    // Create input field items:
    _pNameInputField = new acTextCtrl(this, ID_REQUEST_FREE_LICENSE_NAME_FIELD, "");
    GT_ASSERT_ALLOCATION(_pNameInputField);

    _pCompanyInputField = new acTextCtrl(this, ID_REQUEST_FREE_LICENSE_COMPANY_FIELD, "");
    GT_ASSERT_ALLOCATION(_pCompanyInputField);

    _pEmailInputField = new acTextCtrlWithInlineDescription(this, ID_REQUEST_FREE_LICENSE_EMAIL_FIELD, GD_STR_RequestFreeDialogEmailFieldInlineText);
    GT_ASSERT_ALLOCATION(_pEmailInputField);

    _pTelNumberInputField = new acTextCtrl(this, ID_REQUEST_FREE_LICENSE_TEL_NUM_FIELD, "");
    GT_ASSERT_ALLOCATION(_pTelNumberInputField);

    pNameFieldSizer->Add(pNameFieldStaticText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    pNameFieldSizer->Add(_pNameInputField, 1);

    pCompanyFieldSizer->Add(pCompanyFieldStaticText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    pCompanyFieldSizer->Add(_pCompanyInputField, 1);

    pEmailFieldSizer->Add(pEmailFieldStaticText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    pEmailFieldSizer->Add(_pEmailInputField, 1);

    pTelNumFieldSizer->Add(pTelNumFieldStaticText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    pTelNumFieldSizer->Add(_pTelNumberInputField, 1);

    _pAdvancedOptionsSizer->Add(pUsingAProxyHyperLink, 0, wxALIGN_LEFT | wxBOTTOM, 10);
    // Uri, 22/6/11 - see the comment above near the creation of pGetLicenseByWebFormHyperLink.
    // _pAdvancedOptionsSizer->Add(pGetLicenseByWebFormHyperLink, 0, wxALIGN_LEFT | wxBOTTOM, 10);

    // Add items into the main dialog sizer:
    _pTextItemsSizer->Add(pStaticText, 0, wxALIGN_LEFT | wxBOTTOM | wxLEFT | wxRIGHT, 10);
    _pTextItemsSizer->Add(pNameFieldSizer, 0, wxALIGN_LEFT | wxBOTTOM | wxLEFT | wxRIGHT | wxGROW, 10);
    _pTextItemsSizer->Add(pCompanyFieldSizer, 0, wxALIGN_LEFT | wxBOTTOM | wxLEFT | wxRIGHT | wxGROW, 10);
    _pTextItemsSizer->Add(pEmailFieldSizer, 0, wxALIGN_LEFT | wxBOTTOM | wxLEFT | wxRIGHT | wxGROW, 10);
    _pTextItemsSizer->Add(pTelNumFieldSizer, 0, wxALIGN_LEFT | wxBOTTOM | wxLEFT | wxRIGHT | wxGROW, 10);
    _pTextItemsSizer->Add(_pSendMeInformationCheckBox, 0, wxALIGN_LEFT | wxBOTTOM | wxLEFT | wxRIGHT, 10);
    _pTextItemsSizer->Add(_pSendMePartnerInformationCheckBox, 0, wxALIGN_LEFT | wxBOTTOM | wxLEFT | wxRIGHT, 10);
    _pTextItemsSizer->Add(pClickHereHyperLink, 0, wxALIGN_LEFT | wxBOTTOM | wxLEFT | wxRIGHT, 10);
    _pTextItemsSizer->Add(_pAdvancedOptionsSizer, 0, wxBOTTOM | wxLEFT | wxRIGHT | wxGROW, 10);
    _pTextItemsSizer->AddStretchSpacer(1);
    _pTextItemsSizer->Add(pStaticText1, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT, 10);
    _pTextItemsSizer->AddSpacer(5);
    _pTextItemsSizer->Hide(_pAdvancedOptionsSizer, true);

    // Create the dialog buttons:
    _pAdvancedOptionsButton = new wxButton(this, ID_REQUEST_FREE_LICENSE_ADVANCED_BUTTON, GD_STR_RequestFreeDialogAdvancedButtonCaptionShow);
    GT_ASSERT_ALLOCATION(_pAdvancedOptionsButton);

    _pPrivacyPolicyButton = new wxButton(this, ID_REQUEST_FREE_LICENSE_PRIVACY_POLICY, GD_STR_RequestFreeDialogPrivacyPolicyButtonCaption);
    GT_ASSERT_ALLOCATION(_pPrivacyPolicyButton);

    _pSendRequestButton = new wxButton(this, ID_REQUEST_FREE_LICENSE_SEND_BUTTON, GD_STR_RequestFreeDialogSendButtonCaption);
    GT_ASSERT_ALLOCATION(_pSendRequestButton);

    _pCancelButton = new wxButton(this, ID_REQUEST_FREE_LICENSE_CANCEL_BUTTON, GD_STR_RequestFreeDialogCancelButtonCaption);
    GT_ASSERT_ALLOCATION(_pCancelButton);

    // Add the buttons to the buttons sizer:
    pButtonsSizer->Add(_pAdvancedOptionsButton, 0, wxRIGHT, 10);
    pButtonsSizer->Add(_pPrivacyPolicyButton, 0, wxRIGHT, 10);
    pButtonsSizer->AddStretchSpacer(1);
    pButtonsSizer->Add(_pSendRequestButton, 0, wxLEFT, 10);
    pButtonsSizer->Add(_pCancelButton, 0, wxLEFT, 10);

    // Add items into the main dialog sizer:
    pDialogVerticalSizer->Add(_pTextItemsSizer, 1, wxTOP | wxLEFT | wxRIGHT | wxALIGN_CENTER | wxGROW, 10);
    pDialogVerticalSizer->Add(pButtonsSizer, 0, wxALL | wxGROW, 10);
    pDialogMainSizer->Add(pDialogVerticalSizer, 1, wxGROW, 0);
    pDialogMainSizer->Add(pCompanyLogo, 0, wxLEFT, 10);

    // Set this sizer ad the main dialog sizer:
    SetSizer(pDialogMainSizer);
    SetAutoLayout(true);
    pDialogMainSizer->SetSizeHints(this);
    pDialogMainSizer->Fit(this);

    // Set the Name input field to be focused:
    _pNameInputField->SetFocus();

    // Set the Yes button to be the default button:
    _pSendRequestButton->SetDefault();

    // Center the dialog box on the entire screen:
    Centre(wxBOTH | wxCENTRE_ON_SCREEN);
}


// ---------------------------------------------------------------------------
// Name:        gdRequestFreeLicenseDialog::onAcceptButton
// Description: Is called when the user clicks on "Yes" button.
// Author:      Yaki Tebeka
// Date:        19/5/2008
// ---------------------------------------------------------------------------
void gdRequestFreeLicenseDialog::onSendRequestButton(wxCommandEvent& event)
{
    bool isValid = validateFreeLicenseRequestInfo();

    if (isValid)
    {
        // Get the user's (licensee) name and email:
        gtString licenseeName(_pNameInputField->GetValue().c_str());
        gtString licenseeCompany(_pCompanyInputField->GetValue().c_str());
        gtString licenseeEmail(_pEmailInputField->GetValue().c_str());
        gtString licenseeTelephone(_pTelNumberInputField->GetValue().c_str());
        bool sendMeInfo = _pSendMeInformationCheckBox->GetValue();
        bool sendMePartnerInfo = _pSendMePartnerInformationCheckBox->GetValue();

        // Check if a proxy is used:
        bool isUsingProxy = false;
        osPortAddress proxyServer("", 0);
        bool rc1 = gdCodeXLGlobalVariablesManager::instance().getProxyInformation(isUsingProxy, proxyServer);

        if (!rc1)
        {
            GT_ASSERT(rc1);
            isUsingProxy = false;
        }

        // Request and, if available, install a free license file:
        lmLicenseStatus freeLicenseStatus = lmRequestAndInstallFreeLicense(licenseeName, licenseeCompany, licenseeEmail, licenseeTelephone, isUsingProxy, proxyServer, sendMeInfo, sendMePartnerInfo);

        if (freeLicenseStatus == LIC_FREE_LICENSE_OK)
        {
            // Log the newly installed license:
            gdGetAndStoreLicenseParameters();
            _wasFreeLicenseInstalled = true;

            // Display a success message to the user:
            acMessageBox(GD_STR_freeLicenseInstalledText, GD_STR_freeLicenseInstalledTitle, wxICON_INFORMATION | wxOK);

            // Exit the dialog:
            EndModal(ID_REQUEST_FREE_LICENSE_SEND_BUTTON);
        }
        else
        {
            // Display an error message box:
            osWindowHandle dialogNativeWinHandle = (osWindowHandle)(this->GetHandle());
            lmDisplayLicenseErrorMessage(freeLicenseStatus, GD_STR_CodeXLProductLineName, true, dialogNativeWinHandle);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdRequestFreeLicenseDialog::onCancelButton
// Description: Is called when the user clicks on the "No" button.
// Author:      Yaki Tebeka
// Date:       19/5/2008
// ---------------------------------------------------------------------------
void gdRequestFreeLicenseDialog::onCancelButton(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

// ---------------------------------------------------------------------------
// Name:        gdRequestFreeLicenseDialog::onAdvancedOptionsButton
// Description: Is called when the user clicks the "More >>" / "Less <<" button
// Author:      Uri Shomroni
// Date:        11/2/2009
// ---------------------------------------------------------------------------
void gdRequestFreeLicenseDialog::onAdvancedOptionsButton(wxCommandEvent& event)
{
    if (_areAdvancedOptionsShown)
    {
        // Hide the options and change the button to note "More >>":
        _pTextItemsSizer->Hide(_pAdvancedOptionsSizer, true);
        _pAdvancedOptionsButton->SetLabel(GD_STR_RequestFreeDialogAdvancedButtonCaptionShow);
    }
    else
    {
        // Show the options and change the button to note "Less <<":
        _pTextItemsSizer->Show(_pAdvancedOptionsSizer, true, true);
        _pAdvancedOptionsButton->SetLabel(GD_STR_RequestFreeDialogAdvancedButtonCaptionHide);
    }

    // Apply the change to the window (and resize it if needed):
    wxSizer* pMainSizer = GetSizer();
    pMainSizer->Layout();
    pMainSizer->Fit(this);
    pMainSizer->SetSizeHints(this);

    // Toggle the flag:
    _areAdvancedOptionsShown = !_areAdvancedOptionsShown;
}

// ---------------------------------------------------------------------------
// Name:        gdRequestFreeLicenseDialog::onPrivacyPolicyButton
// Description: Called when the user presses the "privacy policy" button.
// Author:      Uri Shomroni
// Date:        9/6/2011
// ---------------------------------------------------------------------------
void gdRequestFreeLicenseDialog::onPrivacyPolicyButton(wxCommandEvent& eve)
{
    osFileLauncher fileLauncher(GD_STR_PrivacyPolicyURL);
    bool rc = fileLauncher.launchFile();
    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        gdRequestFreeLicenseDialog::onClickHerePressed
// Description: Is called when the user clicks the "click here to see the sent
//              information" hyperlink
// Author:      Uri Shomroni
// Date:        28/5/2008
// ---------------------------------------------------------------------------
void gdRequestFreeLicenseDialog::onClickHerePressed(wxHyperlinkEvent& eve)
{
    if (eve.GetId() == ID_REQUEST_FREE_LICENSE_CLICK_HERE)
    {
        gtString freeRequestDetails;
        getFreeRequestDetails(freeRequestDetails);

        // Open the more details Dialog:
        acMessageDialog detailsDialog(freeRequestDetails.asCharArray(), GD_STR_RequestFreeDialogDetailsDialogTitle, GD_STR_RequestFreeDialogDetailsDescription GD_STR_NewLine GD_STR_NewLine GD_STR_PrivacyPolicyUserInformationDisclaimer, this, wxID_ANY, wxSize(350, 400));
        gdSetWindowAlwaysOnTopStatus(detailsDialog);

        // Show the dialog:
        detailsDialog.ShowModal();
    }
}


// ---------------------------------------------------------------------------
// Name:        gdRequestFreeLicenseDialog::onUsingAProxyPressed
// Description: Is called when the user clicks the "Using a proxy" hyperlink
// Author:      Uri Shomroni
// Date:        4/9/2008
// ---------------------------------------------------------------------------
void gdRequestFreeLicenseDialog::onUsingAProxyPressed(wxHyperlinkEvent& eve)
{
    if (eve.GetId() == ID_REQUEST_FREE_LICENSE_USING_PROXY)
    {
        // Show the options dialog, starting with the "connection" tab
        gdOptionsDialog optionsDialog(this);
        optionsDialog.ShowModalOnConnectionTab();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdRequestFreeLicenseDialog::onLicenseByWebFormPressed
// Description: Is called when the user clicks the "Get a license by email" hyperlink
// Author:      Avi Shapira
// Date:        18/7/2008
// ---------------------------------------------------------------------------
void gdRequestFreeLicenseDialog::onLicenseByWebFormPressed(wxHyperlinkEvent& eve)
{
    if (eve.GetId() == ID_REQUEST_FREE_LICENSE_LICENSE_BY_EMAIL)
    {
        gtString licenseByWebFormDetails;
        getLicenseByWebFormDetails(licenseByWebFormDetails);

        // Open the more details Dialog:
        acMessageDialog detailsDialog(licenseByWebFormDetails.asCharArray(), GD_STR_RequestFreeDialogLicenseByFormDialogTitle, GD_STR_RequestFreeDialogLicenseByFormDetailsDescription, this, wxID_ANY, wxSize(350, 400));
        gdSetWindowAlwaysOnTopStatus(detailsDialog);

        // Show the dialog:
        detailsDialog.ShowModal();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdRequestFreeLicenseDialog::getFreeRequestDetails
// Description: Constructs a string to show in the "sent information" dialog
// Arguments: freeRequestDetails the details go here
// Author:      Uri Shomroni
// Date:        28/5/2008
// ---------------------------------------------------------------------------
void gdRequestFreeLicenseDialog::getFreeRequestDetails(gtString& freeRequestDetails)
{
    freeRequestDetails.makeEmpty();

    // Get the user's name:
    gtString nameString(_pNameInputField->GetValue().c_str());

    if (nameString.isEmpty())
    {
        nameString = GD_STR_NotAvailable;
    }

    // Get the user's company name:
    gtString companyName(_pCompanyInputField->GetValue().c_str());

    if (companyName.isEmpty())
    {
        companyName = GD_STR_NotAvailable;
    }

    // Get the user's email:
    gtString emailString(_pEmailInputField->GetValue().c_str());

    if (emailString.isEmpty())
    {
        emailString = GD_STR_NotAvailable;
    }

    // Get the user's telephone number
    gtString telNum(_pTelNumberInputField->GetValue().c_str());

    if (telNum.isEmpty())
    {
        telNum = GD_STR_NotAvailable;
    }

    // Get the software version:
    osProductVersion currApplicationVersion;
    osGetApplicationVersion(GD_STANDALONE_APPLICATION_TYPE, currApplicationVersion);
    gtString applicationVersionAsString = currApplicationVersion.toString();

    // Get the local computer name:
    gtString localMachineName;
    bool rc1 = osGetLocalMachineName(localMachineName);

    if ((!rc1) || localMachineName.isEmpty())
    {
        localMachineName = GD_STR_NotAvailable;
    }

    // Get the local computer id:
    gtString localMachineMACAddress;
    bool rc2 = lmGetCurrentMachineFirstValidMacAddress(localMachineMACAddress);

    if ((!rc2) || localMachineMACAddress.isEmpty())
    {
        localMachineMACAddress = GD_STR_NotAvailable;
    }

    // Get the OS type:
    gtString osTypeString;
    osGetOSShortDescriptionString(osTypeString);

    // Get the binaries address space:
    gtString binariesAddressSpace;
    osGetBinariesAddressSpaceString(binariesAddressSpace);

    // Build the platform description string:
    gtString platformString = osTypeString;
    platformString += L"-bin";
    platformString += binariesAddressSpace;

    // On Linux - add the C++ compiler version to the OS type:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    {
        platformString.appendFormattedString(L"cpp%d", AMDT_GNU_CPP_COMPILER_VERSION);
    }
#endif


    // Add the user information:
    freeRequestDetails.append(GD_STR_Name);
    freeRequestDetails.append(L":\n");
    freeRequestDetails.append(nameString);
    freeRequestDetails.append(L"\n\n");

    // Add the company name information:
    freeRequestDetails.append(GD_STR_RequestFreeDialogCompanyFieldCaption);
    freeRequestDetails.append(L":\n");
    freeRequestDetails.append(companyName);
    freeRequestDetails.append(L"\n\n");

    // Add the user email address:
    freeRequestDetails.append(GD_STR_RequestFreeDialogEmailFieldCaption);
    freeRequestDetails.append(L":\n");
    freeRequestDetails.append(emailString);
    freeRequestDetails.append(L"\n\n");

    // Add the telephone number information:
    freeRequestDetails.append(GD_STR_RequestFreeDialogTelNumFieldCaption);
    freeRequestDetails.append(L":\n");
    freeRequestDetails.append(telNum);
    freeRequestDetails.append(L"\n\n");

    // Add the current application version:
    freeRequestDetails.append(GD_STR_RequestFreeDialogApplicationVersion);
    freeRequestDetails.append(L":\n");
    freeRequestDetails.append(applicationVersionAsString);
    freeRequestDetails.append(L"\n\n");

    // Add the local computer name:
    freeRequestDetails.append(GD_STR_RequestFreeDialogComputerName);
    freeRequestDetails.append(L":\n");
    freeRequestDetails.append(localMachineName);
    freeRequestDetails.append(L"\n\n");

    // Add the local machine MAC address:
    freeRequestDetails.append(GD_STR_RequestFreeDialogComputerID);
    freeRequestDetails.append(L":\n");
    freeRequestDetails.append(localMachineMACAddress);
    freeRequestDetails.append(L"\n\n");

    // Add the OS type:
    freeRequestDetails.append(GD_STR_RequestFreeDialogOSType);
    freeRequestDetails.append(L":\n");
    freeRequestDetails.append(platformString);
}


// ---------------------------------------------------------------------------
// Name:        gdRequestFreeLicenseDialog::getLicenseByWebFormDetails
// Description: Constructs a string to show in the "Get License By Email" dialog
// Arguments: licenseByWebFormDetails the details go here
// Author:      Avi Shapira
// Date:        28/5/2008
// ---------------------------------------------------------------------------
void gdRequestFreeLicenseDialog::getLicenseByWebFormDetails(gtString& licenseByWebFormDetails)
{
    licenseByWebFormDetails.makeEmpty();

    // Get the free request details.
    gtString freeRequestDetails;
    getFreeRequestDetails(freeRequestDetails);

    licenseByWebFormDetails = GD_STR_RequestFreeDialogLicenseByFormHeader;
    licenseByWebFormDetails += freeRequestDetails;
    licenseByWebFormDetails.append(GD_STR_RequestFreeDialogLicenseByFormFooter);
}


// ---------------------------------------------------------------------------
// Name:        gdRequestFreeLicenseDialog::validateFreeLicenseRequestInfo
// Description: Validates the licensee information (ie name and email)
// Return Val: bool - Valid / Invalid
// Author:      Uri Shomroni
// Date:        29/5/2008
// ---------------------------------------------------------------------------
bool gdRequestFreeLicenseDialog::validateFreeLicenseRequestInfo()
{
    bool retVal = true;

    gtString licenseeName(_pNameInputField->GetValue().c_str());
    gtString licenseeCompany(_pCompanyInputField->GetValue().c_str());
    gtString licenseeEmail(_pEmailInputField->GetValue().c_str());
    gtString licenseeTel(_pTelNumberInputField->GetValue().c_str());

    // Verify that name, company and tel number are not empty:
    bool isNameOk = !licenseeName.isEmpty();
    bool isCompanyOk = !licenseeCompany.isEmpty();
    bool isTelNumOk = !licenseeTel.isEmpty();

    // Verify that email has exactly one @ * Email has at least one . after the @
    // email does not end in . * there isn't a . right after the @
    int emailLength = licenseeEmail.length();
    int emailFirstAt = licenseeEmail.find('@');
    int emailSecondAt = licenseeEmail.find('@', emailFirstAt + 1);
    int emailLastPoint = licenseeEmail.reverseFind('.');
    int emailFirstPointAfterAt = licenseeEmail.find('.', emailFirstAt + 1);
    bool isEmailOk = ((emailFirstAt != -1) && (emailSecondAt == -1) &&
                      (emailLastPoint != (emailLength - 1)) && (emailLastPoint > emailFirstAt) &&
                      (emailFirstPointAfterAt > (emailFirstAt + 1)));

    gtVector<gtString> failureStrings;

    if (!isNameOk)
    {
        failureStrings.push_back(gtString(GD_STR_RequestFreeDialogInvalidInformationName));
    }

    if (!isCompanyOk)
    {
        failureStrings.push_back(gtString(GD_STR_RequestFreeDialogInvalidInformationCompany));
    }

    if (!isTelNumOk)
    {
        failureStrings.push_back(gtString(GD_STR_RequestFreeDialogInvalidInformationTelephone));
    }

    if (!isEmailOk)
    {
        failureStrings.push_back(gtString(GD_STR_RequestFreeDialogInvalidInformationEmail));
    }

    // If there were failures
    int failureStringsAmount = failureStrings.size();

    if (0 < failureStringsAmount)
    {
        retVal = false;

        wxString errMsg = GD_STR_RequestFreeDialogInvalidInformation;

        for (int i = 0; i < failureStringsAmount; i++)
        {
            if (1 < failureStringsAmount)
            {
                if (i == (failureStringsAmount - 1))
                {
                    errMsg += " and ";
                }
                else if (0 < i)
                {
                    errMsg += ", ";
                }
            }

            errMsg += failureStrings[i].asCharArray();
        }

        errMsg += ".";

        acMessageBox(errMsg, GD_STR_RequestFreeDialogInvalidInformationTitle, wxICON_ERROR | wxOK);
    }

    return retVal;
}

