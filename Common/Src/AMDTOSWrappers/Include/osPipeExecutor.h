//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osPipeExecutor.h
///
//=====================================================================

//------------------------------ osPipeExecutor.h ------------------------------

#ifndef __OSPIPEEXECUTOR_H
#define __OSPIPEEXECUTOR_H

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>

// Foreward declarations:
class gtString;

// ----------------------------------------------------------------------------------
// Class Name:           osPipeExecutor
// General Description: A class that runs string commands as if inputted in a terminal
//                      / command prompt window and returns their output
// Author:      AMD Developer Tools Team
// Creation Date:        29/10/2008
// ----------------------------------------------------------------------------------
class OS_API osPipeExecutor
{
public:
    osPipeExecutor();
    ~osPipeExecutor();

    bool executeCommand(const gtString& command, gtString& output);
};

#endif //__OSPIPEEXECUTOR_H

