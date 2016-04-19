//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsPBuffersMonitor.cpp
///
//==================================================================================

//------------------------------ gsPBuffersMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apDefinitions.h>
#include <AMDTAPIClasses/Include/apOpenGLParameters.h>

// Spies Utilities:
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Local:
#include <src/gsStringConstants.h>
#include <src/gsGlobalVariables.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsExtensionsManager.h>
#include <src/gsRenderContextMonitor.h>
#include <src/gsTextureSerializer.h>
#include <src/gsPBuffersMonitor.h>
#include <src/gsBufferSerializer.h>

#define GS_PBUFFERS_MONITOR_PBUFFER_NOT_FOUND -1


// ---------------------------------------------------------------------------
// Name:        gsPBuffersMonitor::gsPBuffersMonitor
// Description: Constructor
// Author:      Eran Zinman
// Date:        24/8/2007
// ---------------------------------------------------------------------------
gsPBuffersMonitor::gsPBuffersMonitor()
{
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffersMonitor::~gsPBuffersMonitor
// Description: Destructor
// Author:      Eran Zinman
// Date:        24/8/2007
// ---------------------------------------------------------------------------
gsPBuffersMonitor::~gsPBuffersMonitor()
{
    // Delete the PBuffers wrappers vector:
    int amountOfPBuffers = (int)_pbuffers.size();

    for (int i = 0; i < amountOfPBuffers; i++)
    {
        if (_pbuffers[i])
        {
            delete _pbuffers[i];
            _pbuffers[i] = NULL;
        }
    }

    // Clear the vector entirely
    _pbuffers.clear();
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffersMonitor::addPBufferItem
// Description: Add the new PBuffer object into the PBuffer vector
// Arguments:   pPBufferItem - The PBuffer object to be inserted in the PBuffers
//              vector.
// Author:      Eran Zinman
// Date:        25/8/2007
// ---------------------------------------------------------------------------
void gsPBuffersMonitor::addPBufferItem(gsPBuffer* pPBufferItem)
{
    GT_IF_WITH_ASSERT(pPBufferItem != NULL)
    {
        // Just add the new PBuffer at the end of the vector
        _pbuffers.push_back(pPBufferItem);
    }
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffersMonitor::onPBufferCreation
// Description: Is called when PBuffer is created in the current render context
// Arguments:   pbufferHandler - The PBuffer handler that was received upon PBuffer creation.
//              hDC - specifies a device context for the device on which the PBuffer is created.
//              iPixelFormat - specifies a non-generic pixel format descriptor index.
//              iWidth, iHeight - specify the pixel width and height of the rectangular PBuffer.
//              piAttribList -  is a list of attributes {type, value} pairs containing integer
//              attribute values.
// Author:      Eran Zinman
// Date:        25/8/2007
// ---------------------------------------------------------------------------
void gsPBuffersMonitor::onPBufferCreation(const oaPBufferHandle& pbufferHandler, oaDeviceContextHandle hDC, int iPixelFormat, int iWidth, int iHeight, const int* piAttribList, GLenum target, GLenum format, GLint level)
{
    (void)(hDC); // unused
    (void)(iPixelFormat); // unused
    (void)(piAttribList); // unused
    // Check if the PBuffer was created successfully, by checking it's handler:
    GT_IF_WITH_ASSERT(pbufferHandler != 0)
    {
        // Set the PBuffer ID - which is the position in the PBuffer vector
        int pbufferID = (int)_pbuffers.size();

        // Create the PBuffer object:
        gsPBuffer* pbufferObject = new gsPBuffer(pbufferHandler, pbufferID);
        GT_IF_WITH_ASSERT(pbufferObject != NULL)
        {
            // Set the PBuffer dimensions (width and height) and other properties:
            pbufferObject->setDimensions(iWidth, iHeight);

            apPBuffer::pbufferBindTarget targetAsEnum = apPBuffer::AP_UNDEFINED_PBUFFER;

            switch (target)
            {
                case GL_TEXTURE_2D:
                    targetAsEnum = apPBuffer::AP_2D_PBUFFER;
                    break;

                case GL_TEXTURE_CUBE_MAP:
                    targetAsEnum = apPBuffer::AP_CUBE_MAP_PBUFFER;
                    break;

                case GL_TEXTURE_RECTANGLE_ARB:
                    targetAsEnum = apPBuffer::AP_RECTANGLE_PBUFFER;
                    break;

                case GL_NONE:
                    // This is a pseudoparameter we pass for Linux and Windows PBuffers, do nothing
                    break;

                default:
                {
                    // Unexpected value:
                    gtString errMsg;
                    apGLenumValueToString(target, errMsg);
                    errMsg.prepend(L"Unexpected PBuffer bind target: ");
                    GT_ASSERT_EX(false, errMsg.asCharArray());
                }
                break;
            }

            pbufferObject->setBindTarget(targetAsEnum);

            apPBuffer::pbufferInternalFormat formatAsEnum = apPBuffer::AP_UNDEFINED_FORMAT_PBUFFER;

            switch (format)
            {
                case GL_RGB:
                    formatAsEnum = apPBuffer::AP_RGB_PBUFFER;
                    break;

                case GL_RGBA:
                    formatAsEnum = apPBuffer::AP_RGBA_PBUFFER;
                    break;

                case GL_NONE:
                    // This is a pseudoparameter we pass for Linux and Windows PBuffers, do nothing
                    break;

                default:
                {
                    // Unexpected value:
                    gtString errMsg;
                    apGLenumValueToString(target, errMsg);
                    errMsg.prepend(L"Unexpected PBuffer internal format: ");
                    GT_ASSERT_EX(false, errMsg.asCharArray());
                }
                break;
            }

            pbufferObject->setInternalFormat(formatAsEnum);

            pbufferObject->setMaxMipmapLevel(level);

            // Add the new PBuffer object into the PBuffers vector:
            addPBufferItem(pbufferObject);

            // Register this object in the allocated objects monitor:
            su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pbufferObject);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffersMonitor::onPBufferDeletion
// Description: Is called when PBuffer is being deleted from the current render context
// Arguments:   pbufferHandler - The PBuffer handler that needs to be deleted
// Author:      Eran Zinman
// Date:        25/8/2007
// ---------------------------------------------------------------------------
void gsPBuffersMonitor::onPBufferDeletion(const oaPBufferHandle& pbufferHandler)
{
    // Check if we got a valid PBuffer handler pointer:
    if (pbufferHandler != 0)
    {
        // Get the PBuffer object holding this handle
        int pbufferVectorIndex = getPBufferObject(pbufferHandler);
        GT_IF_WITH_ASSERT(pbufferVectorIndex != GS_PBUFFERS_MONITOR_PBUFFER_NOT_FOUND)
        {
            // Get PBuffer item from the PBuffers vector
            gsPBuffer* pbufferItem = _pbuffers[pbufferVectorIndex];
            GT_IF_WITH_ASSERT(pbufferItem != NULL)
            {
                // Tell the PBuffer he got deleted
                pbufferItem->onDeletion();
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffersMonitor::getPBufferObject
// Description: Returns the gsPBuffer object that is linked with the HPBUFFERARB handle
// Arguments:   pbufferHandler - The PBuffer handler that needs to be found
// Return Val:  gsPBuffer* - The pointer to the matching gsPBuffer object holding the
//              HPBUFFERARB handle
// Author:      Eran Zinman
// Date:        26/8/2007
// ---------------------------------------------------------------------------
int gsPBuffersMonitor::getPBufferObject(const oaPBufferHandle& pbufferHandler) const
{
    int pbufferVectorIndex = GS_PBUFFERS_MONITOR_PBUFFER_NOT_FOUND;

    // Check if we got a valid PBuffer handler pointer:
    if (pbufferHandler != 0)
    {
        // Get amount of PBuffers;
        int amountOfIndices = (int)_pbuffers.size();

        // Look for the item
        for (int i = 0; i < amountOfIndices; i++)
        {
            // Get PBuffer Item
            gsPBuffer* pPBufferItem = _pbuffers[i];
            GT_IF_WITH_ASSERT(pPBufferItem != NULL)
            {
                // Get the PBuffer Item Handler
                oaPBufferHandle bufferHandler = pPBufferItem->pbufferHandler();

                // Is this the handler we are looking for?
                if (bufferHandler == pbufferHandler)
                {
                    // We found the PBuffer that we were looking for, return the vector Index
                    pbufferVectorIndex = i;

                    // Skip the loop, we are done here
                    break;
                }
            }
        }
    }

    return pbufferVectorIndex;
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffersMonitor::onPBufferhDCGeneration
// Description: When an hDC is generated for a PBuffer, we updated the PBuffer
//              item with the new hDC
// Arguments:   pbufferHandler - The PBuffer handler that needs to be updated
//              pbufferhDC - The new hDC for the PBuffer
// Author:      Eran Zinman
// Date:        26/8/2007
// ---------------------------------------------------------------------------
void gsPBuffersMonitor::onPBufferhDCGeneration(const oaPBufferHandle& pbufferHandler, const oaDeviceContextHandle& pbufferhDC)
{
    // Check if we got a valid PBuffer handler pointer:
    if (pbufferHandler != 0)
    {
        // Get the PBuffer object holding this handle
        int pbufferVectorIndex = getPBufferObject(pbufferHandler);
        GT_IF_WITH_ASSERT(pbufferVectorIndex != GS_PBUFFERS_MONITOR_PBUFFER_NOT_FOUND)
        {
            // Get PBuffer item from the PBuffers vector
            gsPBuffer* pPBufferItem = _pbuffers[pbufferVectorIndex];
            GT_IF_WITH_ASSERT(pPBufferItem != NULL)
            {
                // Update the PBuffer item with the new hDC
                pPBufferItem->setPBufferHDC(pbufferhDC);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffersMonitor::onPBufferhDCRelease
// Description: When an hDC is released from a PBuffer, we updated the PBuffer item
// Arguments:   pbufferHandler - The PBuffer handler that needs to be updated
//              pbufferhDC - The hDC that needs to be released
// Author:      Eran Zinman
// Date:        26/8/2007
// ---------------------------------------------------------------------------
void gsPBuffersMonitor::onPBufferhDCRelease(const oaPBufferHandle& pbufferHandler, const oaDeviceContextHandle& pbufferhDC)
{
    // Check if we got a valid PBuffer handler pointer:
    if (pbufferHandler != 0)
    {
        // Get the PBuffer object holding this handle
        int pbufferVectorIndex = getPBufferObject(pbufferHandler);
        GT_IF_WITH_ASSERT(pbufferVectorIndex != GS_PBUFFERS_MONITOR_PBUFFER_NOT_FOUND)
        {
            // Get PBuffer item from the PBuffers vector
            gsPBuffer* pPBufferItem = _pbuffers[pbufferVectorIndex];
            GT_IF_WITH_ASSERT(pPBufferItem != NULL)
            {
                // Update the PBuffer item with the new hDC
                pPBufferItem->releasePBufferHDC(pbufferhDC);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffersMonitor::updateStaticBuffersDimensions
// Description: Updates all existing static buffer with the current thread HDC size
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/10/2008
// ---------------------------------------------------------------------------
bool gsPBuffersMonitor::updatePBuffersDimensions()
{
    bool retVal = true;

    // Get amount of PBuffers;
    int amountOfIndices = (int)_pbuffers.size();

    // Look for the PBuffer that have the same hDC and the one that just made current
    for (int i = 0; i < amountOfIndices; i++)
    {
        // Get PBuffer Item
        gsPBuffer* pPBufferItem = _pbuffers[i];

        if (pPBufferItem)
        {
            int contextId = pPBufferItem->pbufferContextId();
            gsRenderContextMonitor* pRCMon = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
            GT_IF_WITH_ASSERT(pRCMon != NULL)
            {
                retVal = pRCMon->buffersMonitor().updateStaticBuffersDimensions() && retVal;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffersMonitor::onWGLMakeCurrent
// Description: This function is called every time *any* hDC is made current with
//              *any* hRC. We check if the hDC belongs to any of PBuffers in our
//              list. If it does, we update the render context of that PBuffer.
// Arguments:   hDC - The hDC that becomes current
//              hRC - The hRC that becomes current
//              and writing.
// Author:      Eran Zinman
// Date:        26/8/2007
// ---------------------------------------------------------------------------
void gsPBuffersMonitor::onWGLMakeCurrent(oaDeviceContextHandle hDC, oaOpenGLRenderContextHandle hRC)
{
    // Get amount of PBuffers;
    int amountOfIndices = (int)_pbuffers.size();

    // Look for the PBuffer that have the same hDC and the one that just made current
    for (int i = 0; i < amountOfIndices; i++)
    {
        // Get PBuffer Item
        gsPBuffer* pbufferItem = _pbuffers[i];

        if (pbufferItem)
        {
            // Get the PBuffer hDC
            oaDeviceContextHandle pbufferItemhDC = pbufferItem->deviceContextOSHandle();

            // Is this the handler we are looking for?
            if (pbufferItemhDC == hDC)
            {
                // Notify the PBuffer that it was made current
                pbufferMadeCurrent(pbufferItem, hRC);

                // Item was found, break the loop
                break;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffersMonitor::pbufferMadeCurrent
// Description: Called when a PBuffer becomes current. When it does become
//              current, we update it's hRC and update it's context data
//              snapshot (active static buffer inside the PBuffer)
// Arguments:   pbufferItem - The PBuffer item that became current
//              pbufferhRC - The hRC that is attached to the PBuffer
//              and writing.
// Author:      Eran Zinman
// Date:        3/2/2008
// ---------------------------------------------------------------------------
void gsPBuffersMonitor::pbufferMadeCurrent(gsPBuffer* pbufferItem, oaOpenGLRenderContextHandle pbufferhRC)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pbufferItem != NULL)
    {
        // Get the spy render context Id
        int spyContextId = getRenderContextSpyId(pbufferhRC);
        GT_IF_WITH_ASSERT(spyContextId > 0)
        {
            // Update the PBuffer attached render context (spy render context Id)
            pbufferItem->setPBufferRenderContextSpyId(spyContextId);

            // Update the PBuffer context data snapshot
            pbufferItem->updateDataSnapshot();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffersMonitor::onGLXMakeCurrent
// Description: This function is called every time *any* hDC is made current with
//              *any* hRC. We check if the hDC belongs to any of PBuffers in our
//              list. If it does, we update the render context of that PBuffer.
// Arguments:   hDC - The hDC that becomes current
//              draw - Specifies a GLX drawable to render into.
//                     Must be an XID representing a GLXWindow, GLXPixmap, or GLXPbuffer.
//              read - Specifies a GLX drawable to read from.
//              Must be an XID representing a GLXWindow, GLXPixmap, or GLXPbuffer.
//              hRC - The hRC that becomes current
//              and writing.
// Author:      Eran Zinman
// Date:        26/8/2007
// ---------------------------------------------------------------------------
void gsPBuffersMonitor::onGLXMakeCurrent(oaDeviceContextHandle hDC, oaPBufferHandle draw, oaPBufferHandle read, oaOpenGLRenderContextHandle hRC)
{
    // Get amount of PBuffers;
    int amountOfIndices = (int)_pbuffers.size();

    // Look for the PBuffer that have the same hDC and the one that just made current
    for (int i = 0; i < amountOfIndices; i++)
    {
        // Get PBuffer Item
        gsPBuffer* pbufferItem = _pbuffers[i];

        if (pbufferItem)
        {
            // Get the PBuffer handler:
            oaPBufferHandle pbufferHandler = pbufferItem->pbufferHandler();

            // Check if the draw to read surfaces are actually our PBuffer:
            if (draw == pbufferHandler || read == pbufferHandler)
            {
                // According to the OpenGL reference pages:
                // ========================================
                // "glXMakeContextCurrent binds ctx to the current rendering thread and to the
                //  draw and read GLX drawables. draw and read may be the same."

                // The PBuffer will be now binded with the new hRC; Get the spy render context Id
                int spyContextId = getRenderContextSpyId(hRC);
                GT_IF_WITH_ASSERT(spyContextId > 0)
                {
                    // Update the PBuffer attached render context (spy render context Id)
                    pbufferItem->setPBufferRenderContextSpyId(spyContextId);

                    // Set the PBuffer attached hDC
                    pbufferItem->setPBufferHDC(hDC);

                    // Update the PBuffer context data snapshot
                    pbufferItem->updateDataSnapshot();

                    // Item was found, break the loop
                    break;
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffersMonitor::onCGLSetPBuffer
// Description: Called whenever a context is made current to a pbuffer
// Arguments: oaOpenGLRenderContextHandle hRC
//            oaPBufferHandle hPBuffer
//            GLenum cubeMapFace
//            GLint mipLevel
// Return Val: void
// Author:      Uri Shomroni
// Date:        11/3/2009
// ---------------------------------------------------------------------------
void gsPBuffersMonitor::onCGLSetPBuffer(oaOpenGLRenderContextHandle hRC, oaPBufferHandle hPBuffer, GLenum cubeMapFace, GLint mipLevel)
{
    // Get the spy render context Id
    int spyContextId = getRenderContextSpyId(hRC);
    GT_IF_WITH_ASSERT(spyContextId > 0)
    {
        // Make sure no other pbuffers are current with this context (or, if hPBuffer is NULL,
        // make sure no pbuffers are current with this context at all).
        size_t numberOfPBuffers = _pbuffers.size();

        for (size_t i = 0; i < numberOfPBuffers; i++)
        {
            gsPBuffer* pCurrentPBuffer = _pbuffers[i];
            GT_IF_WITH_ASSERT(pCurrentPBuffer != NULL)
            {
                if (pCurrentPBuffer->pbufferHandler() == hPBuffer)
                {
                    // This is the PBuffer we want to make current:
                    // Update the PBuffer attached render context (spy render context Id)
                    pCurrentPBuffer->setPBufferRenderContextSpyId(spyContextId);

                    // Set the level and face:
                    pCurrentPBuffer->setMipmapLevel(mipLevel);
                    pCurrentPBuffer->setCubeMapFace(cubeMapFace);

                    // Update the PBuffer context data snapshot
                    pCurrentPBuffer->updateDataSnapshot();
                }
                else
                {
                    // If this is a different PBuffer, that was formerly made current with this context:
                    if (pCurrentPBuffer->pbufferContextId() == spyContextId)
                    {
                        // Clear this flag:
                        pCurrentPBuffer->setPBufferRenderContextSpyId(-1);
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffersMonitor::onCGLTexImagePBuffer
// Description: Called when CGLTexImagePBuffer is used, to bind the texture to
//              the pbuffer
// Arguments: int contextId - the ID of the context where the texture resides
//            oaPBufferHandle hPBuffer - the handle to the pbuffer
//            GLenum source - the static buffer (in the PBuffer) which will be
//                            bound to the texture (according to the apple spec,
//                            this should only be GL_FRONT or GL_BACK)
// Author:      Uri Shomroni
// Date:        11/3/2009
// ---------------------------------------------------------------------------
void gsPBuffersMonitor::onCGLTexImagePBuffer(int contextId, oaPBufferHandle hPBuffer, GLenum source) const
{
    // Get the render context monitor:
    gsRenderContextMonitor* pRCMon = gs_stat_openGLMonitorInstance.renderContextMonitor(contextId);
    GT_IF_WITH_ASSERT(pRCMon != NULL)
    {
        int pbufferIndex = getPBufferObject(hPBuffer);
        GT_IF_WITH_ASSERT(pbufferIndex > -1)
        {
            gsPBuffer* pPBuffer = getPBufferObjectDetails(pbufferIndex);
            GT_IF_WITH_ASSERT(pPBuffer != NULL)
            {
                // Determine the bind target:
                GLenum target = GL_TEXTURE_2D;

                switch (pPBuffer->bindTarget())
                {
                    case apPBuffer::AP_2D_PBUFFER:
                        target = GL_TEXTURE_2D;
                        break;

                    case apPBuffer::AP_CUBE_MAP_PBUFFER:
                    {
                        GLenum face = pPBuffer->cubeMapFace();

                        if ((face == GL_TEXTURE_CUBE_MAP_POSITIVE_X) || (face == GL_TEXTURE_CUBE_MAP_NEGATIVE_X) ||
                            (face == GL_TEXTURE_CUBE_MAP_POSITIVE_Y) || (face == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y) ||
                            (face == GL_TEXTURE_CUBE_MAP_POSITIVE_Z) || (face == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z))
                        {
                            target = face;
                        }
                        else
                        {
                            // Undefined texture face:
                            GT_ASSERT(false);
                            target = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
                        }
                    }
                    break;

                    case apPBuffer::AP_RECTANGLE_PBUFFER:
                        target = GL_TEXTURE_RECTANGLE_ARB;
                        break;

                    default:
                        // Something is wrong
                        GT_ASSERT(false);
                        break;
                }

                // Determine the internal format:
                GLenum internalformat = GL_RGBA;

                switch (pPBuffer->internalFormat())
                {
                    case apPBuffer::AP_RGB_PBUFFER:
                        internalformat = GL_RGB;
                        break;

                    case apPBuffer::AP_RGBA_PBUFFER:
                        internalformat = GL_RGBA;
                        break;

                    default:
                        // Something is wrong
                        GT_ASSERT(false);
                        break;
                }

                bool rcTex = pRCMon->onTextureImageLoaded(target, pPBuffer->mipmapLevel(), internalformat, pPBuffer->width(), pPBuffer->height(), 0, 0, internalformat, GL_UNSIGNED_BYTE);
                GT_IF_WITH_ASSERT(rcTex)
                {
                    // Mark that the texture need to be constantly updated (By using this function, the PBuffer effectively becomes
                    // its own framebuffer):
                    gsTexturesMonitor* pTexMon = pRCMon->texturesMonitor();
                    GT_IF_WITH_ASSERT(pTexMon != NULL)
                    {
                        gsGLTexture* pTex = pTexMon->getCurrentlyBoundTextureObjectDetails(target);
                        GT_IF_WITH_ASSERT(pTex != NULL)
                        {
                            pTex->markTextureAsBoundToActiveFBO(true);
                            pTex->setPBufferName(pbufferIndex);
                            pTex->setPBufferStaticBuffer(apGLEnumToColorIndexBufferType(source));
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffersMonitor::getRenderContextSpyId
// Description: This function transforms a render context to it's equivalent
//              spyID render context, taken from the context monitor
// Arguments:   pbufferhRC - The hRC that we are interested in his Spy ID
// Return Val:  SpyID render context, if found. (-1) If ID not found
// Author:      Eran Zinman
// Date:        26/8/2007
// ---------------------------------------------------------------------------
int gsPBuffersMonitor::getRenderContextSpyId(const oaOpenGLRenderContextHandle& pbufferhRC)
{
    // Will get the input context spy id:
    int spyContextId = 0;

    // If this is a real context:
    if (pbufferhRC != NULL)
    {
        // Get the context spy id:
        spyContextId =  gs_stat_openGLMonitorInstance.renderContextSpyId(pbufferhRC);
    }

    return spyContextId;
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffersMonitor::getPBufferObjectDetails
// Description: Get PBuffer object associated with the pbufferId
// Arguments:   pbufferId - The PBuffer to retrieve the details from
// Return Val:  A Pointer to the PBuffer object
// Author:      Eran Zinman
// Date:        28/8/2007
// ---------------------------------------------------------------------------
gsPBuffer* gsPBuffersMonitor::getPBufferObjectDetails(int pbufferId) const
{
    gsPBuffer* pPBufferObj = NULL;

    // Range check:
    int amountOfPBuffersIndices = (int)_pbuffers.size();
    GT_IF_WITH_ASSERT((0 <= pbufferId) && (pbufferId < amountOfPBuffersIndices))
    {
        // Get the pPBufferObj
        pPBufferObj = _pbuffers[pbufferId];
    }

    return pPBufferObj;
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffersMonitor::updatePBufferStaticBufferRawData
// Description: Updates the static buffer raw data object associated with the pbufferId
// Arguments:   pbufferId - The PBuffer to update its static buffer raw data
//              bufferType - static buffer to update
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        19/1/2008
// ---------------------------------------------------------------------------
bool gsPBuffersMonitor::updatePBufferStaticBufferRawData(int pbufferId, apDisplayBuffer bufferType)
{
    bool retVal = false;

    // Range check:
    int amountOfPBuffersIndices = (int)_pbuffers.size();
    GT_IF_WITH_ASSERT((0 <= pbufferId) && (pbufferId < amountOfPBuffersIndices))
    {
        // Get the pPBufferObj
        gsPBuffer* pPBufferObj = _pbuffers[pbufferId];
        GT_IF_WITH_ASSERT(pPBufferObj != NULL)
        {
            retVal = pPBufferObj->updatePBufferStaticBufferRawData(bufferType);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffersMonitor::amountOfPBufferContentBuffers
// Description: Return the number of static buffer associated with a PBuffer
// Arguments:   pbufferId - The PBuffer to retrieve the details from
// Return Val:  Amount of static buffer inside the PBuffer
// Author:      Eran Zinman
// Date:        28/8/2007
// ---------------------------------------------------------------------------
int gsPBuffersMonitor::amountOfPBufferContentBuffers(int pbufferId) const
{
    int staticBufferAmount = 0;

    // Range check:
    int amountOfPBuffersIndices = (int)_pbuffers.size();
    GT_IF_WITH_ASSERT((0 <= pbufferId) && (pbufferId < amountOfPBuffersIndices))
    {
        // Get pbuffer item
        gsPBuffer* pbufferItem = _pbuffers[pbufferId];
        GT_IF_WITH_ASSERT(pbufferItem != NULL)
        {
            // Get amount of static buffers
            int contextId = pbufferItem->pbufferContextId();
            gsRenderContextMonitor* pRCMon = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
            GT_IF_WITH_ASSERT(pRCMon != NULL)
            {
                staticBufferAmount = pRCMon->buffersMonitor().amountOfStaticBuffers();
            }
        }
    }

    return staticBufferAmount;
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffersMonitor::getPBufferStaticBufferObjectDetails
// Description: Return the number of static buffer associated with a PBuffer
// Arguments:   pbufferId - The PBuffer to retrieve the details from
//              bufferType - The static buffer in the PBuffer
// Return Val:  The details of the static buffer object
// Author:      Eran Zinman
// Date:        03/09/2007
// ---------------------------------------------------------------------------
apStaticBuffer* gsPBuffersMonitor::getPBufferStaticBufferObjectDetails(int pbufferId, apDisplayBuffer bufferType) const
{
    apStaticBuffer* pStaticBuffer = NULL;

    // Range check:
    int amountOfPBuffersIndices = (int)_pbuffers.size();
    bool rc = ((0 <= pbufferId) && (pbufferId < amountOfPBuffersIndices));
    GT_IF_WITH_ASSERT(rc)
    {
        // Get PBuffer item
        gsPBuffer* pPBufferItem = _pbuffers[pbufferId];
        GT_IF_WITH_ASSERT(pPBufferItem != NULL)
        {
            // Get the static buffer details
            int contextId = pPBufferItem->pbufferContextId();
            gsRenderContextMonitor* pRCMon = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
            GT_IF_WITH_ASSERT(pRCMon != NULL)
            {
                pStaticBuffer = pRCMon->buffersMonitor().getStaticBufferObjectDetails(bufferType);
            }
        }
    }

    return pStaticBuffer;
}

// ---------------------------------------------------------------------------
// Name:        gsPBuffersMonitor::getPBufferStaticBufferType
// Description: Return the static buffer type, given a pbufferId and
//              a static buffer iter.
// Arguments:   pbufferId - The PBuffer to retrieve the details from
//              staticBufferIter - The static buffer iter
//              bufferType - Output static buffer type
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        2/1/2008
// ---------------------------------------------------------------------------
bool gsPBuffersMonitor::getPBufferStaticBufferType(int pbufferId, int staticBufferIter, apDisplayBuffer& bufferType) const
{
    bool retVal = false;

    // Range check:
    int amountOfPBuffersIndices = (int)_pbuffers.size();
    bool rc = ((0 <= pbufferId) && (pbufferId < amountOfPBuffersIndices));
    GT_IF_WITH_ASSERT(rc)
    {
        // Get PBuffer item
        gsPBuffer* pPBufferItem = _pbuffers[pbufferId];
        GT_IF_WITH_ASSERT(pPBufferItem != NULL)
        {
            // Get the static buffer details
            apStaticBuffer* pStaticBuffer = NULL;
            int contextId = pPBufferItem->pbufferContextId();
            gsRenderContextMonitor* pRCMon = gsOpenGLMonitor::instance().renderContextMonitor(contextId);
            GT_IF_WITH_ASSERT(pRCMon != NULL)
            {
                pStaticBuffer = pRCMon->buffersMonitor().getStaticBufferObjectDetails(staticBufferIter);
            }

            GT_IF_WITH_ASSERT(pStaticBuffer != NULL)
            {
                // Get buffer type
                bufferType = pStaticBuffer->bufferType();

                retVal = true;
            }
        }
    }

    return retVal;
}


