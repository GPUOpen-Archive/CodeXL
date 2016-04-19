//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdExpiredLicenseOrMaintenanceReminderDialog.cpp
///
//==================================================================================

//------------------------------ gdExpiredLicenseOrMaintenanceReminderDialog.cpp ------------------------------

// For compilers that support pre-compilation, includes "wx/wx.h".


// wxWindows
#include "wx/statline.h"
#include "wx/textctrl.h"
#include "wx/hyperlink.h"
#include "wx/artprov.h"

// Infra:
#include <VersionInfo/VersionInfo.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osFileLauncher.h>
#include <GRLicenseManager/lmLicenseManagerFunctions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationFramework/views/afImageBannerWindow.h>

// Local:
#include <CodeXLAppCode/gdStringConstants.h>
#include <CodeXLAppCode/gdCommandIDs.h>
#include <CodeXLAppCode/gdAidFunctions.h>
#include <CodeXLAppCode/gdCodeXLGlobalVariablesManager.h>
#include <CodeXLAppCode/dialogs/gdHelpAboutDialog.h>
#include <CodeXLAppCode/dialogs/gdExpiredLicenseOrMaintenanceReminderDialog.h>


BEGIN_EVENT_TABLE(gdExpiredLicenseOrMaintenanceReminderDialog, acDialog)
    EVT_BUTTON(ID_EXPIRED_MAINTENANCE_DIALOG_PURCHASE_BUTTON, gdExpiredLicenseOrMaintenanceReminderDialog::onPurchaseButton)
END_EVENT_TABLE()

// ---------------------------------------------------------------------------
// Name:        gdExpiredLicenseOrMaintenanceReminderDialog::gdExpiredLicenseOrMaintenanceReminderDialog
// Description: Constructor
// Author:      Avi Shapira
// Date:        15/9/2007
// ---------------------------------------------------------------------------
gdExpiredLicenseOrMaintenanceReminderDialog::gdExpiredLicenseOrMaintenanceReminderDialog(wxWindow* parent , const lmLicenseParameters& aboutToExpirelicenseParameters)
    : acDialog(parent, wxID_ANY, GD_STR_ExpiredMaintenanceReminderTitle1, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
      _aboutToExpireLicenseParameters(aboutToExpirelicenseParameters), _pInformationIcon(NULL), _isLicenseAboutToExpire(false)
{
    // Add the Icon to the dialog
    gdLoadCodeXLTitleBarIcon(*this);

    // Get licensee name:
    gtASCIIString licenseeNameASCII = aboutToExpirelicenseParameters._licenseeName;

    // Get the licensee name as unicode string:
    gtString licenseeName;
    licenseeName.fromASCIIString(licenseeNameASCII.asCharArray());

    // Check if the license or the maintenance is about to expire:
    _isLicenseAboutToExpire = isLicenseAboutToExpire(_aboutToExpireLicenseParameters);

    // Get the license / maintenance expiration date:
    if (_isLicenseAboutToExpire)
    {
        aboutToExpirelicenseParameters._licenseExpirationDate.dateAsString(_expirationDateString, osTime::WINDOWS_STYLE, osTime::LOCAL);
    }
    else
    {
        aboutToExpirelicenseParameters._upgradesExpirationDate.dateAsString(_expirationDateString, osTime::WINDOWS_STYLE, osTime::LOCAL);
    }

    // Get project type name
    gdCalculateCodeXLProductString(_currentProjectTypeName);

    // Set the dialog title according the project type.
    // If _currentProjectTypeName is empty, keep the dialog title as is.
    if (!_currentProjectTypeName.isEmpty())
    {
        gtString dialogTitle;
        dialogTitle.append(licenseeName.asCharArray());
        dialogTitle.append(L" - ");
        dialogTitle.append(_currentProjectTypeName);

        // If the license is about to expire:
        if (_isLicenseAboutToExpire)
        {
            dialogTitle.append(GD_STR_ExpiredMaintenanceReminderTitle2);
        }
        else
        {
            // Maintenance is about to expire:
            dialogTitle.append(GD_STR_ExpiredMaintenanceReminderTitle1);
        }

        SetTitle(dialogTitle.asCharArray());
    }

    // Creates the dialog objects that will be displayed in the dialog box
    setDialogLayout();
}


// ---------------------------------------------------------------------------
// Name:        gdExpiredLicenseOrMaintenanceReminderDialog::userDecision
// Description: Returns the user's selected check box value.
// Author:      Yaki Tebeka
// Date:        27/2/2008
// ---------------------------------------------------------------------------
gdReminderDate gdExpiredLicenseOrMaintenanceReminderDialog::userDecision()
{
    gdReminderDate retVal = GD_REMIND_WHEN_NEEDED;

    // If we should remind the user in a week from today:
    bool shouldRemindInOneWeek = _pRemindMeInOneWeek->GetValue();

    if (shouldRemindInOneWeek)
    {
        retVal = GD_REMIND_IN_A_WEEK;
    }
    else
    {
        // If we should not remind again:
        bool shouldNotRemindAgain = _pDontRemindMeAgain->GetValue();

        if (shouldNotRemindAgain)
        {
            retVal = GD_DONT_REMIND_AGAIN;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdExpiredLicenseOrMaintenanceReminderDialog::setDialogLayout
// Description: Creates the dialogs internal objects and sets their layout
//              within the dialog.
// Author:      Avi Shapira
// Date:        15/9/2007
// ---------------------------------------------------------------------------
void gdExpiredLicenseOrMaintenanceReminderDialog::setDialogLayout()
{
    // Get the system default background color:
    wxColor defaultBackgroundcolor = acGetSystemDefaultBackgroundColor();
    this->SetBackgroundColour(defaultBackgroundcolor);

    // Create the main sizer wrapper
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // If the license is about to expire:
    wxStaticBoxSizer* pLicenseExpiredNoteSizer = NULL;

    if (_isLicenseAboutToExpire)
    {
        pLicenseExpiredNoteSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, GD_STR_ExpiredMaintenanceReminderAlertSizerTitle2), wxVERTICAL);
    }
    else
    {
        pLicenseExpiredNoteSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, GD_STR_ExpiredMaintenanceReminderAlertSizerTitle1), wxVERTICAL);
    }

    // Create the horizontal sizer that contains the icon and the text
    wxBoxSizer* horzSizer = new wxBoxSizer(wxHORIZONTAL);

    // Create the vertical sizer that contains the radio buttons:
    wxBoxSizer* vertSizer = new wxBoxSizer(wxVERTICAL);

    // Get the current time and date:
    osTime today;
    today.setFromCurrentTime();
    time_t todayAsSec = today.secondsFrom1970();

    // Will get the license / maintenance expiration alert string:
    gtString expiredAlertString;

    // If the license is about to expire:
    if (_isLicenseAboutToExpire)
    {
        expiredAlertString.appendFormattedString(GD_STR_ExpiredMaintenanceReminderAlertBeforeLicenseExpiration, _expirationDateString.asCharArray());
        expiredAlertString.appendFormattedString(GD_STR_ExpiredMaintenanceReminderAlertBeforeExpiration2, _currentProjectTypeName.asCharArray());
    }
    else
    {
        // Maintenance is about to expire:

        // Check if the maintenance package is already expired:
        bool expirationDateHasPassed = false;
        time_t maintenanceExpirationDateAsSec = _aboutToExpireLicenseParameters._upgradesExpirationDate.secondsFrom1970();

        if (maintenanceExpirationDateAsSec < todayAsSec)
        {
            expirationDateHasPassed = true;
        }

        // Set the License Maintenance package expired reminder string:
        if (expirationDateHasPassed)
        {
            expiredAlertString.appendFormattedString(GD_STR_ExpiredMaintenanceReminderAlertBeforeExpirationPast, _expirationDateString.asCharArray());
        }
        else
        {
            expiredAlertString.appendFormattedString(GD_STR_ExpiredMaintenanceReminderAlertBeforeMaintenanceExpiration, _expirationDateString.asCharArray());
        }

        expiredAlertString.appendFormattedString(GD_STR_ExpiredMaintenanceReminderAlertBeforeExpiration1, _currentProjectTypeName.asCharArray());
    }

    // Create the alert icon
    const wxSize imageSize(32, 32);
    wxIcon informationIcon = wxArtProvider::GetIcon(wxART_INFORMATION, wxART_OTHER, imageSize);
    _pInformationIcon = new wxStaticBitmap(this, wxID_ANY, informationIcon);

    // Radio buttons for the reminder option:
    _pRemindMeAgain     = new wxRadioButton;
    _pRemindMeInOneWeek = new wxRadioButton;
    _pDontRemindMeAgain = new wxRadioButton;

    _pRemindMeAgain->Create(this, ID_EXPIRED_MAINTENANCE_REMINDER_DIALOG_REMIND_ME_RADIO_BUTTON, GD_STR_ExpiredMaintenanceReminderRemindMeLater, wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    _pRemindMeInOneWeek->Create(this, ID_EXPIRED_MAINTENANCE_REMINDER_DIALOG_REMIND_ME_ONE_WEEK_RADIO_BUTTON, GD_STR_ExpiredMaintenanceReminderRemindMeOneWeek);
    _pDontRemindMeAgain->Create(this, ID_EXPIRED_MAINTENANCE_REMINDER_DIALOG_DONT_REMIND_ME_RADIO_BUTTON, GD_STR_ExpiredMaintenanceReminderDontRemindMeLater);


    // Add the alert icon to the horizontal sizer
    horzSizer->Add(_pInformationIcon, 0, wxCENTER | wxALL, 5);

    // Add the text to the horizontal sizer
    horzSizer->Add(CreateTextSizer(expiredAlertString.asCharArray()), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);

    vertSizer->Add(_pRemindMeAgain, 0, wxALL, 5);
    vertSizer->Add(_pRemindMeInOneWeek, 0, wxALL, 5);
    vertSizer->Add(_pDontRemindMeAgain, 0, wxLEFT | wxTOP, 5);

    wxSize iconSize = _pInformationIcon->GetBestSize();

    // Create the upper sizer and add the horizontal sizer (icon + text) and the additional text paragraph
    pLicenseExpiredNoteSizer->Add(horzSizer, 0, wxCENTRE | wxALL, 0);
    pLicenseExpiredNoteSizer->Add(vertSizer, 0, wxLEFT, (iconSize.GetX() + 10));

    // If the license is about to expire:
    if (_isLicenseAboutToExpire)
    {
        pLicenseExpiredNoteSizer->Add(CreateTextSizer(GD_STR_ExpiredMaintenanceReminderExtendYourLicenseSuggestion2), 0, wxCENTER | wxALL, 5);
    }
    else
    {
        // The maintenance is about to expire:
        pLicenseExpiredNoteSizer->Add(CreateTextSizer(GD_STR_ExpiredMaintenanceReminderExtendYourLicenseSuggestion1), 0, wxCENTER | wxALL, 5);
    }

    // Add the web page link
    wxHyperlinkCtrl* pWebsiteURL = NULL;

    if (_isLicenseAboutToExpire)
    {
        pWebsiteURL = new wxHyperlinkCtrl(this, wxID_ANY, GD_STR_ExpiredMaintenanceReminderRenewLicenseWebLink1, GD_STR_ExpiredMaintenanceReminderRenewLicenseWebLink1 L"?d");
        GT_ASSERT_ALLOCATION(pWebsiteURL);
        pWebsiteURL->SetToolTip(GD_STR_ExpiredMaintenanceReminderRenewLicenseWebLink1);
        pWebsiteURL->SetBackgroundColour(defaultBackgroundcolor);
    }
    else
    {
        pWebsiteURL = new wxHyperlinkCtrl(this, wxID_ANY, GD_STR_ExpiredMaintenanceReminderRenewLicenseWebLink2, GD_STR_ExpiredMaintenanceReminderRenewLicenseWebLink2 L"?d");
        GT_ASSERT_ALLOCATION(pWebsiteURL);
        pWebsiteURL->SetToolTip(GD_STR_ExpiredMaintenanceReminderRenewLicenseWebLink2);
        pWebsiteURL->SetBackgroundColour(defaultBackgroundcolor);
    }

    pLicenseExpiredNoteSizer->Add(pWebsiteURL, 0, wxCENTRE | wxALL, 10);

    // Add the elements to the main sizer
    sizer->Add(pLicenseExpiredNoteSizer, 0, wxCENTRE | wxALL, 10);

    wxSize sizerSize = sizer->GetMinSize();

    // Calculate the banner width:
    int bannerWidth = sizerSize.x;
    wxWindow* pCodeXLImageBanner = new gdImageBannerWindow(this, bannerWidth);
    GT_ASSERT_ALLOCATION(pCodeXLImageBanner);

    // Create the dialog buttons:
    _pOKButton = new wxButton(pCodeXLImageBanner, wxID_OK, GD_STR_OK_Button);
    _pPurchaseButton = new wxButton(pCodeXLImageBanner, ID_EXPIRED_MAINTENANCE_DIALOG_PURCHASE_BUTTON, GD_STR_ExpiredMaintenanceReminderPurchaseButtonCaption);
    GT_ASSERT_ALLOCATION(_pOKButton);
    GT_ASSERT_ALLOCATION(_pPurchaseButton);

    // Calculate the buttons' locations and set them:
    int buttonWidth = _pOKButton->GetSize().GetX();

    // Make sure both buttons are the same size:
    if (buttonWidth < _pPurchaseButton->GetSize().GetWidth())
    {
        buttonWidth = _pPurchaseButton->GetSize().GetWidth();
        _pOKButton->SetSize(buttonWidth, _pOKButton->GetSize().GetHeight());
    }
    else
    {
        _pPurchaseButton->SetSize(buttonWidth, _pPurchaseButton->GetSize().GetHeight());
    }

    int buttonPosHeight = GD_IMAGE_BANNER_WINDOW_TOP_BUTTON_MARGIN;
    wxPoint okButtonPos = wxPoint(sizerSize.x - buttonWidth - GD_IMAGE_BANNER_WINDOW_RIGHT_BUTTON_MARGIN, buttonPosHeight);
    wxPoint purchaseButtonPos = wxPoint(sizerSize.x - buttonWidth * 2 - GD_IMAGE_BANNER_WINDOW_BUTTON_SPACING - GD_IMAGE_BANNER_WINDOW_RIGHT_BUTTON_MARGIN, buttonPosHeight);

    _pOKButton->SetPosition(okButtonPos);
    _pPurchaseButton->SetPosition(purchaseButtonPos);

    _pPurchaseButton->SetDefault();
    _pPurchaseButton->SetFocus();

    // Get rid of the white border around the buttons:
    wxColour buttonForegroundColor = _pOKButton->GetForegroundColour();
    _pOKButton->SetForegroundColour(buttonForegroundColor);
    _pPurchaseButton->SetForegroundColour(buttonForegroundColor);

    sizer->Add(pCodeXLImageBanner, 0, wxCENTRE | wxALL, 0);

    // Activate the main sizer
    SetMinSize(wxSize(300, -1));
    SetSizer(sizer);
    SetAutoLayout(true);
    sizer->SetSizeHints(this);
    sizer->Fit(this);
    Centre(wxBOTH | wxCENTRE_ON_SCREEN);
}

// ---------------------------------------------------------------------------
// Name:        gdExpiredLicenseOrMaintenanceReminderDialog::onPurchaseButton
// Description: Opens the maintenance package webpage. doesn't close the dialog.
// Arguments: eve - the button event
// Author:      Uri Shomroni
// Date:        6/4/2008
// ---------------------------------------------------------------------------
void gdExpiredLicenseOrMaintenanceReminderDialog::onPurchaseButton(wxCommandEvent& eve)
{
    gtString maintenanceURL;

    if (_isLicenseAboutToExpire)
    {
        maintenanceURL = GD_STR_ExpiredMaintenanceReminderRenewLicenseWebLink1;
    }
    else
    {
        maintenanceURL = GD_STR_ExpiredMaintenanceReminderRenewLicenseWebLink2;
    }

    maintenanceURL.append(L"?d");
    osFileLauncher fileLauncher(maintenanceURL.asCharArray());
    bool rc = fileLauncher.launchFile();
    GT_ASSERT(rc);
}


// ---------------------------------------------------------------------------
// Name:        gdExpiredLicenseOrMaintenanceReminderDialog::isLicenseAboutToExpire
// Description:
//   Checks if a given license is about to expire.
//   Notice - we check the license expiry and not the maintenance expiry.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        24/6/2008
// ---------------------------------------------------------------------------
bool gdExpiredLicenseOrMaintenanceReminderDialog::isLicenseAboutToExpire(const lmLicenseParameters& licenseParameters)
{
    bool retVal = false;

    // Get the current time and date:
    osTime today;
    today.setFromCurrentTime();
    time_t todayAsSec = today.secondsFrom1970();

    // Calculate license or maintenance reminder period in seconds:
    time_t stat_daysToSeconds = (24 * 60 * 60);
    time_t GD_LICENSE_OR_MAINTANANCE_REMINDER_DAYSAsSeconds = VI_LICENSE_OR_MAINTANANCE_REMINDER_DAYS * stat_daysToSeconds;

    // If the amount of days left for license expiration is smaller than VI_LICENSE_OR_MAINTANANCE_REMINDER_DAYS:
    time_t licenseExpirationDateAsSec = licenseParameters._licenseExpirationDate.secondsFrom1970();
    time_t amountOfLicenseSecondsLeft = licenseExpirationDateAsSec - todayAsSec;

    if (amountOfLicenseSecondsLeft < GD_LICENSE_OR_MAINTANANCE_REMINDER_DAYSAsSeconds)
    {
        // The license is about to expire:
        retVal = true;
    }

    return retVal;
}

