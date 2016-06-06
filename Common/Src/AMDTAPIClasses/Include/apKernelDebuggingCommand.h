//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apKernelDebuggingCommand.h
///
//==================================================================================

//------------------------------ apKernelDebuggingCommand.h ------------------------------

#ifndef __APKERNELDEBUGGINGCOMMAND_H
#define __APKERNELDEBUGGINGCOMMAND_H


// ----------------------------------------------------------------------------------
// Class Name:          apKernelDebuggingCommand
// General Description: Enumerates kernel debugging commands, to be passed with
//                      gaSetKernelDebuggingCommand to the debugging API
// Author:  AMD Developer Tools Team
// Creation Date:       2/11/2010
// ----------------------------------------------------------------------------------
enum apKernelDebuggingCommand
{
    AP_KERNEL_CONTINUE,
    AP_KERNEL_STEP_OVER,
    AP_KERNEL_STEP_OUT,
    AP_KERNEL_STEP_IN,
};

// ----------------------------------------------------------------------------------
// Class Name:          apMultipleKernelDebuggingDispatchMode
// General Description: Enumerates kernel debugging commands, to be passed with
//                      gaSetMultipleKernelDebugDispatchMode to the debugging API
// Author:  AMD Developer Tools Team
// Creation Date:       2/11/2010
// ----------------------------------------------------------------------------------
enum apMultipleKernelDebuggingDispatchMode
{
    AP_MULTIPLE_KERNEL_DISPATCH_CONCURRENT, // Dispatch kernels as they are required, ignoring debugging in progress. May cause conflicts.
    AP_MULTIPLE_KERNEL_DISPATCH_WAIT,       // Wait for all current debugging to finish before checking if the next kernel should be debugged. May cause hangs.
    AP_MULTIPLE_KERNEL_DISPATCH_NO_DEBUG,   // Ignore requests for dispatches and run without
};

#endif //__APKERNELDEBUGGINGCOMMAND_H

