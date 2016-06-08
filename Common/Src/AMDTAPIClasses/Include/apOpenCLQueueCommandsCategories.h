//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenCLQueueCommandsCategories.h
///
//==================================================================================

//------------------------------ apOpenCLQueueCommandsCategories.h ------------------------------

#ifndef __APOPENCLQUEUECOMMANDSCATEGORIES_H
#define __APOPENCLQUEUECOMMANDSCATEGORIES_H

// Forward declarations:
class gtString;

// Infra:
#include <AMDTOSWrappers/Include/osTransferableObjectType.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>


// Enumeration that groups OpenCL queue commands into categories
enum apOpenCLQueueCommandsCategories
{
    AP_KERNEL_QUEUE_COMMANDS,       // Kernel related queue commands.
    AP_WRITE_QUEUE_COMMANDS,        // Write related queue commands.
    AP_COPY_QUEUE_COMMANDS,         // Copy related queue commands.
    AP_READ_QUEUE_COMMANDS,         // Read related queue commands.
    AP_OTHER_QUEUE_COMMANDS,        // All other queue commands.

    AP_NUMBER_OF_QUEUE_COMMAND_CATEGORIES
};


AP_API apOpenCLQueueCommandsCategories apOpenCLQueueCommandsCategoryFromTransferableObjectType(osTransferableObjectType tobjType);
AP_API void apOpenCLQueueCommandsCategoryToString(apOpenCLQueueCommandsCategories category, gtString& categoryName);


#endif //__APOPENCLQUEUECOMMANDSCATEGORIES_H

