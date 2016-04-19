//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsMacOSXInterception.cpp
///
//==================================================================================

//------------------------------ gsMacOSXInterception.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>

// Spies utilities:
#include <AMDTServerUtilities/Include/suMacOSXInterception.h>

// Local:
#include <src/gsExtensionsManager.h>
#include <src/gsMacOSXInterception.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsGlobalVariables.h>

#ifdef _GR_IPHONE_BUILD
    #include <src/gsEAGLWrappers.h>
#endif

// ---------------------------------------------------------------------------
// Name:        gsInitializeMacOSXOpenGLInterception
// Description: Initializes the interception of the OpenGL function indicated by funcId
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        18/1/2009
// ---------------------------------------------------------------------------
bool gsInitializeMacOSXOpenGLInterception(apMonitoredFunctionId funcId)
{
    bool retVal = false;

    // Sanity check
    GT_IF_WITH_ASSERT((funcId < apFirstOpenCLFunctionIndex) && (funcId >= 0))
    {
        // We use the function name for getting the wrapper function address, as well as for debug log printouts
        gtString funcName = apMonitoredFunctionsManager::instance().monitoredFunctionName(funcId);

        // Get the real and wrapper addresses:
        osProcedureAddress pWrapperFunction = gs_stat_extensionsManager.wrapperFunctionAddress(funcName);

        // We need to get the real pointer second, since calling the above function also initializes the address
        // for extension functions.
        osProcedureAddress pRealFunction = ((osProcedureAddress*)(&gs_stat_realFunctionPointers))[funcId];

        // Will get the new function start address if needed:
        osProcedureAddress pNewRealFunction = pRealFunction;

        // Call the GRSpiesUtilities to intercept the function:
        retVal = suInitializeMacOSXInterception(funcId, funcName, pRealFunction, pWrapperFunction, pNewRealFunction);

        // If the interception created a new function base pointer, set it into the struct:
        if (pRealFunction != pNewRealFunction)
        {
            ((void**)&gs_stat_realFunctionPointers)[funcId] = pNewRealFunction;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsInitializeMacOSXOpenGLInterception
// Description: Initializes the Interception for all monitored OpenGL functions.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        22/1/2009
// ---------------------------------------------------------------------------
bool gsInitializeMacOSXOpenGLInterception()
{
    bool retVal = true;

    // Iterate all the OpenGLfunctions by their index:
    for (unsigned int i = 0; i < apFirstOpenCLFunctionIndex; i++)
    {
        bool functionOk = gsInitializeMacOSXOpenGLInterception((apMonitoredFunctionId)i);
        retVal = retVal && functionOk;
    }

    GT_RETURN_WITH_ASSERT(retVal);
}

// ---------------------------------------------------------------------------
// Name:        gsInitializeEAGLInterception
// Description: Initializes the EAGL wrappers
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        21/5/2009
// ---------------------------------------------------------------------------
bool gsInitializeEAGLInterception()
{
    bool retVal = false;

#ifndef _GR_IPHONE_BUILD
    // Nothing to do
    retVal = true;
#else
    bool rcInitEAGL = gsInitializeEAGLWrappers();
    GT_IF_WITH_ASSERT(rcInitEAGL)
    {
        void* pEAGLContext_initWithAPI = NULL;
        void* pgsEAGLContext_initWithAPI = NULL;
        void* pEAGLContext_initWithAPI_sharegroup = NULL;
        void* pgsEAGLContext_initWithAPI_sharegroup = NULL;
        void* pEAGLContext_dealloc = NULL;
        void* pgsEAGLContext_dealloc = NULL;
        void* pEAGLContext_setCurrentContext = NULL;
        void* pgsEAGLContext_setCurrentContext = NULL;
        void* pEAGLContext_currentContext = NULL;
        void* pgsEAGLContext_currentContext = NULL;
        void* pEAGLContext_API = NULL;
        void* pgsEAGLContext_API = NULL;
        void* pEAGLContext_sharegroup = NULL;
        void* pgsEAGLContext_sharegroup = NULL;
        void* pEAGLContext_renderbufferStorage_fromDrawable = NULL;
        void* pgsEAGLContext_renderbufferStorage_fromDrawable = NULL;
        void* pEAGLContext_presentRenderbuffer = NULL;
        void* pgsEAGLContext_presentRenderbuffer = NULL;

        gsGetEAGLFunctionPointers(pEAGLContext_initWithAPI, pEAGLContext_initWithAPI_sharegroup, pEAGLContext_dealloc, pEAGLContext_setCurrentContext,
                                  pEAGLContext_currentContext, pEAGLContext_API, pEAGLContext_sharegroup, pEAGLContext_renderbufferStorage_fromDrawable, pEAGLContext_presentRenderbuffer);

        gsGetEAGLWrapperFunctionPointers(pgsEAGLContext_initWithAPI, pgsEAGLContext_initWithAPI_sharegroup, pgsEAGLContext_dealloc, pgsEAGLContext_setCurrentContext,
                                         pgsEAGLContext_currentContext, pgsEAGLContext_API, pgsEAGLContext_sharegroup, pgsEAGLContext_renderbufferStorage_fromDrawable, pgsEAGLContext_presentRenderbuffer);

        // Store the real pointers in the gs_stat_realFunctionPointers vector so that the GS_START/END_FUNCTION_WRAPPER (etc) macros will work:
        ((void**)&gs_stat_realFunctionPointers)[ap_EAGLContext_initWithAPI] = pEAGLContext_initWithAPI;
        ((void**)&gs_stat_realFunctionPointers)[ap_EAGLContext_initWithAPI_sharegroup] = pEAGLContext_initWithAPI_sharegroup;
        ((void**)&gs_stat_realFunctionPointers)[ap_EAGLContext_dealloc] = pEAGLContext_dealloc;
        ((void**)&gs_stat_realFunctionPointers)[ap_EAGLContext_setCurrentContext] = pEAGLContext_setCurrentContext;
        ((void**)&gs_stat_realFunctionPointers)[ap_EAGLContext_currentContext] = pEAGLContext_currentContext;
        ((void**)&gs_stat_realFunctionPointers)[ap_EAGLContext_API] = pEAGLContext_API;
        ((void**)&gs_stat_realFunctionPointers)[ap_EAGLContext_sharegroup] = pEAGLContext_sharegroup;
        ((void**)&gs_stat_realFunctionPointers)[ap_EAGLContext_renderbufferStorage_fromDrawable] = pEAGLContext_renderbufferStorage_fromDrawable;
        ((void**)&gs_stat_realFunctionPointers)[ap_EAGLContext_presentRenderbuffer] = pEAGLContext_presentRenderbuffer;

        void* pNewEAGLContext_initWithAPI = pEAGLContext_initWithAPI;
        void* pNewEAGLContext_initWithAPI_sharegroup = pEAGLContext_initWithAPI_sharegroup;
        void* pNewEAGLContext_dealloc = pEAGLContext_dealloc;
        void* pNewEAGLContext_setCurrentContext = pEAGLContext_setCurrentContext;
        void* pNewEAGLContext_currentContext = pEAGLContext_currentContext;
        void* pNewEAGLContext_API = pEAGLContext_API;
        void* pNewEAGLContext_sharegroup = pEAGLContext_sharegroup;
        void* pNewEAGLContext_renderbufferStorage_fromDrawable = pEAGLContext_renderbufferStorage_fromDrawable;
        void* pNewEAGLContext_presentRenderbuffer = pEAGLContext_presentRenderbuffer;

        bool rc1 = suInitializeMacOSXInterception(ap_EAGLContext_initWithAPI, "-[EAGLContext initWithAPI:]", pEAGLContext_initWithAPI, pgsEAGLContext_initWithAPI, pNewEAGLContext_initWithAPI);
        bool rc2 = suInitializeMacOSXInterception(ap_EAGLContext_initWithAPI_sharegroup, "-[EAGLContext initWithAPI: sharegroup:]", pEAGLContext_initWithAPI_sharegroup, pgsEAGLContext_initWithAPI_sharegroup, pNewEAGLContext_initWithAPI_sharegroup);
        bool rc3 = suInitializeMacOSXInterception(ap_EAGLContext_dealloc, "-[EAGLContext dealloc]", pEAGLContext_dealloc, pgsEAGLContext_dealloc, pNewEAGLContext_dealloc);
        bool rc4 = suInitializeMacOSXInterception(ap_EAGLContext_setCurrentContext, "+[EAGLContext setCurrentContext:]", pEAGLContext_setCurrentContext, pgsEAGLContext_setCurrentContext, pNewEAGLContext_setCurrentContext);
        bool rc5 = suInitializeMacOSXInterception(ap_EAGLContext_currentContext, "+[EAGLContext currentContext]", pEAGLContext_currentContext, pgsEAGLContext_currentContext, pNewEAGLContext_currentContext);
        bool rc6 = suInitializeMacOSXInterception(ap_EAGLContext_API, "-[EAGLContext API]", pEAGLContext_API, pgsEAGLContext_API, pNewEAGLContext_API);
        bool rc7 = suInitializeMacOSXInterception(ap_EAGLContext_sharegroup, "-[EAGLContext sharegroup]", pEAGLContext_sharegroup, pgsEAGLContext_sharegroup, pNewEAGLContext_sharegroup);
        bool rc8 = suInitializeMacOSXInterception(ap_EAGLContext_renderbufferStorage_fromDrawable, "-[EAGLContext renderbufferStorage: fromDrawable:]", pEAGLContext_renderbufferStorage_fromDrawable, pgsEAGLContext_renderbufferStorage_fromDrawable, pNewEAGLContext_renderbufferStorage_fromDrawable);
        bool rc9 = suInitializeMacOSXInterception(ap_EAGLContext_presentRenderbuffer, "-[EAGLContext presentRenderbuffer:]", pEAGLContext_presentRenderbuffer, pgsEAGLContext_presentRenderbuffer, pNewEAGLContext_presentRenderbuffer);

        if (pNewEAGLContext_initWithAPI != pEAGLContext_initWithAPI)
        {((void**)&gs_stat_realFunctionPointers)[ap_EAGLContext_initWithAPI] = pNewEAGLContext_initWithAPI;}

        if (pNewEAGLContext_initWithAPI_sharegroup != pEAGLContext_initWithAPI_sharegroup)
        {((void**)&gs_stat_realFunctionPointers)[ap_EAGLContext_initWithAPI_sharegroup] = pNewEAGLContext_initWithAPI_sharegroup;}

        if (pNewEAGLContext_dealloc != pEAGLContext_dealloc)
        {((void**)&gs_stat_realFunctionPointers)[ap_EAGLContext_dealloc] = pNewEAGLContext_dealloc;}

        if (pNewEAGLContext_setCurrentContext != pEAGLContext_setCurrentContext)
        {((void**)&gs_stat_realFunctionPointers)[ap_EAGLContext_setCurrentContext] = pNewEAGLContext_setCurrentContext;}

        if (pNewEAGLContext_currentContext != pEAGLContext_currentContext)
        {((void**)&gs_stat_realFunctionPointers)[ap_EAGLContext_currentContext] = pNewEAGLContext_currentContext;}

        if (pNewEAGLContext_API != pEAGLContext_API)
        {((void**)&gs_stat_realFunctionPointers)[ap_EAGLContext_API] = pNewEAGLContext_API;}

        if (pNewEAGLContext_sharegroup != pEAGLContext_sharegroup)
        {((void**)&gs_stat_realFunctionPointers)[ap_EAGLContext_sharegroup] = pNewEAGLContext_sharegroup;}

        if (pNewEAGLContext_renderbufferStorage_fromDrawable != pEAGLContext_renderbufferStorage_fromDrawable)
        {((void**)&gs_stat_realFunctionPointers)[ap_EAGLContext_renderbufferStorage_fromDrawable] = pNewEAGLContext_renderbufferStorage_fromDrawable;}

        if (pNewEAGLContext_presentRenderbuffer != pEAGLContext_presentRenderbuffer)
        {((void**)&gs_stat_realFunctionPointers)[ap_EAGLContext_presentRenderbuffer] = pNewEAGLContext_presentRenderbuffer;}
    }
#endif

    return retVal;
}
