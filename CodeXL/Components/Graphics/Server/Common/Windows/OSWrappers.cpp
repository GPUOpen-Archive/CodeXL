//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Windows-Specific wrapper functions. Use a generic platform-agnostic function
///         to perform OS-Specific functionality
//==============================================================================

#include <Windows.h>
#include <Evntprov.h>
#include <Rpc.h>
#include <sstream>

#include "../OSWrappers.h"
#include "../Logger.h"
#include "../defines.h"

namespace OSWrappers
{

//--------------------------------------------------------------------------
/// Get whether an executable file is 32 or 64 bit. Used to verify if an
/// app is compatible with the 32 or 64 bit build of PerfStudio.
///
/// \param moduleName name of executable to test
/// \arch whether this executable is 32 or 64 bit, indirectly returned.
/// \return true if successful, in which case arch will contain a valid
///  value or false if error
//--------------------------------------------------------------------------
bool GetBinaryType(const char* moduleName, osModuleArchitecture* arch)
{
    bool retVal = false;
    DWORD executableBinaryType = SCS_32BIT_BINARY;
    BOOL retCode = ::GetBinaryType(moduleName, &executableBinaryType);

    if (retCode != FALSE)
    {
        switch (executableBinaryType)
        {
            case SCS_32BIT_BINARY:
            {
                // This is a 32-bit binary.
                retVal = true;
                *arch = OS_I386_ARCHITECTURE;
            }
            break;

            case SCS_64BIT_BINARY:
            {
                // This is a 64-bit binary.
                retVal = true;
                *arch = OS_X86_64_ARCHITECTURE;
            }
            break;

            case SCS_DOS_BINARY:
            case SCS_WOW_BINARY:
            case SCS_PIF_BINARY:
            case SCS_POSIX_BINARY:
            case SCS_OS216_BINARY:
            {
                // Unsupported binary type.
                retVal = true;
                *arch = OS_UNSUPPORTED_ARCHITECTURE;
            }
            break;

            default:
            {
                // Unknown binary type!
                *arch = OS_UNKNOWN_ARCHITECTURE;
            }
        }
    }

    return retVal;
}

//--------------------------------------------------------------------------
/// Get the current thread handle
/// \return Handle to the current thread
//--------------------------------------------------------------------------
osThreadHandle GetCurrentThreadHandle()
{
    return ::GetCurrentThread();
}

//--------------------------------------------------------------------------
/// Retrieves the current value of the high-resolution performance counter
///
/// \param lpPerformanceCount Pointer to a variable that receives the
///  current performance-counter value, in counts
/// \return true if successful, false if error.
//--------------------------------------------------------------------------
bool  QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
{
    if (::QueryPerformanceCounter(lpPerformanceCount) == TRUE)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------
/// Displays a modal dialog box that contains a system icon, a set of
/// buttons, and a brief application-specific message, such as status or
/// error information.
///
/// \param text The message to be displayed. If the string consists of more
///  than one line, the lines can be separated using a carriage return
///  and/or linefeed character between each line.
/// \param caption The dialog box title. If this parameter is NULL, the
///  default title is Error.
/// \param flags The contents and behavior of the dialog box.
/// \return An integer value that indicates which button the user clicked
//--------------------------------------------------------------------------
int   MessageBox(const char* text, const char* caption, unsigned int flags)
{
    return ::MessageBox(NULL, text, caption, flags);
}

//--------------------------------------------------------------------------
/// Emit a GPUView event
//--------------------------------------------------------------------------
void EmitSystemEvent()
{
#if ENABLE_OS_EVENT_EMISSION
    static REGHANDLE s_gpuViewEventHandle = 0;

    if (s_gpuViewEventHandle == 0)
    {
        GUID guid = { 0xa9744ea3 , 0xe5ac , 0x4f2f , { 0xbe, 0x6a, 0x42, 0xaa, 0xd0, 0x8a, 0x9c, 0x6f } };
        EventRegister(&guid, nullptr, nullptr, &s_gpuViewEventHandle);
    }

    EventWriteString(s_gpuViewEventHandle, 0, 0, L"GPS");
#else
    LogConsole(logMESSAGE, "Set ENABLE_OS_EVENT_EMISSION=1 if trying to send GPUView events, and don't forget to use logGPS.cmd.\n");
#endif
}

//--------------------------------------------------------------------------
/// Write basic system and device info into the incoming string.
/// \param ioInfoString The string to be filled with formatted system information.
//--------------------------------------------------------------------------
void WriteSystemInfoString(std::string& outSystemInfo)
{
    std::stringstream infoBuilder;

    // Insert the system info header line
    infoBuilder << "//==System Info==" << std::endl;

    /*
    // @TODO: Must implement this to print basic information related to Devices that can render/compute.
    // Here's an example of what the return value of this function might look like:
    //
    //Device AMD FX(tm)-8150 Eight-Core Processor            Platform Vendor = Advanced Micro Devices, Inc.
    //Device AMD FX(tm)-8150 Eight-Core Processor            Platform Name = AMD Accelerated Parallel Processing
    //Device AMD FX(tm)-8150 Eight-Core Processor            Platform Version = OpenCL 2.0 AMD-APP (1756.4)
    //Device AMD FX(tm)-8150 Eight-Core Processor            CLDriver Version = 1756.4 (sse2,avx,fma4)
    //Device AMD FX(tm)-8150 Eight-Core Processor            CLRuntime Version = OpenCL 1.2 AMD-APP (1756.4)
    //Device AMD FX(tm)-8150 Eight-Core Processor            NumberAppAddressBits = 32
    //Device Barts Platform Vendor = Advanced Micro Devices, Inc.
    //Device Barts Platform Name = AMD Accelerated Parallel Processing
    //Device Barts Platform Version = OpenCL 2.0 AMD-APP (1756.4)
    //Device Barts CLDriver Version = 1756.4 (VM)
    //Device Barts CLRuntime Version = OpenCL 1.2 AMD-APP (1756.4)
    //Device Barts NumberAppAddressBits = 32
    */

    // Return the system info string.
    outSystemInfo.assign(infoBuilder.str());
}

//--------------------------------------------------------------------------
/// Query if a process is running in the system
/// \param szProcessName the name of the process to query
/// \param useSubString indicates whether szProcessName is the full process
/// or a substring. If it's a substring, a substring comparison will be used
/// on the process name. Useful when certain processes contain platform and
/// build information on the end (ie GPUPerfStudio-x64-d.exe)
/// \return true if the process specified is running, false if not
//--------------------------------------------------------------------------
bool IsProcessRunning(const char* szProcessName, bool useSubString)
{
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;

    // get a snapshot of the current processes
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        Log(logWARNING, "CreateToolhelp32Snapshot ( of processes )\n");
        return false;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    // get the first process
    if (Process32First(hProcessSnap, &pe32) == false)
    {
        Log(logWARNING, "Process32First\n");
        CloseHandle(hProcessSnap);
        return false;
    }

    // check to see if the library is in each process
    bool found = false;

    do
    {
        if (useSubString)
        {
            found = (NULL != strstr(pe32.szExeFile, szProcessName));
        }
        else
        {
            found = (0 == strcmp(szProcessName, pe32.szExeFile));
        }
    }
    while (Process32Next(hProcessSnap, &pe32) && found == false);

    CloseHandle(hProcessSnap);
    return found;
}

} // namespace OSWrappers
