//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsGLDebugOutputManager.h
///
//==================================================================================

//------------------------------ gsGLDebugOutputManager.h ------------------------------

#ifndef __GSGLDEBUGOUTPUTMANAGER_H
#define __GSGLDEBUGOUTPUTMANAGER_H

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))

// Forward declarations:
class gsForcedModesManager;

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTAPIClasses/Include/apGLDebugOutput.h>

// ----------------------------------------------------------------------------------
// Class Name:          gsGLDebugOutputManager
// General Description: Manages the integration with debug output extensions
//
// Author:              Sigal Algranaty
// Creation Date:       7/6/2010
// ----------------------------------------------------------------------------------
class gsGLAMDDebugOutputManager
{
public:
    gsGLAMDDebugOutputManager();
    ~gsGLAMDDebugOutputManager();

    bool initialize();

    bool setCallback(GLDEBUGPROCAMD callback, GLvoid* userData);
    bool applyMessageFilter(GLenum category, GLenum severity, bool enable);
    bool clearMessageFilter(bool enable);

private:
    bool m_isInitialized;
    PFNGLDEBUGMESSAGEENABLEAMDPROC m_glDebugMessageEnableAMD;
    PFNGLDEBUGMESSAGECALLBACKAMDPROC m_glDebugMessageCallbackAMD;
};

// ----------------------------------------------------------------------------------
// Class Name:          gsGLARBDebugOutputManager
// General Description: Manages the integration with debug output extensions
//
// Author:              Uri Shomroni
// Creation Date:       13/11/2012
// ----------------------------------------------------------------------------------
class gsGLARBDebugOutputManager
{
public:
    gsGLARBDebugOutputManager();
    ~gsGLARBDebugOutputManager();

    bool initialize();

    bool setCallback(GLDEBUGPROCARB callback, GLvoid* userData);
    bool applyMessageFilter(GLenum source, GLenum type, GLenum severity, bool enable);
    bool clearMessageFilter(bool enable);

private:
    bool m_isInitialized;
    PFNGLDEBUGMESSAGECONTROLARBPROC m_glDebugMessageControlARB;
    PFNGLDEBUGMESSAGECALLBACKARBPROC m_glDebugMessageCallbackARB;
};

// ----------------------------------------------------------------------------------
// Class Name:          gsGL43DebugOutputManager
// General Description: Manages the integration with the OpenGL 4.3 / GL_KHR_debug
//                      version of the debug output extensions.
// Author:              Uri Shomroni
// Creation Date:       23/6/2014
// ----------------------------------------------------------------------------------
class gsGL43DebugOutputManager
{
public:
    gsGL43DebugOutputManager();
    ~gsGL43DebugOutputManager();

    bool initialize();

    bool setCallback(GLDEBUGPROC callback, const void* userData);
    bool applyMessageFilter(GLenum source, GLenum type, GLenum severity, bool enable);
    bool clearMessageFilter(bool enable);

private:
    bool m_isInitialized;
    PFNGLDEBUGMESSAGECONTROLPROC m_glDebugMessageControl;
    PFNGLDEBUGMESSAGECALLBACKPROC m_glDebugMessageCallback;
};

// ----------------------------------------------------------------------------------
// Class Name:          gsGLDebugOutputManager
// General Description: Manages the integration with all debug output extensions and
//                      connects to the specific manager.
// Author:              Uri Shomroni
// Creation Date:       23/6/2014
// ----------------------------------------------------------------------------------
class gsGLDebugOutputManager
{
public:
    gsGLDebugOutputManager(int contextId, gsForcedModesManager& forcedModesManager);
    ~gsGLDebugOutputManager();

    void onFirstTimeContextMadeCurrent();
    void applyDebugOutputSettingsToContext();

    void on43UserCallbackSet(GLDEBUGPROC callback, const void* userParam) { m_43UserCallbackDetails.m_userCallback = callback; m_43UserCallbackDetails.m_userData = userParam; };
    void onARBUserCallbackSet(GLDEBUGPROCARB callback, GLvoid* userParam) { m_arbUserCallbackDetails.m_userCallback = callback; m_arbUserCallbackDetails.m_userData = userParam; };
    void onAMDUserCallbackSet(GLDEBUGPROCAMD callback, GLvoid* userParam) { m_amdUserCallbackDetails.m_userCallback = callback; m_amdUserCallbackDetails.m_userData = userParam; };

private:
    enum gsDebugOutputSupportLevel
    {
        GS_OPENGL_43_OR_KHR_EXTENSION_SUPPORTED,
        GS_ARB_EXTENSION_SUPPORTED,
        GS_AMD_EXTENSION_SUPPORTED,
        GS_NO_DEBUG_OUTPUT_SUPPORT
    };
    struct gs43UserCallbackDetails
    {
    public:
        gs43UserCallbackDetails() : m_userCallback(NULL), m_userData(NULL) {};
        ~gs43UserCallbackDetails() {};
        GLDEBUGPROC m_userCallback;
        const void* m_userData;
    };
    struct gsARBUserCallbackDetails
    {
    public:
        gsARBUserCallbackDetails() : m_userCallback(NULL), m_userData(NULL) {};
        ~gsARBUserCallbackDetails() {};
        GLDEBUGPROCARB m_userCallback;
        GLvoid* m_userData;
    };
    struct gsAMDUserCallbackDetails
    {
    public:
        gsAMDUserCallbackDetails() : m_userCallback(NULL), m_userData(NULL) {};
        ~gsAMDUserCallbackDetails() {};
        GLDEBUGPROCAMD m_userCallback;
        GLvoid* m_userData;
    };

private:
    // Disallow use of default constructor, copy constructor and assignment operator:
    gsGLDebugOutputManager() = delete;
    gsGLDebugOutputManager(const gsGLDebugOutputManager&) = delete;
    gsGLDebugOutputManager& operator=(const gsGLDebugOutputManager&) = delete;

    static void APIENTRY gsGL43DebugOutputCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam);
    static void APIENTRY gsGLARBDebugOutputCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam);
    static void APIENTRY gsGLAMDDebugOutputCallbackFunction(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam);

    void onMessageCallback(apGLDebugOutputSource source, apGLDebugOutputType type, GLuint id, apGLDebugOutputSeverity severity, GLsizei length, const GLchar* message);
    const gs43UserCallbackDetails& get43CallbackDetails() const { return m_43UserCallbackDetails; };
    const gsARBUserCallbackDetails& getARBCallbackDetails() const { return m_arbUserCallbackDetails; };
    const gsAMDUserCallbackDetails& getAMDCallbackDetails() const { return m_amdUserCallbackDetails; };

private:
    int m_contextId;
    gsDebugOutputSupportLevel m_currentContextDebugOutputExtensionSupportLevel;

    gsGL43DebugOutputManager m_43OutputManager;
    gs43UserCallbackDetails m_43UserCallbackDetails;
    gsGLARBDebugOutputManager m_arbOutputManager;
    gsARBUserCallbackDetails m_arbUserCallbackDetails;
    gsGLAMDDebugOutputManager m_amdOutputManager;
    gsAMDUserCallbackDetails m_amdUserCallbackDetails;
    int m_amoutOfGLDebugOutputPrintouts;

    gsForcedModesManager& m_forcedModesManager;
};


#endif

#endif //__gsGLDebugOutputManager_H
