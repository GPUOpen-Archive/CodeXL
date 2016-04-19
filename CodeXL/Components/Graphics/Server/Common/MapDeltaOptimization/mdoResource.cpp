//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Resource wrapper implementation file for MDO.
///         This class contains most of the low-level logic used by MDO. Here we
///         page-guard memory, track maps and unmaps, and implement two
///         methods of delta storage and reconstruction.
//==============================================================================

#include "mdoResource.h"

#include "mdoStats.h"

/**
**************************************************************************************************
*   MdoResource::MdoResource
*
*   @brief
*       Create a new MdoResource
**************************************************************************************************
*/
MdoResource::MdoResource() :
    m_captureMapId(-1),
    m_playbackMapId(0),
    m_state(MDO_STATE_CAPTURE_START),
    m_size(0),
    m_activeMappedCount(0)
{
    memset(&m_createInfo, 0, sizeof(m_createInfo));
    memset(&m_reflectionData, 0, sizeof(m_reflectionData));
}

/**
**************************************************************************************************
*   MdoResource::~MdoResource
*
*   @brief
*       Destroy the MdoResource
**************************************************************************************************
*/
MdoResource::~MdoResource()
{
    ResetMapEvents();
    DeleteReflectionData();
}

/**
**************************************************************************************************
*   MdoResource::NewReflectionData
*
*   @brief
*       Create the buffer of memory which we provide to the app (instead of driver mem)
**************************************************************************************************
*/
void MdoResource::NewReflectionData()
{
    m_reflectionData.pReferenceData = new unsigned char[m_size];
    m_reflectionData.pNewData = new unsigned char[m_size];

    if (m_createInfo.mdoConfig.deltaStorage == MDO_DELTA_STORAGE_PER_BYTE)
    {
        m_reflectionData.pHistories = new SlotHistories[m_size];
    }

    memset(m_reflectionData.pReferenceData, 0, m_size);
    memset(m_reflectionData.pNewData, 0, m_size);
}

/**
**************************************************************************************************
*   MdoResource::DeleteRefData
*
*   @brief
*       Clean up
**************************************************************************************************
*/
void MdoResource::DeleteReflectionData()
{
    MDO_SAFE_DELETE_ARRAY(m_reflectionData.pReferenceData);
    MDO_SAFE_DELETE_ARRAY(m_reflectionData.pNewData);

    if (m_createInfo.mdoConfig.deltaStorage == MDO_DELTA_STORAGE_PER_BYTE)
    {
        MDO_SAFE_DELETE_ARRAY(m_reflectionData.pHistories);
    }
}

/**
**************************************************************************************************
*   MdoResource::UpdateReferenceData
*
*   @brief
*       This backs up the lastly-written app data
**************************************************************************************************
*/
void MdoResource::UpdateReferenceData()
{
    memcpy(m_reflectionData.pReferenceData, m_reflectionData.pNewData, m_size);
}

/**
**************************************************************************************************
*   MdoResource::NewMapEvent
*   @brief
*       A new map is coming along, so we need to track it
*   @param mapInfo
**************************************************************************************************
*/
void MdoResource::NewMapEvent(const MdoResMapInfo& mapInfo)
{
    // If this is the first map, make sure we start with a clean slate
    if (m_state == MDO_STATE_CAPTURE_START)
    {
        ResetMapEvents();
    }

    m_captureMapId++;

    MapEvent deltaInfo = {};
    deltaInfo.mapInfo = mapInfo;

    // For debugging
    if (m_createInfo.mdoConfig.dbgMirrorAppMaps == true)
    {
        deltaInfo.pAppMapMirror = new unsigned char[m_size];

        memset(deltaInfo.pAppMapMirror, 0, m_size);
    }

    m_mapData[m_captureMapId] = deltaInfo;

    if (m_createInfo.mdoConfig.bypassExceptionFiltering == true)
    {
        DataChunk fullResource;
        fullResource.offset = 0;
        fullResource.size = m_size;
        fullResource.pData = nullptr;

        TrackDirtyPage(fullResource);
    }

    m_activeMappedCount++;

    m_state = MDO_STATE_CAPTURE;
}

/**
**************************************************************************************************
*   MdoResource::NewUnmapEvent
*
*   @brief
*       Register an unmap event
**************************************************************************************************
*/
void MdoResource::NewUnmapEvent()
{
    m_activeMappedCount--;
}

/**
**************************************************************************************************
*   MdoResource::CalcDeltaRegionsPerByteStorage
*
*   @brief
*       This method delta storage works by storing a delta history for each byte in the buffer.
*       Whenever a byte changes, a new "history" element is added for the current byte.
*       The history element contains a record of the map number and new data.
**************************************************************************************************
*/
void MdoResource::CalcDeltaRegionsPerByteStorage()
{
    MapEvent& currMapEvent = m_mapData[m_captureMapId];

    if (m_createInfo.mdoConfig.dbgMirrorAppMaps == true)
    {
        memcpy(currMapEvent.pAppMapMirror, m_reflectionData.pNewData, m_size);
    }

    // On the first map, we always want the full buffer
    if (m_captureMapId == 0)
    {
        for (UINT32 i = 0; i < m_size; i++)
        {
            SlotHistory newHistory;
            newHistory.mapId = m_captureMapId;
            newHistory.data = m_reflectionData.pNewData[i];

            m_reflectionData.pHistories[i].push_back(newHistory);
        }
    }

    // Not the first map on this capture run
    else
    {
        // Go through all pages
        for (UINT32 i = 0; i < currMapEvent.dirtyPages.size(); i++)
        {
            DataChunks& dirtyPages = currMapEvent.dirtyPages;
            DataChunk& currPage = dirtyPages[i];

            const UINT32 pageBoundary = currPage.offset + currPage.size;

            // Check all bytes of current dirty page
            for (UINT32 j = currPage.offset; j < pageBoundary; j++)
            {
                // Add a new byte history element if there was a change
                if (m_reflectionData.pNewData[j] != m_reflectionData.pReferenceData[j])
                {
                    SlotHistory newHistory;
                    newHistory.mapId = m_captureMapId;
                    newHistory.data = m_reflectionData.pNewData[j];

                    m_reflectionData.pHistories[j].push_back(newHistory);
                }
            }
        }
    }
}

/**
**************************************************************************************************
*   MdoResource::CalcDeltaRegionsPerMapStorage
*
*   @brief
*       This method delta storage works by storing ranges of changed bytes for the current map.
*       When a dirty region is detected, it is stored and associated with the current map ID.
**************************************************************************************************
*/
void MdoResource::CalcDeltaRegionsPerMapStorage()
{
    MapEvent& currMapEvent = m_mapData[m_captureMapId];

    if (m_createInfo.mdoConfig.dbgMirrorAppMaps == true)
    {
        memcpy(currMapEvent.pAppMapMirror, m_reflectionData.pNewData, m_size);
    }

    DataChunks& deltas = currMapEvent.deltas;

    // On the first map, we always want the full buffer
    if (m_captureMapId == 0)
    {
        DataChunk fullDelta;
        fullDelta.offset = 0;
        fullDelta.size = m_size;
        fullDelta.pData = new unsigned char[m_size];

        memcpy(fullDelta.pData, m_reflectionData.pNewData, m_size);

        m_accumDeltas[m_captureMapId].push_back(fullDelta);

        deltas.push_back(fullDelta);
    }

    // Not the first map on this capture run
    else
    {
        int currDelta = -1;

        // Go through all pages
        for (UINT32 i = 0; i < currMapEvent.dirtyPages.size(); i++)
        {
            DataChunks& dirtyPages = currMapEvent.dirtyPages;
            DataChunk& currPage = dirtyPages[i];

            const UINT32 pageBoundary = currPage.offset + currPage.size;
            bool parsingDeltaRegion = false;

            // Check all bytes of current dirty page
            for (UINT32 j = currPage.offset; j < pageBoundary; j++)
            {
                if (m_reflectionData.pNewData[j] != m_reflectionData.pReferenceData[j])
                {
                    // Starting a new dirty region
                    if (parsingDeltaRegion == false)
                    {
                        DataChunk newDeltaRange;
                        newDeltaRange.offset = j;
                        newDeltaRange.size = 1;
                        newDeltaRange.pData = nullptr;

                        deltas.push_back(newDeltaRange);
                        currDelta++;

                        parsingDeltaRegion = true;
                    }
                    else
                    {
                        // Increase the number of changed bytes this region
                        if (currDelta != -1)
                        {
                            deltas[currDelta].size++;
                        }
                    }
                }
                else
                {
                    parsingDeltaRegion = false;
                }
            }
        }

        // Store all identified delta regions
        for (UINT32 j = 0; j < deltas.size(); j++)
        {
            DataChunk& currDelta1 = deltas[j];
            currDelta1.pData = new unsigned char[currDelta1.size];

            memcpy(currDelta1.pData, m_reflectionData.pNewData + currDelta1.offset, (size_t)currDelta1.size);

            m_accumDeltas[m_captureMapId].push_back(currDelta1);
        }
    }
}

/**
**************************************************************************************************
*   MdoResource::CalcDeltaRegions
*
*   @brief
*       Switch between the two delta-calculating methods
**************************************************************************************************
*/
void MdoResource::CalcDeltaRegions()
{
    MDO_ASSERT(m_captureMapId >= 0);

    if (m_createInfo.mdoConfig.deltaStorage == MDO_DELTA_STORAGE_PER_BYTE)
    {
        CalcDeltaRegionsPerByteStorage();
    }
    else
    {
        CalcDeltaRegionsPerMapStorage();
    }
}

/**
**************************************************************************************************
*   MdoResource::UploadMapDelta
*
*   @brief
*       It's time to replay our data, which means this function has to look through the buffer
*       history and reconstruct the buffer as each map happened. Reconstruction is different
*       depending on which delta storage mechanism was chosen.
*
*   @return
*       True if successful
**************************************************************************************************
*/
bool MdoResource::UploadMapDelta()
{
    MDO_ASSERT((int)m_playbackMapId <= m_captureMapId);

    bool success = false;

    if (m_mapData.count(m_playbackMapId) != 0)
    {
        MapEvent& currMapEvent = m_mapData[m_playbackMapId];
        MdoResMapInfo& mapInfo = currMapEvent.mapInfo;

        void* pDriverMem = nullptr;
        success = Map(mapInfo, &pDriverMem);

        if (success == true)
        {
            // Per-byte delta storage method
            if (m_createInfo.mdoConfig.deltaStorage == MDO_DELTA_STORAGE_PER_BYTE)
            {
                char* pDriverLoc = nullptr;

                // For each byte in the buffer...
                for (UINT32 i = 0; i < m_size; i++)
                {
                    SlotHistories& currHistories = m_reflectionData.pHistories[i];

                    pDriverLoc = (char*)pDriverMem + i;

                    UINT32 historyIdx = 0;

                    // Identify the correct delta to use
                    // It must be the latest delta, up until the current map id
                    for (UINT32 j = 0; j < currHistories.size(); j++)
                    {
                        SlotHistory& currHistory = currHistories[j];

                        if ((int)currHistory.mapId <= m_playbackMapId)
                        {
                            historyIdx = j;
                        }
                        else
                        {
                            break;
                        }
                    }

                    // Write out the changed byte
                    *pDriverLoc = currHistories[historyIdx].data;
                }
            }

            // Per-map delta storage method
            else
            {
                memset(pDriverMem, 0, m_size);

                // For each map that happened to this buffer...
                for (std::map<int, DataChunks>::iterator it = m_accumDeltas.begin(); it != m_accumDeltas.end(); it++)
                {
                    // Write out the delta range, up until reaching the current map id
                    // This will likely write a the byte more than once, so it's not as efficient as the prior method
                    if (it->first <= m_playbackMapId)
                    {
                        DataChunks& currChunks = it->second;

                        for (UINT32 i = 0; i < currChunks.size(); i++)
                        {
                            memcpy((char*)pDriverMem + currChunks[i].offset, currChunks[i].pData, (size_t)currChunks[i].size);
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }

            // For debugging
            if (m_createInfo.mdoConfig.dbgMirrorAppMaps == true)
            {
                // Break if we found a difference between app data and our reconstructed data
                if (memcmp(pDriverMem, currMapEvent.pAppMapMirror, m_size) != 0)
                {
                    MDO_ASSERT_ALWAYS();
                }
            }
        }
    }

    m_playbackMapId++;

    m_state = MDO_STATE_PLAYBACK;

    // Played back the last map, so get ready to reset
    if (m_playbackMapId > m_captureMapId)
    {
        m_state = MDO_STATE_CAPTURE_START;

        m_playbackMapId = 0;
    }

    // We assume the Unmap will be done by the app here next

    return success;
}

/**
**************************************************************************************************
*   MdoResource::TrackDirtyPage
*
*   @brief
*       This tells the resource that the a range of bytes was touched during this map.
*       It gets called by the OS's exception handler.
*   @param dirtyPage Page to track
**************************************************************************************************
*/
void MdoResource::TrackDirtyPage(const DataChunk& dirtyPage)
{
    MDO_ASSERT(m_captureMapId >= 0);
    MDO_ASSERT(dirtyPage.size != 0);

    if (m_createInfo.mdoConfig.bypassExceptionFiltering == true)
    {
        if (m_mapData[m_captureMapId].dirtyPages.size() == 0)
        {
            m_mapData[m_captureMapId].dirtyPages.push_back(dirtyPage);
        }
        else
        {
            m_mapData[m_captureMapId].dirtyPages[0] = dirtyPage;
        }
    }
    else
    {
        m_mapData[m_captureMapId].dirtyPages.push_back(dirtyPage);
    }
}

/**
**************************************************************************************************
*   MdoResource::ResetMapEvents
*
*   @brief
*       Reset this resource and free its heap mem
**************************************************************************************************
*/
void MdoResource::ResetMapEvents()
{
    for (MapEvents::iterator it = m_mapData.begin(); it != m_mapData.end(); it++)
    {
        MapEvent& currMapData = it->second;

        for (UINT32 i = 0; i < currMapData.dirtyPages.size(); i++)
        {
            MDO_SAFE_DELETE_ARRAY(currMapData.dirtyPages[i].pData);
        }

        for (UINT32 i = 0; i < currMapData.deltas.size(); i++)
        {
            MDO_SAFE_DELETE_ARRAY(currMapData.deltas[i].pData);
        }

        if (m_createInfo.mdoConfig.dbgMirrorAppMaps == true)
        {
            MDO_SAFE_DELETE_ARRAY(currMapData.pAppMapMirror);
        }
    }

    if (m_createInfo.mdoConfig.deltaStorage == MDO_DELTA_STORAGE_PER_BYTE)
    {
        for (UINT32 i = 0; i < m_size; i++)
        {
            m_reflectionData.pHistories[i].clear();
        }
    }
    else
    {
        m_accumDeltas.clear();
    }

    m_mapData.clear();

    m_captureMapId = -1;
    m_playbackMapId = 0;
}

/**
**************************************************************************************************
*   MdoResource::Guard
*
*   @brief
*       Guard the memory that is provided from this resource for app writing.
*
*   @return
*       True if successful
**************************************************************************************************
*/
bool MdoResource::Guard()
{
    BOOL result = FALSE;

    if (m_createInfo.mdoConfig.bypassExceptionFiltering == false)
    {
        if (m_activeMappedCount == 1)
        {
            DWORD oldProtect = 0;
            result = VirtualProtect(m_reflectionData.pNewData, m_size, PAGE_READWRITE | PAGE_GUARD, &oldProtect);
            MDO_ASSERT(result == TRUE);
        }
    }
    else
    {
        result = TRUE;
    }

    return (result == TRUE) ? true : false;
}

/**
**************************************************************************************************
*   MdoResource::Unguard
*
*   @brief
*       Unguard the memory that is provided from this resource for app writing.
*
*   @return
*       True if successful
**************************************************************************************************
*/
bool MdoResource::Unguard()
{
    BOOL result = FALSE;

    if (m_createInfo.mdoConfig.bypassExceptionFiltering == false)
    {
        if (m_activeMappedCount == 1)
        {
            DWORD oldProtect = 0;
            result = VirtualProtect(m_reflectionData.pNewData, m_size, PAGE_READWRITE, &oldProtect);
            MDO_ASSERT(result == TRUE);
        }
    }
    else
    {
        result = TRUE;
    }

    return (result == TRUE) ? true : false;
}

/**
**************************************************************************************************
*   MdoResource::OriginalMapSuccessful
*
*   @brief
*       Determine if one of the app's map calls was successful
*
*   @return
*       True if the map call was successful
**************************************************************************************************
*/
bool MdoResource::OriginalMapSuccessful()
{
    MDO_ASSERT((int)m_playbackMapId <= m_captureMapId);

    bool success = false;

    if (m_mapData.count(m_playbackMapId) != 0)
    {
        if (m_mapData[m_playbackMapId].mapInfo.pDriverMem != nullptr)
        {
            success = true;
        }
    }

    return success;
}
