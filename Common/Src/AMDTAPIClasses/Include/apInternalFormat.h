//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apInternalFormat.h
///
//==================================================================================

//------------------------------ apInternalFormat.h ------------------------------

#ifndef __APINTERNALFORMAT
#define __APINTERNALFORMAT

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>

AP_API gtString apInternalFormatValueAsString(GLenum internalFormat);
AP_API bool apGetPixelSizeInBitsByInternalFormat(GLenum internalFormat, GLuint& pixelSize);
AP_API bool apGetChannelCountByInternalFormat(GLenum internalFormat, int& channels);
AP_API bool apGetIsInternalFormatCompressed(GLenum internalFormat);
AP_API bool apGetVBOFormatFromTextureBufferFormat(GLenum texBufferInternalFormat, oaTexelDataFormat& vboFormat);

#endif  // __APINTERNALFORMAT
