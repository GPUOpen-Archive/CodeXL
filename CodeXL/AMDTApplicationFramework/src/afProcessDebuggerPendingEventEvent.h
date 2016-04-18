//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afProcessDebuggerPendingEventEvent.h
///
//==================================================================================

#ifndef __AFPROCESSDEBUGGERPENDINGEVENTEVENT
#define __AFPROCESSDEBUGGERPENDINGEVENTEVENT

// QtGui
#include <QtWidgets>

// Local:
#include <AMDTApplicationFramework//Include/afApplicationFrameworkDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           afProcessDebuggerPendingEventEvent : public QObject
// General Description:
//   A wxWindows event. It is fired when a debugged process event occurs.
// Author:               Yaki Tebeka
// Creation Date:        6/4/2004
// Move to Qt Gilad Yarnitzky 6/19/2012
// ----------------------------------------------------------------------------------
class AF_API afProcessDebuggerPendingEventEvent : public QObject
{
    Q_OBJECT

public:
    static afProcessDebuggerPendingEventEvent& instance();
    ~afProcessDebuggerPendingEventEvent();

    // emit the event:
    void emitEvent();

Q_SIGNALS:
    void pendingDebugEvent();

private:
    afProcessDebuggerPendingEventEvent();

    friend class afSingletonsDelete;

    // The single instance of this class:
    static afProcessDebuggerPendingEventEvent* m_pMySingleInstance;
};


#endif  // __GDPROCESSDEBUGGERPENDINGEVENTEVENT
