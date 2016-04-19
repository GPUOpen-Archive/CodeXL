//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsStateVariablesSnapshot.cpp
///
//==================================================================================

//------------------------------ gsStateVariablesSnapshot.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/apParameters.h>
#include <AMDTAPIClasses/Include/apOpenGLStateVariablesManager.h>
#include <AMDTAPIClasses/Include/apOpenGLAPIType.h>

// Local:
#include <src/gsStringConstants.h>
#include <src/gsWrappersCommon.h>
#include <src/gsExtensionsManager.h>
#include <src/gsStateVariableReader.h>
#include <src/gsRenderContextMonitor.h>
#include <src/gsStateVariablesSnapshot.h>
#include <src/gsOpenGLMonitor.h>


// ---------------------------------------------------------------------------
// Name:        gsStateVariablesSnapshot::gsStateVariablesSnapshot
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        17/7/2004
// ---------------------------------------------------------------------------
gsStateVariablesSnapshot::gsStateVariablesSnapshot()
{
}


// ---------------------------------------------------------------------------
// Name:        gsStateVariablesSnapshot::~gsStateVariablesSnapshot
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        17/7/2004
// ---------------------------------------------------------------------------
gsStateVariablesSnapshot::~gsStateVariablesSnapshot()
{
}


// ---------------------------------------------------------------------------
// Name:        gsStateVariablesSnapshot::onFirstTimeContextMadeCurrent
// Description: Is called on the first time in which my context is made the
//              current context.
// Arguments: myRenderContextMtr - The render context who's state variables
//                                 I log.
// Author:      Yaki Tebeka
// Date:        22/2/2006
// ---------------------------------------------------------------------------
void gsStateVariablesSnapshot::onFirstTimeContextMadeCurrent(const gsRenderContextMonitor& myRenderContextMtr)
{
    // Update the OpenGL standard extensions state variables support:
#if (defined(_GR_IPHONE_BUILD) || defined(_GR_OPENGLES_COMMON))
    updateOGLESStandardStateVariablesSupport(myRenderContextMtr);
#else
    updateOGLStandardStateVariablesSupport(myRenderContextMtr);
#endif

    // Update extensions state variables support:
    updateExtensionsStateVariablesSupport(myRenderContextMtr);

    // Set my render context monitor handle:
    _pMyRenderContextMtr = &myRenderContextMtr;
}

// ---------------------------------------------------------------------------
// Name:        gsStateVariablesSnapshot::updateAllStateVariables
// Description: Updates the state variable values from the current render
//              context. This function updates all state variables (not only the filtered
//              ones).
// Author:      Yaki Tebeka
// Date:        17/7/2004
// ---------------------------------------------------------------------------
bool gsStateVariablesSnapshot::updateAllStateVariables()
{
    bool retVal = true;

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateStateVariablesDataStarted, OS_DEBUG_LOG_DEBUG);

    // Get the instance of the state variables manager:
    apOpenGLStateVariablesManager& theStateVariablesMgr = apOpenGLStateVariablesManager::instance();

    // Define the valid state variables types (as a bitwise mask):
    unsigned int validStateVariablesTypesMask = 0;

    // Get the valid state variable types mask from my render context monitor:
    validStateVariablesTypesMask = getValidStateVariableTypesMask(true, _pMyRenderContextMtr);

    // Iterate on the OpenGL state variables:
    for (int i = 0; i < apOpenGLStateVariablesAmount; i++)
    {
        // Will get true iff we managed to get the current state variable value:
        bool gotCurStateVarValue = false;

        // If the current state variable is NOT supported by my monitored context:
        if (_isStateVariableSupported[i] != true)
        {
            // Check if this parameter is already allocated:
            if ((_stateVariableValues[i] != NULL) && (_stateVariableValues[i]->type() == OS_TOBJ_ID_NOT_SUPPORTED_PARAMETER))
            {
                gotCurStateVarValue = true;
            }
            else
            {
                // Delete existing parameter:
                apStateVariablesSnapShot::deleteParameter(_stateVariableValues[i]);
                _stateVariableValues[i] = &apStateVariablesSnapShot::_staticNotSupportedParameter;
                gotCurStateVarValue = true;
            }
        }
        else
        {
            // Get the current state variable type:
            unsigned int curStateVarTypes = theStateVariablesMgr.stateVariableGlobalType(i);

            // If this is a valid state variable (for this spy type):
            bool isStateVariableTypeValid = ((curStateVarTypes & validStateVariablesTypesMask) != 0);

            if (isStateVariableTypeValid)
            {
                // Get the current variable value:
                gsStateVariableReader stateVarValueReader(i, _pMyRenderContextMtr);
                gotCurStateVarValue = stateVarValueReader.getStateVariableValue(_stateVariableValues[i]);
            }
        }

        if (gotCurStateVarValue == true)
        {
            _isStateVariableAvailable[i] = true;
        }
        else
        {
            // We didn't manage to get the state variable value. Set the value to be apNotAvailableParameter:
            // Sigal: 16.9.08 NOTICE:
            // We do not release memory of the original parameter, since we would want to use it later, once the
            // parameter is available again. Memory allocation and release consumes alot of time, and we want to be as
            // cheap as possible
            _isStateVariableAvailable[i] = false;
        }
    }

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateStateVariablesDataEnded, OS_DEBUG_LOG_DEBUG);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStateVariablesSnapshot::getValidStateVariableTypesMask
// Description: Get the mask of state variables supported by this implementation (see apOpenGLAPIType.h)
// Arguments:   withWindowingSystem - Whether or not to add the windowing systems
//              (WGL, CGL, GLX, etc.) defined state variables
// Author:      Uri Shomroni
// Date:        9/7/2009
// ---------------------------------------------------------------------------
unsigned int gsStateVariablesSnapshot::getValidStateVariableTypesMask(bool withWindowingSystem, const gsRenderContextMonitor* pMyRenderContextMonitor)
{
    GT_UNREFERENCED_PARAMETER(pMyRenderContextMonitor);

    unsigned int validStateVariablesTypesMask = 0;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // On Windows (OpenGL):
#ifdef _AMDT_OPENGLSERVER_EXPORTS
    validStateVariablesTypesMask |= AP_OPENGL_STATE_VAR;

    if (withWindowingSystem)
    {
        validStateVariablesTypesMask |= AP_WGL_STATE_VAR;
    }

#endif

    // On Windows (OpenGL ES emulator):
#ifdef OS_OGL_ES_IMPLEMENTATION_DLL_BUILD
    validStateVariablesTypesMask |= AP_OPENGL_ES_1_STATE_VAR;
#endif

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    // Resolve the compiler warning for the Linux variant
    (void)(withWindowingSystem);
    (void)(pMyRenderContextMonitor);
    // On Linux:
#ifdef _AMDT_OPENGLSERVER_EXPORTS
    validStateVariablesTypesMask |= AP_OPENGL_STATE_VAR;
    /* // Uri, 09/07/09: There are no GLX state variables currently defined:
    if (withWindowingSystem)
    {
        validStateVariablesTypesMask |= AP_GLX_STATE_VAR;
    }
    */
#endif

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
#ifdef _GR_OPENGLES_IPHONE
    // On the iPhone:
    // There are some OpenGL ES state variables that are supported by OpenGL ES 1.1 and not by 2.0 and vice-versa:
    int oglesMajorVersion = 0;
    int oglesMinorVersion = 0;
    GT_IF_WITH_ASSERT(pMyRenderContextMonitor != NULL)
    {
        pMyRenderContextMonitor->getOpenGLVersion(oglesMajorVersion, oglesMinorVersion);
    }

    if (oglesMajorVersion == 1)
    {
        validStateVariablesTypesMask |= AP_OPENGL_ES_1_STATE_VAR;
    }
    else if (oglesMajorVersion == 2)
    {
        validStateVariablesTypesMask |= AP_OPENGL_ES_2_STATE_VAR;
    }
    else
    {
        // Unsupported OpenGL ES version:
        GT_ASSERT(false);
    }

#endif

#ifdef _AMDT_OPENGLSERVER_EXPORTS
    // On Mac:
    validStateVariablesTypesMask |= AP_OPENGL_STATE_VAR;

    if (withWindowingSystem)
    {
        validStateVariablesTypesMask |= AP_CGL_STATE_VAR;
    }

#endif
#endif

    return validStateVariablesTypesMask;
}

// ---------------------------------------------------------------------------
// Name:        gsStateVariablesSnapshot::updateContextDataSnapshot
// Description: Update context data snapshot. This function checks if the snapshot is currently filtered,
//              and if it is, it call a function that updates only the filtered state variables,
//              otherwise, it call a function that updates all state variables.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/9/2008
// ---------------------------------------------------------------------------
bool gsStateVariablesSnapshot::updateContextDataSnapshot()
{
    bool retVal = false;

    // Check if this snapshot contain state variables filtering:
    if (_pStateVariablesFilteredIdsVector != NULL)
    {
        retVal = updateOnlyFilteredStateVariables();
    }
    else
    {
        // No filtering - update all variables:
        retVal = updateAllStateVariables();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStateVariablesSnapshot::updateOnlyFilteredStateVariables
// Description: Updates only filtered state variable id
// Return Val: void
// Author:      Sigal Algranaty
// Date:        8/9/2008
// ---------------------------------------------------------------------------
bool gsStateVariablesSnapshot::updateOnlyFilteredStateVariables()
{
    bool retVal = true;

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateStateVariablesDataStarted, OS_DEBUG_LOG_DEBUG);

    // Get the instance of the state variables manager:
    apOpenGLStateVariablesManager& theStateVariablesMgr = apOpenGLStateVariablesManager::instance();

    // Get the valid state variables types (as a bitwise mask):
    unsigned int validStateVariablesTypesMask = 0;

    // Get the valid state variable types mask from my render context monitor:
    validStateVariablesTypesMask = getValidStateVariableTypesMask(false, _pMyRenderContextMtr);

    GT_IF_WITH_ASSERT(_pStateVariablesFilteredIdsVector != NULL)
    {
        // Get the filtered state variables amount:
        size_t filteredStateVariablesAmount = _pStateVariablesFilteredIdsVector->size();

        // Iterate on the filtered state variables:
        for (size_t j = 0; j < filteredStateVariablesAmount; j++)
        {
            // Get the current state variable id:
            int variableId = (*_pStateVariablesFilteredIdsVector)[j];

            // Will get true iff we managed to get the current state variable value:
            bool gotCurStateVarValue = false;

            // Check if the state variable is supported:
            if (_isStateVariableSupported[variableId] == true)
            {
                // Get the current state variable type:
                unsigned int curStateVarTypes = theStateVariablesMgr.stateVariableGlobalType(variableId);

                // If this is a valid state variable (for this spy type):
                if (curStateVarTypes & validStateVariablesTypesMask)
                {
                    // Get the current variable value:
                    gsStateVariableReader stateVarValueReader(variableId, _pMyRenderContextMtr);
                    gotCurStateVarValue = stateVarValueReader.getStateVariableValue(_stateVariableValues[variableId]);
                }

                if (gotCurStateVarValue == true)
                {
                    _isStateVariableAvailable[variableId] = true;
                }
                else
                {
                    // We didn't manage to get the state variable value. Set the value to be apNotAvailableParameter:
                    // Sigal: 16.9.08 NOTICE:
                    // We do not release memory of the original parameter, since we would want to use it later, once the
                    // parameter is available again. Memory allocation and release consumes alot of time, and we want to be as
                    // cheap as possible
                    _isStateVariableAvailable[variableId] = false;
                }
            }
        }
    }

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateStateVariablesFilteredDataEnded, OS_DEBUG_LOG_DEBUG);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsStateVariablesSnapshot::updateOGLStandardStateVariablesSupport
// Description: Updates the standard OpenGL state variables support.
// Arguments: myRenderContextMtr - The render context whose state variables
//                                 I log.
// Author:      Yaki Tebeka
// Date:        22/2/2006
// ---------------------------------------------------------------------------
void gsStateVariablesSnapshot::updateOGLStandardStateVariablesSupport(const gsRenderContextMonitor& myRenderContextMtr)
{
#if defined(_GR_OPENGLES_COMMON) || defined(_GR_OPENGLES_IPHONE)
    // We should not get here in OpenGL ES:
    GT_ASSERT(false);
#else

    // Get my monitored render context supported OpenGL version:
    int oglVerMajorNumber = 1;
    int oglVerMinorNumber = 1;
    myRenderContextMtr.getOpenGLVersion(oglVerMajorNumber, oglVerMinorNumber);

    // OpenGL 1.1 state variables:
    if (((oglVerMajorNumber == 1) && (1 <= oglVerMinorNumber)) ||
        (1 < oglVerMajorNumber))
    {
        // Mark all OpenGL 1.1 state variables as supported:
        for (int i = 0; i <= AP_LAST_OGL_1_1_STATE_VAR_INDEX; i++)
        {
            _isStateVariableSupported[i] = true;
        }
    }

    // OpenGL 1.2 state variables:
    if (((oglVerMajorNumber == 1) && (2 <= oglVerMinorNumber)) ||
        (1 < oglVerMajorNumber))
    {
        // Mark all OpenGL 1.2 state variables as supported:
        for (int i = (AP_LAST_OGL_1_1_STATE_VAR_INDEX + 1); i <= AP_LAST_OGL_1_2_STATE_VAR_INDEX; i++)
        {
            _isStateVariableSupported[i] = true;
        }
    }

    // OpenGL 1.3 state variables:
    if (((oglVerMajorNumber == 1) && (3 <= oglVerMinorNumber)) ||
        (1 < oglVerMajorNumber))
    {
        // Mark all OpenGL 1.3 state variables as supported:
        for (int i = (AP_LAST_OGL_1_2_STATE_VAR_INDEX + 1); i <= AP_LAST_OGL_1_3_STATE_VAR_INDEX; i++)
        {
            _isStateVariableSupported[i] = true;
        }
    }

    // OpenGL 1.4 state variables:
    if (((oglVerMajorNumber == 1) && (4 <= oglVerMinorNumber)) ||
        (1 < oglVerMajorNumber))
    {
        // Mark all OpenGL 1.4 state variables as supported:
        for (int i = (AP_LAST_OGL_1_3_STATE_VAR_INDEX + 1); i <= AP_LAST_OGL_1_4_STATE_VAR_INDEX; i++)
        {
            _isStateVariableSupported[i] = true;
        }
    }

    // OpenGL 1.5 state variables:
    if (((oglVerMajorNumber == 1) && (5 <= oglVerMinorNumber)) ||
        (1 < oglVerMajorNumber))
    {
        // Mark all OpenGL 1.5 state variables as supported:
        for (int i = (AP_LAST_OGL_1_4_STATE_VAR_INDEX + 1); i <= AP_LAST_OGL_1_5_STATE_VAR_INDEX; i++)
        {
            _isStateVariableSupported[i] = true;
        }
    }

    // OpenGL 2.0 state variables:
    if (2 <= oglVerMajorNumber)
    {
        // Mark all OpenGL 2.0 state variables as supported:
        for (int i = (AP_LAST_OGL_1_5_STATE_VAR_INDEX + 1); i <= AP_LAST_OGL_2_0_STATE_VAR_INDEX; i++)
        {
            _isStateVariableSupported[i] = true;
        }
    }

    if (3 <= oglVerMajorNumber)
    {
        // Mark all OpenGL 3.0 state variables as supported:
        for (int i = (AP_LAST_OGL_2_0_STATE_VAR_INDEX + 1); i <= AP_LAST_OGL_3_0_STATE_VAR_INDEX; i++)
        {
            _isStateVariableSupported[i] = true;
        }

        // The following state variables, from the GL_ARB_framebuffer_object extension, are reused by OpenGL 3.0:
        _isStateVariableSupported[apGL_FRAMEBUFFER_BINDING] = true;
        _isStateVariableSupported[apGL_RENDERBUFFER_BINDING] = true;
        _isStateVariableSupported[apGL_MAX_RENDERBUFFER_SIZE] = true;
    }

    if (4 <= oglVerMajorNumber)
    {
        // Mark all OpenGL 4.0 state variables as supported:
        for (int i = (AP_LAST_OGL_3_0_STATE_VAR_INDEX + 1); i <= AP_LAST_OGL_4_0_STATE_VAR_INDEX; i++)
        {
            _isStateVariableSupported[i] = true;
        }
    }

    if (((4 <= oglVerMajorNumber) && (1 <= oglVerMinorNumber)) ||
        (4 < oglVerMajorNumber))
    {
        // Mark all OpenGL 4.1 state variables as supported:
        for (int i = (AP_LAST_OGL_4_0_STATE_VAR_INDEX + 1); i <= AP_LAST_OGL_4_1_STATE_VAR_INDEX; i++)
        {
            _isStateVariableSupported[i] = true;
        }
    }

    if (((4 <= oglVerMajorNumber) && (2 <= oglVerMinorNumber)) ||
        (4 < oglVerMajorNumber))
    {
        // Mark all OpenGL 4.2 state variables as supported:
        for (int i = (AP_LAST_OGL_4_1_STATE_VAR_INDEX + 1); i <= AP_LAST_OGL_4_2_STATE_VAR_INDEX; i++)
        {
            _isStateVariableSupported[i] = true;
        }
    }

    if (((4 <= oglVerMajorNumber) && (3 <= oglVerMinorNumber)) ||
        (4 < oglVerMajorNumber))
    {
        // Mark all OpenGL 4.3 state variables as supported:
        for (int i = (AP_LAST_OGL_4_2_STATE_VAR_INDEX + 1); i <= AP_LAST_OGL_4_3_STATE_VAR_INDEX; i++)
        {
            _isStateVariableSupported[i] = true;
        }
    }

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // CGL state variables:
    // Mark all CGL state variables as supported:
    for (int i = AP_FIRST_CGL_STATE_VAR_INDEX; i <= AP_LAST_CGL_STATE_VAR_INDEX; i++)
    {
        _isStateVariableSupported[i] = true;
    }

#endif

#endif // !defined(_GR_OPENGLES_COMMON) || defined(_GR_OPENGLES_IPHONE)
}

// ---------------------------------------------------------------------------
// Name:        gsStateVariablesSnapshot::updateOGLESStandardStateVariablesSupport
// Description: Updates the standard OpenGL ES state variables support (this is
//              later further filtered by using the AP_OPENGL_ES_[1|2]_STATE_VAR bits).
//              This is done to save time when checking the validity of each state variable.
// Author:      Uri Shomroni
// Date:        27/5/2009
// ---------------------------------------------------------------------------
void gsStateVariablesSnapshot::updateOGLESStandardStateVariablesSupport(const gsRenderContextMonitor& myRenderContextMtr)
{
#if !(defined(_GR_OPENGLES_COMMON) || defined(_GR_OPENGLES_IPHONE))
    // We should not get here in OpenGL:
    GT_ASSERT(false);
    // Resolve the compiler warning for the Linux variant
    (void)(myRenderContextMtr);
#else

    // Get my monitored render context supported OpenGL version:
    int oglesVerMajorNumber = 1;
    int oglesVerMinorNumber = 1;
    myRenderContextMtr.getOpenGLVersion(oglesVerMajorNumber, oglesVerMinorNumber);

    // OpenGL ES 1.1 state variables:
    if (((oglesVerMajorNumber == 1) && (1 <= oglesVerMinorNumber)) ||
        (1 < oglesVerMajorNumber))
    {
        // Mark all OpenGL 1.1 state variables as supported:
        for (int i = 0; i <= AP_LAST_OGLES_1_1_STATE_VAR_INDEX; i++)
        {
            _isStateVariableSupported[i] = true;
        }
    }

    // OpenGL ES 2.0 state variables:
    if (oglesVerMajorNumber >= 2)
    {
        for (int i = AP_LAST_OGLES_1_1_STATE_VAR_INDEX + 1; i <= AP_LAST_OGLES_2_0_STATE_VAR_INDEX; i++)
        {
            _isStateVariableSupported[i] = true;
        }

        // The following state variables, from the GL_ARB_framebuffer_object extension, are reused by OpenGL ES 2.0:
        _isStateVariableSupported[apGL_FRAMEBUFFER_BINDING] = true;
        _isStateVariableSupported[apGL_RENDERBUFFER_BINDING] = true;
        _isStateVariableSupported[apGL_MAX_RENDERBUFFER_SIZE] = true;
    }

    // Also add OpenGL ES-specific and OpenGL ES extension state variables:
    for (int i = AP_FIRST_OGLES_ONLY_STATE_VAR_INDEX; i <= AP_LAST_OGLES_ONLY_STATE_VAR_INDEX; i++)
    {
        _isStateVariableSupported[i] = true;
    }

#endif // defined(_GR_OPENGLES_COMMON) || defined(_GR_OPENGLES_IPHONE)
}


// ---------------------------------------------------------------------------
// Name:        gsStateVariablesSnapshot::updateExtensionsStateVariablesSupport
// Description: Update extensions state variables support.
// Arguments: myRenderContextMtr - The render context who's state variables
//                                 I log.
// Author:      Yaki Tebeka
// Date:        22/2/2006
// ---------------------------------------------------------------------------
void gsStateVariablesSnapshot::updateExtensionsStateVariablesSupport(const gsRenderContextMonitor& myRenderContextMtr)
{
    // Get the extensions manager instance:
    gsExtensionsManager& theExtensionsMgr = gsExtensionsManager::instance();

    // Get my render context id:
    int myRenderContextId = myRenderContextMtr.spyId();

    // If this is a real context:
    if (0 < myRenderContextId)
    {
        //////////////////////////////////////////////////////////////////////////
        // GL_NV_primitive_restart
        //////////////////////////////////////////////////////////////////////////
        bool isGL_NV_primitive_restartSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_NV_primitive_restart);

        if (isGL_NV_primitive_restartSupported)
        {
            _isStateVariableSupported[apGL_PRIMITIVE_RESTART_NV] = true;
            _isStateVariableSupported[apGL_PRIMITIVE_RESTART_INDEX_NV] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_HP_occlusion_test extension
        //////////////////////////////////////////////////////////////////////////
        bool isGL_HP_occlusion_testSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_HP_occlusion_test);

        if (isGL_HP_occlusion_testSupported)
        {
            _isStateVariableSupported[apGL_OCCLUSION_TEST_HP] = true;
            _isStateVariableSupported[apGL_OCCLUSION_TEST_RESULT_HP] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_NV_occlusion_query
        //////////////////////////////////////////////////////////////////////////
        bool isGL_NV_occlusion_querySupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_NV_occlusion_query);

        if (isGL_NV_occlusion_querySupported)
        {
            _isStateVariableSupported[apGL_CURRENT_OCCLUSION_QUERY_ID_NV] = true;
            _isStateVariableSupported[apGL_PIXEL_COUNTER_BITS_NV] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_occlusion_query extension
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_occlusion_querySupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_occlusion_query);

        if (isGL_ARB_occlusion_querySupported)
        {
            _isStateVariableSupported[apGL_CURRENT_QUERY_ARB] = true;
            _isStateVariableSupported[apGL_QUERY_COUNTER_BITS_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_texture_cube_map extension
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_texture_cube_mapSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_texture_cube_map);

        if (isGL_ARB_texture_cube_mapSupported)
        {
            _isStateVariableSupported[apGL_TEXTURE_CUBE_MAP_ARB] = true;
            _isStateVariableSupported[apGL_TEXTURE_BINDING_CUBE_MAP_ARB] = true;
            _isStateVariableSupported[apGL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_texture_compression extension
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_texture_compressionSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_texture_compression);

        if (isGL_ARB_texture_compressionSupported)
        {
            _isStateVariableSupported[apGL_TEXTURE_COMPRESSION_HINT_ARB] = true;
            _isStateVariableSupported[apGL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB] = true;
            _isStateVariableSupported[apGL_COMPRESSED_TEXTURE_FORMATS_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_vertex_buffer_object extension
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_vertex_buffer_objectSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_vertex_buffer_object);

        if (isGL_ARB_vertex_buffer_objectSupported)
        {
            _isStateVariableSupported[apGL_ARRAY_BUFFER_BINDING_ARB] = true;
            _isStateVariableSupported[apGL_VERTEX_ARRAY_BUFFER_BINDING_ARB] = true;
            _isStateVariableSupported[apGL_NORMAL_ARRAY_BUFFER_BINDING_ARB] = true;
            _isStateVariableSupported[apGL_COLOR_ARRAY_BUFFER_BINDING_ARB] = true;
            _isStateVariableSupported[apGL_INDEX_ARRAY_BUFFER_BINDING_ARB] = true;
            _isStateVariableSupported[apGL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB] = true;
            _isStateVariableSupported[apGL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB] = true;
            _isStateVariableSupported[apGL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB] = true;
            _isStateVariableSupported[apGL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB] = true;
            // apGL_WEIGHT_ARRAY_BUFFER_BINDING_ARB also requires GL_ARB_vertex_blend - added there
            _isStateVariableSupported[apGL_ELEMENT_ARRAY_BUFFER_BINDING_ARB] = true;
            // apGL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB also requires GL_ARB_vertex_program -added there

            _isStateVariableSupported[apGL_ARRAY_BUFFER_GL_BUFFER_USAGE_ARB] = true;
            _isStateVariableSupported[apGL_ARRAY_BUFFER_GL_BUFFER_MAPPED_ARB] = true;
            _isStateVariableSupported[apGL_ARRAY_BUFFER_GL_BUFFER_SIZE_ARB] = true;
            _isStateVariableSupported[apGL_ARRAY_BUFFER_GL_BUFFER_ACCESS_ARB] = true;

            _isStateVariableSupported[apGL_ELEMENT_ARRAY_BUFFER_GL_BUFFER_USAGE_ARB] = true;
            _isStateVariableSupported[apGL_ELEMENT_ARRAY_BUFFER_GL_BUFFER_MAPPED_ARB] = true;
            _isStateVariableSupported[apGL_ELEMENT_ARRAY_BUFFER_GL_BUFFER_SIZE_ARB] = true;
            _isStateVariableSupported[apGL_ELEMENT_ARRAY_BUFFER_GL_BUFFER_ACCESS_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_vertex_blend extension
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_vertex_blendSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_vertex_blend);

        if (isGL_ARB_vertex_blendSupported)
        {
            if (isGL_ARB_vertex_buffer_objectSupported)
            {
                _isStateVariableSupported[apGL_WEIGHT_ARRAY_BUFFER_BINDING_ARB] = true;
            }

            _isStateVariableSupported[apGL_CURRENT_WEIGHT_ARB] = true;
            _isStateVariableSupported[apGL_WEIGHT_ARRAY_ARB] = true;
            _isStateVariableSupported[apGL_WEIGHT_ARRAY_TYPE_ARB] = true;
            _isStateVariableSupported[apGL_WEIGHT_ARRAY_SIZE_ARB] = true;
            _isStateVariableSupported[apGL_WEIGHT_ARRAY_STRIDE_ARB] = true;
            _isStateVariableSupported[apGL_WEIGHT_ARRAY_POINTER_ARB] = true;
            _isStateVariableSupported[apGL_ACTIVE_VERTEX_UNITS_ARB] = true;
            _isStateVariableSupported[apGL_VERTEX_BLEND_ARB] = true;
            _isStateVariableSupported[apGL_MAX_VERTEX_UNITS_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW0_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW1_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW2_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW3_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW4_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW5_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW6_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW7_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW8_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW9_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW10_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW11_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW12_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW13_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW14_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW15_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW16_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW17_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW18_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW19_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW20_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW21_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW22_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW23_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW24_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW25_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW26_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW27_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW28_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW29_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW30_ARB] = true;
            _isStateVariableSupported[apGL_MODELVIEW31_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_EXT_texture3D extension
        //////////////////////////////////////////////////////////////////////////
        bool isGL_EXT_texture3DSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_EXT_texture3D);

        if (isGL_EXT_texture3DSupported)
        {
            _isStateVariableSupported[apGL_TEXTURE_3D_EXT] = true;
            _isStateVariableSupported[apGL_PACK_SKIP_IMAGES_EXT] = true;
            _isStateVariableSupported[apGL_PACK_IMAGE_HEIGHT_EXT] = true;
            _isStateVariableSupported[apGL_UNPACK_SKIP_IMAGES_EXT] = true;
            _isStateVariableSupported[apGL_UNPACK_IMAGE_HEIGHT_EXT] = true;
            _isStateVariableSupported[apGL_MAX_3D_TEXTURE_SIZE_EXT] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_vertex_program
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_vertex_programSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_vertex_program);

        if (isGL_ARB_vertex_programSupported)
        {
            if (isGL_ARB_vertex_buffer_objectSupported)
            {
                // See comment in apOpenGLStateVariableId.h under GL_ARB_vertex_program
                //_isStateVariableSupported[apGL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB] = true;
            }

            _isStateVariableSupported[apGL_VERTEX_PROGRAM_ARB] = true;
            _isStateVariableSupported[apGL_VERTEX_PROGRAM_POINT_SIZE_ARB] = true;
            _isStateVariableSupported[apGL_VERTEX_PROGRAM_TWO_SIDE_ARB] = true;
            _isStateVariableSupported[apGL_PROGRAM_BINDING_ARB_vertex_program] = true;
            _isStateVariableSupported[apGL_PROGRAM_LENGTH_ARB_vertex_program] = true;
            _isStateVariableSupported[apGL_PROGRAM_FORMAT_ARB_vertex_program] = true;
            _isStateVariableSupported[apGL_PROGRAM_INSTRUCTIONS_ARB] = true;
            _isStateVariableSupported[apGL_PROGRAM_TEMPORARIES_ARB] = true;
            _isStateVariableSupported[apGL_PROGRAM_PARAMETERS_ARB] = true;
            _isStateVariableSupported[apGL_PROGRAM_ATTRIBS_ARB] = true;
            _isStateVariableSupported[apGL_PROGRAM_ADDRESS_REGISTERS_ARB] = true;
            _isStateVariableSupported[apGL_PROGRAM_NATIVE_INSTRUCTIONS_ARB] = true;
            _isStateVariableSupported[apGL_PROGRAM_NATIVE_TEMPORARIES_ARB] = true;
            _isStateVariableSupported[apGL_PROGRAM_NATIVE_PARAMETERS_ARB] = true;
            _isStateVariableSupported[apGL_PROGRAM_NATIVE_ATTRIBS_ARB] = true;
            _isStateVariableSupported[apGL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB] = true;
            _isStateVariableSupported[apGL_PROGRAM_UNDER_NATIVE_LIMITS_ARB] = true;
            _isStateVariableSupported[apGL_CURRENT_MATRIX_ARB] = true;
            _isStateVariableSupported[apGL_CURRENT_MATRIX_STACK_DEPTH_ARB] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_ENV_PARAMETERS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_MATRICES_ARB] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_INSTRUCTIONS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_TEMPORARIES_ARB] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_PARAMETERS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_ATTRIBS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_fragment_program
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_fragment_programSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_fragment_program);

        if (isGL_ARB_fragment_programSupported)
        {
            _isStateVariableSupported[apGL_FRAGMENT_PROGRAM_ARB] = true;
            _isStateVariableSupported[apGL_PROGRAM_ALU_INSTRUCTIONS_ARB] = true;
            _isStateVariableSupported[apGL_PROGRAM_TEX_INSTRUCTIONS_ARB] = true;
            _isStateVariableSupported[apGL_PROGRAM_TEX_INDIRECTIONS_ARB] = true;
            _isStateVariableSupported[apGL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB] = true;
            _isStateVariableSupported[apGL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB] = true;
            _isStateVariableSupported[apGL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_TEXTURE_COORDS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_TEXTURE_IMAGE_UNITS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_vertex_shader
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_vertex_shaderSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_vertex_shader);

        if (isGL_ARB_vertex_shaderSupported)
        {
            _isStateVariableSupported[apGL_MAX_VERTEX_ATTRIBS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_VARYING_FLOATS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_shader_objects
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_shader_objectsSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_shader_objects);

        if (isGL_ARB_shader_objectsSupported)
        {
            _isStateVariableSupported[apGL_PROGRAM_OBJECT_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_fragment_shader
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_fragment_shaderSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_fragment_shader);

        if (isGL_ARB_fragment_shaderSupported)
        {
            _isStateVariableSupported[apGL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB] = true;
            _isStateVariableSupported[apGL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_NV_vertex_program
        //////////////////////////////////////////////////////////////////////////
        bool isGL_NV_vertex_programSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_NV_vertex_program);

        if (isGL_NV_vertex_programSupported)
        {
            _isStateVariableSupported[apGL_VERTEX_PROGRAM_NV] = true;
            _isStateVariableSupported[apGL_VERTEX_PROGRAM_POINT_SIZE_NV] = true;
            _isStateVariableSupported[apGL_VERTEX_PROGRAM_TWO_SIDE_NV] = true;
            _isStateVariableSupported[apGL_VERTEX_PROGRAM_BINDING_NV] = true;
            _isStateVariableSupported[apGL_VERTEX_ATTRIB_ARRAY0_NV] = true;
            _isStateVariableSupported[apGL_VERTEX_ATTRIB_ARRAY1_NV] = true;
            _isStateVariableSupported[apGL_VERTEX_ATTRIB_ARRAY2_NV] = true;
            _isStateVariableSupported[apGL_VERTEX_ATTRIB_ARRAY3_NV] = true;
            _isStateVariableSupported[apGL_VERTEX_ATTRIB_ARRAY4_NV] = true;
            _isStateVariableSupported[apGL_VERTEX_ATTRIB_ARRAY5_NV] = true;
            _isStateVariableSupported[apGL_VERTEX_ATTRIB_ARRAY6_NV] = true;
            _isStateVariableSupported[apGL_VERTEX_ATTRIB_ARRAY7_NV] = true;
            _isStateVariableSupported[apGL_VERTEX_ATTRIB_ARRAY8_NV] = true;
            _isStateVariableSupported[apGL_VERTEX_ATTRIB_ARRAY9_NV] = true;
            _isStateVariableSupported[apGL_VERTEX_ATTRIB_ARRAY10_NV] = true;
            _isStateVariableSupported[apGL_VERTEX_ATTRIB_ARRAY11_NV] = true;
            _isStateVariableSupported[apGL_VERTEX_ATTRIB_ARRAY12_NV] = true;
            _isStateVariableSupported[apGL_VERTEX_ATTRIB_ARRAY13_NV] = true;
            _isStateVariableSupported[apGL_VERTEX_ATTRIB_ARRAY14_NV] = true;
            _isStateVariableSupported[apGL_VERTEX_ATTRIB_ARRAY15_NV] = true;
            _isStateVariableSupported[apGL_CURRENT_MATRIX_STACK_DEPTH_NV] = true;
            _isStateVariableSupported[apGL_CURRENT_MATRIX_NV] = true;
            _isStateVariableSupported[apGL_MAX_TRACK_MATRIX_STACK_DEPTH_NV] = true;
            _isStateVariableSupported[apGL_MAX_TRACK_MATRICES_NV] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ATI_fragment_shader
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ATI_fragment_shaderSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ATI_fragment_shader);

        if (isGL_ATI_fragment_shaderSupported)
        {
            _isStateVariableSupported[apGL_FRAGMENT_SHADER_ATI] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_EXT_vertex_shader
        //////////////////////////////////////////////////////////////////////////
        bool isGL_EXT_vertex_shaderSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_EXT_vertex_shader);

        if (isGL_EXT_vertex_shaderSupported)
        {
            _isStateVariableSupported[apGL_VERTEX_SHADER_INSTRUCTIONS_EXT] = true;
            _isStateVariableSupported[apGL_VERTEX_SHADER_VARIANTS_EXT] = true;
            _isStateVariableSupported[apGL_VERTEX_SHADER_INVARIANTS_EXT] = true;
            _isStateVariableSupported[apGL_VERTEX_SHADER_LOCAL_CONSTANTS_EXT] = true;
            _isStateVariableSupported[apGL_VERTEX_SHADER_LOCALS_EXT] = true;
            _isStateVariableSupported[apGL_VERTEX_SHADER_OPTIMIZED_EXT] = true;
            _isStateVariableSupported[apGL_VERTEX_SHADER_EXT] = true;
            _isStateVariableSupported[apGL_VERTEX_SHADER_BINDING_EXT] = true;
            _isStateVariableSupported[apGL_VARIANT_ARRAY_EXT] = true;
            _isStateVariableSupported[apGL_MAX_VERTEX_SHADER_INSTRUCTIONS_EXT] = true;
            _isStateVariableSupported[apGL_MAX_VERTEX_SHADER_VARIANTS_EXT] = true;
            _isStateVariableSupported[apGL_MAX_VERTEX_SHADER_INVARIANTS_EXT] = true;
            _isStateVariableSupported[apGL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT] = true;
            _isStateVariableSupported[apGL_MAX_VERTEX_SHADER_LOCALS_EXT] = true;
            _isStateVariableSupported[apGL_MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS_EXT] = true;
            _isStateVariableSupported[apGL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS_EXT] = true;
            _isStateVariableSupported[apGL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT] = true;
            _isStateVariableSupported[apGL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS_EXT] = true;
            _isStateVariableSupported[apGL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS_EXT] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_NV_texture_shader
        //////////////////////////////////////////////////////////////////////////
        bool isGL_NV_texture_shaderSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_NV_texture_shader);

        if (isGL_NV_texture_shaderSupported)
        {
            _isStateVariableSupported[apGL_LO_BIAS_NV] = true;
            _isStateVariableSupported[apGL_DS_BIAS_NV] = true;
            _isStateVariableSupported[apGL_DT_BIAS_NV] = true;
            _isStateVariableSupported[apGL_MAGNITUDE_BIAS_NV] = true;
            _isStateVariableSupported[apGL_VIBRANCE_BIAS_NV] = true;
            _isStateVariableSupported[apGL_HI_SCALE_NV] = true;
            _isStateVariableSupported[apGL_LO_SCALE_NV] = true;
            _isStateVariableSupported[apGL_DS_SCALE_NV] = true;
            _isStateVariableSupported[apGL_DT_SCALE_NV] = true;
            _isStateVariableSupported[apGL_MAGNITUDE_SCALE_NV] = true;
            _isStateVariableSupported[apGL_VIBRANCE_SCALE_NV] = true;
            _isStateVariableSupported[apGL_TEXTURE_SHADER_NV] = true;
            _isStateVariableSupported[apGL_SHADER_OPERATION_NV] = true;
            _isStateVariableSupported[apGL_CULL_MODES_NV] = true;
            _isStateVariableSupported[apGL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV] = true;
            _isStateVariableSupported[apGL_PREVIOUS_TEXTURE_INPUT_NV] = true;
            _isStateVariableSupported[apGL_CONST_EYE_NV] = true;
            _isStateVariableSupported[apGL_OFFSET_TEXTURE_MATRIX_NV] = true;
            _isStateVariableSupported[apGL_OFFSET_TEXTURE_SCALE_NV] = true;
            _isStateVariableSupported[apGL_OFFSET_TEXTURE_SCALE_NV] = true;
            _isStateVariableSupported[apGL_OFFSET_TEXTURE_BIAS_NV] = true;
            _isStateVariableSupported[apGL_SHADER_CONSISTENT_NV] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_NV_texture_shader3
        //////////////////////////////////////////////////////////////////////////
        bool isGL_NV_texture_shader3Supported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_NV_texture_shader3);

        if (isGL_NV_texture_shader3Supported)
        {
            _isStateVariableSupported[apGL_SHADER_OPERATION_NV] = true;
            _isStateVariableSupported[apGL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ATI_text_fragment_shader
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ATI_text_fragment_shaderSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ATI_text_fragment_shader);

        if (isGL_ATI_text_fragment_shaderSupported)
        {
            _isStateVariableSupported[apGL_TEXT_FRAGMENT_SHADER_ATI] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_NV_fragment_program
        //////////////////////////////////////////////////////////////////////////
        bool isGL_NV_fragment_programSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_NV_fragment_program);

        if (isGL_NV_fragment_programSupported)
        {
            _isStateVariableSupported[apGL_FRAGMENT_PROGRAM_NV] = true;
            _isStateVariableSupported[apGL_FRAGMENT_PROGRAM_BINDING_NV] = true;
            _isStateVariableSupported[apGL_MAX_TEXTURE_COORDS_NV] = true;
            _isStateVariableSupported[apGL_MAX_TEXTURE_IMAGE_UNITS_NV] = true;
            _isStateVariableSupported[apGL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_NV_vertex_program2_option
        //////////////////////////////////////////////////////////////////////////
        bool isGL_NV_vertex_program2_optionSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_NV_vertex_program2_option);

        if (isGL_NV_vertex_program2_optionSupported)
        {
            _isStateVariableSupported[apGL_MAX_PROGRAM_EXEC_INSTRUCTIONS_NV] = true;
            _isStateVariableSupported[apGL_MAX_PROGRAM_CALL_DEPTH_NV] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_NV_shader_buffer_load
        //////////////////////////////////////////////////////////////////////////
        bool isGL_NV_shader_buffer_loadSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_NV_shader_buffer_load);

        if (isGL_NV_shader_buffer_loadSupported)
        {
            _isStateVariableSupported[apGL_MAX_SHADER_BUFFER_ADDRESS_NV] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_NV_vertex_buffer_unified_memory
        //////////////////////////////////////////////////////////////////////////
        bool isGL_NV_vertex_buffer_unified_memorySupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_NV_vertex_buffer_unified_memory);

        if (isGL_NV_vertex_buffer_unified_memorySupported)
        {
            _isStateVariableSupported[apGL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV] = true;
            _isStateVariableSupported[apGL_ELEMENT_ARRAY_UNIFIED_NV] = true;
            _isStateVariableSupported[apGL_VERTEX_ARRAY_LENGTH_NV] = true;
            _isStateVariableSupported[apGL_NORMAL_ARRAY_LENGTH_NV] = true;
            _isStateVariableSupported[apGL_COLOR_ARRAY_LENGTH_NV] = true;
            _isStateVariableSupported[apGL_INDEX_ARRAY_LENGTH_NV] = true;
            _isStateVariableSupported[apGL_EDGE_FLAG_ARRAY_LENGTH_NV] = true;
            _isStateVariableSupported[apGL_SECONDARY_COLOR_ARRAY_LENGTH_NV] = true;
            _isStateVariableSupported[apGL_FOG_COORD_ARRAY_LENGTH_NV] = true;
            _isStateVariableSupported[apGL_ELEMENT_ARRAY_LENGTH_NV] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_AMD_debug_output
        //////////////////////////////////////////////////////////////////////////
        bool isGL_AMD_debug_outputSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_AMD_debug_output);

        if (isGL_AMD_debug_outputSupported)
        {
            _isStateVariableSupported[apGL_MAX_DEBUG_MESSAGE_LENGTH_ARB] = true;
            _isStateVariableSupported[apGL_MAX_DEBUG_LOGGED_MESSAGES_ARB] = true;
            _isStateVariableSupported[apGL_DEBUG_LOGGED_MESSAGES_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_debug_output
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_debug_outputSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_debug_output);

        if (isGL_ARB_debug_outputSupported)
        {
            _isStateVariableSupported[apGL_MAX_DEBUG_MESSAGE_LENGTH_ARB] = true;
            _isStateVariableSupported[apGL_MAX_DEBUG_LOGGED_MESSAGES_ARB] = true;
            _isStateVariableSupported[apGL_DEBUG_LOGGED_MESSAGES_ARB] = true;
            _isStateVariableSupported[apGL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB] = true;
            _isStateVariableSupported[apGL_DEBUG_CALLBACK_FUNCTION_ARB] = true;
            _isStateVariableSupported[apGL_DEBUG_CALLBACK_USER_PARAM_ARB] = true;
        }

        // TO_DO: support GL_KHR_debug (the variables are the same as ARB_debug_output

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_draw_buffers
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_draw_buffersSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_draw_buffers);

        if (isGL_ARB_draw_buffersSupported)
        {
            _isStateVariableSupported[apGL_MAX_DRAW_BUFFERS_ARB] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER0_ARB] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER1_ARB] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER2_ARB] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER3_ARB] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER4_ARB] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER5_ARB] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER6_ARB] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER7_ARB] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER8_ARB] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER9_ARB] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER10_ARB] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER11_ARB] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER12_ARB] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER13_ARB] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER14_ARB] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER15_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ATI_draw_buffers
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ATI_draw_buffersSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ATI_draw_buffers);

        if (isGL_ATI_draw_buffersSupported)
        {
            _isStateVariableSupported[apGL_MAX_DRAW_BUFFERS_ATI] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER0_ATI] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER1_ATI] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER2_ATI] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER3_ATI] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER4_ATI] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER5_ATI] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER6_ATI] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER7_ATI] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER8_ATI] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER9_ATI] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER10_ATI] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER11_ATI] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER12_ATI] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER13_ATI] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER14_ATI] = true;
            _isStateVariableSupported[apGL_DRAW_BUFFER15_ATI] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_color_buffer_float
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_color_buffer_floatSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_color_buffer_float);

        if (isGL_ARB_color_buffer_floatSupported)
        {
            _isStateVariableSupported[apGL_RGBA_FLOAT_MODE_ARB] = true;
            _isStateVariableSupported[apGL_CLAMP_VERTEX_COLOR_ARB] = true;
            _isStateVariableSupported[apGL_CLAMP_FRAGMENT_COLOR_ARB] = true;
            _isStateVariableSupported[apGL_CLAMP_READ_COLOR_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_point_sprite
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_point_spriteSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_point_sprite);

        if (isGL_ARB_point_spriteSupported)
        {
            _isStateVariableSupported[apGL_POINT_SPRITE_ARB] = true;
            _isStateVariableSupported[apGL_COORD_REPLACE_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_EXT_stencil_two_side
        //////////////////////////////////////////////////////////////////////////
        bool isGL_EXT_stencil_two_sideSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_EXT_stencil_two_side);

        if (isGL_EXT_stencil_two_sideSupported)
        {
            _isStateVariableSupported[apGL_STENCIL_TEST_TWO_SIDE_EXT] = true;
            _isStateVariableSupported[apGL_ACTIVE_STENCIL_FACE_EXT] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_multitexture
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_multitextureSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_multitexture);

        if (isGL_ARB_multitextureSupported)
        {
            _isStateVariableSupported[apGL_ACTIVE_TEXTURE_ARB] = true;
            _isStateVariableSupported[apGL_CLIENT_ACTIVE_TEXTURE_ARB] = true;
            _isStateVariableSupported[apGL_MAX_TEXTURE_UNITS_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_EXT_blend_logic_op
        //////////////////////////////////////////////////////////////////////////
        bool isGL_EXT_blend_logic_opSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_EXT_blend_logic_op);

        if (isGL_EXT_blend_logic_opSupported)
        {
            _isStateVariableSupported[apGL_BLEND_EQUATION_EXT] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_EXT_blend_minmax
        //////////////////////////////////////////////////////////////////////////
        bool isGL_EXT_blend_minmaxSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_EXT_blend_minmax);

        if (isGL_EXT_blend_minmaxSupported)
        {
            _isStateVariableSupported[apGL_BLEND_EQUATION_EXT] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_SGIX_interlace
        //////////////////////////////////////////////////////////////////////////
        bool isGL_SGIX_interlaceSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_SGIX_interlace);

        if (isGL_SGIX_interlaceSupported)
        {
            _isStateVariableSupported[apGL_INTERLACE_SGIX] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_EXT_framebuffer_object
        //////////////////////////////////////////////////////////////////////////
        bool isGL_EXT_framebuffer_objectSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_EXT_framebuffer_object);

        if (isGL_EXT_framebuffer_objectSupported)
        {
            _isStateVariableSupported[apGL_FRAMEBUFFER_BINDING_EXT] = true;
            _isStateVariableSupported[apGL_RENDERBUFFER_BINDING_EXT] = true;
            _isStateVariableSupported[apGL_MAX_RENDERBUFFER_SIZE_EXT] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_EXT_framebuffer_blit
        //////////////////////////////////////////////////////////////////////////
        bool isGL_EXT_framebuffer_blitSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_EXT_framebuffer_blit);

        if (isGL_EXT_framebuffer_blitSupported)
        {
            _isStateVariableSupported[apGL_DRAW_FRAMEBUFFER_BINDING_EXT] = true;
            _isStateVariableSupported[apGL_READ_FRAMEBUFFER_BINDING_EXT] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_EXT_framebuffer_multisample
        //////////////////////////////////////////////////////////////////////////
        bool isGL_EXT_framebuffer_multisampleSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_EXT_framebuffer_multisample);

        if (isGL_EXT_framebuffer_multisampleSupported)
        {
            _isStateVariableSupported[apGL_MAX_SAMPLES_EXT] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_framebuffer_object
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_framebuffer_objectSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_framebuffer_object);

        if (isGL_ARB_framebuffer_objectSupported)
        {
            _isStateVariableSupported[apGL_FRAMEBUFFER_BINDING] = true;
            _isStateVariableSupported[apGL_RENDERBUFFER_BINDING] = true;
            _isStateVariableSupported[apGL_MAX_RENDERBUFFER_SIZE] = true;
            _isStateVariableSupported[apGL_DRAW_FRAMEBUFFER_BINDING] = true;
            _isStateVariableSupported[apGL_READ_FRAMEBUFFER_BINDING] = true;
            _isStateVariableSupported[apGL_MAX_SAMPLES] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_texture_rectangle
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_texture_rectangleSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_texture_rectangle);

        if (isGL_ARB_texture_rectangleSupported)
        {
            _isStateVariableSupported[apGL_TEXTURE_RECTANGLE_ARB] = true;
            _isStateVariableSupported[apGL_TEXTURE_BINDING_RECTANGLE_ARB] = true;
            _isStateVariableSupported[apGL_MAX_RECTANGLE_TEXTURE_SIZE_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_NV_texture_rectangle
        //////////////////////////////////////////////////////////////////////////
        bool isGL_NV_texture_rectangleSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_NV_texture_rectangle);

        if (isGL_NV_texture_rectangleSupported)
        {
            _isStateVariableSupported[apGL_TEXTURE_RECTANGLE_NV] = true;
            _isStateVariableSupported[apGL_TEXTURE_BINDING_RECTANGLE_NV] = true;
            _isStateVariableSupported[apGL_MAX_RECTANGLE_TEXTURE_SIZE_NV] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_multisample
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_multisampleSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_multisample);

        if (isGL_ARB_multisampleSupported)
        {
            _isStateVariableSupported[apGL_MULTISAMPLE_ARB] = true;
            _isStateVariableSupported[apGL_SAMPLE_ALPHA_TO_COVERAGE_ARB] = true;
            _isStateVariableSupported[apGL_SAMPLE_ALPHA_TO_ONE_ARB] = true;
            _isStateVariableSupported[apGL_SAMPLE_COVERAGE_ARB] = true;
            _isStateVariableSupported[apGL_SAMPLE_COVERAGE_INVERT_ARB] = true;
            _isStateVariableSupported[apGL_SAMPLE_COVERAGE_VALUE_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_EXT_geometry_shader4
        //////////////////////////////////////////////////////////////////////////
        bool isGL_EXT_geometry_shader4Supported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_EXT_geometry_shader4);

        if (isGL_EXT_geometry_shader4Supported)
        {
            _isStateVariableSupported[apGL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT] = true;
            _isStateVariableSupported[apGL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT] = true;
            _isStateVariableSupported[apGL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT] = true;
            _isStateVariableSupported[apGL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT] = true;
            _isStateVariableSupported[apGL_MAX_GEOMETRY_VARYING_COMPONENTS_EXT] = true;
            _isStateVariableSupported[apGL_MAX_VERTEX_VARYING_COMPONENTS_EXT] = true;
            _isStateVariableSupported[apGL_MAX_VARYING_COMPONENTS_EXT] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_EXT_texture_array
        //////////////////////////////////////////////////////////////////////////
        bool isGL_EXT_texture_arraySupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_EXT_texture_array);

        if (isGL_EXT_texture_arraySupported)
        {
            _isStateVariableSupported[apGL_TEXTURE_BINDING_1D_ARRAY_EXT] = true;
            _isStateVariableSupported[apGL_TEXTURE_BINDING_2D_ARRAY_EXT] = true;
            _isStateVariableSupported[apGL_MAX_ARRAY_TEXTURE_LAYERS_EXT] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_EXT_compiled_vertex_array
        //////////////////////////////////////////////////////////////////////////
        bool isGL_EXT_compiled_vertex_arraySupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_EXT_compiled_vertex_array);

        if (isGL_EXT_compiled_vertex_arraySupported)
        {
            _isStateVariableSupported[apGL_ARRAY_ELEMENT_LOCK_FIRST_EXT] = true;
            _isStateVariableSupported[apGL_ARRAY_ELEMENT_LOCK_COUNT_EXT] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_transpose_matrix
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_transpose_matrixSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_transpose_matrix);

        if (isGL_ARB_transpose_matrixSupported)
        {
            _isStateVariableSupported[apGL_TRANSPOSE_MODELVIEW_MATRIX_ARB] = true;
            _isStateVariableSupported[apGL_TRANSPOSE_PROJECTION_MATRIX_ARB] = true;
            _isStateVariableSupported[apGL_TRANSPOSE_TEXTURE_MATRIX_ARB] = true;
            _isStateVariableSupported[apGL_TRANSPOSE_COLOR_MATRIX_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_point_parameters
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_point_parametersSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_point_parameters);

        if (isGL_ARB_point_parametersSupported)
        {
            _isStateVariableSupported[apGL_POINT_SIZE_MIN_ARB] = true;
            _isStateVariableSupported[apGL_POINT_SIZE_MAX_ARB] = true;
            _isStateVariableSupported[apGL_POINT_FADE_THRESHOLD_SIZE_ARB] = true;
            _isStateVariableSupported[apGL_POINT_DISTANCE_ATTENUATION_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_texture_float
        //////////////////////////////////////////////////////////////////////////
        // Uri 22/9/08: see comment in apOpenGLStateVariableId.h under the
        // GL_ARB_texture_float header.
        /*
        bool isGL_ARB_texture_floatSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_texture_float);
        if (isGL_ARB_texture_floatSupported)
        {
        _isStateVariableSupported[apGL_TEXTURE_RED_TYPE_ARB] = true;
        _isStateVariableSupported[apGL_TEXTURE_GREEN_TYPE_ARB] = true;
        _isStateVariableSupported[apGL_TEXTURE_BLUE_TYPE_ARB] = true;
        _isStateVariableSupported[apGL_TEXTURE_ALPHA_TYPE_ARB] = true;
        _isStateVariableSupported[apGL_TEXTURE_LUMINANCE_TYPE_ARB] = true;
        _isStateVariableSupported[apGL_TEXTURE_INTENSITY_TYPE_ARB] = true;
        _isStateVariableSupported[apGL_TEXTURE_DEPTH_TYPE_ARB] = true;
        }
        */

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_pixel_buffer_object
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_pixel_buffer_objectSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_pixel_buffer_object);

        if (isGL_ARB_pixel_buffer_objectSupported)
        {
            _isStateVariableSupported[apGL_PIXEL_PACK_BUFFER_BINDING_ARB] = true;
            _isStateVariableSupported[apGL_PIXEL_UNPACK_BUFFER_BINDING_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_framebuffer_sRGB
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_framebuffer_sRGBSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_framebuffer_sRGB);

        if (isGL_ARB_framebuffer_sRGBSupported)
        {
            _isStateVariableSupported[apGL_FRAMEBUFFER_SRGB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_geometry_shader4
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_geometry_shader4Supported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_geometry_shader4);

        if (isGL_ARB_geometry_shader4Supported)
        {
            _isStateVariableSupported[apGL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_GEOMETRY_OUTPUT_VERTICES_ARB] = true;
            _isStateVariableSupported[apGL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_GEOMETRY_VARYING_COMPONENTS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_VERTEX_VARYING_COMPONENTS_ARB] = true;
            _isStateVariableSupported[apGL_MAX_VARYING_COMPONENTS] = true;
        }

        /*  //////////////////////////////////////////////////////////////////////////
        // GL_ARB_instanced_arrays
        //////////////////////////////////////////////////////////////////////////
        // Uri, 09/09/08: The state variable identifier (GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ARB = 0x88FE) does not appear in glext.h
        bool isGL_ARB_instanced_arraysSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_instanced_arrays);
        if (isGL_ARB_instanced_arraysSupported)
        {
        _isStateVariableSupported[apGL_VERTEX_ATTRIB_ARRAY_DIVISOR_ARB] = true;
        }*/

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_texture_buffer_object
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_texture_buffer_objectSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_texture_buffer_object);

        if (isGL_ARB_texture_buffer_objectSupported)
        {
            _isStateVariableSupported[apGL_TEXTURE_BINDING_BUFFER_ARB] = true;
            _isStateVariableSupported[apGL_TEXTURE_BUFFER_DATA_STORE_BINDING_EXT] = true;
            _isStateVariableSupported[apGL_TEXTURE_BUFFER_FORMAT_ARB] = true;
            _isStateVariableSupported[apGL_TEXTURE_BUFFER_ARB] = true;
            _isStateVariableSupported[apGL_MAX_TEXTURE_BUFFER_SIZE_ARB] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_EXT_texture_buffer_object
        //////////////////////////////////////////////////////////////////////////
        bool isGL_EXT_texture_buffer_objectSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_EXT_texture_buffer_object);

        if (isGL_EXT_texture_buffer_objectSupported)
        {
            _isStateVariableSupported[apGL_TEXTURE_BINDING_BUFFER_EXT] = true;
            _isStateVariableSupported[apGL_TEXTURE_BUFFER_FORMAT_EXT] = true;
            _isStateVariableSupported[apGL_TEXTURE_BUFFER_EXT] = true;
            _isStateVariableSupported[apGL_MAX_TEXTURE_BUFFER_SIZE_EXT] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_vertex_array_object
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_vertex_array_objectSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_vertex_array_object);

        if (isGL_ARB_vertex_array_objectSupported)
        {
            _isStateVariableSupported[apGL_VERTEX_ARRAY_BINDING] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_uniform_buffer_object
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_uniform_buffer_objectSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_uniform_buffer_object);

        if (isGL_ARB_uniform_buffer_objectSupported)
        {
            _isStateVariableSupported[apGL_UNIFORM_BUFFER_BINDING] = true;
            _isStateVariableSupported[apGL_MAX_VERTEX_UNIFORM_BLOCKS] = true;
            _isStateVariableSupported[apGL_MAX_FRAGMENT_UNIFORM_BLOCKS] = true;
            _isStateVariableSupported[apGL_MAX_GEOMETRY_UNIFORM_BLOCKS] = true;
            _isStateVariableSupported[apGL_MAX_COMBINED_UNIFORM_BLOCKS] = true;
            _isStateVariableSupported[apGL_MAX_UNIFORM_BUFFER_BINDINGS] = true;
            _isStateVariableSupported[apGL_MAX_UNIFORM_BLOCK_SIZE] = true;
            _isStateVariableSupported[apGL_MAX_VERTEX_UNIFORM_COMPONENTS] = true;
            _isStateVariableSupported[apGL_MAX_FRAGMENT_UNIFORM_COMPONENTS] = true;
            _isStateVariableSupported[apGL_MAX_GEOMETRY_UNIFORM_COMPONENTS] = true;
            _isStateVariableSupported[apGL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS] = true;
            _isStateVariableSupported[apGL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS] = true;
            _isStateVariableSupported[apGL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS] = true;
            _isStateVariableSupported[apGL_UNIFORM_BUFFER_OFFSET_ALIGNMENT] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_provoking_vertex
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_provoking_vertexSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_provoking_vertex);

        if (isGL_ARB_provoking_vertexSupported)
        {
            _isStateVariableSupported[apGL_PROVOKING_VERTEX] = true;
            _isStateVariableSupported[apGL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_seamless_cube_map
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_seamless_cube_mapSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_seamless_cube_map);

        if (isGL_ARB_seamless_cube_mapSupported)
        {
            _isStateVariableSupported[apGL_TEXTURE_CUBE_MAP_SEAMLESS] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_sync
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_syncSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_sync);

        if (isGL_ARB_syncSupported)
        {
            _isStateVariableSupported[apGL_MAX_SERVER_WAIT_TIMEOUT] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_texture_multisample
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_texture_multisampleSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_texture_multisample);

        if (isGL_ARB_texture_multisampleSupported)
        {
            _isStateVariableSupported[apGL_SAMPLE_MASK] = true;
            //_isStateVariableSupported[apGL_SAMPLE_MASK_VALUE] = true;
            _isStateVariableSupported[apGL_TEXTURE_BINDING_2D_MULTISAMPLE] = true;
            _isStateVariableSupported[apGL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY] = true;
            _isStateVariableSupported[apGL_MAX_SAMPLE_MASK_WORDS] = true;
            _isStateVariableSupported[apGL_MAX_COLOR_TEXTURE_SAMPLES] = true;
            _isStateVariableSupported[apGL_MAX_DEPTH_TEXTURE_SAMPLES] = true;
            _isStateVariableSupported[apGL_MAX_INTEGER_SAMPLES] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_vertex_array_bgra
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_vertex_array_bgraSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_vertex_array_bgra);

        if (isGL_ARB_vertex_array_bgraSupported)
        {
            // See comment in apOpenGLStateVariableId.h under GL_ARB_vertex_array_bgra
            //_isStateVariableSupported[apGL_VERTEX_ATTRIB_ARRAY_SIZE] = true;
        }


        //////////////////////////////////////////////////////////////////////////
        // GL_ARB_depth_clamp
        //////////////////////////////////////////////////////////////////////////
        bool isGL_ARB_depth_clampSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_ARB_depth_clamp);

        if (isGL_ARB_depth_clampSupported)
        {
            _isStateVariableSupported[apGL_DEPTH_CLAMP] = true;
        }

        // Windows only:
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

        //////////////////////////////////////////////////////////////////////////
        // WGL_EXT_swap_control
        //////////////////////////////////////////////////////////////////////////
        bool isWGL_EXT_swap_control = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_WGL_EXT_swap_control);

        if (isWGL_EXT_swap_control)
        {
            _isStateVariableSupported[apWGL_SWAP_INTERVAL_EXT] = true;
        }

#endif

        // Mac only:
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
        //////////////////////////////////////////////////////////////////////////
        // GL_APPLE_aux_depth_stencil
        // This extension is supported only on MAC OS
        //////////////////////////////////////////////////////////////////////////
        bool isGL_APPLE_aux_depth_stencilSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_APPLE_aux_depth_stencil);

        if (isGL_APPLE_aux_depth_stencilSupported)
        {
            _isStateVariableSupported[apGL_AUX_DEPTH_STENCIL_APPLE] = true;
        }

#endif

        //////////////////////////////////////////////////////////////////////////
        // GL_APPLE_client_storage
        //////////////////////////////////////////////////////////////////////////
        bool isGL_APPLE_client_storageSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_APPLE_client_storage);

        if (isGL_APPLE_client_storageSupported)
        {
            _isStateVariableSupported[apGL_UNPACK_CLIENT_STORAGE_APPLE] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_APPLE_element_array
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        bool isGL_APPLE_element_arraySupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_APPLE_element_array);

        if (isGL_APPLE_element_arraySupported)
        {
            _isStateVariableSupported[apGL_ELEMENT_ARRAY_APPLE] = true;
            _isStateVariableSupported[apGL_ELEMENT_ARRAY_TYPE_APPLE] = true;
            _isStateVariableSupported[apGL_ELEMENT_ARRAY_POINTER_APPLE] = true;
        }

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
        //////////////////////////////////////////////////////////////////////////
        // GL_APPLE_float_pixels
        //////////////////////////////////////////////////////////////////////////
        bool isGL_APPLE_float_pixelsSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_APPLE_float_pixels);

        if (isGL_APPLE_float_pixelsSupported)
        {
            _isStateVariableSupported[apGL_COLOR_FLOAT_APPLE] = true;
        }

#endif

        //////////////////////////////////////////////////////////////////////////
        // GL_APPLE_specular_vector
        //////////////////////////////////////////////////////////////////////////
        bool isGL_APPLE_specular_vectorSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_APPLE_specular_vector);

        if (isGL_APPLE_specular_vectorSupported)
        {
            _isStateVariableSupported[apGL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_APPLE_transform_hint
        //////////////////////////////////////////////////////////////////////////
        bool isGL_APPLE_transform_hintSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_APPLE_transform_hint);

        if (isGL_APPLE_transform_hintSupported)
        {
            _isStateVariableSupported[apGL_TRANSFORM_HINT_APPLE] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_APPLE_vertex_array_object
        //////////////////////////////////////////////////////////////////////////
        bool isGL_APPLE_vertex_array_objectSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_APPLE_vertex_array_object);

        if (isGL_APPLE_vertex_array_objectSupported)
        {
            _isStateVariableSupported[apGL_VERTEX_ARRAY_BINDING_APPLE] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_APPLE_vertex_array_range,
        //////////////////////////////////////////////////////////////////////////
        bool isGL_vertex_array_rangeSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_APPLE_vertex_array_range);

        if (isGL_vertex_array_rangeSupported)
        {
            _isStateVariableSupported[apGL_VERTEX_ARRAY_RANGE_APPLE] = true;
            _isStateVariableSupported[apGL_VERTEX_ARRAY_RANGE_POINTER_APPLE] = true;
            _isStateVariableSupported[apGL_VERTEX_ARRAY_RANGE_LENGTH_APPLE] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_EXT_texture_integer,
        //////////////////////////////////////////////////////////////////////////
        bool isGL_EXT_texture_integerSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_EXT_texture_integer);

        if (isGL_EXT_texture_integerSupported)
        {
            _isStateVariableSupported[apGL_RGBA_INTEGER_MODE_EXT] = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // GL_EXT_bindable_uniform
        //////////////////////////////////////////////////////////////////////////
        bool isGL_EXT_bindable_uniformSupported = theExtensionsMgr.isExtensionSupported(myRenderContextId, AP_GL_EXT_bindable_uniform);

        if (isGL_EXT_bindable_uniformSupported)
        {
            _isStateVariableSupported[apGL_UNIFORM_BUFFER_BINDING_EXT] = true;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsStateVariablesSnapshot::initFilters
// Description: This function enables the filter support of the state variable
//              When filter is enabled, only the state variable ids in _pStateVariableIds
//              values are read
// Arguments:   const gtVector<apOpenGLStateVariableId>* pVectorOfFilteredStateVariables - a pointer to the vector of filtered ids
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        18/8/2008
// ---------------------------------------------------------------------------
bool gsStateVariablesSnapshot::supportOnlyFilteredStateVariableIds(const gtVector<apOpenGLStateVariableId>* pVectorOfFilteredStateVariables)
{
    bool retVal = false;

    _pStateVariablesFilteredIdsVector = pVectorOfFilteredStateVariables;
    return retVal;
}
