//==============================================================================
// Copyright (c) 2007-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Support for session related activities
//==============================================================================

#ifndef GPS_SESSION_MANAGER_H
#define GPS_SESSION_MANAGER_H

#include "TSingleton.h"
#include "../Common/OSwrappers.h"

/// This struct is used to pass data in and out of the GetSessionManagerData() method.
typedef struct
{
    gtASCIIString pathToDataDirectory; ///< Output value
    gtASCIIString metadataFilename; ///< Output value
    gtString toolDirectory; ///< Output value
    gtString appName; ///< Output value
    gtString projectName; ///< Output value
    int year; ///< Output value
    int month; ///< Output value
    int day; ///< Output value
    int hour;  ///< Output value
    int minute; ///< Output value
    int second; ///< Output value
    int frameIndex; ///<  Input value
    osModuleArchitecture moduleArchitecture; ///< Input value
} SessionManagerData;

/// This class is responsible for managing the session related aspects of the capture to disk feature
class SessionManager : public TSingleton< SessionManager >
{
    /// TSingleton needs to be able to use our constructor.
    friend class TSingleton<SessionManager>;

    /// The name of the session.
    gtASCIIString m_strSessionName;

    /// The fully formatted session name string used once per server session. This string is the sessionName with date information appended to the end.
    gtASCIIString m_strSessionNameString;

    /// Flag to control when the session name needs to be regenerated.
    bool m_bRegenSessionNameString;

    /// The default session name
    gtASCIIString m_strDefaultName;

    /// The name of the project.
    gtASCIIString m_strProjectName;

public:

    /// Constructor
    SessionManager()
    {
        m_strDefaultName = "Session_";

        SetDefaultSessionName();

        // Force a regeneration of the session name string
        m_bRegenSessionNameString = true;
    }

    /// Sets the session name to the default
    void SetDefaultSessionName()
    {
        m_strSessionName = m_strDefaultName;
    }

    /// This method populates the data fields in the input SessionManagerData so they can be used by the API Trace and Frame Capture tools when writing their capture data to disk.
    /// \param smd The input/output session manger data
    /// \return True if success, False if failure.
    bool GetSessionManagerData(SessionManagerData& smd);

    /// Set the name of the session
    /// \param name The session name
    bool SetSessionName(gtASCIIString name);

    /// Set the name of the project
    /// \param name The project name
    bool SetProjectName(gtASCIIString name);
};

#endif //GPS_SESSION_MANAGER_H