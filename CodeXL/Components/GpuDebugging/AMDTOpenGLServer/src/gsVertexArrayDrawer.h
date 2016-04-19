//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsVertexArrayDrawer.h
///
//==================================================================================

//------------------------------ gsVertexArrayDrawer.h ------------------------------

#ifndef __GSVERTEXARRAYDRAWER_H
#define __GSVERTEXARRAYDRAWER_H

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <src/gsVertexArrayData.h>


// ----------------------------------------------------------------------------------
// Struct Name:          gsVertexArrayDrawer
// General Description:
//   Draws a verex array. If needed, performs data migrations to support the
//   hardware capabilities.
//
// Author:               Yaki Tebeka
// Creation Date:        1/3/2006
// ----------------------------------------------------------------------------------
class gsVertexArrayDrawer
{
public:
    gsVertexArrayDrawer(const gsVertexArrayData& vertexArrayData) : _vertexArrayData(vertexArrayData) {};
    ~gsVertexArrayDrawer() = default;

    bool drawArrays(GLenum mode, GLint first, GLsizei count);
    bool drawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);

private:
    // Disallow use of default constructor, copy constructor and assignment operator:
    gsVertexArrayDrawer() = delete;
    gsVertexArrayDrawer(const gsVertexArrayDrawer&) = delete;
    gsVertexArrayDrawer& operator=(const gsVertexArrayDrawer&) = delete;

    bool performDataMigrations(GLint first, GLsizei count);
    bool migrateArrayDataToFloatData(const gsArrayPointer& dataToBeMigrated, bool is0To255Data,
                                     GLint first, GLsizei count,
                                     gtVector<GLfloat>& migratedData);

    bool performIndexedDataMigrations(GLsizei count, GLenum type, const GLvoid* indices);
    bool migrateIndexedDataToNoneIndexedFloatData(const gsArrayPointer& dataToBeMigrated, bool is0To255Data,
                                                  GLsizei count, GLenum type, const GLvoid* indices,
                                                  gtVector<GLfloat>& migratedData);

private:
    // The vertex arrays data:
    const gsVertexArrayData& _vertexArrayData;

    // Converted vertices data:
    gtVector<GLfloat> _convertedVerticesData;

    // Converted normals data:
    gtVector<GLfloat> _convertedNormalsData;

    // Converted texture coordinates data:
    gtVector<GLfloat> _convertedTextureCoordinatesData;

    // Converted color data:
    gtVector<GLfloat> _convertedColorData;
};


#endif //__GSVERTEXARRAYDRAWER_H
