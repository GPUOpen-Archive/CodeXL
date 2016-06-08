//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLTextureParams.cpp
///
//==================================================================================

// -----------------------------   apGLTextureParams.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/apOpenGLParameters.h>
#include <AMDTAPIClasses/Include/apInternalFormat.h>
#include <AMDTAPIClasses/Include/ap2DRectangle.h>
#include <AMDTAPIClasses/Include/apGLTextureParams.h>

// Static members initializations:
apNotAvailableParameter apGLTextureParams::_stat_NotAvailableParameter;


// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::apGLTextureParams
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        19/12/2004
// ---------------------------------------------------------------------------
apGLTextureParams::apGLTextureParams()
{
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::gsGLTexture
// Description: Copy constructor
// Arguments: other - The other texture class from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        2/7/2006
// ---------------------------------------------------------------------------
apGLTextureParams::apGLTextureParams(const apGLTextureParams& other)
{
    apGLTextureParams::operator=(other);
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::~apGLTextureParams
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        25/12/2004
// ---------------------------------------------------------------------------
apGLTextureParams::~apGLTextureParams()
{
    // Delete texture parameters:
    deleteAllParameters();
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::operator=
// Description: Assignment operator
// Arguments:   other - The other object from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        30/12/2004
// ---------------------------------------------------------------------------
apGLTextureParams& apGLTextureParams::operator=(const apGLTextureParams& other)
{
    // Clear allocated values:
    deleteAllParameters();

    // Copy the texture parameter name values from other:
    int paramsAmount = other.amountOfTextureParameters();

    for (int i = 0; i < paramsAmount; i++)
    {
        apGLTextureParameter* pParam = other._textureParameters[i];

        if (pParam != NULL)
        {
            apGLTextureParameter* pCurrOtherParameterObj = new apGLTextureParameter(*pParam);
            _textureParameters.push_back(pCurrOtherParameterObj);
        }
    }

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::addDefaultGLTextureLevelsParameters
// Description: Adds the OpenGL default texture level parameters.
// Author:  AMD Developer Tools Team
// Date:        28/10/2008
// ---------------------------------------------------------------------------
void apGLTextureParams::addDefaultGLTextureParameters()
{
    // Add GL_TEXTURE_MIN_FILTER:
    gtAutoPtr<apParameter> aptrMinFilter = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pMinFilterParam = new apGLTextureParameter(GL_TEXTURE_MIN_FILTER, aptrMinFilter);
    _textureParameters.push_back(pMinFilterParam);

    // Add GL_TEXTURE_MAG_FILTER:
    gtAutoPtr<apParameter> aptrMagFilter = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pMagFilterParam = new apGLTextureParameter(GL_TEXTURE_MAG_FILTER, aptrMagFilter);
    _textureParameters.push_back(pMagFilterParam);

    // Add GL_TEXTURE_WRAP_S:
    gtAutoPtr<apParameter> aptrWrapS = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pWrapSParam = new apGLTextureParameter(GL_TEXTURE_WRAP_S, aptrWrapS);
    _textureParameters.push_back(pWrapSParam);

    // Add GL_TEXTURE_WRAP_T:
    gtAutoPtr<apParameter> aptrWrapT = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pWrapTParam = new apGLTextureParameter(GL_TEXTURE_WRAP_T, aptrWrapT);
    _textureParameters.push_back(pWrapTParam);

    // Add GL_TEXTURE_WRAP_R:
    gtAutoPtr<apParameter> aptrWrapR = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pWrapRParam = new apGLTextureParameter(GL_TEXTURE_WRAP_R, aptrWrapR);
    _textureParameters.push_back(pWrapRParam);

    // Add GL_TEXTURE_MIN_LOD:
    gtAutoPtr<apParameter> aptrMinLOD = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pMinLODParam = new apGLTextureParameter(GL_TEXTURE_MIN_LOD, aptrMinLOD);
    _textureParameters.push_back(pMinLODParam);

    // Add GL_TEXTURE_MAX_LOD:
    gtAutoPtr<apParameter> aptrMaxLOD = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pMaxLODParam = new apGLTextureParameter(GL_TEXTURE_MAX_LOD, aptrMaxLOD);
    _textureParameters.push_back(pMaxLODParam);

    // Add GL_TEXTURE_BASE_LEVEL:
    gtAutoPtr<apParameter> aptrBaseLevel = new apGLintParameter(0);
    apGLTextureParameter* pBaseLevelParam = new apGLTextureParameter(GL_TEXTURE_BASE_LEVEL, aptrBaseLevel);
    _textureParameters.push_back(pBaseLevelParam);

    // Add GL_TEXTURE_MAX_LEVEL:
    gtAutoPtr<apParameter> aptrMaxLevel = new apGLintParameter(0);
    apGLTextureParameter* pMaxLevelParam = new apGLTextureParameter(GL_TEXTURE_MAX_LEVEL, aptrMaxLevel);
    _textureParameters.push_back(pMaxLevelParam);

    // Add GL_GENERATE_MIPMAP:
    gtAutoPtr<apParameter> aptrGenerateMipMap = new apGLbooleanParameter(GL_FALSE);
    apGLTextureParameter* pGenerateMipMapParam = new apGLTextureParameter(GL_GENERATE_MIPMAP, aptrGenerateMipMap);
    _textureParameters.push_back(pGenerateMipMapParam);

    // Add GL_TEXTURE_PRIORITY:
    gtAutoPtr<apParameter> aptrTexturePriority = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTexturePriorityParam = new apGLTextureParameter(GL_TEXTURE_PRIORITY, aptrTexturePriority);
    _textureParameters.push_back(pTexturePriorityParam);

    // Add GL_TEXTURE_BORDER_COLOR:
    apVectorParameter* pBorderColor = new apVectorParameter(4);

    // Initialize the border color to (0,0,0,0):
    for (int i = 0; i < 4; i++)
    {
        apParameter* pCurrentColorVecComponent = new apGLfloatParameter(0.0f);
        gtAutoPtr<apParameter> aptrVecComponent = pCurrentColorVecComponent;
        pBorderColor->setItem(i, aptrVecComponent);
    }

    gtAutoPtr<apParameter> aptrBorderColor = pBorderColor;
    apGLTextureParameter* pBorderColorParam = new apGLTextureParameter(GL_TEXTURE_BORDER_COLOR, aptrBorderColor);
    _textureParameters.push_back(pBorderColorParam);

    // Add GL_TEXTURE_RESIDENT:
    gtAutoPtr<apParameter> aptrTextureResident = new apGLbooleanParameter(GL_FALSE);
    apGLTextureParameter* pTextureResidentParam = new apGLTextureParameter(GL_TEXTURE_RESIDENT, aptrTextureResident);
    _textureParameters.push_back(pTextureResidentParam);

    // Add GL_TEXTURE_COMPARE_MODE:
    gtAutoPtr<apParameter> aptrCompareMode = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pCompareModeParam = new apGLTextureParameter(GL_TEXTURE_COMPARE_MODE, aptrCompareMode);
    _textureParameters.push_back(pCompareModeParam);

    // Add GL_TEXTURE_COMPARE_FUNC:
    gtAutoPtr<apParameter> aptrCompareFunc = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pCompareFuncParam = new apGLTextureParameter(GL_TEXTURE_COMPARE_FUNC, aptrCompareFunc);
    _textureParameters.push_back(pCompareFuncParam);

    // Add GL_DEPTH_TEXTURE_MODE:
    gtAutoPtr<apParameter> aptrDepthTextureMode = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pDepthTextureparam = new apGLTextureParameter(GL_DEPTH_TEXTURE_MODE, aptrDepthTextureMode);
    _textureParameters.push_back(pDepthTextureparam);

    // Add GL_TEXTURE_SWIZZLE_R:
    gtAutoPtr<apParameter> aptrTextureSwizzleR = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pSwizzleRParam = new apGLTextureParameter(GL_TEXTURE_SWIZZLE_R, aptrTextureSwizzleR);
    _textureParameters.push_back(pSwizzleRParam);

    // Add GL_TEXTURE_SWIZZLE_G:
    gtAutoPtr<apParameter> aptrTextureSwizzleG = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pSwizzleGParam = new apGLTextureParameter(GL_TEXTURE_SWIZZLE_G, aptrTextureSwizzleG);
    _textureParameters.push_back(pSwizzleGParam);

    // Add GL_TEXTURE_SWIZZLE_B:
    gtAutoPtr<apParameter> aptrTextureSwizzleB = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pSwizzleBParam = new apGLTextureParameter(GL_TEXTURE_SWIZZLE_B, aptrTextureSwizzleB);
    _textureParameters.push_back(pSwizzleBParam);

    // Add GL_TEXTURE_SWIZZLE_A:
    gtAutoPtr<apParameter> aptrTextureSwizzleA = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pSwizzleAParam = new apGLTextureParameter(GL_TEXTURE_SWIZZLE_A, aptrTextureSwizzleA);
    _textureParameters.push_back(pSwizzleAParam);
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::addDefaultGLTextureLevelParameters
// Description: Add the default texture level parameters
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        30/10/2008
// ---------------------------------------------------------------------------
void apGLTextureParams::addDefaultGLTextureLevelParameters(bool isNVIDIAPlatform)
{
    // Add GL_TEXTURE_WIDTH:
    gtAutoPtr<apParameter> aptrTextureWidth = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureWidthParam = new apGLTextureParameter(GL_TEXTURE_WIDTH, aptrTextureWidth);
    _textureParameters.push_back(pTextureWidthParam);

    // Add GL_TEXTURE_HEIGHT:
    gtAutoPtr<apParameter> aptrTextureHeight = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureHeightParam = new apGLTextureParameter(GL_TEXTURE_HEIGHT, aptrTextureHeight);
    _textureParameters.push_back(pTextureHeightParam);

    // Add GL_TEXTURE_DEPTH:
    gtAutoPtr<apParameter> aptrTextureDepth = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureDepthParam = new apGLTextureParameter(GL_TEXTURE_DEPTH, aptrTextureDepth);
    _textureParameters.push_back(pTextureDepthParam);

    // Add GL_TEXTURE_INTERNAL_FORMAT:
    gtAutoPtr<apParameter> aptrInternalFormat = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pInternalFormatParam = new apGLTextureParameter(GL_TEXTURE_INTERNAL_FORMAT, aptrInternalFormat);
    _textureParameters.push_back(pInternalFormatParam);

    // Add GL_TEXTURE_BORDER:
    gtAutoPtr<apParameter> aptrTextureBorder = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureBorder = new apGLTextureParameter(GL_TEXTURE_BORDER, aptrTextureBorder);
    _textureParameters.push_back(pTextureBorder);

    // Add GL_TEXTURE_RED_SIZE:
    gtAutoPtr<apParameter> aptrTextureRedSize = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureRedSizeParam = new apGLTextureParameter(GL_TEXTURE_RED_SIZE, aptrTextureRedSize);
    _textureParameters.push_back(pTextureRedSizeParam);

    // Add GL_TEXTURE_GREEN_SIZE:
    gtAutoPtr<apParameter> aptrTextureGreenSize = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureGreenSizeParam = new apGLTextureParameter(GL_TEXTURE_GREEN_SIZE, aptrTextureGreenSize);
    _textureParameters.push_back(pTextureGreenSizeParam);

    // Add GL_TEXTURE_BLUE_SIZE:
    gtAutoPtr<apParameter> aptrTextureBlueSize = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureBlueSizeParam = new apGLTextureParameter(GL_TEXTURE_BLUE_SIZE, aptrTextureBlueSize);
    _textureParameters.push_back(pTextureBlueSizeParam);

    // Add GL_TEXTURE_ALPHA_SIZE:
    gtAutoPtr<apParameter> aptrTextureAlphaSize = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureAlphaSizeParam = new apGLTextureParameter(GL_TEXTURE_ALPHA_SIZE, aptrTextureAlphaSize);
    _textureParameters.push_back(pTextureAlphaSizeParam);

    // Add GL_TEXTURE_LUMINANCE_SIZE:
    gtAutoPtr<apParameter> aptrTextureLuminanceSize = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureLuminanceSizeParam = new apGLTextureParameter(GL_TEXTURE_LUMINANCE_SIZE, aptrTextureLuminanceSize);
    _textureParameters.push_back(pTextureLuminanceSizeParam);

    // Add GL_TEXTURE_INTENSITY_SIZE:
    gtAutoPtr<apParameter> aptrTextureInensitySize = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureInensitySizeParam = new apGLTextureParameter(GL_TEXTURE_INTENSITY_SIZE, aptrTextureInensitySize);
    _textureParameters.push_back(pTextureInensitySizeParam);

    // Add GL_TEXTURE_DEPTH_SIZE:
    gtAutoPtr<apParameter> aptrTextureDepthSize = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureDepthSizeParam = new apGLTextureParameter(GL_TEXTURE_DEPTH_SIZE, aptrTextureDepthSize);
    _textureParameters.push_back(pTextureDepthSizeParam);

    // Add GL_TEXTURE_SHARED_SIZE:
    gtAutoPtr<apParameter> aptrTextureSharedSize = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureSharedSizeParam = new apGLTextureParameter(GL_TEXTURE_SHARED_SIZE, aptrTextureSharedSize);
    _textureParameters.push_back(pTextureSharedSizeParam);

    // Add GL_TEXTURE_STENCIL_SIZE:
    gtAutoPtr<apParameter> aptrTextureStencilSize = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureStencilSizeParam = new apGLTextureParameter(GL_TEXTURE_STENCIL_SIZE, aptrTextureStencilSize);
    _textureParameters.push_back(pTextureStencilSizeParam);

    // Add GL_TEXTURE_COMPRESSED:
    gtAutoPtr<apParameter> aptrTextureCompressed = new apGLbooleanParameter(GL_FALSE);
    apGLTextureParameter* pTextureCompressedParam = new apGLTextureParameter(GL_TEXTURE_COMPRESSED, aptrTextureCompressed);
    _textureParameters.push_back(pTextureCompressedParam);

    // Add GL_TEXTURE_COMPRESSED_IMAGE_SIZE:
    gtAutoPtr<apParameter> aptrTextureCompressedImageSize = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureCompressedImageSizeParam = new apGLTextureParameter(GL_TEXTURE_COMPRESSED_IMAGE_SIZE, aptrTextureCompressedImageSize);
    _textureParameters.push_back(pTextureCompressedImageSizeParam);

    // Add GL_TEXTURE_RED_TYPE:
    gtAutoPtr<apParameter> aptrTextureRedType = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pTextureRedTypeParam = new apGLTextureParameter(GL_TEXTURE_RED_TYPE, aptrTextureRedType);
    _textureParameters.push_back(pTextureRedTypeParam);

    // Add GL_TEXTURE_GREEN_TYPE:
    gtAutoPtr<apParameter> aptrTextureGreenType = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pTextureGreenTypeParam = new apGLTextureParameter(GL_TEXTURE_GREEN_TYPE, aptrTextureGreenType);
    _textureParameters.push_back(pTextureGreenTypeParam);

    // Add GL_TEXTURE_BLUE_TYPE:
    gtAutoPtr<apParameter> aptrTextureBlueType = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pTextureBlueTypeParam = new apGLTextureParameter(GL_TEXTURE_BLUE_TYPE, aptrTextureBlueType);
    _textureParameters.push_back(pTextureBlueTypeParam);

    // Add GL_TEXTURE_ALPHA_TYPE:
    gtAutoPtr<apParameter> aptrTextureAlphaType = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pTextureAlphaTypeParam = new apGLTextureParameter(GL_TEXTURE_ALPHA_TYPE, aptrTextureAlphaType);
    _textureParameters.push_back(pTextureAlphaTypeParam);

    // Add GL_TEXTURE_DEPTH_TYPE:
    gtAutoPtr<apParameter> aptrTextureDepthType = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pTextureDepthTypeParam = new apGLTextureParameter(GL_TEXTURE_DEPTH_TYPE, aptrTextureDepthType);
    _textureParameters.push_back(pTextureDepthTypeParam);

    // Add multisample texture extension parameters:
    addMultisampleTextureShaderParameters();

    if (isNVIDIAPlatform)
    {
        // Add NVIDIA texture shader extension size parameters:
        addNVTextureShaderParameters();
    }
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::addNVTextureShaderParameters
// Description: Add GL_texture_shader extension parameters
//              These parameters are used for size calculations, therefore, we
//              want to add it separately, only when the application supports
//              GL_texture_shader extension
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        1/1/2009
// ---------------------------------------------------------------------------
void apGLTextureParams::addNVTextureShaderParameters()
{
    // Add GL_TEXTURE_HI_SIZE_NV:
    gtAutoPtr<apParameter> aptrTextureHiSizeNV = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureHiSizeNVParam = new apGLTextureParameter(GL_TEXTURE_HI_SIZE_NV, aptrTextureHiSizeNV);
    _textureParameters.push_back(pTextureHiSizeNVParam);

    // Add GL_TEXTURE_LO_SIZE_NV:
    gtAutoPtr<apParameter> aptrTextureLoSizeNV = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureLoSizeNVParam = new apGLTextureParameter(GL_TEXTURE_LO_SIZE_NV, aptrTextureLoSizeNV);
    _textureParameters.push_back(pTextureLoSizeNVParam);

    // Add GL_TEXTURE_DS_SIZE_NV:
    gtAutoPtr<apParameter> aptrTextureDsSizeNV = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureDsSizeNVParam = new apGLTextureParameter(GL_TEXTURE_DS_SIZE_NV, aptrTextureDsSizeNV);
    _textureParameters.push_back(pTextureDsSizeNVParam);

    // Add GL_TEXTURE_DT_SIZE_NV:
    gtAutoPtr<apParameter> aptrTextureDtSizeNV = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureDtSizeNVParam = new apGLTextureParameter(GL_TEXTURE_DT_SIZE_NV, aptrTextureDtSizeNV);
    _textureParameters.push_back(pTextureDtSizeNVParam);

    // Add GL_TEXTURE_MAG_SIZE_NV:
    gtAutoPtr<apParameter> aptrTextureMagSizeNV = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureMagSizeNVParam = new apGLTextureParameter(GL_TEXTURE_MAG_SIZE_NV, aptrTextureMagSizeNV);
    _textureParameters.push_back(pTextureMagSizeNVParam);
}




// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::addMultisampleTextureShaderParameters
// Description: Add GL_multisample_texture extension parameters
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        5/11/2009
// ---------------------------------------------------------------------------
void apGLTextureParams::addMultisampleTextureShaderParameters()
{
    // Add GL_TEXTURE_SAMPLES:
    gtAutoPtr<apParameter> aptrTextureSamples = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureSamplesParam = new apGLTextureParameter(GL_TEXTURE_SAMPLES, aptrTextureSamples);
    _textureParameters.push_back(pTextureSamplesParam);

    // Add GL_TEXTURE_FIXED_SAMPLE_LOCATIONS:
    gtAutoPtr<apParameter> aptrTextureFixedSampleLocation = new apGLfloatParameter(0.0f);
    apGLTextureParameter* pTextureFixedSampleLocationParam = new apGLTextureParameter(GL_TEXTURE_FIXED_SAMPLE_LOCATIONS, aptrTextureFixedSampleLocation);
    _textureParameters.push_back(pTextureFixedSampleLocationParam);
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::addDefaultGLESTextureLevelParameters
// Description: Add the default parameters for texture level for OpenGLES
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        30/10/2008
// ---------------------------------------------------------------------------
void apGLTextureParams::addDefaultGLESTextureLevelParameters()
{
    // Do nothing - no OpenGLES texture level parameters
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::addDefaultGLESTextureParameters
// Description: Adds the OpenGL ES default texture parameters to texture mip level 0.
//              TODO: implement for all levels
// Return Val: bool - success / failure.
// Date:        25/12/2004
// ---------------------------------------------------------------------------
void apGLTextureParams::addDefaultGLESTextureParameters()
{
    // Add GL_TEXTURE_MIN_FILTER:
    gtAutoPtr<apParameter> aptrMinFilter = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pMinFilterParam = new apGLTextureParameter(GL_TEXTURE_MIN_FILTER, aptrMinFilter);
    _textureParameters.push_back(pMinFilterParam);

    // Add GL_TEXTURE_MAG_FILTER:
    gtAutoPtr<apParameter> aptrMagFilter = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pMagFilterParam = new apGLTextureParameter(GL_TEXTURE_MAG_FILTER, aptrMagFilter);
    _textureParameters.push_back(pMagFilterParam);

    // Add GL_TEXTURE_WRAP_S:
    gtAutoPtr<apParameter> aptrWrapS = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pWrapSParam = new apGLTextureParameter(GL_TEXTURE_WRAP_S, aptrWrapS);
    _textureParameters.push_back(pWrapSParam);

    // Add GL_TEXTURE_WRAP_T:
    gtAutoPtr<apParameter> aptrWrapT = new apGLenumParameter(GL_NONE);
    apGLTextureParameter* pWrapTParam = new apGLTextureParameter(GL_TEXTURE_WRAP_T, aptrWrapT);
    _textureParameters.push_back(pWrapTParam);

    // Add GL_GENERATE_MIPMAP:
    gtAutoPtr<apParameter> aptrGenerateMipMap = new apGLbooleanParameter(GL_FALSE);
    apGLTextureParameter* pGenerateMipMapParam = new apGLTextureParameter(GL_GENERATE_MIPMAP, aptrGenerateMipMap);
    _textureParameters.push_back(pGenerateMipMapParam);
}


// ---------------------------------------------------------------------------
// Name:        createParameterObj
// Description: Creates an apParameter obj that will contain an input texture
//              parameter value.
// Arguments:   parameterName - The input parameter name.
//              parameterValueType - The type of the input parameter value.
//              parameterValue - The input parameter value.
//
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
apParameter* apGLTextureParams::createParameterObj(GLenum parameterName, osTransferableObjectType parameterValueType, void* pParameterValue)
{
    apParameter* retVal = NULL;

    // Get the parameter type (as defined in the OpenGL spec):
    osTransferableObjectType parameterType = OS_TOBJ_ID_GL_INT_PARAMETER;
    int amountOfParameterItems = 0;
    getParameterType(parameterName, parameterType, amountOfParameterItems);

    if (amountOfParameterItems == 1)
    {
        // Create an apOpeGLParameter object that will contain the texture parameter value:
        retVal = createSingleItemParameterObj(parameterType, parameterValueType, pParameterValue);
    }
    else // amountOfParameterItems > 1
    {
        // Create a vector parameter:
        apVectorParameter* pVectorParam = new apVectorParameter(amountOfParameterItems);

        if (nullptr != pVectorParam)
        {
            // Iterate the vector items:
            GLbyte* pCurrentValue = (GLbyte*)pParameterValue;

            for (int i = 0; i < amountOfParameterItems; i++)
            {
                // Create an apOpeGLParameter object that will contain the texture parameter value:
                apOpenGLParameter* pOpenGLParameter = createSingleItemParameterObj(parameterType, parameterValueType, pCurrentValue);

                if (pOpenGLParameter)
                {
                    // Add it to the vector:
                    gtAutoPtr<apParameter> aptrParam = (apParameter*)pOpenGLParameter;
                    pVectorParam->setItem(i, aptrParam);

                    // Advance the current value pointer to the next value:
                    gtSize_t dataSize = pOpenGLParameter->sizeofData();
                    pCurrentValue += dataSize;
                }
            }

            retVal = pVectorParam;
        }
    }

    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::createSingleItemParameterObj
// Description: Create a single item (not vector / matrix) parameter object.
// Arguments:   parameterType - The parameter type.
//              parameterValueType - The type in which the parameter value is given.
//              pParameterValue - Pointer to the parameter value type (casted into void*).
// Return Val:  apParameter* - Will get the created parameter pointer. It is the caller
//                             responsibility to delete this object.
// Author:  AMD Developer Tools Team
// Date:        11/4/2005
// ---------------------------------------------------------------------------
apOpenGLParameter* apGLTextureParams::createSingleItemParameterObj(osTransferableObjectType parameterType, osTransferableObjectType parameterValueType, void* pParameterValue)
{
    apOpenGLParameter* retVal = NULL;

    // Create an apOpeGLParameter object that will contain the texture parameter value:
    apOpenGLParameter* pOpenGLParameter = apCreateOpenGLParameterObject(parameterType);

    if (NULL != pOpenGLParameter)
    {
        switch (parameterValueType)
        {
            case OS_TOBJ_ID_GL_INT_PARAMETER:
            case OS_TOBJ_ID_GL_ENUM_PARAMETER:
            case OS_TOBJ_ID_GL_BOOL_PARAMETER:
            {
                GLint parameterValueAsInt = *((GLint*)pParameterValue);
                pOpenGLParameter->setValueFromInt(parameterValueAsInt);
            }
            break;

            case OS_TOBJ_ID_GL_FLOAT_PARAMETER:
            {
                GLfloat parameterValueAsFloat = *((GLfloat*)pParameterValue);
                pOpenGLParameter->setValueFromFloat(parameterValueAsFloat);
            }
            break;

            default:
            {
                gtString errMsg;
                errMsg.appendFormattedString(L"Failed to set value of OpenGL parameter of type %u", parameterValueType);
                GT_ASSERT_EX(false, errMsg.asCharArray());
                break;
            }
        }

        retVal = pOpenGLParameter;
    }
    else // NULL == pOpenGLParameter
    {
        gtString errMsg;
        errMsg.appendFormattedString(L"Failed to create OpenGL parameter of type %u", parameterType);
        GT_ASSERT_EX(false, errMsg.asCharArray());
    }

    GT_ASSERT(retVal != NULL);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::setTextureParameterValueFromFloat
// Description: Set a parameter value from float pointer
//              The function convert the float pointer to the parameter value type,
//              and then set the value
// Arguments: GLenum parameterName
//            GLfloat* pParameterValue
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        2/11/2008
// ---------------------------------------------------------------------------
void apGLTextureParams::setTextureParameterValueFromFloat(GLenum parameterName, GLfloat* pParameterValue)
{
    // Get the parameter type:
    osTransferableObjectType parameterValueType = OS_AMOUNT_OF_TRANSFERABLE_OBJECT_TYPES;
    int amountOfItems = -1;
    getParameterType(parameterName, parameterValueType, amountOfItems);

    // Convert the void pointer according to the parameter type:
    void* pValueAsVoidPointer = NULL;
    GLint paramValAsInt = 0;

    if ((parameterValueType == OS_TOBJ_ID_GL_INT_PARAMETER) || (parameterValueType == OS_TOBJ_ID_GL_ENUM_PARAMETER) || (parameterValueType == OS_TOBJ_ID_GL_BOOL_PARAMETER))
    {
        paramValAsInt = (GLint)(*pParameterValue);
        pValueAsVoidPointer = (void*)&paramValAsInt;
    }
    else if (parameterValueType == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
    {
        pValueAsVoidPointer = (void*)pParameterValue;
    }
    else
    {
        GT_ASSERT(0);
    }

    // Set the parameter value from void pointer:
    setTextureParameterValueFromFloat(parameterName, pValueAsVoidPointer);
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::setTextureParameterValueFromFloat
// Description: Sets a parameter value.
//              * Create a new parameter in case of non existing parameter  The type of the new parameter is
//                either parameterValueType or concluded from the parameter name.
//              * pParameterValue is supposed to contain a pointer to the right value type (float* or int* - depends on the parameter type)
// Arguments: GLenum parameterName
//            void* pParameterValue
//            osTransferableObjectType parameterValueType
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        10/11/2008
// ---------------------------------------------------------------------------
void apGLTextureParams::setTextureParameterValueFromFloat(GLenum parameterName, void* pParameterValue, osTransferableObjectType parameterValueType)
{
    int amountOfItems = 1;

    // If the user did not request for a specific parameter type:
    if (parameterValueType == OS_TOBJ_ID_NOT_AVAILABLE_PARAMETER)
    {
        // Get the parameter type from the parameter name:
        getParameterType(parameterName, parameterValueType, amountOfItems);
    }

    // Get the parameter index (in this class vectors):
    int paramIndex = getTextureParameterIndex(parameterName);

    // If the param value does not exists in this class:
    if (paramIndex < 0)
    {
        // Create a parameter that contains the new texture parameter value:
        apParameter* pNewParamVal = createParameterObj(parameterName, parameterValueType, pParameterValue);
        GT_IF_WITH_ASSERT(pNewParamVal != NULL)
        {
            // Add the new parameter value to this class vector:
            gtAutoPtr<apParameter> aptrParamVal = pNewParamVal;
            apGLTextureParameter* pTextureParam = new apGLTextureParameter(parameterName, aptrParamVal);
            _textureParameters.push_back(pTextureParam);
        }
    }
    else
    {
        // Get the texture parameter object:
        apGLTextureParameter* pTextureParameter = _textureParameters[paramIndex];
        GT_IF_WITH_ASSERT(pTextureParameter != NULL)
        {
            // Replace the old value by the new value:
            const apParameter* pParamVal = pTextureParameter->_aptrParameterValue.pointedObject();

            if (amountOfItems == 1)
            {
                if ((pParamVal == NULL) || (pParamVal->type() != parameterValueType))
                {
                    // Create a parameter that contains the new texture parameter value:
                    apParameter* pNewParamVal = createParameterObj(parameterName, parameterValueType, pParameterValue);
                    GT_IF_WITH_ASSERT(pNewParamVal != NULL)
                    {
                        pTextureParameter->_aptrParameterValue = pNewParamVal;
                    }
                }
                else
                {
                    pTextureParameter->_aptrParameterValue->readValueFromPointer(pParameterValue);
                }
            }
            else
            {
                GT_IF_WITH_ASSERT(pParamVal->type() == OS_TOBJ_ID_VECTOR_PARAMETER)
                {
                    // Convert the parameter to a vector parameter and set its values:
                    apVectorParameter* pVectorParameter = (apVectorParameter*)pParamVal;
                    GLfloat* pFloatValues = (GLfloat*)pParameterValue;

                    // Loop the values and set each of them:
                    for (int i = 0; i < amountOfItems; i++)
                    {
                        apParameter* pParam = (apParameter*)(*pVectorParameter)[i];
                        GT_IF_WITH_ASSERT(pParam->type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
                        {
                            apGLfloatParameter* pFloatParam = (apGLfloatParameter*)pParam;
                            GLfloat floatVal = pFloatValues[i];
                            pFloatParam->setValueFromFloat(floatVal);
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::setTextureParameterIntValue
// Description: Sets an int texture parameter value.
// Arguments:   parameterName - The input parameter name.
//              parameterValue - The input parameter value.
// Author:  AMD Developer Tools Team
// Date:        22/1/2005
// ---------------------------------------------------------------------------
void apGLTextureParams::setTextureParameterIntValue(GLenum parameterName, const GLint* pParameterValue)
{
    setTextureParameterValueFromFloat(parameterName, (void*)pParameterValue, OS_TOBJ_ID_GL_INT_PARAMETER);
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::setTextureParameterUIntValue
// Description: Sets a uint texture parameter value.
// Arguments:   parameterName - The input parameter name.
//              parameterValue - The input parameter value.
// Author:  AMD Developer Tools Team
// Date:        15/9/2009
// ---------------------------------------------------------------------------
void apGLTextureParams::setTextureParameterUIntValue(GLenum parameterName, const GLuint* pParameterValue)
{
    setTextureParameterValueFromFloat(parameterName, (void*)pParameterValue, OS_TOBJ_ID_GL_UINT_PARAMETER);
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::setTextureParameterFloatValue
// Description: Sets a float texture parameter value.
// Arguments:   parameterName - The input parameter name.
//              parameterValue - The input parameter value.
// Author:  AMD Developer Tools Team
// Date:        22/1/2005
// ---------------------------------------------------------------------------
void apGLTextureParams::setTextureParameterFloatValue(GLenum parameterName, const GLfloat* pParameterValue)
{
    setTextureParameterValueFromFloat(parameterName, (void*)pParameterValue, OS_TOBJ_ID_GL_FLOAT_PARAMETER);
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::getTextureParameterValue
// Description: Inputs a texture parameter index and outputs its value.
// Arguments:   parameterIndex - The queried parameter index.
// Return Val:  const apParameter* - The queried parameter value or NULL if it does not exist.
// Author:  AMD Developer Tools Team
// Date:        19/12/2004
// ---------------------------------------------------------------------------
const apParameter* apGLTextureParams::getTextureParameterValue(int parameterIndex) const
{
    const apParameter* retVal = NULL;

    // Index range check:
    GT_IF_WITH_ASSERT((0 <= parameterIndex) && (parameterIndex < (int)_textureParameters.size()))
    {
        // Get the queried parameter:
        const apGLTextureParameter* pParameter = _textureParameters[parameterIndex];
        GT_IF_WITH_ASSERT(pParameter != NULL)
        {
            // If parameter value is NOT updated from hardware:
            if (!pParameter->_isUpdatedFromHardware)
            {
                // Return a not available parameter:
                retVal = &_stat_NotAvailableParameter;
            }
            else
            {
                retVal = pParameter->_aptrParameterValue.pointedObject();
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::getTextureBoolenParameterValue
// Description: return a parameter value as GLBoolean
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/10/2008
// ---------------------------------------------------------------------------
bool apGLTextureParams::getTextureBoolenParameterValue(GLenum parameterName, GLboolean& valueAsBool)const
{
    bool retVal = false;

    // Get parameter index:
    int paramIndex = getTextureParameterIndex(parameterName);

    if (0 <= paramIndex)
    {
        // Get the parameters value:
        const apParameter* pParamValue = getTextureParameterValue(paramIndex);

        if (pParamValue != NULL)
        {
            // Check if the parameter type is GLboolean:
            if (pParamValue->type() == OS_TOBJ_ID_GL_BOOL_PARAMETER)
            {
                // Down cast the parameter to float parameter:
                apGLbooleanParameter* pIntParameters = (apGLbooleanParameter*)pParamValue;

                // Get the value:
                valueAsBool = pIntParameters->value();
                retVal = true;
            }
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::getTextureIntParameterValue
// Description: return a parameter value as GLInt
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        25/11/2008
// ---------------------------------------------------------------------------
bool apGLTextureParams::getTextureIntParameterValue(GLenum parameterName, GLint& valueAsInt)const
{
    bool retVal = false;

    // Get parameter index:
    int paramIndex = getTextureParameterIndex(parameterName);

    if (0 <= paramIndex)
    {
        // Get the parameters value:
        const apParameter* pParamValue = getTextureParameterValue(paramIndex);

        if (pParamValue != NULL)
        {
            // Check if the parameter type is GLenum:
            if (pParamValue->type() == OS_TOBJ_ID_GL_INT_PARAMETER)
            {
                // Down cast the parameter to float parameter:
                apGLintParameter* pIntParameters = (apGLintParameter*)pParamValue;

                // Get the value:
                valueAsInt = pIntParameters->value();
                retVal = true;
            }
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::getTextureEnumParameterValue
// Description: return a parameter value as GLEnum
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/10/2008
// ---------------------------------------------------------------------------
bool apGLTextureParams::getTextureEnumParameterValue(GLenum parameterName, GLenum& valueAsEnum)const
{
    bool retVal = false;

    // Get parameter index:
    int paramIndex = getTextureParameterIndex(parameterName);

    if (0 <= paramIndex)
    {
        // Get the parameters value:
        const apParameter* pParamValue = getTextureParameterValue(paramIndex);

        if (pParamValue != NULL)
        {
            // Check if the parameter type is GLenum:
            if (pParamValue->type() == OS_TOBJ_ID_GL_ENUM_PARAMETER)
            {
                // Down cast the parameter to float parameter:
                apGLenumParameter* pIntParameters = (apGLenumParameter*)pParamValue;

                // Get the value:
                valueAsEnum = pIntParameters->value();
                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::getTextureFloatParameterValue
// Description: return a parameter value as float
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/10/2008
// ---------------------------------------------------------------------------
bool apGLTextureParams::getTextureFloatParameterValue(GLenum parameterName, GLfloat& valueAsFloat)const
{
    bool retVal = false;

    // Get parameter index:
    int paramIndex = getTextureParameterIndex(parameterName);

    if (0 <= paramIndex)
    {
        // Get the parameters value:
        const apParameter* pParamValue = getTextureParameterValue(paramIndex);

        if (pParamValue != NULL)
        {
            // Check if the parameter type is GLfloat:
            if (pParamValue->type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
            {
                // Down cast the parameter to float parameter:
                apGLfloatParameter* pFloatParameters = (apGLfloatParameter*)pParamValue;

                // Get the value:
                valueAsFloat = pFloatParameters->value();
                retVal = true;
            }
        }
    }

    return retVal;
}




// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::getTextureParameterName
// Description: Inputs a texture parameter index and outputs its OpenGL name.
// Arguments:   parameterIndex - The queried parameter index.
// Return Val:  GLenum - The queried parameter name or GL_NONE if it does not exist.
// Author:  AMD Developer Tools Team
// Date:        4/11/2008
// ---------------------------------------------------------------------------
GLenum apGLTextureParams::getTextureParameterName(int parameterIndex) const
{
    GLenum retVal = GL_NONE;

    // Index range check:
    GT_IF_WITH_ASSERT((0 <= parameterIndex) && (parameterIndex < (int)_textureParameters.size()))
    {
        // Get the queried parameter:
        const apGLTextureParameter* pParameter = _textureParameters[parameterIndex];
        GT_IF_WITH_ASSERT(pParameter != NULL)
        {
            retVal = pParameter->_parameterName;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        19/12/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLTextureParams::type() const
{
    return OS_TOBJ_ID_GL_TEXTURE;
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::writeSelfIntoChannel
// Description: Writes this class into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        19/12/2004
// ---------------------------------------------------------------------------
bool apGLTextureParams::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the amount of texture parameters:
    gtInt64 paramsAmount = (gtInt64)_textureParameters.size();
    ipcChannel << paramsAmount;

    // Write the texture parameters:
    for (int i = 0; i < paramsAmount; i++)
    {
        // Current parameter name:
        const apGLTextureParameter* pCurrParameter = _textureParameters[i];
        bool isParameterPresent = (pCurrParameter != NULL);
        ipcChannel << isParameterPresent;

        GT_IF_WITH_ASSERT(isParameterPresent)
        {
            ipcChannel << (gtInt32)pCurrParameter->_parameterName;

            // Current parameter value:
            const apParameter* pParamValue = pCurrParameter->_aptrParameterValue.pointedObject();
            ipcChannel << *pParamValue;

            // Current parameter update status:
            ipcChannel << pCurrParameter->_isUpdatedFromHardware;
        }
    }


    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::readSelfFromChannel
// Description: Read self from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/12/2004
// ---------------------------------------------------------------------------
bool apGLTextureParams::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Clear allocated values:
    deleteAllParameters();

    // Read the amount of texture parameters:
    gtInt64 paramsAmount = 0;
    ipcChannel >> paramsAmount;

    // Read the texture parameters:
    for (int i = 0; i < paramsAmount; i++)
    {
        bool isParameterPresent = false;
        ipcChannel >> isParameterPresent;

        if (isParameterPresent)
        {
            // Current parameter name:
            GLenum paramName = 0;
            ipcChannel >> paramName;

            // Current parameter value:
            gtAutoPtr<osTransferableObject> aptrReadTransferableObj;
            ipcChannel >> aptrReadTransferableObj;

            // Current parameter update status:
            bool paramUpdated = false;
            ipcChannel >> paramUpdated;

            // Sanity check:
            if (!(aptrReadTransferableObj->isParameterObject()))
            {
                GT_ASSERT(false);
                retVal = false;
            }
            else
            {
                // Store the read parameters:
                apGLTextureParameter* pCurrParam = new apGLTextureParameter;
                GT_IF_WITH_ASSERT(pCurrParam != NULL)
                {
                    pCurrParam->_parameterName = paramName;
                    pCurrParam->_aptrParameterValue = (apParameter*)(aptrReadTransferableObj.releasePointedObjectOwnership());
                    pCurrParam->_isUpdatedFromHardware = paramUpdated;
                    _textureParameters.push_back(pCurrParam);
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::deleteTextureLevelParameters
// Description:
//   Deletes the texture parameters.
//   I.E: Empties _textureParametersNames and _textureParametersValues
// Author:  AMD Developer Tools Team
// Date:        30/12/2004
// ---------------------------------------------------------------------------
void apGLTextureParams::deleteAllParameters()
{
    _textureParameters.deleteElementsAndClear();
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::getTextureParameterIndex
// Description: Inputs a parameter name and returns its index in this class.
//              (or -1 if it does not exist)
// Author:  AMD Developer Tools Team
// Date:        22/1/2005
// ---------------------------------------------------------------------------
int apGLTextureParams::getTextureParameterIndex(GLenum parameterName) const
{
    int retVal = -1;

    // Iterate the existing texture parameters:
    int paramsAmount = (int)_textureParameters.size();

    for (int i = 0; i < paramsAmount; i++)
    {
        apGLTextureParameter* pTexParam = _textureParameters[i];
        GT_IF_WITH_ASSERT(pTexParam != NULL)
        {
            // If we found the queried parameter:
            if (pTexParam->_parameterName == parameterName)
            {
                retVal = i;
                break;
            }
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::getParameterType
// Description: Inputs an OpenGL parameter name and returns its type and elements amount.
// Arguments:   paramName - The input parameter name.
//              parameterType - The output parameter type.
//              amountOfParameterItems - the amount of parameters for this parameters name
// Author:  AMD Developer Tools Team
// Date:        23/1/2005
// ---------------------------------------------------------------------------
void apGLTextureParams::getParameterType(GLenum paramName, osTransferableObjectType& parameterType, int& amountOfParameterItems)
{
    // Amount of items is 1 unless set otherwise:
    amountOfParameterItems = 1;
    parameterType = OS_TOBJ_ID_NOT_AVAILABLE_PARAMETER;

    switch (paramName)
    {
        // From here on the parameter names are mip level parameters:
        // (for later more convenient seperation between texture level and mipmap level parameters)
        case GL_TEXTURE_WIDTH:
        case GL_TEXTURE_HEIGHT:
        case GL_TEXTURE_DEPTH:
        case GL_TEXTURE_BORDER:
        case GL_TEXTURE_RED_SIZE:
        case GL_TEXTURE_GREEN_SIZE:
        case GL_TEXTURE_BLUE_SIZE:
        case GL_TEXTURE_ALPHA_SIZE:
        case GL_TEXTURE_LUMINANCE_SIZE:
        case GL_TEXTURE_INTENSITY_SIZE:
        case GL_TEXTURE_DEPTH_SIZE:
        case GL_TEXTURE_COMPRESSED_IMAGE_SIZE:
        case GL_TEXTURE_SHARED_SIZE:
        {
            parameterType = OS_TOBJ_ID_GL_FLOAT_PARAMETER;
            break;
        }

        case GL_TEXTURE_COMPRESSED:
        {
            parameterType = OS_TOBJ_ID_GL_BOOL_PARAMETER;
            break;
        }

        case GL_TEXTURE_INTERNAL_FORMAT:
        {
            parameterType = OS_TOBJ_ID_GL_ENUM_PARAMETER;
            break;
        }

        // GLenum parameters:
        case GL_TEXTURE_MIN_FILTER:
        case GL_TEXTURE_MAG_FILTER:
        case GL_TEXTURE_WRAP_S:
        case GL_TEXTURE_WRAP_T:
        case GL_TEXTURE_WRAP_R:
        case GL_TEXTURE_COMPARE_MODE:
        case GL_TEXTURE_COMPARE_FUNC:

        //      case GL_TEXTURE_COMPARE_MODE_ARB:  // same as GL_TEXTURE_COMPARE_MODE
        //      case GL_TEXTURE_COMPARE_FUNC_ARB:  // same as GL_TEXTURE_COMPARE_FUNC
        case GL_TEXTURE_COMPARE_OPERATOR_SGIX:
        case GL_DEPTH_TEXTURE_MODE:
        {
            parameterType = OS_TOBJ_ID_GL_ENUM_PARAMETER;
            break;
        };

        // GLint parameters:
        case GL_TEXTURE_BASE_LEVEL:
        case GL_TEXTURE_MAX_LEVEL:
        {
            parameterType = OS_TOBJ_ID_GL_INT_PARAMETER;
            break;
        };

        // GLfloat parameters:
        case GL_TEXTURE_COMPARE_FAIL_VALUE_ARB:
        case GL_TEXTURE_MIN_LOD:
        case GL_TEXTURE_MAX_LOD:
        case GL_TEXTURE_PRIORITY:
        {
            parameterType = OS_TOBJ_ID_GL_FLOAT_PARAMETER;
            break;
        }

        // GLboolean parameters:
        case GL_GENERATE_MIPMAP:
        case GL_TEXTURE_COMPARE_SGIX:
        case GL_TEXTURE_RESIDENT:
        {
            parameterType = OS_TOBJ_ID_GL_BOOL_PARAMETER;
            break;
        }

        // Color vector parameters:
        case GL_TEXTURE_BORDER_COLOR:
        {
            parameterType = OS_TOBJ_ID_GL_FLOAT_PARAMETER;

            // Try getting the internal format parameter, if it is available:
            GLenum internalFormatName = GL_NONE;

            if (getTextureEnumParameterValue(GL_TEXTURE_INTERNAL_FORMAT, internalFormatName))
            {
                int channels = 1;

                if (apGetChannelCountByInternalFormat(internalFormatName, channels))
                {
                    if (channels > 0)
                    {
                        amountOfParameterItems = channels;
                    }
                }
            }

            break;
        }

        default:
        {
            int paramIndex = getTextureParameterIndex(paramName);

            if (paramIndex < 0)
            {
                // Default value (will be assigned for unknown parameter types):
                parameterType = OS_TOBJ_ID_GL_FLOAT_PARAMETER;
            }
            else
            {
                // Get the parameter value:
                // Do not call getTextureParameterValue since it returns non available parameter for not updated params
                apGLTextureParameter* pTexParam = _textureParameters[paramIndex];
                GT_IF_WITH_ASSERT(pTexParam != NULL)
                {
                    const apParameter* pParam = pTexParam->_aptrParameterValue.pointedObject();
                    GT_IF_WITH_ASSERT(pParam != NULL)
                    {
                        if (pParam->type() != OS_TOBJ_ID_NOT_AVAILABLE_PARAMETER)
                        {
                            parameterType = pParam->type();
                        }
                        else
                        {
                            parameterType = OS_TOBJ_ID_GL_FLOAT_PARAMETER;
                        }
                    }
                }
            }

        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::setTextureParameterUpdateStatus
// Description: Mark the parameter as updated
// Arguments: int parameterIndex
//            bool isUpdated
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        2/11/2008
// ---------------------------------------------------------------------------
void apGLTextureParams::setTextureParameterUpdateStatus(int parameterIndex, bool isUpdated)
{
    // Index range test:
    GT_IF_WITH_ASSERT((0 <= parameterIndex) && (parameterIndex < (int)_textureParameters.size()))
    {
        // Mark the parameter update status:
        apGLTextureParameter* pTexParameter = _textureParameters[parameterIndex];
        GT_IF_WITH_ASSERT(pTexParameter != NULL)
        {
            pTexParameter->_isUpdatedFromHardware = isUpdated;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureParams::clearAllParameters
// Description:
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        5/11/2009
// ---------------------------------------------------------------------------
void apGLTextureParams::clearAllParameters()
{
    _textureParameters.deleteElementsAndClear();
}

