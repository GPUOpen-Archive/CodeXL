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


////////////////////////////////////////////////////////////////////////////////////
/// \class suSWMRInstance
/// \brief Single Read - Multiple Write pattern stub. 
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
    static void SharedLock();

    ////////////////////////////////////////////////////////////////////////////////////
    /// \brief "Multiple read" shared unlock
    ///
    /// \author AMD Developer Tools Team
    /// \date 11/05/2016
    static void SharedUnLock();

    ////////////////////////////////////////////////////////////////////////////////////
    /// \brief "Single write" unique lock
    ///
    /// \author AMD Developer Tools Team
    /// \date 11/05/2016
    static void UniqueLock();

    ////////////////////////////////////////////////////////////////////////////////////
    /// \brief "Single write" unique unlock
    ///
    /// \author AMD Developer Tools Team
    /// \date 11/05/2016
    static void UniqueUnLock();


private:
};

#endif // __SUSWMRINSTANCE_H

