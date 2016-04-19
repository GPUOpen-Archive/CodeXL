//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsVBOMonitor.cpp
///
//==================================================================================

//------------------------------ gsVBOMonitor.cpp ------------------------------

// Standard C:
#include <math.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/apDefinitions.h>

// Spy Utilities:
#include <AMDTServerUtilities/Include/suStringConstants.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Local:
#include <src/gsOpenGLMonitor.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsWrappersCommon.h>
#include <src/gsExtensionsManager.h>
#include <src/gsRenderContextMonitor.h>
#include <src/gsVBOMonitor.h>
#include <src/gsStringConstants.h>
#include <src/gsBufferSerializer.h>


// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::gsVBOMonitor
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        25/5/2008
// ---------------------------------------------------------------------------
gsVBOMonitor::gsVBOMonitor(int spyContextId): _spyContextId(spyContextId),
    m_bindArrayBufferVBOName(0),
    m_bindAtmoicCounterBufferVBOName(0),
    m_bindCopyReadBufferVBOName(0),
    m_bindCopyWriteBufferVBOName(0),
    m_bindDispatchIndirectBufferVBOName(0),
    m_bindDrawIndirectBufferVBOName(0),
    m_bindElementArrayBufferVBOName(0),
    m_bindPixelPackBuffer(0),
    m_bindPixelUnPackBuffer(0),
    m_bindQueryBufferVBOName(0),
    m_bindShaderStorageBufferVBOName(0),
    m_bindTextureBufferName(0),
    m_bindTransformFeedbackBufferVBOName(0),
    m_bindUniformBufferName(0),
    m_bindUniformBufferEXTName(0)
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    , _glBindBuffer(NULL), _glGetBufferSubData(NULL)
#endif
{
}


// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::~gsVBOMonitor
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        25/5/2008
// ---------------------------------------------------------------------------
gsVBOMonitor::~gsVBOMonitor()
{
    // Delete the texture wrappers vector:
    int amountOfVBOs = (int)_vbos.size();

    for (int i = 0; i < amountOfVBOs; i++)
    {
        delete _vbos[i];
        _vbos[i] = NULL;
    }

    _vbos.clear();

    _vboOpenGLNameToIndex.clear();

    _vboNameToTargetMap.clear();
}

// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::constructNewFbo
// Description: Construct a new aspGLVBO object, and add it to the existing VBOs
// Arguments: GLuint vboName
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/5/2008
// ---------------------------------------------------------------------------
apGLVBO* gsVBOMonitor::addNewVBO(GLuint vboName)
{
    // Construct new object:
    apGLVBO* pVBO = new apGLVBO;

    // Set the object's name:
    pVBO->setName(vboName);

    // There is currently no free index - we will allocate a new index:
    int newVBOIndex = (int)_vbos.size();
    _vbos.push_back(pVBO);

    // Add the monitor index to the _textureOpenGLNameToIndex map:
    _vboOpenGLNameToIndex[vboName] = newVBOIndex;

    return pVBO;
}

// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::removeVBO
// Description: Destroys the apGLVBO object named vboName
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/10/2008
// ---------------------------------------------------------------------------
bool gsVBOMonitor::removeVBO(GLuint vboName)
{
    bool retVal = false;

    int i = (int)getVBOIndex(vboName);
    int n = (int)_vbos.size();

    if ((i < n) && (i > -1))
    {
        apGLVBO* pCurrentVBO = _vbos[i];
        GT_IF_WITH_ASSERT(pCurrentVBO != NULL)
        {
            GT_IF_WITH_ASSERT(pCurrentVBO->name() == vboName)
            {
                // Remove it from the textures array and map:
                _vbos[i] = NULL;

                // Release memory:
                delete pCurrentVBO;

                // Remove the vbo from the mapping:
                _vboOpenGLNameToIndex[vboName] = -1;

                for (int j = i; j < (n - 1); j++)
                {
                    // Shift each vector element after the one we removed one spot back:
                    _vbos[j] = _vbos[j + 1];

                    // Update the Mapping:
                    GLuint currentName = _vbos[j]->name();
                    GT_ASSERT(_vboOpenGLNameToIndex[currentName] == (j + 1));
                    _vboOpenGLNameToIndex[currentName] = j;
                }

                // remove the last (now duplicate) element in the vector
                _vbos.pop_back();

                retVal = true;
            }
        }

        if (!retVal)
        {
            // The VBO exists in our system, but we couldn't delete it, print an error message:
            gtString errMsg;
            errMsg.appendFormattedString(L"Could not delete vertex buffer object %u", vboName);
            GT_ASSERT_EX(retVal, errMsg.asCharArray());
        }
    }
    else
    {
        // Print out a warning message to the log:
        gtString errorMessage = GS_STR_deletingNonExistingVBO;
        errorMessage.appendFormattedString(L" %u", vboName);
        OS_OUTPUT_DEBUG_LOG(errorMessage.asCharArray(), OS_DEBUG_LOG_INFO);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::getVBODetails
// Description: Returns an apGLVBO object according to GL VBO name
// Arguments: GLuint vboName
// Return Val: apGLVBO*
// Author:      Sigal Algranaty
// Date:        25/5/2008
// ---------------------------------------------------------------------------
apGLVBO* gsVBOMonitor::getVBODetails(GLuint vboName) const
{
    apGLVBO* retVal = NULL;

    gtMap<GLuint, int>::const_iterator endIter = _vboOpenGLNameToIndex.end();
    gtMap<GLuint, int>::const_iterator iter = _vboOpenGLNameToIndex.find(vboName);

    if (iter != endIter)
    {
        int i = (*iter).second;
        GT_IF_WITH_ASSERT((i >= 0) && (i < (int)_vbos.size()))
        {
            apGLVBO* pVBO = _vbos[i];
            GT_IF_WITH_ASSERT(pVBO != NULL)
            {
                GT_IF_WITH_ASSERT(pVBO->name() == vboName)
                {
                    retVal = pVBO;
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::onVertexBufferObjectGeneration
// Description: Is called when vertex buffer objects are generated.
// Arguments:   amountOfGeneratedVertexBuffers - The amount of generated vertex buffer objects.
//              vertexBufferNames - An array, containing the names of the generated vertex buffers objects.
// Author:      Sigal Algranaty
// Date:        22/10/2008
// ---------------------------------------------------------------------------
void gsVBOMonitor::onVertexBufferObjectGeneration(GLsizei amountOfGeneratedVertexBuffers, GLuint* vertexBufferNames)
{
    // If the monitored context is NOT the NULL context:
    GT_IF_WITH_ASSERT(_spyContextId != AP_NULL_CONTEXT_ID)
    {
        gtVector<apAllocatedObject*> vbosForAllocationMonitor;

        for (int i = 0; i < amountOfGeneratedVertexBuffers; i++)
        {
            // Get the name of the currently created render buffer object:
            GLuint newVertexBufferName = vertexBufferNames[i];

            // Ignore failed indices:
            if (newVertexBufferName > 0)
            {
                apGLVBO* pCreatedVBO = addNewVBO(newVertexBufferName);
                GT_IF_WITH_ASSERT(NULL != pCreatedVBO)
                {
                    vbosForAllocationMonitor.push_back(pCreatedVBO);
                }
            }
        }

        // Register this object in the allocated objects monitor:
        su_stat_theAllocatedObjectsMonitor.registerAllocatedObjects(vbosForAllocationMonitor);
    }
}

// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::onRenderBufferObjectsDeletion
// Description: Is called when vbo objects are deleted.
// Arguments:   amountOfDeletedVBOs - The amount of deleted vertex buffer objects.
//              vboNames - An array, containing the names of the vertex buffer objects to be deleted.
// Author:      Sigal Algranaty
// Date:        22/10/2008
// ---------------------------------------------------------------------------
void gsVBOMonitor::onVertexBufferObjectDeletion(GLsizei amountOfDeletedVBOs, const GLuint* vboNames)
{
    // If the monitored context is NOT the NULL context:
    GT_IF_WITH_ASSERT(_spyContextId != AP_NULL_CONTEXT_ID)
    {
        for (int i = 0; i < amountOfDeletedVBOs; i++)
        {
            // Delete the input texture param:
            GLuint currentVBOName = vboNames[i];

            bool rc = removeVBO(currentVBOName);
            GT_ASSERT(rc);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::onVertexBufferObjectTargetBind
// Description: Is called when a vbo object target is bound
// Arguments:   GLenum target - vbo target
//              vboNames - vbo OpenGL name
// Author:      Sigal Algranaty
// Date:        22/10/2008
// ---------------------------------------------------------------------------
void gsVBOMonitor::onVertexBufferObjectTargetBind(GLenum target, GLuint vboName)
{
    // If this is not an unbind command:
    if (vboName != 0)
    {
        addTargetToBufferObject(target, vboName, true);
    }

    GLuint previousBuffer = 0;

    switch (target)
    {
        case GL_ARRAY_BUFFER:
            previousBuffer = m_bindArrayBufferVBOName;
            m_bindArrayBufferVBOName = vboName;
            break;

        case GL_ATOMIC_COUNTER_BUFFER:
            previousBuffer = m_bindAtmoicCounterBufferVBOName;
            m_bindAtmoicCounterBufferVBOName = vboName;
            break;

        case GL_COPY_READ_BUFFER:
            previousBuffer = m_bindCopyReadBufferVBOName;
            m_bindCopyReadBufferVBOName = vboName;
            break;

        case GL_COPY_WRITE_BUFFER:
            previousBuffer = m_bindCopyWriteBufferVBOName;
            m_bindCopyWriteBufferVBOName = vboName;
            break;

        case GL_DISPATCH_INDIRECT_BUFFER:
            previousBuffer = m_bindDispatchIndirectBufferVBOName;
            m_bindDispatchIndirectBufferVBOName = vboName;
            break;

        case GL_DRAW_INDIRECT_BUFFER:
            previousBuffer = m_bindDrawIndirectBufferVBOName;
            m_bindDrawIndirectBufferVBOName = vboName;
            break;

        case GL_ELEMENT_ARRAY_BUFFER:
            previousBuffer = m_bindElementArrayBufferVBOName;
            m_bindElementArrayBufferVBOName = vboName;
            break;

        case GL_PIXEL_PACK_BUFFER:
            previousBuffer = m_bindPixelPackBuffer;
            m_bindPixelPackBuffer = vboName;
            break;

        case GL_PIXEL_UNPACK_BUFFER:
            previousBuffer = m_bindPixelUnPackBuffer;
            m_bindPixelUnPackBuffer = vboName;
            break;

        case GL_QUERY_BUFFER:
            previousBuffer = m_bindQueryBufferVBOName;
            m_bindQueryBufferVBOName = vboName;
            break;

        case GL_SHADER_STORAGE_BUFFER:
            previousBuffer = m_bindShaderStorageBufferVBOName;
            m_bindShaderStorageBufferVBOName = vboName;
            break;

        case GL_TEXTURE_BUFFER:
            previousBuffer = m_bindTextureBufferName;
            m_bindTextureBufferName = vboName;
            break;

        case GL_TRANSFORM_FEEDBACK_BUFFER:
            previousBuffer = m_bindTransformFeedbackBufferVBOName;
            m_bindTransformFeedbackBufferVBOName = vboName;
            break;

        case GL_UNIFORM_BUFFER:
            previousBuffer = m_bindUniformBufferName;
            m_bindUniformBufferName = vboName;
            break;

        case GL_UNIFORM_BUFFER_EXT:
            previousBuffer = m_bindUniformBufferEXTName;
            m_bindUniformBufferEXTName = vboName;
            break;

        default:
        {
            // Have we used this target before?
            const auto& findIter = m_vboTargetToAttachedNameMap.find(target);
            const auto& endIter = m_vboTargetToAttachedNameMap.end();
            bool isFirstTimeUsed = (findIter == endIter);

            if (isFirstTimeUsed)
            {
                // Print the unsupported VBO target to the log file:
                gtString debugMessage;
                apGLenumValueToString(target, debugMessage);
                debugMessage.prepend(L"Unsupported vbo target ");
                OS_OUTPUT_DEBUG_LOG(debugMessage.asCharArray(), OS_DEBUG_LOG_DEBUG);
            }
            else
            {
                previousBuffer = findIter->second;
            }

            // Save it in the map:
            m_vboTargetToAttachedNameMap[target] = vboName;
        }
        break;
    }

    // Set the bind targets in the VBO objects:
    if (0 != previousBuffer)
    {
        apGLVBO* pPreviousBuffer = getVBODetails(previousBuffer);
        GT_IF_WITH_ASSERT(nullptr != pPreviousBuffer)
        {
            pPreviousBuffer->onUnbindFromTarget(target);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::addTargetToBufferObject
// Description: Adds a buffer target to a buffer without binding it (DSA)
// Arguments:   GLenum target - the target
//              GLuint vboName - the buffer name
// ---------------------------------------------------------------------------
void gsVBOMonitor::addTargetToBufferObject(GLenum target, GLuint vboName, bool bind)
{
    // Get the VBO object
    apGLVBO* pVBO = getVBODetails(vboName);

    // If this is a application-generated name (i.e. calling glBind before glGen):
    if (pVBO == NULL)
    {
        // Generate the buffer's monitor:
        onVertexBufferObjectGeneration(1, &vboName);

        // Get the generated VBO:
        pVBO = getVBODetails(vboName);
    }

    // Add the VBO name target to the map:
    _vboNameToTargetMap[vboName] = target;

    GT_IF_WITH_ASSERT(nullptr != pVBO)
    {
        pVBO->onBindToTarget(target);

        if (target == GL_ELEMENT_ARRAY_BUFFER)
        {
            // Set the VBO format to index array format:
            pVBO->setBufferDisplayFormat(OA_TEXEL_FORMAT_I1UI);
        }

        if (!bind)
        {
            pVBO->onUnbindFromTarget(target);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::getVBOAttachment
// Description: Returns the vbo attachement according to the vbo name
// Arguments: GLuint vboName
//            GLenum& vboAttachment
// Return Val: void
// Author:      Sigal Algranaty
// Date:        23/10/2008
// ---------------------------------------------------------------------------
GLenum gsVBOMonitor::getVBOLatestAttachment(GLuint vboName) const
{
    GLenum vboAttachment = GL_NONE;

    vboAttachment = 0;
    gtMap<GLuint, GLenum>::const_iterator iter = _vboNameToTargetMap.find(vboName);

    if (iter != _vboNameToTargetMap.end())
    {
        vboAttachment = (*iter).second;
    }

    return vboAttachment;
}
void gsVBOMonitor::getAllCurrentVBOAttachments(GLuint vboName, gtVector<GLenum>& vboAttachments) const
{
    if (0 != vboName)
    {
        if (m_bindArrayBufferVBOName == vboName)
        {
            vboAttachments.push_back(GL_ARRAY_BUFFER);
        }

        if (m_bindAtmoicCounterBufferVBOName == vboName)
        {
            vboAttachments.push_back(GL_ATOMIC_COUNTER_BUFFER);
        }

        if (m_bindCopyReadBufferVBOName == vboName)
        {
            vboAttachments.push_back(GL_COPY_READ_BUFFER);
        }

        if (m_bindCopyWriteBufferVBOName == vboName)
        {
            vboAttachments.push_back(GL_COPY_WRITE_BUFFER);
        }

        if (m_bindDispatchIndirectBufferVBOName == vboName)
        {
            vboAttachments.push_back(GL_DISPATCH_INDIRECT_BUFFER);
        }

        if (m_bindDrawIndirectBufferVBOName == vboName)
        {
            vboAttachments.push_back(GL_DRAW_INDIRECT_BUFFER);
        }

        if (m_bindElementArrayBufferVBOName == vboName)
        {
            vboAttachments.push_back(GL_ELEMENT_ARRAY_BUFFER);
        }

        if (m_bindPixelPackBuffer == vboName)
        {
            vboAttachments.push_back(GL_PIXEL_PACK_BUFFER);
        }

        if (m_bindPixelUnPackBuffer == vboName)
        {
            vboAttachments.push_back(GL_PIXEL_UNPACK_BUFFER);
        }

        if (m_bindQueryBufferVBOName == vboName)
        {
            vboAttachments.push_back(GL_QUERY_BUFFER);
        }

        if (m_bindShaderStorageBufferVBOName == vboName)
        {
            vboAttachments.push_back(GL_SHADER_STORAGE_BUFFER);
        }

        if (m_bindTextureBufferName == vboName)
        {
            vboAttachments.push_back(GL_TEXTURE_BUFFER);
        }

        if (m_bindTransformFeedbackBufferVBOName == vboName)
        {
            vboAttachments.push_back(GL_TRANSFORM_FEEDBACK_BUFFER);
        }

        if (m_bindUniformBufferName == vboName)
        {
            vboAttachments.push_back(GL_UNIFORM_BUFFER);
        }

        if (m_bindUniformBufferEXTName == vboName)
        {
            vboAttachments.push_back(GL_UNIFORM_BUFFER_EXT);
        }

        // Also check if the VBO has been attached to another target:
        GLenum lastTarget = getVBOLatestAttachment(vboName);
        gtMap<GLenum, GLuint>::const_iterator findIter = m_vboTargetToAttachedNameMap.find(lastTarget);

        if ((m_vboTargetToAttachedNameMap.end() != findIter) && (vboName == findIter->second))
        {
            vboAttachments.push_back(lastTarget);
        }
    }
}



// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::getAttachedVBOName
// Description: Returns the vbo attached to the attachment
// Arguments: GLenum vboAttachment - the requested VBO attachment
//            GLuint& vboName - output - the VBO name
// Return Val: void
// Author:      Sigal Algranaty
// Date:        23/10/2008
// ---------------------------------------------------------------------------
GLuint gsVBOMonitor::getAttachedVBOName(GLenum vboAttachment) const
{
    GLuint vboName = 0;

    if (GL_NONE != vboAttachment)
    {
        // Check if the vbo is one of the attachments:
        if (GL_ARRAY_BUFFER == vboAttachment)
        {
            vboName = m_bindArrayBufferVBOName;
        }
        else if (GL_ATOMIC_COUNTER_BUFFER == vboAttachment)
        {
            vboName = m_bindAtmoicCounterBufferVBOName;
        }
        else if (GL_COPY_READ_BUFFER == vboAttachment)
        {
            vboName = m_bindCopyReadBufferVBOName;
        }
        else if (GL_COPY_WRITE_BUFFER == vboAttachment)
        {
            vboName = m_bindCopyWriteBufferVBOName;
        }
        else if (GL_DISPATCH_INDIRECT_BUFFER == vboAttachment)
        {
            vboName = m_bindDispatchIndirectBufferVBOName;
        }
        else if (GL_DRAW_INDIRECT_BUFFER == vboAttachment)
        {
            vboName = m_bindDrawIndirectBufferVBOName;
        }
        else if (GL_ELEMENT_ARRAY_BUFFER == vboAttachment)
        {
            vboName = m_bindElementArrayBufferVBOName;
        }
        else if (GL_PIXEL_PACK_BUFFER == vboAttachment)
        {
            vboName = m_bindPixelPackBuffer;
        }
        else if (GL_PIXEL_UNPACK_BUFFER == vboAttachment)
        {
            vboName = m_bindPixelUnPackBuffer;
        }
        else if (GL_QUERY_BUFFER == vboAttachment)
        {
            vboName = m_bindQueryBufferVBOName;
        }
        else if (GL_SHADER_STORAGE_BUFFER == vboAttachment)
        {
            vboName = m_bindShaderStorageBufferVBOName;
        }
        else if (GL_TEXTURE_BUFFER == vboAttachment)
        {
            vboName = m_bindTextureBufferName;
        }
        else if (GL_TRANSFORM_FEEDBACK_BUFFER == vboAttachment)
        {
            vboName = m_bindTransformFeedbackBufferVBOName;
        }
        else if (GL_UNIFORM_BUFFER == vboAttachment)
        {
            vboName = m_bindUniformBufferName;
        }
        else if (GL_UNIFORM_BUFFER_EXT == vboAttachment)
        {
            vboName = m_bindUniformBufferEXTName;
        }
        else
        {
            vboName = 0;
            gtMap<GLenum, GLuint>::const_iterator iter = m_vboTargetToAttachedNameMap.find(vboAttachment);

            if (iter != m_vboTargetToAttachedNameMap.end())
            {
                vboName = (*iter).second;
            }
        }
    }

    return vboName;
}

// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::onVertexBufferObjectDirectAccessDataSet
// Description  Handles buffer data set (direct state access)
// Arguments: GLuint buffer
//            GLsizeiptr sizeOfData
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/9/2009
// ---------------------------------------------------------------------------
void gsVBOMonitor::onVertexBufferObjectDirectAccessDataSet(GLuint buffer, GLsizeiptr sizeOfData)
{
    apGLVBO* pVBO = getVBODetails(buffer);

    if (pVBO != NULL)
    {
        pVBO->setSize(sizeOfData);
        pVBO->markAsDirty(true);
    }
}

// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::onVertexBufferObjectDataSet
// Description  Handles buffer data set
// Arguments: GLuint buffer
//            GLsizeiptr sizeOfData
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/9/2009
// ---------------------------------------------------------------------------
void gsVBOMonitor::onVertexBufferObjectDataSet(GLenum target, GLsizeiptr sizeOfData)
{
    apGLVBO* pBoundVBO = getBoundVBO(target);

    if (pBoundVBO != NULL)
    {
        pBoundVBO->setSize(sizeOfData);
        pBoundVBO->markAsDirty(true);
    }
}

// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::onVertexBufferObjectDirectAccessSubDataSet
// Description: Handle VBO data set for direct state access extension
// Arguments: GLuint buffer - the buffer name
//            GLenum target - the target
//            GLsizeiptr sizeOfData - size of data set
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/9/2009
// ---------------------------------------------------------------------------
void gsVBOMonitor::onVertexBufferObjectDirectAccessSubDataSet(GLuint buffer)
{
    apGLVBO* pVBO = getVBODetails(buffer);

    if (pVBO != NULL)
    {
        pVBO->markAsDirty(true);
    }
}


// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::onVertexBufferObjectSubDataSet
// Description: Handle VBO data set for direct state access extension
// Arguments: GLuint buffer - the buffer name
//            GLenum target - the target
//            GLsizeiptr sizeOfData - size of data set
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/9/2009
// ---------------------------------------------------------------------------
void gsVBOMonitor::onVertexBufferObjectSubDataSet(GLenum target)
{
    // Get the VBO bound to the requested taret:
    apGLVBO* pBoundVBO = getBoundVBO(target);

    if (pBoundVBO != NULL)
    {
        pBoundVBO->markAsDirty(true);
    }
}

// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::getBoundVBO
// Description: Given a target, return the bound VBO to this target
// Arguments: GLenum target
// Return Val: apGLVBO*
// Author:      Sigal Algranaty
// Date:        15/9/2009
// ---------------------------------------------------------------------------
apGLVBO* gsVBOMonitor::getBoundVBO(GLenum target)
{
    apGLVBO* pRetVal = NULL;
    GLuint vboName = getAttachedVBOName(target);

    if (vboName > 0)
    {
        gtMap<GLuint, int>::const_iterator endIter = _vboOpenGLNameToIndex.end();
        gtMap<GLuint, int>::const_iterator iter = _vboOpenGLNameToIndex.find(vboName);

        GT_IF_WITH_ASSERT(iter != endIter)
        {
            int vboIndex = (*iter).second;
            // If the monitored context is NOT the NULL context:
            GT_IF_WITH_ASSERT(_spyContextId != AP_NULL_CONTEXT_ID)
            {
                GT_IF_WITH_ASSERT((vboIndex >= 0) && (vboIndex < (int)_vbos.size()))
                {
                    pRetVal = _vbos[vboIndex];
                }
            }
        }
    }

    return pRetVal;
}
// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::getVBOIndex
// Description: Inputs an OpenGL vertex buffer object name and outputs index
// Author:      Sigal Algranaty
// Date:        22/10/2008
// ---------------------------------------------------------------------------
size_t gsVBOMonitor::getVBOIndex(GLuint vboName) const
{
    size_t retVal = 0;

    gtMap<GLuint, int>::const_iterator endIter = _vboOpenGLNameToIndex.end();
    gtMap<GLuint, int>::const_iterator iter = _vboOpenGLNameToIndex.find(vboName);

    if (iter != endIter)
    {
        retVal = (*iter).second;
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::getVBOName
// Description: Returns an VBO object name according to the FBO index in the list
// Arguments: int vboIndex
//            GLuint& vboName
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/10/2008
// ---------------------------------------------------------------------------
bool gsVBOMonitor::getVBOName(int vboIndex, GLuint& vboName) const
{
    bool retVal = false;
    GT_IF_WITH_ASSERT((vboIndex >= 0) && (vboIndex < (int)_vbos.size()))
    {
        if (_vbos[vboIndex] != NULL)
        {
            // Get the FBO object from the list of FBOs:
            apGLVBO* pVbo = _vbos[vboIndex];

            // Get the FBO name:
            vboName = pVbo->name();

            retVal = true;
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::setVBODisplayProperties
// Description: Sets a VBO display properties
// Arguments: GLuint vboName
//            oaTexelDataFormat displayFormat
//            int offset
//            GLsizei stride
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/4/2009
// ---------------------------------------------------------------------------
bool gsVBOMonitor::setVBODisplayProperties(GLuint vboName, oaTexelDataFormat displayFormat, int offset, GLsizei stride)
{
    bool retVal = false;

    // Get the VBO index by name:
    size_t i = getVBOIndex(vboName);

    if ((i < _vbos.size()) && (i != (size_t)(-1)))
    {
        // Get the VBO object:
        apGLVBO* pCurrentVBO = _vbos[i];
        GT_IF_WITH_ASSERT(pCurrentVBO != NULL)
        {
            // Set the VBO properties:
            pCurrentVBO->setBufferDisplayProperties(displayFormat, offset, stride);
            retVal = true;
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::updateVBORawData
// Description: Updates a given VBO raw data file.
// Arguments:   pVBO - The VBO to be updated
//              oaTexelDataFormat dataFormat - the buffer data format. For texture
//              buffers, this format is taken from the texture, and for other buffers,
//              it is a default value set by us.
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/4/2009
// ---------------------------------------------------------------------------
bool gsVBOMonitor::updateVBORawData(apGLVBO* pVBO, oaTexelDataFormat dataFormat)
{
    (void)(dataFormat); // unused
    bool retVal = false;
#ifndef _GR_IPHONE_BUILD
    GT_IF_WITH_ASSERT(pVBO != NULL)
    {
        bool rc1 = (pVBO->name() != 0);
        GT_IF_WITH_ASSERT(rc1)
        {
            // If the VBO is "dirty":
            if (pVBO->isDirty())
            {
                // If this is a GL-CL interop buffer, print a warning message to the log, since we might
                // run into undefined behavior if the buffer is acquired by OpenCL:
                if (pVBO->openCLBufferName() > -1)
                {
                    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_aboutToReadCLObject, OS_DEBUG_LOG_INFO);
                }

                // Get the VBO name:
                GLuint vboName = pVBO->name();
                GT_IF_WITH_ASSERT(0 != vboName)
                {
                    // Bind the VBO in order to get its data:
                    GLenum target =  getVBOLatestAttachment(vboName);
                    GLuint currentlyBoundVBO = getAttachedVBOName(target);
                    bool rc = true;

                    if (vboName != currentlyBoundVBO)
                    {
                        rc = bindVBO(vboName, target);
                    }

                    GT_IF_WITH_ASSERT(rc)
                    {
                        // Generate the texture file name:pgd
                        osFilePath bufferFilePath;
                        generateVBOFilePath(vboName, bufferFilePath);

                        // Set the buffer file path:
                        pVBO->setBufferFilePath(bufferFilePath);

                        // Save the buffer content to a file:
                        gsBufferSerializer bufferSerializer;
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                        bool rc2 = bufferSerializer.saveBufferToFile(*pVBO, target, 0, bufferFilePath, _glGetBufferSubData);
#else
                        bool rc2 = bufferSerializer.saveBufferToFile(*pVBO, target, 0, bufferFilePath);
#endif
                        GT_ASSERT(rc2);

                        // Restore the previously bound VBO:
                        bool rc3 = true;

                        if (vboName != currentlyBoundVBO)
                        {
                            bindVBO(currentlyBoundVBO, target);
                        }

                        GT_ASSERT(rc3);

                        if (rc2 && rc3)
                        {
                            retVal = true;
                        }

                        if (!retVal)
                        {
                            // Raise an assertion failure:
                            // TO_DO: Move me to string table:
                            gtString errorMessage = L"Failed to save buffer to a file: ";
                            errorMessage += bufferFilePath.asString();
                            GT_ASSERT_EX(false, errorMessage.asCharArray());
                        }
                    }
                }
            }
        }
    }
    else
    {
        // The buffer is not connect to an FBO object:
    }

#endif

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::generateVBOFilePath
// Description: Generates a VBO file path.
// Arguments:   bufferName - The VBO OpenGL name.
//              bufferFilePath - The output texture file path.
// Author:      Sigal Algranaty
// Date:        6/4/2009
// ---------------------------------------------------------------------------
void gsVBOMonitor::generateVBOFilePath(GLuint vboName, osFilePath& bufferFilePath) const
{
    // Build the log file name:
    gtString logFileName;
    logFileName.appendFormattedString(GS_STR_vboFilePath, _spyContextId, vboName);

    // Set the log file path:
    bufferFilePath = suCurrentSessionLogFilesDirectory();
    bufferFilePath.setFileName(logFileName);

    // Set the log file extension:
    bufferFilePath.setFileExtension(SU_STR_rawFileExtension);
}



// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::bindVBO
// Description: Bind a VBO, and set the currently bound VBO
// Arguments:   GLuint vboName
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/4/2009
// ---------------------------------------------------------------------------
bool gsVBOMonitor::bindVBO(GLuint vboName, GLenum target)
{
    bool retVal = false;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
#define _glBindBuffer gs_stat_realFunctionPointers.glBindBuffer
#endif

    // Clear any previous OpenGL errors:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
    GLenum previousError = gs_stat_realFunctionPointers.glGetError();
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

    GT_ASSERT(previousError == GL_NO_ERROR);

    // Bind the VBO:
    if (vboName != 0 && target != 0)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindBuffer);
        _glBindBuffer(target, vboName);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindBuffer);

        // Get OpenGL errors (if any were generated by glBindBuffer):
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
        GLenum currentError = gs_stat_realFunctionPointers.glGetError();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);
        GT_IF_WITH_ASSERT(currentError == GL_NO_ERROR)
        {
            retVal = true;
        }
    }

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
#undef _glBindBuffer
#endif
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::onFirstTimeContextMadeCurrent
// Description: Is called on the first time in which my VBO's monitor context
//              is made the current context.
// Return Val: void
// Author:      Sigal Algranaty
// Date:        6/4/2009
// ---------------------------------------------------------------------------
void gsVBOMonitor::onFirstTimeContextMadeCurrent()
{
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    // Initialize GL_ARB_shader_objects function pointers:
    _glBindBuffer = (PFNGLBINDBUFFERPROC)gsGetSystemsOGLModuleProcAddress("glBindBuffer");
    _glGetBufferSubData = (PFNGLGETBUFFERSUBDATAPROC)gsGetSystemsOGLModuleProcAddress("glGetBufferSubData");
#else
    // MAC OS X - OpenGL is the base level:
#endif
}


// ---------------------------------------------------------------------------
// Name:        gsVBOMonitor::calculateBuffersMemorySize
// Description: Calculate the current existing VBOs memory size
// Arguments:   gtUInt64& buffersMemorySize
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/7/2010
// ---------------------------------------------------------------------------
bool gsVBOMonitor::calculateBuffersMemorySize(gtUInt64& buffersMemorySize) const
{
    bool retVal = true;
    buffersMemorySize = 0;

    // Iterate the VBOs:
    for (int i = 0; i < (int)_vbos.size(); i++)
    {
        // Get the current VBO:
        apGLVBO* pVBO = _vbos[i];

        if (pVBO != NULL)
        {
            // Set the object size in KB (ceil the number for 0.1KB to turn to be 1KB):
            gtSize_t vboSize = pVBO->size();

            if (vboSize > 0)
            {
                float val = (float)vboSize / (1024.0F);
                vboSize = (gtSize_t)ceil(val);

                if (vboSize == 0)
                {
                    vboSize = 1;
                }

                buffersMemorySize += vboSize;
            }
        }
    }

    return retVal;
}
