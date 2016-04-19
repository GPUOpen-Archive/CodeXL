//------------------------------ AMDTTeapotOGLCanvasStressTest.h ------------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------


#ifndef __AMDTTEAPOTOGLCANVASSTRESSTEST_H
#define __AMDTTEAPOTOGLCANVASSTRESSTEST_H


// Forward decelerations:
struct FIBITMAP;
typedef struct __CFBundle* CFBundleRef;

// OpenGL:
#define GL_GLEXT_LEGACY
#if defined(__APPLE__)
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif
#include <GL/glext.h>
#include <GL/GRemedyGLExtensions.h>

// Linux specific includes:
#if defined(__linux__)
    #include <GL/glx.h>
#endif

// Type definitions:
typedef void (*tpProcedureAddress)(void);
typedef void (APIENTRYP PFNGLACTIVETEXTUREPROC)(GLenum texture);

#if defined(__APPLE__)
    typedef void (* PFNGLGENBUFFERSPROC)(GLsizei n, GLuint* buffers);
    typedef void (* PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint* buffers);
    typedef void (* PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
    typedef void (* PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
#endif

// ----------------------------------------------------------------------------------
// Class Name:           AMDTTeapotOGLCanvas
// General Description:
//   A canvas on which OpenGL draws.
// ----------------------------------------------------------------------------------
class AMDTTeapotOGLCanvas
{
public:
    AMDTTeapotOGLCanvas(long style = 0);
    virtual ~AMDTTeapotOGLCanvas();

    // wxWidgets event handlers:
    void onPaint(wxPaintEvent& event);
    void onSize(wxSizeEvent& event);
    void onEraseBackground(wxEraseEvent& event);
    void onKeyDown(wxKeyEvent& event);
    void onLeftMouseDown(wxMouseEvent& event);
    void onLeftMouseUp(wxMouseEvent& event);
    void onMouseMove(wxMouseEvent& event);
    void onIdle(wxIdleEvent& event);

public:
    void crashThisApplication();
    void outputDebugStringExample();
    void generateOpenGLError();
    void throwBreakPointException();
    void generateDetectedError();
    void cycleBackgroundColor();
    void toggleFragmentShaders();
    void increaseTextureInfluence();
    void decreaseTextureInfluence();
    void increaseSpikiness();
    void decreaseSpikiness();
    void cycleShadingProgram();
    void cycleGeometryShader();
    void moveObjectOutOfView();
    void changeObjectShininess();
    void cycleRasterModes();

    bool canIncreaseTextureInfluence();
    bool canDecreaseTextureInfluence();
    bool canIncreaseSpikiness();
    bool canDecreaseSpikiness();
    bool canCycleShadingProgram();
    bool canCycleGeometryShader();
    bool isShadingProgramInUse();
    bool isGeometryShaderInUse();

private:
    void paintWindow();
    void clearGraphicBuffers();
    void drawScene();
    void drawTeaPot(GLfloat scale);
    void setMaterial();
    void setupLights();
    void outputDebugString(const char* pDebugStr);
    void createAndLoadTexture(GLuint& textureName, bool loadImage, int imageIndex);
    void createAndLoadTexture();
    bool readStringFromFile(const char* filePath, int bufferLength, char* pBuffer);
    bool isExtensionSupported(char* extensionName, const char* extensionsString);
    tpProcedureAddress getProcAddress(const char* pProcName);
    void initializeExtensions();
    bool setupProgramsAndShaders();
    bool createShaderObjects();
    bool setShadersSourceCodes();
    bool compileShaderObjects();
    bool createProgramObject();
    bool attachShadersToProgram();
    bool linkProgramObject();
    void useShadingProgram(bool isShadingProgramUsed);
    void useGeometryShader(bool isGeometryShaderUsed);
    void setupVertexArrays();
    bool createVertexBufferObjects();
    bool loadAndBindDataIntoVertexBufferObjects();
    void bindVertexArrayLocalData();
    void performGeneralOpenGLInitializations();
    void performGeneralOpenGLCleanups();
    void onWindowSizeChanged(int width, int height);
    tpProcedureAddress getMacOSXExtensionFunctionAddress(const char* pProcName);

    // Stress test functions:
    void createStressTestObjects();
    void createDummyRenderBuffers();
    bool touchDirtyTexture();

    // Do not allow the use of my default constructor:
    AMDTTeapotOGLCanvas();

private:
    // The canvas on which OpenGL draws:
    AMDTTeapotOGLCanvas* _pOGLCanvas;

    // The background color index:
    int _backgroundColorIndex;

    // Window position and size:
    int _windowXPosition;
    int _windowYPosition;
    int _windowWidth;
    int _windowHeight;

    // Graphics view-port:
    int _viewPortX;
    int _viewPortY;
    int _viewWidth;
    int _viewHeight;

    // Heads up message raster position:
    float _headsUpRasterPosX;
    float _headsUpRasterPosY;

    // The initial mouse position:
    GLint _mouseInitialPosX;
    GLint _mouseInitialPosY;

    // The difference in mouse position from the last observed mouse position:
    GLint _mouseDeltaY;
    GLint _mouseDeltaX;

    // Contains true iff we are in viewing mode:
    // (when the user manipulates the object using the mouse)
    bool _isInViewingMode;

    // The object rotation (degrees around the X Y and Z axis):
    GLfloat _objectXRot;
    GLfloat _objectYRot;
    GLfloat _objectZRot;

    // The object translation:
    GLfloat _objectXTran;
    GLfloat _objectYTran;
    GLfloat _objectZTran;

    // Contains true iff the model view matrix is changed:
    bool _isModelViewMatrixChanged;

    // Contains true iff the rendered object shininess is changed:
    bool _isObjectShininessChanged;

    // The used fragment shader:
    int _fragmentShaderIndex;

    // VBOs used to hold the teapot data:
    GLuint _verticesVBOName;
    GLuint _normalsVBOName;
    GLuint _texCoordVBOName;
    GLuint _indicesVBOName;

    // The base pointer (in a VBO or in client memory) used for glDrawElements:
    const GLvoid* _indicesBasePointer;

    // The name of the used texture object:
    GLuint _textureObjName;

    // The used raster mode:
    GLenum _rasterMode;

    // Shaders and Programs names:
    GLhandleARB _vertexShaderName;
    GLhandleARB _geometryShaderName;
    GLhandleARB _fragmentShaderName;
    GLhandleARB _shadingProgramName;

    // Controls texture influence [0,10]:
    int _textureInfluenceAsInt;

    // Texture influence as GLfloat [0,1]:
    GLfloat _textureInfluenceAsFloat;

    // textureInfluence uniform location in _shadingProgramName:
    GLint _textureInfluenceUniformLocation;

    // Controls geometry shader effect (spikiness) [-20, 50]:
    int _spikinessAsInt;

    // Spikiness as GLfloat [-10, 25]:
    GLfloat _spikinessAsFloat;

    // Spikiness uniform location in _shadingProgramName:
    GLint _spikinessUniformLocation;

    // Contains true iff the shading program was created successfully:
    bool _shadingProgramExists;

    // Contains true iff the shading program is currently used:
    bool _shadingProgramInUse;

    // Contains true iff the geometry shader was compiled successfully:
    bool _isGeometryShaderCompiled;

    // Contains true iff the geometry shader is being used:
    bool _geometryShaderInUse;

    // Handled to the System's OpenGL Framework (used on Mac OS X only):
    void* _hSystemOpenGLFramework;

    // Handle to the Graphic Remedy's OpenGL server (used on Mac OS X only):
    void* _hGremedyOpenGLServer;

    // GL_GREMEDY_string_marker function pointer:
    PFNGLSTRINGMARKERGREMEDYPROC _glStringMarkerGREMEDY;

    // GL_GREMEDY_frame_terminator function pointers:
    PFNGLFRAMETERMINATORGREMEDYPROC _glFrameTerminatorGREMEDY;

    // GL_ARB_shader_objects extension functions:
    bool _isGL_ARB_shader_objectsSupported;
    PFNGLCREATESHADEROBJECTARBPROC _glCreateShaderObjectARB;
    PFNGLSHADERSOURCEARBPROC _glShaderSourceARB;
    PFNGLCOMPILESHADERARBPROC _glCompileShaderARB;
    PFNGLGETOBJECTPARAMETERIVARBPROC _glGetObjectParameterivARB;
    PFNGLCREATEPROGRAMOBJECTARBPROC _glCreateProgramObjectARB;
    PFNGLATTACHOBJECTARBPROC _glAttachObjectARB;
    PFNGLLINKPROGRAMARBPROC _glLinkProgramARB;
    PFNGLUSEPROGRAMOBJECTARBPROC _glUseProgramObjectARB;
    PFNGLGETINFOLOGARBPROC _glGetInfoLogARB;
    PFNGLGETUNIFORMLOCATIONARBPROC _glGetUniformLocationARB;
    PFNGLUNIFORM1FARBPROC _glUniform1fARB;
    PFNGLDELETEOBJECTARBPROC _glDeleteObjectARB;
    PFNGLDETACHOBJECTARBPROC _glDetachObjectARB;

    // GL_ARB_vertex_buffer_object extension functions:
    bool _isGL_ARB_vertex_buffer_objectSupported;
    PFNGLGENBUFFERSARBPROC _glGenBuffersARB;
    PFNGLBINDBUFFERARBPROC _glBindBufferARB;
    PFNGLBUFFERDATAARBPROC _glBufferDataARB;
    PFNGLDELETEBUFFERSARBPROC _glDeleteBuffersARB;

    // OpenGL 1.3 extension functions:
    PFNGLACTIVETEXTUREPROC _glActiveTexture;

    // OpenGL 1.5 extension functions:
    bool _isOpenGL1_5Supported;
    PFNGLGENBUFFERSPROC _glGenBuffers;
    PFNGLBINDBUFFERPROC _glBindBuffer;
    PFNGLBUFFERDATAPROC _glBufferData;
    PFNGLDELETEBUFFERSPROC _glDeleteBuffers;

    // GL_EXT_geometry_shader4 objects extension functions:
    bool _isGL_EXT_geometry_shader4Supported;
    PFNGLPROGRAMPARAMETERIEXTPROC _glProgramParameteriEXT;

    // GL_NV_vertex_buffer_unified_memory function pointers:
    bool _isGL_NV_vertex_buffer_unified_memorySupported;
    PFNGLVERTEXATTRIBIFORMATNVPROC _glVertexAttribIFormatNV;

    // GL_ARB_framebuffer_object extension functions:
    PFNGLGENFRAMEBUFFERSPROC _glGenFramebuffers;
    PFNGLBINDFRAMEBUFFERPROC _glBindFramebuffer;
    PFNGLFRAMEBUFFERRENDERBUFFERPROC _glFramebufferRenderbuffer;
    PFNGLGENRENDERBUFFERSPROC _glGenRenderbuffers;
    PFNGLBINDRENDERBUFFERPROC _glBindRenderbuffer;
    PFNGLRENDERBUFFERSTORAGEPROC _glRenderbufferStorage;
    PFNGLGETRENDERBUFFERPARAMETERIVPROC _glGetRenderbufferParameteriv;
    PFNGLISRENDERBUFFERPROC _glIsRenderbuffer;

    // Stress test attributes:
    bool _shouldCreateDummyRenderBuffers;
    bool _shouldCreateDummyVBOs;
    bool _shouldCreateDummyTextures;
    bool _shouldCreateDummyDisplayLists;
    bool _shouldCreateDummyShaders;

    GLuint _dirtyBigTextureName;

private:
    // wxWidgets events table:
    DECLARE_EVENT_TABLE()
};


#endif //__AMDTTEAPOTOGLCANVASSTRESSTEST_H

