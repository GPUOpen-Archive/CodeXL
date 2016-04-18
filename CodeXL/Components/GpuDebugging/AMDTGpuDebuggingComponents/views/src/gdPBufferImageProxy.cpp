//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdPBufferImageProxy.cpp
///
//==================================================================================

//------------------------------ gdPBufferImageProxy.cpp ------------------------------

// Qt
#include <QtWidgets>




// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osStopWatch.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTAPIClasses/Include/apPBuffer.h>
#include <AMDTApplicationComponents/Include/acRawFileHandler.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/views/gdPBufferImageProxy.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>

// ---------------------------------------------------------------------------
// Name:        gdPBufferImageProxy::generateImage
// Description: Generates a static buffer image preview
// Return Val:  QImage texture preview image if successful,
//              NULL otherwise
// Author:      Eran Zinman
// Date:        31/12/2007
// ---------------------------------------------------------------------------
bool gdPBufferImageProxy::loadImage()
{
    bool retVal = false;

    // First, clear the loaded image if such an image exists
    releaseLoadedImage();

    // Generate a static buffer preview only if we not in a glBegin - glEnd block
    if (!_isInGLBeginEndBlock)
    {
        // Try to generate the buffer image
        bool rc1 = generateBufferImage();

        if (!rc1)
        {
            // We failed to generate the buffer image. Show buffer not available message:
            gtString msg;
            msg.appendFormattedString(GD_STR_ImageProxyObjectNAMessage, GD_STR_ImageProxyPBuffer);
            m_pLoadedQImage = createMessageImage(msg);
        }
    }
    else
    {
        // We are in a glBegin - glEnd block. generate the appropriate message
        gtString msg;
        msg.appendFormattedString(GD_STR_ImageProxyGLBeginEndMessage, GD_STR_ImageProxyPBuffer);
        m_pLoadedQImage = createMessageImage(msg);
    }

    // Return true if we got a valid image
    retVal = (m_pLoadedQImage != NULL);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdPBufferImageProxy::generateBufferImage
// Description: Generates the buffer image.
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        24/1/2008
// ---------------------------------------------------------------------------
bool gdPBufferImageProxy::generateBufferImage()
{
    bool retVal = false;

    // Get static buffer type
    apDisplayBuffer bufferType = _bufferDetails.bufferType();

    // Extract the static buffer raw data to disk
    bool rc1 = gaUpdatePBufferStaticBufferRawData(_pbufferContext, _pbufferID, bufferType);

    if (rc1)
    {
        // Get the static buffer filename
        osFilePath bufferFile;
        _bufferDetails.getBufferFilePath(bufferFile);

        // Localize the path as needed:
        gaRemoteToLocalFile(bufferFile, false);

        // Load the raw data file
        acRawFileHandler rawFileHandler;
        bool rc3 = rawFileHandler.loadFromFile(bufferFile);
        GT_IF_WITH_ASSERT(rc3)
        {
            // If raw data was loaded successfully
            GT_IF_WITH_ASSERT(rawFileHandler.isOk())
            {
                // For now we normalize the values only for DEPTH and STENCIL buffers:
                bool normalizeValues = ((bufferType == AP_DEPTH_BUFFER) || (bufferType == AP_STENCIL_BUFFER));

                if (normalizeValues)
                {
                    // Get the buffer data format:
                    oaTexelDataFormat bufferDataFormat;
                    oaDataType bufferDataType;
                    _bufferDetails.getBufferFormat(bufferDataFormat, bufferDataType);
                    rc3 = rawFileHandler.normalizeValues(bufferDataFormat);
                    GT_ASSERT(rc3);
                }

                // Convert the raw data to QImage type
                m_pLoadedQImage = rawFileHandler.convertToQImage();

                // Return true if image was generated successfully
                retVal = (m_pLoadedQImage != NULL);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdPBufferImageProxy::buildTooltipText
// Description: Builds the PBuffer tooltip text
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        24/8/2010
// ---------------------------------------------------------------------------
void gdPBufferImageProxy::buildTooltipText()
{
    m_tooltipText.makeEmpty();

    // PBuffer name:
    m_tooltipText.appendFormattedString(GD_STR_ImagesAndBuffersViewerPBufferName, _pbufferID);
    m_tooltipText.append(AF_STR_NewLine);

    // Get the PBuffer details:
    apPBuffer pbufferDetails;
    gaGetPBufferObjectDetails(_pbufferID, pbufferDetails);

    // Associated render context id
    gtString strAttachedRenderContext;
    strAttachedRenderContext.appendFormattedString(GD_STR_PropertiesPBufferHRC, _pbufferContext);
    m_tooltipText.appendFormattedString(L"Attached RC: %ls", strAttachedRenderContext.asCharArray());

    // Associate device context:
    gtString strAttachedDeviceContext;
    oaDeviceContextHandle deviceContextHandle = pbufferDetails.deviceContextOSHandle();
    strAttachedDeviceContext.appendFormattedString(L"%p", deviceContextHandle);
    m_tooltipText.appendFormattedString(L"Attached DC: %ls", strAttachedDeviceContext.asCharArray());

    // pbuffer handler:
    gtString strPBufferHandler;
    oaPBufferHandle pbufferHandler = pbufferDetails.pbufferHandler();
    strPBufferHandler.appendFormattedString(L"%p", pbufferHandler);
    m_tooltipText.appendFormattedString(L"PBuffer handler: %ls", strPBufferHandler.asCharArray());
}


