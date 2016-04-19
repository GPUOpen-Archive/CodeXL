//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Resource cache implementation file for MDO.
///         This class tracks MdoResource objects throughout their lifetime.
//==============================================================================

#include "mdoResourceCache.h"

#include "mdoResource.h"

/// MDO resource cache instance
static MdoResourceCache* g_pInstance = nullptr;

/**
***************************************************************************************************
*   MdoResourceCache::Instance
*
*   @brief
*       Get the MdoResourceCache instance
*
*   @return
*       MdoResourceCache ptr
***************************************************************************************************
*/
MdoResourceCache* MdoResourceCache::Instance()
{
    return g_pInstance;
}

/**
**************************************************************************************************
*   MdoResourceCache::Init
*
*   @brief
*       Init the MdoResourceCache
*   @param mdoConfig Input config
*   @return
*       True if successful
**************************************************************************************************
*/
bool MdoResourceCache::Init(const MdoConfig& mdoConfig)
{
    memcpy(&m_mdoConfig, &mdoConfig, sizeof(mdoConfig));

    g_pInstance = this;

    return true;
}

/**
**************************************************************************************************
*   MdoResourceCache::ResHandleExists
*
*   @brief
*       Check if a resource handle exists in the cache
*   @param resHandle Input resource handle
*   @return
*       True if the resource exists in the cache
**************************************************************************************************
*/
bool MdoResourceCache::ResHandleExists(UINT64 resHandle)
{
    return m_resourceList.count(resHandle) != 0;
}

/**
**************************************************************************************************
*   MdoResourceCache::UnregisterResource
*
*   @brief
*       Clean up and remove a MdoResource from the cache
*   @param resHandle Input resource handle
*   @return
*       True if successful
**************************************************************************************************
*/
bool MdoResourceCache::UnregisterResource(UINT64 resHandle)
{
    bool success = false;

    MdoResource* pMdoResource = GetMdoResource(resHandle, nullptr, false);

    if (pMdoResource != nullptr)
    {
        pMdoResource->ResetMapEvents();

        delete pMdoResource;

        m_resourceList.erase(resHandle);

        success = true;
    }

    return success;
}

/**
**************************************************************************************************
*   MdoResourceCache::GetMdoResource
*
*   @brief
*       Retrieve an existing MdoResource from the cache
*   @param resHandle Input resource handle
*   @param pDevice Input device
*   @param newMdoResource Input flag for a new resource
*   @return
*       An MdoResource ptr
**************************************************************************************************
*/
MdoResource* MdoResourceCache::GetMdoResource(UINT64 resHandle, void* pDevice, bool newMdoResource)
{
    MdoResource* pOut = nullptr;

    const bool resHandleExists = ResHandleExists(resHandle);

    if (resHandleExists == true)
    {
        pOut = m_resourceList[resHandle];
    }
    else if ((resHandleExists == false) && (newMdoResource == true))
    {
        MDO_ASSERT(pDevice != nullptr);

        pOut = RegisterResource(pDevice, resHandle);
    }

    return pOut;
}

/**
**************************************************************************************************
*   MdoResourceCache::TrackCreate
*
*   @brief
*       Track creation of a new resource for the cache.
*       Intended for next-gen APIs, where we will need to track allocations as they happen.
*   @param resHandle Input resource handle
**************************************************************************************************
*/
void MdoResourceCache::TrackCreate(UINT64 resHandle)
{
    MdoResource* pResource = GetMdoResource(resHandle, nullptr, false);

    // Resource already present, so our pMdoResource must now use new data
    if (pResource != nullptr)
    {
        pResource->DeleteReflectionData();
        pResource->NewReflectionData();
    }
}

/**
**************************************************************************************************
*   MdoResourceCache::TrackDestroy
*
*   @brief
*       Track destruction of a current resource in the cache.
*       Intended for next-gen APIs, where we will need to track allocations as they get destroyed.
*   @param resHandle Input resource handle
**************************************************************************************************
*/
void MdoResourceCache::TrackDestroy(UINT64 resHandle)
{
    MdoResource* pResource = GetMdoResource(resHandle, nullptr, false);

    // Resource was delete it, so give back our MdoResource mem
    if (pResource != nullptr)
    {
        pResource->DeleteReflectionData();
    }
}

/**
**************************************************************************************************
*   MdoResourceCache::ResetResources
*
*   @brief
*       Reset all resources in the cache back to start state
**************************************************************************************************
*/
void MdoResourceCache::ResetResources()
{
    for (ResourceMap::iterator it = m_resourceList.begin(); it != m_resourceList.end(); it++)
    {
        MdoResource* pCurrResource = it->second;

        if (pCurrResource != nullptr)
        {
            pCurrResource->ResetMapEvents();
        }
    }
}

/**
**************************************************************************************************
*   MdoResourceCache::FindResource
*
*   @brief
*       Locate a resource in the cache, given a memory location
*   @param exceptionAddr exception address
*   @return
*       An MdoResource ptr
**************************************************************************************************
*/
MdoResource* MdoResourceCache::FindResource(UINT64 exceptionAddr)
{
    MdoResource* pOut = nullptr;

    for (ResourceMap::iterator it = m_resourceList.begin(); it != m_resourceList.end(); it++)
    {
        MdoResource* pCurrResource = it->second;

        const UINT64 start = (UINT64)pCurrResource->GetReflectionData()->pNewData;
        const UINT64 end = start + pCurrResource->ResourceByteSize();

        if ((exceptionAddr >= start) && (exceptionAddr < end))
        {
            pOut = pCurrResource;
            break;
        }
    }

    return pOut;
}
