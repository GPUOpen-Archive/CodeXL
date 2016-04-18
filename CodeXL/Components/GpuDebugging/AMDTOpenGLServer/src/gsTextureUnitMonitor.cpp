//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsTextureUnitMonitor.cpp
///
//==================================================================================

//------------------------------ gsTextureUnitMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTAPIClasses/Include/apDefaultTextureNames.h>

// Local:
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsWrappersCommon.h>
#include <src/gsTextureUnitMonitor.h>
#include <src/gsExtensionsManager.h>


// ---------------------------------------------------------------------------
// Name:        gsTextureUnitMonitor::gsTextureUnitMonitor
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        18/4/2005
// ---------------------------------------------------------------------------
gsTextureUnitMonitor::gsTextureUnitMonitor(GLenum textureUnitName)
    : _textureUnitName(textureUnitName),
      _enabledTexturingMode(AP_UNKNOWN_TEXTURE_TYPE),
      _bind1DTextureName(0), _bind2DTextureName(0),
      _bind3DTextureName(0), _bind1DArrayTextureName(0), _bind2DArrayTextureName(0),
      _bindMultiSampleTextureName(0), _bindMultiSampleArrayTextureName(0),
      _bindCubeMapTextureName(0), _bindCubeMapArrayTextureName(0), _bindTextureRectangleName(0),
      _bindTextureBufferName(0)
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    , _glActiveTexture(NULL)
#endif
{
    // Sanity check:
    bool isTextureUnitNameOK = ((GL_TEXTURE0 <= textureUnitName) && (textureUnitName <= GL_TEXTURE31));
    GT_ASSERT(isTextureUnitNameOK);
}


// ---------------------------------------------------------------------------
// Name:        gsTextureUnitMonitor::~gsTextureUnitMonitor
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        18/4/2005
// ---------------------------------------------------------------------------
gsTextureUnitMonitor::~gsTextureUnitMonitor()
{
}


// ---------------------------------------------------------------------------
// Name:        gsTextureUnitMonitor::onFirstTimeContextMadeCurrent
// Description: Is called on the first time that my context is made the
//              current context.
// Author:      Yaki Tebeka
// Date:        18/4/2005
// ---------------------------------------------------------------------------
void gsTextureUnitMonitor::onFirstTimeContextMadeCurrent()
{
    // Get extension functions pointers:
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    _glActiveTexture = (PFNGLACTIVETEXTUREPROC)(gsGetSystemsOGLModuleProcAddress("glActiveTexture"));
#endif
}


// ---------------------------------------------------------------------------
// Name:        gsTextureUnitMonitor::onTextureTargetBind
// Description: Is called when a texture object is bind to a bind target.
// Arguments:   target - The bind target.
//              textureName - The name of the bound texture.
// Author:      Yaki Tebeka
// Date:        18/04/2005
// ---------------------------------------------------------------------------
void gsTextureUnitMonitor::onTextureTargetBind(GLenum target, GLuint textureName)
{
    switch (target)
    {
        case GL_TEXTURE_1D:
            _bind1DTextureName = textureName;
            break;

        case GL_TEXTURE_2D:
            _bind2DTextureName = textureName;
            break;

        case GL_TEXTURE_3D:
            _bind3DTextureName = textureName;
            break;

        case GL_TEXTURE_1D_ARRAY:
            _bind1DArrayTextureName = textureName;
            break;

        case GL_TEXTURE_2D_ARRAY:
            _bind2DArrayTextureName = textureName;
            break;

        case GL_TEXTURE_2D_MULTISAMPLE:
            _bindMultiSampleTextureName = textureName;
            break;

        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
            _bindMultiSampleArrayTextureName = textureName;
            break;

        case GL_TEXTURE_CUBE_MAP:
            _bindCubeMapTextureName = textureName;
            break;

        case GL_TEXTURE_CUBE_MAP_ARRAY:
            _bindCubeMapArrayTextureName = textureName;
            break;

        case GL_TEXTURE_RECTANGLE_ARB:
            _bindTextureRectangleName = textureName;
            break;

        case GL_TEXTURE_BUFFER:
            _bindTextureBufferName = textureName;
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        gsTextureUnitMonitor::unbindAllTextures
// Description: Clears all texture target bindings for this unit.
// Author:      Uri Shomroni
// Date:        21/06/2015
// ---------------------------------------------------------------------------
void gsTextureUnitMonitor::unbindAllTextures()
{
    _bind1DTextureName = 0;
    _bind2DTextureName = 0;
    _bind3DTextureName = 0;
    _bind1DArrayTextureName = 0;
    _bind2DArrayTextureName = 0;
    _bindMultiSampleTextureName = 0;
    _bindMultiSampleArrayTextureName = 0;
    _bindCubeMapTextureName = 0;
    _bindCubeMapArrayTextureName = 0;
    _bindTextureRectangleName = 0;
    _bindTextureBufferName = 0;
}


// ---------------------------------------------------------------------------
// Name:        gsTextureUnitMonitor::getEnabledTexturingMode
// Description: Returns the currently enabled texturing mode.
// Arguments:   contextId - The queried context id.
//              isTexturingEnabled - Will get true iff texturing is enabled.
//              enabledTexturingMode - Will get the enabled texturing mode.
// Author:      Yaki Tebeka
// Date:        18/04/2005
// ---------------------------------------------------------------------------
void gsTextureUnitMonitor::getEnabledTexturingMode(bool& isTexturingEnabled, apTextureType& enabledTexturingMode) const
{
    isTexturingEnabled = false;
    enabledTexturingMode = _enabledTexturingMode;

    if (_enabledTexturingMode != AP_UNKNOWN_TEXTURE_TYPE)
    {
        isTexturingEnabled = true;
    }
}


// ---------------------------------------------------------------------------
// Name:        gsTextureUnitMonitor::bindTextureName
// Description: Inputs a bind target and outputs the OpenGL name of the texture
//              that is bind to this target.
// Arguments:   target - The queried bind target.
//              textureName - The output texture name.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        18/04/2005
// Implementation notes:
//   The currently supported bound targets are: GL_TEXTURE_1D, GL_TEXTURE_2D and GL_TEXTURE_3D
// ---------------------------------------------------------------------------
GLuint gsTextureUnitMonitor::bindTextureName(apTextureType bindTarget) const
{
    GLuint retVal = 0;

    switch (bindTarget)
    {
        case AP_1D_TEXTURE:
            retVal = _bind1DTextureName;
            break;

        case AP_2D_TEXTURE:
            retVal = _bind2DTextureName;
            break;

        case AP_3D_TEXTURE:
            retVal = _bind3DTextureName;
            break;

        case AP_1D_ARRAY_TEXTURE:
            retVal = _bind1DArrayTextureName;
            break;

        case AP_2D_ARRAY_TEXTURE:
            retVal = _bind2DArrayTextureName;
            break;

        case AP_2D_TEXTURE_MULTISAMPLE:
            retVal = _bindMultiSampleTextureName;
            break;

        case AP_2D_TEXTURE_MULTISAMPLE_ARRAY:
            retVal = _bindMultiSampleArrayTextureName;
            break;

        case AP_CUBE_MAP_TEXTURE:
            retVal = _bindCubeMapTextureName;
            break;

        case AP_CUBE_MAP_ARRAY_TEXTURE:
            retVal = _bindCubeMapArrayTextureName;
            break;

        case AP_TEXTURE_RECTANGLE:
            retVal = _bindTextureRectangleName;
            break;

        case AP_BUFFER_TEXTURE:
            retVal = _bindTextureBufferName;
            break;

        default:
            // Should not reach here:
            GT_ASSERT(false);
            break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsTextureUnitMonitor::applyForcedStubTextureObjects
// Description: Applies the "Forces stub textures" mode.
// Arguments:   stub1DTexName, stub2DTexName, stub3DTexName, stubCubeMapTexName,
//              stubRectangleTexName - The stub texture names (or 0 if they does not exist).
// Author:      Yaki Tebeka
// Date:        18/4/2005
// ---------------------------------------------------------------------------
void gsTextureUnitMonitor::applyForcedStubTextureObjects(GLuint stub1DTexName, GLuint stub2DTexName,
                                                         GLuint stub3DTexName, GLuint stubCubeMapTexName,
                                                         GLuint stubRectangleTexName)
{
    // Set the active texture unit to be the texture unit that this class monitors:
    GLuint curActiveTextureUnit = 0;
    setActiveTexureUnit(_textureUnitName, curActiveTextureUnit);

    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindTexture);

    // Replace currently bind textures (if exists) with the stub textures:
    if (_bind1DTextureName != 0)
    {
        gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_1D, stub1DTexName);
    }

    if (_bind2DTextureName != 0)
    {
        gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_2D, stub2DTexName);
    }

    if ((_bind3DTextureName != 0) && (stub3DTexName != 0))
    {
        gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_3D, stub3DTexName);
    }

    if ((_bindCubeMapTextureName != 0) && (stubCubeMapTexName != 0))
    {
        gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_CUBE_MAP, stubCubeMapTexName);
    }

    if ((_bindTextureRectangleName != 0) && (stubRectangleTexName != 0))
    {
        gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_RECTANGLE_ARB, stubRectangleTexName);
    }

    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindTexture);

    // Restore the active texture unit:
    GLuint ignored = 0;
    setActiveTexureUnit(curActiveTextureUnit, ignored);

    // Test for OpenGL error:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
    GLenum error = gs_stat_realFunctionPointers.glGetError();
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

    if (error != GL_NO_ERROR)
    {
        GT_ASSERT_EX(false, L"Error");
    }

}


// ---------------------------------------------------------------------------
// Name:        gsTextureUnitMonitor::cancelForcedStubTextureObjects
// Description: Cancels the "Forces stub textures" mode.
// Author:      Yaki Tebeka
// Date:        18/4/2005
// ---------------------------------------------------------------------------
void gsTextureUnitMonitor::cancelForcedStubTextureObjects()
{
    // Set the active texture unit to be the texture unit that this class monitors:
    GLuint curActiveTextureUnit = 0;
    setActiveTexureUnit(_textureUnitName, curActiveTextureUnit);

    // Calculate the names of the "original" program textures:
    // ------------------------------------------------------
    GLuint resumedTex1D = _bind1DTextureName;
    GLuint resumedTex2D = _bind2DTextureName;
    GLuint resumedTex3D = _bind3DTextureName;
    GLuint resumedTexCubeMap = _bindCubeMapTextureName;
    GLuint resumedTexRectangle = _bindTextureRectangleName;
    GLuint resumedTexBuffer = _bindTextureBufferName;

    if (_bind1DTextureName != 0)
    {
        // If the "original" bind texture is a default textures:
        if (apIsDefaultTextureName(resumedTex1D))
        {
            resumedTex1D = 0;
        }
    }

    if (_bind2DTextureName != 0)
    {
        // If the "original" bind texture is a default textures:
        if (apIsDefaultTextureName(resumedTex2D))
        {
            resumedTex2D = 0;
        }
    }

    if (_bind3DTextureName != 0)
    {
        // If the "original" bind texture is a default textures:
        if (apIsDefaultTextureName(resumedTex3D))
        {
            resumedTex3D = 0;
        }
    }

    if (_bindCubeMapTextureName != 0)
    {
        // If the "original" bind texture is a default textures:
        if (apIsDefaultTextureName(resumedTexCubeMap))
        {
            resumedTexCubeMap = 0;
        }
    }

    if (_bindTextureRectangleName != 0)
    {
        // If the "original" bind texture is a default textures:
        if (apIsDefaultTextureName(resumedTexRectangle))
        {
            resumedTexRectangle = 0;
        }
    }

    if (_bindTextureBufferName != 0)
    {
        // If the "original" bind texture is a default textures:
        if (apIsDefaultTextureName(resumedTexBuffer))
        {
            resumedTexBuffer = 0;
        }
    }

    // Resume the "original" bind textures:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindTexture);
#if ((defined OS_OGL_ES_IMPLEMENTATION_DLL_BUILD) || (defined _GR_IPHONE_BUILD))
    gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_2D, resumedTex2D);
#else
    gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_1D, resumedTex1D);
    gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_2D, resumedTex2D);
    gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_3D, resumedTex3D);
    gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_CUBE_MAP, resumedTexCubeMap);
    gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_RECTANGLE_ARB, resumedTexRectangle);
    gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_BUFFER, resumedTexBuffer);
#endif
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindTexture);

    // Test for OpenGL error:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
    GLenum error = gs_stat_realFunctionPointers.glGetError();
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

    if (error != GL_NO_ERROR)
    {
        GT_ASSERT_EX(false, L"Error");
    }

    // Restore the active texture unit:
    GLuint ignored = 0;
    setActiveTexureUnit(curActiveTextureUnit, ignored);
}


// ---------------------------------------------------------------------------
// Name:        gsTextureUnitMonitor::clearContextDataSnapshot
// Description: Clears the enabled texturing mode (_enabledTexturingMode).
// Author:      Yaki Tebeka
// Date:        15/6/2005
// ---------------------------------------------------------------------------
void gsTextureUnitMonitor::clearContextDataSnapshot()
{
    _enabledTexturingMode = AP_UNKNOWN_TEXTURE_TYPE;
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::updateContextDataSnapshot
// Description: Updates the enabled texturing mode (_enabledTexturingMode).
// Author:      Yaki Tebeka
//
// Implementation notes:
//   If more than one texture bind target is enabled, OpenGL acts according to
//   priorities. From lowest priority to highest priority: GL_TEXTURE_1D,
//   GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP.
//   For more details see:
//   a. The OpenGL red book - Chapter 9: Texture mapping -> Steps in texture mapping ->
//      Enable texture mapping, pages 365, 366
//   b. GL_ARB_texture_rectangle extension specification - Issue 10: How are rectangular
//      textures enabled?
//
// Date:        17/1/2005
// ---------------------------------------------------------------------------
void gsTextureUnitMonitor::updateContextDataSnapshot(int callingContextId, const int callingContextOGLVersion[2])
{
    _enabledTexturingMode = AP_UNKNOWN_TEXTURE_TYPE;

    // Set the active texture unit to be the texture unit that this class monitors:
    GLuint curActiveTextureUnit = 0;
    setActiveTexureUnit(_textureUnitName, curActiveTextureUnit);

    //////////////////////////////////////////////////////////////////////////
    // Check which of the optional (extension) texturing modes is supported:
#if defined (_GR_OPENGLES_IPHONE) || defined (_GR_OPENGLES_COMMON) || defined (_GR_OPENGLES_COMMON_LITE)
    // OpenGL ES 1.1 supports none of these, 2.0 supports only cube maps:
    bool is3DTexturingSupported = false;
    bool isCubeMapTexturingSupported = (callingContextOGLVersion[0] > 1);
    bool isTextureRectangleSupported = false;
    bool isArrayTexturingSupported = false;
    bool isMultiSampleTexutringSupported = false;
    bool isTextureBufferSupported = false;
#else // !(defined (_GR_OPENGLES_IPHONE) || defined (_GR_OPENGLES_COMMON) || defined (_GR_OPENGLES_COMMON_LITE))
    // Check for the extensions that enable each target, or the OpenGL version that started base support of them:
    // Uri, 31/12/09 - Note that as note below, OpenGL 3.1 and higher does not support the way we check in older
    // versions, so we do not include it in our calculations (commented out below).
    gsExtensionsManager& theExtensionsManager = gsExtensionsManager::instance();

    bool is3DTexturingSupported = theExtensionsManager.isExtensionSupported(callingContextId, AP_GL_EXT_texture3D) ||
                                  ((callingContextOGLVersion[0] > 1) || ((callingContextOGLVersion[0] == 1) && (callingContextOGLVersion[1] >= 2)));

    bool isCubeMapTexturingSupported = ((callingContextOGLVersion[0] > 1) || ((callingContextOGLVersion[0] == 1) && (callingContextOGLVersion[1] >= 3))) ||
                                       theExtensionsManager.isExtensionSupported(callingContextId, AP_GL_ARB_texture_cube_map);

    bool isTextureRectangleSupported = /*((callingContextOGLVersion[0] > 3) || ((callingContextOGLVersion[0] == 3) && (callingContextOGLVersion[1] >= 1))) ||*/
        theExtensionsManager.isExtensionSupported(callingContextId, AP_GL_NV_texture_rectangle) ||
        theExtensionsManager.isExtensionSupported(callingContextId, AP_GL_ARB_texture_rectangle);

    bool isArrayTexturingSupported = /*(callingContextOGLVersion[0] >= 3) ||*/
        ((callingContextOGLVersion[0] == 3) && (callingContextOGLVersion[1] == 0)) ||
        theExtensionsManager.isExtensionSupported(callingContextId, AP_GL_EXT_texture_array);

    bool isMultiSampleTexutringSupported = /*((callingContextOGLVersion[0] > 3) || ((callingContextOGLVersion[0] == 3) && (callingContextOGLVersion[1] >= 2))) ||*/
        theExtensionsManager.isExtensionSupported(callingContextId, AP_GL_ARB_texture_multisample);

    bool isTextureBufferSupported = /*((callingContextOGLVersion[0] > 3) || ((callingContextOGLVersion[0] == 3) && (callingContextOGLVersion[1] >= 1))) ||*/
        theExtensionsManager.isExtensionSupported(callingContextId, AP_GL_ARB_texture_buffer_object) ||
        theExtensionsManager.isExtensionSupported(callingContextId, AP_GL_EXT_texture_buffer_object);
#endif // defined (_GR_OPENGLES_IPHONE) || defined (_GR_OPENGLES_COMMON) || defined (_GR_OPENGLES_COMMON_LITE)

    //////////////////////////////////////////////////////////////////////////
    // Get the enables texturing modes:
    GLboolean is1DTexturingEnabled = GL_FALSE;
    GLboolean is2DTexturingEnabled = GL_FALSE;
    GLboolean is3DTexturingEnabled = GL_FALSE;
    GLboolean isCubeMapTexturingEnabled = GL_FALSE;
    GLboolean isTextureRectangleEnabled = GL_FALSE;
    GLboolean is1DArrayTextureEnabled = GL_FALSE;
    GLboolean is2DArrayTextureEnabled = GL_FALSE;
    GLboolean isMultiSampleTextureEnabled = GL_FALSE;
    GLboolean isMultiSampleArrayTextureEnabled = GL_FALSE;
    GLboolean isTextureBufferEnabled = GL_FALSE;

    if ((callingContextOGLVersion[0] > 3) || ((callingContextOGLVersion[0] == 3) && (callingContextOGLVersion[1] >= 1)))
    {
        // In OpenGL 3.1 and higher, glIsEnabled(GL_TEXTURE_*) is not allowed as it is part of the fixed pipeline fragment processing.
        // Instead, a target is "enabled" if a texture is bound there:
        if (_bind1DTextureName > 0)
        {
            is1DTexturingEnabled = GL_TRUE;
        }

        if (_bind2DTextureName > 0)
        {
            is2DTexturingEnabled = GL_TRUE;
        }

        if (_bind3DTextureName > 0)
        {
            is3DTexturingEnabled = GL_TRUE;
        }

        if (_bindCubeMapTextureName > 0)
        {
            isCubeMapTexturingEnabled = GL_TRUE;
        }

        if (_bindTextureRectangleName > 0)
        {
            isTextureRectangleEnabled = GL_TRUE;
        }

        if (_bind1DArrayTextureName > 0)
        {
            is1DArrayTextureEnabled = GL_TRUE;
        }

        if (_bind2DArrayTextureName > 0)
        {
            is2DArrayTextureEnabled = GL_TRUE;
        }

        if (_bindMultiSampleTextureName > 0)
        {
            isMultiSampleTextureEnabled = GL_TRUE;
        }

        if (_bindMultiSampleArrayTextureName > 0)
        {
            isMultiSampleArrayTextureEnabled = GL_TRUE;
        }

        if (_bindTextureBufferName > 0)
        {
            isTextureBufferEnabled = GL_TRUE;
        }
    }
    else // ((callingContextOGLVersion[0] < 3) || ((callingContextOGLVersion[0] == 3) && (callingContextOGLVersion[1] < 1)))
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glIsEnabled);
        is1DTexturingEnabled = gs_stat_realFunctionPointers.glIsEnabled(GL_TEXTURE_1D);

        is2DTexturingEnabled = gs_stat_realFunctionPointers.glIsEnabled(GL_TEXTURE_2D);

        if (is3DTexturingSupported)
        {
            is3DTexturingEnabled = gs_stat_realFunctionPointers.glIsEnabled(GL_TEXTURE_3D);
        }

        if (isCubeMapTexturingSupported)
        {
            isCubeMapTexturingEnabled = gs_stat_realFunctionPointers.glIsEnabled(GL_TEXTURE_CUBE_MAP);
        }

        if (isTextureRectangleSupported)
        {
            isTextureRectangleEnabled = gs_stat_realFunctionPointers.glIsEnabled(GL_TEXTURE_RECTANGLE);
        }

        if (isArrayTexturingSupported)
        {
            is1DArrayTextureEnabled = gs_stat_realFunctionPointers.glIsEnabled(GL_TEXTURE_1D_ARRAY);
            is2DArrayTextureEnabled = gs_stat_realFunctionPointers.glIsEnabled(GL_TEXTURE_2D_ARRAY);
        }

        if (isMultiSampleTexutringSupported)
        {
            isMultiSampleTextureEnabled = gs_stat_realFunctionPointers.glIsEnabled(GL_TEXTURE_2D_MULTISAMPLE);

            if (isArrayTexturingSupported)
            {
                isMultiSampleArrayTextureEnabled = gs_stat_realFunctionPointers.glIsEnabled(GL_TEXTURE_2D_MULTISAMPLE_ARRAY);
            }
        }

        if (isTextureBufferSupported)
        {
            isTextureBufferEnabled = gs_stat_realFunctionPointers.glIsEnabled(GL_TEXTURE_BUFFER);
        }

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glIsEnabled);
    }

    //////////////////////////////////////////////////////////////////////////
    // Calculate the active texturing mode:
    // (See "Implementation notes" in this function documentation):

    //.Is this the right place to add the array textures?
    if (is1DArrayTextureEnabled == GL_TRUE)
    {
        _enabledTexturingMode = AP_1D_ARRAY_TEXTURE;
    }
    else if (is2DArrayTextureEnabled == GL_TRUE)
    {
        _enabledTexturingMode = AP_2D_ARRAY_TEXTURE;
    }
    else if (isCubeMapTexturingEnabled == GL_TRUE)
    {
        _enabledTexturingMode = AP_CUBE_MAP_TEXTURE;
    }
    else if (is3DTexturingEnabled == GL_TRUE)
    {
        _enabledTexturingMode = AP_3D_TEXTURE;
    }
    else if (isTextureRectangleEnabled)
    {
        _enabledTexturingMode = AP_TEXTURE_RECTANGLE;
    }
    else if (is2DTexturingEnabled == GL_TRUE)
    {
        _enabledTexturingMode = AP_2D_TEXTURE;
    }
    else if (is1DTexturingEnabled == GL_TRUE)
    {
        _enabledTexturingMode = AP_1D_TEXTURE;
    }
    else if (isTextureBufferEnabled == GL_TRUE)
    {
        _enabledTexturingMode = AP_BUFFER_TEXTURE;
    }
    else if (isMultiSampleTextureEnabled == GL_TRUE)
    {
        _enabledTexturingMode = AP_2D_TEXTURE_MULTISAMPLE;
    }
    else if (isMultiSampleArrayTextureEnabled == GL_TRUE)
    {
        _enabledTexturingMode = AP_2D_TEXTURE_MULTISAMPLE_ARRAY;
    }
    else
    {
        _enabledTexturingMode = AP_UNKNOWN_TEXTURE_TYPE;
    }

    // Restore the active texture unit:
    GLuint ignored = 0;
    setActiveTexureUnit(curActiveTextureUnit, ignored);
}


// ---------------------------------------------------------------------------
// Name:        gsTextureUnitMonitor::setActiveTexureUnit
// Description: Sets my render context active texture unit.
// Arguments:   newTexUnit - The texture unit that will become the active texture unit.
//              currentTexUnit - Will get current active texture unit.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        18/4/2005
// ---------------------------------------------------------------------------
bool gsTextureUnitMonitor::setActiveTexureUnit(GLuint newTexUnit, GLuint& currentTexUnit)
{
    bool retVal = false;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
#define _glActiveTexture gs_stat_realFunctionPointers.glActiveTexture
#endif

    // If multi textures are supported:
    if (_glActiveTexture != NULL)
    {
        // Get the currently active texture unit:
        GLint currentTextureUnit = 0;
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
        gs_stat_realFunctionPointers.glGetIntegerv(GL_ACTIVE_TEXTURE, &currentTextureUnit);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
        currentTexUnit = currentTextureUnit;

        // Set the active texture unit to the input texture unit:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glActiveTexture);
        _glActiveTexture(newTexUnit);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glActiveTexture);

        retVal = true;
    }

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
#undef _glActiveTexture
#endif

    return retVal;
}
