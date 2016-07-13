//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osModule.cpp
///
//=====================================================================

//------------------------------ osModule.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtSet.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>

// Local:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>

// Mac only:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    #include <AMDTOSWrappers/Include/osBundle.h>
#endif


// ---------------------------------------------------------------------------
// Name:        osIs64BitModule
// Description: sets is64Bit to display whether modulePath points to a 64-bit
//              module or a 32-bit one.
//              In the case of Mac multi-architecture binaries, we choose the
//              largest address space among intel architectures.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/9/2009
// ---------------------------------------------------------------------------
bool osIs64BitModule(const osFilePath& modulePath, bool& is64Bit)
{
    bool retVal = false;

    gtVector<osModuleArchitecture> moduleArchs;
    bool rcArchs = osGetModuleArchitectures(modulePath, moduleArchs);
    GT_IF_WITH_ASSERT(rcArchs)
    {
        int numberOfArchs = (int)moduleArchs.size();

        for (int i = 0; i < numberOfArchs; i++)
        {
            osModuleArchitecture currentArch = moduleArchs[i];

            switch (currentArch)
            {
                case OS_I386_ARCHITECTURE:
                {
                    // If we haven't found another architecture yet:
                    if (!retVal)
                    {
                        // Mark this as a 32-bit module:
                        retVal = true;
                        is64Bit = false;
                    }
                }
                break;

                case OS_X86_64_ARCHITECTURE:
                {
                    // If the binary has both 32 and 64-bit architectures, mark it as 64-bit:
                    retVal = true;
                    is64Bit = true;
                }
                break;

                default:
                {
                    // Do nothing...
                }
                break;
            }
        }
    }

    // Assert if we found no matching architecture:
    GT_RETURN_WITH_ASSERT(retVal);
}

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
static void osGetEtcConfGLPaths(gtVector<osFilePath>& systemOGLModulePathStrings)
{
    // Optimization, we do only once during process run scan etc conf files, thus in consecutive calls of the function we use already read results
    static bool readEtcConfFiles = false;
    static gtSet<osFilePath> systemOGLModulePathStringsInner;

    // If this is the first time this is called, read etc conf files:
    if (!readEtcConfFiles)
    {
        osFilePath etcConfPath(L"/etc/ld.so.conf.d/");
        osDirectory etcConfDir(etcConfPath);
        gtList<osFilePath> filePaths;
        bool rc1 = etcConfDir.getContainedFilePaths(L"*GL*.conf",    osDirectory::SORT_BY_DATE_ASCENDING, filePaths       );
        GT_ASSERT(rc1);
        bool rc2 = etcConfDir.getContainedFilePaths(L"*fglrx*.conf", osDirectory::SORT_BY_DATE_ASCENDING, filePaths, false);
        GT_ASSERT(rc2);
        bool rc3 = etcConfDir.getContainedFilePaths(L"*amd*.conf",   osDirectory::SORT_BY_DATE_ASCENDING, filePaths, false);
        GT_ASSERT(rc3);

        // Remove duplicates
        filePaths.sort();
        filePaths.unique();

        for (const osFilePath& glConfPath : filePaths)
        {
            osFile file(glConfPath);
            file.open(osChannel::OS_ASCII_TEXT_CHANNEL);
            gtASCIIString line;
            while (file.readLine(line))
            {
                if (!line.isEmpty())
                {
                   gtString glPath;
                   glPath.fromASCIIString(line.asCharArray()).append(L"/" OS_OPENGL_MODULE_NAME);
                   systemOGLModulePathStringsInner.insert(glPath);
                }
            }
        }

        readEtcConfFiles = true;
    }

    // Add the pre-calculated paths:
    systemOGLModulePathStrings.reserve(systemOGLModulePathStrings.size() + systemOGLModulePathStringsInner.size());
    for (const osFilePath& fp : systemOGLModulePathStringsInner)
    {
        systemOGLModulePathStrings.push_back(fp);
    }
}
#endif // ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))

// ---------------------------------------------------------------------------
// Name:        osGetSystemOpenGLModulePath
// Description: Calculates the System's OpenGL module (DLL / shared library / framework / etc) path.
// Arguments: systemOGLModulePath - Will get the system's OpenGL module path.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        11/4/2010
// ---------------------------------------------------------------------------
void osGetSystemOpenGLModulePath(gtVector<osFilePath>& systemOGLModulePaths)
{
    // If the SA special environment variable is set, use it:
    gtString systemModuleDirs;

    if (osGetCurrentProcessEnvVariableValue(OS_STR_envVar_systemModulesDirs, systemModuleDirs))
    {
        gtStringTokenizer strTok(systemModuleDirs, osFilePath::osEnvironmentVariablePathsSeparator);
        gtString currentToken;

        while (strTok.getNextToken(currentToken))
        {
            if (!currentToken.isEmpty())
            {
                gtString currentModulePath = currentToken;
                currentModulePath.append(osFilePath::osPathSeparator).append(OS_OPENGL_MODULE_NAME);
                systemOGLModulePaths.push_back(osFilePath(currentModulePath));
            }
        }
    }

    // Will get the system's OpenGL module path:
    gtString systemOGLModulePathStr;

    // On Mac OS X:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    {
        // If we are on iPhone platforms:
#ifdef _GR_IPHONE_BUILD
        {
            // Get the OpenGL ES framework path:
            systemOGLModulePathStr = osGetOpenGLESFrameworkPath();
            systemOGLModulePathStr += osFilePath::osPathSeparator;
            systemOGLModulePathStr += OS_OPENGL_ES_MODULE_NAME;
        }
#else
        {
            // We are on the Mac desktop platform:

            // Get the OpenGL framework path:
            osFilePath oglFramewrokDirPath(OS_OPENGL_FRAMEWORK_PATH);

            // Build the OpenGL module path:
            systemOGLModulePathStr = oglFramewrokDirPath.asString();
            systemOGLModulePathStr += osFilePath::osPathSeparator;
            systemOGLModulePathStr += OS_OPENGL_MODULE_NAME;
        }
#endif

        systemOGLModulePaths.push_back(systemOGLModulePathStr);
    }
#else // ! AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
    {
        // If we are being debugged and our parent specified a known system
        // library path then prefer their path over using the system lookup
        // rules.
        //
        // They may have purtubed our environment in the process of
        // implementing library interposition. This lets them pass us the real
        // path without us needing to (poorly) implement the system's library
        // resolution rules.
        //
        // Specifically, this won't break Linux systems which use /etc/ld.so.conf
        // to prioritise library paths outside of the typical system
        // directories.
        gtString glModulePathStrFromEnvVar;
        bool rcEnv = osGetCurrentProcessEnvVariableValue(OS_STR_envVar_suSystemOpenGLModulePath, glModulePathStrFromEnvVar);
        if (rcEnv)
        {
            osFilePath glModulePathFromEnvVar(glModulePathStrFromEnvVar);
            systemOGLModulePaths.push_back(glModulePathFromEnvVar);
        }

        // Next, look for an environment variable that contains the OpenGL driver path:
        const gtString AmdGpuDriverPathEnvVariable(L"AMDGPU_LIBGL_PATH");
        const gtString GLDriverPathEnvVariable(L"LIBGL_DRIVERS_PATH");
        gtVector<gtString> amdGpuDriverPaths;
        gtVector<gtString> glDriverPaths;
        gtString delimiters(osFilePath::osEnvironmentVariablePathsSeparator);
        bool hasAmdGpuDriverPathEnvVariable = osTokenizeCurrentProcessEnvVariableValue(AmdGpuDriverPathEnvVariable, delimiters, amdGpuDriverPaths);
        bool hasGLDriverPathEnvVariable = osTokenizeCurrentProcessEnvVariableValue(GLDriverPathEnvVariable, delimiters, glDriverPaths);

        // If we have such an environment variable - use it's content as the system's OpenGL module's directory:
        if (hasGLDriverPathEnvVariable || hasAmdGpuDriverPathEnvVariable)
        {
            // Merge the lists of paths from the 2 environment variables
            glDriverPaths.insert(glDriverPaths.begin(), amdGpuDriverPaths.begin(), amdGpuDriverPaths.end());

            // On 64-bit linux machines, this is set to /usr/lib/dri:/usr/lib64/dri. We must search both paths:
            for (gtString libPath : glDriverPaths)
            {
                libPath.removeTrailing(osFilePath::osPathSeparator).append(osFilePath::osPathSeparator).append('.').append('.');

                // Build the OpenGL module path:
                libPath += osFilePath::osPathSeparator;
                libPath += OS_OPENGL_MODULE_NAME;

                // Verify whether that file exists:
                osFilePath systemOGLModulePathAsFilePath(libPath);

                if (systemOGLModulePathAsFilePath.isRegularFile())
                {
                    systemOGLModulePaths.push_back(systemOGLModulePathAsFilePath);
                }
            }
        }

        // Add the system folders as fallback location:
        osFilePath oglModuleDirectoryPath(osFilePath::OS_SYSTEM_DIRECTORY);
        systemOGLModulePathStr = oglModuleDirectoryPath.asString();

        // Build the OpenGL module path:
        systemOGLModulePathStr += osFilePath::osPathSeparator;
        systemOGLModulePathStr += OS_OPENGL_MODULE_NAME;
        systemOGLModulePaths.push_back(systemOGLModulePathStr);

        // Get the x86 system directory path:
        osFilePath systemx86DirPath(osFilePath::OS_SYSTEM_X86_DIRECTORY);

        // If the two folders are different:
        if (systemx86DirPath != oglModuleDirectoryPath)
        {
            systemOGLModulePathStr = systemx86DirPath.asString();

            // Build the OpenGL module path:
            systemOGLModulePathStr += osFilePath::osPathSeparator;
            systemOGLModulePathStr += OS_OPENGL_MODULE_NAME;
            systemOGLModulePaths.push_back(systemOGLModulePathStr);
        }

        // DEV463567: On Linux, add a few more hard-coded fallback locations, in case the LIBGL_DRIVERS_PATH or AMDGPU_LIBGL_PATH are not set correctly.
        // This is ugly and I don't like it =(
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
        {
            static const gtVector<gtString> systemOGLModulePathStrings =
            {
                // Ubuntu locations for AMD open-source driver
                L"/usr/lib/x86_64-linux-gnu/amdgpu-pro/" OS_OPENGL_MODULE_NAME,

                // Red-Hat locations for AMD open-source driver
                L"/usr/lib64/amdgpu-pro/" OS_OPENGL_MODULE_NAME,

                // Locations of the AMD closed-source driver
                L"/usr/lib/fglrx/" OS_OPENGL_MODULE_NAME,
                L"/usr/lib/fglrx/dri/" OS_OPENGL_MODULE_NAME,

                // Locations of the mesa driver
                L"/usr/lib/x86_64-linux-gnu/" OS_OPENGL_MODULE_NAME,
                L"/usr/lib/x86_64-linux-gnu/mesa/" OS_OPENGL_MODULE_NAME
            };

            systemOGLModulePaths.insert(systemOGLModulePaths.end(), systemOGLModulePathStrings.begin(), systemOGLModulePathStrings.end());

            // Add additional paths from etc conf
            osGetEtcConfGLPaths(systemOGLModulePaths);
        }
#endif
    }
#endif // AMDT_BUILD_TARGET
}


// ---------------------------------------------------------------------------
// Name:        osGetSystemOpenCLModulePath
// Description: Calculates the System's OpenCL module (DLL / shared library / framework / etc) path.
// Arguments: systemOCLModulePath - Will get the system's OpenGL module path.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        11/4/2010
// ---------------------------------------------------------------------------
bool osGetSystemOpenCLModulePath(gtVector<osFilePath>& systemOCLModulePaths)
{
    bool retVal = false;
    systemOCLModulePaths.clear();

    // If the SA special environment variable is set, use it:
    gtString systemModuleDirs;

    if (osGetCurrentProcessEnvVariableValue(OS_STR_envVar_systemModulesDirs, systemModuleDirs))
    {
        gtStringTokenizer strTok(systemModuleDirs, osFilePath::osEnvironmentVariablePathsSeparator);
        gtString currentToken;

        while (strTok.getNextToken(currentToken))
        {
            if (!currentToken.isEmpty())
            {
                gtString currentModulePath1 = currentToken;
                currentModulePath1.append(osFilePath::osPathSeparator).append(OS_OPENCL_ICD_MODULE_NAME);
                systemOCLModulePaths.push_back(osFilePath(currentModulePath1));
                gtString currentModulePath2 = currentToken;
                currentModulePath2.append(osFilePath::osPathSeparator).append(OS_OPENCL_ICD_MODULE_ALTERNATIVE_NAME);
                systemOCLModulePaths.push_back(osFilePath(currentModulePath2));
            }
        }
    }

    // On Mac OS X:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    {
        gtString systemOCLModulePathStr;

        // If we are on iPhone platforms:
#ifdef _GR_IPHONE_BUILD
        {
            // OpenCL ES is not yet available on the iPhone device:
            GT_ASSERT(false);
            retVal = false;

            // Get the OpenCL ES framework path:
            // systemOCLModulePathStr = osGetOpenCLESFrameworkPath();
            // systemOCLModulePathStr += osFilePath::osPathSeparator;
            // systemOCLModulePathStr += OS_OPENCL_ES_MODULE_NAME;
        }
#else
        {
            // We are on the Mac desktop platform:

            // Get the OpenCL framework path:
            osFilePath oclFramewrokDirPath(OS_OPENCL_FRAMEWORK_PATH);

            // Build the OpenCL module path:
            systemOCLModulePathStr = oclFramewrokDirPath.asString();
            systemOCLModulePathStr += osFilePath::osPathSeparator;
            systemOCLModulePathStr += OS_OPENCL_ICD_MODULE_NAME;

            retVal = true;
        }
#endif

        osFilePath systemOCLModulePath = systemOCLModulePathStr;
        systemOCLModulePaths.push_back(systemOCLModulePath);
    }
#else // ! AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
    {
        // Get the system directory path:
        osFilePath systemDirPath(osFilePath::OS_SYSTEM_DIRECTORY);

        // Build the OpenCL module paths:
        gtString systemOCLModulePath1Str = systemDirPath.asString();
        systemOCLModulePath1Str += osFilePath::osPathSeparator;
        systemOCLModulePath1Str += OS_OPENCL_ICD_MODULE_NAME;
        osFilePath systemOCLModulePath1 = systemOCLModulePath1Str;
        systemOCLModulePaths.push_back(systemOCLModulePath1);

        gtString systemOCLModulePath2Str = systemDirPath.asString();
        systemOCLModulePath2Str += osFilePath::osPathSeparator;
        systemOCLModulePath2Str += OS_OPENCL_ICD_MODULE_ALTERNATIVE_NAME;
        osFilePath systemOCLModulePath2 = systemOCLModulePath2Str;
        systemOCLModulePaths.push_back(systemOCLModulePath2);

        // Get the x86 system directory path:
        osFilePath systemx86DirPath(osFilePath::OS_SYSTEM_X86_DIRECTORY);

        // If the two folders are different:
        if (systemx86DirPath != systemDirPath)
        {
            // Build the OpenCL module paths:
            gtString systemOCLModulePath3Str = systemx86DirPath.asString();
            systemOCLModulePath3Str += osFilePath::osPathSeparator;
            systemOCLModulePath3Str += OS_OPENCL_ICD_MODULE_NAME;
            osFilePath systemOCLModulePath3 = systemOCLModulePath3Str;
            systemOCLModulePaths.push_back(systemOCLModulePath3);

            gtString systemOCLModulePath4Str = systemx86DirPath.asString();
            systemOCLModulePath4Str += osFilePath::osPathSeparator;
            systemOCLModulePath4Str += OS_OPENCL_ICD_MODULE_ALTERNATIVE_NAME;
            osFilePath systemOCLModulePath4 = systemOCLModulePath4Str;
            systemOCLModulePaths.push_back(systemOCLModulePath4);
        }

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
        {
            gtString systemOCLModulePathStr = L"/usr/lib32/fglrx/" OS_OPENCL_ICD_MODULE_NAME;
            systemOCLModulePaths.push_back(systemOCLModulePathStr);
            systemOCLModulePathStr = L"/usr/lib/fglrx/" OS_OPENCL_ICD_MODULE_NAME;
            systemOCLModulePaths.push_back(systemOCLModulePathStr);

            systemOCLModulePathStr = L"/usr/lib/x86_64-linux-gnu/amdgpu-pro/" OS_OPENCL_ICD_MODULE_NAME;
            systemOCLModulePaths.push_back(systemOCLModulePathStr);
        }
#endif
        retVal = true;
    }
#endif // AMDT_BUILD_TARGET

    return retVal;
}

osModule::osModule()
: m_moduleHandle(nullptr)
{

}

osModule::~osModule()
{
    unloadModule();
}

bool osModule::loadModule(const osFilePath& modulePath, gtString* o_pErrMsg/* = nullptr*/, bool assertOnFail/* = true*/)
{
    unloadModule();

    bool retVal = osLoadModule(modulePath, m_moduleHandle, o_pErrMsg, assertOnFail);

    return retVal;
}

void osModule::unloadModule()
{
    if (nullptr != m_moduleHandle)
    {
        bool rcRel = osReleaseModule(m_moduleHandle);
        GT_ASSERT(rcRel);
        m_moduleHandle = nullptr;
    }
}
