//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ProcessWorkingSet.h
///
//==================================================================================

#ifndef _PROCESSWORKINGSET_H_
#define _PROCESSWORKINGSET_H_

#include "ExecutableFile.h"

class osReadWriteLock;

template <typename T>
struct TRange
{
    T m_min;
    T m_max;

    bool operator<(const TRange& range) const
    {
        return m_max < range.m_min;
    }

    bool operator==(const TRange& range) const
    {
        return 0 == memcmp(this, &range, sizeof(*this));
    }

    bool operator!=(const TRange& range) const
    {
        return !(*this == range);
    }
};

typedef TRange<gtVAddr> VAddrRange;
typedef TRange<gtRVAddr> RVAddrRange;

class EXE_API ProcessWorkingSet
{
private:
    typedef gtMap<VAddrRange, ExecutableFile*> ModulesMap;

    ModulesMap m_modulesMap;
    union
    {
        gtUIntPtr m_noSymbols;
        wchar_t* m_pSearchPath;
    };
    wchar_t* m_pServerList;
    wchar_t* m_pCachePath;
    osReadWriteLock* m_pLock;

    bool LoadModule(ExecutableFile* pExe, gtVAddr baseVa) const;
    ExecutableFile* FindModuleInternal(gtVAddr va, osReadWriteLock* pLock);

public:
    explicit ProcessWorkingSet(bool initSymbolEngine,
                               const wchar_t* pSearchPath = NULL,
                               const wchar_t* pServerList = NULL,
                               const wchar_t* pCachePath = NULL,
                               osReadWriteLock* pLock = NULL);

    ~ProcessWorkingSet();

    ExecutableFile* AddModule(gtVAddr imageBase, gtUInt32 imageSize, const wchar_t* pImageName);
    ExecutableFile* FindModule(gtVAddr va);

    void Clear();

    void SetLock(osReadWriteLock* pLock);
    osReadWriteLock* GetLock() const;

    bool IsInitializingSymbols() const;
    void InitializeSymbols(bool enable);
    void SetSymbolsSearchPath(const wchar_t* pSearchPath = NULL,
                              const wchar_t* pServerList = NULL,
                              const wchar_t* pCachePath = NULL);

    unsigned GetModulesCount() const { return static_cast<unsigned>(m_modulesMap.size()); }

    typedef ModulesMap::const_iterator const_iterator;
    typedef ModulesMap::iterator             iterator;

    const_iterator begin() const { return m_modulesMap.begin(); }
    iterator begin()       { return m_modulesMap.begin(); }
    const_iterator end() const { return m_modulesMap.end(); }
    iterator end()       { return m_modulesMap.end(); }
};

#endif // _PROCESSWORKINGSET_H_
