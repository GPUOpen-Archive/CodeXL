//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Auxil.cpp
/// \brief  Implements the initial framework interface for the CodeAnalyst component
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/Auxil.cpp#34 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================

#ifdef PROTO_ONLY
    #include <string.h>
#else
    #include <memory.h>
    #include <assert.h>
#endif

// Qt:
#include <QtCore>
#include <QtWidgets>
#include <QDir>
#include <QFileDialog>
#include <QProgressDialog>

// AMDTOSWrappers:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

//Local
#include "inc/Auxil.h"
#include <inc/CpuProfilingOptions.h>


void AuxGetSymbolSearchPath(gtString& searchPath, gtString& serverList, gtString& cachePath)
{
    gtUInt32 dUseSymServer = 0;

    PROFILE_OPTIONS* pao = CpuProfilingOptions::instance().options();

    if (pao->addDebug)
    {
        searchPath = acQStringToGTString(pao->debugSearchPaths);
    }

    QString symServerList;

    // check symbol server options
    if (pao->enableSymServer)
    {
        dUseSymServer = pao->useSymSrvMask;
        symServerList = pao->symSrvList;
        cachePath = acQStringToGTString(pao->symbolDownloadDir);
    }

    serverList.makeEmpty();

    if (!symServerList.isEmpty() && 0 != dUseSymServer)
    {
        QStringList servers = symServerList.split(';', QString::SkipEmptyParts);

        for (int i = 0; i < servers.size(); ++i)
        {
            if ((dUseSymServer & (1 << i)) != 0)
            {
                serverList += acQStringToGTString(servers.at(i));
                serverList += L';';
            }
        }
    }
}


bool AuxInitializeSymbolEngine(ExecutableFile* pExecutable)
{
    gtString searchPath;
    gtString serverList;
    gtString cachePath;

    AuxGetSymbolSearchPath(searchPath, serverList, cachePath);

    const wchar_t* pSearchPath = (!searchPath.isEmpty()) ? searchPath.asCharArray() : nullptr;
    const wchar_t* pServerList = (!serverList.isEmpty()) ? serverList.asCharArray() : nullptr;
    const wchar_t* pCachePath = (nullptr != pServerList && !cachePath.isEmpty()) ? cachePath.asCharArray() : nullptr;
    return pExecutable->InitializeSymbolEngine(pSearchPath, pServerList, pCachePath);
}


// the purpose of this API is for 64-bit module under \windows\system32
bool AuxFileExists(QString fileNamePath)
{
    bool bExists = false;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    PVOID oldValue = nullptr;
    BOOL doRedirect = false;
    IsWow64Process(GetCurrentProcess(), &doRedirect);

    if (doRedirect)
    {
        doRedirect = (BOOL) Wow64DisableWow64FsRedirection(&oldValue);
    }

#endif
    bExists = QFile::exists(fileNamePath);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    if (doRedirect)
    {
        Wow64RevertWow64FsRedirection(oldValue);
    }

#endif

    return bExists;
}

bool AuxGetExecutablePath(QString& exePath,
                          CpuProfileReader& profileReader,
                          const QString& sessionDir,
                          const QString& processPath,
                          QWidget* pParent,
                          CpuProfileModule* pModule)
{
    gtString processPathGt = acQStringToGTString(processPath);

    bool ret = (nullptr != pModule || nullptr != (pModule = profileReader.getModuleDetail(processPathGt)));

    if (ret)
    {
        exePath = processPath;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        // In Windows the path is not case sensitive
        gtString profiledPath = profileReader.getProfileInfo()->m_targetPath;
        profiledPath.toLowerCase();
        processPathGt.toLowerCase();
#else
        const gtString& profiledPath = profileReader.getProfileInfo()->m_targetPath;
#endif

        // Only cache the profiled executable with unmanaged code
        if (profiledPath == processPathGt && CpuProfileModule::UNMANAGEDPE == pModule->m_modType)
        {
            // Look for a cached file
            bool cached = GetCachedFile(sessionDir, processPath, exePath);

            exePath.replace('/', PATH_SLASH);

            if (!AuxFileExists(exePath))
            {
                // Popup the user to find the missing file if needed
                if (nullptr != pParent)
                {
                    exePath = FindModuleFile(pParent, exePath);
                }

                if (!exePath.isEmpty())
                {
                    exePath.replace('/', PATH_SLASH);
                    CacheRelocatedSource(sessionDir, processPath, exePath, true, true);
                }
                else
                {
                    ret = false;
                }
            }
            else if (!cached)
            {
                // Automatically cache files that aren't in the system directory
                static const osFilePath sysPath(osFilePath::OS_SYSTEM_DIRECTORY);
                static const osFilePath sysx86Path(osFilePath::OS_SYSTEM_X86_DIRECTORY);
                static const QString sysPathQStr = acGTStringToQString(sysPath.asString());
                static const QString sysx86PathQStr = acGTStringToQString(sysx86Path.asString());

                if (!(exePath.startsWith(sysPathQStr) || exePath.startsWith(sysx86PathQStr)))
                {
                    CacheRelocatedSource(sessionDir, processPath, exePath, true, true);

                    // Use the now cached file instead of the original
                    GetCachedFile(sessionDir, processPath, exePath);
                }
            }
        }
        else
        {
            exePath.replace('/', PATH_SLASH);
        }
    }

    return ret;
}

void AuxAppendSampleName(QString& sampleName, gtVAddr va)
{
    char buf[32];
    sprintf(buf, "0x%08llx", static_cast<unsigned long long>(va));
    sampleName.append(buf);
}

bool AuxGetParentFunctionName(const CpuProfileModule* pModule, const CpuProfileFunction* pFunction, gtVAddr va, QString& name)
{
    if (nullptr != pModule && nullptr == pFunction)
    {
        pFunction = pModule->findFunction(va);
    }

    bool ret = !(nullptr == pFunction ||
                 pFunction->getFuncName().isEmpty() ||
                 (nullptr != pModule && pModule->isUnchartedFunction(*pFunction)));

    if (ret)
    {
        name = acGTStringToQString(pFunction->getFuncName());
    }
    else
    {
        if (nullptr != pFunction && !pFunction->getFuncName().isEmpty())
        {
            name = acGTStringToQString(pFunction->getFuncName());
        }
        else if (nullptr != pModule)
        {
            gtString modFile;

            if (pModule->extractFileName(modFile))
            {
                name = acGTStringToQString(modFile);
                name += '!';
            }
        }

        AuxAppendSampleName(name, va);
    }

    return ret;
}


static bool IsWindowsSystemModuleNoExt(const gtString& absolutePath)
{
    bool ret = false;

    // 21 is the minimum of: "\\windows\\system\\*.***"
    if (absolutePath.length() >= 21)
    {
        gtString lowerAbsolutePath = absolutePath;

        for (int i = 0, e = lowerAbsolutePath.length(); i != e; ++i)
        {
            wchar_t& wc = lowerAbsolutePath[i];

            if ('/' == wc)
            {
                wc = '\\';
            }
            else
            {
                // Paths in windows are case insensitive
                wc = tolower(wc);
            }
        }

        int rootPos = lowerAbsolutePath.find(L"\\windows\\");

        if (-1 != rootPos)
        {
            // 9 is the length of "\\windows\\"
            rootPos += 9;

            if (lowerAbsolutePath.compare(rootPos, 3, L"sys") == 0)
            {
                rootPos += 3;

                if (lowerAbsolutePath.compare(rootPos, 4, L"tem\\")   == 0 || // "\\windows\\system\\"
                    lowerAbsolutePath.compare(rootPos, 6, L"tem32\\") == 0 || // "\\windows\\system32\\"
                    lowerAbsolutePath.compare(rootPos, 6, L"wow64\\") == 0)   // "\\windows\\syswow64\\"
                {
                    ret = true;
                }
            }
            else
            {
                if (lowerAbsolutePath.compare(rootPos, 7, L"winsxs\\") == 0)
                {
                    ret = true;
                }
            }
        }
    }

    return ret;
}

static bool IsLinuxSystemModuleNoExt(const gtString& absolutePath)
{
    // Kernel samples
    bool ret = (absolutePath.find(L"[kernel.kallsyms]") != -1);

    if (!ret && L'/' == absolutePath[0])
    {
        if (absolutePath.compare(1, 3, L"lib") == 0)
        {
            ret = true;
        }
        else
        {
            if (absolutePath.compare(1, 4, L"usr/") == 0)
            {
                if (absolutePath.compare(5, 3,  L"lib")       ||
                    absolutePath.compare(5, 9,  L"local/lib") ||
                    absolutePath.compare(5, 10, L"share/gdb") == 0)
                {
                    ret = true;
                }
            }
        }
    }

    return ret;
}

// This function tries to tell whether a given module name is a Windows system library.
//
// We look for ':' (C:...) or name ending with ".dll", ".sys" or ".exe".
// If so, look at the path to see whether it includes "\\Windows\\" or contains "\\Sys*" or "\\winsxs\\".
//
bool AuxIsWindowsSystemModule(const gtString& absolutePath)
{
    bool ret;

    if (absolutePath.length() > 4 && (absolutePath.endsWith(L".dll") ||
                                      absolutePath.endsWith(L".sys") ||
                                      absolutePath.endsWith(L".exe")))
    {
        ret = IsWindowsSystemModuleNoExt(absolutePath);
    }
    else
    {
        ret = false;
    }

    return ret;
}

// This function tries to tell whether a given module name is a Linux system library.
//
// The special name "[kernel.kallsyms]" is the module name for samples within the kernel.
// Then, if the path does not start with '/' we assume it's not a system library.
// The name must then start with "lib" and have ".so" within it.
// If so, we consider these files to be system libraries if they are from:
//          /lib*
//          /usr/lib*
//          /usr/local/lib*
//          /usr/share/gdb*
//
bool AuxIsLinuxSystemModule(const gtString& absolutePath)
{
    bool ret;

    int len = absolutePath.length();

    if (len > 3 && 0 == memcmp(absolutePath.asCharArray() + len - 3, L".so", 3 * sizeof(wchar_t)))
    {
        ret = IsLinuxSystemModuleNoExt(absolutePath);
    }
    else
    {
        ret = false;
    }

    return ret;
}

bool AuxIsSystemModule(const gtString& absolutePath)
{
    bool ret;

    if (absolutePath.length() > 4 && (absolutePath.endsWith(L".dll") ||
                                      absolutePath.endsWith(L".sys") ||
                                      absolutePath.endsWith(L".exe")))
    {
        ret = IsWindowsSystemModuleNoExt(absolutePath);
    }
    else
    {
        ret = AuxIsLinuxSystemModule(absolutePath);
    }

    return ret;
}

bool AuxIsSystemModule(const osFilePath& modulePath)
{
    return AuxIsSystemModule(modulePath.asString());
}
