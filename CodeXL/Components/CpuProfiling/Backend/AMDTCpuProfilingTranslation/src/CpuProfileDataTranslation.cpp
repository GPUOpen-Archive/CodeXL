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
#include "CpuProfileDataAccess.h"

#include <AMDTOSWrappers/Include/osProcess.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // Windows only stuff

    #include <AMDTCpuProfilingTranslation/inc/Windows/TaskInfoInterface.h>
    #include "Windows/PrdTranslator.h"

#else // WINDOWS_ONLY
    // Linux only stuff
    #include <unistd.h>
    #include "Linux/CaPerfTranslator.h"

#endif // WINDOWS_ONLY


gtList<ReaderHandle*> g_validReaders;

//returns true if valid and false if not
static bool helpCheckValidReader(const ReaderHandle* pReaderHandle)
{
    gtList<ReaderHandle*>::const_iterator it = g_validReaders.begin(), itEnd = g_validReaders.end();

    for (; it != itEnd; ++it)
    {
        if (*it == pReaderHandle)
        {
            break;
        }
    }

    return it != itEnd;
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
    ////////////////////////////////////////////////////
    //create new object
    CpuProfileDataAccess* pDataAccess = new CpuProfileDataAccess();

    if (NULL == pDataAccess)
    {
        return E_OUTOFMEMORY;
    }

    *pReaderHandle = static_cast<ReaderHandle*>(pDataAccess);

    HRESULT hr;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    //add prd file to object
    hr = pDataAccess->OpenPrdFile(pFilePath);

    if (S_OK != hr)
    {
        delete pDataAccess;
        *pReaderHandle = NULL;
    }
    else
    {
        g_validReaders.push_back(*pReaderHandle);
    }

#else
    hr = E_NOTIMPL;
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    return hr;
}


static HRESULT helpHandleEbpFile(
    wchar_t* pFilePath,
    ReaderHandle** pReaderHandle)
{
    ////////////////////////////////////////////////////
    //create new object
    CpuProfileDataAccess* pDataAccess = new CpuProfileDataAccess();

    if (NULL == pDataAccess)
    {
        return E_OUTOFMEMORY;
    }

    *pReaderHandle = static_cast<ReaderHandle*>(pDataAccess);

    //add data set to object from file
    HRESULT hr = pDataAccess->ImportDataSet(pFilePath);

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
static HRESULT helpHandleCaPerfFile(wchar_t* pFilePath, ReaderHandle** pReaderHandle)
{
    HRESULT hr;
    gtString path = pFilePath;

    // Create CaPerfTranslator
    CaPerfTranslator* pTrans = new CaPerfTranslator(path.asASCIICharArray());

    if (NULL == pTrans)
    {
        return E_OUTOFMEMORY;
    }

    *pReaderHandle = static_cast<ReaderHandle*>(pTrans);
    g_validReaders.push_back(*pReaderHandle);
    hr = S_OK;

    return hr;
}
#endif  // AMDT_BUILD_TARGET == AMDT_LINUX_OS

// Note: This function currently handles:
// - prd files
// - ebp,tbp files
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

    ////////////////////////////////////////////////////
    // Check file extension
    wchar_t tempFileName[OS_MAX_PATH];

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    wcscpy_s(tempFileName, OS_MAX_PATH, pPath);

    // Handle different types of input path
    // Check if prd file
    if (0 == _wcsicmp(L".prd", &(tempFileName[wcslen(tempFileName) - 4])))
    {
        hr = helpHandlePrdFile(tempFileName, pReaderHandle);
    }
    // Check if tbp/ebp file
    else if ((0 == _wcsicmp(L".ebp", &(tempFileName[wcslen(tempFileName) - 4])))
             || (0 == _wcsicmp(L".tbp", &(tempFileName[wcslen(tempFileName) - 4]))))
    {
        hr = helpHandleEbpFile(tempFileName, pReaderHandle);
    }

#else
    wcsncpy(tempFileName, pPath, OS_MAX_PATH);

    // Handle different types of input path
    // Check if caperf file
    if (0 == wcscmp(L".caperf", &(tempFileName[wcslen(tempFileName) - 7])))
    {
        hr = helpHandleCaPerfFile(tempFileName, pReaderHandle);
    }

#endif

    return hr;
}


//TODO: [Suravee] This one is deprecated
HRESULT fnOpenAggregatedProfile(
    /*in*/ wchar_t* pFileName,
    /*out*/ ReaderHandle** pReaderHandle)
{
    if ((NULL == pReaderHandle) || (NULL == pFileName))
    {
        return E_INVALIDARG;
    }

    //Get absolute path
    QFileInfo aggFile(QString::fromWCharArray(pFileName));
    QString aggPath = aggFile.absoluteFilePath();
    aggPath.replace('/', '\\');

    return fnOpenProfile(aggPath.toStdWString().c_str(), pReaderHandle);
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

    // BUG168352
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


HRESULT fnSetJitDir(
    /*in*/ ReaderHandle* pReaderHandle,
    /*in*/ wchar_t* pJitDirName)
{
    if (NULL == pReaderHandle)
    {
        return E_INVALIDARG;
    }

    if ((NULL != pJitDirName) && (0 == wcslen(pJitDirName)))
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    if (NULL != pJitDirName)
    {
        HRESULT hr = helpValidatePath(pJitDirName);

        if (S_OK != hr)
        {
            return hr;
        }
    }

    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);
    return pDataAccess->SetJitDir(pJitDirName);
}


HRESULT fnClearTempJitDir()
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    return fnCleanupJitInformation();
#else
    //TODO: [Suravee]
    return E_NOTIMPL;
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
}


HRESULT fnGetStartTimeMark(
    /*in*/ ReaderHandle* pReaderHandle,
    /*out*/ CPA_TIME* pTimeMark)
{
    if ((NULL == pReaderHandle) || (NULL == pTimeMark))
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }


    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);

    if (!pDataAccess->IsProfileDataAvailable())
    {
        return E_FAIL;
    }

    return pDataAccess->GetStartTimeMark(pTimeMark);
}


HRESULT fnGetEndTimeMark(
    /*in*/ ReaderHandle* pReaderHandle,
    /*out*/ CPA_TIME* pTimeMark)
{
    if ((NULL == pReaderHandle) || (NULL == pTimeMark))
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }


    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);

    if (!pDataAccess->IsProfileDataAvailable())
    {
        return E_FAIL;
    }

    return pDataAccess->GetEndTimeMark(pTimeMark);
}


HRESULT fnAddDataSet(
    /*in*/ ReaderHandle* pReaderHandle,
    /*in*/ ProfileDataSetInterval* pIntervals,
    /*in*/ unsigned int count,
    /*in*/ const wchar_t* pDataSetName)
{
    if ((NULL == pReaderHandle) || (NULL == pDataSetName))
    {
        return E_INVALIDARG;
    }

    //if count is 0 and pIntervals is NULL, it's an empty data set
    if (((0 != count) && (NULL == pIntervals)) ||
        ((0 == count) && (NULL != pIntervals)))
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);

    if (!pDataAccess->IsProfileDataAvailable())
    {
        return E_FAIL;
    }

    if (pDataAccess->IsDataSetExists(pDataSetName))
    {
        return E_ABORT;
    }

    return pDataAccess->AddDataSet(pIntervals, count, pDataSetName);
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

HRESULT fnListDataSets(
    /*in*/ ReaderHandle* pReaderHandle,
    /*in*/ unsigned int maxSize,
    /*out*/ wchar_t** ppDataSets)
{
    if ((NULL == pReaderHandle) || (NULL == ppDataSets) || (0 == maxSize))
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

    if (dataList.size() > maxSize)
    {
        hr = E_OUTOFMEMORY;
    }

    if (S_OK != hr)
    {
        return hr;
    }

    gtList<wchar_t*>::iterator it = dataList.begin();

    for (int i = 0; it != dataList.end(); it++, i++)
    {
        ppDataSets[i] = *it;
    }

    return hr;
}


HRESULT fnRemoveDataSet(
    /*in*/ ReaderHandle* pReaderHandle,
    /*in*/ wchar_t* pDataSetName)
{
    if ((NULL == pReaderHandle) || (NULL == pDataSetName))
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);

    if (!pDataAccess->IsDataSetExists(pDataSetName))
    {
        return E_INVALIDARG;
    }

    return pDataAccess->RemoveDataSet(pDataSetName);
}


HRESULT fnAggregateDataSets(
    /*in*/ ReaderHandle* pReaderHandle,
    /*in*/ bool* pCancel,
    /*in*/ wchar_t* pJitDataDirectory,
    /*out*/ float* pPercentComplete)
{
    if (NULL == pReaderHandle)
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    if (pJitDataDirectory)
    {
        QString tempDir = QString::fromWCharArray(pJitDataDirectory);

        //Replaces / with \ in directories
        tempDir.replace('/', '\\');

        if (!tempDir.endsWith("\\"))
        {
            tempDir += "\\";
        }

        //recursively create directories, if needed
        int sections = tempDir.count("\\");

        for (int sects = 1; sects < sections; sects ++)
        {
            QDir dir;
            dir.setPath(tempDir.section("\\", 0, sects));

            if (!dir.exists())
            {
                if (!dir.mkdir(dir.path()))
                {
                    return E_ACCESSDENIED;
                }
            }
        }
    }

    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);

    return pDataAccess->AggregateDataSets(pCancel, pJitDataDirectory,
                                          pPercentComplete);
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
    string sessionName;
    sessionName = string(outDir.baseName().toLatin1());

    ///////////////////////////////////////////////////////
    // Setup CSS file
    string cssFile;
    cssFile = cssFile + string(outDirPath.toLatin1());// + "/" + sessionName + ".css";

    if (S_OK != pTrans->setupCssFile(cssFile))
    {
        fwprintf(stderr, L"Failed to setup CSS file %s\n", cssFile.c_str());
        hr = E_FAIL;
    }

    ///////////////////////////////////////////////////////
    // Translate
    int ret = 0;

    if ((S_OK == hr) && (0 != (ret = pTrans->translatePerfDataToCaData(string(outDirPath.toLatin1())))))
    {
        hr = ((static_cast<int>(E_NODATA) == ret) || (static_cast<int>(E_INVALIDDATA) == ret)) ? ret : E_FAIL;
    }

    ///////////////////////////////////////////////////////
    // Write EBP
    string ebpFile;
    ebpFile = ebpFile + string(outDirPath.toLatin1()) + "/" + sessionName + ".ebp";

    if ((S_OK == hr) && (0 != pTrans->writeEbpOutput(ebpFile)))
    {
        fwprintf(stderr, L"Failed to write EBP data (%s)\n", ebpFile.c_str());
        hr = E_FAIL;
    }

#endif // WINDOWS_ONLY

    return hr;
}


HRESULT fnAppendDataSets(
    /*in*/ ReaderHandle* pReaderHandle,
    /*in*/ const wchar_t* pDestinationDataSet,
    /*in*/ const wchar_t* pSourceDataSet)
{
    if ((NULL == pReaderHandle) || (NULL == pDestinationDataSet))
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);

    if ((!pDataAccess->IsDataSetExists(pDestinationDataSet))
        || (!pDataAccess->IsDataSetExists(pSourceDataSet)))
    {
        return E_INVALIDARG;
    }

    if ((!pDataAccess->IsDataSetReady(pDestinationDataSet))
        || (!pDataAccess->IsDataSetReady(pSourceDataSet)))
    {
        return E_PENDING;
    }

    return pDataAccess->AppendDataSets(pDestinationDataSet, pSourceDataSet);
}


HRESULT fnGetAvailableCoreData(
    /*in*/ ReaderHandle* pReaderHandle,
    /*out*/ gtUInt64* pCoreMask)
{
    if ((NULL == pReaderHandle) || (NULL == pCoreMask))
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);
    return pDataAccess->GetAvailableCoreData(pCoreMask);
}


HRESULT fnSetRuleCoreData(
    /*in*/ ReaderHandle* pReaderHandle,
    /*in*/ gtUInt64 coreMask)
{
    if ((NULL == pReaderHandle) || (0 == coreMask))
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);
    return pDataAccess->SetRuleCoreData(coreMask);
}


HRESULT fnSetRuleForProcesses(
    /*in*/ ReaderHandle* pReaderHandle,
    /*in*/ unsigned int count,
    /*in*/ unsigned int* pProcessIdList)
{
    if ((NULL == pReaderHandle) || (NULL == pProcessIdList))
    {
        return E_INVALIDARG;
    }

    if (0 == count)
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);
    return pDataAccess->SetRuleForProcesses(count, pProcessIdList);
}


HRESULT fnSetRuleForThreads(
    /*in*/ ReaderHandle* pReaderHandle,
    /*in*/ unsigned int count,
    /*in*/ unsigned int* pThreadIdList)
{
    if ((NULL == pReaderHandle) || (NULL == pThreadIdList))
    {
        return E_INVALIDARG;
    }

    if (0 == count)
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);
    return pDataAccess->SetRuleForThreads(count, pThreadIdList);
}


HRESULT fnResetRules(/*in*/ ReaderHandle* pReaderHandle)
{
    if (NULL == pReaderHandle)
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);
    return pDataAccess->ResetRules();
}


/** This will get the count of the performance events used to get the sample
    data returned in the other fnGetXData functions.

    \ingroup data
    @param[in] pReaderHandle The open reader handle
    @param[out] pCount The count of the performance events
    \return The success of retrieving the data event count
    \retval S_OK Success
    \retval E_INVALIDARG pReaderHandle was not an open handle, or
        pCount was NULL
    \retval E_UNEXPECTED an unexpected error occurred
*/
HRESULT fnGetDataEventCount(
    /*in*/ ReaderHandle* pReaderHandle,
    /*out*/ unsigned int* pCount)
{
    if (NULL == pReaderHandle)
    {
        return E_INVALIDARG;
    }

    if (NULL == pCount)
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);
    *pCount = (unsigned int)pDataAccess->GetDataEventCount();
    return S_OK;
}


HRESULT fnGetDataEvents(
    /*in*/ ReaderHandle* pReaderHandle,
    /*in*/ unsigned int maxSize,
    /*out*/ gtUInt64* pPerformanceEvents,
    /*out*/ wchar_t** ppDataLabels,
    /*out*/ gtUInt64* pSampIntvls)
{
    if (NULL == pReaderHandle)
    {
        return E_INVALIDARG;
    }

    if (0 == maxSize)
    {
        return E_INVALIDARG;
    }

    if ((NULL == pPerformanceEvents) && (NULL == ppDataLabels) &&
        (NULL == pSampIntvls))
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);
    return pDataAccess->GetDataEvents(maxSize, pPerformanceEvents, ppDataLabels,
                                      pSampIntvls);
}


HRESULT fnGetModuleDataCount(
    /*in*/ ReaderHandle* pReaderHandle,
    /*in*/ wchar_t* pDataSetName,
    /*out*/ unsigned int* pCount)
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

    if (!pDataAccess->IsDataSetExists(pDataSetName))
    {
        return E_INVALIDARG;
    }

    if (!pDataAccess->IsDataSetReady(pDataSetName))
    {
        return E_PENDING;
    }

    *pCount = static_cast<unsigned int>(pDataAccess->GetModuleDataCount(pDataSetName));
    return S_OK;
}


HRESULT fnGetModuleData(
    /*in*/ ReaderHandle* pReaderHandle,
    /*in*/ wchar_t* pDataSetName,
    /*in*/ unsigned int maxSize,
    /*out*/ ModuleDataType* pSystemData)
{
    if ((NULL == pReaderHandle) || (NULL == pSystemData) || (0 == maxSize))
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);

    if (!pDataAccess->IsDataSetExists(pDataSetName))
    {
        return E_INVALIDARG;
    }

    if (!pDataAccess->IsDataSetReady(pDataSetName))
    {
        return E_PENDING;
    }

    return pDataAccess->GetModuleData(pDataSetName, maxSize, pSystemData);
}


HRESULT fnGetProcessDataCount(
    /*in*/ ReaderHandle* pReaderHandle,
    /*in*/ wchar_t* pDataSetName,
    /*out*/ unsigned int* pCount)
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

    if (!pDataAccess->IsDataSetExists(pDataSetName))
    {
        return E_INVALIDARG;
    }

    if (!pDataAccess->IsDataSetReady(pDataSetName))
    {
        return E_PENDING;
    }

    *pCount = (unsigned int)pDataAccess->GetProcessDataCount(pDataSetName);
    return S_OK;
}


HRESULT fnGetProcessData(
    /*in*/ ReaderHandle* pReaderHandle,
    /*in*/ wchar_t* pDataSetName,
    /*in*/ unsigned int maxSize,
    /*out*/ ModuleDataType* pProcessData)
{
    if ((NULL == pReaderHandle) || (NULL == pProcessData) || (0 == maxSize))
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);

    if (!pDataAccess->IsDataSetExists(pDataSetName))
    {
        return E_INVALIDARG;
    }

    if (!pDataAccess->IsDataSetReady(pDataSetName))
    {
        return E_PENDING;
    }

    return pDataAccess->GetProcessData(pDataSetName, maxSize, pProcessData);
}


HRESULT fnGetInstructionDataCount(
    /*in*/ ReaderHandle* pReaderHandle,
    /*in*/ wchar_t* pDataSetName,
    /*in*/ wchar_t* modulePath,
    /*out*/ unsigned int* pCount)
{
    if ((NULL == pReaderHandle) || (NULL == pCount) || (NULL == modulePath))
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);

    if (!pDataAccess->IsDataSetExists(pDataSetName))
    {
        return E_INVALIDARG;
    }

    if (!pDataAccess->IsDataSetReady(pDataSetName))
    {
        return E_PENDING;
    }

    return pDataAccess->GetInstructionDataCount(pDataSetName, modulePath, pCount);
}


HRESULT fnGetInstructionData(
    /*in*/ ReaderHandle* pReaderHandle,
    /*in*/ wchar_t* pDataSetName,
    /*in*/ wchar_t* modulePath,
    /*in*/ unsigned int maxSize,
    /*out*/ InstructionDataType* pInstructionData,
    /*out*/ gtUInt64* pModuleLoadAddress,
    /*out*/ ModuleType* pModuleType)
{
    if ((NULL == pReaderHandle) || (NULL == pInstructionData)
        || (NULL == modulePath) || (0 == maxSize))
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);

    if (!pDataAccess->IsDataSetExists(pDataSetName))
    {
        return E_INVALIDARG;
    }

    if (!pDataAccess->IsDataSetReady(pDataSetName))
    {
        return E_PENDING;
    }

    return pDataAccess->GetInstructionData(pDataSetName, modulePath, maxSize,
                                           pInstructionData, pModuleLoadAddress, pModuleType);
}


HRESULT fnGetJitData(
    /*in*/ ReaderHandle* pReaderHandle,
    /*in*/ wchar_t* pDataSetName,
    /*in*/ gtUInt64 address,
    /*in*/ wchar_t* pJncDataFile,
    /*in*/ ModuleType jitType,
    /*out*/ JITDataType* pJitData)
{
    if ((NULL == pReaderHandle) || (NULL == pJitData)
        || (NULL == pJncDataFile))
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    CpuProfileDataAccess* pDataAccess =  static_cast<CpuProfileDataAccess*>(pReaderHandle);

    if (!pDataAccess->IsDataSetExists(pDataSetName))
    {
        return E_INVALIDARG;
    }

    if (!pDataAccess->IsDataSetReady(pDataSetName))
    {
        return E_PENDING;
    }

    return pDataAccess->GetJitData(pDataSetName, address, pJncDataFile,
                                   jitType, pJitData);
}


HRESULT fnGetFirstRawRecord(
    /*in*/ ReaderHandle* pReaderHandle,
    /*in*/ wchar_t* pDataSetName,
    /*out*/ RawDataType* pData)
{
    if ((NULL == pReaderHandle) || (NULL == pData))
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    CpuProfileDataAccess* pDataAccess = static_cast<CpuProfileDataAccess*>(pReaderHandle);

    if (!pDataAccess->IsProfileDataAvailable())
    {
        return E_ACCESSDENIED;
    }

    if (!pDataAccess->IsDataSetExists(pDataSetName))
    {
        return E_INVALIDARG;
    }

    return pDataAccess->GetFirstRawRecord(pDataSetName, pData);
}


HRESULT fnGetNextRawRecord(
    /*in*/ ReaderHandle* pReaderHandle,
    /*out*/ RawDataType* pData,
    /*out*/ float* pPercentComplete)
{
    if ((NULL == pReaderHandle) || (NULL == pData))
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    CpuProfileDataAccess* pDataAccess = static_cast<CpuProfileDataAccess*>(pReaderHandle);

    if (!pDataAccess->IsProfileDataAvailable())
    {
        return E_ACCESSDENIED;
    }

    return pDataAccess->GetNextRawRecord(pData, pPercentComplete);
}


HRESULT fnGetCpuInfo(
    /*in*/ ReaderHandle* pReaderHandle,
    /*out*/ unsigned int* pCpuFamily,
    /*out*/ unsigned int* pCpuModel,
    /*out*/ unsigned int* pCoreCount)
{
    if ((NULL == pReaderHandle) || (NULL == pCpuFamily) || (NULL == pCpuModel))
    {
        return E_INVALIDARG;
    }

    if (!helpCheckValidReader(pReaderHandle))
    {
        return E_INVALIDARG;
    }

    CpuProfileDataAccess* pDataAccess = static_cast<CpuProfileDataAccess*>(pReaderHandle);

    //If the reader doesn't have data
    if ((!pDataAccess->IsProfileDataAvailable()) &&
        (!pDataAccess->IsDataSetReady(NULL)))
    {
        return E_INVALIDARG;
    }

    *pCpuFamily = pDataAccess->GetCpuFamily();
    *pCpuModel = pDataAccess->GetCpuModel();

    if (NULL != pCoreCount)
    {
        pDataAccess->GetAvailableCoreCount(pCoreCount);
    }

    return S_OK;
}
