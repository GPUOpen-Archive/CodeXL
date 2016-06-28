//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktInstancebase.cpp
/// \brief  Implementation file for a Vulkan instance base.
///         This class represents the parent for all wrapped Vulkan objects.
//==============================================================================

#include "vktInstanceBase.h"
#include "vktObjectDatabaseProcessor.h"

//-----------------------------------------------------------------------------
/// Constructor invoked when wrapping a new interface instance.
//-----------------------------------------------------------------------------
VktInstanceBase::VktInstanceBase()
    : m_parentDevice(VK_NULL_HANDLE)
{
}

//-----------------------------------------------------------------------------
/// Write the object data for the wrapped object as XML.
/// \param outObjectXML The CreateInfo formatted in XML.
//-----------------------------------------------------------------------------
void VktInstanceBase::AppendObjectXML(gtASCIIString& outObjectXML) const
{
    GT_UNREFERENCED_PARAMETER(outObjectXML);
}

//-----------------------------------------------------------------------------
/// Write the tag data for the wrapped object as XML.
/// \param outTagDataString The CreateInfo formatted in XML.
//-----------------------------------------------------------------------------
bool VktInstanceBase::AppendTagDataXML(gtASCIIString& outTagDataString) const
{
    GT_UNREFERENCED_PARAMETER(outTagDataString);

    return false;
}

//-----------------------------------------------------------------------------
/// Print the formatted application handle (the wrapper pointer) to the incoming string.
/// \param outHandleString A string containing the printed application handle.
//-----------------------------------------------------------------------------
void VktInstanceBase::PrintFormattedApplicationHandle(gtASCIIString& outHandleString) const
{
    GT_UNREFERENCED_PARAMETER(outHandleString);
}

//-----------------------------------------------------------------------------
/// Stringify the type of interface being wrapped for display in the client.
/// \returns A string containing the interface type.
//-----------------------------------------------------------------------------
const char* VktInstanceBase::GetTypeAsString() const
{
    return "";
}
