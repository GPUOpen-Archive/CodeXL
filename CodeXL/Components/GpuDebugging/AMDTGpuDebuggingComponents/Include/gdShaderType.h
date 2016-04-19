//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdShaderType.h
///
//==================================================================================

//------------------------------ gdShaderType.h ------------------------------

#ifndef __GDSHADERTYPE_H
#define __GDSHADERTYPE_H

// Infra:
#include <AMDTOSWrappers/Include/osTransferableObjectType.h>
#include <AMDTApplicationFramework/Include/afTreeItemType.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>


// Forward Declarations:
class gtString;

// ----------------------------------------------------------------------------------
// Class Name:           gdShaderType
// General Description: An enumeration describing a shader type
//                      not to be confused with apGLShaderObjectType, which in fact
//                      describes the shader's (OpenGL implementation) version
// Author:               Uri Shomroni
// Creation Date:        16/11/2008
// ----------------------------------------------------------------------------------
enum gdShaderType
{
    GD_VERTEX_SHADER,
    GD_TESSELLATION_CONTROL_SHADER,
    GD_TESSELLATION_EVALUATION_SHADER,
    GD_GEOMETRY_SHADER,
    GD_FRAGMENT_SHADER,
    GD_COMPUTE_SHADER,
    GD_UNSUPPORTED_SHADER,
    GD_UNKNOWN_SHADER
};

GD_API void gdShaderNameStringFromNameAndType(/*GLuint*/ unsigned int name, gdShaderType type, gtString& shaderNameString);
GD_API gdShaderType gdShaderTypeFromTransferableObjectType(const osTransferableObjectType& tobjType, afTreeItemType& shaderTreeObjectType);
GD_API void gdShaderTypeToString(const gdShaderType& shaderType, gtString& shaderTypeAsString);

#endif //__GDSHADERTYPE_H

