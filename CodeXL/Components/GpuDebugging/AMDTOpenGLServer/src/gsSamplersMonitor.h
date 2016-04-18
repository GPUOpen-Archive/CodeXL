//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsSamplersMonitor.h
///
//==================================================================================
#ifndef __gsSamplersMonitor_h
#define __gsSamplersMonitor_h

#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTAPIClasses/Include/apGLSampler.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

class gsSamplersMonitor
{
public:
    gsSamplersMonitor();
    ~gsSamplersMonitor();

    void onFirstTimeContextMadeCurrent();

    bool getSamplerData(GLuint samplerName, apGLSampler& buffer) const;

    bool getSamplerNameByIndex(size_t samplerIndex, GLuint& samplerNameBuffer) const;

    void genSamplers(GLuint samplersCount, const GLuint* pSamplersArr);

    void deleteSamplers(GLuint samplersCount, const GLuint* pSamplersArr);

    bool bindSampler(GLuint textureUnit, GLuint samplerName);

    void bindSamplers(GLuint first, GLuint samplersCount, const GLuint* pSamplerArr);

    size_t getAmountOfSamplerObjects() const;

    bool updateContextDataSnapshot();

    // Sets the RGBA color for the specified sampler.
    void setSamplerColor(GLuint samplerName, GLfloat r, GLfloat g, GLfloat b, GLfloat a);

    void setSamplerComparisonFunction(GLuint samplerName, GLenum comparisonFunction);

    void setSamplerComparisonMode(GLuint samplerName, GLenum comparisonMode);

    void setSamplerLodBias(GLuint samplerName, GLfloat lodBias);

    void setSamplerMaxLod(GLuint samplerName, GLfloat maxLod);

    void setSamplerMinLod(GLuint samplerName, GLfloat minLod);

    void setSamplerMagnificationFunction(GLuint samplerName, GLenum magFunction);

    void setSamplerMinificationFunction(GLuint samplerName, GLenum minFunction);

    void setSwrapMode(GLuint samplerName, GLenum sWrapMode);

    void setTwrapMode(GLuint samplerName, GLenum tWrapMode);

    void setRwrapMode(GLuint samplerName, GLenum rWrapMode);

private:

    apGLSampler* getSampler(GLuint samplerName) const;
    bool updateSingleSamplerData(apGLSampler* pSampler);

    // Monitored Sampler objects.
    gtVector<apGLSampler*> m_glSamplers;

    // Function pointer to OpenGL API.
    PFNGLGETSAMPLERPARAMETERFVPROC m_glGetSamplerParameterfv;
    PFNGLGETSAMPLERPARAMETERIVPROC m_glGetSamplerParameteriv;

};

#endif // __gsSamplersMonitor_h
