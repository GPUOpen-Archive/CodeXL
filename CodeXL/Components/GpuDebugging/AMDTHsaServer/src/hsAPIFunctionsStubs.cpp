//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file hsAPIFunctionsStubs.cpp
///
//==================================================================================

// ------------------------------ hsAPIFunctionsStubs.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTAPIClasses/Include/apAPIFunctionId.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>

// Local:
#include <src/hsAPIFunctionsImplementations.h>
#include <src/hsAPIFunctionsStubs.h>


// ---------------------------------------------------------------------------
// Name:        hsRegisterAPIStubFunctions
// Description: Registers HSA Server module stub functions.
// Author:      Uri Shomroni
// Date:        13/9/2015
// ---------------------------------------------------------------------------
void hsRegisterAPIStubFunctions()
{
    suRegisterAPIFunctionStub(GA_FID_gaIsInHSAKernelBreakpoint, &gaIsInHSAKernelBreakpointStub);
    suRegisterAPIFunctionStub(GA_FID_gaHSAGetCurrentLine, &gaHSAGetCurrentLineStub);
    suRegisterAPIFunctionStub(GA_FID_gaHSAGetSourceFilePath, &gaHSAGetSourceFilePathStub);
    suRegisterAPIFunctionStub(GA_FID_gaHSASetNextDebuggingCommand, &gaHSASetNextDebuggingCommandStub);
    suRegisterAPIFunctionStub(GA_FID_gaHSASetBreakpoint, &gaHSASetBreakpointStub);
    suRegisterAPIFunctionStub(GA_FID_gaHSAListVariables, &gaHSAListVariablesStub);
    suRegisterAPIFunctionStub(GA_FID_gaHSAGetExpressionValue, &gaHSAGetExpressionValueStub);
    suRegisterAPIFunctionStub(GA_FID_gaHSAListWorkItems, &gaHSAListWorkItemsStub);
    suRegisterAPIFunctionStub(GA_FID_gaHSASetActiveWorkItemIndex, &gaHSASetActiveWorkItemIndexStub);
    suRegisterAPIFunctionStub(GA_FID_gaHSAGetWorkDims, &gaHSAGetWorkDimsStub);
}

void gaIsInHSAKernelBreakpointStub(osSocket& apiSocket)
{
    bool retVal = gaIsInHSAKernelBreakpointImpl();

    apiSocket << retVal;
}

void gaHSAGetCurrentLineStub(osSocket& apiSocket)
{
    gtUInt64 line = 0;
    gtUInt64 addr = 0;
    bool retVal = gaHSAGetCurrentLineImpl(line, addr);

    apiSocket << retVal;

    if (retVal)
    {
        apiSocket << line;
        apiSocket << addr;
    }
}

void gaHSAGetSourceFilePathStub(osSocket& apiSocket)
{
    osFilePath srcPath;
    gtString kernelName;
    bool retVal = gaHSAGetSourceFilePathImpl(srcPath, kernelName);

    apiSocket << retVal;

    if (retVal)
    {
        retVal = srcPath.writeSelfIntoChannel(apiSocket);

        apiSocket << kernelName;
    }
}

void gaHSASetNextDebuggingCommandStub(osSocket& apiSocket)
{
    gtUInt32 cmdAsUInt32 = 0;
    apiSocket >> cmdAsUInt32;

    bool retVal = gaHSASetNextDebuggingCommandImpl((apKernelDebuggingCommand)cmdAsUInt32);

    apiSocket << retVal;
}

void gaHSASetBreakpointStub(osSocket& apiSocket)
{
    gtString kernelName;
    apiSocket >> kernelName;

    gtUInt64 line = 0;
    apiSocket >> line;

    bool retVal = gaHSASetBreakpointImpl(kernelName, line);

    apiSocket << retVal;
}

void gaHSAListVariablesStub(osSocket& apiSocket)
{
    gtInt32 evalDepthAsInt32 = 0;
    apiSocket >> evalDepthAsInt32;

    gtVector<apExpression> variables;
    bool retVal = gaHSAListVariablesImpl((int)evalDepthAsInt32, variables);

    apiSocket << retVal;

    if (retVal)
    {
        gtUInt32 varCount = (gtUInt32)variables.size();
        apiSocket << varCount;

        for (gtUInt32 i = 0; varCount > i; ++i)
        {
            bool rcVar = variables[i].writeSelfIntoChannel(apiSocket);
            GT_ASSERT(rcVar);
        }
    }
}

void gaHSAGetExpressionValueStub(osSocket& apiSocket)
{
    gtString varName;
    apiSocket >> varName;

    gtInt32 evalDepthAsInt32 = 0;
    apiSocket >> evalDepthAsInt32;

    apExpression expr;
    bool retVal = gaHSAGetExpressionValueImpl(varName, (int)evalDepthAsInt32, expr);

    apiSocket << retVal;

    if (retVal)
    {
        bool rcVar = expr.writeSelfIntoChannel(apiSocket);
        GT_ASSERT(rcVar);
    }
}

void gaHSAListWorkItemsStub(osSocket& apiSocket)
{
    gtVector<gtUInt32> gidLidWgid;
    bool retVal = gaHSAListWorkItemsImpl(gidLidWgid);

    gtUInt32 coordCount = (gtUInt32)gidLidWgid.size();
    retVal = retVal && (0 == (coordCount % 9));

    apiSocket << retVal;

    if (retVal)
    {
        // Send the work item count:
        apiSocket << (coordCount / 9);

        for (gtUInt32 i = 0; coordCount > i; ++i)
        {
            apiSocket << gidLidWgid[i];
        }
    }
}

void gaHSASetActiveWorkItemIndexStub(osSocket& apiSocket)
{
    gtUInt32 wiIndex = 0;
    apiSocket >> wiIndex;

    bool retVal = gaHSASetActiveWorkItemIndexImpl(wiIndex);

    apiSocket << retVal;
}

void gaHSAGetWorkDimsStub(osSocket& apiSocket)
{
    gtUByte dims = 0;
    bool retVal = gaHSAGetWorkDimsImpl(dims);

    apiSocket << retVal;

    if (retVal)
    {
        apiSocket << dims;
    }
}
