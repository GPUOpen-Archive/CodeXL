#pragma once
//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suSWMRImpl.h
///
//==================================================================================


#ifndef __SUSWMRIMPL_H
#define __SUSWMRIMPL_H

/// Boost
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

/// Stl
#include <mutex>

////////////////////////////////////////////////////////////////////////////////////
/// \class suSWMRImpl
/// \brief Single Read - Multiple Write pattern implementation. 
///	Based on boost shared_mutex. Realized as singleton.
///
/// \author AMD Developer Tools Team
/// \date 11/05/2016
class suSWMRImpl
{
public:
    ////////////////////////////////////////////////////////////////////////////////////
    /// \brief "Multiple read" shared lock
    ///
    /// \author AMD Developer Tools Team
    /// \date 11/05/2016
    void SharedLock();

    ////////////////////////////////////////////////////////////////////////////////////
    /// \brief "Multiple read" shared unlock
    ///
    /// \author AMD Developer Tools Team
    /// \date 11/05/2016
    void SharedUnLock();

    ////////////////////////////////////////////////////////////////////////////////////
    /// \brief "Single write" unique lock
    ///
    /// \author AMD Developer Tools Team
    /// \date 11/05/2016
    void UniqueLock();

    ////////////////////////////////////////////////////////////////////////////////////
    /// \brief "Single write" unique unlock
    ///
    /// \author AMD Developer Tools Team
    /// \date 11/05/2016
    void UniqueUnLock();

    ////////////////////////////////////////////////////////////////////////////////////
    /// \brief Get singleton instance
    ///
    /// \return Reference to the suSWMRInstance 
    ///
    /// \author AMD Developer Tools Team
    /// \date 11/05/2016
    static suSWMRImpl& GetInstance();

    ////////////////////////////////////////////////////////////////////////////////////
    /// \brief Standard destructor. 
    ///
    /// \author AMD Developer Tools Team
    /// \date 11/05/2016
    ~suSWMRImpl();

private:
    ////////////////////////////////////////////////////////////////////////////////////
    /// \brief Standard constructor. Hidden by private section
    ///
    /// \author AMD Developer Tools Team
    /// \date 11/05/2016
    suSWMRImpl();

    ////////////////////////////////////////////////////////////////////////////////////
    /// \brief Standard copy constructor. Hidden by private section
    ///
    /// \author AMD Developer Tools Team
    /// \date 11/05/2016
    suSWMRImpl(const suSWMRImpl& ) {};

    bool                            m_bUniqLocked;           ///! True in case resource was uniq locked already and false vice versa.
                                                             ///! Uses for prevent to unlock the unique locked shared mutex without lock him previously.
                                                             ///! Call the "shared_unlock" on unlocked shared mutex leading to boost::exception

    std::mutex                      m_mtxUniqLockedVariable; ///! The bUniqLocked flax synchronization object

    boost::shared_mutex             m_mtxShared;             ///! Boost shared mutex instance.
};

#endif // __SUSWMRIMPL_H

