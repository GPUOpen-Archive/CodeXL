//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file defines internal functions
//==============================================================================

#ifndef _CL_INTERNAL_FUNCTION_DEFS_H_
#define _CL_INTERNAL_FUNCTION_DEFS_H_

#include <CL/opencl.h>
#include <CL/internal/cl_kernel_info_amd.h>

// AMD Internal extension for querying SC stats
typedef CL_API_ENTRY cl_int
(CL_API_CALL* clGetKernelInfoAMDProc)(
    cl_kernel           /* kernel */,
    cl_device_id        /* device */,
    cl_kernel_info_amd  /* param_name */,
    size_t              /* param_value_size */,
    void*               /* param_value */,
    size_t*             /* param_value_size_ret */);

//------------------------------------------------------------------------------------
// OpenCL AMD Extension access class
//------------------------------------------------------------------------------------
class clExtAMDDispatchTable
{
public:
    /// Get singleton instance
    /// \return a pointer to m_instance
    static clExtAMDDispatchTable* Instance();

public:
    /// A pointer to AMD Internal extension for querying SC stats
    clGetKernelInfoAMDProc GetKernelInfoAMD;
    /// Add more internal function pointers here...
    /// ...
private:
    /// Constructor
    clExtAMDDispatchTable();

    static clExtAMDDispatchTable m_instance;  ///< Static instance of clExtAMDDispatchTable
};

#endif //_CL_INTERNAL_FUNCTION_DEFS_H_
