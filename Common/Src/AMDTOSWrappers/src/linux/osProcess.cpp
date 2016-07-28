//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osProcess.cpp
///
//=====================================================================


// Standard C:
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <algorithm>

// POSIX:
#include <signal.h>
#include <dirent.h>
#include <errno.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osProcessSharedFile.h>

// Local:
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osProcess.h>

// redirection files:
osProcessSharedFile g_outputRedirectFile;
osProcessSharedFile g_inputRedirectFile;

void osRemovePathFromLibraryPath(const gtString& pathToRemove)
{
    // Get the current LD_LIBRARY_PATH value
    gtString envVarName(L"LD_LIBRARY_PATH");
    gtString envVarValue;

    osGetCurrentProcessEnvVariableValue(envVarName, envVarValue);

    // find the starting position of the codexl path and find the ending ':' or end of the envVarValue and remove it
    // continue until all paths are removed
    int runTimeLibPos = -1;

    do
    {
        runTimeLibPos = envVarValue.find(pathToRemove);

        if (runTimeLibPos != -1)
        {
            int endPos = envVarValue.find(':', runTimeLibPos + 1);

            if (-1 == endPos)
            {
                // if not found erase until the end
                envVarValue.extruct(runTimeLibPos, envVarValue.length());
            }
            else
            {
                // if found erase between the starting of the position and the ending ':'
                envVarValue.extruct(runTimeLibPos, endPos);
            }
        }
    }
    while (runTimeLibPos != -1);

    osEnvironmentVariable envVariable(envVarName, envVarValue);
    bool rc = osSetCurrentProcessEnvVariable(envVariable);
    GT_ASSERT(rc);
}

// remove the codexl run time lib path from LD_LIBRARY_PATH
void osRemoveRuntimeLibsPathFromLibraryPath()
{
    // the script of running CodeXL can add the RunTimeLibs in two way
    // either as .../bin/./RunTimeLibs or
    //        as .../bin/RunTimeLibs
    // so the two cases needs to be checked
    // start by getting the execution path
    osFilePath codeXLExecutionPath(osFilePath::OS_CODEXL_BINARIES_PATH);
    osFilePath codeXLPath(codeXLExecutionPath);
    codeXLPath.appendSubDirectory(OS_STR_runTimeDirectory);
    gtString codeXLPathAsStr = codeXLPath.asString();

    osRemovePathFromLibraryPath(codeXLPathAsStr);

    codeXLPath = codeXLExecutionPath;
    codeXLPath.appendSubDirectory(L".");
    codeXLPath.appendSubDirectory(OS_STR_runTimeDirectory);
    codeXLPathAsStr = codeXLPath.asString();

    osRemovePathFromLibraryPath(codeXLPathAsStr);
}

// ---------------------------------------------------------------------------
// Name:        osSetCurrentProcessEnvVariable
// Description: Adds / sets the value of an environment variable in the current
//              process environment block.
// Arguments:   envVariable - The environment variable to be added / set.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        11/12/2006
// ---------------------------------------------------------------------------
bool osSetCurrentProcessEnvVariable(const osEnvironmentVariable& envVariable)
{
    bool retVal = false;

    // Add / Set the environment variable value:
    int rc = ::setenv(envVariable._name.asASCIICharArray(), envVariable._value.asASCIICharArray(), 1);

    if (rc == 0)
    {
        retVal = true;
    }

    // In case of failure:
    if (!retVal)
    {
        gtString errMessage = OS_STR_FailedToSetEnvVariable;
        errMessage += envVariable._name;

        GT_ASSERT_EX(false, errMessage.asCharArray());
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osRemoveCurrentProcessEnvVariable
// Description: Remove an environment variable from the current process
//              environment block.
// Arguments:   envVariableName - The name of the environment variable to be removed.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        11/12/2006
// ---------------------------------------------------------------------------
bool osRemoveCurrentProcessEnvVariable(const gtString& envVariableName)
{
    bool retVal = false;

    // Remove the input variable name from the environment:
    int rc = ::unsetenv(envVariableName.asASCIICharArray());

    if (rc == 0)
    {
        retVal = true;
    }

    // In case of failure:
    if (!retVal)
    {
        gtString errMessage = OS_STR_FailedToRemoveEnvVariable;
        errMessage += envVariableName;

        GT_ASSERT_EX(false, errMessage.asCharArray());
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetCurrentProcessEnvVariableValue
// Description: Retrieves the value of an environment variable in the current
//              process environment block.
// Arguments:   envVariableName - The queried environment variable name
//              envVariableValue - Will get the environment variable value.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        11/12/2006
// ---------------------------------------------------------------------------
bool osGetCurrentProcessEnvVariableValue(const gtString& envVariableName, gtString& envVariableValue)
{
    bool retVal = false;

    // Get the environment variable value:
    const char* pEnvVariableValueStr = ::getenv(envVariableName.asASCIICharArray());

    if (pEnvVariableValueStr != NULL)
    {
        // Output the environment variable value:
        envVariableValue.fromASCIIString(pEnvVariableValueStr);
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetCurrentProcessId
// Description: Returns the OS Id of the current process.
// Author:      AMD Developer Tools Team
// Date:        11/12/2006
// ---------------------------------------------------------------------------
osProcessId osGetCurrentProcessId()
{
    osProcessId retVal = ::getpid();
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osExitCurrentProcess
// Description:
//   Exits the current process and all its threads in a "clean way".
//   (This includes notifying all attached DLLs about the process exit, etc)
// Author:      AMD Developer Tools Team
// Date:        21/3/2007
// ---------------------------------------------------------------------------
void osExitCurrentProcess(int exitCode)
{
    exit(exitCode);
}

// ---------------------------------------------------------------------------
// Name:        osWaitForProcessToTerminate
// Description: Waits for a process to terminate, with a limit of timeoutMsec Milli-seconds.
//              if pExitCode is not NULL (and retVal isn't false), returns the
//              process's exit code into it.  But we can't tell, so it will always be zero
// Return Val: true - the process exited.
// NOTE:        This is slightly semantically different than the Windows version.
//              The windows version will immediately return, but the granularity
//              of return on Linux is timeoutMsec.
//             false - the timeout expired before the process exited.
// Author:      AMD Developer Tools Team
// Date:        5-July-2012
// ---------------------------------------------------------------------------
bool osWaitForProcessToTerminate(osProcessId processId, unsigned long timeoutMsec, long* pExitCode, bool child)
{
    bool    theProcessExited = false;
    pid_t   waitPid;
    int     status;

    // Special (easy) case where the windows version has specified a timeout of INFINITE (-1)
    // In this case, we perform a blocking wait for the termination
    if (timeoutMsec == ULONG_MAX)
    {
        waitPid = waitpid(processId, &status, 0);
        theProcessExited = ((-1 != waitPid) && WIFEXITED(status));
    }
    else
    {
        // Sleep for timeoutMsec milliseconds, then check for the existance of /proc/<pid>
        // There is a pitfall here: pid recycling.  This won't happen if timeoutMsec is
        // reasonable, but if it is long enough for a whole 32-bits worth of new processes to
        // have been created during the sleep period, we will report the result incorrectly.
        // Unlikely, but possible.
        timespec    toSleep;
        long accumulatedWaitTimeNanoseconds = 0;
        long nanoSecondsInSingleWait = 0;
        long timeoutNanoseconds = timeoutMsec * 1000 * 1000;
        
        // pay attention to the possibility of overflow for a 32-bit long
        // basically, don't convert the number into nanoseconds and then clean it up.
        nanoSecondsInSingleWait = std::min<long>(50 * 1000 * 1000, timeoutNanoseconds);
        toSleep.tv_sec = 0;
        toSleep.tv_nsec = nanoSecondsInSingleWait;

        while (false == theProcessExited && accumulatedWaitTimeNanoseconds < timeoutNanoseconds)
        {
            (void)nanosleep(&toSleep, NULL);

            if (child)
            {
                // And now do a non-blocking wait
                waitPid = waitpid(processId, &status, WNOHANG);

                // 0 means child exists
                theProcessExited = (waitPid != 0);
            }
            else
            {
                osIsProcessAlive(processId, theProcessExited);
                theProcessExited = !theProcessExited;
            }
            accumulatedWaitTimeNanoseconds += nanoSecondsInSingleWait;
        }
    }

    // Unfortunately, we cannot detect the child process exit code on Linux.
    // So we lie
    if (pExitCode != NULL)
    {
        *pExitCode = 0;
    }

    return theProcessExited;
}


// ---------------------------------------------------------------------------
// Name:        osTerminateProcess
// Description: Terminates a process, running on the local machine.
// Arguments: processId - The id of the process to be terminated.
//            exitCode - ignored on Linux (used only in iPhone on-device).
//            isTerminateChildren - should processes spawned by the target process be terminated too
//            isGracefulShutdownRequired - attempts to close the process gracefully before forcefully terminating it. 
//                                          This option is currently not implemented on Windows.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        17/8/2009
// ---------------------------------------------------------------------------
bool osTerminateProcess(osProcessId processId, long exitCode, bool isTerminateChildren, bool isGracefulShutdownRequired)
{
    bool retVal = false;

    // Uri, 25/4/10: When debugging the iPhone on-device, we currently use XCode, which intercepts
    // all signals sent to the debugged process. So, we first try using ::exit in this case, to get
    // an exit in XCode, and only if that fails, we use the SIGKILL method.
#ifdef _GR_IPHONE_DEVICE_BUILD

    if (processId == osGetCurrentProcessId())
    {
        ::exit(exitCode);

        // We shouldn't get here:
        OS_OUTPUT_DEBUG_LOG(L"Could not terminate debugged application with ::exit(), using ::kill() instead.", OS_DEBUG_LOG_INFO);
    }

#endif

    if (isTerminateChildren)
    {
        osTerminateChildren(processId, isGracefulShutdownRequired);
    }

    if (isGracefulShutdownRequired)
    {
        int rcKill = ::kill(processId, SIGTERM);

        if (rcKill == 0)
        {
            const unsigned long timeoutWaitingForGracefulShutdownInMilliseconds = 2000;
            retVal = osWaitForProcessToTerminate(processId, timeoutWaitingForGracefulShutdownInMilliseconds);
        }
    }

    // If process is not dead yet
    if (!retVal)
    {
        int rcKill = ::kill(processId, SIGKILL);

        if (rcKill == 0)
        {
            retVal = true;
            waitpid(processId, NULL, 0);
        }
    }

    (void)(exitCode); // unused
    return retVal;
}


// Pipes to do handshakes between parent and child processes
#define READ_PIPE 0
#define WRITE_PIPE 1
static int stat_parentPipe[2];
static int stat_childPipe[2];

//Assumes arguments are either stand alone or wrapped in quotes.  Does not currently handle
// escaped characters.
bool osLaunchSuspendedProcess(
    const osFilePath& executablePath,
    const gtString& arguments,
    const osFilePath& workDirectory,
    osProcessId& processId,
    osProcessHandle& /*processHandle = ignored */,
    osThreadHandle& /*processThreadHandle = ignored */,
    bool createWindow,
    bool redirectFiles,
    bool removeCodeXLPaths)
{
    // Check if has permission to run the executablePath
    std::string utf8ExePath, utf8WorkDirectory;
    executablePath.asString().asUtf8(utf8ExePath);
    bool retVal = (0 == access(utf8ExePath.c_str(), X_OK));

    if (retVal)
    {
        // Check if has permission to chdir to the executablePath
        workDirectory.asString().asUtf8(utf8WorkDirectory);
        retVal = (0 == access(utf8WorkDirectory.c_str(), X_OK));

        if (retVal)
        {
            // Create the command string
            gtString cmdStr;

            // NOTE [Suravee]: On Linux, we use xterm to create
            // a separate window to launch the target application.
            // In this case, we basically launching the following
            // command:
            // /usr/bin/xterm -e <executablePath> <arguments>
            if ((createWindow) && (0 == access("/usr/bin/xterm", X_OK)))
            {
                cmdStr.append(L"/usr/bin/xterm -e ");
            }

            // Append executablePath and arguments to the command
            cmdStr.append(L"\"");
            cmdStr.append(executablePath.asString());
            cmdStr.append(L"\"");

            cmdStr.append(L" ");
            cmdStr.append(arguments);

            if (redirectFiles)
            {
                gtString outputFileName;
                gtString inputFileName;
                bool appendMode;

                if (osCheckForOutputRedirection(cmdStr, outputFileName, appendMode))
                {
                    g_outputRedirectFile.openFile(outputFileName, true, appendMode);
                }

                if (osCheckForInputRedirection(cmdStr, inputFileName))
                {
                    g_inputRedirectFile.openFile(inputFileName, false, false);
                }
            }

            // Create the arguments buffer
            wchar_t* pCmdBuf = static_cast<wchar_t*>(calloc(cmdStr.length() + 1, sizeof(wchar_t)));
            retVal = (pCmdBuf != nullptr);

            if (!retVal)
            {
                gtString errMsg(L"osLaunchSuspendedProcess: Failed to allocate space for args buffer\n");
                perror(errMsg.asASCIICharArray());
                OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
            }
            else
            {
                wcsncpy(pCmdBuf, cmdStr.asCharArray(), cmdStr.length());

                int offset = 0;
                wchar_t quoteStr[2] = { 0, 0 };
                wchar_t whiteStr[] = L" \t\n";
                gtString oneSubStr;
                gtVector<gtString> strArgList;

                //Get each argument
                while (offset < cmdStr.length())
                {
                    //Find the first character that's not white space
                    offset += wcsspn(&(pCmdBuf[offset]), whiteStr);

                    if (offset >= cmdStr.length())
                    {
                        break;
                    }

                    unsigned int subLen = 0;

                    //Check whether the substring is surrounded by quotes
                    if ((L'\'' == pCmdBuf[offset]) || (L'\"' == pCmdBuf[offset]))
                    {
                        //Get the length of the quoted substring
                        quoteStr[0] = pCmdBuf[offset];
                        offset++;
                        subLen = wcscspn(&(pCmdBuf[offset]), quoteStr);
                    }
                    else
                    {
                        subLen = wcscspn(&(pCmdBuf[offset]), whiteStr);
                    }

                    //Save the substring as an element in a vector
                    oneSubStr.makeEmpty();
                    oneSubStr = &(pCmdBuf[offset]);
                    oneSubStr.truncate(0, subLen - 1);
                    strArgList.push_back(oneSubStr);
                    offset += subLen;

                    //If ending a quote, skip the end quote
                    if (pCmdBuf[offset] == quoteStr[0])
                    {
                        offset++;
                        quoteStr[0] = L'\0';
                    }
                }

                // Counting number of tokens
                int argc = strArgList.size();

                // Create the arguments vector
                char** argv = static_cast<char**>(calloc((sizeof(char*) * (argc + 1)), 1));
                retVal = (argv != nullptr);

                if (!retVal)
                {
                    gtString errMsg(L"osLaunchSuspendedProcess: Failed to allocate space for arg vector\n");
                    perror(errMsg.asASCIICharArray());
                    OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
                }
                else
                {
                    // Argument 0 should be the launch string
                    gtVector<gtString>::const_iterator it = strArgList.begin();
                    gtVector<gtString>::const_iterator endIt = strArgList.end();
                    offset = 0;
                    int argI(0);
                    char utf8CmdBuf[FILENAME_MAX * 2] = {};
                    std::string utf8string;

                    //For each substring in the vector
                    for (; it != endIt; ++it)
                    {
                        // Convert wide char to UTF8
                        it->asUtf8(utf8string);

                        // Copy the arguments into the first available location
                        char*   argStart = &utf8CmdBuf[offset];
                        size_t  argLen = utf8string.length();
                        strcpy(argStart, utf8string.c_str());
                        argv[argI++] = argStart;

                        // Force-Write the terminator for this argv
                        utf8CmdBuf[offset + argLen] = 0x0;

                        // And adjust the offset to point to the next potential starting location
                        offset = offset + argLen + 1;
                    }

                    argv[argc] = NULL;
                    strArgList.clear();

                    // Forking new process
                    retVal = (pipe(stat_parentPipe) != -1);

                    if (!retVal)
                    {
                        gtString errMsg(L"Failed to parentPipe");
                        perror(errMsg.asASCIICharArray());
                        OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
                    }

                    retVal = (pipe(stat_childPipe) != -1);

                    if (!retVal)
                    {
                        gtString errMsg(L"Failed to parentPipe");
                        perror(errMsg.asASCIICharArray());
                        OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
                    }

                    pid_t targetPid = fork();
                    retVal = (targetPid >= 0);

                    if (!retVal)
                    {
                        gtString errMsg(L"Failed to fork");
                        perror(errMsg.asASCIICharArray());
                        OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
                        free(pCmdBuf);
                        free(argv);
                    }
                    else
                    {
                        if (!targetPid)
                        {
                            if (redirectFiles)
                            {
                                if (g_outputRedirectFile.handle() != 0)
                                {
                                    dup2(g_outputRedirectFile.handle(), STDOUT_FILENO);
                                    g_outputRedirectFile.closeFile();
                                }

                                if (g_inputRedirectFile.handle() != 0)
                                {
                                    dup2(g_inputRedirectFile.handle(), STDIN_FILENO);
                                    g_inputRedirectFile.closeFile();
                                }
                            }

                            // For child process
                            close(stat_parentPipe[READ_PIPE]);
                            close(stat_childPipe[WRITE_PIPE]);
                            fcntl(stat_childPipe[READ_PIPE], F_SETFD, FD_CLOEXEC);

                            // Tell parent, we are ready to exec the target app
                            close(stat_parentPipe[WRITE_PIPE]);

                            // Wait for the parent to tell us to exec
                            char buf;

                            if (read(stat_childPipe[READ_PIPE], &buf, 1) == -1)
                            {
                                gtString errMsg(L"osLaunchSuspendedProcess: Child cannot read the pipe.");
                                perror(errMsg.asASCIICharArray());
                                OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
                            }

                            // Go to workDirectory
                            if (0 == chdir(utf8WorkDirectory.c_str()))
                            {
                                // clear the codexl ld_library_path
                                if (removeCodeXLPaths)
                                {
                                    osRemoveRuntimeLibsPathFromLibraryPath();
                                }

                                // If success, this function will not return.
                                execvp(argv[0], argv);
                            }

                            // We reach this point only if the execvp function failed
                            gtString errMsg(L"osLaunchSuspendedProcess: Fail to execute with execvp");
                            perror(errMsg.asASCIICharArray());
                            OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
                            _exit(-1);
                        }

                        // For parent process
                        close(stat_childPipe[READ_PIPE]);
                        close(stat_parentPipe[WRITE_PIPE]);

                        // Set the pids
                        processId = targetPid;
                        free(pCmdBuf);
                        free(argv);
                    }
                }
            }
        }
    }

    return retVal;
}


bool osResumeSuspendedProcess(const osProcessId& /* processId */,
                              const osProcessHandle& /* processHandle = ignored */,
                              const osThreadHandle& /* processThreadHandle = ignored */,
                              bool /* closeHandles */)
{
    char buf;

    // wait for the child to notify that he is ready to exec
    if (read(stat_parentPipe[READ_PIPE], &buf, 1) == -1)
    {
        gtString errMsg(L"osResumeSuspendedProcess: Parent cannot read the pipe.");
        OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
        perror(errMsg.asASCIICharArray());
        return false;
    }

    close(stat_parentPipe[READ_PIPE]);

    // Now, tell child to exec the target app
    close(stat_childPipe[WRITE_PIPE]);

    return true;
}

// ---------------------------------------------------------------------------
// Name:        osCloseProcessRedirectionFiles
// Description: close redirection files for the process
// Author:      AMD Developer Tools Team
// Date:        23/6/2013
// ---------------------------------------------------------------------------
void osCloseProcessRedirectionFiles()
{
    g_inputRedirectFile.closeFile();
    g_outputRedirectFile.closeFile();
}


bool osSetProcessAffinityMask(osProcessId processId, const osProcessHandle processHandle, gtUInt64 affinityMask)
{
    (void)(processHandle); // unused
    cpu_set_t cpumask;
    CPU_ZERO(&cpumask);

    for (unsigned int i = 0; i < (sizeof(affinityMask) * 8); i++)
    {
        if (affinityMask & 1)
        {
            CPU_SET(i, &cpumask);
        }

        affinityMask >>= 1;
    }

    int res = sched_setaffinity(processId, sizeof(cpu_set_t), &cpumask);
    return 0 == res;
}


// ---------------------------------------------------------------------------
// Name:        osIsProcessAlive
// Description: Returns true iff the function succeeded. Buffer is set to true
//              iff the process is alive.
// Author:      AMD Developer Tools Team
// Date:        08/08/2013
// ---------------------------------------------------------------------------
bool osIsProcessAlive(osProcessId processId, bool& buffer)
{
    int rc = kill(processId, 0);
    buffer = (rc == 0);
    return true;
}


bool osGetProcessIdentificationInfo(osProcessId& processId, char* pName, gtSize_t* pNameLen)
{
   GT_ASSERT(pName != nullptr);
   GT_ASSERT(pNameLen != nullptr);
   char buffer[1024] = {};
   snprintf(buffer, sizeof(buffer), "/proc/%d/exe", processId);

   bool ret = false;
   char buf[512] = {};
   int count = readlink(buffer, buf, sizeof(buf));
   if (count >= 0 && count <= static_cast<int>(*pNameLen))
   {
     gtString name;
     name.fromASCIIString(buf);
     osFilePath path(name);
     path.getFileName(name);
     *pNameLen = name.length();
     memcpy(pName, name.asASCIICharArray(), *pNameLen);
     ret = true;
   }
  return ret;
}

bool osGetProcessIdentificationInfo(osProcessId& processId, osProcessId* pParentProcessId, osProcessId* pGroupId,
                                    char* pName, gtSize_t* pNameLen)
{
    char buffer[1024]= {};
    snprintf(buffer, sizeof(buffer), "/proc/%d/status", processId);

    bool ret = false;
    int fd = open(buffer, O_RDONLY, 0);

    if (-1 != fd)
    {
        int len = read(fd, buffer, sizeof(buffer) - 1);
        close(fd);

        if (0 < len)
        {
            buffer[len] = '\0';
            char* pLine = buffer;

            unsigned state = (1U                                                       << 0) |
                             (static_cast<unsigned>(NULL != pParentProcessId)          << 1) |
                             (static_cast<unsigned>(NULL != pGroupId)                  << 2) |
                             (static_cast<unsigned>(NULL != pName && NULL != pNameLen) << 3);

            for (char* pNewLine = NULL; 0 != state && NULL != pLine; pLine = pNewLine)
            {
                pNewLine = strchr(pLine, '\n');

                if (NULL != pNewLine)
                {
                    *pNewLine++ = '\0';
                }

                if (0U != ((1U << 0) & state))
                {
                    if (0 == memcmp(pLine, "Pid:", 4))
                    {
                        pLine += 4;

                        while (isspace(*pLine))
                        {
                            pLine++;
                        }

                        processId = atoi(pLine);
                        state ^= 1U << 0;
                        continue;
                    }
                }

                if (0U != ((1U << 1) & state))
                {
                    if (0 == memcmp(pLine, "PPid:", 5))
                    {
                        pLine += 5;

                        while (isspace(*pLine))
                        {
                            pLine++;
                        }

                        *pParentProcessId = atoi(pLine);
                        state ^= 1U << 1;
                        continue;
                    }
                }

                if (0U != ((1U << 2) & state))
                {
                    if (0 == memcmp(pLine, "Tgid:", 5))
                    {
                        pLine += 5;

                        while (isspace(*pLine))
                        {
                            pLine++;
                        }

                        *pGroupId = atoi(pLine);
                        state ^= 1U << 2;
                        continue;
                    }
                }

                if (0U != ((1U << 3) & state))
                {
                    if (0 == memcmp(pLine, "Name:", 5))
                    {
                        pLine += 5;

                        while (isspace(*pLine))
                        {
                            pLine++;
                        }

                        gtSize_t maxLen = *pNameLen;
                        *pNameLen = strlen(pLine);

                        if (*pNameLen < maxLen)
                        {
                            maxLen = *pNameLen + 1;
                        }

                        memcpy(pName, pLine, maxLen);
                        state ^= 1U << 3;
                    }
                }
            }

            ret = (0 == state);
        }
    }

    return ret;
}

bool osGetProcessExecutablePath(osProcessId processId, gtString& executablePath)
{
    executablePath.makeEmpty();

    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "/proc/%d/exe", processId);

    int len = static_cast<int>(readlink(buffer, buffer, sizeof(buffer)));

    if (0 < len)
    {
        buffer[len] = '\0';
        executablePath.fromASCIIString(buffer, len);
    }

    // If the error is ENOENT, then the process does not have an executable, and we should not fail.
    return 0 <= len || ENOENT == errno;
}

bool osGetProcessCommandLine(osProcessId processId, gtString& commandLine)
{
    commandLine.makeEmpty();

    char buffer[1024];

    snprintf(buffer, sizeof(buffer), "/proc/%u/cmdline", processId);
    int fd = open(buffer, O_RDONLY);

    if (fd == -1)
    {
        return false;
    }

    buffer[0] = '\0';

    unsigned n = 0;

    for (;;)
    {
        ssize_t r = read(fd, buffer + n, sizeof(buffer) - n);

        if (r == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }

            break;
        }

        n += r;

        if (n == sizeof(buffer))
        {
            break; // filled the buffer
        }

        if (r == 0)
        {
            break; // EOF
        }
    }

    close(fd);

    if (n)
    {
        int i;

        if (n == sizeof(buffer))
        {
            n--;
        }

        buffer[n] = '\0';

        i = n;

        while (i--)
        {
            int c = buffer[i];

            if (c < ' ' || c > '~')
            {
                buffer[i] = ' ';
            }
        }

        if ('\0' != buffer[0])
        {
            commandLine.fromASCIIString(buffer);
        }
    }

    return true;
}

bool osGetProcessWorkingDirectory(osProcessId processId, gtString& workDirectory)
{
    workDirectory.makeEmpty();

    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "/proc/%d/cwd", processId);

    int len = static_cast<int>(readlink(buffer, buffer, sizeof(buffer)));

    if (0 < len)
    {
        buffer[len] = '\0';
        workDirectory.fromASCIIString(buffer);
    }

    return 0 <= len;
}

bool osGetProcessPlatform(osProcessId processId, osRuntimePlatform& platform)
{
    char buffer[9600];
    snprintf(buffer, sizeof(buffer), "/proc/%d/maps", processId);

    FILE* pFile = fopen(buffer, "r");
    bool retVal = (pFile != nullptr);

    if (retVal)
    {
        platform = OS_NATIVE_PLATFORM;

        while (fgets(buffer, sizeof(buffer), pFile) != NULL)
        {
            size_t len = strlen(buffer);

            if (10 < len)
            {
                char* ptr = buffer + len - 1;

                if ('\n' != *ptr)
                {
                    ptr++;
                }

                if (0 == memcmp(ptr - 11, "/libjava.so", 11) || 0 == memcmp(ptr - 10, "/libjvm.so", 10))
                {
                    platform = OS_JAVA_PLATFORM;
                    break;
                }
            }
        }

        fclose(pFile);
    }
    else
    {
        platform = OS_UNKNOWN_PLATFORM;
    }

    return retVal;
}

bool osGetProcessArchitecture(const osFilePath& executablePath, osModuleArchitecture& arch)
{
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    arch = OS_X86_64_ARCHITECTURE;
#else
    arch = OS_I386_ARCHITECTURE;
#endif

    gtVector<osModuleArchitecture> archs;

    if (executablePath.isExecutable() && osGetModuleArchitectures(executablePath, archs) && !archs.empty())
    {
        osModuleArchitecture executableArch = archs.back();

        if (OS_I386_ARCHITECTURE == executableArch || OS_X86_64_ARCHITECTURE == executableArch)
        {
            arch = executableArch;
        }
    }

    return true;
}

bool osGetProcessType(osProcessId processId, osModuleArchitecture& arch, osRuntimePlatform& platform, bool setPrivilege)
{
    (void)(setPrivilege); // Unused

    gtString executablePath;
    osGetProcessExecutablePath(processId, executablePath);

    return osGetProcessArchitecture(executablePath, arch) &&
           osGetProcessPlatform(processId, platform);
}

bool osGetProcessLaunchInfo(osProcessId processId,
                            osModuleArchitecture& arch, osRuntimePlatform& platform,
                            gtString& executablePath, gtString& commandLine, gtString& workDirectory,
                            bool setPrivilege)
{
    (void)(setPrivilege); // Unused

    return osGetProcessWorkingDirectory(processId, workDirectory) &&
           osGetProcessExecutablePath(processId, executablePath) &&
           osGetProcessCommandLine(processId, commandLine) &&
           osGetProcessArchitecture(executablePath, arch) &&
           osGetProcessPlatform(processId, platform);
}

bool osIsProcessAttachable(osProcessId processId)
{
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "/proc/%d/cwd", processId);

    return (0 <= readlink(buffer, buffer, sizeof(buffer)));
}

osProcessesEnumerator::osProcessesEnumerator() : m_pEnumHandler(NULL)
{
}

osProcessesEnumerator::~osProcessesEnumerator()
{
    deinitialize();
}

bool osProcessesEnumerator::initialize()
{
    m_pEnumHandler = opendir("/proc");
    return (NULL != m_pEnumHandler);
}

void osProcessesEnumerator::deinitialize()
{
    if (NULL != m_pEnumHandler)
    {
        closedir(reinterpret_cast<DIR*>(m_pEnumHandler));
        m_pEnumHandler = NULL;
    }
}

bool osProcessesEnumerator::next(osProcessId& processId, gtString* pName)
{
    struct dirent entry, *pNext;
    bool ret = false;
    GT_IF_WITH_ASSERT(nullptr != pName)
     {               
	 while (0 == readdir_r(reinterpret_cast<DIR*>(m_pEnumHandler), &entry, &pNext) && NULL != pNext)
	   {
	       if (isdigit(*entry.d_name))
		{
		    processId = static_cast<gtUInt32>(strtoul(entry.d_name, NULL, 10));

		    char name[OS_MAX_PATH] = {};
		    gtSize_t maxLen = OS_MAX_PATH - 1;

		    if (osGetProcessIdentificationInfo(processId, name, &maxLen))
		    {
		       pName->fromUtf8String(name);		      
   	               ret = true;
		       break;
		    }
		}
	    }//while
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        osTerminateChildren
// Description: Terminates all of the processes which were spawned by the process
//              with pid processId
// Author:      AMD Developer Tools Team
// Date:        19/02/2013
// ---------------------------------------------------------------------------
OS_API bool osTerminateChildren(osProcessId parentProcessId, bool isGracefulShutdownRequired)
{
    bool retVal = false;
    std::vector<osProcessId> children;
    osProcessesEnumerator processEnum;

    if (processEnum.initialize())
    {
        osProcessId processId;
        gtString executableName;

        while (processEnum.next(processId, &executableName))
        {
            if (((osProcessId)0) == processId || parentProcessId == processId)
            {
                continue;
            }

            bool isChildProcess = osIsParent(parentProcessId, processId);
            if (isChildProcess)
            {
                children.push_back(processId);
            }
        }
        retVal = true;
    }

    for (auto childProcessId : children)
    {
        bool isSuccessfulTermination = osTerminateProcess(childProcessId, 0, true, isGracefulShutdownRequired);
        retVal = isSuccessfulTermination;
    }

    return retVal;
}

// This is the data structure that is being used
// to communicate with the spawned process.
struct popen2_data_t
{
    pid_t m_childPid;
    int   m_fromChildChannel;
    int   m_toChildChannel;
};

// ---------------------------------------------------------------------------
// Name:        popen2
// Description: This is an alternative implementation of popen() that provides the caller
//              with a bidirectional communication channel to the spawned process.
//              the caller to terminate the command's execution.
// Arguments:   Input: pCmd - the command to be invoked.
//              Output: childInfo - a struct containing the information about the child process
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        30/08/2015
// ---------------------------------------------------------------------------
bool popen2(const char* pCmd, popen2_data_t& childInfo)
{
    bool ret = false;
    pid_t p;
    int pipe_stdin[2], pipe_stdout[2];

    if (pCmd != nullptr && !pipe(pipe_stdin) && !pipe(pipe_stdout))
    {
        // Create the child process.
        p = fork();

        if (p > -1)
        {
            if (p == 0)
            {
                // This is the child process. Execute the command.
                close(pipe_stdin[1]);
                dup2(pipe_stdin[0], 0);
                close(pipe_stdout[0]);
                dup2(pipe_stdout[1], 1);

                // clear the codexl ld_library_path
                osRemoveRuntimeLibsPathFromLibraryPath();

                execl("/bin/sh", "sh", "-c", pCmd, 0);
                perror("execl");
                exit(99);
            }

            // Set the output.
            childInfo.m_childPid = p;
            childInfo.m_toChildChannel = pipe_stdin[1];
            childInfo.m_fromChildChannel = pipe_stdout[0];

            // We are done.
            ret = true;
        }
    }

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        osExecAndGrabOutput
// Description: Executes the in a different process and captures its output.
//              This routine blocks, but, using the cancelSignal flag, it allows
//              the caller to terminate the command's execution.
// Arguments:   cmd - The command to be executed.
//              cancelSignal - A reference to the cancel flag. Upon calling this
//              routine, the cancelSignal flag should be set to false. In case that
//              the caller wants to terminate the commands' execution, this flag
//              should be set to true.
//              cmdOutput - an output parameter to hold the command's output.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        30/08/2015
// ---------------------------------------------------------------------------
bool osExecAndGrabOutput(const char* cmd, const bool& cancelSignal, gtString& cmdOutput)
{
    // The default buffer size.
    const size_t BUFF_SIZE = 65536;
    bool ret = false;

    // Clear the output buffer.
    cmdOutput.makeEmpty();

    if (cmd != nullptr)
    {
        // Log the launch command to the CodeXL log file.
        if (osDebugLog::instance().loggedSeverity() >= OS_DEBUG_LOG_DEBUG)
        {
            gtString logCommand;
            logCommand.fromASCIIString(cmd);
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG,
                                       L"Launching command: %ls",
                                       logCommand.asCharArray());
        }

        // Launch the command.
        popen2_data_t procData;

        if (popen2(cmd, procData))
        {
            // Prepare the temporary buffer.
            char tmpBuffer[BUFF_SIZE];
            memset(tmpBuffer, 0, BUFF_SIZE);

            while (true)
            {
                // The timeout interval in milliseconds.
                unsigned SLEEP_INTERVAL_MS = 50;

                // Check the status of the CLI with a timeout. Note that the busy-waiting
                // period is already mitigated by the callee's sleep period, so we can leave
                // the loop as is.
                if (cancelSignal || osWaitForProcessToTerminate(procData.m_childPid, SLEEP_INTERVAL_MS))
                {
                    // If the the build cancel signal was set, or the CLI has finished
                    // running (or we have an error waiting), stop waiting.
                    break;
                }
            }

            if (cancelSignal)
            {
                osTerminateProcess(procData.m_childPid, 0);
            }
            else
            {
                // Grab the command's output.
                // Set this operation to be non-blocking in case that we have no
                // pending data.
                fcntl(procData.m_fromChildChannel, F_SETFL, O_NONBLOCK);
                ret = (read(procData.m_fromChildChannel, tmpBuffer, BUFF_SIZE) != -1);

                if (!ret)
                {
                    gtString errMsg(L"Failed to read output");
                    OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
                }
                else
                {
                    // Assign the output buffer.
                    cmdOutput << tmpBuffer;

                    // Set the return value.
                    ret = !cmdOutput.isEmpty();
                }
            }

            // Close the child's output stream handle.
            close(procData.m_fromChildChannel);
        }
    }

    return ret;
}

OS_API bool osIsParent(osProcessId parentProcessId, osProcessId processId)
{
    osProcessId originalParentProcessId;

    bool retVal = false;

    bool rc = osGetProcessIdentificationInfo(processId, &originalParentProcessId);

    while (rc && originalParentProcessId != 0)
    {
	    if (originalParentProcessId == parentProcessId)
        {
            retVal = true;
            break;
	    }
        processId = originalParentProcessId;
        rc = osGetProcessIdentificationInfo(processId, &originalParentProcessId);
    }

    return retVal;
}
