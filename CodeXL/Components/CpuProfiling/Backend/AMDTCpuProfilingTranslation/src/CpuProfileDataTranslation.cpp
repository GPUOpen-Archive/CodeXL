//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileDataTranslation.cpp
/// \brief CPU profile data translation interface implementation.
///
//==================================================================================

#include <CpuProfileDataTranslation.h>
#include <CpuProfileDataMigrator.h>

#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osDirectory.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // Windows only stuff
    #include <CXLTaskInfo/inc/TaskInfoInterface.h>
    #include "Windows/PrdTranslator.h"
#else
    // Linux only stuff
    #include <unistd.h>
    #include "Linux/CaPerfTranslator.h"
#endif


gtList<ReaderHandle*> g_validReaders;

//returns true if valid and false if not
static bool helpCheckValidReader(const ReaderHandle* pReaderHandle)
{
    bool found = false;

    for (const auto& it : g_validReaders)
    {
        if (it == pReaderHandle)
        {
            found = true;
            break;
        }
    }

    return found;
}


static HRESULT helpValidatePath(const wchar_t* pPath)
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#else
    char path[OS_MAX_PATH] = {'\0'};

    if (0 >= wcstombs(path, pPath, OS_MAX_PATH - 1))
    {
        return E_INVALIDARG;
    }
#endif

    //Checking for existence
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    if (-1 == _waccess(pPath, 0))
#else
    if (-1 == access(path, F_OK))
#endif
    {
        return E_NOFILE;
    }

    // Checking for read
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    if (-1 == _waccess(pPath, 04))
#else
    if (-1 == access(path, R_OK))
#endif
    {
        return E_ACCESSDENIED;
    }

    return S_OK;
}


#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
static HRESULT helpHandlePrdFile(const wchar_t* pFilePath, ReaderHandle** pReaderHandle)
{
    HRESULT hr = S_OK;

    // Create the PrdTranslator
    gtString envVal;
    bool collectStats = osGetCurrentProcessEnvVariableValue(L"CODEXL_CPU_TRANS_STATS", envVal);
    gtString path = pFilePath;

    PrdTranslator* pDataTranslator = new PrdTranslator(pFilePath, collectStats);

    if (nullptr == pDataTranslator)
    {
        return E_OUTOFMEMORY;
    }

    *pReaderHandle = static_cast<ReaderHandle*>(pDataTranslator);
    g_validReaders.push_back(*pReaderHandle);

    return hr;
}
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
static HRESULT helpHandleCaPerfFile(const wchar_t* pFilePath, ReaderHandle** pReaderHandle)
{
    HRESULT hr = S_OK;
    gtString path = pFilePath;

    // Create CaPerfTranslator
    CaPerfTranslator* pTrans = new CaPerfTranslator(path.asASCIICharArray());

    if (NULL == pTrans)
    {
        return E_OUTOFMEMORY;
    }

    *pReaderHandle = static_cast<ReaderHandle*>(pTrans);
    g_validReaders.push_back(*pReaderHandle);

    return hr;
}
#endif  // AMDT_BUILD_TARGET == AMDT_LINUX_OS

// Note: This function currently handles: prd, caperf, ebp files
HRESULT fnOpenProfile(
    /*in*/ const wchar_t* pPath,
    /*out*/ ReaderHandle** pReaderHandle)
{
    HRESULT hr = E_UNEXPECTED;

    if ((NULL == pReaderHandle) || (NULL == pPath))
    {
        return E_INVALIDARG;
    }

    if ((NULL != *pReaderHandle) && (helpCheckValidReader(*pReaderHandle)))
    {
        return E_HANDLE;
    }

    // Check the given path
    if (S_OK != (hr = helpValidatePath(pPath)))
    {
        return hr;
    }

    osFilePath filePath(pPath);

    gtString fileExtn;
    filePath.getFileExtension(fileExtn);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // Check if prd file
    if (0 == fileExtn.compareNoCase(L"prd"))
    {
        hr = helpHandlePrdFile(pPath, pReaderHandle);
    }
#else
    // Check if caperf file
    if (0 == fileExtn.compareNoCase(L"caperf"))
    {
        hr = helpHandleCaPerfFile(pPath, pReaderHandle);
    }
#endif

    return hr;
}

HRESULT fnCloseProfile(
    /*in*/ ReaderHandle** pReaderHandle)
{
    if (NULL == pReaderHandle)
    {
        return E_INVALIDARG;
    }

    if (NULL == *pReaderHandle)
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(*pReaderHandle))
    {
        return E_INVALIDARG;
    }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    PrdTranslator* pTrans =  static_cast<PrdTranslator*>(*pReaderHandle);
    delete pTrans;
#else
    CaPerfTranslator* pTrans = static_cast<CaPerfTranslator*>(*pReaderHandle);
    delete pTrans;
#endif

    // All error conditions checked. Remove the reader from the list
    // of valid readers and delete the CpuProfileDataAccess object.
    g_validReaders.remove(*pReaderHandle);

    *pReaderHandle = NULL;
    return S_OK;
}

HRESULT fnCpuProfileDataTranslate(
    /*in*/ ReaderHandle* pReaderHandle,
    /*in*/ const wchar_t* pFileName,
    /*in*/ PfnProgressBarCallback pfnProgressBarCallback,
    /*in*/ const wchar_t* pSearchPath,
    /*in*/ const wchar_t* pServerList,
    /*in*/ const wchar_t* pCachePath)
{
    HRESULT hr = S_OK;

    if ((NULL == pReaderHandle) || (NULL == pFileName))
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    osFilePath outFile(pFileName);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    PrdTranslator* pDataTranslator = static_cast<PrdTranslator*>(pReaderHandle);

    if (nullptr != pDataTranslator)
    {
        pDataTranslator->SetDebugSymbolsSearchPath(pSearchPath, pServerList, pCachePath);

        MissedInfoType missedInfo;
        gtString errorString;
        gtList<gtString> processFilters;

        hr = pDataTranslator->TranslateData(outFile,
                                            &missedInfo, //MissedInfoType *pMissedInfo
                                            processFilters,
                                            errorString, //QWidget *pApp = NULL
                                            false, //bool bThread = false
                                            false, //bool bCLUtil = false
                                            false,
                                            pfnProgressBarCallback); //bool bLdStCollect = false
    }
#else // WINDOWS_ONLY
    (void)pfnProgressBarCallback; // unused
    CaPerfTranslator* pTrans = static_cast<CaPerfTranslator*>(pReaderHandle);

    pTrans->SetDebugSymbolsSearchPath(pSearchPath, pServerList, pCachePath);

    osDirectory outDir;
    outFile.getFileDirectory(outDir);

    if (!outDir.exists())
    {
        fwprintf(stderr, L"Directory %ls does not exist\n", pFileName);
        return E_FAIL;
    }

    // Setup CSS file
    osFilePath cssFilePath(outFile);
    cssFilePath.setFileExtension(L"css");

    if (S_OK != pTrans->setupCssFile(std::string(cssFilePath.asString().asASCIICharArray())))
    {
        fwprintf(stderr, L"Failed to setup CSS file %s\n", cssFilePath.asString().asASCIICharArray());
        hr = E_FAIL;
    }

    // Translate
    int ret = 0;
    gtString sessionDir = outDir.asString();

    if (sessionDir[sessionDir.length() - 1] == '/')
    {
        sessionDir[sessionDir.length() - 1] = 0;
    }

    if ((S_OK == hr) && (0 != (ret = pTrans->translatePerfDataToCaData(std::string(sessionDir.asASCIICharArray())))))
    {
        hr = ((static_cast<int>(E_NODATA) == ret) || (static_cast<int>(E_INVALIDDATA) == ret)) ? ret : E_FAIL;
    }

    // Write cxlcpdb file
    if ((S_OK == hr) && (0 != pTrans->writeEbpOutput(std::string(outFile.asString().asASCIICharArray()))))
    {
        fwprintf(stderr, L"Failed to write cxlcpdb data (%s)\n", outFile.asString().asASCIICharArray());
        hr = E_FAIL;
    }
#endif // WINDOWS_ONLY

    return hr;
}

HRESULT fnMigrateEBPToDB(
    /*in*/ const osFilePath& ebpFilePath)
{
    HRESULT hr = S_OK;

    DataMigrator dataMigrator(ebpFilePath);
    bool rc = dataMigrator.Migrate();

    if (!rc)
    {
        hr = E_FAIL;
    }

    return hr;
}
