//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaStringConstants.h
///
//==================================================================================

//------------------------------ gaStringConstants.h ------------------------------

#ifndef __GASTRINGCONSTANTS
#define __GASTRINGCONSTANTS

// Shared memory objects prefix and suffix:
#define GA_SHARED_MEM_OBJ_PREFIX L"AMDTSharedMemObj"
#define GA_SHARED_MEM_OBJ_API_SUFFIX L"-API"
#define GA_SHARED_MEM_OBJ_INCOMING_EVENTS_SUFFIX L"-IncomingEvents"

// Debug log strings:
#define GA_STR_FailedToInitOGLSpyAPI L"The OpenGL Spy side of the API failed to initialize"
#define GA_STR_FailedToInitOCLSpyAPI L"The OpenCL Spy side of the API failed to initialize"
#define GA_STR_FailedToInitAPIConnection L"Failed to initialize the CodeXL API connection"
#define GA_STR_FailedToResumeMainThread L"Failed to resume the debugged process main thread run"
#define GA_STR_FailedToSetSpyPersistantData L"Failed to set the spy persistant data values"
#define GA_STR_InitAPIDll L"Initializing API package"
#define GA_STR_FinishedInitAPIDll L"Finished initializing API Package"
#define GA_STR_FailedToInitAPIDll L"Failed to initialize the API package"
#define GA_STR_CreatedAPIToSpyConnector L"API to Spies connector initialized"
#define GA_STR_UnkownAPIConnectionType L"Unknown API connection type"
#define GA_STR_gaGRApiFunctionsRegisterdWithNonNullInstance L"!!! gaGRApiFunctions::registerInstance() called twice or called after gaGRApiFunctions::instance() !!!"
#define GA_STR_GotOpenGLServerAPIConnection L"Got API connection with the OpenGL Server"
#define GA_STR_GotOpenCLServerAPIConnection L"Got API connection with the OpenCL Server"


#endif  // __GASTRINGCONSTANTS
