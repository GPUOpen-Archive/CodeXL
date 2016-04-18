//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStaticBufferImageProxy.cpp
///
//==================================================================================

//------------------------------ gdStaticBufferImageProxy.cpp ------------------------------

// Qt
#include <QtWidgets>




// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osStopWatch.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTAPIClasses/Include/apGLTexture.h>
#include <AMDTApplicationComponents/Include/acRawFileHandler.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/views/gdStaticBufferImageProxy.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>

// ---------------------------------------------------------------------------
// Name:        gdStaticBufferImageProxy::generateImage
// Description: Generates a static buffer image preview
// Return Val:  QImage texture preview image if successful,
//              NULL otherwise
// Author:      Eran Zinman
// Date:        31/12/2007
// ---------------------------------------------------------------------------
bool gdStaticBufferImageProxy::loadImage()
{
    bool retVal = false;

    // First, clear the loaded image if such an image exists
    releaseLoadedImage();

    // Generate a static buffer preview only if we not in a glBegin - glEnd block
    if (!_isInGLBeginEndBlock && !_isFBOBound)
    {
        // Check if the object is supported for OpenGL ES projects:
        bool isSupportedBufferType = true;

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)

        if (apDoesProjectTypeSupportOpenGLES(gdGDebuggerGlobalVariablesManager::instance().CodeXLProjectType()))
        {
            // Get the buffer type:
            apDisplayBuffer bufferType = _bufferDetails.bufferType();

            if ((bufferType == AP_AUX0_BUFFER) || (bufferType == AP_AUX1_BUFFER) ||
                (bufferType == AP_AUX2_BUFFER) || (bufferType == AP_AUX3_BUFFER) ||
                (bufferType == AP_DEPTH_BUFFER))
            {
                isSupportedBufferType = false;
            }
        }

#endif
        bool rc1 = false;
        gtString bufferMessage;

        if (isSupportedBufferType)
        {
            // Try to generate the buffer image
            rc1 = generateBufferImage();
        }
        else
        {
            bufferMessage = GD_STR_ImageProxyRenderBufferOpenGLESUnsupportedBufferType;
        }

        if (!rc1)
        {
            if (bufferMessage.isEmpty())
            {
                bufferMessage.appendFormattedString(GD_STR_ImageProxyObjectNAMessage, GD_STR_ImageProxyStaticBuffer);
            }

            // We failed to generate the buffer image. Show buffer not available message
            m_pLoadedQImage = createMessageImage(bufferMessage);
        }

    }
    else
    {
        if (_isInGLBeginEndBlock)
        {
            // We are in a glBegin - glEnd block. generate the appropriate message
            gtString msg;
            msg.appendFormattedString(GD_STR_ImageProxyGLBeginEndMessage, GD_STR_ImageProxyStaticBuffer);
            m_pLoadedQImage = createMessageImage(msg);
        }
        else if (_isFBOBound)
        {
            // There's a bound FBO - we cannot load buffer:
            m_pLoadedQImage = createMessageImage(GD_STR_ImageProxyRenderBufferUnbound);
        }
    }

    // Return true if we got a valid image
    retVal = (m_pLoadedQImage != NULL);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStaticBufferImageProxy::generateBufferImage
// Description: Generates the buffer image.
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        24/1/2008
// ---------------------------------------------------------------------------
bool gdStaticBufferImageProxy::generateBufferImage()
{
    bool retVal = false;

    // Get static buffer type
    apDisplayBuffer bufferType = _bufferDetails.bufferType();

    // Extract the static buffer raw data to disk
    bool rc1 = gaUpdateStaticBufferRawData(_activeContext, bufferType);

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
                // Get buffer data type:
                oaTexelDataFormat bufferDataFormat;
                oaDataType bufferDataType;
                _bufferDetails.getBufferFormat(bufferDataFormat, bufferDataType);

                // For now we normalize the values only for DEPTH and STENCIL buffers:
                bool normalizeValues = ((bufferType == AP_DEPTH_BUFFER) || (bufferType == AP_STENCIL_BUFFER));

                if (normalizeValues)
                {
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
// Name:        gdStaticBufferImageProxy::buildTooltipText
// Description: Builds the static buffer tooltip
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        24/8/2010
// ---------------------------------------------------------------------------
void gdStaticBufferImageProxy::buildTooltipText()
{
    // Build the texture tooltip:
    m_tooltipText.makeEmpty();

    // Add the buffer name as tooltip heading:
    apGetBufferName(_bufferDetails.bufferType(), m_tooltipText);
    m_tooltipText.append(AF_STR_NewLine);

    // Add the texel data format to the tooltip:
    if (_bufferDetails.bufferFormat() != OA_TEXEL_FORMAT_UNKNOWN)
    {
        gtString texelFormatStr;
        oaGetTexelDataFormatName(_bufferDetails.bufferFormat(), texelFormatStr);
        m_tooltipText.appendFormattedString(GD_STR_ObjectTooltipPixelFormat, texelFormatStr.asCharArray());
        m_tooltipText.append(AF_STR_NewLine);
    }

    // Add the data type to the tooltip:
    if (_bufferDetails.bufferDataType() != OA_UNKNOWN_DATA_TYPE)
    {
        gtString dataTypeStr;
        GLenum glDataType = oaDataTypeToGLEnum(_bufferDetails.bufferDataType());
        apGLenumParameter dataTypeParam(glDataType);
        dataTypeParam.valueAsString(dataTypeStr);
        m_tooltipText.appendFormattedString(GD_STR_ObjectTooltipDataType, dataTypeStr.asCharArray());
    }

}



