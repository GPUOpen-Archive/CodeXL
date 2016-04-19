//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Intercepted OpenCL APIs
//==============================================================================

#include <AMDTOSWrappers/Include/osThread.h>

#include "Logger.h"
#include "CLIntercept.h"
#include "CLOccupancyInfoManager.h"
#include "../CLCommon/CLFunctionDefs.h"
#include "../Common/OSUtils.h"
#include "../CLCommon/CLUtils.h"
#include "DeviceInfoUtils.h"

using namespace GPULogger;


const unsigned int KERNEL_MAX_DIM = 3;

cl_int CL_API_CALL
CL_OCCUPANCY_API_ENTRY_EnqueueNDRangeKernel(
    cl_command_queue cq,
    cl_kernel        kernel,
    cl_uint          wgDim,
    const size_t*    globalOffset,
    const size_t*    globalWS,
    const size_t*    localWS,
    cl_uint          nEventsInWaitList,
    const cl_event*  pEventList,
    cl_event*        pEvent)
{
    // call original API
    cl_int status = g_nextDispatchTable.EnqueueNDRangeKernel(cq,
                                                             kernel,
                                                             wgDim,
                                                             globalOffset,
                                                             globalWS,
                                                             localWS,
                                                             nEventsInWaitList,
                                                             pEventList,
                                                             pEvent);

    if (!OccupancyInfoManager::Instance()->IsProfilingEnabled())
    {
        // profiling is disabled by AMDTActivityLogger
        return status;
    }

    if (status != CL_SUCCESS)
    {
        return status;
    }

    cl_device_id device = NULL;
    cl_int occupancy_status = g_realDispatchTable.GetCommandQueueInfo(cq, CL_QUEUE_DEVICE, sizeof(cl_device_id), &device, NULL);

    if (occupancy_status != CL_SUCCESS)
    {
        Log(logERROR, "Unable to query the command queue device\n");
        return status;
    }

    cl_device_type deviceType;
    occupancy_status = g_realDispatchTable.GetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_device_type), &deviceType, NULL);

    if (occupancy_status != CL_SUCCESS)
    {
        Log(logERROR, "Unable to query the device type\n");
        return status;
    }

    if (deviceType != CL_DEVICE_TYPE_GPU)
    {
        Log(logMESSAGE, "Device is CPU.  Occupancy not supported\n");
        // Ignore CPU device
        return status;
    }

    //get the thread ID
    osThreadId tid = osGetUniqueCurrentThreadId();

    OccupancyInfoEntry* pEntry = new(std::nothrow) OccupancyInfoEntry();
    SpAssertRet(pEntry != NULL) status;

    pEntry->m_tid = tid;

    // Query the kernel name
    const size_t KERNEL_NAME_BUFFER_SIZE = 256;
    char szKernelNameBuffer[ KERNEL_NAME_BUFFER_SIZE ];
    occupancy_status = g_realDispatchTable.GetKernelInfo(kernel, CL_KERNEL_FUNCTION_NAME, KERNEL_NAME_BUFFER_SIZE, szKernelNameBuffer, NULL);

    if (occupancy_status != CL_SUCCESS)
    {
        Log(logERROR, "Unable to query the kernel name\n");
        pEntry->m_strKernelName = "UNKNOWN_KERNEL";
    }
    else
    {
        pEntry->m_strKernelName = szKernelNameBuffer;
    }

    // Get device name
    if (CLUtils::GetDeviceName(device, pEntry->m_strDeviceName) != CL_SUCCESS)
    {
        Log(logERROR, "Unable to get the device name\n");
        SAFE_DELETE(pEntry);
        return status;
    }

    GDT_HW_GENERATION gen;

    if (!AMDTDeviceInfoUtils::Instance()->GetHardwareGeneration(pEntry->m_strDeviceName.c_str(), gen))
    {
        Log(logERROR, "Unable to query the hw generation\n");
        SAFE_DELETE(pEntry);
        return status;
    }

    if (gen >= GDT_HW_GENERATION_VOLCANICISLAND && gen < GDT_HW_GENERATION_LAST)
    {
        pEntry->m_pCLCUInfo = new(std::nothrow) CLCUInfoVI();
    }
    else if (gen == GDT_HW_GENERATION_SOUTHERNISLAND || gen == GDT_HW_GENERATION_SEAISLAND)
    {
        pEntry->m_pCLCUInfo = new(std::nothrow) CLCUInfoSI();
    }
    else
    {
        Log(logERROR, "Unsupported hw generation\n");
        SAFE_DELETE(pEntry);
        return status;
    }

    SpAssertRet(pEntry->m_pCLCUInfo != NULL) status;

    //compute work-group size
    pEntry->m_nWorkGroupItemCount = 1;

    if (localWS == NULL)
    {
        occupancy_status = g_realDispatchTable.GetKernelWorkGroupInfo(kernel, device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &pEntry->m_nWorkGroupItemCount, NULL);

        if (occupancy_status != CL_SUCCESS)
        {
            Log(logERROR, "Unable to query the kernel workgroup info\n");
            SAFE_DELETE(pEntry);
            return status;
        }
    }
    else
    {
        for (unsigned int i = 0; i < wgDim; ++i)
        {
            pEntry->m_nWorkGroupItemCount *= localWS[i];
        }
    }

    // max work group size
    occupancy_status = g_realDispatchTable.GetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &pEntry->m_nWorkGroupItemCountMax, NULL);

    if (occupancy_status != CL_SUCCESS)
    {
        Log(logERROR, "Unable to query the device max workgroup size\n");
        SAFE_DELETE(pEntry);
        return status;
    }

    // global work-size
    pEntry->m_nGlobalItemCount = 1;

    if (globalWS == NULL)
    {
        pEntry->m_nGlobalItemCount = 0;
    }
    else
    {
        for (unsigned int i = 0; i < wgDim; i++)
        {
            pEntry->m_nGlobalItemCount *= globalWS[i];
        }
    }

    // max global work size
    size_t nDims;
    occupancy_status = g_realDispatchTable.GetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(size_t), &nDims, NULL);

    if (occupancy_status != CL_SUCCESS)
    {
        Log(logWARNING, "Unable to query the device max work item dimensions\n");
        pEntry->m_nGlobalItemCountMax = 0;
    }
    else
    {
        size_t nMaxSize[ KERNEL_MAX_DIM ];
        occupancy_status = g_realDispatchTable.GetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, nDims * sizeof(size_t), nMaxSize, NULL);

        if (occupancy_status != CL_SUCCESS)
        {
            Log(logWARNING, "Unable to query the device max work item sizes\n");
            pEntry->m_nGlobalItemCountMax = 0;
        }
        else
        {
            pEntry->m_nGlobalItemCountMax = 1;

            for (unsigned int j = 0; j < nDims; j++)
            {
                pEntry->m_nGlobalItemCountMax *= nMaxSize[j];
            }
        }
    }

    // max number of compute units
    occupancy_status = g_realDispatchTable.GetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(size_t), &pEntry->m_nNumberOfComputeUnits, NULL);

    if (occupancy_status != CL_SUCCESS)
    {
        Log(logERROR, "Unable to query the device max compute units\n");
        SAFE_DELETE(pEntry);
        return status;
    }

    //compute occupancy

    KernelInfo kinfo;

    if (!CLUtils::QueryKernelInfo(kernel, pEntry->m_strDeviceName, device, kinfo))
    {
        SAFE_DELETE(pEntry);
        Log(logERROR, "Unable to query the kernel info\n");
        return status;
    }

    pEntry->m_pCLCUInfo->SetCUParam(CU_PARAMS_VECTOR_GPRS_MAX, kinfo.m_nAvailableGPRs);
    pEntry->m_pCLCUInfo->SetCUParam(CU_PARAMS_SCALAR_GPRS_MAX, kinfo.m_nAvailableScalarGPRs);
    pEntry->m_pCLCUInfo->SetCUParam(CU_PARAMS_LDS_MAX, kinfo.m_nAvailableLDSSize);

    pEntry->m_pCLCUInfo->SetCUParam(CU_PARAMS_VECTOR_GPRS_USED, kinfo.m_nUsedGPRs);
    pEntry->m_pCLCUInfo->SetCUParam(CU_PARAMS_SCALAR_GPRS_USED, kinfo.m_nUsedScalarGPRs);
    pEntry->m_pCLCUInfo->SetCUParam(CU_PARAMS_LDS_USED, kinfo.m_nUsedLDSSize);

    pEntry->m_pCLCUInfo->SetCUParam(CU_PARAMS_KERNEL_WG_SIZE, pEntry->m_nWorkGroupItemCount);
    pEntry->m_pCLCUInfo->SetCUParam(CU_PARAMS_WG_SIZE_MAX, pEntry->m_nWorkGroupItemCountMax);
    pEntry->m_pCLCUInfo->SetCUParam(CU_PARAMS_KERNEL_GLOBAL_SIZE, pEntry->m_nGlobalItemCount);
    pEntry->m_pCLCUInfo->SetCUParam(CU_PARAMS_GLOBAL_SIZE_MAX, pEntry->m_nGlobalItemCountMax);

    pEntry->m_pCLCUInfo->SetCUParam(CU_PARAMS_NBR_COMPUTE_UNITS, pEntry->m_nNumberOfComputeUnits);

    pEntry->m_pCLCUInfo->SetCUParam(CU_PARAMS_DEVICE_NAME, pEntry->m_strDeviceName);

    int computeOccupancyStatus = pEntry->m_pCLCUInfo->ComputeCUOccupancy((unsigned int)pEntry->m_nWorkGroupItemCount);

    if (AMD_CUPARAMS_LOADED != computeOccupancyStatus)
    {
        Log(logERROR, "Unable to compute occupancy\n");
        SAFE_DELETE(pEntry);
        return status;
    }

    OccupancyInfoManager::Instance()->AddTraceInfoEntry(pEntry);
    return status;
}

cl_int CL_API_CALL
CL_OCCUPANCY_API_ENTRY_ReleaseContext(cl_context  context)
{
    // call entry in next dispatch table
    cl_int ret = g_nextDispatchTable.ReleaseContext(context);

    // In timeout mode, flush from clReleaseContext (just like in trace agent)
    if (OccupancyInfoManager::Instance()->IsTimeOutMode())
    {
        OccupancyInfoManager::Instance()->StopTimer();
        OccupancyInfoManager::Instance()->TrySwapBuffer();
        OccupancyInfoManager::Instance()->FlushTraceData(true);
        OccupancyInfoManager::Instance()->TrySwapBuffer();
        OccupancyInfoManager::Instance()->FlushTraceData(true);
        OccupancyInfoManager::Instance()->ResumeTimer();
    }

    return ret;
}
