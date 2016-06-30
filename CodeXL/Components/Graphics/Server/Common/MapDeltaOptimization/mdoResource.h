//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Resource wrapper header file for MDO.
///         This class contains most of the low-level logic used by MDO. Here we
///         page-guard memory, track maps and unmaps, and implement two
///         methods of delta storage and reconstruction.
//==============================================================================

#ifndef __MDO_RESOURCE_H__
#define __MDO_RESOURCE_H__

#include "mdoUtil.h"

/// Resource wrapper header file for MDO.
class MdoResource
{
public:

    /// Destructor
    virtual ~MdoResource();

    /// Map member function
    /// \param mapInfo
    /// \param pMappedPtr
    /// \return True if success, false if failure
    virtual bool Map(const MdoResMapInfo& mapInfo, void** pMappedPtr) = 0;

    /// Unmap member function
    virtual bool Unmap() = 0;

    /// Skip any situations when we don't need to do any MDO capture work
    virtual bool RunMdoCaptureWork() = 0;

    /// Skip any situations when we don't need to do any MDO playback work
    virtual bool RunMdoPlaybackWork() = 0;

    void NewMapEvent(const MdoResMapInfo& mapInfo);
    void NewUnmapEvent();

    void CalcDeltaRegions();
    bool UploadMapDelta();

    /// Get the byte size of the resource
    /// \return The size in bytes
    UINT32 ResourceByteSize()
    {
        return m_size;
    }

    void DeleteReflectionData();
    void NewReflectionData();
    void UpdateReferenceData();

    void TrackDirtyPage(const DataChunk& deltaData);
    void ResetMapEvents();


    /// Get the reflection data
    /// \return Pointer to the reflection data
    ReflectionData* GetReflectionData()
    {
        return &m_reflectionData;
    }

    bool Guard();
    bool Unguard();

    bool OriginalMapSuccessful();

protected:
    MdoResource();

    void CalcDeltaRegionsPerByteStorage();
    void CalcDeltaRegionsPerMapStorage();

    MdoState              m_state; ///< Mdo state
    MdoResourceCreateInfo m_createInfo; ///< Resource creation info
    ReflectionData        m_reflectionData; ///< Reflection data
    MapEvents             m_mapData; ///< Map data
    OrderedAccumDeltas    m_accumDeltas; ///< Deltas

    int                   m_captureMapId; ///< capture map id
    int                   m_playbackMapId; ///< playback map id
    UINT32                m_size; ///< size
    int                   m_activeMappedCount; ///< map count
};

#endif