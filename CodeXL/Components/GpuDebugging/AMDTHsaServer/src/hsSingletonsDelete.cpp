//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file hsSingletonsDelete.cpp
///
//==================================================================================


// Local:
#include <src/hsDebuggingManager.h>
#include <src/hsHSAMonitor.h>


class hsSingletonsDelete
{
public:
    hsSingletonsDelete()
    {
    }

    ~hsSingletonsDelete()
    {
        delete hsDebuggingManager::ms_pMySingleInstance;
        hsDebuggingManager::ms_pMySingleInstance = nullptr;

        delete hsHSAMonitor::ms_pMySingleInstance;
        hsHSAMonitor::ms_pMySingleInstance = nullptr;
    }
};

hsSingletonsDelete g_singletonDeleter;
