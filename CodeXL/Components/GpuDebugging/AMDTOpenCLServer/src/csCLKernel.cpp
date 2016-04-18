//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csCLKernel.cpp
///
//==================================================================================

//------------------------------ csCLKernel.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTServerUtilities/Include/suIKernelDebuggingManager.h>
#include <AMDTServerUtilities/Include/suInterceptionMacros.h>

// Local:
#include <src/csAMDKernelDebuggingFunctionPointers.h>
#include <src/csCLKernel.h>
#include <src/csGlobalVariables.h>
#include <src/csMonitoredFunctionPointers.h>

#define CS_MAX_KERNEL_ARGUMENT_SIZE (1024 * 1024)

// ---------------------------------------------------------------------------
// Name:        csCLKernel::csCLKernel
// Description: Constructor
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
csCLKernel::csCLKernel(oaCLProgramHandle programHandle, int programIndex, oaCLKernelHandle kernelHandle, const gtString& kernelName)
    : apCLKernel(programHandle, programIndex, kernelHandle, kernelName), m_kernelUsesSVMPointers(false)
{

}

// ---------------------------------------------------------------------------
// Name:        csCLKernel::~csCLKernel
// Description: Destructor
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
csCLKernel::~csCLKernel()
{

}

// ---------------------------------------------------------------------------
// Name:        csCLKernel::onArgumentSet
// Description: Called when a kernel argument is set
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
void csCLKernel::onArgumentSet(cl_uint argIndex, gtSizeType argSize, const void* argValue, bool isSVMPointer)
{
    // If this is not a kernel-wide setting:
    if (argIndex != ((cl_uint) - 1))
    {
        // Create a struct for the argument:
        kernelArgumentData argData;
        argData._argumentSize = argSize;
        argData.m_isSVMPointer = isSVMPointer;

        // Some parameter types allow NULL as a value:
        if ((0 < argData._argumentSize) && (CS_MAX_KERNEL_ARGUMENT_SIZE > argData._argumentSize) && (argValue != NULL))
        {
            argData._argumentData = new gtUByte[argData._argumentSize];

            if (nullptr != argData._argumentData)
            {
                ::memcpy(argData._argumentData, argValue, argData._argumentSize);
            }
        }
        else // argValue == NULL
        {
            argData._argumentData = NULL;
        }

        // Set it into the map (possibly replacing the old value:
        _kernelArguments[argIndex] = argData;
    }
    else // argIndex != ((cl_uint)-1)
    {
        // Kernel-wide setting:
        if (isSVMPointer)
        {
            m_kernelUsesSVMPointers = true;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csCLKernel::restoreKernelArguments
// Description: Restore the kernel arguments after we've rebuilt it, causing its
//              internal handle to change.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
bool csCLKernel::restoreKernelArguments(oaCLKernelHandle kernelInternalHandle) const
{
    bool retVal = true;

    gtMap<unsigned int, kernelArgumentData>::const_iterator iter = _kernelArguments.begin();
    gtMap<unsigned int, kernelArgumentData>::const_iterator endIter = _kernelArguments.end();
    bool isSWKernelDebuggingOn = cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled() && (suIKernelDebuggingManager::CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER == cs_stat_pIKernelDebuggingManager->kernelDebuggerType());

    cl_kernel internalKernelHandleAsCLKernel = (cl_kernel)kernelInternalHandle;
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clSetKernelArg);
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clSetKernelArgSVMPointer);

    while (iter != endIter)
    {
        // Get the argument:
        cl_uint argIndex = (*iter).first;
        const kernelArgumentData& argData = (*iter).second;

        if (!argData.m_isSVMPointer)
        {
            // Set it:
            if (isSWKernelDebuggingOn)
            {
                cl_int retCode = cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptSetKernelArg(internalKernelHandleAsCLKernel, argIndex, argData._argumentSize, argData._argumentData);
                retVal = retVal && (retCode == CL_SUCCESS);
            }
            else // !isKernelDebugging
            {
                cl_int retCode = cs_stat_realFunctionPointers.clSetKernelArg(internalKernelHandleAsCLKernel, argIndex, argData._argumentSize, argData._argumentData);
                retVal = retVal && (retCode == CL_SUCCESS);
            }
        }
        else // argData.m_isSVMPointer
        {
            // Sanity check:
            GT_IF_WITH_ASSERT(NULL != cs_stat_realFunctionPointers.clSetKernelArgSVMPointer)
            {
                // Uri, 17/12/13 This is not currently supported by the software debugger:
                GT_ASSERT(!isSWKernelDebuggingOn);

                cl_int retCode = cs_stat_realFunctionPointers.clSetKernelArgSVMPointer(internalKernelHandleAsCLKernel, argIndex, argData._argumentData);
                retVal = retVal && (retCode == CL_SUCCESS);
            }
        }

        // Advance to the next argument:
        iter++;
    }

    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clSetKernelArgSVMPointer);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clSetKernelArg);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csCLKernel::kernelHasSVMArguments
// Description: Returns true iff this object has at least one kernel argument that
//              was set with clSetKernelArgSVMPointer.
// Author:      Uri Shomroni
// Date:        17/12/2013
// ---------------------------------------------------------------------------
bool csCLKernel::kernelHasSVMArguments() const
{
    bool retVal = false;

    // Iterate all the arguments that have values set:
    gtMap<unsigned int, kernelArgumentData>::const_iterator iter = _kernelArguments.begin();
    gtMap<unsigned int, kernelArgumentData>::const_iterator endIter = _kernelArguments.end();

    while (iter != endIter)
    {
        // Get the current argument's data:
        const kernelArgumentData& argData = (*iter).second;

        if (argData.m_isSVMPointer)
        {
            // If the kernel has at least one SVM argument, return true and stop looking:
            retVal = true;
            break;
        }

        // Advance to the next argument:
        iter++;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csCLKernel::kernelArgumentData::operator=
// Description: Assignment operator. Creates a copy of the data.
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
csCLKernel::kernelArgumentData& csCLKernel::kernelArgumentData::operator=(const csCLKernel::kernelArgumentData& other)
{
    // Clear the previous data:
    delete[] _argumentData;
    _argumentData = NULL;

    // Copy the argument size:
    _argumentSize = other._argumentSize;
    m_isSVMPointer = other.m_isSVMPointer;

    // Some parameter types allow NULL as a value:
    if (other._argumentData != NULL)
    {
        // Copy the data:
        _argumentData = new gtUByte[_argumentSize];
        ::memcpy(_argumentData, other._argumentData, _argumentSize);
    }

    return *this;
}
