//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktWrappedObjectDatabase.cpp
/// \brief  Implementation file for a Vulkan object database.
///         This class holds C++ wrappers for live Vulkan objects.
//==============================================================================

#include "../../../Common/IInstanceBase.h"

#include "vktWrappedObjectDatabase.h"
#include "vktInstanceBase.h"

//-----------------------------------------------------------------------------
/// Retrieve a vector of objects with a given type.
/// \param inObjectType The type of wrapped object to retrieve a vector of.
/// \param outObjectInstancesOfGivenType The vector of all objects found to have the given type.
/// \param inbOnlyCurrentObjects A flag to specify if all instances should be returned, or only active instances.
//-----------------------------------------------------------------------------
void VktWrappedObjectDatabase::GetObjectsByType(eObjectType inObjectType, WrappedInstanceVector& outObjectInstancesOfGivenType, bool inbOnlyCurrentObjects) const
{
    GT_UNREFERENCED_PARAMETER(inObjectType);
    GT_UNREFERENCED_PARAMETER(outObjectInstancesOfGivenType);
    GT_UNREFERENCED_PARAMETER(inbOnlyCurrentObjects);
}

//-----------------------------------------------------------------------------
/// Retrieve a pointer to the wrapper instance of the given object.
/// \param pInstanceHandle A handle to the interface instance.
/// \returns A IInstanceBase pointer, which is the wrapper for the given input instance.
//-----------------------------------------------------------------------------
IInstanceBase* VktWrappedObjectDatabase::GetWrappedInstance(void* pInstanceHandle) const
{
    GT_UNREFERENCED_PARAMETER(pInstanceHandle);
    return nullptr;
}

//-----------------------------------------------------------------------------
/// Add a wrapper metadata object to the object database.
/// \param pWrappedInstance A pointer to the new object.
//-----------------------------------------------------------------------------
void VktWrappedObjectDatabase::StoreWrappedInstance(IInstanceBase* pWrappedInstance)
{
    GT_UNREFERENCED_PARAMETER(pWrappedInstance);
}

//-----------------------------------------------------------------------------
/// A handler that's invoked when a device is destroyed. Responsible for cleaning up wrappers.
/// \param pDeviceInstance A wrapper instance for the device being destroyed.
//-----------------------------------------------------------------------------
void VktWrappedObjectDatabase::OnDeviceDestroyed(IInstanceBase* pDeviceInstance)
{
    GT_UNREFERENCED_PARAMETER(pDeviceInstance);
}