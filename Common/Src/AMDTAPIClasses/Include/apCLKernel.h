//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLKernel.h
///
//==================================================================================

//------------------------------ apCLKernel.h ------------------------------

#ifndef __APCLKERNEL_H
#define __APCLKERNEL_H

// CL:
#include <CL/cl.h>

// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apAllocatedObject.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLKernel : public apAllocatedObject
// General Description:
//   Represents an OpenCL kernel object.
//
// Author:  AMD Developer Tools Team
// Creation Date:        22/11/2009
// ----------------------------------------------------------------------------------
class AP_API apCLKernel : public apAllocatedObject
{
public:

    // Self functions:
    apCLKernel();
    apCLKernel(oaCLProgramHandle programHandle, int programIndex, oaCLKernelHandle kernelHandle, const gtString& kernelName);
    virtual ~apCLKernel();

    struct apKernelArgInfo
    {
        gtString _argName;
        gtString _argTypeName;
        cl_kernel_arg_address_qualifier _addressQualifier;
        cl_kernel_arg_access_qualifier _accessQualifier;
        cl_kernel_arg_type_qualifier _argTypeQualifier;
    };

    struct apKernelWorkgroupInfo
    {
        gtInt32 m_deviceId;
        gtUInt64 m_maxWorkgroupSize;
        gtUInt64 m_requiredWorkGroupSize[3];
        gtUInt64 m_requiredLocalMemorySize;
    };

    // Program Handle:
    oaCLProgramHandle programHandle() const {return _hProgram;};

    // Program index:
    int programIndex() const {return _programIndex;};

    // Kernel handle:
    oaCLKernelHandle kernelHandle() const {return _hKernel;}

    // Kernel name;
    const gtString& kernelFunctionName() const {return _kernelFunctionName;};

    // Kernel number of arguments:
    int numArgs() const {return _numArgs;};
    void setNumArgs(int numArguments) {_numArgs = numArguments;};

    void clearArgumentsInfo() {_kernelArgsInfo.clear();};
    void addKernelArgumentInfo(cl_kernel_arg_address_qualifier addressQualifier, cl_kernel_arg_access_qualifier accessQualifier, cl_kernel_arg_type_qualifier argTypeQualifier, const gtASCIIString& argTypeName, const gtASCIIString& argName);
    int amountOfKernelArgsInfos() const {return (int)_kernelArgsInfo.size();};
    bool getKernelArgsInfoAsString(int index, gtString& argInfoAsStr) const;

    void clearWorkgroupInfo() { m_kernelWorkgroupInfo.clear(); };
    void addKernelWorkgroupInfo(gtInt32 deviceId, gtUInt64 maxWorkgroupSize, gtUInt64 requiredWorkGroupSizeX, gtUInt64 requiredWorkGroupSizeY, gtUInt64 requiredWorkGroupSizeZ, gtUInt64 requiredLocalMemorySize);
    int amountOfKernelWorkgroupInfos() const { return (int)m_kernelWorkgroupInfo.size(); };
    bool getKernelWorkgroupInfo(int wgInfoIndex, gtInt32& deviceId, gtUInt64& maxWorkgroupSize, gtUInt64& requiredWorkGroupSizeX, gtUInt64& requiredWorkGroupSizeY, gtUInt64& requiredWorkGroupSizeZ, gtUInt64& requiredLocalMemorySize) const;

    // Kernel reference count:
    int referenceCount() const {return _referenceCount;};
    void setRefCount(int refCount) {_referenceCount = refCount;};

    // Kernel context handle:
    oaCLContextHandle contextHandle() const {return _hContext;};
    void setContextHandle(oaCLContextHandle handle) {_hContext = handle;};

    // cl_gremedy_object_naming:
    const gtString& kernelName() const {return _kernelName;};
    void setKernelName(const gtString& name) {_kernelName = name;};

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:
    // Handle & index for the OpenCL program containing this kernel:
    oaCLProgramHandle _hProgram;
    int _programIndex;

    // Handle to the OpenCL kernel:
    oaCLKernelHandle _hKernel;

    // A function name in the program, declared with the __kernel qualifier, which this kernel
    // object encapsulates:
    gtString _kernelFunctionName;

    // Handle the kernel OpenCL context:
    oaCLContextHandle _hContext;

    // Was the kernel deleted?
    bool _wasMarkedForDeletion;

    // Amount of kernel args:
    unsigned int _numArgs;

    // Kernel reference count:
    unsigned int _referenceCount;

    // Kernel workgroup size:
    size_t _workgroupSize;

    // cl_gremedy_object_naming:
    gtString _kernelName;

    // Kernel arguments info:
    gtVector<apKernelArgInfo> _kernelArgsInfo;

    // Kernel workgroup info:
    gtVector<apKernelWorkgroupInfo> m_kernelWorkgroupInfo;
};



#endif //__APCLKERNEL_H

