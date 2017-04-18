#ifndef __beDataTypes_h
#define __beDataTypes_h
#include <AMDTBaseTools/Include/gtString.h>

// A structure to hold the shader file names of a given pipeline.
struct beProgramPipeline
{
    // Clears all pipeline shaders.
    void ClearAll()
    {
        m_vertexShader.makeEmpty();
        m_tessControlShader.makeEmpty();
        m_tessEvaluationShader.makeEmpty();
        m_geometryShader.makeEmpty();
        m_fragmentShader.makeEmpty();
        m_computeShader.makeEmpty();
    }

    // Vertex shader.
    gtString m_vertexShader;

    // Tessellation control shader.
    gtString m_tessControlShader;

    // Tessellation evaluation shader.
    gtString m_tessEvaluationShader;

    // Geometry shader.
    gtString m_geometryShader;

    // Fragment shader.
    gtString m_fragmentShader;

    // Compute shader.
    gtString m_computeShader;
};


#endif // __beDataTypes_h
