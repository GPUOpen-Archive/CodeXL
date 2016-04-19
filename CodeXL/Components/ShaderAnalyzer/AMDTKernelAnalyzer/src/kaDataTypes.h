#ifndef __kaDataTypes_h
#define __kaDataTypes_h

#include <AMDTBaseTools/Include/gtString.h>

enum AnalyzerBuildArchitecture
{
    kaBuildArch32_bit = 0,
    kaBuildArch64_bit,
};


enum kaPlatform
{
    kaPlatformUnknown = -1,
    kaPlatformOpenCL,
    kaPlatformOpenGL,
    kaPlatformDirectX,
    kaPlatformVulkan,
};

enum kaProgramTypes
{
    kaProgramUnknown = -1,
    kaProgramGL_Rendering,
    kaProgramGL_Compute,
    kaProgramVK_Rendering,
    kaProgramVK_Compute,
    kaProgramCL,
    kaProgramDX,
};

enum BuildType
{
    CL_BUILD,
    DX_BUILD,
    DX_FXC_BUILD,
    GL_BUILD,
    VK_BUILD,
};

enum kaFileTypes
{
    kaFileTypeUnknown = -1,
    kaFileTypeOpenCL,
    kaFileTypeDXVertex,
    kaFileTypeDXHull,
    kaFileTypeDXDomain,
    kaFileTypeDXGeometry,
    kaFileTypeDXPixel,
    kaFileTypeDXCompute,
    kaFileTypeDXGenericShader,
    kaFileTypeGLSLVert,
    kaFileTypeGLSLTesc,
    kaFileTypeGLSLTese,
    kaFileTypeGLSLGeom,
    kaFileTypeGLSLFrag,
    kaFileTypeGLSLComp,
    kaFileTypeGLSLGenericShader,
};

struct kaPipelineShaders
{
    void Clear() { m_vertexShader = m_tessControlShader = m_tessEvaluationShader = m_geometryShader = m_fragmentShader = m_computeShader = L""; }
    gtString m_vertexShader;
    gtString m_tessControlShader;
    gtString m_tessEvaluationShader;
    gtString m_geometryShader;
    gtString m_fragmentShader;
    gtString m_computeShader;
};

#endif // __kaDataTypes_h
