//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csOpenCLExtensionsWrappers.cpp
///
//==================================================================================

//------------------------------ csOpenCLExtensionsWrappers.cpp ------------------------------


// ---------------------------------------------------------------------------
// File:
//  This file contains wrapper functions for the supported OpenCL extensions
//  functions.
// ---------------------------------------------------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Local:
#include <src/csWrappersCommon.h>
#include <src/csExtensionsManager.h>
#include <src/csGlobalVariables.h>
#include <src/csOpenCLMonitor.h>

// Export AMD-only extension as extern C:

#ifdef __cplusplus
extern "C" {
#endif
extern CL_API_ENTRY cl_int CL_API_CALL clBeginComputationFrameAMD(cl_context context);
extern CL_API_ENTRY cl_int CL_API_CALL clEndComputationFrameAMD(cl_context context);
extern CL_API_ENTRY cl_int CL_API_CALL clNameContextAMD(cl_context context, const char* name, size_t length);
extern CL_API_ENTRY cl_int CL_API_CALL clNameCommandQueueAMD(cl_command_queue command_queue, const char* name, size_t length);
extern CL_API_ENTRY cl_int CL_API_CALL clNameMemObjectAMD(cl_mem memobj, const char* name, size_t length);
extern CL_API_ENTRY cl_int CL_API_CALL clNameSamplerAMD(cl_sampler sampler, const char* name, size_t length);
extern CL_API_ENTRY cl_int CL_API_CALL clNameProgramAMD(cl_program program, const char* name, size_t length);
extern CL_API_ENTRY cl_int CL_API_CALL clNameKernelAMD(cl_kernel kernel, const char* name, size_t length);
extern CL_API_ENTRY cl_int CL_API_CALL clNameEventAMD(cl_event event, const char* name, size_t length);
#ifdef __cplusplus
}
#endif


// --------------------------------------------------------
//             OpenGL Extensions Wrapper functions
// --------------------------------------------------------
//////////////////////////////////////////////////////////////////////////
// cl_
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// cl_
//////////////////////////////////////////////////////////////////////////

// Add new extensions here:


// --------------------------------------------------------
//             Graphic Remedy extensions Wrapper functions
// --------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// cl_amd_computation_frame
//////////////////////////////////////////////////////////////////////////
CL_API_ENTRY cl_int CL_API_CALL clBeginComputationFrameAMD(cl_context context)
{
    cl_int retVal = CL_SUCCESS;

    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)context, ap_clBeginComputationFrameAMD, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context);

    csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor((oaCLContextHandle)context);

    if (pContextMonitor != NULL)
    {
        bool isAlreadyInFrame = pContextMonitor->isInComputationFrame();

        if (!isAlreadyInFrame)
        {
            pContextMonitor->onComputationFrameStarted();
        }
        else // isAlreadyInFrame
        {
            retVal = CL_INVALID_OPERATION;
        }
    }
    else // pContextMonitor == NULL
    {
        retVal = CL_INVALID_CONTEXT;
    }

    return retVal;
}

CL_API_ENTRY cl_int CL_API_CALL clEndComputationFrameAMD(cl_context context)
{
    cl_int retVal = CL_SUCCESS;

    cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)context, ap_clEndComputationFrameAMD, 1, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context);

    csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor((oaCLContextHandle)context);

    if (pContextMonitor != NULL)
    {
        bool isAlreadyInFrame = pContextMonitor->isInComputationFrame();

        if (isAlreadyInFrame)
        {
            pContextMonitor->onComputationFrameEnded();
        }
        else // isAlreadyInFrame
        {
            retVal = CL_INVALID_OPERATION;
        }
    }
    else // pContextMonitor == NULL
    {
        retVal = CL_INVALID_CONTEXT;
    }

    return retVal;
}



//////////////////////////////////////////////////////////////////////////
// cl_amd_object_naming
//////////////////////////////////////////////////////////////////////////
CL_API_ENTRY cl_int CL_API_CALL clNameContextAMD(cl_context context, const char* name, size_t length)
{
    cl_int retVal = CL_SUCCESS;

    // Will get the new name
    gtString newName;

    if (name != NULL)
    {
        // If we got a string
        if (length > 0)
        {
            newName.fromASCIIString(name, (int)length);
        }
        else // (length == 0)
        {
            newName.fromASCIIString(name);
        }

        // Log the call to the function:
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)context, ap_clNameContextAMD, 2, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_STRING_PARAMETER, newName.asCharArray());
    }
    else // (name == NULL)
    {
        // We only allow clearing names with length == 0:
        if (length > 0)
        {
            // Generate an error (and don't perform the operation):
            retVal = CL_INVALID_VALUE;
        }

        // Log the call to the function:
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)context, ap_clNameContextAMD, 3, OS_TOBJ_ID_CL_HANDLE_PARAMETER, context, OS_TOBJ_ID_POINTER_PARAMETER, name, OS_TOBJ_ID_SIZE_T_PARAMETER, length);
    }

    // If the name string is valid:
    if (retVal == CL_SUCCESS)
    {
        // Get the context to be named:
        csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.clContextMonitor((oaCLContextHandle)context);

        if (pContextMonitor != NULL)
        {
            // Check the current name:
            const gtString& currentName = pContextMonitor->contextName();

            if (currentName.isEmpty() || newName.isEmpty())
            {
                pContextMonitor->setContextName(newName);
                cs_stat_openCLMonitorInstance.openCLHandleMonitor().nameHandledObject((oaCLHandle)context, newName);
            }
            else // (!currentName.isEmpty() && !newName.isEmpty())
            {
                // We do not allow renaming without clearing the name:
                retVal = CL_INVALID_OPERATION;
            }
        }
        else
        {
            // Context not found:
            retVal = CL_INVALID_CONTEXT;
        }
    }

    return retVal;
}

CL_API_ENTRY cl_int CL_API_CALL clNameCommandQueueAMD(cl_command_queue command_queue, const char* name, size_t length)
{
    cl_int retVal = CL_SUCCESS;

    // Will get the new name
    gtString newName;

    if (name != NULL)
    {
        // If we got a string
        if (length > 0)
        {
            newName.fromASCIIString(name, (int)length);
        }
        else // (length == 0)
        {
            newName.fromASCIIString(name);
        }

        // Log the call to the function:
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clNameCommandQueueAMD, 2, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_STRING_PARAMETER, newName.asCharArray());
    }
    else // (name == NULL)
    {
        // We only allow clearing names with length == 0:
        if (length > 0)
        {
            // Generate an error (and don't perform the operation):
            retVal = CL_INVALID_VALUE;
        }

        // Log the call to the function:
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)command_queue, ap_clNameCommandQueueAMD, 3, OS_TOBJ_ID_CL_HANDLE_PARAMETER, command_queue, OS_TOBJ_ID_POINTER_PARAMETER, name, OS_TOBJ_ID_SIZE_T_PARAMETER, length);
    }

    // If the name string is valid:
    if (retVal == CL_SUCCESS)
    {
        // verify we get the queue name:
        retVal = CL_INVALID_COMMAND_QUEUE;

        // Get the queue to be named:
        csCommandQueueMonitor* pCommandQueuetMonitor = cs_stat_openCLMonitorInstance.commandQueueMonitor((oaCLCommandQueueHandle)command_queue);
        GT_IF_WITH_ASSERT(pCommandQueuetMonitor != NULL)
        {
            // Check the current name:
            apCLCommandQueue& queueInfo = pCommandQueuetMonitor->commandQueueInfo();
            const gtString& currentName = queueInfo.queueName();

            if (currentName.isEmpty() || newName.isEmpty())
            {
                // Name the queue / clear the name:
                queueInfo.setQueueName(newName);
                cs_stat_openCLMonitorInstance.openCLHandleMonitor().nameHandledObject((oaCLHandle)command_queue, newName);
                retVal = CL_SUCCESS;
            }
            else // (!currentName.isEmpty() && !newName.isEmpty())
            {
                // We do not allow renaming without clearing the name:
                retVal = CL_INVALID_OPERATION;
            }
        }
    }

    return retVal;
}

CL_API_ENTRY cl_int CL_API_CALL clNameMemObjectAMD(cl_mem memobj, const char* name, size_t length)
{
    cl_int retVal = CL_SUCCESS;

    // Will get the new name
    gtString newName;

    if (name != NULL)
    {
        // If we got a string
        if (length > 0)
        {
            newName.fromASCIIString(name, (int)length);
        }
        else // (length == 0)
        {
            newName.fromASCIIString(name);
        }

        // Log the call to the function:
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)memobj, ap_clNameMemObjectAMD, 2, OS_TOBJ_ID_CL_HANDLE_PARAMETER, memobj, OS_TOBJ_ID_STRING_PARAMETER, newName.asCharArray());
    }
    else // (name == NULL)
    {
        // We only allow clearing names with length == 0:
        if (length > 0)
        {
            // Generate an error (and don't perform the operation):
            retVal = CL_INVALID_VALUE;
        }

        // Log the call to the function:
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)memobj, ap_clNameMemObjectAMD, 3, OS_TOBJ_ID_CL_HANDLE_PARAMETER, memobj, OS_TOBJ_ID_POINTER_PARAMETER, name, OS_TOBJ_ID_SIZE_T_PARAMETER, length);
    }

    // If the name string is valid:
    if (retVal == CL_SUCCESS)
    {
        // verify we get the mem object:
        retVal = CL_INVALID_MEM_OBJECT;

        // Get the mem object to be named:
        csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.contextContainingMemObject((oaCLMemHandle)memobj);

        if (pContextMonitor != NULL)
        {
            apCLMemObject* pMemObj = pContextMonitor->imagesAndBuffersMonitor().getMemObjectDetails((oaCLMemHandle)memobj);

            if (pMemObj != NULL)
            {
                // Check the current name:
                const gtString& currentName = pMemObj->memObjectName();

                if (currentName.isEmpty() || newName.isEmpty())
                {
                    // Name the mem object / clear the name:
                    pMemObj->setMemObjectName(newName);
                    cs_stat_openCLMonitorInstance.openCLHandleMonitor().nameHandledObject((oaCLHandle)memobj, newName);
                    retVal = CL_SUCCESS;
                }
                else // (!currentName.isEmpty() && !newName.isEmpty())
                {
                    // We do not allow renaming without clearing the name:
                    retVal = CL_INVALID_OPERATION;
                }
            }
        }
    }

    return retVal;
}

CL_API_ENTRY cl_int CL_API_CALL clNameSamplerAMD(cl_sampler sampler, const char* name, size_t length)
{
    cl_int retVal = CL_SUCCESS;

    // Will get the new name
    gtString newName;

    if (name != NULL)
    {
        // If we got a string
        if (length > 0)
        {
            newName.fromASCIIString(name, (int)length);
        }
        else // (length == 0)
        {
            newName.fromASCIIString(name);
        }

        // Log the call to the function:
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)sampler, ap_clNameSamplerAMD, 2, OS_TOBJ_ID_CL_HANDLE_PARAMETER, sampler, OS_TOBJ_ID_STRING_PARAMETER, newName.asCharArray());
    }
    else // (name == NULL)
    {
        // We only allow clearing names with length == 0:
        if (length > 0)
        {
            // Generate an error (and don't perform the operation):
            retVal = CL_INVALID_VALUE;
        }

        // Log the call to the function:
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)sampler, ap_clNameSamplerAMD, 3, OS_TOBJ_ID_CL_HANDLE_PARAMETER, sampler, OS_TOBJ_ID_POINTER_PARAMETER, name, OS_TOBJ_ID_SIZE_T_PARAMETER, length);
    }

    // If the name string is valid:
    if (retVal == CL_SUCCESS)
    {
        // verify we get the sampler name:
        retVal = CL_INVALID_SAMPLER;

        // Get the sampler to be named:
        csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.contextContainingSampler((oaCLSamplerHandle)sampler);

        if (pContextMonitor != NULL)
        {
            apCLSampler* pSampler = pContextMonitor->samplersMonitor().getSamplerDetails((oaCLSamplerHandle)sampler);

            if (pSampler != NULL)
            {
                // Check the current name:
                const gtString& currentName = pSampler->samplerName();

                if (currentName.isEmpty() || newName.isEmpty())
                {
                    // Name the sampler / clear the name:
                    pSampler->setSamplerName(newName);
                    cs_stat_openCLMonitorInstance.openCLHandleMonitor().nameHandledObject((oaCLHandle)sampler, newName);
                    retVal = CL_SUCCESS;
                }
                else // (!currentName.isEmpty() && !newName.isEmpty())
                {
                    // We do not allow renaming without clearing the name:
                    retVal = CL_INVALID_OPERATION;
                }
            }
        }
    }

    return retVal;
}

CL_API_ENTRY cl_int CL_API_CALL clNameProgramAMD(cl_program program, const char* name, size_t length)
{
    cl_int retVal = CL_SUCCESS;

    // Will get the new name
    gtString newName;

    if (name != NULL)
    {
        // If we got a string
        if (length > 0)
        {
            newName.fromASCIIString(name, (int)length);
        }
        else // (length == 0)
        {
            newName.fromASCIIString(name);
        }

        // Log the call to the function:
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)program, ap_clNameProgramAMD, 2, OS_TOBJ_ID_CL_HANDLE_PARAMETER, program, OS_TOBJ_ID_STRING_PARAMETER, newName.asCharArray());
    }
    else // (name == NULL)
    {
        // We only allow clearing names with length == 0:
        if (length > 0)
        {
            // Generate an error (and don't perform the operation):
            retVal = CL_INVALID_VALUE;
        }

        // Log the call to the function:
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)program, ap_clNameProgramAMD, 3, OS_TOBJ_ID_CL_HANDLE_PARAMETER, program, OS_TOBJ_ID_POINTER_PARAMETER, name, OS_TOBJ_ID_SIZE_T_PARAMETER, length);
    }

    // If the name string is valid:
    if (retVal == CL_SUCCESS)
    {
        // verify we get the program name:
        retVal = CL_INVALID_PROGRAM;

        // Get the program to be named:
        csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.contextContainingProgram((oaCLProgramHandle)program);

        if (pContextMonitor != NULL)
        {
            apCLProgram* pProgram = pContextMonitor->programsAndKernelsMonitor().programMonitor((oaCLProgramHandle)program);

            if (pProgram != NULL)
            {
                // Check the current name:
                const gtString& currentName = pProgram->programName();

                if (currentName.isEmpty() || newName.isEmpty())
                {
                    // Name the program / clear the name:
                    pProgram->setProgramName(newName);
                    cs_stat_openCLMonitorInstance.openCLHandleMonitor().nameHandledObject((oaCLHandle)program, newName);
                    retVal = CL_SUCCESS;
                }
                else // (!currentName.isEmpty() && !newName.isEmpty())
                {
                    // We do not allow renaming without clearing the name:
                    retVal = CL_INVALID_OPERATION;
                }
            }
        }
    }

    return retVal;
}

CL_API_ENTRY cl_int CL_API_CALL clNameKernelAMD(cl_kernel kernel, const char* name, size_t length)
{
    cl_int retVal = CL_SUCCESS;

    // Will get the new name
    gtString newName;

    if (name != NULL)
    {
        // If we got a string
        if (length > 0)
        {
            newName.fromASCIIString(name, (int)length);
        }
        else // (length == 0)
        {
            newName.fromASCIIString(name);
        }

        // Log the call to the function:
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)kernel, ap_clNameKernelAMD, 2, OS_TOBJ_ID_CL_HANDLE_PARAMETER, kernel, OS_TOBJ_ID_STRING_PARAMETER, newName.asCharArray());
    }
    else // (name == NULL)
    {
        // We only allow clearing names with length == 0:
        if (length > 0)
        {
            // Generate an error (and don't perform the operation):
            retVal = CL_INVALID_VALUE;
        }

        // Log the call to the function:
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)kernel, ap_clNameKernelAMD, 3, OS_TOBJ_ID_CL_HANDLE_PARAMETER, kernel, OS_TOBJ_ID_POINTER_PARAMETER, name, OS_TOBJ_ID_SIZE_T_PARAMETER, length);
    }

    // If the name string is valid:
    if (retVal == CL_SUCCESS)
    {
        // verify we get the kernel name:
        retVal = CL_INVALID_KERNEL;

        // Get the kernel to be named:
        csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.contextContainingKernel((oaCLKernelHandle)kernel);

        if (pContextMonitor != NULL)
        {
            apCLKernel* pKernel = pContextMonitor->programsAndKernelsMonitor().kernelMonitor((oaCLKernelHandle)kernel);

            if (pKernel != NULL)
            {
                // Check the current name:
                const gtString& currentName = pKernel->kernelName();

                if (currentName.isEmpty() || newName.isEmpty())
                {
                    // Name the kernel / clear the name:
                    pKernel->setKernelName(newName);
                    cs_stat_openCLMonitorInstance.openCLHandleMonitor().nameHandledObject((oaCLHandle)kernel, newName);
                    retVal = CL_SUCCESS;
                }
                else // (!currentName.isEmpty() && !newName.isEmpty())
                {
                    // We do not allow renaming without clearing the name:
                    retVal = CL_INVALID_OPERATION;
                }
            }
        }
    }

    return retVal;
}

CL_API_ENTRY cl_int CL_API_CALL clNameEventAMD(cl_event event, const char* name, size_t length)
{
    cl_int retVal = CL_SUCCESS;

    // Will get the new name
    gtString newName;

    if (name != NULL)
    {
        // If we got a string
        if (length > 0)
        {
            newName.fromASCIIString(name, (int)length);
        }
        else // (length == 0)
        {
            newName.fromASCIIString(name);
        }

        // Log the call to the function:
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)event, ap_clNameEventAMD, 2, OS_TOBJ_ID_CL_HANDLE_PARAMETER, event, OS_TOBJ_ID_STRING_PARAMETER, newName.asCharArray());
    }
    else // (name == NULL)
    {
        // We only allow clearing names with length == 0:
        if (length > 0)
        {
            // Generate an error (and don't perform the operation):
            retVal = CL_INVALID_VALUE;
        }

        // Log the call to the function:
        cs_stat_openCLMonitorInstance.addFunctionCall((oaCLHandle)event, ap_clNameEventAMD, 3, OS_TOBJ_ID_CL_HANDLE_PARAMETER, event, OS_TOBJ_ID_POINTER_PARAMETER, name, OS_TOBJ_ID_SIZE_T_PARAMETER, length);
    }

    // If the name string is valid:
    if (retVal == CL_SUCCESS)
    {
        // verify we get the event name:
        retVal = CL_INVALID_EVENT;

        // Get the event to be named:
        csContextMonitor* pContextMonitor = cs_stat_openCLMonitorInstance.contextContainingEvent((oaCLEventHandle)event);
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            const apCLEvent* pEvent = pContextMonitor->eventsMonitor().eventDetails((oaCLEventHandle)event);

            if (pEvent != NULL)
            {
                // Check the current name:
                const gtString& currentName = pEvent->eventName();

                if (currentName.isEmpty() || newName.isEmpty())
                {
                    // Name the event / clear the name:
                    ((apCLEvent*)pEvent)->setEventName(newName);
                    cs_stat_openCLMonitorInstance.openCLHandleMonitor().nameHandledObject((oaCLHandle)event, newName);
                    retVal = CL_SUCCESS;
                }
                else // (!currentName.isEmpty() && !newName.isEmpty())
                {
                    // We do not allow renaming without clearing the name:
                    retVal = CL_INVALID_OPERATION;
                }
            }
        }
    }

    return retVal;
}

