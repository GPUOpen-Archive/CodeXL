//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Class used for time related override functions. Used to override the games's time values.
//==============================================================================

#include "timer.h"

//-----------------------------------------------------------------------------
///
/// Accessor to the singleton TimeControl instance
///
//-----------------------------------------------------------------------------
TimeControl& TimeControl::Singleton()
{
    static TimeControl s_timeControl;

    return s_timeControl;
}

//---------------------------------------------------------------------
///
/// Constructor for the TimeControl class
///
//---------------------------------------------------------------------
TimeControl::TimeControl():
    m_bFreezeTime(false)
{
    m_SpeedControl = 1.0;
    m_RealPause = false;
}

//-----------------------------------------------------------------------------
///
/// Destructor for the TimeControl class
///
//-----------------------------------------------------------------------------
TimeControl::~TimeControl()
{
}

//---------------------------------------------------------------------
///
/// This function gets the current pause state
///
/// \return the current pause state
//---------------------------------------------------------------------
bool TimeControl::GetFreezeTime()
{
    return m_bFreezeTime;
}

//---------------------------------------------------------------------
///
/// This function sets the current play speed
///
/// \param fSpeed the current play speed
//---------------------------------------------------------------------
void TimeControl::SetPlaySpeed(float fSpeed)
{
    //
    // Only set FreezeTime to true (for apps which can handle delta t == 0)
    // if RealPause is true (the user has checked it) and the
    // requested speed is 0.
    SetFreezeTime((m_RealPause == true) && (fSpeed == 0.0f));
    m_SpeedControl = fSpeed ;
}

//---------------------------------------------------------------------
///
/// This function gets the current play speed
///
/// \return the current play speed
//---------------------------------------------------------------------
float TimeControl::GetPlaySpeed()
{
    return m_SpeedControl;
}

//---------------------------------------------------------------------
///
/// This function sets the RealPause state
///
/// Only set this value to true if the application being debugged can
/// handle delta t = 0.
///
/// \param bRealPause the current RealPause state
//---------------------------------------------------------------------
void TimeControl::SetRealPause(bool bRealPause)
{
    m_RealPause = bRealPause;
    SetPlaySpeed(m_SpeedControl);
}


//---------------------------------------------------------------------
///
/// This function gets the RealPause state
///
/// \return the current RealPause state
//---------------------------------------------------------------------
bool TimeControl::GetRealPause()
{
    return m_RealPause;
}

