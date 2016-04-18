//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsForcedModesManager.h
///
//==================================================================================

//------------------------------ gsForcedModesManager.h ------------------------------

#ifndef __GSFORCEDMODESMANAGER
#define __GSFORCEDMODESMANAGER

// Predeclerations:
class gsRenderContextMonitor;

// Infra:
#include <AMDTAPIClasses/Include/apApplicationModesEventsType.h>
#include <AMDTAPIClasses/Include/apGLDebugOutput.h>
#include <AMDTAPIClasses/Include/apRasterMode.h>

// Local:
#include <src/gsAttribStack.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsForcedModesManager
// General Description:
//   Manages, applies and removed forces modes.
// Author:               Yaki Tebeka
// Creation Date:        14/4/2005
// ----------------------------------------------------------------------------------
class gsForcedModesManager
{
public:
    gsForcedModesManager();
    ~gsForcedModesManager();

public:
    // Events:
    void onFirstTimeContextMadeCurrent(gsRenderContextMonitor& forcedRenderContextMonitor);
    void onFrameTerminatorCall();
    void onViewPortSet(GLint x, GLint y, GLsizei width, GLsizei height);
    void onSetDrawBuffersCall(GLenum drawnBuffers);
    void onPolygonRasterModeSet(GLenum face, GLenum rasterMode);
    void onPushAttribCalled(GLbitfield mask);
    void onPopAttribCalled();

    // Force / remove forced modes:
    void forceStub(apOpenGLForcedModeType stubType, bool isStubForced);
    void forcePolygonRasterMode(apRasterMode rasterMode);
    void cancelPolygonRasterModeForcing();

    // Query "is mode on":
    bool isStubForced(apOpenGLForcedModeType stubType) const { return _isModeForced[stubType]; };
    bool areStubShadersForced() const { return (_isModeForced[AP_OPENGL_FORCED_STUB_FRAGMENT_SHADERS_MODE] || _isModeForced[AP_OPENGL_FORCED_STUB_GEOMETRY_SHADERS_MODE]); };

    // OpenGL debug output:
    void setGLDebugOutputLoggingEnabled(bool enabled) { m_debugOutputLoggingEnabled = enabled; };
    bool isDebugOutputLoggingEnabled() const { return m_debugOutputLoggingEnabled; };
    void setGLDebugOutputSeverityEnabled(apGLDebugOutputSeverity severity, bool enabled);
    const bool* debugOutputLoggedSeverities() const { return m_debugOutputSeverityEnabled; };
    void setGLDebugOutputKindMask(const gtUInt64& mask);
    const bool* debugOutputLoggedMessageKinds() const { return m_debugOutputKindsEnabled; };

    // Misc:
    const GLint* realViewPort() const { return _attribStack.topItem()._viewPort; };
    apRasterMode forcedPolygonRasterMode() const { return _forcedPolygonRasterMode; };
    void applyPendingForcedModesEvents();
    void copyForcedModesFromOtherManager(const gsForcedModesManager& otherMgr);

private:
    // Help function for all the forced modes:
    bool applyForceMode(apOpenGLForcedModeType forceModeEvent, bool shouldApply);

    // Force front buffer draw mode:
    void applyForcedFrontDrawBuffer();
    void cancelForcedFrontDrawBuffer();

    // Force polygon raster mode:
    void applyForcedPolygonRasterMode();
    void cancelForcedPolygonRasterMode();

    // Force stub textures mode:
    void applyForcedStubTextures();
    void cancelForcedStubTextures();

    // Force single pixel view-port mode:
    void applyForcedSinglePixelViewPort();
    void cancelForcedSinglePixelViewPort();

    // Forced stub fragment shader:
    void applyForcedStubGeometryShader();
    void cancelForcedStubGeometryShader();

    // Forced stub fragment shader:
    void applyForcedStubFragmentShader();
    void cancelForcedStubFragmentShader();

    // Forced no lights mode:
    void applyForcedNoLightsMode();
    void cancelForcedNoLightsMode();

    // Debug Output parameter change:
    void applyDebugOutputParameterChange();

    // Misc:
    void reapplyOrCancelForcedModes();

private:
    // The render context monitor on which I apply the forced modes:
    gsRenderContextMonitor* _pForcedRenderContextMonitor;

    // A stack that imitates the OpenGL attribute stack.
    // Holds the "real" (not forced) attributes
    gsAttribStack _attribStack;

    // Are forced modes "on":
    bool _isModeForced[AP_OPENGL_AMOUNT_OF_FORCED_STUBS];

    // Are forced modes "on" pending values:
    bool _isPendingModeForced[AP_OPENGL_AMOUNT_OF_FORCED_STUBS];

    // Contains the forced polygon raster mode:
    apRasterMode _forcedPolygonRasterMode;

    // Contains the pending forced polygon raster mode:
    apRasterMode _pendingForcedPolygonRasterMode;

    // Contains true iff glPolygonMode can be called with GL_FRONT and GL_BACK separately:
    bool _separatePolygonModesSupported;

    // OpenGL debug output:
    bool m_debugOutputLoggingEnabled;
    bool m_debugOutputSeverityEnabled[AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES];
    bool m_debugOutputKindsEnabled[AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES * AP_NUMBER_OF_DEBUG_OUTPUT_TYPES];
};

#endif  // __GSFORCEDMODESMANAGER
