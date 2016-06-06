//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenGLExtensionsId.cpp
///
//==================================================================================

//------------------------------ apOpenGLExtensionsId.cpp ------------------------------

// Standard C:
#include <string.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apOpenGLExtensionsId.h>


// ---------------------------------------------------------------------------
// Name:        apOpenGLExtensionsIdToString
// Description:
//   Inputs an extension id and outputs the extension name as a string.
//
// Arguments: extensionId - The extension id.
//            extensionString - The extension string, as appears in
//                              glGetString(GL_EXTENSIONS) or glGetStringi(GL_EXTENSIONS, i)
//
// Return Val: bool  - Success / failure.
//
// Author:  AMD Developer Tools Team
// Date:        22/2/2006
// ---------------------------------------------------------------------------
bool apOpenGLExtensionsIdToString(apOpenGLExtensionsId extensionId, gtString& extensionString)
{
    bool retVal = true;

    switch (extensionId)
    {
        case AP_GL_ARB_color_buffer_float:
            extensionString = L"GL_ARB_color_buffer_float";
            break;

        case AP_GL_ARB_compatibility:
            extensionString = L"GL_ARB_compatibility";
            break;

        case AP_GL_ARB_depth_buffer_float:
            extensionString = L"GL_ARB_depth_buffer_float";
            break;

        case AP_GL_ARB_depth_texture:
            extensionString = L"GL_ARB_depth_texture";
            break;

        case AP_GL_ARB_draw_buffers:
            extensionString = L"GL_ARB_draw_buffers";
            break;

        case AP_GL_ARB_draw_instanced:
            extensionString = L"GL_ARB_draw_instanced";
            break;

        case AP_GL_ARB_fragment_program:
            extensionString = L"GL_ARB_fragment_program";
            break;

        case AP_GL_ARB_fragment_program_shadow:
            extensionString = L"GL_ARB_fragment_program_shadow";
            break;

        case AP_GL_ARB_fragment_shader:
            extensionString = L"GL_ARB_fragment_shader";
            break;

        case AP_GL_ARB_framebuffer_sRGB:
            extensionString = L"GL_ARB_framebuffer_sRGB";
            break;

        case AP_GL_ARB_geometry_shader4:
            extensionString = L"GL_ARB_geometry_shader4";
            break;

        case AP_GL_ARB_half_float_pixel:
            extensionString = L"GL_ARB_half_float_pixel";
            break;

        case AP_GL_ARB_half_float_vertex:
            extensionString = L"GL_ARB_half_float_vertex";
            break;

        case AP_GL_ARB_instanced_arrays:
            extensionString = L"GL_ARB_instanced_arrays";
            break;

        case AP_GL_ARB_map_buffer_range:
            extensionString = L"GL_ARB_map_buffer_range";
            break;

        case AP_GL_ARB_matrix_palette:
            extensionString = L"GL_ARB_matrix_palette";
            break;

        case AP_GL_ARB_multisample:
            extensionString = L"GL_ARB_multisample";
            break;

        case AP_GL_ARB_multitexture:
            extensionString = L"GL_ARB_multitexture";
            break;

        case AP_GL_ARB_occlusion_query:
            extensionString = L"GL_ARB_occlusion_query";
            break;

        case AP_GL_ARB_pixel_buffer_object:
            extensionString = L"GL_ARB_pixel_buffer_object";
            break;

        case AP_GL_ARB_point_parameters:
            extensionString = L"GL_ARB_point_parameters";
            break;

        case AP_GL_ARB_point_sprite:
            extensionString = L"GL_ARB_point_sprite";
            break;

        case AP_GL_ARB_shader_objects:
            extensionString = L"GL_ARB_shader_objects";
            break;

        case AP_GL_ARB_shading_language_100:
            extensionString = L"GL_ARB_shading_language_100";
            break;

        case AP_GL_ARB_shadow:
            extensionString = L"GL_ARB_shadow";
            break;

        case AP_GL_ARB_shadow_ambient:
            extensionString = L"GL_ARB_shadow_ambient";
            break;

        case AP_GL_ARB_texture_border_clamp:
            extensionString = L"GL_ARB_texture_border_clamp";
            break;

        case AP_GL_ARB_texture_buffer_object:
            extensionString = L"GL_ARB_texture_buffer_object";
            break;

        case AP_GL_ARB_texture_compression:
            extensionString = L"GL_ARB_texture_compression";
            break;

        case AP_GL_ARB_texture_compression_rgtc:
            extensionString = L"GL_ARB_texture_compression_rgtc";
            break;

        case AP_GL_ARB_texture_cube_map:
            extensionString = L"GL_ARB_texture_cube_map";
            break;

        case AP_GL_ARB_texture_env_add:
            extensionString = L"GL_ARB_texture_env_add";
            break;

        case AP_GL_ARB_texture_env_combine:
            extensionString = L"GL_ARB_texture_env_combine";
            break;

        case AP_GL_ARB_texture_env_crossbar:
            extensionString = L"GL_ARB_texture_env_crossbar";
            break;

        case AP_GL_ARB_texture_env_dot3:
            extensionString = L"GL_ARB_texture_env_dot3";
            break;

        case AP_GL_ARB_texture_float:
            extensionString = L"GL_ARB_texture_float";
            break;

        case AP_GL_ARB_texture_mirrored_repeat:
            extensionString = L"GL_ARB_texture_mirrored_repeat";
            break;

        case AP_GL_ARB_texture_non_power_of_two:
            extensionString = L"GL_ARB_texture_non_power_of_two";
            break;

        case AP_GL_ARB_texture_rectangle:
            extensionString = L"GL_ARB_texture_rectangle";
            break;

        case AP_GL_ARB_texture_rg:
            extensionString = L"GL_ARB_texture_rg";
            break;

        case AP_GL_ARB_transpose_matrix:
            extensionString = L"GL_ARB_transpose_matrix";
            break;

        case AP_GL_ARB_vertex_array_object:
            extensionString = L"GL_ARB_vertex_array_object";
            break;

        case AP_GL_ARB_uniform_buffer_object:
            extensionString = L"GL_ARB_uniform_buffer_object";
            break;

        case AP_GL_ARB_copy_buffer:
            extensionString = L"GL_ARB_copy_buffer";
            break;

        case AP_GL_ARB_depth_clamp:
            extensionString = L"GL_ARB_depth_clamp";
            break;

        case AP_GL_ARB_draw_elements_base_vertex:
            extensionString = L"GL_ARB_draw_elements_base_vertex";
            break;

        case AP_GL_ARB_fragment_coord_conventions:
            extensionString = L"GL_ARB_fragment_coord_conventions";
            break;

        case AP_GL_ARB_provoking_vertex:
            extensionString = L"GL_ARB_provoking_vertex";
            break;

        case AP_GL_ARB_seamless_cube_map:
            extensionString = L"GL_ARB_seamless_cube_map";
            break;

        case AP_GL_ARB_sync:
            extensionString = L"GL_ARB_sync";
            break;

        case AP_GL_ARB_texture_multisample:
            extensionString = L"GL_ARB_texture_multisample";
            break;

        case AP_GL_ARB_vertex_array_bgra:
            extensionString = L"GL_ARB_vertex_array_bgra";
            break;

        case AP_GL_ARB_vertex_blend:
            extensionString = L"GL_ARB_vertex_blend";
            break;

        case AP_GL_ARB_vertex_buffer_object:
            extensionString = L"GL_ARB_vertex_buffer_object";
            break;

        case AP_GL_ARB_vertex_program:
            extensionString = L"GL_ARB_vertex_program";
            break;

        case AP_GL_ARB_vertex_shader:
            extensionString = L"GL_ARB_vertex_shader";
            break;

        case AP_GL_ARB_window_pos:
            extensionString = L"GL_ARB_window_pos";
            break;

        case AP_GL_ARB_debug_output:
            extensionString = L"GL_ARB_debug_output";
            break;

        case AP_GL_KHR_debug:
            extensionString = L"GL_KHR_debug";
            break;

        case AP_GL_ATI_draw_buffers:
            extensionString = L"GL_ATI_draw_buffers";
            break;

        case AP_GL_ATI_fragment_shader:
            extensionString = L"GL_ATI_fragment_shader";
            break;

        case AP_GL_ATI_text_fragment_shader:
            extensionString = L"GL_ATI_text_fragment_shader";
            break;

        case AP_GL_EXT_bgra:
            extensionString = L"GL_EXT_bgra";
            break;

        case AP_GL_EXT_blend_logic_op:
            extensionString = L"GL_EXT_blend_logic_op";
            break;

        case AP_GL_EXT_blend_minmax:
            extensionString = L"GL_EXT_blend_minmax";
            break;

        case AP_GL_EXT_compiled_vertex_array:
            extensionString = L"GL_EXT_compiled_vertex_array";
            break;

        case AP_GL_EXT_blend_subtract:
            extensionString = L"GL_EXT_blend_subtract";
            break;

        case AP_GL_EXT_draw_instanced:
            extensionString = L"GL_EXT_draw_instanced";
            break;

        case AP_GL_EXT_texture_array:
            extensionString = L"GL_EXT_texture_array";
            break;

        case AP_GL_EXT_texture_buffer_object:
            extensionString = L"GL_EXT_texture_buffer_object";
            break;

        case AP_GL_EXT_framebuffer_object:
            extensionString = L"GL_EXT_framebuffer_object";
            break;

        case AP_GL_EXT_framebuffer_blit:
            extensionString = L"GL_EXT_framebuffer_blit";
            break;

        case AP_GL_EXT_framebuffer_multisample:
            extensionString = L"GL_EXT_framebuffer_multisample";
            break;

        case AP_GL_EXT_geometry_shader4:
            extensionString = L"GL_EXT_geometry_shader4";
            break;

        case AP_GL_EXT_multi_draw_arrays:
            extensionString = L"GL_EXT_multi_draw_arrays";
            break;

        case AP_GL_EXT_packed_pixels:
            extensionString = L"GL_EXT_packed_pixels";
            break;

        case AP_GL_EXT_stencil_two_side:
            extensionString = L"GL_EXT_stencil_two_side";
            break;

        case AP_GL_EXT_texture:
            extensionString = L"GL_EXT_texture";
            break;

        case AP_GL_EXT_texture_shared_exponent:
            extensionString = L"GL_EXT_texture_shared_exponent";
            break;

        case AP_GL_EXT_texture3D:
            extensionString = L"GL_EXT_texture3D";
            break;

        case AP_GL_EXT_vertex_shader:
            extensionString = L"GL_EXT_vertex_shader";
            break;

        case AP_GL_GREMEDY_string_marker:
            extensionString = L"GL_GREMEDY_string_marker";
            break;

        case AP_GL_GREMEDY_frame_terminator:
            extensionString = L"AP_GL_GREMEDY_frame_terminator";
            break;

        case AP_GL_HP_occlusion_test:
            extensionString = L"GL_HP_occlusion_test";
            break;

        case AP_GL_NV_fragment_program:
            extensionString = L"GL_NV_fragment_program";
            break;

        case AP_GL_NV_fragment_program_option:
            extensionString = L"GL_NV_fragment_program_option";
            break;

        case AP_GL_NV_fragment_program2:
            extensionString = L"GL_NV_fragment_program2";
            break;

        case AP_GL_NV_geometry_shader4:
            extensionString = L"GL_NV_geometry_shader4";
            break;

        case AP_GL_NV_occlusion_query:
            extensionString = L"GL_NV_occlusion_query";
            break;

        case AP_GL_NV_primitive_restart:
            extensionString = L"GL_NV_primitive_restart";
            break;

        case AP_GL_NV_texgen_reflection:
            extensionString = L"GL_NV_texgen_reflection";
            break;

        case AP_GL_NV_texture_rectangle:
            extensionString = L"GL_NV_texture_rectangle";
            break;

        case AP_GL_NV_texture_shader:
            extensionString = L"GL_NV_texture_shader";
            break;

        case AP_GL_NV_texture_shader3:
            extensionString = L"GL_NV_texture_shader3";
            break;

        case AP_GL_NV_vertex_program:
            extensionString = L"GL_NV_vertex_program";
            break;

        case AP_GL_NV_vertex_program1_1:
            extensionString = L"GL_NV_vertex_program1_1";
            break;

        case AP_GL_NV_vertex_program2:
            extensionString = L"GL_NV_vertex_program2";
            break;

        case AP_GL_NV_vertex_program2_option:
            extensionString = L"GL_NV_vertex_program2_option";
            break;

        case AP_GL_NV_vertex_program3:
            extensionString = L"GL_NV_vertex_program3";
            break;

        case AP_GL_NV_shader_buffer_load:
            extensionString = L"GL_NV_shader_buffer_load";
			break;

        case AP_GL_NV_vertex_buffer_unified_memory:
            extensionString = L"GL_NV_vertex_buffer_unified_memory";
            break;

        case AP_GL_AMD_debug_output:
            extensionString = L"GL_AMD_debug_output";
            break;

        case AP_GL_AMDX_debug_output:
            extensionString = L"GL_AMDX_debug_output";
            break;

        case AP_GL_SGIS_generate_mipmap:
            extensionString = L"GL_SGIS_generate_mipmap";
            break;

        case AP_GL_SGIS_texture_border_clamp:
            extensionString = L"GL_SGIS_texture_border_clamp";
            break;

        case AP_GL_SGIS_texture_edge_clamp:
            extensionString = L"GL_SGIS_texture_edge_clamp";
            break;

        case AP_GL_SGIS_texture_lod:
            extensionString = L"GL_SGIS_texture_lod";
            break;

        case AP_GL_SGIS_texture_select:
            extensionString = L"GL_SGIS_texture_select";
            break;

        case AP_GL_SGIX_depth_texture:
            extensionString = L"GL_SGIX_depth_texture";
            break;

        case AP_GL_SGIX_interlace:
            extensionString = L"GL_SGIX_interlace";
            break;

        case AP_GL_SGIX_shadow:
            extensionString = L"GL_SGIX_shadow";
            break;

        case AP_GL_SGIX_shadow_ambient:
            extensionString = L"GL_SGIX_shadow_ambient";
            break;

        case AP_WGL_ARB_create_context:
            extensionString = L"WGL_ARB_create_context";
            break;

        case AP_WGL_ARB_extensions_string:
            extensionString = L"WGL_ARB_extensions_string";
            break;

        case AP_WGL_ARB_framebuffer_sRGB:
            extensionString = L"WGL_ARB_framebuffer_sRGB";
            break;

        case AP_WGL_ARB_make_current_read:
            extensionString = L"WGL_ARB_make_current_read";
            break;

        case AP_WGL_ARB_multisample:
            extensionString = L"WGL_ARB_multisample";
            break;

        case AP_WGL_ARB_pbuffer:
            extensionString = L"WGL_ARB_pbuffer";
            break;

        case AP_WGL_ARB_pixel_format:
            extensionString = L"WGL_ARB_pixel_format";
            break;

        case AP_WGL_ARB_pixel_format_float:
            extensionString = L"WGL_ARB_pixel_format_float";
            break;

        case AP_WGL_ARB_render_texture:
            extensionString = L"WGL_ARB_render_texture";
            break;

        case AP_WGL_ARB_buffer_region:
            extensionString = L"WGL_ARB_buffer_region";
            break;

        case AP_WGL_EXT_swap_control:
            extensionString = L"WGL_EXT_swap_control";
            break;

        case AP_WGL_I3D_genlock:
            extensionString = L"WGL_I3D_genlock";
            break;

        case AP_WGL_NV_gpu_affinity:
            extensionString = L"WGL_NV_gpu_affinity";
            break;

        case AP_GLX_ARB_fbconfig_float:
            extensionString = L"GLX_ARB_fbconfig_float";
            break;

        case AP_GLX_ARB_create_context:
            extensionString = L"GLX_ARB_create_context";
            break;

        case AP_GLX_ARB_framebuffer_sRGB:
            extensionString = L"GLX_ARB_framebuffer_sRGB";
            break;

        case AP_GLX_ARB_get_proc_address:
            extensionString = L"GLX_ARB_get_proc_address";
            break;

        case AP_GLX_ARB_multisample:
            extensionString = L"GLX_ARB_multisample";
            break;

        case AP_GLX_SGI_video_sync:
            extensionString = L"GLX_SGI_video_sync";
            break;

        case AP_GLX_SGIX_fbconfig:
            extensionString = L"GLX_SGIX_fbconfig";
            break;

        case AP_GLX_SGIX_pbuffer:
            extensionString = L"GLX_SGIX_pbuffer";
            break;

        case AP_GL_APPLE_aux_depth_stencil:
            extensionString = L"GL_APPLE_aux_depth_stencil";
            break;

        case AP_GL_APPLE_client_storage:
            extensionString = L"GL_APPLE_client_storage";
            break;

        case AP_GL_APPLE_element_array:
            extensionString = L"GL_APPLE_element_array";
            break;

        case AP_GL_APPLE_fence:
            extensionString = L"GL_APPLE_fence";
            break;

        case AP_GL_APPLE_float_pixels:
            extensionString = L"GL_APPLE_float_pixels";
            break;

        case AP_GL_APPLE_flush_buffer_range:
            extensionString = L"GL_APPLE_flush_buffer_range";
            break;

        case AP_GL_APPLE_flush_render:
            extensionString = L"GL_APPLE_flush_render ";
            break;

        case AP_GL_APPLE_object_purgeable:
            extensionString = L"GL_APPLE_object_purgeable";
            break;

        case AP_GL_APPLE_packed_pixels:
            extensionString = L"GL_APPLE_packed_pixels";
            break;

        case AP_GL_APPLE_pixel_buffer:
            extensionString = L"GL_APPLE_pixel_buffer";
            break;

        case AP_GL_APPLE_specular_vector :
            extensionString = L"GL_APPLE_specular_vector ";
            break;

        case AP_GL_APPLE_texture_range:
            extensionString = L"GL_APPLE_texture_range";
            break;

        case AP_GL_APPLE_transform_hint:
            extensionString = L"GL_APPLE_transform_hint";
            break;

        case AP_GL_APPLE_vertex_array_object:
            extensionString = L"GL_APPLE_vertex_array_object";
            break;

        case AP_GL_APPLE_vertex_array_range:
            extensionString = L"GL_APPLE_vertex_array_range";
            break;

        case AP_GL_APPLE_vertex_program_evaluators:
            extensionString = L"GL_APPLE_vertex_program_evaluators";
            break;

        case AP_GL_APPLE_ycbcr_422:
            extensionString = L"GL_APPLE_ycbcr_422";
            break;

        case AP_GL_ARB_framebuffer_object:
            extensionString = L"GL_ARB_framebuffer_object";
            break;

        case AP_GL_EXT_direct_state_access:
            extensionString = L"GL_EXT_direct_state_access";
            break;

        case AP_GL_EXT_bindable_uniform:
            extensionString = L"GL_EXT_bindable_uniform";
            break;

        case AP_GL_EXT_texture_integer:
            extensionString = L"GL_EXT_texture_integer";
            break;


        default:
            // A non supported extension was queried:
            extensionString.makeEmpty();
            retVal = false;
            break;
    }

    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        apIsOpenGLExtensionSupported
// Description:
//   Inputs the OpenGL extensions string and and extension name and
//   returns true iff the extension name appears in the extensions string.
// Arguments: extensionsString - The OpenGL extensions string, as returned from
//                               glGetString(GL_EXTENSIONS) or glGetStringi(GL_EXTENSIONS, i).
//            extensionName - The queried extension name.
//
// Return Val: bool  - true iff extensionName appears in extensionsString.
//
// Author:  AMD Developer Tools Team
// Date:        22/2/2006
//
// Implementation notes:
// Use of strstr() is not sufficient because extension names can be prefixes of
// other extension names. Could use strtok() but the constant string returned by
// glGetString can be in read-only memory.
// ---------------------------------------------------------------------------
bool apIsOpenGLExtensionSupported(const char* extensionsString, const char* extensionName)
{
    bool retVal = false;

    // Get the start and end positions of the extensions string:
    char* pCurrentPos = (char*)extensionsString;
    char* pEndPos = pCurrentPos + strlen(extensionsString);

    // Get the extension name length:
    gtSize_t extensionNameLen = strlen(extensionName);

    // Search for extensionName in the extensionsString:
    while (pCurrentPos < pEndPos)
    {

        gtSize_t n = strcspn(pCurrentPos, " ");

        if ((extensionNameLen == n) && (strncmp(extensionName, pCurrentPos, n) == 0))
        {
            retVal = true;
            break;
        }

        pCurrentPos += (n + 1);
    }

    return retVal;
}
