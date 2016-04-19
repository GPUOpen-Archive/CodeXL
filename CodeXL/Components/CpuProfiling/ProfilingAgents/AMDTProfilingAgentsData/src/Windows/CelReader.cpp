//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CelReader.cpp
///
//==================================================================================

#include <AMDTProfilingAgentsData/inc/Windows/CelReader.h>


CelReader::CelReader() : m_curEventType(evInvalidCLREvent)
{
}

CelReader::~CelReader()
{
    Close();
}

bool CelReader::Open(const wchar_t* pCelFileName)
{
    bool ret = false;

    if (NULL != pCelFileName)
    {
        if (m_fileStream.open(pCelFileName, FMODE_TEXT("r+b")))
        {
            ReadHeader();
            ret = true;
        }
    }

    return ret;
}

void CelReader::Close()
{
    if (m_fileStream.isOpened())
    {
        m_fileStream.close();
    }
}

void CelReader::ReadHeader()
{
    m_fileStream.read(m_celHeader.signature);
    m_fileStream.read(m_celHeader.version);
    m_fileStream.read(m_celHeader.num_Blocks);
    m_fileStream.read(m_celHeader.processId);
    m_fileStream.read(m_celHeader.b32_bit);
}

bool CelReader::Is32BitProcess() const
{
    return 0 != m_celHeader.b32_bit;
}

gtUInt32 CelReader::GetNumRecord() const
{
    return m_celHeader.num_Blocks;
}

CLREventType CelReader::GetNextRecordType()
{
    gtUInt32 eventType = evInvalidCLREvent;

    m_fileStream.read(eventType);

    if (evExceptionCLRCatcherExecute < eventType)
    {
        eventType = evInvalidCLREvent;
    }

    m_curEventType = static_cast<CLREventType>(eventType);
    return m_curEventType;
}

HRESULT CelReader::GetAppDomainCreationRecord(AppDomainRecord* pDomainRec)
{
    HRESULT hr = E_FAIL;

    if (evAppDomainCreationFinished == m_curEventType)
    {
        gtUInt64 tData;
        m_fileStream.read(tData);
        pDomainRec->domainId = static_cast<AppDomainID>(tData);


        m_fileStream.read(tData);
        pDomainRec->loadTime = tData;

        ReadString(pDomainRec->domainName);

        hr = S_OK;
    }

    return hr;
}


HRESULT CelReader::GetAppDomainShutdownRecord(AppDomainID* pAppDomainId, gtUInt64* pSystime)
{
    HRESULT hr = E_FAIL;

    if (evAppDomainShutdownStarted == m_curEventType)
    {
        gtUInt64 tData;
        m_fileStream.read(tData);
        *pAppDomainId = static_cast<AppDomainID>(tData);

        m_fileStream.read(tData);
        *pSystime = tData;

        hr = S_OK;
    }

    return hr;
}



HRESULT CelReader::GetAssemblyLoadRecord(AssemblyRecord* pAsmRec)
{
    HRESULT hr = E_FAIL;

    if (evAssemblyLoadFinished == m_curEventType)
    {
        gtUInt64 tData;
        m_fileStream.read(tData);
        pAsmRec->asmId = static_cast<AssemblyID>(tData);

        m_fileStream.read(tData);
        pAsmRec->appId = static_cast<AppDomainID>(tData);

        m_fileStream.read(tData);
        pAsmRec->loadTime = tData;

        ReadString(pAsmRec->asmName);

        hr = S_OK;
    }

    return hr;
}


HRESULT CelReader::GetAssemblyUnloadRecord(AssemblyID* pAssemblyId, gtUInt64* pSystime)
{
    HRESULT hr = E_FAIL;

    if (evAssemblyUnloadStarted == m_curEventType)
    {
        gtUInt64 tData;
        m_fileStream.read(tData);
        *pAssemblyId = static_cast<AssemblyID>(tData);

        m_fileStream.read(tData);
        *pSystime = tData;

        hr = S_OK;
    }

    return hr;
}


HRESULT CelReader::GetModuleLoadRecord(ModuleRecord* pModRec)
{
    HRESULT hr = E_FAIL;

    if (evModuleLoadFinished == m_curEventType)
    {
        gtUInt64 tData;
        m_fileStream.read(tData);
        pModRec->modId = (ModuleID) tData;

        m_fileStream.read(tData);
        pModRec->loadTime = tData;

        ReadString(pModRec->modName);

        hr = S_OK;
    }

    return hr;
}


HRESULT CelReader::GetModuleUnloadRecord(ModuleID* pModId, gtUInt64* pSystime)
{
    HRESULT hr = E_FAIL;

    if (evModuleUnloadStarted == m_curEventType)
    {
        gtUInt64 tData;

        m_fileStream.read(tData);
        *pModId = (ModuleID) tData;

        m_fileStream.read(tData);
        *pSystime = tData;

        hr = S_OK;
    }

    return hr;
}

HRESULT CelReader::GetModuleAttachedToAssemblyRec(ModuleID* pModuleId, AssemblyID* pAssemblyId,
                                                  gtUInt64* pAssemblyLoadAddr, wchar_t* pAssemlbyFullName, unsigned int size)

{
    HRESULT hr = S_OK;

    if (evModuleAttachedToAssembly == m_curEventType)
    {
        gtUInt64 tData;
        m_fileStream.read(tData);
        *pModuleId = (ModuleID) tData;

        m_fileStream.read(tData);
        *pAssemblyId = (AssemblyID) tData;

        if (m_celHeader.version > CEL_VERSION_03)
        {
            // assembly load address and and assembly full name is attached;
            m_fileStream.read(tData);

            if (NULL != pAssemblyLoadAddr)
            {
                *pAssemblyLoadAddr = tData;
            }

            gtUInt32 length = 0;
            m_fileStream.read(length);

            if (0 < length)
            {
                if (length < size && NULL != pAssemlbyFullName)
                {
                    if (m_fileStream.read(pAssemlbyFullName, sizeof(wchar_t) * length))
                    {
                        pAssemlbyFullName[length] = L'\0';
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                }
                else
                {
                    for (gtUInt32 i = 0; i < length; i++)
                    {
                        wchar_t ch;
                        m_fileStream.read(ch);
                    }

                    hr = E_FAIL;
                }
            }
        }
    }

    return hr;
}

HRESULT CelReader::GetClassLoadRecord(ClassRecord* pClassRec)
{
    HRESULT hr = E_FAIL;

    if (evClassLoadFinished == m_curEventType)
    {
        gtUInt64 tData;
        m_fileStream.read(tData);
        pClassRec->classId = static_cast<ClassID>(tData);

        m_fileStream.read(tData);
        pClassRec->modId = static_cast<ModuleID>(tData);

        m_fileStream.read(tData);
        pClassRec->loadTime = tData;

        gtUInt32 cSize;
        m_fileStream.read(cSize);
        memset(pClassRec->className, 0, sizeof(pClassRec->className));
        m_fileStream.read(reinterpret_cast<gtByte*>(pClassRec->className), sizeof(wchar_t) * cSize);

        hr = S_OK;
    }

    return hr;
}

HRESULT CelReader::GetClassUnloadRec(ClassID* pClassId, gtUInt64* pSystime)
{
    HRESULT hr = E_FAIL;

    if (evClassUnloadStarted == m_curEventType)
    {
        gtUInt64 tData;
        m_fileStream.read(tData);
        *pClassId = (ClassID) tData;

        m_fileStream.read(tData);
        *pSystime = tData;

        hr = S_OK;
    }

    return hr;
}


HRESULT CelReader::GetJITCompilationFinished(FunctionRecord* pFuncRec)
{
    HRESULT hr = E_FAIL;

    if (evJITCompilationFinished == m_curEventType)
    {
        gtUInt64 tData;
        m_fileStream.read(tData);
        pFuncRec->modId = static_cast<ModuleID>(tData);

        m_fileStream.read(tData);
        pFuncRec->funcId = static_cast<FunctionID>(tData);

        // class name
        ReadString(pFuncRec->className);

        // function name
        ReadString(pFuncRec->funcName);

        // jnc name
        ReadString(pFuncRec->jncFileName);

        m_fileStream.read(tData);
        pFuncRec->loadTime = tData;

        m_fileStream.read(tData);
        pFuncRec->jitLoadAddr = tData;

        gtUInt32 cSize;
        m_fileStream.read(cSize);
        pFuncRec->codeSize = cSize;

        hr = S_OK;
    }

    return hr;
}


HRESULT CelReader::GetFunctionUnloadStarted(FunctionID* pFuncId, gtUInt64* pSystime)
{
    HRESULT hr = E_FAIL;

    if (evFunctionUnloadStarted == m_curEventType)
    {
        gtUInt64 tData;
        m_fileStream.read(tData);
        *pFuncId = (FunctionID) tData;

        m_fileStream.read(tData);
        *pSystime = tData;

        hr = S_OK;
    }

    return hr;
}

void CelReader::ReadString(wchar_t* pString)
{
    gtUInt32 cSize;

    if (m_fileStream.read(cSize) && m_fileStream.read(pString, sizeof(wchar_t) * cSize))
    {
        pString[cSize] = L'\0';
    }
    else
    {
        pString[0] = L'\0';
    }
}
