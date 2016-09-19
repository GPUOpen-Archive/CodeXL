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
    out += GetParameterString();
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
        unsigned int bufferLength = BYTES_PER_PARAMETER_DATA;
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
                Log(logMESSAGE, "VktAPIEntry::AddParameter: Parameter string too large. String will be truncated\n");
            }

            break;

        case PARAMETER_VK_HANDLE:
            length = (char)sizeof(VkCommandBuffer);
            break;

        case PARAMETER_VK_VkPipelineCacheHeaderVersion:
        case PARAMETER_VK_VkResultCode:
        case PARAMETER_VK_VkStructureType:
        case PARAMETER_VK_VkSystemAllocationScope:
        case PARAMETER_VK_VkInternalAllocationType:
        case PARAMETER_VK_VkFormat:
        case PARAMETER_VK_VkImageType:
        case PARAMETER_VK_VkImageTiling:
        case PARAMETER_VK_VkPhysicalDeviceType:
        case PARAMETER_VK_VkQueryType:
        case PARAMETER_VK_VkSharingMode:
        case PARAMETER_VK_VkImageLayout:
        case PARAMETER_VK_VkImageViewType:
        case PARAMETER_VK_VkComponentSwizzle:
        case PARAMETER_VK_VkVertexInputRate:
        case PARAMETER_VK_VkPrimitiveTopology:
        case PARAMETER_VK_VkPolygonMode:
        case PARAMETER_VK_VkFrontFace:
        case PARAMETER_VK_VkCompareOp:
        case PARAMETER_VK_VkStencilOp:
        case PARAMETER_VK_VkLogicOp:
        case PARAMETER_VK_VkBlendFactor:
        case PARAMETER_VK_VkBlendOp:
        case PARAMETER_VK_VkDynamicState:
        case PARAMETER_VK_VkFilter:
        case PARAMETER_VK_VkSamplerMipmapMode:
        case PARAMETER_VK_VkSamplerAddressMode:
        case PARAMETER_VK_VkBorderColor:
        case PARAMETER_VK_VkDescriptorType:
        case PARAMETER_VK_VkAttachmentLoadOp:
        case PARAMETER_VK_VkAttachmentStoreOp:
        case PARAMETER_VK_VkPipelineBindPoint:
        case PARAMETER_VK_VkCmdBufferLevel:
        case PARAMETER_VK_VkIndexType:
        case PARAMETER_VK_VkSubpassContents:
        case PARAMETER_VK_VkColorSpaceKHR:
        case PARAMETER_VK_VkPresentModeKHR:
        case PARAMETER_VK_VkDebugReportObjectTypeEXT:
        case PARAMETER_VK_VkDebugReportErrorEXT:
        case PARAMETER_VK_VkFormatFeatureFlags:
        case PARAMETER_VK_VkImageUsageFlags:
        case PARAMETER_VK_VkImageCreateFlags:
        case PARAMETER_VK_VkSampleCountFlags:
        case PARAMETER_VK_VkQueueFlags:
        case PARAMETER_VK_VkMemoryPropertyFlags:
        case PARAMETER_VK_VkMemoryMapFlags:
        case PARAMETER_VK_VkSparseImageFormatFlags:
        case PARAMETER_VK_VkSparseMemoryBindFlags:
        case PARAMETER_VK_VkFenceCreateFlags:
        case PARAMETER_VK_VkQueryPipelineStatisticFlags:
        case PARAMETER_VK_VkQueryResultFlags:
        case PARAMETER_VK_VkBufferUsageFlags:
        case PARAMETER_VK_VkBufferCreateFlags:
        case PARAMETER_VK_VkImageAspectFlags:
        case PARAMETER_VK_VkColorComponentFlags:
        case PARAMETER_VK_VkDescriptorPoolCreateFlags:
        case PARAMETER_VK_VkPipelineCreateFlags:
        case PARAMETER_VK_VkShaderStageFlags:
        case PARAMETER_VK_VkCullModeFlags:
        case PARAMETER_VK_VkAttachmentDescriptionFlags:
        case PARAMETER_VK_VkPipelineStageFlags:
        case PARAMETER_VK_VkAccessFlags:
        case PARAMETER_VK_VkDependencyFlags:
        case PARAMETER_VK_VkCmdPoolCreateFlags:
        case PARAMETER_VK_VkCmdPoolResetFlags:
        case PARAMETER_VK_VkCmdBufferUsageFlags:
        case PARAMETER_VK_VkStencilFaceFlags:
        case PARAMETER_VK_VkQueryControlFlags:
        case PARAMETER_VK_VkCommandBufferResetFlags:
        case PARAMETER_VK_VkSurfaceTransformFlagBitsKHRFlags:
        case PARAMETER_VK_VkCompositeAlphaFlags:
            length = (char)sizeof(uint32_t);
            break;

        default:
            break;
        }

        bufferSize_t len = (bufferSize_t)length;
        memcpy(buffer, &len, sizeof(bufferSize_t));
        buffer += sizeof(bufferSize_t);

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
void VktAPIEntry::GetParameterAsString(PARAMETER_TYPE paramType, UINT dataLength, const char* pRawData, char* ioParameterString) const
{
    int bufferLength = BYTES_PER_PARAMETER_DATA;

    switch (paramType)
    {
    case PARAMETER_VK_HANDLE:
    case PARAMETER_POINTER:
    {
        void* data = nullptr;
        memcpy(&data, pRawData, sizeof(void*));

#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
        sprintf_s(ioParameterString, bufferLength, GT_POINTER_FORMAT, reinterpret_cast<gtUInt32>(data));
#else
        sprintf_s(ioParameterString, bufferLength, GT_POINTER_FORMAT, reinterpret_cast<gtUInt64>(data));
#endif

        break;
    }

    case PARAMETER_POINTER_SPECIAL:
    {
        void* data = nullptr;
        memcpy(&data, pRawData, sizeof(void*));

#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
        sprintf_s(ioParameterString, bufferLength, "+" GT_POINTER_FORMAT, reinterpret_cast<gtUInt32>(data));
#else
        sprintf_s(ioParameterString, bufferLength, "+" GT_POINTER_FORMAT, reinterpret_cast<gtUInt64>(data));
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

    case PARAMETER_VK_VkPipelineCacheHeaderVersion:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WritePipelineCacheHeaderVersionEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkResultCode:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteResultCodeEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkStructureType:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteStructureTypeEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkSystemAllocationScope:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteSystemAllocationScopeEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkInternalAllocationType:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteInternalAllocationTypeEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkFormat:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteFormatEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkImageType:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteImageTypeEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkImageTiling:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteImageTilingEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkPhysicalDeviceType:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WritePhysicalDeviceTypeEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkQueryType:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteQueryTypeEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkSharingMode:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteSharingModeEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkImageLayout:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteImageLayoutEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkImageViewType:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteImageViewTypeEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkComponentSwizzle:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteComponentSwizzleEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkVertexInputRate:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteVertexInputRateEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkPrimitiveTopology:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WritePrimitiveTopologyEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkPolygonMode:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WritePolygonModeEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkFrontFace:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteFrontFaceEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkCompareOp:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteCompareOpEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkStencilOp:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteStencilOpEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkLogicOp:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteLogicOpEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkBlendFactor:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteBlendFactorEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkBlendOp:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteBlendOpEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkDynamicState:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteDynamicStateEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkFilter:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteFilterEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkSamplerMipmapMode:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteSamplerMipmapModeEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkSamplerAddressMode:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteSamplerAddressModeEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkBorderColor:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteBorderColorEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkDescriptorType:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteDescriptorTypeEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkAttachmentLoadOp:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteAttachmentLoadOpEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkAttachmentStoreOp:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteAttachmentStoreOpEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkPipelineBindPoint:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WritePipelineBindPointEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkCmdBufferLevel:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteCmdBufferLevelEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkIndexType:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteIndexTypeEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkSubpassContents:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteSubpassContentsEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkColorSpaceKHR:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteColorSpaceKHREnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkPresentModeKHR:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WritePresentModeKHREnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkDebugReportObjectTypeEXT:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteDebugReportObjectTypeEXTEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkDebugReportErrorEXT:
    {
        uint32_t val = {};
        memcpy(&val, pRawData, sizeof(uint32_t));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::WriteDebugReportErrorEXTEnumAsString(val));
        break;
    }
    case PARAMETER_VK_VkFormatFeatureFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeFormatFeatureFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkImageUsageFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeImageUsageFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkImageCreateFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeImageCreateFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkSampleCountFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeSampleCountFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkQueueFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeQueueFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkMemoryPropertyFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeMemoryPropertyFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkMemoryMapFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeMemoryMapFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkSparseImageFormatFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeSparseImageFormatFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkSparseMemoryBindFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeSparseMemoryBindFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkFenceCreateFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeFenceCreateFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkQueryPipelineStatisticFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeQueryPipelineStatisticFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkQueryResultFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeQueryResultFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkBufferUsageFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeBufferUsageFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkBufferCreateFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeBufferCreateFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkImageAspectFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeImageAspectFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkColorComponentFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeColorComponentFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkDescriptorPoolCreateFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeDescriptorPoolCreateFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkPipelineCreateFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposePipelineCreateFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkShaderStageFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeShaderStageFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkCullModeFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeCullModeFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkAttachmentDescriptionFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeAttachmentDescriptionFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkPipelineStageFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposePipelineStageFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkAccessFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeAccessFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkDependencyFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeDependencyFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkCmdPoolCreateFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeCmdPoolCreateFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkCmdPoolResetFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeCmdPoolResetFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkCmdBufferUsageFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeCmdBufferUsageFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkStencilFaceFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeStencilFaceFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkQueryControlFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeQueryControlFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkCommandBufferResetFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeCommandBufferResetFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkSurfaceTransformFlagBitsKHRFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeSurfaceTransformFlagBitsKHRFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkCompositeAlphaFlags:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeCompositeAlphaFlagsEnumAsString(flags).c_str());
        break;
    }
    case PARAMETER_VK_VkDebugReportFlagsEXT:
    {
        VkFlags flags = {};
        memcpy(&flags, pRawData, sizeof(VkFlags));
        sprintf_s(ioParameterString, bufferLength, "%s", VktUtil::DecomposeDebugReportFlagsEXTEnumAsString(flags).c_str());
        break;
    }

    default:
    {
        PsAssert(0);
        break;
    }
    }
}
