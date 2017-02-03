//==================================================================================
// Copyright (c) 2017 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTPowerProfilePlugin.h
///
//==================================================================================

#ifndef _AMDTPOWERPROFILEPLUGIN_H_
#define _AMDTPOWERPROFILEPLUGIN_H_

/// Nested calls to amdtBeginMarker will result in markers showing in a hierarchical way.
/// \param szMarkerName Marker name
/// \param szGroupName Group name, Optional, Pass in NULL to use default group name
///        If group name is specified, additional sub-branch will be created under PerfMarker branch
///        in Timeline and all markers that belong to the group will be displayed in the group branch.
/// \param szUserString Currently ignored -- reserved for future use (see below for one expected future use)
///        User string, Optional, Pass in NULL to use default color and no user specific string.
///        If User string is specified it should be formatted as a XML string. Optional tag is Color, as in the following example
///        amdtBeginMarker("MyMarker", "MyGroup", "<Color Hex="FF0000"/>\n<UserComment Comment="Starting major calulcation"/>")
/// \return status code
extern int amdtBeginMarker(const char* szMarkerName, const char* szGroupName, const char* szUserString);

/// End AMDTActivityLogger block
/// \return status code
extern int amdtEndMarker();

#endif //_AMDTPOWERPROFILEPLUGIN_H_
