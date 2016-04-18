//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains utility functions for OpenCL API.
//==============================================================================

#ifndef _CL_UTILS_H_
#define _CL_UTILS_H_

#include <string>
#include <set>
#include <vector>
#include "CLPlatformInfo.h"
#include "KernelStats.h"

/// \defgroup CLUtils CLUtils
/// This module manages the OpenCL utility functions.
///
/// \ingroup CLProfileAgent
// @{

/// Utility function related to OpenCL
namespace CLUtils
{

/// Get the device name from a device id.
/// \param device           the CL device id
/// \param strDeviceNameOut the output device name
/// \return the CL error code (CL_SUCCESS if successful)
cl_int GetDeviceName(cl_device_id device, std::string& strDeviceNameOut);

/// Check whether the device is of deviceType given the device id.
/// \param device     the CL device id
/// \param deviceType the CL device type to check against
/// \return true if it is, false otherwise
bool IsDeviceType(cl_device_id device, cl_device_type deviceType);

/// Check whether the CL context contains a device of deviceType.
/// \param context     the CL context
/// \param deviceType  the CL device type to check against
/// \return true if the it does, false otherwise
bool HasDeviceType(const cl_context& context, cl_device_type deviceType);

/// Check if the list of devices includes a device of deviceType.
/// \param nDevices   the number of devices
/// \param pDevices   the list of devices
/// \param deviceType the CL device type to check against
/// \return true if the device is in the list, false otherwise
bool HasDeviceType(cl_uint             nDevices,
                   const cl_device_id* pDevices,
                   cl_device_type      deviceType);

/// Get the elapsed time (END - START) in milliseconds from a cl event.
/// \param pEvent the input cl event (must not be NULL).
/// \param dTimeOut the output elapsed time in milliseconds
/// \return true if successful, false otherwise
bool GetElapsedTimeFromEvent(const cl_event* pEvent, double& dTimeOut);

/// Get the information about all the platforms on a system
/// \param[out] platformList the list of available platforms
/// \return true if successful, false otherwise
bool GetPlatformInfo(CLPlatformSet& platformList);

/// Get the AMD platform, if it is available. The result is cached once it is retrieved,
/// so subsequent calls will return the same platform.
/// \return the AMD platform, if it is available, otherwise NULL (which can indicate the system default platform)
cl_platform_id GetDefaultPlatform();

/// Query kernel info from runtime
/// \param[in] kernel OpenCL kernel object
/// \param[in] strDeviceName OpenCL device name
/// \param[in] device OpenCL device object
/// \param[out] outKernelInfo output Kernel Info
bool QueryKernelInfo(cl_kernel kernel,
                     const std::string& strDeviceName,
                     cl_device_id device,
                     KernelInfo& outKernelInfo);

/// typedef for the queue properties vector
typedef std::vector<cl_queue_properties> QueuePropertiesList;

/// Utility code to ensure that a command queue created with clCreateCommandQueueWithProperties has profiling enabled
/// \param[in] properties the properties passed to clCreateCommandQueueWithProperties
/// \param[out] vecProperties vector of queue properties that will also include the profiling flag
/// \return true if the properties already had profiling enabled, false if not
bool EnableQueueProfiling(const cl_queue_properties* properties, QueuePropertiesList& vecProperties);
} // CLUtils

// @}

#endif // _CL_UTILS_H_
