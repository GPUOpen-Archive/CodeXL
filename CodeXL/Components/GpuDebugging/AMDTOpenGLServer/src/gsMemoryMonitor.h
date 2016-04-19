//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsMemoryMonitor.h
///
//==================================================================================

//------------------------------ gsMemoryMonitor.h ------------------------------

#ifndef __GSMEMORYMONITOR
#define __GSMEMORYMONITOR

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTAPIClasses/Include/Events/apMemoryLeakEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsMemoryMonitor
// General Description:  Is handling memory monitoring
// Author:               Sigal Algranaty
// Creation Date:        14/7/2010
// ----------------------------------------------------------------------------------
class gsMemoryMonitor
{
public:

    static gsMemoryMonitor& instance();
    ~gsMemoryMonitor();

public:

    // Events:
    void beforeSpyTermination();
    bool beforeContextDeletion(apContextID deleteContextID);

private:

    // Memory leaks and related checks:
    void checkForMemoryLeaksOnBreak(apBreakReason breakReason, bool& debuggedProcessHasGraphicMemoryLeaks, bool& breakpointIsMemoryRelated, apMemoryLeakEvent& leakEve, gtString& memoryLeakMessage);
    void checkForMemoryLeaksOnOpenGLUnloaded();
    void checkForMemoryLeaksOnOpenGLContextResourcesDeleted(const apContextID& deletedContextID);

    bool calculateMemoryLeakForContext(const gsRenderContextMonitor* pRenderContextMonitor, gtUInt64& contextMemorySize);

    bool prepareContextDataForMemoryDetection(apContextID deletedContextID);
    bool updateTextureParameters(gsRenderContextMonitor* pRenderContextMonitor);

    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead:
    gsMemoryMonitor& operator=(const gsMemoryMonitor& otherMonitor);
    gsMemoryMonitor(const gsMemoryMonitor& otherMonitor);

    // Only my instance() method should create me:
    gsMemoryMonitor();

private:
    // A pointer to this class single instance:
    static gsMemoryMonitor* _pMySingleInstance;

    // Allow gsSingletonsDelete to clean up the single instance of this class.
    friend class gsSingletonsDelete;
};


#endif  // __GSMEMORYMONITOR
