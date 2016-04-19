//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suLunuxThrdsSuspender.h
/// \brief The suLinuxThrdsSuspender class definition
///
//==================================================================================

#include <pthread.h>
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <condition_variable>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>

/////////////////////////////////////////////////////
/// \class suLunuxThrdsSuspender
/// \brief Suspend and result host process threads
///
/// \description GDB launching the host process with "non stop" mode on.
///    By this reason on breakpoint hit only current thread will be suspended
///    while other threads continue running. This is undesirable behavior and
///    the CodeXL will send request to suspend/resume all other threads except the
///    spy thread. The suLinuxThrdsSuspender implement resume/suspend threads logic
///
/// \author Vadim Entov
/// \date 12/17/2015
class suLinuxThrdsSuspender
{
public:
    /////////////////////////////////////////////////////
    /// \brief Standard constructor
    ///
    /// \author Vadim Entov
    /// \date 12/17/2015
    suLinuxThrdsSuspender();

    /////////////////////////////////////////////////////
    /// \brief Standard destructor
    ///
    /// \author Vadim Entov
    /// \date 12/17/2015
    ~suLinuxThrdsSuspender();

    /////////////////////////////////////////////////////
    /// \brief Suspend threads
    ///
    /// \param thrds a vector of threads native handles
    /// \return true - success / false - failed
    /// \author Vadim Entov
    /// \date 12/17/2015
    bool SuspendThreads(const std::vector<osThreadId>& thrds);

    //////////////////////////////////////////////////////////////////
    /// \brief Resume threads. All previously suspended threads
    ///   stored into the internal structure
    ///
    /// \return true - success / false - failed
    /// \author Vadim Entov
    /// \date 12/17/2015
    bool ResumeThreads();

    //////////////////////////////////////////////////////////////////
    /// \brief Get singleton instance
    ///
    /// \return Reference to singleton instance
    /// \author Vadim Entov
    /// \date 12/17/2015
    static suLinuxThrdsSuspender& getInstance();

private:
    static std::set<pthread_t>        m_suspendedThreads;   ///< Set of suspended threads
    static std::condition_variable    m_cvHandler;          ///< Thread suspended conditional variable
    static std::mutex                 m_mtxCvHandler;       ///< Conditional variable synchronization objcet
    std::mutex                        m_mtx;                ///< Suspend/resume synchronization object

    //////////////////////////////////////////////////////////////////
    /// \brief Handle "sig" in the target thread, to suspend it until receiving
    /// sig(resume).Note that this is run with "sig" blocked.
    ///
    /// \param sig a blocking signal
    /// \author Vadim Entov
    /// \date 12/17/2015
    static void SuspendSignalHandler(int sig);
};

