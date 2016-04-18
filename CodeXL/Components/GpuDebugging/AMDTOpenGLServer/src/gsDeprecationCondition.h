//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsDeprecationCondition.h
///
//==================================================================================

//------------------------------ gsDeprecationAnalyzer.h ------------------------------

#ifndef __GSDEPRECETIONCONDITION
#define __GSDEPRECETIONCONDITION

// Forward declarations:
class gsRenderContextMonitor;

// Infra:
#include <AMDTAPIClasses/Include/apFunctionDeprecation.h>
#include <AMDTAPIClasses/Include/apAPIVersion.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>

// ----------------------------------------------------------------------------------
// class Name:   gsDeprecationCondition
// General Description: The class represent a function call deprecation condition
// Author:               Sigal Algranaty
// Creation Date:        9/3/2008
// ----------------------------------------------------------------------------------
class gsDeprecationCondition
{
public:
    gsDeprecationCondition();
    virtual ~gsDeprecationCondition();

    // Function that check function call parameters, and return the function call deprecation status and details.
    // The functions are virtual and implemented differently for each deprecation condition:
    virtual bool checkFunctionCallDeprecationStatus(gsRenderContextMonitor* pContextMonitor, int argumentsAmount, va_list& pArgumentList, apFunctionDeprecationStatus& functionDeprecationStatus) = 0;
    virtual bool checkFunctionCallDeprecationDetails(const gtList<const apParameter*>& functionArguments, apFunctionDeprecation& functionDeprecationDetails) = 0;
    virtual bool isArgumentValueDeprecationCondition() const;

protected:
    // Translate an argument to a apParameter class from the list of OpenGL arguments:
    bool extractParameterValueFromArgs(gsRenderContextMonitor* pContextMonitor, int argumentsAmount, va_list& pArgumentList, int argumentIndex, const apParameter*& pParamValue);

private:
    apAPIVersion _deprecatedAtVersion;
    apAPIVersion _removedAtVersion;
};

// ----------------------------------------------------------------------------------
// Class Name:           gsArgumentValueEqualDeprecationCondition : public gsDeprecationCondition
// General Description: Represent a deprecated function call argument value
// Author:               Sigal Algranaty
// Creation Date:        9/3/2009
// ----------------------------------------------------------------------------------
class gsArgumentValueEqualDeprecationCondition : public gsDeprecationCondition
{
public:
    gsArgumentValueEqualDeprecationCondition(int argumentIndex, const gtString& argumentName, apFunctionDeprecationStatus deprecationStatus);
    ~gsArgumentValueEqualDeprecationCondition();

    // Override base class virtual deprecation test:
    virtual bool checkFunctionCallDeprecationStatus(gsRenderContextMonitor* pContextMonitor, int argumentsAmount, va_list& pArgumentList, apFunctionDeprecationStatus& functionDeprecationStatus);
    virtual bool checkFunctionCallDeprecationDetails(const gtList<const apParameter*>& functionArguments, apFunctionDeprecation& functionDeprecationDetails);
    virtual bool isArgumentValueDeprecationCondition() const;

    void addInvalidArgumentValue(apParameter* pInvalidParameterValue);
    int numberOfInvalidArgumentValues() const {return (int)_invalidArgumentValues.size();};
    const apParameter* invalidArgumentValue(int valueIndex) const;
    apFunctionDeprecationStatus functionDeprecationStatus() const {return _deprecationStatus;};

private:
    int _argumentIndex;
    apFunctionDeprecationStatus _deprecationStatus;
    gtPtrVector<apParameter*> _invalidArgumentValues;
    gtString _argumentName;
};


enum gsArgValueComparisonType
{
    GS_EQUAL,                   // Value should be equal
    GS_GREATER,                 // Value should greater than
    GS_SMALLER                  // Value should be smaller than
};
// ----------------------------------------------------------------------------------
// Class Name:           gsArgumentValueComparisonDeprecationCondition : public gsDeprecationCondition
// General Description: Represent a deprecated function call argument value with comparison
// Author:               Sigal Algranaty
// Creation Date:        23/3/2009
// ----------------------------------------------------------------------------------
class gsArgumentValueComparisonDeprecationCondition : public gsDeprecationCondition
{
public:
    gsArgumentValueComparisonDeprecationCondition(int argumentIndex, const gtString& argumentName, GLfloat valueToCompare, gsArgValueComparisonType compareType, apFunctionDeprecationStatus deprecationStatus);
    ~gsArgumentValueComparisonDeprecationCondition();

    // Override base class virtual deprecation test:
    virtual bool checkFunctionCallDeprecationStatus(gsRenderContextMonitor* pContextMonitor, int argumentsAmount, va_list& pArgumentList, apFunctionDeprecationStatus& functionDeprecationStatus);
    virtual bool checkFunctionCallDeprecationDetails(const gtList<const apParameter*>& functionArguments, apFunctionDeprecation& functionDeprecationDetails);

private:
    gtString _argumentName;
    int _argumentIndex;
    apFunctionDeprecationStatus _deprecationStatus;
    GLfloat _valueToCompare;
    gsArgValueComparisonType _compareType;

};



// gsUGeneratedObjectType - represent an OpenGL object type
enum gsUGeneratedObjectType
{
    AP_GL_TEXTURE = 1,
    AP_GL_VBO = 2,
    AP_GL_DISPLAY_LIST = 3,
    AP_GL_PROGRAM = 4

                    /* unsupported deprecations:
                    glGenFencesAPPLE
                    glGenSymbolsEXT
                    glGenQueries
                    */
};

// ----------------------------------------------------------------------------------
// Class Name:           gsUngeneratedObjectDeprecationCondition : public gsDeprecationCondition
// General Description: Represent a deprecated function call - when a function is using an
//                      object before the glGen* appropriate function was called
// Author:               Sigal Algranaty
// Creation Date:        9/3/2009
// ----------------------------------------------------------------------------------
class gsUngeneratedObjectDeprecationCondition : public gsDeprecationCondition
{
public:
    gsUngeneratedObjectDeprecationCondition(int argumentIndex, gsUGeneratedObjectType objectType);
    ~gsUngeneratedObjectDeprecationCondition();

    // Override base class virtual deprecation test:
    virtual bool checkFunctionCallDeprecationStatus(gsRenderContextMonitor* pContextMonitor, int argumentsAmount, va_list& pArgumentList, apFunctionDeprecationStatus& functionDeprecationStatus);
    virtual bool checkFunctionCallDeprecationDetails(const gtList<const apParameter*>& functionArguments, apFunctionDeprecation& functionDeprecationDetails);

private:
    // The argument that contain the object name:
    int _argumentIndex;

    // The object type:
    gsUGeneratedObjectType _objectType;
};

// ----------------------------------------------------------------------------------
// Class Name:           gsUnboundedVertexArrayPointerDeprecationCondition : public gsDeprecationCondition
// General Description: Represent a call to glVertexPointer with an unbound vertex array object
// Author:               Sigal Algranaty
// Creation Date:        2/4/2009
// ----------------------------------------------------------------------------------
class gsUnboundedVertexArrayPointerDeprecationCondition : public gsDeprecationCondition
{
public:
    gsUnboundedVertexArrayPointerDeprecationCondition();
    ~gsUnboundedVertexArrayPointerDeprecationCondition();

    // Override base class virtual deprecation test:
    virtual bool checkFunctionCallDeprecationStatus(gsRenderContextMonitor* pContextMonitor, int argumentsAmount, va_list& pArgumentList, apFunctionDeprecationStatus& functionDeprecationStatus);
    virtual bool checkFunctionCallDeprecationDetails(const gtList<const apParameter*>& functionArguments, apFunctionDeprecation& functionDeprecationDetails);
};



#endif  // __GSDEPRECETIONCONDITION
