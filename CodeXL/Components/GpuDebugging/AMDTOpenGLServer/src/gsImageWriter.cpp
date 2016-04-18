//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsImageWriter.cpp
///
//==================================================================================

//------------------------------ gsImageWriter.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apGLenumParameter.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <src/gsStringConstants.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsWrappersCommon.h>
#include <src/gsImageWriter.h>

// Static members initializations:
bool gsImageWriter::_arePixelStoreEnvVarsForced = false;

// ---------------------------------------------------------------------------
// Name:        gsImageWriter::gsImageWriter
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        23/8/2006
// ---------------------------------------------------------------------------
gsImageWriter::gsImageWriter()
    : _width(0), _height(0), _depth(0), _valueOfPackAlignmentEnvVar(GL_NONE)
#ifndef _GR_IPHONE_BUILD
    , _valueOfPackLSBFirstEnvVar(GL_FALSE),
      _valueOfPackSwapBytesEnvVar(GL_FALSE),
      _valueOfPackRowLengthEnvVar(GL_NONE),
      _valueOfPackSkipPixelsEnvVar(GL_NONE),
      _valueOfPackSkipRowsEnvVar(GL_NONE),
      _valueOfPackSkipImagesEnvVar(GL_NONE),
      _valueOfPackImageHeightEnvVar(GL_NONE),
      _valueOfReadBufferParameter(GL_NONE)
#endif
{
}


// ---------------------------------------------------------------------------
// Name:        gsImageWriter::~gsImageWriter
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        23/8/2006
// ---------------------------------------------------------------------------
gsImageWriter::~gsImageWriter()
{
}


// ---------------------------------------------------------------------------
// Name:        gsImageWriter::setOpenGLPixelPackParameters
// Description:
//   Sets OpenGL's "pixel pack" parameter.
//
// Arguments:
//   packAlignment - This parameter specifies the alignment requirements for
//                   the start of each pixel row in memory. The allowable values are
//                   - 1 (byte-alignment),
//                   - 2 (rows aligned to even-numbered bytes),
//                   - 4 (word-alignment),
//                   - 8 (rows start on double-word boundaries).
//
// Author:      Yaki Tebeka
// Date:        2/1/2005
// ---------------------------------------------------------------------------
void gsImageWriter::setOpenGLPixelPackParameters(int packAlignment)
{
#ifdef _GR_IPHONE_BUILD
    // In the iPhone, the only allowed glPixelStorei values are GL_PACK_ALIGNMENT and GL_UNPACK_ALIGNMENT.
    // Since unpacking doesn't concern us, we get the value of GL_PACK_ALIGNMENT and set out value.
    // Make sure we didn't set this before:
    GT_IF_WITH_ASSERT(!_arePixelStoreEnvVarsForced)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
        GLint currentValue = GL_NONE;
        gs_stat_realFunctionPointers.glGetIntegerv(GL_PACK_ALIGNMENT, &currentValue);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
        {
            // Mark that we are forcing the value:
            _arePixelStoreEnvVarsForced = true;

            // Store the previous value:
            _valueOfPackAlignmentEnvVar = currentValue;

            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glPixelStorei);
            gs_stat_realFunctionPointers.glPixelStorei(GL_PACK_ALIGNMENT, packAlignment);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glPixelStorei);
        }
    }
#else // ndef _GR_IPHONE_BUILD
    // Make sure we didn't set this before:
    GT_IF_WITH_ASSERT(!_arePixelStoreEnvVarsForced)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
        // Clear any previous errors:
        GLenum previousOGLError = gs_stat_realFunctionPointers.glGetError();
        GT_ASSERT(previousOGLError == GL_NO_ERROR);

        // Get the current values:
        GLint currentPackAlignmentValue = GL_NONE;
        GLboolean currentPackLSBFirstValue = GL_FALSE;
        GLboolean currentPackSwapBytesValue = GL_FALSE;
        GLint currentPackRowLengthValue = GL_NONE;
        GLint currentPackSkipPixelsValue = GL_NONE;
        GLint currentPackSkipRowsValue = GL_NONE;
        GLint currentPackSkipImagesValue = GL_NONE;
        GLint currentPackImageHeightValue = GL_NONE;

        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetBooleanv);
        gs_stat_realFunctionPointers.glGetIntegerv(GL_PACK_ALIGNMENT, &currentPackAlignmentValue);
        gs_stat_realFunctionPointers.glGetBooleanv(GL_PACK_LSB_FIRST, &currentPackLSBFirstValue);
        gs_stat_realFunctionPointers.glGetBooleanv(GL_PACK_SWAP_BYTES, &currentPackSwapBytesValue);
        gs_stat_realFunctionPointers.glGetIntegerv(GL_PACK_ROW_LENGTH, &currentPackRowLengthValue);
        gs_stat_realFunctionPointers.glGetIntegerv(GL_PACK_SKIP_PIXELS, &currentPackSkipPixelsValue);
        gs_stat_realFunctionPointers.glGetIntegerv(GL_PACK_SKIP_ROWS, &currentPackSkipRowsValue);
        gs_stat_realFunctionPointers.glGetIntegerv(GL_PACK_SKIP_IMAGES, &currentPackSkipImagesValue);
        gs_stat_realFunctionPointers.glGetIntegerv(GL_PACK_IMAGE_HEIGHT, &currentPackImageHeightValue);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetBooleanv);

        GLenum oglError = gs_stat_realFunctionPointers.glGetError();
        GT_IF_WITH_ASSERT((currentPackAlignmentValue != GL_NONE) && (oglError == GL_NO_ERROR))
        {
            // Mark that we are forcing the value:
            _arePixelStoreEnvVarsForced = true;

            // Store the previous values:
            _valueOfPackAlignmentEnvVar = currentPackAlignmentValue;
            _valueOfPackLSBFirstEnvVar = currentPackLSBFirstValue;
            _valueOfPackSwapBytesEnvVar = currentPackSwapBytesValue;
            _valueOfPackRowLengthEnvVar = currentPackRowLengthValue;
            _valueOfPackSkipPixelsEnvVar = currentPackSkipPixelsValue;
            _valueOfPackSkipRowsEnvVar = currentPackSkipRowsValue;
            _valueOfPackSkipImagesEnvVar = currentPackSkipImagesValue;
            _valueOfPackImageHeightEnvVar = currentPackImageHeightValue;

            // Set the parameters to values that enable us copy
            // the bitmap pixels into the free image bitmap
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glPixelStorei);
            gs_stat_realFunctionPointers.glPixelStorei(GL_PACK_LSB_FIRST, 0);
            gs_stat_realFunctionPointers.glPixelStorei(GL_PACK_SWAP_BYTES, 0);
            gs_stat_realFunctionPointers.glPixelStorei(GL_PACK_ROW_LENGTH, 0);
            gs_stat_realFunctionPointers.glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
            gs_stat_realFunctionPointers.glPixelStorei(GL_PACK_SKIP_ROWS, 0);
            gs_stat_realFunctionPointers.glPixelStorei(GL_PACK_SKIP_IMAGES, 0);
            gs_stat_realFunctionPointers.glPixelStorei(GL_PACK_IMAGE_HEIGHT, 0);
            gs_stat_realFunctionPointers.glPixelStorei(GL_PACK_ALIGNMENT, packAlignment);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glPixelStorei);

            oglError = gs_stat_realFunctionPointers.glGetError();
            testAndReportOpenGLError(oglError);
        }

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);
    }
#endif
}


// ---------------------------------------------------------------------------
// Name:        gsImageWriter::restoreOpenGLPixelPackParameters
// Description:
//  Restores the OpenGL pixel "pack" parameters.
//  I.E: Removes the effect of setOpenGLPixelPackParameters()
// Author:      Yaki Tebeka
// Date:        2/1/2005
// ---------------------------------------------------------------------------
void gsImageWriter::restoreOpenGLPixelPackParameters()
{
#ifdef _GR_IPHONE_BUILD
    // Restore the value from our member:
    GT_IF_WITH_ASSERT(_arePixelStoreEnvVarsForced)
    {
        _arePixelStoreEnvVarsForced = false;
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glPixelStorei);
        gs_stat_realFunctionPointers.glPixelStorei(GL_PACK_ALIGNMENT, _valueOfPackAlignmentEnvVar);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glPixelStorei);

        // Reset the member's value:
        _valueOfPackAlignmentEnvVar = GL_NONE;
    }
#else // ndef _GR_IPHONE_BUILD
    // Restore the value from our members:
    GT_IF_WITH_ASSERT(_arePixelStoreEnvVarsForced)
    {
        // Note we aren't forcing the variables anymore:
        _arePixelStoreEnvVarsForced = false;

        // Clear any previous errors:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
        GLenum previousOGLError = gs_stat_realFunctionPointers.glGetError();
        GT_ASSERT(previousOGLError == GL_NO_ERROR);

        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glPixelStorei);
        gs_stat_realFunctionPointers.glPixelStorei(GL_PACK_LSB_FIRST, _valueOfPackLSBFirstEnvVar);
        gs_stat_realFunctionPointers.glPixelStorei(GL_PACK_SWAP_BYTES, _valueOfPackSwapBytesEnvVar);
        gs_stat_realFunctionPointers.glPixelStorei(GL_PACK_ROW_LENGTH, _valueOfPackRowLengthEnvVar);
        gs_stat_realFunctionPointers.glPixelStorei(GL_PACK_SKIP_PIXELS, _valueOfPackSkipPixelsEnvVar);
        gs_stat_realFunctionPointers.glPixelStorei(GL_PACK_SKIP_ROWS, _valueOfPackSkipRowsEnvVar);
        gs_stat_realFunctionPointers.glPixelStorei(GL_PACK_SKIP_IMAGES, _valueOfPackSkipImagesEnvVar);
        gs_stat_realFunctionPointers.glPixelStorei(GL_PACK_IMAGE_HEIGHT, _valueOfPackImageHeightEnvVar);
        gs_stat_realFunctionPointers.glPixelStorei(GL_PACK_ALIGNMENT, _valueOfPackAlignmentEnvVar);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glPixelStorei);

        // Check for errors:
        GLenum oglError = gs_stat_realFunctionPointers.glGetError();
        testAndReportOpenGLError(oglError);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

        // Reset the members' values:
        _valueOfPackAlignmentEnvVar = GL_NONE;
        _valueOfPackLSBFirstEnvVar = GL_FALSE;
        _valueOfPackSwapBytesEnvVar = GL_FALSE;
        _valueOfPackRowLengthEnvVar = GL_NONE;
        _valueOfPackSkipPixelsEnvVar = GL_NONE;
        _valueOfPackSkipRowsEnvVar = GL_NONE;
        _valueOfPackSkipImagesEnvVar = GL_NONE;
        _valueOfPackImageHeightEnvVar = GL_NONE;
    }
#endif
}

// ---------------------------------------------------------------------------
// Name:        gsImageWriter::setOpenGLReadBufferParameter
// Description: Sets the GL_READ_BUFFER value.
// Return Val:  bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        5/1/2010
// ---------------------------------------------------------------------------
bool gsImageWriter::setOpenGLReadBufferParameter(GLenum readBuffer)
{
    bool retVal = true;

    // glReadBuffer does not exist in OpenGL ES:
#ifndef _GR_IPHONE_BUILD
    // Make sure we didn't do this before:
    GT_IF_WITH_ASSERT(_valueOfReadBufferParameter == GL_NONE)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
        // Clear any previous errors:
        GLenum previousOGLError = gs_stat_realFunctionPointers.glGetError();
        GT_ASSERT(previousOGLError == GL_NO_ERROR);

        // Get the current value:
        GLint currentReadBufferValueAsInt = GL_NONE;
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
        gs_stat_realFunctionPointers.glGetIntegerv(GL_READ_BUFFER, &currentReadBufferValueAsInt);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);

        GLenum oglError = gs_stat_realFunctionPointers.glGetError();
        GT_IF_WITH_ASSERT((oglError == GL_NO_ERROR) && (currentReadBufferValueAsInt != GL_NONE))
        {
            _valueOfReadBufferParameter = (GLenum)currentReadBufferValueAsInt;

            // Set the value:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glReadBuffer);
            gs_stat_realFunctionPointers.glReadBuffer(readBuffer);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glReadBuffer);

            oglError = gs_stat_realFunctionPointers.glGetError();
            retVal = testAndReportOpenGLError(oglError);
        }
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);
    }
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsImageWriter::restoreOpenGLReadBufferParameter
// Description: Restores the previous GL_READ_BUFFER value.
// Return Val:  bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        5/1/2010
// ---------------------------------------------------------------------------
bool gsImageWriter::restoreOpenGLReadBufferParameter()
{
    bool retVal = true;

    // glReadBuffer does not exist in OpenGL ES:
#ifndef _GR_IPHONE_BUILD
    // Make sure we didn't do this before:
    GT_IF_WITH_ASSERT(_valueOfReadBufferParameter != GL_NONE)
    {
        // Restore the value:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glReadBuffer);
        gs_stat_realFunctionPointers.glReadBuffer(_valueOfReadBufferParameter);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glReadBuffer);

        // Reset the member:
        _valueOfReadBufferParameter = GL_NONE;

        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
        GLenum oglError = gs_stat_realFunctionPointers.glGetError();
        retVal = testAndReportOpenGLError(oglError);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);
    }
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsImageWriter::testAndReportOpenGLError
// Description: Checks if oglError is an error, and asserts it if it is.
// Return Val:  true iff oglError == GL_NO_ERROR
// Author:      Uri Shomroni
// Date:        5/1/2010
// ---------------------------------------------------------------------------
bool gsImageWriter::testAndReportOpenGLError(GLenum oglError)
{
    bool retVal = true;

    if (oglError != GL_NO_ERROR)
    {
        // We generated an OpenGL error:
        gtString oglEnumAsString;
        apGLenumValueToString(oglError, oglEnumAsString);

        gtString errorMessage = GS_STR_oglError;
        errorMessage.appendFormattedString(L": %ls", oglEnumAsString.asCharArray());
        GT_ASSERT_EX(false, errorMessage.asCharArray());

        retVal = false;
    }

    return retVal;
}
