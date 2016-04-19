//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PerfPmuTarget.h
///
//==================================================================================

#ifndef _PERFPMUTARGET_H_
#define _PERFPMUTARGET_H_

// Standard headers
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <limits.h>
#include <wchar.h>

// C++ Headers
#include <vector>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

typedef std::vector<int> CACpuVec;
typedef std::vector<pid_t> CAThreadVec;

class PerfPmuTarget
{
public:
    PerfPmuTarget() : m_nbrPids(0), m_pPids(NULL), m_nbrCpus(0), m_pCpus(NULL), m_isSWP(false)
    {
    }

    // Dtor
    ~PerfPmuTarget();

    // Copy ctor
    PerfPmuTarget(const PerfPmuTarget& tgt);

    // Assignment operator
    PerfPmuTarget& operator= (const PerfPmuTarget& tgt);

    // specify target-pid(s) for per process mode
    // The cpus can be specified here,
    // if the user wants to profile
    // the target-pid(s) on some select CPUs.
    HRESULT setPids(size_t nbr, pid_t* pids, size_t nbrCpus = 0, int* cpus = NULL, bool isSWP = false);

    // specify cpu(s) for system wide mode
    HRESULT setCpus(size_t nbr, int* cpus);
    bool isSWP() const { return m_isSWP; }

    void getPids(size_t* nbr, int** pids) const
    {
        if (nbr) { *nbr = m_nbrPids; }

        if (pids) { *pids = m_pPids; }
    }

    void getCpus(size_t* nbr, int** cpus) const
    {
        if (nbr) { *nbr = m_nbrCpus; }

        if (cpus) { *cpus = m_pCpus; }
    }

    void print();

public:
    size_t  m_nbrPids;
    pid_t*  m_pPids;

    size_t  m_nbrCpus;
    int*    m_pCpus;

    bool    m_isSWP;
};

#endif // _PERFPMUTARGET_H_
