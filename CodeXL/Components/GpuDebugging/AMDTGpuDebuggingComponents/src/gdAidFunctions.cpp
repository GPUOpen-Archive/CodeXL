//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdAidFunctions.cpp
///
//==================================================================================

//------------------------------ gdAidFunctions.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osBundle.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLRenderContext.h>
#include <AMDTOSWrappers/Include/osRawMemoryBuffer.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTAPIClasses/Include/apCLBuffer.h>
#include <AMDTAPIClasses/Include/apCLContext.h>
#include <AMDTAPIClasses/Include/apCLDevice.h>
#include <AMDTAPIClasses/Include/apCLKernel.h>
#include <AMDTAPIClasses/Include/apCLImage.h>
#include <AMDTAPIClasses/Include/apCLObjectID.h>
#include <AMDTAPIClasses/Include/apCLPipe.h>
#include <AMDTAPIClasses/Include/apCLProgram.h>
#include <AMDTAPIClasses/Include/apKernelFunctionNameBreakpoint.h>
#include <AMDTAPIClasses/Include/apHostSourceBreakPoint.h>
#include <AMDTAPIClasses/Include/apKernelSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/apFunctionCall.h>
#include <AMDTAPIClasses/Include/apGLRenderBuffer.h>
#include <AMDTAPIClasses/Include/apGLRenderContextGraphicsInfo.h>
#include <AMDTAPIClasses/Include/apGLRenderContextInfo.h>
#include <AMDTAPIClasses/Include/apGLShaderObject.h>
#include <AMDTAPIClasses/Include/apGLTexture.h>
#include <AMDTAPIClasses/Include/apGLTextureMipLevel.h>
#include <AMDTAPIClasses/Include/apGLVBO.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionBreakPoint.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>
#include <AMDTAPIClasses/Include/apSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/Events/apExecutionModeChangedEvent.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acColours.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>


// Platform-specific includes:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    #include <Carbon/Carbon.h>
#endif

// This definition is missing from the Linux 2.9.2 headers for some reason
#ifndef wxST_MARKUP
    #define wxST_MARKUP 0x0002
#endif // ndef wxST_MARKUP

// Transforms days to seconds
#define daysToSeconds (24 * 60 * 60)

// Used in gdSetViewerSizeAndPosition
#define GD_MINIMAL_VIEWER_WIDTH_PROPORTION 0.3F


// ---------------------------------------------------------------------------
// Name:        gdGetCurrentBreakpointFunction
// Description: Inputs a breakpoint event and output formatted strings that contain
//              the function name and its arguments.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        1/7/2004
// ---------------------------------------------------------------------------
bool gdGetCurrentBreakpointFunction(const apFunctionCall* pBreakedOnFunctionCall, gtString& funcName, gtString& funcArgs)
{
    bool retVal = false;

    if (pBreakedOnFunctionCall)
    {
        // Get the function name:
        apMonitoredFunctionId funcId = pBreakedOnFunctionCall->functionId();
        bool rc = gaGetMonitoredFunctionName(funcId, funcName);

        if (rc)
        {
            // Get the function Arguments:
            funcArgs += L"(";

            // Get the function arguments:
            const gtList<const apParameter*>& funcArguments = pBreakedOnFunctionCall->arguments();

            // Iterate them:
            gtList<const apParameter*>::const_iterator iter = funcArguments.begin();
            gtList<const apParameter*>::const_iterator endIter = funcArguments.end();

            gtString currentArgumentValueAsString;

            while (iter != endIter)
            {
                // Get the current argument value (as a string):
                (*(*iter)).valueAsString(currentArgumentValueAsString);

                // Add it to the dialog:
                funcArgs += currentArgumentValueAsString;

                iter++;

                // Add the "," only if it is NOT the last parameter
                if (iter != endIter)
                {
                    funcArgs += L" , ";
                }
            }
        }

        funcArgs += L")";

        retVal = true;
    }

    return retVal;
}





// ---------------------------------------------------------------------------
// Name:        gdGetContextNameString
// Description: Inputs into contextNameString a string with the name of
//              the OpenGL / OpenCL context. Examples: "CL Context 3", "GL Context 7 (deleted)",
//              "GL Context 11 (Shared - GL2)", "CL Context 3 (Shared - GL5)","No Context".
// Author:      Uri Shomroni
// Date:        10/11/2008
// ---------------------------------------------------------------------------
void gdGetContextNameString(apContextID contextID, gtString& contextNameString, bool addDeletionMark)
{
    GT_IF_WITH_ASSERT(contextID._contextId >= 0)
    {
        contextNameString.makeEmpty();

        // For OpenGL Contexts other than the "No Context", we need to know if the context
        // is deleted or shared:
        if (contextID.isOpenGLContext())
        {
            // Check if the context was deleted:
            bool wasDeleted = gaWasContextDeleted(contextID);

            // Get the context shared context id:
            apGLRenderContextInfo contextInfo;
            int glSharingContextID = -1;
            int clSharedContextID = -1;
            bool rcRCInfo = gaGetRenderContextDetails(contextID._contextId, contextInfo);
            GT_IF_WITH_ASSERT(rcRCInfo)
            {
                // Get the OpenGL sharing context id:
                glSharingContextID = contextInfo.sharingContextID();

                // Get the OpenCL shared context id:
                clSharedContextID = contextInfo.openCLSpyID();
            }

            // Translate the context id to a string:
            contextID.toString(contextNameString, wasDeleted, glSharingContextID, clSharedContextID);
        }
        else // (!contextID.isOpenGLContext())
        {
            bool wasDeleted = false;

            // OpenCL context can be released, OpenGL Context 0 cannot:
            if (contextID.isOpenCLContext())
            {
                wasDeleted = gaWasContextDeleted(contextID);
            }

            // Get the OpenGL shared context:
            apCLContext contextInfo;
            int glSharedContext = -1;

            if (!contextID.isDefault())
            {
                bool rcCtxInfo = gaGetOpenCLContextDetails(contextID._contextId, contextInfo);
                GT_IF_WITH_ASSERT(rcCtxInfo)
                {
                    glSharedContext = contextInfo.openGLSpyID();
                }
            }

            // Translate the context id to a string:
            contextID.toString(contextNameString, wasDeleted, glSharedContext);

            // Add '*' in the end of a deleted context name:
            if (wasDeleted && addDeletionMark)
            {
                contextNameString.append(L" *");
            }

            // Check if has AMD ext name and attach it:
            if (!contextInfo.contextName().isEmpty())
            {
                contextNameString.append(L" (");
                contextNameString.append(contextInfo.contextName());
                contextNameString.append(L")");
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdExtractiPhoneSDKNameFromFullPath
// Description: Inputs an iPhone SDK / OpenGL ES framework path and outputs its
//              version names as a string (e.g. "3.1") or an empty string if the
//              format was not matched
// Author:      Uri Shomroni
// Date:        5/5/2010
// ---------------------------------------------------------------------------
void gdExtractiPhoneSDKNameFromFullPath(const gtString& sdkFullPath, gtString& sdkName)
{
    // Search the SDK name format in the end of string:
    static const gtString sdkNamePrefix = GD_STR_DebugSettingsiPhoneSimulatorSDKsPrefix;
    static const int sdkNamePrefixLength = sdkNamePrefix.length();
    static const gtString sdkNameSuffix = GD_STR_DebugSettingsiPhoneSimulatorSDKsSuffix;
    int sdkNameSuffixPosition = sdkFullPath.reverseFind(sdkNameSuffix);
    int sdkNamePrefixPosition = sdkFullPath.reverseFind(sdkNamePrefix, sdkNameSuffixPosition);

    // Note that suff > pref + len > pref > -1 implies suff > -1 (even suff > 0), so we don't need to test it specifically:
    if ((sdkNameSuffixPosition > sdkNamePrefixPosition + sdkNamePrefixLength) && (sdkNamePrefixPosition > -1))
    {
        // Get only the SDK version:
        sdkFullPath.getSubString(sdkNamePrefixPosition + sdkNamePrefixLength, sdkNameSuffixPosition - 1, sdkName);
    }
    else // !((sdkNameSuffixPosition > sdkNamePrefixPosition + sdkNamePrefixLength) && (sdkNamePrefixPosition > -1))
    {
        // No SDK format found in the path
        sdkName.makeEmpty();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdKernelDebuggingCurrentWorkItemAsString
// Description: Gets the current work item coordiante as a string
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        20/9/2011
// ---------------------------------------------------------------------------
bool gdKernelDebuggingCurrentWorkItemAsString(gtString& wiString)
{
    bool retVal = false;
    wiString.makeEmpty();

    // Get the indices:
    int xCoord = -2;
    int yCoord = -2;
    int zCoord = -2;
    bool rcWI = gaGetKernelDebuggingCurrentWorkItem(xCoord, yCoord, zCoord);

    if (rcWI)
    {
        retVal = true;

        // Build the string:
        if (xCoord > -1)
        {
            wiString.appendFormattedString(L"{X:%d", xCoord);

            if (yCoord > -1)
            {
                wiString.appendFormattedString(L", Y:%d", yCoord);

                if (zCoord > -1)
                {
                    wiString.appendFormattedString(L", Z:%d", zCoord);
                }
            }

            wiString.append('}');
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdIsHSAKernelName
// Description: Returns true iff a kernel should be considered an HSA kernel.
// Author:      Uri Shomroni
// Date:        18/11/2015
// ---------------------------------------------------------------------------
bool gdIsHSAKernelName(const gtString& kernelName)
{
    bool retVal = false;
#ifdef GD_ALLOW_HSA_DEBUGGING

    // HSA currently only supported on Linux:
    if (!kernelName.isEmpty())
    {
        retVal = ('&' == kernelName[0]);
    }

#elif defined (GD_DISALLOW_HSA_DEBUGGING)
    GT_UNREFERENCED_PARAMETER(kernelName);
#else
#error GD_ALLOW_HSA_DEBUGGING and GD_DISALLOW_HSA_DEBUGGING both not defined. Please include gdApplicationCommands.h!
#endif // GD_ALLOW_HSA_DEBUGGING

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdGetVBODisplayName
// Description: Creates a GL VBO's display name.
// Author:      Sigal Algranaty
// Date:        25/7/2010
// ---------------------------------------------------------------------------
void gdGetVBODisplayName(const apGLVBO& vboDetails, gtString& vboDisplayName)
{
    // Build the VBO basic name:
    GLenum lastBindTarget = vboDetails.lastBufferTarget();
    GLuint vboName = vboDetails.name();

    switch (lastBindTarget)
    {
        case GL_ARRAY_BUFFER:
            vboDisplayName.appendFormattedString(GD_STR_ImagesAndBuffersViewerGLBufferNameArray, vboName);
            break;

        case GL_DRAW_INDIRECT_BUFFER:
            vboDisplayName.appendFormattedString(GD_STR_ImagesAndBuffersViewerGLBufferNameDrawIndir, vboName);
            break;

        case GL_DISPATCH_INDIRECT_BUFFER:
            vboDisplayName.appendFormattedString(GD_STR_ImagesAndBuffersViewerGLBufferNameDispatchIndir, vboName);
            break;

        case GL_ELEMENT_ARRAY_BUFFER:
            vboDisplayName.appendFormattedString(GD_STR_ImagesAndBuffersViewerGLBufferNameElementArray, vboName);
            break;

        case GL_PIXEL_PACK_BUFFER:
            vboDisplayName.appendFormattedString(GD_STR_ImagesAndBuffersViewerGLBufferNamePixelPack, vboName);
            break;

        case GL_PIXEL_UNPACK_BUFFER:
            vboDisplayName.appendFormattedString(GD_STR_ImagesAndBuffersViewerGLBufferNamePixelUnpack, vboName);
            break;

        case GL_COPY_READ_BUFFER:
            vboDisplayName.appendFormattedString(GD_STR_ImagesAndBuffersViewerGLBufferNameCopyRead, vboName);
            break;

        case GL_COPY_WRITE_BUFFER:
            vboDisplayName.appendFormattedString(GD_STR_ImagesAndBuffersViewerGLBufferNameCopyWrite, vboName);
            break;

        case GL_TRANSFORM_FEEDBACK_BUFFER:
            vboDisplayName.appendFormattedString(GD_STR_ImagesAndBuffersViewerGLBufferNameTransformFeedback, vboName);
            break;

        case GL_UNIFORM_BUFFER:
            vboDisplayName.appendFormattedString(GD_STR_ImagesAndBuffersViewerGLBufferNameUniform, vboName);
            break;

        case GL_ATOMIC_COUNTER_BUFFER:
            vboDisplayName.appendFormattedString(GD_STR_ImagesAndBuffersViewerGLBufferNameAtomic, vboName);
            break;

        case GL_SHADER_STORAGE_BUFFER:
            vboDisplayName.appendFormattedString(GD_STR_ImagesAndBuffersViewerGLBufferNameShaderStorage, vboName);
            break;

        case GL_QUERY_BUFFER:
            vboDisplayName.appendFormattedString(GD_STR_ImagesAndBuffersViewerGLBufferNameQuery, vboName);
            break;

        case GL_TEXTURE_BUFFER:
            vboDisplayName.appendFormattedString(GD_STR_ImagesAndBuffersViewerGLBufferNameTexture, vboName);
            break;

        case GL_NONE:
            vboDisplayName.appendFormattedString(GD_STR_ImagesAndBuffersViewerGLBufferName, vboName);
            break;

        default:
#if AMDT_BUILD_TARGET == AMDT_DEBUG_BUILD
            // Unsupported buffer target:
            GT_ASSERT(false);
#endif
            vboDisplayName.appendFormattedString(GD_STR_ImagesAndBuffersViewerGLBufferNameUnknown, vboName);
            break;
    }

    // Add OpenCL interoperability:
    if (vboDetails.openCLBufferIndex() >= 0)
    {
        vboDisplayName.appendFormattedString(GD_STR_PropertiesVBOCLPosfix, vboDetails.openCLSpyID(), vboDetails.openCLBufferName());
    }
}

// ---------------------------------------------------------------------------
// Name:        gdGetBufferDisplayName
// Description: Creates a CL buffer's display name. If the buffer had a name defined
//              using clNameMemObjectGREMEDY, display it followed by a number string,
//              otherwise, display just the number string
// Author:      Uri Shomroni
// Date:        22/7/2010
// ---------------------------------------------------------------------------
void gdGetBufferDisplayName(const apCLBuffer& bufferDetails, gtString& bufferDisplayName, bool longDefaultName)
{
    const gtString& bufferNameFromNamingExtension = bufferDetails.memObjectName();

    if (bufferNameFromNamingExtension.isEmpty())
    {
        bufferDisplayName.makeEmpty().appendFormattedString(longDefaultName ? GD_STR_TexturesViewerNameCLBuffer : GD_STR_ImagesAndBuffersViewerCLBufferNameFormat, bufferDetails.bufferName());
    }
    else // !bufferNameFromNamingExtension.isEmpty()
    {
        bufferDisplayName.makeEmpty().append(bufferNameFromNamingExtension).append(L" (").appendFormattedString(longDefaultName ? GD_STR_TexturesViewerNameCLBuffer : GD_STR_ImagesAndBuffersViewerCLBufferNameFormat, bufferDetails.bufferName()).append(L")");
    }

    // Add the OpenGL shared buffer name (if relevant):
    GLuint glBufferName = bufferDetails.openGLBufferName();
    int glSpyID = bufferDetails.openGLSpyID();

    if ((glBufferName > 0) && (glSpyID > 0))
    {
        bufferDisplayName.appendFormattedString(GD_STR_PropertiesCLBufferGLPosfix, glSpyID, glBufferName);
    }

    // Add AMD ext name if exists:
    if (!bufferDetails.memObjectName().isEmpty())
    {
        bufferDisplayName.append(L" (");
        bufferDisplayName.append(bufferDetails.memObjectName());
        bufferDisplayName.append(L")");
    }

}

// ---------------------------------------------------------------------------
// Name:        gdGetImageDisplayName
// Description: Creates a CL image's display name. If the image had a name defined
//              using clNameMemObjectGREMEDY, display it followed by a number string,
//              otherwise, display just the number string
// Author:      Uri Shomroni
// Date:        25/7/2010
// ---------------------------------------------------------------------------
void gdGetImageDisplayName(const apCLImage& imageDetails, gtString& imageDisplayName, bool longDefaultName)
{
    const gtString& imageNameFromNamingExtension = imageDetails.memObjectName();

    if (imageNameFromNamingExtension.isEmpty())
    {
        imageDisplayName.makeEmpty().appendFormattedString(longDefaultName ? GD_STR_PropertiesImageNameFormat : GD_STR_PropertiesImageNameFormatShort, imageDetails.imageName());
    }
    else // !imageNameFromNamingExtension.isEmpty()
    {
        imageDisplayName.makeEmpty().append(imageNameFromNamingExtension).append(L" (").appendFormattedString(longDefaultName ? GD_STR_PropertiesImageNameFormat : GD_STR_PropertiesImageNameFormatShort, imageDetails.imageName()).append(L")");
    }
}

// ---------------------------------------------------------------------------
// Name:        gdGetPipeDisplayName
// Description: Creates a CL pipe's display name. If the pipe had a name defined
//              using clNameMemObjectGREMEDY, display it followed by a number string,
//              otherwise, display just the number string
// Author:      Uri Shomroni
// Date:        22/7/2010
// ---------------------------------------------------------------------------
void gdGetPipeDisplayName(const apCLPipe& pipeDetails, gtString& pipeDisplayName, bool longDefaultName)
{
    const gtString& pipeNameFromNamingExtension = pipeDetails.memObjectName();

    if (pipeNameFromNamingExtension.isEmpty())
    {
        pipeDisplayName.makeEmpty().appendFormattedString(longDefaultName ? GD_STR_TexturesViewerNameCLPipe : GD_STR_ImagesAndBuffersViewerCLPipeNameFormat, pipeDetails.pipeName());
    }
    else // !pipeNameFromNamingExtension.isEmpty()
    {
        pipeDisplayName.makeEmpty().append(pipeNameFromNamingExtension).append(L" (").appendFormattedString(longDefaultName ? GD_STR_TexturesViewerNameCLPipe : GD_STR_ImagesAndBuffersViewerCLPipeNameFormat, pipeDetails.pipeName()).append(L")");
    }
}


// ---------------------------------------------------------------------------
// Name:        gdOpenGLErrorToString
// Description: Inputs an Open error enum and returns it as a string.
// Author:      Yaki Tebeka
// Date:        25/8/2004
// ---------------------------------------------------------------------------
void gdOpenGLErrorToString(GLenum openGLError, gtString& errorString)
{
    // No error - 0:
    if (openGLError == GL_NO_ERROR)
    {
        errorString = GD_STR_ProcessEventsViewOGLErrorGL_NO_ERRORName;
    }
    else
    {
        // Get the enum value as string:
        bool rcGetError = apGLenumValueToString(openGLError, errorString);

        if (!rcGetError)
        {
            errorString = GD_STR_ProcessEventsViewOGLErrorUnknown;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdOpenGLErrorToDescriptionString
// Description: Inputs an Open error enum and returns a string that
//              describes it.
// Author:      Yaki Tebeka
// Date:        25/8/2004
// ---------------------------------------------------------------------------
void gdOpenGLErrorToDescriptionString(GLenum openGLError, gtString& errorString)
{
    switch (openGLError)
    {
        case GL_NO_ERROR:
            errorString = GD_STR_ProcessEventsViewOGLErrorGL_NO_ERRORDesc;
            break;

        case GL_INVALID_ENUM:
            errorString = GD_STR_ProcessEventsViewOGLErrorGL_INVALID_ENUMDesc;
            break;

        case GL_INVALID_VALUE:
            errorString = GD_STR_ProcessEventsViewOGLErrorGL_INVALID_VALUEDesc;
            break;

        case GL_INVALID_OPERATION:
            errorString = GD_STR_ProcessEventsViewOGLErrorGL_INVALID_OPERATIONDesc;
            break;

        case GL_STACK_OVERFLOW:
            errorString = GD_STR_ProcessEventsViewOGLErrorGL_STACK_OVERFLOWDesc;
            break;

        case GL_STACK_UNDERFLOW:
            errorString = GD_STR_ProcessEventsViewOGLErrorGL_STACK_UNDERFLOWDesc;
            break;

        case GL_OUT_OF_MEMORY:
            errorString = GD_STR_ProcessEventsViewOGLErrorGL_OUT_OF_MEMORYDesc;
            break;

        case GL_TABLE_TOO_LARGE:
            errorString = GD_STR_ProcessEventsViewOGLErrorGL_TABLE_TOO_LARGEDesc;
            break;

        case GL_INVALID_FRAMEBUFFER_OPERATION:
            errorString = GD_STR_ProcessEventsViewOGLErrorGL_INVALID_FRAMEBUFFER_OPERATIONDesc;
            break;

        default:
            errorString = GD_STR_ProcessEventsViewOGLErrorUnknown;
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdOpenCLErrorToString
// Description: Inputs an OpenCL error code, and returns it as string
// Arguments: int openCLError
//            gtString& errorString
// Return Val: void
// Author:      Sigal Algranaty
// Date:        21/2/2010
// ---------------------------------------------------------------------------
void gdOpenCLErrorToString(int openCLError, gtString& errorString)
{
    switch (openCLError)
    {

        case CL_SUCCESS:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_SUCCESSName;
            break;
        }

        case CL_DEVICE_NOT_FOUND:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_DEVICE_NOT_FOUNDName;
            break;
        }

        case CL_DEVICE_NOT_AVAILABLE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_DEVICE_NOT_AVAILABLEName;
            break;
        }

        case CL_COMPILER_NOT_AVAILABLE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_COMPILER_NOT_AVAILABLEName;
            break;
        }

        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_MEM_OBJECT_ALLOCATION_FAILUREName;
            break;
        }

        case CL_OUT_OF_RESOURCES:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_OUT_OF_RESOURCESName;
            break;
        }

        case CL_OUT_OF_HOST_MEMORY:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_OUT_OF_HOST_MEMORYName;
            break;
        }

        case CL_PROFILING_INFO_NOT_AVAILABLE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_PROFILING_INFO_NOT_AVAILABLEName;
            break;
        }

        case CL_MEM_COPY_OVERLAP:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_MEM_COPY_OVERLAPName;
            break;
        }

        case CL_IMAGE_FORMAT_MISMATCH:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_IMAGE_FORMAT_MISMATCHName;
            break;
        }

        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_IMAGE_FORMAT_NOT_SUPPORTEDName;
            break;
        }

        case CL_BUILD_PROGRAM_FAILURE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_BUILD_PROGRAM_FAILUREName;
            break;
        }

        case CL_MAP_FAILURE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_MAP_FAILUREName;
            break;
        }

        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_MISALIGNED_SUB_BUFFER_OFFSETName;
            break;
        }

        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LISTName;
            break;
        }

        case CL_INVALID_PROPERTY:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_PROPERTYName;
            break;
        }

        case CL_INVALID_VALUE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_VALUEName;
            break;
        }

        case CL_INVALID_DEVICE_TYPE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_DEVICE_TYPEName;
            break;
        }

        case CL_INVALID_PLATFORM:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_PLATFORMName;
            break;
        }

        case CL_INVALID_DEVICE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_DEVICEName;
            break;
        }

        case CL_INVALID_CONTEXT:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_CONTEXTName;
            break;
        }

        case CL_INVALID_QUEUE_PROPERTIES:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_QUEUE_PROPERTIESName;
            break;
        }

        case CL_INVALID_COMMAND_QUEUE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_COMMAND_QUEUEName;
            break;
        }

        case CL_INVALID_HOST_PTR:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_HOST_PTRName;
            break;
        }

        case CL_INVALID_MEM_OBJECT:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_MEM_OBJECTName;
            break;
        }

        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_IMAGE_FORMAT_DESCRIPTORName;
            break;
        }

        case CL_INVALID_IMAGE_SIZE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_IMAGE_SIZEName;
            break;
        }

        case CL_INVALID_SAMPLER:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_SAMPLERName;
            break;
        }

        case CL_INVALID_BINARY:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_BINARYName;
            break;
        }

        case CL_INVALID_BUILD_OPTIONS:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_BUILD_OPTIONSName;
            break;
        }

        case CL_INVALID_PROGRAM:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_PROGRAMName;
            break;
        }

        case CL_INVALID_PROGRAM_EXECUTABLE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_PROGRAM_EXECUTABLEName;
            break;
        }

        case CL_INVALID_KERNEL_NAME:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_KERNEL_NAMEName;
            break;
        }

        case CL_INVALID_KERNEL_DEFINITION:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_KERNEL_DEFINITIONName;
            break;
        }

        case CL_INVALID_KERNEL:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_KERNELName;
            break;
        }

        case CL_INVALID_ARG_INDEX:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_ARG_INDEXName;
            break;
        }

        case CL_INVALID_ARG_VALUE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_ARG_VALUEName;
            break;
        }

        case CL_INVALID_ARG_SIZE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_ARG_SIZEName;
            break;
        }

        case CL_INVALID_KERNEL_ARGS:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_KERNEL_ARGSName;
            break;
        }

        case CL_INVALID_WORK_DIMENSION:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_WORK_DIMENSIONName;
            break;
        }

        case CL_INVALID_WORK_GROUP_SIZE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_WORK_GROUP_SIZEName;
            break;
        }

        case CL_INVALID_WORK_ITEM_SIZE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_WORK_ITEM_SIZEName;
            break;
        }

        case CL_INVALID_GLOBAL_OFFSET:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_GLOBAL_OFFSETName;
            break;
        }

        case CL_INVALID_EVENT_WAIT_LIST:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_EVENT_WAIT_LISTName;
            break;
        }

        case CL_INVALID_EVENT:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_EVENTName;
            break;
        }

        case CL_INVALID_OPERATION:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_OPERATIONName;
            break;
        }

        case CL_INVALID_GL_OBJECT:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_GL_OBJECTName;
            break;
        }

        case CL_INVALID_BUFFER_SIZE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_BUFFER_SIZEName;
            break;
        }

        case CL_INVALID_MIP_LEVEL:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_MIP_LEVELName;
            break;
        }

        case CL_INVALID_GLOBAL_WORK_SIZE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_GLOBAL_WORK_SIZEName;
            break;
        }

        case CL_INVALID_PIPE_SIZE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_PIPE_SIZEName;
            break;
        }

        case CL_INVALID_DEVICE_QUEUE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_DEVICE_QUEUEName;
            break;
        }

        default:
            errorString = GD_STR_ProcessEventsViewOCLErrorUnknown;
            break;
    }
}


// ---------------------------------------------------------------------------
// Name:        gdOpenCLErrorToDescriptionString
// Description: Inputs an OpenCL error code, and returns the error description as string
// Arguments:   int openCLError
//              gtString& errorString
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        21/2/2010
// ---------------------------------------------------------------------------
void gdOpenCLErrorToDescriptionString(int openCLError, gtString& errorString)
{
    switch (openCLError)
    {

        case CL_SUCCESS:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_SUCCESSDescription;
            break;
        }

        case CL_DEVICE_NOT_FOUND:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_DEVICE_NOT_FOUNDDescription;
            break;
        }

        case CL_DEVICE_NOT_AVAILABLE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_DEVICE_NOT_AVAILABLEDescription;
            break;
        }

        case CL_COMPILER_NOT_AVAILABLE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_COMPILER_NOT_AVAILABLEDescription;
            break;
        }

        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_MEM_OBJECT_ALLOCATION_FAILUREDescription;
            break;
        }

        case CL_OUT_OF_RESOURCES:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_OUT_OF_RESOURCESDescription;
            break;
        }

        case CL_OUT_OF_HOST_MEMORY:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_OUT_OF_HOST_MEMORYDescription;
            break;
        }

        case CL_PROFILING_INFO_NOT_AVAILABLE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_PROFILING_INFO_NOT_AVAILABLEDescription;
            break;
        }

        case CL_MEM_COPY_OVERLAP:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_MEM_COPY_OVERLAPDescription;
            break;
        }

        case CL_IMAGE_FORMAT_MISMATCH:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_IMAGE_FORMAT_MISMATCHDescription;
            break;
        }

        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_IMAGE_FORMAT_NOT_SUPPORTEDDescription;
            break;
        }

        case CL_BUILD_PROGRAM_FAILURE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_BUILD_PROGRAM_FAILUREDescription;
            break;
        }

        case CL_MAP_FAILURE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_MAP_FAILUREDescription;
            break;
        }

        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_MISALIGNED_SUB_BUFFER_OFFSETDescription;
            break;
        }

        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LISTDescription;
            break;
        }

        case CL_INVALID_PROPERTY:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_PROPERTYDescription;
            break;
        }

        case CL_INVALID_VALUE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_VALUEDescription;
            break;
        }

        case CL_INVALID_DEVICE_TYPE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_DEVICE_TYPEDescription;
            break;
        }

        case CL_INVALID_PLATFORM:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_PLATFORMDescription;
            break;
        }

        case CL_INVALID_DEVICE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_DEVICEDescription;
            break;
        }

        case CL_INVALID_CONTEXT:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_CONTEXTDescription;
            break;
        }

        case CL_INVALID_QUEUE_PROPERTIES:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_QUEUE_PROPERTIESDescription;
            break;
        }

        case CL_INVALID_COMMAND_QUEUE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_COMMAND_QUEUEDescription;
            break;
        }

        case CL_INVALID_HOST_PTR:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_HOST_PTRDescription;
            break;
        }

        case CL_INVALID_MEM_OBJECT:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_MEM_OBJECTDescription;
            break;
        }

        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_IMAGE_FORMAT_DESCRIPTORDescription;
            break;
        }

        case CL_INVALID_IMAGE_SIZE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_IMAGE_SIZEDescription;
            break;
        }

        case CL_INVALID_SAMPLER:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_SAMPLERDescription;
            break;
        }

        case CL_INVALID_BINARY:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_BINARYDescription;
            break;
        }

        case CL_INVALID_BUILD_OPTIONS:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_BUILD_OPTIONSDescription;
            break;
        }

        case CL_INVALID_PROGRAM:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_PROGRAMDescription;
            break;
        }

        case CL_INVALID_PROGRAM_EXECUTABLE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_PROGRAM_EXECUTABLEDescription;
            break;
        }

        case CL_INVALID_KERNEL_NAME:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_KERNEL_NAMEDescription;
            break;
        }

        case CL_INVALID_KERNEL_DEFINITION:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_KERNEL_DEFINITIONDescription;
            break;
        }

        case CL_INVALID_KERNEL:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_KERNELDescription;
            break;
        }

        case CL_INVALID_ARG_INDEX:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_ARG_INDEXDescription;
            break;
        }

        case CL_INVALID_ARG_VALUE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_ARG_VALUEDescription;
            break;
        }

        case CL_INVALID_ARG_SIZE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_ARG_SIZEDescription;
            break;
        }

        case CL_INVALID_KERNEL_ARGS:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_KERNEL_ARGSDescription;
            break;
        }

        case CL_INVALID_WORK_DIMENSION:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_WORK_DIMENSIONDescription;
            break;
        }

        case CL_INVALID_WORK_GROUP_SIZE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_WORK_GROUP_SIZEDescription;
            break;
        }

        case CL_INVALID_WORK_ITEM_SIZE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_WORK_ITEM_SIZEDescription;
            break;
        }

        case CL_INVALID_GLOBAL_OFFSET:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_GLOBAL_OFFSETDescription;
            break;
        }

        case CL_INVALID_EVENT_WAIT_LIST:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_EVENT_WAIT_LISTDescription;
            break;
        }

        case CL_INVALID_EVENT:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_EVENTDescription;
            break;
        }

        case CL_INVALID_OPERATION:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_OPERATIONDescription;
            break;
        }

        case CL_INVALID_GL_OBJECT:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_GL_OBJECTDescription;
            break;
        }

        case CL_INVALID_BUFFER_SIZE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_BUFFER_SIZEDescription;
            break;
        }

        case CL_INVALID_MIP_LEVEL:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_MIP_LEVELDescription;
            break;
        }

        case CL_INVALID_GLOBAL_WORK_SIZE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_GLOBAL_WORK_SIZEDescription;
            break;
        }

        case CL_INVALID_PIPE_SIZE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_PIPE_SIZEDescription;
            break;
        }

        case CL_INVALID_DEVICE_QUEUE:
        {
            errorString = GD_STR_ProcessEventsViewOCLErrorCL_INVALID_DEVICE_QUEUEDescription;
            break;
        }

        default:
            errorString = GD_STR_ProcessEventsViewOCLErrorUnknown;
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdParameterAsString
// Description: Build a string for the function argument. If the parameter is a
//              logical parameter, add an HTML link to the string
// Arguments:   const apParameter* pParam
//            gtString& functionArgAsString
//            bool addOriginalValue
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/3/2010
// ---------------------------------------------------------------------------
bool gdParameterAsString(const apParameter* pParam, gtString& functionArgAsString, bool addOriginalValue, bool addLink)
{
    bool retVal = false;
    gtString argumentValueAsString;
    functionArgAsString.makeEmpty();
    GT_IF_WITH_ASSERT(pParam != nullptr)
    {
        retVal = true;

        // Check if this is a handle parameter:
        bool isHandleParameter = (pParam->type() == OS_TOBJ_ID_CL_HANDLE_PARAMETER);

        // Get the current argument value (as a string):
        pParam->valueAsString(argumentValueAsString);

        // Display the argument original value (for links with values, or for 'ordinary' parameters):
        if ((addOriginalValue) || !isHandleParameter)
        {
            // Add the parameter value as string:
            functionArgAsString.append(argumentValueAsString);

            if (addLink && isHandleParameter)
            {
                functionArgAsString.append(L" - ");
            }
        }

        // OpenCL handle:
        if (isHandleParameter)
        {
            apCLHandleParameter* pHandleParam = (apCLHandleParameter*)pParam;
            GT_IF_WITH_ASSERT(pHandleParam != nullptr)
            {
                gdOpenCLHandleAsString(pHandleParam->pointerValue(), functionArgAsString, addLink, true, false);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdOpenCLHandleAsString
// Description: Creates a string from an OpenCL handle and adds the original value
//              and a link upon request. The hand
// Arguments:   stringContainsFormattedHandle - should be true if the i/o parameter
//              functionArgAsString already contains the handle formatted as a pointer.
//              Otherwise, functionArgAsString should be empty.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        22/7/2010
// ---------------------------------------------------------------------------
bool gdOpenCLHandleAsString(oaCLHandle clObjectHandle, gtString& functionArgAsString, bool addLink, bool stringContainsFormattedHandle, bool useCommandQueuesViewerNameFormat)
{
    bool retVal = false;

    if (!stringContainsFormattedHandle)
    {
        GT_ASSERT(functionArgAsString.isEmpty());
        apCLHandleParameter dummyParam(clObjectHandle);
        dummyParam.valueAsString(functionArgAsString);

        if (addLink)
        {
            functionArgAsString.append(L" - ");
        }
    }

    apCLObjectID clObjectId;
    bool rc = gaGetOpenCLHandleObjectDetails(clObjectHandle, clObjectId);

    if (rc)
    {
        retVal = true;
        const gtString& clObjectName = clObjectId._objectName;
        bool clObjectHasName = !clObjectName.isEmpty();

        // Add link only for non deleted objects:
        addLink = addLink && (clObjectId._objectId >= 0);

        switch (clObjectId._objectType)
        {
            case OS_TOBJ_ID_CL_BUFFER:
            {
                if (addLink)
                {
                    // Add the link to the parameter:
                    functionArgAsString.prependFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesCLBufferLink, clObjectId._objectDisplayName, clObjectId._contextId);
                }

                if (clObjectHasName)
                {
                    functionArgAsString.append(clObjectName);

                    if (addLink)
                    {
                        functionArgAsString.append(L" - ");
                    }
                }

                if ((!clObjectHasName) || (addLink))
                {
                    functionArgAsString.appendFormattedString(useCommandQueuesViewerNameFormat ? GD_STR_PropertiesQueueCommandBufferName : GD_STR_PropertiesCLBufferName, clObjectId._objectDisplayName);
                }
            }
            break;

            case OS_TOBJ_ID_CL_SUB_BUFFER:
            {
                if (addLink)
                {
                    // Add the link to the parameter:
                    functionArgAsString.prependFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesCLSubBufferLink, clObjectId._objectDisplayName, clObjectId._contextId);
                }

                if (clObjectHasName)
                {
                    functionArgAsString.append(clObjectName);

                    if (addLink)
                    {
                        functionArgAsString.append(L" - ");
                    }
                }
            }
            break;

            case OS_TOBJ_ID_CL_IMAGE:
            {
                if (addLink)
                {
                    // Add the link to the parameter:
                    functionArgAsString.prependFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesCLImageLink, clObjectId._objectDisplayName, clObjectId._contextId);
                }

                if (clObjectHasName)
                {
                    functionArgAsString.append(clObjectName);

                    if (addLink)
                    {
                        functionArgAsString.append(L" - ");
                    }
                }

                if ((!clObjectHasName) || (addLink))
                {
                    functionArgAsString.appendFormattedString(useCommandQueuesViewerNameFormat ? GD_STR_PropertiesQueueCommandImageName : GD_STR_PropertiesCLImageName, clObjectId._objectDisplayName);
                }
            }
            break;

            case OS_TOBJ_ID_CL_PIPE:
            {
                if (addLink)
                {
                    // Add the link to the parameter:
                    functionArgAsString.prependFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesCLPipeLink, clObjectId._objectDisplayName, clObjectId._contextId);
                }

                if (clObjectHasName)
                {
                    functionArgAsString.append(clObjectName);

                    if (addLink)
                    {
                        functionArgAsString.append(L" - ");
                    }
                }

                if ((!clObjectHasName) || (addLink))
                {
                    functionArgAsString.appendFormattedString(useCommandQueuesViewerNameFormat ? GD_STR_PropertiesQueueCommandPipeName : GD_STR_PropertiesCLPipeName, clObjectId._objectDisplayName);
                }
            }
            break;

            case OS_TOBJ_ID_CL_PROGRAM:
            {
                if (addLink)
                {
                    // Add the link to the parameter:
                    functionArgAsString.prependFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesCLProgramLink, clObjectId._objectDisplayName, clObjectId._contextId);
                }

                if (clObjectHasName)
                {
                    functionArgAsString.append(clObjectName);

                    if (addLink)
                    {
                        functionArgAsString.append(L" - ");
                    }
                }

                if ((!clObjectHasName) || (addLink))
                {
                    functionArgAsString.appendFormattedString(GD_STR_PropertiesCLProgramlName, clObjectId._objectId + 1);
                }
            }
            break;

            case OS_TOBJ_ID_CL_KERNEL:
            {
                if (addLink)
                {
                    // Add the link to the parameter:
                    functionArgAsString.prependFormattedString(AF_STR_HtmlPropertiesLink3ParamStart, GD_STR_HtmlPropertiesCLKernelLink, clObjectId._objectDisplayName, clObjectId._contextId, clObjectId._ownerObjectId);
                }

                if (clObjectHasName)
                {
                    functionArgAsString.append(clObjectName);

                    if (addLink)
                    {
                        functionArgAsString.append(L" - ");
                    }
                }

                if ((!clObjectHasName) || (addLink))
                {
                    functionArgAsString.appendFormattedString(useCommandQueuesViewerNameFormat ? GD_STR_PropertiesQueueCommandKernelName : GD_STR_PropertiesCLProgramlNameShort AF_STR_Space GD_STR_PropertiesCLKernelName, clObjectId._ownerObjectId + 1, clObjectId._objectDisplayName);
                }
            }
            break;

            case OS_TOBJ_ID_CL_EVENT:
            {
                // If the event still exists:
                if (clObjectId._objectId > -1)
                {
                    if (addLink)
                    {
                        // Add the link to the parameter:
                        functionArgAsString.prependFormattedString(AF_STR_HtmlPropertiesLink3ParamStart, GD_STR_HtmlPropertiesCLEventLink, clObjectId._objectDisplayName, clObjectId._contextId, clObjectId._ownerObjectId);
                    }

                    if (clObjectHasName)
                    {
                        functionArgAsString.append(clObjectName);

                        if (addLink)
                        {
                            functionArgAsString.append(L" - ");
                        }
                    }

                    if ((!clObjectHasName) || (addLink))
                    {
                        functionArgAsString.appendFormattedString(GD_STR_PropertiesCLQueueNameShort, clObjectId._ownerObjectId + 1);
                        functionArgAsString.appendFormattedString(AF_STR_Space);
                        functionArgAsString.appendFormattedString(useCommandQueuesViewerNameFormat ? GD_STR_PropertiesQueueCommandEventName : GD_STR_PropertiesCLEventName, clObjectId._objectDisplayName);
                    }
                }
            }
            break;

            case OS_TOBJ_ID_CL_SAMPLER:
            {
                if (addLink)
                {
                    // Add the link to the parameter:
                    functionArgAsString.prependFormattedString(AF_STR_HtmlPropertiesLink2ParamStart, GD_STR_HtmlPropertiesCLSamplerLink, clObjectId._contextId, clObjectId._objectDisplayName);
                }

                if (clObjectHasName)
                {
                    functionArgAsString.append(clObjectName);

                    if (addLink)
                    {
                        functionArgAsString.append(L" - ");
                    }
                }

                if ((!clObjectHasName) || (addLink))
                {
                    functionArgAsString.appendFormattedString(GD_STR_PropertiesCLSamplerName, clObjectId._objectDisplayName);
                }
            }
            break;

            case OS_TOBJ_ID_CL_DEVICE:
            {
                if (addLink)
                {
                    // Add the link to the parameter:
                    functionArgAsString.prependFormattedString(AF_STR_HtmlPropertiesLink1ParamStart, GD_STR_HtmlPropertiesCLDeviceLink, clObjectId._objectDisplayName);
                }

                if (clObjectHasName)
                {
                    functionArgAsString.append(clObjectName);

                    if (addLink)
                    {
                        functionArgAsString.append(L" - ");
                    }
                }

                if ((!clObjectHasName) || (addLink))
                {
                    functionArgAsString.appendFormattedString(GD_STR_PropertiesDeviceTitle, apCLDeviceIndexToDisplayIndex(clObjectId._objectId));
                }
            }
            break;

            case OS_TOBJ_ID_CL_CONTEXT:
            {
                if (addLink)
                {
                    // Add the link to the parameter:
                    functionArgAsString.prependFormattedString(AF_STR_HtmlPropertiesLink1ParamStart, GD_STR_HtmlPropertiesCLContextLink, clObjectId._contextId);
                }

                if (clObjectHasName)
                {
                    functionArgAsString.append(clObjectName);

                    if (addLink)
                    {
                        functionArgAsString.append(L" - ");
                    }
                }

                if ((!clObjectHasName) || (addLink))
                {
                    functionArgAsString.appendFormattedString(GD_STR_PropertiesCLContextName, clObjectId._contextId);
                }
            }
            break;

            default:
            {
                // TO_DO: OpenCL handle all link objects:
                retVal = false;
            }
            break;
        }

        // If the object was deleted:
        if (clObjectId._objectId <= -1)
        {
            functionArgAsString.append(AF_STR_Space);
            functionArgAsString.append(AF_STR_Hyphen);
            functionArgAsString.append(AF_STR_Space AF_STR_Deleted);
        }
    }

    // If the link is not empty, add the HTML tag for the link (if requested):
    if (retVal && addLink)
    {
        functionArgAsString.append(GD_STR_HtmlPropertiesLinkEnd);
    }

    // If, by some odd failing, the string is still empty, simply format the handle as a pointer:
    if (functionArgAsString.isEmpty())
    {
        gdUserApplicationAddressToDisplayString((osProcedureAddress64)clObjectHandle, functionArgAsString);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdFunctionCallAsString
// Description: Convert a function call to a string
// Arguments:   const apFunctionCall* pFunctionCall
//              gtString& functionCallAsString
// TO_DO: implement logical parameters also for OpenGL function parameters
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/3/2010
// ---------------------------------------------------------------------------
bool gdFunctionCallAsString(const apFunctionCall* pFunctionCall, gtString& functionCallAsString, apContextType functionContextType)
{
    bool retVal = false;

    functionCallAsString.makeEmpty();
    GT_IF_WITH_ASSERT(pFunctionCall != nullptr)
    {
        retVal = true;

        if (functionContextType == AP_OPENGL_CONTEXT)
        {
            pFunctionCall->asString(functionCallAsString);
        }
        else
        {
            // Get the function name:
            static apMonitoredFunctionsManager& monitoredFuncMgr = apMonitoredFunctionsManager::instance();
            functionCallAsString += monitoredFuncMgr.monitoredFunctionName(pFunctionCall->functionId());

            // Start the function arguments section:
            functionCallAsString += '(';

            // Iterate the function arguments:
            const gtList<const apParameter*>& funcArguments = pFunctionCall->arguments();
            gtList<const apParameter*>::const_iterator iter = funcArguments.begin();
            gtList<const apParameter*>::const_iterator endIter = funcArguments.end();

            gtString currentArgumentValueAsString;

            while (iter != endIter)
            {
                // Get the current parameter as string:
                const apParameter* pParameter = (*iter);
                GT_IF_WITH_ASSERT(pParameter != nullptr)
                {
                    bool rc = gdParameterAsString(*iter, currentArgumentValueAsString, false, false);
                    GT_ASSERT(rc);
                }

                // Add it to the function call string:
                functionCallAsString += currentArgumentValueAsString;
                iter++;

                // Add the "," only if it is NOT the last parameter
                if (iter != endIter)
                {
                    functionCallAsString += L", ";
                }
            }

            // End the function arguments section:
            functionCallAsString += L")";
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdIsDebugging64BitApplication
// Description: Returns true iff the current selected application is a 64-bit application
// Author:      Uri Shomroni
// Date:        3/5/2012
// ---------------------------------------------------------------------------
bool gdIsDebugging64BitApplication()
{
    bool retVal = false;

    retVal = gaIsDebugging64BitApplication();

    // The below method does not work with remote debugging, where the path might not exist on the client machine:
    /*
    const osFilePath& currentExecutablePath = afProjectManager::instance().currentProjectSettings().executablePath();

    bool rc = osIs64BitModule(currentExecutablePath, retVal);
    GT_ASSERT(rc);

    retVal = retVal && rc
    */

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdUserApplicationAddressToDisplayString
// Description: Outputs pointer as a string (like "%p") appropriate in length
//              to the debugged process' address space size.
// Author:      Uri Shomroni
// Date:        21/9/2009
// ---------------------------------------------------------------------------
void gdUserApplicationAddressToDisplayString(osProcedureAddress64 pointer, gtString& outputString, bool inUppercase)
{
    outputString.makeEmpty();

    // Get the executable type:
    bool is64BitApp = gdIsDebugging64BitApplication();

    osProcedureAddressToString(pointer, is64BitApp, inUppercase, outputString);
}

// ---------------------------------------------------------------------------
// Name:        gdDoesModeChangeApplyToDebuggerViews
// Description: Returns true if the mode change is relevant to debugger views.
//              If so, enableView will be changed to reflect whether the change
//              means the debugger should enable its views or disable them.
// Author:      Uri Shomroni
// Date:        29/7/2014
// ---------------------------------------------------------------------------
bool gdDoesModeChangeApplyToDebuggerViews(const apEvent& eve, bool& enableView)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(eve.eventType() == apEvent::AP_EXECUTION_MODE_CHANGED_EVENT)
    {
        const apExecutionModeChangedEvent& execModeEve = (const apExecutionModeChangedEvent&)eve;
        const gtString& modeType = execModeEve.modeType();
        static const gtString debugModeType = GD_STR_executionMode;

        if (debugModeType == modeType)
        {
            // Changing to debug mode:
            retVal = true;
            enableView = true;
        }
        else if (!modeType.isEmpty() && !execModeEve.onlySessionTypeIndex())
        {
            // Changing to another mode:
            retVal = true;
            enableView = false;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdSearchForKernelDeclarationInSourceFile
// Description: Searches for a kernel declaration in a source file
// Author:      Uri Shomroni
// Date:        19/12/2010
// ---------------------------------------------------------------------------
GD_API int gdSearchForKernelDeclarationInSourceFile(const osFilePath& sourceCodeFilePath, const gtString& kernelFuncName)
{
    int retVal = -1;

    // Open the file for reading:
    osFile programSourceCodeAsFile(sourceCodeFilePath);
    bool rcOpen = programSourceCodeAsFile.open(osChannel::OS_ASCII_TEXT_CHANNEL);
    GT_IF_WITH_ASSERT(rcOpen)
    {
        // Read the source code into a string:
        gtASCIIString programSourceAsASCII;
        bool rcRead = programSourceCodeAsFile.readIntoString(programSourceAsASCII);
        GT_IF_WITH_ASSERT(rcRead)
        {
            // Convert to a gtString:
            gtString programSource;
            programSource.fromASCIIString(programSourceAsASCII.asCharArray());
            int programSourceLength = programSource.length();

            // Look for the kernel name between the kernel declaration modifier and opening parentheses:
            static const gtString kernelDeclaration = AF_STR_CLKernelDeclaration;
            static const int kernelDeclarationLength = kernelDeclaration.length();
            static const gtString kernelAlternativeDeclaration = AF_STR_CLKernelDeclarationAlternative;
            static const int kernelAlternativeDeclarationLength = kernelAlternativeDeclaration.length();
            static const gtString kernelReturnType = AF_STR_CLKernelReturnType;
            static const gtString charactersAllowedInSymbols = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_";
            bool goOn = true;
            int currPosition = -1;
            int foundPosition = -1;

            while (goOn)
            {
                int declarationPosition = programSource.find(kernelDeclaration, currPosition + 1);
                int alternativePosition = programSource.find(kernelAlternativeDeclaration, currPosition + 1);
                currPosition = -1;

                // Make sure that the declarations we found are not part of symbols:
                if (declarationPosition > 0)
                {
                    // No character before that can be part of a symbol:
                    const wchar_t& charBeforeDecl = programSource[declarationPosition - 1];

                    if ((charactersAllowedInSymbols.find(charBeforeDecl) == -1) &&
                        (declarationPosition < (programSourceLength - kernelDeclarationLength - 1)))
                    {
                        // No character after that can be part of a symbol:
                        const wchar_t& charAfterDecl = programSource[declarationPosition + kernelDeclarationLength];

                        if (charactersAllowedInSymbols.find(charAfterDecl) == -1)
                        {
                            currPosition = declarationPosition;
                        }
                    }
                }

                // We check the alternative after the "normal" one, since it's a substring of it:
                if ((alternativePosition > 0) && ((alternativePosition < declarationPosition) || currPosition == -1))
                {
                    // No character before that can be part of a symbol:
                    const wchar_t& charBeforeAlt = programSource[alternativePosition - 1];

                    if ((charactersAllowedInSymbols.find(charBeforeAlt) == -1) &&
                        (alternativePosition < (programSourceLength - kernelAlternativeDeclarationLength - 1)))
                    {
                        // No character after that can be part of a symbol:
                        const wchar_t& charAfterAlt = programSource[alternativePosition + kernelAlternativeDeclarationLength];

                        if (charactersAllowedInSymbols.find(charAfterAlt) == -1)
                        {
                            currPosition = alternativePosition;
                        }
                    }
                }

                if (currPosition > -1)
                {
                    // Find the next opening parentheses and the next instance of the kernel name:
                    int returnTypePosition = programSource.find(kernelReturnType, currPosition);

                    if (returnTypePosition > currPosition)
                    {
                        // Get the next parentheses position:
                        int parenthesesPosition = programSource.find('(', returnTypePosition);
                        int kernelNamePosition = programSource.find(kernelFuncName, currPosition);

                        if ((parenthesesPosition > kernelNamePosition) && (kernelNamePosition > -1))
                        {
                            // Check that there are only white spaces between the kernel name and parentheses:
                            bool onlySpaces = true;
                            int kernelNameEnd = kernelNamePosition + kernelFuncName.length();

                            for (int i = kernelNameEnd; i < parenthesesPosition; i++)
                            {
                                // Get the current character:
                                char ch = programSource[i];
                                onlySpaces = onlySpaces && isspace(ch);

                                if (!onlySpaces)
                                {
                                    break;
                                }
                            }

                            if (onlySpaces)
                            {
                                // If the kernel name is before the parentheses, we found the kernel function, so stop:
                                foundPosition = kernelNamePosition;
                                break;
                            }
                        }
                    }
                }
                else // currPosition <= -1
                {
                    goOn = false;
                }
            }

            // If we found the kernel function:
            if (foundPosition > -1)
            {
                // Find which line is it in:
                retVal = programSource.lineNumberFromCharacterIndex(foundPosition);
            }
        }

        // Close the file:
        programSourceCodeAsFile.close();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gwSourceCodeViewsManager::findObjectFilePath
// Description: Get the requested object file path
// Arguments:   const gdDebugApplicationTreeData* pItemData
//              osFilePath& filePath
//              lineNumber - the output line number
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/8/2011
// ---------------------------------------------------------------------------
bool gdFindObjectFilePath(const afApplicationTreeItemData* pItemData, osFilePath& filePath, int& lineNumber)
{
    bool retVal = true;

    // By default do not select a source code line:
    lineNumber = 0;

    // Sanity check:
    GT_IF_WITH_ASSERT(pItemData != nullptr)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData !=  nullptr)
        {
            switch (pItemData->m_itemType)
            {
                case AF_TREE_ITEM_CL_KERNEL:
                {
                    // Get the OpenCL kernel details:
                    apCLKernel kernelDetails(OA_CL_NULL_HANDLE, -1, OA_CL_NULL_HANDLE, AF_STR_Empty);
                    bool rc = gaGetOpenCLKernelObjectDetails(pGDData->_contextId._contextId, pGDData->_clKernelHandle, kernelDetails);

                    if (rc)
                    {
                        // Get the kernel name:
                        gtString kernelFuncName = kernelDetails.kernelFunctionName();

                        // The program index is the kernel's program index:
                        int progIndex = kernelDetails.programIndex();
                        apCLProgram programDetails(0);
                        rc = gaGetOpenCLProgramObjectDetails(pGDData->_contextId._contextId, progIndex, programDetails);
                        GT_IF_WITH_ASSERT(rc)
                        {
                            filePath = programDetails.sourceCodeFilePath();
                            gaRemoteToLocalFile(filePath, true);
                        }

                        // If we found a file to open:
                        if (filePath.exists())
                        {
                            // If we have a kernel function name, try and find it in the source:
                            if (!kernelFuncName.isEmpty())
                            {
                                lineNumber = gdSearchForKernelDeclarationInSourceFile(filePath, kernelFuncName);
                                lineNumber --;
                            }
                        }
                    }
                }
                break;

                case AF_TREE_ITEM_CL_PROGRAM:
                {
                    // Get the program details:
                    apCLProgram programDetails(0);
                    bool rc = gaGetOpenCLProgramObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, programDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        filePath = programDetails.sourceCodeFilePath();
                        gaRemoteToLocalFile(filePath, true);
                    }
                }
                break;

                case AF_TREE_ITEM_GL_VERTEX_SHADER:
                case AF_TREE_ITEM_GL_TESSELLATION_CONTROL_SHADER:
                case AF_TREE_ITEM_GL_TESSELLATION_EVALUATION_SHADER:
                case AF_TREE_ITEM_GL_GEOMETRY_SHADER:
                case AF_TREE_ITEM_GL_FRAGMENT_SHADER:
                case AF_TREE_ITEM_GL_COMPUTE_SHADER:
                case AF_TREE_ITEM_GL_UNSUPPORTED_SHADER:
                {
                    // This is a V/G/F Shader - show the source code window
                    gtAutoPtr<apGLShaderObject> aptrShaderDetails = nullptr;
                    bool rcShad = gaGetShaderObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenGLName, aptrShaderDetails);

                    GT_IF_WITH_ASSERT(rcShad && (aptrShaderDetails.pointedObject() != nullptr))
                    {
                        // Get the shader file path, and make sure the file exists locally:
                        filePath = aptrShaderDetails->sourceCodeFilePath();
                    }

                    // check if the file exists:
                    retVal = filePath.isRegularFile();
                }
                break;

                default:
                {
                    GT_ASSERT_EX(false, L"Unsupported object in source code view");
                    retVal = false;
                    break;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointAsString
// Description: Convert the apBreakpintObject to a string
// Arguments:   const gtAutoPtr<apBreakPoint>& aptrBreakpoint
//              gtString& breakpointName
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/9/2011
// ---------------------------------------------------------------------------
bool gdBreakpointAsString(const gtAutoPtr<apBreakPoint>& aptrBreakpoint, gtString& breakpointName)
{
    bool retVal = false;

    // Get the breakpoint type:
    osTransferableObjectType breakpointType = aptrBreakpoint->type();

    // Get the string by breakpoint type:
    switch (breakpointType)
    {
        case OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT:
        {
            // Down cast it to apMonitoredFunctionBreakPoint:
            apMonitoredFunctionBreakPoint* pFunctionBreakpoint = (apMonitoredFunctionBreakPoint*)(aptrBreakpoint.pointedObject());
            GT_IF_WITH_ASSERT(pFunctionBreakpoint != nullptr)
            {
                // Get the function name:
                retVal = gaGetMonitoredFunctionName((apMonitoredFunctionId)pFunctionBreakpoint->monitoredFunctionId(), breakpointName);
            }
        }
        break;

        case OS_TOBJ_ID_GENERIC_BREAKPOINT:
        {
            // Down cast it to apMonitoredFunctionBreakPoint:
            apGenericBreakpoint* pGenericBreakpoint = (apGenericBreakpoint*)(aptrBreakpoint.pointedObject());
            GT_IF_WITH_ASSERT(pGenericBreakpoint != nullptr)
            {
                // Convert the integer into a breakpoint type enumeration:
                apGenericBreakpointType breakType = pGenericBreakpoint->breakpointType();

                // Get the breakpoint name:
                retVal = apGenericBreakpoint::breakpointTypeToString(breakType, breakpointName);
            }
        }
        break;

        case OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT:
        {
            // Down cast it to apKernelSourceCodeBreakpoint:
            apKernelSourceCodeBreakpoint* pKernelBreakpoint = (apKernelSourceCodeBreakpoint*)(aptrBreakpoint.pointedObject());
            GT_IF_WITH_ASSERT(pKernelBreakpoint != nullptr)
            {
                if (pKernelBreakpoint->isUnresolved() || (pKernelBreakpoint->programHandle() == OA_CL_NULL_HANDLE))
                {
                    // Build the kernel string from the unresolved path:

                    // Find the resolved file path from the CL program handle:
                    osFilePath filePath = pKernelBreakpoint->unresolvedPath();
                    int sourceLine = pKernelBreakpoint->lineNumber();

                    // Get the file name:
                    gtString fileName;
                    filePath.getFileNameAndExtension(fileName);
                    breakpointName.appendFormattedString(L"%ls, %d", fileName.asCharArray(), sourceLine);
                    retVal = true;
                }
                else
                {
                    // Find the resolved file path from the CL program handle:
                    osFilePath filePath;
                    int sourceLine;
                    bool rc = gaCodeLocationFromKernelSourceBreakpoint(*pKernelBreakpoint, filePath, sourceLine);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        // Get the file name:
                        gtString fileName;
                        filePath.getFileNameAndExtension(fileName);
                        breakpointName.appendFormattedString(L"%ls, %d", fileName.asCharArray(), sourceLine);
                        retVal = true;
                    }
                }
            }
        }
        break;

        case OS_TOBJ_ID_SOURCE_CODE_BREAKPOINT:
        {
            // Down cast it to apSourceCodeBreakpoint:
            apSourceCodeBreakpoint* pSourceCodeBreakpoint = (apSourceCodeBreakpoint*)(aptrBreakpoint.pointedObject());
            GT_IF_WITH_ASSERT(pSourceCodeBreakpoint != nullptr)
            {
                // Get the file path:
                osFilePath filePath = pSourceCodeBreakpoint->filePath();

                // Get the line number:
                int sourceLine = pSourceCodeBreakpoint->lineNumber();

                // Get the file name:
                gtString fileName;
                filePath.getFileNameAndExtension(fileName);
                breakpointName.appendFormattedString(L"%ls, %d", fileName.asCharArray(), sourceLine);
                retVal = true;
            }
        }
        break;

        case OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT:
        {
            // Down cast it to apKernelFunctionNameBreakpoint:
            apKernelFunctionNameBreakpoint* pFunctionNameBreakpoint = (apKernelFunctionNameBreakpoint*)(aptrBreakpoint.pointedObject());
            GT_IF_WITH_ASSERT(pFunctionNameBreakpoint != nullptr)
            {
                // Get the kernel function's name:
                breakpointName = pFunctionNameBreakpoint->kernelFunctionName();

                // Add a prefix so we'll know it's a kernel:
                breakpointName.prepend(gdIsHSAKernelName(breakpointName) ? GD_STR_HSAKernelFunctionNameBreakpointPrefix : GD_STR_KernelFunctionNameBreakpointPrefix);
                retVal = true;
            }
        }
        break;

        case OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT:
        {
            // Down cast it to apHostSourceCodeBreakpoint:
            apHostSourceCodeBreakpoint* pSourceCodeBreakpoint = (apHostSourceCodeBreakpoint*)(aptrBreakpoint.pointedObject());
            GT_IF_WITH_ASSERT(pSourceCodeBreakpoint != nullptr)
            {
                // Get the file path:
                osFilePath filePath = pSourceCodeBreakpoint->filePath();

                // Get the line number:
                int sourceLine = pSourceCodeBreakpoint->lineNumber();

                // Get the file name:
                gtString fileName;
                filePath.getFileNameAndExtension(fileName);
                breakpointName.appendFormattedString(L"%ls, %d", fileName.asCharArray(), sourceLine);
                retVal = true;
            }
        }
        break;

        default:
        {
            retVal = false;
        }
        break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdFindSourceCodeBreakpointMathingProgramHandle
// Description: Find within the existing OpenCL program objects, a CL handle connected
//              to the input kernel source code breakpoint
// Arguments:   const apKernelSourceCodeBreakpoint& kernelSourceCodeBP
// Return Val:  oaCLHandle - the program handle
// Author:      Sigal Algranaty
// Date:        19/7/2012
// ---------------------------------------------------------------------------
oaCLHandle gdFindSourceCodeBreakpointMathingProgramHandle(const apKernelSourceCodeBreakpoint& kernelSourceCodeBP)
{
    oaCLHandle retVal = OA_CL_NULL_HANDLE;

    // Get the amount of CL contexts:
    int currentContextsAmount = 0;
    bool rc = gaGetAmountOfOpenCLContexts(currentContextsAmount);

    // Do not throw an exception, when there's no OpenCL spy, this function fails:
    if (rc)
    {
        for (int i = 1; i < currentContextsAmount; i++)
        {
            // Get the current context programs amount:
            int amountOfPrograms = 0;
            rc = gaGetAmountOfOpenCLProgramObjects(i, amountOfPrograms);

            if (rc)
            {
                for (int k = 0; k < amountOfPrograms; k++)
                {
                    // Go through the current cl program objects and find a program matching for the requested source file path:
                    apCLProgram programDetails(0);
                    rc = gaGetOpenCLProgramObjectDetails(i, k, programDetails);

                    osFilePath programLocalFilePath = programDetails.sourceCodeFilePath();
                    bool rcRtL = gaRemoteToLocalFile(programLocalFilePath, true);
                    GT_ASSERT(rcRtL);

                    if (programLocalFilePath == kernelSourceCodeBP.unresolvedPath())
                    {
                        retVal = programDetails.programHandle();
                        break;
                    }
                }
            }

            if (retVal != OA_CL_NULL_HANDLE)
            {
                break;
            }
        }
    }

    return retVal;
}


