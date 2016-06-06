//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLProgram.cpp
///
//==================================================================================

//------------------------------ apGLProgram.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apGLShaderObject.h>
#include <AMDTAPIClasses/Include/apGLProgram.h>


// ---------------------------------------------------------------------------
// Name:        apGLProgram::apGLProgram
// Description: Constructor
// Arguments:   programName - The program OpenGL name.
//              programType - The program type.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
apGLProgram::apGLProgram(GLuint programName, apGLShadingObjectType programType)
    : apAllocatedObject(), _programName(programName), _programType(programType), _isProgramLinkedSuccessfully(false),
      _isMarkedForDeletion(false), _wasUsedInLastFrame(false), _geometryInputType(GL_TRIANGLES),
      _geometryOutputType(GL_TRIANGLE_STRIP), _maxGeometryOutputVertices(0)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLProgram::apGLProgram
// Description: Copy constructor
// Arguments:   other - The other program from which I am initialized.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
apGLProgram::apGLProgram(const apGLProgram& other)
    : apAllocatedObject(), _programName(other._programName), _programType(other._programType),
      _isProgramLinkedSuccessfully(other._isProgramLinkedSuccessfully),
      _programLinkLog(other._programLinkLog), _isMarkedForDeletion(other._isMarkedForDeletion),
      _geometryInputType(other._geometryInputType), _geometryOutputType(other._geometryOutputType),
      _maxGeometryOutputVertices(other._maxGeometryOutputVertices)
{
    // Copy the other instance member vectors:
    copyVectors(other);

    setAllocatedObjectId(other.getAllocatedObjectId(), true);
}


// ---------------------------------------------------------------------------
// Name:        apGLProgram::~apGLProgram
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
apGLProgram::~apGLProgram()
{
    clearVectors();
}


// ---------------------------------------------------------------------------
// Name:        apGLProgram::operator=
// Description: Copy other content into self.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
apGLProgram& apGLProgram::operator=(const apGLProgram& other)
{
    // Copy the program type:
    _programType = other.programType();

    // Copy the program name:
    _programName = other.programName();

    // Copy the link status:
    _isProgramLinkedSuccessfully = other.isProgramLinkedSuccesfully();

    // Copy the link log:
    _programLinkLog = other.programLinkLog();

    // Copy the "is marked for deletion" flag:
    _isMarkedForDeletion = other.isMarkedForDeletion();

    // Copy the "was used in last frame" flag:
    _wasUsedInLastFrame = other.wasUsedInLastFrame();

    // Copy the "Geometry input primitive type" enumerator:
    _geometryInputType = other.geometryInputType();

    // Copy the "Geometry output primitive type" enumerator:
    _geometryOutputType = other.geometryOutputType();

    // Copy the "Maximum geometry output vertices per input primitive" value:
    _maxGeometryOutputVertices = other.maxGeometryOutputVertices();

    // Copy the other instance member vectors:
    copyVectors(other);

    setAllocatedObjectId(other.getAllocatedObjectId(), true);

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        apGLProgram::onShaderObjectAttached
// Description: Is called when a shader object is attached to this program.
// Arguments:   shaderObj - The attached shader object.
// Return Val:  bool - Success / failure.
//                     (Failure occurs when the shader name is already attached
//                      to this program).
// Author:  AMD Developer Tools Team
// Date:        5/4/2005
// ---------------------------------------------------------------------------
bool apGLProgram::onShaderObjectAttached(const apGLShaderObject& shaderObj)
{
    bool retVal = false;

    // Get the shader object name:
    GLuint shaderName = shaderObj.shaderName();

    // Verify that the shader object is not already attached:
    if (!isShaderObjectAttached(shaderName))
    {
        // Attach the shader object to this program:
        _shaderObjects.push_back(shaderName);
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLProgram::onShaderObjectDetached
// Description: Is called when a shader object is detached from this program.
// Arguments:   shaderObj - The detached shader object.
// Return Val:  bool - Success / failure.
//                     (Failure occurs if the shader name is not attached
//                      to this program).
// Author:  AMD Developer Tools Team
// Date:        5/4/2005
// ---------------------------------------------------------------------------
bool apGLProgram::onShaderObjectDetached(const apGLShaderObject& shaderObj)
{
    bool retVal = false;

    // Get the shader object name:
    GLuint shaderName = shaderObj.shaderName();

    // Search the shader name in the attaches shader objects list:
    gtList<GLuint>::iterator firstIter = _shaderObjects.begin();
    gtList<GLuint>::iterator endIter = _shaderObjects.end();
    gtList<GLuint>::iterator searchIter = gtFind(firstIter, endIter, shaderName);

    // If we found the shader name:
    if (searchIter != endIter)
    {
        // Remove it from the list.
        _shaderObjects.erase(searchIter);
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLProgram::onProgramLink
// Description: Is called when a program is linked.
// Arguments:   wasLinkSuccessful - true iff the link was successful.
//              linkLog - The link log.
// Author:  AMD Developer Tools Team
// Date:        31/5/2005
// ---------------------------------------------------------------------------
void apGLProgram::onProgramLink(bool wasLinkSuccessful, const gtString& linkLog)
{
    _isProgramLinkedSuccessfully = wasLinkSuccessful;
    _programLinkLog = linkLog;
}


// ---------------------------------------------------------------------------
// Name:        apGLProgram::onProgramMarkedForDeletion
// Description: Is called when the program is marked for deletion.
// Author:  AMD Developer Tools Team
// Date:        6/6/2005
// ---------------------------------------------------------------------------
void apGLProgram::onProgramMarkedForDeletion()
{
    _isMarkedForDeletion = true;
}

// ---------------------------------------------------------------------------
// Name:        apGLProgram::onProgramParameteri
// Description: Called when a gl_ProgramParameteriEXT function calls changes
//              the value of one of the program's parameters
// Arguments: same as gl_ProgramParameteriEXT
// Return Val: bool  - Success / failure. Note that failure is reported even
//              for successful parameter changes if they are irrelevant to us
// Author:  AMD Developer Tools Team
// Date:        13/5/2008
// ---------------------------------------------------------------------------
bool apGLProgram::onProgramParameteri(const GLenum& pname, const GLint& value)
{
    bool retVal = false;

    switch (pname)
    {
        case GL_GEOMETRY_INPUT_TYPE_EXT:
        {
            _geometryInputType = value;
            retVal = true;
        }
        break;

        case GL_GEOMETRY_OUTPUT_TYPE_EXT:
        {
            _geometryOutputType = value;
            retVal = true;
        }
        break;

        case GL_GEOMETRY_VERTICES_OUT_EXT:
        {
            _maxGeometryOutputVertices = value;
            retVal = true;
        }
        break;

        default:
        {
            // do nothing
        }
        break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLProgram::isShaderObjectAttached
// Description: Return true iff the input shader object is attached to this
//              program.
// Arguments:   shaderName - The queried shader object OpenGL name.
// Author:  AMD Developer Tools Team
// Date:        5/4/2005
// ---------------------------------------------------------------------------
bool apGLProgram::isShaderObjectAttached(GLuint shaderName) const
{
    bool retVal = false;

    // Search the shader name in the attaches shader objects list:
    gtList<GLuint>::const_iterator firstIter = _shaderObjects.begin();
    gtList<GLuint>::const_iterator endIter = _shaderObjects.end();
    gtList<GLuint>::const_iterator searchIter = gtFind(firstIter, endIter, shaderName);

    // If we found the shader name:
    if (endIter != searchIter)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLProgram::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
osTransferableObjectType apGLProgram::type() const
{
    return OS_TOBJ_ID_GL_PROGRAM;
}


// ---------------------------------------------------------------------------
// Name:        apGLProgram::writeSelfIntoChannel
// Description: Writes my content into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
bool apGLProgram::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the program type:
    ipcChannel << _programType;

    // Write the program name:
    ipcChannel << (gtUInt32)_programName;

    // Write the link status:
    ipcChannel << _isProgramLinkedSuccessfully;

    // Write the link log:
    ipcChannel << _programLinkLog;

    // Write the "is marked for deletion" flag:
    ipcChannel << _isMarkedForDeletion;

    // Write the "was used in last frame" flag:
    ipcChannel << _wasUsedInLastFrame;

    // Write the "Geometry input primitive type" enumerator:
    ipcChannel << (gtInt32)_geometryInputType;

    // Write the "Geometry output primitive type" enumerator:
    ipcChannel << (gtInt32)_geometryOutputType;

    // Write the "Maximum geometry output vertices per input primitive" value:
    ipcChannel << (gtInt32)_maxGeometryOutputVertices;

    // Write the shader objects vector:
    gtInt64 amountOfShaderObjects = (gtInt64)_shaderObjects.size();
    ipcChannel << amountOfShaderObjects;

    gtList<GLuint>::const_iterator iter = _shaderObjects.begin();
    gtList<GLuint>::const_iterator endIter = _shaderObjects.end();

    while (iter != endIter)
    {
        ipcChannel << (gtUInt32)*iter;
        iter++;
    }

    apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLProgram::readSelfFromChannel
// Description: Reads my content from an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
bool apGLProgram::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the program type:
    gtInt32 programTypeAsInt = AP_GL_2_0_SHADING_OBJECT;
    ipcChannel >> programTypeAsInt;
    _programType = (apGLShadingObjectType)programTypeAsInt;

    // Read the program name:
    gtUInt32 programNameAsInt32 = 0;
    ipcChannel >> programNameAsInt32;
    _programName = (GLuint)programNameAsInt32;

    // Read the link status:
    ipcChannel >> _isProgramLinkedSuccessfully;

    // Read the link log:
    ipcChannel >> _programLinkLog;

    // Read the "is marked for deletion" flag:
    ipcChannel >> _isMarkedForDeletion;

    // Read the "was used in last frame" flag:
    ipcChannel >> _wasUsedInLastFrame;

    // Read the "Geometry input primitive type" enumerator:
    gtInt32 geometryInputTypeAsInt32 = 0;
    ipcChannel >> geometryInputTypeAsInt32;
    _geometryInputType = (GLint)geometryInputTypeAsInt32;

    // Read the "Geometry output primitive type" enumerator:
    gtInt32 geometryOutputTypeAsInt32 = 0;
    ipcChannel >> geometryOutputTypeAsInt32;
    _geometryOutputType = (GLint)geometryOutputTypeAsInt32;

    // Read the "Maximum geometry output vertices per input primitive" value:
    gtInt32 maxGeometryOutputVerticesAsInt32 = 0;
    ipcChannel >> maxGeometryOutputVerticesAsInt32;
    _maxGeometryOutputVertices = (GLint)maxGeometryOutputVerticesAsInt32;

    // Read the shader objects vector:
    _shaderObjects.clear();
    gtInt64 amountOfShaderObjects = 0;
    ipcChannel >> amountOfShaderObjects;

    for (int i = 0; i < amountOfShaderObjects; i++)
    {
        gtUInt32 currentShaderObjectNameAsUInt32 = 0;
        ipcChannel >> currentShaderObjectNameAsUInt32;
        GLuint currentShaderObjectName = (GLuint)currentShaderObjectNameAsUInt32;
        _shaderObjects.push_back(currentShaderObjectName);
    }

    apAllocatedObject::readSelfFromChannel(ipcChannel);

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apGLProgram::clearVectors
// Description: Clear this class vectors.
// Author:  AMD Developer Tools Team
// Date:        30/5/2005
// ---------------------------------------------------------------------------
void apGLProgram::clearVectors()
{
    _shaderObjects.clear();
}

// ---------------------------------------------------------------------------
// Name:        apGLProgram::copyVectors
// Description: Copies class member vectors values from another instance of
//              this class to me.
// Arguments: other - The other instance of this class.
// Author:  AMD Developer Tools Team
// Date:        4/4/2005
// ---------------------------------------------------------------------------
void apGLProgram::copyVectors(const apGLProgram& other)
{
    // Clear this class vectors:
    clearVectors();

    // Copy this class vectors:
    _shaderObjects = other.shaderObjects();
}

