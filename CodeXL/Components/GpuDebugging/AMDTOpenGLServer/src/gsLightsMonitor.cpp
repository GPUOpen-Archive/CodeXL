//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsLightsMonitor.cpp
///
//==================================================================================

//------------------------------ gsLightsMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsWrappersCommon.h>
#include <src/gsLightsMonitor.h>
#include <src/gsRenderContextMonitor.h>
#include <src/gsOpenGLMonitor.h>

// An index that represents "no index":
#define GS_NO_INDEX -1


// ---------------------------------------------------------------------------
// Name:        gsLightsMonitor::gsLightsMonitor
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        13/4/2005
// ---------------------------------------------------------------------------
gsLightsMonitor::gsLightsMonitor()
    : _maxLights(0), _isNoLightsModeForced(false)
{
}


// ---------------------------------------------------------------------------
// Name:        gsLightsMonitor::~gsLightsMonitor
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        13/4/2005
// ---------------------------------------------------------------------------
gsLightsMonitor::~gsLightsMonitor()
{
}


// ---------------------------------------------------------------------------
// Name:        gsLightsMonitor::onFirstTimeContextMadeCurrent
// Description: Is called at the first time that my render context is made current.
// Author:      Yaki Tebeka
// Date:        13/4/2005
// ---------------------------------------------------------------------------
void gsLightsMonitor::onFirstTimeContextMadeCurrent()
{
    // Get the maximal amount of lights supported by my render context:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
    gs_stat_realFunctionPointers.glGetIntegerv(GL_MAX_LIGHTS, &_maxLights);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);

    if (_maxLights > 0)
    {
        // Resize this class vectors:
        for (int i = 0; i < _maxLights; i++)
        {
            _isLightOn.push_back(false);
        }
    }
    else
    {
        // No lights support !!
#ifdef _GR_IPHONE_BUILD
        // Lights are not supported in OpenGL ES 2.0 and higher:
        gsRenderContextMonitor* pCurrentThreadContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();
        GT_IF_WITH_ASSERT(pCurrentThreadContextMonitor != NULL)
        {
            int oglesMajorVersion = 0;
            int oglesMinorVersion = 0;
            pCurrentThreadContextMonitor->getOpenGLVersion(oglesMajorVersion, oglesMinorVersion);
            GT_ASSERT(oglesMajorVersion >= 2);
        }
#else
        GT_ASSERT(false);
#endif
    }
}


// ---------------------------------------------------------------------------
// Name:        gsLightsMonitor::onLightTurnedOn
// Description: Is called when a light is turned on.
// Arguments:   lightId - The id of the light that was turned on.
// Return Val:  bool - Success / failure. Failure occur when turning on a
//                     light index that does not exist.
// Author:      Yaki Tebeka
// Date:        14/4/2005
// ---------------------------------------------------------------------------
bool gsLightsMonitor::onLightTurnedOn(GLenum lightId)
{
    bool retVal = false;

    // Get the light index of our _isLightOn vector:
    int vecIndex = lightVecIndex(lightId);

    if (vecIndex != GS_NO_INDEX)
    {
        // Mark that the light was turned on:
        _isLightOn[vecIndex] = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsLightsMonitor::onLightTurnedOff
// Description: Is called when a light is turned off.
// Arguments:   lightId - The id of the light that was turned off.
// Return Val:  bool - Success / failure. Failure occur when turning off a
//                     light index that does not exist.
// Author:      Yaki Tebeka
// Date:        14/4/2005
// ---------------------------------------------------------------------------
bool gsLightsMonitor::onLightTurnedOff(GLenum lightId)
{
    bool retVal = false;

    // Get the light index of our _isLightOn vector:
    int vecIndex = lightVecIndex(lightId);

    if (vecIndex != GS_NO_INDEX)
    {
        // Mark that the light was turned off:
        _isLightOn[vecIndex] = false;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsLightsMonitor::isLightOn
// Description: Inputs an OpenGL light id and returns true iff it is turned on.
// Author:      Yaki Tebeka
// Date:        14/4/2005
// ---------------------------------------------------------------------------
bool gsLightsMonitor::isLightOn(GLenum lightId) const
{
    bool retVal = false;

    // Get the light index of our _isLightOn vector:
    int vecIndex = lightVecIndex(lightId);

    if (vecIndex != GS_NO_INDEX)
    {
        retVal = _isLightOn[vecIndex];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsLightsMonitor::applyForcedNoLightsMode
// Description: Applies the "Forced No lights" mode.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        14/4/2005
// ---------------------------------------------------------------------------
bool gsLightsMonitor::applyForcedNoLightsMode()
{
    bool retVal = true;

    if (!_isNoLightsModeForced)
    {
        // Iterate the supported lights:
        for (int i = 0; i < _maxLights; i++)
        {
            // If the current light is turned on - turn it off:
            if (_isLightOn[i])
            {
                // Get the light OpenGL id:
                GLenum lightOGLId = lightOpenGLId(i);

                if (lightOGLId != GL_FALSE)
                {
                    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDisable);
                    gs_stat_realFunctionPointers.glDisable(lightOGLId);
                    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDisable);
                }
                else
                {
                    GT_ASSERT(0);
                }
            }
        }

        _isNoLightsModeForced = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsLightsMonitor::cancelForcedNoLightsMode
// Description: Cancels the "Forced no lights" mode.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        14/4/2005
// ---------------------------------------------------------------------------
bool gsLightsMonitor::cancelForcedNoLightsMode()
{
    bool retVal = true;

    if (_isNoLightsModeForced)
    {
        // Iterate the supported lights:
        for (int i = 0; i < _maxLights; i++)
        {
            // If the current light should be turned on - turn it on:
            if (_isLightOn[i])
            {
                // Get the light OpenGL id:
                GLenum lightOGLId = lightOpenGLId(i);

                if (lightOGLId != GL_FALSE)
                {
                    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glEnable);
                    gs_stat_realFunctionPointers.glEnable(lightOGLId);
                    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glEnable);
                }
                else
                {
                    GT_ASSERT(0);
                }
            }
        }

        _isNoLightsModeForced = false;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsLightsMonitor::lightVecIndex
// Description: Inputs an OpenGL light id (GL_LIGHTi) and returns its
//              _isLightOn index, or GS_NO_INDEX
//              if it is an invalid light id.
// Author:      Yaki Tebeka
// Date:        14/4/2005
// ---------------------------------------------------------------------------
int gsLightsMonitor::lightVecIndex(GLenum lightId) const
{
    int retVal = GS_NO_INDEX;

    // Get the light index in our _isLightOn vector:
    int vecIndex = lightId - GL_LIGHT0;

    // Sanity check:
    if ((0 <= vecIndex) && (vecIndex < _maxLights))
    {
        retVal = vecIndex;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsLightsMonitor::lightOpenGLId
// Description: Inputs an _isLightOn vector index and returns the OpenGL light id
//              (GL_LIGHTi), or GL_FALSE if the input vex index is invalid.
// Author:      Yaki Tebeka
// Date:        14/4/2005
// ---------------------------------------------------------------------------
GLenum gsLightsMonitor::lightOpenGLId(int vecIndex) const
{
    GLenum retVal = GL_FALSE;

    // Sanity check:
    if ((0 <= vecIndex) && (vecIndex < _maxLights))
    {
        retVal = GL_LIGHT0 + vecIndex;
    }

    return retVal;
}
