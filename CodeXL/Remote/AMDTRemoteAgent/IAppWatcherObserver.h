//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file IAppWatcherObserver.h
///
//==================================================================================

#ifndef __IAppEventHandler_h
#define __IAppEventHandler_h

// An interface that lets its implementor handle dmnAppWatcherThread-related events.
class IAppWatcherObserver
{
public:
    virtual ~IAppWatcherObserver(void) {}

    // Called when the target app is terminated.
    virtual void onAppTerminated() = 0;
};

#endif // __IAppEventHandler_h
