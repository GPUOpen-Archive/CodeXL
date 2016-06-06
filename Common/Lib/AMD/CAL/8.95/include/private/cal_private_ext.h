/* ============================================================

Copyright (c) 2007 Advanced Micro Devices, Inc.  All rights reserved.

Redistribution and use of this material is permitted under the following
conditions:

Redistributions must retain the above copyright notice and all terms of this
license.

In no event shall anyone redistributing or accessing or using this material
commence or participate in any arbitration or legal action relating to this
material against Advanced Micro Devices, Inc. or any copyright holders or
contributors. The foregoing shall survive any expiration or termination of
this license or any agreement or access or use related to this material.

ANY BREACH OF ANY TERM OF THIS LICENSE SHALL RESULT IN THE IMMEDIATE REVOCATION
OF ALL RIGHTS TO REDISTRIBUTE, ACCESS OR USE THIS MATERIAL.

THIS MATERIAL IS PROVIDED BY ADVANCED MICRO DEVICES, INC. AND ANY COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" IN ITS CURRENT CONDITION AND WITHOUT ANY
REPRESENTATIONS, GUARANTEE, OR WARRANTY OF ANY KIND OR IN ANY WAY RELATED TO
SUPPORT, INDEMNITY, ERROR FREE OR UNINTERRUPTED OPERATION, OR THAT IT IS FREE
FROM DEFECTS OR VIRUSES.  ALL OBLIGATIONS ARE HEREBY DISCLAIMED - WHETHER
EXPRESS, IMPLIED, OR STATUTORY - INCLUDING, BUT NOT LIMITED TO, ANY IMPLIED
WARRANTIES OF TITLE, MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
ACCURACY, COMPLETENESS, OPERABILITY, QUALITY OF SERVICE, OR NON-INFRINGEMENT.
IN NO EVENT SHALL ADVANCED MICRO DEVICES, INC. OR ANY COPYRIGHT HOLDERS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, PUNITIVE,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, REVENUE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED OR BASED ON ANY THEORY OF LIABILITY
ARISING IN ANY WAY RELATED TO THIS MATERIAL, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE. THE ENTIRE AND AGGREGATE LIABILITY OF ADVANCED MICRO DEVICES,
INC. AND ANY COPYRIGHT HOLDERS AND CONTRIBUTORS SHALL NOT EXCEED TEN DOLLARS
(US $10.00). ANYONE REDISTRIBUTING OR ACCESSING OR USING THIS MATERIAL ACCEPTS
THIS ALLOCATION OF RISK AND AGREES TO RELEASE ADVANCED MICRO DEVICES, INC. AND
ANY COPYRIGHT HOLDERS AND CONTRIBUTORS FROM ANY AND ALL LIABILITIES,
OBLIGATIONS, CLAIMS, OR DEMANDS IN EXCESS OF TEN DOLLARS (US $10.00). THE
FOREGOING ARE ESSENTIAL TERMS OF THIS LICENSE AND, IF ANY OF THESE TERMS ARE
CONSTRUED AS UNENFORCEABLE, FAIL IN ESSENTIAL PURPOSE, OR BECOME VOID OR
DETRIMENTAL TO ADVANCED MICRO DEVICES, INC. OR ANY COPYRIGHT HOLDERS OR
CONTRIBUTORS FOR ANY REASON, THEN ALL RIGHTS TO REDISTRIBUTE, ACCESS OR USE
THIS MATERIAL SHALL TERMINATE IMMEDIATELY. MOREOVER, THE FOREGOING SHALL
SURVIVE ANY EXPIRATION OR TERMINATION OF THIS LICENSE OR ANY AGREEMENT OR
ACCESS OR USE RELATED TO THIS MATERIAL.

NOTICE IS HEREBY PROVIDED, AND BY REDISTRIBUTING OR ACCESSING OR USING THIS
MATERIAL SUCH NOTICE IS ACKNOWLEDGED, THAT THIS MATERIAL MAY BE SUBJECT TO
RESTRICTIONS UNDER THE LAWS AND REGULATIONS OF THE UNITED STATES OR OTHER
COUNTRIES, WHICH INCLUDE BUT ARE NOT LIMITED TO, U.S. EXPORT CONTROL LAWS SUCH
AS THE EXPORT ADMINISTRATION REGULATIONS AND NATIONAL SECURITY CONTROLS AS
DEFINED THEREUNDER, AS WELL AS STATE DEPARTMENT CONTROLS UNDER THE U.S.
MUNITIONS LIST. THIS MATERIAL MAY NOT BE USED, RELEASED, TRANSFERRED, IMPORTED,
EXPORTED AND/OR RE-EXPORTED IN ANY MANNER PROHIBITED UNDER ANY APPLICABLE LAWS,
INCLUDING U.S. EXPORT CONTROL LAWS REGARDING SPECIFICALLY DESIGNATED PERSONS,
COUNTRIES AND NATIONALS OF COUNTRIES SUBJECT TO NATIONAL SECURITY CONTROLS.
MOREOVER, THE FOREGOING SHALL SURVIVE ANY EXPIRATION OR TERMINATION OF ANY
LICENSE OR AGREEMENT OR ACCESS OR USE RELATED TO THIS MATERIAL.

NOTICE REGARDING THE U.S. GOVERNMENT AND DOD AGENCIES: This material is
provided with "RESTRICTED RIGHTS" and/or "LIMITED RIGHTS" as applicable to
computer software and technical data, respectively. Use, duplication,
distribution or disclosure by the U.S. Government and/or DOD agencies is
subject to the full extent of restrictions in all applicable regulations,
including those found at FAR52.227 and DFARS252.227 et seq. and any successor
regulations thereof. Use of this material by the U.S. Government and/or DOD
agencies is acknowledgment of the proprietary rights of any copyright holders
and contributors, including those of Advanced Micro Devices, Inc., as well as
the provisions of FAR52.227-14 through 23 regarding privately developed and/or
commercial computer software.

This license forms the entire agreement regarding the subject matter hereof and
supersedes all proposals and prior discussions and writings between the parties
with respect thereto. This license does not affect any ownership, rights, title,
or interest in, or relating to, this material. No terms of this license can be
modified or waived, and no breach of this license can be excused, unless done
so in a writing signed by all affected parties. Each term of this license is
separately enforceable. If any term of this license is determined to be or
becomes unenforceable or illegal, such term shall be reformed to the minimum
extent necessary in order for this license to remain in effect in accordance
with its terms as modified by such reformation. This license shall be governed
by and construed in accordance with the laws of the State of Texas without
regard to rules on conflicts of law of any state or jurisdiction or the United
Nations Convention on the International Sale of Goods. All disputes arising out
of this license shall be subject to the jurisdiction of the federal and state
courts in Austin, Texas, and all defenses are hereby waived concerning personal
jurisdiction and venue of these courts.

============================================================ */

#ifndef __CAL_PRIVATE_EXT_H__
#define __CAL_PRIVATE_EXT_H__

#include "cal_private.h"
#include "cal_ext.h"
#include "cal_ext_counter.h"
#include "calcl.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CALAPIENTRYP
#define CALAPIENTRYP CALAPIENTRY *
#endif

#define CAL_CONFIG_TILING_KEY                  "CAL_TILING_MODE"
#define CAL_CONFIG_TILING_DEFAULT              "0"
#define CAL_CONFIG_TILING_LINEAR               "1"
#define CAL_CONFIG_TILING_TILED                "2"
#define CAL_CONFIG_FORCE_TEXTURE_CACHE_KEY     "CAL_FORCE_TEXTURE_CACHE"
#define CAL_CONFIG_FORCE_TEXTURE_CACHE_DEFAULT "0"
#define CAL_CONFIG_FORCE_TEXTURE_CACHE_HIT     "1"
#define CAL_CONFIG_FORCE_TEXTURE_CACHE_MISS    "2"
#define CAL_CONFIG_USE_RECT_PRIMITIVE_KEY      "CAL_USE_RECT_PRIMITIVE"
#define CAL_CONFIG_USE_RECT_PRIMITIVE_DEFAULT  "0"
#define CAL_CONFIG_USE_RECT_PRIMITIVE_OFF      "1"
#define CAL_CONFIG_USE_RECT_PRIMITIVE_ON       "2"
#define CAL_CONFIG_FORCE_REMOTE_MEMORY_KEY     "CAL_FORCE_REMOTE_MEMORY"
#define CAL_CONFIG_FORCE_REMOTE_MEMORY_DEFAULT "0"
#define CAL_CONFIG_FORCE_REMOTE_MEMORY_ON      "1"
#define CAL_CONFIG_DISABLE_ASYNC_DMA_KEY       "CAL_DISABLE_ASYNC_DMA"
#define CAL_CONFIG_DISABLE_ASYNC_DMA_DEFAULT   "0"
#define CAL_CONFIG_DISABLE_ASYNC_DMA_ON        "1"

#define CAL_CONFIG_DUMP_IL_KEY                 "CAL_DUMP_IL"
#define CAL_CONFIG_DUMP_IL_OFF                 "0"
#define CAL_CONFIG_DUMP_IL_ON                  "1"

#define CAL_CONFIG_DUMP_ISA_KEY                "CAL_DUMP_ISA"
#define CAL_CONFIG_DUMP_ISA_OFF                "0"
#define CAL_CONFIG_DUMP_ISA_ON                 "1"

#define CAL_CONFIG_OPENCL_MODE_KEY             "CAL_OPENCL_MODE"
#define CAL_CONFIG_OPENCL_MODE_OFF             "0"
#define CAL_CONFIG_OPENCL_MODE_ON              "1"

#define CAL_CONFIG_THREAD_SAFE_KEY             "CAL_THREAD_SAFE"
#define CAL_CONFIG_THREAD_SAFE_OFF             "0"
#define CAL_CONFIG_THREAD_SAFE_ON              "1"

#define CAL_MAX_CONSTANT_BUFFERS                16

typedef enum calPrivateExtidEnum {
    CAL_PRIVATE_EXT_PM4CAP                  = 0x8001,
    CAL_PRIVATE_EXT_SAMPLER_PARAM           = 0x8002,
    CAL_PRIVATE_EXT_DOMAIN_PARAMS           = 0x8003,
    CAL_PRIVATE_EXT_INTERNAL_COUNTERS       = 0x8005,
    CAL_PRIVATE_EXT_RUNTIME_CONFIG          = 0x8007,
    CAL_PRIVATE_EXT_RESOURCES               = 0x8008,
    CAL_PRIVATE_EXT_SYNC_OBJECT             = 0x8009,
    CAL_PRIVATE_EXT_DEVICE_CLOCKUP          = 0x800A,
    CAL_PRIVATE_EXT_MEMCOPY_RAW             = 0x800B,
    CAL_PRIVATE_EXT_EXTENDED_RUNPROGRAMGRID = 0x800C,
    CAL_PRIVATE_EXT_OPENGL                  = 0x800D,
    CAL_PRIVATE_EXT_RES_ALLOC               = 0x800E,
    CAL_PRIVATE_EXT_RES_ALLOC_SLICE_VIEW    = 0x800F,
    CAL_PRIVATE_EXT_HEAP                    = 0x8010,
    CAL_PRIVATE_EXT_ATOMIC_COUNTER          = 0x8011,
    CAL_PRIVATE_EXT_VIDEO                   = 0x8012,
    CAL_PRIVATE_EXT_COMPILER                = 0x8013,
    CAL_PRIVATE_EXT_EXTENDED_RUNPROGRAMGRID_CB_OFFSET = 0x8014,
    CAL_PRIVATE_EXT_EXTENDED_DEVICEATTRIBS  = 0x8015,
    CAL_PRIVATE_EXT_FLUSH_CACHE             = 0x8016,
    CAL_PRIVATE_EXT_MEMCOPY_PARTIAL         = 0x8017,

    CAL_PRIVATE_EXT_FIRST             = CAL_PRIVATE_EXT_PM4CAP,
    CAL_PRIVATE_EXT_LAST              = CAL_PRIVATE_EXT_MEMCOPY_PARTIAL,
} calPrivateExtid;


//
// Memory usage in a heap situation
//
typedef struct CALmemusageRec
{
    CALmem* mem;        ///< Array of CALmem
    CALuint memCount;   ///< CALmem count
} CALmemusage;

typedef struct CALprogramGridExtendedRec
{
    CALprogramGrid programGrid;     /** CALprogramGrid*/
    CALuint        struct_size;     /** size of extended structure */
    CALuint        extendedFlags;   /** Extended flags */
    CALuint        localSize;       /** size of OpenCL Local Memory in bytes */
    CALmemusage*   memUsage;        /** Memory to be paged in if using global heap */
    CALuint        cbOffsets[16];   /** Constant buffer offsets */
    CALdomain3D    partialGridBlock;/** Partial grid block */
} CALprogramGridExtended;

typedef enum CALprogramGridFlagsPrivateEnum
{
    CAL_RUNPROGRAMGRID_EXTENDED_STRUCTURE = (1 << 30),
} CALprogramGridFlagsPrivate;

typedef enum CALprogramGridExtendedFlagsPrivateEnum
{
    CAL_RUNPROGRAMGRID_EXTENDED_SET_LOCAL_SIZE          = (1 << 0),
    CAL_RUNPROGRAMGRID_EXTENDED_MEMORY_USAGE            = (1 << 1),
    CAL_RUNPROGRAMGRID_EXTENDED_CONSTANT_BUFFER_OFFSET  = (1 << 2),
    CAL_RUNPROGRAMGRID_EXTENDED_PARTIAL_GRID_BLOCK      = (1 << 3),
} CALprogramGridExtendedFlagsPrivate;

// flags for calCtxWaitForEvents
typedef enum CALwaitTypeEnum
{
    CAL_WAIT_LOW_CPU_UTILIZATION    = 0,
    CAL_WAIT_POLLING                = 1,
} CALwaitType;

typedef enum CALcounterExtendedTypeEnum
{
    CAL_COUNTER_EXTENDED_IDLE                   = CAL_COUNTER_IDLE,
    CAL_COUNTER_EXTENDED_INPUT_CACHE_HIT_RATE   = CAL_COUNTER_INPUT_CACHE_HIT_RATE,
    CAL_COUNTER_EXTENDED_TIMER,
    CAL_COUNTER_EXTENDED_TIMESTAMP,
} CALcounterExtendedType;

#define CAL_CHECK_ENUM_RANGE(value,type) CALAssert(value>=type##_FIRST && value<=type##_LAST)

//calEnableLogging
typedef void (CALAPIENTRYP PFNCALENABLELOGGINGPROC) (void);
//calLoggingCaptureMem
typedef void (CALAPIENTRYP PFNCALLOGGINGCAPTUREMEMPROC) (CALcontext ctx, CALmem mem);

//DEPRECATED!!!  Only kept around until everyone switches to the public extension.
//calCtxSetSamplerParameter
typedef CALresult (CALAPIENTRYP PFNCALSETSAMPLERPARAMETER) (CALcontext ctx, CALname name, CALsamplerParameter param, CALvoid* vals);
//DEPRECATED!!!

/*
 * Create a new performance counter object.
 *
 * counter (out) - Pointer to the returned counter object handle.
 * ctx (in) - Context in which to create the counter.
 */
typedef CALresult (CALAPIENTRYP PFNCALCTXCREATEPRIVATECOUNTER)(CALcounter* counter, CALcontext ctx);

/*
 * Configure a counter object to record the specified event in the specified
 * hardware block.  The meaning of the block and event inputs is platform
 * dependent.  One counter object can manage the counting of multiple events
 * in multiple blocks though some care must be taken when configuring the
 * counters as each block has a limit on the number of events it can count
 * simultaneously.  For instance, on the R6XX the GRBM (block 0) only has
 * two counters so it can count at most 2 events simultaneously.  If you
 * wish to count more events than a block supports you must run your test
 * case multiple times configuring different counters with each run.
 *
 * ctx (in) - Context that owns the counter.
 * counter (in) - Counter to configure.
 * block (in) - Index of the block to configure.
 * index (in) - Index of the hardware counter within the block to configure.
 * event (in) - Event you wish to count with the counter specified by block +
 *              index.
 */
typedef CALresult (CALAPIENTRYP PFNCALCTXCONFIGPRIVATECOUNTER)(CALcontext ctx, CALcounter counter, CALuint block, CALuint index, CALuint event);

/*
 * Retrieve the results from a counter object.  This should be called after
 * calCtxEndCounter.
 *
 * result (out) - Pointer to an array of CALuints that will store counter results.
 * ctx (in) - Context that owns the counter.
 * counter (in) - Counter from which to retrieve results.
 */
typedef CALresult (CALAPIENTRYP PFNCALCTXGETPRIVATECOUNTER)(CALuint64* result, CALcontext ctx, CALcounter counter);

/**
 * Set the compiler configuration flags.
 *
 * key (in) - String of key to set value for
 * value (in) - string value for key
 */
typedef CALresult (CALAPIENTRYP PFNCALCLCONFIG)(const CALchar* key, const CALchar* value);

/**
 * Clear the compiler configuration flags
 */
typedef CALvoid (CALAPIENTRYP PFNCALCLCLEARCONFIG)(void);

/**
 * Set the runtime configuration flags.
 *
 * key (in) - String of key to set value for
 * value (in) - string value for key
 */
typedef CALresult (CALAPIENTRYP PFNCALCONFIG)(const CALchar* key, const CALchar* value);

/**
 * Clear the runtime configuration flags
 */
typedef CALvoid (CALAPIENTRYP PFNCALCLEARCONFIG)(void);

//
// calResAllocView typedefs
//
typedef enum CALresallocviewflagsRec {
    CAL_RESALLOCVIEW_GLOBAL_BUFFER    = CAL_RESALLOC_GLOBAL_BUFFER, /**< used for global import/export buffer */
    CAL_RESALLOCVIEW_CACHEABLE        = CAL_RESALLOC_CACHEABLE,      /**< cacheable memory? */
    CAL_RESALLOCVIEW_LINEAR_ALIGNED   = CAL_RESALLOC_GLOBAL_BUFFER, /**< 256 byte alignment restriction. */
    CAL_RESALLOCVIEW_LINEAR_UNALIGNED = 3,                          /**< no alignment restrictions */
} CALresallocviewflags;

typedef enum CALrchannelorderRec {      //added to handle OpenCL channel_order requirement.
    CAL_CHANNEL_ORDER_CL_R,             /**<  */
    CAL_CHANNEL_ORDER_CL_A,             /**<  */
    CAL_CHANNEL_ORDER_CL_RG,            /**<  */
    CAL_CHANNEL_ORDER_CL_RA,            /**<  */
    CAL_CHANNEL_ORDER_CL_RGB,           /**< can only be used if format is UNORM_SHORT_565, UNORM_SHORT_555, or UNORM_INT101010 */
    CAL_CHANNEL_ORDER_CL_RGBA,          /**<  */
    CAL_CHANNEL_ORDER_CL_ARGB,          /**< can only be used if format is UNORM_INT8, SNORM_INT8, SIGNED_INT8, or UNSIGNED_INT8 */
    CAL_CHANNEL_ORDER_CL_BGRA,          /**< can only be used if format is UNORM_INT8, SNORM_INT8, SIGNED_INT8, or UNSIGNED_INT8 */
    CAL_CHANNEL_ORDER_CL_REPLICATE_R,
    CAL_CHANNEL_ORDER_CL_INTENSITY,
    CAL_CHANNEL_ORDER_CL_LUMINANCE,
    CAL_CHANNEL_ORDER_UNSPECIFIED,        /**<  */
    CAL_CHANNEL_ORDER_FIRST = CAL_CHANNEL_ORDER_CL_R,
    CAL_CHANNEL_ORDER_LAST  = CAL_CHANNEL_ORDER_UNSPECIFIED,
} CALchannelorder;

//cal 3D image support
/** CAL resource allocation flags **/
typedef enum CALresallocTypeEnum {
    CAL_RESALLOC_TYPE_SYSTEM            = 0,
    CAL_RESALLOC_TYPE_REMOTE            = 1,
    CAL_RESALLOC_TYPE_LOCAL             = 2
} CALresallocType;

typedef struct CALresourceDescRec {
    CALresallocType type;
    CALdomain3D     size;
    CALformat       format;
    CALchannelorder channelOrder;
    CALdimension    dimension;
    CALuint         mipLevels;
    CALvoid*        systemMemory;
    CALuint         flags;
    CALuint         systemMemorySize;
} CALresourceDesc;

typedef struct CALdeviceDescRec {
    CALdevice *  dev;
    CALuint      devCount;
} CALdeviceDesc;

typedef enum CALresallocsliceviewflagsRec {
    CAL_RESALLOCSLICEVIEW_GLOBAL_BUFFER    = CAL_RESALLOC_GLOBAL_BUFFER, /**< used for global import/export buffer */
    CAL_RESALLOCSLICEVIEW_CACHEABLE        = CAL_RESALLOC_CACHEABLE,      /**< cacheable memory? */
    CAL_RESALLOCSLICEVIEW_LINEAR_ALIGNED   = CAL_RESALLOC_GLOBAL_BUFFER, /**< 256 byte alignment restriction. */
    CAL_RESALLOCSLICEVIEW_LINEAR_UNALIGNED = CAL_RESALLOCVIEW_LINEAR_UNALIGNED,                          /**< no alignment restrictions */
    CAL_RESALLOCSLICEVIEW_LEVEL = 0x10, /**< sliceDesc.layer is not used, the whole level is only*/
    CAL_RESALLOCSLICEVIEW_LAYER = 0x20, /**< sliceDesc.layer is not used, the whole level is only*/
    CAL_RESALLOCSLICEVIEW_LEVEL_AND_LAYER = CAL_RESALLOCSLICEVIEW_LEVEL | CAL_RESALLOCSLICEVIEW_LAYER,
} CALresallocsliceviewflags;

typedef struct CALsliceDescRec {
   CALuint level;    //the level ID of 3d image
   CALuint layer;    //the layer ID of 3d image
} CALsliceDesc;

/**
 * @fn calResAllocSliceView(CALresource* resSlice
 *                     CALresource res,
 *                     CALdevice dev,
 *                     CALdomain3D size,
 *                     CALdomain offset,
 *                     CALformat format,
 *                     CALchannelorder channelOrder,
 *                     CALdimension resType,
 *                     CALsliceDesc sliceDesc,
 *                     CALuint flags)
 *
 * @brief Allocate an aliased memory resource
 *
 * allocates an aliased memory resource attached to a device <i>dev</i> and returns a
 * resource handle in <i>*resSlice</i> if successful. This memory is structured the same
 * as the memory resource passed in.
 *
 * Implementation will allow this memory to be accessible by all contexts
 * created on this device only. Contexts residing on other devices cannot access
 * this memory.
 *
 * <i>flags</i> can be zero or CAL_RESALLOC_GLOBAL_BUFFER, for 3d image, it is 0
 * - to specify that the resource will be used as a global
 *   buffer.
 *
 * There are some performance implications when <i>width</i> is not a multiple
 * of 64 for R6xx GPUs.
 *
 * @param resSlice (out) - returned resource handle. On error, res will be zero.
 * @param res (in)      - resource to alias.
 * @param dev (in)      - device the resource should be local.
 * @param size (in)     - size of aliased resource (in elements).
 * @param offset (in)   - offset of the aliased resource (in elements).
 * @param format (in)   - format/type of each element of the resource.
 * @param channelOrder (in) - color channel order associated with the format.
 * @param resType (in)  - type of resource view to create.
 * @param sliceDesc (in)  - the description for allocating a slice in 3d image.
 * @param flags (in)    - 0 or CAL_RESALLOC_GLOBAL_BUFFER.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calResFree
 */
typedef CALresult (CALAPIENTRYP PFNCALRESALLOCSLICEVIEW)(CALresource* resSlice, CALresource res, CALdevice dev, CALdomain3D size, CALdomain offset, CALformat format, CALchannelorder channelOrder, CALdimension resType, CALsliceDesc sliceDesc, CALuint flags);

/**
 * @fn calResAllocView(CALresource* resView
 *                     CALresource res,
 *                     CALdevice dev,
 *                     CALdomain3D size,
 *                     CALdomain offset,
 *                     CALformat format,
 *                     CALchannelorder channelOrder,
 *                     CALdimension resType,
 *                     CALuint flags)
 *
 * @brief Allocate an aliased memory resource
 *
 * allocates an aliased memory resource attached to a device <i>dev</i> and returns a
 * resource handle in <i>*res</i> if successful. This memory is structured the same
 * as the memory resource passed in.
 *
 * Implementation will allow this memory to be accessible by all contexts
 * created on this device only. Contexts residing on other devices cannot access
 * this memory.
 *
 * <i>flags</i> can be zero or CAL_RESALLOC_GLOBAL_BUFFER
 * - to specify that the resource will be used as a global
 *   buffer.
 *
 * There are some performance implications when <i>width</i> is not a multiple
 * of 64 for R6xx GPUs.
 *
 * @param resView (out) - returned resource handle. On error, res will be zero.
 * @param res (in)      - resource to alias.
 * @param dev (in)      - device the resource should be local.
 * @param size (in)     - size of aliased resource (in elements).
 * @param offset (in)   - offset of the aliased resource (in elements).
 * @param format (in)   - format/type of each element of the resource.
 * @param channelOrder (in) - color channel order associated with the format.
 * @param resType (in)  - type of resource view to create.
 * @param flags (in)    - currently unused.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calResFree
 */
typedef CALresult (CALAPIENTRYP PFNCALRESALLOCVIEW)(CALresource* resView, CALresource res, CALdevice dev, CALdomain3D size, CALdomain offset, CALformat format, CALchannelorder channelOrder, CALdimension resType, CALuint flags);

/**
 * @fn calCtxWaitForEvents(CALcontext ctx,
 *                         CALevent *events,
 *                         CALuint n,
 *                         CALuint flags)
 *
 * @brief wait until all programs referenced by event list have executed.
 *
 * @param ctx (in)    - CAL context
 * @param events (in) - array of events
 * @param n (in)      - number of events
 * @param flags (in)  - currently unused.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 */
typedef CALresult (CALAPIENTRYP PFNCALCTXWAITFOREVENTS) (CALcontext ctx, CALevent *events, CALuint n, CALuint flags);

/**
 * @fn calCtxFlushCache(CALcontext ctx,
 *                      CALuint flags)
 *
 * @brief Syncs command queue and flushes caches
 *
 * @param ctx (in)    - CAL context
 * @param flags (in)  - currently unused.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 */
typedef CALresult (CALAPIENTRYP PFNCALCTXFLUSHCACHE) (CALcontext ctx, CALuint flags);


/**
 * @fn calDeviceClockUp(CALdevice dev, CALuint flag)
 *
 * @brief clock up or clock down a adptor
 *
 * returns in <i>*info</i> the information regarding the func.
 * This method returns CAL_RESULT_ERROR if there was an error
 *
 * @param dev (in) - device handle.
 * @param flag (in) - flag to set clock up or down.
 *
 * @return Returns CAL_RESULT_OK on success,
 *         or CAL_RESULT_ERROR if there was an error.
 *
 */
typedef CALresult (CALAPIENTRYP PFNCALDEVICECLOCKUP) (CALdevice dev, CALuint flag);

//
///  Memory Object Location (pulled directly from gsl.)
//
typedef enum CALmemLocationEnum {
    CAL_MEMORY_AGP,                     ///< Host memory in agp space
    CAL_MEMORY_SYSTEM,                  ///< Host memory in non agp space: the memory is not allocated but mapped so that the asic can access it
    CAL_MEMORY_CARD,                    ///< On board memory
    CAL_MEMORY_CARD_EXT,                ///< Extended on board memory
    CAL_MEMORY_CARD_ATI_XXX,            ///< do not use
    CAL_MEMORY_SVP,                     ///< swvap output vertex buffers have to be identified with others
    CAL_MEMORY_CLIENT,                  ///< memory in client space, it only used to optimize client vertex buffer and index buffer on IGP
    CAL_MEMORY_CARD_EXT_NONEXT,         ///< Extended on board memory, then regular as second choice
    CAL_MEMORY_CARD_EXT_NONEXT_REMOTE,  ///< Extended on board memory, then regular as second choice but letting remote memory to be used as well.
    CAL_MEMORY_REMOTE_CACHEABLE,        ///< Host memory in cacheable agp space
    CAL_MEMORY_CARD_LOCKABLE,           ///< lockable video memory
//CAL CAPS
    CAL_MEMORY_REMOTE_SHARED,           ///< gart shared memory
    CAL_MEMORY_REMOTE_SHARED_CACHEABLE, ///< gart shared/cacheable memory
    CAL_MEMORY_ALIAS,                   ///< memory owned by external lib (cal interop)
//
} CALmemLocation;

//
/// Memory Object Tiling
//
typedef enum CALmemTilingEnum {
    CAL_MEMORY_TILING_LINEAR_ALIGNED,      ///< surface linear surface, alignment restrictions
    CAL_MEMORY_TILING_TILED,               ///< surface tiled surface
    CAL_MEMORY_TILING_XT_TILED,            ///< surface extreme tiled surface (memory mapping)
    CAL_MEMORY_TILING_MACRO,               ///< surface macro tiled
    CAL_MEMORY_TILING_MICRO,               ///< surface micro tiled
    CAL_MEMORY_TILING_MICRO_SLICED,        ///< micro tiled with z slice swizzles
    CAL_MEMORY_TILING_TILED_SLICED,        ///< tiled surface with z slice swizzles
    CAL_MEMORY_TILING_3D_TILED,            ///< tiled surface with 3D tiling
    CAL_MEMORY_TILING_3D_TILED_SLICED,     ///< tiled surface with 3D tiling and z slice swizzles
    CAL_MEMORY_TILING_LINEAR_GENERAL,      ///< surface linear surface, no alignment restrictions
    CALmemTiling_FIRST = CAL_MEMORY_TILING_LINEAR_ALIGNED,         ///< first enum
    CALmemTiling_LAST  = CAL_MEMORY_TILING_LINEAR_GENERAL ///< last enum
} CALmemTiling;

//
/// Memory Object displayable attribute
//
typedef enum CALmemDisplayableEnum {
    CAL_MEMORY_DISPLAYABLE_NO,           ///< Not displayable
    CAL_MEMORY_DISPLAYABLE_LAYOUT,       ///< Request memory layout for display, but won't actually display
    CAL_MEMORY_DISPLAYABLE_YES,          ///< Request memory layout for display and actually display
} CALmemDisplayable;

//
// Memory Object Number Format
//
typedef enum CALmemNumFormatEnum {
    CAL_MEMORY_FORMAT_NORM,        ///< Normalized
    CAL_MEMORY_FORMAT_UNORM,       ///< Unnormalized
} CALmemNumFormat;

//
// Allocation Type
//
typedef enum CALmemAllocationTypeEnum {
    CAL_MEMORY_ALLOCATION_MIRRORED,    ///< Mirrored
    CAL_MEMORY_ALLOCATION_INSTANCED,   ///< Instanced
} CALmemAllocationType;

/** CAL resource information **/
typedef struct CALresInfoRec
{
    CALuint                 mappedAddress;           ///< CPU address of a mapped surface, 0 if not mapped
    CALuint                 addressHandle;           ///< Address handle
    CALuint64               address;                 ///< Device-dependent address.
    CALuint                 offset;                  ///< Surface offset of the SubMem object in bytes.
    CALuint                 alignment_xxx;           ///< Base alignment of the mem object
    CALdimension            type;                    ///< The type of the mem object
    CALmemLocation          location;                ///< Location in memory of the mem object
    CALmemAllocationType    allocationType;          ///< Mirrored or instanced allocation
    CALmemTiling            tilingFormat;            ///< Tiling format of the mem object
    CALmemDisplayable       displayable;             ///< Displayable property of the mem object
    CALboolean              mipmap;                  ///< Was the mem object created with mipmaps
    CALuint                 samples;                 ///< Number of samples for the mem object
    CALformat               format;                ///< Pixel format of the mem object
    CALmemNumFormat         numberFormat;            ///< number format
    CALboolean              shared;                  ///< Is the buffer shared?
    CALuint                 pitch;                   ///< Pitch in pixels
    CALuint                 height;                  ///< Height in pixels
    CALuint                 width;                   ///< Width in pixels
    CALuint                 layers;                  ///< Number of layers in the mem object
    CALuint                 rotation;                ///< Surface rotation
    CALuint                 byteSize;                ///< Surface size in bytes
    CALuint                 bitsPerElement;          ///< Number of bits per element (normal = 1 pixel, compressed = N pixels)
} CALresInfo;

/**
 * @fn calResQueryInfo(CALresource res, CALresInfo *resInfo)
 *
 * @brief query infomation about a specific cal resource
 *
 * returns in <i>*resInfo</i> the information regarding the resource.
 * This method returns CAL_RESULT_ERROR if there was an error
 *
 * @param res (in) - resource handle.
 * @param resInfo (out) - information about the given resource. See CALresInfo.
 *
 * @return Returns CAL_RESULT_OK on success,
 *         or CAL_RESULT_ERROR if there was an error.
 *
 */
typedef CALresult (CALAPIENTRYP PFNCALRESQUERYINFO) (CALresource res, CALresInfo *resInfo);

/**
 * @fn calResMemCopy(CALresource srcRes, CALresource dstRes, CALuint flags)
 *
 * @brief Copy resource to resource
 *
 * This method returns CAL_RESULT_ERROR if there was an error
 *
 * @param srcRes (in) - source resource handle.
 * @param dstRes (in) - destination resource handle.
 * @param flags  (in) - flags
 *
 * @return Returns CAL_RESULT_OK on success,
 *         or CAL_RESULT_ERROR if there was an error.
 *
 */
typedef CALresult (CALAPIENTRYP PFNCALRESMEMCOPY) (CALresource srcRes, CALresource dstRes, CALuint flags);

/**
 * @fn calMemCopyRaw(CALevent* event, CALcontext ctx, CALmem srcMem, CALuint srcOffset, CALmem dstMem, CALuint dstOffset, CALuint size, CALuint flags)
 *
 * @brief raw copy of memory to memory with byte offset
 *
 * This method returns CAL_RESULT_ERROR if there was an error
 *
 * @param event     (out) - event
 * @param ctx       (in)  - CAL context
 * @param srcMem    (in)  - source memory
 * @param srcOffset (in)  - byte offset into source memory
 * @param dstMem    (in)  - destination memory
 * @param dstOffset (in)  - byte offset into destination memory
 * @param size      (in)  - number of bytes to copy
 * @param flags     (in)  - input memcopy flags
 *
 */

typedef CALresult (CALAPIENTRYP PFNCALMEMCOPYRAW) (CALevent*     event,
                                                   CALcontext    ctx,
                                                   CALmem        srcMem,
                                                   CALuint       srcOffset,
                                                   CALmem        dstMem,
                                                   CALuint       dstOffset,
                                                   CALuint       size,
                                                   CALuint       flags);

/**
 * @fn calMemCopyPartial(CALevent* event, CALcontext ctx, CALmem srcMem, CALuint* srcOffset, CALmem dstMem, CALuint* dstOffset, CALuint* size, CALuint flags)
 *
 * @brief raw copy of memory to memory with byte offset
 *
 * This method returns CAL_RESULT_ERROR if there was an error
 *
 * @param event     (out) - event
 * @param ctx       (in)  - CAL context
 * @param srcMem    (in)  - source memory
 * @param srcOffset (in)  - byte/pixel offset into source memory
 * @param dstMem    (in)  - destination memory
 * @param dstOffset (in)  - byte/pixel offset into destination memory
 * @param size      (in)  - number of bytes/pixels to copy
 * @param flags     (in)  - input memcopy flags
 *
 */

typedef CALresult (CALAPIENTRYP PFNCALMEMCOPYPARTIAL) (CALevent*    event,
                                                       CALcontext   ctx,
                                                       CALmem       srcMem,
                                                       CALuint*     srcOffset,
                                                       CALmem       dstMem,
                                                       CALuint*     dstOffset,
                                                       CALuint*     size,
                                                       CALuint      flags);

/**
 * @fn calResAlloc(    CALdeviceDesc* devDesc,
 *                     CALresourceDesc* resDesc,
 *                     CALresource*     resource)
 *
 * @brief Allocate a new memory resource
 *
 * allocates a new memory resource attached to a device <i>dev</i> and returns a
 * resource handle in <i>*resource</i> if successful. This memory is structured the same
 * as the memory resource passed in.
 *
 * Implementation will allow this memory to be accessible by all contexts
 * created on this device only. Contexts residing on other devices cannot access
 * this memory.
 *
 * @param devDesc (in)      - device descriptor.
 * @param resDesc (in)      - resource descriptor.
 * @param ressource (out) - returned resource handle. On error, res will be zero.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calResFree
 */
typedef CALresult (CALAPIENTRYP PFNCALRESALLOC) (CALdeviceDesc* devDesc, const CALresourceDesc* resDesc, CALresource* resource);

//
// Heap Type
//
typedef enum CALheapTypeEnum {
    CAL_HEAP_GLOBAL,    ///< Global
} CALheapType;

/**
 * @fn calResGetHeap(CALresource*     res,
 *                   CALdeviceDesc*   devDesc,
 *                   CALheapType      type,
 *                   CALuint          size)
 *
 * @brief Get a heap
 *
 * gets a heap resource attached to a device <i>dev</i> and returns a
 * resource handle in <i>*resource</i> if successful.
 *
 * Implementation will allow this heap to be accessible by all contexts
 * created on this device only. Contexts residing on other devices cannot access
 * this heap.
 *
 * @param resDesc (out)     - resource descriptor.
 * @param devDesc (in)      - device descriptor.
 * @param type (in)         - heap type.
 * @param size (in)         - heap size - ignored for CAL_HEAP_GLOBAL heap type.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calResFree
 */
typedef CALresult (CALAPIENTRYP PFNCALRESGETHEAP) (CALresource* res, CALdeviceDesc* devDesc, CALheapType type, CALuint size);

/**
 * @fn calGetFuncInfoFromImage(CALimage         image, 
 *                             CALfuncInfo*     pFuncInfo)
 *
 * @brief get func information from image file
 *
 * gets a func information from <i>image</i> and returns the 
 * information in <i>*pFuncInfo</i> if successful.
 *
 * Implementation will allow func information accessible dependent from context
 *
 * @param pFuncInfo (out)   - information descriptor.
 * @param image (in)        - image descriptor.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 */
typedef CALresult (CALAPIENTRYP PFNGETFUNCINFOFROMIMAGE)(CALimage image, CALfuncInfo* pFuncInfo);

/**
 * @fn calConvertTextToBinary(CALvoid**  binary,
 *                          CALuint* bSize,
 *                          CALlanguage lanEnum,
 *                          const CALchar*    source)
 *
 * @brief compiles IL text to a IL bytecode
 *
 * returns a pointer to the ILbytecode binary along with the generated binary size.
 *
 * The memory for the binary and the binarysize pointer is dynamically allocated and hence needs to be released with calFreeILBinary to prevent memory leaks.
 *
 * @param binary (out)   - pointer to the ILbytecode
 * @param bSize (out)    - size fo the generated binary.
 * @param lanEnum (in)   - language type to parse for assembly.
 * @param source (in)    - string containing kernel source code.
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 */
typedef CALresult (CALAPIENTRYP PFNCALCONVERTTEXTTOBINARY)(CALvoid**  binary, CALuint* binarySize, CALlanguage lanEnum, const CALchar* source);

/**
 * @fn calConvertBinaryToText(const CALvoid* binary ,
 *                          CALuint binarySize,
 *                          CALlanguage lanEnum,
 *                          CALchar**   source)
 *
 * @brief Disassembles ILbytecode to a IL string
 *
 * returns a pointer to the IL text string.
 *
 * The memory for the Il source pointer is dynamically allocated and hence needs to be released with calFreeILBinary to prevent memory leaks.
 *
 * @param binary (in)    - pointer to the ILbytecode
 * @param binarySize (in)- size fo the generated binary.
 * @param lanEnum (in)   - language type to parse for assembly.
 * @param source (in)    - string containing kernel source code.
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 */
typedef CALresult (CALAPIENTRYP PFNCONVERTBINARYTOTEXT)(const CALvoid* binary, CALuint binarySize, CALlanguage lanEnum, CALchar** source);

/**
 * @fn calFreeTextBinary(const CALvoid* binary)
 *
 * @brief Free memory returned by the IL or the Binary pointer. This is used with calCompileILToBinary and calCompileBinaryToIL. 
 *
 *
 * The memory for the Il source pointer is dynamically allocated and hence needs to be released to prevent memory leaks.
 *
 * @param binary (in)    - pointer to the ILbytecode or ILtext
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 */
typedef CALresult (CALAPIENTRYP PFNCALFREETEXTBINARY)(CALvoid* ilbinary);

/** CALdeviceattribsExt **/
typedef struct CALdeviceattribsExtRec
{
    CALuint                 struct_size;             ///< Client filled out size of CALdeviceattribsExt struct
    CALboolean              isVMEnabled;             ///< Status of virtual addressing mode
} CALdeviceattribsExt;

/**
 * @fn calDeviceGetAttribsExt(CALdeviceattribsExt* attribsExt, CALdevice device)
 *
 * @brief Retrieve extended information for the specified device.
 *
 * Returns the device specific attributes in *attribsExt.
 *
 * @param attribsExt (out) - the device attribute struct for the specified device.
 * @param device (in) - handle of the device from which status is to be retrieved.
 *
 * @return Returns CAL_RESULT_OK on success, CAL_RESULT_ERROR if there was an error.
 *
 * @sa calDeviceGetStatus calDeviceGetAttribs 
 */
typedef CALresult (CALAPIENTRYP PFNCALDEVICEGETATTRIBSEXT) (CALdeviceattribsExt* attribsExt, CALdevice device);


//
// Video Extension
//

typedef struct CALvideoPropertiesRec    CALvideoProperties;
typedef struct CALprogramVideoRec       CALprogramVideo;
typedef struct CALdeviceVideoAttribsRec CALdeviceVideoAttribs;
typedef struct CALcontextPropertiesRec  CALcontextProperties;
typedef struct CALprogramVideoDecodeRec CALprogramVideoDecode;
typedef struct CALprogramVideoEncodeRec CALprogramVideoEncode;
typedef struct CALvideoAttribRec        CALvideoAttrib;
typedef struct CALvideoEncAttribRec     CALvideoEncAttrib;

// VCE
typedef struct CALEncodeCreateVCERec                     CALEncodeCreateVCE;
typedef struct CALEncodeGetDeviceInfoRec                 CALEncodeGetDeviceInfo;
typedef struct CALEncodeGetNumberOfModesRec              CALEncodeGetNumberOfModes;
typedef struct CALEncodeGetModesRec                      CALEncodeGetModes;
typedef struct CALEncodeGetDeviceCAPRec                  CALEncodeGetDeviceCAP;
typedef struct CALEncodeSetStateRec                      CALEncodeSetState;
typedef struct CALEncodeGetPictureControlConfigRec       CALEncodeGetPictureControlConfig;
typedef struct CALEncodeGetRateControlConfigRec          CALEncodeGetRateControlConfig;
typedef struct CALEncodeGetMotionEstimationConfigRec     CALEncodeGetMotionEstimationConfig;
typedef struct CALEncodeGetRDOControlConfigRec           CALEncodeGetRDOControlConfig;


typedef enum
{
    CAL_VID_NV12_INTERLEAVED = 1,// NV12
    CAL_VID_YV12_INTERLEAVED,   // YV12
} CALdecodeFormat;

typedef enum
{
    CAL_VID_H264_BASELINE = 1,  // H.264 bitstream acceleration baseline profile
    CAL_VID_H264_MAIN,          // H.264 bitstream acceleration main profile
    CAL_VID_H264_HIGH,          // H.264 bitstream acceleration high profile
    CAL_VID_VC1_SIMPLE,         // VC-1 bitstream acceleration simple profile
    CAL_VID_VC1_MAIN,           // VC-1 bitstream acceleration main profile
    CAL_VID_VC1_ADVANCED,       // VC-1 bitstream acceleration advanced profile
    CAL_VID_MPEG2_VLD,          // MPEG2 bitstream acceleration VLD profile
} CALdecodeProfile;

typedef enum
{
    CAL_VID_ENC_H264_BASELINE = 1,  // H.264 bitstream acceleration baseline profile
    CAL_VID_ENC_H264_MAIN,          // H.264 bitstream acceleration main profile
    CAL_VID_ENC_H264_HIGH,          // H.264 bitstream acceleration high profile  
} CALencodeProfile;

typedef enum
{
    CAL_CONTEXT_VIDEO      = 1,
    CAL_CONTEXT_3DCOMPUTE  = 2,
    CAL_CONTEXT_COMPUTE0   = 3,
    CAL_CONTEXT_COMPUTE1   = 4,
    CAL_CONTEXT_DRMDMA0    = 5,
    CAL_CONTEXT_DRMDMA1    = 6,
    CAL_CONTEXT_VIDEO_VCE,
    CALcontextEnum_FIRST = CAL_CONTEXT_VIDEO,
    CALcontextEnum_LAST  = CAL_CONTEXT_VIDEO_VCE,
} CALcontextEnum;

typedef enum
{
    CAL_PRIORITY_NEUTRAL = 0,
    CAL_PRIORITY_HIGH    = 1,
    CAL_PRIORITY_LOW     = 2
} CALpriorityEnum;


typedef enum
{
    CAL_VIDEO_DECODE = 1,
    CAL_VIDEO_ENCODE = 2
} CALvideoType;

struct CALcontextPropertiesRec
{
    CALcontextEnum name;
    CALpriorityEnum priority;
    CALvoid*       data;
};

struct CALvideoPropertiesRec
{
    CALuint          size;
    CALuint          flags;
    CALdecodeProfile profile;
    CALdecodeFormat  format;
    CALuint          width;
    CALuint          height;
    CALcontextEnum   VideoEngine_name; 
};

struct CALprogramVideoRec
{
    CALuint      size;
    CALvideoType type;
    CALuint      flags;
};

struct CALprogramVideoDecodeRec
{
    CALprogramVideo videoType;
    CALmem          outputSurface;
    void*           picture_parameter_1;
    void*           picture_parameter_2;
    CALuint         picture_parameter_2_size;
    void*           bitstream_data;
    CALuint         bitstream_data_size;
    void*           slice_data_control;
    CALuint         slice_data_size;
};

struct CALprogramVideoEncodeRec
{
    CALprogramVideo videoType;
    CALuint         pictureParam1Size;
    CALuint         pictureParam2Size;
    void*           pictureParam1;
    void*           pictureParam2;
    CALuint         uiTaskID;
};

struct CALvideoAttribRec
{
    CALdecodeProfile decodeProfile;
    CALdecodeFormat  decodeFormat;
};

struct CALvideoEncAttribRec
{
    CALencodeProfile encodeProfile;
    CALdecodeFormat  encodeFormat;  // decode format is the same as the encode format
};

struct CALdeviceVideoAttribsRec
{
    CALuint                 data_size;  // in - size of the struct,
                                        // out - bytes of data incl. pointed to
    CALuint                 max_decode_sessions;
    const CALvideoAttrib*   video_attribs;  // list of supported
                                            // profile/format pairs
    const CALvideoEncAttrib*	  video_enc_attribs; 
};

#if 1 
typedef CALresult (CALAPIENTRYP PFNCALCTXPROPERTIESCREATE)(CALcontext* pCtx, CALdevice device, CALcontextProperties* pProperties);
typedef CALresult (CALAPIENTRYP PFNCALCTXRUNPROGRAMVIDEO)(CALevent* pEvent, CALcontext ctx, CALprogramVideo* pProgramVideo);
typedef CALresult (CALAPIENTRYP PFNCALDEVICEGETVIDEOATTRIBS)(CALdeviceVideoAttribs* pAttribs, CALuint ordinal, CALuint flags);
typedef CALresult (CALAPIENTRYP PFNCALDEVICEGETCTXPROPERTIES)(CALdevice device, CALuint maxProperties, CALcontextProperties* pProperties);
typedef CALresult (CALAPIENTRYP PFNCALGETCTXPROPERTIES)(CALcontext ctx, CALuint maxProperties, CALcontextProperties* pProperties);
#else
typedef CALresult (CALAPIENTRYP PFNCALCTXPROPERTIESCREATE)(CALcontext* pCtx, CALdevice device, CALcontextProperties* pProperties);
typedef CALresult (CALAPIENTRYP PFNCALCODECCREATE)(CALcontext* pCtx, CALdevice device, CALcontextProperties* pProperties);
typedef CALresult (CALAPIENTRYP PFNCALDEVICEGETVIDEOATTRIBS)(CALcontext Ctx, CALdeviceVideoAttribs* pAttribs, CALuint flags);
#endif

////// VCE
struct CALEncodeCreateVCERec
{
    CALvoid*    VCEsession;
};

struct CALEncodeGetDeviceInfoRec
{
    unsigned int 	device_id;
    unsigned int 	max_encode_stream;
    unsigned int 	encode_cap_list_size;
};

struct CALEncodeGetNumberOfModesRec
{
    unsigned int num_of_encode_Mode;
};

typedef enum 
{
    CAL_VID_encode_MODE_NONE	= 0,
    CAL_VID_encode_AVC_FULL		= 1,
    CAL_VID_encode_AVC_ENTROPY	= 2,
} CALencodeMode;

struct CALEncodeGetModesRec
{
    CALuint   	   NumEncodeModesToRetrieve;
    CALencodeMode *pEncodeModes;
};

typedef enum
{
    CAL_VID_ENCODE_JOB_PRIORITY_NONE   = 0,
    CAL_VID_ENCODE_JOB_PRIORITY_LEVEL1 = 1,      // Always in normal queue
    CAL_VID_ENCODE_JOB_PRIORITY_LEVEL2 = 2       // possibly in low-latency queue
}   CAL_VID_ENCODE_JOB_PRIORITY;

typedef struct _CAL_VID_PROFILE_LEVEL
{
    CALuint      profile;		//based on  H.264 standard 
    CALuint      level;
} CAL_VID_PROFILE_LEVEL;

typedef enum
{
    CAL_VID_PICTURE_NONOE  = 0,
    CAL_VID_PICTURE_NV12   = 1,
} CAL_VID_PICTURE_FORMAT;

#define CAL_VID_MAX_NUM_PICTURE_FORMATS_H264_AVC	  10
#define CAL_VID_MAX_NUM_PROFILE_LEVELS_H264_AVC    20

typedef struct
{
    CALuint                       maxPicSizeInMBs;    // Max picture size in MBs
    CALuint                       minPicSizeInMBs;     // Min picture size in MBs
    CALuint                       numPictureFormats;   // number of supported picture formats
    CAL_VID_PICTURE_FORMAT        supportedPictureFormats[CAL_VID_MAX_NUM_PICTURE_FORMATS_H264_AVC]; 
    CALuint                       numProfileLevels;     // number of supported profiles/levels returne;
    CAL_VID_PROFILE_LEVEL         supportedProfileLevel[CAL_VID_MAX_NUM_PROFILE_LEVELS_H264_AVC];
    CALuint                       maxBitRate;               // Max bit rate
    CALuint                       minBitRate;                // min bit rate
    CAL_VID_ENCODE_JOB_PRIORITY   supportedJobPriority;// supported max level of job priority
}CAL_VID_ENCODE_CAPS_FULL;

typedef struct
{
    CAL_VID_ENCODE_JOB_PRIORITY  supportedJobPriority;// supported max level of job priority
    CALuint                      maxJobQueueDepth;    // Max job queue depth
}CAL_VID_ENCODE_CAPS_ENTROPY;

typedef struct
{
    CALencodeMode  EncodeModes;
    CALuint        encode_cap_size;
    union
    {
       CAL_VID_ENCODE_CAPS_FULL      *encode_cap_full;
       CAL_VID_ENCODE_CAPS_ENTROPY   *encode_cap_entropy;
       void                          *encode_cap;
    }  caps;
} CAL_VID_ENCODE_CAPS;

struct CALEncodeGetDeviceCAPRec
{
    CALuint               num_of_encode_cap;
    CAL_VID_ENCODE_CAPS  *encode_caps;
};



typedef enum
{
    CAL_VID__ENCODE_STATE_START   	= 1,
    CAL_VID__ENCODE_STATE_PAUSE   	= 2,
    CAL_VID__ENCODE_STATE_RESUME  	= 3,
    CAL_VID__ENCODE_STATE_STOP    	= 4
} CAL_VID_ENCODE_STATE   ;

typedef struct
{
    CALuint             size;                         	     // structure size

    CALuint             useConstrainedIntraPred;        // binary var - force the use of constrained intra prediction when set to 1
    //CABAC options
    CALuint             cabacEnable;                    // Enable CABAC entropy coding
    CALuint             cabacIDC;                       // cabac_init_id = 0; cabac_init_id = 1; cabac_init_id = 2;

    CALuint             loopFilterDisable;              // binary var - disable loop filter when 1 - enable loop filter when 0 (0 and 1 are the only two supported cases)
    int                encLFBetaOffset;                // -- move with loop control flag , Loop filter control, slice_beta_offset (N.B. only used if deblocking filter is not disabled, and there is no div2 as defined in the h264 bitstream syntax)
    int                encLFAlphaC0Offset;             // Loop filter control, slice_alpha_c0_offset (N.B. only used if deblocking filter is not disabled, and there is no div2 as defined in the h264 bitstream syntax)
    CALuint             encIDRPeriod;
    CALuint             encIPicPeriod;                  // spacing for I pictures, in case driver doesnt force/select a picture type, this will be used for inference
    int                encHeaderInsertionSpacing;      // spacing for inserting SPS/PPS. Example usage cases are: 0 for inserting at the beginning of the stream only, 1 for every picture, "GOP size" to align it with GOP boundaries etc. For compliance reasons, these headers might be inserted when SPS/PPS parameters change from the config packages.
    CALuint             encCropLeftOffset;
    CALuint             encCropRightOffset;
    CALuint             encCropTopOffset;
    CALuint             encCropBottomOffset;
    CALuint             encNumMBsPerSlice;              // replaces encSliceArgument - Slice control - number of MBs per slice
    CALuint             encNumSlicesPerFrame;           // Slice control - number of slices in this frame, pre-calculated to avoid DIV operation in firmware
    CALuint             encForceIntraRefresh;           // 1 serves to load intra refresh bitmap from address force_intra_refresh_bitmap_mc_addr when equal to 1, 3 also loads dirty clean bitmap on top of the intra refresh
    CALuint             encForceIMBPeriod;              // --- package with intra referesh -Intra MB spacing. if encForceIntraRefresh = 2, shifts intra refreshed MBs by frame number
    CALuint             encInsertVUIParam;              // insert VUI params in SPS
    CALuint             encInsertSEIMsg;                // insert SEI messages (bit 0 for buffering period; bit 1 for picture timing; bit 2 for pan scan)
} CAL_VID_ENCODE_PICTURE_CONTROL;

typedef struct
{
    CALuint        size;                       // structure size
    CALuint        encRateControlMethod;           // rate control method to be used
    CALuint        encRateControlTargetBitRate;    // target bit rate
    CALuint        encRateControlPeakBitRate;      // peak bit rate
    CALuint        encRateControlFrameRateNumerator;  // target frame rate
    CALuint        encGOPSize;                     // RC GOP size
    CALuint        encRCOptions;                   // packed bitfield definition for extending options here, bit 0: RC will not generate skipped frames in order to meet GOP target, bits 1-30: up for grabs by the RC alg designer
    CALuint        encQP_I;                        // I frame quantization only if rate control is disabled
    CALuint        encQP_P;                        // P frame quantization if rate control is disabled
    CALuint        encQP_B;                        // B frame quantization if rate control is disabled
    CALuint        encVBVBufferSize;               // VBV buffer size - this is CPB Size, and the default is per Table A-1 of the spec
    CALuint        encRateControlFrameRateDenominator;// target frame rate
} CAL_VID_ENCODE_RATE_CONTROL;

 // mode estimation control options
typedef struct
{
    CALuint  	        size;                   // structure size
    CALuint             imeDecimationSearch;            // decimation search is on
    CALuint             motionEstHalfPixel;             // enable half pel motion estimation
    CALuint             motionEstQuarterPixel;          // enable quarter pel motion estimation
    CALuint             disableFavorPMVPoint;           // disable favorization of PMV point
    CALuint             forceZeroPointCenter;           // force [0,0] point as search window center in IME
    CALuint             lsmVert;                        //  Luma Search window in MBs, set to either VCE_ENC_SEARCH_WIND_5x3 or VCE_ENC_SEARCH_WIND_9x5 or VCE_ENC_SEARCH_WIND_13x7
    CALuint             encSearchRangeX;                // forward prediction - Manual limiting of horizontal motion vector range (for performance) in pel resolution
    CALuint             encSearchRangeY;                // forward prediction - Manual limiting of vertical motion vector range (for performance)
    CALuint             encSearch1RangeX;               // for 2nd ref - curr IME_SEARCH_SIZE doesn't have SIZE__SEARCH1_X bitfield
    CALuint             encSearch1RangeY;               // for 2nd ref
    CALuint             disable16x16Frame1;             // second reference (B frame) limitation
    CALuint             disableSATD;                    // Disable SATD cost calculation (SAD only)
    CALuint             enableAMD;                      // FME advanced mode decision
    CALuint             encDisableSubMode;              // --- FME
    CALuint             encIMESkipX;                    // sub sample search window horz --- UENC_IME_OPTIONS.SKIP_POINT_X
    CALuint             encIMESkipY;                    // sub sample search window vert --- UENC_IME_OPTIONS.SKIP_POINT_Y
    CALuint             encEnImeOverwDisSubm;           // Enable overwriting of fme_disable_submode in IME with enabled mode number equal to ime_overw_dis_subm_no (only 8x8 and above could be enabled)
    CALuint             encImeOverwDisSubmNo;           // Numbers of mode IME will pick if en_ime_overw_dis_subm equal to 1.
    CALuint             encIME2SearchRangeX;            // IME Additional Search Window Size: horizontal 1-4 (+- this value left and right from center)
    CALuint             encIME2SearchRangeY;            // IME Additional Search Window Size: vertical not-limited (+- this value up and down from center)
                                                //   (+- this value up and down from center)
} CAL_VID_ENCODE_MOTION_ESTIMATION_CONTROL;                  // structure aligned to 88 bytes

typedef struct
{
    CALuint         size;                          		// structure size
    CALuint         encDisableTbePredIFrame;        	// Disable Prediction Modes For I-Frames
    CALuint         encDisableTbePredPFrame;        	// same as above for P frames
    CALuint         useFmeInterpolY;                	// zero_residues_luma
    CALuint         useFmeInterpolUV;               	// zero_residues_chroma
    CALuint         enc16x16CostAdj;                	// --- UENC_FME_MD.M16x16_COST_ADJ
    CALuint         encSkipCostAdj;                 	// --- UENC_FME_MD.MSkip_COST_ADJ
    unsigned char  encForce16x16skip;
} CAL_VID_ENCODE_RDO_CONTROL;


struct CALEncodeSetStateRec
{
    CAL_VID_ENCODE_STATE  encode_states;
};
struct CALEncodeGetPictureControlConfigRec
{
    CAL_VID_ENCODE_PICTURE_CONTROL  encode_picture_control;
};
struct CALEncodeGetRateControlConfigRec
{
    CAL_VID_ENCODE_RATE_CONTROL  encode_rate;
};
struct CALEncodeGetMotionEstimationConfigRec
{
    CAL_VID_ENCODE_MOTION_ESTIMATION_CONTROL  encode_motion_estimation;
};
struct CALEncodeGetRDOControlConfigRec
{
    CAL_VID_ENCODE_RDO_CONTROL  encode_RDO;
};

typedef enum
{
    CAL_VID_CONFIG_TYPE_NONE 			  = 0,
    CAL_VID_CONFIG_TYPE_PICTURECONTROL 	  = 1,
    CAL_VID_CONFIG_TYPE_RATECONTROL		  = 2,
    CAL_VID_CONFIG_TYPE_MOTIONSESTIMATION = 3,
    CAL_VID_CONFIG_TYPE_RDO 			  = 4
} CAL_VID_CONFIG_TYPE;

typedef struct
{
    CAL_VID_CONFIG_TYPE                    configType;
    union
    {
        CAL_VID_ENCODE_PICTURE_CONTROL*            pPictureControl;
        CAL_VID_ENCODE_RATE_CONTROL*               pRateControl;
        CAL_VID_ENCODE_MOTION_ESTIMATION_CONTROL*  pMotionEstimation;
        CAL_VID_ENCODE_RDO_CONTROL*                pRDO;
    }    config;
} CAL_VID_CONFIG;

typedef enum
{
    CAL_VID_PICTURE_STRUCTURE_H264_NONE         = 0,
    CAL_VID_PICTURE_STRUCTURE_H264_FRAME        = 1, 
    CAL_VID_PICTURE_STRUCTURE_H264_TOP_FIELD    = 2,
    CAL_VID_PICTURE_STRUCTURE_H264_BOTTOM_FIELD = 3
} CAL_VID_PICTURE_STRUCTURE_H264;

// Used to force picture type
typedef enum _CU_VID_PICTURE_TYPE_H264
{
    CAL_VID_PICTURE_TYPE_H264_NONE                  = 0,
    CAL_VID_PICTURE_TYPE_H264_SKIP                  = 1,
    CAL_VID_PICTURE_TYPE_H264_IDR                   = 2,
    CAL_VID_PICTURE_TYPE_H264_I                     = 3,
    CAL_VID_PICTURE_TYPE_H264_P                     = 4
} CAL_VID_PICTURE_TYPE_H264;

typedef union _CAL_VID_ENCODE_PARAMETERS_H264_FLAGS
{
    struct
    {
        // enable/disable features
        unsigned int                            reserved    : 32;               // reserved fields must be set to zero
    }                                           flags;
    unsigned int                                value;
}CAL_VID_ENCODE_PARAMETERS_H264_FLAGS;

typedef struct
{
    CALuint                              size;                           // structure size. Must be always set to the size of AVE_ENCODE_PARAMETERS_H264.

    CAL_VID_ENCODE_PARAMETERS_H264_FLAGS flags;                          // enable/disable any supported features

    CALboolean                           insertSPS;    
    CAL_VID_PICTURE_STRUCTURE_H264       pictureStructure;
    CALboolean                           forceRefreshMap;
    CALuint                              forceIMBPeriod;
    CAL_VID_PICTURE_TYPE_H264            forcePicType;
} CAL_VID_ENCODE_PARAMETERS_H264;

typedef enum
{
    CAL_VID_BUFFER_TYPE_NONE                 = 0,
    CAL_VID_BUFFER_TYPE_PICTURE              = 2,
    CAL_VID_BUFFER_TYPE_INTRA_REFRESH_BITMAP = 3,
    CAL_VID_BUFFER_TYPE_DIRTY_CLEAN_BITMAP   = 4,
    CAL_VID_BUFFER_TYPE_SLICE_HEADER         = 5,
    CAL_VID_BUFFER_TYPE_SLICE                = 6
} CAL_VID_BUFFER_TYPE;

#define CAL_VID_SURFACE_HANDLE                      void* 

typedef struct
{
    CAL_VID_BUFFER_TYPE           bufferType;
    union
    {
        CALmem                    pPicture;
        CAL_VID_SURFACE_HANDLE    pIntraRefreshBitmap;
        CAL_VID_SURFACE_HANDLE    pDirtyCleanBitmap;
        CAL_VID_SURFACE_HANDLE    pSliceHeader;
        CAL_VID_SURFACE_HANDLE    pSlice;
    }   buffer;
} CAL_VID_BUFFER_DESCRIPTION;

typedef enum
{
    CAL_VID_TASK_STATUS_NONE        = 0,
    CAL_VID_TASK_STATUS_COMPLETE    = 1,	 // encoding task has finished successfully.
    CAL_VID_TASK_STATUS_FAILED      = 2         	 // encoding task has finished but failed. 
} CAL_VID_TASK_STATUS;

typedef struct
{
    CALuint                 size;                         // structure size
    CALuint                 taskID;                    // task ID
    CAL_VID_TASK_STATUS    status;                       // Task status. May be duplicated if current task has multiple output blocks.
    CALuint                 size_of_bitstream_data;                       // data size of the output block 
    void*                  bitstream_data;    // read pointer the top portion of the generated bitstream data for the current task
} CAL_VID_OUTPUT_DESCRIPTION;


typedef CALresult (CALAPIENTRYP PFNCALENCODECREATEVCE)(CALcontext pCtx, CALEncodeCreateVCE* pEncodeVCE);
typedef CALresult (CALAPIENTRYP PFNCALENCODEDESTROYVCE)(CALcontext pCtx);
typedef CALresult (CALAPIENTRYP PFNCALENCODEGETDEVICEINFO)(CALcontext pCtx, CALEncodeGetDeviceInfo* pEncodeDeviceInfo);
typedef CALresult (CALAPIENTRYP PFNCALENCODEGETNUMBEROFMODES)(CALcontext pCtx, CALEncodeGetNumberOfModes* pEncodeNumberOfModes);
typedef CALresult (CALAPIENTRYP PFNCALENCODEGETMODES)(CALcontext pCtx, CALuint device_id, CALuint NumEncodeModesToRetrieve,CALEncodeGetModes* pEncodeModes);
typedef CALresult (CALAPIENTRYP PFNCALENCODEGETDEVICECAP)(CALcontext pCtx, CALuint device_id, CALuint encode_cap_total_size, CALEncodeGetDeviceCAP *pEncodeCAP);

typedef CALresult (CALAPIENTRYP PFNCALENCODECREATESESSION)(CALcontext ctx, CALuint device_id, 
                                                   CALencodeMode encode_mode, CAL_VID_PROFILE_LEVEL encode_profile_level,
                                                   CAL_VID_PICTURE_FORMAT encode_formatm, 
                                                   CALuint encode_width, CALuint encode_height,
                                                   CALuint frameRateNum, CALuint frameRateDenom, CAL_VID_ENCODE_JOB_PRIORITY  encode_priority_level);
typedef CALresult (CALAPIENTRYP PFNCALENCODECLOSESESSION)(CALcontext ctx, CALuint device_id);

typedef CALresult (CALAPIENTRYP PFNCALENCODESETSTATE)(CALcontext pCtx, CALEncodeSetState pEncodState);
typedef CALresult (CALAPIENTRYP PFNCALENCODEGETPICTURECONTROLCONFIG)(CALcontext pCtx, CALEncodeGetPictureControlConfig *pPictureControlConfig);
typedef CALresult (CALAPIENTRYP PFNCALENCODEGETRATECONTROLCONFIG)(CALcontext pCtx, CALEncodeGetRateControlConfig *pRateControlConfig);
typedef CALresult (CALAPIENTRYP PFNCALENCODEGETMOTIONESTIMATIONCONFIG)(CALcontext pCtx, CALEncodeGetMotionEstimationConfig *pMotionEstimationConfig);
typedef CALresult (CALAPIENTRYP PFNCALENCODEGETRDOCONTROLCONFIG)(CALcontext pCtx, CALEncodeGetRDOControlConfig *pRDOConfig);

typedef CALresult (CALAPIENTRYP PFNCALENCODESENDCONFIG)(CALcontext pCtx, CALuint num_of_config_buffers, CAL_VID_CONFIG *pConfigBuffers);
typedef CALresult (CALAPIENTRYP PFNCALENCODEPICTURE)(CALcontext pCtx, CALuint num_of_encode_task_input_buffer, CAL_VID_BUFFER_DESCRIPTION *encode_task_input_buffer_list, void *picture_parameter, CALuint *pTaskID);
typedef CALresult (CALAPIENTRYP PFNCALENCODEQUERYTASKDESCRIPTION)(CALcontext pCtx, CALuint num_of_task_description_request, CALuint *num_of_task_description_return, CAL_VID_OUTPUT_DESCRIPTION *task_description_list);
typedef CALresult (CALAPIENTRYP PFNCALENCODERELEASEOUTPUTRESOURCE)(CALcontext pCtx, CALuint taskID);


// Maximum number of atomic counters
#define CAL_MAX_ATOMIC_COUNTER  11

typedef enum CalAtomicCounterSyncModeEnum
{
    CalAtomicCounterSyncWrite,
    CalAtomicCounterSyncRead
} CalAtomicCounterSyncMode;

/**
 * @fn calReadAtomicCounter(CALdevice dev, CALuint index, CALuint* value)
 *
 * @brief Read the value from a atomic counter.
 *
 * This method returns CAL_RESULT_BAD_HANDLE if dev is invalid.
 * This method returns CAL_RESULT_INVALID_PARAMETER if index is not
 * in [0...MAX_ATOMIC_COUNTER-1]
 *
 * @param dev (in) - device.
 * @param index (in) - index.
 * @param value (out) - the value of atomic counter.
 *
 * @return Returns CAL_RESULT_OK on success,
 *         CAL_RESULT_BAD_HANDLE or CAL_RESULT_INVALID_PARAMETER
 *         if there was an error.
 *
 */
typedef CALresult (CALAPIENTRYP PFNCALREADATOMICCOUNTER) (CALdevice dev, CALuint index, CALuint* value);

/**
 * @fn calWriteAtomicCounter(CALdevice dev, CALuint index, CALuint value)
 *
 * @brief Write the value to a atomic counter.
 *
 * This method returns CAL_RESULT_BAD_HANDLE if dev is invalid.
 * This method returns CAL_RESULT_INVALID_PARAMETER if index is not
 * in [0...MAX_ATOMIC_COUNTER-1]
 *
 * @param dev (in) - device.
 * @param index (in) - index.
 * @param value (in) - the value to write.
 *
 * @return Returns CAL_RESULT_OK on success,
 *         CAL_RESULT_BAD_HANDLE or CAL_RESULT_INVALID_PARAMETER
 *         if there was an error.
 *
 */
typedef CALresult (CALAPIENTRYP PFNCALWRITEATOMICCOUNTER) (CALdevice dev, CALuint index, CALuint value);

/**
 * @fn calBindAtomicCounter(CALdevice dev, CALuint index, CALmem handle)
 *
 * @brief Bind a atomic counter to a memory handle.
 *
 * This method returns CAL_RESULT_BAD_HANDLE if dev is invalid.
 * This method returns CAL_RESULT_INVALID_PARAMETER if index is not
 * in [0...MAX_ATOMIC_COUNTER-1]
 *
 * @param dev (in) - device.
 * @param index (in) - index.
 * @param handle (in) - the memory to bind.
 *
 * @return Returns CAL_RESULT_OK on success,
 *         CAL_RESULT_BAD_HANDLE or CAL_RESULT_INVALID_PARAMETER
 *         if there was an error.
 *
 */
typedef CALresult (CALAPIENTRYP PFNCALBINDATOMICCOUNTER) (CALcontext ctx, CALuint index, CALmem handle);

/**
 * @fn calSyncAtomicCounter(CALevent* event, CALdevice dev, CALuint index, 
 *                          CalAtomicCounterSyncMode sync)
 *
 * @brief Invoke a sync between atomic counter and its bound memory.
 *
 * This method returns CAL_RESULT_BAD_HANDLE if dev is invalid.
 * This method returns CAL_RESULT_INVALID_PARAMETER if index is not
 * in [0...MAX_ATOMIC_COUNTER-1]
 *
 * @param event (out) - event associated with atomic counter.
 * @param dev (in) - device.
 * @param index (in) - index.
 * @param sync (in) - the type of sync action.
 *
 * @return Returns CAL_RESULT_OK on success,
 *         CAL_RESULT_BAD_HANDLE or CAL_RESULT_INVALID_PARAMETER
 *         if there was an error.
 *
 */
typedef CALresult (CALAPIENTRYP PFNCALSYNCATOMICCOUNTER) (CALevent* event, CALcontext ctx, CALuint index, CalAtomicCounterSyncMode sync);

#ifdef __cplusplus
}
#endif
#endif // __CAL_PRIVATE_EXT_H__



