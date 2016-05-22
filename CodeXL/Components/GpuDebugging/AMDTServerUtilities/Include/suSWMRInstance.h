#pragma once
//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suSWMRInstance.h
///
//==================================================================================


#ifndef __SUSWMRINSTANCE_H
#define __SUSWMRINSTANCE_H

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// This function has about a 5% impact on OpenGL application performance, so we
// dont use it on Windows:
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
#define SU_USE_SINGLE_WRITE_MULTIPLE_READ_SYNC 1

#include <mutex>

////////////////////////////////////////////////////////////////////////////////////
/// \class suSWMRInstance
/// \brief Single Read - Multiple Write pattern implementation. 
///	Based on boost shared_mutex. Realized as singleton.
///
/// \author AMD Developer Tools Team
/// \date 11/05/2016
class suSWMRInstance
{
public:
    ////////////////////////////////////////////////////////////////////////////////////
    /// \brief "Multiple read" shared lock
    ///
    /// \author AMD Developer Tools Team
    /// \date 11/05/2016
    void SharedLock();

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
    static suSWMRInstance& GetInstance();

    ////////////////////////////////////////////////////////////////////////////////////
    /// \brief Standard destructor. 
    ///
    /// \author AMD Developer Tools Team
    /// \date 11/05/2016
    ~suSWMRInstance();

private:
    ////////////////////////////////////////////////////////////////////////////////////
    /// \brief Standard constructor. Hidden by private section
    ///
    /// \author AMD Developer Tools Team
    /// \date 11/05/2016
    suSWMRInstance();

    ////////////////////////////////////////////////////////////////////////////////////
    /// \brief Standard copy constructor. Hidden by private section
    ///
    /// \author AMD Developer Tools Team
    /// \date 11/05/2016
    suSWMRInstance(const suSWMRInstance& ) {};

    bool         bUniqLocked;
    std::mutex   mtxUniqLockedVariable;
};

#endif // AMDT_BUILD_TARGET == AMDT_LINUX_OS

#endif // __SUSWMRINSTANCE_H

