//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenCLQueueCommandsCategories.cpp
///
//==================================================================================

//------------------------------ apOpenCLQueueCommandsCategories.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/apOpenCLQueueCommandsCategories.h>


// ---------------------------------------------------------------------------
// Name:        apOpenCLQueueCommandsCategoryFromTransferableObjectType
// Description: Returns the appropriate command queue category for a command type
// Author:  AMD Developer Tools Team
// Date:        25/2/2010
// ---------------------------------------------------------------------------
apOpenCLQueueCommandsCategories apOpenCLQueueCommandsCategoryFromTransferableObjectType(osTransferableObjectType tobjType)
{
    apOpenCLQueueCommandsCategories retVal = AP_OTHER_QUEUE_COMMANDS;

    switch (tobjType)
    {
        case OS_TOBJ_ID_CL_NATIVE_KERNEL_COMMAND:
        case OS_TOBJ_ID_CL_ND_RANGE_KERNEL_COMMAND:
        case OS_TOBJ_ID_CL_TASK_COMMAND:
            retVal = AP_KERNEL_QUEUE_COMMANDS;
            break;

        case OS_TOBJ_ID_CL_WRITE_BUFFER_COMMAND:
        case OS_TOBJ_ID_CL_WRITE_BUFFER_RECT_COMMAND:
        case OS_TOBJ_ID_CL_WRITE_IMAGE_COMMAND:
        case OS_TOBJ_ID_CL_FILL_BUFFER_COMMAND:
        case OS_TOBJ_ID_CL_FILL_IMAGE_COMMAND:
            retVal = AP_WRITE_QUEUE_COMMANDS;
            break;

        case OS_TOBJ_ID_CL_COPY_BUFFER_COMMAND:
        case OS_TOBJ_ID_CL_COPY_BUFFER_RECT_COMMAND:
        case OS_TOBJ_ID_CL_COPY_BUFFER_TO_IMAGE_COMMAND:
        case OS_TOBJ_ID_CL_COPY_IMAGE_COMMAND:
        case OS_TOBJ_ID_CL_COPY_IMAGE_TO_BUFFER_COMMAND:
            retVal = AP_COPY_QUEUE_COMMANDS;
            break;

        case OS_TOBJ_ID_CL_READ_BUFFER_COMMAND:
        case OS_TOBJ_ID_CL_READ_BUFFER_RECT_COMMAND:
        case OS_TOBJ_ID_CL_READ_IMAGE_COMMAND:
            retVal = AP_READ_QUEUE_COMMANDS;
            break;

        case OS_TOBJ_ID_CL_ACQUIRE_GL_OBJECTS_COMMAND:
        case OS_TOBJ_ID_CL_BARRIER_COMMAND:
        case OS_TOBJ_ID_CL_MAP_BUFFER_COMMAND:
        case OS_TOBJ_ID_CL_MAP_IMAGE_COMMAND:
        case OS_TOBJ_ID_CL_MARKER_COMMAND:
        case OS_TOBJ_ID_CL_RELEASE_GL_OBJECTS_COMMAND:
        case OS_TOBJ_ID_CL_UNMAP_MEM_OBJECT_COMMAND:
        case OS_TOBJ_ID_CL_WAIT_FOR_EVENTS_COMMAND:
        case OS_TOBJ_ID_CL_MIGRATE_MEM_OBJECTS_COMMAND:
        case OS_TOBJ_ID_CL_MARKER_WITH_WAIT_LIST_COMMAND:
        case OS_TOBJ_ID_CL_BARRIER_WITH_WAIT_LIST_COMMAND:
        case OS_TOBJ_ID_CL_SVM_FREE_COMMAND:
        case OS_TOBJ_ID_CL_SVM_MEMCPY_COMMAND:
        case OS_TOBJ_ID_CL_SVM_MEM_FILL_COMMAND:
        case OS_TOBJ_ID_CL_SVM_MAP_COMMAND:
        case OS_TOBJ_ID_CL_SVM_UNMAP_COMMAND:
            retVal = AP_OTHER_QUEUE_COMMANDS;
            break;

        case OS_TOBJ_ID_CL_QUEUE_IDLE_TIME:
            // Idles should not be shown in the gantt chart:
            GT_ASSERT(false);
            break;

        default:
            // Not a known command type:
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apOpenCLQueueCommandsCategoryToString
// Description: Returns category's name as a string
// Author:  AMD Developer Tools Team
// Date:        11/3/2010
// ---------------------------------------------------------------------------
void apOpenCLQueueCommandsCategoryToString(apOpenCLQueueCommandsCategories category, gtString& categoryName)
{
    switch (category)
    {
        case AP_KERNEL_QUEUE_COMMANDS:
        {
            categoryName = L"Kernel";
        }
        break;

        case AP_WRITE_QUEUE_COMMANDS:
        {
            categoryName = L"Write";
        }
        break;

        case AP_COPY_QUEUE_COMMANDS:
        {
            categoryName = L"Copy";
        }
        break;

        case AP_READ_QUEUE_COMMANDS:
        {
            categoryName = L"Read";
        }
        break;

        case AP_OTHER_QUEUE_COMMANDS:
        {
            categoryName = L"Other";
        }
        break;

        default:
        {
            // Unexpected value!
            GT_ASSERT(false);
            categoryName = L"Unknown";
        }
        break;
    }
}

