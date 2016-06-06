//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLDevice.h
///
//==================================================================================

//------------------------------ apCLDevice.h ------------------------------

#ifndef __APCLDEVICE_H
#define __APCLDEVICE_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apAllocatedObject.h>
#include <AMDTAPIClasses/Include/apOpenCLParameters.h>

// The type of the clGetDeviceInfo function:
typedef cl_int(CL_API_CALL* clGetDeviceInfoProc)(cl_device_id device, cl_device_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);

enum apCLMemoryCacheType
{
    AP_CL_MEM_CACHE_NONE,
    AP_CL_MEM_READ_ONLY_CACHE,
    AP_CL_MEM_READ_WRITE_CACHE,
    AP_CL_MEM_UNKNOWN_CACHE
};

enum apCLLocalMemType
{
    AP_CL_MEM_LOCAL,
    AP_CL_MEM_GLOBAL,
    AP_CL_MEM_UNKNOWN
};

// User-readable values of device info values (instead of the apCLXXXXParameter::valueAsString functions, which show the enum names):
AP_API void apCLDeviceTypeAsString(const apCLDeviceTypeParameter& deviceType, gtString& deviceTypeAsStr);
AP_API int apCLDeviceIndexToDisplayIndex(int deviceAPIIndex);
AP_API void apCLFPConfigAsString(const apCLDeviceFloatingPointConfigParameter& deviceFPConfig, gtString& deviceFPConfigAsStr);
AP_API void apCLExecutionCapabilitiesAsString(const apCLDeviceExecutionCapabilitiesParameter& deviceExecCaps, gtString& deviceExecAsStr);
AP_API void apCLMemoryCacheTypeAsString(apCLMemoryCacheType deviceMemCacheType, gtString& deviceCacheTypeAsStr);
AP_API void apCLDeviceQueuePropertiesAsString(cl_command_queue_properties deviceQueueProperties, gtString& deviceQueuePropertiesAsStr);
AP_API void apCLDeviceSVMCapabilitiesAsString(cl_device_svm_capabilities deviceSVMCapabilities, gtString& deviceSVMCapabilitiesAsStr);
AP_API void apCLLocalMemTypeAsString(apCLLocalMemType memType, gtString& memTypeAsStr);

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLDevice : public apAllocatedObject
// General Description:
//   Represents an OpenCL device object.
//
// Author:  AMD Developer Tools Team
// Creation Date:        29/11/2009
// ----------------------------------------------------------------------------------
class AP_API apCLDevice : public apAllocatedObject
{
public:
    // Self functions:
    apCLDevice(gtInt32 APIID = -1, oaCLDeviceID deviceHandle = 0);
    virtual ~apCLDevice();

    bool initialize(clGetDeviceInfoProc pclGetDeviceInfo, bool fullAttributesList);

    // Device id:
    gtInt32 APIID() const { return _APIID; };
    oaCLDeviceID deviceHandle() const { return _deviceHandle; };

    // Device Address Bits:
    gtUInt32 addressBits() const { return _addressBits;}

    // Is device available:
    bool isAvailable()const { return _isAvailable; };

    // Is device compiler available:
    bool isCompilerAvailable() const { return _isCompilerAvailable; };

    // Deletion status:
    bool wasMarkedForDeletion() const {return _wasMarkedForDeletion;};
    void onDeviceMarkedForDeletion() {_wasMarkedForDeletion = true;};

    // Device floating point configuration:
    const apCLDeviceFloatingPointConfigParameter& deviceSingleFPConfig() const { return _deviceSingleFPConfig; };
    const apCLDeviceFloatingPointConfigParameter& deviceDoubleFPConfig() const { return _deviceDoubleFPConfig; };
    const apCLDeviceFloatingPointConfigParameter& deviceHalfFPConfig() const { return _deviceHalfFPConfig; };

    // Is device endian little:
    bool isEndianLittle() const { return _isEndianLittle; };

    // Is device error correction support:
    bool isDeviceErrorCorrectionSupport() const {return _isDeviceErrorCorrectionSupport;};

    // Device execution capabilities:
    const apCLDeviceExecutionCapabilitiesParameter& deviceExecutionCapabilities() const { return _deviceExecutionCapabilities; };

    // Global memory cache size:
    gtUInt64 globalMemCacheSize() const { return _globalMemCacheSize; };

    // Memory cache type:
    apCLMemoryCacheType memoryCacheType() const { return _memoryCacheType; };

    // Global memory cache line size:
    gtUInt32 globalMemCacheLineSize() const { return _globalMemCacheLineSize; };

    // Global memory size:
    gtUInt64 globalMemSize() const { return _globalMemSize; };

    // Does the host have unified memory:
    bool isHostUnifiedMemory() const { return _isHostUnifiedMemory; };

    // Is device support image:
    bool isImageSupport() const { return _isImageSupport; };

    // Max image dimensions:
    void maxImage2DDimension(gtUInt32& maxImage2DDimension1, gtUInt32& maxImage2DDimension2) const;
    void maxImage3DDimension(gtUInt32& maxImage3DDimension1, gtUInt32& maxImage3DDimension2, gtUInt32& maxImage3DDimension3) const;

    // Local memory size:
    gtUInt64 localMemSize() const { return _localMemSize; };

    // Local memory type:
    apCLLocalMemType localMemType() const { return _localMemType; };

    // Maximal clock frequency:
    gtUInt32 maxClockFrequency() const { return _maxClockFrequency; };

    // Maximal compute units:
    gtUInt32 maxComputeUnits() const { return _maxComputeUnits; };

    // Maximal constant arguments:
    gtUInt32 maxConstantArgs() const { return _maxConstantArgs; };

    // Maximal constant buffer size:
    gtUInt64 maxConstantBufferSize() const { return _maxConstantBufferSize; };

    // Maximal memory allocation size:
    gtUInt64 maxMemAllocSize() const { return _maxMemAllocSize; };

    // Maximal parameter size:
    gtUInt32 maxParamSize() const { return _maxParamSize; };

    // Maximal read image arguments:
    gtUInt32 maxReadImageArgs() const { return _maxReadImageArgs; };

    // Maximal samplers:
    gtUInt32 maxSamplers() const { return _maxSamplers; };

    // Maximal group size:
    gtUInt32 maxWorkgroupSize() const { return _maxWorkgroupSize; };

    // Maximal work item dimensions:
    gtUInt32 maxWorkItemDimensions() const { return _maxWorkItemDimensions; };

    // Maximal work item sizes:
    gtUInt32* maxWorkItemSizes() const { return _pMaxWorkItemSizes; };

    // Maximal write image arguments:
    gtUInt32 maxWriteImageArgs() const {return _maxWriteImageArgs;};
    gtUInt32 maxReadWriteImageArgs() const {return m_maxReadWriteImageArgs;};

    // Memory base address alignment:
    gtUInt32 memBaseAddrAlign() const { return _memBaseAddrAlign; };

    // Minimal data type alignment size:
    gtUInt32 minDataTypeAlignSize() const { return _minDataTypeAlignSize; };

    // Parse OpenCL version string:
    bool parseOpenCLVersionString();

    // Device platform ID:
    oaCLPlatformID platformID() const { return _platformID; };

    // Native vector width size for built-in scalar types:
    gtUInt32 nativeVecWidthChar() const { return _nativeVecWidthChar; };
    gtUInt32 nativeVecWidthShort() const { return _nativeVecWidthShort; };
    gtUInt32 nativeVecWidthInt() const { return _nativeVecWidthInt; };
    gtUInt32 nativeVecWidthLong() const { return _nativeVecWidthLong; };
    gtUInt32 nativeVecWidthFloat() const { return _nativeVecWidthFloat; };
    gtUInt32 nativeVecWidthDouble() const { return _nativeVecWidthDouble; };
    gtUInt32 nativeVecWidthHalf() const { return _nativeVecWidthHalf; };

    // Preferred native vector width size for built-in scalar types:
    gtUInt32 preferredVecWidthChar() const { return _preferredVecWidthChar; };
    gtUInt32 preferredVecWidthShort() const { return _preferredVecWidthShort; };
    gtUInt32 preferredVecWidthInt() const { return _preferredVecWidthInt; };
    gtUInt32 preferredVecWidthLong() const { return _preferredVecWidthLong; };
    gtUInt32 preferredVecWidthFloat() const { return _preferredVecWidthFloat; };
    gtUInt32 preferredVecWidthDouble() const { return _preferredVecWidthDouble; };
    gtUInt32 preferredVecWidthHalf() const { return _preferredVecWidthHalf; };

    // Device profile:
    const gtString& profileStr() const { return _profileStr; };

    // Device profiling timer resolution:
    gtUInt64 profilingTimerResolution() const { return _profilingTimerResolution; };

    // Device type:
    const apCLDeviceTypeParameter& deviceType() const { return _deviceType; };

    // Device name:
    const gtString& deviceName() const { return _deviceName; };
    const gtString& deviceNameForDisplay() const { return _deviceNameForDisplay; };

    // Device OpenCL C Version:
    const gtString& deviceOpenCLCVersion() const { return _openCLCVersion; };

    // Device version:
    const gtString& deviceVersion() const { return _deviceVersion; };
    int clMajorVersion() const { return _clMajorVersion; };
    int clMinorVersion() const { return _clMinorVersion; };

    // Device vendor:
    const gtString& deviceVendor() const { return _deviceVendor; };

    // Device vendor ID:
    gtUInt32 deviceVendorID() const { return _deviceVendorID; };

    // Queue properties:
    cl_command_queue_properties deviceQueueProperties() const { return _deviceQueueProperties; };

    // Program global variables:
    gtUInt64 deviceMaxGlobalVariableSize() const { return m_deviceMaxGlobalVariableSize; };
    gtUInt64 deviceGlobalVariablePreferredTotalSize() const { return m_deviceGlobalVariablePreferredTotalSize; };

    // On-device queues:
    cl_command_queue_properties queueOnDeviceProperties() const { return m_queueOnDeviceProperties; };
    gtUInt32 queueOnDevicePreferredSize() const { return m_queueOnDevicePreferredSize; };
    gtUInt32 queueOnDeviceMaxSize() const { return m_queueOnDeviceMaxSize; };
    gtUInt32 deviceMaxOnDeviceQueues() const { return m_deviceMaxOnDeviceQueues; };
    gtUInt32 deviceMaxOnDeviceEvents() const { return m_deviceMaxOnDeviceEvents; };

    // SVM capabilities:
    cl_device_svm_capabilities deviceSVMCapabilities() const { return m_deviceSVMCapabilities; };

    // Pipes:
    gtUInt32 deviceMaxPipeArgs() const { return m_deviceMaxPipeArgs; };
    gtUInt32 devicePipeMaxActiveReservations() const { return m_devicePipeMaxActiveReservations; };
    gtUInt32 devicePipeMaxPacketSize() const { return m_devicePipeMaxPacketSize; };

    // Atomic alignments:
    gtUInt32 devicePreferredPlatformAtomicAlignment() const { return m_devicePreferredPlatformAtomicAlignment; };
    gtUInt32 devicePreferredGlobalAtomicAlignment() const { return m_devicePreferredGlobalAtomicAlignment; };
    gtUInt32 devicePreferredLocalAtomicAlignment() const { return m_devicePreferredLocalAtomicAlignment; };

    // Device extensions:
    const gtVector<gtString>& extensions() const { return _extensions; };

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:
    bool deviceParamInfoAsString(clGetDeviceInfoProc pclGetDeviceInfo, cl_device_info param_name, gtString& paramValueAsStr);
    bool deviceParamInfoAsUInt(clGetDeviceInfoProc pclGetDeviceInfo, cl_device_info param_name, gtUInt32& paramValueAsUInt);
    bool deviceParamInfoAsULong(clGetDeviceInfoProc pclGetDeviceInfo, cl_device_info param_name, gtUInt64& paramValueAsUInt);
    bool deviceParamInfoAsBool(clGetDeviceInfoProc pclGetDeviceInfo, cl_device_info param_name, bool& paramValueAsBool);
    bool deviceParamInfoAsSizeT(clGetDeviceInfoProc pclGetDeviceInfo, cl_device_info param_name, size_t& paramValueAsSizeT);
    bool deviceParamInfoAsSizeTPtr(clGetDeviceInfoProc pclGetDeviceInfo, cl_device_info param_name, size_t*& pParamValueAsSizeTPtr, int pointerSize);
    bool deviceParamInfoAsPlatformID(clGetDeviceInfoProc pclGetDeviceInfo, cl_device_info param_name, oaCLPlatformID& paramValueAsPlatformID);
    bool getDeviceFPConfig(clGetDeviceInfoProc pclGetDeviceInfo, cl_device_info param_name, apCLDeviceFloatingPointConfigParameter& paramValueAsFPConfig);

    // Utilities for the device enumerated properties:
    bool getDeviceType(clGetDeviceInfoProc pclGetDeviceInfo);
    bool getCommandQueueProperties(clGetDeviceInfoProc pclGetDeviceInfo, cl_device_info param_name, cl_command_queue_properties& queueProps);
    bool getSVMCapabilities(clGetDeviceInfoProc pclGetDeviceInfo);
    bool getDeviceExecutionCapabilities(clGetDeviceInfoProc pclGetDeviceInfo);
    bool getDeviceMemoryCacheType(clGetDeviceInfoProc pclGetDeviceInfo);
    bool getMemoryCacheType(clGetDeviceInfoProc pclGetDeviceInfo);
    bool getDeviceLocalMemType(clGetDeviceInfoProc pclGetDeviceInfo);
    bool getDeviceExtensions(clGetDeviceInfoProc pclGetDeviceInfo);

private:
    // The API's ID for this device:
    gtInt32 _APIID;

    // Device handle (OpenCL id):
    oaCLDeviceID _deviceHandle;

    // Deletion status:
    bool _wasMarkedForDeletion;

    // Device Address Bits:
    gtUInt32 _addressBits;

    // Is device available:
    bool _isAvailable;

    // Is device compiler available:
    bool _isCompilerAvailable;

    // Device floating point configuration:
    apCLDeviceFloatingPointConfigParameter _deviceSingleFPConfig;
    apCLDeviceFloatingPointConfigParameter _deviceDoubleFPConfig;
    apCLDeviceFloatingPointConfigParameter _deviceHalfFPConfig;

    // Is device endian little:
    bool _isEndianLittle;

    // Is device error correction support:
    bool _isDeviceErrorCorrectionSupport;

    // Device execution capabilities:
    apCLDeviceExecutionCapabilitiesParameter _deviceExecutionCapabilities;

    // Global memory cache size:
    gtUInt64 _globalMemCacheSize;

    // Memory cache type:
    apCLMemoryCacheType _memoryCacheType;

    // Global memory cache line size:
    gtUInt32 _globalMemCacheLineSize;

    // Global memory size:
    gtUInt64 _globalMemSize;

    // Doest the host have unified memory:
    bool _isHostUnifiedMemory;

    // Is device support image:
    bool _isImageSupport;

    // Max image dimensions:
    gtUInt32 _maxImage2DDimension[2];
    gtUInt32 _maxImage3DDimension[3];

    // Local memory size:
    gtUInt64 _localMemSize;

    // Local memory type:
    apCLLocalMemType _localMemType;

    // Maximal clock frequency:
    gtUInt32 _maxClockFrequency;

    // Maximal compute units:
    gtUInt32 _maxComputeUnits;

    // Maximal constant arguments:
    gtUInt32 _maxConstantArgs;

    // Maximal constant buffer size:
    gtUInt64 _maxConstantBufferSize;

    // Maximal memory allocation size:
    gtUInt64 _maxMemAllocSize;

    // Maximal parameter size:
    gtUInt32 _maxParamSize;

    // Maximal read image arguments:
    gtUInt32 _maxReadImageArgs;

    // Maximal samplers:
    gtUInt32 _maxSamplers;

    // Maximal group size:
    gtUInt32 _maxWorkgroupSize;

    // Maximal work item dimensions:
    gtUInt32 _maxWorkItemDimensions;

    // Maximal work item sizes:
    gtUInt32* _pMaxWorkItemSizes;

    // Maximal write image arguments:
    gtUInt32 _maxWriteImageArgs;        // OpenCL 1.2 or older
    gtUInt32 m_maxReadWriteImageArgs;   // OpenCL 2.0 or newer

    // Memory base address alignment:
    gtUInt32 _memBaseAddrAlign;

    // Minimal data type alignment size:
    gtUInt32 _minDataTypeAlignSize;

    // Device name:
    gtString _deviceName;

    // Device name:
    gtString _deviceNameForDisplay;

    // OpenCL C Version:
    gtString _openCLCVersion;

    // Device platform ID:
    oaCLPlatformID _platformID;

    // Native vector width size for built-in scalar types:
    gtUInt32 _nativeVecWidthChar;
    gtUInt32 _nativeVecWidthShort;
    gtUInt32 _nativeVecWidthInt;
    gtUInt32 _nativeVecWidthLong;
    gtUInt32 _nativeVecWidthFloat;
    gtUInt32 _nativeVecWidthDouble;
    gtUInt32 _nativeVecWidthHalf;

    // Preferred native vector width size for built-in scalar types:
    gtUInt32 _preferredVecWidthChar;
    gtUInt32 _preferredVecWidthShort;
    gtUInt32 _preferredVecWidthInt;
    gtUInt32 _preferredVecWidthLong;
    gtUInt32 _preferredVecWidthFloat;
    gtUInt32 _preferredVecWidthDouble;
    gtUInt32 _preferredVecWidthHalf;

    // Device profile:
    gtString _profileStr;

    // Device profiling timer resolution:
    gtUInt64 _profilingTimerResolution;

    // Device type:
    apCLDeviceTypeParameter _deviceType;

    // Device vendor:
    gtString _deviceVendor;

    // Device vendor ID:
    gtUInt32 _deviceVendorID;

    // Device version:
    gtString _deviceVersion;
    gtUInt32 _clMajorVersion;
    gtUInt32 _clMinorVersion;

    // Driver version:
    gtString _driverVersion;

    // Queue properties:
    cl_command_queue_properties _deviceQueueProperties;

    // OpenCL 2.0:
    bool m_expectOpenCL20Properties;

    // Program global variables:
    gtUInt64 m_deviceMaxGlobalVariableSize;
    gtUInt64 m_deviceGlobalVariablePreferredTotalSize;

    // On-Device queues:
    cl_command_queue_properties m_queueOnDeviceProperties;
    gtUInt32 m_queueOnDevicePreferredSize;
    gtUInt32 m_queueOnDeviceMaxSize;
    gtUInt32 m_deviceMaxOnDeviceQueues;
    gtUInt32 m_deviceMaxOnDeviceEvents;

    // SVM capabilities:
    cl_device_svm_capabilities m_deviceSVMCapabilities;

    // Pipes:
    gtUInt32 m_deviceMaxPipeArgs;
    gtUInt32 m_devicePipeMaxActiveReservations;
    gtUInt32 m_devicePipeMaxPacketSize;

    // Atomic alignment:
    gtUInt32 m_devicePreferredPlatformAtomicAlignment;
    gtUInt32 m_devicePreferredGlobalAtomicAlignment;
    gtUInt32 m_devicePreferredLocalAtomicAlignment;

    // Device extensions:
    gtVector<gtString> _extensions;
};



#endif //__APCLDEVICE_H

