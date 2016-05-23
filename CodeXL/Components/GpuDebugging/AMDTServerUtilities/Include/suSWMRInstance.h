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

#endif // AMDT_BUILD_TARGET == AMDT_LINUX_OS

#endif // __SUSWMRINSTANCE_H

