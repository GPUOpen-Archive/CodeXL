//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12ObjectDatabaseProcessor.h
/// \brief  The DX12ObjectDatabaseProcessor is responsible for extending the
///         ObjectDatabaseProcessor to work with DX12.
//=============================================================================

#ifndef DX12OBJECTDATABASEPROCESSOR_H
#define DX12OBJECTDATABASEPROCESSOR_H

#include "../../Common/ObjectDatabaseProcessor.h"
#include "../../Common/TSingleton.h"
#include "DX12WrappedObjectDatabase.h"
#include "IDX12InstanceBase.h"
#include "DX12Defines.h"
#include "Objects/CustomWrappers/Wrapped_ID3D12CommandQueueCustom.h"

class Wrapped_DX12CreateInfoBase;
class Wrapped_ID3D12Device;

//-----------------------------------------------------------------------------
/// Generic object wrapper template. Common to all D3D12 objects being wrapped.
/// \param inParentDevice The parent device that was responsible for creating the wrapped instance.
/// \param ppReal A double pointer to the real instance of the D3D12 object.
/// \param inObjectType The type of object being wrapped.
/// \param inCreateInfo The CreateInfo structure for the newly-wrapped instance.
/// \returns True if the object has just been wrapped. False if it was already wrapped.
//-----------------------------------------------------------------------------
template <class T, class U>
bool GenericWrapObject(Wrapped_ID3D12Device* inParentDevice, T** ppReal, eObjectType inObjectType, Wrapped_DX12CreateInfoBase* inCreateInfo)
{
    DX12ObjectDatabaseProcessor* dx12ObjectDBProcessor = DX12ObjectDatabaseProcessor::Instance();
    DX12WrappedObjectDatabase* objectDatabase = static_cast<DX12WrappedObjectDatabase*>(dx12ObjectDBProcessor->GetObjectDatabase());

    // Is object already in database?
    if (objectDatabase->WrappedObject((IUnknown**)ppReal))
    {
        // already wrapped - ppReal is updated to wrapped object
        return false;
    }

    //wrap it
    U* pWrapper = new U(*ppReal);

    // Create a new "Wrapper metadata object" to store all information about the wrapper.
    IDX12InstanceBase* objectData = new IDX12InstanceBase(static_cast<IUnknown*>(*ppReal), static_cast<IUnknown*>(pWrapper), inObjectType, inCreateInfo);
    objectDatabase->Add(objectData);
    objectData->SetParentDeviceHandle(inParentDevice);

    // Reassign the outgoing pointer to our wrapped instance instead of the runtime instance. This allows us to hook all interface calls.
    *ppReal = pWrapper;
    return true;
}

//-----------------------------------------------------------------------------
/// The DX12ObjectDatabaseProcessor is responsible for extending the ObjectDatabaseProcessor to work with DX12.
//-----------------------------------------------------------------------------
class DX12ObjectDatabaseProcessor : public ObjectDatabaseProcessor, public TSingleton < DX12ObjectDatabaseProcessor >
{
    //-----------------------------------------------------------------------------
    /// TSingleton is a friend of the DX12ObjectDatabaseProcessor.
    //-----------------------------------------------------------------------------
    friend class TSingleton < DX12ObjectDatabaseProcessor >;
public:
    //-----------------------------------------------------------------------------
    /// Default constructor for the DX12ObjectDatabaseProcessor.
    //-----------------------------------------------------------------------------
    DX12ObjectDatabaseProcessor();

    //-----------------------------------------------------------------------------
    /// Default destructor for the DX12ObjectDatabaseProcessor class.
    //-----------------------------------------------------------------------------
    virtual ~DX12ObjectDatabaseProcessor() {}

    //-----------------------------------------------------------------------------
    /// Retrieve the DX12-specific object database instance.
    //-----------------------------------------------------------------------------
    virtual WrappedObjectDatabase* GetObjectDatabase() { return &mObjectDatabase; }

    //-----------------------------------------------------------------------------
    /// Retrieve a pointer to the parent LayerManager used by this tool.
    /// \returns A pointer to the parent LayerManager used by this tool.
    //-----------------------------------------------------------------------------
    virtual ModernAPILayerManager* GetParentLayerManager();

protected:
    //-----------------------------------------------------------------------------
    /// Retrieve the object type enumeration value from a type string.
    /// \param inObjectTypeString A string containing the type of object to get the value for.
    /// \returns The enumeration value for the incoming object type string.
    //-----------------------------------------------------------------------------
    virtual int GetObjectTypeFromString(const gtASCIIString& inObjectTypeString) const;

    //-----------------------------------------------------------------------------
    /// Return a number indicating the ordinal of the first ID3D12 interface wrapper type.
    /// \returns The eObjectType ordinal corresponding with the first wrapped object type.
    //-----------------------------------------------------------------------------
    virtual int GetFirstObjectType() const { return kObjectType_Begin_Range; }

    //-----------------------------------------------------------------------------
    /// Return a number indicating the ordinal of the last ID3D12 interface wrapper type.
    /// \returns The eObjectType ordinal corresponding with the last wrapped object type.
    //-----------------------------------------------------------------------------
    virtual int GetLastObjectType() const { return kObjectType_End_Range; }

    //-----------------------------------------------------------------------------
    /// Return a number indicating the ordinal of the ID3D12Device wrapper type.
    /// \returns The eObjectType ordinal corresponding with the Device object type.
    //-----------------------------------------------------------------------------
    virtual int GetDeviceType() const { return kObjectType_ID3D12Device; }

private:
    /// The database where wrapped DX12 interface instances are stored.
    DX12WrappedObjectDatabase mObjectDatabase;
};

#endif // DX12OBJECTDATABASEPROCESSOR_H