//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuAffinityThread.h
/// \brief  A brief file description that Doxygen makes note of.
///
//==================================================================================

#ifndef _CPUAFFINITYTHREAD_H
#define _CPUAFFINITYTHREAD_H

//AMDTOsWrappers
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTCommonHeaders/AMDTCommonProfileDataTypes.h>

class CpuAffinityThread : public osThread
{
public:
    CpuAffinityThread(unsigned int core, AMDTCpuTopology* pSessionTopology);
    ~CpuAffinityThread();

    // Overrides osThread
    virtual int entryPoint();
    virtual void beforeTermination();

    bool processEnded();
private:
    // Disallow use of my default constructor:
    CpuAffinityThread();

    //The target core
    unsigned int m_core;

    // The topology of the core
    AMDTCpuTopology* m_pSessionTopology;
};
#endif //_CPUAFFINITYTHREAD_H