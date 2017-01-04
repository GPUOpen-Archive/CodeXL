//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileDataTranslation.cpp
/// \brief CPU profile data translation interface implementation.
///
//==================================================================================

#include <QFileInfo>

#include <CpuProfileDataTranslation.h>
#include <CpuProfileDataMigrator.h>

#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osDirectory.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // Windows only stuff
    #include <AMDTCpuProfilingTranslation/inc/Windows/TaskInfoInterface.h>
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

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    PrdTranslator* pDataTranslator = static_cast<PrdTranslator*>(pReaderHandle);

    if (nullptr != pDataTranslator)
    {
        pDataTranslator->SetDebugSymbolsSearchPath(pSearchPath, pServerList, pCachePath);

        osFilePath outFile(pFileName);
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

    // For Linux, we don't handle dataset stuff yet

    CaPerfTranslator* pTrans = static_cast<CaPerfTranslator*>(pReaderHandle);

    pTrans->SetDebugSymbolsSearchPath(pSearchPath, pServerList, pCachePath);

    ///////////////////////////////////////////////////////
    //Get absolute path
    // NOTE: "pFileName" is the directory to store the output    
    QFileInfo outDir(QString::fromStdWString(pFileName));
    QString outDirPath = outDir.absoluteFilePath();
    outDirPath.replace('\\', '/');

    if (!outDir.exists())
    {
        wchar_t tmp[OS_MAX_PATH] = {L'\0'};
        outDirPath.toWCharArray(tmp);
        fwprintf(stderr, L"Directory %ls does not exist\n", tmp);
        return E_FAIL;
    }

    ///////////////////////////////////////////////////////
    // Get session Name
    std::string sessionName;
    sessionName = std::string(outDir.baseName().toLatin1());

    ///////////////////////////////////////////////////////
    // Setup CSS file
    std::string cssFile;
    cssFile = cssFile + std::string(outDirPath.toLatin1());// + "/" + sessionName + ".css";

    if (S_OK != pTrans->setupCssFile(cssFile))
    {
        fwprintf(stderr, L"Failed to setup CSS file %s\n", cssFile.c_str());
        hr = E_FAIL;
    }

    ///////////////////////////////////////////////////////
    // Translate
    int ret = 0;

    if ((S_OK == hr) && (0 != (ret = pTrans->translatePerfDataToCaData(std::string(outDirPath.toLatin1())))))
    {
        hr = ((static_cast<int>(E_NODATA) == ret) || (static_cast<int>(E_INVALIDDATA) == ret)) ? ret : E_FAIL;
    }

    ///////////////////////////////////////////////////////
    // Write EBP
    std::string ebpFile;
    ebpFile = ebpFile + std::string(outDirPath.toLatin1()) + "/" + sessionName + ".ebp";

    if ((S_OK == hr) && (0 != pTrans->writeEbpOutput(ebpFile)))
    {
        fwprintf(stderr, L"Failed to write EBP data (%s)\n", ebpFile.c_str());
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