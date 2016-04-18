//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsArrayPointer.h
///
//==================================================================================

//------------------------------ gsArrayPointer.h ------------------------------

#ifndef __GSARRAYPOINTER_H
#define __GSARRAYPOINTER_H

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>


// ----------------------------------------------------------------------------------
// Struct Name:          gsArrayPointer
// General Description:
//   Holds data related with an OpenGL array pointer.
//   This pointed data can represent vertices, normals, colors, etc
//
// Author:               Yaki Tebeka
// Creation Date:        18/2/2006
// ----------------------------------------------------------------------------------
struct gsArrayPointer
{
    // The number of coordinates that build up a single array item (vertex / normal / etc):
    GLint _numOfCoordinates;

    // The coordinate data type:
    // (GL_BYTE, GL_SHORT, GL_FIXED, GL_FLOAT, etc)
    GLenum _dataType;

    // Byte offset between consecutive items in the array:
    // (If stride is 0, the items are tightly packed in the array)
    GLsizei _stride;

    // Pointer to the first coordinate of the first item in the array:
    const GLvoid* _pArrayRawData;

public:
    gsArrayPointer() : _numOfCoordinates(4), _dataType(GL_FLOAT), _stride(0), _pArrayRawData(NULL) {};
};


#endif //__GSARRAYPOINTER_H
