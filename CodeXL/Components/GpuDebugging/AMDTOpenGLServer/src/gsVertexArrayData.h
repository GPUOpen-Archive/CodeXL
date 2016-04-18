//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsVertexArrayData.h
///
//==================================================================================

//------------------------------ gsVertexArrayData.h ------------------------------

#ifndef __GSVERTEXARRAYDATA_H
#define __GSVERTEXARRAYDATA_H

// Local:
#include <src/gsArrayPointer.h>


// ----------------------------------------------------------------------------------
// Struct Name:          gsVertexArrayData
// General Description:
//   Holds data related with a vertex array: vertices data, normals data, etc.
//
// Author:               Yaki Tebeka
// Creation Date:        18/2/2006
// ----------------------------------------------------------------------------------
struct gsVertexArrayData
{
    // Pointed vertices array data:
    gsArrayPointer _verticesArray;

    // Pointed normals array data:
    gsArrayPointer _normalsArray;

    // Pointed colors array data:
    gsArrayPointer _colorsArray;

    // Pointed texture coordinates data:
    gsArrayPointer _textureCoordinatesArray;

    // Contains true iff the relevant array is enabled:
    bool _isVerticesArrayEnabled;
    bool _isNormalsArrayEnabled;
    bool _isColorsArrayEnabled;
    bool _isTextureCoorinatesArrayEnabled;

public:
    gsVertexArrayData() : _isVerticesArrayEnabled(false), _isNormalsArrayEnabled(false),
        _isColorsArrayEnabled(false), _isTextureCoorinatesArrayEnabled(false) {};

};


#endif //__GSVERTEXARRAYDATA_H
