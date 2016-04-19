//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnUtils.cpp
///
//==================================================================================

#include <iostream>
#include <string>
#include <sstream>
#include <exception>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTRemoteAgent/Public Include/dmnStringConstants.h>
#include <AMDTRemoteAgent/dmnUtils.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // For recursive dir creation.
    #include <Shlobj.h>
    #include <windows.h>
#endif


// STATICALLY LINKED UTILS - START
static void LogMessageFromStream(const std::wstringstream& stream, osDebugLogSeverity severity)
{
    dmnUtils::LogMessage(stream.str(), severity);
}
// STATICALLY LINKED UTILS - END

void dmnUtils::HandleException(const std::exception& x)
{
    wstringstream msgStream;
    msgStream << L"An exception caught in CodeXL Daemon. Exception Message = ";
    msgStream << x.what();
    gtString msgString(msgStream.str().c_str());
    OS_OUTPUT_DEBUG_LOG(msgString.asCharArray(), OS_DEBUG_LOG_ERROR);
}

void dmnUtils::LogMessage(const std::wstring& msg, osDebugLogSeverity severity)
{
    gtString msgString(msg.c_str());
    OS_OUTPUT_DEBUG_LOG(msgString.asCharArray(), severity);
}


void dmnUtils::LogMessage(const std::string& msg, osDebugLogSeverity severity)
{
    try
    {
        wstringstream msgStream;
        msgStream << msg.c_str();
        LogMessageFromStream(msgStream, severity);
    }
    catch (...)
    {
        // Nothing much that we can do here.
        GT_ASSERT(false);
    }
}

std::wstring dmnUtils::BufferToString(gtByte* buffer, unsigned int _bufSize)
{
    std::wstringstream _stream;
    _stream.width(2);
    _stream << "Buffer size is " << _bufSize << " Bytes: { ";
    gtByte* pCur = buffer;
    unsigned int curPos = 0;

    while (curPos < _bufSize)
    {
        _stream << hex << " 0x" << (0x000000ff & static_cast<int>(*(pCur)++));
        ++curPos;
    }

    _stream << " }" << endl;
    return std::wstring(_stream.str());
}

std::wstring dmnUtils::OpModeToString(REMOTE_OPERATION_MODE mode)
{
    switch (mode)
    {
        case romDEBUG:
            return L"Remote Debugging";

        case romPROFILE:
            return L"Remote Profiling";

        default:
            return L"Unknown mode";
    }
}

bool dmnUtils::ToGtString(const std::string& str, gtString& buffer)
{
    bool ret = false;

    try
    {
        buffer.makeEmpty();
        wstringstream stream;
        stream << str.c_str();
        buffer = gtString(stream.str().c_str());
        ret = true;
    }
    catch (const std::exception&)
    {
        // Nothing much that we can do over here.
    }
    catch (...) {}

    return ret;
}

bool dmnUtils::CreateDirHierarchy(const gtString& dirPath)
{
    bool ret = false;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    int rc = SHCreateDirectoryEx(NULL, dirPath.asCharArray(), NULL);
    ret = (0 == rc);
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    std::string cmd("mkdir -p ");
    cmd.append(dirPath.asASCIICharArray());
    int rc = system(cmd.c_str());
    ret = (rc != -1);
#else
#error Unknown build configuration!
#endif

    return ret;
}

bool dmnUtils::GetCurrentDirectory(gtString& buffer)
{
    osFilePath fileBuffer;
    bool ret = osGetCurrentApplicationPath(fileBuffer);
    GT_IF_WITH_ASSERT(ret)
    {
        buffer = fileBuffer.fileDirectoryAsString();
    }
    return ret;
}

bool dmnUtils::GetUserDataFolder(osFilePath& buffer)
{
    buffer.setPath(osFilePath::OS_USER_APPLICATION_DATA);
    buffer.appendSubDirectory(DMN_STR_AMD);
    buffer.appendSubDirectory(DMN_STR_CODEXL);
    return true;
}
