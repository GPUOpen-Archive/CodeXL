//------------------------------ oaPlatformSpecificFunctionPointers.h ------------------------------

#ifndef __OAPLATFORMSPECIFICFUNCTIONPOINTERS_H
#define __OAPLATFORMSPECIFICFUNCTIONPOINTERS_H

// If we are NOT on the iPhone:
#ifndef _GR_IPHONE_BUILD

    // Mac OS X:
    #include <OpenGL/OpenGL.h>

    // Local:
    #include <AMDTOSWrappers/Include/osOSDefinitions.h>
    #include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

    // CGL function types:
    typedef CGLError(*PFNCGLDESCRIBEPIXELFORMAT)(CGLPixelFormatObj pix, GLint pix_num, CGLPixelFormatAttribute attrib, GLint* value);
    typedef CGLError(*PFNCGLCHOOSEPIXELFORMAT)(const CGLPixelFormatAttribute* attribs, CGLPixelFormatObj* pix, GLint* npix);
    typedef CGLError(*PFNCGLDESTROYPIXELFORMAT)(CGLPixelFormatObj pix);
    typedef CGLError(*PFNCGLCREATECONTEXT)(CGLPixelFormatObj pix, CGLContextObj share, CGLContextObj* ctx);
    typedef CGLError(*PFNCGLDESTROYCONTEXT)(CGLContextObj ctx);
    typedef CGLError(*PFNCGLSETCURRENTCONTEXT)(CGLContextObj ctx);
    typedef CGLError(*PFNCGLSETOFFSCREEN)(CGLContextObj ctx, GLsizei width, GLsizei height, GLint rowbytes, void* baseaddr);
    typedef CGLError(*PFNCGLGETOFFSCREEN)(CGLContextObj ctx, GLsizei* width, GLsizei* height, GLint* rowbytes, void** baseaddr);
    typedef CGLError(*PFNCGLCLEARDRAWABLE)(CGLContextObj ctx);

    // CGL function pointers:
    extern PFNCGLDESCRIBEPIXELFORMAT pOACGLDescribePixelFormat;
    extern PFNCGLCHOOSEPIXELFORMAT pOACGLChoosePixelFormat;
    extern PFNCGLDESTROYPIXELFORMAT pOACGLDestroyPixelFormat;
    extern PFNCGLCREATECONTEXT pOACGLCreateContext;
    extern PFNCGLDESTROYCONTEXT pOACGLDestroyContext;
    extern PFNCGLSETCURRENTCONTEXT pOACGLSetCurrentContext;
    extern PFNCGLSETOFFSCREEN pOACGLSetOffScreen;
    extern PFNCGLGETOFFSCREEN pOACGLGetOffScreen;
    extern PFNCGLCLEARDRAWABLE pOACGLClearDrawable;

    // Aid functions:
    bool oaLoadCGLFunctionPointers();

#else
    // TO_DO iPhone: EAGL includes
#endif


#endif //__OAPLATFORMSPECIFICFUNCTIONPOINTERS_H

