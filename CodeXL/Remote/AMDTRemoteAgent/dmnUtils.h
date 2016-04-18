//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnUtils.h
///
//==================================================================================

#ifndef __dmnUtils_h
#define __dmnUtils_h

#include <string>
#include <AMDTRemoteAgent/Public Include/dmnDefinitions.h>
#include <AMDTRemoteAgent/dmnConfigManager.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osApplication.h>

// static utilities class.
class dmnUtils
{
public:

    // Writes a message to the log file.
    static void LogMessage(const std::string& msg, osDebugLogSeverity serverity);

    // Writes a message to the log file.
    static void LogMessage(const std::wstring& msg, osDebugLogSeverity serverity);

    // Handles an exception by tracing its message to the log file.
    static void HandleException(const std::exception& x);

    // Returns the byte array in a printable hexadecimal format (0xff).
    static std::wstring BufferToString(gtByte* buffer, unsigned int _bufSize);

    static std::wstring OpModeToString(REMOTE_OPERATION_MODE mode);

    // Converts str into a gtString and fills buffer with the value.
    // Returns true iff the conversion process did not fail.
    static bool ToGtString(const std::string& str, gtString& buffer);

    // Creates a directory with the full hierarchy if required.
    // Returns true iff the result is true.
    static bool CreateDirHierarchy(const gtString& dirPath);

    // Gets the directory of the current executable.
    // (To be moved to OSWrappers).
    static bool GetCurrentDirectory(gtString& buffer);

    // Gets the current user's data folder on the local machine.
    static bool GetUserDataFolder(osFilePath& buffer);

};

#endif // __dmnUtils_h