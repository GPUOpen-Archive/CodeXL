//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsGLDebugOutputManager.cpp
///
//==================================================================================

//------------------------------ gsGLDebugOutputManager.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAPIClasses/Include/apGLDebugOutput.h>
#include <AMDTAPIClasses/Include/Events/apGLDebugOutputMessageEvent.h>

// Spy utilities:
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>

// Local:
//#include <src/gsExtensionsManager.h>
#include <src/gsForcedModesManager.h>
#include <src/gsGLDebugOutputManager.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsStringConstants.h>
#include <src/gsWrappersCommon.h>

// The maximal amount of debug output printouts:
#define GS_MAX_DEBUG_OUTPUT_PRINTOUTS 500


///////////////////////////////
///////////////////////////////
// gsGLAMDDebugOutputManager //
///////////////////////////////
///////////////////////////////

// ---------------------------------------------------------------------------
// Name:        gsGLAMDDebugOutputManager::gsGLAMDDebugOutputManager
// Description: Constructor
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
gsGLAMDDebugOutputManager::gsGLAMDDebugOutputManager()
    : m_isInitialized(false), m_glDebugMessageEnableAMD(NULL), m_glDebugMessageCallbackAMD(NULL)
{

}

// ---------------------------------------------------------------------------
// Name:        gsGLAMDDebugOutputManager::~gsGLAMDDebugOutputManager
// Description: Destructor
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
gsGLAMDDebugOutputManager::~gsGLAMDDebugOutputManager()
{

}

// ---------------------------------------------------------------------------
// Name:        gsGLAMDDebugOutputManager::initialize
// Description: Called on the first time the context is made current to see if
//              this version of the debug output is supported.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
bool gsGLAMDDebugOutputManager::initialize()
{
    if (!m_isInitialized)
    {
        m_glDebugMessageEnableAMD = (PFNGLDEBUGMESSAGEENABLEAMDPROC)gsGetSystemsOGLModuleProcAddress("glDebugMessageEnableAMD");
        m_glDebugMessageCallbackAMD = (PFNGLDEBUGMESSAGECALLBACKAMDPROC)gsGetSystemsOGLModuleProcAddress("glDebugMessageCallbackAMD");

        m_isInitialized = ((NULL != m_glDebugMessageEnableAMD) && (NULL != m_glDebugMessageCallbackAMD));
    }

    return m_isInitialized;
}

// ---------------------------------------------------------------------------
// Name:        gsGLAMDDebugOutputManager::setCallback
// Description: Wrapper for glDebugMessageCallback
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
bool gsGLAMDDebugOutputManager::setCallback(GLDEBUGPROCAMD callback, GLvoid* userData)
{
    bool retVal = false;

    if (m_isInitialized)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);

        // Clear any previous GL errors:
        GLenum err = gs_stat_realFunctionPointers.glGetError();

        if (GL_NO_ERROR != err)
        {
            gtString errMsg = GS_STR_previousOpenGLError;
            errMsg.appendFormattedString(L"%#x", err);
            GT_ASSERT_EX(GL_NO_ERROR == err, errMsg.asCharArray());
        }

        if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
        {
            gtString logMsg = GS_STR_settingDebugMessageCallback;
            logMsg.appendFormattedString(L"(%p, %p)", (void*)callback, (void*)userData);
            OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
        }

        m_glDebugMessageCallbackAMD(callback, userData);

        err = gs_stat_realFunctionPointers.glGetError();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

        retVal = (GL_NO_ERROR == err);

        if (!retVal)
        {
            gtString errMsg = GS_STR_errorWhileSettingDebugMessageCallback;
            errMsg.appendFormattedString(L"(%p, %p) -> %#x", (void*)callback, (void*)userData, err);
            GT_ASSERT_EX(GL_NO_ERROR == err, errMsg.asCharArray());
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsGLAMDDebugOutputManager::applyMessageFilter
// Description: Wrapper for glDebugMessageControl
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
bool gsGLAMDDebugOutputManager::applyMessageFilter(GLenum category, GLenum severity, bool enable)
{
    bool retVal = false;

    if (m_isInitialized)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);

        // Clear any previous GL errors:
        GLenum err = gs_stat_realFunctionPointers.glGetError();

        if (GL_NO_ERROR != err)
        {
            gtString errMsg = GS_STR_previousOpenGLError;
            errMsg.appendFormattedString(L"%#x", err);
            GT_ASSERT_EX(GL_NO_ERROR == err, errMsg.asCharArray());
        }

        if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
        {
            gtString logMsg = GS_STR_settingDebugMessageFilter;
            logMsg.appendFormattedString(L"(%#x, %#x, %c)", category, severity, enable ? 'T' : 'F');
            OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
        }

        m_glDebugMessageEnableAMD(category, severity, 0, NULL, enable ? GL_TRUE : GL_FALSE);

        err = gs_stat_realFunctionPointers.glGetError();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

        retVal = (GL_NO_ERROR == err);

        if (!retVal)
        {
            gtString errMsg = GS_STR_errorWhileSettingDebugMessageFilter;
            errMsg.appendFormattedString(L"(%#x, %#x, %c) -> %#x", category, severity, enable ? 'T' : 'F', err);
            GT_ASSERT_EX(GL_NO_ERROR == err, errMsg.asCharArray());
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsGLAMDDebugOutputManager::clearMessageFilter
// Description: Sets the entire message filter to true / false
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
bool gsGLAMDDebugOutputManager::clearMessageFilter(bool enable)
{
    bool retVal = applyMessageFilter(GL_DONT_CARE, GL_DONT_CARE, enable);

    return retVal;
}

///////////////////////////////
///////////////////////////////
// gsGLARBDebugOutputManager //
///////////////////////////////
///////////////////////////////

// ---------------------------------------------------------------------------
// Name:        gsGLARBDebugOutputManager::gsGLARBDebugOutputManager
// Description: Constructor
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
gsGLARBDebugOutputManager::gsGLARBDebugOutputManager()
    : m_isInitialized(false), m_glDebugMessageControlARB(NULL), m_glDebugMessageCallbackARB(NULL)
{

}

// ---------------------------------------------------------------------------
// Name:        gsGLARBDebugOutputManager::~gsGLARBDebugOutputManager
// Description: Destructor
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
gsGLARBDebugOutputManager::~gsGLARBDebugOutputManager()
{

}

// ---------------------------------------------------------------------------
// Name:        gsGLARBDebugOutputManager::initialize
// Description: Called on the first time the context is made current to see if
//              this version of the debug output is supported.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
bool gsGLARBDebugOutputManager::initialize()
{
    if (!m_isInitialized)
    {
        m_glDebugMessageControlARB = (PFNGLDEBUGMESSAGECONTROLARBPROC)gsGetSystemsOGLModuleProcAddress("glDebugMessageControlARB");
        m_glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)gsGetSystemsOGLModuleProcAddress("glDebugMessageCallbackARB");

        m_isInitialized = ((NULL != m_glDebugMessageControlARB) && (NULL != m_glDebugMessageCallbackARB));
    }

    return m_isInitialized;
}

// ---------------------------------------------------------------------------
// Name:        gsGLARBDebugOutputManager::setCallback
// Description: Wrapper for glDebugMessageCallback
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
bool gsGLARBDebugOutputManager::setCallback(GLDEBUGPROCARB callback, GLvoid* userData)
{
    bool retVal = false;

    if (m_isInitialized)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);

        // Clear any previous GL errors:
        GLenum err = gs_stat_realFunctionPointers.glGetError();

        if (GL_NO_ERROR != err)
        {
            gtString errMsg = GS_STR_previousOpenGLError;
            errMsg.appendFormattedString(L"%#x", err);
            GT_ASSERT_EX(GL_NO_ERROR == err, errMsg.asCharArray());
        }

        if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
        {
            gtString logMsg = GS_STR_settingDebugMessageCallback;
            logMsg.appendFormattedString(L"(%p, %p)", (void*)callback, (void*)userData);
            OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
        }

        m_glDebugMessageCallbackARB(callback, userData);

        err = gs_stat_realFunctionPointers.glGetError();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

        retVal = (GL_NO_ERROR == err);

        if (!retVal)
        {
            gtString errMsg = GS_STR_errorWhileSettingDebugMessageCallback;
            errMsg.appendFormattedString(L"(%p, %p) -> %#x", (void*)callback, (void*)userData, err);
            GT_ASSERT_EX(GL_NO_ERROR == err, errMsg.asCharArray());
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsGLARBDebugOutputManager::applyMessageFilter
// Description: Wrapper for glDebugMessageControl
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
bool gsGLARBDebugOutputManager::applyMessageFilter(GLenum source, GLenum type, GLenum severity, bool enable)
{
    bool retVal = false;

    if (m_isInitialized)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);

        // Clear any previous GL errors:
        GLenum err = gs_stat_realFunctionPointers.glGetError();

        if (GL_NO_ERROR != err)
        {
            gtString errMsg = GS_STR_previousOpenGLError;
            errMsg.appendFormattedString(L"%#x", err);
            GT_ASSERT_EX(GL_NO_ERROR == err, errMsg.asCharArray());
        }

        if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
        {
            gtString logMsg = GS_STR_settingDebugMessageFilter;
            logMsg.appendFormattedString(L"(%#x, %#x, %#x, %c)", source, type, severity, enable ? 'T' : 'F');
            OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
        }

        m_glDebugMessageControlARB(source, type, severity, 0, NULL, enable ? GL_TRUE : GL_FALSE);

        err = gs_stat_realFunctionPointers.glGetError();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

        retVal = (GL_NO_ERROR == err);

        if (!retVal)
        {
            gtString errMsg = GS_STR_errorWhileSettingDebugMessageFilter;
            errMsg.appendFormattedString(L"(%#x, %#x, %#x, %c) -> %#x", source, type, severity, enable ? 'T' : 'F', err);
            GT_ASSERT_EX(GL_NO_ERROR == err, errMsg.asCharArray());
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsGLARBDebugOutputManager::clearMessageFilter
// Description: Sets the entire message filter to true / false
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
bool gsGLARBDebugOutputManager::clearMessageFilter(bool enable)
{
    bool retVal = applyMessageFilter(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, enable);

    return retVal;
}

//////////////////////////////
//////////////////////////////
// gsGL43DebugOutputManager //
//////////////////////////////
//////////////////////////////

// ---------------------------------------------------------------------------
// Name:        gsGL43DebugOutputManager::gsGL43DebugOutputManager
// Description: Constructor
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
gsGL43DebugOutputManager::gsGL43DebugOutputManager()
    : m_isInitialized(false), m_glDebugMessageControl(NULL), m_glDebugMessageCallback(NULL)
{

}

// ---------------------------------------------------------------------------
// Name:        gsGL43DebugOutputManager::~gsGL43DebugOutputManager
// Description: Destructor
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
gsGL43DebugOutputManager::~gsGL43DebugOutputManager()
{

}

// ---------------------------------------------------------------------------
// Name:        gsGL43DebugOutputManager::initialize
// Description: Called on the first time the context is made current to see if
//              this version of the debug output is supported.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
bool gsGL43DebugOutputManager::initialize()
{
    if (!m_isInitialized)
    {
        m_glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC)gsGetSystemsOGLModuleProcAddress("glDebugMessageControl");
        m_glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)gsGetSystemsOGLModuleProcAddress("glDebugMessageCallback");

        m_isInitialized = ((NULL != m_glDebugMessageControl) && (NULL != m_glDebugMessageCallback));
    }

    return m_isInitialized;
}

// ---------------------------------------------------------------------------
// Name:        gsGL43DebugOutputManager::setCallback
// Description: Wrapper for glDebugMessageCallback
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
bool gsGL43DebugOutputManager::setCallback(GLDEBUGPROC callback, const void* userData)
{
    bool retVal = false;

    if (m_isInitialized)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);

        // Clear any previous GL errors:
        GLenum err = gs_stat_realFunctionPointers.glGetError();

        if (GL_NO_ERROR != err)
        {
            gtString errMsg = GS_STR_previousOpenGLError;
            errMsg.appendFormattedString(L"%#x", err);
            GT_ASSERT_EX(GL_NO_ERROR == err, errMsg.asCharArray());
        }

        if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
        {
            gtString logMsg = GS_STR_settingDebugMessageCallback;
            logMsg.appendFormattedString(L"(%p, %p)", (void*)callback, (void*)userData);
            OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
        }

        m_glDebugMessageCallback(callback, userData);

        err = gs_stat_realFunctionPointers.glGetError();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

        retVal = (GL_NO_ERROR == err);

        if (!retVal)
        {
            gtString errMsg = GS_STR_errorWhileSettingDebugMessageCallback;
            errMsg.appendFormattedString(L"(%p, %p) -> %#x", (void*)callback, (void*)userData, err);
            GT_ASSERT_EX(GL_NO_ERROR == err, errMsg.asCharArray());
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsGL43DebugOutputManager::applyMessageFilter
// Description: Wrapper for glDebugMessageControl
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
bool gsGL43DebugOutputManager::applyMessageFilter(GLenum source, GLenum type, GLenum severity, bool enable)
{
    bool retVal = false;

    if (m_isInitialized)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);

        // Clear any previous GL errors:
        GLenum err = gs_stat_realFunctionPointers.glGetError();

        if (GL_NO_ERROR != err)
        {
            gtString errMsg = GS_STR_previousOpenGLError;
            errMsg.appendFormattedString(L"%#x", err);
            GT_ASSERT_EX(GL_NO_ERROR == err, errMsg.asCharArray());
        }

        if (OS_DEBUG_LOG_DEBUG <= osDebugLog::instance().loggedSeverity())
        {
            gtString logMsg = GS_STR_settingDebugMessageFilter;
            logMsg.appendFormattedString(L"(%#x, %#x, %#x, %c)", source, type, severity, enable ? 'T' : 'F');
            OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
        }

        m_glDebugMessageControl(source, type, severity, 0, NULL, enable ? GL_TRUE : GL_FALSE);

        err = gs_stat_realFunctionPointers.glGetError();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

        retVal = (GL_NO_ERROR == err);

        if (!retVal)
        {
            gtString errMsg = GS_STR_errorWhileSettingDebugMessageFilter;
            errMsg.appendFormattedString(L"(%#x, %#x, %#x, %c) -> %#x", source, type, severity, enable ? 'T' : 'F', err);
            GT_ASSERT_EX(GL_NO_ERROR == err, errMsg.asCharArray());
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsGL43DebugOutputManager::clearMessageFilter
// Description: Sets the entire message filter to true / false
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
bool gsGL43DebugOutputManager::clearMessageFilter(bool enable)
{
    bool retVal = applyMessageFilter(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, enable);

    return retVal;
}



////////////////////////////
////////////////////////////
// gsGLDebugOutputManager //
////////////////////////////
////////////////////////////

// ---------------------------------------------------------------------------
// Name:        gsGLDebugOutputManager::gsGLDebugOutputManager
// Description: Constructor
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
gsGLDebugOutputManager::gsGLDebugOutputManager(int contextId, gsForcedModesManager& forcedModesManager)
    : m_contextId(contextId), m_currentContextDebugOutputExtensionSupportLevel(GS_NO_DEBUG_OUTPUT_SUPPORT), m_amoutOfGLDebugOutputPrintouts(0), m_forcedModesManager(forcedModesManager)
{

}

// ---------------------------------------------------------------------------
// Name:        gsGLDebugOutputManager::~gsGLDebugOutputManager
// Description: Destructor
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
gsGLDebugOutputManager::~gsGLDebugOutputManager()
{

}

// ---------------------------------------------------------------------------
// Name:        gsGLDebugOutputManager::onFirstTimeContextMadeCurrent
// Description: Called on the first time my controlling context becomes current.
//              Determines the context's debug output support level.
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
void gsGLDebugOutputManager::onFirstTimeContextMadeCurrent()
{
    bool is43Supported = m_43OutputManager.initialize();
    bool isARBSupported = m_arbOutputManager.initialize();
    bool isAMDSupported = m_amdOutputManager.initialize();

    if (is43Supported)
    {
        m_currentContextDebugOutputExtensionSupportLevel = GS_OPENGL_43_OR_KHR_EXTENSION_SUPPORTED;
    }
    else if (isARBSupported)
    {
        m_currentContextDebugOutputExtensionSupportLevel = GS_ARB_EXTENSION_SUPPORTED;
    }
    else if (isAMDSupported)
    {
        m_currentContextDebugOutputExtensionSupportLevel = GS_AMD_EXTENSION_SUPPORTED;
    }
    else
    {
        m_currentContextDebugOutputExtensionSupportLevel = GS_NO_DEBUG_OUTPUT_SUPPORT;
    }

    applyDebugOutputSettingsToContext();
}

// ---------------------------------------------------------------------------
// Name:        gsGLDebugOutputManager::applyDebugOutputSettingsToContext
// Description: Applies the forced modes manager's settings to this context.
//              This function assumes the controlling context is current.
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
void gsGLDebugOutputManager::applyDebugOutputSettingsToContext()
{
    // If this context supports debug output at all:
    if (GS_NO_DEBUG_OUTPUT_SUPPORT != m_currentContextDebugOutputExtensionSupportLevel)
    {
        // Get the output parameters:
        bool isLoggingEnabled = m_forcedModesManager.isDebugOutputLoggingEnabled();
        const bool* loggedSeverities = m_forcedModesManager.debugOutputLoggedSeverities();
        const bool* loggedMessageKinds = m_forcedModesManager.debugOutputLoggedMessageKinds();

        switch (m_currentContextDebugOutputExtensionSupportLevel)
        {
            case GS_OPENGL_43_OR_KHR_EXTENSION_SUPPORTED:
            {
                // Clear all message reporting:
                bool rcClr = m_43OutputManager.clearMessageFilter(false);
                GT_ASSERT(rcClr);

                if (isLoggingEnabled)
                {
                    // Set our callback data:
                    bool rcCB = m_43OutputManager.setCallback((GLDEBUGPROC)&gsGL43DebugOutputCallbackFunction, (GLvoid*)this);
                    GT_ASSERT(rcCB);

                    // Set the message filter combinations:
                    for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES; i++)
                    {
                        // Is this severity logged?
                        GLenum currentSeverity = apDebugOutputSeverityToGL43Enum((apGLDebugOutputSeverity)i);

                        if (loggedSeverities[i])
                        {
                            // For each source / type combination:
                            for (int j = 0; j < AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES; j++)
                            {
                                GLenum currentSource = apDebugOutputSourceToGL43Enum((apGLDebugOutputSource)j);

                                for (int k = 0; k < AP_NUMBER_OF_DEBUG_OUTPUT_TYPES; k++)
                                {
                                    // If it is chosen:
                                    if (apGLDebugOutputKindFromFlagArray(loggedMessageKinds, (apGLDebugOutputSource)j, (apGLDebugOutputType)k))
                                    {
                                        // Enable this specific combination:
                                        GLenum currentType = apDebugOutputTypeToGL43Enum((apGLDebugOutputType)k);
                                        bool rcMsg = m_43OutputManager.applyMessageFilter(currentSource, currentType, currentSeverity, true);
                                        GT_ASSERT(rcMsg);
                                    }
                                }
                            }
                        }
                        else // !loggedSeverities[i]
                        {
                            // No need to un-set it, since we cleared the whole field to begin:
                            /*
                            bool rcSev = m_43OutputManager.applyMessageFilter(GL_DONT_CARE, GL_DONT_CARE, currentSeverity, false);
                            GT_ASSERT(rcSev);
                            */
                        }
                    }
                }
                else // !isLoggingEnabled
                {
                    // Set the logging callback from the user:
                    bool rcUser = m_43OutputManager.setCallback(m_43UserCallbackDetails.m_userCallback, m_43UserCallbackDetails.m_userData);
                    GT_ASSERT(rcUser);
                }
            }
            break;

            case GS_ARB_EXTENSION_SUPPORTED:
            {
                // Clear all message reporting:
                bool rcClr = m_arbOutputManager.clearMessageFilter(false);
                GT_ASSERT(rcClr);

                if (isLoggingEnabled)
                {
                    // Set our callback data:
                    bool rcCB = m_arbOutputManager.setCallback((GLDEBUGPROCARB)&gsGLARBDebugOutputCallbackFunction, (GLvoid*)this);
                    GT_ASSERT(rcCB);

                    // Set the message filter combinations:
                    for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES; i++)
                    {
                        // Is this severity logged?
                        GLenum currentSeverity = apDebugOutputSeverityToGLARBEnum((apGLDebugOutputSeverity)i);

                        if (loggedSeverities[i])
                        {
                            // For each source / type combination:
                            for (int j = 0; j < AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES; j++)
                            {
                                GLenum currentSource = apDebugOutputSourceToGLARBEnum((apGLDebugOutputSource)j);

                                for (int k = 0; k < AP_NUMBER_OF_DEBUG_OUTPUT_TYPES; k++)
                                {
                                    // If it is chosen:
                                    if ((apGLDebugOutputKindFromFlagArray(loggedMessageKinds, (apGLDebugOutputSource)j, (apGLDebugOutputType)k)))
                                    {
                                        // Enable this specific combination:
                                        GLenum currentType = apDebugOutputTypeToGLARBEnum((apGLDebugOutputType)k);
                                        bool rcMsg = m_arbOutputManager.applyMessageFilter(currentSource, currentType, currentSeverity, true);
                                        GT_ASSERT(rcMsg);
                                    }
                                }
                            }
                        }
                        else // !loggedSeverities[i]
                        {
                            // No need to un-set it, since we cleared the whole field to begin:
                            /*
                            bool rcSev = m_arbOutputManager.applyMessageFilter(GL_DONT_CARE, GL_DONT_CARE, currentSeverity, false);
                            GT_ASSERT(rcSev);
                            */
                        }
                    }
                }
                else // !isLoggingEnabled
                {
                    // Set the logging callback from the user:
                    bool rcUser = m_arbOutputManager.setCallback(m_arbUserCallbackDetails.m_userCallback, m_arbUserCallbackDetails.m_userData);
                    GT_ASSERT(rcUser);
                }
            }
            break;

            case GS_AMD_EXTENSION_SUPPORTED:
            {
                // Clear all message reporting:
                bool rcClr = m_amdOutputManager.clearMessageFilter(false);
                GT_ASSERT(rcClr);

                if (isLoggingEnabled)
                {
                    // Set our callback data:
                    bool rcCB = m_amdOutputManager.setCallback((GLDEBUGPROCAMD)&gsGLAMDDebugOutputCallbackFunction, (GLvoid*)this);
                    GT_ASSERT(rcCB);

                    // Set the message filter combinations:
                    for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES; i++)
                    {
                        // Is this severity logged?
                        GLenum currentSeverity = apDebugOutputSeverityToGLAMDEnum((apGLDebugOutputSeverity)i);

                        if (loggedSeverities[i])
                        {
                            // For each source / type combination:
                            for (int j = 0; j < AP_NUMBER_OF_DEBUG_OUTPUT_CATEGORIES; j++)
                            {
                                // If it is chosen:
                                if (apIsDebugOutputCategoryFlagged(loggedMessageKinds, (apGLDebugOutputCategory)j))
                                {
                                    // Enable this specific combination:
                                    GLenum currentCategory = apDebugOutputCategoryToGLAMDEnum((apGLDebugOutputCategory)j);
                                    bool rcMsg = m_amdOutputManager.applyMessageFilter(currentCategory, currentSeverity, true);
                                    GT_ASSERT(rcMsg);
                                }
                            }
                        }
                        else // !loggedSeverities[i]
                        {
                            // No need to un-set it, since we cleared the whole field to begin:
                            /*
                            bool rcSev = m_amdOutputManager.applyMessageFilter(GL_DONT_CARE, GL_DONT_CARE, currentSeverity, false);
                            GT_ASSERT(rcSev);
                            */
                        }
                    }
                }
                else // !isLoggingEnabled
                {
                    // Set the logging callback from the user:
                    bool rcUser = m_amdOutputManager.setCallback(m_amdUserCallbackDetails.m_userCallback, m_amdUserCallbackDetails.m_userData);
                    GT_ASSERT(rcUser);
                }
            }
            break;

            case GS_NO_DEBUG_OUTPUT_SUPPORT:
            default:
                // Unexpected value!
                GT_ASSERT(false);
                break;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsGLDebugOutputManager::gsGL43DebugOutputCallbackFunction
// Description: Callback function used for OpenGL 4.3 / KHR version of the debug output
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
void APIENTRY gsGLDebugOutputManager::gsGL43DebugOutputCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
    gsGLDebugOutputManager* pDebugOutputManager = (gsGLDebugOutputManager*)userParam;
    GT_IF_WITH_ASSERT(NULL != pDebugOutputManager)
    {
        apGLDebugOutputSource apSource = apDebugOutputSourceFromGL43Enum(source);
        apGLDebugOutputType apType = apDebugOutputTypeFromGL43Enum(type);
        apGLDebugOutputSeverity apSeverity = apDebugOutputSeverityFromGL43Enum(severity);
        pDebugOutputManager->onMessageCallback(apSource, apType, id, apSeverity, length, message);

        // Call the user callback:
        const gs43UserCallbackDetails& crCallback = pDebugOutputManager->get43CallbackDetails();

        if (NULL != crCallback.m_userCallback)
        {
            crCallback.m_userCallback(source, type, id, severity, length, message, crCallback.m_userData);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsGLDebugOutputManager::gsGLARBDebugOutputCallbackFunction
// Description: Callback function used for ARB version of the debug output
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
void APIENTRY gsGLDebugOutputManager::gsGLARBDebugOutputCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
    gsGLDebugOutputManager* pDebugOutputManager = (gsGLDebugOutputManager*)userParam;
    GT_IF_WITH_ASSERT(NULL != pDebugOutputManager)
    {
        apGLDebugOutputSource apSource = apDebugOutputSourceFromGLARBEnum(source);
        apGLDebugOutputType apType = apDebugOutputTypeFromGLARBEnum(type);
        apGLDebugOutputSeverity apSeverity = apDebugOutputSeverityFromGLARBEnum(severity);
        pDebugOutputManager->onMessageCallback(apSource, apType, id, apSeverity, length, message);

        // Call the user callback:
        const gsARBUserCallbackDetails& crCallback = pDebugOutputManager->getARBCallbackDetails();

        if (NULL != crCallback.m_userCallback)
        {
            crCallback.m_userCallback(source, type, id, severity, length, message, crCallback.m_userData);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsGLDebugOutputManager::gsGLAMDDebugOutputCallbackFunction
// Description: Callback function used for AMD version of the debug output
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
void APIENTRY gsGLDebugOutputManager::gsGLAMDDebugOutputCallbackFunction(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
    gsGLDebugOutputManager* pDebugOutputManager = (gsGLDebugOutputManager*)userParam;
    GT_IF_WITH_ASSERT(NULL != pDebugOutputManager)
    {
        GLenum source = GL_DEBUG_SOURCE_OTHER;
        GLenum type = GL_DEBUG_TYPE_OTHER;

        // Translate the AMD extension category to ARB/KHR/4.3 extension source + type:
        switch (category)
        {
            case GL_DEBUG_CATEGORY_API_ERROR_AMD:
                source = GL_DEBUG_SOURCE_API;
                type = GL_DEBUG_TYPE_ERROR;
                break;

            case GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD:
                source = GL_DEBUG_SOURCE_WINDOW_SYSTEM;
                type = GL_DONT_CARE;
                break;

            case GL_DEBUG_CATEGORY_DEPRECATION_AMD:
                source = GL_DONT_CARE;
                type = GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR;
                break;

            case GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD:
                source = GL_DONT_CARE;
                type = GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR;
                break;

            case GL_DEBUG_CATEGORY_PERFORMANCE_AMD:
                source = GL_DONT_CARE;
                type = GL_DEBUG_TYPE_PERFORMANCE;
                break;

            case GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD:
                source = GL_DEBUG_SOURCE_SHADER_COMPILER;
                type = GL_DONT_CARE;
                break;

            case GL_DEBUG_CATEGORY_APPLICATION_AMD:
                source = GL_DEBUG_SOURCE_APPLICATION;
                type = GL_DONT_CARE;
                break;

            case GL_DEBUG_CATEGORY_OTHER_AMD:
                source = GL_DEBUG_SOURCE_OTHER;
                type = GL_DEBUG_TYPE_OTHER;
                break;

            default:
                // Unexpected value!
                GT_ASSERT(false);
                break;
        }

        apGLDebugOutputSource apSource = (GL_DONT_CARE == source) ? AP_GL_DEBUG_OUTPUT_SOURCE_OTHER : apDebugOutputSourceFromGL43Enum(source);
        apGLDebugOutputType apType = (GL_DONT_CARE == type) ? AP_GL_DEBUG_OUTPUT_TYPE_OTHER : apDebugOutputTypeFromGL43Enum(type);
        apGLDebugOutputSeverity apSeverity = apDebugOutputSeverityFromGLAMDEnum(severity);

        pDebugOutputManager->onMessageCallback(apSource, apType, id, apSeverity, length, message);

        // Call the user callback:
        const gsAMDUserCallbackDetails& crCallback = pDebugOutputManager->getAMDCallbackDetails();

        if (NULL != crCallback.m_userCallback)
        {
            crCallback.m_userCallback(id, category, severity, length, message, crCallback.m_userData);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsGLDebugOutputManager::onMessageCallback
// Description: Checks if an incoming message passes the CodeXL filter, and reports
//              it to the client if it does.
// Author:      Uri Shomroni
// Date:        26/6/2014
// ---------------------------------------------------------------------------
void gsGLDebugOutputManager::onMessageCallback(apGLDebugOutputSource source, apGLDebugOutputType type, GLuint id, apGLDebugOutputSeverity severity, GLsizei length, const GLchar* message)
{
    // Only if the debug output is enabled:
    if (m_forcedModesManager.isDebugOutputLoggingEnabled())
    {
        // Increment the amount of debug output printouts reported so far:
        m_amoutOfGLDebugOutputPrintouts++;

        gtString messageStr;

        if (GS_MAX_DEBUG_OUTPUT_PRINTOUTS <= m_amoutOfGLDebugOutputPrintouts)
        {
            // If we just reached the maximal amount of debug output printouts:
            if (m_amoutOfGLDebugOutputPrintouts == GS_MAX_DEBUG_OUTPUT_PRINTOUTS)
            {
                // Tell the user that we reached the maximal debug output printouts:
                source = AP_GL_DEBUG_OUTPUT_SOURCE_THIRD_PARTY;
                static gtString maxReportsMsg;
                maxReportsMsg.appendFormattedString(GS_STR_GLDebugOutputMaxPrintoutsReached, GS_MAX_DEBUG_OUTPUT_PRINTOUTS);
                messageStr = maxReportsMsg;
            }

            // Prevent _amoutOfGLDebugOutputPrintouts overflow:
            m_amoutOfGLDebugOutputPrintouts = GS_MAX_DEBUG_OUTPUT_PRINTOUTS + 2;

            // Disable the debug output mechanism:
            m_forcedModesManager.setGLDebugOutputLoggingEnabled(false);

            applyDebugOutputSettingsToContext();
        }
        else
        {
            messageStr.fromASCIIString(message, length < 2048 ? length : 2048);
        }

        // Notify the debugger about the debug message:
        // Translate the severity into a string:
        gtString severityAsString;
        bool rc = apGLDebugOutputSeverityAsString(severity, severityAsString);
        GT_ASSERT(rc);

        // Translate the source into a string:
        gtString sourceAsString;
        rc = apGLDebugOutputSourceAsString(source, sourceAsString);
        GT_ASSERT(rc);

        // Translate the type into a string:
        gtString typeAsString;
        rc = apGLDebugOutputTypeAsString(type, typeAsString);
        GT_ASSERT(rc);

        // Write the event to the events socket:
        osThreadId currentThreadId = osGetCurrentThreadId();
        apGLDebugOutputMessageEvent glDebugOutputMessageEvent(currentThreadId, sourceAsString, typeAsString, severityAsString, messageStr, m_contextId, id);
        bool rcEve = suForwardEventToClient(glDebugOutputMessageEvent);
        GT_ASSERT(rcEve);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

        // If we were asked to break on debug output messages:
        if (suBreakpointsManager::instance().breakOnGenericBreakpoint(AP_BREAK_ON_DEBUG_OUTPUT))
        {
            // If this is an OpenGL ES spy dll build:
#if (defined (OS_OGL_ES_IMPLEMENTATION_DLL_BUILD) || defined (_GR_OPENGLES_IPHONE))
            // There is no immediate mode in OpenGL ES:
            bool isInGLBeginEndBlock = false;
#else
            // We don't know our status here, so we mark true to be on the safe side:
            bool isInGLBeginEndBlock = true;
#endif

            // Trigger a breakpoint exception:
            gsOpenGLMonitor& theOGLMtr = gsOpenGLMonitor::instance();
            theOGLMtr.triggerBreakpointException(AP_GLDEBUG_OUTPUT_REPORT_BREAKPOINT_HIT, GL_NO_ERROR, isInGLBeginEndBlock);
        }

#endif // (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    }
}



