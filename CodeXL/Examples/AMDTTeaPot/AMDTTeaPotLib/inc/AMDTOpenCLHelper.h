//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTOpenCLHelper.h
///
//==================================================================================

//------------------------------ AMDTOpenCLHelper.h ------------------------------

#ifndef __AMDTOPENCLHELPER_H
#define __AMDTOPENCLHELPER_H

// Platform specific includes:
#if defined (__APPLE__)
    #include <dlfcn.h>
#elif defined(__linux__)
    #include <signal.h>
#elif defined(_WINDOWS) || defined(_WIN32)
    #include <Windows.h>
#endif

// Forward decelerations:

// OpenCL:
#if defined(__APPLE__) || defined(__MACOSX)
    #include <OpenCL/cl.h>
#else
    #include <CL/cl.h>
    #include <CL/cl_ext.h>
    #include <CL/cl_gl.h>
    #include <CL/cl_icd.h>
#endif

// Local definitions
typedef void (*oclProcedureAddress)(void);

/******************************************************************************
 *
 * OCLInfo/OCLPlatform/OCLDevice are used to construct of tree of information
 * about all available OpenCL platforms and devices.
 *
 * OCLInfo is an array of OCLPlatform instances. OCLPlatform is an array of
 * OCLDevice instance. OCLPlatform and OCLDevice contain useful information
 * that was extracted from the OpenCL runtime such as name, vendor,
 * GL sharing/association capabilities.
 *
 ******************************************************************************/
enum OCLVendorId
{
    OCL_VENDOR_UNKNOWN,
    OCL_VENDOR_INTEL,
    OCL_VENDOR_AMD
};
class OCLPlatform;
class OCLDevice
{
    friend class AMDTOpenCLHelper;

private:
    OCLDevice();
    OCLDevice(const OCLDevice& platform) { (void)(platform); }
    OCLDevice& operator=(const OCLDevice& lhs)
    {
        (void)(lhs);
        return *this;
    }

private:
    OCLPlatform* _platform;
    cl_device_id _id;
    cl_device_type _type;
    char* _name;
    char* _vendor;
    OCLVendorId _OCLVendorId;
    char* _extensions;
    char* _versionString;
    char* _cVersion;
    float _version;
    bool _glSharing;
    bool _haveUnifiedMemory;
    bool _canGLAssociate;
    unsigned int _maxWorkItemDimensions;
    size_t* _maxWorkItemSizes;
    size_t _maxWorkGroupSize;
    cl_ulong _localMemSize;

public:
    OCLDevice(OCLPlatform* platform, cl_device_id id, cl_device_type type);
    ~OCLDevice();

    OCLPlatform* getPlatform() const;
    cl_device_id getDeviceID() const;
    cl_device_type getDeviceType() const;
    void setName(char* str);
    const char* getName() const;
    void setVendor(char* str);
    const char* getVendor() const;
    OCLVendorId getVendorId() const;
    void setExtensions(char* str);
    const char* getExtensions() const;
    void setVersion(char* str);
    const char* getVersionString() const;
    float getVersion() const;
    void setCVersion(char* str);
    const char* getCVersion() const;
    void setHaveUnifiedMemory(bool state);
    bool getHaveUnifiedMemory() const;
    void setGLSharing(bool state);
    bool getGLSharing() const;
    void setGLAssociation(bool state);
    bool getGLAssociation() const;
    unsigned int getMaxWorkItemDimensions() const;
    size_t getMaxWorkGroupSize() const;
    cl_ulong  getLocalMemSize() const;
    size_t getMaxWorkItemSize(int dim) const;
};

class OCLPlatform
{
    friend class AMDTOpenCLHelper;

private:
    OCLPlatform();
    OCLPlatform(const OCLPlatform& platform) { (void)(platform); }
    OCLPlatform& operator=(const OCLPlatform& lhs)
    {
        (void)(lhs);
        return *this;
    }

    void incrGLAssociation();
    void decrGLAssociation();

    // Only certain functions in OCLDevice will be able to access
    // private members of OCLPlatform.
    friend void OCLDevice::setGLAssociation(bool state);

private:
    cl_platform_id _id;
    char* _versionString;
    char* _name;
    char* _vendor;
    char* _extensions;
    OCLDevice** _devices;
    int _numDevices;
    int _glAssociation;
    float _version;
    bool _glSharing;
    OCLVendorId _OCLVendorId;

public:
    OCLPlatform(cl_platform_id id, int numDevices);
    ~OCLPlatform();

    cl_platform_id getPlatformID() const;
    void setVersion(char* str);
    const char* getVersionString() const;
    float getVersion() const;
    void setName(char* str);
    const char* getName() const;
    void setVendor(char* str);
    const char* getVendor() const;
    OCLVendorId getVendorId() const;
    void setExtensions(char* str);
    const char* getExtensions() const;
    void addDevice(OCLDevice* device);
    int getNumDevices() const;
    OCLDevice* getDevice(int index) const;
    void setGLSharing(bool state);
    bool getGLSharing() const;
    bool getGLAssociation() const;
};

class OCLInfo
{
    friend class AMDTOpenCLHelper;

private:
    OCLInfo(const OCLInfo& platform) { (void)(platform); }
    OCLInfo& operator=(const OCLInfo& lhs)
    {
        (void)(lhs);
        return *this;
    }
    OCLInfo();

    void addPlatform(OCLPlatform* platform);

private:
    int _numPlatforms;
    OCLPlatform** _platforms;

public:
    OCLInfo(int numPlatforms);
    ~OCLInfo();

    int getNumPlatforms() const;
    OCLPlatform* getPlatform(int index) const;
};

/******************************************************************************
 *
 * AMDTOpenCLHelper
 * --------------
 *
 * This is a singleton class. At present it is not thread-safe. When the
 * singleton is created, it loads the OpenCL library, imports the OpenCL runtime
 * API function pointers (publicly available as members of the instance) and
 * populates the OCLInfo tree with information about available OpenCL platforms
 * and devices.
 *
 ******************************************************************************/
class AMDTOpenCLHelper
{
public:
    // OpenCL function pointers
    clGetExtensionFunctionAddress_fn    _clGetExtensionFunctionAddress;
    clGetPlatformIDs_fn                 _clGetPlatformIDs;
    clGetPlatformInfo_fn                _clGetPlatformInfo;
    clGetDeviceIDs_fn                   _clGetDeviceIDs;
    clGetDeviceInfo_fn                  _clGetDeviceInfo;
    clCreateContext_fn                  _clCreateContext;
    clCreateContextFromType_fn          _clCreateContextFromType;
    clGetContextInfo_fn                 _clGetContextInfo;
    clReleaseContext_fn                 _clReleaseContext;
    clCreateCommandQueue_fn             _clCreateCommandQueue;
    clReleaseCommandQueue_fn            _clReleaseCommandQueue;
    clCreateProgramWithSource_fn        _clCreateProgramWithSource;
    clReleaseProgram_fn                 _clReleaseProgram;
    clBuildProgram_fn                   _clBuildProgram;
    clUnloadCompiler_fn                 _clUnloadCompiler;
    clGetProgramInfo_fn                 _clGetProgramInfo;
    clGetProgramBuildInfo_fn            _clGetProgramBuildInfo;
    clCreateKernel_fn                   _clCreateKernel;
    clReleaseKernel_fn                  _clReleaseKernel;
    clSetKernelArg_fn                   _clSetKernelArg;
    clEnqueueNDRangeKernel_fn           _clEnqueueNDRangeKernel;
    clEnqueueCopyBufferToImage_fn       _clEnqueueCopyBufferToImage;
    clEnqueueMapBuffer_fn               _clEnqueueMapBuffer;
    clEnqueueUnmapMemObject_fn          _clEnqueueUnmapMemObject;
    clCreateBuffer_fn                   _clCreateBuffer;
    clCreateImage2D_fn                  _clCreateImage2D;
    clCreateImage3D_fn                  _clCreateImage3D;
    clReleaseMemObject_fn               _clReleaseMemObject;
    clGetGLContextInfoKHR_fn            _clGetGLContextInfoKHR;
    clGetSupportedImageFormats_fn       _clGetSupportedImageFormats;
    clCreateFromGLBuffer_fn             _clCreateFromGLBuffer;
    clCreateFromGLTexture3D_fn          _clCreateFromGLTexture3D;
    clGetKernelWorkGroupInfo_fn         _clGetKernelWorkGroupInfo;
    clWaitForEvents_fn                  _clWaitForEvents;
    clReleaseEvent_fn                   _clReleaseEvent;
    clEnqueueAcquireGLObjects_fn        _clEnqueueAcquireGLObjects;
    clEnqueueReleaseGLObjects_fn        _clEnqueueReleaseGLObjects;
    clEnqueueWriteBuffer_fn             _clEnqueueWriteBuffer;
    clEnqueueWriteBufferRect_fn         _clEnqueueWriteBufferRect;
    clFlush_fn                          _clFlush;
    clFinish_fn                         _clFinish;
    clGetGLObjectInfo_fn                _clGetGLObjectInfo;

public:
    // Get a reference to the singleton. If this is the first call, the
    // singleton is created.
    static AMDTOpenCLHelper* GetInstance();

    // Remove reference to the singleton. If there are no more references,
    // the singleton is destroyed.
    void ReleaseInstance();

    // Check if the helper has initialized successfully - OpenCL library was
    // loaded, main function pointers were located and OCLInfo was populated.
    bool isReady() const;

    // Get a pointer to OCLInfo.
    const OCLInfo* getOpenCLInfo() const;

    // Query platforms for which devices can be associated with the
    // current OpenGL context. This will update OCLInfo platform/device
    // GL association flags.
    bool updateGLCLBinding();

    // Read kernel source from a file and return an caller-owned string.
    char* readKernelSource(const wchar_t* path);

    // Get string representation of OpenCL error codes.
    const char* getOpenCLErrorCodeStr(cl_int status);

    // Get the last error that occurred.
    const char* getLastError();

private:
    // Make constructor/deconstructor private.
    AMDTOpenCLHelper();
    virtual ~AMDTOpenCLHelper();

    // Map all needed OpenCL functions and extensions.
    bool initialize();

    // Lookup proc address in OpenCL library
    oclProcedureAddress getProcAddress(const char* pProcName);

    // Check if OpenCL is available aside the CodeXL server:
    bool isOpenCLAvailableBypassCodeXLInterception();

    // Check if an OpenCL extension is defined in an extension string.
    bool isExtensionSupported(
        char* extensionName,
        const char* extensionsString);

    // Collect information about all OpenCL platforms/devices on this host
    // and create the OCLInfo structure.
    bool initOpenCLInfo();

    // Helper function to get and store platform string info.
    bool getPlatformInfo(
        cl_platform_id platform,
        cl_platform_info param_name,
        char** out_info);

    // Helper function to get and store device string info.
    bool getDeviceInfo(
        cl_device_id device,
        cl_device_info param_name,
        char** out_info);

    // Set last error for an OpenCL function call failure.
    void setLastError(const char* clFunctionName, cl_int status);

private:
    // Singleton instance.
    static AMDTOpenCLHelper*      _instance;

    // Number of references to this singleton.
    unsigned int                _refCounter;

    // If the OpenCL library is loaded, function pointers collected and
    // OCLInfo successfully populated, this is set to true.
    bool                        _ready;

    // Handle to the OpenCL library
    void* _hOpenCLLibrary;

    // Information about all OpenCL platforms/devices on this host.
    OCLInfo*                    _clInfo;

    // Storage for last error
    char _lastError[1024];
};

#endif //__AMDTOPENCLHELPER_H
