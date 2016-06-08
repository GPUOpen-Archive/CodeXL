//=====================================================================
// Copyright (c) 2010 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/Common/Lib/AMD/OpenCLDebugAPI/1.3/Include/CLDispatchTable.h $
/// \version $Revision: #9 $
/// \brief Functions to get and set the CL function entry points to be used by the debugger.
//
//=====================================================================
// $Id: //devtools/main/Common/Lib/AMD/OpenCLDebugAPI/1.3/Include/CLDispatchTable.h#9 $
// Last checkin:   $DateTime: 2014/10/01 21:09:09 $
// Last edited by: $Author: chesik $
// Change list:    $Change: 506204 $
//=====================================================================

#ifndef CL_DISPATCH_TABLE_H
#define CL_DISPATCH_TABLE_H

#include <CL/cl.h>

#include <CL/cl_ext.h>
#include <CL/internal/cl_icd_amd.h>


#ifdef __cplusplus
extern "C" {
#endif

/// Function to get the current CL function entry points being used by the debug API
/// \param[out] pDispatchTable  Pointer to copy the table of function entry points being used to.
/// \return                     Returns CL_SUCCESS if the function executed successfully.
///                             Otherwise, it returns a CL error code.
extern CL_API_ENTRY cl_int CL_API_CALL
amdclDebugGetDispatchTable( cl_icd_dispatch_table* pCLDispatchTable );

/// Function to set the current CL function entry points to be used by the debug API
/// \param[in] pDispatchTable  Pointer to the table of function entry points to use.
/// \return                    Returns CL_SUCCESS if the function executed successfully.
///                            Otherwise, it returns a CL error code.
extern CL_API_ENTRY cl_int CL_API_CALL
amdclDebugSetDispatchTable( const cl_icd_dispatch_table* pCLDispatchTable );

#ifdef __cplusplus
}
#endif

#endif // CL_DISPATCH_TABLE_H
