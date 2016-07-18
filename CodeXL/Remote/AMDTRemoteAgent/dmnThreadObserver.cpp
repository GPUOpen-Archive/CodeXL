//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnThreadObserver.cpp
///
//==================================================================================

#include <sstream>
#include <algorithm>
#include <AMDTRemoteAgent/dmnThreadObserver.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

dmnThreadObserver::dmnThreadObserver(void) : m_threads()
{
}

dmnThreadObserver::~dmnThreadObserver(void)
{
}

void dmnThreadObserver::clean()
{
    //std::for_each(m_threads.begin(), m_threads.end(), [](osThread* pThread)
    gtList<osThread*>::iterator iter;

    for (iter = m_threads.begin(); iter != m_threads.end(); iter++)
    {
        if ((*iter) != NULL)
        {
            std::wstringstream stream;
            stream << L"Daemon Thread Observer: terminating the following thread: " << (*iter)->id();
            OS_OUTPUT_DEBUG_LOG(stream.str().c_str(), OS_DEBUG_LOG_DEBUG);
            bool isOk = (*iter)->terminate();

            if (!isOk)
            {
                stream << L" FAILURE" << std::endl;
                OS_OUTPUT_DEBUG_LOG(stream.str().c_str(), OS_DEBUG_LOG_ERROR);
            }

            delete(*iter);
        }
    }
}

void dmnThreadObserver::onThreadCreation(osThread* pCreatedThread)
{
    m_threads.push_back(pCreatedThread);
}
