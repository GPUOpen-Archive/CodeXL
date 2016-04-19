//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsDeprecationAnalyzer.h
///
//==================================================================================

//------------------------------ gsDeprecationAnalyzer.h ------------------------------

#ifndef __GSDEPRECETIONANALYZER
#define __GSDEPRECETIONANALYZER

// Forward declarations:
class gsRenderContextMonitor;

// Infra:
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTAPIClasses/Include/apFunctionDeprecation.h>
#include <AMDTAPIClasses/Include/apFunctionCall.h>
#include <AMDTAPIClasses/Include/apGLPixelInternalFormatParameter.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>

// Local:
#include <src/gsDeprecationCondition.h>


typedef gtPtrVector<gsDeprecationCondition*> gsFunctionDeprecations;


// ----------------------------------------------------------------------------------
// Struct Name:   gsDeprecationAnalyzer
// General Description: Handle function call deprecation analyzing
// Author:               Sigal Algranaty
// Creation Date:        3/3/2008
// ----------------------------------------------------------------------------------
class gsDeprecationAnalyzer
{
public:
    gsDeprecationAnalyzer();
    virtual ~gsDeprecationAnalyzer();
    static gsDeprecationAnalyzer& instance();
    void initialize();

    bool getFunctionCallDeprecationStatus(gsRenderContextMonitor* pContextMonitor, apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList, apFunctionDeprecationStatus& functionDeprecationStatus);
    bool getFunctionCallDeprecationDetails(const apFunctionCall* pFunctionCall, const gtList<const apParameter*>& functionArguments, apFunctionDeprecation& functionDeprecationDetails);

private:
    // Private functions for initializing the function deprecations:
    void initializeInvalidArgumentDeprecationsForOpenGL30();
    void initializeUngeneratedObjectsDeprecationsForOpenGL30();
    void initializeOpenGL30Deprecations();
    void initializeOpenGL32Deprecations();
    void initializeOpenGL42Deprecations();
    void initializeOpenGL43Deprecations();
    void addEnumDeprecatedValue(GLenum pname, apFunctionDeprecationStatus assosiatedDeprecationStatus, const gtString& varName, int variableIndex, apMonitoredFunctionId functionId);
    void addEnumDeprecatedValueForGetFunctions(GLenum pname, apFunctionDeprecationStatus assosiatedDeprecationStatus);
    void addEnumDeprecatedValueForEnableFunctions(GLenum pname, apFunctionDeprecationStatus assosiatedDeprecationStatus);
    void addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GLenum pname, apFunctionDeprecationStatus assosiatedDeprecationStatus);
    void addEnumDeprecatedValues(const gtVector<GLenum>& pnames, apFunctionDeprecationStatus assosiatedDeprecationStatus, const gtString& varName, int variableIndex, apMonitoredFunctionId functionId);
    void addGLClearBitfieldDeprecatedValue(GLbitfield bitfield, apFunctionDeprecationStatus assosiatedDeprecationStatus, const gtString& varName, int variableIndex, apMonitoredFunctionId functionId);
    void addFloatComparisonDeprecatedValue(GLfloat valueToCompare, gsArgValueComparisonType compareType, apFunctionDeprecationStatus assosiatedDeprecationStatus, const gtString& varName, int variableIndex, apMonitoredFunctionId functionId);
    void addPixelFormatDeprecatedValues(const gtVector<int>& pixelFormatValuesVector, apFunctionDeprecationStatus assosiatedDeprecationStatus, const gtString& varName, int variableIndex, apMonitoredFunctionId functionId);
    void getDeprecatedInternalFormats(gtVector<int>& pixelFormatIllegalValues);
    void getDeprecatedExternalFormats(gtVector<GLenum>& pixelFormatIllegalValues);

    // State variables:
    bool initializeStateVariableDeprecations();
    bool isStateVariableDeprecated(GLenum pname, apMonitoredFunctionId getFunctionId, apAPIVersion& removedVersion);

private:
    friend class gsSingletonsDelete;

    // The single instance of this class:
    static gsDeprecationAnalyzer* _pMySingleInstance;

    // Function deprecations:
    gsFunctionDeprecations* _monitoredFunctionsDeprecations[apMonitoredFunctionsAmount];
};


#endif  // __GSDEPRECETIONANALYZER
