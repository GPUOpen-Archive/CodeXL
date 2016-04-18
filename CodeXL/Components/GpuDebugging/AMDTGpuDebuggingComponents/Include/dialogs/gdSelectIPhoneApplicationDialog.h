//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdSelectIPhoneApplicationDialog.h
///
//==================================================================================

//------------------------------ gdSelectIPhoneApplicationDialog.h ------------------------------

#ifndef __GDSELECTIPHONEAPPLICATIONDIALOG_H
#define __GDSELECTIPHONEAPPLICATIONDIALOG_H

// Forward decelerations:
class wxImageList;
class wxListEvent;
class acWXListCtrl;
class osFilePath;

// wxWidgets:
#include <wx/dialog.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdCodeXLAppDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           gdSelectIPhoneApplicationDialog : public acDialog
// General Description:
// Author:               Yaki Tebeka
// Creation Date:        24/5/2009
// ----------------------------------------------------------------------------------
class GD_API gdSelectIPhoneApplicationDialog : public acDialog
{
public:
    gdSelectIPhoneApplicationDialog(wxWindow* pMyParentWindow);
    virtual ~gdSelectIPhoneApplicationDialog();
    const gtString& selectedApplicationPath() const;
    const gtString& selectedApplicationSDKVersion() const;

    // Overrides wxDialog:
    virtual int ShowModal();

private:
    struct gdApplicationsListItemData
    {
    public:
        gdApplicationsListItemData(const osFilePath& applicationBundlePath, const gtString& applicationSDKVersionAsString)
            : _applicationBundlePath(applicationBundlePath), _applicationSDKVersionAsString(applicationSDKVersionAsString) {};
        ~gdApplicationsListItemData() {};

    public:
        // The Full path of the application bundle:
        osFilePath _applicationBundlePath;

        // The SDK version as a string:
        gtString _applicationSDKVersionAsString;
    };

private:
    void setDialogLayout();
    bool setDialogValues();
    bool addDirectoryContentsToListCtrl(const osFilePath& dirPath, const gtString& sdkVersionAsString, int& nextListItemIndex);
    void clearApplicationsList();
    bool addApplicationToDialogValues(const osFilePath& applicationDirPath, const gtString& sdkVersionAsString, int listItemIndex);
    bool getApplicationDisplayName(const gtString& infoPlistFilePath, gtString& displayName);

    void onApplicationsListFocused(wxListEvent& eve);
    void onApplicationsListActivated(wxListEvent& eve);
    void onManualSelectionButton(wxCommandEvent& eve);
    static void applicationsListToolTipCallback(acWXListCtrl* pListCtrl, long item, gtString& toolTipText);

private:
    // Dialog's main sizer:
    wxSizer* _pDialogMainSizer;

    // Dialog's description:
    wxStaticText* _pDialogDescription;

    // iPhone applications list control:
    acWXListCtrl* _pIPhoneApplicationsListCtrl;

    // Manual selection button:
    wxButton* _pManualSelectButton;

    // The bottom image banner:
    wxWindow* _pImageBanner;

    // Dialog buttons:
    wxButton* _pOKButton;
    wxButton* _pCancelButton;

    // Holds the selected application's full path and SDK version:
    gtString _selectedApplicationPath;
    gtString _selectedApplicationSDKVersion;

private:
    // wxWidgets events table:
    DECLARE_EVENT_TABLE()
};


#endif //__GDSELECTIPHONEAPPLICATIONDIALOG_H

