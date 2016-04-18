//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsVertexArrayDrawer.cpp
///
//==================================================================================

//------------------------------ gsVertexArrayDrawer.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/osOSDefinitions.h>
#include <AMDTAPIClasses/Include/apGLFixedWrapper.h>
#include <AMDTAPIClasses/Include/apParameters.h>

// Local:
#include <inc/gsMonitoredFunctionPointers.h>
#include <inc/gsWrappersCommon.h>
#include <inc/gsVertexArrayDrawer.h>

// A multiplexing factor that translates values from the [0,255] range to the [0,1] range:
static float stat_0To255To0To1Translator = 1.0f / 255.0f;


// ---------------------------------------------------------------------------
// Name:        gsVertexArrayDrawer::drawArrays
// Description: Draws vertex array. If needed, performs data migrations.
// Arguments: mode - The kind of primitives to render (GL_POINTS, GL_LINE_STRIP, etc).
//            first - The starting index in the array.
//            count - Number of indices to be rendered.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/3/2006
// ---------------------------------------------------------------------------
bool gsVertexArrayDrawer::drawArrays(GLenum mode, GLint first, GLsizei count)
{
    bool retVal = false;

    // Perform required data migrations:
    bool rc = performDataMigrations(first, count);

    if (rc)
    {
        // Draw the vertex array:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDrawArrays);
        gs_stat_realFunctionPointers.glDrawArrays(mode, 0, count);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDrawArrays);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsVertexArrayDrawer::drawElements
//
// Description: Draws vertex array elements. If needed, performs data migrations.
//
// Arguments: mode - The kind of primitives to render (GL_POINTS, GL_LINE_STRIP, etc).
//            count - Number of elements to be rendered.
//            type - type of the values in indices (GL_UNSIGNED_BYTE / GL_UNSIGNED_SHORT / etc).
//            indices - Pointer to indices array.
//
// Return Val: bool  - Success / failure.
//
// Author:      Yaki Tebeka
// Date:        1/3/2006
// ---------------------------------------------------------------------------
bool gsVertexArrayDrawer::drawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
    bool retVal = false;

    // Perform required data migrations:
    bool rc = performIndexedDataMigrations(count, type, indices);

    if (rc)
    {
        // Draw the vertex array elements:
        // (During the data migration we transformed all enabled arrays from indexed to non indexed data):
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDrawArrays);
        gs_stat_realFunctionPointers.glDrawArrays(mode, 0, count);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDrawArrays);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsVertexArrayDrawer::performDataMigrations
// Description: Performs required arrays data migrations to match the current
//              hardware capabilities.
//
// Arguments: first - The first array index to be converted.
//            count - Number of array items to be converted.
// Author:      Yaki Tebeka
// Date:        1/3/2006
// ---------------------------------------------------------------------------
bool gsVertexArrayDrawer::performDataMigrations(GLint first, GLsizei count)
{
    bool retVal = true;

    // Get the arrays data:
    const gsArrayPointer& verticesArrayData = _vertexArrayData._verticesArray;
    const gsArrayPointer& normalsArrayData = _vertexArrayData._normalsArray;
    const gsArrayPointer& textureCoordinatesArrayData = _vertexArrayData._textureCoordinatesArray;
    const gsArrayPointer& colorArrayData = _vertexArrayData._colorsArray;

    // If we need to migrate the vertices data:
    if (_vertexArrayData._isVerticesArrayEnabled)
    {
        // Migrate the vertices data to floats:
        bool rc = migrateArrayDataToFloatData(verticesArrayData, false, first, count, _convertedVerticesData);

        if (rc)
        {
            // Set the OpenGL vertex pointer to point the converted data array:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glVertexPointer);
            gs_stat_realFunctionPointers.glVertexPointer(verticesArrayData._numOfCoordinates,
                                                         GL_FLOAT, 0,
                                                         &(_convertedVerticesData.rawData()));
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glVertexPointer);
        }
        else
        {
            retVal = false;
        }
    }

    // If we need to migrate the normals data:
    if (_vertexArrayData._isNormalsArrayEnabled)
    {
        // Migrate the vertices data to floats:
        bool rc = migrateArrayDataToFloatData(normalsArrayData, false, first, count, _convertedNormalsData);

        if (rc)
        {
            // Set the OpenGL normal pointer to point the converted data array:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glNormalPointer);
            gs_stat_realFunctionPointers.glNormalPointer(GL_FLOAT, 0, &(_convertedNormalsData.rawData()));
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glNormalPointer);
        }
        else
        {
            retVal = false;
        }
    }

    // If we need to migrate the texture coordinates data:
    if (_vertexArrayData._isTextureCoorinatesArrayEnabled)
    {
        // Migrate the texture data to floats:
        bool rc = migrateArrayDataToFloatData(textureCoordinatesArrayData, false, first, count, _convertedTextureCoordinatesData);

        if (rc)
        {
            // Set the OpenGL texture coordinates pointer to point the converted data array:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glTexCoordPointer);
            gs_stat_realFunctionPointers.glTexCoordPointer(textureCoordinatesArrayData._numOfCoordinates,
                                                           GL_FLOAT, 0,
                                                           &(_convertedTextureCoordinatesData.rawData()));
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glTexCoordPointer);
        }
        else
        {
            retVal = false;
        }
    }

    // If we need to migrate the colors data:
    if (_vertexArrayData._isColorsArrayEnabled)
    {
        // We assume all color data types (except float) to be [0,255] data:
        bool is0To255Data = true;

        if (colorArrayData._dataType == GL_FLOAT)
        {
            is0To255Data = false;
        }

        // Migrate the vertices data to floats:
        bool rc = migrateArrayDataToFloatData(colorArrayData, is0To255Data, first, count, _convertedColorData);

        if (rc)
        {
            // Set the OpenGL color pointer to point the converted data array:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glColorPointer);
            gs_stat_realFunctionPointers.glColorPointer(colorArrayData._numOfCoordinates,
                                                        GL_FLOAT, 0,
                                                        &(_convertedColorData.rawData()));
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glColorPointer);
        }
        else
        {
            retVal = false;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsMigrateArrayToFloatData
// Description:
//   Converts data data to GLfloat data array.
//
// Template arguments: DataArrayType - The array data type.
//
// Arguments: dataToBeMigrated - The array of data to be migrated.
//            is0To255Data - true iff the input data is [0, 255] data. This kind of data
//                           will be translated to float data of [0,1].
//            first - first elements to be converted.
//            count - Amount of elements to be converted.
//            migratedData - Will get the migrated data.
//
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/6/2006
// ---------------------------------------------------------------------------
template<class DataArrayType>
bool gsMigrateArrayToFloatData(const gsArrayPointer& dataToBeMigrated, bool is0To255Data,
                               GLint first, GLsizei count, gtVector<GLfloat>& migratedData)
{
    bool retVal = true;

    // Get the size (in bytes) of a single coordinate:
    int inputCoordSize = sizeof(DataArrayType);

    // Get the stride (jump in bytes between two elements):
    GLsizei stride = dataToBeMigrated._stride;

    // Get the data to be migrated:
    DataArrayType* pDataToBeMigrated = (DataArrayType*)(dataToBeMigrated._pArrayRawData);

    if (pDataToBeMigrated == NULL)
    {
        retVal = false;
        GT_ASSERT(0);
    }
    else
    {
        // Clear the migrated data vector:
        migratedData.clear();

        // Calculate the first item position:
        int amountOfItemCoordinates = dataToBeMigrated._numOfCoordinates;
        DataArrayType* pCurrentItem = pDataToBeMigrated;

        if (stride == 0)
        {
            // stride = 0 is a special case that means that the items are tightly packed:
            pCurrentItem += (amountOfItemCoordinates * first);
        }
        else
        {
            gtByte* pCurrentItemAsBytesPtr = (gtByte*)pCurrentItem;
            pCurrentItemAsBytesPtr += (stride * first);
            pCurrentItem = (DataArrayType*)pCurrentItemAsBytesPtr;
        }

        // Iterate the items to be migrated:
        for (int currItem = 0; currItem < count; currItem++)
        {
            // Iterate the current item coordinates:
            for (int currCoord = 0; currCoord < amountOfItemCoordinates; currCoord++)
            {
                // Migrate the current coordinate data to GLfloats:
                // (Notice that for GLfixed data, the used float operator is apGLfixedWrapper::float())
                DataArrayType itemData = pCurrentItem[currCoord];
                GLfloat itemDataAsFloat = float(itemData);

                // If the data should be mapped from [0,255] to [0,1]:
                if (is0To255Data)
                {
                    itemDataAsFloat *= stat_0To255To0To1Translator;
                }

                migratedData.push_back(itemDataAsFloat);
            }

            // Move pCurrentItem to point the next item:
            if (stride == 0)
            {
                // stride = 0 is a special case that means that the items are tightly packed:
                pCurrentItem += amountOfItemCoordinates;
            }
            else
            {
                // Jump to the next item data:
                gtByte* pCurrentItemAsBytesPtr = (gtByte*)pCurrentItem;
                pCurrentItemAsBytesPtr += stride;
                pCurrentItem = (DataArrayType*)pCurrentItemAsBytesPtr;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsVertexArrayDrawer::migrateArrayDataToFloatData
// Description: Migrates data array to GLfloat data array.
// Arguments: dataToBeMigrated - The array of data to be migrated.
//            is0To255Data - true iff the input data is [0, 255] data. This kind of data
//                           will be translated to float data of [0,1].
//            first - The index of the first item to be migrated.
//            count - The amount of items to be migrated.
//            migratedData - Will get the migrated data.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/6/2006
// ---------------------------------------------------------------------------
bool gsVertexArrayDrawer::migrateArrayDataToFloatData(const gsArrayPointer& dataToBeMigrated,
                                                      bool is0To255Data,
                                                      GLint first, GLsizei count,
                                                      gtVector<GLfloat>& migratedData)
{
    bool retVal = false;

    // Work according to the data array type:
    switch (dataToBeMigrated._dataType)
    {
        case GL_BYTE:
            retVal = gsMigrateArrayToFloatData<GLbyte>(dataToBeMigrated, is0To255Data, first, count, migratedData);
            break;

        case GL_UNSIGNED_BYTE:
            retVal = gsMigrateArrayToFloatData<GLubyte>(dataToBeMigrated, is0To255Data, first, count, migratedData);
            break;

        case GL_SHORT:
            retVal = gsMigrateArrayToFloatData<GLshort>(dataToBeMigrated, is0To255Data, first, count, migratedData);
            break;

        case GL_FIXED:
            retVal = gsMigrateArrayToFloatData<apGLFixedWrapper>(dataToBeMigrated, is0To255Data, first, count, migratedData);
            break;

        case GL_FLOAT:
            retVal = gsMigrateArrayToFloatData<GLfloat>(dataToBeMigrated, is0To255Data, first, count, migratedData);
            break;

        default:
            // Unsupported data array type:
            GT_ASSERT_EX(false, L"Unsupported data array type");
            break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsVertexArrayDrawer::performIndexedDataMigrations
// Description:
//   Performs required indexed arrays data migrations to match
//   the current hardware capabilities.
//
// Arguments: count - Number of elements to be converted.
//            type - Type of the values in indices. (GL_UNSIGNED_BYTE or GL_UNSIGNED_SHORT).
//            indices - Pointer to the indices array.
//
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/3/2006
//
// Implementation notes:
//   We transform the data from indexed to non indexed arrays. Therefore, we need
//   to transform all enabled arrays (not only enabled and GL_FIXED arrays).
//
// ---------------------------------------------------------------------------
bool gsVertexArrayDrawer::performIndexedDataMigrations(GLsizei count, GLenum type, const GLvoid* indices)
{
    bool retVal = true;

    // Get the arrays data:
    const gsArrayPointer& verticesArrayData = _vertexArrayData._verticesArray;
    const gsArrayPointer& normalsArrayData = _vertexArrayData._normalsArray;
    const gsArrayPointer& textureCoordinatesArrayData = _vertexArrayData._textureCoordinatesArray;
    const gsArrayPointer& colorArrayData = _vertexArrayData._colorsArray;

    // If we need to migrate the vertices data:
    if (_vertexArrayData._isVerticesArrayEnabled)
    {
        // Migrate the vertices data to floats:
        bool rc = migrateIndexedDataToNoneIndexedFloatData(verticesArrayData, false, count, type, indices, _convertedVerticesData);

        if (rc)
        {
            // Set the OpenGL vertex pointer to point the converted data array:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glVertexPointer);
            gs_stat_realFunctionPointers.glVertexPointer(verticesArrayData._numOfCoordinates,
                                                         GL_FLOAT, 0,
                                                         &(_convertedVerticesData.rawData()));
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glVertexPointer);
        }
        else
        {
            retVal = false;
        }
    }

    // If we need to migrate the normals data:
    if (_vertexArrayData._isNormalsArrayEnabled)
    {
        // Migrate the vertices data to floats:
        bool rc = migrateIndexedDataToNoneIndexedFloatData(normalsArrayData, false, count, type, indices, _convertedNormalsData);

        if (rc)
        {
            // Set the OpenGL normal pointer to point the converted data array:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glNormalPointer);
            gs_stat_realFunctionPointers.glNormalPointer(GL_FLOAT, 0, &(_convertedNormalsData.rawData()));
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glNormalPointer);
        }
        else
        {
            retVal = false;
        }
    }

    // If we need to migrate the texture coordinates data:
    if (_vertexArrayData._isTextureCoorinatesArrayEnabled)
    {
        // Migrate the vertices data to floats:
        bool rc = migrateIndexedDataToNoneIndexedFloatData(textureCoordinatesArrayData, false, count, type, indices, _convertedTextureCoordinatesData);

        if (rc)
        {
            // Set the OpenGL texture coordinates pointer to point the converted data array:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glTexCoordPointer);
            gs_stat_realFunctionPointers.glTexCoordPointer(textureCoordinatesArrayData._numOfCoordinates,
                                                           GL_FLOAT, 0,
                                                           &(_convertedTextureCoordinatesData.rawData()));
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glTexCoordPointer);
        }
        else
        {
            retVal = false;
        }
    }

    // If we need to migrate the colors data:
    if (_vertexArrayData._isColorsArrayEnabled)
    {
        // We assume all color data types (except float) to be [0,255] data:
        bool is0To255Data = true;

        if (colorArrayData._dataType == GL_FLOAT)
        {
            is0To255Data = false;
        }

        // Migrate the vertices data to floats:
        bool rc = migrateIndexedDataToNoneIndexedFloatData(colorArrayData, is0To255Data, count, type, indices, _convertedColorData);

        if (rc)
        {
            // Set the OpenGL color pointer to point the converted data array:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glColorPointer);
            gs_stat_realFunctionPointers.glColorPointer(colorArrayData._numOfCoordinates,
                                                        GL_FLOAT, 0,
                                                        &(_convertedColorData.rawData()));
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glColorPointer);
        }
        else
        {
            retVal = false;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsMigrateIndexedDataToNoneIndexedFloatData
// Description:
//   Converts indexed array data to GLfloat none indexed data array.
//
// Template arguments: IndexType - They data type of the indices array.
//                     DataArrayType - The data type of the data array.
//
// Arguments: dataToBeMigrated - The array of data to be migrated.
//            is0To255Data - true iff the input data is [0, 255] data. This kind of data
//                           will be translated to float data of [0,1].
//            count - count - Number of elements to be converted.
//            indices - Pointer to the indices array.
//            migratedData - Will get the migrated data.
//
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        18/6/2006
// ---------------------------------------------------------------------------
template<class IndexArrayType, class DataArrayType>
bool gsMigrateIndexedDataToNoneIndexedFloatData(const gsArrayPointer& dataToBeMigrated,
                                                bool is0To255Data, GLsizei count, const IndexArrayType* indices,
                                                gtVector<GLfloat>& migratedData)
{
    bool retVal = true;

    // Get the size (in bytes) of a single coordinate:
    int inputCoordSize = sizeof(IndexArrayType);

    // Get the stride (jump in bytes between two elements):
    GLsizei stride = dataToBeMigrated._stride;

    // Get the data to be migrated:
    DataArrayType* pDataToBeMigrated = (DataArrayType*)(dataToBeMigrated._pArrayRawData);

    if (pDataToBeMigrated == NULL)
    {
        retVal = false;
        GT_ASSERT(0);
    }
    else
    {
        // Clear the migrated data vector:
        migratedData.clear();

        // Get the amount of coordinates that build an item:
        int amountOfItemCoordinates = dataToBeMigrated._numOfCoordinates;

        // Iterate the items to be migrated:
        for (int currItem = 0; currItem < count; currItem++)
        {
            // Get the current item's index:
            int currentItemIndex = (int)(indices[currItem]);

            // Calculate the current item position:
            DataArrayType* pCurrentItemPos = NULL;

            // stride = 0 is a special case that means that the items are tightly packed:
            if (stride == 0)
            {
                int elementPos = currentItemIndex * amountOfItemCoordinates;
                pCurrentItemPos = &(pDataToBeMigrated[elementPos]);
            }
            else
            {
                gtByte* pDataToBeMigratedAsBytesPtr = (gtByte*)pDataToBeMigrated;
                gtByte* pCurrentItemPosAsBytesPtr = pDataToBeMigratedAsBytesPtr + (currentItemIndex * stride);
                pCurrentItemPos = (DataArrayType*)pCurrentItemPosAsBytesPtr;
            }

            // Iterate the current item coordinates:
            for (int currCoord = 0; currCoord < amountOfItemCoordinates; currCoord++)
            {
                // Migrate the current coordinate data from DataArrayType to GLfloat:
                // (Notice that for GLfixed data, the used float operator is apGLfixedWrapper::float())
                DataArrayType itemData = pCurrentItemPos[currCoord];
                GLfloat itemDataAsFloat = float(itemData);

                // If the data should be mapped from [0,255] to [0,1]:
                if (is0To255Data)
                {
                    itemDataAsFloat *= stat_0To255To0To1Translator;
                }

                migratedData.push_back(itemDataAsFloat);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsVertexArrayDrawer::migrateIndexedDataToNoneIndexedFloatData
// Description: Converts indexed array data to GLfloat none indexed data.
//
// Arguments: dataToBeMigrated - The array of data to be migrated.
//            is0To255Data - true iff the input data is [0, 255] data. This kind of data
//                           will be translated to float data of [0,1].
//            count - count - Number of elements to be converted.
//            type - Type of the values in indices. (GL_UNSIGNED_BYTE or GL_UNSIGNED_SHORT).
//            indices - Pointer to the indices array.
//            migratedData - Will get the migrated data.
//
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/3/2006
// ---------------------------------------------------------------------------
bool gsVertexArrayDrawer::migrateIndexedDataToNoneIndexedFloatData(const gsArrayPointer& dataToBeMigrated,
                                                                   bool is0To255Data,
                                                                   GLsizei count, GLenum type,
                                                                   const GLvoid* indices,
                                                                   gtVector<GLfloat>& migratedData)
{
    bool retVal = false;

    // Work according to the index array type:
    // (OpenGL ES 1.1 supports only GL_UNSIGNED_BYTE or GL_UNSIGNED_SHORT indices)
    if (type == GL_UNSIGNED_BYTE)
    {
        // Work according to the data array type:
        switch (dataToBeMigrated._dataType)
        {
            case GL_BYTE:
                retVal = gsMigrateIndexedDataToNoneIndexedFloatData<GLubyte, GLbyte>(dataToBeMigrated, is0To255Data, count, (const GLubyte*)indices, migratedData);
                break;

            case GL_UNSIGNED_BYTE:
                retVal = gsMigrateIndexedDataToNoneIndexedFloatData<GLubyte, GLubyte>(dataToBeMigrated, is0To255Data, count, (const GLubyte*)indices, migratedData);
                break;

            case GL_SHORT:
                retVal = gsMigrateIndexedDataToNoneIndexedFloatData<GLubyte, GLshort>(dataToBeMigrated, is0To255Data, count, (const GLubyte*)indices, migratedData);
                break;

            case GL_FIXED:
                retVal = gsMigrateIndexedDataToNoneIndexedFloatData<GLubyte, apGLFixedWrapper>(dataToBeMigrated, is0To255Data, count, (const GLubyte*)indices, migratedData);
                break;

            case GL_FLOAT:
                retVal = gsMigrateIndexedDataToNoneIndexedFloatData<GLubyte, GLfloat>(dataToBeMigrated, is0To255Data, count, (const GLubyte*)indices, migratedData);
                break;

            default:
                // Unsupported data array type:
                GT_ASSERT_EX(false, L"Unsupported data array type");
                break;
        }
    }
    else if (type == GL_UNSIGNED_SHORT)
    {
        // Work according to the data array type:
        switch (dataToBeMigrated._dataType)
        {
            case GL_BYTE:
                retVal = gsMigrateIndexedDataToNoneIndexedFloatData<GLushort, GLbyte>(dataToBeMigrated, is0To255Data, count, (const GLushort*)indices, migratedData);
                break;

            case GL_UNSIGNED_BYTE:
                retVal = gsMigrateIndexedDataToNoneIndexedFloatData<GLushort, GLubyte>(dataToBeMigrated, is0To255Data, count, (const GLushort*)indices, migratedData);
                break;

            case GL_SHORT:
                retVal = gsMigrateIndexedDataToNoneIndexedFloatData<GLushort, GLshort>(dataToBeMigrated, is0To255Data, count, (const GLushort*)indices, migratedData);
                break;

            case GL_FIXED:
                retVal = gsMigrateIndexedDataToNoneIndexedFloatData<GLushort, apGLFixedWrapper>(dataToBeMigrated, is0To255Data, count, (const GLushort*)indices, migratedData);
                break;

            case GL_FLOAT:
                retVal = gsMigrateIndexedDataToNoneIndexedFloatData<GLushort, GLfloat>(dataToBeMigrated, is0To255Data, count, (const GLushort*)indices, migratedData);
                break;

            default:
                // Unsupported data array type:
                GT_ASSERT_EX(false, L"Unsupported data array type");
                break;
        }
    }
    else
    {
        // Unsupported index array type:
        GT_ASSERT_EX(false, L"Unsupported index array type");
    }

    return retVal;
}



