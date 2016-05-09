//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdHTMLProperties.cpp
///
//==================================================================================

//------------------------------ gdHTMLProperties.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

#include <math.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLRenderContext.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAPIClasses/Include/Events/apApiConnectionEndedEvent.h>
#include <AMDTAPIClasses/Include/Events/apApiConnectionEstablishedEvent.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>
#include <AMDTAPIClasses/Include/Events/apComputeContextCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apComputeContextDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessDetectedErrorEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessOutputStringEvent.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>
#include <AMDTAPIClasses/Include/Events/apGDBErrorEvent.h>
#include <AMDTAPIClasses/Include/Events/apGDBOutputStringEvent.h>
#include <AMDTAPIClasses/Include/Events/apGLDebugOutputMessageEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLErrorEvent.h>
#include <AMDTAPIClasses/Include/Events/apMemoryLeakEvent.h>
#include <AMDTAPIClasses/Include/Events/apModuleLoadedEvent.h>
#include <AMDTAPIClasses/Include/Events/apModuleUnloadedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramBuildEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLQueueCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLQueueDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOutputDebugStringEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessCreationFailureEvent.h>
#include <AMDTAPIClasses/Include/Events/apRenderContextCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apRenderContextDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apSearchingForMemoryLeaksEvent.h>
#include <AMDTAPIClasses/Include/Events/apTechnologyMonitorFailureEvent.h>
#include <AMDTAPIClasses/Include/Events/apThreadCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apThreadTerminatedEvent.h>
#include <AMDTAPIClasses/Include/ap2DRectangle.h>
#include <AMDTAPIClasses/Include/apAIDFunctions.h>
#include <AMDTAPIClasses/Include/apBreakReason.h>
#include <AMDTAPIClasses/Include/apCLBuffer.h>
#include <AMDTAPIClasses/Include/apCLCommandQueue.h>
#include <AMDTAPIClasses/Include/apCLContext.h>
#include <AMDTAPIClasses/Include/apCLDevice.h>
#include <AMDTAPIClasses/Include/apCLEnqueuedCommands.h>
#include <AMDTAPIClasses/Include/apCLEvent.h>
#include <AMDTAPIClasses/Include/apCLKernel.h>
#include <AMDTAPIClasses/Include/apCLObjectID.h>
#include <AMDTAPIClasses/Include/apCLPipe.h>
#include <AMDTAPIClasses/Include/apCLProgram.h>
#include <AMDTAPIClasses/Include/apCLSampler.h>
#include <AMDTAPIClasses/Include/apCLSubBuffer.h>
#include <AMDTAPIClasses/Include/apCLImage.h>
#include <AMDTAPIClasses/Include/apDefaultTextureNames.h>
#include <AMDTAPIClasses/Include/apFunctionCall.h>
#include <AMDTAPIClasses/Include/apGLDisplayList.h>
#include <AMDTAPIClasses/Include/apGLFBO.h>
#include <AMDTAPIClasses/Include/apGLItemsCollection.h>
#include <AMDTAPIClasses/Include/apGLPixelInternalFormatParameter.h>
#include <AMDTAPIClasses/Include/apGLProgram.h>
#include <AMDTAPIClasses/Include/apGLRenderBuffer.h>
#include <AMDTAPIClasses/Include/apGLPipeline.h>
#include <AMDTAPIClasses/Include/apGLSampler.h>
#include <AMDTAPIClasses/Include/apGLRenderContextGraphicsInfo.h>
#include <AMDTAPIClasses/Include/apGLRenderContextInfo.h>
#include <AMDTAPIClasses/Include/apGLShaderObject.h>
#include <AMDTAPIClasses/Include/apGLSync.h>
#include <AMDTAPIClasses/Include/apGLTexture.h>
#include <AMDTAPIClasses/Include/apGLVBO.h>
#include <AMDTAPIClasses/Include/apInternalFormat.h>
#include <AMDTAPIClasses/Include/apOpenGLBuiltInUniformManager.h>
#include <AMDTAPIClasses/Include/apPBuffer.h>
#include <AMDTAPIClasses/Include/apStaticBuffer.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>
#include <AMDTAPIClasses/Include/apGLenumParameter.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acImageManager.h>
#include <AMDTOpenGLServer/Include/gsPublicStringConstants.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>


// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdBreakpointsItemData.h>
#include <AMDTGpuDebuggingComponents/Include/gdDeprecationStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdShaderType.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdRenderBufferImageProxy.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStaticBufferImageProxy.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdTextureImageProxy.h>

// Static members:
gtVector<gtString> gdHTMLProperties::m_sCubemapTitles;
bool gdHTMLProperties::m_sAreStaticInitialized = false;

// **********************************************************
// *** INTERNALLY LINKED STATIC UTILITY FUNCTIONS - START ***
// **********************************************************
// ---------------------------------------------------------------------------
// Name:        GetShaderLinkNameByShaderType
// Description: Utility function that converts a shader type to its name in CodeXL link representation
// Arguments:   gdShaderType shaderType - the relevant shader type
//              gtString& shaderLinkNameBuffer - a buffer to hold the output string
// Return Val:  bool (success/failure)
// Author:      Amit Ben-Moshe
// Date:        3/8/2014
// ---------------------------------------------------------------------------
static bool GetShaderLinkNameByShaderType(gdShaderType shaderType, gtString& shaderLinkNameBuffer)
{
    bool ret = true;

    switch (shaderType)
    {
        case GD_VERTEX_SHADER:
        {
            shaderLinkNameBuffer = GD_STR_HtmlPropertiesGLVertexShaderLink;
        }
        break;

        case GD_TESSELLATION_CONTROL_SHADER:
        {
            shaderLinkNameBuffer = GD_STR_HtmlPropertiesGLTessellationControlShaderLink;
        }
        break;

        case GD_TESSELLATION_EVALUATION_SHADER:
        {
            shaderLinkNameBuffer = GD_STR_HtmlPropertiesGLTessellationEvaluationShaderLink;
        }
        break;

        case GD_GEOMETRY_SHADER:
        {
            shaderLinkNameBuffer = GD_STR_HtmlPropertiesGLGeometryShaderLink;
        }
        break;

        case GD_FRAGMENT_SHADER:
        {
            shaderLinkNameBuffer = GD_STR_HtmlPropertiesGLFragmentShaderLink;
        }
        break;

        case GD_COMPUTE_SHADER:
        {
            shaderLinkNameBuffer = GD_STR_HtmlPropertiesGLComputeShaderLink;
        }
        break;

        case GD_UNSUPPORTED_SHADER:
        {
            shaderLinkNameBuffer = GD_STR_HtmlPropertiesGLUnsupportedShaderLink;
        }
        break;

        case GD_UNKNOWN_SHADER:
        default:
        {
            GT_ASSERT_EX(false, L"Unknown shader type.");
            ret = false;
        }
        break;
    }

    return ret;
}
// **********************************************************
// *** INTERNALLY LINKED STATIC UTILITY FUNCTIONS - END ***
// **********************************************************

// ---------------------------------------------------------------------------
// Name:        validateValueAvailability
// Description: Shortcut to a line repeating itself
// Arguments: gtString& value
// Return Val: void
// Author:      Sigal Algranaty
// Date:        16/12/2009
// ---------------------------------------------------------------------------
void validateValueAvailability(gtString& value)
{
    if (value.isEmpty())
    {
        value = AF_STR_NotAvailable;
    }
}
// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::gdHTMLProperties
// Description: Constructor
// Return Val:
// Author:      Sigal Algranaty
// Date:        15/10/2008
// ---------------------------------------------------------------------------
gdHTMLProperties::gdHTMLProperties()
{
    // Initialize static items:
    if (!m_sAreStaticInitialized)
    {
        initializeStaticMembers();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::initializeStaticMembers
// Description: Initializes static members
// Author:      Sigal Algranaty
// Date:        16/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::initializeStaticMembers()
{
    // Add the parameters cube map title:
    m_sCubemapTitles.clear();
    m_sCubemapTitles.resize(6);

    // Add pos x title:
    gtString title;
    title.fromASCIIString(GD_STR_ImagesAndBuffersViewerCubeMapPositiveX);
    title.append(AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_Value);
    m_sCubemapTitles.push_back(title);

    // Add neg x title:
    title.fromASCIIString(GD_STR_ImagesAndBuffersViewerCubeMapNegativeX);
    title.append(AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_Value);
    m_sCubemapTitles.push_back(title);

    // Add pos y title:
    title.fromASCIIString(GD_STR_ImagesAndBuffersViewerCubeMapPositiveY);
    title.append(AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_Value);
    m_sCubemapTitles.push_back(title);

    // Add neg y title:
    title.fromASCIIString(GD_STR_ImagesAndBuffersViewerCubeMapNegativeY);
    title.append(AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_Value);
    m_sCubemapTitles.push_back(title);

    // Add pos z title:
    title.fromASCIIString(GD_STR_ImagesAndBuffersViewerCubeMapPositiveZ);
    title.append(AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_Value);
    m_sCubemapTitles.push_back(title);

    // Add neg z title:
    title.fromASCIIString(GD_STR_ImagesAndBuffersViewerCubeMapNegativeZ);
    title.append(AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_Value);
    m_sCubemapTitles.push_back(title);

    m_sAreStaticInitialized = true;
}
// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildDebuggedApplicationHTMLPropertiesString
// Description: builds a message for the application
// Author:      Uri Shomroni
// Date:        6/11/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildDebuggedApplicationHTMLPropertiesString(afHTMLContent& htmlContent)
{
    // Get the app name:
    // Get the new project file name:
    gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();

    // Get the app name:
    gtString appName;
    afApplicationCommands* pApplicationCommnads = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommnads != NULL)
    {
        appName = pApplicationCommnads->applicationRootString();
    }

    apDebugProjectSettings creationData = globalVarsManager.currentDebugProjectSettings();

    // Add the debugged application's full path:
    gtString appFullPath = creationData.executablePath().asString();

    if (appFullPath.isEmpty())
    {
        appFullPath = AF_STR_NotAvailable;
    }

    // Get the command Line Arguments:
    gtString commandLineArguments = creationData.commandLineArguments();
    validateValueAvailability(commandLineArguments);

    // Get the environment variables:
    gtString envVars;
    creationData.environmentVariablesAsString(envVars);
    validateValueAvailability(envVars);

    // Build the HTML content:
    htmlContent.setTitle(appName);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDebuggedApplicationFullPath, appFullPath);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDebuggedApplicationCommandLineArgs, commandLineArguments);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDebuggedApplicationEnvVars, envVars);
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildRenderContextHTMLPropertiesString
// Description: Builds a message for a render context
// Arguments: int renderContextID
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Uri Shomroni
// Date:        6/11/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildRenderContextHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent)
{
    // Create the title:
    gtString renderContextNameString;
    renderContextNameString.appendFormattedString(GD_STR_PropertiesContextHeadline, contextID._contextId);

    // Build the HTML content:
    htmlContent.setTitle(renderContextNameString);

    // Create the Header "General":
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    // Get the context's deleted status:
    gtString deletionStatusStr;
    bool wasDeleted = gaWasContextDeleted(contextID);
    deletionStatusStr = (wasDeleted) ?  AF_STR_Yes : AF_STR_No;

    // Build the context link string:
    gtString contextLinkStr;
    contextID.toString(contextLinkStr);
    contextLinkStr.prependFormattedString(AF_STR_HtmlPropertiesLink1ParamStart, GD_STR_HtmlPropertiesGLContextLink, contextID._contextId);
    contextLinkStr.append(GD_STR_HtmlPropertiesLinkEnd);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesContextName, contextLinkStr);

    // Add the deletion status:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_Deleted, deletionStatusStr);

    // Get the render context info:
    apGLRenderContextInfo renderContextInfo;
    int sharedID = -1;
    bool rcRC = gaGetRenderContextDetails(contextID._contextId, renderContextInfo);
    GT_IF_WITH_ASSERT(rcRC)
    {
        sharedID = renderContextInfo.sharingContextID();

        if (sharedID > 0)
        {
            gtString sharedContextStr;
            sharedContextStr.appendFormattedString(GD_STR_PropertiesContextHeadline, sharedID);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesRenderContextShareLists, sharedContextStr);
        }
    }

    // TO_DO: Find more info to put here (hDC, hRC, etc)
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesRenderContextMessage);
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLContextHTMLPropertiesString
// Description: Build an OpenCL properties string
// Arguments: apContextID contextID
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        7/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLContextHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent)
{
    // Get the context info from API:
    apCLContext contextInfo;
    bool rcCtxInfo = gaGetOpenCLContextDetails(contextID._contextId, contextInfo);
    GT_ASSERT(rcCtxInfo);

    // Create the title:
    gtString contextNameString;
    contextNameString.appendFormattedString(GD_STR_PropertiesContextHeadline, contextID._contextId);

    // Build the context link string:
    gtString contextLinkString;
    contextLinkString.appendFormattedString(AF_STR_HtmlPropertiesLink1ParamStart GD_STR_PropertiesViewDetailsTemplate GD_STR_HtmlPropertiesLinkEnd, GD_STR_HtmlPropertiesCLContextLink, contextID._contextId, contextNameString.asCharArray());

    bool wasDeleted = contextInfo.wasMarkedForDeletion();
    gtString deletionStatus = wasDeleted ? AF_STR_Yes : AF_STR_No;

    // Build reference count string:
    gtUInt32 refCount = contextInfo.referenceCount();
    gtString refCountAsStr;
    refCountAsStr.appendFormattedString(L"%u", refCount);

    // Build the HTML content:
    htmlContent.setTitle(contextNameString);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReferenceCountHeader, refCountAsStr);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogMarkedForDeletion, deletionStatus);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, contextLinkString);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesRenderContextMessage);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildStaticBuffersListHTMLPropertiesString
// Description: builds a message for the static buffers list in the given render context
// Author:      Uri Shomroni
// Date:        6/11/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildStaticBuffersListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects)
{
    (void)(numberOfObjects);  // unused
    // Create the Static buffers title:
    gtString buffersTitle;
    buffersTitle.appendFormattedString(GD_STR_PropertiesStaticBuffersListHeadline, contextID._contextId);

    // Build the HTML content:
    htmlContent.setTitle(buffersTitle);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesSelectItemMessage);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildStaticBufferHTMLPropertiesString
// Description: Builds a static buffer properties in HTML format
// Arguments: int renderContextId
//            const apStaticBuffer& staticBufferDetails
//            bool bAddThumbnail
//            wxWindow* pCallingWindow
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/10/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildStaticBufferHTMLPropertiesString(const apContextID& contextID, const apStaticBuffer& staticBufferDetails, bool bAddThumbnail, afProgressBarWrapper* pProgressBar, afHTMLContent& htmlContent)
{
    // Get buffer dimensions:
    GLint bufferWidth, bufferHeight;
    gtString strBufferDimensions;
    staticBufferDetails.getBufferDimensions(bufferWidth, bufferHeight);

    strBufferDimensions.appendFormattedString(GD_STR_Properties2DDimensions, bufferWidth, bufferHeight);

    // Get buffer format:
    oaTexelDataFormat bufferDataFormat;
    oaDataType bufferDataType;
    gtString strBufferDataType, strBufferDataFormat;
    staticBufferDetails.getBufferFormat(bufferDataFormat, bufferDataType);

    // Buffer Data Type:
    GLenum bufferDataTypeEnum = oaDataTypeToGLEnum(bufferDataType);
    apGLenumValueToString(bufferDataTypeEnum, strBufferDataType);

    // Buffer Data Format:
    GLenum bufferDataFormatEnum = oaTexelDataFormatToGLEnum(bufferDataFormat);
    apGLenumValueToString(bufferDataFormatEnum, strBufferDataFormat);

    // Get buffer type:
    gtString strBufferType, strBufferTypeWithLink;
    apGetBufferName(staticBufferDetails.bufferType(), strBufferType);

    strBufferTypeWithLink.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesGLStaticBufferLink, staticBufferDetails.bufferType(), contextID._contextId);
    strBufferTypeWithLink.append(strBufferType);
    strBufferTypeWithLink.append(GD_STR_HtmlPropertiesLinkEnd);

    gtString strBufferThumbnail;

    if (bAddThumbnail)
    {
        // Generate the texture thumbnail:
        // Will hold the generated texture path and dimensions
        osFilePath texturePreviewPath;
        int imageWidth = 0;
        int imageHeight = 0;

        bool rc1 = generateObjectPreview(contextID, AP_HTML_STATIC_BUFFER, 0, staticBufferDetails.bufferType(), texturePreviewPath, imageWidth, imageHeight, pProgressBar);

        if (rc1)
        {
            // Image was generated successfully - add the image to the properties view
            strBufferThumbnail.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesGLStaticBufferLink, staticBufferDetails.bufferType(), contextID._contextId);
            strBufferThumbnail.appendFormattedString(AF_STR_HtmlImageWithSizeSpecsAndRightAlign, texturePreviewPath.asString().asCharArray(), imageWidth, imageHeight);
            strBufferThumbnail += GD_STR_HtmlPropertiesLinkEnd;
        }
    }

    // Build the HTML content:
    htmlContent.setTitle(strBufferType, strBufferThumbnail);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    // Add the buffer type:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBufferTypeTitle, strBufferTypeWithLink);

    // Add the buffer dimensions header:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDimensions, strBufferDimensions);

    // Add the buffer pixel format:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPixelFormatTitle, strBufferDataFormat);

    // Add the buffer "texels" type:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_Type, strBufferDataType);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildVBOsListHTMLPropertiesString
// Description: builds a message for the vertex buffers list in the given render context
// Author:      Uri Shomroni
// Date:        6/11/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildVBOsListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects)
{
    (void)(numberOfObjects);  // unused
    // Create the Header "VBOs list" title:
    gtString vbosListTitle;
    vbosListTitle.appendFormattedString(GD_STR_PropertiesVBOsListHeadline, contextID._contextId);

    // Build the HTML content:
    htmlContent.setTitle(vbosListTitle);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesSelectItemMessage);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildVBOHTMLPropertiesString
// Description: Build a vbo properties in HTML format
// Arguments: const apGLVBO vboDetails
//            GLenum vboAttachment
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        23/10/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildVBOHTMLPropertiesString(const apContextID& contextID, const apGLVBO& vboDetails, GLenum& vboLastAttachment, gtVector<GLenum>& vboCurrentAttachments, afHTMLContent& htmlContent, bool showDetailsLink)
{
    // Get attachment as string:
    gtString strBufferLastAttachment;

    if (vboLastAttachment == 0)
    {
        strBufferLastAttachment = GD_STR_PropertiesRenderBufferUnAttached;
    }
    else
    {
        apGLenumValueToString(vboLastAttachment, strBufferLastAttachment);
    }

    gtString strBufferCurrentAttachments;
    int currentAttachmentsCount = (int)vboCurrentAttachments.size();

    if (0 == currentAttachmentsCount)
    {
        strBufferCurrentAttachments == AF_STR_None;
    }
    else
    {
        gtString currentAttachment;

        for (int i = 0; i < currentAttachmentsCount; ++i)
        {
            if (0 < i)
            {
                strBufferCurrentAttachments.append(L", ");
            }

            apGLenumValueToString(vboCurrentAttachments[i], currentAttachment);
            strBufferCurrentAttachments.append(currentAttachment);
        }
    }

    // Get buffer size:
    gtSize_t bufferSize = vboDetails.size();

    if (bufferSize > 0)
    {
        // We have the information in bits, convert it to kilobytes
        bufferSize = (gtSize_t)ceil((float)bufferSize / (1024.0F));

        if (bufferSize == 0)
        {
            bufferSize = 1;
        }
    }

    // Build the VBO size string:
    gtString strBufferSize;
    strBufferSize.appendFormattedString(L"%u", bufferSize);
    strBufferSize.addThousandSeperators();
    strBufferSize.append(AF_STR_KilobytesShort);

    // Build the buffer name:
    gtString strBufferName;
    gdGetVBODisplayName(vboDetails, strBufferName);

    // Build the OpenCL buffer link:
    gtString openCLBufferLink;

    if (vboDetails.openCLSpyID() > 0)
    {
        openCLBufferLink.makeEmpty();
        openCLBufferLink.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesCLBufferLink, vboDetails.openCLBufferIndex(), vboDetails.openCLSpyID());
        openCLBufferLink.appendFormattedString(GD_STR_PropertiesCLBufferFullName, vboDetails.openCLSpyID() , vboDetails.openCLBufferName());
        openCLBufferLink.append(GD_STR_HtmlPropertiesLinkEnd);
    }

    // Build the HTML content:
    htmlContent.setTitle(strBufferName);

    // Build the VBO name (with link):
    gtString vboNameWithLink = strBufferName;

    if (showDetailsLink)
    {
        vboNameWithLink.prependFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesGLVBOLink, vboDetails.name(), contextID._contextId);
        vboNameWithLink.append(GD_STR_HtmlPropertiesLinkEnd);
    }

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBufferNameTitle, vboNameWithLink);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_Size, strBufferSize);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBufferAttachmentTitle, strBufferLastAttachment);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBufferCurrentAttachmentsTitle, strBufferCurrentAttachments);

    if (!openCLBufferLink.isEmpty())
    {
        // Add the buffer OpenCL shared buffer:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesGLBufferCLShared, openCLBufferLink);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildTexBufferHTMLPropertiesString
// Description: Build a texture buffer properties in HTML format
// Arguments: contextID - the render context id
//            const apGLTexture textureBufferDetails - the texture object details
//            bAddThumbnail - should add thumbnail to the texture buffer properties message
//            afHTMLContent& htmlContent
//            afProgressBarWrapper *pProgressBar - pointer to progress bar for update
// Return Val: void
// Author:      Sigal Algranaty
// Date:        23/10/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildTexBufferHTMLPropertiesString(const apContextID& contextID, const apGLTextureMiplevelData& textureBufferDetails, afHTMLContent& htmlContent, bool bAddThumbnail, afProgressBarWrapper* pProgressBar)
{
    // Create the tex buffer thumbnail if necessary:
    gtString texBufferThumbnailStr;

    if (bAddThumbnail)
    {
        // Generate the texture thumbnail:
        // Will hold the generated texture path and dimensions:
        osFilePath texturePreviewPath;
        int imageWidth = 0;
        int imageHeight = 0;

        // Generate the texture preview:
        apGLTexture textureDetails;
        bool rc0 = gaGetTextureObjectDetails(contextID._contextId, textureBufferDetails.textureName(), textureDetails);
        GT_IF_WITH_ASSERT(rc0)
        {
            bool rc1 = generateObjectPreview(contextID, AP_HTML_TEXTURE, textureBufferDetails.textureName(), AP_DISPLAY_BUFFER_UNKNOWN, texturePreviewPath, imageWidth, imageHeight, pProgressBar);

            if (rc1)
            {
                // Image was generated successfully - add the image to the properties view
                texBufferThumbnailStr.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesGLTextureLink, textureBufferDetails.textureName(), contextID._contextId);
                texBufferThumbnailStr.appendFormattedString(AF_STR_HtmlImageWithSizeSpecsAndRightAlign, texturePreviewPath.asString().asCharArray(), imageWidth, imageHeight);
                texBufferThumbnailStr += GD_STR_HtmlPropertiesLinkEnd;
            }
        }
    }

    // Get attachment as string:
    gtString strBufferAttachment;

    // Get buffer size (from the VBO attached to the texture buffer):
    apGLVBO vboDetails;
    gtSize_t bufferSize = 0;
    int texWidth = 0;
    bool rc = gaGetVBODetails(contextID._contextId, textureBufferDetails.bufferName(), vboDetails);
    GT_IF_WITH_ASSERT(rc)
    {
        bufferSize = vboDetails.size();
        texWidth = bufferSize;

        if (bufferSize > 0)
        {
            // We have the information in bits, convert it to kilobytes
            bufferSize = (gtSize_t)ceil((float)bufferSize / (1024.0F));

            if (bufferSize == 0)
            {
                bufferSize = 1;
            }

            // Get the pixel format size to calculate the texture width:
            GLuint pixelSize = 0;
            bool rc2 = apGetPixelSizeInBitsByInternalFormat(textureBufferDetails.bufferInternalFormat(), pixelSize);
            GT_IF_WITH_ASSERT(rc2)
            {
                float newSize = (float)texWidth;
                newSize = (newSize / (float)pixelSize);
                texWidth = (int)floor(newSize);
            }
        }
    }

    gtString strBufferSize;
    strBufferSize.appendFormattedString(L"%u", bufferSize);
    strBufferSize.addThousandSeperators();
    strBufferSize.append(AF_STR_KilobytesShort);

    gtString strBufferName;
    strBufferName.appendFormattedString(L"%d", textureBufferDetails.bufferName());

    // Get the texture type:
    apTextureType textureType = textureBufferDetails.textureType();

    // Get the texture type string:
    gtString strTextureType, strTextureDimensions;
    apTextureTypeAsString(textureType, strTextureType);
    strTextureDimensions.appendFormattedString(GD_STR_Properties1DDimensions, texWidth);

    htmlContent.clear();

    gtString strTextureName;
    strTextureName.appendFormattedString(GD_STR_PropertiesTextureNameFormat, textureBufferDetails.textureName());

    // Build the html content:
    htmlContent.setTitle(strTextureName);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    // Add texture type line:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_Type, strTextureType.append(GD_STR_PropertiesTextureSuffix));

    // Add the texture dimensions:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDimensions, strTextureDimensions);

    // Add the buffer name:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesAttachedBufferName, strBufferName);

    // Add buffer size:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBufferSize, strBufferSize);

    // Get the texture buffer data format:
    GLenum openGLDataFormat = textureBufferDetails.bufferInternalFormat();
    oaTexelDataFormat dataFormat = OA_TEXEL_FORMAT_UNKNOWN;
    bool rc2 = oaGLEnumToTexelDataFormat(openGLDataFormat, dataFormat);
    GT_IF_WITH_ASSERT(rc2)
    {
        oaDataType componentDataType;
        // Convert the format to data type:
        bool rc3 = apGLTexture::textureBufferFormatToDataType(openGLDataFormat, componentDataType);
        GT_IF_WITH_ASSERT(rc3)
        {
            // Translate the buffer format to a string:
            gtString strBufferDataFormat;
            gtString strBufferDataType;
            apGLenumValueToString(openGLDataFormat, strBufferDataFormat);

            // Buffer Data type:
            GLenum bufferDataTypeEnum = oaDataTypeToGLEnum(componentDataType);
            apGLenumValueToString(bufferDataTypeEnum, strBufferDataType);

            // Add "Internal Pixel Format" subtitle:
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesInternalPixelFormat);

            // Add the pixel format details:
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPixelFormatFormat, strBufferDataFormat);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_Type, strBufferDataType);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildProgramsListHTMLPropertiesString
// Description: Builds a message for the list of shading programs in a given RC
// Author:      Uri Shomroni
// Date:        16/11/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildProgramsListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects)
{
    (void)(numberOfObjects);  // unused
    gtString programsHeadline;
    programsHeadline.appendFormattedString(GD_STR_PropertiesGLProgramsListHeadline, contextID._contextId);

    // Build the string of program explanations:
    gtString explanation;
    explanation.append(GD_STR_PropertiesMemorySizeProgramsExplanation AF_STR_HtmlNewLine AF_STR_HtmlNewLine GD_STR_PropertiesMemorySizeNAExplanation);

    // Build the HTML content:
    htmlContent.setTitle(programsHeadline);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, explanation);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesSelectItemMessage);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildProgramHTMLPropertiesString
// Description: Builds a message for a given shading program in a given RC
// Author:      Uri Shomroni
// Date:        16/11/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildProgramHTMLPropertiesString(int contextId, const apGLProgram& programDetails, bool showActiveUniforms, afHTMLContent& htmlContent)
{
    // Get program details:
    gtString programLinkStatus;
    gtString progNameString;
    gtString markedForDeletionStatus;
    gtString usedInLastFrameStatus;
    gtString programLinkLogStr;

    // Get the program name:
    GLuint progName = programDetails.programName();
    progNameString.appendFormattedString(GD_STR_PropertiesProgramNameFormat, progName);

    // Get the status of IsProgramLinked:
    bool isProgramLinked = programDetails.isProgramLinkedSuccesfully();
    programLinkStatus = (isProgramLinked) ?  AF_STR_Yes : AF_STR_No;

    // Build marked for deletion status:
    markedForDeletionStatus = (programDetails.isMarkedForDeletion()) ?  AF_STR_Yes : AF_STR_No;

    // Build the used in last frame status;
    usedInLastFrameStatus = (programDetails.wasUsedInLastFrame()) ?  AF_STR_Yes : AF_STR_No;

    // Get the link Log
    gtString programLinkLog;
    programLinkLog = programDetails.programLinkLog().asCharArray();

    // Replace the \n with <br> to show the matrix value in HTMLCtrl
    programLinkLog.replace(AF_STR_NewLine, AF_STR_HtmlNewLine, true);

    // Build the program link log string:
    programLinkLogStr = (programLinkLog == AF_STR_Empty) ? AF_STR_EmptyStr : programLinkLog;

    // Build the HTML content:
    htmlContent.setTitle(progNameString);

    // Build the attached shaders string:
    gtString attachedShadersStr;

    gtList<GLuint>::const_iterator iter = programDetails.shaderObjects().begin();
    gtList<GLuint>::const_iterator endIter = programDetails.shaderObjects().end();

    if (iter == endIter)
    {
        // There are no attached shaders:
        attachedShadersStr = AF_STR_None;
    }
    else
    {
        // Display a list of the shaders attached to this program:
        while (iter != endIter)
        {
            gtAutoPtr<apGLShaderObject> aptrCurrentShader = NULL;
            bool rcShad = gaGetShaderObjectDetails(contextId, (*iter), aptrCurrentShader);
            GT_IF_WITH_ASSERT(rcShad && (aptrCurrentShader.pointedObject() != NULL))
            {
                afTreeItemType objectType = AF_TREE_ITEM_ITEM_NONE;
                gdShaderType curShadType = gdShaderTypeFromTransferableObjectType(aptrCurrentShader->type(), objectType);
                gtString curShadString;
                GLuint curShadName = aptrCurrentShader->shaderName();
                gdShaderNameStringFromNameAndType(curShadName, curShadType, curShadString);

                const wchar_t* shaderTypeLink = GD_STR_HtmlPropertiesGLUnsupportedShaderLink;

                switch (curShadType)
                {
                    case GD_VERTEX_SHADER:
                        shaderTypeLink = GD_STR_HtmlPropertiesGLVertexShaderLink;
                        break;

                    case GD_TESSELLATION_CONTROL_SHADER:
                        shaderTypeLink = GD_STR_HtmlPropertiesGLTessellationControlShaderLink;
                        break;

                    case GD_TESSELLATION_EVALUATION_SHADER:
                        shaderTypeLink = GD_STR_HtmlPropertiesGLTessellationEvaluationShaderLink;
                        break;

                    case GD_GEOMETRY_SHADER:
                        shaderTypeLink = GD_STR_HtmlPropertiesGLGeometryShaderLink;
                        break;

                    case GD_FRAGMENT_SHADER:
                        shaderTypeLink = GD_STR_HtmlPropertiesGLFragmentShaderLink;
                        break;

                    case GD_UNSUPPORTED_SHADER:
                    case GD_UNKNOWN_SHADER:
                    default:
                        shaderTypeLink = GD_STR_HtmlPropertiesGLUnsupportedShaderLink;
                        break;
                }

                // Make the shader name into a link:
                curShadString.prependFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, shaderTypeLink, curShadName, contextId).append(GD_STR_HtmlPropertiesLinkEnd);

                // Build the shader title:
                gtString shaderTypeAsString;
                gdShaderTypeToString(curShadType, shaderTypeAsString);

                // There are no attached shaders:
                if (!attachedShadersStr.isEmpty())
                {
                    // Add , to the shaders links string:
                    attachedShadersStr.append(L", ");
                }

                attachedShadersStr.append(curShadString);
            }
            iter++;
        }
    }

    // Add the attached shaders links to the HTML table:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesProgramAttachedShadersHeader, attachedShadersStr);

    // Add the link status to the table:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesProgramIsLinked, programLinkStatus);

    // Add "Marked for deletion" to the table:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesProgramIsMarkedForDeletion, markedForDeletionStatus);

    // Add UsedInLastFrame to the table:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesProgramWasUsedInLastFrame, usedInLastFrameStatus);

    // Add the program link log to the table:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesProgramLinkLog, programLinkLogStr);

    // Add program geometry parameters:
    addProgramGeometryParametersToHTMLContent(htmlContent, programDetails);

    if (showActiveUniforms)
    {
        // Add the program's active uniform types:
        htmlContent.addSpaceLines(3);
        bool rcUnif = addActiveUniformsToHTMLContext(contextId, progName, htmlContent);
        GT_ASSERT(rcUnif)
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildShadersListHTMLPropertiesString
// Description: Builds a message for the shaders list of a given render context.
// Arguments: apContextID contextID
//            afHTMLContent& htmlContent
//            int numberOfShadersTypes
//            int* pShadersNumbersByType
// Return Val: void
// Author:      Uri Shomroni
// Date:        16/11/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildShadersListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, gtMap<gdShaderType, int>& shaderTypesAmountsMap)
{
    // Create the Header "Context ### Shaders"
    gtString shadersHeadline;
    shadersHeadline.appendFormattedString(GD_STR_PropertiesShadersListHeadline, contextID._contextId);

    // Build the HTML content:
    htmlContent.setTitle(shadersHeadline);

    int amountOfShaderTypes = (int)shaderTypesAmountsMap.size();

    if (amountOfShaderTypes > 0)
    {
        // Add the Shaders Type / Amount Titles:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesShaderTypeTitle, GD_STR_PropertiesAmountTitle);

        // Create the shaders types table:
        gtString shaderTypesTable;

        // Count total number of shaders:
        int totalNumberOfShaders = 0;

        for (int i = 0; i < amountOfShaderTypes; i++)
        {
            // Get the current shader type:
            gdShaderType shaderType = (gdShaderType)i;

            // Get the amount of current shader type:
            int amountOfCurrentShader = shaderTypesAmountsMap[shaderType];

            // The last item is "Unknown Shader". Display it only if we have any unknown shaders:
            if ((i != GD_UNKNOWN_SHADER) || (amountOfShaderTypes > 0))
            {
                // Get the shader type:
                shaderType = (gdShaderType)i;
                gtString shaderTypeAsString;
                gdShaderTypeToString(shaderType, shaderTypeAsString);

                // Add the shader type name and value line to the table:
                gtString shadersTypeAmountStr;
                shadersTypeAmountStr.appendFormattedString(L"%d", amountOfCurrentShader);
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, shaderTypeAsString, shadersTypeAmountStr);

                // Add the current texture type amount to the total number of textures:
                totalNumberOfShaders += amountOfCurrentShader;
            }
        }

        // Add the "Total" line:
        htmlContent.addHTMLTotalLine(totalNumberOfShaders);

    }

    // Add the "Select an item" line:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesSelectItemMessage);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildShaderHTMLPropertiesString
// Description: Builds a message for a given shader in a given RC
// Arguments: apContextID contextID
//            const apGLShaderObject& shader
//            bool isExpanded - the expanded properties page is for the shader source
//                              code viewer, while the short one is for the memory viewer
//            afHTMLContent& htmlContent - the properties string
// Author:      Uri Shomroni
// Date:        17/11/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildShaderHTMLPropertiesString(const apContextID& contextID, const apGLShaderObject& shader, bool isExpanded, afHTMLContent& htmlContent)
{
    gtString shadNameString;
    gtString sourceCodeLengthStr;
    gtString sourceCodeFilePathStr;
    gtString shadersCompileLog;
    gtString markedForDeletionStatus;
    gtString compilationStatus;
    gtString compilationLogStr;

    // Get the shader name title:
    GLuint shadName = shader.shaderName();
    shadNameString.appendFormattedString(GD_STR_PropertiesShaderNameFormat, shadName);

    // Build the shader type string:
    afTreeItemType objectType = AF_TREE_ITEM_ITEM_NONE;
    gdShaderType shadType = gdShaderTypeFromTransferableObjectType(shader.type(), objectType);
    gtString shderTypeString;
    gdShaderTypeToString(shadType, shderTypeString);

    // Add the shader's source code length:
    unsigned long sourceCodeLength = shader.sourceCodeLength();

    // Note that the source size is given in bytes, so no need to divide to convert to chars.
    sourceCodeLengthStr.appendFormattedString(L"%d", sourceCodeLength);

    // Add the "marked for deletion" status:
    markedForDeletionStatus = (shader.isMarkedForDeletion()) ?  AF_STR_Yes : AF_STR_No;

    if (isExpanded)
    {
        shadersCompileLog = shader.compilationLog().asCharArray();

        // Replace the \n with <br> to show the matrix value in HTMLCtrl
        shadersCompileLog.replace(AF_STR_NewLine, AF_STR_HtmlNewLine, true);

        // Replace empty value with N/A:
        validateValueAvailability(shadersCompileLog);

        // Build the compilation status string:
        compilationStatus = (shader.isShaderCompiled()) ?  AF_STR_Yes : AF_STR_No;

        // Build the source code path string:
        sourceCodeFilePathStr = shader.sourceCodeFilePath().asString();
    }

    // Build the HTML content:
    htmlContent.setTitle(shadNameString);

    // Add "General" Subtitle:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    // Shader name with link:
    gtString shaderNameWithLinkStr = shadNameString;

    if (!isExpanded)
    {
        const wchar_t* shaderTypeLink = GD_STR_HtmlPropertiesGLUnsupportedShaderLink;

        switch (shadType)
        {
            case GD_VERTEX_SHADER:
                shaderTypeLink = GD_STR_HtmlPropertiesGLVertexShaderLink;
                break;

            case GD_TESSELLATION_CONTROL_SHADER:
                shaderTypeLink = GD_STR_HtmlPropertiesGLTessellationControlShaderLink;
                break;

            case GD_TESSELLATION_EVALUATION_SHADER:
                shaderTypeLink = GD_STR_HtmlPropertiesGLTessellationEvaluationShaderLink;
                break;

            case GD_GEOMETRY_SHADER:
                shaderTypeLink = GD_STR_HtmlPropertiesGLGeometryShaderLink;
                break;

            case GD_FRAGMENT_SHADER:
                shaderTypeLink = GD_STR_HtmlPropertiesGLFragmentShaderLink;
                break;

            case GD_UNSUPPORTED_SHADER:
            case GD_UNKNOWN_SHADER:
            default:
                shaderTypeLink = GD_STR_HtmlPropertiesGLUnsupportedShaderLink;
                break;
        }

        shaderNameWithLinkStr.prependFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, shaderTypeLink, shadName, contextID._contextId);
        shaderNameWithLinkStr.append(GD_STR_HtmlPropertiesLinkEnd);
    }

    // Add Shader name line:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesShaderName, shaderNameWithLinkStr);

    // Add shader type line:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesShaderTypeTitle, shderTypeString);

    // Add shader code length:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesShaderSourceCodeLengthTitle, sourceCodeLengthStr);

    // Add "Marked for deletion" status:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesShaderMarkedForDeletion, markedForDeletionStatus);

    if (isExpanded)
    {
        // Add the compilation status of the shader:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesShaderIsCompiled, compilationStatus);

        // Add the shader source code files path to the table(the file path is added in a new line, since it is long!:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesShaderFilePath, sourceCodeFilePathStr);

        // Add the shader compilation log:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesShaderCompileLog, shadersCompileLog);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildDisplayListsListHTMLPropertiesString
// Description: Builds a message for the list of display lists in a given context
// Author:      Uri Shomroni
// Date:        17/11/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildDisplayListsListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects)
{
    (void)(numberOfObjects);  // unused
    // Create the Header "Context #n Display Lists"
    gtString displayListsTitle;
    displayListsTitle.appendFormattedString(GD_STR_PropertiesDisplayListsListHeadline, contextID._contextId);

    // Build the HTML content:
    htmlContent.setTitle(displayListsTitle);

    // Add the "We do not count display lists' sizes" explanation:
    gtString displayListExplanation;
    displayListExplanation.append(GD_STR_PropertiesMemorySizeDisplayListsExplanation AF_STR_HtmlNewLine AF_STR_HtmlNewLine GD_STR_PropertiesMemorySizeNAExplanation);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, displayListExplanation);

    // Add the "Select an item" message:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesSelectItemMessage);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildDisplayListHTMLPropertiesString
// Description: Builds message for a given display list in a given context
// Author:      Uri Shomroni
// Date:        17/11/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildDisplayListHTMLPropertiesString(const apContextID& contextID, const apGLDisplayList& dpyList, afHTMLContent& htmlContent)
{
    (void)(contextID);  // unused
    // Build the display list title:
    GLuint dpyListName = dpyList.getDisplayListName();
    gtString dpyListNameString;
    dpyListNameString.appendFormattedString(GD_STR_PropertiesDisplayListNameFormat, dpyListName);

    gtString dpyListModeString;

    if (dpyList.getDisplayListMode() != 0)
    {
        apGLenumValueToString(dpyList.getDisplayListMode(), dpyListModeString);
    }
    else
    {
        dpyListModeString = AF_STR_NotAvailable;
    }

    // Add the "We do not count display lists' sizes" explanation:
    gtString displayListExplanation;
    displayListExplanation.append(GD_STR_PropertiesMemorySizeDisplayListsExplanation AF_STR_HtmlNewLine GD_STR_PropertiesMemorySizeNAExplanation);

    // Build the HTML content:
    htmlContent.setTitle(dpyListNameString);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDisplayListListMode, dpyListModeString);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, displayListExplanation);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildFBOsListHTMLPropertiesString
// Description: Builds a message for the FBO list in a given context
// Author:      Uri Shomroni
// Date:        17/11/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildFBOsListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects)
{
    (void)(numberOfObjects);  // unused

    gtString fbosListTitle;
    fbosListTitle.appendFormattedString(GD_STR_PropertiesFBOsListHeadline, contextID._contextId);

    htmlContent.setTitle(fbosListTitle);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesSelectItemMessage);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildFBOHTMLPropertiesString
// Description: Builds a message for a given FBO in a given context
// Author:      Uri Shomroni
// Date:        17/11/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildFBOHTMLPropertiesString(const apContextID& contextID, const apGLFBO& fbo, bool forMemoryViewer, afHTMLContent& htmlContent)
{
    // Get the FBO name:
    GLuint fboName = fbo.getFBOName();
    gtString fboNameString;
    fboNameString.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeFBOName, fboName);

    // Build the HTML content:
    htmlContent.setTitle(fboNameString);

    // Add "General" Subtitle:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    // Add the buffer name:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBufferNameTitle, fboNameString);

    // Add the "Amount of bound objects" line:
    gtString strAmountOfBoundObjects;
    strAmountOfBoundObjects.appendFormattedString(L"%d", fbo.amountOfBindedObjects());
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesFBONumberOfBoundObjects, strAmountOfBoundObjects);

    if (forMemoryViewer)
    {
        // Add the "more info" link:
        gtString moreInfoStr;
        moreInfoStr.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesGLFBOLink, fboName, contextID._contextId);
        moreInfoStr.appendFormattedString(GD_STR_PropertiesViewDetailsTemplate, fboNameString.asCharArray());
        moreInfoStr.append(GD_STR_HtmlPropertiesLinkEnd);

        // Add the buffer attachment to the table:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_ITEM_LINK, moreInfoStr);
    }
    else
    {

        // Get the FBO binded objects:
        gtList<apFBOBindObject*> bindedObjects = fbo.getBindedObjects();

        if (bindedObjects.length() > 0)
        {
            // Add FBO attached objects title:
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesFBOAttachedObjects);

            gtList<apFBOBindObject*>::const_iterator iter = bindedObjects.begin();
            gtList<apFBOBindObject*>::const_iterator iterEnd = bindedObjects.end();

            for (; iter != iterEnd ; iter++)
            {
                // Get the current binded object:
                apFBOBindObject* pBindedObject = (*iter);

                if (pBindedObject != NULL)
                {
                    // Build the string describing the bind object:
                    gtString bindedObjectStr, bindedTargetStr;

                    // Get the buffer type:
                    apDisplayBuffer bufferType = apGLEnumToColorIndexBufferType(pBindedObject->_attachmentPoint);

                    // Get the buffer short name:
                    bool rc1 = apGetBufferShortName(bufferType, bindedTargetStr);
                    GT_ASSERT(rc1);

                    // Get the buffer attachment string:
                    gtString attachmentTargetStr;
                    bool rc2 = apGLFBO::fboAttachmentTargetToString(pBindedObject->_attachmentTarget, attachmentTargetStr);
                    GT_ASSERT(rc2);

                    // Build the binded object string:
                    bindedObjectStr.appendFormattedString(L"%ls %u", attachmentTargetStr.asCharArray(), pBindedObject->_name);

                    // Check if this is a texture attachment:
                    bool isTextureAttachment = false;
                    bool rc = apGLFBO::isTextureAttachmentTarget(pBindedObject->_attachmentTarget, isTextureAttachment);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        if (isTextureAttachment && (pBindedObject->_textureLayer != 0))
                        {
                            // Add the texture layer to the binded object string:
                            bindedObjectStr.appendFormattedString(L" (Layer %d)", pBindedObject->_textureLayer);
                        }
                    }

                    // Append the binded object string to the HTML message:
                    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, bindedTargetStr, bindedObjectStr);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildSearchingForMemoryLeakEventHTMLPropertiesString
// Description: Searching for memory leaks event string
// Arguments:   const apSearchingForMemoryLeaskEvent& event
//              gtString htmlPropertiesString
// Author:      Sigal Algranaty
// Date:        26/3/2012
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildSearchingForMemoryLeakEventHTMLPropertiesString(const apSearchingForMemoryLeaksEvent& eve, afHTMLContent& htmlContent)
{
    // Build the HTML content:
    htmlContent.setTitle(GD_STR_ProcessEventsViewPropertiesTitleMemoryLeak);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, eve.message());
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewPropertiesTitleSearchingForMemoryLeaksDescription);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildMemoryLeakEventHTMLPropertiesString
// Description: Builds a message specifying and explaining a memory leak from an event
// Author:      Uri Shomroni
// Date:        11/11/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildMemoryLeakEventHTMLPropertiesString(const apMemoryLeakEvent& eve, bool displayIcon, afHTMLContent& htmlContent)
{
    gtString thumbnailStr;

    if (displayIcon)
    {
        gtString iconPath;
        bool rcPath = afGetApplicationImagesPath(iconPath);
        GT_ASSERT(rcPath);
        iconPath.append(osFilePath::osPathSeparator);
        iconPath.append(AF_STR_WarningYellowIconFileName);
        thumbnailStr.appendFormattedString(AF_STR_HtmlImage AF_STR_HtmlPropertiesNonbreakingSpace, iconPath.asCharArray());
    }

    // Build the HTML content:
    htmlContent.setTitle(GD_STR_ProcessEventsViewPropertiesTitleMemoryLeak, thumbnailStr);

    if (eve.memoryLeakExists())
    {
        // Which kind of leak is this?
        apMemoryLeakEvent::apMemoryLeakType leakType = eve.leakType();

        switch (leakType)
        {
            case apMemoryLeakEvent::AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK:
            {
                // Add the leak type:
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_MemoryLeakAppTerminationLeakTypeHeading, GD_STR_MemoryLeakAppTerminationLeakTypeRC);

                // Get the render contexts information:
                gtString leakedRCsStr;
                bool rcRC = eve.leakingRenderContextsAsString(leakedRCsStr);
                GT_ASSERT(rcRC);

                if (leakedRCsStr.isEmpty())
                {
                    leakedRCsStr = AF_STR_None;
                }

                // Add the leaked render contexts lists:
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_MemoryLeakAppTerminationRenderContextsHTML, leakedRCsStr);

                // Get the Pixel Buffer information:
                gtString leakedPBOsStr;
                bool rcPBO = eve.leakingPBuffersAsString(leakedPBOsStr);
                GT_ASSERT(rcPBO);

                if (leakedPBOsStr.isEmpty())
                {
                    leakedPBOsStr = AF_STR_None;
                }

                // Add the leaked pbuffers:
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_MemoryLeakAppTerminationPBuffersHTML, leakedPBOsStr);

                // Get the sync objects information:
                gtString leakedSyncObjectsStr;
                bool rcSync = eve.leakingSyncObjectsAsString(leakedSyncObjectsStr);
                GT_ASSERT(rcSync);

                if (leakedSyncObjectsStr.isEmpty())
                {
                    leakedSyncObjectsStr = AF_STR_None;
                }

                // Add the leaked sync objects:
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_MemoryLeakAppTerminationSyncObjectsHTML, leakedSyncObjectsStr);

                // Add the explanation:
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_MemoryLeakAppTerminationDetailsHeading , GD_STR_MemoryLeakAppTerminationDetailsGL);
            }
            break;

            case apMemoryLeakEvent::AP_INDEPENDENT_CL_ALLOCATED_OBJECT_LEAK:
            {
                // Add the leak type:
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_MemoryLeakAppTerminationLeakTypeHeading, GD_STR_MemoryLeakAppTerminationLeakTypeCLC);

                // Get the compute contexts information:
                gtString leakedCtxsStr;
                bool rcCtx = eve.leakingComputeContextsAsString(leakedCtxsStr);
                GT_ASSERT(rcCtx);

                if (leakedCtxsStr.isEmpty())
                {
                    leakedCtxsStr = AF_STR_None;
                }

                // Add the leaked compute contexts lists:
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_MemoryLeakAppTerminationComputeContextsHTML, leakedCtxsStr);

                // Add the explanation:
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_MemoryLeakAppTerminationDetailsHeading , GD_STR_MemoryLeakAppTerminationDetailsCL);
            }
            break;

            case apMemoryLeakEvent::AP_GL_CONTEXT_ALLOCATED_OBJECT_LEAK:
            {
                // Add the leak type:
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_MemoryLeakAppTerminationLeakTypeHeading, GD_STR_MemoryLeakAllocatedObjectsLeakTypeGraphic);

                // Add the render context id:
                gtString renderContextID;
                renderContextID.appendFormattedString(L"%u", eve.leakingObjectsRenderContextID());
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_MemoryLeakAllocatedObjectsRenderContextHTML, renderContextID);

                int leakTex = -1;
                int leakSmp = -1;
                int leakRBO = -1;
                int leakFBO = -1;
                int leakVBO = -1;
                int leakProg = -1;
                int leakShad = -1;
                int leakPipe = -1;
                int leakList = -1;
                eve.leakingAllocatedGLObjects(leakTex, leakSmp, leakRBO, leakFBO, leakVBO, leakProg, leakShad, leakPipe, leakList);
                gtString interimString;

                if (leakTex > 0)
                {
                    interimString.appendFormattedString(GD_STR_MemoryLeakAllocatedObjectsTextures, leakTex);
                }

                if (leakSmp > 0)
                {
                    if (!interimString.isEmpty())
                    {
                        interimString.append(L", ");
                    }

                    interimString.appendFormattedString(GD_STR_MemoryLeakAllocatedObjectsGLSamplers, leakSmp);
                }

                if (leakRBO > 0)
                {
                    if (!interimString.isEmpty())
                    {
                        interimString.append(L", ");
                    }

                    interimString.appendFormattedString(GD_STR_MemoryLeakAllocatedObjectsRenderBuffers, leakRBO);
                }

                if (leakFBO > 0)
                {
                    if (!interimString.isEmpty())
                    {
                        interimString.append(L", ");
                    }

                    interimString.appendFormattedString(GD_STR_MemoryLeakAllocatedObjectsFBOs, leakFBO);
                }

                if (leakVBO > 0)
                {
                    if (!interimString.isEmpty())
                    {
                        interimString.append(L", ");
                    }

                    interimString.appendFormattedString(GD_STR_MemoryLeakAllocatedObjectsVBOs, leakVBO);
                }

                if (leakProg > 0)
                {
                    if (!interimString.isEmpty())
                    {
                        interimString.append(L", ");
                    }

                    interimString.appendFormattedString(GD_STR_MemoryLeakAllocatedObjectsGLPrograms, leakProg);
                }

                if (leakShad > 0)
                {
                    if (!interimString.isEmpty())
                    {
                        interimString.append(L", ");
                    }

                    interimString.appendFormattedString(GD_STR_MemoryLeakAllocatedObjectsShaders, leakShad);
                }

                if (leakPipe > 0)
                {
                    if (!interimString.isEmpty())
                    {
                        interimString.append(L", ");
                    }

                    interimString.appendFormattedString(GD_STR_MemoryLeakAllocatedObjectsPipelines, leakPipe);
                }

                if (leakList > 0)
                {
                    if (!interimString.isEmpty())
                    {
                        interimString.append(L", ");
                    }

                    interimString.appendFormattedString(GD_STR_MemoryLeakAllocatedObjectsDisplayLists, leakList);
                }

                // Make sure we have valid information:
                GT_IF_WITH_ASSERT(!interimString.isEmpty())
                {
                    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_MemoryLeakAllocatedObjectsObjectTypesHTML, interimString);
                }

                // Add the explanation:
                gtString detailsStr;
                detailsStr.appendFormattedString(GD_STR_MemoryLeakAllocatedObjectsDetailsHTML, eve.leakingObjectsRenderContextID());
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_MemoryLeakAppTerminationDetailsHeading, detailsStr);
            }
            break;

            case apMemoryLeakEvent::AP_CL_CONTEXT_ALLOCATED_OBJECT_LEAK:
            {
                // TO_DO: OpenCL
            }
            break;

            case apMemoryLeakEvent::AP_CL_PROGRAM_ALLOCATED_OBJECT_LEAK:
            {
                // TO_DO: OpenCL
            }
            break;

            default:
            {
                // We added a new leak type but didn't implement it here:
                GT_ASSERT(false);
            }
            break;
        }

    }
    else
    {
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AP_STR_MemoryLeakNone);
    }

}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildRenderBuffersListHTMLPropertiesString
// Description: builds a message for the render buffers list in the given render context
// Author:      Uri Shomroni
// Date:        6/11/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildRenderBuffersListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects)
{
    (void)(numberOfObjects);  // unused
    gtString renderBuffersListTitle;
    renderBuffersListTitle.appendFormattedString(GD_STR_PropertiesRenderBuffersListHeadline, contextID._contextId);

    htmlContent.setTitle(renderBuffersListTitle);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesSelectItemMessage);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildRenderBufferHTMLPropertiesString
// Description: Builds a render buffer properties in HTML format
// Arguments: apContextID contextID - the render context id
//            const apGLRenderBuffer& renderBufferDetails
//            bAddThumbnail - should add thumbnail to the render buffer properties message
//            , acFrame* pCallingWindow - the calling window (for the progress update)
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/10/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildRenderBufferHTMLPropertiesString(const apContextID& contextID, const apGLRenderBuffer& renderBufferDetails, bool bAddThumbnail, afProgressBarWrapper* pProgressBar, afHTMLContent& htmlContent)
{
    // Get buffer type:
    gtString strBufferType;
    bool rc = apGetBufferName(renderBufferDetails.getBufferType(), strBufferType);

    if (!rc)
    {
        // Buffer is not connected to an FBO, therefore it's type is unknown, and also legal.
        strBufferType = GD_STR_PropertiesRenderBufferUnAttached;
    }

    // Get buffer dimensions:
    GLint bufferWidth, bufferHeight;
    gtString strBufferDimensions;
    renderBufferDetails.getBufferDimensions(bufferWidth, bufferHeight);
    strBufferDimensions.appendFormattedString(GD_STR_Properties2DDimensions, bufferWidth, bufferHeight);

    // Build the buffer thumbnail string:
    gtString bufferThumbnailStr;

    if (bAddThumbnail)
    {
        // Generate the texture thumbnail:
        // Will hold the generated texture path and dimensions
        osFilePath texturePreviewPath;
        int imageWidth = 0;
        int imageHeight = 0;

        bool rc1 = generateObjectPreview(contextID, AP_HTML_RENDER_BUFFER, renderBufferDetails.renderBufferName(), AP_DISPLAY_BUFFER_UNKNOWN, texturePreviewPath, imageWidth, imageHeight, pProgressBar);

        if (rc1)
        {
            // Image was generated successfully - add the image to the properties view
            bufferThumbnailStr.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesGLRenderBufferLink, renderBufferDetails.renderBufferName(), contextID._contextId);
            bufferThumbnailStr.appendFormattedString(AF_STR_HtmlImageWithSizeSpecsAndRightAlign, texturePreviewPath.asString().asCharArray(), imageWidth, imageHeight);
            bufferThumbnailStr += GD_STR_HtmlPropertiesLinkEnd;
        }
    }

    // Build the OpenCL image link:
    gtString strBufferCLLink;
    int clImageIndex = -1, clImageName = -1, clSpyId = -1;
    renderBufferDetails.getCLImageDetails(clImageIndex, clImageName, clSpyId);

    if ((clImageIndex >= 0) && (clSpyId > 0))
    {
        strBufferCLLink.makeEmpty();
        strBufferCLLink.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesCLImageLink, clImageIndex, clSpyId);
        strBufferCLLink.appendFormattedString(GD_STR_PropertiesImageNameFullFormat, clSpyId, clImageName);
        strBufferCLLink.append(GD_STR_HtmlPropertiesLinkEnd);
    }

    // Get buffer format:
    oaTexelDataFormat bufferDataFormat;
    oaDataType bufferDataType;
    gtString strBufferDataType, strBufferDataFormat;
    bufferDataFormat = renderBufferDetails.bufferFormat();
    bufferDataType = renderBufferDetails.bufferDataType();

    // Buffer Data Type:
    GLenum bufferDataTypeEnum = oaDataTypeToGLEnum(bufferDataType);
    apGLenumValueToString(bufferDataTypeEnum, strBufferDataType);

    // Buffer Data Format:
    GLenum bufferDataFormatEnum = oaTexelDataFormatToGLEnum(bufferDataFormat);
    apGLenumValueToString(bufferDataFormatEnum, strBufferDataFormat);

    // Buffer owner FBO:
    gtString strBufferFBOName;
    GLuint fboName = renderBufferDetails.getFBOName();
    strBufferFBOName.appendFormattedString(L"%u", fboName);

    // Build the buffer name string:
    gtString bufferNameStr;
    bufferNameStr.appendFormattedString(GD_STR_PropertiesRenderBufferHeadline, renderBufferDetails.renderBufferName());

    // Build the HTML content:
    htmlContent.setTitle(bufferNameStr);

    // Add "General" Subtitle:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    // Add the buffer type:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBufferTypeTitle, strBufferType);

    // Add the buffer FBO:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesFBOName, strBufferFBOName);

    // Add the buffer dimensions header
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDimensions, strBufferDimensions);

    // Add the buffer pixel format:
    // Add the buffer pixel format
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesPixelFormatTitle);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPixelFormatFormat, strBufferDataFormat);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_Type, strBufferDataType);

    // Add the CL interoperability link:
    if (!strBufferCLLink.isEmpty())
    {
        // Add the texture OpenCL shared image:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesGLTextureCLShared, strBufferCLLink);
    }
}


// ---------------------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildPipelinesListHTMLPropertiesString
// Description: Builds a message for the program pipelines list in a given OpenGL context
// Author:      Amit Ben-Moshe
// Date:        23/6/2014
// ---------------------------------------------------------------------------------------
void gdHTMLProperties::buildPipelinesListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects)
{
    (void)(numberOfObjects);  // unused
    gtString pipelineListTitle;
    pipelineListTitle.appendFormattedString(GD_STR_PropertiesPipelinesListHeadline, contextID._contextId);

    htmlContent.setTitle(pipelineListTitle);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesSelectItemMessage);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildPipelinesHTMLPropertiesString
// Description: Builds a program pipeline's properties in HTML format
// Arguments: const apContextID contextID - the relevant OpenGL context id
//            const apGLPipeline& pipelineDetails - an object holding the pipeline's details
//            afHTMLContent& htmlContent - the HTML object to be updated
// Return Val: void
// Author:      Amit Ben-Moshe
// Date:        23/6/2014
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildPipelinesHTMLPropertiesString(const apContextID& contextID, const apGLPipeline& pipelineDetails, afHTMLContent& htmlContent)
{
    // Build the pipeline name string.
    gtString pipelineNameStr;
    pipelineNameStr.appendFormattedString(GD_STR_PropertiesPiplineHeadline, pipelineDetails.pipelineName());

    // Create string buffers.
    gtString activeProgram;
    gtString vertexShader;
    gtString tessCtrlShader;
    gtString tessEvalShader;
    gtString geometryShader;
    gtString fragmentShader;
    gtString computeShader;
    gtString isBound;

    // Build the actual strings.
    const int glCtxId = contextID._contextId;
    getAssociatedProgramPropertiesViewMessage(pipelineDetails.getActiveProgram(), activeProgram);
    generateShaderHtmlLinkString(glCtxId, pipelineDetails.getVertexShader(), GD_VERTEX_SHADER, vertexShader);
    generateShaderHtmlLinkString(glCtxId, pipelineDetails.getTessCtrlShader(), GD_TESSELLATION_CONTROL_SHADER, tessCtrlShader);
    generateShaderHtmlLinkString(glCtxId, pipelineDetails.getTessEvaluationShader(), GD_TESSELLATION_EVALUATION_SHADER, tessEvalShader);
    generateShaderHtmlLinkString(glCtxId, pipelineDetails.getGeometryShader(), GD_GEOMETRY_SHADER, geometryShader);
    generateShaderHtmlLinkString(glCtxId, pipelineDetails.getFragmentShader(), GD_FRAGMENT_SHADER, fragmentShader);
    generateShaderHtmlLinkString(glCtxId, pipelineDetails.getComputeShader(), GD_COMPUTE_SHADER, computeShader);
    isBound << (pipelineDetails.isPipelineBound() ? L"Yes" : L"No");

    // Build the HTML content.
    htmlContent.setTitle(pipelineNameStr);

    // Add the pipeline's state descriptors.
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPipelineActiveProgram, activeProgram);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPipelineVertexShader, vertexShader);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPipelineTessCtrlShader, tessCtrlShader);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPipelineTessEvalShader, tessEvalShader);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPipelineGeometryShader, geometryShader);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPipelineFragmentShader, fragmentShader);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPipelineComputeShader, computeShader);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPipelineBindingStatus, isBound);

}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildPBuffersListHTMLPropertiesString
// Description: builds a message for the pixel buffers list
// Author:      Uri Shomroni
// Date:        6/11/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildPBuffersListHTMLPropertiesString(afHTMLContent& htmlContent, int numberOfObjects)
{
    (void)(numberOfObjects);  // unused
    // Build the HTML content:
    htmlContent.setTitle(GD_STR_PropertiesPBuffersListHeadline);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesSelectItemMessage);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildPBufferHTMLPropertiesString
// Description: builds a message for the pixel buffer item
// Arguments: pbufferID - the pbuffer's ID
//            isParent - is this item a parent of static buffers (in the tex viewer),
//            so we should add a "choose a buffer" line; or is it not a parent (in the
//            mem viewer), so we should add an explanation about PBuffers' memory size?
// Author:      Uri Shomroni
// Date:        6/11/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildPBufferHTMLPropertiesString(int pbufferID, const apPBuffer& pbufferDetails, afHTMLContent& htmlContent, bool isParent)
{
    // PBuffer headline:
    gtString pbufferHeadline;
    pbufferHeadline.appendFormattedString(GD_STR_PropertiesPBufferHeadline, pbufferID);

    // Associated render context id
    gtString strAttachedRenderContext;
    int contextID = pbufferDetails.pbufferContextId();
    strAttachedRenderContext.appendFormattedString(GD_STR_PropertiesPBufferHRC, contextID);

    // Associate device context:
    gtString strAttachedDeviceContext;
    oaDeviceContextHandle deviceContextHandle = pbufferDetails.deviceContextOSHandle();
    strAttachedDeviceContext.appendFormattedString(L"%p", deviceContextHandle);

    // pbuffer handler:
    gtString strPBufferHandler;
    oaPBufferHandle pbufferHandler = pbufferDetails.pbufferHandler();
    strPBufferHandler.appendFormattedString(L"%p", pbufferHandler);

    // Build the HTML content:
    htmlContent.setTitle(pbufferHeadline);

    // Add "General" Subtitle:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    // Add the PBuffer handler, render context and device context
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPBufferHandlerHead, strPBufferHandler);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPBufferHRCHead, strAttachedRenderContext);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPBufferHDCHead, strAttachedDeviceContext);

    // Add the pbuffer's dimensions:
    gtString dimensionsStr;
    dimensionsStr.appendFormattedString(GD_STR_Properties2DDimensions, pbufferDetails.width(), pbufferDetails.height());
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDimensions, dimensionsStr);

    if (isParent)
    {
        // Add the "choose a buffer" line:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesSelectItemMessage);
    }
    else
    {
        // Add the "View pbuffer details" line:
        gtString pbufferName, pbufferLinkStr;
        pbufferName.appendFormattedString(GD_STR_ImagesAndBuffersViewerPBufferName, pbufferID);
        pbufferLinkStr.appendFormattedString(AF_STR_HtmlPropertiesLink1ParamStart GD_STR_PropertiesViewDetailsTemplate GD_STR_HtmlPropertiesLinkEnd, GD_STR_HtmlPropertiesGLPBufferLink , pbufferID, pbufferName.asCharArray());
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_ITEM_LINK, pbufferLinkStr);

        // Add the "pbuffers have no size" explanation:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesMemorySizePBuffersExplanation);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildPBufferStaticBufferHTMLPropertiesString
// Description: Builds a pbuffer's static buffer sub item properties in HTML format
// Arguments: const apStaticBuffer& staticBufferDetails
//            const apPBuffer& pbufferDetails
//            int bufferId
//            gtString propertiesHTMLMessage
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/10/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildPBufferStaticBufferHTMLPropertiesString(const apStaticBuffer& staticBufferDetails, const apPBuffer& pbufferDetails, int bufferId, bool bAddThumbnail, afProgressBarWrapper* pProgressBar, afHTMLContent& htmlContent)
{
    // PBuffer headline:
    gtString pbufferHeadline;
    pbufferHeadline.appendFormattedString(GD_STR_PropertiesPBufferHeadline, bufferId);

    // Associated render context id
    gtString strAttachedRenderContext;
    int renderContextID = pbufferDetails.pbufferContextId();
    apContextID contextID(AP_OPENGL_CONTEXT, renderContextID);
    strAttachedRenderContext.appendFormattedString(GD_STR_PropertiesPBufferHRC, renderContextID);

    // Associate device context:
    gtString strAttachedDeviceContext;
    oaDeviceContextHandle deviceContextHandle = pbufferDetails.deviceContextOSHandle();
    strAttachedDeviceContext.appendFormattedString(L"%p", deviceContextHandle);

    // PBuffer handler:
    gtString strPBufferHandler;
    oaPBufferHandle pbufferHandler = pbufferDetails.pbufferHandler();
    strPBufferHandler.appendFormattedString(L"%p", pbufferHandler);

    // Get buffer type:
    gtString strBufferType;
    apGetBufferName(staticBufferDetails.bufferType(), strBufferType);

    // Get buffer dimensions:
    GLint bufferWidth, bufferHeight;
    gtString strBufferDimensions;
    staticBufferDetails.getBufferDimensions(bufferWidth, bufferHeight);
    strBufferDimensions.appendFormattedString(GD_STR_Properties2DDimensions, bufferWidth, bufferHeight);

    // Get buffer format:
    oaTexelDataFormat bufferDataFormat;
    oaDataType bufferDataType;
    gtString strBufferDataType, strBufferDataFormat;
    staticBufferDetails.getBufferFormat(bufferDataFormat, bufferDataType);

    // Buffer Data Type:
    GLenum bufferDataTypeEnum = oaDataTypeToGLEnum(bufferDataType);
    apGLenumValueToString(bufferDataTypeEnum, strBufferDataType);

    // Buffer Data Format:
    GLenum bufferDataFormatEnum = oaTexelDataFormatToGLEnum(bufferDataFormat);
    apGLenumValueToString(bufferDataFormatEnum, strBufferDataFormat);

    // Generate the buffer thumbnail string:
    gtString bufferThumbnailStr;

    if (bAddThumbnail)
    {
        // Generate the texture thumbnail:
        // Will hold the generated texture path and dimensions
        osFilePath previewPath;
        int imageWidth = 0;
        int imageHeight = 0;

        bool rc1 = generateObjectPreview(contextID, AP_HTML_PBUFFER, bufferId, staticBufferDetails.bufferType(), previewPath, imageWidth, imageHeight, pProgressBar);

        if (rc1)
        {
            // Image was generated successfully - add the image to the properties view
            bufferThumbnailStr.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesGLStaticBufferLink, staticBufferDetails.bufferType(), contextID._contextId);
            bufferThumbnailStr.appendFormattedString(AF_STR_HtmlImageWithSizeSpecsAndRightAlign, previewPath.asString().asCharArray(), imageWidth, imageHeight);
            bufferThumbnailStr += GD_STR_HtmlPropertiesLinkEnd;
        }
    }

    // Build the HTML content:
    htmlContent.setTitle(pbufferHeadline, bufferThumbnailStr);

    // Add "General" Subtitle:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    // Add the PBuffer handler, render context and device context
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPBufferHandlerHead, strPBufferHandler);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPBufferHRCHead, strAttachedRenderContext);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPBufferHDCHead, strAttachedDeviceContext);

    // Add the inner buffer type:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBufferTypeTitle, strBufferType);

    // Add the buffer dimensions header:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDimensions, strBufferDimensions);

    // Add the buffer pixel format
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesPixelFormatTitle);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPixelFormatFormat, strBufferDataFormat);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_Type, strBufferDataType);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildTextureHTMLPropertiesString
// Description: Get a texture object details, and writes a texture HTML properties string
// Arguments: int renderContextId
//            GLuint textureID
//            afHTMLContent& htmlContent
//            bool bBuildParametersTable
//            bool bAddThumbnail
//            afProgressBarWrapper *pProgressBar - pointer to progress bar for update
// Return Val: void
// Author:      Sigal Algranaty
// Date:        19/11/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildTextureHTMLPropertiesString(const apContextID& contextID, apGLTextureMipLevelID textureID, afHTMLContent& htmlContent, bool bBuildParametersTable, bool bAddThumbnail, afProgressBarWrapper* pProgressBar)
{
    GT_IF_WITH_ASSERT(AP_OPENGL_CONTEXT == contextID._contextType)
    {
        // Try to update the texture data:
        gtVector<apGLTextureMipLevelID> idVec(1);
        idVec[0] = textureID;
        bool rcUpd = gaUpdateTextureParameters(contextID._contextId, idVec, false);
        GT_ASSERT(rcUpd);

        // Check the texture type:
        apGLTextureMiplevelData textureMiplevelData;
        bool rc1 = gaGetTextureObjectThumbnailData(contextID._contextId, textureID._textureName, textureMiplevelData);
        GT_IF_WITH_ASSERT(rc1)
        {
            if (textureMiplevelData.textureType() == AP_BUFFER_TEXTURE)
            {
                buildTexBufferHTMLPropertiesString(contextID, textureMiplevelData, htmlContent, bAddThumbnail, pProgressBar);
            }
            else
            {
                apGLTexture textureDetails;
                bool rc2 = gaGetTextureObjectDetails(contextID._contextId, textureID._textureName, textureDetails);
                GT_IF_WITH_ASSERT(rc2)
                {
                    // Build the texture HTML string:
                    buildTextureHTMLPropertiesString(textureDetails, contextID, textureID, htmlContent, bBuildParametersTable, bAddThumbnail, pProgressBar);
                }
            }
        }
    }
}
// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildTextureHTMLPropertiesString
// Description: Builds a texture properties in HTML format
// Arguments: const apGLTexture& textureDetails
//          gtString propertiesHTMLMessage
//          int renderContextId - render context id
//          bool bBuildParametersTable - should add the texture parameters table to the HTML text
//          bool bAddThumbnail - should add the texture thumbnail to the HTML text
//          afProgressBarWrapper *pProgressBar - progress bar for displaying status
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/10/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildTextureHTMLPropertiesString(const apGLTexture& textureDetails, const apContextID& contextID, apGLTextureMipLevelID textureID, afHTMLContent& htmlContent, bool bBuildParametersTable, bool bAddThumbnail, afProgressBarWrapper* pProgressBar)
{
    GT_IF_WITH_ASSERT(AP_OPENGL_CONTEXT == contextID._contextType)
    {
        // Try to update the texture data:
        gtVector<apGLTextureMipLevelID> idVec(1);
        idVec[0] = textureID;
        bool rcUpd = gaUpdateTextureParameters(contextID._contextId, idVec, false);
        GT_ASSERT(rcUpd);
    }

    // Get the texture properties
    gtString strTextureType;
    gtString strTextureAmountOfMipLevels;
    gtString strTextureDimensions;
    gtString strTextureRequestedInternalFormat;
    gtString strTextureUsedInternalFormat;
    bool internalFormatsMatch = false;
    gtString strTextureBorderWidth;
    gtString strTexturePixelFormat;
    gtString strTextureTexelsType;
    gtString strTextureCompressRate;
    gtString strEstimatedMemorySize;
    gtString strTextureThumbnail;
    gtString strTextureCLLink;

    // Get the texture properties as string:
    getTextureProperties(textureDetails, textureID._textureMipLevel, strTextureType, strTextureAmountOfMipLevels, strTextureDimensions, strTextureRequestedInternalFormat, strTextureUsedInternalFormat, internalFormatsMatch,
                         strTextureBorderWidth, strTextureCompressRate, strEstimatedMemorySize, strTexturePixelFormat, strTextureTexelsType, strTextureCLLink);

    // Generate the texture thumbnail string:
    bool rc1 = getTextureThumbnail(contextID, textureID._textureName, textureID._textureName, strTextureThumbnail, bAddThumbnail, pProgressBar);
    GT_ASSERT(rc1);

    // Get the OpenCL interoperability details:
    int clImageIndex = -1, clImageName = -1, clSpyId = -1;
    textureDetails.getCLImageDetails(clImageIndex, clImageName, clSpyId);

    // Create the texture name string:
    gtString strTextureName;
    getGLTextureName(textureID, clImageName, clSpyId, strTextureName);

    // Cube map textures table should contain 6 cells for the texture faces parameters,
    // therefore, each table line should be added with the right colspan:
    int amountOfCells = 2;

    apTextureType texType = textureDetails.textureType();

    if (AP_CUBE_MAP_TEXTURE == texType)
    {
        amountOfCells = 7;
    }

    // Build the HTML content:
    htmlContent.setTitle(strTextureName, strTextureThumbnail, amountOfCells);

    // Add "General" Subtitle:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    // Add texture type line:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_Type, strTextureType.append(GD_STR_PropertiesTextureSuffix));

    // Add the texture number of levels:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesTextureMiplevels, strTextureAmountOfMipLevels.append(GD_STR_PropertiesLevelsPostfix));

    // Add the texture dimensions:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDimensions, strTextureDimensions);

    // Add the texture border width
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBorderWidth, strTextureBorderWidth.append(L"px"));

    // Add the CL interoperability link:
    if (!strTextureCLLink.isEmpty())
    {
        // Add the texture OpenCL shared image:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesGLTextureCLShared, strTextureCLLink);
    }

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesInternalPixelFormat);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPixelRequestedInternalFormat, strTextureRequestedInternalFormat);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPixelUsedInternalFormatFormat, strTextureUsedInternalFormat);

    // If the formats are different, note this here:
    if (!internalFormatsMatch)
    {
        // Construct the string containing the warning for the different formats:
        gtString warningStr;
        gtString iconPath;
        bool rcPath = afGetApplicationImagesPath(iconPath);
        GT_ASSERT(rcPath);
        iconPath.append(osFilePath::osPathSeparator);
        iconPath.append(AF_STR_WarningYellowIconFileName);
        gtString stam;
        stam = L"<font SIZE=-1><table bgcolor='#F5F5F5'><tr VALIGN=top ALIGN=left><td><img SRC='%ls'></td><td>The used internal pixel format is different from the requested internal format. This is probably caused by requesting a pixel format that is not supported by the graphic system.</td></tr></table></font>";
        warningStr.appendFormattedString(stam.asCharArray(), iconPath.asCharArray());

        // Add the warning table line to the texture table:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_HEADING, warningStr);
    }

    // Texture compress rate:
    if (!strTextureCompressRate.isEmpty() && !strEstimatedMemorySize.isEmpty())
    {
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesTextureCompressRateTitle);

        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesEstimatedMemorySize, strEstimatedMemorySize);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesCompressRate, strTextureCompressRate);
    }

    // Pixel format is not displayed for multisample texture;
    bool isMultisampleTexture = (textureDetails.textureType() == AP_2D_TEXTURE_MULTISAMPLE) || (textureDetails.textureType() == AP_2D_TEXTURE_MULTISAMPLE_ARRAY);

    if (!isMultisampleTexture)
    {
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesPixelFormatTitle);

        // Empty texture pixel format is for compressed textures:
        if (!strTexturePixelFormat.isEmpty())
        {
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPixelFormatFormat, strTexturePixelFormat);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_Type, strTextureTexelsType);
        }
    }

    // Add the texture parameters:
    if (bBuildParametersTable)
    {
        addTexturesParametersToHTMLContent(htmlContent, textureID._textureMipLevel, textureDetails);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildTextureTypesHTMLPropertiesString
// Description: Build a properties string for texture base tree item
// Arguments: int renderContextId - the render context id
//            int* pTexturesAmount
//            int numOfTypes
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        26/10/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildTextureTypesHTMLPropertiesString(const apContextID& contextID, gtMap<apTextureType, int>& texturesTypesAmount, afHTMLContent& htmlContent)
{
    // We only support GL and CL textures:
    bool isGLContext = contextID.isOpenGLContext();
    GT_ASSERT(contextID.isValid() && (!contextID.isDefault()));

    // Initialize the HTML properties string:
    gtString contextTexturesTitle;
    contextTexturesTitle.appendFormattedString(isGLContext ? GD_STR_PropertiesTextureThumbnailHeader : GD_STR_PropertiesImageThumbnailHeader, contextID._contextId);

    // Build the HTML content:
    htmlContent.setTitle(contextTexturesTitle);

    // Add the Texture Type / Amount Titles:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, isGLContext ? GD_STR_PropertiesTextureTypeTitle : GD_STR_PropertiesImageTypeTitle, GD_STR_PropertiesAmountTitle);

    // Count total number of textures:
    int totalNumberOfTextures = 0;

    // Check the range of texture types according to API type:
    int firstTextureType = 0;
    int lastTextureType = (int)texturesTypesAmount.size();

    if (contextID.isOpenCLContext())
    {
        firstTextureType = AP_2D_TEXTURE;
        lastTextureType = AP_3D_TEXTURE + 1;
    }

    // Now that we have done counting the texture types, create the table. Show only existing types
    // Get the texture type string
    for (int i = firstTextureType; i < lastTextureType; i++)
    {
        // Get the current texture type:
        apTextureType currentTextureType = (apTextureType)i;

        // Get the amount of textures from this type:
        int amountOfTexturesInType = texturesTypesAmount[currentTextureType];

        // First item is "Unknown Texture". Display it only if we have more than 0 unknown textures
        if ((i != AP_UNKNOWN_TEXTURE_TYPE) || (amountOfTexturesInType > 0))
        {
            // Get the texture type name
            apTextureType textureType = apTextureType(i);
            gtString strTextureType;
            apTextureTypeAsString(textureType, strTextureType);

            // Add "Texture" string to it
            strTextureType.append(isGLContext ? GD_STR_PropertiesTextureSuffix : GD_STR_PropertiesImageSuffix);

            // Get the amount of textures type
            gtString strAmountOfTexturesType;
            strAmountOfTexturesType.appendFormattedString(L"%u", amountOfTexturesInType);

            // Add this type line:
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, strTextureType, strAmountOfTexturesType);

            // Add the current texture type amount to the total number of textures:
            totalNumberOfTextures += amountOfTexturesInType;
        }
    }

    // Convert number of textures to a string:
    htmlContent.addHTMLTotalLine(totalNumberOfTextures);

    // Add the "choose a buffer" line:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesSelectItemMessage);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildAllBuffersHTMLPropertiesString
// Description: Builds a table with the summary of buffers by type
// Arguments: int amountOfStaticBuffers
//            int amountOfRenderBuffers
//            int amountOfVBOs
//            int amountOfFBOs
//            int amountOfPBuffers
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        3/5/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildAllBuffersHTMLPropertiesString(const apContextID& contextID, int amountOfStaticBuffers, int amountOfRenderBuffers, int amountOfVBOs, int amountOfFBOs, int amountOfPBuffers, afHTMLContent& htmlContent)
{
    // Initialize the HTML string:
    gtString contextBuffersTitle;
    contextBuffersTitle.appendFormattedString(GD_STR_PropertiesBuffersThumbnailHeader, contextID._contextId);

    // Build the HTML content:
    htmlContent.setTitle(contextBuffersTitle);

    // Add the Buffer Type / Amount Titles:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesBufferTypeTitle, GD_STR_PropertiesAmountTitle);

    gtString amountStr;

    // Add the static buffers row to the table counting the buffers:
    amountStr.appendFormattedString(L"%d", amountOfStaticBuffers);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_TexturesViewerStaticBuffers, amountStr);

    // Add the FBO row to the table counting the buffers:
    amountStr = AF_STR_Empty;
    amountStr.appendFormattedString(L"%d", amountOfFBOs);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_TexturesViewerFBOs, amountStr);

    // Add the PBuffers row to the table counting the buffers:
    amountStr = AF_STR_Empty;
    amountStr.appendFormattedString(L"%d", amountOfPBuffers);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_TexturesViewerPBuffers, amountStr);

    // Add the VBOs row to the table counting the buffers:
    amountStr = AF_STR_Empty;
    amountStr.appendFormattedString(L"%d", amountOfVBOs);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_TexturesViewerVBOs, amountStr);

    // Add the render buffers row to the table counting the buffers:
    amountStr = AF_STR_Empty;
    amountStr.appendFormattedString(L"%d", amountOfRenderBuffers);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_TexturesViewerRenderBuffers, amountStr);

    // Count all buffers:
    int totalAmountOfBuffers = amountOfVBOs + amountOfFBOs + amountOfPBuffers + amountOfRenderBuffers + amountOfStaticBuffers;

    // Add the "Total" row:
    htmlContent.addHTMLTotalLine(totalAmountOfBuffers);

    // Add the "choose a buffer" line:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesSelectItemMessage);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildTexturesListHTMLPropertiesString
// Description: Displays a generic message for when the "all textures" item is selected
//              but we don't have the textures' information:
// Author:      Uri Shomroni
// Date:        10/11/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildTexturesListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent)
{
    gtString title;

    if (contextID.isOpenGLContext())
    {
        title.appendFormattedString(GD_STR_PropertiesTexturesListHeadline, contextID._contextId);
    }
    else if (contextID.isOpenCLContext())
    {
        title.appendFormattedString(GD_STR_PropertiesImagesListHeadline, contextID._contextId);
    }
    else // contextID.isDefault()
    {
        // Unexpected context type:
        GT_ASSERT(false);
    }

    htmlContent.setTitle(title);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSelectItemMessage);
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::getGLDefaultTextureName
// Description: Construct a default texture name
// Arguments:   apGLTextureMipLevelID textureID
//              gtString& strTextureName
//              bool shortVersion
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        21/7/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::getGLDefaultTextureName(apGLTextureMipLevelID textureID, gtString& strTextureName, bool shortVersion)
{
    // default texture
    GLenum textureUnitNameEnum = 0;

    apTextureType textureType = AP_UNKNOWN_TEXTURE_TYPE;
    apGetDefaultTextureUnitAndType(textureID._textureName, textureUnitNameEnum, textureType);

    // Get the unit name
    int texUnitIndex = textureUnitNameEnum - GL_TEXTURE0;

    // Setting the name of the default texture into the string name
    switch (textureType)
    {
        case AP_1D_TEXTURE:
            strTextureName.appendFormattedString(shortVersion ? GD_STR_TexturesViewerDefault1DNameShortTextures : GD_STR_TexturesViewerDefault1DNameTextures, texUnitIndex);
            break;

        case AP_2D_TEXTURE:
            strTextureName.appendFormattedString(shortVersion ? GD_STR_TexturesViewerDefault2DNameShortTextures : GD_STR_TexturesViewerDefault2DNameTextures, texUnitIndex);
            break;

        case AP_3D_TEXTURE:
            strTextureName.appendFormattedString(shortVersion ? GD_STR_TexturesViewerDefault3DNameShortTextures : GD_STR_TexturesViewerDefault3DNameTextures, texUnitIndex);
            break;

        case AP_1D_ARRAY_TEXTURE:
            strTextureName.appendFormattedString(shortVersion ? GD_STR_TexturesViewerDefault1DArrayNameShortTextures : GD_STR_TexturesViewerDefault1DArrayNameTextures, texUnitIndex);
            break;

        case AP_2D_ARRAY_TEXTURE:
            strTextureName.appendFormattedString(shortVersion ? GD_STR_TexturesViewerDefault2DArrayNameShortTextures : GD_STR_TexturesViewerDefault2DArrayNameTextures, texUnitIndex);
            break;

        case AP_CUBE_MAP_TEXTURE:
            strTextureName.appendFormattedString(shortVersion ? GD_STR_TexturesViewerDefaultCubeMapNameShortTextures : GD_STR_TexturesViewerDefaultCubeMapNameTextures, texUnitIndex);
            break;

        case AP_CUBE_MAP_ARRAY_TEXTURE:
            strTextureName.appendFormattedString(shortVersion ? GD_STR_TexturesViewerDefaultCubeMapArrayNameShortTextures : GD_STR_TexturesViewerDefaultCubeMapArrayNameTextures, texUnitIndex);
            break;

        case AP_TEXTURE_RECTANGLE:
            strTextureName.appendFormattedString(shortVersion ? GD_STR_TexturesViewerDefaultTextureRectangleNameShort : GD_STR_TexturesViewerDefaultTextureRectangleName, texUnitIndex);
            break;

        case AP_BUFFER_TEXTURE:
            strTextureName.appendFormattedString(shortVersion ? GD_STR_TexturesViewerDefaultTexBufferNameShort : GD_STR_TexturesViewerDefaultTexBufferName, texUnitIndex);
            break;

        case AP_UNKNOWN_TEXTURE_TYPE:
            strTextureName.appendFormattedString(shortVersion ? GD_STR_TexturesViewerDefaultUnknownNameShortTextures : GD_STR_TexturesViewerDefaultUnknownNameTextures, texUnitIndex);
            break;

        default:
            strTextureName.appendFormattedString(shortVersion ? GD_STR_TexturesViewerDefaultUnknownNameShortTextures : GD_STR_TexturesViewerDefaultUnknownNameTextures, texUnitIndex);
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::getGLTextureName
// Description: Generate an OpenGL texture name string
// Arguments:   apGLTextureMipLevelID textureID - the texture ID
//              gtString& textureName - output - the texture name as string
//              bool shortVersion - Tex vs. Texture
// Return Val: void
// Author:      Sigal Algranaty
// Date:        10/11/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::getGLTextureName(apGLTextureMipLevelID textureID, int clImageName, int clSpyID, gtString& strTextureName, bool shortVersion)
{

    // Set the texture name into the string
    if (apIsDefaultTextureName(textureID._textureName))
    {
        // Build the texture default name:
        getGLDefaultTextureName(textureID, strTextureName, shortVersion);
    }
    else // ! apIsDefaultTextureName(textureID._textureName)
    {
        // Not a default texture
        if (shortVersion)
        {
            strTextureName.appendFormattedString(GD_STR_PropertiesTextureNameFormatShort, textureID._textureName);
        }
        else // !shortVersion
        {
            strTextureName.appendFormattedString(GD_STR_PropertiesTextureNameFormat, textureID._textureName);
        }
    }

    // If the image is GL shared, add the GL texture details:
    if (clImageName > 0)
    {
        strTextureName.appendFormattedString(GD_STR_PropertiesTextureCLDetailsFormat, clSpyID, clImageName);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::getCLImageName
// Description:
// Arguments:   const apCLImage& textureDetails - the OpenCL texture object
//              gtString& strTextureName - output - the texture name
//              bool shortVersion - Img vs. Image
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        21/7/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::getCLImageName(const apCLImage& textureDetails, gtString& strTextureName, bool shortVersion)
{
    // CL texture:
    gdGetImageDisplayName(textureDetails, strTextureName, !shortVersion);

    // Add OpenGL interoperability postfix:
    addOpenGLInteropPostfix(textureDetails, strTextureName, shortVersion);

    // Add AMD ext name if exists:
    if (!textureDetails.memObjectName().isEmpty())
    {
        strTextureName.append(L" (");
        strTextureName.append(textureDetails.memObjectName());
        strTextureName.append(L")");
    }
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::addOpenGLInteropPostfix
// Description: Adds an OpenGL interoperability postfix to the image name
// Arguments:   onst apCLImage& textureDetails
//              gtString& strTextureName
//              bool shortVersion
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        26/7/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::addOpenGLInteropPostfix(const apCLImage& textureDetails, gtString& strTextureName, bool shortVersion)
{
    // Get the shared GL texture details:
    GLuint glTextureName;
    GLint glMipLevel = 0;
    GLenum target = 0;
    textureDetails.getGLTextureDetails(glTextureName, glMipLevel, target);

    if (glTextureName > 0)
    {
        // Get the target as texture type:
        apTextureType glTextureType = apTextureBindTargetToTextureType(target);

        // Get the texture type as string:
        gtString textureTypeAsString;
        apTextureTypeAsString(glTextureType, textureTypeAsString);

        // Get the OpenGL spy ID:
        int glContextID = textureDetails.openGLSpyID();

        // If the image is GL shared, add the GL texture details:
        if (glMipLevel > 0)
        {
            if (shortVersion)
            {
                strTextureName.appendFormattedString(GD_STR_PropertiesImageGLDetailsFormatWithMipLevelShort, glContextID, glTextureName, glMipLevel);
            }
            else
            {
                strTextureName.appendFormattedString(GD_STR_PropertiesImageGLDetailsFormatWithMipLevel, glContextID, glTextureName, glMipLevel);
            }
        }
        else
        {
            strTextureName.appendFormattedString(GD_STR_PropertiesImageGLTexureDetailsFormat, glContextID, glTextureName);
        }
    }
    else
    {
        // Check if the image is attached to an OpenGL render buffer:
        if (textureDetails.openGLRenderBufferName() > 0)
        {
            strTextureName.appendFormattedString(GD_STR_PropertiesImageGLRenderBufferDetailsFormat, textureDetails.openGLSpyID(), textureDetails.openGLRenderBufferName());
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::getTextureProperties
// Description: Return texture properties as strings
// Arguments:   textureDetails - The texture details
// Return Val:  Strings containing texture details
// Author:      Eran Zinman
// Date:        8/6/2007
// ---------------------------------------------------------------------------
void gdHTMLProperties::getTextureProperties(const apGLTexture& textureDetails, int mipLevel, gtString& strTextureType, gtString& strTextureAmountOfMipLevels,
                                            gtString& strTextureDimensions, gtString& strTextureRequestedInternalFormat, gtString& strTextureUsedInternalFormat, bool& bInternalFormatsMatch,
                                            gtString& strTextureBorderWidth, gtString& strTextureCompressRate,
                                            gtString& strEstimatedMemorySize, gtString& strTexturePixelFormat,
                                            gtString& strTextureTexelsType, gtString& strTextureCLLink)
{
    // Get the texture type (1D, 2D, 3D, CUBE, ...)
    apTextureType textureType = textureDetails.textureType();

    // Get the texture type string
    apTextureTypeAsString(textureType, strTextureType);

    // Get the texture amount of mip levels:
    {
        GLuint minLevel = 0, maxLevel = 1000;
        bool rc = textureDetails.getTextureMinMaxLevels(minLevel, maxLevel);

        bool resetValues = !rc;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();
        apProjectType CodeXLProjectType = globalVarsManager.CodeXLProjectType();

        // The iPhone OpenGL ES implementation doesn't give us texture level information, so we suffice in the
        // auto generated value (min = 0 max = log2(width)).
        resetValues = resetValues && (!apDoesProjectTypeSupportOpenGLES(CodeXLProjectType));
#endif

        if (resetValues)
        {
            minLevel = 0;
            maxLevel = 1000;
        }

        int amountOfTextureLevels = maxLevel - minLevel + 1;
        strTextureAmountOfMipLevels.appendFormattedString(GD_STR_Properties1DDimensions , amountOfTextureLevels);
    }

    // Get the texture dimensions (width, height, depth, border width)
    {
        GLsizei texWidth = 0;
        GLsizei texHeight = 0;
        GLsizei texDepth = 0;
        GLsizei borderWidth = 0;

        // Retrieve values:
        textureDetails.getDimensions(texWidth, texHeight, texDepth, borderWidth, mipLevel);

        // Build texture dimension string:
        if (textureType == AP_1D_TEXTURE)
        {
            strTextureDimensions.appendFormattedString(GD_STR_Properties1DDimensions, texWidth);
        }
        else if ((textureType == AP_2D_TEXTURE) || (textureType == AP_1D_ARRAY_TEXTURE) || (textureType == AP_CUBE_MAP_TEXTURE) || (textureType == AP_TEXTURE_RECTANGLE) || (textureType == AP_2D_TEXTURE_MULTISAMPLE))
        {
            strTextureDimensions.appendFormattedString(GD_STR_Properties2DDimensions, texWidth, texHeight);
        }
        else if ((textureType == AP_3D_TEXTURE) || (textureType == AP_2D_ARRAY_TEXTURE) || (textureType == AP_2D_TEXTURE_MULTISAMPLE_ARRAY) || (textureType == AP_CUBE_MAP_ARRAY_TEXTURE))
        {
            GLsizei displayDepth = ((textureType == AP_CUBE_MAP_ARRAY_TEXTURE) ? (texDepth / 6) : texDepth);
            strTextureDimensions.appendFormattedString(GD_STR_Properties3DDimensions, texWidth, texHeight, displayDepth);
        }

        // Texture Border Width:
        strTextureBorderWidth.appendFormattedString(GD_STR_Properties1DDimensions, borderWidth);
    }

    // Check if the texture's internal format is compressed:
    bool isCompressInternalFormat = false;

    // Get the texture Internal format, Pixel Format and Texels type
    // Get the texture level parameters:
    const apGLTextureParams* pTextureLevelParams = textureDetails.textureLevelParameters(mipLevel);
    GT_IF_WITH_ASSERT(pTextureLevelParams != NULL)
    {
        // Requested internal format:
        GLint textureRequestedInternalFormat = textureDetails.requestedInternalPixelFormat();
        apGLPixelInternalFormatParameter internalRequestedPixelFormat;
        internalRequestedPixelFormat.setValueFromInt(textureRequestedInternalFormat);
        internalRequestedPixelFormat.valueAsString(strTextureRequestedInternalFormat);

        GLenum usedInternalFormat = textureDetails.usedInternalPixelFormat();

        if (usedInternalFormat != 0)
        {
            GLenum internalFormatAsEnum = GL_NONE;
            bool rc = pTextureLevelParams->getTextureEnumParameterValue(GL_TEXTURE_INTERNAL_FORMAT, internalFormatAsEnum);
            GT_IF_WITH_ASSERT(rc)
            {
                // Case the parameter to apGLPixelInternalFormatParameter to get the right value string:
                apGLPixelInternalFormatParameter internalUsedPixelFormatParam(internalFormatAsEnum);
                internalUsedPixelFormatParam.valueAsString(strTextureUsedInternalFormat);

                bInternalFormatsMatch = doesInternalFormatMatchRequestedInternalFormat(textureRequestedInternalFormat, internalFormatAsEnum);

                // Check if the internal format is compressed:
                isCompressInternalFormat = apGetIsInternalFormatCompressed(internalFormatAsEnum);
            }
        }
        else
        {
            // We don't know what is the internal pixel format, so we assume it is the same as the requested:
            bInternalFormatsMatch = true;
        }

        validateValueAvailability(strTextureUsedInternalFormat);

        // Pixel format
        if (textureDetails.pixelFormat() != GL_NONE)
        {
            apGLenumValueToString(textureDetails.pixelFormat(), strTexturePixelFormat);
        }
        else
        {
            if (isCompressInternalFormat)
            {
                strTexturePixelFormat = AF_STR_Empty;
            }
            else
            {
                strTexturePixelFormat = AF_STR_NotAvailable;
            }
        }

        // Texels type
        if (textureDetails.texelsType() != GL_NONE)
        {
            apGLenumValueToString(textureDetails.texelsType(), strTextureTexelsType);
        }
        else
        {
            strTextureTexelsType = AF_STR_NotAvailable;
        }
    }

    // Get the texture compression rate, and estimated memory size:
    float compressRate = 0;
    gtSize_t estimatedSize = 0;
    bool rc1 = textureDetails.getCompressionRate(compressRate);

    if (rc1)
    {
        strTextureCompressRate.appendFormattedString(GD_STR_PropertiesCompressRateString, compressRate);
        bool rc2 = textureDetails.getEstimatedMemorySize(estimatedSize);
        float val = (float)estimatedSize / (float)(1024 * 8);
        estimatedSize = (gtUInt32)ceil(val);
        GT_IF_WITH_ASSERT(rc2)
        {
            strEstimatedMemorySize.appendFormattedString(L"%d", estimatedSize).append(AF_STR_KilobytesShort);
        }
    }

    // Build the OpenCL image link:
    int clImageIndex = -1, clImageName = -1, clSpyId = -1;
    textureDetails.getCLImageDetails(clImageIndex, clImageName, clSpyId);

    if ((clImageIndex >= 0) && (clSpyId > 0))
    {
        strTextureCLLink.makeEmpty();
        strTextureCLLink.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesCLImageLink, clImageName, clSpyId);
        strTextureCLLink.appendFormattedString(GD_STR_PropertiesImageNameFullFormat, clSpyId, clImageName);
        strTextureCLLink.append(GD_STR_HtmlPropertiesLinkEnd);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::getTextureProperties
// Description: Return texture properties as strings
// Arguments:   apContextID contextID - the context id
//              textureID - the texture id
// Return Val:  Strings containing texture details
// Author:      Sigal Algranaty
// Date:        3/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::getCLImageProperties(const apContextID& contextID, int textureIndex, int& textureName, gtString& textureNameStr,
                                            gtString& strTextureType, gtString& strTextureDimensions, gtString& strTextureDataType,
                                            gtString& strTexturePixelFormat, gtString& strTextureHandle, gtString& strTextureGLLink,
                                            gtString& destructorPfnNotifyStr, gtString& destructorUserDataStr)
{
    // Get the texture details:
    apCLImage textureDetails;
    bool rc = gaGetOpenCLImageObjectDetails(contextID._contextId, textureIndex, textureDetails);
    GT_IF_WITH_ASSERT(rc)
    {
        // Set the texture name:
        textureName = textureDetails.imageName();

        // Create the texture name string:
        gtString strTextureName;
        getCLImageName(textureDetails, textureNameStr);

        // Get the texture type (2D/ 3D):
        apTextureType textureType = textureDetails.imageType();

        // Get the texture type string:
        apTextureTypeAsString(textureType, strTextureType);

        // Get the texture dimensions (width, height, depth):
        gtSize_t texWidth = 0;
        gtSize_t texHeight = 0;
        gtSize_t texDepth = 0;

        // Retrieve values:
        textureDetails.getDimensions(texWidth, texHeight, texDepth);

        // Build texture dimension string:
        if (textureType == AP_1D_TEXTURE)
        {
            strTextureDimensions.appendFormattedString(GD_STR_Properties1DDimensions, texWidth);
        }
        else if ((textureType == AP_2D_TEXTURE) || (textureType == AP_1D_ARRAY_TEXTURE) || (textureType == AP_CUBE_MAP_TEXTURE) || (textureType == AP_TEXTURE_RECTANGLE) || (textureType == AP_2D_TEXTURE_MULTISAMPLE))
        {
            strTextureDimensions.appendFormattedString(GD_STR_Properties2DDimensions, texWidth, texHeight);
        }
        else if ((textureType == AP_3D_TEXTURE) || (textureType == AP_2D_ARRAY_TEXTURE) || (textureType == AP_2D_TEXTURE_MULTISAMPLE_ARRAY) || (textureType == AP_CUBE_MAP_ARRAY_TEXTURE))
        {
            GLsizei displayDepth = ((textureType == AP_CUBE_MAP_ARRAY_TEXTURE) ? (texDepth / 6) : texDepth);
            strTextureDimensions.appendFormattedString(GD_STR_Properties3DDimensions, texWidth, texHeight, displayDepth);
        }

        // Get the texture data format:
        apCLEnumValueToString(textureDetails.dataFormat(), strTexturePixelFormat);

        // Get the cl enumeration as string:
        apCLEnumValueToString(textureDetails.dataType(), strTextureDataType);

        // Build the texture handle:
        gdUserApplicationAddressToDisplayString(textureDetails.memObjectHandle(), strTextureHandle);

        // Build the OpenGL texture link:
        GLuint glTextureName;
        GLint glMipLevel;
        GLenum glTarget;
        int glSpyID = textureDetails.openGLSpyID();
        textureDetails.getGLTextureDetails(glTextureName, glMipLevel, glTarget);

        if (glTextureName > 0)
        {
            strTextureGLLink.makeEmpty();

            if (glMipLevel > 0)
            {
                strTextureGLLink.appendFormattedString(AF_STR_HtmlPropertiesLink3ParamStart, GD_STR_HtmlPropertiesGLTextureLink, glTextureName, glSpyID, glMipLevel);
                strTextureGLLink.appendFormattedString(GD_STR_PropertiesGLTextureFullNameFormatWithMiplevel, glSpyID, glTextureName, glMipLevel);
            }
            else
            {
                strTextureGLLink.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesGLTextureLink, glTextureName, glSpyID);
                strTextureGLLink.appendFormattedString(GD_STR_PropertiesGLTextureFullNameFormat, glSpyID, glTextureName);
            }

            strTextureGLLink.append(GD_STR_HtmlPropertiesLinkEnd);
        }
        else if (textureDetails.openGLRenderBufferName() > 0)
        {
            strTextureGLLink.makeEmpty();
            strTextureGLLink.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesGLRenderBufferLink, textureDetails.openGLRenderBufferName(), textureDetails.openGLSpyID());
            strTextureGLLink.appendFormattedString(GD_STR_PropertiesGLRenderBufferFullNameFormat, textureDetails.openGLSpyID(),  textureDetails.openGLRenderBufferName());
            strTextureGLLink.append(GD_STR_HtmlPropertiesLinkEnd);
        }

        // Get the destructor info:
        const osProcedureAddress64& pfnNotify = textureDetails.destructorPfnNotify();
        const osProcedureAddress64& userData = textureDetails.destructorUserData();

        if ((OS_NULL_PROCEDURE_ADDRESS_64 == pfnNotify) && (OS_NULL_PROCEDURE_ADDRESS_64 == userData))
        {
            destructorPfnNotifyStr = AF_STR_NotAvailable;
            destructorUserDataStr = AF_STR_NotAvailable;
        }
        else // (OS_NULL_PROCEDURE_ADDRESS_64 != pfnNotify) || (OS_NULL_PROCEDURE_ADDRESS_64 != userData)
        {
            gdUserApplicationAddressToDisplayString(pfnNotify, destructorPfnNotifyStr);
            gdUserApplicationAddressToDisplayString(userData, destructorUserDataStr);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::addTexturesParametersToHTMLContent
// Description: Adds the texture parameter to the HTML properties table
// Arguments: afHTMLContent& htmlContent
//            const apGLTexture& textureDetails
// Return Val: void
// Author:      Sigal Algranaty
// Date:        17/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::addTexturesParametersToHTMLContent(afHTMLContent& htmlContent, int mipLevel, const apGLTexture& textureDetails)
{
    // Add "Texture Parameters" heading:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_COLOR_SUBTITLE, GD_STR_PropertiesTextureParametersHeading);

    // Add the parameters title:
    htmlContent.addHTMLItemWithColSpan(afHTMLContent::AP_HTML_SUB_TITLE, AF_STR_Name, AF_STR_Value, 2);

    // Get the texture parameters:
    const apGLTextureParams& textureParams = textureDetails.textureParameters();

    apTextureType textureType = textureDetails.textureType();

    // Check if this texture is a cube map texture:
    bool isCubeMap = (textureType == AP_CUBE_MAP_TEXTURE);

    // Get amount of parameters:
    bool isMultisampleTexture = ((textureDetails.textureType() == AP_2D_TEXTURE_MULTISAMPLE) || (textureDetails.textureType() == AP_2D_TEXTURE_MULTISAMPLE_ARRAY));
    int textureParametersAmount = textureParams.amountOfTextureParameters();

    if ((textureParametersAmount > 0) && !isMultisampleTexture)
    {
        // Loop through the texture parameters and get them all:
        for (int parameterIndex = 0; parameterIndex < textureParametersAmount; parameterIndex++)
        {
            // Retrieve the current parameter value:
            const apParameter* pParameterValue = textureParams.getTextureParameterValue(parameterIndex);
            gtString parameterValueAsStr;
            pParameterValue->valueAsString(parameterValueAsStr);

            // Get the parameter name:
            gtString parameterNameString;
            GLenum parameterName = textureParams.getTextureParameterName(parameterIndex);
            apGLenumValueToString(parameterName, parameterNameString);

            // Add the parameter only if we found it's name:
            GT_IF_WITH_ASSERT(!parameterNameString.isEmpty())
            {
                // Add the parameters name and it's value
                htmlContent.addHTMLItemWithColSpan(afHTMLContent::AP_HTML_LINE_NO_PADDING, parameterNameString , parameterValueAsStr, 2);
            }
        }
    }

    // Only for cube map, start another HTML table (since the cube map mip level parameters
    // contain 6 value columns):
    // Add Texture level parameter heading:
    gtString texLevelParameterHeading;
    texLevelParameterHeading.appendFormattedString(GD_STR_PropertiesTextureParametersMipLevelHeading, mipLevel);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_COLOR_SUBTITLE, texLevelParameterHeading);

    if (!isCubeMap)
    {
        // Add the parameters title:
        htmlContent.addHTMLItemWithColSpan(afHTMLContent::AP_HTML_SUB_TITLE, AF_STR_Name, AF_STR_Value, 2);
    }
    else
    {
        // Add the cube map titles:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, AF_STR_Name, gdHTMLProperties::cubemapTitles());
    }

    // Get amount of parameters for the current mip level:
    const apGLTextureParams* pTextureLevel0Params = textureDetails.textureLevelParameters(mipLevel);
    GT_IF_WITH_ASSERT(pTextureLevel0Params != NULL)
    {
        int textureMipLevelParametersAmount = pTextureLevel0Params->amountOfTextureParameters();

        // Check number of texture panes:
        apGLTextureMipLevel::apTextureFaceIndex firstFaceIndex = apGLTextureMipLevel::AP_SINGLE_TEXTURE_FACE_INDEX;
        apGLTextureMipLevel::apTextureFaceIndex lastFaceIndex = apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_NEGATIVE_X_FACE_INDEX;

        if (isCubeMap)
        {
            firstFaceIndex = apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_POSITIVE_X_FACE_INDEX;
            lastFaceIndex = apGLTextureMipLevel::AP_MAX_AMOUNT_OF_TEXTURE_FACES;
        }

        // Loop through the parameters and get them all
        for (int parameterIndex = 0; parameterIndex < textureMipLevelParametersAmount; parameterIndex++)
        {
            const apParameter* pParameterValue = NULL;

            // Retrieve the current parameter and it's value:
            GLenum paramName = pTextureLevel0Params->getTextureParameterName(parameterIndex);
            gtString parameterNameString;
            apGLenumValueToString(paramName, parameterNameString);

            // Add the parameter only if we found it's name:
            GT_IF_WITH_ASSERT(!parameterNameString.isEmpty())
            {
                // Define a vector for the parameter values:
                gtVector<gtString> parameterValues;

                // Get the parameters for each cube map face:
                for (int textureFace = firstFaceIndex; textureFace < lastFaceIndex; textureFace++)
                {
                    // Convert int to texture face index:
                    apGLTextureMipLevel::apTextureFaceIndex faceIndex = (apGLTextureMipLevel::apTextureFaceIndex)textureFace;

                    // Get the current face parameters:
                    const apGLTextureParams* pTextureFaceParams = textureDetails.textureLevelParameters(mipLevel, faceIndex);
                    GT_IF_WITH_ASSERT(pTextureFaceParams != NULL)
                    {
                        pParameterValue = pTextureFaceParams->getTextureParameterValue(parameterIndex);

                        GT_IF_WITH_ASSERT(pParameterValue != NULL)
                        {
                            gtString currentParameterString;
                            pParameterValue->valueAsString(currentParameterString);

                            parameterValues.push_back(currentParameterString);
                        }
                    }
                }

                if (isCubeMap)
                {
                    // Add this parameter HTML line:
                    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE_NO_PADDING, parameterNameString, parameterValues);
                }
                else
                {
                    // Add this parameter HTML line:
                    htmlContent.addHTMLItemWithColSpan(afHTMLContent::AP_HTML_LINE_NO_PADDING, parameterNameString, parameterValues[0], 2);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::getTextureThumbnail
// Description:
// Arguments:   apContextID contextID
//             int textureIndex - the texture place within the textures vector (in OpenGL == textureName)
//             int textureId
//             gtString& strTextureThumbnail
//             bool addThumbnail
//             afProgressBarWrapper *pProgressBar
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        10/11/2009
// ---------------------------------------------------------------------------
bool gdHTMLProperties::getTextureThumbnail(const apContextID& contextID, int textureIndex, int textureName, gtString& strTextureThumbnail, bool addThumbnail, afProgressBarWrapper* pProgressBar)
{
    bool retVal = false;

    if (addThumbnail)
    {
        // Generate the texture thumbnail string:
        osFilePath texturePreviewPath;
        int imageWidth = 0;
        int imageHeight = 0;
        retVal = generateObjectPreview(contextID, AP_HTML_TEXTURE, textureIndex, AP_DISPLAY_BUFFER_UNKNOWN, texturePreviewPath, imageWidth, imageHeight, pProgressBar);

        if (retVal)
        {
            // Image was generated successfully - add the image to the properties view
            if (contextID.isOpenGLContext())
            {
                strTextureThumbnail.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesGLTextureLink, textureName , contextID._contextId);
            }
            else
            {
                strTextureThumbnail.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesCLImageLink, textureIndex , contextID._contextId);
            }

            strTextureThumbnail.appendFormattedString(AF_STR_HtmlImageWithSizeSpecsAndRightAlign, texturePreviewPath.asString().asCharArray(), imageWidth, imageHeight);
            strTextureThumbnail += GD_STR_HtmlPropertiesLinkEnd;
        }
    }
    else
    {
        retVal = true;
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::doesInternalFormatMatchRequestedInternalFormat
// Description: Returns true iff used is a pixel format which is one belonging to the
//              group that requested represents.
///             eg: requested   used    retVal
//                  GL_RGB      GL_RGB8 true
//                  GL_RGB16    GL_RGB8 false
//                  GL_RGBA     GL_RGB8 false
//                  GL_RGBA8    GL_RGB8 false
//                  GL_RGB8     GL_RGB8 true
// Author:      Uri Shomroni
// Date:        2/3/2009
// ---------------------------------------------------------------------------
bool gdHTMLProperties::doesInternalFormatMatchRequestedInternalFormat(GLint requested, GLenum used)
{
    bool retVal = false;

    switch (requested)
    {
        case GL_ALPHA:
        {
            retVal = ((used == GL_ALPHA) || (used == GL_ALPHA4) || (used == GL_ALPHA8) || (used == GL_ALPHA12) || (used == GL_ALPHA16));
        }
        break;

        case GL_DEPTH_COMPONENT:
        {
            retVal = ((used == GL_DEPTH_COMPONENT) || (used == GL_DEPTH_COMPONENT16) || (used == GL_DEPTH_COMPONENT24) || (used == GL_DEPTH_COMPONENT32));
        }
        break;

        case GL_LUMINANCE:
        {
            retVal = ((used == GL_LUMINANCE) || (used == GL_LUMINANCE4) || (used == GL_LUMINANCE8) || (used == GL_LUMINANCE12) || (used == GL_LUMINANCE16));
        }
        break;

        case GL_LUMINANCE_ALPHA:
        {
            retVal = ((used == GL_LUMINANCE_ALPHA) || (used == GL_LUMINANCE4_ALPHA4) || (used == GL_LUMINANCE6_ALPHA2) || (used == GL_LUMINANCE8_ALPHA8) || (used == GL_LUMINANCE12_ALPHA4) || (used == GL_LUMINANCE12_ALPHA12) || (used == GL_LUMINANCE16_ALPHA16));
        }
        break;

        case GL_INTENSITY:
        {
            retVal = ((used == GL_INTENSITY) || (used == GL_INTENSITY4) || (used == GL_INTENSITY8) || (used == GL_INTENSITY12) || (used == GL_INTENSITY16));
        }
        break;

        case GL_RGB:
        {
            retVal = ((used == GL_R3_G3_B2) || (used == GL_RGB) || (used == GL_RGB4) || (used == GL_RGB5) || (used == GL_RGB8) || (used == GL_RGB10) || (used == GL_RGB12) || (used == GL_RGB16));
        }
        break;

        case GL_RGBA:
        {
            retVal = ((used == GL_RGBA) || (used == GL_RGBA2) || (used == GL_RGBA4) || (used == GL_RGB5_A1) || (used == GL_RGBA8) || (used == GL_RGB10_A2) || (used == GL_RGBA12) || (used == GL_RGBA16));
        }
        break;

        case GL_SLUMINANCE:
        {
            retVal = ((used == GL_SLUMINANCE) || (used == GL_SLUMINANCE8));
        }
        break;

        case GL_SLUMINANCE_ALPHA:
        {
            retVal = ((used == GL_SLUMINANCE_ALPHA) || (used == GL_SLUMINANCE8_ALPHA8));
        }
        break;

        case GL_SRGB:
        {
            retVal = ((used == GL_SRGB) || (used == GL_SRGB8));
        }
        break;

        case GL_SRGB_ALPHA:
        {
            retVal = ((used == GL_SRGB_ALPHA) || (used == GL_SRGB8_ALPHA8));
        }
        break;

        default:
        {
            retVal = ((GLenum)requested == used);
        }
        break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildFunctionCallHTMLPropertiesString
// Description: Builds a function call properties message
// Arguments: apExecutionMode currentExecutionMode
//            apContextID functionContextId
//            int functionCallIndex
//            afHTMLContent& htmlContent
//            afProgressBarWrapper *pProgressBar - progress bar for displaying status
// Author:      Sigal Algranaty
// Date:        19/10/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildFunctionCallHTMLPropertiesString(apExecutionMode currentExecutionMode, apContextID functionContextId, int functionCallIndex, gtString& propertiesHTMLMessage, afProgressBarWrapper* pProgressBar)
{
    // Calculate the title string:
    gtString title = GD_STR_PropertiesGLFunctionTitle;

    if (functionContextId.isOpenCLContext())
    {
        title = GD_STR_PropertiesCLFunctionTitle;
    }
    else if (functionContextId.isDefault())
    {
        title = GD_STR_PropertiesNoContextFunctionTitle;
    }

    // Get the current function call:
    gtAutoPtr<apFunctionCall> aptrFunctionCall;
    bool gotFunctionDetails = false;
    gtString textureThumbnail;

    if (currentExecutionMode != AP_PROFILING_MODE)
    {
        // Get the function call:
        gotFunctionDetails = gaGetCurrentFrameFunctionCall(functionContextId, functionCallIndex, aptrFunctionCall);

        if (gotFunctionDetails)
        {
            // Search for a texture thumbnail within the additional parameters:
            getAdditionalTextureThumbnailPropertiesViewMessage(functionContextId, *aptrFunctionCall, textureThumbnail, pProgressBar);
        }
    }

    // Build the HTML content:
    afHTMLContent htmlContent(title, textureThumbnail);

    // If we are in profiling mode:
    if (currentExecutionMode == AP_PROFILING_MODE)
    {
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesFunctionCallsNotLoggedInProfilingMode);
    }
    else
    {
        if (gotFunctionDetails)
        {
            apMonitoredFunctionId funcId = aptrFunctionCall->functionId();
            // Get the current function name:
            gtString funcName;
            gtString funcArgsStr;
            gtString additionalParamsStr;
            bool rc = gaGetMonitoredFunctionName(funcId, funcName);
            GT_ASSERT(rc);

            // Get the current function arguments:
            const gtList<const apParameter*>& funcArguments = aptrFunctionCall->arguments();
            rc = buildFunctionCallArguments(functionContextId, aptrFunctionCall, funcArguments, pProgressBar, funcArgsStr, additionalParamsStr);
            GT_ASSERT(rc);

            // Set the properties view message:
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_Name, funcName);

            // Add the arguments line:
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_Arguments, funcArgsStr);

            // Add the additional parameters text to the message:
            if (!additionalParamsStr.isEmpty())
            {
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesAssociatedParams, additionalParamsStr);
            }

        }
    }

    htmlContent.toString(propertiesHTMLMessage);
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildFunctionCallHTMLPropertiesString
// Description: The function build an HTML properties string for statistics viewer.
//              The properties contain:
//              1. Function name with arguments
//              2. Analyze mode Warnings (Get/Redundant state change function)
//              3. Deprecated function warnings
// Arguments: apExecutionMode currentExecutionMode
//            apContextID functionContextId
//            int functionCallIndex
//            afIconType iconType - the function call icon type
//            const gtString& functionNameWithArgsStr
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        1/3/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildFunctionCallHTMLPropertiesString(apExecutionMode currentExecutionMode, apContextID functionContextId, int functionCallIndex,
                                                             afIconType iconType, const gtString& functionNameWithArgsStr, gtString& propertiesHTMLMessage)
{
    // The function warnings title (for example: Redundant state change, Deprecated function):
    gtString functionWarningsTitle;

    // The function warnings content:
    gtString functionWarning1;
    gtString functionWarning2;

    if (currentExecutionMode == AP_ANALYZE_MODE)
    {
        // In analyze mode - get the function call warnings description:
        getFunctionCallWarnings(functionContextId, functionCallIndex, iconType, functionWarning1, functionWarning2, functionWarningsTitle);
    }

    // Add the first icon to the html string (original size):
    bool rc1 = afHTMLContent::addIconPath(iconType, functionNameWithArgsStr, false, propertiesHTMLMessage);
    GT_ASSERT(rc1);

    if (functionNameWithArgsStr.length() > 0)
    {
        propertiesHTMLMessage.append(AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_HtmlPropertiesNonbreakingSpace);
        propertiesHTMLMessage.append(AF_STR_HtmlPropertiesFontHeaderTagStart);
        propertiesHTMLMessage.append(functionNameWithArgsStr);
        propertiesHTMLMessage.append(AF_STR_HtmlPropertiesFontHeaderTagEnd);
    }

    // Add the warning title (all the warnings for this function):
    if (!functionWarningsTitle.isEmpty())
    {
        propertiesHTMLMessage.appendFormattedString(GD_STR_HtmlPropertiesCallsHistoryWarningTemplate, functionWarningsTitle.asCharArray());
    }

    // Add the first warning string:
    if (functionWarning1.length() > 0)
    {
        propertiesHTMLMessage.append(functionWarning1);
    }

    if (functionWarning2.length() > 0)
    {
        propertiesHTMLMessage.append(functionWarning2.asCharArray());
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::getAdditionalDataParametersPropertiesViewMessage
// Description: Inputs a function call and outputs its additional data parameters
//              properties view message.
// Arguments:   int renderContextId - render context id
//              propertiesHTMLMessage - Will get the properties view message.
//              functionCall - The input function call.
//              afProgressBarWrapper *pProgressBar - progress bar for displaying status
// Author:      Yaki Tebeka
// Date:        22/5/2005
// ---------------------------------------------------------------------------
void gdHTMLProperties::getAdditionalDataParametersPropertiesViewMessage(apContextID contextID, const apFunctionCall& functionCall, gtString& propertiesHTMLMessage, afProgressBarWrapper* pProgressBar)
{
    (void)(contextID);  // unused
    (void)(pProgressBar);  // unused
    // Iterate the function call additional parameters:
    const gtList<const apPseudoParameter*>& additionalParams = functionCall.additionalDataParameters();
    gtList<const apPseudoParameter*>::const_iterator iter = additionalParams.begin();
    gtList<const apPseudoParameter*>::const_iterator endIter = additionalParams.end();

    while (iter != endIter)
    {
        const apPseudoParameter* pParameter = *iter;
        GT_IF_WITH_ASSERT(pParameter != NULL)
        {
            // Get the additional data type:
            osTransferableObjectType objType = pParameter->type();

            // Get the current associated data properties view message:
            gtString currentDataPropertiedViewMessage;

            switch (objType)
            {

                case OS_TOBJ_ID_ASSOCIATED_PROGRAM_NAME_PSEUDO_PARAMETER:
                {
                    apAssociatedProgramNamePseudoParameter* pAssociatedProgramParam = (apAssociatedProgramNamePseudoParameter*)(*iter);

                    if (pAssociatedProgramParam)
                    {
                        getAssociatedProgramPropertiesViewMessage(*pAssociatedProgramParam, currentDataPropertiedViewMessage);
                    }
                }
                break;

                case OS_TOBJ_ID_ASSOCIATED_SHADER_NAME_PSEUDO_PARAMETER:
                {
                    apAssociatedShaderNamePseudoParameter* pAssociatedShaderParam = (apAssociatedShaderNamePseudoParameter*)(*iter);

                    if (pAssociatedShaderParam)
                    {
                        getAssociatedShaderPropertiesViewMessage(*pAssociatedShaderParam, currentDataPropertiedViewMessage);
                    }
                }
                break;

                default:
                    // Do nothing...
                    break;
            }

            // Add the current data message to the properties view message:
            propertiesHTMLMessage += currentDataPropertiedViewMessage;
        }

        iter++;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::getAdditionalDataParametersPropertiesViewMessage
// Description: Inputs a function call and outputs its texture thumbnail additional data parameters
//              if it exists
// Arguments:   int renderContextId - render context id
//              textureThumbnail - Will get the texture thumbnail
//              functionCall - The input function call.
//              afProgressBarWrapper *pProgressBar - progress bar for displaying status
// Author:      Sigal Algranaty
// Date:        5/1/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::getAdditionalTextureThumbnailPropertiesViewMessage(apContextID contextID, const apFunctionCall& functionCall, gtString& textureThumbnail, afProgressBarWrapper* pProgressBar)
{
    // Iterate the function call additional parameters:
    const gtList<const apPseudoParameter*>& additionalParams = functionCall.additionalDataParameters();
    gtList<const apPseudoParameter*>::const_iterator iter = additionalParams.begin();
    gtList<const apPseudoParameter*>::const_iterator endIter = additionalParams.end();

    while (iter != endIter)
    {
        const apPseudoParameter* pParameter = *iter;
        GT_IF_WITH_ASSERT(pParameter != NULL)
        {
            // Get the additional data type:
            osTransferableObjectType objType = pParameter->type();

            // Get the current associated data properties view message:
            if (objType == OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER)
            {
                const apAssociatedTextureNamesPseudoParameter* pAssociatedTextureParam = (const apAssociatedTextureNamesPseudoParameter*)pParameter;

                if (pAssociatedTextureParam != NULL)
                {
                    getAssociatedTexturePropertiesViewMessage(contextID, *pAssociatedTextureParam, textureThumbnail, pProgressBar);
                }

                break;
            }
        }

        iter++;
    }
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::getAssociatedTexturePropertiesViewMessage
// Description: Generated the properties view message of an associated texture
// Arguments:   int renderContextId - the texture render context id
//              associatedTextureNames - A parameter object that contains the associated
//                                       texture names.
//              propertiesHTMLMessage - Will get the properties view message.
//              afProgressBarWrapper *pProgressBar - progress bar for displaying status
// Return Val: void
// Author:      Yaki Tebeka
// Date:        22/5/2005
// ---------------------------------------------------------------------------
void gdHTMLProperties::getAssociatedTexturePropertiesViewMessage(apContextID contextID, const apAssociatedTextureNamesPseudoParameter& associatedTexNames, gtString& propertiesHTMLMessage, afProgressBarWrapper* pProgressBar)
{
    // Empty properties view output message
    propertiesHTMLMessage.makeEmpty();

    // Get the associated Textures names
    gtVector<GLuint> associatedTextureNames = associatedTexNames.associatedTextureNames();
    int numberOfTextures = associatedTextureNames.size();

    // Show a preview for single textures
    if (numberOfTextures == 1)
    {
        // Get the texture name
        GLuint textureID = associatedTextureNames[0];

        // "Texture 0" is a symbolic name for texture unbinding:
        if (textureID != 0)
        {
            // Will hold the generated texture path and dimensions
            osFilePath texturePreviewPath;
            int imageWidth = 0;
            int imageHeight = 0;

            // Generate the texture preview:
            apGLTexture textureDetails;
            bool rc0 = gaGetTextureObjectDetails(contextID._contextId, textureID, textureDetails);
            GT_IF_WITH_ASSERT(rc0)
            {
                bool rc1 = generateObjectPreview(contextID, AP_HTML_TEXTURE, textureID, AP_DISPLAY_BUFFER_UNKNOWN, texturePreviewPath, imageWidth, imageHeight, pProgressBar);

                if (rc1)
                {
                    // Image was generated successfully - add the image to the properties view
                    if (contextID.isOpenGLContext())
                    {
                        propertiesHTMLMessage.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesGLTextureLink, textureID, contextID._contextId);
                    }
                    else
                    {
                        propertiesHTMLMessage.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesCLImageLink, textureID, contextID._contextId);
                    }

                    propertiesHTMLMessage.appendFormattedString(AF_STR_HtmlImageWithSizeSpecifications, texturePreviewPath.asString().asCharArray(), 64, 64);
                    propertiesHTMLMessage.append(GD_STR_HtmlPropertiesLinkEnd);
                }
            }
            else
            {
                // Image was not generated - show a generic unable to show texture message
                propertiesHTMLMessage.appendFormattedString(GD_STR_PropertiesTextureDoesNotExist, textureID);
            }
        }
    }
    else if (numberOfTextures > 1)
    {
        gtVector<GLuint>::iterator iter = associatedTextureNames.begin();
        gtVector<GLuint>::iterator endIter = associatedTextureNames.end();

        // Iterate the textures
        while (iter != endIter)
        {
            // Get the texture name
            GLuint textureID = *iter;

            // "Texture 0" is a symbolic name for texture unbinding:
            if (textureID != 0)
            {
                // Start a new line
                propertiesHTMLMessage += AF_STR_HtmlNewLine;

                // Add the link to the image
                if (contextID.isOpenGLContext())
                {
                    propertiesHTMLMessage.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesGLTextureLink, textureID, contextID._contextId);
                }
                else
                {
                    propertiesHTMLMessage.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesCLImageLink, textureID, contextID._contextId);
                }

                propertiesHTMLMessage.appendFormattedString(GD_STR_PropertiesTextureNameFormat, textureID);
                propertiesHTMLMessage += GD_STR_HtmlPropertiesLinkEnd;
            }

            // Next texture:
            iter++;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::getAssociatedProgramPropertiesViewMessage
// Description: Generated the properties view message of an associated program.
// Arguments:   associatedProgramName - A parameter object that contains the associated
//                                      program names.
//              propertiesHTMLMessage - Will get the properties view message.
// Author:      Yaki Tebeka
// Date:        22/5/2005
// ---------------------------------------------------------------------------
void gdHTMLProperties::getAssociatedProgramPropertiesViewMessage(const apAssociatedProgramNamePseudoParameter& associatedProgramName, gtString& propertiesHTMLMessage)
{
    // Get the program name
    GLuint associatedProgName = associatedProgramName.associatedProgramName();

    // If this is the "No program" program id:
    if (associatedProgName == 0)
    {
        // Add the source code icon + program id (without a link):
        propertiesHTMLMessage.append(AF_STR_HtmlPropertiesNonbreakingSpace);
        propertiesHTMLMessage.append(GD_STR_PropertiesAssociatedProgramNoProgram);
    }
    else
    {
        // As these properties are only attached to context-specific function calls, we can assume the associated program belongs
        // in the currently selected render context.
        int contextID = -1;

        // Add a source code icon + link to the program:
        propertiesHTMLMessage.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesGLProgramLink, associatedProgName, contextID);
        propertiesHTMLMessage.append(AF_STR_HtmlPropertiesNonbreakingSpace);
        propertiesHTMLMessage.appendFormattedString(GD_STR_PropertiesProgramNameFormat, associatedProgName);
        propertiesHTMLMessage.append(GD_STR_HtmlPropertiesLinkEnd);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::getAssociatedShaderPropertiesViewMessage
// Description: Generated the properties view message of an associated shader.
// Arguments:   associatedShaderName - A parameter object that contains the associated
//                                      shader names.
//              propertiesHTMLMessage - Will get the properties view message.
// Author:      Yaki Tebeka
// Date:        22/5/2005
// ---------------------------------------------------------------------------
void gdHTMLProperties::getAssociatedShaderPropertiesViewMessage(const apAssociatedShaderNamePseudoParameter& associatedShaderName,
                                                                gtString& propertiesHTMLMessage)
{
    // Make sure that the output string is empty:
    propertiesHTMLMessage.makeEmpty();

    // Get the shader name
    GLuint shaderName = associatedShaderName.associatedShaderName();

    // As these properties are only attached to context-specific function calls, we can assume the associated shader belongs
    // in the currently selected render context.
    int contextID = gdGDebuggerGlobalVariablesManager::instance().chosenContext()._contextId;

    // Get the shader type:
    const wchar_t* shaderTypeLink = GD_STR_HtmlPropertiesGLUnsupportedShaderLink;
    gtAutoPtr<apGLShaderObject> aptrCurrentShader = NULL;
    bool rcShad = gaGetShaderObjectDetails(contextID, shaderName, aptrCurrentShader);
    GT_IF_WITH_ASSERT(rcShad && (aptrCurrentShader.pointedObject() != NULL))
    {
        afTreeItemType objectType = AF_TREE_ITEM_ITEM_NONE;
        gdShaderType curShadType = gdShaderTypeFromTransferableObjectType(aptrCurrentShader->type(), objectType);
        gtString curShadString;
        GLuint curShadName = aptrCurrentShader->shaderName();
        gdShaderNameStringFromNameAndType(curShadName, curShadType, curShadString);

        switch (curShadType)
        {
            case GD_VERTEX_SHADER:
                shaderTypeLink = GD_STR_HtmlPropertiesGLVertexShaderLink;
                break;

            case GD_TESSELLATION_CONTROL_SHADER:
                shaderTypeLink = GD_STR_HtmlPropertiesGLTessellationControlShaderLink;
                break;

            case GD_TESSELLATION_EVALUATION_SHADER:
                shaderTypeLink = GD_STR_HtmlPropertiesGLTessellationEvaluationShaderLink;
                break;

            case GD_GEOMETRY_SHADER:
                shaderTypeLink = GD_STR_HtmlPropertiesGLGeometryShaderLink;
                break;

            case GD_FRAGMENT_SHADER:
                shaderTypeLink = GD_STR_HtmlPropertiesGLFragmentShaderLink;
                break;

            case GD_UNSUPPORTED_SHADER:
            case GD_UNKNOWN_SHADER:
            default:
                shaderTypeLink = GD_STR_HtmlPropertiesGLUnsupportedShaderLink;
                break;
        }
    }

    // Add a source code icon + link to the shader:
    propertiesHTMLMessage.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, shaderTypeLink, shaderName, contextID);
    propertiesHTMLMessage.appendFormattedString(GD_STR_PropertiesShaderNameFormat, shaderName);
    propertiesHTMLMessage.append(GD_STR_HtmlPropertiesLinkEnd);
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::getImagesDir
// Description: Returns the directory in which the CodeXL images reside.
//              (Usually: "c:\program files\graphic remedy\CodeXL\images")
// Author:      Yaki Tebeka
// Date:        23/5/2005
// ---------------------------------------------------------------------------
void gdHTMLProperties::getImagesDir(osFilePath& imagesDir)
{
    // Will cache the images directory:
    static osFilePath stat_imagesDirectory;

    // Calculate the directory only on the first call to this function:
    if (stat_imagesDirectory.asString().isEmpty())
    {
        // Get the gDEBuggeer application installation path:
        gtString CodeXLImagesDirPathAsString;
        bool rc = afGetApplicationImagesPath(CodeXLImagesDirPathAsString);
        GT_IF_WITH_ASSERT(rc)
        {
            // Add path separator:
            CodeXLImagesDirPathAsString.append(osFilePath::osPathSeparator);

            // Build the images directory:
            stat_imagesDirectory.setFullPathFromString(CodeXLImagesDirPathAsString);
        }
        else
        {
            // We cannot get the application installation path:
            stat_imagesDirectory.setFileDirectory(AF_STR_NotAvailable);
        }
    }

    imagesDir = stat_imagesDirectory;
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::generateObjectPreview
// Description: Generates a preview for a given buffer ID
// Arguments: int renderContextId - the render context id
//            gdHTMLPropertiesObjectType objectType - the object type
//            GLuint objectID - the object opengl id
//            osFilePath& previewFile - the preview file name
//            apDisplayBuffer bufferType - the buffer type
//            int& imageWidth
//            int& imageHeight
//            afProgressBarWrapper *pProgressBar - pointer to progress bar for update
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/10/2008
// ---------------------------------------------------------------------------
bool gdHTMLProperties::generateObjectPreview(const apContextID& contextID, gdHTMLPropertiesObjectType objectType, GLuint objectID, apDisplayBuffer bufferType, osFilePath& previewFile, int& imageWidth, int& imageHeight, afProgressBarWrapper* pProgressBar)
{
    bool retVal = false;

    if (pProgressBar != NULL)
    {
        // Show the progress bar in the calling parent window:
        pProgressBar->setProgressText(GD_STR_PropertiesGeneratingPreview);

        // Set the progress bar range:
        pProgressBar->setProgressRange(100);

        // Show the progress dialog and set initial value to 20%:
        pProgressBar->setProgressRange(20);
    }

    // Get the user defined texture output file format (same as render buffers?):
    gdGDebuggerGlobalVariablesManager& theStateManager = gdGDebuggerGlobalVariablesManager::instance();
    apFileType loggedBufferFileType = theStateManager.imagesFileFormat();

    apStaticBuffer staticBufferDetails;
    apGLRenderBuffer renderBufferDetails(0);

    // Find OpenGL specific properties:
    bool isInGLBeginEndBlock = false;
    bool isFBOBound = false;

    if (contextID.isOpenGLContext())
    {
        // Check if we are in glBegin-glEnd Block:
        isInGLBeginEndBlock = gaIsInOpenGLBeginEndBlock(contextID._contextId);
        GLuint activeFBO = 0;
        bool rc = gaGetActiveFBO(contextID._contextId, activeFBO);
        GT_ASSERT(rc);
        isFBOBound = (activeFBO != 0);
    }

    acImageDataProxy* pImageProxy = NULL;

    if (objectType == AP_HTML_TEXTURE)
    {
        afApplicationTreeItemData* pItemData = new afApplicationTreeItemData;

        // Create an item data for the proxy:
        gdDebugApplicationTreeData* pGDItemData = new gdDebugApplicationTreeData;

        pItemData->setExtendedData(pGDItemData);
        pGDItemData->_contextId = contextID;

        if (contextID.isOpenGLContext())
        {
            pGDItemData->_textureMiplevelID._textureName = objectID;
            pGDItemData->_textureMiplevelID._textureMipLevel = 0;
        }
        else
        {
            pGDItemData->_objectOpenCLIndex = objectID;
        }

        // Allocate a proxy to generate an image:
        pImageProxy = new gdTextureImageProxy(pItemData, isInGLBeginEndBlock, AC_IMAGES_MANAGER_THUMBNAIL_SIZE, AC_IMAGES_MANAGER_THUMBNAIL_SIZE, true);

    }
    else if (objectType == AP_HTML_RENDER_BUFFER)
    {
        // Read the render buffer details again (so that the file path would be updated):
        bool rc2 = gaGetRenderBufferObjectDetails(contextID._contextId, objectID, renderBufferDetails);
        GT_IF_WITH_ASSERT(rc2)
        {
            pImageProxy = new gdRenderBufferImageProxy(objectID, renderBufferDetails, contextID._contextId, isInGLBeginEndBlock);

        }
    }
    else if (objectType == AP_HTML_STATIC_BUFFER)
    {
        // Read the render buffer details again (so that the file path would be updated):
        bool rc2 = gaGetStaticBufferObjectDetails(contextID._contextId, bufferType, staticBufferDetails);
        GT_IF_WITH_ASSERT(rc2)
        {
            pImageProxy = new gdStaticBufferImageProxy(staticBufferDetails, contextID._contextId, isInGLBeginEndBlock, isFBOBound);

        }
    }

    // Generate image preview:
    bool rc4 = pImageProxy->loadImage();
    GT_IF_WITH_ASSERT(rc4)
    {
        if (pProgressBar != NULL)
        {
            // Update progress:
            pProgressBar->updateProgressBar(40);
        }

        // Get proxy generated thumbnail (With background):
        QImage* pImageThumbnail = pImageProxy->createThumnbailImage(AC_IMAGES_MANAGER_THUMBNAIL_SIZE, AC_IMAGES_MANAGER_THUMBNAIL_SIZE, true);
        GT_IF_WITH_ASSERT(pImageThumbnail != NULL)
        {
            if (pProgressBar != NULL)
            {
                // Update progress:
                pProgressBar->updateProgressBar(60);
            }

            // Get thumbnail width and height
            imageWidth = pImageThumbnail->width();
            imageHeight = pImageThumbnail->height();

            // Generate texture preview file name
            bool rc5 = false;

            if (objectType == AP_HTML_TEXTURE)
            {
                apGLTextureMipLevelID mipLevelId;
                mipLevelId._textureName = objectID;

                // Get the optimized mip level for texture file name generation:
                gdTextureImageProxy* pTextureImageProxy = (gdTextureImageProxy*)pImageProxy;
                mipLevelId._textureMipLevel = pTextureImageProxy->getOptimizedMiplevel();

                // Generate the texture preview file name:
                rc5 = generateTexturePreviewFileName(contextID, mipLevelId, 0, loggedBufferFileType, previewFile);
            }
            else if (objectType == AP_HTML_RENDER_BUFFER)
            {
                // Generate the render buffer preview file name:
                rc5 = generateRenderBufferPreviewFileName(renderBufferDetails, loggedBufferFileType, previewFile);
            }
            else if (objectType == AP_HTML_STATIC_BUFFER)
            {
                // Read the static buffer details again (so that the file path would be updated):
                apStaticBuffer updatedStaticBufferDetails;
                rc5 = gaGetStaticBufferObjectDetails(contextID._contextId, bufferType, updatedStaticBufferDetails);
                GT_IF_WITH_ASSERT(rc5)
                {
                    // Generate the static buffer preview file name:
                    rc5 = generateStaticBufferPreviewFileName(updatedStaticBufferDetails, loggedBufferFileType, previewFile);
                }
            }

            // Release the image proxy memory:
            if (pImageProxy != NULL)
            {
                delete pImageProxy;
            }

            if (pProgressBar != NULL)
            {
                // Update progress:
                pProgressBar->updateProgressBar(40);
            }

            GT_IF_WITH_ASSERT(rc5)
            {
                // Save preview texture to disk:
                bool rc6 = pImageThumbnail->save(acGTStringToQString(previewFile.asString()));
                GT_IF_WITH_ASSERT(rc6)
                {
                    retVal = true;
                }
            }

            // Remove thumbnail from memory
            delete pImageThumbnail;
        }
    }

    if (pProgressBar != NULL)
    {
        // Update progress to 100%:
        pProgressBar->updateProgressBar(100);
        pProgressBar->hideProgressBar();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::generateTexturePreviewFileName
// Description: Generates the texture preview file name
// Arguments:   int renderContextId - the render context id
//              const apGLTextureMipLevelID mipLevelId - the requested texture mip level id
//              int faceIndex - the requested face index
//              fileType - Texture output image format file type
//              previewFileName - Output path to preview file
// Author:      Eran Zinman
// Date:        29/1/2008
// ---------------------------------------------------------------------------
bool gdHTMLProperties::generateTexturePreviewFileName(const apContextID& contextID, const apGLTextureMipLevelID mipLevelId, int faceIndex, apFileType fileType, osFilePath& previewFileName)
{
    bool retVal = false;

    // Generate image extension according to the user chose image file format
    gtString fileExtension;
    bool rc1 = apFileTypeToFileExtensionString(fileType, fileExtension);
    GT_IF_WITH_ASSERT(rc1)
    {
        if (contextID.isOpenGLContext())
        {
            // Determine the texture output image path
            rc1 = gaGetTextureMiplevelDataFilePath(contextID._contextId, mipLevelId, faceIndex, previewFileName);
            GT_ASSERT(rc1);
        }
        else if (contextID.isOpenCLContext())
        {
            apCLImage textureDetails;
            rc1 = gaGetOpenCLImageObjectDetails(contextID._contextId, mipLevelId._textureName, textureDetails);
            GT_IF_WITH_ASSERT(rc1)
            {
                textureDetails.imageFilePath(previewFileName);
            }
        }
        else // !(contextID.isOpenGLContext() || contextID.isOpenCLContext())
        {
            // Unexpected context type:
            GT_ASSERT(false);
        }

        // Localize the path as needed:
        gaRemoteToLocalFile(previewFileName, false);

        // Get current texture raw data file name
        gtString imageName;
        bool rc2 = previewFileName.getFileName(imageName);

        if (rc2)
        {
            // Add a "_thumb" suffix to the image name
            imageName.append(AF_STR_HtmlPropertiesThumbnailFileNameSuffix);

            // Set texture preview image file name
            previewFileName.setFileName(imageName);
            previewFileName.setFileExtension(fileExtension);

            // Localize the path as needed:
            gaRemoteToLocalFile(previewFileName, false);

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::generateRenderBufferPreviewFileName
// Description: Generates the render buffer preview file name
// Arguments:   const apGLRenderBuffer& renderBufferDetails - the render buffer details
//              fileType - Texture output image format file type
//              previewFileName - Output path to preview file
// Author:      Sigal Algranaty
// Date:        27/10/2008
// ---------------------------------------------------------------------------
bool gdHTMLProperties::generateRenderBufferPreviewFileName(const apGLRenderBuffer& renderBufferDetails, apFileType fileType, osFilePath& previewFileName)
{
    bool retVal = false;

    // Generate image extension according to the user chose image file format
    gtString fileExtension;
    bool rc1 = apFileTypeToFileExtensionString(fileType, fileExtension);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Determine the buffer output image path:
        renderBufferDetails.getBufferFilePath(previewFileName);
        GT_ASSERT(rc1);

        // Localize the path as needed:
        gaRemoteToLocalFile(previewFileName, false);

        // Get current texture raw data file name:
        gtString imageName;
        bool rc2 = previewFileName.getFileName(imageName);

        if (rc2)
        {
            // Add a "_thumb" suffix to the image name
            imageName.append(AF_STR_HtmlPropertiesThumbnailFileNameSuffix);

            // Set texture preview image file name
            previewFileName.setFileName(imageName);
            previewFileName.setFileExtension(fileExtension);
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::generateStaticBufferPreviewFileName
// Description: Generates the static buffer preview file name
// Arguments:   const apStaticBuffer& renderBufferDetails - the render buffer details
//              fileType - Texture output image format file type
//              previewFileName - Output path to preview file
// Author:      Sigal Algranaty
// Date:        27/10/2008
// ---------------------------------------------------------------------------
bool gdHTMLProperties::generateStaticBufferPreviewFileName(const apStaticBuffer& staticBufferDetails, apFileType fileType, osFilePath& previewFileName)
{
    bool retVal = false;

    // Generate image extension according to the user chose image file format
    gtString fileExtension;
    bool rc1 = apFileTypeToFileExtensionString(fileType, fileExtension);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Determine the buffer output image path:
        staticBufferDetails.getBufferFilePath(previewFileName);
        GT_ASSERT(rc1);

        // Localize the path as needed:
        gaRemoteToLocalFile(previewFileName, false);

        // Get current texture raw data file name:
        gtString imageName;
        bool rc2 = previewFileName.getFileName(imageName);

        if (rc2)
        {
            // Add a "_thumb" suffix to the image name
            imageName.append(AF_STR_HtmlPropertiesThumbnailFileNameSuffix);

            // Set texture preview image file name
            previewFileName.setFileName(imageName);
            previewFileName.setFileExtension(fileExtension);
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildStateChangeFunctionHTMLPropertiesString
// Description: Build an HTML string for state change function properties window
// Arguments: const gtString& functionName
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        2/3/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildStateChangeFunctionHTMLPropertiesString(const gtString& functionName, afIconType iconType, gtString& propertiesHTMLMessage)
{
    // String contain information on how sever is the redundancy status:
    gtString functionComment;

    switch (iconType)
    {
        case AF_ICON_INFO:
        {
            propertiesHTMLMessage.append(GD_STR_HtmlPropertiesRedundantNoWarning);
        }
        break;

        case AF_ICON_WARNING1:
        {
            gtString yellowColorHex = acQColorAsHexString(acQYELLOW_WARNING_COLOUR);
            propertiesHTMLMessage.appendFormattedString(GD_STR_HtmlPropertiesRedundantYellowWarning, yellowColorHex.asCharArray());
        }
        break;

        case AF_ICON_WARNING2:
        {
            gtString orangeColorHex = acQColorAsHexString(acQORANGE_WARNING_COLOUR);
            propertiesHTMLMessage.appendFormattedString(GD_STR_HtmlPropertiesRedundantOrangeWarning, orangeColorHex.asCharArray());
        }
        break;

        case AF_ICON_WARNING3:
        {
            gtString redColorHex = acQColorAsHexString(acQRED_WARNING_COLOUR);
            propertiesHTMLMessage.appendFormattedString(GD_STR_HtmlPropertiesRedundantRedWarning, redColorHex.asCharArray());
        }
        break;

        default:
        {
            // something's wrong
            GT_ASSERT(false);
        }
    }

    // Add the icon path before the function name:
    afHTMLContent::addIconPath(iconType, functionName, false, propertiesHTMLMessage);

    // Add the function name as title:
    if (functionName.length() > 0)
    {
        propertiesHTMLMessage.append(AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_HtmlPropertiesNonbreakingSpace);
        propertiesHTMLMessage.append(AF_STR_HtmlPropertiesFontHeaderTagStart);
        propertiesHTMLMessage.append(functionName);
        propertiesHTMLMessage.append(AF_STR_HtmlPropertiesFontHeaderTagEnd);
    }

    // Add the explanation of redundant state changes to the properties window content:
    propertiesHTMLMessage.append(GD_STR_HtmlPropertiesRedundantStateChange);
    propertiesHTMLMessage.append(AF_STR_HtmlNewLine);
    afHTMLContent::addIconPath(AF_ICON_LIGHT_BULB, AF_STR_Empty, true, propertiesHTMLMessage);
    propertiesHTMLMessage.append(GD_STR_HtmlPropertiesRedundantStateChangeAlternative);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildDeprecatedFunctionHTMLPropertiesString
// Description: Builds a properties string for a deprecated function
// Arguments: int functionId - the function id
//            const gtString& functionName - the function name
//            apFunctionDeprecationStatus functionDeprecationStatus (Full - for
//            complete function deprecation, other for function call deprecation)
//            apAPIVersion derecatedAtVersion
//            apAPIVersion removedAtVersion
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/3/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildDeprecatedFunctionHTMLPropertiesString(int functionId, const gtString& functionName, apFunctionDeprecationStatus functionDeprecationStatus, apAPIVersion deprecatedAtVersion, apAPIVersion removedAtVersion, gtString& propertiesHTMLMessage)
{
    // Initialize icon id according to deprecation severity:
    afIconType iconType = AF_ICON_INFO;

    if (removedAtVersion != AP_GL_VERSION_NONE)
    {
        iconType = AF_ICON_WARNING1;
    }

    // Add the warning icon:
    bool rc1 = afHTMLContent::addIconPath(iconType, functionName, false, propertiesHTMLMessage);
    GT_ASSERT(rc1);

    // Build the function title:
    if (functionName.length() > 0)
    {
        propertiesHTMLMessage.append(AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_HtmlPropertiesNonbreakingSpace);
        propertiesHTMLMessage.append(AF_STR_HtmlPropertiesFontHeaderTagStart);
        propertiesHTMLMessage.append(functionName);
        propertiesHTMLMessage.append(AF_STR_HtmlPropertiesFontHeaderTagEnd);
    }

    // Get function deprecation status by function id:
    apFunctionDeprecationStatus functionTypeDeprecationStatus = functionDeprecationStatus;

    if (functionDeprecationStatus == AP_DEPRECATION_FULL)
    {
        bool rc11 = apFunctionDeprecation::getDeprecationStatusByFunctionId(functionId, functionTypeDeprecationStatus);
        GT_ASSERT(rc11);
    }

    // Append the function deprecation documentation to the HTML string:
    propertiesHTMLMessage.append(AF_STR_HtmlNewLine);

    // The function deprecation string structure:
    // Title (with the specific deprecation status string)
    // Header (different for fully deprecated/partially deprecated):
    // desc1 (expansion of the fully deprecated function names for this status)
    // desc1 (expansion of the function illegal argument values for this status)

    gtString title;
    gtString warning;
    gtString header;
    gtString desc1, desc2;

    // Build the general warning for deprecation:
    afHTMLContent::addIconPath(AF_ICON_WARNING1, AF_STR_Empty, true, warning);
    warning.append(AF_STR_HtmlPropertiesNonbreakingSpace);
    warning.append(AF_STR_HtmlPropertiesNonbreakingSpace);
    warning.append(GD_STR_DeprecationWarning);

    // Add function alternative description:
    gtString functionDeprecationStatusAlternative;
    bool rc2 = buildDeprecationAlternativeByStatus(functionTypeDeprecationStatus, functionName, functionDeprecationStatusAlternative);
    GT_ASSERT(rc2);

    if (!functionDeprecationStatusAlternative.isEmpty())
    {
        gtString alternativeStr;
        afHTMLContent::addIconPath(AF_ICON_LIGHT_BULB, AF_STR_Empty, true, alternativeStr);
        alternativeStr.append(AF_STR_HtmlPropertiesNonbreakingSpace);
        alternativeStr.append(AF_STR_HtmlPropertiesNonbreakingSpace);
        alternativeStr.append(functionDeprecationStatusAlternative);
        functionDeprecationStatusAlternative = alternativeStr;
    }

    if (functionTypeDeprecationStatus != AP_DEPRECATION_FULL)
    {
        // Get the string for the deprecation status title:
        // Example: 'Edge flags and fixed-function vertex processing'
        bool rc22 = apFunctionDeprecationStatusToString(functionTypeDeprecationStatus, title);
        GT_ASSERT(rc22);
    }
    else
    {
        title.append(L"Fully deprecated functions");
    }

    // Get header for the function deprecation description:
    if (functionDeprecationStatus == AP_DEPRECATION_FULL)
    {
        // For completely deprecated functions:
        // 'The function glXXX was marked as deprecated in OpenGL version 3.0 and is removed from OpenGL at version 3.1.It is recommended to avoid using deprecated functions,
        // as their compatibility with future OpenGL versions is not assured.'
        rc1 = gdHTMLProperties::buildDeprecationHeader(AP_DEPRECATION_FULL, functionName, deprecatedAtVersion, removedAtVersion, header);
        GT_ASSERT(rc1);

        rc2 = gdHTMLProperties::buildDeprecatedFunctionList(functionTypeDeprecationStatus, desc1);
        GT_ASSERT(rc2);

        bool rc3 = gdHTMLProperties::buildDeprecatedFunctionCallsList(functionTypeDeprecationStatus, desc2);
        GT_ASSERT(rc3);

    }
    else
    {

        // For deprecated function calls:
        // 'Calling glXXXX with parameter values related to edge flags and fixed-function vertex processing
        // is deprecated behavior since OpenGL version 3.X and [was removed from OpenGL at version 3.XX | will be removed from OpenGL at a future version]'
        rc2 = gdHTMLProperties::buildDeprecationHeader(functionDeprecationStatus, functionName, deprecatedAtVersion, removedAtVersion, header);
        GT_ASSERT(rc2);

        bool rc3 = gdHTMLProperties::buildDeprecatedFunctionList(functionDeprecationStatus, desc2);
        GT_ASSERT(rc3);

        bool rc4 = gdHTMLProperties::buildDeprecatedFunctionCallsList(functionDeprecationStatus, desc1);
        GT_ASSERT(rc4);
    }

    // Build the whole string:
    if (!title.isEmpty())
    {
        propertiesHTMLMessage.append(GD_STR_DeprecationReasonTitle);
        propertiesHTMLMessage.append(AF_STR_HtmlPropertiesNonbreakingSpace);
        propertiesHTMLMessage.append(AF_STR_HtmlPropertiesNonbreakingSpace);
        propertiesHTMLMessage.append(title);
        propertiesHTMLMessage.append(AF_STR_HtmlNewLine);
        propertiesHTMLMessage.append(AF_STR_HtmlNewLine);
    }

    if (!header.isEmpty())
    {
        propertiesHTMLMessage.append(header);
        propertiesHTMLMessage.append(AF_STR_HtmlNewLine);
    }

    if (!warning.isEmpty())
    {
        propertiesHTMLMessage.append(AF_STR_HtmlNewLine);
        propertiesHTMLMessage.append(warning);
        propertiesHTMLMessage.append(AF_STR_HtmlNewLine);
        propertiesHTMLMessage.append(AF_STR_HtmlNewLine);
    }

    if (!functionDeprecationStatusAlternative.isEmpty())
    {
        propertiesHTMLMessage.append(functionDeprecationStatusAlternative);
        propertiesHTMLMessage.append(AF_STR_HtmlNewLine);
    }

    if (!desc1.isEmpty())
    {
        propertiesHTMLMessage.append(AF_STR_HtmlNewLine);
        propertiesHTMLMessage.append(desc1);
        propertiesHTMLMessage.append(AF_STR_HtmlNewLine);
    }

    if (!desc2.isEmpty())
    {
        propertiesHTMLMessage.append(AF_STR_HtmlNewLine);
        propertiesHTMLMessage.append(desc2);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::getFunctionCallWarnings
// Description:
// Arguments: int renderContextId - the function render context id
//            int functionCallIndex - the function call index
//            afIconType iconType - the function call type
//            gtString& functionWarning1 - the first function warning
//            gtString& functionWarning2 - the second function warning
//            gtString& functionWarningTitle - the function warnings title
// Return Val: void
// Author:      Sigal Algranaty
// Date:        3/3/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::getFunctionCallWarnings(const apContextID& contextID, int functionCallIndex, afIconType iconType,
                                               gtString& functionWarning1, gtString& functionWarning2, gtString& functionWarningTitle)
{
    // Get the current function call:
    gtAutoPtr<apFunctionCall> aptrFunctionCall(nullptr);
    bool rc1 = gaGetCurrentFrameFunctionCall(contextID, functionCallIndex, aptrFunctionCall);

    if (rc1)
    {
        GT_ASSERT(aptrFunctionCall.pointedObject() != nullptr);
        // Get the function id:
        apMonitoredFunctionId funcId = aptrFunctionCall->functionId();

        // Get the API string according to the project & context type:
        gtString apiSTR;
        apContextTypeToString(contextID._contextType, apiSTR);

        // Get the current function type:
        unsigned int functionType;
        bool rc = gaGetMonitoredFunctionType(funcId, functionType);
        GT_ASSERT(rc);

        gtString functionName;
        bool rc2 = gaGetMonitoredFunctionName(funcId, functionName);
        GT_ASSERT(rc2);

        if ((functionType & AP_GET_FUNC) && (contextID.isOpenGLContext()))
        {
            afHTMLContent::addIconPath(AF_ICON_WARNING2, AF_STR_Empty, true, functionWarning1);
            functionWarning1.append(AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_HtmlPropertiesNonbreakingSpace);
            functionWarning1.append(GD_STR_HtmlPropertiesCallsHistoryGetSubtitle AF_STR_HtmlNewLine);
            functionWarning1.appendFormattedString(GD_STR_HtmlPropertiesGetFunctions, apiSTR.asCharArray());
            functionWarning1.append(GD_STR_HtmlPropertiesGetIsFunctions);

            // Add get function warning to function warning title:
            functionWarningTitle.append(GD_STR_StatisticsViewerPropertiesGetTitle);
        }
        else if (functionType & AP_STATE_CHANGE_FUNC)
        {
            // For redundant function calls, get a different item attributes:
            apFunctionRedundancyStatus redundancyStatus = aptrFunctionCall->getRedundanctStatus();

            if (redundancyStatus == AP_REDUNDANCY_REDUNDANT)
            {
                // Add redundancy warning icon:
                rc2 = afHTMLContent::addIconPath(iconType, functionName, true, functionWarning1);
                GT_ASSERT(rc2);

                // Build redundancy full information string:
                functionWarning1.append(AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_HtmlPropertiesNonbreakingSpace);
                functionWarning1.append(GD_STR_HtmlPropertiesCallsHistoryRedundantSubtitle AF_STR_HtmlNewLine
                                        GD_STR_HtmlPropertiesRedundantStateChange);

                // Build the redundancy warning title:
                functionWarningTitle.append(GD_STR_HtmlPropertiesCallsHistoryRedundantTitle);
            }
        }

        if (aptrFunctionCall->getDeprecationStatus() != AP_DEPRECATION_NONE)
        {
            if (!functionWarningTitle.isEmpty())
            {
                functionWarningTitle.appendFormattedString(AF_STR_Comma AF_STR_HtmlPropertiesNonbreakingSpace);
            }

            // Add deprecation warning to function warning title:
            functionWarningTitle.append(GD_STR_StatisticsViewerPropertiesDeprecatedTitle);

            // TO_DO: deprecation model: Add alternative mechanisms suggestion:
            apAPIVersion deprecatedAtVersion = AP_GL_VERSION_NONE, removedAtVersion = AP_GL_VERSION_NONE;
            rc1 = apFunctionDeprecation::getDeprecationAndRemoveVersionsByStatus(aptrFunctionCall->getDeprecationStatus(), deprecatedAtVersion, removedAtVersion);

            if (!rc1)
            {
                rc1 = gaGetMonitoredFunctionDeprecationVersion(aptrFunctionCall->functionId(), deprecatedAtVersion, removedAtVersion);
                GT_ASSERT(rc1);
            }

            afIconType deprecationIconType = AF_ICON_WARNING1;

            if (removedAtVersion == AP_GL_VERSION_NONE)
            {
                deprecationIconType = AF_ICON_INFO;
            }

            // Add warning icon for deprecation:
            bool rc3 = afHTMLContent::addIconPath(deprecationIconType, functionName, true, functionWarning2);
            GT_ASSERT(rc3);

            // Add deprecation subtitle:
            functionWarning2.append(AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_HtmlPropertiesNonbreakingSpace);
            functionWarning2.append(GD_STR_HtmlPropertiesCallsHistoryDeprecatedSubtitle);
            functionWarning2.append(AF_STR_HtmlNewLine);

            // Get the function call deprecation status:
            apFunctionDeprecationStatus deprecationStatus = aptrFunctionCall->getDeprecationStatus();

            if (deprecationStatus == AP_DEPRECATION_FULL)
            {
                gtString functionDeprecationDescription;
                rc2 = gdHTMLProperties::buildDeprecationDocumentationByFunctionId(funcId, functionName, deprecatedAtVersion, removedAtVersion, functionDeprecationDescription);
                GT_ASSERT(rc2);

                // Get the specific function call deprecation status:
                rc = apFunctionDeprecation::getDeprecationStatusByFunctionId(funcId, deprecationStatus);
                GT_ASSERT(rc);

                // Fully deprecated function:
                functionWarning2.append(functionDeprecationDescription.asCharArray());
            }
            else
            {
                // Get the function deprecation details:
                apFunctionDeprecation functionDeprecation;
                rc = gaGetCurrentFrameFunctionCallDeprecationDetails(contextID._contextId, functionCallIndex, functionDeprecation);
                GT_IF_WITH_ASSERT(rc)
                {
                    // Translate the deprecation class to a string:
                    gtString deprecationString;
                    rc = functionDeprecation.toString(deprecationString);
                    GT_ASSERT(rc);

                    // Append the deprecation string to the warning:
                    functionWarning2.append(deprecationString.asCharArray());
                }
            }

            // Get the alternative string according to the function call deprecation status:
            gtString functionDeprecationStatusAlternative;
            rc2 = buildDeprecationAlternativeByStatus(deprecationStatus, functionName, functionDeprecationStatusAlternative);
            GT_ASSERT(rc2);

            if (!functionDeprecationStatusAlternative.isEmpty())
            {
                gtString alternativeStr;
                functionWarning2.append(AF_STR_HtmlNewLine);
                functionWarning2.append(AF_STR_HtmlNewLine);
                afHTMLContent::addIconPath(AF_ICON_LIGHT_BULB, AF_STR_Empty, true, functionWarning2);
                functionWarning2.append(AF_STR_HtmlPropertiesNonbreakingSpace);
                functionWarning2.append(AF_STR_HtmlPropertiesNonbreakingSpace);
                functionWarning2.append(functionDeprecationStatusAlternative);
            }
        }

        // If the warning is not empty, complete it:
        if (!functionWarningTitle.isEmpty())
        {
            functionWarningTitle.append(AF_STR_HtmlPropertiesNonbreakingSpace GD_STR_StatisticsViewerPropertiesFunction);
        }

        // For function with no information, append the standard information string;
        if (functionWarning1.isEmpty() && functionWarning2.isEmpty())
        {
            functionWarning1.appendFormattedString(GD_STR_StatisticsViewerPropertiesGenericCallsHistory, apiSTR.asCharArray());
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildFunctionHTMLPropertiesString
// Description: Build a function description HTML string
// Arguments: const gtString& functionName
//            afIconType iconType
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        3/3/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildFunctionHTMLPropertiesString(const apContextID& contextID, const gtString& functionName, afIconType iconType, gtString& propertiesHTMLMessage)
{
    // Add the icon before the title:
    bool rc = afHTMLContent::addIconPath(iconType, functionName, false, propertiesHTMLMessage);
    GT_ASSERT(rc);

    // Add the function name as title:
    if (functionName.length() > 0)
    {
        propertiesHTMLMessage.append(AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_HtmlPropertiesNonbreakingSpace);
        propertiesHTMLMessage.append(AF_STR_HtmlPropertiesFontHeaderTagStart);
        propertiesHTMLMessage.append(functionName);
        propertiesHTMLMessage.append(AF_STR_HtmlPropertiesFontHeaderTagEnd);
    }

    // Build the properties content:
    gtString propertiesContent;

    if ((functionName.find(L"glBegin ") > -1) || (functionName == L"glEnd"))
    {
        propertiesContent = GD_STR_HtmlPropertiesglBeginglEnd;
        afHTMLContent::addIconPath(AF_ICON_LIGHT_BULB, AF_STR_Empty, true, propertiesContent);
        propertiesContent.append(GD_STR_HtmlPropertiesglBeginglEndAlternative);
    }
    else if (functionName == L"glFinish")
    {
        propertiesContent = GD_STR_HtmlPropertiesglFinish;
        afHTMLContent::addIconPath(AF_ICON_LIGHT_BULB, AF_STR_Empty, true, propertiesContent);
        propertiesContent.append(GD_STR_HtmlPropertiesglFinishAlternative);
    }
    else if (functionName == L"glFlush")
    {
        propertiesContent = GD_STR_HtmlPropertiesglFlush;
        afHTMLContent::addIconPath(AF_ICON_LIGHT_BULB, AF_STR_Empty, true, propertiesContent);
        propertiesContent.append(GD_STR_HtmlPropertiesglFlushAlternative);
    }
    else if (functionName == L"glReadPixels")
    {
        propertiesContent = GD_STR_HtmlPropertiesglReadPixels;
        afHTMLContent::addIconPath(AF_ICON_LIGHT_BULB, AF_STR_Empty, true, propertiesContent);
        propertiesContent.append(GD_STR_HtmlPropertiesglReadPixelsAlternative);
    }
    else if (functionName == L"glGetPixelMap")
    {
        propertiesContent = GD_STR_HtmlPropertiesglGetPixelMap;
        afHTMLContent::addIconPath(AF_ICON_LIGHT_BULB, AF_STR_Empty, true, propertiesContent);
        propertiesContent.append(GD_STR_HtmlPropertiesglGetPixelMapAlternative);
    }
    else if (functionName == L"glGetShaderSource")
    {
        propertiesContent = GD_STR_HtmlPropertiesglGetShaderSource;
        afHTMLContent::addIconPath(AF_ICON_LIGHT_BULB, AF_STR_Empty, true, propertiesContent);
        propertiesContent.append(GD_STR_HtmlPropertiesglGetShaderSourceAlternative);
    }
    else if (functionName == L"glGetTexImage")
    {
        propertiesContent = GD_STR_HtmlPropertiesglGetTexImage;
    }
    else if ((functionName == L"eglGetError") || (functionName == L"glGetError") ||
             (functionName == L"glGetInfoLogARB") || (functionName == L"glGetProgramInfoLog") ||
             (functionName == L"glGetShaderInfoLog"))
    {
        propertiesContent = GD_STR_HtmlPropertiesglGetError;
        afHTMLContent::addIconPath(AF_ICON_LIGHT_BULB, AF_STR_Empty, true, propertiesContent);
        propertiesContent.append(GD_STR_HtmlPropertiesglGetErrorAlternative);
    }
    else if ((functionName == L"eglGetConfigAttrib") || (functionName == L"eglGetConfigs") || (functionName == L"eglGetDisplay") ||
             (functionName == L"eglGetProcAddress") || (functionName == L"eglQueryString") || (functionName == L"glGetActiveAttrib") ||
             (functionName == L"glGetActiveAttribARB") || (functionName == L"glGetActiveUniform") || (functionName == L"glGetActiveUniformARB") ||
             (functionName == L"glGetAttribLocation") || (functionName == L"glGetAttribLocationARB") || (functionName == L"glGetBufferPointerv") ||
             (functionName == L"glGetBufferPointervARB") || (functionName == L"glGetHandleARB") || (functionName == L"glGetProgramStringARB") ||
             (functionName == L"glGetProgramStringNV") || (functionName == L"glGetString") || (functionName == L"glGetUniformLocation") ||
             (functionName == L"glGetUniformLocationARB") || (functionName == L"glXGetClientString") || (functionName == L"glXGetConfig") ||
             (functionName == L"glXGetCurrentDisplay") || (functionName == L"glXGetCurrentDrawable") || (functionName == L"glXGetCurrentReadDrawable") ||
             (functionName == L"glXGetFBConfigAttrib") || (functionName == L"glXGetFBConfigAttribSGIX") || (functionName == L"glXGetFBConfigFromVisualSGIX") ||
             (functionName == L"glXGetFBConfigs") || (functionName == L"glXGetProcAddress") || (functionName == L"glXGetProcAddressARB") ||
             (functionName == L"glXGetVideoSyncSGI") || (functionName == L"glXGetVisualFromFBConfig") || (functionName == L"glXGetVisualFromFBConfigSGIX") ||
             (functionName == L"glXIsDirect") || (functionName == L"glXQueryContext") || (functionName == L"glXQueryExtension") ||
             (functionName == L"glXQueryExtensionsString") || (functionName == L"glXQueryServerString") || (functionName == L"glXQueryVersion") ||
             (functionName == L"wglDescribeLayerPlane") || (functionName == L"wglDescribePixelFormat") || (functionName == L"wglGetCurrentDC") ||
             (functionName == L"wglGetCurrentReadDCARB") || (functionName == L"wglGetDefaultProcAddress") || (functionName == L"wglGetExtensionsStringARB") ||
             (functionName == L"wglGetLayerPaletteEntries") || (functionName == L"wglGetPixelFormat") || (functionName == L"wglGetPixelFormatAttribfvARB") ||
             (functionName == L"wglGetPixelFormatAttribivARB") || (functionName == L"wglGetProcAddress"))
    {
        propertiesContent = GD_STR_HtmlPropertiesInitializationGetFunctions;
        afHTMLContent::addIconPath(AF_ICON_LIGHT_BULB, AF_STR_Empty, true, propertiesContent);
        propertiesContent.append(GD_STR_HtmlPropertiesInitializationGetFunctionsAlternative);
    }
    else if ((functionName == L"glAreTexturesResident") || (functionName == L"glGetAttachedObjectsARB") || (functionName == L"glGetAttachedShaders") ||
             (functionName == L"glGetBooleanv") || (functionName == L"glGetBufferParameteriv") || (functionName == L"glGetBufferParameterivARB") ||
             (functionName == L"glGetBufferSubData") || (functionName == L"glGetBufferSubDataARB") || (functionName == L"glGetClipPlane") ||
             (functionName == L"glGetClipPlanef") || (functionName == L"glGetClipPlanex") || (functionName == L"glGetColorTable") ||
             (functionName == L"glGetColorTableParameterfv") || (functionName == L"glGetColorTableParameteriv") || (functionName == L"glGetCompressedTexImage") ||
             (functionName == L"glGetCompressedTexImageARB") || (functionName == L"glGetConvolutionFilter") || (functionName == L"glGetConvolutionParameterfv") ||
             (functionName == L"glGetConvolutionParameteriv") || (functionName == L"glGetDoublev") || (functionName == L"glGetFixedv") ||
             (functionName == L"glGetFloatv") || (functionName == L"glGetFramebufferAttachmentParameterivEXT") || (functionName == L"glGetHistogram") ||
             (functionName == L"glGetHistogramParameterfv") || (functionName == L"glGetHistogramParameteriv") || (functionName == L"glGetIntegerv") ||
             (functionName == L"glGetLightfv") || (functionName == L"glGetLightiv") || (functionName == L"glGetLightxv") ||
             (functionName == L"glGetLocalConstantBooleanvEXT") || (functionName == L"glGetLocalConstantFloatvEXT") || (functionName == L"glGetLocalConstantIntegervEXT") ||
             (functionName == L"glGetMapdv") || (functionName == L"glGetMapfv") || (functionName == L"glGetMapiv") ||
             (functionName == L"glGetMaterialfv") || (functionName == L"glGetMaterialiv") || (functionName == L"glGetMaterialxv") ||
             (functionName == L"glGetMinmax") || (functionName == L"glGetMinmaxParameterfv") || (functionName == L"glGetMinmaxParameteriv") ||
             (functionName == L"glGetObjectParameterfvARB") || (functionName == L"glGetObjectParameterivARB") || (functionName == L"glGetOcclusionQueryivNV") ||
             (functionName == L"glGetOcclusionQueryuivNV") || (functionName == L"glGetPixelMapfv") || (functionName == L"glGetPixelMapuiv") ||
             (functionName == L"glGetPixelMapusv") || (functionName == L"glGetPointerv") || (functionName == L"glGetPolygonStipple") ||
             (functionName == L"glGetProgramEnvParameterdvARB") || (functionName == L"glGetProgramEnvParameterfvARB") || (functionName == L"glGetProgramiv") ||
             (functionName == L"glGetProgramivARB") || (functionName == L"glGetProgramivNV") || (functionName == L"glGetProgramLocalParameterdvARB") ||
             (functionName == L"glGetProgramLocalParameterfvARB") || (functionName == L"glGetProgramNamedParameterdvNV") || (functionName == L"glGetProgramNamedParameterfvNV") ||
             (functionName == L"glGetProgramParameterdvNV") || (functionName == L"glGetProgramParameterfvNV") || (functionName == L"glGetQueryObjectivARB") ||
             (functionName == L"glGetQueryObjectuivARB") || (functionName == L"glGetRenderbufferParameterivEXT") || (functionName == L"glGetShaderiv") ||
             (functionName == L"glGetShaderSourceARB") || (functionName == L"glGetTexEnvfv") || (functionName == L"glGetTexEnviv") ||
             (functionName == L"glGetTexEnvxv") || (functionName == L"glGetTexGendv") || (functionName == L"glGetTexGenfv") ||
             (functionName == L"glGetTexGeniv") || (functionName == L"glGetTexLevelParameterfv") || (functionName == L"glGetTexLevelParameteriv") ||
             (functionName == L"glGetTexParameterfv") || (functionName == L"glGetTexParameteriv") || (functionName == L"glGetTexParameterxv") ||
             (functionName == L"glGetTrackMatrixivNV") || (functionName == L"glGetUniformfv") || (functionName == L"glGetUniformfvARB") ||
             (functionName == L"glGetUniformiv") || (functionName == L"glGetUniformivARB") || (functionName == L"glGetVariantBooleanvEXT") ||
             (functionName == L"glGetVariantFloatvEXT") || (functionName == L"glGetVariantIntegervEXT") || (functionName == L"glGetVertexAttribdv") ||
             (functionName == L"glGetVertexAttribdvNV") || (functionName == L"glGetVertexAttribfv") || (functionName == L"glGetVertexAttribfvNV") ||
             (functionName == L"glGetVertexAttribiv") || (functionName == L"glGetVertexAttribivNV") || (functionName == L"glGetVertexAttribPointerv") ||
             (functionName == L"glGetVertexAttribPointervNV") || (functionName == L"glIsBuffer") || (functionName == L"glIsBufferARB") ||
             (functionName == L"glIsEnabled") || (functionName == L"glIsFramebufferEXT") || (functionName == L"glIsList") ||
             (functionName == L"glIsOcclusionQueryNV") || (functionName == L"glIsProgram") || (functionName == L"glIsProgramARB") ||
             (functionName == L"glIsProgramNV") || (functionName == L"glIsQuery") || (functionName == L"glIsQueryARB") ||
             (functionName == L"glIsRenderbufferEXT") || (functionName == L"glIsShader") || (functionName == L"glIsTexture") ||
             (functionName == L"glIsVariantEnabledEXT") || (functionName == L"glIsVertexArray") || (functionName == L"glXGetCurrentContext") ||
             (functionName == L"wglGenlockSourceEdgeI3D") || (functionName == L"wglGetCurrentContext") || (functionName == L"wglGetGenlockSampleRateI3D") ||
             (functionName == L"wglGetGenlockSourceDelayI3D") || (functionName == L"wglGetGenlockSourceEdgeI3D") || (functionName == L"wglGetGenlockSourceI3D") ||
             (functionName == L"wglIsEnabledGenlockI3D"))
    {
        propertiesContent = GD_STR_HtmlPropertiesGetIsFunctions;
        afHTMLContent::addIconPath(AF_ICON_LIGHT_BULB, AF_STR_Empty, true, propertiesContent);
        propertiesContent.append(GD_STR_HtmlPropertiesGetIsFunctionsFunctionsAlternative);
    }
    else
    {
        bool isClFunction = contextID.isOpenCLContext();

        if (contextID.isOpenGLContext())
        {
            isClFunction = false;
        }
        else // !contextID.isOpenGLContext()
        {
            // Get the function type from the function name:
            isClFunction = functionName.startsWith(L"cl");
        }

        // For other functions (ie ones that aren't problematic), add a generic text:
        if (isClFunction)
        {
            propertiesContent = GD_STR_StatisticsViewerPropertiesGenericCLCallsStatistics;
        }
        else
        {
            propertiesContent = GD_STR_StatisticsViewerPropertiesGenericGLCallsStatistics;
        }
    }

    // Add the function properties content to the HTML string:
    propertiesHTMLMessage.append(propertiesContent);
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildTotalStatisticsPropertiesString
// Description: Build total statistics item description for HTML properties window
// Arguments: const gdFuncCallsViewTypes& functionType - the function type
//            bool isTotalItem
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        10/3/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildTotalStatisticsPropertiesString(apContextID& functionContextID, const gdFuncCallsViewTypes& functionType, bool isTotalItem, gtString& propertiesHTMLMessage)
{
    // Build the properties title and content:
    gtString title, content, apiSTR;
    afIconType iconType = AF_ICON_INFO;

    // Get the API string according to the project & context type:
    apContextTypeToString(functionContextID._contextType, apiSTR);

    const wchar_t* apiStrAsCharArray = apiSTR.asCharArray();

    if (isTotalItem)
    {
        // If we are focused on the total item, the frames item, or no item at alcolorsAmount show the general message:
        gtString totalStatsDescription;
        content.appendFormattedString(GD_STR_HtmlPropertiesTotalStatsGeneral, apiStrAsCharArray, apiStrAsCharArray);
        title = GD_STR_StatisticsViewerTotalStatisticsCaptionW;
        iconType = AF_ICON_INFO;
    }
    else
    {
        switch (functionType)
        {
            case GD_GET_FUNCTIONS_INDEX:
            {
                content.appendFormattedString(GD_STR_HtmlPropertiesGetFunctions, apiStrAsCharArray);

                if (functionContextID.isOpenGLContext())
                {
                    // Add OpenGL 'Get' functions warning:
                    content += GD_STR_HtmlPropertiesGetIsFunctions;

                    // The warning icon is relevant only for OpenGL get functions:
                    iconType = AF_ICON_WARNING2;
                }

                title = GD_STR_TotalStatisticsViewerGetFunctions;
            }
            break;

            case GD_REDUNDANT_STATE_CHANGE_FUNCTIONS_INDEX:
            {
                content = GD_STR_HtmlPropertiesRedundantStateChange;
                apExecutionMode currentExecMode = AP_DEBUGGING_MODE;
                gaGetDebuggedProcessExecutionMode(currentExecMode);

                if (currentExecMode == AP_ANALYZE_MODE)
                {
                    iconType = AF_ICON_WARNING3;
                }

                title = GD_STR_TotalStatisticsViewerRedundantStateChangeFunctions;
            }
            break;

            case GD_DEPRECATED_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesDeprecatedFunctions;
                iconType = AF_ICON_WARNING1;
                title = GD_STR_TotalStatisticsViewerDeprecatedFunctions;
                break;

            case GD_EFFECTIVE_STATE_CHANGE_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesEffectiveStateChangeFunctions;
                title = GD_STR_TotalStatisticsViewerEffectiveStateChangeFunctions;
                break;

            case GD_STATE_CHANGE_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesStateChangeFunctions;
                title = GD_STR_TotalStatisticsViewerStateChangeFunctions;
                break;

            case GD_DRAW_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesDrawFunctions;
                title = GD_STR_TotalStatisticsViewerDrawFunctions;
                break;

            case GD_RASTER_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesRasterFunctions;
                title = GD_STR_TotalStatisticsViewerRasterFunctions;
                break;

            case GD_PROGRAM_AND_SHADERS_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesProgramsAndShadersFunctions;
                title = GD_STR_TotalStatisticsViewerProgramsAndShadersFunctions;
                break;

            case GD_PROGRAM_AND_KERNELS_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesProgramsAndKernelsFunctions;
                title = GD_STR_TotalStatisticsViewerProgramsAndKernelsFunctions;
                break;

            case GD_TEXTURE_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesTextureFunctions;
                title = GD_STR_TotalStatisticsViewerTextureFunctions;
                break;

            case GD_MATRIX_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesMatrixFunctions;
                title = GD_STR_TotalStatisticsViewerMatrixFunctions;
                break;

            case GD_NAME_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesNameFunctions;
                title = GD_STR_TotalStatisticsViewerNameFunctions;
                break;

            case GD_QUERY_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesQueryFunctions;
                title = GD_STR_TotalStatisticsViewerQueryFunctions;
                break;

            case GD_BUFFER_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesBufferFunctions;
                title = GD_STR_TotalStatisticsViewerBufferFunctions;
                break;

            case GD_BUFFERS_AND_IMAGE_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesBufferImageFunctions;
                title = GD_STR_TotalStatisticsViewerBufferImageFunctions;
                break;

            case GD_QUEUE_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesQueueFunctions;
                title = GD_STR_TotalStatisticsViewerQueueFunctions;
                break;

            case GD_SYNC_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesSyncFunctions;
                title = GD_STR_TotalStatisticsViewerSyncFunctions;
                break;

            case GD_FEEDBACK_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesFeedbackFunctions;
                title = GD_STR_TotalStatisticsViewerFeedbackFunctions;
                break;

            case GD_VERTEX_ARRAY_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesVertexArrayFunctions;
                title = GD_STR_TotalStatisticsViewerVertexArrayFunctions;
                break;

            case GD_DEBUG_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesDebugFunctions;
                title = GD_STR_TotalStatisticsViewerDebugFunctions;
                break;

            case GD_CL_NULL_CONTEXT_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesCLNullContextFunctions;
                title = GD_STR_TotalStatisticsViewerCLNullContextFunctions;
                break;

            case GD_GL_NULL_CONTEXT_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesGLNullContextFunctions;
                title = GD_STR_TotalStatisticsViewerGLNullContextFunctions;
                break;

            case GD_CL_CONTEXT_BOUND_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesCLContextBoundFunctions AF_STR_HtmlNewLine GD_STR_HtmlPropertiesContextBoundFunctionsWarning;
                iconType = AF_ICON_WARNING2;
                title = GD_STR_TotalStatisticsViewerCLContextBoundFunctions;
                break;

            case GD_GL_CONTEXT_BOUND_FUNCTIONS_INDEX:
                content = GD_STR_HtmlPropertiesGLContextBoundFunctions AF_STR_HtmlNewLine GD_STR_HtmlPropertiesContextBoundFunctionsWarning;
                iconType = AF_ICON_WARNING2;
                title = GD_STR_TotalStatisticsViewerGLContextBoundFunctions;
                break;

            default:
                // we got an unexpected value:
                GT_ASSERT(false);
                break;
        }
    }

    // Add the icon:
    bool rc = afHTMLContent::addIconPath(iconType, title, false, propertiesHTMLMessage);
    GT_ASSERT(rc);

    // Add the title:
    if (title.length() > 0)
    {
        propertiesHTMLMessage.append(AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_HtmlPropertiesNonbreakingSpace);
        propertiesHTMLMessage.append(AF_STR_HtmlPropertiesFontHeaderTagStart);
        propertiesHTMLMessage.append(title);
        propertiesHTMLMessage.append(AF_STR_HtmlPropertiesFontHeaderTagEnd);
    }

    // Add the content:
    propertiesHTMLMessage.append(content);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildDeprecationHeader
// Description: Creates a header for function deprecation description
// Arguments: apFunctionDeprecationStatus functionCallDeprecationStatus
//            const gtString& functionName
//            apAPIVersion deprecatedAtVersion
//            apAPIVersion removedAtVersion
//            gtString& header
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/3/2009
// ---------------------------------------------------------------------------
bool gdHTMLProperties::buildDeprecationHeader(apFunctionDeprecationStatus functionCallDeprecationStatus, const gtString& functionName, apAPIVersion deprecatedAtVersion, apAPIVersion removedAtVersion, gtString& header)
{
    // TO_DO: deprecation model: clean up. No need for so many strings, just add the functionality name for a generic string.
    // Build generic string with the deprecation version details:
    bool retVal = true;
    // Get the deprecation version strings:
    gtString deprecatedAtVersionStr, removedAtVersionStr;
    bool rc1 = apAPIVersionToString(deprecatedAtVersion, deprecatedAtVersionStr);
    GT_ASSERT(rc1);

    bool rc2 = apAPIVersionToString(removedAtVersion, removedAtVersionStr);
    GT_ASSERT(rc2);

    switch (functionCallDeprecationStatus)
    {
        case AP_DEPRECATION_FULL:
        {
            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationHeader1, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray(), removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationHeader2, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());
            }

            break;
        }

        case AP_DEPRECATION_PIXEL_FORMAT:
        {
            header.appendFormattedString(GD_STR_DeprecationPixelFormatHeader, functionName.asCharArray());
            break;
        }

        case AP_DEPRECATION_APPLICATION_GENERATED_NAMES:
        {
            header.appendFormattedString(GD_STR_DeprecationApplicationGenNamesHeader, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING:
        {
            header.appendFormattedString(GD_STR_DeprecationEdgeFlagHeaderPrefix, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_COLOR_INDEX_MODE:
        {
            header.appendFormattedString(GD_STR_DeprecationColorIndexHeaderPrefix, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_IMMEDIATE_MODE:
        {
            header.appendFormattedString(GD_STR_DeprecationImmediateModeHeaderPrefix, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_DISPLAY_LISTS:
        {
            header.appendFormattedString(GD_STR_DeprecationDisplayListHeaderPrefix , functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_RECTANGLE:
        {
            header.appendFormattedString(GD_STR_DeprecationRectHeaderPrefix, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_ATTRIBUTE_STACKS_FUNCTION:
        case AP_DEPRECATION_ATTRIBUTE_STACKS_STATE:
        {
            header.appendFormattedString(GD_STR_DeprecationAttributeStackHeaderPrefix, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_RASTER_POS_FUNCTION:
        case AP_DEPRECATION_RASTER_POS_STATE:
        {
            header.appendFormattedString(GD_STR_DeprecationRasterPositionHeaderPrefix, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_SEPERATE_POLYGON_DRAW_MODE:
        {
            header.appendFormattedString(GD_STR_DeprecationSeperatePolygonDrawModeHeaderPrefix, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_FUNCTION:
        case AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_STATE:
        {
            header.appendFormattedString(GD_STR_DeprecationPolygonAndLineStippleHeaderPrefix, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_BITMAP:
        {
            header.appendFormattedString(GD_STR_DeprecationBitmapHeaderPrefix, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_PIXEL_DRAWING:
        {
            header.appendFormattedString(GD_STR_DeprecationPixelDrawHeaderPrefix, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_TEXTURE_CLAMP_WRAP_MODE:
        {
            header.appendFormattedString(GD_STR_DeprecationTextureClampWrapModeHeaderPrefix, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_TEXTURE_AUTO_MIPMAP:
        {
            header.appendFormattedString(GD_STR_DeprecationTextureAutoMipmapHeaderPrefix , functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_ALPHA_TEST_FUNCTION:
        case AP_DEPRECATION_ALPHA_TEST_STATE:
        {
            header.appendFormattedString(GD_STR_DeprecationAlphaTestHeaderPrefix , functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_EVALUATORS_FUNCTION:
        case AP_DEPRECATION_EVALUATORS_STATE:
        {
            header.appendFormattedString(GD_STR_DeprecationEvaluatorsHeaderPrefix , functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_FEEDBACK:
        {
            header.appendFormattedString(GD_STR_DeprecationFeedbackHeaderPrefix , functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_HINTS:
        {
            header.appendFormattedString(GD_STR_DeprecationHintsHeaderPrefix , functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_NON_SPRITE_POINTS:
        {
            header.appendFormattedString(GD_STR_DeprecationNonSpriteHeaderPrefix , functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_LINE_WIDTH:
        {
            header.appendFormattedString(GD_STR_DeprecationLineWidthHeaderPrefix , functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_TEXTURE_BORDER:
        {
            header.appendFormattedString(GD_STR_DeprecationTextureBorderHeaderPrefix , functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_FUNCTION:
        case AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE:
        {
            header.appendFormattedString(GD_STR_DeprecationFixedFunctionFragmentProcessingHeaderPrefix, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_ACCUMMULATION_BUFFERS_FUNCTION:
        case AP_DEPRECATION_ACCUMMULATION_BUFFERS_STATE:
        {
            header.appendFormattedString(GD_STR_DeprecationAccumulationBuffersHeaderPrefix, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_FRAMEBUFFER_SIZE_QUERIES:
        {
            header.appendFormattedString(GD_STR_DeprecationFramebufferSizeQueriesHeaderPrefix, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_PIXEL_COPY:
        {
            header.appendFormattedString(GD_STR_DeprecationPixelCopyHeaderPrefix, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES:
        {
            header.appendFormattedString(GD_STR_DeprecationPolygonQuadsPrimitivesHeaderPrefix, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_UNIFIED_EXTENSION_STRING:
        {
            header.appendFormattedString(GD_STR_DeprecationUnifiedExtesionStringHeaderPrefix, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_AUXILIRY_BUFFERS:
        {
            header.appendFormattedString(GD_STR_DeprecationAuxiliryBuffersHeaderPrefix, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_CLIENT_VERTEX_AND_INDEX_ARRAYS:
        {
            header.appendFormattedString(GD_STR_DeprecationClientVertexAndIndexArrayHeaderPrefix, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_PIXEL_TRANSFER:
        {
            header.appendFormattedString(GD_STR_DeprecationPixelTransferHeaderPrefix, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_MAX_VARYING:
        {
            header.appendFormattedString(GD_STR_DeprecationMaxVaryingHeaderPrefix , functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_COMPRESSED_TEXTURE_FORMATS:
        {
            header.appendFormattedString(GD_STR_DeprecationCompressedTextureFormatsHeaderPrefix , functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_LSB_FIRST_PIXEL_PACKING:
        {
            header.appendFormattedString(GD_STR_DeprecationLSBFirstPixelPackingHeaderPrefix , functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        case AP_DEPRECATION_GLSL_VERSION:
        {
            gtString shaderName, fnName;
            int findSpace = functionName.find(AF_STR_Space);
            functionName.getSubString(0, findSpace, fnName);
            int findShaderName = functionName.find(L"-");
            functionName.getSubString(findShaderName + 2, functionName.length() - 1, shaderName);
            header.appendFormattedString(GD_STR_DeprecationGLSLVersionHeaderPrefix, fnName.asCharArray(), shaderName.asCharArray(), deprecatedAtVersionStr.asCharArray());

            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix1, removedAtVersionStr.asCharArray());
            }
            else
            {
                header.appendFormattedString(GD_STR_DeprecationPostfix2);
            }

            break;
        }

        default:
        {
            retVal = false;
        }

    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildDeprecatedFunctionList
// Description: The function builds a list of fully deprecated function with the
//              input deprecation status
// Arguments: apFunctionDeprecationStatus deprecationStatus
//            gtString& functionsDeprecatedList
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/3/2009
// ---------------------------------------------------------------------------
bool gdHTMLProperties::buildDeprecatedFunctionList(apFunctionDeprecationStatus deprecationStatus, gtString& functionsDeprecatedList)
{
    bool retVal = true;
    functionsDeprecatedList.append(GD_STR_DeprecationDescriptionTitle2);

    switch (deprecationStatus)
    {
        case AP_DEPRECATION_APPLICATION_GENERATED_NAMES:
        case AP_DEPRECATION_SEPERATE_POLYGON_DRAW_MODE:
        case AP_DEPRECATION_TEXTURE_CLAMP_WRAP_MODE:
        case AP_DEPRECATION_TEXTURE_AUTO_MIPMAP:
        case AP_DEPRECATION_HINTS:
        case AP_DEPRECATION_NON_SPRITE_POINTS:
        case AP_DEPRECATION_LINE_WIDTH:
        case AP_DEPRECATION_TEXTURE_BORDER:
        case AP_DEPRECATION_FRAMEBUFFER_SIZE_QUERIES:
        case AP_DEPRECATION_GLSL_VERSION:
        case AP_DEPRECATION_RASTER_POS_STATE:
        case AP_DEPRECATION_ALPHA_TEST_STATE:
        case AP_DEPRECATION_EVALUATORS_STATE:
        case AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_STATE:
        case AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES:
        case AP_DEPRECATION_AUXILIRY_BUFFERS:
        case AP_DEPRECATION_CLIENT_VERTEX_AND_INDEX_ARRAYS:
        case AP_DEPRECATION_ATTRIBUTE_STACKS_STATE:
        case AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE:
        case AP_DEPRECATION_UNIFIED_EXTENSION_STRING:
        case AP_DEPRECATION_MAX_VARYING:
        case AP_DEPRECATION_COMPRESSED_TEXTURE_FORMATS:
        case AP_DEPRECATION_LSB_FIRST_PIXEL_PACKING:
        case AP_DEPRECATION_FULL:
        {
            // No list of fully deprecated functions:
            functionsDeprecatedList = AF_STR_Empty;
            break;
        }

        case AP_DEPRECATION_PIXEL_FORMAT:
        {
            // Add here the function names with pixel format argument values:
            functionsDeprecatedList.append(L"glTexImage2D");
            break;
        }

        case AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING:
        {
            functionsDeprecatedList.append(L"glColorPointer, glEdgeFlagPointer, glFogCoordPointer, glIndexPointer, glNormalPointer, glSecondaryColorPointer, glTexCoordPointer, glVertexPointer, glEnableClientState, glDisableClientState, and glInterleavedArrays, glClientActiveTexture, glFrustum, glLoadIdentity, glLoadMatrix, glLoadTransposeMatrix, glMatrixMode, glMultMatrix, glMultTransposeMatrix, glOrtho*, glPopMatrix, glPushMatrix, glRotate*, glScale*, and glTranslate*, glMaterial*, glLight*, glLightModel*, and glColorMaterial, glShadeModel, and glClipPlane*.");
            break;
        }

        case AP_DEPRECATION_COLOR_INDEX_MODE:
        {
            functionsDeprecatedList.append(L"glIndex*, glColor*.");
            break;
        }

        case AP_DEPRECATION_IMMEDIATE_MODE:
        {
            // Add here the function names with pixel format argument values:
            functionsDeprecatedList.append(L"glBegin, glEnd, and glEdgeFlag*; glColor*, glFogCoord*, glIndex*, glNormal3*, glSecondaryColor3*, glTexCoord*, glVertex* ,glColor*");
            break;
        }

        case AP_DEPRECATION_DISPLAY_LISTS:
        {
            // Add here the function names with pixel format argument values:
            functionsDeprecatedList.append(L"glNewList, glEndList, glCallList, glCallLists, glListBase, glGenLists, glIsList, and glDeleteLists");
            break;
        }

        case AP_DEPRECATION_RECTANGLE:
        {
            // Add here the function names with pixel format argument values:
            functionsDeprecatedList.append(L"glRect*");
            break;
        }

        case AP_DEPRECATION_ATTRIBUTE_STACKS_FUNCTION:
        {
            // Add here the function names with pixel format argument values:
            functionsDeprecatedList.append(L"glPushAttrib, glPushClientAttrib, glPopAttrib, glPopClientAttrib");
            break;
        }

        case AP_DEPRECATION_RASTER_POS_FUNCTION:
        {
            // Add here the function names for raster position deprecation:
            functionsDeprecatedList.append(L"glRasterPos*, glWindowPos*");
            break;
        }

        case AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_FUNCTION:
        {
            functionsDeprecatedList.append(L"glPolygonStipple, glLineStipple");
            break;
        }

        case AP_DEPRECATION_BITMAP:
        {
            functionsDeprecatedList.append(L"glBitmap");
            break;
        }

        case AP_DEPRECATION_PIXEL_DRAWING:
        {
            functionsDeprecatedList.append(L"glDrawPixels, glPixelZoom");
            break;
        }

        case AP_DEPRECATION_ALPHA_TEST_FUNCTION:
        {
            functionsDeprecatedList.append(L"glAlphaFunc");
            break;
        }

        case AP_DEPRECATION_EVALUATORS_FUNCTION:
        {
            functionsDeprecatedList.append(L"glMap*, glEvalCoord*, glMapGrid*, glEvalMesh*, glEvalPoint*");
            break;
        }

        case AP_DEPRECATION_FEEDBACK:
        {
            functionsDeprecatedList.append(L"glRenderMode, glInitNames, glPopName, glPushName, glLoadName, glSelectBuffer, glFeedbackBuffer, glPassThrough");
            break;
        }

        case AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_FUNCTION:
        {
            functionsDeprecatedList.append(L"glAreTexturesResident, glPrioritizeTextures, and glFog*");
            break;
        }

        case AP_DEPRECATION_ACCUMMULATION_BUFFERS_FUNCTION:
        case AP_DEPRECATION_ACCUMMULATION_BUFFERS_STATE:
        {
            functionsDeprecatedList.append(L"glClearAccum and glAccum");
            break;
        }

        case AP_DEPRECATION_PIXEL_COPY:
        {
            functionsDeprecatedList.append(L"glCopyPixels");
            break;
        }

        case AP_DEPRECATION_PIXEL_TRANSFER:
        {
            functionsDeprecatedList.append(L"glPixelTranfer*");
            break;
        }

        default:
        {
            // Unsupported deprecation status:
            functionsDeprecatedList = AF_STR_Empty;
            retVal = false;
        }

    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildDeprecatedFunctionCallsList
// Description: The function build a list of deprecated function calls list
//              with the input deprecation status
// Arguments: apFunctionDeprecationStatus deprecationStatus
//            gtString& functionsDeprecatedCallsList
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/3/2009
// ---------------------------------------------------------------------------
bool gdHTMLProperties::buildDeprecatedFunctionCallsList(apFunctionDeprecationStatus deprecationStatus, gtString& functionsDeprecatedCallsList)
{
    bool retVal = true;
    functionsDeprecatedCallsList.append(GD_STR_DeprecationDescriptionTitle1);

    switch (deprecationStatus)
    {
        case AP_DEPRECATION_FULL:
        {
            // On full deprecation we do not show the list of functions, it is only for specific deprecations:
            functionsDeprecatedCallsList = AF_STR_Empty;
            break;
        }

        case AP_DEPRECATION_PIXEL_FORMAT:
        {
            // Add here the function names with pixel format argument values:
            functionsDeprecatedCallsList.append(GD_STR_DeprecationDescriptionPixelFormat);
            break;
        }

        case AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationPipelineVertextProcessingDetails2);
            break;
        }

        case AP_DEPRECATION_APPLICATION_GENERATED_NAMES:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationApplicationGeneratedNamesDetails2);
            break;
        }

        case AP_DEPRECATION_RASTER_POS_FUNCTION:
        case AP_DEPRECATION_RASTER_POS_STATE:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationRasterPosDetails2);
            break;
        }

        case AP_DEPRECATION_SEPERATE_POLYGON_DRAW_MODE:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationSeperatePolygonDrawModeDetails2);
            break;
        }

        case AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_FUNCTION:
        case AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_STATE:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationPolygonAndLineStippleDetails2);
            break;
        }

        case AP_DEPRECATION_BITMAP:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationBitmapDetails2);
            break;
        }

        case AP_DEPRECATION_TEXTURE_CLAMP_WRAP_MODE:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationTextureClampWrapModeDetails2);
            break;
        }

        case AP_DEPRECATION_TEXTURE_AUTO_MIPMAP:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationTextureAutoMipmapDetails2);
            break;
        }

        case AP_DEPRECATION_ALPHA_TEST_FUNCTION:
        case AP_DEPRECATION_ALPHA_TEST_STATE:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationAlphaTestDetails2);
            break;
        }

        case AP_DEPRECATION_EVALUATORS_FUNCTION:
        case AP_DEPRECATION_EVALUATORS_STATE:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationEvaluatorsDetails2);
            break;
        }

        case AP_DEPRECATION_FEEDBACK:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationFeedbackDetails2);
            break;
        }

        case AP_DEPRECATION_HINTS:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationHintsDetails2);
            break;
        }

        case AP_DEPRECATION_NON_SPRITE_POINTS:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationNonSpritePointsDetails2);
            break;
        }

        case AP_DEPRECATION_LINE_WIDTH:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationLineWidthDetails2);
            break;
        }

        case AP_DEPRECATION_TEXTURE_BORDER:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationTextureBorderDetails2);
            break;
        }

        case AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_FUNCTION:
        case AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationFixedFunctionFragmentProcessingDetails2);
            break;
        }

        case AP_DEPRECATION_ACCUMMULATION_BUFFERS_FUNCTION:
        case AP_DEPRECATION_ACCUMMULATION_BUFFERS_STATE:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationAccumulationBuffersDetails2);
            break;
        }

        case AP_DEPRECATION_FRAMEBUFFER_SIZE_QUERIES:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationFramebuffersSizeQueriesDetails2);
            break;
        }

        case AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationPolygonQuadsDetails2);
            break;
        }

        case AP_DEPRECATION_UNIFIED_EXTENSION_STRING:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationUnifiedExtensionStringDetails2);
            break;
        }

        case AP_DEPRECATION_AUXILIRY_BUFFERS:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationAuxiliryBuffersDetails2);
            break;
        }

        case AP_DEPRECATION_CLIENT_VERTEX_AND_INDEX_ARRAYS:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationClindVertexAndIndexArrayDetails2);
            break;
        }

        case AP_DEPRECATION_ATTRIBUTE_STACKS_FUNCTION:
        case AP_DEPRECATION_ATTRIBUTE_STACKS_STATE:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationAttributeStackDetails2);
            break;
        }

        case AP_DEPRECATION_MAX_VARYING:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationMaxVaryingDetails2);
            break;
        }

        case AP_DEPRECATION_COMPRESSED_TEXTURE_FORMATS:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationCompressedTextureFormatsDetails2);
            break;
        }

        case AP_DEPRECATION_LSB_FIRST_PIXEL_PACKING:
        {
            functionsDeprecatedCallsList.append(GD_STR_DeprecationLSBFirstPixelPackingDetails2);
            break;
        }

        case AP_DEPRECATION_IMMEDIATE_MODE:
        case AP_DEPRECATION_DISPLAY_LISTS:
        case AP_DEPRECATION_COLOR_INDEX_MODE:
        case AP_DEPRECATION_RECTANGLE:
        case AP_DEPRECATION_PIXEL_DRAWING:
        case AP_DEPRECATION_PIXEL_COPY:
        case AP_DEPRECATION_PIXEL_TRANSFER:
        case AP_DEPRECATION_GLSL_VERSION:
        {
            // There is no deprecated behavior for these features:
            functionsDeprecatedCallsList = AF_STR_Empty;
            break;
        }

        default:
        {
            // Unsupported deprecation status:
            functionsDeprecatedCallsList = AF_STR_Empty;
            retVal = false;
        }

    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildDeprecationDocumentationByStatus
// Description: Return a string documenting a specific deprecation status
//              with a suggestion how to replace the deprecated mechanism
// Arguments: apFunctionDeprecationStatus functionDeprecationStatus
//            const gtString& functionName
//            int functionId
//            apAPIVersion deprecatedAtVersion
//            apAPIVersion removedAtVersion
//            gtString& documentationString
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/3/2009
// ---------------------------------------------------------------------------
bool gdHTMLProperties::buildDeprecationDocumentationByStatus(apFunctionDeprecationStatus functionDeprecationStatus, const gtString& functionName, int functionId,
                                                             apAPIVersion deprecatedAtVersion, apAPIVersion removedAtVersion, gtString& documentationString)
{
    (void)(functionId);  // unused
    // TO_DO: deprecation model: string constants
    bool retVal = true;

    // Get the deprecation version strings:
    gtString deprecatedAtVersionStr, removedAtVersionStr;
    bool rc1 = apAPIVersionToString(deprecatedAtVersion, deprecatedAtVersionStr);
    GT_ASSERT(rc1);

    bool rc2 = apAPIVersionToString(removedAtVersion, removedAtVersionStr);
    GT_ASSERT(rc2);

    switch (functionDeprecationStatus)
    {
        // OpenGL 3.0 - 3.1 deprecations:
        case AP_DEPRECATION_PIXEL_FORMAT:
        {
            documentationString.appendFormattedString(GD_STR_DeprecationPixelFormatDetails, functionName.asCharArray());
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING:
        {
            documentationString.appendFormattedString(GD_STR_DeprecationPipelineVertextProcessingDetails, functionName.asCharArray());
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_APPLICATION_GENERATED_NAMES:
        {
            // Application Generated object names
            documentationString.appendFormattedString(GD_STR_DeprecationApplicationGeneratedNamesDetails, functionName.asCharArray());
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            retVal = true;
            break;
        }

        case AP_DEPRECATION_IMMEDIATE_MODE:
        {
            documentationString.append(GD_STR_DeprecationImmediateModeDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_COLOR_INDEX_MODE:
        {
            documentationString.append(GD_STR_DeprecationColorIndexModeDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_DISPLAY_LISTS:
        {
            documentationString.append(GD_STR_DeprecationDisplayListDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_ATTRIBUTE_STACKS_FUNCTION:
        case AP_DEPRECATION_ATTRIBUTE_STACKS_STATE:
        {
            documentationString.append(GD_STR_DeprecationAttributeStackDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_RASTER_POS_FUNCTION:
        case AP_DEPRECATION_RASTER_POS_STATE:
        {
            documentationString.append(GD_STR_DeprecationRasterPosDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_SEPERATE_POLYGON_DRAW_MODE:
        {
            documentationString.append(GD_STR_DeprecationSeperatePolygonDrawModeDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_FUNCTION:
        case AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_STATE:
        {
            documentationString.append(GD_STR_DeprecationPolygonAndLineStippleDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_BITMAP:
        {
            documentationString.append(GD_STR_DeprecationBitmapDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_PIXEL_DRAWING:
        {
            documentationString.append(GD_STR_DeprecationPixelDrawingDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_TEXTURE_CLAMP_WRAP_MODE:
        {
            documentationString.append(GD_STR_DeprecationTextureClampWrapModeDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_TEXTURE_AUTO_MIPMAP:
        {
            documentationString.append(GD_STR_DeprecationTextureAutoMipmapDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_ALPHA_TEST_FUNCTION:
        case AP_DEPRECATION_ALPHA_TEST_STATE:
        {
            documentationString.append(GD_STR_DeprecationAlphaTestDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_EVALUATORS_FUNCTION:
        case AP_DEPRECATION_EVALUATORS_STATE:
        {
            documentationString.append(GD_STR_DeprecationEvalutorsDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_RECTANGLE:
        {
            documentationString.append(GD_STR_DeprecationRectangleDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_FEEDBACK:
        {
            documentationString.append(GD_STR_DeprecationFeedbackDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_HINTS:
        {
            documentationString.append(GD_STR_DeprecationHintsDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_NON_SPRITE_POINTS:
        {
            documentationString.append(GD_STR_DeprecationNonSpritePointsDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_LINE_WIDTH:
        {
            documentationString.append(GD_STR_DeprecationLineWidthDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_TEXTURE_BORDER:
        {
            documentationString.append(GD_STR_DeprecationTextureBorderDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_FUNCTION:
        case AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE:
        {
            documentationString.append(GD_STR_DeprecationFixedFunctionFragmentProcessingDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_ACCUMMULATION_BUFFERS_FUNCTION:
        case AP_DEPRECATION_ACCUMMULATION_BUFFERS_STATE:
        {
            documentationString.append(GD_STR_DeprecationAccumulationBuffersDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_FRAMEBUFFER_SIZE_QUERIES:
        {
            documentationString.append(GD_STR_DeprecationFramebufferSizeQueriesDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_PIXEL_COPY:
        {
            documentationString.append(GD_STR_DeprecationPixelCopyDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES:
        {
            documentationString.append(GD_STR_DeprecationPolygonQuadsDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_AUXILIRY_BUFFERS:
        {
            documentationString.append(GD_STR_DeprecationAuxiliryBuffersDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_CLIENT_VERTEX_AND_INDEX_ARRAYS:
        {
            documentationString.append(GD_STR_DeprecationCliendVertexAndIndexArraysDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_PIXEL_TRANSFER:
        {
            documentationString.append(GD_STR_DeprecationPixelTransferDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_UNIFIED_EXTENSION_STRING:
        {
            documentationString.append(GD_STR_DeprecationUnifiedExtensionStringDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_GLSL_VERSION:
        {
            documentationString.append(GD_STR_DeprecationGLSLVersionDetails);
            documentationString.append(GD_STR_DeprecationOpenGLSpec);
            break;
        }

        case AP_DEPRECATION_FULL:
        {
            // Add the deprecation detailed string:
            if (removedAtVersion != AP_GL_VERSION_NONE)
            {
                documentationString.appendFormattedString(GD_STR_DeprecationHeader1, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray(), removedAtVersionStr.asCharArray());
            }
            else
            {
                documentationString.appendFormattedString(GD_STR_DeprecationHeader2, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());
            }

            break;
        }

        default:
        {
            retVal = false;
            GT_ASSERT(0);
            break;
        }
    }

    // Add recommendation to avoid using deprecated functions:
    documentationString.append(AF_STR_HtmlNewLine);
    documentationString.append(GD_STR_DeprecationWarning);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::getDeprecationDocumentationByFunctionId
// Description: The function return a deprecation details documentation by
//              deprecated function id
// Arguments: int functionId
//            const gtString& documentationString
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/3/2009
// ---------------------------------------------------------------------------
bool gdHTMLProperties::buildDeprecationDocumentationByFunctionId(int functionId, const gtString& functionName, apAPIVersion deprecatedAtVersion, apAPIVersion removedAtVersion, gtString& documentationString)
{
    bool retVal = false;

    // Try to add deprecation status specific documentation:
    apFunctionDeprecationStatus deprecationStatus;
    bool rc = apFunctionDeprecation::getDeprecationStatusByFunctionId(functionId, deprecationStatus);

    if (rc)
    {
        retVal = buildDeprecationDocumentationByStatus(deprecationStatus, functionName, functionId, deprecatedAtVersion, removedAtVersion, documentationString);
    }
    else
    {
        // Build generic string with the deprecation version details:

        // Get the deprecation version strings:
        gtString deprecatedAtVersionStr, removedAtVersionStr;
        bool rc1 = apAPIVersionToString(deprecatedAtVersion, deprecatedAtVersionStr);
        GT_ASSERT(rc1);

        bool rc2 = apAPIVersionToString(removedAtVersion, removedAtVersionStr);
        GT_ASSERT(rc2);

        if (removedAtVersion != AP_GL_VERSION_NONE)
        {
            documentationString.appendFormattedString(GD_STR_DeprecationHeader1, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray(), removedAtVersionStr.asCharArray());
        }
        else
        {
            documentationString.appendFormattedString(GD_STR_DeprecationHeader2, functionName.asCharArray(), deprecatedAtVersionStr.asCharArray());
        }

        // Add recommendation to avoid using deprecated functions:
        documentationString.append(AF_STR_HtmlNewLine);
        documentationString.append(GD_STR_DeprecationWarning);

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::getDeprecationAlternativeByStatus
// Description: The function builds a string suggesting the user how to use an
//              alternative OpenGL function or mechanism to its used deprecated
//              function call
// Arguments: apFunctionDeprecationStatus functionDeprecationStatus
//            const gtString& functionName - the function name
//            gtString& documentationString
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        12/3/2009
// ---------------------------------------------------------------------------
bool gdHTMLProperties::buildDeprecationAlternativeByStatus(apFunctionDeprecationStatus functionDeprecationStatus, const gtString& functionName, gtString& documentationString)
{
    (void)(functionName);  // unused

    bool retVal = true;

    switch (functionDeprecationStatus)
    {
        case AP_DEPRECATION_PIXEL_FORMAT:
        {
            documentationString.append(GD_STR_DeprecationAlternativePixelFormat);
            break;
        }

        case AP_DEPRECATION_APPLICATION_GENERATED_NAMES:
        {
            documentationString.append(GD_STR_DeprecationAlternativeApplicationGenNames);
            break;
        }

        case AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING:
        {
            documentationString.append(GD_STR_DeprecationAlternativeFixedVertexProcessing);
            break;
        }

        case AP_DEPRECATION_COLOR_INDEX_MODE:
        {
            documentationString.append(GD_STR_DeprecationAlternativeColorIndexMode);
            break;
        }

        case AP_DEPRECATION_IMMEDIATE_MODE:
        {
            documentationString.append(GD_STR_DeprecationAlternativeImmediateMode);
            break;
        }

        case AP_DEPRECATION_DISPLAY_LISTS:
        {
            documentationString.append(GD_STR_DeprecationAlternativeDisplayList);
            break;
        }

        case AP_DEPRECATION_ATTRIBUTE_STACKS_FUNCTION:
        case AP_DEPRECATION_ATTRIBUTE_STACKS_STATE:
        {
            documentationString.append(GD_STR_DeprecationAlternativeAttributesStack);
            break;
        }

        case AP_DEPRECATION_RASTER_POS_FUNCTION:
        case AP_DEPRECATION_RASTER_POS_STATE:
        {
            documentationString.append(GD_STR_DeprecationAlternativeRaster);
            break;
        }

        case AP_DEPRECATION_SEPERATE_POLYGON_DRAW_MODE:
        {
            documentationString.append(GD_STR_DeprecationAlternativeSeperatePolygonDrawMode);
            break;
        }

        case AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_FUNCTION:
        case AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_STATE:
        {
            documentationString.append(GD_STR_DeprecationAlternativePolygonAndLineStipple);
            break;
        }

        case AP_DEPRECATION_PIXEL_DRAWING:
        {
            documentationString.append(GD_STR_DeprecationAlternativePixelDraw);
            break;
        }

        case AP_DEPRECATION_BITMAP:
        {
            documentationString.append(GD_STR_DeprecationAlternativeBitmap);
            break;
        }

        case AP_DEPRECATION_TEXTURE_CLAMP_WRAP_MODE:
        {
            documentationString.append(GD_STR_DeprecationAlternativeClampMode);
            break;
        }

        case AP_DEPRECATION_TEXTURE_AUTO_MIPMAP:
        {
            documentationString.append(GD_STR_DeprecationAlternativeAutoMipmap);
            break;
        }

        case AP_DEPRECATION_ALPHA_TEST_FUNCTION:
        case AP_DEPRECATION_ALPHA_TEST_STATE:
        {
            documentationString.append(GD_STR_DeprecationAlternativeAlphaTest);
            break;
        }

        case AP_DEPRECATION_EVALUATORS_FUNCTION:
        case AP_DEPRECATION_EVALUATORS_STATE:
        {
            documentationString.append(GD_STR_DeprecationAlternativeEvaluators);
            break;
        }

        case AP_DEPRECATION_FEEDBACK:
        {
            documentationString.append(GD_STR_DeprecationAlternativeFeedback);
            break;
        }

        case AP_DEPRECATION_HINTS:
        {
            documentationString.append(GD_STR_DeprecationAlternativeHints);
            break;
        }

        case AP_DEPRECATION_NON_SPRITE_POINTS:
        {
            documentationString.append(GD_STR_DeprecationAlternativeNonSpritePoints);
            break;
        }

        case AP_DEPRECATION_LINE_WIDTH:
        {
            documentationString.append(GD_STR_DeprecationAlternativeLineWidth);
            break;
        }

        case AP_DEPRECATION_TEXTURE_BORDER:
        {
            documentationString.append(GD_STR_DeprecationAlternativeTextureBorder);
            break;
        }

        case AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_FUNCTION:
        case AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE:
        {
            documentationString.append(GD_STR_DeprecationAlternativeFixedFunctionFragmentProcessing);
            break;
        }

        case AP_DEPRECATION_ACCUMMULATION_BUFFERS_FUNCTION:
        case AP_DEPRECATION_ACCUMMULATION_BUFFERS_STATE:
        {
            documentationString.append(GD_STR_DeprecationAlternativeAccumulationBuffers);
            break;
        }

        case AP_DEPRECATION_FRAMEBUFFER_SIZE_QUERIES:
        {
            documentationString.append(GD_STR_DeprecationAlternativeFramebufferSizeQueries);
            break;
        }

        case AP_DEPRECATION_PIXEL_COPY:
        {
            documentationString.append(GD_STR_DeprecationAlternativePixelCopy);
            break;
        }

        case AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES:
        {
            documentationString.append(GD_STR_DeprecationAlternativePolygonQuads);
            break;
        }

        case AP_DEPRECATION_UNIFIED_EXTENSION_STRING:
        {
            documentationString.append(GD_STR_DeprecationAlternativeUnifiedExtensionString);
            break;
        }

        case AP_DEPRECATION_AUXILIRY_BUFFERS:
        {
            documentationString.append(GD_STR_DeprecationAlternativeAuxiliryBuffers);
            break;
        }

        case AP_DEPRECATION_CLIENT_VERTEX_AND_INDEX_ARRAYS:
        {
            documentationString.append(GD_STR_DeprecationAlternativeCliendVertexAndIndexArrays);
            break;
        }

        case AP_DEPRECATION_PIXEL_TRANSFER:
        {
            documentationString.append(GD_STR_DeprecationAlternativePixelTransfer);
            break;
        }

        case AP_DEPRECATION_GLSL_VERSION:
        {
            documentationString.append(GD_STR_DeprecationAlternativeGLSLVersion);
            break;
        }

        default:
        {
            // No alternative for now:
            break;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildBatchStatisticsPropertiesString
// Description: Builds a properties string for batch statistics item
// Arguments: int minAmountOfVertices
//            int maxAmountOfVertices
//            float percentageOfVertices
//            float percentageOfBatches
//            afIconType iconType - icon type
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        24/5/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildBatchStatisticsPropertiesString(int minAmountOfVertices, int maxAmountOfVertices, float percentageOfVertices, float percentageOfBatches, afIconType iconType, gtString& propertiesHTMLMessage)
{
    if (percentageOfBatches > 0)
    {
        // Add the item icon:
        afHTMLContent::addIconPath(iconType, AF_STR_Empty, false, propertiesHTMLMessage);

        // Translate the vertex amounts to strings:
        gtString minRangeStr, maxRangeStr;
        minRangeStr.appendFormattedString(L"%d", minAmountOfVertices);
        maxRangeStr.appendFormattedString(L"%d", maxAmountOfVertices);
        minRangeStr.addThousandSeperators();
        maxRangeStr.addThousandSeperators();

        // Create the batch statistics item title:
        if ((maxAmountOfVertices != 0) && (minAmountOfVertices != maxAmountOfVertices))
        {
            propertiesHTMLMessage.append(AF_STR_HtmlPropertiesNonbreakingSpace);
            propertiesHTMLMessage.append(AF_STR_HtmlPropertiesNonbreakingSpace);
            propertiesHTMLMessage.appendFormattedString(GD_STR_HtmlPropertiesBatchRangeStatisticsTitle, minRangeStr.asCharArray(), maxRangeStr.asCharArray());
        }
        else
        {
            propertiesHTMLMessage.append(AF_STR_HtmlPropertiesNonbreakingSpace);
            propertiesHTMLMessage.append(AF_STR_HtmlPropertiesNonbreakingSpace);
            propertiesHTMLMessage.appendFormattedString(GD_STR_HtmlPropertiesBatchStatisticsTitle, minRangeStr.asCharArray());
        }

        // Create the batch statistics item title:
        propertiesHTMLMessage.appendFormattedString(GD_STR_HtmlPropertiesBatchStatisticsDescription, percentageOfVertices, percentageOfBatches);
    }

    // Add light bulb icon:
    afHTMLContent::addIconPath(AF_ICON_LIGHT_BULB, AF_STR_Empty, true, propertiesHTMLMessage);
    propertiesHTMLMessage.append(AF_STR_HtmlPropertiesNonbreakingSpace);
    propertiesHTMLMessage.append(AF_STR_HtmlPropertiesNonbreakingSpace);

    // Add a general explanation for best practice of vertices batches:
    propertiesHTMLMessage.append(GD_STR_HtmlPropertiesBatchStatisticsBestPractice);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildSyncObjectHTMLPropertiesString
// Description: Create a sync object HTML string
// Arguments: const apGLSync& syncObjectDetails
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        28/10/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildSyncObjectHTMLPropertiesString(const apGLSync& syncObjectDetails, afHTMLContent& htmlContent)
{
    gtString strSyncId;
    strSyncId.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeSyncName, syncObjectDetails.syncID());

    gtString strSyncHandle;
    gdUserApplicationAddressToDisplayString(syncObjectDetails.syncHandle(), strSyncHandle);

    // Build the HTML content:
    htmlContent.setTitle(strSyncId);

    // Create the Header "General":
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    // Add the sync handle line:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSyncHandle, strSyncHandle);

    // Add the explanation about sync objects having no size:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesMemorySizeSyncObjectsExplanation AF_STR_HtmlNewLine AF_STR_HtmlNewLine GD_STR_PropertiesMemorySizeNAExplanation);

    // TO_DO: See if there's any more info to add here:
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildSyncListHTMLPropertiesString
// Description: Create sync objects general HTML string
// Arguments:   int amountOfSyncObjects
//              afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        28/10/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildSyncListHTMLPropertiesString(int amountOfSyncObjects, afHTMLContent& htmlContent)
{
    (void)(amountOfSyncObjects);  // unused
    // Build the HTML content:
    htmlContent.setTitle(GD_STR_PropertiesSyncObjects);

    // Add the "syncs have no size" explanation:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesMemorySizeSyncObjectsExplanation AF_STR_HtmlNewLine AF_STR_HtmlNewLine GD_STR_PropertiesMemorySizeNAExplanation);

    // Add the "choose an item" line:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesSelectItemMessage);
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::addProgramGeometryParametersToHTMLContent
// Description: Adds the program geometry parameters (GL_GEOMETRY_INPUT_TYPE_EXT, GL_GEOMETRY_OUTPUT_TYPE_EXT
//              and GL_GEOMETRY_VERTICES_OUT_EXT) if the hardware supports them:
// Arguments: afHTMLContent& htmlContent
//            const apGLProgram& programDetails
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        12/11/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::addProgramGeometryParametersToHTMLContent(afHTMLContent& htmlContent, const apGLProgram& programDetails)
{
    // Check (only one time) if the extension is supported on this machine:
    static bool stat_isFirstTimeCheckingForGeometrySupport = true;
    static bool stat_isGeometryShadingSupported = false;

    if (stat_isFirstTimeCheckingForGeometrySupport)
    {
        stat_isFirstTimeCheckingForGeometrySupport = false;

        oaOpenGLRenderContext* pDefaultRenderContext = oaOpenGLRenderContext::getDefaultRenderContext();
        GT_IF_WITH_ASSERT(pDefaultRenderContext != NULL)
        {
            stat_isGeometryShadingSupported = pDefaultRenderContext->isExtensionSupported(L"GL_EXT_geometry_shader4");
        }
    }

    if (stat_isGeometryShadingSupported)
    {
        // Add heading to this section:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_COLOR_SUBTITLE, GD_STR_PropertiesProgramGeometryParametersTitle);

        // Add name/ value table cell titles:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, AF_STR_Name, AF_STR_Type);

        apGLPrimitiveTypeParameter geometryInput(programDetails.geometryInputType());
        apGLPrimitiveTypeParameter geometryOutput(programDetails.geometryOutputType());
        gtString geometryInputAsString;
        gtString geometryOutputAsString;
        geometryInput.valueAsString(geometryInputAsString);
        geometryOutput.valueAsString(geometryOutputAsString);

        // Add input type table row:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, L"GL_GEOMETRY_INPUT_TYPE_EXT", geometryInputAsString);
        // Add output type table row:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, L"GL_GEOMETRY_OUTPUT_TYPE_EXT", geometryOutputAsString);

        // Add geometry vertices out table row:
        gtString geomVerticesAmountStr;
        geomVerticesAmountStr.appendFormattedString(L"%d", programDetails.maxGeometryOutputVertices());
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, L"GL_GEOMETRY_VERTICES_OUT_EXT", geomVerticesAmountStr);

    }
    else
    {
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesGeometryParametersNA);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLDeviceHTMLPropertiesString
// Description: Builds an OpenCL device's HTML properties page
// Author:      Uri Shomroni
// Date:        28/3/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLDeviceHTMLPropertiesString(const apCLDevice& deviceDetails, int numberOfQueuesOnDevice, bool showQueuesNumber, afHTMLContent& htmlContent)
{
    // Convert the device details to strings:
    gtString deviceTitleStr;
    deviceTitleStr.appendFormattedString(GD_STR_PropertiesDeviceTitle, apCLDeviceIndexToDisplayIndex(deviceDetails.APIID()));
    gtString deviceQueuesNumberStr;
    deviceQueuesNumberStr.appendFormattedString(L"%d", numberOfQueuesOnDevice);
    gtString deviceTypeStr;
    apCLDeviceTypeAsString(deviceDetails.deviceType(), deviceTypeStr);
    const gtString& deviceName = deviceDetails.deviceNameForDisplay();
    const gtString& deviceVersion = deviceDetails.deviceVersion();
    const gtString& deviceVendor = deviceDetails.deviceVendor();
    gtString deviceIdStr;
    gdUserApplicationAddressToDisplayString((osProcedureAddress64)deviceDetails.deviceHandle(), deviceIdStr);

    // Create the HTML content:
    htmlContent.setTitle(deviceTitleStr);

    if (showQueuesNumber)
    {
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDeviceNumberOfQueues, deviceQueuesNumberStr);
    }

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDeviceType, deviceTypeStr);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDeviceName, deviceName);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDeviceVersion, deviceVersion);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDeviceVendor, deviceVendor);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesCLDeviceID, deviceIdStr);

    // TO_DO: OpenCL - Add deviceAllowedQueueProperties and many other device properties that are not currently
    // expressed in apCLDevice

    // Output the HTML string:

}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLProgramHTMLPropertiesString
// Description: Builds a OpenCL program HTML properties page
// Arguments: int contextId
//            const apCLProgram& programDetails
//            bool rebaseMePlease
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        22/11/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLProgramHTMLPropertiesString(int contextId, const apCLProgram& programDetails , int programId, bool isExpanded, afHTMLContent& htmlContent)
{
    // Get program details:
    gtString programBuildStatus;
    gtString programNameString;
    gtString programLinkLogStr;
    gtString sourceCodeFilePathStr;
    gtString filePathStr;
    gtString programHandle;

    // Build the program name string:
    programNameString.appendFormattedString(GD_STR_PropertiesProgramNameFormat, programId);

    // Build the program handle string:
    gdUserApplicationAddressToDisplayString(programDetails.programHandle(), programHandle);

    // Get the status of the program build:
    apProgramBuildStatusAsString(programDetails.devicesBuildData()[0]._buildStatus, programBuildStatus);

    // Build the program source code file path string:
    const osFilePath& sourceCodeFilePath = programDetails.sourceCodeFilePath();
    osFile sourceCodeFile(sourceCodeFilePath);
    sourceCodeFilePathStr = sourceCodeFilePath.asString();

    // Build the HTML content:
    htmlContent.setTitle(programNameString);

    // Create the Header "General":
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    // Add the program handle line:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesProgramHandleHeader, programHandle);

    // Add the reference count to the table:
    gtString refCountStr;
    refCountStr.appendFormattedString(L"%d", programDetails.referenceCount());
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReferenceCountHeader, refCountStr);

    // Add the link status to the table:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesProgramBuildStatus, programBuildStatus);

    // Get the program binary type:
    gtString programBinaryType = programDetails.programBinaryTypeAsString();
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesProgramBinaryType, programBinaryType);

    // Add source code file path:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesProgramFilePath, sourceCodeFilePathStr);

    // Add program build Options:
    gtString buildOptionsStr = programDetails.devicesBuildData()[0]._buildOptions;

    if (buildOptionsStr.isEmpty())
    {
        buildOptionsStr = AF_STR_None;
    }

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesProgramBuildOptions, buildOptionsStr);

    // Get the related programs:
    const gtVector<oaCLProgramHandle>& relatedPrograms = programDetails.relatedProgramHandles();
    int numberOfRelatedPrograms = (int)relatedPrograms.size();

    if (numberOfRelatedPrograms > 0)
    {
        // Add the kernelHandles title:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesProgramRelatedProgramsHeader);

        // Add each of the program objects:
        for (int i = 0; i < numberOfRelatedPrograms; i++)
        {
            // Get the current kernel handle:
            oaCLProgramHandle currentRelatedProgram = relatedPrograms[i];

            // Get the kernel details:
            apCLObjectID relatedProgramDetails;

            // Get the current kernel details:
            bool rcID = gaGetOpenCLHandleObjectDetails(currentRelatedProgram, relatedProgramDetails);
            GT_IF_WITH_ASSERT(rcID && (relatedProgramDetails._objectType == OS_TOBJ_ID_CL_PROGRAM))
            {
                // Add a link for to the current kernel:
                gtString programLinkStr;
                int relatedProgramIndex = relatedProgramDetails._objectId;
                programLinkStr.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesCLProgramLink, relatedProgramIndex + 1, contextId);
                programLinkStr.appendFormattedString(GD_STR_PropertiesCLProgramName, relatedProgramIndex + 1);
                programLinkStr.append(GD_STR_HtmlPropertiesLinkEnd);

                // Add the kernel name to the table:
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, programLinkStr);
            }
        }
    }

    if (isExpanded)
    {
        // Get the program kernelHandles:
        const gtVector<oaCLKernelHandle>& programKernels = programDetails.kernelHandles();
        int numberOfKernels = (int)programKernels.size();

        if (numberOfKernels > 0)
        {
            // Add the kernelHandles title:
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesProgramKernelHeader);

            // Add each of the program kernelHandles:
            for (int i = 0; i < numberOfKernels; i++)
            {
                // Get the current kernel handle:
                oaCLKernelHandle kernelHandle = programKernels[i];

                // Get the kernel details:
                apCLKernel kernelDetails(OA_CL_NULL_HANDLE, -1, OA_CL_NULL_HANDLE, AF_STR_Empty);

                // Get the current kernel details:
                bool rc = gaGetOpenCLKernelObjectDetails(contextId, kernelHandle, kernelDetails);
                GT_ASSERT(rc);

                // Add a link for to the current kernel:
                gtString kernelLinkStr;
                kernelLinkStr.appendFormattedString(AF_STR_HtmlPropertiesLink3ParamStart, GD_STR_HtmlPropertiesCLKernelLink, i + 1, contextId, programId - 1);
                kernelLinkStr.append(kernelDetails.kernelFunctionName());
                kernelLinkStr.append(GD_STR_HtmlPropertiesLinkEnd);

                // Add the kernel name to the table:
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, kernelLinkStr);
            }
        }
    }

    // Get the program kernels:
    const gtVector<gtString>& kernelNames = programDetails.kernelNames();
    int numberOfKernelNames = (int)kernelNames.size();

    if (numberOfKernelNames > 0)
    {
        // Add the kernelHandles title:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesProgramKernelNamesHeader);

        // Add each of the program kernels:
        for (int i = 0; i < numberOfKernelNames; i++)
        {
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, kernelNames[i]);
        }
    }

    // Get the program devices:
    const gtVector<int>& programDevices = programDetails.devices();
    int numberOfDevices = (int)programDevices.size();

    if (numberOfDevices > 0)
    {
        // Add the kernelHandles title:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesProgramDevicesHeader);

        // Add each of the program devices:
        for (int i = 0; i < numberOfDevices; i++)
        {
            // Get the current device id:
            int deviceId = programDevices[i];

            // Get the device details:
            apCLDevice deviceDetails;
            bool rc = gaGetOpenCLDeviceObjectDetails(deviceId, deviceDetails);
            GT_IF_WITH_ASSERT(rc)
            {
                // Add the device id to the table:
                gtString deviceTypeAsStr;
                apCLDeviceTypeAsString(deviceDetails.deviceType(), deviceTypeAsStr);
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, deviceDetails.deviceNameForDisplay(), deviceTypeAsStr);
            }
        }
    }

    // Add program build log (in 2 table rows) (should be always last item in table):
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesProgramBuildLog);
    gtString buildLogStr = programDetails.devicesBuildData()[0]._buildLog;

    if (buildLogStr.isEmpty())
    {
        buildLogStr = AF_STR_EmptyStr;
    }

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, buildLogStr);
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLProgramsListHTMLPropertiesString
// Description: builds a message for the OpenCL programs list in the given context
// Author:      Sigal Algranaty
// Date:        10/3/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLProgramsListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects)
{
    (void)(numberOfObjects);  // unused
    // Create the Header "Programs list" title:
    gtString programsListTitle;
    programsListTitle.appendFormattedString(GD_STR_PropertiesCLProgramsHeadline, contextID._contextId);

    // Build the HTML content:
    htmlContent.setTitle(programsListTitle);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesSelectItemMessage);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLSamplerHTMLPropertiesString
// Description: Builds a OpenCL sampler HTML properties page
// Arguments:   int contextId
//              const apCLProgram& programDetails
//              bool rebaseMePlease
//              afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        22/11/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLSamplerHTMLPropertiesString(int contextId, const apCLSampler& samplerDetails, afHTMLContent& htmlContent)
{
    (void)(contextId);  // unused
    // Get sampler details:
    gtString samplerName;
    gtString samplerHandle;
    gtString areCoordsNormalized;
    gtString addressingMode;
    gtString filterMode;
    gtString referenceCount;

    if (!samplerDetails.samplerName().isEmpty())
    {
        // cl_gremedy_object_naming:
        samplerName = samplerDetails.samplerName();
    }
    else
    {
        // Build the sampler name string:
        samplerName.appendFormattedString(GD_STR_PropertiesSamplerNameFormat, samplerDetails.samplerId());
    }

    // Build the sampler handle string:
    gdUserApplicationAddressToDisplayString(samplerDetails.samplerHandle(), samplerHandle);

    // Build the string for the "are coord normalized" property:
    areCoordsNormalized = (samplerDetails.areCoordsNormalized()) ?  AF_STR_Yes : AF_STR_No;

    // Build the string for the filter mode:
    filterMode = apCLSampler::filterModeAsString(samplerDetails.filterMode());

    // Build the string for the addressing mode:
    addressingMode = apCLSampler::addressingModeAsString(samplerDetails.addressingMode());

    // Build the HTML content:
    htmlContent.setTitle(samplerName);

    // Create the Header "General":
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    // Add the sampler handle line:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSamplerHandleHeader, samplerHandle);

    // Add the sampler coords normalized:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSamplerAreCoordsNormalizedHeader, areCoordsNormalized);

    // Add the sampler coords normalized:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSamplerFilterModeHeader, filterMode);

    // Add the sampler coords normalized:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSamplerAddressingModeHeader, addressingMode);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLSamplersListHTMLPropertiesString
// Description: builds a message for the OpenCL sampelrs list in the given context
// Author:      Sigal Algranaty
// Date:        4/5/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLSamplersListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects)
{
    (void)(numberOfObjects);  // unused
    // Create the Header "Samplers list" title:
    gtString samplersListTitle;
    samplersListTitle.appendFormattedString(GD_STR_PropertiesCLSamplersHeadline, contextID._contextId);

    // Build the HTML content:
    htmlContent.setTitle(samplersListTitle);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesSelectItemMessage);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLEventHTMLPropertiesString
// Description: Builds a OpenCL event HTML properties page
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLEventHTMLPropertiesString(int contextId, int eventIndex, const apCLEvent& eventDetails, afHTMLContent& htmlContent)
{
    (void)(contextId);  // unused
    // Get event details:
    gtString eventName;
    gtString eventHandle;
    gtString eventQueue;
    gtString referenceCount;

    if (!eventDetails.eventName().isEmpty())
    {
        // cl_gremedy_object_naming:
        eventName = eventDetails.eventName();
    }
    else
    {
        // Build the event name string:
        eventName.appendFormattedString(GD_STR_PropertiesCLEventName, eventIndex);
    }

    // Build the event handle string:
    gdUserApplicationAddressToDisplayString(eventDetails.eventHandle(), eventHandle);

    // Build the HTML content:
    htmlContent.setTitle(eventName);

    // Create the Header "General":
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    // Add the event handle line:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesEventHandleHeader, eventHandle);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLEventsListHTMLPropertiesString
// Description: builds a message for the OpenCL events list in the given context
// Author:      Sigal Algranaty
// Date:        4/5/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLEventsListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects)
{
    (void)(numberOfObjects);  // unused
    // Create the Header "events list" title:
    gtString eventsListTitle;
    eventsListTitle.appendFormattedString(GD_STR_PropertiesCLEventsHeadline, contextID._contextId);

    // Build the HTML content:
    htmlContent.setTitle(eventsListTitle);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesSelectItemMessage);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLKernelHTMLPropertiesString
// Description: Builds a OpenCL kernel HTML properties page
// Arguments: int contextId
//            const apCLKernel& kernelDetails
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        24/11/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLKernelHTMLPropertiesString(int contextId, const apCLKernel& kernelDetails , int kernelId, afHTMLContent& htmlContent)
{
    (void)(contextId);  // unused
    // Get program details:
    gtString programBuildStatus;
    gtString programHandle;
    gtString kernelHandle;
    gtString contextHandle;
    gtString kernelIdStr;

    // Build the kernel handles string:
    kernelHandle.appendFormattedString(L"0x%x", kernelDetails.kernelHandle());

    // Build the kernel id string:
    kernelIdStr.appendFormattedString(GD_STR_PropertiesKernelNameFormat, kernelId);

    // Build the HTML content:
    htmlContent.setTitle(kernelIdStr);

    // Create the Header "General":
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    // Add the kernel name line:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesKernelFunctionName, kernelDetails.kernelFunctionName());

    // Add the kernel handle line:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesKernelHandle, kernelHandle);

    // Add the reference count to the table:
    gtString refCountStr;
    refCountStr.appendFormattedString(L"%d", kernelDetails.referenceCount());
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReferenceCountHeader, refCountStr);

    // Add amount of arguments line:
    gtString amountStr;
    amountStr.appendFormattedString(L"%d", kernelDetails.numArgs());
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesKernelNumArgsHeader, amountStr);

    // Add a list of the kernel arguments:
    int amountOfKernelArgs = kernelDetails.amountOfKernelArgsInfos();

    if (amountOfKernelArgs > 0)
    {
        // Create the Header "Kernel args":
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesKernelArgsInfoHeader);
    }

    for (int i = 0; i < amountOfKernelArgs; i++)
    {
        gtString currentKernelArgInfo;
        bool rc = kernelDetails.getKernelArgsInfoAsString(i, currentKernelArgInfo);

        if (rc)
        {
            // Add the kernel arg info to the HTML:
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, currentKernelArgInfo);
        }
    }

    // Add workgroup information:
    int amountOfWGInfos = kernelDetails.amountOfKernelWorkgroupInfos();

    if (amountOfWGInfos > 0)
    {
        // Create the Header "Kernel args":
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesKernelWorkgroupInfoHeader);
    }

    for (int i = 0; i < amountOfWGInfos; i++)
    {
        gtInt32 deviceId = -1;
        gtUInt64 maxWorkgroupSize = 0;
        gtUInt64 requiredWorkGroupSize[3] = { 0 };
        gtUInt64 requiredLocalMemorySize = 0;
        bool rc = kernelDetails.getKernelWorkgroupInfo(i, deviceId, maxWorkgroupSize, requiredWorkGroupSize[0], requiredWorkGroupSize[1], requiredWorkGroupSize[2], requiredLocalMemorySize);

        if (rc)
        {
            // Add the workgroup info to the HTML:
            gtString currDeviceIdAsLink;
            int deviceDisplayIndex = apCLDeviceIndexToDisplayIndex((int)deviceId);
            currDeviceIdAsLink.appendFormattedString(AF_STR_HtmlPropertiesLink1ParamStart, GD_STR_HtmlPropertiesCLDeviceLink, deviceDisplayIndex);
            currDeviceIdAsLink.appendFormattedString(AF_STR_ContextsInformationsDialogDeviceHeader, deviceDisplayIndex);
            currDeviceIdAsLink.append(GD_STR_HtmlPropertiesLinkEnd);

            gtString maxWGSizeStr;
            maxWGSizeStr.appendFormattedString(L"%d", maxWorkgroupSize);

            gtString reqdWGSizeStr;

            if ((0 != requiredWorkGroupSize[0]) || (0 != requiredWorkGroupSize[1]) || (0 != requiredWorkGroupSize[2]))
            {
                reqdWGSizeStr.appendFormattedString(L"(%d, %d, %d)", requiredWorkGroupSize[0], requiredWorkGroupSize[1], requiredWorkGroupSize[2]);
            }
            else
            {
                reqdWGSizeStr = GD_STR_PropertiesKernelRequiredWorkgroupSizeNoValue;
            }

            gtString reqdLocalMemStr;
            reqdLocalMemStr.fromMemorySize(requiredLocalMemorySize);

            if (0 < i)
            {
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SPACE, AF_STR_HtmlPropertiesNonbreakingSpace);
            }

            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationsDialogDeviceId, currDeviceIdAsLink);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesKernelMaxWorkgroupSizeHeader, maxWGSizeStr);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesKernelRequiredWorkgroupSizeHeader, reqdWGSizeStr);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesKernelRequiredLocalMemoryHeader, reqdLocalMemStr);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLBufferHTMLPropertiesString
// Description: Builds a OpenCL buffer HTML properties page
// Arguments: const apCLBuffer& bufferDetails
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLBufferHTMLPropertiesString(const apContextID& contextID, const apCLBuffer& bufferDetails, int bufferIndex, bool addBufferLink, afHTMLContent& htmlContent)
{
    (void)(bufferIndex);  // unused
    // Build the buffer name:
    gtString bufferName;
    gdGetBufferDisplayName(bufferDetails, bufferName);

    // Build the buffers flag string:
    gtString bufferFlagsStr;
    apCLMemFlags bufferFlags = bufferDetails.memoryFlags();
    bufferFlags.valueAsString(bufferFlagsStr);

    // Replace the "|" character with HTML new line:
    bufferFlagsStr.replace(L"|", AF_STR_HtmlNewLine);

    // Get the destructor info:
    gtString destructorPfnNotifyStr;
    gtString destructorUserDataStr;
    const osProcedureAddress64& pfnNotify = bufferDetails.destructorPfnNotify();
    const osProcedureAddress64& userData = bufferDetails.destructorUserData();

    if ((OS_NULL_PROCEDURE_ADDRESS_64 == pfnNotify) && (OS_NULL_PROCEDURE_ADDRESS_64 == userData))
    {
        destructorPfnNotifyStr = AF_STR_NotAvailable;
        destructorUserDataStr = AF_STR_NotAvailable;
    }
    else // (OS_NULL_PROCEDURE_ADDRESS_64 != pfnNotify) || (OS_NULL_PROCEDURE_ADDRESS_64 != userData)
    {
        gdUserApplicationAddressToDisplayString(pfnNotify, destructorPfnNotifyStr);
        gdUserApplicationAddressToDisplayString(userData, destructorUserDataStr);
    }

    // Get buffer size:
    gtSize_t bufferSize = bufferDetails.bufferSize();

    if (bufferSize > 0)
    {
        // We have the information in bits, convert it to kilobytes
        bufferSize = (gtSize_t)ceil((float)bufferSize / (1024.0F));

        if (bufferSize == 0)
        {
            bufferSize = 1;
        }
    }

    // Build the buffer handle string:
    gtString strBufferHandle;
    gdUserApplicationAddressToDisplayString(bufferDetails.memObjectHandle(), strBufferHandle);

    // Build the buffer size string:
    gtString strBufferSize;
    strBufferSize.appendFormattedString(L"%u", bufferSize);
    strBufferSize.addThousandSeperators();
    strBufferSize.append(AF_STR_KilobytesShort);

    // Build the OpenGL buffer link:
    gtString openGLBufferLink;

    if (bufferDetails.openGLSpyID() > 0)
    {
        openGLBufferLink.makeEmpty();
        openGLBufferLink.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesGLVBOLink, bufferDetails.openGLBufferName(), bufferDetails.openGLSpyID());
        openGLBufferLink.appendFormattedString(GD_STR_PropertiesGLVBOFullName, bufferDetails.openGLSpyID(), bufferDetails.openGLBufferName());
        openGLBufferLink.append(GD_STR_HtmlPropertiesLinkEnd);
    }

    // Build the HTML content:
    htmlContent.setTitle(bufferName);

    // Add "General" Subtitle:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    if (addBufferLink)
    {
        // Add the buffer name with a link:
        gtString bufferNameWithLink;
        bufferNameWithLink.prependFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesCLBufferLink, bufferDetails.bufferName(), contextID._contextId);
        bufferNameWithLink.append(bufferName);
        bufferNameWithLink.append(GD_STR_HtmlPropertiesLinkEnd);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBufferNameTitle, bufferNameWithLink);
    }
    else // !addBufferLink
    {
        // Add the buffer name without link:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBufferNameTitle, bufferName);
    }

    // Add buffer handle:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBufferHandleHeader, strBufferHandle);

    // Add buffer size:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_Size, strBufferSize);

    // Add buffer flags:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesFlagsTitle, bufferFlagsStr);

    // Add destructor info:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDestructorPfnNotifyTitle, destructorPfnNotifyStr);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDestructorUserDataTitle, destructorUserDataStr);

    if (!openGLBufferLink.isEmpty())
    {
        // Add the buffer OpenGL shared buffer:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesCLBufferGLShared, openGLBufferLink);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLSubBufferHTMLPropertiesString
// Description: Builds a OpenCL buffer HTML properties page
// Arguments:   const apCLSubBuffer& subBufferDetails
//              afHTMLContent& htmlContent
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/10/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLSubBufferHTMLPropertiesString(const apContextID& contextID, const apCLSubBuffer& subBufferDetails, bool addSubBufferLink, afHTMLContent& htmlContent)
{
    // Build the sub buffer name:
    gtString subBufferName;
    subBufferName.appendFormattedString(GD_STR_TexturesViewerNameCLSubBuffer, subBufferDetails.subBufferName());

    // Build the buffers flag string:
    gtString bufferFlagsStr;
    apCLMemFlags bufferFlags = subBufferDetails.memoryFlags();
    bufferFlags.valueAsString(bufferFlagsStr);

    // Replace the "|" character with HTML new line:
    bufferFlagsStr.replace(L"|", AF_STR_HtmlNewLine);

    // Get the destructor info:
    gtString destructorPfnNotifyStr;
    gtString destructorUserDataStr;
    const osProcedureAddress64& pfnNotify = subBufferDetails.destructorPfnNotify();
    const osProcedureAddress64& userData = subBufferDetails.destructorUserData();

    if ((OS_NULL_PROCEDURE_ADDRESS_64 == pfnNotify) && (OS_NULL_PROCEDURE_ADDRESS_64 == userData))
    {
        destructorPfnNotifyStr = AF_STR_NotAvailable;
        destructorUserDataStr = AF_STR_NotAvailable;
    }
    else // (OS_NULL_PROCEDURE_ADDRESS_64 != pfnNotify) || (OS_NULL_PROCEDURE_ADDRESS_64 != userData)
    {
        gdUserApplicationAddressToDisplayString(pfnNotify, destructorPfnNotifyStr);
        gdUserApplicationAddressToDisplayString(userData, destructorUserDataStr);
    }

    // Add the buffer creation type:
    cl_buffer_create_type createType = subBufferDetails.subBufferCreateType();

    // Region create type:
    gtString strCreateType, strBufferRegion;

    if (createType == CL_BUFFER_CREATE_TYPE_REGION)
    {
        // Get the create type as string:
        apCLEnumValueToString(createType, strCreateType);

        // Get the sub buffer size:
        size_t subBufferSize = subBufferDetails.bufferRegion().size;

        if (subBufferSize > 0)
        {
            // We have the information in bits, convert it to kilobytes
            subBufferSize = (gtSize_t)ceil((float)subBufferSize / (1024.0F));

            if (subBufferSize == 0)
            {
                subBufferSize = 1;
            }
        }

        // Build the buffer size string:
        gtString strSubBufferSize;
        strSubBufferSize.appendFormattedString(L"%u", subBufferSize);
        strSubBufferSize.addThousandSeperators();
        strSubBufferSize.append(AF_STR_KilobytesShort);

        // Display the buffer origin as pointer:
        gtString bufferOriginStr;
        gdUserApplicationAddressToDisplayString((osProcedureAddress64)subBufferDetails.bufferRegion().origin, bufferOriginStr);

        // Get the buffer region as string:
        strBufferRegion.appendFormattedString(GD_STR_PropertiesSubBufferRegionFormat, bufferOriginStr.asCharArray(), strSubBufferSize.asCharArray());
    }
    else
    {
        GT_ASSERT_EX(false, GD_STR_PropertiesSubBufferUnsupportedCreateType);
    }

    // Build the buffer handle string:
    gtString strBufferHandle;
    gdUserApplicationAddressToDisplayString(subBufferDetails.memObjectHandle(), strBufferHandle);

    // Build the HTML content:
    htmlContent.setTitle(subBufferName);

    // Add "General" Subtitle:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    // Add the sub-buffer name:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSubBufferName, subBufferName);

    // Build the buffer name:
    gtString bufferName;
    bufferName.appendFormattedString(GD_STR_TexturesViewerNameCLBuffer, subBufferDetails.bufferName());

    if (addSubBufferLink)
    {
        // Add the buffer name with a link:
        gtString subBufferNameWithLink;
        subBufferNameWithLink.prependFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesCLSubBufferLink, subBufferDetails.bufferName(), contextID._contextId);
        subBufferNameWithLink.append(bufferName);
        subBufferNameWithLink.append(GD_STR_HtmlPropertiesLinkEnd);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBufferNameTitle, subBufferNameWithLink);
    }
    else // !addBufferLink
    {
        // Add the buffer name with no link:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_TexturesViewerNameCLBuffer, bufferName);
    }

    // Add buffer handle:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBufferHandleHeader, strBufferHandle);

    // Add the buffer create type:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSubBufferCreateType, strCreateType);

    // Add buffer region:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSubBufferRegion, strBufferRegion);

    // Add buffer flags:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesFlagsTitle, bufferFlagsStr);

    // Add destructor info:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDestructorPfnNotifyTitle, destructorPfnNotifyStr);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDestructorUserDataTitle, destructorUserDataStr);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLBuffersListHTMLPropertiesString
// Description: builds a message for the OpenCL buffers list in the given context
// Author:      Sigal Algranaty
// Date:        2/12/2008
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLBuffersListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects)
{
    (void)(numberOfObjects);  // unused
    (void)(htmlContent);  // unused
    // Create the Header "Buffers list" title:
    gtString buffersListTitle;
    buffersListTitle.appendFormattedString(GD_STR_PropertiesCLBuffersListHeadline, contextID._contextId);

    // Build the HTML content:
    htmlContent.setTitle(buffersListTitle);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesSelectItemMessage);
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLImageHTMLPropertiesString
// Description:
// Arguments: apContextID contextID
//            apGLTextureMipLevelID textureID
//            bool addThumbnail
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        3/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLImageHTMLPropertiesString(const apContextID& contextID, int textureIndex, bool addThumbnail, afHTMLContent& htmlContent, afProgressBarWrapper* pProgressBar)
{
    // Get the texture properties
    gtString strTextureName;
    gtString strTextureType;
    gtString strTextureDimensions;
    gtString strTexturePixelFormat;
    gtString strTextureDataType;
    gtString strTextureHandle;
    gtString strTextureThumbnail;
    gtString strTextureGLLink;
    gtString destructorPfnNotifyStr;
    gtString destructorUserDataStr;
    int textureName = 0;

    // Get the texture properties as string:
    getCLImageProperties(contextID, textureIndex, textureName, strTextureName, strTextureType, strTextureDimensions, strTextureDataType, strTexturePixelFormat, strTextureHandle, strTextureGLLink, destructorPfnNotifyStr, destructorUserDataStr);

    if (addThumbnail)
    {
        GT_IF_WITH_ASSERT(pProgressBar != NULL)
        {
            // Generate the texture thumbnail string:
            bool rc1 = getTextureThumbnail(contextID, textureIndex, textureName, strTextureThumbnail, addThumbnail, pProgressBar);
            GT_ASSERT(rc1);
        }
    }

    // Build the HTML content:
    htmlContent.setTitle(strTextureName, strTextureThumbnail);

    // Add "General" Subtitle:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    // Add texture type line:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_Type, strTextureType.append(GD_STR_PropertiesImageSuffix));

    // Texture handle:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesImageHandleHeader, strTextureHandle);

    // Add the texture dimensions:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDimensions, strTextureDimensions);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesImageFormat);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPixelFormat, strTexturePixelFormat);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDataType, strTextureDataType);

    // Add destructor info:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDestructorPfnNotifyTitle, destructorPfnNotifyStr);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDestructorUserDataTitle, destructorUserDataStr);

    if (!strTextureGLLink.isEmpty())
    {
        // Add the image OpenGL shared texture:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesCLImageGLShared, strTextureGLLink);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLPipeHTMLPropertiesString
// Description: Builds a OpenCL pipe HTML properties page
// Author:      Uri Shomroni
// Date:        1/10/2014
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLPipeHTMLPropertiesString(const apContextID& contextID, const apCLPipe& pipeDetails, int pipeIndex, bool addPipeLink, afHTMLContent& htmlContent)
{
    (void)(pipeIndex);  // unused

    // Build the pipe name:
    gtString pipeName;
    gdGetPipeDisplayName(pipeDetails, pipeName);

    // Build the pipes flag string:
    gtString pipeFlagsStr;
    apCLMemFlags pipeFlags = pipeDetails.memoryFlags();
    pipeFlags.valueAsString(pipeFlagsStr);

    // Replace the "|" character with HTML new line:
    pipeFlagsStr.replace(L"|", AF_STR_HtmlNewLine);

    // Get the destructor info:
    gtString destructorPfnNotifyStr;
    gtString destructorUserDataStr;
    const osProcedureAddress64& pfnNotify = pipeDetails.destructorPfnNotify();
    const osProcedureAddress64& userData = pipeDetails.destructorUserData();

    if ((OS_NULL_PROCEDURE_ADDRESS_64 == pfnNotify) && (OS_NULL_PROCEDURE_ADDRESS_64 == userData))
    {
        destructorPfnNotifyStr = AF_STR_NotAvailable;
        destructorUserDataStr = AF_STR_NotAvailable;
    }
    else // (OS_NULL_PROCEDURE_ADDRESS_64 != pfnNotify) || (OS_NULL_PROCEDURE_ADDRESS_64 != userData)
    {
        gdUserApplicationAddressToDisplayString(pfnNotify, destructorPfnNotifyStr);
        gdUserApplicationAddressToDisplayString(userData, destructorUserDataStr);
    }

    // Get pipe packet size:
    gtUInt32 pipePacketSize = pipeDetails.pipePacketSize();

    // Get pipe max packets:
    gtUInt32 pipeMaxPackets = pipeDetails.pipeMaxPackets();

    // Calculate the pipe size.We have the information in bits, convert it to kilobytes:
    gtUInt32 pipeSize = (gtUInt32)(((gtUInt64)pipePacketSize * pipeMaxPackets + 1023) / 1024);

    // Build the pipe handle string:
    gtString strPipeHandle;
    gdUserApplicationAddressToDisplayString(pipeDetails.memObjectHandle(), strPipeHandle);

    // Build the pipe packet size string:
    gtString strPipePacketSize;
    strPipePacketSize.appendFormattedString(L"%u", pipePacketSize);

    // Build the pipe max packets string:
    gtString strPipeMaxPackets;
    strPipeMaxPackets.appendFormattedString(L"%u", pipeMaxPackets);

    // Build the pipe size string:
    gtString strPipeSize;
    strPipeSize.appendFormattedString(L"%u", pipeSize);
    strPipeSize.addThousandSeperators();
    strPipeSize.append(AF_STR_KilobytesShort);

    // Build the HTML content:
    htmlContent.setTitle(pipeName);

    // Add "General" Subtitle:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    if (addPipeLink)
    {
        // Add the pipe name with a link:
        gtString pipeNameWithLink;
        pipeNameWithLink.prependFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesCLPipeLink, pipeDetails.pipeName(), contextID._contextId);
        pipeNameWithLink.append(pipeName);
        pipeNameWithLink.append(GD_STR_HtmlPropertiesLinkEnd);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPipeNameTitle, pipeNameWithLink);
    }
    else // !addPipeLink
    {
        // Add the pipe name without link:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPipeNameTitle, pipeName);
    }

    // Add pipe handle:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPipeHandleHeader, strPipeHandle);

    // Add pipe packet size:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPipePacketSize, strPipePacketSize);

    // Add pipe max packets count:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesPipeMaxPackets, strPipeMaxPackets);

    // Add pipe size:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_Size, strPipeSize);

    // Add pipe flags:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesFlagsTitle, pipeFlagsStr);

    // Add destructor info:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDestructorPfnNotifyTitle, destructorPfnNotifyStr);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDestructorUserDataTitle, destructorUserDataStr);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLPipesListHTMLPropertiesString
// Description: builds a message for the OpenCL pipes list in the given context
// Author:      Uri Shomroni
// Date:        1/10/2014
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLPipesListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects)
{
    (void)(numberOfObjects);  // unused
    (void)(htmlContent);  // unused
    // Create the Header "Pipes list" title:
    gtString pipesListTitle;
    pipesListTitle.appendFormattedString(GD_STR_PropertiesCLPipesListHeadline, contextID._contextId);

    // Build the HTML content:
    htmlContent.setTitle(pipesListTitle);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesSelectItemMessage);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLCommandQueueHTMLPropertiesString
// Description: Builds a OpenCL command queue HTML properties page
// Arguments: const apCLCommandQueue& commandQueueDetails - the command queue object
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        16/2/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLCommandQueueHTMLPropertiesString(const apContextID& contextID, const apCLCommandQueue& commandQueueDetails, int queueIndex, bool addQueueLink, afHTMLContent& htmlContent)
{
    (void)(contextID);  // unused
    // Build the command queue name:
    gtString commandQueueName;
    commandQueueName.appendFormattedString(GD_STR_PropertiesCLCommandQueueName, queueIndex);

    // Build the HTML content:
    htmlContent.setTitle(commandQueueName);

    // Add "General" Subtitle:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    // Add command queue handle:
    gtString strCommandQueueHandle;
    gdUserApplicationAddressToDisplayString(commandQueueDetails.commandQueueHandle(), strCommandQueueHandle);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_PropertiesHandle, strCommandQueueHandle);

    if (addQueueLink)
    {
        commandQueueName = L"";
        commandQueueName.appendFormattedString(GD_STR_PropertiesCLCommandQueueName , queueIndex);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesCommmandQueueNameTitle, commandQueueName);
    }

    // Add Device Index:
    gtString strCommandQueueDeviceIndex;
    strCommandQueueDeviceIndex.appendFormattedString(AF_STR_ContextsInformationsDialogDeviceHeader, apCLDeviceIndexToDisplayIndex(commandQueueDetails.deviceIndex()));
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesCLDeviceID, strCommandQueueDeviceIndex);

    // Add out of order execution mode:
    gtString strCommandQueueOutOfOrder = (commandQueueDetails.outOfOrderExecutionModeEnable()) ? AF_STR_Enabled : AF_STR_Disabled;
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueOutOfOrderExecutionMode, strCommandQueueOutOfOrder);

    // Add profiling mode:
    gtString strCommandQueueProfiling = (commandQueueDetails.profilingModeEnable()) ? AF_STR_Enabled : AF_STR_Disabled;
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueProfilingMode, strCommandQueueProfiling);

    // Queue on-device details:
    bool isOnDevice = commandQueueDetails.queueOnDevice();
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueOnDevice, isOnDevice ? GD_STR_PropertiesQueueOnDeviceOnDevice : GD_STR_PropertiesQueueOnDeviceHost);

    if (isOnDevice)
    {
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueOnDeviceDefault, commandQueueDetails.isDefaultOnDeviceQueue() ? AF_STR_Yes : AF_STR_No);
        gtString queueSizeStr;
        queueSizeStr.appendFormattedString(AF_STR_UIntFormat, commandQueueDetails.queueSize());
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueSize, queueSizeStr);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLCommandQueuesListHTMLPropertiesString
// Description: builds a message for the OpenCL command queues list in the given context
// Author:      Sigal Algranaty
// Date:        16/2/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLCommandQueuesListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects)
{
    (void)(numberOfObjects);  // unused
    // Create the Header "Command Queues list" title:
    gtString commandQueuesListTitle;
    commandQueuesListTitle.appendFormattedString(GD_STR_PropertiesCLCommandQueuesListHeadline, contextID._contextId);

    // Build the HTML content:
    htmlContent.setTitle(commandQueuesListTitle);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesSelectItemMessage);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLEnqueuedCommandHTMLPropertiesString
// Description: Builds a message for a command enqueued in an OpenCL command queue
// Author:      Uri Shomroni
// Date:        3/3/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLEnqueuedCommandHTMLPropertiesString(const apCLEnqueuedCommand& commandDetails, gtString& commandName, afHTMLContent& htmlContent)
{
    // Create the HTML content:
    htmlContent.setTitle(commandName);

    // Get the command type:
    bool isIdle = (commandDetails. type() == OS_TOBJ_ID_CL_QUEUE_IDLE_TIME);

    // Idles don't have many of the other parameters:
    if (isIdle)
    {
        // Get the idle time:
        gtUInt64 idleTime = commandDetails.executionDuration();
        gtString idleAsString;
        idleAsString.appendFormattedString(L"%llu", idleTime).addThousandSeperators().append(GD_STR_PropertiesQueueCommandNanoseconds);

        // Add a line for it:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandIdleTime, idleAsString);
    }
    else // !isIdle
    {
        // Add the times:
        gtUInt64 duration = commandDetails.executionDuration();
        gtString durationAsString;
        durationAsString.appendFormattedString(L"%llu", duration).addThousandSeperators().append(GD_STR_PropertiesQueueCommandNanoseconds);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandDuration, durationAsString);
        gtUInt64 waitedForSubmission = commandDetails.waitForSubmit();
        gtString waitedForSubmissionAsString;
        waitedForSubmissionAsString.appendFormattedString(L"%llu", waitedForSubmission).addThousandSeperators().append(GD_STR_PropertiesQueueCommandNanoseconds);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandWaitedForSubmission, waitedForSubmissionAsString);
        gtUInt64 waitedForExecution = commandDetails.waitForExecution();
        gtString waitedForExecutionAsString;
        waitedForExecutionAsString.appendFormattedString(L"%llu", waitedForExecution).addThousandSeperators().append(GD_STR_PropertiesQueueCommandNanoseconds);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandWaitedForExecution, waitedForExecutionAsString);

        // Add command-specific items:
        addCommandSpecificParametersToHtmlContent(htmlContent, commandDetails);
    }

    // Output the string:

}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildFunctionCallArguments
// Description: Builds a function call arguments string
// Arguments: const gtList<const apParameter*>& funcArguments
//            gtString& funcArgsStr
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/12/2009
// ---------------------------------------------------------------------------
bool gdHTMLProperties::buildFunctionCallArguments(apContextID functionContextId, gtAutoPtr<apFunctionCall> aptrFunctionCall, const gtList<const apParameter*>& funcArguments, afProgressBarWrapper* pProgressBar,
                                                  gtString& funcArgsStr, gtString& additionalParamsStr)
{
    bool retVal = true;
    // Iterate them:
    gtList<const apParameter*>::const_iterator iter = funcArguments.begin();
    gtList<const apParameter*>::const_iterator endIter = funcArguments.end();

    gtString currentArgumentValueAsString;
    funcArgsStr += AF_STR_HtmlPropertiesFunctionArgumentsListStart;

    while (iter != endIter)
    {
        // Get the current argument as parameter object:
        const apParameter* pParam = (*iter);

        // Build the current parameter string:
        bool rcParamAsStr = gdParameterAsString(pParam, currentArgumentValueAsString, true, true);
        GT_ASSERT(rcParamAsStr);

        // Add quotes for string parameters:
        if ((pParam->type() == OS_TOBJ_ID_STRING_PARAMETER) || (pParam->type() == OS_TOBJ_ID_GL_STRING_PARAMETER))
        {
            currentArgumentValueAsString.prepend(AF_STR_HtmlQuot);
            currentArgumentValueAsString.append(AF_STR_HtmlQuot);
        }

        // Add this argument to the arguments:
        funcArgsStr += currentArgumentValueAsString;
        iter++;

        // Add the "," only if it is NOT the last parameter
        if (iter != endIter)
        {
            funcArgsStr += L", ";
        }
    }

    funcArgsStr += AF_STR_HtmlPropertiesFunctionArgumentsListEnd;

    // Replace the \n with <br> to show the matrix value in HTMLCtrl
    funcArgsStr.replace(AF_STR_NewLine, AF_STR_HtmlNewLine, true);

    // Add additional data parameters:
    if (functionContextId.isOpenGLContext())
    {
        getAdditionalDataParametersPropertiesViewMessage(functionContextId, *aptrFunctionCall, additionalParamsStr, pProgressBar);
    }

    // Add a line break when displaying matrix
    if (funcArgsStr.find(AF_STR_HtmlNewLine) != -1)
    {
        funcArgsStr += AF_STR_HtmlNewLine;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::addCommandSpecificParametersToHtmlContent
// Description: Adds parameters specific to a command type to the html content
// Author:      Uri Shomroni
// Date:        4/3/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::addCommandSpecificParametersToHtmlContent(afHTMLContent& htmlContent, const apCLEnqueuedCommand& commandDetails)
{
    osTransferableObjectType commandType = commandDetails.type();

    // The event wait list and event parameters are added at the end, where needed:
    switch (commandType)
    {
        case OS_TOBJ_ID_CL_ACQUIRE_GL_OBJECTS_COMMAND:
        {
            // Cast into the command type:
            const apCLAcquireGLObjectsCommand& acquireGLObjectsCmd = (const apCLAcquireGLObjectsCommand&)commandDetails;

            // Acquired GL objects:
            gtString acquiredGLObjectsAsString;
            memoryObjectsVectorToHTMLString(acquireGLObjectsCmd.memObjects(), acquiredGLObjectsAsString, true, true);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandAcquiredObjects, acquiredGLObjectsAsString);
        }
        break;

        case OS_TOBJ_ID_CL_BARRIER_COMMAND:
            // No further parameters for apCLBarrierCommand
            break;

        case OS_TOBJ_ID_CL_COPY_BUFFER_COMMAND:
        {
            // Cast into the command type:
            const apCLCopyBufferCommand& copyBufferCmd = (const apCLCopyBufferCommand&)commandDetails;

            // Source Buffer:
            gtString srcBufferAsString;
            memoryObjectHandleToHTMLString(copyBufferCmd.sourceBuffer(), srcBufferAsString, true, false);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandSourcePrefix GD_STR_PropertiesQueueCommandBuffer, srcBufferAsString);

            // Destination Buffer:
            gtString dstBufferAsString;
            memoryObjectHandleToHTMLString(copyBufferCmd.destinationBuffer(), dstBufferAsString, true, false);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandDestinationPrefix GD_STR_PropertiesQueueCommandBuffer, dstBufferAsString);

            // Source Offset:
            gtString srcOffsetAsString;
            gdUserApplicationAddressToDisplayString((osProcedureAddress64)copyBufferCmd.sourceOffset(), srcOffsetAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandSourcePrefix GD_STR_PropertiesQueueCommandOffset, srcOffsetAsString);

            // Destination Offset:
            gtString dstOffsetAsString;
            gdUserApplicationAddressToDisplayString((osProcedureAddress64)copyBufferCmd.destinationOffset(), dstOffsetAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandDestinationPrefix GD_STR_PropertiesQueueCommandOffset, dstOffsetAsString);

            // Copied Data Size:
            gtString copiedDataSizeAsString;
            copiedDataSizeAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)copyBufferCmd.copiedBytes());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandCopiedPrefix GD_STR_PropertiesQueueCommandDataSize, copiedDataSizeAsString);
        }
        break;

        case OS_TOBJ_ID_CL_COPY_BUFFER_RECT_COMMAND:
        {
            // Cast into the command type:
            const apCLCopyBufferRectCommand& copyBufferRectCmd = (const apCLCopyBufferRectCommand&)commandDetails;

            // Source Buffer:
            gtString srcBufferAsString;
            memoryObjectHandleToHTMLString(copyBufferRectCmd.sourceBuffer(), srcBufferAsString, true, false);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandSourcePrefix GD_STR_PropertiesQueueCommandBuffer, srcBufferAsString);

            // Destination Buffer:
            gtString dstBufferAsString;
            memoryObjectHandleToHTMLString(copyBufferRectCmd.destinationBuffer(), dstBufferAsString, true, false);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandDestinationPrefix GD_STR_PropertiesQueueCommandBuffer, dstBufferAsString);

            // Source Origin:
            gtString srcOriginAsString;
            srcOriginAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DOffsetValueFormat, (unsigned long)copyBufferRectCmd.sourceOriginX(), (unsigned long)copyBufferRectCmd.sourceOriginY(), (unsigned long)copyBufferRectCmd.sourceOriginZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandSourcePrefix GD_STR_PropertiesQueueCommandOrigin, srcOriginAsString);

            // Destination Origin:
            gtString dstOriginAsString;
            dstOriginAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DOffsetValueFormat, (unsigned long)copyBufferRectCmd.destinationOriginX(), (unsigned long)copyBufferRectCmd.destinationOriginY(), (unsigned long)copyBufferRectCmd.destinationOriginZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandDestinationPrefix GD_STR_PropertiesQueueCommandOrigin, dstOriginAsString);

            // Copied Region:
            gtString copiedRegionAsString;
            copiedRegionAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DSizeValueFormat, (unsigned long)copyBufferRectCmd.copiedRegionX(), (unsigned long)copyBufferRectCmd.copiedRegionY(), (unsigned long)copyBufferRectCmd.copiedRegionZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandCopiedPrefix GD_STR_PropertiesQueueCommandRegion, copiedRegionAsString);

            // Source Row Pitch:
            gtString srcRowPitchAsString;
            srcRowPitchAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)copyBufferRectCmd.sourceRowPitch());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandSourcePrefix GD_STR_PropertiesQueueCommandRowPitch, srcRowPitchAsString);

            // Source Slice Pitch:
            gtString srcSlicePitchAsString;
            srcSlicePitchAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)copyBufferRectCmd.sourceSlicePitch());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandSourcePrefix GD_STR_PropertiesQueueCommandSlicePitch, srcSlicePitchAsString);

            // Destination Row Pitch:
            gtString dstRowPitchAsString;
            dstRowPitchAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)copyBufferRectCmd.destinationRowPitch());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandDestinationPrefix GD_STR_PropertiesQueueCommandRowPitch, dstRowPitchAsString);

            // Slice Pitch:
            gtString dstSlicePitchAsString;
            dstSlicePitchAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)copyBufferRectCmd.destinationSlicePitch());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandDestinationPrefix GD_STR_PropertiesQueueCommandSlicePitch, dstSlicePitchAsString);
        }
        break;

        case OS_TOBJ_ID_CL_COPY_BUFFER_TO_IMAGE_COMMAND:
        {
            // Cast into the command type:
            const apCLCopyBufferToImageCommand& copyBufferToImageCmd = (const apCLCopyBufferToImageCommand&)commandDetails;

            // Source Buffer:
            gtString srcBufferAsString;
            memoryObjectHandleToHTMLString(copyBufferToImageCmd.sourceBuffer(), srcBufferAsString, true, false);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandSourcePrefix GD_STR_PropertiesQueueCommandBuffer, srcBufferAsString);

            // Destination Image:
            gtString dstImageAsString;
            memoryObjectHandleToHTMLString(copyBufferToImageCmd.destinationImage(), dstImageAsString, false, true);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandDestinationPrefix GD_STR_PropertiesQueueCommandImage, dstImageAsString);

            // Source Offset:
            gtString srcOffsetAsString;
            gdUserApplicationAddressToDisplayString((osProcedureAddress64)copyBufferToImageCmd.sourceOffset(), srcOffsetAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandSourcePrefix GD_STR_PropertiesQueueCommandOffset, srcOffsetAsString);

            // Destination Origin:
            gtString dstOriginAsString;
            dstOriginAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DOffsetValueFormat, (unsigned long)copyBufferToImageCmd.destinationOriginX(), (unsigned long)copyBufferToImageCmd.destinationOriginY(), (unsigned long)copyBufferToImageCmd.destinationOriginZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandDestinationPrefix GD_STR_PropertiesQueueCommandOrigin, dstOriginAsString);

            // Copied Region:
            gtString copiedRegionAsString;
            copiedRegionAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DSizeValueFormat, (unsigned long)copyBufferToImageCmd.copiedRegionX(), (unsigned long)copyBufferToImageCmd.copiedRegionY(), (unsigned long)copyBufferToImageCmd.copiedRegionZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandCopiedPrefix GD_STR_PropertiesQueueCommandRegion, copiedRegionAsString);
        }
        break;

        case OS_TOBJ_ID_CL_COPY_IMAGE_COMMAND:
        {
            // Cast into the command type:
            const apCLCopyImageCommand& copyImageCmd = (const apCLCopyImageCommand&)commandDetails;

            // Source Image:
            gtString srcImageAsString;
            memoryObjectHandleToHTMLString(copyImageCmd.sourceImage(), srcImageAsString, false, true);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandSourcePrefix GD_STR_PropertiesQueueCommandImage, srcImageAsString);

            // Destination Image:
            gtString dstImageAsString;
            memoryObjectHandleToHTMLString(copyImageCmd.destinationImage(), dstImageAsString, false, true);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandDestinationPrefix GD_STR_PropertiesQueueCommandImage, dstImageAsString);

            // Source Origin:
            gtString srcOriginAsString;
            srcOriginAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DOffsetValueFormat, (unsigned long)copyImageCmd.sourceOriginX(), (unsigned long)copyImageCmd.sourceOriginY(), (unsigned long)copyImageCmd.sourceOriginZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandSourcePrefix GD_STR_PropertiesQueueCommandOrigin, srcOriginAsString);

            // Destination Origin:
            gtString dstOriginAsString;
            dstOriginAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DOffsetValueFormat, (unsigned long)copyImageCmd.destinationOriginX(), (unsigned long)copyImageCmd.destinationOriginY(), (unsigned long)copyImageCmd.destinationOriginZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandDestinationPrefix GD_STR_PropertiesQueueCommandOrigin, dstOriginAsString);

            // Copied Region:
            gtString copiedRegionAsString;
            copiedRegionAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DSizeValueFormat, (unsigned long)copyImageCmd.copiedRegionX(), (unsigned long)copyImageCmd.copiedRegionY(), (unsigned long)copyImageCmd.copiedRegionZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandCopiedPrefix GD_STR_PropertiesQueueCommandRegion, copiedRegionAsString);
        }
        break;

        case OS_TOBJ_ID_CL_COPY_IMAGE_TO_BUFFER_COMMAND:
        {
            // Cast into the command type:
            const apCLCopyImageToBufferCommand& copyImageToBufferCmd = (const apCLCopyImageToBufferCommand&)commandDetails;

            // Source Image:
            gtString srcImageAsString;
            memoryObjectHandleToHTMLString(copyImageToBufferCmd.sourceImage(), srcImageAsString, false, true);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandSourcePrefix GD_STR_PropertiesQueueCommandImage, srcImageAsString);

            // Destination Buffer:
            gtString dstBufferAsString;
            memoryObjectHandleToHTMLString(copyImageToBufferCmd.destinationBuffer(), dstBufferAsString, true, false);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandDestinationPrefix GD_STR_PropertiesQueueCommandBuffer, dstBufferAsString);

            // Source Origin:
            gtString srcOriginAsString;
            srcOriginAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DOffsetValueFormat, (unsigned long)copyImageToBufferCmd.sourceOriginX(), (unsigned long)copyImageToBufferCmd.sourceOriginY(), (unsigned long)copyImageToBufferCmd.sourceOriginZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandSourcePrefix GD_STR_PropertiesQueueCommandOrigin, srcOriginAsString);

            // Destination Offset:
            gtString dstOffsetAsString;
            gdUserApplicationAddressToDisplayString((osProcedureAddress64)copyImageToBufferCmd.destinationOffset(), dstOffsetAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandDestinationPrefix GD_STR_PropertiesQueueCommandOffset, dstOffsetAsString);

            // Copied Region:
            gtString copiedRegionAsString;
            copiedRegionAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DSizeValueFormat, (unsigned long)copyImageToBufferCmd.copiedRegionX(), (unsigned long)copyImageToBufferCmd.copiedRegionY(), (unsigned long)copyImageToBufferCmd.copiedRegionZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandCopiedPrefix GD_STR_PropertiesQueueCommandRegion, copiedRegionAsString);
        }
        break;

        case OS_TOBJ_ID_CL_MAP_BUFFER_COMMAND:
        {
            // Cast into the command type:
            const apCLMapBufferCommand& mapBufferCmd = (const apCLMapBufferCommand&)commandDetails;

            // Mapped Buffer:
            gtString mapBufferAsString;
            memoryObjectHandleToHTMLString(mapBufferCmd.mappedBuffer(), mapBufferAsString, true, false);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandMappedPrefix GD_STR_PropertiesQueueCommandBuffer, mapBufferAsString);

            // Is Blocking Map:
            apCLBoolParameter blockingMap(mapBufferCmd.isBlockingMap());
            gtString isBlockingMapAsString;
            blockingMap.valueAsString(isBlockingMapAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandBlockingMap, isBlockingMapAsString);

            // Map flags:
            apCLMapFlagsParameter mappingFlags(mapBufferCmd.mapFlags());
            gtString mapFlagsAsString;
            mappingFlags.valueAsString(mapFlagsAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandMapFlags, mapFlagsAsString);

            // Mapped Offset:
            gtString mapOffsetAsString;
            gdUserApplicationAddressToDisplayString((osProcedureAddress64)mapBufferCmd.mappedDataOffset(), mapOffsetAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandMappedPrefix GD_STR_PropertiesQueueCommandOffset, mapOffsetAsString);

            // Mapped Data Size:
            gtString mapDataSizeAsString;
            mapDataSizeAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)mapBufferCmd.mappedDataBytes());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandMappedPrefix GD_STR_PropertiesQueueCommandDataSize, mapDataSizeAsString);

            // TO_DO: OpenCL -  mapBufferCmd.errorCodeRet()
        }
        break;

        case OS_TOBJ_ID_CL_MAP_IMAGE_COMMAND:
        {
            // Cast into the command type
            const apCLMapImageCommand& mapImageCmd = (const apCLMapImageCommand&)commandDetails;

            // Mapped Image:
            gtString mapImageAsString;
            memoryObjectHandleToHTMLString(mapImageCmd.mappedImage(), mapImageAsString, false, true);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandMappedPrefix GD_STR_PropertiesQueueCommandImage, mapImageAsString);

            // Is Blocking Map:
            apCLBoolParameter blockingMap(mapImageCmd.isBlockingMap());
            gtString isBlockingMapAsString;
            blockingMap.valueAsString(isBlockingMapAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandBlockingMap, isBlockingMapAsString);

            // Map flags:
            apCLMapFlagsParameter mappingFlags(mapImageCmd.mapFlags());
            gtString mapFlagsAsString;
            mappingFlags.valueAsString(mapFlagsAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandMapFlags, mapFlagsAsString);

            // Mapped Origin:
            gtString mapOriginAsString;
            mapOriginAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DOffsetValueFormat, (unsigned long)mapImageCmd.mappedDataOriginX(), (unsigned long)mapImageCmd.mappedDataOriginY(), (unsigned long)mapImageCmd.mappedDataOriginZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandMappedPrefix GD_STR_PropertiesQueueCommandOrigin, mapOriginAsString);

            // Mapped Region:
            gtString mappedRegionAsString;
            mappedRegionAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DSizeValueFormat, (unsigned long)mapImageCmd.mappedDataRegionX(), (unsigned long)mapImageCmd.mappedDataRegionY(), (unsigned long)mapImageCmd.mappedDataRegionZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandMappedPrefix GD_STR_PropertiesQueueCommandRegion, mappedRegionAsString);

            // Row Pitch:
            gtString rowPitchAsString;
            rowPitchAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)mapImageCmd.hostDataRowPitch());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandRowPitch, rowPitchAsString);

            // Slice Pitch:
            gtString slicePitchAsString;
            slicePitchAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)mapImageCmd.hostDataSlicePitch());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandSlicePitch, slicePitchAsString);

            // TO_DO: OpenCL -  mapImageCmd.errorCodeRet()
        }
        break;

        case OS_TOBJ_ID_CL_MARKER_COMMAND:
            // No further parameters for apCLMarkerCommand
            break;

        case OS_TOBJ_ID_CL_NATIVE_KERNEL_COMMAND:
        {
            // Cast into the command type:
            const apCLNativeKernelCommand& nativeKernelCmd = (const apCLNativeKernelCommand&)commandDetails;

            // User Function Address:
            gtString usrFunctionAddressAsString;
            gdUserApplicationAddressToDisplayString(nativeKernelCmd.userFunctionAddress(), usrFunctionAddressAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandUserFunctionAddress, usrFunctionAddressAsString);

            // Arguments Pointer:
            gtString argsPointerAsString;
            gdUserApplicationAddressToDisplayString(nativeKernelCmd.userArgumentsPointer(), argsPointerAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandArgumentsPointer, argsPointerAsString);

            // Arguments Size:
            gtString argsSizeAsString;
            argsSizeAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)nativeKernelCmd.sizeOfArgumentsData());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandArgumentsSize, argsSizeAsString);

            // Mem Obejct Arguments:
            gtString memObjArgsAsString;
            memoryObjectsVectorToHTMLString(nativeKernelCmd.memoryObjectArguments(), memObjArgsAsString, true, true);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandMemObjectArguments, memObjArgsAsString);

            // TO_DO: OpenCL - nativeKernelCmd.memoryObjectArgumentMemoryLocations()
        }
        break;

        case OS_TOBJ_ID_CL_ND_RANGE_KERNEL_COMMAND:
        {
            // Cast into the command type:
            const apCLNDRangeKernelCommand& ndRangeKernelCmd = (const apCLNDRangeKernelCommand&)commandDetails;

            // Kernel:
            gtString kernelAsString;
            gdOpenCLHandleAsString(ndRangeKernelCmd.kernelHandle(), kernelAsString, true, true, true);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandKernel, kernelAsString);

            // Work Dimensions:
            gtString workDimsAsString;
            cl_uint workDimsCLUInt = ndRangeKernelCmd.workDimensions();
            apCLuintParameter workDims(workDimsCLUInt);
            workDims.valueAsString(workDimsAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandWorkDimensions, workDimsAsString);

            // Work global offset, global size and local size:
            gtString gOffsetAsString;
            gtString gSizeAsString;
            gtString lSizeAsString;
            const gtVector<gtSize_t>& gOffset = ndRangeKernelCmd.globalWorkOffset();
            const gtVector<gtSize_t>& gSize = ndRangeKernelCmd.globalWorkSize();
            const gtVector<gtSize_t>& lSize = ndRangeKernelCmd.localWorkSize();

            if (workDimsCLUInt == 1)
            {
                gOffsetAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DOffsetValueFormat, gOffset[0]);
                gSizeAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat, gSize[0]);
                lSizeAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat, lSize[0]);
            }
            else if (workDimsCLUInt == 2)
            {
                gOffsetAsString.appendFormattedString(GD_STR_PropertiesQueueCommand2DOffsetValueFormat, gOffset[0], gOffset[1]);
                gSizeAsString.appendFormattedString(GD_STR_PropertiesQueueCommand2DSizeValueFormat, gSize[0], gSize[1]);
                lSizeAsString.appendFormattedString(GD_STR_PropertiesQueueCommand2DSizeValueFormat, lSize[0], lSize[1]);
            }
            else if (workDimsCLUInt == 3)
            {
                gOffsetAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DOffsetValueFormat, gOffset[0], gOffset[1], gOffset[2]);
                gSizeAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DSizeValueFormat, gSize[0], gSize[1], gSize[2]);
                lSizeAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DSizeValueFormat, lSize[0], lSize[1], lSize[2]);
            }
            else // workDimsCLUInt != 1, 2, 3
            {
                gOffsetAsString = AF_STR_NotAvailable;
                gSizeAsString = AF_STR_NotAvailable;
                lSizeAsString = AF_STR_NotAvailable;
            }

            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandGlobalWorkOffset, gOffsetAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandGlobalWorkSize, gSizeAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandLocalWorkSize, lSizeAsString);
        }
        break;

        case OS_TOBJ_ID_CL_READ_BUFFER_COMMAND:
        {
            // Cast into the command type:
            const apCLReadBufferCommand& readBufferCmd = (const apCLReadBufferCommand&)commandDetails;

            // Read Buffer:
            gtString readBufferAsString;
            memoryObjectHandleToHTMLString(readBufferCmd.readBuffer(), readBufferAsString, true, false);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandReadPrefix GD_STR_PropertiesQueueCommandBuffer, readBufferAsString);

            // Is Blocking Read:
            apCLBoolParameter blockingRead(readBufferCmd.isBlockingRead());
            gtString isBlockingReadAsString;
            blockingRead.valueAsString(isBlockingReadAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandBlockingRead, isBlockingReadAsString);

            // Read Offset:
            gtString readOffsetAsString;
            gdUserApplicationAddressToDisplayString((osProcedureAddress64)readBufferCmd.readDataOffset(), readOffsetAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandReadPrefix GD_STR_PropertiesQueueCommandOffset, readOffsetAsString);

            // Read Data Size:
            gtString readDataSizeAsString;
            readDataSizeAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)readBufferCmd.readDataBytes());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandReadPrefix GD_STR_PropertiesQueueCommandDataSize, readDataSizeAsString);

            // Host Data Pointer:
            gtString hostDataPtrAsString;
            gdUserApplicationAddressToDisplayString(readBufferCmd.hostDataPointer(), hostDataPtrAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandHostDataPointer, hostDataPtrAsString);
        }
        break;

        case OS_TOBJ_ID_CL_READ_BUFFER_RECT_COMMAND:
        {
            // Cast into the command type:
            const apCLReadBufferRectCommand& readBufferRectCmd = (const apCLReadBufferRectCommand&)commandDetails;

            // Read Buffer:
            gtString readBufferAsString;
            memoryObjectHandleToHTMLString(readBufferRectCmd.readBuffer(), readBufferAsString, true, false);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandReadPrefix GD_STR_PropertiesQueueCommandBuffer, readBufferAsString);

            // Is Blocking Read:
            apCLBoolParameter blockingRead(readBufferRectCmd.isBlockingRead());
            gtString isBlockingReadAsString;
            blockingRead.valueAsString(isBlockingReadAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandBlockingRead, isBlockingReadAsString);

            // Read Buffer Origin:
            gtString readOriginAsString;
            readOriginAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DOffsetValueFormat, (unsigned long)readBufferRectCmd.readDataOriginX(), (unsigned long)readBufferRectCmd.readDataOriginY(), (unsigned long)readBufferRectCmd.readDataOriginZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandReadPrefix GD_STR_PropertiesQueueCommandBufferPrefix GD_STR_PropertiesQueueCommandOrigin, readOriginAsString);

            // Host Origin:
            gtString hostOriginAsString;
            hostOriginAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DOffsetValueFormat, (unsigned long)readBufferRectCmd.readDataOriginX(), (unsigned long)readBufferRectCmd.readDataOriginY(), (unsigned long)readBufferRectCmd.readDataOriginZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandHostPrefix GD_STR_PropertiesQueueCommandOrigin, hostOriginAsString);

            // Read Region:
            gtString readRegionAsString;
            readRegionAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DSizeValueFormat, (unsigned long)readBufferRectCmd.readDataRegionX(), (unsigned long)readBufferRectCmd.readDataRegionY(), (unsigned long)readBufferRectCmd.readDataRegionZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandReadPrefix GD_STR_PropertiesQueueCommandRegion, readRegionAsString);

            // Buffer Row Pitch:
            gtString bufferRowPitchAsString;
            bufferRowPitchAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)readBufferRectCmd.readDataRowPitch());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandBufferPrefix GD_STR_PropertiesQueueCommandRowPitch, bufferRowPitchAsString);

            // Buffer Slice Pitch:
            gtString bufferSlicePitchAsString;
            bufferSlicePitchAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)readBufferRectCmd.readDataSlicePitch());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandBufferPrefix GD_STR_PropertiesQueueCommandSlicePitch, bufferSlicePitchAsString);

            // Host Row Pitch:
            gtString hostRowPitchAsString;
            hostRowPitchAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)readBufferRectCmd.hostDataRowPitch());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandHostPrefix GD_STR_PropertiesQueueCommandRowPitch, hostRowPitchAsString);

            // Host Slice Pitch:
            gtString hostSlicePitchAsString;
            hostSlicePitchAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)readBufferRectCmd.hostDataSlicePitch());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandHostPrefix GD_STR_PropertiesQueueCommandSlicePitch, hostSlicePitchAsString);

            // Host Data Pointer:
            gtString hostDataPtrAsString;
            gdUserApplicationAddressToDisplayString(readBufferRectCmd.hostDataPointer(), hostDataPtrAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandHostDataPointer, hostDataPtrAsString);
        }
        break;

        case OS_TOBJ_ID_CL_READ_IMAGE_COMMAND:
        {
            // Cast into the command type:
            const apCLReadImageCommand& readImageCmd = (const apCLReadImageCommand&)commandDetails;

            // Read Image:
            gtString readImageAsString;
            memoryObjectHandleToHTMLString(readImageCmd.readImage(), readImageAsString, false, true);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandReadPrefix GD_STR_PropertiesQueueCommandImage, readImageAsString);

            // Is Blocking Read:
            apCLBoolParameter blockingRead(readImageCmd.isBlockingRead());
            gtString isBlockingReadAsString;
            blockingRead.valueAsString(isBlockingReadAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandBlockingRead, isBlockingReadAsString);

            // Read Origin:
            gtString readOriginAsString;
            readOriginAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DOffsetValueFormat, (unsigned long)readImageCmd.readDataOriginX(), (unsigned long)readImageCmd.readDataOriginY(), (unsigned long)readImageCmd.readDataOriginZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandReadPrefix GD_STR_PropertiesQueueCommandOrigin, readOriginAsString);

            // Read Region:
            gtString readRegionAsString;
            readRegionAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DSizeValueFormat, (unsigned long)readImageCmd.readDataRegionX(), (unsigned long)readImageCmd.readDataRegionY(), (unsigned long)readImageCmd.readDataRegionZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandReadPrefix GD_STR_PropertiesQueueCommandRegion, readRegionAsString);

            // Row Pitch:
            gtString rowPitchAsString;
            rowPitchAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)readImageCmd.hostDataRowPitch());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandRowPitch, rowPitchAsString);

            // Slice Pitch:
            gtString slicePitchAsString;
            slicePitchAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)readImageCmd.hostDataSlicePitch());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandSlicePitch, slicePitchAsString);

            // Host Data Pointer:
            gtString hostDataPtrAsString;
            gdUserApplicationAddressToDisplayString(readImageCmd.hostDataPointer(), hostDataPtrAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandHostDataPointer, hostDataPtrAsString);
        }
        break;

        case OS_TOBJ_ID_CL_RELEASE_GL_OBJECTS_COMMAND:
        {
            // Cast into the command type:
            const apCLReleaseGLObjectsCommand& releaseGLObjectsCmd = (const apCLReleaseGLObjectsCommand&)commandDetails;

            // Released GL objects:
            gtString releasedGLObjectsAsString;
            memoryObjectsVectorToHTMLString(releaseGLObjectsCmd.memObjects(), releasedGLObjectsAsString, true, true);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandReleasedObjects, releasedGLObjectsAsString);
        }
        break;

        case OS_TOBJ_ID_CL_TASK_COMMAND:
        {
            // Cast into the command
            const apCLTaskCommand& taskCmd = (const apCLTaskCommand&)commandDetails;

            // Kernel:
            gtString kernelAsString;
            gdOpenCLHandleAsString(taskCmd.kernelHandle(), kernelAsString, true, true, true);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandKernel, kernelAsString);
        }
        break;

        case OS_TOBJ_ID_CL_UNMAP_MEM_OBJECT_COMMAND:
        {
            // Cast into the command type:
            const apCLUnmapMemObjectCommand& unmapMemObjectCmd = (const apCLUnmapMemObjectCommand&)commandDetails;

            // Unmapped Mem Object:
            gtString unmapMemObjAsString;
            memoryObjectHandleToHTMLString(unmapMemObjectCmd.mappedMemoryObject(), unmapMemObjAsString, true, true);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandUnmappedPrefix GD_STR_PropertiesQueueCommandMemoryObject, unmapMemObjAsString);

            // Unmapped Host Data Pointer:
            gtString unmapHostDataPtrAsString;
            gdUserApplicationAddressToDisplayString(unmapMemObjectCmd.mappedPointer(), unmapHostDataPtrAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandUnmappedPrefix GD_STR_PropertiesQueueCommandPointer, unmapHostDataPtrAsString);
        }
        break;

        case OS_TOBJ_ID_CL_WAIT_FOR_EVENTS_COMMAND:
            // No further parameters for apCLWaitForEventsCommand
            break;

        case OS_TOBJ_ID_CL_WRITE_BUFFER_COMMAND:
        {
            // Cast into the command type:
            const apCLWriteBufferCommand& writeBufferCmd = (const apCLWriteBufferCommand&)commandDetails;

            // Written Buffer:
            gtString writtenBufferAsString;
            memoryObjectHandleToHTMLString(writeBufferCmd.writtenBuffer(), writtenBufferAsString, true, false);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandWrittenPrefix GD_STR_PropertiesQueueCommandBuffer, writtenBufferAsString);

            // Is Blocking Write:
            apCLBoolParameter blockingWrite(writeBufferCmd.isBlockingWrite());
            gtString isBlockingWriteAsString;
            blockingWrite.valueAsString(isBlockingWriteAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandBlockingWrite, isBlockingWriteAsString);

            // Written Offset:
            gtString writtenOffsetAsString;
            gdUserApplicationAddressToDisplayString((osProcedureAddress64)writeBufferCmd.writtenDataOffset(), writtenOffsetAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandWrittenPrefix GD_STR_PropertiesQueueCommandOffset, writtenOffsetAsString);

            // Written Data Size:
            gtString writtenDataSizeAsString;
            writtenDataSizeAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)writeBufferCmd.writtenDataBytes());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandWrittenPrefix GD_STR_PropertiesQueueCommandDataSize, writtenDataSizeAsString);

            // Host Data Pointer:
            gtString hostDataPtrAsString;
            gdUserApplicationAddressToDisplayString(writeBufferCmd.hostDataPointer(), hostDataPtrAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandHostDataPointer, hostDataPtrAsString);
        }
        break;

        case OS_TOBJ_ID_CL_WRITE_BUFFER_RECT_COMMAND:
        {
            // Cast into the command type:
            const apCLWriteBufferRectCommand& writeBufferRectCmd = (const apCLWriteBufferRectCommand&)commandDetails;

            // Written Buffer:
            gtString writtenBufferAsString;
            memoryObjectHandleToHTMLString(writeBufferRectCmd.writtenBuffer(), writtenBufferAsString, true, false);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandWrittenPrefix GD_STR_PropertiesQueueCommandBuffer, writtenBufferAsString);

            // Is Blocking Write:
            apCLBoolParameter blockingWrite(writeBufferRectCmd.isBlockingWrite());
            gtString isBlockingWriteAsString;
            blockingWrite.valueAsString(isBlockingWriteAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandBlockingWrite, isBlockingWriteAsString);

            // Written Buffer Origin:
            gtString writtenOriginAsString;
            writtenOriginAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DOffsetValueFormat, (unsigned long)writeBufferRectCmd.writtenDataOriginX(), (unsigned long)writeBufferRectCmd.writtenDataOriginY(), (unsigned long)writeBufferRectCmd.writtenDataOriginZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandWrittenPrefix GD_STR_PropertiesQueueCommandBufferPrefix GD_STR_PropertiesQueueCommandOrigin, writtenOriginAsString);

            // Host Origin:
            gtString hostOriginAsString;
            hostOriginAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DOffsetValueFormat, (unsigned long)writeBufferRectCmd.hostDataOriginX(), (unsigned long)writeBufferRectCmd.hostDataOriginY(), (unsigned long)writeBufferRectCmd.hostDataOriginZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandHostPrefix GD_STR_PropertiesQueueCommandOrigin, hostOriginAsString);

            // Written Region:
            gtString writtenRegionAsString;
            writtenRegionAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DSizeValueFormat, (unsigned long)writeBufferRectCmd.writtenDataRegionX(), (unsigned long)writeBufferRectCmd.writtenDataRegionY(), (unsigned long)writeBufferRectCmd.writtenDataRegionZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandWrittenPrefix GD_STR_PropertiesQueueCommandRegion, writtenRegionAsString);

            // Buffer Row Pitch:
            gtString bufferRowPitchAsString;
            bufferRowPitchAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)writeBufferRectCmd.writtenDataRowPitch());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandBufferPrefix GD_STR_PropertiesQueueCommandRowPitch, bufferRowPitchAsString);

            // Buffer Slice Pitch:
            gtString bufferSlicePitchAsString;
            bufferSlicePitchAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)writeBufferRectCmd.writtenDataSlicePitch());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandBufferPrefix GD_STR_PropertiesQueueCommandSlicePitch, bufferSlicePitchAsString);

            // Host Row Pitch:
            gtString hostRowPitchAsString;
            hostRowPitchAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)writeBufferRectCmd.hostDataRowPitch());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandHostPrefix GD_STR_PropertiesQueueCommandRowPitch, hostRowPitchAsString);

            // Host Slice Pitch:
            gtString hostSlicePitchAsString;
            hostSlicePitchAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)writeBufferRectCmd.hostDataSlicePitch());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandHostPrefix GD_STR_PropertiesQueueCommandSlicePitch, hostSlicePitchAsString);

            // Host Data Pointer:
            gtString hostDataPtrAsString;
            gdUserApplicationAddressToDisplayString(writeBufferRectCmd.hostDataPointer(), hostDataPtrAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandHostDataPointer, hostDataPtrAsString);
        }
        break;

        case OS_TOBJ_ID_CL_WRITE_IMAGE_COMMAND:
        {
            // Cast into the command type:
            const apCLWriteImageCommand& writeImageCmd = (const apCLWriteImageCommand&)commandDetails;

            // Written Image:
            gtString writtenImageAsString;
            memoryObjectHandleToHTMLString(writeImageCmd.writtenImage(), writtenImageAsString, false, true);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandWrittenPrefix GD_STR_PropertiesQueueCommandImage, writtenImageAsString);

            // Is Blocking Write:
            apCLBoolParameter blockingWrite(writeImageCmd.isBlockingWrite());
            gtString isBlockingWriteAsString;
            blockingWrite.valueAsString(isBlockingWriteAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandBlockingWrite, isBlockingWriteAsString);

            // Written Origin:
            gtString writtenOriginAsString;
            writtenOriginAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DOffsetValueFormat, (unsigned long)writeImageCmd.writtenDataOriginX(), (unsigned long)writeImageCmd.writtenDataOriginY(), (unsigned long)writeImageCmd.writtenDataOriginZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandWrittenPrefix GD_STR_PropertiesQueueCommandOrigin, writtenOriginAsString);

            // Written Region:
            gtString writtenRegionAsString;
            writtenRegionAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DSizeValueFormat, (unsigned long)writeImageCmd.writtenDataRegionX(), (unsigned long)writeImageCmd.writtenDataRegionY(), (unsigned long)writeImageCmd.writtenDataRegionZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandWrittenPrefix GD_STR_PropertiesQueueCommandRegion, writtenRegionAsString);

            // Row Pitch:
            gtString rowPitchAsString;
            rowPitchAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)writeImageCmd.hostDataRowPitch());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandRowPitch, rowPitchAsString);

            // Slice Pitch:
            gtString slicePitchAsString;
            slicePitchAsString.appendFormattedString(GD_STR_PropertiesQueueCommand1DSizeValueFormat GD_STR_PropertiesQueueCommandBytesValueSuffix, (unsigned long)writeImageCmd.hostDataSlicePitch());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandSlicePitch, slicePitchAsString);

            // Host Data Pointer:
            gtString hostDataPtrAsString;
            gdUserApplicationAddressToDisplayString(writeImageCmd.hostDataPointer(), hostDataPtrAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandHostDataPointer, hostDataPtrAsString);
        }
        break;

        case OS_TOBJ_ID_CL_FILL_IMAGE_COMMAND:
        {
            // Cast into the command type:
            const apCLFillImageCommand& fillImageCmd = (const apCLFillImageCommand&)commandDetails;

            // Written Image:
            gtString fillImageAsString;
            memoryObjectHandleToHTMLString(fillImageCmd.filledImage(), fillImageAsString, false, true);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandFillPrefix GD_STR_PropertiesQueueCommandImage, fillImageAsString);

            // Filled Region:
            gtString filledColorAsString;
            gdUserApplicationAddressToDisplayString(fillImageCmd.fillColor(), filledColorAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandFillPrefix GD_STR_PropertiesQueueCommandColor, filledColorAsString);

            // Filled Origin:
            gtString filledOriginAsString;
            filledOriginAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DOffsetValueFormat, (unsigned long)fillImageCmd.filledOriginX(), (unsigned long)fillImageCmd.filledOriginY(), (unsigned long)fillImageCmd.filledOriginZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandFillPrefix GD_STR_PropertiesQueueCommandOrigin, filledOriginAsString);

            // Filled Region:
            gtString filledRegionAsString;
            filledRegionAsString.appendFormattedString(GD_STR_PropertiesQueueCommand3DSizeValueFormat, (unsigned long)fillImageCmd.filledRegionX(), (unsigned long)fillImageCmd.filledRegionY(), (unsigned long)fillImageCmd.filledRegionZ());
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandFillPrefix GD_STR_PropertiesQueueCommandRegion, filledRegionAsString);

        }
        break;

        case OS_TOBJ_ID_CL_QUEUE_IDLE_TIME:
        {
            // We should not get here with idles:
            GT_ASSERT(commandType != OS_TOBJ_ID_CL_QUEUE_IDLE_TIME);
        }
        break;

        default:
        {
            // Unknown command type
            GT_ASSERT(false);
        }
        break;
    }

    // Add common parameters
    addCommandEventsListAndEventParametersToHtmlContent(htmlContent, commandDetails);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::addCommandEventsListAndEventParametersToHtmlContent
// Description: Adds the "event_wait_list" and "event" parameters, which are
//              common to most command types, to the html content for a command
// Author:      Uri Shomroni
// Date:        4/3/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::addCommandEventsListAndEventParametersToHtmlContent(afHTMLContent& htmlContent, const apCLEnqueuedCommand& commandDetails)
{
    // Add the event wait list:
    if (commandDetails.eventWaitListParameterAvailable())
    {
        // Convert each event to a name and link:
        gtString eventWaitListAsString;
        const gtVector<oaCLEventHandle>& commandEventWaitList = commandDetails.eventWaitList();
        int numberOfEventsInWaitList = (int)commandEventWaitList.size();

        for (int i = 0; i < numberOfEventsInWaitList; i++)
        {
            if (i > 0)
            {
                eventWaitListAsString.append(L", ");
            }

            // Get the current event handle:
            oaCLEventHandle currentEventHandle = commandEventWaitList[i];
            gtString currentEventAsString;
            gdOpenCLHandleAsString(currentEventHandle, currentEventAsString, true, true, true);
            eventWaitListAsString.append(currentEventAsString);
        }

        if (eventWaitListAsString.isEmpty())
        {
            eventWaitListAsString = AF_STR_EmptyStr;
        }

        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandEventWaitList, eventWaitListAsString);
    }

    // Add the event:
    if (commandDetails.eventParameterAvailable())
    {
        // Translate the event handle (if acquired) to a string:
        gtString eventAsString;
        oaCLEventHandle eventHandle = commandDetails.userGeneratedEvent();

        if (eventHandle != OA_CL_NULL_HANDLE)
        {
            // Try to get the event number and queue:
            apCLObjectID eventAsCLObject;
            bool rcEveHand = gaGetOpenCLHandleObjectDetails((oaCLHandle)eventHandle, eventAsCLObject);
            GT_IF_WITH_ASSERT(rcEveHand)
            {
                // If the event still exists:
                if (eventAsCLObject._objectId > -1)
                {
                    // Show the handle as a link:
                    eventAsString.appendFormattedString(AF_STR_HtmlPropertiesLink3ParamStart, GD_STR_HtmlPropertiesCLEventLink, eventAsCLObject._objectId, eventAsCLObject._contextId, eventAsCLObject._ownerObjectId);
                    eventAsString.appendFormattedString(GD_STR_PropertiesQueueCommandEventName, eventAsCLObject._objectId + 1);
                    eventAsString.append(GD_STR_HtmlPropertiesLinkEnd);
                }
                else // eventAsCLObject._objectId <= -1
                {
                    // Add a link to the queue instead:
                    eventAsString.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesCLEventLink, eventAsCLObject._ownerObjectId, eventAsCLObject._contextId);
                    eventAsString.append(GD_STR_PropertiesQueueCommandEventReleasedName);
                    eventAsString.append(GD_STR_HtmlPropertiesLinkEnd);
                }

                GT_ASSERT(eventAsCLObject._objectType == OS_TOBJ_ID_CL_EVENT);
            }
            else // !rcEveHand
            {
                // Just show the handle as a pointer:
                gdUserApplicationAddressToDisplayString((osProcedureAddress64)eventHandle, eventAsString);
            }
        }
        else // eventHandle == OA_CL_NULL_HANDLE
        {
            eventAsString = AF_STR_None;
        }

        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueCommandEvent, eventAsString);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::memoryObjectsVectorToHTMLString
// Description: Converts the memory object handles vector to a list of object
//              names, with links where possible
// Author:      Uri Shomroni
// Date:        4/3/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::memoryObjectsVectorToHTMLString(const gtVector<oaCLMemHandle>& memHandles, gtString& memHandlesAsString, bool allowBuffers, bool allowTextures)
{
    int numberOfMemObjects = (int)memHandles.size();

    if (numberOfMemObjects > 0)
    {
        // Get each event's handle and use them to make a string with links:
        memHandlesAsString.makeEmpty();

        for (int i = 0; i < numberOfMemObjects; i++)
        {
            if (i > 0)
            {
                memHandlesAsString.append(L", ");
            }

            // Try to get the current object name and type:
            oaCLMemHandle currentMemHandle = memHandles[i];
            gtString currentMemHandleAsString;
            memoryObjectHandleToHTMLString(currentMemHandle, currentMemHandleAsString, allowBuffers, allowTextures);

            memHandlesAsString.append(currentMemHandleAsString);
        }
    }
    else // numberOfMemObjects == 0
    {
        memHandlesAsString = AF_STR_None;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::memoryObjectHandleToHTMLString
// Description: Translates a memory object handle to a string, with a link if
//              possible.
// Author:      Uri Shomroni
// Date:        4/3/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::memoryObjectHandleToHTMLString(oaCLMemHandle memHandle, gtString& memHandleAsString, bool allowBuffer, bool allowTexture)
{
    // Clear the output string:
    memHandleAsString.makeEmpty();

    // Get the object number and type:
    apCLObjectID memObjectAsCLObject;
    bool rcMOHand = gaGetOpenCLHandleObjectDetails((oaCLHandle)memHandle, memObjectAsCLObject);
    bool gotHandleAsString = false;
    GT_IF_WITH_ASSERT(rcMOHand)
    {
        // Add a link according to the type:
        osTransferableObjectType objType = memObjectAsCLObject._objectType;

        if (objType == OS_TOBJ_ID_CL_BUFFER)
        {
            // This is a buffer:
            gdOpenCLHandleAsString(memHandle, memHandleAsString, true, true, true);

            gotHandleAsString = true;
            GT_ASSERT(allowBuffer);
        }
        else if (objType == OS_TOBJ_ID_CL_IMAGE)
        {
            // This is a texture:
            gdOpenCLHandleAsString(memHandle, memHandleAsString, true, true, true);

            gotHandleAsString = true;
            GT_ASSERT(allowTexture);
        }
        else if (objType == OS_TOBJ_ID_CL_PIPE)
        {
            // This is a pipe:
            gdOpenCLHandleAsString(memHandle, memHandleAsString, true, true, true);

            gotHandleAsString = true;
            // GT_ASSERT(allowPipe);
        }
        else // objType != OS_TOBJ_ID_CL_BUFFER, OS_TOBJ_ID_CL_IMAGE
        {
            // Unexpected object type:
            GT_ASSERT(false);
        }
    }

    // If something went wrong:
    if (!gotHandleAsString)
    {
        // Just add the handle as a pointer:
        gdUserApplicationAddressToDisplayString((osProcedureAddress64)memHandle, memHandleAsString);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildThreadCreatedEventPropertiesString
// Description: Builds a thread created event properties string
// Arguments: const apThreadCreatedEvent& threadCreatedEvent
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildThreadCreatedEventPropertiesString(const apThreadCreatedEvent& threadCreatedEvent, afHTMLContent& htmlContent)
{
    // Report the process creation date and time:
    const osTime& creationTime = threadCreatedEvent.threadCreationTime();
    gtString threadCreationDate;
    gtString threadCreationTime;
    creationTime.dateAsString(threadCreationDate, osTime::WINDOWS_STYLE, osTime::LOCAL);
    creationTime.timeAsString(threadCreationTime, osTime::WINDOWS_STYLE, osTime::LOCAL);
    threadCreationDate.appendFormattedString(L" %ls", threadCreationTime.asCharArray());

    gtString threadId;
    threadId.appendFormattedString(L"%lu", threadCreatedEvent.threadOSId());

    // Handle LWP (Linux and Mac only):
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    {
        // Add the LWP ID
        threadId.appendFormattedString(GD_STR_ThreadEventsViewOsIdLWP, threadCreatedEvent.lwpOSId());
    }
#endif

    gtString modulePathReport = threadCreatedEvent.threadStartModuleFilePath().asString().asCharArray();
    validateValueAvailability(modulePathReport);

    gtString functionSourceCodeFile = threadCreatedEvent.threadStartFunctionName();
    validateValueAvailability(functionSourceCodeFile);

    gtString filePathReport = threadCreatedEvent.startFunctionSourceCodeFile().asString();
    validateValueAvailability(filePathReport);

    int functionLineNumber = threadCreatedEvent.startFunctionSourceCodeFileLineNum();
    gtString functionLineNumberReport;

    if (functionLineNumber < 1)
    {
        functionLineNumberReport.append(AF_STR_NotAvailable);
    }
    else
    {
        functionLineNumberReport.appendFormattedString(L"%d", functionLineNumber);
    }

    gtString threadStartAddressReport;
    gdUserApplicationAddressToDisplayString((osProcedureAddress64)(threadCreatedEvent.threadStartAddress()), threadStartAddressReport);

    // Build the HTML content:
    htmlContent.setTitle(GD_STR_ProcessEventsViewPropertiesTitleThreadCreated);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesThreadId, threadId);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ProcessEventsViewCreationTime, threadCreationDate);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_DLLName, modulePathReport);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_FunctionName, functionSourceCodeFile);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_FileName, filePathReport);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ThreadEventsViewLineNumber, functionLineNumberReport);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ThreadEventsViewStartAddress, threadStartAddressReport);
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildThreadTerminatedEventPropertiesString
// Description: Builds a thread terminated event properties string
// Arguments: const apThreadTerminatedEvent& threadTerminatedEvent
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildThreadTerminatedEventPropertiesString(const apThreadTerminatedEvent& threadTerminatedEvent, afHTMLContent& htmlContent)
{

    // Report the process creation date and time:
    const osTime& terminationTime = threadTerminatedEvent.threadTerminationTime();
    gtString threadTerminationDate;
    gtString threadTerminationTime;
    terminationTime.dateAsString(threadTerminationDate, osTime::WINDOWS_STYLE, osTime::LOCAL);
    terminationTime.timeAsString(threadTerminationTime, osTime::WINDOWS_STYLE, osTime::LOCAL);
    threadTerminationDate.appendFormattedString(L" %ls", threadTerminationTime.asCharArray());

    gtString threadId;
    threadId.appendFormattedString(L"%lu", threadTerminatedEvent.threadOSId());

    gtString terminationCodeReport;
    terminationCodeReport.appendFormattedString(L"%d", threadTerminatedEvent.threadExitCode());

    // Build the HTML content:
    htmlContent.setTitle(GD_STR_ProcessEventsViewPropertiesTitleThreadTerminated);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesThreadId, threadId);

    // Add the deleted thread properties to the table;
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ThreadEventsViewTerminationTime, threadTerminationDate);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ThreadEventsViewTerminationCode, terminationCodeReport);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildProcessCreationFailureEventPropertiesString
// Description: Build a process creation failure event properties string
// Arguments: const apDebugProjectSettings& processStartedData
//            const apDebuggedProcessCreationFailureEvent& processCreationFailureEvent
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        11/2/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildProcessCreationFailureEventPropertiesString(const apDebugProjectSettings& processStartedData, const apDebuggedProcessCreationFailureEvent& processCreationFailureEvent, afHTMLContent& htmlContent)
{
    // Build the HTML content:
    htmlContent.setTitle(GD_STR_ProcessEventsViewProcessCreationFailed);

    // Build the process name:
    gtString processName, fileExtension;
    processStartedData.executablePath().getFileName(processName);
    processStartedData.executablePath().getFileExtension(fileExtension);

    if (!fileExtension.isEmpty())
    {
        processName.appendFormattedString(L".%ls", fileExtension.asCharArray());
    }

    // Report the process creation failure date and time:
    osTime creationTime = processCreationFailureEvent.processCreationTime();
    gtString processCreationDate;
    gtString processCreationTime;
    creationTime.dateAsString(processCreationDate, osTime::WINDOWS_STYLE, osTime::LOCAL);
    creationTime.timeAsString(processCreationTime, osTime::WINDOWS_STYLE, osTime::LOCAL);
    processCreationDate.appendFormattedString(L" %ls", processCreationTime.asCharArray());

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, processName);

    // Add the process creation failure error to the table;
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewProcessCreationError, processCreationFailureEvent.processCreationError());

    // Add the process command line properties to the table;
    const gtString& creationCommandLine = processCreationFailureEvent.createdProcessCommandLine();

    if (!creationCommandLine.isEmpty())
    {
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewProcessCommandLine, creationCommandLine);
    }

    // Add the working directory properties to the table;
    const gtString& creationWorkingDirectory = processCreationFailureEvent.createdProcessWorkDir();

    if (!creationWorkingDirectory.isEmpty())
    {
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewProcessWorkingFolder, creationWorkingDirectory);
    }

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ProcessEventsViewCreationTime, processCreationDate);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildExceptionPropertiesString
// Description: Builds an exception HTML properties string
// Arguments: const apExceptionEvent& exceptionEvent
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildExceptionPropertiesString(const apExceptionEvent& exceptionEvent, afHTMLContent& htmlContent)
{
    // Get the exception address (In debugged process address space):
    osProcedureAddress64 exceptionAddress = (osProcedureAddress64)exceptionEvent.exceptionAddress();
    gtString exceptionAddressString;
    gdUserApplicationAddressToDisplayString(exceptionAddress, exceptionAddressString);

    // Get the exception reason as string:
    osExceptionReason exceptionReason = exceptionEvent.exceptionReason();
    gtString exceptionReasonString;
    osExceptionReasonToString(exceptionReason, exceptionReasonString);

    // Get the exception reason detailed explanation as string:
    gtString exceptionReasonExplanationString;
    osExceptionReasonToExplanationString(exceptionReason, exceptionReasonExplanationString);

    // Build the HTML content:
    // The terminology is:
    // - Windows - exception
    // - Linux / Mac - signal
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    htmlContent.setTitle(GD_STR_ProcessEventsViewPropertiesTitleException);
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    htmlContent.setTitle(GD_STR_ProcessEventsViewPropertiesTitleSignal);
#else
#error Unknown build configuration!
#endif

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_ProcessEventsViewPropertiesTitleEventProperties);
    // Add the exception properties to the table;
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ThreadEventsViewReason, exceptionReasonString);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ThreadEventsViewAddress, exceptionAddressString);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ThreadEventsViewDetails, exceptionReasonExplanationString);
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOutputDebugStringEventString
// Description: Builds an output debug string event properties string
// Arguments: const apOutputDebugStringEvent& outputDebugStringEvent
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOutputDebugStringEventString(const apOutputDebugStringEvent& outputDebugStringEvent, afHTMLContent& htmlContent)
{
    // Get the outputted debug string:
    gtString debugString = outputDebugStringEvent.debugString();

    htmlContent.setTitle(GD_STR_ProcessEventsViewPropertiesTitleOutputDebugString);

    // Add the exception properties to the table:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, debugString);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildDebuggedProcessOutputStringEventEventString
// Description: Builds a debugged process output string event properties string
// Arguments: const apDebuggedProcessOutputStringEvent& debuggedProcessOutputEvent
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildDebuggedProcessOutputStringEventString(const apDebuggedProcessOutputStringEvent& debuggedProcessOutputEvent, afHTMLContent& htmlContent)
{

    htmlContent.setTitle(GD_STR_ProcessEventsViewDebuggedProcessOutputString);

    // Add the exception properties to the table;
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, debuggedProcessOutputEvent.debuggedProcessOutputString());
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildGDBOutputStringEventPropertiesString
// Description: Builds a GDB output string event properties string
// Arguments: const apGDBOutputStringEvent& outputGDBStringEvent
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildGDBOutputStringEventPropertiesString(const apGDBOutputStringEvent& outputGDBStringEvent, afHTMLContent& htmlContent)
{

    // Build the HTML content:
    htmlContent.setTitle(GD_STR_ProcessEventsViewPropertiesTitleGDBOutputString);

    // Add the exception properties to the table;
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, outputGDBStringEvent.gdbOutputString());
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildGDBErrorPropertiesString
// Description: Builds a GDB error properties string
// Arguments: const apGDBErrorEvent& gdbErrorEvent
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildGDBErrorPropertiesString(const apGDBErrorEvent& gdbErrorEvent, afHTMLContent& htmlContent)
{

    // Build the HTML content:
    htmlContent.setTitle(GD_STR_ProcessEventsViewPropertiesTitleGDBError);

    // Add the exception properties to the table;
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, gdbErrorEvent.gdbErrorString());
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildErrorEventPropertiesString
// Description: Builds an error event properties string
// Arguments: const apDebuggedProcessDetectedErrorEvent& errorEvent
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildErrorEventPropertiesString(const apDebuggedProcessDetectedErrorEvent& errorEvent, afHTMLContent& htmlContent)
{
    // Get the detected error parameters:
    const apDetectedErrorParameters& detectedErrorParameters = errorEvent.detectedErrorParameters();

    // The OpenGL function name:
    apMonitoredFunctionId assosiatedErrorFunctionCode = detectedErrorParameters._detectedErrorAssociatedFunction;
    gtString assosiatedErrorFunctionName;
    gaGetMonitoredFunctionName(assosiatedErrorFunctionCode, assosiatedErrorFunctionName);

    // Get the error description:
    const gtString& errorDescription = detectedErrorParameters._detectedErrorDescription;

    // The OpenGL error code:
    apErrorCode errorCode = (apErrorCode)detectedErrorParameters._detectedErrorCode;
    gtString errorString;
    apDetectedErrorCodeToString(errorCode, errorString);

    // Build the HTML content:
    htmlContent.setTitle(GD_STR_ProcessEventsViewPropertiesTitleDetectedError);

    // Add the error properties to the table;
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReason, GD_STR_ProcessEventsViewPropertiesTitleDetectedError);

    if (!assosiatedErrorFunctionName.isEmpty())
    {
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesOccurredIn, assosiatedErrorFunctionName);
    }

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewErrorCode, errorString);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewErrorDescription, errorDescription);

    if (errorEvent.wasGeneratedByBreak())
    {
        // Break-only parameters:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewErrorNoteTitle, GD_STR_ProcessEventsViewErrorNoteTurnOff);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_ProcessEventsViewStepHitAfterSubtitle);
    }
    else // !wasGeneratedByBreak()
    {
        // Non-break-only parameters:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewErrorNoteTitle, GD_STR_ProcessEventsViewErrorNoteTurnOn);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildDLLLoadPropertiesString
// Description: Build DLL load HTML string
// Arguments: const apModuleLoadedEvent& moduleLoadEvent
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildDLLLoadPropertiesString(const apModuleLoadedEvent& moduleLoadEvent, afHTMLContent& htmlContent)
{
    gtString title = GD_STR_ProcessEventsViewPropertiesTitleDllLoaded;

    // Build the HTML content:
    htmlContent.setTitle(title);

    // Build the DLL name:
    gtString dllName;
    osFilePath filePath(moduleLoadEvent.modulePath());
    gtString fileName, fileExtension;
    filePath.getFileName(fileName);
    filePath.getFileExtension(fileExtension);

    if (!fileExtension.isEmpty())
    {
        fileName.appendFormattedString(L".%ls", fileExtension.asCharArray());
    }

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_DLLName, fileName);

    // Get the loaded module path:
    gtString modulePath = moduleLoadEvent.modulePath();
    validateValueAvailability(modulePath);

    // Add the created thread properties to the table;
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_FilePath, modulePath);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildDLLUnloadPropertiesString
// Description: Build DLL unload HTML string
// Arguments: const apModuleLoadedEvent& moduleLoadEvent
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildDLLUnloadPropertiesString(const apModuleUnloadedEvent& moduleUnloadEvent, afHTMLContent& htmlContent)
{
    gtString title = GD_STR_ProcessEventsViewPropertiesTitleDllUnloaded;

    // Build the HTML content:
    htmlContent.setTitle(title);

    // Build the DLL name:
    gtString dllName, fileExtension;
    osFilePath filePath(moduleUnloadEvent.modulePath());
    filePath.getFileName(dllName);
    filePath.getFileExtension(fileExtension);

    if (!fileExtension.isEmpty())
    {
        dllName.appendFormattedString(L".%ls", fileExtension.asCharArray());
    }

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_DLLName, dllName);

    // Get the loaded module path:
    gtString modulePath = moduleUnloadEvent.modulePath();
    validateValueAvailability(modulePath);

    // Add the created thread properties to the table;
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_FileName, modulePath);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildMultipleItemPropertiesString
// Description: Builds multiple item selected string
// Arguments:   const gtString& title - the HTML properties title
//              const gtString& itemName - the item type as string
//              afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        16/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildMultipleItemPropertiesString(const gtString& title, const gtString& itemName, afHTMLContent& htmlContent)
{
    // Build the HTML content:
    htmlContent.setTitle(title);

    // Add the "select single item string"
    gtString selectSingleItemString;
    selectSingleItemString.appendFormattedString(GD_STR_PropertiesMultipleItemsMessage, itemName.asCharArray());
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, selectSingleItemString);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildCallStackPropertiesString
// Description: Build call stack properties string
// Arguments: const gtString& functionName
//            const osFilePath& sourceCodeFilePath
//            const osFilePath& sourceCodeModulePath
//            int sourceCodeFileLineNumber
//            osInstructionPointer& functionStartAddress
//            osInstructionPointer instructionCounterAddress
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        16/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildCallStackPropertiesString(const gtString& functionName, const osFilePath& sourceCodeFilePath, const osFilePath& sourceCodeModulePath, int sourceCodeFileLineNumber,
                                                      osInstructionPointer& functionStartAddress, osInstructionPointer instructionCounterAddress, afHTMLContent& htmlContent)
{
    // Build function name:
    gtString functionNameStr = functionName;
    validateValueAvailability(functionNameStr);

    // Build source code file path string:
    gtString sourceCodeFilePathString = sourceCodeFilePath.asString();
    validateValueAvailability(sourceCodeFilePathString);

    // Build source module path string:
    gtString sourceCodeModulePathString = sourceCodeModulePath.asString().asCharArray();
    validateValueAvailability(sourceCodeModulePathString);

    // Build line number string:
    gtString sourceCodeFileLineNumberString;

    if (sourceCodeFileLineNumber < 1)
    {
        sourceCodeFileLineNumberString = AF_STR_NotAvailable;
    }
    else
    {
        sourceCodeFileLineNumberString.appendFormattedString(L"%d", sourceCodeFileLineNumber);
    }

    // Build function start address string:
    gtString functionStartAddressString;

    if (functionStartAddress != 0)
    {
        gdUserApplicationAddressToDisplayString((osProcedureAddress64)functionStartAddress, functionStartAddressString);
    }
    else
    {
        functionStartAddressString = AF_STR_NotAvailable;
    }

    // Build instruction counter address string:
    gtString instructionCounterAddressString;

    if (instructionCounterAddress != 0)
    {
        gdUserApplicationAddressToDisplayString((osProcedureAddress64)instructionCounterAddress, instructionCounterAddressString);
    }
    else
    {
        instructionCounterAddressString = AF_STR_NotAvailable;
    }

    // Build the HTML content:
    htmlContent.setTitle(GD_STR_CallsStackPropertiesTitle);

    // Add function name line:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_FunctionName, functionNameStr);

    // Add source file path line:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_FilePath, sourceCodeFilePathString);

    // Add line number:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_CallsStackPropertiesLineNumber, sourceCodeFileLineNumberString);

    // Add module name:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_CallsStackPropertiesModuleName, sourceCodeModulePathString);

    // Add instruction counter address:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_CallsStackPropertiesInstructionCounterAddress, instructionCounterAddressString);

    // Add function start address:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_CallsStackPropertiesFunctionStartAddress, functionStartAddressString);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildSimpleHTMLMessage
// Description: Builds a simple HTML message (only title and content)
// Arguments: const gtString& title
//            const gtString& content
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        16/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildSimpleHTMLMessage(const gtString& title, const gtString& content, gtString& propertiesHTMLMessage, bool shouldCreateHTMLDesignTable)
{
    if (shouldCreateHTMLDesignTable)
    {
        afHTMLContent htmlContent;
        // Build the HTML content:
        htmlContent.setTitle(title);

        // Add the content:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, content);

    }
    else
    {
        // Initialize HTML header string:
        afHTMLContent::buildHTMLHeader(propertiesHTMLMessage);

        // Add title:
        if (!title.isEmpty())
        {
            propertiesHTMLMessage.append(AF_STR_HtmlBoldTagStart);
            propertiesHTMLMessage.append(title);
            propertiesHTMLMessage.append(AF_STR_HtmlBoldTagEnd);
            propertiesHTMLMessage.append(AF_STR_HtmlEmptyLine);
        }

        // Add content:
        if (!content.isEmpty())
        {
            propertiesHTMLMessage.append(L"<font size=+2>");
            propertiesHTMLMessage.append(AF_STR_HtmlBoldTagStart);
            propertiesHTMLMessage.append(content);
            propertiesHTMLMessage.append(AF_STR_HtmlBoldTagEnd);
            propertiesHTMLMessage.append(AF_STR_HtmlFontTagEnd);
        }

        // Initialize HTML header string:
        afHTMLContent::endHTML(propertiesHTMLMessage);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildBreakpointPropertiesString
// Description: Builds a breakpoint properties string
// Arguments: const gtString& funcName
//            const gtString& funcArgs
//            const apBreakpointHitEvent& breakpointEvent
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildBreakpointPropertiesString(const gtString& funcName, const gtString& funcArgs, const apBreakpointHitEvent& breakpointEvent, afHTMLContent& htmlContent)
{
    // Build the function name with arguments string:
    gtString functionNameWithArgsStr = funcName;
    functionNameWithArgsStr.append(funcArgs);

    // Get the break reason:
    apBreakReason breakReason = breakpointEvent.breakReason();

    htmlContent.setTitle(GD_STR_ProcessEventsViewPropertiesTitleBreakHit);

    // Insert the debug string into this view list:
    gtString breakpointDescription = GD_STR_ProcessEventsViewStepHitBeforeSubtitle;

    if (breakReason == AP_REDUNDANT_STATE_CHANGE_BREAKPOINT_HIT)
    {
        breakpointDescription = GD_STR_ProcessEventsViewStepHitAfterSubtitle;
    }

    // Build the breakpoint content according to the breakpoint reason:
    switch (breakReason)
    {
        case AP_MONITORED_FUNCTION_BREAKPOINT_HIT:
        {
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReason, GD_STR_ProcessEventsViewPropertiesTitleBreakpoint);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakOn, functionNameWithArgsStr);
            break;
        }

        case AP_KERNEL_SOURCE_CODE_BREAKPOINT_HIT:
        {
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReason, GD_STR_ProcessEventsViewPropertiesTitleBreakpoint);
            // TO_DO: add breakpoint details
            // htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakOn, functionNameWithArgsStr);
            break;
        }

        case AP_KERNEL_FUNCTION_NAME_BREAKPOINT_HIT:
        {
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReason, GD_STR_ProcessEventsViewPropertiesTitleBreakpoint);
            // TO_DO: add breakpoint details
            // htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakOn, functionNameWithArgsStr);
            break;
        }

        case AP_NEXT_MONITORED_FUNCTION_BREAKPOINT_HIT:
        {
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReason, GD_STR_ProcessEventsViewPropertiesTitleStepHit);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakOn, functionNameWithArgsStr);
            break;
        }

        case AP_DRAW_MONITORED_FUNCTION_BREAKPOINT_HIT:
        {
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReason, GD_STR_ProcessEventsViewPropertiesTitleDrawStepHit);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakOn, functionNameWithArgsStr);
            break;
        }

        case AP_FRAME_BREAKPOINT_HIT:
        {
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReason, GD_STR_ProcessEventsViewPropertiesTitleFrameStepHit);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakOn, functionNameWithArgsStr);
            break;
        }

        case AP_STEP_IN_BREAKPOINT_HIT:
        {
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReason, GD_STR_ProcessEventsViewPropertiesTitleStepHit);
            // TO_DO: add step details
            // htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakOn, functionNameWithArgsStr);
            break;
        }

        case AP_STEP_OVER_BREAKPOINT_HIT:
        {
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReason, GD_STR_ProcessEventsViewPropertiesTitleStepHit);
            // TO_DO: add step details
            // htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakOn, functionNameWithArgsStr);
            break;
        }

        case AP_STEP_OUT_BREAKPOINT_HIT:
        {
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReason, GD_STR_ProcessEventsViewPropertiesTitleStepHit);
            // TO_DO: add step details
            // htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakOn, functionNameWithArgsStr);
            break;
        }

        case AP_BREAK_COMMAND_HIT:
        {
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReason, GD_STR_ProcessEventsViewPropertiesTitlePause);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakOn, functionNameWithArgsStr);
            break;
        }

        case AP_OPENGL_ERROR_BREAKPOINT_HIT:
        {
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReason, GD_STR_ProcessEventsViewPropertiesTitleOpenGLErrorHit);

            // Get the OpenGL error as string:
            GLenum openGLError = breakpointEvent.openGLErrorCode();
            gtString errorAsString;
            gdOpenGLErrorToString(openGLError, errorAsString);

            // Get the breakpoint description:
            gtString errorDescription;
            gdOpenGLErrorToDescriptionString(openGLError, errorDescription);

            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakOn, functionNameWithArgsStr);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewErrorCode, errorAsString);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewErrorDescription, errorDescription);
            breakpointDescription = GD_STR_ProcessEventsViewStepHitBeforeSubtitle;
            break;
        }

        case AP_REDUNDANT_STATE_CHANGE_BREAKPOINT_HIT:
        {
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReason, GD_STR_ProcessEventsViewPropertiesTitleRedundantStateChangeHit);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakOn, functionNameWithArgsStr);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewRedundantStateChangeHitDescription);
            break;
        }

        case AP_DEPRECATED_FUNCTION_BREAKPOINT_HIT:
        {
            // TO_DO: deprecation model: Add specific explanation for the function call
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReason, GD_STR_ProcessEventsViewPropertiesTitleDeprecatedFunctionHit);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakOn, functionNameWithArgsStr);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewDeprecatedFunctionHitDescription);
            break;
        }

        case AP_DETECTED_ERROR_BREAKPOINT_HIT:
        {
            // TO_DO: deprecation model: Add specific explanation for the function call
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReason, GD_STR_ProcessEventsViewPropertiesTitleDeprecatedFunctionHit);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakOn, functionNameWithArgsStr);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewDetectedErrorFunctionHitDescription);
            break;
        }

        case AP_SOFTWARE_FALLBACK_BREAKPOINT_HIT:
        {
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReason, GD_STR_ProcessEventsViewPropertiesTitleSoftwareFallbackHit);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakOn, functionNameWithArgsStr);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewSoftwareFallBackHitDescription);
            break;
        }

        case AP_HOST_BREAKPOINT_HIT:
        {
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReason, GD_STR_ProcessEventsViewPropertiesTitleBreakpoint);
            // TO_DO: add breakpoint details
            // htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakOn, functionNameWithArgsStr);
            break;
        }

        case AP_FOREIGN_BREAK_HIT:
        {
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReason, GD_STR_ProcessEventsViewPropertiesTitleForeignBreakHit);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewForeignBreakHitDescription);
            break;
        }

        default:
            GT_ASSERT_EX(false, L"unrecognized breakpoint");
            break;
    }

    // Add the breakpoint before/after execution description:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, breakpointDescription);
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildBreakpointPropertiesString
// Description: Build the breakpoint HTML string
// Arguments:   gdBreakpointsItemData* pBreakpointItemData
//              afHTMLContent& htmlContent
// Author:      Sigal Algranaty
// Date:        20/3/2012
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildBreakpointPropertiesString(gdBreakpointsItemData* pBreakpointItemData, afHTMLContent& htmlContent)
{
    // Sanity check
    GT_IF_WITH_ASSERT(pBreakpointItemData != NULL)
    {
        // Find the title according to the breakpoint type:
        gtString title;

        switch (pBreakpointItemData->_breakpointType)
        {
            case OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT:
            {
                title = GD_STR_PropertiesBreakpointAPITitle;
            }
            break;

            case OS_TOBJ_ID_SOURCE_CODE_BREAKPOINT:
            {
                title = GD_STR_PropertiesBreakpointSourceCodeTitle;
            }
            break;

            case OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT:
            {
                title = GD_STR_PropertiesBreakpointKernelFunctionTitle;
            }
            break;

            case OS_TOBJ_ID_GENERIC_BREAKPOINT:
            {
                title = GD_STR_PropertiesBreakpointGenericTitle;
            }
            break;

            default:
                break;
        }

        // Define an HTML content:
        htmlContent.setTitle(title);

        // Build the breakpoint content according to the breakpoint reason:
        switch (pBreakpointItemData->_breakpointType)
        {
            case OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT:
            {
                gtString breakpointFunctionName;
                (void) gaGetMonitoredFunctionName(pBreakpointItemData->_monitoredFunctionId, breakpointFunctionName);
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakpointAPIFunctionName, breakpointFunctionName);
                break;
            }

            case OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT:
            case OS_TOBJ_ID_SOURCE_CODE_BREAKPOINT:
            {
                gtString lineNumberStr;
                lineNumberStr.appendFormattedString(L"%d", pBreakpointItemData->_sourceCodeLine);
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakpointSourceCodeFilePath, pBreakpointItemData->_sourceCodeFilePath.asString());
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakpointSourceCodeLineNumber, lineNumberStr);
                break;
            }

            case OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT:
            {
                gtString lineNumberStr;
                lineNumberStr.appendFormattedString(L"%d", pBreakpointItemData->_sourceCodeLine);
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakpointSourceCodeFilePath, pBreakpointItemData->_sourceCodeFilePath.asString());
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakpointSourceCodeLineNumber, lineNumberStr);
                break;
            }

            case OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT:
            {
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakpointKernelFunctionName, pBreakpointItemData->_kernelFunctionName);
                break;
            }

            case OS_TOBJ_ID_GENERIC_BREAKPOINT:
            {
                gtString genericFunctionName;
                bool rc = apGenericBreakpoint::breakpointTypeToString(pBreakpointItemData->_genericBreakpointType, genericFunctionName);
                GT_IF_WITH_ASSERT(rc)
                {
                    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakpointGenericName, genericFunctionName);
                }
                break;
            }

            default:
                GT_ASSERT_EX(false, L"unrecognized breakpoint");
                break;
        }

        gtString isEnabledStr = pBreakpointItemData->_isEnabled ? AF_STR_Yes : AF_STR_No;
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakpointIsEnabled, isEnabledStr);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildWatchVariablePropertiesString
// Description: Build a watch expression HTML string
// Arguments:   const gtString& variableName
//              const gtString& variableValue
//              const gtString& variableType
//              afHTMLContent& htmlContent
// Author:      Sigal Algranaty
// Date:        21/3/2012
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildWatchVariablePropertiesString(const gtString& variableName, const gtString& variableValue, const gtString& variableType, afHTMLContent& htmlContent)
{
    htmlContent.setTitle(GD_STR_PropertiesWatchTitle);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesWatchName, variableName);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesWatchType, variableType);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesWatchValue, variableValue);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildLocalVariablePropertiesString
// Description: Build a local variable HTML string
// Arguments:   const gtString& variableName - the name of the variable
//              const gtVector<gtString&> variableSubNames - the names of the first
//              level children
//              const gtVector<gtString&> variableValues - the values of the first
//              level children
//              afHTMLContent& htmlContent
// Author:      Sigal Algranaty
// Date:        21/3/2012
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildLocalVariablePropertiesString(const gtString& variableName, const gtString& variableType, const gtVector<gtString>& variableSubNames,
                                                          const gtVector<gtString>& variableValues, afHTMLContent& htmlContent)
{
    htmlContent.setTitle(GD_STR_PropertiesLocalsTitle);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesWatchName, variableName);

    // Sanity check
    GT_IF_WITH_ASSERT(variableValues.size() > 0)
    {
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesWatchType, variableType);

        if (variableSubNames.size() > 0)
        {
            // Sanity check:
            GT_IF_WITH_ASSERT(variableValues.size() == variableSubNames.size())
            {
                for (unsigned int i = 0; i < variableSubNames.size(); i++)
                {
                    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, variableValues[i], variableSubNames[i]);
                }
            }
        }
        else
        {
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesWatchValue, variableValues[0]);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildAPIConnectionEstablishedEventProperties
// Description: Builds an API connection established properties string
// Arguments: apiConnectionEstablishedEvent - Contains the api connection event details.
//            propertiesHTMLMessage - Will get the properties HTML string
// Author:      Yaki Tebeka
// Date:        24/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildAPIConnectionEstablishedEventProperties(const apApiConnectionEstablishedEvent& apiConnectionEstablishedEvent, afHTMLContent& htmlContent)
{
    // Build the HTML content:
    gtString title = GD_STR_ProcessEventsViewPropertiesTitleAPIConnectionEstablished;
    htmlContent.setTitle(title);

    // Get the established API connection type:
    apAPIConnectionType establishedConnectionType = apiConnectionEstablishedEvent.establishedConnectionType();
    gtString establishedConnectionAsStr;
    apAPIConnectionTypeToString(establishedConnectionType, establishedConnectionAsStr);

    // Add the API connection properties to the table;
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewAPIName, establishedConnectionAsStr);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildAPIConnectionEndedEventProperties
// Description: Builds an API connecion established properties string
// Arguments: apiConnectionEndedEvent - Contains the api connection event details.
//            propertiesHTMLMessage - Will get the properties HTML string
// Author:      Uri Shomroni
// Date:        13/1/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildAPIConnectionEndedEventProperties(const apApiConnectionEndedEvent& apiConnectionEndedEvent, afHTMLContent& htmlContent)
{
    // Build the HTML content:
    gtString title = GD_STR_ProcessEventsViewPropertiesTitleAPIConnectionEnded;
    htmlContent.setTitle(title);

    // Get the ended API connection type:
    apAPIConnectionType endedConnectionType = apiConnectionEndedEvent.connectionType();
    gtString establishedConnectionAsStr;
    apAPIConnectionTypeToString(endedConnectionType, establishedConnectionAsStr);

    // Add the API connection properties to the table;
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewAPIName, establishedConnectionAsStr);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildRenderContextCreatedEventProperties
// Description: Builds a render context created event properties string
// Arguments: renderContextCreatedEvent - Contains the event details.
//            propertiesHTMLMessage - Will get the properties HTML string
// Author:      Yaki Tebeka
// Date:        27/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildRenderContextCreatedEventProperties(const apRenderContextCreatedEvent& renderContextCreatedEvent, afHTMLContent& htmlContent)
{
    // Build the HTML content:
    gtString title = GD_STR_ProcessEventsViewRenderContextWasCreated;
    htmlContent.setTitle(title);

    // Add the context id:
    gtString contextLinkStr;
    apContextID contextId(AP_OPENGL_CONTEXT, renderContextCreatedEvent.contextId());
    contextId.toString(contextLinkStr);
    contextLinkStr.prependFormattedString(AF_STR_HtmlPropertiesLink1ParamStart, GD_STR_HtmlPropertiesGLContextLink, renderContextCreatedEvent.contextId());
    contextLinkStr.append(GD_STR_HtmlPropertiesLinkEnd);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesContextName, contextLinkStr);

    // Add the creating thread id:
    osThreadId creatingThreadId = renderContextCreatedEvent.triggeringThreadId();
    gtString creatingThreadIdAsStr;
    osThreadIdAsString(creatingThreadId, creatingThreadIdAsStr);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewCreatingThreadId, creatingThreadIdAsStr);

    // Create the render context name:
    gtString renderContextNameString;
    renderContextNameString.appendFormattedString(GD_STR_PropertiesContextHeadline, contextId._contextId);
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildRenderContextDeletedEventProperties
// Description: Builds a render context deleted event properties string
// Arguments: renderContextDeletedEvent - Contains the event details.
//            propertiesHTMLMessage - Will get the properties HTML string
// Author:      Yaki Tebeka
// Date:        27/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildRenderContextDeletedEventProperties(const apRenderContextDeletedEvent& renderContextDeletedEvent, afHTMLContent& htmlContent)
{
    // Build the HTML content:
    gtString title = GD_STR_ProcessEventsViewRenderContextWasDeleted;
    htmlContent.setTitle(title);

    // Add the context id:
    gtString contextLinkStr;
    apContextID contextId(AP_OPENGL_CONTEXT, renderContextDeletedEvent.contextId());
    contextId.toString(contextLinkStr);
    contextLinkStr.prependFormattedString(AF_STR_HtmlPropertiesLink1ParamStart, GD_STR_HtmlPropertiesGLContextLink, renderContextDeletedEvent.contextId());
    contextLinkStr.append(GD_STR_HtmlPropertiesLinkEnd);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesContextName, contextLinkStr);

    // Add the deleting thread id:
    osThreadId deletingThreadId = renderContextDeletedEvent.triggeringThreadId();
    gtString deletingThreadIdAsStr;
    osThreadIdAsString(deletingThreadId, deletingThreadIdAsStr);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewDeletingThreadId, deletingThreadIdAsStr);

    // Create the render context name:
    gtString renderContextNameString;
    renderContextNameString.appendFormattedString(GD_STR_PropertiesContextHeadline, contextId._contextId);
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildComputeContextCreatedEventProperties
// Description: Builds a compute context created event properties string
// Arguments: renderContextDeletedEvent - Contains the event details.
//            propertiesHTMLMessage - Will get the properties HTML string
// Author:      Yaki Tebeka
// Date:        17/1/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildComputeContextCreatedEventProperties(const apComputeContextCreatedEvent& computeContextCreatedEvent, afHTMLContent& htmlContent)
{
    // Build the HTML content:
    gtString title = GD_STR_ProcessEventsViewComputeContextWasCreated;
    htmlContent.setTitle(title);

    // Add the context id:
    gtString contextLinkStr;
    apContextID contextId(AP_OPENCL_CONTEXT, computeContextCreatedEvent.contextId());
    contextId.toString(contextLinkStr);
    contextLinkStr.prependFormattedString(AF_STR_HtmlPropertiesLink1ParamStart, GD_STR_HtmlPropertiesCLContextLink, computeContextCreatedEvent.contextId());
    contextLinkStr.append(GD_STR_HtmlPropertiesLinkEnd);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesContextName, contextLinkStr);

    // Add the creating thread id:
    osThreadId creatingThreadId = computeContextCreatedEvent.triggeringThreadId();
    gtString creatingThreadIdAsStr;
    osThreadIdAsString(creatingThreadId, creatingThreadIdAsStr);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewCreatingThreadId, creatingThreadIdAsStr);

    // Create the context name:
    gtString computeContextNameString;
    computeContextNameString.appendFormattedString(GD_STR_PropertiesContextHeadline, contextId._contextId);
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildComputeContextDeletedEventProperties
// Description: Builds a compute context deleted event properties string
// Arguments: renderContextDeletedEvent - Contains the event details.
//            propertiesHTMLMessage - Will get the properties HTML string
// Author:      Yaki Tebeka
// Date:        17/1/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildComputeContextDeletedEventProperties(const apComputeContextDeletedEvent& computeContextDeletedEvent, afHTMLContent& htmlContent)
{
    // Build the HTML content:
    gtString title = GD_STR_ProcessEventsViewOpenCLQueueWasDeleted;
    htmlContent.setTitle(title);

    // Add the context id:
    gtString contextLinkStr;
    apContextID contextId(AP_OPENCL_CONTEXT, computeContextDeletedEvent.contextId());
    contextId.toString(contextLinkStr);
    contextLinkStr.prependFormattedString(AF_STR_HtmlPropertiesLink1ParamStart, GD_STR_HtmlPropertiesCLContextLink, computeContextDeletedEvent.contextId());
    contextLinkStr.append(GD_STR_HtmlPropertiesLinkEnd);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesContextName, contextLinkStr);

    // Add the deleting thread id:
    osThreadId deletingThreadId = computeContextDeletedEvent.triggeringThreadId();
    gtString deletingThreadIdAsStr;
    osThreadIdAsString(deletingThreadId, deletingThreadIdAsStr);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewDeletingThreadId, deletingThreadIdAsStr);

    // Create the render context name:
    gtString renderContextNameString;
    renderContextNameString.appendFormattedString(GD_STR_PropertiesContextHeadline, contextId._contextId);
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLQueueCreatedEventProperties
// Description: Builds an OpenCL queue created event properties string
// Arguments:   queueCreatedEvent - Contains the event details.
//              propertiesHTMLMessage - Will get the properties HTML string
// Author:      Sigal Algranaty
// Date:        6/4/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLQueueCreatedEventProperties(const apOpenCLQueueCreatedEvent& queueCreatedEvent, afHTMLContent& htmlContent)
{
    // Build the HTML content:
    gtString title = GD_STR_ProcessEventsViewOpenCLQueueWasCreated;
    htmlContent.setTitle(title);

    // Build the command queue name with link (to the command queues viewer):
    int queueIndex = queueCreatedEvent.queueID();
    gtString commandQueueName;
    commandQueueName.appendFormattedString(GD_STR_PropertiesCLCommandQueueName, queueIndex + 1);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueName, commandQueueName);
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLQueueDeletedEventProperties
// Description: Builds an OpenCL queue deleted event properties string
// Arguments:   queueDeletedEvent - Contains the event details.
//              propertiesHTMLMessage - Will get the properties HTML string
// Author:      Sigal Algranaty
// Date:        6/4/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLQueueDeletedEventProperties(const apOpenCLQueueDeletedEvent& queueDeletedEvent, afHTMLContent& htmlContent)
{
    // Build the HTML content:
    gtString title = GD_STR_ProcessEventsViewOpenCLQueueWasDeleted;
    htmlContent.setTitle(title);

    // Build the command queue name with link (to the command queues viewer):
    int queueIndex = queueDeletedEvent.queueID();
    gtString commandQueueName;
    commandQueueName.appendFormattedString(GD_STR_PropertiesCLCommandQueueName, queueIndex);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesQueueName, commandQueueName);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLProgramCreatedEventProperties
// Description: Builds an OpenCL program created event properties string
// Arguments:   programCreatedEvent - Contains the event details.
//              propertiesHTMLMessage - Will get the properties HTML string
// Author:      Uri Shomroni
// Date:        1/5/2011
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLProgramCreatedEventProperties(const apOpenCLProgramCreatedEvent& programCreatedEvent, afHTMLContent& htmlContent)
{
    // Build the HTML content:
    gtString title = GD_STR_ProcessEventsViewOpenCLProgramWasCreated;
    htmlContent.setTitle(title);

    // Build the program name with link (to the programs viewer):
    int programIndex = programCreatedEvent.programIndex();
    int contextId = programCreatedEvent.contextID();
    gtString programNameWithLink;
    programNameWithLink.prependFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesCLProgramLink, programIndex + 1, contextId);
    programNameWithLink.appendFormattedString(GD_STR_PropertiesCLProgramName, programIndex + 1);
    programNameWithLink.append(GD_STR_HtmlPropertiesLinkEnd);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesProgramName, programNameWithLink);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLProgramDeletedEventProperties
// Description: Builds an OpenCL program deleted event properties string
// Arguments:   programDeletedEvent - Contains the event details.
//              propertiesHTMLMessage - Will get the properties HTML string
// Author:      Uri Shomroni
// Date:        1/5/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLProgramDeletedEventProperties(const apOpenCLProgramDeletedEvent& programDeletedEvent, afHTMLContent& htmlContent)
{
    // Build the HTML content:
    gtString title = GD_STR_ProcessEventsViewOpenCLProgramWasDeleted;
    htmlContent.setTitle(title);

    // Build the program name with link (to the programs viewer):
    int programIndex = programDeletedEvent.programIndex();
    int contextId = programDeletedEvent.contextID();
    gtString programNameWithLink;
    programNameWithLink.prependFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesCLProgramLink, programIndex + 1, contextId);
    programNameWithLink.appendFormattedString(GD_STR_PropertiesCLProgramName, programIndex);
    programNameWithLink.append(GD_STR_HtmlPropertiesLinkEnd);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesProgramName, programNameWithLink);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLProgramBuildEventProperties
// Description: Builds an OpenCL program build event properties string
// Arguments:   const apOpenCLProgramBuildEvent& programBuildEvent
//              afHTMLContent& htmlContent
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        15/3/2012
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLProgramBuildEventProperties(const apOpenCLProgramBuildEvent& programBuildEvent, afHTMLContent& htmlContent)
{
    // Build the HTML content:
    gtString title = GD_STR_ProcessEventsViewOpenCLProgramBuildStarted;

    if (programBuildEvent.wasBuildEnded())
    {
        title = GD_STR_ProcessEventsViewOpenCLProgramBuildEnded;
    }

    htmlContent.setTitle(title);

    // Build the program name with link (to the programs viewer):
    int programIndex = programBuildEvent.programIndex();
    int contextId = programBuildEvent.contextID();
    gtString programNameWithLink;
    int programDisplayName = programIndex + 1;
    programNameWithLink.prependFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesCLProgramLink, programDisplayName, contextId);
    programNameWithLink.appendFormattedString(GD_STR_PropertiesCLProgramName, programDisplayName);
    programNameWithLink.append(GD_STR_HtmlPropertiesLinkEnd);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesProgramName, programNameWithLink);

    if (programBuildEvent.wasBuildEnded())
    {
        // Get the build data:
        const gtVector<apCLProgram::programBuildData>& devicesBuildData = programBuildEvent.devicesBuildData();
        int sizeOfBuildData = (int)devicesBuildData.size();

        // Iterate the devices and add the build log:
        for (int i = 0; i < sizeOfBuildData; i++)
        {
            // Get the current build data:
            const apCLProgram::programBuildData& currentBuildData = devicesBuildData[i];

            gtString buildStatusStr;

            if (currentBuildData._buildStatus == CL_BUILD_NONE)
            {
                buildStatusStr = GD_STR_PropertiesProgramBuildLogStatusNone;
            }
            else if (currentBuildData._buildStatus == CL_BUILD_ERROR)
            {
                buildStatusStr = GD_STR_PropertiesProgramBuildLogStatusError;
            }
            else if (currentBuildData._buildStatus == CL_BUILD_SUCCESS)
            {
                buildStatusStr = GD_STR_PropertiesProgramBuildLogStatusSuccess;
            }
            else if (currentBuildData._buildStatus == CL_BUILD_IN_PROGRESS)
            {
                buildStatusStr = GD_STR_PropertiesProgramBuildLogStatusInProgress;
            }

            if (currentBuildData._buildStatus != CL_BUILD_NONE)
            {
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesProgramBuildLogStatus, buildStatusStr);
            }

            if (!currentBuildData._buildLog.isEmpty())
            {
                // Add the current build log to the HTML:
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesProgramBuildLog, currentBuildData._buildLog);
            }
            else
            {
                // Add the current build log to the HTML:
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesProgramBuildLogSucceeded, currentBuildData._buildLog);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildTechnologyMonitorFailureEventProperties
// Description: Builds a technology monitor failure event properties page
// Author:      Uri Shomroni
// Date:        16/2/2014
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildTechnologyMonitorFailureEventProperties(const apTechnologyMonitorFailureEvent& monitorFailureEvent, afHTMLContent& htmlContent)
{
    // Add the header:
    htmlContent.setTitle(GD_STR_PropertiesTechnologyMonitorFailure);

    // Add the event details:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesTechnologyMonitorFailureReason, monitorFailureEvent.failInformation());
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildGLDebugOutputMessageEventProperties
// Description: Builds a debug output message event properties page
// Arguments:   const apGLDebugOutputMessageEvent& glDebugOutputMessageEvent
//              afHTMLContent& htmlContent
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        10/6/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildGLDebugOutputMessageEventProperties(const apGLDebugOutputMessageEvent& glDebugOutputMessageEvent, afHTMLContent& htmlContent)
{
    // Build the HTML content:
    htmlContent.setTitle(GD_STR_PropertiesGLDebugOutputReport);

    // Build the context link string:
    gtString contextLinkStr;
    apContextID contextID(AP_OPENGL_CONTEXT, glDebugOutputMessageEvent.contextID());
    contextID.toString(contextLinkStr);
    contextLinkStr.prependFormattedString(AF_STR_HtmlPropertiesLink1ParamStart, GD_STR_HtmlPropertiesGLContextLink, contextID._contextId);
    contextLinkStr.append(GD_STR_HtmlPropertiesLinkEnd);

    // Add the context link to the HTML:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesContextName, contextLinkStr);

    // Add the message details:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesMessageSource, glDebugOutputMessageEvent.debugOutputSource());
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesMessageType, glDebugOutputMessageEvent.debugOutputType());
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesMessageSeverity, glDebugOutputMessageEvent.debugOutputSeverity());
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesMessage, glDebugOutputMessageEvent.debugOutputMessageContent());

    // Add the message id:
    gtString messageIdStr;
    messageIdStr.appendFormattedString(L"%d", glDebugOutputMessageEvent.messageID());
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesMessageID, messageIdStr);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildCLErrorEventProperties
// Description: Builds an OpenCL error event properties string
// Arguments: const apOpenCLErrorEvent& clErrorEvent
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        21/2/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildCLErrorEventProperties(const gtString& funcName, const gtString& funcArgs, const apOpenCLErrorEvent& clErrorEvent, afHTMLContent& htmlContent)
{

    // Build the function name with arguments string:
    gtString functionNameWithArgsStr = funcName;
    functionNameWithArgsStr.append(funcArgs);

    // Create the HTML content:
    htmlContent.setTitle(GD_STR_ProcessEventsViewPropertiesTitleOpenCLErrorHit);

    // Get the OpenCL error as string:
    int openCLErrorCode = clErrorEvent.openCLErrorParameters()._openCLErrorCode;
    gtString errorAsString;
    gdOpenCLErrorToString(openCLErrorCode, errorAsString);

    // Get the breakpoint description:
    gtString errorDescription;
    gdOpenCLErrorToDescriptionString(openCLErrorCode, errorDescription);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesBreakOn, functionNameWithArgsStr);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewErrorCode, errorAsString);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ProcessEventsViewErrorDescription, errorDescription);
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildStateVariablePropertiesString
// Description: Builds a state variables properties string
// Arguments: const gtString& variableName
//            const gtString& variableValue
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildStateVariablePropertiesString(const gtString& variableName, const gtString& variableValue, afHTMLContent& htmlContent)
{
    if (variableName.isEmpty())
    {
        htmlContent.setTitle(GD_STR_StateVariablesPropertiesAddVariable);
    }
    else
    {
        htmlContent.setTitle(GD_STR_StateVariablesPropertiesTitle);

        // Build the variable value for HTML:
        gtString varValueStr = variableValue;
        varValueStr.replace(L"} {", L"}<br>{", true);

        // Add the state variable properties:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_Name, variableName);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_Value, varValueStr);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildStartRecordingWarningPropertiesString
// Description: Builds a start recording warning properties string
// Arguments: const gtString& logFilesDirectory
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildStartRecordingWarningPropertiesString(const gtString& logFilesDirectory, afHTMLContent& htmlContent)
{
    // Build the warning HTML content:
    htmlContent.setTitle(GD_STR_CallsHistoryToolbarRecordingWarningTitle);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_CallsHistoryToolbarRecordingWarningMessage);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_CallsHistoryToolbarLogFilesDirectoryTitle, logFilesDirectory);

}
// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenRecordingFilePropertiesString
// Description: Builds an open recording file properties string
// Arguments: const gtString& recordFilePath
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenRecordingFilePropertiesString(const gtString& recordFilePath, afHTMLContent& htmlContent)
{
    // Build the warning HTML content:
    htmlContent.setTitle(GD_STR_CallsHistoryToolbarRecordingWarningTitle);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_CallsHistoryToolbarOpeningRecordedFileTitle);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_CallsHistoryToolbarOpeningRecordedFileMessage);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_CallsHistoryToolbarLogFilePath, recordFilePath);
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildPerformanceWarningPropertiesString
// Description: Builds a performance warning for the execution toolbar
// Arguments: apExecutionMode executionMode
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildPerformanceWarningPropertiesString(apExecutionMode executionMode, afHTMLContent& htmlContent)
{
    switch (executionMode)
    {
        case AP_ANALYZE_MODE:
        {
            htmlContent.setTitle(GD_STR_ExecutionModeToolbarPropertiesViewTitleAnalyze);

            htmlContent.addHTMLItemWithColor(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesAnalyzeWarning, AF_STR_Empty, L"Red");
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesAnalyzeTextLine1);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesAnalyzeTextLine2);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_BOLD_LINE, GD_STR_PropertiesAnalyzeSummary);

            break;
        }

        case AP_DEBUGGING_MODE:
        {
            htmlContent.setTitle(GD_STR_ExecutionModeToolbarPropertiesViewTitleDebug);

            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesDebugText);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_BOLD_LINE, GD_STR_PropertiesDebugSummary);
            break;
        }

        case AP_PROFILING_MODE:
        {
            htmlContent.setTitle(GD_STR_ExecutionModeToolbarPropertiesViewTitleProfile);

            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesProfilingText);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_BOLD_LINE, GD_STR_PropertiesProfilingSummary);
            break;
        }

        default:
            break;
    }
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildProcessRunResumedMessage
// Description: Builds a process run resumed message
// Arguments: afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        31/12/2009
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildProcessRunResumedMessage(afHTMLContent& htmlContent)
{
    htmlContent.setTitle(AF_STR_PropertiesProcessRunning);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesViewViewsInformationComment);

}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildShaderCompilationHTMLStatus
// Description: Build a shader source code viewer compilation status
// Arguments: int numberOfCompileSucceeded
//            int numberOfCompileFailed
//            bool wasLinked
//            bool wasLinkSuccessful
//            bool wasValidated
//            int wasValidationSuccessful
//            gtString& compilationStatusStr
// Return Val: void
// Author:      Sigal Algranaty
// Date:        7/1/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildShaderCompilationHTMLStatus(int numberOfCompileSucceeded, int numberOfCompileFailed, bool wasLinked, bool wasLinkSuccessful, bool wasValidated, int wasValidationSuccessful, gtString& compilationSummaryStr)
{
    // Empty the output string:
    compilationSummaryStr.makeEmpty();

    // Build the compile status string:
    gtString compileStatusFormat;
    gtString compileStatusString;
    compileStatusFormat = (numberOfCompileFailed > 0) ? GD_STR_ShadersSourceCodeViewerCompileStatusForamtFailure : GD_STR_ShadersSourceCodeViewerCompileStatusForamtSuccess;
    compileStatusString.appendFormattedString(compileStatusFormat.asCharArray(), numberOfCompileSucceeded, numberOfCompileFailed);

    // Build the link status string:
    gtString linkStatusString = GD_STR_ShadersSourceCodeViewerLinkStatusPrefix;
    gtString statusPostfix = GD_STR_ShadersSourceCodeViewerStatusNotExecuted;

    if (wasLinked)
    {
        statusPostfix = (wasLinkSuccessful) ? GD_STR_ShadersSourceCodeViewerStatusSuccess : GD_STR_ShadersSourceCodeViewerStatusFailure;
    }

    linkStatusString.append(statusPostfix);

    // Build the validation status string:
    gtString validateStatusString = GD_STR_ShadersSourceCodeViewerValidateStatusPrefix;
    statusPostfix = GD_STR_ShadersSourceCodeViewerStatusNotExecuted;

    if (wasValidated)
    {
        statusPostfix = (wasValidationSuccessful) ? GD_STR_ShadersSourceCodeViewerStatusSuccess : GD_STR_ShadersSourceCodeViewerStatusFailure;
    }

    validateStatusString.append(statusPostfix);

    // Add compilation summary:
    compilationSummaryStr.append(AF_STR_HtmlEmptyLine);
    compilationSummaryStr.append(AF_STR_DoneLine);
    compilationSummaryStr.append(AF_STR_HtmlEmptyLine);

    compilationSummaryStr.append(AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_HtmlPropertiesNonbreakingSpace);
    compilationSummaryStr.append(compileStatusString);
    compilationSummaryStr.append(AF_STR_HtmlEmptyLine);
    compilationSummaryStr.append(AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_HtmlPropertiesNonbreakingSpace);
    compilationSummaryStr.append(linkStatusString);
    compilationSummaryStr.append(AF_STR_HtmlEmptyLine);
    compilationSummaryStr.append(AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_HtmlPropertiesNonbreakingSpace);
    compilationSummaryStr.append(validateStatusString);

    afHTMLContent::endHTML(compilationSummaryStr);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLProgramBuildHTMLStatus
// Description: Summarizes the build results of an OpenCL program
// Author:      Uri Shomroni
// Date:        19/1/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLProgramBuildHTMLStatus(const afApplicationTreeItemData& compiledObjectData, bool wasBuildSuccessful, const apCLProgram& afterBuildProgramDetails, gtString& buildSummaryString)
{
    buildSummaryString.makeEmpty();

    // Get the program's devices:
    const gtVector<int>& programDevices = afterBuildProgramDetails.devices();
    const gtVector<apCLProgram::programBuildData>& programDevicesBuildData = afterBuildProgramDetails.devicesBuildData();
    int numberOfDevices = (int)programDevices.size();

    // The number should be equal, but let's avoid a vector superscript access violation:
    int numberOfDevicesBuildData = (int)programDevicesBuildData.size();

    if (numberOfDevicesBuildData < numberOfDevices)
    {
        GT_ASSERT(numberOfDevicesBuildData == numberOfDevices);
        numberOfDevices = numberOfDevicesBuildData;
    }

    // Write a line for each:
    for (int i = 0; i < numberOfDevices; i++)
    {
        // Get the device data:
        gtString deviceBuildString;
        gtString deviceName = AF_STR_Unknown;
        gtString deviceTypeAsStr = AF_STR_Unknown;
        apCLDevice currentDevice;
        bool rcDevice = gaGetOpenCLDeviceObjectDetails(programDevices[i], currentDevice);
        GT_IF_WITH_ASSERT(rcDevice)
        {
            deviceName = currentDevice.deviceNameForDisplay();
            apCLDeviceTypeAsString(currentDevice.deviceType(), deviceTypeAsStr);
        }

        // Get the build status:
        gtString buildStatusAsString = GD_STR_ShadersSourceCodeViewerSkippedBuild;
        cl_build_status buildStatus = programDevicesBuildData[i]._buildStatus;

        if (buildStatus != CL_BUILD_NONE)
        {
            apProgramBuildStatusAsString(buildStatus, buildStatusAsString);
            buildStatusAsString.toLowerCase();

            // highlight it if it's a failure:
            if (buildStatus != CL_BUILD_SUCCESS)
            {
                buildStatusAsString.prepend(AF_STR_HtmlBoldTagStart L"<font color=red>").append(L"</font>" AF_STR_HtmlBoldTagEnd);
            }
        }

        // Construct the string:
        deviceBuildString = deviceName;
        deviceBuildString.prepend(AF_STR_HtmlBoldTagStart).append(AF_STR_HtmlBoldTagEnd);
        deviceBuildString.append(L" (").append(deviceTypeAsStr).append(L") - ").append(GD_STR_ShadersSourceCodeViewerCLDeviceBuildStatusPrefix).append(buildStatusAsString);

        // Add the strings to the output string:
        buildSummaryString.append(deviceBuildString);
        buildSummaryString.append(AF_STR_HtmlEmptyLine);

        const gtString& deviceBuildLog = programDevicesBuildData[i]._buildLog;

        if (!deviceBuildLog.isEmpty())
        {
            // Parse the build log:
            gtString parsedBuildLog;
            parseOpenCLProgramBuildLog(compiledObjectData, deviceBuildLog, parsedBuildLog);

            // Add it:
            buildSummaryString.append(parsedBuildLog);
        }

        buildSummaryString.append(AF_STR_HtmlEmptyLine);
    }

    buildSummaryString.append(AF_STR_HtmlEmptyLine);
    buildSummaryString.append(AF_STR_DoneLine);
    buildSummaryString.append(AF_STR_HtmlEmptyLine);

    buildSummaryString.append(AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_HtmlPropertiesNonbreakingSpace);
    buildSummaryString.append(GD_STR_ShadersSourceCodeViewerBuildStatusPrefix);
    buildSummaryString.append(wasBuildSuccessful ? GD_STR_ShadersSourceCodeViewerStatusSuccess : GD_STR_ShadersSourceCodeViewerStatusFailure);
    afHTMLContent::endHTML(buildSummaryString);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildProgramActiveUniformHTMLString
// Description: Build a program active uniform HTML table string
// Arguments: int contextId
//            GLuint programName
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        7/1/2010
// ---------------------------------------------------------------------------
bool gdHTMLProperties::addActiveUniformsToHTMLContext(int contextId, GLuint programName, afHTMLContent& htmlContent)
{
    bool retVal = false;

    // Get the program active uniforms:
    apGLItemsCollection activeUniforms;
    retVal = gaGetProgramActiveUniforms(contextId, programName, activeUniforms);

    if (retVal)
    {
        // Build the HTML content:
        htmlContent.setColspan(3);

        // Defining a local struct to avoid having to synchronize the vectors:
        struct gdActiveUniformTableLine
        {
            gtString m_name;
            gtVector<gtString> m_typeAndValue;
            gdActiveUniformTableLine() : m_typeAndValue(2) {};
            ~gdActiveUniformTableLine() {};
        };

        gtVector<gdActiveUniformTableLine*> activeUniformDetails;
        gtVector<gdActiveUniformTableLine*> activeBuiltInUniformDetails;
        gtVector<gdActiveUniformTableLine*> activeUniformBufferDetails;

        // Iterate the uniforms, and add each of them to a vector of strings (later would be added to the table):
        int amountOfUniforms = activeUniforms.amountOfItems();

        // Allocate the uniforms:
        gdActiveUniformTableLine* pUniformDetails = new gdActiveUniformTableLine[amountOfUniforms + 1];

        const apOpenGLBuiltInUniformManager& theBuiltInUniformsMgr = apOpenGLBuiltInUniformManager::instance();

        for (int i = 0; i < amountOfUniforms; i++)
        {
            gdActiveUniformTableLine* pCurrentUniformDetails = &(pUniformDetails[i]);

            // Get the item name:
            gtString& uniformName = pCurrentUniformDetails->m_name;
            uniformName = activeUniforms.itemName(i);

            // Get the item type:
            GLenum curUniformType = activeUniforms.itemType(i);
            apGLenumValueToString(curUniformType, pCurrentUniformDetails->m_typeAndValue[0]);

            // Get the item value:
            const apParameter* pCurUniformValue = activeUniforms.itemValue(i);
            gtString& currentUniformValue = pCurrentUniformDetails->m_typeAndValue[1];
            currentUniformValue = AF_STR_NotAvailable;

            // Uniform buffer object:
            if ((curUniformType == GL_UNIFORM_BUFFER) || (curUniformType == GL_UNIFORM_BUFFER_EXT))
            {
                // Print the uniform value (as link to texture viewer):
                if (pCurUniformValue->type() == OS_TOBJ_ID_GL_UINT_PARAMETER)
                {
                    apGLuintParameter* pGLUIntParam = (apGLuintParameter*)pCurUniformValue;
                    GT_IF_WITH_ASSERT(pGLUIntParam != NULL)
                    {
                        // Build the VBO as link:
                        currentUniformValue.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesGLVBOLink, pGLUIntParam->value(), contextId);
                        currentUniformValue.appendFormattedString(GD_STR_ImagesAndBuffersViewerGLBufferNameUniform, pGLUIntParam->value());
                        currentUniformValue.append(GD_STR_HtmlPropertiesLinkEnd);
                    }
                }

                // Add the current uniform to the vector of uniform buffers:
                activeUniformBufferDetails.push_back(pCurrentUniformDetails);

            }
            else if (uniformName.startsWith(L"gl_"))
            {
                // Set the current uniform value:
                theBuiltInUniformsMgr.GetBuiltInUniformOrMemberFormula(uniformName, currentUniformValue, true);

                // Add the uniform value to the active uniform values:
                activeBuiltInUniformDetails.push_back(pCurrentUniformDetails);
            }
            else // !curUniformName.startsWith(L"gl_")
            {
                // User-defined uniform:
                if (nullptr != pCurUniformValue)
                {
                    // If the uniform is a texture uniform:
                    if (apIsTextureUniformType(curUniformType))
                    {
                        currentUniformValue = GD_STR_ShadersSourceCodeViewerHtmlTextureData;
                        gtString activeUnitAsString;
                        pCurUniformValue->valueAsString(activeUnitAsString);

                        if (activeUnitAsString != '0')
                        {
                            currentUniformValue.append(GD_STR_ShadersSourceCodeViewerHtmlTextureDataUnitNumber).append(activeUnitAsString);
                        }
                        else //activeUnitAsString == '0'
                        {
                            currentUniformValue.append(GD_STR_ShadersSourceCodeViewerHtmlTextureDataUnitZero);
                        }
                    }
                    else
                    {
                        pCurUniformValue->valueAsString(currentUniformValue);

                        // Replace the \n with <br> to show the matrix value in HTMLCtrl
                        currentUniformValue.replace(AF_STR_NewLine, AF_STR_HtmlEmptyLine, true);
                    }
                }

                // Add the uniform value to the active uniform values:
                activeUniformDetails.push_back(pCurrentUniformDetails);
            }
        }

        static gdActiveUniformTableLine emptyTableLine;
        static bool is_firstTime = true;

        if (is_firstTime)
        {
            is_firstTime = false;
            emptyTableLine.m_name = AF_STR_None;
            emptyTableLine.m_typeAndValue[0] = AF_STR_HtmlPropertiesNonbreakingSpace;
            emptyTableLine.m_typeAndValue[1] = AF_STR_HtmlPropertiesNonbreakingSpace;
        }

        // Add empty values fro he empty tables:
        if (activeUniformDetails.size() == 0)
        {
            activeUniformDetails.push_back(&emptyTableLine);
        }

        if (activeUniformBufferDetails.size() == 0)
        {
            activeUniformBufferDetails.push_back(&emptyTableLine);
        }

        if (activeBuiltInUniformDetails.size() == 0)
        {
            activeBuiltInUniformDetails.push_back(&emptyTableLine);
        }

        // Build the active uniform table section:

        // Build column headers vectors:
        gtVector<gtString> columnHeaders;
        columnHeaders.push_back(AF_STR_Type);
        columnHeaders.push_back(AF_STR_Value);

        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_COLOR_SUBTITLE, GD_STR_ShadersSourceCodeViewerHtmlActiveUniformsTitle);

        // Add an header line:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, AF_STR_Name, columnHeaders);
        int numberOfActiveUniforms = (int)activeUniformDetails.size();

        for (int i = 0; i < numberOfActiveUniforms; i++)
        {
            gdActiveUniformTableLine* pCurrentUniform = activeUniformDetails[i];
            GT_IF_WITH_ASSERT(nullptr != pCurrentUniform)
            {
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, pCurrentUniform->m_name, pCurrentUniform->m_typeAndValue);
            }
        }

        htmlContent.addSpaceLines(3);

        // Add a subtitle for the active built in uniforms:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_COLOR_SUBTITLE, GD_STR_ShadersSourceCodeViewerHtmlActiveBuildInUniformsTitle);

        // Add an header line:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, AF_STR_Name, columnHeaders);
        int numberOfActiveBuiltInUniforms = (int)activeBuiltInUniformDetails.size();

        for (int i = 0; i < numberOfActiveBuiltInUniforms; i++)
        {
            gdActiveUniformTableLine* pCurrentUniform = activeBuiltInUniformDetails[i];
            GT_IF_WITH_ASSERT(nullptr != pCurrentUniform)
            {
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, pCurrentUniform->m_name, pCurrentUniform->m_typeAndValue);
            }
        }

        htmlContent.addSpaceLines(3);

        // Add a subtitle for the active built in uniforms:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_COLOR_SUBTITLE, GD_STR_ShadersSourceCodeViewerHtmlActiveUniformBufferObjectsTitle);

        // Add an header line:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, AF_STR_Name, columnHeaders);
        int numberOfActiveUniformBuffers = (int)activeUniformBufferDetails.size();

        for (int i = 0; i < numberOfActiveUniformBuffers; i++)
        {
            gdActiveUniformTableLine* pCurrentUniform = activeUniformBufferDetails[i];
            GT_IF_WITH_ASSERT(nullptr != pCurrentUniform)
            {
                htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, pCurrentUniform->m_name, pCurrentUniform->m_typeAndValue);
            }
        }

        delete[] pUniformDetails;
    }
    else
    {
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_ShadersSourceCodeViewerHtmlActiveUniformsNotAvailable);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildProgramActiveUniformHTMLString
// Description: Build a program active uniform HTML table string
// Arguments: int contextId
//            GLuint programName
//            afHTMLContent& htmlContent
// Return Val: void
// Author:      Sigal Algranaty
// Date:        7/1/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildProgramActiveUniformHTMLString(int contextId, GLuint programName, afHTMLContent& htmlContent)
{
    // Build the program icon path:
    gtString iconPath;
    bool rcImagesPath = afGetApplicationImagesPath(iconPath);
    GT_ASSERT(rcImagesPath);

    // Build the program icon path:
    iconPath.append(osFilePath::osPathSeparator);
    iconPath.append(AF_STR_ShadingProgramFileName);
    gtString title;
    title.appendFormattedString(GD_STR_PropertiesProgramNameFormat, programName);

    // Build the HTML content:
    htmlContent.setColspan(3);
    htmlContent.setImageTitle(title, iconPath);

    addActiveUniformsToHTMLContext(contextId, programName, htmlContent);
}

// ---------------------------------------------------------------------------
// Name:        gdShadersSourceCodeViewer::parseOpenGLProgramOrShaderBuildLog
// Description: Parse the output log and HTML it
// Arguments:   compiledObjectData - the data identifying the item being compiled
// Author:      Avi Shapira
// Date:        22/11/2005
// ---------------------------------------------------------------------------
void gdHTMLProperties::parseOpenGLProgramOrShaderBuildLog(const afApplicationTreeItemData& compiledObjectData, const gtString& buildLog, gtString& htmlParsedBuildLog)
{
    htmlParsedBuildLog.makeEmpty();

    // Parse the output log and HTML it
    gtStringTokenizer tokenizer(buildLog, AF_STR_NewLine);

    gtString currentLine;
    gtString currentLineLowerCase;

    // Run on the rest of the lines:
    while (tokenizer.getNextToken(currentLine))
    {
        if (!currentLine.isEmpty())
        {
            // Add indent to the compilation lines
            htmlParsedBuildLog.append(AF_STR_HtmlPropertiesNonbreakingSpace);
            htmlParsedBuildLog.append(AF_STR_HtmlPropertiesNonbreakingSpace);
            htmlParsedBuildLog.append(AF_STR_HtmlPropertiesNonbreakingSpace);
            htmlParsedBuildLog.append(AF_STR_HtmlPropertiesNonbreakingSpace);
            htmlParsedBuildLog.append(AF_STR_HtmlPropertiesNonbreakingSpace);
        }

        // Convert to lower case:
        currentLineLowerCase = currentLine;
        currentLineLowerCase.toLowerCase();

        // parse the shader compiler output:
        int openBrackets = currentLine.find(L"(");

        // TO_DO: VS GLSL: Fix the code to work with all the errors formats
        // Check if there is an error in the line:
        int errorPosition = currentLineLowerCase.find(L"error");

        if (errorPosition != -1)
        {
            // parse the compiler output (This style was 3DLabs'):
            if (currentLineLowerCase.startsWith(L"error:"))
            {
                bool validLineNumber = false;

                int dummyInt = 0;
                int lineNumberStartPosition = 0;
                int lineNumberEndPosition = 0;
                dummyInt = currentLine.find(L":", 0);

                if (dummyInt != -1)
                {
                    lineNumberStartPosition = currentLine.find(L":", dummyInt + 1);

                    if (lineNumberStartPosition != -1)
                    {
                        lineNumberEndPosition = currentLine.find(L":", lineNumberStartPosition + 1);
                    }
                }

                if ((lineNumberStartPosition > 0) && (lineNumberEndPosition > 0))
                {
                    // Get the error line number:
                    gtString errorLineNumberString;
                    currentLine.getSubString(lineNumberStartPosition + 1, lineNumberEndPosition - 1, errorLineNumberString);
                    int errorLineNumberInt = 0;
                    bool rc = errorLineNumberString.toIntNumber(errorLineNumberInt);

                    if (rc)
                    {
                        // Get the error line number:
                        gtString preLineNumberString;
                        gtString postLineNumberString;
                        currentLine.getSubString(0, lineNumberStartPosition, preLineNumberString);
                        currentLine.getSubString(lineNumberEndPosition, -1, postLineNumberString);

                        // Get the html link string:
                        gtString htmlLink;
                        createShadersEditorOpenGLSourceCodeLocationLink(compiledObjectData, errorLineNumberInt, htmlLink);

                        // build the output string:
                        htmlParsedBuildLog.append(preLineNumberString);
                        htmlParsedBuildLog.append(htmlLink);
                        htmlParsedBuildLog.append(postLineNumberString);
                        htmlParsedBuildLog.append(AF_STR_HtmlEmptyLine);

                        validLineNumber = true;
                    }
                }

                if (!validLineNumber)
                {
                    // We did not found a valid line number
                    htmlParsedBuildLog.append(currentLine);
                    htmlParsedBuildLog.append(AF_STR_HtmlEmptyLine);
                }
            }

            else if (openBrackets != -1)
            {
                int closeBrackets = currentLine.find(L")");

                // Get the error line number:
                gtString errorLineNumberString;
                currentLine.getSubString(openBrackets + 1, closeBrackets - 1, errorLineNumberString);
                int errorLineNumberInt = 0;
                errorLineNumberString.toIntNumber(errorLineNumberInt);

                // Get the rest of the line:
                gtString errorString;
                currentLine.getSubString(closeBrackets, -1, errorString);

                // Get the HTML link string:
                gtString htmlLink;
                createShadersEditorOpenGLSourceCodeLocationLink(compiledObjectData, errorLineNumberInt, htmlLink);

                // build the output string:
                htmlParsedBuildLog.append(L"(");
                htmlParsedBuildLog.append(htmlLink);
                htmlParsedBuildLog.append(errorString);
                htmlParsedBuildLog.append(AF_STR_HtmlEmptyLine);
            }
        }
        else
        {
            // No error in line:
            htmlParsedBuildLog.append(currentLine);
            htmlParsedBuildLog.append(AF_STR_HtmlEmptyLine);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::parseOpenCLProgramBuildLog
// Description: Parses the build log into HTML format
// Author:      Uri Shomroni
// Date:        19/1/2010
// ---------------------------------------------------------------------------
void gdHTMLProperties::parseOpenCLProgramBuildLog(const afApplicationTreeItemData& compiledObjectData, const gtString& buildLog, gtString& htmlParsedBuildLog)
{
    (void)(compiledObjectData);  // unused
    htmlParsedBuildLog.makeEmpty();

    // Parse the output log and HTML it
    gtStringTokenizer tokenizer(buildLog, AF_STR_NewLine);

    gtString currentLine;
    gtString currentLineLowerCase;

    // Run on the rest of the lines:
    while (tokenizer.getNextToken(currentLine))
    {
        if (!currentLine.isEmpty())
        {
            // Add indent to the compilation lines
            htmlParsedBuildLog.append(AF_STR_HtmlPropertiesNonbreakingSpace);
            htmlParsedBuildLog.append(AF_STR_HtmlPropertiesNonbreakingSpace);
            htmlParsedBuildLog.append(AF_STR_HtmlPropertiesNonbreakingSpace);
            htmlParsedBuildLog.append(AF_STR_HtmlPropertiesNonbreakingSpace);
            htmlParsedBuildLog.append(AF_STR_HtmlPropertiesNonbreakingSpace);
        }

        // No error in line:
        htmlParsedBuildLog.append(currentLine);
        htmlParsedBuildLog.append(AF_STR_HtmlEmptyLine);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdShadersSourceCodeViewer::createShadersEditorOpenGLSourceCodeLocationLink
// Description: Create HTML link for shader / program object
// Author:      Avi Shapira
// Date:        21/11/2005
// ---------------------------------------------------------------------------
void gdHTMLProperties::createShadersEditorOpenGLSourceCodeLocationLink(const afApplicationTreeItemData& compiledObjectData, int lineNumber, gtString& htmlLink)
{
    htmlLink.makeEmpty();

    // This format must match the format in gdShadersSourceCodeBuildLogDialog::OnHTMLLinkClicked!
    // Get the link string:
    gtString objectHREFLinkStr;
    objectDataToHTMLLink(compiledObjectData, lineNumber, objectHREFLinkStr);

    // This format must match the format in gdShadersSourceCodeBuildLogDialog::OnHTMLLinkClicked!
    htmlLink.appendFormattedString(L"<A HREF=%ls>", objectHREFLinkStr.asCharArray());
    htmlLink.append(AF_STR_HtmlBoldTagStart);
    htmlLink.appendFormattedString(L"%d", lineNumber);
    htmlLink.append(AF_STR_HtmlBoldTagEnd L"</A>");

}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::htmlLinkToObjectDetails
// Description: Parse an HTML link, and translate it from a string to an object type,
//              and index
// Arguments:   const gtString& htmlLink
//              const apContextID& contextID
//              afTreeItemType& objectType
//              int& objectName
//              int& objectAdditionalParameter
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/11/2010
// ---------------------------------------------------------------------------
bool gdHTMLProperties::htmlLinkToObjectDetails(const gtString& htmlLink, afApplicationTreeItemData& objectID, int& additionalParameter)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(!htmlLink.isEmpty())
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(objectID.extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Define a tokenizer to parse the link:
            gtStringTokenizer linkTokenizer(htmlLink, L"-");

            // Get the object type:
            gtString objectTypeStr;
            bool rc = linkTokenizer.getNextToken(objectTypeStr);
            GT_IF_WITH_ASSERT(rc)
            {
                // Initialize the output:
                pGDData->_contextId._contextType = AP_OPENGL_CONTEXT;
                pGDData->_contextId._contextId = -1;

                // Get the object full name:
                gtString objectFullNameStr;
                rc = linkTokenizer.getNextToken(objectFullNameStr);

                // Get the parameters:
                int objectName = -1, objectOwnerName = -1;

                if (rc)
                {
                    rc = objectFullNameStr.toIntNumber(objectName);
                }

                // Get the second parameter:
                gtString paramStr;
                bool rc2 = linkTokenizer.getNextToken(paramStr);

                if (rc2)
                {
                    rc = paramStr.toIntNumber(pGDData->_contextId._contextId);
                    rc = rc && rc2;
                }

                // Get the third parameter:
                rc2 = linkTokenizer.getNextToken(paramStr);

                if (!paramStr.isEmpty())
                {
                    rc2 = paramStr.toIntNumber(objectOwnerName);
                    rc = rc && rc2;
                }

                // Get the forth parameter:
                rc2 = linkTokenizer.getNextToken(paramStr);

                if (!paramStr.isEmpty())
                {
                    rc2 = paramStr.toIntNumber(additionalParameter);
                    rc = rc && rc2;
                }

                // Check if this is a GL / CL object:
                if (objectTypeStr.startsWith(GD_STR_HtmlPropertiesCLPrefix))
                {
                    // OpenCL object:
                    pGDData->_contextId._contextType = AP_OPENCL_CONTEXT;
                    pGDData->_objectOpenCLName = objectName;
                    pGDData->_objectOwnerName = objectOwnerName;
                }
                else if (objectTypeStr.startsWith(GD_STR_HtmlPropertiesGLPrefix))
                {
                    // OpenGL object:
                    pGDData->_objectOpenGLName = objectName;
                }
                else
                {
                    GT_ASSERT(false);
                    rc = false;
                }

                GT_IF_WITH_ASSERT(rc)
                {
                    retVal = true;

                    // Find the right item type, and fill with details:
                    if (objectTypeStr == GD_STR_HtmlPropertiesCLContextLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_CL_CONTEXT;
                        pGDData->_contextId._contextId = objectName;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLContextLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_GL_RENDER_CONTEXT;
                        pGDData->_contextId._contextId = objectName;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLTextureLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_GL_TEXTURE;

                        // Show the texture viewer and select the texture name:
                        if (objectOwnerName < 0)
                        {
                            objectOwnerName = 0;
                        }

                        pGDData->_textureMiplevelID._textureMipLevel = objectOwnerName;
                        pGDData->_textureMiplevelID._textureName = objectName;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLTexturesThumbnailLink)
                    {

                        objectID.m_itemType = AF_TREE_ITEM_GL_TEXTURES_NODE;

                        // In this case, the link parameter is the context id:
                        pGDData->_contextId._contextId = objectName;
                        objectName = -1;
                    }

                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLRenderBuffersThumbnailLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE;

                        // In this case, the link parameter is the context id:
                        pGDData->_contextId._contextId = objectName;
                        objectName = -1;
                    }

                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLVBOsThumbnailLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_GL_VBO_NODE;

                        // In this case, the link parameter is the context id:
                        pGDData->_contextId._contextId = objectName;
                        objectName = -1;
                    }

                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLFBOsThumbnailLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_GL_FBO_NODE;

                        // In this case, the link parameter is the context id:
                        pGDData->_contextId._contextId = objectName;
                        objectName = -1;
                    }

                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLStaticBuffersThumbnailLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE;

                        // In this case, the link parameter is the context id:
                        pGDData->_contextId._contextId = objectName;
                        objectName = -1;

                    }

                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLProgramLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_GL_PROGRAM;
                    }

                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLVertexShaderLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_GL_VERTEX_SHADER;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLTessellationControlShaderLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_GL_TESSELLATION_CONTROL_SHADER;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLTessellationEvaluationShaderLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_GL_TESSELLATION_EVALUATION_SHADER;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLGeometryShaderLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_GL_GEOMETRY_SHADER;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLFragmentShaderLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_GL_FRAGMENT_SHADER;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLComputeShaderLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_GL_COMPUTE_SHADER;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLUnsupportedShaderLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_GL_UNSUPPORTED_SHADER;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesCLProgramLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_CL_PROGRAM;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesCLKernelLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_CL_KERNEL;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLStaticBufferLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_GL_STATIC_BUFFER;
                        pGDData->_bufferType = (apDisplayBuffer)objectName;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesCLDeviceLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_CL_DEVICE;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesCLBufferLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_CL_BUFFER;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesCLSubBufferLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_CL_SUB_BUFFER;
                        pGDData->_objectOpenCLOwnerIndex = objectOwnerName;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesCLImageLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_CL_IMAGE;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesCLPipeLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_CL_PIPE;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesCLPlatformLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_CL_PLATFORM;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesCLImageThumbnailsLink)
                    {
                        // In this case, the link parameter is the context id:
                        pGDData->_contextId._contextId = objectName;
                        objectName = -1;

                        objectID.m_itemType = AF_TREE_ITEM_CL_IMAGES_NODE;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesCLBuffersThumbnailLink)
                    {
                        // In this case, the link parameter is the context id:
                        pGDData->_contextId._contextId = objectName;
                        objectName = -1;

                        objectID.m_itemType = AF_TREE_ITEM_CL_BUFFERS_NODE;
                    }

                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLRenderBufferLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_GL_RENDER_BUFFER;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLFBOLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_GL_FBO;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLVBOLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_GL_VBO;
                    }
                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLPBufferLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_GL_PBUFFER_NODE;
                    }

                    else if (objectTypeStr == GD_STR_HtmlPropertiesGLPBufferStaticBufferLink)
                    {
                        objectID.m_itemType = AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER;
                        pGDData->_bufferType = (apDisplayBuffer)objectName;
                        pGDData->_objectOpenGLName = objectOwnerName;
                        pGDData->_contextId._contextType = AP_OPENGL_CONTEXT;
                    }

                    else
                    {

                        // Unrecognized object type:
                        GT_ASSERT(false);
                        retVal = false;
                    }
                }
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::objectDataToHTMLLink
// Description: Builds an HTML link from an object type and index
// Arguments:   gdDebugApplicationTreeData& objectData - the tree item data representing the object
//              int objectAdditionalParameter
//              gtString& htmlLink
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/11/2010
// ---------------------------------------------------------------------------
bool gdHTMLProperties::objectDataToHTMLLink(const afApplicationTreeItemData& objectData, int additionalParameter, gtString& htmlLink)
{
    bool retVal = true;

    gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(objectData.extendedItemData());
    GT_IF_WITH_ASSERT(pGDData != NULL)
    {
        // Get the object name according to the object type:
        int objectName = pGDData->_contextId.isOpenGLContext() ? pGDData->_objectOpenGLName : pGDData->_objectOpenCLName;

        if (objectData.m_itemType == AF_TREE_ITEM_GL_STATIC_BUFFER)
        {
            objectName = (int)pGDData->_bufferType;
        }

        if (objectData.m_itemType == AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER)
        {
            objectName = (int)pGDData->_objectOpenGLName;
        }

        switch (objectData.m_itemType)
        {
            case AF_TREE_ITEM_GL_RENDER_CONTEXT:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF1ParamStart, GD_STR_HtmlPropertiesGLContextLink, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_CL_CONTEXT:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF1ParamStart, GD_STR_HtmlPropertiesCLContextLink, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_GL_TEXTURE:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF2ParamStart, GD_STR_HtmlPropertiesGLTextureLink, objectName, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_GL_TEXTURES_NODE:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF1ParamStart, GD_STR_HtmlPropertiesGLTexturesThumbnailLink, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_GL_STATIC_BUFFER:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF2ParamStart, GD_STR_HtmlPropertiesGLStaticBufferLink, objectName, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF1ParamStart, GD_STR_HtmlPropertiesGLStaticBuffersThumbnailLink, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_GL_PBUFFER_NODE:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF1ParamStart, GD_STR_HtmlPropertiesGLPBufferLink, objectName);
                break;

            case AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF3ParamStart, GD_STR_HtmlPropertiesGLPBufferStaticBufferLink, pGDData->_bufferType, pGDData->_contextId._contextId, objectName);
                break;

            case AF_TREE_ITEM_GL_PROGRAM:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF2ParamStart, GD_STR_HtmlPropertiesGLProgramLink, objectName, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_GL_VERTEX_SHADER:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF2ParamStart, GD_STR_HtmlPropertiesGLVertexShaderLink, objectName, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_GL_TESSELLATION_CONTROL_SHADER:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF2ParamStart, GD_STR_HtmlPropertiesGLTessellationControlShaderLink, objectName, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_GL_TESSELLATION_EVALUATION_SHADER:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF2ParamStart, GD_STR_HtmlPropertiesGLTessellationEvaluationShaderLink, objectName, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_GL_GEOMETRY_SHADER:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF2ParamStart, GD_STR_HtmlPropertiesGLGeometryShaderLink, objectName, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_GL_FRAGMENT_SHADER:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF2ParamStart, GD_STR_HtmlPropertiesGLFragmentShaderLink, objectName, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_GL_COMPUTE_SHADER:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF2ParamStart, GD_STR_HtmlPropertiesGLComputeShaderLink, objectName, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_GL_UNSUPPORTED_SHADER:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF2ParamStart, GD_STR_HtmlPropertiesGLUnsupportedShaderLink, objectName, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_GL_RENDER_BUFFER:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF2ParamStart, GD_STR_HtmlPropertiesGLRenderBufferLink, objectName, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF1ParamStart, GD_STR_HtmlPropertiesGLRenderBuffersThumbnailLink, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_GL_FBO:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF2ParamStart, GD_STR_HtmlPropertiesGLFBOLink, objectName, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_GL_VBO:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF2ParamStart, GD_STR_HtmlPropertiesGLVBOLink, objectName, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_GL_VBO_NODE:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF1ParamStart, GD_STR_HtmlPropertiesGLVBOsThumbnailLink, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_GL_FBO_NODE:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF1ParamStart, GD_STR_HtmlPropertiesGLFBOsThumbnailLink, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_CL_BUFFER:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF2ParamStart, GD_STR_HtmlPropertiesCLBufferLink, objectName, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_CL_SUB_BUFFER:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF3ParamStart, GD_STR_HtmlPropertiesCLSubBufferLink, objectName, pGDData->_contextId._contextId, pGDData->_objectOwnerName);
                break;

            case AF_TREE_ITEM_CL_IMAGE:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF2ParamStart, GD_STR_HtmlPropertiesCLImageLink, objectName, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_CL_PIPE:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF2ParamStart, GD_STR_HtmlPropertiesCLPipeLink, objectName, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_CL_IMAGES_NODE:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF1ParamStart, GD_STR_HtmlPropertiesCLImageThumbnailsLink, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_CL_BUFFERS_NODE:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF1ParamStart, GD_STR_HtmlPropertiesCLBuffersThumbnailLink, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_CL_PROGRAM:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF2ParamStart, GD_STR_HtmlPropertiesCLProgramLink, objectName, pGDData->_contextId._contextId);
                break;

            case AF_TREE_ITEM_CL_KERNEL:
                htmlLink.appendFormattedString(AF_STR_HtmlPropertiesLinkHREF2ParamStart, GD_STR_HtmlPropertiesCLKernelLink, objectName, pGDData->_contextId._contextId, pGDData->_objectOwnerName);
                break;

            default:
                GT_ASSERT_EX(false, L"Unsupported object type");
                retVal = false;
        }

        // Add the additional parameter to the HTML link:
        if (additionalParameter >= 0)
        {
            htmlLink.appendFormattedString(L"-%d", additionalParameter);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::itemLinkToDisplayString
// Description:
// Arguments:   const gtString& itemLinkStr
//              gtString& itemDisplayString
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        3/1/2011
// ---------------------------------------------------------------------------
bool gdHTMLProperties::itemLinkToDisplayString(const gtString& itemLinkStr, gtString& itemDisplayString)
{
    bool retVal = false;

    // Initialize the item display string:
    itemDisplayString = AF_STR_ItemIDUnknown;

    // Translate the object link to an item data:
    afApplicationTreeItemData objectID;
    gdDebugApplicationTreeData* pGDData = new gdDebugApplicationTreeData;

    objectID.setExtendedData(pGDData);
    int additionalParameter = -1;
    bool rc = htmlLinkToObjectDetails(itemLinkStr, objectID, additionalParameter);
    GT_IF_WITH_ASSERT(rc)
    {
        retVal = itemIDAsString(objectID, itemDisplayString);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::itemIDAsString
// Description:
// Arguments:   const gdDebugApplicationTreeData& objectID
//              gtString& itemDisplayString
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/1/2011
// ---------------------------------------------------------------------------
bool gdHTMLProperties::itemIDAsString(const afApplicationTreeItemData& objectID, gtString& itemDisplayString)
{
    bool retVal = true;

    gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(objectID.extendedItemData());
    GT_IF_WITH_ASSERT(pGDData != NULL)
    {
        itemDisplayString.makeEmpty();

        if (!pGDData->_contextId.isDefault())
        {
            // Initialize the string with the item context id:
            pGDData->_contextId.toString(itemDisplayString);
            itemDisplayString.append(AF_STR_Space);
        }

        // Get the item type as string:
        gtString itemTypeStr;
        afApplicationTreeItemData::itemTypeAsString(objectID.m_itemType, itemTypeStr);

        if ((objectID.m_itemType != AF_TREE_ITEM_GL_RENDER_CONTEXT) && (objectID.m_itemType != AF_TREE_ITEM_CL_CONTEXT))
        {
            itemDisplayString.append(itemTypeStr);

            // If the object is not a thumbnail, add the item name:
            if (!afApplicationTreeItemData::isItemThumbnail(objectID.m_itemType))
            {
                // Static buffers:
                if ((objectID.m_itemType == AF_TREE_ITEM_GL_STATIC_BUFFER) || (objectID.m_itemType == AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER))
                {
                    // Get the static buffer type as string:
                    gtString bufferName;
                    bool rc = apGetBufferName(pGDData->_bufferType, bufferName);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        itemDisplayString.appendFormattedString(L" %ls", bufferName.asCharArray());
                    }
                }
                else
                {
                    // Append the object name to the string:
                    int objectName = pGDData->_objectOpenGLName;

                    if (pGDData->_contextId.isOpenCLContext())
                    {
                        objectName = pGDData->_objectOpenCLName;
                    }

                    if ((objectName >= 0) && !afApplicationTreeItemData::isItemTypeRoot(objectID.m_itemType))
                    {
                        itemDisplayString.appendFormattedString(L" %d", objectName);
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::addDevicesToHTMLContent
// Description: Add the OpenCL devices to an HTML content
// Arguments:   const apCLContext& contextInfo
//              afHTMLContent& htmlContent
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/2/2011
// ---------------------------------------------------------------------------
bool gdHTMLProperties::addDevicesToHTMLContent(const apCLContext& contextInfo, afHTMLContent& htmlContent)
{
    bool retVal = true;

    // Get the context's devices:
    const gtVector<int>& contextDevices = contextInfo.deviceIDs();

    // Add the kernelHandles title:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, AF_STR_ContextsInformationsDialogDevices);

    // Iterate the input devices:
    size_t devicesAmount = contextDevices.size();

    for (size_t i = 0; i < devicesAmount; i++)
    {
        // Get the current device details:
        int currDeviceId = contextDevices[i];
        apCLDevice deviceDetails;
        bool rc = gaGetOpenCLDeviceObjectDetails(currDeviceId, deviceDetails);
        GT_IF_WITH_ASSERT(rc)
        {
            // Add the device id:
            gtString currDeviceIdAsStr;
            currDeviceIdAsStr.appendFormattedString(AF_STR_ContextsInformationsDialogDeviceHeader, apCLDeviceIndexToDisplayIndex(currDeviceId));

            // Format the device id as HTML link:
            gtString currDeviceIdAsLink;
            currDeviceIdAsLink.appendFormattedString(AF_STR_HtmlPropertiesLink1ParamStart, GD_STR_HtmlPropertiesCLDeviceLink, apCLDeviceIndexToDisplayIndex(currDeviceId));
            currDeviceIdAsLink.append(currDeviceIdAsStr);
            currDeviceIdAsLink.append(GD_STR_HtmlPropertiesLinkEnd);

            // Add the device as link:
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationsDialogDeviceId, currDeviceIdAsLink);

            // Add the device type:
            gtString deviceTypeAsStr;
            apCLDeviceTypeAsString(deviceDetails.deviceType(), deviceTypeAsStr);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationsDialogDeviceType, deviceTypeAsStr);

            // Add the device name:
            const gtString& deviceName = deviceDetails.deviceNameForDisplay();
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationsDialogDeviceName, deviceName);

            // Add the device version:
            const gtString& deviceVersion = deviceDetails.deviceVersion();
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationsDialogDeviceVersion, deviceVersion);

            // Add the device vendor:
            const gtString& deviceVendor = deviceDetails.deviceVendor();
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationsDialogDeviceVendor, deviceVendor);

            // TO_DO: OpenCL - Add deviceAllowedQueueProperties and many other device properties that are not currently
            // expressed in apCLDevice
        }
        else
        {
            retVal = false;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenCLContextHTMLPropertiesString
// Description: Build an OpenCL context HTML string
// Arguments:   apCLContext contextInfo - the OpenCL context information
//              afHTMLContent& htmlContent
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        6/2/2011
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenCLContextHTMLPropertiesString(const apContextID& contextID, const apCLContext& contextInfo, afHTMLContent& htmlContent)
{
    // Set the context's HTML information title:
    gtString contextStr;
    contextID.toString(contextStr);
    htmlContent.setTitle(contextStr);

    // Add "General" header:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    // Add the context's reference count:
    gtUInt32 refCount = contextInfo.referenceCount();
    gtString refCountAsStr;
    refCountAsStr.appendFormattedString(L"%u", refCount);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesReferenceCountHeader, refCountAsStr);

    // Context deletion status:
    bool wasDeleted = contextInfo.wasMarkedForDeletion();
    gtString deletionStatus = wasDeleted ? AF_STR_Yes : AF_STR_No;
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogMarkedForDeletion, deletionStatus);

    gtString glContextStr;
    int glContextIndex = contextInfo.openGLSpyID();

    if (glContextIndex > 0)
    {
        // The context is sharing, so show which context it is sharing:
        glContextStr.appendFormattedString(GD_STR_HtmlPropertiesGLContextFullLink, glContextIndex, glContextIndex);
    }
    else
    {
        glContextStr = AF_STR_None;
    }

    // Add the OpenGL Context HTML line:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogGLSharedContext, glContextStr);

    // Add the context creation properties:
    const apCLContextProperties& contextProps = contextInfo.contextCreationProperties();
    int amountOfProps = contextProps.amountOfProperties();

    if (amountOfProps > 0)
    {
        // Add the context created properties header:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesContextCreationPropertiesHeader);
    }

    for (int i = 0; i < amountOfProps; i++)
    {
        // Get the current property as string:
        gtString propName, propValue;
        contextProps.clPropertyAsString(i, propName, propValue);

        if (propName == L"CL_CONTEXT_PLATFORM")
        {
            // Convert the platform to a link:
            unsigned long long platformIdAsULongLong = 0;
            bool rc = propValue.toUnsignedLongLongNumber(platformIdAsULongLong);
            GT_IF_WITH_ASSERT(rc)
            {
                // Get the platform name from the platform id:
                int platformName;
                gtUInt64 platformIdAsUInt64 = (gtUInt64)platformIdAsULongLong;
                rc = gaGetOpenCLPlatformAPIID(platformIdAsUInt64, platformName);
                GT_IF_WITH_ASSERT(rc)
                {
                    propValue.makeEmpty();
                    propValue.appendFormattedString(L"Platform %d", platformName + 1);
                    propValue.prependFormattedString(AF_STR_HtmlPropertiesLink1ParamStart, GD_STR_HtmlPropertiesCLPlatformLink, platformName);
                    propValue.append(GD_STR_HtmlPropertiesLinkEnd);
                }
            }
        }

        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, propName, propValue);
    }

    // Add the OpenCL devices for this context:
    bool rcDevices = addDevicesToHTMLContent(contextInfo, htmlContent);
    GT_ASSERT(rcDevices);
}


// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::addOpenGLContextHeaderToHTMLContent
// Description: Add an OpenGL header information to the HTML content
// Arguments:   const apContextID& contextID
//              const apGLRenderContextInfo& contextInfo
//              const apGLRenderContextGraphicsInfo& contextGraphicsInfo
//              afHTMLContent& htmlContent
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        22/2/2011
// ---------------------------------------------------------------------------
void gdHTMLProperties::addOpenGLContextHeaderToHTMLContent(const apContextID& contextID, const apGLRenderContextInfo& contextInfo,
                                                           const apGLRenderContextGraphicsInfo& contextGraphicsInfo, afHTMLContent& htmlContent)
{
    // Check if this context deletion and share statuses:
    bool wasDeleted = gaWasContextDeleted(contextID);
    bool isSharing = (contextInfo.sharingContextID() != -1);
    bool isShared = (contextGraphicsInfo.getSharingContexts().size() > 0);

    // Add "General" header to the HTML content:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, GD_STR_PropertiesGeneral);

    // OpenGL and GLSL versions:
    int openGLMajorVersion = -1, openGLMinorVersion = -1;
    contextGraphicsInfo.getOpenGLVersion(openGLMajorVersion, openGLMinorVersion);
    gtString oglVersionStr;
    oglVersionStr.appendFormattedString(L"%d.%d", openGLMajorVersion, openGLMinorVersion);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogOpenGLVersion, oglVersionStr);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogShadingLangVersion, contextGraphicsInfo.getShadingLanguageVersionString());

    // Add compatibility flags:
    gtString backwardCompatibleStr = (contextGraphicsInfo.isComaptibilityContext()) ? AF_STR_Yes : AF_STR_No;
    gtString forewardCompatibleStr = (contextGraphicsInfo.isForwardCompatible()) ? AF_STR_Yes : AF_STR_No;
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogIsBackwardCompatible, backwardCompatibleStr);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogIsForewardCompatible, forewardCompatibleStr);

    // Add debug flags:
    gtString debugContextStr = (contextGraphicsInfo.isDebugContext()) ? AF_STR_Yes : AF_STR_No;

    if (contextGraphicsInfo.isDebugContextFlagForced())
    {
        debugContextStr.append(AF_STR_ContextsInformationDialogCodeXLForced);
    }

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogIsDebugContext, debugContextStr);

    // Add Graphic card details:
    gtString rendererVendor;
    gtString rendererName;
    gtString rendererVersion;
    contextGraphicsInfo.getRendererInformation(rendererVendor, rendererName, rendererVersion);

    gtString rendererType;
    apGLRenderContextGraphicsInfo::hardwareAcceleration accelLevel = contextGraphicsInfo.hardwareAccelerationLevel();

    if (accelLevel == apGLRenderContextGraphicsInfo::AP_FULL_HARDWARE_ACCELERATED_CONTEXT)
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        {
            rendererType = AF_STR_SystemInformationCommandInstallableClient;
        }
#else
        {
            rendererType = AF_STR_SystemInformationCommandHardwareRenderer;
        }
#endif
    }
    else if (accelLevel == apGLRenderContextGraphicsInfo::AP_PARTIAL_HARDWARE_ACCELERATED_CONTEXT)
    {
        rendererType = AF_STR_SystemInformationCommandMiniClient;
    }
    else
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        {
            rendererType = AF_STR_SystemInformationCommandGenericOpenGLSoftwareRenderer;
        }
#else
        {
            rendererType = AF_STR_SystemInformationCommandSoftwareRenderer;
        }
#endif
    }

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogRendererVendor, rendererVendor);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogRendererName, rendererName);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogRendererVersion, rendererVersion);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogRendererType, rendererType);

    // Context deletion status:
    gtString deletionStatus = wasDeleted ? AF_STR_Yes : AF_STR_No;
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogMarkedForDeletion, deletionStatus);

    // Sharing / shared information:
    if (isSharing)
    {
        // The context is sharing, so show which context it is sharing:
        int sharingContext = contextInfo.sharingContextID();
        gtString contextLink;
        contextLink.appendFormattedString(GD_STR_HtmlPropertiesGLContextFullLink, sharingContext, sharingContext);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogGLSharedContext, contextLink);
    }
    else
    {
        size_t numberOfSharingContexts = contextGraphicsInfo.getSharingContexts().size();
        gtString sharedContextStr;

        if (isShared)
        {
            // The context is shared, show which contexts are using its resources:
            int contextNumber = contextGraphicsInfo.getSharingContexts()[0];
            sharedContextStr.appendFormattedString(GD_STR_HtmlPropertiesGLContextFullLink, contextNumber, contextNumber);

            for (size_t i = 1; i < numberOfSharingContexts; i++)
            {
                contextNumber = contextGraphicsInfo.getSharingContexts()[i];
                sharedContextStr.append(L", ").appendFormattedString(GD_STR_HtmlPropertiesGLContextFullLink, contextNumber, contextNumber);
            }
        }
        else
        {
            // The context is neither shared nor sharing:
            sharedContextStr.append(AF_STR_None);
        }

        // Add "Sharing contexts" section:
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogGLSharingContexts, sharedContextStr);
    }

    gtString clContextStr;

    if (contextInfo.openCLSpyID() > 0)
    {
        // The context is sharing, so show which context it is sharing:
        int clContextIndex = contextInfo.openCLSpyID();
        clContextStr.appendFormattedString(GD_STR_HtmlPropertiesCLContextFullLink, clContextIndex, clContextIndex);
    }
    else
    {
        clContextStr = AF_STR_None;
    }

    // Add the OpenCL Context HTML line:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogCLSharedContexts, clContextStr);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenGLContextHTMLPropertiesString
// Description: Builds an OpenGL context information HTML
// Arguments:   const apContextID& contextID
//              const apGLRenderContextInfo& contextInfo
//              const apGLRenderContextGraphicsInfo& contextGraphicsInfo
//              bool isOpenGLESProject
//              afHTMLContent& htmlContent
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        6/2/2011
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenGLContextHTMLPropertiesString(const apContextID& contextID, const apGLRenderContextInfo& contextInfo, const apGLRenderContextGraphicsInfo& contextGraphicsInfo,
                                                              bool isOpenGLESProject, afHTMLContent& htmlContent)
{
    // Set the context's HTML information title:
    gtString contextStr;
    contextID.toString(contextStr);
    htmlContent.setTitle(contextStr);

    // Add the OpenGL "general" header to the HTML content:
    addOpenGLContextHeaderToHTMLContent(contextID, contextInfo, contextGraphicsInfo, htmlContent);

    // Pixel format information:
    int pixelFormatIndex = -1;
    bool isDoubleBuffered = false;
    apGLRenderContextGraphicsInfo::hardwareAcceleration acceleration = apGLRenderContextGraphicsInfo::AP_UNKNOWN_HARDWARE_ACCELERATED_CONTEXT;
    bool isStereographic = false;
    bool supportsNative = false;
    contextGraphicsInfo.getGeneralGraphicsInfo(pixelFormatIndex, isDoubleBuffered, acceleration, isStereographic, supportsNative);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, AF_STR_ContextsInformationDialogPixelFormatInfoHeader);

    if (pixelFormatIndex > 0)
    {
        gtString pixelFormatStr;
        pixelFormatStr.appendFormattedString(L"%d", pixelFormatIndex);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogPixelFormatId, pixelFormatStr);
    }

    // Hardware acceleration:
    gtString hardwareAccelerationStr;

    switch (acceleration)
    {
        case apGLRenderContextGraphicsInfo::AP_FULL_HARDWARE_ACCELERATED_CONTEXT:
            hardwareAccelerationStr.append(AF_STR_Full);
            break;

        case apGLRenderContextGraphicsInfo::AP_PARTIAL_HARDWARE_ACCELERATED_CONTEXT:
            hardwareAccelerationStr.append(AF_STR_Partial);
            break;

        case apGLRenderContextGraphicsInfo::AP_NOT_HARDWARE_ACCELERATED_CONTEXT:
            hardwareAccelerationStr.append(AF_STR_None);
            break;

        case apGLRenderContextGraphicsInfo::AP_UNKNOWN_HARDWARE_ACCELERATED_CONTEXT:
        default:
        {
            hardwareAccelerationStr.append(AF_STR_Unknown);
            GT_ASSERT(false);
        }
        break;
    }

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogHardwareAcceleration, hardwareAccelerationStr);

    // Double buffer:
    gtString contentStr;
    contentStr.append(isDoubleBuffered ? AF_STR_Yes : AF_STR_No);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogDoubleBuffered, contentStr);

    // OpenGL ES does not support stereography:
    if (!isOpenGLESProject)
    {
        // Stereographic:
        contentStr.makeEmpty();
        contentStr.append(isStereographic ? AF_STR_Yes : AF_STR_No);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogStereographic, contentStr);
    }

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    // GDI rendering (relevant only on windows):
    contentStr.makeEmpty();
    contentStr.append(supportsNative ? AF_STR_Yes : AF_STR_No);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogSupportsNative, contentStr);
#endif

    // Channels and bit depths for each:
    unsigned int red = 0, green = 0, blue = 0, alpha = 0, index = 0, depth = 0, stencil = 0, accum = 0;
    contextGraphicsInfo.getChannels(red, green, blue, alpha, index, depth, stencil, accum);

    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_SUB_TITLE, AF_STR_ContextsInformationDialogChannelHeader, AF_STR_ContextsInformationDialogBitsHeader);

    if (index > 0)
    {
        // If there is an index channel, there should be no RGBA channels:
        GT_ASSERT((red + green + blue + alpha) == 0);
        contentStr.makeEmpty();
        contentStr.appendFormattedString(L"%u", index);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogIndexChannel, contentStr);
    }
    else
    {
        // If there is no index channel, there should be RGBA channels:
        GT_ASSERT((red + green + blue + alpha) > 0);
        contentStr.makeEmpty();
        contentStr.appendFormattedString(L"%u", red);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogRedChannel, contentStr);
        contentStr.makeEmpty();
        contentStr.appendFormattedString(L"%u", blue);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogBlueChannel, contentStr);
        contentStr.makeEmpty();
        contentStr.appendFormattedString(L"%u", green);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogGreenChannel, contentStr);
        contentStr.makeEmpty();
        contentStr.appendFormattedString(L"%u", alpha);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogAlphaChannel, contentStr);
    }

    contentStr.makeEmpty();
    contentStr.appendFormattedString(L"%u", depth);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogDepthChannel, contentStr);

    contentStr.makeEmpty();
    contentStr.appendFormattedString(L"%u", stencil);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogStencilChannel, contentStr);

    // OpenGL ES does no support accumulation buffers:
    if (!isOpenGLESProject)
    {
        contentStr.makeEmpty();
        contentStr.appendFormattedString(L"%u", accum);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, AF_STR_ContextsInformationDialogAccumulationChannel, contentStr);
    }
}

// ---------------------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenGlSamplersListHTMLPropertiesString
// Description: Builds a message for the samplers list in a given OpenGL context
// Author:      Amit Ben-Moshe
// Date:        23/6/2014
// ---------------------------------------------------------------------------------------
void gdHTMLProperties::buildOpenGlSamplersListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects)
{
    (void)(numberOfObjects);  // unused
    gtString samplerListTitle;
    samplerListTitle.appendFormattedString(GD_STR_PropertiesOpenGlSamplersListHeadline, contextID._contextId);

    htmlContent.setTitle(samplerListTitle);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, GD_STR_PropertiesSelectItemMessage);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::buildOpenGlSamplersHTMLPropertiesString
// Description: Builds an OpenGL sampler's properties in HTML format
// Arguments:   const apContextID contextID - the relevant OpenGL context id
//              const apGLSampler& samplerDetails - an object holding the sampler's details
//              afHTMLContent& htmlContent - the HTML object to be updated
// Return Val:  void
// Author:      Amit Ben-Moshe
// Date:        23/6/2014
// ---------------------------------------------------------------------------
void gdHTMLProperties::buildOpenGlSamplersHTMLPropertiesString(const apGLSampler& samplerDetails, afHTMLContent& htmlContent)
{
    // Build the sampler name string.
    gtString samplerNameStr;
    samplerNameStr.appendFormattedString(GD_STR_PropertiesOpenGlSamplerHeadline, samplerDetails.samplerName());

    // Create string buffers.
    gtVector<GLuint> boundTextures;
    samplerDetails.getBoundTextures(boundTextures);

    // Build the bound textures string.
    gtString boundTexturesStr(L"{ ");
    size_t upperBound = boundTextures.size();

    for (size_t i = 0; i < upperBound; ++i)
    {
        boundTexturesStr << boundTextures[i];

        if (i < (upperBound - 1))
        {
            // If this is not the last one - append a comma.
            boundTexturesStr << L", ";
        }
    }

    boundTexturesStr << L" }";

    // Build the RGBA border color strings.
    gtString rgbaBorderColorRed;
    gtString rgbaBorderColorGreen;
    gtString rgbaBorderColorBlue;
    gtString rgbaBorderColorAlpha;

    // Extract the color RGBA values.
    GLfloat r, g, b, a;
    samplerDetails.getSamplerRgbaColor(r, g, b, a);

    // Build the color value strings.
    rgbaBorderColorRed   << r;
    rgbaBorderColorGreen << g;
    rgbaBorderColorBlue  << b;
    rgbaBorderColorAlpha << a;

    // Build the texture Level of Detail strings.
    gtString lodBias;
    gtString maxLod;
    gtString minLod;

    // Extract the Level of Detail values.
    lodBias << samplerDetails.getTextureLodBias();
    maxLod << samplerDetails.getTextureMaxLod();
    minLod << samplerDetails.getTextureMinLod();

    // Build all other strings.
    gtString comparisonMode;
    gtString comparisonFunction;
    gtString magFilter;
    gtString minFilter;
    gtString wrapS;
    gtString wrapT;
    gtString wrapR;

    // Extract the values and convert them to strings for representation.
    GT_ASSERT(samplerDetails.getSamplerComparisonModeAsString(comparisonMode));
    GT_ASSERT(apGLenumValueToString(samplerDetails.getSamplerComparisonFunction(), comparisonFunction));
    GT_ASSERT(apGLenumValueToString(samplerDetails.getTextureMagFilter(), magFilter));
    GT_ASSERT(apGLenumValueToString(samplerDetails.getTextureMinFilter(), minFilter));
    GT_ASSERT(apGLenumValueToString(samplerDetails.getTextureWrapS(), wrapS));
    GT_ASSERT(apGLenumValueToString(samplerDetails.getTextureWrapT(), wrapT));
    GT_ASSERT(apGLenumValueToString(samplerDetails.getTextureWrapR(), wrapR));

    // Build the HTML content.
    htmlContent.setTitle(samplerNameStr);

    //// Add the sampler's state descriptors.
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSamplerBoundTextures, boundTexturesStr);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSamplerBorderColorRgbaRed, rgbaBorderColorRed);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSamplerBorderColorRgbaGreen, rgbaBorderColorGreen);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSamplerBorderColorRgbaBlue, rgbaBorderColorBlue);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSamplerBorderColorRgbaAlpha, rgbaBorderColorAlpha);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSamplerTextureComparisonFunc, comparisonFunction);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSamplerTextureComparisonMode, comparisonMode);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSamplerTextureLodBias, lodBias);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSamplerTextureMaxLod, maxLod);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSamplerTextureMinLod, minLod);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSamplerTextureMagFilter, magFilter);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSamplerTextureMinFilter, minFilter);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSamplerTextureWrapS, wrapS);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSamplerTextureWrapT, wrapT);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, GD_STR_PropertiesSamplerTextureWrapR, wrapR);
}

// ---------------------------------------------------------------------------
// Name:        gdHTMLProperties::generateShaderHtmlLinkString
// Description: Builds a link string for a given shader type
// Arguments:   int contextId - the relevant GL context id
//              GLuint programName - the relevant GL program object (where the shader should be located)
//              gdShaderType shaderType - the shader type according which the string should be created
//              gtString& linkStrBuffer - a buffer to hold the output string
// Return Val:  bool
// Author:      Amit Ben-Moshe
// Date:        3/8/2014
// ---------------------------------------------------------------------------
bool gdHTMLProperties::generateShaderHtmlLinkString(int contextId, GLuint programName, gdShaderType shaderType, gtString& linkStrBuffer)
{
    bool ret = false;
    linkStrBuffer = L"";
    GLuint curShadName = 0;
    gtString curShadString;

    // If program not exist not make sense to continue
    if (programName == 0)
    {
        linkStrBuffer = GD_STR_ShadersSourceCodeViewerListCtrlOpenCLProgramNotAttached;
        ret = true;
    }
    else
    {
        // First get the shader link name.
        gtString shaderLinkName;
        bool isOk = GetShaderLinkNameByShaderType(shaderType, shaderLinkName);
        GT_IF_WITH_ASSERT(isOk)
        {
            apGLProgram progBuffer;

            // Get the details of the relevant program object.
            isOk = gaGetProgramObjectDetails(contextId, programName, progBuffer);
            GT_IF_WITH_ASSERT(isOk)
            {
                const gtList<GLuint> progShaders = progBuffer.shaderObjects();

                // Go through the program's shader, and find the relevant shader object.
                for (gtList<GLuint>::const_iterator iter = progShaders.begin(); iter != progShaders.end(); ++iter)
                {
                    gtAutoPtr<apGLShaderObject> aptrCurrentShader = NULL;
                    bool rcShad = gaGetShaderObjectDetails(contextId, (*iter), aptrCurrentShader);
                    GT_IF_WITH_ASSERT(rcShad && (aptrCurrentShader.pointedObject() != NULL))
                    {
                        afTreeItemType objectType = AF_TREE_ITEM_ITEM_NONE;
                        gdShaderType curShadType = gdShaderTypeFromTransferableObjectType(aptrCurrentShader->type(), objectType);

                        if (curShadType == shaderType)
                        {
                            // This is the one.
                            curShadName = aptrCurrentShader->shaderName();
                            gdShaderNameStringFromNameAndType(curShadName, curShadType, curShadString);
                            curShadString.prependFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, shaderLinkName.asCharArray(), curShadName, contextId).append(GD_STR_HtmlPropertiesLinkEnd);
                            break;
                        }
                    }
                }

                // Shader was not found.
                if (curShadString.isEmpty())
                {
                    curShadString = GD_STR_ShadersSourceCodeViewerListCtrlUnknownShaderNotAttached;
                }

                // Now build the output string.
                gtString programNameStr;
                programNameStr.appendFormattedString(GD_STR_PropertiesProgramNameFormat, programName);

                linkStrBuffer.appendFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesGLProgramLink, programName, contextId);
                linkStrBuffer.append(programNameStr).append(GD_STR_HtmlPropertiesLinkEnd).append(L" - ").append(curShadString);
                ret = true;
            }
        }
    }

    return ret;
}

