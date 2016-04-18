//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PerfPmuTarget.cpp
///
//==================================================================================

// Standard headers
#include <string.h>
#include <stdlib.h>

// Project headers
#include "PerfPmuTarget.h"
#include <AMDTOSWrappers/Include/osDebugLog.h>

//
//  class PerfPmuTarget
//

PerfPmuTarget::~PerfPmuTarget()
{
    if (m_pPids)
    {
        delete[] m_pPids;
        m_pPids = NULL;
    }

    if (m_pCpus)
    {
        delete[] m_pCpus;
        m_pCpus = NULL;
    }
}

// Copy Constructor
PerfPmuTarget::PerfPmuTarget(const PerfPmuTarget& tgt)
{
    m_nbrPids = tgt.m_nbrPids;
    m_nbrCpus = tgt.m_nbrCpus;
    m_isSWP   = tgt.m_isSWP;

    if (0 != m_nbrPids)
    {
        m_pPids = new pid_t[m_nbrPids];
        memcpy(m_pPids, tgt.m_pPids, m_nbrPids * sizeof(pid_t));
    }

    if (0 != m_nbrCpus)
    {
        m_pCpus = new int[m_nbrCpus];
        memcpy(m_pCpus, tgt.m_pCpus, m_nbrCpus * sizeof(int));
    }
}

// Assignment operator
PerfPmuTarget& PerfPmuTarget::operator= (const PerfPmuTarget& tgt)
{
    if (this != &tgt)
    {
        m_nbrPids = tgt.m_nbrPids;
        m_nbrCpus = tgt.m_nbrCpus;
        m_isSWP   = tgt.m_isSWP;

        if (NULL != m_pPids)
        {
            delete[] m_pPids;
            m_pPids = NULL;
        }

        if (NULL != m_pCpus)
        {
            delete[] m_pCpus;
            m_pCpus = NULL;
        }

        if (0 != m_nbrPids)
        {
            m_pPids = new pid_t[m_nbrPids];
            memcpy(m_pPids, tgt.m_pPids, m_nbrPids * sizeof(pid_t));
        }

        if (0 != m_nbrCpus)
        {
            m_pCpus = new int[m_nbrCpus];
            memcpy(m_pCpus, tgt.m_pCpus, m_nbrCpus * sizeof(int));
        }
    }

    return *this;
}

HRESULT PerfPmuTarget::setPids(size_t nbr, pid_t* pids, size_t nbrCpus, int* cpus, bool isSWP)
{
    HRESULT ret = S_OK;

    if (nbr && pids)
    {
        m_nbrPids = nbr;
        m_pPids = new pid_t[nbr];

        memcpy(m_pPids, pids, nbr * sizeof(pid_t));
    }


    if (nbrCpus && cpus)
    {
        m_nbrCpus = nbrCpus;
        m_pCpus = new int[nbrCpus];

        memcpy(m_pCpus, cpus, nbrCpus * sizeof(int));
    }

    m_isSWP = isSWP;

    return ret;
}

HRESULT PerfPmuTarget::setCpus(size_t nbr, int* cpus)
{
    return setPids(0, NULL, nbr, cpus, true);
}

void PerfPmuTarget::print()
{
    OS_OUTPUT_DEBUG_LOG(L"PMU Target Info:-", OS_DEBUG_LOG_INFO);

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Msmt : %ls", (m_isSWP ? L"SWP" : L"Per Process"));

    if (m_isSWP)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Nbr CPUs : %d\nCPUs : ", m_nbrCpus);

        for (size_t j = 0; j < m_nbrCpus; j++)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L" %d, ", m_pCpus[j]);
        }
    }
    else
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"Nbr Processes : %d\nPIDs : ", m_nbrPids);

        for (size_t j = 0; j < m_nbrPids; j++)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L" %d, ", m_pPids[j]);
        }
    }

    OS_OUTPUT_DEBUG_LOG(L"\n", OS_DEBUG_LOG_INFO);
}
