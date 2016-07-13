//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osModule.h
///
//=====================================================================

//------------------------------ osModule.h ------------------------------

#ifndef __OSMODULE
#define __OSMODULE

// Forward declarations:
class osFilePath;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osModuleArchitecture.h>

OS_API bool osLoadModule(const osFilePath& modulePath, osModuleHandle& moduleHandle, gtString* o_pErrMsg = NULL, bool assertOnFail = true);
OS_API bool osGetLoadedModuleHandle(const osFilePath& modulePath, osModuleHandle& moduleHandle);
OS_API bool osGetLoadedModulePath(const osModuleHandle& moduleHandle, osFilePath& modulePath);
OS_API bool osReleaseModule(const osModuleHandle& moduleHandle);
OS_API bool osGetProcedureAddress(const osModuleHandle& moduleHandle, const char* procedureName, osProcedureAddress& procedureAddress, bool assertOnFail = true);
OS_API bool osGetModuleArchitectures(const osFilePath& modulePath, gtVector<osModuleArchitecture>& archs);
OS_API bool osIs64BitModule(const osFilePath& modulePath, bool& is64Bit);
OS_API void osGetSystemOpenGLModulePath(gtVector<osFilePath>& systemOGLModulePaths);
OS_API bool osGetSystemOpenCLModulePath(gtVector<osFilePath>& systemOCLModulePaths);
OS_API bool osGetSystemModuleVersionAsString(const gtString& moduleName, gtString& moduleVersion);

///Simple RAII class for load/unload library
class OS_API osModule
{
public:
    osModule();
    virtual ~osModule();

    bool loadModule(const osFilePath& modulePath, gtString* o_pErrMsg = nullptr, bool assertOnFail = true);
    const osModuleHandle& GetModuleHandle() const { return m_moduleHandle; };

private:
    void unloadModule();

private:
    osModuleHandle m_moduleHandle;
};

/// check if the moduleName dll file exist in searched paths.
/// if it exists - the path will be added to the foundPaths list.
/// the searched paths:
/// 1.  The directory from which the application loaded.
/// 2.  The system directory.Use the GetSystemDirectory function to get the path of this directory.
/// 3.  The 16 - bit system directory.There is no function that obtains the path of this directory, but it is searched.
/// 4.  The Windows directory.Use the GetWindowsDirectory function to get the path of this directory.
/// 5.  The current directory.
/// 6.  The directories that are listed in the PATH environment variable
/// \param moduleName is the dll name
/// \param foundPaths is the list of the paths the dll exist in
/// \return true if the foundPaths list is not empty (list size > 0)
OS_API bool osSearchForModuleInLoaderOrder(const gtString& moduleName, gtVector<osFilePath>& foundPaths);

// Module constructor and destructor functions markers:
// (Currently supported by Linux and Mac only)
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define OS_MODULE_CONSTRUCTOR __attribute__((constructor))
    #define OS_MODULE_DESTRUCTOR __attribute__((destructor))
#endif

#endif  // __OSMODULE
