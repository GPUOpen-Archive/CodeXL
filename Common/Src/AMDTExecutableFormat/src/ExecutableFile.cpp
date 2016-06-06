//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ExecutableFile.cpp
///
//==================================================================================

// Local:
#include <ExecutableFile.h>
#include "ElfFile.h"

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include "PeFile.h"
#endif

#include <AMDTOSWrappers/Include/osGeneralFunctions.h>

ExecutableFile::ExecutableFile(const wchar_t* pImageName) : m_loadAddress(GT_INVALID_VADDR),
    m_pSymbolEngine(NULL),
    m_processInlineInfo(false),
    m_aggrInlinedInstances(false)
{
    if (NULL != pImageName)
    {
        wcsncpy(m_modulePath, pImageName, OS_MAX_PATH - 1);
        m_modulePath[OS_MAX_PATH - 1] = L'\0';
    }
    else
    {
        m_modulePath[0] = L'\0';
    }
}

ExecutableFile::~ExecutableFile()
{
}

void ExecutableFile::Reset(const wchar_t* pImageName)
{
    Close();

    if (NULL != pImageName)
    {
        wcsncpy(m_modulePath, pImageName, OS_MAX_PATH - 1);
        m_modulePath[OS_MAX_PATH - 1] = L'\0';
    }
    else
    {
        m_modulePath[0] = L'\0';
    }
}

gtVAddr ExecutableFile::GetLoadAddress() const
{
    return m_loadAddress;
}

const wchar_t* ExecutableFile::GetFilePath() const
{
    return m_modulePath;
}

gtRVAddr ExecutableFile::VaToRva(gtVAddr va) const
{
    return static_cast<gtRVAddr>(va - m_loadAddress);
}

gtVAddr ExecutableFile::RvaToVa(gtRVAddr rva) const
{
    return static_cast<gtVAddr>(rva) + m_loadAddress;
}

SymbolEngine* ExecutableFile::GetSymbolEngine() const
{
    return m_pSymbolEngine;
}

bool ExecutableFile::IsSystemExecutable() const
{
    bool ret = false;

    if (m_modulePath[0] != L'\0')
    {
        ret = osIsSystemModule(m_modulePath);
    }

    return ret;
}

void ExecutableFile::SetProcessInlineInfo(bool processInlineInfo, bool aggrInlinedInstances)
{
    m_processInlineInfo = processInlineInfo;
    m_aggrInlinedInstances = aggrInlinedInstances;
}

bool ExecutableFile::IsProcessInlineInfo() const
{
    return m_processInlineInfo;
}

bool ExecutableFile::IsAggregateInlinedInstances() const
{
    return m_aggrInlinedInstances;
}

ExecutableFile* ExecutableFile::Open(const wchar_t* pImageName, gtVAddr loadAddress)
{
    ExecutableFile* pExe = nullptr;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    pExe = new PeFile(pImageName);

    if (NULL != pExe && !pExe->Open(loadAddress))
    {
        delete pExe;
#endif

        pExe = new ElfFile(pImageName);

        if (NULL != pExe && !pExe->Open(loadAddress))
        {
            delete pExe;
            pExe = nullptr;
        }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    }

#endif

    return pExe;
}