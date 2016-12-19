//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileDataTranslation.cpp
/// \brief CPU profile data translation interface implementation.
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingTranslation/src/CpuProfileDataTranslation.cpp#14 $
// Last checkin:   $DateTime: 2016/04/14 01:44:54 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569055 $
//=====================================================================

#include <QtCore>
#include <QDir>
#include <QFileInfo>

//Commented out until config.h is added to repository or it is #ifdeffed only for Linux
//#include "config.h"
#include <CpuProfileDataTranslation.h>
#include <CpuProfileDataMigrator.h>
#include "CpuProfileDataAccess.h"

#include <AMDTOSWrappers/Include/osProcess.h>

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
    //create new object
    CpuProfileDataAccess* pDataAccess = new CpuProfileDataAccess();

    if (NULL == pDataAccess)
    {
        return E_OUTOFMEMORY;
    }

    *pReaderHandle = static_cast<ReaderHandle*>(pDataAccess);

    //add prd file to object
    HRESULT hr = pDataAccess->OpenPrdFile(pFilePath);

    if (S_OK != hr)
    {
        delete pDataAccess;
        *pReaderHandle = NULL;
    }
    else
    {
        g_validReaders.push_back(*pReaderHandle);
    }

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
    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(*pReaderHandle);

    if (pDataAccess->IsAggregationInProgress())
    {
        return E_ACCESSDENIED;
    }

    delete pDataAccess;
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

HRESULT fnGetDataSetCount(
    /*in*/ ReaderHandle* pReaderHandle,
    /*out*/unsigned int* pCount)
{
    if ((NULL == pReaderHandle) || (NULL == pCount))
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);
    gtList<wchar_t*> dataList;
    HRESULT hr = pDataAccess->GetDataSets(&dataList);
    *pCount = (unsigned int)dataList.size();
    return hr;
}

HRESULT fnWriteSetToFile(
    /*in*/ ReaderHandle* pReaderHandle,
    /*in*/ const wchar_t* pDataSetName,
    /*in*/ const wchar_t* pFileName,
    /*in*/ PfnProgressBarCallback pfnProgressBarCallback,
    /*in*/ const wchar_t* pSearchPath,
    /*in*/ const wchar_t* pServerList,
    /*in*/ const wchar_t* pCachePath)
{
    (void)(pDataSetName); // unused
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
    unsigned int numDataSet = 0;
    hr = fnGetDataSetCount(pReaderHandle, &numDataSet);

    if (S_OK != hr)
    {
        return hr;
    }

    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);

    //Get absolute path
    QFileInfo outFile(QString::fromStdWString(pFileName));
    QString outPath = outFile.absoluteFilePath();
    outPath.replace('/', '\\');

    if ((NULL == pDataSetName) || (0 == numDataSet))
    {
        //**********************
        // Use CADataTranslator
        //**********************

        // Get the PRD file name from DataAccess
        QString prdFile(QString::fromWCharArray(pDataAccess->GetPrdFilePath()));

        gtString envVal;
        bool collectStats = osGetCurrentProcessEnvVariableValue(L"CODEXL_CPU_TRANS_STATS", envVal);

        // Create the PrdTranslator
        PrdTranslator* pDataTranslator = new PrdTranslator(prdFile, collectStats);

        pDataTranslator->SetDebugSymbolsSearchPath(pSearchPath, pServerList, pCachePath);

        if (NULL == pDataTranslator)
        {
            return E_OUTOFMEMORY;
        }

        MissedInfoType missedInfo;
        QString errorString;
        hr = pDataTranslator->TranslateData(
                 outPath ,
                 &missedInfo, //MissedInfoType *pMissedInfo
                 QStringList(), //QStringList processFilters
                 QStringList(), //QStringList targetPidList
                 errorString, //QWidget *pApp = NULL
                 false, //bool bThread = false
                 false, //bool bCLUtil = false
                 false,
                 pfnProgressBarCallback); //bool bLdStCollect = false

        delete pDataTranslator;
    }
    else
    {
        if (!pDataAccess->IsDataSetExists(pDataSetName))
        {
            return E_INVALIDARG;
        }

        if (!pDataAccess->IsDataSetReady(pDataSetName))
        {
            return E_PENDING;
        }

        hr = pDataAccess->WriteSetToFile(pDataSetName, outPath.toStdWString().c_str());
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