//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osModule.cpp
///
//=====================================================================

//------------------------------ osModule.cpp ------------------------------

// POSIX:
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

#if AMDT_BUILD_TARGET != AMDT_LINUX_OS
#error "Attempting to build Linux specific code for non-Linux target"
#endif

// Linux-only headers:
#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
    #include <elf.h>
    #include <link.h>
#endif

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osModule.h>


// ---------------------------------------------------------------------------
// Name:        osLoadModule
// Description: Dynamically load a module (shared library / etc) into the current
//              process address space.
// Arguments:   modulePath - The path of the module to be loaded.
//              modulePath - Will get the module handle.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/11/2006
// ---------------------------------------------------------------------------
bool osLoadModule(const osFilePath& modulePath, osModuleHandle& moduleHandle, gtString* o_pErrMsg, bool assertOnFail)
{
    bool retVal = false;

    // Try to load the module:
    std::string utf8Path;
    modulePath.asString().asUtf8(utf8Path);
    moduleHandle = ::dlopen(utf8Path.c_str(), RTLD_NOW | RTLD_LOCAL);

    // If we failed to load the module:
    if (moduleHandle == NULL)
    {
        // Get the POSIX API error:
        gtString posixErrorString;
        posixErrorString.fromASCIIString(::dlerror());

        // Build an error log message:
        gtString logMessage = OS_STR_FailedToLoadModule;
        logMessage += posixErrorString;
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
    else
    {
        retVal = true;
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
    // From dlopen man page:
    // RTLD_NOLOAD - Don't load the library.  This can be used to test if the library is
    //               already resident  (dlopen()  returns NULL if it is not, or the library's
    //               handle if it is resident).
    std::string utf8Path;
    modulePath.asString().asUtf8(utf8Path);
    moduleHandle = ::dlopen(utf8Path.c_str(), RTLD_LAZY | RTLD_NOLOAD | RTLD_LOCAL);

    if (moduleHandle != NULL)
    {
        retVal = true;
    }
    else
    {
        // Log the error:
        gtString posixErrorString;
        posixErrorString.fromASCIIString(::dlerror());
        OS_OUTPUT_DEBUG_LOG(posixErrorString.asCharArray(), OS_DEBUG_LOG_ERROR);
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

#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT

    struct link_map* lmap = nullptr;

    // Prefer to query the linkmap over RTLD_DI_ORIGIN so we don't have to
    // worry about memory allocations or potentially work around PATHMAX.
    int rcInfo = ::dlinfo(moduleHandle, RTLD_DI_LINKMAP, &lmap);
    if (0 == rcInfo)
    {
        gtString pathStr;
        pathStr.fromASCIIString(lmap->l_name);
        modulePath = pathStr;

        retVal = true;
    }
    else // 0 != rcInfo
    {
        gtString errMsg;
        errMsg.fromASCIIString(::dlerror());

        GT_ASSERT_EX(false, errMsg.asCharArray());
    }

#else // AMDT_LINUX_VARIANT != AMDT_GENERIC_LINUX_VARIANT
    GT_UNREFERENCED_PARAMETER(moduleHandle);
    GT_UNREFERENCED_PARAMETER(modulePath);

    // TO_DO: not implemented for MacOS due to lack of RTLD_DI_LINKMAP.
    // Support should be fairly straightforward to add though.
#endif // AMDT_LINUX_VARIANT

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osReleaseModule
// Description: Relase a dynamically load module, loaded by osLoadModule.
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
        int rc = ::dlclose(moduleHandle);
        GT_IF_WITH_ASSERT(rc == 0)
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
    procedureAddress = (osProcedureAddress)(::dlsym(moduleHandle, procedureName));

    if (procedureAddress != NULL)
    {
        retVal = true;
    }
    else
    {
        // Output a log message with the procedure name:
        gtString dbgMessage;
        dbgMessage.fromASCIIString(procedureName);
        dbgMessage.prepend(L"Cannot retrieve function pointer: ");

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

//////////////////////////////////////////////////////////////////////////
// The mac version of this function is implemented in src/mac/osModule.cpp
//////////////////////////////////////////////////////////////////////////
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)
// ---------------------------------------------------------------------------
// Name:        osGetModuleArchitecture
// Description: Inputs a module and outputs the module's architectures
// Arguments: modulePath - The input module.
//            archs - Will get the module architecture (Generic Linux supports a single architecture per module)
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        5/8/2010
// ---------------------------------------------------------------------------
bool osGetModuleArchitectures(const osFilePath& modulePath, gtVector<osModuleArchitecture>& archs)
{
    bool retVal = false;
    archs.clear();

    // Verify that the file is an executable file:
    bool isExecutableFile = modulePath.isExecutable();
    GT_IF_WITH_ASSERT(isExecutableFile)
    {
        // Open the input module file for binary reading:
        std::string utf8Path;
        modulePath.asString().asUtf8(utf8Path);
        int fdModule = open(utf8Path.c_str(), O_RDONLY);
        GT_IF_WITH_ASSERT(fdModule != -1)
        {
            // Read the module's ELF header
            size_t elfHeaderSize = sizeof(Elf32_Ehdr);
            gtUByte* fileHeaderBuff = (u_char*)malloc(elfHeaderSize);

            size_t readBytes = read(fdModule, fileHeaderBuff, elfHeaderSize);
            GT_IF_WITH_ASSERT(readBytes == elfHeaderSize)
            {
                // Verify that we are handling an ELF file:
                Elf32_Ehdr* pELFHeader = (Elf32_Ehdr*)fileHeaderBuff;
                bool isELFHeaderValid = (pELFHeader->e_ident[EI_MAG0] == 0x7F) &&
                                        (pELFHeader->e_ident[EI_MAG1] == 'E') &&
                                        (pELFHeader->e_ident[EI_MAG2] == 'L') &&
                                        (pELFHeader->e_ident[EI_MAG3] == 'F');
                GT_IF_WITH_ASSERT(isELFHeaderValid)
                {
                    // Get the ELF file's required architecture:
                    Elf32_Half elfMachineType = pELFHeader->e_machine;

                    // If the ELF file's required architecture is Intel I386:
                    if (elfMachineType == EM_386)
                    {
                        archs.push_back(OS_I386_ARCHITECTURE);
                        retVal = true;
                    }

                    // If the ELF file's required architecture is AMD X86_64:
                    if (elfMachineType == EM_X86_64)
                    {
                        archs.push_back(OS_X86_64_ARCHITECTURE);
                        retVal = true;
                    }
                }
            }

            // Release the header buffer:
            free(fileHeaderBuff);
            fileHeaderBuff = NULL;

            // Close the input module file:
            close(fdModule);
            fdModule = -1;
        }
    }

    return retVal;
}

#endif

