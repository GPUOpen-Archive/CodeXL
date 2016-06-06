//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaMessageBox.cpp
///
//=====================================================================

//------------------------------ oaMessageBox.cpp ------------------------------

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaMessageBox.h>

// Forward declarations:
unsigned int oaMessageBoxIconToOSStyle(osMessageBox::osMessageBoxIcon icon);

// ---------------------------------------------------------------------------
// Name:        osMessageBox::display
// Description:
//   Displays the message box.
//   The message box will be displayed in a "Modal" way (blocks the application
//   GUI until the user close the message box).
// Author:      AMD Developer Tools Team
// Date:        6/10/2004
// ---------------------------------------------------------------------------
void oaMessageBoxDisplayCB(const gtString& title, const gtString& message, osMessageBox::osMessageBoxIcon icon, /*oaWindowHandle*/ void* hParentWindow)
{
    // The message box will have a single "Ok" button:
    UINT okButtonStyle = MB_OK;

    // Translate the icon to Win32 style:
    unsigned int iconStyle = oaMessageBoxIconToOSStyle(icon);

    // Combine the styles into a styles mask:
    UINT messageBoxWin32Style = okButtonStyle | iconStyle;

    // Display the message box:
    MessageBox((oaWindowHandle)hParentWindow, message.asCharArray(), title.asCharArray(), messageBoxWin32Style);
}

// ---------------------------------------------------------------------------
// Name:        osMessageBox::iconToOSStyle
// Description: Translated osMessageBoxIcon to OS message box style mask.
// Arguments:   icon - The input icon.
// Return Val:  unsigned int - The output OS style mask.
// Author:      AMD Developer Tools Team
// Date:        6/10/2004
// ---------------------------------------------------------------------------
unsigned int oaMessageBoxIconToOSStyle(osMessageBox::osMessageBoxIcon icon)
{
    // Translate from osDefaultOSIconType to Win32 message box styles:
    unsigned int retVal = 0;

    switch (icon)
    {
        case osMessageBox::OS_EXCLAMATION_POINT_ICON:
            retVal = MB_ICONEXCLAMATION;
            break;

        case osMessageBox::OS_DISPLAYED_INFO_ICON:
            retVal = MB_ICONINFORMATION;
            break;

        case osMessageBox::OS_QUESTION_MARK_ICON:
            retVal = MB_ICONQUESTION;
            break;

        case osMessageBox::OS_STOP_SIGN_ICON:
            retVal = MB_ICONSTOP;
            break;

        default:
            // Unknown icon type:
            GT_ASSERT(false);
    }

    return retVal;
}

