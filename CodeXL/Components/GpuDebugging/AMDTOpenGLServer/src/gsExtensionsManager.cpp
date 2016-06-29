//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsExtensionsManager.cpp
///
//==================================================================================

//------------------------------ gsExtensionsManager.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>
#include <AMDTServerUtilities/Include/suInterceptionFunctions.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>
#include <AMDTServerUtilities/Include/suAPIConnector.h>

// Local:
#include <src/gsStringConstants.h>
#include <src/gsOpenGLSpyInitFuncs.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsWrappersCommon.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsRenderContextExtensionsData.h>
#include <src/gsExtensionsManager.h>

// Linux / Mac specific includes:
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <dlfcn.h>
#endif

// Static members initializations:
gsExtensionsManager* gsExtensionsManager::_pMySingleInstance = NULL;


// On Linux and Mac , we use context 0 extensions data for all contexts:
// (See Implementation notes at this class header file)
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define GS_CONTEXT_DATA_STORAGE(contextSpyId) 0
#else
    #define GS_CONTEXT_DATA_STORAGE(contextSpyId) contextSpyId
#endif

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define GS_CHECK_CONTEXT_RANGE(contextSpyId) (void)(contextSpyId); if (true)
#else
    #define GS_CHECK_CONTEXT_RANGE(contextSpyId) GT_IF_WITH_ASSERT ((0 < contextSpyId) && (contextSpyId < (int)(_renderContextsExtensionsData.size())))
#endif


// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::instance
// Description: Returns the single instance of the gsExtensionsManager class
// Author:      Yaki Tebeka
// Date:        29/8/2003
// ---------------------------------------------------------------------------
gsExtensionsManager& gsExtensionsManager::instance()
{
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new gsExtensionsManager;
        GT_ASSERT(_pMySingleInstance);
    }

    return *_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::gsExtensionsManager
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        29/8/2004
// ---------------------------------------------------------------------------
gsExtensionsManager::gsExtensionsManager()
{
}


// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::~gsExtensionsManager
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        29/8/2004
// ---------------------------------------------------------------------------
gsExtensionsManager::~gsExtensionsManager()
{
    // Clear context to extension function map
    _renderContextsExtensionsData.deleteElementsAndClear();
}


// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::initialize
// Description: Initializes the extensions manager.
// Author:      Yaki Tebeka
// Date:        12/12/2010
// ---------------------------------------------------------------------------
bool gsExtensionsManager::initialize()
{
    bool retVal = false;

    // Initialize the OpenGL wrappers:
    bool rcWrappers = gsInitializeWrapperFunctions();
    GT_ASSERT(rcWrappers);

    // Initialize the _extensionIdToWrapperAddress map:
    bool rcWrappersAddresses = initializeWrapperAddresses();
    GT_ASSERT(rcWrappersAddresses);

    // Create context 0 (the non context context) extension function pointers structure:
    onContextCreatedEvent(0);

    retVal = rcWrappers && rcWrappersAddresses;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::getSpyUnifiedExtensionsString
// Description:
//   Inputs a render context id and returns the spy version of the supported
//   extensions string for this render context.
//   The "spy version" of the extensions string is:
//   - For OpenGL: An extension string that contains the OpenGL implementation
//     extensions string + Graphic Remedy's extensions.
//   - For OpenGL ES: An extension string that contains the extensions that were
//     explicitly marked as supported by Graphic Remedy's OpenGL ES implementation.
//
// Arguments: contextSpyId - The input render context spy index.
// Return Val: const GLubyte* - Will get the Spy version of render context
//                              supported extensions string, or NULL if the
//                              index is not valid or the render context was
//                              never current.
//
// Author:      Yaki Tebeka
// Date:        13/6/2006
// ---------------------------------------------------------------------------
const GLubyte* gsExtensionsManager::getSpyUnifiedExtensionsString(int contextSpyId) const
{
    const GLubyte* retVal = NULL;

    // Render context index range check:
    GS_CHECK_CONTEXT_RANGE(contextSpyId)
    {
        // Get the render context spy extensions string:
        int contextDataStorage = GS_CONTEXT_DATA_STORAGE(contextSpyId);
        const gtASCIIString& renderContextSpyExtStr = _renderContextsExtensionsData[contextDataStorage]->_spyExtensionsUnifiedString;

        // On Linux and Mac - glGetString can be called before any context was made active.
        // The below code handles this case.
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
        {
            if (renderContextSpyExtStr.isEmpty())
            {
                ((gsExtensionsManager*)this)->calculateRenderContextSpyUnifiedExtensionsString(0);
            }
        }
#endif

        // If the extensions string was calculated:
        if (!(renderContextSpyExtStr.isEmpty()))
        {
            // Return it:
            retVal = (const GLubyte*)(renderContextSpyExtStr.asCharArray());
        }
        else
        {
            // Call the real function (This would generate an OpenGL error
            // on 3.1+ contexts, emulating the real behavior):
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetString);
            retVal = gs_stat_realFunctionPointers.glGetString(GL_EXTENSIONS);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetString);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::getSpyExtensionString
// Description: Returns the name of the extension at the extensionIndex-th place
//              in context number contextSpyId GL_EXTENSIONS string array
// Author:      Uri Shomroni
// Date:        3/1/2010
// ---------------------------------------------------------------------------
const GLubyte* gsExtensionsManager::getSpyExtensionString(int contextSpyId, GLuint extensionIndex) const
{
    const GLubyte* retVal = NULL;

    GS_CHECK_CONTEXT_RANGE(contextSpyId)
    {
        int contextDataStorage = GS_CONTEXT_DATA_STORAGE(contextSpyId);
        const gsRenderContextExtensionsData* pContextExtensionsData = _renderContextsExtensionsData[contextDataStorage];
        GT_IF_WITH_ASSERT(pContextExtensionsData != NULL)
        {
            // Get the vector size:
            unsigned int numberOfExtensions = (unsigned int)(pContextExtensionsData->_spyExtensionStrings.size());

            // On Linux and Mac - glGetStringi can be called before any context was made active.
            // The below code handles this case.
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
            {
                if (numberOfExtensions == 0)
                {
                    ((gsExtensionsManager*)this)->calculateRenderContextSpyExtensionStrings(0);
                    numberOfExtensions = (unsigned int)(pContextExtensionsData->_spyExtensionStrings.size());
                }
            }
#endif

            if (extensionIndex < numberOfExtensions)
            {
                // The extension number is valid, return the string:
                retVal = (const GLubyte*)(pContextExtensionsData->_spyExtensionStrings[extensionIndex].asCharArray());
            }
            else
            {
                // Call the real function to generate an OpenGL error as the real implementation does:
#ifdef _AMDT_OPENGLSERVER_EXPORTS
                if (pContextExtensionsData->_extensionFunctionsRealImpl.glGetStringi != NULL)
                {
                    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetStringi);
                    retVal = pContextExtensionsData->_extensionFunctionsRealImpl.glGetStringi(GL_EXTENSIONS, extensionIndex);
                    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetStringi);
                }

#endif
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::getNumberOfSpyExtensionStrings
// Description: Returns the virtual value of glGetInteger(GL_NUM_EXTENSIONS)
// Author:      Uri Shomroni
// Date:        3/1/2010
// ---------------------------------------------------------------------------
unsigned int gsExtensionsManager::getNumberOfSpyExtensionStrings(int contextSpyId) const
{
    unsigned int retVal = 0;

    GS_CHECK_CONTEXT_RANGE(contextSpyId)
    {
        int contextDataStorage = GS_CONTEXT_DATA_STORAGE(contextSpyId);
        const gsRenderContextExtensionsData* pContextExtensionsData = _renderContextsExtensionsData[contextDataStorage];
        GT_IF_WITH_ASSERT(pContextExtensionsData != NULL)
        {
            retVal = (unsigned int)(pContextExtensionsData->_spyExtensionStrings.size());

            // On Linux and Mac - glGetIntegerv(GL_NUM_EXTENSIONS) can be called before any context was made active.
            // The below code handles this case.
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
            {
                if (retVal == 0)
                {
                    ((gsExtensionsManager*)this)->calculateRenderContextSpyExtensionStrings(0);
                    retVal = (unsigned int)(pContextExtensionsData->_spyExtensionStrings.size());
                }
            }
#endif
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::isExtensionSupported
// Description: Checks if a given extension is supported by a given render context.
// Arguments: contextSpyId - The queried render context spy id.
//            extensionId - The queried extension id (see apOpenGLExtensionsId).
// Return Val: bool  - true iff the input extension is supported by the input
//                     render context.
// Author:      Yaki Tebeka
// Date:        13/6/2006
// ---------------------------------------------------------------------------
bool gsExtensionsManager::isExtensionSupported(int contextSpyId, apOpenGLExtensionsId extensionId) const
{
    bool retVal = false;

    // Render context index range check:
    GS_CHECK_CONTEXT_RANGE(contextSpyId)
    {
        // Extension id range check:
        GT_IF_WITH_ASSERT((0 <= extensionId) && (extensionId < AP_AMOUNT_OF_SUPPORTED_OGL_EXTENSIONS))
        {
            // Return the extension's support status:
            int contextDataStorage = GS_CONTEXT_DATA_STORAGE(contextSpyId);
            retVal = _renderContextsExtensionsData[contextDataStorage]->_isExtensionSupported[extensionId];
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::wrapperFunctionAddress
// Description:
//   Inputs an OpenGL / WGL extension function name and returns its wrapper
//   function pointer,
//   or:
//   - NULL if the extension is not supported for the current thread render context.
//   - The real function pointer, if the current spy does not support this extension.
//
// Arguments:   functionName - The OpenGL / WGL extension function name (as passed
//                             to wglGetProcAddress)
// Return val: osProcedureAddress - The wrapper function pointer.
// Author:      Yaki Tebeka
// Date:        6/9/2004
// Implementation Notes:
//   If the extension is supported for the current context
//     If the Spy supports this extension:
//        - log the extension support.
//        - returns a pointer to the extension wrapper function.
//     Else
//        - return a pointer to the real extension implementation.
//   Else (the extension is not supported for the current context)
//     If the extension is a Graphic Remedy extension that is supported by CodeXL
//        - returns its wrapper address.
//     Else
//        - returns NULL.
// ---------------------------------------------------------------------------
osProcedureAddress gsExtensionsManager::wrapperFunctionAddress(const gtASCIIString& functionName)
{
    osProcedureAddress retVal = NULL;

    // Build the function name as unicode string:
    gtString functionNameUnicode;
    functionNameUnicode.fromASCIIString(functionName.asCharArray());

    // Output debug log printout:
    gtString debugLogMsg = GS_STR_retrievingExtensionFunctionPointer;
    debugLogMsg += functionNameUnicode;
    OS_OUTPUT_DEBUG_LOG(debugLogMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);

    // Ask OpenGL for the real implementation of the requested extension:
    // TO_DO: Unicode - check performance
    osProcedureAddress realExtensionAddress = gsGetSystemsOGLModuleProcAddress(functionName.asCharArray());

    // If extension is NOT supported for the current context:
    if (realExtensionAddress == NULL)
    {
        // If its a function implemented by the spy - get its pointer:
        // (Otherwise - we return NULL)
        retVal = spyImplementedExtensionAddress(functionNameUnicode);
    }
    else
    {
        // The extension is supported for the current context:

        // The return value will be the real extension address until we will find the wraper function address:
        retVal = realExtensionAddress;

        // Get the extension function id:
        apMonitoredFunctionId extensionFuncId = su_stat_theMonitoredFunMgr.monitoredFunctionId(functionNameUnicode.asCharArray());
        int extensionFunctionIndex = (extensionFuncId < apMonitoredFunctionsAmount) ? functionIndexFromMonitoredFunctionId(extensionFuncId) : -1;

        if (extensionFunctionIndex == -1)
        {
            // This extension is not supported (yet :-) by the gremedy OpenGL server:
            gtString debugMsg = GS_STR_unsupportedExtensionUse;
            debugMsg += L" (";
            debugMsg += functionNameUnicode;
            debugMsg += L")";
            OS_OUTPUT_DEBUG_LOG(debugMsg.asCharArray(), OS_DEBUG_LOG_INFO);
        }
        else
        {
            // Find the appropriate wrapper function address:
            osProcedureAddress wrapperFuncAddress = NULL;

            // Sanity check:
            if ((extensionFunctionIndex >= 0) && (extensionFunctionIndex < (int)_extensionIdToWrapperAddress.size()))
            {
                wrapperFuncAddress = _extensionIdToWrapperAddress[extensionFunctionIndex];
            }

            if (wrapperFuncAddress != NULL)
            {
                // Get the current thread "current render context" Spy id:
                int renderContextid = 0;

#ifndef _GR_IPHONE_DEVICE_BUILD
                // On the iPhone, calling this causes thread creation to be reported too early, so we avoid it here (since it is ignored by extensionRealImplementationPointers )
                gsOpenGLMonitor& theOpenGLMonitor = gsOpenGLMonitor::instance();
                renderContextid = theOpenGLMonitor.currentThreadRenderContextSpyId();
#endif

                // Store the address of the "real" function implementation struct for the
                // current render context:
                gsMonitoredFunctionPointers* pExtensionsRealImplPtrs = extensionsRealImplementationPointers(renderContextid);

                if (pExtensionsRealImplPtrs)
                {
                    ((osProcedureAddress*)(pExtensionsRealImplPtrs))[extensionFunctionIndex] = realExtensionAddress;

                    // Return the wrapper function address:
                    retVal = wrapperFuncAddress;
                }
            }
            else
            {
                // On Linux, glxGetProcAddress and glxGetProcAddressARB should return also base (I.E: not extension)
                // function pointers. The below code returns the spy implementation pointer of base functions.
                // In Mac, this class is used in the interception method, and thus we need to get the pointers
                // to "normal" functions as well.
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
                {
                    // Get gremedy's OGL server module implementation for the input function:
                    osProcedureAddress spyImplementationAddress = getOGLBaseFunctionSpyAddress(functionName);

                    // If the spy has an implementation for this function, return the spy implementation
                    // function pointer:
                    if (spyImplementationAddress != NULL)
                    {
                        retVal = spyImplementationAddress;
                    }
                }
#endif
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::spyImplementedExtensionAddress
// Description:
//   Inputs an OpenGL / ES extension function name. If it is an extension that is
//   implemented by the OpenGL / ES spy, returns the spy implementing function pointer,
//   Otherwise - returns NULL.
// Arguments:   functionName - The queried OpenGL extension function name.
// Author:      Yaki Tebeka
// Date:        2/2/2005
// ---------------------------------------------------------------------------
osProcedureAddress gsExtensionsManager::spyImplementedExtensionAddress(const gtString& functionName) const
{
    osProcedureAddress retVal = NULL;

    // Get the extension function id:
    apMonitoredFunctionId extensionFuncId = su_stat_theMonitoredFunMgr.monitoredFunctionId(functionName.asCharArray());
    int extensionFunctionIndex = (extensionFuncId < apMonitoredFunctionsAmount) ? functionIndexFromMonitoredFunctionId(extensionFuncId) : -1;

    if ((extensionFunctionIndex >= 0) && (extensionFunctionIndex < (int)_extensionIdToWrapperAddress.size()))
    {
        // Spy implemented function types:
        static unsigned int spyImplementedFunctionTypes = AP_OPENGL_GREMEDY_EXTENSION_FUNC | AP_OPENGL_ES_EXTENSION_FUNC | AP_EGL_EXTENSION_FUNC;

        // Verify that this is a spy implemented function:
        unsigned int functionAPIType = su_stat_theMonitoredFunMgr.monitoredFunctionAPIType(extensionFuncId);

        if (functionAPIType & spyImplementedFunctionTypes)
        {
            // Get its wrapper address:
            retVal = _extensionIdToWrapperAddress[extensionFunctionIndex];
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::extensionsRealImplementationPointers
// Description: Inputs a render context index, and return a structure that holds
//              its extension functions real implementation pointers.
// Author:      Yaki Tebeka
// Date:        8/9/2004
// ---------------------------------------------------------------------------
gsMonitoredFunctionPointers* gsExtensionsManager::extensionsRealImplementationPointers(int contextSpyId)
{
#ifdef _GR_IPHONE_DEVICE_BUILD
    // We change the pointers in the gsMonitoredFunctionPointers structs in run time, so
    // we want to allow usage of only one such struct. So when the spy requests a context's
    // pointers, we return the static one:
    gsMonitoredFunctionPointers* retVal = &gs_stat_realFunctionPointers;
#else
    (void)(contextSpyId); // Resolve the compiler warning for the Linux variant
    // Get the structure that match the requested pixel format index:
    int contextDataStorage = GS_CONTEXT_DATA_STORAGE(contextSpyId);
    gsMonitoredFunctionPointers* retVal = &_renderContextsExtensionsData[contextDataStorage]->_extensionFunctionsRealImpl;
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::currentRenderContextExtensionsRealImplPointers
// Description: Returns a structure containing the real implementation functions
//              pointers of the current render context.
//              (Or NULL in case of failure)
// Author:      Yaki Tebeka
// Date:        8/9/2004
// ---------------------------------------------------------------------------
gsMonitoredFunctionPointers* gsExtensionsManager::currentRenderContextExtensionsRealImplPointers()
{
    gsMonitoredFunctionPointers* retVal = NULL;

    // See implementation notes
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    int contextDataStorage = 0;
#else
    // Get the spy id of the "current render context" of the thread that called this function:
    int contextDataStorage = gs_stat_openGLMonitorInstance.currentThreadRenderContextSpyId();
#endif

    GT_IF_WITH_ASSERT(contextDataStorage < (int)_renderContextsExtensionsData.size())
    {
#ifdef _GR_IPHONE_DEVICE_BUILD
        // We change the pointers in the gsMonitoredFunctionPointers structs in run time, so
        // we want to allow usage of only one such struct. So when the spy requests a context's
        // pointers, we return the static one:
        retVal = &gs_stat_realFunctionPointers;
#else
        retVal = &(_renderContextsExtensionsData[contextDataStorage]->_extensionFunctionsRealImpl);
#endif
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::copyExtensionPointerFromOtherContexts
// Description: Inputs an extension function id. Looks for the function pointer in
//              all render contexts extension function pointers structures. If the function
//              pointer is found, copies it into the current render context extension
//              functions structure.
// Arguments:   funcId - The searched extension function id.
// Return Val:  bool - true iff the extension function pointer was found (and copied)
// Author:      Yaki Tebeka
// Date:        23/3/2005
// ---------------------------------------------------------------------------
bool gsExtensionsManager::copyExtensionPointerFromOtherContexts(apMonitoredFunctionId funcId)
{
    bool retVal = false;

    // Iterate all render contexts:
    int renderContextsAmount = (int)_renderContextsExtensionsData.size();

    for (int i = 0; i < renderContextsAmount; i++)
    {
        // Get this render context pointer for the requested function id:
        gsMonitoredFunctionPointers* pExtensionsRealImplPtrs = extensionsRealImplementationPointers(i);
        void* pRealExtensionAddress = ((void**)(pExtensionsRealImplPtrs))[funcId];

        // If we found a render context that has the extension function pointer:
        if (pRealExtensionAddress)
        {
            // Copy the extension pointer to the current context extension functions pointers structure:
            gsMonitoredFunctionPointers* pCurrentContextExtensionsRealImplPtrs = currentRenderContextExtensionsRealImplPointers();

            if (pCurrentContextExtensionsRealImplPtrs != NULL)
            {
                ((void**)(pCurrentContextExtensionsRealImplPtrs))[funcId] = pRealExtensionAddress;
                retVal = true;
            }

            break;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::getExtensionPointerFromSystem
// Description: Inputs an extension function id. Calls the system (WGL) to get
//              its function pointer and copies it into the current render context
//              extension functions structure.
// Arguments:   funcId - The searched extension function id.
// Return Val:  bool - true iff the extension function pointer was found (and copied)
// Author:      Yaki Tebeka
// Date:        12/6/2006
// ---------------------------------------------------------------------------
bool gsExtensionsManager::getExtensionPointerFromSystem(apMonitoredFunctionId funcId)
{
    bool retVal = false;

    // Get the extension function name:
    gtString functionName = su_stat_theMonitoredFunMgr.monitoredFunctionName(funcId);
    GT_IF_WITH_ASSERT(!functionName.isEmpty())
    {
        // TO_DO: Unicode check performance
        // Ask the system for the real implementation of the requested extension:
        osProcedureAddress realExtensionAddress = gsGetSystemsOGLModuleProcAddress(functionName.asASCIICharArray());

        if (realExtensionAddress != NULL)
        {
            // Copy the extension pointer to the current context extension functions pointers structure:
            gsMonitoredFunctionPointers* pCurrentContextExtensionsRealImplPtrs = currentRenderContextExtensionsRealImplPointers();

            if (pCurrentContextExtensionsRealImplPtrs != NULL)
            {
                ((osProcedureAddress*)(pCurrentContextExtensionsRealImplPtrs))[funcId] = realExtensionAddress;
                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::onContextCreatedEvent
// Description: Is called when a new context is created.
// Arguments:   contextSpyId - The spy id of the created context.
// Author:      Yaki Tebeka
// Date:        1/9/2004
// ---------------------------------------------------------------------------
void gsExtensionsManager::onContextCreatedEvent(int contextSpyId)
{
    // On Linux and Mac, we use context 0 extensions data for all contexts:
    // (See Implementation notes at this class header file)
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    if (contextSpyId != 0)
    {
        return;
    }

#endif

    // Get the current extensions data vector size:
    int currentVecSize = (int)_renderContextsExtensionsData.size();

    // If we need to allocate new "extensions data" structures:
    if (contextSpyId >= currentVecSize)
    {
        // Allocate them:
        for (int i = currentVecSize; i <= contextSpyId; i++)
        {
            // Allocate the current structure:
            gsRenderContextExtensionsData* pStruct = new gsRenderContextExtensionsData;

            // Add it to the "extensions data" structures vector:
            _renderContextsExtensionsData.push_back(pStruct);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::onFirstTimeContextMadeCurrent
// Description: Is called when a context becomes current for the first time.
// Arguments: contextSpyId - The spy id of the render context.
// Author:      Yaki Tebeka
// Date:        13/6/2006
// ---------------------------------------------------------------------------
void gsExtensionsManager::onFirstTimeContextMadeCurrent(int contextSpyId, const int contextOpenGLVersion[2], bool isCompatibilityProfileActive)
{
    // Note that we perform both operations for OpenGL 3.0 contexts and compatibility contexts, so we could answer calls to
    // glGetString(GL_EXTENSIONS) as well as ones to glGetStringi(GL_EXTENSIONS, index).
    if (contextOpenGLVersion[0] >= 3)
    {
        // Calculate the extension strings vector:
        calculateRenderContextSpyExtensionStrings(contextSpyId);
    }

    // If one of the extensions is GL_ARB_compatibility or we have the compatibility profile, we can call glGetString(GL_EXTENSIONS):
    bool isCompatibiltyContext = isCompatibilityProfileActive;
    GS_CHECK_CONTEXT_RANGE(contextSpyId)
    {
        // Get the context data:
        int contextDataStorage = GS_CONTEXT_DATA_STORAGE(contextSpyId);

        const gtVector<gtASCIIString>& contextExtensionStrings = _renderContextsExtensionsData[contextDataStorage]->_spyExtensionStrings;
        int numberOfExtensions = (int)contextExtensionStrings.size();

        if (numberOfExtensions > 0)
        {
            // Search for the extension:
            gtString compatibilityExtensionName;
            apOpenGLExtensionsIdToString(AP_GL_ARB_compatibility, compatibilityExtensionName);

            for (int i = 0; i < numberOfExtensions; i++)
            {
                if (compatibilityExtensionName.isEqual(contextExtensionStrings[i].asCharArray()))
                {
                    isCompatibiltyContext = true;
                    break;
                }
            }
        }
    }

    if (isCompatibiltyContext || (contextOpenGLVersion[0] < 3) || ((contextOpenGLVersion[0] == 3) && (contextOpenGLVersion[1] == 0)))
    {
        // Calculate the render context "spy extensions string":
        calculateRenderContextSpyUnifiedExtensionsString(contextSpyId);
    }

    // Update the render context's extensions support:
    updateExtensionsSupport(contextSpyId, contextOpenGLVersion);
}

// ---------------------------------------------------------------------------
// Name:        functionIndexFromMonitoredFunctionId
// Description: Takes a monitored function Id and returns its index in the
//              gsMonitoredFunctionPointers struct, or -1 if it isn't an OpenGL
//              function.
// Author:      Uri Shomroni
// Date:        26/07/2015
// ---------------------------------------------------------------------------
int gsExtensionsManager::functionIndexFromMonitoredFunctionId(apMonitoredFunctionId funcId) const
{
    int retVal = -1;

    // We currently do not support OpenGL ES:
    bool isGLES = ((apFirstOpenGLESFunction <= funcId) && (apLastOpenGLESFunction >= funcId));

    if (!isGLES)
    {
        GT_IF_WITH_ASSERT((funcId >= apFirstOpenGLFunction) && (funcId <= apLastOpenGLFunction))
        {
            retVal = (int)funcId - (int)apFirstOpenGLFunction;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::initializeWrapperAddresses
// Description: Initialize the _extensionIdToWrapperAddress map.
// Author:      Yaki Tebeka
// Date:        1/9/2004
// ---------------------------------------------------------------------------
bool gsExtensionsManager::initializeWrapperAddresses()
{
    bool retVal = false;

    // Get the OpenGL spy type according to configuration:
    suAPIConnector::suSpyType oglSpyType = suAPIConnector::SU_SPY_UNKNOWN;
    /*  #if defined (_GR_OPENGL32) || defined (_GR_OPENGL_MODULE)
            oglSpyType = suAPIConnector::SU_SPY;
        #elif defined(_GR_OPENGLES_COMMON) || defined(_GR_OPENGLES_IPHONE)
            #ifdef _GR_IPHONE_DEVICE_BUILD
                oglSpyType = suAPIConnector::SU_SPY_IPHONE;
            #else
                oglSpyType = suAPIConnector::SU_SPY_ES;
            #endif
        #elif defined(_GR_OPENGLES_COMMON_LITE)
            oglSpyType = suAPIConnector::SU_SPY_ES_LITE;
        #else
            #error Error: Unknown compile target!
        #endif*/
#ifdef _AMDT_OPENGLSERVER_EXPORTS
    oglSpyType = suAPIConnector::SU_SPY;
#else
#error Unknown Server Type!
#endif

    // Get this module handle:
    osModuleHandle thisModuleHandle;
    bool rc1 = suAPIConnector::instance().getServerModuleHandle(oglSpyType, AP_OPENGL_API_CONNECTION, thisModuleHandle);
    GT_IF_WITH_ASSERT(rc1)
    {
        retVal = true;

        // Get the monitored functions manager instance:
        apMonitoredFunctionsManager& theMonitoredFunMgr = apMonitoredFunctionsManager::instance();

        _extensionIdToWrapperAddress.reserve(apLastOpenGLFunction - apFirstOpenGLFunction + 1);

        // Iterate the monitored extension functions:
        for (int i = apFirstOpenGLFunction; i <= apLastOpenGLFunction; i++)
        {
            // Will get the current extension function address:
            osProcedureAddress procedureAddress = NULL;

            // If this is an extension function:
            if ((apLastOpenGLBaseFunction < i) && (i <= apLastOpenGLExtensionFunction))
            {
                // Get the current extension wrapper function type:
                unsigned int extensionFuncType = theMonitoredFunMgr.monitoredFunctionAPIType((apMonitoredFunctionId)i);

                // Get the extension function types supported by this spy:
                // ------------------------------------------------------

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
#ifdef _GR_OPENGLES_IPHONE
                // In the iPhone, we need to get access to all the functions' wrappers, including the
                // "Regular" OpenGL ES ones.
                unsigned int supportedFunctionTypes = AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC |
                                                      AP_OPENGL_ES_MAC_EXTENSION_FUNC | AP_OPENGL_GREMEDY_EXTENSION_FUNC;
#else
                // In Mac OS X, we need to get access to all the functions' wrappers, including the
                // "Regular" OpenGL ones.
                unsigned int supportedFunctionTypes = AP_OPENGL_GENERIC_FUNC | AP_OPENGL_EXTENSION_FUNC |
                                                      AP_OPENGL_GREMEDY_EXTENSION_FUNC | AP_CGL_FUNC;
#endif
#else
#ifdef _AMDT_OPENGLSERVER_EXPORTS
                // OpenGL spy DLL or module:
                unsigned int supportedFunctionTypes = AP_OPENGL_EXTENSION_FUNC | AP_WGL_EXTENSION_FUNC |
                                                      AP_OPENGL_GREMEDY_EXTENSION_FUNC;
#elif defined (OS_OGL_ES_IMPLEMENTATION_DLL_BUILD)
                // OpenGL ES implementation DLL:
                unsigned int supportedFunctionTypes = AP_OPENGL_ES_EXTENSION_FUNC | AP_EGL_EXTENSION_FUNC |
                                                      AP_OPENGL_GREMEDY_EXTENSION_FUNC;
#else
#error Error: unknown module!
#endif
#endif

                // If the extension function is supported by this spy:
                if (extensionFuncType & supportedFunctionTypes)
                {
                    // Get the current extension wrapper procedure name:
                    // TO_DO: Unicode check performance
                    gtString functionName = theMonitoredFunMgr.monitoredFunctionName((apMonitoredFunctionId)i);

                    // Get the wrapper function address:
                    bool rc2 = osGetProcedureAddress(thisModuleHandle, functionName.asASCIICharArray(), procedureAddress, false);

                    if (!rc2)
                    {
                        // We didn't manage to to get the spy queried function.
                        // This means that either:
                        // a. It was not exported through the spy OpenGL32Spy.def file.
                        // b. It was not implemented in the gsExtensionsWrappers.cpp.
                        // THIS PROBLEM HAS TO BE FIXED !!!
                        gtString errMsg;
                        errMsg.appendFormattedString(L"Could not find procedure address of functions %ls.", functionName.asCharArray());
                        GT_ASSERT_EX(false, errMsg.asCharArray());
                        retVal = false;
                    }
                }
            }

            // Add the current extension to the _extensionIdToWrapperAddress array:
            _extensionIdToWrapperAddress.push_back(procedureAddress);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::updateExtensionsSupport
// Description: Updates the extensions support of a given render context.
//              This must be called after the appropriate calculateExtensionString
//              function.
// Author:      Yaki Tebeka
// Date:        22/2/2006
// Implementation notes:
//   We assume that the input render context is the current render context.
// ---------------------------------------------------------------------------
void gsExtensionsManager::updateExtensionsSupport(int contextSpyId, const int contextOpenGLVersion[2])
{
    // Render context index range check:
    GS_CHECK_CONTEXT_RANGE(contextSpyId)
    {
        // Get the render context extensions support array:
        int contextDataStorage = GS_CONTEXT_DATA_STORAGE(contextSpyId);
        gsRenderContextExtensionsData* pContextExtensionsData = _renderContextsExtensionsData[contextDataStorage];
        GT_IF_WITH_ASSERT(pContextExtensionsData != NULL)
        {
            bool* pIsExtensionSupported = (bool*)(&(pContextExtensionsData->_isExtensionSupported));

            gtASCIIString unifiedExtensionString;

            if (contextOpenGLVersion[0] < 3)
            {
                // Get the OpenGL extensions string for the the input context:
                unifiedExtensionString = pContextExtensionsData->_spyExtensionsUnifiedString;
            }
            else // contextOpenGLVersion[0] >= 3
            {
                // Construct a string from the partial strings:
                const gtVector<gtASCIIString>& extensionStringsVector = pContextExtensionsData->_spyExtensionStrings;
                int numberOfExtensions = (int)extensionStringsVector.size();

                for (int i = 0; i < numberOfExtensions; i++)
                {
                    // Add a space AFTER each extension, so that the string ends in a space as well.
                    unifiedExtensionString.append(extensionStringsVector[i]).append(' ');
                }
            }

            const char* pOpenGLExtensionsString2 = NULL;

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
            {
                // In Windows, get the wgl extensions string:
                // make sure we have a pointer to wglGetExtensionsStringARB:
                if (gs_stat_realFunctionPointers.wglGetExtensionsStringARB == NULL)
                {
                    bool rcFunc = getExtensionPointerFromSystem(ap_wglGetExtensionsStringARB);
                    GT_ASSERT(rcFunc);
                }

                const gsRenderContextMonitor* pRCMon = gs_stat_openGLMonitorInstance.renderContextMonitor(contextSpyId);
                GT_IF_WITH_ASSERT(pRCMon != NULL)
                {
                    HDC contextHDC = pRCMon->deviceContextOSHandle();
                    GT_IF_WITH_ASSERT(contextHDC != NULL)
                    {
                        gsMonitoredFunctionPointers* pRealExtFuncsPtrs = gs_stat_extensionsManager.currentRenderContextExtensionsRealImplPointers();

                        GT_IF_WITH_ASSERT(pRealExtFuncsPtrs != NULL)
                        {
                            if ((pRealExtFuncsPtrs->wglGetExtensionsStringARB) != NULL)
                            {
                                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_wglGetExtensionsStringARB);
                                pOpenGLExtensionsString2 = (pRealExtFuncsPtrs->wglGetExtensionsStringARB)(contextHDC);
                                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_wglGetExtensionsStringARB);
                            }
                        }
                    }
                }
            }
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
            {
                // In Linux, get the glX extensions String:
                const gsRenderContextMonitor* pRCMon = gs_stat_openGLMonitorInstance.renderContextMonitor(contextSpyId);
                GT_IF_WITH_ASSERT(pRCMon != NULL)
                {
                    // Get the display related to the render context:
                    Display* pContextDisplay = pRCMon->deviceContextOSHandle();

                    if (pContextDisplay != NULL)
                    {
                        // Get the id of the screen associated with the display:
                        int screenId = DefaultScreen(pContextDisplay);

                        // Get the GLX extensions string:
                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glXQueryExtensionsString);
                        pOpenGLExtensionsString2 = (const char*)gs_stat_realFunctionPointers.glXQueryExtensionsString(pContextDisplay, screenId);
                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glXQueryExtensionsString);
                    }
                }
            }
#endif

            GT_IF_WITH_ASSERT(!unifiedExtensionString.isEmpty())
            {
                // Get the pointer now to make fewer calls to the accessor:
                const char* pOpenGLExtensionsString = unifiedExtensionString.asCharArray();

                // Iterate the supported extensions:
                for (int i = 0; i < AP_AMOUNT_OF_SUPPORTED_OGL_EXTENSIONS; i++)
                {
                    // Get the current extension name:
                    gtString currentExtName;
                    bool rc = apOpenGLExtensionsIdToString(apOpenGLExtensionsId(i), currentExtName);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        // Check if the extension is supported:
                        // TO_DO: Unicode is this converted needed?
                        bool isCurrentExtensionSupported = apIsOpenGLExtensionSupported(pOpenGLExtensionsString, currentExtName.asASCIICharArray());
                        pIsExtensionSupported[i] = isCurrentExtensionSupported;

                        if (!isCurrentExtensionSupported)
                        {
                            // This could be a WGL or GLX extension:
                            if (pOpenGLExtensionsString2 != NULL)
                            {
                                // TO_DO: Unicode - performance
                                isCurrentExtensionSupported = apIsOpenGLExtensionSupported(pOpenGLExtensionsString2, currentExtName.asASCIICharArray());
                                pIsExtensionSupported[i] = isCurrentExtensionSupported;
                            }
                        }
                    }
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::calculateRenderContextSpyUnifiedExtensionsString
// Description: Inputs a render context spy id and calculates its spy extensions
//              string.
// Author:      Yaki Tebeka
// Date:        13/6/2006
// Implementation notes:
//   We assume that the input render context is the current render context.
// ---------------------------------------------------------------------------
void gsExtensionsManager::calculateRenderContextSpyUnifiedExtensionsString(int contextSpyId)
{
    // Render context index range check:
    GS_CHECK_CONTEXT_RANGE(contextSpyId)
    {
        // Get the render context spy extensions string:
        int contextDataStorage = GS_CONTEXT_DATA_STORAGE(contextSpyId);
        gtASCIIString& renderContextSpyExtStr = _renderContextsExtensionsData[contextDataStorage]->_spyExtensionsUnifiedString;

        // If this is the Win32 / Linux spy:
        // #if defined (_GR_OPENGL32) || defined (_GR_OPENGL_MODULE) || defined (_GR_OPENGLES_IPHONE)
#ifdef _AMDT_OPENGLSERVER_EXPORTS
        calculateCurrentRenderContextOpenGLSpyUnifiedExtensionsString(renderContextSpyExtStr);
#endif

        // If this is the OpenGL ES emulator build:
#ifdef OS_OGL_ES_IMPLEMENTATION_DLL_BUILD
        calculateCurrentRenderContextOpenGLESSpyExtensionsString(renderContextSpyExtStr);
#endif
    }
}

// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::calculateRenderContextSpyExtensionStrings
// Description: Calculates the extension strings vector for context number contextSpyId
// Author:      Uri Shomroni
// Date:        3/1/2010
// ---------------------------------------------------------------------------
void gsExtensionsManager::calculateRenderContextSpyExtensionStrings(int contextSpyId)
{
#ifdef _AMDT_OPENGLSERVER_EXPORTS
    // Render context index range check:
    GS_CHECK_CONTEXT_RANGE(contextSpyId)
    {
        // Get the context data:
        int contextDataStorage = GS_CONTEXT_DATA_STORAGE(contextSpyId);
        gsRenderContextExtensionsData* pContextExtensionsData = _renderContextsExtensionsData[contextDataStorage];

        GT_IF_WITH_ASSERT(pContextExtensionsData != NULL)
        {
            // Get the render context spy extension strings vector:
            gtVector<gtASCIIString>& extensionStringsVector = pContextExtensionsData->_spyExtensionStrings;

            // Clear it:
            extensionStringsVector.clear();

            // Get the real number of extensions:
            GLint numberOfExtensions = 0;
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
            gs_stat_realFunctionPointers.glGetIntegerv(GL_NUM_EXTENSIONS, &numberOfExtensions);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);

            // Make sure we have the function pointer:
            gsMonitoredFunctionPointers& contextRealFuncs = pContextExtensionsData->_extensionFunctionsRealImpl;

            if (contextRealFuncs.glGetStringi == NULL)
            {
                bool rcFunc = getExtensionPointerFromSystem(ap_glGetStringi);
                GT_ASSERT(rcFunc);
            }

            GT_IF_WITH_ASSERT(contextRealFuncs.glGetStringi != NULL)
            {
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetStringi);
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);

                for (int i = 0; i < numberOfExtensions; i++)
                {
                    const GLubyte* pCurrentExt = contextRealFuncs.glGetStringi(GL_EXTENSIONS, (GLuint)i);

                    if (pCurrentExt != NULL)
                    {
                        gtASCIIString currentExtension = (const char*)pCurrentExt;
                        extensionStringsVector.push_back(currentExtension);
                    }
                    else
                    {
                        // Construct an error string:
                        GLenum oglError = gs_stat_realFunctionPointers.glGetError();
                        gtString errString;
                        errString.appendFormattedString(L"Could not get string for extension #%d. Error code is: 0x%04X.", i, oglError);
                        OS_OUTPUT_DEBUG_LOG(errString.asCharArray(), OS_DEBUG_LOG_DEBUG);

                        // Insert a dummy string instead, to avoid index shifting:
                        static const gtASCIIString dummyExtensionString;
                        extensionStringsVector.push_back(dummyExtensionString);
                    }
                }

                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetStringi);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);
            }

            // Add the gremedy extensions:
            extensionStringsVector.push_back(GS_STR_GREMEDY_SUPPORTED_EXTENSION1);
            extensionStringsVector.push_back(GS_STR_GREMEDY_SUPPORTED_EXTENSION2);
        }
    }
#else
    // This function should not be called:
    GT_ASSERT(false);
#endif
}

// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::calculateCurrentRenderContextOpenGLSpyUnifiedExtensionsString
// Description: Calculates the current render context OpenGL spy extensions string.
// Arguments: spyExtesnionsStr - Will get the calculated string
// Author:      Yaki Tebeka
// Date:        14/6/2006
// ---------------------------------------------------------------------------
void gsExtensionsManager::calculateCurrentRenderContextOpenGLSpyUnifiedExtensionsString(gtASCIIString& spyExtesnionsStr)
{
    // Initialize the spy extensions string to contain the OpenGL implementation extensions string:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetString);
    spyExtesnionsStr = (const char*)(gs_stat_realFunctionPointers.glGetString(GL_EXTENSIONS));
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetString);

    // Check if the extensions string last char is a space:
    int extensionsStringLength = spyExtesnionsStr.length();
    bool isLastExtensionsStringCharSpace = false;

    if (extensionsStringLength > 0)
    {
        isLastExtensionsStringCharSpace = (spyExtesnionsStr[extensionsStringLength - 1] == ' ');
    }

    // Add space before Graphic Remedy's extensions (if it does not already there):
    if (!isLastExtensionsStringCharSpace)
    {
        spyExtesnionsStr += ' ';
    }

    // Add Graphic Remedy's extensions:
    spyExtesnionsStr += GS_STR_GREMEDY_SUPPORTED_EXTENSIONS;

    // If the extensions string last char was a space, we will also make the last char of the new
    // extension string a space (Its not a part of the OpenGL glGetString spec, but some applications
    // are sensitive to this space - see Case 994)
    if (isLastExtensionsStringCharSpace)
    {
        spyExtesnionsStr += ' ';
    }
}


// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::calculateCurrentRenderContextOpenGLESSpyExtensionsString
// Description: Inputs a render context spy id and calculates its OpenGL ES spy
//              extensions string.
// Arguments: spyExtesnionsStr - Will get the calculated string
// Author:      Yaki Tebeka
// Date:        14/6/2006
// Implementation notes:
//   We assume that the input render context is the current render context.
// ---------------------------------------------------------------------------
void gsExtensionsManager::calculateCurrentRenderContextOpenGLESSpyExtensionsString(gtASCIIString& spyExtesnionsStr)
{
    // Get the real renderer extensions string for this render context:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetString);
    gtASCIIString realRendererExensionsStr;
    realRendererExensionsStr = (const char*)(gs_stat_realFunctionPointers.glGetString(GL_EXTENSIONS));
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetString);

    // Extensions emulated by our OpenGL ES implementation:
    spyExtesnionsStr += "OES_draw_texture ";

    // If the real renderer supports GL_EXT_texture:
    if (apIsOpenGLExtensionSupported(realRendererExensionsStr.asCharArray(), "GL_EXT_texture"))
    {
        spyExtesnionsStr += "GL_EXT_texture ";
    }

    // If the real renderer supports GL_SGIS_texture_lod:
    if (apIsOpenGLExtensionSupported(realRendererExensionsStr.asCharArray(), "GL_SGIS_texture_lod"))
    {
        spyExtesnionsStr += "GL_SGIS_texture_lod ";
    }

    // If the real renderer supports GL_ARB_texture_compression:
    if (apIsOpenGLExtensionSupported(realRendererExensionsStr.asCharArray(), "GL_ARB_texture_compression"))
    {
        spyExtesnionsStr += "GL_ARB_texture_compression ";
    }

    // If the read renderer supports GL_EXT_texture_compression_s3tc:
    if (apIsOpenGLExtensionSupported(realRendererExensionsStr.asCharArray(), "GL_EXT_texture_compression_s3tc"))
    {
        // Add the appropriate OpenGL ES extensions to the output string:
        spyExtesnionsStr += "GL_EXT_texture_compression_s3tc ";
        spyExtesnionsStr += "GL_NV_texture_compression_dxt1 ";
        spyExtesnionsStr += "GL_EXT_texture_compression_dxt1 ";
    }

    // Add Graphic Remedy's OpenGL ES extensions:
    spyExtesnionsStr += GS_STR_GREMEDY_SUPPORTED_EXTENSIONS;
}


// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::getOGLBaseFunctionSpyAddress
// Description: Retrieves the Spy implementation address of an OpenGL base
//              (I.E: not extension) function.
// Arguments: functionName - The queried function name.
// Return Val: osProcedureAddress - Will get the spy implementation address
//                                  of this function, or NULL if it is not
//                                  supported by the spy.
// Author:      Yaki Tebeka
// Date:        15/6/2007
// ---------------------------------------------------------------------------
osProcedureAddress gsExtensionsManager::getOGLBaseFunctionSpyAddress(const gtASCIIString& functionName)
{
    osProcedureAddress retVal = NULL;

    // Get the OpenGL spy type according to configuration:
    suAPIConnector::suSpyType oglSpyType = suAPIConnector::SU_SPY_UNKNOWN;
    /*  #if defined (_GR_OPENGL32) || defined (_GR_OPENGL_MODULE)
            oglSpyType = suAPIConnector::SU_SPY;
        #elif defined(_GR_OPENGLES_COMMON) || defined(_GR_OPENGLES_IPHONE)
            #ifdef _GR_IPHONE_DEVICE_BUILD
                oglSpyType = suAPIConnector::SU_SPY_IPHONE;
            #else
                oglSpyType = suAPIConnector::SU_SPY_ES;
            #endif
        #elif defined(_GR_OPENGLES_COMMON_LITE)
            oglSpyType = suAPIConnector::SU_SPY_ES_LITE;
        #else
            #error Error: Unknown compile target!
        #endif*/
#ifdef _AMDT_OPENGLSERVER_EXPORTS
    oglSpyType = suAPIConnector::SU_SPY;
#else
#error Unknown Server Type!
#endif

    // Get gremedy's OGL server module handle:
    osModuleHandle thisModuleHandle = NULL;
    bool rc1 = suAPIConnector::instance().getServerModuleHandle(oglSpyType, AP_OPENGL_API_CONNECTION, thisModuleHandle);

    if (rc1)
    {
        // Try to get gremedy's OGL server implementation for the input function:
        osProcedureAddress spyImplementationAddress = NULL;
        bool rc2 = osGetProcedureAddress(thisModuleHandle, functionName.asCharArray(), spyImplementationAddress, true);

        if (rc2)
        {
            retVal = spyImplementationAddress;
        }
    }

    return retVal;
}


