//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csSamplersMonitor.h
///
//==================================================================================

//------------------------------ csSamplersMonitor.h ------------------------------

#ifndef __CSSAMPLERSMONITOR_H
#define __CSSAMPLERSMONITOR_H

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTAPIClasses/Include/apCLSampler.h>

// ----------------------------------------------------------------------------------
// Class Name:           csSamplersMonitor
// General Description: Monitors a context's OpenCL sampler objects
// Author:               Uri Shomroni
// Creation Date:        24/1/2010
// ----------------------------------------------------------------------------------
class csSamplersMonitor
{
public:
    csSamplersMonitor(int openCLContextId);
    ~csSamplersMonitor();

    // Events:
    void onSamplerCreation(cl_sampler samplerHandle, cl_bool normalizedCoords, cl_addressing_mode addressingMode, cl_filter_mode filterMode);
    void onSamplerCreationWithProperties(cl_sampler samplerHandle, const cl_sampler_properties* properties);
    void onSamplerMarkedForDeletion(cl_sampler samplerHandle);

    // Reference count checking:
    void checkForReleasedSamplers();

    // Sampler object accessors:
    int amountOfSamplers() const {return (int)_samplers.size();};
    const apCLSampler* getSamplerDetails(int samplerIndex) const;
    apCLSampler* getSamplerDetails(int samplerIndex);
    const apCLSampler* getSamplerDetails(oaCLSamplerHandle samplerHandle) const;
    apCLSampler* getSamplerDetails(oaCLSamplerHandle samplerHandle);

private:
    // Disallow use of my default constructor:
    csSamplersMonitor();

private:
    // My context's Id:
    int _openCLContextId;

    // The sampler objects:
    gtPtrVector<apCLSampler*> _samplers;

    // The first index free for sampler id:
    int _nextFreeSamplerId;

};

#endif //__CSSAMPLERSMONITOR_H

