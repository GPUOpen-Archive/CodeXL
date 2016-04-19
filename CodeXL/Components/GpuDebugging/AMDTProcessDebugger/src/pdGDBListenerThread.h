//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdGDBListenerThread.h
///
//==================================================================================

//------------------------------ pdGDBListenerThread.h ------------------------------

#ifndef __PDGDBLISTENERTHREAD
#define __PDGDBLISTENERTHREAD

// Forward decelerations:
class pdGDBOutputReader;
class pdGDBDriver;
class osPipeSocket;

// Infra
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osCondition.h>

// Local:
#include <src/pdGDBCommandInfo.h>


// ----------------------------------------------------------------------------------
// Class Name:           pdGDBListenerThread : private osThread
// General Description:
//   Listens to GDB outputs when an asynchronos command is running.
//
// Author:               Yaki Tebeka
// Creation Date:        21/12/2006
// ----------------------------------------------------------------------------------
class pdGDBListenerThread : private osThread
{
public:
    pdGDBListenerThread();
    virtual ~pdGDBListenerThread();

    bool startListening(osPipeSocket& gdbCommunicationPipe, pdGDBDriver& GDBDriver, pdGDBOutputReader& gdbOutputReader,
                        pdGDBCommandId executedGDBCommandId);

private:
    // Overrides osThread:
    virtual int entryPoint();
    virtual void beforeTermination();

private:
    void readDataFromGDBStdoutPipe(bool& wasDebuggedProcessTerminated, bool& wasDebuggedProcessSuspended);
    bool exitListenerThread();

private:
    // A pipe to which the gdb process stdout is redirected:
    osPipeSocket* _pGDBCommunicationPipe;

    // The executed GDB asynchronos command:
    pdGDBCommandId _executedGDBCommandId;

    // A condition that halts this thread run when it does not need to listen
    // to the GDB output pipe:
    osCondition _shouldListenToPipeCondition;

    // A GDB driver that can be used to drive GDB and perform additional actions:
    pdGDBDriver* _pGDBDriver;

    // A reader that reads GDB's output:
    pdGDBOutputReader* _pGDBOutputReader;

    // Contains true iff we should exit the GDB listener thread:
    bool _shouldExitListenerThread;
};


#endif  // __PDGDBLISTENERTHREAD
