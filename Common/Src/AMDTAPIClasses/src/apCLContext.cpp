//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLContext.cpp
///
//==================================================================================

//------------------------------ apCLContext.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apCLContext.h>


// ---------------------------------------------------------------------------
// Name:        apCLContext::apCLContext
// Description: Constructor.
// Arguments: APIID - The API's ID for this compute context.
// Author:  AMD Developer Tools Team
// Date:        17/6/2008
// ---------------------------------------------------------------------------
apCLContext::apCLContext(oaCLContextHandle contextHandle, gtInt32 APIID)
    : _contextHandle(contextHandle), _wasMarkedForDeletion(false), _APIID(APIID), _referenceCount(0), _contextCreationProperties(NULL), _contextName(L""), _openGLSpyID(-1)
{
}


// ---------------------------------------------------------------------------
// Name:        apCLContext::apCLContext
// Description: Copy constructor
// Arguments: other - The other context information class from which I am created.
// Author:  AMD Developer Tools Team
// Date:        19/1/2010
// ---------------------------------------------------------------------------
apCLContext::apCLContext(const apCLContext& other)
{
    this->operator=(other);
}


// ---------------------------------------------------------------------------
// Name:        apCLContext::~apCLContext
// Description: Destructor.
// Author:  AMD Developer Tools Team
// Date:        18/1/2010
// ---------------------------------------------------------------------------
apCLContext::~apCLContext()
{
}


// ---------------------------------------------------------------------------
// Name:        apCLContext::operator=
// Description: Copies information from another class instance into me.
// Author:  AMD Developer Tools Team
// Date:        19/1/2010
// ---------------------------------------------------------------------------
apCLContext& apCLContext::operator=(const apCLContext& other)
{
    // Copy handle, deletion status, API ID and reference count:
    _contextHandle = other._contextHandle;
    _wasMarkedForDeletion = other._wasMarkedForDeletion;
    _APIID = other._APIID;
    _referenceCount = other._referenceCount;

    _contextCreationProperties = other._contextCreationProperties;

    // Copy the context associated device IDs:
    _deviceIDs.clear();
    gtSize_t devicesAmount = other._deviceIDs.size();

    for (gtSize_t j = 0; j < devicesAmount; j++)
    {
        gtInt32 currDeviceID = other._deviceIDs[j];
        _deviceIDs.push_back(currDeviceID);
    }

    m_platform = other.m_platform;
    m_isAMDPlatform = other.m_isAMDPlatform;
    _contextName = other._contextName;
    _openGLSpyID = other._openGLSpyID;

    setAllocatedObjectId(other.getAllocatedObjectId(), true);
    return *this;
}


// ---------------------------------------------------------------------------
// Name:        apCLContext::setContextCreationProperties
// Description: Logs the OpenCL's context creation properties.
// Arguments: pPropertiesList - The context's creation properties
// Author:  AMD Developer Tools Team
// Date:        18/1/2010
// ---------------------------------------------------------------------------
void apCLContext::setContextCreationProperties(cl_context_properties* pPropertiesList)
{
    // Initialize the context properties with the data from the CL API:
    _contextCreationProperties.initFromCLContextProperties(pPropertiesList);
}


// ---------------------------------------------------------------------------
// Name:        apCLContext::readSelfFromChannel
// Description: Writes this class data into ipcChannel.
// Author:  AMD Developer Tools Team
// Date:        17/6/2008
// ---------------------------------------------------------------------------
bool apCLContext::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the context's handle, deletion status, spy id and reference count:
    ipcChannel << (gtUInt64)_contextHandle;
    ipcChannel << _wasMarkedForDeletion;
    ipcChannel << _APIID;
    ipcChannel << _referenceCount;

    // Write the context properties list:
    _contextCreationProperties.writeSelfIntoChannel(ipcChannel);

    // Write the context associated device IDs:
    gtSize_t devicesAmount = _deviceIDs.size();
    ipcChannel << (gtUInt64)devicesAmount;

    for (gtSize_t j = 0; j < devicesAmount; j++)
    {
        ipcChannel << _deviceIDs[j];
    }

    // Write the context platform:
    ipcChannel << (gtUInt64)m_platform;
    ipcChannel << m_isAMDPlatform;

    // Write the context Name:
    ipcChannel << _contextName;

    // Write the GL spy ID:
    ipcChannel << (gtInt32)_openGLSpyID;

    // Write base class data:
    bool retVal = apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLContext::readSelfFromChannel
// Description: Reads this class data from ipcChannel.
// Author:  AMD Developer Tools Team
// Date:        17/6/2008
// ---------------------------------------------------------------------------
bool apCLContext::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the context's handle, deletion status, spy id and reference count:
    gtUInt64 contextHandleAsUInt64 = 0;
    ipcChannel >> contextHandleAsUInt64;
    _contextHandle = (oaCLContextHandle)contextHandleAsUInt64;
    ipcChannel >> _wasMarkedForDeletion;
    ipcChannel >> _APIID;
    ipcChannel >> _referenceCount;

    // Read the context properties list:
    _contextCreationProperties.readSelfFromChannel(ipcChannel);

    // Read the context associated device IDs:
    _deviceIDs.clear();
    gtUInt64 devicesAmount = 0;
    ipcChannel >> devicesAmount;

    for (gtUInt64 j = 0; j < devicesAmount; j++)
    {
        gtInt32 currDeviceID = 0;
        ipcChannel >> currDeviceID;
        _deviceIDs.push_back(currDeviceID);
    }

    // Read the context platform:
    gtUInt64 platformAsUInt64 = 0;
    ipcChannel >> platformAsUInt64;
    m_platform = (oaCLPlatformID)platformAsUInt64;
    ipcChannel >> m_isAMDPlatform;

    // Read the context name:
    ipcChannel >> _contextName;

    // Write the GL spy ID:
    gtInt32 int32Var = 0;
    ipcChannel >> int32Var;
    _openGLSpyID = (int)int32Var;

    // Read base class data:
    bool retVal = apAllocatedObject::readSelfFromChannel(ipcChannel);

    return retVal;
}
