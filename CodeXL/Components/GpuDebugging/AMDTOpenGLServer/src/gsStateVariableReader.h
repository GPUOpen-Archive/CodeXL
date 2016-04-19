//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsStateVariableReader.h
///
//==================================================================================

//------------------------------ gsStateVariableReader.h ------------------------------

#ifndef __GSSTATEVARIABLEREADER_H
#define __GSSTATEVARIABLEREADER_H

// Infra:
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTAPIClasses/Include/apOpenGLParameters.h>

// Local:
#include <src/gsRenderContextMonitor.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsStateVariableReader
// General Description:
//   Reads the value of an OpenGL state variable from the CURRENT OpenGL context.
//
// Author:               Yaki Tebeka
// Creation Date:        31/7/2004
// ----------------------------------------------------------------------------------
class gsStateVariableReader
{
public:
    gsStateVariableReader(int stateVariableId, const gsRenderContextMonitor* pRenderContextMtr);
    bool getStateVariableValue(apParameter*& pStateVariableValue);

private:
    bool getEnumStateVariableValue(apParameter*& pStateVariableValue);
    bool getIsEnabledStateVariable(apParameter*& pStateVariableValue);
    bool getBooleanStateVariableValue(apParameter*& pStateVariableValue);
    bool getIntegerStateVariableValue(apParameter*& pStateVariableValue);
    bool getInteger64StateVariableValue(apParameter*& pStateVariableValue);
    bool getIntegerui64NVStateVariableValue(apParameter*& pStateVariableValue);
    bool getFloatStateVariableValue(apParameter*& pStateVariableValue);
    bool getDoubleStateVariableValue(apParameter*& pStateVariableValue);
    bool getMaterialVariableValue(apParameter*& pStateVariableValue);
    bool getLightVariableValue(apParameter*& pStateVariableValue);
    bool getClipPlaneVariableValue(apParameter*& pStateVariableValue);
    bool getTexEnvVariableValue(apParameter*& pStateVariableValue);
    bool getTexGenVariableValue(apParameter*& pStateVariableValue);
    bool getPointerVariableValue(apParameter*& pStateVariableValue);
    bool getQueryIntVariableValue(apParameter*& pStateVariableValue);
    bool getBufferParameterARBIntValue(apParameter*& pStateVariableValue);
    bool getBufferParameterIntValue(apParameter*& pStateVariableValue);
    bool getStringVariableValue(apParameter*& pStateVariableValue);
    bool getProgramIntVariableValue(apParameter*& pStateVariableValue);
    bool getHandleVariableValue(apParameter*& pStateVariableValue);

    void beforeGettingStateVariableValue();
    void afterGettingStateVariableValue(bool retVal, GLenum openGLError);

    // Mac only:
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
#ifdef _GR_OPENGLES_IPHONE
    // iPhone build: Yaki 31/5/2009 - Currently, EAGL does not define any OpenGL ES state variables.
#else
    bool getCGLStateVariableVariableValue(apParameter*& pStateVariableValue, apMonitoredFunctionId getFunctionId);
#endif
#endif

    // Windows only:
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    bool getWGLSwapInterval(apParameter*& pStateVariableValue);
#endif

    // Do not allow the use of my default constructor:
    gsStateVariableReader();

private:
    // The state variable id:
    int _stateVariableId;

    // Render context monitor handle:
    const gsRenderContextMonitor* _pMyRenderContextMtr;

};


#endif //__GSSTATEVARIABLEREADER_H
