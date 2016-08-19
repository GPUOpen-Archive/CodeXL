////////////////////////////////////////////////////////////////////////////////
//
// The University of Illinois/NCSA
// Open Source License (NCSA)
// 
// Copyright (c) 2014-2015, Advanced Micro Devices, Inc. All rights reserved.
// 
// Developed by:
// 
//                 AMD Research and AMD HSA Software Development
// 
//                 Advanced Micro Devices, Inc.
// 
//                 www.amd.com
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal with the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
// 
//  - Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimers.
//  - Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimers in
//    the documentation and/or other materials provided with the distribution.
//  - Neither the names of Advanced Micro Devices, Inc,
//    nor the names of its contributors may be used to endorse or promote
//    products derived from this Software without specific prior written
//    permission.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS WITH THE SOFTWARE.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef HSA_EXT_IMAGE_H
#define HSA_EXT_IMAGE_H

#include "hsa.h"

#undef HSA_API
#ifdef HSA_EXPORT_IMAGES
#define HSA_API HSA_API_EXPORT
#else
#define HSA_API HSA_API_IMPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/ 

/** \defgroup ext-images Images and Samplers
 *  @{
 */

/**
 * @brief Image handle, populated by ::hsa_ext_image_create. Images
 * handles are only unique within an agent, not across agents.
 *
 */
typedef struct hsa_ext_image_s {
  /**
   * Opaque handle.
   */
  uint64_t handle;

} hsa_ext_image_t;

/**
 * @brief Geometry associated with the HSA image (image dimensions allowed in
 * HSA). The enumeration values match the BRIG type BrigImageGeometry.
 */
typedef enum {
  /**
   * One-dimensional image addressed by width coordinate.
   */
  HSA_EXT_IMAGE_GEOMETRY_1D = 0,

  /**
   * Two-dimensional image addressed by width and height coordinates.
   */
  HSA_EXT_IMAGE_GEOMETRY_2D = 1,

  /**
   * Three-dimensional image addressed by width, height, and depth coordinates.
   */
  HSA_EXT_IMAGE_GEOMETRY_3D = 2,

  /**
   * Array of one-dimensional images with the same size and format. 1D arrays
   * are addressed by index and width coordinate.
   */
  HSA_EXT_IMAGE_GEOMETRY_1DA = 3,

  /**
   * Array of two-dimensional images with the same size and format. 2D arrays
   * are addressed by index and width and height coordinates.
   */
  HSA_EXT_IMAGE_GEOMETRY_2DA = 4,

  /**
   * One-dimensional image interpreted as a buffer with specific restrictions.
   */
  HSA_EXT_IMAGE_GEOMETRY_1DB = 5,

  /**
   * Two-dimensional depth image addressed by width and height coordinates.
   */
  HSA_EXT_IMAGE_GEOMETRY_2DDEPTH = 6,

  /**
   * Array of two-dimensional depth images with the same size and format.  2D
   * arrays are addressed by index and width and height coordinates.
   */
  HSA_EXT_IMAGE_GEOMETRY_2DADEPTH = 7
} hsa_ext_image_geometry_t;

/**
 * @brief Channel type associated with the elements of an image. See the Image
 * section in the HSA Programming Reference Manual for definitions on each
 * component type. The enumeration values match the BRIG type
 * BrigImageChannelType.
 */
typedef enum {
  HSA_EXT_IMAGE_CHANNEL_TYPE_SNORM_INT8 = 0,
  HSA_EXT_IMAGE_CHANNEL_TYPE_SNORM_INT16 = 1,
  HSA_EXT_IMAGE_CHANNEL_TYPE_UNORM_INT8 = 2,
  HSA_EXT_IMAGE_CHANNEL_TYPE_UNORM_INT16 = 3,
  HSA_EXT_IMAGE_CHANNEL_TYPE_UNORM_INT24 = 4,
  HSA_EXT_IMAGE_CHANNEL_TYPE_UNORM_SHORT_555 = 5,
  HSA_EXT_IMAGE_CHANNEL_TYPE_UNORM_SHORT_565 = 6,
  HSA_EXT_IMAGE_CHANNEL_TYPE_UNORM_SHORT_101010 = 7,
  HSA_EXT_IMAGE_CHANNEL_TYPE_SIGNED_INT8 = 8,
  HSA_EXT_IMAGE_CHANNEL_TYPE_SIGNED_INT16 = 9,
  HSA_EXT_IMAGE_CHANNEL_TYPE_SIGNED_INT32 = 10,
  HSA_EXT_IMAGE_CHANNEL_TYPE_UNSIGNED_INT8 = 11,
  HSA_EXT_IMAGE_CHANNEL_TYPE_UNSIGNED_INT16 = 12,
  HSA_EXT_IMAGE_CHANNEL_TYPE_UNSIGNED_INT32 = 13,
  HSA_EXT_IMAGE_CHANNEL_TYPE_HALF_FLOAT = 14,
  HSA_EXT_IMAGE_CHANNEL_TYPE_FLOAT = 15
} hsa_ext_image_channel_type_t;

/**
 *
 * @brief Channel order associated with the elements of an image. See the
 * Image section in the HSA Programming Reference Manual for definitions on each
 * component order. The enumeration values match the BRIG type
 * BrigImageChannelOrder.
 */
typedef enum {
  HSA_EXT_IMAGE_CHANNEL_ORDER_A = 0,
  HSA_EXT_IMAGE_CHANNEL_ORDER_R = 1,
  HSA_EXT_IMAGE_CHANNEL_ORDER_RX = 2,
  HSA_EXT_IMAGE_CHANNEL_ORDER_RG = 3,
  HSA_EXT_IMAGE_CHANNEL_ORDER_RGX = 4,
  HSA_EXT_IMAGE_CHANNEL_ORDER_RA = 5,
  HSA_EXT_IMAGE_CHANNEL_ORDER_RGB = 6,
  HSA_EXT_IMAGE_CHANNEL_ORDER_RGBX = 7,
  HSA_EXT_IMAGE_CHANNEL_ORDER_RGBA = 8,
  HSA_EXT_IMAGE_CHANNEL_ORDER_BGRA = 9,
  HSA_EXT_IMAGE_CHANNEL_ORDER_ARGB = 10,
  HSA_EXT_IMAGE_CHANNEL_ORDER_ABGR = 11,
  HSA_EXT_IMAGE_CHANNEL_ORDER_SRGB = 12,
  HSA_EXT_IMAGE_CHANNEL_ORDER_SRGBX = 13,
  HSA_EXT_IMAGE_CHANNEL_ORDER_SRGBA = 14,
  HSA_EXT_IMAGE_CHANNEL_ORDER_SBGRA = 15,
  HSA_EXT_IMAGE_CHANNEL_ORDER_INTENSITY = 16,
  HSA_EXT_IMAGE_CHANNEL_ORDER_LUMINANCE = 17,
  HSA_EXT_IMAGE_CHANNEL_ORDER_DEPTH = 18,
  HSA_EXT_IMAGE_CHANNEL_ORDER_DEPTH_STENCIL = 19
} hsa_ext_image_channel_order_t;

/**
 * @brief Image format.
 */
typedef struct hsa_ext_image_format_s {
  /**
   * Channel type.
   */
  hsa_ext_image_channel_type_t channel_type;

  /**
   * Channel order.
   */
  hsa_ext_image_channel_order_t channel_order;
} hsa_ext_image_format_t;

/**
 * @brief Implementation-independent image descriptor.
 */
typedef struct hsa_ext_image_descriptor_s {
  /**
   * Image geometry.
   */
  hsa_ext_image_geometry_t geometry;
  /**
   * Width of the image, in components.
   */
  size_t width;
  /**
   * Height of the image, in components. Only defined if the geometry is 2D or
   * higher.
   */
  size_t height;
  /**
   * Depth of the image, in components. Only defined if @a geometry is
   * ::HSA_EXT_IMAGE_GEOMETRY_3D. A depth of 0 is same as a depth of 1.
   */
  size_t depth;
  /**
   * Number of images in the image array. Only defined if @a geometry is
   * ::HSA_EXT_IMAGE_GEOMETRY_1DA, ::HSA_EXT_IMAGE_GEOMETRY_2DA, or
   * HSA_EXT_IMAGE_GEOMETRY_2DADEPTH.
   */
  size_t array_size;
  /**
   * Image format.
   */
  hsa_ext_image_format_t format;
} hsa_ext_image_descriptor_t;

/**
 * @brief Image capability.
 */
typedef enum {
  /**
   * Images of this geometry and format are not supported in the agent.
   */
  HSA_EXT_IMAGE_CAPABILITY_NOT_SUPPORTED = 0x0,
  /**
   * Read-only images of this geometry and format are supported by the
   * agent.
   */
  HSA_EXT_IMAGE_CAPABILITY_READ_ONLY = 0x1,
  /**
   * Write-only images of this geometry and format are supported by the
   * agent.
   */
  HSA_EXT_IMAGE_CAPABILITY_WRITE_ONLY = 0x2,
  /**
   * Read-write images of this geometry and format are supported by the
   * agent.
   */
  HSA_EXT_IMAGE_CAPABILITY_READ_WRITE = 0x4,
  /**
   * Images of this geometry and format can be accessed from read-modify-write
   * operations in the agent.
   */
  HSA_EXT_IMAGE_CAPABILITY_READ_MODIFY_WRITE = 0x8,
  /**
   * Images of this geometry and format are guaranteed to have a consistent
   * data layout regardless of how they are accessed by the associated
   * agent.
   */
  HSA_EXT_IMAGE_CAPABILITY_ACCESS_INVARIANT_DATA_LAYOUT = 0x10
} hsa_ext_image_capability_t;

/**
 * @brief Retrieve the supported image capabilities for a given combination of
 * agent, image format and geometry.
 *
 * @param[in] agent Agent to be associated with the image.
 *
 * @param[in] geometry Geometry.
 *
 * @param[in] image_format Pointer to an image format. Must not be NULL.
 *
 * @param[out] capability_mask Pointer to a memory location where the HSA
 * runtime stores a bit-mask of supported image capability
 * (::hsa_ext_image_capability_t) values. Must not be NULL.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_AGENT The agent is invalid.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p geometry is not a valid image
 * geometry value, @p image_format is NULL, or @p capability_mask is NULL.
 */
hsa_status_t HSA_API
    hsa_ext_image_get_capability(hsa_agent_t agent,
                                 hsa_ext_image_geometry_t geometry,
                                 const hsa_ext_image_format_t *image_format,
                                 uint32_t *capability_mask);

/**
 * @brief Agent-specific image size and alignment requirements, populated by
 * ::hsa_ext_image_data_get_info.
 */
typedef struct hsa_ext_image_data_info_s {
  /**
   * Image data size, in bytes.
   */
  size_t size;

  /**
   * Image data alignment, in bytes.
   */
  size_t alignment;

} hsa_ext_image_data_info_t;

/**
 * @brief Retrieve the image data requirements for a given combination of image
 * descriptor, access permission, and agent.
 *
 * @details The optimal image data size and alignment requirements may vary
 * depending on the image attributes specified in @p image_descriptor. Also,
 * different implementation of the HSA runtime may return different requirements
 * for the same input values.
 *
 * The implementation must return the same image data requirements for different
 * access permissions with exactly the same image descriptor as long as
 * ::hsa_ext_image_get_capability reports
 * ::HSA_EXT_IMAGE_CAPABILITY_ACCESS_INVARIANT_DATA_LAYOUT for the geometry
 * and image format contained in the image descriptor.
 *
 * @param[in] agent Agent to be associated with the image.
 *
 * @param[in] image_descriptor Pointer to an image descriptor. Must not be NULL.
 *
 * @param[in] access_permission Image access mode for @a agent.
 *
 * @param[out] image_data_info Memory location where the runtime stores the
 * size and alignment requirements. Must not be NULL.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_AGENT The agent is invalid.
 *
 * @retval ::HSA_EXT_STATUS_ERROR_IMAGE_FORMAT_UNSUPPORTED The agent does
 * not support the image format specified by the descriptor.
 *
 * @retval ::HSA_EXT_STATUS_ERROR_IMAGE_SIZE_UNSUPPORTED The agent does
 * not support the image dimensions specified by the format descriptor.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p image_descriptor is NULL, @p
 * access_permission is not a valid access permission value, or @p
 * image_data_info is NULL.
 */
hsa_status_t HSA_API hsa_ext_image_data_get_info(
    hsa_agent_t agent, const hsa_ext_image_descriptor_t *image_descriptor,
    hsa_access_permission_t access_permission,
    hsa_ext_image_data_info_t *image_data_info);

/**
 * @brief Creates a agent-defined image handle from an
 * implementation-independent image descriptor and a agent-specific image
 * data.
 *
 * @details Image created with different access permissions but the same image
 * descriptor can share the same image data if
 * ::HSA_EXT_IMAGE_CAPABILITY_ACCESS_INVARIANT_DATA_LAYOUT is reported by
 * ::hsa_ext_image_get_capability for the image format specified in the image
 * descriptor. Images with a s-form channel order can share the same image data
 * with other images that have the corresponding non-s-form channel order,
 * provided the rest of their image descriptors are identical.
 *
 * If necessary, an application can use image operations (import, export, copy,
 * clear) to prepare the image for the intended use regardless of the access
 * permissions.
 *
 * @param[in] agent agent to be associated with the image.
 *
 * @param[in] image_descriptor Pointer to an image descriptor. Must not be NULL.
 *
 * @param[in] image_data Image data buffer that must have been allocated
 * according to the size and alignment requirements dictated by
 * ::hsa_ext_image_data_get_info.  Must not be NULL.
 *
 * Any previous memory contents are preserved upon creation. The application is
 * responsible for ensuring that the lifetime of the image data exceeds that of
 * all the associated images.
 *
 * @param[in] access_permission Access permission of the image by the
 * agent. The access permission defines how the agent expects to use the
 * image and must match the corresponding HSAIL image handle type. The agent
 * must support the image format specified in @p image_descriptor for the given
 * permission.
 *
 * @param[out] image Pointer to a memory location where the HSA runtime stores
 * the newly created image handle. Must not be NULL.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_AGENT The agent is invalid.
 *
 * @retval ::HSA_EXT_STATUS_ERROR_IMAGE_FORMAT_UNSUPPORTED The agent does
 * not have the capability to support the image format contained in the image
 * descriptor using the specified access permission.
 *
 * @retval ::HSA_STATUS_ERROR_OUT_OF_RESOURCES The HSA runtime cannot create the
 * image because it is out of resources (for example, the agent does not
 * support the creation of more image handles with the given access permission).
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p image_descriptor is NULL, @p
 * image_data is NULL, @p access_permission is not a valid access permission
 * value, or @p image is NULL.
 */
hsa_status_t HSA_API
    hsa_ext_image_create(hsa_agent_t agent,
                         const hsa_ext_image_descriptor_t *image_descriptor,
                         const void *image_data,
                         hsa_access_permission_t access_permission,
                         hsa_ext_image_t *image);

/**
 * @brief Destroy an image previously created using ::hsa_ext_image_create.
 *
 * @details Destroying the image handle does not free the associated image data,
 * or modify its contents. The application should not destroy an image while
 * there are references to it queued for execution or currently being used in a
 * kernel.
 *
 * @param[in] agent Agent associated with the image.
 *
 * @param[in] image Image.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_AGENT The agent is invalid.
 */
hsa_status_t HSA_API
    hsa_ext_image_destroy(hsa_agent_t agent, hsa_ext_image_t image);

/**
 * @brief Copies a portion of one image (the source) to another image (the
 * destination).
 *
 * @details The source and destination image formats should match, except if the
 * channel type of one of the images is the standard form of the channel type of
 * the other image. For example, it is allowed to copy a source image with a
 * channel type of HSA_EXT_IMAGE_CHANNEL_ORDER_SRGB to a destination image with
 * a channel type of HSA_EXT_IMAGE_CHANNEL_ORDER_RGB.
 *
 * The source and destination images do not have to be of the same geometry and
 * appropriate scaling is performed by the HSA runtime. It is possible to copy
 * subregions between any combinations of source and destination types, provided
 * that the dimensions of the subregions are the same. For example, it is
 * allowed to copy a rectangular region from a 2D image to a slice of a 3D
 * image.
 *
 * If the source and destination image data overlap, or the combination of
 * offset and range references an out-out-bounds element in any of the images,
 * the behavior is undefined.
 *
 * @param[in] agent Agent associated with both images.
 *
 * @param[in] src_image Source image. The agent associated with the source
 * image must be identical to that of the destination image.
 *
 * @param[in] src_offset Pointer to the offset within the source image where to
 * copy the data from. Must not be NULL.
 *
 * @param[in] dst_image Destination image.
 *
 * @param[in] dst_offset Pointer to the offset within the destination
 * image where to copy the data. Must not be NULL.
 *
 * @param[in] range Dimensions of the image portion to be copied. The HSA
 * runtime computes the size of the image data to be copied using this
 * argument. Must not be NULL.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_AGENT The agent is invalid.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p src_offset is
 * NULL, @p dst_offset is NULL, or @p range is NULL.
 */
hsa_status_t HSA_API
    hsa_ext_image_copy(hsa_agent_t agent, hsa_ext_image_t src_image,
                       const hsa_dim3_t *src_offset, hsa_ext_image_t dst_image,
                       const hsa_dim3_t *dst_offset, const hsa_dim3_t *range);

/**
 * @brief Image region.
 */
typedef struct hsa_ext_image_region_s {
  /**
   * Offset within an image (in coordinates).
   */
  hsa_dim3_t offset;

  /**
   * Dimensions of the image range (in coordinates). The x, y, and z dimensions
   * correspond to width, height, and depth respectively.
   */
  hsa_dim3_t range;
} hsa_ext_image_region_t;

/**
 * @brief Import a linearly organized image data from memory directly to an
 * image handle.
 *
 * @details This operation updates the image data referenced by the image handle
 * from the source memory. The size of the data imported from memory is
 * implicitly derived from the image region.
 *
 * If @p src_row_pitch is smaller than the destination region width (in bytes),
 * then @p src_row_pitch = region width.
 *
 * If @p src_slice_pitch is smaller than the destination region width * region
 * height (in bytes), then @p src_slice_pitch = region width * region height.
 *
 * It is the application's responsibility to avoid out of bounds memory access.
 *
 * None of the source memory or image data memory in the previously created
 * ::hsa_ext_image_create image handle can overlap.  Overlapping of any
 * of the source and destination memory within the import operation produces
 * undefined results.
 *
 * @param[in] agent Agent associated with the image.
 *
 * @param[in] src_memory Source memory. Must not be NULL.
 *
 * @param[in] src_row_pitch Number of bytes in one row of the source memory.
 *
 * @param[in] src_slice_pitch Number of bytes in one slice of the source memory.
 *
 * @param[in] dst_image Destination image.
 *
 * @param[in] image_region Pointer to the image region to be updated. Must not
 * be NULL.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_AGENT The agent is invalid.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p src_memory is NULL, or @p
 * image_region is NULL.
 *
 */
hsa_status_t HSA_API
    hsa_ext_image_import(hsa_agent_t agent, const void *src_memory,
                         size_t src_row_pitch, size_t src_slice_pitch,
                         hsa_ext_image_t dst_image,
                         const hsa_ext_image_region_t *image_region);

/**
 * @brief Export the image data to linearly organized memory.
 *
 * @details The operation updates the destination memory with the image data of
 * @p src_image. The size of the data exported to memory is implicitly derived
 * from the image region.
 *
 * If @p dst_row_pitch is smaller than the source region width (in bytes), then
 * @p dst_row_pitch = region width.
 *
 * If @p dst_slice_pitch is smaller than the source region width * region height
 * (in bytes), then @p dst_slice_pitch = region width * region height.
 *
 * It is the application's responsibility to avoid out of bounds memory access.
 *
 * None of the destination memory or image data memory in the previously created
 * ::hsa_ext_image_create image handle can overlap. Overlapping of any of
 * the source and destination memory within the export operation produces
 * undefined results.
 *
 * @param[in] agent Agent associated with the image.
 *
 * @param[in] src_image Source image.
 *
 * @param[in] dst_memory Destination memory. Must not be NULL.
 *
 * @param[in] dst_row_pitch Number of bytes in one row of the destination
 * memory.
 *
 * @param[in] dst_slice_pitch Number of bytes in one slice of the destination
 * memory.
 *
 * @param[in] image_region Pointer to the image region to be exported. Must not
 * be NULL.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_AGENT The agent is invalid.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p dst_memory is NULL, or @p
 * image_region is NULL.
 */
hsa_status_t HSA_API
    hsa_ext_image_export(hsa_agent_t agent, hsa_ext_image_t src_image,
                         void *dst_memory, size_t dst_row_pitch,
                         size_t dst_slice_pitch,
                         const hsa_ext_image_region_t *image_region);

/**
 * @brief Clear an image to the specified value.
 *
 * @details Clearing an image does not perform any format conversion and the
 * provided clear data is directly stored regardless of the image format. The
 * lowest bits of the data (number of bits depending on the image component
 * type) stored in the cleared image are based on the image component order.
 *
 * The number of elements in @p data should match the number of access
 * components for the channel order of @p image, as determined by the HSA
 * Programmer's Reference Manual. A single element is required for
 * HSA_EXT_IMAGE_CHANNEL_ORDER_DEPTH and
 * HSA_EXT_IMAGE_CHANNEL_ORDER_DEPTH_STENCIL, while any other channel order
 * requires 4 elements.
 *
 * Each element in @p data is a 32-bit value. The type of each element
 * should match the access type associated with the channel type of @p image,
 * as determined by the HSA Programmer's Reference Manual:
 *  - HSA_EXT_IMAGE_CHANNEL_TYPE_SIGNED_INT8,
 *    HSA_EXT_IMAGE_CHANNEL_TYPE_SIGNED_INT16, and
 *    HSA_EXT_IMAGE_CHANNEL_TYPE_SIGNED_INT32 map to int32_t.
 *  - HSA_EXT_IMAGE_CHANNEL_TYPE_UNSIGNED_INT8,
 *    HSA_EXT_IMAGE_CHANNEL_TYPE_UNSIGNED_INT16, and
 *    HSA_EXT_IMAGE_CHANNEL_TYPE_UNSIGNED_INT32 map to uint32_t.
 *  - Any other channel type maps to a 32-bit float.
 *
 * @param[in] agent Agent associated with the image.
 *
 * @param[in] image Image to be cleared.
 *
 * @param[in] data Clear value array. Specifying a clear value outside of the
 * range that can be represented by an image format results in undefined
 * behavior. Must not be NULL.
 *
 * @param[in] image_region Pointer to the image region to clear. Must not be
 * NULL. If the region references an out-out-bounds element, the behavior is
 * undefined.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_AGENT The agent is invalid.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p data is NULL, or @p
 * image_region is NULL.
 */
hsa_status_t HSA_API
    hsa_ext_image_clear(hsa_agent_t agent, hsa_ext_image_t image,
                        const void *data,
                        const hsa_ext_image_region_t *image_region);

/**
 * @brief Sampler handle. Samplers are populated by
 * ::hsa_ext_sampler_create. Sampler handles are only unique within an
 * agent, not across agents.
 */
typedef struct hsa_ext_sampler_s {
  /**
   * Opaque handle.
   */
  uint64_t handle;
} hsa_ext_sampler_t;

/**
 * @brief Sampler address modes. The sampler address mode describes the
 * processing of out-of-range image coordinates. The values match the BRIG
 * type BrigSamplerAddressing.
 */
typedef enum {
  /**
   * Out-of-range coordinates are not handled.
   */
  HSA_EXT_SAMPLER_ADDRESSING_MODE_UNDEFINED = 0,

  /**
   * Clamp out-of-range coordinates to the image edge.
   */
  HSA_EXT_SAMPLER_ADDRESSING_MODE_CLAMP_TO_EDGE = 1,

  /**
   * Clamp out-of-range coordinates to the image border.
   */
  HSA_EXT_SAMPLER_ADDRESSING_MODE_CLAMP_TO_BORDER = 2,

  /**
   * Wrap out-of-range coordinates back into the valid coordinate range.
   */
  HSA_EXT_SAMPLER_ADDRESSING_MODE_REPEAT = 3,

  /**
   * Mirror out-of-range coordinates back into the valid coordinate range.
   */
  HSA_EXT_SAMPLER_ADDRESSING_MODE_MIRRORED_REPEAT = 4

} hsa_ext_sampler_addressing_mode_t;

/**
 * @brief Sampler coordinate modes. The enumeration values match the BRIG
 * BRIG_SAMPLER_COORD bit in BrigSamplerModifier.
 */
typedef enum {
  /**
   * Coordinates are all in the range of 0 to (dimension-1).
   */
  HSA_EXT_SAMPLER_COORDINATE_MODE_UNNORMALIZED = 0,

  /**
   * Coordinates are all in the range of 0.0 to 1.0.
   */
  HSA_EXT_SAMPLER_COORDINATE_MODE_NORMALIZED = 1

} hsa_ext_sampler_coordinate_mode_t;

/**
 * @brief Sampler filter modes. The enumeration values match the BRIG type
 * BrigSamplerFilter.
 */
typedef enum {
  /**
   * Filter to the image element nearest (in Manhattan distance) to the
   * specified coordinate.
   */
  HSA_EXT_SAMPLER_FILTER_MODE_NEAREST = 0,

  /**
   * Filter to the image element calculated by combining the elements in a 2x2
   * square block or 2x2x2 cube block around the specified coordinate. The
   * elements are combined using linear interpolation.
   */
  HSA_EXT_SAMPLER_FILTER_MODE_LINEAR = 1

} hsa_ext_sampler_filter_mode_t;

/**
 * @brief Implementation-independent sampler descriptor.
 */
typedef struct hsa_ext_sampler_descriptor_s {
  /**
   * Sampler coordinate mode describes the normalization of image coordinates.
   */
  hsa_ext_sampler_coordinate_mode_t coordinate_mode;

  /**
   * Sampler filter type describes the type of sampling performed.
   */
  hsa_ext_sampler_filter_mode_t filter_mode;

  /**
   * Sampler address mode describes the processing of out-of-range image
   * coordinates.
   */
  hsa_ext_sampler_addressing_mode_t address_mode;

} hsa_ext_sampler_descriptor_t;

/**
 * @brief Create a kernel agent defined sampler handle for a given combination
 * of a (agent-independent) sampler descriptor and agent.
 *
 * @param[in] agent Agent to be associated with the sampler.
 *
 * @param[in] sampler_descriptor Pointer to a sampler descriptor. Must not be
 * NULL.
 *
 * @param[out] sampler Memory location where the HSA runtime stores the newly
 * created sampler handle. Must not be NULL.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_AGENT The agent is invalid.
 *
 * @retval ::HSA_STATUS_ERROR_OUT_OF_RESOURCES The agent cannot create the
 * specified handle because it is out of resources.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_ARGUMENT @p sampler_descriptor is NULL, or
 * @p sampler is NULL.
 */
hsa_status_t HSA_API hsa_ext_sampler_create(
    hsa_agent_t agent, const hsa_ext_sampler_descriptor_t *sampler_descriptor,
    hsa_ext_sampler_t *sampler);

/**
 * @brief Destroy a sampler previously created using ::hsa_ext_sampler_create.
 *
 * @param[in] agent Agent associated with the sampler.
 *
 * @param[in] sampler Sampler.  The sampler handle should not be destroyed while
 * there are references to it queued for execution or currently being used in a
 * dispatch.
 *
 * @retval ::HSA_STATUS_SUCCESS The function has been executed successfully.
 *
 * @retval ::HSA_STATUS_ERROR_NOT_INITIALIZED The HSA runtime has not been
 * initialized.
 *
 * @retval ::HSA_STATUS_ERROR_INVALID_AGENT The agent is invalid.
 */
hsa_status_t HSA_API
    hsa_ext_sampler_destroy(hsa_agent_t agent, hsa_ext_sampler_t sampler);

/**
 * @brief Enumeration constants added to ::hsa_status_t by this extension.
 */
enum {
  /**
   * Image format is not supported.
   */
  HSA_EXT_STATUS_ERROR_IMAGE_FORMAT_UNSUPPORTED = 0x3000,
  /**
   * Image size is not supported.
   */
  HSA_EXT_STATUS_ERROR_IMAGE_SIZE_UNSUPPORTED = 0x3001
};

/**
 * @brief Enumeration constants added to ::hsa_agent_info_t by this
 * extension. The value of any of these attributes is undefined if the
 * agent is not a kernel agent, or the implementation does not support images.
 */
enum {
  /**
   * Maximum number of elements in 1D images. Must be at most 16384. The type
   * of this attribute is uint32_t.
   */
  HSA_EXT_AGENT_INFO_IMAGE_1D_MAX_ELEMENTS = 0x3000,
  /**
   * Maximum number of elements in 1DA images. Must be at most 16384. The type
   * of this attribute is uint32_t.
   */
  HSA_EXT_AGENT_INFO_IMAGE_1DA_MAX_ELEMENTS = 0x3001,
  /**
   * Maximum number of elements in 1DB images. Must be at most 65536.  The type
   * of this attribute is uint32_t.
   */
  HSA_EXT_AGENT_INFO_IMAGE_1DB_MAX_ELEMENTS = 0x3002,
  /**
   * Maximum dimensions (width, height) of 2D images, in image elements. The X
   * and Y maximums must be at most 16384. The type of this attribute is
   * uint32_t[2].
   */
  HSA_EXT_AGENT_INFO_IMAGE_2D_MAX_ELEMENTS = 0x3003,
  /**
   * Maximum dimensions (width, height) of 2DA images, in image elements. The X
   * and Y maximums must be at most 16384. The type of this attribute is
   * uint32_t[2].
   */
  HSA_EXT_AGENT_INFO_IMAGE_2DA_MAX_ELEMENTS = 0x3004,
  /**
   * Maximum dimensions (width, height) of 2DDEPTH images, in image
   * elements. The X and Y maximums must be at most 16384. The type of this
   * attribute is uint32_t[2].
   */
  HSA_EXT_AGENT_INFO_IMAGE_2DDEPTH_MAX_ELEMENTS = 0x3005,
  /**
   * Maximum dimensions (width, height) of 2DADEPTH images, in image
   * elements. The X and Y maximums must be at most 16384. The type of this
   * attribute is uint32_t[2].
   */
  HSA_EXT_AGENT_INFO_IMAGE_2DADEPTH_MAX_ELEMENTS = 0x3006,
  /**
   * Maximum dimensions (width, height, depth) of 3D images, in image
   * elements. The maximum along any dimension cannot exceed 2048. The type of
   * this attribute is uint32_t[3].
   */
  HSA_EXT_AGENT_INFO_IMAGE_3D_MAX_ELEMENTS = 0x3007,
  /**
   * Maximum number of image layers in a image array. Must not exceed 2048. The
   * type of this attribute is uint32_t.
   */
  HSA_EXT_AGENT_INFO_IMAGE_ARRAY_MAX_LAYERS = 0x3008,
  /**
   * Maximum number of read-only image handles that can be created at any one
   * time. Must be at least 128. The type of this attribute is uint32_t.
   */
  HSA_EXT_AGENT_INFO_MAX_IMAGE_RD_HANDLES = 0x3009,
  /**
   * Maximum number of write-only and read-write image handles (combined) that
   * can be created at any one time. Must be at least 64. The type of this
   * attribute is uint32_t.
   */
  HSA_EXT_AGENT_INFO_MAX_IMAGE_RORW_HANDLES = 0x300A,
  /**
   * Maximum number of sampler handlers that can be created at any one
   * time. Must be at least 16. The type of this attribute is uint32_t.
   */
  HSA_EXT_AGENT_INFO_MAX_SAMPLER_HANDLERS = 0x300B
};

/** @} */

#define hsa_ext_images_1_00

typedef struct hsa_ext_images_1_00_pfn_s {
  hsa_status_t (*hsa_ext_image_get_capability)(
      hsa_agent_t agent, hsa_ext_image_geometry_t geometry,
      const hsa_ext_image_format_t *image_format, uint32_t *capability_mask);

  hsa_status_t (*hsa_ext_image_data_get_info)(
      hsa_agent_t agent, const hsa_ext_image_descriptor_t *image_descriptor,
      hsa_access_permission_t access_permission,
      hsa_ext_image_data_info_t *image_data_info);

  hsa_status_t (*hsa_ext_image_create)(
      hsa_agent_t agent, const hsa_ext_image_descriptor_t *image_descriptor,
      const void *image_data, hsa_access_permission_t access_permission,
      hsa_ext_image_t *image);

  hsa_status_t (*hsa_ext_image_destroy)(hsa_agent_t agent,
                                        hsa_ext_image_t image);

  hsa_status_t (*hsa_ext_image_copy)(hsa_agent_t agent,
                                     hsa_ext_image_t src_image,
                                     const hsa_dim3_t *src_offset,
                                     hsa_ext_image_t dst_image,
                                     const hsa_dim3_t *dst_offset,
                                     const hsa_dim3_t *range);

  hsa_status_t (*hsa_ext_image_import)(
      hsa_agent_t agent, const void *src_memory, size_t src_row_pitch,
      size_t src_slice_pitch, hsa_ext_image_t dst_image,
      const hsa_ext_image_region_t *image_region);

  hsa_status_t (*hsa_ext_image_export)(
      hsa_agent_t agent, hsa_ext_image_t src_image, void *dst_memory,
      size_t dst_row_pitch, size_t dst_slice_pitch,
      const hsa_ext_image_region_t *image_region);

  hsa_status_t (*hsa_ext_image_clear)(
      hsa_agent_t agent, hsa_ext_image_t image, const void *data,
      const hsa_ext_image_region_t *image_region);

  hsa_status_t (*hsa_ext_sampler_create)(
      hsa_agent_t agent, const hsa_ext_sampler_descriptor_t *sampler_descriptor,
      hsa_ext_sampler_t *sampler);

  hsa_status_t (*hsa_ext_sampler_destroy)(hsa_agent_t agent,
                                          hsa_ext_sampler_t sampler);

} hsa_ext_images_1_00_pfn_t;

#ifdef __cplusplus
}  // end extern "C" block
#endif /*__cplusplus*/ 

#endif
