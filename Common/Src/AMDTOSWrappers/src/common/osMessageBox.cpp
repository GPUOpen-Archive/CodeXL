//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osMessageBox.cpp
///
//=====================================================================

//------------------------------ osMessageBox.cpp ------------------------------

// STL:
#include <iostream>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTOSWrappers/Include/osMessageBox.h>

// Static members initialization:
osMessageBox::osMessageBoxCB osMessageBox::ms_cb = &osMessageBox::defaultMessageBoxDisplayCB;


// ---------------------------------------------------------------------------
// Name:        osMessageBox::osMessageBox
// Description: Constructor
// Arguments:   message - The string to be displayed in the message box.
//              icon - The icon to be displayed in the message box.
//              hParentWindow - The message box parent window.
// Author:      AMD Developer Tools Team
// Date:        6/10/2004
// ---------------------------------------------------------------------------
osMessageBox::osMessageBox(const gtString& title, const gtString& message, osMessageBoxIcon icon, void* /*oaWindowHandle*/ hParentWindow)
    : _title(title), _message(message), _icon(icon), _hParentWindow(hParentWindow)
{
}


// ---------------------------------------------------------------------------
// Name:        osMessageBox::display
// Description:
//   Displays the message box.
//   The message box will be displayed in a "Modal" way (blocks the application
//   GUI until the user close the message box).
// Author:      AMD Developer Tools Team
// Date:        6/10/2004
// ---------------------------------------------------------------------------
void osMessageBox::display()
{
    GT_IF_WITH_ASSERT(NULL != ms_cb)
    {
        ms_cb(_title, _message, _icon, _hParentWindow);
    }
}

// ---------------------------------------------------------------------------
// Name:        osMessageBox::defaultMessageBoxDisplayCB
// Description:
//   Default message box callback. Since we cannot assume that this is a
//   configuration with a display, we print to the console.
// Author:      AMD Developer Tools Team
// Date:        4/6/2015
// ---------------------------------------------------------------------------
void osMessageBox::defaultMessageBoxDisplayCB(const gtString& title, const gtString& message, osMessageBoxIcon icon, /*oaWindowHandle*/ void* hParentWindow)
{
    GT_UNREFERENCED_PARAMETER(hParentWindow);

    // Create the title string:
    gtString titleLine;
    titleLine.append(L"| ");

    switch (icon)
    {
        // *INDENT-OFF*
        case OS_EXCLAMATION_POINT_ICON: titleLine.append(L"/!\\");   break;
        case OS_DISPLAYED_INFO_ICON:    titleLine.append(L"(i)");    break;
        case OS_QUESTION_MARK_ICON:     titleLine.append(L"(?)");    break;
        case OS_STOP_SIGN_ICON:         titleLine.append(L"(x)");    break;
        // *INDENT-ON*
    }

    titleLine.append(L" | ").append(title);

    // Count space for an additional " |"
    int titleLineLength = titleLine.length() + 2;

    // Count the max line length in the message:
    int currentLoc = 0;
    int maxLen = 0;
    int msgLen = message.length();

    while (currentLoc < msgLen)
    {
        int prevLoc = currentLoc;
        currentLoc = message.find('\n', currentLoc + 1);

        if (0 > currentLoc)
        {
            currentLoc = msgLen;
        }

        int currentLineLen = currentLoc - prevLoc;

        if (maxLen < currentLineLen)
        {
            maxLen = currentLineLen;
        }
    }

    // Widen the title as needed:
    while (titleLineLength < maxLen)
    {
        titleLine.append(' ');
        ++titleLineLength;
    }

    titleLine.append(L" |");

    // Create the separator line:
    gtString separatorLine = '+';

    for (int i = 0; (titleLineLength - 2) > i; ++i)
    {
        separatorLine.append('-');
    }

    separatorLine.append('+');

    // Create the complete message:
    gtString completeMessage = separatorLine;
    completeMessage.append(titleLine).append(separatorLine).append(message).append(separatorLine).append(separatorLine);

    // Write it to the console:
    std::cout << completeMessage.asCharArray();
}

// ---------------------------------------------------------------------------
// Name:        osMessageBox::setMessageBoxDisplayCB
// Description:
//   Sets the message box display callback.
// Author:      AMD Developer Tools Team
// Date:        4/6/2015
// ---------------------------------------------------------------------------
void osMessageBox::setMessageBoxDisplayCB(osMessageBoxCB pCB)
{
    ms_cb = pCB;
}
