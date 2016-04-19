//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsMacOSXInterception.h
///
//==================================================================================

//------------------------------ gsMacOSXInterception.h ------------------------------

#ifndef __GSMACOSXINTERCEPTION_H
#define __GSMACOSXINTERCEPTION_H


bool gsInitializeMacOSXOpenGLInterception(apMonitoredFunctionId funcId);
bool gsInitializeMacOSXOpenGLInterception();
bool gsInitializeEAGLInterception();

#endif //__GSMACOSXINTERCEPTION_H

