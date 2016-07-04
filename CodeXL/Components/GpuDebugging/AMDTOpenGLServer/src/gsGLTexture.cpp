//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsGLTexture.cpp
///
//==================================================================================

//------------------------------ gsGLTexture.cpp ------------------------------
#include <math.h>
// Infra:
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTAPIClasses/Include/apOpenGLExtensionsId.h>
#include <AMDTAPIClasses/Include/apOpenGLParameters.h>
#include <AMDTServerUtilities/Include/suAllocatedObjectsMonitor.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Local:
#include <src/gsGLTexture.h>
#include <src/gsGlobalVariables.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsWrappersCommon.h>
#ifdef _GR_IPHONE_BUILD
    #include <src/gsOpenGLMonitor.h>
    #include <src/gsRenderContextMonitor.h>
#endif


// ---------------------------------------------------------------------------
// Name:        gsGLTexture::gsGLTexture
// Description: Constructor
// Arguments: textureName - The name of the texture that this class represents.
// Author:      Yaki Tebeka
// Date:        29/6/2006
// ---------------------------------------------------------------------------
gsGLTexture::gsGLTexture(GLuint textureName)
    : apGLTexture(textureName), _isBoundToActiveFBO(false), _shouldAutoGenerateMipmap(false),
      _mipmapBaseLevel(0), _mipmapMaxLevel(1000)
{
#if defined(_GR_OPENGLES_COMMON) || defined(_GR_OPENGLES_IPHONE)
    _isOpenGLESTexture = true;
#endif
}


// ---------------------------------------------------------------------------
// Name:        gsGLTexture::gsGLTexture
// Description: Copy constructor
// Arguments: other - The other texture class from which I am copied.
// Author:      Yaki Tebeka
// Date:        2/7/2006
// ---------------------------------------------------------------------------
gsGLTexture::gsGLTexture(const gsGLTexture& other) : apGLTexture(other)
{
    gsGLTexture::operator=(other);
}


// ---------------------------------------------------------------------------
// Name:        gsGLTexture::~gsGLTexture
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        29/6/2006
// ---------------------------------------------------------------------------
gsGLTexture::~gsGLTexture()
{
}


// ---------------------------------------------------------------------------
// Name:        gsGLTexture::operator=
// Description: Assignment operator.
// Arguments: other - The other texture class from which I am copied.
// Return Val: gsGLTexture& - Will get reference to me.
// Author:      Yaki Tebeka
// Date:        2/7/2006
// ---------------------------------------------------------------------------
gsGLTexture& gsGLTexture::operator=(const gsGLTexture& other)
{
    // Copy base class data:
    apGLTexture::operator=(other);

    // Copy this class data:
    _isBoundToActiveFBO = other._isBoundToActiveFBO;

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        gsGLTexture::updateTextureParameters
// Description: Updates each of the texture and texture level parameters
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        29/10/2008
// ---------------------------------------------------------------------------
bool gsGLTexture::updateTextureParameters(bool shouldUpdateOnlyMemoryParams, bool isOpenGL31CoreContext)
{
    bool retVal = true;

    // Get the current texture type:
    GLenum bindTarget = apTextureTypeToTextureBindTarget(textureType());

    // If the texture is binded, update it's parameters:
    if (bindTarget != GL_NONE)
    {
        // Do not update texture parameters for multisample textures:
        bool rc1 = true;

        if ((textureType() != AP_2D_TEXTURE_MULTISAMPLE_ARRAY) && (textureType() != AP_2D_TEXTURE_MULTISAMPLE))
        {
            // Update texture parameters:
            rc1 = updateTextureParameters(bindTarget, shouldUpdateOnlyMemoryParams, isOpenGL31CoreContext);
            GT_ASSERT(rc1);
        }
        else
        {
            // For multisample texture - clear all parameters (multisample texture do not contain texture parameters):
            _textureParameters.clearAllParameters();
        }

        // Update the texture mip levels parameters:
        bool rc2 = updateTextureMipLevelsParameters(bindTarget, shouldUpdateOnlyMemoryParams, isOpenGL31CoreContext);
        GT_ASSERT(rc2);

        retVal = rc1 && rc2;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsGLTexture::markAllParametersAsUpdated
// Description: Mark all the texture and the texture level parameters as updated
// Return Val: void
// Author:      Sigal Algranaty
// Date:        30/11/2008
// ---------------------------------------------------------------------------
void gsGLTexture::markAllParametersAsUpdated(bool isUpdated)
{
    // Get the texture parameters:
    apGLTextureParams& textureParams = textureParameters();
    // Get the amount of parameters:
    int parametersAmount = textureParams.amountOfTextureParameters();

    for (int i = 0; i < parametersAmount; i++)
    {
        textureParams.setTextureParameterUpdateStatus(i, isUpdated);
    }

    // Check number of texture panes:
    apGLTextureMipLevel::apTextureFaceIndex firstFaceIndex = apGLTextureMipLevel::AP_SINGLE_TEXTURE_FACE_INDEX;
    apGLTextureMipLevel::apTextureFaceIndex lastFaceIndex = apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_NEGATIVE_X_FACE_INDEX;

    apTextureType texType = textureType();

    if (AP_CUBE_MAP_TEXTURE == texType)
    {
        firstFaceIndex = apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_POSITIVE_X_FACE_INDEX;
        lastFaceIndex = apGLTextureMipLevel::AP_MAX_AMOUNT_OF_TEXTURE_FACES;
    }

    int numberOfMipLevels = (int)_textureMipLevels.size();

    for (int mipLevel = 0; mipLevel < numberOfMipLevels; mipLevel++)
    {
        // Update the parameters for each texture pane:
        for (int textureFace = firstFaceIndex; textureFace < lastFaceIndex; textureFace++)
        {
            // Convert int to texture face index:
            apGLTextureMipLevel::apTextureFaceIndex faceIndex = (apGLTextureMipLevel::apTextureFaceIndex)textureFace;

            // Get the texture pane parameters:
            apGLTextureParams* pTextureLevelParams = textureLevelParameters(mipLevel, faceIndex);

            // Skip the null mip levels:
            if (pTextureLevelParams)
            {
                // Get amount of texture level parameters:
                int amountOfTextureLevelParams = pTextureLevelParams->amountOfTextureParameters();

                // Update all texture parameters:
                for (int paramIndex = 0; paramIndex < amountOfTextureLevelParams; paramIndex++)
                {
                    // Update the parameters update status:
                    pTextureLevelParams->setTextureParameterUpdateStatus(paramIndex, isUpdated);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsGLTexture::setNewMipLevelsAllocIds
// Description: Updates the alloc IDs (and allocation stacks) for all non-assigned
//              mip levels in this texture. This should be called whenever mipmaps
//              are created.
// Author:      Uri Shomroni
// Date:        6/11/2014
// ---------------------------------------------------------------------------
void gsGLTexture::setNewMipLevelsAllocIds()
{
    // Make sure all the texture's mip levels are registered in the allocated objects manager:
    GLuint minLevel = 0, maxLevel = 1000;
    bool rc = getTextureMinMaxLevels(minLevel, maxLevel);
    GT_IF_WITH_ASSERT(rc)
    {
        int newAllocId = -1;

        for (GLuint i = minLevel; i <= maxLevel; i++)
        {
            int allocId = getTextureMipLevelAllocatedObjectId(i);
            GT_IF_WITH_ASSERT(allocId != -2)
            {
                if (allocId == -1)
                {
                    // Generate an allocation ID for this call stack:
                    if (-1 == newAllocId)
                    {
                        apGLTextureMipLevel dummyMipLevel;
                        su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(dummyMipLevel);
                        newAllocId = dummyMipLevel.getAllocatedObjectId();
                    }

                    // Register this mip-map level in the allocated objects monitor:
                    setTextureMipLevelAllocatedObjectId(i, newAllocId);
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsGLTexture::readTextureParameters
// Description: Updates the texture level parameters
// Arguments: GLenum bindTarget
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/11/2008
// ---------------------------------------------------------------------------
bool gsGLTexture::updateTextureParameters(GLenum bindTarget, bool shouldUpdateOnlyMemoryParams, bool isOpenGL31CoreContext)
{
    bool retVal = true;

    // Update all texture parameters:
    int numberOfParams = _textureParameters.amountOfTextureParameters();

    for (int paramIndex = 0; paramIndex < numberOfParams; paramIndex++)
    {
        // Get the current parameter enum:
        GLenum paramName = _textureParameters.getTextureParameterName(paramIndex);

        // Check if this parameter should be updated:
        bool shouldUpdateCurrentParameter = true;

        if (shouldUpdateOnlyMemoryParams)
        {
            shouldUpdateCurrentParameter = isMemoryParameter(paramName);
        }

        // Only update the compressed size if the texture is compressed:
        if (shouldUpdateCurrentParameter && (GL_TEXTURE_COMPRESSED_IMAGE_SIZE == paramName))
        {
            shouldUpdateCurrentParameter = false;
            bool isCompressed = false;
            bool rcCmp = isTextureCompressed(_textureParameters, isCompressed);

            if (rcCmp)
            {
                shouldUpdateCurrentParameter = isCompressed;
            }
        }

        if (shouldUpdateCurrentParameter && isDeprecatedTexParam(paramName))
        {
            shouldUpdateCurrentParameter = !isOpenGL31CoreContext;
        }

        if (shouldUpdateCurrentParameter)
        {
            // Declare a float array to store the parameter value into:
            GLfloat pFloatParamValue[4] = {0, 0, 0, 0};

            // Get the current OpenGL parameter value:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetTexParameterfv);
            gs_stat_realFunctionPointers.glGetTexParameterfv(bindTarget, paramName, pFloatParamValue);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetTexParameterfv);

            // Check for OpenGL errors:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
            GLenum openGLError = gs_stat_realFunctionPointers.glGetError();
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

            // If there was an OpenGL error - fail the function:
            if (openGLError != GL_NO_ERROR)
            {
                // If this is a deprecated parameter, and we failed on the deprecation:
                if ((GL_INVALID_ENUM != openGLError) || (!isDeprecatedTexParam(paramName)))
                {
                    // Check if a parameter failure should fail the function:
                    bool shouldFailOnTexParameterUpdate = shouldFailOnTextureParameterUpdate(_textureParameters, paramName);

                    // Fail the function only for parameter that their value must be updated:
                    if (shouldFailOnTexParameterUpdate)
                    {
                        retVal = false;
                    }

                    gtString errS;
                    gtString pnameS;
                    apGLenumValueToString(openGLError, errS);
                    apGLenumValueToString(paramName, pnameS);
                    gtString dbg = errS;
                    dbg.appendFormattedString(L" Error on parameter %d: ", paramIndex).append(pnameS).appendFormattedString(L" (Texture %d)", textureName(), errS.asCharArray());
                    OS_OUTPUT_DEBUG_LOG(dbg.asCharArray(), OS_DEBUG_LOG_DEBUG);
                }

                // Mark the texture parameter as not updated:
                _textureParameters.setTextureParameterUpdateStatus(paramIndex, false);
            }
            else
            {
                // Set the parameter values:
                _textureParameters.setTextureParameterValueFromFloat(paramName, (GLfloat*)&pFloatParamValue);

                // Mark the texture parameter as updated:
                _textureParameters.setTextureParameterUpdateStatus(paramIndex, true);
            }
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gsGLTexture::updateTextureMipLevelsParameters
// Description: Read each of the texture levels parameters, and return false if one of the mip levels
//              failed
// Arguments: GLenum bindTarget
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/11/2008
// ---------------------------------------------------------------------------
bool gsGLTexture::updateTextureMipLevelsParameters(GLenum bindTarget, bool shouldUpdateOnlyMemoryParams, bool isOpenGL31CoreContext)
{
    bool retVal = true;

    // Check number of texture panes:
    apGLTextureMipLevel::apTextureFaceIndex firstFaceIndex = apGLTextureMipLevel::AP_SINGLE_TEXTURE_FACE_INDEX;
    apGLTextureMipLevel::apTextureFaceIndex lastFaceIndex = apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_NEGATIVE_X_FACE_INDEX;

    apTextureType texType = textureType();

    if (AP_CUBE_MAP_TEXTURE == texType)
    {
        firstFaceIndex = apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_POSITIVE_X_FACE_INDEX;
        lastFaceIndex = apGLTextureMipLevel::AP_MAX_AMOUNT_OF_TEXTURE_FACES;
    }

    // Set each of the texture mip levels parameters:
    int amountOfMipLevels = (int)_textureMipLevels.size();

    for (int mipLevel = 0; mipLevel < amountOfMipLevels; mipLevel++)
    {
        // Update the parameters for each texture pane:
        for (int textureFace = firstFaceIndex; textureFace < lastFaceIndex; textureFace++)
        {
            // Convert int to texture face index:
            apGLTextureMipLevel::apTextureFaceIndex faceIndex = (apGLTextureMipLevel::apTextureFaceIndex)textureFace;

            // Get the texture pane parameters:
            apGLTextureParams* pTextureLevelParams = textureLevelParameters(mipLevel, faceIndex);

            if (pTextureLevelParams != NULL)
            {
                // Get amount of texture level parameters:
                int amountOfTextureLevelParams = pTextureLevelParams->amountOfTextureParameters();

                // Update all texture parameters:
                for (int paramIndex = 0; paramIndex < amountOfTextureLevelParams; paramIndex++)
                {
                    // Get the current parameter enum:
                    GLenum paramName = pTextureLevelParams->getTextureParameterName(paramIndex);

                    // Check if this parameter should be updated:
                    bool shouldUpdateCurrentParameter = true;

                    if (shouldUpdateOnlyMemoryParams)
                    {
                        shouldUpdateCurrentParameter = isMemoryParameter(paramName);
                    }

                    // Only update the compressed size if the texture is compressed:
                    if (shouldUpdateCurrentParameter && (GL_TEXTURE_COMPRESSED_IMAGE_SIZE == paramName))
                    {
                        shouldUpdateCurrentParameter = false;
                        bool isCompressed = false;
                        bool rcCmp = isTextureCompressed(*pTextureLevelParams, isCompressed);

                        if (rcCmp)
                        {
                            shouldUpdateCurrentParameter = isCompressed;
                        }
                    }

                    if (shouldUpdateCurrentParameter && isDeprecatedTexParam(paramName))
                    {
                        shouldUpdateCurrentParameter = !isOpenGL31CoreContext;
                    }

                    if (shouldUpdateCurrentParameter)
                    {

                        // Declare a float array to store the parameter value into:
                        GLfloat pFloatParamValue[4] = {0, 0, 0, 0};

                        // Get the bind target according to texture pane index:
                        GLenum texturePaneBindTarget = apGLTextureMipLevel::textureFaceIndexToBindTarget(faceIndex, bindTarget);

                        // Clear previous OpenGL errors:
                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
                        GLenum previousOpenGLError = gs_stat_realFunctionPointers.glGetError();
                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);
                        GT_ASSERT(previousOpenGLError == GL_NO_ERROR);

#ifndef _GR_IPHONE_BUILD
                        // Get the current OpenGL parameter value:
                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetTexLevelParameterfv);
                        gs_stat_realFunctionPointers.glGetTexLevelParameterfv(texturePaneBindTarget, mipLevel, paramName, pFloatParamValue);
                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetTexLevelParameterfv);
#else
                        // OpenGL ES does not support level parameters:
                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetTexParameterfv);
                        gs_stat_realFunctionPointers.glGetTexParameterfv(texturePaneBindTarget, paramName, pFloatParamValue);
                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetTexParameterfv);
#endif

                        // Check for OpenGL errors:
                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
                        GLenum openGLError = gs_stat_realFunctionPointers.glGetError();
                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

                        // If there was an OpenGL error - fail the function:
                        if (openGLError != GL_NO_ERROR)
                        {
                            // If this is a deprecated parameter, and we failed on the deprecation:
                            if ((GL_INVALID_ENUM != openGLError) || (!isDeprecatedTexParam(paramName)))
                            {
                                // Check if the parameter should be checked:
                                bool shouldFailOnTexParameterUpdate = shouldFailOnTextureParameterUpdate(*pTextureLevelParams, paramName);

                                // Fail the function only for parameter that their value must be updated:
                                if (shouldFailOnTexParameterUpdate)
                                {
                                    retVal = false;
                                }

                                gtString errS;
                                gtString pnameS;
                                apGLenumValueToString(openGLError, errS);
                                apGLenumValueToString(paramName, pnameS);
                                gtString dbg = errS;
                                dbg.appendFormattedString(L" Error on parameter %d: ", paramIndex).append(pnameS).appendFormattedString(L" (Texture %d: Mip level:%d, face: %d)", textureName(), mipLevel, textureFace, errS.asCharArray());
                                OS_OUTPUT_DEBUG_LOG(dbg.asCharArray(), OS_DEBUG_LOG_DEBUG);
                            }

                            // Mark the texture parameter as updated:
                            pTextureLevelParams->setTextureParameterUpdateStatus(paramIndex, false);
                        }
                        else
                        {
                            // Mark the texture parameter as updated:
                            pTextureLevelParams->setTextureParameterUpdateStatus(paramIndex, true);

                            // Set the parameter values:
                            pTextureLevelParams->setTextureParameterValueFromFloat(paramName, (GLfloat*)&pFloatParamValue);
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsGLTexture::shouldFailOnTextureParameterUpdate
// Description: Check if the parameter update can fail. Some of the parameters are
//              supported only with certain extension or OpenGL version supported, so
//              we want to check the parameter update success, and not fail the function if not
// Arguments: GLenum paramName
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        30/10/2008
// ---------------------------------------------------------------------------
bool gsGLTexture::shouldFailOnTextureParameterUpdate(const apGLTextureParams& textureParams, GLenum paramName)const
{
    bool retVal = true;

#ifdef _GR_IPHONE_BUILD
    // OpenGL ES only supports several texture parameters:
    retVal = ((paramName == GL_TEXTURE_MIN_FILTER) || (paramName == GL_TEXTURE_MAG_FILTER) ||
              (paramName == GL_TEXTURE_WRAP_S) || (paramName == GL_TEXTURE_WRAP_T));

    if (paramName == GL_GENERATE_MIPMAP)
    {
        // This parameter only exists in OpenGL ES 1.1 and lower:
        gsRenderContextMonitor* pRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();
        GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
        {
            int oglesMajorVersion = 0;
            int oglesMinorVersion = 0;
            pRenderContextMonitor->getOpenGLVersion(oglesMajorVersion, oglesMinorVersion);

            retVal = (oglesMajorVersion < 2);
        }
    }

#else

    switch (paramName)
    {
        case GL_TEXTURE_COMPRESSED:
        {
            retVal = false;
        }
        break;

        case GL_TEXTURE_COMPRESSED_IMAGE_SIZE:
        {
            // We expect the GL_TEXTURE_COMPRESSED parameter to be updated first:
            retVal = false;
            bool isCompressed = false;
            bool rcCmp = isTextureCompressed(textureParams, isCompressed);
            GT_IF_WITH_ASSERT(rcCmp)
            {
                retVal = isCompressed;
            }
        }
        break;

        // Extension parameters - do not fail the update process for failure, since we do not know if the extension is supported:
        case GL_TEXTURE_SHARED_SIZE:
        case GL_TEXTURE_STENCIL_SIZE:
        case GL_TEXTURE_RED_TYPE:
        case GL_TEXTURE_GREEN_TYPE:
        case GL_TEXTURE_BLUE_TYPE:
        case GL_TEXTURE_ALPHA_TYPE:
        case GL_TEXTURE_DEPTH_TYPE:
        case GL_TEXTURE_HI_SIZE_NV:
        case GL_TEXTURE_LO_SIZE_NV:
        case GL_TEXTURE_DS_SIZE_NV:
        case GL_TEXTURE_DT_SIZE_NV:
        case GL_TEXTURE_MAG_SIZE_NV:
        case GL_TEXTURE_SAMPLES:
        case GL_TEXTURE_FIXED_SAMPLE_LOCATIONS:
        case GL_TEXTURE_SWIZZLE_R:
        case GL_TEXTURE_SWIZZLE_G:
        case GL_TEXTURE_SWIZZLE_B:
        case GL_TEXTURE_SWIZZLE_A:
        {
            retVal = false;
        }
        break;

        default:
        {
            retVal = true;
        }
        break;
    }

#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsGLTexture::isDeprecatedTexParam
// Description: Returns true iff the parameter is deprecated
// Author:      Uri Shomroni
// Date:        22/07/2015
// ---------------------------------------------------------------------------
bool gsGLTexture::isDeprecatedTexParam(GLenum paramName) const
{
    bool retVal = false;

    switch (paramName)
    {
        case GL_GENERATE_MIPMAP:
        case GL_TEXTURE_PRIORITY:
        case GL_DEPTH_TEXTURE_MODE:
        case GL_TEXTURE_BORDER:
        {
            retVal = true;
        }
        break;

        default:
        {
            retVal = false;
        }
        break;
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        apGLTexture::generateAutoMipmapLevels
// Description: The function is called when mipmap auto generation flag is changed
//              by the user. The function generate or destroy the auto generated
//              mip levels in texture.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        14/1/2009
// ---------------------------------------------------------------------------
bool gsGLTexture::generateAutoMipmapLevels()
{
    bool retVal = false;

    if (_shouldAutoGenerateMipmap)
    {
        // Auto generate mipmap levels:

        // Get texture mip level 0 (to copy attributes from):
        apGLTextureMipLevel* pTextureMipLevel0 = getTextureMipLevel(0);
        GT_IF_WITH_ASSERT(pTextureMipLevel0 != NULL)
        {
            // Get the texture base level dimensions:
            GLsizei width = 0, height = 0, depth = 0, borderSize;
            bool rc1 = pTextureMipLevel0->getDimensions(width, height, depth, borderSize);
            GT_IF_WITH_ASSERT(rc1)
            {
                // If texture dimensions are not initialized, do nothing (base and max levels cannot be calculated):
                bool dimensionsInitialized = !((width == 0) && (height == 0) && (depth == 0));

                if (dimensionsInitialized)
                {
                    // Calculate base and max levels:
                    GLuint baseLevel = 0, maxLevel = 0;
                    bool isMiplevelsNumEstimated = false;
                    bool rc2 = calcNumberOfAutoGeneratedMiplevels(baseLevel, maxLevel, isMiplevelsNumEstimated);
                    GT_IF_WITH_ASSERT(rc2)
                    {
                        retVal = true;

                        // Do not regenrate level 0:
                        if (baseLevel == 0)
                        {
                            baseLevel ++;
                        }

                        float w = (float)width;
                        float h = (float)height;
                        float d = (float)depth;

                        // Generate the levels:
                        for (unsigned int level = baseLevel; level <= maxLevel; level++)
                        {
                            // Get this level dimensions:
                            w = max(1.0F, (float)floor(w / 2));

                            apTextureType texType = textureType();

                            // 1D textures do not have height. 1D texture arrays' height does not shrink with mip levels:
                            if ((texType != AP_1D_TEXTURE) && (texType != AP_1D_ARRAY_TEXTURE))
                            {
                                h = max(1.0F, (float)floor(h / 2));
                            }

                            // Only 3D textures (and 2D array textures) have depth, but 2D array textures' depth does not shrink with mip levels:
                            if (texType == AP_3D_TEXTURE)
                            {
                                d = max(1.0F, (float)floor(d / 2));
                            }

                            // Check number of texture panes:
                            apGLTextureMipLevel::apTextureFaceIndex firstFaceIndex = apGLTextureMipLevel::AP_SINGLE_TEXTURE_FACE_INDEX;
                            apGLTextureMipLevel::apTextureFaceIndex lastFaceIndex = apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_NEGATIVE_X_FACE_INDEX;

                            if (AP_CUBE_MAP_TEXTURE == texType)
                            {
                                firstFaceIndex = apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_POSITIVE_X_FACE_INDEX;
                                lastFaceIndex = apGLTextureMipLevel::AP_MAX_AMOUNT_OF_TEXTURE_FACES;
                            }

                            // Update the parameters for each texture pane:
                            for (int textureFace = firstFaceIndex; textureFace < lastFaceIndex; textureFace++)
                            {
                                // Convert int to texture face index:
                                apGLTextureMipLevel::apTextureFaceIndex faceIndex = (apGLTextureMipLevel::apTextureFaceIndex)textureFace;

                                // Get level 0 internal format:
                                GLint internalFormat = pTextureMipLevel0->getUsedInternalPixelFormat(faceIndex);

                                // Get the level 0 texel type:
                                GLenum texelType = pTextureMipLevel0->getTexelsType();

                                // Get the level 0 pixel format:
                                GLenum format = pTextureMipLevel0->getPixelFormat();

                                // Add the new mip level:
                                bool rc3 = addTextureMipLevel(level, internalFormat, (GLsizei)w, (GLsizei)h, (GLsizei)d, (GLsizei)borderSize, format, texelType);
                                GT_ASSERT(rc3);

                                retVal = retVal && rc3;
                            }

                            // Mark all texture faces as dirty:
                            if (texType != AP_CUBE_MAP_TEXTURE)
                            {
                                GLenum bindTarget = apTextureTypeToTextureBindTarget(texType);
                                markTextureAsDirty(bindTarget);
                            }
                            else
                            {
                                markTextureAsDirty(GL_TEXTURE_CUBE_MAP_POSITIVE_X);
                                markTextureAsDirty(GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
                                markTextureAsDirty(GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
                                markTextureAsDirty(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
                                markTextureAsDirty(GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
                                markTextureAsDirty(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
                            }
                        }
                    }
                }
                else
                {
                    // Nothing to be done, texture is not initialized yet:
                    retVal = true;
                }
            }
        }
    }
    else
    {
        // Nothing to be done:
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsGLTexture::getMipmapBaseMaxLevels
// Description: Returns the user set max and base levels
// Arguments: GLuint& baseLevel
//            GLuint& maxLevel
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/1/2009
// ---------------------------------------------------------------------------
bool gsGLTexture::getMipmapBaseMaxLevels(GLuint& baseLevel, GLuint& maxLevel)const
{
    bool retVal = false;

    // Try to get the levels from the texture parameters:
    bool rc = apGLTexture::getMipmapBaseMaxLevels(baseLevel, maxLevel);

    if (!rc || _isOpenGLESTexture)
    {
        // The parameters are probably not updated:
        baseLevel = _mipmapBaseLevel;
        maxLevel = _mipmapMaxLevel;
        retVal = true;
    }
    else
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsGLTexture::getTextureMipmapType
// Description: Returns the texture mipmap type
//              Overrider apGLTexture implementation because apGLTexture's
//              implementation is based upon parameters that aren't always updated
// Return Val: apTextureMipMapType
// Author:      Sigal Algranaty
// Date:        7/10/2008
// ---------------------------------------------------------------------------
apTextureMipMapType gsGLTexture::getTextureMipmapType()const
{
    apTextureMipMapType retVal = AP_MIPMAP_NONE;

    if (_shouldAutoGenerateMipmap)
    {
        retVal = AP_MIPMAP_AUTO_GENERATE;
    }
    else
    {
        retVal = apGLTexture::getTextureMipmapType();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsGLTexture::dirtyTextureRawDataExists
// Description: Texture dirty raw data. If the texture is bound to active FBO,
//              the texture might be draw target, so it is dirty by definition
// Arguments: int level
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        30/8/2009
// ---------------------------------------------------------------------------
bool gsGLTexture::dirtyTextureRawDataExists(int level) const
{
    bool retVal = true;

    if (_isBoundToActiveFBO)
    {
        // Let the miplevel know it is about to be updated as dirty. Cast the texture object to non-const,
        // so the texture level can become ready for the update:
        gsGLTexture* pNonConstMe = (gsGLTexture*)this;

        // Go over all the texture's bind targets and mark them as dirty:
        // Mark all texture faces as dirty:
        apTextureType texType = textureType();

        if (texType != AP_CUBE_MAP_TEXTURE)
        {
            GLenum bindTarget = apTextureTypeToTextureBindTarget(texType);
            pNonConstMe->markTextureAsDirty(bindTarget, level);
        }
        else // textureType() == AP_CUBE_MAP_TEXTURE
        {
            pNonConstMe->markTextureAsDirty(GL_TEXTURE_CUBE_MAP_POSITIVE_X, level);
            pNonConstMe->markTextureAsDirty(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, level);
            pNonConstMe->markTextureAsDirty(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, level);
            pNonConstMe->markTextureAsDirty(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, level);
            pNonConstMe->markTextureAsDirty(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, level);
            pNonConstMe->markTextureAsDirty(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, level);
        }
    }
    else // !_isBoundToActiveFBO
    {
        // Actually check if the level is dirty:
        retVal = apGLTexture::dirtyTextureRawDataExists(level);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsGLTexture::dirtyTextureImageExists
// Description: Texture dirty image. If the texture is bound to active FBO,
//              the texture might be draw target, so it is dirty by definition
// Arguments: int level
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        30/8/2009
// ---------------------------------------------------------------------------
bool gsGLTexture::dirtyTextureImageExists(int level) const
{
    bool retVal = _isBoundToActiveFBO;

    if (!retVal)
    {
        retVal = apGLTexture::dirtyTextureImageExists(level);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsGLTexture::isMemoryParameter
// Description: Checks if the parameter is relevant for the texture memory check
// Arguments:   GLenum parameterName
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        30/5/2010
// ---------------------------------------------------------------------------
bool gsGLTexture::isMemoryParameter(GLenum parameterName)
{
    bool retVal = false;

    switch (parameterName)
    {
        case GL_TEXTURE_BASE_LEVEL:
        case GL_TEXTURE_MAX_LEVEL:
        case GL_GENERATE_MIPMAP:
        case GL_TEXTURE_WIDTH:
        case GL_TEXTURE_HEIGHT:
        case GL_TEXTURE_DEPTH:
        case GL_TEXTURE_INTERNAL_FORMAT:
        case GL_TEXTURE_BORDER:
        case GL_TEXTURE_RED_SIZE:
        case GL_TEXTURE_GREEN_SIZE:
        case GL_TEXTURE_BLUE_SIZE:
        case GL_TEXTURE_ALPHA_SIZE:
        case GL_TEXTURE_LUMINANCE_SIZE:
        case GL_TEXTURE_INTENSITY_SIZE:
        case GL_TEXTURE_DEPTH_SIZE:
        case GL_TEXTURE_SHARED_SIZE:
        case GL_TEXTURE_STENCIL_SIZE:
        case GL_TEXTURE_COMPRESSED:
        case GL_TEXTURE_COMPRESSED_IMAGE_SIZE:
            retVal = true;
            break;

        default:
            retVal = false;
            break;

    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsGLTexture::isTextureCompressed
// Description: Checks if the texture is compressed
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        17/1/2013
// ---------------------------------------------------------------------------
bool gsGLTexture::isTextureCompressed(const apGLTextureParams& textureParams, bool& isCompressed) const
{
    bool retVal = false;

    // Check if the image is compressed:
    GLboolean isCompressedGL = GL_FALSE;
    bool rc = textureParams.getTextureBoolenParameterValue(GL_TEXTURE_COMPRESSED, isCompressedGL);

    if (rc)
    {
        retVal = true;
        isCompressed = (isCompressedGL != GL_FALSE);
    }

    return retVal;
}


