//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  MDO manager header file.
///         This is the highest level class of MDO. The goal is to track, store,
///         and read-back resource deltas as they change map-over-map. It
///         also implements page-tracking using page guards, in order to determine
///         which memory regions have been touched, and therefore minimize
///         expensive delta calculations.
//==============================================================================

#ifndef __MDO_MANAGER_H__
#define __MDO_MANAGER_H__

#include "mdoUtil.h"

class MdoResourceCache;
class MdoStats;

/// Manager class to track the main map/unmap activities.
class MdoManager
{
public:
    static LONG WINAPI ExceptionFilter(_EXCEPTION_POINTERS* pException);
    static MdoManager* Instance();

    /// Destructor
    ~MdoManager() {}

    // Capture code
    bool Capture_PostResourceMap(void* pDevice, MdoResMapInfo& mapInfo, void** pAddr);
    bool Capture_PreResourceUnmap(UINT64 resHandle);

    /// Playback code
    bool Playback_PostResourceMap(UINT64 resHandle);

    /// We may need to also track creates/destroys, in case the API returns the same handle twice
    void TrackCreate(UINT64 resHandle);
    void TrackDestroy(UINT64 resHandle);

    /// Hold our exception filter logic
    void ExceptionFilter(UINT64 exceptionAddress);

    /// Create per-API resource cache
    /// \param mdoConfig MDO config
    /// \return pointer to teh resource cache
    virtual MdoResourceCache* CreateResourceCache(const MdoConfig& mdoConfig)
    {
        UNREFERENCED_PARAMETER(mdoConfig);
        return nullptr;
    }

protected:
    bool Init(const MdoConfig& mdoConfig);
    MdoManager() : m_pResourceCache(nullptr), m_pMdoStats(nullptr), m_pageSize(0) {}

private:
    MdoConfig         m_mdoConfig; ///< MDO config
    MdoResourceCache* m_pResourceCache; ///<  Resource cache
    MdoStats*         m_pMdoStats; ///< MDO stats
    MdoMutex          m_mtx; ///< MDO mutex
    UINT32            m_pageSize; ///<  Page size
};

#endif