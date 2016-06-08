//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLKernel.cpp
///
//==================================================================================

//------------------------------ apCLKernel.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTAPIClasses/Include/apCLKernel.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>

// ---------------------------------------------------------------------------
// Name:        apCLKernel::apCLKernel
// Description: Default Constructor
// Author:  AMD Developer Tools Team
// Date:        4/5/2010
// ---------------------------------------------------------------------------
apCLKernel::apCLKernel()
    : _hProgram(OA_CL_NULL_HANDLE), _programIndex(-1), _hKernel(OA_CL_NULL_HANDLE), _kernelFunctionName(L""), _hContext(OA_CL_NULL_HANDLE), _wasMarkedForDeletion(false), _numArgs(0), _referenceCount(0), _workgroupSize(0)
{

}


// ---------------------------------------------------------------------------
// Name:        apCLKernel::apCLKernel
// Description: Constructor
// Arguments: programHandle - The handle of the OpenCL program containing this kernel.
//            kernelHandle - The kernel's OpenCL handle.
//            kernelName - A program's function which this kernel object encapsulates.
// Author:  AMD Developer Tools Team
// Date:        22/11/2009
// ---------------------------------------------------------------------------
apCLKernel::apCLKernel(oaCLProgramHandle programHandle, int programIndex, oaCLKernelHandle kernelHandle, const gtString& kernelName)
    : _hProgram(programHandle), _programIndex(programIndex), _hKernel(kernelHandle), _kernelFunctionName(kernelName), _hContext(OA_CL_NULL_HANDLE), _wasMarkedForDeletion(false), _numArgs(0), _referenceCount(0), _workgroupSize(0)

{
}


// ---------------------------------------------------------------------------
// Name:        apCLKernel::~apCLKernel
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        22/11/2009
// ---------------------------------------------------------------------------
apCLKernel::~apCLKernel()
{
}


// ---------------------------------------------------------------------------
// Name:        apCLKernel::type
// Description: Returns this transferable object type: OS_TOBJ_ID_CL_KERNEL.
// Author:  AMD Developer Tools Team
// Date:        22/11/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLKernel::type() const
{
    return OS_TOBJ_ID_CL_KERNEL;
}


// ---------------------------------------------------------------------------
// Name:        apCLKernel::writeSelfIntoChannel
// Description: Writes this object's data into an IPC channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        22/11/2009
// ---------------------------------------------------------------------------
bool apCLKernel::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt64)_hProgram;
    ipcChannel << (gtInt32)_programIndex;
    ipcChannel << (gtUInt64)_hKernel;
    ipcChannel << _kernelFunctionName;
    ipcChannel << (gtUInt64)_hContext;
    ipcChannel << _wasMarkedForDeletion;
    ipcChannel << (gtUInt32)_numArgs;
    ipcChannel << (gtUInt32)_referenceCount;
    ipcChannel << (gtUInt64)_workgroupSize;
    ipcChannel << _kernelName;

    int kernelArgInfoCount = (int)_kernelArgsInfo.size();
    ipcChannel << (gtUInt32)kernelArgInfoCount;

    for (int i = 0; i < kernelArgInfoCount; i++)
    {
        const apKernelArgInfo& info = _kernelArgsInfo[i];
        ipcChannel << info._argName;
        ipcChannel << info._argTypeName;
        ipcChannel << (gtUInt32)info._addressQualifier;
        ipcChannel << (gtUInt32)info._accessQualifier;
        ipcChannel << (gtUInt32)info._argTypeQualifier;
    }

    int kernelWorkGroupInfoCount = (int)m_kernelWorkgroupInfo.size();
    ipcChannel << (gtUInt32)kernelWorkGroupInfoCount;

    for (int i = 0; i < kernelWorkGroupInfoCount; ++i)
    {
        const apKernelWorkgroupInfo& wgInfo = m_kernelWorkgroupInfo[i];
        ipcChannel << wgInfo.m_deviceId;
        ipcChannel << wgInfo.m_maxWorkgroupSize;
        ipcChannel << wgInfo.m_requiredWorkGroupSize[0];
        ipcChannel << wgInfo.m_requiredWorkGroupSize[1];
        ipcChannel << wgInfo.m_requiredWorkGroupSize[2];
        ipcChannel << wgInfo.m_requiredLocalMemorySize;
    }

    // Write the allocated object Info:
    apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLKernel::readSelfFromChannel
// Description: Reads this object's data from an IPC channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        22/11/2009
// ---------------------------------------------------------------------------
bool apCLKernel::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt64 hProgramAsUint64 = 0;
    ipcChannel >> hProgramAsUint64;
    _hProgram = (oaCLProgramHandle)hProgramAsUint64;

    gtInt32 programIndexAsint32 = 0;
    ipcChannel >> programIndexAsint32;
    _programIndex = (int)programIndexAsint32;

    gtUInt64 hKernelAsUint64 = 0;
    ipcChannel >> hKernelAsUint64;
    _hKernel = (oaCLKernelHandle)hKernelAsUint64;

    ipcChannel >> _kernelFunctionName;

    gtUInt64 hContextAsUint64 = 0;
    ipcChannel >> hContextAsUint64;
    _hContext = (oaCLContextHandle)hContextAsUint64;

    ipcChannel >> _wasMarkedForDeletion;

    gtUInt32 numArgsUint32 = 0;
    ipcChannel >> numArgsUint32;
    _numArgs = (unsigned int)numArgsUint32;

    gtUInt32 referenceCountUint32 = 0;
    ipcChannel >> referenceCountUint32;
    _referenceCount = (unsigned int)referenceCountUint32 ;

    gtUInt64 workgroupSizeUint64 = 0;
    ipcChannel >> workgroupSizeUint64;
    _workgroupSize = (size_t)workgroupSizeUint64 ;

    ipcChannel >> _kernelName;

    // Read the kernel arg info:
    clearArgumentsInfo();
    gtUInt32 kernelArgInfoCount = 0;
    ipcChannel >> kernelArgInfoCount;

    for (gtUInt32 i = 0; i < kernelArgInfoCount; i++)
    {
        apKernelArgInfo info;
        ipcChannel >> info._argName;
        ipcChannel >> info._argTypeName;

        gtUInt32 varAsUInt32;
        ipcChannel >> varAsUInt32;
        info._addressQualifier = (cl_kernel_arg_address_qualifier)varAsUInt32;

        ipcChannel >> varAsUInt32;
        info._accessQualifier = (cl_kernel_arg_access_qualifier)varAsUInt32;

        ipcChannel >> varAsUInt32;
        info._argTypeQualifier = (cl_kernel_arg_type_qualifier)varAsUInt32;

        _kernelArgsInfo.push_back(info);
    }

    clearWorkgroupInfo();
    gtUInt32 workgroupInfoCount = 0;
    ipcChannel >> workgroupInfoCount;

    m_kernelWorkgroupInfo.resize(workgroupInfoCount);

    for (gtUInt32 i = 0; i < workgroupInfoCount; ++i)
    {
        apKernelWorkgroupInfo& wgInfo = m_kernelWorkgroupInfo[i];
        ipcChannel >> wgInfo.m_deviceId;
        ipcChannel >> wgInfo.m_maxWorkgroupSize;
        ipcChannel >> wgInfo.m_requiredWorkGroupSize[0];
        ipcChannel >> wgInfo.m_requiredWorkGroupSize[1];
        ipcChannel >> wgInfo.m_requiredWorkGroupSize[2];
        ipcChannel >> wgInfo.m_requiredLocalMemorySize;
    }

    // Read the allocated object Info:
    apAllocatedObject::readSelfFromChannel(ipcChannel);

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLKernel::addKernelArgumentInfo
// Description: Add kernel argument info
// Arguments:   cl_kernel_arg_address_qualifier addressQualifier
//              cl_kernel_arg_access_qualifier accessQualifier
//              cl_kernel_arg_type_qualifier argTypeQualifier
//              const gtASCIIString& argTypeName
//              const gtASCIIString& argName
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        11/1/2012
// ---------------------------------------------------------------------------
void apCLKernel::addKernelArgumentInfo(cl_kernel_arg_address_qualifier addressQualifier, cl_kernel_arg_access_qualifier accessQualifier,
                                       cl_kernel_arg_type_qualifier argTypeQualifier, const gtASCIIString& argTypeName, const gtASCIIString& argName)
{
    apKernelArgInfo info;
    info._addressQualifier = addressQualifier;
    info._accessQualifier = accessQualifier;
    info._argTypeQualifier = argTypeQualifier;
    info._argTypeName.fromASCIIString(argTypeName.asCharArray());
    info._argName.fromASCIIString(argName.asCharArray());

    _kernelArgsInfo.push_back(info);
}


// ---------------------------------------------------------------------------
// Name:        apCLKernel::getKernelArgsInfoAsString
// Description: Get the kernel argument info as string
// Arguments:   int index
//              gtString& argInfoAsStr
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/1/2012
// ---------------------------------------------------------------------------
bool apCLKernel::getKernelArgsInfoAsString(int index, gtString& argInfoAsStr) const
{
    bool retVal = false;
    // Sanity check
    GT_IF_WITH_ASSERT((index >= 0) && (index < (int)_kernelArgsInfo.size()))
    {
        retVal = true;

        // Get the current kernel arg info:
        apKernelArgInfo info = _kernelArgsInfo[index];
        argInfoAsStr.makeEmpty();


        // Add the argument type qualifier:
        if (info._argTypeQualifier != CL_KERNEL_ARG_TYPE_NONE)
        {
            argInfoAsStr.append(L"(");

            if (info._argTypeQualifier & CL_KERNEL_ARG_TYPE_CONST)
            {
                argInfoAsStr.append(AP_STR_clKernelArgTypeQualifierConst AP_STR_Space);
            }

            if (info._argTypeQualifier & CL_KERNEL_ARG_TYPE_RESTRICT)
            {
                argInfoAsStr.append(AP_STR_clKernelArgTypeQualifierRestrict AP_STR_Space);
            }

            if (info._argTypeQualifier & CL_KERNEL_ARG_TYPE_VOLATILE)
            {
                argInfoAsStr.append(AP_STR_clKernelArgTypeQualifierVolatile AP_STR_Space);
            }

            if (info._argTypeQualifier & CL_KERNEL_ARG_TYPE_PIPE)
            {
                argInfoAsStr.append(AP_STR_clKernelArgTypeQualifierPipe AP_STR_Space);
            }

            argInfoAsStr.append(L")");
        }

        // Add the arg access qualifier (read/write):
        switch (info._accessQualifier)
        {

            case CL_KERNEL_ARG_ACCESS_READ_ONLY:
            {
                argInfoAsStr.append(AP_STR_clKernelArgAccessQualifierReadOnly AP_STR_Space);
                break;
            }

            case CL_KERNEL_ARG_ACCESS_WRITE_ONLY:
            {
                argInfoAsStr.append(AP_STR_clKernelArgAccessQualifierWriteOnly AP_STR_Space);
                break;
            }

            case CL_KERNEL_ARG_ACCESS_READ_WRITE:
            {
                argInfoAsStr.append(AP_STR_clKernelArgAccessQualifierReadWrite AP_STR_Space);
                break;
            }

            default:
            {
                break;
            }
        }

        switch (info._addressQualifier)
        {
            case CL_KERNEL_ARG_ADDRESS_GLOBAL:
            {
                argInfoAsStr.append(AP_STR_clKernelArgAddressQualifierGlobal AP_STR_Space);
                break;
            }

            case CL_KERNEL_ARG_ADDRESS_LOCAL:
            {
                argInfoAsStr.append(AP_STR_clKernelArgAddressQualifierLocal AP_STR_Space);
                break;
            }

            case CL_KERNEL_ARG_ADDRESS_CONSTANT:
            {
                argInfoAsStr.append(AP_STR_clKernelArgAddressQualifierConstant AP_STR_Space);
                break;
            }

            case CL_KERNEL_ARG_ADDRESS_PRIVATE:
            {
                argInfoAsStr.append(AP_STR_clKernelArgAddressQualifierPrivate AP_STR_Space);
                break;
            }

            default:
            {
                break;
            }
        }

        argInfoAsStr.append(info._argTypeName);
        argInfoAsStr.append(AP_STR_Space);
        argInfoAsStr.append(info._argName);

    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLKernel::addKernelWorkgroupInfo
// Description: Adds a set of work group info for a device
// Arguments:   int deviceId - our device name
//              gtUInt64 maxWorkgroupSize - max flattened work-group size by device limits (registers, wave size, etc)
//              gtUInt64 requiredWorkGroupSizeX - required work-group size by kernel __attribute__((reqd_work_group_size(x, y, z)))
//              gtUInt64 requiredWorkGroupSizeY - see above
//              gtUInt64 requiredWorkGroupSizeZ - see above
//              gtUInt64 requiredLocalMemorySize - local memory size used by the kernel
// Author:  AMD Developer Tools Team
// Date:        1/7/2015
// ---------------------------------------------------------------------------
void apCLKernel::addKernelWorkgroupInfo(gtInt32 deviceId, gtUInt64 maxWorkgroupSize, gtUInt64 requiredWorkGroupSizeX, gtUInt64 requiredWorkGroupSizeY, gtUInt64 requiredWorkGroupSizeZ, gtUInt64 requiredLocalMemorySize)
{
    apKernelWorkgroupInfo wgInfo;
    wgInfo.m_deviceId = deviceId;
    wgInfo.m_maxWorkgroupSize = maxWorkgroupSize;
    wgInfo.m_requiredWorkGroupSize[0] = requiredWorkGroupSizeX;
    wgInfo.m_requiredWorkGroupSize[1] = requiredWorkGroupSizeY;
    wgInfo.m_requiredWorkGroupSize[2] = requiredWorkGroupSizeZ;
    wgInfo.m_requiredLocalMemorySize = requiredLocalMemorySize;
    m_kernelWorkgroupInfo.push_back(wgInfo);
}

// ---------------------------------------------------------------------------
// Name:        apCLKernel::addKernelWorkgroupInfo
// Description: Gets a set of work group info for a device
// Arguments:   int wgInfoIndex - index in the vector
//              int deviceId - our device name
//              gtUInt64 maxWorkgroupSize - max flattened work-group size by device limits (registers, wave size, etc)
//              gtUInt64 requiredWorkGroupSizeX - required work-group size by kernel __attribute__((reqd_work_group_size(x, y, z)))
//              gtUInt64 requiredWorkGroupSizeY - see above
//              gtUInt64 requiredWorkGroupSizeZ - see above
//              gtUInt64 requiredLocalMemorySize - local memory size used by the kernel
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        1/7/2015
// ---------------------------------------------------------------------------
bool apCLKernel::getKernelWorkgroupInfo(int wgInfoIndex, gtInt32& deviceId, gtUInt64& maxWorkgroupSize, gtUInt64& requiredWorkGroupSizeX, gtUInt64& requiredWorkGroupSizeY, gtUInt64& requiredWorkGroupSizeZ, gtUInt64& requiredLocalMemorySize) const
{
    bool retVal = false;

    if ((0 <= wgInfoIndex) && ((int)m_kernelWorkgroupInfo.size() > wgInfoIndex))
    {
        retVal = true;
        const apKernelWorkgroupInfo& wgInfo = m_kernelWorkgroupInfo[wgInfoIndex];
        deviceId = wgInfo.m_deviceId;
        maxWorkgroupSize = wgInfo.m_maxWorkgroupSize;
        requiredWorkGroupSizeX = wgInfo.m_requiredWorkGroupSize[0];
        requiredWorkGroupSizeY = wgInfo.m_requiredWorkGroupSize[1];
        requiredWorkGroupSizeZ = wgInfo.m_requiredWorkGroupSize[2];
        requiredLocalMemorySize = wgInfo.m_requiredLocalMemorySize;
    }

    return retVal;
}

