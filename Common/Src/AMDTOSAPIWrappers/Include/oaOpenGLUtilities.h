//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaOpenGLUtilities.h
///
//=====================================================================

//------------------------------ oaOpenGLUtilities.h ------------------------------

#ifndef __OAOPENGLUTILITIES_H
#define __OAOPENGLUTILITIES_H

// Local:
#include <AMDTOSAPIWrappers/Include/oaOSAPIWrappersDLLBuild.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

OA_API GLint oaUnProjectPoint(GLdouble winx, GLdouble winy, GLdouble winz, const GLdouble modelMatrix[16], const GLdouble projMatrix[16], const GLint viewport[4], GLdouble* objx, GLdouble* objy, GLdouble* objz);
OA_API int oaInvertMatrixd(const GLdouble m[16], GLdouble invOut[16]);
OA_API void oaMultMatricesd(const GLdouble a[16], const GLdouble b[16], GLdouble r[16]);
OA_API void oaMultMatrixVecd(const GLdouble matrix[16], const GLdouble in[4], GLdouble out[4]);

#endif //__OAOPENGLUTILITIES_H

