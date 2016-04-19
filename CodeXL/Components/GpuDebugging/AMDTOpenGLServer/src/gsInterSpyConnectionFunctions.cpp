//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsInterSpyConnectionFunctions.cpp
///
//==================================================================================

//------------------------------ gsInterSpyConnectionFunctions.cpp------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/apDefinitions.h>

// Local:
#include <src/gsOpenGLMonitor.h>

// TO_DO: gl-cl interp:
#if defined(_WIN32)
#define GS_API __declspec(dllexport)
// On Windows, exporting the functions as C functions is done via the .def file.
#else
#define GS_API

// On Linux, export the functions as C:
extern "C"
{
    bool GS_API gsShareGLContextWithCLContext(oaOpenGLRenderContextHandle glRenderContextOSHandle, int clContextID, int& glContextID);
    bool GS_API gsShareGLTextureWithCLImage(int clImageIndex, int clImageName, int clSpyID, int glSpyID, GLuint glTextureName, int textureMipLevel, GLenum textureTarget, gtString& detectedErrorStr);
    bool GS_API gsShareGLRenderBufferWithCLImage(int clImageIndex, int clImageName, int clSpyID, int glSpyID, GLuint glRenderBufferName, gtString& detectedErrorStr);
    bool GS_API gsShareGLVBOWithCLBuffer(int clBufferIndex, int clBufferName, int clSpyID, int glSpyID, GLuint glVBOName, gtString& detectedErrorStr);
};
#endif

// ---------------------------------------------------------------------------
// Name:        gsShareGLContextWithCLContext
// Description: Inform the OpenGL spy with the sharing of an OpenCL context with
//              an OpenGL context
// Arguments:   oaOpenGLRenderContextHandle glRenderContextOSHandle
//              int clContextID - the OpenCL context id
//              int& glContextID - the OpenGL context (output)
// Return Val:  bool GS_API - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/7/2010
// ---------------------------------------------------------------------------
bool GS_API gsShareGLContextWithCLContext(oaOpenGLRenderContextHandle glRenderContextOSHandle, int clContextID, int& glContextID)
{
    bool retVal = false;

    // Get the render context spy id matching the input render context handle:
    glContextID = gsOpenGLMonitor::instance().renderContextSpyId(glRenderContextOSHandle);
    GT_IF_WITH_ASSERT(glContextID > 0)
    {
        // Get the OpenGL render context monitor:
        gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(glContextID);
        GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
        {
            // Set the OpenCL spy id:
            pRenderContextMonitor->setOpenCLSharedContextID(clContextID);
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsShareGLTextureWithCLImage
// Description: Inform the OpenGL spy with the sharing of an OpenCL image with
//              an OpenGL texture
// Arguments:   int clImageIndex - the OpenCL image index
//              int clImageName - the OpenCL image name
//              int clSpyID - the OpenCL spy id
//              int glSpyID - the OpenGL spy id
//              GLuint glTextureName - the texture name
//              int textureMipLevel - the OpenGL texture mip level
//              GLenum textureTarget - the OpenGL texture target
//              gtString& detectedError - contain a detected error if there is one.
// Return Val:  bool GS_API - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/7/2010
// ---------------------------------------------------------------------------
bool GS_API gsShareGLTextureWithCLImage(int clImageIndex, int clImageName, int clSpyID, int glSpyID, GLuint glTextureName, int textureMipLevel, GLenum textureTarget, gtString& detectedErrorStr)
{
    bool retVal = false;

    // Get the render context monitor for the requested spy ID:
    GT_IF_WITH_ASSERT(glSpyID > 0)
    {
        gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(glSpyID);
        GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
        {
            // Get the OpenGL textures monitor:
            gsTexturesMonitor* pTexturesMonitor = pRenderContextMonitor->texturesMonitor();
            GT_IF_WITH_ASSERT(pTexturesMonitor != NULL)
            {
                // Get the requested texture object:
                gsGLTexture* pTextureObject = pTexturesMonitor->getTextureObjectDetails(glTextureName);

                if (pTextureObject != NULL)
                {
                    // Make sure that the texture target matches to texture type:
                    apTextureType textureType = apTextureBindTargetToTextureType(textureTarget);

                    if (textureType != pTextureObject->textureType())
                    {
                        // Add detected error:
                        detectedErrorStr = L"Trying to share an OpenGL texture with other target";
                    }
                    else
                    {
                        // Share the texture object:
                        retVal = pTextureObject->shareTextureMiplevelWithCLImage(clImageIndex, clImageName, clSpyID, textureMipLevel);
                    }
                }
                else
                {
                    // Add detected error:
                    detectedErrorStr = L"The shared OpenGL texture does not exist";
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsShareGLRenderBufferWithCLImage
// Description: Inform the OpenGL spy with the sharing of an OpenCL image with
//              an OpenGL render buffer
// Arguments:   int clImageIndex - the OpenCL image index
//              int clImageName - the OpenCL image name
//              int clSpyID - the OpenCL spy id
//              int glSpyID - the OpenGL spy id
//              GLuint glRenderBufferName - the render buffer name
//              gtString& detectedError - contain a detected error if there is one.
// Return Val:  bool GS_API - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/7/2010
// ---------------------------------------------------------------------------
bool GS_API gsShareGLRenderBufferWithCLImage(int clImageIndex, int clImageName, int clSpyID, int glSpyID, GLuint glRenderBufferName, gtString& detectedErrorStr)
{
    bool retVal = false;

    // Get the render context monitor for the requested spy ID:
    GT_IF_WITH_ASSERT(glSpyID > 0)
    {
        gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(glSpyID);
        GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
        {
            // Get the OpenGL RBO monitor:
            gsRenderBuffersMonitor* pBuffersMonitor = pRenderContextMonitor->renderBuffersMonitor();
            GT_IF_WITH_ASSERT(pBuffersMonitor  != NULL)
            {
                // Get the requested render buffer object:
                apGLRenderBuffer* pRenderBufferObject = pBuffersMonitor->getRenderBufferObjectDetails(glRenderBufferName);

                if (pRenderBufferObject != NULL)
                {
                    // Share the render buffer object:
                    pRenderBufferObject->shareBufferWithCLImage(clImageIndex, clImageName, clSpyID);
                    retVal = true;
                }
                else
                {
                    // Add detected error:
                    detectedErrorStr = L"The shared OpenGL render buffer does not exist";
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsShareGLVBOWithCLBuffer
// Description: Inform the OpenGL spy with the sharing of an OpenCL buffer with
//              an OpenGL VBO
// Arguments:   int clBufferIndex
//            int clSpyID
//            int glSpyID
//            GLuint glVBOName
//            gtString& detectedErrorStr
// Return Val:  bool GS_API - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/7/2010
// ---------------------------------------------------------------------------
bool GS_API gsShareGLVBOWithCLBuffer(int clBufferIndex, int clBufferName, int clSpyID, int glSpyID, GLuint glVBOName, gtString& detectedErrorStr)
{
    bool retVal = false;

    // Get the render context monitor for the requested spy ID:
    GT_IF_WITH_ASSERT(glSpyID > 0)
    {
        gsRenderContextMonitor* pRenderContextMonitor = gsOpenGLMonitor::instance().renderContextMonitor(glSpyID);
        GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
        {
            // Get the OpenGL VBOs monitor:
            gsVBOMonitor* pVBOMonitor = pRenderContextMonitor->vboMonitor();
            GT_IF_WITH_ASSERT(pVBOMonitor != NULL)
            {
                // Get the requested VBO object:
                apGLVBO* pVBO = pVBOMonitor->getVBODetails(glVBOName);

                if (pVBO != NULL)
                {
                    // Share the VBO:
                    pVBO->shareVBOWithCLBuffer(clBufferIndex, clBufferName, clSpyID);
                    retVal = true;
                }
                else
                {
                    // Add detected error:
                    detectedErrorStr = L"The shared OpenGL VBO does not exist";
                }
            }
        }
    }

    return retVal;
}
