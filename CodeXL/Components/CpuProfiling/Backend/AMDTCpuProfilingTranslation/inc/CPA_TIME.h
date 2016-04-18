//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CPA_TIME.h
/// \brief Helper functions for CPA_TIME.
///
//==================================================================================

#ifndef _CPA_TIME_H_
#define _CPA_TIME_H_

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

/** \struct CPA_TIME
    \brief Data structure containing time since the Epoch
    \ingroup sampling

    This data structure contains time value since the Epoch in second and
    microsecond. Epoch is defined as the time 00:00:00 +0000 (UTC) on 1970-01-01 .
*/
struct CPA_TIME
{
    CPA_TIME() : second(0), microsec(0) {}

    gtUInt32 second; // Number of second
    gtUInt32 microsec; // Number of microsecond
};

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    HRESULT CPA_TIME_to_FILETIME(CPA_TIME cpaTime, FILETIME* pFileTime);
    HRESULT FILETIME_to_CPA_TIME(FILETIME fileTime, CPA_TIME* pCpaTime);
#endif

#endif // _CPA_TIME_H_
