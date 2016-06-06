//=============================================================
// (c) 2016 Advanced Micro Devices, Inc.
//
/// \author CodeXL Developer Tools
/// \version $Revision: $
/// \brief Common Data Types used by codexl cpu and power profilers
//
//=============================================================

#ifndef _AMDTCOMMONDATATYPES_H_
#define _AMDTCOMMONDATATYPES_H_

// Base headers
#include <AMDTCommonHeaders/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

//
//  Macros
//

// Session Info Keys
#define AMDT_SESSION_INFO_KEY_DB_SCHEME_VERSION         L"DB Scheme Version"
#define AMDT_SESSION_INFO_VALUE_DB_SCHEME_VERSION       L"2.0"

#define AMDT_SESSION_INFO_KEY_TARGET_MACHINE            L"Target Machine Name"
#define AMDT_SESSION_INFO_KEY_TARGET_APP_WORKING_DIR    L"Target App Working Dir"
#define AMDT_SESSION_INFO_KEY_TARGET_APP_PATH           L"Target App Path"
#define AMDT_SESSION_INFO_KEY_SESSION_DIR               L"Session Dir"
#define AMDT_SESSION_INFO_KEY_TARGET_APP_CMD_LINE_ARGS  L"Target App Cmd Line Args"
#define AMDT_SESSION_INFO_KEY_TARGET_APP_ENV_VARS       L"Target App Env Vars"
#define AMDT_SESSION_INFO_KEY_SESSION_TYPE              L"Session Type"
#define AMDT_SESSION_INFO_KEY_SESSION_SCOPE             L"Session Scope"
#define AMDT_SESSION_INFO_KEY_SYSTEM_DETAILS            L"System Details"
#define AMDT_SESSION_INFO_KEY_SESSION_START_TIME        L"Session Start Time"
#define AMDT_SESSION_INFO_KEY_SESSION_END_TIME          L"Session End Time"

#define AMDT_SESSION_INFO_CPU_FAMILY                    L"CPU Family"
#define AMDT_SESSION_INFO_CPU_MODEL                     L"CPU Model"
#define AMDT_SESSION_INFO_CORE_AFFNITY                  L"CPU Affinity"
#define AMDT_SESSION_INFO_CSS_UNWIND_DEPTH              L"CSS Unwind Depth"
#define AMDT_SESSION_INFO_CSS_UNWIND_SCOPE              L"CSS Unwind Scope"
#define AMDT_SESSION_INFO_CSS_ENABLED                   L"CSS Enabled"
#define AMDT_SESSION_INFO_FPO_ENABLED                   L"CSS FPO Enabled"

#define AMDT_SESSION_INFO_VALUE_NO                      L"No"
#define AMDT_SESSION_INFO_VALUE_YES                     L"Yes"

//
//  Structs
//

struct AMDTProfileSessionInfo
{
    AMDTProfileSessionInfo() {}
    ~AMDTProfileSessionInfo() {}

    AMDTProfileSessionInfo(const AMDTProfileSessionInfo& other) = delete;

    void Clear()
    {
        m_sessionDbFullPath.makeEmpty();
        m_targetMachineName.makeEmpty();
        m_targetAppPath.makeEmpty();
        m_targetAppWorkingDir.makeEmpty();
        m_sessionDir.makeEmpty();
        m_targetAppCmdLineArgs.makeEmpty();
        m_targetAppEnvVars.makeEmpty();
        m_sessionType.makeEmpty();
        m_sessionScope.makeEmpty();
        m_sessionStartTime.makeEmpty();
        m_sessionEndTime.makeEmpty();
        m_systemDetails.makeEmpty();

        m_cpuFamily         = 0;
        m_cpuModel          = 0;
        m_coreAffinity      = 0;
        m_unwindDepth       = 0;
        m_unwindScope       = 0;
        m_cssEnabled        = false;
        m_cssFPOEnabled     = false;
    };

    // DB full path.
    gtString    m_sessionDbFullPath;

    // Target machine name.
    gtString    m_targetMachineName;

    // Target application's path.
    gtString    m_targetAppPath;

    // Target application's working directory.
    gtString    m_targetAppWorkingDir;

    // Session directory.
    gtString    m_sessionDir;

    // Target application's command line arguments.
    gtString    m_targetAppCmdLineArgs;

    // Target application's environment variables.
    gtString    m_targetAppEnvVars;

    // Session type.
    gtString    m_sessionType;

    // Session scope.
    gtString    m_sessionScope;

    // Session start time.
    gtString    m_sessionStartTime;

    // Session end time.
    gtString    m_sessionEndTime;

    // System details.
    gtString    m_systemDetails;

    gtUInt32    m_cpuFamily = 0;
    gtUInt32    m_cpuModel = 0;
    gtUInt64    m_coreAffinity = 0;
    gtUInt16    m_unwindDepth = 0;
    gtUInt16    m_unwindScope = 0;
    bool        m_cssEnabled = false;
    bool        m_cssFPOEnabled = false;
};

#endif // _AMDTCOMMONDATATYPES_H_