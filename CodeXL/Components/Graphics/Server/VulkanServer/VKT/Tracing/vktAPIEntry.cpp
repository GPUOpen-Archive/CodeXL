//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktAPIEntry.cpp
/// \brief  A Vulkan-specific entry for a traced API call.
//=============================================================================

#include "vktAPIEntry.h"
#include "../Tracing/vktTraceAnalyzerLayer.h"
#include "../Profiling/vktFrameProfilerLayer.h"
#include "../../../Common/TypeToString.h"
#include "../Util/vktUtil.h"
#include "../vktDefines.h"

//-----------------------------------------------------------------------------
/// Constructor used to initialize members of new VktAPIEntry instances.
/// \param inThreadId The ThreadId that the call was invoked from.
/// \param inFunctionId The function Id of the API call being traced.
/// \param inArguments A stringified list of arguments for the invocation.
/// \param pWrappedCmdBuf The CommandBuffer affected by the API call.
//-----------------------------------------------------------------------------
VktAPIEntry::VktAPIEntry(UINT inThreadId, FuncId inFunctionId, const std::string& inArguments, VktWrappedCmdBuf* pWrappedCmdBuf)
    : APIEntry(inThreadId, inFunctionId, inArguments)
    , m_sampleId(0)
    , m_returnValue(VK_INCOMPLETE)
    , m_pWrappedCmdBuf(pWrappedCmdBuf)
{
}

//-----------------------------------------------------------------------------
/// Constructor used to initialize members of new VktAPIEntry instances.
/// \param inThreadId The ThreadId that the call was invoked from.
/// \param inFunctionId The function Id of the API call being traced.
/// \param pParams A list of parameters for this call.
/// \param paramCount The parameter count for this call.
/// \param pWrappedCmdBuf The CommandBuffer affected by the API call.
//-----------------------------------------------------------------------------
VktAPIEntry::VktAPIEntry(UINT inThreadId, FuncId inFunctionId, ParameterEntry* pParams, UINT32 paramCount, VktWrappedCmdBuf* pWrappedCmdBuf)
    : APIEntry(inThreadId, inFunctionId, paramCount)
    , m_sampleId(0)
    , m_returnValue(VK_INCOMPLETE)
    , m_pWrappedCmdBuf(pWrappedCmdBuf)
{
    if (mParameterBuffer != nullptr)
    {
        for (UINT32 i = 0; i < paramCount; i++)
        {
            AddParameter(i, pParams[i].mType, pParams[i].mData);
        }
    }
}

//-----------------------------------------------------------------------------
/// Convert the APIEntry's FunctionId to a readable string.
/// \returns A stringified version of the API name.
//-----------------------------------------------------------------------------
const char* VktAPIEntry::GetAPIName() const
{
    return VktTraceAnalyzerLayer::Instance()->GetFunctionNameFromId(mFunctionId);
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

//-----------------------------------------------------------------------------
/// Add a single API trace parameter to the list
/// \param index the parameter index (0-based)
/// \param type the data type of the parameter
/// \param pParameterValue pointer to the parameter value
/// Each buffer parameter is BYTES_PER_PARAMETER bytes. The first 4 bytes
/// are for the parameter data type. The next byte is the number of bytes
/// used to store the parameter value. The remaining space is used for the
/// parameter value.
//-----------------------------------------------------------------------------
void VktAPIEntry::AddParameter(unsigned int index, int type, const void* pParameterValue)
{
    VKT_ASSERT(index < mNumParameters);

    if (mParameterBuffer)
    {
        unsigned int bufferLength = BYTES_PER_PARAMETER - 5;
        unsigned int length = 0;
        char* buffer = mParameterBuffer;

        // find index and write data type
        buffer += (index * BYTES_PER_PARAMETER);
        memcpy(buffer, &type, sizeof(PARAMETER_TYPE));

        // increment pointer and write data length. Standard data types don't store length
        // to save time since the length is known and will be fixed up when displaying the data
        buffer += sizeof(PARAMETER_TYPE);

        switch (type)
        {
            // TODO: have this as a look up table
        case PARAMETER_POINTER:
        case PARAMETER_POINTER_SPECIAL:
            length = (char)sizeof(void*);
            break;

        case PARAMETER_INT:
        case PARAMETER_UNSIGNED_INT:
            length = (char)sizeof(unsigned int);
            break;

        case PARAMETER_UNSIGNED_CHAR:
            length = (char)sizeof(unsigned char);
            break;

        case PARAMETER_FLOAT:
            length = (char)sizeof(float);
            break;

        case PARAMETER_BOOL:
            length = (char)sizeof(BOOL);
            break;

        case PARAMETER_UINT64:
            length = (char)sizeof(UINT64);
            break;

        case PARAMETER_SIZE_T:
            length = (char)sizeof(size_t);
            break;

        case PARAMETER_STRING:
            length = (char)strlen((const char*)pParameterValue);
            length++;                       // add null terminator to length

            if (length > bufferLength)
            {
                // 1 byte less to account for terminator
                length = bufferLength - 1;
            }

            break;

        case PARAMETER_WIDE_STRING:
            length = (char)wcslen((const LPCWSTR)pParameterValue);
            length++;                       // add null terminator to length
            length *= sizeof(WCHAR);        // convert length to bytes

            if (length > bufferLength)
            {
                // round down length to even number
                length = ((bufferLength - 2) & 0xfffffffe);
            }

            break;

        default:
            break;
        }

        *buffer++ = (char)length;

        if (type == PARAMETER_POINTER || type == PARAMETER_POINTER_SPECIAL)
        {
            // write the value of the pointer
            memcpy_s(buffer, bufferLength, &pParameterValue, length);
        }
        else
        {
            // make sure last 2 bytes are null characters incase it's a string or wide string.
            // This is used as the terminator for a truncated string
            int* bufPtr = (int*)&buffer[bufferLength - 4];
            *bufPtr = 0;
            // write what the pointer is pointing to
            memcpy_s(buffer, bufferLength, pParameterValue, length);
        }
    }
}

//-----------------------------------------------------------------------------
/// Convert an API parameter from raw data to a string for display
/// \param paramType the data type of the parameter
/// \param dataLength the number of bytes used to contain the data
/// \param pRawData a pointer to the raw data
/// \param ioParameterString a buffer passed in where the string is to be stored
//-----------------------------------------------------------------------------
void VktAPIEntry::GetParameterAsString(PARAMETER_TYPE paramType, const char dataLength, const char* pRawData, char* ioParameterString) const
{
    int bufferLength = BYTES_PER_PARAMETER - 5;

    switch (paramType)
    {
    case PARAMETER_POINTER:
    {
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
        sprintf_s(ioParameterString, bufferLength, GT_POINTER_FORMAT, reinterpret_cast<gtUInt32>(pRawData));
#else
        sprintf_s(ioParameterString, bufferLength, GT_POINTER_FORMAT, reinterpret_cast<gtUInt64>(pRawData));
#endif
        break;
    }

    case PARAMETER_POINTER_SPECIAL:
    {
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
        sprintf_s(ioParameterString, bufferLength, "+" GT_POINTER_FORMAT, reinterpret_cast<gtUInt32>(pRawData));
#else
        sprintf_s(ioParameterString, bufferLength, "+" GT_POINTER_FORMAT, reinterpret_cast<gtUInt64>(pRawData));
#endif
        break;
    }

    case PARAMETER_INT:
    {
        int data = 0;
        memcpy(&data, pRawData, sizeof(int));
        sprintf_s(ioParameterString, bufferLength, "%d", data);
        break;
    }

    case PARAMETER_UNSIGNED_INT:
    {
        unsigned int data = 0;
        memcpy(&data, pRawData, sizeof(unsigned int));
        sprintf_s(ioParameterString, bufferLength, "%u", data);
        break;
    }

    case PARAMETER_UNSIGNED_CHAR:
    {
        unsigned char data = 0;
        memcpy(&data, pRawData, sizeof(unsigned char));
        sprintf_s(ioParameterString, bufferLength, "%hhu", data);
        break;
    }
    ;
    case PARAMETER_BOOL:
    {
        BOOL data = FALSE;
        memcpy(&data, pRawData, sizeof(BOOL));
        sprintf_s(ioParameterString, bufferLength, "%s", ((data == TRUE) ? "TRUE" : "FALSE"));
        break;
    }

    case PARAMETER_FLOAT:
    {
        float data = 0.0f;
        memcpy(&data, pRawData, sizeof(float));
        sprintf_s(ioParameterString, bufferLength, "%f", data);
        break;
    }

    case PARAMETER_UINT64:
    {
        UINT64 data = 0;
        memcpy(&data, pRawData, sizeof(UINT64));
        sprintf_s(ioParameterString, bufferLength, "%llu", data);
        break;
    }

    case PARAMETER_SIZE_T:
    {
        size_t data = 0;
        memcpy(&data, pRawData, sizeof(size_t));
        sprintf_s(ioParameterString, bufferLength, "%Iu", data);
        break;
    }

    case PARAMETER_STRING:
    {
        // copy string data directly to output buffer
        memcpy_s(ioParameterString, bufferLength, pRawData, dataLength);
        break;
    }

    case PARAMETER_WIDE_STRING:
    {
        sprintf_s(ioParameterString, bufferLength, "%hs", pRawData);
        break;
    }

    default:
    {
        PsAssert(0);
        break;
    }
    }
}