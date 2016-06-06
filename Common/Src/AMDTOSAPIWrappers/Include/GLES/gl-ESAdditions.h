//=====================================================================
// Copyright 2005-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gl-ESAdditions.h
/// \brief This file contains functions and definitions that appear in OpenGL ES gl.h file and not in OpenGL gl.h file
///        It enables including both OpenGLES and OpenGL headers
///
//=====================================================================
#ifndef __gl_ESAdditions_h_
#define __gl_ESAdditions_h_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(AEE_SIMULATOR)
#define __GL_EXPORTS
#endif

#ifdef _WIN32
#   ifdef __GL_EXPORTS
#       define GL_API __declspec(dllexport)
#   else
#       define GL_API __declspec(dllimport)
#   endif
#else
#   ifdef __GL_EXPORTS
#       define GL_API
#   else
#       define GL_API extern
#   endif
#endif


/*
    Yaki 29.11.05:
    On Win32, changed OpenGL ES calling convention to be the same
    as the OpenGL calling convention.
*/
#ifdef WIN32
#define GL_APIENTRY APIENTRY
#else
#define GL_APIENTRY
#endif

#ifndef GLAPI
#   define GLAPI GL_API
#endif


/* Data types: */
typedef int             GLfixed;
typedef int             GLclampx;
#define GL_FIXED                          0x140C

/* OpenGL ES core versions */
#define GL_OES_VERSION_1_0                1
#define GL_OES_VERSION_1_1                1

/* Extensions */
#define GL_OES_byte_coordinates           1
#define GL_OES_compressed_paletted_texture 1
#define GL_OES_draw_texture               1
#define GL_OES_fixed_point                1
#define GL_OES_matrix_get                 1
#define GL_OES_matrix_palette             1
#define GL_OES_point_size_array           1
#define GL_OES_point_sprite               1
#define GL_OES_read_format                1
#define GL_OES_single_precision           1


/*****************************************************************************************/
/*                                 OES extension functions                               */
/*****************************************************************************************/

/* OES_draw_texture */
#define GL_TEXTURE_CROP_RECT_OES          0x8B9D

/* OES_matrix_get */
#define GL_MODELVIEW_MATRIX_FLOAT_AS_INT_BITS_OES   0x898D
#define GL_PROJECTION_MATRIX_FLOAT_AS_INT_BITS_OES  0x898E
#define GL_TEXTURE_MATRIX_FLOAT_AS_INT_BITS_OES     0x898F

/* OES_matrix_palette */
#define GL_MAX_VERTEX_UNITS_OES           0x86A4
#define GL_MAX_PALETTE_MATRICES_OES       0x8842
#define GL_MATRIX_PALETTE_OES             0x8840
#define GL_MATRIX_INDEX_ARRAY_OES         0x8844
#define GL_WEIGHT_ARRAY_OES               0x86AD
#define GL_CURRENT_PALETTE_MATRIX_OES     0x8843 // This definition is for some reason missing from the Windows OpenGL ES header.

#define GL_MATRIX_INDEX_ARRAY_SIZE_OES    0x8846
#define GL_MATRIX_INDEX_ARRAY_TYPE_OES    0x8847
#define GL_MATRIX_INDEX_ARRAY_STRIDE_OES  0x8848
#define GL_MATRIX_INDEX_ARRAY_POINTER_OES 0x8849
#define GL_MATRIX_INDEX_ARRAY_BUFFER_BINDING_OES 0x8B9E

#define GL_WEIGHT_ARRAY_SIZE_OES          0x86AB
#define GL_WEIGHT_ARRAY_TYPE_OES          0x86A9
#define GL_WEIGHT_ARRAY_STRIDE_OES        0x86AA
#define GL_WEIGHT_ARRAY_POINTER_OES       0x86AC
#define GL_WEIGHT_ARRAY_BUFFER_BINDING_OES 0x889E

/* OES_point_size_array */
#define GL_POINT_SIZE_ARRAY_OES           0x8B9C
#define GL_POINT_SIZE_ARRAY_TYPE_OES      0x898A
#define GL_POINT_SIZE_ARRAY_STRIDE_OES    0x898B
#define GL_POINT_SIZE_ARRAY_POINTER_OES   0x898C
#define GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES 0x8B9F

/* OES_point_sprite */
#define GL_POINT_SPRITE_OES               0x8861
#define GL_COORD_REPLACE_OES              0x8862

/* GL_OES_compressed_paletted_texture */
#define GL_PALETTE4_RGB8_OES                                    0x8B90
#define GL_PALETTE4_RGBA8_OES                                   0x8B91
#define GL_PALETTE4_R5_G6_B5_OES                                0x8B92
#define GL_PALETTE4_RGBA4_OES                                   0x8B93
#define GL_PALETTE4_RGB5_A1_OES                                 0x8B94
#define GL_PALETTE8_RGB8_OES                                    0x8B95
#define GL_PALETTE8_RGBA8_OES                                   0x8B96
#define GL_PALETTE8_R5_G6_B5_OES                                0x8B97
#define GL_PALETTE8_RGBA4_OES                                   0x8B98
#define GL_PALETTE8_RGB5_A1_OES                                 0x8B99

/*
    Uri, 13/7/09: OpenGL ES 2.0 enums. Some of these might be added to OpenGL in the future,
    so they can be removed from here then.
*/
#define GL_IMPLEMENTATION_COLOR_READ_TYPE   0x8B9A
#define GL_IMPLEMENTATION_COLOR_READ_FORMAT 0x8B9B
#define GL_MAX_VERTEX_UNIFORM_VECTORS     0x8DFB
#define GL_MAX_VARYING_VECTORS            0x8DFC
#define GL_MAX_FRAGMENT_UNIFORM_VECTORS   0x8DFD
#define GL_SHADER_COMPILER                0x8DFA
#define GL_SHADER_BINARY_FORMATS          0x8DF8
#define GL_NUM_SHADER_BINARY_FORMATS      0x8DF9
#define GL_LOW_FLOAT                  0x8DF0
#define GL_MEDIUM_FLOAT               0x8DF1
#define GL_HIGH_FLOAT                 0x8DF2
#define GL_LOW_INT                    0x8DF3
#define GL_MEDIUM_INT                 0x8DF4
#define GL_HIGH_INT                   0x8DF5
#define GL_RGB565                                           0x8D62
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS                0x8CD9


/*****************************************************************************************/
/*                                 OES core functions                                    */
/*****************************************************************************************/

GL_API void GL_APIENTRY glAlphaFuncx(GLenum func, GLclampx ref);
GL_API void GL_APIENTRY glClearColorx(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha);
// GL_API void GL_APIENTRY glClearDepthf (GLclampf depth); // Already appears in glext.h for OpenGL 4.1
GL_API void GL_APIENTRY glClearDepthx(GLclampx depth);
GL_API void GL_APIENTRY glClipPlanef(GLenum plane, const GLfloat* equation);
GL_API void GL_APIENTRY glClipPlanex(GLenum plane, const GLfixed* equation);
GL_API void GL_APIENTRY glColor4x(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
// GL_API void GL_APIENTRY glDepthRangef (GLclampf zNear, GLclampf zFar); // Already appears in glext.h for OpenGL 4.1
GL_API void GL_APIENTRY glDepthRangex(GLclampx zNear, GLclampx zFar);
GL_API void GL_APIENTRY glFogx(GLenum pname, GLfixed param);
GL_API void GL_APIENTRY glFogxv(GLenum pname, const GLfixed* params);
GL_API void GL_APIENTRY glFrustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
GL_API void GL_APIENTRY glFrustumx(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
GL_API void GL_APIENTRY glGetClipPlanef(GLenum pname, GLfloat eqn[4]);
GL_API void GL_APIENTRY glGetClipPlanex(GLenum pname, GLfixed eqn[4]);
GL_API void GL_APIENTRY glGetFixedv(GLenum pname, GLfixed* params);
GL_API void GL_APIENTRY glGetLightxv(GLenum light, GLenum pname, GLfixed* params);
GL_API void GL_APIENTRY glGetMaterialxv(GLenum face, GLenum pname, GLfixed* params);
GL_API void GL_APIENTRY glGetTexEnvxv(GLenum env, GLenum pname, GLfixed* params);
GL_API void GL_APIENTRY glGetTexParameterxv(GLenum target, GLenum pname, GLfixed* params);
GL_API void GL_APIENTRY glLightModelx(GLenum pname, GLfixed param);
GL_API void GL_APIENTRY glLightModelxv(GLenum pname, const GLfixed* params);
GL_API void GL_APIENTRY glLightx(GLenum light, GLenum pname, GLfixed param);
GL_API void GL_APIENTRY glLightxv(GLenum light, GLenum pname, const GLfixed* params);
GL_API void GL_APIENTRY glLineWidthx(GLfixed width);
GL_API void GL_APIENTRY glLoadMatrixx(const GLfixed* m);
GL_API void GL_APIENTRY glMaterialx(GLenum face, GLenum pname, GLfixed param);
GL_API void GL_APIENTRY glMaterialxv(GLenum face, GLenum pname, const GLfixed* params);
GL_API void GL_APIENTRY glMultMatrixx(const GLfixed* m);
GL_API void GL_APIENTRY glMultiTexCoord4x(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q);
GL_API void GL_APIENTRY glNormal3x(GLfixed nx, GLfixed ny, GLfixed nz);
GL_API void GL_APIENTRY glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
GL_API void GL_APIENTRY glOrthox(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
GL_API void GL_APIENTRY glPointParameterx(GLenum pname, GLfixed param);
GL_API void GL_APIENTRY glPointParameterxv(GLenum pname, const GLfixed* params);
GL_API void GL_APIENTRY glPointSizex(GLfixed size);
GL_API void GL_APIENTRY glPolygonOffsetx(GLfixed factor, GLfixed units);
GL_API void GL_APIENTRY glRotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
GL_API void GL_APIENTRY glSampleCoveragex(GLclampx value, GLboolean invert);
GL_API void GL_APIENTRY glScalex(GLfixed x, GLfixed y, GLfixed z);
GL_API void GL_APIENTRY glTexEnvx(GLenum target, GLenum pname, GLfixed param);
GL_API void GL_APIENTRY glTexEnvxv(GLenum target, GLenum pname, const GLfixed* params);
GL_API void GL_APIENTRY glTexParameterx(GLenum target, GLenum pname, GLfixed param);
GL_API void GL_APIENTRY glTexParameterxv(GLenum target, GLenum pname, const GLfixed* params);
GL_API void GL_APIENTRY glTranslatex(GLfixed x, GLfixed y, GLfixed z);

/*****************************************************************************************/
/*                                 OES extension functions                               */
/*****************************************************************************************/
/* OES_matrix_palette */
GL_API void GL_APIENTRY glCurrentPaletteMatrixOES(GLuint matrixpaletteindex);
GL_API void GL_APIENTRY glLoadPaletteFromModelViewMatrixOES(void);
GL_API void GL_APIENTRY glMatrixIndexPointerOES(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
GL_API void GL_APIENTRY glWeightPointerOES(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);

/* OES_point_size_array */
GL_API void GL_APIENTRY glPointSizePointerOES(GLenum type, GLsizei stride, const GLvoid* pointer);

/* OES_draw_texture */
GL_API void GL_APIENTRY glDrawTexsOES(GLshort x, GLshort y, GLshort z, GLshort width, GLshort height);
GL_API void GL_APIENTRY glDrawTexiOES(GLint x, GLint y, GLint z, GLint width, GLint height);
GL_API void GL_APIENTRY glDrawTexxOES(GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height);

GL_API void GL_APIENTRY glDrawTexsvOES(const GLshort* coords);
GL_API void GL_APIENTRY glDrawTexivOES(const GLint* coords);
GL_API void GL_APIENTRY glDrawTexxvOES(const GLfixed* coords);

GL_API void GL_APIENTRY glDrawTexfOES(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height);
GL_API void GL_APIENTRY glDrawTexfvOES(const GLfloat* coords);


#ifdef __cplusplus
}
#endif

#endif /* __gl_ESAdditions_h_ */

