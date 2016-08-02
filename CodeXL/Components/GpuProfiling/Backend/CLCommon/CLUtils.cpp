//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains utility functions for OpenCL API.
//==============================================================================

#include <CL/opencl.h>

#include <cstdio>
#include "CLUtils.h"
#include "CLFunctionDefs.h"
#include "CLInternalFunctionDefs.h"
#include "DeviceInfoUtils.h"
#include "Defs.h"
#include "Logger.h"
#include "CLPlatformInfo.h"
#include <cstring>

using namespace CLUtils;
using namespace GPULogger;

cl_int CLUtils::GetDeviceName(cl_device_id device, std::string& strDeviceNameOut)
{
    strDeviceNameOut = "";

    char pszDeviceName[256];
    cl_int result = g_realDispatchTable.GetDeviceInfo(device, CL_DEVICE_NAME, sizeof(pszDeviceName), pszDeviceName, NULL);

    if (CL_SUCCESS == result)
    {
        strDeviceNameOut = std::string(pszDeviceName);
    }

    return result;
}

bool CLUtils::IsDeviceType(cl_device_id device, cl_device_type deviceType)
{
    bool bRetVal = false;

    cl_device_type tempDeviceType;

    if (CL_SUCCESS == g_realDispatchTable.GetDeviceInfo(device,
                                                        CL_DEVICE_TYPE,
                                                        sizeof(cl_device_type),
                                                        &tempDeviceType,
                                                        NULL))
    {
        if (deviceType == tempDeviceType)
        {
            bRetVal = true;
        }
    }

    return bRetVal;
}

bool CLUtils::HasDeviceType(const cl_context& context, cl_device_type deviceType)
{
    bool bRetVal = false;

    // get the size of the device list
    size_t deviceListSize;
    cl_int status = g_realDispatchTable.GetContextInfo(context,
                                                       CL_CONTEXT_DEVICES,
                                                       0,
                                                       NULL,
                                                       &deviceListSize);

    cl_device_id* pDevices = new(std::nothrow) cl_device_id[deviceListSize];
    SpAssert(NULL != pDevices);

    if (NULL != pDevices)
    {
        // get the device list
        status |= g_realDispatchTable.GetContextInfo(context,
                                                     CL_CONTEXT_DEVICES,
                                                     deviceListSize,
                                                     pDevices,
                                                     NULL);

        if (status == CL_SUCCESS)
        {
            bRetVal = HasDeviceType(static_cast<cl_uint>(deviceListSize),
                                    pDevices,
                                    deviceType);
        }
    }

    delete[] pDevices;
    return bRetVal;
}

bool CLUtils::HasDeviceType(cl_uint             nDevices,
                            const cl_device_id* pDevices,
                            cl_device_type      deviceType)
{
    bool bRetVal = false;

    if (NULL != pDevices)
    {
        for (cl_uint i = 0; i < nDevices; ++i)
        {
            if (IsDeviceType(pDevices[i], deviceType))
            {
                bRetVal = true;
                break;
            }
        }
    }

    return bRetVal;
}

bool CLUtils::GetElapsedTimeFromEvent(const cl_event* pEvent, double& dTimeOut)
{
    bool bRetVal = false;

    if (NULL != pEvent)
    {
        // wait for the memory call to finish execution
        cl_int status = g_realDispatchTable.WaitForEvents(1, pEvent);

        cl_ulong llStartTime;
        cl_ulong llEndTime;
        status |= g_realDispatchTable.GetEventProfilingInfo(*pEvent,
                                                            CL_PROFILING_COMMAND_START,
                                                            sizeof(cl_ulong),
                                                            &llStartTime,
                                                            NULL);

        status |= g_realDispatchTable.GetEventProfilingInfo(*pEvent,
                                                            CL_PROFILING_COMMAND_END,
                                                            sizeof(cl_ulong),
                                                            &llEndTime,
                                                            NULL);

        /* Compute total time (also convert from nanoseconds to milliseconds) */
        dTimeOut = (double)(llEndTime - llStartTime) / 1e6;

        bRetVal = status == CL_SUCCESS;
    }

    return bRetVal;
}

bool CLUtils::GetPlatformInfo(CLPlatformSet& platformList)
{
    bool bRetVal = false;
    cl_int status = CL_SUCCESS;

    CLPlatformInfo::platform_info platformInfo;

    const unsigned int MAX_INFO_BUFFER_SIZE = 512;

    //Query the number of platforms/devices
    cl_uint nPlatformsDetected = 0;
    cl_uint nDevicesDetected = 0;
    cl_uint nDeviceAddressBits = 0;

    char szInfoBuffer[ MAX_INFO_BUFFER_SIZE ];
    size_t nBytesRead = 0;
    cl_platform_id platform;

    if (g_realDispatchTable.GetDeviceIDs != NULL)
    {
        status = g_realDispatchTable.GetDeviceIDs(NULL, CL_DEVICE_TYPE_ALL, 0, NULL, &nDevicesDetected);

        if (status != CL_SUCCESS)
        {
            // Note: the call to clGetDeviceIDs may fail if clGetPlatformIDs is not called first. Adding this dummy call to make sure we can query the devices
            status = g_realDispatchTable.GetPlatformIDs(0, NULL, &nPlatformsDetected);
            status |= g_realDispatchTable.GetDeviceIDs(NULL, CL_DEVICE_TYPE_ALL, 0, NULL, &nDevicesDetected);
        }

        if (status == CL_SUCCESS)
        {
            cl_device_id* pDevices = NULL;
            pDevices = new(std::nothrow) cl_device_id[nDevicesDetected];

            if (pDevices != NULL)
            {
                status = g_realDispatchTable.GetDeviceIDs(NULL, CL_DEVICE_TYPE_ALL, nDevicesDetected, pDevices, NULL);

                if (status == CL_SUCCESS)
                {
                    for (unsigned int i = 0; i < nDevicesDetected; ++i)
                    {
                        status = g_realDispatchTable.GetDeviceInfo(pDevices[i], CL_DEVICE_NAME, MAX_INFO_BUFFER_SIZE, szInfoBuffer, &nBytesRead);

                        if (status == CL_SUCCESS)
                        {
                            platformInfo.strDeviceName.assign(szInfoBuffer);
                        }

                        strcpy(szInfoBuffer, "");
                        status = g_realDispatchTable.GetDeviceInfo(pDevices[i], CL_DEVICE_PLATFORM, sizeof(cl_platform_id), &platform, &nBytesRead);

                        if (status == CL_SUCCESS)
                        {
                            status |= g_realDispatchTable.GetPlatformInfo(platform, CL_PLATFORM_NAME, MAX_INFO_BUFFER_SIZE, szInfoBuffer, &nBytesRead);
                        }

                        if (status == CL_SUCCESS)
                        {
                            platformInfo.strPlatformName.assign(szInfoBuffer);
                        }

                        strcpy(szInfoBuffer, "");
                        status |= g_realDispatchTable.GetPlatformInfo(platform, CL_PLATFORM_VERSION, MAX_INFO_BUFFER_SIZE, szInfoBuffer, &nBytesRead);

                        if (status == CL_SUCCESS)
                        {
                            platformInfo.strPlatformVersion.assign(szInfoBuffer);
                        }

                        strcpy(szInfoBuffer, "");
                        status |= g_realDispatchTable.GetPlatformInfo(platform, CL_PLATFORM_VENDOR, MAX_INFO_BUFFER_SIZE, szInfoBuffer, &nBytesRead);

                        if (status == CL_SUCCESS)
                        {
                            platformInfo.strPlatformVendor.assign(szInfoBuffer);
                        }

                        strcpy(szInfoBuffer, "");
                        status |= g_realDispatchTable.GetDeviceInfo(pDevices[i], CL_DRIVER_VERSION, MAX_INFO_BUFFER_SIZE, szInfoBuffer, &nBytesRead);

                        if (status == CL_SUCCESS)
                        {
                            platformInfo.strDriverVersion.assign(szInfoBuffer);
                        }

                        strcpy(szInfoBuffer, "");
                        status |= g_realDispatchTable.GetDeviceInfo(pDevices[i], CL_DEVICE_VERSION, MAX_INFO_BUFFER_SIZE, szInfoBuffer, &nBytesRead);

                        if (status == CL_SUCCESS)
                        {
                            platformInfo.strCLRuntime.assign(szInfoBuffer);
                        }

                        strcpy(szInfoBuffer, "");
                        status |= g_realDispatchTable.GetDeviceInfo(pDevices[i], CL_DEVICE_ADDRESS_BITS, sizeof(cl_uint), &nDeviceAddressBits, &nBytesRead);

                        if (status == CL_SUCCESS)
                        {
                            platformInfo.uiNbrAddressBits = nDeviceAddressBits;
                        }

                        strcpy(szInfoBuffer, "");
                        status |= g_realDispatchTable.GetDeviceInfo(pDevices[i], CL_DEVICE_BOARD_NAME_AMD, MAX_INFO_BUFFER_SIZE, szInfoBuffer, &nBytesRead);

                        if (status == CL_SUCCESS)
                        {
                            platformInfo.strBoardName.assign(szInfoBuffer);
                        }

                        if (status == CL_SUCCESS)
                        {
                            platformList.insert(platformInfo);
                        }
                    }
                }

                delete[] pDevices;
                bRetVal = platformList.size() > 0;
            }
        }
    }

    return bRetVal;
}

cl_platform_id CLUtils::GetDefaultPlatform()
{
    static cl_platform_id s_defaultPlatform = NULL;
    static bool s_bPlatformChecked = false;

    if (!s_bPlatformChecked)
    {
        s_bPlatformChecked = true;
        cl_uint numPlatforms;

        cl_int status = g_realDispatchTable.GetPlatformIDs(0, NULL, &numPlatforms);

        if (status == CL_SUCCESS && 0 < numPlatforms)
        {
            cl_platform_id* pPlatforms = new(std::nothrow) cl_platform_id[numPlatforms];

            if (pPlatforms != NULL)
            {
                status = g_realDispatchTable.GetPlatformIDs(numPlatforms, pPlatforms, NULL);

                if (status == CL_SUCCESS)
                {
                    for (unsigned i = 0; i < numPlatforms; ++i)
                    {
                        char pbuf[100] = "";
                        status = g_realDispatchTable.GetPlatformInfo(pPlatforms[i],
                                                                     CL_PLATFORM_VENDOR,
                                                                     sizeof(pbuf),
                                                                     pbuf,
                                                                     NULL);

                        if (!strcmp(pbuf, "Advanced Micro Devices, Inc."))
                        {
                            s_defaultPlatform = pPlatforms[i];
                            break;
                        }
                    }
                }

                delete[] pPlatforms;
            }
        }
    }

    return s_defaultPlatform;
}

bool CLUtils::QueryKernelInfo(cl_kernel kernel,
                              const std::string& strDeviceName,
                              cl_device_id device,
                              KernelInfo& outKernelInfo)
{
    bool bRetVal = true;

    if (clExtAMDDispatchTable::Instance()->GetKernelInfoAMD == NULL)
    {
        Log(logERROR, "CLUtils::QueryKernelInfo: GetKernelInfoAMD entry point not initialized\n");
        bRetVal = false;
    }
    else
    {
        size_t tmp;

        bool isGCN = false;

        if (!AMDTDeviceInfoUtils::Instance()->IsGCN(strDeviceName.c_str(), isGCN))
        {
            bRetVal = false;
            Log(logERROR, "CLUtils::QueryKernelInfo: IsGCN failed\n");
        }
        else
        {
            cl_int status;

            status = clExtAMDDispatchTable::Instance()->GetKernelInfoAMD(kernel, device, CL_KERNELINFO_SCRATCH_REGS, sizeof(tmp), &tmp, NULL);

            if (status == CL_SUCCESS)
            {
                outKernelInfo.m_nScratchReg = tmp;
            }
            else
            {
                bRetVal = false;
                Log(logERROR, "CLUtils::QueryKernelInfo: GetKernelInfoAMD(CL_KERNELINFO_SCRATCH_REGS) failed\n");
            }

            status = clExtAMDDispatchTable::Instance()->GetKernelInfoAMD(kernel, device, CL_KERNELINFO_WAVEFRONT_SIZE, sizeof(tmp), &tmp, NULL);

            if (status == CL_SUCCESS)
            {
                outKernelInfo.m_nWavefrontSize = tmp;
            }
            else
            {
                bRetVal = false;
                Log(logERROR, "CLUtils::QueryKernelInfo: GetKernelInfoAMD(CL_KERNELINFO_WAVEFRONT_SIZE) failed\n");
            }

            if (isGCN)
            {
                status = clExtAMDDispatchTable::Instance()->GetKernelInfoAMD(kernel, device, CL_KERNELINFO_AVAILABLE_VGPRS, sizeof(tmp), &tmp, NULL);

                if (status == CL_SUCCESS)
                {
                    outKernelInfo.m_nAvailableGPRs = tmp;
                }
                else
                {
                    bRetVal = false;
                    Log(logERROR, "CLUtils::QueryKernelInfo: GetKernelInfoAMD(CL_KERNELINFO_AVAILABLE_VGPRS) failed\n");
                }

                status = clExtAMDDispatchTable::Instance()->GetKernelInfoAMD(kernel, device, CL_KERNELINFO_USED_VGPRS, sizeof(tmp), &tmp, NULL);

                if (status == CL_SUCCESS)
                {
                    outKernelInfo.m_nUsedGPRs = tmp;
                }
                else
                {
                    bRetVal = false;
                    Log(logERROR, "CLUtils::QueryKernelInfo: GetKernelInfoAMD(CL_KERNELINFO_USED_VGPRS) failed\n");
                }

                status = clExtAMDDispatchTable::Instance()->GetKernelInfoAMD(kernel, device, CL_KERNELINFO_AVAILABLE_SGPRS, sizeof(tmp), &tmp, NULL);

                if (status == CL_SUCCESS)
                {
                    outKernelInfo.m_nAvailableScalarGPRs = tmp;
                }
                else
                {
                    bRetVal = false;
                    Log(logERROR, "CLUtils::QueryKernelInfo: GetKernelInfoAMD(CL_KERNELINFO_AVAILABLE_SGPRS) failed\n");
                }

                status = clExtAMDDispatchTable::Instance()->GetKernelInfoAMD(kernel, device, CL_KERNELINFO_USED_SGPRS, sizeof(tmp), &tmp, NULL);

                if (status == CL_SUCCESS)
                {
                    outKernelInfo.m_nUsedScalarGPRs = tmp;
                }
                else
                {
                    bRetVal = false;
                    Log(logERROR, "CLUtils::QueryKernelInfo: GetKernelInfoAMD(CL_KERNELINFO_USED_SGPRS) failed\n");
                }
            }
            else
            {
                status = clExtAMDDispatchTable::Instance()->GetKernelInfoAMD(kernel, device, CL_KERNELINFO_AVAILABLE_GPRS, sizeof(tmp), &tmp, NULL);

                if (status == CL_SUCCESS)
                {
                    outKernelInfo.m_nAvailableGPRs = tmp;
                }
                else
                {
                    bRetVal = false;
                    Log(logERROR, "CLUtils::QueryKernelInfo: GetKernelInfoAMD(CL_KERNELINFO_AVAILABLE_GPRS) failed\n");
                }

                status = clExtAMDDispatchTable::Instance()->GetKernelInfoAMD(kernel, device, CL_KERNELINFO_USED_GPRS, sizeof(tmp), &tmp, NULL);

                if (status == CL_SUCCESS)
                {
                    outKernelInfo.m_nUsedGPRs = tmp;
                }
                else
                {
                    bRetVal = false;
                    Log(logERROR, "CLUtils::QueryKernelInfo: GetKernelInfoAMD(CL_KERNELINFO_USED_GPRS) failed\n");
                }

                status = clExtAMDDispatchTable::Instance()->GetKernelInfoAMD(kernel, device, CL_KERNELINFO_AVAILABLE_STACK_SIZE, sizeof(tmp), &tmp, NULL);

                if (status == CL_SUCCESS)
                {
                    outKernelInfo.m_nAvailableStackSize = tmp;
                }
                else
                {
                    bRetVal = false;
                    Log(logERROR, "CLUtils::QueryKernelInfo: GetKernelInfoAMD(CL_KERNELINFO_AVAILABLE_STACK_SIZE) failed\n");
                }

                status = clExtAMDDispatchTable::Instance()->GetKernelInfoAMD(kernel, device, CL_KERNELINFO_USED_STACK_SIZE, sizeof(tmp), &tmp, NULL);

                if (status == CL_SUCCESS)
                {
                    outKernelInfo.m_nUsedStackSize = tmp;
                }
                else
                {
                    bRetVal = false;
                    Log(logERROR, "CLUtils::QueryKernelInfo: GetKernelInfoAMD(CL_KERNELINFO_USED_STACK_SIZE) failed\n");
                }
            }

            status = g_realDispatchTable.GetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE_PER_COMPUTE_UNIT_AMD, sizeof(tmp), &tmp, NULL);

            if (status == CL_SUCCESS)
            {
                outKernelInfo.m_nAvailableLDSSize = tmp;
            }
            else
            {
                bRetVal = false;
                Log(logERROR, "CLUtils::QueryKernelInfo: GetDeviceInfo(CL_DEVICE_LOCAL_MEM_SIZE_PER_COMPUTE_UNIT_AMD) failed\n");
            }

            // get the used local memory by the kernel
            cl_ulong ulLocalMem = 0;

            status = g_realDispatchTable.GetKernelWorkGroupInfo(kernel, device,
                                                                CL_KERNEL_LOCAL_MEM_SIZE,
                                                                sizeof(cl_ulong), &ulLocalMem, NULL);

            if (status == CL_SUCCESS)
            {
                outKernelInfo.m_nUsedLDSSize = (unsigned long) ulLocalMem;
            }
            else
            {
                bRetVal = false;
                Log(logERROR, "CLUtils::QueryKernelInfo: GetKernelWorkGroupInfo(CL_KERNEL_LOCAL_MEM_SIZE) failed\n");
            }

            SP_TODO("Replace clGetKernelWorkGroupInfo with internal extension GetKernelInfoAMD when this extension is fixed")

            //   if (clExtAMDDispatchTable::Instance()->GetKernelInfoAMD(kernel, device, CL_KERNELINFO_USED_LDS_SIZE, sizeof(tmp), &tmp, NULL) == CL_SUCCESS)
            //   {
            //      outKernelInfo.m_nUsedLDSSize = tmp;
            //   }

            GDT_DeviceInfo dinfo;

            if (AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(strDeviceName.c_str(), dinfo))
            {
                outKernelInfo.m_nWavefrontPerSIMD = dinfo.m_nMaxWavePerSIMD;
            }
            else
            {
                bRetVal = false;
                Log(logERROR, "CLUtils::QueryKernelInfo: GetDeviceInfo failed\n");
            }
        }
    }

    return bRetVal;
}

bool CLUtils::EnableQueueProfiling(const cl_queue_properties* properties, QueuePropertiesList& vecProperties)
{
    bool bQueuePropsFound = false;
    bool bUserSetProfileFlag = false;

    if (properties != NULL)
    {
        // properties is 0 terminated
        while (properties[0] != 0)
        {
            vecProperties.push_back(properties[0]);
            bQueuePropsFound = properties[0] == CL_QUEUE_PROPERTIES;

            if (bQueuePropsFound)
            {
                properties++;

                cl_command_queue_properties props = properties[0];

                if ((props & CL_QUEUE_PROFILING_ENABLE) == 0)
                {
                    props |= CL_QUEUE_PROFILING_ENABLE;
                }
                else
                {
                    bUserSetProfileFlag = true;
                }

                vecProperties.push_back((cl_queue_properties)props);
            }

            if (properties[0] == CL_QUEUE_SIZE)
            {
                properties++;
                vecProperties.push_back((cl_queue_properties)properties[0]);
            }

            properties++;
        }
    }

    if (!bQueuePropsFound)
    {
        cl_command_queue_properties props = CL_QUEUE_PROFILING_ENABLE;
        vecProperties.insert(vecProperties.begin(), (cl_queue_properties)props);
        vecProperties.insert(vecProperties.begin(), CL_QUEUE_PROPERTIES);
    }

    vecProperties.push_back(0);

    return bUserSetProfileFlag;
}

