//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktAPIEntry.cpp
/// \brief  A Vulkan-specific entry for a traced API call.
//=============================================================================

#include "vktAPIEntry.h"
#include "../Tracing/VktTraceAnalyzerLayer.h"
#include "../Profiling/VktFrameProfilerLayer.h"
#include "../../../Common/TypeToString.h"

//-----------------------------------------------------------------------------
/// Constructor used to initialize members of new VktAPIEntry instances.
/// \param inThreadId The ThreadId that the call was invoked from.
/// \param inFunctionId The function Id of the API call being traced.
/// \param inArguments A stringified list of arguments for the invocation.
/// \param pWrappedCmdBuf The CommandBuffer affected by the API call.
//-----------------------------------------------------------------------------
VktAPIEntry::VktAPIEntry(UINT inThreadId, FuncId inFunctionId, const std::string& inArguments, VktWrappedCmdBuf* pWrappedCmdBuf)
    : APIEntry(inThreadId, inFunctionId, inArguments)
    , m_returnValue(VK_INCOMPLETE)
    , m_sampleId(0)
    , m_pWrappedCmdBuf(pWrappedCmdBuf)
{
}

//-----------------------------------------------------------------------------
/// Convert the APIEntry's FunctionId to a readable string.
/// \returns A stringified version of the API name.
//-----------------------------------------------------------------------------
const char* VktAPIEntry::GetAPIName() const
{
    return VktTraceAnalyzerLayer::Instance()->GetFunctionNameFromId(this->mFunctionId);
}

//-----------------------------------------------------------------------------
/// Append this APIEntry's information to the end of the API Trace response.
/// \param out The output trace response string to append the trace line to.
/// \param startTime The start time for the API call.
/// \param endTime The end time for the API call.
//-----------------------------------------------------------------------------
void VktAPIEntry::AppendAPITraceLine(gtASCIIString& out, double startTime, double endTime) const
{
    const char* pResultCode = m_returnValue != -1 ? VktUtil::WriteResultCodeEnumAsString(m_returnValue) : "void";

    out += IntToString(VktTraceAnalyzerLayer::Instance()->GetAPIGroupFromAPI(mFunctionId));
    out += " ";

    out += IntToString(mFunctionId);
    out += " ";

    out += "0x0000000000000000";
    out += " ";

    out += "Vulkan_";

    out += GetAPIName();

    out += "(";
    out += mParameters.asCharArray();
    out += ") = ";

    out += pResultCode;

    out += " ";
    out += DoubleToString(startTime);

    out += " ";
    out += DoubleToString(endTime);

    out += " ";
    out += UINT64ToString(m_sampleId);

    out += "\n";
}

//-----------------------------------------------------------------------------
/// Set the return value for this logged APIEntry
/// \param inReturnValue The return value
//-----------------------------------------------------------------------------
void VktAPIEntry::SetReturnValue(INT64 inReturnValue)
{
    m_returnValue = (VkResult)inReturnValue;
}

//-----------------------------------------------------------------------------
/// Check if this APIEntry is classified as a Draw call.
/// \returns True if this is a draw call. False if it's not.
//-----------------------------------------------------------------------------
bool VktAPIEntry::IsDrawCall() const
{
    return VktFrameProfilerLayer::Instance()->ShouldProfileFunction(mFunctionId);
}