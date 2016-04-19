//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuAffinityThread.h
/// \brief  A brief file description that Doxygen makes note of.
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/CpuAffinityThread.h#6 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================
#ifndef _CPUAFFINITYTHREAD_H
#define _CPUAFFINITYTHREAD_H

//AMDTOsWrappers
#include <AMDTOSWrappers/Include/osThread.h>

#include <AMDTCpuProfilingRawData/inc/CpuProfileInfo.h>

class CpuAffinityThread : public osThread
{
public:
    CpuAffinityThread(unsigned int core, CoreTopology* pSessionTopology);
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
    CoreTopology* m_pSessionTopology;
};
#endif //_CPUAFFINITYTHREAD_H
