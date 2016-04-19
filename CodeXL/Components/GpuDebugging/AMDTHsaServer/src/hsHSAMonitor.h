//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file hsHSAMonitor.h
///
//==================================================================================

#ifndef __HSHSAMONITOR_H
#define __HSHSAMONITOR_H

// Infra:
#include <AMDTServerUtilities/Include/suITechnologyMonitor.h>

class hsHSAMonitor : public suITechnologyMonitor
{
public:
    static hsHSAMonitor& instance();
    void onDebuggedProcessTerminationAlert() override;
    void beforeDebuggedProcessSuspended() override;
    void afterDebuggedProcessResumed() override;
    void beforeBreakpointException(bool isInOpenGLBeginEndBlock) override;
    void afterBreakpointException(bool isInOpenGLBeginEndBlock) override;
    void onDebuggedProcessExecutionModeChanged(apExecutionMode newExecutionMode) override;

private:
    hsHSAMonitor();
    ~hsHSAMonitor();
    friend class hsSingletonsDelete;

private:

    static hsHSAMonitor* ms_pMySingleInstance;
};

#endif // __HSHSAMONITOR_H
