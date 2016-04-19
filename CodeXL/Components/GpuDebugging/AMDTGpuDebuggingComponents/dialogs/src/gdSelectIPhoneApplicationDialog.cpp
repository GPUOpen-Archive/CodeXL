//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdSelectIPhoneApplicationDialog.cpp
///
//==================================================================================

//------------------------------ gdSelectIPhoneApplicationDialog.cpp ------------------------------

// For compilers that support pre-compilation, includes "wx/wx.h".


// Infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTApplicationComponents/Include/acWXListCtrl.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afImageBannerWindow.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/dialogs/gdWXFileDialog.h>
#include <AMDTGpuDebuggingComponents/Include/dialogs/gdSelectIPhoneApplicationDialog.h>

// Layout and sizing constants:
#define GD_IPHONE_APPLICATION_SELECTION_LIST_CONTROL_SIZE wxSize(375, 225)

// wxWidgets events table:
BEGIN_EVENT_TABLE(gdSelectIPhoneApplicationDialog, acDialog)
    EVT_LIST_ITEM_FOCUSED(ID_SELECT_IPHONE_APP_LIST_CTRL, gdSelectIPhoneApplicationDialog::onApplicationsListFocused)
    EVT_LIST_ITEM_SELECTED(ID_SELECT_IPHONE_APP_LIST_CTRL, gdSelectIPhoneApplicationDialog::onApplicationsListFocused)
    EVT_LIST_ITEM_ACTIVATED(ID_SELECT_IPHONE_APP_LIST_CTRL, gdSelectIPhoneApplicationDialog::onApplicationsListActivated)
    EVT_BUTTON(ID_SELECT_IPHONE_APP_MANUAL_BUTTON, gdSelectIPhoneApplicationDialog::onManualSelectionButton)
END_EVENT_TABLE()


// ---------------------------------------------------------------------------
// Name:        gdSelectIPhoneApplicationDialog::gdSelectIPhoneApplicationDialog
// Description: Constructor.
// Arguments: pMyParentWindow - A pointer to this dialog's parent window (or NULL if
//                              there is no parent window.
// Author:      Yaki Tebeka
// Date:        24/5/2009
// ---------------------------------------------------------------------------
gdSelectIPhoneApplicationDialog::gdSelectIPhoneApplicationDialog(wxWindow* pMyParentWindow)
    : acDialog(pMyParentWindow, ID_SELECT_IPHONE_APP_DIALOG, GD_STR_SelectIPhoneAppDialogTitle,
               wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
      _pDialogMainSizer(NULL), _pDialogDescription(NULL), _pIPhoneApplicationsListCtrl(NULL),
      _pManualSelectButton(NULL), _pImageBanner(NULL), _pOKButton(NULL), _pCancelButton(NULL)
{
    // Set the title:
    // this->SetTitle(title.asCharArray());

    // Load and set the dialog's title bar icon:
    afLoadCodeXLTitleBarIcon(*this);

    // Set the default background color to the dialog
    gdSetDialogDefaultBackgroundColor(*this);

    // Create the dialog items and layout them:
    setDialogLayout();

    afSetWindowAlwaysOnTopStatus(*this);
}


// ---------------------------------------------------------------------------
// Name:        gdSelectIPhoneApplicationDialog::~gdSelectIPhoneApplicationDialog
// Description: Destructor
// Author:      Avi Shapira
// Date:        17/5/2006
// ---------------------------------------------------------------------------
gdSelectIPhoneApplicationDialog::~gdSelectIPhoneApplicationDialog()
{
    // Clear the application selection list control:
    clearApplicationsList();
}


// ---------------------------------------------------------------------------
// Name:        gdSelectIPhoneApplicationDialog::selectedApplicationPath
// Description: Returns the path of the selected iPhone application.
// Author:      Yaki Tebeka
// Date:        26/5/2009
// ---------------------------------------------------------------------------
const gtString& gdSelectIPhoneApplicationDialog::selectedApplicationPath() const
{
    return _selectedApplicationPath;
}

// ---------------------------------------------------------------------------
// Name:        gdSelectIPhoneApplicationDialog::selectedApplicationSDKVersion
// Description: Returns the SDK version of the selected application as a string (e.g. "3.1")
// Author:      Uri Shomroni
// Date:        4/5/2010
// ---------------------------------------------------------------------------
const gtString& gdSelectIPhoneApplicationDialog::selectedApplicationSDKVersion() const
{
    return _selectedApplicationSDKVersion;
}

// ---------------------------------------------------------------------------
// Name:        gdSelectIPhoneApplicationDialog::ShowModal
// Description: Displays this dialog in a Modal mode.
// Return Val: int - Represents the dialog's pressed button.
// Author:      Yaki Tebeka
// Date:        24/5/2009
// ---------------------------------------------------------------------------
int gdSelectIPhoneApplicationDialog::ShowModal()
{
    int retVal = wxID_CANCEL;

    // Initialize the dialog items values:
    bool rc1 = setDialogValues();
    GT_IF_WITH_ASSERT(rc1)
    {
        // Display the dialog:
        retVal = wxDialog::ShowModal();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdSelectIPhoneApplicationDialog::setDialogLayout
// Description: Creates the dialog items and layouts them.
// Author:      Yaki Tebeka
// Date:        24/5/2009
// ---------------------------------------------------------------------------
void gdSelectIPhoneApplicationDialog::setDialogLayout()
{
    // Get the system default background color:
    wxColor defaultBackgroundcolor = gdGetSystemDefaultBackgroundColor();
    this->SetBackgroundColour(defaultBackgroundcolor);

    // Description string:
    _pDialogDescription = new wxStaticText;
    GT_ASSERT_ALLOCATION(_pDialogDescription);

    _pDialogDescription->Create(this, wxID_ANY, AF_STR_Empty, wxDefaultPosition, wxDefaultSize, wxTE_RICH | wxTE_MULTILINE);
    _pDialogDescription->SetLabel(GD_STR_SelectIPhoneAppDialogDescription);

    // List control:
    _pIPhoneApplicationsListCtrl = new acWXListCtrl(this, ID_SELECT_IPHONE_APP_LIST_CTRL, wxDefaultPosition,
                                                    GD_IPHONE_APPLICATION_SELECTION_LIST_CONTROL_SIZE,
                                                    wxLC_REPORT | wxSUNKEN_BORDER | wxLC_NO_HEADER | wxLC_SINGLE_SEL);
    GT_ASSERT_ALLOCATION(_pIPhoneApplicationsListCtrl);

    // The list will contain a single column:
    wxListItem listItem;
    listItem.m_mask = wxLIST_MASK_TEXT;
    listItem.m_text = GD_STR_SelectIPhoneAppDialogListColumnName;
    listItem.m_image = -1;
    listItem.m_format = wxLIST_FORMAT_LEFT;
    _pIPhoneApplicationsListCtrl->InsertColumn(0, listItem);

    // Adjust the column width to the list size:
    int startupListWidth = _pIPhoneApplicationsListCtrl->GetClientSize().GetWidth();
    int listNetoWidth = afGetClientNetoWidth(startupListWidth);
    _pIPhoneApplicationsListCtrl->SetColumnWidth(0, listNetoWidth);

    // Set the list's tooltip handler:
    _pIPhoneApplicationsListCtrl->setToolTipCallBack(&applicationsListToolTipCallback);

    // Manual select button:
    _pManualSelectButton = new wxButton(this, ID_SELECT_IPHONE_APP_MANUAL_BUTTON, GD_STR_SelectIPhoneAppDialogManualButton);
    _pManualSelectButton->SetToolTip(GD_STR_SelectIPhoneAppDialogManualButtonToolTip);

    // Dialog's main sizer:
    _pDialogMainSizer = new wxBoxSizer(wxVERTICAL);
    GT_ASSERT_ALLOCATION(_pDialogMainSizer);

    _pDialogMainSizer->Add(_pDialogDescription, 0, wxALL | wxGROW, 10);
    _pDialogMainSizer->Add(_pIPhoneApplicationsListCtrl, 1, wxALL | wxGROW, 10);
    _pDialogMainSizer->Add(_pManualSelectButton, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxALIGN_RIGHT, 10);

    // Bottom image banner:
    wxSize dialogMainSizerSize = _pDialogMainSizer->GetMinSize();
    int bannerWidth = dialogMainSizerSize.x;
    _pImageBanner = new afImageBannerWindow(this, bannerWidth);
    GT_ASSERT_ALLOCATION(_pImageBanner);

    // Create the dialog buttons:
    _pOKButton = new wxButton(_pImageBanner, wxID_OK, AF_STR_OK_Button);
    _pCancelButton = new wxButton(_pImageBanner, wxID_CANCEL, AF_STR_Cancel_Button);
    GT_ASSERT_ALLOCATION(_pOKButton);
    GT_ASSERT_ALLOCATION(_pCancelButton);

    // Calculate and set the buttons' locations:
    int buttonWidth = _pOKButton->GetSize().GetX();
    int buttonPosY = AF_IMAGE_BANNER_WINDOW_TOP_BUTTON_MARGIN;
    wxPoint okButtonPos = wxPoint(dialogMainSizerSize.x - buttonWidth * 2 - AF_IMAGE_BANNER_WINDOW_BUTTON_SPACING - AF_IMAGE_BANNER_WINDOW_RIGHT_BUTTON_MARGIN, buttonPosY);
    wxPoint cancelButtonPos = wxPoint(dialogMainSizerSize.x - buttonWidth - AF_IMAGE_BANNER_WINDOW_RIGHT_BUTTON_MARGIN, buttonPosY);
    _pCancelButton->SetPosition(cancelButtonPos);
    _pOKButton->SetPosition(okButtonPos);

    // Set the "OK" button as default:
    _pOKButton->SetDefault();
    _pOKButton->SetFocus();

    // Get rid of the white border around the buttons:
    wxColour buttonForegroundColor = _pOKButton->GetForegroundColour();
    _pOKButton->SetForegroundColour(buttonForegroundColor);
    _pCancelButton->SetForegroundColour(buttonForegroundColor);

    // Add the buttons to the main sizer:
    _pDialogMainSizer->Add(_pImageBanner, 0, wxALL | wxALIGN_LEFT, 0);

    // Activate the dialog's main sizer:
    SetSizer(_pDialogMainSizer);
    SetAutoLayout(true);
    _pDialogMainSizer->SetSizeHints(this);
    _pDialogMainSizer->Fit(this);

    // Center the dialog on the display:
    Centre(wxBOTH | wxCENTRE_ON_SCREEN);
}


// ---------------------------------------------------------------------------
// Name:        gdSelectIPhoneApplicationDialog::setDialogValues
// Description: Initialize the dialog items values.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        24/5/2009
// ---------------------------------------------------------------------------
bool gdSelectIPhoneApplicationDialog::setDialogValues()
{
    bool retVal = true;

    // Clear previous list items:
    clearApplicationsList();

    // Calculate the compiled iPhone simulator applications directory:
    osFilePath simulatorAppsRootDirectoryPath(osFilePath::OS_USER_DOCUMENTS);
    simulatorAppsRootDirectoryPath.appendSubDirectory(AF_STR_CodeXLiPhoneApplicationsRootDirPath);

    // Verify the directory exists:
    osDirectory simulatorAppsRootDirectory(simulatorAppsRootDirectoryPath);

    if (simulatorAppsRootDirectory.exists())
    {
        // Get its subdirectories in reverse name order (so that SDK 4.0 will appear before 3.2, etc.):
        gtList<osFilePath> simulatorAppsRootSubdirectories;
        bool rcDirs = simulatorAppsRootDirectory.getSubDirectoriesPaths(osDirectory::SORT_BY_NAME_DESC, simulatorAppsRootSubdirectories);
        GT_IF_WITH_ASSERT(rcDirs)
        {
            // Will get all subdirectory paths for directories which are not formatted as version numbers:
            gtList<osFilePath> nonVersionSubdirectories;
            static const gtString charactersValidInVersionNumber = "0123456789.";

            // The current item index in the list:
            int listItemIndex = 0;

            // Iterate the subdirectories:
            gtList<osFilePath>::const_iterator iter = simulatorAppsRootSubdirectories.begin();
            gtList<osFilePath>::const_iterator endIter = simulatorAppsRootSubdirectories.end();

            while (iter != endIter)
            {
                // Get the current path:
                const osFilePath& currentSubdirectoryPath = (*iter);

                // Get the last directory name, which we expect to be a version number for most subdirs:
                gtString subdirectoryName = currentSubdirectoryPath.asString();
                int lastPathSeparator = subdirectoryName.removeTrailing(osFilePath::osPathSeparator).reverseFind(osFilePath::osPathSeparator);
                int subDirectoryNameLen = subdirectoryName.length();

                if ((lastPathSeparator > -1) && (lastPathSeparator < subDirectoryNameLen - 1))
                {
                    subdirectoryName.truncate(lastPathSeparator + 1, subDirectoryNameLen - 1);
                }

                // See if this string is a version number format (x.y or x.y.z are expected):
                if (subdirectoryName.onlyContainsCharacters(charactersValidInVersionNumber))
                {
                    // Add the header for this subdirectory:
                    gtString sdkHeader = subdirectoryName;
                    sdkHeader.prepend(GD_STR_CodeXLiPhoneApplicationsDialogSDKHeaderPrefix);
                    _pIPhoneApplicationsListCtrl->InsertItem(listItemIndex, sdkHeader.asCharArray());
                    listItemIndex++;

                    // Add the subdirectory contents to the list:
                    addDirectoryContentsToListCtrl(currentSubdirectoryPath, subdirectoryName, listItemIndex);
                }
                else // !versionIdentifierHelper.fromString(subdirectoryName)
                {
                    // Add it to the vector of subdirectories to be handled under the "Unknown" header:
                    nonVersionSubdirectories.push_back(currentSubdirectoryPath);
                }

                iter++;
            }

            // Add all the other subdirectories under the "unknown" header:
            gtList<osFilePath>::const_iterator nonVersIter = nonVersionSubdirectories.begin();
            gtList<osFilePath>::const_iterator nonVersEndIter = nonVersionSubdirectories.end();

            if (nonVersIter != nonVersEndIter)
            {
                // Add the "unknown" header:
                _pIPhoneApplicationsListCtrl->InsertItem(listItemIndex, AF_STR_CodeXLiPhoneApplicationsDialogUnknownSDKHeader);
                listItemIndex++;

                // Use the default SDK for these items:
                static const gtString defaultSDKVersionNumber = GD_STR_DebugSettingsiPhoneSimulatorDefaultSDKVersionAsString;

                // Iterate all the "other" subdirectories:
                while (nonVersIter != nonVersEndIter)
                {
                    // Add this subdirectory's file paths to the list:
                    addDirectoryContentsToListCtrl(*nonVersIter, defaultSDKVersionNumber, listItemIndex);

                    nonVersIter++;
                }
            }
        }
    }

    // Find the first item that has item data, i.e. the first app in the dialog:
    int numberOfListItems = _pIPhoneApplicationsListCtrl->GetItemCount();
    int defaultSelection = -1;
    const gdApplicationsListItemData* pDefaultSelectionData = NULL;

    for (int i = 0; i < numberOfListItems; i++)
    {
        // If we found an item with data:
        pDefaultSelectionData = (const gdApplicationsListItemData*)_pIPhoneApplicationsListCtrl->GetItemData(i);

        if (pDefaultSelectionData != NULL)
        {
            // Note its index and stop looking:
            defaultSelection = i;
            break;
        }
    }

    // If the dialog has ANY valid items:
    if ((defaultSelection > -1) && (pDefaultSelectionData != NULL))
    {
        // Select that item:
        _pIPhoneApplicationsListCtrl->selectAndFocusItem(defaultSelection);

        // Copy the item's data to our members:
        _selectedApplicationPath = pDefaultSelectionData->_applicationBundlePath.asString();
        _selectedApplicationSDKVersion = pDefaultSelectionData->_applicationSDKVersionAsString;
    }
    else // defaultSelection <= -1
    {
        // Disable the OK button:
        _pOKButton->Disable();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdSelectIPhoneApplicationDialog::addDirectoryContentsToListCtrl
// Description: Adds all the iPhone apps in a directory to the list control.
//              nextListItemIndex inputs as the first item to be added and outputs
//              as the first free index after the items added
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        4/5/2010
// ---------------------------------------------------------------------------
bool gdSelectIPhoneApplicationDialog::addDirectoryContentsToListCtrl(const osFilePath& dirPath, const gtString& sdkVersionAsString, int& nextListItemIndex)
{
    bool retVal = false;

    // Get the Applications subdirectory and verify it exists:
    osFilePath simulatorApplicationsDirPath = dirPath;
    simulatorApplicationsDirPath.reinterpretAsDirectory().appendSubDirectory(AF_STR_CodeXLiPhoneApplicationsDirApplicationsSubdirectoryName);

    if (simulatorApplicationsDirPath.isDirectory())
    {
        // Create an osDirectory:
        osDirectory simulatorAppDirectory(simulatorApplicationsDirPath);
        GT_IF_WITH_ASSERT(simulatorAppDirectory.exists())
        {
            // Get the directory's sub directories:
            gtList<osFilePath> subDirectoriesPaths;
            bool rc1 = simulatorAppDirectory.getSubDirectoriesPaths(osDirectory::SORT_BY_DATE_DESC, subDirectoriesPaths);

            if (rc1)
            {
                // Iterate the sub directories:
                gtList <osFilePath>::iterator subDirectoriesIterator = subDirectoriesPaths.begin();

                while (subDirectoriesIterator != subDirectoriesPaths.end())
                {
                    // Get the current sub directory:
                    const osFilePath& currentSubDirPath = *subDirectoriesIterator;

                    // Add the current application to the dialog displayed values:
                    bool rc2 = addApplicationToDialogValues(currentSubDirPath, sdkVersionAsString, nextListItemIndex);

                    if (rc2)
                    {
                        nextListItemIndex++;
                    }

                    // Next sub directory:
                    subDirectoriesIterator++;
                }
            }
        }
    }
    else // !simulatorApplicationsDirPath.isDirectory()
    {
        // Nothing to add, report success:
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdSelectIPhoneApplicationDialog::clearApplicationsList
// Description: Clears the application selection list control's content.
// Author:      Yaki Tebeka
// Date:        26/5/2009
// ---------------------------------------------------------------------------
void gdSelectIPhoneApplicationDialog::clearApplicationsList()
{
    // Delete the list item's data:
    int listSize = _pIPhoneApplicationsListCtrl->GetItemCount();

    for (int i = 0; i < listSize; i++)
    {
        delete(gdApplicationsListItemData*)(_pIPhoneApplicationsListCtrl->GetItemData(i));
    }

    // Clear the list:
    _pIPhoneApplicationsListCtrl->DeleteAllItems();
}


// ---------------------------------------------------------------------------
// Name:        gdSelectIPhoneApplicationDialog::addApplicationToDialogValues
// Description: Adds a given application to the dialog values.
// Arguments: applicationDirPath - The given application's directory path.
//            sdkVersionAsString - a string of the iPhone Simulator SDK version to be used
//                for this application.
//            listItemIndex - The list control index into which the application should be added.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        26/5/2009
// ---------------------------------------------------------------------------
bool gdSelectIPhoneApplicationDialog::addApplicationToDialogValues(const osFilePath& applicationDirPath, const gtString& sdkVersionAsString, int listItemIndex)
{
    bool retVal = false;

    // Get the application directory sub directories:
    osDirectory applicationDir(applicationDirPath);
    gtList<osFilePath> subDirectoriesPaths;
    bool rc1 = applicationDir.getSubDirectoriesPaths(osDirectory::SORT_BY_DATE_DESC, subDirectoriesPaths);

    if (rc1)
    {
        // Iterate the sub directories:
        gtList <osFilePath>::iterator subDirectoriesIterator = subDirectoriesPaths.begin();
        gtList <osFilePath>::iterator subDirectoriesEndIterator = subDirectoriesPaths.end();

        while (subDirectoriesIterator != subDirectoriesEndIterator)
        {
            // Get the current sub directory:
            const osFilePath& currentSubDirPath = *subDirectoriesIterator;

            // If this is a ".app" directory:
            gtString subDirExtension;
            currentSubDirPath.getFileExtension(subDirExtension);

            if (subDirExtension == GD_STR_CodeXLMacBundleExtension)
            {
                // Calculate its Info.plist file path:
                gtString infoPlistFilePath = currentSubDirPath.asString();
                infoPlistFilePath.append(osFilePath::osPathSeparator).append(L"Info.plist");

                // Get the application display name and executable path out of the Info.plist file:
                gtString applicationDisplayName;
                bool rc2 = getApplicationDisplayName(infoPlistFilePath, applicationDisplayName);
                GT_IF_WITH_ASSERT(rc2)
                {
                    // Indent the app names:
                    applicationDisplayName.prepend(GD_STR_CodeXLiPhoneApplicationsDirAppNameIndentation);

                    // Add the application display name into the dialog's list:
                    long newItemIndex = _pIPhoneApplicationsListCtrl->InsertItem(listItemIndex, applicationDisplayName.asCharArray());

                    // Add a list data item, containing the executable's full path:
                    gdApplicationsListItemData* pItemData = new gdApplicationsListItemData(currentSubDirPath, sdkVersionAsString);
                    GT_ASSERT_ALLOCATION(pItemData);
                    bool rc3 = _pIPhoneApplicationsListCtrl->SetItemData(newItemIndex, (long)pItemData);
                    GT_IF_WITH_ASSERT(rc3)
                    {
                        retVal = true;
                    }
                }

                // We found the ".app" directory, no need to iterate other sub directories:
                break;
            }

            // Next sub directory:
            subDirectoriesIterator++;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdSelectIPhoneApplicationDialog::getApplicationDisplayName
// Description: Inputs a plist file and outputs the associated application display name.
// Arguments: infoPlistFilePath - The info.plist file path.
//            displayName - Will get the application's display name out of the info.plist file.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        26/5/2009
// ---------------------------------------------------------------------------
bool gdSelectIPhoneApplicationDialog::getApplicationDisplayName(const gtString& infoPlistFilePath, gtString& displayName)
{
    bool retVal = false;

    // Get application's display name out of the info.plist file:
    bool rc1 = osGetStringPropertyValueFromPListFile(infoPlistFilePath, "CFBundleDisplayName", displayName);

    if (rc1 && !displayName.isEmpty())
    {
        retVal = true;
    }
    else
    {
        // We didn't manage to get the display name out of the info.plist file. Use the executable name instead:
        bool rc2 = osGetStringPropertyValueFromPListFile(infoPlistFilePath, "CFBundleExecutable", displayName);

        if (rc2 && !displayName.isEmpty())
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdSelectIPhoneApplicationDialog::onApplicationsListFocused
// Description: Is called when an application's list item is focused.
// Author:      Yaki Tebeka
// Date:        26/5/2009
// ---------------------------------------------------------------------------
void gdSelectIPhoneApplicationDialog::onApplicationsListFocused(wxListEvent& eve)
{
    // Get the the focused item:
    int focusedItemId = eve.GetIndex();

    // Display the application's full path as a tooltip:
    gdApplicationsListItemData* pFocusedItemData = (gdApplicationsListItemData*)(_pIPhoneApplicationsListCtrl->GetItemData(focusedItemId));

    if (pFocusedItemData != NULL)
    {
        // Set the tooltip to contain the selected application's full path, and allow pressing OK:
        _selectedApplicationPath = pFocusedItemData->_applicationBundlePath.asString();
        _selectedApplicationSDKVersion = pFocusedItemData->_applicationSDKVersionAsString;
        _pOKButton->Enable();
    }
    else // pFocusedItemData == NULL
    {
        // No item data means this is one of the SDK headers, and disallow pressing OK:
        _selectedApplicationPath.makeEmpty();
        _selectedApplicationSDKVersion.makeEmpty();
        _pOKButton->Disable();
    }

    // Set the focus on the list control:
    _pIPhoneApplicationsListCtrl->SetFocus();
}


// ---------------------------------------------------------------------------
// Name:        gdSelectIPhoneApplicationDialog::onApplicationsListActivated
// Description: Is called when an application list item is activated.
// Author:      Yaki Tebeka
// Date:        26/5/2009
// ---------------------------------------------------------------------------
void gdSelectIPhoneApplicationDialog::onApplicationsListActivated(wxListEvent& eve)
{
    // If we can close with an "OK" selection, do it:
    if (_pOKButton->IsEnabled())
    {
        EndModal(wxID_OK);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdSelectIPhoneApplicationDialog::onManualSelectionButton
// Description: Called when the user click the manual selection button.
// Author:      Uri Shomroni
// Date:        6/5/2010
// ---------------------------------------------------------------------------
void gdSelectIPhoneApplicationDialog::onManualSelectionButton(wxCommandEvent& eve)
{
    // Display a file dialog to the user:
    osFilePath defaultDirectory(osFilePath::OS_USER_DOCUMENTS);
    afWXFileDialog selectFileDlg(this, GD_STR_SelectIPhoneAppDialogManualSelectionDialogTitle, defaultDirectory.asString().asCharArray(), AF_STR_Empty, GD_STR_macProgramFilesDetails, wxFD_OPEN);
    int userChoice = selectFileDlg.showModal();

    // If the user chose "Ok":
    if (userChoice == wxID_OK)
    {
        // Copy the selected path:
        _selectedApplicationPath = selectFileDlg.GetPath().c_str();

        // Set the default SDK version:
        _selectedApplicationSDKVersion = GD_STR_DebugSettingsiPhoneSimulatorDefaultSDKVersionAsString;

        // Close this (parent) dialog:
        EndModal(wxID_OK);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdSelectIPhoneApplicationDialog::applicationsListToolTipCallback
// Description:
//   Is called when determining what the tooltip for an application list item.
//   Sets the tooltip to contain the application's full path
// Author:      Yaki Tebeka
// Date:        26/5/2009
// ---------------------------------------------------------------------------
void gdSelectIPhoneApplicationDialog::applicationsListToolTipCallback(acWXListCtrl* pListCtrl, long item, gtString& toolTipText)
{
    toolTipText.makeEmpty();

    // Get the focused item:
    gdApplicationsListItemData* pFocusedItemData = (gdApplicationsListItemData*)(pListCtrl->GetItemData(item));

    if (pFocusedItemData != NULL)
    {
        // Set the tooltip to contain the selected application's full path:
        toolTipText = pFocusedItemData->_applicationBundlePath.asString();
    }
    else
    {
        // This is a header item, set the value as the string value:
        toolTipText = pListCtrl->GetItemText(item).c_str();
    }
}
