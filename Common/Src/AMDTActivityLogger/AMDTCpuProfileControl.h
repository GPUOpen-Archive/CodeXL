//=============================================================================
//
// Author: AMD Developer Tools
//         AMD, Inc.
//
//=============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc.  All rights reserved.
//=============================================================================

#ifndef _AMDTCPUPROFILECONTROL_H_
#define _AMDTCPUPROFILECONTROL_H_

// These are internal helper functions to control Cpu Profiling session

// Resume the profile data collection during AMD CodeXL CPU profile session.
int AMDTCpuProfileResume(void);

// Pause the profile data collection during AMD CodeXL CPU profile session.
int AMDTCpuProfilePause(void);

// Close the controller interface
bool AMDTCpuProfileControlClose(void);


#endif  // _AMDTCPUPROFILECONTROL_H_