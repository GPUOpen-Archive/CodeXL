//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsStateVariablesSnapshot.h
///
//==================================================================================

//------------------------------ gsStateVariablesSnapshot.h ------------------------------

#ifndef __GSSTATEVARIABLESSNAPSHOT
#define __GSSTATEVARIABLESSNAPSHOT

// Pre-declarations:
class apParameter;
class gsRenderContextMonitor;

// Infra:
#include <AMDTAPIClasses/Include/apOpenGLStateVariableId.h>
#include <AMDTAPIClasses/Include/apStateVariablesSnapShot.h>


// ----------------------------------------------------------------------------------
// Class Name:   gsStateVariablesSnapshot
// General Description:
//   Holds a snapshot of an OpenGL render context state variables.
//
// Author:               Yaki Tebeka
// Creation Date:        17/7/2004
// ----------------------------------------------------------------------------------
class gsStateVariablesSnapshot: public apStateVariablesSnapShot
{
public:
    gsStateVariablesSnapshot();
    ~gsStateVariablesSnapshot();
    void onFirstTimeContextMadeCurrent(const gsRenderContextMonitor& myRenderContextMtr);

    bool updateContextDataSnapshot();

    bool supportOnlyFilteredStateVariableIds(const gtVector<apOpenGLStateVariableId>* pVectorOfFilteredStateVariables);
    static unsigned int getValidStateVariableTypesMask(bool withWindowingSystem, const gsRenderContextMonitor* pMyRenderContextMonitor);

private:

    void updateOGLStandardStateVariablesSupport(const gsRenderContextMonitor& myRenderContextMtr);
    void updateOGLESStandardStateVariablesSupport(const gsRenderContextMonitor& myRenderContextMtr);
    void updateExtensionsStateVariablesSupport(const gsRenderContextMonitor& myRenderContextMtr);
    bool updateOnlyFilteredStateVariables();
    bool updateAllStateVariables();

    // Render context monitor handle:
    const gsRenderContextMonitor* _pMyRenderContextMtr;

};


#endif  // __GSSTATEVARIABLESSNAPSHOT
