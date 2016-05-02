//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktObjectDatabaseProcessor.cpp
/// \brief  Implementation file for a Vulkan object database processor.
///         This class maintains handles to object content and queries their
///         instances to retrieve information.
//==============================================================================

#include "vktObjectDatabaseProcessor.h"

#include "../vktLayerManager.h"

//-----------------------------------------------------------------------------
/// Retrieve a pointer to the parent LayerManager used by this tool.
/// \returns A pointer to the parent LayerManager used by this tool.
//-----------------------------------------------------------------------------
ModernAPILayerManager* VktObjectDatabaseProcessor::GetParentLayerManager()
{
    return VktLayerManager::GetLayerManager();
}

//-----------------------------------------------------------------------------
/// Retrieve the object type enumeration value from a type string.
/// \param inObjectTypeString A string containing the type of object to get the value for.
/// \returns The enumeration value for the incoming object type string.
//-----------------------------------------------------------------------------
int VktObjectDatabaseProcessor::GetObjectTypeFromString(const gtASCIIString& inObjectTypeString) const
{
    UNREFERENCED_PARAMETER(inObjectTypeString);

    return kObjectType_Undefined;
}