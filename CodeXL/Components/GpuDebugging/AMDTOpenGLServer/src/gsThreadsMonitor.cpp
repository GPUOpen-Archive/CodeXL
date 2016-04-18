//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsThreadsMonitor.cpp
///
//==================================================================================

//------------------------------ gsThreadsMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osThreadLocalData.h>
#include <AMDTAPIClasses/Include/apDefinitions.h>

// Local:
#include <src/gsOpenGLMonitor.h>
#include <src/gsRenderContextMonitor.h>
#include <src/gsStringConstants.h>
#include <src/gsThreadsMonitor.h>

// iPhone on-device only:
#ifdef _GR_IPHONE_DEVICE_BUILD
    #include <AMDTAPIClasses/Include/Events/apThreadCreatedEvent.h>
    #include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>
#endif

// A handle to a thread local storage data that holds the current thread's context id:
static osTheadLocalDataHandle stat_hCurrentContextTLD;

// ---------------------------------------------------------------------------
// Name:        gsThreadsMonitor::gsThreadsMonitor
// Description:
// Return Val:
// Author:      Uri Shomroni
// Date:        2/11/2009
// ---------------------------------------------------------------------------
gsThreadsMonitor::gsThreadsMonitor()
{
    // Allocate thread local data to hold the current thread's current context id:
    bool rc = osAllocateThreadsLocalData(stat_hCurrentContextTLD);
    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        gsThreadsMonitor::~gsThreadsMonitor
// Description:
// Return Val:
// Author:      Uri Shomroni
// Date:        2/11/2009
// ---------------------------------------------------------------------------
gsThreadsMonitor::~gsThreadsMonitor()
{
    // Release TLS (Thread local storage) data:
    bool rc = osFreeThreadsLocalData(stat_hCurrentContextTLD);
    GT_ASSERT(rc);

    // Lock the _threadCurrentRenderContextCS map access:
    osCriticalSectionLocker criticalSectionLocker(((gsThreadsMonitor*)(this))->_threadIdToThreadLocalDataCS);

    // Look for the render context id in the _threadCurrentRenderContext map:
    gtMap<osThreadId, gsThreadLocalData*>::const_iterator iter = _threadIdToThreadLocalData.begin();
    gtMap<osThreadId, gsThreadLocalData*>::const_iterator endIter = _threadIdToThreadLocalData.end();

    // Iterate the map items:
    while (iter != endIter)
    {
        // Get the current thread local data:
        gsThreadLocalData* pCurrThreadLocalData = ((*iter).second);

        if (pCurrThreadLocalData != NULL)
        {
            delete pCurrThreadLocalData;
        }

        iter++;
    }

    // Unlock the critical section:
    criticalSectionLocker.leaveCriticalSection();
}


// ---------------------------------------------------------------------------
// Name:        gsThreadsMonitor::threadCurrentRenderContext
// Description: Inputs a thread id and returns its current render context id.
// Arguments - threadId - The input thread id.
// Return Val:  int - The output render context id, or 0 if the input thread
//                    does not have a current render context.
// Author:      Yaki Tebeka
// Date:        5/5/2005
// ---------------------------------------------------------------------------
int gsThreadsMonitor::threadCurrentRenderContext(const osThreadId& threadId) const
{
    int retVal = 0;

    // Lock the _threadCurrentRenderContextCS map access:
    osCriticalSectionLocker criticalSectionLocker(((gsThreadsMonitor*)(this))->_threadIdToThreadLocalDataCS);

    // Look for the thread id in the _threadCurrentRenderContext map:
    gtMap<osThreadId, gsThreadLocalData*>::const_iterator iter = _threadIdToThreadLocalData.find(threadId);

    // If we found the thread id:
    if (iter != _threadIdToThreadLocalData.end())
    {
        // Get the thread local data:
        gsThreadLocalData* pThreadLocalData = ((*iter).second);
        GT_IF_WITH_ASSERT(pThreadLocalData != NULL)
        {
            // Return the render context id:
            retVal = pThreadLocalData->_spyContextId;
        }
    }

    // Unlock the critical section:
    criticalSectionLocker.leaveCriticalSection();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsThreadsMonitor::renderContextCurrentThread
// Description: Inputs a render context and outputs the thread that uses this
//              render context as "current render context".
// Arguments: renderContextId - The input render context spy id.
// Return Val: osThreadId - The output thread id, or OS_NO_THREAD_ID if no thread
//                          uses the input render context as "current render context".
// Author:      Yaki Tebeka
// Date:        31/1/2008
// ---------------------------------------------------------------------------
osThreadId gsThreadsMonitor::renderContextCurrentThread(int renderContextId) const
{
    osThreadId retVal = OS_NO_THREAD_ID;

    // Lock the _threadCurrentRenderContextCS map access:
    osCriticalSectionLocker criticalSectionLocker(((gsThreadsMonitor*)(this))->_threadIdToThreadLocalDataCS);

    // Look for the render context id in the _threadCurrentRenderContext map:
    gtMap<osThreadId, gsThreadLocalData*>::const_iterator iter = _threadIdToThreadLocalData.begin();
    gtMap<osThreadId, gsThreadLocalData*>::const_iterator endIter = _threadIdToThreadLocalData.end();

    // Iterate the map items:
    while (iter != endIter)
    {
        // Get the current thread local data:
        gsThreadLocalData* pCurrThreadLocalData = ((*iter).second);
        GT_IF_WITH_ASSERT(pCurrThreadLocalData != NULL)
        {
            // If the thread's current context is our input context:
            int currThreadCurrContextId = pCurrThreadLocalData->_spyContextId;

            if (currThreadCurrContextId == renderContextId)
            {
                // Output the thread id:
                retVal = (*iter).first;

                // Exit the loop:
                break;
            }
        }

        iter++;
    }

    // Unlock the critical section:
    criticalSectionLocker.leaveCriticalSection();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsThreadsMonitor::currentThreadRenderContextSpyId
// Description: Returns the current render context of the calling thread.
// Return Val:  int - Will get the spy id of the render context that is the
//                    "current render context" of the thread that called this function.
// Author:      Yaki Tebeka
// Date:        6/11/2005
// ---------------------------------------------------------------------------
int gsThreadsMonitor::currentThreadRenderContextSpyId() const
{
    int retVal = AP_NULL_CONTEXT_ID;

    // Get the thread local data that holds the current render context:
    gsThreadLocalData* pThreadLocalData = (gsThreadLocalData*)osGetCurrentThreadLocalData(stat_hCurrentContextTLD);

    // If the thread local data was not created yet - create it now:
    if (pThreadLocalData == NULL)
    {
        ((gsThreadsMonitor*)this)->setCurrentThreadCurrentContext(NULL, 0, 0, AP_NULL_CONTEXT_ID);
    }
    else
    {
        // Get the current render context id:
        retVal = pThreadLocalData->_spyContextId;

        // If this is not the NULL context:
        if (retVal != AP_NULL_CONTEXT_ID)
        {
            // Get the queried render context monitor:
            gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(retVal);

            if (pRenderContextMonitor)
            {
                // Check if the context was deleted from the OS:
                bool wasRenderContextDeleted = pRenderContextMonitor->wasDeleted();

                if (wasRenderContextDeleted)
                {
                    GT_ASSERT(false);

                    // Make the NULL context the current context of the calling thread:
                    ((gsThreadsMonitor*)this)->setCurrentThreadCurrentContext(NULL, 0, 0, AP_NULL_CONTEXT_ID);
                    retVal = AP_NULL_CONTEXT_ID;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsThreadsMonitor::removeRenderContextFromAllThreads
// Description: Removed a render context from being the current render context
//              of any thread.
// Arguments: pRenderContext - The render context handle, casted into void*.
// Author:      Yaki Tebeka
// Date:        14/11/2005
// ---------------------------------------------------------------------------
void gsThreadsMonitor::removeRenderContextFromAllThreads(oaOpenGLRenderContextHandle pRenderContext)
{
    // Get the input render context spy id:
    int inputContextSpyId = gsOpenGLMonitor::instance().renderContextSpyId(pRenderContext);

    // If we didn't find the input context:
    if (inputContextSpyId <= 0)
    {
        GT_ASSERT_EX(0, GS_STR_UnkownContextHandleUsed);
    }
    else
    {
        // Lock the _threadCurrentRenderContext map access:
        osCriticalSectionLocker criticalSectionLocker(_threadIdToThreadLocalDataCS);

        // Iterate the recorded threads:
        gtMap<osThreadId, gsThreadLocalData*>::iterator iter = _threadIdToThreadLocalData.begin();
        gtMap<osThreadId, gsThreadLocalData*>::iterator endIter = _threadIdToThreadLocalData.end();

        while (iter != endIter)
        {
            // Get the current thread's local data:
            gsThreadLocalData* pThreadLocalData = (*iter).second;
            GT_IF_WITH_ASSERT(pThreadLocalData != NULL)
            {
                // Get the current thread's current context id:
                int curThreadCurrentContextId = pThreadLocalData->_spyContextId;

                // If the current thread's "current" render context is the input context:
                if (curThreadCurrentContextId == inputContextSpyId)
                {
                    // Remove it from being the thread's "current" render context:
                    pThreadLocalData->_spyContextId = 0;
                }
            }

            iter++;
        }

        // Release the map access:
        criticalSectionLocker.leaveCriticalSection();
    }
}
// ---------------------------------------------------------------------------
// Name:        gsThreadsMonitor::setCurrentThreadCurrentContextId
// Description: Set the current thread current context.
// Arguments:  hDC - The device (display) on which the drawables exist.
//             drawSurface - The surface (drawable) on which OpenGL draws.
//             readSurface - The surface (drawable) from which OpenGL reads.
//             spyContextId - The spy id of the context that is now the
//                             current context of the thread that called this
//                             function.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        23/11/2006
// ---------------------------------------------------------------------------
bool gsThreadsMonitor::setCurrentThreadCurrentContext(oaDeviceContextHandle hDC, oaDrawableHandle drawSurface,
                                                      oaDrawableHandle readSurface, int spyContextId)
{
    bool retVal = false;

    // Lock the _threadCurrentRenderContext map access:
    osCriticalSectionLocker criticalSectionLocker(_threadIdToThreadLocalDataCS);

    // Get the thread local data that holds the current render context:
    gsThreadLocalData* pThreadLocalData = (gsThreadLocalData*)osGetCurrentThreadLocalData(stat_hCurrentContextTLD);

    // If the thread local data was not created yet:
    if (pThreadLocalData == NULL)
    {
        // Create the thread local data:
        pThreadLocalData = new gsThreadLocalData;
        GT_IF_WITH_ASSERT(pThreadLocalData != NULL)
        {
            // Set it as the current thread's local data:
            osSetCurrentThreadLocalData(stat_hCurrentContextTLD, pThreadLocalData);

            // Log it also in the threads local data map:
            osThreadId currentThreadId = osGetCurrentThreadId();
            _threadIdToThreadLocalData[currentThreadId] = pThreadLocalData;

#ifdef _GR_IPHONE_DEVICE_BUILD
            // On the iPhone device, we don't know when threads are created unless they pass
            // through here. For that reason, we send a thread created event:
            osTime now;
            now.setFromCurrentTime();

            // TO_DO: Uri, 2/11/09 - find a way to get the LWP value
            apThreadCreatedEvent threadCreatedEvent(currentThreadId, OS_NO_THREAD_ID, now, NULL);
            osSocket* pEventsSocket = suEventForwardingSocket();

            // Verify that we have an events socket before using it:
            int iterationsCount = 0;

            while ((pEventsSocket == NULL) && (iterationsCount < 20))
            {
                osSleep(500);
                pEventsSocket = suEventForwardingSocket();
                iterationsCount++;
            }

            GT_IF_WITH_ASSERT(pEventsSocket != NULL)
            {
                bool rcEve = suForwardEventToClient(threadCreatedEvent);
                GT_ASSERT(rcEve);
            }
#endif
        }
    }

    // Sanity check:
    if (pThreadLocalData != NULL)
    {
        // Log the current thread's drawables:
        pThreadLocalData->_hDC = hDC;
        pThreadLocalData->_drawSurface = drawSurface;
        pThreadLocalData->_readSurface = readSurface;

        // Set the current thread current context id:
        pThreadLocalData->_spyContextId = spyContextId;
        retVal = true;

        // Log it also in the threads local data map:
        osThreadId currentThreadId = osGetCurrentThreadId();
        _threadIdToThreadLocalData[currentThreadId] = pThreadLocalData;
    }

    // Unlock the critical section:
    criticalSectionLocker.leaveCriticalSection();

    return retVal;
}

