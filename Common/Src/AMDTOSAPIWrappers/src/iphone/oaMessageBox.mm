//------------------------------ oaMessageBox.mm ------------------------------

// iPhone UIKit:
#import <UIKit/UIKit.h>

// Local:
#import <GROSWrappers/osMessageBox.h>
#import <inc/common/osStringConstants.h>


// ---------------------------------------------------------------------------
// Name:        osMessageBox::display
// Description: Displays the message box. The message box will be displayed in
//				 a non-modal way (doesn't block the application GUI until the user close the message box).
// Author:      Uri Shomroni
// Date:        6/10/2004
// ---------------------------------------------------------------------------
void oaMessageBoxDisplayCB(const gtString& title, const gtString& message, osMessageBox::osMessageBoxIcon icon, /*oaWindowHandle*/ void* hParentWindow)
{
	// This function might be called from a non-main thread, so use an autorelease pool
	// to avoid memory leak warnings (see "Autorelease Pools" in 
	// http://developer.apple.com/mac/library/documentation/Cocoa/Conceptual/MemoryMgmt/MemoryMgmt.html):
	NSAutoreleasePool* pPool = [[NSAutoreleasePool alloc] init];

	// Convert the title and message NSString format:
	NSString* titleAsNSString = [[NSString alloc] initWithCString:title.asCharArray() encoding:NSMacOSRomanStringEncoding];
	NSString* messageAsNSString = [[NSString alloc] initWithCString:message.asCharArray() encoding:NSMacOSRomanStringEncoding];

	// Create and show an alert view:
	UIAlertView* alertView = [[UIAlertView alloc] initWithTitle:titleAsNSString message:messageAsNSString delegate:nil cancelButtonTitle:@OS_STR_OK otherButtonTitles:nil];
	[alertView show];

	[pPool release];
}

