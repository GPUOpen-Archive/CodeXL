//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktWrappedObject.cpp
/// \brief  Super class for object wrappers.
//=============================================================================

#ifndef __I_WRAPPED_OBJECT_H__
#define __I_WRAPPED_OBJECT_H__

#include "../../Util/vktUtil.h"
#include "../../Interception/vktIntercept.h"

//-----------------------------------------------------------------------------
/// This class will be responsible for setting up wrappers as dispatchable objects.
//-----------------------------------------------------------------------------
class VktWrappedObject
{
public:
    virtual ~VktWrappedObject() {}

    /// Save off the app's handle in this wrapper
    virtual void StoreAppHandle(UINT64 hAppObject) = 0;

    //virtual UINT64 GetAppHandle() = 0;
    //virtual UINT64 GetIcdHandle() = 0;
    //virtual void FreeAllocatedHeapMemory() = 0;
    //virtual VkDevice GetParentDevice() = 0;
    //virtual VkResult DestroyObject(UINT64 hObject) = 0;
};

//-----------------------------------------------------------------------------
/// Holds the magic number, dispatch table, and pointer to wrapped object.
//-----------------------------------------------------------------------------
struct IcdApiObject
{
    VK_LOADER_DATA   loaderData;      ///< Should hold magic value
    VktWrappedObject* pWrappedObject;  ///< Pointer to wrapper
};

//-----------------------------------------------------------------------------
/// Helper functions.
//-----------------------------------------------------------------------------
UINT64 CreateAppHandle(VktWrappedObject* pWrappedObject);
void DestroyWrappedHandle(UINT64 handle);
VktWrappedObject* WrappedObjectFromAppHandle(UINT64 handle);
const VkLayerDispatchTable* DispatchTableFromHandle(UINT64 handle);

#endif // __I_WRAPPED_OBJECT_H__
