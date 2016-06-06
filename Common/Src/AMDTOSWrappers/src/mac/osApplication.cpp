//------------------------------ osApplication.cpp ------------------------------

#ifdef _GR_IPHONE_BUILD
    #error Build this file only on Mac OS X
#endif

// Mac OS X:
#include <Carbon/Carbon.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osApplication.h>


// ---------------------------------------------------------------------------
// Name:        osGetCurrentApplicationPath
// Description: Returns the current application path (exe full path).
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        2/12/2008
// Implementation notes:
// See the carbon process manager spec in
// http://developer.apple.com/documentation/Carbon/Reference/Process_Manager/Reference/reference.html
// Converting from a FSRef to a CFStringRef as exaplined in listing 7 in
// http://developer.apple.com/technotes/tn2002/tn2078.html#GETTINGAFILEPATH
// ---------------------------------------------------------------------------
bool osGetCurrentApplicationPath(osFilePath& applicationPath, bool convertToLower)
{
    (void)(convertToLower); // unused
    bool retVal = false;

    // Get the current process serial number:
    ProcessSerialNumber psn = {0, kCurrentProcess};
    OSErr errorCode = GetCurrentProcess(&psn);
    GT_IF_WITH_ASSERT(errorCode == noErr)
    {
        FSRef executableFileSystemReference;
        errorCode = GetProcessBundleLocation(&psn, &executableFileSystemReference);
        GT_IF_WITH_ASSERT(errorCode == noErr)
        {
            CFURLRef executablePathAsCFURLRef = CFURLCreateFromFSRef(kCFAllocatorDefault, &executableFileSystemReference);

            GT_IF_WITH_ASSERT(executablePathAsCFURLRef != NULL)
            {
                // Convert the URL to a string:
                CFStringRef executableNameAsCFStringRef = CFURLCopyFileSystemPath(executablePathAsCFURLRef, kCFURLPOSIXPathStyle);
                gtString executableNameAsString = CFStringGetCStringPtr(executableNameAsCFStringRef, kCFStringEncodingMacRoman);
                GT_IF_WITH_ASSERT(!executableNameAsString.isEmpty())
                {
                    applicationPath.setFullPathFromString(executableNameAsString);
                    retVal = true;
                }

                // Release the CFURL and CFString:
                CFRelease(executablePathAsCFURLRef);
                CFRelease(executableNameAsCFStringRef);
            }
        }
    }

    return retVal;
}
