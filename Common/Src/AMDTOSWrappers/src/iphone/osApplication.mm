//------------------------------ osApplication.mm ------------------------------

#ifndef _GR_IPHONE_BUILD
	#error Build this file only on the iPhone
#endif

// Foundation framework:
#import <Foundation/Foundation.h>

// Infra:
#import <GRBaseTools/gtAssert.h>

// Local:
#import <GROSWrappers/osFilePath.h>
#import <GROSWrappers/osApplication.h>

// ---------------------------------------------------------------------------
// Name:        osGetCurrentApplicationPath
// Description: Returns the current application path (exe full path).
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        1/6/2009
// ---------------------------------------------------------------------------
bool osGetCurrentApplicationPath(osFilePath& applicationPath)
{
	bool retVal = false;

	// This function might be called from a non-main thread, so use an autorelease pool
	// to avoid memory leak warnings (see "Autorelease Pools" in 
	// http://developer.apple.com/mac/library/documentation/Cocoa/Conceptual/MemoryMgmt/MemoryMgmt.html):
	NSAutoreleasePool* pPool = [[NSAutoreleasePool alloc] init];

	NSString* mainBundleExecutablePath = [[[NSBundle class] mainBundle] executablePath];
	GT_IF_WITH_ASSERT (mainBundleExecutablePath != NULL)
	{
		const char* pExecutablePathAsString = [mainBundleExecutablePath cStringUsingEncoding: NSMacOSRomanStringEncoding];
		GT_IF_WITH_ASSERT(pExecutablePathAsString != NULL)
		{
			gtString executablePathAsGTString = pExecutablePathAsString;
			applicationPath.setFullPathFromString(executablePathAsGTString);
			retVal = applicationPath.exists();
		}
	}

	[pPool release];

	return retVal;
}

#ifdef _GR_IPHONE_DEVICE_BUILD
// ---------------------------------------------------------------------------
// Name:        osGetiPhoneApplicationSpecialPath
// Description: Gets a predefined path which is relative to the app bundle on
//				the iPhone.
//				Currently only implemented for the temp folder.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        8/11/2009
// ---------------------------------------------------------------------------
bool osGetiPhoneApplicationSpecialPath(osFilePath::osPreDefinedFilePaths predefinedPath, gtString& fullPathAsString)
{
	bool retVal = false;

	// This function might be called from a non-main thread, so use an autorelease pool
	// to avoid memory leak warnings (see "Autorelease Pools" in 
	// http://developer.apple.com/mac/library/documentation/Cocoa/Conceptual/MemoryMgmt/MemoryMgmt.html):
	NSAutoreleasePool* pPool = [[NSAutoreleasePool alloc] init];

	NSString* filePathAsNSString = nil;

	switch (predefinedPath)
	{
	case osFilePath::OS_TEMP_DIRECTORY:
		{
			filePathAsNSString = NSTemporaryDirectory();
		}
		break;

	default:
		{
			// We shouldn't get here:
			GT_ASSERT(false);
		}
		break;
	}

	if (filePathAsNSString != nil)
	{
		fullPathAsString = [filePathAsNSString cStringUsingEncoding: NSMacOSRomanStringEncoding];
		retVal = !fullPathAsString.isEmpty();
	}

	[pPool release];

	return retVal;
}
#endif
