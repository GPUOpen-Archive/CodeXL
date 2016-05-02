//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktInstancebase.h
/// \brief  Header file for a Vulkan instance base.
///         This class represents the parent for all wrapped Vulkan objects.
//==============================================================================

#ifndef __VKT_INSTANCE_BASE_H__
#define __VKT_INSTANCE_BASE_H__

#include "../Util/vktUtil.h"

#include "../../../Common/IInstanceBase.h"
#include "../../../Common/xml.h"

//-----------------------------------------------------------------------------
/// Defines the base class for all wrapped instances of Vulkan objects. This
/// class exists for convenience as a container and will be responsible for
/// storing the creation info for objects created through grCreate* calls.
//-----------------------------------------------------------------------------
class VktInstanceBase : public IInstanceBase
{
public:
    VktInstanceBase();
    virtual ~VktInstanceBase() {}

    void AppendObjectXML(gtASCIIString& outObjectXML) const;

    virtual void PrintFormattedApplicationHandle(gtASCIIString& outHandleString) const;
    virtual bool AppendTagDataXML(gtASCIIString& outTagDataString) const;
    virtual const char* GetTypeAsString() const;

    /// Retrieve the parent device handle
    virtual void* GetParentDeviceHandle() const { return static_cast<VkDevice>(m_parentDevice); }

    /// Retrieve the app's handle for this object
    virtual void* GetApplicationHandle() const { return nullptr; }

private:

    /// The object's parent device
    VkDevice m_parentDevice;
};

#endif // __VKT_INSTANCE_BASE_H__