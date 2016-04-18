//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  MDO manager implementation file.
///         This is the highest level class of MDO. The goal is to track, store,
///         and read-back resource deltas as they change map-over-map. It
///         also implements page-tracking using page guards, in order to determine
///         which memory regions have been touched, and therefore minimize
///         expensive delta calculations.
//==============================================================================

#include "mdoManager.h"

#include "mdoResource.h"
#include "mdoResourceCache.h"
#include "mdoStats.h"

/// Global instance of MDO manager
static MdoManager* g_pInstance = nullptr;

/**
***************************************************************************************************
*   MdoManager::Instance
*
*   @brief
*       Return the MdoManager instance
*
*   @return
*       MdoManager ptr
***************************************************************************************************
*/
MdoManager* MdoManager::Instance()
{
    return g_pInstance;
}

/**
**************************************************************************************************
*   MdoManager::ExceptionFilter
*
*   @brief
*       This gets called when the app writes to guarded memory
*   @param pException Exception pointers address
**************************************************************************************************
*/
LONG WINAPI MdoManager::ExceptionFilter(_EXCEPTION_POINTERS* pException)
{
    if (pException->ExceptionRecord->ExceptionCode == STATUS_GUARD_PAGE_VIOLATION)
    {
        g_pInstance->ExceptionFilter((UINT64)pException->ExceptionRecord->ExceptionInformation[1]);

        return EXCEPTION_CONTINUE_EXECUTION;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

/**
**************************************************************************************************
*   MdoManager::ExceptionFilter
*
*   @brief
*       This logic determines and tracks the dirty range within a resource.
*       It is specific to the current page that this exception landed in
*   @param exceptionAddress Exception address
**************************************************************************************************
*/
void MdoManager::ExceptionFilter(UINT64 exceptionAddress)
{
    MdoResource* pHomeResource = m_pResourceCache->FindResource(exceptionAddress);

    if (pHomeResource != nullptr)
    {
        const UINT64 deltaBegin = exceptionAddress;
        UINT64 deltaEnd = exceptionAddress + m_pageSize;

        const UINT64 pageAlignedExceptionAddr = MdoUtil::Pow2Align(exceptionAddress, m_pageSize);

        // The very first touched region won't likely be page-aligned
        if (pageAlignedExceptionAddr != deltaBegin)
        {
            deltaEnd = pageAlignedExceptionAddr;
        }

        const UINT32 deltaRange = (UINT32)(deltaEnd - deltaBegin);
        const UINT32 resourceSize = pHomeResource->ResourceByteSize();

        UINT32 dirtyBytes = resourceSize;

        if (dirtyBytes > deltaRange)
        {
            dirtyBytes = deltaRange;
        }

        ReflectionData* pReflData = pHomeResource->GetReflectionData();

        const UINT64 resAddressBegin = (UINT64)pReflData->pNewData;
        const UINT32 offset = (UINT32)(deltaBegin - resAddressBegin);

        // Make sure we don't run off the end
        if (offset + dirtyBytes > resourceSize)
        {
            dirtyBytes = resourceSize - offset;
        }

        DataChunk dirty;
        dirty.offset = offset;
        dirty.size = dirtyBytes;
        dirty.pData = nullptr;

        pHomeResource->TrackDirtyPage(dirty);
    }
}

/**
**************************************************************************************************
*   MdoManager::Init
*
*   @brief
*       Init the MdoManager
*   @param mdoConfig Input config
*   @return
*       True if successful
**************************************************************************************************
*/
bool MdoManager::Init(const MdoConfig& mdoConfig)
{
    bool success = false;

    if ((mdoConfig.deltaStorage == MDO_DELTA_STORAGE_PER_MAP) ||
        (mdoConfig.deltaStorage == MDO_DELTA_STORAGE_PER_BYTE))
    {
        if (mdoConfig.bypassExceptionFiltering == false)
        {
            AddVectoredExceptionHandler(1, &MdoManager::ExceptionFilter);
            //SetUnhandledExceptionFilter(&MdoManager::ExceptionFilter);
        }

        SYSTEM_INFO si;
        GetSystemInfo(&si);
        m_pageSize = si.dwPageSize;

        memcpy(&m_mdoConfig, &mdoConfig, sizeof(mdoConfig));

        m_pResourceCache = CreateResourceCache(mdoConfig);

        if ((mdoConfig.dbgMdoSpaceUsage == true) || (mdoConfig.dbgMdoTimeUsage == true))
        {
            m_pMdoStats = MdoStats::Create(mdoConfig);
        }

        if (m_pResourceCache != nullptr)
        {
            success = true;

            g_pInstance = this;
        }
    }

    return success;
}

/**
**************************************************************************************************
*   MdoManager::Capture_PostResourceMap
*
*   @brief
*       Capture-time operations, to be called right after the driver call to map a resource
*   @param pDevice Device
*   @param mapInfo Map info
*   @param pAddr Pointer to the reflection data
*   @return
*       True if successful
**************************************************************************************************
*/
bool MdoManager::Capture_PostResourceMap(
    void*          pDevice,
    MdoResMapInfo& mapInfo,
    void**         pAddr)
{
    ScopedLock lock(&m_mtx);

    bool success = false;

    // Track whether the app's map call failed
    mapInfo.pDriverMem = *pAddr;

    MdoResource* pMdoResource = m_pResourceCache->GetMdoResource(mapInfo.resHandle, pDevice, true);

    if (pMdoResource != nullptr)
    {
        pMdoResource->NewMapEvent(mapInfo);

        if (pMdoResource->RunMdoCaptureWork() == true)
        {
            // App got a good ptr from the driver
            if (mapInfo.pDriverMem != nullptr)
            {
                // The app intends to map this resource, so backup current data
                pMdoResource->UpdateReferenceData();

                ReflectionData* pReflectionData = pMdoResource->GetReflectionData();

                // Provide app with our memory
                *pAddr = pReflectionData->pNewData;

                // Protect our memory with page guard
                success = pMdoResource->Guard();
            }
        }
    }

    return success;
}

/**
**************************************************************************************************
*   MdoManager::Capture_PreResourceUnmap
*
*   @brief
*       Capture-time operations, to be called right before the driver call to unmap a resource
*   @param resHandle Handle to the resource
*   @return
*       True if successful
**************************************************************************************************
*/
bool MdoManager::Capture_PreResourceUnmap(UINT64 resHandle)
{
    ScopedLock lock(&m_mtx);

    bool success = false;

    MdoResource* pMdoResource = m_pResourceCache->GetMdoResource(resHandle, nullptr, false);

    if (pMdoResource != nullptr)
    {
        if (pMdoResource->RunMdoCaptureWork() == true)
        {
            // Un-protect our memory so we can do stuff with it now
            success = pMdoResource->Unguard();

            if (success == true)
            {
                // Calculate deltas
                pMdoResource->CalcDeltaRegions();
            }
        }

        pMdoResource->NewUnmapEvent();
    }

    return success;
}

/**
**************************************************************************************************
*   MdoManager::Playback_PostResourceMap
*
*   @brief
*       Playback-time operations, to be called once done capturing and we want to upload data
*   @param resHandle Handle to the resource
*   @return
*       True if successful
**************************************************************************************************
*/
bool MdoManager::Playback_PostResourceMap(UINT64 resHandle)
{
    ScopedLock lock(&m_mtx);

    bool success = false;

    MdoResource* pMdoResource = m_pResourceCache->GetMdoResource(resHandle, nullptr, false);

    if (pMdoResource != nullptr)
    {
        if (pMdoResource->RunMdoPlaybackWork() == true)
        {
            // Make sure we only do work if the app's map was actually successful
            success = pMdoResource->OriginalMapSuccessful();

            if (success == true)
            {
                // Close out the original map call
                pMdoResource->Unmap();

                // Reconstruct and upload our captured deltas, as they happened overtime
                success = pMdoResource->UploadMapDelta();
            }
        }
    }

    return success;
}

/**
**************************************************************************************************
*   MdoManager::TrackCreate
*
*   @brief
*       Intended for next-gen APIs, where we will need to track allocations as they happen
*   @param resHandle Handle to the resource
**************************************************************************************************
*/
void MdoManager::TrackCreate(UINT64 resHandle)
{
    ScopedLock lock(&m_mtx);

    m_pResourceCache->TrackCreate(resHandle);
}

/**
**************************************************************************************************
*   MdoManager::TrackDestroy
*
*   @brief
*       Intended for next-gen APIs, where we will need to track allocations as they get destroyed
*   @param resHandle Handle to the resource
**************************************************************************************************
*/
void MdoManager::TrackDestroy(UINT64 resHandle)
{
    ScopedLock lock(&m_mtx);

    m_pResourceCache->TrackDestroy(resHandle);
}
