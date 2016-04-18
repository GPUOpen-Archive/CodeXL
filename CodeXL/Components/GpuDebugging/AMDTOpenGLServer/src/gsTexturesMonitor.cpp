//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsTexturesMonitor.cpp
///
//==================================================================================

//------------------------------ gsTexturesMonitor.cpp ------------------------------

#include <math.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/apDefinitions.h>
#include <AMDTAPIClasses/Include/apDefaultTextureNames.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Spy Utilities:
#include <AMDTServerUtilities/Include/suStringConstants.h>

// Local:
#include <src/gsStringConstants.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsWrappersCommon.h>
#include <src/gsExtensionsManager.h>
#include <src/gsRenderContextMonitor.h>
#include <src/gsTextureSerializer.h>
#include <src/gsTexturesMonitor.h>

// Set the Beginning of the texture names to be somewhere near the end of the UINT range
#define GS_RANDOM_NUMBER 861408
#define GS_STUB_TEXTURE_NAMES_STARTING_NUMBER (UINT_MAX - GS_RANDOM_NUMBER)

// Static members initializations:
bool gsTexturesMonitor::_areStubTexturesGenerated = false;

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::gsTexturesMonitor
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        18/4/2005
// ---------------------------------------------------------------------------
gsTexturesMonitor::gsTexturesMonitor(int spyContextId)
    : _spyContextId(spyContextId),
      _amountOfTextureObjects(0),
      _amountOfLoadedLevel0Texels(0),
      _are3DTexturesSupported(false),
      _areCubeMapTexturesSupported(false),
      _isTextureRectangleSupported(false),
      _areMultiTexturesSupported(false),
      _areClientStatesSupported(false),
      _stub1DTextureName(GS_STUB_TEXTURE_NAMES_STARTING_NUMBER),
      _stub2DTextureName(GS_STUB_TEXTURE_NAMES_STARTING_NUMBER + 1),
      _stub3DTextureName(GS_STUB_TEXTURE_NAMES_STARTING_NUMBER + 2),
      _stubCubeMapTextureName(GS_STUB_TEXTURE_NAMES_STARTING_NUMBER + 3),
      _stubTextureRectangleName(GS_STUB_TEXTURE_NAMES_STARTING_NUMBER + 4),
      _currentlyBoundTextureName(0),
      _currentlyBoundTarget(0),
      _shouldRestoreTextureBindAfterUpdate(false)
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    , _glTexImage3D(NULL),
      _glActiveTexture(NULL),
      _glClientActiveTexture(NULL)
#endif
#ifdef _GR_IPHONE_BUILD
    , _maxTextureUnitsSymbolicName(GL_MAX_TEXTURE_UNITS)
#endif
{
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::~gsTexturesMonitor
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        18/4/2005
// ---------------------------------------------------------------------------
gsTexturesMonitor::~gsTexturesMonitor()
{
    // Delete the texture wrappers vector:
    int amountOfTextures = (int)_textures.size();

    for (int i = 0; i < amountOfTextures; i++)
    {
        delete _textures[i];
        _textures[i] = NULL;
    }

    _textures.clear();
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::onFirstTimeContextMadeCurrent
// Description: Is called on the first time in which my context is made the
//              current context.
// Author:      Yaki Tebeka
// Date:        18/4/2005
// ---------------------------------------------------------------------------
void gsTexturesMonitor::onFirstTimeContextMadeCurrent()
{
    // If my monitored context is NOT the NULL context:
    if (_spyContextId != AP_NULL_CONTEXT_ID)
    {
        // Update texture hardware limits:
        updateTexturesHardwareLimits();
    }
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::onContextDeletion
// Description: Is called when the render context is deleted.
// Author:      Yaki Tebeka
// Date:        3/7/2006
// ---------------------------------------------------------------------------
void gsTexturesMonitor::onContextDeletion()
{

}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::beforeContextDeletion
// Description: Is called before the render context is deleted.
// Return Val: void
// Author:      Sigal Algranaty
// Date:        30/11/2008
// ---------------------------------------------------------------------------
void gsTexturesMonitor::beforeContextDeletion()
{
    // Update all textures parameters:
    for (int i = 0; i < (int)_textures.size(); i++)
    {
        // Get the current texture:
        gsGLTexture* pTexture = _textures[i];

        // Update the texture parameters:
        GT_IF_WITH_ASSERT(pTexture != NULL)
        {
            pTexture->updateTextureParameters(false);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::onTextureObjectsGeneration
// Description: Is called when texture objects are generated.
// Arguments:   amountOfGeneratedTextures - The amount of generated texture objects.
//              textureNames - An array, containing the names of the generated texture
//                             objects.
// Author:      Yaki Tebeka
// Date:        25/12/2004
// ---------------------------------------------------------------------------
void gsTexturesMonitor::onTextureObjectsGeneration(GLsizei amountOfGeneratedTextures, GLuint* textureNames)
{
    // If the monitored context is NOT the NULL context:
    GT_IF_WITH_ASSERT(_spyContextId != AP_NULL_CONTEXT_ID)
    {
        gtVector<apAllocatedObject*> texturesForAllocationMonitor;

        for (int i = 0; i < amountOfGeneratedTextures; i++)
        {
            // Create a texture object monitor for the created texture:
            GLuint newTextureName = textureNames[i];
            gsGLTexture* pCreatedTex = createTextureObjectMonitor(newTextureName);
            GT_IF_WITH_ASSERT(NULL != pCreatedTex)
            {
                // Register each texture, and its level 0 mipmap, in the allocation monitor:
                texturesForAllocationMonitor.push_back(pCreatedTex);
                apGLTextureMipLevel* pTexLevel0 = pCreatedTex->getTextureMipLevel(0);
                GT_IF_WITH_ASSERT(NULL != pCreatedTex)
                {
                    texturesForAllocationMonitor.push_back(pTexLevel0);
                }
            }
        }

        // Register this object in the allocated objects monitor:
        su_stat_theAllocatedObjectsMonitor.registerAllocatedObjects(texturesForAllocationMonitor);
    }
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::onTextureObjectsDeletion
// Description: Is called when textures objects are deleted.
// Arguments:   amountOfDeletedTextures - The amount of deleted texture objects.
//              textureNames - An array, containing the names of the texture objects
//                             to be deleted.
// Author:      Yaki Tebeka
// Date:        25/12/2004
// ---------------------------------------------------------------------------
void gsTexturesMonitor::onTextureObjectsDeletion(GLsizei amountOfDeletedTextures, const GLuint* textureNames)
{
    // If the monitored context is NOT the NULL context:
    GT_IF_WITH_ASSERT(_spyContextId != AP_NULL_CONTEXT_ID)
    {
        for (int i = 0; i < amountOfDeletedTextures; i++)
        {
            // Delete the input texture param:
            GLuint currentTextureName = textureNames[i];

            // Ignore texture 0:
            if (currentTextureName != 0)
            {
                // Get the texture object monitor index:
                int monitorObjIndex = textureObjMonitorIndex(currentTextureName);

                // If the user is trying to delete a non existing texture:
                if (monitorObjIndex == -1)
                {
                    // Display an error message to the user -
                    // TO_DO: Replace me by a detected error!
                    gtString errorMessage = GS_STR_deletingNonExistingTexture;
                    errorMessage.appendFormattedString(L" %u", currentTextureName);
                    osOutputDebugString(errorMessage);
                }
                else
                {
                    // Get the object that represents the texture that is going to be deleted:
                    gsGLTexture* pTextureToBeDeleted = _textures[monitorObjIndex];
                    GT_IF_WITH_ASSERT(pTextureToBeDeleted != NULL)
                    {
                        // Get the deleted texture getDimensions:
                        GLsizei width = 0; GLsizei height = 0; GLsizei depth = 0; GLsizei border = 0;
                        _textures[monitorObjIndex]->getDimensions(width, height, depth, border);

                        // If a texture image was loaded into this texture:
                        if (0 < width)
                        {
                            // Update about the amount of removed level 0 texels:
                            onLevel0TexelsRemoval(width, height, depth, border);

                            // If calls logging to HTML file was on:
                            // const gsOpenGLMonitor& theOpenGLMonitor = gsOpenGLMonitor::instance();
                            bool wereHTMLLogFilesActive = suWereHTMLLogFilesActive();

                            if (wereHTMLLogFilesActive)
                            {
                                // Save the textures raw data to disk:
                                // updateTextureRawData(pTextureToBeDeleted);

                                // TO_DO: Yaki 25/11/2008:
                                // The above textures raw data is saved to disk, so that the CodeXL application can later on
                                // convert the raw data files into thumbnail files to be displayed in the HTML log.
                                // However, only the raw data of textures that were NOT deleted are converted to thumbnails.
                                // Until the problem is solved, I commented out the above updateTextureRawData line.
                                // To solve the problem, we need to:
                                // - Uncomment the above updateTextureRawData line.
                                // - When the debugged process exits, if recording was on, traverse the log file directory
                                //   (and NOT the existing textures), looking for raw data files and convert all to thumbnails.
                                // * See case 4452 for more details about this problem and similar problems.
                            }
                        }

                        // Delete the texture param object:
                        delete pTextureToBeDeleted;
                    }

                    // Update the texture indices after the texture deletion:
                    updateTextureIndicesAfterDeletion(monitorObjIndex, currentTextureName);

                    // Decrement the allocated textures amount:
                    _amountOfTextureObjects--;

                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::onTextureMipmapGenerate
// Description: Handle mipmap generation event
// Arguments: gsGLTexture* pTextureObj - the texture object
// Return Val: void
// Author:      Sigal Algranaty
// Date:        18/1/2009
// ---------------------------------------------------------------------------
void gsTexturesMonitor::onTextureMipmapGenerate(gsGLTexture* pTextureObj)
{
    GT_IF_WITH_ASSERT(pTextureObj != NULL)
    {
        // Set the mipmap auto generation flag:
        pTextureObj->setMipmapAutoGeneration(true);

        // Auto generate mipmap levels:
        bool rc = pTextureObj->generateAutoMipmapLevels();
        GT_ASSERT(rc);
    }
}
// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::onTextureImageLoaded
// Description: Is called after texture data is loaded into OpenGL.
// Arguments:   width, height, depth - The texture getDimensions.
//              border - The texture border width.
//              format - The format of the input texture data.
//                       I.E: How are the input texels arranged (GL_RGB, GL_RGBA, etc).
//              type - The type of the input texture texels (GL_BYTE, GL_SHORT, etc).
// Author:      Yaki Tebeka
// Date:        25/12/2004
// ---------------------------------------------------------------------------
void gsTexturesMonitor::onTextureImageLoaded(gsGLTexture* pTextureObj, GLsizei width, GLsizei height,
                                             GLsizei depth, GLint border, GLenum format, GLenum type)
{
    (void)(format); // unused
    (void)(type); // unused
    // If the monitored context is NOT the NULL context and pTextureObj exists:
    GT_IF_WITH_ASSERT((_spyContextId != AP_NULL_CONTEXT_ID) && (pTextureObj != NULL))
    {
        // Get the previous texture getDimensions:
        GLsizei prevWidth = 0; GLsizei prevHeight = 0; GLsizei prevDepth = 0; GLsizei prevBorder = 0;
        pTextureObj->getDimensions(prevWidth, prevHeight, prevDepth, prevBorder);

        // Store the new texture getDimensions:
        pTextureObj->setDimensions(width, height, depth, border);

        // Add default parameters to texture:
        // NOTICE: This function is called here, since it's the first time we know what is the
        // texture type and can take it into consideration when deciding how many parameter sets
        // we need to add

#if (defined OS_OGL_ES_IMPLEMENTATION_DLL_BUILD) || (defined _GR_IPHONE_BUILD)
        // Add OpenGL ES default texture parameters:
        pTextureObj->addDefaultGLESTextureParameters();
#else
        // Add OpenGL default texture parameters:
        pTextureObj->addDefaultGLTextureParameters();
#endif

        // If an texture image was already loaded into this texture:
        if (0 < prevWidth)
        {
            // Update about the amount of removed level 0 texels:
            onLevel0TexelsRemoval(prevWidth, prevHeight, prevDepth, prevBorder);
        }

        // Update the about amount of loaded level 0 texels:
        onLevel0TexelsAddition(width, height, depth, border);

        // Generate the mipmap levels if needed:
        bool rc2 = pTextureObj->generateAutoMipmapLevels();
        GT_ASSERT(rc2);
    }
}
void onTextureBuffer(GLenum target, GLenum internalformat, GLuint buffer);

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::getTextureObjectName
// Description: Inputs a texture id (in this context) and returns its OpenGL name.
// Arguments:   contextId - The context in which the texture resides.
//              textureObjIndex - The index of the texture in this context.
//              textureName - The OpenGL name of the texture.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        30/12/2004
// ---------------------------------------------------------------------------
bool gsTexturesMonitor::getTextureObjectName(int textureObjIndex, GLuint& textureName) const
{
    bool retVal = false;

    // Sanity test:
    int amountOfTextureIndices = (int)_textures.size();

    if ((0 <= textureObjIndex) && (textureObjIndex < amountOfTextureIndices))
    {

        // The queried texture index is the _textures array index:
        textureName = _textures[textureObjIndex]->textureName();
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::getTextureObjectType
// Description: Inputs a texture id (in this context) and returns its type.
// Arguments: contextId - The context in which the texture resides.
//              textureObjIndex - The index of the texture in this context.
//            apTextureType& textureType - Output - the texture type
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/8/2009
// ---------------------------------------------------------------------------
bool gsTexturesMonitor::getTextureObjectType(int textureObjIndex, apTextureType& textureType) const
{
    bool retVal = false;

    // Sanity test:
    int amountOfTextureIndices = (int)_textures.size();

    if ((0 <= textureObjIndex) && (textureObjIndex < amountOfTextureIndices))
    {
        // The queried texture index is the _textures array index:
        textureType = _textures[textureObjIndex]->textureType();
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::getTextureObjectDetails
// Description: Returns a queried texture object details.
// Arguments:   textureName - The OpenGL name of the texture.
// Return Val:  const gsGLTexture* - Will get the texture details, or NULL if a
//                                   texture of this name does not exist.
// Author:      Yaki Tebeka
// Date:        30/12/2004
// ---------------------------------------------------------------------------
gsGLTexture* gsTexturesMonitor::getTextureObjectDetails(GLuint textureName) const
{
    gsGLTexture* retVal = NULL;

    // Get the id of the input texture object monitor:
    int monitorObjIndex = textureObjMonitorIndex(textureName);

    if (monitorObjIndex != -1)
    {
        // Return the texture object details:
        retVal = _textures[monitorObjIndex];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::getTextureObjectDetailsByIndex
// Description: Returns a queried texture object details by the texture index.
// Arguments:   textureIbjIndex - the texture object location within the textures vector
// Return Val:  const gsGLTexture* - Will get the texture details, or NULL if the
//                                   texture of does not exist.
// Author:      Sigal Algranaty
// Date:        19/7/2010
// ---------------------------------------------------------------------------
gsGLTexture* gsTexturesMonitor::getTextureObjectDetailsByIndex(int textureObjIndex) const
{
    gsGLTexture* retVal = NULL;

    if ((textureObjIndex >= 0) && (textureObjIndex < (int)_textures.size()))
    {
        // Return the texture object details:
        retVal = _textures[textureObjIndex];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::applyForcedStubTextureObjects
// Description: Applies the "Forces stub textures" mode.
// Author:      Yaki Tebeka
// Date:        2/3/2005
// ---------------------------------------------------------------------------
void gsTexturesMonitor::applyForcedStubTextureObjectsToTexUnitMtr(gsTextureUnitMonitor* texUntMtr)
{
    if (!_areStubTexturesGenerated)
    {
        // Load the spy aid textures:
        _areStubTexturesGenerated = loadSpyTextures();
        GT_ASSERT(_areStubTexturesGenerated);
    }

    // Replace currently bound textures with the stub textures:
    texUntMtr->applyForcedStubTextureObjects(_stub1DTextureName, _stub2DTextureName,
                                             _stub3DTextureName, _stubCubeMapTextureName,
                                             _stubTextureRectangleName);
}

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::forcedStubTextureObjectName
// Description: Returns the forced stub texture object name for an input bind target.
// Author:      Yaki Tebeka
// Date:        4/3/2005
// ---------------------------------------------------------------------------
GLuint gsTexturesMonitor::forcedStubTextureObjectName(GLenum bindTarget) const
{
    GLuint retVal = 0;

    switch (bindTarget)
    {
        case GL_TEXTURE_1D:
            retVal = _stub1DTextureName;
            break;

        case GL_TEXTURE_2D:
            retVal = _stub2DTextureName;
            break;

        case GL_TEXTURE_3D:
            retVal = _stub3DTextureName;
            break;

        case GL_TEXTURE_CUBE_MAP:
            retVal = _stubCubeMapTextureName;
            break;

        case GL_TEXTURE_RECTANGLE_ARB:
            retVal = _stubTextureRectangleName;
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::textureObjMonitorIndex
// Description: Inputs an OpenGL texture name and outputs its monitor's location (index)
//              in the _textures array, or -1 if it does not exist.
// Author:      Yaki Tebeka
// Date:        25/12/2004
// ---------------------------------------------------------------------------
int gsTexturesMonitor::textureObjMonitorIndex(GLuint textureName) const
{
    int retVal = -1;

    gtMap<GLuint, int>::const_iterator endIter = _textureOpenGLNameToIndex.end();
    gtMap<GLuint, int>::const_iterator iter = _textureOpenGLNameToIndex.find(textureName);

    if (iter != endIter)
    {
        retVal = (*iter).second;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::loadSpyTextures
// Description: Loads aid textures used by this spy.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        2/3/2005
// ---------------------------------------------------------------------------
bool gsTexturesMonitor::loadSpyTextures()
{
    bool retVal = false;

    // Set OpenGL pixel pack parameters:
    setOpenGLPixelUnPackParameters();

    // Load spy stub textures (according to the current OGL implementation extension's support):
    // ----------------------------------------------------------------------------------------

#ifdef OS_OGL_ES_IMPLEMENTATION_DLL_BUILD
    bool rc3 = loadStub2DTexture();
    retVal = rc3;
#elif defined (_GR_IPHONE_BUILD)
    bool rc2 = loadStub2DTexture();
    bool rc3 = !_areCubeMapTexturesSupported;

    if (!rc3)
    {
        rc3 = loadStubCubeMapTexture();
    }

    retVal = rc2 && rc3;
#else
    bool rc2 = loadStub1DTexture();
    bool rc3 = loadStub2DTexture();

    if (_are3DTexturesSupported)
    {
        loadStub3DTexture();
    }

    if (_areCubeMapTexturesSupported)
    {
        loadStubCubeMapTexture();
    }

    if (_isTextureRectangleSupported)
    {
        loadStubTextureRectangle();
    }

    retVal = rc2 && rc3;
#endif

    // Restore the pixel un-pack parameters:
    restoreOpenGLPixelUnPackParameters();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::loadStub1DTexture
// Description: Created and loads a 2X1 stub 1D texture.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        2/3/2005
// ---------------------------------------------------------------------------
bool gsTexturesMonitor::loadStub1DTexture()
{
    bool retVal = true;

    // We do not need to generate the stub 1D texture name with glGenTextures
    // as we choose a "random" one ourselves (see initializations);
#ifdef _GR_IPHONE_BUILD
    // Uri, 11/6/09: 1D textures are not currently supported on OpenGL ES
    GT_ASSERT(false);
#else
    // Make it the active texture:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindTexture);
    gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_1D, _stub1DTextureName);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindTexture);

    // Load a stub 2 X 1 texture into it:
    GLubyte stubTextureData[2][3] = {{0, 0, 255}, {0, 255, 0}};
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glTexImage1D);
    gs_stat_realFunctionPointers.glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB8, 2, 0, GL_RGB,
                                              GL_UNSIGNED_BYTE, stubTextureData);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glTexImage1D);

    // Set the stub texture parameters:
    setStubTextureParameters(GL_TEXTURE_1D);

    // Set the bind texture to be the default texture name:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindTexture);
    gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_1D, 0);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindTexture);
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::loadStub2DTexture
// Description: Created and loads a 2X2 stub 2D texture.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        2/3/2005
// ---------------------------------------------------------------------------
bool gsTexturesMonitor::loadStub2DTexture()
{
    bool retVal = true;

    // We do not need to generate the stub 2D texture name with glGenTextures
    // as we choose a "random" one ourselves (see initializations);

    // Make it the active texture:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindTexture);
    gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_2D, _stub2DTextureName);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindTexture);

    // Load a stub 2 X 2 texture into it:
    GLubyte stubTextureData[2][2][3] = {{{0, 0, 255}, {0, 255, 0}},
        {{255, 0, 0}, {255, 255, 0}}
    };
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glTexImage2D);
#ifdef _GR_IPHONE_BUILD
    gs_stat_realFunctionPointers.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB,
                                              GL_UNSIGNED_BYTE, stubTextureData);
#else
    gs_stat_realFunctionPointers.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 2, 2, 0, GL_RGB,
                                              GL_UNSIGNED_BYTE, stubTextureData);
#endif
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glTexImage2D);

    // Set the stub texture parameters:
    setStubTextureParameters(GL_TEXTURE_2D);

    // Set the bind texture to be the default texture name:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindTexture);
    gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_2D, 0);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindTexture);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::loadStub3DTexture
// Description: Created and loads a 2X2X2 stub 3D texture.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        2/3/2005
// ---------------------------------------------------------------------------
bool gsTexturesMonitor::loadStub3DTexture()
{
    bool retVal = false;

#ifdef _GR_IPHONE_BUILD
    // Uri, 11/6/09: 3D textures are not supported in OpenGL ES
    GT_ASSERT(false);
#else
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
#define _glTexImage3D gs_stat_realFunctionPointers.glTexImage3D
#endif

    // If 3D textures are supported (they are always supported in Mac):
    if (_glTexImage3D)
    {
        // We do not need to generate the stub 3D texture name with glGenTextures
        // as we choose a "random" one ourselves (see initializations);

        // Make it the active texture:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindTexture);
        gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_3D, _stub3DTextureName);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindTexture);

        // Load a stub 2 X 2 X 2 texture into it:
        GLubyte stubTextureData[2][2][2][3] =
        {
            {{{0, 0, 255},     {0, 255, 0}},   {{255, 0, 0},   {255, 255, 0}}},
            {{{255, 255, 255},  {0, 0, 0}},     {{255, 0, 255}, {0, 255, 255}}}
        };

        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glTexImage3D);
        gs_stat_realFunctionPointers.glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB8, 2, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, stubTextureData);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glTexImage3D);
        retVal = true;
    }

    // Set the stub texture parameters:
    setStubTextureParameters(GL_TEXTURE_3D);

    // Set the bind texture to be the default texture name:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindTexture);
    gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_3D, 0);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindTexture);

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
#undef _glTexImage3D
#endif

#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::loadStubCubeMapTexture
// Description: Created and loads a 2X2X6 stub cube map texture.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        2/3/2005
// ---------------------------------------------------------------------------
bool gsTexturesMonitor::loadStubCubeMapTexture()
{
    bool retVal = true;

    // Clear openGL errors (if any):
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
    GLenum curOpenGLErr = gs_stat_realFunctionPointers.glGetError();
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

    // We do not need to generate the stub Cube map texture name with glGenTextures
    // as we choose a "random" one ourselves (see initializations);

    // Make it the active texture:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindTexture);
    gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_CUBE_MAP, _stubCubeMapTextureName);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindTexture);

    // Load a stub 2 X 2 cube map textures into it:
    GLubyte stubTextureData[2][2][3] = {{{0, 0, 255}, {0, 255, 0}},
        {{255, 0, 0}, {255, 255, 0}}
    };
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glTexImage2D);
    gs_stat_realFunctionPointers.glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB8, 2, 2, 0, GL_RGB,
                                              GL_UNSIGNED_BYTE, stubTextureData);

    gs_stat_realFunctionPointers.glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB8, 2, 2, 0, GL_RGB,
                                              GL_UNSIGNED_BYTE, stubTextureData);

    gs_stat_realFunctionPointers.glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB8, 2, 2, 0, GL_RGB,
                                              GL_UNSIGNED_BYTE, stubTextureData);

    gs_stat_realFunctionPointers.glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB8, 2, 2, 0, GL_RGB,
                                              GL_UNSIGNED_BYTE, stubTextureData);

    gs_stat_realFunctionPointers.glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB8, 2, 2, 0, GL_RGB,
                                              GL_UNSIGNED_BYTE, stubTextureData);

    gs_stat_realFunctionPointers.glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB8, 2, 2, 0, GL_RGB,
                                              GL_UNSIGNED_BYTE, stubTextureData);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glTexImage2D);

    // Set the stub texture parameters:
    setStubTextureParameters(GL_TEXTURE_CUBE_MAP);

    // If we generated OpenGL errors:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
    curOpenGLErr = gs_stat_realFunctionPointers.glGetError();
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

    if (curOpenGLErr != GL_NO_ERROR)
    {
        // Failure clean up:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDeleteTextures);
        gs_stat_realFunctionPointers.glDeleteTextures(1, &_stubCubeMapTextureName);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDeleteTextures);
        _stubCubeMapTextureName = 0;
    }

    // Set the bind texture to be the default texture name:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindTexture);
    gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindTexture);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::loadStubTextureRectangle
// Description: Created and loads a 2X2 stub 2D texture.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        2/3/2005
// ---------------------------------------------------------------------------
bool gsTexturesMonitor::loadStubTextureRectangle()
{
    bool retVal = true;

    // Clear openGL errors (if any):
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
    GLenum curOpenGLErr = gs_stat_realFunctionPointers.glGetError();
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

    // We do not need to generate the stub Rectangle texture name with glGenTextures
    // as we choose a "random" one ourselves (see initializations);

#ifdef _GR_IPHONE_BUILD
    // Uri, 11/6/09: OpenGL ES does not support rectangle textures:
    GT_ASSERT(false);
#else
    // Make it the active texture:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindTexture);
    gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _stubTextureRectangleName);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindTexture);

    // Load a stub 2 X 2 texture into it:
    GLubyte stubTextureData[2][2][3] = {{{0, 0, 255}, {0, 255, 0}},
        {{255, 0, 0}, {255, 255, 0}}
    };
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glTexImage2D);
    gs_stat_realFunctionPointers.glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB8, 2, 2, 0,
                                              GL_RGB, GL_UNSIGNED_BYTE, stubTextureData);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glTexImage2D);

    // Set the stub texture parameters:
    setStubTextureParameters(GL_TEXTURE_RECTANGLE_ARB);

    // If we generated OpenGL errors:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
    curOpenGLErr = gs_stat_realFunctionPointers.glGetError();
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

    if (curOpenGLErr != GL_NO_ERROR)
    {
        // Failure clean up:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDeleteTextures);
        gs_stat_realFunctionPointers.glDeleteTextures(1, &_stubTextureRectangleName);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDeleteTextures);
        _stubTextureRectangleName = 0;
    }

    // Set the bind texture to be the default texture name:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindTexture);
    gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindTexture);
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::setOpenGLPixelUnPackParameters
// Description: Sets the OpenGL pixel "unpack" parameters to enable us copy
//              the Free image bitmap pixels into OpenGL.
// Arguments:   bytesPerPixel - The amount of bytes per pixel.
// Author:      Yaki Tebeka
// Date:        2/1/2005
// ---------------------------------------------------------------------------
void gsTexturesMonitor::setOpenGLPixelUnPackParameters()
{
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glPixelStorei);
    gs_stat_realFunctionPointers.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glPixelStorei);
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::restoreOpenGLPixelUnPackParameters
// Description:
//  Restores the OpenGL pixel "unpack" parameters.
//  I.E: Removes the effect of setOpenGLPixelPackParameters()
// Author:      Yaki Tebeka
// Date:        2/1/2005
// Implementation notes:
//   We only need to restore to the OpenGL defined initial value, since we are called
//   right after the context is created, and before issuing any OpenGL command).
// ---------------------------------------------------------------------------
void gsTexturesMonitor::restoreOpenGLPixelUnPackParameters()
{
    // Restore the client pixel "unpack" parameters to the OpenGL defined initial value:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glPixelStorei);
    gs_stat_realFunctionPointers.glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glPixelStorei);
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::setStubTextureParameters
// Description: Sets a stub texture parameters
// Arguments: bindTarget - The stub texture bind target.
// Author:      Yaki Tebeka
// Date:        2/3/2005
// ---------------------------------------------------------------------------
void gsTexturesMonitor::setStubTextureParameters(GLenum bindTarget)
{
    // Set the texture wrap parameters to GL_REPEAT:
    // (GL_TEXTURE_RECTANGLE_ARB does not support GL_REPEAT, so we will use GL_CLAMP instead):
    GLenum wrapMode = GL_REPEAT;

    if (bindTarget == GL_TEXTURE_RECTANGLE_ARB)
    {
        wrapMode = GL_CLAMP;
    }

    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glTexParameteri);
    gs_stat_realFunctionPointers.glTexParameteri(bindTarget, GL_TEXTURE_WRAP_S, wrapMode);

    if (bindTarget != GL_TEXTURE_1D)
    {
        gs_stat_realFunctionPointers.glTexParameteri(bindTarget, GL_TEXTURE_WRAP_T, wrapMode);
    }

    if (bindTarget == GL_TEXTURE_3D)
    {
        gs_stat_realFunctionPointers.glTexParameteri(bindTarget, GL_TEXTURE_WRAP_R, GL_REPEAT);
    }

    // Set the texture mag and min filters:
    gs_stat_realFunctionPointers.glTexParameteri(bindTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gs_stat_realFunctionPointers.glTexParameteri(bindTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glTexParameteri);
}

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::updateTextureRawData
// Description: Updates a given texture raw data file.
// Arguments:   textureObj - The texture object to be updated.
// Return Val:  bool  - Success / failure.
// Author:      Eran Zinman
// Date:        1/12/2007
// ---------------------------------------------------------------------------
bool gsTexturesMonitor::updateTextureRawData(apGLTextureMipLevelID textureId)
{
    bool retVal = false;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
#define _glActiveTexture gs_stat_realFunctionPointers.glActiveTexture
#define _glClientActiveTexture gs_stat_realFunctionPointers.glClientActiveTexture
#endif

    // Get the texture object details:
    gsGLTexture* pTextureObj = getTextureObjectDetails(textureId._textureName);
    GT_IF_WITH_ASSERT(pTextureObj != NULL)
    {
        retVal = true;

        if (pTextureObj->dirtyTextureRawDataExists(textureId._textureMipLevel))
        {
            // If this is a GL-CL interop texture, print a warning message to the log, since we might
            // run into undefined behaviour if the texture is acquired by OpenCL:
            int clImageIndex = -1;
            int clImageName = -1;
            int clContextId = -1;
            pTextureObj->getCLImageDetails(clImageIndex, clImageName, clContextId);

            if (clImageName > -1)
            {
                OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_aboutToReadCLObject, OS_DEBUG_LOG_INFO);
            }

            // Will get the texture unit on which we will work:
            GLenum textureUnitName = GL_TEXTURE0;

            // Get the texture object name:
            GLuint textureName = pTextureObj->textureName();

            // If this is a "default texture":
            bool isDefaultTexture = apIsDefaultTextureName(textureName);

            if (isDefaultTexture)
            {
                // Get the texture unit in which it is the default texture resides:
                apTextureType texType = AP_UNKNOWN_TEXTURE_TYPE;
                bool rc = apGetDefaultTextureUnitAndType(textureName, textureUnitName, texType);
                GT_ASSERT(rc);

                // Set the texture name to be 0 (this is a "default texture"):
                textureName = 0;
            }

            apGLTextureMipLevel* pTextureMipLevel = pTextureObj->getTextureLogicMipLevel(textureId._textureMipLevel);
            GT_IF_WITH_ASSERT(pTextureMipLevel != NULL)
            {
                // Iterate the texture images that needs to be updated:
                const gtVector<GLenum>& dirtyTextureImages = pTextureMipLevel->getDirtyTextureData();
                gtVector<GLenum>::const_iterator iter = dirtyTextureImages.begin();
                gtVector<GLenum>::const_iterator endIter = dirtyTextureImages.end();

                while (iter != endIter)
                {
                    // Get the bind target that represents that image that needs to be updated:
                    _currentlyBoundTarget = *iter;

                    // Make textureUnitName the active texture unit:
                    GLenum currentlyActiveTextureUnit = GL_TEXTURE0;

                    if (_glActiveTexture != NULL)
                    {
                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
                        gs_stat_realFunctionPointers.glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)(&currentlyActiveTextureUnit));
                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glActiveTexture);
                        _glActiveTexture(textureUnitName);
                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glActiveTexture);
                    }

                    // Client states are not supported on OpenGL ES 2.0 and OpenGL 3.1+:
                    GLenum currentlyClientActiveTextureUnit = GL_TEXTURE0;

                    if (_areClientStatesSupported)
                    {
                        if (_glClientActiveTexture != NULL)
                        {
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
                            gs_stat_realFunctionPointers.glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE, (GLint*)(&currentlyClientActiveTextureUnit));
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glClientActiveTexture);
                            _glClientActiveTexture(textureUnitName);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glClientActiveTexture);
                        }
                    }

                    // We distinguish between bind target that we use in glBindTexture and query data
                    // target that we use in glGet commends (glGetTexImage, etc). (This is important when
                    // using cube map textures).
                    GLenum queryDataTarget = _currentlyBoundTarget;
                    apTextureType textureType = apTextureBindTargetToTextureType(_currentlyBoundTarget);

                    if (textureType == AP_CUBE_MAP_TEXTURE)
                    {
                        _currentlyBoundTarget = GL_TEXTURE_CUBE_MAP;
                    }

                    // Bind the texture before update:
                    bindTextureForUpdate(textureName, _currentlyBoundTarget);

                    // Generate the texture file name:
                    osFilePath textureFilePath;
                    generateTextureMiplevelFilePath(textureId, queryDataTarget, textureFilePath);

                    // Save the texture raw data to a file:
                    gsTextureSerializer textureSerializer(queryDataTarget, textureId._textureMipLevel);
                    bool rc1 = textureSerializer.saveRawDataToFile(pTextureObj, textureFilePath);

                    if (rc1)
                    {
                        // Store the updated texture raw data file:
                        pTextureObj->updateTextureDataFile(queryDataTarget, textureFilePath, textureId._textureMipLevel);
                    }
                    else
                    {
                        // Raise an assertion failure:
                        gtString errorMessage = GS_STR_FailedToSaveTextureImage;
                        errorMessage += textureFilePath.asString();
                        GT_ASSERT_EX(false, errorMessage.asCharArray());
                    }

                    retVal = retVal && rc1;

                    // Restore the texture that was bound to this bind target, if the one we update is
                    // different than the currently binded one:
                    restoreBindedTextureAfterUpdate();

                    // Restore the active texture units:
                    if (_glActiveTexture != NULL)
                    {
                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glActiveTexture);
                        _glActiveTexture(currentlyActiveTextureUnit);
                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glActiveTexture);
                    }

                    // Client states are not supported on OpenGL ES 2.0 and OpenGL 3.1+:
                    if (_areClientStatesSupported)
                    {
                        if (_glClientActiveTexture != NULL)
                        {
                            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glClientActiveTexture);
                            _glClientActiveTexture(currentlyClientActiveTextureUnit);
                            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glClientActiveTexture);
                        }
                    }

                    // Next "dirty" image:
                    iter++;
                }
            }

            // If operation was successful
            if (retVal)
            {
                // If the texture is bound to the active FBO we cannot know when the texture image is becoming dirty again:
                if (!pTextureObj->isTextureBoundToActiveFBO())
                {
                    // Mark all textures raw data as updated:
                    pTextureObj->markAllTextureRawDataAsUpdated(textureId._textureMipLevel);
                }
            }
        }
    }

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
#undef _glActiveTexture
#undef _glClientActiveTexture
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::generateTextureElementsPaths
// Description: Generates a texture elements file path (Without saving
//              any data).
// Arguments:   pTextureObj - Texture object to update it's elements paths
// Author:      Eran Zinman
// Date:        1/2/2008
// ---------------------------------------------------------------------------
bool gsTexturesMonitor::generateTextureElementsPaths(gsGLTexture* pTextureObj) const
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(pTextureObj != NULL)
    {
        retVal = true;

        // It is possible that file paths wasn't generated only for items that are marked as "dirty"
        if (pTextureObj->dirtyTextureRawDataExists(0))
        {
            // Will be used for the texture file name:
            apGLTextureMipLevelID textureMipLevelId;
            textureMipLevelId._textureName = pTextureObj->textureName();
            textureMipLevelId._textureMipLevel = 0;

            // Get the requested texture mip level:
            apGLTextureMipLevel* pTextureMipLevel = pTextureObj->getTextureLogicMipLevel(0);

            if (pTextureMipLevel != NULL)
            {
                // Iterate the texture files that needs to be updated:
                const gtVector<GLenum>& dirtyTextureImages = pTextureMipLevel->getDirtyTextureData();
                gtVector<GLenum>::const_iterator iter = dirtyTextureImages.begin();
                gtVector<GLenum>::const_iterator endIter = dirtyTextureImages.end();

                while (iter != endIter)
                {
                    // We distinguish between bind target that we use in glBindTexture and query data
                    // target that we use in glGet commends (glGetTexImage, etc). (This is important when
                    // using cube map textures).
                    GLenum queryDataTarget = *iter;

                    // Generate the texture file name:
                    osFilePath textureFilePath;
                    generateTextureMiplevelFilePath(textureMipLevelId, queryDataTarget, textureFilePath);

                    // Store the updated texture raw data file:
                    pTextureObj->updateTextureDataFile(queryDataTarget, textureFilePath, textureMipLevelId._textureMipLevel);

                    // Next "dirty" image:
                    iter++;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::getCurrentlyBoundTextureObjectDetails
// Description: Gets the gsGLTexture item that represents the texture currently bound to target
// Author:      Uri Shomroni
// Date:        11/3/2009
// ---------------------------------------------------------------------------
gsGLTexture* gsTexturesMonitor::getCurrentlyBoundTextureObjectDetails(GLenum target) const
{
    gsGLTexture* retVal = NULL;
    GLuint boundTextureName = getCurrentlyBoundTexture(target);
    GT_IF_WITH_ASSERT(boundTextureName > 0)
    {
        retVal = getTextureObjectDetails(boundTextureName);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::generateTextureMiplevelFilePath
// Description: Generates a texture file path.
// Arguments:   apGLTextureMipLevelID textureMipLevelId - The texture level OpenGL name.
//              textureBindTarget - The texture bind target.
//              textureFilePath - The output texture file path.
// Author:      Yaki Tebeka
// Date:        27/12/2004
// ---------------------------------------------------------------------------
void gsTexturesMonitor::generateTextureMiplevelFilePath(apGLTextureMipLevelID textureMipLevelId, GLenum textureBindTarget, osFilePath& textureFilePath) const
{
    // Get the bind target as string:
    gtString bindTargetAsString;
    textureBindTargetToString(textureBindTarget, bindTargetAsString);

    // Build the log file name:
    gtString logFileName;
    logFileName.appendFormattedString(GS_STR_textureFilePath, _spyContextId, textureMipLevelId._textureName, textureMipLevelId._textureMipLevel, bindTargetAsString.asCharArray());

    // Set the log file path:
    textureFilePath = suCurrentSessionLogFilesDirectory();
    textureFilePath.setFileName(logFileName);

    // Set the log file extension:
    textureFilePath.setFileExtension(SU_STR_rawFileExtension);
}

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::textureBindTargetToString
// Description: Translates an OpenGL bind target to a string.
// Author:      Yaki Tebeka
// Date:        27/12/2004
// ---------------------------------------------------------------------------
void gsTexturesMonitor::textureBindTargetToString(GLenum textureBindTarget, gtString& bindTargetAsString) const
{
    bindTargetAsString.makeEmpty();

    switch (textureBindTarget)
    {
        case GL_TEXTURE_1D:
            bindTargetAsString = L"1D";
            break;

        case GL_TEXTURE_2D:
            bindTargetAsString = L"2D";
            break;

        case GL_TEXTURE_3D:
            bindTargetAsString = L"3D";
            break;

        case GL_TEXTURE_1D_ARRAY:
            bindTargetAsString = L"1DArray";
            break;

        case GL_TEXTURE_2D_ARRAY:
            bindTargetAsString = L"2DArray";
            break;

        case GL_TEXTURE_2D_MULTISAMPLE:
            bindTargetAsString = L"MultiSample";
            break;

        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
            bindTargetAsString = L"MultiSampleArray";
            break;

        case GL_TEXTURE_CUBE_MAP:
            bindTargetAsString = L"CubeMap";
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            bindTargetAsString = L"CubeMapPosX";
            break;

        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            bindTargetAsString = L"CubeMapNegX";
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            bindTargetAsString = L"CubeMapPosY";
            break;

        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            bindTargetAsString = L"CubeMapNegY";
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            bindTargetAsString = L"CubeMapPosZ";
            break;

        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            bindTargetAsString = L"CubeMapNegZ";
            break;

        case GL_TEXTURE_CUBE_MAP_ARRAY:
            bindTargetAsString = L"CubeMapArray";
            break;

        case GL_TEXTURE_RECTANGLE_ARB:
            bindTargetAsString = L"TextureRectangle";
            break;
    }
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::getCurrentlyBoundTexture
// Description: Inputs a bind target and returns the name of the texture that
//              is currently bound to it.
// Author:      Yaki Tebeka
// Date:        3/7/2006
// ---------------------------------------------------------------------------
GLuint gsTexturesMonitor::getCurrentlyBoundTexture(GLenum bindTarget) const
{
    GLuint retVal = 0;

    // Get the enumerator that queries the input bind target bound texture:
    GLenum boundTextureQueryEnumerator = 0;

    switch (bindTarget)
    {
        case GL_TEXTURE_1D:
            boundTextureQueryEnumerator = GL_TEXTURE_BINDING_1D;
            break;

        case GL_TEXTURE_2D:
            boundTextureQueryEnumerator = GL_TEXTURE_BINDING_2D;
            break;

        case GL_TEXTURE_3D:
            boundTextureQueryEnumerator = GL_TEXTURE_BINDING_3D;
            break;

        case GL_TEXTURE_1D_ARRAY:
            boundTextureQueryEnumerator = GL_TEXTURE_BINDING_1D_ARRAY;
            break;

        case GL_TEXTURE_2D_ARRAY:
            boundTextureQueryEnumerator = GL_TEXTURE_BINDING_2D_ARRAY;
            break;

        case GL_TEXTURE_2D_MULTISAMPLE:
            boundTextureQueryEnumerator = GL_TEXTURE_BINDING_2D_MULTISAMPLE;
            break;

        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
            boundTextureQueryEnumerator = GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY;
            break;

        case GL_TEXTURE_CUBE_MAP:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        {
            boundTextureQueryEnumerator = GL_TEXTURE_BINDING_CUBE_MAP;
            break;
        }

        case GL_TEXTURE_RECTANGLE:
            boundTextureQueryEnumerator = GL_TEXTURE_BINDING_RECTANGLE;
            break;

        case GL_TEXTURE_CUBE_MAP_ARRAY:
            boundTextureQueryEnumerator = GL_TEXTURE_BINDING_CUBE_MAP_ARRAY;
            break;

        default:
        {
            // Unknown bind target:
            gtString errorMsg;
            apGLenumValueToString(bindTarget, errorMsg);
            errorMsg.prepend(GS_STR_UnknownBindTarget);
            GT_ASSERT_EX(false, errorMsg.asCharArray());
            break;
        }
    }

    if (boundTextureQueryEnumerator != 0)
    {
        // Get the name of the texture that is bound to the input bind target:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
        gs_stat_realFunctionPointers.glGetIntegerv(boundTextureQueryEnumerator, (GLint*)&retVal);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::loggedTextureFileType
// Description: Inputs a bind target and outputs the file type that should be
//              used for logging its textures.
// Author:      Yaki Tebeka
// Date:        18/1/2005
// ---------------------------------------------------------------------------
apFileType gsTexturesMonitor::loggedTextureFileType(GLenum bindTarget) const
{
    apFileType retVal = AP_JPEG_FILE;

    // Get the texture type:
    apTextureType textureType = apTextureBindTargetToTextureType(bindTarget);

    // 3D textures are logged using TIFF format:
    // TO_DO: multisample texture
    if ((textureType == AP_3D_TEXTURE) || (textureType == AP_1D_ARRAY_TEXTURE) || (textureType == AP_2D_ARRAY_TEXTURE))
    {
        retVal = AP_TIFF_FILE;
    }
    else
    {
        // Get the user defined logged texture file type:
        retVal = suLoggedTexturesFileType();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::updateTexturesHardwareLimits
// Description: Updated the hardware texture limits.
// Author:      Yaki Tebeka
// Date:        18/6/2005
// ---------------------------------------------------------------------------
void gsTexturesMonitor::updateTexturesHardwareLimits()
{
    // Clear OpenGL errors (if any):
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
    GLenum curGLError = gs_stat_realFunctionPointers.glGetError();
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);
    GT_ASSERT(curGLError == GL_NO_ERROR);

    // Get my render context monitor:
    const gsOpenGLMonitor& theOpenGLMonitor = gsOpenGLMonitor::instance();
    const gsRenderContextMonitor* pMyRenderCtxMtr = theOpenGLMonitor.renderContextMonitor(_spyContextId);
    GT_IF_WITH_ASSERT(pMyRenderCtxMtr != NULL)
    {
        // Support by OGL version:
        // ----------------------

        int majorVersion = 0;
        int minorVersion = 0;
        pMyRenderCtxMtr->getOpenGLVersion(majorVersion, minorVersion);

#ifdef _GR_IPHONE_BUILD
        // OpenGL ES supports multitextures and doesn't support 3D and rectangle textures:
        _areMultiTexturesSupported = true;
        _are3DTexturesSupported = false;
        _isTextureRectangleSupported = false;

        if (majorVersion >= 2)
        {
            // OpenGL ES 2.0 and higher supports cube map textures:
            _areCubeMapTexturesSupported = true;

            // Also, to query the maximal number of texture units, there is a different
            // GLenum to be used in glGetIntegerv's pname parameter:
            _maxTextureUnitsSymbolicName = GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS;
        }
        else
        {
            // OpenGL ES 1.1 and lower supports glClientActiveTexture:
            _areClientStatesSupported = true;
        }

#else

        // 3D textures are supported by OpenGL 1.2 or higher:
        if (((1 == majorVersion) && (3 <=  minorVersion)) || (2 <= majorVersion))
        {
            _are3DTexturesSupported = true;
        }

        // Cube map textures and multi textures are supported by OpenGL 1.3 or higher:
        if (((1 == majorVersion) && (3 <= minorVersion)) || (2 <= majorVersion))
        {
            _areCubeMapTexturesSupported = true;
            _areMultiTexturesSupported = true;
        }

        // Client states are supported by all OpenGL version up to 3.0 (for glClientActiveTexture, we further
        // check for a function pointer):
        if ((majorVersion <= 2) || ((majorVersion == 3) && (minorVersion == 0)))
        {
            _areClientStatesSupported = true;
        }

        // Support by OGL extension:
        // ------------------------

        // Get the extensions manager:
        gsExtensionsManager& theExtensionsMgr = gsExtensionsManager::instance();

        // Get my render context spy id:
        int myRenderCtxSpyId = pMyRenderCtxMtr->spyId();


        // 3D textures may also be supported by an extension:
        if (!_are3DTexturesSupported)
        {
            _are3DTexturesSupported = theExtensionsMgr.isExtensionSupported(myRenderCtxSpyId, AP_GL_EXT_texture3D);
        }

        // If 3D textures are supported:
        if (_are3DTexturesSupported)
        {
            // Get a pointer to glTexImage3D or its Ext equivalent:
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            _glTexImage3D = (PFNGLTEXIMAGE3DPROC)gsGetSystemsOGLModuleProcAddress("glTexImage3D");

            if (_glTexImage3D == NULL)
            {
                _glTexImage3D = (PFNGLTEXIMAGE3DPROC)gsGetSystemsOGLModuleProcAddress("glTexImage3DEXT");
            }

            // Sanity check:
            if (_glTexImage3D == NULL)
            {
                _are3DTexturesSupported = false;
                GT_ASSERT(false);
            }

#endif
        }

        // Cube map textures may also be supported by an extension:
        if (!_areCubeMapTexturesSupported)
        {
            _areCubeMapTexturesSupported = theExtensionsMgr.isExtensionSupported(myRenderCtxSpyId, AP_GL_ARB_texture_cube_map);
        }

        // Multi textures may also be supported by an extension:
        if (!_areMultiTexturesSupported)
        {
            _areMultiTexturesSupported = theExtensionsMgr.isExtensionSupported(myRenderCtxSpyId, AP_GL_ARB_multitexture);
        }

        // Texture rectangle may also be supported by an extension:
        if (!_isTextureRectangleSupported)
        {
            _isTextureRectangleSupported = theExtensionsMgr.isExtensionSupported(myRenderCtxSpyId, AP_GL_ARB_texture_rectangle);
        }

        // If multi textures are supported:
        if (_areMultiTexturesSupported)
        {
            // Get pointer to glActiveTexture or glActiveTextureARB:
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            _glActiveTexture = (PFNGLACTIVETEXTUREPROC)gsGetSystemsOGLModuleProcAddress("glActiveTexture");

            if (_glActiveTexture == NULL)
            {
                _glActiveTexture = (PFNGLACTIVETEXTUREPROC)gsGetSystemsOGLModuleProcAddress("glActiveTextureARB");
            }

            // Get pointer to glClientActiveTexture or glClientActiveTextureARB:
            _glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)gsGetSystemsOGLModuleProcAddress("glClientActiveTexture");

            if (_glClientActiveTexture == NULL)
            {
                _glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)gsGetSystemsOGLModuleProcAddress("glClientActiveTextureARB");
            }

#endif
        }

#endif
    }
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::createTextureObjectMonitor
// Description: Creates a texture object monitor, initializes it and registeres
//              it in this class vectors and maps.
//
// Arguments: textureObjectName - The OpenGL name of the texture that the
//                                created texture monitor will monitor.
//
// Return Val: gsGLTexture*  - The created texture monitor (Or NULL in case of failure).
//
// Author:      Yaki Tebeka
// Date:        11/4/2006
// ---------------------------------------------------------------------------
gsGLTexture* gsTexturesMonitor::createTextureObjectMonitor(GLuint textureObjectName)
{
    gsGLTexture* retVal = NULL;

    // Create the texture object monitor:
    gsGLTexture* pNewTextureMtr = new gsGLTexture(textureObjectName);
    GT_IF_WITH_ASSERT(pNewTextureMtr != NULL)
    {
        // Add the texture default parameters:
        addDefaultTextureParameters(*pNewTextureMtr);

        // Add the texture object to the _textures vector:
        // ----------------------------------------------

        // Will contain the texture index in the _textures vector:
        int newTextureIndex = 0;

        // There is currently no free index - we will allocate a new index:
        newTextureIndex = (int)_textures.size();
        _textures.push_back(pNewTextureMtr);

        // Add the monitor index to the _textureOpenGLNameToIndex map:
        _textureOpenGLNameToIndex[textureObjectName] = newTextureIndex;

        // Increment the allocated textures amount:
        _amountOfTextureObjects++;

        // Return the created texture object:
        retVal = pNewTextureMtr;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::addDefaultTextureParameters
// Description: Adds default OpenGL / ES texture parameters (and values) into
//              an input texture monitor object.
// Arguments: textureMonitor - The input texture monitor object.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/4/2006
// ---------------------------------------------------------------------------
void gsTexturesMonitor::addDefaultTextureParameters(gsGLTexture& textureMonitor)
{
    // If we are building an OpenGL ES spy:
#if (defined OS_OGL_ES_IMPLEMENTATION_DLL_BUILD) || (defined _GR_IPHONE_BUILD)

    // Add OpenGL ES default texture parameters:
    textureMonitor.addDefaultGLESTextureParameters();

#else

    // Add OpenGL default texture parameters:
    textureMonitor.addDefaultGLTextureParameters();

#endif

}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::onLevel0TexelsAddition
// Description: Is called when level 0 texels are added. Updates the amount
//              of loaded level 0 texels.
// Arguments:   width, height, depth - The loaded texture image size.
//              border - The texture image border size.
// Author:      Yaki Tebeka
// Date:        12/9/2005
// ---------------------------------------------------------------------------
void gsTexturesMonitor::onLevel0TexelsAddition(GLsizei width, GLsizei height, GLsizei depth, GLint border)
{
    // Calculate the amount of added texels:
    double amountOfAddedTexels = amountOfTextureImageTexels(width, height, depth, border);

    // Update the amount of loaded texels:
    _amountOfLoadedLevel0Texels += amountOfAddedTexels;
}



// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::onLevel0TexelsRemoval
// Description: Is called when level 0 texels are removed. Updates the amount
//              of loaded level 0 texels.
// Arguments:   width, height, depth - The loaded texture image size.
//              border - The texture image border size.
// Author:      Yaki Tebeka
// Date:        12/9/2005
// Usage Sample:
// Implementation Notes:
// History:
// ---------------------------------------------------------------------------
void gsTexturesMonitor::onLevel0TexelsRemoval(GLsizei width, GLsizei height, GLsizei depth, GLint border)
{
    // Calculate the amount of removed texels:
    double amountOfRemovedTexels = amountOfTextureImageTexels(width, height, depth, border);

    // Update the amount of loaded texels:
    _amountOfLoadedLevel0Texels -= amountOfRemovedTexels;
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::amountOfTextureImageTexels
// Description: Inputs a texture image getDimensions and returns the amount of texels it uses.
//
// Arguments: width, height, depth - The texture image size.
//            border - The texture image border size.
// Return Val: double - Will return the amount of texels that are used for holding the
//                      input image.
// Author:      Yaki Tebeka
// Date:        4/5/2006
// ---------------------------------------------------------------------------
double gsTexturesMonitor::amountOfTextureImageTexels(GLsizei width, GLsizei height, GLsizei depth, GLint border) const
{
    (void)(border); // unused
    double retVal = 0;

    // Replace 0 sizes by 1:
    if (height <= 0)
    {
        height = 1;
    }

    if (depth <= 0)
    {
        depth = 1;
    }

    // Calculate the amount of texture image texels:
    retVal = width * height * depth;
    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::bindTextureForUpdate
// Description: Bind requested texture for uppdate actions
//            GLuint textureName - the texture name
// Author:      Sigal Algranaty
// Date:        3/11/2008
// ---------------------------------------------------------------------------
void gsTexturesMonitor::bindTextureForUpdate(GLuint textureName, GLenum bindTarget)
{
    // Get the texture currently bound to this bind target:
    _currentlyBoundTarget = bindTarget;
    _currentlyBoundTextureName = getCurrentlyBoundTexture(_currentlyBoundTarget);

    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindTexture);
    gs_stat_realFunctionPointers.glBindTexture(_currentlyBoundTarget, textureName);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindTexture);
}

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::restoreBindedTextureAfterUpdate
// Description: Restore texture target bind after texture update operation
// Return Val: void
// Author:      Sigal Algranaty
// Date:        3/11/2008
// ---------------------------------------------------------------------------
void gsTexturesMonitor::restoreBindedTextureAfterUpdate()
{
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindTexture);
    gs_stat_realFunctionPointers.glBindTexture(_currentlyBoundTarget, _currentlyBoundTextureName);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindTexture);
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::markTextureParametersAsNonUpdated
// Description: The function mark all the textures in the textures monitor
//              as not updated
// Return Val: void
// Author:      Sigal Algranaty
// Date:        30/11/2008
// ---------------------------------------------------------------------------
void gsTexturesMonitor::markTextureParametersAsNonUpdated()
{
    for (int i = 0; i < (int)_textures.size(); i++)
    {
        // Get the current texture:
        gsGLTexture* pTexture = _textures[i];

        // Mark the texture parameters as not updated:
        pTexture->markAllParametersAsUpdated(false);

    }
}

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::updateTextureIndicesAfterDeletion
// Description: Updates textures indices after deletion (moves each texture after the deleted
// Arguments: int deletedTextureIndex
//            GLuint textureName
// Return Val: void
// Author:      Sigal Algranaty
// Date:        9/11/2008
// ---------------------------------------------------------------------------
void gsTexturesMonitor::updateTextureIndicesAfterDeletion(int deletedTextureIndex, GLuint textureName)
{
    // Remove this texture from the map:
    gtMap<GLuint, int>::const_iterator endIter = _textureOpenGLNameToIndex.end();
    gtMap<GLuint, int>::iterator iter = _textureOpenGLNameToIndex.find(textureName);

    if (iter != endIter)
    {
        _textureOpenGLNameToIndex.erase(iter);
    }

    int lastTextureIndex = (int)_textures.size() - 1;

    // Iterate from the deleted texture till the last one:
    for (int i = deletedTextureIndex; i < lastTextureIndex; i++)
    {
        // Get the current moved texture:
        gsGLTexture* pMovedTexture = _textures[i + 1];
        GT_IF_WITH_ASSERT(pMovedTexture != NULL)
        {
            // Move the texture one place left:
            _textures[i] = pMovedTexture;

            // Get the texture name:
            textureName = pMovedTexture->textureName();

            // Set the new texture index in map:
            _textureOpenGLNameToIndex[textureName] = i;
        }
    }

    // Pop the last texture:
    _textures.pop_back();
}

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::amountOfTexBufferObjects
// Description: Amount of texture buffer objects
// Return Val: int
// Author:      Sigal Algranaty
// Date:        2/8/2009
// ---------------------------------------------------------------------------
int gsTexturesMonitor::amountOfTexBufferObjects() const
{
    int retVal = 0;

    for (int i = 0; i < (int)_textures.size(); i++)
    {
        // Get the current texture:
        gsGLTexture* pTexture = _textures[i];

        // Update the texture parameters:
        GT_IF_WITH_ASSERT(pTexture != NULL)
        {
            if (pTexture->textureType() == AP_BUFFER_TEXTURE)
            {
                retVal ++;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::calculateTexturesMemorySize
// Description: Calculate the current existing textures memory size
// Arguments:   gtUInt64& texturesMemorySize
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/7/2010
// ---------------------------------------------------------------------------
bool gsTexturesMonitor::calculateTexturesMemorySize(gtUInt64& texturesMemorySize) const
{
    bool retVal = true;
    texturesMemorySize = 0;

    // Iterate the current existing textures, and calculate their memory size:
    for (int i = 0; i < (int) _textures.size(); i++)
    {
        // Get the current texture:
        gsGLTexture* pTextureObject = _textures[i];

        if (pTextureObject != NULL)
        {
            // Get the calculated texture object size (all miplevels sizes):
            bool isMemoryEstimated = false;
            gtSize_t currentTextureMemorySize = 0;
            bool rc = pTextureObject->getMemorySize(currentTextureMemorySize, isMemoryEstimated);
            GT_IF_WITH_ASSERT(rc)
            {
                // Set the object size in KB (ceil the number for 0.1KB to turn to be 1KB):
                if (currentTextureMemorySize)
                {
                    float val = (float)currentTextureMemorySize / (float)(1024 * 8);
                    currentTextureMemorySize = (gtUInt32)ceil(val);

                    if (currentTextureMemorySize == 0)
                    {
                        currentTextureMemorySize = 1;
                    }
                }

                // Add the current texture memory size:
                texturesMemorySize += currentTextureMemorySize;
            }

            retVal = retVal && rc;
        }
    }

    return retVal;
}
