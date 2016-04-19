//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csOpenCLServerInitialization.h
///
//==================================================================================

//------------------------------ csOpenCLServerInitialization.h ------------------------------

#ifndef __CSOPENCLSERVERINITIALIZATION_H
#define __CSOPENCLSERVERINITIALIZATION_H


bool csInitializeOpenCLServer();
bool csTerminateOpenCLServer();
bool csPerformOpenCLMonitorTerminationActions();
void csDisplayInitializationSuccededMessage();
void csOnInitializationError();

#endif //__CSOPENCLSERVERINITIALIZATION_H

