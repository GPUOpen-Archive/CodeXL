//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdShaderType.cpp
///
//==================================================================================

//------------------------------ gdShaderType.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdShaderType.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeData.h>


// ---------------------------------------------------------------------------
// Name:        gdShaderNameStringFromNameAndType
// Description: Creates a shader's name string from its name and type (eg
//              "Vertex Shader 14", "Geometry Shader 8, "N/A Shader 19")
// Arguments: GLuint name - the shader's OpenGL name
//            gdShaderType type - the shader type
//            gtString& shaderNameString - will hold the shader's name string
// Author:      Uri Shomroni
// Date:        16/11/2008
// ---------------------------------------------------------------------------
void gdShaderNameStringFromNameAndType(/*GLuint*/ unsigned int name, gdShaderType type, gtString& shaderNameString)
{
    // Make the string contain the shader type:
    gdShaderTypeToString(type, shaderNameString);

    // Add the shader's OGL name:
    shaderNameString.appendFormattedString(L" %u", name);
}


// ---------------------------------------------------------------------------
// Name:        gdShaderTypeFromTransferableObjectType
// Description: "translates" a shader's osTransferableObjectType to the
//              appropriate gdShaderType
// Author:      Uri Shomroni
// Date:        16/11/2008
// ---------------------------------------------------------------------------
gdShaderType gdShaderTypeFromTransferableObjectType(const osTransferableObjectType& tobjType, afTreeItemType& shaderTreeObjectType)
{
    gdShaderType shadType = GD_UNKNOWN_SHADER;

    switch (tobjType)
    {
        case OS_TOBJ_ID_GL_VERTEX_SHADER:
        {
            shadType = GD_VERTEX_SHADER;
            shaderTreeObjectType = AF_TREE_ITEM_GL_VERTEX_SHADER;
        }
        break;

        case OS_TOBJ_ID_GL_TESSELLATION_CONTROL_SHADER:
        {
            shadType = GD_TESSELLATION_CONTROL_SHADER;
            shaderTreeObjectType = AF_TREE_ITEM_GL_TESSELLATION_CONTROL_SHADER;
        }
        break;

        case OS_TOBJ_ID_GL_TESSELLATION_EVALUATION_SHADER:
        {
            shadType = GD_TESSELLATION_EVALUATION_SHADER;
            shaderTreeObjectType = AF_TREE_ITEM_GL_TESSELLATION_EVALUATION_SHADER;
        }
        break;

        case OS_TOBJ_ID_GL_GEOMETRY_SHADER:
        {
            shadType = GD_GEOMETRY_SHADER;
            shaderTreeObjectType = AF_TREE_ITEM_GL_GEOMETRY_SHADER;
        }
        break;

        case OS_TOBJ_ID_GL_FRAGMENT_SHADER:
        {
            shadType = GD_FRAGMENT_SHADER;
            shaderTreeObjectType = AF_TREE_ITEM_GL_FRAGMENT_SHADER;
        }
        break;

        case OS_TOBJ_ID_GL_COMPUTE_SHADER:
        {
            shadType = GD_COMPUTE_SHADER;
            shaderTreeObjectType = AF_TREE_ITEM_GL_COMPUTE_SHADER;
        }
        break;

        case OS_TOBJ_ID_GL_UNSUPPORTED_SHADER:
        {
            shadType = GD_UNSUPPORTED_SHADER;
            shaderTreeObjectType = AF_TREE_ITEM_GL_UNSUPPORTED_SHADER;
        }
        break;

        default:
        {
            shadType = GD_UNKNOWN_SHADER;
            shaderTreeObjectType = AF_TREE_ITEM_ITEM_NONE;
            // We implemented a new shader transferable object, but didn't add it here:
            GT_ASSERT(false);
        }
        break;
    }

    return shadType;
}

// ---------------------------------------------------------------------------
// Name:        gdShaderTypeToString
// Description: Sets shaderTypeAsString to be a string appropriate to shaderType
// Author:      Uri Shomroni
// Date:        19/11/2008
// ---------------------------------------------------------------------------
void gdShaderTypeToString(const gdShaderType& shaderType, gtString& shaderTypeAsString)
{
    shaderTypeAsString.makeEmpty();

    switch (shaderType)
    {
        case GD_VERTEX_SHADER:
        {
            shaderTypeAsString.appendFormattedString(GD_STR_ShadersSourceCodeViewerListCtrlVertexShaderName);
        }
        break;

        case GD_TESSELLATION_CONTROL_SHADER:
        {
            shaderTypeAsString.appendFormattedString(GD_STR_ShadersSourceCodeViewerListCtrlTessellationControlShaderName);
        }
        break;

        case GD_TESSELLATION_EVALUATION_SHADER:
        {
            shaderTypeAsString.appendFormattedString(GD_STR_ShadersSourceCodeViewerListCtrlTessellationEvaluationShaderName);
        }
        break;

        case GD_GEOMETRY_SHADER:
        {
            shaderTypeAsString.appendFormattedString(GD_STR_ShadersSourceCodeViewerListCtrlGeometryShaderName);
        }
        break;

        case GD_FRAGMENT_SHADER:
        {
            shaderTypeAsString.appendFormattedString(GD_STR_ShadersSourceCodeViewerListCtrlFragmentShaderName);
        }
        break;

        case GD_COMPUTE_SHADER:
        {
            shaderTypeAsString.appendFormattedString(GD_STR_ShadersSourceCodeViewerListCtrlComputeShaderName);
        }
        break;

        case GD_UNSUPPORTED_SHADER:
        {
            // Legal representation of an unknown shader:
            shaderTypeAsString.appendFormattedString(GD_STR_ShadersSourceCodeViewerListCtrlUnknownShaderName);
        }
        break;

        case GD_UNKNOWN_SHADER:
        {
            // Unexpected representation of an unknown shader:
            GT_ASSERT(false);
            shaderTypeAsString.appendFormattedString(GD_STR_ShadersSourceCodeViewerListCtrlUnknownShaderName);
        }
        break;

        default:
        {
            // We added a new shader type, but didn't implement it here:
            GT_ASSERT(false);
        }
        break;
    }
}
