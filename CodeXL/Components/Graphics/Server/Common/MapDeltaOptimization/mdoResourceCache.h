//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Resource cache header file for MDO.
///         This class tracks MdoResource objects throughout their lifetime.
//==============================================================================

#ifndef __MDO_RESOURCE_CACHE_H__
#define __MDO_RESOURCE_CACHE_H__

#include "mdoUtil.h"

/// Class to track the creation and destruction of resources
class MdoResourceCache
{
public:

static MdoResourceCache* Instance();

~MdoResourceCache() {}

MdoResource* GetMdoResource(UINT64 resHandle, void* pDevice, bool newMdoResource);
void ResetResources();

void TrackCreate(UINT64 resHandle);
void TrackDestroy(UINT64 resHandle);

MdoResource* FindResource(UINT64 exceptionAddr);

protected:
bool Init(const MdoConfig& mdoConfig);
MdoResourceCache() {}

/// Register a resource
/// \param pDevice Device associated with the resource
/// \param resHandle Handle to the resource
/// \return null or pointer to resource
virtual MdoResource* RegisterResource(void* pDevice, UINT64 resHandle)
{
    UNREFERENCED_PARAMETER(pDevice);
    UNREFERENCED_PARAMETER(resHandle);
    return nullptr;
}

bool UnregisterResource(UINT64 resHandle);
bool ResHandleExists(UINT64 resHandle);

MdoConfig   m_mdoConfig; ///< Mdo config
ResourceMap m_resourceList; ///< List of resources
};

#endif