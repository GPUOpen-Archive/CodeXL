//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Responsible for controlling the playback of captures API calls
//==============================================================================

#if defined (_WIN32)
    #include <windows.h>
#elif defined (_LINUX)
    #include "WinDefs.h"
#endif
#include <vector>
#include "CaptureStream.h"
#include "misc.h"
#include "OSWrappers.h"

void CapturedAPICalls::Add(Capture* pCap)
{
    ScopeLock t(m_pCapturedAPICallsMutex);
    m_captureList.push_back(pCap);

    //printf( "%i %s\n", m_tokens.size(), pCap->Print().c_str() );
}

void CapturedAPICalls::ReleaseCapture()
{
    for (CaptureList::iterator iter = m_captureList.begin(); iter != m_captureList.end(); ++iter)
    {
        delete *iter;
    }

    m_captureList.clear();
}

void CapturedAPICalls::PlayCapture()
{
    unsigned int i = 0;

    try
    {
        for (CaptureList::iterator iter = m_captureList.begin(); iter != m_captureList.end(); ++iter)
        {
            if (m_LogCallsAsItReplays)
            {
                LogConsole(logMESSAGE, "replay: %4i %s\n", i, (*iter)->Print().c_str());
            }

            // Debugging code used to locate which API call index is causing a problem
            //if (i % 1000 == 0)
            //{
            //Log(logDEBUG, "Replay Index: %ld\n", i);
            //Log(logDEBUG, "replay: %4i %s\n", i, (*iter)->Print().c_str());
            //}

            (*iter)->Play();

            i++;
        }
    }
    catch (...)
    {
        OSWrappers::MessageBox(FormatString("Frame Capture playback: Exception at drawcall %i", i).c_str(), (const char*)"Exception!", 0);
    }
}

void CapturedAPICalls::PlayOverride(CaptureOverride* pCO)
{
    unsigned int i = 0;

    try
    {
        for (CaptureList::iterator iter = m_captureList.begin(); iter != m_captureList.end(); ++iter)
        {
            (*iter)->PlayOverride(pCO);
            ++i;
        }
    }
    catch (...)
    {
        OSWrappers::MessageBox(FormatString("Exception on %i", i).c_str(), (const char*)"exception!", 0);
    }
}

void CapturedAPICalls::PlayCaptureAndGetTimingData(TimingLog& timing)
{
    unsigned int i = 0;

    LARGE_INTEGER startTime;

    try
    {
        for (CaptureList::iterator iter = m_captureList.begin(); iter != m_captureList.end(); ++iter)
        {
            startTime = timing.GetRaw();

            (*iter)->Play();

            timing.Add((*iter)->GetThreadID(), startTime);
            ++i;
        }
    }
    catch (...)
    {
        OSWrappers::MessageBox(FormatString("Exception on %i", i).c_str(), (const char*)"exception!", 0);
    }
}

std::string CapturedAPICalls::GetCaptureLog()
{
    std::string tmp = "";

    if (m_captureList.empty() == false)
    {
        tmp.reserve((const int)(m_captureList.size() + 1) * 80);

        if (m_pActiveDevice != NULL)
        {
            tmp += FormatText("MainContext=0x%p\n", m_pActiveDevice).asCharArray();
        }

        for (CaptureList::iterator iter = m_captureList.begin(); iter != m_captureList.end(); ++iter)
        {
            tmp += FormatString("%d ", (*iter)->GetThreadID()) + (*iter)->Print() + "\n";
        }
    }

    return tmp;
}
