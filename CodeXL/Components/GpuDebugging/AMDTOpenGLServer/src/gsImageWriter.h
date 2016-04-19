//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsImageWriter.h
///
//==================================================================================

//------------------------------ gsImageWriter.h ------------------------------

#ifndef __GSIMAGEWRITER_H
#define __GSIMAGEWRITER_H

// Forward decelerations:
class osFilePath;

// Infra:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTAPIClasses/Include/apFileType.h>

// The amount of bytes per pixel:
#define GS_BYTES_PER_PIXEL 4


// ----------------------------------------------------------------------------------
// Class Name:           gsImageWriter
// General Description:
//   Base class for classes that write images data (1D, 2D and 3D) into disk.
//
// Author:               Yaki Tebeka
// Creation Date:        12/4/2005
// ----------------------------------------------------------------------------------
class gsImageWriter
{
public:
    gsImageWriter();
    virtual ~gsImageWriter();

protected:
    void getImageSize(GLsizei& width, GLsizei& height, GLsizei& depth) const { width = _width; height = _height; depth = _depth; };
    void setImageSize(GLsizei width, GLsizei height, GLsizei depth) { _width = width; _height = height; _depth = depth; };

    void setOpenGLPixelPackParameters(int packAlignment);
    void restoreOpenGLPixelPackParameters();

    bool setOpenGLReadBufferParameter(GLenum readBuffer);
    bool restoreOpenGLReadBufferParameter();

private:
    bool testAndReportOpenGLError(GLenum oglError);

private:
    // The image width, height and depth (including the border additions):
    GLsizei _width;
    GLsizei _height;
    GLsizei _depth;

    // Attrib stacks are not supported on the iPhone or OpenGL 3.1+, so we hold this
    // value ourselves.
    GLint _valueOfPackAlignmentEnvVar;

#ifndef _GR_IPHONE_BUILD
    // These environment variables are not supported on the iPhone, we only need the alignment there:
    GLboolean _valueOfPackLSBFirstEnvVar;
    GLboolean _valueOfPackSwapBytesEnvVar;
    GLint _valueOfPackRowLengthEnvVar;
    GLint _valueOfPackSkipPixelsEnvVar;
    GLint _valueOfPackSkipRowsEnvVar;
    GLint _valueOfPackSkipImagesEnvVar;
    GLint _valueOfPackImageHeightEnvVar;

    GLenum _valueOfReadBufferParameter;
#endif

    static bool _arePixelStoreEnvVarsForced;
};



#endif //__GSIMAGEWRITER_H
