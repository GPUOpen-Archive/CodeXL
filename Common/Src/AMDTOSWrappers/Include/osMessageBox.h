//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osMessageBox.h
///
//=====================================================================

//------------------------------ osMessageBox.h ------------------------------

#ifndef __OSMESSAGEBOX
#define __OSMESSAGEBOX

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>


// ----------------------------------------------------------------------------------
// Class Name:           OS_API osMessageBox
// General Description: Implement a simple message box.
// Author:      AMD Developer Tools Team
// Creation Date:        6/10/2004
// ----------------------------------------------------------------------------------
class OS_API osMessageBox
{
public:

    // Describes default OS Message box icons:
    enum osMessageBoxIcon
    {
        OS_EXCLAMATION_POINT_ICON, // Exclamation point icon (!)
        OS_DISPLAYED_INFO_ICON,    // Information icon (i)
        OS_QUESTION_MARK_ICON ,    // Question mark icon (?)
        OS_STOP_SIGN_ICON          // Stop sign icon (usually displayed on errors).
    };

    osMessageBox(const gtString& title, const gtString& message, osMessageBoxIcon icon = OS_DISPLAYED_INFO_ICON, /*oaWindowHandle*/ void* hParentWindow = 0);
    void display();

    typedef void(*osMessageBoxCB)(const gtString& title, const gtString& message, osMessageBoxIcon icon, /*oaWindowHandle*/ void* hParentWindow);
    static void defaultMessageBoxDisplayCB(const gtString& title, const gtString& message, osMessageBoxIcon icon, /*oaWindowHandle*/ void* hParentWindow);
    static void setMessageBoxDisplayCB(osMessageBoxCB pCB);

private:
    // Do not allow the use of my default constructor:
    osMessageBox();

private:
    // The message box title:
    gtString _title;

    // The message that will be displayed in the message box:
    gtString _message;

    // The icon that will be displayed in the message box:
    osMessageBoxIcon _icon;

    // The message box parent window:
    void* _hParentWindow;

    // The message box display callback:
    static osMessageBoxCB ms_cb;
};


#endif  // __OSMESSAGEBOX
