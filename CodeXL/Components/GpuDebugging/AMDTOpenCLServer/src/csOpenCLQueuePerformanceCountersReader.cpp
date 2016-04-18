//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csOpenCLQueuePerformanceCountersReader.cpp
///
//==================================================================================

//------------------------------ csOpenCLQueuePerformanceCountersReader.cpp ------------------------------

// ANSI C:
#include <memory.h>
#include <stdlib.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osTimer.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTAPIClasses/Include/apCLEnqueuedCommands.h>
#include <AMDTAPIClasses/Include/apOpenCLQueuePerformanceCounters.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>

// Local:
#include <src/csCommandQueueMonitor.h>
#include <src/csOpenCLQueuePerformanceCountersReader.h>

#define CS_COMMAND_QUEUE_READER_MIN_NUMBER_OF_COMMANDS_USED 50
#define CS_COMMAND_QUEUE_READER_MAX_NUMBER_OF_COMMANDS_USED 100


// ----------------------------------------------------------------------------------
// Class Name:          csOpenCLQueueTimeSection
// General Description: Assistance struct used to calculate total times
// Author:              Uri Shomroni
// Creation Date:       10/3/2010
// ----------------------------------------------------------------------------------
struct csOpenCLQueueTimeSection
{
public:
    csOpenCLQueueTimeSection() : _startTime(0), _endTime(0) {};
    csOpenCLQueueTimeSection(const apCLEnqueuedCommand& commandToRead);
    gtUInt64 sectionLength() const {return (_endTime > _startTime) ? (_endTime - _startTime) : 0;};
    csOpenCLQueueTimeSection& operator=(const csOpenCLQueueTimeSection& other) {_startTime = other._startTime; _endTime = other._endTime; return *this;};
    bool doesIntersect(const csOpenCLQueueTimeSection& other) const;
    void unify(const csOpenCLQueueTimeSection& other);

public:
    gtUInt64 _startTime;
    gtUInt64 _endTime;
};

// ---------------------------------------------------------------------------
// Name:        csOpenCLQueueTimeSection::csOpenCLQueueTimeSection
// Description: Constructor
// Author:      Uri Shomroni
// Date:        10/3/2010
// ---------------------------------------------------------------------------
csOpenCLQueueTimeSection::csOpenCLQueueTimeSection(const apCLEnqueuedCommand& commandToRead)
    : _startTime(commandToRead.executionStartedTime()), _endTime(commandToRead.executionEndedTime())
{
    if (_startTime > _endTime)
    {
        _startTime = _endTime;
    }
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLQueueTimeSection::doesIntersect
// Description: Returns true iff this section and other intersect as closed sections
// Author:      Uri Shomroni
// Date:        10/3/2010
// ---------------------------------------------------------------------------
bool csOpenCLQueueTimeSection::doesIntersect(const csOpenCLQueueTimeSection& other) const
{
    bool retVal = false;

    // We only assert in debug builds, as this is called O(n^2) in the calculation loop
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    GT_ASSERT(_startTime <= _endTime);
    GT_ASSERT(other._startTime <= other._endTime);
#endif

    // The sections intersect if one doesn't end before the other starts:
    if (!((other._endTime < _startTime) || (_endTime < other._startTime)))
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLQueueTimeSection::unify
// Description: Set this section to be a union of its current values and the other section
// Author:      Uri Shomroni
// Date:        10/3/2010
// ---------------------------------------------------------------------------
void csOpenCLQueueTimeSection::unify(const csOpenCLQueueTimeSection& other)
{
    // We only allow unifying sections that intersect:
    bool rcInters = doesIntersect(other);

    // We only assert in debug builds, as this is called O(n) (worst case) in the calculation loop
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    GT_ASSERT(rcInters);
#endif

    if (rcInters)
    {
        if (_startTime > other._startTime)
        {
            _startTime = other._startTime;
        }

        if (_endTime < other._endTime)
        {
            _endTime = other._endTime;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csIntegrateTimeSectionInVector
// Description: Integrates newSection into sectionsVector, so that newSection and
//              all existing sections that intersect it are melded into one new section
//              which is their union.
//              sectionsVector is an in/out parameter, and it is assumed all sections
//              in the vector are disjoint.
// Author:      Uri Shomroni
// Date:        10/3/2010
// ---------------------------------------------------------------------------
void csIntegrateTimeSectionInVector(csOpenCLQueueTimeSection sectionsVector[CS_COMMAND_QUEUE_READER_MAX_NUMBER_OF_COMMANDS_USED], int& amountOfSectionsInVector, const csOpenCLQueueTimeSection& newSection)
{
    // Go over the existing sections to see which ones need to get melded:
    csOpenCLQueueTimeSection meldedSection = newSection;

    // We will overwrite the existing vector items, to save performance:
    int newVectorIndex = 0;

    for (int i = 0; i < amountOfSectionsInVector; i++)
    {
        const csOpenCLQueueTimeSection& currentSection = sectionsVector[i];

        // Try to intersect this section with the melded section we are making:
        if (meldedSection.doesIntersect(currentSection))
        {
            // Meld it into the melded section:
            meldedSection.unify(currentSection);
        }
        else // !meldedSection.doesIntersect(currentSection)
        {
            // This section is disjoint from our melded section, just write it to the new vector (if it is actually a new item):
            if (i != newVectorIndex)
            {
                sectionsVector[newVectorIndex] = sectionsVector[i];
            }

            // Advance the index:
            newVectorIndex++;
        }
    }

    // Add the melded item as the next object and dispose of any excessive items in the vector.
    // If the index is the same as the number of objects originally in the vector, it means all the sections are disjoint
    // from the new section, and the vector was not changed at all:
    if (newVectorIndex == amountOfSectionsInVector)
    {
        // Add the melded section to the new vector:
        sectionsVector[newVectorIndex] = meldedSection;

        newVectorIndex++;
    }
    else // newVectorIndex != numberOfSectionsInOriginalVector
    {
        // The new vector index can not be higher than the number of items in the original vector,
        // but we make a sanity check just in case:
        if (newVectorIndex < CS_COMMAND_QUEUE_READER_MAX_NUMBER_OF_COMMANDS_USED)
        {
            // Copy the melded section to the vector:
            sectionsVector[newVectorIndex] = meldedSection;

            // Note The original vector might have been longer (multiple sections were melded into the new one), but we "remove" the extra items
            // but having the new size variable smaller. However, we only do this if the size is small enough to begin with:
            newVectorIndex++;
        }
    }

    // The new number of sections is the last used new vector index, plus one (for the melded section):
    amountOfSectionsInVector = newVectorIndex;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuePerformanceCountersReader::csOpenCLQueuePerformanceCountersReader
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        9/3/2010
// ---------------------------------------------------------------------------
csOpenCLQueuePerformanceCountersReader::csOpenCLQueuePerformanceCountersReader()
    : _pCommandQueueMonitor(NULL), _pCountersValues(NULL), _totalTime(0), _totalBusyTime(0), _busyPercentage(0.0), _totalBusyTimeWithDuplicates(0),
      _workItemCount(0), _readSize(0), _writeSize(0), _copySize(0),
      _lastRealFPSValue(0), _endedFramesTimeIntervalsSum(0), _frameCounter(0)
{
    // Start the stop watch run:
    _fpsStopWatch.start();
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuePerformanceCountersReader::~csOpenCLQueuePerformanceCountersReader
// Description: Destructor.
// Author:      Sigal Algranaty
// Date:        9/3/2010
// ---------------------------------------------------------------------------
csOpenCLQueuePerformanceCountersReader::~csOpenCLQueuePerformanceCountersReader()
{
    if (_fpsStopWatch.isRunning())
    {
        _fpsStopWatch.stop();
    }

}

// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuePerformanceCountersReader::updateCounterValues
// Description: Updates the counters values into the counter values snapshot.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/3/2010
// ---------------------------------------------------------------------------
bool csOpenCLQueuePerformanceCountersReader::updateCounterValues()
{
    bool retVal = true;

    // Verify that we have a place to store the counter values:
    GT_IF_WITH_ASSERT(_pCountersValues != NULL)
    {
        // Get the amount of seconds since the last call:
        double amountOfSeconds = 0;
        _timeSinceLastCounterValuesUpdate.getTimeInterval(amountOfSeconds);
        _timeSinceLastCounterValuesUpdate.stop();
        _timeSinceLastCounterValuesUpdate.start();

        // Try to update the time totals. Note that if we happen to call this
        // just as the queue commands were cleared, we will fail here - so don't assert the value:
        bool timesUpdated = updateTimeTotals();

        if (timesUpdated)
        {
            // Reset counters values:
            memset(_pCountersValues, 0, AP_COMMAND_QUEUE_COUNTERS_AMOUNT * sizeof(int));

            // Get the frames / sec counter value:
            _pCountersValues[AP_QUEUE_FPS_COUNTER] = calculateAvarageFramePerSecondRatio();

            // Get kernel utilization value:
            _pCountersValues[AP_KERNEL_COMMANDS_QUEUE_UTILIZATION_COUNTER] = calculateKernelUtilization();

            // Get write utilization value:
            _pCountersValues[AP_WRITE_COMMANDS_QUEUE_UTILIZATION_COUNTER] = calculateWriteUtilization();

            // Get copy utilization value:
            _pCountersValues[AP_COPY_COMMANDS_QUEUE_UTILIZATION_COUNTER] = calculateCopyUtilization();

            // Get read utilization value:
            _pCountersValues[AP_READ_COMMANDS_QUEUE_UTILIZATION_COUNTER] = calculateReadUtilization();

            // Get other utilization value:
            _pCountersValues[AP_OTHER_COMMANDS_QUEUE_UTILIZATION_COUNTER] = calculateOtherUtilization();

            // Get busy value:
            _pCountersValues[AP_QUEUE_BUSY_COUNTER] = _busyPercentage;

            // Get idle value:
            _pCountersValues[AP_QUEUE_IDLE_COUNTER] = calculateIdleTime();

            // Get work item size per second value:
            _pCountersValues[AP_QUEUE_WORK_ITEM_SIZE_PER_SECOND] = calculateWorkItemSize(amountOfSeconds);

            // Get read size per second value:
            _pCountersValues[AP_QUEUE_READ_BYTES_PER_SECOND] = calculateReadSize(amountOfSeconds);

            // Get write size per second value:
            _pCountersValues[AP_QUEUE_WRITE_BYTES_PER_SECOND] = calculateWriteSize(amountOfSeconds);

            // Get copy size per second value:
            _pCountersValues[AP_QUEUE_COPY_BYTES_PER_SECOND] = calculateCopySize(amountOfSeconds);

            // Restart the periodic counters:
            restartPeriodicCounters();

        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuePerformanceCountersReader::setCounterValuesPointer
// Description: Sets the counter values pointer
// Arguments: double* pCounterValues
// Return Val: void
// Author:      Sigal Algranaty
// Date:        9/3/2010
// ---------------------------------------------------------------------------
void csOpenCLQueuePerformanceCountersReader::setCounterValuesPointer(double* pCounterValues)
{
    // Set the counters new value pointer:
    _pCountersValues = pCounterValues;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuePerformanceCountersReader::updateTimeTotals
// Description: Updates the total times values by conferring with the command queue
//              monitor
// Author:      Uri Shomroni
// Date:        10/3/2010
// ---------------------------------------------------------------------------
bool csOpenCLQueuePerformanceCountersReader::updateTimeTotals()
{
    bool retVal = false;

    // Reset all the values to 0:
    _totalTime = 0;
    _totalBusyTime = 0;
    _busyPercentage = 0.0;
    _totalBusyTimeWithDuplicates = 0;

    for (int cat = 0; cat < AP_NUMBER_OF_QUEUE_COMMAND_CATEGORIES; cat++)
    {
        _busyTimeByCategory[cat] = 0;
    }

    // Sanity check:
    GT_IF_WITH_ASSERT(_pCommandQueueMonitor != NULL)
    {
        // Don't allow the command queue monitor to change the data "under our feet":
        _pCommandQueueMonitor->beforeAPIThreadDataAccess();

        // Update the queue times:
        bool rcUpd = _pCommandQueueMonitor->updateQueueTimesIfDeviceAllowsProfiling();

        if (rcUpd)
        {
            int numberOfCommandsInQueue = _pCommandQueueMonitor->amountOfCommandsInQueue();

            // If we have enough commands to calculate the sample size:
            if (numberOfCommandsInQueue >= CS_COMMAND_QUEUE_READER_MIN_NUMBER_OF_COMMANDS_USED)
            {
                // We use a static vector (with a size variable to make its size "dynamic", but limited), to aid us in calculating the special times
                csOpenCLQueueTimeSection totalBusySections[CS_COMMAND_QUEUE_READER_MAX_NUMBER_OF_COMMANDS_USED];
                int numberOfSectionsInBusySectionsVector = 0;

                // Decide where to start:
                int firstCommandIndex = numberOfCommandsInQueue - CS_COMMAND_QUEUE_READER_MAX_NUMBER_OF_COMMANDS_USED;

                if (firstCommandIndex < 0)
                {
                    firstCommandIndex = 0;
                }

                // Iterate the commands:
                for (int i = firstCommandIndex; i < numberOfCommandsInQueue; i++)
                {
                    // Pointer sanity check:
                    const apCLEnqueuedCommand* pCurrentCommand = _pCommandQueueMonitor->getEnqueuedCommandDetails(i);
                    GT_IF_WITH_ASSERT(pCurrentCommand != NULL)
                    {
                        // If this command has finished:
                        if (pCurrentCommand->areTimesUpdated())
                        {
                            // Ignore the CodeXL-generated idles:
                            osTransferableObjectType currentCommandType = pCurrentCommand->type();

                            if (currentCommandType != OS_TOBJ_ID_CL_QUEUE_IDLE_TIME)
                            {
                                // Get the category for this command:
                                apOpenCLQueueCommandsCategories currentCommandCategory = apOpenCLQueueCommandsCategoryFromTransferableObjectType(currentCommandType);

                                // Create a time section from it:
                                csOpenCLQueueTimeSection currentCommandTimeSection(*pCurrentCommand);

                                // Meld it into the busy sections:
                                csIntegrateTimeSectionInVector(totalBusySections, numberOfSectionsInBusySectionsVector, currentCommandTimeSection);

                                // Add its length where needed:
                                gtUInt64 commandTimeSectionLength = currentCommandTimeSection.sectionLength();
                                _totalBusyTimeWithDuplicates += commandTimeSectionLength;
                                _busyTimeByCategory[currentCommandCategory] += commandTimeSectionLength;
                            }
                        }
                    }
                }

                // Get the total times (without duplicates):
                gtUInt64 earliestStartTime = 0;
                gtUInt64 latestEndTime = 0;

                for (int sec = 0; sec < numberOfSectionsInBusySectionsVector; sec++)
                {
                    const csOpenCLQueueTimeSection& currentSection = totalBusySections[sec];

                    // Stretch the limit values:
                    if ((sec == 0) || (currentSection._startTime < earliestStartTime))
                    {
                        earliestStartTime = currentSection._startTime;
                    }

                    if ((sec == 0) || (currentSection._endTime > latestEndTime))
                    {
                        latestEndTime = currentSection._endTime;
                    }

                    // Add this section to the total busy time:
                    _totalBusyTime += currentSection.sectionLength();
                }

                // Set the total time span:
                _totalTime = (earliestStartTime < latestEndTime) ? (latestEndTime - earliestStartTime) : 0;

                // If we have a total time:
                if (_totalTime > 0)
                {
                    // Calculate the busy percentage:
                    _busyPercentage = ((double)_totalBusyTime / (double)_totalTime) * 100.0;

                    // This also means we succeeded:
                    retVal = true;
                }
            }
        }
        else
        {
            // Assert if this is the first time this happens:
            static bool wasUpdateFailureReported = false;

            if (!wasUpdateFailureReported)
            {
                wasUpdateFailureReported = true;
                GT_ASSERT(rcUpd);
            }
        }

        // Re-allow the command queue monitor to access its vector:
        _pCommandQueueMonitor->afterAPIThreadDataAccess();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuePerformanceCountersReader::calculateKernelUtilization
// Description: Calculate kernel utilization counter value
// Return Val:  double
// Author:      Sigal Algranaty
// Date:        9/3/2010
// ---------------------------------------------------------------------------
double csOpenCLQueuePerformanceCountersReader::calculateKernelUtilization()
{
    double retVal = 0.0;

    if (_totalBusyTimeWithDuplicates > 0)
    {
        // We consider this value to be the relative part of kernels in the total busy percentage.
        // While in cases of out of order execution this is not 100% accurate, it is a good estimate:
        retVal = ((double)(_busyTimeByCategory[AP_KERNEL_QUEUE_COMMANDS]) / (double)_totalBusyTimeWithDuplicates) * _busyPercentage;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuePerformanceCountersReader::calculateWriteUtilization
// Description: Calculate write utilization counter value
// Return Val:  double
// Author:      Sigal Algranaty
// Date:        9/3/2010
// ---------------------------------------------------------------------------
double csOpenCLQueuePerformanceCountersReader::calculateWriteUtilization()
{
    double retVal = 0.0;

    if (_totalBusyTimeWithDuplicates > 0)
    {
        // We consider this value to be the relative part of writes in the total busy percentage.
        // While in cases of out of order execution this is not 100% accurate, it is a good estimate:
        retVal = ((double)(_busyTimeByCategory[AP_WRITE_QUEUE_COMMANDS]) / (double)_totalBusyTimeWithDuplicates) * _busyPercentage;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuePerformanceCountersReader::calculateCopyUtilization
// Description: Calculate copy utilization counter value
// Return Val:  double
// Author:      Sigal Algranaty
// Date:        9/3/2010
// ---------------------------------------------------------------------------
double csOpenCLQueuePerformanceCountersReader::calculateCopyUtilization()
{
    double retVal = 0.0;

    if (_totalBusyTimeWithDuplicates > 0)
    {
        // We consider this value to be the relative part of copies in the total busy percentage.
        // While in cases of out of order execution this is not 100% accurate, it is a good estimate:
        retVal = ((double)(_busyTimeByCategory[AP_COPY_QUEUE_COMMANDS]) / (double)_totalBusyTimeWithDuplicates) * _busyPercentage;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuePerformanceCountersReader::calculateReadUtilization
// Description: Calculate read utilization counter value
// Return Val:  double
// Author:      Sigal Algranaty
// Date:        9/3/2010
// ---------------------------------------------------------------------------
double csOpenCLQueuePerformanceCountersReader::calculateReadUtilization()
{
    double retVal = 0.0;

    if (_totalBusyTimeWithDuplicates > 0)
    {
        // We consider this value to be the relative part of reads in the total busy percentage.
        // While in cases of out of order execution this is not 100% accurate, it is a good estimate:
        retVal = ((double)(_busyTimeByCategory[AP_READ_QUEUE_COMMANDS]) / (double)_totalBusyTimeWithDuplicates) * _busyPercentage;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuePerformanceCountersReader::calculateOtherUtilization
// Description: Calculate other utilization counter value
// Return Val:  double
// Author:      Sigal Algranaty
// Date:        9/3/2010
// ---------------------------------------------------------------------------
double csOpenCLQueuePerformanceCountersReader::calculateOtherUtilization()
{
    double retVal = 0.0;

    if (_totalBusyTimeWithDuplicates > 0)
    {
        // We consider this value to be the relative part of other commands in the total busy percentage.
        // While in cases of out of order execution this is not 100% accurate, it is a good estimate:
        retVal = ((double)(_busyTimeByCategory[AP_OTHER_QUEUE_COMMANDS]) / (double)_totalBusyTimeWithDuplicates) * _busyPercentage;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuePerformanceCountersReader::calculateIdleTime
// Description: Calculate idle time counter value
// Return Val:  double
// Author:      Sigal Algranaty
// Date:        9/3/2010
// ---------------------------------------------------------------------------
double csOpenCLQueuePerformanceCountersReader::calculateIdleTime()
{
    double retVal = 0.0;

    // If we have any sort of values
    if (_totalTime > 0)
    {
        retVal = 100.0 - _busyPercentage;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuePerformanceCountersReader::calculateWorkItemSize
// Description: Calculate work item size counter value
// Return Val:  double
// Author:      Sigal Algranaty
// Date:        5/5/2010
// ---------------------------------------------------------------------------
double csOpenCLQueuePerformanceCountersReader::calculateWorkItemSize(double amountOfSeconds)
{
    double retVal = 0.0;

    if (amountOfSeconds > 0)
    {
        // Get the value normalized per second:
        retVal = _workItemCount / amountOfSeconds;

        // Reset the value for next sample:
        _workItemCount = 0;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuePerformanceCountersReader::calculateReadSize
// Description: Calculate read size counter value
// Return Val:  double
// Author:      Sigal Algranaty
// Date:        5/5/2010
// ---------------------------------------------------------------------------
double csOpenCLQueuePerformanceCountersReader::calculateReadSize(double amountOfSeconds)
{
    double retVal = 0.0;

    if (amountOfSeconds > 0)
    {
        // Get the value normalized per second:
        retVal = _readSize / amountOfSeconds;

        // Reset the value for next sample:
        _readSize = 0;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuePerformanceCountersReader::calculateWriteSize
// Description: Calculate write size counter value
// Return Val:  double
// Author:      Sigal Algranaty
// Date:        5/5/2010
// ---------------------------------------------------------------------------
double csOpenCLQueuePerformanceCountersReader::calculateWriteSize(double amountOfSeconds)
{
    double retVal = 0.0;

    if (amountOfSeconds > 0)
    {
        // Get the value normalized per second:
        retVal = _writeSize / amountOfSeconds;

        // Reset the value for next sample:
        _writeSize = 0;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuePerformanceCountersReader::calculateCopySize
// Description: Calculate copy size counter value
// Return Val:  double
// Author:      Sigal Algranaty
// Date:        5/5/2010
// ---------------------------------------------------------------------------
double csOpenCLQueuePerformanceCountersReader::calculateCopySize(double amountOfSeconds)
{
    double retVal = 0.0;

    if (amountOfSeconds > 0)
    {
        // Get the value normalized per second:
        retVal = _copySize / amountOfSeconds;

        // Reset the value for next sample:
        _copySize = 0;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuePerformanceCountersReader::calculateAvarageFramePerSecondRatio
// Description: Returns the average frame / sec ration measured since the last call
//              to restartPeriodicCounters().
// Author:      Sigal Algranaty
// Date:        27/7/2010
// ---------------------------------------------------------------------------
double csOpenCLQueuePerformanceCountersReader::calculateAvarageFramePerSecondRatio()
{
    double retVal = _lastRealFPSValue;

    // If at least one frame ended:
    if (_endedFramesTimeIntervalsSum > 0)
    {
        // Calculate the average F/S:
        retVal = double(_frameCounter) / _endedFramesTimeIntervalsSum;
        _lastRealFPSValue = retVal;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuePerformanceCountersReader::restartPeriodicCounters
// Description: Restart periodic counters.
// Author:      Sigal Algranaty
// Date:        27/7/2010
// ---------------------------------------------------------------------------
void csOpenCLQueuePerformanceCountersReader::restartPeriodicCounters()
{
    // Restart the ended frames time intervals sum:
    _endedFramesTimeIntervalsSum = 0;

    // Restart the frames count:
    _frameCounter = 0;
}


// ---------------------------------------------------------------------------
// Name:        csOpenCLQueuePerformanceCountersReader::onFrameTerminatorCall
// Description: Is called when a frame terminator function call is executed
//              within the monitored context
// Author:      Sigal Algranaty
// Date:        27/7/2010
// ---------------------------------------------------------------------------
void csOpenCLQueuePerformanceCountersReader::onFrameTerminatorCall()
{
    // Lock the access to the threads common variables:
    osCriticalSectionLocker csLocker(_threadSyncCriticalSection);

    // Increment the frames count:
    _frameCounter++;

    // Calculate the time spent since the current frame started:
    double currentFrameTimeInterval = 0;
    _fpsStopWatch.getTimeInterval(currentFrameTimeInterval);

    // Add it to the ended frames time intervals sum:
    _endedFramesTimeIntervalsSum += currentFrameTimeInterval;

    // Restart the current frame time stop watch:
    _fpsStopWatch.start();

    // Unlock the access to the threads common variables:
    csLocker.leaveCriticalSection();
}


