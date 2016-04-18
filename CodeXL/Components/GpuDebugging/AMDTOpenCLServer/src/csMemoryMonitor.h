//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csMemoryMonitor.h
///
//==================================================================================

//------------------------------ csMemoryMonitor.h ------------------------------

#ifndef __CSMEMORYMONITOR
#define __CSMEMORYMONITOR

// Forward declarations:
class apContextID;
class csContextMonitor;

// Infra:
#include <AMDTAPIClasses/Include/Events/apMemoryLeakEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           csMemoryMonitor
// General Description:  Is handling memory monitoring
// Author:               Sigal Algranaty
// Creation Date:        14/7/2010
// ----------------------------------------------------------------------------------
class csMemoryMonitor
{
public:
    static csMemoryMonitor& instance();
    ~csMemoryMonitor();

public:
    // Events:
    bool beforeSpyTermination();
    bool beforeContextDeletion(const apContextID& deleteContextID);
    bool beforeComputationProgramReleased(const apContextID& contextID, int programIndex);

private:
    // Memory leaks and related checks:
    void checkForMemoryLeaksOnOpenCLUnloaded();
    void checkForMemoryLeaksOnOpenCLContextReleased(const apContextID& deletedContextID);
    void checkForMemoryLeaksOnOpenCLProgramReleased(const apContextID& deletedProgramContextID, int deletedProgramIndex);
    bool calculateMemoryLeakForContext(const csContextMonitor* pContextMonitor, gtUInt64& contextMemorySize);
    bool calculateMemoryLeakForProgram(const csContextMonitor* pContextMonitor, int deletedProgramIndex, gtUInt64& contextMemorySize);

    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead:
    csMemoryMonitor& operator=(const csMemoryMonitor& otherMonitor);
    csMemoryMonitor(const csMemoryMonitor& otherMonitor);

    // Only my instance() method should create me:
    csMemoryMonitor();

private:
    // A pointer to this class single instance:
    static csMemoryMonitor* _pMySingleInstance;

    // Allow gsSingletonsDelete to clean up the single instance of this class.
    friend class gsSingletonsDelete;
};


#endif  // __CSMEMORYMONITOR
