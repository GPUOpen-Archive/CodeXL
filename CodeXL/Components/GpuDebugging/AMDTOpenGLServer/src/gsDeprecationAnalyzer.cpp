//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsDeprecationAnalyzer.cpp
///
//==================================================================================

//------------------------------ gsDeprecationAnalyzer.cpp ------------------------------

// C:
#include <AMDTOSWrappers/Include/osStdLibIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>
#include <AMDTAPIClasses/Include/apOpenGLStateVariablesManager.h>

// Local:
#include <src/gsDeprecationAnalyzer.h>
#include <src/gsCallsHistoryLogger.h>
#include <src/gsStringConstants.h>

gsDeprecationAnalyzer* gsDeprecationAnalyzer::_pMySingleInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::gsDeprecationAnalyzer
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        3/3/2008
// ---------------------------------------------------------------------------
gsDeprecationAnalyzer::gsDeprecationAnalyzer()
{
    // Put NULLs in each of the monitored functions deprecations:
    for (int i = 0; i < apMonitoredFunctionsAmount; i++)
    {
        _monitoredFunctionsDeprecations[i] = NULL;
    }
}


// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::~gsDeprecationAnalyzer
// Description: Destructor.
// Author:      Sigal Algranaty
// Date:        3/3/2008
// ---------------------------------------------------------------------------
gsDeprecationAnalyzer::~gsDeprecationAnalyzer()
{
    // Clear the deprecations vector:
    for (int i = 0; i < apMonitoredFunctionsAmount; i++)
    {
        if (_monitoredFunctionsDeprecations[i] != NULL)
        {
            delete _monitoredFunctionsDeprecations[i];
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::instance
// Description: Returns the single instance of this class. Creates it on
//              the first call to this function.
// Author:      Sigal Algranaty
// Date:        3/3/2008
// ---------------------------------------------------------------------------
gsDeprecationAnalyzer& gsDeprecationAnalyzer::instance()
{
    // If my single instance was not created yet - create it:
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new gsDeprecationAnalyzer;
        GT_ASSERT(_pMySingleInstance);
    }

    return *_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::initialize
// Description: Initializes the static data that is used for deprecation test
// Author:      Sigal Algranaty
// Date:        3/3/2009
// ---------------------------------------------------------------------------
void gsDeprecationAnalyzer::initialize()
{
    // Initialize 3.0 deprecations:
    initializeOpenGL30Deprecations();

    // Initialize 3.2 deprecations:
    initializeOpenGL32Deprecations();

    // Initialize 4.2 deprecations:
    initializeOpenGL42Deprecations();

    // Initialize 4.3 deprecations:
    initializeOpenGL43Deprecations();

    // OpenGL ES does not define a deprecation model:
#ifdef _AMDT_OPENGLSERVER_EXPORTS
    {
        // Use the function deprecation data to initialize state variable deprecations:
        initializeStateVariableDeprecations();
    }
#endif
}


// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::getFunctionCallDeprecationStatus
// Description: Checks a function call deprecation status with the
//              OpenGL argument list
// Arguments: apMonitoredFunctionId calledFunctionId - the monitored function id
//            int argumentsAmount - the amount of function call arguments
//            va_list& pArgumentList - the arguments list
//            apFunctionDeprecationStatus& functionDeprecationStatus - the
//            function call deprecation status
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        3/3/2009
// ---------------------------------------------------------------------------
bool gsDeprecationAnalyzer::getFunctionCallDeprecationStatus(gsRenderContextMonitor* pContextMonitor, apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList, apFunctionDeprecationStatus& functionDeprecationStatus)
{
    bool retVal = false;

    // OpenGL ES does not have deprecation:
#if !((defined _GR_IPHONE_BUILD) || (defined _GR_OPENGLES_COMMON))
    // Initialize deprecation status as none:
    functionDeprecationStatus = AP_DEPRECATION_NONE;

    // Get the monitored function manager:
    // static apMonitoredFunctionsManager& monitoredFuncMgr = apMonitoredFunctionsManager::instance();

    // Check if function is deprecated:
    bool isFunctionDeprecated = ((apMonitoredFunctionsManager::instance().monitoredFunctionType(calledFunctionId) & AP_DEPRECATED_FUNC) != 0);

    // If the function is fully deprecated, set the deprecation status as fully deprecated:
    if (isFunctionDeprecated)
    {
        functionDeprecationStatus = AP_DEPRECATION_FULL;
        retVal = true;
    }
    else
    {
        retVal = true;
        // Get the function deprecations vector for this function id:
        gsFunctionDeprecations* pFunctionDeprecations = _monitoredFunctionsDeprecations[calledFunctionId];

        if (pFunctionDeprecations != NULL)
        {
            if (pFunctionDeprecations->size() > 0)
            {
                // There are no deprecations for this function:
                for (int i = 0; i < (int)pFunctionDeprecations->size(); i++)
                {
                    // Get the current deprecation condition:
                    gsDeprecationCondition* pCurrentCondition = (*pFunctionDeprecations)[i];
                    GT_IF_WITH_ASSERT(pCurrentCondition != NULL)
                    {
                        // Check if this deprecation condition takes place:
                        apFunctionDeprecationStatus deprStatus;
                        pCurrentCondition->checkFunctionCallDeprecationStatus(pContextMonitor, argumentsAmount, pArgumentList, deprStatus);

                        if (deprStatus != AP_DEPRECATION_NONE)
                        {
                            functionDeprecationStatus = deprStatus;
                            break;
                        }
                    }
                }
            }
        }
    }

#endif // !((defined _GR_IPHONE_BUILD) || (defined _GR_OPENGLES_COMMON))

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::getFunctionCallDeprecationDetails
// Description: Checks a function call deprecation status with a list of parameters
// Arguments: const apFunctionCall* pFunctionCall
//            const gtList<const apParameter*>& functionArguments
//            apFunctionDeprecation& functionDeprecationDetails
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/3/2009
// ---------------------------------------------------------------------------
bool gsDeprecationAnalyzer::getFunctionCallDeprecationDetails(const apFunctionCall* pFunctionCall, const gtList<const apParameter*>& functionArguments, apFunctionDeprecation& functionDeprecationDetails)
{
    bool retVal = false;

    // Initialize deprecation status as none:
    functionDeprecationDetails.setStatus(AP_DEPRECATION_NONE);

    GT_IF_WITH_ASSERT(pFunctionCall != NULL)
    {
        // Get the monitored function manager:
        static apMonitoredFunctionsManager& monitoredFuncMgr = apMonitoredFunctionsManager::instance();

        // Get the function id:
        apMonitoredFunctionId functionId = pFunctionCall->functionId();

        // Get the function type:
        unsigned int functionType = monitoredFuncMgr.monitoredFunctionType(functionId);

        // If the function is fully deprecated, set the deprecation status as fully deprecated:
        if (functionType & AP_DEPRECATED_FUNC)
        {
            functionDeprecationDetails.setStatus(AP_DEPRECATION_FULL);
            retVal = true;
        }
        else
        {
            retVal = true;
            // Get the function deprecations vector for this function id:
            gsFunctionDeprecations* pFunctionDeprecations = _monitoredFunctionsDeprecations[functionId];

            if (pFunctionDeprecations != NULL)
            {
                if (pFunctionDeprecations->size() > 0)
                {
                    // There are no deprecations for this function:
                    for (int i = 0; i < (int)pFunctionDeprecations->size(); i++)
                    {
                        // Get the current deprecation condition:
                        gsDeprecationCondition* pCurrentCondition = (*pFunctionDeprecations)[i];
                        GT_IF_WITH_ASSERT(pCurrentCondition != NULL)
                        {
                            // Check if this deprecation condition takes place:
                            apFunctionDeprecation currentDepracationDetails;
                            pCurrentCondition->checkFunctionCallDeprecationDetails(functionArguments, currentDepracationDetails);

                            if (currentDepracationDetails.status() != AP_DEPRECATION_NONE)
                            {
                                functionDeprecationDetails = currentDepracationDetails;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::initializeUngeneratedObjectsDeprecationsForOpenGL30
// Description: Initializes the deprecations conditions for unregenerated object
//              deprecation behavior
// Return Val: void
// Author:      Sigal Algranaty
// Date:        10/3/2009
// ---------------------------------------------------------------------------
void gsDeprecationAnalyzer::initializeUngeneratedObjectsDeprecationsForOpenGL30()
{
    // glBindTexture:

    // Initialize deprecation class for glBindTexture:
    gsFunctionDeprecations* pBindTextureDeprecation = new gsFunctionDeprecations;


    // Set glBindTexture deprecations:
    _monitoredFunctionsDeprecations[ap_glBindTexture] = pBindTextureDeprecation;

    // Create a non generated object deprecation condition for glBindTexture:
    gsUngeneratedObjectDeprecationCondition* pUngeneratedTextureCondition = new gsUngeneratedObjectDeprecationCondition(2, AP_GL_TEXTURE);


    // Add the non generated texture condition to glBindTexture deprecations:
    pBindTextureDeprecation->push_back(pUngeneratedTextureCondition);

    // glBindBuffer:
    // Initialize deprecation class for glBindBuffer:
    gsFunctionDeprecations* pBindBufferDeprecation = new gsFunctionDeprecations;


    // Set glBindBuffer deprecations:
    _monitoredFunctionsDeprecations[ap_glBindBuffer] = pBindBufferDeprecation;

    // Create a non generated object deprecation condition for glBindBuffer:
    gsUngeneratedObjectDeprecationCondition* pUngeneratedBufferCondition = new gsUngeneratedObjectDeprecationCondition(2, AP_GL_VBO);


    // Add the non generated texture condition to glBindBuffer deprecations:
    pBindBufferDeprecation->push_back(pUngeneratedBufferCondition);

    // glBindBufferARB:
    // Initialize deprecation class for glBindBufferArb:
    gsFunctionDeprecations* pBindBufferARBDeprecation = new gsFunctionDeprecations;


    // Set glBindBuffer deprecations:
    _monitoredFunctionsDeprecations[ap_glBindBufferARB] = pBindBufferARBDeprecation;

    // Create a non generated object deprecation condition for glBindBuffer:
    gsUngeneratedObjectDeprecationCondition* pUngeneratedBufferARBCondition = new gsUngeneratedObjectDeprecationCondition(2, AP_GL_VBO);


    // Add the non generated texture condition to glBindBuffer deprecations:
    pBindBufferARBDeprecation->push_back(pUngeneratedBufferARBCondition);

    // glNewList
    // Initialize deprecation class for glNewList:
    gsFunctionDeprecations* pNewListDeprecation = new gsFunctionDeprecations;


    // Set glNewList deprecations:
    _monitoredFunctionsDeprecations[ap_glNewList] = pNewListDeprecation;

    // Create a non generated object deprecation condition for glNewList:
    gsUngeneratedObjectDeprecationCondition* pListUngeneratedBufferCondition = new gsUngeneratedObjectDeprecationCondition(1, AP_GL_DISPLAY_LIST);


    // Add the non generated texture condition to glNewList deprecations:
    pNewListDeprecation->push_back(pListUngeneratedBufferCondition);

    // glDeleteList
    // Initialize deprecation class for glDeleteList:
    gsFunctionDeprecations* pDeleteListDeprecation = new gsFunctionDeprecations;


    // Set glDeleteList deprecations:
    _monitoredFunctionsDeprecations[ap_glDeleteLists] = pDeleteListDeprecation;

    // Create a non generated object deprecation condition for glDeleteList:
    gsUngeneratedObjectDeprecationCondition* pDeleteListUngeneratedBufferCondition = new gsUngeneratedObjectDeprecationCondition(1, AP_GL_DISPLAY_LIST);


    // Add the non generated texture condition to glDeleteList deprecations:
    pDeleteListDeprecation->push_back(pDeleteListUngeneratedBufferCondition);


    // glVertexPointer
    // Initialize deprecation class for glVertexPointer:
    gsFunctionDeprecations* pVertexPointerDeprecation = new gsFunctionDeprecations;


    // Set glVertexPointer deprecations:
    _monitoredFunctionsDeprecations[ap_glVertexPointer] = pVertexPointerDeprecation;

    // Create a non generated object deprecation condition for glDeleteList:
    gsUnboundedVertexArrayPointerDeprecationCondition* pVertexPointerUnboundArrayBufferCondition = new gsUnboundedVertexArrayPointerDeprecationCondition();


    // Add the non generated texture condition to glDeleteList deprecations:
    pVertexPointerDeprecation->push_back(pVertexPointerUnboundArrayBufferCondition);

}


// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::initializeInvalidArgumentDeprecationsForOpenGL30
// Description: Initializes the deprecation conditions for invalid argument
//              function calls
// Return Val: void
// Author:      Sigal Algranaty
// Date:        10/3/2009
// ---------------------------------------------------------------------------
void gsDeprecationAnalyzer::initializeInvalidArgumentDeprecationsForOpenGL30()
{
    // Get the deprecated internal pixel format values:
    gtVector<int> internalPixelFormatDeprecatedValues;
    getDeprecatedInternalFormats(internalPixelFormatDeprecatedValues);

    // Add pixel format deprecations:
    addPixelFormatDeprecatedValues(internalPixelFormatDeprecatedValues, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationInternalFormatArgName, 3, ap_glTexImage1D);
    addPixelFormatDeprecatedValues(internalPixelFormatDeprecatedValues, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationInternalFormatArgName, 3, ap_glTexImage2D);
    addPixelFormatDeprecatedValues(internalPixelFormatDeprecatedValues, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationInternalFormatArgName, 3, ap_glTexImage3D);
    addPixelFormatDeprecatedValues(internalPixelFormatDeprecatedValues, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationInternalFormatArgName, 3, ap_glCompressedTexImage1D);
    addPixelFormatDeprecatedValues(internalPixelFormatDeprecatedValues, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationInternalFormatArgName, 3, ap_glCompressedTexImage2D);
    addPixelFormatDeprecatedValues(internalPixelFormatDeprecatedValues, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationInternalFormatArgName, 3, ap_glCompressedTexImage3D);
    addPixelFormatDeprecatedValues(internalPixelFormatDeprecatedValues, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationInternalFormatArgName, 5, ap_glCompressedTexSubImage1D);
    addPixelFormatDeprecatedValues(internalPixelFormatDeprecatedValues, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationInternalFormatArgName, 5, ap_glCompressedTexSubImage2D);
    addPixelFormatDeprecatedValues(internalPixelFormatDeprecatedValues, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationInternalFormatArgName, 5, ap_glCompressedTexSubImage3D);
    addPixelFormatDeprecatedValues(internalPixelFormatDeprecatedValues, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationInternalFormatArgName, 3, ap_glCopyTexImage1D);
    addPixelFormatDeprecatedValues(internalPixelFormatDeprecatedValues, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationInternalFormatArgName, 3, ap_glCopyTexImage2D);
    addPixelFormatDeprecatedValues(internalPixelFormatDeprecatedValues, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationInternalFormatArgName, 2, ap_glColorTable);
    addPixelFormatDeprecatedValues(internalPixelFormatDeprecatedValues, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationInternalFormatArgName, 2, ap_glSeparableFilter2D);
    addPixelFormatDeprecatedValues(internalPixelFormatDeprecatedValues, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationInternalFormatArgName, 2, ap_glConvolutionFilter1D);
    addPixelFormatDeprecatedValues(internalPixelFormatDeprecatedValues, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationInternalFormatArgName, 2, ap_glCopyConvolutionFilter1D);
    addPixelFormatDeprecatedValues(internalPixelFormatDeprecatedValues, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationInternalFormatArgName, 3, ap_glHistogram);
    addPixelFormatDeprecatedValues(internalPixelFormatDeprecatedValues, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationInternalFormatArgName, 2, ap_glMinmax);

    // Get the deprecated external pixel format values:
    gtVector<GLenum> externalPixelFormatDeprecatedValue;
    getDeprecatedExternalFormats(externalPixelFormatDeprecatedValue);

    // Add external pixel format deprecated values:
    addEnumDeprecatedValues(externalPixelFormatDeprecatedValue, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationFormatArgName, 6, ap_glTexImage1D);
    addEnumDeprecatedValues(externalPixelFormatDeprecatedValue, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationFormatArgName, 7, ap_glTexImage2D);
    addEnumDeprecatedValues(externalPixelFormatDeprecatedValue, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationFormatArgName, 8, ap_glTexImage3D);
    addEnumDeprecatedValues(externalPixelFormatDeprecatedValue, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationFormatArgName, 4, ap_glConvolutionFilter1D);
    addEnumDeprecatedValues(externalPixelFormatDeprecatedValue, AP_DEPRECATION_PIXEL_FORMAT, GS_STR_DeprecationFormatArgName, 5, ap_glConvolutionFilter2D);

    // Add fixed pipeline processing deprecations:
    addEnumDeprecatedValueForEnableFunctions(GL_VERTEX_ARRAY, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_VERTEX_ARRAY_BUFFER_BINDING, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValue(GL_VERTEX_ARRAY_POINTER, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationPNameArgName, 1, ap_glGetPointerv);
    addEnumDeprecatedValueForGetFunctions(GL_VERTEX_ARRAY_TYPE, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_VERTEX_ARRAY_SIZE, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_VERTEX_ARRAY_STRIDE, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);

    addEnumDeprecatedValueForEnableFunctions(GL_NORMALIZE, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForEnableFunctions(GL_RESCALE_NORMAL, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForEnableFunctions(GL_NORMAL_ARRAY, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_NORMAL_ARRAY_BUFFER_BINDING, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValue(GL_NORMAL_ARRAY_POINTER, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationPNameArgName, 1, ap_glGetPointerv);
    addEnumDeprecatedValueForGetFunctions(GL_NORMAL_ARRAY_TYPE, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_NORMAL_ARRAY_STRIDE, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_CURRENT_NORMAL, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);

    addEnumDeprecatedValueForEnableFunctions(GL_TEXTURE_GEN_S, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_S, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGend);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_S, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGendv);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_S, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGenf);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_S, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGenfv);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_S, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGeni);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_S, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGeniv);

    addEnumDeprecatedValueForEnableFunctions(GL_TEXTURE_GEN_T, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_T, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGend);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_T, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGendv);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_T, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGenf);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_T, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGenfv);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_T, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGeni);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_T, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGeniv);

    addEnumDeprecatedValueForEnableFunctions(GL_TEXTURE_GEN_R, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_R, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGend);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_R, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGendv);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_R, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGenf);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_R, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGenfv);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_R, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGeni);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_R, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGeniv);

    addEnumDeprecatedValueForEnableFunctions(GL_TEXTURE_GEN_Q, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_Q, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGend);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_Q, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGendv);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_Q, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGenf);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_Q, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGenfv);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_Q, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGeni);
    addEnumDeprecatedValue(GL_TEXTURE_GEN_Q, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationCoordArgName, 1, ap_glTexGeniv);

    addEnumDeprecatedValueForEnableFunctions(GL_LIGHTING, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForEnableFunctions(GL_LIGHT0, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForEnableFunctions(GL_LIGHT1, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForEnableFunctions(GL_LIGHT2, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForEnableFunctions(GL_LIGHT3, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForEnableFunctions(GL_LIGHT4, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForEnableFunctions(GL_LIGHT5, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForEnableFunctions(GL_LIGHT6, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForEnableFunctions(GL_LIGHT7, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);

    addEnumDeprecatedValueForEnableFunctions(GL_COLOR_MATERIAL, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);

    addEnumDeprecatedValueForGetFunctions(GL_MATRIX_MODE, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_MODELVIEW_MATRIX, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_MODELVIEW_STACK_DEPTH, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_PROJECTION_MATRIX, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_PROJECTION_STACK_DEPTH, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);

    addEnumDeprecatedValueForEnableFunctions(GL_VERTEX_PROGRAM_TWO_SIDE, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);

    addEnumDeprecatedValueForEnableFunctions(GL_COLOR_ARRAY, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_COLOR_ARRAY_BUFFER_BINDING, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValue(GL_COLOR_ARRAY_POINTER, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationPNameArgName, 1, ap_glGetPointerv);
    addEnumDeprecatedValueForGetFunctions(GL_COLOR_ARRAY_TYPE, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_COLOR_ARRAY_SIZE, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_COLOR_ARRAY_STRIDE, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_CURRENT_COLOR, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForEnableFunctions(GL_SECONDARY_COLOR_ARRAY, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValue(GL_SECONDARY_COLOR_ARRAY_POINTER, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationPNameArgName, 1, ap_glGetPointerv);
    addEnumDeprecatedValueForGetFunctions(GL_SECONDARY_COLOR_ARRAY_TYPE, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_SECONDARY_COLOR_ARRAY_SIZE, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_SECONDARY_COLOR_ARRAY_STRIDE, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_CURRENT_SECONDARY_COLOR, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);

    addEnumDeprecatedValueForEnableFunctions(GL_TEXTURE_COORD_ARRAY, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValue(GL_TEXTURE_COORD_ARRAY_POINTER, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationPNameArgName, 1, ap_glGetPointerv);
    addEnumDeprecatedValueForGetFunctions(GL_TEXTURE_COORD_ARRAY_TYPE, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_TEXTURE_COORD_ARRAY_SIZE, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_TEXTURE_COORD_ARRAY_STRIDE, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_CURRENT_TEXTURE_COORDS, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);

    addEnumDeprecatedValueForGetFunctions(GL_EDGE_FLAG_ARRAY, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_EDGE_FLAG_ARRAY_BUFFER_BINDING, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_EDGE_FLAG_ARRAY_POINTER, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_EDGE_FLAG_ARRAY_STRIDE, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);
    addEnumDeprecatedValueForGetFunctions(GL_EDGE_FLAG, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING);

    // Add deprecation for glClampColor functions:
    addEnumDeprecatedValue(GL_CLAMP_FRAGMENT_COLOR, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationTargetArgName, 1, ap_glClampColor);
    addEnumDeprecatedValue(GL_CLAMP_FRAGMENT_COLOR, AP_DEPRECATION_FIXED_PIPELINE_VERTEX_PROCESSING, GS_STR_DeprecationTargetArgName, 1, ap_glClampColor);

    // Add color Index mode deprecations:
    addEnumDeprecatedValueForGetFunctions(GL_INDEX_ARRAY, AP_DEPRECATION_COLOR_INDEX_MODE);
    addEnumDeprecatedValueForGetFunctions(GL_INDEX_ARRAY_BUFFER_BINDING, AP_DEPRECATION_COLOR_INDEX_MODE);
    addEnumDeprecatedValueForGetFunctions(GL_INDEX_ARRAY_POINTER, AP_DEPRECATION_COLOR_INDEX_MODE);
    addEnumDeprecatedValueForGetFunctions(GL_INDEX_ARRAY_TYPE, AP_DEPRECATION_COLOR_INDEX_MODE);
    addEnumDeprecatedValueForGetFunctions(GL_INDEX_ARRAY_STRIDE, AP_DEPRECATION_COLOR_INDEX_MODE);
    addEnumDeprecatedValueForGetFunctions(GL_CURRENT_INDEX, AP_DEPRECATION_COLOR_INDEX_MODE);

    // Add display list deprecations:
    addEnumDeprecatedValueForGetFunctions(GL_LIST_INDEX, AP_DEPRECATION_DISPLAY_LISTS);
    addEnumDeprecatedValueForGetFunctions(GL_LIST_MODE, AP_DEPRECATION_DISPLAY_LISTS);
    addEnumDeprecatedValueForGetFunctions(GL_LIST_BASE, AP_DEPRECATION_DISPLAY_LISTS);
    addEnumDeprecatedValueForGetFunctions(GL_MAX_LIST_NESTING, AP_DEPRECATION_DISPLAY_LISTS);

    // Add attribute stack deprecations:
    addEnumDeprecatedValueForGetFunctions(GL_MAX_ATTRIB_STACK_DEPTH, AP_DEPRECATION_ATTRIBUTE_STACKS_STATE);

    // Add state variables for raster position functions:
    addEnumDeprecatedValueForGetFunctions(GL_CURRENT_RASTER_POSITION, AP_DEPRECATION_RASTER_POS_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_CURRENT_RASTER_POSITION_VALID, AP_DEPRECATION_RASTER_POS_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_CURRENT_RASTER_DISTANCE, AP_DEPRECATION_RASTER_POS_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_CURRENT_RASTER_COLOR, AP_DEPRECATION_RASTER_POS_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_CURRENT_RASTER_SECONDARY_COLOR, AP_DEPRECATION_RASTER_POS_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_CURRENT_RASTER_INDEX, AP_DEPRECATION_RASTER_POS_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_CURRENT_RASTER_TEXTURE_COORDS, AP_DEPRECATION_RASTER_POS_STATE);

    // Add polygon mode state variables:
    addEnumDeprecatedValueForGetFunctions(GL_POLYGON_MODE, AP_DEPRECATION_SEPERATE_POLYGON_DRAW_MODE);
    addEnumDeprecatedValue(GL_FRONT, AP_DEPRECATION_SEPERATE_POLYGON_DRAW_MODE, GS_STR_DeprecationPNameArgName, 1, ap_glPolygonMode);
    addEnumDeprecatedValue(GL_BACK, AP_DEPRECATION_SEPERATE_POLYGON_DRAW_MODE, GS_STR_DeprecationPNameArgName, 1, ap_glPolygonMode);

    // Add polygon stipple state variables:
    addEnumDeprecatedValueForEnableFunctions(GL_POLYGON_STIPPLE, AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_LINE_STIPPLE, AP_DEPRECATION_POLYGON_AND_LINE_STIPPLE_STATE);

    // Add bitmap deprecations:
    addEnumDeprecatedValue(GL_BITMAP, AP_DEPRECATION_BITMAP, GS_STR_DeprecationTypeArgName, 6, ap_glReadPixels);
    addEnumDeprecatedValue(GL_BITMAP, AP_DEPRECATION_BITMAP, GS_STR_DeprecationTypeArgName, 7, ap_glTexImage1D);
    addEnumDeprecatedValue(GL_BITMAP, AP_DEPRECATION_BITMAP, GS_STR_DeprecationTypeArgName, 8, ap_glTexImage2D);
    addEnumDeprecatedValue(GL_BITMAP, AP_DEPRECATION_BITMAP, GS_STR_DeprecationTypeArgName, 10, ap_glTexSubImage2D);
    addEnumDeprecatedValue(GL_BITMAP, AP_DEPRECATION_BITMAP, GS_STR_DeprecationTypeArgName, 3, ap_glGetColorTable);
    addEnumDeprecatedValue(GL_BITMAP, AP_DEPRECATION_BITMAP, GS_STR_DeprecationTypeArgName, 5, ap_glConvolutionFilter1D);
    addEnumDeprecatedValue(GL_BITMAP, AP_DEPRECATION_BITMAP, GS_STR_DeprecationTypeArgName, 4, ap_glGetTexImage);
    addEnumDeprecatedValue(GL_BITMAP, AP_DEPRECATION_BITMAP, GS_STR_DeprecationTypeArgName, 4, ap_glGetHistogram);
    addEnumDeprecatedValue(GL_BITMAP, AP_DEPRECATION_BITMAP, GS_STR_DeprecationTypeArgName, 3, ap_glGetConvolutionFilter);

    // Add clamp deprecations:
    addEnumDeprecatedValue(GL_CLAMP, AP_DEPRECATION_TEXTURE_CLAMP_WRAP_MODE, GS_STR_DeprecationParamArgName, 3, ap_glTexParameterf);
    addEnumDeprecatedValue(GL_CLAMP, AP_DEPRECATION_TEXTURE_CLAMP_WRAP_MODE, GS_STR_DeprecationParamArgName, 3, ap_glTexParameteri);
    addEnumDeprecatedValue(GL_CLAMP, AP_DEPRECATION_TEXTURE_CLAMP_WRAP_MODE, GS_STR_DeprecationParamArgName, 3, ap_glTexParameterfv);
    addEnumDeprecatedValue(GL_CLAMP, AP_DEPRECATION_TEXTURE_CLAMP_WRAP_MODE, GS_STR_DeprecationParamArgName, 3, ap_glTexParameteriv);

    // Add texture mipmap auto generation deprecation:
    addEnumDeprecatedValue(GL_GENERATE_MIPMAP, AP_DEPRECATION_TEXTURE_AUTO_MIPMAP, GS_STR_DeprecationPNameArgName, 2, ap_glTexParameterf);
    addEnumDeprecatedValue(GL_GENERATE_MIPMAP, AP_DEPRECATION_TEXTURE_AUTO_MIPMAP, GS_STR_DeprecationPNameArgName, 2, ap_glTexParameteri);
    addEnumDeprecatedValue(GL_GENERATE_MIPMAP, AP_DEPRECATION_TEXTURE_AUTO_MIPMAP, GS_STR_DeprecationPNameArgName, 2, ap_glTexParameterfv);
    addEnumDeprecatedValue(GL_GENERATE_MIPMAP, AP_DEPRECATION_TEXTURE_AUTO_MIPMAP, GS_STR_DeprecationPNameArgName, 2, ap_glTexParameteriv);
    addEnumDeprecatedValue(GL_GENERATE_MIPMAP, AP_DEPRECATION_TEXTURE_AUTO_MIPMAP, GS_STR_DeprecationPNameArgName, 2, ap_glGetTexParameterfv);
    addEnumDeprecatedValue(GL_GENERATE_MIPMAP, AP_DEPRECATION_TEXTURE_AUTO_MIPMAP, GS_STR_DeprecationPNameArgName, 2, ap_glGetTexParameteriv);
    addEnumDeprecatedValueForGetFunctions(GL_GENERATE_MIPMAP_HINT, AP_DEPRECATION_TEXTURE_AUTO_MIPMAP);

    // Add alpha test deprecations:
    addEnumDeprecatedValueForEnableFunctions(GL_ALPHA_TEST, AP_DEPRECATION_ALPHA_TEST_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_ALPHA_TEST_FUNC, AP_DEPRECATION_ALPHA_TEST_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_ALPHA_TEST_REF, AP_DEPRECATION_ALPHA_TEST_STATE);

    // Add evaluators deprecations:
    addEnumDeprecatedValueForGetFunctions(GL_MAX_EVAL_ORDER, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_MAP1_GRID_DOMAIN, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_MAP2_GRID_DOMAIN, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_MAP1_GRID_SEGMENTS, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_MAP2_GRID_SEGMENTS, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_MAP1_VERTEX_3, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_MAP2_VERTEX_3, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_MAP1_VERTEX_4, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_MAP2_VERTEX_4, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_MAP1_INDEX, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_MAP2_INDEX, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_MAP1_COLOR_4, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_MAP2_COLOR_4, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_MAP1_NORMAL, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_MAP2_NORMAL, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_MAP1_TEXTURE_COORD_1, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_MAP1_TEXTURE_COORD_2, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_MAP1_TEXTURE_COORD_3, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_MAP1_TEXTURE_COORD_4, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_MAP2_TEXTURE_COORD_1, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_MAP2_TEXTURE_COORD_2, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_MAP2_TEXTURE_COORD_3, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_MAP2_TEXTURE_COORD_4, AP_DEPRECATION_EVALUATORS_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_AUTO_NORMAL, AP_DEPRECATION_EVALUATORS_STATE);

    // Selection and feedback modes:
    addEnumDeprecatedValueForGetFunctions(GL_RENDER_MODE, AP_DEPRECATION_FEEDBACK);
    addEnumDeprecatedValue(GL_FEEDBACK_BUFFER_POINTER, AP_DEPRECATION_FEEDBACK, GS_STR_DeprecationPNameArgName, 1, ap_glGetPointerv);
    addEnumDeprecatedValueForGetFunctions(GL_FEEDBACK_BUFFER_SIZE, AP_DEPRECATION_FEEDBACK);
    addEnumDeprecatedValueForGetFunctions(GL_FEEDBACK_BUFFER_TYPE, AP_DEPRECATION_FEEDBACK);
    addEnumDeprecatedValue(GL_SELECTION_BUFFER_POINTER, AP_DEPRECATION_FEEDBACK, GS_STR_DeprecationPNameArgName, 1, ap_glGetPointerv);
    addEnumDeprecatedValueForGetFunctions(GL_SELECTION_BUFFER_SIZE, AP_DEPRECATION_FEEDBACK);
    addEnumDeprecatedValueForGetFunctions(GL_NAME_STACK_DEPTH, AP_DEPRECATION_FEEDBACK);
    addEnumDeprecatedValueForGetFunctions(GL_MAX_NAME_STACK_DEPTH, AP_DEPRECATION_FEEDBACK);

    // Add Hints deprecations:
    addEnumDeprecatedValue(GL_PERSPECTIVE_CORRECTION_HINT, AP_DEPRECATION_HINTS, GS_STR_DeprecationTargetArgName, 1, ap_glHint);
    addEnumDeprecatedValueForGetFunctions(GL_PERSPECTIVE_CORRECTION_HINT, AP_DEPRECATION_HINTS);
    addEnumDeprecatedValue(GL_POINT_SMOOTH_HINT, AP_DEPRECATION_HINTS, GS_STR_DeprecationTargetArgName, 1, ap_glHint);
    addEnumDeprecatedValueForGetFunctions(GL_POINT_SMOOTH_HINT, AP_DEPRECATION_HINTS);
    addEnumDeprecatedValue(GL_FOG_HINT, AP_DEPRECATION_HINTS, GS_STR_DeprecationTargetArgName, 1, ap_glHint);
    addEnumDeprecatedValueForGetFunctions(GL_FOG_HINT, AP_DEPRECATION_HINTS);
    addEnumDeprecatedValue(GL_GENERATE_MIPMAP_HINT, AP_DEPRECATION_HINTS, GS_STR_DeprecationTargetArgName, 1, ap_glHint);
    // addEnumDeprecatedValueForGetFunctions(GL_GENERATE_MIPMAP_HINT, AP_DEPRECATION_HINTS); // is added as AP_DEPRECATION_TEXTURE_AUTO_MIPMAP

    // Add Non sprite points deprecations:
    addEnumDeprecatedValueForEnableFunctions(GL_POINT_SMOOTH, AP_DEPRECATION_NON_SPRITE_POINTS);
    addEnumDeprecatedValueForEnableFunctions(GL_POINT_SPRITE, AP_DEPRECATION_NON_SPRITE_POINTS);

    // Add Line width deprecations:
    addFloatComparisonDeprecatedValue(1.0f, GS_SMALLER, AP_DEPRECATION_LINE_WIDTH, GS_STR_DeprecationWidthName, 1, ap_glLineWidth);

    // Add texture border deprecations:
    addFloatComparisonDeprecatedValue(0.0f, GS_EQUAL, AP_DEPRECATION_TEXTURE_BORDER, GS_STR_DeprecationBorderName, 5, ap_glTexImage1D);
    addFloatComparisonDeprecatedValue(0.0f, GS_EQUAL, AP_DEPRECATION_TEXTURE_BORDER, GS_STR_DeprecationBorderName, 6, ap_glTexImage2D);
    addFloatComparisonDeprecatedValue(0.0f, GS_EQUAL, AP_DEPRECATION_TEXTURE_BORDER, GS_STR_DeprecationBorderName, 7, ap_glTexImage3D);
    addFloatComparisonDeprecatedValue(0.0f, GS_EQUAL, AP_DEPRECATION_TEXTURE_BORDER, GS_STR_DeprecationBorderName, 7, ap_glCompressedTexImage3D);
    addFloatComparisonDeprecatedValue(0.0f, GS_EQUAL, AP_DEPRECATION_TEXTURE_BORDER, GS_STR_DeprecationBorderName, 7, ap_glCopyTexImage1D);
    addFloatComparisonDeprecatedValue(0.0f, GS_EQUAL, AP_DEPRECATION_TEXTURE_BORDER, GS_STR_DeprecationBorderName, 8, ap_glCopyTexImage2D);

    // Add fixed function fragment processing deprecations:
    addEnumDeprecatedValue(GL_TEXTURE_PRIORITY, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE, GS_STR_DeprecationPNameArgName, 2, ap_glTexParameterf);
    addEnumDeprecatedValue(GL_TEXTURE_PRIORITY, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE, GS_STR_DeprecationPNameArgName, 2, ap_glTexParameteri);
    addEnumDeprecatedValue(GL_TEXTURE_PRIORITY, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE, GS_STR_DeprecationPNameArgName, 2, ap_glTexParameterfv);
    addEnumDeprecatedValue(GL_TEXTURE_PRIORITY, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE, GS_STR_DeprecationPNameArgName, 2, ap_glTexParameteriv);
    addEnumDeprecatedValue(GL_TEXTURE_PRIORITY, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE, GS_STR_DeprecationPNameArgName, 2, ap_glGetTexParameterfv);
    addEnumDeprecatedValue(GL_TEXTURE_PRIORITY, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE, GS_STR_DeprecationPNameArgName, 2, ap_glGetTexParameteriv);
    addEnumDeprecatedValue(GL_TEXTURE_ENV, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE, GS_STR_DeprecationTargetArgName, 1, ap_glTexEnvf);
    addEnumDeprecatedValue(GL_TEXTURE_ENV, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE, GS_STR_DeprecationTargetArgName, 1, ap_glTexEnvfv);
    addEnumDeprecatedValue(GL_TEXTURE_ENV, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE, GS_STR_DeprecationTargetArgName, 1, ap_glTexEnvi);
    addEnumDeprecatedValue(GL_TEXTURE_ENV, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE, GS_STR_DeprecationTargetArgName, 1, ap_glTexEnviv);
    addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GL_TEXTURE_ENV_MODE, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GL_TEXTURE_ENV_COLOR, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GL_COMBINE_RGB, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GL_COMBINE_ALPHA, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GL_RGB_SCALE, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GL_ALPHA_SCALE, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GL_SRC0_RGB, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GL_SRC1_RGB, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GL_SRC2_RGB, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GL_SRC0_ALPHA, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GL_SRC1_ALPHA, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GL_SRC2_ALPHA, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GL_OPERAND0_RGB, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GL_OPERAND1_RGB, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GL_OPERAND2_RGB, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GL_OPERAND0_ALPHA, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GL_OPERAND1_ALPHA, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GL_OPERAND2_ALPHA, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GL_TEXTURE_LOD_BIAS, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_TEXTURE_1D, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_TEXTURE_2D, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_TEXTURE_3D, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_TEXTURE_1D_ARRAY, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_TEXTURE_2D_ARRAY, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_TEXTURE_CUBE_MAP, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_TEXTURE_MATRIX, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_COLOR_SUM, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForEnableFunctions(GL_FOG, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_FOG_COLOR, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_FOG_INDEX, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_FOG_DENSITY, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_FOG_START, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_FOG_END, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_FOG_MODE, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    // Note that GL_FOG_COORD_ARRAY_xxx are all aliases for the old GL_FOG_COORDINATE_xxx, so there is no need to do this twice:
    addEnumDeprecatedValueForEnableFunctions(GL_FOG_COORD_ARRAY, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_FOG_COORD_ARRAY_BUFFER_BINDING, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValue(GL_FOG_COORD_ARRAY_POINTER, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE, GS_STR_DeprecationPNameArgName, 1, ap_glGetPointerv);
    addEnumDeprecatedValueForGetFunctions(GL_FOG_COORD_ARRAY_STRIDE, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_FOG_COORD_ARRAY_TYPE, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_CURRENT_FOG_COORD, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_MAX_TEXTURE_UNITS, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_MAX_TEXTURE_COORDS, AP_DEPRECATION_FIXED_FUNCTION_FRAGMENT_PROCESSING_STATE);

    // Add accumulation buffers deprecation:
    addGLClearBitfieldDeprecatedValue(GL_ACCUM_BUFFER_BIT, AP_DEPRECATION_ACCUMMULATION_BUFFERS_FUNCTION, GS_STR_DeprecationMaskName, 1, ap_glClear);
    addEnumDeprecatedValueForGetFunctions(GL_ACCUM_RED_BITS, AP_DEPRECATION_ACCUMMULATION_BUFFERS_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_ACCUM_GREEN_BITS, AP_DEPRECATION_ACCUMMULATION_BUFFERS_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_ACCUM_BLUE_BITS, AP_DEPRECATION_ACCUMMULATION_BUFFERS_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_ACCUM_ALPHA_BITS, AP_DEPRECATION_ACCUMMULATION_BUFFERS_STATE);
    addEnumDeprecatedValueForGetFunctions(GL_ACCUM_CLEAR_VALUE, AP_DEPRECATION_ACCUMMULATION_BUFFERS_STATE);

    // Add framebuffer size queries deprecations:
    addEnumDeprecatedValueForGetFunctions(GL_RED_BITS, AP_DEPRECATION_FRAMEBUFFER_SIZE_QUERIES);
    addEnumDeprecatedValueForGetFunctions(GL_GREEN_BITS, AP_DEPRECATION_FRAMEBUFFER_SIZE_QUERIES);
    addEnumDeprecatedValueForGetFunctions(GL_BLUE_BITS, AP_DEPRECATION_FRAMEBUFFER_SIZE_QUERIES);
    addEnumDeprecatedValueForGetFunctions(GL_ALPHA_BITS, AP_DEPRECATION_FRAMEBUFFER_SIZE_QUERIES);
    addEnumDeprecatedValueForGetFunctions(GL_DEPTH_BITS, AP_DEPRECATION_FRAMEBUFFER_SIZE_QUERIES);
    addEnumDeprecatedValueForGetFunctions(GL_STENCIL_BITS, AP_DEPRECATION_FRAMEBUFFER_SIZE_QUERIES);

    // Add quadrilateral and polygon primitives deprecations:
    addEnumDeprecatedValue(GL_POLYGON, AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES, GS_STR_DeprecationModeArgName, 1, ap_glDrawElements);
    addEnumDeprecatedValue(GL_QUAD_STRIP, AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES, GS_STR_DeprecationModeArgName, 1, ap_glDrawElements);
    addEnumDeprecatedValue(GL_QUADS, AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES, GS_STR_DeprecationModeArgName, 1, ap_glDrawElements);
    addEnumDeprecatedValue(GL_POLYGON, AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES, GS_STR_DeprecationModeArgName, 1, ap_glDrawRangeElements);
    addEnumDeprecatedValue(GL_QUAD_STRIP, AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES, GS_STR_DeprecationModeArgName, 1, ap_glDrawRangeElements);
    addEnumDeprecatedValue(GL_QUADS, AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES, GS_STR_DeprecationModeArgName, 1, ap_glDrawRangeElements);
    addEnumDeprecatedValue(GL_POLYGON, AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES, GS_STR_DeprecationModeArgName, 1, ap_glMultiDrawElements);
    addEnumDeprecatedValue(GL_QUAD_STRIP, AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES, GS_STR_DeprecationModeArgName, 1, ap_glMultiDrawElements);
    addEnumDeprecatedValue(GL_QUADS, AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES, GS_STR_DeprecationModeArgName, 1, ap_glMultiDrawElements);
    addEnumDeprecatedValue(GL_POLYGON, AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES, GS_STR_DeprecationModeArgName, 1, ap_glMultiDrawArrays);
    addEnumDeprecatedValue(GL_QUAD_STRIP, AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES, GS_STR_DeprecationModeArgName, 1, ap_glMultiDrawArrays);
    addEnumDeprecatedValue(GL_QUADS, AP_DEPRECATION_POLYGON_QUADS_PRIMITIVES, GS_STR_DeprecationModeArgName, 1, ap_glMultiDrawArrays);

    // Add auxiliary buffers deprecations:
    addEnumDeprecatedValue(GL_AUX0, AP_DEPRECATION_AUXILIRY_BUFFERS, GS_STR_DeprecationModeArgName, 1, ap_glDrawBuffer);
    addEnumDeprecatedValue(GL_AUX1, AP_DEPRECATION_AUXILIRY_BUFFERS, GS_STR_DeprecationModeArgName, 1, ap_glDrawBuffer);
    addEnumDeprecatedValue(GL_AUX2, AP_DEPRECATION_AUXILIRY_BUFFERS, GS_STR_DeprecationModeArgName, 1, ap_glDrawBuffer);
    addEnumDeprecatedValue(GL_AUX3, AP_DEPRECATION_AUXILIRY_BUFFERS, GS_STR_DeprecationModeArgName, 1, ap_glDrawBuffer);
    addEnumDeprecatedValue(GL_AUX0, AP_DEPRECATION_AUXILIRY_BUFFERS, GS_STR_DeprecationModeArgName, 1, ap_glDrawBuffers);
    addEnumDeprecatedValue(GL_AUX1, AP_DEPRECATION_AUXILIRY_BUFFERS, GS_STR_DeprecationModeArgName, 1, ap_glDrawBuffers);
    addEnumDeprecatedValue(GL_AUX2, AP_DEPRECATION_AUXILIRY_BUFFERS, GS_STR_DeprecationModeArgName, 1, ap_glDrawBuffers);
    addEnumDeprecatedValue(GL_AUX3, AP_DEPRECATION_AUXILIRY_BUFFERS, GS_STR_DeprecationModeArgName, 1, ap_glDrawBuffers);
    addEnumDeprecatedValue(GL_AUX0, AP_DEPRECATION_AUXILIRY_BUFFERS, GS_STR_DeprecationModeArgName, 1, ap_glReadBuffer);
    addEnumDeprecatedValue(GL_AUX1, AP_DEPRECATION_AUXILIRY_BUFFERS, GS_STR_DeprecationModeArgName, 1, ap_glReadBuffer);
    addEnumDeprecatedValue(GL_AUX2, AP_DEPRECATION_AUXILIRY_BUFFERS, GS_STR_DeprecationModeArgName, 1, ap_glReadBuffer);
    addEnumDeprecatedValue(GL_AUX3, AP_DEPRECATION_AUXILIRY_BUFFERS, GS_STR_DeprecationModeArgName, 1, ap_glReadBuffer);

    // Add unified extension string deprecation:
    addEnumDeprecatedValue(GL_EXTENSIONS, AP_DEPRECATION_UNIFIED_EXTENSION_STRING, GS_STR_DeprecationNameArgName, 1, ap_glGetString);

    // Add pixel tranfer deprecation:
    addEnumDeprecatedValueForGetFunctions(GL_COLOR_MATRIX, AP_DEPRECATION_PIXEL_TRANSFER);
    addEnumDeprecatedValueForGetFunctions(GL_COLOR_MATRIX_STACK_DEPTH, AP_DEPRECATION_PIXEL_TRANSFER);
    addEnumDeprecatedValueForGetFunctions(GL_MAP_COLOR, AP_DEPRECATION_PIXEL_TRANSFER);
    addEnumDeprecatedValueForGetFunctions(GL_MAP_STENCIL, AP_DEPRECATION_PIXEL_TRANSFER);
    addEnumDeprecatedValueForGetFunctions(GL_INDEX_SHIFT, AP_DEPRECATION_PIXEL_TRANSFER);
    addEnumDeprecatedValueForGetFunctions(GL_INDEX_OFFSET, AP_DEPRECATION_PIXEL_TRANSFER);
    addEnumDeprecatedValueForGetFunctions(GL_RED_SCALE, AP_DEPRECATION_PIXEL_TRANSFER);
    addEnumDeprecatedValueForGetFunctions(GL_RED_BIAS, AP_DEPRECATION_PIXEL_TRANSFER);
    addEnumDeprecatedValueForGetFunctions(GL_GREEN_SCALE, AP_DEPRECATION_PIXEL_TRANSFER);
    addEnumDeprecatedValueForGetFunctions(GL_GREEN_BIAS, AP_DEPRECATION_PIXEL_TRANSFER);
    addEnumDeprecatedValueForGetFunctions(GL_BLUE_SCALE, AP_DEPRECATION_PIXEL_TRANSFER);
    addEnumDeprecatedValueForGetFunctions(GL_BLUE_BIAS, AP_DEPRECATION_PIXEL_TRANSFER);
    addEnumDeprecatedValueForGetFunctions(GL_ALPHA_SCALE, AP_DEPRECATION_PIXEL_TRANSFER);
    addEnumDeprecatedValueForGetFunctions(GL_ALPHA_BIAS, AP_DEPRECATION_PIXEL_TRANSFER);
    addEnumDeprecatedValueForGetFunctions(GL_DEPTH_SCALE, AP_DEPRECATION_PIXEL_TRANSFER);
    addEnumDeprecatedValueForGetFunctions(GL_DEPTH_BIAS, AP_DEPRECATION_PIXEL_TRANSFER);
}


// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::addEnumDeprecatedValue
// Description: Add an enumeration deprecated argument value
// Arguments: GLenum pname
//            apFunctionDeprecationStatus assosiatedDeprecationStatus
//            const gtString& varName
//            int variableIndex - the variable index (within the function)
//            apMonitoredFunctionId functionId
// Return Val: void
// Author:      Sigal Algranaty
// Date:        19/3/2009
// ---------------------------------------------------------------------------
void gsDeprecationAnalyzer::addEnumDeprecatedValue(GLenum pname, apFunctionDeprecationStatus assosiatedDeprecationStatus, const gtString& varName, int variableIndex, apMonitoredFunctionId functionId)
{
    // Get/Create deprecation for the function:
    gsFunctionDeprecations* pStateVarFunction = _monitoredFunctionsDeprecations[functionId];

    if (pStateVarFunction == NULL)
    {
        pStateVarFunction = new gsFunctionDeprecations;

        _monitoredFunctionsDeprecations[functionId] = pStateVarFunction;
    }

    // Create an argument value deprecation condition for glClampColor and glFrontFace:
    gsArgumentValueEqualDeprecationCondition* pArgValueCondition = new gsArgumentValueEqualDeprecationCondition(variableIndex, varName, assosiatedDeprecationStatus);


    // Create an enum parameter for the state variable value:
    apGLenumParameter* pDeprecatedArgValue = new apGLenumParameter;

    pDeprecatedArgValue->readValueFromPointer(&pname);

    // Add the invalid argument value to the function illegal values:
    pArgValueCondition->addInvalidArgumentValue(pDeprecatedArgValue);

    pStateVarFunction->push_back(pArgValueCondition);
}

// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::addEnumDeprecatedValueForGetFunctions
// Description: Add a pname as deprecated argument for glGet[Integer|Boolean|Float|Double]v,
//              since any parameter passable to one of the is passable to all the others
//              and the deprecation is the same.
// Author:      Uri Shomroni
// Date:        4/1/2010
// ---------------------------------------------------------------------------
void gsDeprecationAnalyzer::addEnumDeprecatedValueForGetFunctions(GLenum pname, apFunctionDeprecationStatus assosiatedDeprecationStatus)
{
    static const gtString pnameArgName = GS_STR_DeprecationPNameArgName;

    // The deprecated parameter in all these functions is parameter 1, called pname:
    addEnumDeprecatedValue(pname, assosiatedDeprecationStatus, pnameArgName, 1, ap_glGetIntegerv);
    addEnumDeprecatedValue(pname, assosiatedDeprecationStatus, pnameArgName, 1, ap_glGetBooleanv);
    addEnumDeprecatedValue(pname, assosiatedDeprecationStatus, pnameArgName, 1, ap_glGetFloatv);
    addEnumDeprecatedValue(pname, assosiatedDeprecationStatus, pnameArgName, 1, ap_glGetDoublev);
}

// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::addEnumDeprecatedValueForEnableFunctions
// Description: Adds cap as deprecated argument for glEnable, glDisable and glIsEnabled
//              since any parameter passable to one of the is passable to all the others
//              and the deprecation is the same.
// Author:      Uri Shomroni
// Date:        4/1/2010
// ---------------------------------------------------------------------------
void gsDeprecationAnalyzer::addEnumDeprecatedValueForEnableFunctions(GLenum cap, apFunctionDeprecationStatus assosiatedDeprecationStatus)
{
    static const gtString capArgName = GS_STR_DeprecationCapArgName;

    // The deprecated parameter in all these functions is parameter 1, called cap:
    addEnumDeprecatedValue(cap, assosiatedDeprecationStatus, capArgName, 1, ap_glEnable);
    addEnumDeprecatedValue(cap, assosiatedDeprecationStatus, capArgName, 1, ap_glDisable);
    addEnumDeprecatedValue(cap, assosiatedDeprecationStatus, capArgName, 1, ap_glIsEnabled);

    // Any parameter passable to glIsEnabled is passable to glGetBooleanv, and thus, to all get functions:
    addEnumDeprecatedValueForGetFunctions(cap, assosiatedDeprecationStatus);
}

// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::addEnumDeprecatedValueForPNameArgOfTexEnvFunctions
// Description: Adds pname as deprecated argument for gl[Get]TexEnv*'s second parameter,
//              pname (note that "target" values are rarer, and should be done manually.
// Author:      Uri Shomroni
// Date:        4/1/2010
// ---------------------------------------------------------------------------
void gsDeprecationAnalyzer::addEnumDeprecatedValueForPNameArgOfTexEnvFunctions(GLenum pname, apFunctionDeprecationStatus assosiatedDeprecationStatus)
{
    static const gtString pnameArgName = GS_STR_DeprecationPNameArgName;

    addEnumDeprecatedValue(pname, assosiatedDeprecationStatus, pnameArgName, 2, ap_glTexEnvf);
    addEnumDeprecatedValue(pname, assosiatedDeprecationStatus, pnameArgName, 2, ap_glTexEnvfv);
    addEnumDeprecatedValue(pname, assosiatedDeprecationStatus, pnameArgName, 2, ap_glTexEnvi);
    addEnumDeprecatedValue(pname, assosiatedDeprecationStatus, pnameArgName, 2, ap_glTexEnviv);
    addEnumDeprecatedValue(pname, assosiatedDeprecationStatus, pnameArgName, 2, ap_glGetTexEnvfv);
    addEnumDeprecatedValue(pname, assosiatedDeprecationStatus, pnameArgName, 2, ap_glGetTexEnviv);
}

// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::addEnumDeprecatedValue
// Description: Add an enumeration deprecated argument value
// Arguments: const gtVector<GLenum>& pnames
//            apFunctionDeprecationStatus assosiatedDeprecationStatus
//            const gtString& varName
//            int variableIndex - the variable index (within the function)
//            apMonitoredFunctionId functionId
// Return Val: void
// Author:      Sigal Algranaty
// Date:        19/3/2009
// ---------------------------------------------------------------------------
void gsDeprecationAnalyzer::addEnumDeprecatedValues(const gtVector<GLenum>& pnames, apFunctionDeprecationStatus assosiatedDeprecationStatus, const gtString& varName, int variableIndex, apMonitoredFunctionId functionId)
{
    // Get/Create deprecation for the function:
    gsFunctionDeprecations* pStateVarFunction = _monitoredFunctionsDeprecations[functionId];

    if (pStateVarFunction == NULL)
    {
        pStateVarFunction = new gsFunctionDeprecations;

        _monitoredFunctionsDeprecations[functionId] = pStateVarFunction;
    }

    // Create an argument value deprecation condition for glClampColor and glFrontFace:
    gsArgumentValueEqualDeprecationCondition* pArgValueCondition = new gsArgumentValueEqualDeprecationCondition(variableIndex, varName, assosiatedDeprecationStatus);


    for (int i = 0; i < (int)pnames.size(); i++)
    {
        // Get the current pname:
        GLenum pname = pnames[i];

        // Create an enum parameter for the state variable value:
        apGLenumParameter* pDeprecatedArgValue = new apGLenumParameter;

        pDeprecatedArgValue->readValueFromPointer(&pname);

        // Add the invalid argument value to the function illegal values:
        pArgValueCondition->addInvalidArgumentValue(pDeprecatedArgValue);
    }

    pStateVarFunction->push_back(pArgValueCondition);
}

// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::addGLClearBitfieldDeprecatedValue
// Description: Add a deprecation for function call with deprecation bitfield value
// Arguments: GLbitfield bitfield
//            apFunctionDeprecationStatus assosiatedDeprecationStatus
//            const gtString& varName
//            int variableIndex
//            apMonitoredFunctionId functionId
// Return Val: void
// Author:      Sigal Algranaty
// Date:        24/3/2009
// ---------------------------------------------------------------------------
void gsDeprecationAnalyzer::addGLClearBitfieldDeprecatedValue(GLbitfield bitfield, apFunctionDeprecationStatus assosiatedDeprecationStatus, const gtString& varName, int variableIndex, apMonitoredFunctionId functionId)
{
    // Get/Create deprecation for the function:
    gsFunctionDeprecations* pStateVarFunction = _monitoredFunctionsDeprecations[functionId];

    if (pStateVarFunction == NULL)
    {
        pStateVarFunction = new gsFunctionDeprecations;

        _monitoredFunctionsDeprecations[functionId] = pStateVarFunction;
    }

    // Create an argument value deprecation condition for glClampColor and glFrontFace:
    gsArgumentValueEqualDeprecationCondition* pArgValueCondition = new gsArgumentValueEqualDeprecationCondition(variableIndex, varName, assosiatedDeprecationStatus);


    // Create an enum parameter for the state variable value:
    apGLclearBitfieldParameter* pDeprecatedArgValue = new apGLclearBitfieldParameter;

    pDeprecatedArgValue->readValueFromPointer(&bitfield);

    // Add the invalid argument value to the function illegal values:
    pArgValueCondition->addInvalidArgumentValue(pDeprecatedArgValue);

    pStateVarFunction->push_back(pArgValueCondition);
}

// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::addFloatComparisonDeprecatedValue
// Description: Add float argument value comparison deprecation
// Arguments: GLfloat valueToCompare
//            gsArgValueComparisonType compareType - state if value should be equal/greater than/ smaller than
//            apFunctionDeprecationStatus assosiatedDeprecationStatus
//            const gtString& varName
//            int variableIndex
//            apMonitoredFunctionId functionId
// Return Val: void
// Author:      Sigal Algranaty
// Date:        23/3/2009
// ---------------------------------------------------------------------------
void gsDeprecationAnalyzer::addFloatComparisonDeprecatedValue(GLfloat valueToCompare, gsArgValueComparisonType compareType, apFunctionDeprecationStatus assosiatedDeprecationStatus, const gtString& varName, int variableIndex, apMonitoredFunctionId functionId)
{
    // Get/Create deprecation for the function:
    gsFunctionDeprecations* pStateVarFunction = _monitoredFunctionsDeprecations[functionId];

    if (pStateVarFunction == NULL)
    {
        pStateVarFunction = new gsFunctionDeprecations;

        _monitoredFunctionsDeprecations[functionId] = pStateVarFunction;
    }

    // Create an argument value deprecation condition for glClampColor and glFrontFace:
    gsArgumentValueComparisonDeprecationCondition* pArgValueCondition = new gsArgumentValueComparisonDeprecationCondition(variableIndex, varName, valueToCompare, compareType, assosiatedDeprecationStatus);


    pStateVarFunction->push_back(pArgValueCondition);
}




// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::addPixelFormatDeprecatedValues
// Description: Add a deprecation for function call with deprecation bitfield value
// Arguments: const gtVector<int>& pixelFormatValuesVector - the pixel format deprecated values
//            apFunctionDeprecationStatus assosiatedDeprecationStatus
//            const gtString& varName
//            int variableIndex
//            apMonitoredFunctionId functionId
// Return Val: void
// Author:      Sigal Algranaty
// Date:        24/3/2009
// ---------------------------------------------------------------------------
void gsDeprecationAnalyzer::addPixelFormatDeprecatedValues(const gtVector<int>& pixelFormatValuesVector, apFunctionDeprecationStatus assosiatedDeprecationStatus, const gtString& varName, int variableIndex, apMonitoredFunctionId functionId)
{
    // Get/Create deprecation for the function:
    gsFunctionDeprecations* pStateVarFunction = _monitoredFunctionsDeprecations[functionId];

    if (pStateVarFunction == NULL)
    {
        pStateVarFunction = new gsFunctionDeprecations;

        _monitoredFunctionsDeprecations[functionId] = pStateVarFunction;
    }

    // Create an argument value deprecation condition for glClampColor and glFrontFace:
    gsArgumentValueEqualDeprecationCondition* pArgValueCondition = new gsArgumentValueEqualDeprecationCondition(variableIndex, varName, assosiatedDeprecationStatus);



    // Iterate the deprecated values, and add an invalid argument for each of them:
    for (int i = 0; i < (int)pixelFormatValuesVector.size(); i++)
    {
        // Get the current deprecated value:
        int currentPixelFormatValue = pixelFormatValuesVector[i];

        // Create an enum parameter for the state variable value:
        apGLPixelInternalFormatParameter* pIllegalPixelFormatValue = new apGLPixelInternalFormatParameter;

        pIllegalPixelFormatValue->setValueFromInt(currentPixelFormatValue);

        // Add the invalid argument value to the function illegal values:
        pArgValueCondition->addInvalidArgumentValue(pIllegalPixelFormatValue);
    }

    pStateVarFunction->push_back(pArgValueCondition);
}


// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::getDeprecatedInternalFormats
// Description: Gets all deprecated pixel internal format values
// Arguments: gtVector<int>& pixelFormatIllegalValues
// Return Val:
// Author:      Sigal Algranaty
// Date:        25/3/2009
// ---------------------------------------------------------------------------
void gsDeprecationAnalyzer::getDeprecatedInternalFormats(gtVector<int>& pixelFormatIllegalValues)
{
    pixelFormatIllegalValues.push_back(1);
    pixelFormatIllegalValues.push_back(2);
    pixelFormatIllegalValues.push_back(3);
    pixelFormatIllegalValues.push_back(4);
    pixelFormatIllegalValues.push_back(GL_ALPHA4);
    pixelFormatIllegalValues.push_back(GL_ALPHA8);
    pixelFormatIllegalValues.push_back(GL_ALPHA12);
    pixelFormatIllegalValues.push_back(GL_ALPHA16);
    pixelFormatIllegalValues.push_back(GL_LUMINANCE4);
    pixelFormatIllegalValues.push_back(GL_LUMINANCE8);
    pixelFormatIllegalValues.push_back(GL_LUMINANCE12);
    pixelFormatIllegalValues.push_back(GL_LUMINANCE16);
    pixelFormatIllegalValues.push_back(GL_LUMINANCE4_ALPHA4);
    pixelFormatIllegalValues.push_back(GL_LUMINANCE6_ALPHA2);
    pixelFormatIllegalValues.push_back(GL_LUMINANCE8_ALPHA8);
    pixelFormatIllegalValues.push_back(GL_LUMINANCE12_ALPHA4);
    pixelFormatIllegalValues.push_back(GL_LUMINANCE12_ALPHA12);
    pixelFormatIllegalValues.push_back(GL_LUMINANCE16_ALPHA16);
    pixelFormatIllegalValues.push_back(GL_INTENSITY);
    pixelFormatIllegalValues.push_back(GL_INTENSITY4);
    pixelFormatIllegalValues.push_back(GL_INTENSITY8);
    pixelFormatIllegalValues.push_back(GL_INTENSITY12);
    pixelFormatIllegalValues.push_back(GL_INTENSITY16);
    pixelFormatIllegalValues.push_back(GL_COMPRESSED_ALPHA);
    pixelFormatIllegalValues.push_back(GL_COMPRESSED_LUMINANCE);
    pixelFormatIllegalValues.push_back(GL_COMPRESSED_LUMINANCE_ALPHA);
    pixelFormatIllegalValues.push_back(GL_COMPRESSED_INTENSITY);
}



// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::getDeprecatedExternalFormats
// Description: Gets all deprecated pixel external format values
// Arguments: gtVector<int>& pixelFormatIllegalValues
// Return Val:
// Author:      Sigal Algranaty
// Date:        25/3/2009
// ---------------------------------------------------------------------------
void gsDeprecationAnalyzer::getDeprecatedExternalFormats(gtVector<GLenum>& pixelFormatIllegalValues)
{
    pixelFormatIllegalValues.push_back(GL_ALPHA4);
    pixelFormatIllegalValues.push_back(GL_ALPHA8);
    pixelFormatIllegalValues.push_back(GL_ALPHA12);
    pixelFormatIllegalValues.push_back(GL_ALPHA16);
    pixelFormatIllegalValues.push_back(GL_LUMINANCE4);
    pixelFormatIllegalValues.push_back(GL_LUMINANCE8);
    pixelFormatIllegalValues.push_back(GL_LUMINANCE12);
    pixelFormatIllegalValues.push_back(GL_LUMINANCE16);
    pixelFormatIllegalValues.push_back(GL_LUMINANCE4_ALPHA4);
    pixelFormatIllegalValues.push_back(GL_LUMINANCE6_ALPHA2);
    pixelFormatIllegalValues.push_back(GL_LUMINANCE8_ALPHA8);
    pixelFormatIllegalValues.push_back(GL_LUMINANCE12_ALPHA4);
    pixelFormatIllegalValues.push_back(GL_LUMINANCE12_ALPHA12);
    pixelFormatIllegalValues.push_back(GL_LUMINANCE16_ALPHA16);
    pixelFormatIllegalValues.push_back(GL_INTENSITY);
    pixelFormatIllegalValues.push_back(GL_INTENSITY4);
    pixelFormatIllegalValues.push_back(GL_INTENSITY8);
    pixelFormatIllegalValues.push_back(GL_INTENSITY12);
    pixelFormatIllegalValues.push_back(GL_INTENSITY16);
    pixelFormatIllegalValues.push_back(GL_COMPRESSED_ALPHA);
    pixelFormatIllegalValues.push_back(GL_COMPRESSED_LUMINANCE);
    pixelFormatIllegalValues.push_back(GL_COMPRESSED_LUMINANCE_ALPHA);
    pixelFormatIllegalValues.push_back(GL_COMPRESSED_INTENSITY);

}

// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::initializeStateVariableDeprecations
// Description: Use the function deprecation data to initialize the state variable
//              deprecation data
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        4/1/2010
// ---------------------------------------------------------------------------
bool gsDeprecationAnalyzer::initializeStateVariableDeprecations()
{
    bool retVal = true;

    apOpenGLStateVariablesManager& theStateVariablesManager = apOpenGLStateVariablesManager::instance();
    int amountOfSupportedStateVars = theStateVariablesManager.amountOfStateVariables();

    for (int i = 0; i < amountOfSupportedStateVars; i++)
    {
        GLenum pname = theStateVariablesManager.stateVariableOpenGLParamEnum(i);
        apMonitoredFunctionId getFunctionId = theStateVariablesManager.stateVariableGetFunctionId(i);
        apAPIVersion removedVersion = AP_GL_VERSION_NONE;

        // See if this state variable is deprecated (i.e. is it a deprecated argument value
        // for its get function or its get function is deprecated):
        bool isDeprecated = isStateVariableDeprecated(pname, getFunctionId, removedVersion);

        if (isDeprecated)
        {
            // Do not assert this, as features deprecated in the latest OpenGL version would be
            // deprecated at AP_GL_VERSION_X_X and removed at AP_GL_VERSION_NONE
            if (removedVersion != AP_GL_VERSION_NONE)
            {
                // Update the state variables manager:
                theStateVariablesManager.setStateVariablesRemovedAtVersion(i, removedVersion);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::isStateVariableDeprecated
// Description: Checks if a deprecation exists for a certain get function with
//              a certain parameter, effectively a deprecated state variable.
// Author:      Uri Shomroni
// Date:        4/1/2010
// ---------------------------------------------------------------------------
bool gsDeprecationAnalyzer::isStateVariableDeprecated(GLenum pname, apMonitoredFunctionId getFunctionId, apAPIVersion& removedVersion)
{
    bool retVal = false;

    // See if the get function is deprecated (e.g. glGetLightfv):
    bool isFunctionDeprecated = ((apMonitoredFunctionsManager::instance().monitoredFunctionType(getFunctionId) & AP_DEPRECATED_FUNC) != 0);

    if (isFunctionDeprecated)
    {
        apFunctionDeprecationStatus deprecationStatus = AP_DEPRECATION_FULL;
        bool rcStat = apFunctionDeprecation::getDeprecationStatusByFunctionId(getFunctionId, deprecationStatus);
        GT_ASSERT(rcStat);

        apAPIVersion deprecatedVersion;
        bool rcVers = apFunctionDeprecation::getDeprecationAndRemoveVersionsByStatus(deprecationStatus, deprecatedVersion, removedVersion);
        GT_ASSERT(rcVers);

        retVal = true;
    }
    else
    {
        // Get the function deprecations vector for this function id:
        gsFunctionDeprecations* pFunctionDeprecations = _monitoredFunctionsDeprecations[getFunctionId];

        if (pFunctionDeprecations != NULL)
        {
            int numberOfDeprecations = (int)pFunctionDeprecations->size();

            if (numberOfDeprecations > 0)
            {
                apFunctionDeprecationStatus deprecationStatus = AP_DEPRECATION_NONE;
                apGLenumParameter pnameAsParameter(pname);

                // Get the deprecations for this function:
                for (int i = 0 ; i < numberOfDeprecations; i++)
                {
                    // Check if this is a "parameter value" deprecation:
                    gsDeprecationCondition* pCurrentCondition = (*pFunctionDeprecations)[i];

                    if (pCurrentCondition != NULL)
                    {
                        // If this is an "invalid argument value" condition:
                        if (pCurrentCondition->isArgumentValueDeprecationCondition())
                        {
                            // Cast to the appropriate type:
                            gsArgumentValueEqualDeprecationCondition* pCurrentArgumentCondition = (gsArgumentValueEqualDeprecationCondition*)pCurrentCondition;
                            int numberOfDisallowedValues = pCurrentArgumentCondition->numberOfInvalidArgumentValues();

                            for (int j = 0; j < numberOfDisallowedValues; j++)
                            {
                                // Get the current disallowed value:
                                const apParameter* pCurrentParameter = pCurrentArgumentCondition->invalidArgumentValue(j);

                                if (pCurrentParameter != NULL)
                                {
                                    // If the values are equal:
                                    if (pCurrentParameter->compareToOther(pnameAsParameter))
                                    {
                                        // Get the deprecation status:
                                        deprecationStatus = pCurrentArgumentCondition->functionDeprecationStatus();
                                        break;
                                    }
                                }
                            }

                            if (deprecationStatus != AP_DEPRECATION_NONE)
                            {
                                // We found the deprecation, no need to continue:
                                break;
                            }
                        }
                    }
                }

                if (deprecationStatus != AP_DEPRECATION_NONE)
                {
                    retVal = true;

                    apAPIVersion deprecatedVersion;
                    bool rcDepr = apFunctionDeprecation::getDeprecationAndRemoveVersionsByStatus(deprecationStatus, deprecatedVersion, removedVersion);
                    GT_ASSERT(rcDepr);
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::initializeOpenGL30Deprecations
// Description: Initializes OpenGL 3.0 deprecated features
// Return Val: void
// Author:      Sigal Algranaty
// Date:        14/10/2009
// ---------------------------------------------------------------------------
void gsDeprecationAnalyzer::initializeOpenGL30Deprecations()
{
    // Initialize the invalid argument deprecations:
    initializeInvalidArgumentDeprecationsForOpenGL30();

    // Initialize the ungenerated objects deprecations:
    initializeUngeneratedObjectsDeprecationsForOpenGL30();
}

// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::initializeOpenGL32Deprecations
// Description: Initializes OpenGL 3.2 deprecated features
// Return Val: void
// Author:      Sigal Algranaty
// Date:        14/10/2009
// ---------------------------------------------------------------------------
void gsDeprecationAnalyzer::initializeOpenGL32Deprecations()
{
    addEnumDeprecatedValueForGetFunctions(GL_MAX_VARYING_COMPONENTS, AP_DEPRECATION_MAX_VARYING);
    addEnumDeprecatedValueForGetFunctions(GL_MAX_VARYING_FLOATS, AP_DEPRECATION_MAX_VARYING);
}

// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::initializeOpenGL42Deprecations
// Description: Initializes OpenGL 4.2 deprecated features
// Author:      Uri Shomroni
// Date:        11/9/2013
// ---------------------------------------------------------------------------
void gsDeprecationAnalyzer::initializeOpenGL42Deprecations()
{
    addEnumDeprecatedValueForGetFunctions(GL_NUM_COMPRESSED_TEXTURE_FORMATS, AP_DEPRECATION_COMPRESSED_TEXTURE_FORMATS);
    addEnumDeprecatedValueForGetFunctions(GL_COMPRESSED_TEXTURE_FORMATS, AP_DEPRECATION_COMPRESSED_TEXTURE_FORMATS);
}

// ---------------------------------------------------------------------------
// Name:        gsDeprecationAnalyzer::initializeOpenGL43Deprecations
// Description: Initializes OpenGL 4.3 deprecated features
// Author:      Uri Shomrni
// Date:        11/9/2013
// ---------------------------------------------------------------------------
void gsDeprecationAnalyzer::initializeOpenGL43Deprecations()
{
    static const gtString pnameArgName = GS_STR_DeprecationPNameArgName;
    addEnumDeprecatedValue(GL_PACK_LSB_FIRST, AP_DEPRECATION_LSB_FIRST_PIXEL_PACKING, pnameArgName, 1, ap_glPixelStoref);
    addEnumDeprecatedValue(GL_PACK_LSB_FIRST, AP_DEPRECATION_LSB_FIRST_PIXEL_PACKING, pnameArgName, 1, ap_glPixelStorei);
    addEnumDeprecatedValueForGetFunctions(GL_PACK_LSB_FIRST, AP_DEPRECATION_LSB_FIRST_PIXEL_PACKING);
    addEnumDeprecatedValue(GL_UNPACK_LSB_FIRST, AP_DEPRECATION_LSB_FIRST_PIXEL_PACKING, pnameArgName, 1, ap_glPixelStoref);
    addEnumDeprecatedValue(GL_UNPACK_LSB_FIRST, AP_DEPRECATION_LSB_FIRST_PIXEL_PACKING, pnameArgName, 1, ap_glPixelStorei);
    addEnumDeprecatedValueForGetFunctions(GL_UNPACK_LSB_FIRST, AP_DEPRECATION_LSB_FIRST_PIXEL_PACKING);
}

