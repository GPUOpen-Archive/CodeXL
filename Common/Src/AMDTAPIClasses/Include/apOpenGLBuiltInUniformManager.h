//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenGLBuiltInUniformManager.h
///
//==================================================================================

//------------------------------ apOpenGLBuiltInUniformManager.h ------------------------------

#ifndef __APOPENGLBUILTINUNIFORMMANAGER
#define __APOPENGLBUILTINUNIFORMMANAGER

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>


class AP_API apOpenGLBuiltInUniformManager
{
public:
    enum apBuiltInUniformId
    {
        ap_gl_BackLightModelProduct,
        ap_gl_BackLightProduct,
        ap_gl_BackMaterial,
        ap_gl_ClipPlane,
        ap_gl_DepthRange,
        ap_gl_EyePlaneQ,
        ap_gl_EyePlaneR,
        ap_gl_EyePlaneS,
        ap_gl_EyePlaneT,
        ap_gl_Fog,
        ap_gl_FrontLightModelProduct,
        ap_gl_FrontLightProduct,
        ap_gl_FrontMaterial,
        ap_gl_LightModel,
        ap_gl_LightSource,
        ap_gl_ModelViewMatrix,
        ap_gl_ModelViewMatrixInverse,
        ap_gl_ModelViewMatrixInverseTranspose,
        ap_gl_ModelViewMatrixTranspose,
        ap_gl_ModelViewProjectionMatrix,
        ap_gl_ModelViewProjectionMatrixInverse,
        ap_gl_ModelViewProjectionMatrixInverseTranspose,
        ap_gl_ModelViewProjectionMatrixTranspose,
        ap_gl_NormalMatrix,
        ap_gl_NormalScale,
        ap_gl_NumSamples,
        ap_gl_ObjectPlaneQ,
        ap_gl_ObjectPlaneR,
        ap_gl_ObjectPlaneS,
        ap_gl_ObjectPlaneT,
        ap_gl_Point,
        ap_gl_ProjectionMatrix,
        ap_gl_ProjectionMatrixInverse,
        ap_gl_ProjectionMatrixInverseTranspose,
        ap_gl_ProjectionMatrixTranspose,
        ap_gl_TextureEnvColor,
        ap_gl_TextureMatrix,
        ap_gl_TextureMatrixInverse,
        ap_gl_TextureMatrixInverseTranspose,
        ap_gl_TextureMatrixTranspose,
        ap_numberOfSupportedBuiltInUniforms
    };

    enum apBuiltInUniformMemberId
    {
        ap_gl_TextureMatrix_i,
        ap_gl_TextureMatrixInverse_i,
        ap_gl_TextureMatrixTranspose_i,
        ap_gl_TextureMatrixInverseTranspose_i,
        ap_gl_DepthRange_near,
        ap_gl_DepthRange_far,
        ap_gl_DepthRange_diff,
        ap_gl_ClipPlane_i,
        ap_gl_Point_size,
        ap_gl_Point_sizeMin,
        ap_gl_Point_sizeMax,
        ap_gl_Point_fadeThresholdSize,
        ap_gl_Point_distanceConstantAttenuation,
        ap_gl_Point_distanceLinearAttenuation,
        ap_gl_Point_distanceQuadraticAttenuation,
        ap_gl_FrontMaterial_emission,
        ap_gl_FrontMaterial_ambient,
        ap_gl_FrontMaterial_diffuse,
        ap_gl_FrontMaterial_specular,
        ap_gl_FrontMaterial_shininess,
        ap_gl_BackMaterial_emission,
        ap_gl_BackMaterial_ambient,
        ap_gl_BackMaterial_diffuse,
        ap_gl_BackMaterial_specular,
        ap_gl_BackMaterial_shininess,
        ap_gl_LightSource_i_ambient,
        ap_gl_LightSource_i_diffuse,
        ap_gl_LightSource_i_specular,
        ap_gl_LightSource_i_position,
        ap_gl_LightSource_i_halfVector,
        ap_gl_LightSource_i_spotDirection,
        ap_gl_LightSource_i_spotExponent,
        ap_gl_LightSource_i_spotCutoff,
        ap_gl_LightSource_i_spotCosCutoff,
        ap_gl_LightSource_i_constantAttenuation,
        ap_gl_LightSource_i_linearAttenuation,
        ap_gl_LightSource_i_quadraticAttenuation,
        ap_gl_LightModel_ambient,
        ap_gl_FrontLightModelProduct_sceneColor,
        ap_gl_BackLightModelProduct_sceneColor,
        ap_gl_FrontLightProduct_i_ambient,
        ap_gl_FrontLightProduct_i_diffuse,
        ap_gl_FrontLightProduct_i_specular,
        ap_gl_BackLightProduct_i_ambient,
        ap_gl_BackLightProduct_i_diffuse,
        ap_gl_BackLightProduct_i_specular,
        ap_gl_TextureEnvColor_i,
        ap_gl_EyePlaneS_i,
        ap_gl_EyePlaneT_i,
        ap_gl_EyePlaneR_i,
        ap_gl_EyePlaneQ_i,
        ap_gl_ObjectPlaneS_i,
        ap_gl_ObjectPlaneT_i,
        ap_gl_ObjectPlaneR_i,
        ap_gl_ObjectPlaneQ_i,
        ap_gl_Fog_color,
        ap_gl_Fog_density,
        ap_gl_Fog_start,
        ap_gl_Fog_end,
        ap_gl_Fog_scale,
        ap_numberOfSupportedBuiltInUniformMembers
    };

public:
    static apOpenGLBuiltInUniformManager& instance();

    bool ParseUniformMemberAccess(const gtString& fullName, apBuiltInUniformId& id, int& index, apBuiltInUniformMemberId& mid) const;
    bool GetBuiltInUniformOrMemberFormula(const gtString& fullName, gtString& formula, bool htmlFormat = true) const;

    bool GetBuiltInUniformId(const gtString& uniformName, apBuiltInUniformId& id) const;
    const gtString& UniformName(apBuiltInUniformId id) const;
    const gtString& UniformCalculationFormula(apBuiltInUniformId id, bool htmlFormat = true) const;
    const gtString& UniformMemberCalculationFormula(apBuiltInUniformMemberId id, bool htmlFormat = true) const;

private:
    friend class apSingeltonsDelete;

    struct apBuiltInUniformData
    {
        gtString m_name;
        gtString m_calculationFormula;
        gtString m_calculationFormulaHTML;
    };

    struct apBuiltInUniformDataWithMembers : apBuiltInUniformData
    {
        gtMap<gtString, apBuiltInUniformMemberId> m_memberMapping;
    };

private:
    apOpenGLBuiltInUniformManager();
    ~apOpenGLBuiltInUniformManager();

    void InitializeUniformsData();

    void SetUniformData(apBuiltInUniformId id, const gtString& name, const gtString& formula, const gtString& formulaHTML);
    void SetUniformMemberData(apBuiltInUniformMemberId mid, apBuiltInUniformId id, const gtString& formula, const gtString& formulaHTML);
    void SetUniformMemberData(apBuiltInUniformMemberId mid, apBuiltInUniformId id, const gtString& memberName, const gtString& formula, const gtString& formulaHTML);

private:
    static apOpenGLBuiltInUniformManager* m_spMySingleInstance;
    apBuiltInUniformDataWithMembers m_uniformData[ap_numberOfSupportedBuiltInUniforms];
    apBuiltInUniformData m_uniformMemberData[ap_numberOfSupportedBuiltInUniformMembers];
};


#endif  // __APOPENGLBUILTINUNIFORMMANAGER
