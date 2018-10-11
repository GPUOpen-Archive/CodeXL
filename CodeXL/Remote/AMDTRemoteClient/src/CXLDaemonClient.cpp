//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CXLDaemonClient.cpp
///
//==================================================================================

// Local.
#include "CXLDaemonClient.h"
#include "RemoteClientUtils.h"

// Remote agent definitions.
#include <AMDTRemoteAgent/Public Include/dmnStringConstants.h>

// Infra.
#include <AMDTOSWrappers/Include/osMutex.h>
#include <AMDTOSWrappers/Include/osMutexLocker.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osDirectory.h>

// For RDS's definitions.
#include <AMDTRemoteDebuggingServer/Include/rdStringConstants.h>

// C++.
#include <sstream>
#include <algorithm>

// Required by zlib to link properly.
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define ZLIB_WINAPI
#endif

#include <zlib.h>


#define PLATFORM_LINUX L"Linux"
#define PLATFROM_WINDOWS L"Windows"
// STATIC STORAGE - START.

// True if we need a new impl.
static bool gs_IsDirty = false;
static bool gs_IsInitialized = false;
static long gs_ReadTimeout = 0;
static osPortAddress gs_DaemonAddress;
static CXLDaemonClient* gs_Instance = nullptr;
static osMutex gs_InstanceLock;

// TCP KeepAlive settings (relevant only to Windows OS).
// For more details, consult the osTCPSocket::setKeepAlive() function documentation.
const unsigned long TCP_KEEPALIVE_TIMEOUT  = 1000;
const unsigned long TCP_KEEPALIVE_INTERVAL = 1000;

static bool IsDifferentAddress(const osPortAddress& first, const osPortAddress& second)
{
    gtString a = L"";
    gtString b = L"";
    first.toString(a);
    second.toString(b);

    return (0 != a.compareNoCase(b));
}

// STATIC STORAGE - END.

class CXLDaemonClient::Impl
{
public:
    Impl(long readTimeout) : m_readTimeout(readTimeout), m_isConnected(false), m_tcpClient(), m_asyncTasks()
    {
    }

    ~Impl()
    {
        bool isOk = m_tcpClient.close();
        GT_ASSERT_EX(isOk, L"Closing TCP client.");
        CleanAllocatedTasks();
    }

    bool ConnectToDaemon(osPortAddress& connectionPortBuffer)
    {
        bool ret = true;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(ret);

        // First, sanity check.
        if (!m_isConnected)
        {

            // Recreate our tcp client.
            if (m_tcpClient.isOpen())
            {
                m_tcpClient.close();
            }

            // Set the socket client to force immediate resolution of DNSes:
            m_tcpClient.setBlockOnDNS(true);

            ret = m_tcpClient.open();
            GT_IF_WITH_ASSERT(ret)
            {
                // Set read timeout for the tcp client.
                m_tcpClient.setReadOperationTimeOut(m_readTimeout);

                // Set the TCP keep alive setting.
                ret = m_tcpClient.setKeepAlive(TCP_KEEPALIVE_TIMEOUT, TCP_KEEPALIVE_INTERVAL);
                GT_ASSERT_EX(ret, L"DMN Client: Turning on TCP Keep Alive for the client socket.");

                m_tcpClient.setSingleThreadAccess(true);
                // Connect to the daemon.
                ret = m_tcpClient.connect(gs_DaemonAddress);
                GT_IF_WITH_ASSERT(ret)
                {
                    // Assign the buffer with the connection details.
                    ret = m_tcpClient.getCurrentAddress(connectionPortBuffer);
                    GT_ASSERT_EX(ret, L"DMN Client: Unable to retrieve current address.")
                }
            }
            m_isConnected = ret;
        }
        else
        {
            OS_OUTPUT_DEBUG_LOG(L"Client already connected to daemon.", OS_DEBUG_LOG_DEBUG);
        }

        return ret;
    }

    static DaemonOpCode OpModeToLaunchOpCode(REMOTE_OPERATION_MODE mode)
    {
        switch (mode)
        {
            case romDEBUG:
                return docLaunchRds;

            case romPROFILE:
                return docLaunchProfiler;

            case romGRAPHICS:
                return docLaunchGraphicsBeckendServer;

            default:
                return docUnknown;
        }
    }

    static DaemonOpCode OpModeToTerminationOpCode(REMOTE_OPERATION_MODE mode)
    {
        switch (mode)
        {
            case romDEBUG:
                return docTerminateDebuggingSession;

            case romPROFILE:
                return docTerminateProfilingSession;

            case romGRAPHICS:
                return docTerminateGraphicsBeckendServerSession;

            default:
                return docUnknown;
        }
    }

    static bool IsFileExists(const gtString& fullPathFileName)
    {
        bool ret = false;

        if (!fullPathFileName.isEmpty())
        {
            osFilePath filePath(fullPathFileName);
            ret = filePath.exists();
        }

        return ret;
    }


    bool LaunchRdsHelper(const gtString& cmdLineArgs, const std::vector<osEnvironmentVariable>& envVars)
    {
        bool ret = false;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(ret);

        // Sanity.
        GT_IF_WITH_ASSERT(m_isConnected)
        {
            gtInt32 opCode = OpModeToLaunchOpCode(romDEBUG);
            ret = (opCode != romUNKNOWN);
            GT_IF_WITH_ASSERT(ret)
            {
                // Send the opcode.
                m_tcpClient << opCode;

                // Verify.
                gtInt32 opStatus = dosFailure;
                m_tcpClient >> opStatus;
                ret = (opStatus == dosSuccess);

                GT_IF_WITH_ASSERT(ret)
                {
                    // Send the command line args.
                    ret = m_tcpClient.writeString(cmdLineArgs);
                    GT_ASSERT_EX(ret, L"Writing a cmd line args to the daemon.");

                    if (ret)
                    {
                        // Verify.
                        m_tcpClient >> opStatus;
                        ret = (opStatus == dosSuccess);
                        GT_IF_WITH_ASSERT(ret)
                        {

                            // Send the number of environment variables.
                            gtInt32 envVarsCount = envVars.size();
                            m_tcpClient << envVarsCount;

                            // Verify.
                            m_tcpClient >> opStatus;
                            ret = (opStatus == dosSuccess);
                            GT_IF_WITH_ASSERT(ret)
                            {
                                bool isFailure = false;

                                // Send the environment variables pairs.
                                for (int i = 0; i < envVarsCount; i++)
                                {
                                    if (! m_tcpClient.writeString(envVars[i]._name))
                                    {
                                        OS_OUTPUT_DEBUG_LOG(L"Failed passing env var key.", OS_DEBUG_LOG_ERROR);
                                        isFailure = true;
                                    }

                                    if (!m_tcpClient.writeString(envVars[i]._value))
                                    {
                                        OS_OUTPUT_DEBUG_LOG(L"Failed passing env var value.", OS_DEBUG_LOG_ERROR);
                                        isFailure = true;
                                    }
                                }

                                ret = !isFailure;
                                GT_IF_WITH_ASSERT(ret)
                                {

                                    // Verify.
                                    m_tcpClient >> opStatus;
                                    ret = (opStatus == dosSuccess);
                                    GT_ASSERT_EX(ret, L"On receive of env vars ACK.");

                                    if (ret)
                                    {
                                        // Verify.
                                        m_tcpClient >> opStatus;
                                        GT_ASSERT_EX(opStatus == dosSuccess, L"On RDS process launch ACK.");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return ret;
    }

    bool LaunchRDS(const gtString& cmdLineArgs, const std::vector<osEnvironmentVariable>& envVars)
    {
        // Launch the relevant process.
        return LaunchRdsHelper(cmdLineArgs, envVars);
    }

    bool SendIsAliveOpCode(REMOTE_OPERATION_MODE mode)
    {
        bool ret = true;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(ret);

        if (mode == romDEBUG)
        {
            m_tcpClient << docDebuggingSessionStatusQuery;
        }
        else if (mode == romPROFILE)
        {
            m_tcpClient << docProfilingSessionStatusQuery;
        }
        else
        {
            OS_OUTPUT_DEBUG_LOG(L"Unknown operation mode.", OS_DEBUG_LOG_ERROR);
            ret = false;
        }

        return ret;
    }

    static bool IsValidSessionStatus(gtInt32 val)
    {
        return (val >= dssUknown && val <= dssTerminated);
    }

    bool GetSessionStatus(REMOTE_OPERATION_MODE mode, DaemonSessionStatus& buffer)
    {
        bool ret = false;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(ret);

        buffer = dssUknown;
        GT_IF_WITH_ASSERT(m_isConnected)
        {
            ret = SendIsAliveOpCode(mode);
            GT_IF_WITH_ASSERT(ret)
            {
                gtInt32 opStatus = dosFailure;

                // Verify.
                m_tcpClient >> opStatus;
                ret = (opStatus == dosSuccess);
                GT_ASSERT_EX(ret, L"On debugging session status query ACK.");

                if (ret)
                {
                    // Check session status.
                    gtInt32 sessionStatus = dssUknown;
                    m_tcpClient >> sessionStatus;

                    ret = IsValidSessionStatus(sessionStatus);
                    GT_ASSERT_EX(ret, L"Invalid session status value.");

                    if (ret)
                    {
                        buffer = static_cast<DaemonSessionStatus>(sessionStatus);
                    }
                }
            }
        }
        return ret;
    }

    bool GetRemoteFile(const gtString& remoteFileName, const gtString& localTargetFileName)
    {
        bool ret = false;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(ret);

        if ((!remoteFileName.isEmpty()) && (!localTargetFileName.isEmpty()))
        {
            GT_IF_WITH_ASSERT(m_isConnected)
            {
                // File transfer.
                m_tcpClient << docGetRemoteFile;

                // Verify.
                gtInt32 opStatus = dosFailure;
                m_tcpClient >> opStatus;
                ret = (opStatus == dosSuccess);
                GT_ASSERT_EX(ret, L"On file transfer (sending) - status query ACK.");

                if (ret)
                {
                    ret = m_tcpClient.writeString(remoteFileName);

                    GT_IF_WITH_ASSERT(ret)
                    {
                        // Verify the daemon received the file name.
                        m_tcpClient >> opStatus;

                        ret = (opStatus == dosSuccess);
                        GT_ASSERT_EX(ret, L"On file transfer file name received ACK.");

                        if (ret)
                        {
                            osRawMemoryBuffer tmpBuffer;
                            ret = tmpBuffer.readSelfFromChannel(m_tcpClient);
                            GT_IF_WITH_ASSERT(ret)
                            {
                                osFilePath outputFilePath(localTargetFileName);
                                ret = tmpBuffer.toFile(outputFilePath);

                                GT_IF_WITH_ASSERT(ret)
                                {
                                    OS_OUTPUT_DEBUG_LOG(L"DMN Client: A file was successfully received and written to disk.", OS_DEBUG_LOG_INFO);

                                    // Receive ack.
                                    m_tcpClient >> opStatus;
                                    ret = (opStatus == dosSuccess);
                                    GT_ASSERT_EX(ret, L"On file transfer file successfully sent ACK.");
                                }
                            }
                        }
                    }
                }
            }
        }

        return ret;
    }

    bool SendRemoteFile(gtInt32 dmnFileType, const gtString& remoteFileName)
    {
        bool ret = false;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(ret);

        GT_IF_WITH_ASSERT(m_isConnected)
        {
            ret = true;

            // Send the file type.
            m_tcpClient << dmnFileType;

            if (dmnFileType != dftMissingFile)
            {
                gtString tokenizedFileName = remoteFileName;
                // First tokenize the local path string.
                ret =  TokenizePath(tokenizedFileName);
                GT_ASSERT_EX(ret, L"DMN: Failed plenting tokens in remote file name.");

                if (ret)
                {
                    // Send the file name.
                    ret = m_tcpClient.writeString(tokenizedFileName);

                    // Verify.
                    gtInt32 opStatus = dosFailure;
                    m_tcpClient >> opStatus;
                    ret = (opStatus == dosSuccess);

                    // Send the file.
                    opStatus = dosFailure;
                    osFilePath filePath(remoteFileName);
                    osRawMemoryBuffer buffer;
                    ret = buffer.fromFile(filePath);
                    GT_IF_WITH_ASSERT(ret)
                    {
                        ret = buffer.writeSelfIntoChannel(m_tcpClient);
                    }

                    // Verify.
                    m_tcpClient >> opStatus;
                    ret = (opStatus == dosSuccess);
                    GT_ASSERT_EX(ret, L"On file transfer (sending) - final status query ACK.");

                }
            }
        }
        return ret;
    }

    // This functionality is already available in AMDTApplicationFramework.dll, but since its
    // so simple and limited, we chose to reimplement this functionality locally to elimintae
    // the potential dependency between AMDTRemoteClient and AMDTApplicationFramework.
    bool GetCurrentUserDataFolder(osFilePath& buffer)
    {
        bool ret = buffer.setPath(osFilePath::OS_USER_APPLICATION_DATA);
        GT_IF_WITH_ASSERT(ret)
        {
            buffer.appendSubDirectory(DWM_STR_CODEXL_DIR_NAME);
        }
        return ret;
    }

    bool TokenizePath(gtString& strToFix)
    {
        bool ret = strToFix.isEmpty();

        if (!ret)
        {
            // Should be read from dmnDefinitions.h.
            const gtString PATH_TOKEN = L"$$$";

            osFilePath pathToReplace;
            ret = GetCurrentUserDataFolder(pathToReplace);
            GT_IF_WITH_ASSERT(ret && (strToFix.length() > PATH_TOKEN.length()))
            {
                gtString pathToReplaceStr = pathToReplace.asString();
#ifdef WIN32
                pathToReplaceStr.replace(0, pathToReplaceStr.length() - 1, L"\\", L"/", true);
#endif
                strToFix.replace(0, strToFix.length() - 1, pathToReplaceStr, PATH_TOKEN, true);
            }
        }

        return ret;
    }


    bool FixLocalFilePaths(const gtString& cmdLineArgs, gtString& fixedCmdLineArgs)
    {
        bool ret = false;

        // This should be transferred to dmnDefinitions.h to be shared with the daemon.
        const gtString PATH_TOKEN = L"$$$";

        // This is the part which we should not edit.
        int suffixBeginningPos = cmdLineArgs.reverseFind(L"-w ");
        GT_IF_WITH_ASSERT(suffixBeginningPos > 0)
        {
            cmdLineArgs.getSubString(0, suffixBeginningPos, fixedCmdLineArgs);
            gtString suffix;
            cmdLineArgs.getSubString(suffixBeginningPos + 1, cmdLineArgs.length() - 1, suffix);

            if (!fixedCmdLineArgs.isEmpty() && !suffix.isEmpty())
            {
                ret = TokenizePath(fixedCmdLineArgs);
                GT_IF_WITH_ASSERT(ret)
                {
                    // Now, append the suffix.
                    fixedCmdLineArgs.append(suffix);
                    ret = true;
                }
            }
        }
        return ret;
    }

    bool DecompressFile(const osFilePath& originalFile, gtInt32 originalFileSize)
    {
        bool ret = false;

        // First read compressed file.
        osFile compressedFile(originalFile);
        ret = compressedFile.open(osChannel::OS_BINARY_CHANNEL);
        GT_ASSERT_EX(ret, L"DMN CLIENT: Failed to open received compressed file.");

        if (ret)
        {
            unsigned long compressedFileSize = 0;
            ret = compressedFile.getSize(compressedFileSize);
            GT_ASSERT_EX(ret, L"DMN CLIENT: Failed to get compressed file's size.");

            if (ret)
            {
                gtByte* dcmpBuffer = new(std::nothrow) gtByte[compressedFileSize];

                gtSize_t dcmpAmountRead = 0;
                compressedFile.readAvailableData(dcmpBuffer, compressedFileSize, dcmpAmountRead);

                // Close and delete the compressed file.
                compressedFile.close();
                GT_ASSERT_EX(compressedFile.deleteFile(), L"Deleting compressed file.");

                // IMPORTANT: Note that we are using the ORIGINAL data size.
                gtByte* restoredFileBuffer = new(std::nothrow) gtByte[originalFileSize];

                unsigned long _originalFileSize = originalFileSize;
                int res = uncompress((unsigned char*)restoredFileBuffer,
                                     &_originalFileSize, (unsigned char*)dcmpBuffer, dcmpAmountRead);
                ret = (res == Z_OK);
                GT_ASSERT_EX(ret, L"DMN CLIENT: Failed to decompress file on compression proxy.");

                if (ret)
                {
                    // Now save it to disk so that we would be able to verify the correctness.
                    osFilePath restoredFilePath = originalFile;
                    gtString restoredFileName;
                    originalFile.getFileName(restoredFileName);

                    ret = (-1 != restoredFileName.find(L"_zipped") &&
                           -1 != restoredFileName.replace(0, restoredFileName.length() - 1, L"_zipped", L""));
                    GT_ASSERT_EX(ret, L"DMN CLIENT: Invalid compressed file name.");

                    if (ret)
                    {
                        restoredFilePath.setFileName(restoredFileName);
                        osFile restoredFile;
                        ret = restoredFile.open(restoredFilePath, osChannel::OS_BINARY_CHANNEL, osFile::OS_OPEN_TO_WRITE);
                        GT_ASSERT_EX(ret, L"DMN CLIENT: Failed to create decompressed file on disk.");

                        if (ret)
                        {
                            ret = restoredFile.write(restoredFileBuffer, _originalFileSize);
                            GT_ASSERT_EX(ret, L"DMN CLIENT: Failed to write decompressed file to disk.");
                            restoredFile.close();
                        }
                    }
                }

                // Clean.
                delete[] restoredFileBuffer;
                delete[] dcmpBuffer;
            }
        }

        return ret;
    }


    bool ReceiveRemoteFile(const gtString& whereToSave, bool isCompressed = true)
    {
        bool ret = false;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(ret);

        gtString sprofOutFileName;
        ret = m_tcpClient.readString(sprofOutFileName);
        GT_ASSERT_EX(ret, L"DMN Client: Failed reading rcprof output file name.");

        // Now get the original file size.
        gtInt32 originalFileSize = 0;
        m_tcpClient >> originalFileSize;
        ret = (originalFileSize > 0);

        // Report back.
        gtInt32 report = (ret) ? dosSuccess : dosFailure;
        m_tcpClient << report;

        if (ret)
        {
            // Now read the file.
            osRawMemoryBuffer tmpBuffer;
            ret = tmpBuffer.readSelfFromChannel(m_tcpClient);

            // Report back whether the file was successfully received.
            report = (ret) ? dosSuccess : dosFailure;
            m_tcpClient << report;

            GT_IF_WITH_ASSERT(ret)
            {
                osFilePath outputFilePath;
                outputFilePath.setFileDirectory(whereToSave);
                outputFilePath.setFileName(sprofOutFileName);
                ret = tmpBuffer.toFile(outputFilePath);
                GT_ASSERT_EX(ret, L"DMN Client: Failed to receive rcprof output file.");

                // Now check if we need to decompress the file.
                if (isCompressed)
                {
                    ret = DecompressFile(outputFilePath, originalFileSize);
                    GT_ASSERT_EX(ret, L"DMN Client: Failed to decompress received file..");
                }
            }
        }

        return ret;
    }

    // Extracts the target application part from rcprof's command line arguments.
    static void ExtractGpuProfilerTargetApp(const gtString& fixedCmdLineArgs, gtString& targetApp)
    {
        int beginIndex = 0;
        int endIndex = 0;

        endIndex = fixedCmdLineArgs.reverseFind(L"\"", -1);

        if (endIndex != -1)
        {
            endIndex--;
        }

        beginIndex = fixedCmdLineArgs.reverseFind(L"\"", endIndex);

        if (beginIndex != -1)
        {
            beginIndex++;
        }

        if (beginIndex < endIndex)
        {
            fixedCmdLineArgs.getSubString(beginIndex, endIndex, targetApp);
        }
    }

    bool LaunchGpuProfiler(const gtString& cmdLineArgs, const gtString& localGpuProfilerBaseDir,
                           const gtString& counterFileName, const gtString& envVarsFileName,
                           const gtString& apiFilterFileName, const gtString& apiRulesFileName,
                           const gtVector<gtString>& specificKernels, RemoteClientError& errorCode)
    {
        const unsigned DEFAULT_TIMEOUT_MS = 15000;
        errorCode = rceUnknown;

        bool ret = false;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(ret);
        // Sanity.
        GT_IF_WITH_ASSERT(m_isConnected)
        {
            // Send the opcode.
            gtInt32 opCode = OpModeToLaunchOpCode(romPROFILE);
            ret = (opCode != romUNKNOWN);
            GT_IF_WITH_ASSERT(ret)
            {
                gtString fixedCmdLineArgs;
                ret = FixLocalFilePaths(cmdLineArgs, fixedCmdLineArgs);
                GT_ASSERT_EX(ret, L"DMN: Fixing local paths in rcprof params.");

                if (ret)
                {

                    // Send the opcode.
                    m_tcpClient << opCode;

                    // Verify.
                    gtInt32 opStatus = dosFailure;
                    m_tcpClient >> opStatus;
                    ret = (opStatus == dosSuccess);
                    GT_ASSERT_EX(ret, L"OpCode receive ack.");

                    if (ret)
                    {
                        // Verify that the target application exists.
                        gtString targetApp;
                        bool isTargetAppExists = false;
                        ExtractGpuProfilerTargetApp(fixedCmdLineArgs, targetApp);

                        // Send target application's name.
                        m_tcpClient << targetApp;

                        // Verify.
                        m_tcpClient >> isTargetAppExists;
                        ret = isTargetAppExists;

                        if (isTargetAppExists)
                        {
                            // Send the command line args.
                            ret = m_tcpClient.writeString(fixedCmdLineArgs);
                            GT_ASSERT_EX(ret, L"Transferring rcprof cmd line string.");

                            // Verify.
                            opStatus = dosFailure;
                            m_tcpClient >> opStatus;
                            ret = (opStatus == dosSuccess);

                            if (ret)
                            {
                                // Now send the 4 files in the following order:
                                // 1. Counter file.
                                // 2. Env vars file.
                                // 3. API filter file.
                                // 4. API rules file.
                                gtInt32 counterFileType = (IsFileExists(counterFileName) ? dftProfilerCounterFile : dftMissingFile);
                                gtInt32 envVarsFileType = (IsFileExists(envVarsFileName) ? dftProfilerEnvVarsFile : dftMissingFile);
                                gtInt32 apiFiltersFileType = (IsFileExists(apiFilterFileName) ? dftProfilerApiFiltersFile : dftMissingFile);
                                gtInt32 apiRulesFileType = (IsFileExists(apiRulesFileName) ? dftProfilerApiRulesFile : dftMissingFile);

                                ret = ret &&  SendRemoteFile(counterFileType, counterFileName);
                                GT_ASSERT_EX(ret, L"DMN Client: Failed sending rcprof counters file.");
                                ret = SendRemoteFile(envVarsFileType, envVarsFileName);
                                GT_ASSERT_EX(ret, L"DMN Client: Failed sending rcprof env vars file.");
                                ret = ret && SendRemoteFile(apiFiltersFileType, apiFilterFileName);
                                GT_ASSERT_EX(ret, L"DMN Client: Failed sending rcprof api filters file.");
                                ret = ret && SendRemoteFile(apiRulesFileType, apiRulesFileName);
                                GT_ASSERT_EX(ret, L"DMN Client: Failed sending rcprof api rules file.");

                                if (ret)
                                {
                                    // Verify.
                                    opStatus = dosFailure;
                                    m_tcpClient >> opStatus;
                                    ret = (opStatus == dosSuccess);
                                    GT_ASSERT_EX(ret, L"DMN Cilent: verifying rcprof files arrival to dmn.");

                                    // Tell the agent if we are in kernel-specific mode.
                                    bool isKernelSpecific = !specificKernels.empty();
                                    m_tcpClient << isKernelSpecific;

                                    if (isKernelSpecific)
                                    {
                                        // Transfer the number of specific kernels.
                                        gtUInt32 numOfKernels = specificKernels.size();
                                        m_tcpClient << numOfKernels;

                                        // Transfer the names of the kernels to be profiled.
                                        for (const gtString& kernelName : specificKernels)
                                        {
                                            m_tcpClient << kernelName;
                                        }
                                    }

                                    if (ret)
                                    {
                                        // Now set the timeout to maximum.
                                        m_tcpClient.setReadOperationTimeOut(LONG_MAX);

                                        // Verify launch success.
                                        opStatus = dosFailure;
                                        m_tcpClient >> opStatus;
                                        ret = (opStatus == dosSuccess);

                                        if (ret)
                                        {
                                            GT_ASSERT_EX(ret, L"DMN Cilent: verifying rcprof remote process launch succeeded.");

                                            // Wait for the number of output files.
                                            gtInt32 sprofOutputFilesCount = 0;
                                            m_tcpClient >> sprofOutputFilesCount;
                                            ret = (sprofOutputFilesCount > 0);
                                            GT_ASSERT_EX(ret, L"DMN Client: Non-positive number of rcprof output files.");

                                            // Report back whether the number of files was received successfully.
                                            gtInt32 report = (ret) ? dosSuccess : dosFailure;
                                            m_tcpClient << report;

                                            for (gtInt32 i = 0; i < sprofOutputFilesCount; i++)
                                            {
                                                ret = ReceiveRemoteFile(localGpuProfilerBaseDir);
                                                GT_ASSERT_EX(ret, L"DMN Client: Receiving remote file.");
                                            }

                                            // Reset the timeout to 15[sec].
                                            m_tcpClient.setReadOperationTimeOut(DEFAULT_TIMEOUT_MS);
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            errorCode = rceTargetAppNotFound;
                        }
                    }
                }
            }
        }

        if (!ret)
        {
            // Indicates that we need to regenerate the connection next time.
            SetDirtyConnection();
        }

        return ret;

    }
    bool LaunchGraphicsBeckendServer(const osFilePath& serverPath, const gtString& cmdLineArgs, const osDirectory& workDirectory, RemoteClientError& errorCode)
    {
        //const unsigned DEFAULT_TIMEOUT_MS = 15000;
        errorCode = rceUnknown;

        bool ret = false;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(ret);
        // Sanity.
        GT_IF_WITH_ASSERT(m_isConnected)
        {
            // Send the opcode.
            gtInt32 opCode = OpModeToLaunchOpCode(romGRAPHICS);
            ret = (opCode != romUNKNOWN);
            GT_IF_WITH_ASSERT(ret)
            {
                gtString fixedCmdLineArgs = cmdLineArgs;
                ret = TokenizePath(fixedCmdLineArgs);

                // Send the opcode.
                m_tcpClient << opCode;

                // Verify.
                gtInt32 opStatus = dosFailure;
                m_tcpClient >> opStatus;
                ret = (opStatus == dosSuccess);
                GT_ASSERT_EX(ret, L"OpCode receive ack.");

                if (ret)
                {
                    // 1. Send target application
                    m_tcpClient << serverPath.asString();

                    bool isServerUp = false;
                    m_tcpClient >> isServerUp;

                    if (isServerUp == false)
                    {
                        // Verify.
                        bool isTargetAppExists = true;
                        m_tcpClient >> isTargetAppExists;
                        ret = isTargetAppExists;

                        if (isTargetAppExists)
                        {
                            // 2. Send the command line args.
                            ret = m_tcpClient.writeString(cmdLineArgs);
                            GT_ASSERT_EX(ret, L"Transferring PerfStudio cmd line string.");

                            // Verify.
                            opStatus = dosFailure;
                            m_tcpClient >> opStatus;
                            ret = (opStatus == dosSuccess);

                            bool isPortAvailable = true;
                            m_tcpClient >> isPortAvailable;

                            if (isPortAvailable)
                            {

                                // 3. Send the working directory
                                ret = m_tcpClient.writeString(workDirectory.asString());
                                GT_ASSERT_EX(ret, L"Transferring PerfStudio work directory.");
                            }
                            else
                            {
                                errorCode = rcePortUnavailable;
                            }

                            // Verify.
                            opStatus = dosFailure;
                            m_tcpClient >> opStatus;
                            ret = (opStatus == dosSuccess);
                        }
                        else
                        {
                            errorCode = rceTargetAppNotFound;
                            // Verify.
                            opStatus = dosFailure;
                            m_tcpClient >> opStatus;
                            ret = (opStatus == dosSuccess);
                        }
                    }//serverUp == false
                    else
                    {
                        errorCode = rceTargetAppIsAlreadyRunning;
                        // Verify.
                        opStatus = dosFailure;
                        m_tcpClient >> opStatus;
                        ret = (opStatus == dosSuccess);
                    }


                }
            }
        }

        if (!ret)
        {
            // Indicates that we need to regenerate the connection next time.
            SetDirtyConnection();
        }

        return ret;

    }

    bool GetCapturedFrames(const gtString& projectName, gtString& capturedFramesXmlStr, RemoteClientError& errorCode)
    {
        bool retVal = false;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

        errorCode = rceUnknown;
        m_tcpClient << docGetCapturedFrames;

        // Verify.
        gtInt32 opStatus = dosFailure;
        m_tcpClient >> opStatus;
        retVal = (opStatus == dosSuccess);
        GT_ASSERT_EX(retVal, L"OpCode receive ack.");

        if (retVal)
        {
            // Send project name
            m_tcpClient << projectName;

            // Verify.
            opStatus = dosFailure;
            m_tcpClient >> opStatus;
            retVal = (opStatus == dosSuccess);
            GT_ASSERT_EX(retVal, L"OpCode receive ack.");

            if (retVal)
            {
                m_tcpClient >> capturedFramesXmlStr;
                GT_ASSERT(capturedFramesXmlStr.isEmpty() == false);
                errorCode = rceNoError;
            }
        }

        return retVal;
    }
    bool GetCapturedFrames(const gtString& projectName, const gtString& sessionName, const osTime* pCapturedMinimalTime, gtString& capturedFramesXmlStr, RemoteClientError& errorCode)
    {
        bool retVal = false;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

        gtInt64 timeSince1970 = 0;
        GT_IF_WITH_ASSERT(pCapturedMinimalTime != nullptr)
        {
            timeSince1970 = pCapturedMinimalTime->secondsFrom1970();
        }

        errorCode = rceUnknown;
        m_tcpClient << docGetCapturedFramesByTime;

        gtInt32 opStatus = dosFailure;
        m_tcpClient >> opStatus;
        retVal = (opStatus == dosSuccess);
        GT_ASSERT_EX(retVal, L"OpCode receive ack.");

        // Send project name
        m_tcpClient << projectName;

        // Send session name name
        m_tcpClient << sessionName;

        // Send minimal frame time
        m_tcpClient << timeSince1970;

        // Read the captured frames XML
        m_tcpClient >> capturedFramesXmlStr;
        GT_IF_WITH_ASSERT(!capturedFramesXmlStr.isEmpty())
        {
            errorCode = rceNoError;
        }

        return retVal;
    }

    bool GetCapturedFrameData(const gtString& projectName, const gtString& sessionName, const int frameIndex, FrameInfo& frameData, RemoteClientError& errorCode)
    {
        bool retVal = false;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

        errorCode = rceUnknown;
        m_tcpClient << docCapturedFrameData;
        // Verify.
        gtInt32 opStatus = dosFailure;
        m_tcpClient >> opStatus;
        retVal = (opStatus == dosSuccess);
        GT_ASSERT_EX(retVal, L"OpCode receive ack.");

        // Send project name
        m_tcpClient << projectName;

        // Send session name
        m_tcpClient << sessionName;

        // Send frame index
        m_tcpClient << frameIndex;

        // Verify that data was received by the remote agent
        opStatus = dosFailure;
        m_tcpClient >> opStatus;
        retVal = (opStatus == dosSuccess);

        if (retVal)
        {
            // Read all frame analysis files content and set it into the frame info structure
            retVal = ReadFrameFilesData(frameData);

            if (retVal)
            {
                errorCode = rceNoError;
                retVal = true;
            }
        }

        return retVal;
    }

    bool DeleteFrameAnalysisSession(const gtString& projectName, const gtString& sessionName, RemoteClientError& errorCode)
    {
        bool retVal = false;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);
        errorCode = rceUnknown;
        m_tcpClient << docDeleteFrameAnalysisSession;

        gtInt32 opStatus = dosFailure;
        m_tcpClient >> opStatus;
        retVal = (opStatus == dosSuccess);
        GT_ASSERT_EX(retVal, L"OpCode receive ack.");

        // Send project name
        m_tcpClient << projectName;

        // Send session name
        m_tcpClient << sessionName;

        // Verify that data was received by the remote agent
        opStatus = dosFailure;
        m_tcpClient >> opStatus;
        retVal = (opStatus == dosSuccess);

        return retVal;
    }

    // Resets the state of the current object.
    void SetDirtyConnection()
    {
        gs_IsInitialized = m_isConnected = false;
    }

    bool TerminateRemoteProcess(REMOTE_OPERATION_MODE mode)
    {
        bool ret = false;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(ret);

        // Retrieve the relevant OpCode.
        gtInt32 opCode = OpModeToTerminationOpCode(mode);
        GT_IF_WITH_ASSERT(opCode != docUnknown)
        {
            // Send the opcode.
            m_tcpClient << opCode;

            // Verify submission.
            gtInt32 opStatus = dosFailure;
            m_tcpClient >> opStatus;
            ret = (opStatus == dosSuccess);
            GT_ASSERT_EX(ret, L"Terminate remote process opcode submission.");

            // Verify termination.
            m_tcpClient >> opStatus;
            ret = (opStatus == dosSuccess);
            if (!ret)
            {
                OS_OUTPUT_DEBUG_LOG(L"CodeXLRemoteAgent reported failure to terminate a remote process. Note: in case of Graphics Server it may have already shutdown due to separate command.", OS_DEBUG_LOG_ERROR);
            }
            
        }

        return ret;
    }

    void AddAllocatedTask(osThread* pThread)
    {
        m_asyncTasks.push_back(pThread);
    }

    void CleanAllocatedTasks()
    {
        // Don't use +lambda+ until cpp11 flag is turned on on linux compiler.
        //std::for_each(m_asyncTasks.begin(), m_asyncTasks.end(), [](osThread* pThread)
        for (gtSize_t i = 0; i < m_asyncTasks.size(); ++i)
        {
            if (m_asyncTasks[i] != NULL)
            {
                std::wstringstream stream;
                stream << L"Daemon Thread Observer: terminating the following thread: " << m_asyncTasks[i]->id();
                OS_OUTPUT_DEBUG_LOG(stream.str().c_str(), OS_DEBUG_LOG_DEBUG);
                bool isOk = m_asyncTasks[i]->terminate();

                if (!isOk)
                {
                    stream << L" FAILURE" << std::endl;
                    OS_OUTPUT_DEBUG_LOG(stream.str().c_str(), OS_DEBUG_LOG_ERROR);
                }

                delete m_asyncTasks[i];
                m_asyncTasks[i] = NULL;
            }
        }
    }

    const osPortAddress& GetDaemonAddress() const
    {
        return gs_DaemonAddress;
    }

    bool GetDaemonCodeXLVersion(gtString& versionBuffer)
    {
        bool ret = false;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(ret);

        versionBuffer = L"";
        gtInt32 opCode = docGetDaemonCXLVersion;
        m_tcpClient << opCode;

        // Verify submission.
        gtInt32 opStatus = dosFailure;
        m_tcpClient >> opStatus;
        ret = (opStatus == dosSuccess);
        GT_ASSERT_EX(ret, L"Get Daemon CXL Version opcode submission.");

        if (ret)
        {
            ret = m_tcpClient.readString(versionBuffer);
            GT_ASSERT_EX(ret, L"Failed reading daemon CXL version string.");
        }

        return ret;
    }


    gtString PlatformToString(DaemonPlatform platform)
    {
        switch (platform)
        {
            case dpWindows:
                return PLATFROM_WINDOWS;

            case dpLinux:
                return PLATFORM_LINUX;

            case dpUnknown:
            default:
                return L"Unknown platform";
        }
    }


    bool GetDaemonPlatform(gtString& platformBuffer)
    {
        bool ret = false;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(ret);

        platformBuffer = L"";
        gtInt32 opCode = docGetDaemonPlatform;
        m_tcpClient << opCode;

        // Verify submission.
        gtInt32 opStatus = dosFailure;
        m_tcpClient >> opStatus;
        ret = (opStatus == dosSuccess);
        GT_ASSERT_EX(ret, L"Get Daemon CXL Version opcode submission.");

        if (ret)
        {
            gtInt32 agentPlatform = dpUnknown;
            m_tcpClient >> agentPlatform;
            ret = (agentPlatform > dpUnknown && agentPlatform <= dpPlatformCount);
            GT_ASSERT_EX(ret, L"DMN Client: Invalid remote agent platform value.");

            // Convert to string value.
            platformBuffer = PlatformToString(static_cast<DaemonPlatform>(agentPlatform));
        }

        return ret;
    }

    bool TerminateWholeSession()
    {
        bool ret = true;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(ret);

        if (m_isConnected)
        {
            // Send the opcode.
            gtInt32 opCode = docTerminateWholeSession;
            m_tcpClient << opCode;

            // Verify submission.
            gtInt32 opStatus = dosFailure;
            m_tcpClient >> opStatus;
            ret = (opStatus == dosSuccess);
            GT_ASSERT_EX(ret, L"Terminate Whole Session opcode submission.");

            // Reset the current state.
            SetDirtyConnection();
        }

        return ret;
    }

    gtString GetClientPlatform()
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        return PLATFROM_WINDOWS;
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
        return PLATFORM_LINUX;
#else
        return L"Unknown";
#endif
    }

    bool IsPlatformMatch(bool& resultBuffer, gtString& clientPlatformBuffer, gtString& agentPlatformBuffer)
    {
        bool ret = false;

        // Start with the platform.
        ret = GetDaemonPlatform(agentPlatformBuffer);
        GT_ASSERT_EX(ret, L"Failed retrieving agent's platform.");

        if (ret)
        {
            // Get the client platform.
            clientPlatformBuffer = GetClientPlatform();
            resultBuffer = true; //TODO  We support cross platform Win/Linux GPU profiling, as soon as new platform added we must add here specific logic for it
        }

        return ret;
    }

    bool IsVersionMatch(bool& resultBuffer, gtString& clientVersionBuffer, gtString& agentVersionBuffer)
    {
        resultBuffer = false;
        bool ret = GetDaemonCodeXLVersion(agentVersionBuffer);
        GT_ASSERT_EX(ret, L"Failed retrieving agent's version.");

        if (ret)
        {
            // Compare versions.
            osProductVersion appProductVersion;
            osGetApplicationVersion(appProductVersion);
            osProductVersion dmnProductVersion;
            dmnProductVersion.fromString(agentVersionBuffer);
            resultBuffer = (dmnProductVersion._majorVersion == appProductVersion._majorVersion &&
                            dmnProductVersion._minorVersion == appProductVersion._minorVersion);
            GT_ASSERT_EX(resultBuffer, L"Mismatch between client and agent versions.");

            // Return the client version.
            clientVersionBuffer = appProductVersion.toString();
        }

        return ret;
    }

    bool PerformHandshake(bool& isMatch, gtString& reasonStrBuffer)
    {
        isMatch = false;
        gtString clientVersionBuffer;
        gtString clientPlatformBuffer;
        gtString agentVersionBuffer;
        gtString agentPlatformBuffer;
        bool ret = IsPlatformMatch(isMatch, clientPlatformBuffer, agentPlatformBuffer);

        if (ret)
        {
            if (!isMatch)
            {
                reasonStrBuffer = RemoteClientUtils::GetPlatformMismatchMsg(clientPlatformBuffer, agentPlatformBuffer);
            }
            else
            {
                ret = IsVersionMatch(isMatch, clientVersionBuffer, agentVersionBuffer);

                if (!isMatch)
                {
                    reasonStrBuffer = RemoteClientUtils::GetVersionMismatchMsg(clientVersionBuffer, agentVersionBuffer);
                }
            }
        }

        return ret;
    }

    bool DisconnectWithoutClosing()
    {
        bool ret = false;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(ret);

        // Transfer the opcode.
        m_tcpClient << docPowerDisconnectWithoutClosing;

        // Verify.
        gtInt32 opStatus = dosFailure;
        m_tcpClient >> opStatus;
        ret = (opStatus == dosSuccess);
        GT_ASSERT_EX(ret, L"Disconnecting from agent during power profiling session - status query ACK.");

        return ret;
    }

    ///
    bool ReadFrameFilesData(FrameInfo& frameData)
    {
        bool retVal = true;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

        // Read the files count
        m_tcpClient.readString(frameData.m_descriptionFileRemotePath);
        int filesCount = 0;
        m_tcpClient >> filesCount;

        gtString message;
        message.appendFormattedString(L"ReadFrameFilesData: Reading %d files", filesCount);
        OS_OUTPUT_DEBUG_LOG(message.asCharArray(), OS_DEBUG_LOG_DEBUG);

        for (int i = 0; i < filesCount; i++)
        {
            // Get the current file attribute (extension and isBinary flag)
            bool isBinary;
            gtString extension;
            m_tcpClient >> extension;
            m_tcpClient >> isBinary;
            gtUInt32 dataSize = 0;
            if (isBinary)
            {
                gtByte* pBuffer = nullptr;

                // Read the data size from the remote agent
                m_tcpClient >> dataSize;

                message.makeEmpty();
                message.appendFormattedString(L"ReadFrameFilesData: Reading binary file. dataSize: %d ", (int)dataSize);
                OS_OUTPUT_DEBUG_LOG(message.asCharArray(), OS_DEBUG_LOG_DEBUG);


                // Read the data size from the remote agent
                pBuffer = new gtByte[dataSize]();
                bool rc = m_tcpClient.read((gtByte*)pBuffer, dataSize);

                if (rc)
                {
                    if (extension.compareNoCase(FRAME_IMAGE_FILE_EXT) == 0)
                    {
                        frameData.m_pImageBuffer = reinterpret_cast<unsigned char*>(pBuffer);
                        frameData.m_imageSize = dataSize;
                    }
                    else
                    {
                        gtString msg(L"Unsupported file format sent from the remote agent: ");
                        msg.append(extension);
                        GT_ASSERT_EX(false, msg.asCharArray());
                        retVal = false;
                    }
                }
                else
                {
                    gtString msg;
                    msg.appendFormattedString(L"Failed to read binary file sent from remote agent. File size = %d. File extension = ", dataSize);
                    msg.append(extension);
                    GT_ASSERT_EX(false, msg.asCharArray());
                    // Reading the binary file content failed. This invalidates the communication protocol so we have
                    // to close the TCP connection to avoid losing sync with the remote agent communication stage
                    m_tcpClient.close();
                    retVal = false;
                    break;
                }
            }
            else
            {
                message.makeEmpty();
                message.appendFormattedString(L"ReadFrameFilesData: Reading text file. %ls ", extension.asCharArray());
                OS_OUTPUT_DEBUG_LOG(message.asCharArray(), OS_DEBUG_LOG_DEBUG);

                if (extension.compareNoCase(FRAME_DESCRITPION_FILE_EXT) == 0)
                {
                    // Read the frame info XML string from the remote agent
                    // Read the data size from the remote agent
                    m_tcpClient >> dataSize;
                    gtByte* pBuffer = new gtByte[dataSize+1]();
                    bool rc = m_tcpClient.read((gtByte*)pBuffer, dataSize);
                    GT_ASSERT(rc == true);
                    frameData.m_frameInfoXML = pBuffer;
                    delete [] pBuffer;
                }
                else if (extension.compareNoCase(FRAME_TRACE_FILE_EXT) == 0)
                {
                    m_tcpClient >> frameData.m_frameTrace;
                }
                else
                {
                    gtString msg(L"Unsupported file format sent from the remote agent: ");
                    msg.append(extension);
                    GT_ASSERT_EX(false, msg.asCharArray());

                    // Read into a temp string (avoid breaking the protocol)
                    gtASCIIString tempStr;
                    m_tcpClient >> tempStr;

                    retVal = false;

                }
            }
        }

        return retVal;
    }

    bool IsProcessRunning(const gtString& processName, bool& isProcessRunning, RemoteClientError& errorCode)
    {
        bool retVal = false;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

        errorCode = rceUnknown;
        m_tcpClient << docIsProcessRunning;
        // Verify.
        gtInt32 opStatus = dosFailure;
        m_tcpClient >> opStatus;
        retVal = (opStatus == dosSuccess);
        GT_ASSERT_EX(retVal, L"OpCode receive ack.");

        // Send process name
        m_tcpClient << processName;

        // Read the process running flag
        m_tcpClient >> isProcessRunning;

        // Verify that data was received by the remote agent
        opStatus = dosFailure;
        m_tcpClient >> opStatus;
        retVal = (opStatus == dosSuccess);

        return retVal;
    }

    bool KillRunningProcess(const gtString& processName, RemoteClientError& errorCode)
    {
        bool retVal = false;
        OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);
        errorCode = rceUnknown;
        m_tcpClient << docKillRunningProcess;
        // Verify.
        gtInt32 opStatus = dosFailure;
        m_tcpClient >> opStatus;
        retVal = (opStatus == dosSuccess);
        GT_ASSERT_EX(retVal, L"OpCode receive ack.");

        // Send process name
        m_tcpClient << processName;

        // Verify that data was received by the remote agent
        opStatus = dosFailure;
        m_tcpClient >> opStatus;
        retVal = (opStatus == dosSuccess);

        return retVal;
    }

    bool IsHSAEnabled()
    {
        bool retVal = false;
        bool isHSAEnabled = false;
        m_tcpClient << docIsHSAEnabled;
        // Verify.
        gtInt32 opStatus = dosFailure;
        m_tcpClient >> opStatus;
        retVal = (opStatus == dosSuccess);
        GT_ASSERT_EX(retVal, L"OpCode receive ack.");


        if (retVal)
        {
            m_tcpClient >> isHSAEnabled;
            // Verify that data was received by the remote agent
            opStatus = dosFailure;
            m_tcpClient >> opStatus;
            retVal = (opStatus == dosSuccess);
            GT_ASSERT_EX(retVal, L"OpCode receive ack.");
        }
        return isHSAEnabled;
    }

    bool ValidateAppPaths(const gtString& appFilePath, const gtString& workingFolderPath, bool& isAppValid, bool& isWorkingFolderValid)
    {
        bool retVal = false;
        m_tcpClient << docValidateAppPaths;
        // Verify.
        gtInt32 opStatus = dosFailure;
        m_tcpClient >> opStatus;
        retVal = (opStatus == dosSuccess);
        GT_ASSERT_EX(retVal, L"OpCode receive ack.");


        if (retVal)
        {
            m_tcpClient << appFilePath;
            m_tcpClient >> isAppValid;

            m_tcpClient << workingFolderPath;
            m_tcpClient >> isWorkingFolderValid;
            
            // Verify that data was received by the remote agent
            opStatus = dosFailure;
            m_tcpClient >> opStatus;
            retVal = (opStatus == dosSuccess);
            GT_ASSERT_EX(retVal, L"OpCode receive ack.");
        }
       
        return retVal;
    }


private:
    long m_readTimeout;
    bool m_isConnected;
    osTCPSocketClient m_tcpClient;
    std::vector<osThread*> m_asyncTasks;
};


// Represents an async file transfer operation.
class RemoteFileTransferAsyncTask :
    public osThread
{
public:

    RemoteFileTransferAsyncTask(CXLDaemonClient* instance, const gtString& remoteFileName, const gtString& localTargetFileName,
                                CXLDaemonClient::FileTransferCompletedCallback cb, void* params) :  osThread(L"File Transfer"), m_clientInstance(instance), m_remoteFileName(remoteFileName),
        m_localTargetFileName(localTargetFileName), m_cb(cb), m_params(params) {}

    // Don't use +override+ until cpp11 flag is turned on on linux compiler.
    virtual int entryPoint() /*override*/
    {
        int ret = -1;
        GT_IF_WITH_ASSERT(m_clientInstance != NULL)
        {
            bool bRet = m_clientInstance->GetRemoteFile(m_remoteFileName, m_localTargetFileName, false, NULL, NULL);
            GT_ASSERT_EX(bRet, L"Remote file transfer failed, before invoking callback.");
            m_cb(bRet, m_remoteFileName, m_localTargetFileName, m_params);
        }
        return ret;
    }

private:
    RemoteFileTransferAsyncTask& operator=(const RemoteFileTransferAsyncTask&);
    CXLDaemonClient* m_clientInstance;
    const gtString m_remoteFileName;
    const gtString m_localTargetFileName;
    CXLDaemonClient::FileTransferCompletedCallback m_cb;
    void* m_params;
};


CXLDaemonClient::CXLDaemonClient(const osPortAddress& daemonAddress, long readTimeout) : m_pImpl(NULL)
{
    gs_DaemonAddress = daemonAddress;
    m_pImpl = new(std::nothrow)CXLDaemonClient::Impl(readTimeout);

}


bool CXLDaemonClient::ValidateAppPaths(const osPortAddress& daemonAddress, const gtString& appFilePath, const gtString& workingFolderPath, bool& isAppValid, bool& isWorkingFolderValid)
{
    const unsigned CONNECTION_VALIDATION_TIMEOUT_MS = 5000;
    isAppValid = isWorkingFolderValid = false;
    bool ret = true;
    bool terminate = false;
    if (IsInitialized(daemonAddress) == false)
    {
         ret = Init(daemonAddress, CONNECTION_VALIDATION_TIMEOUT_MS, true);
         //we shall terminate session since, it's was only open for this specific validation
         terminate = true;
    }

    GT_IF_WITH_ASSERT(ret)
    {
        CXLDaemonClient* pClient = CXLDaemonClient::GetInstance();
        GT_IF_WITH_ASSERT(pClient != NULL)
        {
            // We will not use this data.
            osPortAddress tmpClientAddr;
            // Check if we can connect.
            ret = pClient->ConnectToDaemon(tmpClientAddr);

            // If required, terminate this dummy session.
            if (ret)
            {
                //TODO check paths valid
                ret = pClient->ValidateAppPaths(appFilePath, workingFolderPath, isAppValid, isWorkingFolderValid);
                if (terminate)
                {
                    pClient->TerminateWholeSession();
                }
            }
        }
    }
    return ret;
}

bool CXLDaemonClient::IsAgentPlatformLinux() const
{
    //set result to default platform
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS 
    bool result = true;
#else
    bool result = false;
#endif

    if (gs_IsInitialized)
    {
        gtString platformBuffer;
        m_pImpl->GetDaemonPlatform(platformBuffer);
        result = platformBuffer.compare(PLATFORM_LINUX) == 0;
    }
    return result;
}

CXLDaemonClient::~CXLDaemonClient(void)
{
    if (m_pImpl != NULL)
    {
        delete m_pImpl;
        m_pImpl = nullptr;
    }
}

bool CXLDaemonClient::ConnectToDaemon(osPortAddress& connectionPortBuffer)
{
    bool ret = false;

    if (m_pImpl != NULL)
    {
        ret = m_pImpl->ConnectToDaemon(connectionPortBuffer);
    }

    return ret;
}

bool CXLDaemonClient::IsProcessRunning(const gtString& processName, bool& isProcessRunning, RemoteClientError& errorCode)
{
    bool ret = false;

    if (m_pImpl != NULL)
    {
        ret = m_pImpl->IsProcessRunning(processName, isProcessRunning, errorCode);
    }

    return ret;
}

bool CXLDaemonClient::KillRunningProcess(const gtString& processName, RemoteClientError& errorCode)
{
    bool ret = false;

    if (m_pImpl != NULL)
    {
        ret = m_pImpl->KillRunningProcess(processName, errorCode);
    }

    return ret;
}

bool CXLDaemonClient::LaunchRDS(const gtString& cmdLineArgs,
                                const std::vector<osEnvironmentVariable>& envVars)
{
    bool ret = false;

    if (m_pImpl != NULL)
    {
        ret = m_pImpl->LaunchRDS(cmdLineArgs, envVars);
    }

    return ret;
}

bool CXLDaemonClient::GetSessionStatus(REMOTE_OPERATION_MODE mode, DaemonSessionStatus& buffer)
{
    bool ret = false;
    buffer = dssUknown;

    if (m_pImpl != NULL)
    {
        ret = m_pImpl->GetSessionStatus(mode, buffer);
    }

    return ret;
}

bool CXLDaemonClient::GetRemoteFile(const gtString& remoteFileName, const gtString& localTargetFileName,
                                    bool isAsync, FileTransferCompletedCallback cb, void* params)
{
    bool ret = false;

    if (m_pImpl != NULL)
    {
        if (!isAsync)
        {
            ret = m_pImpl->GetRemoteFile(remoteFileName, localTargetFileName);
        }
        else
        {
            RemoteFileTransferAsyncTask* pTransferTask = new(std::nothrow)RemoteFileTransferAsyncTask(this,
                    remoteFileName, localTargetFileName, cb, params);

            m_pImpl->AddAllocatedTask(pTransferTask);
            ret = pTransferTask->execute();
        }
    }

    return ret;
}

bool CXLDaemonClient::TerminateRemoteProcess(REMOTE_OPERATION_MODE mode)
{
    bool ret = false;

    if (m_pImpl != NULL)
    {
        ret = m_pImpl->TerminateRemoteProcess(mode);
    }

    return ret;
}


bool CXLDaemonClient::Init(const osPortAddress& daemonAddress, long readTimeout, bool isForcedInitialization)
{
    bool ret = false;

    if (!gs_IsInitialized)
    {
        gs_DaemonAddress = daemonAddress;
        gs_ReadTimeout   = readTimeout;
        ret = gs_IsInitialized = true;
    }
    else
    {
        // Check if it's a different address.
        if (IsDifferentAddress(gs_DaemonAddress, daemonAddress))
        {
            gs_DaemonAddress = daemonAddress;
            gs_ReadTimeout   = readTimeout;
            ret = gs_IsDirty = true;
        }
        else if (isForcedInitialization)
        {
            ret = gs_IsDirty = true;
        }

        OS_OUTPUT_DEBUG_LOG(L"Re-Initialization of CodeXL Daemon Client.", OS_DEBUG_LOG_ERROR);
    }

    return ret;
}

bool CXLDaemonClient::IsInitialized(const osPortAddress& dmnAddress)
{
    bool isDifferentAddr = IsDifferentAddress(gs_DaemonAddress, dmnAddress);
    return (isDifferentAddr) ? false : gs_IsInitialized;
}

// This is NOT thread safe!
CXLDaemonClient* CXLDaemonClient::GetInstance()
{
    CXLDaemonClient* ret = NULL;

    // Enter critical section.
    osMutexLocker locker(gs_InstanceLock);

    if (gs_IsInitialized)
    {
        if (gs_Instance == NULL)
        {
            gs_Instance = new CXLDaemonClient(gs_DaemonAddress, gs_ReadTimeout);
        }
        else if (gs_IsDirty)
        {
            // Recreate the connection.
            delete gs_Instance;
            gs_Instance = new CXLDaemonClient(gs_DaemonAddress, gs_ReadTimeout);
            gs_IsDirty = false;
        }

        ret = gs_Instance;
    }

    return ret;
}

void CXLDaemonClient::Close()
{
    if (gs_Instance != NULL)
    {
        delete gs_Instance;
        gs_Instance = NULL;

        // Reset other static storage.
        gs_IsInitialized = false;
        gs_IsDirty  = false;
        gs_ReadTimeout = 0;
    }
}

bool CXLDaemonClient::LaunchGPUProfiler(const gtString& cmdLineArgs, const gtString& localGpuProfilerBaseDir,
                                        const gtString& counterFileName, const gtString& envVarsFileName,
                                        const gtString& apiFilterFileName, const gtString& apiRulesFileName,
                                        const gtVector<gtString>& specificKernels, RemoteClientError& errorCode)
{
    bool ret = false;

    if (m_pImpl != NULL)
    {
        ret = m_pImpl->LaunchGpuProfiler(cmdLineArgs, localGpuProfilerBaseDir,
                                         counterFileName, envVarsFileName,
                                         apiFilterFileName, apiRulesFileName,
                                         specificKernels, errorCode);
    }

    return ret;
}

bool CXLDaemonClient::LaunchGraphicsBeckendServer(const osFilePath& serverPath, const gtString& cmdLineArgs, const osDirectory& workDirectory, RemoteClientError& errorCode)
{
    bool ret = false;

    if (m_pImpl != NULL)
    {
        ret = m_pImpl->LaunchGraphicsBeckendServer(serverPath, cmdLineArgs, workDirectory, errorCode);
    }

    return ret;
}

bool CXLDaemonClient::GetCapturedFrames(const gtString& projectName, gtString& capturedFramesXmlStr, RemoteClientError& errorCode)
{
    bool ret = false;

    if (m_pImpl != NULL)
    {
        ret = m_pImpl->GetCapturedFrames(projectName, capturedFramesXmlStr, errorCode);
    }

    return ret;
}

bool CXLDaemonClient::GetCapturedFrames(const gtString& projectName, const gtString& session, const osTime* pCapturedMinimalTime, gtString& capturedFramesXmlStr, RemoteClientError& errorCode)
{
    bool ret = false;

    if (m_pImpl != NULL)
    {
        ret = m_pImpl->GetCapturedFrames(projectName, session, pCapturedMinimalTime, capturedFramesXmlStr, errorCode);
    }

    return ret;
}

bool CXLDaemonClient::GetCapturedFrameData(const gtString& projectName, const gtString& sessionName, const int frameIndex, FrameInfo& frameData, RemoteClientError& errorCode)
{
    bool ret = false;

    if (m_pImpl != NULL)
    {
        ret = m_pImpl->GetCapturedFrameData(projectName, sessionName, frameIndex, frameData, errorCode);
    }

    return ret;
}

bool CXLDaemonClient::DeleteFrameAnalysisSession(const gtString& projectName, const gtString& sessionName, RemoteClientError& errorCode)
{
    bool ret = false;

    if (m_pImpl != NULL)
    {
        ret = m_pImpl->DeleteFrameAnalysisSession(projectName, sessionName, errorCode);
    }

    return ret;
}


bool CXLDaemonClient::GetDaemonCodeXLVersion(gtString& versionBuffer)
{
    bool ret = false;

    if (m_pImpl != NULL)
    {
        ret = m_pImpl->GetDaemonCodeXLVersion(versionBuffer);
    }

    return ret;
}

bool CXLDaemonClient::TerminateWholeSession()
{
    bool ret = false;

    if (m_pImpl != NULL)
    {
        ret = m_pImpl->TerminateWholeSession();
    }

    return ret;
}

bool CXLDaemonClient::PerformHandshake(bool& resultBuffer, gtString& errorStrBuffer)
{
    bool ret = false;

    if (m_pImpl != NULL)
    {
        ret = m_pImpl->PerformHandshake(resultBuffer, errorStrBuffer);
    }

    return ret;
}


bool CXLDaemonClient::ValidateConnectivity(const osPortAddress& daemonAddress, bool& isConnectivityValid)
{
    const unsigned CONNECTION_VALIDATION_TIMEOUT_MS = 5000;
    isConnectivityValid = false;
    bool ret = Init(daemonAddress, CONNECTION_VALIDATION_TIMEOUT_MS, true);
    GT_IF_WITH_ASSERT(ret)
    {
        CXLDaemonClient* pClient = CXLDaemonClient::GetInstance();
        GT_IF_WITH_ASSERT(pClient != NULL)
        {
            // We will not use this data.
            osPortAddress tmpClientAddr;

            // Check if we can connect.
            isConnectivityValid = pClient->ConnectToDaemon(tmpClientAddr);

            // If required, terminate this dummy session.
            if (isConnectivityValid)
            {
                pClient->TerminateWholeSession();
            }
        }
    }
    return ret;
}

bool CXLDaemonClient::DisconnectWithoutClosing()
{
    bool ret = false;

    if (m_pImpl != NULL)
    {
        ret = m_pImpl->DisconnectWithoutClosing();
    }

    return ret;
}

bool CXLDaemonClient::GetDaemonAddress(osPortAddress& address)
{
    if (m_pImpl != NULL)
    {
        address = m_pImpl->GetDaemonAddress();
        return true;
    }

    return false;
}

bool CXLDaemonClient::IsHSAEnabled()
{
    bool result = false;
    if (m_pImpl != nullptr)
    {
        result = m_pImpl->IsHSAEnabled();
    }

    return result;
}

bool CXLDaemonClient::ValidateAppPaths(const gtString& appFilePath, const gtString& workingFolderPath, bool& isAppValid, bool& isWorkingFolderValid)
{
    bool result = false;
    if (m_pImpl != nullptr)
    {
        result = m_pImpl->ValidateAppPaths(appFilePath, workingFolderPath, isAppValid, isWorkingFolderValid);
    }

    return result;
}
