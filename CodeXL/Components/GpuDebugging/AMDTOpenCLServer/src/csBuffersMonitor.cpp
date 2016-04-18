//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csBuffersMonitor.cpp
///
//==================================================================================

//------------------------------ csBuffersMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Spy Utils:
#include <GRSpiesUtilities/suGlobalVariables.h>
#include <GRSpiesUtilities/suStringConstants.h>
#include <GRSpiesUtilities/suInterceptionMacros.h>

// Local:
#include <inc/csBuffersMonitor.h>
#include <inc/csOpenCLMonitor.h>
#include <inc/csGlobalVariables.h>
#include <inc/csMonitoredFunctionPointers.h>
#include <inc/csBufferSerializer.h>


// ---------------------------------------------------------------------------
// Name:        csBuffersMonitor::csBuffersMonitor
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        18/11/2009
// ---------------------------------------------------------------------------
csBuffersMonitor::csBuffersMonitor(): _spyContextId(0), _commandQueue(NULL)
{
}


// ---------------------------------------------------------------------------
// Name:        csBuffersMonitor::~csBuffersMonitor
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        18/11/2009
// ---------------------------------------------------------------------------
csBuffersMonitor::~csBuffersMonitor()
{
}


// ---------------------------------------------------------------------------
// Name:        csBuffersMonitor::onBufferCreation
// Description: Handles buffer object creation
// Arguments: cl_mem_flags flags
//            size_t size
// Return Val: void
// Author:      Sigal Algranaty
// Date:        1/12/2009
// ---------------------------------------------------------------------------
void csBuffersMonitor::onBufferCreation(cl_mem bufferMemoryHandle, cl_mem_flags flags, size_t size)
{
    // Create the buffer object:
    apCLBuffer* pBufferObject = new apCLBuffer;
    GT_ASSERT_ALLOCATION(pBufferObject);

    // Set buffer properties:
    pBufferObject->setMemoryFlags(flags);
    pBufferObject->setBufferSize(size);
    pBufferObject->setBufferHandle((osCLMemHandle)bufferMemoryHandle);

    // Add the monitor to the vector of buffers:
    _bufferMonitors.push_back(pBufferObject);

}


// ---------------------------------------------------------------------------
// Name:        csBuffersMonitor::amountOfBuffers
// Description: Returns the amount of logged buffers.
// Author:      Yaki Tebeka
// Date:        18/11/2009
// ---------------------------------------------------------------------------
int csBuffersMonitor::amountOfBuffers() const
{
    int retVal = (int)_bufferMonitors.size();
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csBuffersMonitor::bufferDetails
// Description: Inputs a buffer index and return it's monitor.
// Author:      Yaki Tebeka
// Date:        18/11/2009
// ---------------------------------------------------------------------------
const apCLBuffer* csBuffersMonitor::bufferDetails(int bufferId) const
{
    const apCLBuffer* retVal = NULL;

    // Index range check:
    int buffersAmount = (int)_bufferMonitors.size();
    GT_IF_WITH_ASSERT((0 <= buffersAmount) && (bufferId < buffersAmount))
    {
        retVal = _bufferMonitors[bufferId];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csBuffersMonitor::bufferDetails
// Description: Inputs a buffer index and return it's monitor.
// Author:      Yaki Tebeka
// Date:        18/11/2009
// ---------------------------------------------------------------------------
apCLBuffer* csBuffersMonitor::bufferDetails(int bufferId)
{
    apCLBuffer* retVal = NULL;

    // Index range check:
    int buffersAmount = (int)_bufferMonitors.size();
    GT_IF_WITH_ASSERT((0 <= buffersAmount) && (bufferId < buffersAmount))
    {
        retVal = _bufferMonitors[bufferId];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csBuffersMonitor::updateBufferRawData
// Description: Updates a buffer raw data
// Arguments:   bufferId - the updated buffer id
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool csBuffersMonitor::updateBufferRawData(int bufferId)
{
    bool retVal = false;

    // Get the buffer object:
    apCLBuffer* pBufferDetails = bufferDetails(bufferId);
    GT_IF_WITH_ASSERT(pBufferDetails != NULL)
    {
        // If the buffer is "dirty":
        if (pBufferDetails->isDirty())
        {
            // Generate the buffer file name:
            osFilePath bufferFilePath;
            generateBufferFilePath(bufferId, bufferFilePath);

            // Set the buffer file path:
            pBufferDetails->setBufferFilePath(bufferFilePath);

            // Initialize the command queue used for reading the buffer:
            bool rc = initializeCommandQueue();
            GT_IF_WITH_ASSERT(rc && (_commandQueue != NULL))
            {
                // Save the buffer content to a file:
                csBufferSerializer bufferSerializer;
                retVal = bufferSerializer.saveBufferToFile(*pBufferDetails, bufferFilePath, _commandQueue);
            }

        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csBuffersMonitor::generateBufferFilePath
// Description: Generates a buffer file path.
// Arguments: int bufferId
//            osFilePath& bufferFilePath
// Return Val: void
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
void csBuffersMonitor::generateBufferFilePath(int bufferId, osFilePath& bufferFilePath) const
{
    // Build the log file name:
    gtString logFileName;
    logFileName.appendFormattedString("CLContext%d-Buffer%d", _spyContextId, bufferId);

    // Set the log file path:
    bufferFilePath = suLogFilesDirectory();
    bufferFilePath.setFileName(logFileName);

    // Set the log file extension:
    bufferFilePath.setFileExtension(SU_STR_rawFileExtension);
}


// ---------------------------------------------------------------------------
// Name:        csBuffersMonitor::initializeCommandQueue
// Description: Initializes the command queue for buffers reading
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool csBuffersMonitor::initializeCommandQueue()
{
    bool retVal = false;

    if (_commandQueue == NULL)
    {
        // Get the OpenCL monitor:
        csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();

        // Get the context monitor:
        const csContextMonitor* pContextMonitor = theOpenCLMonitor.contextMonitor(_spyContextId);
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            // Get the buffer context handle:
            osCLContextHandle contextHandle = pContextMonitor->contextHandle();

            // Get a device handle:
            csDevicesMonitor& devicesMonitor = theOpenCLMonitor.devicesMonitor();

            // Get the devices details:
            // TO_DO: OpenCL buffers  - which device should be used here???
            osCLDeviceID deviceId = devicesMonitor.getGPUDeviceHandle();

            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clCreateCommandQueue);

            // Create the OpenCL command queue:
            cl_int cRetVal = 0;
            cl_command_queue commandQueue = cs_stat_realFunctionPointers.clCreateCommandQueue((cl_context)contextHandle, (cl_device_id)deviceId, 0, &cRetVal);

            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clCreateCommandQueue);
            GT_IF_WITH_ASSERT((cRetVal == CL_SUCCESS) && (commandQueue != NULL))
            {
                _commandQueue = (osCLCommandQueueHandle)commandQueue;
                retVal = true;
            }

        }
    }
    else
    {
        retVal = true;
    }

    return retVal;
}
