//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnSessionThread.cpp
///
//==================================================================================


#ifdef _WIN32
    #pragma warning ( push )
    #pragma warning(disable : 4996)
    #pragma warning(disable : 4244)
#endif // _WIN32

// Boost
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string_regex.hpp>

#ifdef _WIN32
    #pragma warning ( pop )
#endif

// Infra
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSAPIWrappers/Include/oaDriver.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define ZLIB_WINAPI
#endif

// For file compression proxy.
#include <zlib.h>


// C++.
#include <sstream>
#include <iostream>
#include <fstream>

// Local.
#include <AMDTRemoteAgent/dmnSessionThread.h>
#include <AMDTRemoteAgent/dmnConnectionWatcherThread.h>
#include <AMDTRemoteAgent/Public Include/dmnStringConstants.h>
#include <AMDTRemoteClient/Include/RemoteClientDataTypes.h>

// CodeXL Infrastructure.
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osTCPSocket.h>
#include <AMDTOSWrappers/Include/osRawMemoryBuffer.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers//Include/osGeneralFunctions.h>

// Internal defs.
const unsigned int OPCODE_BUFFER_SIZE = sizeof(gtInt32);
const int LOOP_SLEEP_INTERVAL_MS = 100;
const int FS_REFRESH_INTERVAL_MS = 1000;
const gtString COMPRESSED_FILE_SUFFIX = L"_zipped";

// Executable names and paths:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define DMN_REMOTE_DEBUGGING_SERVER_EXECUTABLE_NAME L"CXLRemoteDebuggingServer" GDT_PROJECT_SUFFIX_W
    #define DMN_REMOTE_DEBUGGING_SERVER_EXECUTABLE_EXTENSION L"exe"
    #define DMN_PROFILER_BACKEND_32_BIT_SUBFOLDER OS_STR_32BitDirectoryName
    #define DMN_PROFILER_BACKEND_64_BIT_SUBFOLDER OS_STR_64BitDirectoryName
    #define DMN_PROFILER_BACKEND_EXECUTABLE_NAME L"CodeXLGpuProfiler" GDT_PROJECT_SUFFIX_W
    #define DMN_PROFILER_BACKEND_64_BIT_EXECUTABLE_NAME L"CodeXLGpuProfiler-x64" GDT_PROJECT_SUFFIX_W
    #define DMN_PROFILER_BACKEND_EXECUTABLE_EXTENSION L"exe"
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    // Linux version has same name for both debug and release builds:
    #define DMN_REMOTE_DEBUGGING_SERVER_EXECUTABLE_NAME L"CXLRemoteDebuggingServer"
    #define DMN_REMOTE_DEBUGGING_SERVER_EXECUTABLE_EXTENSION L""
    // Linux does not currently support 32-bit profiling, so we do not have an L"x86" option:
    #define DMN_PROFILER_BACKEND_32_BIT_SUBFOLDER OS_STR_64BitDirectoryName
    #define DMN_PROFILER_BACKEND_64_BIT_SUBFOLDER OS_STR_64BitDirectoryName
    #define DMN_PROFILER_BACKEND_INTERNAL_EXECUTABLE_NAME L"CodeXLGpuProfiler-internal"
    #define DMN_PROFILER_BACKEND_NORMAL_EXECUTABLE_NAME L"CodeXLGpuProfiler"

    #define DMN_PROFILER_BACKEND_EXECUTABLE_EXTENSION L""
#else
    #error Unknown build target!
#endif

// Maximum number of environment variables.
const int MAX_ENV_VAR_COUNT = 1024;


// *************** INTERNALLY-LINKED UTILITY FUNCTIONS - START ***************  //

static bool TerminateProcess(osProcessId procId)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(procId != 0)
    {
        ret = osTerminateProcess(procId);
    }
    return ret;
}

static bool FillPathBuffers(REMOTE_OPERATION_MODE mode, bool is64BitTarget, osFilePath& fileBuffer, osFilePath& pathBuffer)
{
    bool ret = false;

    // Creating a full path for the application.
    // In the production daemon, we should read from the config file the
    // full path of CodeXLGpuProfiler and RDS, and write one function that fills the osFilePath
    // structures according to the opcode received from the client.
    gtString dirPath;
    gtString exePath;
    ret = dmnUtils::GetCurrentDirectory(dirPath);
    dirPath.removeTrailing(osFilePath::osPathSeparator).append(osFilePath::osPathSeparator);
    GT_ASSERT_EX(ret, L"DMN: Extracting current directory.");

    if (mode == romDEBUG)
    {
        exePath = dirPath;
        exePath.append(DMN_REMOTE_DEBUGGING_SERVER_EXECUTABLE_NAME).append(osFilePath::osExtensionSeparator).append(DMN_REMOTE_DEBUGGING_SERVER_EXECUTABLE_EXTENSION);
        ret = true;
    }
    else if (mode == romPROFILE)
    {
        ret = false;
        const dmnConfigManager* pConfigMgr = dmnConfigManager::Instance();
        GT_IF_WITH_ASSERT(pConfigMgr != NULL)
        {
            exePath = dirPath;

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
            GT_ASSERT_EX(is64BitTarget, L"DMN: Error: trying to profile non-64-bit target on linux.");
            bool isInternalVersion = pConfigMgr->IsInternalVersion();
            exePath.append(isInternalVersion ? DMN_PROFILER_BACKEND_INTERNAL_EXECUTABLE_NAME : DMN_PROFILER_BACKEND_NORMAL_EXECUTABLE_NAME).append(osFilePath::osExtensionSeparator)
            .append(DMN_PROFILER_BACKEND_EXECUTABLE_EXTENSION);
#elif AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            exePath.append(is64BitTarget ? DMN_PROFILER_BACKEND_64_BIT_EXECUTABLE_NAME : DMN_PROFILER_BACKEND_EXECUTABLE_NAME).append(osFilePath::osExtensionSeparator).append(DMN_PROFILER_BACKEND_EXECUTABLE_EXTENSION);
#else
#error Unknown build target!
#endif

            ret = true;
        }
    }

    if (ret)
    {
        fileBuffer.setFullPathFromString(exePath);
        pathBuffer.setFullPathFromString(dirPath);
    }

    return ret;
}



static bool CompressFile(const osFilePath& fileToCompress, osFilePath& compressedFileBuffer)
{
    osFilePath compressedFilePath = fileToCompress;
    gtString compressedFileName;
    bool ret = fileToCompress.getFileName(compressedFileName);
    GT_IF_WITH_ASSERT(ret)
    {
        compressedFileName.append(COMPRESSED_FILE_SUFFIX);
        compressedFilePath.setFileName(compressedFileName);

        // First load the original file.
        osFile src(fileToCompress);
        ret = src.open(osChannel::OS_BINARY_CHANNEL);
        GT_ASSERT_EX(ret, L"DMN: Failed to open the file to compress.");

        if (ret)
        {
            unsigned long fileSize = 0;
            ret = src.getSize(fileSize);
            GT_ASSERT_EX(ret, L"DMN: Failed to the size of the file to compress.");

            if (ret)
            {
                gtByte* buffer = new(nothrow) gtByte[fileSize]();

                gtSize_t amountRead = 0;
                ret = src.readAvailableData(buffer, fileSize, amountRead);
                GT_ASSERT_EX(ret, L"DMN: Unable to read the data of the file to compress.");
                src.close();

                if (ret)
                {
                    // Now compress it.
                    unsigned long sizeDataCompressed  = amountRead + (amountRead / 10) + 12;
                    gtByte* dstBuffer = new(nothrow) gtByte[sizeDataCompressed]();

                    int res = compress((unsigned char*)dstBuffer, &sizeDataCompressed, (unsigned char*)buffer, amountRead);
                    ret = (res == Z_OK);
                    GT_ASSERT_EX(ret, L"DMN: Failed to compress file on compression proxy.");

                    if (ret)
                    {
                        // Now let's save the compressed file.
                        osFile compressedFile;
                        compressedFile.open(compressedFilePath, osChannel::OS_BINARY_CHANNEL, osFile::OS_OPEN_TO_WRITE);
                        compressedFile.write(dstBuffer, sizeDataCompressed);
                        compressedFile.close();

                        // Assign the buffer, since we succeeded.
                        compressedFileBuffer = compressedFilePath;
                    }

                    // Clean.
                    delete[] dstBuffer;
                    delete[] buffer;
                }
            }
        }
    }
    return ret;
}



static bool TransferFile(const osFilePath& filePath, osChannel* channel, bool isCompressionRequired = true)
{
    bool isOk = true;
    osFilePath filePathToUse;

    if (isCompressionRequired)
    {
        // Save the original file size.
        // In case compression is required, we need to send it to the remote client.
        // This represents the upper bound of the size of the decompressed file (after decompression).
        unsigned long originalFileSize = 0;
        osFile fileToSend(filePath);
        isOk = fileToSend.getSize(originalFileSize) && (originalFileSize > 0);
        GT_ASSERT_EX(isOk, L"DMN: Failed to extract the size of the original file.");

        isOk = CompressFile(filePath, filePathToUse);
        GT_ASSERT_EX(isOk, L"DMN: Failed to compress flie on file transfer.");

        // First send the original size.
        gtInt32 fileSizeToSend = static_cast<gtInt32>(originalFileSize);
        (*channel) << fileSizeToSend;

        // Verify.
        gtInt32 opStatus = dosFailure;
        (*channel) >> opStatus;
        isOk = (opStatus == dosSuccess);
        GT_ASSERT_EX(isOk, L"DMN: Failed to receive ack for file size of tranferred file.");
    }
    else
    {
        filePathToUse = filePath;
    }

    if (isOk)
    {
        osRawMemoryBuffer buffer;
        isOk = buffer.fromFile(filePathToUse);
        GT_IF_WITH_ASSERT(isOk && channel != NULL)
        {
            isOk = buffer.writeSelfIntoChannel(*channel);
        }
    }

    return isOk;
}

static bool TransferFile(const gtString& fileName, osChannel* channel,  bool isCompressionRequired = true)
{
    osFilePath filePath(fileName);
    return TransferFile(filePath, channel, isCompressionRequired);
}



static void ReportSuccess(osChannel* pTargetChannel)
{
    GT_IF_WITH_ASSERT(pTargetChannel != NULL)
    {
        (*pTargetChannel) << dosSuccess;
    }
}

static void ReportFailure(osChannel* pTargetChannel)
{
    GT_IF_WITH_ASSERT(pTargetChannel != NULL)
    {
        (*pTargetChannel) << dosFailure;
    }
}

static void ReportResult(bool isSuccess, osChannel* pChannel)
{
    isSuccess ? ReportSuccess(pChannel) : ReportFailure(pChannel);
}



static bool ExtractCmdLineString(osChannel* pChannel, gtString& buffer)
{
    // Extract the string from the channel.
    bool isOk = false;
    GT_IF_WITH_ASSERT(pChannel != NULL)
    {
        isOk = pChannel->readString(buffer);
        GT_ASSERT(isOk);

        // Tracing.
        wstringstream stream;
        stream << L"DMN: RECEIVED CMD LINE ARGS -> \"";
        stream << buffer.asCharArray() << L"\"";
        dmnUtils::LogMessage(stream.str(), OS_DEBUG_LOG_DEBUG);

        // Respond.
        ReportResult(isOk, pChannel);
    }
    return isOk;
}

static bool ExtractEnvVariables(osChannel* pChannel, vector<osEnvironmentVariable>& envVarsBuffer, gtInt32& envVarsCountBuffer)
{
    bool isOk = false;

    GT_IF_WITH_ASSERT(pChannel != NULL)
    {
        // Extract the number of environment vars.
        envVarsCountBuffer = 0;
        (*pChannel) >> envVarsCountBuffer;

        isOk = (envVarsCountBuffer >= 0 && envVarsCountBuffer <= MAX_ENV_VAR_COUNT);

        // Respond.
        ReportResult(isOk, pChannel);

        GT_IF_WITH_ASSERT(isOk)
        {
            // Tracing.
            wstringstream _stream;
            _stream << L"DMN: Number of environment variables is: " << envVarsCountBuffer;
            dmnUtils::LogMessage(_stream.str(), OS_DEBUG_LOG_DEBUG);

            // Get the environment variables.
            gtInt32 envVarsCountCopy = envVarsCountBuffer;

            isOk = true;

            while (isOk && envVarsCountCopy-- > 0)
            {
                gtString _key, _value;
                isOk = pChannel->readString(_key);
                isOk = isOk && pChannel->readString(_value);
                GT_ASSERT(isOk);

                envVarsBuffer.push_back(osEnvironmentVariable(_key, _value));
            }

            GT_ASSERT(isOk);

            // Respond.
            ReportResult(isOk, pChannel);
        }
    }
    return isOk;
}

static bool ExtractServerPath(osChannel* pChannel, osFilePath& serverPath)
{
    bool isOk = false;

    GT_IF_WITH_ASSERT(pChannel != NULL)
    {
        gtString serverPathStr;
        (*pChannel) >> serverPathStr;
        serverPath = serverPathStr;

        isOk = (serverPath.exists());

        // Respond.
        gtString serverFileName;
        serverPath.getFileNameAndExtension(serverFileName);
        bool isServerUp = osIsProcessAlive(serverFileName);
        (*pChannel) << isServerUp;
        isOk = isServerUp == false;
        GT_IF_WITH_ASSERT(isOk)
        {
            // Tracing.
            wstringstream _stream;
            _stream << L"DMN: Srever path is: " << serverPathStr.asCharArray();
            dmnUtils::LogMessage(_stream.str(), OS_DEBUG_LOG_DEBUG);
        }
    }
    return isOk;
}

static bool ExtractOpCode(osChannel* pChannel, gtByte* opCodeBuffer)
{
    bool isOk = false;
    gtSize_t utilizedBytes = 0;
    GT_IF_WITH_ASSERT(pChannel != NULL)
    {
        // Wait for opcode.
        isOk = pChannel->readAvailableData(opCodeBuffer, OPCODE_BUFFER_SIZE, utilizedBytes);
        GT_ASSERT(isOk && utilizedBytes == OPCODE_BUFFER_SIZE);

        if (utilizedBytes > 0)
        {
            // Respond.
            ReportResult(isOk, pChannel);
        }
    }
    return isOk;
}


static bool FixTokenizedPathString(const gtString& tokenizedPath, gtString& fixedPath)
{
    // Should be read from dmnDefinitions.h.
    const gtString PATH_TOKEN = L"$$$";
    const gtString CodeXLInstallPath = L"$CXL_Install_Path$\\";
    bool ret = false;
    fixedPath = tokenizedPath;

    if (fixedPath.find(PATH_TOKEN) != -1)
    {
        osFilePath localUserDataPath;
        ret = dmnUtils::GetUserDataFolder(localUserDataPath);
        GT_IF_WITH_ASSERT(ret)
        {
            gtString tokenFiller = localUserDataPath.asString();
            fixedPath.replace(0, fixedPath.length() - 1, PATH_TOKEN, tokenFiller, true);
        }
    }

    // replace cxl install path
    if (fixedPath.find(CodeXLInstallPath) != -1)
    {
        osFilePath appExePath;
        ret = osGetCurrentApplicationPath(appExePath);
        GT_IF_WITH_ASSERT(ret)
        {
            osDirectory appDir;
            appExePath.getFileDirectory(appDir);
            gtString replaceStr = appDir.asString();
            fixedPath.replace(0, fixedPath.length() - 1, CodeXLInstallPath, replaceStr, true);
        }
    }

    return ret;
}


static bool FixCodeXLGpuProfilerCmdLineArgs(const gtString& originalCmdLineArgs, gtString& fixedRemotePath)
{
    return FixTokenizedPathString(originalCmdLineArgs, fixedRemotePath);
}


static gtString ExtractDirFromCodeXLGpuProfilerCmdLineArgsString(const gtString& fixedCmdLineStr)
{
    gtString ret = L"";

    const gtString PREFIX = L" --outputfile \"";
    const gtString SUFFIX = L"--sessionname";

    int prefIndex = fixedCmdLineStr.find(PREFIX);
    GT_IF_WITH_ASSERT(prefIndex > -1)
    {
        prefIndex += PREFIX.length();
        int suffIndex = fixedCmdLineStr.find(SUFFIX);
        GT_IF_WITH_ASSERT(suffIndex > -1 && prefIndex < suffIndex)
        {
            fixedCmdLineStr.getSubString(prefIndex, suffIndex, ret);

            if (!ret.isEmpty())
            {
                // Sometimes there is mixed separators in these cmd line string:
                osFilePath::adjustStringToCurrentOS(ret);

                int fileNameSeparator = ret.reverseFind(osFilePath::osPathSeparator);

                if (fileNameSeparator > 0)
                {
                    ret.getSubString(0, fileNameSeparator - 1, ret);
                }
            }
        }
    }

    return ret;
}

static bool SetupCodeXLGpuProfilerOutputDir(const gtString& fixedCmdLineArgs, osDirectory& outDirBuffer)
{
    gtString codeXLGpuProfilerOutDirStr = ExtractDirFromCodeXLGpuProfilerCmdLineArgsString(fixedCmdLineArgs);
    bool isOk = !codeXLGpuProfilerOutDirStr.isEmpty();
    GT_ASSERT_EX(isOk, L"DMN: Extracted empty CodeXLGpuProfiler output dir from cmd line args.");

    if (isOk)
    {
        osFilePath codeXLGpuProfilerOutDirPath(codeXLGpuProfilerOutDirStr);
        osDirectory codeXLGpuProfilerOutDir(codeXLGpuProfilerOutDirPath);

        if (!codeXLGpuProfilerOutDir.exists())
        {
            isOk = dmnUtils::CreateDirHierarchy(codeXLGpuProfilerOutDirStr);
            GT_ASSERT_EX(isOk, L"DMN: Failed to create output directory for CodeXLGpuProfiler.");

            if (isOk)
            {
                outDirBuffer = osDirectory(codeXLGpuProfilerOutDir);
            }
        }
    }

    return isOk;
}

static void dmnHandleSampleSourceFilePath(osFilePath& pathToHandle)
{
    // This entire operation is only relevant for release builds:
#if AMDT_BUILD_CONFIGURATION == AMDT_RELEASE_BUILD
    {
        static const gtString sampleSourceExtension = L"cpp";
        gtString fileExt;
        pathToHandle.getFileExtension(fileExt);

        if (sampleSourceExtension == fileExt)
        {
            // The following code that checks for the teapot sample sources is stolen from gdCallsStackListCtrl.cpp
            // This code should probably be moved to a lower module, so that it can be used by both the debugger
            // and profiler without each component having its own copy of this code.
            gtString sourceCodeFileName;
            pathToHandle.getFileName(sourceCodeFileName);

            // If the file is the GRTeaPot example
            // change the path to be relative to the installation directory
            static const gtString amdTeaPotLibSrcName1 = L"amdtteapotoclsmokesystem";
            static const gtString amdTeaPotLibSrcName2 = L"amdtteapotoglcanvas";
            static const gtString amdTeaPotLibSrcName3 = L"amdtteapotrenderstate";
            static const gtString amdTeaPotLibSrcName4 = L"amdtfluidgrid";
            static const gtString amdTeaPotLibSrcName5 = L"amdtimage";
            static const gtString amdTeaPotLibSrcName6 = L"amdtopenclhelper";
            static const gtString amdTeaPotLibSrcName7 = L"amdtopenglhelper";
            static const gtString amdTeaPotLibSrcName8 = L"amdtopenglmath";

            static const gtString amdTeaPotSrcName1 = L"amdtgtkmain";
            static const gtString amdTeaPotSrcName2 = L"amdtmainwin";
            static const gtString amdTeaPotSrcName3 = L"amdtteapot";

            gtString sourceCodeFileNameLower = sourceCodeFileName;
            sourceCodeFileNameLower.toLowerCase();

            bool isTPLibPath = (sourceCodeFileNameLower == amdTeaPotLibSrcName1) ||
                               (sourceCodeFileNameLower == amdTeaPotLibSrcName2) ||
                               (sourceCodeFileNameLower == amdTeaPotLibSrcName3) ||
                               (sourceCodeFileNameLower == amdTeaPotLibSrcName4) ||
                               (sourceCodeFileNameLower == amdTeaPotLibSrcName5) ||
                               (sourceCodeFileNameLower == amdTeaPotLibSrcName6) ||
                               (sourceCodeFileNameLower == amdTeaPotLibSrcName7) ||
                               (sourceCodeFileNameLower == amdTeaPotLibSrcName8);

            bool isTPSrcPath = (sourceCodeFileNameLower == amdTeaPotSrcName1) ||
                               (sourceCodeFileNameLower == amdTeaPotSrcName2);
            bool isTPPath = (sourceCodeFileNameLower == amdTeaPotSrcName3);

            if (isTPLibPath || isTPSrcPath || isTPPath)
            {
                // Get the path to be placed. This matches afGetApplicationRelatedFilePath.
                osFilePath exePath;
                bool rc1 = osGetCurrentApplicationDllsPath(exePath);

                if (!rc1)
                {
                    // If the dlls path is not set (such as in the standalone client), use the current application's path:
                    rc1 = osGetCurrentApplicationPath(exePath);
                }

                GT_IF_WITH_ASSERT(rc1)
                {
                    exePath.clearFileExtension().clearFileName();

                    if (isTPLibPath)
                    {
                        exePath.appendSubDirectory(L"examples")
                        .appendSubDirectory(L"Teapot")
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
                        .appendSubDirectory(L"AMDTTeaPotLib")
#endif
                        .appendSubDirectory(L"AMDTTeaPotLib")
                        .appendSubDirectory(L"src");
                    }
                    else if (isTPSrcPath)
                    {
                        exePath.appendSubDirectory(L"examples")
                        .appendSubDirectory(L"Teapot")
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
                        .appendSubDirectory(L"AMDTTeaPot")
#endif
                        .appendSubDirectory(L"AMDTTeaPot")
                        .appendSubDirectory(L"src");
                    }
                    else if (isTPPath)
                    {
                        exePath.appendSubDirectory(L"examples")
                        .appendSubDirectory(L"Teapot")
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
                        .appendSubDirectory(L"AMDTTeaPot")
#endif
                        .appendSubDirectory(L"AMDTTeaPot");
                    }

                    gtString localSamplePath = exePath.asString();
                    localSamplePath.removeTrailing(osFilePath::osPathSeparator);

                    pathToHandle.setFileDirectory(localSamplePath);
                }
            }
        }
    }
#else
    // Unused parameter:
    (void)(pathToHandle);
#endif
}

// *************** INTERNALLY-LINKED UTILITY FUNCTIONS - END ***************  //

// static members initialization
gtMap<gtString, bool> dmnSessionThread::m_sFrameAnalysisFileExtensionToBinaryfileTypeMap;
gtSet<gtString> dmnSessionThread::m_ProcessNamesTerminationSet;

dmnSessionThread::dmnSessionThread(osTCPSocketServerConnectionHandler* pConnHandler,
                                   const gtString& threadName, bool syncTermination) : osThread(threadName, syncTermination), m_pConnHandler(pConnHandler), m_rdsProcId(0), m_sProfProcId(0), m_sGraphicsProcId(0),
    m_powerBackendAdapter(this, m_pConnHandler), m_isForcedTerminationRequired(false)

{
    GT_ASSERT(pConnHandler != NULL);

    // Initialize the static extensions map if it was not initialized yet
    if (m_sFrameAnalysisFileExtensionToBinaryfileTypeMap.empty())
    {
        m_sFrameAnalysisFileExtensionToBinaryfileTypeMap[FRAME_DESCRITPION_FILE_EXT] = false;
        m_sFrameAnalysisFileExtensionToBinaryfileTypeMap[FRAME_IMAGE_FILE_EXT] = true;
        // Selectively decide which file to send.
        // LTR should not be sent because the CapturePlayer will send its data later when the client open's the frame timeline
        //m_sFrameAnalysisFileExtensionToBinaryfileTypeMap[FRAME_TRACE_FILE_EXT] = false;
    }
}



dmnSessionThread::~dmnSessionThread(void)
{
    releaseResources();
}


static bool HandleProcessStatusQuery(osChannel* pChannel, osProcessId processId)
{
    bool isOk = true;
    gtInt32 response = dssTerminated;

    if (processId > 0)
    {
        bool isAlive = false;
        isOk = osIsProcessAlive(processId, isAlive);
        GT_ASSERT_EX(isOk, L"DMN: Checking if process is alive.");

        if (isAlive)
        {
            response = dssAlive;
        }
    }

    // Transmit result.
    (*pChannel) << response;

    return isOk;
}

static bool HandleProcessTerminationRequest(osProcessId& processId)
{
    // before terminating the process - terminate its children. This prevents the children from remaining as zombies
    // and may also help to free the process itself.
    osTerminateChildren(processId);

    bool isOk = TerminateProcess(processId);

    if (isOk)
    {
        processId = 0;
    }

    return isOk;
}

static bool ReceiveRemoteFile(osChannel* pChannel, osDirectory& localTargetDir)
{
    dmnUtils::LogMessage(L"DMN: RECEIVING REMOTE FILE.", OS_DEBUG_LOG_DEBUG);

    bool ret = true;

    // First get file type.
    gtInt32 fileTypeBuffer = 0;
    (*pChannel) >> fileTypeBuffer;

    // Receive file name as CodeXLGpuProfiler knows it.
    if (fileTypeBuffer != dftMissingFile)
    {
        gtString expectedFileName;
        ret = pChannel->readString(expectedFileName);
        GT_ASSERT_EX(ret, L"Getting expected file name for CodeXLGpuProfiler file.");

        // Ack.
        ReportResult(ret, pChannel);

        if (ret)
        {
            // Fix the tokenized file name.
            gtString fixedExpectedFileName;
            FixTokenizedPathString(expectedFileName, fixedExpectedFileName);


            // Receive the file.
            osRawMemoryBuffer tmpBuffer;
            ret = tmpBuffer.readSelfFromChannel(*pChannel);

            // Send ack.
            ReportResult(ret, pChannel);

            GT_IF_WITH_ASSERT(ret)
            {
                osFilePath outputFilePath(fixedExpectedFileName);

                // Get the directory from the file path.
                gtString fileNameNoPath;
                outputFilePath.getFileNameAndExtension(fileNameNoPath);

                gtString dirToCreateStr;

                outputFilePath.clearFileExtension();
                outputFilePath.clearFileName();

                // Set local dir.
                localTargetDir = outputFilePath;

                if (!outputFilePath.exists())
                {
                    osDirectory dirToCreate;
                    ret = outputFilePath.getFileDirectory(dirToCreate);
                    GT_ASSERT_EX(ret, L"Failed getting file directory for CodeXLGpuProfiler files.");

                    if (ret)
                    {
                        dirToCreateStr = dirToCreate.directoryPath().asString();
                        dmnUtils::CreateDirHierarchy(dirToCreateStr);
                    }
                }

                outputFilePath.setFileName(fileNameNoPath);

                ret = tmpBuffer.toFile(outputFilePath);
                GT_ASSERT_EX(ret, L"DMN: Failed saving file to disk for CodeXLGpuProfiler.");

                // Trace the status.
                wstringstream msgStream;
                msgStream << L"DMN: Trying to receive the following file: " << fixedExpectedFileName.asCharArray();
                GT_IF_WITH_ASSERT(ret)
                {
                    msgStream << L"-> " << L"Successfully received and written to disk.";
                    dmnUtils::LogMessage(msgStream.str(), OS_DEBUG_LOG_DEBUG);
                }
                else
                {
                    msgStream << L"-> " << L"Failure on receiving file.";
                    dmnUtils::LogMessage(msgStream.str(), OS_DEBUG_LOG_ERROR);
                }
            }
        }
    }

    return ret;
}

int dmnSessionThread::entryPoint()
{
    int ret = -1;

    if (m_pConnHandler != NULL)
    {
        gtByte buffer[OPCODE_BUFFER_SIZE];
        bool isTerminationRequired = false;

        while (!isTerminationRequired && !m_isForcedTerminationRequired)
        {
            // Smoothen the busy waiting period.
            osSleep(LOOP_SLEEP_INTERVAL_MS);
            memset(buffer, 0, OPCODE_BUFFER_SIZE);
            bool isOk = ExtractOpCode(m_pConnHandler, buffer);
            GT_IF_WITH_ASSERT(isOk && m_pConnHandler->isOpen())
            {
                switch (buffer[0])
                {
                    case docLaunchRds:
                    {
                        LaunchRds();
                        break;
                    }

                    case docLaunchProfiler:
                    {
                        LaunchProfiler(isTerminationRequired);
                        break;
                    }

                    case docTerminateDebuggingSession:
                    {
                        isOk = TerminateDebuggingSession();
                        break;
                    }

                    case docTerminateProfilingSession:
                    {
                        isOk = TerminateProfilingSession();
                        break;
                    }

                    case docTerminateGraphicsBeckendServerSession:
                    {
                        isOk = TerminateGraphicsBeckendServerSession();
                        break;
                    }

                    case docTerminateWholeSession:
                    {
                        TerminateWholeSession(isTerminationRequired, ret);
                        break;
                    }

                    case docDebuggingSessionStatusQuery:
                    {
                        dmnUtils::LogMessage(L"DMN: RECEIVED REMOTE OPCODE -> DEBUGGING SESSION STATUS QUERY REQUEST.", OS_DEBUG_LOG_DEBUG);
                        isOk = HandleProcessStatusQuery(m_pConnHandler, m_rdsProcId);

                        break;
                    }

                    case docProfilingSessionStatusQuery:
                    {
                        dmnUtils::LogMessage(L"DMN: RECEIVED REMOTE OPCODE -> PROFILING SESSION STATUS QUERY REQUEST.", OS_DEBUG_LOG_DEBUG);
                        isOk = HandleProcessStatusQuery(m_pConnHandler, m_sProfProcId);

                        // Respond.
                        ReportResult(isOk, m_pConnHandler);
                        break;
                    }

                    case docGetRemoteFile:
                    {
                        GetRemoteFile();
                        break;
                    }

                    case docGetDaemonPlatform:
                    {
                        GetDaemonPlatform();
                        break;
                    }

                    case docGetDaemonCXLVersion:
                    {
                        isOk = GetDaemonCXLVersion();
                        break;
                    }

                    // RT Power Profiling.
                    case docPowerInit:
                    {
                        isOk = m_powerBackendAdapter.handlePowerSessionInitRequest();
                        GT_ASSERT_EX(isOk, L"DMN: Failed to init power profiling backend.");
                        break;
                    }

                    case docPowerSetSamplingConfig:
                    {
                        isOk = m_powerBackendAdapter.handlePowerSessionConfigRequest();
                        GT_ASSERT_EX(isOk, L"DMN: Failed to set power profiling sampling configuration.");
                        break;
                    }

                    case docPowerSetSamplingInterval:
                    {
                        isOk = m_powerBackendAdapter.handleSetSamplingIntervalMsRequest();
                        GT_ASSERT_EX(isOk, L"DMN: Failed to set the power profiling sampling interval.");
                        break;
                    }

                    case docPowerGetSystemTopology:
                    {
                        isOk = m_powerBackendAdapter.handleGetSystemTopologyRequest();
                        GT_ASSERT_EX(isOk, L"DMN: Failed to get the system topology.");
                        break;
                    }

                    case docPowerGetMinimumSamplingInterval:
                    {
                        isOk = m_powerBackendAdapter.handleGetSMinSamplingIntervalMsRequest();
                        GT_ASSERT_EX(isOk, L"DMN: Failed to get the min sampling interval.");
                        break;
                    }

                    case docPowerGetCurrentSamplingPeriod:
                    {
                        isOk = m_powerBackendAdapter.handleGetSCurrentSamplingIntervalMsRequest();
                        GT_ASSERT_EX(isOk, L"DMN: Failed to get the current sampling interval.");
                        break;
                    }

                    case docPowerEnableCounter:
                    {
                        isOk = m_powerBackendAdapter.handleEnableCounterRequest();
                        GT_ASSERT_EX(isOk, L"DMN: Failed to enable a power profiling counter.");
                        break;
                    }

                    case docPowerDisableCounter:
                    {
                        isOk = m_powerBackendAdapter.handleDisableCounterRequest();
                        GT_ASSERT_EX(isOk, L"DMN: Failed to disable a power profiling counter.");
                        break;
                    }

                    case docPowerIsCounterEnabled:
                    {
                        isOk = m_powerBackendAdapter.handleIsCounterEnabledRequest();
                        GT_ASSERT_EX(isOk, L"DMN: Failed to check if a power profiling counter is enabled.");
                        break;
                    }

                    case docPowerStartProfiling:
                    {
                        isOk = m_powerBackendAdapter.handleStartPowerProfilingRequest();
                        GT_ASSERT_EX(isOk, L"DMN: Failed to start the power profiling session.");
                        break;
                    }

                    case docPowerStopProfiling:
                    {
                        isOk = m_powerBackendAdapter.handleStopPowerProfilingRequest();
                        GT_ASSERT_EX(isOk, L"DMN: Failed to stop the power profiling session.");

                        // Terminate this session.
                        isTerminationRequired = true;
                        notifyUserAboutDisconnection();

                        break;
                    }

                    case docPowerDisconnectWithoutClosing:
                    {
                        // Terminate this session.
                        isTerminationRequired = true;
                        notifyUserAboutDisconnection();

                        break;
                    }

                    case docPowerPauseProfiling:
                    {
                        isOk = m_powerBackendAdapter.handlePausePowerProfilingRequest();
                        GT_ASSERT_EX(isOk, L"DMN: Failed to pause the power profiling session.");
                        break;
                    }

                    case docPowerResumeProfiling:
                    {
                        isOk = m_powerBackendAdapter.handleResumePowerProfilingRequest();
                        GT_ASSERT_EX(isOk, L"DMN: Failed to resume the power profiling session.");
                        break;
                    }

                    case docPowerClose:
                    {
                        isOk = m_powerBackendAdapter.handleClosePowerProfilingSessionRequest();
                        GT_ASSERT_EX(isOk, L"DMN: Failed to close the power profiling session.");
                        break;
                    }

                    case docPowerGetSamplesBatch:
                    {
                        isOk = m_powerBackendAdapter.handleReadAllEnabledCountersRequest();
                        GT_ASSERT_EX(isOk, L"DMN: Failed to read enabled counters.");
                        break;
                    }

                    case docPowerGetDeviceCounters:
                    {
                        isOk = m_powerBackendAdapter.handleGetDeviceCountersRequest();
                        GT_ASSERT_EX(isOk, L"DMN: Failed to get a power device's counters.");
                        break;
                    }

                    case docLaunchGraphicsBeckendServer:
                    {
                        isOk = LaunchGraphicsBeckendServer();
                        GT_ASSERT(isOk);
                        break;
                    }

                    case docGetCapturedFrames:
                    {
                        isOk = GetCapturedFrames();
                        GT_ASSERT_EX(isOk, L"docGetCapturedFrames Failed");
                        break;
                    }

                    case docGetCapturedFramesByTime:
                    {
                        isOk = GetCapturedFramesByTime();
                        GT_ASSERT_EX(isOk, L"docGetCapturedFramesByTime Failed");
                        break;
                    }

                    case docCapturedFrameData:
                    {
                        isOk = GetCapturedFrameData();
                        GT_ASSERT(isOk);
                        break;
                    }

                    case docDeleteFrameAnalysisSession:
                    {
                        isOk = DeleteFrameAnalysisSession();
                        GT_ASSERT(isOk);
                        break;
                    }

                    case docIsProcessRunning:
                    {
                        isOk = IsProcessRunning();
                        GT_ASSERT(isOk);
                        break;
                    }

                    case docKillRunningProcess:
                    {
                        isOk = KillRunningProcess();
                        GT_ASSERT(isOk);
                        break;
                    }
                    case docIsHSAEnabled:
                    {
                        isOk = IsHSAEnabled();
                        GT_ASSERT(isOk);
                        break;
                    }
                    case docValidateAppPaths:
                    {
                        isOk = ValidateAppPaths();
                        GT_ASSERT(isOk);
                        break;
                    }

                    default:
                    {
                        dmnUtils::LogMessage(L"DMN: RECEIVED REMOTE OPCODE -> DEFAULT.", OS_DEBUG_LOG_DEBUG);
                        break;
                    }
                }
            }
            else if (m_pConnHandler->isOpen() == false)
            {
                isTerminationRequired = true;
                OS_OUTPUT_DEBUG_LOG(L"Connection handler socket closed, terminating session", OS_DEBUG_LOG_INFO)
            }
        }// while (!isTerminationRequired && !m_isForcedTerminationRequired)
    }

    return ret;
}

bool dmnSessionThread::GetDaemonCXLVersion()
{
    bool isOk = false;
    // Extract the CXL version with which the daemon works.
    osProductVersion cxlProductVersion;
    osGetApplicationVersion(cxlProductVersion);

    gtString cxlVersionAsString = cxlProductVersion.toString();
    GT_ASSERT_EX(!cxlVersionAsString.isEmpty(), L"DMN: Unable to extract CXL version.");

    // Transfer the version to the client.
    isOk = m_pConnHandler->writeString(cxlVersionAsString);
    GT_ASSERT_EX(isOk, L"DMN: Failed writing client version.");
    return isOk;
}

void dmnSessionThread::GetDaemonPlatform()
{
    gtInt32 agentPlatform = dpUnknown;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    agentPlatform = dpWindows;
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    agentPlatform = dpLinux;
#endif

    // Send the agent platform.
    (*m_pConnHandler) << agentPlatform;
}

bool dmnSessionThread::TerminateGraphicsBeckendServerSession()
{
    bool isOk = false;
    dmnUtils::LogMessage(L"DMN: RECEIVED REMOTE OPCODE -> TERMINATE PERFSTUDIO SESSION REQUEST.", OS_DEBUG_LOG_DEBUG);
    isOk = HandleProcessTerminationRequest(m_sGraphicsProcId);
    GT_ASSERT_EX(isOk, L"PERFSTUDIO Termination.");

    // Respond.
    ReportResult(isOk, m_pConnHandler);                        return isOk;
}

bool dmnSessionThread::TerminateProfilingSession()
{
    bool isOk = false;
    dmnUtils::LogMessage(L"DMN: RECEIVED REMOTE OPCODE -> TERMINATE PROFILING SESSION REQUEST.", OS_DEBUG_LOG_DEBUG);
    isOk = HandleProcessTerminationRequest(m_sProfProcId);
    GT_ASSERT_EX(isOk, L"CodeXLGpuProfiler Termination.");

    // Respond.
    ReportResult(isOk, m_pConnHandler);
    return isOk;
}

bool dmnSessionThread::TerminateDebuggingSession()
{
    bool isOk = false;
    dmnUtils::LogMessage(L"DMN: RECEIVED REMOTE OPCODE -> TERMINATE DEBUGGING SESSION REQUEST.", OS_DEBUG_LOG_DEBUG);
    isOk = HandleProcessTerminationRequest(m_rdsProcId);
    GT_ASSERT_EX(isOk, L"RDS Termination.");

    // Respond.
    ReportResult(isOk, m_pConnHandler);
    return isOk;
}

bool dmnSessionThread::IsProcessRunning()
{
    bool retVal = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    // Read the process name sent by the CodeXL client
    GT_IF_WITH_ASSERT(m_pConnHandler != nullptr)
    {
        gtString  processName;
        retVal = m_pConnHandler->readString(processName);

        // Check if the process is alive
        bool isAlive = osIsProcessAlive(processName);

        (*m_pConnHandler) << isAlive;

        // Send a success status to CodeXL client after getting the command arguments
        ReportSuccess(m_pConnHandler);
    }

    return retVal;
}

bool dmnSessionThread::KillRunningProcess()
{
    bool retVal = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    // Read the process name sent by the CodeXL client
    GT_IF_WITH_ASSERT(m_pConnHandler != nullptr)
    {
        gtString  processName;
        retVal = m_pConnHandler->readString(processName);

        gtVector<gtString> processNames;
        processNames.push_back(processName);
        osTerminateProcessesByName(processNames);

        retVal = true;

        // Send a success status to CodeXL client after getting the command arguments
        ReportSuccess(m_pConnHandler);
    }

    return retVal;
}

bool dmnSessionThread::IsHSAEnabled()
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(m_pConnHandler != nullptr)
    {
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
        bool isHSAInstalled = false;
#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
        bool isHSAInstalled = oaIsHSADriver();
#endif
        (*m_pConnHandler) << isHSAInstalled;
        retVal = true;

        // Send a success status to CodeXL client after getting the command arguments
        ReportSuccess(m_pConnHandler);
    }

    return retVal;
}

bool dmnSessionThread::ValidateAppPaths()
{
    GT_ASSERT(m_pConnHandler != nullptr);

    bool retVal = true;

    gtString appPath;
    bool appPathValid = false;

    retVal &= m_pConnHandler->readString(appPath);
    osFilePath appFilePath(appPath);
    osFile appFile(appFilePath);
    appPathValid = appFile.IsExecutable();
    (*m_pConnHandler) << appPathValid;

    gtString workingDir;
    bool workDirValid = false;

    retVal &= m_pConnHandler->readString(workingDir);
    osFilePath workingDirFilePath(workingDir);
    osDirectory workingDirectory(workingDirFilePath);
    workDirValid = workingDirectory.exists();
    (*m_pConnHandler) << workDirValid;

    // Send a success status to CodeXL client after getting the command arguments
    ReportSuccess(m_pConnHandler);

    return retVal;
}

void dmnSessionThread::LaunchProfiler(bool& isTerminationRequired)
{
    bool isOk = false;
    // Tracing.
    dmnUtils::LogMessage(L"DMN: RECEIVED REMOTE OPCODE -> PROFILER LAUNCH.", OS_DEBUG_LOG_DEBUG);

    // Get the target application's full path.
    bool isTargetAppExists = false;
    gtString targetApp;
    isOk = m_pConnHandler->readString(targetApp);

    if (isOk)
    {
        osFilePath targetAppFilePath(targetApp);
        isTargetAppExists = targetAppFilePath.exists();
        (*m_pConnHandler) << isTargetAppExists;

    }

    if (isTargetAppExists)
    {
        gtString cmdLineArgsBuffer;
        isOk = m_pConnHandler->readString(cmdLineArgsBuffer);
        GT_ASSERT_EX(isOk, L"Reading cmd line args string for CodeXLGpuProfiler.");
        ReportResult(isOk, m_pConnHandler);

        if (isOk)
        {
            gtString fixedCmdLineArgs;
            isOk = FixCodeXLGpuProfilerCmdLineArgs(cmdLineArgsBuffer, fixedCmdLineArgs);
            GT_ASSERT_EX(isOk, L"Fixing CodeXLGpuProfiler cmd line args.");

            if (isOk)
            {
                // This is a temporary buffer.
                // The implementation will be changed when supporting different file naming on client and host.
                osDirectory profFilesDir;

                // ***************************************************
                // Get the four required files in the following order:
                // 1. Counter file.
                // 2. Env vars file.
                // 3. API filter file.
                // 4. API rules file.
                // ***************************************************

                // Then make sure you get a string with the file type before each file.
                isOk = ReceiveRemoteFile(m_pConnHandler, profFilesDir);
                GT_ASSERT_EX(isOk, L"Receiving counters file for CodeXLGpuProfiler.");
                isOk = ReceiveRemoteFile(m_pConnHandler, profFilesDir);
                GT_ASSERT_EX(isOk, L"Receiving env vars file for CodeXLGpuProfiler.");
                isOk = ReceiveRemoteFile(m_pConnHandler, profFilesDir);
                GT_ASSERT_EX(isOk, L"Receiving api filters for CodeXLGpuProfiler.");
                isOk = ReceiveRemoteFile(m_pConnHandler, profFilesDir);
                GT_ASSERT_EX(isOk, L"Receiving api rules file for CodeXLGpuProfiler.");

                // Check if we still need to create the output directory.
                // This will happen in case no files are sent by the client.
                isOk = SetupCodeXLGpuProfilerOutputDir(fixedCmdLineArgs, profFilesDir);

                // Ack.
                ReportResult(isOk, m_pConnHandler);

                // Prepare a container to hold specific kernel names (if required).
                gtVector<gtString> specificKernelNames;

                // Is it a kernel-specific session.
                bool isKernelSpecific = false;
                (*m_pConnHandler) >> isKernelSpecific;

                osFile kernelListFile;

                if (isKernelSpecific)
                {
                    gtUInt32 numOfKernels = 0;
                    (*m_pConnHandler) >> numOfKernels;

                    gtString currKernelName;

                    // For security reasons.
                    const gtUInt32 MAX_NUM_OF_KERNELS = 256;

                    if (numOfKernels < MAX_NUM_OF_KERNELS)
                    {
                        while (numOfKernels-- > 0)
                        {
                            (*m_pConnHandler) >> currKernelName;
                            specificKernelNames.push_back(currKernelName);
                            currKernelName.makeEmpty();
                        }
                    }

                    // Append the kernel specific section CodeXLGpuProfiler's command line arguments.
                    gtString kernelSpecificCmd = L" --kernellistfile ";
                    osFilePath kernelListFilePath = profFilesDir.directoryPath();
                    kernelListFilePath.setFileName(L"specificKernels");

                    // Create the kernel list text file.
                    kernelListFile = osFile(kernelListFilePath);
                    kernelListFile.open(osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

                    for (const gtString& kernelName : specificKernelNames)
                    {
                        gtASCIIString asciiStr = kernelName.asASCIICharArray();
                        asciiStr.append("\n");
                        osChannel& fileAsChannel = kernelListFile;
                        fileAsChannel.writeString(asciiStr);
                    }

                    kernelListFile.close();

                    // Adjust the command line string.
                    gtString specificKernelsFileName = kernelListFilePath.asString();
                    int lastQuoteChar = fixedCmdLineArgs.reverseFind(L'\"');
                    lastQuoteChar = fixedCmdLineArgs.reverseFind(L'\"', lastQuoteChar - 1);

                    // Extract the target application's full path.
                    gtString targetAppFullPath;
                    int endPos = fixedCmdLineArgs.length();
                    fixedCmdLineArgs.getSubString(lastQuoteChar, endPos, targetAppFullPath);

                    // Rebuild the command line string.
                    fixedCmdLineArgs.replace(targetAppFullPath, L"");
                    fixedCmdLineArgs << kernelSpecificCmd << L"\"" << specificKernelsFileName << L"\" " << targetAppFullPath;
                }

                GT_IF_WITH_ASSERT(isOk)
                {
                    // Trace.
                    wstringstream logMsgStream;
                    logMsgStream << L"CodeXLGpuProfiler command line args string: ";
                    logMsgStream << fixedCmdLineArgs.asCharArray();
                    dmnUtils::LogMessage(logMsgStream.str(), OS_DEBUG_LOG_DEBUG);

                    // Currently env vars for CodeXLGpuProfiler are empty.
                    vector<osEnvironmentVariable> envVars;

                    // Create the process.
                    isOk = CreateProcess(romPROFILE, fixedCmdLineArgs, envVars);
                    GT_ASSERT(isOk);

                    // Respond.
                    ReportResult(isOk, m_pConnHandler);

                    // Now wait for the process to terminate.
                    if (isOk)
                    {
                        // First, pack up the parent processes, whose children
                        // need to be terminated in case of communication failure.
                        gtVector<osProcessId> parentProcesses;
                        parentProcesses.push_back(m_rdsProcId);
                        parentProcesses.push_back(m_sProfProcId);

                        // Launch the communication watcher thread.
                        dmnConnectionWatcherThread* pWatcher = new dmnConnectionWatcherThread(L"Connection Watcher Thread", m_pConnHandler, parentProcesses);
                        bool isWatcherThreadLaunchSuccessful = pWatcher->execute();
                        GT_ASSERT(isWatcherThreadLaunchSuccessful);

                        // Wait as long as the Profiler process is alive.
                        long exitCode = 0;
                        isOk = osWaitForProcessToTerminate(m_sProfProcId, ULONG_MAX, &exitCode);
                        GT_ASSERT_EX(isOk, L"Error on waiting for process to terminate.");
                        m_rdsProcId = 0x0;

                        // Delete the kernel list file if it exists.
                        if (isKernelSpecific && kernelListFile.exists())
                        {
                            kernelListFile.deleteFile();
                        }

                        // Check if this session's connection is broken.
                        // If it is broken, this session should be terminated.
                        isTerminationRequired = pWatcher->isConnectionBroken();

                        // Release the watcher thread.
                        pWatcher->terminate();
                        delete pWatcher;
                        pWatcher = NULL;
                    }

                    // If this session's connection is broken, this thread should be terminated.
                    if (!isTerminationRequired)
                    {

                        // Let the filesystem update.
                        // Without this sleep, it happens that the getContainedFilePaths() function
                        // captures the former state of the file system (the state before CodeXLGpuProfiler's execution).
                        dmnConfigManager* pConfigMgr = dmnConfigManager::Instance();
                        GT_IF_WITH_ASSERT(pConfigMgr != NULL)
                        {
                            // Now profiling is finished. Pass all files in the working dir.
                            const gtString fileFilter = L"*";
                            gtList<osFilePath> containedFiles;

                            isOk = profFilesDir.getContainedFilePaths(fileFilter,
                                                                      osDirectory::SORT_BY_NAME_ASCENDING, containedFiles);

                            GT_ASSERT_EX(isOk, L"Failed retrieving the number of CodeXLGpuProfiler output files.");

                            // Notify the client how many files are required.
                            gtInt32 numOfFiles = containedFiles.size();
                            (*m_pConnHandler) << numOfFiles;

                            // Verify.
                            gtInt32 opStatus = dosFailure;
                            (*m_pConnHandler) >> opStatus;
                            isOk = (opStatus == dosSuccess);
                            GT_ASSERT_EX(isOk, L"DMN: Verifying that the client received the number of output files.");

                            // Now actually send the files.
                            for (gtList<osFilePath>::iterator iter = containedFiles.begin();
                                 iter != containedFiles.end(); iter++)
                            {
                                // Send file name.
                                gtString sprofOutfileName;
                                gtString sprofOutfileExtension;
                                (*iter).getFileName(sprofOutfileName);
                                (*iter).getFileExtension(sprofOutfileExtension);

                                // Currently compressed by default.
                                // const bool isCompressed = true;

                                // if (isCompressed)
                                {
                                    // Concatenate the compressed file suffix.
                                    sprofOutfileName.append(COMPRESSED_FILE_SUFFIX);
                                }

                                if (!sprofOutfileExtension.isEmpty())
                                {
                                    sprofOutfileExtension.prepend(L'.');
                                    sprofOutfileName.append(sprofOutfileExtension);
                                }

                                isOk = m_pConnHandler->writeString(sprofOutfileName);
                                GT_ASSERT_EX(isOk, L"DMN: Failed transferring CodeXLGpuProfiler file name to the client.");

                                // Transfer the file.
                                TransferFile(*iter, m_pConnHandler);

                                // Verify.
                                opStatus = dosFailure;
                                (*m_pConnHandler) >> opStatus;
                                isOk = (opStatus == dosSuccess);
                                GT_ASSERT_EX(isOk, L"DMN: Verifying that the client successfully received an CodeXLGpuProfiler output files.");
                            }
                        }
                    }// if (!isTerminationRequired)
                }// GT_IF_WITH_ASSERT(isOk)
            }
        }
    }
    else
    {
        // Terminate this session.
        isTerminationRequired = true;
        notifyUserAboutDisconnection();
    }
}


void dmnSessionThread::AddFADependantProcessToTerminationList(const osFilePath& serverPath, const gtString& arguments) const
{
    gtString processFilePathToKill;
    serverPath.getFileNameAndExtension(processFilePathToKill);
    ///update process termination list
    m_ProcessNamesTerminationSet.insert(processFilePathToKill);
    ///update process termination list
    int processNameTokenPosition = arguments.findFirstOf(gtString(L" "));

    if (processNameTokenPosition > 0)
    {
        processFilePathToKill.makeEmpty();
        arguments.getSubString(0, processNameTokenPosition, processFilePathToKill);

        if (processFilePathToKill.find(DMN_STR_FA_playerName) >= 0)
        {
            //remove quotes
            processFilePathToKill.removeChar(L'\"');
            osFilePath(processFilePathToKill).getFileNameAndExtension(processFilePathToKill);
            ///update process termination list
            m_ProcessNamesTerminationSet.insert(processFilePathToKill.trim());
        }
    }
}

bool dmnSessionThread::LaunchGraphicsBeckendServer()
{
    bool isOk = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(isOk);
    // Tracing.
    dmnUtils::LogMessage(L"DMN: RECEIVED REMOTE OPCODE -> GraphicsBeckendServer LAUNCH.", OS_DEBUG_LOG_DEBUG);

    // 1. Extract the server path.
    osFilePath serverPath;
    isOk = ExtractServerPath(m_pConnHandler, serverPath);

    if (isOk)
    {
        osFilePath realServerPath;
        osGetCurrentApplicationPath(realServerPath, false);

        gtString fileName;
        gtString fileExt;
        serverPath.getFileExtension(fileExt);
        serverPath.getFileName(fileName);
        realServerPath.setFileName(fileName);
        realServerPath.setFileExtension(fileExt);

        bool isTargetAppExists = false;
        isTargetAppExists = realServerPath.exists();
        (*m_pConnHandler) << isTargetAppExists;

        if (isTargetAppExists)
        {
            // Kill the server process if it exists
            KillServerExistingProcess(realServerPath);

            // 2. Extract the command line args
            gtString cmdLineArgsBuffer;
            ExtractCmdLineString(m_pConnHandler, cmdLineArgsBuffer);

            gtString fixedCmdLineArgs;
            isOk = FixCodeXLGpuProfilerCmdLineArgs(cmdLineArgsBuffer, fixedCmdLineArgs);

            GT_ASSERT_EX(isOk, L"Fixing CodeXLGpuProfiler cmd line args.");

            const auto port = GetGraphicServerPortFromArgs(fixedCmdLineArgs);
            bool isGraphicServerPortAvailable = true;
            isOk = isGraphicServerPortAvailable = osIsLocalPortAvaiable(port);

            //report if port is available
            (*m_pConnHandler) << isGraphicServerPortAvailable;

            if (isGraphicServerPortAvailable)
            {
                AddFADependantProcessToTerminationList(realServerPath, fixedCmdLineArgs);

                // 3. Extract the work dir.
                gtString workDirectoryStr;
                m_pConnHandler->readString(workDirectoryStr);
                osFilePath workDirectory = workDirectoryStr;

                if (m_sGraphicsProcId == 0)
                {
                    vector<osEnvironmentVariable> envVars;

                    // Create the process.
                    isOk = CreateProcess(romGRAPHICS, fixedCmdLineArgs, envVars, realServerPath, workDirectory);

                    GT_ASSERT(isOk);
                }
                else
                {
                    // We already have a working session.
                    isOk = false;
                    dmnUtils::LogMessage(L"Another PerfStudio session is already running.", OS_DEBUG_LOG_ERROR);
                }
            }
        }
    }

    // Respond.
    ReportResult(isOk, m_pConnHandler);
    return isOk;
}

unsigned short dmnSessionThread::GetGraphicServerPortFromArgs(const gtString& fixedCmdLineArgs) const
{
    unsigned short port = 8080;//default

    //find port in args
    try
    {
        vector<wstring> argsSplitted;
        wstring argsAsString = fixedCmdLineArgs.asCharArray();
        boost::algorithm::split_regex(argsSplitted, argsAsString, boost::wregex(L"--port"));

        if (argsSplitted.size() > 1)
        {
            wstring portStr = argsSplitted[1];

            portStr.erase(remove(portStr.begin(), portStr.end(), '\"'), portStr.end());
            boost::trim(portStr);
            port = boost::lexical_cast<unsigned short>(portStr);
        }
    }
    catch (...)
    {
        GT_ASSERT(false)
    }

    return port;
}

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
static bool IsProfiledApp64Bit(const gtString& cmdLineArgs, bool& buffer)
{
    bool ret = false;
    gtString exePath;

    int firstQuote = cmdLineArgs.reverseFind(L'"');
    GT_IF_WITH_ASSERT(firstQuote > 0)
    {
        int secondQuote = cmdLineArgs.reverseFind(L'"', firstQuote - 1);
        GT_IF_WITH_ASSERT(secondQuote > 0 && ((secondQuote + 1) < (firstQuote - 1)))
        {
            cmdLineArgs.getSubString(secondQuote + 1, firstQuote - 1, exePath);
            osFilePath exeFile(exePath);
            GT_IF_WITH_ASSERT(exeFile.exists())
            {
                ret = osIs64BitModule(exePath, buffer);
                GT_ASSERT_EX(ret, L"DMN: Failed to check profiled app's bitness.");
            }
        }
    }
    return ret;
}
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
static bool IsProfiledApp64Bit(const gtString& cmdLineArgs, bool& buffer)
{
    // Unused parameters:
    (void)(cmdLineArgs);

    // Currently consider all Linux files as 64-bit:
    buffer = true;
    return true;
}
#else
#error Unknown build target!
#endif

bool dmnSessionThread::CreateProcess(REMOTE_OPERATION_MODE mode, const gtString& cmdLineArgs, vector<osEnvironmentVariable>& envVars, const osFilePath& filePath, const osFilePath& dirPath)
{
    osFilePath filePathTemp = filePath;
    osFilePath dirPathTemp = dirPath;
    osProcessId procId;
    osProcessHandle procHandle = 0;
    osThreadHandle procThreadHandle = 0;


    bool isOk = false;

    // Indicates whether the target application (the application which we want to debug/profile) is 64-bit.
    // Note that on linux we treat 64-bit as the default.
    bool is64BitTarget = true;

    if (mode == romPROFILE)
    {
        // Find out the bitness.
        isOk = IsProfiledApp64Bit(cmdLineArgs, is64BitTarget);
        GT_ASSERT_EX(isOk, L"DMN: Failed to check if profiled app's bitness.");
    }

    if (filePathTemp.isEmpty() || dirPathTemp.isEmpty())
    {
        isOk = FillPathBuffers(mode, is64BitTarget, filePathTemp, dirPathTemp);
    }
    else
    {
        // perf studio paths are passed as arguments
        isOk = true;
    }

    GT_ASSERT(isOk);

    // Create the environment for the future child.
    // DTOR will clean the environment after child creation.
    osEnvVarScope envScope(envVars);

    // Create the RDS/CodeXLGpuProfiler as a suspended process.
    // Note no regressions occur for the debugging scenario (which worked correctly with L"" for the cmdLineArgs.
    isOk = osLaunchSuspendedProcess(filePathTemp, cmdLineArgs, dirPathTemp, procId, procHandle, procThreadHandle);

    GT_IF_WITH_ASSERT(isOk)
    {
        isOk = osResumeSuspendedProcess(procId, procHandle, procThreadHandle, true);
        GT_IF_WITH_ASSERT(isOk)
        {
            if (mode == romDEBUG)
            {
                m_rdsProcId = procId;
            }
            else if (mode == romPROFILE)
            {
                m_sProfProcId = procId;
            }
            else if (mode == romGRAPHICS)
            {
                m_sGraphicsProcId = procId;
            }
        }
        else
        {
            wstringstream stream;
            stream << L"Unable to resume a process: " << filePathTemp.asString().asCharArray() << endl;
            dmnUtils::LogMessage(stream.str(), OS_DEBUG_LOG_ERROR);
        }
    }
    else
    {
        wstringstream stream;
        stream << L"Unable to create a process: " << filePathTemp.asString().asCharArray() << endl;
        dmnUtils::LogMessage(stream.str(), OS_DEBUG_LOG_ERROR);
    }

    return isOk;
}

bool dmnSessionThread::SendFrameAnalysisFileData(const osFilePath& filePath)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pConnHandler != nullptr)
    {
        gtString extension;
        filePath.getFileExtension(extension);
        GT_IF_WITH_ASSERT(m_sFrameAnalysisFileExtensionToBinaryfileTypeMap.find(extension) != m_sFrameAnalysisFileExtensionToBinaryfileTypeMap.end())
        {
            bool isBinary = m_sFrameAnalysisFileExtensionToBinaryfileTypeMap[extension];

            // Send the file extension
            m_pConnHandler->writeString(extension);

            // Send the isBinary flag to CodeXL client
            (*m_pConnHandler) << isBinary;

            unique_ptr <gtByte[]> pBuffer;
            unsigned long fileSize;
            retVal = ReadFile(filePath, pBuffer, fileSize);
            GT_IF_WITH_ASSERT(retVal)
            {
                // Write the file size to the CodeXL client
                (*m_pConnHandler) << fileSize;

                // Send the binary file content
                retVal = m_pConnHandler->write(pBuffer.get(), fileSize);
                GT_ASSERT(retVal);
            }
        }
    }

    return retVal;
}

bool dmnSessionThread::ReadFile(const osFilePath& filePath, std::unique_ptr <gtByte[]>& pBuffer, unsigned long& fileSize) const
{
    // Read the file size
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS //on windows we have unicode file names
    const wchar_t*   fname = filePath.asString().asCharArray();
#else
    const char* fname = filePath.asString().asASCIICharArray();
#endif
    ifstream file(fname, ios::binary | ios::ate);
    fileSize = static_cast<unsigned long>(file.tellg());
    file.seekg(0, ios::beg);

    // Allocate a buffer and read the file content into it
    pBuffer.reset(new gtByte[fileSize]());

    bool retVal = file.read(pBuffer.get(), fileSize) ? true : false;
    return retVal;
}

bool dmnSessionThread::terminateProcess(REMOTE_OPERATION_MODE mode)
{
    bool ret = false;
    osProcessId procId = 0;

    if (mode == romDEBUG)
    {
        procId = m_rdsProcId;
        m_rdsProcId = 0x0;
    }
    else if (mode == romPROFILE)
    {
        procId = m_sProfProcId;
        m_sProfProcId = 0x0;
    }
    else if (mode == romGRAPHICS)
    {
        procId = m_sGraphicsProcId;
        m_sGraphicsProcId = 0x0;
    }

    ret = TerminateProcess(procId);
    GT_ASSERT(ret);

    if (!ret)
    {
        wstringstream stream;
        stream << "Unable to terminate process with mode: " << dmnUtils::OpModeToString(mode) << ".";
        dmnUtils::LogMessage(stream.str(), OS_DEBUG_LOG_ERROR);
    }

    return ret;
}

void dmnSessionThread::releaseResources()
{
    if (m_pConnHandler != NULL)
    {
        m_pConnHandler->close();
        delete m_pConnHandler;
    }

    if (m_rdsProcId != 0)
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        // First, terminate RDS's child processes.
        // This is currently only supported on Windows.
        osTerminateChildren(m_rdsProcId);
#endif
        // Terminate the RDS itself.
        TerminateProcess(m_rdsProcId);
    }

    if (m_sProfProcId != 0)
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        // First, terminate RDS's child processes.
        // This is currently only supported on Windows.
        osTerminateChildren(m_sProfProcId);
#endif

        TerminateProcess(m_sProfProcId);
    }

    if (m_sGraphicsProcId != 0)
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        // First, terminate PerfStudio's child processes.
        // This is currently only supported on Windows.
        osTerminateChildren(m_sGraphicsProcId);
#endif

        TerminateProcess(m_sGraphicsProcId);
    }
}
void dmnSessionThread::KillDependantProcesses()
{
    osProcessesEnumerator processEnum;

    if (processEnum.initialize())
    {
        osProcessId processId = 0;
        gtString executableName;

        while (m_ProcessNamesTerminationSet.empty() == false && processEnum.next(processId, &executableName))
        {
            if (((osProcessId)0) == processId)
            {
                continue;
            }

            auto itr = m_ProcessNamesTerminationSet.find(executableName);

            if (itr != m_ProcessNamesTerminationSet.end())
            {
                // Terminate the process
                osTerminateProcess(processId);
                m_ProcessNamesTerminationSet.erase(itr);
            }
        }
    }
}


void dmnSessionThread::KillServerExistingProcess(const osFilePath& serverPath)
{
    gtString fileName;
    serverPath.getFileNameAndExtension(fileName);
    gtVector<gtString> fileNamesToTerminate;
    fileNamesToTerminate.push_back(fileName);

    osTerminateProcessesByName(fileNamesToTerminate);
}

bool dmnSessionThread::GetCapturedFramesByTime()
{
    bool retVal = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    gtString  projectName;
    gtString sessionName;
    m_pConnHandler->readString(projectName);
    m_pConnHandler->readString(sessionName);
    gtInt64 timeSinceEpoch = -1;
    (*m_pConnHandler) >> timeSinceEpoch;
    GT_ASSERT(timeSinceEpoch != -1);

    osTime frameMinimalTime(timeSinceEpoch);
    auto sessionFilter = [&](const osFilePath & sessionCurrent)
    {
        gtString currentSessionName;
        sessionCurrent.getFileNameAndExtension(currentSessionName);
        bool bRes = 0 == currentSessionName.compareNoCase(sessionName);
        return bRes;
    };

    auto frameFilter = [&](const osFilePath & frameCurrent)
    {
        bool bRes = false;
        osStatStructure fileStruct;
        GT_IF_WITH_ASSERT(0 == osWStat(frameCurrent.asString(), fileStruct))
        {
            osTime frameCurrentTime(fileStruct.st_atime);
            bRes = (frameCurrentTime > frameMinimalTime);
        }
        return bRes;
    };

    retVal = CreateCapturedFramesInfoFile(projectName, sessionFilter, frameFilter);

    return retVal;
}

bool dmnSessionThread::GetCapturedFrameData()
{
    // Read the project name sent by the CodeXL client
    bool retVal = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);
    GT_IF_WITH_ASSERT(m_pConnHandler != nullptr)
    {
        gtString  projectName;
        retVal = m_pConnHandler->readString(projectName);

        // Read the session name sent by the CodeXL client
        gtString sessionName;
        retVal = m_pConnHandler->readString(sessionName);

        // Read the frame index sent by the CodeXL client
        int  frameIndex = 0;
        (*m_pConnHandler) >> frameIndex;

        // Send a success status to CodeXL client after getting the command arguments
        ReportSuccess(m_pConnHandler);

        WriteFrameFilesData(frameIndex, projectName, sessionName);
    }


    return retVal;
}

bool dmnSessionThread::DeleteFrameAnalysisSession()
{
    // Read the project name sent by the CodeXL client
    bool retVal = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    GT_IF_WITH_ASSERT(m_pConnHandler != nullptr)
    {
        gtString  projectName;
        retVal = m_pConnHandler->readString(projectName);

        // Read the session name sent by the CodeXL client
        gtString sessionName;
        retVal = m_pConnHandler->readString(sessionName);

        osFilePath sessionFolderPath;
        GT_IF_WITH_ASSERT(GetCurrentUserFrameAnalysisFolder(sessionFolderPath))
        {
            sessionFolderPath.appendSubDirectory(projectName);
            sessionFolderPath.appendSubDirectory(sessionName);
            osDirectory sessionDir;
            retVal = sessionFolderPath.getFileDirectory(sessionDir);
            GT_IF_WITH_ASSERT(retVal)
            {
                if (sessionDir.exists())
                {
                    retVal = sessionDir.deleteRecursively();
                }
                else
                {
                    retVal = true;
                }
            }
        }

        // Send a success status to CodeXL client after getting the command arguments
        ReportResult(retVal, m_pConnHandler);
    }

    return retVal;
}

void dmnSessionThread::WriteFrameFilesData(int frameIndex, const gtString& projectName, const gtString& sessionName)
{
    osFilePath frameFolderPath;
    GT_IF_WITH_ASSERT(GetCurrentUserFrameAnalysisFolder(frameFolderPath))
    {
        gtString frameFolderName;
        frameFolderName.appendFormattedString(L"Frame_%010d", frameIndex);

        frameFolderPath.appendSubDirectory(projectName).appendSubDirectory(sessionName).appendSubDirectory(frameFolderName);
        gtList<osFilePath> framesFiles;
        //frame path is folder
        gtString   descriptionFilePath;
        int fileCount = 0;

        osDirectory frameDir(frameFolderPath.asString());
        GT_IF_WITH_ASSERT(frameDir.getContainedFilePaths(L"*.*", osDirectory::SORT_BY_NAME_ASCENDING, framesFiles))
        {
            // Count the files that needs to be sent
            for (auto filePath : framesFiles)
            {
                gtString fileExtention;
                gtString fileName;
                filePath.getFileExtension(fileExtention);
                filePath.getFileName(fileName);

                if (m_sFrameAnalysisFileExtensionToBinaryfileTypeMap.find(fileExtention) != m_sFrameAnalysisFileExtensionToBinaryfileTypeMap.end())
                {
                    // look for file "description-*.xml"
                    if (0 == fileExtention.compareNoCase(FRAME_DESCRITPION_FILE_EXT) && (fileName.find(FRAME_DESCRITPION_FILE_PATTERN) == 0))
                    {
                        descriptionFilePath = filePath.asString();
                    }

                    fileCount++;
                }
            }
        }

        // Send the description file path
        m_pConnHandler->writeString(descriptionFilePath);
        // Send the file count to CodeXL client
        (*m_pConnHandler) << fileCount;

        for (auto filePath : framesFiles)
        {
            gtString fileExtention;
            filePath.getFileExtension(fileExtention);

            if (m_sFrameAnalysisFileExtensionToBinaryfileTypeMap.find(fileExtention) != m_sFrameAnalysisFileExtensionToBinaryfileTypeMap.end())
            {
                SendFrameAnalysisFileData(filePath);
            }
        }
    }
}

bool dmnSessionThread::GetCapturedFrames()
{
    bool retVal = false;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    gtString  projectName;
    retVal = m_pConnHandler->readString(projectName);
    ReportResult(retVal, m_pConnHandler);
    GT_IF_WITH_ASSERT(retVal)
    {
        auto dummyFunc = [](const osFilePath&) { return true; };
        retVal = CreateCapturedFramesInfoFile(projectName, dummyFunc, dummyFunc);
    }
    return retVal;
}

bool dmnSessionThread::CreateCapturedFramesInfoFile(const gtString& projectName, const FilePathFilter& sessionFilterFunc, const FilePathFilter& frameFilterFunc)
{
    bool retVal = false;
    osFilePath amdDataRoot;
    GT_IF_WITH_ASSERT(GetCurrentUserFrameAnalysisFolder(amdDataRoot))
    {
        amdDataRoot.appendSubDirectory(projectName);
        osDirectory dir(amdDataRoot);
        gtList<osFilePath> subDirectoriesPaths;
        retVal = dir.getSubDirectoriesPaths(osDirectory::SORT_BY_NAME_ASCENDING, subDirectoriesPaths);

        TiXmlDocument doc;
        TiXmlDeclaration* decl = new TiXmlDeclaration(DMN_STR_FA_sessionInfoXMLVersion, "", "");
        doc.LinkEndChild(decl);
        TiXmlElement* sessions = new TiXmlElement(DMN_STR_FA_sessionInfoXMLSessions);
        doc.LinkEndChild(sessions);

        for (auto sessionDirPath : subDirectoriesPaths)
        {
            if (sessionFilterFunc(sessionDirPath) == false)
            {
                continue;//skip this session
            }

            TiXmlElement* sessionElement = new TiXmlElement(DMN_STR_FA_sessionInfoXMLSession);
            gtString sessionName;
            sessionDirPath.getFileNameAndExtension(sessionName);//its folder name so we don't split the extension
            sessionElement->SetAttribute(DMN_STR_FA_sessionInfoXMLName, sessionName.asASCIICharArray());

            gtList<osFilePath> sessionFramesPaths;
            //session path is folder
            osDirectory sessionDir(sessionDirPath.asString());

            GT_IF_WITH_ASSERT(sessionDir.getSubDirectoriesPaths(osDirectory::SORT_BY_NAME_ASCENDING, sessionFramesPaths))
            {
                for (auto framePath : sessionFramesPaths)
                {
                    if (frameFilterFunc(framePath) == false)
                    {
                        continue;//skip this frame
                    }

                    gtString frameName;
                    framePath.getFileName(frameName);
                    TiXmlElement* frameElement = new TiXmlElement(DMN_STR_FA_sessionInfoXMLFrame);
                    // We're cutting off "Frame_" prefix(5 characters)
                    frameName.getSubString(5, frameName.length(), frameName);

                    if (frameName.startsWith(L"_"))
                    {
                        frameName.getSubString(1, frameName.length(), frameName);
                    }

                    // Try to convert the string to an integer, to make sure that the name is valid
                    int frameIndex = -1;
                    bool rc = frameName.toIntNumber(frameIndex);
                    GT_ASSERT(rc);
                    frameElement->SetAttribute(DMN_STR_FA_sessionInfoXMLIndex, frameIndex);
                    sessionElement->LinkEndChild(frameElement);

                    //frame path is folder
                    gtList<osFilePath> framesFiles;
                    osDirectory frameDir(framePath.asString());
                    GT_IF_WITH_ASSERT(frameDir.getContainedFilePaths(OS_ALL_CONTAINED_FILES_SEARCH_STR, osDirectory::SORT_BY_NAME_ASCENDING, framesFiles))
                    {
                        BuildFrameCaptureInfoNode(framesFiles, frameElement);
                    }
                }
            }

            sessions->LinkEndChild(sessionElement);
        }

        TiXmlPrinter printer;
        printer.SetIndent("\t");
        doc.Accept(&printer);
        string xmltext = printer.CStr();
        gtString xmlWideStr;
        xmlWideStr.fromASCIIString(xmltext.c_str(), xmltext.length());
        m_pConnHandler->writeString(xmlWideStr);


    }
    return retVal;
}

void dmnSessionThread::BuildFrameCaptureInfoNode(const gtList<osFilePath>& framesFiles, TiXmlElement* frameElement) const
{
    GT_IF_WITH_ASSERT(frameElement != nullptr)
    {
        for (auto frameFile : framesFiles)
        {
            TiXmlElement* fileElement = nullptr;
            gtString fileExt;
            frameFile.getFileExtension(fileExt);

            if (0 == fileExt.compareNoCase(FRAME_TRACE_FILE_EXT))
            {
                fileElement = new TiXmlElement(FRAME_TRACE_FILE_EXT.asASCIICharArray());
            }
            else if (0 == fileExt.compareNoCase(FRAME_DESCRITPION_FILE_EXT))
            {
                fileElement = new TiXmlElement("description");
            }
            else if (0 == fileExt.compareNoCase(FRAME_IMAGE_FILE_EXT))
            {
                fileElement = new TiXmlElement("image");
            }
            else
            {
                gtString errMsg = L"Unknown file format : ";
                errMsg.append(frameFile.asString());
                GT_ASSERT_EX(false, errMsg.asCharArray());
            }

            GT_IF_WITH_ASSERT(fileElement != nullptr)
            {
                frameElement->LinkEndChild(fileElement);
            }
        }
    }
}

void dmnSessionThread::notifyUserAboutDisconnection()
{
    // Notify the user that we are disconnecting.
    // Note that the whole mechanism of printing to cout should become
    // event-based for thread safety in the future.
    {
        osPortAddress peerAddr;
        bool isOk = m_pConnHandler->getPeerHostAddress(peerAddr);
        GT_ASSERT_EX(isOk, L"DMN Session thread: failed to extract peer's address during disconnection.");
        wstringstream msgStream;
        msgStream << endl << DMN_STR_SESSION_DISCONNECTION_A;

        if (isOk)
        {
            gtString peerAddrStr;
            peerAddr.toString(peerAddrStr);
            msgStream << L"<" << peerAddrStr.asCharArray() << L">";
        }

        msgStream << DMN_STR_SESSION_DISCONNECTION_B << endl;
        wcout << msgStream.str();
    }
}

void dmnSessionThread::terminateSession()
{
    // Notify the user.
    notifyUserAboutDisconnection();

    // Terminate.
    m_isForcedTerminationRequired = true;
}

bool dmnSessionThread::GetCurrentUserFrameAnalysisFolder(osFilePath& buffer) const
{
    bool ret = buffer.setPath(osFilePath::OS_TEMP_DIRECTORY);
    GT_IF_WITH_ASSERT(ret)
    {
        buffer.appendSubDirectory(DWM_STR_CODEXL_DIR_NAME);
    }
    return ret;
}

bool dmnSessionThread::GetRemoteFile()
{
    dmnUtils::LogMessage(L"DMN: RECEIVED REMOTE OPCODE -> REMOTE FILE REQUEST.", OS_DEBUG_LOG_DEBUG);

    // Get the file name.
    gtString fileName;
    bool isOk = m_pConnHandler->readString(fileName);
    osFilePath fileToGet(fileName);
    dmnHandleSampleSourceFilePath(fileToGet);
    fileName = fileToGet.asString();

    if (isOk)
    {
        // Verify that the requested file actually exists.
        isOk = fileToGet.exists();
    }

    // Respond.
    ReportResult(isOk, m_pConnHandler);

    GT_IF_WITH_ASSERT(isOk)
    {
        wstringstream stream;
        stream << L"DMN: Received a request to transfer the following file: " << fileName.asASCIICharArray();
        dmnUtils::LogMessage(stream.str(), OS_DEBUG_LOG_DEBUG);


        isOk = TransferFile(fileName, m_pConnHandler, false);
        GT_ASSERT_EX(isOk, L"DMN: Transferring a file to the client.");

        // Respond.
        ReportResult(isOk, m_pConnHandler);
    }                        return isOk;
}

void dmnSessionThread::TerminateWholeSession(bool& isTerminationRequired, int& ret)
{
    dmnUtils::LogMessage(L"DMN: RECEIVED REMOTE OPCODE -> TERMINATE WHOLE SESSION REQUEST.", OS_DEBUG_LOG_DEBUG);

    // This thread needs to exit at the end of this loop.
    isTerminationRequired = true;

    if (m_rdsProcId != 0)
    {
        // Terminate the Remote debugging session.
        HandleProcessTerminationRequest(m_rdsProcId);
    }

    if (m_sProfProcId != 0)
    {
        // Terminate the Remote profiling session.
        HandleProcessTerminationRequest(m_sProfProcId);
    }


    // Respond.
    ReportResult(true, m_pConnHandler);

    // Notify the user about disconnection.
    notifyUserAboutDisconnection();

    ret = 0;
}

bool dmnSessionThread::LaunchRds()
{
    // Tracing.
    dmnUtils::LogMessage(L"DMN: RECEIVED REMOTE OPCODE -> RDS LAUNCH.", OS_DEBUG_LOG_DEBUG);

    // Extract the command lines.
    gtString cmdLineArgsBuffer;
    ExtractCmdLineString(m_pConnHandler, cmdLineArgsBuffer);

    // Aggregate the environment variables.
    vector<osEnvironmentVariable> envVars;
    gtInt32 envVarsCount;
    bool isOk = ExtractEnvVariables(m_pConnHandler, envVars, envVarsCount);
    GT_ASSERT_EX(isOk, L"DMN: Extracting env vars for debugging session.");

    if (m_rdsProcId == 0)
    {
        // Create the process.
        isOk = CreateProcess(romDEBUG, cmdLineArgsBuffer, envVars);
        GT_ASSERT(isOk);
    }
    else
    {
        // We already have a working session.
        isOk = false;
        dmnUtils::LogMessage(L"Another debugging session is already running.", OS_DEBUG_LOG_ERROR);
    }

    // Respond.
    ReportResult(isOk, m_pConnHandler);                        return isOk;
}
