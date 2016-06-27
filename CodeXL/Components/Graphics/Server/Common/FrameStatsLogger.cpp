//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A utility class that tracks frame time statistics and logs them to a file.
//==============================================================================

#include "FrameStatsLogger.h"
#include "SharedGlobal.h"

#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osTime.h>

//--------------------------------------------------------------------------
/// Default constructor for FrameStatsLogger.
//--------------------------------------------------------------------------
FrameStatsLogger::FrameStatsLogger()
    : mRunningAverageFPS(0.0)
    , mNumCalculations(0)
    , mMinFPS(FLT_MAX)
    , mMaxFPS(FLT_MIN)
    , mTotalMilliseconds(0.0)
    , mTargetMilliseconds(0.0)
    , mFrameCount(0)
    , mCollectionTrigger(0)
    , mbCollectingStats(false)
{
}

//--------------------------------------------------------------------------
/// Initialize the frame statistics logger.
//--------------------------------------------------------------------------
void FrameStatsLogger::Initialize()
{
    mTargetMilliseconds = (double)SG_GET_UINT(OptionStatsDuration);
    mCollectionTrigger = SG_GET_UINT(OptionStatsTrigger);
    Reset();
}

//--------------------------------------------------------------------------
/// Reset the frame statistics.
//--------------------------------------------------------------------------
void FrameStatsLogger::Reset()
{
    mMinFPS = FLT_MAX;
    mMaxFPS = FLT_MIN;
    mFrameCount = 0;
    mNumCalculations = 0;
    mRunningAverageFPS = 0.0;
    mTotalMilliseconds = 0.0;
    mFrameTimings.clear();
    mFPS.clear();
}

//--------------------------------------------------------------------------
/// Collect statistics for the current frame, and add the data to the history.
//--------------------------------------------------------------------------
void FrameStatsLogger::UpdateStats()
{
    // If we aren't collecting statistics yet, and there's a keypress, start collecting.
#ifdef WIN32
    bool bKeyPressed = (GetKeyState(mCollectionTrigger) & 0x8000) != 0;
#else
    // for now, always collect stats on Linux
    bool bKeyPressed = true;
#endif

    bool bResetInitialOffset = false;

    if (!mbCollectingStats && bKeyPressed)
    {
        mbCollectingStats = true;
        bResetInitialOffset = true;
        Reset();
    }

    // Update the statistics if we're collecting them.
    if (mbCollectingStats)
    {
        double frameMilliseconds = mFrameTimer.LapDouble();

        if (bResetInitialOffset == true)
        {
            mInitialFrameTime = frameMilliseconds;
            bResetInitialOffset = false;
        }

        // Add to the running duration so we know when to stop collecting FPS info.
        mTotalMilliseconds += frameMilliseconds;

        // Record the frame timing data
        mFrameTimings.push_back(mTotalMilliseconds - mInitialFrameTime);

        static double timeSinceLastUpdate = 0.0;
        static int numFramesInLastSecond = 0;

        // It has been one second since the last FPS calculation. Compute it again based on the last second of data.
        if (timeSinceLastUpdate > 1000.0f)
        {
            double currentAverage = static_cast<double>(numFramesInLastSecond) / (timeSinceLastUpdate / 1000.0);

            mFPS.push_back(currentAverage);

            if (currentAverage < mMinFPS)
            {
                mMinFPS = currentAverage;
            }

            if (currentAverage > mMaxFPS)
            {
                mMaxFPS = currentAverage;
            }

            // Increment the total FPS sum, and increment the number of times the FPS was computed.
            mRunningAverageFPS += currentAverage;
            mNumCalculations++;

            // Reset the time since we last computed the FPS.
            timeSinceLastUpdate = 0.0;
            numFramesInLastSecond = 0;
        }

        // Add the latest frametime to count the last second, and increment the frame count.
        timeSinceLastUpdate += frameMilliseconds;
        numFramesInLastSecond++;
        mFrameCount++;

        // Stop collecting and write the log file after a minute.
        if (mTotalMilliseconds >= mTargetMilliseconds)
        {
            mbCollectingStats = false;
            WriteStatsToLogFile();
        }
    }

    mFrameTimer.ResetTimer();
}

//--------------------------------------------------------------------------
/// Write the frame statistics to the log file.
//--------------------------------------------------------------------------
void FrameStatsLogger::WriteStatsToLogFile()
{
    WriteStatFile(STAT_FILE_MINMAX, "minmaxavg.csv");
    WriteStatFile(STAT_FILE_FRAMETIMES, "frametimes.csv");
    WriteStatFile(STAT_FILE_FPS, "fps.csv");
}

void FrameStatsLogger::WriteStatFile(StatFileType statType, const char* fileName)
{
    osModuleArchitecture moduleArchitecture;
    osRuntimePlatform currentPlatform;
    gtString executablePath;
    gtString commandLine;
    gtString workingDirectory;

    if (osGetProcessLaunchInfo(osGetCurrentProcessId(), moduleArchitecture, currentPlatform, executablePath, commandLine, workingDirectory) == false)
    {
        Log(logERROR, "FrameStatsLogger::WriteStatFile: Failed to retrieve process launch info.\n");
        return;
    }

    osFilePath executableFilepath;
    executableFilepath.setFullPathFromString(executablePath);

    gtString appName;

    if (executableFilepath.getFileName(appName) == false)
    {
        Log(logERROR, "FrameStatsLogger::WriteStatFile: Failed to retrieve application filename.\n");
        return;
    }

    osTime currentTime;
    currentTime.setFromCurrentTime();
    tm timeStruct;
    currentTime.timeAsTmStruct(timeStruct, osTime::LOCAL);

    // Need to add 1900, since tm contains "years since 1900".
    int year = timeStruct.tm_year + 1900;

    // Need to add 1, since tm contains "months since January".
    int month = timeStruct.tm_mon + 1;
    int day = timeStruct.tm_mday;

    int hour = timeStruct.tm_hour;
    int minute = timeStruct.tm_min;
    int second = timeStruct.tm_sec;

    gtASCIIString asciiStatsFilename;

    // ExecutableFilename YEAR-MM-DD HOUR-MINUTE-SECOND-??? minmaxavg.csv
    asciiStatsFilename.appendFormattedString("%s %d-%d-%d %d-%d-%d-0 %s",
                                             appName.asASCIICharArray(),
                                             year, month, day,
                                             hour, minute, second, fileName);

    // Build a path to the GPS folder within the Temp directory.
    osFilePath statsFileDirectory;
    statsFileDirectory.setPath(osFilePath::OS_TEMP_DIRECTORY);

    gtString gpsFolder;
    gpsFolder.fromASCIIString(GetPerfStudioDirName());
    statsFileDirectory.appendSubDirectory(gpsFolder);

    gtASCIIString fullStatsFilepath = statsFileDirectory.asString().asASCIICharArray();
    fullStatsFilepath.appendFormattedString("\\%s", asciiStatsFilename.asCharArray());

    gtString fullStatsFilePathAsGtString;
    fullStatsFilePathAsGtString.fromASCIIString(fullStatsFilepath.asCharArray());

    osFile statsFile(fullStatsFilePathAsGtString);
    bool bFileOpened = statsFile.open(osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

    if (bFileOpened == false)
    {
        gtString fullPath;
        statsFileDirectory.getFileNameAndExtension(fullPath);
        Log(logERROR, "Failed to open statistics file '%s' for writing.\n", fullPath.asASCIICharArray());
        return;
    }

    switch (statType)
    {
        case STAT_FILE_MINMAX:
        {
            // Write the stats header.
            gtString headerLine;
            headerLine.appendFormattedString(L"Frames, Time (ms), Min, Max, Avg\n");
            statsFile.writeString(headerLine);

            // Compute the average FPS by averaging the per-frame FPS averages.
            double durationAverageFPS = mRunningAverageFPS / static_cast<double>(mNumCalculations);

            // Write the data.
            gtString dataLine;
            dataLine.appendFormattedString(L"%d, %f, %f, %f, %f\n", mFrameCount, mTotalMilliseconds, mMinFPS, mMaxFPS, durationAverageFPS);
            statsFile.writeString(dataLine);
            break;
        }

        case STAT_FILE_FRAMETIMES:
        {
            // Write the stats header.
            gtString headerLine;
            headerLine.appendFormattedString(L"Frame, Time (ms)\n");
            statsFile.writeString(headerLine);

            for (UINT32 frameIndex = 0; frameIndex < mFrameTimings.size(); frameIndex++)
            {
                // Write the data.
                gtString dataLine;
                dataLine.appendFormattedString(L"%d, %f\n", frameIndex + 1, mFrameTimings[frameIndex]);
                statsFile.writeString(dataLine);
            }

            break;
        }

        case STAT_FILE_FPS:
        {
            // Write the stats header.
            gtString headerLine;
            headerLine.appendFormattedString(L"FPS\n");
            statsFile.writeString(headerLine);

            for (UINT32 fpsIndex = 0; fpsIndex < mFPS.size(); fpsIndex++)
            {
                // Write the data.
                gtString dataLine;
                dataLine.appendFormattedString(L"%ld\n", (long)mFPS[fpsIndex]);
                statsFile.writeString(dataLine);
            }

            break;
        }
    }

    statsFile.close();
}
