//==============================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DriverControl.h
/// \brief This file contains definitions for loading an AMD driver.
//
//===============================================================================

#ifndef _DRIVERCONTROL_H_
#define _DRIVERCONTROL_H_

#include <InitGuid.h>
#include <WinIoCtl.h>
#include <WinSvc.h>

#ifdef  __cplusplus
extern "C" {
#endif

void OpenAmdDriver(IN  LPCTSTR DriverName, OUT HANDLE*  DriverHandle);

BOOL CloseAmdDriver(IN LPCTSTR DriverName, IN HANDLE DriverHandle);

void OpenWdmDriver(IN  LPGUID DriverGuid, OUT HANDLE*  DriverHandle);

BOOL CloseWdmDriver(IN HANDLE DriverHandle);

BOOL InvokeIn(IN  HANDLE DriverHandle,
              IN  DWORD  ioctlCode,
              IN  LPVOID buffer,
              IN  DWORD  sizeIn,
              OUT DWORD*  sizeInvoked);

BOOL InvokeOut(IN  HANDLE DriverHandle,
               IN  DWORD  ioctlCode,
               OUT LPVOID buffer,
               IN  DWORD  sizeOut,
               OUT DWORD*  sizeInvoked);

BOOL InvokeInOut(IN  HANDLE DriverHandle,
                 IN  DWORD  ioctlCode,
                 IN  LPVOID bufferIn,
                 IN  DWORD  sizeIn,
                 OUT LPVOID bufferOut,
                 IN  DWORD  sizeOut,
                 OUT DWORD*  sizeInvoked);

#ifdef  __cplusplus
} // extern "C"
#endif

#endif _DRIVERCONTROL_H_
