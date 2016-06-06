//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osConsole.h
///
//=====================================================================
#ifndef __osConsole_h
#define __osConsole_h

#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>

//***********************************//
//*Console-related utility functions*//
//***********************************//

// Blocks until the user hits a keyboard key.
void OS_API osWaitForKeyPress();

#endif // osConsole_h__

