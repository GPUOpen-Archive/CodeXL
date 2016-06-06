//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCounterScope.h
///
//==================================================================================

//------------------------------ apCounterScope.h ------------------------------

#ifndef __APCOUNTERSCOPE
#define __APCOUNTERSCOPE

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>
#include <AMDTAPIClasses/Include/apContextID.h>



// ----------------------------------------------------------------------------------
// Class Name:           apCounterScope : public osTransferableObject
// General Description: Holds a counter scope
//                      A Counter scope is the counter provider.
//                      Currently there are 3 counter scopes:
//                      1. Global
//                      2. OpenGL Context
//                      3. OpenCL queue
// Author:  AMD Developer Tools Team
// Creation Date:        3/3/2010
// ----------------------------------------------------------------------------------
class AP_API apCounterScope : public osTransferableObject
{
public:

    enum apCounterScopeType
    {
        AP_GLOBAL_COUNTER,
        AP_CONTEXT_COUNTER,
        AP_QUEUE_COUNTER
    };
    // Constructor / destructor:
    apCounterScope(apCounterScopeType counterScopeType = AP_GLOBAL_COUNTER);
    apCounterScope(apContextID contextID);
    apCounterScope(int contextId, int queueId);
    apCounterScope(int contextId);
    virtual ~apCounterScope();

    // Return true iff the scope is either OpenCL default queue or OpenGL default context:
    bool isDefault() const;

    // Static default scopes:
    static apCounterScope defaultGLContextScope() {return _stat_defaultGLContextScope;};
    static apCounterScope defaultCLQueueScope() {return _stat_defaultCLQueueScope;};
    static apCounterScope defaultGlobalScope() {return _stat_defaultGlobalScope;};

    // Translate the counter scope to a string;
    void toString(gtString& contextStr);

    // To enable the usage of apContextId as a map key:
    bool operator<(const apCounterScope& other) const;

    // The counter scope type:
    apCounterScopeType _counterScopeType;

    // The context ID:
    apContextID _contextID;

    // The queue id:
    int _queueId;
    int displayQueueId() const {return _queueId + 1;}

    bool operator==(const apCounterScope& other)const ;
    bool operator!=(const apCounterScope& other)const ;
    apCounterScope& operator=(const apCounterScope& other);

public:

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:

    static apCounterScope _stat_defaultGLContextScope;
    static apCounterScope _stat_defaultGLNoContextScope;
    static apCounterScope _stat_defaultGlobalScope;
    static apCounterScope _stat_defaultCLQueueScope;
    static apCounterScope _stat_defaultCLNoContextScope;
};

#endif  // __APCOUNTERSCOPE
