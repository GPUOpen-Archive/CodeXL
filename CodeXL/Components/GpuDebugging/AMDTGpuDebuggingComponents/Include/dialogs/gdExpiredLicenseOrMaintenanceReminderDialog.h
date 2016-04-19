//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdExpiredLicenseOrMaintenanceReminderDialog.h
///
//==================================================================================

//------------------------------ gdExpiredLicenseOrMaintenanceReminderDialog.h ------------------------------

#ifndef __GDEXPIREDLICENSEORMAINTENANCEREMINDERDIALOG_H
#define __GDEXPIREDLICENSEORMAINTENANCEREMINDERDIALOG_H


// Forward decelerations:
struct lmLicenseParameters;

// ----------------------------------------------------------------------------------
// Class Name:           gdExpiredLicenseOrMaintenanceReminderDialog : public acDialog
// General Description:
//   A dialog that reminds the user that his maintenance period is about to expire.
//
// Author:               Avi Shapira
// Creation Date:        18/2/2008
// ----------------------------------------------------------------------------------
class GD_API gdExpiredLicenseOrMaintenanceReminderDialog : public acDialog
{
public:
    gdExpiredLicenseOrMaintenanceReminderDialog(wxWindow* parent, const lmLicenseParameters& aboutToExpireLicenseParameters);
    gdReminderDate userDecision();

private:
    void setDialogLayout();
    void onPurchaseButton(wxCommandEvent& eve);
    bool isLicenseAboutToExpire(const lmLicenseParameters& licenseParameters);

    // Do not allow the use of my default constructor:
    gdExpiredLicenseOrMaintenanceReminderDialog();

private:
    // wxWidgets events table:
    DECLARE_EVENT_TABLE()

    // Hold the parameters of the license that it's maintenance package is about to expire:
    const lmLicenseParameters& _aboutToExpireLicenseParameters;

    // The dialog icon:
    wxStaticBitmap* _pInformationIcon;

    wxStaticBitmap* _pCompanyLogo;

    // Dialog buttons:
    wxButton* _pOKButton;
    wxButton* _pPurchaseButton;

    // Hold the expiration date as a string:
    gtString _expirationDateString;

    // Holds the current project type as a string:
    gtString _currentProjectTypeName;

    // The dialog radio buttons;
    wxRadioButton* _pRemindMeAgain;
    wxRadioButton* _pRemindMeInOneWeek;
    wxRadioButton* _pDontRemindMeAgain;

    // Gets true iff the license is about to expire:
    bool _isLicenseAboutToExpire;
};



#endif //__GDEXPIREDLICENSEORMAINTENANCEREMINDERDIALOG_H

