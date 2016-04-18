//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsStateChangeExecutor.h
///
//==================================================================================

//------------------------------ gsStateChangeExecutor.h ------------------------------

#ifndef __GSSTATECHANGEEXECUTOR
#define __GSSTATECHANGEEXECUTOR

// Infra
#include <AMDTAPIClasses/Include/apFunctionType.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>

// Local:
#include <src/gsStateVariablesSnapshot.h>
// ----------------------------------------------------------------------------------
// Class Name:           gsStateChangeExecutor
// General Description: This class is used for state change 'virtual' execution.
//                      We use it on begin end block functions that are also state change functions.
//                      The class is used for OpenGL state variable value change simulation.
// Author:               Sigal Algranaty
// Creation Date:        20/8/2008
// ----------------------------------------------------------------------------------
class gsStateChangeExecutor
{
public:
    gsStateChangeExecutor(gsStateVariablesSnapshot* pStateVariableSnapShot);
    virtual ~gsStateChangeExecutor();

public:
    bool applyStateChange(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList, apFunctionRedundancyStatus& redundancyStatus);

private:
    // Specific function types apply functions:
    bool applyColorStateChange(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList, apFunctionRedundancyStatus& redundancyStatus);
    bool applyIndexStateChange(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList, apFunctionRedundancyStatus& redundancyStatus);
    bool applySecondaryColorStateChange(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList, apFunctionRedundancyStatus& redundancyStatus);
    bool applyMaterialStateChange(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList, apFunctionRedundancyStatus& redundancyStatus);

    // Material specific functions:
    bool applyShininessMaterialStateChange(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList, apFunctionRedundancyStatus& redundancyStatus);
    bool getMaterialFaceAndPname(int argumentAmount, va_list& pCurrentArgument, GLenum& face, GLenum& pname);
    bool getMaterial4Values(va_list& pCurrentArgument, float& value1, float& value2, float& value3, float& value4);
    bool getListOfMaterialStateVariableToCompareTo(GLenum pname, GLenum face, gtVector<apOpenGLStateVariableId>& stateVariableIdsVector);

    // Color arguments extraction help functions:
    bool getColorFloatRGBAValuesFromByte(int argumentsAmount, va_list& pArgumentList, GLfloat& rAsFloat, GLfloat& gAsFloat, GLfloat& bAsFloat, GLfloat& aAsFloat, bool isPointer, bool is4Arguments);
    bool getColorFloatRGBAValuesFromUByte(int argumentsAmount, va_list& pArgumentList, GLfloat& rAsFloat, GLfloat& gAsFloat, GLfloat& bAsFloat, GLfloat& aAsFloat, bool isPointer, bool is4Arguments);
    bool getColorFloatRGBAValuesFromFloat(int argumentsAmount, va_list& pArgumentList, GLfloat& rAsFloat, GLfloat& gAsFloat, GLfloat& bAsFloat, GLfloat& aAsFloat, bool isPointer, bool is4Arguments);
    bool getColorFloatRGBAValuesFromDouble(int argumentsAmount, va_list& pArgumentList, GLfloat& rAsFloat, GLfloat& gAsFloat, GLfloat& bAsFloat, GLfloat& aAsFloat, bool isPointer, bool is4Arguments);
    bool getColorFloatRGBAValuesFromInt(int argumentsAmount, va_list& pArgumentList, GLfloat& rAsFloat, GLfloat& gAsFloat, GLfloat& bAsFloat, GLfloat& aAsFloat, bool isPointer, bool is4Arguments);
    bool getColorFloatRGBAValuesFromUInt(int argumentsAmount, va_list& pArgumentList, GLfloat& rAsFloat, GLfloat& gAsFloat, GLfloat& bAsFloat, GLfloat& aAsFloat, bool isPointer, bool is4Arguments);
    bool getColorFloatRGBAValuesFromShort(int argumentsAmount, va_list& pArgumentList, GLfloat& rAsFloat, GLfloat& gAsFloat, GLfloat& bAsFloat, GLfloat& aAsFloat, bool isPointer, bool is4Arguments);
    bool getColorFloatRGBAValuesFromUShort(int argumentsAmount, va_list& pArgumentList, GLfloat& rAsFloat, GLfloat& gAsFloat, GLfloat& bAsFloat, GLfloat& aAsFloat, bool isPointer, bool is4Arguments);
    bool getColorFloatRGBAValuesFromFixed(int argumentsAmount, va_list& pArgumentList, GLfloat& rAsFloat, GLfloat& gAsFloat, GLfloat& bAsFloat, GLfloat& aAsFloat);

    // Single argument extraction functions:
    bool getFloatValueFromShortArg(int argumentsAmount, va_list& pArgumentList, GLfloat& argumentAsFloat, bool isPointer);
    bool getFloatValueFromIntArg(int argumentsAmount, va_list& pArgumentList, GLfloat& argumentAsFloat, bool isPOinter);
    bool getFloatValueFromDoubleArg(int argumentsAmount, va_list& pArgumentList, GLfloat& argumentAsFloat, bool isPointer);
    bool getFloatValueFromFloatArg(int argumentsAmount, va_list& pArgumentList, GLfloat& argumentAsFloat, bool isPointer);
    bool getFloatValueFromUByteArgs(int argumentsAmount, va_list& pArgumentList, GLfloat& argumentAsFloat, bool isPointer);
private:

    gsStateVariablesSnapshot* _pStateVariableSnapShot;
};


#endif  // __GSSTATECHANGEEXECUTOR
