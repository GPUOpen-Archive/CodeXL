//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLSampler.h
///
//==================================================================================

//------------------------------ apCLSampler.h ------------------------------

#ifndef __APCLSAMPLER_H
#define __APCLSAMPLER_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apAllocatedObject.h>

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLSampler : public apAllocatedObject
// General Description: Represent an OpenCL sampler.
// Author:  AMD Developer Tools Team
// Creation Date:       21/1/2010
// ----------------------------------------------------------------------------------
class AP_API apCLSampler : public apAllocatedObject
{
public:
    apCLSampler();
    apCLSampler(gtInt32 samplerId, oaCLSamplerHandle samplerHandle, bool areCoordsNormalized, cl_addressing_mode addressingMode, cl_filter_mode filterMode);
    virtual ~apCLSampler();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    gtInt32 samplerId() const {return _samplerId;}
    oaCLSamplerHandle samplerHandle() const {return _samplerHandle;};
    bool wasMarkedForDeletion() const {return _wasMarkedForDeletion;};
    void onSamplerMarkedForDeletion() {_wasMarkedForDeletion = true;};
    bool areCoordsNormalized() const {return _areCoordsNormalized;};
    cl_addressing_mode addressingMode() const {return _addressingMode;};
    cl_filter_mode filterMode() const {return _filterMode;};
    unsigned int referenceCount() const {return _referenceCount;};
    void setReferenceCount(unsigned int refCount) {_referenceCount = refCount;};

    // cl_gremedy_object_naming:
    const gtString& samplerName() const {return _samplerName;};
    void setSamplerName(const gtString& name) {_samplerName = name;};

    // Static functions used for filter & addressing mode translation to string:
    static gtString filterModeAsString(cl_filter_mode mode);
    static gtString addressingModeAsString(cl_addressing_mode mode);

private:

    // Internal id:
    gtInt32 _samplerId;

    // Sampler handle:
    oaCLSamplerHandle _samplerHandle;

    // Marked for deletion:
    bool _wasMarkedForDeletion;

    // Are coords normalized?
    bool _areCoordsNormalized;

    // Addressing mode:
    cl_addressing_mode _addressingMode;

    // Filter mode:
    cl_filter_mode _filterMode;

    // Reference count:
    unsigned int _referenceCount;

    // cl_gremedy_object_naming:
    gtString _samplerName;
};

#endif //__APCLSAMPLER_H

