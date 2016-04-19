//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csCLKernel.h
///
//==================================================================================

//------------------------------ csCLKernel.h ------------------------------

#ifndef __CSCLKERNEL_H
#define __CSCLKERNEL_H

// Forward declarations:
class gtString;

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTAPIClasses/Include/apCLKernel.h>


// ----------------------------------------------------------------------------------
// Class Name:          csCLKernel : public apCLKernel
// General Description: Used to hold OpenCL-server only data about OpenCL kernels
//                      (parameters, etc, that we don't want to pass through the API each time)
// Author:              Uri Shomroni
// Creation Date:       18/1/2010
// ----------------------------------------------------------------------------------
class csCLKernel : public apCLKernel
{
public:
    csCLKernel(oaCLProgramHandle programHandle, int programIndex, oaCLKernelHandle kernelHandle, const gtString& kernelName);
    virtual ~csCLKernel();

    void onArgumentSet(cl_uint argIndex, gtSizeType argSize, const void* argValue, bool isSVMPointer);
    bool restoreKernelArguments(oaCLKernelHandle kernelInternalHandle) const;
    bool kernelHasSVMArguments() const;
    bool kernelUsesSVMPointers() const {return m_kernelUsesSVMPointers;};

private:
    // Disallow use of my default constructor:
    csCLKernel();

private:
    struct kernelArgumentData
    {
        gtSizeType _argumentSize;
        gtUByte* _argumentData;
        bool m_isSVMPointer;

        kernelArgumentData() : _argumentSize(0), _argumentData(NULL), m_isSVMPointer(false) {};
        kernelArgumentData(const kernelArgumentData& other) : _argumentSize(0), _argumentData(NULL), m_isSVMPointer(false) {operator=(other);};
        ~kernelArgumentData() {delete[] _argumentData;};
        kernelArgumentData& operator=(const kernelArgumentData& other);
    };

private:
    // Maps the kernel arguments indices to the arguments:
    gtMap<unsigned int, kernelArgumentData> _kernelArguments;
    bool m_kernelUsesSVMPointers;
};

#endif //__CSCLKERNEL_H

