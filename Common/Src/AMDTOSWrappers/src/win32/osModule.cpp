//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osModule.cpp
///
//=====================================================================

//------------------------------ osModule.cpp ------------------------------

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <Shlwapi.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osProcess.h>

// ---------------------------------------------------------------------------
// Name:        osLoadModule
// Description: Dynamically load a module (dll / exe / etc) into the current
//              process address space.
// Arguments:   modulePath - The path of the module to be loaded.
//              moduleHandle - Will get the module handle.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/11/2006
// ---------------------------------------------------------------------------
bool osLoadModule(const osFilePath& modulePath, osModuleHandle& moduleHandle, gtString* o_pErrMsg, bool assertOnFail)
{
    bool retVal = false;
    moduleHandle = OS_NO_MODULE_HANDLE;

    // Try to load the module:
    moduleHandle = ::LoadLibrary(modulePath.asString().asCharArray());

    // If we managed to load the module:s
    if (moduleHandle != NULL)
    {
        retVal = true;
    }
    else
    {
        // We failed to load the module:

        // Get the Win API error code:
        DWORD winAPIErrorCode = ::GetLastError();

        // Build an error log message:
        gtString logMessage = OS_STR_FailedToLoadModule;
        logMessage.appendFormattedString(L"%d", winAPIErrorCode);
        gtString moduleName;

        if (modulePath.getFileName(moduleName))
        {
            logMessage.append(L". Module = ").append(moduleName);
        }

        // Trigger an assertion failure:
        GT_ASSERT_EX(!assertOnFail, logMessage.asCharArray())

        // Output debug log message:
        OS_OUTPUT_DEBUG_LOG(logMessage.asCharArray(), assertOnFail ? OS_DEBUG_LOG_INFO : OS_DEBUG_LOG_DEBUG);

        if (NULL != o_pErrMsg)
        {
            *o_pErrMsg = logMessage;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetLoadedModuleHandle
// Description:
//  Retrieves a handle to a module that was already loaded (dynamically
//  or statically) into the current process address space.
//
//  Notice: You should not call osReleaseModule on an handle retrieved by
//          this function.
//
// Arguments: modulePath - A path to the file from which the module was loaded.
//            moduleHandle - Will get the loaded module handle.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        16/4/2007
// ---------------------------------------------------------------------------
bool osGetLoadedModuleHandle(const osFilePath& modulePath, osModuleHandle& moduleHandle)
{
    bool retVal = false;

    // Try to retrieve the module handle:
    moduleHandle = ::GetModuleHandle(modulePath.asString().asCharArray());

    if (moduleHandle != NULL)
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetLoadedModulePath
// Description: Returns the file path of a module, based on its handle
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
bool osGetLoadedModulePath(const osModuleHandle& moduleHandle, osFilePath& modulePath)
{
    bool retVal = false;

    // Get the module path:
    wchar_t pNameBuffer[MAX_PATH + 1] = {0};
    DWORD strLen = ::GetModuleFileName(moduleHandle, pNameBuffer, MAX_PATH);

    if (strLen > 0)
    {
        gtString modulePathAsString = pNameBuffer;
        modulePath.setFullPathFromString(modulePathAsString);

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osReleaseModule
// Description: Release a dynamically load module, loaded by osLoadModule.
// Arguments:   modulePath - The handle of the module to be released.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/11/2006
// ---------------------------------------------------------------------------
bool osReleaseModule(const osModuleHandle& moduleHandle)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(moduleHandle != NULL)
    {
        // Release the loaded module:
        BOOL rc = ::FreeLibrary(moduleHandle);
        GT_IF_WITH_ASSERT(rc != FALSE)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetProcedureAddress
// Description: Returns the memory address of a procedure that resides in a
//              dynamically loaded module.
// Arguments:   modulePath - The handle of the module in which the procedure resides.
//              procedureName - Procedure name.
//              procedureAddress - Will get the procedure address.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/11/2006
// ---------------------------------------------------------------------------
bool osGetProcedureAddress(const osModuleHandle& moduleHandle, const char* procedureName,
                           osProcedureAddress& procedureAddress, bool assertOnFail /* = true */)
{
    bool retVal = false;

    // Try to get the procedure address:
    procedureAddress = (osProcedureAddress)(::GetProcAddress(moduleHandle, procedureName));

    if (procedureAddress != NULL)
    {
        retVal = true;
    }
    else
    {
        // Output a log message with the procedure name:
        gtString dbgMessage;
        dbgMessage.fromASCIIString(procedureName);
        dbgMessage.prepend(L"Cannot retrieve function pointer: ").appendFormattedString(L" (Error code %#x)", ::GetLastError());

        if (assertOnFail)
        {
            GT_ASSERT_EX(false, dbgMessage.asCharArray());
        }
        else
        {
            OS_OUTPUT_DEBUG_LOG(dbgMessage.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetModuleArchitecture
// Description: Gets the module's architectures
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/9/2009
// ---------------------------------------------------------------------------
bool osGetModuleArchitectures(const osFilePath& modulePath, gtVector<osModuleArchitecture>& archs)
{
    bool retVal = false;
    archs.clear();

    // If this is an executable:
    gtString moduleExtension;
    modulePath.getFileExtension(moduleExtension);

    if (moduleExtension.toLowerCase() == L"exe")
    {
        DWORD executableBinaryType = SCS_32BIT_BINARY;
        BOOL retCode = ::GetBinaryType(modulePath.asString().asCharArray(), &executableBinaryType);
        GT_IF_WITH_ASSERT(retCode == TRUE)
        {
            switch (executableBinaryType)
            {
                case SCS_32BIT_BINARY:
                {
                    // This is a 32-bit binary.
                    retVal = true;
                    archs.push_back(OS_I386_ARCHITECTURE);
                }
                break;

                case SCS_64BIT_BINARY:
                {
                    // This is a 64-bit binary.
                    retVal = true;
                    archs.push_back(OS_X86_64_ARCHITECTURE);
                }
                break;

                case SCS_DOS_BINARY:
                case SCS_WOW_BINARY:
                case SCS_PIF_BINARY:
                case SCS_POSIX_BINARY:
                case SCS_OS216_BINARY:
                {
                    // Unsupported binary type.
                    retVal = true;
                    archs.push_back(OS_UNSUPPORTED_ARCHITECTURE);
                }
                break;

                default:
                {
                    // Unknown binary type!
                    GT_ASSERT(false);
                    archs.push_back(OS_UNKNOWN_ARCHITECTURE);
                }
            }
        }

        if (retCode == FALSE)
        {
            // Get the Win API error code:
            DWORD winAPIErrorCode = ::GetLastError();

            gtString msg;
            msg.appendFormattedString(L"GetBinaryType failed. File path: %ls. Win API Error Code: %d", modulePath.asString().asCharArray(), winAPIErrorCode);
            OS_OUTPUT_DEBUG_LOG(msg.asCharArray(), OS_DEBUG_LOG_ERROR);
        }
    }
    else // !modulePath.isExecutable()
    {
        // This is a Dll, check if it's 64-bit:
        // Open the file:
        HANDLE hFile = CreateFile(modulePath.asString().asCharArray(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            // Get its memory mapping:
            HANDLE hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

            if (hFileMapping != NULL)
            {
                // Get the binary data:
                LPVOID lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);

                if (lpFileBase != NULL)
                {
                    // If the MAGIC number is right, this is a DOS (binary) file, otherwise, it's an object/image file:
                    PIMAGE_DOS_HEADER pFileAsDOS = (PIMAGE_DOS_HEADER)lpFileBase;
                    PIMAGE_FILE_HEADER pFileAsImage = NULL;

                    if (pFileAsDOS->e_magic == IMAGE_DOS_SIGNATURE)
                    {
                        // According to the PECOFF spec, this is the offset to the file header:
                        // http://www.microsoft.com/whdc/system/platform/firmware/PECOFF.mspx
                        gtUInt32 fileOffset = *(gtUInt32*)((gtSize_t)lpFileBase + 0x3c);
                        pFileAsImage = (PIMAGE_FILE_HEADER)((gtSize_t)lpFileBase + (gtSize_t)fileOffset + 4);
                    }
                    else // pFileAsDOS->e_magic != IMAGE_DOS_SIGNATURE
                    {
                        // The file header is the start of the file:
                        pFileAsImage = (PIMAGE_FILE_HEADER)lpFileBase;
                    }

                    if (pFileAsImage != NULL)
                    {
                        switch (pFileAsImage->Machine)
                        {
                            case IMAGE_FILE_MACHINE_I386:
                            {
                                // This is a 32-bit binary.
                                retVal = true;
                                archs.push_back(OS_I386_ARCHITECTURE);
                            }
                            break;

                            case IMAGE_FILE_MACHINE_IA64:
                            case IMAGE_FILE_MACHINE_AMD64:
                            {
                                // This is a 64-bit binary.
                                retVal = true;
                                archs.push_back(OS_X86_64_ARCHITECTURE);
                            }
                            break;

                            case IMAGE_FILE_MACHINE_UNKNOWN:
                            case IMAGE_FILE_MACHINE_R3000:
                            case IMAGE_FILE_MACHINE_R4000:
                            case IMAGE_FILE_MACHINE_R10000:
                            case IMAGE_FILE_MACHINE_WCEMIPSV2:
                            case IMAGE_FILE_MACHINE_ALPHA:
                            case IMAGE_FILE_MACHINE_SH3:
                            case IMAGE_FILE_MACHINE_SH3DSP:
                            case IMAGE_FILE_MACHINE_SH3E:
                            case IMAGE_FILE_MACHINE_SH4:
                            case IMAGE_FILE_MACHINE_SH5:
                            case IMAGE_FILE_MACHINE_ARM:
                            case IMAGE_FILE_MACHINE_THUMB:
                            case IMAGE_FILE_MACHINE_AM33:
                            case IMAGE_FILE_MACHINE_POWERPC:
                            case IMAGE_FILE_MACHINE_POWERPCFP:
                            case IMAGE_FILE_MACHINE_MIPS16:
                            case IMAGE_FILE_MACHINE_ALPHA64:
                            case IMAGE_FILE_MACHINE_MIPSFPU:
                            case IMAGE_FILE_MACHINE_MIPSFPU16:

                            // IMAGE_FILE_MACHINE_AXP64 = IMAGE_FILE_MACHINE_ALPHA64
                            case IMAGE_FILE_MACHINE_TRICORE:
                            case IMAGE_FILE_MACHINE_CEF:
                            case IMAGE_FILE_MACHINE_EBC:
                            case IMAGE_FILE_MACHINE_M32R:
                            case IMAGE_FILE_MACHINE_CEE:
                            {
                                // Unsupported binary type.
                                retVal = true;
                                archs.push_back(OS_UNSUPPORTED_ARCHITECTURE);
                            }
                            break;

                            default:
                                GT_ASSERT(false);
                                break;
                        }
                    }

                    // Close the binary:
                    UnmapViewOfFile(lpFileBase);
                }

                // Close the mapping handle:
                CloseHandle(hFileMapping);
            }

            // Close the file handle:
            CloseHandle(hFile);
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        osGetSystemModuleVersionAsString
// Description: Get the module file version as string
// Arguments:   const osFilePath& modulePath
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        8/12/2010
// ---------------------------------------------------------------------------
bool osGetSystemModuleVersionAsString(const gtString& moduleName, gtString& moduleVersion)
{
    bool retVal = false;

    // Get the system directory path:
    osFilePath modulePath(osFilePath::OS_SYSTEM_DIRECTORY);
    modulePath.setFileName(moduleName);
    modulePath.setFileExtension(OS_MODULE_EXTENSION);

    // The DLL does not export DLLGetVersion, use GetFileVersionInfo instead:
    DWORD dwDummy = 0;
    DWORD dwSize  = ::GetFileVersionInfoSize(modulePath.asString().asCharArray(), &dwDummy);

    if (dwSize > 0)
    {
        // Allocate a memory for the version information:
        LPBYTE lpbyVIB = (LPBYTE)malloc(dwSize);


        BOOL rcGetInfo = ::GetFileVersionInfo(modulePath.asString().asCharArray(), 0, dwSize, lpbyVIB);
        GT_IF_WITH_ASSERT(rcGetInfo == TRUE)
        {
            UINT uLen = 0;
            LPVOID lpVSFFI = NULL;

            if (::VerQueryValue(lpbyVIB, L"\\", (LPVOID*)&lpVSFFI, &uLen))
            {
                // Fixed File Info (FFI):
                VS_FIXEDFILEINFO vsffi;
                ::CopyMemory(&vsffi, lpVSFFI, sizeof(VS_FIXEDFILEINFO));
                GT_IF_WITH_ASSERT(vsffi.dwSignature == VS_FFI_SIGNATURE)
                {
                    int majorVersion = HIWORD(vsffi.dwFileVersionMS);
                    int minorVersion = LOWORD(vsffi.dwFileVersionMS);
                    int buildNumber = HIWORD(vsffi.dwFileVersionLS);
                    int fileVersion = LOWORD(vsffi.dwFileVersionLS);
                    moduleVersion.appendFormattedString(L"%d.%d.%d.%d", majorVersion, minorVersion, buildNumber, fileVersion);
                    retVal = true;
                }

                ::ZeroMemory(&vsffi, sizeof(VS_FIXEDFILEINFO));
                free(lpbyVIB);
            }
        }
    }

    return retVal;
}

bool osSearchForModuleInLoaderOrder(const gtString& moduleName, gtVector<osFilePath>& foundPaths)
{
    //1. search the directory from which the application loaded.
    osFilePath filePath;

    if (osGetCurrentApplicationPath(filePath, false))
    {
        filePath.setFileName(moduleName);
        filePath.setFileExtension(OS_MODULE_EXTENSION);

        if (filePath.exists())
        {
            foundPaths.push_back(filePath);
        }
    }

    // 2. search the system directory
    wchar_t systemDir[OS_MAX_PATH];

    if (0 != GetSystemDirectory(systemDir, OS_MAX_PATH - 1))
    {
        filePath.setFileDirectory(systemDir);

        if (filePath.exists())
        {
            foundPaths.push_back(filePath);
        }
    }

    // 3. search the 16-bit system directory
    filePath.setFileDirectory(L"c:\\Windows\\System");

    if (filePath.exists())
    {
        foundPaths.push_back(filePath);
    }

    // 4. search the Windows directory
    wchar_t windowsDir[OS_MAX_PATH];

    if (0 != GetWindowsDirectory(windowsDir, OS_MAX_PATH - 1))
    {
        filePath.setFileDirectory(windowsDir);

        if (filePath.exists())
        {
            foundPaths.push_back(filePath);
        }
    }

    // 5. search the current directory
    wchar_t curDir[OS_MAX_PATH];

    if (0 != GetCurrentDirectory(OS_MAX_PATH - 1, curDir))
    {
        filePath.setFileDirectory(curDir);

        if (filePath.exists())
        {
            foundPaths.push_back(filePath);
        }
    }

    // 6, search the directories that are listed in the 'PATH' environment variable
    gtString envPath;

    if (osGetCurrentProcessEnvVariableValue(L"PATH", envPath))
    {
        gtString seperator = osFilePath::osEnvironmentVariablePathsSeparator;

        // split the 'PATH' environment variables string by the seperator, and check if the dll exist in each path
        gtStringTokenizer tokenizer(envPath, seperator);
        gtString curEnvVal;

        while (tokenizer.getNextToken(curEnvVal))
        {
            filePath.setFileDirectory(curEnvVal);

            if (filePath.exists())
            {
                gtVector<osFilePath>::iterator it = std::find(foundPaths.begin(), foundPaths.end(), filePath);

                // if the same path found - don't add it again
                if (it == foundPaths.end())
                {
                    foundPaths.push_back(filePath);
                }
            }
        }
    }

    return (foundPaths.size() > 0);
}
