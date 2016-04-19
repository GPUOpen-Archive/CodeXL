//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csMemoryObjectReader.cpp
///
//==================================================================================

//------------------------------ csMemoryObjectReader.cpp ------------------------------

// Standard C:
#include <stdlib.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>

// Spy Utilities:
#include <AMDTServerUtilities/Include/suInterceptionMacros.h>

// Local:
#include <src/csMemoryObjectReader.h>
#include <src/csGlobalVariables.h>
#include <src/csMonitoredFunctionPointers.h>

// ---------------------------------------------------------------------------
// Name:        csMemoryObjectReader::csMemoryObjectReader
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
csMemoryObjectReader::csMemoryObjectReader() : suBufferReader(),
    _bufferMemoryHandle(OA_CL_NULL_HANDLE), _commandQueueHandle(OA_CL_NULL_HANDLE), _memoryObjectType(OS_TOBJ_ID_CL_BUFFER)
{
}


// ---------------------------------------------------------------------------
// Name:        csMemoryObjectReader::~csMemoryObjectReader
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
csMemoryObjectReader::~csMemoryObjectReader()
{
}

// ---------------------------------------------------------------------------
// Name:        csMemoryObjectReader::readBufferContentFromAPI
// Description: Reads the buffer content from OpenCL API
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool csMemoryObjectReader::readBufferContentFromAPI()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_commandQueueHandle != OA_CL_NULL_HANDLE)
    {
        if (_memoryObjectType == OS_TOBJ_ID_CL_BUFFER)
        {
            // Make sure that we have a valid buffer handle:
            GT_IF_WITH_ASSERT(_bufferMemoryHandle != OA_CL_NULL_HANDLE)
            {
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clEnqueueReadBuffer);
                cl_int cRetVal = cs_stat_realFunctionPointers.clEnqueueReadBuffer((cl_command_queue)_commandQueueHandle, (cl_mem)_bufferMemoryHandle, CL_TRUE, _bufferOffset, _bufferWidth, _pReadBufferData, 0, NULL, NULL);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clEnqueueReadBuffer);
                GT_IF_WITH_ASSERT(cRetVal == CL_SUCCESS)
                {
                    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clFlush);
                    cRetVal = cs_stat_realFunctionPointers.clFlush((cl_command_queue)_commandQueueHandle);
                    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clFlush);
                    retVal = true;
                }
            }
        }
        else if (_memoryObjectType == OS_TOBJ_ID_CL_IMAGE)
        {
            // Make sure that we have a valid buffer handle:
            GT_IF_WITH_ASSERT(_bufferMemoryHandle != OA_CL_NULL_HANDLE)
            {
                // Prepare the data to read the texture:
                size_t origin[3] = {(size_t)0, (size_t)0, (size_t)0};
                size_t region[3] = {(size_t)_bufferWidth, (size_t)_bufferHeight, (size_t)_bufferDepth};

                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clEnqueueReadImage);
                cl_int cRetVal = cs_stat_realFunctionPointers.clEnqueueReadImage((cl_command_queue)_commandQueueHandle, (cl_mem)_bufferMemoryHandle, CL_TRUE, origin, region, 0, 0, _pReadBufferData, 0, NULL, NULL);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clEnqueueReadImage);
                GT_IF_WITH_ASSERT(cRetVal == CL_SUCCESS)
                {
                    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clFlush);
                    cRetVal = cs_stat_realFunctionPointers.clFlush((cl_command_queue)_commandQueueHandle);
                    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clFlush);
                    retVal = true;
                }
            }
        }
        else
        {
            GT_ASSERT_EX(false, L"Unknown object type");
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csMemoryObjectReader::readMemoryObjectContent
// Description: Reads the buffer content into an internal raw data buffer:
// Arguments: oaCLCommandQueueHandle commandQueueHandle
//            oaCLMemHandle bufferMemoryHandle
//            int bufferWidth
//            int bufferHeight
//            oaTexelDataFormat bufferDataFormat
//            oaDataType componentDataType
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool csMemoryObjectReader::readMemoryObjectContent(oaCLCommandQueueHandle commandQueueHandle, oaCLMemHandle bufferMemoryHandle,
                                                   int bufferWidth, int bufferHeight, int bufferDepth, int bufferOffset,
                                                   osTransferableObjectType objectType, oaTexelDataFormat bufferDataFormat, oaDataType componentDataType)
{
    bool retVal = false;

    // Set the specific CL properties:
    _bufferMemoryHandle = bufferMemoryHandle;
    _commandQueueHandle = commandQueueHandle;
    _memoryObjectType = objectType;

    // Call the base class implementation:
    retVal = suBufferReader::readBufferContent(bufferWidth, bufferHeight, bufferDepth, bufferOffset, bufferDataFormat, componentDataType);
    return retVal;
}

