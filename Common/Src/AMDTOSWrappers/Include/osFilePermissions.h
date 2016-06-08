//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osFilePermissions.h
///
//=====================================================================

//------------------------------ osFilePermissions.h ------------------------------

#ifndef __OSFILEPERMISSIONS_H
#define __OSFILEPERMISSIONS_H

// Local:
#include <AMDTOSWrappers/Include/osFilePath.h>

enum osFilePermissions
{
    OS_READ_PERMISSION      = 0x0001,   // Allow to read the file's content.
    OS_WRITE_PERMISSION     = 0x0002,   // Allow to modify the file's content.
    OS_EXECUTE_PERMISSION   = 0x0004    // Allow to execute the file.
};


OS_API bool osAddAllUsersFilePermissions(const osFilePath& filePath, unsigned int filePermissionsMask);


#endif //__OSFILEPERMISSIONS_H

