//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenGLStateVariablesManager.cpp
///
//==================================================================================

//------------------------------ apOpenGLStateVariablesManager.cpp ------------------------------

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTAPIClasses/Include/apOpenGLAPIType.h>
#include <AMDTAPIClasses/Include/apOpenGLStateVariablesManager.h>


// Static members initializations:
apOpenGLStateVariablesManager* apOpenGLStateVariablesManager::_pMySingleInstance = NULL;


// ---------------- apOpenGLStateVariablesManager::StateVariableData ----------------------


// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::StateVariableData::StateVariableData
// Description: Default constructor - Initialize the State variable data to
//              contain 0 elements.
// Author:  AMD Developer Tools Team
// Date:        15/7/2004
// ---------------------------------------------------------------------------
apOpenGLStateVariablesManager::StateVariableData::StateVariableData()
    : _name(NULL), _isEnumStateVariable(false), _getFunctionId(ap_glGetIntegerv),
      _amountOfElementsN(0), _amountOfElementsM(0), _openGLParamEnum(0), _additionalOpenGLParamEnum(0),
      _removedVersion(AP_GL_VERSION_NONE), _stateVariableGlobalType(0), _cglEnumerationAsUInt(0)
{
}


// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::StateVariableData::StateVariableData
// Description: Enum element (GLenum) state variable constructor
// Arguments:   name - The state variable name.
//              openGLParamEnum - The OpenGL state variable parameter enum:
//                                (The parameter we give as an input to glGetXXX functions
//                                 when we ask for the variable value)
//              additionalOpenGLParamEnum - Some OpenGL get commands require an additional enum param.
//              stateVariableGlobalType - The state variable type (as a bit mask of apOpenGLAPIType):
// Author:  AMD Developer Tools Team
// Date:        15/7/2004
// ---------------------------------------------------------------------------
apOpenGLStateVariablesManager::StateVariableData::StateVariableData(const wchar_t* name, GLenum openGLParamEnum,
                                                                    GLenum additionalOpenGLParamEnum, unsigned int stateVariableGlobalType)
    : _name(name), _isEnumStateVariable(true), _getFunctionId(ap_glGetIntegerv),
      _amountOfElementsN(1), _amountOfElementsM(0), _openGLParamEnum(openGLParamEnum),
      _additionalOpenGLParamEnum(additionalOpenGLParamEnum), _removedVersion(AP_GL_VERSION_NONE),
      _stateVariableGlobalType(stateVariableGlobalType), _cglEnumerationAsUInt(0)
{
}

// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::StateVariableData::StateVariableData
// Description: Single element state variable constructor
// Arguments:   name - The state variable name.
//              getFunctionId - The OpenGL function that is used for getting the value of this state variable.
//              openGLParamEnum - The OpenGL state variable parameter enum:
//                                (The parameter we give as an input to glGetXXX functions
//                                 when we ask for the variable value)
//              additionalOpenGLParamEnum - Some OpenGL get commands require an additional enum param.
//              stateVariableGlobalType - The state variable type (as a bit mask of apOpenGLAPIType):
// Author:  AMD Developer Tools Team
// Date:        15/7/2004
// ---------------------------------------------------------------------------
apOpenGLStateVariablesManager::StateVariableData::StateVariableData(const wchar_t* name, apMonitoredFunctionId getFunctionId,
                                                                    GLenum openGLParamEnum, GLenum additionalOpenGLParamEnum,
                                                                    unsigned int stateVariableGlobalType)
    : _name(name), _isEnumStateVariable(false), _getFunctionId(getFunctionId),
      _amountOfElementsN(1), _amountOfElementsM(0),
      _openGLParamEnum(openGLParamEnum), _additionalOpenGLParamEnum(additionalOpenGLParamEnum), _removedVersion(AP_GL_VERSION_NONE),
      _stateVariableGlobalType(stateVariableGlobalType), _cglEnumerationAsUInt(0)
{
}


// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::StateVariableData::StateVariableData
// Description: Vector state variable constructor:
// Arguments:   name - The state variable name.
//              getFunctionId - The OpenGL function that is used for getting the value of this state variable.
//              elementsAmount - The amount of vector elements.
//              openGLParamEnum - The OpenGL state variable parameter enum:
//                                (The parameter we give as an input to glGetXXX functions
//                                 when we ask for the variable value)
//              additionalOpenGLParamEnum - Some OpenGL get commands require an additional enum param.
//              stateVariableGlobalType - The state variable type (as a bit mask of apOpenGLAPIType):
// Author:  AMD Developer Tools Team
// Date:        15/7/2004
// ---------------------------------------------------------------------------
apOpenGLStateVariablesManager::StateVariableData::StateVariableData(const wchar_t* name, apMonitoredFunctionId getFunctionId,
                                                                    int elementsAmount, GLenum openGLParamEnum,
                                                                    GLenum additionalOpenGLParamEnum, unsigned int stateVariableGlobalType)
    : _name(name), _isEnumStateVariable(false), _getFunctionId(getFunctionId),
      _amountOfElementsN(elementsAmount), _amountOfElementsM(0),
      _openGLParamEnum(openGLParamEnum), _additionalOpenGLParamEnum(additionalOpenGLParamEnum), _removedVersion(AP_GL_VERSION_NONE),
      _stateVariableGlobalType(stateVariableGlobalType), _cglEnumerationAsUInt(0)
{
}


// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::StateVariableData::StateVariableData
// Description: Matrix state variable constructor:
// Arguments:   name - The state variable name.
//              getFunctionId - The OpenGL function that is used for getting the value of this state variable.
//              sizeN, sizeM - The Matrix size (N x M).
//              openGLParamEnum - The OpenGL state variable parameter enum:
//                                (The parameter we give as an input to glGetXXX functions
//                                 when we ask for the variable value)
//              additionalOpenGLParamEnum - Some OpenGL get commands require an additional enum param.
//              stateVariableGlobalType - The state variable type (as a bit mask of apOpenGLAPIType):
// Author:  AMD Developer Tools Team
// Date:        15/7/2004
// ---------------------------------------------------------------------------
apOpenGLStateVariablesManager::StateVariableData::StateVariableData(const wchar_t* name, apMonitoredFunctionId getFunctionId,
                                                                    int sizeN, int sizeM, GLenum openGLParamEnum,
                                                                    GLenum additionalOpenGLParamEnum, unsigned int stateVariableGlobalType)
    : _name(name), _isEnumStateVariable(false), _getFunctionId(getFunctionId),
      _amountOfElementsN(sizeN), _amountOfElementsM(sizeM),
      _openGLParamEnum(openGLParamEnum), _additionalOpenGLParamEnum(additionalOpenGLParamEnum), _removedVersion(AP_GL_VERSION_NONE),
      _stateVariableGlobalType(stateVariableGlobalType), _cglEnumerationAsUInt(0)
{
}


// CGL state variable constructor:

// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::StateVariableData::StateVariableData
// Description:
// Arguments: const wchar_t* name - The state variable name.
//            apMonitoredFunctionId getFunctionId - The CGL function that is used for getting the value of this state variable.
//            unsigned int cglEnumerationAsInt - the CGL variable enumeration value as int
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
apOpenGLStateVariablesManager::StateVariableData::StateVariableData(const wchar_t* name, apMonitoredFunctionId getFunctionId,
                                                                    unsigned int cglEnumerationAsInt)
    : _name(name), _isEnumStateVariable(false), _getFunctionId(getFunctionId), _amountOfElementsN(0), _amountOfElementsM(0),
      _openGLParamEnum(0), _additionalOpenGLParamEnum(0), _removedVersion(AP_GL_VERSION_NONE),
      _stateVariableGlobalType(AP_CGL_STATE_VAR), _cglEnumerationAsUInt(cglEnumerationAsInt)
{

}

// ---------------------------- apOpenGLStateVariablesManager ----------------------



// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::instance
// Description: Returns the single instance of this class.
//              If it does not exist - creates it.
// Author:  AMD Developer Tools Team
// Date:        11/7/2004
// ---------------------------------------------------------------------------
apOpenGLStateVariablesManager& apOpenGLStateVariablesManager::instance()
{
    // If my single instance does not exist - create it:
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new apOpenGLStateVariablesManager;
        GT_ASSERT(_pMySingleInstance);
    }

    return *_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::apOpenGLStateVariablesManager
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        11/7/2004
// ---------------------------------------------------------------------------
apOpenGLStateVariablesManager::apOpenGLStateVariablesManager()
{
    // Initialize the _stateVariablesData array:
    initializeStateVariablesData();
}


// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::~apOpenGLStateVariablesManager
// Description: Destructor.
// Author:  AMD Developer Tools Team
// Date:        11/7/2004
// ---------------------------------------------------------------------------
apOpenGLStateVariablesManager::~apOpenGLStateVariablesManager()
{
    terminateStateVariablesData();
}


// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::stateVariableType
// Description: Inputs the index of a state variable and returns its type.
// Author:  AMD Developer Tools Team
// Date:        18/7/2004
// ---------------------------------------------------------------------------
apOpenGLStateVariablesManager::apStateVariableType apOpenGLStateVariablesManager::stateVariableType(int variableIndex) const
{
    apStateVariableType retVal = apOpenGLStateVariablesManager::SimpleStateVariable;

    // Sanity check:
    GT_IF_WITH_ASSERT(_stateVariablesData[variableIndex] != NULL)
    {
        if (_stateVariablesData[variableIndex]->_amountOfElementsN > 1)
        {
            if (_stateVariablesData[variableIndex]->_amountOfElementsM > 0)
            {
                retVal = apOpenGLStateVariablesManager::MatrixStateVariable;
            }
            else
            {
                retVal = apOpenGLStateVariablesManager::VectorStateVariable;
            }
        }
        else
        {
            if (_stateVariablesData[variableIndex]->_isEnumStateVariable)
            {
                if (_stateVariablesData[variableIndex]->_additionalOpenGLParamEnum != 0)
                {
                    retVal = apOpenGLStateVariablesManager::VectorStateVariable;
                }
            }
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::stateVariableGlobalType
// Description: Queries the global type of a given state variable.
// Arguments: variableIndex - The queried state variable index.
// Return Val: int - Will get the state variable type as a bit mask of apOpenGLAPIType.
// Author:  AMD Developer Tools Team
// Date:        5/3/2007
// ---------------------------------------------------------------------------
unsigned int apOpenGLStateVariablesManager::stateVariableGlobalType(int variableIndex) const
{
    unsigned int retVal = 0;

    // Sanity check:
    GT_IF_WITH_ASSERT(_stateVariablesData[variableIndex] != NULL)
    {
        retVal = _stateVariablesData[variableIndex]->_stateVariableGlobalType;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::stateVariableName
// Description: Queries a given state variable name.
// Arguments: variableIndex - The queried state variable index.
// Return Val: const wchar_t* - Will get the state variable name.
// Author:  AMD Developer Tools Team
// Date:        5/3/2007
// ---------------------------------------------------------------------------
const wchar_t* apOpenGLStateVariablesManager::stateVariableName(int variableIndex) const
{
    const wchar_t* retVal = NULL;

    // Sanity check:
    GT_IF_WITH_ASSERT(_stateVariablesData[variableIndex] != NULL)
    {
        retVal = _stateVariablesData[variableIndex]->_name;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::amountOfStateVariableElementsN
// Description: Queries a given state variable N size (of N x M in case of a vector / matrix).
// Arguments: variableIndex - The queried state variable index.
// Return Val: int - Will get the state variable N size.
// Author:  AMD Developer Tools Team
// Date:        5/3/2007
// ---------------------------------------------------------------------------
int apOpenGLStateVariablesManager::amountOfStateVariableElementsN(int variableIndex) const
{
    int retVal = 0;

    // Sanity check:
    GT_IF_WITH_ASSERT(_stateVariablesData[variableIndex] != NULL)
    {
        retVal = _stateVariablesData[variableIndex]->_amountOfElementsN;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::amountOfStateVariableElementsM
// Description: Queries a given state variable M size (of N x M in case of a vector / matrix).
// Arguments: variableIndex - The queried state variable index.
// Return Val: int - Will get the state variable M size.
// Author:  AMD Developer Tools Team
// Date:        5/3/2007
// ---------------------------------------------------------------------------
int apOpenGLStateVariablesManager::amountOfStateVariableElementsM(int variableIndex) const
{
    int retVal = 0;

    // Sanity check:
    GT_IF_WITH_ASSERT(_stateVariablesData[variableIndex] != NULL)
    {
        retVal = _stateVariablesData[variableIndex]->_amountOfElementsM;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::stateVariableGetFunctionId
// Description: Queries a given state variable get function.
// Arguments: variableIndex - The queried state variable index.
// Return Val: apMonitoredFunctionId - Will get the id of a function that can
//                                     be used to retrieve the state variable
//                                     value.
// Author:  AMD Developer Tools Team
// Date:        5/3/2007
// ---------------------------------------------------------------------------
apMonitoredFunctionId apOpenGLStateVariablesManager::stateVariableGetFunctionId(int variableIndex) const
{
    apMonitoredFunctionId retVal = ap_glGetIntegerv;

    // Sanity check:
    GT_IF_WITH_ASSERT(_stateVariablesData[variableIndex] != NULL)
    {
        retVal = _stateVariablesData[variableIndex]->_getFunctionId;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::stateVariableOpenGLParamEnum
// Description: Queries a given state variable GLEnum parameter that is used
//              for retrieving its value.
// Arguments: variableIndex - The queried state variable index.
// Return Val: GLenum - Will get the GLenum that is used for retrieving the
//                      state variable value.
// Author:  AMD Developer Tools Team
// Date:        5/3/2007
// ---------------------------------------------------------------------------
GLenum apOpenGLStateVariablesManager::stateVariableOpenGLParamEnum(int variableIndex) const
{
    GLenum retVal = GL_LIGHT0;

    // Sanity check:
    GT_IF_WITH_ASSERT(_stateVariablesData[variableIndex] != NULL)
    {
        retVal = _stateVariablesData[variableIndex]->_openGLParamEnum;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::stateVariableOpenGLParamEnum
// Description: Queries a given state variable CGL enumeration parameter that is used
//              for retrieving its value.
// Arguments: variableIndex - The queried state variable index.
// Return Val: unsigned int  - Will get the unsigned int that is used for retrieving the
//             CGL state variable value.
// Author:  AMD Developer Tools Team
// Date:        15/12/2008
// ---------------------------------------------------------------------------
unsigned int apOpenGLStateVariablesManager::stateVariableCGLParam(int variableIndex) const
{
    unsigned int retVal = 0;

    // Sanity check:
    GT_IF_WITH_ASSERT(_stateVariablesData[variableIndex] != NULL)
    {
        retVal = _stateVariablesData[variableIndex]->_cglEnumerationAsUInt;
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::stateVariableAdditionalOpenGLParamEnum
// Description:
//  Few OpenGL state variables requires two GLenum's to described the query data.
//  Example: To query the value of GL_EMISSION you need to also specify the queried face
//           (GL_BACK, etc).
//  This function Queries a given state variable additional GLEnum parameter
//  that is used for retrieving its value.
// Arguments: variableIndex - The queried state variable index.
// Return Val: GLenum - Will get the additional GLenum that is used for retrieving the
//                      state variable value.
// Author:  AMD Developer Tools Team
// Date:        5/3/2007
// ---------------------------------------------------------------------------
GLenum apOpenGLStateVariablesManager::stateVariableAdditionalOpenGLParamEnum(int variableIndex) const
{
    GLenum retVal = GL_LIGHT0;

    // Sanity check:
    GT_IF_WITH_ASSERT(_stateVariablesData[variableIndex] != NULL)
    {
        retVal = _stateVariablesData[variableIndex]->_additionalOpenGLParamEnum;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::isEnumStateVariable
// Description:
//  Returns true iff the queried state variable is a GLenum state variable.
// Arguments: variableIndex - The queried state variable index.
// Return Val: bool - true iff the queries state variable is a GLenum state variable.
// Author:  AMD Developer Tools Team
// Date:        5/3/2007
// ---------------------------------------------------------------------------
bool apOpenGLStateVariablesManager::isEnumStateVariable(int variableIndex) const
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_stateVariablesData[variableIndex] != NULL)
    {
        retVal = _stateVariablesData[variableIndex]->_isEnumStateVariable;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::stateVariableRemovedVersion
// Description: Returns the version at which this state variable is removed (i.e.
//              the minimum version after which trying to get this variable would
//              create a GL error).
// Author:  AMD Developer Tools Team
// Date:        4/1/2010
// ---------------------------------------------------------------------------
apAPIVersion apOpenGLStateVariablesManager::stateVariableRemovedVersion(int variableIndex) const
{
    apAPIVersion retVal = AP_GL_VERSION_NONE;

    // Sanity check:
    GT_IF_WITH_ASSERT(_stateVariablesData[variableIndex] != NULL)
    {
        retVal = _stateVariablesData[variableIndex]->_removedVersion;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::setStateVariablesRemovedAtVersion
// Description: Set the version at which this variable is removed. Should only
//              be used by gsDeprecationAnalyzer.
// Author:  AMD Developer Tools Team
// Date:        4/1/2010
// ---------------------------------------------------------------------------
void apOpenGLStateVariablesManager::setStateVariablesRemovedAtVersion(int variableIndex, apAPIVersion removedVersion)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_stateVariablesData[variableIndex] != NULL)
    {
        _stateVariablesData[variableIndex]->_removedVersion = removedVersion;
    }
}

// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::initializeStateVariablesData
// Description: Initialize the data of the OpenGL state variables that we
//              currently monitor.
// Author:  AMD Developer Tools Team
// Date:        11/7/2004
// Implementation notes:
//  The following will not be treated as state variables:
//     a. Textures: variables that are get by functions: glGetTexImage, glGetTexParameter,
//        glGetTexGenfv, glGetTexGeniv and glGetTexLevelParameter.
//        - Textures will be displayed in a special view that shows the texture itself
//          (as a bitmap).
//
//     b. Evaluator maps: glGetMapXXX parameters, GL_MAP1_XX parameters.
//        - Evaluator maps will be displayed in a special view.
// ---------------------------------------------------------------------------
void apOpenGLStateVariablesManager::initializeStateVariablesData()
{
    // Initialize the state variables data to contain NULL pointers:
    for (int i = 0; i < apOpenGLStateVariablesAmount; i++)
    {
        _stateVariablesData[i] = NULL;
    }

    // ----------------------------------------------------
    //      OpenGL state variables:
    // ----------------------------------------------------

    _stateVariablesData[apGL_CURRENT_INDEX] = new StateVariableData(L"GL_CURRENT_INDEX", ap_glGetFloatv, GL_CURRENT_INDEX, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CURRENT_TEXTURE_COORDS] = new StateVariableData(L"GL_CURRENT_TEXTURE_COORDS", ap_glGetFloatv, 4, GL_CURRENT_TEXTURE_COORDS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_CURRENT_NORMAL] = new StateVariableData(L"GL_CURRENT_NORMAL", ap_glGetFloatv, 3, GL_CURRENT_NORMAL, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_CURRENT_RASTER_POSITION] = new StateVariableData(L"GL_CURRENT_RASTER_POSITION", ap_glGetFloatv, 4, GL_CURRENT_RASTER_POSITION, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CURRENT_RASTER_DISTANCE] = new StateVariableData(L"GL_CURRENT_RASTER_DISTANCE", ap_glGetFloatv, GL_CURRENT_RASTER_DISTANCE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CURRENT_RASTER_COLOR] = new StateVariableData(L"GL_CURRENT_RASTER_COLOR", ap_glGetFloatv, 4, GL_CURRENT_RASTER_COLOR, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CURRENT_RASTER_INDEX] = new StateVariableData(L"GL_CURRENT_RASTER_INDEX", ap_glGetFloatv, GL_CURRENT_RASTER_INDEX, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CURRENT_RASTER_TEXTURE_COORDS] = new StateVariableData(L"GL_CURRENT_RASTER_TEXTURE_COORDS", ap_glGetFloatv, 4, GL_CURRENT_RASTER_TEXTURE_COORDS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CURRENT_RASTER_POSITION_VALID] = new StateVariableData(L"GL_CURRENT_RASTER_POSITION_VALID", ap_glGetBooleanv, GL_CURRENT_RASTER_POSITION_VALID, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_EDGE_FLAG] = new StateVariableData(L"GL_EDGE_FLAG", ap_glGetBooleanv, GL_EDGE_FLAG, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW_MATRIX] = new StateVariableData(L"GL_MODELVIEW_MATRIX", ap_glGetFloatv, 4, 4, GL_MODELVIEW_MATRIX, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_PROJECTION_MATRIX] = new StateVariableData(L"GL_PROJECTION_MATRIX", ap_glGetFloatv, 4, 4, GL_PROJECTION_MATRIX, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_MATRIX] = new StateVariableData(L"GL_TEXTURE_MATRIX", ap_glGetFloatv, 4, 4, GL_TEXTURE_MATRIX, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_VIEWPORT] = new StateVariableData(L"GL_VIEWPORT", ap_glGetIntegerv, 4, GL_VIEWPORT, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_DEPTH_RANGE] = new StateVariableData(L"GL_DEPTH_RANGE", ap_glGetFloatv, 2, GL_DEPTH_RANGE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW_STACK_DEPTH] = new StateVariableData(L"GL_MODELVIEW_STACK_DEPTH", ap_glGetIntegerv, GL_MODELVIEW_STACK_DEPTH, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_PROJECTION_STACK_DEPTH] = new StateVariableData(L"GL_PROJECTION_STACK_DEPTH", ap_glGetIntegerv, GL_PROJECTION_STACK_DEPTH, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_STACK_DEPTH] = new StateVariableData(L"GL_TEXTURE_STACK_DEPTH", ap_glGetIntegerv, GL_TEXTURE_STACK_DEPTH, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_NORMALIZE] = new StateVariableData(L"GL_NORMALIZE", ap_glGetBooleanv, GL_NORMALIZE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHTING] = new StateVariableData(L"GL_LIGHTING", ap_glGetBooleanv, GL_LIGHTING, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_COLOR_MATERIAL] = new StateVariableData(L"GL_COLOR_MATERIAL", ap_glGetBooleanv, GL_COLOR_MATERIAL, 0, AP_OPENGL_STATE_VAR  | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_COLOR_MATERIAL_PARAMETER] = new StateVariableData(L"GL_COLOR_MATERIAL_PARAMETER", GL_COLOR_MATERIAL_PARAMETER, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_COLOR_MATERIAL_FACE] = new StateVariableData(L"GL_COLOR_MATERIAL_FACE", GL_COLOR_MATERIAL_FACE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_FOG_COLOR] = new StateVariableData(L"GL_FOG_COLOR", ap_glGetFloatv, 4, GL_FOG_COLOR, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_FOG_INDEX] = new StateVariableData(L"GL_FOG_INDEX", ap_glGetFloatv, GL_FOG_INDEX, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_FOG_DENSITY] = new StateVariableData(L"GL_FOG_DENSITY", ap_glGetFloatv, GL_FOG_DENSITY, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_FOG_START] = new StateVariableData(L"GL_FOG_START", ap_glGetFloatv, GL_FOG_START, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_FOG_END] = new StateVariableData(L"GL_FOG_END", ap_glGetFloatv, GL_FOG_END, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_FOG_MODE] = new StateVariableData(L"GL_FOG_MODE", GL_FOG_MODE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_FOG] = new StateVariableData(L"GL_FOG", ap_glGetBooleanv, GL_FOG, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT_MODEL_AMBIENT] = new StateVariableData(L"GL_LIGHT_MODEL_AMBIENT", ap_glGetFloatv, 4, GL_LIGHT_MODEL_AMBIENT, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT_MODEL_LOCAL_VIEWER] = new StateVariableData(L"GL_LIGHT_MODEL_LOCAL_VIEWER", ap_glGetBooleanv, GL_LIGHT_MODEL_LOCAL_VIEWER, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_LIGHT_MODEL_TWO_SIDE] = new StateVariableData(L"GL_LIGHT_MODEL_TWO_SIDE", ap_glGetBooleanv, GL_LIGHT_MODEL_TWO_SIDE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_POINT_SIZE] = new StateVariableData(L"GL_POINT_SIZE", ap_glGetFloatv, GL_POINT_SIZE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_POINT_SMOOTH] = new StateVariableData(L"GL_POINT_SMOOTH", ap_glGetBooleanv, GL_POINT_SMOOTH, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LINE_WIDTH] = new StateVariableData(L"GL_LINE_WIDTH", ap_glGetFloatv, GL_LINE_WIDTH, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_LINE_SMOOTH] = new StateVariableData(L"GL_LINE_SMOOTH", ap_glGetBooleanv, GL_LINE_SMOOTH, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LINE_STIPPLE_PATTERN] = new StateVariableData(L"GL_LINE_STIPPLE_PATTERN", ap_glGetIntegerv, GL_LINE_STIPPLE_PATTERN, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_LINE_STIPPLE_REPEAT] = new StateVariableData(L"GL_LINE_STIPPLE_REPEAT", ap_glGetIntegerv, GL_LINE_STIPPLE_REPEAT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_LINE_STIPPLE] = new StateVariableData(L"GL_LINE_STIPPLE", ap_glGetBooleanv, GL_LINE_STIPPLE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CULL_FACE] = new StateVariableData(L"GL_CULL_FACE", ap_glGetBooleanv, GL_CULL_FACE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_CULL_FACE_MODE] = new StateVariableData(L"GL_CULL_FACE_MODE", GL_CULL_FACE_MODE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_FRONT_FACE] = new StateVariableData(L"GL_FRONT_FACE", GL_FRONT_FACE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_POLYGON_SMOOTH] = new StateVariableData(L"GL_POLYGON_SMOOTH", ap_glGetBooleanv, GL_POLYGON_SMOOTH, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_POLYGON_MODE] = new StateVariableData(L"GL_POLYGON_MODE", GL_POLYGON_MODE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_POLYGON_STIPPLE] = new StateVariableData(L"GL_POLYGON_STIPPLE", ap_glGetBooleanv, GL_POLYGON_STIPPLE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_1D] = new StateVariableData(L"GL_TEXTURE_1D", ap_glGetBooleanv, GL_TEXTURE_1D, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_2D] = new StateVariableData(L"GL_TEXTURE_2D", ap_glGetBooleanv, GL_TEXTURE_2D, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_GEN_S] = new StateVariableData(L"GL_TEXTURE_GEN_S", ap_glGetBooleanv, GL_TEXTURE_GEN_S, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_GEN_T] = new StateVariableData(L"GL_TEXTURE_GEN_T", ap_glGetBooleanv, GL_TEXTURE_GEN_T, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_GEN_R] = new StateVariableData(L"GL_TEXTURE_GEN_R", ap_glGetBooleanv, GL_TEXTURE_GEN_R, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_GEN_Q] = new StateVariableData(L"GL_TEXTURE_GEN_Q", ap_glGetBooleanv, GL_TEXTURE_GEN_Q, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_GEN_MODE_s] = new StateVariableData(L"GL_TEXTURE_GEN_MODE - s", ap_glGetTexGeniv, GL_TEXTURE_GEN_MODE, GL_S, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_GEN_MODE_t] = new StateVariableData(L"GL_TEXTURE_GEN_MODE - t", ap_glGetTexGeniv, GL_TEXTURE_GEN_MODE, GL_T, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_GEN_MODE_r] = new StateVariableData(L"GL_TEXTURE_GEN_MODE - r", ap_glGetTexGeniv, GL_TEXTURE_GEN_MODE, GL_R, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_GEN_MODE_q] = new StateVariableData(L"GL_TEXTURE_GEN_MODE - q", ap_glGetTexGeniv, GL_TEXTURE_GEN_MODE, GL_Q, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_OBJECT_PLANE_s] = new StateVariableData(L"GL_OBJECT_PLANE - s", ap_glGetTexGenfv, GL_OBJECT_PLANE, GL_S, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_OBJECT_PLANE_t] = new StateVariableData(L"GL_OBJECT_PLANE - t", ap_glGetTexGenfv, GL_OBJECT_PLANE, GL_T, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_OBJECT_PLANE_r] = new StateVariableData(L"GL_OBJECT_PLANE - r", ap_glGetTexGenfv, GL_OBJECT_PLANE, GL_R, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_OBJECT_PLANE_q] = new StateVariableData(L"GL_OBJECT_PLANE - q", ap_glGetTexGenfv, GL_OBJECT_PLANE, GL_Q, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_EYE_PLANE_s] = new StateVariableData(L"GL_EYE_PLANE - s", ap_glGetTexGenfv, GL_EYE_PLANE, GL_S, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_EYE_PLANE_t] = new StateVariableData(L"GL_EYE_PLANE - t", ap_glGetTexGenfv, GL_EYE_PLANE, GL_T, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_EYE_PLANE_r] = new StateVariableData(L"GL_EYE_PLANE - r", ap_glGetTexGenfv, GL_EYE_PLANE, GL_R, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_EYE_PLANE_q] = new StateVariableData(L"GL_EYE_PLANE - q", ap_glGetTexGenfv, GL_EYE_PLANE, GL_Q, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_SCISSOR_TEST] = new StateVariableData(L"GL_SCISSOR_TEST", ap_glGetBooleanv, GL_SCISSOR_TEST, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_SCISSOR_BOX] = new StateVariableData(L"GL_SCISSOR_BOX", ap_glGetIntegerv, 4, GL_SCISSOR_BOX, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_STENCIL_TEST] = new StateVariableData(L"GL_STENCIL_TEST", ap_glGetBooleanv, GL_STENCIL_TEST, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_STENCIL_FUNC] = new StateVariableData(L"GL_STENCIL_FUNC", GL_STENCIL_FUNC, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_STENCIL_VALUE_MASK] = new StateVariableData(L"GL_STENCIL_VALUE_MASK", ap_glGetIntegerv, GL_STENCIL_VALUE_MASK, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_STENCIL_REF] = new StateVariableData(L"GL_STENCIL_REF", ap_glGetIntegerv, GL_STENCIL_REF, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_STENCIL_FAIL] = new StateVariableData(L"GL_STENCIL_FAIL", GL_STENCIL_FAIL, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_STENCIL_PASS_DEPTH_FAIL] = new StateVariableData(L"GL_STENCIL_PASS_DEPTH_FAIL", GL_STENCIL_PASS_DEPTH_FAIL, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_STENCIL_PASS_DEPTH_PASS] = new StateVariableData(L"GL_STENCIL_PASS_DEPTH_PASS", GL_STENCIL_PASS_DEPTH_PASS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_ALPHA_TEST] = new StateVariableData(L"GL_ALPHA_TEST", ap_glGetBooleanv, GL_ALPHA_TEST, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_ALPHA_TEST_FUNC] = new StateVariableData(L"GL_ALPHA_TEST_FUNC", GL_ALPHA_TEST_FUNC, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_ALPHA_TEST_REF] = new StateVariableData(L"GL_ALPHA_TEST_REF", ap_glGetFloatv, GL_ALPHA_TEST_REF, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_DEPTH_TEST] = new StateVariableData(L"GL_DEPTH_TEST", ap_glGetBooleanv, GL_DEPTH_TEST, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_DEPTH_FUNC] = new StateVariableData(L"GL_DEPTH_FUNC", GL_DEPTH_FUNC, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_BLEND] = new StateVariableData(L"GL_BLEND", ap_glGetBooleanv, GL_BLEND, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_BLEND_SRC] = new StateVariableData(L"GL_BLEND_SRC", GL_BLEND_SRC, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_BLEND_DST] = new StateVariableData(L"GL_BLEND_DST", GL_BLEND_DST, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LOGIC_OP] = new StateVariableData(L"GL_LOGIC_OP", ap_glGetBooleanv, GL_LOGIC_OP, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_LOGIC_OP_MODE] = new StateVariableData(L"GL_LOGIC_OP_MODE", GL_LOGIC_OP_MODE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_DITHER] = new StateVariableData(L"GL_DITHER", ap_glGetBooleanv, GL_DITHER, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER] = new StateVariableData(L"GL_DRAW_BUFFER", GL_DRAW_BUFFER, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_INDEX_WRITEMASK] = new StateVariableData(L"GL_INDEX_WRITEMASK", ap_glGetIntegerv, GL_INDEX_WRITEMASK, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_COLOR_WRITEMASK] = new StateVariableData(L"GL_COLOR_WRITEMASK", ap_glGetBooleanv, 4, GL_COLOR_WRITEMASK, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_DEPTH_WRITEMASK] = new StateVariableData(L"GL_DEPTH_WRITEMASK", ap_glGetBooleanv, GL_DEPTH_WRITEMASK, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_STENCIL_WRITEMASK] = new StateVariableData(L"GL_STENCIL_WRITEMASK", ap_glGetIntegerv, GL_STENCIL_WRITEMASK, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_COLOR_CLEAR_VALUE] = new StateVariableData(L"GL_COLOR_CLEAR_VALUE", ap_glGetFloatv, 4, GL_COLOR_CLEAR_VALUE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_INDEX_CLEAR_VALUE] = new StateVariableData(L"GL_INDEX_CLEAR_VALUE", ap_glGetFloatv, GL_INDEX_CLEAR_VALUE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DEPTH_CLEAR_VALUE] = new StateVariableData(L"GL_DEPTH_CLEAR_VALUE", ap_glGetIntegerv, GL_DEPTH_CLEAR_VALUE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_STENCIL_CLEAR_VALUE] = new StateVariableData(L"GL_STENCIL_CLEAR_VALUE", ap_glGetIntegerv, GL_STENCIL_CLEAR_VALUE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_ACCUM_CLEAR_VALUE] = new StateVariableData(L"GL_ACCUM_CLEAR_VALUE", ap_glGetFloatv, GL_ACCUM_CLEAR_VALUE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_UNPACK_SWAP_BYTES] = new StateVariableData(L"GL_UNPACK_SWAP_BYTES", ap_glGetBooleanv, GL_UNPACK_SWAP_BYTES, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_UNPACK_LSB_FIRST] = new StateVariableData(L"GL_UNPACK_LSB_FIRST", ap_glGetBooleanv, GL_UNPACK_LSB_FIRST, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_UNPACK_ROW_LENGTH] = new StateVariableData(L"GL_UNPACK_ROW_LENGTH", ap_glGetIntegerv, GL_UNPACK_ROW_LENGTH, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_UNPACK_SKIP_ROWS] = new StateVariableData(L"GL_UNPACK_SKIP_ROWS", ap_glGetIntegerv, GL_UNPACK_SKIP_ROWS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_UNPACK_SKIP_PIXELS] = new StateVariableData(L"GL_UNPACK_SKIP_PIXELS", ap_glGetIntegerv, GL_UNPACK_SKIP_PIXELS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_UNPACK_ALIGNMENT] = new StateVariableData(L"GL_UNPACK_ALIGNMENT", ap_glGetIntegerv, GL_UNPACK_ALIGNMENT, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_PACK_SWAP_BYTES] = new StateVariableData(L"GL_PACK_SWAP_BYTES", ap_glGetBooleanv, GL_PACK_SWAP_BYTES, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PACK_LSB_FIRST] = new StateVariableData(L"GL_PACK_LSB_FIRST", ap_glGetBooleanv, GL_PACK_LSB_FIRST, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PACK_ROW_LENGTH] = new StateVariableData(L"GL_PACK_ROW_LENGTH", ap_glGetIntegerv, GL_PACK_ROW_LENGTH, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PACK_SKIP_ROWS] = new StateVariableData(L"GL_PACK_SKIP_ROWS", ap_glGetIntegerv, GL_PACK_SKIP_ROWS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PACK_SKIP_PIXELS] = new StateVariableData(L"GL_PACK_SKIP_PIXELS", ap_glGetIntegerv, GL_PACK_SKIP_PIXELS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PACK_ALIGNMENT] = new StateVariableData(L"GL_PACK_ALIGNMENT", ap_glGetIntegerv, GL_PACK_ALIGNMENT, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_MAP_COLOR] = new StateVariableData(L"GL_MAP_COLOR", ap_glGetBooleanv, GL_MAP_COLOR, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAP_STENCIL] = new StateVariableData(L"GL_MAP_STENCIL", ap_glGetBooleanv, GL_MAP_STENCIL, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_INDEX_SHIFT] = new StateVariableData(L"GL_INDEX_SHIFT", ap_glGetIntegerv, GL_INDEX_SHIFT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_INDEX_OFFSET] = new StateVariableData(L"GL_INDEX_OFFSET", ap_glGetIntegerv, GL_INDEX_OFFSET, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_RED_SCALE] = new StateVariableData(L"GL_RED_SCALE", ap_glGetFloatv, GL_RED_SCALE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_GREEN_SCALE] = new StateVariableData(L"GL_GREEN_SCALE", ap_glGetFloatv, GL_GREEN_SCALE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_BLUE_SCALE] = new StateVariableData(L"GL_BLUE_SCALE", ap_glGetFloatv, GL_BLUE_SCALE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ALPHA_SCALE] = new StateVariableData(L"GL_ALPHA_SCALE", ap_glGetFloatv, GL_ALPHA_SCALE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DEPTH_SCALE] = new StateVariableData(L"GL_DEPTH_SCALE", ap_glGetFloatv, GL_DEPTH_SCALE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_RED_BIAS] = new StateVariableData(L"GL_RED_BIAS", ap_glGetFloatv, GL_RED_BIAS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_GREEN_BIAS] = new StateVariableData(L"GL_GREEN_BIAS", ap_glGetFloatv, GL_GREEN_BIAS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_BLUE_BIAS] = new StateVariableData(L"GL_BLUE_BIAS", ap_glGetFloatv, GL_BLUE_BIAS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ALPHA_BIAS] = new StateVariableData(L"GL_ALPHA_BIAS", ap_glGetFloatv, GL_ALPHA_BIAS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DEPTH_BIAS] = new StateVariableData(L"GL_DEPTH_BIAS", ap_glGetFloatv, GL_DEPTH_BIAS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ZOOM_X] = new StateVariableData(L"GL_ZOOM_X", ap_glGetFloatv, GL_ZOOM_X, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ZOOM_Y] = new StateVariableData(L"GL_ZOOM_Y", ap_glGetFloatv, GL_ZOOM_Y, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_READ_BUFFER] = new StateVariableData(L"GL_READ_BUFFER", GL_READ_BUFFER, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAP1_GRID_SEGMENTS] = new StateVariableData(L"GL_MAP1_GRID_SEGMENTS", ap_glGetFloatv, GL_MAP1_GRID_SEGMENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAP2_GRID_SEGMENTS] = new StateVariableData(L"GL_MAP2_GRID_SEGMENTS", ap_glGetFloatv, 2, GL_MAP2_GRID_SEGMENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_AUTO_NORMAL] = new StateVariableData(L"GL_AUTO_NORMAL", ap_glGetBooleanv, GL_AUTO_NORMAL, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PERSPECTIVE_CORRECTION_HINT] = new StateVariableData(L"GL_PERSPECTIVE_CORRECTION_HINT", GL_PERSPECTIVE_CORRECTION_HINT, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_POINT_SMOOTH_HINT] = new StateVariableData(L"GL_POINT_SMOOTH_HINT", GL_POINT_SMOOTH_HINT, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LINE_SMOOTH_HINT] = new StateVariableData(L"GL_LINE_SMOOTH_HINT", GL_LINE_SMOOTH_HINT, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_POLYGON_SMOOTH_HINT] = new StateVariableData(L"GL_POLYGON_SMOOTH_HINT", GL_POLYGON_SMOOTH_HINT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_FOG_HINT] = new StateVariableData(L"GL_FOG_HINT", GL_FOG_HINT, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_MAX_LIGHTS] = new StateVariableData(L"GL_MAX_LIGHTS", ap_glGetIntegerv, GL_MAX_LIGHTS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_MAX_CLIP_PLANES] = new StateVariableData(L"GL_MAX_CLIP_PLANES", ap_glGetIntegerv, GL_MAX_CLIP_PLANES, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_MAX_MODELVIEW_STACK_DEPTH] = new StateVariableData(L"GL_MAX_MODELVIEW_STACK_DEPTH", ap_glGetIntegerv, GL_MAX_MODELVIEW_STACK_DEPTH, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROJECTION_STACK_DEPTH] = new StateVariableData(L"GL_MAX_PROJECTION_STACK_DEPTH", ap_glGetIntegerv, GL_MAX_PROJECTION_STACK_DEPTH, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_MAX_TEXTURE_STACK_DEPTH] = new StateVariableData(L"GL_MAX_TEXTURE_STACK_DEPTH", ap_glGetIntegerv, GL_MAX_TEXTURE_STACK_DEPTH, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_SUBPIXEL_BITS] = new StateVariableData(L"GL_SUBPIXEL_BITS", ap_glGetIntegerv, GL_SUBPIXEL_BITS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_MAX_TEXTURE_SIZE] = new StateVariableData(L"GL_MAX_TEXTURE_SIZE", ap_glGetIntegerv, GL_MAX_TEXTURE_SIZE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_MAX_PIXEL_MAP_TABLE] = new StateVariableData(L"GL_MAX_PIXEL_MAP_TABLE", ap_glGetIntegerv, GL_MAX_PIXEL_MAP_TABLE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_NAME_STACK_DEPTH] = new StateVariableData(L"GL_MAX_NAME_STACK_DEPTH", ap_glGetIntegerv, GL_MAX_NAME_STACK_DEPTH, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_LIST_NESTING] = new StateVariableData(L"GL_MAX_LIST_NESTING", ap_glGetIntegerv, GL_MAX_LIST_NESTING, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_EVAL_ORDER] = new StateVariableData(L"GL_MAX_EVAL_ORDER", ap_glGetIntegerv, GL_MAX_EVAL_ORDER, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VIEWPORT_DIMS] = new StateVariableData(L"GL_MAX_VIEWPORT_DIMS", ap_glGetIntegerv, GL_MAX_VIEWPORT_DIMS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_MAX_ATTRIB_STACK_DEPTH] = new StateVariableData(L"GL_MAX_ATTRIB_STACK_DEPTH", ap_glGetIntegerv, GL_MAX_ATTRIB_STACK_DEPTH, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_AUX_BUFFERS] = new StateVariableData(L"GL_AUX_BUFFERS", ap_glGetIntegerv, GL_AUX_BUFFERS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_RGBA_MODE] = new StateVariableData(L"GL_RGBA_MODE", ap_glGetBooleanv, GL_RGBA_MODE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_INDEX_MODE] = new StateVariableData(L"GL_INDEX_MODE", ap_glGetBooleanv, GL_INDEX_MODE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DOUBLEBUFFER] = new StateVariableData(L"GL_DOUBLEBUFFER", ap_glGetBooleanv, GL_DOUBLEBUFFER, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_STEREO] = new StateVariableData(L"GL_STEREO", ap_glGetFloatv, GL_STEREO, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_POINT_SIZE_RANGE] = new StateVariableData(L"GL_POINT_SIZE_RANGE", ap_glGetFloatv, GL_POINT_SIZE_RANGE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_POINT_SIZE_GRANULARITY] = new StateVariableData(L"GL_POINT_SIZE_GRANULARITY", ap_glGetFloatv, GL_POINT_SIZE_GRANULARITY, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_LINE_WIDTH_RANGE] = new StateVariableData(L"GL_LINE_WIDTH_RANGE", ap_glGetFloatv, GL_LINE_WIDTH_RANGE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_LINE_WIDTH_GRANULARITY] = new StateVariableData(L"GL_LINE_WIDTH_GRANULARITY", ap_glGetFloatv, GL_LINE_WIDTH_GRANULARITY, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_RED_BITS] = new StateVariableData(L"GL_RED_BITS", ap_glGetIntegerv, GL_RED_BITS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_GREEN_BITS] = new StateVariableData(L"GL_GREEN_BITS", ap_glGetIntegerv, GL_GREEN_BITS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_BLUE_BITS] = new StateVariableData(L"GL_BLUE_BITS", ap_glGetIntegerv, GL_BLUE_BITS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_ALPHA_BITS] = new StateVariableData(L"GL_ALPHA_BITS", ap_glGetIntegerv, GL_ALPHA_BITS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_INDEX_BITS] = new StateVariableData(L"GL_INDEX_BITS", ap_glGetIntegerv, GL_INDEX_BITS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DEPTH_BITS] = new StateVariableData(L"GL_DEPTH_BITS", ap_glGetIntegerv, GL_DEPTH_BITS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_STENCIL_BITS] = new StateVariableData(L"GL_STENCIL_BITS", ap_glGetIntegerv, GL_STENCIL_BITS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_ACCUM_RED_BITS] = new StateVariableData(L"GL_ACCUM_RED_BITS", ap_glGetIntegerv, GL_ACCUM_RED_BITS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ACCUM_GREEN_BITS] = new StateVariableData(L"GL_ACCUM_GREEN_BITS", ap_glGetIntegerv, GL_ACCUM_GREEN_BITS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ACCUM_BLUE_BITS] = new StateVariableData(L"GL_ACCUM_BLUE_BITS", ap_glGetIntegerv, GL_ACCUM_BLUE_BITS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ACCUM_ALPHA_BITS] = new StateVariableData(L"GL_ACCUM_ALPHA_BITS", ap_glGetIntegerv, GL_ACCUM_ALPHA_BITS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_LIST_BASE] = new StateVariableData(L"GL_LIST_BASE", ap_glGetIntegerv, GL_LIST_BASE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_LIST_INDEX] = new StateVariableData(L"GL_LIST_INDEX", ap_glGetIntegerv, GL_LIST_INDEX, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_LIST_MODE] = new StateVariableData(L"GL_LIST_MODE", GL_LIST_MODE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ATTRIB_STACK_DEPTH] = new StateVariableData(L"GL_ATTRIB_STACK_DEPTH", ap_glGetIntegerv, GL_ATTRIB_STACK_DEPTH, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_NAME_STACK_DEPTH] = new StateVariableData(L"GL_NAME_STACK_DEPTH", ap_glGetIntegerv, GL_NAME_STACK_DEPTH, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_RENDER_MODE] = new StateVariableData(L"GL_RENDER_MODE", GL_RENDER_MODE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_AMBIENT_back] = new StateVariableData(L"GL_AMBIENT - back", ap_glGetMaterialfv, 4, GL_AMBIENT, GL_BACK, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_AMBIENT_front] = new StateVariableData(L"GL_AMBIENT - front", ap_glGetMaterialfv, 4, GL_AMBIENT, GL_FRONT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_DIFFUSE_back] = new StateVariableData(L"GL_DIFFUSE - back", ap_glGetMaterialfv, 4, GL_DIFFUSE, GL_BACK, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_DIFFUSE_front] = new StateVariableData(L"GL_DIFFUSE - front", ap_glGetMaterialfv, 4, GL_DIFFUSE, GL_FRONT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_SPECULAR_back] = new StateVariableData(L"GL_SPECULAR - back", ap_glGetMaterialfv, 4, GL_SPECULAR, GL_BACK, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_SPECULAR_front] = new StateVariableData(L"GL_SPECULAR - front", ap_glGetMaterialfv, 4, GL_SPECULAR, GL_FRONT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_EMISSION_back] = new StateVariableData(L"GL_EMISSION - back", ap_glGetMaterialfv, 4, GL_EMISSION, GL_BACK, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_EMISSION_front] = new StateVariableData(L"GL_EMISSION - front", ap_glGetMaterialfv, 4, GL_EMISSION, GL_FRONT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_SHININESS_back] = new StateVariableData(L"GL_SHININESS - back", ap_glGetMaterialfv, GL_SHININESS, GL_BACK, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_SHININESS_front] = new StateVariableData(L"GL_SHININESS - front", ap_glGetMaterialfv, GL_SHININESS, GL_FRONT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_COLOR_INDEXES_back] = new StateVariableData(L"GL_COLOR_INDEXES - back", ap_glGetMaterialfv, 3, GL_COLOR_INDEXES, GL_BACK, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_COLOR_INDEXES_front] = new StateVariableData(L"GL_COLOR_INDEXES - front", ap_glGetMaterialfv, 3, GL_COLOR_INDEXES, GL_FRONT, AP_OPENGL_STATE_VAR);

    // Light sources:
    _stateVariablesData[apGL_LIGHT0] = new StateVariableData(L"GL_LIGHT0", ap_glGetBooleanv, GL_LIGHT0, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT0_ambient] = new StateVariableData(L"GL_LIGHT0 - ambient", ap_glGetLightfv, 4, GL_LIGHT0, GL_AMBIENT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT0_diffuse] = new StateVariableData(L"GL_LIGHT0 - diffuse", ap_glGetLightfv, 4, GL_LIGHT0, GL_DIFFUSE, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT0_specular] = new StateVariableData(L"GL_LIGHT0 - specular", ap_glGetLightfv, 4, GL_LIGHT0, GL_SPECULAR, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT0_position] = new StateVariableData(L"GL_LIGHT0 - position", ap_glGetLightfv, 4, GL_LIGHT0, GL_POSITION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT0_spot_direction] = new StateVariableData(L"GL_LIGHT0 - spot direction", ap_glGetLightfv, 3, GL_LIGHT0, GL_SPOT_DIRECTION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT0_spot_exponent] = new StateVariableData(L"GL_LIGHT0 - spot exponent", ap_glGetLightfv, GL_LIGHT0, GL_SPOT_EXPONENT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT0_spot_cutoff] = new StateVariableData(L"GL_LIGHT0 - spot cutoff", ap_glGetLightfv, GL_LIGHT0, GL_SPOT_CUTOFF, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT0_constant_attenuation] = new StateVariableData(L"GL_LIGHT0 - constant attenuation", ap_glGetLightfv, GL_LIGHT0, GL_CONSTANT_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT0_linear_attenuation] = new StateVariableData(L"GL_LIGHT0 - linear attenuation", ap_glGetLightfv, GL_LIGHT0, GL_LINEAR_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT0_quadratic_attenuation] = new StateVariableData(L"GL_LIGHT0 - quadratic attenuation", ap_glGetLightfv, GL_LIGHT0, GL_QUADRATIC_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);

    _stateVariablesData[apGL_LIGHT1] = new StateVariableData(L"GL_LIGHT1", ap_glGetBooleanv, GL_LIGHT1, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT1_ambient] = new StateVariableData(L"GL_LIGHT1 - ambient", ap_glGetLightfv, 4, GL_LIGHT1, GL_AMBIENT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT1_diffuse] = new StateVariableData(L"GL_LIGHT1 - diffuse", ap_glGetLightfv, 4, GL_LIGHT1, GL_DIFFUSE, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT1_specular] = new StateVariableData(L"GL_LIGHT1 - specular", ap_glGetLightfv, 4, GL_LIGHT1, GL_SPECULAR, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT1_position] = new StateVariableData(L"GL_LIGHT1 - position", ap_glGetLightfv, 4, GL_LIGHT1, GL_POSITION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT1_spot_direction] = new StateVariableData(L"GL_LIGHT1 - spot direction", ap_glGetLightfv, 3, GL_LIGHT1, GL_SPOT_DIRECTION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT1_spot_exponent] = new StateVariableData(L"GL_LIGHT1 - spot exponent", ap_glGetLightfv, GL_LIGHT1, GL_SPOT_EXPONENT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT1_spot_cutoff] = new StateVariableData(L"GL_LIGHT1 - spot cutoff", ap_glGetLightfv, GL_LIGHT1, GL_SPOT_CUTOFF, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT1_constant_attenuation] = new StateVariableData(L"GL_LIGHT1 - constant attenuation", ap_glGetLightfv, GL_LIGHT1, GL_CONSTANT_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT1_linear_attenuation] = new StateVariableData(L"GL_LIGHT1 - linear attenuation", ap_glGetLightfv, GL_LIGHT1, GL_LINEAR_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT1_quadratic_attenuation] = new StateVariableData(L"GL_LIGHT1 - quadratic attenuation", ap_glGetLightfv, GL_LIGHT1, GL_QUADRATIC_ATTENUATION, AP_OPENGL_STATE_VAR);

    _stateVariablesData[apGL_LIGHT2] = new StateVariableData(L"GL_LIGHT2", ap_glGetBooleanv, GL_LIGHT2, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT2_ambient] = new StateVariableData(L"GL_LIGHT2 - ambient", ap_glGetLightfv, 4, GL_LIGHT2, GL_AMBIENT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT2_diffuse] = new StateVariableData(L"GL_LIGHT2 - diffuse", ap_glGetLightfv, 4, GL_LIGHT2, GL_DIFFUSE, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT2_specular] = new StateVariableData(L"GL_LIGHT2 - specular", ap_glGetLightfv, 4, GL_LIGHT2, GL_SPECULAR, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT2_position] = new StateVariableData(L"GL_LIGHT2 - position", ap_glGetLightfv, 4, GL_LIGHT2, GL_POSITION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT2_spot_direction] = new StateVariableData(L"GL_LIGHT2 - spot direction", ap_glGetLightfv, 3, GL_LIGHT2, GL_SPOT_DIRECTION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT2_spot_exponent] = new StateVariableData(L"GL_LIGHT2 - spot exponent", ap_glGetLightfv, GL_LIGHT2, GL_SPOT_EXPONENT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT2_spot_cutoff] = new StateVariableData(L"GL_LIGHT2 - spot cutoff", ap_glGetLightfv, GL_LIGHT2, GL_SPOT_CUTOFF, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT2_constant_attenuation] = new StateVariableData(L"GL_LIGHT2 - constant attenuation", ap_glGetLightfv, GL_LIGHT2, GL_CONSTANT_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT2_linear_attenuation] = new StateVariableData(L"GL_LIGHT2 - linear attenuation", ap_glGetLightfv, GL_LIGHT2, GL_LINEAR_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT2_quadratic_attenuation] = new StateVariableData(L"GL_LIGHT2 - quadratic attenuation", ap_glGetLightfv, GL_LIGHT2, GL_QUADRATIC_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);

    _stateVariablesData[apGL_LIGHT3] = new StateVariableData(L"GL_LIGHT3", ap_glGetBooleanv, GL_LIGHT3, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT3_ambient] = new StateVariableData(L"GL_LIGHT3 - ambient", ap_glGetLightfv, 4, GL_LIGHT3, GL_AMBIENT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT3_diffuse] = new StateVariableData(L"GL_LIGHT3 - diffuse", ap_glGetLightfv, 4, GL_LIGHT3, GL_DIFFUSE, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT3_specular] = new StateVariableData(L"GL_LIGHT3 - specular", ap_glGetLightfv, 4, GL_LIGHT3, GL_SPECULAR, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT3_position] = new StateVariableData(L"GL_LIGHT3 - position", ap_glGetLightfv, 4, GL_LIGHT3, GL_POSITION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT3_spot_direction] = new StateVariableData(L"GL_LIGHT3 - spot direction", ap_glGetLightfv, 3, GL_LIGHT3, GL_SPOT_DIRECTION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT3_spot_exponent] = new StateVariableData(L"GL_LIGHT3 - spot exponent", ap_glGetLightfv, GL_LIGHT3, GL_SPOT_EXPONENT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT3_spot_cutoff] = new StateVariableData(L"GL_LIGHT3 - spot cutoff", ap_glGetLightfv, GL_LIGHT3, GL_SPOT_CUTOFF, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT3_constant_attenuation] = new StateVariableData(L"GL_LIGHT3 - constant attenuation", ap_glGetLightfv, GL_LIGHT3, GL_CONSTANT_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT3_linear_attenuation] = new StateVariableData(L"GL_LIGHT3 - linear attenuation", ap_glGetLightfv, GL_LIGHT3, GL_LINEAR_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT3_quadratic_attenuation] = new StateVariableData(L"GL_LIGHT3 - quadratic attenuation", ap_glGetLightfv, GL_LIGHT3, GL_QUADRATIC_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);

    _stateVariablesData[apGL_LIGHT4] = new StateVariableData(L"GL_LIGHT4", ap_glGetBooleanv, GL_LIGHT4, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT4_ambient] = new StateVariableData(L"GL_LIGHT4 - ambient", ap_glGetLightfv, 4, GL_LIGHT4, GL_AMBIENT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT4_diffuse] = new StateVariableData(L"GL_LIGHT4 - diffuse", ap_glGetLightfv, 4, GL_LIGHT4, GL_DIFFUSE, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT4_specular] = new StateVariableData(L"GL_LIGHT4 - specular", ap_glGetLightfv, 4, GL_LIGHT4, GL_SPECULAR, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT4_position] = new StateVariableData(L"GL_LIGHT4 - position", ap_glGetLightfv, 4, GL_LIGHT4, GL_POSITION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT4_spot_direction] = new StateVariableData(L"GL_LIGHT4 - spot direction", ap_glGetLightfv, 3, GL_LIGHT4, GL_SPOT_DIRECTION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT4_spot_exponent] = new StateVariableData(L"GL_LIGHT4 - spot exponent", ap_glGetLightfv, GL_LIGHT4, GL_SPOT_EXPONENT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT4_spot_cutoff] = new StateVariableData(L"GL_LIGHT4 - spot cutoff", ap_glGetLightfv, GL_LIGHT4, GL_SPOT_CUTOFF, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT4_constant_attenuation] = new StateVariableData(L"GL_LIGHT4 - constant attenuation", ap_glGetLightfv, GL_LIGHT4, GL_CONSTANT_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT4_linear_attenuation] = new StateVariableData(L"GL_LIGHT4 - linear attenuation", ap_glGetLightfv, GL_LIGHT4, GL_LINEAR_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT4_quadratic_attenuation] = new StateVariableData(L"GL_LIGHT4 - quadratic attenuation", ap_glGetLightfv, GL_LIGHT4, GL_QUADRATIC_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);

    _stateVariablesData[apGL_LIGHT5] = new StateVariableData(L"GL_LIGHT5", ap_glGetBooleanv, GL_LIGHT5, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT5_ambient] = new StateVariableData(L"GL_LIGHT5 - ambient", ap_glGetLightfv, 4, GL_LIGHT5, GL_AMBIENT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT5_diffuse] = new StateVariableData(L"GL_LIGHT5 - diffuse", ap_glGetLightfv, 4, GL_LIGHT5, GL_DIFFUSE, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT5_specular] = new StateVariableData(L"GL_LIGHT5 - specular", ap_glGetLightfv, 4, GL_LIGHT5, GL_SPECULAR, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT5_position] = new StateVariableData(L"GL_LIGHT5 - position", ap_glGetLightfv, 4, GL_LIGHT5, GL_POSITION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT5_spot_direction] = new StateVariableData(L"GL_LIGHT5 - spot direction", ap_glGetLightfv, 3, GL_LIGHT5, GL_SPOT_DIRECTION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT5_spot_exponent] = new StateVariableData(L"GL_LIGHT5 - spot exponent", ap_glGetLightfv, GL_LIGHT5, GL_SPOT_EXPONENT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT5_spot_cutoff] = new StateVariableData(L"GL_LIGHT5 - spot cutoff", ap_glGetLightfv, GL_LIGHT5, GL_SPOT_CUTOFF, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT5_constant_attenuation] = new StateVariableData(L"GL_LIGHT5 - constant attenuation", ap_glGetLightfv, GL_LIGHT5, GL_CONSTANT_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT5_linear_attenuation] = new StateVariableData(L"GL_LIGHT5 - linear attenuation", ap_glGetLightfv, GL_LIGHT5, GL_LINEAR_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT5_quadratic_attenuation] = new StateVariableData(L"GL_LIGHT5 - quadratic attenuation", ap_glGetLightfv, GL_LIGHT5, GL_QUADRATIC_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);

    _stateVariablesData[apGL_LIGHT6] = new StateVariableData(L"GL_LIGHT6", ap_glGetBooleanv, GL_LIGHT6, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT6_ambient] = new StateVariableData(L"GL_LIGHT6 - ambient", ap_glGetLightfv, 4, GL_LIGHT6, GL_AMBIENT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT6_diffuse] = new StateVariableData(L"GL_LIGHT6 - diffuse", ap_glGetLightfv, 4, GL_LIGHT6, GL_DIFFUSE, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT6_specular] = new StateVariableData(L"GL_LIGHT6 - specular", ap_glGetLightfv, 4, GL_LIGHT6, GL_SPECULAR, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT6_position] = new StateVariableData(L"GL_LIGHT6 - position", ap_glGetLightfv, 4, GL_LIGHT6, GL_POSITION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT6_spot_direction] = new StateVariableData(L"GL_LIGHT6 - spot direction", ap_glGetLightfv, 3, GL_LIGHT6, GL_SPOT_DIRECTION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT6_spot_exponent] = new StateVariableData(L"GL_LIGHT6 - spot exponent", ap_glGetLightfv, GL_LIGHT6, GL_SPOT_EXPONENT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT6_spot_cutoff] = new StateVariableData(L"GL_LIGHT6 - spot cutoff", ap_glGetLightfv, GL_LIGHT6, GL_SPOT_CUTOFF, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT6_constant_attenuation] = new StateVariableData(L"GL_LIGHT6 - constant attenuation", ap_glGetLightfv, GL_LIGHT6, GL_CONSTANT_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT6_linear_attenuation] = new StateVariableData(L"GL_LIGHT6 - linear attenuation", ap_glGetLightfv, GL_LIGHT6, GL_LINEAR_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT6_quadratic_attenuation] = new StateVariableData(L"GL_LIGHT6 - quadratic attenuation", ap_glGetLightfv, GL_LIGHT6, GL_QUADRATIC_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);

    _stateVariablesData[apGL_LIGHT7] = new StateVariableData(L"GL_LIGHT7", ap_glGetBooleanv, GL_LIGHT7, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT7_ambient] = new StateVariableData(L"GL_LIGHT7 - ambient", ap_glGetLightfv, 4, GL_LIGHT7, GL_AMBIENT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT7_diffuse] = new StateVariableData(L"GL_LIGHT7 - diffuse", ap_glGetLightfv, 4, GL_LIGHT7, GL_DIFFUSE, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT7_specular] = new StateVariableData(L"GL_LIGHT7 - specular", ap_glGetLightfv, 4, GL_LIGHT7, GL_SPECULAR, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT7_position] = new StateVariableData(L"GL_LIGHT7 - position", ap_glGetLightfv, 4, GL_LIGHT7, GL_POSITION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT7_spot_direction] = new StateVariableData(L"GL_LIGHT7 - spot direction", ap_glGetLightfv, 3, GL_LIGHT7, GL_SPOT_DIRECTION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT7_spot_exponent] = new StateVariableData(L"GL_LIGHT7 - spot exponent", ap_glGetLightfv, GL_LIGHT7, GL_SPOT_EXPONENT, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT7_spot_cutoff] = new StateVariableData(L"GL_LIGHT7 - spot cutoff", ap_glGetLightfv, GL_LIGHT7, GL_SPOT_CUTOFF, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT7_constant_attenuation] = new StateVariableData(L"GL_LIGHT7 - constant attenuation", ap_glGetLightfv, GL_LIGHT7, GL_CONSTANT_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT7_linear_attenuation] = new StateVariableData(L"GL_LIGHT7 - linear attenuation", ap_glGetLightfv, GL_LIGHT7, GL_LINEAR_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT7_quadratic_attenuation] = new StateVariableData(L"GL_LIGHT7 - quadratic attenuation", ap_glGetLightfv, GL_LIGHT7, GL_QUADRATIC_ATTENUATION, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);

    // Clip planes:
    _stateVariablesData[apGL_CLIP_PLANE0] = new StateVariableData(L"GL_CLIP_PLANE0", ap_glGetBooleanv, GL_CLIP_PLANE0, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_CLIP_PLANE1] = new StateVariableData(L"GL_CLIP_PLANE1", ap_glGetBooleanv, GL_CLIP_PLANE1, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_CLIP_PLANE2] = new StateVariableData(L"GL_CLIP_PLANE2", ap_glGetBooleanv, GL_CLIP_PLANE2, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_CLIP_PLANE3] = new StateVariableData(L"GL_CLIP_PLANE3", ap_glGetBooleanv, GL_CLIP_PLANE3, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_CLIP_PLANE4] = new StateVariableData(L"GL_CLIP_PLANE4", ap_glGetBooleanv, GL_CLIP_PLANE4, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_CLIP_PLANE5] = new StateVariableData(L"GL_CLIP_PLANE5", ap_glGetBooleanv, GL_CLIP_PLANE5, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);

    _stateVariablesData[apGL_CLIP_PLANE0_equation] = new StateVariableData(L"GL_CLIP_PLANE0 - equation", ap_glGetClipPlane, 4, GL_CLIP_PLANE0, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_CLIP_PLANE1_equation] = new StateVariableData(L"GL_CLIP_PLANE1 - equation", ap_glGetClipPlane, 4, GL_CLIP_PLANE1, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_CLIP_PLANE2_equation] = new StateVariableData(L"GL_CLIP_PLANE2 - equation", ap_glGetClipPlane, 4, GL_CLIP_PLANE2, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_CLIP_PLANE3_equation] = new StateVariableData(L"GL_CLIP_PLANE3 - equation", ap_glGetClipPlane, 4, GL_CLIP_PLANE3, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_CLIP_PLANE4_equation] = new StateVariableData(L"GL_CLIP_PLANE4 - equation", ap_glGetClipPlane, 4, GL_CLIP_PLANE4, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_CLIP_PLANE5_equation] = new StateVariableData(L"GL_CLIP_PLANE5 - equation", ap_glGetClipPlane, 4, GL_CLIP_PLANE5, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);

    // Pixel transfer map sizes (See glPixelMapf documentation):
    _stateVariablesData[apGL_PIXEL_MAP_I_TO_I_SIZE] = new StateVariableData(L"GL_PIXEL_MAP_I_TO_I_SIZE", ap_glGetIntegerv, GL_PIXEL_MAP_I_TO_I_SIZE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PIXEL_MAP_S_TO_S_SIZE] = new StateVariableData(L"GL_PIXEL_MAP_S_TO_S_SIZE", ap_glGetIntegerv, GL_PIXEL_MAP_S_TO_S_SIZE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PIXEL_MAP_I_TO_R_SIZE] = new StateVariableData(L"GL_PIXEL_MAP_I_TO_R_SIZE", ap_glGetIntegerv, GL_PIXEL_MAP_I_TO_R_SIZE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PIXEL_MAP_I_TO_G_SIZE] = new StateVariableData(L"GL_PIXEL_MAP_I_TO_G_SIZE", ap_glGetIntegerv, GL_PIXEL_MAP_I_TO_G_SIZE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PIXEL_MAP_I_TO_B_SIZE] = new StateVariableData(L"GL_PIXEL_MAP_I_TO_B_SIZE", ap_glGetIntegerv, GL_PIXEL_MAP_I_TO_B_SIZE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PIXEL_MAP_I_TO_A_SIZE] = new StateVariableData(L"GL_PIXEL_MAP_I_TO_A_SIZE", ap_glGetIntegerv, GL_PIXEL_MAP_I_TO_A_SIZE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PIXEL_MAP_R_TO_R_SIZE] = new StateVariableData(L"GL_PIXEL_MAP_R_TO_R_SIZE", ap_glGetIntegerv, GL_PIXEL_MAP_R_TO_R_SIZE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PIXEL_MAP_G_TO_G_SIZE] = new StateVariableData(L"GL_PIXEL_MAP_G_TO_G_SIZE", ap_glGetIntegerv, GL_PIXEL_MAP_G_TO_G_SIZE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PIXEL_MAP_B_TO_B_SIZE] = new StateVariableData(L"GL_PIXEL_MAP_B_TO_B_SIZE", ap_glGetIntegerv, GL_PIXEL_MAP_B_TO_B_SIZE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PIXEL_MAP_A_TO_A_SIZE] = new StateVariableData(L"GL_PIXEL_MAP_A_TO_A_SIZE", ap_glGetIntegerv, GL_PIXEL_MAP_A_TO_A_SIZE, 0, AP_OPENGL_STATE_VAR);

    // State variables that return GLenum:
    _stateVariablesData[apGL_MATRIX_MODE] = new StateVariableData(L"GL_MATRIX_MODE", GL_MATRIX_MODE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_SHADE_MODEL] = new StateVariableData(L"GL_SHADE_MODEL", GL_SHADE_MODEL, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);

    // texture_object
    _stateVariablesData[apGL_TEXTURE_BINDING_1D] = new StateVariableData(L"GL_TEXTURE_BINDING_1D", ap_glGetIntegerv, GL_TEXTURE_BINDING_1D, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_BINDING_2D] = new StateVariableData(L"GL_TEXTURE_BINDING_2D", ap_glGetIntegerv, GL_TEXTURE_BINDING_2D, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_BINDING_3D] = new StateVariableData(L"GL_TEXTURE_BINDING_3D", ap_glGetIntegerv, GL_TEXTURE_BINDING_3D, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_ENV_MODE] = new StateVariableData(L"GL_TEXTURE_ENV_MODE", ap_glGetTexEnvfv, GL_TEXTURE_ENV_MODE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_ENV_COLOR] = new StateVariableData(L"GL_TEXTURE_ENV_COLOR", ap_glGetTexEnvfv, GL_TEXTURE_ENV_COLOR, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);

    // Additions of missing variables (9/11/2005)
    _stateVariablesData[apGL_CURRENT_COLOR] = new StateVariableData(L"GL_CURRENT_COLOR", ap_glGetFloatv, 4, GL_CURRENT_COLOR, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_RESCALE_NORMAL] = new StateVariableData(L"GL_RESCALE_NORMAL", ap_glIsEnabled, GL_RESCALE_NORMAL, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_POLYGON_OFFSET_FACTOR] = new StateVariableData(L"GL_POLYGON_OFFSET_FACTOR", ap_glGetFloatv, GL_POLYGON_OFFSET_FACTOR, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_POLYGON_OFFSET_UNITS] = new StateVariableData(L"GL_POLYGON_OFFSET_UNITS", ap_glGetFloatv, GL_POLYGON_OFFSET_UNITS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_POLYGON_OFFSET_POINT] = new StateVariableData(L"GL_POLYGON_OFFSET_POINT", ap_glIsEnabled, GL_POLYGON_OFFSET_POINT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_POLYGON_OFFSET_LINE] = new StateVariableData(L"GL_POLYGON_OFFSET_LINE", ap_glIsEnabled, GL_POLYGON_OFFSET_LINE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_POLYGON_OFFSET_FILL] = new StateVariableData(L"GL_POLYGON_OFFSET_FILL", ap_glIsEnabled, GL_POLYGON_OFFSET_FILL, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);

    // ----------------------------------------------------
    //      OpenGL extensions state variables:
    // ----------------------------------------------------

    // OpenGL 1.2
    _stateVariablesData[apGL_TEXTURE_3D] = new StateVariableData(L"GL_TEXTURE_3D", ap_glGetBooleanv, GL_TEXTURE_3D, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_ELEMENTS_VERTICES] = new StateVariableData(L"GL_MAX_ELEMENTS_VERTICES", ap_glGetIntegerv, GL_MAX_ELEMENTS_VERTICES, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_ELEMENTS_INDICES] = new StateVariableData(L"GL_MAX_ELEMENTS_INDICES", ap_glGetIntegerv, GL_MAX_ELEMENTS_INDICES, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_BLEND_COLOR] = new StateVariableData(L"GL_BLEND_COLOR", ap_glGetFloatv, 4, GL_BLEND_COLOR, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_BLEND_EQUATION] = new StateVariableData(L"GL_BLEND_EQUATION", GL_BLEND_EQUATION, 0, AP_OPENGL_STATE_VAR);
    // Additions of missing variables (9/11/2005)
    _stateVariablesData[apGL_VERTEX_ARRAY] = new StateVariableData(L"GL_VERTEX_ARRAY", ap_glIsEnabled, GL_VERTEX_ARRAY, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ARRAY_SIZE] = new StateVariableData(L"GL_VERTEX_ARRAY_SIZE", ap_glGetIntegerv, GL_VERTEX_ARRAY_SIZE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ARRAY_STRIDE] = new StateVariableData(L"GL_VERTEX_ARRAY_STRIDE", ap_glGetIntegerv, GL_VERTEX_ARRAY_STRIDE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ARRAY_TYPE] = new StateVariableData(L"GL_VERTEX_ARRAY_TYPE", GL_VERTEX_ARRAY_TYPE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ARRAY_POINTER] = new StateVariableData(L"GL_VERTEX_ARRAY_POINTER", ap_glGetPointerv, GL_VERTEX_ARRAY_POINTER, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_NORMAL_ARRAY] = new StateVariableData(L"GL_NORMAL_ARRAY", ap_glIsEnabled, GL_NORMAL_ARRAY, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_NORMAL_ARRAY_STRIDE] = new StateVariableData(L"GL_NORMAL_ARRAY_STRIDE", ap_glGetIntegerv, GL_NORMAL_ARRAY_STRIDE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_NORMAL_ARRAY_TYPE] = new StateVariableData(L"GL_NORMAL_ARRAY_TYPE", GL_NORMAL_ARRAY_TYPE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_NORMAL_ARRAY_POINTER] = new StateVariableData(L"GL_NORMAL_ARRAY_POINTER", ap_glGetPointerv, GL_NORMAL_ARRAY_POINTER, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_COLOR_ARRAY] = new StateVariableData(L"GL_COLOR_ARRAY", ap_glIsEnabled, GL_COLOR_ARRAY, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_COLOR_ARRAY_SIZE] = new StateVariableData(L"GL_COLOR_ARRAY_SIZE", ap_glGetIntegerv, GL_COLOR_ARRAY_SIZE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_COLOR_ARRAY_STRIDE] = new StateVariableData(L"GL_COLOR_ARRAY_STRIDE", ap_glGetIntegerv, GL_COLOR_ARRAY_STRIDE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_COLOR_ARRAY_TYPE] = new StateVariableData(L"GL_COLOR_ARRAY_TYPE", GL_COLOR_ARRAY_TYPE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_COLOR_ARRAY_POINTER] = new StateVariableData(L"GL_COLOR_ARRAY_POINTER", ap_glGetPointerv, GL_COLOR_ARRAY_POINTER, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_COORD_ARRAY] = new StateVariableData(L"GL_TEXTURE_COORD_ARRAY", ap_glIsEnabled, GL_TEXTURE_COORD_ARRAY, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_COORD_ARRAY_SIZE] = new StateVariableData(L"GL_TEXTURE_COORD_ARRAY_SIZE", ap_glGetIntegerv, GL_TEXTURE_COORD_ARRAY_SIZE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_COORD_ARRAY_STRIDE] = new StateVariableData(L"GL_TEXTURE_COORD_ARRAY_STRIDE", ap_glGetIntegerv, GL_TEXTURE_COORD_ARRAY_STRIDE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_COORD_ARRAY_TYPE] = new StateVariableData(L"GL_TEXTURE_COORD_ARRAY_TYPE", GL_TEXTURE_COORD_ARRAY_TYPE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_COORD_ARRAY_POINTER] = new StateVariableData(L"GL_TEXTURE_COORD_ARRAY_POINTER", ap_glGetPointerv, GL_TEXTURE_COORD_ARRAY_POINTER, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);

    // OpenGL 1.3
    _stateVariablesData[apGL_TEXTURE_COMPRESSION_HINT] = new StateVariableData(L"GL_TEXTURE_COMPRESSION_HINT", GL_TEXTURE_COMPRESSION_HINT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_SAMPLE_COVERAGE_VALUE] = new StateVariableData(L"GL_SAMPLE_COVERAGE_VALUE", ap_glGetFloatv, GL_SAMPLE_COVERAGE_VALUE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_SAMPLE_COVERAGE_INVERT] = new StateVariableData(L"GL_SAMPLE_COVERAGE_INVERT", ap_glGetBooleanv, GL_SAMPLE_COVERAGE_INVERT, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_SAMPLE_COVERAGE] = new StateVariableData(L"GL_SAMPLE_COVERAGE", ap_glGetBooleanv, GL_SAMPLE_COVERAGE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_SAMPLE_ALPHA_TO_COVERAGE] = new StateVariableData(L"GL_SAMPLE_ALPHA_TO_COVERAGE", ap_glGetBooleanv, GL_SAMPLE_ALPHA_TO_COVERAGE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_SAMPLE_ALPHA_TO_ONE] = new StateVariableData(L"GL_SAMPLE_ALPHA_TO_ONE", ap_glGetBooleanv, GL_SAMPLE_ALPHA_TO_ONE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_MULTISAMPLE] = new StateVariableData(L"GL_MULTISAMPLE", ap_glGetBooleanv, GL_MULTISAMPLE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_ACTIVE_TEXTURE] = new StateVariableData(L"GL_ACTIVE_TEXTURE", GL_ACTIVE_TEXTURE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_CLIENT_ACTIVE_TEXTURE] = new StateVariableData(L"GL_CLIENT_ACTIVE_TEXTURE", GL_CLIENT_ACTIVE_TEXTURE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_MAX_TEXTURE_UNITS] = new StateVariableData(L"GL_MAX_TEXTURE_UNITS", ap_glGetIntegerv, GL_MAX_TEXTURE_UNITS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);

    // OpenGL 1.4
    _stateVariablesData[apGL_BLEND_SRC_RGB] = new StateVariableData(L"GL_BLEND_SRC_RGB", GL_BLEND_SRC_RGB, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_BLEND_SRC_ALPHA] = new StateVariableData(L"GL_BLEND_SRC_ALPHA", GL_BLEND_SRC_ALPHA, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_BLEND_DST_RGB] = new StateVariableData(L"GL_BLEND_DST_RGB", GL_BLEND_DST_RGB, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_BLEND_DST_ALPHA] = new StateVariableData(L"GL_BLEND_DST_ALPHA", GL_BLEND_DST_ALPHA, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_CURRENT_FOG_COORDINATE] = new StateVariableData(L"GL_CURRENT_FOG_COORDINATE", ap_glGetDoublev, GL_CURRENT_FOG_COORDINATE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_FOG_COORDINATE_ARRAY] = new StateVariableData(L"GL_FOG_COORDINATE_ARRAY", ap_glGetBooleanv, GL_FOG_COORDINATE_ARRAY, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_FOG_COORDINATE_ARRAY_STRIDE] = new StateVariableData(L"GL_FOG_COORDINATE_ARRAY_STRIDE", ap_glGetIntegerv, GL_FOG_COORDINATE_ARRAY_STRIDE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_FOG_COORDINATE_ARRAY_TYPE] = new StateVariableData(L"GL_FOG_COORDINATE_ARRAY_TYPE", GL_FOG_COORDINATE_ARRAY_TYPE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_FOG_COORDINATE_ARRAY_POINTER] = new StateVariableData(L"GL_FOG_COORDINATE_ARRAY_POINTER", ap_glGetPointerv, GL_FOG_COORDINATE_ARRAY_POINTER, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_POINT_SIZE_MIN] = new StateVariableData(L"GL_POINT_SIZE_MIN", ap_glGetFloatv, GL_POINT_SIZE_MIN, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_POINT_SIZE_MAX] = new StateVariableData(L"GL_POINT_SIZE_MAX", ap_glGetFloatv, GL_POINT_SIZE_MAX, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_POINT_FADE_THRESHOLD_SIZE] = new StateVariableData(L"GL_POINT_FADE_THRESHOLD_SIZE", ap_glGetFloatv, GL_POINT_FADE_THRESHOLD_SIZE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_POINT_DISTANCE_ATTENUATION] = new StateVariableData(L"GL_POINT_DISTANCE_ATTENUATION", ap_glGetFloatv, 3, GL_POINT_DISTANCE_ATTENUATION, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_CURRENT_SECONDARY_COLOR] = new StateVariableData(L"GL_CURRENT_SECONDARY_COLOR", ap_glGetFloatv, 4, GL_CURRENT_SECONDARY_COLOR, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_COLOR_SUM] = new StateVariableData(L"GL_COLOR_SUM", ap_glGetBooleanv, GL_COLOR_SUM, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_SECONDARY_COLOR_ARRAY] = new StateVariableData(L"GL_SECONDARY_COLOR_ARRAY", ap_glIsEnabled, GL_SECONDARY_COLOR_ARRAY, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_SECONDARY_COLOR_ARRAY_SIZE] = new StateVariableData(L"GL_SECONDARY_COLOR_ARRAY_SIZE", ap_glGetIntegerv, GL_SECONDARY_COLOR_ARRAY_SIZE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_SECONDARY_COLOR_ARRAY_TYPE] = new StateVariableData(L"GL_SECONDARY_COLOR_ARRAY_TYPE", GL_SECONDARY_COLOR_ARRAY_TYPE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_SECONDARY_COLOR_ARRAY_STRIDE] = new StateVariableData(L"GL_SECONDARY_COLOR_ARRAY_STRIDE", ap_glGetIntegerv, GL_SECONDARY_COLOR_ARRAY_STRIDE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_SECONDARY_COLOR_ARRAY_POINTER] = new StateVariableData(L"GL_SECONDARY_COLOR_ARRAY_POINTER", ap_glGetPointerv, GL_SECONDARY_COLOR_ARRAY_POINTER, 0, AP_OPENGL_STATE_VAR);

    // OpenGL 1.5
    _stateVariablesData[apGL_BUFFER_SIZE_array_buffer] = new StateVariableData(L"GL_BUFFER_SIZE - array buffer", ap_glGetBufferParameteriv, GL_ARRAY_BUFFER, GL_BUFFER_SIZE, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_BUFFER_USAGE_array_buffer] = new StateVariableData(L"GL_BUFFER_USAGE - array buffer", ap_glGetBufferParameteriv, GL_ARRAY_BUFFER, GL_BUFFER_USAGE, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_BUFFER_ACCESS_array_buffer] = new StateVariableData(L"GL_ARRAY_BUFFER - GL_BUFFER_ACCESS", ap_glGetBufferParameteriv, GL_ARRAY_BUFFER, GL_BUFFER_ACCESS, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_BUFFER_MAPPED_array_buffer] = new StateVariableData(L"GL_ARRAY_BUFFER - GL_BUFFER_MAPPED", ap_glGetBufferParameteriv, GL_ARRAY_BUFFER, GL_BUFFER_MAPPED, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_BUFFER_SIZE_element_array_buffer] = new StateVariableData(L"GL_BUFFER_SIZE - element array buffer", ap_glGetBufferParameteriv, GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_BUFFER_USAGE_element_array_buffer] = new StateVariableData(L"GL_BUFFER_USAGE - element array buffer", ap_glGetBufferParameteriv, GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_USAGE, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_BUFFER_ACCESS_element_array_buffer] = new StateVariableData(L"GL_ELEMENT_ARRAY_BUFFER - GL_BUFFER_ACCESS", ap_glGetBufferParameteriv, GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_ACCESS, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_BUFFER_MAPPED_element_array_buffer] = new StateVariableData(L"GL_ELEMENT_ARRAY_BUFFER - GL_BUFFER_MAPPED", ap_glGetBufferParameteriv, GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_MAPPED, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_QUERY_COUNTER_BITS] = new StateVariableData(L"GL_QUERY_COUNTER_BITS", ap_glGetQueryiv, GL_SAMPLES_PASSED_ARB, GL_QUERY_COUNTER_BITS, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ARRAY_BUFFER_BINDING] = new StateVariableData(L"GL_ARRAY_BUFFER_BINDING", ap_glGetIntegerv, GL_ARRAY_BUFFER_BINDING, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_ELEMENT_ARRAY_BUFFER_BINDING] = new StateVariableData(L"GL_ELEMENT_ARRAY_BUFFER_BINDING", ap_glGetIntegerv, GL_ELEMENT_ARRAY_BUFFER_BINDING, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ARRAY_BUFFER_BINDING] = new StateVariableData(L"GL_VERTEX_ARRAY_BUFFER_BINDING", ap_glGetIntegerv, GL_VERTEX_ARRAY_BUFFER_BINDING, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_NORMAL_ARRAY_BUFFER_BINDING] = new StateVariableData(L"GL_NORMAL_ARRAY_BUFFER_BINDING", ap_glGetIntegerv, GL_NORMAL_ARRAY_BUFFER_BINDING, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_COLOR_ARRAY_BUFFER_BINDING] = new StateVariableData(L"GL_COLOR_ARRAY_BUFFER_BINDING", ap_glGetIntegerv, GL_COLOR_ARRAY_BUFFER_BINDING, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_INDEX_ARRAY_BUFFER_BINDING] = new StateVariableData(L"GL_INDEX_ARRAY_BUFFER_BINDING", ap_glGetIntegerv, GL_INDEX_ARRAY_BUFFER_BINDING, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_COORD_ARRAY_BUFFER_BINDING] = new StateVariableData(L"GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING", ap_glGetIntegerv, GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_EDGE_FLAG_ARRAY_BUFFER_BINDING] = new StateVariableData(L"GL_EDGE_FLAG_ARRAY_BUFFER_BINDING", ap_glGetIntegerv, GL_EDGE_FLAG_ARRAY_BUFFER_BINDING, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING] = new StateVariableData(L"GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING", ap_glGetIntegerv, GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_FOG_COORD_SRC] = new StateVariableData(L"GL_FOG_COORD_SRC", GL_FOG_COORD_SRC, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CURRENT_FOG_COORD] = new StateVariableData(L"GL_CURRENT_FOG_COORD", ap_glGetFloatv, GL_CURRENT_FOG_COORD, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_FOG_COORD_ARRAY_TYPE] = new StateVariableData(L"GL_FOG_COORD_ARRAY_TYPE", GL_FOG_COORD_ARRAY_TYPE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_FOG_COORD_ARRAY_STRIDE] = new StateVariableData(L"GL_FOG_COORD_ARRAY_STRIDE", ap_glGetIntegerv, GL_FOG_COORD_ARRAY_STRIDE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_FOG_COORD_ARRAY_POINTER] = new StateVariableData(L"GL_FOG_COORD_ARRAY_POINTER", ap_glGetPointerv, GL_FOG_COORD_ARRAY_POINTER, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_FOG_COORD_ARRAY] = new StateVariableData(L"GL_FOG_COORD_ARRAY", ap_glIsEnabled, GL_FOG_COORD_ARRAY, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_FOG_COORD_ARRAY_BUFFER_BINDING] = new StateVariableData(L"GL_FOG_COORD_ARRAY_BUFFER_BINDING", ap_glGetIntegerv, GL_FOG_COORD_ARRAY_BUFFER_BINDING, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_SRC0_RGB] = new StateVariableData(L"GL_SRC0_RGB", ap_glGetTexEnvfv, GL_SRC0_RGB, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_SRC1_RGB] = new StateVariableData(L"GL_SRC1_RGB", ap_glGetTexEnvfv, GL_SRC1_RGB, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_SRC2_RGB] = new StateVariableData(L"GL_SRC2_RGB", ap_glGetTexEnvfv, GL_SRC2_RGB, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_SRC0_ALPHA] = new StateVariableData(L"GL_SRC0_ALPHA", ap_glGetTexEnvfv, GL_SRC0_ALPHA, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_SRC1_ALPHA] = new StateVariableData(L"GL_SRC1_ALPHA", ap_glGetTexEnvfv, GL_SRC1_ALPHA, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_SRC2_ALPHA] = new StateVariableData(L"GL_SRC2_ALPHA", ap_glGetTexEnvfv, GL_SRC2_ALPHA, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);

    // Additions of missing variables (9/11/2005)
    _stateVariablesData[apGL_COMBINE_RGB] = new StateVariableData(L"GL_COMBINE_RGB", ap_glGetTexEnviv, GL_COMBINE_RGB, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_COMBINE_ALPHA] = new StateVariableData(L"GL_COMBINE_ALPHA", ap_glGetTexEnviv, GL_COMBINE_ALPHA, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_OPERAND0_RGB] = new StateVariableData(L"GL_OPERAND0_RGB", ap_glGetTexEnviv, GL_OPERAND0_RGB, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_OPERAND1_RGB] = new StateVariableData(L"GL_OPERAND1_RGB", ap_glGetTexEnviv, GL_OPERAND1_RGB, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_OPERAND2_RGB] = new StateVariableData(L"GL_OPERAND2_RGB", ap_glGetTexEnviv, GL_OPERAND2_RGB, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_OPERAND0_ALPHA] = new StateVariableData(L"GL_OPERAND0_ALPHA", ap_glGetTexEnviv, GL_OPERAND0_ALPHA, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_OPERAND1_ALPHA] = new StateVariableData(L"GL_OPERAND1_ALPHA", ap_glGetTexEnviv, GL_OPERAND1_ALPHA, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_OPERAND2_ALPHA] = new StateVariableData(L"GL_OPERAND2_ALPHA", ap_glGetTexEnviv, GL_OPERAND2_ALPHA, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_RGB_SCALE] = new StateVariableData(L"GL_RGB_SCALE", ap_glGetTexEnviv, GL_RGB_SCALE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_ALPHA_SCALE_TexEnv] = new StateVariableData(L"GL_ALPHA_SCALE - texture combiner", ap_glGetTexEnviv, GL_RGB_SCALE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_COLOR_LOGIC_OP] = new StateVariableData(L"GL_COLOR_LOGIC_OP", ap_glIsEnabled, GL_COLOR_LOGIC_OP, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_GENERATE_MIPMAP_HINT] = new StateVariableData(L"GL_GENERATE_MIPMAP_HINT", GL_GENERATE_MIPMAP_HINT, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_ALIASED_POINT_SIZE_RANGE] = new StateVariableData(L"GL_ALIASED_POINT_SIZE_RANGE", ap_glGetFloatv, GL_ALIASED_POINT_SIZE_RANGE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_SMOOTH_POINT_SIZE_RANGE] = new StateVariableData(L"GL_SMOOTH_POINT_SIZE_RANGE", ap_glGetFloatv, GL_SMOOTH_POINT_SIZE_RANGE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_ALIASED_LINE_WIDTH_RANGE] = new StateVariableData(L"GL_ALIASED_LINE_WIDTH_RANGE", ap_glGetFloatv, GL_ALIASED_LINE_WIDTH_RANGE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_SMOOTH_LINE_WIDTH_RANGE] = new StateVariableData(L"GL_SMOOTH_LINE_WIDTH_RANGE", ap_glGetFloatv, GL_SMOOTH_LINE_WIDTH_RANGE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_SAMPLE_BUFFERS] = new StateVariableData(L"GL_SAMPLE_BUFFERS", ap_glGetIntegerv, GL_SAMPLE_BUFFERS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_SAMPLES] = new StateVariableData(L"GL_SAMPLES", ap_glGetIntegerv, GL_SAMPLES, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_COMPRESSED_TEXTURE_FORMATS] = new StateVariableData(L"GL_COMPRESSED_TEXTURE_FORMATS", GL_COMPRESSED_TEXTURE_FORMATS, GL_NUM_COMPRESSED_TEXTURE_FORMATS, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_NUM_COMPRESSED_TEXTURE_FORMATS] = new StateVariableData(L"GL_NUM_COMPRESSED_TEXTURE_FORMATS", ap_glGetIntegerv, GL_NUM_COMPRESSED_TEXTURE_FORMATS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_1_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);

    // OpenGL 2.0
    _stateVariablesData[apGL_BLEND_EQUATION_RGB] = new StateVariableData(L"GL_BLEND_EQUATION_RGB", GL_BLEND_EQUATION_RGB, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_PROGRAM_POINT_SIZE] = new StateVariableData(L"GL_VERTEX_PROGRAM_POINT_SIZE", ap_glIsEnabled, GL_VERTEX_PROGRAM_POINT_SIZE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_PROGRAM_TWO_SIDE] = new StateVariableData(L"GL_VERTEX_PROGRAM_TWO_SIDE", ap_glIsEnabled, GL_VERTEX_PROGRAM_TWO_SIDE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_STENCIL_BACK_FUNC] = new StateVariableData(L"GL_STENCIL_BACK_FUNC", GL_STENCIL_BACK_FUNC, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_STENCIL_BACK_FAIL] = new StateVariableData(L"GL_STENCIL_BACK_FAIL", GL_STENCIL_BACK_FAIL, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_STENCIL_BACK_PASS_DEPTH_FAIL] = new StateVariableData(L"GL_STENCIL_BACK_PASS_DEPTH_FAIL", GL_STENCIL_BACK_PASS_DEPTH_FAIL, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_STENCIL_BACK_PASS_DEPTH_PASS] = new StateVariableData(L"GL_STENCIL_BACK_PASS_DEPTH_PASS", GL_STENCIL_BACK_PASS_DEPTH_PASS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_MAX_DRAW_BUFFERS] = new StateVariableData(L"GL_MAX_DRAW_BUFFERS", ap_glGetIntegerv, GL_MAX_DRAW_BUFFERS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER0] = new StateVariableData(L"GL_DRAW_BUFFER0", GL_DRAW_BUFFER0, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER1] = new StateVariableData(L"GL_DRAW_BUFFER1", GL_DRAW_BUFFER1, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER2] = new StateVariableData(L"GL_DRAW_BUFFER2", GL_DRAW_BUFFER2, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER3] = new StateVariableData(L"GL_DRAW_BUFFER3", GL_DRAW_BUFFER3, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER4] = new StateVariableData(L"GL_DRAW_BUFFER4", GL_DRAW_BUFFER4, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER5] = new StateVariableData(L"GL_DRAW_BUFFER5", GL_DRAW_BUFFER5, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER6] = new StateVariableData(L"GL_DRAW_BUFFER6", GL_DRAW_BUFFER6, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER7] = new StateVariableData(L"GL_DRAW_BUFFER7", GL_DRAW_BUFFER7, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER8] = new StateVariableData(L"GL_DRAW_BUFFER8", GL_DRAW_BUFFER8, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER9] = new StateVariableData(L"GL_DRAW_BUFFER9", GL_DRAW_BUFFER9, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER10] = new StateVariableData(L"GL_DRAW_BUFFER10", GL_DRAW_BUFFER10, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER11] = new StateVariableData(L"GL_DRAW_BUFFER11", GL_DRAW_BUFFER11, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER12] = new StateVariableData(L"GL_DRAW_BUFFER12", GL_DRAW_BUFFER12, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER13] = new StateVariableData(L"GL_DRAW_BUFFER13", GL_DRAW_BUFFER13, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER14] = new StateVariableData(L"GL_DRAW_BUFFER14", GL_DRAW_BUFFER14, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER15] = new StateVariableData(L"GL_DRAW_BUFFER15", GL_DRAW_BUFFER15, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_BLEND_EQUATION_ALPHA] = new StateVariableData(L"GL_BLEND_EQUATION_ALPHA", GL_BLEND_EQUATION_ALPHA, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_POINT_SPRITE] = new StateVariableData(L"GL_POINT_SPRITE", ap_glIsEnabled, GL_POINT_SPRITE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_ATTRIBS] = new StateVariableData(L"GL_MAX_VERTEX_ATTRIBS", ap_glGetIntegerv, GL_MAX_VERTEX_ATTRIBS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_MAX_TEXTURE_COORDS] = new StateVariableData(L"GL_MAX_TEXTURE_COORDS", ap_glGetIntegerv, GL_MAX_TEXTURE_COORDS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TEXTURE_IMAGE_UNITS] = new StateVariableData(L"GL_MAX_TEXTURE_IMAGE_UNITS", ap_glGetIntegerv, GL_MAX_TEXTURE_IMAGE_UNITS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_UNIFORM_COMPONENTS] = new StateVariableData(L"GL_MAX_VERTEX_UNIFORM_COMPONENTS", ap_glGetIntegerv, GL_MAX_VERTEX_UNIFORM_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VARYING_FLOATS] = new StateVariableData(L"GL_MAX_VARYING_FLOATS", ap_glGetIntegerv, GL_MAX_VARYING_FLOATS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_TEXTURE_IMAGE_UNITS] = new StateVariableData(L"GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS", ap_glGetIntegerv, GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS] = new StateVariableData(L"GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS", ap_glGetIntegerv, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_FRAGMENT_SHADER_DERIVATIVE_HINT] = new StateVariableData(L"GL_FRAGMENT_SHADER_DERIVATIVE_HINT", GL_FRAGMENT_SHADER_DERIVATIVE_HINT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CURRENT_PROGRAM] = new StateVariableData(L"GL_CURRENT_PROGRAM", ap_glGetIntegerv, GL_CURRENT_PROGRAM, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_STENCIL_BACK_REF] = new StateVariableData(L"GL_STENCIL_BACK_REF", ap_glGetIntegerv, GL_STENCIL_BACK_REF, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_STENCIL_BACK_VALUE_MASK] = new StateVariableData(L"GL_STENCIL_BACK_VALUE_MASK", ap_glGetIntegerv, GL_STENCIL_BACK_VALUE_MASK, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_STENCIL_BACK_WRITEMASK] = new StateVariableData(L"GL_STENCIL_BACK_WRITEMASK", ap_glGetIntegerv, GL_STENCIL_BACK_WRITEMASK, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);

    // OpenGL 3.0
    _stateVariablesData[apGL_CLIP_DISTANCE0] = new StateVariableData(L"GL_CLIP_DISTANCE0", ap_glGetBooleanv, GL_CLIP_DISTANCE0, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CLIP_DISTANCE1] = new StateVariableData(L"GL_CLIP_DISTANCE1", ap_glGetBooleanv, GL_CLIP_DISTANCE1, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CLIP_DISTANCE2] = new StateVariableData(L"GL_CLIP_DISTANCE2", ap_glGetBooleanv, GL_CLIP_DISTANCE2, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CLIP_DISTANCE3] = new StateVariableData(L"GL_CLIP_DISTANCE3", ap_glGetBooleanv, GL_CLIP_DISTANCE3, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CLIP_DISTANCE4] = new StateVariableData(L"GL_CLIP_DISTANCE4", ap_glGetBooleanv, GL_CLIP_DISTANCE4, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CLIP_DISTANCE5] = new StateVariableData(L"GL_CLIP_DISTANCE5", ap_glGetBooleanv, GL_CLIP_DISTANCE5, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_CLIP_DISTANCES] = new StateVariableData(L"GL_MAX_CLIP_DISTANCES", ap_glGetIntegerv, GL_MAX_CLIP_DISTANCES, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAJOR_VERSION] = new StateVariableData(L"GL_MAJOR_VERSION", ap_glGetIntegerv, GL_MAJOR_VERSION, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MINOR_VERSION] = new StateVariableData(L"GL_MINOR_VERSION", ap_glGetIntegerv, GL_MINOR_VERSION, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_NUM_EXTENSIONS] = new StateVariableData(L"GL_NUM_EXTENSIONS", ap_glGetIntegerv, GL_NUM_EXTENSIONS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CONTEXT_FLAGS] = new StateVariableData(L"GL_CONTEXT_FLAGS", ap_glGetIntegerv, GL_CONTEXT_FLAGS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_ARRAY_TEXTURE_LAYERS] = new StateVariableData(L"GL_MAX_ARRAY_TEXTURE_LAYERS", ap_glGetIntegerv, GL_MAX_ARRAY_TEXTURE_LAYERS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MIN_PROGRAM_TEXEL_OFFSET] = new StateVariableData(L"GL_MIN_PROGRAM_TEXEL_OFFSET", ap_glGetIntegerv, GL_MIN_PROGRAM_TEXEL_OFFSET, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_TEXEL_OFFSET] = new StateVariableData(L"GL_MAX_PROGRAM_TEXEL_OFFSET", ap_glGetIntegerv, GL_MAX_PROGRAM_TEXEL_OFFSET, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CLAMP_VERTEX_COLOR] = new StateVariableData(L"GL_CLAMP_VERTEX_COLOR", GL_CLAMP_VERTEX_COLOR, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CLAMP_FRAGMENT_COLOR] = new StateVariableData(L"GL_CLAMP_FRAGMENT_COLOR", GL_CLAMP_FRAGMENT_COLOR, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CLAMP_READ_COLOR] = new StateVariableData(L"GL_CLAMP_READ_COLOR", GL_CLAMP_READ_COLOR, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VARYING_COMPONENTS] = new StateVariableData(L"GL_MAX_VARYING_COMPONENTS", ap_glGetIntegerv, GL_MAX_VARYING_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_BINDING_1D_ARRAY] = new StateVariableData(L"GL_TEXTURE_BINDING_1D_ARRAY", ap_glGetIntegerv, GL_TEXTURE_BINDING_1D_ARRAY, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_BINDING_2D_ARRAY] = new StateVariableData(L"GL_TEXTURE_BINDING_2D_ARRAY", ap_glGetIntegerv, GL_TEXTURE_BINDING_2D_ARRAY, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS] = new StateVariableData(L"GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS", ap_glGetIntegerv, GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TRANSFORM_FEEDBACK_BUFFER_BINDING] = new StateVariableData(L"GL_TRANSFORM_FEEDBACK_BUFFER_BINDING", ap_glGetIntegerv, GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_RASTERIZER_DISCARD] = new StateVariableData(L"GL_RASTERIZER_DISCARD", ap_glIsEnabled, GL_RASTERIZER_DISCARD, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS] = new StateVariableData(L"GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS", ap_glGetIntegerv, GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS] = new StateVariableData(L"GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS", ap_glGetIntegerv, GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS, 0, AP_OPENGL_STATE_VAR);

    // TO_DO: 3.1? 3.2? 3.3?

    // OpenGL 4.0
    _stateVariablesData[apGL_DRAW_INDIRECT_BUFFER_BINDING] = new StateVariableData(L"GL_DRAW_INDIRECT_BUFFER_BINDING", ap_glGetIntegerv, GL_DRAW_INDIRECT_BUFFER_BINDING, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_GEOMETRY_SHADER_INVOCATIONS] = new StateVariableData(L"GL_MAX_GEOMETRY_SHADER_INVOCATIONS", ap_glGetIntegerv, GL_MAX_GEOMETRY_SHADER_INVOCATIONS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MIN_FRAGMENT_INTERPOLATION_OFFSET] = new StateVariableData(L"GL_MIN_FRAGMENT_INTERPOLATION_OFFSET", ap_glGetIntegerv, GL_MIN_FRAGMENT_INTERPOLATION_OFFSET, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_FRAGMENT_INTERPOLATION_OFFSET] = new StateVariableData(L"GL_MAX_FRAGMENT_INTERPOLATION_OFFSET", ap_glGetIntegerv, GL_MAX_FRAGMENT_INTERPOLATION_OFFSET, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_FRAGMENT_INTERPOLATION_OFFSET_BITS] = new StateVariableData(L"GL_FRAGMENT_INTERPOLATION_OFFSET_BITS", ap_glGetIntegerv, GL_FRAGMENT_INTERPOLATION_OFFSET_BITS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_STREAMS] = new StateVariableData(L"GL_MAX_VERTEX_STREAMS", ap_glGetIntegerv, GL_MAX_VERTEX_STREAMS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_SUBROUTINES] = new StateVariableData(L"GL_MAX_SUBROUTINES", ap_glGetIntegerv, GL_MAX_SUBROUTINES, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_SUBROUTINE_UNIFORM_LOCATIONS] = new StateVariableData(L"GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS", ap_glGetIntegerv, GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PATCH_VERTICES] = new StateVariableData(L"GL_PATCH_VERTICES", ap_glGetIntegerv, GL_PATCH_VERTICES, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PATCH_DEFAULT_INNER_LEVEL] = new StateVariableData(L"GL_PATCH_DEFAULT_INNER_LEVEL", ap_glGetIntegerv, GL_PATCH_DEFAULT_INNER_LEVEL, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PATCH_DEFAULT_OUTER_LEVEL] = new StateVariableData(L"GL_PATCH_DEFAULT_OUTER_LEVEL", ap_glGetIntegerv, GL_PATCH_DEFAULT_OUTER_LEVEL, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PATCH_VERTICES] = new StateVariableData(L"GL_MAX_PATCH_VERTICES", ap_glGetIntegerv, GL_MAX_PATCH_VERTICES, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_GEN_LEVEL] = new StateVariableData(L"GL_MAX_TESS_GEN_LEVEL", ap_glGetIntegerv, GL_MAX_TESS_GEN_LEVEL, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS] = new StateVariableData(L"GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS", ap_glGetIntegerv, GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS] = new StateVariableData(L"GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS", ap_glGetIntegerv, GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS] = new StateVariableData(L"GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS", ap_glGetIntegerv, GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS] = new StateVariableData(L"GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS", ap_glGetIntegerv, GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS] = new StateVariableData(L"GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS", ap_glGetIntegerv, GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_PATCH_COMPONENTS] = new StateVariableData(L"GL_MAX_TESS_PATCH_COMPONENTS", ap_glGetIntegerv, GL_MAX_TESS_PATCH_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS] = new StateVariableData(L"GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS", ap_glGetIntegerv, GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS] = new StateVariableData(L"GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS", ap_glGetIntegerv, GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_CONTROL_UNIFORM_BLOCKS] = new StateVariableData(L"GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS", ap_glGetIntegerv, GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS] = new StateVariableData(L"GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS", ap_glGetIntegerv, GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_CONTROL_INPUT_COMPONENTS] = new StateVariableData(L"GL_MAX_TESS_CONTROL_INPUT_COMPONENTS", ap_glGetIntegerv, GL_MAX_TESS_CONTROL_INPUT_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_EVALUATION_INPUT_COMPONENTS] = new StateVariableData(L"GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS", ap_glGetIntegerv, GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS] = new StateVariableData(L"GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS", ap_glGetIntegerv, GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS] = new StateVariableData(L"GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS", ap_glGetIntegerv, GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER] = new StateVariableData(L"GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER", ap_glGetIntegerv, GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER] = new StateVariableData(L"GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER", ap_glGetIntegerv, GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TRANSFORM_FEEDBACK_BUFFER_PAUSED] = new StateVariableData(L"GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED", ap_glGetBooleanv, GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE] = new StateVariableData(L"GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE", ap_glGetBooleanv, GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TRANSFORM_FEEDBACK_BINDING] = new StateVariableData(L"GL_TRANSFORM_FEEDBACK_BINDING", ap_glGetIntegerv, GL_TRANSFORM_FEEDBACK_BINDING, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TRANSFORM_FEEDBACK_BUFFERS] = new StateVariableData(L"GL_MAX_TRANSFORM_FEEDBACK_BUFFERS", ap_glGetIntegerv, GL_MAX_TRANSFORM_FEEDBACK_BUFFERS, 0, AP_OPENGL_STATE_VAR);

    // OpenGL 4.1
    _stateVariablesData[apGL_IMPLEMENTATION_COLOR_READ_TYPE] = new StateVariableData(L"GL_IMPLEMENTATION_COLOR_READ_TYPE", GL_IMPLEMENTATION_COLOR_READ_TYPE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_IMPLEMENTATION_COLOR_READ_FORMAT] = new StateVariableData(L"GL_IMPLEMENTATION_COLOR_READ_FORMAT", GL_IMPLEMENTATION_COLOR_READ_FORMAT, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_SHADER_COMPILER] = new StateVariableData(L"GL_SHADER_COMPILER", ap_glGetBooleanv, GL_SHADER_COMPILER, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_SHADER_BINARY_FORMATS] = new StateVariableData(L"GL_SHADER_BINARY_FORMATS", GL_SHADER_BINARY_FORMATS, GL_NUM_SHADER_BINARY_FORMATS, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_NUM_SHADER_BINARY_FORMATS] = new StateVariableData(L"GL_NUM_SHADER_BINARY_FORMATS", ap_glGetIntegerv, GL_NUM_SHADER_BINARY_FORMATS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_UNIFORM_VECTORS] = new StateVariableData(L"GL_MAX_VERTEX_UNIFORM_VECTORS", ap_glGetIntegerv, GL_MAX_VERTEX_UNIFORM_VECTORS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_MAX_VARYING_VECTORS] = new StateVariableData(L"GL_MAX_VARYING_VECTORS", ap_glGetIntegerv, GL_MAX_VARYING_VECTORS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_MAX_FRAGMENT_UNIFORM_VECTORS] = new StateVariableData(L"GL_MAX_FRAGMENT_UNIFORM_VECTORS", ap_glGetIntegerv, GL_MAX_FRAGMENT_UNIFORM_VECTORS, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_NUM_PROGRAM_BINARY_FORMATS] = new StateVariableData(L"GL_NUM_PROGRAM_BINARY_FORMATS", ap_glGetIntegerv, GL_NUM_PROGRAM_BINARY_FORMATS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_BINARY_FORMATS] = new StateVariableData(L"GL_PROGRAM_BINARY_FORMATS", GL_PROGRAM_BINARY_FORMATS, GL_NUM_PROGRAM_BINARY_FORMATS, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_PIPELINE_BINDING] = new StateVariableData(L"GL_PROGRAM_PIPELINE_BINDING", ap_glGetIntegerv, GL_PROGRAM_PIPELINE_BINDING, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VIEWPORTS] = new StateVariableData(L"GL_MAX_VIEWPORTS", ap_glGetIntegerv, GL_MAX_VIEWPORTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VIEWPORT_SUBPIXEL_BITS] = new StateVariableData(L"GL_VIEWPORT_SUBPIXEL_BITS", ap_glGetIntegerv, GL_VIEWPORT_SUBPIXEL_BITS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VIEWPORT_BOUNDS_RANGE] = new StateVariableData(L"GL_VIEWPORT_BOUNDS_RANGE", ap_glGetIntegerv, 2, GL_VIEWPORT_BOUNDS_RANGE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_LAYER_PROVOKING_VERTEX] = new StateVariableData(L"GL_LAYER_PROVOKING_VERTEX", GL_LAYER_PROVOKING_VERTEX, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VIEWPORT_INDEX_PROVOKING_VERTEX] = new StateVariableData(L"GL_VIEWPORT_INDEX_PROVOKING_VERTEX", GL_VIEWPORT_INDEX_PROVOKING_VERTEX, 0, AP_OPENGL_STATE_VAR);

    // OpenGL 4.2
    _stateVariablesData[apGL_UNPACK_COMPRESSED_BLOCK_WIDTH] = new StateVariableData(L"GL_UNPACK_COMPRESSED_BLOCK_WIDTH", ap_glGetIntegerv, GL_UNPACK_COMPRESSED_BLOCK_WIDTH, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_UNPACK_COMPRESSED_BLOCK_HEIGHT] = new StateVariableData(L"GL_UNPACK_COMPRESSED_BLOCK_HEIGHT", ap_glGetIntegerv, GL_UNPACK_COMPRESSED_BLOCK_HEIGHT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_UNPACK_COMPRESSED_BLOCK_DEPTH] = new StateVariableData(L"GL_UNPACK_COMPRESSED_BLOCK_DEPTH", ap_glGetIntegerv, GL_UNPACK_COMPRESSED_BLOCK_DEPTH, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_UNPACK_COMPRESSED_BLOCK_SIZE] = new StateVariableData(L"GL_UNPACK_COMPRESSED_BLOCK_SIZE", ap_glGetIntegerv, GL_UNPACK_COMPRESSED_BLOCK_SIZE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PACK_COMPRESSED_BLOCK_WIDTH] = new StateVariableData(L"GL_PACK_COMPRESSED_BLOCK_WIDTH", ap_glGetIntegerv, GL_PACK_COMPRESSED_BLOCK_WIDTH, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PACK_COMPRESSED_BLOCK_HEIGHT] = new StateVariableData(L"GL_PACK_COMPRESSED_BLOCK_HEIGHT", ap_glGetIntegerv, GL_PACK_COMPRESSED_BLOCK_HEIGHT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PACK_COMPRESSED_BLOCK_DEPTH] = new StateVariableData(L"GL_PACK_COMPRESSED_BLOCK_DEPTH", ap_glGetIntegerv, GL_PACK_COMPRESSED_BLOCK_DEPTH, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PACK_COMPRESSED_BLOCK_SIZE] = new StateVariableData(L"GL_PACK_COMPRESSED_BLOCK_SIZE", ap_glGetIntegerv, GL_PACK_COMPRESSED_BLOCK_SIZE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MIN_MAP_BUFFER_ALIGNMENT] = new StateVariableData(L"GL_MIN_MAP_BUFFER_ALIGNMENT", ap_glGetIntegerv, GL_MIN_MAP_BUFFER_ALIGNMENT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ATOMIC_COUNTER_BUFFER_BINDING] = new StateVariableData(L"GL_ATOMIC_COUNTER_BUFFER_BINDING", ap_glGetIntegerv, GL_ATOMIC_COUNTER_BUFFER_BINDING, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS] = new StateVariableData(L"GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS", ap_glGetIntegerv, GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS] = new StateVariableData(L"GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS", ap_glGetIntegerv, GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS] = new StateVariableData(L"GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS", ap_glGetIntegerv, GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS] = new StateVariableData(L"GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS", ap_glGetIntegerv, GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS] = new StateVariableData(L"GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS", ap_glGetIntegerv, GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS] = new StateVariableData(L"GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS", ap_glGetIntegerv, GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_ATOMIC_COUNTERS] = new StateVariableData(L"GL_MAX_VERTEX_ATOMIC_COUNTERS", ap_glGetIntegerv, GL_MAX_VERTEX_ATOMIC_COUNTERS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_CONTROL_ATOMIC_COUNTERS] = new StateVariableData(L"GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS", ap_glGetIntegerv, GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS] = new StateVariableData(L"GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS", ap_glGetIntegerv, GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_GEOMETRY_ATOMIC_COUNTERS] = new StateVariableData(L"GL_MAX_GEOMETRY_ATOMIC_COUNTERS", ap_glGetIntegerv, GL_MAX_GEOMETRY_ATOMIC_COUNTERS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_FRAGMENT_ATOMIC_COUNTERS] = new StateVariableData(L"GL_MAX_FRAGMENT_ATOMIC_COUNTERS", ap_glGetIntegerv, GL_MAX_FRAGMENT_ATOMIC_COUNTERS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMBINED_ATOMIC_COUNTERS] = new StateVariableData(L"GL_MAX_COMBINED_ATOMIC_COUNTERS", ap_glGetIntegerv, GL_MAX_COMBINED_ATOMIC_COUNTERS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_ATOMIC_COUNTER_BUFFER_SIZE] = new StateVariableData(L"GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE", ap_glGetIntegerv, GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS] = new StateVariableData(L"GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS", ap_glGetIntegerv, GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_IMAGE_UNITS] = new StateVariableData(L"GL_MAX_IMAGE_UNITS", ap_glGetIntegerv, GL_MAX_IMAGE_UNITS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_IMAGE_SAMPLES] = new StateVariableData(L"GL_MAX_IMAGE_SAMPLES", ap_glGetIntegerv, GL_MAX_IMAGE_SAMPLES, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_IMAGE_UNIFORMS] = new StateVariableData(L"GL_MAX_VERTEX_IMAGE_UNIFORMS", ap_glGetIntegerv, GL_MAX_VERTEX_IMAGE_UNIFORMS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_CONTROL_IMAGE_UNIFORMS] = new StateVariableData(L"GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS", ap_glGetIntegerv, GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS] = new StateVariableData(L"GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS", ap_glGetIntegerv, GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_GEOMETRY_IMAGE_UNIFORMS] = new StateVariableData(L"GL_MAX_GEOMETRY_IMAGE_UNIFORMS", ap_glGetIntegerv, GL_MAX_GEOMETRY_IMAGE_UNIFORMS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_FRAGMENT_IMAGE_UNIFORMS] = new StateVariableData(L"GL_MAX_FRAGMENT_IMAGE_UNIFORMS", ap_glGetIntegerv, GL_MAX_FRAGMENT_IMAGE_UNIFORMS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMBINED_IMAGE_UNIFORMS] = new StateVariableData(L"GL_MAX_COMBINED_IMAGE_UNIFORMS", ap_glGetIntegerv, GL_MAX_COMBINED_IMAGE_UNIFORMS, 0, AP_OPENGL_STATE_VAR);

    // OpenGL 4.3
    _stateVariablesData[apGL_NUM_SHADING_LANGUAGE_VERSIONS] = new StateVariableData(L"GL_NUM_SHADING_LANGUAGE_VERSIONS", ap_glGetIntegerv, GL_NUM_SHADING_LANGUAGE_VERSIONS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PRIMITIVE_RESTART_FIXED_INDEX] = new StateVariableData(L"GL_PRIMITIVE_RESTART_FIXED_INDEX", ap_glIsEnabled, GL_PRIMITIVE_RESTART_FIXED_INDEX, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_ELEMENT_INDEX] = new StateVariableData(L"GL_MAX_ELEMENT_INDEX", ap_glGetIntegerv, GL_MAX_ELEMENT_INDEX, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMPUTE_UNIFORM_BLOCKS] = new StateVariableData(L"GL_MAX_COMPUTE_UNIFORM_BLOCKS", ap_glGetIntegerv, GL_MAX_COMPUTE_UNIFORM_BLOCKS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS] = new StateVariableData(L"GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS", ap_glGetIntegerv, GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMPUTE_IMAGE_UNIFORMS] = new StateVariableData(L"GL_MAX_COMPUTE_IMAGE_UNIFORMS", ap_glGetIntegerv, GL_MAX_COMPUTE_IMAGE_UNIFORMS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMPUTE_SHARED_MEMORY_SIZE] = new StateVariableData(L"GL_MAX_COMPUTE_SHARED_MEMORY_SIZE", ap_glGetIntegerv, GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMPUTE_UNIFORM_COMPONENTS] = new StateVariableData(L"GL_MAX_COMPUTE_UNIFORM_COMPONENTS", ap_glGetIntegerv, GL_MAX_COMPUTE_UNIFORM_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS] = new StateVariableData(L"GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS", ap_glGetIntegerv, GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMPUTE_ATOMIC_COUNTERS] = new StateVariableData(L"GL_MAX_COMPUTE_ATOMIC_COUNTERS", ap_glGetIntegerv, GL_MAX_COMPUTE_ATOMIC_COUNTERS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS] = new StateVariableData(L"GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS", ap_glGetIntegerv, GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS] = new StateVariableData(L"GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS", ap_glGetIntegerv, GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DISPATCH_INDIRECT_BUFFER_BINDING] = new StateVariableData(L"GL_DISPATCH_INDIRECT_BUFFER_BINDING", ap_glGetIntegerv, GL_DISPATCH_INDIRECT_BUFFER_BINDING, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DEBUG_OUTPUT_SYNCHRONOUS] = new StateVariableData(L"GL_DEBUG_OUTPUT_SYNCHRONOUS", ap_glIsEnabled, GL_DEBUG_OUTPUT_SYNCHRONOUS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH] = new StateVariableData(L"GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH", ap_glGetIntegerv, GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DEBUG_CALLBACK_FUNCTION] = new StateVariableData(L"GL_DEBUG_CALLBACK_FUNCTION", ap_glGetPointerv, GL_DEBUG_CALLBACK_FUNCTION, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DEBUG_CALLBACK_USER_PARAM] = new StateVariableData(L"GL_DEBUG_CALLBACK_USER_PARAM", ap_glGetPointerv, GL_DEBUG_CALLBACK_USER_PARAM, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_DEBUG_MESSAGE_LENGTH] = new StateVariableData(L"GL_MAX_DEBUG_MESSAGE_LENGTH", ap_glGetIntegerv, GL_MAX_DEBUG_MESSAGE_LENGTH, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_DEBUG_LOGGED_MESSAGES] = new StateVariableData(L"GL_MAX_DEBUG_LOGGED_MESSAGES", ap_glGetIntegerv, GL_MAX_DEBUG_LOGGED_MESSAGES, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DEBUG_LOGGED_MESSAGES] = new StateVariableData(L"GL_DEBUG_LOGGED_MESSAGES", ap_glGetIntegerv, GL_DEBUG_LOGGED_MESSAGES, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_DEBUG_GROUP_STACK_DEPTH] = new StateVariableData(L"GL_MAX_DEBUG_GROUP_STACK_DEPTH", ap_glGetIntegerv, GL_MAX_DEBUG_GROUP_STACK_DEPTH, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DEBUG_GROUP_STACK_DEPTH] = new StateVariableData(L"GL_DEBUG_GROUP_STACK_DEPTH", ap_glGetIntegerv, GL_DEBUG_GROUP_STACK_DEPTH, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_LABEL_LENGTH] = new StateVariableData(L"GL_MAX_LABEL_LENGTH", ap_glGetIntegerv, GL_MAX_LABEL_LENGTH, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DEBUG_OUTPUT] = new StateVariableData(L"GL_DEBUG_OUTPUT", ap_glIsEnabled, GL_DEBUG_OUTPUT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_UNIFORM_LOCATIONS] = new StateVariableData(L"GL_MAX_UNIFORM_LOCATIONS", ap_glGetIntegerv, GL_MAX_UNIFORM_LOCATIONS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_FRAMEBUFFER_WIDTH] = new StateVariableData(L"GL_MAX_FRAMEBUFFER_WIDTH", ap_glGetIntegerv, GL_MAX_FRAMEBUFFER_WIDTH, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_FRAMEBUFFER_HEIGHT] = new StateVariableData(L"GL_MAX_FRAMEBUFFER_HEIGHT", ap_glGetIntegerv, GL_MAX_FRAMEBUFFER_HEIGHT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_FRAMEBUFFER_LAYERS] = new StateVariableData(L"GL_MAX_FRAMEBUFFER_LAYERS", ap_glGetIntegerv, GL_MAX_FRAMEBUFFER_LAYERS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_FRAMEBUFFER_SAMPLES] = new StateVariableData(L"GL_MAX_FRAMEBUFFER_SAMPLES", ap_glGetIntegerv, GL_MAX_FRAMEBUFFER_SAMPLES, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_SHADER_STORAGE_BUFFER_BINDING] = new StateVariableData(L"GL_SHADER_STORAGE_BUFFER_BINDING", ap_glGetIntegerv, GL_SHADER_STORAGE_BUFFER_BINDING, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_SHADER_STORAGE_BLOCKS] = new StateVariableData(L"GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS", ap_glGetIntegerv, GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS] = new StateVariableData(L"GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS", ap_glGetIntegerv, GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS] = new StateVariableData(L"GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS", ap_glGetIntegerv, GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS] = new StateVariableData(L"GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS", ap_glGetIntegerv, GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS] = new StateVariableData(L"GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS", ap_glGetIntegerv, GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS] = new StateVariableData(L"GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS", ap_glGetIntegerv, GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMBINED_SHADER_STORAGE_BLOCKS] = new StateVariableData(L"GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS", ap_glGetIntegerv, GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_SHADER_STORAGE_BUFFER_BINDINGS] = new StateVariableData(L"GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS", ap_glGetIntegerv, GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_SHADER_STORAGE_BLOCK_SIZE] = new StateVariableData(L"GL_MAX_SHADER_STORAGE_BLOCK_SIZE", ap_glGetIntegerv, GL_MAX_SHADER_STORAGE_BLOCK_SIZE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT] = new StateVariableData(L"GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT", ap_glGetIntegerv, GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES] = new StateVariableData(L"GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES", ap_glGetIntegerv, GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_BUFFER_OFFSET_ALIGNMENT] = new StateVariableData(L"GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT", ap_glGetIntegerv, GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET] = new StateVariableData(L"GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET", ap_glGetIntegerv, GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_ATTRIB_BINDINGS] = new StateVariableData(L"GL_MAX_VERTEX_ATTRIB_BINDINGS", ap_glGetIntegerv, GL_MAX_VERTEX_ATTRIB_BINDINGS, 0, AP_OPENGL_STATE_VAR);


    //////////////////////////////////////////////////////////////////////////
    // GL_NV_primitive_restart
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_PRIMITIVE_RESTART_NV] = new StateVariableData(L"GL_PRIMITIVE_RESTART_NV", ap_glIsEnabled, GL_PRIMITIVE_RESTART_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PRIMITIVE_RESTART_INDEX_NV] = new StateVariableData(L"GL_PRIMITIVE_RESTART_INDEX_NV", ap_glGetIntegerv, GL_PRIMITIVE_RESTART_INDEX_NV, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_HP_occlusion_test extension
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_OCCLUSION_TEST_HP] = new StateVariableData(L"GL_OCCLUSION_TEST_HP", ap_glIsEnabled, GL_OCCLUSION_TEST_HP, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_OCCLUSION_TEST_RESULT_HP] = new StateVariableData(L"GL_OCCLUSION_TEST_RESULT_HP", ap_glGetBooleanv, GL_OCCLUSION_TEST_RESULT_HP, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_occlusion_query
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_CURRENT_OCCLUSION_QUERY_ID_NV] = new StateVariableData(L"GL_CURRENT_OCCLUSION_QUERY_ID_NV", ap_glGetIntegerv, GL_CURRENT_OCCLUSION_QUERY_ID_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PIXEL_COUNTER_BITS_NV] = new StateVariableData(L"GL_PIXEL_COUNTER_BITS_NV", ap_glGetIntegerv, GL_PIXEL_COUNTER_BITS_NV, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_occlusion_query extension
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_CURRENT_QUERY_ARB] = new StateVariableData(L"GL_CURRENT_QUERY_ARB", ap_glGetQueryiv, GL_SAMPLES_PASSED_ARB, GL_CURRENT_QUERY_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_QUERY_COUNTER_BITS_ARB] = new StateVariableData(L"GL_QUERY_COUNTER_BITS_ARB", ap_glGetQueryiv, GL_SAMPLES_PASSED_ARB, GL_QUERY_COUNTER_BITS_ARB, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_texture_cube_map extension
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_TEXTURE_CUBE_MAP_ARB] = new StateVariableData(L"GL_TEXTURE_CUBE_MAP_ARB", ap_glIsEnabled, GL_TEXTURE_CUBE_MAP_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_BINDING_CUBE_MAP_ARB] = new StateVariableData(L"GL_TEXTURE_BINDING_CUBE_MAP_ARB", ap_glGetIntegerv, GL_TEXTURE_BINDING_CUBE_MAP_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB] = new StateVariableData(L"GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB", ap_glGetIntegerv, GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_texture_compression extension
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_TEXTURE_COMPRESSION_HINT_ARB] = new StateVariableData(L"GL_TEXTURE_COMPRESSION_HINT_ARB", GL_TEXTURE_COMPRESSION_HINT_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB] = new StateVariableData(L"GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB", ap_glGetIntegerv, GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_COMPRESSED_TEXTURE_FORMATS_ARB] = new StateVariableData(L"GL_COMPRESSED_TEXTURE_FORMATS_ARB", GL_COMPRESSED_TEXTURE_FORMATS_ARB, GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB, AP_OPENGL_STATE_VAR); // TO_DO: See comment above for GL_COMPRESSED_TEXTURE_FORMATS

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_vertex_buffer_object extension
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_ARRAY_BUFFER_BINDING_ARB] = new StateVariableData(L"GL_ARRAY_BUFFER_BINDING_ARB", ap_glGetIntegerv, GL_ARRAY_BUFFER_BINDING_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ARRAY_BUFFER_BINDING_ARB] = new StateVariableData(L"GL_VERTEX_ARRAY_BUFFER_BINDING_ARB", ap_glGetIntegerv, GL_VERTEX_ARRAY_BUFFER_BINDING_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_NORMAL_ARRAY_BUFFER_BINDING_ARB] = new StateVariableData(L"GL_NORMAL_ARRAY_BUFFER_BINDING_ARB", ap_glGetIntegerv, GL_NORMAL_ARRAY_BUFFER_BINDING_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_COLOR_ARRAY_BUFFER_BINDING_ARB] = new StateVariableData(L"GL_COLOR_ARRAY_BUFFER_BINDING_ARB", ap_glGetIntegerv, GL_COLOR_ARRAY_BUFFER_BINDING_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_INDEX_ARRAY_BUFFER_BINDING_ARB] = new StateVariableData(L"GL_INDEX_ARRAY_BUFFER_BINDING_ARB", ap_glGetIntegerv, GL_INDEX_ARRAY_BUFFER_BINDING_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB] = new StateVariableData(L"GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB", ap_glGetIntegerv, GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB] = new StateVariableData(L"GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB", ap_glGetIntegerv, GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB] = new StateVariableData(L"GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB", ap_glGetIntegerv, GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB] = new StateVariableData(L"GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB", ap_glGetIntegerv, GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_WEIGHT_ARRAY_BUFFER_BINDING_ARB] = new StateVariableData(L"GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB", ap_glGetIntegerv, GL_WEIGHT_ARRAY_BUFFER_BINDING_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ELEMENT_ARRAY_BUFFER_BINDING_ARB] = new StateVariableData(L"GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB", ap_glGetIntegerv, GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB, 0, AP_OPENGL_STATE_VAR);
    // See comment in apOpenGLStateVariableId.h under GL_ARB_vertex_program
    //_stateVariablesData[apGL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB] = new StateVariableData(L"GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB", ap_glGetIntegerv, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB, 0, AP_OPENGL_STATE_VAR);

    _stateVariablesData[apGL_ARRAY_BUFFER_GL_BUFFER_USAGE_ARB] = new StateVariableData(L"GL_ARRAY_BUFFER - GL_BUFFER_USAGE_ARB", ap_glGetBufferParameterivARB, GL_ARRAY_BUFFER, GL_BUFFER_USAGE_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ARRAY_BUFFER_GL_BUFFER_MAPPED_ARB] = new StateVariableData(L"GL_ARRAY_BUFFER - GL_BUFFER_MAPPED_ARB", ap_glGetBufferParameterivARB, GL_ARRAY_BUFFER, GL_BUFFER_MAPPED_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ARRAY_BUFFER_GL_BUFFER_SIZE_ARB] = new StateVariableData(L"GL_ARRAY_BUFFER - GL_BUFFER_SIZE_ARB", ap_glGetBufferParameterivARB, GL_ARRAY_BUFFER, GL_BUFFER_SIZE_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ARRAY_BUFFER_GL_BUFFER_ACCESS_ARB] = new StateVariableData(L"GL_ARRAY_BUFFER - GL_BUFFER_ACCESS_ARB", ap_glGetBufferParameterivARB, GL_ARRAY_BUFFER, GL_BUFFER_ACCESS_ARB, AP_OPENGL_STATE_VAR);

    _stateVariablesData[apGL_ELEMENT_ARRAY_BUFFER_GL_BUFFER_USAGE_ARB] = new StateVariableData(L"GL_ELEMENT_ARRAY_BUFFER - GL_BUFFER_USAGE_ARB", ap_glGetBufferParameterivARB, GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_USAGE_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ELEMENT_ARRAY_BUFFER_GL_BUFFER_MAPPED_ARB] = new StateVariableData(L"GL_ELEMENT_ARRAY_BUFFER - GL_BUFFER_MAPPED_ARB", ap_glGetBufferParameterivARB, GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_MAPPED_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ELEMENT_ARRAY_BUFFER_GL_BUFFER_SIZE_ARB] = new StateVariableData(L"GL_ELEMENT_ARRAY_BUFFER - GL_BUFFER_SIZE_ARB", ap_glGetBufferParameterivARB, GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ELEMENT_ARRAY_BUFFER_GL_BUFFER_ACCESS_ARB] = new StateVariableData(L"GL_ELEMENT_ARRAY_BUFFER - GL_BUFFER_ACCESS_ARB", ap_glGetBufferParameterivARB, GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_ACCESS_ARB, AP_OPENGL_STATE_VAR);


    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_vertex_blend extension
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_CURRENT_WEIGHT_ARB] = new StateVariableData(L"GL_CURRENT_WEIGHT_ARB", ap_glGetFloatv, GL_CURRENT_WEIGHT_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_WEIGHT_ARRAY_ARB] = new StateVariableData(L"GL_WEIGHT_ARRAY_ARB", ap_glIsEnabled, GL_WEIGHT_ARRAY_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_WEIGHT_ARRAY_TYPE_ARB] = new StateVariableData(L"GL_WEIGHT_ARRAY_TYPE_ARB", GL_WEIGHT_ARRAY_TYPE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_WEIGHT_ARRAY_SIZE_ARB] = new StateVariableData(L"GL_WEIGHT_ARRAY_SIZE_ARB", ap_glGetIntegerv, GL_WEIGHT_ARRAY_SIZE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_WEIGHT_ARRAY_STRIDE_ARB] = new StateVariableData(L"GL_WEIGHT_ARRAY_STRIDE_ARB", ap_glGetIntegerv, GL_WEIGHT_ARRAY_STRIDE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_WEIGHT_ARRAY_POINTER_ARB] = new StateVariableData(L"GL_WEIGHT_ARRAY_POINTER_ARB", ap_glGetIntegerv, GL_WEIGHT_ARRAY_POINTER_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ACTIVE_VERTEX_UNITS_ARB] = new StateVariableData(L"GL_ACTIVE_VERTEX_UNITS_ARB", ap_glGetIntegerv, GL_ACTIVE_VERTEX_UNITS_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_BLEND_ARB] = new StateVariableData(L"GL_VERTEX_BLEND_ARB", ap_glIsEnabled, GL_VERTEX_BLEND_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_UNITS_ARB] = new StateVariableData(L"GL_MAX_VERTEX_UNITS_ARB", ap_glGetIntegerv, GL_MAX_VERTEX_UNITS_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW0_ARB] = new StateVariableData(L"GL_MODELVIEW0_ARB", ap_glGetFloatv, GL_MODELVIEW0_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW1_ARB] = new StateVariableData(L"GL_MODELVIEW1_ARB", ap_glGetFloatv, GL_MODELVIEW1_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW2_ARB] = new StateVariableData(L"GL_MODELVIEW2_ARB", ap_glGetFloatv, GL_MODELVIEW2_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW3_ARB] = new StateVariableData(L"GL_MODELVIEW3_ARB", ap_glGetFloatv, GL_MODELVIEW3_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW4_ARB] = new StateVariableData(L"GL_MODELVIEW4_ARB", ap_glGetFloatv, GL_MODELVIEW4_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW5_ARB] = new StateVariableData(L"GL_MODELVIEW5_ARB", ap_glGetFloatv, GL_MODELVIEW5_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW6_ARB] = new StateVariableData(L"GL_MODELVIEW6_ARB", ap_glGetFloatv, GL_MODELVIEW6_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW7_ARB] = new StateVariableData(L"GL_MODELVIEW7_ARB", ap_glGetFloatv, GL_MODELVIEW7_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW8_ARB] = new StateVariableData(L"GL_MODELVIEW8_ARB", ap_glGetFloatv, GL_MODELVIEW8_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW9_ARB] = new StateVariableData(L"GL_MODELVIEW9_ARB", ap_glGetFloatv, GL_MODELVIEW9_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW10_ARB] = new StateVariableData(L"GL_MODELVIEW10_ARB", ap_glGetFloatv, GL_MODELVIEW10_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW11_ARB] = new StateVariableData(L"GL_MODELVIEW11_ARB", ap_glGetFloatv, GL_MODELVIEW11_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW12_ARB] = new StateVariableData(L"GL_MODELVIEW12_ARB", ap_glGetFloatv, GL_MODELVIEW12_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW13_ARB] = new StateVariableData(L"GL_MODELVIEW13_ARB", ap_glGetFloatv, GL_MODELVIEW13_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW14_ARB] = new StateVariableData(L"GL_MODELVIEW14_ARB", ap_glGetFloatv, GL_MODELVIEW14_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW15_ARB] = new StateVariableData(L"GL_MODELVIEW15_ARB", ap_glGetFloatv, GL_MODELVIEW15_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW16_ARB] = new StateVariableData(L"GL_MODELVIEW16_ARB", ap_glGetFloatv, GL_MODELVIEW16_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW17_ARB] = new StateVariableData(L"GL_MODELVIEW17_ARB", ap_glGetFloatv, GL_MODELVIEW17_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW18_ARB] = new StateVariableData(L"GL_MODELVIEW18_ARB", ap_glGetFloatv, GL_MODELVIEW18_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW19_ARB] = new StateVariableData(L"GL_MODELVIEW19_ARB", ap_glGetFloatv, GL_MODELVIEW19_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW20_ARB] = new StateVariableData(L"GL_MODELVIEW20_ARB", ap_glGetFloatv, GL_MODELVIEW20_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW21_ARB] = new StateVariableData(L"GL_MODELVIEW21_ARB", ap_glGetFloatv, GL_MODELVIEW21_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW22_ARB] = new StateVariableData(L"GL_MODELVIEW22_ARB", ap_glGetFloatv, GL_MODELVIEW22_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW23_ARB] = new StateVariableData(L"GL_MODELVIEW23_ARB", ap_glGetFloatv, GL_MODELVIEW23_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW24_ARB] = new StateVariableData(L"GL_MODELVIEW24_ARB", ap_glGetFloatv, GL_MODELVIEW24_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW25_ARB] = new StateVariableData(L"GL_MODELVIEW25_ARB", ap_glGetFloatv, GL_MODELVIEW25_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW26_ARB] = new StateVariableData(L"GL_MODELVIEW26_ARB", ap_glGetFloatv, GL_MODELVIEW26_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW27_ARB] = new StateVariableData(L"GL_MODELVIEW27_ARB", ap_glGetFloatv, GL_MODELVIEW27_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW28_ARB] = new StateVariableData(L"GL_MODELVIEW28_ARB", ap_glGetFloatv, GL_MODELVIEW28_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW29_ARB] = new StateVariableData(L"GL_MODELVIEW29_ARB", ap_glGetFloatv, GL_MODELVIEW29_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW30_ARB] = new StateVariableData(L"GL_MODELVIEW30_ARB", ap_glGetFloatv, GL_MODELVIEW30_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MODELVIEW31_ARB] = new StateVariableData(L"GL_MODELVIEW31_ARB", ap_glGetFloatv, GL_MODELVIEW31_ARB, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // EXT_texture3D extension
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_TEXTURE_3D_EXT] = new StateVariableData(L"GL_TEXTURE_3D_EXT", ap_glIsEnabled, GL_TEXTURE_3D_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PACK_SKIP_IMAGES_EXT] = new StateVariableData(L"GL_PACK_SKIP_IMAGES_EXT", ap_glGetIntegerv, GL_PACK_SKIP_IMAGES_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PACK_IMAGE_HEIGHT_EXT] = new StateVariableData(L"GL_PACK_IMAGE_HEIGHT_EXT", ap_glGetIntegerv, GL_PACK_IMAGE_HEIGHT_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_UNPACK_SKIP_IMAGES_EXT] = new StateVariableData(L"GL_UNPACK_SKIP_IMAGES_EXT", ap_glGetIntegerv, GL_UNPACK_SKIP_IMAGES_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_UNPACK_IMAGE_HEIGHT_EXT] = new StateVariableData(L"GL_UNPACK_IMAGE_HEIGHT_EXT", ap_glGetIntegerv, GL_UNPACK_IMAGE_HEIGHT_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_3D_TEXTURE_SIZE_EXT] = new StateVariableData(L"GL_MAX_3D_TEXTURE_SIZE_EXT", ap_glGetIntegerv, GL_MAX_3D_TEXTURE_SIZE_EXT, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_vertex_program
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_VERTEX_PROGRAM_ARB] = new StateVariableData(L"GL_VERTEX_PROGRAM_ARB", ap_glIsEnabled, GL_VERTEX_PROGRAM_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_PROGRAM_POINT_SIZE_ARB] = new StateVariableData(L"GL_VERTEX_PROGRAM_POINT_SIZE_ARB", ap_glIsEnabled, GL_VERTEX_PROGRAM_POINT_SIZE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_PROGRAM_TWO_SIDE_ARB] = new StateVariableData(L"GL_VERTEX_PROGRAM_TWO_SIDE_ARB", ap_glIsEnabled, GL_VERTEX_PROGRAM_TWO_SIDE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_BINDING_ARB_vertex_program] = new StateVariableData(L"GL_PROGRAM_BINDING_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_BINDING_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_LENGTH_ARB_vertex_program] = new StateVariableData(L"GL_PROGRAM_LENGTH_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_LENGTH_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_FORMAT_ARB_vertex_program] = new StateVariableData(L"GL_PROGRAM_FORMAT_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_INSTRUCTIONS_ARB] = new StateVariableData(L"GL_PROGRAM_INSTRUCTIONS_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_INSTRUCTIONS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_TEMPORARIES_ARB] = new StateVariableData(L"GL_PROGRAM_TEMPORARIES_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_TEMPORARIES_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_PARAMETERS_ARB] = new StateVariableData(L"GL_PROGRAM_PARAMETERS_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_PARAMETERS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_ATTRIBS_ARB] = new StateVariableData(L"GL_PROGRAM_ATTRIBS_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_ATTRIBS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_ADDRESS_REGISTERS_ARB] = new StateVariableData(L"GL_PROGRAM_ADDRESS_REGISTERS_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_ADDRESS_REGISTERS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_NATIVE_INSTRUCTIONS_ARB] = new StateVariableData(L"GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_NATIVE_TEMPORARIES_ARB] = new StateVariableData(L"GL_PROGRAM_NATIVE_TEMPORARIES_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_NATIVE_TEMPORARIES_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_NATIVE_PARAMETERS_ARB] = new StateVariableData(L"GL_PROGRAM_NATIVE_PARAMETERS_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_NATIVE_PARAMETERS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_NATIVE_ATTRIBS_ARB] = new StateVariableData(L"GL_PROGRAM_NATIVE_ATTRIBS_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_NATIVE_ATTRIBS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB] = new StateVariableData(L"GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_UNDER_NATIVE_LIMITS_ARB] = new StateVariableData(L"GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CURRENT_MATRIX_ARB] = new StateVariableData(L"GL_CURRENT_MATRIX_ARB", ap_glGetFloatv, 4, 4, GL_CURRENT_MATRIX_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CURRENT_MATRIX_STACK_DEPTH_ARB] = new StateVariableData(L"GL_CURRENT_MATRIX_STACK_DEPTH_ARB", ap_glGetIntegerv, GL_CURRENT_MATRIX_STACK_DEPTH_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_ENV_PARAMETERS_ARB] = new StateVariableData(L"GL_MAX_PROGRAM_ENV_PARAMETERS_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_ENV_PARAMETERS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB] = new StateVariableData(L"GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_MATRICES_ARB] = new StateVariableData(L"GL_MAX_PROGRAM_MATRICES_ARB", ap_glGetIntegerv, GL_MAX_PROGRAM_MATRICES_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB] = new StateVariableData(L"GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB", ap_glGetIntegerv, GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_INSTRUCTIONS_ARB] = new StateVariableData(L"GL_MAX_PROGRAM_INSTRUCTIONS_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_INSTRUCTIONS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_TEMPORARIES_ARB] = new StateVariableData(L"GL_MAX_PROGRAM_TEMPORARIES_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_TEMPORARIES_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_PARAMETERS_ARB] = new StateVariableData(L"GL_MAX_PROGRAM_PARAMETERS_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_PARAMETERS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_ATTRIBS_ARB] = new StateVariableData(L"GL_MAX_PROGRAM_ATTRIBS_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_ATTRIBS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB] = new StateVariableData(L"GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB] = new StateVariableData(L"GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB] = new StateVariableData(L"GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB] = new StateVariableData(L"GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB] = new StateVariableData(L"GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB] = new StateVariableData(L"GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB - vertex program", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_ARB, GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_fragment_program
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_FRAGMENT_PROGRAM_ARB] = new StateVariableData(L"GL_FRAGMENT_PROGRAM_ARB", ap_glIsEnabled , GL_FRAGMENT_PROGRAM_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_ALU_INSTRUCTIONS_ARB] = new StateVariableData(L"GL_PROGRAM_ALU_INSTRUCTIONS_ARB - fragment program", ap_glGetProgramivARB, GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_ALU_INSTRUCTIONS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_TEX_INSTRUCTIONS_ARB] = new StateVariableData(L"GL_PROGRAM_TEX_INSTRUCTIONS_ARB - fragment program", ap_glGetProgramivARB, GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_TEX_INSTRUCTIONS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_TEX_INDIRECTIONS_ARB] = new StateVariableData(L"GL_PROGRAM_TEX_INDIRECTIONS_ARB - fragment program", ap_glGetProgramivARB, GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_TEX_INDIRECTIONS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB] = new StateVariableData(L"GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB - fragment program", ap_glGetProgramivARB, GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB] = new StateVariableData(L"GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB - fragment program", ap_glGetProgramivARB, GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB] = new StateVariableData(L"GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB - fragment program", ap_glGetProgramivARB, GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TEXTURE_COORDS_ARB] = new StateVariableData(L"GL_MAX_TEXTURE_COORDS_ARB", ap_glGetIntegerv, GL_MAX_TEXTURE_COORDS_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TEXTURE_IMAGE_UNITS_ARB] = new StateVariableData(L"GL_MAX_TEXTURE_IMAGE_UNITS_ARB", ap_glGetIntegerv, GL_MAX_TEXTURE_IMAGE_UNITS_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB] = new StateVariableData(L"GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB - fragment program", ap_glGetProgramivARB, GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB] = new StateVariableData(L"GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB - fragment program", ap_glGetProgramivARB, GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB] = new StateVariableData(L"GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB - fragment program", ap_glGetProgramivARB, GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB] = new StateVariableData(L"GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB - fragment program", ap_glGetProgramivARB, GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB] = new StateVariableData(L"GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB - fragment program", ap_glGetProgramivARB, GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB] = new StateVariableData(L"GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB - fragment program", ap_glGetProgramivARB, GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_vertex_shader
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_MAX_VERTEX_ATTRIBS_ARB] = new StateVariableData(L"GL_MAX_VERTEX_ATTRIBS_ARB", ap_glGetIntegerv, GL_MAX_VERTEX_ATTRIBS_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB] = new StateVariableData(L"GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB", ap_glGetIntegerv, GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VARYING_FLOATS_ARB] = new StateVariableData(L"GL_MAX_VARYING_FLOATS_ARB", ap_glGetIntegerv, GL_MAX_VARYING_FLOATS_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB] = new StateVariableData(L"GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB", ap_glGetIntegerv, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB] = new StateVariableData(L"GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB", ap_glGetIntegerv, GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_shader_objects
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_PROGRAM_OBJECT_ARB] = new StateVariableData(L"GL_PROGRAM_OBJECT_ARB", ap_glGetHandleARB, GL_PROGRAM_OBJECT_ARB, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_fragment_shader
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB] = new StateVariableData(L"GL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB", GL_FRAGMENT_SHADER_DERIVATIVE_HINT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB] = new StateVariableData(L"GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB", ap_glGetIntegerv, GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_vertex_program
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_VERTEX_PROGRAM_NV] = new StateVariableData(L"GL_VERTEX_PROGRAM_NV", ap_glIsEnabled, GL_VERTEX_PROGRAM_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_PROGRAM_POINT_SIZE_NV] = new StateVariableData(L"GL_VERTEX_PROGRAM_POINT_SIZE_NV", ap_glIsEnabled, GL_VERTEX_PROGRAM_POINT_SIZE_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_PROGRAM_TWO_SIDE_NV] = new StateVariableData(L"GL_VERTEX_PROGRAM_TWO_SIDE_NV", ap_glIsEnabled, GL_VERTEX_PROGRAM_TWO_SIDE_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_PROGRAM_BINDING_NV] = new StateVariableData(L"GL_VERTEX_PROGRAM_BINDING_NV", ap_glGetIntegerv, GL_VERTEX_PROGRAM_BINDING_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ATTRIB_ARRAY0_NV] = new StateVariableData(L"GL_VERTEX_ATTRIB_ARRAY0_NV", ap_glIsEnabled, GL_VERTEX_ATTRIB_ARRAY0_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ATTRIB_ARRAY1_NV] = new StateVariableData(L"GL_VERTEX_ATTRIB_ARRAY1_NV", ap_glIsEnabled, GL_VERTEX_ATTRIB_ARRAY1_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ATTRIB_ARRAY2_NV] = new StateVariableData(L"GL_VERTEX_ATTRIB_ARRAY2_NV", ap_glIsEnabled, GL_VERTEX_ATTRIB_ARRAY2_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ATTRIB_ARRAY3_NV] = new StateVariableData(L"GL_VERTEX_ATTRIB_ARRAY3_NV", ap_glIsEnabled, GL_VERTEX_ATTRIB_ARRAY3_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ATTRIB_ARRAY4_NV] = new StateVariableData(L"GL_VERTEX_ATTRIB_ARRAY4_NV", ap_glIsEnabled, GL_VERTEX_ATTRIB_ARRAY4_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ATTRIB_ARRAY5_NV] = new StateVariableData(L"GL_VERTEX_ATTRIB_ARRAY5_NV", ap_glIsEnabled, GL_VERTEX_ATTRIB_ARRAY5_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ATTRIB_ARRAY6_NV] = new StateVariableData(L"GL_VERTEX_ATTRIB_ARRAY6_NV", ap_glIsEnabled, GL_VERTEX_ATTRIB_ARRAY6_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ATTRIB_ARRAY7_NV] = new StateVariableData(L"GL_VERTEX_ATTRIB_ARRAY7_NV", ap_glIsEnabled, GL_VERTEX_ATTRIB_ARRAY7_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ATTRIB_ARRAY8_NV] = new StateVariableData(L"GL_VERTEX_ATTRIB_ARRAY8_NV", ap_glIsEnabled, GL_VERTEX_ATTRIB_ARRAY8_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ATTRIB_ARRAY9_NV] = new StateVariableData(L"GL_VERTEX_ATTRIB_ARRAY9_NV", ap_glIsEnabled, GL_VERTEX_ATTRIB_ARRAY9_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ATTRIB_ARRAY10_NV] = new StateVariableData(L"GL_VERTEX_ATTRIB_ARRAY10_NV", ap_glIsEnabled, GL_VERTEX_ATTRIB_ARRAY10_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ATTRIB_ARRAY11_NV] = new StateVariableData(L"GL_VERTEX_ATTRIB_ARRAY11_NV", ap_glIsEnabled, GL_VERTEX_ATTRIB_ARRAY11_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ATTRIB_ARRAY12_NV] = new StateVariableData(L"GL_VERTEX_ATTRIB_ARRAY12_NV", ap_glIsEnabled, GL_VERTEX_ATTRIB_ARRAY12_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ATTRIB_ARRAY13_NV] = new StateVariableData(L"GL_VERTEX_ATTRIB_ARRAY13_NV", ap_glIsEnabled, GL_VERTEX_ATTRIB_ARRAY13_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ATTRIB_ARRAY14_NV] = new StateVariableData(L"GL_VERTEX_ATTRIB_ARRAY14_NV", ap_glIsEnabled, GL_VERTEX_ATTRIB_ARRAY14_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ATTRIB_ARRAY15_NV] = new StateVariableData(L"GL_VERTEX_ATTRIB_ARRAY15_NV", ap_glIsEnabled, GL_VERTEX_ATTRIB_ARRAY15_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CURRENT_MATRIX_STACK_DEPTH_NV] = new StateVariableData(L"GL_CURRENT_MATRIX_STACK_DEPTH_NV", ap_glGetIntegerv, GL_CURRENT_MATRIX_STACK_DEPTH_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CURRENT_MATRIX_NV] = new StateVariableData(L"GL_CURRENT_MATRIX_NV", ap_glGetFloatv, 4, 4, GL_CURRENT_MATRIX_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TRACK_MATRIX_STACK_DEPTH_NV] = new StateVariableData(L"GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV", ap_glGetIntegerv, GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TRACK_MATRICES_NV] = new StateVariableData(L"GL_MAX_TRACK_MATRICES_NV", ap_glGetIntegerv, GL_MAX_TRACK_MATRICES_NV, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ATI_fragment_shader
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_FRAGMENT_SHADER_ATI] = new StateVariableData(L"GL_FRAGMENT_SHADER_ATI", ap_glIsEnabled, GL_FRAGMENT_SHADER_ATI, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_vertex_shader
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_VERTEX_SHADER_INSTRUCTIONS_EXT] = new StateVariableData(L"GL_VERTEX_SHADER_INSTRUCTIONS_EXT", ap_glGetIntegerv, GL_VERTEX_SHADER_INSTRUCTIONS_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_SHADER_VARIANTS_EXT] = new StateVariableData(L"GL_VERTEX_SHADER_VARIANTS_EXT", ap_glGetIntegerv, GL_VERTEX_SHADER_VARIANTS_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_SHADER_INVARIANTS_EXT] = new StateVariableData(L"GL_VERTEX_SHADER_INVARIANTS_EXT", ap_glGetIntegerv, GL_VERTEX_SHADER_INVARIANTS_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_SHADER_LOCAL_CONSTANTS_EXT] = new StateVariableData(L"GL_VERTEX_SHADER_LOCAL_CONSTANTS_EXT", ap_glGetIntegerv, GL_VERTEX_SHADER_LOCAL_CONSTANTS_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_SHADER_LOCALS_EXT] = new StateVariableData(L"GL_VERTEX_SHADER_LOCALS_EXT", ap_glGetIntegerv, GL_VERTEX_SHADER_LOCALS_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_SHADER_OPTIMIZED_EXT] = new StateVariableData(L"GL_VERTEX_SHADER_OPTIMIZED_EXT", ap_glGetBooleanv, GL_VERTEX_SHADER_OPTIMIZED_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_SHADER_EXT] = new StateVariableData(L"GL_VERTEX_SHADER_EXT", ap_glIsEnabled, GL_VERTEX_SHADER_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_SHADER_BINDING_EXT] = new StateVariableData(L"GL_VERTEX_SHADER_BINDING_EXT", ap_glGetIntegerv, GL_VERTEX_SHADER_BINDING_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VARIANT_ARRAY_EXT] = new StateVariableData(L"GL_VARIANT_ARRAY_EXT", ap_glIsEnabled, GL_VARIANT_ARRAY_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_SHADER_INSTRUCTIONS_EXT] = new StateVariableData(L"GL_MAX_VERTEX_SHADER_INSTRUCTIONS_EXT", ap_glGetIntegerv, GL_MAX_VERTEX_SHADER_INSTRUCTIONS_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_SHADER_VARIANTS_EXT] = new StateVariableData(L"GL_MAX_VERTEX_SHADER_VARIANTS_EXT", ap_glGetIntegerv, GL_MAX_VERTEX_SHADER_VARIANTS_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_SHADER_INVARIANTS_EXT] = new StateVariableData(L"GL_MAX_VERTEX_SHADER_INVARIANTS_EXT", ap_glGetIntegerv, GL_MAX_VERTEX_SHADER_INVARIANTS_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT] = new StateVariableData(L"GL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT", ap_glGetIntegerv, GL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_SHADER_LOCALS_EXT] = new StateVariableData(L"GL_MAX_VERTEX_SHADER_LOCALS_EXT", ap_glGetIntegerv, GL_MAX_VERTEX_SHADER_LOCALS_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS_EXT] = new StateVariableData(L"GL_MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS_EXT", ap_glGetIntegerv, GL_MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS_EXT] = new StateVariableData(L"GL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS_EXT", ap_glGetIntegerv, GL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT] = new StateVariableData(L"GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT", ap_glGetIntegerv, GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS_EXT] = new StateVariableData(L"GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS_EXT", ap_glGetIntegerv, GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS_EXT] = new StateVariableData(L"GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS_EXT", ap_glGetIntegerv, GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS_EXT, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_texture_shader
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_LO_BIAS_NV] = new StateVariableData(L"GL_LO_BIAS_NV", ap_glGetFloatv, GL_LO_BIAS_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DS_BIAS_NV] = new StateVariableData(L"GL_DS_BIAS_NV", ap_glGetFloatv, GL_DS_BIAS_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DT_BIAS_NV] = new StateVariableData(L"GL_DT_BIAS_NV", ap_glGetFloatv, GL_DT_BIAS_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAGNITUDE_BIAS_NV] = new StateVariableData(L"GL_MAGNITUDE_BIAS_NV", ap_glGetFloatv, GL_MAGNITUDE_BIAS_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VIBRANCE_BIAS_NV] = new StateVariableData(L"GL_VIBRANCE_BIAS_NV", ap_glGetFloatv, GL_VIBRANCE_BIAS_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_HI_SCALE_NV] = new StateVariableData(L"GL_HI_SCALE_NV", ap_glGetFloatv, GL_HI_SCALE_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_LO_SCALE_NV] = new StateVariableData(L"GL_LO_SCALE_NV", ap_glGetFloatv, GL_LO_SCALE_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DS_SCALE_NV] = new StateVariableData(L"GL_DS_SCALE_NV", ap_glGetFloatv, GL_DS_SCALE_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DT_SCALE_NV] = new StateVariableData(L"GL_DT_SCALE_NV", ap_glGetFloatv, GL_DT_SCALE_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAGNITUDE_SCALE_NV] = new StateVariableData(L"GL_MAGNITUDE_SCALE_NV", ap_glGetFloatv, GL_MAGNITUDE_SCALE_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VIBRANCE_SCALE_NV] = new StateVariableData(L"GL_VIBRANCE_SCALE_NV", ap_glGetFloatv, GL_VIBRANCE_SCALE_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_SHADER_NV] = new StateVariableData(L"GL_TEXTURE_SHADER_NV", ap_glIsEnabled, GL_TEXTURE_SHADER_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_SHADER_OPERATION_NV] = new StateVariableData(L"GL_SHADER_OPERATION_NV", ap_glGetTexEnvfv, GL_SHADER_OPERATION_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CULL_MODES_NV] = new StateVariableData(L"GL_CULL_MODES_NV", ap_glGetTexEnviv, GL_CULL_MODES_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV] = new StateVariableData(L"GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV", ap_glGetTexEnvfv, GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PREVIOUS_TEXTURE_INPUT_NV] = new StateVariableData(L"GL_PREVIOUS_TEXTURE_INPUT_NV", ap_glGetTexEnviv, GL_PREVIOUS_TEXTURE_INPUT_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CONST_EYE_NV] = new StateVariableData(L"GL_CONST_EYE_NV", ap_glGetTexEnvfv, GL_CONST_EYE_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_OFFSET_TEXTURE_MATRIX_NV] = new StateVariableData(L"GL_OFFSET_TEXTURE_MATRIX_NV", ap_glGetTexEnvfv, GL_OFFSET_TEXTURE_MATRIX_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_OFFSET_TEXTURE_SCALE_NV] = new StateVariableData(L"GL_OFFSET_TEXTURE_SCALE_NV", ap_glGetTexEnvfv, GL_OFFSET_TEXTURE_SCALE_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_OFFSET_TEXTURE_BIAS_NV] = new StateVariableData(L"GL_OFFSET_TEXTURE_BIAS_NV", ap_glGetTexEnvfv, GL_OFFSET_TEXTURE_BIAS_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_SHADER_CONSISTENT_NV] = new StateVariableData(L"GL_SHADER_CONSISTENT_NV", ap_glGetTexEnviv, GL_SHADER_CONSISTENT_NV, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_texture_shader3
    //////////////////////////////////////////////////////////////////////////
    // State variables are already implemented in GL_NV_texture_shader

    //////////////////////////////////////////////////////////////////////////
    // GL_ATI_text_fragment_shader
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_TEXT_FRAGMENT_SHADER_ATI] = new StateVariableData(L"GL_TEXT_FRAGMENT_SHADER_ATI", ap_glIsEnabled, GL_TEXT_FRAGMENT_SHADER_ATI, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_fragment_program
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_FRAGMENT_PROGRAM_NV] = new StateVariableData(L"GL_FRAGMENT_PROGRAM_NV", ap_glIsEnabled, GL_FRAGMENT_PROGRAM_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_FRAGMENT_PROGRAM_BINDING_NV] = new StateVariableData(L"GL_FRAGMENT_PROGRAM_BINDING_NV", ap_glGetIntegerv, GL_FRAGMENT_PROGRAM_BINDING_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TEXTURE_COORDS_NV] = new StateVariableData(L"GL_MAX_TEXTURE_COORDS_NV", ap_glGetIntegerv, GL_MAX_TEXTURE_COORDS_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TEXTURE_IMAGE_UNITS_NV] = new StateVariableData(L"GL_MAX_TEXTURE_IMAGE_UNITS_NV", ap_glGetIntegerv, GL_MAX_TEXTURE_IMAGE_UNITS_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV] = new StateVariableData(L"GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV", ap_glGetIntegerv, GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_vertex_program2_option
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_MAX_PROGRAM_EXEC_INSTRUCTIONS_NV] = new StateVariableData(L"GL_MAX_PROGRAM_EXEC_INSTRUCTIONS_NV", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_NV, GL_MAX_PROGRAM_EXEC_INSTRUCTIONS_NV, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_PROGRAM_CALL_DEPTH_NV] = new StateVariableData(L"GL_MAX_PROGRAM_CALL_DEPTH_NV", ap_glGetProgramivARB, GL_VERTEX_PROGRAM_NV, GL_MAX_PROGRAM_CALL_DEPTH_NV, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_shader_buffer_load
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_MAX_SHADER_BUFFER_ADDRESS_NV] = new StateVariableData(L"GL_MAX_SHADER_BUFFER_ADDRESS_NV", ap_glGetIntegerui64vNV, GL_MAX_SHADER_BUFFER_ADDRESS_NV, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_vertex_buffer_unified_memory
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV] = new StateVariableData(L"GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV", ap_glIsEnabled, GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ELEMENT_ARRAY_UNIFIED_NV] = new StateVariableData(L"GL_ELEMENT_ARRAY_UNIFIED_NV", ap_glIsEnabled, GL_ELEMENT_ARRAY_UNIFIED_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ARRAY_LENGTH_NV] = new StateVariableData(L"GL_VERTEX_ARRAY_LENGTH_NV", ap_glGetIntegerv, GL_VERTEX_ARRAY_LENGTH_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_NORMAL_ARRAY_LENGTH_NV] = new StateVariableData(L"GL_NORMAL_ARRAY_LENGTH_NV", ap_glGetIntegerv, GL_NORMAL_ARRAY_LENGTH_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_COLOR_ARRAY_LENGTH_NV] = new StateVariableData(L"GL_COLOR_ARRAY_LENGTH_NV", ap_glGetIntegerv, GL_COLOR_ARRAY_LENGTH_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_INDEX_ARRAY_LENGTH_NV] = new StateVariableData(L"GL_INDEX_ARRAY_LENGTH_NV", ap_glGetIntegerv, GL_INDEX_ARRAY_LENGTH_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_EDGE_FLAG_ARRAY_LENGTH_NV] = new StateVariableData(L"GL_EDGE_FLAG_ARRAY_LENGTH_NV", ap_glGetIntegerv, GL_EDGE_FLAG_ARRAY_LENGTH_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_SECONDARY_COLOR_ARRAY_LENGTH_NV] = new StateVariableData(L"GL_SECONDARY_COLOR_ARRAY_LENGTH_NV", ap_glGetIntegerv, GL_SECONDARY_COLOR_ARRAY_LENGTH_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_FOG_COORD_ARRAY_LENGTH_NV] = new StateVariableData(L"GL_FOG_COORD_ARRAY_LENGTH_NV", ap_glGetIntegerv, GL_FOG_COORD_ARRAY_LENGTH_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ELEMENT_ARRAY_LENGTH_NV] = new StateVariableData(L"GL_ELEMENT_ARRAY_LENGTH_NV", ap_glGetIntegerv, GL_ELEMENT_ARRAY_LENGTH_NV, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_debug_output (also AMD and AMDX versions)
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_MAX_DEBUG_MESSAGE_LENGTH_ARB] = new StateVariableData(L"GL_MAX_DEBUG_MESSAGE_LENGTH_ARB", ap_glGetIntegerv, GL_MAX_DEBUG_MESSAGE_LENGTH_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_DEBUG_LOGGED_MESSAGES_ARB] = new StateVariableData(L"GL_MAX_DEBUG_LOGGED_MESSAGES_ARB", ap_glGetIntegerv, GL_MAX_DEBUG_LOGGED_MESSAGES_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DEBUG_LOGGED_MESSAGES_ARB] = new StateVariableData(L"GL_DEBUG_LOGGED_MESSAGES_ARB", ap_glGetIntegerv, GL_DEBUG_LOGGED_MESSAGES_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB] = new StateVariableData(L"GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB", ap_glGetIntegerv, GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DEBUG_CALLBACK_FUNCTION_ARB] = new StateVariableData(L"GL_DEBUG_CALLBACK_USER_PARAM_ARB", ap_glGetPointerv, GL_DEBUG_CALLBACK_USER_PARAM_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DEBUG_CALLBACK_USER_PARAM_ARB] = new StateVariableData(L"GL_DEBUG_CALLBACK_USER_PARAM_ARB", ap_glGetPointerv, GL_DEBUG_CALLBACK_USER_PARAM_ARB, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_draw_buffers
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_MAX_DRAW_BUFFERS_ARB] = new StateVariableData(L"GL_MAX_DRAW_BUFFERS_ARB", ap_glGetIntegerv, GL_MAX_DRAW_BUFFERS_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER0_ARB] = new StateVariableData(L"GL_DRAW_BUFFER0_ARB", GL_DRAW_BUFFER0_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER1_ARB] = new StateVariableData(L"GL_DRAW_BUFFER1_ARB", GL_DRAW_BUFFER1_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER2_ARB] = new StateVariableData(L"GL_DRAW_BUFFER2_ARB", GL_DRAW_BUFFER2_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER3_ARB] = new StateVariableData(L"GL_DRAW_BUFFER3_ARB", GL_DRAW_BUFFER3_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER4_ARB] = new StateVariableData(L"GL_DRAW_BUFFER4_ARB", GL_DRAW_BUFFER4_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER5_ARB] = new StateVariableData(L"GL_DRAW_BUFFER5_ARB", GL_DRAW_BUFFER5_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER6_ARB] = new StateVariableData(L"GL_DRAW_BUFFER6_ARB", GL_DRAW_BUFFER6_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER7_ARB] = new StateVariableData(L"GL_DRAW_BUFFER7_ARB", GL_DRAW_BUFFER7_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER8_ARB] = new StateVariableData(L"GL_DRAW_BUFFER8_ARB", GL_DRAW_BUFFER8_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER9_ARB] = new StateVariableData(L"GL_DRAW_BUFFER9_ARB", GL_DRAW_BUFFER9_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER10_ARB] = new StateVariableData(L"GL_DRAW_BUFFER10_ARB", GL_DRAW_BUFFER10_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER11_ARB] = new StateVariableData(L"GL_DRAW_BUFFER11_ARB", GL_DRAW_BUFFER11_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER12_ARB] = new StateVariableData(L"GL_DRAW_BUFFER12_ARB", GL_DRAW_BUFFER12_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER13_ARB] = new StateVariableData(L"GL_DRAW_BUFFER13_ARB", GL_DRAW_BUFFER13_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER14_ARB] = new StateVariableData(L"GL_DRAW_BUFFER14_ARB", GL_DRAW_BUFFER14_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER15_ARB] = new StateVariableData(L"GL_DRAW_BUFFER15_ARB", GL_DRAW_BUFFER15_ARB, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ATI_draw_buffers
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_MAX_DRAW_BUFFERS_ATI] = new StateVariableData(L"GL_MAX_DRAW_BUFFERS_ATI", GL_MAX_DRAW_BUFFERS_ATI, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER0_ATI] = new StateVariableData(L"GL_DRAW_BUFFER0_ATI", GL_DRAW_BUFFER0_ATI, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER1_ATI] = new StateVariableData(L"GL_DRAW_BUFFER1_ATI", GL_DRAW_BUFFER1_ATI, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER2_ATI] = new StateVariableData(L"GL_DRAW_BUFFER2_ATI", GL_DRAW_BUFFER2_ATI, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER3_ATI] = new StateVariableData(L"GL_DRAW_BUFFER3_ATI", GL_DRAW_BUFFER3_ATI, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER4_ATI] = new StateVariableData(L"GL_DRAW_BUFFER4_ATI", GL_DRAW_BUFFER4_ATI, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER5_ATI] = new StateVariableData(L"GL_DRAW_BUFFER5_ATI", GL_DRAW_BUFFER5_ATI, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER6_ATI] = new StateVariableData(L"GL_DRAW_BUFFER6_ATI", GL_DRAW_BUFFER6_ATI, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER7_ATI] = new StateVariableData(L"GL_DRAW_BUFFER7_ATI", GL_DRAW_BUFFER7_ATI, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER8_ATI] = new StateVariableData(L"GL_DRAW_BUFFER8_ATI", GL_DRAW_BUFFER8_ATI, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER9_ATI] = new StateVariableData(L"GL_DRAW_BUFFER9_ATI", GL_DRAW_BUFFER9_ATI, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER10_ATI] = new StateVariableData(L"GL_DRAW_BUFFER10_ATI", GL_DRAW_BUFFER10_ATI, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER11_ATI] = new StateVariableData(L"GL_DRAW_BUFFER11_ATI", GL_DRAW_BUFFER11_ATI, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER12_ATI] = new StateVariableData(L"GL_DRAW_BUFFER12_ATI", GL_DRAW_BUFFER12_ATI, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER13_ATI] = new StateVariableData(L"GL_DRAW_BUFFER13_ATI", GL_DRAW_BUFFER13_ATI, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER14_ATI] = new StateVariableData(L"GL_DRAW_BUFFER14_ATI", GL_DRAW_BUFFER14_ATI, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_DRAW_BUFFER15_ATI] = new StateVariableData(L"GL_DRAW_BUFFER15_ATI", GL_DRAW_BUFFER15_ATI, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_color_buffer_float
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_RGBA_FLOAT_MODE_ARB] = new StateVariableData(L"GL_RGBA_FLOAT_MODE_ARB", ap_glGetBooleanv, GL_RGBA_FLOAT_MODE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CLAMP_VERTEX_COLOR_ARB] = new StateVariableData(L"GL_CLAMP_VERTEX_COLOR_ARB", GL_CLAMP_VERTEX_COLOR_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CLAMP_FRAGMENT_COLOR_ARB] = new StateVariableData(L"GL_CLAMP_FRAGMENT_COLOR_ARB", GL_CLAMP_FRAGMENT_COLOR_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CLAMP_READ_COLOR_ARB] = new StateVariableData(L"GL_CLAMP_READ_COLOR_ARB", GL_CLAMP_READ_COLOR_ARB, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_point_sprite
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_POINT_SPRITE_ARB] = new StateVariableData(L"GL_POINT_SPRITE_ARB", ap_glIsEnabled, GL_POINT_SPRITE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_COORD_REPLACE_ARB] = new StateVariableData(L"GL_COORD_REPLACE_ARB", ap_glGetTexEnvfv, GL_COORD_REPLACE_ARB, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_stencil_two_side
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_STENCIL_TEST_TWO_SIDE_EXT] = new StateVariableData(L"GL_STENCIL_TEST_TWO_SIDE_EXT", ap_glIsEnabled, GL_STENCIL_TEST_TWO_SIDE_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ACTIVE_STENCIL_FACE_EXT] = new StateVariableData(L"GL_ACTIVE_STENCIL_FACE_EXT", GL_ACTIVE_STENCIL_FACE_EXT, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_multitexture
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_ACTIVE_TEXTURE_ARB] = new StateVariableData(L"GL_ACTIVE_TEXTURE_ARB", GL_ACTIVE_TEXTURE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_CLIENT_ACTIVE_TEXTURE_ARB] = new StateVariableData(L"GL_CLIENT_ACTIVE_TEXTURE_ARB", GL_CLIENT_ACTIVE_TEXTURE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TEXTURE_UNITS_ARB] = new StateVariableData(L"GL_MAX_TEXTURE_UNITS_ARB", ap_glGetIntegerv, GL_MAX_TEXTURE_UNITS_ARB, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_blend_logic_op and GL_EXT_blend_minmax
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_BLEND_EQUATION_EXT] = new StateVariableData(L"GL_BLEND_EQUATION_EXT", GL_BLEND_EQUATION_EXT, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_SGIX_interlace
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_INTERLACE_SGIX] = new StateVariableData(L"GL_INTERLACE_SGIX", ap_glIsEnabled, GL_INTERLACE_SGIX, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_framebuffer_object
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_FRAMEBUFFER_BINDING_EXT] = new StateVariableData(L"GL_FRAMEBUFFER_BINDING_EXT", ap_glGetIntegerv, GL_FRAMEBUFFER_BINDING_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_RENDERBUFFER_BINDING_EXT] = new StateVariableData(L"GL_RENDERBUFFER_BINDING_EXT", ap_glGetIntegerv, GL_RENDERBUFFER_BINDING_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_RENDERBUFFER_SIZE_EXT] = new StateVariableData(L"GL_MAX_RENDERBUFFER_SIZE_EXT", ap_glGetIntegerv, GL_MAX_RENDERBUFFER_SIZE_EXT, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_framebuffer_blit
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_DRAW_FRAMEBUFFER_BINDING_EXT] = new StateVariableData(L"GL_DRAW_FRAMEBUFFER_BINDING_EXT", ap_glGetIntegerv, GL_DRAW_FRAMEBUFFER_BINDING_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_READ_FRAMEBUFFER_BINDING_EXT] = new StateVariableData(L"GL_READ_FRAMEBUFFER_BINDING_EXT", ap_glGetIntegerv, GL_READ_FRAMEBUFFER_BINDING_EXT, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_framebuffer_multisample
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_MAX_SAMPLES_EXT] = new StateVariableData(L"GL_MAX_SAMPLES_EXT", ap_glGetIntegerv, GL_MAX_SAMPLES_EXT, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_framebuffer_object
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_FRAMEBUFFER_BINDING] = new StateVariableData(L"GL_FRAMEBUFFER_BINDING", ap_glGetIntegerv, GL_FRAMEBUFFER_BINDING, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_RENDERBUFFER_BINDING] = new StateVariableData(L"GL_RENDERBUFFER_BINDING", ap_glGetIntegerv, GL_RENDERBUFFER_BINDING, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_MAX_RENDERBUFFER_SIZE] = new StateVariableData(L"GL_MAX_RENDERBUFFER_SIZE", ap_glGetIntegerv, GL_MAX_RENDERBUFFER_SIZE, 0, AP_OPENGL_STATE_VAR | AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_DRAW_FRAMEBUFFER_BINDING] = new StateVariableData(L"GL_DRAW_FRAMEBUFFER_BINDING", ap_glGetIntegerv, GL_DRAW_FRAMEBUFFER_BINDING, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_READ_FRAMEBUFFER_BINDING] = new StateVariableData(L"GL_READ_FRAMEBUFFER_BINDING", ap_glGetIntegerv, GL_READ_FRAMEBUFFER_BINDING, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_SAMPLES] = new StateVariableData(L"GL_MAX_SAMPLES", ap_glGetIntegerv, GL_MAX_SAMPLES, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_texture_rectangle
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_TEXTURE_RECTANGLE_ARB] = new StateVariableData(L"GL_TEXTURE_RECTANGLE_ARB", ap_glIsEnabled, GL_TEXTURE_RECTANGLE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_BINDING_RECTANGLE_ARB] = new StateVariableData(L"GL_TEXTURE_BINDING_RECTANGLE_ARB", ap_glGetIntegerv, GL_TEXTURE_BINDING_RECTANGLE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_RECTANGLE_TEXTURE_SIZE_ARB] = new StateVariableData(L"GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB", ap_glGetIntegerv, GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_texture_rectangle
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_TEXTURE_RECTANGLE_NV] = new StateVariableData(L"GL_TEXTURE_RECTANGLE_NV", ap_glIsEnabled, GL_TEXTURE_RECTANGLE_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_BINDING_RECTANGLE_NV] = new StateVariableData(L"GL_TEXTURE_BINDING_RECTANGLE_NV", ap_glGetIntegerv, GL_TEXTURE_BINDING_RECTANGLE_NV, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_RECTANGLE_TEXTURE_SIZE_NV] = new StateVariableData(L"GL_MAX_RECTANGLE_TEXTURE_SIZE_NV", ap_glGetIntegerv, GL_MAX_RECTANGLE_TEXTURE_SIZE_NV, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_multisample
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_MULTISAMPLE_ARB] = new StateVariableData(L"GL_MULTISAMPLE_ARB", ap_glGetBooleanv, GL_MULTISAMPLE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_SAMPLE_ALPHA_TO_COVERAGE_ARB] = new StateVariableData(L"GL_SAMPLE_ALPHA_TO_COVERAGE_ARB", ap_glGetBooleanv, GL_SAMPLE_ALPHA_TO_COVERAGE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_SAMPLE_ALPHA_TO_ONE_ARB] = new StateVariableData(L"GL_SAMPLE_ALPHA_TO_ONE_ARB", ap_glGetBooleanv, GL_SAMPLE_ALPHA_TO_ONE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_SAMPLE_COVERAGE_ARB] = new StateVariableData(L"GL_SAMPLE_COVERAGE_ARB", ap_glGetBooleanv, GL_SAMPLE_COVERAGE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_SAMPLE_COVERAGE_INVERT_ARB] = new StateVariableData(L"GL_SAMPLE_COVERAGE_INVERT_ARB", ap_glGetBooleanv, GL_SAMPLE_COVERAGE_INVERT_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_SAMPLE_COVERAGE_VALUE_ARB] = new StateVariableData(L"GL_SAMPLE_COVERAGE_VALUE_ARB", ap_glGetFloatv, GL_SAMPLE_COVERAGE_VALUE_ARB, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_geometry_shader4
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT] = new StateVariableData(L"GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT", ap_glGetIntegerv, GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT] = new StateVariableData(L"GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT", ap_glGetIntegerv, GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT] = new StateVariableData(L"GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT", ap_glGetIntegerv, GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT] = new StateVariableData(L"GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT", ap_glGetIntegerv, GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_GEOMETRY_VARYING_COMPONENTS_EXT] = new StateVariableData(L"GL_MAX_GEOMETRY_VARYING_COMPONENTS_EXT", ap_glGetIntegerv, GL_MAX_GEOMETRY_VARYING_COMPONENTS_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_VARYING_COMPONENTS_EXT] = new StateVariableData(L"GL_MAX_VERTEX_VARYING_COMPONENTS_EXT", ap_glGetIntegerv, GL_MAX_VERTEX_VARYING_COMPONENTS_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VARYING_COMPONENTS_EXT] = new StateVariableData(L"GL_MAX_VARYING_COMPONENTS_EXT", ap_glGetIntegerv, GL_MAX_VARYING_COMPONENTS_EXT, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_texture_array
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_TEXTURE_BINDING_1D_ARRAY_EXT] = new StateVariableData(L"GL_TEXTURE_BINDING_1D_ARRAY_EXT", ap_glGetIntegerv, GL_TEXTURE_BINDING_1D_ARRAY_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_BINDING_2D_ARRAY_EXT] = new StateVariableData(L"GL_TEXTURE_BINDING_2D_ARRAY_EXT", ap_glGetIntegerv, GL_TEXTURE_BINDING_2D_ARRAY_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_ARRAY_TEXTURE_LAYERS_EXT] = new StateVariableData(L"GL_MAX_ARRAY_TEXTURE_LAYERS_EXT", ap_glGetIntegerv, GL_MAX_ARRAY_TEXTURE_LAYERS_EXT, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_texture_buffer_object
    //////////////////////////////////////////////////////////////////////////
    // TO_DO: find out what the lengths of these vectors are:
    // 2* xZ+
    _stateVariablesData[apGL_TEXTURE_BINDING_BUFFER_EXT] = new StateVariableData(L"GL_TEXTURE_BINDING_BUFFER_EXT", ap_glGetIntegerv, GL_TEXTURE_BINDING_BUFFER_EXT, 0, AP_OPENGL_STATE_VAR);
    // nxZ+ - this is also an enum...
    _stateVariablesData[apGL_TEXTURE_BUFFER_FORMAT_EXT] = new StateVariableData(L"GL_TEXTURE_BUFFER_FORMAT_EXT", GL_TEXTURE_BUFFER_FORMAT_EXT, 0, AP_OPENGL_STATE_VAR);
    // The last two are OK
    _stateVariablesData[apGL_TEXTURE_BUFFER_EXT] = new StateVariableData(L"GL_TEXTURE_BUFFER_EXT", ap_glGetIntegerv, GL_TEXTURE_BUFFER_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TEXTURE_BUFFER_SIZE_EXT] = new StateVariableData(L"GL_MAX_TEXTURE_BUFFER_SIZE_EXT", ap_glGetIntegerv, GL_MAX_TEXTURE_BUFFER_SIZE_EXT, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_compiled_vertex_array
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_ARRAY_ELEMENT_LOCK_FIRST_EXT] = new StateVariableData(L"GL_ARRAY_ELEMENT_LOCK_FIRST_EXT", ap_glGetIntegerv, GL_ARRAY_ELEMENT_LOCK_FIRST_EXT, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ARRAY_ELEMENT_LOCK_COUNT_EXT] = new StateVariableData(L"GL_ARRAY_ELEMENT_LOCK_COUNT_EXT", ap_glGetIntegerv, GL_ARRAY_ELEMENT_LOCK_COUNT_EXT, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_transpose_matrix
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_TRANSPOSE_MODELVIEW_MATRIX_ARB] = new StateVariableData(L"GL_TRANSPOSE_MODELVIEW_MATRIX_ARB", ap_glGetFloatv, 4, 4, GL_TRANSPOSE_MODELVIEW_MATRIX_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TRANSPOSE_PROJECTION_MATRIX_ARB] = new StateVariableData(L"GL_TRANSPOSE_PROJECTION_MATRIX_ARB", ap_glGetFloatv, 4, 4, GL_TRANSPOSE_MODELVIEW_MATRIX_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TRANSPOSE_TEXTURE_MATRIX_ARB] = new StateVariableData(L"GL_TRANSPOSE_TEXTURE_MATRIX_ARB", ap_glGetFloatv, 4, 4, GL_TRANSPOSE_MODELVIEW_MATRIX_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TRANSPOSE_COLOR_MATRIX_ARB] = new StateVariableData(L"GL_TRANSPOSE_COLOR_MATRIX_ARB", ap_glGetFloatv, 4, 4, GL_TRANSPOSE_COLOR_MATRIX_ARB, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_point_parameters
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_POINT_SIZE_MIN_ARB] = new StateVariableData(L"GL_POINT_SIZE_MIN_ARB", ap_glGetFloatv, GL_POINT_SIZE_MIN_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_POINT_SIZE_MAX_ARB] = new StateVariableData(L"GL_POINT_SIZE_MAX_ARB", ap_glGetFloatv, GL_POINT_SIZE_MAX_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_POINT_FADE_THRESHOLD_SIZE_ARB] = new StateVariableData(L"GL_POINT_FADE_THRESHOLD_SIZE_ARB", ap_glGetFloatv, GL_POINT_FADE_THRESHOLD_SIZE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_POINT_DISTANCE_ATTENUATION_ARB] = new StateVariableData(L"GL_POINT_DISTANCE_ATTENUATION_ARB", ap_glGetFloatv, 3, GL_POINT_DISTANCE_ATTENUATION_ARB, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_texture_float
    //////////////////////////////////////////////////////////////////////////
    // Uri 22/9/08: see comment in apOpenGLStateVariableId.h under the
    // GL_ARB_texture_float header.
    /*
    _stateVariablesData[apGL_TEXTURE_RED_TYPE_ARB] = new StateVariableData(L"GL_TEXTURE_RED_TYPE_ARB", ap_glGetTexLevelParameteriv, 3, GL_TEXTURE_RED_TYPE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_GREEN_TYPE_ARB] = new StateVariableData(L"GL_TEXTURE_GREEN_TYPE_ARB", ap_glGetTexLevelParameteriv, 3, GL_TEXTURE_GREEN_TYPE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_BLUE_TYPE_ARB] = new StateVariableData(L"GL_TEXTURE_BLUE_TYPE_ARB", ap_glGetTexLevelParameteriv, 3, GL_TEXTURE_BLUE_TYPE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_ALPHA_TYPE_ARB] = new StateVariableData(L"GL_TEXTURE_ALPHA_TYPE_ARB", ap_glGetTexLevelParameteriv, 3, GL_TEXTURE_ALPHA_TYPE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_LUMINANCE_TYPE_ARB] = new StateVariableData(L"GL_TEXTURE_LUMINANCE_TYPE_ARB", ap_glGetTexLevelParameteriv, 3, GL_TEXTURE_LUMINANCE_TYPE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_INTENSITY_TYPE_ARB] = new StateVariableData(L"GL_TEXTURE_INTENSITY_TYPE_ARB", ap_glGetTexLevelParameteriv, 3, GL_TEXTURE_INTENSITY_TYPE_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_DEPTH_TYPE_ARB] = new StateVariableData(L"GL_TEXTURE_DEPTH_TYPE_ARB", ap_glGetTexLevelParameteriv, 3, GL_TEXTURE_DEPTH_TYPE_ARB, 0, AP_OPENGL_STATE_VAR);
    */

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_pixel_buffer_object
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_PIXEL_PACK_BUFFER_BINDING_ARB] = new StateVariableData(L"GL_PIXEL_PACK_BUFFER_BINDING_ARB", ap_glGetIntegerv, GL_PIXEL_PACK_BUFFER_BINDING_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_PIXEL_UNPACK_BUFFER_BINDING_ARB] = new StateVariableData(L"GL_PIXEL_UNPACK_BUFFER_BINDING_ARB", ap_glGetIntegerv, GL_PIXEL_UNPACK_BUFFER_BINDING_ARB, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_framebuffer_sRGB
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_FRAMEBUFFER_SRGB] = new StateVariableData(L"GL_FRAMEBUFFER_SRGB", ap_glIsEnabled, GL_FRAMEBUFFER_SRGB, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_geometry_shader4
    //////////////////////////////////////////////////////////////////////////
    // _stateVariablesData[apGL_FRAMEBUFFER_ATTACHMENT_LAYERED_ARB] = new StateVariableData(L"GL_FRAMEBUFFER_ATTACHMENT_LAYERED_ARB", ap_glGetFramebufferAttachmentParameteriv, GL_FRAMEBUFFER_ATTACHMENT_LAYERED_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB] = new StateVariableData(L"GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB", ap_glGetIntegerv, GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_GEOMETRY_OUTPUT_VERTICES_ARB] = new StateVariableData(L"GL_MAX_GEOMETRY_OUTPUT_VERTICES_ARB", ap_glGetIntegerv, GL_MAX_GEOMETRY_OUTPUT_VERTICES_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB] = new StateVariableData(L"GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB", ap_glGetIntegerv, GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB] = new StateVariableData(L"GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB", ap_glGetIntegerv, GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_GEOMETRY_VARYING_COMPONENTS_ARB] = new StateVariableData(L"GL_MAX_GEOMETRY_VARYING_COMPONENTS_ARB", ap_glGetIntegerv, GL_MAX_GEOMETRY_VARYING_COMPONENTS_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_VARYING_COMPONENTS_ARB] = new StateVariableData(L"GL_MAX_VERTEX_VARYING_COMPONENTS_ARB", ap_glGetIntegerv, GL_MAX_VERTEX_VARYING_COMPONENTS_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VARYING_COMPONENTS_ARB] = new StateVariableData(L"GL_MAX_VARYING_COMPONENTS_ARB", ap_glGetIntegerv, GL_MAX_VARYING_COMPONENTS, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_instanced_arrays
    //////////////////////////////////////////////////////////////////////////
    // Uri, 09/09/08: The state variable identifier (GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ARB = 0x88FE) does not appear in glext.h
    // Note that if we sometime add this, we will need to implement glGetVertexAttrib* in the state variables reader
    // This state variable could be 16+ long, we need to see how to get its length.
    // _stateVariablesData[apGL_VERTEX_ATTRIB_ARRAY_DIVISOR_ARB] = new StateVariableData(L"GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ARB", ap_glGetVertexAttribiv, GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ARB, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_texture_buffer_object
    //////////////////////////////////////////////////////////////////////////
    // TO_DO: find out what the lengths of these vectors are:
    // 2* xZ+
    _stateVariablesData[apGL_TEXTURE_BINDING_BUFFER_ARB] = new StateVariableData(L"GL_TEXTURE_BINDING_BUFFER_ARB", ap_glGetIntegerv, GL_TEXTURE_BINDING_BUFFER_ARB, 0, AP_OPENGL_STATE_VAR);
    // nxZ+
    _stateVariablesData[apGL_TEXTURE_BUFFER_DATA_STORE_BINDING_EXT] = new StateVariableData(L"GL_TEXTURE_BUFFER_DATA_STORE_BINDING_EXT", ap_glGetIntegerv, GL_TEXTURE_BUFFER_DATA_STORE_BINDING_EXT, 0, AP_OPENGL_STATE_VAR);
    // nxZ+ - this is also an enum...
    _stateVariablesData[apGL_TEXTURE_BUFFER_FORMAT_ARB] = new StateVariableData(L"GL_TEXTURE_BUFFER_FORMAT_ARB", GL_TEXTURE_BUFFER_FORMAT_ARB, 0, AP_OPENGL_STATE_VAR);
    // The last two are OK
    _stateVariablesData[apGL_TEXTURE_BUFFER_ARB] = new StateVariableData(L"GL_TEXTURE_BUFFER_ARB", ap_glGetIntegerv, GL_TEXTURE_BUFFER_ARB, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_TEXTURE_BUFFER_SIZE_ARB] = new StateVariableData(L"GL_MAX_TEXTURE_BUFFER_SIZE_ARB", ap_glGetIntegerv, GL_MAX_TEXTURE_BUFFER_SIZE_ARB, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_vertex_array_object
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_VERTEX_ARRAY_BINDING] = new StateVariableData(L"GL_VERTEX_ARRAY_BINDING", ap_glGetIntegerv, GL_VERTEX_ARRAY_BINDING, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_uniform_buffer_object
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_UNIFORM_BUFFER_BINDING] = new StateVariableData(L"GL_UNIFORM_BUFFER_BINDING", ap_glGetIntegerv, GL_UNIFORM_BUFFER_BINDING, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_UNIFORM_BLOCKS] = new StateVariableData(L"GL_MAX_VERTEX_UNIFORM_BLOCKS", ap_glGetIntegerv, GL_MAX_VERTEX_UNIFORM_BLOCKS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_FRAGMENT_UNIFORM_BLOCKS] = new StateVariableData(L"GL_MAX_FRAGMENT_UNIFORM_BLOCKS", ap_glGetIntegerv, GL_MAX_FRAGMENT_UNIFORM_BLOCKS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_GEOMETRY_UNIFORM_BLOCKS] = new StateVariableData(L"GL_MAX_GEOMETRY_UNIFORM_BLOCKS", ap_glGetIntegerv, GL_MAX_GEOMETRY_UNIFORM_BLOCKS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMBINED_UNIFORM_BLOCKS] = new StateVariableData(L"GL_MAX_COMBINED_UNIFORM_BLOCKS", ap_glGetIntegerv, GL_MAX_COMBINED_UNIFORM_BLOCKS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_UNIFORM_BUFFER_BINDINGS] = new StateVariableData(L"GL_MAX_UNIFORM_BUFFER_BINDINGS", ap_glGetIntegerv, GL_MAX_UNIFORM_BUFFER_BINDINGS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_UNIFORM_BLOCK_SIZE] = new StateVariableData(L"GL_MAX_UNIFORM_BLOCK_SIZE", ap_glGetIntegerv, GL_MAX_UNIFORM_BLOCK_SIZE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_VERTEX_UNIFORM_COMPONENTS] = new StateVariableData(L"GL_MAX_VERTEX_UNIFORM_COMPONENTS", ap_glGetIntegerv, GL_MAX_VERTEX_UNIFORM_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_FRAGMENT_UNIFORM_COMPONENTS] = new StateVariableData(L"GL_MAX_FRAGMENT_UNIFORM_COMPONENTS", ap_glGetIntegerv, GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_GEOMETRY_UNIFORM_COMPONENTS] = new StateVariableData(L"GL_MAX_GEOMETRY_UNIFORM_COMPONENTS", ap_glGetIntegerv, GL_MAX_GEOMETRY_UNIFORM_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS] = new StateVariableData(L"GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS", ap_glGetIntegerv, GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS] = new StateVariableData(L"GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS", ap_glGetIntegerv, GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS] = new StateVariableData(L"GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS", ap_glGetIntegerv, GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_UNIFORM_BUFFER_OFFSET_ALIGNMENT] = new StateVariableData(L"GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT", ap_glGetIntegerv, GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_depth_clamp
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_DEPTH_CLAMP] = new StateVariableData(L"GL_DEPTH_CLAMP", ap_glIsEnabled, GL_DEPTH_CLAMP, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_provoking_vertex
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_PROVOKING_VERTEX] = new StateVariableData(L"GL_PROVOKING_VERTEX", ap_glGetIntegerv, GL_PROVOKING_VERTEX, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION] = new StateVariableData(L"GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION", ap_glGetBooleanv, GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_seamless_cube_map
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_TEXTURE_CUBE_MAP_SEAMLESS] = new StateVariableData(L"GL_TEXTURE_CUBE_MAP_SEAMLESS", ap_glIsEnabled, GL_TEXTURE_CUBE_MAP_SEAMLESS, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_vertex_array_bgra
    //////////////////////////////////////////////////////////////////////////
    // See comment in apOpenGLStateVariableId.h under "GL_ARB_vertex_array_bgra"
    // _stateVariablesData[apGL_VERTEX_ATTRIB_ARRAY_SIZE] = new StateVariableData(L"GL_VERTEX_ATTRIB_ARRAY_SIZE", ap_glGetIntegerv, GL_VERTEX_ATTRIB_ARRAY_SIZE, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_sync
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_MAX_SERVER_WAIT_TIMEOUT] = new StateVariableData(L"GL_MAX_SERVER_WAIT_TIMEOUT", ap_glGetInteger64v, GL_MAX_SERVER_WAIT_TIMEOUT, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_texture_multisample
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_SAMPLE_MASK] = new StateVariableData(L"GL_SAMPLE_MASK", ap_glIsEnabled, GL_SAMPLE_MASK, 0, AP_OPENGL_STATE_VAR);
    // TO_DO: OpenGL3.2
    // _stateVariablesData[apGL_SAMPLE_MASK_VALUE] = new StateVariableData(L"GL_SAMPLE_MASK_VALUE", ap_glGetIntegeri_v, apGL_SAMPLE_MASK_VALUE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_BINDING_2D_MULTISAMPLE] = new StateVariableData(L"GL_TEXTURE_BINDING_2D_MULTISAMPLE", ap_glGetIntegerv, GL_TEXTURE_BINDING_2D_MULTISAMPLE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY] = new StateVariableData(L"GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY", ap_glGetIntegerv, GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_SAMPLE_MASK_WORDS] = new StateVariableData(L"GL_MAX_SAMPLE_MASK_WORDS", ap_glGetIntegerv, GL_MAX_SAMPLE_MASK_WORDS, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_COLOR_TEXTURE_SAMPLES] = new StateVariableData(L"GL_MAX_COLOR_TEXTURE_SAMPLES", ap_glGetIntegerv, GL_MAX_COLOR_TEXTURE_SAMPLES, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_DEPTH_TEXTURE_SAMPLES] = new StateVariableData(L"GL_MAX_DEPTH_TEXTURE_SAMPLES", ap_glGetIntegerv, GL_MAX_DEPTH_TEXTURE_SAMPLES, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_MAX_INTEGER_SAMPLES] = new StateVariableData(L"GL_MAX_INTEGER_SAMPLES", ap_glGetIntegerv, GL_MAX_INTEGER_SAMPLES, 0, AP_OPENGL_STATE_VAR);

    // ----------------------------------------------------
    //      WGL extensions state variables:
    // ----------------------------------------------------

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    //////////////////////////////////////////////////////////////////////////
    // WGL_ARB_pixel_format_float - See GL_ARB_color_buffer_float
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // WGL_ARB_framebuffer_sRGB - See GL_ARB_framebuffer_sRGB
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // WGL_EXT_swap_control
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apWGL_SWAP_INTERVAL_EXT] = new StateVariableData(L"WGL_SWAP_INTERVAL_EXT", ap_wglGetSwapIntervalEXT, 0, 0, AP_WGL_STATE_VAR);

#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_aux_depth_stencil
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_AUX_DEPTH_STENCIL_APPLE] = new StateVariableData(L"GL_AUX_DEPTH_STENCIL_APPLE", ap_glGetIntegerv, GL_AUX_DEPTH_STENCIL_APPLE, 0, AP_OPENGL_STATE_VAR);
#endif

    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_client_storage
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_UNPACK_CLIENT_STORAGE_APPLE] = new StateVariableData(L"GL_UNPACK_CLIENT_STORAGE_APPLE", ap_glGetBooleanv, GL_UNPACK_CLIENT_STORAGE_APPLE, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_element_array
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_ELEMENT_ARRAY_APPLE] = new StateVariableData(L"GL_ELEMENT_ARRAY_APPLE", ap_glIsEnabled, GL_ELEMENT_ARRAY_APPLE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ELEMENT_ARRAY_TYPE_APPLE] = new StateVariableData(L"GL_ELEMENT_ARRAY_TYPE_APPLE", GL_ELEMENT_ARRAY_TYPE_APPLE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_ELEMENT_ARRAY_POINTER_APPLE] = new StateVariableData(L"GL_ELEMENT_ARRAY_POINTER_APPLE", ap_glGetPointerv, GL_ELEMENT_ARRAY_POINTER_APPLE, 0, AP_OPENGL_STATE_VAR);

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_float_pixels
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_COLOR_FLOAT_APPLE] = new StateVariableData(L"GL_COLOR_FLOAT_APPLE", ap_glGetBooleanv, GL_COLOR_FLOAT_APPLE, 0, AP_OPENGL_STATE_VAR);
#endif

    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_specular_vector
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE] = new StateVariableData(L"GL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE", ap_glGetBooleanv, GL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_transform_hint
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_TRANSFORM_HINT_APPLE] = new StateVariableData(L"GL_TRANSFORM_HINT_APPLE", GL_TRANSFORM_HINT_APPLE, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_vertex_array_object
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_VERTEX_ARRAY_BINDING_APPLE] = new StateVariableData(L"GL_VERTEX_ARRAY_BINDING_APPLE", ap_glGetIntegerv, GL_VERTEX_ARRAY_BINDING_APPLE, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_vertex_array_range
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_VERTEX_ARRAY_RANGE_APPLE] = new StateVariableData(L"GL_VERTEX_ARRAY_RANGE_APPLE", ap_glIsEnabled, GL_VERTEX_ARRAY_RANGE_APPLE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ARRAY_RANGE_POINTER_APPLE] = new StateVariableData(L"GL_VERTEX_ARRAY_RANGE_POINTER_APPLE", ap_glGetPointerv, GL_VERTEX_ARRAY_RANGE_POINTER_APPLE, 0, AP_OPENGL_STATE_VAR);
    _stateVariablesData[apGL_VERTEX_ARRAY_RANGE_LENGTH_APPLE] = new StateVariableData(L"GL_VERTEX_ARRAY_RANGE_LENGTH_APPLE", ap_glGetIntegerv, GL_VERTEX_ARRAY_RANGE_LENGTH_APPLE, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_texture_integer
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_RGBA_INTEGER_MODE_EXT] = new StateVariableData(L"GL_RGBA_INTEGER_MODE_EXT", ap_glGetBooleanv, GL_RGBA_INTEGER_MODE_EXT, 0, AP_OPENGL_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_bindable_uniform
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_UNIFORM_BUFFER_BINDING_EXT] = new StateVariableData(L"GL_RGBA_INTEGER_MODE_EXT", ap_glGetIntegerv, GL_UNIFORM_BUFFER_BINDING_EXT, 0, AP_OPENGL_STATE_VAR);

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
#ifdef _GR_IPHONE_BUILD

    // iPhone build:
    // Yaki 31/5/2009 - Currently, EAGL does not define any OpenGL ES state variables.

#else

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    // CGL
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    // Sigal 7.1.09: The parameters in comment for some reason do not work, and we weren't able to understand
    // what's their use, so currently we don't support it

    // CGL Context Parameters:
    _stateVariablesData[ap_kCGLCPSwapRectangle] = new StateVariableData(L"kCGLCPSwapRectangle", ap_CGLGetParameter, kCGLCPSwapRectangle);
    _stateVariablesData[ap_kCGLCPSwapInterval] = new StateVariableData(L"kCGLCPSwapInterval", ap_CGLGetParameter, kCGLCPSwapInterval);
    _stateVariablesData[ap_kCGLCPDispatchTableSize] = new StateVariableData(L"kCGLCPDispatchTableSize", ap_CGLGetParameter , kCGLCPDispatchTableSize);
    _stateVariablesData[ap_kCGLCPClientStorage] = new StateVariableData(L"kCGLCPClientStorage", ap_CGLGetParameter , kCGLCPClientStorage);
    _stateVariablesData[ap_kCGLCPSurfaceTexture] = new StateVariableData(L"kCGLCPSurfaceTexture", ap_CGLGetParameter , kCGLCPSurfaceTexture);
    _stateVariablesData[ap_kCGLCPSurfaceOrder] = new StateVariableData(L"kCGLCPSurfaceOrder", ap_CGLGetParameter , kCGLCPSurfaceOrder);
    //_stateVariablesData[ap_kCGLCPSurfaceOpacity] = new StateVariableData(L"kCGLCPSurfaceOpacity", ap_CGLGetParameter , kCGLCPSurfaceOpacity);
    //_stateVariablesData[ap_kCGLCPSurfaceBackingSize] = new StateVariableData(L"kCGLCPSurfaceBackingSize", ap_CGLGetParameter , kCGLCPSurfaceBackingSize);
    _stateVariablesData[ap_kCGLCPSurfaceSurfaceVolatile] = new StateVariableData(L"kCGLCPSurfaceSurfaceVolatile", ap_CGLGetParameter , kCGLCPSurfaceSurfaceVolatile);
    //_stateVariablesData[ap_kCGLCPReclaimResources] = new StateVariableData(L"kCGLCPReclaimResources", ap_CGLGetParameter , kCGLCPReclaimResources);
    _stateVariablesData[ap_kCGLCPCurrentRendererID] = new StateVariableData(L"kCGLCPCurrentRendererID", ap_CGLGetParameter , kCGLCPCurrentRendererID);
    _stateVariablesData[ap_kCGLCPGPUVertexProcessing] = new StateVariableData(L"kCGLCPGPUVertexProcessing", ap_CGLGetParameter , kCGLCPGPUVertexProcessing);
    _stateVariablesData[ap_kCGLCPGPUFragmentProcessing] = new StateVariableData(L"kCGLCPGPUFragmentProcessing", ap_CGLGetParameter , kCGLCPGPUFragmentProcessing);

    // CGL Context enable:
    _stateVariablesData[ap_kCGLCESwapRectangle] = new StateVariableData(L"kCGLCESwapRectangle", ap_CGLIsEnabled , kCGLCESwapRectangle);
    _stateVariablesData[ap_kCGLCERasterization] = new StateVariableData(L"kCGLCERasterization", ap_CGLIsEnabled , kCGLCERasterization);
    _stateVariablesData[ap_kCGLCEStateValidation] = new StateVariableData(L"kCGLCEStateValidation", ap_CGLIsEnabled , kCGLCEStateValidation);
    //_stateVariablesData[ap_kCGLCESurfaceBackingSize] = new StateVariableData(L"kCGLCESurfaceBackingSize", ap_CGLIsEnabled , kCGLCESurfaceBackingSize);
    _stateVariablesData[ap_kCGLCEDisplayListOptimization] = new StateVariableData(L"kCGLCEDisplayListOptimization", ap_CGLIsEnabled , kCGLCEDisplayListOptimization);

    // CGL global options:
    _stateVariablesData[ap_kCGLGOFormatCacheSize] = new StateVariableData(L"kCGLGOFormatCacheSize", ap_CGLGetOption, kCGLGOFormatCacheSize);
    _stateVariablesData[ap_kCGLGOClearFormatCache] = new StateVariableData(L"kCGLGOClearFormatCache", ap_CGLGetOption, kCGLGOClearFormatCache);
    _stateVariablesData[ap_kCGLGORetainRenderers] = new StateVariableData(L"kCGLGORetainRenderers", ap_CGLGetOption, kCGLGORetainRenderers);
    _stateVariablesData[ap_kCGLGOResetLibrary] = new StateVariableData(L"kCGLGOResetLibrary", ap_CGLGetOption, kCGLGOResetLibrary);
    _stateVariablesData[ap_kCGLGOUseErrorHandler] = new StateVariableData(L"kCGLGOUseErrorHandler", ap_CGLGetOption, kCGLGOUseErrorHandler);

#endif // _GR_IPHONE_BUILD

#endif


    // OpenGL ES is currently only supported on Windows and Mac:
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //// OpenGL ES
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // OpenGL ES 1.1
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_LIGHT0_emission] = new StateVariableData(L"GL_LIGHT0 - emission", ap_glGetLightfv, 4, GL_LIGHT0, GL_EMISSION, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT1_emission] = new StateVariableData(L"GL_LIGHT1 - emission", ap_glGetLightfv, 4, GL_LIGHT1, GL_EMISSION, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT2_emission] = new StateVariableData(L"GL_LIGHT2 - emission", ap_glGetLightfv, 4, GL_LIGHT2, GL_EMISSION, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT3_emission] = new StateVariableData(L"GL_LIGHT3 - emission", ap_glGetLightfv, 4, GL_LIGHT3, GL_EMISSION, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT4_emission] = new StateVariableData(L"GL_LIGHT4 - emission", ap_glGetLightfv, 4, GL_LIGHT4, GL_EMISSION, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT5_emission] = new StateVariableData(L"GL_LIGHT5 - emission", ap_glGetLightfv, 4, GL_LIGHT5, GL_EMISSION, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT6_emission] = new StateVariableData(L"GL_LIGHT6 - emission", ap_glGetLightfv, 4, GL_LIGHT6, GL_EMISSION, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_LIGHT7_emission] = new StateVariableData(L"GL_LIGHT7 - emission", ap_glGetLightfv, 4, GL_LIGHT7, GL_EMISSION, AP_OPENGL_ES_1_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // OpenGL ES 2.0
    //////////////////////////////////////////////////////////////////////////
    // Currently only supported on the iPhone:
    // The 8 variables below are part of OpenGL 4.1 support, so we clear them here to avoid duplication:
    // _stateVariablesData[apGL_IMPLEMENTATION_COLOR_READ_TYPE] = new StateVariableData(L"GL_IMPLEMENTATION_COLOR_READ_TYPE", GL_IMPLEMENTATION_COLOR_READ_TYPE, 0, AP_OPENGL_ES_2_STATE_VAR);
    // _stateVariablesData[apGL_IMPLEMENTATION_COLOR_READ_FORMAT] = new StateVariableData(L"GL_IMPLEMENTATION_COLOR_READ_FORMAT", GL_IMPLEMENTATION_COLOR_READ_FORMAT, 0, AP_OPENGL_ES_2_STATE_VAR);
    // _stateVariablesData[apGL_MAX_VERTEX_UNIFORM_VECTORS] = new StateVariableData(L"GL_MAX_VERTEX_UNIFORM_VECTORS", ap_glGetIntegerv, GL_MAX_VERTEX_UNIFORM_VECTORS, 0, AP_OPENGL_ES_2_STATE_VAR);
    // _stateVariablesData[apGL_MAX_VARYING_VECTORS] = new StateVariableData(L"GL_MAX_VARYING_VECTORS", ap_glGetIntegerv, GL_MAX_VARYING_VECTORS, 0, AP_OPENGL_ES_2_STATE_VAR);
    // _stateVariablesData[apGL_MAX_FRAGMENT_UNIFORM_VECTORS] = new StateVariableData(L"GL_MAX_FRAGMENT_UNIFORM_VECTORS", ap_glGetIntegerv, GL_MAX_FRAGMENT_UNIFORM_VECTORS, 0, AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_TEXTURE_BINDING_CUBE_MAP] = new StateVariableData(L"GL_TEXTURE_BINDING_CUBE_MAP", ap_glGetIntegerv, GL_TEXTURE_BINDING_CUBE_MAP, 0, AP_OPENGL_ES_2_STATE_VAR);
    _stateVariablesData[apGL_MAX_CUBE_MAP_TEXTURE_SIZE] = new StateVariableData(L"GL_MAX_CUBE_MAP_TEXTURE_SIZE", ap_glGetIntegerv, GL_MAX_CUBE_MAP_TEXTURE_SIZE, 0, AP_OPENGL_ES_2_STATE_VAR);
    // _stateVariablesData[apGL_SHADER_COMPILER] = new StateVariableData(L"GL_SHADER_COMPILER", ap_glGetBooleanv, GL_SHADER_COMPILER, 0, AP_OPENGL_ES_2_STATE_VAR);
    // _stateVariablesData[apGL_SHADER_BINARY_FORMATS] = new StateVariableData(L"GL_SHADER_BINARY_FORMATS", GL_SHADER_BINARY_FORMATS, GL_NUM_SHADER_BINARY_FORMATS, AP_OPENGL_ES_2_STATE_VAR);
    // _stateVariablesData[apGL_NUM_SHADER_BINARY_FORMATS] = new StateVariableData(L"GL_NUM_SHADER_BINARY_FORMATS", ap_glGetIntegerv, GL_NUM_SHADER_BINARY_FORMATS, 0, AP_OPENGL_ES_2_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // OES_matrix_palette
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_MAX_VERTEX_UNITS_OES] = new StateVariableData(L"GL_MAX_VERTEX_UNITS_OES", ap_glGetIntegerv, GL_MAX_VERTEX_UNITS_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_MAX_PALETTE_MATRICES_OES] = new StateVariableData(L"GL_MAX_PALETTE_MATRICES_OES", ap_glGetIntegerv, GL_MAX_PALETTE_MATRICES_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_MATRIX_PALETTE_OES] = new StateVariableData(L"GL_MATRIX_PALETTE_OES", ap_glIsEnabled, GL_MATRIX_PALETTE_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_CURRENT_PALETTE_MATRIX_OES] = new StateVariableData(L"GL_CURRENT_PALETTE_MATRIX_OES", ap_glGetIntegerv, GL_CURRENT_PALETTE_MATRIX_OES, 0, AP_OPENGL_ES_1_STATE_VAR); // Does not appear in the Windows gl.h file :~)
    _stateVariablesData[apGL_MATRIX_INDEX_ARRAY_OES] = new StateVariableData(L"GL_MATRIX_INDEX_ARRAY_OES", ap_glIsEnabled, GL_MATRIX_INDEX_ARRAY_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_MATRIX_INDEX_ARRAY_SIZE_OES] = new StateVariableData(L"GL_MATRIX_INDEX_ARRAY_SIZE_OES", ap_glGetIntegerv, GL_MATRIX_INDEX_ARRAY_SIZE_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_MATRIX_INDEX_ARRAY_TYPE_OES] = new StateVariableData(L"GL_MATRIX_INDEX_ARRAY_TYPE_OES", GL_MATRIX_INDEX_ARRAY_TYPE_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_MATRIX_INDEX_ARRAY_STRIDE_OES] = new StateVariableData(L"GL_MATRIX_INDEX_ARRAY_STRIDE_OES", ap_glGetIntegerv, GL_MATRIX_INDEX_ARRAY_STRIDE_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_MATRIX_INDEX_ARRAY_POINTER_OES] = new StateVariableData(L"GL_MATRIX_INDEX_ARRAY_POINTER_OES", ap_glGetPointerv, GL_MATRIX_INDEX_ARRAY_POINTER_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_MATRIX_INDEX_ARRAY_BUFFER_BINDING_OES] = new StateVariableData(L"GL_MATRIX_INDEX_ARRAY_BUFFER_BINDING_OES", ap_glGetIntegerv, GL_MATRIX_INDEX_ARRAY_BUFFER_BINDING_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_WEIGHT_ARRAY_OES] = new StateVariableData(L"GL_WEIGHT_ARRAY_OES", ap_glIsEnabled, GL_WEIGHT_ARRAY_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_WEIGHT_ARRAY_SIZE_OES] = new StateVariableData(L"GL_WEIGHT_ARRAY_SIZE_OES", ap_glGetIntegerv, GL_WEIGHT_ARRAY_SIZE_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_WEIGHT_ARRAY_TYPE_OES] = new StateVariableData(L"GL_WEIGHT_ARRAY_TYPE_OES", GL_WEIGHT_ARRAY_TYPE_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_WEIGHT_ARRAY_STRIDE_OES] = new StateVariableData(L"GL_WEIGHT_ARRAY_STRIDE_OES", ap_glGetIntegerv, GL_WEIGHT_ARRAY_STRIDE_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_WEIGHT_ARRAY_POINTER_OES] = new StateVariableData(L"GL_WEIGHT_ARRAY_POINTER_OES", ap_glGetPointerv, GL_WEIGHT_ARRAY_POINTER_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_WEIGHT_ARRAY_BUFFER_BINDING_OES] = new StateVariableData(L"GL_WEIGHT_ARRAY_BUFFER_BINDING_OES", ap_glGetIntegerv, GL_WEIGHT_ARRAY_BUFFER_BINDING_OES, 0, AP_OPENGL_ES_1_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // OES_point_sprite
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_POINT_SPRITE_OES] = new StateVariableData(L"GL_POINT_SPRITE_OES", ap_glIsEnabled, GL_POINT_SPRITE_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_COORD_REPLACE_OES] = new StateVariableData(L"GL_COORD_REPLACE_OES", ap_glGetTexEnviv, GL_COORD_REPLACE_OES, 0, AP_OPENGL_ES_1_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // OES_point_size_array
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_POINT_SIZE_ARRAY_OES] = new StateVariableData(L"GL_POINT_SIZE_ARRAY_OES", ap_glIsEnabled, GL_POINT_SIZE_ARRAY_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_POINT_SIZE_ARRAY_TYPE_OES] = new StateVariableData(L"GL_POINT_SIZE_ARRAY_TYPE_OES", GL_POINT_SIZE_ARRAY_TYPE_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_POINT_SIZE_ARRAY_STRIDE_OES] = new StateVariableData(L"GL_POINT_SIZE_ARRAY_STRIDE_OES", ap_glGetIntegerv, GL_POINT_SIZE_ARRAY_STRIDE_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_POINT_SIZE_ARRAY_POINTER_OES] = new StateVariableData(L"GL_POINT_SIZE_ARRAY_POINTER_OES", ap_glGetPointerv, GL_POINT_SIZE_ARRAY_POINTER_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES] = new StateVariableData(L"GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES", ap_glGetIntegerv, GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES, 0, AP_OPENGL_ES_1_STATE_VAR);

    //////////////////////////////////////////////////////////////////////////
    // GL_OES_read_format
    //////////////////////////////////////////////////////////////////////////
    _stateVariablesData[apGL_IMPLEMENTATION_COLOR_READ_TYPE_OES] = new StateVariableData(L"GL_IMPLEMENTATION_COLOR_READ_TYPE_OES", GL_IMPLEMENTATION_COLOR_READ_TYPE_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
    _stateVariablesData[apGL_IMPLEMENTATION_COLOR_READ_FORMAT_OES] = new StateVariableData(L"GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES", GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES, 0, AP_OPENGL_ES_1_STATE_VAR);
#endif // (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
}


// ---------------------------------------------------------------------------
// Name:        apOpenGLStateVariablesManager::terminateStateVariablesData
// Description: Releases the data allocated by the _stateVariablesData array.
// Author:  AMD Developer Tools Team
// Date:        5/3/2007
// ---------------------------------------------------------------------------
void apOpenGLStateVariablesManager::terminateStateVariablesData()
{
    for (int i = 0; i < apOpenGLStateVariablesAmount; i++)
    {
        delete _stateVariablesData[i];
        _stateVariablesData[i] = NULL;
    }
}
