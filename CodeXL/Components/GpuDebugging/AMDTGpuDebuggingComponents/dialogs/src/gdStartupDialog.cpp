//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStartupDialog.cpp
///
//==================================================================================

//------------------------------ gdStartupDialog.cpp ------------------------------

// Warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

//Qt
#include <QtGui>

// For compilers that support pre-compilation, includes "wx/wx.h".


// wxWindows
#include "wx/statline.h"
#include "wx/imaglist.h"

// Infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osBundle.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osPipeExecutor.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>
#include <AMDTAPIClasses/Include/apFrameTerminators.h>
#include <AMDTProcessDebugger/Include/pdProcessDebugger.h>
#include <GRApiFunctions/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acWXMessageDialog.h>
#include <AMDTApplicationComponents/Include/acWXListCtrl.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/views/afImageBannerWindow.h>

// Local
#include <CodeXLAppCode/gdApplicationCommands.h>
#include <CodeXLAppCode/gdCommandIDs.h>
#include <CodeXLAppCode/gdAidFunctions.h>
#include <CodeXLAppCode/gdAppWindowsManger.h>
#include <CodeXLAppCode/gdCodeXLGlobalVariablesManager.h>
#include <CodeXLAppCode/gdStringConstants.h>
#include <CodeXLAppCode/dialogs/gdStartupDialog.h>
#include <CodeXLAppCode/dialogs/gdNewWorkspaceWizard.h>
#include <CodeXLAppCode/dialogs/gdPerformanceCountersDialog.h>
#include <CodeXLAppCode/views/gdStateVariablesView.h>
#include <CodeXLAppCode/commands/gdLoadWorkspaceCommand.h>
#include <CodeXLAppCode/commands/gdSaveWorkspaceCommand.h>

// Icons (GR)
#include <CodeXLAppCode/res/icons/OpenGL_and_OpenCL_ico_xpm.xpm>
#include <CodeXLAppCode/res/icons/OpenGL_and_OpenCL_ico_alt_xpm.xpm>
#include <CodeXLAppCode/res/icons/OpenGL_ico_xpm.xpm>
#include <CodeXLAppCode/res/icons/OpenGL_ico_alt_xpm.xpm>
#include <CodeXLAppCode/res/icons/OpenCL_ico_xpm.xpm>
#include <CodeXLAppCode/res/icons/OpenCL_ico_alt_xpm.xpm>
#include <CodeXLAppCode/res/icons/new_ico_xpm.xpm>
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    #include <CodeXLAppCode/res/icons/iPhone_small_ico_xpm.xpm>
    #include <CodeXLAppCode/res/icons/iPhone_small_ico_alt_xpm.xpm>
#else
    #include <CodeXLAppCode/res/icons/OpenGL_ES_ico_xpm.xpm>
    #include <CodeXLAppCode/res/icons/OpenGL_ES_ico_alt_xpm.xpm>
#endif

// The index of the last fixed (hard coded) item in the projects list:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) ||((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
    #ifdef OS_REMOVE_OPENGL_ES_SUPPORT
        // Starting with V5.8, OpenGL ES is no longer supported, so the OpenGL ES samples will no longer be packed
        #define GD_STARTUP_DIALOG_LAST_FIXED_ITEM 1
    #else // ndef OS_REMOVE_OPENGL_ES_SUPPORT
        #define GD_STARTUP_DIALOG_LAST_FIXED_ITEM 2
    #endif
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    // CodeXL Linux does not support OpenGL ES, so it doesn't have
    // an ES sample application
    #define GD_STARTUP_DIALOG_LAST_FIXED_ITEM 1
#else
    #error Unknown build configuration!
#endif

// Layout and sizing constants:
#define GD_STARTUP_DIALOG_LIST_CONTROL_SIZE wxSize(375, 225)

BEGIN_EVENT_TABLE(gdStartupDialog, acDialog)
    EVT_LIST_ITEM_FOCUSED(ID_STARTUP_LISTCTRL, gdStartupDialog::onStartupListFocused)
    EVT_LIST_ITEM_SELECTED(ID_STARTUP_LISTCTRL, gdStartupDialog::onStartupListFocused)
    EVT_LIST_ITEM_ACTIVATED(ID_STARTUP_LISTCTRL, gdStartupDialog::onStartupListActivated)
    EVT_LIST_ITEM_RIGHT_CLICK(ID_STARTUP_LISTCTRL, gdStartupDialog::onListRightClick)

    EVT_LIST_KEY_DOWN(ID_STARTUP_LISTCTRL, gdStartupDialog::onStartupListKeyDown)
    EVT_MENU(ID_STARTUP_DELETE_WORKSPACE, gdStartupDialog::onDeleteItem)
END_EVENT_TABLE()


// ---------------------------------------------------------------------------
// Name:        gdStartupDialog::gdStartupDialog
// Description: Definition of the Help About Dialog.
// Author:      Avi Shapira
// Date:        9/7/2004
// ---------------------------------------------------------------------------
gdStartupDialog::gdStartupDialog(wxWindow* parent)
    : acDialog(parent, ID_STARTUP_DIALOG, _(GD_STR_StartupTitle), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
      _newProjectImageIndex(-1), _glclSampleImageIndex(-1), _glclProjectImageIndex(-1)
{
    // Get the new workspace file name:
    gdCodeXLGlobalVariablesManager& globalVarsManager = gdCodeXLGlobalVariablesManager::instance();

    // Get the CodeXL project type title as string:
    gtString title = globalVarsManager.CodeXLProjectTitleAsString();
    // Add the specific dialog caption
    title.append(GD_STR_StartupTitle);

    // Set the title:
    this->SetTitle(title.asCharArray());

    //Add the Icon to the dialog
    gdLoadCodeXLTitleBarIcon(*this);

    // StaticText for the description of the dialog
    _pStartupDialogDescription = new wxStaticText;
    _pStartupDialogDescription->Create(this, wxID_ANY, GD_STR_Empty, wxDefaultPosition, wxDefaultSize, wxTE_RICH | wxTE_MULTILINE);
    _pStartupDialogDescription->SetLabel(GD_STR_StartupDescription);

    _pStartupListCtrl = new acWXListCtrl(this, ID_STARTUP_LISTCTRL, wxDefaultPosition, GD_STARTUP_DIALOG_LIST_CONTROL_SIZE, wxLC_REPORT | wxSUNKEN_BORDER | wxLC_NO_HEADER | wxLC_SINGLE_SEL);
    _pStartupListCtrl->setToolTipCallBack(&listToolTipCallback);

    // Create the list icons:
    wxIcon newProjectIcon(new_ico_xpm);
    wxIcon openGLAndOpenCLIcon(OpenGL_and_OpenCL_ico_xpm);
    wxIcon openGLAndOpenCLIconAlt(OpenGL_and_OpenCL_ico_alt_xpm);
    wxIcon openGLIcon(OpenGL_ico_xpm);
    wxIcon openGLIconAlt(OpenGL_ico_alt_xpm);
    wxIcon openCLIcon(OpenCL_ico_xpm);
    wxIcon openCLIconAlt(OpenCL_ico_alt_xpm);

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    wxIcon openGLESIcon(iPhone_small_ico_xpm);
    wxIcon openGLESIconAlt(iPhone_small_ico_alt_xpm);
#else
    wxIcon openGLESIcon(OpenGL_ES_ico_xpm);
    wxIcon openGLESIconAlt(OpenGL_ES_ico_alt_xpm);
#endif

    _pListImageList = new wxImageList(16, 16, true);
    _newProjectImageIndex = _pListImageList->Add(newProjectIcon);
    _glclSampleImageIndex = _pListImageList->Add(openGLAndOpenCLIcon);
    _glclProjectImageIndex = _pListImageList->Add(openGLAndOpenCLIconAlt);

    _pStartupListCtrl->SetImageList(_pListImageList, wxIMAGE_LIST_SMALL);

    // layout components
    //  _pMainSizer->Add(_pStartupListCtrl);

    _pSizer = new wxBoxSizer(wxVERTICAL);

    _pSizer->Add(_pStartupDialogDescription, 0, wxALL | wxGROW, 10);
    _pSizer->Add(_pStartupListCtrl, 1, wxALL | wxGROW, 10);

    wxSize sizerSize = _pSizer->GetMinSize();

    // Calculate the banner width:
    int bannerWidth = sizerSize.x;
    wxWindow* pCodeXLImageBanner = new gdImageBannerWindow(this, bannerWidth);
    GT_ASSERT_ALLOCATION(pCodeXLImageBanner);

    // Create the dialog buttons:
    _pOKButton = new wxButton(pCodeXLImageBanner, wxID_OK, GD_STR_OK_Button);
    _pCancelButton = new wxButton(pCodeXLImageBanner, wxID_CANCEL, GD_STR_Cancel_Button);
    GT_ASSERT_ALLOCATION(_pOKButton);
    GT_ASSERT_ALLOCATION(_pCancelButton);

    // Calculate the buttons' locations and set them:
    int buttonWidth = _pOKButton->GetSize().GetX();
    int buttonPosY = GD_IMAGE_BANNER_WINDOW_TOP_BUTTON_MARGIN; //should be (pCodeXLImageBanner->GetSize().GetY() - _pOKButton->GetSize().GetY()) / 2
    wxPoint okButtonPos = wxPoint(sizerSize.x - buttonWidth * 2 - GD_IMAGE_BANNER_WINDOW_BUTTON_SPACING - GD_IMAGE_BANNER_WINDOW_RIGHT_BUTTON_MARGIN, buttonPosY);
    wxPoint cancelButtonPos = wxPoint(sizerSize.x - buttonWidth - GD_IMAGE_BANNER_WINDOW_RIGHT_BUTTON_MARGIN, buttonPosY);
    _pCancelButton->SetPosition(cancelButtonPos);
    _pOKButton->SetPosition(okButtonPos);

    // TO_DO: Make the TAB key go from the list to the buttons
    // TO_DO: Set the "OK" button as the default selection
    _pOKButton->SetDefault();
    _pOKButton->SetFocus();

    // Get rid of the white border around the buttons:
    wxColour buttonForegroundColor = _pOKButton->GetForegroundColour();
    _pOKButton->SetForegroundColour(buttonForegroundColor);
    _pCancelButton->SetForegroundColour(buttonForegroundColor);

    _pSizer->Add(pCodeXLImageBanner, 0, wxALL | wxALIGN_LEFT, 0);

    bool rc = setDialogValues();
    GT_ASSERT(rc);

    // activate
    SetSizer(_pSizer);
    SetAutoLayout(true);
    _pSizer->SetSizeHints(this);
    _pSizer->Fit(this);
    Centre(wxBOTH | wxCENTRE_ON_SCREEN);
}


// ---------------------------------------------------------------------------
// Name:        gdStartupDialog::~gdStartupDialog
// Description: Destructor
// Author:      Avi Shapira
// Date:        17/5/2006
// ---------------------------------------------------------------------------
gdStartupDialog::~gdStartupDialog()
{
    int listSize = _pStartupListCtrl->GetItemCount();

    // delete the item data in the list
    for (int i = 0; i < listSize; i++)
    {
        delete(gdStartupDialogListItemData*)(_pStartupListCtrl->GetItemData(i));
    }

    // Delete the image list:
    if (_pListImageList != NULL)
    {
        delete _pListImageList;
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStartupDialog::ShowModal
// Description: Overrides the wxDialog ShowModal
// Return Val:  int
// Author:      Avi Shapira
// Date:        13/7/2004
// ---------------------------------------------------------------------------
int gdStartupDialog::ShowModal()
{
    int retVal = wxID_CANCEL;

    // While we need to display the startup dialog:
    bool shouldDisplayStartupDialog = true;

    while (shouldDisplayStartupDialog)
    {
        // Display the startup dialog:
        retVal = wxDialog::ShowModal();
        shouldDisplayStartupDialog = false;

        // Add the dialog chosen breakpoints to the application
        if (retVal == wxID_OK)
        {
            if (_fileFullPath == GD_STR_StartupNewWorkspaceData)
            {
                // Load the New workspace wizard
                gdNewWorkspaceWizard* pWizard = new gdNewWorkspaceWizard(this);

                if (pWizard)
                {
                    // Launching the Wizard
                    bool finishedOK = pWizard->launchWizard();

                    if (finishedOK)
                    {
                        // Get the current process creation data from the gdCodeXLGlobalVariablesManager:
                        gdCodeXLGlobalVariablesManager& theStateManager = gdCodeXLGlobalVariablesManager::instance();

                        // Get the new workspace file path:
                        const osFilePath& workspaceFilePath = theStateManager.currentWorkspaceFilePath();

                        // Save the workspace:
                        gdSaveWorkspaceCommand saveWorkspaceCmd(workspaceFilePath);
                        saveWorkspaceCmd.execute();
                    }

                    pWizard->Destroy();
                    delete pWizard;
                }
            }
            else if (_fileFullPath == GD_STR_StartupTeapotExampleData)
            {
                // Load the GRTeapot Example
                retVal = setTeapotExampleSettings();
            }
            else if (_fileFullPath == GD_STR_StartupTeapotESExampleData)
            {
                // Load the GRTeapotES Example
                retVal = setTeapotESExampleSettings();
            }
            else if (_fileFullPath == GD_STR_StartupiPhoneExampleData)
            {
                // Load the GRemedyiPhoneExample Example
                retVal = setiPhoneExampleSettings();
            }
            else
            {
                // Load the selected workspace
                gdLoadWorkspaceCommand loadWorkspaceCmd(_fileFullPath.asCharArray());
                retVal = loadWorkspaceCmd.execute();

                if (!retVal)
                {
                    // If we failed to load the workspace - mark that we need to display the startup dialog again:
                    shouldDisplayStartupDialog = true;
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdStartupDialog::setDialogValues
// Description: Set the dialog values
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        9/7/2004
// ---------------------------------------------------------------------------
bool gdStartupDialog::setDialogValues()
{
    bool retVal = true;
    int currentItem = 0;
    long newItemId = 0;
    bool rc = true;

    // Create the "Startup dialog" column:
    gtString columnText = GD_STR_CallsHistoryTitle;

    wxListItem listItem;
    listItem.m_mask = wxLIST_MASK_TEXT | wxLIST_MASK_IMAGE;
    listItem.m_text = columnText.asCharArray();
    listItem.m_image = -1;
    listItem.m_format = wxLIST_FORMAT_LEFT;
    _pStartupListCtrl->InsertColumn(0, listItem);

    // Get the the User AppData directory
    osFilePath workspacePath(osFilePath::OS_USER_APPLICATION_DATA);
    workspacePath.appendSubDirectory(GD_STR_CodeXLAppDataDirectory);

    // Set the first item in the list to be create new workspace
    newItemId = _pStartupListCtrl->InsertItem(currentItem, GD_STR_StartupNewWorkspaceData);

    // Add the user data
    gdStartupDialogListItemData* pNewWizardItemData = new gdStartupDialogListItemData;
    pNewWizardItemData->_fullPath = GD_STR_StartupNewWorkspaceData;
    rc = _pStartupListCtrl->SetItemData(newItemId, (long)pNewWizardItemData);

    currentItem++;

    // Set the second item in the list to be GRteapot example
    newItemId = _pStartupListCtrl->InsertItem(currentItem, GD_STR_StartupTeapotExampleData);

    // Add the user data
    gdStartupDialogListItemData* pTeapotExampleItemData = new gdStartupDialogListItemData;
    pTeapotExampleItemData->_fullPath = GD_STR_StartupTeapotExampleData;
    rc = _pStartupListCtrl->SetItemData(newItemId, (long)pTeapotExampleItemData);

    currentItem++;

    // Starting with V5.8, OpenGL ES is no longer supported, so the OpenGL ES samples will no longer be packed
#ifndef OS_REMOVE_OPENGL_ES_SUPPORT
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // Set the third item in the list to be GRteapotES (OpenGL ES) example
    newItemId = _pStartupListCtrl->InsertItem(currentItem, GD_STR_StartupTeapotESExampleData);

    // Add the user data
    gdStartupDialogListItemData* pOpenGLESExampleItemData = new gdStartupDialogListItemData;
    pOpenGLESExampleItemData->_fullPath = GD_STR_StartupTeapotESExampleData;
    rc = _pStartupListCtrl->SetItemData(newItemId, (long)pOpenGLESExampleItemData);

    currentItem++;
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
#if AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
    // Set the third item in the list to be GRemedyiPhoneExample (OpenGL ES) example
    newItemId = _pStartupListCtrl->InsertItem(currentItem, GD_STR_StartupiPhoneExampleData);

    // Add the user data
    gdStartupDialogListItemData* pIPhoneExampleItemData = new gdStartupDialogListItemData;
    pIPhoneExampleItemData->_fullPath = GD_STR_StartupiPhoneExampleData;
    rc = _pStartupListCtrl->SetItemData(newItemId, (long)pIPhoneExampleItemData);
    currentItem++;
#elif AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
    // OpenGL ES is currently not supported on Linux.
    // When it will be supported, we will need to add an OpenGL ES example here.
#endif
#else
#error Unknown build configuration!
#endif
#endif // ndef OS_REMOVE_OPENGL_ES_SUPPORT

    // If the workspaces directory does not exist - create it:
    bool worksapceDirExists = true;
    osDirectory dir(workspacePath);

    if (!dir.exists())
    {
        worksapceDirExists = dir.create();
        GT_ASSERT(worksapceDirExists);
    }

    if (worksapceDirExists)
    {
        gtList<osFilePath> filePaths;
        bool rc1 = dir.getContainedFilePaths(gtString(L"*." GD_STR_workspaceFileExtension), osDirectory::SORT_BY_DATE_DESC, filePaths);

        if (rc1)
        {
            // Get the current workspace name:
            osFilePath workspaceFilePath;

            int listItems = filePaths.size();

            if (listItems > 0)
            {
                gtList <osFilePath>::iterator filesIterator = filePaths.begin();

                while (filesIterator != filePaths.end())
                {
                    osFilePath& currentFile = *filesIterator;
                    workspaceFilePath = currentFile;

                    gtString fileExtension;
                    workspaceFilePath.getFileExtension(fileExtension);

                    // The search function might bring us files that have extensions that start the same but aren't our type,
                    // e.g. "gdbvs" when looking for "gdb". So make sure the file name is correct:
                    if (fileExtension == GD_STR_workspaceFileExtension)
                    {
                        gtString fileFullName;
                        workspaceFilePath.getFileNameAndExtension(fileFullName);

                        // Add the item into the List
                        newItemId = _pStartupListCtrl->InsertItem(currentItem, fileFullName.asCharArray());

                        // Add the user data to the list (Full path info)
                        gdStartupDialogListItemData* pItemData = new gdStartupDialogListItemData;
                        pItemData->_fullPath = workspaceFilePath.asString();
                        rc = _pStartupListCtrl->SetItemData(newItemId, (long)pItemData);
                        retVal = retVal && rc;

                        currentItem++;
                    }

                    filesIterator++;
                }
            }
        }
    }

    // Get the startup list width
    int startupListWidth = _pStartupListCtrl->GetClientSize().GetWidth();
    int listNetoWidth = gdGetClientNetoWidth(startupListWidth);
    _pStartupListCtrl->SetColumnWidth(0, listNetoWidth);

    // Set the fixed items icons:
    _pStartupListCtrl->SetItemImage(0, _newProjectImageIndex);
    _pStartupListCtrl->SetItemImage(1, _glclSampleImageIndex);

    // Set the dynamic items icons:
    int listSize = _pStartupListCtrl->GetItemCount();

    for (int i = GD_STARTUP_DIALOG_LAST_FIXED_ITEM + 1; i < listSize; i++)
    {
        const gtString& projectFilePath = ((gdStartupDialogListItemData*)(_pStartupListCtrl->GetItemData(i)))->_fullPath;
        _pStartupListCtrl->SetItemImage(i, _glclProjectImageIndex);
    }

    // Set the default selection
    long defaultSelection = 1;

    long firstNonFixedItem = GD_STARTUP_DIALOG_LAST_FIXED_ITEM + 1;

    if (firstNonFixedItem < listSize)
    {
        defaultSelection = firstNonFixedItem;
    }

    _pStartupListCtrl->selectAndFocusItem(defaultSelection);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStartupDialog::onStartupListFocused
// Description: Store the selected item's full path and add tool tip to the focused list item.
// Author:      Avi Shapira
// Date:        13/7/2004
// ---------------------------------------------------------------------------
void gdStartupDialog::onStartupListFocused(wxListEvent& eve)
{
    int focusedItemId = eve.GetIndex();

    // Get the filepath of the current item (will also be displayed as a tooltip)
    listToolTipCallback(_pStartupListCtrl, focusedItemId, _fileFullPath);
    _pStartupListCtrl->SetFocus();
}

// ---------------------------------------------------------------------------
// Name:        gdStartupDialog::listToolTipCallback
// Description: Is called when determining what the tooltip for an item will be
// Author:      Uri Shomroni
// Date:        1/3/2009
// ---------------------------------------------------------------------------
void gdStartupDialog::listToolTipCallback(acWXListCtrl* pListCtrl, long item, gtString& toolTipText)
{
    toolTipText.makeEmpty();
    gdStartupDialogListItemData* pFocusedItemItem = (gdStartupDialogListItemData*)(pListCtrl->GetItemData(item));

    GT_IF_WITH_ASSERT(pFocusedItemItem != NULL)
    {
        toolTipText = pFocusedItemItem->_fullPath;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStartupDialog::onStartupListActivated
// Description: Select the item from the list
// Arguments:   wxListEvent &eve
// Author:      Avi Shapira
// Date:        13/7/2004
// ---------------------------------------------------------------------------
void gdStartupDialog::onStartupListActivated(wxListEvent& eve)
{
    EndModal(wxID_OK);
}


// ---------------------------------------------------------------------------
// Name:        gdStartupDialog::onStartupListKeyDown
// Description: Handle the pressed key
// Arguments:   wxListEvent &eve
// Author:      Avi Shapira
// Date:        11/4/2005
// ---------------------------------------------------------------------------
void gdStartupDialog::onStartupListKeyDown(wxListEvent& eve)
{
    int keyCode = eve.GetKeyCode();
    int itemIndex = eve.GetIndex();

    if (itemIndex != -1)
    {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

        // Delete or backspace were pressed
        if ((keyCode == WXK_DELETE) || (keyCode == WXK_BACK))
#else

        // Delete was pressed
        if (keyCode == WXK_DELETE)
#endif
        {
            if (itemIndex > GD_STARTUP_DIALOG_LAST_FIXED_ITEM)
            {
                // Delete the item
                bool rc = deleteItem(itemIndex);
                GT_ASSERT(rc);
            }
        }
    }
}



// ---------------------------------------------------------------------------
// Name:        gdStartupDialog::onListRightClick
// Description: Right click event for the list control - opens a context menu
//              for project deletion
// Arguments: wxListEvent &eve
// Return Val: void
// Author:      Sigal Algranaty
// Date:        12/3/2008
// ---------------------------------------------------------------------------
void gdStartupDialog::onListRightClick(wxListEvent& eve)
{
    wxPoint point = eve.GetPoint();
    wxPoint listPos = _pStartupListCtrl->GetPosition();
    point += listPos;

    // Pop context menu
    showContextMenu(point);
}


// ---------------------------------------------------------------------------
// Name:        gdStartupDialog::canSelectedBeDeleted
// Description:
// Return Val: bool  -  true if any of the selected projects can be deleted
//                      false - if none of the selected can be deleted
// Author:      Sigal Algranaty
// Date:        12/3/2008
// ---------------------------------------------------------------------------
bool gdStartupDialog::canSelectedBeDeleted()
{
    bool retVal = false;
    // Iterate the selected projects and delete each of them
    long item = -1;
    item = _pStartupListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    // Get all the selected Items
    while (item != -1)
    {
        if (item != -1)
        {
            if (item > GD_STARTUP_DIALOG_LAST_FIXED_ITEM)
            {
                retVal = true;
                break;
            }
        }

        // Get the next item
        item = _pStartupListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdStartupDialog::showContextMenu
// Description: Display a right click menu
// Arguments: const wxPoint& pos
// Return Val: void
// Author:      Sigal Algranaty
// Date:        12/3/2008
// ---------------------------------------------------------------------------
void gdStartupDialog::showContextMenu(const wxPoint& pos)
{
    wxMenu menu;
    bool enableDeletion = canSelectedBeDeleted();
    menu.Append(ID_STARTUP_DELETE_WORKSPACE, GD_STR_DeleteSelected);

    menu.Enable(ID_STARTUP_DELETE_WORKSPACE, enableDeletion);

    // Display context menu
    PopupMenu(&menu, pos.x, pos.y);
}




// ---------------------------------------------------------------------------
// Name:        gdStartupDialog::onDeleteItem
// Description: Event Handler for right click menu item "Delete"
// Arguments: wxCommandEvent &eve
// Return Val: void
// Author:      Sigal Algranaty
// Date:        12/3/2008
// ---------------------------------------------------------------------------
void gdStartupDialog::onDeleteItem(wxCommandEvent& eve)
{
    // Iterate the selected projects and delete each of them
    long item = -1;
    item = _pStartupListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    // Get all the selected Items
    while (item != -1)
    {
        if (item != -1)
        {
            bool rc = deleteItem(item);
            GT_ASSERT(rc);
        }

        // Get the next item
        item = _pStartupListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStartupDialog::deleteItem
// Description: Delete item according to index
// Arguments: int itemIndex
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        12/3/2008
// ---------------------------------------------------------------------------
bool gdStartupDialog::deleteItem(int itemIndex)
{
    bool retVal = true;
    gdStartupDialogListItemData* pFocusedItemItem = (gdStartupDialogListItemData*)(_pStartupListCtrl->GetItemData(itemIndex));


    osFilePath workspaceFullPath = pFocusedItemItem->_fullPath;
    gtString workspaceName;
    bool rc = workspaceFullPath.getFileName(workspaceName);
    GT_ASSERT(rc);

    gtString deleteMessage = GD_STR_ConfirmDeletion;
    deleteMessage.append(workspaceName);
    deleteMessage.append(L" workspace?");

    if (acWXMessageBox(deleteMessage.asCharArray(), GD_STR_Question, wxICON_QUESTION | wxYES_NO, this) == wxYES)
    {

        osDirectory fileDirectory;
        workspaceFullPath.getFileDirectory(fileDirectory);
        gtString workspaceFileName;
        gtString breakpointFileName;
        gtString workspaceFileExtension;
        workspaceFullPath.getFileName(workspaceFileName);
        workspaceFullPath.getFileExtension(workspaceFileExtension);
        workspaceFileName.append(osFilePath::osExtensionSeparator);
        workspaceFileName.append(workspaceFileExtension);

        workspaceFullPath.getFileName(breakpointFileName);
        breakpointFileName.append(osFilePath::osExtensionSeparator);
        breakpointFileName.append(GD_STR_breakpointFileExtension);

        // Delete the workspace file
        retVal = fileDirectory.deleteFile(workspaceFileName);
        // Delete the breakpoint file
        retVal = retVal && fileDirectory.deleteFile(breakpointFileName);

        if (retVal)
        {
            // Delete the item from the list
            _pStartupListCtrl->DeleteItem(itemIndex);
        }
        else
        {
            // Error message
            gtString errorMessage = L"Cannot delete ";
            errorMessage.append(workspaceFileName);
            acWXMessageBox(errorMessage.asCharArray(), GD_STR_Error, wxICON_ERROR | wxOK);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStartupDialog::setTeapotExampleSettings
// Description: Set the settings of the teapot example
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        1/8/2004
// ---------------------------------------------------------------------------
bool gdStartupDialog::setTeapotExampleSettings()
{
    bool retVal = false;

    osFilePath CodeXLExamplesDir;
    gtString debuggedAppWorkingDir, debuggedAppExe;

    // Get the gdCodeXLGlobalVariablesManager instance
    // in order to set the updated parameters from the Dialog
    gdCodeXLGlobalVariablesManager& theStateManager = gdCodeXLGlobalVariablesManager::instance();


    bool rc1 = gdGetApplicationRelatedFilePath(CodeXLExamplesDir, GD_APPLICATION_EXAMPLES_PATH);
    GT_IF_WITH_ASSERT(rc1)
    {
        debuggedAppWorkingDir = CodeXLExamplesDir.asString();
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        // Since the Mac Teapot is a bundle, it doesn't reside in a subdirectory
#else
        debuggedAppWorkingDir.append(osFilePath::osPathSeparator).append(GD_STR_CodeXLTeapotExampleDirName);
#endif
        debuggedAppExe = debuggedAppWorkingDir;
        debuggedAppExe.append(osFilePath::osPathSeparator);

        // Set the teapot application name according to the OS type:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        debuggedAppExe.append(L"GRTeaPot.exe");
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
        debuggedAppExe.append(L"GRTeapotApp-bin");
#elif AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
        debuggedAppExe.append(L"GRTeapot.app");
#else
#error Unknown Linux Variant!
#endif
#else
#error Error: Unknown build configuration!
#endif

        // Get the process creation data:
        apDebugProjectSettings processCreationData = theStateManager.currentProjectSettings();

        processCreationData.setExecutablePath(debuggedAppExe);
        processCreationData.setWorkDirectory(osFilePath(debuggedAppWorkingDir));
        processCreationData.setCommandLineArguments(GD_STR_Empty);

        // Set the log file directory to the temp directory by default
        // Get the the Temp directory
        osFilePath tempDirectory(osFilePath::OS_TEMP_DIRECTORY);

        // The log file Directory is set in the process creation data:
        // processCreationData.setLogFilesDirectoryPath(tempDirectory);

        // Swap Buffers will be the default frame terminators.
        processCreationData.setFrameTerminators(AP_DEFAULT_GL_FRAME_TERMINATOR);

        theStateManager.setCurrentProjectSettings(processCreationData);

        // Get the the User AppData directory
        osFilePath currentWorkspaceFilePath(osFilePath::OS_USER_APPLICATION_DATA);

        // Add the application directory to it
        currentWorkspaceFilePath.appendSubDirectory(GD_STR_CodeXLAppDataDirectory);

        currentWorkspaceFilePath.setFileName(L"GRTeaPot");
        currentWorkspaceFilePath.setFileExtension(GD_STR_workspaceFileExtension);

        theStateManager.setCurrentWorkspaceFilePath(currentWorkspaceFilePath);

        // Add the default state variables:
        bool rc2 = addDefaultStateVariables();
        GT_ASSERT(rc2);

        // Add the default performance counters:
        bool rc3 = addDefaultPerformanceCounters();
        GT_ASSERT(rc3);

        // rc3 is not included in function retVal since on some ocassions performance counters
        // cannot be added, and we don't want to fail the project load for that
        retVal = rc2;
    }


    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        gdStartupDialog::setTeapotESExampleSettings
// Description: Set the settings of the OpenGL ES teapot example
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        22/2/2006
// ---------------------------------------------------------------------------
bool gdStartupDialog::setTeapotESExampleSettings()
{
    bool retVal = false;

    // Starting with V5.8, OpenGL ES is no longer supported, so the OpenGL ES samples will no longer be packed
#ifndef OS_REMOVE_OPENGL_ES_SUPPORT

    // The wakebreaker example is only available on Windows:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    osFilePath CodeXLExamplesDir;
    gtString debuggedAppExe;
    gtString debuggedAppWorkingDir;

    // Get the gdCodeXLGlobalVariablesManager instance
    // in order to set the updated parameters from the Dialog
    gdCodeXLGlobalVariablesManager& theStateManager = gdCodeXLGlobalVariablesManager::instance();

    // Change the project type:
    bool projectFileChangedOk = theStateManager.setCodeXLProjectType(AP_OPENGL_ES_PROJECT, false);

    if (!projectFileChangedOk)
    {
        // If we was not able to change the project type:
        retVal = false;
    }
    else
    {
        bool rc1 = gdGetApplicationRelatedFilePath(CodeXLExamplesDir, GD_APPLICATION_EXAMPLES_PATH);
        GT_IF_WITH_ASSERT(rc1)
        {
            gtString buildConfiguration = L"Debug";

#if AMDT_BUILD_CONFIGURATION == AMDT_RELEASE_BUILD
            {
                buildConfiguration = L"Release";
            }
#endif //AMDT_BUILD_CONFIGURATION

            debuggedAppWorkingDir = CodeXLExamplesDir.asString();
            debuggedAppWorkingDir.append(osFilePath::osPathSeparator).append(L"WakeBreaker");
            debuggedAppWorkingDir.append(osFilePath::osPathSeparator).append(buildConfiguration);
            debuggedAppWorkingDir.append(osFilePath::osPathSeparator).append(L"bin");
            debuggedAppWorkingDir.append(osFilePath::osPathSeparator).append(L"win32");
            debuggedAppExe = debuggedAppWorkingDir;
            debuggedAppExe.append(osFilePath::osPathSeparator).append(L"WakeBreaker.exe");

            // Get the process creation data:
            apDebugProjectSettings processCreationData = theStateManager.currentProjectSettings();

            processCreationData.setExecutablePath(debuggedAppExe);
            processCreationData.setWorkDirectory(osFilePath(debuggedAppWorkingDir));
            processCreationData.setCommandLineArguments(GD_STR_Empty);

            // Set the log file directory to the temp directory by default
            // Get the the Temp directory
            osFilePath tempDirectory(osFilePath::OS_TEMP_DIRECTORY);

            // The log file Directory is set in the process creation data:
            // processCreationData.setLogFilesDirectoryPath(tempDirectory);

            // Swap Buffers will be the default frame terminators.
            processCreationData.setFrameTerminators(AP_DEFAULT_GL_FRAME_TERMINATOR);

            theStateManager.setCurrentProjectSettings(processCreationData);

            // Get the the User AppData directory
            osFilePath currentWorkspaceFilePath(osFilePath::OS_USER_APPLICATION_DATA);

            // Add the application directory to it
            currentWorkspaceFilePath.appendSubDirectory(GD_STR_CodeXLAppDataDirectory);

            currentWorkspaceFilePath.setFileName(L"WakeBreaker");
            currentWorkspaceFilePath.setFileExtension(GD_STR_workspaceFileExtension);

            theStateManager.setCurrentWorkspaceFilePath(currentWorkspaceFilePath);

            // Add the default state variables:
            bool rc2 = addDefaultStateVariables();
            GT_ASSERT(rc2);

            // Add the default performance counters:
            bool rc3 = addDefaultPerformanceCounters();
            GT_ASSERT(rc3);

            // rc3 is not included in function retVal since on some ocassions performance counters
            // cannot be added, and we don't want to fail the project load for that
            retVal = rc2;
        }
    }

#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#endif // ndef OS_REMOVE_OPENGL_ES_SUPPORT

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStartupDialog::setiPhoneExampleSettings
// Description: Sets the iPhone example's settings
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        31/5/2009
// ---------------------------------------------------------------------------
bool gdStartupDialog::setiPhoneExampleSettings()
{
    bool retVal = false;

    // Starting with V5.8, OpenGL ES is no longer supported, so the OpenGL ES samples will no longer be packed
#ifndef OS_REMOVE_OPENGL_ES_SUPPORT

    // The GRemedyiPhoneExample example is only available on Mac OS X:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Get the gdCodeXLGlobalVariablesManager instance
    // in order to set the updated parameters from the Dialog
    gdCodeXLGlobalVariablesManager& theStateManager = gdCodeXLGlobalVariablesManager::instance();

    // Change the project type and execution target:
    bool projectFileChangedOk = theStateManager.setCodeXLProjectType(AP_OPENGL_ES_PROJECT, false);

    if (projectFileChangedOk)
    {
        projectFileChangedOk = theStateManager.setCodeXLProjectExecutionTarget(AP_IPHONE_SIMULATOR_EXECUTION_TARGET);
    }

    if (!projectFileChangedOk)
    {
        // If we was not able to change the project type:
        retVal = false;
    }
    else
    {
        bool isExecutableOkay = false;

        // Construct the example file path:
        gtString debuggedAppExeAsString;
        gtString debuggedAppWorkingDir;

        // Calculate the compiled iPhone simulator applications directory:
        osFilePath simulatorAppDirectoryPath(osFilePath::OS_USER_DOCUMENTS);
        simulatorAppDirectoryPath.appendSubDirectory(GD_STR_CodeXLiPhoneApplicationsRootDirPath GD_STR_DebugSettingsiPhoneSimulatorDefaultSDKVersionAsString);
        simulatorAppDirectoryPath.appendSubDirectory(GD_STR_CodeXLiPhoneApplicationsDirApplicationsSubdirectoryName);
        debuggedAppWorkingDir = simulatorAppDirectoryPath.asString();
        debuggedAppExeAsString = debuggedAppWorkingDir;
        debuggedAppExeAsString.append(osFilePath::osPathSeparator).append(GD_STR_CodeXLiPhoneSampleAppDirPath).append(osFilePath::osPathSeparator).append(GD_STR_CodeXLiPhoneSampleAppName);

        osFilePath debuggedAppExe(debuggedAppExeAsString);

        // Find out if this app already exists and remove it if it does:
        osDirectory simulatorAppDirectory(simulatorAppDirectoryPath);
        GT_IF_WITH_ASSERT(simulatorAppDirectory.exists())
        {
            // Get the directory's sub directories:
            gtList<osFilePath> subDirectoriesPaths;
            bool rc1 = simulatorAppDirectory.getSubDirectoriesPaths(osDirectory::SORT_BY_DATE_DESC, subDirectoriesPaths);

            if (rc1)
            {
                // Iterate the sub directories:
                gtList <osFilePath>::iterator iter = subDirectoriesPaths.begin();
                gtList <osFilePath>::iterator endIter = subDirectoriesPaths.end();

                while (iter != endIter)
                {
                    // Make a mockup file path of where we'd expect to find the sample app if it were under this subdirectory:
                    gtString mockupPathAsString = (*iter).asString();
                    mockupPathAsString.append(osFilePath::osPathSeparator);
                    osFilePath mockupPath(mockupPathAsString);
                    mockupPath.appendSubDirectory(GD_STR_CodeXLiPhoneSampleAppName);

                    if (mockupPath.exists())
                    {
                        // Don't break the loop in case there is more than one copy:
                        osDirectory mockupPathDir(mockupPathAsString);
                        mockupPathDir.deleteRecursively();
                    }

                    iter++;
                }
            }
        }

        osFilePath CodeXLExamplesDir;
        bool rc1 = gdGetApplicationRelatedFilePath(CodeXLExamplesDir, GD_APPLICATION_EXAMPLES_PATH);
        GT_IF_WITH_ASSERT(rc1)
        {
            // Copy the application:
            gtString copyCommand = "cp -Rf ";
            gtString srcPath = CodeXLExamplesDir.asString();
            srcPath.append(osFilePath::osPathSeparator).append(GD_STR_CodeXLiPhoneSampleAppDirPath).replace(GD_STR_Space, "\\ ");
            gtString dstPath = simulatorAppDirectoryPath.asString();
            dstPath.append(osFilePath::osPathSeparator).replace(GD_STR_Space, "\\ ");
            copyCommand.append(srcPath).append(' ').append(dstPath);
            osPipeExecutor pipeExec;
            gtString output;
            pipeExec.executeCommand(copyCommand, output);

            // Copy the sandbox file:
            gtString sandboxFileName = GD_STR_CodeXLiPhoneSampleAppDirPath;
            sandboxFileName.append(osFilePath::osExtensionSeparator).append(GD_STR_CodeXLiPhoneSampleSandboxFileExt);
            copyCommand.replace(GD_STR_CodeXLiPhoneSampleAppDirPath, sandboxFileName);
            pipeExec.executeCommand(copyCommand, output);

            // Only continue if we copied the example successfully:
            isExecutableOkay = debuggedAppExe.exists();
        }

        GT_IF_WITH_ASSERT(isExecutableOkay)
        {
            // Get the process creation data:
            apDebugProjectSettings processCreationData = theStateManager.currentProjectSettings();

            processCreationData.setExecutablePath(debuggedAppExe);
            processCreationData.setWorkDirectory(osFilePath(debuggedAppWorkingDir));
            processCreationData.setCommandLineArguments(GD_STR_Empty);

            // Set the log file directory to the temp directory by default
            // Get the the Temp directory
            osFilePath tempDirectory(osFilePath::OS_TEMP_DIRECTORY);

            // The log file Directory is set in the process creation data:
            // processCreationData.setLogFilesDirectoryPath(tempDirectory);

            // Swap Buffers will be the default frame terminators.
            processCreationData.setFrameTerminators(AP_DEFAULT_GL_FRAME_TERMINATOR);

            theStateManager.setCurrentProjectSettings(processCreationData);

            // Set the default iPhone SDK Path:
            osSetOpenGLESFrameworkPath(GD_STR_DebugSettingsiPhoneSDKDefaultValue GD_STR_DebugSettingsiPhoneSDKOpenGLESRelativePath);

            // Get the project path:
            osFilePath currentWorkspaceFilePath;
            bool rcPth = gdDefaultProjectFilePath(debuggedAppExe, AP_OPENGL_ES_PROJECT, AP_IPHONE_SIMULATOR_EXECUTION_TARGET, currentWorkspaceFilePath);

            if (!rcPth)
            {
                // Fallback to the default project path:
                // Get the the User AppData directory
                currentWorkspaceFilePath.setPath(osFilePath::OS_USER_APPLICATION_DATA);

                // Add the application directory to it
                currentWorkspaceFilePath.appendSubDirectory(GD_STR_CodeXLAppDataDirectory);

                currentWorkspaceFilePath.setFileName("GRemedyiPhoneExample");
                currentWorkspaceFilePath.setFileExtension(GD_STR_workspaceFileExtension);
            }

            theStateManager.setCurrentWorkspaceFilePath(currentWorkspaceFilePath);

            // Add the default state variables:
            bool rc2 = addDefaultStateVariables();
            GT_ASSERT(rc2);

            // Add the default performance counters:
            bool rc3 = addDefaultPerformanceCounters();
            GT_ASSERT(rc3);

            // rc3 is not included in function retVal since on some occasions performance counters
            // cannot be added, and we don't want to fail the project load for that
            retVal = rc2;
        }
    }

#endif // ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
#endif // ndef OS_REMOVE_OPENGL_ES_SUPPORT

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStartupDialog::addDefaultStateVariables
// Description: Adds the default state variables to the state variables watch view.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        18/6/2007
// ---------------------------------------------------------------------------
bool gdStartupDialog::addDefaultStateVariables()
{
    bool retVal = false;

    // Get the application command instance:
    gdApplicationCommands* pApplicationCommands = gdApplicationCommands::gdInstance();
    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
    {
        // Get the state variables view:
        gdStateVariablesView* pStateVariableView = pApplicationCommands->stateVariablesView();
        GT_IF_WITH_ASSERT(pStateVariableView != NULL)
        {
            // Add the default state variables:
            gtString variableName;
            variableName = L"GL_MODELVIEW_MATRIX";
            pStateVariableView->AddStateVariable(variableName);

            variableName = L"GL_PROJECTION_MATRIX";
            pStateVariableView->AddStateVariable(variableName);

            variableName = L"GL_VIEWPORT";
            pStateVariableView->AddStateVariable(variableName);
        }

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStartupDialog::addDefaultPerformanceCounters
// Description: Adds the default performance counters to the performance
//              counters dialog.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        18/6/2007
// ---------------------------------------------------------------------------
bool gdStartupDialog::addDefaultPerformanceCounters()
{
    bool retVal = false;

    // Get the performance counters dialog:
    gdAppWindowsManger& theAppWindowsMgr = gdAppWindowsManger::instance();
    gdPerformanceCountersDialog* pPerformanceCountersDialog = (gdPerformanceCountersDialog*)(theAppWindowsMgr.getAppWindow(ID_PERFORMANCE_DIALOG_COUNTERS_DIALOG));
    GT_IF_WITH_ASSERT(pPerformanceCountersDialog != NULL)
    {
        // Remove all previous counters:
        pPerformanceCountersDialog->removeAllCounters();

        // Add the default performance counters:
        retVal = pPerformanceCountersDialog->setDialogDefaultActiveCounters();
    }

    return retVal;
}


