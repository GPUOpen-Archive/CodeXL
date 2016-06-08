//------------------------------ osGeneralFunctions.cpp ------------------------------

// POSIX:
#include <sys/utsname.h>

// Mac OS X
#include <sys/types.h>
#include <sys/sysctl.h>
#ifdef _GR_IPHONE_BUILD
    #include <CFNetwork/CFNetwork.h>
#else
    #include <Carbon/Carbon.h>
#endif

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osPipeExecutor.h>

// ---------------------------------------------------------------------------
// Name:        osGetOSShortDescriptionString
// Description: Retrieves a short string describing the Operating System on which
//              this program runs.
// Author:      Uri Shomroni
// Return Val: bool  - Success / failure.
// Date:        3/12/2008
// ---------------------------------------------------------------------------
bool osGetOSShortDescriptionString(gtString& osDescriptionString)
{
    osDescriptionString.makeEmpty();

    // Get the OS address space:
    gtString osAddressSpaceString;
    osGetOSAddressSpaceString(osAddressSpaceString);

    // Get OS type string length:
    gtSize_t osTypeLength = -1;
    int mib[2] = {CTL_KERN, KERN_OSTYPE};
    int resultCode = sysctl(mib, 2, NULL, &osTypeLength, NULL, 0);
    GT_IF_WITH_ASSERT((resultCode == 0) && (osTypeLength > 0))
    {
        // Allocate a string to hold the OS type:
        char* osTypeString = new char[osTypeLength];
        resultCode = sysctl(mib, 2, osTypeString, &osTypeLength, NULL, 0);
        GT_IF_WITH_ASSERT(resultCode == 0)
        {
            osDescriptionString.append(osTypeString);
        }

        delete[] osTypeString;
    }

    // If we don't know the specific OS type, write "Darwin":
    if (osDescriptionString.isEmpty())
    {
        osDescriptionString.append("Darwin");
    }

    // Add the version number:
    int major = 0, minor = 0, build = 0;
    bool rcBuild = osGetOperatingSystemVersionNumber(major, minor, build);

    if (rcBuild)
    {
        osDescriptionString.appendFormattedString(L"%d.%d", major, minor);
    }

    // Add the OS address space:
    osDescriptionString.append("-");
    osDescriptionString += osAddressSpaceString;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        osGetOperatingSystemVersionNumber
// Description: gets the operating system's version number as three ints
// Arguments: majorVersion - will contain the kernel major version
//            minorVersion - will contain the kernel minor version
//            buildNumber = will contain the kernel revision number
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        15/5/2008
// ---------------------------------------------------------------------------
bool osGetOperatingSystemVersionNumber(int& majorVersion, int& minorVersion, int& buildNumber)
{
    bool retVal = false;

    int mib[2] = {CTL_KERN, KERN_OSRELEASE};

    // Get the string length:
    gtSize_t osReleaseLength = -1;
    int resultCode = sysctl(mib, 2, NULL, &osReleaseLength, NULL, 0);
    GT_IF_WITH_ASSERT((resultCode == 0) && (osReleaseLength > 0))
    {
        char* osReleaseString = new char[osReleaseLength];
        resultCode = sysctl(mib, 2, osReleaseString, &osReleaseLength, NULL, 0);
        GT_IF_WITH_ASSERT(resultCode == 0)
        {
            gtString osRelease = osReleaseString;
            GT_IF_WITH_ASSERT(osRelease.count('.') >= 2)
            {
                gtString majStr, minStr, bldStr;
                int firstPeriod = osRelease.find('.');
                int secondPeriod = osRelease.find('.', (firstPeriod + 1));
                int thirdPeriond = osRelease.find('.', (secondPeriod + 1));

                if (thirdPeriond == -1)
                {
                    thirdPeriond = osRelease.length();
                }

                osRelease.getSubString(0, (firstPeriod - 1), majStr);
                osRelease.getSubString((firstPeriod + 1), (secondPeriod - 1), minStr);
                osRelease.getSubString((secondPeriod + 1), (thirdPeriond - 1), bldStr);
                bool rcMaj = majStr.toIntNumber(majorVersion);
                bool rcMin = minStr.toIntNumber(minorVersion);
                bool rcBld = bldStr.toIntNumber(buildNumber);
                retVal = rcMaj && rcMin && rcBld;
            }
        }
        delete[] osReleaseString;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetOperatingSystemVersionString
// Description: Get the Operating System Version as String
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        22/5/2008
// ---------------------------------------------------------------------------
bool osGetOperatingSystemVersionString(gtString& osVersionName)
{

    osVersionName.makeEmpty();

    bool rcOSXVers = false;

    // We get the OS X version with sw_vers. See http://developer.apple.com/DOCUMENTATION/Darwin/Reference/ManPages/man1/sw_vers.1.html
    // Get the OS name (OS X, OS X Server, etc)
    gtString macOSXName;
    osPipeExecutor pipeExec;
    bool rcSWVers = pipeExec.executeCommand("sw_vers -productName", macOSXName);
    GT_IF_WITH_ASSERT(rcSWVers)
    {
        osVersionName.append(macOSXName);

        // Get the OS X version
        rcSWVers = pipeExec.executeCommand("sw_vers -productVersion", macOSXName);
        GT_IF_WITH_ASSERT(rcSWVers)
        {
            osVersionName.append(" ").append(macOSXName);
            rcOSXVers = true;
        }
    }

    if (osVersionName.isEmpty())
    {
        osVersionName.append("Mac");
    }

    bool rcKern = false;
    gtString kernelVersion;

    // Get the kernel version from sysctl
    int mib[2] = {CTL_KERN, KERN_VERSION};

    // Get the string length:
    gtSize_t versionNameLength = -1;
    int resultCode = sysctl(mib, 2, NULL, &versionNameLength, NULL, 0);
    GT_IF_WITH_ASSERT((resultCode == 0) && (versionNameLength > 0))
    {
        char* versionNameString = new char[versionNameLength];
        resultCode = sysctl(mib, 2, versionNameString, &versionNameLength, NULL, 0);
        GT_IF_WITH_ASSERT(resultCode == 0)
        {
            osVersionName.append(" - ").append(versionNameString);
            rcKern = true;
        }
        delete[] versionNameString;
    }

    bool retVal = rcOSXVers && rcKern;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetOSAddressSpaceString
// Description:
//   Retrieves a string describing the address space type of the
//   Operating System on which we run.
// Author:      Yaki Tebeka
// Date:        4/2/2009
// ---------------------------------------------------------------------------
void osGetOSAddressSpaceString(gtString& addressSpaceString)
{
    addressSpaceString = "Unknown";

    // Get information about the OS kernel on which we run:
    struct utsname unameData;
    int rc1 = uname(&unameData);
    GT_IF_WITH_ASSERT(rc1 == 0)
    {
        gtString machineType = unameData.machine;

        if (machineType.compareNoCase("x86_64") == 0)
        {
            addressSpaceString = OS_64_BIT_ADDRESS_SPACE_AS_STR;
        }
        else if (machineType.compareNoCase("ia64") == 0)
        {
            addressSpaceString = OS_ITANIUM_64_BIT_ADDRESS_SPACE_AS_STR;
        }
        else if ((machineType.compareNoCase("i386") == 0) || (machineType.compareNoCase("i686") == 0))
        {
            addressSpaceString = OS_32_BIT_ADDRESS_SPACE_AS_STR;
        }
    }
}

// GUI elements are not relevant to the iPhone:
#ifndef _GR_IPHONE_BUILD

// We get build problems when compiling these functions under 64 bit (where they are not needed):
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE

// ---------------------------------------------------------------------------
// Name:        osGetMacListControlCurrentSortOrder
// Description: Uses carbon to determine the direction the current list's arrow
//              header is pointing
// Return Val:  true = ascending order (arrow is pointing up)
//              false = descending order (arrow is pointint down)
// Author:      Uri Shomroni
// Date:        31/12/2008
// ---------------------------------------------------------------------------
bool osGetMacListControlCurrentSortOrder(ControlRef hListCtrl)
{
    bool retVal = true;
    DataBrowserSortOrder order;
    OSStatus err = GetDataBrowserSortOrder(hListCtrl, &order);
    GT_IF_WITH_ASSERT(err == noErr)
    {
        if (order == kDataBrowserOrderDecreasing)
        {
            retVal = false;
        }
        else
        {
            GT_ASSERT((order == kDataBrowserOrderUndefined) || (order == kDataBrowserOrderIncreasing));
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osSetMacListControlSortOrderAndColumn
// Description: Uses carbon to set the list control's arrow to be in the requested
//              direction on the requested column. If column = -1, only change the
//              direction.
// Author:      Uri Shomroni
// Date:        1/1/2009
// ---------------------------------------------------------------------------
void osSetMacListControlSortOrderAndColumn(ControlRef hListCtrl, int column, bool ascending)
{
    bool shouldApplyDirectionChange = false;
    OSStatus err = noErr;

    if (column >= 0)
    {
        // Get the carbon ID of the column:
        DataBrowserPropertyID columnId;
        DataBrowserTableViewColumnIndex columnIndex = (DataBrowserTableViewColumnIndex)column;

        err = GetDataBrowserTableViewColumnProperty(hListCtrl, columnIndex, &columnId);
        GT_IF_WITH_ASSERT(err == noErr)
        {
            // Set it to be highlighted (blue)
            err = SetDataBrowserSortProperty(hListCtrl, columnId);
            GT_IF_WITH_ASSERT(err == noErr)
            {
                // If we tried to change the column number and failed, we don't want
                // to change the direction.
                shouldApplyDirectionChange = true;
            }
        }
    }
    else
    {
        shouldApplyDirectionChange = true;
    }

    if (shouldApplyDirectionChange)
    {
        // Calculate the constant for the sort order
        DataBrowserSortOrder order = kDataBrowserOrderUndefined;

        if (ascending)
        {
            order = kDataBrowserOrderIncreasing;
        }
        else
        {
            order = kDataBrowserOrderDecreasing;
        }

        // Set the direction of the arrow
        err = SetDataBrowserSortOrder(hListCtrl, order);
        GT_ASSERT(err == noErr);
    }
}

// ---------------------------------------------------------------------------
// Name:        osSetMacAlwaysOnTopStatusOfWindow
// Description: Applies the current application-wide "always on top" status to
//              rWindow
// Arguments: /* WindowRef */ void* rWindow
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        22/2/2009
// ---------------------------------------------------------------------------
bool osSetMacAlwaysOnTopStatusOfWindow(/* WindowRef */ void* rWindow, bool alwaysOnTop)
{
    bool retVal = false;

    // Utility windows float above floating windows of other applications:
    WindowClass winClass = kDocumentWindowClass;

    if (alwaysOnTop)
    {
        winClass = kUtilityWindowClass;
    }

    WindowGroupRef newGroup = GetWindowGroupOfClass(winClass);

    OSStatus errCode = SetWindowGroup((WindowRef)rWindow, newGroup);
    GT_IF_WITH_ASSERT(errCode == noErr)
    {
        retVal = true;
    }

    return retVal;
}

#endif // AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
#endif // not defined _GR_IPHONE_BUILD

// ---------------------------------------------------------------------------
// Name:        osGetStringPropertyValueFromPListFile
// Description: Opens the plist file specified, looks up for propertyName in its
//              root dictionary and inserts its value (currently only supports
//              CFString values) to propertyValue
//              More info at: http://developer.apple.com/documentation/CoreFoundation/Reference/CFPropertyListRef/Reference/reference.html
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        6/1/2009
// ---------------------------------------------------------------------------
bool osGetStringPropertyValueFromPListFile(const gtString& fileFullPath, const gtString& propertyName, gtString& propertyValue)
{
    bool retVal = false;

    CFStringRef stringValue = CFStringCreateWithCString(kCFAllocatorDefault, fileFullPath.asCharArray(), kCFStringEncodingMacRoman);

    // http://developer.apple.com/documentation/CoreFoundation/Reference/CFURLRef/Reference/reference.html#//apple_ref/c/func/CFURLCreateWithFileSystemPath
    CFURLRef plistFileAsCFURLRef = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, stringValue, kCFURLPOSIXPathStyle, false);

    GT_IF_WITH_ASSERT(plistFileAsCFURLRef != NULL)
    {
        // http://developer.apple.com/documentation/CoreFoundation/Reference/CFURLAccessUtils/Reference/reference.html#//apple_ref/c/func/CFURLCreateDataAndPropertiesFromResource
        CFDataRef plistXMLDataAsCFDataRef;
        SInt32 errorCode;
        Boolean status = CFURLCreateDataAndPropertiesFromResource(kCFAllocatorDefault, plistFileAsCFURLRef, &plistXMLDataAsCFDataRef, NULL, NULL, &errorCode);

        if (status == true)
        {
            CFStringRef errorCFString;
            CFPropertyListRef propertyList = CFPropertyListCreateFromXMLData(kCFAllocatorDefault, plistXMLDataAsCFDataRef, kCFPropertyListImmutable, &errorCFString);

            if (propertyList != NULL)
            {
                GT_IF_WITH_ASSERT(CFGetTypeID(propertyList) == CFDictionaryGetTypeID())
                {
                    stringValue = CFStringCreateWithCString(kCFAllocatorDefault, propertyName.asCharArray(), kCFStringEncodingMacRoman);
                    CFDictionaryRef plistDictionary = (CFDictionaryRef)propertyList;
                    CFStringRef valueAsCFString = (CFStringRef)CFDictionaryGetValue(plistDictionary, stringValue);

                    GT_IF_WITH_ASSERT_EX((valueAsCFString != NULL), "Could not find request value in plist file")
                    {
                        propertyValue = CFStringGetCStringPtr(valueAsCFString, kCFStringEncodingMacRoman);
                        retVal = true;
                    }
                }

                CFRelease(propertyList);
            }
            else
            {
                gtString errorDescription = "Could not open plist file \"";
                errorDescription.append(fileFullPath).append("\".\nCF returned the following error: ");
                errorDescription.append(CFStringGetCStringPtr(errorCFString, kCFStringEncodingMacRoman));
                GT_ASSERT_EX(false, errorDescription.asCharArray());
            }

            CFRelease(plistXMLDataAsCFDataRef);
        }
        else
        {
            gtString foobar = "Error in getting file data from file named \"";
            foobar.append(fileFullPath);
            foobar.appendFormattedString(L"\". Error code is %d.", errorCode);
            GT_ASSERT_EX(false, foobar.asCharArray());
        }

        CFRelease(plistFileAsCFURLRef);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetExecutableFromMacApplicationBundle
// Description: Takes an application bundle (as either a folder or a *.app file)
//              and returns the actual executable from inside it.
// Author:      Uri Shomroni
// Date:        12/5/2009
// ---------------------------------------------------------------------------
gtString osGetExecutableFromMacApplicationBundle(const gtString& bundlePath)
{
    gtString executablePathAsString = bundlePath;

    // If we got the string as a directory, make it into a "file":
    executablePathAsString.removeTrailing(osFilePath::osPathSeparator);

    // Create a spoof file path which takes the .app directory to be a "file":
    osFilePath executableSpoofPath(executablePathAsString);

    gtString executableExtension;
    GT_IF_WITH_ASSERT(executableSpoofPath.getFileExtension(executableExtension))
    {
        if (executableExtension == OS_MAC_APPLICATION_BUNDLE_FILE_EXTENSION)
        {
            // The executable "file" is actually a directory:
            gtString bundleContentsDirectory = executableSpoofPath.asString();
            bool shouldAttemptDefaultExecutable = true;

            // Add the internal structure of the bundle:
            bundleContentsDirectory.append(osFilePath::osPathSeparator).append(OS_MAC_APPLICATION_BUNDLE_INTERNAL_PATH1);
            gtString execPathDirectory = bundleContentsDirectory;
            execPathDirectory.append(osFilePath::osPathSeparator).append(OS_MAC_APPLICATION_BUNDLE_INTERNAL_PATH2);

            gtString execPathExecutable;
            gtString bundleInfoPListFileFullPath = bundleContentsDirectory;
            bundleInfoPListFileFullPath.append(osFilePath::osPathSeparator).append("Info.plist");

            // Check if the bundle has an Info.plist file:
            osFilePath bundleInfoPlist(bundleInfoPListFileFullPath);
            bool bundleHasInfoPlistFile = bundleInfoPlist.exists();

            if (!bundleHasInfoPlistFile)
            {
                // Some bundles (notably iPhone bundles) have the Info.plist file directly
                // under the bundle instead of in the Contents/ directory.
                osDirectory infoPlistDir;
                bundleInfoPlist.getFileDirectory(infoPlistDir);
                infoPlistDir.upOneLevel();
                bundleInfoPlist.setFileDirectory(infoPlistDir);
                bundleHasInfoPlistFile = bundleInfoPlist.exists();
                bundleInfoPListFileFullPath = bundleInfoPlist.asString();
            }

            if (bundleHasInfoPlistFile)
            {
                bool rcName = osGetStringPropertyValueFromPListFile(bundleInfoPListFileFullPath, "CFBundleExecutable", execPathExecutable);

                if (rcName)
                {
                    gtString execRealPathAsString = execPathDirectory;
                    execRealPathAsString.append(osFilePath::osPathSeparator).append(execPathExecutable);

                    // Convert to an osFilePath to verify this file actually exists:
                    osFilePath execRealFilePath(execRealPathAsString);

                    if (execRealFilePath.exists())
                    {
                        executablePathAsString = execRealFilePath.asString();
                        shouldAttemptDefaultExecutable = false;
                    }
                    else
                    {
                        // Some bundles (notably iPhone bundles) have the executable directly under the bundle instead
                        // of under the Contents/MacOS/ folder:
                        osDirectory execFileDir;
                        execRealFilePath.getFileDirectory(execFileDir);
                        execFileDir.upOneLevel().upOneLevel();
                        execRealFilePath.setFileDirectory(execFileDir);
                        GT_IF_WITH_ASSERT(execRealFilePath.exists())
                        {
                            executablePathAsString = execRealFilePath.asString();
                            shouldAttemptDefaultExecutable = false;
                        }
                    }
                }
            }

            if (shouldAttemptDefaultExecutable)
            {
                // If the Info.plist file didn't have a CFBundleExecutable property or the specified
                // executable doesn't exist, we should try the default executable, which is the bundle
                // name:

                bool rcName = executableSpoofPath.getFileName(execPathExecutable);
                GT_IF_WITH_ASSERT(rcName)
                {
                    gtString execRealPathAsString = execPathDirectory;
                    execRealPathAsString.append(osFilePath::osPathSeparator).append(execPathExecutable);

                    // Convert to an osFilePath to verify this file actually exists:
                    osFilePath execRealFilePath(execRealPathAsString);
                    GT_IF_WITH_ASSERT(execRealFilePath.exists())
                    {
                        executablePathAsString = execRealFilePath.asString();
                    }
                }
            }
        }
    }

    return executablePathAsString;
}

// ---------------------------------------------------------------------------
// Name:        osDoesProcessExist
// Description: Checks if there is a process with the name processName currently running
// Author:      Uri Shomroni
// Date:        10/6/2009
// ---------------------------------------------------------------------------
bool osDoesProcessExist(const gtString& processName)
{
    bool retVal = false;

    int mib[3] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL};
    gtSize_t osKernProcAllLength = -1;
    int resultCode = sysctl(mib, 3, NULL, &osKernProcAllLength, NULL, 0);
    GT_IF_WITH_ASSERT((resultCode == 0) && (osKernProcAllLength > 0))
    {
        gtByte* pKInfoProcAsBytes = new gtByte[osKernProcAllLength];
        struct kinfo_proc* pKInfoProc = (struct kinfo_proc*)pKInfoProcAsBytes;
        resultCode = sysctl(mib, 3, pKInfoProc, &osKernProcAllLength, NULL, 0);
        GT_IF_WITH_ASSERT(resultCode == 0)
        {
            int numberOfProcesses = osKernProcAllLength / sizeof(struct kinfo_proc);
            GT_ASSERT((osKernProcAllLength % sizeof(struct kinfo_proc)) == 0);

            for (int i = 0; i < numberOfProcesses; i++)
            {
                // Get the name and PID of the i-th process:
                pid_t currentPID = pKInfoProc[i].kp_proc.p_pid;
                gtString currentProcessName = pKInfoProc[i].kp_proc.p_comm;

                // Check if a process of the required name exists and has a legal PID:
                if (currentPID > 0)
                {
                    if (currentProcessName == processName)
                    {
                        retVal = true;
                        break;
                    }
                    else
                    {
                        // Check if this is the process full path, currentProcessName is of the format ".../" + processName:
                        int processNamePosition = currentProcessName.reverseFind(processName);

                        if (processNamePosition > 0)
                        {
                            if ((currentProcessName[processNamePosition - 1] == osFilePath::osPathSeparator) && // There's a path separator before the process name
                                ((processNamePosition + processName.length()) == currentProcessName.length()))  // The string ends with the process name
                            {
                                retVal = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
        delete[] pKInfoProcAsBytes;
    }

    return retVal;
}

