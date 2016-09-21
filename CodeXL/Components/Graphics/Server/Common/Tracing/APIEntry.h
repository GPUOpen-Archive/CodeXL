//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file APIEntry.h
/// \brief The APIEntry structure is used to track traced call invocations.
//==============================================================================

#ifndef APIENTRY_H
#define APIENTRY_H

#include <AMDTBaseTools/Include/gtASCIIString.h>
#include "../CommonTypes.h"
#include <string>

enum FuncId : int;

//-----------------------------------------------------------------------------
/// List of parameter types used for the API trace parameters.
/// Data is stored as binary and reconstituted before sending the data back to the client.
//-----------------------------------------------------------------------------
enum PARAMETER_TYPE
{
    // Generic data types
    PARAMETER_POINTER,
    PARAMETER_POINTER_SPECIAL,
    PARAMETER_INT,
    PARAMETER_UNSIGNED_INT,
    PARAMETER_UNSIGNED_CHAR,
    PARAMETER_FLOAT,
    PARAMETER_BOOL,
    PARAMETER_UINT64,
    PARAMETER_SIZE_T,
    PARAMETER_STRING,
    PARAMETER_WIDE_STRING,
    PARAMETER_ARRAY,

    // DX12-specific data types requiring special treatment
    PARAMETER_DX12_GUID,
    PARAMETER_DX12_REFIID,
    PARAMETER_DX12_DXGI_FORMAT,
    PARAMETER_DX12_PRIMITIVE_TOPOLOGY,
    PARAMETER_DX12_QUERY_TYPE,
    PARAMETER_DX12_PREDICATION_OP,
    PARAMETER_DX12_COMMAND_LIST,
    PARAMETER_DX12_FEATURE,
    PARAMETER_DX12_DESCRIPTOR_HEAP,
    PARAMETER_DX12_HEAP_TYPE,
    PARAMETER_DX12_RESOURCE_STATES,

    // Vulkan-specific data types requiring special treatment
    PARAMETER_VK_HANDLE,
    PARAMETER_VK_VkPipelineCacheHeaderVersion,
    PARAMETER_VK_VkResultCode,
    PARAMETER_VK_VkStructureType,
    PARAMETER_VK_VkSystemAllocationScope,
    PARAMETER_VK_VkInternalAllocationType,
    PARAMETER_VK_VkFormat,
    PARAMETER_VK_VkImageType,
    PARAMETER_VK_VkImageTiling,
    PARAMETER_VK_VkPhysicalDeviceType,
    PARAMETER_VK_VkQueryType,
    PARAMETER_VK_VkSharingMode,
    PARAMETER_VK_VkImageLayout,
    PARAMETER_VK_VkImageViewType,
    PARAMETER_VK_VkComponentSwizzle,
    PARAMETER_VK_VkVertexInputRate,
    PARAMETER_VK_VkPrimitiveTopology,
    PARAMETER_VK_VkPolygonMode,
    PARAMETER_VK_VkFrontFace,
    PARAMETER_VK_VkCompareOp,
    PARAMETER_VK_VkStencilOp,
    PARAMETER_VK_VkLogicOp,
    PARAMETER_VK_VkBlendFactor,
    PARAMETER_VK_VkBlendOp,
    PARAMETER_VK_VkDynamicState,
    PARAMETER_VK_VkFilter,
    PARAMETER_VK_VkSamplerMipmapMode,
    PARAMETER_VK_VkSamplerAddressMode,
    PARAMETER_VK_VkBorderColor,
    PARAMETER_VK_VkDescriptorType,
    PARAMETER_VK_VkAttachmentLoadOp,
    PARAMETER_VK_VkAttachmentStoreOp,
    PARAMETER_VK_VkPipelineBindPoint,
    PARAMETER_VK_VkCmdBufferLevel,
    PARAMETER_VK_VkIndexType,
    PARAMETER_VK_VkSubpassContents,
    PARAMETER_VK_VkColorSpaceKHR,
    PARAMETER_VK_VkPresentModeKHR,
    PARAMETER_VK_VkDebugReportObjectTypeEXT,
    PARAMETER_VK_VkDebugReportErrorEXT,
    PARAMETER_VK_VkFormatFeatureFlags,
    PARAMETER_VK_VkImageUsageFlags,
    PARAMETER_VK_VkImageCreateFlags,
    PARAMETER_VK_VkSampleCountFlags,
    PARAMETER_VK_VkQueueFlags,
    PARAMETER_VK_VkMemoryPropertyFlags,
    PARAMETER_VK_VkMemoryMapFlags,
    PARAMETER_VK_VkSparseImageFormatFlags,
    PARAMETER_VK_VkSparseMemoryBindFlags,
    PARAMETER_VK_VkFenceCreateFlags,
    PARAMETER_VK_VkQueryPipelineStatisticFlags,
    PARAMETER_VK_VkQueryResultFlags,
    PARAMETER_VK_VkBufferUsageFlags,
    PARAMETER_VK_VkBufferCreateFlags,
    PARAMETER_VK_VkImageAspectFlags,
    PARAMETER_VK_VkColorComponentFlags,
    PARAMETER_VK_VkDescriptorPoolCreateFlags,
    PARAMETER_VK_VkPipelineCreateFlags,
    PARAMETER_VK_VkShaderStageFlags,
    PARAMETER_VK_VkCullModeFlags,
    PARAMETER_VK_VkAttachmentDescriptionFlags,
    PARAMETER_VK_VkPipelineStageFlags,
    PARAMETER_VK_VkAccessFlags,
    PARAMETER_VK_VkDependencyFlags,
    PARAMETER_VK_VkCmdPoolCreateFlags,
    PARAMETER_VK_VkCmdPoolResetFlags,
    PARAMETER_VK_VkCmdBufferUsageFlags,
    PARAMETER_VK_VkStencilFaceFlags,
    PARAMETER_VK_VkQueryControlFlags,
    PARAMETER_VK_VkCommandBufferResetFlags,
    PARAMETER_VK_VkSurfaceTransformFlagBitsKHRFlags,
    PARAMETER_VK_VkCompositeAlphaFlags,
    PARAMETER_VK_VkDebugReportFlagsEXT,
};

//-----------------------------------------------------------------------------
/// Enum containing flags on how to display a return value. Default is decimal
/// Other formats may be needed later such as custom bitfields.
//-----------------------------------------------------------------------------
enum ReturnDisplayType
{
    RETURN_VALUE_DECIMAL,
    RETURN_VALUE_HEX,
};

//-----------------------------------------------------------------------------
/// The type and pointer for a parameter that needs to be stringified.
//-----------------------------------------------------------------------------
struct ParameterEntry
{
    int            mType; ///< Data type
    const void*    mData; ///< Data pointer
};

//-----------------------------------------------------------------------------
/// Amount of memory needed for each parameter. Take into account that some
/// parameters may be wide strings, so buffer length is double 
//-----------------------------------------------------------------------------
static const int BYTES_PER_PARAMETER = 512;

/// Data type for buffer length parameter (incase it needs to be changed later)
typedef uint16_t  bufferSize_t;

/// Actual amount of data in the buffer reserved for the actual parameter string
static const int BYTES_PER_PARAMETER_DATA = BYTES_PER_PARAMETER - sizeof(bufferSize_t) - sizeof(PARAMETER_TYPE);

//--------------------------------------------------------------------------
/// The APIEntry structure is used to track all calls that are traced at runtime.
//--------------------------------------------------------------------------
class APIEntry
{
public:
    //--------------------------------------------------------------------------
    /// Constructor used to initialize members of new CallData instances.
    /// \param inThreadId The thread Id that the call was invoked from.
    /// \param inFuncId The function ID of this API call
    /// \param inArguments A string containing the arguments of the invoked call.
    //--------------------------------------------------------------------------
    APIEntry(UINT inThreadId, FuncId inFuncId, const std::string& inArguments);

    //--------------------------------------------------------------------------
    /// Constructor used to initialize members of new CallData instances.
    /// \param inThreadId The thread Id that the call was invoked from.
    /// \param inFuncId The function ID of this API call
    /// \param inNumParameters The number of params for this api call
    //--------------------------------------------------------------------------
    APIEntry(UINT inThreadId, FuncId inFuncId, UINT32 inNumParameters);

    //--------------------------------------------------------------------------
    /// Virtual destructor since this is subclassed elsewhere.
    //--------------------------------------------------------------------------
    virtual ~APIEntry();

    //--------------------------------------------------------------------------
    /// Use API-specific methods to determine the API name for this entry.
    //--------------------------------------------------------------------------
    virtual const char* GetAPIName() const = 0;

    //--------------------------------------------------------------------------
    /// Append this APIEntry's information to the end of the API Trace response.
    /// \param out The API Trace response stream to append to.
    /// \param inStartTime The start time for the API call.
    /// \param inEndTime The end time for the API call.
    //--------------------------------------------------------------------------
    virtual void AppendAPITraceLine(gtASCIIString& out, double inStartTime, double inEndTime) const = 0;

    //--------------------------------------------------------------------------
    /// Check if this logged APIEntry is a Draw call.
    /// \returns True if the API is a draw call. False if it's not.
    //--------------------------------------------------------------------------
    virtual bool IsDrawCall() const = 0;

    //-----------------------------------------------------------------------------
    /// Convert an API parameter from raw data to a string for display
    /// \param paramType the data type of the parameter
    /// \param dataLength the number of bytes used to contain the data
    /// \param pRawData a pointer to the raw data
    /// \param ioParameterString a buffer passed in where the string is to be stored
    //-----------------------------------------------------------------------------
    virtual void GetParameterAsString(PARAMETER_TYPE paramType, UINT dataLength, const char* pRawData, char* ioParameterString) const = 0;

    //-----------------------------------------------------------------------------
    /// Convert a numeric return value into a human-readable string.
    /// \param inReturnValue A numeric return value. Could be an HRESULT, or a number.
    /// \param inDisplayType A flag indicating how the return value should be displayed
    /// \param ioReturnValue A string used to write the return value to
    //-----------------------------------------------------------------------------
    void PrintReturnValue(const INT64 inReturnValue, const ReturnDisplayType inDisplayType, gtASCIIString& ioReturnValue) const;

    //-----------------------------------------------------------------------------
    /// Get the API parameters as a single string. Build the string from the
    /// individual parameters if necessary
    /// \return parameter string
    //-----------------------------------------------------------------------------
    const char* GetParameterString(gtASCIIString& parameterString) const;

    //-----------------------------------------------------------------------------
    /// Track whether we got to gather GPU time for this call.
    /// \param gathered True if results were gathered, and false if not.
    //-----------------------------------------------------------------------------
    void SetGatheredGpuTime(bool gathered) { m_gatheredGpuTime = gathered; }

    //-----------------------------------------------------------------------------
    /// Return whether we got to gather GPU time for this call.
    /// \return True if results were gathered, and false if not.
    //-----------------------------------------------------------------------------
    bool GatheredGpuTime() { return m_gatheredGpuTime; }

    //--------------------------------------------------------------------------
    /// The ID of the thread that the call was invoked/logged in.
    //--------------------------------------------------------------------------
    UINT mThreadId;

    //--------------------------------------------------------------------------
    /// A string with the call parameters. Each API call is printed with its own format string.
    //--------------------------------------------------------------------------
    gtASCIIString mParameters;

    //--------------------------------------------------------------------------
    /// An instance of the FuncId enum. Will be converted to a string before seen in the client.
    //--------------------------------------------------------------------------
    FuncId mFunctionId;

protected:
    /// The number of parameters this API call takes
    UINT32 mNumParameters;

    /// Buffer used to store raw parameter data for formatting later
    char* mParameterBuffer;

private:
    APIEntry() {}

    /// Track whether we got to gather GPU time for this call
    bool m_gatheredGpuTime;
};

#endif // APIENTRY_H
