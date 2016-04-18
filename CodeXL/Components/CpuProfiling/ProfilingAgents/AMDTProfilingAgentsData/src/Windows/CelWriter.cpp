//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CelWriter.cpp
///
//==================================================================================

#include <AMDTProfilingAgentsData/inc/Windows/CelWriter.h>


CelWriter::CelWriter() : m_entryCount(0)
{
}

CelWriter::~CelWriter()
{
    if (m_fileStream.isOpened())
    {
        m_fileStream.seekCurrentPosition(CrtFile::ORIGIN_BEGIN, NUM_BLOCK_OFFSET);
        m_fileStream.write(m_entryCount);
        m_fileStream.close();
    }
}

bool CelWriter::Initialize(const wchar_t* pCelFileName, int bitness)
{
    bool ret = false;

    if (NULL != pCelFileName)
    {
        if (m_fileStream.open(pCelFileName, FMODE_TEXT("w+b")))
        {
            WriteHeader(bitness);
            ret = true;
        }
    }

    return ret;
}

void CelWriter::WriteHeader(int bitness)
{
    CelHeader header;

    strncpy(header.signature, CEL_HEADER_SIGNATURE, 8);
    header.version = CEL_VERSION;
    header.num_Blocks = 0;
    header.processId = static_cast<gtUInt64>(GetCurrentProcessId());

    if (0 == bitness)
    {
        header.b32_bit = static_cast<gtUInt32>(AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE);
    }
    else
    {
        header.b32_bit = static_cast<gtUInt32>(32 == bitness);
    }

    m_fileStream.write(header.signature);
    m_fileStream.write(header.version);
    m_fileStream.write(header.num_Blocks);
    m_fileStream.write(header.processId);
    m_fileStream.write(header.b32_bit);
    m_fileStream.flush();
}


void CelWriter::WriteAppDomainCreationFinished(AppDomainID appDomainId, gtUInt64 systime,
                                               unsigned int cSize, const wchar_t* pAppDomainName)
{
    gtUInt32 eventType = evAppDomainCreationFinished;
    m_fileStream.write(eventType);

    // use 64-bit value for both 32-bit and 64-bit OS.
    gtUInt64 tData = static_cast<gtUInt64>(appDomainId);
    m_fileStream.write(tData);

    m_fileStream.write(systime);
    m_fileStream.write(cSize);
    m_fileStream.write(pAppDomainName, sizeof(wchar_t) * cSize);
    m_entryCount++;
    m_fileStream.flush();
}

void CelWriter::WriteAppDomainShutdownStarted(AppDomainID appDomainId, gtUInt64 systime)
{
    gtUInt32 eventType = evAppDomainShutdownStarted;
    m_fileStream.write(eventType);

    gtUInt64 tData = static_cast<gtUInt64>(appDomainId);
    m_fileStream.write(tData);

    m_fileStream.write(systime);
    m_entryCount++;
    m_fileStream.flush();
}

void CelWriter::WriteAssemblyLoadFinished(AssemblyID assemblyId, AppDomainID appDomainId,
                                          gtUInt64 systime, unsigned int cSize, const wchar_t* pAssemblyName)
{
    gtUInt32 eventType = evAssemblyLoadFinished;
    m_fileStream.write(eventType);

    // use 64-bit value for both 32-bit and 64-bit OS.
    gtUInt64 tData = static_cast<gtUInt64>(assemblyId);
    m_fileStream.write(tData);

    tData = static_cast<gtUInt64>(appDomainId);
    m_fileStream.write(tData);

    m_fileStream.write(systime);
    m_fileStream.write(cSize);
    m_fileStream.write(pAssemblyName, sizeof(wchar_t) * cSize);
    m_entryCount++;
    m_fileStream.flush();
}

void CelWriter::WriteAssemblyUnloadStarted(AssemblyID assemblyId, gtUInt64 systime)
{
    gtUInt32 eventType = evAssemblyUnloadStarted;
    m_fileStream.write(eventType);

    gtUInt64 tData = static_cast<gtUInt64>(assemblyId);
    m_fileStream.write(tData);

    m_fileStream.write(systime);
    m_entryCount++;
    m_fileStream.flush();
}

void CelWriter::WriteModuleLoadFinished(ModuleID moduleId, gtUInt64 systime, unsigned int cSize, const wchar_t* pModuleName)
{
    gtUInt32 eventType = evModuleLoadFinished;
    m_fileStream.write(eventType);

    gtUInt64 tData = static_cast<gtUInt64>(moduleId);
    m_fileStream.write(tData);

    m_fileStream.write(systime);
    m_fileStream.write(cSize);
    m_fileStream.write(pModuleName, sizeof(wchar_t) * cSize);
    m_entryCount++;
    m_fileStream.flush();
}

void CelWriter::WriteModuleUnloadStarted(ModuleID moduleId, gtUInt64 systime)
{
    gtUInt32 eventType = evModuleUnloadStarted;
    m_fileStream.write(eventType);
    gtUInt64 tData = static_cast<gtUInt64>(moduleId);

    m_fileStream.write(tData);
    m_fileStream.write(systime);
    m_entryCount++;
    m_fileStream.flush();
}


void CelWriter::WriteModuleAttachedToAssembly(ModuleID moduleId, AssemblyID assemlbyId,
                                              gtUInt64 assemblyLoadAddr, const wchar_t* pAssemblyFilePath)
{
    gtUInt32 eventType = evModuleAttachedToAssembly;
    m_fileStream.write(eventType);

    gtUInt64 tData = static_cast<gtUInt64>(moduleId);
    m_fileStream.write(tData);

    tData = static_cast<gtUInt64>(assemlbyId);
    m_fileStream.write(tData);

    m_fileStream.write(assemblyLoadAddr);

    if (NULL != pAssemblyFilePath)
    {
        WriteString(pAssemblyFilePath);
    }
    else
    {
        m_fileStream.write(0);
    }

    m_entryCount++;
    m_fileStream.flush();
}

void CelWriter::WriteClassLoadFinished(ClassID classId, ModuleID moduleId, gtUInt64 systime,
                                       unsigned int cSize, const wchar_t* pClassName)
{
    gtUInt32 eventType = evClassLoadFinished;
    m_fileStream.write(eventType);

    gtUInt64 tData = static_cast<gtUInt64>(classId);
    m_fileStream.write(tData);

    tData = static_cast<gtUInt64>(moduleId);
    m_fileStream.write(tData);

    m_fileStream.write(systime);
    m_fileStream.write(cSize);
    m_fileStream.write(pClassName, sizeof(wchar_t) * cSize);
    m_entryCount++;
    m_fileStream.flush();
}

void CelWriter::WriteClassUnloadStarted(ClassID classId, gtUInt64 systime)
{
    gtUInt32 eventType = evClassUnloadStarted;
    m_fileStream.write(eventType);

    gtUInt64 tData = static_cast<gtUInt64>(classId);
    m_fileStream.write(tData);

    m_fileStream.write(systime);
    m_entryCount++;
    m_fileStream.flush();
}


void CelWriter::WriteJITCompilationFinished(ModuleID moduleId, FunctionID funcId,
                                            const wchar_t* pClassName, const wchar_t* pFuncName, const wchar_t* pJncFileName,
                                            gtUInt64 systime, LPCBYTE startAddr, unsigned int codeSize)
{
    gtUInt32 eventType = evJITCompilationFinished;
    m_fileStream.write(eventType);

    gtUInt64 tData = static_cast<gtUInt64>(moduleId);
    m_fileStream.write(tData);

    tData = static_cast<gtUInt64>(funcId);
    m_fileStream.write(tData);

    WriteString(pClassName);
    WriteString(pFuncName);
    WriteString(pJncFileName);

    m_fileStream.write(systime);
    gtUInt64 tAddress = reinterpret_cast<gtUInt64>(startAddr);
    m_fileStream.write(tAddress);
    m_fileStream.write(codeSize);
    m_entryCount++;
    m_fileStream.flush();
}

void CelWriter::ReWriteJITCompilation(ModuleID moduleId, FunctionID funcId,
                                      const wchar_t* pClassName, const wchar_t* pFuncName, const wchar_t* pJncFileName,
                                      gtUInt64 systime, gtUInt64 startAddr, unsigned int codeSize)
{
    gtUInt32 eventType = evJITCompilationFinished;
    m_fileStream.write(eventType);

    gtUInt64 tData = static_cast<gtUInt64>(moduleId);
    m_fileStream.write(tData);

    tData = static_cast<gtUInt64>(funcId);
    m_fileStream.write(tData);

    WriteString(pClassName);
    WriteString(pFuncName);
    WriteString(pJncFileName);

    m_fileStream.write(systime);
    gtUInt64 tAddress = startAddr;
    m_fileStream.write(tAddress);
    m_fileStream.write(codeSize);
    m_entryCount++;
    m_fileStream.flush();
}

void CelWriter::WriteFunctionUnloadStarted(FunctionID funcId, gtUInt64 systime)
{
    gtUInt32 eventType = evFunctionUnloadStarted;
    m_fileStream.write(eventType);

    gtUInt64 tData = static_cast<gtUInt64>(funcId);
    m_fileStream.write(tData);

    m_fileStream.write(systime);
    m_entryCount++;
    m_fileStream.flush();
}

void CelWriter::WriteString(const wchar_t* pString)
{
    gtUInt32 cSize = static_cast<gtUInt32>(wcslen(pString));
    m_fileStream.write(cSize);
    m_fileStream.write(pString, sizeof(wchar_t) * cSize);
}
