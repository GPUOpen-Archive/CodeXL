//==============================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DriverControl.c
/// \brief Win32 routines to dynamically load and unload an AMD driver
///        or an already loaded WDM driver on Windows 9x, NT, 2000 or XP.
//
//===============================================================================

// Disable warning:
#pragma warning( disable : 4996 )


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setupapi.h>
#include <objbase.h>
#ifdef DEBUG
    #include <dprintf.h>
#endif
#include <TCHAR.H>
#include <shlwapi.h>
#include "../inc/DriverControl.h"

//
// Display Debug messages
//

static BOOLEAN SetupDriverName(IN LPCTSTR DriverName, LPTSTR DriverLocation);
static BOOLEAN InstallDriver(IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName, IN LPCTSTR ServiceExe);
static BOOLEAN ManageDriver(IN LPCTSTR DriverName, IN LPCTSTR ServiceName, IN USHORT Function);
static BOOLEAN RemoveDriver(IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName);
static BOOLEAN StartDriver(IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName);
static BOOLEAN StopDriver(IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName);

//
// Following are used to install and remove the driver dynamically.
//
#define DRIVER_FUNC_INSTALL     0x01
#define DRIVER_FUNC_REMOVE      0x02



////////////////////////////////////////////////////////////////////////
// OpenAmdDriver
//
//      Open a handle to the requested device
//
//      parameters:     DriverName, a constant string containing the
//                      driver name without any file extension
//
//                      DriverHandle, a pointer to the handle to the driver
//
//
//      returns:        nothing

void OpenAmdDriver(IN LPCTSTR DriverPath, OUT HANDLE* DriverHandle)
{
    HANDLE fileHandle;
    TCHAR driverLocation[MAX_PATH];
    TCHAR drvName[MAX_PATH];
#ifdef UNICODE
    TCHAR slash = L'\\';
#else
    TCHAR slash = '\\';
#endif
    TCHAR* pDriverName;
    DWORD dwStrLenName, dwStrLenPath;
    DWORD errNum = 0;
    OSVERSIONINFO osInfo;

    osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osInfo);

    StrCpy(drvName, TEXT("\\\\.\\"));

    // User has to send in a full path name (e.g. c:\amd\caprof)
    // or just the name of the driver (e.g. caprof - compatibility).
    pDriverName = StrRChr(DriverPath, NULL, slash);


    // if NULL returned then user only sent down driverName, no path included - compatibility
    if (pDriverName != NULL)
    {
        wchar_t* next_token = NULL;

        // getting lengths to determine size of absolute path
#ifdef UNICODE
        dwStrLenName = (DWORD)wcslen(pDriverName);
        dwStrLenPath = (DWORD)wcslen(DriverPath);
#else
        dwStrLenName = (DWORD)strlen(pDriverName);
        dwStrLenPath = (DWORD)strlen(DriverPath);
#endif

        //StrCpyN appends null charactor automatically
        StrCpyN(driverLocation, DriverPath, (dwStrLenPath - dwStrLenName + 1));

        //driverLocation[dwStrLenPath - dwStrLenName] = L'\0';  // terminate it
#ifdef UNICODE
        pDriverName = wcstok_s(pDriverName, &slash, &next_token);  //deletes the slash from front
#else
        pDriverName = strtok(pDriverName, &slash);
#endif
#ifdef DEBUG
        wprintf(L"Stripped driver name = %s and DriverPath = %s\n", pDriverName, driverLocation);
#endif
        StrCat(drvName, pDriverName);
    }
    else
    {
        StrCpy(driverLocation, TEXT("\0")); // just putting in a null char to check later
        pDriverName = (TCHAR*)DriverPath;
#ifdef DEBUG
        wprintf(L"Stripped driver name = %s\n", pDriverName);
#endif
        StrCat(drvName, DriverPath);
    }

    //
    // If Windows 9x platform, load VxD driver dynamically.
    //
    if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
    {
        StrCat(drvName, TEXT(".vxd"));

        if ((fileHandle = CreateFile(drvName,
                                     GENERIC_READ | GENERIC_WRITE,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_FLAG_DELETE_ON_CLOSE,
                                     NULL)) == INVALID_HANDLE_VALUE)
        {
            errNum = GetLastError();

#ifdef DEBUG
            dprintf("CreateFile failed!  Handle = %x, Error = %d\n", fileHandle, errNum);
#endif

            *DriverHandle = INVALID_HANDLE_VALUE;
            return;
        }

        *DriverHandle = fileHandle;
        return;
    }


    //
    // Otherwise, it must be NT system, so load SYS driver.
    // Try to connect to driver.  If this fails, try to load the driver
    // dynamically.
    //

    //
    // Driverworkds convention
    //
    StrCat(drvName, TEXT("0"));

    //
    // Is it already there?
    //
    if ((fileHandle = CreateFile(drvName,
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL
                                )) == INVALID_HANDLE_VALUE)
    {
        errNum = GetLastError();

        if (errNum != ERROR_FILE_NOT_FOUND)
        {

#ifdef DEBUG
            dprintf("CreateFile failed!  Error = %d\n", errNum);
#endif

            *DriverHandle = INVALID_HANDLE_VALUE;
            return;
        }

        //
        // Setup full path to driver name.
        //
        if (!SetupDriverName(pDriverName, driverLocation))
        {
            *DriverHandle = INVALID_HANDLE_VALUE;
            return;
        }

        //
        // Install driver.
        //
        if (!ManageDriver(pDriverName,
                          driverLocation,
                          DRIVER_FUNC_INSTALL))
        {

#ifdef DEBUG
            dprintf("Unable to install driver. \n");
#endif

            //
            // Error - remove driver.
            //
            ManageDriver(pDriverName,
                         driverLocation,
                         DRIVER_FUNC_REMOVE);
            *DriverHandle = INVALID_HANDLE_VALUE;
            return;
        }

        //
        // Try to open the newly installed driver.
        //
        if ((fileHandle = CreateFile(drvName,
                                     GENERIC_READ | GENERIC_WRITE,
                                     FILE_SHARE_READ,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL
                                    )) == INVALID_HANDLE_VALUE)
        {

#ifdef DEBUG
            dprintf("CreateFile failed!  Error = %d\n", GetLastError());
#endif
            //
            // Error - remove driver.
            //
            ManageDriver(pDriverName,
                         driverLocation,
                         DRIVER_FUNC_REMOVE);
            *DriverHandle = INVALID_HANDLE_VALUE;
            return;
        }
    }

    *DriverHandle = fileHandle;
    return;
}

////////////////////////////////////////////////////////////////////////
// CloseAmdDriver
//
//      Close the device

BOOL CloseAmdDriver(IN LPCTSTR DriverName, IN HANDLE DriverHandle)
{
    BOOL result;
    OSVERSIONINFO osInfo;

    osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osInfo);

    if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
    {
        result = CloseHandle(DriverHandle);
    }
    else
    {
        result = CloseHandle(DriverHandle);

        if (!result)
        {
            return result;
        }

        result = ManageDriver(DriverName,
                              (LPCTSTR)1, // Service name is only used for installing a driver
                              DRIVER_FUNC_REMOVE);

    }

    return result;
}



////////////////////////////////////////////////////////////////////////
// OpenWdmDriver
//
//      Open a handle to the requested WDM device
//
//      parameters:     DriverGuid, a pointer to a GUID which identifies
//                          the WDM device
//
//                      DriverHandle, a pointer to the handle to the driver
//
//
//      returns:        nothing

void OpenWdmDriver(IN LPGUID DriverGuid, OUT HANDLE* DriverHandle)
{
    BOOL initSuccess = FALSE;
    HDEVINFO                            hInfo;
    SP_DEVICE_INTERFACE_DATA            deviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    pDeviceInterfaceDetailData = NULL;
    ULONG                               predictedLength = 0;
    ULONG                               requiredLength = 0;
    HANDLE                              theHandle;

    //++
    //      SetupDiGetClassDevs() returns a handle to a device information set
    //      containing all installed devices matching the specified parameters.
    //      You need hInfo for SetupDiEnumDeviceInterfaces(), SetupDiGetDeviceInterfaceDetail()
    //      SetupDiGetDeviceInterfaceDetail(), and SetupDiDestroyDeviceInfoList().
    //      hInfo is destroyed in SetupDiDestroyDeviceInfoList().
    //--

    hInfo = SetupDiGetClassDevs((LPGUID)DriverGuid, NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));

    if (hInfo == INVALID_HANDLE_VALUE)
    {
        *DriverHandle = INVALID_HANDLE_VALUE;
        return;
    }
    else
    {
        deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        initSuccess = SetupDiEnumDeviceInterfaces(hInfo,
                                                  0,
                                                  (LPGUID)DriverGuid,
                                                  0,
                                                  &deviceInterfaceData);

        if (initSuccess)
        {

            // First find out the required length of the buffer

            SetupDiGetDeviceInterfaceDetail(hInfo, &deviceInterfaceData, NULL, 0, &requiredLength, NULL);

            predictedLength = requiredLength;

            pDeviceInterfaceDetailData = malloc(predictedLength);

            if (pDeviceInterfaceDetailData)
            {
                pDeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
            }
            else
            {
#ifdef DEBUG
                dprintf("Couldn't allocate %d bytes for device interface details.\n", predictedLength);
#endif
                SetupDiDestroyDeviceInfoList(hInfo);
                *DriverHandle = INVALID_HANDLE_VALUE;
                return;
            }

            initSuccess = SetupDiGetDeviceInterfaceDetail(hInfo,
                                                          &deviceInterfaceData,
                                                          pDeviceInterfaceDetailData,
                                                          predictedLength,
                                                          &requiredLength,
                                                          NULL);

            if (!initSuccess)
            {
                SetupDiDestroyDeviceInfoList(hInfo);
                free(pDeviceInterfaceDetailData);
                *DriverHandle = INVALID_HANDLE_VALUE;
                return;
            }
        }

        SetupDiDestroyDeviceInfoList(hInfo);

        if (!pDeviceInterfaceDetailData)
        {
            free(pDeviceInterfaceDetailData);
            *DriverHandle = INVALID_HANDLE_VALUE;
            return;
        }

        // Otherwise you can open a device interface

        theHandle = CreateFile(pDeviceInterfaceDetailData->DevicePath,
                               (GENERIC_READ | GENERIC_WRITE),
                               (FILE_SHARE_READ | FILE_SHARE_WRITE),
                               NULL,
                               OPEN_EXISTING,
                               0,
                               NULL);

        free(pDeviceInterfaceDetailData);

        if (theHandle == INVALID_HANDLE_VALUE)
        {
#ifdef DEBUG
            dprintf("CreateFile failed!  Error = %d\n", GetLastError());
#endif

            *DriverHandle = INVALID_HANDLE_VALUE;
        }
        else
        {
            *DriverHandle = theHandle;
        }
    }

    return;

}



////////////////////////////////////////////////////////////////////////
// CloseWdmDriver
//
//      Close the device

BOOL CloseWdmDriver(IN HANDLE DriverHandle)
{
    BOOL result;

    result = CloseHandle(DriverHandle);

    return result;
}



////////////////////////////////////////////////////////////////////////
// InvokeIn
//
//      Calls the driver function DeviceIoControl with the given data
//
//          parameters:     DriverHandle, a pointer to the driver handle
//
//                          ioctlCode, a code which determines which
//                          procedure is desired
//
//                          buffer, a pointer to the data to be sent to
//                          the driver
//
//                          sizeIn, the size of the buffer pointed to
//
//
//          returns:        TRUE (1) if call to driver successful
//                          FALSE (0) if call to driver unsuccessful
//
BOOL InvokeIn(IN  HANDLE DriverHandle,
              IN  DWORD  ioctlCode,
              IN  LPVOID buffer,
              IN  DWORD  sizeIn,
              OUT DWORD*  sizeInvoked)
{
    BOOL result;
    unsigned long bytesReturned;

    result = DeviceIoControl(
                 DriverHandle,   // Private, handle to device
                 ioctlCode,      // Operation
                 buffer,         // pointer to information sent
                 sizeIn,         // size of information
                 NULL,
                 0,
                 &bytesReturned, // bytes sent back
                 NULL            // wait until operation completes
             );

    // make sizeInvoked point to bytesReturned if the user
    // passed in a valid address
    if (sizeInvoked != NULL)
    {
        *sizeInvoked = bytesReturned;
    }

    return result;
}



////////////////////////////////////////////////////////////////////////
// InvokeOut
//
//      Calls the driver function DeviceIoControl with the given data
//
//          parameters:     DriverHandle, a pointer to the driver handle
//
//                          ioctlCode, a code which determines which
//                          procedure is desired
//
//                          buffer, a pointer to the data to be retrieved
//                          from the driver
//
//                          sizeOut, the size of the buffer pointed to
//
//
//          returns:        TRUE (1) if call to driver successful
//                          FALSE (0) if call to driver unsuccessful
//
BOOL InvokeOut(IN  HANDLE DriverHandle,
               IN  DWORD  ioctlCode,
               OUT LPVOID buffer,
               IN  DWORD  sizeOut,
               OUT DWORD*  sizeInvoked)
{
    BOOL result;
    unsigned long bytesReturned;

    result = DeviceIoControl(
                 DriverHandle,   // Private, handle to device
                 ioctlCode,      // Operation
                 NULL,
                 0,
                 buffer,         // pointer to information received
                 sizeOut,        // size of information
                 &bytesReturned, // bytes sent back
                 NULL            // wait until operation completes
             );

    // make sizeInvoked point to bytesReturned if the user
    // passed in a valid address
    if (sizeInvoked != NULL)
    {
        *sizeInvoked = bytesReturned;
    }

    return result;
}



////////////////////////////////////////////////////////////////////////
// InvokeInOut
//
//      Calls the driver function DeviceIoControl with the given data
//
//          parameters:     DriverHandle, a pointer to the driver handle
//
//                          ioctlCode, a code which determines which
//                          procedure is desired
//
//                          bufferIn, a pointer to the data to be sent to
//                          the driver
//
//                          sizeIn, the size of the buffer pointed to by
//                          bufferIn
//
//                          bufferOut, a pointer to the data to be retrieved
//                          from the driver
//
//                          sizeOut, hte size of the buffer pointed to by
//                          bufferOut
//
//
//          returns:        TRUE (1) if call to driver successful
//                          FALSE (0) if call to driver unsuccessful
//
BOOL InvokeInOut(IN  HANDLE DriverHandle,
                 IN  DWORD  ioctlCode,
                 IN  LPVOID bufferIn,
                 IN  DWORD  sizeIn,
                 OUT LPVOID bufferOut,
                 IN  DWORD  sizeOut,
                 OUT DWORD*  sizeInvoked)
{
    BOOL result;
    unsigned long bytesReturned;

    result = DeviceIoControl(
                 DriverHandle,   // Private, handle to device
                 ioctlCode,      // Operation
                 bufferIn,       // pointer to information sent
                 sizeIn,         // size of information sent
                 bufferOut,      // pointer to information received
                 sizeOut,        // size of information received
                 &bytesReturned, // bytes sent back
                 NULL            // wait until operation completes
             );

    // make sizeInvoked point to bytesReturned if the user
    // passed in a valid address
    if (sizeInvoked != NULL)
    {
        *sizeInvoked = bytesReturned;
    }

    return result;
}



////////////////////////////////////////////////////////////////////////
// SetupDriverName [internal]
//
//      Taken from the Microsoft SDK.
//
//      Attempts to associate the Windows NT driver name with its
//      location and then tests to see that it was successful.
//
//      returns TRUE (1) if successful, FALSE (0) if unsuccessful
//

BOOLEAN
SetupDriverName(IN LPCTSTR DriverName,
                LPTSTR DriverLocation)
{
    HANDLE fileHandle;
    DWORD driverLocLen = 0;

    //
    // Get the current directory.
    //
    if (DriverLocation == '\0')
    {
        driverLocLen = GetCurrentDirectory(MAX_PATH, DriverLocation);

        if (!driverLocLen)
        {

#ifdef DEBUG
            dprintf("GetCurrentDirectory failed!  Error = %d \n", GetLastError());
#endif

            return FALSE;
        }
    }

    //
    // Setup path name to driver file.
    //
    StrCat(DriverLocation, TEXT("\\"));
    StrCat(DriverLocation, DriverName);
    StrCat(DriverLocation, TEXT(".sys"));
#ifdef DEBUG
    wprintf(L"SetupDriverName: full path to driver = %s\n", DriverLocation);
#endif

    //
    // Insure driver file is in the specified directory.
    //
    if ((fileHandle = CreateFile(DriverLocation,
                                 GENERIC_READ,
                                 0,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL
                                )) == INVALID_HANDLE_VALUE)
    {

#ifdef DEBUG
#ifdef UNICODE
        wsprintf(TEXT("Driver: %ws is not in the current directory.\n"), DriverLocation);
#else
        dprintf(TEXT("Driver: %s is not in the current directory.\n"), DriverLocation);
#endif
#endif

        //
        // Indicate failure.
        //
        return FALSE;
    }

    //
    // Close open file handle.
    //
    if (fileHandle)
    {
        CloseHandle(fileHandle);
    }

    //
    // Indicate success.
    //
    return TRUE;

}   // SetupDriverName



////////////////////////////////////////////////////////////////////////
// InstallDriver [internal]
//
//      Taken from the Microsoft SDK.
//
//      Attempts to create a service (driver) object.
//
//      returns TRUE (1) if successful, FALSE (0) if unsuccessful
//

BOOLEAN
InstallDriver(
    IN SC_HANDLE  SchSCManager,
    IN LPCTSTR    DriverName,
    IN LPCTSTR    ServiceExe)
{
    BOOLEAN     rCode = TRUE;
    SC_HANDLE   schService;
    DWORD       err;

    //
    // NOTE: This creates an entry for a standalone driver. If this
    //       is modified for use with a driver that requires a Tag,
    //       Group, and/or Dependencies, it may be necessary to
    //       query the registry for existing driver information
    //       (in order to determine a unique Tag, etc.).
    //

    //
    // Create a new a service object.
    //
    schService = CreateService(SchSCManager,           // handle of service control manager database
                               DriverName,             // address of name of service to start
                               DriverName,             // address of display name
                               SERVICE_ALL_ACCESS,     // type of access to service
                               SERVICE_KERNEL_DRIVER,  // type of service
                               SERVICE_DEMAND_START,   // when to start service
                               SERVICE_ERROR_NORMAL,   // severity if service fails to start
                               ServiceExe,             // address of name of binary file
                               NULL,                   // service does not belong to a group
                               NULL,                   // no tag requested
                               NULL,                   // no dependency names
                               NULL,                   // use LocalSystem account
                               NULL                    // no password for service account
                              );

    if (schService == NULL)
    {
        err = GetLastError();

        if (err == ERROR_SERVICE_EXISTS)
        {
            //
            // Ignore this error.
            //
            rCode = TRUE;
        }
        else
        {

#ifdef DEBUG
            dprintf("CreateService failed!  Error = %d \n", err);
#endif

            //
            // Indicate an error.
            //
            rCode = FALSE;
        }
    }

    //
    // Close the service object.
    //
    if (schService)
    {
        CloseServiceHandle(schService);
    }

    //
    // Indicate success.
    //
    return rCode;

}   // InstallDriver



////////////////////////////////////////////////////////////////////////
// ManageDriver [internal]
//
//      Taken from the Microsoft SDK.
//
//      Attempts to perform a particular function on the
//      service (driver).
//
//      returns TRUE (1) if successful, FALSE (0) if unsuccessful
//

BOOLEAN
ManageDriver(
    IN LPCTSTR  DriverName,
    IN LPCTSTR  ServiceName,
    IN USHORT   Function)
{

    SC_HANDLE   schSCManager;
    BOOLEAN     rCode = TRUE;

    //
    // Insure (somewhat) that the driver and service names are valid.
    //
    if (!DriverName || !ServiceName)
    {

#ifdef DEBUG
        dprintf("Invalid Driver or Service provided to ManageDriver() \n");
#endif

        return FALSE;
    }

    //
    // Connect to the Service Control Manager and open the Services database.
    //
    schSCManager = OpenSCManager(NULL,                   // local machine
                                 NULL,                   // local database
                                 SC_MANAGER_ALL_ACCESS   // access required
                                );

    if (!schSCManager)
    {

#ifdef DEBUG
        dprintf("Open SC Manager failed! Error = %d \n", GetLastError());
#endif

        return FALSE;
    }

    //
    // Do the requested function.
    //
    switch (Function)
    {
        case DRIVER_FUNC_INSTALL:

            //
            // Install the driver service.
            //
            if (InstallDriver(schSCManager,
                              DriverName,
                              ServiceName))
            {
                //
                // Start the driver service (i.e. start the driver).
                //
                rCode = StartDriver(schSCManager,
                                    DriverName);
            }
            else
            {
                //
                // Indicate an error.
                //
                rCode = FALSE;
            }

            break;

        case DRIVER_FUNC_REMOVE:
            //
            // Stop the driver.
            //
            rCode = StopDriver(schSCManager,
                               DriverName);

#ifdef DEBUG

            if (!rCode)
            {
                wprintf(L"StopDriver Unsuccessful\n");
            }

#endif
            //
            // Remove the driver service.
            //
            rCode = RemoveDriver(schSCManager,
                                 DriverName);
#ifdef DEBUG

            if (!rCode)
            {
                dprintf("RemoveDriver Unsuccessful\n");
            }

#endif
            //
            // Ignore all errors.
            //
            rCode = TRUE;

            break;

        default:

#ifdef DEBUG
            dprintf("Unknown ManageDriver() function. \n");
#endif

            rCode = FALSE;

            break;
    }

    //
    // Close handle to service control manager.
    //
    if (schSCManager)
    {
        CloseServiceHandle(schSCManager);
    }

    return rCode;

}   // ManageDriver



////////////////////////////////////////////////////////////////////////
// RemoveDriver [internal]
//
//      Taken from the Microsoft SDK.
//
//      Attempts to close the service (driver).
//
//      returns TRUE (1) if successful, FALSE (0) if unsuccessful
//

BOOLEAN
RemoveDriver(
    IN SC_HANDLE    SchSCManager,
    IN LPCTSTR      DriverName)
{
    SC_HANDLE   schService;
    BOOLEAN     rCode;

    //
    // Open the handle to the existing service.
    //
    schService = OpenService(SchSCManager,
                             DriverName,
                             SERVICE_ALL_ACCESS);

    if (schService == NULL)
    {

#ifdef DEBUG
        dprintf("OpenService failed!  Error = %d \n", GetLastError());
#endif

        //
        // Indicate error.
        //
        return FALSE;
    }

    //
    // Mark the service for deletion from the service control manager database.
    //
    if (DeleteService(schService))
    {
        //
        // Indicate success.
        //
        rCode = TRUE;
    }
    else
    {

#ifdef DEBUG
        dprintf("DeleteService failed!  Error = %d \n", GetLastError());
#endif

        //
        // Indicate failure.  Fall through to properly close the service handle.
        //
        rCode = FALSE;
    }

    //
    // Close the service object.
    //
    if (schService)
    {
        CloseServiceHandle(schService);
    }

    return rCode;

}   // RemoveDriver



////////////////////////////////////////////////////////////////////////
// StartDriver [internal]
//
//      Taken from the Microsoft SDK.
//
//      Attempts to start the service (driver).
//
//      returns TRUE (1) if successful, FALSE (0) if unsuccessful
//

BOOLEAN
StartDriver(IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName)
{
    BOOLEAN     rCode = TRUE;
    SC_HANDLE   schService;
    DWORD       err;

    //
    // Open the handle to the existing service.
    //
    schService = OpenService(SchSCManager,
                             DriverName,
                             SERVICE_ALL_ACCESS);

    if (schService == NULL)
    {

#ifdef DEBUG
        dprintf("OpenService failed!  Error = %d \n", GetLastError());
#endif

        //
        // Indicate failure.
        //
        return FALSE;
    }

    //
    // Start the execution of the service (i.e. start the driver).
    //
    if (!StartService(schService,     // service identifier
                      0,              // number of arguments
                      NULL            // pointer to arguments
                     ))
    {
        err = GetLastError();

        if (err == ERROR_SERVICE_ALREADY_RUNNING)
        {
            //
            // Ignore this error.
            //
            rCode = TRUE;
        }
        else
        {

#ifdef DEBUG
            dprintf("StartService failure! Error = %d \n", err);
#endif

            //
            // Indicate failure.  Fall through to properly close the service handle.
            //
            rCode = FALSE;
        }

    }

    //
    // Close the service object.
    //
    if (schService)
    {
        CloseServiceHandle(schService);
    }

    return rCode;

}   // StartDriver



////////////////////////////////////////////////////////////////////////
// StopDriver [internal]
//
//      Taken from the Microsoft SDK.
//
//      Attempts to stop the service (driver).
//
//      returns TRUE (1) if successful, FALSE (0) if unsuccessful
//

BOOLEAN
StopDriver(IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName)
{
    BOOLEAN         rCode = TRUE;
    SC_HANDLE       schService;
    SERVICE_STATUS  serviceStatus;

    //
    // Open the handle to the existing service.
    //
    schService = OpenService(SchSCManager,
                             DriverName,
                             SERVICE_ALL_ACCESS);

    if (schService == NULL)
    {

#ifdef DEBUG
        dprintf("OpenService failed!  Error = %d \n", GetLastError());
#endif

        return FALSE;
    }

    //
    // Request that the service stop.
    //
    if (ControlService(schService,
                       SERVICE_CONTROL_STOP,
                       &serviceStatus))
    {
        //
        // Indicate success.
        //
        rCode = TRUE;
    }
    else
    {

#ifdef DEBUG
        dprintf("ControlService failed!  Error = %d \n", GetLastError());
#endif

        //
        // Indicate failure.  Fall through to properly close the service handle.
        //
        rCode = FALSE;
    }

    //
    // Close the service object.
    //
    if (schService)
    {
        CloseServiceHandle(schService);
    }

    return rCode;

}   //  StopDriver
