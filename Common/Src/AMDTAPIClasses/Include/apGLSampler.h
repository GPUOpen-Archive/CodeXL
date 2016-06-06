//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLSampler.h
///
//==================================================================================

//------------------------------ apGLSampler.h ------------------------------

#ifndef __APGLSAMPLER
#define __APGLSAMPLER

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apAllocatedObject.h>


// ----------------------------------------------------------------------------------
// Class Name:          AP_API apGLSampler : public osTransferableObject
// General Description: Represents an OpenGL sampler object.
// Author:  AMD Developer Tools Team
// Creation Date:       30/4/2014
// ----------------------------------------------------------------------------------
class AP_API apGLSampler : public apAllocatedObject
{
public:
    // Self functions:
    apGLSampler(GLuint name = 0);
    apGLSampler(const apGLSampler& other);
    virtual ~apGLSampler();
    apGLSampler& operator=(const apGLSampler& other);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    void bindToTextureUnit(GLuint textureUnit);
    bool unbindToTextureUnit(GLuint textureUnit);

    // ********
    // Getters:
    // ********
    GLuint samplerName() const { return m_name; };

    void getBoundTextures(gtVector<GLuint>& buffer) const;

    void getSamplerRgbaColor(GLfloat& r, GLfloat& g, GLfloat& b, GLfloat& a) const;

    GLenum getSamplerComparisonFunction() const { return m_textureCompareFunc; }
    GLenum getSamplerComparisonMode() const { return m_textureCompareMode; }
    bool getSamplerComparisonModeAsString(gtString& buffer) const;

    GLfloat getTextureLodBias() const { return m_textureLodBias; }
    GLfloat getTextureMaxLod() const { return m_textureMaxLod; }
    GLfloat getTextureMinLod() const { return m_textureMinLod; }

    GLenum getTextureMagFilter() const { return m_textureMagFilter; }
    GLenum getTextureMinFilter() const { return m_textureMinFilter; }

    GLenum getTextureWrapS() const { return m_textureWrapS; }
    GLenum getTextureWrapT() const { return m_textureWrapT; }
    GLenum getTextureWrapR() const { return m_textureWrapR; }


    // ********
    // Setters:
    // ********
    // sets the RGBA color for the specified sampler.
    void setSamplerColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);

    void setSamplerComparisonFunction(GLenum comparisonFunction);

    void setSamplerComparisonMode(GLenum comparisonMode);

    void setSamplerLodBias(GLfloat lodBias);

    void setSamplerMaxLod(GLfloat maxLod);

    void setSamplerMinLod(GLfloat minLod);

    void setSamplerMagnificationFunction(GLenum magFunction);

    void setSamplerMinificationFunction(GLenum minFunction);

    void setSwrapMode(GLenum sWrapMode);

    void setTwrapMode(GLenum tWrapMode);

    void setRwrapMode(GLenum rWrapMode);

private:

    // Holds the name of the bound textures.
    gtVector<GLuint> m_boundTextures;

    GLuint m_name;

    // The Red part of RGBA.
    GLfloat m_textureBorderColorRed;

    // The Green part of RGBA.
    GLfloat m_textureBorderColorGreen;

    // The Blue part of RGBA.
    GLfloat m_textureBorderColorBlue;

    // The Alpha part of RGBA.
    GLfloat m_textureBorderColorAlpha;

    // Comparison function.
    GLenum m_textureCompareFunc;

    // Comparison mode.
    GLenum m_textureCompareMode;

    // Texture level of detail bias.
    GLfloat m_textureLodBias;

    // Max level of detail.
    GLfloat m_textureMaxLod;

    // Min level of detail.
    GLfloat m_textureMinLod;

    // Magnification function.
    GLenum m_textureMagFilter;

    // *********************************************************************************************************
    // Note for the following data members: m_textureMinFilter, m_textureWrapS, m_textureWrapT, m_textureWrapR -
    // These members get their default value assuming that the bound texture object is not a rectangle.
    // This wouldn't matter, since the actual object state would be retrieved in runtime from the OGL API
    // (during context data snapshot update) and override this default value anyway:
    // *********************************************************************************************************

    // Minification function.
    GLenum m_textureMinFilter;

    // Texcoord s wrap mode.
    GLenum m_textureWrapS;

    // Texcoord t wrap mode (2D, 3D, cube map textures only).
    GLenum m_textureWrapT;

    // Texcoord r wrap mode (3D textures only).
    GLenum m_textureWrapR;
};


#endif  // __APGLSAMPLER
