//------------------------------ osFilePath.cpp ------------------------------

// Mac OS X:
#ifdef  _GR_IPHONE_BUILD
    #include <CFNetwork/CFNetwork.h>
#else
    #include <Carbon/Carbon.h>
#endif

// Standard C:
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// iPhone on-device only includes:
#ifdef _GR_IPHONE_DEVICE_BUILD
    #include <AMDTOSWrappers/Include/osApplication.h>
#endif

// Static members initializations:
const wchar_t osFilePath::osPathSeparator = '/';
const wchar_t osFilePath::osExtensionSeparator = '.';
const wchar_t osFilePath::osEnvironmentVariablePathsSeparator = ':';

// Constants:
// TO_DO MAC: see how this is done in wxStandardPaths and apply instead of constants:
// Uri, 28/1/09: wx uses ([kLocalDomain|kUserDomain], kApplicationSupportFolderType, kCreateFolder)
//               for [~]/Library/Application Support/
//               ([kLocalDomain|kUserDomain], kPreferencesFolderType, kCreateFolder)
//               for [~]/Library/Preferences/
//               (kUserDomain, kDocumentsFolderType, kCreateFolder)
//               for ~/Documents/
//////////////////////////////////////////////////////////////////////////
// Uri, 9/12/08: Kent from Apple indicated using the "user experience", where
// (http://developer.apple.com/documentation/MacOSX/Conceptual/BPFileSystem/Articles/WhereToPutFiles.html)
// it is explained theoretically how to find these. However, it doesn't really work
// and the result paths are incorrect, so until we receive a reply from him, we
// use hard-coded values.
//////////////////////////////////////////////////////////////////////////
#define OS_PATH_STRING_BUFFER_SIZE FILENAME_MAX
#define OS_MAC_TEMP_DIR_PATH L"/tmp"                        // Note that on Mac OS X this is a symbolic link (usually to /private/tmp), and thus is always correct.
#define OS_MAC_COMMON_APPLICATION_DATA_DIR L"/etc"          // Note that on Mac OS X this is a symbolic link (usually to /private/etc), and thus is always correct.
#define OS_MAC_SYSTEM_DIR_PATH L"/usr/lib"
#define OS_CURR_USER_ENV_VARIABLE L"USER"
#define OS_HOME_DIR_ENV_VARIABLE L"HOME"
#define OS_ROOT_USER_HOME_DIR L"/root"
#define OS_MAC_APPLICATION_SUPPORT_DIR1 L"Library"
#define OS_MAC_APPLICATION_SUPPORT_DIR2 L"Application Support"
#define OS_MAC_APPLICATION_SUPPORT_DIR3 L"AMD"


// ---------------------------------------------------------------------------
// Name:        osGetDirectoryNameFromMacOSType
// Description: Helper funciton wrapping FSFindFolder (see
// http://developer.apple.com/documentation/Carbon/Reference/Folder_Manager/Reference/reference.html#//apple_ref/c/func/FSFindFolder
// The input value is from this Enum http://developer.apple.com/documentation/Carbon/Reference/Folder_Manager/Reference/reference.html#//apple_ref/c/tdef/FolderType
// Arguments: const OSType folderType
//            gtString& folderFullPath
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        9/12/2008
// ---------------------------------------------------------------------------
bool osGetDirectoryNameFromMacOSType(const OSType folderType, gtString& folderFullPath)
{
    bool retVal = false;
#ifdef _GR_IPHONE_BUILD
    // TO_DO iPhone
#else
    FSRef folderFileSystemRef;
    OSErr errorCode = FSFindFolder(kOnSystemDisk ,                  // See http://developer.apple.com/documentation/Carbon/Reference/Folder_Manager/Reference/reference.html#//apple_ref/doc/constant_group/Disk_and_Domain_Constants
                                   folderType,
                                   kDontCreateFolder,               // Don't create the folder if it doesn't exist.
                                   &folderFileSystemRef);

    // If we failed while looking for the folder in the system disk, try the local domain:
    if (errorCode != noErr)
    {
        errorCode = FSFindFolder(kLocalDomain, folderType, kDontCreateFolder, &folderFileSystemRef);
    }

    // If we failed while looking for the folder in the local domain, try the user domain:
    if (errorCode != noErr)
    {
        errorCode = FSFindFolder(kUserDomain, folderType, kDontCreateFolder, &folderFileSystemRef);
    }

    if (errorCode == noErr)
    {
        // If we succeeded, convert the FSRef to a string:
        CFURLRef folderAsCFURLRef = CFURLCreateFromFSRef(kCFAllocatorDefault, &folderFileSystemRef);

        GT_IF_WITH_ASSERT(folderAsCFURLRef != NULL)
        {
            // Convert the URL to a string:
            CFStringRef folderAsCFStringRef = CFURLCopyFileSystemPath(folderAsCFURLRef, kCFURLPOSIXPathStyle);
            folderFullPath = CFStringGetCStringPtr(folderAsCFStringRef, kCFStringEncodingMacRoman);
            GT_IF_WITH_ASSERT(!folderFullPath.isEmpty())
            {
                retVal = true;
            }

            // Release the CFURL and CFString:
            CFRelease(folderAsCFURLRef);
            CFRelease(folderAsCFStringRef);
        }
    }
    else
    {
        gtString errMessage = "Error finding folder of type";
        errMessage.append(folderType);
        errMessage.appendFormattedString(L". Error code is %d.", errorCode);
        GT_ASSERT_EX(false, errMessage.asCharArray());
    }

#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osFilePath::setPath
// Description: Sets the file path from a pre-defined file path
// Arguments:   predefinedfilePath - The pre-defined file path
//              (See osFilePath::osPreDefinedFilePaths)
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        30/5/2004
// ---------------------------------------------------------------------------
bool osFilePath::setPath(osPreDefinedFilePaths predefinedfilePath, bool applyRedirection)
{
    bool retVal = false;

    // Folder redirection only exists in Windows:
    GT_UNREFERENCED_PARAMETER(applyRedirection);

    wchar_t stringBuffer[OS_PATH_STRING_BUFFER_SIZE];

    // Get the directory path:
    switch (predefinedfilePath)
    {
        case OS_SYSTEM_DIRECTORY:
        case OS_SYSTEM_X86_DIRECTORY:
        {
            // TO_DO Mac
            //          gtString sysDirPath;
            //          osGetDirectoryNameFromMacOSType(kSystemFolderType, sysDirPath); // usually /usr/lib
            //          strcpy(stringBuffer, sysDirPath.asCharArray());
            wcscpy(stringBuffer, OS_MAC_SYSTEM_DIR_PATH);
            retVal = true;
        }
        break;

        case OS_TEMP_DIRECTORY:
        {
            // TO_DO Mac
            //          gtString tmpDirPath;
            //          osGetDirectoryNameFromMacOSType(kTemporaryFolderType, tmpDirPath); // usually /tmp
            //          strcpy(stringBuffer, tmpDirPath.asCharArray());
#ifdef _GR_IPHONE_DEVICE_BUILD
            gtString predefinedPathAsString;
            retVal = osGetiPhoneApplicationSpecialPath(predefinedfilePath, predefinedPathAsString);
            predefinedPathAsString.removeTrailing(osFilePath::osPathSeparator);

            if (retVal)
            {
                if (predefinedPathAsString.length() > OS_PATH_STRING_BUFFER_SIZE)
                {
                    predefinedPathAsString.truncate(0, (OS_PATH_STRING_BUFFER_SIZE - 1));
                }

                ::wcscpy(stringBuffer, predefinedPathAsString.asCharArray());
            }

#else // ndef _GR_IPHONE_DEVICE_BUILD
            wcscpy(stringBuffer, OS_MAC_TEMP_DIR_PATH);
            retVal = true;
#endif
        }
        break;

        case OS_USER_APPLICATION_DATA:
        {
            InitializeUnicodeCharactersUserFilePath(applyRedirection);
            wcscpy(stringBuffer, ms_userAppDataFilePath.asString().asCharArray());

            retVal = true;
        }
        break;

        case OS_ROOT_USER_APPLICATION_DATA:
        {
            // We define the root user application data directory to be /root/.amd
            // Build /root/.amd:
            osFilePath amdDirPath(OS_ROOT_USER_HOME_DIR);
            amdDirPath.appendSubDirectory(OS_MAC_APPLICATION_SUPPORT_DIR1);
            amdDirPath.appendSubDirectory(OS_MAC_APPLICATION_SUPPORT_DIR2);
            amdDirPath.appendSubDirectory(OS_MAC_APPLICATION_SUPPORT_DIR3);

            wcscpy(stringBuffer, amdDirPath.asString().asCharArray());

            // If it does not exist - create it:
            osDirectory amdDir(amdDirPath);
            bool amdDirExists = amdDir.exists();

            if (!amdDirExists)
            {
                amdDirExists = amdDir.create();
            }

            // Verify that the directory exists:
            GT_IF_WITH_ASSERT(amdDirExists)
            {
                retVal = true;
            }
        }

        case OS_BROWSE_EXECUTABLES_DIRECTORY:   // In Mac, we start browsing in the user's home dir.
        case OS_USER_DOCUMENTS:
        {
            // Get the current user's home directory:
            const wchar_t* pCurrUserHomeDirectory = _wgetenv(OS_HOME_DIR_ENV_VARIABLE);
            GT_IF_WITH_ASSERT(pCurrUserHomeDirectory != NULL)
            {
                // Yaki 7/7/2008:
                // When running as a Linux service at boot time, we get "/" as the current user's home directory.
                // We fix that to be "/root".
                if (wcscmp(pCurrUserHomeDirectory, "/") == 0)
                {
                    static wchar_t stat_rootHomeDir[] = OS_ROOT_USER_HOME_DIR;
                    pCurrUserHomeDirectory = stat_rootHomeDir;
                }

                // Copy it to the path string buffer:
                wcscpy(stringBuffer, pCurrUserHomeDirectory);
                retVal = true;
            }
        }
        break;

        case OS_COMMON_APPLICATION_DATA:
        {
            // TO_DO Mac
            //          gtString etcDirPath;
            //          osGetDirectoryNameFromMacOSType(kApplicationSupportFolderType, etcDirPath); // usually /etc
            //          strcpy(stringBuffer, etcDirPath.asCharArray());
            wcscpy(stringBuffer, OS_MAC_COMMON_APPLICATION_DATA_DIR);
            retVal = true;
        }
        break;

        case OS_CURRENT_DIRECTORY:
        {
            // Get current directory:
            char* rc = getcwd(stringBuffer, OS_PATH_STRING_BUFFER_SIZE);
            GT_IF_WITH_ASSERT(rc != NULL)
            {
                retVal = true;
            }
        }
        break;

        default:
        {
            // Unknown pre-defined directory:
            GT_ASSERT(false);
        }
        break;
    }

    if (retVal)
    {
        _fileDirectory = stringBuffer;
    }

    return retVal;
}

bool osFilePath::GetUserAppDataFilePath(osFilePath& userAppDataFilePath, bool applyRedirection)
{
    bool retVal = false;
    wchar_t stringBuffer[OS_PATH_STRING_BUFFER_SIZE];

    // Get the current user's documents directory:
    userAppDataFilePath = osFilePath(osFilePath::OS_USER_DOCUMENTS);

    // Sanity check:
    GT_IF_WITH_ASSERT(!(userAppDataFilePath._fileDirectory.isEmpty()))
    {
        // We define the application data directory to be <users documents dir>/Library/Application Support/AMD/CodeXL
        userAppDataFilePath.appendSubDirectory(OS_MAC_APPLICATION_SUPPORT_DIR1);
        userAppDataFilePath.appendSubDirectory(OS_MAC_APPLICATION_SUPPORT_DIR2);
        userAppDataFilePath.appendSubDirectory(OS_MAC_APPLICATION_SUPPORT_DIR3);
        wcscpy(stringBuffer, userAppDataFilePath.asString().asCharArray());

        // If it does not exist - create it:
        osDirectory amdDir(userAppDataFilePath);
        bool amdDirExists = amdDir.exists();

        if (!amdDirExists)
        {
            amdDirExists = amdDir.create();
        }

        // Verify that the directory exists:
        GT_ASSERT(amdDirExists);

        userAppDataFilePath.setFileDirectory(stringBuffer);

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osFilePath::adjustToCurrentOS
// Description: Adjust the path string to the current operating system.
// Author:      Yaki Tebeka
// Date:        1/11/2006
// Usage Sample:
// Implementation notes:
//  We have to adjust the file path to Linux conventions:
//  a. Replace "\" by "/".
//  b. Remove trailing "/" ("/foo/" -> "/foo").
// ---------------------------------------------------------------------------
void osFilePath::adjustStringToCurrentOS(gtString& filePathString)
{
    // a. Replace "\" by "/"
    int len = filePathString.length();

    for (int i = 0; i < len; i++)
    {
        if (filePathString[i] == '\\')
        {
            filePathString[i] = osPathSeparator;
        }
    }

    // b. Remove trailing "\":
    bool pathEndedInSeparator = false;
    int filePathLength = filePathString.length();

    if (filePathLength > 0)
    {
        pathEndedInSeparator = (filePathString[filePathLength] == osPathSeparator);
    }

    filePathString.removeTrailing(osPathSeparator);

    if (pathEndedInSeparator)
    {
        filePathString.append(osPathSeparator);
    }
}

// TODO: implement
bool osFilePath::Rename(const gtString& newNameFullPath)
{
    GT_ASSERT(false);
    return false;
}