//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsMonitoredFunctionPointers.h
///
//==================================================================================

//------------------------------ gsMonitoredFunctionPointers.h ------------------------------

#ifndef __GSMONITOREDFUNCTIONPOINTERS
#define __GSMONITOREDFUNCTIONPOINTERS

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

#ifdef _GR_IPHONE_BUILD
    #include <objc/runtime.h>
    #include <AMDTOSWrappers/Include/osOSDefinitions.h>
    // This is copied from EAGL.h, which is an objective-c header, so we cannot include it here:
    typedef NSUInteger EAGLRenderingAPI;
    class EAGLSharegroup;
    class EAGLContext;
#endif

// ----------------------------------------------------------------------------------
// Struct Name:          gsMonitoredFunctionPointers
//
// General Description:
//   Contains pointers to the real implementation of the wrapped functions, and extension pointers.
//
// Author:               Yaki Tebeka
// Creation Date:        18/7/2004
// ----------------------------------------------------------------------------------
struct gsMonitoredFunctionPointers
{

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    // WGL functions:
    int (WINAPI* wglChoosePixelFormat)(HDC a, CONST PIXELFORMATDESCRIPTOR* b);
    BOOL (WINAPI* wglCopyContext)(HGLRC a, HGLRC b, UINT c);
    HGLRC(WINAPI* wglCreateContext)(HDC a);
    HGLRC(WINAPI* wglCreateLayerContext)(HDC a, int b);
    BOOL (WINAPI* wglDeleteContext)(HGLRC a);
    BOOL (WINAPI* wglDescribeLayerPlane)(HDC a, int b, int c, UINT d, LPLAYERPLANEDESCRIPTOR e);
    int (WINAPI* wglDescribePixelFormat)(HDC a, int b, UINT c, LPPIXELFORMATDESCRIPTOR d);
    HGLRC(WINAPI* wglGetCurrentContext)(void);
    HDC(WINAPI* wglGetCurrentDC)(void);
    PROC(WINAPI* wglGetDefaultProcAddress)(LPCSTR a);
    int (WINAPI* wglGetLayerPaletteEntries)(HDC a, int b, int c, int d, COLORREF* e);
    int (WINAPI* wglGetPixelFormat)(HDC a);
    PROC(WINAPI* wglGetProcAddress)(LPCSTR a);
    BOOL (WINAPI* wglMakeCurrent)(HDC a, HGLRC b);
    BOOL (WINAPI* wglRealizeLayerPalette)(HDC a, int b, BOOL c);
    int (WINAPI* wglSetLayerPaletteEntries)(HDC a, int b, int c, int d, CONST COLORREF* e);
    BOOL (WINAPI* wglSetPixelFormat)(HDC a, int b, CONST PIXELFORMATDESCRIPTOR* c);
    BOOL (WINAPI* wglShareLists)(HGLRC a, HGLRC b);
    BOOL (WINAPI* wglSwapBuffers)(HDC a);
    BOOL (WINAPI* wglSwapLayerBuffers)(HDC hdc, UINT fuPlanes);
    DWORD (WINAPI* wglSwapMultipleBuffers)(UINT a, const WGLSWAP* b);
    BOOL (WINAPI* wglUseFontBitmapsA)(HDC a, DWORD b, DWORD c, DWORD d);
    BOOL (WINAPI* wglUseFontBitmapsW)(HDC a, DWORD b, DWORD c, DWORD d);
    BOOL (WINAPI* wglUseFontOutlinesA)(HDC a, DWORD b, DWORD c, DWORD d, FLOAT e, FLOAT f, int g, LPGLYPHMETRICSFLOAT h);
    BOOL (WINAPI* wglUseFontOutlinesW)(HDC a, DWORD b, DWORD c, DWORD d, FLOAT e, FLOAT f, int g, LPGLYPHMETRICSFLOAT h);

#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)

    // GLX functions:
    XVisualInfo* (*glXChooseVisual)(Display* dpy, int screen, int* attribList);
    GLXContext(*glXCreateContext)(Display* dpy, XVisualInfo* vis, GLXContext shareList, Bool direct);
    void (*glXDestroyContext)(Display* dpy, GLXContext ctx);
    Bool(*glXMakeCurrent)(Display* dpy, GLXDrawable drawable, GLXContext ctx);
    void (*glXCopyContext)(Display* dpy, GLXContext src, GLXContext dst, unsigned long mask);
    void (*glXSwapBuffers)(Display* dpy, GLXDrawable drawable);
    GLXPixmap(*glXCreateGLXPixmap)(Display* dpy, XVisualInfo* vis, Pixmap pixmap);
    void (*glXDestroyGLXPixmap)(Display* dpy, GLXPixmap pix);
    Bool(*glXQueryExtension)(Display* dpy, int* errorBase, int* eventBase);
    Bool(*glXQueryVersion)(Display* dpy, int* major, int* minor);
    Bool(*glXIsDirect)(Display* dpy, GLXContext ctx);
    int (*glXGetConfig)(Display* dpy, XVisualInfo* vis, int attrib, int* value);
    GLXContext(*glXGetCurrentContext)(void);
    GLXDrawable(*glXGetCurrentDrawable)(void);
    void (*glXWaitGL)(void);
    void (*glXWaitX)(void);
    void (*glXUseXFont)(Font font, int first, int count, int listBase);
    const char* (*glXQueryExtensionsString)(Display* dpy, int screen);
    const char* (*glXQueryServerString)(Display* dpy, int screen, int name);
    const char* (*glXGetClientString)(Display* dpy, int name);
    Display* (*glXGetCurrentDisplay)(void);
    GLXFBConfig* (*glXChooseFBConfig)(Display* dpy, int screen, const int* attrib_list, int* nelements);
    int (*glXGetFBConfigAttrib)(Display* dpy, GLXFBConfig config, int attribute, int* value);
    GLXFBConfig* (*glXGetFBConfigs)(Display* dpy, int screen, int* nelements);
    XVisualInfo* (*glXGetVisualFromFBConfig)(Display* dpy, GLXFBConfig config);
    GLXWindow(*glXCreateWindow)(Display* dpy, GLXFBConfig config, Window win, const int* attrib_list);
    void (*glXDestroyWindow)(Display* dpy, GLXWindow win);
    GLXPixmap(*glXCreatePixmap)(Display* dpy, GLXFBConfig config, Pixmap pixmap, const int* attrib_list);
    void (*glXDestroyPixmap)(Display* dpy, GLXPixmap pixmap);
    GLXPbuffer(*glXCreatePbuffer)(Display* dpy, GLXFBConfig config, const int* attrib_list);
    void (*glXDestroyPbuffer)(Display* dpy, GLXPbuffer pbuf);
    void (*glXQueryDrawable)(Display* dpy, GLXDrawable draw, int attribute, unsigned int* value);
    GLXContext(*glXCreateNewContext)(Display* dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct);
    Bool(*glXMakeContextCurrent)(Display* display, GLXDrawable draw, GLXDrawable read, GLXContext ctx);
    GLXDrawable(*glXGetCurrentReadDrawable)(void);
    int (*glXQueryContext)(Display* dpy, GLXContext ctx, int attribute, int* value);
    void (*glXSelectEvent)(Display* dpy, GLXDrawable draw, unsigned long event_mask);
    void (*glXGetSelectedEvent)(Display* dpy, GLXDrawable draw, unsigned long* event_mask);
    void (*(*glXGetProcAddress)(const GLubyte* procname))(void);
    void (*(*glXGetProcAddressARB)(const GLubyte* procName))(void);
    GLXContext(*glXCreateContextAttribsARB)(Display* dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int* attrib_list);

    /*
    GLXContextID (*glXGetContextIDEXT) (const GLXContext ctx);
    GLXDrawable (*glXGetCurrentDrawableEXT) (void);
    GLXContext (*glXImportContextEXT) (Display *dpy, GLXContextID contextID);
    void (*glXFreeContextEXT) (Display *dpy, GLXContext ctx);
    int (*glXQueryContextInfoEXT) (Display *dpy, GLXContext ctx, int attribute, int *value);
    */

#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    // CGL is not relevant to the iPhone
#ifndef _GR_IPHONE_BUILD
    // CGL functions:
    CGLError(*CGLChoosePixelFormat)(const CGLPixelFormatAttribute* attribs, CGLPixelFormatObj* pix, GLint* npix);
    CGLError(*CGLDestroyPixelFormat)(CGLPixelFormatObj pix);
    CGLError(*CGLDescribePixelFormat)(CGLPixelFormatObj pix, GLint pix_num, CGLPixelFormatAttribute attrib, GLint* value);
    CGLError(*CGLCreateContext)(CGLPixelFormatObj pix, CGLContextObj share, CGLContextObj* ctx);
    CGLError(*CGLCopyContext)(CGLContextObj src, CGLContextObj dst, GLbitfield mask);
    CGLError(*CGLDestroyContext)(CGLContextObj ctx);
    CGLContextObj(*CGLGetCurrentContext)(void);
    CGLError(*CGLSetCurrentContext)(CGLContextObj ctx);
    CGLError(*CGLEnable)(CGLContextObj ctx, CGLContextEnable pname);
    CGLError(*CGLDisable)(CGLContextObj ctx, CGLContextEnable pname);
    CGLError(*CGLIsEnabled)(CGLContextObj ctx, CGLContextEnable pname, GLint* enable);
    CGLError(*CGLSetParameter)(CGLContextObj ctx, CGLContextParameter pname, const GLint* params);
    CGLError(*CGLGetParameter)(CGLContextObj ctx, CGLContextParameter pname, GLint* params);
    CGLError(*CGLLockContext)(CGLContextObj ctx);
    CGLError(*CGLUnlockContext)(CGLContextObj ctx);
    CGLError(*CGLSetOffScreen)(CGLContextObj ctx, GLsizei width, GLsizei height, GLint rowbytes, void* baseaddr);
    CGLError(*CGLGetOffScreen)(CGLContextObj ctx, GLsizei* width, GLsizei* height, GLint* rowbytes, void** baseaddr);
    CGLError(*CGLSetFullScreen)(CGLContextObj ctx);
    CGLError(*CGLClearDrawable)(CGLContextObj ctx);
    CGLError(*CGLFlushDrawable)(CGLContextObj ctx);
    CGLError(*CGLCreatePBuffer)(GLsizei width, GLsizei height, GLenum target, GLenum internalFormat, GLint max_level, CGLPBufferObj* pbuffer);
    CGLError(*CGLDescribePBuffer)(CGLPBufferObj obj, GLsizei* width, GLsizei* height, GLenum* target, GLenum* internalFormat, GLint* mipmap);
    CGLError(*CGLDestroyPBuffer)(CGLPBufferObj pbuffer);
    CGLError(*CGLGetPBuffer)(CGLContextObj ctx, CGLPBufferObj* pbuffer, GLenum* face, GLint* level, GLint* screen);
    CGLError(*CGLSetPBuffer)(CGLContextObj ctx, CGLPBufferObj pbuffer, GLenum face, GLint level, GLint screen);
    CGLError(*CGLTexImagePBuffer)(CGLContextObj ctx, CGLPBufferObj pbuffer, GLenum source);
    const char* (*CGLErrorString)(CGLError error);
    CGLError(*CGLSetOption)(CGLGlobalOption pname, GLint param);
    CGLError(*CGLGetOption)(CGLGlobalOption pname, GLint* param);
    void (*CGLGetVersion)(GLint* majorvers, GLint* minorvers);
    CGLError(*CGLDescribeRenderer)(CGLRendererInfoObj rend, GLint rend_num, CGLRendererProperty prop, GLint* value);
    CGLError(*CGLDestroyRendererInfo)(CGLRendererInfoObj rend);
    CGLError(*CGLQueryRendererInfo)(GLuint display_mask, CGLRendererInfoObj* rend, GLint* nrend);
    CGLError(*CGLSetVirtualScreen)(CGLContextObj ctx, GLint screen);
    CGLError(*CGLGetVirtualScreen)(CGLContextObj ctx, GLint* screen);

    //////////////////////////////////////////////////////////////////////////
    // Uri, 21/05/09:
    // If adding a function here, we need to add it below to the list as well,
    // so that these functions will be the same numbers as the other ones
    // (otherwise, the next function, glAccum, would have a different index on
    // each implementation, causing a dissonance between the ES spy and CodeXL
    //////////////////////////////////////////////////////////////////////////
#else //_GR_OPENGLES_IPHONE
    // CGL functions dummy pointer (needed so that the apFunctionId offset will be the same:
    void (*CGLChoosePixelFormat)();
    void (*CGLDestroyPixelFormat)();
    void (*CGLDescribePixelFormat)();
    void (*CGLCreateContext)();
    void (*CGLCopyContext)();
    void (*CGLDestroyContext)();
    void (*CGLGetCurrentContext)();
    void (*CGLSetCurrentContext)();
    void (*CGLEnable)();
    void (*CGLDisable)();
    void (*CGLIsEnabled)();
    void (*CGLSetParameter)();
    void (*CGLGetParameter)();
    void (*CGLLockContext)();
    void (*CGLUnlockContext)();
    void (*CGLSetOffScreen)();
    void (*CGLGetOffScreen)();
    void (*CGLSetFullScreen)();
    void (*CGLClearDrawable)();
    void (*CGLFlushDrawable)();
    void (*CGLCreatePBuffer)();
    void (*CGLDescribePBuffer)();
    void (*CGLDestroyPBuffer)();
    void (*CGLGetPBuffer)();
    void (*CGLSetPBuffer)();
    void (*CGLTexImagePBuffer)();
    void (*CGLErrorString)();
    void (*CGLSetOption)();
    void (*CGLGetOption)();
    void (*CGLGetVersion)();
    void (*CGLDescribeRenderer)();
    void (*CGLDestroyRendererInfo)();
    void (*CGLQueryRendererInfo)();
    void (*CGLSetVirtualScreen)();
    void (*CGLGetVirtualScreen)();

    //////////////////////////////////////////////////////////////////////////
    // Uri, 21/05/09:
    // If adding a function here, we need to add it above to the list as well,
    // so that these functions will be the same numbers as the other ones
    // (otherwise, the next function, glAccum, would have a different index on
    // each implementation, causing a dissonance between the ES spy and CodeXL
    //////////////////////////////////////////////////////////////////////////
#endif

#endif // AMDT_BUILD_TARGET
    // OpenGL functions:
    void (APIENTRY* glAccum)(GLenum op, GLfloat value);
    void (APIENTRY* glAlphaFunc)(GLenum func, GLclampf ref);
    GLboolean(APIENTRY* glAreTexturesResident)(GLsizei n, const GLuint* textures, GLboolean* residences);
    void (APIENTRY* glArrayElement)(GLint i);
    void (APIENTRY* glBegin)(GLenum mode);
    void (APIENTRY* glBindTexture)(GLenum target, GLuint texture);
    void (APIENTRY* glBitmap)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte* bitmap);
    void (APIENTRY* glBlendFunc)(GLenum sfactor, GLenum dfactor);
    void (APIENTRY* glCallList)(GLuint list);
    void (APIENTRY* glCallLists)(GLsizei n, GLenum type, const GLvoid* lists);
    void (APIENTRY* glClear)(GLbitfield mask);
    void (APIENTRY* glClearAccum)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void (APIENTRY* glClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
    void (APIENTRY* glClearDepth)(GLclampd depth);
    void (APIENTRY* glClearIndex)(GLfloat c);
    void (APIENTRY* glClearStencil)(GLint s);
    void (APIENTRY* glClipPlane)(GLenum plane, const GLdouble* equation);
    void (APIENTRY* glColor3b)(GLbyte red, GLbyte green, GLbyte blue);
    void (APIENTRY* glColor3bv)(const GLbyte* v);
    void (APIENTRY* glColor3d)(GLdouble red, GLdouble green, GLdouble blue);
    void (APIENTRY* glColor3dv)(const GLdouble* v);
    void (APIENTRY* glColor3f)(GLfloat red, GLfloat green, GLfloat blue);
    void (APIENTRY* glColor3fv)(const GLfloat* v);
    void (APIENTRY* glColor3i)(GLint red, GLint green, GLint blue);
    void (APIENTRY* glColor3iv)(const GLint* v);
    void (APIENTRY* glColor3s)(GLshort red, GLshort green, GLshort blue);
    void (APIENTRY* glColor3sv)(const GLshort* v);
    void (APIENTRY* glColor3ub)(GLubyte red, GLubyte green, GLubyte blue);
    void (APIENTRY* glColor3ubv)(const GLubyte* v);
    void (APIENTRY* glColor3ui)(GLuint red, GLuint green, GLuint blue);
    void (APIENTRY* glColor3uiv)(const GLuint* v);
    void (APIENTRY* glColor3us)(GLushort red, GLushort green, GLushort blue);
    void (APIENTRY* glColor3usv)(const GLushort* v);
    void (APIENTRY* glColor4b)(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
    void (APIENTRY* glColor4bv)(const GLbyte* v);
    void (APIENTRY* glColor4d)(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
    void (APIENTRY* glColor4dv)(const GLdouble* v);
    void (APIENTRY* glColor4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void (APIENTRY* glColor4fv)(const GLfloat* v);
    void (APIENTRY* glColor4i)(GLint red, GLint green, GLint blue, GLint alpha);
    void (APIENTRY* glColor4iv)(const GLint* v);
    void (APIENTRY* glColor4s)(GLshort red, GLshort green, GLshort blue, GLshort alpha);
    void (APIENTRY* glColor4sv)(const GLshort* v);
    void (APIENTRY* glColor4ub)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
    void (APIENTRY* glColor4ubv)(const GLubyte* v);
    void (APIENTRY* glColor4ui)(GLuint red, GLuint green, GLuint blue, GLuint alpha);
    void (APIENTRY* glColor4uiv)(const GLuint* v);
    void (APIENTRY* glColor4us)(GLushort red, GLushort green, GLushort blue, GLushort alpha);
    void (APIENTRY* glColor4usv)(const GLushort* v);
    void (APIENTRY* glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
    void (APIENTRY* glColorMaterial)(GLenum face, GLenum mode);
    void (APIENTRY* glColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
    void (APIENTRY* glCopyPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
    void (APIENTRY* glCopyTexImage1D)(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
    void (APIENTRY* glCopyTexImage2D)(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
    void (APIENTRY* glCopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
    void (APIENTRY* glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void (APIENTRY* glCullFace)(GLenum mode);
    void (APIENTRY* glDeleteLists)(GLuint list, GLsizei range);
    void (APIENTRY* glDeleteTextures)(GLsizei n, const GLuint* textures);
    void (APIENTRY* glDepthFunc)(GLenum func);
    void (APIENTRY* glDepthMask)(GLboolean flag);
    void (APIENTRY* glDepthRange)(GLclampd zNear, GLclampd zFar);
    void (APIENTRY* glDisable)(GLenum cap);
    void (APIENTRY* glDisableClientState)(GLenum array);
    void (APIENTRY* glDrawArrays)(GLenum mode, GLint first, GLsizei count);
    void (APIENTRY* glDrawBuffer)(GLenum mode);
    void (APIENTRY* glDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
    void (APIENTRY* glDrawPixels)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
    void (APIENTRY* glEdgeFlag)(GLboolean flag);
    void (APIENTRY* glEdgeFlagPointer)(GLsizei stride, const GLvoid* pointer);
    void (APIENTRY* glEdgeFlagv)(const GLboolean* flag);
    void (APIENTRY* glEnable)(GLenum cap);
    void (APIENTRY* glEnableClientState)(GLenum array);
    void (APIENTRY* glEnd)(void);
    void (APIENTRY* glEndList)(void);
    void (APIENTRY* glEvalCoord1d)(GLdouble u);
    void (APIENTRY* glEvalCoord1dv)(const GLdouble* u);
    void (APIENTRY* glEvalCoord1f)(GLfloat u);
    void (APIENTRY* glEvalCoord1fv)(const GLfloat* u);
    void (APIENTRY* glEvalCoord2d)(GLdouble u, GLdouble v);
    void (APIENTRY* glEvalCoord2dv)(const GLdouble* u);
    void (APIENTRY* glEvalCoord2f)(GLfloat u, GLfloat v);
    void (APIENTRY* glEvalCoord2fv)(const GLfloat* u);
    void (APIENTRY* glEvalMesh1)(GLenum mode, GLint i1, GLint i2);
    void (APIENTRY* glEvalMesh2)(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
    void (APIENTRY* glEvalPoint1)(GLint i);
    void (APIENTRY* glEvalPoint2)(GLint i, GLint j);
    void (APIENTRY* glFeedbackBuffer)(GLsizei size, GLenum type, GLfloat* buffer);
    void (APIENTRY* glFinish)(void);
    void (APIENTRY* glFlush)(void);
    void (APIENTRY* glFogf)(GLenum pname, GLfloat param);
    void (APIENTRY* glFogfv)(GLenum pname, const GLfloat* params);
    void (APIENTRY* glFogi)(GLenum pname, GLint param);
    void (APIENTRY* glFogiv)(GLenum pname, const GLint* params);
    void (APIENTRY* glFrontFace)(GLenum mode);
    void (APIENTRY* glFrustum)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
    GLuint(APIENTRY* glGenLists)(GLsizei range);
    void (APIENTRY* glGenTextures)(GLsizei n, GLuint* textures);
    void (APIENTRY* glGetBooleanv)(GLenum pname, GLboolean* params);
    void (APIENTRY* glGetClipPlane)(GLenum plane, GLdouble* equation);
    void (APIENTRY* glGetDoublev)(GLenum pname, GLdouble* params);
    GLenum(APIENTRY* glGetError)(void);
    void (APIENTRY* glGetFloatv)(GLenum pname, GLfloat* params);
    void (APIENTRY* glGetIntegerv)(GLenum pname, GLint* params);
    void (APIENTRY* glGetLightfv)(GLenum light, GLenum pname, GLfloat* params);
    void (APIENTRY* glGetLightiv)(GLenum light, GLenum pname, GLint* params);
    void (APIENTRY* glGetMapdv)(GLenum target, GLenum query, GLdouble* v);
    void (APIENTRY* glGetMapfv)(GLenum target, GLenum query, GLfloat* v);
    void (APIENTRY* glGetMapiv)(GLenum target, GLenum query, GLint* v);
    void (APIENTRY* glGetMaterialfv)(GLenum face, GLenum pname, GLfloat* params);
    void (APIENTRY* glGetMaterialiv)(GLenum face, GLenum pname, GLint* params);
    void (APIENTRY* glGetPixelMapfv)(GLenum map, GLfloat* values);
    void (APIENTRY* glGetPixelMapuiv)(GLenum map, GLuint* values);
    void (APIENTRY* glGetPixelMapusv)(GLenum map, GLushort* values);
    void (APIENTRY* glGetPointerv)(GLenum pname, GLvoid** params);
    void (APIENTRY* glGetPolygonStipple)(GLubyte* mask);
    const GLubyte* (APIENTRY* glGetString)(GLenum name);
    void (APIENTRY* glGetTexEnvfv)(GLenum target, GLenum pname, GLfloat* params);
    void (APIENTRY* glGetTexEnviv)(GLenum target, GLenum pname, GLint* params);
    void (APIENTRY* glGetTexGendv)(GLenum coord, GLenum pname, GLdouble* params);
    void (APIENTRY* glGetTexGenfv)(GLenum coord, GLenum pname, GLfloat* params);
    void (APIENTRY* glGetTexGeniv)(GLenum coord, GLenum pname, GLint* params);
    void (APIENTRY* glGetTexImage)(GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels);
    void (APIENTRY* glGetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat* params);
    void (APIENTRY* glGetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint* params);
    void (APIENTRY* glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat* params);
    void (APIENTRY* glGetTexParameteriv)(GLenum target, GLenum pname, GLint* params);
    void (APIENTRY* glHint)(GLenum target, GLenum mode);
    void (APIENTRY* glIndexMask)(GLuint mask);
    void (APIENTRY* glIndexPointer)(GLenum type, GLsizei stride, const GLvoid* pointer);
    void (APIENTRY* glIndexd)(GLdouble c);
    void (APIENTRY* glIndexdv)(const GLdouble* c);
    void (APIENTRY* glIndexf)(GLfloat c);
    void (APIENTRY* glIndexfv)(const GLfloat* c);
    void (APIENTRY* glIndexi)(GLint c);
    void (APIENTRY* glIndexiv)(const GLint* c);
    void (APIENTRY* glIndexs)(GLshort c);
    void (APIENTRY* glIndexsv)(const GLshort* c);
    void (APIENTRY* glIndexub)(GLubyte c);
    void (APIENTRY* glIndexubv)(const GLubyte* c);
    void (APIENTRY* glInitNames)(void);
    void (APIENTRY* glInterleavedArrays)(GLenum format, GLsizei stride, const GLvoid* pointer);
    GLboolean(APIENTRY* glIsEnabled)(GLenum cap);
    GLboolean(APIENTRY* glIsList)(GLuint list);
    GLboolean(APIENTRY* glIsTexture)(GLuint texture);
    void (APIENTRY* glLightModelf)(GLenum pname, GLfloat param);
    void (APIENTRY* glLightModelfv)(GLenum pname, const GLfloat* params);
    void (APIENTRY* glLightModeli)(GLenum pname, GLint param);
    void (APIENTRY* glLightModeliv)(GLenum pname, const GLint* params);
    void (APIENTRY* glLightf)(GLenum light, GLenum pname, GLfloat param);
    void (APIENTRY* glLightfv)(GLenum light, GLenum pname, const GLfloat* params);
    void (APIENTRY* glLighti)(GLenum light, GLenum pname, GLint param);
    void (APIENTRY* glLightiv)(GLenum light, GLenum pname, const GLint* params);
    void (APIENTRY* glLineStipple)(GLint factor, GLushort pattern);
    void (APIENTRY* glLineWidth)(GLfloat width);
    void (APIENTRY* glListBase)(GLuint base);
    void (APIENTRY* glLoadIdentity)(void);
    void (APIENTRY* glLoadMatrixd)(const GLdouble* m);
    void (APIENTRY* glLoadMatrixf)(const GLfloat* m);
    void (APIENTRY* glLoadName)(GLuint name);
    void (APIENTRY* glLogicOp)(GLenum opcode);
    void (APIENTRY* glMap1d)(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble* points);
    void (APIENTRY* glMap1f)(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat* points);
    void (APIENTRY* glMap2d)(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble* points);
    void (APIENTRY* glMap2f)(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat* points);
    void (APIENTRY* glMapGrid1d)(GLint un, GLdouble u1, GLdouble u2);
    void (APIENTRY* glMapGrid1f)(GLint un, GLfloat u1, GLfloat u2);
    void (APIENTRY* glMapGrid2d)(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
    void (APIENTRY* glMapGrid2f)(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
    void (APIENTRY* glMaterialf)(GLenum face, GLenum pname, GLfloat param);
    void (APIENTRY* glMaterialfv)(GLenum face, GLenum pname, const GLfloat* params);
    void (APIENTRY* glMateriali)(GLenum face, GLenum pname, GLint param);
    void (APIENTRY* glMaterialiv)(GLenum face, GLenum pname, const GLint* params);
    void (APIENTRY* glMatrixMode)(GLenum mode);
    void (APIENTRY* glMultMatrixd)(const GLdouble* m);
    void (APIENTRY* glMultMatrixf)(const GLfloat* m);
    void (APIENTRY* glNewList)(GLuint list, GLenum mode);
    void (APIENTRY* glNormal3b)(GLbyte nx, GLbyte ny, GLbyte nz);
    void (APIENTRY* glNormal3bv)(const GLbyte* v);
    void (APIENTRY* glNormal3d)(GLdouble nx, GLdouble ny, GLdouble nz);
    void (APIENTRY* glNormal3dv)(const GLdouble* v);
    void (APIENTRY* glNormal3f)(GLfloat nx, GLfloat ny, GLfloat nz);
    void (APIENTRY* glNormal3fv)(const GLfloat* v);
    void (APIENTRY* glNormal3i)(GLint nx, GLint ny, GLint nz);
    void (APIENTRY* glNormal3iv)(const GLint* v);
    void (APIENTRY* glNormal3s)(GLshort nx, GLshort ny, GLshort nz);
    void (APIENTRY* glNormal3sv)(const GLshort* v);
    void (APIENTRY* glNormalPointer)(GLenum type, GLsizei stride, const GLvoid* pointer);
    void (APIENTRY* glOrtho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
    void (APIENTRY* glPassThrough)(GLfloat token);
    void (APIENTRY* glPixelMapfv)(GLenum map, GLsizei mapsize, const GLfloat* values);
    void (APIENTRY* glPixelMapuiv)(GLenum map, GLsizei mapsize, const GLuint* values);
    void (APIENTRY* glPixelMapusv)(GLenum map, GLsizei mapsize, const GLushort* values);
    void (APIENTRY* glPixelStoref)(GLenum pname, GLfloat param);
    void (APIENTRY* glPixelStorei)(GLenum pname, GLint param);
    void (APIENTRY* glPixelTransferf)(GLenum pname, GLfloat param);
    void (APIENTRY* glPixelTransferi)(GLenum pname, GLint param);
    void (APIENTRY* glPixelZoom)(GLfloat xfactor, GLfloat yfactor);
    void (APIENTRY* glPointSize)(GLfloat size);
    void (APIENTRY* glPolygonMode)(GLenum face, GLenum mode);
    void (APIENTRY* glPolygonOffset)(GLfloat factor, GLfloat units);
    void (APIENTRY* glPolygonStipple)(const GLubyte* mask);
    void (APIENTRY* glPopAttrib)(void);
    void (APIENTRY* glPopClientAttrib)(void);
    void (APIENTRY* glPopMatrix)(void);
    void (APIENTRY* glPopName)(void);
    void (APIENTRY* glPrioritizeTextures)(GLsizei n, const GLuint* textures, const GLclampf* priorities);
    void (APIENTRY* glPushAttrib)(GLbitfield mask);
    void (APIENTRY* glPushClientAttrib)(GLbitfield mask);
    void (APIENTRY* glPushMatrix)(void);
    void (APIENTRY* glPushName)(GLuint name);
    void (APIENTRY* glRasterPos2d)(GLdouble x, GLdouble y);
    void (APIENTRY* glRasterPos2dv)(const GLdouble* v);
    void (APIENTRY* glRasterPos2f)(GLfloat x, GLfloat y);
    void (APIENTRY* glRasterPos2fv)(const GLfloat* v);
    void (APIENTRY* glRasterPos2i)(GLint x, GLint y);
    void (APIENTRY* glRasterPos2iv)(const GLint* v);
    void (APIENTRY* glRasterPos2s)(GLshort x, GLshort y);
    void (APIENTRY* glRasterPos2sv)(const GLshort* v);
    void (APIENTRY* glRasterPos3d)(GLdouble x, GLdouble y, GLdouble z);
    void (APIENTRY* glRasterPos3dv)(const GLdouble* v);
    void (APIENTRY* glRasterPos3f)(GLfloat x, GLfloat y, GLfloat z);
    void (APIENTRY* glRasterPos3fv)(const GLfloat* v);
    void (APIENTRY* glRasterPos3i)(GLint x, GLint y, GLint z);
    void (APIENTRY* glRasterPos3iv)(const GLint* v);
    void (APIENTRY* glRasterPos3s)(GLshort x, GLshort y, GLshort z);
    void (APIENTRY* glRasterPos3sv)(const GLshort* v);
    void (APIENTRY* glRasterPos4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void (APIENTRY* glRasterPos4dv)(const GLdouble* v);
    void (APIENTRY* glRasterPos4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void (APIENTRY* glRasterPos4fv)(const GLfloat* v);
    void (APIENTRY* glRasterPos4i)(GLint x, GLint y, GLint z, GLint w);
    void (APIENTRY* glRasterPos4iv)(const GLint* v);
    void (APIENTRY* glRasterPos4s)(GLshort x, GLshort y, GLshort z, GLshort w);
    void (APIENTRY* glRasterPos4sv)(const GLshort* v);
    void (APIENTRY* glReadBuffer)(GLenum mode);
    void (APIENTRY* glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
    void (APIENTRY* glRectd)(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
    void (APIENTRY* glRectdv)(const GLdouble* v1, const GLdouble* v2);
    void (APIENTRY* glRectf)(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
    void (APIENTRY* glRectfv)(const GLfloat* v1, const GLfloat* v2);
    void (APIENTRY* glRecti)(GLint x1, GLint y1, GLint x2, GLint y2);
    void (APIENTRY* glRectiv)(const GLint* v1, const GLint* v2);
    void (APIENTRY* glRects)(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
    void (APIENTRY* glRectsv)(const GLshort* v1, const GLshort* v2);
    GLint(APIENTRY* glRenderMode)(GLenum mode);
    void (APIENTRY* glRotated)(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
    void (APIENTRY* glRotatef)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
    void (APIENTRY* glScaled)(GLdouble x, GLdouble y, GLdouble z);
    void (APIENTRY* glScalef)(GLfloat x, GLfloat y, GLfloat z);
    void (APIENTRY* glScissor)(GLint x, GLint y, GLsizei width, GLsizei height);
    void (APIENTRY* glSelectBuffer)(GLsizei size, GLuint* buffer);
    void (APIENTRY* glShadeModel)(GLenum mode);
    void (APIENTRY* glStencilFunc)(GLenum func, GLint ref, GLuint mask);
    void (APIENTRY* glStencilMask)(GLuint mask);
    void (APIENTRY* glStencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
    void (APIENTRY* glTexCoord1d)(GLdouble s);
    void (APIENTRY* glTexCoord1dv)(const GLdouble* v);
    void (APIENTRY* glTexCoord1f)(GLfloat s);
    void (APIENTRY* glTexCoord1fv)(const GLfloat* v);
    void (APIENTRY* glTexCoord1i)(GLint s);
    void (APIENTRY* glTexCoord1iv)(const GLint* v);
    void (APIENTRY* glTexCoord1s)(GLshort s);
    void (APIENTRY* glTexCoord1sv)(const GLshort* v);
    void (APIENTRY* glTexCoord2d)(GLdouble s, GLdouble t);
    void (APIENTRY* glTexCoord2dv)(const GLdouble* v);
    void (APIENTRY* glTexCoord2f)(GLfloat s, GLfloat t);
    void (APIENTRY* glTexCoord2fv)(const GLfloat* v);
    void (APIENTRY* glTexCoord2i)(GLint s, GLint t);
    void (APIENTRY* glTexCoord2iv)(const GLint* v);
    void (APIENTRY* glTexCoord2s)(GLshort s, GLshort t);
    void (APIENTRY* glTexCoord2sv)(const GLshort* v);
    void (APIENTRY* glTexCoord3d)(GLdouble s, GLdouble t, GLdouble r);
    void (APIENTRY* glTexCoord3dv)(const GLdouble* v);
    void (APIENTRY* glTexCoord3f)(GLfloat s, GLfloat t, GLfloat r);
    void (APIENTRY* glTexCoord3fv)(const GLfloat* v);
    void (APIENTRY* glTexCoord3i)(GLint s, GLint t, GLint r);
    void (APIENTRY* glTexCoord3iv)(const GLint* v);
    void (APIENTRY* glTexCoord3s)(GLshort s, GLshort t, GLshort r);
    void (APIENTRY* glTexCoord3sv)(const GLshort* v);
    void (APIENTRY* glTexCoord4d)(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
    void (APIENTRY* glTexCoord4dv)(const GLdouble* v);
    void (APIENTRY* glTexCoord4f)(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
    void (APIENTRY* glTexCoord4fv)(const GLfloat* v);
    void (APIENTRY* glTexCoord4i)(GLint s, GLint t, GLint r, GLint q);
    void (APIENTRY* glTexCoord4iv)(const GLint* v);
    void (APIENTRY* glTexCoord4s)(GLshort s, GLshort t, GLshort r, GLshort q);
    void (APIENTRY* glTexCoord4sv)(const GLshort* v);
    void (APIENTRY* glTexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
    void (APIENTRY* glTexEnvf)(GLenum target, GLenum pname, GLfloat param);
    void (APIENTRY* glTexEnvfv)(GLenum target, GLenum pname, const GLfloat* params);
    void (APIENTRY* glTexEnvi)(GLenum target, GLenum pname, GLint param);
    void (APIENTRY* glTexEnviv)(GLenum target, GLenum pname, const GLint* params);
    void (APIENTRY* glTexGend)(GLenum coord, GLenum pname, GLdouble param);
    void (APIENTRY* glTexGendv)(GLenum coord, GLenum pname, const GLdouble* params);
    void (APIENTRY* glTexGenf)(GLenum coord, GLenum pname, GLfloat param);
    void (APIENTRY* glTexGenfv)(GLenum coord, GLenum pname, const GLfloat* params);
    void (APIENTRY* glTexGeni)(GLenum coord, GLenum pname, GLint param);
    void (APIENTRY* glTexGeniv)(GLenum coord, GLenum pname, const GLint* params);
    void (APIENTRY* glTexImage1D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
    void (APIENTRY* glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
    void (APIENTRY* glTexParameterf)(GLenum target, GLenum pname, GLfloat param);
    void (APIENTRY* glTexParameterfv)(GLenum target, GLenum pname, const GLfloat* params);
    void (APIENTRY* glTexParameteri)(GLenum target, GLenum pname, GLint param);
    void (APIENTRY* glTexParameteriv)(GLenum target, GLenum pname, const GLint* params);
    void (APIENTRY* glTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid* pixels);
    void (APIENTRY* glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
    void (APIENTRY* glTranslated)(GLdouble x, GLdouble y, GLdouble z);
    void (APIENTRY* glTranslatef)(GLfloat x, GLfloat y, GLfloat z);
    void (APIENTRY* glVertex2d)(GLdouble x, GLdouble y);
    void (APIENTRY* glVertex2dv)(const GLdouble* v);
    void (APIENTRY* glVertex2f)(GLfloat x, GLfloat y);
    void (APIENTRY* glVertex2fv)(const GLfloat* v);
    void (APIENTRY* glVertex2i)(GLint x, GLint y);
    void (APIENTRY* glVertex2iv)(const GLint* v);
    void (APIENTRY* glVertex2s)(GLshort x, GLshort y);
    void (APIENTRY* glVertex2sv)(const GLshort* v);
    void (APIENTRY* glVertex3d)(GLdouble x, GLdouble y, GLdouble z);
    void (APIENTRY* glVertex3dv)(const GLdouble* v);
    void (APIENTRY* glVertex3f)(GLfloat x, GLfloat y, GLfloat z);
    void (APIENTRY* glVertex3fv)(const GLfloat* v);
    void (APIENTRY* glVertex3i)(GLint x, GLint y, GLint z);
    void (APIENTRY* glVertex3iv)(const GLint* v);
    void (APIENTRY* glVertex3s)(GLshort x, GLshort y, GLshort z);
    void (APIENTRY* glVertex3sv)(const GLshort* v);
    void (APIENTRY* glVertex4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void (APIENTRY* glVertex4dv)(const GLdouble* v);
    void (APIENTRY* glVertex4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void (APIENTRY* glVertex4fv)(const GLfloat* v);
    void (APIENTRY* glVertex4i)(GLint x, GLint y, GLint z, GLint w);
    void (APIENTRY* glVertex4iv)(const GLint* v);
    void (APIENTRY* glVertex4s)(GLshort x, GLshort y, GLshort z, GLshort w);
    void (APIENTRY* glVertex4sv)(const GLshort* v);
    void (APIENTRY* glVertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
    void (APIENTRY* glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);

    // OpenGL extensions

    // !!! NOTICE !!!!
    // We depend on the facts that:
    // The extension functions order is the same as in apMonitoredFunctionId.h.
    //
    // (See gsConnectOpenGLWrappers() implementation for the reason for the above
    //  limitations).


    // OpenGL extensions functions:


    //////////////////////////////////////////////////////////////////////////
    // OpenGL 1.1
    //////////////////////////////////////////////////////////////////////////
    // Supported with the base OpenGL implementation

    //////////////////////////////////////////////////////////////////////////
    // OpenGL 1.2 Extensions
    //////////////////////////////////////////////////////////////////////////

    // D.1 - GL_EXT_texture3D - 1 function
    void (APIENTRY* glTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);

    // D.2 - GL_EXT_bgra - 0 function
    // D.3 - GL_EXT_packed_pixels - 0 function
    // D.4 - GL_EXT_rescale_normal - 0 function
    // D.5 - GL_EXT_separate_specular_color - 0 function
    // D.6 - GL_SGIS_texture_edge_clamp - 0 function
    // D.7 - GL_SGIS_texture_lod - 0 function

    // D.8 - GL_EXT_draw_range_elements - 1 function
    void (APIENTRY* glDrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices);

    // D.9.1 - GL_EXT_color_table
    void (APIENTRY* glColorTable)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid* table);
    void (APIENTRY* glColorTableParameterfv)(GLenum target, GLenum pname, const GLfloat* params);
    void (APIENTRY* glColorTableParameteriv)(GLenum target, GLenum pname, const GLint* params);
    void (APIENTRY* glCopyColorTable)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
    void (APIENTRY* glGetColorTable)(GLenum target, GLenum format, GLenum type, GLvoid* table);
    void (APIENTRY* glGetColorTableParameterfv)(GLenum target, GLenum pname, GLfloat* params);
    void (APIENTRY* glGetColorTableParameteriv)(GLenum target, GLenum pname, GLint* params);

    void (APIENTRY* glTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
    void (APIENTRY* glCopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

    // D.9.1 - GL_EXT_color_subtable - 2 functions
    void (APIENTRY* glColorSubTable)(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid* data);
    void (APIENTRY* glCopyColorSubTable)(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);

    // D.9.2 - GL_EXT_convolution - 13 functions
    void (APIENTRY* glConvolutionFilter1D)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid* image);
    void (APIENTRY* glConvolutionFilter2D)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* image);
    void (APIENTRY* glConvolutionParameterf)(GLenum target, GLenum pname, GLfloat param);
    void (APIENTRY* glConvolutionParameterfv)(GLenum target, GLenum pname, const GLfloat* params);
    void (APIENTRY* glConvolutionParameteri)(GLenum target, GLenum pname, GLint param);
    void (APIENTRY* glConvolutionParameteriv)(GLenum target, GLenum pname, const GLint* params);
    void (APIENTRY* glCopyConvolutionFilter1D)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
    void (APIENTRY* glCopyConvolutionFilter2D)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
    void (APIENTRY* glGetConvolutionFilter)(GLenum target, GLenum format, GLenum type, GLvoid* image);
    void (APIENTRY* glGetConvolutionParameterfv)(GLenum target, GLenum pname, GLfloat* params);
    void (APIENTRY* glGetConvolutionParameteriv)(GLenum target, GLenum pname, GLint* params);
    void (APIENTRY* glGetSeparableFilter)(GLenum target, GLenum format, GLenum type, GLvoid* row, GLvoid* column, GLvoid* span);
    void (APIENTRY* glSeparableFilter2D)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* row, const GLvoid* column);

    // D.9.2 - GL_HP_convolution_border_modes - 0 function
    // D.9.3 - GL_SGI_color_matrix - 0 function

    // D.9.4 - GL_EXT_histogram - 10 functions
    void (APIENTRY* glGetHistogram)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid* values);
    void (APIENTRY* glGetHistogramParameterfv)(GLenum target, GLenum pname, GLfloat* params);
    void (APIENTRY* glGetHistogramParameteriv)(GLenum target, GLenum pname, GLint* params);
    void (APIENTRY* glGetMinmax)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid* values);
    void (APIENTRY* glGetMinmaxParameterfv)(GLenum target, GLenum pname, GLfloat* params);
    void (APIENTRY* glGetMinmaxParameteriv)(GLenum target, GLenum pname, GLint* params);
    void (APIENTRY* glHistogram)(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
    void (APIENTRY* glMinmax)(GLenum target, GLenum internalformat, GLboolean sink);
    void (APIENTRY* glResetHistogram)(GLenum target);
    void (APIENTRY* glResetMinmax)(GLenum target);

    // D.9.5 - GL_EXT_blend_color - 1 function
    void (APIENTRY* glBlendColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);

    // D.9.6 - GL_EXT_blend_minmax - 1 function
    void (APIENTRY* glBlendEquation)(GLenum mode);
    // D.9.6 - GL_EXT_blend_subtract - 0 functions

    //////////////////////////////////////////////////////////////////////////
    // OpenGL 1.3
    //////////////////////////////////////////////////////////////////////////

    // E - GL_ARB_multitexture / SGIS_multitexture
    void (APIENTRY* glActiveTexture)(GLenum texture);
    void (APIENTRY* glClientActiveTexture)(GLenum texture);
    void (APIENTRY* glMultiTexCoord1d)(GLenum target, GLdouble s);
    void (APIENTRY* glMultiTexCoord1dv)(GLenum target, const GLdouble* v);
    void (APIENTRY* glMultiTexCoord1f)(GLenum target, GLfloat s);
    void (APIENTRY* glMultiTexCoord1fv)(GLenum target, const GLfloat* v);
    void (APIENTRY* glMultiTexCoord1i)(GLenum target, GLint s);
    void (APIENTRY* glMultiTexCoord1iv)(GLenum target, const GLint* v);
    void (APIENTRY* glMultiTexCoord1s)(GLenum target, GLshort s);
    void (APIENTRY* glMultiTexCoord1sv)(GLenum target, const GLshort* v);
    void (APIENTRY* glMultiTexCoord2d)(GLenum target, GLdouble s, GLdouble t);
    void (APIENTRY* glMultiTexCoord2dv)(GLenum target, const GLdouble* v);
    void (APIENTRY* glMultiTexCoord2f)(GLenum target, GLfloat s, GLfloat t);
    void (APIENTRY* glMultiTexCoord2fv)(GLenum target, const GLfloat* v);
    void (APIENTRY* glMultiTexCoord2i)(GLenum target, GLint s, GLint t);
    void (APIENTRY* glMultiTexCoord2iv)(GLenum target, const GLint* v);
    void (APIENTRY* glMultiTexCoord2s)(GLenum target, GLshort s, GLshort t);
    void (APIENTRY* glMultiTexCoord2sv)(GLenum target, const GLshort* v);
    void (APIENTRY* glMultiTexCoord3d)(GLenum target, GLdouble s, GLdouble t, GLdouble r);
    void (APIENTRY* glMultiTexCoord3dv)(GLenum target, const GLdouble* v);
    void (APIENTRY* glMultiTexCoord3f)(GLenum target, GLfloat s, GLfloat t, GLfloat r);
    void (APIENTRY* glMultiTexCoord3fv)(GLenum target, const GLfloat* v);
    void (APIENTRY* glMultiTexCoord3i)(GLenum target, GLint s, GLint t, GLint r);
    void (APIENTRY* glMultiTexCoord3iv)(GLenum target, const GLint* v);
    void (APIENTRY* glMultiTexCoord3s)(GLenum target, GLshort s, GLshort t, GLshort r);
    void (APIENTRY* glMultiTexCoord3sv)(GLenum target, const GLshort* v);
    void (APIENTRY* glMultiTexCoord4d)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
    void (APIENTRY* glMultiTexCoord4dv)(GLenum target, const GLdouble* v);
    void (APIENTRY* glMultiTexCoord4f)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
    void (APIENTRY* glMultiTexCoord4fv)(GLenum target, const GLfloat* v);
    void (APIENTRY* glMultiTexCoord4i)(GLenum target, GLint s, GLint t, GLint r, GLint q);
    void (APIENTRY* glMultiTexCoord4iv)(GLenum target, const GLint* v);
    void (APIENTRY* glMultiTexCoord4s)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
    void (APIENTRY* glMultiTexCoord4sv)(GLenum target, const GLshort* v);

    // F.1 - GL_ARB_texture_compression - 7 functions
    void (APIENTRY* glCompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
    void (APIENTRY* glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data);
    void (APIENTRY* glCompressedTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid* data);
    void (APIENTRY* glCompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
    void (APIENTRY* glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data);
    void (APIENTRY* glCompressedTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid* data);
    void (APIENTRY* glGetCompressedTexImage)(GLenum target, GLint lod, GLvoid* img);

    // F.2 - GL_ARB_texture_cube_map - 0 functions

    // F.3 - GL_ARB_multisample - 1 function
    void (APIENTRY* glSampleCoverage)(GLclampf value, GLboolean invert);

    // F.4 - GL_ARB_multitexture - already defined in OpenGL 1.2.1
    // F.5 - GL_ARB_texture_env_add - 0 functions
    // F.6 - GL_ARB_texture_env_combine - 0 functions
    // F.7 - GL_ARB_texture_env_dot3 - 0 functions
    // F.8 - GL_ARB_texture_border_clamp - 0 functions

    // F.9 - GL_ARB_transpose_matrix - 2 functions
    void (APIENTRY* glLoadTransposeMatrixf)(const GLfloat* m);
    void (APIENTRY* glLoadTransposeMatrixd)(const GLdouble* m);
    void (APIENTRY* glMultTransposeMatrixf)(const GLfloat* m);
    void (APIENTRY* glMultTransposeMatrixd)(const GLdouble* m);


    //////////////////////////////////////////////////////////////////////////
    // OpenGL 1.4 Extensions
    //////////////////////////////////////////////////////////////////////////

    void (APIENTRY* glBlendFuncSeparate)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
    void (APIENTRY* glFogCoordf)(GLfloat coord);
    void (APIENTRY* glFogCoordfv)(const GLfloat* coord);
    void (APIENTRY* glFogCoordd)(GLdouble coord);
    void (APIENTRY* glFogCoorddv)(const GLdouble* coord);
    void (APIENTRY* glFogCoordPointer)(GLenum type, GLsizei stride, const GLvoid* pointer);
    void (APIENTRY* glMultiDrawArrays)(GLenum mode, const GLint* first, const GLsizei* count, GLsizei primcount);
    void (APIENTRY* glMultiDrawElements)(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices, GLsizei primcount);
    void (APIENTRY* glPointParameterf)(GLenum pname, GLfloat param);
    void (APIENTRY* glPointParameterfv)(GLenum pname, const GLfloat* params);
    void (APIENTRY* glPointParameteri)(GLenum pname, GLint param);
    void (APIENTRY* glPointParameteriv)(GLenum pname, const GLint* params);
    void (APIENTRY* glSecondaryColor3b)(GLbyte red, GLbyte green, GLbyte blue);
    void (APIENTRY* glSecondaryColor3bv)(const GLbyte* v);
    void (APIENTRY* glSecondaryColor3d)(GLdouble red, GLdouble green, GLdouble blue);
    void (APIENTRY* glSecondaryColor3dv)(const GLdouble* v);
    void (APIENTRY* glSecondaryColor3f)(GLfloat red, GLfloat green, GLfloat blue);
    void (APIENTRY* glSecondaryColor3fv)(const GLfloat* v);
    void (APIENTRY* glSecondaryColor3i)(GLint red, GLint green, GLint blue);
    void (APIENTRY* glSecondaryColor3iv)(const GLint* v);
    void (APIENTRY* glSecondaryColor3s)(GLshort red, GLshort green, GLshort blue);
    void (APIENTRY* glSecondaryColor3sv)(const GLshort* v);
    void (APIENTRY* glSecondaryColor3ub)(GLubyte red, GLubyte green, GLubyte blue);
    void (APIENTRY* glSecondaryColor3ubv)(const GLubyte* v);
    void (APIENTRY* glSecondaryColor3ui)(GLuint red, GLuint green, GLuint blue);
    void (APIENTRY* glSecondaryColor3uiv)(const GLuint* v);
    void (APIENTRY* glSecondaryColor3us)(GLushort red, GLushort green, GLushort blue);
    void (APIENTRY* glSecondaryColor3usv)(const GLushort* v);
    void (APIENTRY* glSecondaryColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
    void (APIENTRY* glWindowPos2d)(GLdouble x, GLdouble y);
    void (APIENTRY* glWindowPos2dv)(const GLdouble* v);
    void (APIENTRY* glWindowPos2f)(GLfloat x, GLfloat y);
    void (APIENTRY* glWindowPos2fv)(const GLfloat* v);
    void (APIENTRY* glWindowPos2i)(GLint x, GLint y);
    void (APIENTRY* glWindowPos2iv)(const GLint* v);
    void (APIENTRY* glWindowPos2s)(GLshort x, GLshort y);
    void (APIENTRY* glWindowPos2sv)(const GLshort* v);
    void (APIENTRY* glWindowPos3d)(GLdouble x, GLdouble y, GLdouble z);
    void (APIENTRY* glWindowPos3dv)(const GLdouble* v);
    void (APIENTRY* glWindowPos3f)(GLfloat x, GLfloat y, GLfloat z);
    void (APIENTRY* glWindowPos3fv)(const GLfloat* v);
    void (APIENTRY* glWindowPos3i)(GLint x, GLint y, GLint z);
    void (APIENTRY* glWindowPos3iv)(const GLint* v);
    void (APIENTRY* glWindowPos3s)(GLshort x, GLshort y, GLshort z);
    void (APIENTRY* glWindowPos3sv)(const GLshort* v);

    //////////////////////////////////////////////////////////////////////////
    // OpenGL 1.5 Extensions
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glGenQueries)(GLsizei n, GLuint* ids);
    void (APIENTRY* glDeleteQueries)(GLsizei n, const GLuint* ids);
    GLboolean(APIENTRY* glIsQuery)(GLuint id);
    void (APIENTRY* glBeginQuery)(GLenum target, GLuint id);
    void (APIENTRY* glEndQuery)(GLenum target);
    void (APIENTRY* glGetQueryiv)(GLenum target, GLenum pname, GLint* params);
    void (APIENTRY* glGetQueryObjectiv)(GLuint id, GLenum pname, GLint* params);
    void (APIENTRY* glGetQueryObjectuiv)(GLuint id, GLenum pname, GLuint* params);
    void (APIENTRY* glBindBuffer)(GLenum target, GLuint buffer);
    void (APIENTRY* glDeleteBuffers)(GLsizei n, const GLuint* buffers);
    void (APIENTRY* glGenBuffers)(GLsizei n, GLuint* buffers);
    GLboolean(APIENTRY* glIsBuffer)(GLuint buffer);
    void (APIENTRY* glBufferData)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
    void (APIENTRY* glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
    void (APIENTRY* glGetBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid* data);
    GLvoid* (APIENTRY* glMapBuffer)(GLenum target, GLenum access);
    GLboolean(APIENTRY* glUnmapBuffer)(GLenum target);
    void (APIENTRY* glGetBufferParameteriv)(GLenum target, GLenum pname, GLint* params);
    void (APIENTRY* glGetBufferPointerv)(GLenum target, GLenum pname, GLvoid** params);

    //////////////////////////////////////////////////////////////////////////
    // OpenGL 2.0 Extensions
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glBlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha);
    void (APIENTRY* glDrawBuffers)(GLsizei n, const GLenum* bufs);
    void (APIENTRY* glStencilOpSeparate)(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
    void (APIENTRY* glStencilFuncSeparate)(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
    void (APIENTRY* glStencilMaskSeparate)(GLenum face, GLuint mask);
    void (APIENTRY* glAttachShader)(GLuint program, GLuint shader);
    void (APIENTRY* glBindAttribLocation)(GLuint program, GLuint index, const GLchar* name);
    void (APIENTRY* glCompileShader)(GLuint shader);
    GLuint(APIENTRY* glCreateProgram)(void);
    GLuint(APIENTRY* glCreateShader)(GLenum type);
    void (APIENTRY* glDeleteProgram)(GLuint program);
    void (APIENTRY* glDeleteShader)(GLuint shader);
    void (APIENTRY* glDetachShader)(GLuint program, GLuint shader);
    void (APIENTRY* glDisableVertexAttribArray)(GLuint index);
    void (APIENTRY* glEnableVertexAttribArray)(GLuint index);
    void (APIENTRY* glGetActiveAttrib)(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
    void (APIENTRY* glGetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
    void (APIENTRY* glGetAttachedShaders)(GLuint program, GLsizei maxCount, GLsizei* count, GLuint* obj);
    GLint(APIENTRY* glGetAttribLocation)(GLuint program, const GLchar* name);
    void (APIENTRY* glGetProgramiv)(GLuint program, GLenum pname, GLint* params);
    void (APIENTRY* glGetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
    void (APIENTRY* glGetShaderiv)(GLuint shader, GLenum pname, GLint* params);
    void (APIENTRY* glGetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
    void (APIENTRY* glGetShaderSource)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source);
    GLint(APIENTRY* glGetUniformLocation)(GLuint program, const GLchar* name);
    void (APIENTRY* glGetUniformfv)(GLuint program, GLint location, GLfloat* params);
    void (APIENTRY* glGetUniformiv)(GLuint program, GLint location, GLint* params);
    void (APIENTRY* glGetVertexAttribdv)(GLuint index, GLenum pname, GLdouble* params);
    void (APIENTRY* glGetVertexAttribfv)(GLuint index, GLenum pname, GLfloat* params);
    void (APIENTRY* glGetVertexAttribiv)(GLuint index, GLenum pname, GLint* params);
    void (APIENTRY* glGetVertexAttribPointerv)(GLuint index, GLenum pname, GLvoid** pointer);
    GLboolean(APIENTRY* glIsProgram)(GLuint program);
    GLboolean(APIENTRY* glIsShader)(GLuint shader);
    void (APIENTRY* glLinkProgram)(GLuint program);
    void (APIENTRY* glShaderSource)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
    void (APIENTRY* glUseProgram)(GLuint program);
    void (APIENTRY* glUniform1f)(GLint location, GLfloat v0);
    void (APIENTRY* glUniform2f)(GLint location, GLfloat v0, GLfloat v1);
    void (APIENTRY* glUniform3f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    void (APIENTRY* glUniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    void (APIENTRY* glUniform1i)(GLint location, GLint v0);
    void (APIENTRY* glUniform2i)(GLint location, GLint v0, GLint v1);
    void (APIENTRY* glUniform3i)(GLint location, GLint v0, GLint v1, GLint v2);
    void (APIENTRY* glUniform4i)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
    void (APIENTRY* glUniform1fv)(GLint location, GLsizei count, const GLfloat* value);
    void (APIENTRY* glUniform2fv)(GLint location, GLsizei count, const GLfloat* value);
    void (APIENTRY* glUniform3fv)(GLint location, GLsizei count, const GLfloat* value);
    void (APIENTRY* glUniform4fv)(GLint location, GLsizei count, const GLfloat* value);
    void (APIENTRY* glUniform1iv)(GLint location, GLsizei count, const GLint* value);
    void (APIENTRY* glUniform2iv)(GLint location, GLsizei count, const GLint* value);
    void (APIENTRY* glUniform3iv)(GLint location, GLsizei count, const GLint* value);
    void (APIENTRY* glUniform4iv)(GLint location, GLsizei count, const GLint* value);
    void (APIENTRY* glUniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glValidateProgram)(GLuint program);
    void (APIENTRY* glVertexAttrib1d)(GLuint index, GLdouble x);
    void (APIENTRY* glVertexAttrib1dv)(GLuint index, const GLdouble* v);
    void (APIENTRY* glVertexAttrib1f)(GLuint index, GLfloat x);
    void (APIENTRY* glVertexAttrib1fv)(GLuint index, const GLfloat* v);
    void (APIENTRY* glVertexAttrib1s)(GLuint index, GLshort x);
    void (APIENTRY* glVertexAttrib1sv)(GLuint index, const GLshort* v);
    void (APIENTRY* glVertexAttrib2d)(GLuint index, GLdouble x, GLdouble y);
    void (APIENTRY* glVertexAttrib2dv)(GLuint index, const GLdouble* v);
    void (APIENTRY* glVertexAttrib2f)(GLuint index, GLfloat x, GLfloat y);
    void (APIENTRY* glVertexAttrib2fv)(GLuint index, const GLfloat* v);
    void (APIENTRY* glVertexAttrib2s)(GLuint index, GLshort x, GLshort y);
    void (APIENTRY* glVertexAttrib2sv)(GLuint index, const GLshort* v);
    void (APIENTRY* glVertexAttrib3d)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
    void (APIENTRY* glVertexAttrib3dv)(GLuint index, const GLdouble* v);
    void (APIENTRY* glVertexAttrib3f)(GLuint index, GLfloat x, GLfloat y, GLfloat z);
    void (APIENTRY* glVertexAttrib3fv)(GLuint index, const GLfloat* v);
    void (APIENTRY* glVertexAttrib3s)(GLuint index, GLshort x, GLshort y, GLshort z);
    void (APIENTRY* glVertexAttrib3sv)(GLuint index, const GLshort* v);
    void (APIENTRY* glVertexAttrib4Nbv)(GLuint index, const GLbyte* v);
    void (APIENTRY* glVertexAttrib4Niv)(GLuint index, const GLint* v);
    void (APIENTRY* glVertexAttrib4Nsv)(GLuint index, const GLshort* v);
    void (APIENTRY* glVertexAttrib4Nub)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
    void (APIENTRY* glVertexAttrib4Nubv)(GLuint index, const GLubyte* v);
    void (APIENTRY* glVertexAttrib4Nuiv)(GLuint index, const GLuint* v);
    void (APIENTRY* glVertexAttrib4Nusv)(GLuint index, const GLushort* v);
    void (APIENTRY* glVertexAttrib4bv)(GLuint index, const GLbyte* v);
    void (APIENTRY* glVertexAttrib4d)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void (APIENTRY* glVertexAttrib4dv)(GLuint index, const GLdouble* v);
    void (APIENTRY* glVertexAttrib4f)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void (APIENTRY* glVertexAttrib4fv)(GLuint index, const GLfloat* v);
    void (APIENTRY* glVertexAttrib4iv)(GLuint index, const GLint* v);
    void (APIENTRY* glVertexAttrib4s)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
    void (APIENTRY* glVertexAttrib4sv)(GLuint index, const GLshort* v);
    void (APIENTRY* glVertexAttrib4ubv)(GLuint index, const GLubyte* v);
    void (APIENTRY* glVertexAttrib4uiv)(GLuint index, const GLuint* v);
    void (APIENTRY* glVertexAttrib4usv)(GLuint index, const GLushort* v);
    void (APIENTRY* glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);

    //////////////////////////////////////////////////////////////////////////
    // OpenGL 2.1 Extensions
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glUniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glUniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glUniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glUniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glUniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glUniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

    //////////////////////////////////////////////////////////////////////////
    // OpenGL 3.0 Extensions
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glColorMaski)(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
    void (APIENTRY* glGetBooleani_v)(GLenum target, GLuint pname, GLboolean* params);
    void (APIENTRY* glGetIntegeri_v)(GLenum target, GLuint pname, GLint* params);
    void (APIENTRY* glEnablei)(GLenum cap, GLuint index);
    void (APIENTRY* glDisablei)(GLenum cap, GLuint index);
    GLboolean(APIENTRY* glIsEnabledi)(GLenum cap, GLuint index);
    void (APIENTRY* glBeginTransformFeedback)(GLenum mode);
    void (APIENTRY* glEndTransformFeedback)();
    void (APIENTRY* glBindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
    void (APIENTRY* glBindBufferBase)(GLenum target, GLuint index, GLuint buffer);
    void (APIENTRY* glTransformFeedbackVaryings)(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode);
    void (APIENTRY* glGetTransformFeedbackVarying)(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name);
    void (APIENTRY* glClampColor)(GLenum target, GLenum clamp);
    void (APIENTRY* glBeginConditionalRender)(GLuint id, GLenum mode);
    void (APIENTRY* glEndConditionalRender)();
    void (APIENTRY* glVertexAttribI1i)(GLuint index, GLint values);
    void (APIENTRY* glVertexAttribI2i)(GLuint index, GLint val1, GLint val2);
    void (APIENTRY* glVertexAttribI3i)(GLuint index, GLint val1, GLint val2, GLint val3);
    void (APIENTRY* glVertexAttribI4i)(GLuint index, GLint val1, GLint val2, GLint val3, GLint val4);
    void (APIENTRY* glVertexAttribI1ui)(GLuint index, GLuint value);
    void (APIENTRY* glVertexAttribI2ui)(GLuint index, GLuint val1, GLuint val2);
    void (APIENTRY* glVertexAttribI3ui)(GLuint index, GLuint val1, GLuint val2, GLuint val3);
    void (APIENTRY* glVertexAttribI4ui)(GLuint index, GLuint val1, GLuint val2, GLuint val3, GLuint val4);
    void (APIENTRY* glVertexAttribI1iv)(GLuint index, const GLint* values);
    void (APIENTRY* glVertexAttribI2iv)(GLuint index, const GLint* values);
    void (APIENTRY* glVertexAttribI3iv)(GLuint index, const GLint* values);
    void (APIENTRY* glVertexAttribI4iv)(GLuint index, const GLint* values);
    void (APIENTRY* glVertexAttribI1uiv)(GLuint index, const GLuint* values);
    void (APIENTRY* glVertexAttribI2uiv)(GLuint index, const GLuint* values);
    void (APIENTRY* glVertexAttribI3uiv)(GLuint index, const GLuint* values);
    void (APIENTRY* glVertexAttribI4uiv)(GLuint index, const GLuint* values);
    void (APIENTRY* glVertexAttribI4bv)(GLuint index, const GLbyte* values);
    void (APIENTRY* glVertexAttribI4sv)(GLuint index, const GLshort* values);
    void (APIENTRY* glVertexAttribI4ubv)(GLuint index, const GLubyte* values);
    void (APIENTRY* glVertexAttribI4usv)(GLuint index, const GLushort* values);
    void (APIENTRY* glVertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
    void (APIENTRY* glGetVertexAttribIiv)(GLuint index, GLenum pname, GLint* params);
    void (APIENTRY* glGetVertexAttribIuiv)(GLuint index, GLenum pname, GLuint* params);
    void (APIENTRY* glGetUniformuiv)(GLuint program, GLint location, GLuint* params);
    void (APIENTRY* glBindFragDataLocation)(GLuint program, GLuint colorNumber, const GLchar* name);
    GLint(APIENTRY* glGetFragDataLocation)(GLuint program, const GLchar* name);
    void (APIENTRY* glUniform1ui)(GLint location, GLuint v0);
    void (APIENTRY* glUniform2ui)(GLint location, GLuint v0, GLuint v1);
    void (APIENTRY* glUniform3ui)(GLint location, GLuint v0, GLuint v1, GLuint v2);
    void (APIENTRY* glUniform4ui)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
    void (APIENTRY* glUniform1uiv)(GLint location, GLsizei size, const GLuint* value);
    void (APIENTRY* glUniform2uiv)(GLint location, GLsizei size, const GLuint* value);
    void (APIENTRY* glUniform3uiv)(GLint location, GLsizei size, const GLuint* value);
    void (APIENTRY* glUniform4uiv)(GLint location, GLsizei size, const GLuint* value);
    void (APIENTRY* glTexParameterIiv)(GLenum target, GLenum pname, const GLint* params);
    void (APIENTRY* glTexParameterIuiv)(GLenum target, GLenum pname, const GLuint* params);
    void (APIENTRY* glGetTexParameterIiv)(GLenum target, GLenum pname, GLint* params);
    void (APIENTRY* glGetTexParameterIuiv)(GLenum target, GLenum pname, GLuint* params);
    void (APIENTRY* glClearBufferiv)(GLenum buffer, GLint drawbuffer, const GLint* value);
    void (APIENTRY* glClearBufferuiv)(GLenum buffer, GLint drawbuffer, const GLuint* value);
    void (APIENTRY* glClearBufferfv)(GLenum buffer, GLint drawbuffer, const GLfloat* value);
    void (APIENTRY* glClearBufferfi)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint value);
    const GLubyte* (APIENTRY* glGetStringi)(GLenum name, GLuint index);

    //////////////////////////////////////////////////////////////////////////
    // OpenGL 3.1 Extensions
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glDrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei primcount);
    void (APIENTRY* glDrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei primcount);
    void (APIENTRY* glTexBuffer)(GLenum target, GLenum internalformat, GLuint buffer);
    void (APIENTRY* glPrimitiveRestartIndex)(GLuint index);

    //////////////////////////////////////////////////////////////////////////
    // OpenGL 3.2 Extensions
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glGetInteger64i_v)(GLenum target, GLuint pname, GLint64* params);
    void (APIENTRY* glGetBufferParameteri64v)(GLenum target, GLenum pname, GLint64* params);
    void (APIENTRY* glFramebufferTexture)(GLenum target, GLenum attachment, GLuint texture, GLint level);
    void (APIENTRY* glFramebufferTextureFace)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face);

    //////////////////////////////////////////////////////////////////////////
    // OpenGL 3.3 Extensions
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glBindFragDataLocationIndexed)(GLuint program, GLuint colorNumber, GLuint index, const GLchar* name);
    GLint(APIENTRY* glGetFragDataIndex)(GLuint program, const GLchar* name);
    void (APIENTRY* glGenSamplers)(GLsizei count, GLuint* samplers);
    void (APIENTRY* glDeleteSamplers)(GLsizei count, const GLuint* samplers);
    GLboolean(APIENTRY* glIsSampler)(GLuint sampler);
    void (APIENTRY* glBindSampler)(GLuint unit, GLuint sampler);
    void (APIENTRY* glSamplerParameteri)(GLuint sampler, GLenum pname, GLint param);
    void (APIENTRY* glSamplerParameteriv)(GLuint sampler, GLenum pname, const GLint* param);
    void (APIENTRY* glSamplerParameterf)(GLuint sampler, GLenum pname, GLfloat param);
    void (APIENTRY* glSamplerParameterfv)(GLuint sampler, GLenum pname, const GLfloat* param);
    void (APIENTRY* glSamplerParameterIiv)(GLuint sampler, GLenum pname, const GLint* param);
    void (APIENTRY* glSamplerParameterIuiv)(GLuint sampler, GLenum pname, const GLuint* param);
    void (APIENTRY* glGetSamplerParameteriv)(GLuint sampler, GLenum pname, GLint* params);
    void (APIENTRY* glGetSamplerParameterIiv)(GLuint sampler, GLenum pname, GLint* params);
    void (APIENTRY* glGetSamplerParameterfv)(GLuint sampler, GLenum pname, GLfloat* params);
    void (APIENTRY* glGetSamplerParameterIuiv)(GLuint sampler, GLenum pname, GLuint* params);
    void (APIENTRY* glQueryCounter)(GLuint id, GLenum target);
    void (APIENTRY* glGetQueryObjecti64v)(GLuint id, GLenum pname, GLint64* params);
    void (APIENTRY* glGetQueryObjectui64v)(GLuint id, GLenum pname, GLuint64* params);
    void (APIENTRY* glVertexAttribDivisor)(GLuint index, GLuint divisor);
    void (APIENTRY* glVertexAttribP1ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
    void (APIENTRY* glVertexAttribP1uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint* value);
    void (APIENTRY* glVertexAttribP2ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
    void (APIENTRY* glVertexAttribP2uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint* value);
    void (APIENTRY* glVertexAttribP3ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
    void (APIENTRY* glVertexAttribP3uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint* value);
    void (APIENTRY* glVertexAttribP4ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
    void (APIENTRY* glVertexAttribP4uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint* value);
    void (APIENTRY* glVertexP2ui)(GLenum type, GLuint value);
    void (APIENTRY* glVertexP2uiv)(GLenum type, const GLuint* value);
    void (APIENTRY* glVertexP3ui)(GLenum type, GLuint value);
    void (APIENTRY* glVertexP3uiv)(GLenum type, const GLuint* value);
    void (APIENTRY* glVertexP4ui)(GLenum type, GLuint value);
    void (APIENTRY* glVertexP4uiv)(GLenum type, const GLuint* value);
    void (APIENTRY* glTexCoordP1ui)(GLenum type, GLuint coords);
    void (APIENTRY* glTexCoordP1uiv)(GLenum type, const GLuint* coords);
    void (APIENTRY* glTexCoordP2ui)(GLenum type, GLuint coords);
    void (APIENTRY* glTexCoordP2uiv)(GLenum type, const GLuint* coords);
    void (APIENTRY* glTexCoordP3ui)(GLenum type, GLuint coords);
    void (APIENTRY* glTexCoordP3uiv)(GLenum type, const GLuint* coords);
    void (APIENTRY* glTexCoordP4ui)(GLenum type, GLuint coords);
    void (APIENTRY* glTexCoordP4uiv)(GLenum type, const GLuint* coords);
    void (APIENTRY* glMultiTexCoordP1ui)(GLenum texture, GLenum type, GLuint coords);
    void (APIENTRY* glMultiTexCoordP1uiv)(GLenum texture, GLenum type, const GLuint* coords);
    void (APIENTRY* glMultiTexCoordP2ui)(GLenum texture, GLenum type, GLuint coords);
    void (APIENTRY* glMultiTexCoordP2uiv)(GLenum texture, GLenum type, const GLuint* coords);
    void (APIENTRY* glMultiTexCoordP3ui)(GLenum texture, GLenum type, GLuint coords);
    void (APIENTRY* glMultiTexCoordP3uiv)(GLenum texture, GLenum type, const GLuint* coords);
    void (APIENTRY* glMultiTexCoordP4ui)(GLenum texture, GLenum type, GLuint coords);
    void (APIENTRY* glMultiTexCoordP4uiv)(GLenum texture, GLenum type, const GLuint* coords);
    void (APIENTRY* glNormalP3ui)(GLenum type, GLuint coords);
    void (APIENTRY* glNormalP3uiv)(GLenum type, const GLuint* coords);
    void (APIENTRY* glColorP3ui)(GLenum type, GLuint color);
    void (APIENTRY* glColorP3uiv)(GLenum type, const GLuint* color);
    void (APIENTRY* glColorP4ui)(GLenum type, GLuint color);
    void (APIENTRY* glColorP4uiv)(GLenum type, const GLuint* color);
    void (APIENTRY* glSecondaryColorP3ui)(GLenum type, GLuint color);
    void (APIENTRY* glSecondaryColorP3uiv)(GLenum type, const GLuint* color);

    //////////////////////////////////////////////////////////////////////////
    // OpenGL 4.0 Extensions
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glMinSampleShading)(GLfloat value);
    void (APIENTRY* glBlendEquationi)(GLuint buf, GLenum mode);
    void (APIENTRY* glBlendEquationSeparatei)(GLuint buf, GLenum modeRGB, GLenum modeAlpha);
    void (APIENTRY* glBlendFunci)(GLuint buf, GLenum src, GLenum dst);
    void (APIENTRY* glBlendFuncSeparatei)(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
    void (APIENTRY* glDrawArraysIndirect)(GLenum mode, const GLvoid* indirect);
    void (APIENTRY* glDrawElementsIndirect)(GLenum mode, GLenum type, const GLvoid* indirect);
    void (APIENTRY* glUniform1d)(GLint location, GLdouble x);
    void (APIENTRY* glUniform2d)(GLint location, GLdouble x, GLdouble y);
    void (APIENTRY* glUniform3d)(GLint location, GLdouble x, GLdouble y, GLdouble z);
    void (APIENTRY* glUniform4d)(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void (APIENTRY* glUniform1dv)(GLint location, GLsizei count, const GLdouble* value);
    void (APIENTRY* glUniform2dv)(GLint location, GLsizei count, const GLdouble* value);
    void (APIENTRY* glUniform3dv)(GLint location, GLsizei count, const GLdouble* value);
    void (APIENTRY* glUniform4dv)(GLint location, GLsizei count, const GLdouble* value);
    void (APIENTRY* glUniformMatrix2dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    void (APIENTRY* glUniformMatrix3dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    void (APIENTRY* glUniformMatrix4dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    void (APIENTRY* glUniformMatrix2x3dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    void (APIENTRY* glUniformMatrix2x4dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    void (APIENTRY* glUniformMatrix3x2dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    void (APIENTRY* glUniformMatrix3x4dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    void (APIENTRY* glUniformMatrix4x2dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    void (APIENTRY* glUniformMatrix4x3dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    void (APIENTRY* glGetUniformdv)(GLuint program, GLint location, GLdouble* params);
    GLint(APIENTRY* glGetSubroutineUniformLocation)(GLuint program, GLenum shadertype, const GLchar* name);
    GLuint(APIENTRY* glGetSubroutineIndex)(GLuint program, GLenum shadertype, const GLchar* name);
    void (APIENTRY* glGetActiveSubroutineUniformiv)(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint* values);
    void (APIENTRY* glGetActiveSubroutineUniformName)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei* length, GLchar* name);
    void (APIENTRY* glGetActiveSubroutineName)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei* length, GLchar* name);
    void (APIENTRY* glUniformSubroutinesuiv)(GLenum shadertype, GLsizei count, const GLuint* indices);
    void (APIENTRY* glGetUniformSubroutineuiv)(GLenum shadertype, GLint location, GLuint* params);
    void (APIENTRY* glGetProgramStageiv)(GLuint program, GLenum shadertype, GLenum pname, GLint* values);
    void (APIENTRY* glPatchParameteri)(GLenum pname, GLint value);
    void (APIENTRY* glPatchParameterfv)(GLenum pname, const GLfloat* values);
    void (APIENTRY* glBindTransformFeedback)(GLenum target, GLuint id);
    void (APIENTRY* glDeleteTransformFeedbacks)(GLsizei n, const GLuint* ids);
    void (APIENTRY* glGenTransformFeedbacks)(GLsizei n, GLuint* ids);
    GLboolean(APIENTRY* glIsTransformFeedback)(GLuint id);
    void (APIENTRY* glPauseTransformFeedback)(void);
    void (APIENTRY* glResumeTransformFeedback)(void);
    void (APIENTRY* glDrawTransformFeedback)(GLenum mode, GLuint id);
    void (APIENTRY* glDrawTransformFeedbackStream)(GLenum mode, GLuint id, GLuint stream);
    void (APIENTRY* glBeginQueryIndexed)(GLenum target, GLuint index, GLuint id);
    void (APIENTRY* glEndQueryIndexed)(GLenum target, GLuint index);
    void (APIENTRY* glGetQueryIndexediv)(GLenum target, GLuint index, GLenum pname, GLint* params);


    //////////////////////////////////////////////////////////////////////////
    // OpenGL 4.1 Extensions
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glReleaseShaderCompiler)(void);
    void (APIENTRY* glShaderBinary)(GLsizei count, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length);
    void (APIENTRY* glGetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);
    void (APIENTRY* glDepthRangef)(GLfloat n, GLfloat f);
    void (APIENTRY* glClearDepthf)(GLfloat d);
    void (APIENTRY* glGetProgramBinary)(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary);
    void (APIENTRY* glProgramBinary)(GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length);
    void (APIENTRY* glProgramParameteri)(GLuint program, GLenum pname, GLint value);
    void (APIENTRY* glUseProgramStages)(GLuint pipeline, GLbitfield stages, GLuint program);
    void (APIENTRY* glActiveShaderProgram)(GLuint pipeline, GLuint program);
    GLuint(APIENTRY* glCreateShaderProgramv)(GLenum type, GLsizei count, const GLchar* const* strings);
    void (APIENTRY* glBindProgramPipeline)(GLuint pipeline);
    void (APIENTRY* glDeleteProgramPipelines)(GLsizei n, const GLuint* pipelines);
    void (APIENTRY* glGenProgramPipelines)(GLsizei n, GLuint* pipelines);
    GLboolean(APIENTRY* glIsProgramPipeline)(GLuint pipeline);
    void (APIENTRY* glGetProgramPipelineiv)(GLuint pipeline, GLenum pname, GLint* params);
    void (APIENTRY* glProgramUniform1i)(GLuint program, GLint location, GLint v0);
    void (APIENTRY* glProgramUniform1iv)(GLuint program, GLint location, GLsizei count, const GLint* value);
    void (APIENTRY* glProgramUniform1f)(GLuint program, GLint location, GLfloat v0);
    void (APIENTRY* glProgramUniform1fv)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
    void (APIENTRY* glProgramUniform1d)(GLuint program, GLint location, GLdouble v0);
    void (APIENTRY* glProgramUniform1dv)(GLuint program, GLint location, GLsizei count, const GLdouble* value);
    void (APIENTRY* glProgramUniform1ui)(GLuint program, GLint location, GLuint v0);
    void (APIENTRY* glProgramUniform1uiv)(GLuint program, GLint location, GLsizei count, const GLuint* value);
    void (APIENTRY* glProgramUniform2i)(GLuint program, GLint location, GLint v0, GLint v1);
    void (APIENTRY* glProgramUniform2iv)(GLuint program, GLint location, GLsizei count, const GLint* value);
    void (APIENTRY* glProgramUniform2f)(GLuint program, GLint location, GLfloat v0, GLfloat v1);
    void (APIENTRY* glProgramUniform2fv)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
    void (APIENTRY* glProgramUniform2d)(GLuint program, GLint location, GLdouble v0, GLdouble v1);
    void (APIENTRY* glProgramUniform2dv)(GLuint program, GLint location, GLsizei count, const GLdouble* value);
    void (APIENTRY* glProgramUniform2ui)(GLuint program, GLint location, GLuint v0, GLuint v1);
    void (APIENTRY* glProgramUniform2uiv)(GLuint program, GLint location, GLsizei count, const GLuint* value);
    void (APIENTRY* glProgramUniform3i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
    void (APIENTRY* glProgramUniform3iv)(GLuint program, GLint location, GLsizei count, const GLint* value);
    void (APIENTRY* glProgramUniform3f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    void (APIENTRY* glProgramUniform3fv)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
    void (APIENTRY* glProgramUniform3d)(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2);
    void (APIENTRY* glProgramUniform3dv)(GLuint program, GLint location, GLsizei count, const GLdouble* value);
    void (APIENTRY* glProgramUniform3ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
    void (APIENTRY* glProgramUniform3uiv)(GLuint program, GLint location, GLsizei count, const GLuint* value);
    void (APIENTRY* glProgramUniform4i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
    void (APIENTRY* glProgramUniform4iv)(GLuint program, GLint location, GLsizei count, const GLint* value);
    void (APIENTRY* glProgramUniform4f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    void (APIENTRY* glProgramUniform4fv)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
    void (APIENTRY* glProgramUniform4d)(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
    void (APIENTRY* glProgramUniform4dv)(GLuint program, GLint location, GLsizei count, const GLdouble* value);
    void (APIENTRY* glProgramUniform4ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
    void (APIENTRY* glProgramUniform4uiv)(GLuint program, GLint location, GLsizei count, const GLuint* value);
    void (APIENTRY* glProgramUniformMatrix2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glProgramUniformMatrix3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glProgramUniformMatrix4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glProgramUniformMatrix2dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    void (APIENTRY* glProgramUniformMatrix3dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    void (APIENTRY* glProgramUniformMatrix4dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    void (APIENTRY* glProgramUniformMatrix2x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glProgramUniformMatrix3x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glProgramUniformMatrix2x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glProgramUniformMatrix4x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glProgramUniformMatrix3x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glProgramUniformMatrix4x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glProgramUniformMatrix2x3dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    void (APIENTRY* glProgramUniformMatrix3x2dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    void (APIENTRY* glProgramUniformMatrix2x4dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    void (APIENTRY* glProgramUniformMatrix4x2dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    void (APIENTRY* glProgramUniformMatrix3x4dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    void (APIENTRY* glProgramUniformMatrix4x3dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    void (APIENTRY* glValidateProgramPipeline)(GLuint pipeline);
    void (APIENTRY* glGetProgramPipelineInfoLog)(GLuint pipeline, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
    void (APIENTRY* glVertexAttribL1d)(GLuint index, GLdouble x);
    void (APIENTRY* glVertexAttribL2d)(GLuint index, GLdouble x, GLdouble y);
    void (APIENTRY* glVertexAttribL3d)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
    void (APIENTRY* glVertexAttribL4d)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void (APIENTRY* glVertexAttribL1dv)(GLuint index, const GLdouble* v);
    void (APIENTRY* glVertexAttribL2dv)(GLuint index, const GLdouble* v);
    void (APIENTRY* glVertexAttribL3dv)(GLuint index, const GLdouble* v);
    void (APIENTRY* glVertexAttribL4dv)(GLuint index, const GLdouble* v);
    void (APIENTRY* glVertexAttribLPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
    void (APIENTRY* glGetVertexAttribLdv)(GLuint index, GLenum pname, GLdouble* params);
    void (APIENTRY* glViewportArrayv)(GLuint first, GLsizei count, const GLfloat* v);
    void (APIENTRY* glViewportIndexedf)(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h);
    void (APIENTRY* glViewportIndexedfv)(GLuint index, const GLfloat* v);
    void (APIENTRY* glScissorArrayv)(GLuint first, GLsizei count, const GLint* v);
    void (APIENTRY* glScissorIndexed)(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height);
    void (APIENTRY* glScissorIndexedv)(GLuint index, const GLint* v);
    void (APIENTRY* glDepthRangeArrayv)(GLuint first, GLsizei count, const GLdouble* v);
    void (APIENTRY* glDepthRangeIndexed)(GLuint index, GLdouble n, GLdouble f);
    void (APIENTRY* glGetFloati_v)(GLenum target, GLuint index, GLfloat* data);
    void (APIENTRY* glGetDoublei_v)(GLenum target, GLuint index, GLdouble* data);

    //////////////////////////////////////////////////////////////////////////
    // OpenGL 4.2 Extensions
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glDrawArraysInstancedBaseInstance)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance);
    void (APIENTRY* glDrawElementsInstancedBaseInstance)(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLuint baseinstance);
    void (APIENTRY* glDrawElementsInstancedBaseVertexBaseInstance)(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance);
    void (APIENTRY* glGetInternalformativ)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params);
    void (APIENTRY* glGetActiveAtomicCounterBufferiv)(GLuint program, GLuint bufferIndex, GLenum pname, GLint* params);
    void (APIENTRY* glBindImageTexture)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
    void (APIENTRY* glMemoryBarrier)(GLbitfield barriers);
    void (APIENTRY* glTexStorage1D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
    void (APIENTRY* glTexStorage2D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
    void (APIENTRY* glTexStorage3D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
    void (APIENTRY* glDrawTransformFeedbackInstanced)(GLenum mode, GLuint id, GLsizei instancecount);
    void (APIENTRY* glDrawTransformFeedbackStreamInstanced)(GLenum mode, GLuint id, GLuint stream, GLsizei instancecount);

    //////////////////////////////////////////////////////////////////////////
    // OpenGL 4.3 Extensions
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glClearBufferData)(GLenum target, GLenum internalformat, GLenum format, GLenum type, const void* data);
    void (APIENTRY* glClearBufferSubData)(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data);
    void (APIENTRY* glDispatchCompute)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
    void (APIENTRY* glDispatchComputeIndirect)(GLintptr indirect);
    void (APIENTRY* glCopyImageSubData)(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
    void (APIENTRY* glFramebufferParameteri)(GLenum target, GLenum pname, GLint param);
    void (APIENTRY* glGetFramebufferParameteriv)(GLenum target, GLenum pname, GLint* params);
    void (APIENTRY* glGetInternalformati64v)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint64* params);
    void (APIENTRY* glInvalidateTexSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth);
    void (APIENTRY* glInvalidateTexImage)(GLuint texture, GLint level);
    void (APIENTRY* glInvalidateBufferSubData)(GLuint buffer, GLintptr offset, GLsizeiptr length);
    void (APIENTRY* glInvalidateBufferData)(GLuint buffer);
    void (APIENTRY* glInvalidateFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum* attachments);
    void (APIENTRY* glInvalidateSubFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height);
    void (APIENTRY* glMultiDrawArraysIndirect)(GLenum mode, const void* indirect, GLsizei drawcount, GLsizei stride);
    void (APIENTRY* glMultiDrawElementsIndirect)(GLenum mode, GLenum type, const void* indirect, GLsizei drawcount, GLsizei stride);
    void (APIENTRY* glGetProgramInterfaceiv)(GLuint program, GLenum programInterface, GLenum pname, GLint* params);
    GLuint(APIENTRY* glGetProgramResourceIndex)(GLuint program, GLenum programInterface, const GLchar* name);
    void (APIENTRY* glGetProgramResourceName)(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name);
    void (APIENTRY* glGetProgramResourceiv)(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum* props, GLsizei bufSize, GLsizei* length, GLint* params);
    GLint(APIENTRY* glGetProgramResourceLocation)(GLuint program, GLenum programInterface, const GLchar* name);
    GLint(APIENTRY* glGetProgramResourceLocationIndex)(GLuint program, GLenum programInterface, const GLchar* name);
    void (APIENTRY* glShaderStorageBlockBinding)(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding);
    void (APIENTRY* glTexBufferRange)(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
    void (APIENTRY* glTexStorage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
    void (APIENTRY* glTexStorage3DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
    void (APIENTRY* glTextureView)(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);
    void (APIENTRY* glBindVertexBuffer)(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
    void (APIENTRY* glVertexAttribFormat)(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
    void (APIENTRY* glVertexAttribIFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
    void (APIENTRY* glVertexAttribLFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
    void (APIENTRY* glVertexAttribBinding)(GLuint attribindex, GLuint bindingindex);
    void (APIENTRY* glVertexBindingDivisor)(GLuint bindingindex, GLuint divisor);
    void (APIENTRY* glDebugMessageControl)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
    void (APIENTRY* glDebugMessageInsert)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf);
    void (APIENTRY* glDebugMessageCallback)(GLDEBUGPROC callback, const void* userParam);
    GLuint(APIENTRY* glGetDebugMessageLog)(GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog);
    void (APIENTRY* glPushDebugGroup)(GLenum source, GLuint id, GLsizei length, const GLchar* message);
    void (APIENTRY* glPopDebugGroup)(void);
    void (APIENTRY* glObjectLabel)(GLenum identifier, GLuint name, GLsizei length, const GLchar* label);
    void (APIENTRY* glGetObjectLabel)(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei* length, GLchar* label);
    void (APIENTRY* glObjectPtrLabel)(const void* ptr, GLsizei length, const GLchar* label);
    void (APIENTRY* glGetObjectPtrLabel)(const void* ptr, GLsizei bufSize, GLsizei* length, GLchar* label);

    //////////////////////////////////////////////////////////////////////////
    // OpenGL 4.4 Extensions
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glBufferStorage)(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags);
    void (APIENTRY* glClearTexImage)(GLuint texture, GLint level, GLenum format, GLenum type, const void* data);
    void (APIENTRY* glClearTexSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* data);
    void (APIENTRY* glBindBuffersBase)(GLenum target, GLuint first, GLsizei count, const GLuint* buffers);
    void (APIENTRY* glBindBuffersRange)(GLenum target, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizeiptr* sizes);
    void (APIENTRY* glBindTextures)(GLuint first, GLsizei count, const GLuint* textures);
    void (APIENTRY* glBindSamplers)(GLuint first, GLsizei count, const GLuint* samplers);
    void (APIENTRY* glBindImageTextures)(GLuint first, GLsizei count, const GLuint* textures);
    void (APIENTRY* glBindVertexBuffers)(GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizei* strides);

    //////////////////////////////////////////////////////////////////////////
    // OpenGL 4.5 Extensions
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glClipControl)(GLenum origin, GLenum depth);
    void (APIENTRY* glCreateTransformFeedbacks)(GLsizei n, GLuint* ids);
    void (APIENTRY* glTransformFeedbackBufferBase)(GLuint xfb, GLuint index, GLuint buffer);
    void (APIENTRY* glTransformFeedbackBufferRange)(GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
    void (APIENTRY* glGetTransformFeedbackiv)(GLuint xfb, GLenum pname, GLint* param);
    void (APIENTRY* glGetTransformFeedbacki_v)(GLuint xfb, GLenum pname, GLuint index, GLint* param);
    void (APIENTRY* glGetTransformFeedbacki64_v)(GLuint xfb, GLenum pname, GLuint index, GLint64* param);
    void (APIENTRY* glCreateBuffers)(GLsizei n, GLuint* buffers);
    void (APIENTRY* glNamedBufferStorage)(GLuint buffer, GLsizeiptr size, const void* data, GLbitfield flags);
    void (APIENTRY* glNamedBufferData)(GLuint buffer, GLsizeiptr size, const void* data, GLenum usage);
    void (APIENTRY* glNamedBufferSubData)(GLuint buffer, GLintptr offset, GLsizeiptr size, const void* data);
    void (APIENTRY* glCopyNamedBufferSubData)(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
    void (APIENTRY* glClearNamedBufferData)(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void* data);
    void (APIENTRY* glClearNamedBufferSubData)(GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data);
    void* (APIENTRY* glMapNamedBuffer)(GLuint buffer, GLenum access);
    void* (APIENTRY* glMapNamedBufferRange)(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access);
    GLboolean(APIENTRY* glUnmapNamedBuffer)(GLuint buffer);
    void (APIENTRY* glFlushMappedNamedBufferRange)(GLuint buffer, GLintptr offset, GLsizeiptr length);
    void (APIENTRY* glGetNamedBufferParameteriv)(GLuint buffer, GLenum pname, GLint* params);
    void (APIENTRY* glGetNamedBufferParameteri64v)(GLuint buffer, GLenum pname, GLint64* params);
    void (APIENTRY* glGetNamedBufferPointerv)(GLuint buffer, GLenum pname, void** params);
    void (APIENTRY* glGetNamedBufferSubData)(GLuint buffer, GLintptr offset, GLsizeiptr size, void* data);
    void (APIENTRY* glCreateFramebuffers)(GLsizei n, GLuint* framebuffers);
    void (APIENTRY* glNamedFramebufferRenderbuffer)(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    void (APIENTRY* glNamedFramebufferParameteri)(GLuint framebuffer, GLenum pname, GLint param);
    void (APIENTRY* glNamedFramebufferTexture)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
    void (APIENTRY* glNamedFramebufferTextureLayer)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
    void (APIENTRY* glNamedFramebufferDrawBuffer)(GLuint framebuffer, GLenum buf);
    void (APIENTRY* glNamedFramebufferDrawBuffers)(GLuint framebuffer, GLsizei n, const GLenum* bufs);
    void (APIENTRY* glNamedFramebufferReadBuffer)(GLuint framebuffer, GLenum src);
    void (APIENTRY* glInvalidateNamedFramebufferData)(GLuint framebuffer, GLsizei numAttachments, const GLenum* attachments);
    void (APIENTRY* glInvalidateNamedFramebufferSubData)(GLuint framebuffer, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height);
    void (APIENTRY* glClearNamedFramebufferiv)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint* value);
    void (APIENTRY* glClearNamedFramebufferuiv)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint* value);
    void (APIENTRY* glClearNamedFramebufferfv)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat* value);
    void (APIENTRY* glClearNamedFramebufferfi)(GLuint framebuffer, GLenum buffer, const GLfloat depth, GLint stencil);
    void (APIENTRY* glBlitNamedFramebuffer)(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
    GLenum(APIENTRY* glCheckNamedFramebufferStatus)(GLuint framebuffer, GLenum target);
    void (APIENTRY* glGetNamedFramebufferParameteriv)(GLuint framebuffer, GLenum pname, GLint* param);
    void (APIENTRY* glGetNamedFramebufferAttachmentParameteriv)(GLuint framebuffer, GLenum attachment, GLenum pname, GLint* params);
    void (APIENTRY* glCreateRenderbuffers)(GLsizei n, GLuint* renderbuffers);
    void (APIENTRY* glNamedRenderbufferStorage)(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
    void (APIENTRY* glNamedRenderbufferStorageMultisample)(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
    void (APIENTRY* glGetNamedRenderbufferParameteriv)(GLuint renderbuffer, GLenum pname, GLint* params);
    void (APIENTRY* glCreateTextures)(GLenum target, GLsizei n, GLuint* textures);
    void (APIENTRY* glTextureBuffer)(GLuint texture, GLenum internalformat, GLuint buffer);
    void (APIENTRY* glTextureBufferRange)(GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
    void (APIENTRY* glTextureStorage1D)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width);
    void (APIENTRY* glTextureStorage2D)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
    void (APIENTRY* glTextureStorage3D)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
    void (APIENTRY* glTextureStorage2DMultisample)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
    void (APIENTRY* glTextureStorage3DMultisample)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
    void (APIENTRY* glTextureSubImage1D)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels);
    void (APIENTRY* glTextureSubImage2D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
    void (APIENTRY* glTextureSubImage3D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels);
    void (APIENTRY* glCompressedTextureSubImage1D)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data);
    void (APIENTRY* glCompressedTextureSubImage2D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data);
    void (APIENTRY* glCompressedTextureSubImage3D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data);
    void (APIENTRY* glCopyTextureSubImage1D)(GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
    void (APIENTRY* glCopyTextureSubImage2D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void (APIENTRY* glCopyTextureSubImage3D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void (APIENTRY* glTextureParameterf)(GLuint texture, GLenum pname, GLfloat param);
    void (APIENTRY* glTextureParameterfv)(GLuint texture, GLenum pname, const GLfloat* param);
    void (APIENTRY* glTextureParameteri)(GLuint texture, GLenum pname, GLint param);
    void (APIENTRY* glTextureParameterIiv)(GLuint texture, GLenum pname, const GLint* params);
    void (APIENTRY* glTextureParameterIuiv)(GLuint texture, GLenum pname, const GLuint* params);
    void (APIENTRY* glTextureParameteriv)(GLuint texture, GLenum pname, const GLint* param);
    void (APIENTRY* glGenerateTextureMipmap)(GLuint texture);
    void (APIENTRY* glBindTextureUnit)(GLuint unit, GLuint texture);
    void (APIENTRY* glGetTextureImage)(GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* pixels);
    void (APIENTRY* glGetCompressedTextureImage)(GLuint texture, GLint level, GLsizei bufSize, void* pixels);
    void (APIENTRY* glGetTextureLevelParameterfv)(GLuint texture, GLint level, GLenum pname, GLfloat* params);
    void (APIENTRY* glGetTextureLevelParameteriv)(GLuint texture, GLint level, GLenum pname, GLint* params);
    void (APIENTRY* glGetTextureParameterfv)(GLuint texture, GLenum pname, GLfloat* params);
    void (APIENTRY* glGetTextureParameterIiv)(GLuint texture, GLenum pname, GLint* params);
    void (APIENTRY* glGetTextureParameterIuiv)(GLuint texture, GLenum pname, GLuint* params);
    void (APIENTRY* glGetTextureParameteriv)(GLuint texture, GLenum pname, GLint* params);
    void (APIENTRY* glCreateVertexArrays)(GLsizei n, GLuint* arrays);
    void (APIENTRY* glDisableVertexArrayAttrib)(GLuint vaobj, GLuint index);
    void (APIENTRY* glEnableVertexArrayAttrib)(GLuint vaobj, GLuint index);
    void (APIENTRY* glVertexArrayElementBuffer)(GLuint vaobj, GLuint buffer);
    void (APIENTRY* glVertexArrayVertexBuffer)(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
    void (APIENTRY* glVertexArrayVertexBuffers)(GLuint vaobj, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizei* strides);
    void (APIENTRY* glVertexArrayAttribBinding)(GLuint vaobj, GLuint attribindex, GLuint bindingindex);
    void (APIENTRY* glVertexArrayAttribFormat)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
    void (APIENTRY* glVertexArrayAttribIFormat)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
    void (APIENTRY* glVertexArrayAttribLFormat)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
    void (APIENTRY* glVertexArrayBindingDivisor)(GLuint vaobj, GLuint bindingindex, GLuint divisor);
    void (APIENTRY* glGetVertexArrayiv)(GLuint vaobj, GLenum pname, GLint* param);
    void (APIENTRY* glGetVertexArrayIndexediv)(GLuint vaobj, GLuint index, GLenum pname, GLint* param);
    void (APIENTRY* glGetVertexArrayIndexed64iv)(GLuint vaobj, GLuint index, GLenum pname, GLint64* param);
    void (APIENTRY* glCreateSamplers)(GLsizei n, GLuint* samplers);
    void (APIENTRY* glCreateProgramPipelines)(GLsizei n, GLuint* pipelines);
    void (APIENTRY* glCreateQueries)(GLenum target, GLsizei n, GLuint* ids);
    void (APIENTRY* glGetQueryBufferObjecti64v)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
    void (APIENTRY* glGetQueryBufferObjectiv)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
    void (APIENTRY* glGetQueryBufferObjectui64v)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
    void (APIENTRY* glGetQueryBufferObjectuiv)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
    void (APIENTRY* glMemoryBarrierByRegion)(GLbitfield barriers);
    void (APIENTRY* glGetTextureSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void* pixels);
    void (APIENTRY* glGetCompressedTextureSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void* pixels);
    GLenum(APIENTRY* glGetGraphicsResetStatus)(void);
    void (APIENTRY* glGetnCompressedTexImage)(GLenum target, GLint lod, GLsizei bufSize, void* pixels);
    void (APIENTRY* glGetnTexImage)(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* pixels);
    void (APIENTRY* glGetnUniformdv)(GLuint program, GLint location, GLsizei bufSize, GLdouble* params);
    void (APIENTRY* glGetnUniformfv)(GLuint program, GLint location, GLsizei bufSize, GLfloat* params);
    void (APIENTRY* glGetnUniformiv)(GLuint program, GLint location, GLsizei bufSize, GLint* params);
    void (APIENTRY* glGetnUniformuiv)(GLuint program, GLint location, GLsizei bufSize, GLuint* params);
    void (APIENTRY* glReadnPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void* data);
    void (APIENTRY* glGetnMapdv)(GLenum target, GLenum query, GLsizei bufSize, GLdouble* v);
    void (APIENTRY* glGetnMapfv)(GLenum target, GLenum query, GLsizei bufSize, GLfloat* v);
    void (APIENTRY* glGetnMapiv)(GLenum target, GLenum query, GLsizei bufSize, GLint* v);
    void (APIENTRY* glGetnPixelMapfv)(GLenum map, GLsizei bufSize, GLfloat* values);
    void (APIENTRY* glGetnPixelMapuiv)(GLenum map, GLsizei bufSize, GLuint* values);
    void (APIENTRY* glGetnPixelMapusv)(GLenum map, GLsizei bufSize, GLushort* values);
    void (APIENTRY* glGetnPolygonStipple)(GLsizei bufSize, GLubyte* pattern);
    void (APIENTRY* glGetnColorTable)(GLenum target, GLenum format, GLenum type, GLsizei bufSize, void* table);
    void (APIENTRY* glGetnConvolutionFilter)(GLenum target, GLenum format, GLenum type, GLsizei bufSize, void* image);
    void (APIENTRY* glGetnSeparableFilter)(GLenum target, GLenum format, GLenum type, GLsizei rowBufSize, void* row, GLsizei columnBufSize, void* column, void* span);
    void (APIENTRY* glGetnHistogram)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void* values);
    void (APIENTRY* glGetnMinmax)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void* values);
    void (APIENTRY* glTextureBarrier)(void);

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_primitive_restart
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glPrimitiveRestartNV)(void);
    void (APIENTRY* glPrimitiveRestartIndexNV)(GLuint index);

    //////////////////////////////////////////////////////////////////////////
    // GL_HP_occlusion_test extension
    //////////////////////////////////////////////////////////////////////////
    // No functions

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_occlusion_query
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glGenOcclusionQueriesNV)(GLsizei n, GLuint* ids);
    void (APIENTRY* glDeleteOcclusionQueriesNV)(GLsizei n, const GLuint* ids);
    GLboolean(APIENTRY* glIsOcclusionQueryNV)(GLuint id);
    void (APIENTRY* glBeginOcclusionQueryNV)(GLuint id);
    void (APIENTRY* glEndOcclusionQueryNV)(void);
    void (APIENTRY* glGetOcclusionQueryivNV)(GLuint id, GLenum pname, GLint* params);
    void (APIENTRY* glGetOcclusionQueryuivNV)(GLuint id, GLenum pname, GLuint* params);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_occlusion_query extension
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glGenQueriesARB)(GLsizei n, GLuint* ids);
    void (APIENTRY* glDeleteQueriesARB)(GLsizei n, const GLuint* ids);
    GLboolean(APIENTRY* glIsQueryARB)(GLuint id);
    void (APIENTRY* glBeginQueryARB)(GLenum target, GLuint id);
    void (APIENTRY* glEndQueryARB)(GLenum target);
    void (APIENTRY* glGetQueryivARB)(GLenum target, GLenum pname, GLint* params);
    void (APIENTRY* glGetQueryObjectivARB)(GLuint id, GLenum pname, GLint* params);
    void (APIENTRY* glGetQueryObjectuivARB)(GLuint id, GLenum pname, GLuint* params);


    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_texture_cube_map extension
    //////////////////////////////////////////////////////////////////////////
    //  No Functions

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_texture_compression extension
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glCompressedTexImage3DARB)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
    void (APIENTRY* glCompressedTexImage2DARB)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data);
    void (APIENTRY* glCompressedTexImage1DARB)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid* data);
    void (APIENTRY* glCompressedTexSubImage3DARB)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
    void (APIENTRY* glCompressedTexSubImage2DARB)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data);
    void (APIENTRY* glCompressedTexSubImage1DARB)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid* data);
    void (APIENTRY* glGetCompressedTexImageARB)(GLenum target, GLint lod, GLvoid* img);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_vertex_buffer_object extension
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glBindBufferARB)(GLenum target, GLuint buffer);
    void (APIENTRY* glDeleteBuffersARB)(GLsizei n, const GLuint* buffers);
    void (APIENTRY* glGenBuffersARB)(GLsizei n, GLuint* buffers);
    GLboolean(APIENTRY* glIsBufferARB)(GLuint buffer);
    void (APIENTRY* glBufferDataARB)(GLenum target, GLsizeiptrARB size, const GLvoid* data, GLenum usage);
    void (APIENTRY* glBufferSubDataARB)(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid* data);
    void (APIENTRY* glGetBufferSubDataARB)(GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid* data);
    void* (APIENTRY* glMapBufferARB)(GLenum target, GLenum access);
    GLboolean(APIENTRY* glUnmapBufferARB)(GLenum target);
    void (APIENTRY* glGetBufferParameterivARB)(GLenum target, GLenum pname, GLint* params);
    void (APIENTRY* glGetBufferPointervARB)(GLenum target, GLenum pname, GLvoid** params);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_vertex_blend extension
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glWeightbvARB)(GLint size, const GLbyte* weights);
    void (APIENTRY* glWeightsvARB)(GLint size, const GLshort* weights);
    void (APIENTRY* glWeightivARB)(GLint size, const GLint* weights);
    void (APIENTRY* glWeightfvARB)(GLint size, const GLfloat* weights);
    void (APIENTRY* glWeightdvARB)(GLint size, const GLdouble* weights);
    void (APIENTRY* glWeightubvARB)(GLint size, const GLubyte* weights);
    void (APIENTRY* glWeightusvARB)(GLint size, const GLushort* weights);
    void (APIENTRY* glWeightuivARB)(GLint size, const GLuint* weights);
    void (APIENTRY* glWeightPointerARB)(GLint size, GLenum type, GLsizei stribe, const GLvoid* pointer);
    void (APIENTRY* glVertexBlendARB)(GLint count);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_texture3D
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glTexImage3DEXT)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
    void (APIENTRY* glTexSubImage3DEXT)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_vertex_program
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glVertexAttrib1dARB)(GLuint index, GLdouble x);
    void (APIENTRY* glVertexAttrib1dvARB)(GLuint index, const GLdouble* v);
    void (APIENTRY* glVertexAttrib1fARB)(GLuint index, GLfloat x);
    void (APIENTRY* glVertexAttrib1fvARB)(GLuint index, const GLfloat* v);
    void (APIENTRY* glVertexAttrib1sARB)(GLuint index, GLshort x);
    void (APIENTRY* glVertexAttrib1svARB)(GLuint index, const GLshort* v);
    void (APIENTRY* glVertexAttrib2dARB)(GLuint index, GLdouble x, GLdouble y);
    void (APIENTRY* glVertexAttrib2dvARB)(GLuint index, const GLdouble* v);
    void (APIENTRY* glVertexAttrib2fARB)(GLuint index, GLfloat x, GLfloat y);
    void (APIENTRY* glVertexAttrib2fvARB)(GLuint index, const GLfloat* v);
    void (APIENTRY* glVertexAttrib2sARB)(GLuint index, GLshort x, GLshort y);
    void (APIENTRY* glVertexAttrib2svARB)(GLuint index, const GLshort* v);
    void (APIENTRY* glVertexAttrib3dARB)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
    void (APIENTRY* glVertexAttrib3dvARB)(GLuint index, const GLdouble* v);
    void (APIENTRY* glVertexAttrib3fARB)(GLuint index, GLfloat x, GLfloat y, GLfloat z);
    void (APIENTRY* glVertexAttrib3fvARB)(GLuint index, const GLfloat* v);
    void (APIENTRY* glVertexAttrib3sARB)(GLuint index, GLshort x, GLshort y, GLshort z);
    void (APIENTRY* glVertexAttrib3svARB)(GLuint index, const GLshort* v);
    void (APIENTRY* glVertexAttrib4NbvARB)(GLuint index, const GLbyte* v);
    void (APIENTRY* glVertexAttrib4NivARB)(GLuint index, const GLint* v);
    void (APIENTRY* glVertexAttrib4NsvARB)(GLuint index, const GLshort* v);
    void (APIENTRY* glVertexAttrib4NubARB)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
    void (APIENTRY* glVertexAttrib4NubvARB)(GLuint index, const GLubyte* v);
    void (APIENTRY* glVertexAttrib4NuivARB)(GLuint index, const GLuint* v);
    void (APIENTRY* glVertexAttrib4NusvARB)(GLuint index, const GLushort* v);
    void (APIENTRY* glVertexAttrib4bvARB)(GLuint index, const GLbyte* v);
    void (APIENTRY* glVertexAttrib4dARB)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void (APIENTRY* glVertexAttrib4dvARB)(GLuint index, const GLdouble* v);
    void (APIENTRY* glVertexAttrib4fARB)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void (APIENTRY* glVertexAttrib4fvARB)(GLuint index, const GLfloat* v);
    void (APIENTRY* glVertexAttrib4ivARB)(GLuint index, const GLint* v);
    void (APIENTRY* glVertexAttrib4sARB)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
    void (APIENTRY* glVertexAttrib4svARB)(GLuint index, const GLshort* v);
    void (APIENTRY* glVertexAttrib4ubvARB)(GLuint index, const GLubyte* v);
    void (APIENTRY* glVertexAttrib4uivARB)(GLuint index, const GLuint* v);
    void (APIENTRY* glVertexAttrib4usvARB)(GLuint index, const GLushort* v);
    void (APIENTRY* glVertexAttribPointerARB)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
    void (APIENTRY* glEnableVertexAttribArrayARB)(GLuint index);
    void (APIENTRY* glDisableVertexAttribArrayARB)(GLuint index);
    void (APIENTRY* glProgramStringARB)(GLenum target, GLenum format, GLsizei len, const GLvoid* string);
    void (APIENTRY* glBindProgramARB)(GLenum target, GLuint program);
    void (APIENTRY* glDeleteProgramsARB)(GLsizei n, const GLuint* programs);
    void (APIENTRY* glGenProgramsARB)(GLsizei n, GLuint* programs);
    void (APIENTRY* glProgramEnvParameter4dARB)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void (APIENTRY* glProgramEnvParameter4dvARB)(GLenum target, GLuint index, const GLdouble* params);
    void (APIENTRY* glProgramEnvParameter4fARB)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void (APIENTRY* glProgramEnvParameter4fvARB)(GLenum target, GLuint index, const GLfloat* params);
    void (APIENTRY* glProgramLocalParameter4dARB)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void (APIENTRY* glProgramLocalParameter4dvARB)(GLenum target, GLuint index, const GLdouble* params);
    void (APIENTRY* glProgramLocalParameter4fARB)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void (APIENTRY* glProgramLocalParameter4fvARB)(GLenum target, GLuint index, const GLfloat* params);
    void (APIENTRY* glGetProgramEnvParameterdvARB)(GLenum target, GLuint index, GLdouble* params);
    void (APIENTRY* glGetProgramEnvParameterfvARB)(GLenum target, GLuint index, GLfloat* params);
    void (APIENTRY* glGetProgramLocalParameterdvARB)(GLenum target, GLuint index, GLdouble* params);
    void (APIENTRY* glGetProgramLocalParameterfvARB)(GLenum target, GLuint index, GLfloat* params);
    void (APIENTRY* glGetProgramivARB)(GLenum target, GLenum pname, GLint* params);
    void (APIENTRY* glGetProgramStringARB)(GLenum target, GLenum pname, GLvoid* string);
    void (APIENTRY* glGetVertexAttribdvARB)(GLuint index, GLenum pname, GLdouble* params);
    void (APIENTRY* glGetVertexAttribfvARB)(GLuint index, GLenum pname, GLfloat* params);
    void (APIENTRY* glGetVertexAttribivARB)(GLuint index, GLenum pname, GLint* params);
    void (APIENTRY* glGetVertexAttribPointervARB)(GLuint index, GLenum pname, GLvoid** pointer);
    GLboolean(APIENTRY* glIsProgramARB)(GLuint program);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_vertex_shader
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glBindAttribLocationARB)(GLhandleARB programObj, GLuint index, const GLcharARB* name);
    void (APIENTRY* glGetActiveAttribARB)(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei* length, GLint* size, GLenum* type, GLcharARB* name);
    GLint(APIENTRY* glGetAttribLocationARB)(GLhandleARB programObj, const GLcharARB* name);


    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_shader_objects
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glDeleteObjectARB)(GLhandleARB obj);
    GLhandleARB(APIENTRY* glGetHandleARB)(GLenum pname);
    void (APIENTRY* glDetachObjectARB)(GLhandleARB containerObj, GLhandleARB attachedObj);
    GLhandleARB(APIENTRY* glCreateShaderObjectARB)(GLenum shaderType);
    void (APIENTRY* glShaderSourceARB)(GLhandleARB shaderObj, GLsizei count, const GLcharARB* const* string, const GLint* length);
    void (APIENTRY* glCompileShaderARB)(GLhandleARB shaderObj);
    GLhandleARB(APIENTRY* glCreateProgramObjectARB)(void);
    void (APIENTRY* glAttachObjectARB)(GLhandleARB containerObj, GLhandleARB obj);
    void (APIENTRY* glLinkProgramARB)(GLhandleARB programObj);
    void (APIENTRY* glUseProgramObjectARB)(GLhandleARB programObj);
    void (APIENTRY* glValidateProgramARB)(GLhandleARB programObj);
    void (APIENTRY* glUniform1fARB)(GLint location, GLfloat v0);
    void (APIENTRY* glUniform2fARB)(GLint location, GLfloat v0, GLfloat v1);
    void (APIENTRY* glUniform3fARB)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    void (APIENTRY* glUniform4fARB)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    void (APIENTRY* glUniform1iARB)(GLint location, GLint v0);
    void (APIENTRY* glUniform2iARB)(GLint location, GLint v0, GLint v1);
    void (APIENTRY* glUniform3iARB)(GLint location, GLint v0, GLint v1, GLint v2);
    void (APIENTRY* glUniform4iARB)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
    void (APIENTRY* glUniform1fvARB)(GLint location, GLsizei count, const GLfloat* value);
    void (APIENTRY* glUniform2fvARB)(GLint location, GLsizei count, const GLfloat* value);
    void (APIENTRY* glUniform3fvARB)(GLint location, GLsizei count, const GLfloat* value);
    void (APIENTRY* glUniform4fvARB)(GLint location, GLsizei count, const GLfloat* value);
    void (APIENTRY* glUniform1ivARB)(GLint location, GLsizei count, const GLint* value);
    void (APIENTRY* glUniform2ivARB)(GLint location, GLsizei count, const GLint* value);
    void (APIENTRY* glUniform3ivARB)(GLint location, GLsizei count, const GLint* value);
    void (APIENTRY* glUniform4ivARB)(GLint location, GLsizei count, const GLint* value);
    void (APIENTRY* glUniformMatrix2fvARB)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glUniformMatrix3fvARB)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glUniformMatrix4fvARB)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glGetObjectParameterfvARB)(GLhandleARB obj, GLenum pname, GLfloat* params);
    void (APIENTRY* glGetObjectParameterivARB)(GLhandleARB obj, GLenum pname, GLint* params);
    void (APIENTRY* glGetInfoLogARB)(GLhandleARB obj, GLsizei maxLength, GLsizei* length, GLcharARB* infoLog);
    void (APIENTRY* glGetAttachedObjectsARB)(GLhandleARB containerObj, GLsizei maxCount, GLsizei* count, GLhandleARB* obj);
    GLint(APIENTRY* glGetUniformLocationARB)(GLhandleARB programObj, const GLcharARB* name);
    void (APIENTRY* glGetActiveUniformARB)(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei* length, GLint* size, GLenum* type, GLcharARB* name);
    void (APIENTRY* glGetUniformfvARB)(GLhandleARB programObj, GLint location, GLfloat* params);
    void (APIENTRY* glGetUniformivARB)(GLhandleARB programObj, GLint location, GLint* params);
    void (APIENTRY* glGetShaderSourceARB)(GLhandleARB obj, GLsizei maxLength, GLsizei* length, GLcharARB* source);

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_vertex_program
    //////////////////////////////////////////////////////////////////////////
    GLboolean(APIENTRY* glAreProgramsResidentNV)(GLsizei n, const GLuint* programs, GLboolean* residences);
    void (APIENTRY* glBindProgramNV)(GLenum target, GLuint id);
    void (APIENTRY* glDeleteProgramsNV)(GLsizei n, const GLuint* programs);
    void (APIENTRY* glExecuteProgramNV)(GLenum target, GLuint id, const GLfloat* params);
    void (APIENTRY* glGenProgramsNV)(GLsizei n, GLuint* programs);
    void (APIENTRY* glGetProgramParameterdvNV)(GLenum target, GLuint index, GLenum pname, GLdouble* params);
    void (APIENTRY* glGetProgramParameterfvNV)(GLenum target, GLuint index, GLenum pname, GLfloat* params);
    void (APIENTRY* glGetProgramivNV)(GLuint id, GLenum pname, GLint* params);
    void (APIENTRY* glGetProgramStringNV)(GLuint id, GLenum pname, GLubyte* program);
    void (APIENTRY* glGetTrackMatrixivNV)(GLenum target, GLuint address, GLenum pname, GLint* params);
    void (APIENTRY* glGetVertexAttribdvNV)(GLuint index, GLenum pname, GLdouble* params);
    void (APIENTRY* glGetVertexAttribfvNV)(GLuint index, GLenum pname, GLfloat* params);
    void (APIENTRY* glGetVertexAttribivNV)(GLuint index, GLenum pname, GLint* params);
    void (APIENTRY* glGetVertexAttribPointervNV)(GLuint index, GLenum pname, GLvoid** pointer);
    GLboolean(APIENTRY* glIsProgramNV)(GLuint id);
    void (APIENTRY* glLoadProgramNV)(GLenum target, GLuint id, GLsizei len, const GLubyte* program);
    void (APIENTRY* glProgramParameter4dNV)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void (APIENTRY* glProgramParameter4dvNV)(GLenum target, GLuint index, const GLdouble* v);
    void (APIENTRY* glProgramParameter4fNV)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void (APIENTRY* glProgramParameter4fvNV)(GLenum target, GLuint index, const GLfloat* v);
    void (APIENTRY* glProgramParameters4dvNV)(GLenum target, GLuint index, GLsizei count, const GLdouble* v);
    void (APIENTRY* glProgramParameters4fvNV)(GLenum target, GLuint index, GLsizei count, const GLfloat* v);
    void (APIENTRY* glRequestResidentProgramsNV)(GLsizei n, const GLuint* programs);
    void (APIENTRY* glTrackMatrixNV)(GLenum target, GLuint address, GLenum matrix, GLenum transform);
    void (APIENTRY* glVertexAttribPointerNV)(GLuint index, GLint fsize, GLenum type, GLsizei stride, const GLvoid* pointer);
    void (APIENTRY* glVertexAttrib1dNV)(GLuint index, GLdouble x);
    void (APIENTRY* glVertexAttrib1dvNV)(GLuint index, const GLdouble* v);
    void (APIENTRY* glVertexAttrib1fNV)(GLuint index, GLfloat x);
    void (APIENTRY* glVertexAttrib1fvNV)(GLuint index, const GLfloat* v);
    void (APIENTRY* glVertexAttrib1sNV)(GLuint index, GLshort x);
    void (APIENTRY* glVertexAttrib1svNV)(GLuint index, const GLshort* v);
    void (APIENTRY* glVertexAttrib2dNV)(GLuint index, GLdouble x, GLdouble y);
    void (APIENTRY* glVertexAttrib2dvNV)(GLuint index, const GLdouble* v);
    void (APIENTRY* glVertexAttrib2fNV)(GLuint index, GLfloat x, GLfloat y);
    void (APIENTRY* glVertexAttrib2fvNV)(GLuint index, const GLfloat* v);
    void (APIENTRY* glVertexAttrib2sNV)(GLuint index, GLshort x, GLshort y);
    void (APIENTRY* glVertexAttrib2svNV)(GLuint index, const GLshort* v);
    void (APIENTRY* glVertexAttrib3dNV)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
    void (APIENTRY* glVertexAttrib3dvNV)(GLuint index, const GLdouble* v);
    void (APIENTRY* glVertexAttrib3fNV)(GLuint index, GLfloat x, GLfloat y, GLfloat z);
    void (APIENTRY* glVertexAttrib3fvNV)(GLuint index, const GLfloat* v);
    void (APIENTRY* glVertexAttrib3sNV)(GLuint index, GLshort x, GLshort y, GLshort z);
    void (APIENTRY* glVertexAttrib3svNV)(GLuint index, const GLshort* v);
    void (APIENTRY* glVertexAttrib4dNV)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void (APIENTRY* glVertexAttrib4dvNV)(GLuint index, const GLdouble* v);
    void (APIENTRY* glVertexAttrib4fNV)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void (APIENTRY* glVertexAttrib4fvNV)(GLuint index, const GLfloat* v);
    void (APIENTRY* glVertexAttrib4sNV)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
    void (APIENTRY* glVertexAttrib4svNV)(GLuint index, const GLshort* v);
    void (APIENTRY* glVertexAttrib4ubNV)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
    void (APIENTRY* glVertexAttrib4ubvNV)(GLuint index, const GLubyte* v);
    void (APIENTRY* glVertexAttribs1dvNV)(GLuint index, GLsizei count, const GLdouble* v);
    void (APIENTRY* glVertexAttribs1fvNV)(GLuint index, GLsizei count, const GLfloat* v);
    void (APIENTRY* glVertexAttribs1svNV)(GLuint index, GLsizei count, const GLshort* v);
    void (APIENTRY* glVertexAttribs2dvNV)(GLuint index, GLsizei count, const GLdouble* v);
    void (APIENTRY* glVertexAttribs2fvNV)(GLuint index, GLsizei count, const GLfloat* v);
    void (APIENTRY* glVertexAttribs2svNV)(GLuint index, GLsizei count, const GLshort* v);
    void (APIENTRY* glVertexAttribs3dvNV)(GLuint index, GLsizei count, const GLdouble* v);
    void (APIENTRY* glVertexAttribs3fvNV)(GLuint index, GLsizei count, const GLfloat* v);
    void (APIENTRY* glVertexAttribs3svNV)(GLuint index, GLsizei count, const GLshort* v);
    void (APIENTRY* glVertexAttribs4dvNV)(GLuint index, GLsizei count, const GLdouble* v);
    void (APIENTRY* glVertexAttribs4fvNV)(GLuint index, GLsizei count, const GLfloat* v);
    void (APIENTRY* glVertexAttribs4svNV)(GLuint index, GLsizei count, const GLshort* v);
    void (APIENTRY* glVertexAttribs4ubvNV)(GLuint index, GLsizei count, const GLubyte* v);

    //////////////////////////////////////////////////////////////////////////
    // GL_ATI_fragment_shader
    //////////////////////////////////////////////////////////////////////////
    GLuint(APIENTRY* glGenFragmentShadersATI)(GLuint range);
    void (APIENTRY* glBindFragmentShaderATI)(GLuint id);
    void (APIENTRY* glDeleteFragmentShaderATI)(GLuint id);
    void (APIENTRY* glBeginFragmentShaderATI)(void);
    void (APIENTRY* glEndFragmentShaderATI)(void);
    void (APIENTRY* glPassTexCoordATI)(GLuint dst, GLuint coord, GLenum swizzle);
    void (APIENTRY* glSampleMapATI)(GLuint dst, GLuint interp, GLenum swizzle);
    void (APIENTRY* glColorFragmentOp1ATI)(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod);
    void (APIENTRY* glColorFragmentOp2ATI)(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod);
    void (APIENTRY* glColorFragmentOp3ATI)(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod);
    void (APIENTRY* glAlphaFragmentOp1ATI)(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod);
    void (APIENTRY* glAlphaFragmentOp2ATI)(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod);
    void (APIENTRY* glAlphaFragmentOp3ATI)(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod);
    void (APIENTRY* glSetFragmentShaderConstantATI)(GLuint dst, const GLfloat* value);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_vertex_shader
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glBeginVertexShaderEXT)(void);
    void (APIENTRY* glEndVertexShaderEXT)(void);
    void (APIENTRY* glBindVertexShaderEXT)(GLuint id);
    GLuint(APIENTRY* glGenVertexShadersEXT)(GLuint range);
    void (APIENTRY* glDeleteVertexShaderEXT)(GLuint id);
    void (APIENTRY* glShaderOp1EXT)(GLenum op, GLuint res, GLuint arg1);
    void (APIENTRY* glShaderOp2EXT)(GLenum op, GLuint res, GLuint arg1, GLuint arg2);
    void (APIENTRY* glShaderOp3EXT)(GLenum op, GLuint res, GLuint arg1, GLuint arg2, GLuint arg3);
    void (APIENTRY* glSwizzleEXT)(GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW);
    void (APIENTRY* glWriteMaskEXT)(GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW);
    void (APIENTRY* glInsertComponentEXT)(GLuint res, GLuint src, GLuint num);
    void (APIENTRY* glExtractComponentEXT)(GLuint res, GLuint src, GLuint num);
    GLuint(APIENTRY* glGenSymbolsEXT)(GLenum datatype, GLenum storagetype, GLenum range, GLuint components);
    void (APIENTRY* glSetInvariantEXT)(GLuint id, GLenum type, const GLvoid* addr);
    void (APIENTRY* glSetLocalConstantEXT)(GLuint id, GLenum type, const GLvoid* addr);
    void (APIENTRY* glVariantbvEXT)(GLuint id, const GLbyte* addr);
    void (APIENTRY* glVariantsvEXT)(GLuint id, const GLshort* addr);
    void (APIENTRY* glVariantivEXT)(GLuint id, const GLint* addr);
    void (APIENTRY* glVariantfvEXT)(GLuint id, const GLfloat* addr);
    void (APIENTRY* glVariantdvEXT)(GLuint id, const GLdouble* addr);
    void (APIENTRY* glVariantubvEXT)(GLuint id, const GLubyte* addr);
    void (APIENTRY* glVariantusvEXT)(GLuint id, const GLushort* addr);
    void (APIENTRY* glVariantuivEXT)(GLuint id, const GLuint* addr);
    void (APIENTRY* glVariantPointerEXT)(GLuint id, GLenum type, GLuint stride, const GLvoid* addr);
    void (APIENTRY* glEnableVariantClientStateEXT)(GLuint id);
    void (APIENTRY* glDisableVariantClientStateEXT)(GLuint id);
    GLuint(APIENTRY* glBindLightParameterEXT)(GLenum light, GLenum value);
    GLuint(APIENTRY* glBindMaterialParameterEXT)(GLenum face, GLenum value);
    GLuint(APIENTRY* glBindTexGenParameterEXT)(GLenum unit, GLenum coord, GLenum value);
    GLuint(APIENTRY* glBindTextureUnitParameterEXT)(GLenum unit, GLenum value);
    GLuint(APIENTRY* glBindParameterEXT)(GLenum value);
    GLboolean(APIENTRY* glIsVariantEnabledEXT)(GLuint id, GLenum cap);
    void (APIENTRY* glGetVariantBooleanvEXT)(GLuint id, GLenum value, GLboolean* data);
    void (APIENTRY* glGetVariantIntegervEXT)(GLuint id, GLenum value, GLint* data);
    void (APIENTRY* glGetVariantFloatvEXT)(GLuint id, GLenum value, GLfloat* data);
    void (APIENTRY* glGetVariantPointervEXT)(GLuint id, GLenum value, GLvoid** data);
    void (APIENTRY* glGetInvariantBooleanvEXT)(GLuint id, GLenum value, GLboolean* data);
    void (APIENTRY* glGetInvariantIntegervEXT)(GLuint id, GLenum value, GLint* data);
    void (APIENTRY* glGetInvariantFloatvEXT)(GLuint id, GLenum value, GLfloat* data);
    void (APIENTRY* glGetLocalConstantBooleanvEXT)(GLuint id, GLenum value, GLboolean* data);
    void (APIENTRY* glGetLocalConstantIntegervEXT)(GLuint id, GLenum value, GLint* data);
    void (APIENTRY* glGetLocalConstantFloatvEXT)(GLuint id, GLenum value, GLfloat* data);

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_fragment_program
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glProgramNamedParameter4fNV)(GLuint id, GLsizei len, const GLubyte* name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void (APIENTRY* glProgramNamedParameter4dNV)(GLuint id, GLsizei len, const GLubyte* name, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void (APIENTRY* glProgramNamedParameter4fvNV)(GLuint id, GLsizei len, const GLubyte* name, const GLfloat* v);
    void (APIENTRY* glProgramNamedParameter4dvNV)(GLuint id, GLsizei len, const GLubyte* name, const GLdouble* v);
    void (APIENTRY* glGetProgramNamedParameterfvNV)(GLuint id, GLsizei len, const GLubyte* name, GLfloat* params);
    void (APIENTRY* glGetProgramNamedParameterdvNV)(GLuint id, GLsizei len, const GLubyte* name, GLdouble* params);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_draw_buffers
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glDrawBuffersARB)(GLsizei n, const GLenum* bufs);

    //////////////////////////////////////////////////////////////////////////
    // GL_ATI_draw_buffers
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glDrawBuffersATI)(GLsizei n, const GLenum* bufs);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_multitexture
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glActiveTextureARB)(GLenum texture);
    void (APIENTRY* glClientActiveTextureARB)(GLenum texture);
    void (APIENTRY* glMultiTexCoord1dARB)(GLenum target, GLdouble s);
    void (APIENTRY* glMultiTexCoord1dvARB)(GLenum target, const GLdouble* v);
    void (APIENTRY* glMultiTexCoord1fARB)(GLenum target, GLfloat s);
    void (APIENTRY* glMultiTexCoord1fvARB)(GLenum target, const GLfloat* v);
    void (APIENTRY* glMultiTexCoord1iARB)(GLenum target, GLint s);
    void (APIENTRY* glMultiTexCoord1ivARB)(GLenum target, const GLint* v);
    void (APIENTRY* glMultiTexCoord1sARB)(GLenum target, GLshort s);
    void (APIENTRY* glMultiTexCoord1svARB)(GLenum target, const GLshort* v);
    void (APIENTRY* glMultiTexCoord2dARB)(GLenum target, GLdouble s, GLdouble t);
    void (APIENTRY* glMultiTexCoord2dvARB)(GLenum target, const GLdouble* v);
    void (APIENTRY* glMultiTexCoord2fARB)(GLenum target, GLfloat s, GLfloat t);
    void (APIENTRY* glMultiTexCoord2fvARB)(GLenum target, const GLfloat* v);
    void (APIENTRY* glMultiTexCoord2iARB)(GLenum target, GLint s, GLint t);
    void (APIENTRY* glMultiTexCoord2ivARB)(GLenum target, const GLint* v);
    void (APIENTRY* glMultiTexCoord2sARB)(GLenum target, GLshort s, GLshort t);
    void (APIENTRY* glMultiTexCoord2svARB)(GLenum target, const GLshort* v);
    void (APIENTRY* glMultiTexCoord3dARB)(GLenum target, GLdouble s, GLdouble t, GLdouble r);
    void (APIENTRY* glMultiTexCoord3dvARB)(GLenum target, const GLdouble* v);
    void (APIENTRY* glMultiTexCoord3fARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r);
    void (APIENTRY* glMultiTexCoord3fvARB)(GLenum target, const GLfloat* v);
    void (APIENTRY* glMultiTexCoord3iARB)(GLenum target, GLint s, GLint t, GLint r);
    void (APIENTRY* glMultiTexCoord3ivARB)(GLenum target, const GLint* v);
    void (APIENTRY* glMultiTexCoord3sARB)(GLenum target, GLshort s, GLshort t, GLshort r);
    void (APIENTRY* glMultiTexCoord3svARB)(GLenum target, const GLshort* v);
    void (APIENTRY* glMultiTexCoord4dARB)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
    void (APIENTRY* glMultiTexCoord4dvARB)(GLenum target, const GLdouble* v);
    void (APIENTRY* glMultiTexCoord4fARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
    void (APIENTRY* glMultiTexCoord4fvARB)(GLenum target, const GLfloat* v);
    void (APIENTRY* glMultiTexCoord4iARB)(GLenum target, GLint s, GLint t, GLint r, GLint q);
    void (APIENTRY* glMultiTexCoord4ivARB)(GLenum target, const GLint* v);
    void (APIENTRY* glMultiTexCoord4sARB)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
    void (APIENTRY* glMultiTexCoord4svARB)(GLenum target, const GLshort* v);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_color_buffer_float
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glClampColorARB)(GLenum target, GLenum clamp);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_stencil_two_side
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glActiveStencilFaceEXT)(GLenum face);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_framebuffer_object
    //////////////////////////////////////////////////////////////////////////
    GLboolean(APIENTRY* glIsRenderbufferEXT)(GLuint renderbuffer);
    void (APIENTRY* glBindRenderbufferEXT)(GLenum target, GLuint renderbuffer);
    void (APIENTRY* glDeleteRenderbuffersEXT)(GLsizei n, const GLuint* renderbuffers);
    void (APIENTRY* glGenRenderbuffersEXT)(GLsizei n, GLuint* renderbuffers);
    void (APIENTRY* glRenderbufferStorageEXT)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
    void (APIENTRY* glGetRenderbufferParameterivEXT)(GLenum target, GLenum pname, GLint* params);
    GLboolean(APIENTRY* glIsFramebufferEXT)(GLuint framebuffer);
    void (APIENTRY* glBindFramebufferEXT)(GLenum target, GLuint framebuffer);
    void (APIENTRY* glDeleteFramebuffersEXT)(GLsizei n, const GLuint* framebuffers);
    void (APIENTRY* glGenFramebuffersEXT)(GLsizei n, GLuint* framebuffers);
    GLenum(APIENTRY* glCheckFramebufferStatusEXT)(GLenum target);
    void (APIENTRY* glFramebufferTexture1DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    void (APIENTRY* glFramebufferTexture2DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    void (APIENTRY* glFramebufferTexture3DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
    void (APIENTRY* glFramebufferRenderbufferEXT)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    void (APIENTRY* glGetFramebufferAttachmentParameterivEXT)(GLenum target, GLenum attachment, GLenum pname, GLint* params);
    void (APIENTRY* glGenerateMipmapEXT)(GLenum target);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_framebuffer_blit
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glBlitFramebufferEXT)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_framebuffer_multisample
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glRenderbufferStorageMultisampleEXT)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_framebuffer_object
    //////////////////////////////////////////////////////////////////////////
    GLboolean(APIENTRY* glIsRenderbuffer)(GLuint renderbuffer);
    void (APIENTRY* glBindRenderbuffer)(GLenum target, GLuint renderbuffer);
    void (APIENTRY* glDeleteRenderbuffers)(GLsizei n, const GLuint* renderbuffers);
    void (APIENTRY* glGenRenderbuffers)(GLsizei n, GLuint* renderbuffers);
    void (APIENTRY* glRenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
    void (APIENTRY* glGetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint* params);
    GLboolean(APIENTRY* glIsFramebuffer)(GLuint framebuffer);
    void (APIENTRY* glBindFramebuffer)(GLenum target, GLuint framebuffer);
    void (APIENTRY* glDeleteFramebuffers)(GLsizei n, const GLuint* framebuffers);
    void (APIENTRY* glGenFramebuffers)(GLsizei n, GLuint* framebuffers);
    GLenum(APIENTRY* glCheckFramebufferStatus)(GLenum target);
    void (APIENTRY* glFramebufferTexture1D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    void (APIENTRY* glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    void (APIENTRY* glFramebufferTexture3D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
    void (APIENTRY* glFramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    void (APIENTRY* glGetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint* params);
    void (APIENTRY* glGenerateMipmap)(GLenum target);
    void (APIENTRY* glBlitFramebuffer)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
    void (APIENTRY* glRenderbufferStorageMultisample)(GLenum, GLsizei, GLenum, GLsizei, GLsizei);
    void (APIENTRY* glFramebufferTextureLayer)(GLenum, GLenum, GLuint, GLint, GLint);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_direct_state_access
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glClientAttribDefaultEXT)(GLbitfield mask);
    void (APIENTRY* glPushClientAttribDefaultEXT)(GLbitfield mask);
    void (APIENTRY* glMatrixLoadfEXT)(GLenum matrixMode, const GLfloat* m);
    void (APIENTRY* glMatrixLoaddEXT)(GLenum matrixMode, const GLdouble* m);
    void (APIENTRY* glMatrixMultfEXT)(GLenum matrixMode, const GLfloat* m);
    void (APIENTRY* glMatrixMultdEXT)(GLenum matrixMode, const GLdouble* m);
    void (APIENTRY* glMatrixLoadIdentityEXT)(GLenum matrixMode);
    void (APIENTRY* glMatrixRotatefEXT)(GLenum matrixMode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
    void (APIENTRY* glMatrixRotatedEXT)(GLenum matrixMode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
    void (APIENTRY* glMatrixScalefEXT)(GLenum matrixMode, GLfloat x, GLfloat y, GLfloat z);
    void (APIENTRY* glMatrixScaledEXT)(GLenum matrixMode, GLdouble x, GLdouble y, GLdouble z);
    void (APIENTRY* glMatrixTranslatefEXT)(GLenum matrixMode, GLfloat x, GLfloat y, GLfloat z);
    void (APIENTRY* glMatrixTranslatedEXT)(GLenum matrixMode, GLdouble x, GLdouble y, GLdouble z);
    void (APIENTRY* glMatrixFrustumEXT)(GLenum matrixMode, GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f);
    void (APIENTRY* glMatrixOrthoEXT)(GLenum matrixMode, GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f);
    void (APIENTRY* glMatrixPopEXT)(GLenum matrixMode);
    void (APIENTRY* glMatrixPushEXT)(GLenum matrixMode);
    void (APIENTRY* glMatrixLoadTransposefEXT)(GLenum matrixMode, const GLfloat* m);
    void (APIENTRY* glMatrixLoadTransposedEXT)(GLenum matrixMode, const GLdouble* m);
    void (APIENTRY* glMatrixMultTransposefEXT)(GLenum matrixMode, const GLfloat* m);
    void (APIENTRY* glMatrixMultTransposedEXT)(GLenum matrixMode, const GLdouble* m);
    void (APIENTRY* glTextureParameterfEXT)(GLuint texture, GLenum target, GLenum pname, GLfloat param);
    void (APIENTRY* glTextureParameterfvEXT)(GLuint texture, GLenum target, GLenum pname, const GLfloat* param);
    void (APIENTRY* glTextureParameteriEXT)(GLuint texture, GLenum target, GLenum pname, GLint param);
    void (APIENTRY* glTextureParameterivEXT)(GLuint texture, GLenum target, GLenum pname, const GLint* param);
    void (APIENTRY* glTextureImage1DEXT)(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
    void (APIENTRY* glTextureImage2DEXT)(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
    void (APIENTRY* glTextureSubImage1DEXT)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid* pixels);
    void (APIENTRY* glTextureSubImage2DEXT)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
    void (APIENTRY* glCopyTextureImage1DEXT)(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
    void (APIENTRY* glCopyTextureImage2DEXT)(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
    void (APIENTRY* glCopyTextureSubImage1DEXT)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
    void (APIENTRY* glCopyTextureSubImage2DEXT)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void (APIENTRY* glGetTextureImageEXT)(GLuint texture, GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels);
    void (APIENTRY* glGetTextureParameterfvEXT)(GLuint texture, GLenum target, GLenum pname, GLfloat* params);
    void (APIENTRY* glGetTextureParameterivEXT)(GLuint texture, GLenum target, GLenum pname, GLint* params);
    void (APIENTRY* glGetTextureLevelParameterfvEXT)(GLuint texture, GLenum target, GLint level, GLenum pname, GLfloat* params);
    void (APIENTRY* glGetTextureLevelParameterivEXT)(GLuint texture, GLenum target, GLint level, GLenum pname, GLint* params);
    void (APIENTRY* glTextureImage3DEXT)(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
    void (APIENTRY* glTextureSubImage3DEXT)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
    void (APIENTRY* glCopyTextureSubImage3DEXT)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

    void (APIENTRY* glMultiTexParameterfEXT)(GLenum texunit, GLenum target, GLenum pname, GLfloat param);
    void (APIENTRY* glMultiTexParameterfvEXT)(GLenum texunit, GLenum target, GLenum pname, const GLfloat* param);
    void (APIENTRY* glMultiTexParameteriEXT)(GLenum texunit, GLenum target, GLenum pname, GLint param);
    void (APIENTRY* glMultiTexParameterivEXT)(GLenum texunit, GLenum target, GLenum pname, const GLint* param);
    void (APIENTRY* glMultiTexImage1DEXT)(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
    void (APIENTRY* glMultiTexImage2DEXT)(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
    void (APIENTRY* glMultiTexSubImage1DEXT)(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid* pixels);
    void (APIENTRY* glMultiTexSubImage2DEXT)(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
    void (APIENTRY* glCopyMultiTexImage1DEXT)(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
    void (APIENTRY* glCopyMultiTexImage2DEXT)(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
    void (APIENTRY* glCopyMultiTexSubImage1DEXT)(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
    void (APIENTRY* glCopyMultiTexSubImage2DEXT)(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void (APIENTRY* glGetMultiTexImageEXT)(GLenum texunit, GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels);
    void (APIENTRY* glGetMultiTexParameterfvEXT)(GLenum texunit, GLenum target, GLenum pname, GLfloat* param);
    void (APIENTRY* glGetMultiTexParameterivEXT)(GLenum texunit, GLenum target, GLenum pname, GLint* param);
    void (APIENTRY* glGetMultiTexLevelParameterfvEXT)(GLenum texunit, GLenum target, GLint level, GLenum pname, GLfloat* param);
    void (APIENTRY* glGetMultiTexLevelParameterivEXT)(GLenum texunit, GLenum target, GLint level, GLenum pname, GLint* param);
    void (APIENTRY* glMultiTexImage3DEXT)(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
    void (APIENTRY* glMultiTexSubImage3DEXT)(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
    void (APIENTRY* glCopyMultiTexSubImage3DEXT)(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void (APIENTRY* glBindMultiTextureEXT)(GLenum texunit, GLenum target, GLuint texture);
    void (APIENTRY* glEnableClientStateIndexedEXT)(GLenum array, GLuint index);
    void (APIENTRY* glDisableClientStateIndexedEXT)(GLenum array, GLuint index);
    void (APIENTRY* glMultiTexCoordPointerEXT)(GLenum texunit, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
    void (APIENTRY* glMultiTexEnvfEXT)(GLenum texunit, GLenum target, GLenum pname, GLfloat param);
    void (APIENTRY* glMultiTexEnvfvEXT)(GLenum texunit, GLenum target, GLenum pname, const GLfloat* param);
    void (APIENTRY* glMultiTexEnviEXT)(GLenum texunit, GLenum target, GLenum pname, GLint param);
    void (APIENTRY* glMultiTexEnvivEXT)(GLenum texunit, GLenum target, GLenum pname, const GLint* param);
    void (APIENTRY* glMultiTexGendEXT)(GLenum texunit, GLenum coord, GLenum pname, GLdouble param);
    void (APIENTRY* glMultiTexGendvEXT)(GLenum texunit, GLenum coord, GLenum pname, const GLdouble* param);
    void (APIENTRY* glMultiTexGenfEXT)(GLenum texunit, GLenum coord, GLenum pname, GLfloat param);
    void (APIENTRY* glMultiTexGenfvEXT)(GLenum texunit, GLenum coord, GLenum pname, const GLfloat* param);
    void (APIENTRY* glMultiTexGeniEXT)(GLenum texunit, GLenum coord, GLenum pname, GLint param);
    void (APIENTRY* glMultiTexGenivEXT)(GLenum texunit, GLenum coord, GLenum pname, const GLint* param);
    void (APIENTRY* glGetMultiTexEnvfvEXT)(GLenum texunit, GLenum coord, GLenum pname, GLfloat* param);
    void (APIENTRY* glGetMultiTexEnvivEXT)(GLenum texunit, GLenum coord, GLenum pname, GLint* param);
    void (APIENTRY* glGetMultiTexGendvEXT)(GLenum texunit, GLenum coord, GLenum pname, GLdouble* param);
    void (APIENTRY* glGetMultiTexGenfvEXT)(GLenum texunit, GLenum coord, GLenum pname, GLfloat* param);
    void (APIENTRY* glGetMultiTexGenivEXT)(GLenum texunit, GLenum coord, GLenum pname, GLint* param);
    void (APIENTRY* glGetFloatIndexedvEXT)(GLenum pname, GLuint index, GLfloat* params);
    void (APIENTRY* glGetDoubleIndexedvEXT)(GLenum pname, GLuint index, GLdouble* params);
    void (APIENTRY* glGetPointerIndexedvEXT)(GLenum pname, GLuint index, GLvoid** params);
    void (APIENTRY* glCompressedTextureImage3DEXT)(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imagesize, const GLvoid* data);
    void (APIENTRY* glCompressedTextureImage2DEXT)(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imagesize, const GLvoid* data);
    void (APIENTRY* glCompressedTextureImage1DEXT)(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imagesize, const GLvoid* data);
    void (APIENTRY* glCompressedTextureSubImage3DEXT)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
    void (APIENTRY* glCompressedTextureSubImage2DEXT)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imagesize, const GLvoid* data);
    void (APIENTRY* glCompressedTextureSubImage1DEXT)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imagesize, const GLvoid* data);
    void (APIENTRY* glGetCompressedTextureImageEXT)(GLuint texture, GLenum target, GLint level, GLvoid* img);
    void (APIENTRY* glCompressedMultiTexImage3DEXT)(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
    void (APIENTRY* glCompressedMultiTexImage2DEXT)(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data);
    void (APIENTRY* glCompressedMultiTexImage1DEXT)(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid* data);
    void (APIENTRY* glCompressedMultiTexSubImage3DEXT)(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
    void (APIENTRY* glCompressedMultiTexSubImage2DEXT)(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data);
    void (APIENTRY* glCompressedMultiTexSubImage1DEXT)(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid* data);
    void (APIENTRY* glGetCompressedMultiTexImageEXT)(GLenum texunit, GLenum target, GLint level, GLvoid* img);
    void (APIENTRY* glNamedProgramStringEXT)(GLuint program, GLenum target, GLenum format, GLsizei len, const GLvoid* string);
    void (APIENTRY* glNamedProgramLocalParameter4dEXT)(GLuint program, GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void (APIENTRY* glNamedProgramLocalParameter4dvEXT)(GLuint program, GLenum target, GLuint index, const GLdouble* params);
    void (APIENTRY* glNamedProgramLocalParameter4fEXT)(GLuint program, GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void (APIENTRY* glNamedProgramLocalParameter4fvEXT)(GLuint program, GLenum target, GLuint index, const GLfloat* params);
    void (APIENTRY* glGetNamedProgramLocalParameterdvEXT)(GLuint program, GLenum target, GLuint index, GLdouble* params);
    void (APIENTRY* glGetNamedProgramLocalParameterfvEXT)(GLuint program, GLenum target, GLuint index, GLfloat* params);
    void (APIENTRY* glGetNamedProgramivEXT)(GLuint program, GLenum target, GLenum pname, GLint* params);
    void (APIENTRY* glGetNamedProgramStringEXT)(GLuint program, GLenum target, GLenum pname, GLvoid* string);
    void (APIENTRY* glNamedProgramLocalParameters4fvEXT)(GLuint program, GLenum target, GLuint index, GLsizei count, const GLfloat* params);
    void (APIENTRY* glNamedProgramLocalParameterI4iEXT)(GLuint program, GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w);
    void (APIENTRY* glNamedProgramLocalParameterI4ivEXT)(GLuint program, GLenum target, GLuint index, const GLint* params);
    void (APIENTRY* glNamedProgramLocalParametersI4ivEXT)(GLuint program, GLenum target, GLuint index, GLsizei count, const GLint* params);
    void (APIENTRY* glNamedProgramLocalParameterI4uiEXT)(GLuint program, GLenum target, GLuint index, GLuint, GLuint, GLuint, GLuint);
    void (APIENTRY* glNamedProgramLocalParameterI4uivEXT)(GLuint program, GLenum target, GLuint index, const GLuint* params);
    void (APIENTRY* glNamedProgramLocalParametersI4uivEXT)(GLuint program, GLenum target, GLuint index, GLsizei count, const GLuint* params);
    void (APIENTRY* glGetNamedProgramLocalParameterIivEXT)(GLuint program, GLenum target, GLuint index, GLint* params);
    void (APIENTRY* glGetNamedProgramLocalParameterIuivEXT)(GLuint program, GLenum target, GLuint index, GLuint* params);
    void (APIENTRY* glTextureParameterIivEXT)(GLuint texture, GLenum target, GLenum pname, const GLint* params);
    void (APIENTRY* glTextureParameterIuivEXT)(GLuint texture, GLenum target, GLenum pname, const GLuint* params);
    void (APIENTRY* glGetTextureParameterIivEXT)(GLuint texture, GLenum target, GLenum pname, GLint* params);
    void (APIENTRY* glGetTextureParameterIuivEXT)(GLuint texture, GLenum target, GLenum pname, GLuint* params);
    void (APIENTRY* glMultiTexParameterIivEXT)(GLenum texunit, GLenum target, GLenum pname, const GLint* params);
    void (APIENTRY* glMultiTexParameterIuivEXT)(GLenum texunit, GLenum target, GLenum pname, const GLuint* params);
    void (APIENTRY* glGetMultiTexParameterIivEXT)(GLenum texunit, GLenum target, GLenum pname, GLint* params);
    void (APIENTRY* glGetMultiTexParameterIuivEXT)(GLenum texunit, GLenum target, GLenum pname, GLuint* params);
    void (APIENTRY* glProgramUniform1fEXT)(GLuint program, GLint location, GLfloat v0);
    void (APIENTRY* glProgramUniform2fEXT)(GLuint program, GLint location, GLfloat v0, GLfloat v1);
    void (APIENTRY* glProgramUniform3fEXT)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    void (APIENTRY* glProgramUniform4fEXT)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    void (APIENTRY* glProgramUniform1iEXT)(GLuint program, GLint location, GLint v0);
    void (APIENTRY* glProgramUniform2iEXT)(GLuint program, GLint location, GLint v0, GLint v1);
    void (APIENTRY* glProgramUniform3iEXT)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
    void (APIENTRY* glProgramUniform4iEXT)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
    void (APIENTRY* glProgramUniform1fvEXT)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
    void (APIENTRY* glProgramUniform2fvEXT)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
    void (APIENTRY* glProgramUniform3fvEXT)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
    void (APIENTRY* glProgramUniform4fvEXT)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
    void (APIENTRY* glProgramUniform1ivEXT)(GLuint program, GLint location, GLsizei count, const GLint* value);
    void (APIENTRY* glProgramUniform2ivEXT)(GLuint program, GLint location, GLsizei count, const GLint* value);
    void (APIENTRY* glProgramUniform3ivEXT)(GLuint program, GLint location, GLsizei count, const GLint* value);
    void (APIENTRY* glProgramUniform4ivEXT)(GLuint program, GLint location, GLsizei count, const GLint* value);
    void (APIENTRY* glProgramUniformMatrix2fvEXT)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glProgramUniformMatrix3fvEXT)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glProgramUniformMatrix4fvEXT)(GLuint program, GLint location, GLsizei counst, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glProgramUniformMatrix2x3fvEXT)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glProgramUniformMatrix3x2fvEXT)(GLuint program, GLint location, GLsizei counst, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glProgramUniformMatrix2x4fvEXT)(GLuint program, GLint location, GLsizei counst, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glProgramUniformMatrix4x2fvEXT)(GLuint program, GLint location, GLsizei counst, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glProgramUniformMatrix3x4fvEXT)(GLuint program, GLint location, GLsizei counst, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glProgramUniformMatrix4x3fvEXT)(GLuint program, GLint location, GLsizei counst, GLboolean transpose, const GLfloat* value);
    void (APIENTRY* glProgramUniform1uiEXT)(GLuint program, GLint location, GLuint v0);
    void (APIENTRY* glProgramUniform2uiEXT)(GLuint program, GLint location, GLuint v0, GLuint v1);
    void (APIENTRY* glProgramUniform3uiEXT)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
    void (APIENTRY* glProgramUniform4uiEXT)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
    void (APIENTRY* glProgramUniform1uivEXT)(GLuint program, GLint location, GLsizei count, const GLuint* value);
    void (APIENTRY* glProgramUniform2uivEXT)(GLuint program, GLint location, GLsizei count, const GLuint* value);
    void (APIENTRY* glProgramUniform3uivEXT)(GLuint program, GLint location, GLsizei count, const GLuint* value);
    void (APIENTRY* glProgramUniform4uivEXT)(GLuint program, GLint location, GLsizei count, const GLuint* value);
    void (APIENTRY* glNamedBufferDataEXT)(GLuint buffer, GLsizeiptr size, const GLvoid* data, GLenum usage);
    void (APIENTRY* glNamedBufferSubDataEXT)(GLuint buffer, GLintptr offset, GLsizeiptr size, const GLvoid* data);
    GLvoid* (APIENTRY* glMapNamedBufferEXT)(GLuint buffer, GLenum access);
    GLboolean(APIENTRY* glUnmapNamedBufferEXT)(GLuint buffer);
    GLvoid* (APIENTRY* glMapNamedBufferRangeEXT)(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access);
    void (APIENTRY* glFlushMappedNamedBufferRangeEXT)(GLuint buffer, GLintptr offset, GLsizeiptr length);
    void (APIENTRY* glNamedCopyBufferSubDataEXT)(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
    void (APIENTRY* glGetNamedBufferParameterivEXT)(GLuint buffer, GLenum pname, GLint* param);
    void (APIENTRY* glGetNamedBufferPointervEXT)(GLuint buffer, GLenum pname, GLvoid** param);
    void (APIENTRY* glGetNamedBufferSubDataEXT)(GLuint buffer, GLintptr offset, GLsizeiptr size, GLvoid* data);
    void (APIENTRY* glTextureBufferEXT)(GLuint texture, GLenum target, GLenum internalformat, GLuint buffer);
    void (APIENTRY* glMultiTexBufferEXT)(GLenum texunit, GLenum target, GLenum internalformat, GLuint buffer);
    void (APIENTRY* glNamedRenderbufferStorageEXT)(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
    void (APIENTRY* glGetNamedRenderbufferParameterivEXT)(GLuint renderbuffer, GLenum pname, GLint* param);
    GLenum(APIENTRY* glCheckNamedFramebufferStatusEXT)(GLuint framebuffer, GLenum target);
    void (APIENTRY* glNamedFramebufferTexture1DEXT)(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    void (APIENTRY* glNamedFramebufferTexture2DEXT)(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    void (APIENTRY* glNamedFramebufferTexture3DEXT)(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
    void (APIENTRY* glNamedFramebufferRenderbufferEXT)(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    void (APIENTRY* glGetNamedFramebufferAttachmentParameterivEXT)(GLuint framebuffer, GLenum attachment, GLenum pname, GLint* params);
    void (APIENTRY* glGenerateTextureMipmapEXT)(GLuint texture, GLenum target);
    void (APIENTRY* glGenerateMultiTexMipmapEXT)(GLenum texunit, GLenum target);
    void (APIENTRY* glFramebufferDrawBufferEXT)(GLuint framebuffer, GLenum mode);
    void (APIENTRY* glFramebufferDrawBuffersEXT)(GLuint framebuffer, GLsizei n, const GLenum* bufs);
    void (APIENTRY* glFramebufferReadBufferEXT)(GLuint framebuffer, GLenum mode);
    void (APIENTRY* glGetFramebufferParameterivEXT)(GLuint framebuffer, GLenum pname, GLint* param);

    void (APIENTRY* glNamedRenderbufferStorageMultisampleEXT)(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
    void (APIENTRY* glNamedRenderbufferStorageMultisampleCoverageEXT)(GLuint renderbuffer, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height);
    void (APIENTRY* glNamedFramebufferTextureEXT)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
    void (APIENTRY* glNamedFramebufferTextureLayerEXT)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
    void (APIENTRY* glNamedFramebufferTextureFaceEXT)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLenum face);
    void (APIENTRY* glTextureRenderbufferEXT)(GLuint texture, GLenum target, GLuint renderbuffer);
    void (APIENTRY* glMultiTexRenderbufferEXT)(GLenum texunit, GLenum target, GLuint renderbuffer);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_bindable_uniform
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glUniformBufferEXT)(GLuint program, GLint location, GLuint buffer);
    GLint(APIENTRY* glGetUniformBufferSizeEXT)(GLuint program, GLint location);
    GLintptr(APIENTRY* glGetUniformOffsetEXT)(GLuint program, GLint location);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_texture_integer
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glTexParameterIivEXT)(GLenum target, GLenum pname, const GLint* params);
    void (APIENTRY* glTexParameterIuivEXT)(GLenum target, GLenum pname, const GLuint* params);
    void (APIENTRY* glGetTexParameterIivEXT)(GLenum target, GLenum pname, GLint* params);
    void (APIENTRY* glGetTexParameterIuivEXT)(GLenum target, GLenum pname, GLuint* params);
    void (APIENTRY* glClearColorIiEXT)(GLint r, GLint g, GLint b, GLint a);
    void (APIENTRY* glClearColorIuiEXT)(GLuint r, GLuint g, GLuint b, GLuint a);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_multi_draw_arrays
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glMultiDrawArraysEXT)(GLenum mode, const GLint* first, const GLsizei* count, GLsizei primcount);
    void (APIENTRY* glMultiDrawElementsEXT)(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices, GLsizei primcount);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_multisample
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glSampleCoverageARB)(GLclampf value, GLboolean invert);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_blend_minmax
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glBlendEquationEXT)(GLenum mode);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_geometry_shader4
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glProgramParameteriEXT)(GLuint program, GLenum pname, GLint value);
    void (APIENTRY* glFramebufferTextureEXT)(GLenum target, GLenum attachment, GLuint texture, GLint level);
    void (APIENTRY* glFramebufferTextureLayerEXT)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
    void (APIENTRY* glFramebufferTextureFaceEXT)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face);

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_geometry_shader4
    //////////////////////////////////////////////////////////////////////////
    // No functions (see GL_EXT_geometry_shader4)

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_shader_buffer_load
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glMakeBufferResidentNV)(GLenum target, GLenum access);
    void (APIENTRY* glMakeBufferNonResidentNV)(GLenum target);
    GLboolean(APIENTRY* glIsBufferResidentNV)(GLenum target);

    // NOTICE: Sigal 19/7/10:
    // The following 2 functions have a different name in OpenGL.org spec and in NVIDIA spec
    // We support both versions.
    void (APIENTRY* glMakeNamedBufferResidentNV)(GLuint buffer, GLenum access);
    void (APIENTRY* glMakeNamedBufferNonResidentNV)(GLuint buffer);
    void (APIENTRY* glNamedMakeBufferResidentNV)(GLuint buffer, GLenum access);
    void (APIENTRY* glNamedMakeBufferNonResidentNV)(GLuint buffer);

    GLboolean(APIENTRY* glIsNamedBufferResidentNV)(GLuint buffer);

    void (APIENTRY* glGetBufferParameterui64vNV)(GLenum target, GLenum pname, GLuint64EXT* params);
    void (APIENTRY* glGetNamedBufferParameterui64vNV)(GLenum buffer, GLenum pname, GLuint64EXT* params);

    void (APIENTRY* glGetIntegerui64vNV)(GLenum value, GLuint64EXT* result);

    void (APIENTRY* glUniformui64NV)(GLint location, GLuint64EXT value);
    void (APIENTRY* glUniformui64vNV)(GLint location, GLsizei count, const GLuint64EXT* value);
    void (APIENTRY* glGetUniformui64vNV)(GLuint program, GLint location, GLuint64EXT* params);
    void (APIENTRY* glProgramUniformui64NV)(GLuint program, GLint location, GLuint64EXT value);
    void (APIENTRY* glProgramUniformui64vNV)(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value);

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_vertex_buffer_unified_memory
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glBufferAddressRangeNV)(GLenum pname, GLuint index, GLuint64EXT address, GLsizeiptr length);
    void (APIENTRY* glVertexFormatNV)(GLint size, GLenum type, GLsizei stride);
    void (APIENTRY* glNormalFormatNV)(GLenum type, GLsizei stride);
    void (APIENTRY* glColorFormatNV)(GLint size, GLenum type, GLsizei stride);
    void (APIENTRY* glIndexFormatNV)(GLenum type, GLsizei stride);
    void (APIENTRY* glTexCoordFormatNV)(GLint size, GLenum type, GLsizei stride);
    void (APIENTRY* glEdgeFlagFormatNV)(GLsizei stride);
    void (APIENTRY* glSecondaryColorFormatNV)(GLint size, GLenum type, GLsizei stride);
    void (APIENTRY* glFogCoordFormatNV)(GLenum type, GLsizei stride);
    void (APIENTRY* glVertexAttribFormatNV)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride);
    void (APIENTRY* glVertexAttribIFormatNV)(GLuint index, GLint size, GLenum type, GLsizei stride);
    void (APIENTRY* glGetIntegerui64i_vNV)(GLenum value, GLuint index, GLuint64EXT* result);

    //////////////////////////////////////////////////////////////////////////
    // GL_AMD_debug_output
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glDebugMessageEnableAMD)(GLenum category, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
    void (APIENTRY* glDebugMessageInsertAMD)(GLenum category, GLenum severity, GLuint id, GLsizei length, const GLchar* buf);
    void (APIENTRY* glDebugMessageCallbackAMD)(GLDEBUGPROCAMD callback, GLvoid* userParam);
    GLuint(APIENTRY* glGetDebugMessageLogAMD)(GLuint count, GLsizei bufsize, GLenum* categories, GLuint* severities, GLuint* ids, GLsizei* lengths, GLchar* message);

    //////////////////////////////////////////////////////////////////////////
    // GL_AMDX_debug_output
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glDebugMessageEnableAMDX)(GLenum category, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
    void (APIENTRY* glDebugMessageInsertAMDX)(GLenum category, GLenum severity, GLuint id, GLsizei length, const GLchar* buf);
    void (APIENTRY* glDebugMessageCallbackAMDX)(GLDEBUGPROCAMD callback, GLvoid* userParam);
    GLuint(APIENTRY* glGetDebugMessageLogAMDX)(GLuint count, GLsizei bufsize, GLenum* categories, GLuint* severities, GLuint* ids, GLsizei* lengths, GLchar* message);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_debug_output
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glDebugMessageControlARB)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
    void (APIENTRY* glDebugMessageInsertARB)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf);
    void (APIENTRY* glDebugMessageCallbackARB)(GLDEBUGPROCARB callback, const GLvoid* userParam);
    GLuint(APIENTRY* glGetDebugMessageLogARB)(GLuint count, GLsizei bufsize, GLenum* sources, GLenum* types, GLuint* ids, GLuint* severities, GLsizei* lengths, GLchar* messageLog);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_draw_instanced
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glDrawArraysInstancedEXT)(GLenum mode, GLint first, GLsizei count, GLsizei primcount);
    void (APIENTRY* glDrawElementsInstancedEXT)(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei primcount);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_draw_instanced
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glDrawArraysInstancedARB)(GLenum mode, GLint first, GLsizei count, GLsizei primcount);
    void (APIENTRY* glDrawElementsInstancedARB)(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei primcount);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_texture_buffer_object
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glTexBufferEXT)(GLenum target, GLenum internalformat, GLuint buffer);

    //////////////////////////////////////////////////////////////////////////
    // GL_EXT_compiled_vertex_array
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glLockArraysEXT)(GLint first, GLsizei count);
    void (APIENTRY* glUnlockArraysEXT)(void);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_transpose_matrix
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glLoadTransposeMatrixfARB)(const GLfloat* m);
    void (APIENTRY* glLoadTransposeMatrixdARB)(const GLdouble* m);
    void (APIENTRY* glMultTransposeMatrixfARB)(const GLfloat* m);
    void (APIENTRY* glMultTransposeMatrixdARB)(const GLdouble* m);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_point_parameters
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glPointParameterfARB)(GLenum pname, GLfloat param);
    void (APIENTRY* glPointParameterfvARB)(GLenum pname, const GLfloat* params);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_matrix_palette
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glCurrentPaletteMatrixARB)(GLint index);
    void (APIENTRY* glMatrixIndexubvARB)(GLint size, const GLubyte* indices);
    void (APIENTRY* glMatrixIndexusvARB)(GLint size, const GLushort* indices);
    void (APIENTRY* glMatrixIndexuivARB)(GLint size, const GLuint* indices);
    void (APIENTRY* glMatrixIndexPointerARB)(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_window_pos
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glWindowPos2dARB)(GLdouble x, GLdouble y);
    void (APIENTRY* glWindowPos2fARB)(GLfloat x, GLfloat y);
    void (APIENTRY* glWindowPos2iARB)(GLint x, GLint y);
    void (APIENTRY* glWindowPos2sARB)(GLshort x, GLshort y);
    void (APIENTRY* glWindowPos2dvARB)(const GLdouble* p);
    void (APIENTRY* glWindowPos2fvARB)(const GLfloat* p);
    void (APIENTRY* glWindowPos2ivARB)(const GLint* p);
    void (APIENTRY* glWindowPos2svARB)(const GLshort* p);
    void (APIENTRY* glWindowPos3dARB)(GLdouble x, GLdouble y, GLdouble z);
    void (APIENTRY* glWindowPos3fARB)(GLfloat x, GLfloat y, GLfloat z);
    void (APIENTRY* glWindowPos3iARB)(GLint x, GLint y, GLint z);
    void (APIENTRY* glWindowPos3sARB)(GLshort x, GLshort y, GLshort z);
    void (APIENTRY* glWindowPos3dvARB)(const GLdouble* p);
    void (APIENTRY* glWindowPos3fvARB)(const GLfloat* p);
    void (APIENTRY* glWindowPos3ivARB)(const GLint* p);
    void (APIENTRY* glWindowPos3svARB)(const GLshort* p);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_fragment_program_shadow
    //////////////////////////////////////////////////////////////////////////
    // No new functions

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_half_float_pixel
    //////////////////////////////////////////////////////////////////////////
    // No new functions

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_texture_float
    //////////////////////////////////////////////////////////////////////////
    // No new functions

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_pixel_buffer_object
    //////////////////////////////////////////////////////////////////////////
    // No new functions

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_depth_buffer_float
    //////////////////////////////////////////////////////////////////////////
    // No new functions

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_framebuffer_sRGB
    //////////////////////////////////////////////////////////////////////////
    // No new functions

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_geometry_shader4
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glProgramParameteriARB)(GLuint program, GLenum pname, GLint value);
    void (APIENTRY* glFramebufferTextureARB)(GLenum target, GLenum attachment, GLuint texture, GLint level);
    void (APIENTRY* glFramebufferTextureLayerARB)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
    void (APIENTRY* glFramebufferTextureFaceARB)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_half_float_vertex
    //////////////////////////////////////////////////////////////////////////
    // No new functions

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_instanced_arrays
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glVertexAttribDivisorARB)(GLuint index, GLuint divisor);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_map_buffer_range
    //////////////////////////////////////////////////////////////////////////
    GLvoid* (APIENTRY* glMapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
    void (APIENTRY* glFlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_texture_buffer_object
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glTexBufferARB)(GLenum target, GLenum internalformat, GLuint buffer);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_texture_compression_rgtc
    //////////////////////////////////////////////////////////////////////////
    // No new functions

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_texture_rg
    //////////////////////////////////////////////////////////////////////////
    // No new functions

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_vertex_array_object
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glBindVertexArray)(GLuint array);
    void (APIENTRY* glDeleteVertexArrays)(GLsizei n, const GLuint* arrays);
    void (APIENTRY* glGenVertexArrays)(GLsizei n, GLuint* arrays);
    GLboolean(APIENTRY* glIsVertexArray)(GLuint array);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_uniform_buffer_object
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glGetUniformIndices)(GLuint, GLsizei, const GLchar* const*, GLuint*);
    void (APIENTRY* glGetActiveUniformsiv)(GLuint, GLsizei, const GLuint*, GLenum, GLint*);
    void (APIENTRY* glGetActiveUniformName)(GLuint, GLuint, GLsizei, GLsizei*, GLchar*);
    GLuint(APIENTRY* glGetUniformBlockIndex)(GLuint, const GLchar*);
    void (APIENTRY* glGetActiveUniformBlockiv)(GLuint, GLuint, GLenum, GLint*);
    void (APIENTRY* glGetActiveUniformBlockName)(GLuint, GLuint, GLsizei, GLsizei*, GLchar*);
    void (APIENTRY* glUniformBlockBinding)(GLuint, GLuint, GLuint);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_copy_buffer
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glCopyBufferSubData)(GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_draw_elements_base_vertex
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glDrawElementsBaseVertex)(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLint basevertex);
    void (APIENTRY* glDrawRangeElementsBaseVertex)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices, GLint basevertex);
    void (APIENTRY* glDrawElementsInstancedBaseVertex)(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei primcount, GLint basevertex);
    void (APIENTRY* glMultiDrawElementsBaseVertex)(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices, GLsizei primcount, const GLint* basevertex);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_provoking_vertex
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glProvokingVertex)(GLenum mode);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_sync
    //////////////////////////////////////////////////////////////////////////
    GLsync(APIENTRY* glFenceSync)(GLenum condition, GLbitfield flags);
    GLboolean(APIENTRY* glIsSync)(GLsync sync);
    void (APIENTRY* glDeleteSync)(GLsync sync);
    GLenum(APIENTRY* glClientWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout);
    void (APIENTRY* glWaitSync)(GLsync sync, GLbitfield flag, GLuint64 timeout);
    void (APIENTRY* glGetInteger64v)(GLenum pname, GLint64* params);
    void (APIENTRY* glGetSynciv)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_texture_multisample
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glTexImage2DMultisample)(GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
    void (APIENTRY* glTexImage3DMultisample)(GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
    void (APIENTRY* glGetMultisamplefv)(GLenum pname, GLuint index, GLfloat* val);
    void (APIENTRY* glSampleMaski)(GLuint index, GLbitfield mask);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_cl_event
    //////////////////////////////////////////////////////////////////////////
    GLsync(APIENTRY* glCreateSyncFromCLeventARB)(struct _cl_context* context, struct _cl_event* event, GLbitfield flags);

    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_aux_depth_stencil
    //////////////////////////////////////////////////////////////////////////
    // No new functions

    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_client_storage
    //////////////////////////////////////////////////////////////////////////
    // No new functions


    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_element_array
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glElementPointerAPPLE)(GLenum type, const GLvoid* pointer);
    void (APIENTRY* glDrawElementArrayAPPLE)(GLenum mode, GLint first, GLsizei count);
    void (APIENTRY* glDrawRangeElementArrayAPPLE)(GLenum mode, GLuint start, GLuint end, GLint first, GLsizei count);
    void (APIENTRY* glMultiDrawElementArrayAPPLE)(GLenum mode, const GLint* first, const GLsizei* count, GLsizei primcount);
    void (APIENTRY* glMultiDrawRangeElementArrayAPPLE)(GLenum mode, GLuint start, GLuint end, const GLint* first, const GLsizei* count, GLsizei primcount);

    //////////////////////////////////////////////////////////////////////////
    // AP_GL_APPLE_fence
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glGenFencesAPPLE)(GLsizei n, GLuint* fences);
    void (APIENTRY* glDeleteFencesAPPLE)(GLsizei n, const GLuint* fences);
    void (APIENTRY* glSetFenceAPPLE)(GLuint fence);
    GLboolean(APIENTRY* glIsFenceAPPLE)(GLuint fence);
    GLboolean(APIENTRY* glTestFenceAPPLE)(GLuint fence);
    void (APIENTRY* glFinishFenceAPPLE)(GLuint fence);
    GLboolean(APIENTRY* glTestObjectAPPLE)(GLenum object, GLuint name);
    void (APIENTRY* glFinishObjectAPPLE)(GLenum object, GLint name);

    //////////////////////////////////////////////////////////////////////////
    // AP_GL_APPLE_float_pixels
    //////////////////////////////////////////////////////////////////////////
    // No new functions

    //////////////////////////////////////////////////////////////////////////
    // AP_GL_APPLE_flush_buffer_range
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glBufferParameteriAPPLE)(GLenum target, GLenum pname, GLint param);
    void (APIENTRY* glFlushMappedBufferRangeAPPLE)(GLenum target, GLintptr offset, GLsizeiptr size);

    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_flush_render
    // No new functions
    //////////////////////////////////////////////////////////////////////////


    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_object_purgeable
    //////////////////////////////////////////////////////////////////////////
    GLenum(APIENTRY* glObjectPurgeableAPPLE)(GLenum objectType, GLuint name, GLenum option);
    GLenum(APIENTRY* glObjectUnpurgeableAPPLE)(GLenum objectType, GLuint name, GLenum option);
    void (APIENTRY* glGetObjectParameterivAPPLE)(GLenum objectType, GLuint name, GLenum pname, GLint* params);

    //////////////////////////////////////////////////////////////////////////
    // AP_GL_APPLE_packed_pixels
    // No new functions
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // AP_GL_APPLE_pixel_buffer
    // No new functions
    //TO_DO: search for this functions (spec is unavailable on the net)
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // AP_GL_APPLE_specular_vector
    // No new functions
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_texture_range
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glTextureRangeAPPLE)(GLenum target, GLsizei length, const GLvoid* pointer);
    void (APIENTRY* glGetTexParameterPointervAPPLE)(GLenum target, GLenum pname, GLvoid** params);


    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_transform_hint
    // No new functions
    //////////////////////////////////////////////////////////////////////////


    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_vertex_array_object
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glBindVertexArrayAPPLE)(GLuint array);
    void (APIENTRY* glDeleteVertexArraysAPPLE)(GLsizei n, const GLuint* arrays);
    void (APIENTRY* glGenVertexArraysAPPLE)(GLsizei n, const GLuint* arrays);
    GLboolean(APIENTRY* glIsVertexArrayAPPLE)(GLuint array);

    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_vertex_array_range
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glVertexArrayRangeAPPLE)(GLsizei length, GLvoid* pointer);
    void (APIENTRY* glFlushVertexArrayRangeAPPLE)(GLsizei length, GLvoid* pointer);
    void (APIENTRY* glVertexArrayParameteriAPPLE)(GLenum pname, GLint param);

    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_vertex_program_evaluators
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glEnableVertexAttribAPPLE)(GLuint index, GLenum pname);
    void (APIENTRY* glDisableVertexAttribAPPLE)(GLuint index, GLenum pname);
    GLboolean(APIENTRY* glIsVertexAttribEnabledAPPLE)(GLuint index, GLenum pname);
    void (APIENTRY* glMapVertexAttrib1dAPPLE)(GLuint index, GLuint size, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble* points);
    void (APIENTRY* glMapVertexAttrib1fAPPLE)(GLuint index, GLuint size, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat* points);
    void (APIENTRY* glMapVertexAttrib2dAPPLE)(GLuint index, GLuint size, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble* points);
    void (APIENTRY* glMapVertexAttrib2fAPPLE)(GLuint index, GLuint size, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat* points);

    //////////////////////////////////////////////////////////////////////////
    // GL_APPLE_ycbcr_422
    // No new functions
    //////////////////////////////////////////////////////////////////////////

    // Add new extensions here:

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    // WGL extensions functions:
    const char* (WINAPI* wglGetExtensionsStringARB)(HDC hdc);

    //////////////////////////////////////////////////////////////////////////
    // WGL_EXT_extensions_string Extension
    //////////////////////////////////////////////////////////////////////////
    const char* wglGetExtensionsStringEXT(void);

    //////////////////////////////////////////////////////////////////////////
    // WGL_I3D_genlock Extension
    //////////////////////////////////////////////////////////////////////////
    BOOL (WINAPI* wglEnableGenlockI3D)(HDC hDC);
    BOOL (WINAPI* wglDisableGenlockI3D)(HDC hDC);
    BOOL (WINAPI* wglIsEnabledGenlockI3D)(HDC hDC, BOOL* pFlag);
    BOOL (WINAPI* wglGenlockSourceI3D)(HDC hDC, UINT uSource);
    BOOL (WINAPI* wglGetGenlockSourceI3D)(HDC hDC, UINT* uSource);
    BOOL (WINAPI* wglGenlockSourceEdgeI3D)(HDC hDC, UINT uEdge);
    BOOL (WINAPI* wglGetGenlockSourceEdgeI3D)(HDC hDC, UINT* uEdge);
    BOOL (WINAPI* wglGenlockSampleRateI3D)(HDC hDC, UINT uRate);
    BOOL (WINAPI* wglGetGenlockSampleRateI3D)(HDC hDC, UINT* uRate);
    BOOL (WINAPI* wglGenlockSourceDelayI3D)(HDC hDC, UINT uDelay);
    BOOL (WINAPI* wglGetGenlockSourceDelayI3D)(HDC hDC, UINT* uDelay);
    BOOL (WINAPI* wglQueryGenlockMaxSourceDelayI3D)(HDC hDC, UINT* uMaxLineDelay, UINT* uMaxPixelDelay);

    //////////////////////////////////////////////////////////////////////////
    // WGL_ARB_pbuffer
    //////////////////////////////////////////////////////////////////////////
    HPBUFFERARB(WINAPI* wglCreatePbufferARB)(HDC hDC, int iPixelFormat, int iWidth, int iHeight, const int* piAttribList);
    HDC(WINAPI* wglGetPbufferDCARB)(HPBUFFERARB hPbuffer);
    int (WINAPI* wglReleasePbufferDCARB)(HPBUFFERARB hPbuffer, HDC hDC);
    BOOL (WINAPI* wglDestroyPbufferARB)(HPBUFFERARB hPbuffer);
    BOOL (WINAPI* wglQueryPbufferARB)(HPBUFFERARB hPbuffer, int iAttribute, int* piValue);


    //////////////////////////////////////////////////////////////////////////
    // WGL_ARB_pixel_format
    //////////////////////////////////////////////////////////////////////////
    BOOL (WINAPI* wglGetPixelFormatAttribivARB)(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int* piAttributes, int* piValues);
    BOOL (WINAPI* wglGetPixelFormatAttribfvARB)(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int* piAttributes, FLOAT* pfValues);
    BOOL (WINAPI* wglChoosePixelFormatARB)(HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats);

    //////////////////////////////////////////////////////////////////////////
    // WGL_ARB_make_current_read
    //////////////////////////////////////////////////////////////////////////
    BOOL (WINAPI* wglMakeContextCurrentARB)(HDC hDrawDC, HDC hReadDC, HGLRC hglrc);
    HDC(WINAPI* wglGetCurrentReadDCARB)(void);

    //////////////////////////////////////////////////////////////////////////
    // WGL_ARB_render_texture
    //////////////////////////////////////////////////////////////////////////
    BOOL (WINAPI* wglBindTexImageARB)(HPBUFFERARB hPbuffer, int iBuffer);
    BOOL (WINAPI* wglReleaseTexImageARB)(HPBUFFERARB hPbuffer, int iBuffer);
    BOOL (WINAPI* wglSetPbufferAttribARB)(HPBUFFERARB hPbuffer, const int* piAttribList);

    //////////////////////////////////////////////////////////////////////////
    // WGL_ARB_buffer_region
    //////////////////////////////////////////////////////////////////////////
    HANDLE(WINAPI* wglCreateBufferRegionARB)(HDC hDC, int iLayerPlane, UINT uType);
    VOID (WINAPI* wglDeleteBufferRegionARB)(HANDLE hRegion);
    BOOL (WINAPI* wglSaveBufferRegionARB)(HANDLE hRegion, int x, int y, int width, int height);
    BOOL (WINAPI* wglRestoreBufferRegionARB)(HANDLE hRegion, int x, int y, int width, int height, int xSrc, int ySrc);

    //////////////////////////////////////////////////////////////////////////
    // WGL_ARB_multisample
    //////////////////////////////////////////////////////////////////////////
    // See GL_ARB_multisample

    //////////////////////////////////////////////////////////////////////////
    // WGL_ARB_pixel_format_float
    //////////////////////////////////////////////////////////////////////////
    // See GL_ARB_color_buffer_float

    //////////////////////////////////////////////////////////////////////////
    // WGL_ARB_framebuffer_sRGB
    //////////////////////////////////////////////////////////////////////////
    // No new functions

    //////////////////////////////////////////////////////////////////////////
    // WGL_ARB_create_context
    //////////////////////////////////////////////////////////////////////////
    HGLRC(WINAPI* wglCreateContextAttribsARB)(HDC hDC, HGLRC hShareContext, const int* attribList);

    //////////////////////////////////////////////////////////////////////////
    // WGL_EXT_swap_control
    //////////////////////////////////////////////////////////////////////////
    BOOL (WINAPI* wglSwapIntervalEXT)(int);
    int (WINAPI* wglGetSwapIntervalEXT)(void);

    //////////////////////////////////////////////////////////////////////////
    // WGL_NV_present_video
    //////////////////////////////////////////////////////////////////////////
    int (WINAPI* wglEnumerateVideoDevicesNV)(HDC, HVIDEOOUTPUTDEVICENV*);
    BOOL (WINAPI* wglBindVideoDeviceNV)(HDC, unsigned int, HVIDEOOUTPUTDEVICENV, const int*);
    BOOL (WINAPI* wglQueryCurrentContextNV)(int, int*);

    //////////////////////////////////////////////////////////////////////////
    // WGL_NV_video_out
    //////////////////////////////////////////////////////////////////////////
    BOOL (WINAPI* wglGetVideoDeviceNV)(HDC, int, HPVIDEODEV*);
    BOOL (WINAPI* wglReleaseVideoDeviceNV)(HPVIDEODEV);
    BOOL (WINAPI* wglBindVideoImageNV)(HPVIDEODEV, HPBUFFERARB, int);
    BOOL (WINAPI* wglReleaseVideoImageNV)(HPBUFFERARB, int);
    BOOL (WINAPI* wglSendPbufferToVideoNV)(HPBUFFERARB, int, unsigned long*, BOOL);
    BOOL (WINAPI* wglGetVideoInfoNV)(HPVIDEODEV, unsigned long*, unsigned long*);

    //////////////////////////////////////////////////////////////////////////
    // WGL_NV_swap_group
    //////////////////////////////////////////////////////////////////////////
    BOOL (WINAPI* wglJoinSwapGroupNV)(HDC, GLuint);
    BOOL (WINAPI* wglBindSwapBarrierNV)(GLuint, GLuint);
    BOOL (WINAPI* wglQuerySwapGroupNV)(HDC, GLuint*, GLuint*);
    BOOL (WINAPI* wglQueryMaxSwapGroupsNV)(HDC, GLuint*, GLuint*);
    BOOL (WINAPI* wglQueryFrameCountNV)(HDC, GLuint*);
    BOOL (WINAPI* wglResetFrameCountNV)(HDC);

    //////////////////////////////////////////////////////////////////////////
    // WGL_NV_gpu_affinity
    //////////////////////////////////////////////////////////////////////////
    BOOL (WINAPI* wglEnumGpusNV)(UINT, HGPUNV*);
    BOOL (WINAPI* wglEnumGpuDevicesNV)(HGPUNV, UINT, PGPU_DEVICE);
    HDC(WINAPI* wglCreateAffinityDCNV)(const HGPUNV*);
    BOOL (WINAPI* wglEnumGpusFromAffinityDCNV)(HDC, UINT, HGPUNV*);
    BOOL (WINAPI* wglDeleteDCNV)(HDC);

    //////////////////////////////////////////////////////////////////////////
    // WGL_AMD_gpu_association
    //////////////////////////////////////////////////////////////////////////
    UINT(WINAPI* wglGetGPUIDsAMD)(UINT, UINT*);
    INT (WINAPI* wglGetGPUInfoAMD)(UINT, int, GLenum, UINT, void*);
    UINT(WINAPI* wglGetContextGPUIDAMD)(HGLRC);
    HGLRC(WINAPI* wglCreateAssociatedContextAMD)(UINT);
    HGLRC(WINAPI* wglCreateAssociatedContextAttribsAMD)(UINT, HGLRC, const int*);
    BOOL (WINAPI* wglDeleteAssociatedContextAMD)(HGLRC);
    BOOL (WINAPI* wglMakeAssociatedContextCurrentAMD)(HGLRC);
    HGLRC(WINAPI* wglGetCurrentAssociatedContextAMD)(void);
    VOID (WINAPI* wglBlitContextFramebufferAMD)(HGLRC, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))

    // GLX Extensions functions:

    //////////////////////////////////////////////////////////////////////////
    // GLX_SGIX_fbconfig
    //////////////////////////////////////////////////////////////////////////
    int (*glXGetFBConfigAttribSGIX)(Display* dpy, GLXFBConfigSGIX config, int attribute, int* value);
    GLXFBConfigSGIX* (*glXChooseFBConfigSGIX)(Display* dpy, int screen, const int* attrib_list, int* nelements);
    GLXPixmap(*glXCreateGLXPixmapWithConfigSGIX)(Display* dpy, GLXFBConfig config, Pixmap pixmap);
    GLXContext(*glXCreateContextWithConfigSGIX)(Display* dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct);
    XVisualInfo* (*glXGetVisualFromFBConfigSGIX)(Display* dpy, GLXFBConfig config);
    GLXFBConfigSGIX(*glXGetFBConfigFromVisualSGIX)(Display* dpy, XVisualInfo* vis);

    //////////////////////////////////////////////////////////////////////////
    // GLX_SGI_video_sync
    //////////////////////////////////////////////////////////////////////////
    int (*glXGetVideoSyncSGI)(unsigned int* count);
    int (*glXWaitVideoSyncSGI)(int divisor, int remainder, unsigned int* count);

    //////////////////////////////////////////////////////////////////////////
    // GLX_ARB_get_proc_address
    //////////////////////////////////////////////////////////////////////////
    // See gsGLXWrappers.cpp

    //////////////////////////////////////////////////////////////////////////
    // GLX_ARB_multisample
    //////////////////////////////////////////////////////////////////////////
    // See GL_ARB_multisample

    //////////////////////////////////////////////////////////////////////////
    // GLX_ARB_fbconfig_float
    //////////////////////////////////////////////////////////////////////////
    // See GL_ARB_color_buffer_float

    //////////////////////////////////////////////////////////////////////////
    // GLX_ARB_framebuffer_sRGB
    //////////////////////////////////////////////////////////////////////////
    // See GL_ARB_framebuffer_sRGB

    //////////////////////////////////////////////////////////////////////////
    // GLX_SGIX_pbuffer
    //////////////////////////////////////////////////////////////////////////
    GLXPbufferSGIX(*glXCreateGLXPbufferSGIX)(Display* dpy, GLXFBConfigSGIX config, unsigned int width, unsigned int height, int* attrib_list);
    void (*glXDestroyGLXPbufferSGIX)(Display* dpy, GLXPbufferSGIX pbuf);
    int (*glXQueryGLXPbufferSGIX)(Display* dpy, GLXPbufferSGIX pbuf, int attribute, unsigned int* value);
    void (*glXSelectEventSGIX)(Display* dpy, GLXDrawable drawable, unsigned long mask);
    void (*glXGetSelectedEventSGIX)(Display* dpy, GLXDrawable drawable, unsigned long* mask);


#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    //////////////////////////////////////////////////////////////////////////
    // Uri, 26/5/09: we need to skip over two functions so that this file
    // matches apMonitoredFunctionId.h, so we add dummy pointers:
    //////////////////////////////////////////////////////////////////////////
    void (*dummy_glStringMarkerGREMEDY)();
    void (*dummy_glFrameTerminatorGREMEDY)();


    // OpenGL ES Core functions:
    // We do not need to export the OpenGL ES function pointers - we emulate them into the OpenGL core functions...
    void (*glAlphaFuncx)(GLenum func, GLclampx ref);
    void (*glClearColorx)(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha);
    // void (*glClearDepthf) (GLclampf depth); // Appears in OpenGL 4.1
    void (*glClearDepthx)(GLclampx depth);
    void (*glClipPlanef)(GLenum plane, const GLfloat* equation);
    void (*glClipPlanex)(GLenum plane, const GLfixed* equation);
    void (*glColor4x)(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
    // void (*glDepthRangef) (GLclampf zNear, GLclampf zFar); // Appears in OpenGL 4.1
    void (*glDepthRangex)(GLclampx zNear, GLclampx zFar);
    void (*glFogx)(GLenum pname, GLfixed param);
    void (*glFogxv)(GLenum pname, const GLfixed* params);
    void (*glFrustumf)(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
    void (*glFrustumx)(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
    void (*glGetClipPlanef)(GLenum pname, GLfloat eqn[4]);
    void (*glGetClipPlanex)(GLenum pname, GLfixed eqn[4]);
    void (*glGetFixedv)(GLenum pname, GLfixed* params);
    void (*glGetLightxv)(GLenum light, GLenum pname, GLfixed* params);
    void (*glGetMaterialxv)(GLenum face, GLenum pname, GLfixed* params);
    void (*glGetTexEnvxv)(GLenum env, GLenum pname, GLfixed* params);
    void (*glGetTexParameterxv)(GLenum target, GLenum pname, GLfixed* params);
    void (*glLightModelx)(GLenum pname, GLfixed param);
    void (*glLightModelxv)(GLenum pname, const GLfixed* params);
    void (*glLightx)(GLenum light, GLenum pname, GLfixed param);
    void (*glLightxv)(GLenum light, GLenum pname, const GLfixed* params);
    void (*glLineWidthx)(GLfixed width);
    void (*glLoadMatrixx)(const GLfixed* m);
    void (*glMaterialx)(GLenum face, GLenum pname, GLfixed param);
    void (*glMaterialxv)(GLenum face, GLenum pname, const GLfixed* params);
    void (*glMultMatrixx)(const GLfixed* m);
    void (*glMultiTexCoord4x)(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q);
    void (*glNormal3x)(GLfixed nx, GLfixed ny, GLfixed nz);
    void (*glOrthof)(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
    void (*glOrthox)(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
    void (*glPointParameterx)(GLenum pname, GLfixed param);
    void (*glPointParameterxv)(GLenum pname, const GLfixed* params);
    void (*glPointSizex)(GLfixed size);
    void (*glPolygonOffsetx)(GLfixed factor, GLfixed units);
    void (*glRotatex)(GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
    void (*glSampleCoveragex)(GLclampx value, GLboolean invert);
    void (*glScalex)(GLfixed x, GLfixed y, GLfixed z);
    void (*glTexEnvx)(GLenum target, GLenum pname, GLfixed param);
    void (*glTexEnvxv)(GLenum target, GLenum pname, const GLfixed* params);
    void (*glTexParameterx)(GLenum target, GLenum pname, GLfixed param);
    void (*glTexParameterxv)(GLenum target, GLenum pname, const GLfixed* params);
    void (*glTranslatex)(GLfixed x, GLfixed y, GLfixed z);

    //////////////////////////////////////////////////////////////////////////
    // OES_draw_texture
    //////////////////////////////////////////////////////////////////////////
    void (*glDrawTexsOES)(GLshort x, GLshort y, GLshort z, GLshort width, GLshort height);
    void (*glDrawTexiOES)(GLint x, GLint y, GLint z, GLint width, GLint height);
    void (*glDrawTexxOES)(GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height);
    void (*glDrawTexsvOES)(const GLshort* coords);
    void (*glDrawTexivOES)(const GLint* coords);
    void (*glDrawTexxvOES)(const GLfixed* coords);
    void (*glDrawTexfOES)(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height);
    void (*glDrawTexfvOES)(const GLfloat* coords);

    //////////////////////////////////////////////////////////////////////////
    // OES_blend_subtract
    //////////////////////////////////////////////////////////////////////////
    void (*glBlendEquationOES)(GLenum mode);

    //////////////////////////////////////////////////////////////////////////
    // OES_framebuffer_object
    //////////////////////////////////////////////////////////////////////////
    GLboolean(*glIsRenderbufferOES)(GLuint renderbuffer);
    void (*glBindRenderbufferOES)(GLenum target, GLuint renderbuffer);
    void (*glDeleteRenderbuffersOES)(GLsizei n, const GLuint* renderbuffers);
    void (*glGenRenderbuffersOES)(GLsizei n, GLuint* renderbuffers);
    void (*glRenderbufferStorageOES)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
    void (*glGetRenderbufferParameterivOES)(GLenum target, GLenum pname, GLint* params);
    GLboolean(*glIsFramebufferOES)(GLuint framebuffer);
    void (*glBindFramebufferOES)(GLenum target, GLuint framebuffer);
    void (*glDeleteFramebuffersOES)(GLsizei n, const GLuint* framebuffers);
    void (*glGenFramebuffersOES)(GLsizei n, GLuint* framebuffers);
    GLenum(*glCheckFramebufferStatusOES)(GLenum target);
    void (*glFramebufferRenderbufferOES)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    void (*glFramebufferTexture2DOES)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    void (*glGetFramebufferAttachmentParameterivOES)(GLenum target, GLenum attachment, GLenum pname, GLint* params);
    void (*glGenerateMipmapOES)(GLenum target);

    //////////////////////////////////////////////////////////////////////////
    // OES_mapbuffer
    //////////////////////////////////////////////////////////////////////////
    void (*glGetBufferPointervOES)(GLenum target, GLenum pname, GLvoid** params);
    GLvoid* (*glMapBufferOES)(GLenum target, GLenum access);
    GLboolean(*glUnmapBufferOES)(GLenum target);

    //////////////////////////////////////////////////////////////////////////
    // OES_matrix_palette
    //////////////////////////////////////////////////////////////////////////
    void (*glCurrentPaletteMatrixOES)(GLuint matrixpaletteindex);
    void (*glLoadPaletteFromModelViewMatrixOES)(void);
    void (*glMatrixIndexPointerOES)(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
    void (*glWeightPointerOES)(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);

    //////////////////////////////////////////////////////////////////////////
    // OES_point_size_array
    //////////////////////////////////////////////////////////////////////////
    void (*glPointSizePointerOES)(GLenum type, GLsizei stride, const GLvoid* pointer);

    //////////////////////////////////////////////////////////////////////////
    // OpenGL ES 2.0
    //////////////////////////////////////////////////////////////////////////
    void (*glGetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);
    void (*glReleaseShaderCompiler)(void);
    void (*glShaderBinary)(GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length);

    //////////////////////////////////////////////////////////////////////////
    // EAGL Obj-C functions
    //////////////////////////////////////////////////////////////////////////
#ifdef _GR_IPHONE_BUILD
    id(*EAGLContext_initWithAPI)(id self, SEL _cmd, EAGLRenderingAPI api);
    id(*EAGLContext_initWithAPI_sharegroup)(id self, SEL _cmd, EAGLRenderingAPI api, EAGLSharegroup* sharegroup);
    void (*EAGLContext_dealloc)(id self, SEL _cmd);
    BOOL (*EAGLContext_setCurrentContext)(id self, SEL _cmd, EAGLContext* context);
    EAGLContext* (*EAGLContext_currentContext)(id self, SEL _cmd);
    EAGLRenderingAPI(*EAGLContext_API)(id self, SEL _cmd);
    EAGLSharegroup* (*EAGLContext_sharegroup)(id self, SEL _cmd);
    BOOL (*EAGLContext_renderbufferStorage_fromDrawable)(id self, SEL _cmd, NSUInteger target, id /*<EAGLDrawable>*/ drawable);
    BOOL (*EAGLContext_presentRenderbuffer)(id self, SEL _cmd, NSUInteger target);
#else
    // these are dummy pointers so they will compile on the CGL spy as well
    void (*EAGLContext_initWithAPI)();
    void (*EAGLContext_initWithAPI_sharegroup)();
    void (*EAGLContext_dealloc)();
    void (*EAGLContext_setCurrentContext)();
    void (*EAGLContext_currentContext)();
    void (*EAGLContext_API)();
    void (*EAGLContext_sharegroup)();
    void (*EAGLContext_renderbufferStorage_fromDrawable)();
    void (*EAGLContext_presentRenderbuffer)();
#endif
    //////////////////////////////////////////////////////////////////////////
    // EAGL C functions:
    //////////////////////////////////////////////////////////////////////////
    void (*EAGLGetVersion)(unsigned int* major, unsigned int* minor);
#endif


    //////////////////////////////////////////////////////////////////////////
    // CodeXL Extensions
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // GL_GREMEDY_string_marker Extension
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glStringMarkerGREMEDY)(GLsizei len, const GLvoid* string);

    //////////////////////////////////////////////////////////////////////////
    // GL_GREMEDY_frame_terminator Extension
    //////////////////////////////////////////////////////////////////////////
    void (APIENTRY* glFrameTerminatorGREMEDY)(void);
};

// ----------------------------------------------------------------------------------
// Struct Name:          gsDriverInternalFunctionPointers
//
// General Description:
//   Contains pointers to the real implementation of the wrapped driver-internal functions.
//
// Author:               Uri Shomroni
// Creation Date:        29/6/2016
// ----------------------------------------------------------------------------------
struct gsDriverInternalFunctionPointers
{
    // AMD Driver internal functions:
    unsigned int (APIENTRY* _loader_get_dispatch_table_size)(void);
    int (APIENTRY* _loader_get_proc_offset)(const char* name);
    int (APIENTRY* _loader_add_dispatch)(const char* const* names, const char* signature);
    void (APIENTRY* _loader_set_dispatch)(const void* dispTable);

    void (*_glapi_noop_enable_warnings)(GLboolean enable);
    void (*_glapi_set_warning_func)(_glapi_warning_func func);
    void (*_glapi_check_multithread)(void);
    void (*_glapi_set_context)(void* context);
    void* (*_glapi_get_context)(void);
    void (*_glapi_set_dispatch)(_glapi_table* dispatch);
    _glapi_table* (*_glapi_get_dispatch)(void);
    int (*_glapi_begin_dispatch_override)(struct _glapi_table* override);
    void (*_glapi_end_dispatch_override)(int layer);
    _glapi_table* (*_glapi_get_override_dispatch)(int layer);
    GLuint (*_glapi_get_dispatch_table_size)(void);
    void (*_glapi_check_table)(const _glapi_table* table);
    int (*_glapi_add_dispatch)(const char* const* function_names, const char* parameter_signature);
    GLint (*_glapi_get_proc_offset)(const char* funcName);
    _glapi_proc (*_glapi_get_proc_address)(const char* funcName);
    const char* (*_glapi_get_proc_name)(GLuint offset);
};


#endif  // __GSMONITOREDFUNCTIONPOINTERS
