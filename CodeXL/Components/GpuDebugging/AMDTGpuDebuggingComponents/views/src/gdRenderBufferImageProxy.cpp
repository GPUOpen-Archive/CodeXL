//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdRenderBufferImageProxy.cpp
///
//==================================================================================

//------------------------------ gdRenderBufferImageProxy.cpp ------------------------------

// Qt
#include <QtWidgets>




// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osStopWatch.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acRawFileHandler.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdRenderBufferImageProxy.h>

// ---------------------------------------------------------------------------
// Name:        gdRenderBufferImageProxy::generateImage
// Description: Generates a render buffer image preview
// Return Val:  QImage render preview image if successful,
//              NULL otherwise
// Author:      Sigal Algranaty
// Date:        3/6/2008
// ---------------------------------------------------------------------------
bool gdRenderBufferImageProxy::loadImage()
{
    bool retVal = false;

    // First, clear the loaded image if such an image exists
    releaseLoadedImage();

    // Generate a static buffer preview only if we not in a glBegin - glEnd block
    if (!_isInGLBeginEndBlock)
    {
        // Try to generate the buffer image
        bool rc1 = generateRenderBufferImage();

        if (!rc1)
        {
            // We failed to generate the render buffer image. Show buffer not available message
            // 1. If render buffer type is unknown, it means that the render buffer is not attahced to an FBO
            // 2. There was a problem writing / reading / converting the texture. Show render buffer not available message

            // Get the buffer type:
            apDisplayBuffer bufferType = _renderBufferDetails.getBufferType();

            // Get the buffer FBO:
            GLuint fboName = _renderBufferDetails.getFBOName();

            // Check if the render buffer display is supported for OpenGL ES projects:
            bool isSupportedFormat = true;
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)

            if (apDoesProjectTypeSupportOpenGLES(gdGDebuggerGlobalVariablesManager::instance().CodeXLProjectType()))
            {
                // Get the buffer format:
                oaTexelDataFormat dataFormat = _renderBufferDetails.bufferFormat();

                // Check if the buffer format is supported for OpenGLES (RGB | RGBA):
                if ((dataFormat != OA_TEXEL_FORMAT_RGB) && (dataFormat != OA_TEXEL_FORMAT_RGBA))
                {
                    isSupportedFormat = false;
                }
            }

#endif

            if (!isSupportedFormat)
            {
                m_pLoadedQImage = createMessageImage(GD_STR_ImageProxyRenderBufferOpenGLESUnsupportedFormat);
            }
            else if ((fboName == 0) || (bufferType == AP_DISPLAY_BUFFER_UNKNOWN))
            {
                m_pLoadedQImage = createMessageImage(GD_STR_ImageProxyRenderBufferUnbound);
            }
            else
            {
                gtString msg;
                msg.appendFormattedString(GD_STR_ImageProxyObjectNAMessage, GD_STR_ImageProxyRenderBuffer);
                m_pLoadedQImage = createMessageImage(msg);
            }
        }
    }
    else
    {
        // We are in a glBegin - glEnd block. generate the appropriate message
        gtString msg;
        msg.appendFormattedString(GD_STR_ImageProxyGLBeginEndMessage, GD_STR_ImageProxyRenderBuffer);
        m_pLoadedQImage = createMessageImage(msg);
    }

    // Return true if we got a valid image
    retVal = (m_pLoadedQImage != NULL);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdRenderBufferImageProxy::generateRenderBufferImage
// Description: Generates the render buffer image.
// Return Val:  bool - Success / Failure
// Author:      Sigal Algranaty
// Date:        3/6/2008
// ---------------------------------------------------------------------------
bool gdRenderBufferImageProxy::generateRenderBufferImage()
{
    bool retVal = false;

    // Get static buffer type
    apDisplayBuffer bufferType = _renderBufferDetails.getBufferType();

    // Extract the static buffer raw data to disk
    gtVector<GLuint> renderBuffersVector;
    renderBuffersVector.push_back(_renderBufferID);
    bool rc1 = gaUpdateRenderBufferRawData(_renderBufferContext, renderBuffersVector);

    if (rc1)
    {
        // Get the selected render buffer details:
        _renderBufferDetails.setAllocatedObjectId(-1, true);
        bool rc2 = gaGetRenderBufferObjectDetails(_renderBufferContext, _renderBufferID, _renderBufferDetails);

        if (rc2)
        {
            // Get the static buffer filename
            osFilePath bufferFile;
            _renderBufferDetails.getBufferFilePath(bufferFile);

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
                        bufferDataFormat = _renderBufferDetails.bufferFormat();
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
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdRenderBufferImageProxy::buildTooltipText
// Description: Build the render buffer tooltip text
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        24/8/2010
// ---------------------------------------------------------------------------
void gdRenderBufferImageProxy::buildTooltipText()
{
    m_tooltipText.makeEmpty();

    // Build the tooltip heading (buffer name):
    m_tooltipText.appendFormattedString(GD_STR_PropertiesRenderBufferHeadline, _renderBufferDetails.renderBufferName());
    m_tooltipText.append(AF_STR_NewLine);

    // Buffer owner FBO:
    GLuint fboName = _renderBufferDetails.getFBOName();
    m_tooltipText.appendFormattedString(GD_STR_ObjectTooltipFBOName, fboName);
    m_tooltipText.append(AF_STR_NewLine);

    // Get buffer type:
    gtString strBufferType;
    bool rc = apGetBufferName(_renderBufferDetails.getBufferType(), strBufferType);

    if (!rc)
    {
        // Buffer is not connected to an FBO, therefore it's type is unknown, and also legal.
        strBufferType = GD_STR_PropertiesRenderBufferUnAttached;
    }

    // Add the buffer type:
    m_tooltipText.appendFormattedString(GD_STR_ObjectTooltipBufferType, strBufferType.asCharArray());
    m_tooltipText.append(AF_STR_NewLine);

    // Get buffer format:
    oaTexelDataFormat bufferDataFormat;
    oaDataType bufferDataType;
    gtString strBufferDataType, strBufferDataFormat;
    bufferDataFormat = _renderBufferDetails.bufferFormat();
    bufferDataType = _renderBufferDetails.bufferDataType();

    // Buffer Data Type:
    GLenum bufferDataTypeEnum = oaDataTypeToGLEnum(bufferDataType);
    apGLenumParameter bufferDataTypeParameter(bufferDataTypeEnum);
    bufferDataTypeParameter.valueAsString(strBufferDataType);

    // Buffer Data Format:
    GLenum bufferDataFormatEnum = oaTexelDataFormatToGLEnum(bufferDataFormat);
    apGLenumParameter bufferDataFormatParameter(bufferDataFormatEnum);
    bufferDataFormatParameter.valueAsString(strBufferDataFormat);

    // Add the buffer type:
    m_tooltipText.appendFormattedString(GD_STR_ObjectTooltipDataFormat, strBufferDataFormat.asCharArray());
    m_tooltipText.append(AF_STR_NewLine);
    m_tooltipText.appendFormattedString(GD_STR_ObjectTooltipDataType, strBufferDataType.asCharArray());
    m_tooltipText.append(AF_STR_NewLine);

}
