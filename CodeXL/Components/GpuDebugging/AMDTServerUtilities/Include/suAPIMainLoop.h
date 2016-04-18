//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suAPIMainLoop.h
///
//==================================================================================

//------------------------------ suAPIMainLoop.h ------------------------------

#ifndef __SUAPIMAINLOOP_H
#define __SUAPIMAINLOOP_H

// Forward decelerations:
class osSocket;

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <AMDTServerUtilities/Include/suSpiesUtilitiesDLLBuild.h>


SU_API void suAPIMainLoop(osSocket& apiSocket);
SU_API int suProcessSingleAPICall();
SU_API int suHandleSingleAPICall(osSocket& apiSocket, gtInt32& executedFunctionId);
SU_API void suCallAPIFunctionStub(osSocket& apiSocket, long functionId);


#endif //__SUAPIMAINLOOP_H

