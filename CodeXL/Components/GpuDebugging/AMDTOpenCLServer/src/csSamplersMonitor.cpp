//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csSamplersMonitor.cpp
///
//==================================================================================

//------------------------------ csSamplersMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTServerUtilities/Include/suAllocatedObjectsMonitor.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Local:
#include <src/csGlobalVariables.h>
#include <src/csOpenCLMonitor.h>
#include <src/csSamplersMonitor.h>


// ---------------------------------------------------------------------------
// Name:        csSamplersMonitor::csSamplersMonitor
// Description: Constructor
// Author:      Uri Shomroni
// Date:        24/1/2010
// ---------------------------------------------------------------------------
csSamplersMonitor::csSamplersMonitor(int openCLContextId)
    : _openCLContextId(openCLContextId), _nextFreeSamplerId(1)
{

}

// ---------------------------------------------------------------------------
// Name:        csSamplersMonitor::~csSamplersMonitor
// Description: Destructor
// Author:      Uri Shomroni
// Date:        24/1/2010
// ---------------------------------------------------------------------------
csSamplersMonitor::~csSamplersMonitor()
{
    _samplers.deleteElementsAndClear();
}

// ---------------------------------------------------------------------------
// Name:        csSamplersMonitor::onSamplerCreation
// Description: Called when an OpenCL sampler is created and adds it to our vectors
// Author:      Uri Shomroni
// Date:        24/1/2010
// ---------------------------------------------------------------------------
void csSamplersMonitor::onSamplerCreation(cl_sampler samplerHandle, cl_bool normalizedCoords, cl_addressing_mode addressingMode, cl_filter_mode filterMode)
{
    // Create an object to represent the new sampler:
    apCLSampler* pNewSampler = new apCLSampler(_nextFreeSamplerId++, (oaCLSamplerHandle)samplerHandle, (normalizedCoords == CL_TRUE), addressingMode, filterMode);

    // Register this sampler's creation in the allocated objects and handles monitors:
    int samplerIndex = (int)_samplers.size();
    cs_stat_openCLMonitorInstance.openCLHandleMonitor().registerOpenCLHandle((oaCLSamplerHandle)samplerHandle, _openCLContextId, samplerIndex, OS_TOBJ_ID_CL_SAMPLER, -1, _nextFreeSamplerId);
    su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pNewSampler);

    // Add it to the vector:
    _samplers.push_back(pNewSampler);
}

// ---------------------------------------------------------------------------
// Name:        csSamplersMonitor::onSamplerCreationWithProperties
// Description: Called when an OpenCL sampler is created and adds it to our vectors
// Author:      Uri Shomroni
// Date:        29/9/2014
// ---------------------------------------------------------------------------
void csSamplersMonitor::onSamplerCreationWithProperties(cl_sampler samplerHandle, const cl_sampler_properties* properties)
{
    // Start with the default values:
    cl_bool normalizedCoords = CL_TRUE;
    cl_addressing_mode addressingMode = CL_ADDRESS_CLAMP;
    cl_filter_mode filterMode = CL_FILTER_NEAREST;

    /*
    // cl_khr_mipmap_image support:
    cl_filter_mode mipFilterMode = CL_FILTER_NONE;
    float lodMin = 0.0f;
    float lodMax = GT_FLOAT32_MAX;
    */
    if (NULL != properties)
    {
        const cl_sampler_properties* pCurrentProp = properties;

        while (0 != *pCurrentProp)
        {
            cl_sampler_properties prop = *pCurrentProp++;
            cl_sampler_properties value = *pCurrentProp++;

            switch (prop)
            {
                case CL_SAMPLER_NORMALIZED_COORDS:
                    normalizedCoords = (cl_bool)value;
                    break;

                case CL_SAMPLER_ADDRESSING_MODE:
                    addressingMode = (cl_addressing_mode)value;
                    break;

                case CL_SAMPLER_FILTER_MODE:
                    filterMode = (cl_filter_mode)value;
                    break;

                /*
                // cl_khr_mipmap_image support:
                case CL_SAMPLER_MIP_FILTER_MODE:
                mipFilterMode = (cl_filter_mode)value;
                break;
                case CL_SAMPLER_LOD_MIN:
                lodMin = *(float*)&value;
                break;
                case CL_SAMPLER_LOD_MAX:
                lodMax = *(float*)&value;
                break;
                */

                default:
                    // Ignore
                    break;
            }
        }
    }

    // Create the sampler via the static function:
    onSamplerCreation(samplerHandle, normalizedCoords, addressingMode, filterMode);
}

// ---------------------------------------------------------------------------
// Name:        csSamplersMonitor::onSamplerMarkedForDeletion
// Description: Called when the sampler is marked for deletion
//              (calling clReleaseSampler with the reference count at 1)
// Author:      Uri Shomroni
// Date:        31/3/2010
// ---------------------------------------------------------------------------
void csSamplersMonitor::onSamplerMarkedForDeletion(cl_sampler samplerHandle)
{
    // Get the sampler object:
    apCLSampler* pSampler = getSamplerDetails((oaCLSamplerHandle)samplerHandle);
    GT_IF_WITH_ASSERT(pSampler != NULL)
    {
        // Mark it as deleted:
        pSampler->onSamplerMarkedForDeletion();
    }
}

// ---------------------------------------------------------------------------
// Name:        csSamplersMonitor::checkForReleasedSamplers
// Description: Checks if any of the objects monitored by this class have been released
// Author:      Uri Shomroni
// Date:        26/8/2013
// ---------------------------------------------------------------------------
void csSamplersMonitor::checkForReleasedSamplers()
{
    // Collect the object handles:
    gtVector<oaCLSamplerHandle> samplerHandles;
    int numberOfSamplers = (int)_samplers.size();

    for (int i = 0; i < numberOfSamplers; i++)
    {
        const apCLSampler* pSampler = _samplers[i];
        GT_IF_WITH_ASSERT(NULL != pSampler)
        {
            samplerHandles.push_back(pSampler->samplerHandle());
        }
    }

    // Check each one. This is done separately, since finding an object
    // that was marked for deletion will release it:
    csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
    int samplersFound = (int)samplerHandles.size();

    for (int i = 0; i < samplersFound; i++)
    {
        theOpenCLMonitor.checkIfSamplerWasDeleted((cl_sampler)samplerHandles[i], false);
    }
}

// ---------------------------------------------------------------------------
// Name:        csSamplersMonitor::getSamplerDetails
// Description: Returns a sampler object by its index in the vector
// Author:      Uri Shomroni
// Date:        24/1/2010
// ---------------------------------------------------------------------------
const apCLSampler* csSamplersMonitor::getSamplerDetails(int samplerIndex) const
{
    const apCLSampler* retVal = NULL;

    // Validate the index:
    int numberOfSamplers = (int)_samplers.size();
    GT_IF_WITH_ASSERT((samplerIndex >= 0) && (samplerIndex < numberOfSamplers))
    {
        // Return the sampler object:
        retVal = _samplers[samplerIndex];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csSamplersMonitor::getSamplerDetails
// Description: Returns a mutable sampler object by its index in the vector
// Author:      Uri Shomroni
// Date:        24/1/2010
// ---------------------------------------------------------------------------
apCLSampler* csSamplersMonitor::getSamplerDetails(int samplerIndex)
{
    apCLSampler* retVal = NULL;

    // Validate the index:
    int numberOfSamplers = (int)_samplers.size();
    GT_IF_WITH_ASSERT((samplerIndex >= 0) && (samplerIndex < numberOfSamplers))
    {
        // Return the sampler object:
        retVal = _samplers[samplerIndex];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csSamplersMonitor::getSamplerDetails
// Description: Returns a sampler object by its OpenCL handle
// Author:      Uri Shomroni
// Date:        24/1/2010
// ---------------------------------------------------------------------------
const apCLSampler* csSamplersMonitor::getSamplerDetails(oaCLSamplerHandle samplerHandle) const
{
    const apCLSampler* retVal = NULL;

    // Iterate the samplers:
    int numberOfSamplers = (int)_samplers.size();

    for (int i = 0; i < numberOfSamplers; i++)
    {
        // Get the current sampler:
        const apCLSampler* pCurrentSampler = _samplers[i];
        GT_IF_WITH_ASSERT(pCurrentSampler != NULL)
        {
            // Check the handle:
            if (pCurrentSampler->samplerHandle() == samplerHandle)
            {
                // Return the sampler object:
                retVal = pCurrentSampler;

                // If we found a deleted sampler, keep looking - there might be a living one which
                // reuses the handle:
                if (!pCurrentSampler->wasMarkedForDeletion())
                {
                    break;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csSamplersMonitor::getSamplerDetails
// Description: Returns a mutable sampler object by its OpenCL handle
// Author:      Uri Shomroni
// Date:        24/1/2010
// ---------------------------------------------------------------------------
apCLSampler* csSamplersMonitor::getSamplerDetails(oaCLSamplerHandle samplerHandle)
{
    apCLSampler* retVal = NULL;

    // Iterate the samplers:
    int numberOfSamplers = (int)_samplers.size();

    for (int i = 0; i < numberOfSamplers; i++)
    {
        // Get the current sampler:
        apCLSampler* pCurrentSampler = _samplers[i];
        GT_IF_WITH_ASSERT(pCurrentSampler != NULL)
        {
            // Check the handle:
            if (pCurrentSampler->samplerHandle() == samplerHandle)
            {
                // Return the sampler object:
                retVal = pCurrentSampler;

                // If we found a deleted sampler, keep looking - there might be a living one which
                // reuses the handle:
                if (!pCurrentSampler->wasMarkedForDeletion())
                {
                    break;
                }
            }
        }
    }

    return retVal;
}


