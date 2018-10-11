//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afTreeItemType.h
///
//==================================================================================

#ifndef __AFTREEITEMTYPE
#define __AFTREEITEMTYPE

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:          afTreeItemType
// General Description: Enumeration for monitored tree object type
// Author:              Sigal Algranaty
// Creation Date:       26/9/2010
// ----------------------------------------------------------------------------------
enum afTreeItemType
{
    AF_TREE_ITEM_ITEM_NONE = -1,

    AF_TREE_ITEM_APP_ROOT,

    AF_TREE_ITEM_MESSAGE,

    AF_TREE_ITEM_PROFILE_SESSION,
    AF_TREE_FIRST_PROFILE_ITEM_TYPE = AF_TREE_ITEM_PROFILE_SESSION,

    AF_TREE_ITEM_PROFILE_SESSION_TYPE,

    AF_TREE_ITEM_PROFILE_GPU_SUMMARY,
    AF_TREE_ITEM_PROFILE_GPU_API_SUMMARY,
    AF_TREE_ITEM_PROFILE_GPU_FIRST_SUMMARY_ITEM_TYPE = AF_TREE_ITEM_PROFILE_GPU_API_SUMMARY,
    AF_TREE_ITEM_PROFILE_GPU_CONTEXT_SUMMARY,
    AF_TREE_ITEM_PROFILE_KERNEL_SUMMARY,
    AF_TREE_ITEM_PROFILE_GPU_TOP10_TRANSFER_SUMMARY,
    AF_TREE_ITEM_PROFILE_GPU_TOP10_KERNEL_SUMMARY,
    AF_TREE_ITEM_PROFILE_GPU_BEST_PRACTICE_SUMMARY,
    AF_TREE_ITEM_PROFILE_GPU_LAST_SUMMARY_ITEM_TYPE = AF_TREE_ITEM_PROFILE_GPU_BEST_PRACTICE_SUMMARY,
    AF_TREE_ITEM_PROFILE_GPU_KERNELS,
    AF_TREE_ITEM_PROFILE_GPU_KERNEL,

    AF_TREE_LAST_PROFILE_ITEM_TYPE = AF_TREE_ITEM_PROFILE_GPU_KERNEL,
    AF_TREE_ITEM_KA_FILE,
    AF_TREE_FIRST_KA_ITEM_TYPE = AF_TREE_ITEM_KA_FILE,
    AF_TREE_ITEM_KA_EXE_FILE,
    AF_TREE_ITEM_KA_OVERVIEW,
    AF_TREE_ITEM_KA_SOURCE,
    AF_TREE_ITEM_KA_KERNEL,
    AF_TREE_ITEM_KA_STATISTICS,
    AF_TREE_ITEM_KA_ANALYSIS,
    AF_TREE_ITEM_KA_DEVICE,
    AF_TREE_ITEM_KA_HISTORY,
    AF_TREE_ITEM_KA_ADD_FILE,
    AF_TREE_ITEM_KA_NEW_FILE,
    AF_TREE_ITEM_KA_SOURCES,
    AF_TREE_ITEM_KA_OUT_DIR,
    AF_TREE_ITEM_KA_REF_TYPE,
    AF_TREE_ITEM_KA_NEW_PROGRAM,
    AF_TREE_ITEM_KA_PROGRAM,
    AF_TREE_ITEM_KA_PROGRAM_GL_GEOM,
    AF_TREE_ITEM_KA_FIRST_FILE_ITEM_TYPE = AF_TREE_ITEM_KA_PROGRAM_GL_GEOM,
    AF_TREE_ITEM_KA_PROGRAM_GL_FRAG,
    AF_TREE_ITEM_KA_PROGRAM_GL_TESC,
    AF_TREE_ITEM_KA_PROGRAM_GL_TESE,
    AF_TREE_ITEM_KA_PROGRAM_GL_VERT,
    AF_TREE_ITEM_KA_PROGRAM_GL_COMP,
    AF_TREE_ITEM_KA_PROGRAM_SHADER,
    AF_TREE_ITEM_KA_LAST_FILE_ITEM_TYPE = AF_TREE_ITEM_KA_PROGRAM_SHADER,

    AF_TREE_LAST_KA_ITEM_TYPE = AF_TREE_ITEM_KA_PROGRAM,

    // TO_DO: VS textures and buffers viewer Add the types added for the textures and buffers viewer!
    // AF_TREE_ITEM_GL_FBO_ATTACHMENT, AF_TREE_ITEM_GL_PBUFFER_NODE
    // For _fboAttachmentFBOName add and fill details of _fboAttachmentFBOName
    // OpenGL:
    AF_TREE_ITEM_GL_RENDER_CONTEXT,
    AF_TREE_FIRST_DEBUGGER_ITEM_TYPE = AF_TREE_ITEM_GL_RENDER_CONTEXT,
    AF_TREE_ITEM_GL_TEXTURES_NODE,
    AF_TREE_ITEM_GL_TEXTURE,
    AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE,
    AF_TREE_ITEM_GL_RENDER_BUFFER,
    AF_TREE_ITEM_GL_PROGRAM_PIPELINES_NODE,
    AF_TREE_ITEM_GL_PROGRAM_PIPELINE,
    AF_TREE_ITEM_GL_SAMPLERS_NODE,
    AF_TREE_ITEM_GL_SAMPLER,
    AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE,
    AF_TREE_ITEM_GL_STATIC_BUFFER,
    AF_TREE_ITEM_GL_PBUFFERS_NODE,
    AF_TREE_ITEM_GL_PBUFFER_NODE,
    AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER,
    AF_TREE_ITEM_GL_SYNC_OBJECTS_NODE,
    AF_TREE_ITEM_GL_SYNC_OBJECT,
    AF_TREE_ITEM_GL_VBO_NODE,
    AF_TREE_ITEM_GL_VBO,
    AF_TREE_ITEM_GL_PROGRAMS_NODE,
    AF_TREE_ITEM_GL_PROGRAM,
    AF_TREE_ITEM_GL_SHADERS_NODE,
    AF_TREE_ITEM_GL_VERTEX_SHADER,
    AF_TREE_ITEM_GL_TESSELLATION_CONTROL_SHADER,
    AF_TREE_ITEM_GL_TESSELLATION_EVALUATION_SHADER,
    AF_TREE_ITEM_GL_GEOMETRY_SHADER,
    AF_TREE_ITEM_GL_FRAGMENT_SHADER,
    AF_TREE_ITEM_GL_COMPUTE_SHADER,
    AF_TREE_ITEM_GL_UNSUPPORTED_SHADER,
    AF_TREE_ITEM_GL_DISPLAY_LISTS_NODE,
    AF_TREE_ITEM_GL_DISPLAY_LIST,
    AF_TREE_ITEM_GL_FBO_NODE,
    AF_TREE_ITEM_GL_FBO,
    AF_TREE_ITEM_GL_FBO_ATTACHMENT,

    // OpenCL:
    AF_TREE_ITEM_CL_CONTEXT,
    AF_TREE_ITEM_CL_IMAGES_NODE,
    AF_TREE_ITEM_CL_IMAGE,
    AF_TREE_ITEM_CL_BUFFERS_NODE,
    AF_TREE_ITEM_CL_BUFFER,
    AF_TREE_ITEM_CL_SUB_BUFFER,
    AF_TREE_ITEM_CL_PIPES_NODE,
    AF_TREE_ITEM_CL_PIPE,
    AF_TREE_ITEM_CL_SAMPLERS_NODE,
    AF_TREE_ITEM_CL_SAMPLER,
    AF_TREE_ITEM_CL_EVENTS_NODE,
    AF_TREE_ITEM_CL_EVENT,
    AF_TREE_ITEM_CL_COMMAND_QUEUES_NODE,
    AF_TREE_ITEM_CL_COMMAND_QUEUE,
    AF_TREE_ITEM_CL_DEVICE,
    AF_TREE_ITEM_CL_PLATFORM,
    AF_TREE_ITEM_CL_PROGRAMS_NODE,
    AF_TREE_ITEM_CL_PROGRAM,
    AF_TREE_ITEM_CL_KERNEL,
    AF_TREE_ITEM_CL_KERNEL_VARIABLE,
    AF_TREE_LAST_DEBUGGER_ITEM_TYPE = AF_TREE_ITEM_CL_KERNEL_VARIABLE,

    AF_TREE_ITEM_OBJECT_TYPES_AMOUNT
};


// Contain enumerations for an item load failure:
enum AF_API afItemLoadFailureDescription
{
    AF_ITEM_ERROR_UNKNOWN,                      // No error:
    AF_ITEM_LOAD_GLBEGIN_END_BLOCK,             // Item was loaded while in glBegin-glEnd block:
    AF_ITEM_LOAD_PROCESS_IS_RUNNING,            // Item was loaded while process is running:
    AF_ITEM_LOAD_PROCESS_IS_TERMINATED,         // Item was loaded while process is terminated:
    AF_ITEM_LOAD_TEXTURE_TYPE_UNKNOWN,          // Item was not loaded - a texture with an unknown type:
    AF_ITEM_LOAD_IMAGE_TYPE_UNKNOWN,            // Item was not loaded - an image with an unknown type:
    AF_ITEM_LOAD_STATIC_BUFFER_BOUND_TO_FBO,    // Item was not loaded - it is bound to an FBO:
    AF_ITEM_LOAD_VARIABLE_LOAD_FAILURE,         // Item was not loaded - kernel variable data failed to load
    AF_ITEM_LOAD_EMPTY_THUMBS,                  // Item was not loaded - there were no available items
    AF_ITEM_LOAD_CL_GL_INTEROP                  // Item was not loaded - GL-CL interoperability items are not supported currently
};

enum AF_API afItemLoadStatusType
{

    AF_ITEM_LOAD_SUCCESS,               // Item was loaded successfully:
    AF_ITEM_LOAD_STALE,                 // Item was loaded successfully, but it contain stale data
    AF_ITEM_LOAD_ERROR,                 // There was an error in the item load process:
    AF_ITEM_NOT_LOADED,                 // Item was not loaded:
    AF_ITEM_PAGE_NOT_LOADED,            // An item page was not loaded:
    AF_ITEM_PAGE_LOAD_ERROR             // There was an item load error:
};

// Status for single item load:
struct AF_API afItemLoadStatus
{
    // Contain the item load status:
    afItemLoadStatusType _itemLoadStatusType;

    // Contain the load description. On success this string should be empty. On failure - should contain the description:
    afItemLoadFailureDescription _loadStatusDescription;
};

#endif  // __AFTREEITEMTYPE
