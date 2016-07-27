//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osProcess.cpp
///
//=====================================================================

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTOSWrappers/Include/osProcess.h>

OS_API void osTerminateProcessesByName(const gtVector<gtString>& processNames, const osProcessId parentProcessId, const bool exactMatch, bool isGracefulShutdownRequired)
{
    osProcessesEnumerator processEnum;

    if (processEnum.initialize())
    {
        osProcessId processId = 0;
        gtString executableName;

        size_t numNames = processNames.size();

        while (processEnum.next(processId, &executableName))
        {
            if (((osProcessId)0) == processId)
            {
                continue;
            }

            for (size_t nName = 0; nName < numNames; nName++)
            {
                if ((exactMatch && executableName == processNames[nName]) || (!exactMatch && executableName.find(processNames[nName]) != -1))
                {
                    // check for the parent process id
                    if (0 == parentProcessId || osIsParent(parentProcessId, processId))
                    {
                        // Terminate the process
                        osTerminateProcess(processId,0,true, isGracefulShutdownRequired);

                        if (exactMatch)
                        {
                            break;
                        }
                    }
                }
            }
        }
    }
}

OS_API bool osIsProcessAlive(const gtString& processName)
{
    osProcessesEnumerator processEnum;
    bool result = false;

    if (processEnum.initialize())
    {
        osProcessId processId = 0;
        gtString executableName;

        while (processEnum.next(processId, &executableName))
        {
            if (executableName.compareNoCase(processName) == 0)
            {
                result = true;
                break;
            }

        }//while
    }

    return result;
}

OS_API bool osTokenizeCurrentProcessEnvVariableValue(const gtString& envVariableName, const gtString& delimiters, gtVector<gtString>& envVariableTokens)
{
    envVariableTokens.clear();
    gtString envVarValue;
    gtString token;
    bool rc = osGetCurrentProcessEnvVariableValue(envVariableName, envVarValue);

    if (rc)
    {
        gtStringTokenizer tokenizer(envVarValue, delimiters);

        while (tokenizer.getNextToken(token))
        {
            envVariableTokens.push_back(token);
        }
    }

    return rc;
}