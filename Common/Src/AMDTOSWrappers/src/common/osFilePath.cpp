//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osFilePath.cpp
///
//=====================================================================

//------------------------------ osFilePath.cpp ------------------------------

// Standard C:
#include <string.h>

// POSIX:
#include <sys/stat.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osTime.h>

// Local:
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// The below file status macros are defined on Linux, but not on Windows:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define __S_ISTYPE(mode, mask)  (((mode) & _S_IFMT) == (mask))
    #define S_ISDIR(mode) __S_ISTYPE((mode), _S_IFDIR)
    #define S_ISREG(mode) __S_ISTYPE((mode), _S_IFREG)
    #define S_ISCHR(mode) __S_ISTYPE((mode), S_IFCHR)
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    #define OS_MAC_APPLICATION_BUNDLE_SUFFIX ".app"
#endif

// Static members initialization:
bool osFilePath::ms_supportUnicodeInUserAppData = true;
osFilePath osFilePath::ms_userAppDataFilePath = osFilePath();

const wchar_t* SPACE = L" ";

// ---------------------------------------------------------------------------
// Name:        osFilePath::osFilePath
// Description: Constructor - Initialize to contain an empty path.
// Author:      AMD Developer Tools Team
// Date:        27/5/2004
// ---------------------------------------------------------------------------
osFilePath::osFilePath()
{
}


// ---------------------------------------------------------------------------
// Name:        osFilePath::osFilePath
// Description: Constructor - Creates a a pre-defined file path.
//                            directory.
// Arguments:   directory - The input pre-defined file path.
// Author:      AMD Developer Tools Team
// Date:        15/5/2004
// ---------------------------------------------------------------------------
osFilePath::osFilePath(osPreDefinedFilePaths predefinedfilePath, bool applyRedirection)
{
    bool rc = setPath(predefinedfilePath, applyRedirection);
    GT_ASSERT(rc);
}

osFilePath::osFilePath(osApplicationSpecialDirectories predefinedFilePath, bool convertToLower)
{
    bool rc = SetInstallRelatedPath(predefinedFilePath, convertToLower);
    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        osFilePath::osFilePath
// Description: Constructor. Inputs a file or directory full path.
//              Example: "C:\TEMP\foo.txt"
// Author:      AMD Developer Tools Team
// Date:        15/5/2004
// ---------------------------------------------------------------------------
osFilePath::osFilePath(const gtString& fileFullPath, bool adjustToOS /* = true*/)
{
    setFullPathFromString(fileFullPath, adjustToOS);
}

// ---------------------------------------------------------------------------
// Name:        osFilePath::osFilePath
// Description: Sets file path from file name + extension
// Arguments:   predefinedfilePath - the file directory enumeration
//              fileName - the file name
//              fileExtension - the file extension
// Author:      AMD Developer Tools Team
// Date:        5/5/2011
// ---------------------------------------------------------------------------
osFilePath::osFilePath(osPreDefinedFilePaths predefinedfilePath, const gtString& fileName, const gtString& fileExtension, bool applyRedirection)
{
    bool rc = setPath(predefinedfilePath, applyRedirection);
    GT_ASSERT(rc);

    _fileName = fileName;
    _fileExtension = fileExtension;
}


osFilePath::osFilePath(const osFilePath& other) : _fileDirectory(other._fileDirectory),
    _fileName(other._fileName),
    _fileExtension(other._fileExtension),
    _fileFullPathString(other._fileFullPathString)
{
}


#if AMDT_HAS_CPP0X

osFilePath::osFilePath(osFilePath&& other) : _fileDirectory(std::move(other._fileDirectory)),
    _fileName(std::move(other._fileName)),
    _fileExtension(std::move(other._fileExtension)),
    _fileFullPathString(std::move(other._fileFullPathString))
{
}


osFilePath& osFilePath::operator=(osFilePath&& other)
{
    if (this != &other)
    {
        _fileDirectory = std::move(other._fileDirectory);
        _fileName = std::move(other._fileName);
        _fileExtension = std::move(other._fileExtension);
        _fileFullPathString = std::move(other._fileFullPathString);
    }

    return *this;
}

#endif // AMDT_HAS_CPP0X


bool osFilePath::operator<(const osFilePath& otherFile) const
{
    gtString str = asString();
    str.toLowerCase();
    gtString otherStr = otherFile.asString();
    otherStr.toLowerCase();
    bool retVal = (str < otherStr);
    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        osFilePath::setFullPathFromString
// Description: Assignment operator.
// Arguments:   fullPathAsString - A path string from which this path will be built.
// Author:      AMD Developer Tools Team
// Date:        7/1/2007
// ---------------------------------------------------------------------------
osFilePath& osFilePath::setFullPathFromString(const gtString& fullPathAsString, bool adjustToOS)
{
    gtString fileDirectoryTemp;
    gtString fileNameTemp;
    gtString fileExtensionTemp;

    if (!fullPathAsString.isEmpty())
    {
        //////////////////////////////////////////////////////////////////////////
        // Look for the last osPathSeparator:
        int lastPathSaperatorPos = fullPathAsString.reverseFind(osPathSeparator);
        // Look for the last osExtensionSeparator:
        int lastExtensionSeparatorPos = fullPathAsString.reverseFind(osExtensionSeparator);

        //////////////////////////////////////////////////////////////////////////
        // Get the file Extension
        //////////////////////////////////////////////////////////////////////////
        // Sanity test:
        if (lastPathSaperatorPos < lastExtensionSeparatorPos)
        {
            // Return the string that starts at the osExtensionSeparator:
            int stringLength = fullPathAsString.length();
            fullPathAsString.getSubString(lastExtensionSeparatorPos + 1, stringLength - 1, fileExtensionTemp);
        }

        //////////////////////////////////////////////////////////////////////////
        // Get the file Name
        //////////////////////////////////////////////////////////////////////////
        // If we found an osExtensionSeparator:
        if ((lastExtensionSeparatorPos != -1) && (lastPathSaperatorPos < lastExtensionSeparatorPos))
        {
            // The file name is the string that starts with the last osPathSeparator
            // and ends with the osExtensionSeparator:
            // Example: "c:\temp\foo.txt" - the file name is "foo"
            fullPathAsString.getSubString(lastPathSaperatorPos + 1, lastExtensionSeparatorPos - 1, fileNameTemp);
        }
        else
        {
            // There is no osExtensionSeparator

            // Use the string that appears after the last osPathSeparator
            // as the file name:
            fullPathAsString.getSubString(lastPathSaperatorPos + 1, fullPathAsString.length() - 1, fileNameTemp);
        }

        //////////////////////////////////////////////////////////////////////////
        // Get the file Directory
        //////////////////////////////////////////////////////////////////////////
        // If we found an osPathSeparator:
        if (lastPathSaperatorPos != -1)
        {
            // The directory path is the string that ends with the last osPathSeparator:
            // Example: "c:\temp\foo.txt" - the path is "c:\temp"
            fullPathAsString.getSubString(0, lastPathSaperatorPos - 1, fileDirectoryTemp);
        }
    }
    else // when fullPathAsString is null string
    {
        // No processing needed.
        // No need for adjustToOS since file path fullPathAsString is null string
        adjustToOS = false;
    }

    // Set the values to the class members:
    _fileDirectory = fileDirectoryTemp;
    _fileName = fileNameTemp;
    _fileExtension = fileExtensionTemp;

    if (adjustToOS)
    {
        // Adjust the path to our current OS conventions:
        adjustToCurrentOS();
    }

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        osFilePath::operator=
// Description: Assignment operator.
// Arguments:   other - A path that will be assigned into this path.
// Author:      AMD Developer Tools Team
// Date:        29/6/2004
// ---------------------------------------------------------------------------
osFilePath& osFilePath::operator=(const osFilePath& other)
{
    // Copy the other path path string:
    _fileDirectory = other._fileDirectory;
    _fileName = other._fileName;
    _fileExtension = other._fileExtension;

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        osFilePath::asString
// Description: Returns the file path as string.
// Author:      AMD Developer Tools Team
// Date:        7/1/2007
// ---------------------------------------------------------------------------
const gtString& osFilePath::asString(bool appendSeparatorToDir) const
{
    osFilePath* pToSelfNonConst = (osFilePath*)this;

    pToSelfNonConst->_fileFullPathString = _fileDirectory;

    if (appendSeparatorToDir && (!_fileDirectory.isEmpty()))
    {
        pToSelfNonConst->_fileFullPathString.append(osPathSeparator);
    }

    if (!_fileName.isEmpty())
    {
        if ((!appendSeparatorToDir) && (!_fileDirectory.isEmpty()))
        {
            pToSelfNonConst->_fileFullPathString.append(osPathSeparator);
        }

        pToSelfNonConst->_fileFullPathString.append(_fileName);
    }

    if (!_fileExtension.isEmpty())
    {
        pToSelfNonConst->_fileFullPathString.append(osExtensionSeparator);
        pToSelfNonConst->_fileFullPathString.append(_fileExtension);
    }

    return _fileFullPathString;
}

// ---------------------------------------------------------------------------
// Name:        osFilePath::exists
// Description: Returns true iff this file path exists on the storage device
//              (disk / other).
// Author:      AMD Developer Tools Team
// Date:        1/11/2006
// ---------------------------------------------------------------------------
bool osFilePath::exists() const
{
    bool retVal = false;
    const gtString& fileFullPath = asString();

    // Try to get the file status:
    osStatStructure fileStatus;
    int rc = osWStat(fileFullPath.asCharArray(), fileStatus);

    if (rc == 0)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osFilePath::clear
// Description: Clears this file path to contain an empty path.
// Author:      AMD Developer Tools Team
// Date:        12/11/2007
// ---------------------------------------------------------------------------
osFilePath& osFilePath::clear()
{
    _fileDirectory.makeEmpty();
    _fileName.makeEmpty();
    _fileExtension.makeEmpty();
    _fileFullPathString.makeEmpty();
    return *this;
}


// ---------------------------------------------------------------------------
// Name:        osFilePath::isDirectory
// Description: Returns true iff this file path exists and represents a directory.
// Author:      AMD Developer Tools Team
// Date:        4/11/2006
// ---------------------------------------------------------------------------
bool osFilePath::isDirectory() const
{
    bool retVal = false;
    const gtString& fileFullPath = asString();

    // Try to get the file status:
    osStatStructure fileStatus;
    int rc = osWStat(fileFullPath.asCharArray(), fileStatus);

    if (rc == 0)
    {
        // If this is a directory:
        if (S_ISDIR(fileStatus.st_mode))
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osFilePath::isRegularFile
// Description: Returns true iff this file path exists and represents a
//              regular file (not a device / socket / etc).
// Author:      AMD Developer Tools Team
// Date:        4/11/2006
// ---------------------------------------------------------------------------
bool osFilePath::isRegularFile() const
{
    bool retVal = false;
    const gtString& fileFullPath = asString();

    // Try to get the file status:
    osStatStructure fileStatus;
    int rc = osWStat(fileFullPath.asCharArray(), fileStatus);

    if (rc == 0)
    {
        // If this is a directory:
        if (S_ISREG(fileStatus.st_mode) || S_ISCHR(fileStatus.st_mode))
        {
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osFilePath::isExecutable
// Description: Returns true iff this file path exists and represents an
//              executable.
// Author:      AMD Developer Tools Team
// Date:        9/12/2008
// ---------------------------------------------------------------------------
bool osFilePath::isExecutable() const
{
    bool retVal = isRegularFile();

    // In Mac OS X, we also support application bundles, which are in fact directories
    // with a .app in the end:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    {
        if (!retVal)
        {
            if (isDirectory())
            {
                const gtString& fullPath = asString();
                int l = fullPath.length();
                int suffixLastPosition = fullPath.reverseFind(OS_MAC_APPLICATION_BUNDLE_SUFFIX);

                // Note that we expect the suffix to be 4 chars from the end:
                // "/Applications/Foo.app"
                //                   4321L
                if ((suffixLastPosition != -1) && (suffixLastPosition == l - 4))
                {
                    retVal = true;
                }
            }
        }
    }
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osFilePath::getFileDirectory
// Description:
//   Returns the file containing directory path.
//   Example: "c:\temp\foo.txt" - the containing directory path is "c:\temp"
// Arguments:   fileDirectory - Will get the file containing directory path.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        27/5/2004
// ---------------------------------------------------------------------------
bool osFilePath::getFileDirectory(osDirectory& fileDirectory) const
{
    bool retVal = false;

    if (!_fileDirectory.isEmpty())
    {
        osFilePath fileDirectoryPath;
        fileDirectoryPath.setFileDirectory(_fileDirectory);
        fileDirectory.setDirectoryPath(fileDirectoryPath);

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osFilePath::getFileName
// Description:
//   Returns the file name (without the path and extension).
//   Example: "c:\temp\foo.txt" - the file name is "foo".
// Arguments:   fileName - Will get the file name.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        27/5/2004
// ---------------------------------------------------------------------------
bool osFilePath::getFileName(gtString& fileName) const
{
    bool retVal = false;

    if (!_fileName.isEmpty())
    {
        fileName = _fileName;

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osFilePath::getFileExtension
// Description:
//   Returns the file extension.
//   Example: The extension of "c:\temp\foo.txt" is "txt".
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        15/5/2004
// ---------------------------------------------------------------------------
bool osFilePath::getFileExtension(gtString& fileExtension) const
{
    bool retVal = false;

    if (!_fileExtension.isEmpty())
    {
        fileExtension = _fileExtension;

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osFilePath::getFileNameAndExtension
// Description: Sets fileNameWithExt to the file full name, e.g. "foo.txt"
// Author:      AMD Developer Tools Team
// Date:        15/9/2010
// ---------------------------------------------------------------------------
void osFilePath::getFileNameAndExtension(gtString& fileNameWithExt) const
{
    // Add the name:
    fileNameWithExt = _fileName;

    // If we have an extension, add it and a separator:
    if (!_fileExtension.isEmpty())
    {
        fileNameWithExt.append(osExtensionSeparator).append(_fileExtension);
    }
}

// ---------------------------------------------------------------------------
// Name:        osFilePath::setFileDirectory
// Description: Sets the file directory.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        27/5/2004
// ---------------------------------------------------------------------------
osFilePath& osFilePath::setFileDirectory(const gtString& fileDirectory)
{
    _fileDirectory = fileDirectory;

    // Adjust the path to the current os:
    adjustToCurrentOS();

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        osFilePath::setFileDirectory
// Description: Sets the file directory.
// Arguments:   path - The new file path (With or without the file name and extension).
// Author:      AMD Developer Tools Team
// Date:        27/5/2004
// ---------------------------------------------------------------------------
osFilePath& osFilePath::setFileDirectory(const osDirectory& fileDirectory)
{
    setFileDirectory(fileDirectory.directoryPath().asString(true));

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        osFilePath::appendSubDirectory
// Description: Appends a sub directory relative path to the current path.
// Author:      AMD Developer Tools Team
// Date:        16/5/2004
// Usage Sample:
//     osFilePath appDataPath(osFilePath::OS_USER_APPLICATION_DATA);
//     appDataPath.appendSubDirectory("MyApplication");
//
//     Will yield the path:
//     "C:\Documents and Settings\User1\Application Data\MyApplication"
// ---------------------------------------------------------------------------
osFilePath& osFilePath::appendSubDirectory(const gtString& subDirRelativePathString)
{
    if (!_fileDirectory.endsWith(osPathSeparator))
    {
        _fileDirectory.append(osPathSeparator);
    }

    _fileDirectory.append(subDirRelativePathString);

    // Adjust the path to the current os:
    adjustToCurrentOS();

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        osFilePath::setFileName
// Description: Sets the file name of this file path.
//              If the path didn't contain a file extension - the file extension
//              is set to .txt.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        27/5/2004
// ---------------------------------------------------------------------------
osFilePath& osFilePath::setFileName(const gtString& fileName)
{
    _fileName = fileName;

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        osFilePath::setFileExtension
// Description: Sets the file extension of this file path.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        27/5/2004
// ---------------------------------------------------------------------------
osFilePath& osFilePath::setFileExtension(const gtString& fileExtension)
{
    _fileExtension = fileExtension;

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        osFilePath::setFromOtherPath
// Description: Allows to selectively copy only parts of another osFilePath.
//              Calling this with true, true, true is the same as operator= .
// Arguments: setDirectory - copy other's directory to me
//            setName - copy other's file name to me
//            setExtension - copy other's file extension to me
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        9/11/2009
// ---------------------------------------------------------------------------
osFilePath& osFilePath::setFromOtherPath(const osFilePath& other, bool setDirectory, bool setName, bool setExtension)
{
    if (setDirectory)
    {
        _fileDirectory = other._fileDirectory;
    }

    if (setName)
    {
        _fileName = other._fileName;
    }

    if (setExtension)
    {
        _fileExtension = other._fileExtension;
    }

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        osFilePath::reinterpretAsDirectory
// Description: Reinterprets the file path as a directory, so if it was "C:\myFolder\mySubfolder"
//              (supposedly a file), it is now "C:\myFolder\mySubfolder\".
//              This is equivalent to taking the path, converting it to a string,
//              adding a path separator, and converting back to osFilePath.
//              If the path was already in this format (i.e. no file name / extension)
//              this function does not change it.
// Author:      AMD Developer Tools Team
// Date:        4/5/2010
// ---------------------------------------------------------------------------
osFilePath& osFilePath::reinterpretAsDirectory()
{
    // Do processing if at least one of _fileName or _fileDirectory is non-empty
    if ((!_fileDirectory.isEmpty()) || (!_fileName.isEmpty()))
    {
        // Make the file directory field contain the other two fields (separated by an extension separator if needed:
        _fileDirectory.removeTrailing(osPathSeparator).append(osPathSeparator).append(_fileName);

        if (!_fileExtension.isEmpty())
        {
            // Only add the extension separator to the file name if there is an extension:
            _fileDirectory.append(osExtensionSeparator).append(_fileExtension);
        }

        // Clear the other fields:
        _fileName.makeEmpty();
        _fileExtension.makeEmpty();

        // Remove the trailing path separator if needed:
        _fileDirectory.removeTrailing(osPathSeparator);

        // Adjust to the current OS:
        adjustToCurrentOS();
    }

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        osFilePath::resolveToAbsolutePath
// Author:      AMD Developer Tools Team
// Date:        9/2/2016
// ---------------------------------------------------------------------------
osFilePath& osFilePath::resolveToAbsolutePath()
{
    adjustToCurrentOS();

    // If the path is relative, make it absolute:
    gtString dirPathAsAbsolute = _fileDirectory;

    if (isRelativePath())
    {
        osFilePath workDir(OS_CURRENT_DIRECTORY);
        dirPathAsAbsolute.prepend(osPathSeparator).prepend(workDir.asString());
    }

    gtVector<gtString> subdirStack;

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    // On linux, we want a path that starts with "/", so we initialize the stack
    // with an empty string:
    subdirStack.push_back(L"");
#endif

    // Tokenize the absolute path, handling .. and . :
    static const gtString delims = osPathSeparator;
    gtStringTokenizer pathTokenizer(dirPathAsAbsolute, delims);
    gtString tok;

    while (pathTokenizer.getNextToken(tok))
    {
        if (L".." == tok)
        {
            // Up one level:
            if (1 < subdirStack.size())
            {
                subdirStack.pop_back();
            }
        }
        else if ((L"." == tok) || (tok.isEmpty()))
        {
            // Do nothing to the stack
        }
        else
        {
            // Append the subdirectory:
            subdirStack.push_back(tok);
        }
    }

    // Now create the output string:
    GT_IF_WITH_ASSERT(0 < subdirStack.size())
    {
        _fileDirectory.makeEmpty();

        for (const gtString& sd : subdirStack)
        {
            _fileDirectory.append(sd).append(osPathSeparator);
        }

        // If the path starts with the path separator, we do not want to remove it.
        if (1 < _fileDirectory.length())
        {
            // Remove the trailing slash:
            _fileDirectory.removeTrailing(osPathSeparator);
        }
    }

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        osFilePath::type
// Description: Returns my transferable object type.
// Author:      AMD Developer Tools Team
// Date:        17/8/2004
// ---------------------------------------------------------------------------
osTransferableObjectType osFilePath::type() const
{
    return OS_TOBJ_ID_FILE_PATH;
}


// ---------------------------------------------------------------------------
// Name:        osFilePath::writeSelfIntoChannel
// Description: Writes self into a channel.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        17/8/2004
// ---------------------------------------------------------------------------
bool osFilePath::writeSelfIntoChannel(osChannel& channel) const
{
    channel << _fileDirectory;
    channel << _fileName;
    channel << _fileExtension;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        osFilePath::readSelfFromChannel
// Description: Read self from a channel.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        17/8/2004
// ---------------------------------------------------------------------------
bool osFilePath::readSelfFromChannel(osChannel& channel)
{
    channel >> _fileDirectory;
    channel >> _fileName;
    channel >> _fileExtension;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        osFilePath::isWritable
// Description: Does the file have write permission?
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        16/8/2011
// ---------------------------------------------------------------------------
bool osFilePath::isWritable() const
{
    bool retVal = false;

    // Get the file full path as string:
    const gtString& fileFullPath = asString();

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // Try to get the file attributes of this file:
    DWORD fileAttributes = GetFileAttributes(fileFullPath.asCharArray());

    // Verify that this is a directory:
    if (fileAttributes & FILE_ATTRIBUTE_READONLY)
    {
        retVal = false;
    }
    else
    {
        retVal = true;
    }

#else
    // Try to get the file status:
    osStatStructure fileStatus;
    int rc = osWStat(fileFullPath.asCharArray(), fileStatus);

    if (rc == 0)
    {
        if (__S_ISTYPE(fileStatus.st_mode, S_IWRITE))
        {
            retVal = true;
        }
    }

#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        adjustToCurrentOS
// Description: Adjusts a file path to the current OS using adjustStringToCurrentOS
// Author:      AMD Developer Tools Team
// Date:        4/12/2011
// ---------------------------------------------------------------------------
osFilePath& osFilePath::adjustToCurrentOS()
{
    // Get the full path as string (so if the file name has any other platforms'
    // path separators, we'll get it).
    gtString filePathAsString = asString(true);

    // Adjust it:
    adjustStringToCurrentOS(filePathAsString);

    // Set it into ourselves:
    setFullPathFromString(filePathAsString, false);

    return *this;
}

void osFilePath::InitializeUnicodeCharactersUserFilePath(bool applyRedirection)
{
    /// Was the app data path unicode characters checked?
    static bool sIsUnicodeUserInitialized = false;

    if (!sIsUnicodeUserInitialized)
    {

        osFilePath appDataFilePath;
        bool rc = GetUserAppDataFilePath(appDataFilePath, applyRedirection);
        GT_IF_WITH_ASSERT(rc)
        {
            const char* pPathAsUtf8 = appDataFilePath.asString().asUTF8CharArray();
            const char* pPathAsChars = appDataFilePath.asString().asASCIICharArray();
            int rcCmp = strcmp(pPathAsChars, pPathAsUtf8);

            /// Does the current user app data path contain unicode characters?
            static bool sIsUnicodeUser = false;

            if (rcCmp != 0)
            {
                sIsUnicodeUser = true;
            }

            if (sIsUnicodeUser)
            {
                ms_userAppDataFilePath = osFilePath(OS_TEMP_DIRECTORY);
            }
            else
            {
                ms_userAppDataFilePath = appDataFilePath;
            }

            sIsUnicodeUserInitialized = true;

            // Print the user app data folder location to the log file:
            gtString message;
            message.appendFormattedString(L"User app data folder. Original location: %ls. Current location: %ls", appDataFilePath.asString().asCharArray(), ms_userAppDataFilePath.asString().asCharArray());
            OS_OUTPUT_DEBUG_LOG(message.asCharArray(), OS_DEBUG_LOG_INFO);

        }
    }
}

// ---------------------------------------------------------------------------
// Name:        osGenerateUniqueFileName
// Description: Inputs a directory path, file name prefix and an extension and
//              generates a unique file name (in the input directory) by appending
//              the current time and few letters to the file name.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/8/2005
// ---------------------------------------------------------------------------
bool osGenerateUniqueFileName(const osFilePath& directoryPath, const gtString& fileNamePrefix,
                              const gtString& fileExtension, osFilePath& uniqueFileNamePath)
{
    bool retVal = false;

    // Initialize the file name to contain the file name prefix:
    gtString fileName = fileNamePrefix;

    // Get the current time and date as strings:
    osTime curretTime;
    curretTime.setFromCurrentTime();

    gtString dateAsString;
    curretTime.dateAsString(dateAsString, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);

    gtString timeAsString;
    curretTime.timeAsString(timeAsString, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);

    // Append the date and time to the file name:
    fileName += L"-";
    fileName += dateAsString;
    fileName += L"-";
    fileName += timeAsString;

    // Set the output unique file path name and extension:
    uniqueFileNamePath.setFileDirectory(directoryPath);
    uniqueFileNamePath.setFileName(fileName);
    uniqueFileNamePath.setFileExtension(fileExtension);

    // If the file path exists:
    bool filePathExists = uniqueFileNamePath.exists();

    if (filePathExists)
    {
        // Try adding it chars (one at a time):
        for (int i = 0; i < 20; i++)
        {
            fileName += L"-";
            uniqueFileNamePath.setFileName(fileName);

            // If we managed to generate a unique file name:
            filePathExists = uniqueFileNamePath.exists();

            if (!filePathExists)
            {
                break;
            }
        }
    }

    // We succeeded iff we managed to create a unique file name:
    retVal = !filePathExists;

    return retVal;
}


// ---------------------------------------------------------------------------
bool osFilePath::IsMatchingExtension(const gtString& extensionsString) const
{
    bool retVal = false;
    gtString currentExtension;
    gtStringTokenizer strTokenizer(extensionsString, SPACE);

    while (strTokenizer.getNextToken(currentExtension))
    {
        if (_fileExtension.compare(currentExtension) == 0)
        {
            retVal = true;
            break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        ConvertCygwinPath
// Description: Convert a CygWin drive Linux-style path to Windows style path
//              NOTE: This is a static function
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        
// ---------------------------------------------------------------------------
bool osFilePath::ConvertCygwinPath(const wchar_t* pPath, int len, gtString& convertedPath)
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    bool ret = (10 < len && 0 == memcmp(pPath, L"/cygdrive/", 10 * sizeof(wchar_t)));

    if (ret)
    {
        pPath += 10;
        len -= 9;
        convertedPath.resize(static_cast<size_t>(len));

        int i = 0;

        while (L'/' != *pPath)
        {
            convertedPath[i++] = *pPath++;
        }

        convertedPath[i++] = L':';
        convertedPath[i++] = L'\\';

        ++pPath;

        for (; i < len; ++i, ++pPath)
        {
            convertedPath[i] = (L'/' == *pPath) ? L'\\' : *pPath;
        }
    }

    return ret;

#else

    GT_UNREFERENCED_PARAMETER(pPath);
    GT_UNREFERENCED_PARAMETER(len);
    GT_UNREFERENCED_PARAMETER(convertedPath);

    return false;

#endif
}

// ---------------------------------------------------------------------------
// Name:        ConvertCygwinPath
// Description: Convert a CygWin drive Linux-style path to Windows style path
//              NOTE: This is a static function
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        
// ---------------------------------------------------------------------------
bool osFilePath::ConvertCygwinPath(const gtString& path, gtString& convertedPath)
{
    return ConvertCygwinPath(path.asCharArray(), path.length(), convertedPath);
}
