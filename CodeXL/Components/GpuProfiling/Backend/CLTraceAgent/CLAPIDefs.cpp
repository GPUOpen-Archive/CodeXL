//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file defines all the OpenCL API objects.
//==============================================================================

#include "CLAPIDefs.h"
#include "CLAPIInfoManager.h"
#include "CLTraceAgent.h"
#include "../Common/Logger.h"

using namespace GPULogger;

cl_uint CLAPI_clCreateContextBase::ms_NumInstance = 0;
AMDTMutex CLAPI_clCreateContextBase::ms_mtx;
cl_uint CLAPI_clCreateCommandQueueBase::ms_NumInstance = 0;
AMDTMutex CLAPI_clCreateCommandQueueBase::ms_mtx;
bool CLAPI_clGetEventInfo::ms_collapseCalls = true;

void CLAPI_clCreateContextBase::AddToInfoManager(cl_context context)
{
    if (context != NULL)
    {
        CLAPIInfoManager::Instance()->AddToContextMap(context, this);
    }
}

void CLAPI_clCreateCommandQueueBase::Create(
    ULONGLONG ullStartTime,
    ULONGLONG ullEndTime,
    cl_context  context,
    cl_device_id   device,
    cl_int*  errcode_ret,
    cl_command_queue retVal)
{
    m_ullStart = ullStartTime;
    m_ullEnd = ullEndTime;
    m_context = context;
    m_device = device;

    m_errcode_ret = errcode_ret;

    if (errcode_ret != NULL)
    {
        m_errcode_retVal = *errcode_ret;
    }
    else
    {
        m_errcode_retVal = 0;
    }

    m_retVal = retVal;

    if (retVal != NULL)
    {
        CLAPIInfoManager::Instance()->AddToCommandQueueMap(retVal, this);
    }

    if (device != NULL)
    {
        cl_int ret = GetRealDispatchTable()->GetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_device_type), &m_dtype, NULL);
        ret = GetRealDispatchTable()->GetDeviceInfo(device, CL_DEVICE_NAME, sizeof(char) * MAX_DEVICE_NAME_STR, m_szDevice, NULL);

        if (ret != CL_SUCCESS)
        {
            // failed to retrieve device type
            Log(logWARNING, "Failed to retrieve device type.\n");
            m_dtype = CL_DEVICE_TYPE_DEFAULT;
        }
    }

    // query clCreateContextBase object from context object
    m_createContextAPIObj = CLAPIInfoManager::Instance()->GetCreateContextAPIObj(context);
}

void CLAPI_clCreateCommandQueue::Create(
    ULONGLONG ullStartTime,
    ULONGLONG ullEndTime,
    cl_context context,
    cl_device_id device,
    cl_command_queue_properties properties,
    cl_int* errcode_ret,
    cl_command_queue retVal)
{
    CLAPI_clCreateCommandQueueBase::Create(ullStartTime,
                                           ullEndTime,
                                           context,
                                           device,
                                           errcode_ret,
                                           retVal);

    m_type = CL_FUNC_TYPE_clCreateCommandQueue;

    m_properties = properties;

    if (m_properties & CL_QUEUE_PROFILING_ENABLE)
    {
        m_bUserSetProfileFlag = true;
    }
    else
    {
        m_bUserSetProfileFlag = false;
    }
}

void CLAPI_clCreateCommandQueueWithProperties::Create(
    ULONGLONG ullStartTime,
    ULONGLONG ullEndTime,
    cl_context context,
    cl_device_id device,
    const cl_queue_properties* pProperties,
    bool bUserSetProfileFlag,
    cl_int* errcode_ret,
    cl_command_queue retVal)
{
    CLAPI_clCreateCommandQueueBase::Create(ullStartTime,
                                           ullEndTime,
                                           context,
                                           device,
                                           errcode_ret,
                                           retVal);

    m_type = CL_FUNC_TYPE_clCreateCommandQueueWithProperties;

    m_pProperties = pProperties;

    int num_properties = 0;

    if (pProperties != NULL)
    {
        // properties is 0 terminated
        while (pProperties[0] != 0 && num_properties < SP_MAX_NUM_CONTEXT_PROPERTIES)
        {
            m_vecProperties.push_back(pProperties[0]);
            pProperties++;
            num_properties++;
        }
    }

    if (num_properties == SP_MAX_NUM_CONTEXT_PROPERTIES)
    {
        //add a dummy value (zero) that tells GetCommandQueuePropertiesString that the list has been truncated
        m_vecProperties.push_back(0);
    }

    m_bUserSetProfileFlag = bUserSetProfileFlag;
}


#define MAX_KERNEL_NAME_LEN 256

void CLAPI_clCreateKernelsInProgram::Create(
    ULONGLONG ullStartTime,
    ULONGLONG ullEndTime,
    cl_program  program,
    cl_uint  num_kernels,
    cl_kernel*  kernels,
    cl_uint* num_kernels_ret,
    bool replaced_null_param,
    cl_int retVal)
{
    m_ullStart = ullStartTime;
    m_ullEnd = ullEndTime;
    m_type = CL_FUNC_TYPE_clCreateKernelsInProgram;
    m_program = program;
    m_num_kernels = num_kernels;
    m_num_kernels_ret = num_kernels_ret;
    m_replaced_null_param = replaced_null_param;
    m_retVal = retVal;

    if (retVal == CL_SUCCESS)
    {
        char buf[MAX_KERNEL_NAME_LEN];

        m_num_kernels_retVal = *num_kernels_ret;
        cl_uint min_num_kernels = RETVALMIN(m_num_kernels_retVal, m_num_kernels);
        DeepCopyArray(&m_kernels, kernels, min_num_kernels);

        if (kernels != NULL)
        {
            for (cl_uint i = 0; i < min_num_kernels; i++)
            {
                // reset buf just in case
                memset(buf, 0, MAX_KERNEL_NAME_LEN * sizeof(char));
                // add all kernels to KernelMap
                cl_int ret = GetRealDispatchTable()->GetKernelInfo(kernels[ i ], CL_KERNEL_FUNCTION_NAME, MAX_KERNEL_NAME_LEN, buf, NULL);

                if (ret != CL_SUCCESS)
                {
                    Log(logWARNING, "Failed to retrieve kernel name.\n");
                }
                else
                {
                    CLAPIInfoManager::Instance()->AddToKernelMap(kernels[ i ], buf);
                }
            }
        }
    }
}


void CLAPI_clCreateKernel::Create(
    ULONGLONG ullStartTime,
    ULONGLONG ullEndTime,
    cl_program  program,
    const char* kernel_name,
    cl_int*  errcode_ret,
    cl_kernel retVal)
{
    m_ullStart = ullStartTime;
    m_ullEnd = ullEndTime;
    m_type = CL_FUNC_TYPE_clCreateKernel;
    m_program = program;

    if (kernel_name == NULL)
    {
        m_str_kernel_name = "NULL";
    }
    else
    {
        m_str_kernel_name = std::string(kernel_name);
    }

    m_kernel_name = kernel_name;

    m_errcode_ret = errcode_ret;

    if (errcode_ret != NULL)
    {
        m_errcode_retVal = *errcode_ret;
    }
    else
    {
        m_errcode_retVal = 0;
    }

    m_retVal = retVal;

    if (retVal != NULL)
    {
        CLAPIInfoManager::Instance()->AddToKernelMap(m_retVal, kernel_name);
    }
}
