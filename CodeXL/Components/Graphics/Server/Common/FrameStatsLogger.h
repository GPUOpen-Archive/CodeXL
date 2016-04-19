//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A utility class that tracks frame time stats and logs them to a file.
//==============================================================================

#ifndef FRAMESTATSLOGGER_H
#define FRAMESTATSLOGGER_H

#include "CommonTypes.h"
#include "SharedGlobal.h"
#include "timer.h"

/// Used to specify the different types of frame statistics data.
typedef enum
{
    STAT_FILE_MINMAX,
    STAT_FILE_FRAMETIMES,
    STAT_FILE_FPS
} StatFileType;

//--------------------------------------------------------------------------
/// The FrameStatsLogger provides GPS with the ability to track frame time statistics.
/// Every once in a while the stats will be dumped to a file in the temp directory.
//--------------------------------------------------------------------------
class FrameStatsLogger
{
public:
    //--------------------------------------------------------------------------
    /// Default constructor for FrameStatsLogger.
    //--------------------------------------------------------------------------
    FrameStatsLogger();

    //--------------------------------------------------------------------------
    /// Default destructor for FrameStatsLogger.
    //--------------------------------------------------------------------------
    ~FrameStatsLogger() {}

    //--------------------------------------------------------------------------
    /// Initialize the frame statistics logger.
    //--------------------------------------------------------------------------
    void Initialize();

    //--------------------------------------------------------------------------
    /// Reset the frame statistics.
    //--------------------------------------------------------------------------
    void Reset();

    //--------------------------------------------------------------------------
    /// Collect stats for the current frame, and add the data to the history.
    //--------------------------------------------------------------------------
    void UpdateStats();

    //--------------------------------------------------------------------------
    /// Dump the frame statistics to the log file.
    //--------------------------------------------------------------------------
    void WriteStatsToLogFile();

    //--------------------------------------------------------------------------
    /// Dump an individual stat file to disk
    //--------------------------------------------------------------------------
    void WriteStatFile(StatFileType statType, char* fileName);

private:
    //--------------------------------------------------------------------------
    /// The timer used to collect frame duration.
    //--------------------------------------------------------------------------
    Timer mFrameTimer;

    //--------------------------------------------------------------------------
    /// The average observed FPS in the last second.
    //--------------------------------------------------------------------------
    double mRunningAverageFPS;

    //--------------------------------------------------------------------------
    /// A count of the number of times the average FPS has been computed.
    //--------------------------------------------------------------------------
    UINT mNumCalculations;

    //--------------------------------------------------------------------------
    /// The minimum observed FPS.
    //--------------------------------------------------------------------------
    double mMinFPS;

    //--------------------------------------------------------------------------
    /// The maximum observed FPS.
    //--------------------------------------------------------------------------
    double mMaxFPS;

    //--------------------------------------------------------------------------
    /// The total number of milliseconds that the stats collector has been measuring.
    //--------------------------------------------------------------------------
    double mTotalMilliseconds;

    //--------------------------------------------------------------------------
    /// Record the CPU time of the first frame the monitored period.
    //--------------------------------------------------------------------------
    double mInitialFrameTime;

    //--------------------------------------------------------------------------
    /// The total number of milliseconds to collect framerate statistics for.
    //--------------------------------------------------------------------------
    double mTargetMilliseconds;

    //--------------------------------------------------------------------------
    /// The number of frames rendered during collection.
    //--------------------------------------------------------------------------
    UINT mFrameCount;

    //--------------------------------------------------------------------------
    /// The Virtual-Key Code used as the trigger to start collecting stats.
    //--------------------------------------------------------------------------
    UINT mCollectionTrigger;

    //--------------------------------------------------------------------------
    /// A flag that determines if stats collection is currently active.
    //--------------------------------------------------------------------------
    bool mbCollectingStats;

    /// Used to record the frame timings
    vector<double> mFrameTimings;

    /// Used to record the FPS for each second of the collection period
    vector<double> mFPS;
};

#endif // FRAMESTATSLOGGER_H