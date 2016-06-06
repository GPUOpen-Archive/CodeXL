//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaDisplay.cpp
///
//=====================================================================

//------------------------------ osDisplay.cpp ------------------------------

// Windows user information:
#define WIN32_LEAN_AND_MEAN 1
#define _WIN32_WINNT 0x0501
#include <windows.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaDisplay.h>

// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineAmountOfMonitors
// Description: returns the number of monitors on the current machine
// Return Val: int
// Author:      AMD Developer Tools Team
// Date:        21/5/2008
// ---------------------------------------------------------------------------
int oaGetLocalMachineAmountOfMonitors()
{
    int retVal = GetSystemMetrics(SM_CMONITORS);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetDisplayMonitorInfo
// Description: Get the Primary display device name
// Arguments:   int nDeviceIndex
//              LPSTR lpszMonitorInfo
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        3/10/2004
// ---------------------------------------------------------------------------
bool oaGetDisplayMonitorInfo(gtString& deviceName, gtString& monitorName)
{
    bool retVal = false;

    // Get a handle for the user32.dll:
    HINSTANCE  hInstUser32 = LoadLibrary(L"User32.DLL");

    if (hInstUser32)
    {
        // Define the EnumDisplayDevices function type:
        typedef BOOL (CALLBACK * EnumDisplayDevicesPROC)(LPCTSTR, DWORD, PDISPLAY_DEVICE, DWORD);

        // Get the address of the EnumDisplayDevices function
        EnumDisplayDevicesPROC EnumDisplayDevices = (EnumDisplayDevicesPROC)GetProcAddress(hInstUser32, "EnumDisplayDevicesW");

        if (EnumDisplayDevices != NULL)
        {
            // Iterate over the available display devices:
            DWORD deviceIndex = 0;
            bool goOn = true;

            while (goOn)
            {
                // Initialize the display device info structure:
                DISPLAY_DEVICE displayDeviceInfo;
                ZeroMemory(&displayDeviceInfo, sizeof(displayDeviceInfo));
                displayDeviceInfo.cb = sizeof(displayDeviceInfo);

                if (EnumDisplayDevices(NULL, deviceIndex, &displayDeviceInfo, 0) == NULL)
                {
                    // No more display devices:
                    goOn = false;
                }
                else
                {
                    if (displayDeviceInfo.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
                    {
                        // Get the current device name:
                        wchar_t currentDeviceName[33];
                        wcscpy(currentDeviceName, (wchar_t*)displayDeviceInfo.DeviceName);
                        deviceName = currentDeviceName;

                        // Get the current device monitor name:
                        wchar_t currentMonitorName[129];
                        EnumDisplayDevices(currentDeviceName, 0, &displayDeviceInfo, 0);
                        wcscpy(currentMonitorName, (wchar_t*)displayDeviceInfo.DeviceString);
                        monitorName = currentMonitorName;

                        retVal = true;
                        break;
                    }

                    deviceIndex++;
                }
            }
        }

        // Clean up:
        FreeLibrary(hInstUser32);
    }

    return retVal;
}
