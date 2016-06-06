//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apDefaultTextureNames.cpp
///
//==================================================================================

//------------------------------ apDefaultTextureNames.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTAPIClasses/Include/apDefaultTextureNames.h>

// Calculate the location of the first and last default texture name:
static GLuint stat_firstDefaultTexName = INT_MAX - AP_DEFAULT_TEXTURE_NAMES_OFFSET;
static GLuint stat_lastDefaultTexName = stat_firstDefaultTexName + AP_AMOUNT_OF_TEXTURE_BIND_TARGETS * AP_AMOUNT_OF_DEFAULT_TEX_NAME_PER_BIND_TARGETS - 1;


// ---------------------------------------------------------------------------
// Name:        apIsDefaultTextureName
// Description: Inputs an OpenGL texture name and returns true iff it is a
//              "default texture name".
//              (For more details, see the "What are default texture names?" comment
//               at apDefaultTextureNames.h).
// Author:  AMD Developer Tools Team
// Date:        20/4/2005
// ---------------------------------------------------------------------------
bool apIsDefaultTextureName(GLuint textureName)
{
    bool retVal = false;

    if ((stat_firstDefaultTexName <= textureName) && (textureName <= stat_lastDefaultTexName))
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGetDefaultTextureName
// Description: Inputs a texture unit name and a texture type, and outputs the
//              appropriate "default texture name".
//              (For more details, see the "What are default texture names?" comment
//               at apDefaultTextureNames.h).
// Arguments:   textureUnitName - The texture unit name.
//              textureType - The texture type.
//              textureName - Will get the output "default texture name".
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/4/2005
// ---------------------------------------------------------------------------
bool apGetDefaultTextureName(GLenum textureUnitName, apTextureType textureType, GLuint& textureName)
{
    bool retVal = false;

    // Unit name range test:
    if ((GL_TEXTURE0 <= textureUnitName) && (textureUnitName <= GL_TEXTURE31))
    {
        retVal = true;

        // Calculate the texture unit index (offset from GL_TEXTURE0):
        int textureUnitIndex = textureUnitName - GL_TEXTURE0;

        // Calculate the amount that should be added to stat_firstDefaultTexName to get our
        // "default texture name":
        int addedAmount = (textureUnitIndex * AP_AMOUNT_OF_TEXTURE_BIND_TARGETS);
        addedAmount += (int)textureType;

        // Output the default texture name:
        textureName = stat_firstDefaultTexName + addedAmount;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGetDefaultTextureUnitAndType
// Description: Inputs a "default texture name" and returns its unit and type.
//              (For more details, see the "What are default texture names?" comment
//               at apDefaultTextureNames.h).
// Arguments:   textureName - The input "default texture name".
//              textureUnitName - The output texture unit name.
//              textureType - The output texture type.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/4/2005
// ---------------------------------------------------------------------------
bool apGetDefaultTextureUnitAndType(GLuint textureName, GLenum& textureUnitName, apTextureType& textureType)
{
    bool retVal = false;

    // Sanity check:
    if (apIsDefaultTextureName(textureName))
    {
        retVal = true;

        // Calculate the amount that was added to stat_firstDefaultTexName to get the
        // input "default texture name":
        int addedAmount = textureName - stat_firstDefaultTexName;

        // Calculate the texture unit name:
        int textureUnitIndex = addedAmount / AP_AMOUNT_OF_TEXTURE_BIND_TARGETS;
        textureUnitName = GL_TEXTURE0 + textureUnitIndex;

        // Calculate the texture type:
        int textureTypeAsInt = addedAmount % AP_AMOUNT_OF_TEXTURE_BIND_TARGETS;
        textureType = (apTextureType)textureTypeAsInt;
    }

    return retVal;
}


