//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenGLStateVariableId.h
///
//==================================================================================

//------------------------------ apOpenGLStateVariableId.h ------------------------------

#ifndef __APOPENGLSTATEVARIABLEID
#define __APOPENGLSTATEVARIABLEID

// Infra:

// An enumeration that lists all the OpenGL state variables that we are
// currently monitor:
// (The enum value is the OpenGL state variable Id).
enum apOpenGLStateVariableId
{
    // OpenGL 1.1 state variables:
    apGL_ACCUM_ALPHA_BITS = 0,
    apGL_ACCUM_BLUE_BITS,
    apGL_ACCUM_CLEAR_VALUE,
    apGL_ACCUM_GREEN_BITS,
    apGL_ACCUM_RED_BITS,
    apGL_ALPHA_BIAS,
    apGL_ALPHA_BITS,
    apGL_ALPHA_SCALE,
    apGL_ALPHA_TEST,
    apGL_ALPHA_TEST_FUNC,
    apGL_ALPHA_TEST_REF,
    apGL_AMBIENT_back,
    apGL_AMBIENT_front,
    apGL_ATTRIB_STACK_DEPTH,
    apGL_AUTO_NORMAL,
    apGL_AUX_BUFFERS,
    apGL_BLEND,
    apGL_BLEND_DST,
    apGL_BLEND_SRC,
    apGL_BLUE_BIAS,
    apGL_BLUE_BITS,
    apGL_BLUE_SCALE,
    apGL_CLIP_PLANE0,
    apGL_CLIP_PLANE0_equation,
    apGL_CLIP_PLANE1,
    apGL_CLIP_PLANE1_equation,
    apGL_CLIP_PLANE2,
    apGL_CLIP_PLANE2_equation,
    apGL_CLIP_PLANE3,
    apGL_CLIP_PLANE3_equation,
    apGL_CLIP_PLANE4,
    apGL_CLIP_PLANE4_equation,
    apGL_CLIP_PLANE5,
    apGL_CLIP_PLANE5_equation,
    apGL_COLOR_CLEAR_VALUE,
    apGL_COLOR_INDEXES_back,
    apGL_COLOR_INDEXES_front,
    apGL_COLOR_MATERIAL,
    apGL_COLOR_MATERIAL_FACE,
    apGL_COLOR_MATERIAL_PARAMETER,
    apGL_COLOR_WRITEMASK,
    apGL_CULL_FACE,
    apGL_CULL_FACE_MODE,
    apGL_CURRENT_INDEX,
    apGL_CURRENT_NORMAL,
    apGL_CURRENT_RASTER_COLOR,
    apGL_CURRENT_RASTER_DISTANCE,
    apGL_CURRENT_RASTER_INDEX,
    apGL_CURRENT_RASTER_POSITION,
    apGL_CURRENT_RASTER_POSITION_VALID,
    apGL_CURRENT_RASTER_TEXTURE_COORDS,
    apGL_CURRENT_TEXTURE_COORDS,
    apGL_DEPTH_BIAS,
    apGL_DEPTH_BITS,
    apGL_DEPTH_CLEAR_VALUE,
    apGL_DEPTH_FUNC,
    apGL_DEPTH_RANGE,
    apGL_DEPTH_SCALE,
    apGL_DEPTH_TEST,
    apGL_DEPTH_WRITEMASK,
    apGL_DIFFUSE_back,
    apGL_DIFFUSE_front,
    apGL_DITHER,
    apGL_DOUBLEBUFFER,
    apGL_DRAW_BUFFER,
    apGL_EDGE_FLAG,
    apGL_EMISSION_back,
    apGL_EMISSION_front,
    apGL_FOG,
    apGL_FOG_COLOR,
    apGL_FOG_DENSITY,
    apGL_FOG_END,
    apGL_FOG_HINT,
    apGL_FOG_INDEX,
    apGL_FOG_MODE,
    apGL_FOG_START,
    apGL_FRONT_FACE,
    apGL_GREEN_BIAS,
    apGL_GREEN_BITS,
    apGL_GREEN_SCALE,
    apGL_INDEX_BITS,
    apGL_INDEX_CLEAR_VALUE,
    apGL_INDEX_MODE,
    apGL_INDEX_OFFSET,
    apGL_INDEX_SHIFT,
    apGL_INDEX_WRITEMASK,
    apGL_LIGHT0,
    apGL_LIGHT0_ambient,
    apGL_LIGHT0_constant_attenuation,
    apGL_LIGHT0_diffuse,
    apGL_LIGHT0_linear_attenuation,
    apGL_LIGHT0_position,
    apGL_LIGHT0_quadratic_attenuation,
    apGL_LIGHT0_specular,
    apGL_LIGHT0_spot_cutoff,
    apGL_LIGHT0_spot_direction,
    apGL_LIGHT0_spot_exponent,
    apGL_LIGHT1,
    apGL_LIGHT1_ambient,
    apGL_LIGHT1_constant_attenuation,
    apGL_LIGHT1_diffuse,
    apGL_LIGHT1_linear_attenuation,
    apGL_LIGHT1_position,
    apGL_LIGHT1_quadratic_attenuation,
    apGL_LIGHT1_specular,
    apGL_LIGHT1_spot_cutoff,
    apGL_LIGHT1_spot_direction,
    apGL_LIGHT1_spot_exponent,
    apGL_LIGHT2,
    apGL_LIGHT2_ambient,
    apGL_LIGHT2_constant_attenuation,
    apGL_LIGHT2_diffuse,
    apGL_LIGHT2_linear_attenuation,
    apGL_LIGHT2_position,
    apGL_LIGHT2_quadratic_attenuation,
    apGL_LIGHT2_specular,
    apGL_LIGHT2_spot_cutoff,
    apGL_LIGHT2_spot_direction,
    apGL_LIGHT2_spot_exponent,
    apGL_LIGHT3,
    apGL_LIGHT3_ambient,
    apGL_LIGHT3_constant_attenuation,
    apGL_LIGHT3_diffuse,
    apGL_LIGHT3_linear_attenuation,
    apGL_LIGHT3_position,
    apGL_LIGHT3_quadratic_attenuation,
    apGL_LIGHT3_specular,
    apGL_LIGHT3_spot_cutoff,
    apGL_LIGHT3_spot_direction,
    apGL_LIGHT3_spot_exponent,
    apGL_LIGHT4,
    apGL_LIGHT4_ambient,
    apGL_LIGHT4_constant_attenuation,
    apGL_LIGHT4_diffuse,
    apGL_LIGHT4_linear_attenuation,
    apGL_LIGHT4_position,
    apGL_LIGHT4_quadratic_attenuation,
    apGL_LIGHT4_specular,
    apGL_LIGHT4_spot_cutoff,
    apGL_LIGHT4_spot_direction,
    apGL_LIGHT4_spot_exponent,
    apGL_LIGHT5,
    apGL_LIGHT5_ambient,
    apGL_LIGHT5_constant_attenuation,
    apGL_LIGHT5_diffuse,
    apGL_LIGHT5_linear_attenuation,
    apGL_LIGHT5_position,
    apGL_LIGHT5_quadratic_attenuation,
    apGL_LIGHT5_specular,
    apGL_LIGHT5_spot_cutoff,
    apGL_LIGHT5_spot_direction,
    apGL_LIGHT5_spot_exponent,
    apGL_LIGHT6,
    apGL_LIGHT6_ambient,
    apGL_LIGHT6_constant_attenuation,
    apGL_LIGHT6_diffuse,
    apGL_LIGHT6_linear_attenuation,
    apGL_LIGHT6_position,
    apGL_LIGHT6_quadratic_attenuation,
    apGL_LIGHT6_specular,
    apGL_LIGHT6_spot_cutoff,
    apGL_LIGHT6_spot_direction,
    apGL_LIGHT6_spot_exponent,
    apGL_LIGHT7,
    apGL_LIGHT7_ambient,
    apGL_LIGHT7_constant_attenuation,
    apGL_LIGHT7_diffuse,
    apGL_LIGHT7_linear_attenuation,
    apGL_LIGHT7_position,
    apGL_LIGHT7_quadratic_attenuation,
    apGL_LIGHT7_specular,
    apGL_LIGHT7_spot_cutoff,
    apGL_LIGHT7_spot_direction,
    apGL_LIGHT7_spot_exponent,
    apGL_LIGHTING,
    apGL_LIGHT_MODEL_AMBIENT,
    apGL_LIGHT_MODEL_LOCAL_VIEWER,
    apGL_LIGHT_MODEL_TWO_SIDE,
    apGL_LINE_SMOOTH,
    apGL_LINE_SMOOTH_HINT,
    apGL_LINE_STIPPLE,
    apGL_LINE_STIPPLE_PATTERN,
    apGL_LINE_STIPPLE_REPEAT,
    apGL_LINE_WIDTH,
    apGL_LINE_WIDTH_GRANULARITY,
    apGL_LINE_WIDTH_RANGE,
    apGL_LIST_BASE,
    apGL_LIST_INDEX,
    apGL_LIST_MODE,
    apGL_LOGIC_OP,
    apGL_LOGIC_OP_MODE,
    apGL_MAP1_GRID_SEGMENTS,
    apGL_MAP2_GRID_SEGMENTS,
    apGL_MAP_COLOR,
    apGL_MAP_STENCIL,
    apGL_MATRIX_MODE,
    apGL_MAX_ATTRIB_STACK_DEPTH,
    apGL_MAX_CLIP_PLANES,
    apGL_MAX_EVAL_ORDER,
    apGL_MAX_LIGHTS,
    apGL_MAX_LIST_NESTING,
    apGL_MAX_MODELVIEW_STACK_DEPTH,
    apGL_MAX_NAME_STACK_DEPTH,
    apGL_MAX_PIXEL_MAP_TABLE,
    apGL_MAX_PROJECTION_STACK_DEPTH,
    apGL_MAX_TEXTURE_SIZE,
    apGL_MAX_TEXTURE_STACK_DEPTH,
    apGL_MAX_VIEWPORT_DIMS,
    apGL_MODELVIEW_MATRIX,
    apGL_MODELVIEW_STACK_DEPTH,
    apGL_NAME_STACK_DEPTH,
    apGL_NORMALIZE,
    apGL_PACK_ALIGNMENT,
    apGL_PACK_LSB_FIRST,
    apGL_PACK_ROW_LENGTH,
    apGL_PACK_SKIP_PIXELS,
    apGL_PACK_SKIP_ROWS,
    apGL_PACK_SWAP_BYTES,
    apGL_PERSPECTIVE_CORRECTION_HINT,
    apGL_PIXEL_MAP_A_TO_A_SIZE,
    apGL_PIXEL_MAP_B_TO_B_SIZE,
    apGL_PIXEL_MAP_G_TO_G_SIZE,
    apGL_PIXEL_MAP_I_TO_A_SIZE,
    apGL_PIXEL_MAP_I_TO_B_SIZE,
    apGL_PIXEL_MAP_I_TO_G_SIZE,
    apGL_PIXEL_MAP_I_TO_I_SIZE,
    apGL_PIXEL_MAP_I_TO_R_SIZE,
    apGL_PIXEL_MAP_R_TO_R_SIZE,
    apGL_PIXEL_MAP_S_TO_S_SIZE,
    apGL_POINT_SIZE,
    apGL_POINT_SIZE_GRANULARITY,
    apGL_POINT_SIZE_RANGE,
    apGL_POINT_SMOOTH,
    apGL_POINT_SMOOTH_HINT,
    apGL_POLYGON_MODE,
    apGL_POLYGON_SMOOTH,
    apGL_POLYGON_SMOOTH_HINT,
    apGL_POLYGON_STIPPLE,
    apGL_PROJECTION_MATRIX,
    apGL_PROJECTION_STACK_DEPTH,
    apGL_READ_BUFFER,
    apGL_RED_BIAS,
    apGL_RED_BITS,
    apGL_RED_SCALE,
    apGL_RENDER_MODE,
    apGL_RGBA_MODE,
    apGL_SCISSOR_BOX,
    apGL_SCISSOR_TEST,
    apGL_SHADE_MODEL,
    apGL_SHININESS_back,
    apGL_SHININESS_front,
    apGL_SPECULAR_back,
    apGL_SPECULAR_front,
    apGL_STENCIL_BITS,
    apGL_STENCIL_CLEAR_VALUE,
    apGL_STENCIL_FAIL,
    apGL_STENCIL_FUNC,
    apGL_STENCIL_PASS_DEPTH_FAIL,
    apGL_STENCIL_PASS_DEPTH_PASS,
    apGL_STENCIL_REF,
    apGL_STENCIL_TEST,
    apGL_STENCIL_VALUE_MASK,
    apGL_STENCIL_WRITEMASK,
    apGL_STEREO,
    apGL_SUBPIXEL_BITS,
    apGL_TEXTURE_1D,
    apGL_TEXTURE_2D,
    apGL_TEXTURE_GEN_Q,
    apGL_TEXTURE_GEN_R,
    apGL_TEXTURE_GEN_S,
    apGL_TEXTURE_GEN_T,
    apGL_TEXTURE_GEN_MODE_s,
    apGL_TEXTURE_GEN_MODE_t,
    apGL_TEXTURE_GEN_MODE_r,
    apGL_TEXTURE_GEN_MODE_q,
    apGL_OBJECT_PLANE_s,
    apGL_OBJECT_PLANE_t,
    apGL_OBJECT_PLANE_r,
    apGL_OBJECT_PLANE_q,
    apGL_EYE_PLANE_s,
    apGL_EYE_PLANE_t,
    apGL_EYE_PLANE_r,
    apGL_EYE_PLANE_q,
    apGL_TEXTURE_MATRIX,
    apGL_TEXTURE_STACK_DEPTH,
    apGL_UNPACK_ALIGNMENT,
    apGL_UNPACK_LSB_FIRST,
    apGL_UNPACK_ROW_LENGTH,
    apGL_UNPACK_SKIP_PIXELS,
    apGL_UNPACK_SKIP_ROWS,
    apGL_UNPACK_SWAP_BYTES,
    apGL_VIEWPORT,
    apGL_ZOOM_X,
    apGL_ZOOM_Y,
    apGL_TEXTURE_BINDING_1D,
    apGL_TEXTURE_BINDING_2D,
    apGL_TEXTURE_BINDING_3D,
    apGL_TEXTURE_ENV_MODE,
    apGL_TEXTURE_ENV_COLOR,
    apGL_CURRENT_COLOR,
    apGL_RESCALE_NORMAL,
    apGL_POLYGON_OFFSET_FACTOR,
    apGL_POLYGON_OFFSET_UNITS,
    apGL_POLYGON_OFFSET_POINT,
    apGL_POLYGON_OFFSET_LINE,
    apGL_POLYGON_OFFSET_FILL,
    AP_LAST_OGL_1_1_STATE_VAR_INDEX = apGL_POLYGON_OFFSET_FILL,

    // OpenGL extensions state variables:

    // OpenGL 1.2
    apGL_TEXTURE_3D,
    apGL_MAX_ELEMENTS_VERTICES,
    apGL_MAX_ELEMENTS_INDICES,
    apGL_BLEND_COLOR,
    apGL_BLEND_EQUATION,
    apGL_VERTEX_ARRAY,
    apGL_VERTEX_ARRAY_SIZE,
    apGL_VERTEX_ARRAY_STRIDE,
    apGL_VERTEX_ARRAY_TYPE,
    apGL_VERTEX_ARRAY_POINTER,
    apGL_NORMAL_ARRAY,
    apGL_NORMAL_ARRAY_STRIDE,
    apGL_NORMAL_ARRAY_TYPE,
    apGL_NORMAL_ARRAY_POINTER,
    apGL_COLOR_ARRAY,
    apGL_COLOR_ARRAY_SIZE,
    apGL_COLOR_ARRAY_STRIDE,
    apGL_COLOR_ARRAY_TYPE,
    apGL_COLOR_ARRAY_POINTER,
    apGL_TEXTURE_COORD_ARRAY,
    apGL_TEXTURE_COORD_ARRAY_SIZE,
    apGL_TEXTURE_COORD_ARRAY_STRIDE,
    apGL_TEXTURE_COORD_ARRAY_TYPE,
    apGL_TEXTURE_COORD_ARRAY_POINTER,
    AP_LAST_OGL_1_2_STATE_VAR_INDEX = apGL_TEXTURE_COORD_ARRAY_POINTER,

    // OpenGL 1.3
    apGL_TEXTURE_COMPRESSION_HINT,
    apGL_SAMPLE_COVERAGE_VALUE,
    apGL_SAMPLE_COVERAGE_INVERT,
    apGL_SAMPLE_COVERAGE,
    apGL_SAMPLE_ALPHA_TO_COVERAGE,
    apGL_SAMPLE_ALPHA_TO_ONE,
    apGL_MULTISAMPLE,
    apGL_ACTIVE_TEXTURE,
    apGL_CLIENT_ACTIVE_TEXTURE,
    apGL_MAX_TEXTURE_UNITS,
    AP_LAST_OGL_1_3_STATE_VAR_INDEX = apGL_MAX_TEXTURE_UNITS,

    // OpenGL 1.4
    apGL_BLEND_SRC_RGB,
    apGL_BLEND_SRC_ALPHA,
    apGL_BLEND_DST_RGB,
    apGL_BLEND_DST_ALPHA,
    apGL_CURRENT_FOG_COORDINATE,
    apGL_FOG_COORDINATE_ARRAY,
    apGL_FOG_COORDINATE_ARRAY_STRIDE,
    apGL_FOG_COORDINATE_ARRAY_TYPE,
    apGL_FOG_COORDINATE_ARRAY_POINTER,
    apGL_POINT_SIZE_MIN,
    apGL_POINT_SIZE_MAX,
    apGL_POINT_FADE_THRESHOLD_SIZE,
    apGL_POINT_DISTANCE_ATTENUATION,
    apGL_CURRENT_SECONDARY_COLOR,
    apGL_COLOR_SUM,
    apGL_SECONDARY_COLOR_ARRAY,
    apGL_SECONDARY_COLOR_ARRAY_SIZE,
    apGL_SECONDARY_COLOR_ARRAY_TYPE,
    apGL_SECONDARY_COLOR_ARRAY_STRIDE,
    apGL_SECONDARY_COLOR_ARRAY_POINTER,
    AP_LAST_OGL_1_4_STATE_VAR_INDEX = apGL_SECONDARY_COLOR_ARRAY_POINTER,

    // OpenGL 1.5
    apGL_BUFFER_SIZE_array_buffer,
    apGL_BUFFER_USAGE_array_buffer,
    apGL_BUFFER_ACCESS_array_buffer,
    apGL_BUFFER_MAPPED_array_buffer,
    apGL_BUFFER_SIZE_element_array_buffer,
    apGL_BUFFER_USAGE_element_array_buffer,
    apGL_BUFFER_ACCESS_element_array_buffer,
    apGL_BUFFER_MAPPED_element_array_buffer,
    apGL_QUERY_COUNTER_BITS,
    apGL_ARRAY_BUFFER_BINDING,
    apGL_ELEMENT_ARRAY_BUFFER_BINDING,
    apGL_VERTEX_ARRAY_BUFFER_BINDING,
    apGL_NORMAL_ARRAY_BUFFER_BINDING,
    apGL_COLOR_ARRAY_BUFFER_BINDING,
    apGL_INDEX_ARRAY_BUFFER_BINDING,
    apGL_TEXTURE_COORD_ARRAY_BUFFER_BINDING,
    apGL_EDGE_FLAG_ARRAY_BUFFER_BINDING,
    apGL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING,
    apGL_FOG_COORD_SRC,
    apGL_CURRENT_FOG_COORD,
    apGL_FOG_COORD_ARRAY_TYPE,
    apGL_FOG_COORD_ARRAY_STRIDE,
    apGL_FOG_COORD_ARRAY_POINTER,
    apGL_FOG_COORD_ARRAY,
    apGL_FOG_COORD_ARRAY_BUFFER_BINDING,
    apGL_SRC0_RGB,
    apGL_SRC1_RGB,
    apGL_SRC2_RGB,
    apGL_SRC0_ALPHA,
    apGL_SRC1_ALPHA,
    apGL_SRC2_ALPHA,
    apGL_COMBINE_RGB,
    apGL_COMBINE_ALPHA,
    apGL_OPERAND0_RGB,
    apGL_OPERAND1_RGB,
    apGL_OPERAND2_RGB,
    apGL_OPERAND0_ALPHA,
    apGL_OPERAND1_ALPHA,
    apGL_OPERAND2_ALPHA,
    apGL_RGB_SCALE,
    apGL_ALPHA_SCALE_TexEnv,
    apGL_COLOR_LOGIC_OP,
    apGL_GENERATE_MIPMAP_HINT,
    apGL_ALIASED_POINT_SIZE_RANGE,
    apGL_SMOOTH_POINT_SIZE_RANGE,
    apGL_ALIASED_LINE_WIDTH_RANGE,
    apGL_SMOOTH_LINE_WIDTH_RANGE,
    apGL_SAMPLE_BUFFERS,
    apGL_SAMPLES,
    apGL_COMPRESSED_TEXTURE_FORMATS,
    apGL_NUM_COMPRESSED_TEXTURE_FORMATS,
    AP_LAST_OGL_1_5_STATE_VAR_INDEX = apGL_NUM_COMPRESSED_TEXTURE_FORMATS,
    AP_LAST_OGLES_1_1_STATE_VAR_INDEX = apGL_NUM_COMPRESSED_TEXTURE_FORMATS,

    // OpenGL 2.0
    apGL_BLEND_EQUATION_RGB,
    apGL_VERTEX_PROGRAM_POINT_SIZE,
    apGL_VERTEX_PROGRAM_TWO_SIDE,
    apGL_STENCIL_BACK_FUNC,
    apGL_STENCIL_BACK_FAIL,
    apGL_STENCIL_BACK_PASS_DEPTH_FAIL,
    apGL_STENCIL_BACK_PASS_DEPTH_PASS,
    apGL_MAX_DRAW_BUFFERS,
    apGL_DRAW_BUFFER0,
    apGL_DRAW_BUFFER1,
    apGL_DRAW_BUFFER2,
    apGL_DRAW_BUFFER3,
    apGL_DRAW_BUFFER4,
    apGL_DRAW_BUFFER5,
    apGL_DRAW_BUFFER6,
    apGL_DRAW_BUFFER7,
    apGL_DRAW_BUFFER8,
    apGL_DRAW_BUFFER9,
    apGL_DRAW_BUFFER10,
    apGL_DRAW_BUFFER11,
    apGL_DRAW_BUFFER12,
    apGL_DRAW_BUFFER13,
    apGL_DRAW_BUFFER14,
    apGL_DRAW_BUFFER15,
    apGL_BLEND_EQUATION_ALPHA,
    apGL_POINT_SPRITE,
    apGL_MAX_VERTEX_ATTRIBS,
    apGL_MAX_TEXTURE_COORDS,
    apGL_MAX_TEXTURE_IMAGE_UNITS,
    apGL_MAX_VERTEX_UNIFORM_COMPONENTS,
    apGL_MAX_VARYING_FLOATS,
    apGL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
    apGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
    apGL_FRAGMENT_SHADER_DERIVATIVE_HINT,
    apGL_CURRENT_PROGRAM,
    apGL_STENCIL_BACK_REF,
    apGL_STENCIL_BACK_VALUE_MASK,
    apGL_STENCIL_BACK_WRITEMASK,
    AP_LAST_OGL_2_0_STATE_VAR_INDEX = apGL_STENCIL_BACK_WRITEMASK,
    AP_LAST_OGLES_2_0_STATE_VAR_INDEX = apGL_STENCIL_BACK_WRITEMASK,

    // OpenGL 3.0
    apGL_CLIP_DISTANCE0,            // Same as apGL_CLIP_PLANEi
    apGL_CLIP_DISTANCE1,
    apGL_CLIP_DISTANCE2,
    apGL_CLIP_DISTANCE3,
    apGL_CLIP_DISTANCE4,
    apGL_CLIP_DISTANCE5,
    apGL_MAX_CLIP_DISTANCES,        // Same as apGL_MAX_CLIP_PLANES
    apGL_MAJOR_VERSION,
    apGL_MINOR_VERSION,
    apGL_NUM_EXTENSIONS,
    apGL_CONTEXT_FLAGS,
    apGL_MAX_ARRAY_TEXTURE_LAYERS,
    apGL_MIN_PROGRAM_TEXEL_OFFSET,
    apGL_MAX_PROGRAM_TEXEL_OFFSET,
    apGL_CLAMP_VERTEX_COLOR,
    apGL_CLAMP_FRAGMENT_COLOR,
    apGL_CLAMP_READ_COLOR,
    apGL_MAX_VARYING_COMPONENTS,    // Same as apGL_MAX_VARYING_FLOATS
    apGL_TEXTURE_BINDING_1D_ARRAY,
    apGL_TEXTURE_BINDING_2D_ARRAY,
    apGL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS,
    apGL_TRANSFORM_FEEDBACK_BUFFER_BINDING,
    apGL_RASTERIZER_DISCARD,
    apGL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS,
    apGL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS,
    AP_LAST_OGL_3_0_STATE_VAR_INDEX = apGL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS,

    // TO_DO: 3.1? 3.2? 3.3?

    // OpenGL 4.0
    apGL_DRAW_INDIRECT_BUFFER_BINDING,
    apGL_MAX_GEOMETRY_SHADER_INVOCATIONS,
    apGL_MIN_FRAGMENT_INTERPOLATION_OFFSET,
    apGL_MAX_FRAGMENT_INTERPOLATION_OFFSET,
    apGL_FRAGMENT_INTERPOLATION_OFFSET_BITS,
    apGL_MAX_VERTEX_STREAMS,
    apGL_MAX_SUBROUTINES,
    apGL_MAX_SUBROUTINE_UNIFORM_LOCATIONS,
    apGL_PATCH_VERTICES,
    apGL_PATCH_DEFAULT_INNER_LEVEL,
    apGL_PATCH_DEFAULT_OUTER_LEVEL,
    apGL_MAX_PATCH_VERTICES,
    apGL_MAX_TESS_GEN_LEVEL,
    apGL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS,
    apGL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS,
    apGL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS,
    apGL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS,
    apGL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS,
    apGL_MAX_TESS_PATCH_COMPONENTS,
    apGL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS,
    apGL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS,
    apGL_MAX_TESS_CONTROL_UNIFORM_BLOCKS,
    apGL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS,
    apGL_MAX_TESS_CONTROL_INPUT_COMPONENTS,
    apGL_MAX_TESS_EVALUATION_INPUT_COMPONENTS,
    apGL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS,
    apGL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS,
    apGL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER,
    apGL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER,
    apGL_TRANSFORM_FEEDBACK_BUFFER_PAUSED,
    apGL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE,
    apGL_TRANSFORM_FEEDBACK_BINDING,
    apGL_MAX_TRANSFORM_FEEDBACK_BUFFERS,
    AP_LAST_OGL_4_0_STATE_VAR_INDEX = apGL_MAX_TRANSFORM_FEEDBACK_BUFFERS,

    // OpenGL 4.1
    apGL_IMPLEMENTATION_COLOR_READ_TYPE,
    apGL_IMPLEMENTATION_COLOR_READ_FORMAT,
    apGL_SHADER_COMPILER,
    apGL_SHADER_BINARY_FORMATS,
    apGL_NUM_SHADER_BINARY_FORMATS,
    apGL_MAX_VERTEX_UNIFORM_VECTORS,
    apGL_MAX_VARYING_VECTORS,
    apGL_MAX_FRAGMENT_UNIFORM_VECTORS,
    apGL_NUM_PROGRAM_BINARY_FORMATS,
    apGL_PROGRAM_BINARY_FORMATS,
    apGL_PROGRAM_PIPELINE_BINDING,
    apGL_MAX_VIEWPORTS,
    apGL_VIEWPORT_SUBPIXEL_BITS,
    apGL_VIEWPORT_BOUNDS_RANGE,
    apGL_LAYER_PROVOKING_VERTEX,
    apGL_VIEWPORT_INDEX_PROVOKING_VERTEX,
    AP_LAST_OGL_4_1_STATE_VAR_INDEX = apGL_VIEWPORT_INDEX_PROVOKING_VERTEX,

    // OpenGL 4.2
    apGL_UNPACK_COMPRESSED_BLOCK_WIDTH,
    apGL_UNPACK_COMPRESSED_BLOCK_HEIGHT,
    apGL_UNPACK_COMPRESSED_BLOCK_DEPTH,
    apGL_UNPACK_COMPRESSED_BLOCK_SIZE,
    apGL_PACK_COMPRESSED_BLOCK_WIDTH,
    apGL_PACK_COMPRESSED_BLOCK_HEIGHT,
    apGL_PACK_COMPRESSED_BLOCK_DEPTH,
    apGL_PACK_COMPRESSED_BLOCK_SIZE,
    apGL_MIN_MAP_BUFFER_ALIGNMENT,
    apGL_ATOMIC_COUNTER_BUFFER_BINDING,
    apGL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS,
    apGL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS,
    apGL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS,
    apGL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS,
    apGL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS,
    apGL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS,
    apGL_MAX_VERTEX_ATOMIC_COUNTERS,
    apGL_MAX_TESS_CONTROL_ATOMIC_COUNTERS,
    apGL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS,
    apGL_MAX_GEOMETRY_ATOMIC_COUNTERS,
    apGL_MAX_FRAGMENT_ATOMIC_COUNTERS,
    apGL_MAX_COMBINED_ATOMIC_COUNTERS,
    apGL_MAX_ATOMIC_COUNTER_BUFFER_SIZE,
    apGL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS,
    apGL_MAX_IMAGE_UNITS,
    apGL_MAX_IMAGE_SAMPLES,
    apGL_MAX_VERTEX_IMAGE_UNIFORMS,
    apGL_MAX_TESS_CONTROL_IMAGE_UNIFORMS,
    apGL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS,
    apGL_MAX_GEOMETRY_IMAGE_UNIFORMS,
    apGL_MAX_FRAGMENT_IMAGE_UNIFORMS,
    apGL_MAX_COMBINED_IMAGE_UNIFORMS,
    AP_LAST_OGL_4_2_STATE_VAR_INDEX = apGL_MAX_COMBINED_IMAGE_UNIFORMS,

    // OpenGL 4.3
    apGL_NUM_SHADING_LANGUAGE_VERSIONS,
    apGL_PRIMITIVE_RESTART_FIXED_INDEX,
    apGL_MAX_ELEMENT_INDEX,
    apGL_MAX_COMPUTE_UNIFORM_BLOCKS,
    apGL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS,
    apGL_MAX_COMPUTE_IMAGE_UNIFORMS,
    apGL_MAX_COMPUTE_SHARED_MEMORY_SIZE,
    apGL_MAX_COMPUTE_UNIFORM_COMPONENTS,
    apGL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS,
    apGL_MAX_COMPUTE_ATOMIC_COUNTERS,
    apGL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS,
    apGL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS,
    apGL_DISPATCH_INDIRECT_BUFFER_BINDING,
    apGL_DEBUG_OUTPUT_SYNCHRONOUS,
    apGL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH,
    apGL_DEBUG_CALLBACK_FUNCTION,
    apGL_DEBUG_CALLBACK_USER_PARAM,
    apGL_MAX_DEBUG_MESSAGE_LENGTH,
    apGL_MAX_DEBUG_LOGGED_MESSAGES,
    apGL_DEBUG_LOGGED_MESSAGES,
    apGL_MAX_DEBUG_GROUP_STACK_DEPTH,
    apGL_DEBUG_GROUP_STACK_DEPTH,
    apGL_MAX_LABEL_LENGTH,
    apGL_DEBUG_OUTPUT,
    apGL_MAX_UNIFORM_LOCATIONS,
    apGL_MAX_FRAMEBUFFER_WIDTH,
    apGL_MAX_FRAMEBUFFER_HEIGHT,
    apGL_MAX_FRAMEBUFFER_LAYERS,
    apGL_MAX_FRAMEBUFFER_SAMPLES,
    apGL_SHADER_STORAGE_BUFFER_BINDING,
    apGL_MAX_VERTEX_SHADER_STORAGE_BLOCKS,
    apGL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS,
    apGL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS,
    apGL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS,
    apGL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS,
    apGL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS,
    apGL_MAX_COMBINED_SHADER_STORAGE_BLOCKS,
    apGL_MAX_SHADER_STORAGE_BUFFER_BINDINGS,
    apGL_MAX_SHADER_STORAGE_BLOCK_SIZE,
    apGL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT,
    apGL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES,
    apGL_TEXTURE_BUFFER_OFFSET_ALIGNMENT,
    apGL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET,
    apGL_MAX_VERTEX_ATTRIB_BINDINGS,
    AP_LAST_OGL_4_3_STATE_VAR_INDEX = apGL_MAX_VERTEX_ATTRIB_BINDINGS,

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_primitive_restart
    //////////////////////////////////////////////////////////////////////////
    apGL_PRIMITIVE_RESTART_NV,
    apGL_PRIMITIVE_RESTART_INDEX_NV,

    //////////////////////////////////////////////////////////////////////////
    // GL_HP_occlusion_test extension
    //////////////////////////////////////////////////////////////////////////
    apGL_OCCLUSION_TEST_HP,
    apGL_OCCLUSION_TEST_RESULT_HP,

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_occlusion_query
    //////////////////////////////////////////////////////////////////////////
    apGL_CURRENT_OCCLUSION_QUERY_ID_NV,
    apGL_PIXEL_COUNTER_BITS_NV,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_occlusion_query extension
    //////////////////////////////////////////////////////////////////////////
    apGL_CURRENT_QUERY_ARB,
    apGL_QUERY_COUNTER_BITS_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_texture_cube_map extension
    //////////////////////////////////////////////////////////////////////////
    apGL_TEXTURE_CUBE_MAP_ARB,
    apGL_TEXTURE_BINDING_CUBE_MAP_ARB,
    apGL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_texture_compression extension
    //////////////////////////////////////////////////////////////////////////
    apGL_TEXTURE_COMPRESSION_HINT_ARB,
    apGL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB,
    apGL_COMPRESSED_TEXTURE_FORMATS_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_vertex_buffer_object extension
    //////////////////////////////////////////////////////////////////////////
    apGL_ARRAY_BUFFER_BINDING_ARB,
    apGL_VERTEX_ARRAY_BUFFER_BINDING_ARB,
    apGL_NORMAL_ARRAY_BUFFER_BINDING_ARB,
    apGL_COLOR_ARRAY_BUFFER_BINDING_ARB,
    apGL_INDEX_ARRAY_BUFFER_BINDING_ARB,
    apGL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB,
    apGL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB,
    apGL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB,
    apGL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB,
    apGL_WEIGHT_ARRAY_BUFFER_BINDING_ARB,
    apGL_ELEMENT_ARRAY_BUFFER_BINDING_ARB,
    // Uri, 31/12/09 - This should be added, along with other values (_SIZE, _TYPE, _STRIDE, _POINTER)
    // on a vertex-attrib base (i.e. for each attrib separately).
    //apGL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB,

    apGL_ARRAY_BUFFER_GL_BUFFER_USAGE_ARB,
    apGL_ARRAY_BUFFER_GL_BUFFER_MAPPED_ARB,
    apGL_ARRAY_BUFFER_GL_BUFFER_SIZE_ARB,
    apGL_ARRAY_BUFFER_GL_BUFFER_ACCESS_ARB,

    apGL_ELEMENT_ARRAY_BUFFER_GL_BUFFER_USAGE_ARB,
    apGL_ELEMENT_ARRAY_BUFFER_GL_BUFFER_MAPPED_ARB,
    apGL_ELEMENT_ARRAY_BUFFER_GL_BUFFER_SIZE_ARB,
    apGL_ELEMENT_ARRAY_BUFFER_GL_BUFFER_ACCESS_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_vertex_blend extension
    //////////////////////////////////////////////////////////////////////////
    apGL_CURRENT_WEIGHT_ARB,
    apGL_WEIGHT_ARRAY_ARB,
    apGL_WEIGHT_ARRAY_TYPE_ARB,
    apGL_WEIGHT_ARRAY_SIZE_ARB,
    apGL_WEIGHT_ARRAY_STRIDE_ARB,
    apGL_WEIGHT_ARRAY_POINTER_ARB,
    apGL_ACTIVE_VERTEX_UNITS_ARB,
    apGL_VERTEX_BLEND_ARB,
    apGL_MAX_VERTEX_UNITS_ARB,
    apGL_MODELVIEW0_ARB,
    apGL_MODELVIEW1_ARB,
    apGL_MODELVIEW2_ARB,
    apGL_MODELVIEW3_ARB,
    apGL_MODELVIEW4_ARB,
    apGL_MODELVIEW5_ARB,
    apGL_MODELVIEW6_ARB,
    apGL_MODELVIEW7_ARB,
    apGL_MODELVIEW8_ARB,
    apGL_MODELVIEW9_ARB,
    apGL_MODELVIEW10_ARB,
    apGL_MODELVIEW11_ARB,
    apGL_MODELVIEW12_ARB,
    apGL_MODELVIEW13_ARB,
    apGL_MODELVIEW14_ARB,
    apGL_MODELVIEW15_ARB,
    apGL_MODELVIEW16_ARB,
    apGL_MODELVIEW17_ARB,
    apGL_MODELVIEW18_ARB,
    apGL_MODELVIEW19_ARB,
    apGL_MODELVIEW20_ARB,
    apGL_MODELVIEW21_ARB,
    apGL_MODELVIEW22_ARB,
    apGL_MODELVIEW23_ARB,
    apGL_MODELVIEW24_ARB,
    apGL_MODELVIEW25_ARB,
    apGL_MODELVIEW26_ARB,
    apGL_MODELVIEW27_ARB,
    apGL_MODELVIEW28_ARB,
    apGL_MODELVIEW29_ARB,
    apGL_MODELVIEW30_ARB,
    apGL_MODELVIEW31_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_texture3D extension
    //////////////////////////////////////////////////////////////////////////
    apGL_TEXTURE_3D_EXT,
    apGL_PACK_SKIP_IMAGES_EXT,
    apGL_PACK_IMAGE_HEIGHT_EXT,
    apGL_UNPACK_SKIP_IMAGES_EXT,
    apGL_UNPACK_IMAGE_HEIGHT_EXT,
    apGL_MAX_3D_TEXTURE_SIZE_EXT,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_vertex_program
    //////////////////////////////////////////////////////////////////////////
    apGL_VERTEX_PROGRAM_ARB,
    apGL_VERTEX_PROGRAM_POINT_SIZE_ARB,
    apGL_VERTEX_PROGRAM_TWO_SIDE_ARB,
    apGL_PROGRAM_BINDING_ARB_vertex_program,
    apGL_PROGRAM_LENGTH_ARB_vertex_program,
    apGL_PROGRAM_FORMAT_ARB_vertex_program,
    apGL_PROGRAM_INSTRUCTIONS_ARB,
    apGL_PROGRAM_TEMPORARIES_ARB,
    apGL_PROGRAM_PARAMETERS_ARB,
    apGL_PROGRAM_ATTRIBS_ARB,
    apGL_PROGRAM_ADDRESS_REGISTERS_ARB,
    apGL_PROGRAM_NATIVE_INSTRUCTIONS_ARB,
    apGL_PROGRAM_NATIVE_TEMPORARIES_ARB,
    apGL_PROGRAM_NATIVE_PARAMETERS_ARB,
    apGL_PROGRAM_NATIVE_ATTRIBS_ARB,
    apGL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB,
    apGL_PROGRAM_UNDER_NATIVE_LIMITS_ARB,
    apGL_CURRENT_MATRIX_ARB,
    apGL_CURRENT_MATRIX_STACK_DEPTH_ARB,
    apGL_MAX_PROGRAM_ENV_PARAMETERS_ARB,
    apGL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB,
    apGL_MAX_PROGRAM_MATRICES_ARB,
    apGL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB,
    apGL_MAX_PROGRAM_INSTRUCTIONS_ARB,
    apGL_MAX_PROGRAM_TEMPORARIES_ARB,
    apGL_MAX_PROGRAM_PARAMETERS_ARB,
    apGL_MAX_PROGRAM_ATTRIBS_ARB,
    apGL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB,
    apGL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB,
    apGL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB,
    apGL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB,
    apGL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB,
    apGL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_fragment_program
    //////////////////////////////////////////////////////////////////////////
    apGL_FRAGMENT_PROGRAM_ARB,
    apGL_PROGRAM_ALU_INSTRUCTIONS_ARB,
    apGL_PROGRAM_TEX_INSTRUCTIONS_ARB,
    apGL_PROGRAM_TEX_INDIRECTIONS_ARB,
    apGL_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB,
    apGL_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB,
    apGL_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB,
    apGL_MAX_TEXTURE_COORDS_ARB,
    apGL_MAX_TEXTURE_IMAGE_UNITS_ARB,
    apGL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB,
    apGL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB,
    apGL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB,
    apGL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB,
    apGL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB,
    apGL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_vertex_shader
    //////////////////////////////////////////////////////////////////////////
    apGL_MAX_VERTEX_ATTRIBS_ARB,
    apGL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB,
    apGL_MAX_VARYING_FLOATS_ARB,
    apGL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB,
    apGL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB,


    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_shader_objects
    //////////////////////////////////////////////////////////////////////////
    apGL_PROGRAM_OBJECT_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_fragment_shader
    //////////////////////////////////////////////////////////////////////////
    apGL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB,
    apGL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_vertex_program
    //////////////////////////////////////////////////////////////////////////
    apGL_VERTEX_PROGRAM_NV,
    apGL_VERTEX_PROGRAM_POINT_SIZE_NV,
    apGL_VERTEX_PROGRAM_TWO_SIDE_NV,
    apGL_VERTEX_PROGRAM_BINDING_NV,
    apGL_VERTEX_ATTRIB_ARRAY0_NV,
    apGL_VERTEX_ATTRIB_ARRAY1_NV,
    apGL_VERTEX_ATTRIB_ARRAY2_NV,
    apGL_VERTEX_ATTRIB_ARRAY3_NV,
    apGL_VERTEX_ATTRIB_ARRAY4_NV,
    apGL_VERTEX_ATTRIB_ARRAY5_NV,
    apGL_VERTEX_ATTRIB_ARRAY6_NV,
    apGL_VERTEX_ATTRIB_ARRAY7_NV,
    apGL_VERTEX_ATTRIB_ARRAY8_NV,
    apGL_VERTEX_ATTRIB_ARRAY9_NV,
    apGL_VERTEX_ATTRIB_ARRAY10_NV,
    apGL_VERTEX_ATTRIB_ARRAY11_NV,
    apGL_VERTEX_ATTRIB_ARRAY12_NV,
    apGL_VERTEX_ATTRIB_ARRAY13_NV,
    apGL_VERTEX_ATTRIB_ARRAY14_NV,
    apGL_VERTEX_ATTRIB_ARRAY15_NV,
    apGL_CURRENT_MATRIX_STACK_DEPTH_NV,
    apGL_CURRENT_MATRIX_NV,
    apGL_MAX_TRACK_MATRIX_STACK_DEPTH_NV,
    apGL_MAX_TRACK_MATRICES_NV,

    //////////////////////////////////////////////////////////////////////////
    // GL_ATI_fragment_shader
    //////////////////////////////////////////////////////////////////////////
    apGL_FRAGMENT_SHADER_ATI,

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_vertex_shader
    //////////////////////////////////////////////////////////////////////////
    apGL_VERTEX_SHADER_INSTRUCTIONS_EXT,
    apGL_VERTEX_SHADER_VARIANTS_EXT,
    apGL_VERTEX_SHADER_INVARIANTS_EXT,
    apGL_VERTEX_SHADER_LOCAL_CONSTANTS_EXT,
    apGL_VERTEX_SHADER_LOCALS_EXT,
    apGL_VERTEX_SHADER_OPTIMIZED_EXT,
    apGL_VERTEX_SHADER_EXT,
    apGL_VERTEX_SHADER_BINDING_EXT,
    apGL_VARIANT_ARRAY_EXT,
    apGL_MAX_VERTEX_SHADER_INSTRUCTIONS_EXT,
    apGL_MAX_VERTEX_SHADER_VARIANTS_EXT,
    apGL_MAX_VERTEX_SHADER_INVARIANTS_EXT,
    apGL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT,
    apGL_MAX_VERTEX_SHADER_LOCALS_EXT,
    apGL_MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS_EXT,
    apGL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS_EXT,
    apGL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT,
    apGL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS_EXT,
    apGL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS_EXT,

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_texture_shader
    //////////////////////////////////////////////////////////////////////////
    apGL_LO_BIAS_NV,
    apGL_DS_BIAS_NV,
    apGL_DT_BIAS_NV,
    apGL_MAGNITUDE_BIAS_NV,
    apGL_VIBRANCE_BIAS_NV,
    apGL_HI_SCALE_NV,
    apGL_LO_SCALE_NV,
    apGL_DS_SCALE_NV,
    apGL_DT_SCALE_NV,
    apGL_MAGNITUDE_SCALE_NV,
    apGL_VIBRANCE_SCALE_NV,
    apGL_TEXTURE_SHADER_NV,
    apGL_SHADER_OPERATION_NV,
    apGL_CULL_MODES_NV,
    apGL_RGBA_UNSIGNED_DOT_PRODUCT_MAPPING_NV,
    apGL_PREVIOUS_TEXTURE_INPUT_NV,
    apGL_CONST_EYE_NV,
    apGL_OFFSET_TEXTURE_MATRIX_NV,
    apGL_OFFSET_TEXTURE_SCALE_NV,
    apGL_OFFSET_TEXTURE_BIAS_NV,
    apGL_SHADER_CONSISTENT_NV,

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_texture_shader3
    //////////////////////////////////////////////////////////////////////////
    // State variables are already implemented in GL_NV_texture_shader

    //////////////////////////////////////////////////////////////////////////
    // GL_ATI_text_fragment_shader
    //////////////////////////////////////////////////////////////////////////
    apGL_TEXT_FRAGMENT_SHADER_ATI,

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_fragment_program
    //////////////////////////////////////////////////////////////////////////
    apGL_FRAGMENT_PROGRAM_NV,
    apGL_FRAGMENT_PROGRAM_BINDING_NV,
    apGL_MAX_TEXTURE_COORDS_NV,
    apGL_MAX_TEXTURE_IMAGE_UNITS_NV,
    apGL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV,

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_vertex_program2_option
    //////////////////////////////////////////////////////////////////////////
    apGL_MAX_PROGRAM_EXEC_INSTRUCTIONS_NV,
    apGL_MAX_PROGRAM_CALL_DEPTH_NV,

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_shader_buffer_load
    //////////////////////////////////////////////////////////////////////////
    apGL_MAX_SHADER_BUFFER_ADDRESS_NV,

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_vertex_buffer_unified_memory
    //////////////////////////////////////////////////////////////////////////
    apGL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV,
    apGL_ELEMENT_ARRAY_UNIFIED_NV,
    apGL_VERTEX_ARRAY_LENGTH_NV,
    apGL_NORMAL_ARRAY_LENGTH_NV,
    apGL_COLOR_ARRAY_LENGTH_NV,
    apGL_INDEX_ARRAY_LENGTH_NV,
    apGL_EDGE_FLAG_ARRAY_LENGTH_NV,
    apGL_SECONDARY_COLOR_ARRAY_LENGTH_NV,
    apGL_FOG_COORD_ARRAY_LENGTH_NV,
    apGL_ELEMENT_ARRAY_LENGTH_NV,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_debug_output (also AMD and AMDX versions)
    //////////////////////////////////////////////////////////////////////////
    apGL_MAX_DEBUG_MESSAGE_LENGTH_ARB,
    apGL_MAX_DEBUG_LOGGED_MESSAGES_ARB,
    apGL_DEBUG_LOGGED_MESSAGES_ARB,
    apGL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB,
    apGL_DEBUG_CALLBACK_FUNCTION_ARB,
    apGL_DEBUG_CALLBACK_USER_PARAM_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_draw_buffers
    //////////////////////////////////////////////////////////////////////////
    apGL_MAX_DRAW_BUFFERS_ARB,
    apGL_DRAW_BUFFER0_ARB,
    apGL_DRAW_BUFFER1_ARB,
    apGL_DRAW_BUFFER2_ARB,
    apGL_DRAW_BUFFER3_ARB,
    apGL_DRAW_BUFFER4_ARB,
    apGL_DRAW_BUFFER5_ARB,
    apGL_DRAW_BUFFER6_ARB,
    apGL_DRAW_BUFFER7_ARB,
    apGL_DRAW_BUFFER8_ARB,
    apGL_DRAW_BUFFER9_ARB,
    apGL_DRAW_BUFFER10_ARB,
    apGL_DRAW_BUFFER11_ARB,
    apGL_DRAW_BUFFER12_ARB,
    apGL_DRAW_BUFFER13_ARB,
    apGL_DRAW_BUFFER14_ARB,
    apGL_DRAW_BUFFER15_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_ATI_draw_buffers
    //////////////////////////////////////////////////////////////////////////
    apGL_MAX_DRAW_BUFFERS_ATI,
    apGL_DRAW_BUFFER0_ATI,
    apGL_DRAW_BUFFER1_ATI,
    apGL_DRAW_BUFFER2_ATI,
    apGL_DRAW_BUFFER3_ATI,
    apGL_DRAW_BUFFER4_ATI,
    apGL_DRAW_BUFFER5_ATI,
    apGL_DRAW_BUFFER6_ATI,
    apGL_DRAW_BUFFER7_ATI,
    apGL_DRAW_BUFFER8_ATI,
    apGL_DRAW_BUFFER9_ATI,
    apGL_DRAW_BUFFER10_ATI,
    apGL_DRAW_BUFFER11_ATI,
    apGL_DRAW_BUFFER12_ATI,
    apGL_DRAW_BUFFER13_ATI,
    apGL_DRAW_BUFFER14_ATI,
    apGL_DRAW_BUFFER15_ATI,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_color_buffer_float
    //////////////////////////////////////////////////////////////////////////
    apGL_RGBA_FLOAT_MODE_ARB,
    apGL_CLAMP_VERTEX_COLOR_ARB,
    apGL_CLAMP_FRAGMENT_COLOR_ARB,
    apGL_CLAMP_READ_COLOR_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_point_sprite
    //////////////////////////////////////////////////////////////////////////
    apGL_POINT_SPRITE_ARB,
    apGL_COORD_REPLACE_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_stencil_two_side
    //////////////////////////////////////////////////////////////////////////
    apGL_STENCIL_TEST_TWO_SIDE_EXT,
    apGL_ACTIVE_STENCIL_FACE_EXT,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_multitexture
    //////////////////////////////////////////////////////////////////////////
    apGL_ACTIVE_TEXTURE_ARB,
    apGL_CLIENT_ACTIVE_TEXTURE_ARB,
    apGL_MAX_TEXTURE_UNITS_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_blend_logic_op and GL_EXT_blend_minMAX
    //////////////////////////////////////////////////////////////////////////
    apGL_BLEND_EQUATION_EXT,

    //////////////////////////////////////////////////////////////////////////
    // GL_SGIX_interlace
    //////////////////////////////////////////////////////////////////////////
    apGL_INTERLACE_SGIX,

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_framebuffer_object
    //////////////////////////////////////////////////////////////////////////
    apGL_FRAMEBUFFER_BINDING_EXT,
    apGL_RENDERBUFFER_BINDING_EXT,
    apGL_MAX_RENDERBUFFER_SIZE_EXT,

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_framebuffer_blit
    //////////////////////////////////////////////////////////////////////////
    apGL_DRAW_FRAMEBUFFER_BINDING_EXT,
    apGL_READ_FRAMEBUFFER_BINDING_EXT,

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_framebuffer_multisample
    //////////////////////////////////////////////////////////////////////////
    apGL_MAX_SAMPLES_EXT,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_framebuffer_object
    //////////////////////////////////////////////////////////////////////////
    apGL_FRAMEBUFFER_BINDING,
    apGL_RENDERBUFFER_BINDING,
    apGL_MAX_RENDERBUFFER_SIZE,
    apGL_DRAW_FRAMEBUFFER_BINDING,
    apGL_READ_FRAMEBUFFER_BINDING,
    apGL_MAX_SAMPLES,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_texture_rectangle
    //////////////////////////////////////////////////////////////////////////
    apGL_TEXTURE_RECTANGLE_ARB,
    apGL_TEXTURE_BINDING_RECTANGLE_ARB,
    apGL_MAX_RECTANGLE_TEXTURE_SIZE_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_texture_rectangle
    //////////////////////////////////////////////////////////////////////////
    apGL_TEXTURE_RECTANGLE_NV,
    apGL_TEXTURE_BINDING_RECTANGLE_NV,
    apGL_MAX_RECTANGLE_TEXTURE_SIZE_NV,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_multisample
    //////////////////////////////////////////////////////////////////////////
    apGL_MULTISAMPLE_ARB,
    apGL_SAMPLE_ALPHA_TO_COVERAGE_ARB,
    apGL_SAMPLE_ALPHA_TO_ONE_ARB,
    apGL_SAMPLE_COVERAGE_ARB,
    apGL_SAMPLE_COVERAGE_INVERT_ARB,
    apGL_SAMPLE_COVERAGE_VALUE_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_geometry_shader4
    //////////////////////////////////////////////////////////////////////////
    apGL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT,
    apGL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT,
    apGL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT,
    apGL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT,
    apGL_MAX_GEOMETRY_VARYING_COMPONENTS_EXT,
    apGL_MAX_VERTEX_VARYING_COMPONENTS_EXT,
    apGL_MAX_VARYING_COMPONENTS_EXT,

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_texture_array
    //////////////////////////////////////////////////////////////////////////
    apGL_TEXTURE_BINDING_1D_ARRAY_EXT,
    apGL_TEXTURE_BINDING_2D_ARRAY_EXT,
    apGL_MAX_ARRAY_TEXTURE_LAYERS_EXT,

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_compiled_vertex_array
    //////////////////////////////////////////////////////////////////////////
    apGL_ARRAY_ELEMENT_LOCK_FIRST_EXT,
    apGL_ARRAY_ELEMENT_LOCK_COUNT_EXT,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_transpose_matrix
    //////////////////////////////////////////////////////////////////////////
    apGL_TRANSPOSE_MODELVIEW_MATRIX_ARB,
    apGL_TRANSPOSE_PROJECTION_MATRIX_ARB,
    apGL_TRANSPOSE_TEXTURE_MATRIX_ARB,
    apGL_TRANSPOSE_COLOR_MATRIX_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_point_parameters
    //////////////////////////////////////////////////////////////////////////
    apGL_POINT_SIZE_MIN_ARB,
    apGL_POINT_SIZE_MAX_ARB,
    apGL_POINT_FADE_THRESHOLD_SIZE_ARB,
    apGL_POINT_DISTANCE_ATTENUATION_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_texture_float
    //////////////////////////////////////////////////////////////////////////
    // Uri 22/9/08: These are not really state variables. If anything, they are
    // texture parameters, but the information they supply (how is the channel
    // data saved on in the OpenGL implementation) is petty, so we do not
    // support them. If you decide to support them, they are retrieved by
    // glGetTexLevelParameteriv.
    /*
    apGL_TEXTURE_RED_TYPE_ARB,
    apGL_TEXTURE_GREEN_TYPE_ARB,
    apGL_TEXTURE_BLUE_TYPE_ARB,
    apGL_TEXTURE_ALPHA_TYPE_ARB,
    apGL_TEXTURE_LUMINANCE_TYPE_ARB,
    apGL_TEXTURE_INTENSITY_TYPE_ARB,
    apGL_TEXTURE_DEPTH_TYPE_ARB,
    */

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_pixel_buffer_object
    //////////////////////////////////////////////////////////////////////////
    apGL_PIXEL_PACK_BUFFER_BINDING_ARB,
    apGL_PIXEL_UNPACK_BUFFER_BINDING_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_framebuffer_sRGB
    //////////////////////////////////////////////////////////////////////////
    apGL_FRAMEBUFFER_SRGB,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_geometry_shader4
    //////////////////////////////////////////////////////////////////////////
    apGL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB,
    apGL_MAX_GEOMETRY_OUTPUT_VERTICES_ARB,
    apGL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB,
    apGL_MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB,
    apGL_MAX_GEOMETRY_VARYING_COMPONENTS_ARB,
    apGL_MAX_VERTEX_VARYING_COMPONENTS_ARB,
    apGL_MAX_VARYING_COMPONENTS_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_instanced_arrays
    //////////////////////////////////////////////////////////////////////////
    // Uri, 09/09/08: The state variable identifier (GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ARB = 0x88FE) does not appear in glext.h
    // apGL_VERTEX_ATTRIB_ARRAY_DIVISOR_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_texture_buffer_object
    //////////////////////////////////////////////////////////////////////////
    apGL_TEXTURE_BINDING_BUFFER_ARB,
    apGL_TEXTURE_BUFFER_DATA_STORE_BINDING_EXT,
    apGL_TEXTURE_BUFFER_FORMAT_ARB,
    apGL_TEXTURE_BUFFER_ARB,
    apGL_MAX_TEXTURE_BUFFER_SIZE_ARB,

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_texture_buffer_object
    //////////////////////////////////////////////////////////////////////////
    apGL_TEXTURE_BINDING_BUFFER_EXT,
    apGL_TEXTURE_BUFFER_FORMAT_EXT,
    apGL_TEXTURE_BUFFER_EXT,
    apGL_MAX_TEXTURE_BUFFER_SIZE_EXT,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_vertex_array_object
    //////////////////////////////////////////////////////////////////////////
    apGL_VERTEX_ARRAY_BINDING,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_uniform_buffer_object
    //////////////////////////////////////////////////////////////////////////
    apGL_UNIFORM_BUFFER_BINDING,
    apGL_MAX_VERTEX_UNIFORM_BLOCKS,
    apGL_MAX_FRAGMENT_UNIFORM_BLOCKS,
    apGL_MAX_GEOMETRY_UNIFORM_BLOCKS,
    apGL_MAX_COMBINED_UNIFORM_BLOCKS,
    apGL_MAX_UNIFORM_BUFFER_BINDINGS,
    apGL_MAX_UNIFORM_BLOCK_SIZE,
    apGL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
    apGL_MAX_GEOMETRY_UNIFORM_COMPONENTS,
    apGL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS,
    apGL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS,
    apGL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS,
    apGL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,


    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_depth_clamp
    //////////////////////////////////////////////////////////////////////////
    apGL_DEPTH_CLAMP,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_provoking_vertex
    //////////////////////////////////////////////////////////////////////////
    apGL_PROVOKING_VERTEX,
    apGL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION,
    // TO_DO: OpenGL 3.2 Should GL_FIRST_VERTEX_CONVENTION and GL_LAST_VERTEX_CONVENTION
    // set by glProvokingVertex be added as state variable?

    //////////////////////////////////////////////////////////////////////////
    // AP_GL_ARB_seamless_cube_map
    //////////////////////////////////////////////////////////////////////////
    apGL_TEXTURE_CUBE_MAP_SEAMLESS,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_sync
    //////////////////////////////////////////////////////////////////////////
    apGL_MAX_SERVER_WAIT_TIMEOUT,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_texture_multisample
    //////////////////////////////////////////////////////////////////////////
    apGL_SAMPLE_MASK,
    // apGL_SAMPLE_MASK_VALUE,
    apGL_TEXTURE_BINDING_2D_MULTISAMPLE,
    apGL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY,
    apGL_MAX_SAMPLE_MASK_WORDS,
    apGL_MAX_COLOR_TEXTURE_SAMPLES,
    apGL_MAX_DEPTH_TEXTURE_SAMPLES,
    apGL_MAX_INTEGER_SAMPLES,

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_vertex_array_bgra
    //////////////////////////////////////////////////////////////////////////
    // Uri, 30/12/09 - This should be added, along with other values (_BUFFER_BINDING, _TYPE, _STRIDE, _POINTER)
    // on a vertex-attrib base (i.e. for each attrib separately). At any rate, it was only modified in this extension
    // not added here. It was added in OpenGL 2.0.
    // apGL_VERTEX_ATTRIB_ARRAY_SIZE,

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    // WGL extensions state variables:

    //////////////////////////////////////////////////////////////////////////
    // WGL_ARB_pixel_format_float
    //////////////////////////////////////////////////////////////////////////
    // See GL_ARB_color_buffer_float

    //////////////////////////////////////////////////////////////////////////
    // WGL_ARB_framebuffer_sRGB
    //////////////////////////////////////////////////////////////////////////
    // seeGL_ARB_framebuffer_sRGB

    //////////////////////////////////////////////////////////////////////////
    // WGL_EXT_swap_control
    //////////////////////////////////////////////////////////////////////////
    apWGL_SWAP_INTERVAL_EXT,
#endif

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_aux_depth_stencil
    // This extension is supported only on MAC OS
    //////////////////////////////////////////////////////////////////////////
    apGL_AUX_DEPTH_STENCIL_APPLE,
#endif

    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_client_storage
    //////////////////////////////////////////////////////////////////////////
    apGL_UNPACK_CLIENT_STORAGE_APPLE,

    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_element_array
    //////////////////////////////////////////////////////////////////////////
    apGL_ELEMENT_ARRAY_APPLE,
    apGL_ELEMENT_ARRAY_TYPE_APPLE,
    apGL_ELEMENT_ARRAY_POINTER_APPLE,

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_float_pixels
    //////////////////////////////////////////////////////////////////////////
    apGL_COLOR_FLOAT_APPLE,
#endif

    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_specular_vector
    //////////////////////////////////////////////////////////////////////////
    apGL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE,

    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_transform_hint
    //////////////////////////////////////////////////////////////////////////
    apGL_TRANSFORM_HINT_APPLE,

    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_vertex_array_object
    //////////////////////////////////////////////////////////////////////////
    apGL_VERTEX_ARRAY_BINDING_APPLE,

    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_vertex_array_range,
    //////////////////////////////////////////////////////////////////////////
    apGL_VERTEX_ARRAY_RANGE_APPLE,
    apGL_VERTEX_ARRAY_RANGE_POINTER_APPLE,
    apGL_VERTEX_ARRAY_RANGE_LENGTH_APPLE,

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_texture_integer
    //////////////////////////////////////////////////////////////////////////
    apGL_RGBA_INTEGER_MODE_EXT,

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_bindable_uniform
    //////////////////////////////////////////////////////////////////////////
    apGL_UNIFORM_BUFFER_BINDING_EXT,

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    // CGL
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    // Sigal 7.1.09: The parameters in comment for some reason do not work, and we weren't able to understand
    // what's their use, so currently we don't support it

    // CGL Context Parameters:
    ap_kCGLCPSwapRectangle,
    AP_FIRST_CGL_STATE_VAR_INDEX = ap_kCGLCPSwapRectangle,
    ap_kCGLCPSwapInterval,
    ap_kCGLCPDispatchTableSize,
    ap_kCGLCPClientStorage,
    ap_kCGLCPSurfaceTexture,
    ap_kCGLCPSurfaceOrder,
    //ap_kCGLCPSurfaceOpacity,
    //ap_kCGLCPSurfaceBackingSize,
    ap_kCGLCPSurfaceSurfaceVolatile,
    //ap_kCGLCPReclaimResources,
    ap_kCGLCPCurrentRendererID,
    ap_kCGLCPGPUVertexProcessing,
    ap_kCGLCPGPUFragmentProcessing,

    // CGL Context enable:
    ap_kCGLCESwapRectangle,
    ap_kCGLCERasterization,
    ap_kCGLCEStateValidation,
    //ap_kCGLCESurfaceBackingSize,
    ap_kCGLCEDisplayListOptimization,

    // CGL global options:
    ap_kCGLGOFormatCacheSize,
    ap_kCGLGOClearFormatCache,
    ap_kCGLGORetainRenderers,
    ap_kCGLGOResetLibrary,
    ap_kCGLGOUseErrorHandler,
    AP_LAST_CGL_STATE_VAR_INDEX = ap_kCGLGOUseErrorHandler,

#endif

    // OpenGL ES is currently only supported on Windows and Mac:
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //// OpenGL ES
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    // Below are OpenGL ES extensions state variables (not supported yet).

    //////////////////////////////////////////////////////////////////////////
    // OpenGL ES 1.1:
    //////////////////////////////////////////////////////////////////////////
    apGL_LIGHT0_emission,
    AP_FIRST_OGLES_ONLY_STATE_VAR_INDEX = apGL_LIGHT0_emission,
    apGL_LIGHT1_emission,
    apGL_LIGHT2_emission,
    apGL_LIGHT3_emission,
    apGL_LIGHT4_emission,
    apGL_LIGHT5_emission,
    apGL_LIGHT6_emission,
    apGL_LIGHT7_emission,

    //////////////////////////////////////////////////////////////////////////
    // OpenGL ES 2.0:
    //////////////////////////////////////////////////////////////////////////
    // The 8 variables below are part of OpenGL 4.1 support, so we clear them here to avoid duplication:
    // apGL_IMPLEMENTATION_COLOR_READ_TYPE,
    // apGL_IMPLEMENTATION_COLOR_READ_FORMAT,
    // apGL_MAX_VERTEX_UNIFORM_VECTORS,
    // apGL_MAX_VARYING_VECTORS,
    // apGL_MAX_FRAGMENT_UNIFORM_VECTORS,
    apGL_TEXTURE_BINDING_CUBE_MAP,
    apGL_MAX_CUBE_MAP_TEXTURE_SIZE,
    // apGL_SHADER_COMPILER,
    // apGL_SHADER_BINARY_FORMATS,
    // apGL_NUM_SHADER_BINARY_FORMATS,

    // TO_DO iPhone: Uri, 13/7/09 : Add shader precision formats:
    /*
    Each combination of one of each:
    GL_VERTEX_SHADER, GL_FRAGMENT_SHADER
    GL_LOW_FLOAT, GL_MEDIUM_FLOAT, GL_HIGH_FLOAT, GL_LOW_INT, GL_MEDIUM_INT, GL_HIGH_INT
    max, min, precision
    (see http://www.khronos.org/opengles/sdk/2.0/docs/man/glGetShaderPrecisionFormat.xml)
    */

    //////////////////////////////////////////////////////////////////////////
    // OES_matrix_palette
    //////////////////////////////////////////////////////////////////////////
    apGL_MAX_VERTEX_UNITS_OES,
    apGL_MAX_PALETTE_MATRICES_OES,
    apGL_MATRIX_PALETTE_OES,
    apGL_CURRENT_PALETTE_MATRIX_OES, // Does not appear in the windows OpenGL ES gl.h file :~)
    apGL_MATRIX_INDEX_ARRAY_OES,
    apGL_MATRIX_INDEX_ARRAY_SIZE_OES,
    apGL_MATRIX_INDEX_ARRAY_TYPE_OES,
    apGL_MATRIX_INDEX_ARRAY_STRIDE_OES,
    apGL_MATRIX_INDEX_ARRAY_POINTER_OES,
    apGL_MATRIX_INDEX_ARRAY_BUFFER_BINDING_OES,
    apGL_WEIGHT_ARRAY_OES,
    apGL_WEIGHT_ARRAY_SIZE_OES,
    apGL_WEIGHT_ARRAY_TYPE_OES,
    apGL_WEIGHT_ARRAY_STRIDE_OES,
    apGL_WEIGHT_ARRAY_POINTER_OES,
    apGL_WEIGHT_ARRAY_BUFFER_BINDING_OES,

    //////////////////////////////////////////////////////////////////////////
    // OES_point_sprite
    //////////////////////////////////////////////////////////////////////////
    apGL_POINT_SPRITE_OES,
    apGL_COORD_REPLACE_OES,

    //////////////////////////////////////////////////////////////////////////
    // OES_point_size_array
    //////////////////////////////////////////////////////////////////////////
    apGL_POINT_SIZE_ARRAY_OES,
    apGL_POINT_SIZE_ARRAY_TYPE_OES,
    apGL_POINT_SIZE_ARRAY_STRIDE_OES,
    apGL_POINT_SIZE_ARRAY_POINTER_OES,
    apGL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES,

    //////////////////////////////////////////////////////////////////////////
    // GL_OES_read_format
    //////////////////////////////////////////////////////////////////////////
    apGL_IMPLEMENTATION_COLOR_READ_TYPE_OES,
    apGL_IMPLEMENTATION_COLOR_READ_FORMAT_OES,
    AP_LAST_OGLES_ONLY_STATE_VAR_INDEX = apGL_IMPLEMENTATION_COLOR_READ_FORMAT_OES,
#endif // (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Holds the amount of supported state variables:
    apOpenGLStateVariablesAmount
};


#endif  // __APOPENGLSTATEVARIABLEID



