//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktUtil.cpp
/// \brief  Implementation file for VktUtil namespace.
///         This contains various utility functions/macros/includes used
///         throughout the Vulkan server.
//==============================================================================

#include "vktUtil.h"

#include "../vktDefines.h"
#include "../../../Common/OSWrappers.h"

//-----------------------------------------------------------------------------
/// Construct a measurement info structure for each call that will be profiled.
/// \param inFuncId The function ID for the function being profiled.
/// \param sampleId The SampleID associated with the profiled command.
/// \param pWrappedCmdBuf The command buffer that executed the profiled command.
/// \param frameNum The frame number.
/// \param fillId An ID used to track how many times a cmdBuf was filled in.
/// \param measurementId A ProfilerMeasurementId containing metadata for the new measurement.
//-----------------------------------------------------------------------------
void VktUtil::ConstructMeasurementInfo(FuncId inFuncId, UINT64 sampleId, VktWrappedCmdBuf* pWrappedCmdBuf, UINT frameNum, UINT64 fillId, ProfilerMeasurementId& measurementId)
{
    measurementId.pWrappedCmdBuf = pWrappedCmdBuf;
    measurementId.sampleId       = sampleId;
    measurementId.frame          = frameNum;
    measurementId.funcId         = inFuncId;
    measurementId.fillId         = fillId;
}

//-----------------------------------------------------------------------------
/// Decompose the incoming packed flags into a pipe-separated string of enum strings.
/// \param flags A UINT instance where packed flags have been bitwise-OR'd into the variable.
/// \param ioFlagsString The string that the result will be packed into.
/// \param inWriteHook A pointer to the function responsible for writing the given enumeration into a string.
/// \param inMinFlag An enumeration member that controls which value to start decomposing flags from.
/// \param inMaxFlag An enumeration member that controls which value to stop decomposing flags from.
//-----------------------------------------------------------------------------
void VktUtil::DecomposeFlags(uint32 flags, gtASCIIString& ioFlagsString, WriteEnum_Hook inWriteHook, uint32 inMinFlag, uint32 inMaxFlag)
{
    if (flags == 0)
    {
        ioFlagsString = "0";
    }
    else
    {
        // Initialize the flag to the minimum enum value.
        uint32 currentFlag = inMinFlag;
        bool bFlagWritten = false;

        do
        {
            // If there's an overlap between the input flags and the current flag bit, append it to the output string.
            if ((currentFlag & flags) != 0)
            {
                // Append a spacer between the new and previous flag string (but only if this isn't the first flag).
                if ((currentFlag != inMinFlag) && bFlagWritten)
                {
                    ioFlagsString.append(" | ");
                }

                bFlagWritten = true;
                ioFlagsString.appendFormattedString("%s", inWriteHook(currentFlag));
            }

            // If the min flag is zero, we're going to loop forever. Increment to "1" to check the next bit, and we'll be able to shift to check the remaining flags.
            if (currentFlag == 0)
            {
                currentFlag = 1;
            }
            else
            {
                // Shift the current flag to the left to check the for the presence of the next flag.
                currentFlag = (currentFlag << 1);
            }

        }
        while (currentFlag <= inMaxFlag);
    }
}

//-----------------------------------------------------------------------------
/// Determine what the name of this Vulkan server is.
/// \return The name of this backend/DLL.
//-----------------------------------------------------------------------------
std::string VktUtil::GetLayerName()
{
#ifdef _LINUX

#ifdef CODEXL_GRAPHICS
    std::string layerNameA = "libCXLGraphicsServerVulkan";
#else
    std::string layerNameA = "VulkanServer";
#endif

#else

#ifdef CODEXL_GRAPHICS
    std::string layerNameA = "CXLGraphicsServerVulkan";
#else
    std::string layerNameA = "VulkanServer";
#endif

    char appPath[PS_MAX_PATH] = {};
    GetModuleFileName(nullptr, appPath, PS_MAX_PATH);

    osModuleArchitecture appBinaryType = OS_X86_64_ARCHITECTURE;

    const bool readBinType = OSWrappers::GetBinaryType(appPath, &appBinaryType);

    if (readBinType)
    {
        if (appBinaryType == OS_I386_ARCHITECTURE)
        {
            layerNameA.append(GDT_DEBUG_SUFFIX);
        }
        else if (appBinaryType == OS_X86_64_ARCHITECTURE)
        {
            layerNameA.append("-x64");
            layerNameA.append(GDT_DEBUG_SUFFIX);
        }
    }

#endif

    return layerNameA;
}

const char* VktUtil::WritePipelineCacheHeaderVersionEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_PIPELINE_CACHE_HEADER_VERSION_ONE, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteResultCodeEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_SUCCESS, pResultString);
        PRINTENUMCASE(VK_NOT_READY, pResultString);
        PRINTENUMCASE(VK_TIMEOUT, pResultString);
        PRINTENUMCASE(VK_EVENT_SET, pResultString);
        PRINTENUMCASE(VK_EVENT_RESET, pResultString);
        PRINTENUMCASE(VK_INCOMPLETE, pResultString);
        PRINTENUMCASE(VK_ERROR_OUT_OF_HOST_MEMORY, pResultString);
        PRINTENUMCASE(VK_ERROR_OUT_OF_DEVICE_MEMORY, pResultString);
        PRINTENUMCASE(VK_ERROR_INITIALIZATION_FAILED, pResultString);
        PRINTENUMCASE(VK_ERROR_DEVICE_LOST, pResultString);
        PRINTENUMCASE(VK_ERROR_MEMORY_MAP_FAILED, pResultString);
        PRINTENUMCASE(VK_ERROR_LAYER_NOT_PRESENT, pResultString);
        PRINTENUMCASE(VK_ERROR_EXTENSION_NOT_PRESENT, pResultString);
        PRINTENUMCASE(VK_ERROR_FEATURE_NOT_PRESENT, pResultString);
        PRINTENUMCASE(VK_ERROR_INCOMPATIBLE_DRIVER, pResultString);
        PRINTENUMCASE(VK_ERROR_TOO_MANY_OBJECTS, pResultString);
        PRINTENUMCASE(VK_ERROR_FORMAT_NOT_SUPPORTED, pResultString);
        PRINTENUMCASE(VK_ERROR_SURFACE_LOST_KHR, pResultString);
        PRINTENUMCASE(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR, pResultString);
        PRINTENUMCASE(VK_SUBOPTIMAL_KHR, pResultString);
        PRINTENUMCASE(VK_ERROR_OUT_OF_DATE_KHR, pResultString);
        PRINTENUMCASE(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR, pResultString);
        PRINTENUMCASE(VK_ERROR_VALIDATION_FAILED_EXT, pResultString);
        PRINTENUMCASE(VK_ERROR_INVALID_SHADER_NV, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteStructureTypeEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_STRUCTURE_TYPE_APPLICATION_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_SUBMIT_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_BIND_SPARSE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_EVENT_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_MEMORY_BARRIER, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_MIR_SURFACE_CREATE_INFO_KHR, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_IMAGE_CREATE_INFO_NV, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_BUFFER_CREATE_INFO_NV, pResultString);
        PRINTENUMCASE(VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteSystemAllocationScopeEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_SYSTEM_ALLOCATION_SCOPE_COMMAND, pResultString);
        PRINTENUMCASE(VK_SYSTEM_ALLOCATION_SCOPE_OBJECT, pResultString);
        PRINTENUMCASE(VK_SYSTEM_ALLOCATION_SCOPE_CACHE, pResultString);
        PRINTENUMCASE(VK_SYSTEM_ALLOCATION_SCOPE_DEVICE, pResultString);
        PRINTENUMCASE(VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteInternalAllocationTypeEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_INTERNAL_ALLOCATION_TYPE_EXECUTABLE, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteFormatEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_FORMAT_UNDEFINED, pResultString);
        PRINTENUMCASE(VK_FORMAT_R4G4_UNORM_PACK8, pResultString);
        PRINTENUMCASE(VK_FORMAT_R4G4B4A4_UNORM_PACK16, pResultString);
        PRINTENUMCASE(VK_FORMAT_B4G4R4A4_UNORM_PACK16, pResultString);
        PRINTENUMCASE(VK_FORMAT_R5G6B5_UNORM_PACK16, pResultString);
        PRINTENUMCASE(VK_FORMAT_B5G6R5_UNORM_PACK16, pResultString);
        PRINTENUMCASE(VK_FORMAT_R5G5B5A1_UNORM_PACK16, pResultString);
        PRINTENUMCASE(VK_FORMAT_B5G5R5A1_UNORM_PACK16, pResultString);
        PRINTENUMCASE(VK_FORMAT_A1R5G5B5_UNORM_PACK16, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8_UNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8_SNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8_USCALED, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8_SSCALED, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8_SINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8_SRGB, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8_UNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8_SNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8_USCALED, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8_SSCALED, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8_SINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8_SRGB, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8B8_UNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8B8_SNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8B8_USCALED, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8B8_SSCALED, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8B8_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8B8_SINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8B8_SRGB, pResultString);
        PRINTENUMCASE(VK_FORMAT_B8G8R8_UNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_B8G8R8_SNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_B8G8R8_USCALED, pResultString);
        PRINTENUMCASE(VK_FORMAT_B8G8R8_SSCALED, pResultString);
        PRINTENUMCASE(VK_FORMAT_B8G8R8_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_B8G8R8_SINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_B8G8R8_SRGB, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8B8A8_UNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8B8A8_SNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8B8A8_USCALED, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8B8A8_SSCALED, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8B8A8_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8B8A8_SINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R8G8B8A8_SRGB, pResultString);
        PRINTENUMCASE(VK_FORMAT_B8G8R8A8_UNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_B8G8R8A8_SNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_B8G8R8A8_USCALED, pResultString);
        PRINTENUMCASE(VK_FORMAT_B8G8R8A8_SSCALED, pResultString);
        PRINTENUMCASE(VK_FORMAT_B8G8R8A8_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_B8G8R8A8_SINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_B8G8R8A8_SRGB, pResultString);
        PRINTENUMCASE(VK_FORMAT_A8B8G8R8_UNORM_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_A8B8G8R8_SNORM_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_A8B8G8R8_USCALED_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_A8B8G8R8_SSCALED_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_A8B8G8R8_UINT_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_A8B8G8R8_SINT_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_A8B8G8R8_SRGB_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_A2R10G10B10_UNORM_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_A2R10G10B10_SNORM_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_A2R10G10B10_USCALED_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_A2R10G10B10_SSCALED_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_A2R10G10B10_UINT_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_A2R10G10B10_SINT_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_A2B10G10R10_UNORM_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_A2B10G10R10_SNORM_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_A2B10G10R10_USCALED_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_A2B10G10R10_SSCALED_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_A2B10G10R10_UINT_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_A2B10G10R10_SINT_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16_UNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16_SNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16_USCALED, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16_SSCALED, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16_SINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16_SFLOAT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16_UNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16_SNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16_USCALED, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16_SSCALED, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16_SINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16_SFLOAT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16B16_UNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16B16_SNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16B16_USCALED, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16B16_SSCALED, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16B16_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16B16_SINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16B16_SFLOAT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16B16A16_UNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16B16A16_SNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16B16A16_USCALED, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16B16A16_SSCALED, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16B16A16_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16B16A16_SINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R16G16B16A16_SFLOAT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R32_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R32_SINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R32_SFLOAT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R32G32_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R32G32_SINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R32G32_SFLOAT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R32G32B32_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R32G32B32_SINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R32G32B32_SFLOAT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R32G32B32A32_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R32G32B32A32_SINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R32G32B32A32_SFLOAT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R64_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R64_SINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R64_SFLOAT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R64G64_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R64G64_SINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R64G64_SFLOAT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R64G64B64_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R64G64B64_SINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R64G64B64_SFLOAT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R64G64B64A64_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R64G64B64A64_SINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_R64G64B64A64_SFLOAT, pResultString);
        PRINTENUMCASE(VK_FORMAT_B10G11R11_UFLOAT_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_D16_UNORM, pResultString);
        PRINTENUMCASE(VK_FORMAT_X8_D24_UNORM_PACK32, pResultString);
        PRINTENUMCASE(VK_FORMAT_D32_SFLOAT, pResultString);
        PRINTENUMCASE(VK_FORMAT_S8_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_D16_UNORM_S8_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_D24_UNORM_S8_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_D32_SFLOAT_S8_UINT, pResultString);
        PRINTENUMCASE(VK_FORMAT_BC1_RGB_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_BC1_RGB_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_BC1_RGBA_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_BC1_RGBA_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_BC2_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_BC2_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_BC3_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_BC3_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_BC4_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_BC4_SNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_BC5_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_BC5_SNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_BC6H_UFLOAT_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_BC6H_SFLOAT_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_BC7_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_BC7_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_EAC_R11_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_EAC_R11_SNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_EAC_R11G11_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_EAC_R11G11_SNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_4x4_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_4x4_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_5x4_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_5x4_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_5x5_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_5x5_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_6x5_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_6x5_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_6x6_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_6x6_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_8x5_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_8x5_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_8x6_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_8x6_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_8x8_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_8x8_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_10x5_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_10x5_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_10x6_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_10x6_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_10x8_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_10x8_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_10x10_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_10x10_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_12x10_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_12x10_SRGB_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_12x12_UNORM_BLOCK, pResultString);
        PRINTENUMCASE(VK_FORMAT_ASTC_12x12_SRGB_BLOCK, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteImageTypeEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_IMAGE_TYPE_1D, pResultString);
        PRINTENUMCASE(VK_IMAGE_TYPE_2D, pResultString);
        PRINTENUMCASE(VK_IMAGE_TYPE_3D, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteImageTilingEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_IMAGE_TILING_LINEAR, pResultString);
        PRINTENUMCASE(VK_IMAGE_TILING_OPTIMAL, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WritePhysicalDeviceTypeEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_PHYSICAL_DEVICE_TYPE_OTHER, pResultString);
        PRINTENUMCASE(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, pResultString);
        PRINTENUMCASE(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, pResultString);
        PRINTENUMCASE(VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU, pResultString);
        PRINTENUMCASE(VK_PHYSICAL_DEVICE_TYPE_CPU, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteQueryTypeEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_QUERY_TYPE_OCCLUSION, pResultString);
        PRINTENUMCASE(VK_QUERY_TYPE_PIPELINE_STATISTICS, pResultString);
        PRINTENUMCASE(VK_QUERY_TYPE_TIMESTAMP, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteSharingModeEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_SHARING_MODE_EXCLUSIVE, pResultString);
        PRINTENUMCASE(VK_SHARING_MODE_CONCURRENT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteImageLayoutEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_IMAGE_LAYOUT_UNDEFINED, pResultString);
        PRINTENUMCASE(VK_IMAGE_LAYOUT_GENERAL, pResultString);
        PRINTENUMCASE(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, pResultString);
        PRINTENUMCASE(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, pResultString);
        PRINTENUMCASE(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, pResultString);
        PRINTENUMCASE(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, pResultString);
        PRINTENUMCASE(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pResultString);
        PRINTENUMCASE(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, pResultString);
        PRINTENUMCASE(VK_IMAGE_LAYOUT_PREINITIALIZED, pResultString);
        PRINTENUMCASE(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteImageViewTypeEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_IMAGE_VIEW_TYPE_1D, pResultString);
        PRINTENUMCASE(VK_IMAGE_VIEW_TYPE_2D, pResultString);
        PRINTENUMCASE(VK_IMAGE_VIEW_TYPE_3D, pResultString);
        PRINTENUMCASE(VK_IMAGE_VIEW_TYPE_CUBE, pResultString);
        PRINTENUMCASE(VK_IMAGE_VIEW_TYPE_1D_ARRAY, pResultString);
        PRINTENUMCASE(VK_IMAGE_VIEW_TYPE_2D_ARRAY, pResultString);
        PRINTENUMCASE(VK_IMAGE_VIEW_TYPE_CUBE_ARRAY, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteComponentSwizzleEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_COMPONENT_SWIZZLE_IDENTITY, pResultString);
        PRINTENUMCASE(VK_COMPONENT_SWIZZLE_ZERO, pResultString);
        PRINTENUMCASE(VK_COMPONENT_SWIZZLE_ONE, pResultString);
        PRINTENUMCASE(VK_COMPONENT_SWIZZLE_R, pResultString);
        PRINTENUMCASE(VK_COMPONENT_SWIZZLE_G, pResultString);
        PRINTENUMCASE(VK_COMPONENT_SWIZZLE_B, pResultString);
        PRINTENUMCASE(VK_COMPONENT_SWIZZLE_A, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteVertexInputRateEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_VERTEX_INPUT_RATE_VERTEX, pResultString);
        PRINTENUMCASE(VK_VERTEX_INPUT_RATE_INSTANCE, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WritePrimitiveTopologyEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_PRIMITIVE_TOPOLOGY_POINT_LIST, pResultString);
        PRINTENUMCASE(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, pResultString);
        PRINTENUMCASE(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, pResultString);
        PRINTENUMCASE(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, pResultString);
        PRINTENUMCASE(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, pResultString);
        PRINTENUMCASE(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, pResultString);
        PRINTENUMCASE(VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY, pResultString);
        PRINTENUMCASE(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY, pResultString);
        PRINTENUMCASE(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY, pResultString);
        PRINTENUMCASE(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY, pResultString);
        PRINTENUMCASE(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WritePolygonModeEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_POLYGON_MODE_FILL, pResultString);
        PRINTENUMCASE(VK_POLYGON_MODE_LINE, pResultString);
        PRINTENUMCASE(VK_POLYGON_MODE_POINT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteFrontFaceEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_FRONT_FACE_COUNTER_CLOCKWISE, pResultString);
        PRINTENUMCASE(VK_FRONT_FACE_CLOCKWISE, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteCompareOpEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_COMPARE_OP_NEVER, pResultString);
        PRINTENUMCASE(VK_COMPARE_OP_LESS, pResultString);
        PRINTENUMCASE(VK_COMPARE_OP_EQUAL, pResultString);
        PRINTENUMCASE(VK_COMPARE_OP_LESS_OR_EQUAL, pResultString);
        PRINTENUMCASE(VK_COMPARE_OP_GREATER, pResultString);
        PRINTENUMCASE(VK_COMPARE_OP_NOT_EQUAL, pResultString);
        PRINTENUMCASE(VK_COMPARE_OP_GREATER_OR_EQUAL, pResultString);
        PRINTENUMCASE(VK_COMPARE_OP_ALWAYS, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteStencilOpEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_STENCIL_OP_KEEP, pResultString);
        PRINTENUMCASE(VK_STENCIL_OP_ZERO, pResultString);
        PRINTENUMCASE(VK_STENCIL_OP_REPLACE, pResultString);
        PRINTENUMCASE(VK_STENCIL_OP_INCREMENT_AND_CLAMP, pResultString);
        PRINTENUMCASE(VK_STENCIL_OP_DECREMENT_AND_CLAMP, pResultString);
        PRINTENUMCASE(VK_STENCIL_OP_INVERT, pResultString);
        PRINTENUMCASE(VK_STENCIL_OP_INCREMENT_AND_WRAP, pResultString);
        PRINTENUMCASE(VK_STENCIL_OP_DECREMENT_AND_WRAP, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteLogicOpEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_LOGIC_OP_CLEAR, pResultString);
        PRINTENUMCASE(VK_LOGIC_OP_AND, pResultString);
        PRINTENUMCASE(VK_LOGIC_OP_AND_REVERSE, pResultString);
        PRINTENUMCASE(VK_LOGIC_OP_COPY, pResultString);
        PRINTENUMCASE(VK_LOGIC_OP_AND_INVERTED, pResultString);
        PRINTENUMCASE(VK_LOGIC_OP_NO_OP, pResultString);
        PRINTENUMCASE(VK_LOGIC_OP_XOR, pResultString);
        PRINTENUMCASE(VK_LOGIC_OP_OR, pResultString);
        PRINTENUMCASE(VK_LOGIC_OP_NOR, pResultString);
        PRINTENUMCASE(VK_LOGIC_OP_EQUIVALENT, pResultString);
        PRINTENUMCASE(VK_LOGIC_OP_INVERT, pResultString);
        PRINTENUMCASE(VK_LOGIC_OP_OR_REVERSE, pResultString);
        PRINTENUMCASE(VK_LOGIC_OP_COPY_INVERTED, pResultString);
        PRINTENUMCASE(VK_LOGIC_OP_OR_INVERTED, pResultString);
        PRINTENUMCASE(VK_LOGIC_OP_NAND, pResultString);
        PRINTENUMCASE(VK_LOGIC_OP_SET, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteBlendFactorEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_BLEND_FACTOR_ZERO, pResultString);
        PRINTENUMCASE(VK_BLEND_FACTOR_ONE, pResultString);
        PRINTENUMCASE(VK_BLEND_FACTOR_SRC_COLOR, pResultString);
        PRINTENUMCASE(VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR, pResultString);
        PRINTENUMCASE(VK_BLEND_FACTOR_DST_COLOR, pResultString);
        PRINTENUMCASE(VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR, pResultString);
        PRINTENUMCASE(VK_BLEND_FACTOR_SRC_ALPHA, pResultString);
        PRINTENUMCASE(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, pResultString);
        PRINTENUMCASE(VK_BLEND_FACTOR_DST_ALPHA, pResultString);
        PRINTENUMCASE(VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA, pResultString);
        PRINTENUMCASE(VK_BLEND_FACTOR_CONSTANT_COLOR, pResultString);
        PRINTENUMCASE(VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR, pResultString);
        PRINTENUMCASE(VK_BLEND_FACTOR_CONSTANT_ALPHA, pResultString);
        PRINTENUMCASE(VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA, pResultString);
        PRINTENUMCASE(VK_BLEND_FACTOR_SRC_ALPHA_SATURATE, pResultString);
        PRINTENUMCASE(VK_BLEND_FACTOR_SRC1_COLOR, pResultString);
        PRINTENUMCASE(VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR, pResultString);
        PRINTENUMCASE(VK_BLEND_FACTOR_SRC1_ALPHA, pResultString);
        PRINTENUMCASE(VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteBlendOpEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_BLEND_OP_ADD, pResultString);
        PRINTENUMCASE(VK_BLEND_OP_SUBTRACT, pResultString);
        PRINTENUMCASE(VK_BLEND_OP_REVERSE_SUBTRACT, pResultString);
        PRINTENUMCASE(VK_BLEND_OP_MIN, pResultString);
        PRINTENUMCASE(VK_BLEND_OP_MAX, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteDynamicStateEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_DYNAMIC_STATE_VIEWPORT, pResultString);
        PRINTENUMCASE(VK_DYNAMIC_STATE_SCISSOR, pResultString);
        PRINTENUMCASE(VK_DYNAMIC_STATE_LINE_WIDTH, pResultString);
        PRINTENUMCASE(VK_DYNAMIC_STATE_DEPTH_BIAS, pResultString);
        PRINTENUMCASE(VK_DYNAMIC_STATE_BLEND_CONSTANTS, pResultString);
        PRINTENUMCASE(VK_DYNAMIC_STATE_DEPTH_BOUNDS, pResultString);
        PRINTENUMCASE(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK, pResultString);
        PRINTENUMCASE(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK, pResultString);
        PRINTENUMCASE(VK_DYNAMIC_STATE_STENCIL_REFERENCE, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteFilterEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_FILTER_NEAREST, pResultString);
        PRINTENUMCASE(VK_FILTER_LINEAR, pResultString);
        PRINTENUMCASE(VK_FILTER_CUBIC_IMG, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteSamplerMipmapModeEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_SAMPLER_MIPMAP_MODE_NEAREST, pResultString);
        PRINTENUMCASE(VK_SAMPLER_MIPMAP_MODE_LINEAR, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteSamplerAddressModeEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_SAMPLER_ADDRESS_MODE_REPEAT, pResultString);
        PRINTENUMCASE(VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, pResultString);
        PRINTENUMCASE(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, pResultString);
        PRINTENUMCASE(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, pResultString);
        PRINTENUMCASE(VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteBorderColorEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK, pResultString);
        PRINTENUMCASE(VK_BORDER_COLOR_INT_TRANSPARENT_BLACK, pResultString);
        PRINTENUMCASE(VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK, pResultString);
        PRINTENUMCASE(VK_BORDER_COLOR_INT_OPAQUE_BLACK, pResultString);
        PRINTENUMCASE(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE, pResultString);
        PRINTENUMCASE(VK_BORDER_COLOR_INT_OPAQUE_WHITE, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteDescriptorTypeEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, pResultString);
        PRINTENUMCASE(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, pResultString);
        PRINTENUMCASE(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, pResultString);
        PRINTENUMCASE(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, pResultString);
        PRINTENUMCASE(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, pResultString);
        PRINTENUMCASE(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, pResultString);
        PRINTENUMCASE(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, pResultString);
        PRINTENUMCASE(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, pResultString);
        PRINTENUMCASE(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, pResultString);
        PRINTENUMCASE(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteAttachmentLoadOpEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_ATTACHMENT_LOAD_OP_LOAD, pResultString);
        PRINTENUMCASE(VK_ATTACHMENT_LOAD_OP_CLEAR, pResultString);
        PRINTENUMCASE(VK_ATTACHMENT_LOAD_OP_DONT_CARE, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteAttachmentStoreOpEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_ATTACHMENT_STORE_OP_STORE, pResultString);
        PRINTENUMCASE(VK_ATTACHMENT_STORE_OP_DONT_CARE, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WritePipelineBindPointEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_PIPELINE_BIND_POINT_COMPUTE, pResultString);
        PRINTENUMCASE(VK_PIPELINE_BIND_POINT_GRAPHICS, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteCmdBufferLevelEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_COMMAND_BUFFER_LEVEL_PRIMARY, pResultString);
        PRINTENUMCASE(VK_COMMAND_BUFFER_LEVEL_SECONDARY, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteIndexTypeEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_INDEX_TYPE_UINT16, pResultString);
        PRINTENUMCASE(VK_INDEX_TYPE_UINT32, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteSubpassContentsEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_SUBPASS_CONTENTS_INLINE, pResultString);
        PRINTENUMCASE(VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteColorSpaceKHREnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WritePresentModeKHREnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_PRESENT_MODE_IMMEDIATE_KHR, pResultString);
        PRINTENUMCASE(VK_PRESENT_MODE_MAILBOX_KHR, pResultString);
        PRINTENUMCASE(VK_PRESENT_MODE_FIFO_KHR, pResultString);
        PRINTENUMCASE(VK_PRESENT_MODE_FIFO_RELAXED_KHR, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteDebugReportObjectTypeEXTEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_EXT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* VktUtil::WriteDebugReportErrorEXTEnumAsString(int enumVal)
{
    const char* pResultString = nullptr;

    switch (enumVal)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_DEBUG_REPORT_ERROR_NONE_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_ERROR_CALLBACK_REF_EXT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

const char* WriteFormatFeatureFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT, pResultString);
        PRINTENUMCASE(VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT, pResultString);
        PRINTENUMCASE(VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT, pResultString);
        PRINTENUMCASE(VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT, pResultString);
        PRINTENUMCASE(VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT, pResultString);
        PRINTENUMCASE(VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT, pResultString);
        PRINTENUMCASE(VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT, pResultString);
        PRINTENUMCASE(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT, pResultString);
        PRINTENUMCASE(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT, pResultString);
        PRINTENUMCASE(VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, pResultString);
        PRINTENUMCASE(VK_FORMAT_FEATURE_BLIT_SRC_BIT, pResultString);
        PRINTENUMCASE(VK_FORMAT_FEATURE_BLIT_DST_BIT, pResultString);
        PRINTENUMCASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT, pResultString);
        PRINTENUMCASE(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeFormatFeatureFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteFormatFeatureFlagsEnumAsString, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT, VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);
    return flagsString.asCharArray();
}

const char* WriteImageUsageFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_IMAGE_USAGE_TRANSFER_SRC_BIT, pResultString);
        PRINTENUMCASE(VK_IMAGE_USAGE_TRANSFER_DST_BIT, pResultString);
        PRINTENUMCASE(VK_IMAGE_USAGE_SAMPLED_BIT, pResultString);
        PRINTENUMCASE(VK_IMAGE_USAGE_STORAGE_BIT, pResultString);
        PRINTENUMCASE(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, pResultString);
        PRINTENUMCASE(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, pResultString);
        PRINTENUMCASE(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, pResultString);
        PRINTENUMCASE(VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeImageUsageFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteImageUsageFlagsEnumAsString, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    return flagsString.asCharArray();
}

const char* WriteImageCreateFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_IMAGE_CREATE_SPARSE_BINDING_BIT, pResultString);
        PRINTENUMCASE(VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT, pResultString);
        PRINTENUMCASE(VK_IMAGE_CREATE_SPARSE_ALIASED_BIT, pResultString);
        PRINTENUMCASE(VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, pResultString);
        PRINTENUMCASE(VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeImageCreateFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteImageCreateFlagsEnumAsString, VK_IMAGE_CREATE_SPARSE_BINDING_BIT, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
    return flagsString.asCharArray();
}

const char* WriteSampleCountFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_SAMPLE_COUNT_1_BIT, pResultString);
        PRINTENUMCASE(VK_SAMPLE_COUNT_2_BIT, pResultString);
        PRINTENUMCASE(VK_SAMPLE_COUNT_4_BIT, pResultString);
        PRINTENUMCASE(VK_SAMPLE_COUNT_8_BIT, pResultString);
        PRINTENUMCASE(VK_SAMPLE_COUNT_16_BIT, pResultString);
        PRINTENUMCASE(VK_SAMPLE_COUNT_32_BIT, pResultString);
        PRINTENUMCASE(VK_SAMPLE_COUNT_64_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeSampleCountFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteSampleCountFlagsEnumAsString, VK_SAMPLE_COUNT_1_BIT, VK_SAMPLE_COUNT_64_BIT);
    return flagsString.asCharArray();
}

const char* WriteQueueFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_QUEUE_GRAPHICS_BIT, pResultString);
        PRINTENUMCASE(VK_QUEUE_COMPUTE_BIT, pResultString);
        PRINTENUMCASE(VK_QUEUE_TRANSFER_BIT, pResultString);
        PRINTENUMCASE(VK_QUEUE_SPARSE_BINDING_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeQueueFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteQueueFlagsEnumAsString, VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_SPARSE_BINDING_BIT);
    return flagsString.asCharArray();
}

const char* WriteMemoryPropertyFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pResultString);
        PRINTENUMCASE(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, pResultString);
        PRINTENUMCASE(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, pResultString);
        PRINTENUMCASE(VK_MEMORY_PROPERTY_HOST_CACHED_BIT, pResultString);
        PRINTENUMCASE(VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeMemoryPropertyFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteMemoryPropertyFlagsEnumAsString, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
    return flagsString.asCharArray();
}

const char* WriteMemoryHeapFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_MEMORY_HEAP_DEVICE_LOCAL_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeMemoryMapFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteMemoryHeapFlagsEnumAsString, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
    return flagsString.asCharArray();
}

const char* WriteImageAspectFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_IMAGE_ASPECT_COLOR_BIT, pResultString);
        PRINTENUMCASE(VK_IMAGE_ASPECT_DEPTH_BIT, pResultString);
        PRINTENUMCASE(VK_IMAGE_ASPECT_STENCIL_BIT, pResultString);
        PRINTENUMCASE(VK_IMAGE_ASPECT_METADATA_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeImageAspectFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteImageAspectFlagsEnumAsString, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_ASPECT_METADATA_BIT);
    return flagsString.asCharArray();
}

const char* WriteSparseImageFormatFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT, pResultString);
        PRINTENUMCASE(VK_SPARSE_IMAGE_FORMAT_ALIGNED_MIP_SIZE_BIT, pResultString);
        PRINTENUMCASE(VK_SPARSE_IMAGE_FORMAT_NONSTANDARD_BLOCK_SIZE_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeSparseImageFormatFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteSparseImageFormatFlagsEnumAsString, VK_SPARSE_IMAGE_FORMAT_SINGLE_MIPTAIL_BIT, VK_SPARSE_IMAGE_FORMAT_NONSTANDARD_BLOCK_SIZE_BIT);
    return flagsString.asCharArray();
}

const char* WriteSparseMemoryBindFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_SPARSE_MEMORY_BIND_METADATA_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeSparseMemoryBindFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteSparseMemoryBindFlagsEnumAsString, VK_SPARSE_MEMORY_BIND_METADATA_BIT, VK_SPARSE_MEMORY_BIND_METADATA_BIT);
    return flagsString.asCharArray();
}

const char* WriteFenceCreateFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_FENCE_CREATE_SIGNALED_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeFenceCreateFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteFenceCreateFlagsEnumAsString, VK_FENCE_CREATE_SIGNALED_BIT, VK_FENCE_CREATE_SIGNALED_BIT);
    return flagsString.asCharArray();
}

const char* WriteQueryPipelineStatisticFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT, pResultString);
        PRINTENUMCASE(VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT, pResultString);
        PRINTENUMCASE(VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT, pResultString);
        PRINTENUMCASE(VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT, pResultString);
        PRINTENUMCASE(VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT, pResultString);
        PRINTENUMCASE(VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT, pResultString);
        PRINTENUMCASE(VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT, pResultString);
        PRINTENUMCASE(VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT, pResultString);
        PRINTENUMCASE(VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT, pResultString);
        PRINTENUMCASE(VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT, pResultString);
        PRINTENUMCASE(VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeQueryPipelineStatisticFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteQueryPipelineStatisticFlagsEnumAsString, VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT, VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT);
    return flagsString.asCharArray();
}

const char* WriteQueryResultFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_QUERY_RESULT_64_BIT, pResultString);
        PRINTENUMCASE(VK_QUERY_RESULT_WAIT_BIT, pResultString);
        PRINTENUMCASE(VK_QUERY_RESULT_WITH_AVAILABILITY_BIT, pResultString);
        PRINTENUMCASE(VK_QUERY_RESULT_PARTIAL_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeQueryResultFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteQueryResultFlagsEnumAsString, VK_QUERY_RESULT_64_BIT, VK_QUERY_RESULT_PARTIAL_BIT);
    return flagsString.asCharArray();
}

const char* WriteBufferCreateFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_BUFFER_CREATE_SPARSE_BINDING_BIT, pResultString);
        PRINTENUMCASE(VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT, pResultString);
        PRINTENUMCASE(VK_BUFFER_CREATE_SPARSE_ALIASED_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeBufferCreateFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteBufferCreateFlagsEnumAsString, VK_BUFFER_CREATE_SPARSE_BINDING_BIT, VK_SPARSE_IMAGE_FORMAT_NONSTANDARD_BLOCK_SIZE_BIT);
    return flagsString.asCharArray();
}

const char* WriteBufferUsageFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, pResultString);
        PRINTENUMCASE(VK_BUFFER_USAGE_TRANSFER_DST_BIT, pResultString);
        PRINTENUMCASE(VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, pResultString);
        PRINTENUMCASE(VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, pResultString);
        PRINTENUMCASE(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, pResultString);
        PRINTENUMCASE(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, pResultString);
        PRINTENUMCASE(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, pResultString);
        PRINTENUMCASE(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, pResultString);
        PRINTENUMCASE(VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeBufferUsageFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteBufferUsageFlagsEnumAsString, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
    return flagsString.asCharArray();
}

const char* WritePipelineCreateFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT, pResultString);
        PRINTENUMCASE(VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT, pResultString);
        PRINTENUMCASE(VK_PIPELINE_CREATE_DERIVATIVE_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposePipelineCreateFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WritePipelineCreateFlagsEnumAsString, VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT, VK_PIPELINE_CREATE_DERIVATIVE_BIT);
    return flagsString.asCharArray();
}

const char* WriteShaderStageFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_SHADER_STAGE_VERTEX_BIT, pResultString);
        PRINTENUMCASE(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, pResultString);
        PRINTENUMCASE(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, pResultString);
        PRINTENUMCASE(VK_SHADER_STAGE_GEOMETRY_BIT, pResultString);
        PRINTENUMCASE(VK_SHADER_STAGE_FRAGMENT_BIT, pResultString);
        PRINTENUMCASE(VK_SHADER_STAGE_COMPUTE_BIT, pResultString);
        PRINTENUMCASE(VK_SHADER_STAGE_ALL_GRAPHICS, pResultString);
        PRINTENUMCASE(VK_SHADER_STAGE_ALL, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeShaderStageFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteShaderStageFlagsEnumAsString, VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_ALL);
    return flagsString.asCharArray();
}

const char* WriteCullModeFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_CULL_MODE_NONE, pResultString);
        PRINTENUMCASE(VK_CULL_MODE_FRONT_BIT, pResultString);
        PRINTENUMCASE(VK_CULL_MODE_BACK_BIT, pResultString);
        PRINTENUMCASE(VK_CULL_MODE_FRONT_AND_BACK, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeCullModeFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteCullModeFlagsEnumAsString, VK_SHADER_STAGE_VERTEX_BIT, VK_CULL_MODE_FRONT_AND_BACK);
    return flagsString.asCharArray();
}


const char* WriteColorComponentFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_COLOR_COMPONENT_R_BIT, pResultString);
        PRINTENUMCASE(VK_COLOR_COMPONENT_G_BIT, pResultString);
        PRINTENUMCASE(VK_COLOR_COMPONENT_B_BIT, pResultString);
        PRINTENUMCASE(VK_COLOR_COMPONENT_A_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeColorComponentFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteColorComponentFlagsEnumAsString, VK_COLOR_COMPONENT_R_BIT, VK_COLOR_COMPONENT_A_BIT);
    return flagsString.asCharArray();
}

const char* WriteDescriptorPoolCreateFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeDescriptorPoolCreateFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteDescriptorPoolCreateFlagsEnumAsString, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT);
    return flagsString.asCharArray();
}

const char* WriteAttachmentDescriptionFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeAttachmentDescriptionFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteAttachmentDescriptionFlagsEnumAsString, VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT, VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT);
    return flagsString.asCharArray();
}

const char* WritePipelineStageFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, pResultString);
        PRINTENUMCASE(VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, pResultString);
        PRINTENUMCASE(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, pResultString);
        PRINTENUMCASE(VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, pResultString);
        PRINTENUMCASE(VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, pResultString);
        PRINTENUMCASE(VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, pResultString);
        PRINTENUMCASE(VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, pResultString);
        PRINTENUMCASE(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, pResultString);
        PRINTENUMCASE(VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, pResultString);
        PRINTENUMCASE(VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, pResultString);
        PRINTENUMCASE(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, pResultString);
        PRINTENUMCASE(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, pResultString);
        PRINTENUMCASE(VK_PIPELINE_STAGE_TRANSFER_BIT, pResultString);
        PRINTENUMCASE(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, pResultString);
        PRINTENUMCASE(VK_PIPELINE_STAGE_HOST_BIT, pResultString);
        PRINTENUMCASE(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, pResultString);
        PRINTENUMCASE(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposePipelineStageFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WritePipelineStageFlagsEnumAsString, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    return flagsString.asCharArray();
}

const char* WriteAccessFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_ACCESS_INDIRECT_COMMAND_READ_BIT, pResultString);
        PRINTENUMCASE(VK_ACCESS_INDEX_READ_BIT, pResultString);
        PRINTENUMCASE(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, pResultString);
        PRINTENUMCASE(VK_ACCESS_UNIFORM_READ_BIT, pResultString);
        PRINTENUMCASE(VK_ACCESS_INPUT_ATTACHMENT_READ_BIT, pResultString);
        PRINTENUMCASE(VK_ACCESS_SHADER_READ_BIT, pResultString);
        PRINTENUMCASE(VK_ACCESS_SHADER_WRITE_BIT, pResultString);
        PRINTENUMCASE(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, pResultString);
        PRINTENUMCASE(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, pResultString);
        PRINTENUMCASE(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT, pResultString);
        PRINTENUMCASE(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, pResultString);
        PRINTENUMCASE(VK_ACCESS_TRANSFER_READ_BIT, pResultString);
        PRINTENUMCASE(VK_ACCESS_TRANSFER_WRITE_BIT, pResultString);
        PRINTENUMCASE(VK_ACCESS_HOST_READ_BIT, pResultString);
        PRINTENUMCASE(VK_ACCESS_HOST_WRITE_BIT, pResultString);
        PRINTENUMCASE(VK_ACCESS_MEMORY_READ_BIT, pResultString);
        PRINTENUMCASE(VK_ACCESS_MEMORY_WRITE_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeAccessFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteAccessFlagsEnumAsString, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, VK_ACCESS_MEMORY_WRITE_BIT);
    return flagsString.asCharArray();
}

const char* WriteDependencyFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_DEPENDENCY_BY_REGION_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeDependencyFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteDependencyFlagsEnumAsString, VK_DEPENDENCY_BY_REGION_BIT, VK_DEPENDENCY_BY_REGION_BIT);
    return flagsString.asCharArray();
}

const char* WriteCmdPoolCreateFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, pResultString);
        PRINTENUMCASE(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeCmdPoolCreateFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteCmdPoolCreateFlagsEnumAsString, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    return flagsString.asCharArray();
}

const char* WriteCmdPoolResetFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeCmdPoolResetFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteCmdPoolResetFlagsEnumAsString, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
    return flagsString.asCharArray();
}

const char* WriteCmdBufferUsageFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, pResultString);
        PRINTENUMCASE(VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT, pResultString);
        PRINTENUMCASE(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeCmdBufferUsageFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteCmdBufferUsageFlagsEnumAsString, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    return flagsString.asCharArray();
}

const char* WriteQueryControlFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_QUERY_CONTROL_PRECISE_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeQueryControlFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteQueryControlFlagsEnumAsString, VK_QUERY_CONTROL_PRECISE_BIT, VK_QUERY_CONTROL_PRECISE_BIT);
    return flagsString.asCharArray();
}

const char* WriteCommandBufferResetFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_QUERY_CONTROL_PRECISE_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeCommandBufferResetFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteCommandBufferResetFlagsEnumAsString, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT, VK_QUERY_CONTROL_PRECISE_BIT);
    return flagsString.asCharArray();
}

const char* WriteStencilFaceFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_STENCIL_FACE_FRONT_BIT, pResultString);
        PRINTENUMCASE(VK_STENCIL_FACE_BACK_BIT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeStencilFaceFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteStencilFaceFlagsEnumAsString, VK_STENCIL_FACE_FRONT_BIT, VK_STENCIL_FACE_BACK_BIT);
    return flagsString.asCharArray();
}

const char* WriteSurfaceTransformFlagBitsKHRFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, pResultString);
        PRINTENUMCASE(VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR, pResultString);
        PRINTENUMCASE(VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR, pResultString);
        PRINTENUMCASE(VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR, pResultString);
        PRINTENUMCASE(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR, pResultString);
        PRINTENUMCASE(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR, pResultString);
        PRINTENUMCASE(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR, pResultString);
        PRINTENUMCASE(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR, pResultString);
        PRINTENUMCASE(VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeSurfaceTransformFlagBitsKHRFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteSurfaceTransformFlagBitsKHRFlagsEnumAsString, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR);
    return flagsString.asCharArray();
}

const char* WriteCompositeAlphaFlagsEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, pResultString);
        PRINTENUMCASE(VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR, pResultString);
        PRINTENUMCASE(VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR, pResultString);
        PRINTENUMCASE(VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeCompositeAlphaFlagsEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteCompositeAlphaFlagsEnumAsString, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR);
    return flagsString.asCharArray();
}

const char* WriteDebugReportFlagsEXTEnumAsString(uint32 flags)
{
    const char* pResultString = nullptr;

    switch (flags)
    {
        // *INDENT-OFF*  to prevent astyle from wrongly indenting this next section
        PRINTENUMCASE(VK_DEBUG_REPORT_INFORMATION_BIT_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_WARNING_BIT_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_ERROR_BIT_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_DEBUG_BIT_EXT, pResultString);
        PRINTENUMCASE(VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT, pResultString);
        // *INDENT-ON*
    }

    return pResultString;
}

std::string VktUtil::DecomposeDebugReportFlagsEXTEnumAsString(uint32 flags)
{
    gtASCIIString flagsString;
    VktUtil::DecomposeFlags(flags, flagsString, WriteDebugReportFlagsEXTEnumAsString, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT);
    return flagsString.asCharArray();
}

//-----------------------------------------------------------------------------
/// Write a pointer value as a string. Used to display pointer values in the
/// form 0x...
/// \param ptr the pointer to be displayed
/// \return string containing the pointer
//-----------------------------------------------------------------------------
std::string VktUtil::WritePointerAsString(const void* ptr)
{
    char string[32] = { '\0' };
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
    sprintf_s(string, 32, GT_POINTER_FORMAT, reinterpret_cast<gtUInt32>(ptr));
#else
    sprintf_s(string, 32, GT_POINTER_FORMAT, reinterpret_cast<gtUInt64>(ptr));
#endif
    return std::string(string);
}

#pragma warning (push)
#pragma warning (disable : 4477)

// This prevents VS2015 from complaining about imperfect "%" formatting when printing Vulkan objects.
// This only applies to the 32-bit version of VulkanServer.
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
    #pragma warning (disable : 4313)
#endif

//-----------------------------------------------------------------------------
/// Write a 64-bit unsigned int value as a string.
/// For now, these are displayed as pointer values, since that's how these were
/// displayed before.
/// \param value the pointer to be displayed
/// \return string containing the pointer
//-----------------------------------------------------------------------------
std::string VktUtil::WriteUint64AsString(uint64_t value)
{
    char string[32] = {'\0'};

    sprintf_s(string, 32, GT_64_BIT_POINTER_ASCII_FORMAT_LOWERCASE, value);
    return std::string(string);
}

#pragma warning (pop)
