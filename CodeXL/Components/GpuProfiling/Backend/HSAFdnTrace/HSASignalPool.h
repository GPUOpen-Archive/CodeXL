//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains a signal pool class.
//==============================================================================

#ifndef _HSA_SIGNAL_POOL_H_
#define _HSA_SIGNAL_POOL_H_

#include <stack>

#include "hsa.h"

#include <AMDTMutex.h>
#include <TSingleton.h>

/// Singleton class to manage hsa signals (allows recycling of no-longer used
/// signals to avoid overhead of constantly creating/destroying signals)
class HSASignalPool : public TSingleton<HSASignalPool>
{
    friend class TSingleton<HSASignalPool>;

public:
    /// Destructor
    ~HSASignalPool();

    /// Get a signal that can be used for a kernel dispatch. Creates one if
    /// needed or uses an existing one from the pool
    /// \param initialValue the initial value for the signal
    /// \param signal the newly-created or recycled signal
    /// \return true if a signal is returned
    bool AcquireSignal(hsa_signal_value_t initialValue, hsa_signal_t& signal);

    /// Marks a signal as longer being used.  Makes it available in the pool
    /// for future dispatches
    /// \param signal the signal that is no longer needed.  It will be added
    ///        to the pool for future dispatches
    /// \return true if a signal is released
    bool ReleaseSignal(hsa_signal_t signal);

    /// Destroys all signals in the pool and clears the pool
    void Clear();

private:
    static const size_t s_MAX_POOL_SIZE = 100; ///< only keep 100 unused signals in the pool

    /// Constructor
    HSASignalPool();

    /// Remove copy constructor and assignment operator
    HSASignalPool(const HSASignalPool&) = delete;
    const HSASignalPool& operator=(const HSASignalPool&) = delete;

    std::stack<hsa_signal_t> m_signalPool;    ///< stack of created-and-no-longer-used signals
    AMDTMutex                m_signalPoolMtx; ///< mutex to protect access to m_signalPool
};

#endif // _HSA_SIGNAL_POOL_H_
