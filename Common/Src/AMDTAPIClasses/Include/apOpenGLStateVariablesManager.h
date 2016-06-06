//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenGLStateVariablesManager.h
///
//==================================================================================

//------------------------------ apOpenGLStateVariablesManager.h ------------------------------

#ifndef __APOPENGLSTATEVARIABLESMANAGER
#define __APOPENGLSTATEVARIABLESMANAGER

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osTransferableObjectType.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTAPIClasses/Include/apOpenGLStateVariableId.h>
#include <AMDTAPIClasses/Include/apAPIVersion.h>


// ----------------------------------------------------------------------------------
// Class Name:           apOpenGLStateVariablesManager
// General Description:
//   Manages the OpenGL state variables that we are able to monitor.
//
// Author:  AMD Developer Tools Team
// Creation Date:        22/4/2004
// ----------------------------------------------------------------------------------
class AP_API apOpenGLStateVariablesManager
{
public:
    // Describes the type of the state variable:
    enum apStateVariableType
    {
        SimpleStateVariable,
        VectorStateVariable,
        MatrixStateVariable
    };

    static apOpenGLStateVariablesManager& instance();
    ~apOpenGLStateVariablesManager();

    int amountOfStateVariables() const { return apOpenGLStateVariablesAmount; };
    apStateVariableType stateVariableType(int variableIndex) const;
    unsigned int stateVariableGlobalType(int variableIndex) const;
    const wchar_t* stateVariableName(int variableIndex) const;
    int amountOfStateVariableElementsN(int variableIndex) const;
    int amountOfStateVariableElementsM(int variableIndex) const;
    apMonitoredFunctionId stateVariableGetFunctionId(int variableIndex) const;
    GLenum stateVariableOpenGLParamEnum(int variableIndex) const;
    unsigned int stateVariableCGLParam(int variableIndex) const ;
    GLenum stateVariableAdditionalOpenGLParamEnum(int variableIndex) const;
    bool isEnumStateVariable(int variableIndex) const;
    apAPIVersion stateVariableRemovedVersion(int variableIndex) const;
    void setStateVariablesRemovedAtVersion(int variableIndex, apAPIVersion removedVersion);

private:
    void initializeStateVariablesData();
    void terminateStateVariablesData();

    // Only my instance() method should be able to create me:
    apOpenGLStateVariablesManager();

    // Only apSingeltonsDelete should delete my single instance:
    friend class apSingeltonsDelete;

private:
    // Contains a state variable data:
    struct AP_API StateVariableData
    {
        // Default constructor:
        StateVariableData();

        // Enum element state variable constructor:
        StateVariableData(const wchar_t* name, GLenum openGLParamEnum, GLenum additionalOpenGLParamEnum, unsigned int stateVariableGlobalType);

        // Single element state variable constructor:
        StateVariableData(const wchar_t* name, apMonitoredFunctionId getFunctionId, GLenum openGLParamEnum,
                          GLenum additionalOpenGLParamEnum, unsigned int stateVariableGlobalType);

        // Vector state variable constructor:
        StateVariableData(const wchar_t* name, apMonitoredFunctionId getFunctionId, int elementsAmount,
                          GLenum openGLParamEnum, GLenum additionalOpenGLParamEnum, unsigned int stateVariableGlobalType);

        // Matrix state variable constructor:
        StateVariableData(const wchar_t* name, apMonitoredFunctionId getFunctionId, int sizeN, int sizeM,
                          GLenum openGLParamEnum, GLenum additionalOpenGLParamEnum, unsigned int stateVariableGlobalType);

        // CGL state variable constructor:
        StateVariableData(const wchar_t* name, apMonitoredFunctionId getFunctionId, unsigned int cglEnumerationAsInt);

        // The state variable name:
        const wchar_t* _name;

        // Contains true iff this is an enum state variable
        // (This is a special case, due to the fact that OpenGL does not have a glGet function
        //  for GLenum values):
        bool _isEnumStateVariable;

        // The OpenGL function that is used for getting the value of this state variable:
        apMonitoredFunctionId _getFunctionId;

        // The amount of elements (N x M) that this state variable contains:
        unsigned int _amountOfElementsN;
        unsigned int _amountOfElementsM;

        // The OpenGL state variable parameter enum:
        // (The parameter we give as an input to OpenGL get functions when we ask for
        //  the variable value)
        GLenum _openGLParamEnum;

        // Few state variables require an additional enum param:
        // (Example: GL_EMISSION required the queried face - GL_BACK, etc)
        GLenum _additionalOpenGLParamEnum;

        // The version at which this state variable was removed from OpenGL (if at all):
        apAPIVersion _removedVersion;

        // The state variable type (as a bit mask of apOpenGLAPIType):
        unsigned int _stateVariableGlobalType;

        // CGL:
        unsigned int _cglEnumerationAsUInt;
    };

    // Contains the monitored function data:
    StateVariableData* _stateVariablesData[apOpenGLStateVariablesAmount];

    // This class single instance:
    static apOpenGLStateVariablesManager* _pMySingleInstance;
};


#endif  // __APOPENGLSTATEVARIABLESMANAGER
