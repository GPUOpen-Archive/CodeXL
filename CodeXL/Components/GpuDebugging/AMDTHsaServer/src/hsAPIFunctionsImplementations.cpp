//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file hsAPIFunctionsImplementations.cpp
///
//==================================================================================


// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apExpression.h>

// Local:
#include <src/hsAPIFunctionsImplementations.h>
#include <src/hsDebuggingManager.h>
#include <src/hsDebugInfo.h>

bool gaIsInHSAKernelBreakpointImpl()
{
    bool retVal = hsDebuggingManager::instance().IsInHSAKernelBreakpoint();

    return retVal;
}

bool gaHSAGetCurrentLineImpl(gtUInt64& line, gtUInt64& addr)
{
    bool retVal = false;

    hsDebuggingManager& theDebuggingMgr = hsDebuggingManager::instance();
    addr = theDebuggingMgr.GetCurrentAddress();
    const hsDebugInfo* pDebugInfo = theDebuggingMgr.GetCurrentDebugInfo();
    GT_IF_WITH_ASSERT((0 != addr) && (nullptr != pDebugInfo))
    {
        retVal = pDebugInfo->AddrToLine(addr, line);
    }

    return retVal;
}

bool gaHSAGetSourceFilePathImpl(osFilePath& srcPath, gtString& kernelName)
{
    bool retVal = false;

    hsDebuggingManager& theDebuggingMgr = hsDebuggingManager::instance();

    kernelName = theDebuggingMgr.GetKernelName();

    const hsDebugInfo* pDebugInfo = theDebuggingMgr.GetCurrentDebugInfo();
    GT_IF_WITH_ASSERT(nullptr != pDebugInfo)
    {
        retVal = true;
        srcPath = pDebugInfo->GetSourceFilePath();
    }

    return retVal;
}

bool gaHSASetNextDebuggingCommandImpl(apKernelDebuggingCommand cmd)
{
    bool retVal = hsDebuggingManager::instance().SetNextDebuggingCommand(cmd);

    return retVal;
}

bool gaHSASetBreakpointImpl(const gtString& kernelName, gtUInt64 line)
{
    bool retVal = hsDebuggingManager::instance().SetBreakpoint(kernelName, line);

    return retVal;
}

bool gaHSAListVariablesImpl(int evalDepth, gtVector<apExpression>& variables)
{
    bool retVal = false;

    const hsDebugInfo* pDebugInfo = hsDebuggingManager::instance().GetCurrentDebugInfo();
    GT_IF_WITH_ASSERT(nullptr != pDebugInfo)
    {
        gtVector<gtString> variableNames;
        retVal = pDebugInfo->ListVariables(variableNames);

        variables.reserve(variableNames.size());
        for (const gtString& varNm : variableNames)
        {
            // TO_DO: currently, onlyNames = false is assumed. This should instead be a parameter.
            apExpression expr;
            bool rcVar = gaHSAGetExpressionValueImpl(varNm, evalDepth, expr);
            GT_ASSERT(rcVar);
            variables.push_back(expr);
        }
    }

    return retVal;
}

bool gaHSAGetExpressionValueImpl(const gtString& varName, int evalDepth, apExpression& varValue)
{
    bool retVal = false;

    const hsDebugInfo* pDebugInfo = hsDebuggingManager::instance().GetCurrentDebugInfo();
    GT_IF_WITH_ASSERT(nullptr != pDebugInfo)
    {
        // TO_DO: handle members and evalDepth:
        GT_UNREFERENCED_PARAMETER(evalDepth);
        retVal = pDebugInfo->EvaluateVariable(varName, varValue.m_value, &varValue.m_valueHex, &varValue.m_type);
    }

    return retVal;
}

bool gaHSAListWorkItemsImpl(gtVector<gtUInt32>& o_gidLidWgid)
{
    bool retVal = false;

    hsDebuggingManager& theDebuggingMgr = hsDebuggingManager::instance();
    gtUInt32 wfCount = theDebuggingMgr.GetWavefrontCount();

    if (0 < wfCount)
    {
        retVal = true;
        bool ignored = false;

        // HS_WAVEFRONT_SIZE work items / wavefront
        // 3 ids / work item
        // 3 coordinates / id
        o_gidLidWgid.resize(wfCount * HS_WAVEFRONT_SIZE * 9);

        for (gtUInt32 i = 0; (wfCount > i) && retVal; ++i)
        {
            for (gtUByte j = 0; (HS_WAVEFRONT_SIZE > j) && retVal; ++j)
            {
                gtUInt32 baseIndex = ((HS_WAVEFRONT_SIZE * i) + (gtUInt32)j) * 9;
                retVal = retVal && theDebuggingMgr.GetWorkItemId(i, j, &(o_gidLidWgid[baseIndex]), &(o_gidLidWgid[baseIndex + 3]), &(o_gidLidWgid[baseIndex + 6]), ignored);
            }
        }
    }

    return retVal;
}

bool gaHSASetActiveWorkItemIndexImpl(gtUInt32 wiIndex)
{
    bool retVal = hsDebuggingManager::instance().SetActiveWavefront(wiIndex / HS_WAVEFRONT_SIZE, (gtUByte)(wiIndex % HS_WAVEFRONT_SIZE));

    return retVal;
}

bool gaHSAGetWorkDimsImpl(gtUByte& dims)
{
    dims = hsDebuggingManager::instance().GetWorkDimensions();

    bool retVal = (0 != dims);

    return retVal;
}


