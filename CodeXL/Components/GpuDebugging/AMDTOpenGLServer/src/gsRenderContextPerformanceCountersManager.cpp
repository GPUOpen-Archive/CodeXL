//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsRenderContextPerformanceCountersManager.cpp
///
//==================================================================================

//------------------------------ gsRenderContextPerformanceCountersManager.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>

// Local:
#include <src/gsTexturesMonitor.h>
#include <src/gsRenderContextMonitor.h>
#include <src/gsRenderContextPerformanceCountersManager.h>


// ---------------------------------------------------------------------------
// Name:        gsRenderContextPerformanceCountersManager::gsRenderContextPerformanceCountersManager
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        26/7/2005
// ---------------------------------------------------------------------------
gsRenderContextPerformanceCountersManager::gsRenderContextPerformanceCountersManager()
    : _pMonitoredRenderContext(NULL),
      _renderContextId(0),
      _frameCounter(0),
      _endedFramesTimeIntervalsSum(0),
      _currentFrameFunctionsCalls(0),
      _endedFramesFunctionCalls(0),
      _lastRealFPSValue(0),
      _lastRealFrameTimeValue(0),
      _lastRealOGLCallsValue(0)
{
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextPerformanceCountersManager::~gsRenderContextPerformanceCountersManager
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        26/7/2005
// ---------------------------------------------------------------------------
gsRenderContextPerformanceCountersManager::~gsRenderContextPerformanceCountersManager()
{
    // Stop the stop watch run:
    if (_stopWatch.isRunning())
    {
        _stopWatch.stop();
    }
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextPerformanceCountersManager::setMonitoredRenderContext
// Description: Sets my monitored render context.
// Author:      Yaki Tebeka
// Date:        12/9/2005
// ---------------------------------------------------------------------------
void gsRenderContextPerformanceCountersManager::setMonitoredRenderContext(gsRenderContextMonitor& monitoredRenderContext)
{
    // Store a pointer to my monitored render context monitor:
    _pMonitoredRenderContext = &monitoredRenderContext;

    if (_pMonitoredRenderContext)
    {
        // Get my render context Spy id:
        _renderContextId = _pMonitoredRenderContext->spyId();

        // If this is not the NULL context id:
        if (_renderContextId != 0)
        {
            // Start the stop watch run:
            _stopWatch.start();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextPerformanceCountersManager::updateCounterValues
// Description: Updates the counters values into the counter values snapshot.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        26/7/2005
// ---------------------------------------------------------------------------
bool gsRenderContextPerformanceCountersManager::updateCounterValues(double* pCountersValuesArray)
{
    bool retVal = true;

    // Verify that we have a place to store the counter values:
    GT_IF_WITH_ASSERT(pCountersValuesArray != NULL)
    {
        // Lock the access to the threads common variables:
        osCriticalSectionLocker csLocker(_threadSyncCriticalSection);

        // Update the frames / sec counter:
        pCountersValuesArray[GS_FPS_COUNTER] = getAvarageFramePerSecondRatio();

        // Update the frame time counter:
        pCountersValuesArray[GS_FRAME_TIME_COUNTER] = getAvarageFrameTime();

        // Update the function calls
        pCountersValuesArray[GS_FOUNCTION_CALLS_PS_COUNTER] = getAvarageFuncCallsPerFrameRatio();

        double amountOfTextureObjects = 0;
        double amountOfLoadedLevel0Texels = 0;

        if (_pMonitoredRenderContext)
        {
            // Get the render context textures monitor:
            const gsTexturesMonitor* texturesMonitor = _pMonitoredRenderContext->texturesMonitor();
            GT_IF_WITH_ASSERT(texturesMonitor != NULL)
            {
                // Get the texture objects amount:
                amountOfTextureObjects = texturesMonitor->amountOfTextureObjects();

                // Get loaded level 0 texels amount:
                amountOfLoadedLevel0Texels = texturesMonitor->amountOfLoadedLevel0Texels();
            }

            // Get the render context render primitives monitor:
            gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = _pMonitoredRenderContext->renderPrimitivesStatisticsLogger();

            // Update the rendered vertices per frame:
            pCountersValuesArray[GS_RENDERED_VERTICES_COUNTER] = renderPrimitivesStatisticsLogger.amountOfRenderedPrimitivesPerFrame(GS_VERTICES);

            // Update the rendered vertices per frame:
            pCountersValuesArray[GS_RENDERED_POINTS_COUNTER] = renderPrimitivesStatisticsLogger.amountOfRenderedPrimitivesPerFrame(GS_POINTS);

            // Update the rendered vertices per frame:
            pCountersValuesArray[GS_RENDERED_LINES_COUNTER] = renderPrimitivesStatisticsLogger.amountOfRenderedPrimitivesPerFrame(GS_LINES);

            // Update the rendered vertices per frame:
            pCountersValuesArray[GS_RENDERED_TRIANGLES_COUNTER] = renderPrimitivesStatisticsLogger.amountOfRenderedPrimitivesPerFrame(GS_TRIANGLES);

            // Update the rendered vertices per frame:
            pCountersValuesArray[GS_RENDERED_PRIMITIVES_COUNTER] = renderPrimitivesStatisticsLogger.amountOfRenderedPrimitivesPerFrame(GS_PRIMITIVES);

            // Restart the periodic counters:
            renderPrimitivesStatisticsLogger.restartPeriodicCounters();
        }

        // Update the texture objects amount:
        pCountersValuesArray[GS_TEXTURE_OBJECTS_AMOUNT_COUNTER] = amountOfTextureObjects;

        // Update the loaded level 0 texels amount:
        pCountersValuesArray[GS_LEVEL_0_LOADED_TEXELS_AMOUNT_COUNTER] = amountOfLoadedLevel0Texels;

        // Restart the periodic counters:
        restartPeriodicCounters();

        // Unlock the access to the threads common variables:
        csLocker.leaveCriticalSection();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextPerformanceCountersManager::getAvarageFramePerSecondRatio
// Description: Returns the average frame / sec ration measured since the last call
//              to restartPeriodicCounters().
// Author:      Yaki Tebeka
// Date:        26/7/2005
// ---------------------------------------------------------------------------
double gsRenderContextPerformanceCountersManager::getAvarageFramePerSecondRatio()
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
// Name:        gsRenderContextPerformanceCountersManager::getAvarageFrameTime
// Description: Returns the average time it took to render a frame since the last
//              time this was sampled.
// Author:      Uri Shomroni
// Date:        30/3/2009
// ---------------------------------------------------------------------------
double gsRenderContextPerformanceCountersManager::getAvarageFrameTime()
{
    double retVal = _lastRealFrameTimeValue;

    // If at least one frame ended:
    if (_endedFramesTimeIntervalsSum > 0)
    {
        // Calculate the average mS/F:
        retVal = (_endedFramesTimeIntervalsSum * 1000.0) / (double)_frameCounter;
        _lastRealFrameTimeValue = retVal;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextPerformanceCountersManager::getAvarageFuncCallsPerFrameRatio
// Description: Returns the average function calls / Frames counts since the last
//              call to restartPeriodicCounters().
// Author:      Yaki Tebeka
// Date:        27/7/2005
// ---------------------------------------------------------------------------
double gsRenderContextPerformanceCountersManager::getAvarageFuncCallsPerFrameRatio()
{
    double retVal = _lastRealOGLCallsValue;

    // If at least one frame ended:
    if (_frameCounter > 0)
    {
        // Calculate the average function calls per frame:
        retVal = _endedFramesFunctionCalls / _frameCounter;
        _lastRealOGLCallsValue = retVal;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextPerformanceCountersManager::restartPeriodicCounters
// Description: Restart periodic counters.
// Author:      Yaki Tebeka
// Date:        27/7/2005
// ---------------------------------------------------------------------------
void gsRenderContextPerformanceCountersManager::restartPeriodicCounters()
{
    // Restart the ended frames function calls count:
    _endedFramesFunctionCalls = 0;

    // Restart the ended frames time intervals sum:
    _endedFramesTimeIntervalsSum = 0;

    // Restart the frames count:
    _frameCounter = 0;
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextPerformanceCountersManager::onFrameTerminatorCall
// Description: Is called when a frame terminator function call is executed
//              within the monitored context
// Author:      Yaki Tebeka
// Date:        26/7/2005
// ---------------------------------------------------------------------------
void gsRenderContextPerformanceCountersManager::onFrameTerminatorCall()
{
    // Lock the access to the threads common variables:
    osCriticalSectionLocker csLocker(_threadSyncCriticalSection);

    // Increment the frames count:
    _frameCounter++;

    // Calculate the time spent since the current frame started:
    double currentFrameTimeInterval = 0;
    _stopWatch.getTimeInterval(currentFrameTimeInterval);

    // Add it to the ended frames time intervals sum:
    _endedFramesTimeIntervalsSum += currentFrameTimeInterval;

    // Restart the current frame time stop watch:
    _stopWatch.start();

    // Update the amount of ended frames function calls:
    _endedFramesFunctionCalls += _currentFrameFunctionsCalls;
    _currentFrameFunctionsCalls = 0;

    // Unlock the access to the threads common variables:
    csLocker.leaveCriticalSection();
}


