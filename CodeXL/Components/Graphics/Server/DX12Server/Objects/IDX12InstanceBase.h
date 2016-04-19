//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   IDX12InstanceBase.h
/// \brief  An info class that holds information related to a wrapped DX12 interface.
//=============================================================================

#ifndef IDX12INSTANCEBASE_H
#define IDX12INSTANCEBASE_H

#include "../Common/IInstanceBase.h"
#include "DX12CreateInfoStructs.h"
#include "../Common/xml.h"
#include "../DX12Defines.h"

class Wrapped_ID3D12Device;

//-----------------------------------------------------------------------------
/// An info class that holds information related to a wrapped DX12 interface.
//-----------------------------------------------------------------------------
class IDX12InstanceBase : public IInstanceBase
{
public:
    //-----------------------------------------------------------------------------
    /// Constructor invoked when wrapping a new interface instance.
    /// \param inRuntimeInstance The real interface pointer that the runtime gives us.
    /// \param inWrapperInterface The Wrapper that we create to surround and hook the runtime interface.
    /// \param inObjectType The type of object being wrapped.
    /// \param inCreateInfo The CreateInfo instance for the newly-created interface.
    //-----------------------------------------------------------------------------
    IDX12InstanceBase(IUnknown* inRuntimeInstance, IUnknown* inWrapperInterface, eObjectType inObjectType, Wrapped_DX12CreateInfoBase* inCreateInfo);

    //-----------------------------------------------------------------------------
    /// Destructor invoked when Wrapped interface instances are destroyed.
    //-----------------------------------------------------------------------------
    virtual ~IDX12InstanceBase();

    //-----------------------------------------------------------------------------
    /// Return the type of object referenced by this metadata object.
    /// \returns The type of object that this object is wrapping.
    //-----------------------------------------------------------------------------
    virtual eObjectType GetObjectType() const { return mObjectType; }

    //-----------------------------------------------------------------------------
    /// Stringify the type of object being wrapped for display in the client.
    /// \returns A string containing the interface type.
    //-----------------------------------------------------------------------------
    virtual const char* GetTypeAsString() const;

    //-----------------------------------------------------------------------------
    /// Retrieve a pointer to the parent device that created this instance.
    /// \returns The Wrapped_ID3D12Device instance that created the wrapped DeviceChild.
    //-----------------------------------------------------------------------------
    virtual void* GetParentDeviceHandle() const { return mParentDevice; }

    //-----------------------------------------------------------------------------
    /// Set the parent device for this wrapper object.
    /// \param inParentDevice A Wrapped_ID3D12Device wrapper instance that created this object.
    //-----------------------------------------------------------------------------
    void SetParentDeviceHandle(Wrapped_ID3D12Device* inParentDevice) { mParentDevice = inParentDevice; }

    //-----------------------------------------------------------------------------
    /// Retrieve a pointer to the handle that the application is using.
    /// \returns A void* to the real API object instance. Always a Wrapped_ID3D12[Something] instance.
    //-----------------------------------------------------------------------------
    virtual void* GetApplicationHandle() const { return mWrapperInstance; }

    //-----------------------------------------------------------------------------
    /// Print a hex-formatted application handle into the input string.
    /// \param outHandleString The string that the application handle will be printed into.
    //-----------------------------------------------------------------------------
    virtual void PrintFormattedApplicationHandle(gtASCIIString& outHandleString) const;

    //-----------------------------------------------------------------------------
    /// Write the create data for the wrapped object as XML.
    /// \param outCreateInfoXML The CreateInfo formatted in XML.
    //-----------------------------------------------------------------------------
    virtual void AppendCreateInfoXML(gtASCIIString& outCreateInfoXML) const;

    //-----------------------------------------------------------------------------
    /// Format the object tag data into the output parameter.
    /// \param outTagDataString A string containing the tag data for this instance.
    /// \returns True if the object contains tag data, false if it doesn't.
    //-----------------------------------------------------------------------------
    virtual bool AppendTagDataXML(gtASCIIString& outTagDataString) const { outTagDataString.appendFormattedString("test"); return true; }

    //-----------------------------------------------------------------------------
    /// Retrieve the CreateInfo structure for this instance.
    /// \returns A polymorphic Wrapped_DX12CreateInfoBase instance with the CreateInfo for this instance.
    //-----------------------------------------------------------------------------
    Wrapped_DX12CreateInfoBase* GetCreateInfoStruct() const { return mCreateInfo; }

    //-----------------------------------------------------------------------------
    /// Return a pointer to the wrapper instance.
    /// \returns A pointer to the wrapper instance. This is the same pointer that the application uses
    //-----------------------------------------------------------------------------
    IUnknown* GetRuntimeInstance() const { return mRealInstance; }

    /// The "Real" runtime instance of the interface.
    IUnknown* mRealInstance;

    /// The wrapper instance (Wrapped_ID3D...) used to hook interface calls.
    IUnknown* mWrapperInstance;

    /// The type of object referenced by this metadata instance.
    eObjectType mObjectType;

    /// A pointer to the cached CreateInfo for this instance.
    Wrapped_DX12CreateInfoBase* mCreateInfo;

    /// A pointer to the parent device that created this instance.
    Wrapped_ID3D12Device* mParentDevice;
};

#endif // IDX12INSTANCEBASE_H
