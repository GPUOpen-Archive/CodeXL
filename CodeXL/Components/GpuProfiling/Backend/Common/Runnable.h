//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class is the base for classes which want to execute Run() function
//         in a new thread
//==============================================================================

#ifndef _RUNNABLE_H_
#define _RUNNABLE_H_

#include "OSUtils.h"

//------------------------------------------------------------------------------------
// This class is the base for classes which want to execute Run() function
// in a new thread
//------------------------------------------------------------------------------------
class Runnable
{
    /// Internal object used to pass parameter
    struct RunnableParam
    {
        Runnable* m_pThis;  ///< This object
        void*     m_pParam; ///< Real parameter that is passed to new thread
    };

public:
    /// Start thread
    /// \param pParam The parameter passed to the new thread
    void Start(void* pParam)
    {
        RunnableParam* pRunnable = new(std::nothrow) RunnableParam();

        if (pRunnable != NULL)
        {
            pRunnable->m_pThis = this;
            pRunnable->m_pParam = pParam;
            m_tid = OSUtils::Instance()->CreateThread(run, pRunnable);
        }
    }

    /// Join created thread
    void Join()
    {
        OSUtils::Instance()->Join(m_tid);
    }

protected:
    /// Constructor
    Runnable()
    {}

    /// Destructor
    virtual ~Runnable()
    {}

    /// New thread function
    /// \param pParam Parameter
    virtual void Run(void* pParam) = 0;
private:
    /// Internal helper function used to launch thread
    static void run(void* pParam)
    {
        RunnableParam* pRunnable = static_cast<RunnableParam*>(pParam);

        if (pRunnable != NULL)
        {
            pRunnable->m_pThis->Run(pRunnable->m_pParam);
            delete pRunnable;
        }
    }

    THREADHANDLE m_tid;  ///< Thread handle
};

#endif //_RUNNABLE_H_
