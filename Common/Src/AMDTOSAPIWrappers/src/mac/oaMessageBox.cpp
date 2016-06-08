//------------------------------ oaMessageBox.cpp ------------------------------

// Mac OS X:
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFString.h>

#ifdef _GR_IPHONE_BUILD

#else
    #include <Carbon/Carbon.h>
    #include <CoreFoundation/CFUserNotification.h>
#endif

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osMessageBox.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>


#ifdef _GR_IPHONE_BUILD
// Uri, 29/6/09: The iPhone implementation of this function is in the parallel .mm file.
#else
// ---------------------------------------------------------------------------
// Name:        osMessageBox::display
// Description:
//   Displays the message box.
//   The message box will be displayed in a "Modal" way (blocks the application
//   GUI until the user close the message box).
// Author:      Yaki Tebeka
// Date:        6/10/2004
// ---------------------------------------------------------------------------
void oaMessageBoxDisplayCB(const gtString& title, const gtString& message, osMessageBox::osMessageBoxIcon icon, /*oaWindowHandle*/ void* hParentWindow)
{
    GT_UNREFERENCED_PARAMETER(hParentWindow);

    // Convert the title and message CFStringRef
    CFStringRef titleStrRef = CFStringCreateWithCString(NULL, title.asCharArray(), 0);
    CFStringRef messageStrRef = CFStringCreateWithCString(NULL, message.asCharArray(), 0);

    // Create the flags for the message box (initialize by icon type):
    CFOptionFlags flags = 0;

    switch (icon)
    {
        case osMessageBox::OS_DISPLAYED_INFO_ICON:
        {
            flags = kCFUserNotificationNoteAlertLevel;
        }
        break;

        case osMessageBox::OS_QUESTION_MARK_ICON:
        {
            flags = kCFUserNotificationNoteAlertLevel;
        }
        break;

        case osMessageBox::OS_STOP_SIGN_ICON:
        {
            flags = kCFUserNotificationStopAlertLevel;
        }
        break;

        case osMessageBox::OS_EXCLAMATION_POINT_ICON:
        {
            flags = kCFUserNotificationCautionAlertLevel;
        }
        break;

        default:
            break;
    }

    // Call the notification message box for MAC OS:
    CFUserNotificationDisplayAlert(0, flags, NULL, NULL, NULL, titleStrRef, messageStrRef, CFSTR(OS_STR_OK), NULL, NULL, NULL);

    // Release the string:
    CFRelease(titleStrRef);
    CFRelease(messageStrRef);
}
#endif


