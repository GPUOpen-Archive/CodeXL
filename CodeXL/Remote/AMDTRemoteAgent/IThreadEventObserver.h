//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file IThreadEventObserver.h
///
//==================================================================================

#ifndef __IThreadEventObserver_h
#define __IThreadEventObserver_h

class osThread;
class IThreadEventObserver
{
public:
    virtual void onThreadCreation(osThread* pCreatedThread) = 0;
    virtual ~IThreadEventObserver(void);
};

#endif // __IThreadEventObserver_h
