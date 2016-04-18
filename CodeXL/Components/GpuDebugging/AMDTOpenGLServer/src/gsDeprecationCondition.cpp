//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsDeprecationCondition.cpp
///
//==================================================================================

//------------------------------ gsDeprecationCondition.cpp ------------------------------

// C:
#include <AMDTOSWrappers/Include/osStdLibIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>
#include <AMDTAPIClasses/Include/apOpenGLParameters.h>

// Local:
#include <src/gsOpenGLMonitor.h>
#include <src/gsDeprecationCondition.h>
#include <src/gsGlobalVariables.h>

// ---------------------------------------------------------------------------
// Name:        gsDeprecationCondition::gsDeprecationCondition
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        9/3/2008
// ---------------------------------------------------------------------------
gsDeprecationCondition::gsDeprecationCondition(): _deprecatedAtVersion(AP_GL_VERSION_NONE), _removedAtVersion(AP_GL_VERSION_NONE)
{
}


// ---------------------------------------------------------------------------
// Name:        gsDeprecationCondition::~gsDeprecationCondition
// Description: Destructor.
// Author:      Sigal Algranaty
// Date:        9/3/2008
// ---------------------------------------------------------------------------
gsDeprecationCondition::~gsDeprecationCondition()
{
}

// ---------------------------------------------------------------------------
// Name:        gsDeprecationCondition::isArgumentValueDeprecationCondition
// Description: Only overridden by gsArgumentValueEqualDeprecationCondition
// Author:      Uri Shomroni
// Date:        4/1/2010
// ---------------------------------------------------------------------------
bool gsDeprecationCondition::isArgumentValueDeprecationCondition() const
{
    return false;
}

// ---------------------------------------------------------------------------
// Name:        gsDeprecationCondition::extractParameterValueFromArgs
// Description: Extract an enum argument value from argument list
// Arguments: int argumentsAmount - the arguments amount
//            va_list& pArgumentList - the argument list
//            int argumentIndex - the parameter location in the list
//            apParameter*& pParamValue
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/3/2009
// ---------------------------------------------------------------------------
bool gsDeprecationCondition::extractParameterValueFromArgs(gsRenderContextMonitor* pContextMonitor, int argumentsAmount, va_list& pArgumentList, int argumentIndex, const apParameter*& pParamValue)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(argumentIndex <= argumentsAmount)
    {
        va_list pCurrentArgument;
        va_copy(pCurrentArgument, pArgumentList);

        // Define a parameter to iterate the list:
        int currentArgumentIndex = 1;

        // While there are still arguments:
        while (currentArgumentIndex <= argumentsAmount)
        {
            // Get and log the argument type:
            osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));

            // Get the static parameters according to type vector:
            apParameter** pTransferableObjTypeToParameter = pContextMonitor->callsHistoryLogger()->getStaticTransferableObjTypeToParameterVector();

            // Get a parameter object that match this argument type:
            apParameter* pStatParameter = pTransferableObjTypeToParameter[argumentType];

            if (pStatParameter)
            {
                // Read the parameter from the arguments list:
                pStatParameter->readValueFromArgumentsList(pCurrentArgument);

                // If this is the argument we are looking for, stop the loop and get the value:
                if (currentArgumentIndex == argumentIndex)
                {
                    pParamValue = pStatParameter;
                    retVal = true;
                    break;
                }
            }
            else
            {
                // We failed to find a parameter that match this argument type:
                GT_ASSERT(0);

                // Exit the loop:
                currentArgumentIndex = argumentsAmount;
            }

            // Increment the current argument index:
            currentArgumentIndex++;
        }

        // Free the arguments pointer:
        va_end(pCurrentArgument);
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsArgumentValueEqualDeprecationCondition::gsArgumentValueEqualDeprecationCondition
// Description: Constructor
// Arguments: int argumentIndex
//            const gtString& argumentName
// Return Val:
// Author:      Sigal Algranaty
// Date:        9/3/2009
// ---------------------------------------------------------------------------
gsArgumentValueEqualDeprecationCondition::gsArgumentValueEqualDeprecationCondition(int argumentIndex, const gtString& argumentName, apFunctionDeprecationStatus deprecationStatus)
    : gsDeprecationCondition(), _argumentIndex(argumentIndex), _deprecationStatus(deprecationStatus), _argumentName(argumentName)
{
}


// ---------------------------------------------------------------------------
// Name:        ~gsArgumentValueEqualDeprecationCondition::gsArgumentValueEqualDeprecationCondition
// Description: Destructor
// Return Val:
// Author:      Sigal Algranaty
// Date:        9/3/2009
// ---------------------------------------------------------------------------
gsArgumentValueEqualDeprecationCondition::~gsArgumentValueEqualDeprecationCondition()
{
    _invalidArgumentValues.deleteElementsAndClear();
}

// ---------------------------------------------------------------------------
// Name:        gsArgumentValueEqualDeprecationCondition::checkFunctionCallDeprecationStatus
// Description: Check the function call deprecation condition against an OpenGL arguments list
// Arguments: int argumentsAmount
//            va_list& pArgumentList
//            apFunctionDeprecationStatus& functionDeprecationStatus
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/3/2009
// ---------------------------------------------------------------------------
bool gsArgumentValueEqualDeprecationCondition::checkFunctionCallDeprecationStatus(gsRenderContextMonitor* pContextMonitor, int argumentsAmount, va_list& pArgumentList, apFunctionDeprecationStatus& functionDeprecationStatus)
{
    bool retVal = false;

    // Initialize deprecation status:
    functionDeprecationStatus = AP_DEPRECATION_NONE;

    // Get the relevant argument value:
    GT_IF_WITH_ASSERT(_argumentIndex <= argumentsAmount)
    {
        const apParameter* pArgumentValue = NULL;
        bool rc = extractParameterValueFromArgs(pContextMonitor, argumentsAmount, pArgumentList, _argumentIndex, pArgumentValue);
        GT_ASSERT(rc);
        retVal = rc;

        // Initialize deprecation status to none:
        functionDeprecationStatus = AP_DEPRECATION_NONE;

        // We have the argument value in hand, we should test it against each of the
        // invalid parameter possible values:
        for (int i = 0; i < (int)_invalidArgumentValues.size(); i++)
        {
            // Get the current invalid argument value:
            const apParameter* pCurrentInvalidArgValue = _invalidArgumentValues[i];

            if (pCurrentInvalidArgValue != NULL)
            {
                // Check if the value equals to the function call argument:
                if (pArgumentValue->type() == pCurrentInvalidArgValue->type())
                {
                    if (pArgumentValue->compareToOther(*pCurrentInvalidArgValue))
                    {
                        // Found argument value deprecation:
                        functionDeprecationStatus = _deprecationStatus;
                        break;
                    }
                }
            }
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsArgumentValueEqualDeprecationCondition::checkFunctionCallDeprecationDetails
// Description: Check the function call deprecation condition against an a list
//              of parameters
// Arguments: const gtList<const apParameter*>& functionArguments
//            apFunctionDeprecation& functionDeprecationDetails
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/3/2009
// ---------------------------------------------------------------------------
bool gsArgumentValueEqualDeprecationCondition::checkFunctionCallDeprecationDetails(const gtList<const apParameter*>& functionArguments, apFunctionDeprecation& functionDeprecationDetails)
{
    bool retVal = false;

    // Get the relevant parameter:
    GT_IF_WITH_ASSERT(_argumentIndex <= (int)functionArguments.size())
    {
        retVal = true;

        const apParameter* pParam = NULL;
        int currentIndex = 1;

        gtList<const apParameter*>::const_iterator iter = functionArguments.begin();
        gtList<const apParameter*>::const_iterator endIter = functionArguments.end();

        for (iter = functionArguments.begin(); iter != endIter; iter++, currentIndex++)
        {
            // Found the parameter:
            if (currentIndex == _argumentIndex)
            {
                pParam = *iter;
            }
        }

        // Initialize deprecation status to none:
        functionDeprecationDetails.setStatus(AP_DEPRECATION_NONE);

        // We have the argument value in hand, we should test it against each of the
        // invalid parameter possible values:
        for (int i = 0; i < (int)_invalidArgumentValues.size(); i++)
        {
            // Get the current invalid argument value:
            const apParameter* pCurrentInvalidArgValue = _invalidArgumentValues[i];

            if (pCurrentInvalidArgValue != NULL)
            {
                // Check if the value equals to the function call argument:
                if (pParam->type() == pCurrentInvalidArgValue->type())
                {
                    if (pParam->compareToOther(*pCurrentInvalidArgValue))
                    {
                        // Found argument value deprecation:
                        functionDeprecationDetails.setStatus(_deprecationStatus);
                        functionDeprecationDetails.setArgumentIndex(_argumentIndex);
                        gtString argValueAsString;
                        pCurrentInvalidArgValue->valueAsString(argValueAsString);
                        functionDeprecationDetails.setReasonString(argValueAsString);
                        break;
                    }
                }
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsArgumentValueEqualDeprecationCondition::isArgumentValueDeprecationCondition
// Description: Overriders gsDeprecationCondition
// Author:      Uri Shomroni
// Date:        4/1/2010
// ---------------------------------------------------------------------------
bool gsArgumentValueEqualDeprecationCondition::isArgumentValueDeprecationCondition() const
{
    return true;
}

// ---------------------------------------------------------------------------
// Name:        gsArgumentValueEqualDeprecationCondition::addInvalidArgumentValue
// Description:
// Arguments: apParameter* pInvalidParameterValue - the parameter value
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/3/2009
// ---------------------------------------------------------------------------
void gsArgumentValueEqualDeprecationCondition::addInvalidArgumentValue(apParameter* pInvalidParameterValue)
{
    _invalidArgumentValues.push_back(pInvalidParameterValue);
}

// ---------------------------------------------------------------------------
// Name:        gsArgumentValueEqualDeprecationCondition::invalidArgumentValue
// Description: Accessor to the disallowed parameter values
// Author:      Uri Shomroni
// Date:        4/1/2010
// ---------------------------------------------------------------------------
const apParameter* gsArgumentValueEqualDeprecationCondition::invalidArgumentValue(int valueIndex) const
{
    const apParameter* retVal = NULL;

    if ((valueIndex >= 0) && (valueIndex < (int)_invalidArgumentValues.size()))
    {
        retVal = _invalidArgumentValues[valueIndex];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsArgumentValueComparisonDeprecationCondition::gsArgumentValueComparisonDeprecationCondition
// Description: Constructor
// Arguments: int argumentIndex - the argument index in the function prototype
//            const gtString& argumentName - the name of the argument
//            GLfloat valueToCompare - the value to compare to
//            gsArgValueComparisonType compareType - state if value should be equal/greater than/ smaller than
//            apFunctionDeprecationStatus deprecationStatus
// Return Val:
// Author:      Sigal Algranaty
// Date:        23/3/2009
// ---------------------------------------------------------------------------
gsArgumentValueComparisonDeprecationCondition::gsArgumentValueComparisonDeprecationCondition(int argumentIndex, const gtString& argumentName, GLfloat valueToCompare, gsArgValueComparisonType compareType, apFunctionDeprecationStatus deprecationStatus)
    : gsDeprecationCondition(), _argumentName(argumentName), _argumentIndex(argumentIndex), _deprecationStatus(deprecationStatus), _valueToCompare(valueToCompare), _compareType(compareType)
{
}

// ---------------------------------------------------------------------------
// Name:        ~gsArgumentValueComparisonDeprecationCondition::gsArgumentValueComparisonDeprecationCondition
// Description: Destructor
// Return Val:
// Author:      Sigal Algranaty
// Date:        23/3/2009
// ---------------------------------------------------------------------------
gsArgumentValueComparisonDeprecationCondition::~gsArgumentValueComparisonDeprecationCondition()
{
}

// ---------------------------------------------------------------------------
// Name:        gsArgumentValueComparisonDeprecationCondition::checkFunctionCallDeprecationStatus
// Description: Check the function call deprecation condition against an OpenGL arguments list
// Arguments: int argumentsAmount
//            va_list& pArgumentList
//            apFunctionDeprecationStatus& functionDeprecationStatus
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/3/2009
// ---------------------------------------------------------------------------
bool gsArgumentValueComparisonDeprecationCondition::checkFunctionCallDeprecationStatus(gsRenderContextMonitor* pContextMonitor, int argumentsAmount, va_list& pArgumentList, apFunctionDeprecationStatus& functionDeprecationStatus)
{
    bool retVal = false;

    // Initialize deprecation status:
    functionDeprecationStatus = AP_DEPRECATION_NONE;

    // Get the relevant argument value:
    GT_IF_WITH_ASSERT(_argumentIndex <= argumentsAmount)
    {
        const apParameter* pArgumentValue = NULL;
        bool rc = extractParameterValueFromArgs(pContextMonitor, argumentsAmount, pArgumentList, _argumentIndex, pArgumentValue);
        GT_ASSERT(rc);

        // Value is expected to be float/int value:
        bool paramValueExtracted = true;
        GLfloat argValueAsFloat = 0.0;

        if (pArgumentValue->type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
        {
            apGLfloatParameter* pFloatParam = (apGLfloatParameter*)pArgumentValue;
            GT_IF_WITH_ASSERT(pFloatParam != NULL)
            {
                argValueAsFloat = pFloatParam->value();
            }
        }
        else if (pArgumentValue->type() == OS_TOBJ_ID_GL_INT_PARAMETER)
        {
            apGLintParameter* pIntParam = (apGLintParameter*)pArgumentValue;
            GT_IF_WITH_ASSERT(pIntParam != NULL)
            {
                argValueAsFloat = (GLfloat)pIntParam->value();
            }
        }
        else
        {
            paramValueExtracted = false;
        }

        GT_IF_WITH_ASSERT(paramValueExtracted)
        {
            switch (_compareType)
            {
                case GS_EQUAL:
                {
                    if (argValueAsFloat != _valueToCompare)
                    {
                        functionDeprecationStatus = _deprecationStatus;
                    }

                    break;
                }

                case GS_GREATER:
                {
                    if (argValueAsFloat < _valueToCompare)
                    {
                        functionDeprecationStatus = _deprecationStatus;
                    }

                    break;
                }

                case GS_SMALLER:
                {
                    if (argValueAsFloat > _valueToCompare)
                    {
                        functionDeprecationStatus = _deprecationStatus;
                    }

                    break;
                }

                default:
                {
                    GT_ASSERT(0);
                }
            }

            retVal = true;
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsArgumentValueComparisonDeprecationCondition::checkFunctionCallDeprecationDetails
// Description: Check the function call deprecation condition against an a list
//              of parameters
// Arguments: const gtList<const apParameter*>& functionArguments
//            apFunctionDeprecation& functionDeprecationDetails
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/3/2009
// ---------------------------------------------------------------------------
bool gsArgumentValueComparisonDeprecationCondition::checkFunctionCallDeprecationDetails(const gtList<const apParameter*>& functionArguments, apFunctionDeprecation& functionDeprecationDetails)
{
    bool retVal = false;

    // Get the relevant parameter:
    GT_IF_WITH_ASSERT(_argumentIndex <= (int)functionArguments.size())
    {
        retVal = true;

        const apParameter* pParam = NULL;
        int currentIndex = 1;

        gtList<const apParameter*>::const_iterator iter = functionArguments.begin();
        gtList<const apParameter*>::const_iterator endIter = functionArguments.end();

        for (iter = functionArguments.begin(); iter != endIter; iter++, currentIndex++)
        {
            // Found the parameter:
            if (currentIndex == _argumentIndex)
            {
                pParam = *iter;
            }
        }

        // Initialize deprecation status to none:
        functionDeprecationDetails.setStatus(AP_DEPRECATION_NONE);

        // Value is expected to be float value:
        GT_IF_WITH_ASSERT(pParam->type() == OS_TOBJ_ID_GL_FLOAT_PARAMETER)
        {
            apGLfloatParameter* pFloatParam = (apGLfloatParameter*)pParam;
            GT_IF_WITH_ASSERT(pFloatParam != NULL)
            {
                // Get the function argument value:
                GLfloat argValue = pFloatParam->value();

                gtString argValueAsString;
                argValueAsString.appendFormattedString(L"%f", argValue);
                functionDeprecationDetails.setReasonString(argValueAsString);
                functionDeprecationDetails.setArgumentIndex(_argumentIndex);


                switch (_compareType)
                {
                    case GS_EQUAL:
                    {
                        if (argValue != _valueToCompare)
                        {
                            functionDeprecationDetails.setStatus(_deprecationStatus);
                        }

                        break;
                    }

                    case GS_GREATER:
                    {
                        if (argValue < _valueToCompare)
                        {
                            functionDeprecationDetails.setStatus(_deprecationStatus);
                        }

                        break;
                    }

                    case GS_SMALLER:
                    {
                        if (argValue > _valueToCompare)
                        {
                            functionDeprecationDetails.setStatus(_deprecationStatus);
                        }

                        break;
                    }

                    default:
                    {
                        GT_ASSERT(0);
                    }
                }

                retVal = true;
            }
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsUngeneratedObjectDeprecationCondition::gsUngeneratedObjectDeprecationCondition
// Description: Constructor
// Arguments: int argumentIndex
//            gsUGeneratedObjectType objectType
// Return Val:
// Author:      Sigal Algranaty
// Date:        9/3/2009
// ---------------------------------------------------------------------------
gsUngeneratedObjectDeprecationCondition::gsUngeneratedObjectDeprecationCondition(int argumentIndex, gsUGeneratedObjectType objectType)
    : _argumentIndex(argumentIndex), _objectType(objectType)
{
}

// ---------------------------------------------------------------------------
// Name:        gsUngeneratedObjectDeprecationCondition::~gsUngeneratedObjectDeprecationCondition
// Description: Destructor
// Arguments: int argumentIndex
//            gsUGeneratedObjectType objectType
// Return Val:
// Author:      Sigal Algranaty
// Date:        9/3/2009
// ---------------------------------------------------------------------------
gsUngeneratedObjectDeprecationCondition::~gsUngeneratedObjectDeprecationCondition()
{
}


// ---------------------------------------------------------------------------
// Name:        gsUngeneratedObjectDeprecationCondition::checkFunctionCallDeprecationStatus
// Description: Checks if an object was used before its generation
// Arguments: int argumentAmount
//            va_list& pArgumentList
//            apFunctionDeprecationStatus& functionDeprecationStatus
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/3/2009
// ---------------------------------------------------------------------------
bool gsUngeneratedObjectDeprecationCondition::checkFunctionCallDeprecationStatus(gsRenderContextMonitor* pContextMonitor, int argumentAmount, va_list& pArgumentList, apFunctionDeprecationStatus& functionDeprecationStatus)
{
    bool retVal = false;

    // Initialize return value:
    functionDeprecationStatus = AP_DEPRECATION_NONE;

    // Get the object name from the argument list;
    GLuint objectName;
    const apParameter* pObjectNameParam = NULL;
    bool rc = extractParameterValueFromArgs(pContextMonitor, argumentAmount, pArgumentList, _argumentIndex, pObjectNameParam);
    GT_IF_WITH_ASSERT(rc)
    {
        GT_IF_WITH_ASSERT(pObjectNameParam != NULL)
        {
            GT_IF_WITH_ASSERT(pObjectNameParam->type() == OS_TOBJ_ID_GL_UINT_PARAMETER)
            {
                apGLuintParameter* pUIntParam = (apGLuintParameter*)pObjectNameParam;
                GT_IF_WITH_ASSERT(pUIntParam != NULL)
                {
                    objectName = pUIntParam->value();

                    // Check if the object name was generated by OpenGL:
                    switch (_objectType)
                    {
                        case AP_GL_VBO:
                        {
                            if (objectName != 0)
                            {
                                // Check the VBO monitor for object with the name:
                                gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

                                if (pCurrentThreadRenderContextMonitor)
                                {
                                    gsVBOMonitor* vboMonitor = pCurrentThreadRenderContextMonitor->vboMonitor();
                                    GT_IF_WITH_ASSERT(vboMonitor != NULL)
                                    {
                                        apGLVBO* pVBOObject = vboMonitor->getVBODetails(objectName);

                                        if (pVBOObject == NULL)
                                        {
                                            functionDeprecationStatus = AP_DEPRECATION_APPLICATION_GENERATED_NAMES;
                                        }

                                        retVal = true;
                                    }
                                }
                            }
                        }
                        break;

                        case AP_GL_TEXTURE:
                        {
                            // Do not perform the test for texture default name (even though it is not generated by Gen function):
                            if (objectName != 0)
                            {
                                // Check the textures monitor for object with the name:
                                gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

                                if (pCurrentThreadRenderContextMonitor)
                                {
                                    gsTexturesMonitor* pTexMonitor = pCurrentThreadRenderContextMonitor->texturesMonitor();
                                    GT_IF_WITH_ASSERT(pTexMonitor != NULL)
                                    {
                                        apGLTexture* pObject = pTexMonitor->getTextureObjectDetails(objectName);

                                        if (pObject == NULL)
                                        {
                                            functionDeprecationStatus = AP_DEPRECATION_APPLICATION_GENERATED_NAMES;
                                        }

                                        retVal = true;
                                    }
                                }
                            }
                        }
                        break;

                        case AP_GL_DISPLAY_LIST:
                        {
                            if (objectName != 0)
                            {
                                // Check the display list monitor for object with the name:
                                gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

                                if (pCurrentThreadRenderContextMonitor)
                                {
                                    gsDisplayListMonitor* pDisplayListMonitor = pCurrentThreadRenderContextMonitor->displayListsMonitor();
                                    GT_IF_WITH_ASSERT(pDisplayListMonitor != NULL)
                                    {
                                        apGLDisplayList* pDisplayList = pDisplayListMonitor->getDisplayListDetails(objectName);

                                        if (pDisplayList == NULL)
                                        {
                                            functionDeprecationStatus = AP_DEPRECATION_APPLICATION_GENERATED_NAMES;
                                        }

                                        retVal = true;
                                    }
                                }
                            }
                        }
                        break;

                        case AP_GL_PROGRAM:
                        {
                            if (objectName != (GLuint)NULL)
                            {
                                // Check the program and shaders monitor for object with the name:
                                gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

                                if (pCurrentThreadRenderContextMonitor)
                                {
                                    gsProgramsAndShadersMonitor* pProgramsAndShadersMonitor = pCurrentThreadRenderContextMonitor->programsAndShadersMonitor();
                                    GT_IF_WITH_ASSERT(pProgramsAndShadersMonitor != NULL)
                                    {
                                        const apGLShaderObject* pObject = pProgramsAndShadersMonitor->shaderObjectDetails(objectName);

                                        if (pObject == NULL)
                                        {
                                            functionDeprecationStatus = AP_DEPRECATION_APPLICATION_GENERATED_NAMES;
                                        }

                                        retVal = true;
                                    }
                                }
                            }
                        }
                        break;

                        default:
                        {
                            GT_ASSERT_EX(0, L"Unsupported object type");
                        }
                        break;
                    }
                }
            }
        }
    }
    return retVal;

}

// ---------------------------------------------------------------------------
// Name:        gsUngeneratedObjectDeprecationCondition::checkFunctionCallDeprecationDetails
// Description: Checks if an object was used before its generation and return the relevant
//              deprecation details
// Arguments: const gtList<const apParameter*>& functionArguments
//            apFunctionDeprecation& functionDeprecationDetails
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/3/2009
// ---------------------------------------------------------------------------
bool gsUngeneratedObjectDeprecationCondition::checkFunctionCallDeprecationDetails(const gtList<const apParameter*>& functionArguments, apFunctionDeprecation& functionDeprecationDetails)
{
    bool retVal = false;

    // Get the object name from the argument list;
    GLuint objectName;
    const apParameter* pObjectNameParam = NULL;
    int currentIndex = 1;

    gtList<const apParameter*>::const_iterator iter = functionArguments.begin();
    gtList<const apParameter*>::const_iterator endIter = functionArguments.end();

    for (iter = functionArguments.begin(); iter != endIter; iter++, currentIndex++)
    {
        // Found the parameter:
        if (currentIndex == _argumentIndex)
        {
            pObjectNameParam = *iter;
        }
    }

    GT_IF_WITH_ASSERT(pObjectNameParam != NULL)
    {
        GT_IF_WITH_ASSERT(pObjectNameParam->type() == OS_TOBJ_ID_GL_UINT_PARAMETER)
        {
            apGLintParameter* pIntParam = (apGLintParameter*)pObjectNameParam;
            GT_IF_WITH_ASSERT(pIntParam != NULL)
            {
                objectName = pIntParam->value();

                // Default object name is valid, and not generated by glGen* functions:
                if (objectName != 0)
                {
                    // Check if the object name was generated by OpenGL:
                    switch (_objectType)
                    {
                        case AP_GL_VBO:
                        {
                            // Check the VBO monitor for object with the name:
                            gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

                            if (pCurrentThreadRenderContextMonitor)
                            {
                                gsVBOMonitor* vboMonitor = pCurrentThreadRenderContextMonitor->vboMonitor();
                                GT_IF_WITH_ASSERT(vboMonitor != NULL)
                                {
                                    apGLVBO* pVBOObject = vboMonitor->getVBODetails(objectName);

                                    if (pVBOObject == NULL)
                                    {
                                        functionDeprecationDetails.setStatus(AP_DEPRECATION_APPLICATION_GENERATED_NAMES);
                                        functionDeprecationDetails.setArgumentIndex(_argumentIndex);
                                        functionDeprecationDetails.setReasonString(L"glGenBuffer");
                                    }
                                }
                                break;
                            }
                        }

                        case AP_GL_TEXTURE:
                        {
                            // Check the VBO monitor for object with the name:
                            gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

                            if (pCurrentThreadRenderContextMonitor)
                            {
                                gsTexturesMonitor* pTexMonitor = pCurrentThreadRenderContextMonitor->texturesMonitor();
                                GT_IF_WITH_ASSERT(pTexMonitor != NULL)
                                {
                                    apGLTexture* pObject = pTexMonitor->getTextureObjectDetails(objectName);

                                    if (pObject == NULL)
                                    {
                                        functionDeprecationDetails.setStatus(AP_DEPRECATION_APPLICATION_GENERATED_NAMES);
                                        functionDeprecationDetails.setArgumentIndex(_argumentIndex);
                                        functionDeprecationDetails.setReasonString(L"glGenTexture");
                                    }
                                }
                            }

                            break;
                        }

                        case AP_GL_DISPLAY_LIST:
                        {
                            // Check the display list monitor for object with the name:
                            gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

                            if (pCurrentThreadRenderContextMonitor)
                            {
                                gsDisplayListMonitor* pDisplayListMonitor = pCurrentThreadRenderContextMonitor->displayListsMonitor();
                                GT_IF_WITH_ASSERT(pDisplayListMonitor != NULL)
                                {
                                    apGLDisplayList* pDisplayList = pDisplayListMonitor->getDisplayListDetails(objectName);

                                    if (pDisplayList == NULL)
                                    {
                                        functionDeprecationDetails.setStatus(AP_DEPRECATION_APPLICATION_GENERATED_NAMES);
                                        functionDeprecationDetails.setArgumentIndex(_argumentIndex);
                                        functionDeprecationDetails.setReasonString(L"glGenLists");
                                    }
                                }
                            }

                            break;
                        }

                        case AP_GL_PROGRAM:
                        {
                            GT_ASSERT_EX(0, L"Unsupported object type");
                            break;
                        }

                        default:
                        {
                            GT_ASSERT_EX(0, L"Unsupported object type");
                        }
                    }
                }
            }
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsUnboundedVertexArrayPointerDeprecationCondition::gsUnboundedVertexArrayPointerDeprecationCondition
// Description: Constructor
// Return Val:
// Author:      Sigal Algranaty
// Date:        2/4/2009
// ---------------------------------------------------------------------------
gsUnboundedVertexArrayPointerDeprecationCondition::gsUnboundedVertexArrayPointerDeprecationCondition()
{
}

// ---------------------------------------------------------------------------
// Name:        gsUnboundedVertexArrayPointerDeprecationCondition::~gsUnboundedVertexArrayPointerDeprecationCondition
// Description: Destructor
// Return Val:
// Author:      Sigal Algranaty
// Date:        2/4/2009
// ---------------------------------------------------------------------------
gsUnboundedVertexArrayPointerDeprecationCondition::~gsUnboundedVertexArrayPointerDeprecationCondition()
{
}


// ---------------------------------------------------------------------------
// Name:        gsUnboundedVertexArrayPointerDeprecationCondition::checkFunctionCallDeprecationStatus
// Description: Checks if an object was used before its generation
// Arguments: int argumentAmount
//            va_list& pArgumentList
//            apFunctionDeprecationStatus& functionDeprecationStatus
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/4/2009
// ---------------------------------------------------------------------------
bool gsUnboundedVertexArrayPointerDeprecationCondition::checkFunctionCallDeprecationStatus(gsRenderContextMonitor* pContextMonitor, int argumentAmount, va_list& pArgumentList, apFunctionDeprecationStatus& functionDeprecationStatus)
{
    (void)(pContextMonitor); // unused
    (void)(argumentAmount); // unused
    (void)(pArgumentList); // unused
    bool retVal = false;

    // Initialize return value:
    functionDeprecationStatus = AP_DEPRECATION_NONE;

    // Check the VBO monitor for object with the name:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        gsVBOMonitor* vboMonitor = pCurrentThreadRenderContextMonitor->vboMonitor();
        GT_IF_WITH_ASSERT(vboMonitor != NULL)
        {
            GLuint vboArrayAttachmentName = vboMonitor->getAttachedVBOName(GL_ARRAY_BUFFER);

            // If no vertex buffer array object is bound:
            if (vboArrayAttachmentName == 0)
            {
                functionDeprecationStatus = AP_DEPRECATION_CLIENT_VERTEX_AND_INDEX_ARRAYS;
            }

            retVal = true;
        }
    }

    return retVal;

}

// ---------------------------------------------------------------------------
// Name:        gsUnboundedVertexArrayPointerDeprecationCondition::checkFunctionCallDeprecationDetails
// Description: Checks if an object was used before its generation and return the relevant
//              deprecation details
// Arguments: const gtList<const apParameter*>& functionArguments
//            apFunctionDeprecation& functionDeprecationDetails
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/3/2009
// ---------------------------------------------------------------------------
bool gsUnboundedVertexArrayPointerDeprecationCondition::checkFunctionCallDeprecationDetails(const gtList<const apParameter*>& functionArguments, apFunctionDeprecation& functionDeprecationDetails)
{
    (void)(functionArguments); // unused
    bool retVal = false;

    // Initialize deprecation status to none:
    functionDeprecationDetails.setStatus(AP_DEPRECATION_NONE);

    // Check the VBO monitor for object with the name:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        gsVBOMonitor* vboMonitor = pCurrentThreadRenderContextMonitor->vboMonitor();
        GT_IF_WITH_ASSERT(vboMonitor != NULL)
        {
            GLenum vboArrayAttachment = vboMonitor->getAttachedVBOName(GL_ARRAY_BUFFER);

            // If no vertex buffer array object is bound:
            if (0 == vboArrayAttachment)
            {
                // Initialize deprecation status to none:
                functionDeprecationDetails.setStatus(AP_DEPRECATION_CLIENT_VERTEX_AND_INDEX_ARRAYS);
            }

            retVal = true;
        }
    }

    return retVal;
}

