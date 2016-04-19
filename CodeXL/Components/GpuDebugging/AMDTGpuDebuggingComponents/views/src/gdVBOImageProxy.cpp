//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdVBOImageProxy.cpp
///
//==================================================================================

//------------------------------ gdTextureImageProxy.cpp ------------------------------

// Qt
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTAPIClasses/Include/apCLBuffer.h>
#include <AMDTAPIClasses/Include/apGLenumParameter.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdVBOImageProxy.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>

// ---------------------------------------------------------------------------
// Name:        gdVBOImageProxy::generateImage
// Description: Generates a VBO image preview
// Return Val:  QImage VBO preview image if successful,
//              NULL otherwise
// Author:      Sigal Algranaty
// Date:        28/4/2009
// ---------------------------------------------------------------------------
bool gdVBOImageProxy::loadImage()
{
    bool retVal = false;

    // First, clear the loaded image if such an image exists
    releaseLoadedImage();

    gtString bufferMessage;
    bufferMessage.fromASCIIString(GD_STR_ImagesAndBuffersViewerBufferClick);

    // OpenGL ES projects VBO data cannot be shown:
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)

    if (apDoesProjectTypeSupportOpenGLES(gdGDebuggerGlobalVariablesManager::instance().CodeXLProjectType()))
    {
        bufferMessage = GD_STR_ImageProxyRenderBufferOpenGLESUnsupportedVBOData;
    }

#endif

    // Generate an unknown texture message
    m_pLoadedQImage = createMessageImage(bufferMessage);

    // Return true if we got a valid image
    retVal = (m_pLoadedQImage != NULL);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdVBOImageProxy::buildTooltipText
// Description: Builds the VBO tooltip
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        24/8/2010
// ---------------------------------------------------------------------------
void gdVBOImageProxy::buildTooltipText()
{
    // Empty the tooltip:
    m_tooltipText.makeEmpty();

    if (_contextID._contextType == AP_OPENGL_CONTEXT)
    {

        // Get the VBO details:
        apGLVBO vboDetails;
        bool rcDetails = gaGetVBODetails(_contextID._contextId, _bufferName, vboDetails);
        GT_IF_WITH_ASSERT(rcDetails)
        {
            // Get the VBO attachments:
            GLenum vboLastAttachment = GL_NONE;
            gtVector<GLenum> vboCurrentAttachments;
            bool rcAttachment = gaGetVBOAttachment(_contextID._contextId, _bufferName, vboLastAttachment, vboCurrentAttachments);
            GT_ASSERT(rcAttachment);

            // Build the buffer name:
            gtString strBufferName;
            gdGetVBODisplayName(vboDetails, m_tooltipText);

            // Get attachment as string:
            gtString strBufferAttachment;

            if (vboLastAttachment == 0)
            {
                strBufferAttachment = GD_STR_PropertiesRenderBufferUnAttached;
            }
            else
            {
                apGLenumValueToString(vboLastAttachment, strBufferAttachment);
            }

            // Add the buffer attachment to the tooltip:
            m_tooltipText.append(GD_STR_ObjectTooltipLastAttachment).append(strBufferAttachment);

            int currentAttachmentsCount = (int)vboCurrentAttachments.size();

            if (0 < currentAttachmentsCount)
            {
                m_tooltipText.append(GD_STR_ObjectTooltipCurrentAttachments);

                for (int i = 0; i < currentAttachmentsCount; ++i)
                {
                    if (0 < i)
                    {
                        m_tooltipText.append(L", ");
                    }

                    apGLenumValueToString(vboCurrentAttachments[i], strBufferAttachment);
                    m_tooltipText.append(strBufferAttachment);
                }
            }
        }
    }
    else if (_contextID._contextType == AP_OPENCL_CONTEXT)
    {
        // Get the OpenCL buffer details:
        apCLBuffer clBufferDetails;
        bool rcDetails = gaGetOpenCLBufferObjectDetails(_contextID._contextId, _bufferName, clBufferDetails);
        GT_IF_WITH_ASSERT(rcDetails)
        {
            // Add the buffer name to the tooltip first line:
            gtString bufferName;
            gdGetBufferDisplayName(clBufferDetails, m_tooltipText);

            // Build the buffers flag string:
            gtString bufferFlagsStr;
            apCLMemFlags bufferFlags = clBufferDetails.memoryFlags();
            bufferFlags.valueAsString(bufferFlagsStr);

            // Replace the "|" character with HTML new line:
            bufferFlagsStr.replace(L"|", AF_STR_NewLine);

            m_tooltipText.appendFormattedString(GD_STR_ObjectTooltipMemoryFlags, bufferFlagsStr.asCharArray());
            m_tooltipText.append(AF_STR_NewLine);

            // Build the buffer handle string:
            gtString strBufferHandle;
            gdUserApplicationAddressToDisplayString(clBufferDetails.memObjectHandle(), strBufferHandle);
            m_tooltipText.appendFormattedString(GD_STR_ObjectTooltipBufferHandle , strBufferHandle.asCharArray());
            m_tooltipText.append(AF_STR_NewLine);
        }
    }
    else
    {
        GT_ASSERT_EX(false, L"Unsupported context type");
    }
}
