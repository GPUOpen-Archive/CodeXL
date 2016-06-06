//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLSampler.cpp
///
//==================================================================================

//------------------------------ apCLSampler.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTAPIClasses/Include/apStringConstants.h>
#include <AMDTAPIClasses/Include/apCLSampler.h>


// ---------------------------------------------------------------------------
// Name:        apCLSampler::apCLSampler
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        21/1/2010
// ---------------------------------------------------------------------------
apCLSampler::apCLSampler()
    : _samplerId(-1), _samplerHandle(OA_CL_NULL_HANDLE), _wasMarkedForDeletion(false), _areCoordsNormalized(false), _addressingMode(0),
      _filterMode(0), _referenceCount(1)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLSampler::apCLSampler
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        21/1/2010
// ---------------------------------------------------------------------------
apCLSampler::apCLSampler(gtInt32 samplerId, oaCLSamplerHandle samplerHandle, bool areCoordsNormalized, cl_addressing_mode addressingMode, cl_filter_mode filterMode)
    : _samplerId(samplerId), _samplerHandle(samplerHandle), _wasMarkedForDeletion(false),  _areCoordsNormalized(areCoordsNormalized), _addressingMode(addressingMode),
      _filterMode(filterMode), _referenceCount(1)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLSampler::~apCLSampler
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        21/1/2010
// ---------------------------------------------------------------------------
apCLSampler::~apCLSampler()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLSampler::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        21/1/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apCLSampler::type() const
{
    return OS_TOBJ_ID_CL_SAMPLER;
}

// ---------------------------------------------------------------------------
// Name:        apCLSampler::writeSelfIntoChannel
// Description: Writes this class into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        21/1/2010
// ---------------------------------------------------------------------------
bool apCLSampler::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    ipcChannel << _samplerId;
    ipcChannel << (gtUInt64)_samplerHandle;
    ipcChannel << _wasMarkedForDeletion;
    ipcChannel << _areCoordsNormalized;
    ipcChannel << (gtUInt32)_addressingMode;
    ipcChannel << (gtUInt32)_filterMode;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLSampler::readSelfFromChannel
// Description: Reads this class from an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        21/1/2010
// ---------------------------------------------------------------------------
bool apCLSampler::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apAllocatedObject::readSelfFromChannel(ipcChannel);

    ipcChannel >> _samplerId;

    gtUInt64 samplerHandleAsUInt64 = (gtUInt64)OA_CL_NULL_HANDLE;
    ipcChannel >> samplerHandleAsUInt64;
    _samplerHandle = (oaCLSamplerHandle)samplerHandleAsUInt64;

    ipcChannel >> _wasMarkedForDeletion;

    ipcChannel >> _areCoordsNormalized;

    gtUInt32 addressingModeAsUInt32 = (gtUInt32)CL_NONE;
    ipcChannel >> addressingModeAsUInt32;
    _addressingMode = (cl_addressing_mode)addressingModeAsUInt32;

    gtUInt32 filterModeAsUInt32 = (gtUInt32)CL_NONE;
    ipcChannel >> filterModeAsUInt32;
    _filterMode = (cl_filter_mode)filterModeAsUInt32;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLSampler::filterModeAsString
// Description: Translate an OpenCL filter mode to string
// Arguments:   cl_filter_mode mode
// Return Val:  gtString
// Author:  AMD Developer Tools Team
// Date:        4/5/2010
// ---------------------------------------------------------------------------
gtString apCLSampler::filterModeAsString(cl_filter_mode mode)
{
    gtString retVal;

    if (mode == CL_FILTER_NEAREST)
    {
        retVal = AP_STR_SamplerFilterModeNearest;
    }
    else if (mode == CL_FILTER_LINEAR)
    {
        retVal = AP_STR_SamplerFilterModeLinear;
    }
    else
    {
        retVal = AP_STR_SamplerFilterModeNone;
        GT_ASSERT_EX(false, L"Unknown sampler filter mode");
    }


    return retVal;

}

// ---------------------------------------------------------------------------
// Name:        apCLSampler::addressingModeAsString
// Description: Translate an OpenCL addressing mode to a string
// Arguments:   cl_addressing_mode mode
// Return Val:  gtString
// Author:  AMD Developer Tools Team
// Date:        4/5/2010
// ---------------------------------------------------------------------------
gtString apCLSampler::addressingModeAsString(cl_addressing_mode mode)
{
    gtString retVal;

    if (mode == CL_ADDRESS_NONE)
    {
        retVal = AP_STR_SamplerAddressingModeNone;
    }
    else if (mode == CL_ADDRESS_CLAMP_TO_EDGE)
    {
        retVal = AP_STR_SamplerAddressingModeClampToEdge;
    }
    else if (mode == CL_ADDRESS_CLAMP)
    {
        retVal = AP_STR_SamplerAddressingModeClamp;
    }
    else if (mode == CL_ADDRESS_REPEAT)
    {
        retVal = AP_STR_SamplerAddressingModeRepeat;
    }
    else if (mode == CL_ADDRESS_MIRRORED_REPEAT)
    {
        retVal = AP_STR_SamplerAddressingModeMirrorRepeat;
    }
    else
    {
        retVal = AP_STR_SamplerAddressingModeNone;
        GT_ASSERT_EX(false, L"Unknown sampler filter mode");
    }

    return retVal;
}

