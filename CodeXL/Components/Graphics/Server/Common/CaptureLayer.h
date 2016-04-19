//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Base class for all derived API specific capture layers
//==============================================================================

#ifndef FRAMECAPTURELAYERCOMMOM_H
#define FRAMECAPTURELAYERCOMMOM_H

#include "ILayer.h"
#include "CommandProcessor.h"
#include "CaptureStream.h"
#include <AMDTOSWrappers/Include/osThread.h>

#ifdef _WIN32
    #include "MapDeltaOptimization/mdoManager.h"
#else
    class MdoManager;
#endif

//=============================================================================
// Capture_StaticWndProc
//=============================================================================
#if defined _WIN32
    LRESULT CALLBACK Capture_StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

//=============================================================================
/// CaptureLayer class responsible for frame capture above the API level
//=============================================================================
class CaptureLayer: public ILayer, public CommandProcessor
{
public:

    /// constructor
    CaptureLayer();

    /// get Replay status
    /// \return if the frame should be replayed
    bool GetReplayLoop()
    {
        return m_ReplayLoop;
    }

    /// Set the Replay frame flag.
    /// \param flag True or False
    void SetReplayLoop(bool flag)
    {
        m_ReplayLoop = flag;
    }

    /// returns a pointer to the captured stream
    /// \return the captured stream of api calls
    CapturedAPICalls* GetCapturedAPICalls()
    {
        return &m_CapturedAPICalls;
    }

    /// Adds a captured call to the stream
    /// \param pCap captured call
    void AddCapturedCall(Capture* pCap);

    /// Replays the captured frame, expects resources to be initialized.
    void ReplayFrame();

    //-----------------------------------------------------------------------------
    /// Setup everything needed at the beginning of a frame needed to perform an
    /// instant capture. Sets up the time control method and other flags needed
    /// before the API calls are captured
    /// \param timeOverrideMode equivalent of client Time Override mode
    /// \param copyMappedBuffersUsingCPU equivalent of client "Copy mapped buffers using CPU" ("True" or "False")
    //-----------------------------------------------------------------------------
    void InstantCaptureBeginFrame(int timeOverrideMode, int filterDrawCallsMask, bool copyMappedBuffersUsingCPU, unsigned int frameCount);

    //-----------------------------------------------------------------------------
    /// Setup everything needed at the end of a frame needed to perform an
    /// instant capture. Sets up certain flags after the API calls have been
    /// captured
    /// \param flattenCommandLists equivalent of client "Flatten Commandlists"
    //-----------------------------------------------------------------------------
    void InstantCaptureEndFrame(bool flattenCommandLists);

    /// Gets a value indicating whether map calls should be handled with CPU-side copies instead of the GPU-side copies.
    /// \return True if CPU copies should be used; false if GPU-side copies should be used.
    bool HandleMapsOnCPU()
    {
        return (bool)m_HandleMapsOnCPU;
    }

    /// Check to see if draw call filtering is ON
    /// \return A bool, true if filtering is ON, false if OFF.
    bool GetFilterDrawCalls()
    {
        return m_filterDrawcallMask != 0;
    }

    /// Command to indicate that frame capture should log the calls as they are being captured
    /// I't helps tell at what point did the capture stopped working
    BoolCommandResponse m_LogCallsAsItCaptures;

    /// Command to indicate that frame capture should log the calls as they are being played
    /// I't helps tell at what point did the capture stopped working
    BoolCommandResponse m_LogCallsAsItReplays;

protected:

    mutex m_CaptureLayerMutex; ///< Mutex used in capturing

    /// Stream of captured calls
    CapturedAPICalls m_CapturedAPICalls;

    /// Command to collect the API timings
    CommandResponse m_TimingLog;

    /// Command to collect the API Trace
    CommandResponse m_CaptureLog;

    /// Command to collect the API Trace
    CommandResponse m_Resources;

    /// number of captured calls that need to happen for the server to ping the client
    ULongCommandResponse m_dwCaptureUpdateFrequency;

    /// Command to capture a frame
    CaptureCommandResponse m_Capture;

    /// Command to capture a frame
    FloatCommandResponse m_Release;

    /// Command to capture buffer locks
    BoolCommandResponse m_CaptureBufferLocks;

    /// Command to capture and play immediately
    BoolCommandResponse m_CaptureAndPlay;

    /// Command to override clear color to red
    BoolCommandResponse m_ClearInRed;

    /// Command to flatten command lists
    BoolCommandResponse m_FlattenCommandLists;

    /// Indicates whether to replay the frame while in the replay loop
    BoolCommandResponse m_ReplayFrame;

    /// Command to enable skippoint
    BoolCommandResponse m_EnableSkipPoint;

    /// frame capture will execute the calls in the capture up to the skip point
    ULongCommandResponse m_dwSkipPoint;

    /// Command to handle mapped buffer copies on the CPU
    BoolCommandResponse m_HandleMapsOnCPU;

    /// Indicates if the capture is flattened (unused)
    bool m_CaptureIsFlattened;

    /// Map delta optimization manager
    MdoManager* m_pMdoManager;

private:

    /// Command to enable the replay loop
    BoolCommandResponse m_ReplayLoop;

    /// A mask indicating which non-draw/dispatch calls are being filtered out. A value of 0 indicates no draw call filtering.
    /// Eventually, this will be extended to a bitmask, with each bit corresponding to a drawcall type
    IntCommandResponse m_filterDrawcallMask;
};

/// Override that times all the executed calls
class Capture_Timing : public CaptureOverride
{
    /// Instance that will hold all the timings
    TimingLog* m_pTiming;

    /// indicated whether to log the ThreadID of the captured call or the threadID when the capture its being replayed
    bool m_UseCapturedThreadID;

public:
    /// initilizes variables
    /// \param pTiming instance that will hold the timings
    Capture_Timing(TimingLog* pTiming)
    {
        m_pTiming = pTiming;
        m_UseCapturedThreadID = true;
    }

    /// simple accessor
    /// \param value true to use the captured thread id's
    void SetUseCaptureThreadID(bool value)
    {
        m_UseCapturedThreadID = value;
    }

    /// callback that logs the timing
    /// \param pCap the captured that is about to be replayed
    /// \return The same value that returns the replayed call
    virtual bool Do(Capture* pCap)
    {
        LARGE_INTEGER startTime = m_pTiming->GetRaw();

        bool result = pCap->Play();

        if (m_UseCapturedThreadID)
        {
            m_pTiming->Add(pCap->GetThreadID(), startTime);
        }
        else
        {
            m_pTiming->Add(osGetCurrentThreadId(), startTime);
        }

        return result;
    }
};

/// Override that skips all calls from a given skip point
class Capture_Skip : public CaptureOverride
{
    /// Counts the number of capture calls made during a frame replay
    DWORD m_CallCount;

    /// Call index after which all calls will be skipped
    DWORD m_SkipPoint;

public:

    /// initialize values
    Capture_Skip()
    {
        m_SkipPoint = DWORD(-1);
        m_CallCount = 0;
    }

    /// Sets skip point
    /// \param SkipPoint call number from where all calls will be skipped
    void SetSkipPoint(DWORD SkipPoint)
    {
        m_SkipPoint = SkipPoint;
    }

    /// callback that skips the capture calls
    /// if they go beyong the skip point
    /// \param pCap the captured that is about to be replayed
    /// \return true if the call was replayed, false if skipped
    virtual bool Do(Capture* pCap)
    {
        bool result = true;

        if (m_CallCount < m_SkipPoint)
        {
            result = pCap->Play();
        }

        m_CallCount++;

        return result;
    }
};

#endif //FRAMECAPTURELAYERCOMMOM_H

