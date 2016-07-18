//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Cache.cpp
/// \brief Implements a class which models cache behaviour
///
//==================================================================================

#include "Cache.h"


Cache::Cache()
{
    m_isInitialized = false;
    m_time = 0;     // For LRU
    m_pCoreCache = NULL;
}

Cache::~Cache()
{
    m_isInitialized = false;

    if (NULL != m_pCoreCache)
    {
        delete m_pCoreCache;
    }

    m_pCoreCache = NULL;
}

bool Cache::Init(UINT8 l1DcAssoc,
                 UINT8 l1DcLineSize,
                 UINT8 l1DcLinesPerTag,
                 UINT8 l1DcSize)
{
    // Currently only support <= 64 bytes per line with a single line per tag
    if ((l1DcLineSize > 64) || (l1DcLinesPerTag != 1))
    {
        assert((l1DcLineSize <= 64) || (l1DcLinesPerTag == 1));
        return false;
    }

    // Line Size should be a power of 2
    if (CountBits(l1DcLineSize) != 1)
    {
        assert(CountBits(l1DcLineSize) == 1);
        return false;
    }

    m_l1DcSize = l1DcSize;
    m_l1DcAssoc = l1DcAssoc;
    m_l1DcLineSize = l1DcLineSize;

    m_offsetMask = l1DcLineSize - 1;
    m_offsetShift = 0;

    UINT32 numLines = ((UINT32) l1DcSize * 1024) / l1DcAssoc / l1DcLineSize;
    m_indexShift = CountBits((UINT64) m_offsetMask);
    assert(CountBits(numLines) == 1);
    m_indexMask = numLines - 1;

    m_tagShift = m_indexShift + CountBits((UINT64) m_indexMask);
    m_tagMask = MAX_UINT64 >> m_tagShift;

    m_pCoreCache = new CoreCacheMap;
    m_isInitialized = (NULL != m_pCoreCache);

    return m_isInitialized;
}

CacheNotifications Cache::CacheAccess(gtVAddr address, CacheDataStuff** data, unsigned char core)
{
    if (!m_isInitialized)
    {
        return CACHE_ERROR;
    }

    // Decompose the address
    UINT64 tag = GetTag(address);
    UINT32 index = GetIndex(address);

    // See if we have accessed this core before
    CoreCacheMap::iterator coreIt = m_pCoreCache->find(core);

    if (coreIt == m_pCoreCache->end())
    {
        // First access from this core
        CacheMap cacheMap;
        coreIt = m_pCoreCache->insert(std::pair <unsigned char, CacheMap > (core, cacheMap)).first;
        assert(coreIt != m_pCoreCache->end());
    }

    CacheMap::iterator SetIt = coreIt->second.find(index);
    bool newLine = false;

    // If no lines allocated for this set, add a new $line
    if (SetIt == coreIt->second.end())
    {
        CacheDataStuff cacheDataStuff;  // Constructor initializes this

        cacheDataStuff.tag = tag;
        SetIt = coreIt->second.insert(CacheMap::value_type(index, cacheDataStuff));
        assert(SetIt != coreIt->second.end());
        newLine = true;
    }

    if (newLine)
    {
        // We added a new $line - all done
        SetIt->second.timestamp = m_time++; // Update access time for LRU
        *data = &(SetIt->second);
        return CACHE_NORMAL;
    }

    // We did not insert a new line.
    // Look for a matching tag.  If found, it's a hit
    // If it's not found, find a candidate for eviction (LRU, based on timestamp)
    CacheMap::iterator LRUIt = coreIt->second.end();
    UINT64 LRUValue = MAX_UINT64;
    bool bTagFound = false;

    while (SetIt->first == index)
    {
        if (SetIt->second.tag == tag)
        {
            // This is the one we're looking for - hit
            bTagFound = true;
            break;
        }

        if (SetIt->second.timestamp < LRUValue)
        {
            LRUIt = SetIt;
            LRUValue = SetIt->second.timestamp;
        }

        SetIt++;

        if (SetIt == coreIt->second.end())
        {
            break;
        }
    }

    if (bTagFound)
    {
        SetIt->second.timestamp = m_time++;
        *data = &(SetIt->second);
        return CACHE_NORMAL;
    }

    // Did not find the tag in this set
    // Check if we've allocated all the ways for this set
    if (coreIt->second.count(index) < m_l1DcAssoc)
    {
        CacheDataStuff cacheDataStuff;  // Constructor initializes this

        cacheDataStuff.tag = tag;
        cacheDataStuff.timestamp = m_time++;
        SetIt = coreIt->second.insert(CacheMap::value_type(index, cacheDataStuff));
        assert(SetIt != coreIt->second.end());
        *data = &(SetIt->second);
        return CACHE_NORMAL;
    }

    // Did not find the tag in the set.
    // Update the tag in the eviction candidate and the timestamp
    LRUIt->second.timestamp = m_time++;
    LRUIt->second.tag = tag;
    *data = &(LRUIt->second);

    return CACHE_EVICTED;
}


CacheDataStuff* Cache::GetNextValidLine(unsigned char& core, unsigned int& index)
{
    CoreCacheMap::iterator coreIt = m_pCoreCache->begin();

    if (coreIt == m_pCoreCache->end())
    {
        return NULL;
    }

    CacheMap::iterator SetIt = coreIt->second.begin();

    if (SetIt == coreIt->second.end())
    {
        return NULL;
    }

    if (MAX_UINT64 == SetIt->second.timestamp)
    {
        coreIt->second.erase(SetIt);
        return GetNextValidLine(core, index);
    }

    SetIt->second.timestamp = MAX_UINT64;
    core = coreIt->first;
    index = SetIt->first;

    return &(SetIt->second);
}
