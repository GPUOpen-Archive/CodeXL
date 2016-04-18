//==============================================================================
// Copyright (c) 2007-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Support for session related activities
//==============================================================================

#include "SessionManager.h"
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include "Logger.h"
#include "misc.h"

bool SessionManager::GetSessionManagerData(SessionManagerData& smd)
{
    osModuleArchitecture moduleArchitecture;
    osRuntimePlatform currentPlatform;
    gtString executablePath;
    gtString commandLine;
    gtString workingDirectory;

    // Retrieve the name of the instrumented application. Construct a metadata filename which references it.
    if (osGetProcessLaunchInfo(osGetCurrentProcessId(), moduleArchitecture, currentPlatform, executablePath, commandLine, workingDirectory) == false)
    {
        Log(logERROR, "Failed to retrieve process launch info for target application.\n");
        return false;
    }

    osFilePath executableFilepath;
    executableFilepath.setFullPathFromString(executablePath);

    if (executableFilepath.getFileName(smd.appName) == false)
    {
        Log(logERROR, "Failed to retrieve the instrumented process's application filename.\n");
        return false;
    }

    osTime currentTime;
    currentTime.setFromCurrentTime();
    tm timeStruct;
    currentTime.timeAsTmStruct(timeStruct, osTime::LOCAL);

    // Need to add 1900, since tm contains "years since 1900".
    smd.year = timeStruct.tm_year + 1900;

    // Need to add 1, since tm contains "months since January".
    smd.month = timeStruct.tm_mon + 1;
    smd.day = timeStruct.tm_mday;

    smd.hour = timeStruct.tm_hour;
    smd.minute = timeStruct.tm_min;
    smd.second = timeStruct.tm_sec;

    // ExecutableFilename-YEAR-MM-DD-HOUR-MINUTE-SECOND
    smd.metadataFilename.appendFormattedString("description-%s-%d-%d-%d-%d-%d-%d.xml",
                                               smd.appName.asASCIICharArray(),
                                               smd.year, smd.month, smd.day,
                                               smd.hour, smd.minute, smd.second);

    // Build a path to the GPS folder within the Temp directory.
    osFilePath systemTempDirectory;
    systemTempDirectory.setPath(osFilePath::OS_TEMP_DIRECTORY);

#ifdef CODEXL_GRAPHICS
    smd.toolDirectory.fromASCIIString("CodeXL");
#else
    smd.toolDirectory.fromASCIIString(GetPerfStudioDirName());
#endif
    systemTempDirectory.appendSubDirectory(smd.toolDirectory);

    // Check to see if the user defined a project name
    if (m_strProjectName.length() > 0)
    {
        // Create a gtString version of the project name
        gtString projectNameString;
        projectNameString.fromASCIIString(m_strProjectName.asCharArray());
        // Add the project name to the path
        systemTempDirectory.appendSubDirectory(projectNameString);

        smd.projectName = projectNameString;
    }
    else
    {
        // Create a gtString version of the app name and use it as the project name
        gtString appNameString;
        appNameString.fromASCIIString(smd.appName.asASCIICharArray());

        // Add the app name to the path
        systemTempDirectory.appendSubDirectory(appNameString);

        smd.projectName = appNameString;
    }

    // Generate the session name string. This string will be re-used by subsequent captures untiol the session name changes.
    if (m_bRegenSessionNameString == true)
    {
        // Construct the session name
        // m_strSessionNameString.appendFormattedString("%s_%d_%d_%d_%d_%d_%d", m_strSessionName.asCharArray(), smd.year, smd.month, smd.day, smd.hour, smd.minute, smd.second);
        m_strSessionNameString = m_strSessionName;
        m_bRegenSessionNameString = false;
    }

    // Create an ascii string version of the path
    smd.pathToDataDirectory = systemTempDirectory.asString().asASCIICharArray();
    // Add the session name and the frame index string.
    smd.pathToDataDirectory.appendFormattedString("\\%s\\Frame_%010d\\", m_strSessionNameString.asCharArray(), smd.frameIndex);

    // Convert the full path back to a gtString
    gtString fullPathToDataDirectoryAsGTString;
    fullPathToDataDirectoryAsGTString.fromASCIIString(smd.pathToDataDirectory.asCharArray());

    // Create the data directory if it doesn't already exist.
    osDirectory dir;
    dir.setDirectoryFullPathFromString(fullPathToDataDirectoryAsGTString);

    if (!dir.exists())
    {
        bool bDirectoryCreated = dir.create();

        if (!bDirectoryCreated)
        {
            Log(logERROR, "Failed to create data directory for traced frame: '%s'.\n", fullPathToDataDirectoryAsGTString.asASCIICharArray());
            return false;
        }
    }

    return true;
}

bool SessionManager::SetSessionName(gtASCIIString name)
{
    // Remove any special XML characters
    name.replace("%20", " ");
    name.replace("%22", "\"");
    name.replace("%5C", "\\");
    name.replace("%E2%80%93", "-");
    name.replace("%26", "&");
    name.replace("%27", "'");
    name.replace("%60", "`");
    name.replace("%E2%80%98", "`");

    // Check for zero length name
    if (name.length() < 1)
    {
        if (m_strSessionName == m_strDefaultName)
        {
            // no need to do anything, the default name is current
            return true;
        }

        // We must regenerate the full path using the default session name
        m_bRegenSessionNameString = true;
        m_strSessionNameString.makeEmpty();
        SetDefaultSessionName();
        return true;
    }

    // Check to see if there is no change
    if (m_strSessionName == name)
    {
        // we do not need to make a change
        return true;
    }

    // Set the new session name and force a regeneration of the full path based on this new name.
    m_bRegenSessionNameString = true;
    m_strSessionNameString.makeEmpty();
    m_strSessionName = name;

    return true;
}

bool SessionManager::SetProjectName(gtASCIIString name)
{
    // Remove any special XML characters
    name.replace("%20", " ");
    name.replace("%22", "\"");
    name.replace("%5C", "\\");
    name.replace("%E2%80%93", "-");
    name.replace("%26", "&");
    name.replace("%27", "'");
    name.replace("%60", "`");
    name.replace("%E2%80%98", "`");

    // Check for zero length name
    if (name.length() < 1)
    {
        m_strProjectName.makeEmpty();

        return true;
    }

    // Check to see if there is no change
    if (m_strProjectName == name)
    {
        // we do not need to make a change
        return true;
    }

    // Set the new session name
    m_strProjectName = name;

    Log(logTRACE, "Setting m_strProjectName to: '%s'\n", m_strProjectName.asCharArray());

    return true;
}
