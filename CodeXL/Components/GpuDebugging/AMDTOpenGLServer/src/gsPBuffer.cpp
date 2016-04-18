//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsPBuffer.cpp
///
//==================================================================================

//------------------------------ gsPBuffer.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Spy Utilities:
#include <AMDTServerUtilities/Include/suStringConstants.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Local:
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsPBuffer.h>
#include <src/gsWrappersCommon.h>
#include <src/gsRenderContextMonitor.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsStringConstants.h>

// The OpenGL monitor single instance:
const gsOpenGLMonitor& stat_theOpenGLMonitor = gsOpenGLMonitor::instance();

// ---------------------------------------------------------------------------
// Name:        gsPBuffer::gsPBuffer
// Description: Constructor
// Author:      Eran Zinman
// Date:        24/8/2007
// ---------------------------------------------------------------------------
gsPBuffer::gsPBuffer(const oaPBufferHandle& pbufferHandler, int pbufferID)
    : apPBuffer(pbufferHandler), _pbufferID(pbufferID)
{
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffer::~gsPBuffer
// Description: Destructor
// Author:      Eran Zinman
// Date:        24/8/2007
// ---------------------------------------------------------------------------
gsPBuffer::~gsPBuffer()
{

}

// ---------------------------------------------------------------------------
// Name:        gsPBuffer::updateDataSnapshot
// Description: Update the PBuffer data snapshot
// Author:      Eran Zinman
// Date:        28/8/2007
// ---------------------------------------------------------------------------
void gsPBuffer::updateDataSnapshot()
{
    // Update all static buffers linked with the PBuffer
    gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(_pbufferRenderContextSpyId);
    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
    {
        pRenderContextMonitor->buffersMonitor().updateContextDataSnapshot(true);
    }
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffer::updateDataSnapshot
// Description: Retrieves the PBuffer render context handle
// Arguments:   renderContextOSHandle - Output PBuffer render context handle
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        3/1/2009
// ---------------------------------------------------------------------------
bool gsPBuffer::getRenderContextOSHandle(oaOpenGLRenderContextHandle& renderContextOSHandle) const
{
    bool retVal = false;

    // Do you have a valid render context id?
    GT_IF_WITH_ASSERT(_pbufferRenderContextSpyId > 0)
    {
        // Get the PBuffer render context monitor
        const gsRenderContextMonitor* pRenderContextMonitor = stat_theOpenGLMonitor.renderContextMonitor(_pbufferRenderContextSpyId);
        GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
        {
            // Get the PBuffer attached render context monitor hRC
            renderContextOSHandle = pRenderContextMonitor->renderContextOSHandle();

            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffer::updatePBufferStaticBufferRawData
// Description: Updates the pPuffer static buffer raw data.
// Arguments:   bufferType - Static buffer type to update
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        24/8/2007
// Implementation Note: We assume that the PBuffer was made current
//                      before calling this function.
// ---------------------------------------------------------------------------
bool gsPBuffer::updatePBufferStaticBufferRawData(apDisplayBuffer bufferType)
{
    // Update the static buffer linked with this PBuffer
    bool retVal = false;
    gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(_pbufferRenderContextSpyId);
    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
    {
        retVal = pRenderContextMonitor->buffersMonitor().updateBufferRawData(bufferType);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsPBuffer::generateBufferFilePath
// Description: Overrides gsBuffersMonitor: we need to generate our own name
//              convention for PBuffers
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        28/8/2007
// ---------------------------------------------------------------------------
bool gsPBuffer::generateBufferFilePath(apDisplayBuffer bufferType, osFilePath& bufferFilePath) const
{
    bool retVal = false;

    // Convert the buffer type into a string describing the buffer
    gtString bufferNameCode;
    bool rc = apGetBufferNameCode(bufferType, bufferNameCode);
    GT_IF_WITH_ASSERT(rc)
    {
        // Build the buffer log file name:
        gtString logFileName;
        logFileName.appendFormattedString(GS_STR_pbufferFilePath, _pbufferRenderContextSpyId, _pbufferID, bufferNameCode.asCharArray());

        // Set the log file path:
        bufferFilePath = suCurrentSessionLogFilesDirectory();

        // Set the file name
        bufferFilePath.setFileName(logFileName);
        bufferFilePath.setFileExtension(SU_STR_rawFileExtension);
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffer::getPBufferStaticBufferObject
// Description: Get a static buffer object from the PBuffer objects
// Arguments:   The iter position of the static buffer object
// Return Val:  Pointer to the apStaticBuffer object if successful, otherwise
//              NULL will be returned
// Author:      Eran Zinman
// Date:        03/09/2007
// ---------------------------------------------------------------------------
apStaticBuffer* gsPBuffer::getPBufferStaticBufferObject(int staticBufferIter) const
{
    apStaticBuffer* pStaticBufferItem = NULL;

    // If PBuffer was not deleted:
    GT_IF_WITH_ASSERT(!_isDeleted)
    {
        // Range check:
        int amountOfStaticBuffersIndices = -1;
        const gsRenderContextMonitor* pRenderContextMonitor = stat_theOpenGLMonitor.renderContextMonitor(_pbufferRenderContextSpyId);
        GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
        {
            amountOfStaticBuffersIndices = pRenderContextMonitor->buffersMonitor().amountOfStaticBuffers();
        }

        GT_IF_WITH_ASSERT((0 <= staticBufferIter) && (staticBufferIter < amountOfStaticBuffersIndices))
        {
            // Get static buffer item
            pStaticBufferItem = getPBufferStaticBufferObject(staticBufferIter);
        }
    }

    return pStaticBufferItem;
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffer::onDeletion
// Description: Occurs when a PBuffer gets deleted
// Author:      Eran Zinman
// Date:        24/8/2007
// ---------------------------------------------------------------------------
void gsPBuffer::onDeletion()
{
    // Flag the PBuffer was deleted
    _isDeleted = true;

    // Clear all buffers contained under this PBuffer
    gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(_pbufferRenderContextSpyId);
    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
    {
        pRenderContextMonitor->buffersMonitor().clearContextDataSnapshot();
    }
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffer::updateStaticBuffersList
// Description: Updates the static buffers list
// Author:      Uri Shomroni
// Date:        26/10/2008
// ---------------------------------------------------------------------------
void gsPBuffer::updateStaticBuffersList()
{
    gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(_pbufferRenderContextSpyId);
    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
    {
        pRenderContextMonitor->buffersMonitor().updateContextDataSnapshot(true);
    }
}

