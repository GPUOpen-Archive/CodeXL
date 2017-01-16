//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpCollect.h
///
//==================================================================================

#ifndef _TPCOLLECT_H_
#define _TPCOLLECT_H_

// Project Headers
#include <tpInternalDataTypes.h>
#include <AMDTThreadProfileDataTypes.h>

//
// tpCollect
//

class tpCollectImpl;

class tpCollect
{
public:
    tpCollect();
    ~tpCollect();

    AMDTResult tpInitialize();
    AMDTResult tpSetThreadProfileConfiguration(AMDTUInt32 flags, const char* pFilePath);
    AMDTResult tpStartThreadProfile();
    AMDTResult tpStopThreadProfile();
    AMDTResult tpClear();

private:
    // This is OS specific implementation
    tpCollectImpl*  m_pImpl;
};

#endif //_TPCOLLECT_H_