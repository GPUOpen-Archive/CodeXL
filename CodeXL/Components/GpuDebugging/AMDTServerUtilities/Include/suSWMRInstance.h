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

#endif // __SUSWMRINSTANCE_H

