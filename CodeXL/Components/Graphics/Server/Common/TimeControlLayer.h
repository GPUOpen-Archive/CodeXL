//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Pauses or slows down the game's time
//==============================================================================

#ifndef TIMECONTROLLAYER_H
#define TIMECONTROLLAYER_H

#include "../Common/ILayer.h"
#include "../Common/TSingleton.h"
#include "../Common/CommandProcessor.h"
#include "../Common/timer.h"

/// Layer which accepts HTTP commands to change play speed and real-pause setting
class TimeControlLayer: public ILayer, public CommandProcessor, public TSingleton< TimeControlLayer >
{
    /// TSingleton needs to be able to use our constructor.
    friend class TSingleton<TimeControlLayer>;

public:

    /// The layer must create its resources here, it may hook some functions if really needed
    /// \param type the creation object type.
    /// \param pPtr Pointer to the object that was just created.
    /// \return True if success, False if fail.
    bool OnCreate(CREATION_TYPE type, void* pPtr);

    /// layer to destroy resources, and detach anything that
    /// might attached previously
    bool OnDestroy(CREATION_TYPE type, void* pPtr);

    /// function to enable/disable the layer.
    /// if can't disable because layer is in the middle of an operation
    /// it should return ERR_BUSY, Otherwise S_OK
    /// When enabled the layer could hook some extra functions that
    /// should be unhooked when asked to disable
    bool OnEnableLayer(bool bNewStatus, CommandResponse* pRequest);

    void BeginFrame();

    /// Sets whether or not real-pause will be used
    /// \param pausedWithFrameCapture true to enable delta time of 0, false to let time increment very slowly
    void SetPausedWithFrameCapture(bool pausedWithFrameCapture)
    {
        m_PausedWithFrameCapture = pausedWithFrameCapture;
    }

protected:

    /// Default constructor
    TimeControlLayer();

    /// Indicates if the user has requested to use freezetime (true) or slowmotion (false) for pausing
    BoolCommandResponse m_RealPause;

    /// The play speed
    FloatCommandResponse m_SpeedControl;

    /// Indicates that the app was paused with frame capture.
    BoolCommandResponse m_PausedWithFrameCapture;

    /// Sets the play speed
    /// \param fSpeed The speed at which to play the application.
    void SetPlaySpeed(float fSpeed);

    /// Sets whether or not real-pause will be used
    /// \param bRealPause true to enable delta time of 0, false to let time increment very slowly
    void SetRealPause(bool bRealPause);

    /// Sets whether or not time is actually frozen (delta time of 0)
    /// \param bFreezeTime true to freeze time
    void SetFreezeTime(bool bFreezeTime);

private:

    //-----------------------------------------------------------------------------
    // No additional settings
    //-----------------------------------------------------------------------------
    virtual string GetDerivedSettings() { return ""; }

    /// Records our current attach state. Prevents us from attaching twice.
    bool m_bAttached;
};

#endif //LOGGERLAYER_H
