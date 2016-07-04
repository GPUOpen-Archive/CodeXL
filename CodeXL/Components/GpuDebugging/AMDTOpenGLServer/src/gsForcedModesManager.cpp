//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsForcedModesManager.cpp
///
//==================================================================================

//------------------------------ gsForcedModesManager.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apDefinitions.h>

// Local:
#include <src/gsStringConstants.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsWrappersCommon.h>
#include <src/gsRenderContextMonitor.h>
#include <src/gsProgramsAndShadersMonitor.h>
#include <src/gsLightsMonitor.h>
#include <src/gsForcedModesManager.h>
#include <src/gsGLDebugOutputManager.h>


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::gsForcedModesManager
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        14/4/2005
// ---------------------------------------------------------------------------
gsForcedModesManager::gsForcedModesManager()
    : _pForcedRenderContextMonitor(NULL),
      _forcedPolygonRasterMode(AP_RASTER_FILL),
      _pendingForcedPolygonRasterMode(AP_RASTER_FILL),
      _separatePolygonModesSupported(true),
      m_debugOutputLoggingEnabled(false)
{
    for (int i = 0; i < AP_OPENGL_AMOUNT_OF_FORCED_STUBS; i++)
    {
        _isModeForced[i] = false;
        _isPendingModeForced[i] = false;
    }

    for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES; i++)
    {
        m_debugOutputSeverityEnabled[i] = false;
    }

    for (int s = 0; s < AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES; s++)
    {
        for (int t = 0; t < AP_NUMBER_OF_DEBUG_OUTPUT_TYPES; t++)
        {
            apGLDebugOutputKindFromMutableFlagArray(m_debugOutputKindsEnabled, (apGLDebugOutputSource)s, (apGLDebugOutputType)t) = false;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager:~gsForcedModesManager
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        14/4/2005
// ---------------------------------------------------------------------------
gsForcedModesManager::~gsForcedModesManager()
{
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::onFirstTimeContextMadeCurrent
// Description:
//   Is called on the first time in which the context on which we force modes
//   is made the current context.
// Arguments:   forcedRenderContextMonitor - The monitor of the render context
//                                           on which we force modes.
// Author:      Yaki Tebeka
// Date:        14/4/2005
// ---------------------------------------------------------------------------
void gsForcedModesManager::onFirstTimeContextMadeCurrent(gsRenderContextMonitor& forcedRenderContextMonitor)
{
    _pForcedRenderContextMonitor = &forcedRenderContextMonitor;

    // If my context is not the NULL context:
    if (_pForcedRenderContextMonitor->spyId() != AP_NULL_CONTEXT_ID)
    {
        // Initialize the attribute stack:
        bool isOpenGL31CoreContext = forcedRenderContextMonitor.isOpenGLVersionOrNewerCoreContext();
        _separatePolygonModesSupported = !isOpenGL31CoreContext;
        _attribStack.onFirstTimeContextMadeCurrent(!isOpenGL31CoreContext);
    }
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::onFrameTerminatorCall
// Description:
//   Is called when a frame terminator function is called.
//   Applies pending forced modes events.
//
// Author:      Yaki Tebeka
// Date:        20/12/2004
// ---------------------------------------------------------------------------
void gsForcedModesManager::onFrameTerminatorCall()
{
    // Apply any pending forced modes events:
    applyPendingForcedModesEvents();
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::onViewPortSet
// Description: Is called when the debugged application sets the view-port
// Arguments:   x, y - The lower-left corner of the view-port rectangle.
//              width, height - The view-port width and height.
// Author:      Yaki Tebeka
// Date:        13/3/2005
// ---------------------------------------------------------------------------
void gsForcedModesManager::onViewPortSet(GLint x, GLint y, GLsizei width, GLsizei height)
{
    _attribStack.onViewPortSet(x, y, width, height);
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::onSetDrawBuffersCall
// Description: Is called when the debugged application changes the drawn buffers.
// Arguments:   drawnBuffers - The new drawn buffers.
// Author:      Yaki Tebeka
// Date:        16/11/2004
// ---------------------------------------------------------------------------
void gsForcedModesManager::onSetDrawBuffersCall(GLenum drawnBuffers)
{
    _attribStack.onSetDrawBuffersCall(drawnBuffers);
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::onPolygonRasterModeSet
// Description: Is called when the debugged application sets the polygon raster
//              mode.
// Arguments:   face - The changed face.
//              rasterMode - the new polygon raster mode.
// Author:      Yaki Tebeka
// Date:        14/11/2004
// ---------------------------------------------------------------------------
void gsForcedModesManager::onPolygonRasterModeSet(GLenum face, GLenum rasterMode)
{
    _attribStack.onPolygonRasterModeSet(face, rasterMode);
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::onPushAttribCalled
// Description: Is called when glPopAttrib is called.
// Arguments:   mask - A mask that indicates which attributes were saved by
//                     the glPopAttrib call.
// Author:      Yaki Tebeka
// Date:        21/6/2005
// ---------------------------------------------------------------------------
void gsForcedModesManager::onPushAttribCalled(GLbitfield mask)
{
    // Push the attribute stack:
    _attribStack.onPushAttribCalled(mask);
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::onPopAttribCalled
// Description: Is called after glPopAttrib was called.
// Author:      Yaki Tebeka
// Date:        19/6/2005
// Implementation Notes:
//  The debugged application just popped the attributes stack.
//  We need to re-apply some of the forced modes:
// ---------------------------------------------------------------------------
void gsForcedModesManager::onPopAttribCalled()
{
    // Pop the attribute stack:
    _attribStack.onPopAttribCalled();

    // Reapply the forced modes (if any):
    reapplyOrCancelForcedModes();
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::forceStub
// Description: Enables to force this render context drawn buffer to be the front buffer.
// Arguments:   apOpenGLForcedModeType stubType - the stub type
//              isStubForced - should stub be forced
// Author:      Yaki Tebeka
// Date:        20/12/2004
// ---------------------------------------------------------------------------
void gsForcedModesManager::forceStub(apOpenGLForcedModeType stubType, bool isStubForced)
{
    // Set the pending force mode. Do not set the real, it would be applied applyPendingForcedModesEvents:
    _isPendingModeForced[stubType] = isStubForced;
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::forcePolygonRasterMode
// Description: Force the polygon raster mode to be the input raster mode.
// Arguments:   rasterMode - The raster mode to be forced.
// Author:      Yaki Tebeka
// Date:        20/12/2004
// ---------------------------------------------------------------------------
void gsForcedModesManager::forcePolygonRasterMode(apRasterMode rasterMode)
{
    // Notice that polygon raster mode can be already on, since the user may
    // change the forced mode (line / fill / points) without canceling the mode first.
    // I.E: Its an error to verify that the mode is off.

    // Store the pending forced raster mode:
    _pendingForcedPolygonRasterMode = rasterMode;

    // Raise the appropriate event flag:
    _isPendingModeForced[AP_OPENGL_FORCED_POLYGON_RASTER_MODE] = true;
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::cancelPolygonRasterModeForcing
// Description: Cancels the "Force Polygon Raster mode" mode.
// Author:      Yaki Tebeka
// Date:        14/11/2004
// ---------------------------------------------------------------------------
void gsForcedModesManager::cancelPolygonRasterModeForcing()
{
    // Raise the appropriate event flag:
    _isPendingModeForced[AP_OPENGL_FORCED_POLYGON_RASTER_MODE] = false;
}

// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::setGLDebugOutputSeverityEnabled
// Description: Applies the debug output severity enable value
// Author:      Uri Shomroni
// Date:        29/06/2014
// ---------------------------------------------------------------------------
void gsForcedModesManager::setGLDebugOutputSeverityEnabled(apGLDebugOutputSeverity severity, bool enabled)
{
    GT_IF_WITH_ASSERT((0 <= severity) && (AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES > severity))
    {
        m_debugOutputSeverityEnabled[severity] = enabled;
    }
}

// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::setGLDebugOutputSeverityEnabled
// Description: Applies the debug output kind enable values
// Author:      Uri Shomroni
// Date:        29/06/2014
// ---------------------------------------------------------------------------
void gsForcedModesManager::setGLDebugOutputKindMask(const gtUInt64& mask)
{
    for (int s = 0; s < AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES; s++)
    {
        for (int t = 0; t < AP_NUMBER_OF_USER_VISIBLE_DEBUG_OUTPUT_TYPES; t++)
        {
            // Calculate the current flag:
            gtUInt64 currentFlag = 1LLU << (t * AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES + s);
            bool shouldEnable = (currentFlag == (mask & currentFlag));

            // Set the flag:
            apGLDebugOutputKindFromMutableFlagArray(m_debugOutputKindsEnabled, (apGLDebugOutputSource)s, (apGLDebugOutputType)t) = shouldEnable;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::applyPendingForcedModesEvents
// Description: Applies any pending forced modes events:
// Author:      Yaki Tebeka
// Date:        2/9/2008
// ---------------------------------------------------------------------------
void gsForcedModesManager::applyPendingForcedModesEvents()
{
    // Sanity check:
    if (_pForcedRenderContextMonitor != NULL)
    {
        // If we are not in glBegin-glEnd block:
        if (!(_pForcedRenderContextMonitor->isInOpenGLBeginEndBlock()))
        {
            for (int i = 0 ; i < AP_OPENGL_AMOUNT_OF_FORCED_STUBS; i++)
            {
                if (_isPendingModeForced[i] != _isModeForced[i])
                {
                    bool rc = applyForceMode((apOpenGLForcedModeType)i, _isPendingModeForced[i]);
                    GT_ASSERT(rc);
                }

                // Copy the new mode:
                _isModeForced[i] = _isPendingModeForced[i];
            }

            // If the raster mode changed (without being turned off), also apply it:
            if (_isModeForced[AP_OPENGL_FORCED_POLYGON_RASTER_MODE] && (_pendingForcedPolygonRasterMode != _forcedPolygonRasterMode))
            {
                bool rc = applyForceMode(AP_OPENGL_FORCED_POLYGON_RASTER_MODE, true);
                GT_ASSERT(rc);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::copyForcedModesFromOtherManager
// Description: Copies the active and pending forced modes from another manager.
// Author:      Uri Shomroni
// Date:        3/2/2010
// ---------------------------------------------------------------------------
void gsForcedModesManager::copyForcedModesFromOtherManager(const gsForcedModesManager& otherMgr)
{
    // Copy all the pending states:
    for (int i = 0; i < AP_OPENGL_AMOUNT_OF_FORCED_STUBS; i++)
    {
        _isPendingModeForced[i] = otherMgr._isPendingModeForced[i];
    }

    _pendingForcedPolygonRasterMode = otherMgr._pendingForcedPolygonRasterMode;
    _forcedPolygonRasterMode = otherMgr._forcedPolygonRasterMode;

    m_debugOutputLoggingEnabled = otherMgr.m_debugOutputLoggingEnabled;

    for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES; i++)
    {
        m_debugOutputSeverityEnabled[i] = otherMgr.m_debugOutputSeverityEnabled[i];
    }

    for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES * AP_NUMBER_OF_DEBUG_OUTPUT_TYPES; i++)
    {
        m_debugOutputKindsEnabled[i] = otherMgr.m_debugOutputKindsEnabled[i];
    }
}

// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::applyForcedFrontDrawBuffer
// Description: Applies the "Forced" front draw buffer
// Author:      Yaki Tebeka
// Date:        16/11/2004
// ---------------------------------------------------------------------------
void gsForcedModesManager::applyForcedFrontDrawBuffer()
{
    if (_pForcedRenderContextMonitor)
    {
        // If my context is not the NULL context:
        if (_pForcedRenderContextMonitor->spyId() != AP_NULL_CONTEXT_ID)
        {
#ifdef _GR_IPHONE_BUILD
            // TO_DO iPhone
#else
            // Force the front buffer draw:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDrawBuffer);
            gs_stat_realFunctionPointers.glDrawBuffer(GL_FRONT);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDrawBuffer);
#endif

            // Mark the attrib stack item as "forced":
            _attribStack.topItem()._isDrawnBufferForced = true;
        }
    }

    // Turn the flag on:
    _isModeForced[AP_OPENGL_FORCED_FRONT_DRAW_BUFFER_MODE] = true;
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::cancelForcedFrontDrawBuffer
// Description: Cancels the "Forced" front draw buffer.
// Author:      Yaki Tebeka
// Date:        16/11/2004
// ---------------------------------------------------------------------------
void gsForcedModesManager::cancelForcedFrontDrawBuffer()
{
    if (_pForcedRenderContextMonitor)
    {
        // If my context is not the NULL context:
        if (_pForcedRenderContextMonitor->spyId() != AP_NULL_CONTEXT_ID)
        {
            // Resume the "real" drawn buffers:
            GLenum realDrawnBuffers = _attribStack.topItem()._drawnBuffers;
#ifdef _GR_IPHONE_BUILD
            // TO_DO iPhone
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDrawBuffer);
            gs_stat_realFunctionPointers.glDrawBuffer(realDrawnBuffers);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDrawBuffer);
#endif

            // Mark the attrib stack item as "not forced":
            _attribStack.topItem()._isDrawnBufferForced = false;
        }
    }

    // Mark that the "Force front draw buffer" mode was canceled:
    _isModeForced[AP_OPENGL_FORCED_FRONT_DRAW_BUFFER_MODE] = false;
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::applyForcedPolygonRasterMode
// Description: Applies the "Forced" polygon raster mode.
// Author:      Yaki Tebeka
// Date:        14/11/2004
// ---------------------------------------------------------------------------
void gsForcedModesManager::applyForcedPolygonRasterMode()
{
    if (_pForcedRenderContextMonitor)
    {
        // If my context is not the NULL context:
        if (_pForcedRenderContextMonitor->spyId() != AP_NULL_CONTEXT_ID)
        {
            // Force the forced polygon mode:
            GLenum forcedPolygonMode = apRasterModeToGLenum(_pendingForcedPolygonRasterMode);
#ifdef _GR_IPHONE_BUILD
            // This is done on the OpenGL ES draw commands (glDrawArrays, glDrawElements),
            // and not here since glPolygonMode is not supported by OpenGL ES.
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glPolygonMode);
            gs_stat_realFunctionPointers.glPolygonMode(GL_FRONT_AND_BACK, forcedPolygonMode);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glPolygonMode);
#endif

            // Mark the attrib stack item as "forced":
            _attribStack.topItem()._isPolygonRasterModeForced = true;

        }
    }

    // Note the forced mode:
    _forcedPolygonRasterMode = _pendingForcedPolygonRasterMode;

    // Turn the flag on:
    _isModeForced[AP_OPENGL_FORCED_POLYGON_RASTER_MODE] = true;
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::cancelForcedPolygonRasterMode
// Description: Cancels the "Forced" polygon raster mode.
// Author:      Yaki Tebeka
// Date:        14/11/2004
// ---------------------------------------------------------------------------
void gsForcedModesManager::cancelForcedPolygonRasterMode()
{
    if (_pForcedRenderContextMonitor)
    {
        // If my context is not the NULL context:
        if (_pForcedRenderContextMonitor->spyId() != AP_NULL_CONTEXT_ID)
        {
#ifdef _GR_IPHONE_BUILD
            // This is done on the OpenGL ES draw commands (glDrawArrays, glDrawElements),
            // and not here since glPolygonMode is not supported by OpenGL ES.
#else

            if (_separatePolygonModesSupported)
            {
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glPolygonMode);
                // Resume the "real" front polygon mode:
                GLenum realRasterModeFront = _attribStack.topItem()._rasterModeFrontFace;
                gs_stat_realFunctionPointers.glPolygonMode(GL_FRONT, realRasterModeFront);

                // Resume the "real" back polygon mode:
                GLenum realRasterModeBack = _attribStack.topItem()._rasterModeBackFace;
                gs_stat_realFunctionPointers.glPolygonMode(GL_BACK, realRasterModeBack);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glPolygonMode);
            }
            else
            {
                // Resume the "real" polygon mode:
                GLenum realRasterModeFront = _attribStack.topItem()._rasterModeFrontFace;
                gs_stat_realFunctionPointers.glPolygonMode(GL_FRONT_AND_BACK, realRasterModeFront);
            }

#endif

            // Mark the attrib stack item as "not forced":
            _attribStack.topItem()._isPolygonRasterModeForced = false;
        }
    }

    // Turn the flag off:
    _isModeForced[AP_OPENGL_FORCED_POLYGON_RASTER_MODE] = false;
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::applyForcedStubTextures
// Description: Applies the "Forces stub textures" mode.
// Author:      Yaki Tebeka
// Date:        15/4/2005
// ---------------------------------------------------------------------------
void gsForcedModesManager::applyForcedStubTextures()
{
    if (_pForcedRenderContextMonitor)
    {
        // If my context is not the NULL context:
        if (_pForcedRenderContextMonitor->spyId() != AP_NULL_CONTEXT_ID)
        {
            // Turn on "Force stub textures mode":
            _pForcedRenderContextMonitor->applyForcedStubTextureObjects();
        }
    }

    // Turn the flag on:
    _isModeForced[AP_OPENGL_FORCED_STUB_TEXTURES_MODE] = true;
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::cancelForcedStubTextures
// Description: Cancels the "Forces stub textures" mode.
// Author:      Yaki Tebeka
// Date:        15/4/2005
// ---------------------------------------------------------------------------
void gsForcedModesManager::cancelForcedStubTextures()
{
    if (_pForcedRenderContextMonitor)
    {
        // If my context is not the NULL context:
        if (_pForcedRenderContextMonitor->spyId() != AP_NULL_CONTEXT_ID)
        {
            // Cancel "Force stub textures mode":
            _pForcedRenderContextMonitor->cancelForcedStubTextureObjects();
        }
    }

    // Turn the flag off:
    _isModeForced[AP_OPENGL_FORCED_STUB_TEXTURES_MODE] = false;
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::applyForcedSinglePixelViewPort
// Description: Applies the "Forced single pixel view-port" mode.
// Author:      Yaki Tebeka
// Date:        13/3/2005
// ---------------------------------------------------------------------------
void gsForcedModesManager::applyForcedSinglePixelViewPort()
{
    if (_pForcedRenderContextMonitor)
    {
        // If my context is not the NULL context:
        if (_pForcedRenderContextMonitor->spyId() != AP_NULL_CONTEXT_ID)
        {
            // Get the current view port:
            const GLint* pCurrentViewPort = _attribStack.topItem()._viewPort;

            // Force a single pixel view port:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glViewport);
            gs_stat_realFunctionPointers.glViewport(pCurrentViewPort[0], pCurrentViewPort[1], 1, 1);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glViewport);

            // Mark the attrib stack item as "forced":
            _attribStack.topItem()._isSinglePixelViewPortForced = true;
        }
    }

    // Turn the flag on:
    _isModeForced[AP_OPENGL_FORCED_SINGLE_PIXEL_VIEW_PORT] = true;
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::cancelForcedSinglePixelViewPort
// Description: Cancels the "Forced single pixel view-port" mode.
// Author:      Yaki Tebeka
// Date:        13/3/2005
// ---------------------------------------------------------------------------
void gsForcedModesManager::cancelForcedSinglePixelViewPort()
{
    if (_pForcedRenderContextMonitor)
    {
        // If my context is not the NULL context:
        if (_pForcedRenderContextMonitor->spyId() != AP_NULL_CONTEXT_ID)
        {
            // Get the current view port:
            const GLint* pCurrentViewPort = _attribStack.topItem()._viewPort;

            // Resume the "real" view port:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glViewport);
            gs_stat_realFunctionPointers.glViewport(pCurrentViewPort[0], pCurrentViewPort[1],
                                                    pCurrentViewPort[2], pCurrentViewPort[3]);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glViewport);

            // Mark the attrib stack item as "not forced":
            _attribStack.topItem()._isSinglePixelViewPortForced = false;
        }
    }

    // Turn the flag off:
    _isModeForced[AP_OPENGL_FORCED_SINGLE_PIXEL_VIEW_PORT] = false;
}

// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::applyForcedStubGeometryShader
// Description: Applies the "Stub Geometry Shaders" mode.
// Author:      Uri Shomroni
// Date:        10/4/2008
// ---------------------------------------------------------------------------
void gsForcedModesManager::applyForcedStubGeometryShader()
{
    if (_pForcedRenderContextMonitor)
    {
        // If my context is not the NULL context:
        if (_pForcedRenderContextMonitor->spyId() != AP_NULL_CONTEXT_ID)
        {
            // Get the programs and shaders monitor:
            gsProgramsAndShadersMonitor* progAndShadersMon = _pForcedRenderContextMonitor->programsAndShadersMonitor();

            GT_IF_WITH_ASSERT(progAndShadersMon != NULL)
            {
#ifdef _GR_IPHONE_BUILD
                // In the iPhone Simulator, attaching and detaching shaders can cause some very odd problems,
                // so instead we replace the shaders' source code (and later restore it from our files:
                bool rc = progAndShadersMon->stubFragmentShaderSources();
                GT_ASSERT(rc);
#else

                // verify that we aren't in forced fragment shaders mode, as these two modes tend to collide:
                if (progAndShadersMon->areStubFragmentShadersForced())
                {
                    bool rcFrag = progAndShadersMon->cancelForcedStubFragmentShader();
                    GT_ASSERT(rcFrag);
                }

                // Ask it to apply the forced stub geometry shaders:
                bool rc = progAndShadersMon->applyForcedStubGeometryShader();
                GT_ASSERT(rc);

                // Restore the active program, which might have changed during the above operations:
                GLuint activeProgramName = _pForcedRenderContextMonitor->activeProgramName();
                progAndShadersMon->restoreActiveProgram(activeProgramName);
#endif
            }
        }
    }

    // Mark that the forced stub fragment shader was applied:
    _isModeForced[AP_OPENGL_FORCED_STUB_GEOMETRY_SHADERS_MODE] = true;
}

// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::cancelForcedStubGeometryShader
// Description: Cancels the "Stub Geometry Shader" mode.
// Author:      Uri Shomroni
// Date:        10/4/2008
// ---------------------------------------------------------------------------
void gsForcedModesManager::cancelForcedStubGeometryShader()
{
    if (_pForcedRenderContextMonitor)
    {
        // If my context is not the NULL context:
        if (_pForcedRenderContextMonitor->spyId() != AP_NULL_CONTEXT_ID)
        {
            // Get the programs and shaders monitor:
            gsProgramsAndShadersMonitor* progAndShadersMon = _pForcedRenderContextMonitor->programsAndShadersMonitor();

            GT_IF_WITH_ASSERT(progAndShadersMon != NULL)
            {
#ifdef _GR_IPHONE_BUILD
                // In the iPhone Simulator, attaching and detaching shaders can cause some very odd problems,
                // so instead we replace the shaders' source code (and later restore it from our files:
                bool rc = progAndShadersMon->restoreStubbedFragmentShaderSources();
                GT_ASSERT(rc);
#else
                // Ask it to resume the "real" geometry shaders:
                bool rc = progAndShadersMon->cancelForcedStubGeometryShader();

                // return the active program from forced FS mode (if there isn't one, do nothing):
                GLuint activeProgramName = _pForcedRenderContextMonitor->activeProgramName();

                if (progAndShadersMon->isProgramObject(activeProgramName))
                {
                    rc = rc && _pForcedRenderContextMonitor->afterProgramRestoredFromStubFS(activeProgramName);
                }

                GT_ASSERT(rc);
#endif
            }
        }
    }

    // Mark that the forced stub geometry shader was canceled:
    _isModeForced[AP_OPENGL_FORCED_STUB_GEOMETRY_SHADERS_MODE] = false;
}

// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::applyForcedStubFragmentShader
// Description: Applies the "Forced single pixel view-port" mode.
// Author:      Yaki Tebeka
// Date:        13/3/2005
// ---------------------------------------------------------------------------
void gsForcedModesManager::applyForcedStubFragmentShader()
{
    if (_pForcedRenderContextMonitor)
    {
        // If my context is not the NULL context:
        if (_pForcedRenderContextMonitor->spyId() != AP_NULL_CONTEXT_ID)
        {
            // Get the programs and shaders monitor:
            gsProgramsAndShadersMonitor* progAndShadersMon = _pForcedRenderContextMonitor->programsAndShadersMonitor();

            GT_IF_WITH_ASSERT(progAndShadersMon != NULL)
            {
#ifdef _GR_IPHONE_BUILD
                // In the iPhone Simulator, attaching and detaching shaders can cause some very odd problems,
                // so instead we replace the shaders' source code (and later restore it from our files:
                bool rc = progAndShadersMon->stubFragmentShaderSources();
                GT_ASSERT(rc);
#else

                // verify that we aren't in forced geometry shaders mode as these two modes tend to collide:
                if (progAndShadersMon->areStubGeometryShadersForced())
                {
                    bool rcGeom = progAndShadersMon->cancelForcedStubGeometryShader();
                    GT_ASSERT(rcGeom);
                }

                // Ask it to apply the forced stub fragment shaders:
                bool rc = progAndShadersMon->applyForcedStubFragmentShader();
                GT_ASSERT(rc);

                // Restore the active program, which might have changed during the above operations:
                GLuint activeProgramName = _pForcedRenderContextMonitor->activeProgramName();
                progAndShadersMon->restoreActiveProgram(activeProgramName);
#endif
            }
        }
    }

    // Mark that the forced stub fragment shader was applied:
    _isModeForced[AP_OPENGL_FORCED_STUB_FRAGMENT_SHADERS_MODE] = true;
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::cancelForcedStubFragmentShader
// Description: Cancels the "Forced single pixel view-port" mode.
// Author:      Yaki Tebeka
// Date:        13/3/2005
// ---------------------------------------------------------------------------
void gsForcedModesManager::cancelForcedStubFragmentShader()
{
    if (_pForcedRenderContextMonitor)
    {
        // If my context is not the NULL context:
        if (_pForcedRenderContextMonitor->spyId() != AP_NULL_CONTEXT_ID)
        {
            // Get the programs and shaders monitor:
            gsProgramsAndShadersMonitor* progAndShadersMon = _pForcedRenderContextMonitor->programsAndShadersMonitor();

            GT_IF_WITH_ASSERT(progAndShadersMon != NULL)
            {
#ifdef _GR_IPHONE_BUILD
                // In the iPhone Simulator, attaching and detaching shaders can cause some very odd problems,
                // so instead we replace the shaders' source code (and later restore it from our files:
                bool rc = progAndShadersMon->restoreStubbedFragmentShaderSources();
                GT_ASSERT(rc);
#else
                // Ask it to resume the "real" fragment shaders:
                bool rc = progAndShadersMon->cancelForcedStubFragmentShader();

                // return the active program from forced FS mode (if there isn't one, do nothing):
                GLuint activeProgramName = _pForcedRenderContextMonitor->activeProgramName();

                if (progAndShadersMon->isProgramObject(activeProgramName))
                {
                    rc = rc && _pForcedRenderContextMonitor->afterProgramRestoredFromStubFS(activeProgramName);
                }

                GT_ASSERT(rc);
#endif
            }
        }
    }

    // Mark that the forced stub fragment shader was canceled:
    _isModeForced[AP_OPENGL_FORCED_STUB_FRAGMENT_SHADERS_MODE] = false;
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::applyForcedNoLightsMode
// Description: Applies the "Forced no lights" mode.
// Author:      Yaki Tebeka
// Date:        13/3/2005
// ---------------------------------------------------------------------------
void gsForcedModesManager::applyForcedNoLightsMode()
{
    if (_pForcedRenderContextMonitor)
    {
        // If my context is not the NULL context:
        if (_pForcedRenderContextMonitor->spyId() != AP_NULL_CONTEXT_ID)
        {
            // Get the lights monitor:
            gsLightsMonitor& lightsMon = _pForcedRenderContextMonitor->lightsMonitor();

            // Ask it to apply the forced "no lights" mode:
            bool rc = lightsMon.applyForcedNoLightsMode();
            GT_ASSERT(rc);
        }
    }

    // Mark that the forced no lights mode was applied:
    _isModeForced[AP_OPENGL_FORCED_NO_LIGHTS_MODE] = true;
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::cancelForcedNoLightsMode
// Description: Cancels the "Forced no lights" mode.
// Author:      Yaki Tebeka
// Date:        13/3/2005
// ---------------------------------------------------------------------------
void gsForcedModesManager::cancelForcedNoLightsMode()
{
    if (_pForcedRenderContextMonitor)
    {
        // If my context is not the NULL context:
        if (_pForcedRenderContextMonitor->spyId() != AP_NULL_CONTEXT_ID)
        {
            // Get the lights monitor:
            gsLightsMonitor& lightsMon = _pForcedRenderContextMonitor->lightsMonitor();

            // Ask it to cancel the forced "no lights" mode:
            bool rc = lightsMon.cancelForcedNoLightsMode();
            GT_ASSERT(rc);
        }
    }

    // Mark that the forced no lights mode was canceled:
    _isModeForced[AP_OPENGL_FORCED_NO_LIGHTS_MODE] = false;
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::applyDebugOutputParameterChange
// Description: Applies a change in the parameters of the OpenGL debug output mechanism.
// Author:      Sigal Algranaty
// Date:        20/6/2010
// ---------------------------------------------------------------------------
void gsForcedModesManager::applyDebugOutputParameterChange()
{
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))

    _pForcedRenderContextMonitor->debugOutputManager().applyDebugOutputSettingsToContext();
#endif
}


// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::reapplyOrCancelForcedModes
// Description:
//   Re-applies the forced modes.
//   Is called right after the attribute stack is popped.
// Author:      Yaki Tebeka
// Date:        15/5/2005
// ---------------------------------------------------------------------------
void gsForcedModesManager::reapplyOrCancelForcedModes()
{
    // If raster mode is forced:
    if (isStubForced(AP_OPENGL_FORCED_POLYGON_RASTER_MODE))
    {
        // Re-apply the forced raster mode;
        applyForcedPolygonRasterMode();
    }
    else
    {
        // If the stack top item is forced:
        if (_attribStack.topItem()._isPolygonRasterModeForced)
        {
            // Cancel the forced polygon raster mode:
            cancelForcedPolygonRasterMode();
        }
    }

    // If front draw buffer is forced:
    if (isStubForced(AP_OPENGL_FORCED_FRONT_DRAW_BUFFER_MODE))
    {
        // Re-apply the forced draw buffer;
        applyForcedFrontDrawBuffer();
    }
    else
    {
        // If the stack top item is forced:
        if (_attribStack.topItem()._isDrawnBufferForced)
        {
            // Cancel the forced polygon raster mode:
            cancelForcedFrontDrawBuffer();
        }
    }

    // If single pixel view port is forced:
    if (isStubForced(AP_OPENGL_FORCED_SINGLE_PIXEL_VIEW_PORT))
    {
        // Re-apply the forced raster mode;
        applyForcedSinglePixelViewPort();
    }
    else
    {
        // If the stack top item is forced:
        if (_attribStack.topItem()._isSinglePixelViewPortForced)
        {
            // Cancel the forced polygon raster mode:
            cancelForcedSinglePixelViewPort();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsForcedModesManager::applyForceMode
// Description: Apply / Cancel force mode by its type
// Arguments:   apOpenGLForcedModeType forceModeEvent
//            bool shouldApply
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/4/2010
// ---------------------------------------------------------------------------
bool gsForcedModesManager::applyForceMode(apOpenGLForcedModeType forceModeEvent, bool shouldApply)
{
    bool retVal = true;

    switch (forceModeEvent)
    {
        case AP_OPENGL_FORCED_FRONT_DRAW_BUFFER_MODE:
            if (shouldApply)
            {
                applyForcedFrontDrawBuffer();
            }
            else
            {
                cancelForcedFrontDrawBuffer();
            }

            break;

        case AP_OPENGL_FORCED_POLYGON_RASTER_MODE:
            if (shouldApply)
            {
                applyForcedPolygonRasterMode();
            }
            else
            {
                cancelForcedPolygonRasterMode();
            }

            break;

        case AP_OPENGL_FORCED_STUB_TEXTURES_MODE:
            if (shouldApply)
            {
                applyForcedStubTextures();
            }
            else
            {
                cancelForcedStubTextures();
            }

            break;

        case AP_OPENGL_FORCED_SINGLE_PIXEL_VIEW_PORT:
            if (shouldApply)
            {
                applyForcedSinglePixelViewPort();
            }
            else
            {
                cancelForcedSinglePixelViewPort();
            }

            break;

        case AP_OPENGL_FORCED_STUB_FRAGMENT_SHADERS_MODE:
            if (shouldApply)
            {
                applyForcedStubFragmentShader();
            }
            else
            {
                cancelForcedStubFragmentShader();
            }

            break;

        case AP_OPENGL_FORCED_NO_LIGHTS_MODE:
            if (shouldApply)
            {
                applyForcedNoLightsMode();
            }
            else
            {
                cancelForcedNoLightsMode();
            }

            break;

        case AP_OPENGL_FORCED_STUB_GEOMETRY_SHADERS_MODE:
            if (shouldApply)
            {
                applyForcedStubGeometryShader();
            }
            else
            {
                cancelForcedStubGeometryShader();
            }

            break;

        case AP_OPENGL_DEBUG_OUTPUT_PARAMETER_CHANGED:
            applyDebugOutputParameterChange();
            break;

        default:
            GT_ASSERT_EX(false, L"Should not get here");
            retVal = false;
            break;
    }


    return retVal;
}

