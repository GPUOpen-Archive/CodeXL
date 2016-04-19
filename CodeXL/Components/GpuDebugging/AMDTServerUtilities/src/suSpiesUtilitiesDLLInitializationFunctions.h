//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suSpiesUtilitiesDLLInitializationFunctions.h
///
//==================================================================================

//------------------------------ suSpiesUtilitiesDLLInitializationFunctions.h ------------------------------

#ifndef __SUSPIESUTILITIESDLLINITIALIZATIONFUNCTIONS_H
#define __SUSPIESUTILITIESDLLINITIALIZATIONFUNCTIONS_H

bool suInitializeSpiesUtilitiesModule();
bool suTerminateSpiesUtilitiesModule();
void suReportDebuggedProcessTermination();
bool suInitializeDebugLogFile();
void suTerminateDebugLogFile();
bool suInitializeAPIOrStandaloneMode();
bool suInitializeSpyToAPIConnection();
bool suInitializeAPIInStandaloneMode();
bool suWasLaunchedUsingCodeXL();
bool suIsAttachedToDebuggedApplication();
bool suRedirectSpyStdOutIfNeeded();
void suInitializeGlobalVariables();
void suDisplayCommunicationFailedMessage();
void suOnSpiesUtilitiesInitializationFailure();

#endif //__SUSPIESUTILITIESDLLINITIALIZATIONFUNCTIONS_H

