//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ glext_additions.h ------------------------------

#ifndef __GLEXT_ADDITIONS_H
#define __GLEXT_ADDITIONS_H

/*
This file includes Linux OS additional glext.h definitions which we support
*/

#ifdef __cplusplus
extern "C"
{
#endif

/* GL_ARB_instanced_arrays */
#ifdef GL_GLEXT_PROTOTYPES
GLAPI void APIENTRY glVertexAttribDivisor(GLuint index, GLuint divisor);
#endif /* GL_GLEXT_PROTOTYPES */
typedef void (APIENTRYP PFNGLVERTEXATTRIBDIVISORPROC)(GLuint index, GLuint divisor);

#ifdef __cplusplus
}
#endif

#endif //__GLEXT_ADDITIONS_H

