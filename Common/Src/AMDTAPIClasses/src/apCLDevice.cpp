//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLDevice.cpp
///
//==================================================================================

//------------------------------ apCLDevice.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/apStringConstants.h>
#include <AMDTAPIClasses/Include/apCLDevice.h>


// ---------------------------------------------------------------------------
// Name:        apCLDeviceTypeAsString
// Description: Input an OpenCL device type and returns a human readable string that represents it
// Author:  AMD Developer Tools Team
// Date:        21/1/2010
// ---------------------------------------------------------------------------
void apCLDeviceTypeAsString(const apCLDeviceTypeParameter& deviceType, gtString& deviceTypeAsStr)
{
    deviceTypeAsStr.makeEmpty();
    bool wasBitEncountered = false;
    cl_device_type deviceTypeEnum = deviceType.value();
    apAddCLBitfieldBitToValueString(deviceTypeEnum, CL_DEVICE_TYPE_CPU, AP_STR_CL_DEVICE_TYPE_CPU_AS_STR, deviceTypeAsStr, wasBitEncountered, false);
    apAddCLBitfieldBitToValueString(deviceTypeEnum, CL_DEVICE_TYPE_GPU, AP_STR_CL_DEVICE_TYPE_GPU_AS_STR, deviceTypeAsStr, wasBitEncountered, false);
    apAddCLBitfieldBitToValueString(deviceTypeEnum, CL_DEVICE_TYPE_ACCELERATOR, AP_STR_CL_DEVICE_TYPE_ACCELERATOR_AS_STR, deviceTypeAsStr, wasBitEncountered, false);
    apAddCLBitfieldBitToValueString(deviceTypeEnum, CL_DEVICE_TYPE_DEFAULT, AP_STR_CL_DEVICE_TYPE_DEFAULT_AS_STR, deviceTypeAsStr, wasBitEncountered, false);
    apAddCLBitfieldBitToValueString(deviceTypeEnum, CL_DEVICE_TYPE_CUSTOM, AP_STR_CL_DEVICE_TYPE_CUSTOM_AS_STR, deviceTypeAsStr, wasBitEncountered, false);

    if (!wasBitEncountered)
    {
        deviceTypeAsStr = AP_STR_CL_DEVICE_TYPE_UNKNOWN_AS_STR;
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceIndexToDisplayIndex
// Description: Takes a device index and converts it to its display index
// Author:  AMD Developer Tools Team
// Date:        28/3/2010
// ---------------------------------------------------------------------------
int apCLDeviceIndexToDisplayIndex(int deviceAPIIndex)
{
    // Devices indices are zero-based, convert the value to one-based
    return deviceAPIIndex + 1;
}

// ---------------------------------------------------------------------------
// Name:        apCLFPConfigAsString
// Description: Input an OpenCL device floating point configuration type and returns a human readable string that represents it
// Author:  AMD Developer Tools Team
// Date:        17/3/2010
// ---------------------------------------------------------------------------
void apCLFPConfigAsString(const apCLDeviceFloatingPointConfigParameter& deviceFPConfig, gtString& deviceFPConfigAsStr)
{
    deviceFPConfigAsStr.makeEmpty();
    bool wasBitEncountered = false;
    cl_device_fp_config deviceTypeEnum = deviceFPConfig.value();
    apAddCLBitfieldBitToValueString(deviceTypeEnum, CL_FP_DENORM, AP_STR_CL_FP_DENORM_AS_STR, deviceFPConfigAsStr, wasBitEncountered, false);
    apAddCLBitfieldBitToValueString(deviceTypeEnum, CL_FP_INF_NAN, AP_STR_CL_FP_INF_NAN_AS_STR, deviceFPConfigAsStr, wasBitEncountered, false);
    apAddCLBitfieldBitToValueString(deviceTypeEnum, CL_FP_ROUND_TO_NEAREST, AP_STR_CL_FP_ROUND_TO_NEAREST_AS_STR, deviceFPConfigAsStr, wasBitEncountered, false);
    apAddCLBitfieldBitToValueString(deviceTypeEnum, CL_FP_ROUND_TO_ZERO, AP_STR_CL_FP_ROUND_TO_ZERO_AS_STR, deviceFPConfigAsStr, wasBitEncountered, false);
    apAddCLBitfieldBitToValueString(deviceTypeEnum, CL_FP_ROUND_TO_INF, AP_STR_CL_FP_ROUND_TO_INF_AS_STR, deviceFPConfigAsStr, wasBitEncountered, false);
    apAddCLBitfieldBitToValueString(deviceTypeEnum, CL_FP_FMA, AP_STR_CL_FP_FMA_AS_STR, deviceFPConfigAsStr, wasBitEncountered, false);
    apAddCLBitfieldBitToValueString(deviceTypeEnum, CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT, AP_STR_CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT, deviceFPConfigAsStr, wasBitEncountered, false);
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    apAddCLBitfieldBitToValueString(deviceTypeEnum, CL_FP_SOFT_FLOAT, AP_STR_CL_FP_SOFT_FLOAT_STR, deviceFPConfigAsStr, wasBitEncountered, false);
#endif

    if (!wasBitEncountered)
    {
        deviceFPConfigAsStr = AP_STR_CL_FP_CONFIG_UNKNOWN_AS_STR;
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceQueuePropertiesAsString
// Description: Translates OpenCL device's queue properties into a human readable string.
// Author:  AMD Developer Tools Team
// Date:        14/3/2010
// ---------------------------------------------------------------------------
void apCLDeviceQueuePropertiesAsString(cl_command_queue_properties deviceQueueProperties, gtString& deviceQueuePropertiesAsStr)
{
    deviceQueuePropertiesAsStr.makeEmpty();
    bool wasBitEncountered = false;
    apAddCLBitfieldBitToValueString(deviceQueueProperties, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, AP_STR_CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE_AS_STR, deviceQueuePropertiesAsStr, wasBitEncountered, false);
    apAddCLBitfieldBitToValueString(deviceQueueProperties, CL_QUEUE_PROFILING_ENABLE, AP_STR_CL_QUEUE_PROFILING_ENABLE_AS_STR, deviceQueuePropertiesAsStr, wasBitEncountered, false);

    if (!wasBitEncountered)
    {
        deviceQueuePropertiesAsStr = AP_STR_CL_QUEUE_PROPERTIES_UNKNOWN_AS_STR;
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLDeviceSVMCapabilitiesAsString
// Description: Translates OpenCL device's SVM capabilities into a human-readable string.
// Author:  AMD Developer Tools Team
// Date:        8/12/2013
// ---------------------------------------------------------------------------
AP_API void apCLDeviceSVMCapabilitiesAsString(cl_device_svm_capabilities deviceSVMCapabilities, gtString& deviceSVMCapabilitiesAsStr)
{
    deviceSVMCapabilitiesAsStr.makeEmpty();
    bool wasBitEncountered = false;
    apAddCLBitfieldBitToValueString(deviceSVMCapabilities, CL_DEVICE_SVM_COARSE_GRAIN_BUFFER, AP_STR_CL_DEVICE_SVM_COARSE_GRAIN_BUFFER_AS_STR, deviceSVMCapabilitiesAsStr, wasBitEncountered, false);
    apAddCLBitfieldBitToValueString(deviceSVMCapabilities, CL_DEVICE_SVM_FINE_GRAIN_BUFFER, AP_STR_CL_DEVICE_SVM_FINE_GRAIN_BUFFER_AS_STR, deviceSVMCapabilitiesAsStr, wasBitEncountered, false);
    apAddCLBitfieldBitToValueString(deviceSVMCapabilities, CL_DEVICE_SVM_FINE_GRAIN_SYSTEM, AP_STR_CL_DEVICE_SVM_FINE_GRAIN_SYSTEM_AS_STR, deviceSVMCapabilitiesAsStr, wasBitEncountered, false);
    apAddCLBitfieldBitToValueString(deviceSVMCapabilities, CL_DEVICE_SVM_ATOMICS, AP_STR_CL_DEVICE_SVM_ATOMICS_AS_STR, deviceSVMCapabilitiesAsStr, wasBitEncountered, false);

    if (!wasBitEncountered)
    {
        deviceSVMCapabilitiesAsStr = AP_STR_CL_SVM_CAPABILITIES_UNKNOWN_AS_STR;
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLExecutionCapabilitiesAsString
// Description: Input an OpenCL execution capabilities type and returns a human readable string that represents it
// Arguments:   apCLExecutionCapabilities execCaps
//            gtString& execCapsAsStr
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        18/3/2010
// ---------------------------------------------------------------------------
void apCLExecutionCapabilitiesAsString(const apCLDeviceExecutionCapabilitiesParameter& execCaps, gtString& execCapsAsStr)
{
    execCapsAsStr.makeEmpty();
    bool wasBitEncountered = false;
    cl_device_exec_capabilities execCapsEnum = execCaps.value();
    apAddCLBitfieldBitToValueString(execCapsEnum, CL_EXEC_KERNEL, AP_STR_CL_EXEC_KERNEL_AS_STR, execCapsAsStr, wasBitEncountered, false);
    apAddCLBitfieldBitToValueString(execCapsEnum, CL_EXEC_NATIVE_KERNEL, AP_STR_CL_EXEC_NATIVE_KERNEL_AS_STR, execCapsAsStr, wasBitEncountered, false);

    if (!wasBitEncountered)
    {
        execCapsAsStr = AP_STR_CL_EXEC_CAPABILITIES_UNKNOWN_AS_STR;
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLMemoryCacheTypeAsString
// Description: Input an OpenCL memory cache type and returns a human readable string that represents it
// Author:  AMD Developer Tools Team
// Date:        18/3/2010
// ---------------------------------------------------------------------------
void apCLMemoryCacheTypeAsString(apCLMemoryCacheType deviceMemCacheType, gtString& deviceCacheTypeAsStr)
{
    switch (deviceMemCacheType)
    {
        case AP_CL_MEM_CACHE_NONE:
            deviceCacheTypeAsStr = AP_STR_CL_MEM_CACHE_NONE_AS_STR;
            break;

        case AP_CL_MEM_READ_ONLY_CACHE:
            deviceCacheTypeAsStr = AP_STR_CL_MEM_READ_ONLY_CACHE_AS_STR;
            break;

        case AP_CL_MEM_READ_WRITE_CACHE:
            deviceCacheTypeAsStr = AP_STR_CL_MEM_READ_WRITE_CACHE_AS_STR;
            break;

        default:
            deviceCacheTypeAsStr = AP_STR_CL_MEM_CACHE_UNKNOWN_AS_STR;
            //      GT_ASSERT(false);
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLMemoryCacheTypeAsString
// Description: Input an OpenCL local memory type and returns a human readable string that represents it
// Author:  AMD Developer Tools Team
// Date:        18/3/2010
// ---------------------------------------------------------------------------
void apCLLocalMemTypeAsString(apCLLocalMemType memType, gtString& memTypeAsStr)
{
    switch (memType)
    {
        case AP_CL_MEM_LOCAL:
            memTypeAsStr = AP_STR_CL_MEM_LOCAL_AS_STR;
            break;

        case AP_CL_MEM_GLOBAL:
            memTypeAsStr = AP_STR_CL_MEM_GLOBAL_AS_STR;
            break;

        default:
            memTypeAsStr = AP_STR_CL_LOCAL_MEM_TYPE_UNKNOWN_AS_STR;
            //      GT_ASSERT(false);
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLDevice::apCLDevice
// Description: Constructor
// Arguments:
//   APIID - The API's ID for this device.
//   deviceId - OpenGL's ID for this device.
// Author:  AMD Developer Tools Team
// Date:        29/11/2009
// ---------------------------------------------------------------------------
apCLDevice::apCLDevice(gtInt32 APIID, oaCLDeviceID deviceHandle)
    : _APIID(APIID), _deviceHandle(deviceHandle), _wasMarkedForDeletion(false), _addressBits(0), _isAvailable(false), _isCompilerAvailable(false),
      _deviceSingleFPConfig(0), _deviceDoubleFPConfig(0), _deviceHalfFPConfig(0), _isEndianLittle(false), _isDeviceErrorCorrectionSupport(false), _deviceExecutionCapabilities(0),
      _globalMemCacheSize(0), _memoryCacheType(AP_CL_MEM_UNKNOWN_CACHE), _globalMemCacheLineSize(0), _globalMemSize(0), _isHostUnifiedMemory(false), _isImageSupport(false), _localMemSize(0), _localMemType(AP_CL_MEM_UNKNOWN),
      _maxClockFrequency(0), _maxComputeUnits(0), _maxConstantArgs(0), _maxConstantBufferSize(0), _maxMemAllocSize(0), _maxParamSize(0), _maxReadImageArgs(0), _maxSamplers(0), _maxWorkgroupSize(0),
      _maxWorkItemDimensions(0), _pMaxWorkItemSizes(NULL), _maxWriteImageArgs(0), m_maxReadWriteImageArgs(0), _memBaseAddrAlign(0), _minDataTypeAlignSize(0), _platformID(OA_CL_NULL_HANDLE),
      _nativeVecWidthChar(0), _nativeVecWidthShort(0), _nativeVecWidthInt(0), _nativeVecWidthLong(0), _nativeVecWidthFloat(0), _nativeVecWidthDouble(0), _nativeVecWidthHalf(0),
      _preferredVecWidthChar(0), _preferredVecWidthShort(0), _preferredVecWidthInt(0), _preferredVecWidthLong(0), _preferredVecWidthFloat(0), _preferredVecWidthDouble(0), _preferredVecWidthHalf(0),
      _profilingTimerResolution(0), _deviceType(0), _deviceVendorID(0), _clMajorVersion(0), _clMinorVersion(0), _deviceQueueProperties(CL_QUEUE_PROFILING_ENABLE),
      m_expectOpenCL20Properties(false), m_deviceMaxGlobalVariableSize(0), m_deviceGlobalVariablePreferredTotalSize(0), m_queueOnDeviceProperties(0), m_queueOnDevicePreferredSize(0),
      m_queueOnDeviceMaxSize(0), m_deviceMaxOnDeviceQueues(0), m_deviceMaxOnDeviceEvents(0), m_deviceSVMCapabilities(0), m_deviceMaxPipeArgs(0), m_devicePipeMaxActiveReservations(0),
      m_devicePipeMaxPacketSize(0), m_devicePreferredPlatformAtomicAlignment(0), m_devicePreferredGlobalAtomicAlignment(0), m_devicePreferredLocalAtomicAlignment(0)
{
    // Initialize max dimension arrays:
    _maxImage2DDimension[0] = 0;
    _maxImage2DDimension[1] = 0;
    _maxImage3DDimension[0] = 0;
    _maxImage3DDimension[1] = 0;
    _maxImage3DDimension[2] = 0;
}


// ---------------------------------------------------------------------------
// Name:        apCLDevice::~apCLDevice
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        29/11/2009
// ---------------------------------------------------------------------------
apCLDevice::~apCLDevice()
{
    if (_pMaxWorkItemSizes != NULL)
    {
        delete [] _pMaxWorkItemSizes;
    }
}


// ---------------------------------------------------------------------------
// Name:        apCLDevice::initialize
// Description: Initializes the device properties from the system's OpenCL device.
// Arguments:   pclGetDeviceInfo - Pointer to the clGetDeviceInfo function to be used for
//                               retrieving the device data.
//              bool fullAttributesList - get the full atributes list / only a reduced list
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/3/2010
// ---------------------------------------------------------------------------
bool apCLDevice::initialize(clGetDeviceInfoProc pclGetDeviceInfo, bool fullAttributesList)
{
    (void)(fullAttributesList); // unused
    bool retVal = true;

    // Get the device version:
    bool rcDeviceVersion = deviceParamInfoAsString(pclGetDeviceInfo, CL_DEVICE_VERSION, _deviceVersion);
    retVal = retVal && rcDeviceVersion;

    // Parse the version string to check if the device supports OpenCL1.1:
    bool rcParseVersionStr = parseOpenCLVersionString();
    GT_ASSERT(rcParseVersionStr);

    // Get the address bits:
    bool rcAddressBits = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_ADDRESS_BITS, _addressBits);
    retVal = retVal && rcAddressBits;

    // Get the is available property:
    bool rcIsAvailable = deviceParamInfoAsBool(pclGetDeviceInfo, CL_DEVICE_AVAILABLE, _isAvailable);
    retVal = retVal && rcIsAvailable;

    // Get the is compiler available property:
    bool rcIsCompilerAvailable = deviceParamInfoAsBool(pclGetDeviceInfo, CL_DEVICE_COMPILER_AVAILABLE, _isCompilerAvailable);
    retVal = retVal && rcIsCompilerAvailable;

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    {
        // Get floating point configuration:
        bool rcGetDeviceFPConfig = getDeviceFPConfig(pclGetDeviceInfo, CL_DEVICE_DOUBLE_FP_CONFIG, _deviceDoubleFPConfig);
        retVal = retVal && rcGetDeviceFPConfig;
        rcGetDeviceFPConfig = getDeviceFPConfig(pclGetDeviceInfo, CL_DEVICE_SINGLE_FP_CONFIG, _deviceSingleFPConfig);
        retVal = retVal && rcGetDeviceFPConfig;
        rcGetDeviceFPConfig = getDeviceFPConfig(pclGetDeviceInfo, CL_DEVICE_HALF_FP_CONFIG, _deviceHalfFPConfig);
        retVal = retVal && rcGetDeviceFPConfig;
    }
#endif

    // Get the is endian little property:
    bool rcIsEndianLittle = deviceParamInfoAsBool(pclGetDeviceInfo, CL_DEVICE_ENDIAN_LITTLE , _isEndianLittle);
    retVal = retVal && rcIsEndianLittle;

    // Get the is error correction support property:
    bool rcIsDeviceErrorCorrectionSupport = deviceParamInfoAsBool(pclGetDeviceInfo, CL_DEVICE_ERROR_CORRECTION_SUPPORT , _isDeviceErrorCorrectionSupport);
    retVal = retVal && rcIsDeviceErrorCorrectionSupport;

    // Get device execution capabilities:
    bool rcGetExecutionCapabilities = getDeviceExecutionCapabilities(pclGetDeviceInfo);
    retVal = retVal && rcGetExecutionCapabilities;

    // Get the global memory cache size:
    bool rcCacheSize = deviceParamInfoAsULong(pclGetDeviceInfo, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, _globalMemCacheSize);
    retVal = retVal && rcCacheSize;

    // Get the global memory cache line size:
    bool rcCacheLineSize = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, _globalMemCacheLineSize);
    retVal = retVal && rcCacheLineSize;

    // Get the memory cache type:
    bool rcGetMemCacheType = getDeviceMemoryCacheType(pclGetDeviceInfo);
    retVal = retVal && rcGetMemCacheType;

    // Get the global memory size:
    bool rcMemSize = deviceParamInfoAsULong(pclGetDeviceInfo, CL_DEVICE_GLOBAL_MEM_SIZE, _globalMemSize);
    retVal = retVal && rcMemSize;

    // Get the OpenCL 1.1 properties:
    if (((_clMajorVersion == 1) && (_clMinorVersion >= 1)) || (_clMajorVersion > 1))
    {
        // Get the is host unified property:
        bool rcHostUnifiedMemory = deviceParamInfoAsBool(pclGetDeviceInfo, CL_DEVICE_HOST_UNIFIED_MEMORY, _isHostUnifiedMemory);
        retVal = retVal && rcHostUnifiedMemory;

        bool rcGetCVersion = deviceParamInfoAsString(pclGetDeviceInfo, CL_DEVICE_OPENCL_C_VERSION, _openCLCVersion);
        retVal = retVal && rcGetCVersion;

        // Get the native vector width size for built-in scalar types:
        bool rcGetNativeVecWidth = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, _nativeVecWidthChar);
        retVal = retVal && rcGetNativeVecWidth;
        rcGetNativeVecWidth = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, _nativeVecWidthShort);
        retVal = retVal && rcGetNativeVecWidth;
        rcGetNativeVecWidth = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, _nativeVecWidthInt);
        retVal = retVal && rcGetNativeVecWidth;
        rcGetNativeVecWidth = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, _nativeVecWidthLong);
        retVal = retVal && rcGetNativeVecWidth;
        rcGetNativeVecWidth = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, _nativeVecWidthFloat);
        retVal = retVal && rcGetNativeVecWidth;
        rcGetNativeVecWidth = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, _nativeVecWidthDouble);
        retVal = retVal && rcGetNativeVecWidth;
        rcGetNativeVecWidth = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, _nativeVecWidthHalf);
        retVal = retVal && rcGetNativeVecWidth;

        bool rcGetPreferredVecWidth = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, _preferredVecWidthHalf);
        retVal = retVal && rcGetPreferredVecWidth;
    }

    // Get the image support property:
    bool rcImageSupport = deviceParamInfoAsBool(pclGetDeviceInfo, CL_DEVICE_IMAGE_SUPPORT, _isImageSupport);
    retVal = retVal && rcImageSupport;

    // Get the image max 2D dimensions values:
    size_t paramValue = 0;
    bool rcImageMaxDims = deviceParamInfoAsSizeT(pclGetDeviceInfo, CL_DEVICE_IMAGE2D_MAX_WIDTH, paramValue);

    if (rcImageMaxDims)
    {
        _maxImage2DDimension[0] = gtUInt32(paramValue);
    }
    else
    {
        retVal = false;
    }

    rcImageMaxDims = deviceParamInfoAsSizeT(pclGetDeviceInfo, CL_DEVICE_IMAGE2D_MAX_HEIGHT, paramValue);

    if (rcImageMaxDims)
    {
        _maxImage2DDimension[1] = gtUInt32(paramValue);
    }
    else
    {
        retVal = false;
    }


    // Get the image max 3D dimensions values:
    rcImageMaxDims = deviceParamInfoAsSizeT(pclGetDeviceInfo, CL_DEVICE_IMAGE3D_MAX_WIDTH, paramValue);

    if (rcImageMaxDims)
    {
        _maxImage3DDimension[0] = gtUInt32(paramValue);
    }
    else
    {
        retVal = false;
    }

    rcImageMaxDims = deviceParamInfoAsSizeT(pclGetDeviceInfo, CL_DEVICE_IMAGE3D_MAX_HEIGHT, paramValue);

    if (rcImageMaxDims)
    {
        _maxImage3DDimension[1] = gtUInt32(paramValue);
    }
    else
    {
        retVal = false;
    }

    rcImageMaxDims = deviceParamInfoAsSizeT(pclGetDeviceInfo, CL_DEVICE_IMAGE3D_MAX_DEPTH, paramValue);

    if (rcImageMaxDims)
    {
        _maxImage3DDimension[2] = gtUInt32(paramValue);
    }
    else
    {
        retVal = false;
    }

    // Get the local memory size:
    bool rcLocalMemSize = deviceParamInfoAsULong(pclGetDeviceInfo, CL_DEVICE_LOCAL_MEM_SIZE, _localMemSize);
    retVal = retVal && rcLocalMemSize;

    // Get local memory type:
    bool rcGetLocalMemType = getDeviceLocalMemType(pclGetDeviceInfo);
    retVal = retVal && rcGetLocalMemType;

    // Get the maximal clock frequency:
    bool rcMaxClockFrequency = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_MAX_CLOCK_FREQUENCY, _maxClockFrequency);
    retVal = retVal && rcMaxClockFrequency;

    // Get the maximal compute units:
    bool rcMaxComputeUnits = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_MAX_COMPUTE_UNITS, _maxComputeUnits);
    retVal = retVal && rcMaxComputeUnits;

    // Get the maximal constant arguments:
    bool rcMaxCostantArgs = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_MAX_CONSTANT_ARGS, _maxConstantArgs);
    retVal = retVal && rcMaxCostantArgs;

    // Get the maximal constant buffer size:
    bool rcMaxCostantBufferSize = deviceParamInfoAsULong(pclGetDeviceInfo, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, _maxConstantBufferSize);
    retVal = retVal && rcMaxCostantBufferSize;

    // Get the maximal memory allocation size:
    bool rcMaxMemAllocSize = deviceParamInfoAsULong(pclGetDeviceInfo, CL_DEVICE_MAX_MEM_ALLOC_SIZE, _maxMemAllocSize);
    retVal = retVal && rcMaxMemAllocSize;

    // Get the maximal parameter size:
    size_t maxParamSizeAsSizet = 0;
    bool rcMaxParamSize = deviceParamInfoAsSizeT(pclGetDeviceInfo, CL_DEVICE_MAX_PARAMETER_SIZE , maxParamSizeAsSizet);
    _maxParamSize = gtUInt32(maxParamSizeAsSizet);
    retVal = retVal && rcMaxParamSize;

    // Get the maximal read image arguments:
    bool rcMaxReadImageArgs = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_MAX_READ_IMAGE_ARGS, _maxReadImageArgs);
    retVal = retVal && rcMaxReadImageArgs;

    // Get the maximal samplers:
    bool rcMaxSamplers = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_MAX_SAMPLERS, _maxSamplers);
    retVal = retVal && rcMaxSamplers;

    // Get the maximal group size:
    size_t maxGroupSizeAsSizet = 0;
    bool rcMaxGroupSize = deviceParamInfoAsSizeT(pclGetDeviceInfo, CL_DEVICE_MAX_WORK_GROUP_SIZE, maxGroupSizeAsSizet);
    _maxWorkgroupSize = gtUInt32(maxGroupSizeAsSizet);
    retVal = retVal && rcMaxGroupSize;

    // Get the maximal work item dimensions:
    bool rcMaxWorkItemDimensions = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, _maxWorkItemDimensions);
    retVal = retVal && rcMaxWorkItemDimensions;

    // Get the values for the maximal work item sizes:
    size_t* pMaxWorkItemSizesAsSizet = NULL;
    bool rcMaxWorkItemSizes = deviceParamInfoAsSizeTPtr(pclGetDeviceInfo, CL_DEVICE_MAX_WORK_ITEM_SIZES, pMaxWorkItemSizesAsSizet, _maxWorkItemDimensions);

    if (rcMaxWorkItemSizes)
    {
        delete _pMaxWorkItemSizes;
        _pMaxWorkItemSizes = new gtUInt32[_maxWorkItemDimensions];
        GT_IF_WITH_ASSERT(_pMaxWorkItemSizes != NULL)
        {
            for (gtUInt32 i = 0; i < _maxWorkItemDimensions; i++)
            {
                _pMaxWorkItemSizes[i] = gtUInt32(pMaxWorkItemSizesAsSizet[i]);
            }
        }

        delete[] pMaxWorkItemSizesAsSizet;
        pMaxWorkItemSizesAsSizet = NULL;
    }
    else
    {
        retVal = false;
    }

    // Max read-write images replaced max write images in OpenCL 2.0, so we only require one of them:
    bool rcMaxWriteImageArgs = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, _maxWriteImageArgs);
    bool rcMaxReadWriteImageArgs = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS, m_maxReadWriteImageArgs);
    retVal = retVal && (rcMaxWriteImageArgs || rcMaxReadWriteImageArgs);

    // Get the memory base address alignment:
    bool rcMemBaseAddrAlign = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_MEM_BASE_ADDR_ALIGN, _memBaseAddrAlign);
    retVal = retVal && rcMemBaseAddrAlign;

    // Get the minimal data type alignment size:
    bool rcMinDataTypeAlignSize = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, _minDataTypeAlignSize);
    retVal = retVal && rcMinDataTypeAlignSize;

    // Get the device name:
    bool rcDeviceName = deviceParamInfoAsString(pclGetDeviceInfo, CL_DEVICE_NAME, _deviceName);
    // Remove leading and trailing whitespace characters that sometimes appear in Intel CPU device names. See BUG421921.
    _deviceNameForDisplay = _deviceName;
    _deviceNameForDisplay.trim();

    retVal = retVal && rcDeviceName;

    // Get the device platform id:
    bool rcGetPlatformID = deviceParamInfoAsPlatformID(pclGetDeviceInfo, CL_DEVICE_PLATFORM, _platformID);
    retVal = retVal && rcGetPlatformID;

    // Get the preferred native vector width size for built-in scalar types:
    bool rcGetPreferredVecWidth = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, _preferredVecWidthChar);
    retVal = retVal && rcGetPreferredVecWidth;
    rcGetPreferredVecWidth = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, _preferredVecWidthShort);
    retVal = retVal && rcGetPreferredVecWidth;
    rcGetPreferredVecWidth = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, _preferredVecWidthInt);
    retVal = retVal && rcGetPreferredVecWidth;
    rcGetPreferredVecWidth = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, _preferredVecWidthLong);
    retVal = retVal && rcGetPreferredVecWidth;
    rcGetPreferredVecWidth = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, _preferredVecWidthFloat);
    retVal = retVal && rcGetPreferredVecWidth;
    rcGetPreferredVecWidth = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, _preferredVecWidthDouble);
    retVal = retVal && rcGetPreferredVecWidth;

    // Get the device profile:
    bool rcGetProfile = deviceParamInfoAsString(pclGetDeviceInfo, CL_DEVICE_PROFILE, _profileStr);
    retVal = retVal && rcGetProfile;

    // Get the device profiling timer resolution:
    size_t profilingTimerResolutionAsSizet = 0;
    bool rcGetProfilingTimerResolution = deviceParamInfoAsSizeT(pclGetDeviceInfo, CL_DEVICE_PROFILING_TIMER_RESOLUTION, profilingTimerResolutionAsSizet);

    if (rcGetProfilingTimerResolution)
    {
        _profilingTimerResolution = gtUInt64(profilingTimerResolutionAsSizet);
    }
    else
    {
        retVal = false;
    }

    // Get the device vendor:
    bool rcGetDeviceVendor = deviceParamInfoAsString(pclGetDeviceInfo, CL_DEVICE_VENDOR, _deviceVendor);
    retVal = retVal && rcGetDeviceVendor;

    // Get the device vendor ID:
    bool rcGetDeviceVendorID = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_VENDOR_ID, _deviceVendorID);
    retVal = retVal && rcGetDeviceVendorID;

    // Get the driver version:
    bool rcDriverVersion = deviceParamInfoAsString(pclGetDeviceInfo, CL_DRIVER_VERSION, _driverVersion);
    retVal = retVal && rcDriverVersion;

    bool rcGetDeviceType = getDeviceType(pclGetDeviceInfo);
    retVal = retVal && rcGetDeviceType;

    bool rcGetQueueProps = getCommandQueueProperties(pclGetDeviceInfo, CL_DEVICE_QUEUE_PROPERTIES, _deviceQueueProperties);
    retVal = retVal && rcGetQueueProps;

    bool rcOpenCL20 = true;

    // Get program global variable details:
    gtSize_t maxGlobalVarSzAsSizeT = 0;
    bool rcMaxGlobalVarSz = deviceParamInfoAsSizeT(pclGetDeviceInfo, CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE, maxGlobalVarSzAsSizeT);
    rcOpenCL20 = rcOpenCL20 && rcMaxGlobalVarSz;

    if (rcMaxGlobalVarSz)
    {
        m_deviceMaxGlobalVariableSize = (gtUInt64)maxGlobalVarSzAsSizeT;
    }

    gtSize_t globalVarPreferredTotSzAsSizeT = 0;
    bool rcPrefGlobalVarTotalSz = deviceParamInfoAsSizeT(pclGetDeviceInfo, CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE, globalVarPreferredTotSzAsSizeT);
    rcOpenCL20 = rcOpenCL20 && rcPrefGlobalVarTotalSz;

    if (rcPrefGlobalVarTotalSz)
    {
        m_deviceGlobalVariablePreferredTotalSize = (gtUInt64)globalVarPreferredTotSzAsSizeT;
    }

    // Get the queue on-device properties:
    bool rcQueueOnDeviceProperties = getCommandQueueProperties(pclGetDeviceInfo, CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES, m_queueOnDeviceProperties);
    rcOpenCL20 = rcOpenCL20 && rcQueueOnDeviceProperties;

    bool rcQODPrefSz = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE, m_queueOnDevicePreferredSize);
    rcOpenCL20 = rcOpenCL20 && rcQODPrefSz;

    bool rcQODMaxSz = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE, m_queueOnDeviceMaxSize);
    rcOpenCL20 = rcOpenCL20 && rcQODMaxSz;

    bool rcMaxQOD = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_MAX_ON_DEVICE_QUEUES, m_deviceMaxOnDeviceQueues);
    rcOpenCL20 = rcOpenCL20 && rcMaxQOD;

    bool rcMaxQODEvents = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_MAX_ON_DEVICE_EVENTS, m_deviceMaxOnDeviceEvents);
    rcOpenCL20 = rcOpenCL20 && rcMaxQODEvents;

    // Get the SVM capabilities
    bool rcGetSVMCap = getSVMCapabilities(pclGetDeviceInfo);
    rcOpenCL20 = rcOpenCL20 && rcGetSVMCap;

    // Get the pipe properties:
    bool rcMaxPipeArgs = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_MAX_PIPE_ARGS, m_deviceMaxPipeArgs);
    rcOpenCL20 = rcOpenCL20 && rcMaxPipeArgs;

    bool rcMaxPipeActiveReserv = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS, m_devicePipeMaxActiveReservations);
    rcOpenCL20 = rcOpenCL20 && rcMaxPipeActiveReserv;

    bool rcMaxPipePacketSz = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_PIPE_MAX_PACKET_SIZE, m_devicePipeMaxPacketSize);
    rcOpenCL20 = rcOpenCL20 && rcMaxPipePacketSz;

    // Get the preferred atomic alignments:
    bool rcPrefPlatAtomicAlign = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT, m_devicePreferredPlatformAtomicAlignment);
    rcOpenCL20 = rcOpenCL20 && rcPrefPlatAtomicAlign;

    bool rcPrefGlobAtomicAlign = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT, m_devicePreferredGlobalAtomicAlignment);
    rcOpenCL20 = rcOpenCL20 && rcPrefGlobAtomicAlign;

    bool rcPrefLocAtomicAlign = deviceParamInfoAsUInt(pclGetDeviceInfo, CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT, m_devicePreferredLocalAtomicAlignment);
    rcOpenCL20 = rcOpenCL20 && rcPrefLocAtomicAlign;

    // If we expect OpenCL 2.0, add it to the retVal:
    if (m_expectOpenCL20Properties)
    {
        retVal = retVal && rcOpenCL20;
    }

    bool rcGetExtensions = getDeviceExtensions(pclGetDeviceInfo);
    retVal = retVal && rcGetExtensions;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLDevice::type
// Description: Returns this transferable object type: OS_TOBJ_ID_CL_DEVICE.
// Author:  AMD Developer Tools Team
// Date:        29/11/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLDevice::type() const
{
    return OS_TOBJ_ID_CL_DEVICE;
}


// ---------------------------------------------------------------------------
// Name:        apCLDevice::writeSelfIntoChannel
// Description: Writes this object's data into an IPC channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/11/2009
// ---------------------------------------------------------------------------
bool apCLDevice::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _APIID;
    ipcChannel << (gtUInt64)_deviceHandle;
    ipcChannel << _wasMarkedForDeletion;
    ipcChannel << _addressBits;

    ipcChannel << _isAvailable;
    ipcChannel << _isCompilerAvailable;

    bool rcSing = _deviceSingleFPConfig.writeSelfIntoChannel(ipcChannel);
    bool rcDoub = _deviceDoubleFPConfig.writeSelfIntoChannel(ipcChannel);
    bool rcHalf = _deviceHalfFPConfig.writeSelfIntoChannel(ipcChannel);

    ipcChannel << _isEndianLittle;
    ipcChannel << _isDeviceErrorCorrectionSupport;
    bool rcExecCap = _deviceExecutionCapabilities.writeSelfIntoChannel(ipcChannel);

    ipcChannel << _globalMemCacheSize;
    ipcChannel << (gtInt32) _memoryCacheType;

    ipcChannel << _globalMemCacheLineSize;
    ipcChannel << _globalMemSize;

    ipcChannel << _isHostUnifiedMemory;
    ipcChannel << _isImageSupport;
    ipcChannel << _maxImage2DDimension[0];
    ipcChannel << _maxImage2DDimension[1];
    ipcChannel << _maxImage3DDimension[0];
    ipcChannel << _maxImage3DDimension[1];
    ipcChannel << _maxImage3DDimension[2];

    ipcChannel << _localMemSize;

    ipcChannel << (gtInt32) _localMemType;
    ipcChannel << _maxClockFrequency;
    ipcChannel << _maxComputeUnits;
    ipcChannel << _maxConstantArgs;
    ipcChannel << _maxConstantBufferSize;
    ipcChannel << _maxMemAllocSize;
    ipcChannel << _maxParamSize;
    ipcChannel << _maxReadImageArgs;
    ipcChannel << _maxSamplers;
    ipcChannel << _maxWorkgroupSize;
    ipcChannel << _maxWorkItemDimensions;

    // Maximal work item sizes:
    for (size_t i = 0; i < _maxWorkItemDimensions; i++)
    {
        ipcChannel << _pMaxWorkItemSizes[i];
    }

    ipcChannel << _maxWriteImageArgs;
    ipcChannel << m_maxReadWriteImageArgs;
    ipcChannel << _memBaseAddrAlign;
    ipcChannel << _minDataTypeAlignSize;
    ipcChannel << _deviceName;
    ipcChannel << _deviceNameForDisplay;
    ipcChannel << _openCLCVersion;

    ipcChannel << (gtUInt64) _platformID;

    ipcChannel << _nativeVecWidthChar;
    ipcChannel << _nativeVecWidthShort;
    ipcChannel << _nativeVecWidthInt;
    ipcChannel << _nativeVecWidthLong;
    ipcChannel << _nativeVecWidthFloat;
    ipcChannel << _nativeVecWidthDouble;
    ipcChannel << _nativeVecWidthHalf;

    ipcChannel << _preferredVecWidthChar;
    ipcChannel << _preferredVecWidthShort;
    ipcChannel << _preferredVecWidthInt;
    ipcChannel << _preferredVecWidthLong;
    ipcChannel << _preferredVecWidthFloat;
    ipcChannel << _preferredVecWidthDouble;
    ipcChannel << _preferredVecWidthHalf;

    ipcChannel <<  _profileStr;
    ipcChannel << _profilingTimerResolution;
    bool rcTyp = _deviceType.writeSelfIntoChannel(ipcChannel);
    ipcChannel << _deviceVendor;
    ipcChannel << _deviceVendorID;
    ipcChannel << _deviceVersion;
    ipcChannel << _clMajorVersion;
    ipcChannel << _clMinorVersion;
    ipcChannel << _driverVersion;

    ipcChannel << (gtUInt32)_deviceQueueProperties;

    ipcChannel << m_expectOpenCL20Properties;
    ipcChannel << m_deviceMaxGlobalVariableSize;
    ipcChannel << m_deviceGlobalVariablePreferredTotalSize;
    ipcChannel << (gtUInt32)m_queueOnDeviceProperties;
    ipcChannel << m_queueOnDevicePreferredSize;
    ipcChannel << m_queueOnDeviceMaxSize;
    ipcChannel << m_deviceMaxOnDeviceQueues;
    ipcChannel << m_deviceMaxOnDeviceEvents;
    ipcChannel << (gtUInt32)m_deviceSVMCapabilities;
    ipcChannel << m_deviceMaxPipeArgs;
    ipcChannel << m_devicePipeMaxActiveReservations;
    ipcChannel << m_devicePipeMaxPacketSize;
    ipcChannel << m_devicePreferredPlatformAtomicAlignment;
    ipcChannel << m_devicePreferredGlobalAtomicAlignment;
    ipcChannel << m_devicePreferredLocalAtomicAlignment;

    bool retVal = rcSing && rcDoub && rcHalf && rcExecCap && rcTyp;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLDevice::readSelfFromChannel
// Description: Reads this object's data from an IPC channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/11/2009
// ---------------------------------------------------------------------------
bool apCLDevice::readSelfFromChannel(osChannel& ipcChannel)
{

    ipcChannel >> _APIID;

    // Read the device id:
    gtUInt64 deviceHandleAsUInt64 = 0;
    ipcChannel >> deviceHandleAsUInt64;
    _deviceHandle = (oaCLDeviceID)deviceHandleAsUInt64;
    ipcChannel >> _wasMarkedForDeletion;

    ipcChannel >> _addressBits;
    ipcChannel >> _isAvailable;
    ipcChannel >> _isCompilerAvailable;

    bool rcSing = _deviceSingleFPConfig.readSelfFromChannel(ipcChannel);
    bool rcDoub = _deviceDoubleFPConfig.readSelfFromChannel(ipcChannel);
    bool rcHalf = _deviceHalfFPConfig.readSelfFromChannel(ipcChannel);

    ipcChannel >> _isEndianLittle;
    ipcChannel >> _isDeviceErrorCorrectionSupport;

    bool rcExecCap = _deviceExecutionCapabilities.readSelfFromChannel(ipcChannel);

    ipcChannel >> _globalMemCacheSize;

    gtUInt32 memoryCacheTypeAsUInt32 = 0;
    ipcChannel >> memoryCacheTypeAsUInt32;
    _memoryCacheType = (apCLMemoryCacheType)memoryCacheTypeAsUInt32;

    ipcChannel >> _globalMemCacheLineSize;
    ipcChannel >> _globalMemSize;

    ipcChannel >> _isHostUnifiedMemory;
    ipcChannel >> _isImageSupport;
    ipcChannel >> _maxImage2DDimension[0];
    ipcChannel >> _maxImage2DDimension[1];
    ipcChannel >> _maxImage3DDimension[0];
    ipcChannel >> _maxImage3DDimension[1];
    ipcChannel >> _maxImage3DDimension[2];

    ipcChannel >> _localMemSize;

    gtUInt32 localMemTypeAsUInt32 = 0;
    ipcChannel >> localMemTypeAsUInt32;
    _localMemType = (apCLLocalMemType)localMemTypeAsUInt32;

    ipcChannel >> _maxClockFrequency;
    ipcChannel >> _maxComputeUnits;
    ipcChannel >> _maxConstantArgs;
    ipcChannel >> _maxConstantBufferSize;
    ipcChannel >> _maxMemAllocSize;
    ipcChannel >> _maxParamSize;
    ipcChannel >> _maxReadImageArgs;
    ipcChannel >> _maxSamplers;
    ipcChannel >> _maxWorkgroupSize;
    ipcChannel >> _maxWorkItemDimensions;

    // Maximal work item sizes:
    _pMaxWorkItemSizes = new gtUInt32[_maxWorkItemDimensions];


    for (size_t i = 0; i < _maxWorkItemDimensions; i++)
    {
        ipcChannel >> _pMaxWorkItemSizes[i];
    }

    ipcChannel >> _maxWriteImageArgs;
    ipcChannel >> m_maxReadWriteImageArgs;
    ipcChannel >> _memBaseAddrAlign;
    ipcChannel >> _minDataTypeAlignSize;
    ipcChannel >> _deviceName;
    ipcChannel >> _deviceNameForDisplay;
    ipcChannel >> _openCLCVersion;

    gtUInt64 platformIDAsUInt64 = 0;
    ipcChannel >> platformIDAsUInt64;
    _platformID = (oaCLPlatformID)platformIDAsUInt64;

    ipcChannel >> _nativeVecWidthChar;
    ipcChannel >> _nativeVecWidthShort;
    ipcChannel >> _nativeVecWidthInt;
    ipcChannel >> _nativeVecWidthLong;
    ipcChannel >> _nativeVecWidthFloat;
    ipcChannel >> _nativeVecWidthDouble;
    ipcChannel >> _nativeVecWidthHalf;

    ipcChannel >> _preferredVecWidthChar;
    ipcChannel >> _preferredVecWidthShort;
    ipcChannel >> _preferredVecWidthInt;
    ipcChannel >> _preferredVecWidthLong;
    ipcChannel >> _preferredVecWidthFloat;
    ipcChannel >> _preferredVecWidthDouble;
    ipcChannel >> _preferredVecWidthHalf;

    ipcChannel >>  _profileStr;
    ipcChannel >> _profilingTimerResolution;

    bool rcTyp = _deviceType.readSelfFromChannel(ipcChannel);

    ipcChannel >> _deviceVendor;
    ipcChannel >> _deviceVendorID;
    ipcChannel >> _deviceVersion;
    ipcChannel >> _clMajorVersion;
    ipcChannel >> _clMinorVersion;
    ipcChannel >> _driverVersion;

    gtUInt32 deviceAllowedQueuePropertiesAsUInt32 = 0;
    ipcChannel >> deviceAllowedQueuePropertiesAsUInt32;
    _deviceQueueProperties = (cl_command_queue_properties)deviceAllowedQueuePropertiesAsUInt32;

    ipcChannel >> m_expectOpenCL20Properties;

    ipcChannel >> m_deviceMaxGlobalVariableSize;
    ipcChannel >> m_deviceGlobalVariablePreferredTotalSize;

    gtUInt32 deviceOnDeviceQueuePropertiesAsUInt32 = 0;
    ipcChannel >> deviceOnDeviceQueuePropertiesAsUInt32;
    m_queueOnDeviceProperties = (cl_command_queue_properties)deviceOnDeviceQueuePropertiesAsUInt32;

    ipcChannel >> m_queueOnDevicePreferredSize;
    ipcChannel >> m_queueOnDeviceMaxSize;
    ipcChannel >> m_deviceMaxOnDeviceQueues;
    ipcChannel >> m_deviceMaxOnDeviceEvents;

    gtUInt32 deviceSVMCapabilitesAsUInt32 = 0;
    ipcChannel >> deviceSVMCapabilitesAsUInt32;
    m_deviceSVMCapabilities = (cl_device_svm_capabilities)deviceSVMCapabilitesAsUInt32;

    ipcChannel >> m_deviceMaxPipeArgs;
    ipcChannel >> m_devicePipeMaxActiveReservations;
    ipcChannel >> m_devicePipeMaxPacketSize;
    ipcChannel >> m_devicePreferredPlatformAtomicAlignment;
    ipcChannel >> m_devicePreferredGlobalAtomicAlignment;
    ipcChannel >> m_devicePreferredLocalAtomicAlignment;

    bool retVal = rcSing && rcDoub && rcHalf && rcExecCap && rcTyp;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLDevice::deviceParamInfoAsString
// Description: The function retrieve single parameter name from the device as string
// Arguments:   clGetDeviceInfoProc pclGetDeviceInfo - a pointer to clGetDeviceInfo function
//            cl_device_info param_name - the parameter name
//            gtString& paramValueAsStr - the parameter value as string
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/3/2010
// ---------------------------------------------------------------------------
bool apCLDevice::deviceParamInfoAsString(clGetDeviceInfoProc pclGetDeviceInfo, cl_device_info param_name, gtString& paramValueAsStr)
{
    bool retVal = false;

    // Get the parameter value:
    gtSizeType stringLen = 0;
    cl_int clRetVal = pclGetDeviceInfo(cl_device_id(_deviceHandle), param_name, 0, NULL, &stringLen);
    GT_IF_WITH_ASSERT((clRetVal == CL_SUCCESS) && (stringLen > 0))
    {
        char* pDeviceParamValue = new char[stringLen + 1];
        clRetVal = pclGetDeviceInfo(cl_device_id(_deviceHandle), param_name, stringLen + 1, pDeviceParamValue, NULL);
        GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
        {
            GT_IF_WITH_ASSERT(pDeviceParamValue != NULL)
            {
                paramValueAsStr.fromASCIIString(pDeviceParamValue);
                retVal = true;
            }
        }

        delete[] pDeviceParamValue;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLDevice::deviceParamInfoAsUInt
// Description: The function retrieve single parameter name from the device as unsigned int
// Arguments:   clGetDeviceInfoProc pclGetDeviceInfo - a pointer to clGetDeviceInfo function
//              cl_device_info param_name - the parameter name
//              gtUInt32& paramValueAsUInt - the parameter value as unsigned int
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/3/2010
// ---------------------------------------------------------------------------
bool apCLDevice::deviceParamInfoAsUInt(clGetDeviceInfoProc pclGetDeviceInfo, cl_device_info param_name, gtUInt32& paramValueAsUInt)
{
    bool retVal = false;

    // Get the parameter value:
    cl_uint paramValue = 0;
    cl_int clRetVal = pclGetDeviceInfo(cl_device_id(_deviceHandle), param_name, sizeof(cl_uint), &paramValue, NULL);

    // Use if instead of GT_IF_WITH_ASSERT because the log and
    // assertion handling mechanism are not initialized yet at this point
    if (clRetVal == CL_SUCCESS)
    {
        paramValueAsUInt = (gtUInt32)paramValue;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLDevice::deviceParamInfoAsPlatformID
// Description: The function retrieve single parameter name from the device as platform ID
// Arguments:   clGetDeviceInfoProc pclGetDeviceInfo - a pointer to clGetDeviceInfo function
//              cl_device_info param_name - the parameter name
//              gtUInt32& paramValueAsUInt - the parameter value as unsigned int
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/3/2010
// ---------------------------------------------------------------------------
bool apCLDevice::deviceParamInfoAsPlatformID(clGetDeviceInfoProc pclGetDeviceInfo, cl_device_info param_name, oaCLPlatformID& paramValueAsPlatformID)
{
    bool retVal = false;

    // Get the parameter value:
    cl_int clRetVal = pclGetDeviceInfo(cl_device_id(_deviceHandle), param_name, sizeof(cl_platform_id), &paramValueAsPlatformID, NULL);
    GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLDevice::deviceParamInfoAsULong
// Description: The function retrieve single parameter name from the device as string
// Arguments:   clGetDeviceInfoProc pclGetDeviceInfo - a pointer to clGetDeviceInfo function
//              cl_device_info param_name - the parameter name
//              gtUInt64& paramValueAsUInt - the parameter value as unsigned long
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/3/2010
// ---------------------------------------------------------------------------
bool apCLDevice::deviceParamInfoAsULong(clGetDeviceInfoProc pclGetDeviceInfo, cl_device_info param_name, gtUInt64& paramValueAsULong)
{
    bool retVal = false;

    // Get the parameter value:
    cl_ulong paramValue = 0;
    cl_int clRetVal = pclGetDeviceInfo(cl_device_id(_deviceHandle), param_name, sizeof(cl_ulong), &paramValue, NULL);
    GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
    {
        paramValueAsULong = (gtUInt64)paramValue;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLDevice::deviceParamInfoAsSizeT
// Description: The function retrieve single parameter name from the device as size t
// Arguments:   clGetDeviceInfoProc pclGetDeviceInfo - a pointer to clGetDeviceInfo function
//              cl_device_info param_name - the parameter name
//              size_t& paramValueAsSizeT - the parameter value as unsigned long
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/3/2010
// ---------------------------------------------------------------------------
bool apCLDevice::deviceParamInfoAsSizeT(clGetDeviceInfoProc pclGetDeviceInfo, cl_device_info param_name, size_t& paramValueAsSizeT)
{
    bool retVal = false;

    // Get the parameter value:
    cl_int clRetVal = pclGetDeviceInfo(cl_device_id(_deviceHandle), param_name, sizeof(size_t), &paramValueAsSizeT, NULL);

    // Use if instead of GT_IF_WITH_ASSERT because the log and
    // assertion handling mechanism are not initialized yet at this point
    if (clRetVal == CL_SUCCESS)
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLDevice::deviceParamInfoAsSizeTPtr
// Description: The function retrieve single parameter name from the device as size t
// Arguments:   clGetDeviceInfoProc pclGetDeviceInfo - a pointer to clGetDeviceInfo function
//              cl_device_info param_name - the parameter name
//              size_t& paramValueAsSizeT - the parameter value as unsigned long
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/3/2010
// ---------------------------------------------------------------------------
bool apCLDevice::deviceParamInfoAsSizeTPtr(clGetDeviceInfoProc pclGetDeviceInfo, cl_device_info param_name, size_t*& pParamValueAsSizeTPtr, int pointerSize)
{
    bool retVal = false;

    // Allocate space for the pointer size:
    delete[] pParamValueAsSizeTPtr;
    pParamValueAsSizeTPtr = new size_t[pointerSize];


    // Get the parameter value:
    cl_int clRetVal = pclGetDeviceInfo(cl_device_id(_deviceHandle), param_name, pointerSize * sizeof(size_t), pParamValueAsSizeTPtr, NULL);
    GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLDevice::deviceParamInfoAsBool
// Description: The function retrieve single parameter name from the device as string
// Arguments:   clGetDeviceInfoProc pclGetDeviceInfo - a pointer to clGetDeviceInfo function
//              cl_device_info param_name - the parameter name
//              bool& paramValueAsBool - the parameter value as boolean
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/3/2010
// ---------------------------------------------------------------------------
bool apCLDevice::deviceParamInfoAsBool(clGetDeviceInfoProc pclGetDeviceInfo, cl_device_info param_name, bool& paramValueAsBool)
{
    bool retVal = false;

    // Get the parameter value:
    cl_bool paramValue = 0;
    cl_int clRetVal = pclGetDeviceInfo(cl_device_id(_deviceHandle), param_name, sizeof(cl_bool), &paramValue, NULL);
    GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
    {
        paramValueAsBool = (paramValue == CL_TRUE);
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLDevice::getDeviceType
// Description: Get the device type from OpenCL, and set the object property
// Arguments:   clGetDeviceInfoProc pclGetDeviceInfo
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/3/2010
// ---------------------------------------------------------------------------
bool apCLDevice::getDeviceType(clGetDeviceInfoProc pclGetDeviceInfo)
{
    bool retVal = false;

    // Get the device type:
    cl_device_type deviceType = CL_DEVICE_TYPE_DEFAULT;
    cl_int clRetVal = pclGetDeviceInfo(cl_device_id(_deviceHandle), CL_DEVICE_TYPE, sizeof(cl_device_type), &deviceType, NULL);
    GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
    {
        _deviceType.setValue(deviceType);
        retVal = true;
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLDevice::getCommandQueueProperties
// Description: Get the command queue properties
// Arguments:   clGetDeviceInfoProc pclGetDeviceInfo
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/3/2010
// ---------------------------------------------------------------------------
bool apCLDevice::getCommandQueueProperties(clGetDeviceInfoProc pclGetDeviceInfo, cl_device_info param_name, cl_command_queue_properties& queueProps)
{
    bool retVal = false;

    // Get the command-queue properties supported by the device:
    cl_command_queue_properties allowedQueueProperties = 0;
    cl_int rcQueue = pclGetDeviceInfo(cl_device_id(_deviceHandle), param_name, sizeof(cl_command_queue_properties), &allowedQueueProperties, NULL);

    if (rcQueue == CL_SUCCESS)
    {
        queueProps = allowedQueueProperties;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLDevice::getSVMCapabilities
// Description: Get the SVM capabilities
// Author:  AMD Developer Tools Team
// Date:        8/12/2013
// ---------------------------------------------------------------------------
bool apCLDevice::getSVMCapabilities(clGetDeviceInfoProc pclGetDeviceInfo)
{
    bool retVal = false;

    // Get the command-queue properties supported by the device:
    cl_device_svm_capabilities svmCapabilities = 0;
    cl_int rcSVM = pclGetDeviceInfo((cl_device_id)_deviceHandle, CL_DEVICE_SVM_CAPABILITIES, sizeof(cl_device_svm_capabilities), &svmCapabilities, NULL);

    if (rcSVM == CL_SUCCESS)
    {
        m_deviceSVMCapabilities = svmCapabilities;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLDevice::getDeviceFPConfig
// Description: Get & Set the object's double FP config property
// Arguments:   clGetDeviceInfoProc pclGetDeviceInfo
//              cl_device_info param_name - the parameter name
//              apCLFPConfig& paramValueAsFPConfig - the device floating point configuration value
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/3/2010
// ---------------------------------------------------------------------------
bool apCLDevice::getDeviceFPConfig(clGetDeviceInfoProc pclGetDeviceInfo, cl_device_info param_name, apCLDeviceFloatingPointConfigParameter& paramValueAsFPConfig)
{
    bool retVal = false;

    // Get the device type:
    cl_device_fp_config deviceFPConfig = CL_FP_DENORM;
    cl_int clRetVal = pclGetDeviceInfo(cl_device_id(_deviceHandle), param_name, sizeof(cl_device_fp_config), &deviceFPConfig, NULL);

    if (clRetVal == CL_SUCCESS)
    {
        paramValueAsFPConfig.setValue(deviceFPConfig);
        retVal = true;
    }
    else
    {
        // Yaki - 5/4/2010:
        // The double and half precision floating point capabilities are optional for an OpenCL device.
        // Therefore, we don't assert if param_name is related to querying these precisions capabilities.
        // Also, these parameters don't exist on Mac :-)
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        {
            GT_ASSERT(false);
        }
#else
        {
            if (!((param_name == CL_DEVICE_DOUBLE_FP_CONFIG) || (param_name == CL_DEVICE_HALF_FP_CONFIG)))
            {
                GT_ASSERT(false);
            }
        }
#endif
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLDevice::getDeviceExecutionCapabilities
// Description: Get & Set the device's execution capabilities
// Arguments:   clGetDeviceInfoProc pclGetDeviceInfo
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/3/2010
// ---------------------------------------------------------------------------
bool apCLDevice::getDeviceExecutionCapabilities(clGetDeviceInfoProc pclGetDeviceInfo)
{
    bool retVal = false;

    // Get the device execution capabilities:
    cl_device_exec_capabilities deviceExecCaps = CL_EXEC_KERNEL;
    cl_int clRetVal = pclGetDeviceInfo(cl_device_id(_deviceHandle), CL_DEVICE_EXECUTION_CAPABILITIES, sizeof(cl_device_exec_capabilities), &deviceExecCaps, NULL);
    GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
    {
        _deviceExecutionCapabilities.setValue(deviceExecCaps);
        retVal = true;
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLDevice::getDeviceExtensions
// Description: Get & Set the device's extensions
// Arguments:   clGetDeviceInfoProc pclGetDeviceInfo
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/4/2010
// ---------------------------------------------------------------------------
bool apCLDevice::getDeviceExtensions(clGetDeviceInfoProc pclGetDeviceInfo)
{
    bool retVal = false;

    // Get the device execution capabilities:
    size_t extensionsSize;
    cl_int clRetVal = pclGetDeviceInfo(cl_device_id(_deviceHandle), CL_DEVICE_EXTENSIONS, 0, NULL, &extensionsSize);
    GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
    {
        // Allocate a space for the extensions:
        char* pExtensions = new char[extensionsSize];

        clRetVal = pclGetDeviceInfo(cl_device_id(_deviceHandle), CL_DEVICE_EXTENSIONS, extensionsSize, pExtensions, NULL);
        GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
        {
            gtString currentExtension;

            for (int i = 0; i < (int)extensionsSize; i++)
            {
                if (pExtensions[i] != ' ')
                {
                    // Add current character to the extension string:
                    currentExtension.append(pExtensions[i]);
                }
                else
                {
                    // Add the last extension to the extensions list:
                    _extensions.push_back(currentExtension);
                    currentExtension.makeEmpty();
                }
            }

            retVal = true;
        }

        // Release the memory:
        delete[] pExtensions;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLDevice::getDeviceMemoryCacheType
// Description: Get & Set the device's memory cache type
// Arguments:   clGetDeviceInfoProc pclGetDeviceInfo
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/3/2010
// ---------------------------------------------------------------------------
bool apCLDevice::getDeviceMemoryCacheType(clGetDeviceInfoProc pclGetDeviceInfo)
{
    bool retVal = false;

    // Get the device memory cache type:
    cl_device_mem_cache_type deviceCacheType = CL_NONE;
    cl_int clRetVal = pclGetDeviceInfo(cl_device_id(_deviceHandle), CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, sizeof(cl_device_mem_cache_type), &deviceCacheType, NULL);
    GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
    {
        switch (deviceCacheType)
        {
            case CL_NONE:
                _memoryCacheType = AP_CL_MEM_CACHE_NONE;
                break;

            case CL_READ_ONLY_CACHE:
                _memoryCacheType = AP_CL_MEM_READ_ONLY_CACHE;
                break;

            case CL_READ_WRITE_CACHE:
                _memoryCacheType = AP_CL_MEM_READ_WRITE_CACHE;
                break;

            default:
                _memoryCacheType = AP_CL_MEM_UNKNOWN_CACHE;
                GT_ASSERT(false);
                break;
        }

        retVal = true;
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLDevice::getDeviceLocalMemType
// Description: Get & Set the device's local memory type
// Arguments:   clGetDeviceInfoProc pclGetDeviceInfo
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/3/2010
// ---------------------------------------------------------------------------
bool apCLDevice::getDeviceLocalMemType(clGetDeviceInfoProc pclGetDeviceInfo)
{
    bool retVal = false;

    // Get the device local memory type:
    cl_device_local_mem_type deviceMemoryType = CL_LOCAL;
    cl_int clRetVal = pclGetDeviceInfo(cl_device_id(_deviceHandle), CL_DEVICE_LOCAL_MEM_TYPE, sizeof(cl_device_local_mem_type), &deviceMemoryType, NULL);
    GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
    {
        switch (deviceMemoryType)
        {
            case CL_GLOBAL:
                _localMemType = AP_CL_MEM_GLOBAL;
                break;

            case CL_LOCAL:
                _localMemType = AP_CL_MEM_LOCAL;
                break;

            default:
                _localMemType = AP_CL_MEM_UNKNOWN;
                GT_ASSERT(false);
                break;
        }

        retVal = true;
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLDevice::maxImage2DDimension
// Description: Get Max image 2D parameter value
// Arguments:   size_t& maxImage2DDimension1
//            size_t& maxImage2DDimension2
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        18/3/2010
// ---------------------------------------------------------------------------
void apCLDevice::maxImage2DDimension(gtUInt32& maxImage2DDimension1, gtUInt32& maxImage2DDimension2) const
{
    if (_pMaxWorkItemSizes != NULL)
    {
        maxImage2DDimension1 = _pMaxWorkItemSizes[0];
        maxImage2DDimension2 = _pMaxWorkItemSizes[1];
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLDevice::maxImage3DDimension
// Description: Get Max image 3D parameter value
// Arguments:   size_t& maxImage3DDimension1
//              size_t& maxImage3DDimension2
//              size_t& maxImage3DDimension3
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        18/3/2010
// ---------------------------------------------------------------------------
void apCLDevice::maxImage3DDimension(gtUInt32& maxImage3DDimension1, gtUInt32& maxImage3DDimension2, gtUInt32& maxImage3DDimension3) const
{
    if (_pMaxWorkItemSizes != NULL)
    {
        maxImage3DDimension1 = _pMaxWorkItemSizes[0];
        maxImage3DDimension2 = _pMaxWorkItemSizes[1];
        maxImage3DDimension3 = _pMaxWorkItemSizes[2];
    }
}


// ---------------------------------------------------------------------------
// Name:        apCLDevice::parseOpenCLVersionString
// Description: Parses the OpenCL version string, and return it as numbers
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        25/10/2010
// ---------------------------------------------------------------------------
bool apCLDevice::parseOpenCLVersionString()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(!_deviceVersion.isEmpty())
    {
        // The expected string is in the following format (from clGetDeviceInfo documentation):
        // OpenCL<space><major_version.minor_version><space><vendor-specific information>
        gtString token;
        gtStringTokenizer tokenizer(_deviceVersion, L" .");

        // Get the first token (expecting "OpenCL"):
        tokenizer.getNextToken(token);
        GT_IF_WITH_ASSERT(token == AP_STR_OpenCL)
        {
            // Get the major version token:
            tokenizer.getNextToken(token);
            GT_IF_WITH_ASSERT(token.isIntegerNumber())
            {
                // Get the major version as integer number:
                int version = 0;
                bool rcGetMajor = token.toIntNumber(version);
                GT_IF_WITH_ASSERT(rcGetMajor)
                {
                    _clMajorVersion = version;

                    // Get the minor version token:
                    tokenizer.getNextToken(token);
                    GT_IF_WITH_ASSERT(token.isIntegerNumber())
                    {
                        // Get the major version as integer number:
                        bool rcGetMinor = token.toIntNumber(version);
                        GT_IF_WITH_ASSERT(rcGetMinor)
                        {
                            _clMinorVersion = version;
                            retVal = true;
                        }
                    }
                }
            }
        }
    }
    return retVal;
}


