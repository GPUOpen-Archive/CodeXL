//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suLinuxThrdsSuspender.cpp
/// \brief The suLinuxThrdsSuspender class implementation
///
//==================================================================================

#include <AMDTServerUtilities/Include/suLinuxThrdsSuspender.h>
#include <thread>

std::set<pthread_t>        suLinuxThrdsSuspender::m_suspendedThreads;   ///< Set of suspended threads
std::condition_variable    suLinuxThrdsSuspender::m_cvHandler;         ///< Thread suspended conditional variable
std::mutex                 suLinuxThrdsSuspender::m_mtxCvHandler;     ///< Conditional variable synchronization objcet

/////////////////////////////////////////////////////
/// \brief Standard constructor
///
/// \author Vadim Entov
/// \date 12/17/2015
suLinuxThrdsSuspender::suLinuxThrdsSuspender()
{
    int status = 0;
    struct sigaction sigcont;

    /*
     * Install the signal handlers for suspend/resume.
     *
     */

    sigcont.sa_flags = 0;
    sigcont.sa_handler = &suLinuxThrdsSuspender::SuspendSignalHandler;
    sigemptyset(&sigcont.sa_mask);
    sigaddset(&sigcont.sa_mask, SIGCONT);

    status = sigaction(SIGCONT, &sigcont, NULL);

    if (status == -1)
    {
        return;
    }
}

/////////////////////////////////////////////////////
/// \brief Standard destructor
///
/// \author Vadim Entov
/// \date 12/17/2015
suLinuxThrdsSuspender::~suLinuxThrdsSuspender()
{
    ResumeThreads();
}

//////////////////////////////////////////////////////////////////
/// \brief Handle "sig" in the target thread, to suspend it until receiving
/// sig(resume).Note that this is run with "sig" blocked.
///
/// \param sig a blocking signal
/// \author Vadim Entov
/// \date 12/17/2015
void suLinuxThrdsSuspender::SuspendSignalHandler(int sig)
{
    sigset_t signal_set;

    /*
     * Block all signals except "sig" while suspended.
     */
    if (m_suspendedThreads.end() == m_suspendedThreads.find(pthread_self()))
    {
        std::unique_lock<std::mutex> lock(m_mtxCvHandler);

        sigfillset(&signal_set);
        sigdelset(&signal_set, sig);

        m_suspendedThreads.insert(pthread_self());
        m_cvHandler.notify_all();

        sigsuspend(&signal_set);
    }
    else
    {
        m_suspendedThreads.erase(pthread_self());
    }
}

/////////////////////////////////////////////////////
/// \brief Suspend threads
///
/// \param thrds a vector of threads native handles
/// \return true - success / false - failed
/// \author Vadim Entov
/// \date 12/17/2015
bool suLinuxThrdsSuspender::SuspendThreads(const std::vector<osThreadId>& thrds)
{
    int status = 0;
    std::unique_lock<std::mutex>    lock(m_mtx);

    for (auto const& it : thrds)
    {
        if (m_suspendedThreads.find(it) != m_suspendedThreads.end())
        {
            /// One of requested threads already suspended
            return false;
        }
    }

    for (auto const& it : thrds)
    {
        status = pthread_kill(it, SIGCONT);

        if (status != 0)
        {
            ResumeThreads();
            return false;
        }

        std::unique_lock<std::mutex> handler_lock(m_mtxCvHandler);
        m_cvHandler.wait(handler_lock, [&] { return m_suspendedThreads.end() != m_suspendedThreads.find(it); });
    }

    return status == 0;
}

//////////////////////////////////////////////////////////////////
/// \brief Resume threads. All previously suspended threads
///   stored into the internal structure
///
/// \return true - success / false - failed
/// \author Vadim Entov
/// \date 12/17/2015
bool suLinuxThrdsSuspender::ResumeThreads()
{
    int status = 0;
    std::unique_lock<std::mutex>    lock(m_mtx);

    for (auto const& it : m_suspendedThreads)
    {
        status = pthread_kill(it, SIGCONT);

        if (status != 0)
        {
            return false;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////
/// \brief Get singleton instance
///
/// \return Reference to singleton instance
/// \author Vadim Entov
/// \date 12/17/2015
suLinuxThrdsSuspender& suLinuxThrdsSuspender::getInstance()
{
    static suLinuxThrdsSuspender   inst; ///< The linux threads suspender instance

    return inst;
}

