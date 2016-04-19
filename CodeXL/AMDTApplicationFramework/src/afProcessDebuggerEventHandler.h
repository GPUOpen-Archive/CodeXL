//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afProcessDebuggerEventHandler.h
///
//==================================================================================

#ifndef __AFPROCESSDEBUGGEREVENTHANDLER
#define __AFPROCESSDEBUGGEREVENTHANDLER

// QtGui
#include <QtWidgets>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           afProcessDebuggerEventHandler : public QObject
// General Description:
//  This is a WX events handler class that catch a single event type: afProcessDebuggerPendingEventEvent.
//  We use this class to synchronize between the process debugger, that runs in his
//  own thread and the main application thread.
//
//  The mechanism is as follows:
//  -  When a debugged process event occurs (a-synchronic to the main application thread),
//     the process debugger throws a afProcessDebuggerPendingEventEvent, which is placed
//     in this event handler events queue.
//  - The events in this event handler queue are handled by the main application thread.
//
// Author:               Yaki Tebeka
// Creation Date:        7/4/2004
// Move to Qt Gilad Yarnitzky 6/19/2012
// ----------------------------------------------------------------------------------
class AF_API afProcessDebuggerEventHandler : public QObject
{
    Q_OBJECT

public:
    static afProcessDebuggerEventHandler& instance();
    ~afProcessDebuggerEventHandler();

public Q_SLOTS:
    void OnDebuggedProcessEvent();

private:
    // Only my instance method should create me:
    afProcessDebuggerEventHandler();

private:

    friend class afSingletonsDelete;

    // The single instance of this class:
    static afProcessDebuggerEventHandler* m_pMySingleInstance;
};

#endif  // __GDPROCESSDEBUGGEREVENTHANDLER
