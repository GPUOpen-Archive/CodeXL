//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdGDBDriver.h
///
//==================================================================================

//------------------------------ pdGDBDriver.h ------------------------------

#ifndef __PDGDBDRIVER_H
#define __PDGDBDRIVER_H

// Forward decelerations:
class gtASCIIString;
struct pdGDBData;
class pdGDBProcessWaiterThread;

// Infra
#include <AMDTBaseTools/Include/gtSet.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSWrappers/Include/osModuleArchitecture.h>
#include <AMDTOSWrappers/Include/osPipeSocketServer.h>

// Local:
#include <src/pdGDBCommandInfo.h>
#include <src/pdGDBOutputReader.h>
#include <src/pdGDBListenerThread.h>
#include <src/pdLinuxDebuggedApplicationOutputReaderThread.h>


// ----------------------------------------------------------------------------------
// Class Name:           pdGDBDriver
// General Description:
//   Drives the GDB debugger.
//
// Author:               Yaki Tebeka
// Creation Date:        19/12/2006
// ----------------------------------------------------------------------------------
class pdGDBDriver
{
public:
    pdGDBDriver();
    virtual ~pdGDBDriver();

    bool initialize(const gtString& gdbExecutable);
    bool terminate();

    bool executeGDBCommand(pdGDBCommandId gdbCommand, const gtASCIIString& commandArgs,
                           const pdGDBData** ppGDBOutputData = NULL);

    bool onDebuggedProcessRunResumed();
    osProcessId gdbProcessId() const { return _gdbProcessId; };
    void waitForInternalDebuggedProcessInterrupt();

    // Helper function for pdGDBOutputReader::handleGetLibraryAtAddressOutput()
    void setInstructionAddressToFind(osInstructionPointer findAddress) {_gdbOutputReader.setInstructionAddressToFind(findAddress);};

    // Set the output reader's pointer width
    void setModuleArchitecture(osModuleArchitecture arch) {_gdbOutputReader.setModuleArchitecture(arch);};

    void kernelDebuggingAboutToStart() {_gdbOutputReader.kernelDebuggingAboutToStart(); };
    void kernelDebuggingJustFinished() {_gdbOutputReader.kernelDebuggingJustFinished(); };

    void flushCommandOutput();

    void getOutputReaderThreadFileName(gtString& fileName) const;
    void getProcessConsolePipeName(gtString& fileName);

    pdLinuxDebuggedApplicationOutputReaderThread* getOutputReaderThread() { return _pDebuggedAppOutputReaderThread; }

    ///////////////////////////////////////////////////////////////////////////////////////
    /// \brief Thread created callback.
    ///
    /// \param threadGDBId a gdb id of new created thread
    /// \author Vadim Entov
    /// \date 23/12/2015
    void OnThreadCreated(int threadGDBId);

    ///////////////////////////////////////////////////////////////////////////////////////
    /// \brief Thread exiting callback.
    ///
    /// \param threadGDBId gdb id of exited thread
    /// \author Vadim Entov
    /// \date 23/12/2015
    void OnThreadExit(int threadGDBId);

    ///////////////////////////////////////////////////////////////////////////////////////
    /// \brief Thread stopped callback.
    ///
    /// \param threadGDBId a gdb id of stopped thread
    /// \author Vadim Entov
    /// \date 23/12/2015
    void OnThreadGDBStopped(int threadGDBId);

    ///////////////////////////////////////////////////////////////////////////////////////
    /// \brief Thread resumed callback.
    ///
    /// \param threadGDBId a gdb id of resumed thread
    /// \author Vadim Entov
    /// \date 23/12/2015
    void OnThreadGDBResumed(int threadGDBId);

    ///////////////////////////////////////////////////////////////////////////////////////
    /// \brief Check host process threads state
    ///
    /// \param Set of threads will may to be running. In case null pointer
    ///     all existing threads will be cheched
    ///
    /// \return true in case all debugged process threads stopped
    /// \author Vadim Entov
    /// \date 23/12/2015
    bool IsAllThreadsStopped(std::set<int>* pUnneededThreads = nullptr);

    ///////////////////////////////////////////////////////////////////////////////////////
    /// \brief Check host process threads running state
    ///
    /// \return true in case all debugged process threads resumed
    /// \author Vadim Entov
    /// \date 18/01/2015
    bool IsAllThreadsRunning();

    ///////////////////////////////////////////////////////////////////////////////////////
    /// \brief Check specified thread state
    ///
    /// \param threadGDBId a requested thread GDB id
    /// \return true in case thread in running state and vice versa
    /// \author Vadim Entov
    /// \date 19/01/2015
    bool IsThreadRunning(int threadGDBId);

    ///////////////////////////////////////////////////////////////////////////////////////
    /// \brief Get count of currently running threads
    ///
    /// \return Running threads count
    /// \author Vadim Entov
    /// \date 28/01/2015
    int GetRunnungThreadsCount();

    ///////////////////////////////////////////////////////////////////////////////////////
    /// \brief Get count of currently existing threads
    ///
    /// \return Running threads count
    /// \author Vadim Entov
    /// \date 11/02/2016
    int GetExistingThreadsCount();

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Get currently host process stopped threads
    ///
    /// \return Set of currently host process stopped threads
    /// \author Vadim Entov
    /// \date 03/02/2016
    const gtSet<int>& GetStoppedThreads();

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief External starting GDB listener thread
    ///
    /// \return true - success, false - fail
    /// \author Vadim Entov
    /// \date 03/02/2016
    bool StartBackgoundGDBListen();

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Flush GDB prompt
    ///
    /// \return true - success, false - fail
    /// \author Vadim Entov
    /// \date 10/02/2016
    bool FlushPrompt();

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief Suspend specific thread
    ///
    /// \param threadGDBId a gdb thread index
    /// \return true - success, false - failed
    /// \author Vadim Entov
    /// \date 18/02/2016
    bool SuspendThread(int threadGDBId);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief Resume specific thread
    ///
    /// \param threadGDBId a gdb thread index
    /// \return true - success, false - failed
    /// \author Vadim Entov
    /// \date 18/02/2016
    bool ResumeThread(int threadGDBId);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief Return if any thread really running in launching process. Created thread and
    ///   launching thrad it's not the same 
    ///
    /// \return true - first thread running after creation
    /// \author Vadim Entov
    /// \date 03/08/2016
    bool IsProcessStarted() const { return m_firstThreadRunning && m_processExistingThreads.size() != 0;}

private:
    const char** getGDBExecutionCommandLineArguments(const gtString& gdbExecutable) const;
    bool setGDBUsedShell();
    bool restoreShellEnvVariableValue();
    bool createGDBCommunicationPipes();
    bool wrapGDBCommunicationPipes();
    bool closeGDBCommunicationPipes();
    bool redirectGDBStdinAndStdoutToPipes();
    bool initializeGDB();
    bool sendGDBInitializationCommands();
    bool buildGDBCommandString(pdGDBCommandId gdbCommand, const gtASCIIString& commandArgs, gtASCIIString& commandString);
    bool buildGDBCommandArguments(const pdGDBCommandInfo& commandInfo, const gtASCIIString& commandArgs, gtASCIIString& commandArgumentsInGDBStyle);
    bool writeToGDBInput(const gtASCIIString& gdbInputString);
    void outputWritingToGDBLogMessage(const gtASCIIString& gdbInputString);
    void clearGDBCommunicationPipeNames();
    void registerBrokenPipeSignalHandler();

private:
    // Contains true iff this class was initialized:
    bool _wasInitialized;

    // GDBs process id:
    osProcessId _gdbProcessId;

    // A thread that releases the GDB process from Zombie state when it terminates:
    pdGDBProcessWaiterThread* m_pGDBProcessWaiterThread;

    // Pipes, used for communicating with the GDB process.
    int _gdbProcessStdin[2];
    int _gdbProcessStdout[2];
    int _gdbProcessConsoleStdIn[2];

    // A pipes wrapper class:
    osPipeSocket* _pGDBCommunicationPipe;

    // Process console pipe
    osPipeSocket* _pGDBProcessConsolePipe;
    osPipeSocket* _pGDBProcessConsolePipeClient;

    // A thread that listens to asynchronous GDB outputs:
    pdGDBListenerThread* _pGDBListenerThread;

    // GDB's output reader:
    pdGDBOutputReader _gdbOutputReader;

    // Debugged application output reader:
    pdLinuxDebuggedApplicationOutputReaderThread* _pDebuggedAppOutputReaderThread;

    // Stores the value of the SHELL environment variable:
    // (See setGDBUsedShell() for more details)
    gtString _storedShellEnvVariableValue;

    gtSet<int>          m_processExistingThreads;   ///< Set of existing threads of debugged process
    gtSet<int>          m_processStoppedThreads;    ///< Set of currently stopped threads of debugged process
    osCriticalSection   m_threadsInfoCS;            ///< Synchrinzation primitive of debugged process threads info
    unsigned int        m_createdProcessThread;     ///< True after first process thread was created and false after last thread exit
    bool                m_firstThreadRunning;       ///< First process thread running flag   
};


#endif  // __PDGDBDRIVER_H
