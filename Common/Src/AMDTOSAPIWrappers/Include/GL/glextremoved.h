//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file glextremoved.h
///
//=====================================================================
#ifndef __glextremoved_h_
#define __glextremoved_h_ 1

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////
// This file contains defintions removed from glext.h, starting //
// past version 65.                                             //
// They are included for backwards-compatibility with older     //
// OpenGL implemetations.                                       //
//////////////////////////////////////////////////////////////////


// OpenGL 3.0 definitions, removed since glext version 65
#define GL_DEPTH_BUFFER                   0x8223
#define GL_STENCIL_BUFFER                 0x8224

// OpenGL 3.1 definitions, removed since glext version 65
#define GL_TEXTURE_BUFFER_FORMAT          0x8C2E

// Extensions removed since glext version 65:
// GL_SGIX_impact_pixel_texture
// GL_SGIX_fog_scale
// GL_SGI_depth_pass_instrument
#ifndef GL_SGIX_impact_pixel_texture
#define GL_PIXEL_TEX_GEN_Q_CEILING_SGIX   0x8184
#define GL_PIXEL_TEX_GEN_Q_ROUND_SGIX     0x8185
#define GL_PIXEL_TEX_GEN_Q_FLOOR_SGIX     0x8186
#define GL_PIXEL_TEX_GEN_ALPHA_REPLACE_SGIX 0x8187
#define GL_PIXEL_TEX_GEN_ALPHA_NO_REPLACE_SGIX 0x8188
#define GL_PIXEL_TEX_GEN_ALPHA_LS_SGIX    0x8189
#define GL_PIXEL_TEX_GEN_ALPHA_MS_SGIX    0x818A
#endif

#ifndef GL_SGIX_fog_scale
#define GL_FOG_SCALE_SGIX                 0x81FC
#define GL_FOG_SCALE_VALUE_SGIX           0x81FD
#endif

#ifndef GL_SGI_depth_pass_instrument
#define GL_DEPTH_PASS_INSTRUMENT_SGIX     0x8310
#define GL_DEPTH_PASS_INSTRUMENT_COUNTERS_SGIX 0x8311
#define GL_DEPTH_PASS_INSTRUMENT_MAX_SGIX 0x8312
#endif

#ifndef GL_AMDX_debug_output
GLAPI void APIENTRY glDebugMessageEnableAMDX(GLenum category, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
GLAPI void APIENTRY glDebugMessageInsertAMDX(GLenum category, GLenum severity, GLuint id, GLsizei length, const GLchar* buf);
GLAPI void APIENTRY glDebugMessageCallbackAMDX(GLDEBUGPROCAMD callback, GLvoid* userParam);
GLAPI GLuint APIENTRY glGetDebugMessageLogAMDX(GLuint count, GLsizei bufsize, GLenum* categories, GLuint* severities, GLuint* ids, GLsizei* lengths, GLchar* message);
#endif

// Possibly typos?
GLAPI void APIENTRY glNamedMakeBufferResidentNV(GLuint buffer, GLenum access);
GLAPI void APIENTRY glNamedMakeBufferNonResidentNV(GLuint buffer);

// OpenGL 4.3 API functions:
GLAPI void APIENTRY glFramebufferTextureFace(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face);

// OpenGL 4.3 definitions renamed between 4.4 and 4.5 headers:
#define GL_MAX_COMPUTE_LOCAL_INVOCATIONS 0x90EB /* Renamed to GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS */
#define GL_COMPUTE_LOCAL_WORK_SIZE 0x8267 /* Renamed from GL_COMPUTE_WORK_GROUP_SIZE */

// GL_ARB_sparse_texture definition removed between 4.4 and 4.5 headers:
#define GL_MIN_SPARSE_LEVEL_ARB           0x919B

// GL_SGIX_resample values which were moved between 4.4 and 4.5 headers:
#define GL_PACK_RESAMPLE_SGIX_            0x842C
#define GL_UNPACK_RESAMPLE_SGIX_          0x842D
#define GL_RESAMPLE_REPLICATE_SGIX_       0x842E
#define GL_RESAMPLE_ZERO_FILL_SGIX_       0x842F

// AMD Driver internal APIs:
#ifndef GLAPIENTRY
#define ADDED_GLAPIENTRY 1
#define GLAPIENTRY APIENTRY
#endif
GLAPI unsigned int GLAPIENTRY _loader_get_dispatch_table_size(void);
GLAPI int GLAPIENTRY _loader_get_proc_offset(const char* name);
GLAPI int GLAPIENTRY _loader_add_dispatch(const char* const* names, const char* signature);
GLAPI void GLAPIENTRY _loader_set_dispatch(const void* dispTable);
typedef unsigned int(*_DRI2_GLAPI_GET_DISPATCH_TABLE_SIZE)(void);
typedef int(*_DRI2_GLAPI_GET_PROC_OFFSET)(const char* name);
typedef int(*_DRI2_GLAPI_ADD_DISPATCH)(const char* const* names, const char* signature);
typedef void(*_DRI2_GLAPI_SET_DISPATCH)(const void* dispTable);
#ifdef ADDED_GLAPIENTRY
#undef GLAPIENTRY
#undef ADDED_GLAPIENTRY
#endif

// MESA Driver internal APIs:
// glapi.h

// If we want to intercept / use it, the definition of the _glapi_table struct can be found in
// the MESA glapitable.h header. See https://dri.freedesktop.org/doxygen/mesa/glapi/struct__glapi__table.html
struct _glapi_table;
typedef void(*_glapi_proc)(void); /* generic function pointer */
typedef void(*_glapi_warning_func)(void *ctx, const char *str, ...);

void _glapi_noop_enable_warnings(GLboolean enable);
void _glapi_set_warning_func(_glapi_warning_func func);
void _glapi_check_multithread(void);
void _glapi_set_context(void *context); 
void * _glapi_get_context(void);
void _glapi_set_dispatch(struct _glapi_table *dispatch); 
struct _glapi_table * _glapi_get_dispatch(void);
int _glapi_begin_dispatch_override(struct _glapi_table *override); 
void _glapi_end_dispatch_override(int layer);
_glapi_table * _glapi_get_override_dispatch(int layer);
GLuint _glapi_get_dispatch_table_size(void);
void _glapi_check_table(const struct _glapi_table *table);
int _glapi_add_dispatch(const char * const * function_names, const char * parameter_signature);
GLint _glapi_get_proc_offset(const char *funcName);
_glapi_proc _glapi_get_proc_address(const char *funcName);
const char * _glapi_get_proc_name(GLuint offset);

#ifdef __cplusplus
}
#endif

#endif // __glextremoved_h_
