//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsLightsMonitor.h
///
//==================================================================================

//------------------------------ gsLightsMonitor.h ------------------------------

#ifndef __GSLIGHTSMONITOR
#define __GSLIGHTSMONITOR

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsLightsMonitor
// General Description:
//   Monitors OpenGL Programs, Vertex Shaders and Fragment Shaders.
//
// Author:               Yaki Tebeka
// Creation Date:        04/04/2004
// ----------------------------------------------------------------------------------
class gsLightsMonitor
{
public:
    gsLightsMonitor();
    ~gsLightsMonitor();

    // On event functions:
    void onFirstTimeContextMadeCurrent();
    bool onLightTurnedOn(GLenum lightId);
    bool onLightTurnedOff(GLenum lightId);

    // Query data functions:
    int maxLights() const { return _maxLights; };
    bool isLightOn(GLenum lightId) const;

    // Forced modes:
    bool applyForcedNoLightsMode();
    bool cancelForcedNoLightsMode();

private:
    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead
    gsLightsMonitor& operator=(const gsLightsMonitor& otherMonitor);
    gsLightsMonitor(const gsLightsMonitor& otherMonitor);

    int lightVecIndex(GLenum lightId) const;
    GLenum lightOpenGLId(int vecIndex) const;

private:
    // Contains the maximum number of lights, supported by the current render context:
    GLint _maxLights;

    // _isLightOn[i] contains true iff light i is currently on:
    gtVector<bool> _isLightOn;

    // Contains true iff "force no lights" mode is on:
    bool _isNoLightsModeForced;
};


#endif  // __GSLIGHTSMONITOR
