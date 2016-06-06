//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLPipeline.cpp
///
//==================================================================================

// -----------------------------   apGLPipeline.cpp ------------------------------

// Infra.
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osChannel.h>

// OpenGL.
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Local.
#include <AMDTAPIClasses/Include/apGLPipeline.h>

// ---------------------------------------------------------------------------
// Name:        apGLPipeline::apGLPipeline
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        30/4/2014
// ---------------------------------------------------------------------------
apGLPipeline::apGLPipeline(GLuint name)
    : apAllocatedObject(), m_name(name), m_activeProgram(0), m_vertexShaderName(0), m_geometryShaderName(0),
      m_fragmentShaderName(0), m_computeShaderName(0), m_tessEvaluationShaderName(0), m_tessControlShaderName(0),
      m_isBound(false)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLPipeline::apGLPipeline
// Description: Copy constructor
// Arguments: other - The other object from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        30/4/2014
// ---------------------------------------------------------------------------
apGLPipeline::apGLPipeline(const apGLPipeline& other)
{
    apGLPipeline::operator=(other);
}


// ---------------------------------------------------------------------------
// Name:        apGLPipeline::~apGLPipeline
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        30/4/2014
// ---------------------------------------------------------------------------
apGLPipeline::~apGLPipeline()
{
}

// ---------------------------------------------------------------------------
// Name:        apGLPipeline::operator=
// Description: Assignment operator
// Arguments:   other - The other object from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        30/4/2014
// ---------------------------------------------------------------------------
apGLPipeline& apGLPipeline::operator=(const apGLPipeline& other)
{
    if (this != &other)
    {
        m_name = other.m_name;
        m_activeProgram = other.m_activeProgram;
        m_vertexShaderName = other.m_vertexShaderName;
        m_geometryShaderName = other.m_geometryShaderName;
        m_fragmentShaderName = other.m_fragmentShaderName;
        m_computeShaderName = other.m_computeShaderName;
        m_tessEvaluationShaderName = other.m_tessEvaluationShaderName;
        m_tessControlShaderName = other.m_tessControlShaderName;
        m_isBound = other.m_isBound;
        setAllocatedObjectId(other.getAllocatedObjectId(), true);
    }

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        apGLPipeline::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        23/6/2014
// ---------------------------------------------------------------------------
osTransferableObjectType apGLPipeline::type() const
{
    return OS_TOBJ_ID_GL_PIPELINE;
}

// ---------------------------------------------------------------------------
// Name:        apGLPipeline::writeSelfIntoChannel
// Description: Writes this class into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        23/6/2014
// ---------------------------------------------------------------------------
bool apGLPipeline::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    ipcChannel << (gtUInt32)m_name;
    ipcChannel << (gtUInt32)m_activeProgram;
    ipcChannel << (gtUInt32)m_vertexShaderName;
    ipcChannel << (gtUInt32)m_geometryShaderName;
    ipcChannel << (gtUInt32)m_fragmentShaderName;
    ipcChannel << (gtUInt32)m_computeShaderName;
    ipcChannel << (gtUInt32)m_tessEvaluationShaderName;
    ipcChannel << (gtUInt32)m_tessControlShaderName;
    ipcChannel << m_isBound;

    apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLPipeline::readSelfFromChannel
// Description: Read self from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        23/6/2014
// ---------------------------------------------------------------------------
bool apGLPipeline::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    gtUInt32 nameAsUInt32 = 0;
    gtUInt32 activeProgramAsUInt32 = 0;
    gtUInt32 vertexShaderAsUInt32 = 0;
    gtUInt32 geometryShaderAsUInt32 = 0;
    gtUInt32 fragmentShaderAsUInt32 = 0;
    gtUInt32 computeShaderAsUInt32 = 0;
    gtUInt32 tessEvaluationShaderAsUInt32 = 0;
    gtUInt32 tessControlShaderAsUInt32 = 0;

    // Read the values from the channel.
    // Then, assign the values to the data members.
    ipcChannel >> nameAsUInt32;
    ipcChannel >> activeProgramAsUInt32;
    ipcChannel >> vertexShaderAsUInt32;
    ipcChannel >> geometryShaderAsUInt32;
    ipcChannel >> fragmentShaderAsUInt32;
    ipcChannel >> computeShaderAsUInt32;
    ipcChannel >> tessEvaluationShaderAsUInt32;
    ipcChannel >> tessControlShaderAsUInt32;
    ipcChannel >> m_isBound;

    m_name = (GLenum)nameAsUInt32;
    m_activeProgram = (GLenum)activeProgramAsUInt32;
    m_vertexShaderName = (GLenum)vertexShaderAsUInt32;
    m_geometryShaderName = (GLenum)geometryShaderAsUInt32;
    m_fragmentShaderName = (GLenum)fragmentShaderAsUInt32;
    m_computeShaderName = (GLenum)computeShaderAsUInt32;
    m_tessEvaluationShaderName = (GLenum)tessEvaluationShaderAsUInt32;
    m_tessControlShaderName = (GLenum)tessControlShaderAsUInt32;

    apAllocatedObject::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLPipeline::setIsPipelineBound
// Description: Sets whether the pipeline is bound or not
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        23/6/2014
// ---------------------------------------------------------------------------
void apGLPipeline::setIsPipelineBound(bool isBound)
{
    m_isBound = isBound;
}

// -------------------------------------------------------------------------------------
// Name:        apGLPipeline::isPipelineBound
// Description: Returns true if the pipeline has been set as bound, and false otherwise
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        23/6/2014
// -------------------------------------------------------------------------------------
bool apGLPipeline::isPipelineBound() const
{
    return m_isBound;
}

// ---------------------------------------------------------------------------
// Name:        apGLPipeline::setActiveProgram
// Description: Sets program as the active program for the pipeline
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        23/6/2014
// ---------------------------------------------------------------------------
void apGLPipeline::setActiveProgram(GLuint program)
{
    m_activeProgram = program;
}

// ---------------------------------------------------------------------------
// Name:        apGLPipeline::useProgramStages
// Description: Tells the pipeline to use program as the current program object
//              for the pipeline stages mentioned by the stages parameter.
// Arguments:   GLbitfield stages - the pipeline stages for which to set program
//                                  as the shader program
//              GLuint program - the name of the shader program
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        23/6/2014
// ---------------------------------------------------------------------------
void apGLPipeline::useProgramStages(GLbitfield stages, GLuint program)
{
    if (stages == GL_ALL_SHADER_BITS)
    {
        // Assign program to all stages.
        m_computeShaderName             =
            m_vertexShaderName              =
                m_tessControlShaderName         =
                    m_tessEvaluationShaderName      =
                        m_geometryShaderName            =
                            m_fragmentShaderName            = program;
    }
    else
    {
        // Check which particular shaders were selected.
        if ((stages & GL_COMPUTE_SHADER_BIT) != 0)
        {
            m_computeShaderName = program;
        }

        if ((stages & GL_VERTEX_SHADER_BIT) != 0)
        {
            m_vertexShaderName = program;
        }

        if ((stages & GL_TESS_CONTROL_SHADER_BIT) != 0)
        {
            m_tessControlShaderName = program;
        }

        if ((stages & GL_TESS_EVALUATION_SHADER_BIT) != 0)
        {
            m_tessEvaluationShaderName = program;
        }

        if ((stages & GL_GEOMETRY_SHADER_BIT) != 0)
        {
            m_geometryShaderName = program;
        }

        if ((stages &  GL_FRAGMENT_SHADER_BIT) != 0)
        {
            m_fragmentShaderName = program;
        }
    }
}

// --------------------------------------------------------
// Name:        apGLPipeline::getActiveProgram
// Description: Returns the active program for the pipeline
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        23/6/2014
// --------------------------------------------------------
GLuint apGLPipeline::getActiveProgram() const
{
    return m_activeProgram;
}

// --------------------------------------------------------------------------
// Name:        apGLPipeline::getVertexShader
// Description: Returns the name of the current vertex shader of the pipeline
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        23/6/2014
// ---------------------------------------------------------------------------
GLuint apGLPipeline::getVertexShader() const
{
    return m_vertexShaderName;
}

// --------------------------------------------------------------------------
// Name:        apGLPipeline::getGeometryShader
// Description: Returns the name of the current geometry shader of the pipeline
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        23/6/2014
// ---------------------------------------------------------------------------
GLuint apGLPipeline::getGeometryShader() const
{
    return m_geometryShaderName;
}

// --------------------------------------------------------------------------
// Name:        apGLPipeline::getFragmentShader
// Description: Returns the name of the current fragment shader of the pipeline
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        23/6/2014
// ---------------------------------------------------------------------------
GLuint apGLPipeline::getFragmentShader() const
{
    return m_fragmentShaderName;
}

// --------------------------------------------------------------------------
// Name:        apGLPipeline::getComputeShader
// Description: Returns the name of the current compute shader of the pipeline
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        23/6/2014
// ---------------------------------------------------------------------------
GLuint apGLPipeline::getComputeShader() const
{
    return m_computeShaderName;
}

// --------------------------------------------------------------------------
// Name:        apGLPipeline::getTessCtrlShader
// Description: Returns the name of the current TCS program of the pipeline
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        23/6/2014
// ---------------------------------------------------------------------------
GLuint apGLPipeline::getTessCtrlShader() const
{
    return m_tessControlShaderName;
}

// --------------------------------------------------------------------------
// Name:        apGLPipeline::getTessCtrlShader
// Description: Returns the name of the current TES program of the pipeline
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        23/6/2014
// ---------------------------------------------------------------------------
GLuint apGLPipeline::getTessEvaluationShader() const
{
    return m_tessEvaluationShaderName;
}


