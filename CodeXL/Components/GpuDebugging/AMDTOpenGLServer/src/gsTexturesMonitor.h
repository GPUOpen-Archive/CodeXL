//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsTexturesMonitor.h
///
//==================================================================================

//------------------------------ gsTexturesMonitor.h ------------------------------

#ifndef __GSTEXTURESMONITOR
#define __GSTEXTURESMONITOR

// Pre decelerations:
struct ap2DRectangle;
class gsTextureUnitMonitor;

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTAPIClasses/Include/apFileType.h>


// Local:
#include <src/gsGLTexture.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsTexturesMonitor
// General Description:
//   Monitors Texture Units and Texture Objects.
//
// Author:               Yaki Tebeka
// Creation Date:        18/04/2004
// ----------------------------------------------------------------------------------
class gsTexturesMonitor
{
public:
    gsTexturesMonitor(int spyContextId);
    ~gsTexturesMonitor();

    // On event functions:
    void onFirstTimeContextMadeCurrent();
    void onContextDeletion();
    void beforeContextDeletion();
    void onTextureObjectsGeneration(GLsizei amountOfGeneratedTextures, GLuint* textureNames);
    void onTextureObjectsDeletion(GLsizei amountOfDeletedTextures, const GLuint* textureNames);
    void onTextureImageLoaded(gsGLTexture* pTextureObj, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type);
    void onTextureMipmapGenerate(gsGLTexture* pTextureObj);

    // Query data functions:
    int textureObjMonitorIndex(GLuint textureName) const;
    bool areMultiTexturesSupported() const {return _areMultiTexturesSupported; };

    // Textures:
    int amountOfTextureObjects() const { return _amountOfTextureObjects; };
    int amountOfTexBufferObjects() const;
    bool getTextureObjectName(int textureObjIndex, GLuint& textureName) const;
    bool getTextureObjectType(int textureObjIndex, apTextureType& textureType) const;
    gsGLTexture* getTextureObjectDetails(GLuint textureName) const;
    gsGLTexture* getTextureObjectDetailsByIndex(int textureObjIndex) const;
    bool updateTextureRawData(apGLTextureMipLevelID textureID);
    bool generateTextureElementsPaths(gsGLTexture* pTextureObj) const;
    gsGLTexture* getCurrentlyBoundTextureObjectDetails(GLenum target) const;

    double amountOfLoadedLevel0Texels() const { return _amountOfLoadedLevel0Texels; };

    // Forced modes:
    void applyForcedStubTextureObjectsToTexUnitMtr(gsTextureUnitMonitor* texUntMtr);
    GLuint forcedStubTextureObjectName(GLenum bindTarget) const;

    // Texture bind for update functions:
    void bindTextureForUpdate(GLuint textureName, GLenum bindTarget);
    void restoreBindedTextureAfterUpdate();
    void markTextureParametersAsNonUpdated();

    // OpenGL ES 2.0 change the enum and value used to get the number of texture units:
#ifdef _GR_IPHONE_BUILD
    GLenum maxTextureUnitsSymbolicName() {return _maxTextureUnitsSymbolicName;};
#endif

    // Memory:
    bool calculateTexturesMemorySize(gtUInt64& texturesMemorySize) const ;

private:
    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead
    gsTexturesMonitor& operator=(const gsTexturesMonitor& otherMonitor);
    gsTexturesMonitor(const gsTexturesMonitor& otherMonitor);

    gsGLTexture* createTextureObjectMonitor(GLuint textureObjectName);

    bool loadSpyTextures();
    bool loadStub1DTexture();
    bool loadStub2DTexture();
    bool loadStub3DTexture();
    bool loadStubCubeMapTexture();
    bool loadStubTextureRectangle();

    void setOpenGLPixelUnPackParameters();
    void restoreOpenGLPixelUnPackParameters();

    void setStubTextureParameters(GLenum bindTarget);
    void generateTextureMiplevelFilePath(apGLTextureMipLevelID textureMipLevelId, GLenum textureBindTarget, osFilePath& textureFilePath) const;

    void textureBindTargetToString(GLenum textureBindTarget, gtString& bindTargetAsString) const;
    GLuint getCurrentlyBoundTexture(GLenum bindTarget) const;
    apFileType loggedTextureFileType(GLenum bindTarget) const;
    void updateTexturesHardwareLimits();
    void addDefaultTextureParameters(gsGLTexture& textureMonitor);

    void onLevel0TexelsAddition(GLsizei width, GLsizei height, GLsizei depth, GLint border);
    void onLevel0TexelsRemoval(GLsizei width, GLsizei height, GLsizei depth, GLint border);
    double amountOfTextureImageTexels(GLsizei width, GLsizei height, GLsizei depth, GLint border) const;
    void updateTextureIndicesAfterDeletion(int deletedTextureIndex, GLuint textureName);

private:
    // The Spy id of my monitored render context:
    int _spyContextId;

    // Holds the amount of allocated texture objects:
    int _amountOfTextureObjects;

    // The amount of loaded texture objects level 0 texels:
    double _amountOfLoadedLevel0Texels;

    // Hold the parameters of textures that reside in this render context:
    gtVector<gsGLTexture*> _textures;

    // Maps texture OpenGL name to the textures vector index:
    gtMap<GLuint, int> _textureOpenGLNameToIndex;

    // Contains true iff 3D textures are supported:
    bool _are3DTexturesSupported;

    // Contains true iff cube map textures are supported:
    bool _areCubeMapTexturesSupported;

    // Contains true iff texture rectangle is supported:
    bool _isTextureRectangleSupported;

    // Contains true iff multi textures are supported:
    bool _areMultiTexturesSupported;

    // Contains true iff client states (glEnableClientState, glClientActiveTexture...) are supported:
    bool _areClientStatesSupported;

    // Have we already generated the stub textures?
    // (used to avoid problems with users calling MakeCurrent before
    // functions such as ShareList)
    static bool _areStubTexturesGenerated;

    // Holds the stub texture names:
    GLuint _stub1DTextureName;
    GLuint _stub2DTextureName;
    GLuint _stub3DTextureName;
    GLuint _stubCubeMapTextureName;
    GLuint _stubTextureRectangleName;

    // Used for update texture data:
    GLuint _currentlyBoundTextureName;
    GLenum _currentlyBoundTarget;
    bool _shouldRestoreTextureBindAfterUpdate;

    // Extension functions pointers:
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    PFNGLTEXIMAGE3DPROC _glTexImage3D;
    PFNGLACTIVETEXTUREPROC _glActiveTexture;
    PFNGLCLIENTACTIVETEXTUREPROC _glClientActiveTexture;
#endif

    // OpenGL ES 2.0 change the enum and value used to get the number of texture units:
#ifdef _GR_IPHONE_BUILD
    GLenum _maxTextureUnitsSymbolicName;
#endif
};


#endif  // __GSTEXTURESMONITOR
