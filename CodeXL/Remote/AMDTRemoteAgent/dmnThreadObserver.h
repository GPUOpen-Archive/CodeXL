//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnThreadObserver.h
///
//==================================================================================

#ifndef __dmnThreadObserver_h
#define __dmnThreadObserver_h

#include <AMDTRemoteAgent/IThreadEventObserver.h>
#include <AMDTBaseTools/Include/gtList.h>

class dmnThreadObserver :
    public IThreadEventObserver
{
public:
    dmnThreadObserver(void);
    virtual ~dmnThreadObserver(void);
    virtual void onThreadCreation(osThread* pCreatedThread) override;

    // We chose not to follow RAII, since we may choose to clean in the middle
    // of the process, while not being required to re-create a dmnThreadObserver.
    void clean();
private:
    gtList<osThread*> m_threads;
};

#endif // __dmnThreadObserver_h
