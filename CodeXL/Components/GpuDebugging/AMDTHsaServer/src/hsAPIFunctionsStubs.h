//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file hsAPIFunctionsStubs.h
///
//==================================================================================

#ifndef __HSAPIFUNCTIONSTUBS_H
#define __HSAPIFUNCTIONSTUBS_H

// Forward Declarations:
class osSocket;

void hsRegisterAPIStubFunctions();

void gaIsInHSAKernelBreakpointStub(osSocket& apiSocket);
void gaHSAGetCurrentLineStub(osSocket& apiSocket);
void gaHSAGetSourceFilePathStub(osSocket& apiSocket);
void gaHSASetNextDebuggingCommandStub(osSocket& apiSocket);
void gaHSASetBreakpointStub(osSocket& apiSocket);
void gaHSAListVariablesStub(osSocket& apiSocket);
void gaHSAGetVariableValueStub(osSocket& apiSocket);
void gaHSAListWorkItemsStub(osSocket& apiSocket);
void gaHSASetActiveWorkItemIndexStub(osSocket& apiSocket);
void gaHSAGetWorkDimsStub(osSocket& apiSocket);

#endif // __HSAPIFUNCTIONSTUBS_H
