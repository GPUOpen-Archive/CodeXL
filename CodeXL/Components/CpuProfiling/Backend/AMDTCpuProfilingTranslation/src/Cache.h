//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Cache.h
/// \brief Implements a class which models cache behaviour
///
//==================================================================================

#ifndef _CACHE_H_
#define _CACHE_H_

#include <assert.h>
#include <map>

#include "CluInfo.h"

using namespace std;

enum CacheNotifications
{
    CACHE_NORMAL = 0,
    CACHE_EVICTED,
    CACHE_ERROR
};

class Cache
{
public:
    Cache();
    ~Cache();

    bool Init(UINT8 l1DcAssoc,
              UINT8 l1DcLineSize,
              UINT8 l1DcLinesPerTag,
              UINT8 l1DcSize);

    CacheNotifications CacheAccess(gtVAddr address,
                                   CacheDataStuff** data,
                                   unsigned char core);

    CacheDataStuff* GetNextValidLine(unsigned char& core, unsigned int& index);

    UINT64 GetTag(gtVAddr addr)
    {
        if (!m_isInitialized) { return MAX_UINT32; }

        return (addr >> m_tagShift) & m_tagMask;
    }

    UINT32 GetIndex(gtVAddr addr)
    {
        return (addr >> m_indexShift) & m_indexMask;
    }

    UINT32 GetOffset(gtVAddr addr)
    {
        return (addr >> m_offsetShift) & m_offsetMask;
    }

    UINT8 GetBytesPerLine()
    {
        return m_l1DcLineSize;
    }

    bool SpansLines(UINT32 offset, unsigned char size)
    {
        return ((offset + size - 1) & ~m_offsetMask) != 0;
    }

    inline UINT8 CountBits(UINT64 bitmap)
    {
        return CountBits((UINT32) bitmap) + CountBits((UINT32)(bitmap >> 32));
    }

    inline UINT8 CountBits(UINT32 bitmap)
    {
        bitmap = (bitmap & 0x55555555) + ((bitmap >>  1) & 0x55555555);
        bitmap = (bitmap & 0x33333333) + ((bitmap >>  2) & 0x33333333);
        bitmap = (bitmap & 0x0f0f0f0f) + ((bitmap >>  4) & 0x0f0f0f0f);
        bitmap = (bitmap & 0x00ff00ff) + ((bitmap >>  8) & 0x00ff00ff);
        return (bitmap & 0x0000ffff) + ((bitmap >> 16) & 0x0000ffff);
    }

    inline UINT8 CountBits(UINT8 bitmap)
    {
        bitmap = (bitmap & 0x55) + ((bitmap >>  1) & 0x55);
        bitmap = (bitmap & 0x33) + ((bitmap >>  2) & 0x33);
        return (bitmap & 0x0f) + ((bitmap >>  4) & 0x0f);
    }

    inline UINT8 ILog2(UINT32 x)
    {
        UINT32 l = 0;

        if (x >= (1 << 16)) { x >>= 16; l |= 16; }

        if (x >= (1 <<  8)) { x >>=  8; l |=  8; }

        if (x >= (1 <<  4)) { x >>=  4; l |=  4; }

        if (x >= (1 <<  2)) { x >>=  2; l |=  2; }

        if (x >= (1 <<  1)) {           l |=  1; }

        return l;
    }

private:
    CoreCacheMap*   m_pCoreCache;
    gtVAddr         m_tagMask;
    UINT32          m_tagShift;
    gtVAddr         m_indexMask;
    UINT32          m_indexShift;
    gtVAddr         m_offsetMask;
    UINT32          m_offsetShift;
    bool            m_isInitialized;
    UINT64          m_time;             // Used for LRU
    UINT8           m_l1DcAssoc;
    UINT8           m_l1DcSize;
    UINT8           m_l1DcLineSize;
};

#endif // _CACHE_H_
