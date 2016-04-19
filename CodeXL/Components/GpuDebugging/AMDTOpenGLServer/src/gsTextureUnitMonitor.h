//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsTextureUnitMonitor.h
///
//==================================================================================

//------------------------------ gsTextureUnitMonitor.h ------------------------------

#ifndef __GSTEXTUREUNITMONITOR
#define __GSTEXTUREUNITMONITOR

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTAPIClasses/Include/apTextureType.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsTextureUnitMonitor
// General Description:
//   Monitors Texture Unit data.
//   A texture unit holds the data needed by a signle multi texture texturing operation.
//   For more details see The OpenGL Programming Guide -> Texture mapping -> Multitextures.
//
// Author:               Yaki Tebeka
// Creation Date:        18/04/2004
// ----------------------------------------------------------------------------------
class gsTextureUnitMonitor
{
public:
    gsTextureUnitMonitor(GLenum textureUnitName);
    ~gsTextureUnitMonitor();

    // On event functions:
    void onFirstTimeContextMadeCurrent();
    void onTextureTargetBind(GLenum bindTarget, GLuint textureName);
    void unbindAllTextures();

    // Query data functions:
    GLenum textureUnitName() const { return _textureUnitName; };
    void getEnabledTexturingMode(bool& isTexturingEnabled, apTextureType& enabledTexturingMode) const;
    GLuint bindTextureName(apTextureType bindTarget) const;

    // Forced modes:
    void applyForcedStubTextureObjects(GLuint stub1DTexName, GLuint stub2DTexName,
                                       GLuint stub3DTexName, GLuint stubCubeMapTexName,
                                       GLuint stubRectangleTexName);
    void cancelForcedStubTextureObjects();

    // Misc:
    void clearContextDataSnapshot();
    void updateContextDataSnapshot(int callingContextId, const int callingContextOGLVersion[2]);

private:
    // Do not allow the use of my default constructor:
    gsTextureUnitMonitor();

    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead
    gsTextureUnitMonitor& operator=(const gsTextureUnitMonitor& otherMonitor);
    gsTextureUnitMonitor(const gsTextureUnitMonitor& otherMonitor);

    bool setActiveTexureUnit(GLuint newTexUnit, GLuint& currentTexUnit);

private:
    // Holds the name of the texture unit that this class monitors:
    GLenum  _textureUnitName;

    // Holds the enabled texturing mode:
    apTextureType _enabledTexturingMode;

    // Holds the bind texture names:
    GLuint _bind1DTextureName;
    GLuint _bind2DTextureName;
    GLuint _bind3DTextureName;
    GLuint _bind1DArrayTextureName;
    GLuint _bind2DArrayTextureName;
    GLuint _bindMultiSampleTextureName;
    GLuint _bindMultiSampleArrayTextureName;
    GLuint _bindCubeMapTextureName;
    GLuint _bindCubeMapArrayTextureName;
    GLuint _bindTextureRectangleName;
    GLuint _bindTextureBufferName;

    // Extension function pointers:
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    PFNGLACTIVETEXTUREPROC _glActiveTexture;
#endif
};


#endif  // __GSTEXTUREUNITMONITOR
