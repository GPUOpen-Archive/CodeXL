//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Pauses or slows down the game's time
//==============================================================================

// TODO: the timer.h/cpp in the common project once is not needed anymore

#ifdef _WIN32
    #include <windows.h>
    #pragma warning (disable: 4201) // suppress for next line only
    #include <Mmsystem.h>
    #pragma warning (default: 4201)
#endif // _WIN32

#include "../Common/ILayer.h"
#include "../Common/HookTimer.h"
#include "TimeControlLayer.h"
#include "../Common/CommandStrings.h"

//=============================================================================
// Constructor
//=============================================================================
TimeControlLayer::TimeControlLayer()
    : m_bAttached(false)
{
    m_SpeedControl.SetValue(1.0f);
    m_RealPause.SetValue(false);

    AddCommand(CONTENT_HTML, "speed",     "Speed",       CMD_SPEED,     NO_DISPLAY, INCLUDE, m_SpeedControl);
    m_SpeedControl.SetEditableContentAutoReply(false);

    AddCommand(CONTENT_HTML, "realpause", "Real Pause",  CMD_REALPAUSE, NO_DISPLAY, INCLUDE, m_RealPause);
    m_RealPause.SetEditableContentAutoReply(false);

    AddCommand(CONTENT_HTML, "PausedWithFrameCapture", "Paused With Frame Capture",  "PausedWithFrameCapture", NO_DISPLAY, INCLUDE, m_PausedWithFrameCapture);
    m_PausedWithFrameCapture.SetEditableContentAutoReply(true);

    SetLayerName("TimeControlLayer");
}

//=============================================================================
/// Create/Destruct Wrapper functions
///
/// Note that the logger will monitor all the devices!
///
//=============================================================================
bool TimeControlLayer::OnCreate(CREATION_TYPE type, void* pPtr)
{
    PS_UNREFERENCED_PARAMETER(type);

    LogTrace(traceENTER, "pPtr = 0x%p", pPtr);

    bool bRet = true;

    if (m_bAttached == false)
    {
        bRet = HookTimer();

        if (bRet)
        {
            m_bAttached = true;
            // update the command processor with the proper values
            m_RealPause = TimeControl::Singleton().GetRealPause();
            m_SpeedControl = TimeControl::Singleton().GetPlaySpeed();
        }
    }

    LogTrace(traceEXIT, "");

    return bRet;
}

bool TimeControlLayer::OnDestroy(CREATION_TYPE type, void* pPtr)
{
    PS_UNREFERENCED_PARAMETER(type);

    LogTrace(traceENTER, "pPtr = 0x%p", pPtr);

    bool bRet = true;

    if (m_bAttached == true)
    {
        bRet = UnhookTimer();

        if (bRet == true)
        {
            m_bAttached = false;
        }
    }

    LogTrace(traceEXIT, "");

    return bRet;
}

void TimeControlLayer::BeginFrame()
{
    if (m_SpeedControl.IsActive())
    {
        if (m_SpeedControl.GetValue() >= 0)
        {
            TimeControl::Singleton().SetPlaySpeed(m_SpeedControl);
            m_SpeedControl.Send("OK");
        }
        else
        {
            m_SpeedControl.SendError("Speed must be a positive value");
        }
    }

    if (m_RealPause.IsActive())
    {
        TimeControl::Singleton().SetRealPause(m_RealPause);
        m_RealPause.Send("OK");
    }
}

//=============================================================================
/// Enables/Disables a layer
/// \param bNewStatus Status of this event
/// \param pRequest Incoming request
/// \return True if success false if fail
//=============================================================================
bool TimeControlLayer::OnEnableLayer(bool bNewStatus, CommandResponse* pRequest)
{
    if (bNewStatus)
    {
        if (pRequest != NULL)
        {
            pRequest->Send("OK");
        }

        return true;
    }
    else
    {
        if (pRequest != NULL)
        {
            pRequest->SendError("Cannot be disabled");
        }

        return false;
    }
}

