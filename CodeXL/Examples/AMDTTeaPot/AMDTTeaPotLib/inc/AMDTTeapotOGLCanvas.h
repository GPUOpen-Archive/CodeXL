//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTTeapotOGLCanvas.h
///
//==================================================================================

//------------------------------ AMDTTeapotOGLCanvas.h ------------------------------

#ifndef __AMDTTEAPOTOGLCANVAS_H
#define __AMDTTEAPOTOGLCANVAS_H


// Forward decelerations:
struct FIBITMAP;
typedef struct __CFBundle* CFBundleRef;

// OpenGL
#include <inc/AMDTOpenGLHelper.h>
#include <inc/AMDTTeapotRenderState.h>

// Timing
#include <inc/AMDTTimer.h>

#include <GL/gl.h>
#include <GL/glu.h>


#include <inc/AMDTTeapotOCLSmokeSystem.h>

// ----------------------------------------------------------------------------------
// Class Name:           AMDTTeapotOGLCanvas
// General Description:
//   A canvas on which OpenGL draws.
// ----------------------------------------------------------------------------------
class AMDTTeapotOGLCanvas : public ISmokeSystemLogger
{
public:
    AMDTTeapotOGLCanvas();
    virtual ~AMDTTeapotOGLCanvas();

    // event handlers:
    void onPaint();
    void onSize(int width, int height);
    void onEraseBackground();
    void onLeftMouseDown(GLdouble x, GLdouble y);
    void onLeftMouseUp(void);
    void onMouseMove(GLdouble x, GLdouble y);
    void onIdle();

public:
    void crashThisApplication();
    void outputDebugStringExample();
    void generateOpenGLError();
    void generateOpenCLError();
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

    void chooseBestDevices();
    void performSmokeSystemCLInitializations();
    void performSmokeSystemGLInitializations();

    // getters/setters
    const OCLInfo*   getOCLInfo()
    {
        return _OCLInfo;
    }
    const OCLDevice* getOCLSmokeSimDevice()
    {
        return _OCLSmokeSimDevice;
    }
    const OCLDevice* getOCLVolSliceDevice()
    {
        return _OCLVolSliceDevice;
    }
    const bool       usingGLCLSharing()
    {
        return _smokeSystem->usingGLCLSharing();
    }
    const bool       usingCL()
    {
        return _smokeSystem->usingCL();
    }
    const bool       canUseCL()
    {
        return _smokeSystem->canUseCL();
    }
    const char*      getProgressMessage()
    {
        return _progressMessage;
    }
    const float      getFrameRate();

public:
    //ISmokeSystemLogger - this is used by the smoke system to report progress.
    virtual void setProgress(const char* msg);

    // This is called by AMDTTeapotFrame to indicate that a menu item was selected
    // that affects the smoke system. Essentially, the command will be passed
    // over to the AMDTTeapotOCLSmokeSystem as-is.
    void processSmokeSystemCommand(SmokeSystemCommand* cmd);

    // This is called by AMDTTeapotFrame to get the last error from the smoke
    // system.
    const char* getSmokeSystemLastError();

    // Get smoke system
    AMDTTeapotOCLSmokeSystem* getSmokeSystem()
    {
        return _smokeSystem;
    }

    // Calculate the model transformation for the teapot
    Mat4 getTeapotModelTrans();

    /// Sets _shouldCrash to true.
    /// Called when "Crash Application" menu item clicked
    void SetCrashFlag() { _shouldCrash = true; }

private:
    void paintWindow();
    void clearGraphicBuffers();
    void drawScene();
    void drawTeaPot(GLfloat scale);
    void setMaterial();
    void outputDebugString(const wchar_t* pDebugStr);
    void createAndLoadTexture();
    bool readStringFromFile(const wchar_t* filePath, int bufferLength, char* pBuffer);
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
    void bindVertexData();
    void unbindVertexData();
    void performGeneralOpenGLInitializations(); // configure_event should call this
    void performGeneralOpenGLCleanups();
    void onWindowSizeChanged(int width, int height);
    void changeScale(float delta);
    void setupGridTransformation();

private:
    const char* _progressMessage;

    const OCLInfo*   _OCLInfo;
    const OCLDevice* _OCLSmokeSimDevice;
    const OCLDevice* _OCLVolSliceDevice;

    // The background color index:
    int _backgroundColorIndex;

    // Window dimensions
    int _windowWidth;
    int _windowHeight;

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
    GLfloat _objectYRotDelta;

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

    // Class to keep track of project/view matrix and lights.
    AMDTTeapotRenderState _state;

    // Handle to OpenGL helper
    AMDTOpenGLHelper* _ogl;

    // Class instance for handling smoke simulation via OpenCL
    AMDTTeapotOCLSmokeSystem* _smokeSystem;

    // GL model transformation
    GLfloat _modelMatrix[16];

    // Model transformation to position and scale fluid grid over the
    // teaspot relative to teapot coordinates (before view transformation).
    Mat4 _gridTransformation;

    // Teapot scaling factor
    float _teapotScalingFactor;

    // Scaling of the 1x1x1 fluid grid
    float _gridScaleX;
    float _gridScaleY;
    float _gridScaleZ;

    // Time each loop
    AMDTTimer _timer;
    float _numTimeMeasurements;
    float _totalDeltaTime;
    float _fps;

    // Flag to tell paint loop if it should check if GL-CL sharing is active
    // after running a computation cycle.
    bool _checkGLCLSharing;

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

    /// This flag is checked in every drawTeaPot operation
    /// and if true - crashThisApplication is called
    bool _shouldCrash;
};



#endif //__AMDTTEAPOTOGLCANVAS_H

