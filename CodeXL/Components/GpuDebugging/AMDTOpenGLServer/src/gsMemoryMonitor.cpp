//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsMemoryMonitor.cpp
///
//==================================================================================

//------------------------------ gsMemoryMonitor.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessIsDuringTerminationEvent.h>
#include <AMDTAPIClasses/Include/Events/apContextDataSnapshotWasUpdatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apSearchingForMemoryLeaksEvent.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>

// Local:
#include <src/gsAPIFunctionsImplementations.h>
#include <src/gsGlobalVariables.h>
#include <src/gsMemoryMonitor.h>
#include <src/gsOpenGLMonitor.h>

// Static members initializations:
gsMemoryMonitor* gsMemoryMonitor::_pMySingleInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        gsMemoryMonitor::instance
// Description: Returns the single instance of the gsOpenGLMonitor class
// Author:      Sigal Algranaty
// Date:        14/7/2010
// ---------------------------------------------------------------------------
gsMemoryMonitor& gsMemoryMonitor::instance()
{
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new gsMemoryMonitor;
        GT_ASSERT(_pMySingleInstance);
    }

    return *_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        gsMemoryMonitor::gsMemoryMonitor
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        14/7/2010
// ---------------------------------------------------------------------------
gsMemoryMonitor::gsMemoryMonitor()
{
}


// ---------------------------------------------------------------------------
// Name:        gsMemoryMonitor::~gsMemoryMonitor
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        25/5/2008
// ---------------------------------------------------------------------------
gsMemoryMonitor::~gsMemoryMonitor()
{
}



// ---------------------------------------------------------------------------
// Name:        gsMemoryMonitor::beforeContextDeletion
// Description: Perform a memory leaks check before a context is deleted
// Arguments:   int contextID
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        14/7/2010
// ---------------------------------------------------------------------------
bool gsMemoryMonitor::beforeContextDeletion(apContextID deleteContextID)
{
    bool retVal = false;

    // Send a "Searching for memory leaks" event:
    gtString searchingMessage;
    searchingMessage.appendFormattedString(SU_STR_MemoryLeakSearchingForMemoryLeaksOnGLContextDeletion, deleteContextID._contextId);
    apSearchingForMemoryLeaksEvent searchingEvent(searchingMessage);

    // Write the searching for memory leak event:
    bool rcEve = suForwardEventToClient(searchingEvent);
    GT_ASSERT(rcEve);

    // Performs actions required for context memory leaks detection:
    bool rc = prepareContextDataForMemoryDetection(deleteContextID);
    GT_ASSERT_EX(rc, L"prepareContextDataForMemoryDetection failed.");

    // Check for memory leaks for the OpenGL context:
    checkForMemoryLeaksOnOpenGLContextResourcesDeleted(deleteContextID);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsMemoryMonitor::beforeSpyTermination
// Description: Handle Spy termination
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        14/7/2010
// ---------------------------------------------------------------------------
void gsMemoryMonitor::beforeSpyTermination()
{
    // Check for global memory leaks:
    checkForMemoryLeaksOnOpenGLUnloaded();
}

// ---------------------------------------------------------------------------
// Name:        gsMemoryMonitor::checkForMemoryLeaksOnOpenGLUnloaded
// Description: Checks for context-independent memory leaks (Render contexts,
//              PBuffers, Sync objects) when the OpenGL spy is unloaded
// Author:      Uri Shomroni
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void gsMemoryMonitor::checkForMemoryLeaksOnOpenGLUnloaded()
{
    // Send a "Searching for memory leaks" event:
    apSearchingForMemoryLeaksEvent searchingEvent(SU_STR_MemoryLeakSearchingForGLMemoryLeaksOnApplicationExit);

    // Write the searching for memory leak event:
    bool rcEve = suForwardEventToClient(searchingEvent);
    GT_ASSERT(rcEve);

    // Define a memory leak event:
    apMemoryLeakEvent memoryLeakEvent;

    // Set the return event type:
    memoryLeakEvent.setLeakType(apMemoryLeakEvent::AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK);

    // Get the render contexts amount:
    int amountOfRCs = gs_stat_openGLMonitorInstance.amountOfContexts();
    unsigned int numberOfLeakedRCs = 0;

    // Iterate the render context and search for not deleted contexts:
    for (int contextID = 1; contextID < amountOfRCs; contextID++)
    {
        // Get the queried render context monitor:
        const gsRenderContextMonitor* pRenderContextMonitor = gs_stat_openGLMonitorInstance.renderContextMonitor(contextID);
        GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
        {
            // Check if the render context was deleted from the OS:
            bool wasContextDeleted = pRenderContextMonitor->wasDeleted();

            if (!wasContextDeleted)
            {
                // Count the still existing contexts:
                numberOfLeakedRCs++;

                // Note this in the event:
                memoryLeakEvent.addLeakingRenderContext(contextID);

                // Calculate the memory leak for this context:
                gtUInt64 contextMemoryLeak = 0;
                bool rcCalculateMemoryLeakSize = calculateMemoryLeakForContext(pRenderContextMonitor, contextMemoryLeak);
                GT_IF_WITH_ASSERT(rcCalculateMemoryLeakSize)
                {
                    // Add the context memory leak to the event:
                    memoryLeakEvent.addMemoryLeakSize(contextMemoryLeak);
                }
            }
        }
    }

    // Get the PBuffers monitor:
    const gsPBuffersMonitor& pbuffersMonitor = gs_stat_openGLMonitorInstance.pbuffersMonitor();

    // Get the PBuffers amount:
    int amountOfPBOs = pbuffersMonitor.amountOfPBuffersObjects();
    unsigned int numberOfLeakedPBOs = 0;
    gtString leakedPBOList;

    // Iterate the PBuffers and looks for memory leak:
    for (int bufferId = 0; bufferId < amountOfPBOs; bufferId++)
    {
        // Get the buffer details:
        apPBuffer* pCurrentPBuffer = pbuffersMonitor.getPBufferObjectDetails(bufferId);
        GT_IF_WITH_ASSERT(pCurrentPBuffer != NULL)
        {
            if (!pCurrentPBuffer->isDeleted())
            {
                // Count the still existing PBOs:
                numberOfLeakedPBOs++;
                leakedPBOList.appendFormattedString(L", %d", bufferId);

                // Note this in the event:
                memoryLeakEvent.addLeakingPBuffer(bufferId);
            }
        }
    }

    // Get the sync objects monitor:
    const gsSyncObjectsMonitor& syncMonitor = gs_stat_openGLMonitorInstance.syncObjectsMonitor();

    // Get the Sync objects amount:
    int amountOfSyncObjects = syncMonitor.amountOfSyncObjects();
    unsigned int numberOfLeakedSyncObjects = 0;
    gtString leakedSyncObjectsList;

    for (int syncObjectId = 0; syncObjectId < amountOfSyncObjects; syncObjectId++)
    {
        const apGLSync* pCurrentSyncObject = syncMonitor.getSyncObjectDetails(syncObjectId);
        GT_IF_WITH_ASSERT(pCurrentSyncObject != NULL)
        {
            // Add the leaked sync id:
            apMemoryLeakEvent::apLeakedSyncObjectID syncId;
            syncId._syncID = pCurrentSyncObject->syncID();
            syncId._syncHandle = pCurrentSyncObject->syncHandle();

            // Note that we don't add to the leak size, as Syncs are insignificant.

            // Count the still existing sync objects:
            numberOfLeakedSyncObjects++;

            // Note this in the event:
            memoryLeakEvent.addLeakingSyncObject(syncId);
        }
    }

    if (memoryLeakEvent.memoryLeakExists())
    {
        rcEve = suForwardEventToClient(memoryLeakEvent);
        GT_ASSERT(rcEve);

        // Check if we should break on memory leaks:
        bool shouldBreakOnMemoryLeaks = su_stat_theBreakpointsManager.breakOnGenericBreakpoint(AP_BREAK_ON_MEMORY_LEAK);

        // If a memory leak was detected by a spy update function, we do not want to trigger a breakpoint, to avoid hangs in the client:
        bool isAPIThread = suIsSpiesAPIThreadId(osGetCurrentThreadId());

        if (shouldBreakOnMemoryLeaks && !isAPIThread)
        {
            // Report that process is during termination:
            apDebuggedProcessIsDuringTerminationEvent duringTerminationEvent;
            bool rcEve2 = suForwardEventToClient(duringTerminationEvent);
            GT_ASSERT(rcEve2);

            // Set the current break reason:
            su_stat_theBreakpointsManager.beforeTriggeringBreakpoint();
            su_stat_theBreakpointsManager.setBreakReason(AP_MEMORY_LEAK_BREAKPOINT_HIT);

            // Get the current context id:
            int spyID = gs_stat_openGLMonitorInstance.currentThreadRenderContextSpyId();
            apContextID triggeringContextID(AP_OPENGL_CONTEXT, spyID);

            // Uri, 20/12/09: We check if the API thread exists, since this function could be called either when the
            // OpenGL spy is unloaded or when the app terminates. If the latter, the API thread should already be dead...
            bool apiThreadExists = suIsAPIThreadRunning();

            if (!apiThreadExists)
            {
                // Let the debugger know we are about to enter the termination loop:
                suBeforeEnteringTerminationAPILoop();
            }

            // Trigger the breakpoint exception:
            su_stat_theBreakpointsManager.triggerBreakpointException(triggeringContextID, apMonitoredFunctionsAmount);
            su_stat_theBreakpointsManager.afterTriggeringBreakpoint();

            if (!apiThreadExists)
            {
                suRunTerminationAPILoop();
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsMemoryMonitor::checkForMemoryLeaksOnOpenGLContextResourcesDeleted
// Description: Check for memory leaks when an OpenGL render context holding
//              resources is deleted (i.e. the last living context of a sharegroup).
//              Here we check for OpenGL context-dependant items (textures, buffers...)
// Author:      Uri Shomroni
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void gsMemoryMonitor::checkForMemoryLeaksOnOpenGLContextResourcesDeleted(const apContextID& deletedContextID)
{
    // Sanity check:
    GT_ASSERT(deletedContextID.isOpenGLContext() && !deletedContextID.isDefault());
    int deletedContextIndex = deletedContextID._contextId;

    // Define a memory leak event:
    apMemoryLeakEvent memoryLeakEvent;

    // Initialize the leak event:
    memoryLeakEvent.setLeakType(apMemoryLeakEvent::AP_GL_CONTEXT_ALLOCATED_OBJECT_LEAK);
    bool rcRC = memoryLeakEvent.setLeakingObjectsRenderContextID(deletedContextIndex);
    GT_ASSERT(rcRC);

    // Get the render context monitor:
    const gsRenderContextMonitor* pRenderContextMonitor = gs_stat_openGLMonitorInstance.renderContextMonitor(deletedContextID._contextId);
    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
    {
        // Textures:
        int leakingTextures = 0;
        const gsTexturesMonitor* pTexturesMonitor = pRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(pTexturesMonitor != NULL)
        {
            // Set the amount of leaking textures:
            leakingTextures = pTexturesMonitor->amountOfTextureObjects();
        }

        // Samplers:
        const gsSamplersMonitor& samplersMonitor = pRenderContextMonitor->samplersMonitor();
        int leakingSamplers = (int)samplersMonitor.getAmountOfSamplerObjects();

        // Render buffer objects:
        int leakingRBOs = 0;
        const gsRenderBuffersMonitor* pRenderBuffersMonitor = pRenderContextMonitor->renderBuffersMonitor();
        GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
        {
            // Set the amount of leaking render buffers:
            leakingRBOs = pRenderBuffersMonitor->amountOfRenderBufferObjects();
        }

        // Frame buffer objects:
        int leakingFBOs = 0;
        const gsFBOMonitor* pFBOsMonitor = pRenderContextMonitor->fboMonitor();
        GT_IF_WITH_ASSERT(pFBOsMonitor != NULL)
        {
            // Set the amount of leaking FBOs:
            leakingFBOs = pFBOsMonitor->amountOfFBOs();
        }

        // Vertex buffer objects:
        int leakingVBOs = 0;
        const gsVBOMonitor* pVBOMonitor = pRenderContextMonitor->vboMonitor();
        GT_IF_WITH_ASSERT(pVBOMonitor != NULL)
        {
            // Set the amount of leaking VBOs:
            leakingVBOs = pVBOMonitor->amountOfVBOs();
        }

        // Program and Shader objects:
        int leakingPrograms = 0;
        int leakingShaders = 0;
        const gsProgramsAndShadersMonitor* pProgramsAndShadersMonitor = pRenderContextMonitor->programsAndShadersMonitor();
        GT_IF_WITH_ASSERT(pProgramsAndShadersMonitor != NULL)
        {
            // Get the amount of programs:
            leakingPrograms = pProgramsAndShadersMonitor->amountOfLivingProgramObjects();

            // Get the amount of shaders:
            leakingShaders = pProgramsAndShadersMonitor->amountOfLivingShaderObjects();
        }

        // Program pipelines:
        const gsPipelineMonitor& pipelineMonitor = pRenderContextMonitor->pipelinesMonitor();
        int leakingPipelines = (int)pipelineMonitor.GetAmountOfPipelineObjects();

        // Display Lists:
        int leakingDisplayLists = 0;
        const gsDisplayListMonitor* pDisplayListMonitor = pRenderContextMonitor->displayListsMonitor();
        GT_IF_WITH_ASSERT(pDisplayListMonitor != NULL)
        {
            // Set the amount of leaking diaplay lists:
            leakingDisplayLists = pDisplayListMonitor->amountOfDisplayLists();
        }

        // Set the values into the event:
        bool rcObj = memoryLeakEvent.setLeakingGLAllocatedObjects(leakingTextures, leakingSamplers, leakingRBOs, leakingFBOs, leakingVBOs, leakingPrograms, leakingShaders, leakingPipelines, leakingDisplayLists);
        GT_ASSERT(rcObj);

        // Calculate the memory leak for this context:
        gtUInt64 contextMemoryLeak = 0;
        bool rcCalculateMemoryLeakSize = calculateMemoryLeakForContext(pRenderContextMonitor, contextMemoryLeak);
        GT_IF_WITH_ASSERT(rcCalculateMemoryLeakSize)
        {
            // Add the context memory leak to the event:
            memoryLeakEvent.addMemoryLeakSize(contextMemoryLeak);
        }

        // Check if the context contain memory leaks:
        if (memoryLeakEvent.memoryLeakExists())
        {
            // Write the memory leak event:
            bool rcEve = suForwardEventToClient(memoryLeakEvent);
            GT_ASSERT(rcEve);

            // Check if we should break on memory leaks:
            bool shouldBreakOnMemoryLeaks = su_stat_theBreakpointsManager.breakOnGenericBreakpoint(AP_BREAK_ON_MEMORY_LEAK);

            // If a memory leak was detected by a spy update function, we do not want to trigger a breakpoint, to avoid hangs in the client:
            bool isAPIThread = suIsSpiesAPIThreadId(osGetCurrentThreadId());

            if (shouldBreakOnMemoryLeaks && !isAPIThread)
            {
                su_stat_theBreakpointsManager.beforeTriggeringBreakpoint();
                su_stat_theBreakpointsManager.setBreakReason(AP_MEMORY_LEAK_BREAKPOINT_HIT);

                // Get the delete function Id:
                apMonitoredFunctionId monitoredFunctionId = apMonitoredFunctionsAmount;
                const suCallsHistoryLogger* pCallsHistoryLogger = pRenderContextMonitor->callsHistoryLogger();
                GT_IF_WITH_ASSERT(pCallsHistoryLogger != NULL)
                {
                    monitoredFunctionId = pCallsHistoryLogger->lastCalledFunctionId();
                }

                // Trigger the breakpoint exception:
                su_stat_theBreakpointsManager.triggerBreakpointException(deletedContextID, monitoredFunctionId);
                su_stat_theBreakpointsManager.afterTriggeringBreakpoint();
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsMemoryMonitor::prepareContextDataForMemoryDetection
// Description: Perform actions required for memory detection.
//              1. Update context data snapshot
//              2. Update texture parameters
// Arguments:   apContextID deletedContextID
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/7/2010
// ---------------------------------------------------------------------------
bool gsMemoryMonitor::prepareContextDataForMemoryDetection(apContextID deletedContextID)
{
    bool retVal = false;

    // This function is available on Windows and Mac only, since glXMakeCurrent requires a drawable, which we don't have:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
    {
        // Get the input render context monitor:
        gsRenderContextMonitor* pRenderContextMonitor = gs_stat_openGLMonitorInstance.renderContextMonitor(deletedContextID._contextId);
        GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
        {
            // Make the input render context the API thread's current render context:
            int currentContextIndex = gs_stat_openGLMonitorInstance.currentThreadRenderContextSpyId();
            bool rcMakeCurrent = gsMakeRenderContextCurrent(deletedContextID._contextId);
            GT_IF_WITH_ASSERT(rcMakeCurrent)
            {
                // Update the context's data snapshot without the state variables:
                pRenderContextMonitor->setStateVariableUpdateMode(false);
                bool rcUpdateSnapshot = pRenderContextMonitor->updateContextDataSnapshot();
                pRenderContextMonitor->setStateVariableUpdateMode(true);

                // Report the context snapshot:
                apContextDataSnapshotWasUpdatedEvent contextUpdatedEvent(deletedContextID, true);
                bool rcEve = suForwardEventToClient(contextUpdatedEvent);
                GT_ASSERT(rcEve);

                // Update the existing texture parameters:
                bool rcUpdateTexParams = updateTextureParameters(pRenderContextMonitor);
                GT_ASSERT(rcUpdateTexParams);

                // Get its static buffers monitor, and update the static buffer:
                gsStaticBuffersMonitor& buffersMtr = pRenderContextMonitor->buffersMonitor();
                bool rcUpdateBuffersDimensions = buffersMtr.updateStaticBuffersDimensions();

                retVal = rcUpdateSnapshot && rcUpdateTexParams && rcUpdateBuffersDimensions;
            }

            // Restore the previously current context:
            bool rcRestoreCurrent = gsMakeRenderContextCurrent(currentContextIndex);
            GT_ASSERT(rcRestoreCurrent);
        }
    }
#else
    (void)(deletedContextID); // Resolve the compiler warning for the Linux variant
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsMemoryMonitor::updateTextureParameters
// Description: Update the existing textures parameter for the render context
// Arguments:   pRenderContextMonitor - the context monitor for the textures context
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/7/2010
// ---------------------------------------------------------------------------
bool gsMemoryMonitor::updateTextureParameters(gsRenderContextMonitor* pRenderContextMonitor)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
    {
        // Get the OpenGL version and profile, for deprecation checking:
        bool isOpenGL31CoreContext = pRenderContextMonitor->isOpenGLVersionOrNewerCoreContext();

        // Get the texture monitor:
        gsTexturesMonitor* pTexturesMonitor = pRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(pTexturesMonitor != NULL)
        {
            // Get the textures amount:
            int amountOfTexObjects = pTexturesMonitor->amountOfTextureObjects();

            retVal = true;

            // Iterate through the textures and update their raw data
            for (int i = 0; i < amountOfTexObjects; i++)
            {
                // Get texture object from the monitor:
                gsGLTexture* pTextureObj = pTexturesMonitor->getTextureObjectDetailsByIndex(i);
                GT_IF_WITH_ASSERT(pTextureObj != NULL)
                {
                    // Get texture type:
                    apTextureType textureType = pTextureObj->textureType();

                    // Get bind target:
                    GLenum bindTarget = apTextureTypeToTextureBindTarget(textureType);

                    // If the texture is binded:
                    if (bindTarget != GL_NONE)
                    {
                        // Get the current texture name:
                        GLuint textureName = 0;
                        bool rcUpdateParams = false;
                        bool rc = pTexturesMonitor->getTextureObjectName(i, textureName);
                        GT_IF_WITH_ASSERT(rc)
                        {
                            // Bind the texture for update:
                            pTexturesMonitor->bindTextureForUpdate(textureName, bindTarget);

                            // Update the texture parameters:
                            rcUpdateParams = pTextureObj->updateTextureParameters(true, isOpenGL31CoreContext);

                            if (!rcUpdateParams)
                            {
                                gtString errorMessage;
                                errorMessage.appendFormattedString(L"Update for texture %d parameter had failed", i);
                                GT_ASSERT_EX(rcUpdateParams, errorMessage.asCharArray());
                            }

                            // Restore the previously binded texture:
                            pTexturesMonitor->restoreBindedTextureAfterUpdate();
                        }

                        retVal = retVal && rcUpdateParams;
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsMemoryMonitor::calculateMemoryLeakForContext
// Description: Calculate a memory leak for a context
// Arguments:   pRenderContextMonitor - the context monitor
//              gtUInt64& contextMemorySize - output - the context memory size
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/7/2010
// ---------------------------------------------------------------------------
bool gsMemoryMonitor::calculateMemoryLeakForContext(const gsRenderContextMonitor* pRenderContextMonitor, gtUInt64& contextMemorySize)
{
    bool retVal = true;

    contextMemorySize = 0;
    bool rcCalcTextureMemory = false;
    bool rcCalcRBOMemory = false;
    bool rcCalcVBOMemory = false;
    bool rcCalcShadersMemory = false;
    bool rcCalcDisplayListMemory = false;

    // Textures:
    const gsTexturesMonitor* pTexturesMonitor = pRenderContextMonitor->texturesMonitor();
    GT_IF_WITH_ASSERT(pTexturesMonitor != NULL)
    {
        // Calculate the textures memory size:
        gtUInt64 texturesLeakMemorySize = 0;
        rcCalcTextureMemory = pTexturesMonitor->calculateTexturesMemorySize(texturesLeakMemorySize);

        // Add the memory size to the event:
        contextMemorySize += texturesLeakMemorySize;
    }

    // Render buffer objects:
    const gsRenderBuffersMonitor* pRenderBuffersMonitor = pRenderContextMonitor->renderBuffersMonitor();
    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
    {
        // Calculate the buffers memory size:
        gtUInt64 buffersLeakMemorySize = 0;
        rcCalcRBOMemory = pRenderBuffersMonitor->calculateBuffersMemorySize(buffersLeakMemorySize);

        // Add the memory size to the event:
        contextMemorySize += buffersLeakMemorySize;
    }

    // Vertex buffer objects:
    const gsVBOMonitor* pVBOMonitor = pRenderContextMonitor->vboMonitor();
    GT_IF_WITH_ASSERT(pVBOMonitor != NULL)
    {
        // Calculate the buffers memory size:
        gtUInt64 buffersLeakMemorySize = 0;
        rcCalcVBOMemory = pVBOMonitor->calculateBuffersMemorySize(buffersLeakMemorySize);

        // Add the memory size to the event:
        contextMemorySize += buffersLeakMemorySize;
    }

    // Program objects:
    const gsProgramsAndShadersMonitor* pProgramsAndShadersMonitor = pRenderContextMonitor->programsAndShadersMonitor();
    GT_IF_WITH_ASSERT(pProgramsAndShadersMonitor != NULL)
    {
        // Calculate the shaders memory size:
        gtUInt64 shadersLeakMemorySize = 0;
        rcCalcShadersMemory = pProgramsAndShadersMonitor->calculateShadersMemorySize(shadersLeakMemorySize);

        // Add the memory size to the event:
        contextMemorySize += shadersLeakMemorySize;
    }

    // Display Lists:
    const gsDisplayListMonitor* pDisplayListMonitor = pRenderContextMonitor->displayListsMonitor();
    GT_IF_WITH_ASSERT(pDisplayListMonitor != NULL)
    {
        // Calculate the display list memory size:
        gtUInt64 dispListsLeakMemorySize = 0;
        rcCalcDisplayListMemory = pDisplayListMonitor->calculateDisplayListsMemorySize(dispListsLeakMemorySize);

        // Add the memory size to the event:
        contextMemorySize += dispListsLeakMemorySize;
    }

    retVal = rcCalcTextureMemory && rcCalcRBOMemory && rcCalcVBOMemory && rcCalcShadersMemory && rcCalcDisplayListMemory;
    return retVal;
}

