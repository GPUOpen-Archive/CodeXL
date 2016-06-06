//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osFilePath.cpp
///
//=====================================================================

//------------------------------ osFilePath.cpp ------------------------------

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <shlobj.h>
#include <sys/stat.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>

// Static members initializations:
const wchar_t osFilePath::osPathSeparator = '\\';
const wchar_t osFilePath::osExtensionSeparator = '.';
const wchar_t osFilePath::osEnvironmentVariablePathsSeparator = ';';

// Constants:
#define OS_PATH_STRING_BUFFER_SIZE MAX_PATH + 1
#define OS_TEMP_DIRECTORY_ENV_VARIABLE_NAME L"TEMP"
#define OS_USER_DOWNLOAD_DIR L"Downloads"

// ---------------------------------------------------------------------------
// Name:        osFilePath::operator==
// Author:      AMD Developer Tools Team
// Date:        7/1/2006
// ---------------------------------------------------------------------------
bool osFilePath::operator==(const osFilePath& other) const
{
    bool retVal = false;

    if (_fileExtension.isEqualNoCase(other._fileExtension))
    {
        if (_fileName.isEqualNoCase(other._fileName))
        {
            if (_fileDirectory.isEqualNoCase(other._fileDirectory))
            {
                retVal = true;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osFilePath::isRelativePath
// Author:      AMD Developer Tools Team
// Date:        9/2/2016
// ---------------------------------------------------------------------------
bool osFilePath::isRelativePath() const
{
    bool retVal = true;

    // In Windows, absolute paths start with a drive letter, followed by a colon and possibly a slash:
    int dirLen = _fileDirectory.length();

    if (1 < dirLen)
    {
        if (((('A' <= _fileDirectory[0]) && ('Z' >= _fileDirectory[0])) ||
             (('a' <= _fileDirectory[0]) && ('z' >= _fileDirectory[0]))) &&
            (':' == _fileDirectory[1]))
        {
            // Paths like "C:" or "C:\" are also absolute paths, so don't check for '\' == _fileDirectory[2]:
            retVal = false;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osFilePath::setPath
// Description: Sets the file path from a pre-defined file path
// Arguments:   predefinedfilePath - The pre-defined file path
//              (See osFilePath::osPreDefinedFilePaths)
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        30/5/2004
// Implementation notes:
//  - For the used folders see Microsoft Knowledge base Article 310294: "How to write a Windows XP
//    Application that stores user and application data in the correct location by using Visual C++ 2005
//    or Visual C++ .NET"
//  - Where applicable, we use SHGetFolderPath, which should be safer than GetEnvironmentVariable.
// ---------------------------------------------------------------------------
bool osFilePath::setPath(osPreDefinedFilePaths predefinedfilePath, bool applyRedirection)
{
    bool retVal = false;
    bool copyBuffer = true;
    wchar_t stringBuffer[OS_PATH_STRING_BUFFER_SIZE] = { 0 };
    DWORD rc = 0;

    // Get the directory path:
    switch (predefinedfilePath)
    {
        case OS_SYSTEM_DIRECTORY:
        {
            // Get the directory where the system is installed:
            HRESULT hr = ::SHGetFolderPath(NULL, CSIDL_SYSTEM, NULL, applyRedirection ? SHGFP_TYPE_CURRENT : SHGFP_TYPE_DEFAULT, stringBuffer);

            if (S_OK != hr)
            {
                // If we failed with / without redirection, try the other option:
                hr = ::SHGetFolderPath(NULL, CSIDL_SYSTEM, NULL, applyRedirection ? SHGFP_TYPE_DEFAULT : SHGFP_TYPE_CURRENT, stringBuffer);
            }

            if (hr == S_OK)
            {
                rc = 1;
            }
            else
            {
                // Yaki 29/11/2009:
                // On WinVista 64, when in a DLLInit function, the above SHGetFolderPath sometimes fail.
                // Therefore, in case of such failure, we fall back to a hard coded system directory:
                wcscpy(stringBuffer, L"c:\\Windows\\System32");
                rc = 1;
            }
        }
        break;

        case OS_SYSTEM_X86_DIRECTORY:
        {
            // Get the directory where the system is installed:
            HRESULT hr = ::SHGetFolderPath(NULL, CSIDL_SYSTEMX86, NULL, applyRedirection ? SHGFP_TYPE_CURRENT : SHGFP_TYPE_DEFAULT, stringBuffer);

            if (S_OK != hr)
            {
                // If we failed with / without redirection, try the other option:
                hr = ::SHGetFolderPath(NULL, CSIDL_SYSTEMX86, NULL, applyRedirection ? SHGFP_TYPE_DEFAULT : SHGFP_TYPE_CURRENT, stringBuffer);
            }

            if (hr == S_OK)
            {
                rc = 1;
            }
            else
            {
                // Yaki 29/11/2009:
                // On WinVista 64, when in a DLLInit function, the above SHGetFolderPath sometimes fail.
                // Therefore, in case of such failure, we fall back to a hard coded system directory:
                BOOL isWOW64 = FALSE;
                IsWow64Process(GetCurrentProcess(), &isWOW64);
                wcscpy(stringBuffer, isWOW64 ? L"c:\\Windows\\SysWOW64" : L"c:\\Windows\\System32");
                rc = 1;
            }
        }
        break;

        case OS_TEMP_DIRECTORY:
        {
            // Get Windows TEMP directory:
            rc = GetEnvironmentVariable(OS_TEMP_DIRECTORY_ENV_VARIABLE_NAME, stringBuffer,
                                        OS_PATH_STRING_BUFFER_SIZE);

            if (0 != rc)
            {
                osDirectory tempDir;
                tempDir.setDirectoryFullPathFromString(stringBuffer);

                if (!tempDir.exists())
                {
                    if (!tempDir.create())
                    {
                        rc = 0;
                    }
                }
            }
        }
        break;

        case OS_USER_APPLICATION_DATA:
        {
            InitializeUnicodeCharactersUserFilePath(applyRedirection);

            wcscpy(stringBuffer, ms_userAppDataFilePath.asString().asCharArray());
            rc = 1;
            retVal = true;

        }
        break;

        case OS_USER_DOCUMENTS:
        {
            // Get the users "My documents" directory:
            HRESULT hr = ::SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, applyRedirection ? SHGFP_TYPE_CURRENT : SHGFP_TYPE_DEFAULT, stringBuffer);

            if (S_OK != hr)
            {
                // If we failed with / without redirection, try the other option:
                hr = ::SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, applyRedirection ? SHGFP_TYPE_DEFAULT : SHGFP_TYPE_CURRENT, stringBuffer);
            }

            if (hr == S_OK)
            {
                rc = 1;
            }
        }
        break;

        case OS_PUBLIC_DOCUMENTS:
        {
            // Get the users "My documents" directory:
            HRESULT hr = ::SHGetFolderPath(NULL, CSIDL_COMMON_DOCUMENTS, NULL, applyRedirection ? SHGFP_TYPE_CURRENT : SHGFP_TYPE_DEFAULT, stringBuffer);

            if (S_OK != hr)
            {
                // If we failed with / without redirection, try the other option:
                hr = ::SHGetFolderPath(NULL, CSIDL_COMMON_DOCUMENTS, NULL, applyRedirection ? SHGFP_TYPE_DEFAULT : SHGFP_TYPE_CURRENT, stringBuffer);
            }

            if (hr == S_OK)
            {
                rc = 1;
            }
        }
        break;

        case OS_USER_DOWNLOADS:
        {

            HRESULT hr = ::SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, applyRedirection ? SHGFP_TYPE_CURRENT : SHGFP_TYPE_DEFAULT, stringBuffer);

            if (S_OK != hr)
            {
                // If we failed with / without redirection, try the other option:
                hr = ::SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, applyRedirection ? SHGFP_TYPE_DEFAULT : SHGFP_TYPE_CURRENT, stringBuffer);
            }

            if (hr == S_OK)
            {
                rc = 1;
                copyBuffer = false;//don't copy buffer on exit
                setFileDirectory(gtString(stringBuffer));
                auto downloads = gtString(OS_USER_DOWNLOAD_DIR);
                appendSubDirectory(downloads);

                osDirectory downloadDir;

                if (getFileDirectory(downloadDir) &&  !downloadDir.exists())
                {
                    if (!downloadDir.create())
                    {
                        rc = 0;
                    }
                }
            }
        }
        break;

        case OS_COMMON_APPLICATION_DATA:
        {
            // Get common (shared across all users) none-roaming application data storage:
            HRESULT hr = ::SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, applyRedirection ? SHGFP_TYPE_CURRENT : SHGFP_TYPE_DEFAULT, stringBuffer);

            if (S_OK != hr)
            {
                // If we failed with / without redirection, try the other option:
                hr = ::SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, applyRedirection ? SHGFP_TYPE_DEFAULT : SHGFP_TYPE_CURRENT, stringBuffer);
            }

            if (hr == S_OK)
            {
                rc = 1;
            }
        }
        break;

        case OS_BROWSE_EXECUTABLES_DIRECTORY:   // In Windows, we start browsing in the current working dir.
        case OS_CURRENT_DIRECTORY:
        {
            // Get Windows current directory:
            rc = ::GetCurrentDirectory(OS_PATH_STRING_BUFFER_SIZE, stringBuffer);
        }
        break;

        default:
        {
            // Unknown pre-defined directory:
            GT_ASSERT(0);
        }
        break;
    }

    if (rc != 0)
    {
        if (copyBuffer)//copy buffer
        {
            _fileDirectory = stringBuffer;
        }

        retVal = true;
    }

    return retVal;
}

bool osFilePath::GetUserAppDataFilePath(osFilePath& userAppDataFilePath, bool applyRedirection)
{
    bool retVal = false;

    wchar_t stringBuffer[OS_PATH_STRING_BUFFER_SIZE];

    // Get per user roaming application data storage:
    // (roaming = should be available to the user as he or she moves between computers within a domain,
    //  see Implementation notes - Article 310294)
    HRESULT hr = ::SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, applyRedirection ? SHGFP_TYPE_CURRENT : SHGFP_TYPE_DEFAULT, stringBuffer);

    if (S_OK != hr)
    {
        // If we failed with / without redirection, try the other option:
        hr = ::SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, applyRedirection ? SHGFP_TYPE_DEFAULT : SHGFP_TYPE_CURRENT, stringBuffer);
        retVal = true;
    }

    retVal = (hr == S_OK);

    userAppDataFilePath.setFileDirectory(stringBuffer);

    return retVal;

}
// ---------------------------------------------------------------------------
// Name:        osFilePath::adjustToCurrentOS
// Description: Adjust the path string to the current operating system.
// Author:      AMD Developer Tools Team
// Date:        15/5/2004
// Usage Sample:
// Implementation notes:
//  We have to adjust the file path to Windows conventions:
//  a. Replace "/" by "\".
//  b. Remove trailing "\" ("c:\foo\\" -> "c:\foo").
//  c. Leave the trailing "\" in case of a drive origin path ("c:\").
// ---------------------------------------------------------------------------
void osFilePath::adjustStringToCurrentOS(gtString& filePathString)
{
    int len = filePathString.length();

    for (int i = 0; i < len; i++)
    {
        if (filePathString[i] == '/')
        {
            filePathString[i] = osPathSeparator;
        }
    }

    // b. Remove trailing "\" ("c:\foo\\" -> "c:\foo").
    bool pathEndedInSeparator = false;
    int filePathLength = filePathString.length();

    if (filePathLength > 0)
    {
        pathEndedInSeparator = (filePathString[filePathLength - 1] == osPathSeparator);
    }

    filePathString.removeTrailing(osPathSeparator);

    if (pathEndedInSeparator)
    {
        filePathString.append(osPathSeparator);
    }

    // c. Leave the trailing "\" in case of a drive origin path ("c:\"):
    if (filePathString.length() == 2)
    {
        if (':' == filePathString[1])
        {
            filePathString.append(osPathSeparator);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        osFilePath::makeWritable
// Description: Make the file write only
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        16/8/2011
// ---------------------------------------------------------------------------
bool osFilePath::makeWritable()
{
    int rc = _wchmod(asString().asCharArray(), _S_IWRITE);
    bool retVal = (rc == 0);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osFilePath::osWStat
// Description: implement windows version os wstat
// Author:      AMD Developer Tools Team
// Date:        19/6/2011
// ---------------------------------------------------------------------------
int osWStat(const gtString& fileName, osStatStructure& fileStruct)
{
    gtString fileNameNoTrailing = fileName;
    fileNameNoTrailing.removeTrailing(osFilePath::osPathSeparator);
    const wchar_t* fileNameStr = fileNameNoTrailing.asCharArray();
    int retVal = _wstat(fileNameStr, &fileStruct);
    const int RETURN_CODE_FOR_SUCCESS = 0;

    if (retVal)
    {
        // _wstat() fails when attempting access to the root folder on a network share.
        // Try the CreateFile API to check if this is the case
        HANDLE hfile = CreateFile(fileNameNoTrailing.asCharArray(), 0, FILE_SHARE_READ, 0, OPEN_EXISTING,  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, 0);

        if (INVALID_HANDLE_VALUE != hfile)
        {
            // The CreateFile API accessed this path successfully
            // Use the GetFileInformationByHandle() API to retrieve the shared folder's information
            BY_HANDLE_FILE_INFORMATION info;
            GetFileInformationByHandle(hfile, &info);
            memset(&fileStruct, 0, sizeof(fileStruct));
            const ULONGLONG unixTimeDiff = 116444736000000000;
            const ULONGLONG unixTimeResolutionDiff = 10000000;
            // We know this is a directory
            fileStruct.st_mode |= _S_IFDIR;
            // Convert Windows FILETIME (100 nanoseconds since 1601) to unix time (seconds since 1970)
            ULONGLONG* pLongNumber = (ULONGLONG*)(&info.ftLastAccessTime);
            fileStruct.st_atime = (*pLongNumber - unixTimeDiff) / unixTimeResolutionDiff;
            pLongNumber = (ULONGLONG*)(&info.ftCreationTime);
            fileStruct.st_ctime = (*pLongNumber - unixTimeDiff) / unixTimeResolutionDiff;
            pLongNumber = (ULONGLONG*)(&info.ftLastWriteTime);
            fileStruct.st_mtime = (*pLongNumber - unixTimeDiff) / unixTimeResolutionDiff;
            // Convert size
            ULONGLONG size = info.nFileSizeHigh;
            size = size << 32;
            size += info.nFileSizeLow;
            fileStruct.st_size = static_cast<_off_t>(size);
            fileStruct.st_dev = info.dwVolumeSerialNumber;
            fileStruct.st_rdev = info.dwVolumeSerialNumber;

            CloseHandle(hfile);
            retVal = RETURN_CODE_FOR_SUCCESS;
        }
    }

    return retVal;
}

bool osFilePath::Rename(const gtString& newNameFullPath)
{
    bool rc = !newNameFullPath.isEmpty();

    osFilePath newFilePath;

    // Check if the new path exists already
    if (rc)
    {
        newFilePath.setFileDirectory(newNameFullPath);
        rc = !newFilePath.exists();
    }

    if (rc)
    {
        gtString currentFileFullPath = _fileDirectory;
        currentFileFullPath += osFilePath::osPathSeparator;
        currentFileFullPath += _fileName;
        currentFileFullPath += L'.';
        currentFileFullPath += _fileExtension;

        rc = (TRUE == MoveFile(currentFileFullPath.asCharArray(), newNameFullPath.asCharArray()));

        if (rc)
        {
            _fileDirectory = newFilePath.fileDirectoryAsString();
            newFilePath.getFileName(_fileName);
            newFilePath.getFileExtension(_fileExtension);
        }
    }

    return rc;
}

void osFilePath::SetSamplePath(osApplicationSpecialDirectories predefinedFilePath)
{
    // The examples root is located in program data folder
    setPath(OS_PUBLIC_DOCUMENTS);

    appendSubDirectory(OS_STR_CodeXLCodeXLExmaplesDirName);
    appendSubDirectory(OS_STR_CodeXLExmaplesDirName);

    switch (predefinedFilePath)
    {
        case osFilePath::OS_CODEXL_EXAMPLES_PATH:
        {
            // Do nothing, set the samples root
        }
        break;

        case OS_CODEXL_TEAPOT_SOURCES_PATH:
        {
            appendSubDirectory(OS_STR_CodeXLTeapotExampleDirName);
            appendSubDirectory(OS_STR_CodeXLTeapotExampleSourceDirName);
        }
        break;

        case OS_CODEXL_TEAPOT_SOURCES_LIB_PATH:
        {
            appendSubDirectory(OS_STR_CodeXLTeapotExampleDirName);
            appendSubDirectory(OS_STR_CodeXLTeapotExampleLibSourceDirName);
            appendSubDirectory(OS_STR_CodeXLSampleSourcesDirName);
        }
        break;

        case OS_CODEXL_TEAPOT_SAMPLE_PATH:
        {
            appendSubDirectory(OS_STR_CodeXLTeapotExampleDirName);
        }
        break;

        case OS_CODEXL_MAT_MUL_SAMPLE_PATH:
        {
            appendSubDirectory(OS_STR_CodeXLMatMulExampleDirName);
        }
        break;

        case OS_CODEXL_D3D_MT_SAMPLE_PATH:
        {
            appendSubDirectory(OS_STR_CodeXLD3D12MultithreadingExampleDirName);
        }
        break;

        default:
            break;
    }
}


bool osFilePath::SetInstallRelatedPath(osApplicationSpecialDirectories predefinedFilePath, bool convertToLower)
{
    bool retVal = false;

    // Get the CodeXL application path:
    osFilePath CodeXLExePath;
    bool rc1 = true;
    bool isDllsDirSet = osGetCurrentApplicationDllsPath(CodeXLExePath);

    if (!isDllsDirSet)
    {
        // If the dlls path is not set (such as in the standalone client), use the current application's path:
        rc1 = osGetCurrentApplicationPath(CodeXLExePath, convertToLower);
    }

    GT_IF_WITH_ASSERT(rc1)
    {
        // In Windows
        // + ... /CodeXL <- application binary location = application path = EULA dir
        // |- Images
        // |- spies <- spies dir
        // |- examples <- examples dir
        // |- Help <- help + tutorial files location

        setFromOtherPath(CodeXLExePath);
        clearFileName();
        clearFileExtension();
        retVal = true;

        switch (predefinedFilePath)
        {

            case OS_CODEXL_BINARIES_PATH:
            case OS_CODEXL_EULA_PATH:
            {
                // Do nothing, we are already there
            }
            break;

            case OS_CODEXL_HELP_FILE:
            {
                appendSubDirectory(OS_STR_CodeXLWindowsHelpFilesDirName);
                setFileName(OS_STR_CodeXLWindowsHelpFileNameNoEXT);
                setFileExtension(OS_STR_CodeXLWindowsHelpFilesExtension);
            }
            break;

            case OS_CODEXL_VS_PACKAGE_HELP_FILE:
            {
                appendSubDirectory(OS_STR_CodeXLWindowsHelpFilesDirName);
                setFileName(OS_STR_CodeXLWindowsHelpFileNameNoEXT);
                setFileExtension(OS_STR_CodeXLWindowsHelpFilesExtension);
            }
            break;

            case OS_CODEXL_VS_PACKAGE_TUTORIAL_FILE:
            {
                appendSubDirectory(OS_STR_CodeXLWindowsHelpFilesDirName);
                setFileName(OS_STR_CodeXLVSPackageWindowsTutorialFileName);
                setFileExtension(OS_STR_CodeXLWindowsHelpFilesExtension);
            }
            break;

            case OS_CODEXL_SERVERS_PATH:
            {
                appendSubDirectory(OS_SPIES_SUB_DIR_NAME);
            }
            break;

            case OS_CODEXL_TUTORIAL_FILE:
            {
                appendSubDirectory(OS_STR_CodeXLWindowsHelpFilesDirName);
                setFileName(OS_STR_CodeXLWindowsQuickStartFileName);
                setFileExtension(OS_STR_CodeXLWindowsQuickStartFileExtension);
            }
            break;

            case OS_CODEXL_QUICK_START_FILE:
            {
                appendSubDirectory(OS_STR_CodeXLWindowsHelpFilesDirName);
                setFileName(OS_STR_CodeXLWindowsQuickStartFileName);
                setFileExtension(OS_STR_CodeXLWindowsQuickStartFileExtension);
            }
            break;

            case OS_CODEXL_DATA_PATH:
            {
                appendSubDirectory(OS_STR_CodeXLDataDirName);
            }
            break;


            case OS_CODEXL_EXAMPLES_PATH:
            case OS_CODEXL_TEAPOT_SOURCES_PATH:
            case OS_CODEXL_TEAPOT_SOURCES_LIB_PATH:
            case OS_CODEXL_TEAPOT_SAMPLE_PATH:
            case OS_CODEXL_MAT_MUL_SAMPLE_PATH:
            case OS_CODEXL_D3D_MT_SAMPLE_PATH:
            {
                SetSamplePath(predefinedFilePath);
            }
            break;

            default:
            {
                // Something's wrong
                GT_ASSERT(false);
                retVal = false;
            }
            break;
        }
    }

    return retVal;
}