//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsStateVariableReader.cpp
///
//==================================================================================

//------------------------------ gsStateVariableReader.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTAPIClasses/Include/apOpenGLStateVariablesManager.h>
#include <AMDTAPIClasses/Include/apStateVariablesSnapShot.h>

// Local:
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsWrappersCommon.h>
#include <src/gsExtensionsManager.h>
#include <src/gsStateVariableReader.h>
#include <src/gsStringConstants.h>

// The maximal amount of OpenGL state variable elements:
#define MAX_AMOUNT_OF_STATE_VARIABLE_ELEMENTS 100


// --------------------- Free functions and templates -----------------------------


// ---------------------------------------------------------------------------
// Name:        gsSetVectorElement
// Description: Sets a parameters vector element.
// Arguments:   pVectorParam - The parameters vector.
//              elementIndex - The index of the element (in the vector).
//              pElementParam - The element to be set.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        15/7/2004
// ---------------------------------------------------------------------------
bool gsSetVectorElement(apVectorParameter* pVectorParam,
                        unsigned int elementIndex,
                        apParameter* pElementParam)
{
    bool retVal = false;

    // Sanity test:
    if (pVectorParam && pElementParam)
    {
        // Add the element into the vector:
        gtAutoPtr<apParameter> aptrParam = pElementParam;
        pVectorParam->setItem(elementIndex, aptrParam);
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsSetMatrixElement
// Description: Sets a parameters matrix element.
// Arguments:   pMatrixParam - The matrix parameter.
//              indexN, indexM - The element position in the matrix.
//              pElementParam - The element to be set
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        18/7/2004
// ---------------------------------------------------------------------------
bool gsSetMatrixElement(apMatrixParameter* pMatrixParam,
                        unsigned int indexN, unsigned int indexM,
                        apParameter* pElementParam)
{
    bool retVal = false;

    // Sanity test:
    if (pMatrixParam && pElementParam)
    {
        // Add the element into the vector:
        gtAutoPtr<apParameter> aptrParam = pElementParam;
        pMatrixParam->setItem(indexN, indexM, aptrParam);
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsGetVectorAsParameter
// Description:
//   Inputs a "C" vector and output an apVectorParameter that represent it.
//
// Template arguments:
//   DataType - The "C" vector data type.
//   ParameterType - The apParameter vector type.
//
// Arguments:
//   intVec - The input "C" vector.
//   amountOfItems - The amount of vector items.
//   aptrFloatVectorAsParam - Will get the output apVectorParameter.
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        22/7/2004
// ---------------------------------------------------------------------------
template <class DataType, class ParameterType>
bool gsGetVectorAsParameter(DataType* dataVec, int amountOfItems,
                            apParameter*& pParam)
{
    bool retVal = false;

    apVectorParameter* pVectorParam = NULL;

    if ((pParam == NULL) || (pParam->type() != OS_TOBJ_ID_VECTOR_PARAMETER))
    {
        // Delete the old parameter:
        apStateVariablesSnapShot::deleteParameter(pParam);
        // Create the vector parameter that will wrap the state variable value:
        pParam = new apVectorParameter(amountOfItems);
    }

    pVectorParam = (apVectorParameter*)pParam;

    if (NULL != pVectorParam)
    {
        retVal = true;

        int vectorSize = (int)pVectorParam->vectorSize();

        // Iterate the vector items:
        for (int i = 0; i < amountOfItems; i++)
        {
            // Try to update the item rather than overwrite it, to improve performance in Analyze mode:
            bool updatedItem = false;

            if (vectorSize > i)
            {
                // Get the item:
                const apParameter* pOldElementParam = (*pVectorParam)[i];

                if (NULL != pOldElementParam)
                {
                    // Cast up to non-const, to modify the value in place:
                    ((apParameter*)pOldElementParam)->readValueFromPointer(&(dataVec[i]));
                    updatedItem = true;
                }
            }

            // If we could not update, overwrite the item:
            if (!updatedItem)
            {
                apParameter* pElementParam = new ParameterType(dataVec[i]);

                GT_IF_WITH_ASSERT(pElementParam != NULL)
                {
                    DataType data = dataVec[i];
                    pElementParam->readValueFromPointer(&data);
                }

                // Add the current item into the output vector:
                bool rc = gsSetVectorElement(pVectorParam, i, (apParameter*)pElementParam);
                retVal = retVal && rc;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsGetMatrixAsParameter
// Description:
//   Inputs a "C" matrix and output an apMatrixParameter that represent it.
//
// Template arguments:
//   DataType - The "C" matrix data type.
//   ParameterType - The apParameter vector type.
//
// Arguments:
//   cMatrix - The input "C" matrix.
//   sizeN, sizeM - The matrix size.
//   aptrMatrixAsParam - Will get the output apMatrixParameter.
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/7/2004
// ---------------------------------------------------------------------------
template <class DataType, class ParameterType>
bool gsGetMatrixAsParameter(DataType* cMatrix, unsigned int sizeN, unsigned int sizeM,
                            apParameter*& pParam)
{
    bool retVal = false;

    // Create the output Matrix parameter:
    apMatrixParameter* pMatrixAsParam = NULL;

    if ((pParam == NULL) || (pParam->type() != OS_TOBJ_ID_MATRIX_PARAMETER))
    {
        // Create the vector parameter that will wrap the state variable value:
        apStateVariablesSnapShot::deleteParameter(pParam);
        pParam = new apMatrixParameter(sizeN, sizeM);
    }

    pMatrixAsParam = (apMatrixParameter*)pParam;
    //const apMatrixParameter* pConstMatrix = (const apMatrixParameter*)pParam;
    retVal = true;

    // Iterate the vector elements:
    for (unsigned int i = 0; i < sizeN; i++)
    {
        for (unsigned int j = 0; j < sizeM; j++)
        {
            // Try to update the item rather than overwrite it, to improve performance in Analyze mode:
            bool updatedItem = false;

            // Get the item:
            const apParameter* pOldElementParam = pMatrixAsParam->element(i, j);

            if (NULL != pOldElementParam)
            {
                // Cast up to non-const, to modify the value in place:
                ((apParameter*)pOldElementParam)->readValueFromPointer(&(cMatrix[i * sizeN + j]));
                updatedItem = true;
            }

            // If we could not update, overwrite the item:
            if (!updatedItem)
            {
                // Add the current element into the output vector:
                apParameter* pElementParam = new ParameterType(cMatrix[i * sizeN + j]);

                // Set matrix element's value:
                GT_IF_WITH_ASSERT(NULL != pElementParam)
                {
                    pElementParam->readValueFromPointer((void*)(&(cMatrix[i * sizeN + j])));
                }

                bool rc = gsSetMatrixElement(pMatrixAsParam, i, j, (apParameter*)pElementParam);
                retVal = retVal && rc;
            }
        }
    }

    return retVal;
}



// --------------------- gsStateVariableReader methods -----------------------------



// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::gsStateVariableReader
// Description: Constructor
// Arguments:   stateVariableId - The state variable id.
// Author:      Yaki Tebeka
// Date:        13/7/2004
// ---------------------------------------------------------------------------
gsStateVariableReader::gsStateVariableReader(int stateVariableId, const gsRenderContextMonitor* pRenderContextMtr)
    : _stateVariableId(stateVariableId), _pMyRenderContextMtr(pRenderContextMtr)
{
}


// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getStateVariableValue
// Description: Returns the value of an OpenGL state variable.
// Arguments: apParameter*& pStateVariableValue - the state variable parameter value:
//              - if the pointer is null - allocate a new parameter value object
//              - if the pStateVariableValue is a apNotAvailableParameter pointer
//                  - destruct it and allocate new relevant parameter
//              - else make it is of the right type, and fill the value
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        13/7/2004
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getStateVariableValue(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Print to the log:
    beforeGettingStateVariableValue();

    // If this is an enum state variable:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();

    bool wasRemoved = false;
    apAPIVersion removedVersion = theStateVarsManager.stateVariableRemovedVersion(_stateVariableId);

    // OpenGL ES does not define a deprecation model:
#ifdef _AMDT_OPENGLSERVER_EXPORTS
    {
        // If this variable was removed:
        if (removedVersion != AP_GL_VERSION_NONE)
        {
            // Check if the version this variable was removed is a past version:
            int removedMajorVersion = 0;
            int removedMinorVersion = 0;
            apOpenGLVersionToInts(removedVersion, removedMajorVersion, removedMinorVersion);

            // Compatibility contexts do not remove older state variables:
            wasRemoved = _pMyRenderContextMtr->isOpenGLVersionOrNewerCoreContext(removedMajorVersion, removedMinorVersion);
        }
    }
#endif // def _AMDT_OPENGLSERVER_EXPORTS

    // If this variable was removed:
    if (wasRemoved)
    {
        // This state variable was removed at this (or a previous) version, return a parameter to show this:
        pStateVariableValue = new apRemovedParameter(removedVersion);
        retVal = true;
    }
    else
    {
        bool isEnumStateVariable = theStateVarsManager.isEnumStateVariable(_stateVariableId);

        if (isEnumStateVariable)
        {
            // Get the enum value:
            retVal = getEnumStateVariableValue(pStateVariableValue);
        }
        else
        {
            // Get the state variable get function:
            apMonitoredFunctionId getFunctionId = theStateVarsManager.stateVariableGetFunctionId(_stateVariableId);

            // Use it to get the state variable value:
            switch (getFunctionId)
            {
                case ap_glGetBooleanv:
                    retVal = getBooleanStateVariableValue(pStateVariableValue);
                    break;

                case ap_glIsEnabled:
                    retVal = getIsEnabledStateVariable(pStateVariableValue);
                    break;

                case ap_glGetIntegerv:
                    retVal = getIntegerStateVariableValue(pStateVariableValue);
                    break;

                case ap_glGetInteger64v:
                    retVal = getInteger64StateVariableValue(pStateVariableValue);
                    break;

                case ap_glGetIntegerui64vNV:
                    retVal = getIntegerui64NVStateVariableValue(pStateVariableValue);
                    break;

                case ap_glGetFloatv:
                    retVal = getFloatStateVariableValue(pStateVariableValue);
                    break;

                case ap_glGetDoublev:
                    retVal = getDoubleStateVariableValue(pStateVariableValue);
                    break;

                case ap_glGetMaterialfv:
                    retVal = getMaterialVariableValue(pStateVariableValue);
                    break;

                case ap_glGetLightfv:
                    retVal = getLightVariableValue(pStateVariableValue);
                    break;

                case ap_glGetClipPlane:
                    retVal = getClipPlaneVariableValue(pStateVariableValue);
                    break;

                case ap_glGetTexEnviv:
                case ap_glGetTexEnvfv:
                    retVal = getTexEnvVariableValue(pStateVariableValue);
                    break;

                case ap_glGetTexGenfv:
                case ap_glGetTexGeniv:
                case ap_glGetTexGendv:
                    retVal = getTexGenVariableValue(pStateVariableValue);
                    break;

                case ap_glGetPointerv:
                    retVal = getPointerVariableValue(pStateVariableValue);
                    break;

                case ap_glGetQueryiv:
                    retVal = getQueryIntVariableValue(pStateVariableValue);
                    break;

                case ap_glGetBufferParameterivARB:
                    retVal = getBufferParameterARBIntValue(pStateVariableValue);
                    break;

                case ap_glGetBufferParameteriv:
                    retVal = getBufferParameterIntValue(pStateVariableValue);
                    break;

                case ap_glGetString:
                    retVal = getStringVariableValue(pStateVariableValue);
                    break;

                case ap_glGetProgramivARB:
                    retVal = getProgramIntVariableValue(pStateVariableValue);
                    break;

                case ap_glGetHandleARB:
                    retVal = getHandleVariableValue(pStateVariableValue);
                    break;

                    // Mac only:
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
#ifdef _GR_OPENGLES_IPHONE
                    // TO_DO iPhone - check if there are EAGL state vars
#else

                case ap_CGLGetParameter:
                case ap_CGLIsEnabled:
                case ap_CGLGetOption:
                    retVal = getCGLStateVariableVariableValue(pStateVariableValue, getFunctionId);
                    break;
#endif
#endif

                    // Windows only:
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

                case ap_wglGetSwapIntervalEXT:
                    retVal = getWGLSwapInterval(pStateVariableValue);
                    break;
#endif

                default:
                {
                    // The below debug assert will be generated in debug mode only:
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
                    const wchar_t* variableName = theStateVarsManager.stateVariableName(_stateVariableId);
                    gtString errorMessage = GS_STR_UnsupportedStateVarGetFunction;
                    errorMessage += variableName;
                    GT_ASSERT_EX(0, errorMessage.asCharArray());
#endif
                }
            }
        }
    }

    // Check for OpenGL errors:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
    GLenum openGLError = gs_stat_realFunctionPointers.glGetError();
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

    // Print to the log:
    afterGettingStateVariableValue(retVal, openGLError);

    // If there was an OpenGL error - fail the function:
    if (openGLError != GL_NO_ERROR)
    {
        retVal = false;

        // Notice that we cannot put an assert here, because some state variables
        // get functions generate an error (Example: Asking for the color index while
        // rendering in RGBA mode)

        // For debugging - do not remove me:
        /*
        const char* name = theStateVarsManager.stateVariableName(_stateVariableId);
        gtString foo = "Debug print - Cannot read the value of: ";
        foo += name;
        OutputDebugString(foo.asCharArray());
        */
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getEnumStateVariableValue
// Description: Reads an enum (GLenum) state variable.
// Arguments:   aptrStateVariableValue - Will get the state variable value.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        24/7/2004
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getEnumStateVariableValue(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Get the state variable OpenGL enum:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    GLenum variableOpenGLEnum = theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId);
    GLenum variableAdditionalOpenGLEnum = theStateVarsManager.stateVariableAdditionalOpenGLParamEnum(_stateVariableId);

    if (variableAdditionalOpenGLEnum == 0)
    {
        // Some variables are dependent on other per-context values to show their existence:
        bool allowVariable = true;

        // GL_DRAW_BUFFERi are dependant on GL_MAX_DRAW_BUFFERS. This is also true for the _ARB and _ATI values,
        // which have the same enums (i.e. GL_DRAW_BUFFER7 = GL_DRAW_BUFFER7_ARB = GL_DRAW_BUFFER7_ATI):
        if ((variableOpenGLEnum >= GL_DRAW_BUFFER0) && (variableOpenGLEnum <= GL_DRAW_BUFFER15))
        {
            int drawBufferIndex = variableOpenGLEnum - GL_DRAW_BUFFER0;
            GT_ASSERT((drawBufferIndex >= 0) && (drawBufferIndex < 16));

            // Get the MAX_DRAW_BUFFERS value:
            GLint maxNumberOfDrawBuffers = -1;
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
            gs_stat_realFunctionPointers.glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxNumberOfDrawBuffers);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);

            // Make sure the current variable is in range:
            allowVariable = ((drawBufferIndex >= 0) && (drawBufferIndex < maxNumberOfDrawBuffers));
        }

        if (allowVariable)
        {
            // Get the state variable value:
            // (Unfortunately, OpenGL does not have a glGet function for GLenum values. Instead, it
            //  uses glGetIntegerv)
            GLint buff[MAX_AMOUNT_OF_STATE_VARIABLE_ELEMENTS];
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
            gs_stat_realFunctionPointers.glGetIntegerv(variableOpenGLEnum, buff);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);

            // If the parameter was not allocated yet, allocate it:
            if ((pStateVariableValue == NULL) || (pStateVariableValue->type() != OS_TOBJ_ID_GL_ENUM_PARAMETER))
            {
                apStateVariablesSnapShot::deleteParameter(pStateVariableValue);
                pStateVariableValue = new apGLenumParameter(*buff);
            }
            else
            {
                // Cast the pointer to the right parameter type, and set it's value:
                apGLenumParameter* pEnumParamter = (apGLenumParameter*)pStateVariableValue;
                pEnumParamter->readValueFromPointer(buff);
            }
        }
        else // !allowVariable
        {
            // This parameter is not available at the moment:
            apStateVariablesSnapShot::deleteParameter(pStateVariableValue);
            pStateVariableValue = new apNotAvailableParameter();
        }

        retVal = true;
    }
    else
    {
        // This is a vector enum variable, which is of a varying size. Get the size:
        GLint numberOfElements = 0;
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
        gs_stat_realFunctionPointers.glGetIntegerv(variableAdditionalOpenGLEnum, &numberOfElements);

        if (numberOfElements > 0)
        {
            // Read the values
            GLint* pValues = new GLint[numberOfElements];
            gs_stat_realFunctionPointers.glGetIntegerv(variableOpenGLEnum, pValues);
            apVectorParameter* pStateVariableValueVector = new apVectorParameter(numberOfElements);

            for (int i = 0; i < numberOfElements; i++)
            {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT) && (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE))
                // Uri, 17/9/09: Mac 64-bit has a problem where glGetIntergerv takes a GLint (= long = 8 bytes) pointer as a parameter, but the output is
                // as int (= 4 bytes). This causes "x" to contain x + (y << 32), "y" to contain width + (height << 32), and "width" and "height" to contain
                // garbage data. So - we cast the pointer to an int* to overcome this:
                GLenum currentValue = (GLenum)(((int*)pValues)[i]);
#else
                GLenum currentValue = (GLenum)(pValues[i]);
#endif
                apGLenumParameter* pCurrentParam = new apGLenumParameter(currentValue);
                gtAutoPtr<apParameter> aptrCurrentParam(pCurrentParam);
                pStateVariableValueVector->setItem(i, aptrCurrentParam);
            }

            apStateVariablesSnapShot::deleteParameter(pStateVariableValue);

            pStateVariableValue = pStateVariableValueVector;

            delete[] pValues;
        }
        else
        {
            // This parameter is not available at the moment:
            apStateVariablesSnapShot::deleteParameter(pStateVariableValue);
            pStateVariableValue = new apNotAvailableParameter();
        }

        retVal = true;

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getBooleanStateVariableValue
// Description: Reads a boolean state variable.
// Arguments:   aptrStateVariableValue - Will get the state variable value.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        18/7/2004
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getBooleanStateVariableValue(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Get the state variable type and OpenGL enum:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    apOpenGLStateVariablesManager::apStateVariableType variableType = theStateVarsManager.stateVariableType(_stateVariableId);
    GLenum variableOpenGLEnum = theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId);

    // Get the state variable value:
    GLboolean buff[MAX_AMOUNT_OF_STATE_VARIABLE_ELEMENTS];
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetBooleanv);
    gs_stat_realFunctionPointers.glGetBooleanv(variableOpenGLEnum, buff);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetBooleanv);

    // If pStateVariableValue is null allocate a new parameter:

    if (variableType == apOpenGLStateVariablesManager::SimpleStateVariable)
    {
        if ((pStateVariableValue == NULL) || (pStateVariableValue->type() != OS_TOBJ_ID_GL_BOOL_PARAMETER))
        {
            apStateVariablesSnapShot::deleteParameter(pStateVariableValue);
            pStateVariableValue = new apGLbooleanParameter(*buff);
        }
        else
        {
            apGLbooleanParameter* pBoolParamter = (apGLbooleanParameter*)pStateVariableValue;
            pBoolParamter->readValueFromPointer(buff);
        }

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getIsEnabledStateVariable
// Description: Reads a state variable that is get using glIsEnabled.
// Arguments:   aptrStateVariableValue - Will get the state variable value.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/9/2004
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getIsEnabledStateVariable(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Get the state variable type and OpenGL enum:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    apOpenGLStateVariablesManager::apStateVariableType variableType = theStateVarsManager.stateVariableType(_stateVariableId);
    GLenum variableOpenGLEnum = theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId);

    // Get the state variable value:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glIsEnabled);
    GLboolean isEnabled = gs_stat_realFunctionPointers.glIsEnabled(variableOpenGLEnum);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glIsEnabled);

    if ((pStateVariableValue == NULL) || (pStateVariableValue->type() != OS_TOBJ_ID_GL_BOOL_PARAMETER))
    {
        if (variableType == apOpenGLStateVariablesManager::SimpleStateVariable)
        {
            apStateVariablesSnapShot::deleteParameter(pStateVariableValue);
            pStateVariableValue = new apGLbooleanParameter(isEnabled);
            retVal = true;
        }
    }
    else
    {
        pStateVariableValue->readValueFromPointer(&isEnabled);
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getStringVariableValue
// Description: Reads a String state variable.
// Arguments:   aptrStateVariableValue - Will get the state variable value.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        28/2/2005
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getStringVariableValue(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Get the state variable type and OpenGL enum:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    apOpenGLStateVariablesManager::apStateVariableType variableType = theStateVarsManager.stateVariableType(_stateVariableId);
    GLenum variableOpenGLEnum = theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId);

    // Get the state variable value:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetString);
    gtASCIIString variableValue = (const char*)(gs_stat_realFunctionPointers.glGetString(variableOpenGLEnum));
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetString);

    if (variableType == apOpenGLStateVariablesManager::SimpleStateVariable)
    {
        if ((pStateVariableValue == NULL) || (pStateVariableValue->type() != OS_TOBJ_ID_GL_STRING_PARAMETER))
        {
            apStateVariablesSnapShot::deleteParameter(pStateVariableValue);
            pStateVariableValue = new apStringParameter(variableValue);
        }
        else
        {
            // Cast the parameter to a string parameter, and set its value:
            apStringParameter* pStringParam = (apStringParameter*)pStateVariableValue;
            pStringParam->setValue(variableValue);
        }

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getFloatStateVariableValue
// Description: Reads a float state variable.
// Arguments:   aptrStateVariableValue - Will get the state variable value.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        18/7/2004
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getFloatStateVariableValue(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Get the state variable type and OpenGL enum:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    apOpenGLStateVariablesManager::apStateVariableType variableType = theStateVarsManager.stateVariableType(_stateVariableId);
    GLenum variableOpenGLEnum = theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId);

    // Get the state variable value:
    GLfloat buff[MAX_AMOUNT_OF_STATE_VARIABLE_ELEMENTS];
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetFloatv);
    gs_stat_realFunctionPointers.glGetFloatv(variableOpenGLEnum, buff);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetFloatv);

    if (variableType == apOpenGLStateVariablesManager::SimpleStateVariable)
    {
        if ((pStateVariableValue == NULL) || (pStateVariableValue->type() != OS_TOBJ_ID_GL_FLOAT_PARAMETER))
        {
            apStateVariablesSnapShot::deleteParameter(pStateVariableValue);
            pStateVariableValue = new apGLfloatParameter(*buff);
        }
        else
        {
            // Cast the parameter to a string parameter, and set its value:
            apGLfloatParameter* pFloatParam = (apGLfloatParameter*)pStateVariableValue;
            pFloatParam->readValueFromPointer(buff);
        }

        retVal = true;
    }
    else if (variableType == apOpenGLStateVariablesManager::VectorStateVariable)
    {
        // Output the float vector as an apVectorParameter:
        int amountOfElements = theStateVarsManager.amountOfStateVariableElementsN(_stateVariableId);
        retVal = gsGetVectorAsParameter<GLfloat, apGLfloatParameter>(buff, amountOfElements, pStateVariableValue);
    }
    else if (variableType == apOpenGLStateVariablesManager::MatrixStateVariable)
    {
        // Create a matrix parameter that will wrap the state variable value:
        int amountOfElementsN = theStateVarsManager.amountOfStateVariableElementsN(_stateVariableId);
        int amountOfElementsM = theStateVarsManager.amountOfStateVariableElementsM(_stateVariableId);
        retVal = gsGetMatrixAsParameter<GLfloat, apGLfloatParameter>(buff, amountOfElementsN, amountOfElementsM, pStateVariableValue);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getDoubleStateVariableValue
// Description: Reads a double state variable.
// Arguments:   aptrStateVariableValue - Will get the state variable value.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        18/7/2004
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getDoubleStateVariableValue(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Get the state variable type and OpenGL enum:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    apOpenGLStateVariablesManager::apStateVariableType variableType = theStateVarsManager.stateVariableType(_stateVariableId);
    GLenum variableOpenGLEnum = theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId);

    // Get the state variable value:
    GLdouble buff[MAX_AMOUNT_OF_STATE_VARIABLE_ELEMENTS];
#ifndef _GR_IPHONE_BUILD
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetDoublev);
    gs_stat_realFunctionPointers.glGetDoublev(variableOpenGLEnum, buff);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetDoublev);
#else
    // EAGL does not support double data type:
    GT_ASSERT(false);
#endif

    if (variableType == apOpenGLStateVariablesManager::SimpleStateVariable)
    {
        if ((pStateVariableValue == NULL) || (pStateVariableValue->type() != OS_TOBJ_ID_GL_DOUBLE_PARAMETER))
        {
            apStateVariablesSnapShot::deleteParameter(pStateVariableValue);
            pStateVariableValue = new apGLdoubleParameter(*buff);
        }
        else
        {
            // Cast the parameter to a string parameter, and set its value:
            apGLdoubleParameter* pDoubleParam = (apGLdoubleParameter*)pStateVariableValue;
            pDoubleParam->readValueFromPointer(buff);
        }

        retVal = true;
    }
    else if (variableType == apOpenGLStateVariablesManager::VectorStateVariable)
    {
        // Output the double vector as an apVectorParameter:
        int amountOfElements = theStateVarsManager.amountOfStateVariableElementsN(_stateVariableId);
        retVal = gsGetVectorAsParameter<GLdouble, apGLdoubleParameter>(buff, amountOfElements, pStateVariableValue);
    }
    else if (variableType == apOpenGLStateVariablesManager::MatrixStateVariable)
    {
        // Create a matrix parameter that will wrap the state variable value:
        int amountOfElementsN = theStateVarsManager.amountOfStateVariableElementsN(_stateVariableId);
        int amountOfElementsM = theStateVarsManager.amountOfStateVariableElementsM(_stateVariableId);
        retVal = gsGetMatrixAsParameter<GLdouble, apGLdoubleParameter>(buff, amountOfElementsN, amountOfElementsM, pStateVariableValue);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getMaterialVariableValue
// Description: Reads a material state variable value.
// Arguments:   aptrStateVariableValue - Will get the state variable value.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/7/2004
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getMaterialVariableValue(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Get the state variable type, OpenGL enum, face and amount of elements:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    GLenum variableOpenGLEnum = theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId);
    GLenum face = theStateVarsManager.stateVariableAdditionalOpenGLParamEnum(_stateVariableId);
    int amountOfElements = theStateVarsManager.amountOfStateVariableElementsN(_stateVariableId);

    // Get the state variable value:
    GLfloat buff[MAX_AMOUNT_OF_STATE_VARIABLE_ELEMENTS];
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetMaterialfv);
    gs_stat_realFunctionPointers.glGetMaterialfv(face, variableOpenGLEnum, buff);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetMaterialfv);

    // Wrap the variable in a parameter object:
    retVal = gsGetVectorAsParameter<GLfloat, apGLfloatParameter>(buff, amountOfElements, pStateVariableValue);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getLightVariableValue
// Description: Reads a light state variable value.
// Arguments:   aptrStateVariableValue - Will get the state variable value.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/7/2004
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getLightVariableValue(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Get the state variable type, OpenGL enum, light param and amount of elements:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    apOpenGLStateVariablesManager::apStateVariableType variableType = theStateVarsManager.stateVariableType(_stateVariableId);
    GLenum variableOpenGLEnum = theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId);
    GLenum lightParam = theStateVarsManager.stateVariableAdditionalOpenGLParamEnum(_stateVariableId);

    // Get the state variable value:
    GLfloat buff[MAX_AMOUNT_OF_STATE_VARIABLE_ELEMENTS];
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetLightfv);
    gs_stat_realFunctionPointers.glGetLightfv(variableOpenGLEnum, lightParam, buff);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetLightfv);

    if (variableType == apOpenGLStateVariablesManager::SimpleStateVariable)
    {
        // If the parameter was not allocated yet, allocate it:
        if ((pStateVariableValue == NULL) || (pStateVariableValue->type() != OS_TOBJ_ID_GL_FLOAT_PARAMETER))
        {
            apStateVariablesSnapShot::deleteParameter(pStateVariableValue);
            pStateVariableValue = new apGLfloatParameter(*buff);
        }
        else
        {
            // Cast the pointer to the right parameter type, and set it's value:
            apGLfloatParameter* pFloatParamter = (apGLfloatParameter*)pStateVariableValue;
            pFloatParamter->readValueFromPointer(buff);
        }

        retVal = true;
    }
    else if (variableType == apOpenGLStateVariablesManager::VectorStateVariable)
    {
        // Output the float vector as an apVectorParameter:
        int amountOfElements = theStateVarsManager.amountOfStateVariableElementsN(_stateVariableId);
        retVal = gsGetVectorAsParameter<GLfloat, apGLfloatParameter>(buff, amountOfElements, pStateVariableValue);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getClipPlaneVariableValue
// Description: Reads a clip plane state variable value.
// Arguments:   aptrStateVariableValue - Will get the state variable value.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/7/2004
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getClipPlaneVariableValue(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Get the state variable type, OpenGL enum and amount of elements:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    GLenum variableOpenGLEnum = theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId);
    int amountOfElements = theStateVarsManager.amountOfStateVariableElementsN(_stateVariableId);

#ifdef _GR_IPHONE_BUILD
    // Get the state variable value:
    GLfloat buff[MAX_AMOUNT_OF_STATE_VARIABLE_ELEMENTS];
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetClipPlanef);
    gs_stat_realFunctionPointers.glGetClipPlanef(variableOpenGLEnum, buff);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetClipPlanef);

    // Wrap the variable in a parameter object:
    retVal = gsGetVectorAsParameter<GLfloat, apGLfloatParameter>(buff, amountOfElements, pStateVariableValue);
#else
    // Get the state variable value:
    GLdouble buff[MAX_AMOUNT_OF_STATE_VARIABLE_ELEMENTS];
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetClipPlane);
    gs_stat_realFunctionPointers.glGetClipPlane(variableOpenGLEnum, buff);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetClipPlane);

    // Wrap the variable in a parameter object:
    retVal = gsGetVectorAsParameter<GLdouble, apGLdoubleParameter>(buff, amountOfElements, pStateVariableValue);
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getPointerVariableValue
// Description: Reads a pointer state variable value.
// Arguments:   aptrStateVariableValue - Will get the state variable value.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        21/9/2004
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getPointerVariableValue(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Get the state variable type and OpenGL enum:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    GLenum variableOpenGLEnum = theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId);

    // Get the state variable value:
    GLvoid* stateVarValue = NULL;
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetPointerv);
    gs_stat_realFunctionPointers.glGetPointerv(variableOpenGLEnum, &stateVarValue);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetPointerv);

    // If the parameter was not allocated yet, allocate it:
    if ((pStateVariableValue == NULL) || (pStateVariableValue->type() != OS_TOBJ_ID_GL_P_VOID_PARAMETER))
    {
        apStateVariablesSnapShot::deleteParameter(pStateVariableValue);
        pStateVariableValue = new apPointerParameter(stateVarValue, OS_TOBJ_ID_GL_P_VOID_PARAMETER);
    }
    else
    {
        // Cast the pointer to the right parameter type, and set it's value:
        apPointerParameter* pPointerParamter = (apPointerParameter*)pStateVariableValue;
        pPointerParamter->setValue((osProcedureAddress64)stateVarValue, OS_TOBJ_ID_GL_P_VOID_PARAMETER);
    }

    retVal = true;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getBufferParameterARBIntValue
// Description: Reads an int buffer object parameter value.
//              (A state variable value that its get function is glGetBufferParameterivARB)
// Arguments:   aptrStateVariableValue - Will get the parameter value.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/12/2004
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getBufferParameterARBIntValue(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Get the state variable type, query target and param name:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    GLenum queryTarget = theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId);
    GLenum paramName = theStateVarsManager.stateVariableAdditionalOpenGLParamEnum(_stateVariableId);

    // Check if the state variable is accessible
    // (According to OpenGL documentation, if there is no bound array buffer,
    //  calling glGetBufferParameterivARB results in a GL_INVALID_OPERATION error)
    GLboolean isStateVariableAccessible = GL_FALSE;

    if (queryTarget == GL_ARRAY_BUFFER_ARB)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetBooleanv);
        gs_stat_realFunctionPointers.glGetBooleanv(GL_ARRAY_BUFFER_BINDING_ARB, &isStateVariableAccessible);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetBooleanv);
    }
    else if (queryTarget == GL_ELEMENT_ARRAY_BUFFER_ARB)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetBooleanv);
        gs_stat_realFunctionPointers.glGetBooleanv(GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB, &isStateVariableAccessible);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetBooleanv);
    }

    if (isStateVariableAccessible == GL_TRUE)
    {
        // Get the extension functions real implementation pointers:
        gsExtensionsManager& theExtensionsManager = gsExtensionsManager::instance();
        gsMonitoredFunctionPointers* pRealExtFuncsPtrs = theExtensionsManager.currentRenderContextExtensionsRealImplPointers();

        if (pRealExtFuncsPtrs)
        {
            // If glGetBufferParameterivARB pointer was not asked for the current context - we will ask for it:
            if ((*pRealExtFuncsPtrs).glGetBufferParameterivARB == NULL)
            {
                (*pRealExtFuncsPtrs).glGetBufferParameterivARB = (PFNGLGETBUFFERPARAMETERIVARBPROC)(gsGetSystemsOGLModuleProcAddress("glGetBufferParameterivARB"));
            }

            // If glGetBufferParameterivARB is supported for the current render context:
            if ((*pRealExtFuncsPtrs).glGetBufferParameterivARB != NULL)
            {
                // Get the state variable value:
                GLint stateVarValue = 0;
#ifndef _GR_IPHONE_BUILD
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetBufferParameterivARB);
                pRealExtFuncsPtrs->glGetBufferParameterivARB(queryTarget, paramName, &stateVarValue);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetBufferParameterivARB);
#else
                // GetBufferParamteriv is not an extension functin in EAGL, so we should never get here (as the state variables
                // are not ARB state variables either)
                GT_ASSERT(false);
#endif

                // Wrap the variable in a parameter object:
                if (pStateVariableValue == NULL)
                {
                    if ((paramName == GL_BUFFER_USAGE_ARB) || (paramName == GL_BUFFER_ACCESS_ARB))
                    {
                        pStateVariableValue = new apGLenumParameter(stateVarValue);
                    }
                    else
                    {
                        pStateVariableValue = new apGLintParameter(stateVarValue);
                    }

                    retVal = true;
                }
                else
                {
                    pStateVariableValue->readValueFromPointer(&stateVarValue);
                    retVal = true;
                }
            }
            else
            {
                // glGetBufferParameterivARB is NOT supported for the current render context
                isStateVariableAccessible = GL_FALSE;
            }
        }
    }

    // If the state variable is not accessible:
    if (isStateVariableAccessible == GL_FALSE)
    {
        retVal = false;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getBufferParameterIntValue
// Description: Reads an int buffer object parameter value.
//              (A state variable value that its get function is glGetBufferParameteriv)
// Arguments:   aptrStateVariableValue - Will get the parameter value.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/12/2004
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getBufferParameterIntValue(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Get the state variable type, query target and param name:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    GLenum queryTarget = theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId);
    GLenum paramName = theStateVarsManager.stateVariableAdditionalOpenGLParamEnum(_stateVariableId);

    // Check if the state variable is accessible
    // (According to OpenGL documentation, if there is no bound array buffer,
    //  calling glGetBufferParameteriv results in a GL_INVALID_OPERATION error)
    GLboolean isStateVariableAccessible = GL_FALSE;

    if (queryTarget == GL_ARRAY_BUFFER)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetBooleanv);
        gs_stat_realFunctionPointers.glGetBooleanv(GL_ARRAY_BUFFER_BINDING, &isStateVariableAccessible);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetBooleanv);
    }
    else if (queryTarget == GL_ELEMENT_ARRAY_BUFFER)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetBooleanv);
        gs_stat_realFunctionPointers.glGetBooleanv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &isStateVariableAccessible);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetBooleanv);
    }

    if (isStateVariableAccessible == GL_TRUE)
    {
        // Get the extension functions real implementation pointers:
        gsExtensionsManager& theExtensionsManager = gsExtensionsManager::instance();
        gsMonitoredFunctionPointers* pRealExtFuncsPtrs = theExtensionsManager.currentRenderContextExtensionsRealImplPointers();

        if (pRealExtFuncsPtrs)
        {
            // If glGetBufferParameteriv pointer was not asked for the current context - we will ask for it:
            if ((*pRealExtFuncsPtrs).glGetBufferParameteriv == NULL)
            {
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                (*pRealExtFuncsPtrs).glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC)(gsGetSystemsOGLModuleProcAddress("glGetBufferParameteriv"));
#else
                // In MAC this function is not an extension function
#endif
            }

            // If glGetBufferParameteriv is supported for the current render context:
            if ((*pRealExtFuncsPtrs).glGetBufferParameteriv != NULL)
            {
                // Get the state variable value:
                GLint stateVarValue = 0;
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetBufferParameteriv);
                pRealExtFuncsPtrs->glGetBufferParameteriv(queryTarget, paramName, &stateVarValue);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetBufferParameteriv);
#else
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetBufferParameteriv);
                gs_stat_realFunctionPointers.glGetBufferParameteriv(queryTarget, paramName, &stateVarValue);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetBufferParameteriv);
#endif

                // Wrap the variable in a parameter object:
                if (pStateVariableValue == NULL)
                {
                    if ((paramName == GL_BUFFER_USAGE) || (paramName == GL_BUFFER_ACCESS))
                    {
                        pStateVariableValue = new apGLenumParameter(stateVarValue);
                    }
                    else
                    {
                        pStateVariableValue = new apGLintParameter(stateVarValue);
                    }

                    retVal = true;
                }
                else
                {
                    pStateVariableValue->readValueFromPointer(&stateVarValue);
                    retVal = true;
                }
            }
            else
            {
                // glGetBufferParameteriv is NOT supported for the current render context
                isStateVariableAccessible = GL_FALSE;
            }
        }
    }

    // If the state variable is not accessible:
    if (isStateVariableAccessible == GL_FALSE)
    {
        // Non available parameter:
        retVal = false;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getProgramIntVariableValue
// Description: Reads an int program variable value.
//              (A state variable value that its get function is glGetProgramivARB)
// Arguments:   aptrStateVariableValue - Will get the parameter value.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        30/3/2005
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getProgramIntVariableValue(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Get the state variable type, query target and param name:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    GLenum queryTarget = theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId);
    GLenum paramName = theStateVarsManager.stateVariableAdditionalOpenGLParamEnum(_stateVariableId);

#ifdef _GR_IPHONE_BUILD
    // GL_ARB_[vertex|fragment]_program (and binary shaders in general)are not supported on the iPhone, and we should not get here:
    GT_ASSERT(false);
#else
    // Get the extension functions real implementation pointers:
    gsExtensionsManager& theExtensionsManager = gsExtensionsManager::instance();
    gsMonitoredFunctionPointers* pRealExtFuncsPtrs = theExtensionsManager.currentRenderContextExtensionsRealImplPointers();

    if (pRealExtFuncsPtrs)
    {
        // If glGetProgramivARB pointer was not asked for the current context - we will ask for it:
        if ((*pRealExtFuncsPtrs).glGetProgramivARB == NULL)
        {
            (*pRealExtFuncsPtrs).glGetProgramivARB = (PFNGLGETPROGRAMIVARBPROC)(gsGetSystemsOGLModuleProcAddress("glGetProgramivARB"));
        }

        // If glGetProgramivARB is supported for the current render context:
        if ((*pRealExtFuncsPtrs).glGetProgramivARB != NULL)
        {
            // Get the state variable value:
            GLint stateVarValue = 0;
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetProgramivARB);
            pRealExtFuncsPtrs->glGetProgramivARB(queryTarget, paramName, &stateVarValue);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetProgramivARB);

            // Wrap the variable in a parameter object:
            if (pStateVariableValue == NULL)
            {
                if (paramName == GL_PROGRAM_FORMAT_ARB)
                {
                    pStateVariableValue = new apGLenumParameter(stateVarValue);
                }
                else
                {
                    pStateVariableValue = new apGLintParameter(stateVarValue);
                }

                retVal = true;
            }
            else
            {
                pStateVariableValue->readValueFromPointer(&stateVarValue);
                retVal = true;
            }

        }
        else
        {
            // Non available parameter:
            retVal = false;
        }
    }

#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getHandleVariableValue
// Description: Reads GLhandle variable value.
//              (A state variable value that its get function is glGetHandleARB)
// Arguments:   aptrStateVariableValue - Will get the parameter value.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        31/3/2005
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getHandleVariableValue(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Get the state variable type and param name:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    GLenum paramName = theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId);

#ifdef _GR_IPHONE_BUILD
    // GL_ARB_shader_objects is not supported on the iPhone, so we should not get here:
    GT_ASSERT(false);
#else
    // Get the extension functions real implementation pointers:
    gsExtensionsManager& theExtensionsManager = gsExtensionsManager::instance();
    gsMonitoredFunctionPointers* pRealExtFuncsPtrs = theExtensionsManager.currentRenderContextExtensionsRealImplPointers();

    if (pRealExtFuncsPtrs)
    {
        // If glGetHandleARB pointer was not asked for the current context - we will ask for it:
        if ((*pRealExtFuncsPtrs).glGetHandleARB == NULL)
        {
            (*pRealExtFuncsPtrs).glGetHandleARB = (PFNGLGETHANDLEARBPROC)(gsGetSystemsOGLModuleProcAddress("glGetHandleARB"));
        }

        // If glGetHandleARB is supported for the current render context:
        if ((*pRealExtFuncsPtrs).glGetHandleARB != NULL)
        {
            // Get the state variable value:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetHandleARB);
            GLhandleARB stateVarValue = pRealExtFuncsPtrs->glGetHandleARB(paramName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetHandleARB);

            // Wrap the variable in a parameter object:
            if ((pStateVariableValue == NULL) || (pStateVariableValue->type() != OS_TOBJ_ID_GL_UINT_PARAMETER))
            {
                apStateVariablesSnapShot::deleteParameter(pStateVariableValue);
                pStateVariableValue = new apGLuintParameter(stateVarValue);
            }
            else
            {
                apGLuintParameter* pUintParam = (apGLuintParameter*)pStateVariableValue;
                pUintParam->readValueFromPointer(&stateVarValue);
            }

            retVal = true;
        }
        else
        {
            // Non available parameter:
            retVal = false;
        }
    }

#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::beforeGettingStateVariableValue
// Description: Called before attempting to get a state variable's value
// Author:      Uri Shomroni
// Date:        23/2/2015
// ---------------------------------------------------------------------------
void gsStateVariableReader::beforeGettingStateVariableValue()
{
    // Print an EXTENSIVE-level line to the log:
    if (osDebugLog::instance().loggedSeverity() >= OS_DEBUG_LOG_EXTENSIVE)
    {
        // Base string:
        gtString logMsg = GS_STR_StateVariableReaderMessageBefore;
        apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();

        logMsg.append(theStateVarsManager.stateVariableName(_stateVariableId));
        logMsg.appendFormattedString(GS_STR_StateVariableReaderMessageBeforeDetails, theStateVarsManager.stateVariableGetFunctionId(_stateVariableId), theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId));

        GLenum additionalEnum = theStateVarsManager.stateVariableAdditionalOpenGLParamEnum(_stateVariableId);

        if (GL_NONE != additionalEnum)
        {
            logMsg.appendFormattedString(GS_STR_StateVariableReaderMessageBeforeAdditional, additionalEnum);
        }

        int nDim = theStateVarsManager.amountOfStateVariableElementsN(_stateVariableId);
        int mDim = theStateVarsManager.amountOfStateVariableElementsM(_stateVariableId);

        if ((1 != nDim) || (1 < mDim))
        {
            logMsg.appendFormattedString(GS_STR_StateVariableReaderMessageBeforeDims, nDim, mDim);
        }

        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
    }
}

// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::afterGettingStateVariableValue
// Description: Called after attempting to get a state variable's value
// Author:      Uri Shomroni
// Date:        23/2/2015
// ---------------------------------------------------------------------------
void gsStateVariableReader::afterGettingStateVariableValue(bool retVal, GLenum openGLError)
{
    // Print an EXTENSIVE-level line to the log:
    if (osDebugLog::instance().loggedSeverity() >= OS_DEBUG_LOG_EXTENSIVE)
    {
        // Base message:
        gtString logMsg = retVal ? GS_STR_StateVariableReaderMessageAfterTrue : GS_STR_StateVariableReaderMessageAfterFalse;

        if (GL_NONE != openGLError)
        {
            gtString openGLErrorStr;
            apGLenumParameter errEnum(openGLError);
            errEnum.valueAsString(openGLErrorStr);

            logMsg.append(GS_STR_StateVariableReaderMessageAfterGLError).append(openGLErrorStr).append('.');
        }
        else if (retVal)
        {
            logMsg.append(GS_STR_StateVariableReaderMessageAfterSuccess);
        }

        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
    }
}

// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getTexEnvVariableValue
// Description: Reads a texture environment state variable value.
// Arguments:   aptrStateVariableValue - Will get the state variable value.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        20/3/2005
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getTexEnvVariableValue(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Get the state variable type, OpenGL enum and amount of elements:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    GLenum variableOpenGLEnum = theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId);

    // Get the state variable value:
    GLfloat buff[MAX_AMOUNT_OF_STATE_VARIABLE_ELEMENTS];
    GLenum target = GL_TEXTURE_ENV;

    if (variableOpenGLEnum == GL_TEXTURE_LOD_BIAS)
    {
        target = GL_TEXTURE_FILTER_CONTROL;
    }
    else if ((variableOpenGLEnum == GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV) || (variableOpenGLEnum == GL_SHADER_OPERATION_NV) ||
             (variableOpenGLEnum == GL_OFFSET_TEXTURE_SCALE_NV) || (variableOpenGLEnum == GL_OFFSET_TEXTURE_BIAS_NV) ||
             (variableOpenGLEnum == GL_PREVIOUS_TEXTURE_INPUT_NV) || (variableOpenGLEnum == GL_CULL_MODES_NV) ||
             (variableOpenGLEnum == GL_OFFSET_TEXTURE_MATRIX_NV) || (variableOpenGLEnum == GL_CONST_EYE_NV))
        // GL_OFFSET_TEXTURE_2D_SCALE_NV is the same as GL_OFFSET_TEXTURE_SCALE_NV
        // GL_OFFSET_TEXTURE_2D_BIAS_NV is the same as GL_OFFSET_TEXTURE_BIAS_NV
        // GL_OFFSET_TEXTURE_2D_MATRIX_NV is the same as OFFSET_TEXTURE_MATRIX_NV
    {
        target = GL_TEXTURE_SHADER_NV;
    }
    else if (variableOpenGLEnum == GL_COORD_REPLACE)
        // Note that GL_COORD_REPLACE[|_ARB|OES] are the same and so are GL_POINT_SPRITE[|_ARB|OES]
    {
        target = GL_POINT_SPRITE;
    }

    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetTexEnvfv);
    gs_stat_realFunctionPointers.glGetTexEnvfv(target, variableOpenGLEnum, buff);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetTexEnvfv);

    switch (variableOpenGLEnum)
    {
        case GL_TEXTURE_ENV_MODE:
        case GL_SHADER_OPERATION_NV:
        case GL_TEXTURE_SHADER_NV:
        case GL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV:
        case GL_SRC0_RGB:
        case GL_SRC1_RGB:
        case GL_SRC2_RGB:
        case GL_SRC0_ALPHA:
        case GL_SRC1_ALPHA:
        case GL_SRC2_ALPHA:
        case GL_COORD_REPLACE_ARB:
        {
            GLint envModeAsInt = (int)(buff[0]);

            if ((pStateVariableValue == NULL) || (pStateVariableValue->type() != OS_TOBJ_ID_GL_ENUM_PARAMETER))
            {
                apStateVariablesSnapShot::deleteParameter(pStateVariableValue);
                pStateVariableValue = new apGLenumParameter(envModeAsInt);
            }
            else
            {
                apGLenumParameter* pEnumParam = (apGLenumParameter*)pStateVariableValue;
                pEnumParam->setValueFromInt(envModeAsInt);
            }

            retVal = true;
            break;
        }

        case GL_TEXTURE_ENV_COLOR:
        {
            retVal = gsGetVectorAsParameter<GLfloat, apGLfloatParameter>(buff, 4, pStateVariableValue);
            break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getTexGenVariableValue
// Description: Reads a texture coordinate generation state variable value.
// Arguments:   aptrStateVariableValue - Will get the state variable value.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        20/7/2015
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getTexGenVariableValue(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Get the state variable type, OpenGL enum and amount of elements:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    GLenum variableOpenGLEnum = theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId);
    GLenum variableAdditionalOpenGLEnum = theStateVarsManager.stateVariableAdditionalOpenGLParamEnum(_stateVariableId);

    // Get the state variable value:
    GLfloat buff[MAX_AMOUNT_OF_STATE_VARIABLE_ELEMENTS];

    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetTexGenfv);
    gs_stat_realFunctionPointers.glGetTexGenfv(variableAdditionalOpenGLEnum, variableOpenGLEnum, buff);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetTexGenfv);

    switch (variableOpenGLEnum)
    {
        case GL_TEXTURE_GEN_MODE:
        {
            GLenum varValueAsEnum = (GLenum)(buff[0]);

            if ((nullptr == pStateVariableValue) || (OS_TOBJ_ID_GL_ENUM_PARAMETER != pStateVariableValue->type()))
            {
                apStateVariablesSnapShot::deleteParameter(pStateVariableValue);
                pStateVariableValue = new apGLenumParameter(varValueAsEnum);
            }
            else
            {
                apGLenumParameter* pEnumParam = (apGLenumParameter*)pStateVariableValue;
                pEnumParam->setValueFromInt(varValueAsEnum);
            }

            retVal = true;
        }
        break;

        case GL_OBJECT_PLANE:
        case GL_EYE_PLANE:
            retVal = gsGetVectorAsParameter<GLfloat, apGLfloatParameter>(buff, 4, pStateVariableValue);
            break;

        default:
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getQueryIntVariableValue
// Description: Reads an int query state variable value.
//              (A state variable value that its get function is glGetQueryiv).
// Arguments:   aptrStateVariableValue - Will get the state variable value.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        28/9/2004
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getQueryIntVariableValue(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Get the state variable type, query target and param name:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    GLenum queryTarget = theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId);
    GLenum paramName = theStateVarsManager.stateVariableAdditionalOpenGLParamEnum(_stateVariableId);

    // Get the extension functions real implementation pointers:
    gsExtensionsManager& theExtensionsManager = gsExtensionsManager::instance();
    gsMonitoredFunctionPointers* pRealExtFuncsPtrs = theExtensionsManager.currentRenderContextExtensionsRealImplPointers();

    if (pRealExtFuncsPtrs)
    {
        // If glGetQueryiv pointer was not asked for the current context - we will ask for it:
        if ((*pRealExtFuncsPtrs).glGetQueryiv == NULL)
        {
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            (*pRealExtFuncsPtrs).glGetQueryiv = (PFNGLGETQUERYIVPROC)(gsGetSystemsOGLModuleProcAddress("glGetQueryivARB"));
#else
            // In MAC this function is not an extension function
#endif
        }

        // If glGetQueryiv is supported for the current render context:
        if ((*pRealExtFuncsPtrs).glGetQueryiv != NULL)
        {
            // Get the state variable value:
            GLint stateVarValue = 0;
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetQueryiv);
            pRealExtFuncsPtrs->glGetQueryiv(queryTarget, paramName, &stateVarValue);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetQueryiv);
#else
#ifndef _GR_IPHONE_BUILD
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetQueryiv);
            gs_stat_realFunctionPointers.glGetQueryiv(queryTarget, paramName, &stateVarValue);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetQueryiv);
#else
            // Query objects are not supported in OpenGL ES (and specificly in EAGL):
            GT_ASSERT(false);
#endif
#endif

            if ((pStateVariableValue == NULL) || (pStateVariableValue->type() != OS_TOBJ_ID_GL_INT_PARAMETER))
            {
                apStateVariablesSnapShot::deleteParameter(pStateVariableValue);
                pStateVariableValue = new apGLintParameter(stateVarValue);
            }
            else
            {
                pStateVariableValue->readValueFromPointer(&stateVarValue);
            }

            retVal = true;
        }
        else
        {
            // glGetQueryiv is NOT supported for the current render context -
            // we will return a N/A value:
            // Non available parameter:
            retVal = false;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getIntegerStateVariableValue
// Description: Reads an integer state variable.
// Arguments:   aptrStateVariableValue - Will get the state variable value.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        18/7/2004
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getIntegerStateVariableValue(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Get the state variable type and OpenGL enum:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    apOpenGLStateVariablesManager::apStateVariableType variableType = theStateVarsManager.stateVariableType(_stateVariableId);
    GLenum variableOpenGLEnum = theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId);

    // Get the state variable value:
    GLint buff[MAX_AMOUNT_OF_STATE_VARIABLE_ELEMENTS];
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
    gs_stat_realFunctionPointers.glGetIntegerv(variableOpenGLEnum, buff);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);

    if (variableType == apOpenGLStateVariablesManager::SimpleStateVariable)
    {
        // Special case - GL_CURRENT_PROGRAM is an unsigned int, but OpenGL does not have
        // an unsigned int glGet function. This force us (and all OpenGL users) to do ugly
        // C castings to get the unsigned int from the glGetIntegerv function.
        // (See Case 1739 for more details).
        if (variableOpenGLEnum == GL_CURRENT_PROGRAM)
        {
            if ((pStateVariableValue == NULL) || (pStateVariableValue->type() != OS_TOBJ_ID_GL_UINT_PARAMETER))
            {
                apStateVariablesSnapShot::deleteParameter(pStateVariableValue);
                pStateVariableValue = new apGLuintParameter(*((GLuint*)buff));
            }
            else
            {
                apGLuintParameter* pUintParam = (apGLuintParameter*)pStateVariableValue;
                pUintParam->readValueFromPointer((GLuint*)buff);
            }
        }
        else
        {
            if ((pStateVariableValue == NULL) || (pStateVariableValue->type() != OS_TOBJ_ID_GL_INT_PARAMETER))
            {
                apStateVariablesSnapShot::deleteParameter(pStateVariableValue);
                pStateVariableValue = new apGLintParameter(*buff);
            }
            else
            {
                apGLintParameter* pIntParam = (apGLintParameter*)pStateVariableValue;
                pIntParam->readValueFromPointer((GLuint*)buff);
            }
        }

        retVal = true;
    }
    else if (variableType == apOpenGLStateVariablesManager::VectorStateVariable)
    {
        // Output the int vector as an apVectorParameter:
        int amountOfElements = theStateVarsManager.amountOfStateVariableElementsN(_stateVariableId);
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT) && (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE))
        // Uri, 17/9/09: Mac 64-bit has a problem where glGetIntergerv takes a GLint (= long = 8 bytes) pointer as a parameter, but the output is
        // as int (= 4 bytes). This causes "x" to contain x + (y << 32), "y" to contain width + (height << 32), and "width" and "height" to contain
        // garbage data. So - we cast the pointer to an int* to overcome this:
        retVal = gsGetVectorAsParameter<int, apGLintParameter>((int*)buff, amountOfElements, pStateVariableValue);
#else
        retVal = gsGetVectorAsParameter<GLint, apGLintParameter>(buff, amountOfElements, pStateVariableValue);
#endif
    }
    else if (variableType == apOpenGLStateVariablesManager::MatrixStateVariable)
    {
        // Create a matrix parameter that will wrap the state variable value:
        int amountOfElementsN = theStateVarsManager.amountOfStateVariableElementsN(_stateVariableId);
        int amountOfElementsM = theStateVarsManager.amountOfStateVariableElementsM(_stateVariableId);
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT) && (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE))
        // Uri, 17/9/09: Mac 64-bit has a problem where glGetIntergerv takes a GLint (= long = 8 bytes) pointer as a parameter, but the output is
        // as int (= 4 bytes). This causes "x" to contain x + (y << 32), "y" to contain width + (height << 32), and "width" and "height" to contain
        // garbage data. So - we cast the pointer to an int* to overcome this:
        retVal = gsGetMatrixAsParameter<int, apGLintParameter>((int*)buff, amountOfElementsN, amountOfElementsM, pStateVariableValue);
#else
        retVal = gsGetMatrixAsParameter<GLint, apGLintParameter>(buff, amountOfElementsN, amountOfElementsM, pStateVariableValue);
#endif
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getInteger64StateVariableValue
// Description: Reads an integer state variable.
// Arguments:   aptrStateVariableValue - Will get the state variable value.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/10/2009
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getInteger64StateVariableValue(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Get the state variable type and OpenGL enum:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    apOpenGLStateVariablesManager::apStateVariableType variableType = theStateVarsManager.stateVariableType(_stateVariableId);
    GLenum variableOpenGLEnum = theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId);

    // Get the glGetInteger64v function pointer from the render context monitor:
    PFNGLGETINTEGER64VPROC glGetInteger64v = NULL;

    if (_pMyRenderContextMtr != NULL)
    {
        glGetInteger64v = _pMyRenderContextMtr->myGLGetInteger64v();
    }

    if (glGetInteger64v != NULL)
    {
        // Get the state variable value:
        GLint64 buff[MAX_AMOUNT_OF_STATE_VARIABLE_ELEMENTS];
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetInteger64v);
        glGetInteger64v(variableOpenGLEnum, buff);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetInteger64v);

        if (variableType == apOpenGLStateVariablesManager::SimpleStateVariable)
        {
            if ((pStateVariableValue == NULL) || (pStateVariableValue->type() != OS_TOBJ_ID_GL_INT_64_PARAMETER))
            {
                apStateVariablesSnapShot::deleteParameter(pStateVariableValue);
                pStateVariableValue = new apGLint64Parameter(*buff);
            }
            else
            {
                apGLint64Parameter* pIntParam = (apGLint64Parameter*)pStateVariableValue;
                pIntParam->readValueFromPointer((GLuint64*)buff);
            }

            retVal = true;
        }
        else if (variableType == apOpenGLStateVariablesManager::VectorStateVariable)
        {
            // Output the int vector as an apVectorParameter:
            int amountOfElements = theStateVarsManager.amountOfStateVariableElementsN(_stateVariableId);
            retVal = gsGetVectorAsParameter<GLint64, apGLint64Parameter>(buff, amountOfElements, pStateVariableValue);
        }
        else if (variableType == apOpenGLStateVariablesManager::MatrixStateVariable)
        {
            // Create a matrix parameter that will wrap the state variable value:
            int amountOfElementsN = theStateVarsManager.amountOfStateVariableElementsN(_stateVariableId);
            int amountOfElementsM = theStateVarsManager.amountOfStateVariableElementsM(_stateVariableId);
            retVal = gsGetMatrixAsParameter<GLint64, apGLint64Parameter>(buff, amountOfElementsN, amountOfElementsM, pStateVariableValue);
        }
    }
    else // _glGetInteger64v == NULL
    {
        gtString debugMessage;
        debugMessage.append(GS_STR_UnsupportedStateVarGetFunction);
        debugMessage.append(theStateVarsManager.stateVariableName(_stateVariableId));
        debugMessage.append(L" (glGetInteger64v)");
        OS_OUTPUT_DEBUG_LOG(debugMessage.asCharArray(), OS_DEBUG_LOG_INFO);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getIntegerui64NVStateVariableValue
// Description: Reads a 64 bit integer state variable.
// Arguments:   aptrStateVariableValue - Will get the state variable value.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        18/4/2010
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getIntegerui64NVStateVariableValue(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Get the state variable type and OpenGL enum:
    apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();
    apOpenGLStateVariablesManager::apStateVariableType variableType = theStateVarsManager.stateVariableType(_stateVariableId);
    GLenum variableOpenGLEnum = theStateVarsManager.stateVariableOpenGLParamEnum(_stateVariableId);

    // Get the glGetInteger64v function pointer from the render context monitor:
    PFNGLGETINTEGERUI64VNVPROC glGetIntegerui64vNV = NULL;

    if (_pMyRenderContextMtr != NULL)
    {
        glGetIntegerui64vNV = _pMyRenderContextMtr->myGLGetIntegerui64vNV();
    }

    if (glGetIntegerui64vNV != NULL)
    {
        // Get the state variable value:
        GLuint64 result;
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerui64vNV);
        glGetIntegerui64vNV(variableOpenGLEnum, &result);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerui64vNV);

        if (variableType == apOpenGLStateVariablesManager::SimpleStateVariable)
        {
            if ((pStateVariableValue == NULL) || (pStateVariableValue->type() != OS_TOBJ_ID_GL_INT_64_PARAMETER))
            {
                apStateVariablesSnapShot::deleteParameter(pStateVariableValue);
                pStateVariableValue = new apGLuint64Parameter(result);
            }
            else
            {
                apGLuint64Parameter* pIntParam = (apGLuint64Parameter*)pStateVariableValue;
                pIntParam->readValueFromPointer((GLuint64*)&result);
            }

            retVal = true;
        }
        else if (variableType == apOpenGLStateVariablesManager::VectorStateVariable)
        {
            // Output the int vector as an apVectorParameter:
            int amountOfElements = theStateVarsManager.amountOfStateVariableElementsN(_stateVariableId);
            retVal = gsGetVectorAsParameter<GLuint64, apGLuint64Parameter>(&result, amountOfElements, pStateVariableValue);
        }
        else if (variableType == apOpenGLStateVariablesManager::MatrixStateVariable)
        {
            // Create a matrix parameter that will wrap the state variable value:
            int amountOfElementsN = theStateVarsManager.amountOfStateVariableElementsN(_stateVariableId);
            int amountOfElementsM = theStateVarsManager.amountOfStateVariableElementsM(_stateVariableId);
            retVal = gsGetMatrixAsParameter<GLuint64, apGLuint64Parameter>(&result, amountOfElementsN, amountOfElementsM, pStateVariableValue);
        }
    }
    else // _glGetIntegerui64vNV == NULL
    {
        gtString debugMessage;
        debugMessage.append(GS_STR_UnsupportedStateVarGetFunction);
        debugMessage.append(theStateVarsManager.stateVariableName(_stateVariableId));
        debugMessage.append(L" (glGetIntegerui64vNV)");
        OS_OUTPUT_DEBUG_LOG(debugMessage.asCharArray(), OS_DEBUG_LOG_INFO);
    }

    return retVal;
}


// Mac only code:
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)

// CGL state variables are not available on the iPhone:
#ifndef _GR_IPHONE_BUILD
// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getCGLStateVariableVariableValue
// Description: Reads a CGL state variable value
// Arguments: apParameter*& pStateVariableValue - will get the state variable value
//            apMonitoredFunctionId getFunctionId - the CGL function id that handles the value
// Return Val: bool   - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/12/2008
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getCGLStateVariableVariableValue(apParameter*& pStateVariableValue, apMonitoredFunctionId getFunctionId)
{
    bool retVal = false;

    // Get the render context handle:
    GT_IF_WITH_ASSERT(_pMyRenderContextMtr != NULL)
    {
        // Get the OS render context handle:
        CGLContextObj pCGLContextObj = NULL;
        pCGLContextObj = _pMyRenderContextMtr->renderContextOSHandle();

        GT_IF_WITH_ASSERT(pCGLContextObj != NULL)
        {
            // Get the state variable manager:
            apOpenGLStateVariablesManager& theStateVarsManager = apOpenGLStateVariablesManager::instance();

            // Get the state variable CGL enumeration:
            unsigned int variableCGLValueAsInt = theStateVarsManager.stateVariableCGLParam(_stateVariableId);

            // Call the relevant CGL function to get the value:
            GLint paramValue;
            bool cglFunctionFound = false;
            CGLError cglError = kCGLNoError;

            if (getFunctionId == ap_CGLGetParameter)
            {
                // Convert the CGL enumeration to CGLContextParameter:
                CGLContextParameter pname = (CGLContextParameter)variableCGLValueAsInt;
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_CGLGetParameter);
                cglError = gs_stat_realFunctionPointers.CGLGetParameter(pCGLContextObj, pname, &paramValue);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_CGLGetParameter);
                cglFunctionFound = true;
            }
            else if (getFunctionId == ap_CGLIsEnabled)
            {
                // Convert the CGL enumeration to CGLContextParameter:
                CGLContextEnable pname = (CGLContextEnable)variableCGLValueAsInt;
                // In 64-bit Mac, CGLIsEnabled internally calls CGLGetParameter:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_CGLIsEnabled);
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_CGLGetParameter);
                cglError = gs_stat_realFunctionPointers.CGLIsEnabled(pCGLContextObj, pname, &paramValue);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_CGLGetParameter);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_CGLIsEnabled);
                cglFunctionFound = true;
            }
            else if (getFunctionId == ap_CGLGetOption)
            {
                // Convert the CGL enumeration to CGLContextParameter:
                CGLGlobalOption pname = (CGLGlobalOption)variableCGLValueAsInt;
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_CGLGetOption);
                cglError = gs_stat_realFunctionPointers.CGLGetOption(pname, &paramValue);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_CGLGetOption);
                cglFunctionFound = true;
            }
            else
            {
                cglFunctionFound = false;
                GT_ASSERT_EX(false, L"Unsupported CGL parameter type");
            }

            // Log error for function failure;
            if (cglError != kCGLNoError)
            {
                gtString errorMessage;
                errorMessage.appendFormattedString(L"CGL Parameter value read failure. Variable id (int): %d", variableCGLValueAsInt);
                errorMessage.append(CGLErrorString(cglError));
                OS_OUTPUT_DEBUG_LOG(errorMessage.asCharArray(), OS_DEBUG_LOG_ERROR);
            }

            // Function found and called, set the parameter's value:
            if ((cglError == kCGLNoError) && cglFunctionFound)
            {
                if ((pStateVariableValue == NULL) || (pStateVariableValue->type() != OS_TOBJ_ID_GL_INT_PARAMETER))
                {
                    apStateVariablesSnapShot::deleteParameter(pStateVariableValue);
                    pStateVariableValue = new apGLintParameter(paramValue);
                }
                else
                {
                    apGLintParameter* pCGLParam = (apGLintParameter*)pStateVariableValue;
                    pCGLParam->readValueFromPointer(&paramValue);
                }

                retVal = true;
            }
        }
    }

    return retVal;
}
#else // _GR_IPHONE_BUILD
// TO_DO iPhone - check if there are EAGL state vars
#endif // _GR_IPHONE_BUILD

#endif // Mac only code

// Windows only code:
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)


// ---------------------------------------------------------------------------
// Name:        gsStateVariableReader::getWGLSwapInterval
// Description:
//  Retrieves the current swap interval - WGL_SWAP_INTERVAL_EXT.
//  WGL_SWAP_INTERVAL_EXT is a "pseudo" state variable from the WGL_EXT_swap_control extension.
//
// Arguments: pStateVariableValue - Will get the swap interval value;
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        16/2/2009
// ---------------------------------------------------------------------------
bool gsStateVariableReader::getWGLSwapInterval(apParameter*& pStateVariableValue)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pMyRenderContextMtr != NULL)
    {
        // Get my render context spy id:
        int renderContextSpyId = _pMyRenderContextMtr->spyId();

        // If the WGL_EXT_swap_control extension is supported by this render context:
        gsExtensionsManager& theExtensionsMgr = gsExtensionsManager::instance();
        bool isWGL_EXT_swap_controlSupported = theExtensionsMgr.isExtensionSupported(renderContextSpyId, AP_WGL_EXT_swap_control);

        if (isWGL_EXT_swap_controlSupported)
        {
            // Get the extension functions implementations for the current context:
            gsMonitoredFunctionPointers* pExtensionsRealImplementationPointers = theExtensionsMgr.extensionsRealImplementationPointers(renderContextSpyId);
            GT_IF_WITH_ASSERT(pExtensionsRealImplementationPointers != NULL)
            {
                // Get a pointer to the wglGetSwapIntervalEXT function:
                PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)(pExtensionsRealImplementationPointers->wglGetSwapIntervalEXT);

                if (wglGetSwapIntervalEXT != NULL)
                {
                    // Get the current swap interval value:
                    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_wglGetSwapIntervalEXT);
                    int currentSwapInterval = wglGetSwapIntervalEXT();
                    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_wglGetSwapIntervalEXT);

                    if ((pStateVariableValue == NULL) || (pStateVariableValue->type() != OS_TOBJ_ID_INT_PARAMETER))
                    {
                        apStateVariablesSnapShot::deleteParameter(pStateVariableValue);
                        pStateVariableValue = new apIntParameter(currentSwapInterval);
                    }
                    else
                    {
                        apIntParameter* pIntParam = (apIntParameter*)pStateVariableValue;
                        pIntParam->readValueFromPointer(&currentSwapInterval);
                    }

                    retVal = true;
                }
            }
        }
    }

    return retVal;
}

#endif

