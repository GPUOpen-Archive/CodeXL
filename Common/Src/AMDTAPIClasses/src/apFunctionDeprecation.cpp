//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apFunctionDeprecation.cpp
///
//==================================================================================

//------------------------------ apFunctionDeprecation.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>
#include <AMDTAPIClasses/Include/apFunctionDeprecation.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>


// ---------------------------------------------------------------------------
// Name:        apFunctionDeprecationStatusToString
// Description: Translate a function deprecation status to string
// Arguments: apFunctionDeprecationStatus status
//            gtString& statusAsStr
// Return Val: AP_API bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        4/3/2009
// ---------------------------------------------------------------------------
bool apFunctionDeprecationStatusToString(apFunctionDeprecationStatus status, gtString& statusAsStr)
{
    bool retVal = true;

    switch (status)
    {
        case AP_DEPRECATION_NONE:
            statusAsStr = AP_STR_DeprecatedNone;
            break;

        case AP_DEPRECATION_FULL:
            statusAsStr = AP_STR_DeprecatedFull;
            break;

        case AP_DEPRECATION_COLOR_INDEX_MODE:
            statusAsStr = AP_STR_DeprecatedIndexMode;
            break;

        case AP_DEPRECATION_IMMEDIATE_MODE:
            statusAsStr = AP_STR_DeprecatedImmediateMode;
            break;

        case AP_DEPRECATION_DISPLAY_LISTS:
            statusAsStr = AP_STR_DeprecatedDisplayLists;
            break;

        case AP_DEPRECATION_ATTRIBUTE_STACKS_FUNCTION:
        case AP_DEPRECATION_ATTRIBUTE_STACKS_STATE:
            statusAsStr = AP_STR_DeprecatedAttributeStacks;
            break;

        case AP_DEPRECATION_PIXEL_FORMAT:
            statusAsStr = AP_STR_DeprecatedPixelFormatArgValue;
            break;

        case AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING:
            statusAsStr = AP_STR_DeprecatedPipelineVertexArgValue;
            break;

        case AP_DEPRECATION_APPLICATION_GENERATED_NAMES:
            statusAsStr = AP_STR_DeprecatedApplicationGeneratedNames;
            break;

        case AP_DEPRECATION_RECTANGLE:
            statusAsStr = AP_STR_DeprecatedRectangle;
            break;

        case AP_DEPRECATION_RASTER_POS_FUNCTION:
        case AP_DEPRECATION_RASTER_POS_STATE:
            statusAsStr = AP_STR_DeprecatedRasterPos;
            break;

        case AP_DEPRECATION_SEPERATE_POLYGON_DRAW_MODE:
            statusAsStr = AP_STR_DeprecatedSeperatePolygonDrawMode;
            break;

        case AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_FUNCTION:
        case AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_STATE:
            statusAsStr = AP_STR_DeprecatedPolygonLineStipple;
            break;

        case AP_DEPRECATION_BITMAP:
            statusAsStr = AP_STR_DeprecatedBitmap;
            break;

        case AP_DEPRECATION_PIXEL_DRAWING:
            statusAsStr = AP_STR_DeprecatedPixelDrawing;
            break;

        case AP_DEPRECATION_TEXTURE_CLAMP_WRAP_MODE:
            statusAsStr = AP_STR_DeprecatedTextureClampWrapMode;
            break;

        case AP_DEPRECATION_TEXTURE_AUTO_MIPMAP:
            statusAsStr = AP_STR_DeprecatedTextureMipmapAutoGenerate;
            break;

        case AP_DEPRECATION_ALPHA_TEST_FUNCTION:
        case AP_DEPRECATION_ALPHA_TEST_STATE:
            statusAsStr = AP_STR_DeprecatedAlphaTest;
            break;

        case AP_DEPRECATION_EVALUATORS_FUNCTION:
        case AP_DEPRECATION_EVALUATORS_STATE:
            statusAsStr = AP_STR_DeprecatedEvaluators;
            break;

        case AP_DEPRECATION_FEEDBACK:
            statusAsStr = AP_STR_DeprecatedFeedback;
            break;

        case AP_DEPRECATION_HINTS:
            statusAsStr = AP_STR_DeprecatedHints;
            break;

        case AP_DEPRECATION_NON_SPRITE_POINTS:
            statusAsStr = AP_STR_DeprecatedNonSpritePoints;
            break;

        case AP_DEPRECATION_LINE_WIDTH:
            statusAsStr = AP_STR_DeprecatedLineWidth;
            break;

        case AP_DEPRECATION_TEXTURE_BORDER:
            statusAsStr = AP_STR_DeprecatedTextureBorder;
            break;

        case AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_FUNCTION:
        case AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE:
            statusAsStr = AP_STR_DeprecatedFixedFunctionFragmentProcessing;
            break;

        case AP_DEPRECATION_ACCUMMULATION_BUFFERS_FUNCTION:
        case AP_DEPRECATION_ACCUMMULATION_BUFFERS_STATE:
            statusAsStr = AP_STR_DeprecatedAccumulationBuffers;
            break;

        case AP_DEPRECATION_FRAMEBUFFER_SIZE_QUERIES:
            statusAsStr = AP_STR_DeprecatedFramebufferSizeQueries;
            break;

        case AP_DEPRECATION_PIXEL_COPY:
            statusAsStr = AP_STR_DeprecatedPixelCopy;
            break;

        case AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES:
            statusAsStr = AP_STR_DeprecatedQuadPolygonPrimitives;
            break;

        case AP_DEPRECATION_UNIFIED_EXTENSION_STRING:
            statusAsStr = AP_STR_DeprecatedUnifiedExtensionString;
            break;

        case AP_DEPRECATION_AUXILIRY_BUFFERS:
            statusAsStr = AP_STR_DeprecatedAuxiliryBuffers;
            break;

        case AP_DEPRECATION_MAX_VARYING:
            statusAsStr = AP_STR_DeprecatedMaxVarying;
            break;

        case AP_DEPRECATION_CLIENT_VERTEX_AND_INDEX_ARRAYS:
            statusAsStr = AP_STR_DeprecatedClientVertexAndIndexArrays;
            break;

        case AP_DEPRECATION_PIXEL_TRANSFER:
            statusAsStr = AP_STR_DeprecatedPixelTransfer;
            break;

        case AP_DEPRECATION_GLSL_VERSION:
            statusAsStr = AP_STR_DeprecatedGLSLVersion;
            break;

        case AP_DEPRECATION_COMPRESSED_TEXTURE_FORMATS:
            statusAsStr = AP_STR_DeprecatedCompressedTextureFormats;
            break;

        case AP_DEPRECATION_LSB_FIRST_PIXEL_PACKING:
            statusAsStr = AP_STR_DeprecatedLSBFirstPixelPacking;
            break;

        default:
            GT_ASSERT_EX(false, L"Unsupported Deprecation Status");
            retVal = false;
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apFunctionDeprecation::apFunctionDeprecation
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        3/3/2009
// ---------------------------------------------------------------------------
apFunctionDeprecation::apFunctionDeprecation()
    : _status(AP_DEPRECATION_NONE), _argumentIndex(-1)
{
}


// ---------------------------------------------------------------------------
// Name:        apFunctionDeprecation::apFunctionDeprecation
// Description: Copy Constructor
// Arguments: const apFunctionDeprecation& other
// Author:  AMD Developer Tools Team
// Date:        9/3/2009
// ---------------------------------------------------------------------------
apFunctionDeprecation::apFunctionDeprecation(const apFunctionDeprecation& other)
{
    this->_status = other._status;
    this->_reasonStr = other._reasonStr;
    this->_argumentIndex = other._argumentIndex;
}

// ---------------------------------------------------------------------------
// Name:        apFunctionDeprecation::~apFunctionDeprecation
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        3/3/2009
// ---------------------------------------------------------------------------
apFunctionDeprecation::~apFunctionDeprecation()
{
}



// ---------------------------------------------------------------------------
// Name:        apFunctionDeprecation::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        3/3/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apFunctionDeprecation::type() const
{
    return OS_TOBJ_ID_FUNCTION_DEPRECATION;
}

// ---------------------------------------------------------------------------
// Name:        apFunctionDeprecation::writeSelfIntoChannel
// Description: Writes this class into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        3/3/2009
// ---------------------------------------------------------------------------
bool apFunctionDeprecation::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the deprecation status:
    ipcChannel << (gtInt32)_status;

    // Write the deprecation reason string:
    ipcChannel << _reasonStr;

    // Write the function deprecated argument index:
    ipcChannel << (gtInt32)_argumentIndex;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apFunctionDeprecation::readSelfFromChannel
// Description: Reads self from an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        3/3/2009
// ---------------------------------------------------------------------------
bool apFunctionDeprecation::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the deprecation status:
    gtInt32 functionStatusAsInt;
    ipcChannel >> functionStatusAsInt;
    _status = (apFunctionDeprecationStatus)functionStatusAsInt;

    // Read the deprecation reason string:
    ipcChannel >> _reasonStr;

    // Read the function deprecated argument index:
    gtInt32 argumentIndexAsInt32 = 0;
    ipcChannel >> argumentIndexAsInt32;
    _argumentIndex = (int)argumentIndexAsInt32;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apFunctionDeprecation::toString
// Description: Translate a deprecation details to string
// Arguments: gtString& str
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        4/3/2009
// ---------------------------------------------------------------------------
bool apFunctionDeprecation::toString(gtString& str)
{
    bool retVal = true;

    switch (_status)
    {
        case AP_DEPRECATION_NONE:
            str = AP_STR_DeprecatedNoneReason;
            break;

        case AP_DEPRECATION_FULL:
            str = AP_STR_DeprecatedFullReason;
            break;

        case AP_DEPRECATION_COLOR_INDEX_MODE:
            str = AP_STR_DeprecatedIndexModeReason;
            break;

        case AP_DEPRECATION_IMMEDIATE_MODE:
            str = AP_STR_DeprecatedImmediateModeReason;
            break;

        case AP_DEPRECATION_DISPLAY_LISTS:
            str = AP_STR_DeprecatedDisplayListsReason;
            break;

        case AP_DEPRECATION_RECTANGLE:
            str = AP_STR_DeprecatedRectanglesReason;
            break;

        case AP_DEPRECATION_RASTER_POS_FUNCTION:
            str = AP_STR_DeprecatedRasterPosFunctionReason;
            break;

        case AP_DEPRECATION_RASTER_POS_STATE:
        case AP_DEPRECATION_ALPHA_TEST_STATE:
        case AP_DEPRECATION_EVALUATORS_STATE:
        case AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_STATE:
        case AP_DEPRECATION_ATTRIBUTE_STACKS_STATE:
        case AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE:
        case AP_DEPRECATION_NON_SPRITE_POINTS:
        case AP_DEPRECATION_ACCUMMULATION_BUFFERS_STATE:
        case AP_DEPRECATION_MAX_VARYING:
        case AP_DEPRECATION_COMPRESSED_TEXTURE_FORMATS:
        case AP_DEPRECATION_LSB_FIRST_PIXEL_PACKING:
            str.appendFormattedString(AP_STR_DeprecatedStateVariableReason, _argumentIndex, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_ATTRIBUTE_STACKS_FUNCTION:
            str = AP_STR_DeprecatedAttributeStacksReason;
            break;

        case AP_DEPRECATION_SEPERATE_POLYGON_DRAW_MODE:
            str = AP_STR_DeprecatedSeperatePolygonDrawModeReason;
            break;

        case AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_FUNCTION:
            str = AP_STR_DeprecatedPolygonAndLineStippleReason;
            break;

        case AP_DEPRECATION_BITMAP:
            str = AP_STR_DeprecatedBitmapReason;
            break;

        case AP_DEPRECATION_PIXEL_DRAWING:
            str = AP_STR_DeprecatedPixelDrawingReason;
            break;

        case AP_DEPRECATION_PIXEL_FORMAT:
            str.appendFormattedString(AP_STR_DeprecatedPixelFormatArgValueReason, _argumentIndex, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING:
            str.appendFormattedString(AP_STR_DeprecatedEdgeFlagsArgValueReason, _argumentIndex, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_FUNCTION:
            str.appendFormattedString(AP_STR_DeprecatedFixedFunctionFragmentProcessingReason, _argumentIndex, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_ACCUMMULATION_BUFFERS_FUNCTION:
            str.appendFormattedString(AP_STR_DeprecatedAccumulationBuffersReason, _argumentIndex, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_FRAMEBUFFER_SIZE_QUERIES:
            str.appendFormattedString(AP_STR_DeprecatedFramebufferSizeQueriesReason, _argumentIndex, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_PIXEL_COPY:
            str.appendFormattedString(AP_STR_DeprecatedPixelCopyReason, _argumentIndex, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES:
            str.appendFormattedString(AP_STR_DeprecatedPolygonQuadsPrimitivesReason, _argumentIndex, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_UNIFIED_EXTENSION_STRING:
            str.appendFormattedString(AP_STR_DeprecatedUnifiedExtensionStringReason, _argumentIndex, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_AUXILIRY_BUFFERS:
            str.appendFormattedString(AP_STR_DeprecatedAuxiliryBuffersReason, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_CLIENT_VERTEX_AND_INDEX_ARRAYS:
            str.appendFormattedString(AP_STR_DeprecatedClientVertexAndIndexArraysReason, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_PIXEL_TRANSFER:
            str.appendFormattedString(AP_STR_DeprecatedPixelTransferReason, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_GLSL_VERSION:
            str.appendFormattedString(AP_STR_DeprecatedGLSLVersionReason, _argumentIndex, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_APPLICATION_GENERATED_NAMES:
            str.appendFormattedString(AP_STR_DeprecatedApplicationGeneratedNamesReason, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_TEXTURE_CLAMP_WRAP_MODE:
            str.appendFormattedString(AP_STR_DeprecatedTextureClampWrapReason, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_TEXTURE_AUTO_MIPMAP:
            str.appendFormattedString(AP_STR_DeprecatedTextureAutoMipmapReason, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_ALPHA_TEST_FUNCTION:
            str.appendFormattedString(AP_STR_DeprecatedAlphaTestReason, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_EVALUATORS_FUNCTION:
            str.appendFormattedString(AP_STR_DeprecatedEvaluatorsReason, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_FEEDBACK:
            str.appendFormattedString(AP_STR_DeprecatedFeedbackReason, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_HINTS:
            str.appendFormattedString(AP_STR_DeprecatedHintsReason, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_LINE_WIDTH:
            str.appendFormattedString(AP_STR_DeprecatedLineWidthReason, _reasonStr.asCharArray());
            break;

        case AP_DEPRECATION_TEXTURE_BORDER:
            str.appendFormattedString(AP_STR_DeprecatedTextureBorderReason, _reasonStr.asCharArray());
            break;

        default:
            GT_ASSERT(0);
            retVal = false;
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apFunctionDeprecation::getDeprecationAndRemoveVersionsByStatus
// Description: Return a OpenGL versions where the function was deprecated and
//              removed.
// Arguments: apFunctionDeprecationStatus functionDeprecationStatus
//            apAPIVersion& deprecatedAtVersion
//            apAPIVersion& removedAtVersion
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/3/2009
// ---------------------------------------------------------------------------
bool apFunctionDeprecation::getDeprecationAndRemoveVersionsByStatus(apFunctionDeprecationStatus functionDeprecationStatus, apAPIVersion& deprecatedAtVersion, apAPIVersion& removedAtVersion)
{
    bool retVal = false;

    switch (functionDeprecationStatus)
    {
        // OpenGL 3.0 - 3.1 deprecations:
        case AP_DEPRECATION_PIXEL_FORMAT:
        case AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING:
        case AP_DEPRECATION_IMMEDIATE_MODE:
        case AP_DEPRECATION_DISPLAY_LISTS:
        case AP_DEPRECATION_RECTANGLE:
        case AP_DEPRECATION_ATTRIBUTE_STACKS_FUNCTION:
        case AP_DEPRECATION_ATTRIBUTE_STACKS_STATE:
        case AP_DEPRECATION_SEPERATE_POLYGON_DRAW_MODE:
        case AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_FUNCTION:
        case AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_STATE:
        case AP_DEPRECATION_PIXEL_DRAWING:
        case AP_DEPRECATION_BITMAP:
        case AP_DEPRECATION_COLOR_INDEX_MODE:
        case AP_DEPRECATION_RASTER_POS_FUNCTION:
        case AP_DEPRECATION_RASTER_POS_STATE:
        case AP_DEPRECATION_APPLICATION_GENERATED_NAMES:
        case AP_DEPRECATION_TEXTURE_CLAMP_WRAP_MODE:
        case AP_DEPRECATION_TEXTURE_AUTO_MIPMAP:
        case AP_DEPRECATION_ALPHA_TEST_FUNCTION:
        case AP_DEPRECATION_ALPHA_TEST_STATE:
        case AP_DEPRECATION_EVALUATORS_FUNCTION:
        case AP_DEPRECATION_EVALUATORS_STATE:
        case AP_DEPRECATION_FEEDBACK:
        case AP_DEPRECATION_HINTS:
        case AP_DEPRECATION_NON_SPRITE_POINTS:
        case AP_DEPRECATION_LINE_WIDTH:
        case AP_DEPRECATION_TEXTURE_BORDER:
        case AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_FUNCTION:
        case AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE:
        case AP_DEPRECATION_ACCUMMULATION_BUFFERS_FUNCTION:
        case AP_DEPRECATION_ACCUMMULATION_BUFFERS_STATE:
        case AP_DEPRECATION_FRAMEBUFFER_SIZE_QUERIES:
        case AP_DEPRECATION_PIXEL_COPY:
        case AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES:
        case AP_DEPRECATION_UNIFIED_EXTENSION_STRING:
        case AP_DEPRECATION_AUXILIRY_BUFFERS:
        case AP_DEPRECATION_CLIENT_VERTEX_AND_INDEX_ARRAYS:
        case AP_DEPRECATION_PIXEL_TRANSFER:
        case AP_DEPRECATION_GLSL_VERSION:
        {
            deprecatedAtVersion = AP_GL_VERSION_3_0;
            removedAtVersion = AP_GL_VERSION_3_1;
            retVal = true;
        }
        break;

        // OpenGL 3.2:
        case AP_DEPRECATION_MAX_VARYING:
        {
            deprecatedAtVersion = AP_GL_VERSION_3_2;
            removedAtVersion = AP_GL_VERSION_NONE;
            retVal = true;
        }
        break;

        // OpenGL 4.2:
        case AP_DEPRECATION_COMPRESSED_TEXTURE_FORMATS:
        {
            deprecatedAtVersion = AP_GL_VERSION_4_2;
            removedAtVersion = AP_GL_VERSION_NONE;
            retVal = true;
        }
        break;

        // OpenGL 4.3:
        case AP_DEPRECATION_LSB_FIRST_PIXEL_PACKING:
        {
            deprecatedAtVersion = AP_GL_VERSION_4_3;
            removedAtVersion = AP_GL_VERSION_NONE;
            retVal = true;
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
// Name:        apFunctionDeprecation::getDeprecationStatusByFunctionId
// Description: This function maps a function Id to deprecation status, so that
//              the properties string for deprecated function names, would be the
//              same as the properties string for deprecated function calls in
//              analyze mode
// Arguments: int functionId
//            apFunctionDeprecationStatus& deprecationStatus
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/3/2009
// ---------------------------------------------------------------------------
bool apFunctionDeprecation::getDeprecationStatusByFunctionId(int functionId, apFunctionDeprecationStatus& deprecationStatus)
{
    bool retVal = true;

    switch (functionId)
    {
        case ap_glBindBuffer:
        case ap_glBindBufferARB:
        case ap_glBindTexture:
        {
            // Application generated names functions:
            deprecationStatus = AP_DEPRECATION_APPLICATION_GENERATED_NAMES;
            break;
        }

        case ap_glLightModelf:
        case ap_glLightModelfv:
        case ap_glLightModeli:
        case ap_glLightModeliv:
        case ap_glLightf:
        case ap_glLightfv:
        case ap_glLighti:
        case ap_glLightiv:
        case ap_glGetLightfv:
        case ap_glGetLightiv:
        case ap_glColorPointer:
        case ap_glEdgeFlagPointer:
        case ap_glFogCoordPointer:
        case ap_glNormalPointer:
        case ap_glSecondaryColorPointer:
        case ap_glTexCoordPointer:
        case ap_glVertexPointer:
        case ap_glEnableClientState:
        case ap_glDisableClientState:
        case ap_glInterleavedArrays:
        case ap_glClientActiveTexture:
        case ap_glFrustum:
        case ap_glLoadIdentity:
        case ap_glLoadMatrixd:
        case ap_glLoadMatrixf:
        case ap_glLoadTransposeMatrixd:
        case ap_glLoadTransposeMatrixf:
        case ap_glMatrixMode:
        case ap_glMultMatrixd:
        case ap_glMultMatrixf:
        case ap_glMultTransposeMatrixf:
        case ap_glMultTransposeMatrixd:
        case ap_glOrtho:
        case ap_glPopMatrix:
        case ap_glPushMatrix:
        case ap_glRotated:
        case ap_glRotatef:
        case ap_glScaled:
        case ap_glScalef:
        case ap_glTranslatef:
        case ap_glTranslated:
        case ap_glMaterialf:
        case ap_glMaterialfv:
        case ap_glMateriali:
        case ap_glMaterialiv:
        case ap_glShadeModel:
        {
            // Edge flags functions:
            deprecationStatus = AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING;
            break;
        }

        case ap_glIndexs:
        case ap_glIndexi:
        case ap_glIndexf:
        case ap_glIndexd:
        case ap_glIndexub:
        case ap_glIndexsv:
        case ap_glIndexiv:
        case ap_glIndexfv:
        case ap_glIndexdv:
        case ap_glIndexubv:
        case ap_glIndexPointer:
        {
            // Color index mode functions:
            deprecationStatus = AP_DEPRECATION_COLOR_INDEX_MODE;
            break;
        }

        case ap_glBegin:
        case ap_glEnd:
        case ap_glColor3b:
        case ap_glColor3bv:
        case ap_glColor3d:
        case ap_glColor3dv:
        case ap_glColor3f:
        case ap_glColor3fv:
        case ap_glColor3i:
        case ap_glColor3iv:
        case ap_glColor3s:
        case ap_glColor3sv:
        case ap_glColor3ub:
        case ap_glColor3ubv:
        case ap_glColor3ui:
        case ap_glColor3uiv:
        case ap_glColor3us:
        case ap_glColor3usv:
        case ap_glColor4b:
        case ap_glColor4bv:
        case ap_glColor4d:
        case ap_glColor4dv:
        case ap_glColor4f:
        case ap_glColor4fv:
        case ap_glColor4i:
        case ap_glColor4iv:
        case ap_glColor4s:
        case ap_glColor4sv:
        case ap_glColor4ub:
        case ap_glColor4ubv:
        case ap_glColor4ui:
        case ap_glColor4uiv:
        case ap_glColor4us:
        case ap_glColor4usv:
        case ap_glNormal3b:
        case ap_glNormal3bv:
        case ap_glNormal3d:
        case ap_glNormal3dv:
        case ap_glNormal3f:
        case ap_glNormal3fv:
        case ap_glNormal3i:
        case ap_glNormal3iv:
        case ap_glNormal3s:
        case ap_glNormal3sv:
        case ap_glVertex2d:
        case ap_glVertex2dv:
        case ap_glVertex2f:
        case ap_glVertex2fv:
        case ap_glVertex2i:
        case ap_glVertex2iv:
        case ap_glVertex2s:
        case ap_glVertex2sv:
        case ap_glVertex3d:
        case ap_glVertex3dv:
        case ap_glVertex3f:
        case ap_glVertex3fv:
        case ap_glVertex3i:
        case ap_glVertex3iv:
        case ap_glVertex3s:
        case ap_glVertex3sv:
        case ap_glVertex4d:
        case ap_glVertex4dv:
        case ap_glVertex4f:
        case ap_glVertex4fv:
        case ap_glVertex4i:
        case ap_glVertex4iv:
        case ap_glVertex4s:
        case ap_glVertex4sv:
        {
            // Immediate mode:
            deprecationStatus = AP_DEPRECATION_IMMEDIATE_MODE;
            break;
        }

        case ap_glNewList:
        case ap_glEndList:
        case ap_glCallList:
        case ap_glCallLists:
        case ap_glListBase:
        case ap_glGenLists:
        case ap_glIsList:
        case ap_glDeleteLists:
        {
            deprecationStatus = AP_DEPRECATION_DISPLAY_LISTS;
            break;
        }

        case ap_glPushAttrib:
        case ap_glPushClientAttrib:
        case ap_glPopAttrib:
        case ap_glPopClientAttrib:
        {
            deprecationStatus = AP_DEPRECATION_ATTRIBUTE_STACKS_FUNCTION;
            break;
        }

        case ap_glRectd:
        case ap_glRectdv:
        case ap_glRectf:
        case ap_glRectfv:
        case ap_glRecti:
        case ap_glRectiv:
        case ap_glRects:
        case ap_glRectsv:
        {
            deprecationStatus = AP_DEPRECATION_RECTANGLE;
            break;
        }

        case ap_glRasterPos2d:
        case ap_glRasterPos2dv:
        case ap_glRasterPos2f:
        case ap_glRasterPos2fv:
        case ap_glRasterPos2i:
        case ap_glRasterPos2iv:
        case ap_glRasterPos2s:
        case ap_glRasterPos2sv:
        case ap_glRasterPos3d:
        case ap_glRasterPos3dv:
        case ap_glRasterPos3f:
        case ap_glRasterPos3fv:
        case ap_glRasterPos3i:
        case ap_glRasterPos3iv:
        case ap_glRasterPos3s:
        case ap_glRasterPos3sv:
        case ap_glRasterPos4d:
        case ap_glRasterPos4dv:
        case ap_glRasterPos4f:
        case ap_glRasterPos4fv:
        case ap_glRasterPos4i:
        case ap_glRasterPos4iv:
        case ap_glRasterPos4s:
        case ap_glRasterPos4sv:
        case ap_glWindowPos2d:
        case ap_glWindowPos2dv:
        case ap_glWindowPos2f:
        case ap_glWindowPos2fv:
        case ap_glWindowPos2i:
        case ap_glWindowPos2iv:
        case ap_glWindowPos2s:
        case ap_glWindowPos2sv:
        case ap_glWindowPos3d:
        case ap_glWindowPos3dv:
        case ap_glWindowPos3f:
        case ap_glWindowPos3fv:
        case ap_glWindowPos3i:
        case ap_glWindowPos3iv:
        case ap_glWindowPos3s:
        case ap_glWindowPos3sv:
        {
            deprecationStatus = AP_DEPRECATION_RASTER_POS_FUNCTION;
            break;
        }

        case ap_glPolygonMode:
        {
            deprecationStatus = AP_DEPRECATION_SEPERATE_POLYGON_DRAW_MODE;
            break;
        }

        case ap_glPolygonStipple:
        case ap_glLineStipple:
        {
            deprecationStatus = AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_FUNCTION;
            break;
        }

        case ap_glBitmap:
        {
            deprecationStatus = AP_DEPRECATION_BITMAP;
            break;
        }

        case ap_glDrawPixels:
        case ap_glPixelZoom:
        {
            deprecationStatus = AP_DEPRECATION_PIXEL_DRAWING;
            break;
        }

        case ap_glAlphaFunc:
        {
            deprecationStatus = AP_DEPRECATION_ALPHA_TEST_FUNCTION;
            break;
        }

        case ap_glRenderMode:
        case ap_glInitNames:
        case ap_glPopName:
        case ap_glPushName:
        case ap_glLoadName:
        case ap_glSelectBuffer:
        case ap_glFeedbackBuffer:
        case ap_glPassThrough:
        {
            deprecationStatus = AP_DEPRECATION_FEEDBACK;
            break;
        }

        case ap_glMap1d:
        case ap_glMap1f:
        case ap_glMap2d:
        case ap_glMap2f:
        case ap_glMapGrid1d:
        case ap_glMapGrid1f:
        case ap_glMapGrid2d:
        case ap_glMapGrid2f:
        case ap_glEvalCoord1d:
        case ap_glEvalCoord1dv:
        case ap_glEvalCoord1f:
        case ap_glEvalCoord1fv:
        case ap_glEvalCoord2d:
        case ap_glEvalCoord2dv:
        case ap_glEvalCoord2f:
        case ap_glEvalCoord2fv:
        case ap_glEvalMesh1:
        case ap_glEvalMesh2:
        case ap_glEvalPoint1:
        case ap_glEvalPoint2:
        {
            deprecationStatus = AP_DEPRECATION_EVALUATORS_FUNCTION;
            break;
        }

        case ap_glAreTexturesResident:
        case ap_glPrioritizeTextures:
        case ap_glFogf:
        case ap_glFogfv:
        case ap_glFogi:
        case ap_glFogiv:
        {
            deprecationStatus = AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_FUNCTION;
            break;
        }

        case ap_glClearAccum:
        case ap_glAccum:
        {
            deprecationStatus = AP_DEPRECATION_ACCUMMULATION_BUFFERS_FUNCTION;
            break;
        }

        case ap_glCopyPixels:
        {
            deprecationStatus = AP_DEPRECATION_PIXEL_COPY;
            break;
        }

        case ap_glPixelTransferf:
        case ap_glPixelTransferi:
        {
            deprecationStatus = AP_DEPRECATION_PIXEL_TRANSFER;
            break;
        }

        default:
        {
            // The function does not have a specific deprecation status:
            deprecationStatus = AP_DEPRECATION_FULL;
            retVal = true;
            break;
        }
    }

    return retVal;
}
