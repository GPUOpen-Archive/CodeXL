//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ProcessWorkingSet.cpp
///
//==================================================================================

#include <ProcessWorkingSet.h>
#include <AMDTOSWrappers/Include/osReadWriteLock.h>

#include <ElfFile.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <PeFile.h>
#endif

static wchar_t* DuplicatePathList(const wchar_t* pPathList);
static bool IsPathListEmpty(const wchar_t* pPathList);

static inline void LockRead(osReadWriteLock* pLock);
static inline void UnlockRead(osReadWriteLock* pLock);
static inline void LockWrite(osReadWriteLock* pLock);
static inline void UnlockWrite(osReadWriteLock* pLock);


ProcessWorkingSet::ProcessWorkingSet(bool initSymbolEngine,
                                     const wchar_t* pSearchPath,
                                     const wchar_t* pServerList,
                                     const wchar_t* pCachePath,
                                     osReadWriteLock* pLock)
{
    m_pSearchPath = DuplicatePathList(pSearchPath);
    m_pServerList = DuplicatePathList(pServerList);
    m_pCachePath  = DuplicatePathList(pCachePath);

    InitializeSymbols(initSymbolEngine);
    m_pLock = pLock;
}

ProcessWorkingSet::~ProcessWorkingSet()
{
    Clear();

    m_noSymbols &= ~gtUIntPtr(1);

    if (NULL != m_pSearchPath)
    {
        free(m_pSearchPath);
    }

    if (NULL != m_pServerList)
    {
        free(m_pServerList);
    }

    if (NULL != m_pCachePath)
    {
        free(m_pCachePath);
    }
}

void ProcessWorkingSet::SetLock(osReadWriteLock* pLock)
{
    m_pLock = pLock;
}

osReadWriteLock* ProcessWorkingSet::GetLock() const
{
    return m_pLock;
}

bool ProcessWorkingSet::IsInitializingSymbols() const
{
    return gtUIntPtr(0) == (m_noSymbols & gtUIntPtr(1));
}

void ProcessWorkingSet::InitializeSymbols(bool enable)
{
    m_noSymbols = (m_noSymbols & (~gtUIntPtr(1))) | static_cast<gtUIntPtr>(!enable);
}

void ProcessWorkingSet::SetSymbolsSearchPath(const wchar_t* pSearchPath, const wchar_t* pServerList, const wchar_t* pCachePath)
{
    LockWrite(m_pLock);

    gtUIntPtr initSymbolsFlag = (m_noSymbols & gtUIntPtr(1));
    m_noSymbols &= ~gtUIntPtr(1);

    if (NULL != m_pSearchPath)
    {
        free(m_pSearchPath);
    }

    if (NULL != m_pServerList)
    {
        free(m_pServerList);
    }

    if (NULL != m_pCachePath)
    {
        free(m_pCachePath);
    }

    m_pSearchPath = DuplicatePathList(pSearchPath);
    m_pServerList = DuplicatePathList(pServerList);
    m_pCachePath  = DuplicatePathList(pCachePath);

    m_noSymbols |= initSymbolsFlag;

    UnlockWrite(m_pLock);
}

ExecutableFile* ProcessWorkingSet::AddModule(gtVAddr imageBase, gtUInt32 imageSize, const wchar_t* pImageName)
{
    ExecutableFile* pExe;

    VAddrRange range = { imageBase, imageBase };
    LockRead(m_pLock);
    ModulesMap::iterator it = m_modulesMap.find(range);
    bool addNew = (it == m_modulesMap.end());

    if (addNew)
    {
        if (NULL != m_pLock)
        {
            m_pLock->unlockRead();
            m_pLock->lockWrite();
            it = m_modulesMap.find(range);
            addNew = (it == m_modulesMap.end());
        }

        if (addNew)
        {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            pExe = new PeFile(pImageName);
#else
            pExe = new ElfFile(pImageName);
#endif

            if (0 == imageSize)
            {
                if (pExe->Open(imageBase))
                {
                    imageSize = pExe->GetImageSize();
                    pExe->Close();
                }
                else
                {
                    delete pExe;
                    pExe = NULL;
                }
            }

            if (NULL != pExe)
            {
                range.m_max += static_cast<gtVAddr>(imageSize - 1);
                m_modulesMap.insert(ModulesMap::value_type(range, pExe));
            }
        }
        else
        {
            pExe = NULL;
        }

        UnlockWrite(m_pLock);
    }
    else
    {
        gtVAddr imageMax = imageBase + static_cast<gtVAddr>(imageSize - 1);
        pExe = (it->first.m_max == imageMax) ? it->second : NULL;

        UnlockRead(m_pLock);
    }

    return pExe;
}

ExecutableFile* ProcessWorkingSet::FindModule(gtVAddr va)
{
    return FindModuleInternal(va, m_pLock);
}

ExecutableFile* ProcessWorkingSet::FindModuleInternal(gtVAddr va, osReadWriteLock* pLock)
{
    ExecutableFile* pExe = NULL;
    VAddrRange range = { va, va };

    LockRead(pLock);
    ModulesMap::iterator it = m_modulesMap.find(range);
    bool ret = it != m_modulesMap.end();

    if (ret)
    {
        pExe = it->second;
        ret = (NULL != pExe && !pExe->IsOpen());
        UnlockRead(pLock);

        if (ret)
        {
            if (NULL != pLock)
            {
                pLock->lockWrite();
                pExe = FindModuleInternal(va, NULL);
                pLock->unlockWrite();
            }
            else if (!LoadModule(pExe, it->first.m_min))
            {
                delete pExe;
                m_modulesMap.erase(it);
                pExe = NULL;
            }
        }
    }
    else
    {
        UnlockRead(pLock);
    }

    return pExe;
}

void ProcessWorkingSet::Clear()
{
    LockWrite(m_pLock);

    for (ModulesMap::iterator it = m_modulesMap.begin(), itEnd = m_modulesMap.end(); it != itEnd; ++it)
    {
        ExecutableFile* pExe = it->second;

        if (NULL != pExe)
        {
            delete pExe;
        }
    }

    m_modulesMap.clear();

    UnlockWrite(m_pLock);
}

bool ProcessWorkingSet::LoadModule(ExecutableFile* pExe, gtVAddr baseVa) const
{
    bool ret = pExe->Open(baseVa);

    if (ret && IsInitializingSymbols())
    {
        pExe->InitializeSymbolEngine(m_pSearchPath, m_pServerList, m_pCachePath);
    }

    return ret;
}


static wchar_t* DuplicatePathList(const wchar_t* pPathList)
{
    wchar_t* pDupPathList;

    if (NULL == pPathList || IsPathListEmpty(pPathList))
    {
        pDupPathList = NULL;
    }
    else
    {
        pDupPathList = wcsdup(pPathList);
    }

    return pDupPathList;
}

static bool IsPathListEmpty(const wchar_t* pPathList)
{
    unsigned i = 0U;

    while ('\0' != pPathList[i])
    {
        if (!isspace(pPathList[i]) && ';' != pPathList[i])
        {
            break;
        }

        i++;
    }

    return ('\0' == pPathList[i]);
}


static inline void LockRead(osReadWriteLock* pLock)
{
    if (NULL != pLock)
    {
        pLock->lockRead();
    }
}

static inline void UnlockRead(osReadWriteLock* pLock)
{
    if (NULL != pLock)
    {
        pLock->unlockRead();
    }
}

static inline void LockWrite(osReadWriteLock* pLock)
{
    if (NULL != pLock)
    {
        pLock->lockWrite();
    }
}

static inline void UnlockWrite(osReadWriteLock* pLock)
{
    if (NULL != pLock)
    {
        pLock->unlockWrite();
    }
}
