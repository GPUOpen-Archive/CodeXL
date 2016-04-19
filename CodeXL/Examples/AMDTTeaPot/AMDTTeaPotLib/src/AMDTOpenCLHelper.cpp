//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTOpenCLHelper.cpp
///
//==================================================================================

//------------------------------ AMDTOpenCLHelper.cpp ----------------------------

#include <stdio.h>
#include <string.h>

#include <inc/AMDTOpenCLHelper.h>

#if defined(WIN32)
#elif defined(__linux__)    // #if defined(WIN32)
    #include <GL/glx.h>
    #include <dlfcn.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <dirent.h>
    #include <iostream>
#endif

#include "AMDTMisc.h"
#include "AMDTDebug.h"
// Static storage for the singleton.
AMDTOpenCLHelper* AMDTOpenCLHelper::_instance = NULL;

// ---------------------------------------------------------------------------
// Name:        AMDTOpenCLHelper::GetInstance()
// Description: Get a reference to this singleton, increasing the reference
//              count by 1.
// ---------------------------------------------------------------------------
AMDTOpenCLHelper* AMDTOpenCLHelper::GetInstance()
{
    if (!_instance)
    {
        _instance = new AMDTOpenCLHelper();
    }

    _instance->_refCounter++;
    return _instance;
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenCLHelper::ReleaseInstance()
// Description: Release reference to singleton. If there are no other
//              references, delete the singleton.
// ---------------------------------------------------------------------------
void AMDTOpenCLHelper::ReleaseInstance()
{
    if (_instance)
    {
        if (_instance->_refCounter == 1)
        {
            delete _instance;
            _instance = NULL;
        }
        else
        {
            --_instance->_refCounter;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenCLHelper::AMDTOpenCLHelper
// Description: Constructor. Load OpenCL library, get function pointers and
//              build up a list of platforms and devices and information about
//              devices.
// ---------------------------------------------------------------------------
AMDTOpenCLHelper::AMDTOpenCLHelper() :
    _clGetExtensionFunctionAddress(NULL),
    _refCounter(0),
    _ready(false),
    _hOpenCLLibrary(NULL),
    _clInfo(NULL)
{
    const char* libName = NULL;

    _lastError[0] = '\0';

    if (isOpenCLAvailableBypassCodeXLInterception())
    {
#if defined (__APPLE__)
        // Mac OS X not supported
        libName = "/System/Library/Frameworks/OpenCL.framework/OpenCL";
        _hOpenCLLibrary = (void*)dlopen(libName, RTLD_LAZY | RTLD_LOCAL);

        if (_hOpenCLLibrary == NULL)
        {
            libName = "/System/Library/Frameworks/OpenCL.framework/Libraries/libCL.dylib");
            _hOpenCLLibrary = (void*)dlopen(libName, RTLD_LAZY | RTLD_LOCAL);
        }

#elif defined(_WINDOWS)
        // Load OpenCL.dll
        libName = "OpenCL.dll";
        _hOpenCLLibrary = (void*)LoadLibrary(L"OpenCL.dll");
#elif defined(__linux__)
        libName = "libOpenCL.so";
        const char* libName_alt = "libOpenCL.so.1";
        _hOpenCLLibrary = dlopen(libName, RTLD_LAZY | RTLD_LOCAL);

        // Unfortunately, this does not always work.  Depending upon how
        // it was installed, there may not actually be a symbolic link
        // from libOpenCL.so to libOpenCL.so.1.
        if (_hOpenCLLibrary == NULL)
        {
            _hOpenCLLibrary = dlopen(libName_alt, RTLD_LAZY | RTLD_LOCAL);
        }

#endif
    }

    if (_hOpenCLLibrary)
    {
        if (initialize() && initOpenCLInfo())
        {
            _ready = true;
        }
    }
    else if (libName != NULL)
    {
        sprintf(_lastError, "Can't load OpenCL library (%s)", libName);
    }
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenCLHelper::~AMDTOpenCLHelper
// Description: Deconstructor. Unload the library.
// ---------------------------------------------------------------------------
AMDTOpenCLHelper::~AMDTOpenCLHelper()
{
#if defined (__APPLE__) || defined(__linux__)

    // Close the OpenCL framework handle:
    if (_hOpenCLLibrary)
    {
        dlclose(_hOpenCLLibrary);
    }

#elif defined(_WINDOWS)

    // Free our reference to OpenCL.dll
    if (_hOpenCLLibrary)
    {
        FreeLibrary(static_cast<HMODULE>(_hOpenCLLibrary));
    }

#endif

    delete _clInfo;
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenCLHelper::isReady()
// Description: Return true if initialization was successful i.e. OpenCL was
//              loaded and platforms/devices were queried.
// ---------------------------------------------------------------------------
bool AMDTOpenCLHelper::isReady() const
{
    return _ready;
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenCLHelper::getOpenCLErrorCodeStr()
// Description: Convert an OpenCL status code into a string.
// ---------------------------------------------------------------------------
const char* AMDTOpenCLHelper::getOpenCLErrorCodeStr(
    cl_int status)
{
#define CASE(STATUS)                                    \
case STATUS:                                        \
    retVal = #STATUS;                                   \
    break

    const char* retVal = NULL;

    switch (status)
    {
            CASE(CL_DEVICE_NOT_FOUND);
            CASE(CL_DEVICE_NOT_AVAILABLE);
            CASE(CL_COMPILER_NOT_AVAILABLE);
            CASE(CL_MEM_OBJECT_ALLOCATION_FAILURE);
            CASE(CL_OUT_OF_RESOURCES);
            CASE(CL_OUT_OF_HOST_MEMORY);
            CASE(CL_PROFILING_INFO_NOT_AVAILABLE);
            CASE(CL_IMAGE_FORMAT_MISMATCH);
            CASE(CL_MEM_COPY_OVERLAP);
            CASE(CL_IMAGE_FORMAT_NOT_SUPPORTED);
            CASE(CL_BUILD_PROGRAM_FAILURE);
            CASE(CL_MAP_FAILURE);
            CASE(CL_MISALIGNED_SUB_BUFFER_OFFSET);
            CASE(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
            CASE(CL_INVALID_VALUE);
            CASE(CL_INVALID_PLATFORM);
            CASE(CL_INVALID_DEVICE);
            CASE(CL_INVALID_CONTEXT);
            CASE(CL_INVALID_QUEUE_PROPERTIES);
            CASE(CL_INVALID_COMMAND_QUEUE);
            CASE(CL_INVALID_HOST_PTR);
            CASE(CL_INVALID_MEM_OBJECT);
            CASE(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
            CASE(CL_INVALID_IMAGE_SIZE);
            CASE(CL_INVALID_SAMPLER);
            CASE(CL_INVALID_BINARY);
            CASE(CL_INVALID_BUILD_OPTIONS);
            CASE(CL_INVALID_PROGRAM);
            CASE(CL_INVALID_PROGRAM_EXECUTABLE);
            CASE(CL_INVALID_KERNEL_NAME);
            CASE(CL_INVALID_KERNEL_DEFINITION);
            CASE(CL_INVALID_KERNEL);
            CASE(CL_INVALID_ARG_INDEX);
            CASE(CL_INVALID_ARG_VALUE);
            CASE(CL_INVALID_ARG_SIZE);
            CASE(CL_INVALID_KERNEL_ARGS);
            CASE(CL_INVALID_WORK_DIMENSION);
            CASE(CL_INVALID_WORK_GROUP_SIZE);
            CASE(CL_INVALID_WORK_ITEM_SIZE);
            CASE(CL_INVALID_GLOBAL_OFFSET);
            CASE(CL_INVALID_EVENT_WAIT_LIST);
            CASE(CL_INVALID_EVENT);
            CASE(CL_INVALID_OPERATION);
            CASE(CL_INVALID_GL_OBJECT);
            CASE(CL_INVALID_BUFFER_SIZE);
            CASE(CL_INVALID_MIP_LEVEL);
            CASE(CL_INVALID_GLOBAL_WORK_SIZE);
            CASE(CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR);
            CASE(CL_PLATFORM_NOT_FOUND_KHR);

        //CASE(CL_INVALID_PROPERTY_EXT);
        //CASE(CL_DEVICE_PARTITION_FAILED_EXT);
        //CASE(CL_INVALID_PARTITION_COUNT_EXT);
        default:
            retVal = "unknown error code";
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenCLHelper::getLastError()
// Description: Return the last error that occurred during initialization,
//              otherwise NULL if there was no error.
// ---------------------------------------------------------------------------
const char* AMDTOpenCLHelper::getLastError()
{
    const char* retVal = NULL;

    if ('\0' != _lastError[0])
    {
        retVal = _lastError;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenCLHelper::initialize()
// Description: Look up function pointers for all OpenCL API calls. Make sure
//              that we get pointers for the functions that are used by this
//              helper to build up the list of platforms/devices.
// ---------------------------------------------------------------------------
bool AMDTOpenCLHelper::initialize()
{
    bool retVal = false;
#define GETPFN(func) _ ## func = reinterpret_cast<func ## _fn>(getProcAddress(# func))

    // We first get the function pointer to clGetExtensionFunctionAddress()
    // since this is used by getProcAddress() (see below) to get the function
    // pointers to the OpenCL extension functions.
    GETPFN(clGetExtensionFunctionAddress);
    GETPFN(clGetPlatformIDs);
    GETPFN(clGetPlatformInfo);
    GETPFN(clGetDeviceIDs);
    GETPFN(clGetDeviceInfo);
    GETPFN(clCreateContext);
    GETPFN(clCreateContextFromType);
    GETPFN(clGetContextInfo);
    GETPFN(clReleaseContext);
    GETPFN(clCreateCommandQueue);
    GETPFN(clReleaseCommandQueue);
    GETPFN(clCreateProgramWithSource);
    GETPFN(clReleaseProgram);
    GETPFN(clBuildProgram);
    GETPFN(clUnloadCompiler);
    GETPFN(clGetProgramInfo);
    GETPFN(clGetProgramBuildInfo);
    GETPFN(clCreateKernel);
    GETPFN(clReleaseKernel);
    GETPFN(clSetKernelArg);
    GETPFN(clEnqueueNDRangeKernel);
    GETPFN(clEnqueueCopyBufferToImage);
    GETPFN(clEnqueueMapBuffer);
    GETPFN(clEnqueueUnmapMemObject);
    GETPFN(clCreateBuffer);
    GETPFN(clCreateImage2D);
    GETPFN(clCreateImage3D);
    GETPFN(clReleaseMemObject);
    GETPFN(clGetGLContextInfoKHR);
    GETPFN(clGetSupportedImageFormats);
    GETPFN(clCreateFromGLBuffer);
    GETPFN(clCreateFromGLTexture3D);
    GETPFN(clGetKernelWorkGroupInfo);
    GETPFN(clWaitForEvents);
    GETPFN(clReleaseEvent);
    GETPFN(clEnqueueAcquireGLObjects);
    GETPFN(clEnqueueReleaseGLObjects);
    GETPFN(clEnqueueWriteBuffer);
    GETPFN(clEnqueueWriteBufferRect);
    GETPFN(clFlush);
    GETPFN(clFinish);
    GETPFN(clGetGLObjectInfo);

    // Sanity check - Make sure we have the main function pointers that will
    // be used during the rest of initialization (querying platforms/devices).
    if (_clGetExtensionFunctionAddress
        && _clGetPlatformIDs
        && _clGetPlatformInfo
        && _clGetDeviceIDs
        && _clGetDeviceInfo)
    {
        retVal = true;
    }

    strcpy(_lastError, "OpenCL library is missing export functions.");
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenCLHelper::getProcAddress
// Description: Inputs an extension function name and returns its pointer
//              from the OpenCL library.
// ---------------------------------------------------------------------------
oclProcedureAddress AMDTOpenCLHelper::getProcAddress(
    const char* pProcName)
{
    oclProcedureAddress retVal = NULL;

    // First try to get the function pointer using the system call.
#if defined(_WIN32)
    retVal = (oclProcedureAddress)GetProcAddress(
                 static_cast<HMODULE>(_hOpenCLLibrary),
                 pProcName);
#elif defined (__APPLE__) || defined(__linux__)
    retVal = (oclProcedureAddress)dlsym(_hOpenCLLibrary, pProcName);
#endif

    // If the system doesn't return the function pointer, then probably
    // its an extension function and we need to use clGetExtensionFunctionAddress().
    if (retVal == NULL && _clGetExtensionFunctionAddress)
    {
        retVal = reinterpret_cast<oclProcedureAddress>(_clGetExtensionFunctionAddress(pProcName));
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenCLHelper::isOpenCLAvailableBypassCodeXLInterception
// Description: Bypasses CodeXL's intercetion to see if a real OpenCL implementation
//              is available.
// ---------------------------------------------------------------------------
bool AMDTOpenCLHelper::isOpenCLAvailableBypassCodeXLInterception()
{
    bool retVal = false;

#if defined (__APPLE__)
    // On Mac, CodeXL interception does not cause dlopen to report false positives:
    retVal = true;
#elif defined(_WINDOWS)

    // Store the dll directory value:
    DWORD dllDirPathLength = ::GetDllDirectory(0, NULL);
    WCHAR* dllDirPath = NULL;

    if (0 < dllDirPathLength)
    {
        dllDirPath = new WCHAR[dllDirPathLength + 1];
        ::GetDllDirectory(dllDirPathLength, dllDirPath);
        dllDirPath[dllDirPathLength] = L'\0';

        // If the dir path is empty, treat it as nonexistent:
        if (L'\0' == dllDirPath[0])
        {
            dllDirPathLength = 0;
            delete[] dllDirPath;
            dllDirPath = nullptr;
        }

        // Clear the dll directory:
        ::SetDllDirectory(L"");
    }

    // See if we can load a library with this search path:
    HMODULE hRealOpenCL = ::LoadLibrary(L"OpenCL.dll");
    retVal = (NULL != hRealOpenCL);

    // Unload it, so we can properly load the OpenCL server:
    if (NULL != hRealOpenCL)
    {
        ::FreeLibrary(hRealOpenCL);
    }

    // On Windows 7, under some scenarios, the Dll directory might be unset:
    if (0 == dllDirPathLength)
    {
        // Check for the CodeXL servers path environment variable. If it is present, we are running under CodeXL
        // and we might want to use it as the value:
        static const wchar_t* envVarName = L"SU_SPIES_DIRECTORY";
        DWORD envVarLength = ::GetEnvironmentVariableW(envVarName, NULL, 0);

        if (0 < envVarLength)
        {
            dllDirPath = new WCHAR[envVarLength + 1];
            ::GetEnvironmentVariableW(envVarName, dllDirPath, envVarLength);
            dllDirPath[envVarLength] = L'\0';
        }
    }

    // Restore the dll directory:
    if (NULL != dllDirPath)
    {
        ::SetDllDirectory(dllDirPath);
        delete[] dllDirPath;
        dllDirPath = NULL;
    }

#elif defined(__linux__)
    DIR* pDir = ::opendir("/etc/OpenCL/vendors");
    struct dirent* ent;
    struct stat st;

    if (pDir != NULL)
    {
        while ((ent = ::readdir(pDir)) != NULL)
        {
            std::string file_name = ent->d_name;
            std::string full_file_name = "/etc/OpenCL/vendors/" + file_name;

            if (file_name[0] == '.')
            {
                continue;
            }

            if (stat(full_file_name.c_str(), &st) == -1)
            {
                continue;
            }

            if (S_ISDIR(st.st_mode))
            {
                continue;
            }

            if (file_name.rfind(".icd") == file_name.length() - ::strlen(".icd"))
            {
                retVal = true;
                break;
            }
        }

        ::closedir(pDir);
    }
    else
    {
        retVal = false;
    }

#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenCLHelper::isExtensionSupported
// Description: Inputs an extension name and returns true if the extension is
//              contained in the platform/device extension string.
// ---------------------------------------------------------------------------
bool AMDTOpenCLHelper::isExtensionSupported(
    char* extensionName,
    const char* extensionsString)
{
    bool retVal = false;

    // Search for extensionName in the extensions string.
    // (The use of strstr() is not sufficient because extension names can be prefixes of
    // other extension names).
    char* pCurrentPos = (char*)extensionsString;
    char* pEndPos;
    int extensionNameLen = strlen(extensionName);
    pEndPos = pCurrentPos + strlen(pCurrentPos);

    while (pCurrentPos < pEndPos)
    {
        int n = strcspn(pCurrentPos, " ");

        if ((extensionNameLen == n) && (strncmp(extensionName, pCurrentPos, n) == 0))
        {
            retVal = true;
            break;
        }

        pCurrentPos += (n + 1);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenCLHelper::setLastError
// Description: Set last error message for failed OpenCL function call.
// ---------------------------------------------------------------------------
void AMDTOpenCLHelper::setLastError(
    const char* clFunctionName,
    cl_int status)
{
    sprintf(_lastError, "Call to %s() returned an error: [%d] %s",
            clFunctionName, status, getOpenCLErrorCodeStr(status));
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenCLHelper::getPlatformInfo
// Description: Helper function to get a platform string info, allocate
//              storage and save.
// ---------------------------------------------------------------------------
bool AMDTOpenCLHelper::getPlatformInfo(
    cl_platform_id platform,
    cl_platform_info param_name,
    char** out_info)
{
    bool retVal = false;
    size_t size;
    cl_int status = _clGetPlatformInfo(platform, param_name, 0, NULL, &size);

    if (status == CL_SUCCESS && size > 0)
    {
        char* str = new char[size];
        status = _clGetPlatformInfo(platform, param_name, size, str, NULL);

        if (status == CL_SUCCESS)
        {
            *out_info = str;
            retVal = true;
        }
        else
        {
            delete [] str;
        }
    }

    setLastError("clGetPlatformInfo", status);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenCLHelper::getDeviceInfo
// Description: Helper function to get a device string info, allocate
//              storage and save.
// ---------------------------------------------------------------------------
bool AMDTOpenCLHelper::getDeviceInfo(
    cl_device_id device,
    cl_device_info param_name,
    char** out_info)
{
    bool retVal = false;
    size_t size;
    cl_int status = _clGetDeviceInfo(device, param_name, 0, NULL, &size);

    if (status == CL_SUCCESS && size > 0)
    {
        char* str = new char[size];
        status = _clGetDeviceInfo(device, param_name, size, str, NULL);

        if (status == CL_SUCCESS)
        {
            *out_info = str;
            retVal = true;
        }
        else
        {
            delete [] str;
        }
    }

    setLastError("clGetDeviceInfo", status);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenCLHelper::initOpenCLInfo
// Description: Collect information about all OpenCL platforms/devices on this
//              host.
// ---------------------------------------------------------------------------
bool
AMDTOpenCLHelper::initOpenCLInfo()
{
    bool retVal = false;

    cl_uint numPlatforms;
    cl_int status = _clGetPlatformIDs(0, NULL, &numPlatforms);

    if (status == CL_SUCCESS && numPlatforms > 0)
    {
        cl_platform_id* platforms = new cl_platform_id[numPlatforms];
        status = _clGetPlatformIDs(numPlatforms, platforms, NULL);

        if (status == CL_SUCCESS)
        {
            // Create the new OCLInfo structure.
            _clInfo = new OCLInfo(numPlatforms);

            for (cl_uint i = 0; i < numPlatforms; ++i)
            {
                // Retrive the number of devices present for this platform.
                cl_uint numDevices;
                status = _clGetDeviceIDs(
                             platforms[i],
                             CL_DEVICE_TYPE_ALL,
                             0,
                             NULL,
                             &numDevices);

                if (status == CL_SUCCESS)
                {
                    cl_device_id* devices = new cl_device_id[numDevices];
                    status = _clGetDeviceIDs(
                                 platforms[i],
                                 CL_DEVICE_TYPE_ALL,
                                 numDevices,
                                 devices,
                                 NULL);

                    if (status == CL_SUCCESS)
                    {
                        // Create a new OCLPlatform structure and query
                        // the various pieces of platform information.
                        char* str;
                        OCLPlatform* platform =
                            new OCLPlatform(platforms[i], numDevices);

                        if (getPlatformInfo(
                                platforms[i],
                                CL_PLATFORM_VERSION,
                                &str))
                        {
                            platform->setVersion(str);
                        }

                        if (getPlatformInfo(
                                platforms[i],
                                CL_PLATFORM_NAME,
                                &str))
                        {
                            platform->setName(str);
                        }

                        if (getPlatformInfo(
                                platforms[i],
                                CL_PLATFORM_VENDOR,
                                &str))
                        {
                            platform->setVendor(str);
                        }

                        if (getPlatformInfo(
                                platforms[i],
                                CL_PLATFORM_EXTENSIONS,
                                &str))
                        {
                            platform->setExtensions(str);
                        }

                        // Only add the platform if we have a valid version.
                        if (platform->getVersion() >= 1.0f)
                        {
                            // Add this platform to OCLInfo.
                            _clInfo->addPlatform(platform);

                            // Check if the GL sharing extension is supported
                            // by this platform and set the flag in the
                            // OCLPlatform structure.
                            if (isExtensionSupported(
                                    (char*)"cl_khr_gl_sharing",
                                    platform->getExtensions()))
                            {
                                platform->setGLSharing(true);
                            }

                            // Go through each device and query information.
                            for (cl_uint j = 0; j < numDevices; ++j)
                            {
                                cl_device_type type;
                                status = _clGetDeviceInfo(
                                             devices[j],
                                             CL_DEVICE_TYPE,
                                             sizeof(cl_device_type),
                                             &type,
                                             NULL);

                                if (status == CL_SUCCESS)
                                {
                                    cl_uint data_uint;
                                    size_t data_sizet;
                                    cl_ulong data_ulong;

                                    OCLDevice* device =
                                        new OCLDevice(platform, devices[j], type);

                                    if (getDeviceInfo(
                                            devices[j],
                                            CL_DEVICE_NAME, &str))
                                    {
                                        device->setName(str);
                                    }

                                    if (getDeviceInfo(
                                            devices[j],
                                            CL_DEVICE_VENDOR, &str))
                                    {
                                        device->setVendor(str);
                                    }

                                    if (getDeviceInfo(
                                            devices[j],
                                            CL_DEVICE_EXTENSIONS, &str))
                                    {
                                        device->setExtensions(str);
                                    }

                                    if (getDeviceInfo(
                                            devices[j],
                                            CL_DEVICE_VERSION, &str))
                                    {
                                        device->setVersion(str);
                                    }

                                    status = _clGetDeviceInfo(
                                                 device->getDeviceID(),
                                                 CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
                                                 sizeof(data_uint),
                                                 &data_uint,
                                                 NULL);

                                    if (status == CL_SUCCESS)
                                    {
                                        device->_maxWorkItemDimensions = data_uint;
                                    }

                                    if (device->_maxWorkItemDimensions > 0)
                                    {
                                        device->_maxWorkItemSizes = new size_t[device->_maxWorkItemDimensions];
                                        int size = sizeof(size_t) * device->_maxWorkItemDimensions;
                                        memset(device->_maxWorkItemSizes, 0, size);
                                        status = _clGetDeviceInfo(
                                                     device->getDeviceID(),
                                                     CL_DEVICE_MAX_WORK_ITEM_SIZES,
                                                     size,
                                                     device->_maxWorkItemSizes,
                                                     NULL);
                                    }

                                    status = _clGetDeviceInfo(
                                                 device->getDeviceID(),
                                                 CL_DEVICE_MAX_WORK_GROUP_SIZE,
                                                 sizeof(data_sizet),
                                                 &data_sizet,
                                                 NULL);

                                    if (status == CL_SUCCESS)
                                    {
                                        device->_maxWorkGroupSize = data_sizet;
                                    }

                                    status = _clGetDeviceInfo(
                                                 device->getDeviceID(),
                                                 CL_DEVICE_LOCAL_MEM_SIZE,
                                                 sizeof(data_ulong),
                                                 &data_ulong,
                                                 NULL);

                                    if (status == CL_SUCCESS)
                                    {
                                        device->_localMemSize = data_ulong;
                                    }

                                    if (device->getVersion() >= 1.1f)
                                    {
                                        cl_bool flag;
                                        status = _clGetDeviceInfo(
                                                     devices[j],
                                                     CL_DEVICE_HOST_UNIFIED_MEMORY,
                                                     sizeof(flag),
                                                     &flag,
                                                     NULL);

                                        if (status == CL_SUCCESS)
                                        {
                                            device->setHaveUnifiedMemory(flag == CL_TRUE);
                                        }

                                        if (getDeviceInfo(
                                                devices[j],
                                                CL_DEVICE_OPENCL_C_VERSION, &str))
                                        {
                                            device->setCVersion(str);
                                        }
                                    }

                                    if (device->getName()
                                        && device->getVendor()
                                        && device->getExtensions()
                                        && device->getVersion())
                                    {
                                        // Add this device to OCLPlatform if
                                        // we were able to query the minimum
                                        // information.
                                        platform->addDevice(device);

                                        // Set GL sharing flag if exported by
                                        // device extension. If device exports
                                        // the extension, then flag the platform
                                        // as well.
                                        if (isExtensionSupported(
                                                (char*)"cl_khr_gl_sharing",
                                                device->getExtensions()))
                                        {
                                            platform->setGLSharing(true);
                                            device->setGLSharing(true);
                                        }
                                    }
                                    else
                                    {
                                        delete device;
                                    }
                                }
                                else
                                {
                                    setLastError("clGetDeviceInfo", status);
                                }
                            }
                        }
                        else
                        {
                            delete platform;
                        }
                    }
                    else
                    {
                        setLastError("clGetDeviceIDs", status);
                    }

                    delete [] devices;
                }
                else
                {
                    setLastError("clGetDeviceIDs", status);
                }
            }

            if (_clInfo->getNumPlatforms() > 0)
            {
                retVal = true;
            }
            else
            {
                strcpy(_lastError, "No valid OpenCL platforms are available.");
            }
        }
        else
        {
            setLastError("clGetPlatformIDs", status);
        }

        delete [] platforms;
    }
    else if (status != CL_SUCCESS)
    {
        setLastError("clGetPlatformIDs", status);
    }
    else
    {
        strcpy(_lastError, "No OpenCL platforms are available.");
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenCLHelper::initGLCLBinding
// Description: For each platform, check which devices can be associated with
//              the current OpenGL context. Flag these devices so that the
//              application can use this information to choose the best device
//              for GL sharing.
// ---------------------------------------------------------------------------
bool AMDTOpenCLHelper::updateGLCLBinding()
{
    bool retVal = true;

    if (!_ready || !_clInfo)
    {
        retVal = false;
    }
    else
    {
        // Reset Gl-association flag for each OCLDevice.
        for (int i = _clInfo->getNumPlatforms() - 1; i >= 0; --i)
        {
            OCLPlatform* platform = _clInfo->getPlatform(i);

            for (int j = platform->getNumDevices() - 1; j >= 0; --j)
            {
                OCLDevice* device = platform->getDevice(j);
                device->setGLAssociation(false);
            }
        }

        // Only query if we were able to get the clGetGLContextInfoKHR()
        // API function pointer.
        if (_clInfo && _clGetGLContextInfoKHR)
        {
            for (int i = _clInfo->getNumPlatforms() - 1; i >= 0; --i)
            {
                // For each platform, call clGetGLContextInfoKHR() to get the
                // list of devices that support GL association.
                OCLPlatform* platform = _clInfo->getPlatform(i);

                size_t reqSize;
                cl_context_properties p[7];
                p[0] = CL_CONTEXT_PLATFORM;
                p[1] = (cl_context_properties)platform->getPlatformID();
                p[2] = CL_GL_CONTEXT_KHR;

#if defined (__linux__)
                p[3] = (cl_context_properties)glXGetCurrentContext();
                p[4] = CL_GLX_DISPLAY_KHR;
                p[5] = (cl_context_properties)glXGetCurrentDisplay();
#else // Windows
                p[3] = (cl_context_properties)wglGetCurrentContext();
                p[4] = CL_WGL_HDC_KHR;
                p[5] = (cl_context_properties)wglGetCurrentDC();
#endif
                p[6] = 0;
                cl_int status = _clGetGLContextInfoKHR(
                                    p,
                                    CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR,
                                    0, NULL, &reqSize);

                if (status == CL_SUCCESS)
                {
                    int numDevices = reqSize / sizeof(cl_device_id);
                    cl_device_id* devids = new cl_device_id[numDevices];
                    status = _clGetGLContextInfoKHR(
                                 p,
                                 CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR,
                                 numDevices * sizeof(cl_device_id),
                                 devids,
                                 NULL);

                    if (status == CL_SUCCESS)
                    {
                        // Go through the devices in the platforms device list
                        // and flag those that were returned by the
                        // call clGetGLContextInfoKHR().
                        for (int j = platform->getNumDevices() - 1; j >= 0; --j)
                        {
                            OCLDevice* device = platform->getDevice(j);
                            device->setGLAssociation(false);

                            for (int k = numDevices - 1; k >= 0; --k)
                            {
                                if (device->getDeviceID() == devids[k])
                                {
                                    device->setGLAssociation(true);
                                    break;
                                }
                            }
                        }
                    }

                    if (devids != NULL)
                    {
                        delete[] devids;
                    }
                }
            }

        }

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenCLHelper::getOpenCLInfo
// Description: Return a pointer to the OpenCL info structure that was
//              populated during intialization of this helper instance.
// ---------------------------------------------------------------------------
const OCLInfo* AMDTOpenCLHelper::getOpenCLInfo() const
{
    const OCLInfo* retVal = NULL;

    if (_ready)
    {
        retVal = _clInfo;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenCLHelper::readKernelSource()
// Description: Read kernel source from a file, returning an allocated string
//              owned by the caller.
// ---------------------------------------------------------------------------
char* AMDTOpenCLHelper::readKernelSource(const wchar_t* path)
{
    char* ret = NULL;

#if defined(__linux__)
    FILE* fh;
    NEW_CS_FROM_WCS(path);
    fh = fopen(_path, "rt");
#else
    FILE* fh = _wfopen(path, L"rt");
#endif

    if (fh)
    {
        if (fseek(fh, 0, SEEK_END) == 0)
        {
            size_t size = ftell(fh);

            if (size > 0)
            {
                ret = new char[size + 1];

                // Move back to start of file.
                fseek(fh, 0, SEEK_SET);

                // Read the file content into the buffer:
                size_t n = fread(ret, sizeof(char), size, fh);

                if ((n == size) || feof(fh))
                {
                    ret[n] = '\0';
                }
                else
                {
                    delete[] ret;
                    ret = NULL;
                }
            }
        }

        fclose(fh);
    }

    return ret;
}

/******************************************************************************/
/* Below are the member implementations classes OCLInfo, OCLPlatform and      */
/* OCLDevice.                                                                 */
/******************************************************************************/

// ---------------------------------------------------------------------------
// Name:        ::stristr(const char* arg1, const char* arg2)
// Description: Case insensitive search for the first occurence of arg2 in arg1.
// ---------------------------------------------------------------------------
static const char* stristr(
    const char* arg1,
    const char* arg2)
{
    const char* a;
    const char* b;
    const char* retVal = NULL;

    for (; *arg1; ++arg1)
    {
        a = arg1;
        b = arg2;

        while ((*a++ | 32) == (*b++ | 32))
        {
            if (!*b)
            {
                retVal = arg1;
                break;
            }
        }

        if (retVal)
        {
            break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::OCLDevice
// Description: Constructor. Inialize an OCLDevice instance for a given
//              OpenCL platform, device ID and device type.
// ---------------------------------------------------------------------------
OCLDevice::OCLDevice(
    OCLPlatform* platform,
    cl_device_id id,
    cl_device_type type)
{
    _platform = platform;
    _id = id;
    _type = type;
    _name = NULL;
    _vendor = NULL;
    _OCLVendorId = OCL_VENDOR_UNKNOWN;
    _extensions = NULL;
    _versionString = NULL;
    _version = 0.0f;
    _cVersion = NULL;
    _glSharing = false;
    _haveUnifiedMemory = false;
    _canGLAssociate = false;
    _maxWorkItemDimensions = 0;
    _maxWorkItemSizes = NULL;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::OCLDevice
// Description: Deconstructor. Free any memoy owned by this device instance.
// ---------------------------------------------------------------------------
OCLDevice::~OCLDevice()
{
    delete _name;
    delete _vendor;
    delete _extensions;
    delete _versionString;
    delete _cVersion;
    delete [] _maxWorkItemSizes;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::getPlatform
// Description: Get the OCLPlatform instance for this device.
// ---------------------------------------------------------------------------
OCLPlatform* OCLDevice::getPlatform() const
{
    return _platform;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::getDeviceID
// Description: Get the OpenCL device ID for this device.
// ---------------------------------------------------------------------------
cl_device_id OCLDevice::getDeviceID() const
{
    return _id;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::getDeviceType
// Description: Get the OpenCL device type for this device.
// ---------------------------------------------------------------------------
cl_device_type OCLDevice::getDeviceType() const
{
    return _type;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::setName
// Description: Set the OpenCL device name string. This instance owns the
//              memory.
// ---------------------------------------------------------------------------
void OCLDevice::setName(char* str)
{
    if (_name)
    {
        delete _name;
    }

    _name = str;
}
// ---------------------------------------------------------------------------
// Name:        OCLDevice::getName
// Description: Get the OpenCL device name string.
// ---------------------------------------------------------------------------
const char* OCLDevice::getName() const
{
    return _name;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::setVendor
// Description: Set the OpenCL device vendor string. Deduce internal vendor Id
//              from the vendor string.
// ---------------------------------------------------------------------------
void OCLDevice::setVendor(char* str)
{
    if (_vendor)
    {
        delete _vendor;
    }

    _vendor = str;

    if (stristr(str, "intel"))
    {
        _OCLVendorId = OCL_VENDOR_INTEL;
    }
    else if (stristr(str, "advanced micro devices") || stristr(str, "authenticamd"))
    {
        _OCLVendorId = OCL_VENDOR_AMD;
    }
    else
    {
        _OCLVendorId = OCL_VENDOR_UNKNOWN;
    }
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::getVendor
// Description: Get the OpenCL device vendor string.
// ---------------------------------------------------------------------------
const char* OCLDevice::getVendor() const
{
    return _vendor;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::getVendorId
// Description: Get the classes internal vendor Id.
// ---------------------------------------------------------------------------
OCLVendorId OCLDevice::getVendorId() const
{
    return _OCLVendorId;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::setExtensions
// Description: Set the OpenCL device extensions string.
// ---------------------------------------------------------------------------
void OCLDevice::setExtensions(char* str)
{
    if (_extensions)
    {
        delete _extensions;
    }

    _extensions = str;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::getExtensions
// Description: Get the OpenCL device extensions string.
// ---------------------------------------------------------------------------
const char* OCLDevice::getExtensions() const
{
    return _extensions;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::setVersion
// Description: Set the OpenCL version string supported by the device. Deduce
//              and store the float representation of the version string.
// ---------------------------------------------------------------------------
void OCLDevice::setVersion(
    char* str)
{
    float ver;
    int n = sscanf(str, "OpenCL %f", &ver);

    if (n == 1)
    {
        delete _versionString;
        _version = ver;
        _versionString = str;
    }
    else
    {
        delete str;
    }
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::\getVersionString
// Description: Get the OpenCL version string.
// ---------------------------------------------------------------------------
const char* OCLDevice::getVersionString() const
{
    return _versionString;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::getVersion
// Description: Get the OpenCL version as a float.
// ---------------------------------------------------------------------------
float OCLDevice::getVersion() const
{
    return _version;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::setCVersion
// Description: Set the OpenCL C version string supported by the device.
// ---------------------------------------------------------------------------
void OCLDevice::setCVersion(
    char* str)
{
    if (_cVersion)
    {
        delete _cVersion;
    }

    _cVersion = str;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::getCVersion
// Description: Get the OpenCL C version string.
// ---------------------------------------------------------------------------
const char* OCLDevice::getCVersion() const
{
    return _cVersion;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::setHaveUnifiedMemory
// Description: Set whether this device and the host have unified memory.
// ---------------------------------------------------------------------------
void OCLDevice::setHaveUnifiedMemory(
    bool state)
{
    _haveUnifiedMemory = state;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::getHaveUnifiedMemory
// Description: Find out if this device and the host have unified memory.
// ---------------------------------------------------------------------------
bool OCLDevice::getHaveUnifiedMemory() const
{
    return _haveUnifiedMemory;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::setGLSharing
// Description: Set if this device supports GL sharing extension.
// ---------------------------------------------------------------------------
void OCLDevice::setGLSharing(
    bool state)
{
    _glSharing = state;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::getGLSharing
// Description: Find out if this device has the GL sharing extension.
// ---------------------------------------------------------------------------
bool OCLDevice::getGLSharing() const
{
    return _glSharing;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::setGLAssociation
// Description: Set whether or not this device can associate with the current
//              GL context. Update the GL association flag in the platform.
// ---------------------------------------------------------------------------
void OCLDevice::setGLAssociation(
    bool state)
{
    if (state != _canGLAssociate)
    {
        _canGLAssociate = state;

        if (state)
        {
            _platform->incrGLAssociation();
        }
        else
        {
            _platform->decrGLAssociation();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::getGLAssociation
// Description: Find out if this device can be associated with the current
//              GL context.
// ---------------------------------------------------------------------------
bool OCLDevice::getGLAssociation() const
{
    return _canGLAssociate;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::getMaxWorkItemDimensions
// Description: Get the maximum number of work item dimensions.
// ---------------------------------------------------------------------------
unsigned int OCLDevice::getMaxWorkItemDimensions() const
{
    return _maxWorkItemDimensions;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::getMaxWorkItemSize
// Description: Get the maximum number of work items for a given dimension.
// ---------------------------------------------------------------------------
size_t OCLDevice::getMaxWorkItemSize(int dim) const
{
    size_t ret = 0;

    if (_maxWorkItemSizes)
    {
        if (dim >= 0 && dim < static_cast<int>(_maxWorkItemDimensions))
        {
            ret = _maxWorkItemSizes[dim];
        }
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::getMaxWorkGroupSize
// Description: Get the maximum number of total work items in a work group.
// ---------------------------------------------------------------------------
size_t OCLDevice::getMaxWorkGroupSize() const
{
    return _maxWorkGroupSize;
}

// ---------------------------------------------------------------------------
// Name:        OCLDevice::getLocalMemSize
// Description: Get the maximum local memory size that can be used.
// ---------------------------------------------------------------------------
cl_ulong OCLDevice::getLocalMemSize() const
{
    return _localMemSize;
}

// ---------------------------------------------------------------------------
// Name:        OCLPlatform::OCLPlatform
// Description: Constructor. Inialize an OCLPlatform instance for a given
//              OpenCL platform id and given number of devices.
// ---------------------------------------------------------------------------
OCLPlatform::OCLPlatform(
    cl_platform_id id,
    int numDevices)
{
    _id = id;
    _versionString = NULL;
    _name = NULL;
    _vendor = NULL;
    _extensions = NULL;
    _devices = new OCLDevice *[numDevices];
    _numDevices = 0;
    _version = 0.0f;
    _glSharing = false;
    _glAssociation = 0;
    _OCLVendorId = OCL_VENDOR_UNKNOWN;
}

// ---------------------------------------------------------------------------
// Name:        OCLPlatform::~OCLPlatform
// Description: Deconstructor. Release all memory resources.
// ---------------------------------------------------------------------------
OCLPlatform::~OCLPlatform()
{
    delete _versionString;
    delete _name;
    delete _vendor;
    delete _extensions;

    for (int i = _numDevices - 1; i >= 0; --i)
    {
        delete _devices[i];
    }

    delete [] _devices;
}

// ---------------------------------------------------------------------------
// Name:        OCLPlatform::getPlatformID
// Description: Return the OpenCL platform ID
// ---------------------------------------------------------------------------
cl_platform_id OCLPlatform::getPlatformID() const
{
    return _id;
}

// ---------------------------------------------------------------------------
// Name:        OCLPlatform::setVersion
// Description: Set the OpenCL version string supported by this platform and
//              decompose and store as a float.
// ---------------------------------------------------------------------------
void OCLPlatform::setVersion(
    char* str)
{
    float ver;
    int n = sscanf(str, "OpenCL %f", &ver);

    if (n == 1)
    {
        delete _versionString;
        _version = ver;
        _versionString = str;
    }
    else
    {
        delete str;
    }
}
// ---------------------------------------------------------------------------
// Name:        OCLPlatform::getVersionString
// Description: Get the OpenCL version string supported by this platform.
// ---------------------------------------------------------------------------
const char* OCLPlatform::getVersionString() const
{
    return _versionString;
}
// ---------------------------------------------------------------------------
// Name:        OCLPlatform::getVersion
// Description: Get the OpenCL version as a float.
// ---------------------------------------------------------------------------
float OCLPlatform::getVersion() const
{
    return _version;
}

// ---------------------------------------------------------------------------
// Name:        OCLPlatform::setName
// Description: Set the OpenCL name of this platform.
// ---------------------------------------------------------------------------
void OCLPlatform::setName(
    char* str)
{
    if (_name)
    {
        delete _name;
    }

    _name = str;
}
// ---------------------------------------------------------------------------
// Name:        OCLPlatform::getName
// Description: Get the OpenCL name of this platform.
// ---------------------------------------------------------------------------
const char* OCLPlatform::getName() const
{
    return _name;
}

// ---------------------------------------------------------------------------
// Name:        OCLPlatform::setVendor
// Description: Set the OpenCL vendor string of this platform and deduce the
//              the corresponding internal vendor ID.
// ---------------------------------------------------------------------------
void OCLPlatform::setVendor(
    char* str)
{
    if (_vendor)
    {
        delete _vendor;
    }

    _vendor = str;

    if (stristr(str, "intel"))
    {
        _OCLVendorId = OCL_VENDOR_INTEL;
    }
    else if (stristr(str, "advanced micro devices"))
    {
        _OCLVendorId = OCL_VENDOR_AMD;
    }
    else
    {
        _OCLVendorId = OCL_VENDOR_UNKNOWN;
    }
}

// ---------------------------------------------------------------------------
// Name:        OCLPlatform::getVendor
// Description: Get the OpenCL vendor string for this platform.
// ---------------------------------------------------------------------------
const char* OCLPlatform::getVendor() const
{
    return _vendor;
}

// ---------------------------------------------------------------------------
// Name:        OCLPlatform::getVendorId
// Description: Get the internal vendor ID for this platform.
// ---------------------------------------------------------------------------
OCLVendorId OCLPlatform::getVendorId() const
{
    return _OCLVendorId;
}

// ---------------------------------------------------------------------------
// Name:        OCLPlatform::setExtensions
// Description: Set the OpenCL extensions string supported by this platform.
// ---------------------------------------------------------------------------
void OCLPlatform::setExtensions(
    char* str)
{
    if (_extensions)
    {
        delete _extensions;
    }

    _extensions = str;
}

// ---------------------------------------------------------------------------
// Name:        OCLPlatform::getExtensions
// Description: Get the OpenCL extensions string supported by this platform.
// ---------------------------------------------------------------------------
const char* OCLPlatform::getExtensions() const
{
    return _extensions;
}

// ---------------------------------------------------------------------------
// Name:        OCLPlatform::addDevice
// Description: Add an OCLDevice to this platform.
// ---------------------------------------------------------------------------
void OCLPlatform::addDevice(
    OCLDevice* device)
{
    _devices[_numDevices++] = device;
}

// ---------------------------------------------------------------------------
// Name:        OCLPlatform::getNumDevices
// Description: Get the number of devices in this platform.
// ---------------------------------------------------------------------------
int OCLPlatform::getNumDevices() const
{
    return _numDevices;
}

// ---------------------------------------------------------------------------
// Name:        OCLPlatform::getDevice
// Description: Get an OCLDevice reference.
// ---------------------------------------------------------------------------
OCLDevice* OCLPlatform::getDevice(
    int index) const
{
    OCLDevice* retVal = NULL;

    if (index >= 0 && index < _numDevices)
    {
        retVal = _devices[index];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        OCLPlatform::setGLSharing
// Description: Set the flag for support of the GL sharing extension.
// ---------------------------------------------------------------------------
void OCLPlatform::setGLSharing(
    bool state)
{
    _glSharing = state;
}
// ---------------------------------------------------------------------------
// Name:        OCLPlatform::getGLSharing
// Description: Find out if this platform has the GL sharing extension.
// ---------------------------------------------------------------------------
bool OCLPlatform::getGLSharing() const
{
    return _glSharing;
}

// ---------------------------------------------------------------------------
// Name:        OCLPlatform::getGLAssociation
// Description: Find out if any devices in the platform can be associated with
//              the current GL context.
// ---------------------------------------------------------------------------
bool OCLPlatform::getGLAssociation() const
{
    return (_glAssociation > 0);
}

// ---------------------------------------------------------------------------
// Name:        OCLPlatform::incrGLAssociation
// Description: Increment the number of devices supporting GL association.
// ---------------------------------------------------------------------------
void OCLPlatform::incrGLAssociation()
{
    if (_glAssociation < _numDevices)
    {
        ++_glAssociation;
    }
}
// ---------------------------------------------------------------------------
// Name:        OCLPlatform::decrGLAssociation
// Description: Decrement the number of devices supporting GL association.
// ---------------------------------------------------------------------------
void OCLPlatform::decrGLAssociation()
{
    if (_glAssociation > 0)
    {
        --_glAssociation;
    }
}

// ---------------------------------------------------------------------------
// Name:        OCLInfo::OCLInfo
// Description: Constructor. Inialize an OCLInfo instance for a given
//              number of platforms.
// ---------------------------------------------------------------------------
OCLInfo::OCLInfo(
    int numPlatforms)
{
    _platforms = new OCLPlatform *[numPlatforms];
    _numPlatforms = 0;
}

// ---------------------------------------------------------------------------
// Name:        OCLInfo::~OCLInfo
// Description: Deconstructor. Free all platform/device resources.
// ---------------------------------------------------------------------------
OCLInfo::~OCLInfo()
{
    for (int i = _numPlatforms - 1; i >= 0; --i)
    {
        delete _platforms[i];
    }

    delete [] _platforms;
}

// ---------------------------------------------------------------------------
// Name:        OCLInfo::addPlatform
// Description: Add an OCLPlatform instance.
// ---------------------------------------------------------------------------
void OCLInfo::addPlatform(
    OCLPlatform* platform)
{
    _platforms[_numPlatforms++] = platform;
}

// ---------------------------------------------------------------------------
// Name:        OCLInfo::getNumPlatforms
// Description: Get the number of platforms.
// ---------------------------------------------------------------------------
int OCLInfo::getNumPlatforms() const
{
    return _numPlatforms;
}

// ---------------------------------------------------------------------------
// Name:        OCLInfo::getPlatform
// Description: Get an OCLPlatform instance.
// ---------------------------------------------------------------------------
OCLPlatform* OCLInfo::getPlatform(
    int index) const
{
    OCLPlatform* retVal = NULL;

    if (index >= 0 && index < _numPlatforms)
    {
        retVal = _platforms[index];
    }

    return retVal;
}
