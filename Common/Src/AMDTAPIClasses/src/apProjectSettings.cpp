//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apProjectSettings.cpp
///
//==================================================================================

//------------------------------ apProjectSettings.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/apProjectSettings.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>

// ---------------------------------------------------------------------------
// Name:        apProjectSettings::apProjectSettings
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        29/6/2004
// ---------------------------------------------------------------------------
apProjectSettings::apProjectSettings()
    : m_isRemoteTarget(false), m_doNotUseVSDebugEngine(false), m_remoteTargetDaemonConnectionPort(0)
{
}

// ---------------------------------------------------------------------------
// Name:        apProjectSettings::apProjectSettings
// Description: Constructor
// Arguments:
//   executablePath - The full path and name of the executable to be debugged.
//   commandLineArguments - Command line arguments as a command line string.
//   workingDirectory - The initial work directory of the debugged process.
// Author:  AMD Developer Tools Team
// Date:        9/11/2003
// ---------------------------------------------------------------------------
apProjectSettings::apProjectSettings(const gtString& projectName, const osFilePath& executablePath,
                                     const gtString& commandLineArguments, const osFilePath& workDirectory)
    : m_projectName(projectName),
      m_isRemoteTarget(false),
      m_doNotUseVSDebugEngine(false),
      m_remoteTargetDaemonConnectionPort(0),
      m_executablePath(executablePath),
      m_commandLineArguments(commandLineArguments),
      m_workDirectory(workDirectory),
      m_sourceFilesDirectories(workDirectory.asString())
{
}

// ---------------------------------------------------------------------------
// Name:        apProjectSettings::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apProjectSettings::type() const
{
    return OS_TOBJ_ID_PROCESS_CREATION_DATA;
}


// ---------------------------------------------------------------------------
// Name:        apProjectSettings::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apProjectSettings::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the project name:
    ipcChannel << m_projectName;

    // Write the remote connection details:
    ipcChannel << m_isRemoteTarget;
    ipcChannel << m_remoteTargetName;
    ipcChannel << m_remoteTargetDaemonConnectionPort;

    // Write the VS debug engine flag:
    ipcChannel << m_doNotUseVSDebugEngine;

    // Write the path of the debugged executable into the channel:
    retVal = m_executablePath.writeSelfIntoChannel(ipcChannel);

    // Write the Windows Store App User Model ID:
    ipcChannel << m_winStoreAppUserModelId;

    // Write the command line arguments into the channel:
    ipcChannel << m_commandLineArguments;

    // Write the initial work directory of the debugged process into the channel:
    retVal = m_workDirectory.writeSelfIntoChannel(ipcChannel) && retVal;

    // Write the path for the log files
    retVal = m_logFilesFolder.directoryPath().writeSelfIntoChannel(ipcChannel) && retVal;

    // Write the environment variables into the channel:
    gtInt64 amountOfVariables = (gtInt64)m_environmentVariables.size();
    ipcChannel << amountOfVariables;

    // Write the environment variable into the channel:
    gtList<osEnvironmentVariable>::const_iterator iter = m_environmentVariables.begin();
    gtList<osEnvironmentVariable>::const_iterator endIter = m_environmentVariables.end();

    while (iter != endIter)
    {
        // Get the current parameter (as non const):
        osEnvironmentVariable currentVariable = (*iter);

        // Write the variable:
        ipcChannel << currentVariable._name;
        ipcChannel << currentVariable._value;
        iter++;
    }

    ipcChannel << m_lastActiveMode;
    ipcChannel << m_lastActiveSessionType;

    ipcChannel << m_sourceFilesDirectories;
    ipcChannel << m_sourceCodeRootLocation;

    ipcChannel << m_recentlyUsedRemoteIPAddressesAsString;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apProjectSettings::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apProjectSettings::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the project name:
    ipcChannel >> m_projectName;

    // Read the remote connection details:
    ipcChannel >> m_isRemoteTarget;
    ipcChannel >> m_remoteTargetName;
    ipcChannel >> m_remoteTargetDaemonConnectionPort;

    // Read the VS debug engine flag:
    ipcChannel >> m_doNotUseVSDebugEngine;

    // Read the path of the debugged executable from the channel:
    retVal = m_executablePath.readSelfFromChannel(ipcChannel);

    // Write the Windows Store App User Model ID:
    ipcChannel >> m_winStoreAppUserModelId;

    // Read the command line arguments from the channel:
    ipcChannel >> m_commandLineArguments;

    // Read the initial work directory of the debugged process from the channel:
    retVal = m_workDirectory.readSelfFromChannel(ipcChannel) && retVal;

    // Write the path for the log files
    osFilePath logFilesDir;
    logFilesDir.readSelfFromChannel(ipcChannel)&& retVal;
    m_logFilesFolder.setDirectoryPath(logFilesDir.reinterpretAsDirectory());

    // Read the environment variables amount:
    gtInt64 amountOfVariables = 0;
    ipcChannel >> amountOfVariables;

    // Read the environment variables:
    for (int i = 0; i < amountOfVariables; i++)
    {
        osEnvironmentVariable currentVariable;

        // Read the current variable name:
        ipcChannel >> currentVariable._name;

        // Read the current variable value:
        ipcChannel >> currentVariable._value;

        m_environmentVariables.push_back(currentVariable);
    }

    ipcChannel >> m_lastActiveMode;
    ipcChannel >> m_lastActiveSessionType;

    ipcChannel >> m_sourceFilesDirectories;
    ipcChannel >> m_sourceCodeRootLocation;

    ipcChannel >> m_recentlyUsedRemoteIPAddressesAsString;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apProjectSettings::addEnvironmentVariablesString
// Description: Add the environment variable from string.
// Author:  AMD Developer Tools Team
// Date:        7/9/2005
// ---------------------------------------------------------------------------
void apProjectSettings::addEnvironmentVariablesString(const gtString& envVariableString, const gtString& delimiters)
{
    // Check if the string is not empty:
    if (!envVariableString.isEmpty())
    {
        gtStringTokenizer tokenizer(envVariableString, delimiters);

        gtString currentToken;

        // Get the next token (variable name and value):
        while (tokenizer.getNextToken(currentToken))
        {
            // find the separator:
            int equalLocation = currentToken.find('=');

            // If we found the separator:
            if (equalLocation != -1)
            {
                // Get the var name and value:
                gtString curVarName;
                gtString curVarValue;
                currentToken.getSubString(0, equalLocation - 1, curVarName);
                currentToken.getSubString(equalLocation + 1, 0, curVarValue);

                osEnvironmentVariable currentVariable;
                currentVariable._name = curVarName;
                currentVariable._value = curVarValue;

                addEnvironmentVariable(currentVariable);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        apProjectSettings::getEnvironmentVariable
// Description: Gets the value of an environment variable, if set, and sets
//              into envVarVal
// Return Val:  true = the variable was found
//              false = the variable was not found
// Author:  AMD Developer Tools Team
// Date:        12/10/2010
// ---------------------------------------------------------------------------
bool apProjectSettings::getEnvironmentVariable(const gtString& envVarName, gtString& envVarVal) const
{
    bool retVal = false;
    envVarVal.makeEmpty();

    // Search the variables list:
    gtList<osEnvironmentVariable>::const_iterator iter = m_environmentVariables.begin();
    gtList<osEnvironmentVariable>::const_iterator endIter = m_environmentVariables.end();

    while (iter != endIter)
    {
        if ((*iter)._name == envVarName)
        {
            envVarVal = (*iter)._value;
            retVal = true;
            break;
        }

        iter++;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apProjectSettings::setEnvironmentVariable
// Description: Changes an environment variable's value to a given value. If it
//              doesn't exist, adds it.
// Author:  AMD Developer Tools Team
// Date:        12/10/2010
// ---------------------------------------------------------------------------
void apProjectSettings::setEnvironmentVariable(const gtString& envVarName, const gtString& envVarVal)
{
    bool foundVar = false;

    // Search the variables list:
    gtList<osEnvironmentVariable>::iterator iter = m_environmentVariables.begin();
    gtList<osEnvironmentVariable>::iterator endIter = m_environmentVariables.end();

    while (iter != endIter)
    {
        if ((*iter)._name == envVarName)
        {
            (*iter)._value = envVarVal;
            foundVar = true;
        }

        iter++;
    }

    // If we haven't found the variable, add it instead:
    if (!foundVar)
    {
        osEnvironmentVariable newEnvVar;
        newEnvVar._name = envVarName;
        newEnvVar._value = envVarVal;
        addEnvironmentVariable(newEnvVar);
    }
}

// ---------------------------------------------------------------------------
// Name:        apProjectSettings::environmentVariablesAsString
// Description: Get the environment variable as string.
// Author:  AMD Developer Tools Team
// Date:        8/9/2005
// ---------------------------------------------------------------------------
void apProjectSettings::environmentVariablesAsString(gtString& envVariablesAsString) const
{
    envVariablesAsString.makeEmpty();

    gtList<osEnvironmentVariable>::const_iterator iter = m_environmentVariables.begin();
    gtList<osEnvironmentVariable>::const_iterator endIter = m_environmentVariables.end();

    bool isFirstItem = true;

    while (iter != endIter)
    {
        // Adding "\n" before each item, while bypassing the first one
        if (isFirstItem)
        {
            isFirstItem = false;
        }
        else
        {
            // Add the separator:
            envVariablesAsString.append(L"\n");
        }

        // Get the current environment variable name:
        envVariablesAsString.append((*iter)._name);

        // Add the equal sign:
        envVariablesAsString.append(L"=");

        // Get the current environment variable value:
        envVariablesAsString.append((*iter)._value);

        iter++;
    }
}


// ---------------------------------------------------------------------------
// Name:        apProjectSettings::setExecutablePathFromString
// Description:
// Arguments:   const gtString& exePath
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        8/4/2012
// ---------------------------------------------------------------------------
void apProjectSettings::setExecutablePathFromString(const gtString& exePath)
{
    m_executablePath.setFullPathFromString(exePath);
}

// ---------------------------------------------------------------------------
// Name:        apProjectSettings::setWorkDirectoryFromString
// Description: Set my working folder from an ASCII string
// Arguments:   const gtString& exePath
// Author:  AMD Developer Tools Team
// Date:        8/4/2012
// ---------------------------------------------------------------------------
void apProjectSettings::setWorkDirectoryFromString(const gtString& workFolderPath)
{
    m_workDirectory.setDirectoryFullPathFromString(workFolderPath);
}

void apProjectSettings::SetRemoteTargetHostname(const gtString& remoteHostName)
{
    // Set the current remote target:
    m_remoteTargetName = remoteHostName;

    if (m_recentlyUsedRemoteIPAddressesAsString.findFirstOf(remoteHostName) < 0)
    {
        m_recentlyUsedRemoteIPAddressesAsString.append(L";");
        m_recentlyUsedRemoteIPAddressesAsString.append(remoteHostName);
    }
}

void apProjectSettings::GetRecentlyUsedRemoteIPAddresses(gtVector<gtString>& addressesVector) const
{
    gtStringTokenizer tokenizer(m_recentlyUsedRemoteIPAddressesAsString, L";");

    // Parse the string and add the addresses to the vector:
    gtString currentHost;

    // Push the rest of the path directories:
    while (tokenizer.getNextToken(currentHost))
    {
        addressesVector.push_back(currentHost);
    }
}
