//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csImagesAndBuffersMonitor.cpp
///
//==================================================================================

//------------------------------ csImagesAndBuffersMonitor.cpp ------------------------------

// Standard C:
#include <math.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Spy Utilities:
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suAPIConnector.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>
#include <AMDTServerUtilities/Include/suInterceptionMacros.h>

// Local:
#include <src/csImagesAndBuffersMonitor.h>
#include <src/csOpenCLMonitor.h>
#include <src/csGlobalVariables.h>
#include <src/csMonitoredFunctionPointers.h>
#include <src/csStringConstants.h>
#include <src/csBufferSerializer.h>


typedef bool (* GSSHAREGLTEXTUREWITHCLIMAGEPROC)(int clImageIndex, int clImageName, int clSpyID, int glSpyID, GLuint glTextureName, int textureMipLevel, GLenum textureTarget, gtString& detectedErrorStr);
typedef bool (* GSSHAREGLRENDERBUFFERWITHCLIMAGEPROC)(int clImageIndex, int clImageName, int clSpyID, int glSpyID, GLuint glRenderBufferName, gtString& detectedErrorStr);
typedef bool (* GSSHAREGLVBOWITHCLBUFFERPROC)(int clBufferIndex, int bufferName, int clSpyID, int glSpyID, GLuint glVBOName, gtString& detectedErrorStr);

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::csImagesAndBuffersMonitor
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        18/11/2009
// ---------------------------------------------------------------------------
csImagesAndBuffersMonitor::csImagesAndBuffersMonitor(): _spyContextId(0), _isMemObjectDestructorCallbackSupported(false), _nextFreeBufferName(1), _nextFreeSubBufferName(1), _nextFreeImageName(1), m_nextFreePipeName(1), _commandQueue(OA_CL_NULL_HANDLE)
{
}


// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::~csImagesAndBuffersMonitor
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        18/11/2009
// ---------------------------------------------------------------------------
csImagesAndBuffersMonitor::~csImagesAndBuffersMonitor()
{
    // If we created a command queue, release it:
    destroyCommandQueue();
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::onBufferCreation
// Description: Handles buffer object creation
// Author:      Sigal Algranaty
// Date:        1/12/2009
// ---------------------------------------------------------------------------
void csImagesAndBuffersMonitor::onBufferCreation(cl_mem bufferMemoryHandle, cl_mem_flags flags, size_t size)
{
    // Create the buffer object:
    int bufferName = _nextFreeBufferName;
    apCLBuffer* pBufferObject = new apCLBuffer(bufferName);

    // Increase buffer next free index:
    _nextFreeBufferName++;

    // Register this object in the allocated objects monitor:
    su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pBufferObject);

    // Set buffer properties:
    pBufferObject->setMemoryFlags(flags);
    pBufferObject->setBufferSize(size);
    pBufferObject->setMemObjectHandle((oaCLMemHandle)bufferMemoryHandle);

    // Add the monitor to the vector of buffers:
    _bufferMonitors.push_back(pBufferObject);

    // Get the handles monitor:
    csOpenCLHandleMonitor& handlesMonitor = cs_stat_openCLMonitorInstance.openCLHandleMonitor();
    int objectIndex = (int)_bufferMonitors.size() - 1;
    handlesMonitor.registerOpenCLHandle((oaCLHandle)bufferMemoryHandle, _spyContextId, objectIndex, OS_TOBJ_ID_CL_BUFFER, -1, bufferName);

    // Set the mapping for the buffer name:
    _bufferNameToIndexMap.insert(pair<int, int>(bufferName, objectIndex));

    // Register a callback to handle this memory object deletion:
    if (_isMemObjectDestructorCallbackSupported && (cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback != NULL))
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
        cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback(bufferMemoryHandle, &csImagesAndBuffersMonitor::onMemoryObjectDeletion, this);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
    }
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::onSubBufferCreation
// Description: Handles sub buffer object create
// Arguments:   cl_mem subBufferMemoryHandle - the sub buffer memory handle
//              cl_mem bufferMemoryHandle - the owner buffer memory handle
//              cl_mem_flags flags - the memory flags
//              cl_buffer_create_type buffer_create_type - the buffer creation type
//              size_t size - the sub buffer size
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        26/10/2010
// ---------------------------------------------------------------------------
void csImagesAndBuffersMonitor::onSubBufferCreation(cl_mem subBufferMemoryHandle, cl_mem bufferMemoryHandle, cl_mem_flags flags, cl_buffer_create_type buffer_create_type, const void* buffer_create_info)
{
    // NOTICE: The OpenCL documentation specifies that on multiple calls to clCreateSubBuffer with the same arguments OpenCL should return the
    // same memory object with the reference count increased. Therefore, before we add the sub-buffer we check if it already exist:
    const apCLMemObject* pExistingSubBuffer = getMemObjectDetails((oaCLMemHandle)subBufferMemoryHandle);

    if (pExistingSubBuffer == NULL)
    {

        // Check if this sub buffer is already monitored:
        // Get the sub buffer name:
        int subBufferName = _nextFreeSubBufferName;

        // Search for my buffer object:
        int bufferName = -1;
        apCLBuffer* pBufferObject = NULL;
        const apCLMemObject* pMemoryObject = getMemObjectDetails((oaCLMemHandle)bufferMemoryHandle);
        GT_IF_WITH_ASSERT(pMemoryObject != NULL)
        {
            // Convert to buffer object:
            pBufferObject = NULL;
            GT_IF_WITH_ASSERT(pMemoryObject->type() == OS_TOBJ_ID_CL_BUFFER)
            {
                pBufferObject = (apCLBuffer*)pMemoryObject;
                GT_IF_WITH_ASSERT(pBufferObject != NULL)
                {
                    // Get the buffer name:
                    bufferName = pBufferObject->bufferName();

                    // Add the new sub buffer to the buffers sub buffers:
                    pBufferObject->addSubBufferIndex(subBufferName);
                }
            }
        }

        // Create the Sub Buffer object:
        apCLSubBuffer* pSubBufferObject = new apCLSubBuffer(bufferName, subBufferName);


        // Increase buffer next free index:
        _nextFreeSubBufferName++;

        // Register this object in the allocated objects monitor:
        su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pSubBufferObject);

        // Set buffer properties:
        pSubBufferObject->setMemoryFlags(flags);
        pSubBufferObject->setMemObjectHandle((oaCLMemHandle)subBufferMemoryHandle);
        GT_IF_WITH_ASSERT(buffer_create_type == CL_BUFFER_CREATE_TYPE_REGION)
        {
            // Convert the buffer create info to a buffer region struct:
            GT_IF_WITH_ASSERT(buffer_create_info != NULL)
            {
                cl_buffer_region* pBufferRegion = (cl_buffer_region*)buffer_create_info;
                GT_IF_WITH_ASSERT(pBufferRegion != NULL)
                {
                    // Set the buffer region:
                    pSubBufferObject->setBufferRegion(*pBufferRegion);
                }
            }
        }

        // Add the monitor to the vector of sub buffers:
        _subBufferMonitors.push_back(pSubBufferObject);

        // Get the handles monitor:
        csOpenCLHandleMonitor& handlesMonitor = cs_stat_openCLMonitorInstance.openCLHandleMonitor();
        int objectIndex = (int)_subBufferMonitors.size() - 1;
        handlesMonitor.registerOpenCLHandle((oaCLHandle)subBufferMemoryHandle, _spyContextId, objectIndex, OS_TOBJ_ID_CL_SUB_BUFFER, -1, subBufferName);

        // Register a callback to handle this memory object deletion:
        if (_isMemObjectDestructorCallbackSupported && (cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback != NULL))
        {
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
            cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback(bufferMemoryHandle, &csImagesAndBuffersMonitor::onMemoryObjectDeletion, this);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::onBufferCreationFromGLVBO
// Description: Called when an OpenCL buffer is created the OpenGL VBO bufobj
//              with clCreateFromGLBuffer
// Author:      Uri Shomroni
// Date:        4/3/2010
// ---------------------------------------------------------------------------
void csImagesAndBuffersMonitor::onBufferCreationFromGLVBO(cl_mem bufferMemoryHandle, cl_mem_flags flags, GLuint bufobj)
{
    // Create the buffer object:
    int bufferName = _nextFreeBufferName;
    apCLBuffer* pBufferObject = new apCLBuffer(bufferName);

    // Increase buffer next free index:
    _nextFreeBufferName++;

    // Register this object in the allocated objects monitor:
    su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pBufferObject);

    // Set buffer properties:
    pBufferObject->setMemoryFlags(flags);

    // Get the buffer size from OpenCL:
    gtSize_t bufferSize = 0;
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetMemObjectInfo);
    cl_int rcSize = cs_stat_realFunctionPointers.clGetMemObjectInfo(bufferMemoryHandle, CL_MEM_SIZE, sizeof(gtSize_t), (void*)(&bufferSize), NULL);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetMemObjectInfo);
    GT_IF_WITH_ASSERT(rcSize == CL_SUCCESS)
    {
        pBufferObject->setBufferSize(bufferSize);
    }

    // Set the buffer memory handle:
    pBufferObject->setMemObjectHandle((oaCLMemHandle)bufferMemoryHandle);

    // Add the monitor to the vector of buffers:
    _bufferMonitors.push_back(pBufferObject);

    // Get the handles monitor:
    csOpenCLHandleMonitor& handlesMonitor = cs_stat_openCLMonitorInstance.openCLHandleMonitor();
    int bufferIndex = (int)_bufferMonitors.size() - 1;
    handlesMonitor.registerOpenCLHandle((oaCLHandle)bufferMemoryHandle, _spyContextId, bufferIndex, OS_TOBJ_ID_CL_BUFFER, -1, bufferName);

    // Set the mapping for the buffer name:
    _bufferNameToIndexMap.insert(pair<int, int>(bufferName, bufferIndex));

    // Register a callback to handle this memory object deletion:
    if (_isMemObjectDestructorCallbackSupported && (cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback != NULL))
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
        cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback(bufferMemoryHandle, &csImagesAndBuffersMonitor::onMemoryObjectDeletion, this);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
    }

    // Handle OpenGL interoperability:
    handleBufferShareWithGLVBO(pBufferObject, bufferIndex, bufobj);

}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::onBufferCreationFromDirectX
// Description: Called when an OpenCL buffer is created from a DirectX buffer
//              with clCreateFromD3D*BufferKHR
// Author:      Uri Shomroni
// Date:        2/10/2014
// ---------------------------------------------------------------------------
void csImagesAndBuffersMonitor::onBufferCreationFromDirectX(cl_mem bufferMemoryHandle, cl_mem_flags flags)
{
    // Create the buffer object:
    int bufferName = _nextFreeBufferName;
    apCLBuffer* pBufferObject = new apCLBuffer(bufferName);

    // Increase buffer next free index:
    _nextFreeBufferName++;

    // Register this object in the allocated objects monitor:
    su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pBufferObject);

    // Set buffer properties:
    pBufferObject->setMemoryFlags(flags);

    // Get the buffer size from OpenCL:
    gtSize_t bufferSize = 0;
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetMemObjectInfo);
    cl_int rcSize = cs_stat_realFunctionPointers.clGetMemObjectInfo(bufferMemoryHandle, CL_MEM_SIZE, sizeof(gtSize_t), (void*)(&bufferSize), NULL);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetMemObjectInfo);
    GT_IF_WITH_ASSERT(rcSize == CL_SUCCESS)
    {
        pBufferObject->setBufferSize(bufferSize);
    }

    // Set the buffer memory handle:
    pBufferObject->setMemObjectHandle((oaCLMemHandle)bufferMemoryHandle);

    // Add the monitor to the vector of buffers:
    _bufferMonitors.push_back(pBufferObject);

    // Get the handles monitor:
    csOpenCLHandleMonitor& handlesMonitor = cs_stat_openCLMonitorInstance.openCLHandleMonitor();
    int bufferIndex = (int)_bufferMonitors.size() - 1;
    handlesMonitor.registerOpenCLHandle((oaCLHandle)bufferMemoryHandle, _spyContextId, bufferIndex, OS_TOBJ_ID_CL_BUFFER, -1, bufferName);

    // Set the mapping for the buffer name:
    _bufferNameToIndexMap.insert(pair<int, int>(bufferName, bufferIndex));

    // Register a callback to handle this memory object deletion:
    if (_isMemObjectDestructorCallbackSupported && (cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback != NULL))
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
        cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback(bufferMemoryHandle, &csImagesAndBuffersMonitor::onMemoryObjectDeletion, this);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
    }
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::onPipeCreation
// Description: Handles pipe object creation
// Author:      Uri Shomroni
// Date:        1/10/2014
// ---------------------------------------------------------------------------
void csImagesAndBuffersMonitor::onPipeCreation(cl_mem pipeMemoryHandle, cl_mem_flags flags, cl_uint packetSize, cl_uint maxPackets)
{
    // Create the pipe object:
    int pipeName = m_nextFreePipeName++;
    apCLPipe* pPipeObject = new apCLPipe((gtInt32)pipeName, (gtUInt32)packetSize, (gtUInt32)maxPackets);

    // Register this object in the allocated objects monitor:
    su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pPipeObject);

    // Set pipe properties:
    pPipeObject->setMemoryFlags(flags);
    pPipeObject->setMemObjectHandle((oaCLMemHandle)pipeMemoryHandle);

    // Add the monitor to the vector of pipes:
    m_pipeMonitors.push_back(pPipeObject);

    // Get the handles monitor:
    csOpenCLHandleMonitor& handlesMonitor = cs_stat_openCLMonitorInstance.openCLHandleMonitor();
    int objectIndex = (int)m_pipeMonitors.size() - 1;
    handlesMonitor.registerOpenCLHandle((oaCLHandle)pipeMemoryHandle, _spyContextId, objectIndex, OS_TOBJ_ID_CL_PIPE, -1, pipeName);

    // Set the mapping for the pipe name:
    m_pipeNameToIndexMap[pipeName] = objectIndex;

    // Register a callback to handle this memory object deletion:
    if (_isMemObjectDestructorCallbackSupported && (cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback != NULL))
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
        cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback(pipeMemoryHandle, &csImagesAndBuffersMonitor::onMemoryObjectDeletion, this);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
    }
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::amountOfBuffers
// Description: Returns the amount of logged buffers.
// Author:      Yaki Tebeka
// Date:        18/11/2009
// ---------------------------------------------------------------------------
int csImagesAndBuffersMonitor::amountOfBuffers() const
{
    int retVal = (int)_bufferMonitors.size();
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::bufferDetails
// Description: Inputs a buffer index and return it's monitor.
// Author:      Yaki Tebeka
// Date:        18/11/2009
// ---------------------------------------------------------------------------
const apCLBuffer* csImagesAndBuffersMonitor::bufferDetails(int bufferId) const
{
    const apCLBuffer* retVal = NULL;

    // Index range check:
    int buffersAmount = (int)_bufferMonitors.size();
    GT_IF_WITH_ASSERT((0 <= buffersAmount) && (bufferId < buffersAmount))
    {
        retVal = _bufferMonitors[bufferId];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::bufferDetails
// Description: Inputs a buffer index and return it's monitor.
// Author:      Yaki Tebeka
// Date:        18/11/2009
// ---------------------------------------------------------------------------
apCLBuffer* csImagesAndBuffersMonitor::bufferDetails(int bufferId)
{
    apCLBuffer* retVal = NULL;

    // Index range check:
    int buffersAmount = (int)_bufferMonitors.size();
    GT_IF_WITH_ASSERT((0 <= bufferId) && (bufferId < buffersAmount))
    {
        retVal = _bufferMonitors[bufferId];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::subBufferDetails
// Description: Inputs a sub buffer index and return it's monitor.
// Author:      Sigal Algranaty
// Date:        26/10/2010
// ---------------------------------------------------------------------------
const apCLSubBuffer* csImagesAndBuffersMonitor::subBufferDetails(int subBufferName) const
{
    const apCLSubBuffer* retVal = NULL;

    // Sub buffer index is the name -1:
    int subBufferIndex = subBufferName - 1;

    // Index range check:
    int subBuffersAmount = (int)_subBufferMonitors.size();
    GT_IF_WITH_ASSERT((0 <= subBufferIndex) && (subBufferIndex < subBuffersAmount))
    {
        retVal = _subBufferMonitors[subBufferIndex];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::subBufferDetails
// Description: Inputs a sub buffer index and return it's monitor.
// Arguments:   subBufferName - the sub buffer name (its index - 1)
// Author:      Sigal Algranaty
// Date:        26/10/2010
// ---------------------------------------------------------------------------
apCLSubBuffer* csImagesAndBuffersMonitor::subBufferDetails(int subBufferName)
{
    apCLSubBuffer* retVal = NULL;

    // Sub buffer index is the name -1:
    int subBufferIndex = subBufferName - 1;

    // Index range check:
    int subBuffersAmount = (int)_subBufferMonitors.size();
    GT_IF_WITH_ASSERT((0 <= subBufferIndex) && (subBufferIndex < subBuffersAmount))
    {
        retVal = _subBufferMonitors[subBufferIndex];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::updateBufferRawData
// Description: Updates a buffer raw data
// Arguments:   bufferId - the updated buffer index
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool csImagesAndBuffersMonitor::updateBufferRawData(int bufferIndex)
{
    bool retVal = false;

    // Get the buffer object:
    apCLBuffer* pBufferDetails = bufferDetails(bufferIndex);
    GT_IF_WITH_ASSERT(pBufferDetails != NULL)
    {
        retVal = true;

        // If the buffer is "dirty":
        if (pBufferDetails->isDirty())
        {
            // Generate the buffer file name:
            osFilePath bufferFilePath;
            generateRawDataFilePath(bufferIndex, bufferFilePath, true);

            // Set the buffer file path:
            pBufferDetails->setBufferFilePath(bufferFilePath);

            // Initialize the command queue used for reading the buffer:
            bool rc = initializeCommandQueue();
            GT_IF_WITH_ASSERT(rc && (_commandQueue != OA_CL_NULL_HANDLE))
            {
                // Save the buffer content to a file:
                csBufferSerializer bufferSerializer;
                retVal = bufferSerializer.saveBufferToFile(*pBufferDetails, bufferFilePath, _commandQueue);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::updateSubBufferRawData
// Description: Updates a sub-buffer raw data
// Arguments:   subBufferId - the updated buffer index
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/1/2011
// ---------------------------------------------------------------------------
bool csImagesAndBuffersMonitor::updateSubBufferRawData(int subBufferId)
{
    bool retVal = false;

    // Get the sub-buffer object:
    apCLSubBuffer* pSubBufferDetails = subBufferDetails(subBufferId);
    GT_IF_WITH_ASSERT(pSubBufferDetails != NULL)
    {
        // Get the index for the buffer object:
        int bufferIndex = bufferObjectMonitorIndex(pSubBufferDetails->bufferName());

        // Get the details for the owner buffer:
        apCLBuffer* pBufferDetails = bufferDetails(bufferIndex);
        GT_IF_WITH_ASSERT(pBufferDetails != NULL)
        {

            retVal = true;

            // If the buffer is "dirty":
            if (pSubBufferDetails->isDirty())
            {
                // Generate the buffer file name:
                osFilePath subBufferFilePath;
                generateRawDataFilePath(subBufferId, subBufferFilePath, true);

                // Set the buffer file path:
                pSubBufferDetails->setSubBufferFilePath(subBufferFilePath);

                // Initialize the command queue used for reading the buffer:
                bool rc = initializeCommandQueue();
                GT_IF_WITH_ASSERT(rc && (_commandQueue != OA_CL_NULL_HANDLE))
                {
                    // Save the buffer content to a file:
                    csBufferSerializer bufferSerializer;
                    retVal = bufferSerializer.saveSubBufferToFile(*pSubBufferDetails, *pBufferDetails, subBufferFilePath, _commandQueue);
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::updateTextureRawData
// Description: Updates a texture raw data
// Arguments:   bufferId - the updated texture id
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool csImagesAndBuffersMonitor::updateTextureRawData(int textureId)
{
    bool retVal = false;

    // Get the texture object:
    apCLImage* pTextureDetails = imageDetails(textureId);
    GT_IF_WITH_ASSERT(pTextureDetails != NULL)
    {
        retVal = true;

        // If the texture is "dirty":
        if (pTextureDetails->isDirty())
        {
            // Get the GL texture details. If this is an interop object, do not try to extract the image data (this feature is not supported):
            if (pTextureDetails->openGLSpyID() > 0)
            {
                gtString debugMessage;
                debugMessage.appendFormattedString(L"The CL image is a CL-GL interop object, which is data update is not supported. (CL Image ID: %d. GL spy ID name: %d", pTextureDetails->imageName(), pTextureDetails->openGLSpyID());
                OS_OUTPUT_DEBUG_LOG(debugMessage.asCharArray(), OS_DEBUG_LOG_INFO);
            }
            else
            {
                // Generate the texture file name:
                osFilePath textureFilePath;
                generateRawDataFilePath(textureId, textureFilePath, false);

                // Set the texture file path:
                pTextureDetails->setImageFilePath(textureFilePath);

                // Initialize the command queue used for reading the buffer:
                bool rc = initializeCommandQueue();
                GT_IF_WITH_ASSERT(rc && (_commandQueue != OA_CL_NULL_HANDLE))
                {
                    // Save the buffer content to a file:
                    csBufferSerializer bufferSerializer;
                    retVal = bufferSerializer.saveTextureToFile(*pTextureDetails, textureFilePath, _commandQueue);
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::setContextOpenCLVersion
// Description: This class uses some operations which are permitted only on certain
//              OpenCL versions. Use the given version to check which ones are
//              available to us.
// Author:      Uri Shomroni
// Date:        28/12/2010
// ---------------------------------------------------------------------------
void csImagesAndBuffersMonitor::setContextOpenCLVersion(int majorVersion, int minorVersion)
{
    // Memory object destructor callback is supported in OpenGL 1.1 and up:
    if (((majorVersion == 1) && (minorVersion >= 1)) || (majorVersion > 1))
    {
        _isMemObjectDestructorCallbackSupported = true;
    }
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::getMemObjectDetails
// Description: Gets a mem object's details by its handle. The returned object
//              is generic (not specifically a buffer or a texture).
// Author:      Uri Shomroni
// Date:        18/2/2010
// ---------------------------------------------------------------------------
const apCLMemObject* csImagesAndBuffersMonitor::getMemObjectDetails(oaCLMemHandle memObjHandle) const
{
    const apCLMemObject* retVal = NULL;

    bool foundLiving = false;

    // Try to find this object in the buffers vector:
    int numberOfBuffers = (int)_bufferMonitors.size();

    for (int i = 0; i < numberOfBuffers; i++)
    {
        // Get the current buffer:
        const apCLBuffer* pCurrentBuffer = _bufferMonitors[i];
        GT_IF_WITH_ASSERT(pCurrentBuffer != NULL)
        {
            // If this is the requested mem object:
            if (memObjHandle == pCurrentBuffer->memObjectHandle())
            {
                retVal = pCurrentBuffer;

                if (!pCurrentBuffer->wasMarkedForDeletion())
                {
                    foundLiving = true;
                    break;
                }
            }
        }
    }

    if (!foundLiving)
    {
        // Try to find this object in the sub buffers vector:
        int numberOfSubBuffers = (int)_subBufferMonitors.size();

        for (int i = 0; i < numberOfSubBuffers; i++)
        {
            // Get the current buffer:
            const apCLSubBuffer* pCurrentSubBuffer = _subBufferMonitors[i];
            GT_IF_WITH_ASSERT(pCurrentSubBuffer != NULL)
            {
                // If this is the requested mem object:
                if (memObjHandle == pCurrentSubBuffer->memObjectHandle())
                {
                    retVal = pCurrentSubBuffer;

                    if (!pCurrentSubBuffer->wasMarkedForDeletion())
                    {
                        foundLiving = true;
                        break;
                    }
                }
            }
        }
    }

    // If the object is not a buffer:
    if (!foundLiving)
    {
        // Try to find this object in the textures vector:
        int numberOfTextures = (int)_imagesMonitors.size();

        for (int i = 0; i < numberOfTextures; i++)
        {
            // Get the current texture:
            const apCLImage* pCurrentTexture = _imagesMonitors[i];
            GT_IF_WITH_ASSERT(pCurrentTexture != NULL)
            {
                // If this is the requested mem object:
                if (memObjHandle == pCurrentTexture->memObjectHandle())
                {
                    retVal = pCurrentTexture;

                    if (!pCurrentTexture->wasMarkedForDeletion())
                    {
                        foundLiving = true;
                        break;
                    }
                }
            }
        }
    }

    // If the object is not a buffer nor an image:
    if (!foundLiving)
    {
        // Try to find this object in the pipes vector:
        int numberOfPipes = (int)m_pipeMonitors.size();

        for (int i = 0; i < numberOfPipes; i++)
        {
            // Get the current pipe:
            const apCLPipe* pCurrentPipe = m_pipeMonitors[i];
            GT_IF_WITH_ASSERT(pCurrentPipe != NULL)
            {
                // If this is the requested mem object:
                if (memObjHandle == pCurrentPipe->memObjectHandle())
                {
                    retVal = pCurrentPipe;

                    if (!pCurrentPipe->wasMarkedForDeletion())
                    {
                        foundLiving = true;
                        break;
                    }
                }
            }
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::getMemObjectDetails
// Description: Gets a mem object's mutable details by its handle. The returned
//              object is generic (not specifically a buffer or a texture).
// Author:      Uri Shomroni
// Date:        18/2/2010
// ---------------------------------------------------------------------------
apCLMemObject* csImagesAndBuffersMonitor::getMemObjectDetails(oaCLMemHandle memObjHandle)
{
    apCLMemObject* retVal = NULL;

    bool foundLiving = false;

    // Try to find this object in the buffers vector:
    int numberOfBuffers = (int)_bufferMonitors.size();

    for (int i = 0; i < numberOfBuffers; i++)
    {
        // Get the current buffer:
        apCLBuffer* pCurrentBuffer = _bufferMonitors[i];
        GT_IF_WITH_ASSERT(pCurrentBuffer != NULL)
        {
            // If this is the requested mem object:
            if (memObjHandle == pCurrentBuffer->memObjectHandle())
            {
                retVal = pCurrentBuffer;

                if (!pCurrentBuffer->wasMarkedForDeletion())
                {
                    foundLiving = true;
                    break;
                }
            }
        }
    }

    if (!foundLiving)
    {
        // Try to find this object in the sub buffers vector:
        int numberOfSubBuffers = (int)_subBufferMonitors.size();

        for (int i = 0; i < numberOfSubBuffers; i++)
        {
            // Get the current buffer:
            apCLSubBuffer* pCurrentSubBuffer = _subBufferMonitors[i];
            GT_IF_WITH_ASSERT(pCurrentSubBuffer != NULL)
            {
                // If this is the requested mem object:
                if (memObjHandle == pCurrentSubBuffer->memObjectHandle())
                {
                    retVal = pCurrentSubBuffer;

                    if (!pCurrentSubBuffer->wasMarkedForDeletion())
                    {
                        foundLiving = true;
                        break;
                    }
                }
            }
        }
    }

    // If the object is not a buffer:
    if (!foundLiving)
    {
        // Try to find this object in the textures vector:
        int numberOfTextures = (int)_imagesMonitors.size();

        for (int i = 0; i < numberOfTextures; i++)
        {
            // Get the current texture:
            apCLImage* pCurrentTexture = _imagesMonitors[i];
            GT_IF_WITH_ASSERT(pCurrentTexture != NULL)
            {
                // If this is the requested mem object:
                if (memObjHandle == pCurrentTexture->memObjectHandle())
                {
                    retVal = pCurrentTexture;

                    if (!pCurrentTexture->wasMarkedForDeletion())
                    {
                        foundLiving = true;
                        break;
                    }
                }
            }
        }
    }

    // If the object is not a buffer nor an image:
    if (!foundLiving)
    {
        // Try to find this object in the pipes vector:
        int numberOfPipes = (int)m_pipeMonitors.size();

        for (int i = 0; i < numberOfPipes; i++)
        {
            // Get the current pipe:
            apCLPipe* pCurrentPipe = m_pipeMonitors[i];
            GT_IF_WITH_ASSERT(pCurrentPipe != NULL)
            {
                // If this is the requested mem object:
                if (memObjHandle == pCurrentPipe->memObjectHandle())
                {
                    retVal = pCurrentPipe;

                    if (!pCurrentPipe->wasMarkedForDeletion())
                    {
                        foundLiving = true;
                        break;
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::getMemObjectDetails
// Description: Gets a mem object's mutable details by its handle. The returned
//              object is generic (not specifically a buffer or a texture).
//              The function also returns the object index within its container
// Author:      Sigal Algranaty
// Date:        27/4/2010
// ---------------------------------------------------------------------------
apCLMemObject* csImagesAndBuffersMonitor::getMemObjectDetails(oaCLMemHandle memObjHandle, int& memoryObjectIndex) const
{
    apCLMemObject* retVal = NULL;

    bool foundLiving = false;

    memoryObjectIndex = -1;
    // Try to find this object in the buffers vector:
    int numberOfBuffers = (int)_bufferMonitors.size();

    for (int i = 0; i < numberOfBuffers; i++)
    {
        // Get the current buffer:
        apCLBuffer* pCurrentBuffer = _bufferMonitors[i];
        GT_IF_WITH_ASSERT(pCurrentBuffer != NULL)
        {
            // If this is the requested mem object:
            if (memObjHandle == pCurrentBuffer->memObjectHandle())
            {
                retVal = pCurrentBuffer;
                memoryObjectIndex = i;

                if (!pCurrentBuffer->wasMarkedForDeletion())
                {
                    foundLiving = true;
                    break;
                }
            }
        }
    }

    if (!foundLiving)
    {
        // Try to find this object in the sub buffers vector:
        int numberOfSubBuffers = (int)_subBufferMonitors.size();

        for (int i = 0; i < numberOfSubBuffers; i++)
        {
            // Get the current buffer:
            apCLSubBuffer* pCurrentSubBuffer = _subBufferMonitors[i];
            GT_IF_WITH_ASSERT(pCurrentSubBuffer != NULL)
            {
                // If this is the requested mem object:
                if (memObjHandle == pCurrentSubBuffer->memObjectHandle())
                {
                    retVal = pCurrentSubBuffer;
                    memoryObjectIndex = i;

                    if (!pCurrentSubBuffer->wasMarkedForDeletion())
                    {
                        foundLiving = true;
                        break;
                    }
                }
            }
        }
    }

    // If the object is not a buffer:
    if (!foundLiving)
    {
        // Try to find this object in the textures vector:
        int numberOfTextures = (int)_imagesMonitors.size();

        for (int i = 0; i < numberOfTextures; i++)
        {
            // Get the current texture:
            apCLImage* pCurrentTexture = _imagesMonitors[i];
            GT_IF_WITH_ASSERT(pCurrentTexture != NULL)
            {
                // If this is the requested mem object:
                if (memObjHandle == pCurrentTexture->memObjectHandle())
                {
                    retVal = pCurrentTexture;
                    memoryObjectIndex = i;

                    if (!pCurrentTexture->wasMarkedForDeletion())
                    {
                        foundLiving = true;
                        break;
                    }
                }
            }
        }
    }

    // If the object is not a buffer nor an image:
    if (!foundLiving)
    {
        // Try to find this object in the pipes vector:
        int numberOfPipes = (int)m_pipeMonitors.size();

        for (int i = 0; i < numberOfPipes; i++)
        {
            // Get the current pipe:
            apCLPipe* pCurrentPipe = m_pipeMonitors[i];
            GT_IF_WITH_ASSERT(pCurrentPipe != NULL)
            {
                // If this is the requested mem object:
                if (memObjHandle == pCurrentPipe->memObjectHandle())
                {
                    retVal = pCurrentPipe;
                    memoryObjectIndex = i;

                    if (!pCurrentPipe->wasMarkedForDeletion())
                    {
                        foundLiving = true;
                        break;
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::generateRawDataFilePath
// Description: Generates a buffer file path.
// Arguments: int bufferId
//            osFilePath& bufferFilePath
//            bool isBuffer - buffer / texture
// Return Val: void
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
void csImagesAndBuffersMonitor::generateRawDataFilePath(int objectId, osFilePath& bufferFilePath, bool isBuffer) const
{
    // Build the log file name:
    gtString logFileName;

    if (isBuffer)
    {
        logFileName.appendFormattedString(CS_STR_bufferFilePath, _spyContextId, objectId);
    }
    else
    {
        logFileName.appendFormattedString(CS_STR_textureFilePath, _spyContextId, objectId);
    }

    // Set the log file path:
    bufferFilePath = suCurrentSessionLogFilesDirectory();
    bufferFilePath.setFileName(logFileName);

    // Set the log file extension:
    bufferFilePath.setFileExtension(SU_STR_rawFileExtension);
}


// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::initializeCommandQueue
// Description: Initializes the command queue for buffers reading
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool csImagesAndBuffersMonitor::initializeCommandQueue()
{
    bool retVal = false;

    // If we haven't created the queue yet:
    if (_commandQueue == OA_CL_NULL_HANDLE)
    {
        // Get the OpenCL monitor:
        csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();

        // Get the context monitor:
        const csContextMonitor* pContextMonitor = theOpenCLMonitor.clContextMonitor(_spyContextId);
        GT_IF_WITH_ASSERT(pContextMonitor != NULL)
        {
            // Get the buffer context handle:
            oaCLContextHandle contextHandle = pContextMonitor->contextHandle();

            // Get this context's first device's details:
            oaCLDeviceID thisContextDeviceId = OA_CL_NULL_HANDLE;
            const gtVector<int>& contextDevices = pContextMonitor->contextInformation().deviceIDs();
            int numberOfContextDevices = (int)contextDevices.size();
            GT_IF_WITH_ASSERT(numberOfContextDevices > 0)
            {
                // Get a device handle:
                csDevicesMonitor& devicesMonitor = theOpenCLMonitor.devicesMonitor();

                // Get the first device:
                const apCLDevice* pFirstDevice = devicesMonitor.getDeviceObjectDetailsByIndex(contextDevices[0]);
                GT_IF_WITH_ASSERT(pFirstDevice != NULL)
                {
                    thisContextDeviceId = pFirstDevice->deviceHandle();
                }
            }

            // Create the OpenCL command queue (clCreateCommandQueue internally calls clSetCommandQueueProperty on Mac):
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clCreateCommandQueue);
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clSetCommandQueueProperty);
            cl_int cRetVal = 0;
            cl_command_queue commandQueue = cs_stat_realFunctionPointers.clCreateCommandQueue((cl_context)contextHandle, (cl_device_id)thisContextDeviceId, 0, &cRetVal);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clCreateCommandQueue);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clSetCommandQueueProperty);

            GT_IF_WITH_ASSERT((cRetVal == CL_SUCCESS) && (commandQueue != NULL))
            {
                _commandQueue = (oaCLCommandQueueHandle)commandQueue;
                retVal = true;
            }
        }
    }
    else // _commandQueue != OA_CL_NULL_HANDLE
    {
        // The queue already exists:
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::destroyCommandQueue
// Description: If we have a command queue, release it:
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        27/1/2010
// ---------------------------------------------------------------------------
bool csImagesAndBuffersMonitor::destroyCommandQueue()
{
    bool retVal = true;

    // If we created a command queue:
    if (_commandQueue != OA_CL_NULL_HANDLE)
    {
        // Delete it:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clReleaseCommandQueue);
        cl_int retCode = cs_stat_realFunctionPointers.clReleaseCommandQueue((cl_command_queue)_commandQueue);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clReleaseCommandQueue);

        retVal = (retCode == CL_SUCCESS);

        // If the deletion worked, mark it as NULL:
        GT_IF_WITH_ASSERT(retVal)
        {
            _commandQueue = OA_CL_NULL_HANDLE;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::onImageCreation
// Description: Handles texture object creation
// Arguments: cl_mem_flags flags
//            apTextureType textureType
//            gtSize_t width
//            gtSize_t height
//            gtSize_t depth
// Return Val: void
// Author:      Sigal Algranaty
// Date:        1/12/2009
// ---------------------------------------------------------------------------
void csImagesAndBuffersMonitor::onImageCreation(cl_mem imageHandle, const cl_image_format* pCLImageFormat, cl_mem_flags flags, apTextureType textureType, gtSize_t width, gtSize_t height, gtSize_t depth)
{
    // Create the texture object:
    int imageName = _nextFreeImageName;
    apCLImage* pImageObject = new apCLImage(imageName, textureType, width, height, depth);

    // Increase texture next free index:
    _nextFreeImageName++;

    // Register this object in the allocated objects monitor:
    su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pImageObject);

    // Set the texture data format and type:
    pImageObject->setFormatAndType(pCLImageFormat->image_channel_order, pCLImageFormat->image_channel_data_type);

    // Set texture properties:
    pImageObject->setMemoryFlags(flags);
    pImageObject->setMemObjectHandle((oaCLMemHandle)imageHandle);

    // Add the texture object:
    _imagesMonitors.push_back(pImageObject);

    // Add the command queue handle:
    csOpenCLHandleMonitor& handlesMonitor = cs_stat_openCLMonitorInstance.openCLHandleMonitor();
    int imageIndex = (int)_imagesMonitors.size() - 1;
    handlesMonitor.registerOpenCLHandle((oaCLHandle)imageHandle, _spyContextId, imageIndex, OS_TOBJ_ID_CL_IMAGE, -1, imageName);

    // Set the mapping for the image name:
    _bufferNameToIndexMap.insert(pair<int, int>(imageName, imageIndex));

    // Register a callback to handle this memory object deletion:
    if (_isMemObjectDestructorCallbackSupported && (cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback != NULL))
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
        cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback(imageHandle, &csImagesAndBuffersMonitor::onMemoryObjectDeletion, this);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
    }
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::onImageCreation
// Description: Handle image creation
// Arguments:   cl_mem textureHandle
//              const cl_image_format* pCLImageFormat
//              cl_mem_flags flags
//              const cl_image_desc* pImageDesc
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        16/1/2012
// ---------------------------------------------------------------------------
void csImagesAndBuffersMonitor::onImageCreation(cl_mem imageHandle, const cl_image_format* pCLImageFormat, cl_mem_flags flags, const cl_image_desc* pImageDesc)
{
    // Sanity check
    GT_IF_WITH_ASSERT(pImageDesc != NULL)
    {
        // Get the image type from the cl image type enumeration:
        apTextureType textureType = AP_UNKNOWN_TEXTURE_TYPE;
        bool rc = apCLImage::textureTypeFromMemObjectType(pImageDesc->image_type, textureType);
        GT_IF_WITH_ASSERT(rc)
        {
            // Create the texture object:
            int imageName = _nextFreeImageName;

            // Get the image dimensions:
            int width = (int)pImageDesc->image_width;
            int height = max((int)pImageDesc->image_height, 1);
            int depth = max((int)pImageDesc->image_depth, 1);

            if (textureType == AP_2D_ARRAY_TEXTURE)
            {
                depth = (int)pImageDesc->image_array_size;
            }
            else if (textureType == AP_1D_ARRAY_TEXTURE)
            {
                height = (int)pImageDesc->image_array_size;
                depth = 1;
            }

            // Create a new image object:
            apCLImage* pImageObject = new apCLImage(imageName, textureType, width, height, depth);


            if (textureType == AP_BUFFER_TEXTURE)
            {
                // Set the image buffer:
#pragma message ("Handle buffer image")
                pImageObject->setCLBuffer((oaCLMemHandle)pImageDesc->buffer);
            }


            // Increase texture next free index:
            _nextFreeImageName++;

            // Register this object in the allocated objects monitor:
            su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pImageObject);

            // Set the texture data format and type:
            pImageObject->setFormatAndType(pCLImageFormat->image_channel_order, pCLImageFormat->image_channel_data_type);

            // Set texture properties:
            pImageObject->setMemoryFlags(flags);
            pImageObject->setMemObjectHandle((oaCLMemHandle)imageHandle);

            // Add the texture object:
            _imagesMonitors.push_back(pImageObject);

            // Add the command queue handle:
            csOpenCLHandleMonitor& handlesMonitor = cs_stat_openCLMonitorInstance.openCLHandleMonitor();
            int imageIndex = (int)_imagesMonitors.size() - 1;
            handlesMonitor.registerOpenCLHandle((oaCLHandle)imageHandle, _spyContextId, imageIndex, OS_TOBJ_ID_CL_IMAGE, -1, imageName);

            // Set the mapping for the image name:
            _bufferNameToIndexMap.insert(pair<int, int>(imageName, imageIndex));

            // Register a callback to handle this memory object deletion:
            if (_isMemObjectDestructorCallbackSupported && (cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback != NULL))
            {
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
                cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback(imageHandle, &csImagesAndBuffersMonitor::onMemoryObjectDeletion, this);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::onImageCreationFromGLTexture
// Description: Called when an OpenCL image is created the OpenGL texture
//              named texture (level #miplevel) of type/face target, with
//              clCreateFromGLTexture
// Author:      Uri Shomroni
// Date:        22/3/2012
// ---------------------------------------------------------------------------
void csImagesAndBuffersMonitor::onImageCreationFromGLTexture(cl_mem imageHandle, cl_mem_flags flags, GLenum target, GLint miplevel, GLuint texture)
{
    // Get the image dimensions and format from OpenCL:
    gtSize_t width = 0;
    gtSize_t height = 0;
    gtSize_t depth = 0;
    cl_image_format imageFormat;
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetImageInfo);
    cl_int rcWid = cs_stat_realFunctionPointers.clGetImageInfo(imageHandle, CL_IMAGE_WIDTH, sizeof(gtSize_t), (void*)(&width), NULL);
    cl_int rcHei = cs_stat_realFunctionPointers.clGetImageInfo(imageHandle, CL_IMAGE_HEIGHT, sizeof(gtSize_t), (void*)(&height), NULL);
    cl_int rcDep = cs_stat_realFunctionPointers.clGetImageInfo(imageHandle, CL_IMAGE_DEPTH, sizeof(gtSize_t), (void*)(&depth), NULL);
    cl_int rcFmt = cs_stat_realFunctionPointers.clGetImageInfo(imageHandle, CL_IMAGE_FORMAT, sizeof(cl_image_format), (void*)(&imageFormat), NULL);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetImageInfo);

    // Get the image type:
    apTextureType textureType = (depth > 0) ? AP_3D_TEXTURE : AP_2D_TEXTURE;

    GT_ASSERT((CL_SUCCESS == rcWid) && (CL_SUCCESS == rcHei) && (CL_SUCCESS == rcDep));

    // Create the texture object:
    int imageName = _nextFreeImageName;
    apCLImage* pImageObject = new apCLImage(imageName, textureType, width, height, (textureType == AP_3D_TEXTURE) ? depth : 1);

    // Increase texture next free index:
    _nextFreeImageName++;

    // Register this object in the allocated objects monitor:
    su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pImageObject);

    // If we got the format:
    GT_IF_WITH_ASSERT(rcFmt == CL_SUCCESS)
    {
        // Set the texture data format and type:
        pImageObject->setFormatAndType(imageFormat.image_channel_order, imageFormat.image_channel_data_type);
    }

    // Set texture properties:
    pImageObject->setMemoryFlags(flags);
    pImageObject->setMemObjectHandle((oaCLMemHandle)imageHandle);

    // Add the texture object:
    _imagesMonitors.push_back(pImageObject);

    // Add the command queue handle:
    csOpenCLHandleMonitor& handlesMonitor = cs_stat_openCLMonitorInstance.openCLHandleMonitor();
    int textureIndex = (int)_imagesMonitors.size() - 1;
    handlesMonitor.registerOpenCLHandle((oaCLHandle)imageHandle, _spyContextId, textureIndex, OS_TOBJ_ID_CL_IMAGE, -1, imageName);

    // Register a callback to handle this memory object deletion:
    if (_isMemObjectDestructorCallbackSupported && (cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback != NULL))
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
        cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback(imageHandle, &csImagesAndBuffersMonitor::onMemoryObjectDeletion, this);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
    }

    // Handle OpenGL interoperability:
    bool rcGL = handleImageShareWithGLTexture(pImageObject, textureIndex, target, miplevel, texture);
    GT_ASSERT(rcGL);
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::onImageCreationFromGL2DTexture
// Description: Called when an OpenCL image is created the OpenGL 2D texture
//              named texture (level #miplevel) of type/face target, with
//              clCreateFromGLTexture2D
// Author:      Uri Shomroni
// Date:        4/3/2010
// ---------------------------------------------------------------------------
void csImagesAndBuffersMonitor::onImageCreationFromGL2DTexture(cl_mem imageHandle, cl_mem_flags flags, apTextureType textureType, GLenum target, GLint miplevel, GLuint texture)
{
    // Get the image dimensions and format from OpenCL:
    gtSize_t width = 0;
    gtSize_t height = 0;
    cl_image_format imageFormat;
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetImageInfo);
    cl_int rcWid = cs_stat_realFunctionPointers.clGetImageInfo(imageHandle, CL_IMAGE_WIDTH, sizeof(gtSize_t), (void*)(&width), NULL);
    cl_int rcHei = cs_stat_realFunctionPointers.clGetImageInfo(imageHandle, CL_IMAGE_HEIGHT, sizeof(gtSize_t), (void*)(&height), NULL);
    cl_int rcFmt = cs_stat_realFunctionPointers.clGetImageInfo(imageHandle, CL_IMAGE_FORMAT, sizeof(cl_image_format), (void*)(&imageFormat), NULL);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetImageInfo);

    GT_ASSERT((rcWid == CL_SUCCESS) && (rcHei == CL_SUCCESS));

    // Create the texture object:
    int imageName = _nextFreeImageName;
    apCLImage* pImageObject = new apCLImage(imageName, textureType, width, height, 1);

    // Increase texture next free index:
    _nextFreeImageName++;

    // Register this object in the allocated objects monitor:
    su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pImageObject);

    // If we got the format:
    GT_IF_WITH_ASSERT(rcFmt == CL_SUCCESS)
    {
        // Set the texture data format and type:
        pImageObject->setFormatAndType(imageFormat.image_channel_order, imageFormat.image_channel_data_type);
    }

    // Set texture properties:
    pImageObject->setMemoryFlags(flags);
    pImageObject->setMemObjectHandle((oaCLMemHandle)imageHandle);

    // Add the texture object:
    _imagesMonitors.push_back(pImageObject);

    // Add the command queue handle:
    csOpenCLHandleMonitor& handlesMonitor = cs_stat_openCLMonitorInstance.openCLHandleMonitor();
    int textureIndex = (int)_imagesMonitors.size() - 1;
    handlesMonitor.registerOpenCLHandle((oaCLHandle)imageHandle, _spyContextId, textureIndex, OS_TOBJ_ID_CL_IMAGE, -1, imageName);

    // Register a callback to handle this memory object deletion:
    if (_isMemObjectDestructorCallbackSupported && (cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback != NULL))
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
        cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback(imageHandle, &csImagesAndBuffersMonitor::onMemoryObjectDeletion, this);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
    }

    // Handle OpenGL interoperability:
    bool rcGL = handleImageShareWithGLTexture(pImageObject, textureIndex, target, miplevel, texture);
    GT_ASSERT(rcGL);
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::onTextureCreationFromGL3DTexture
// Description: Called when an OpenCL image is created the OpenGL 3D texture
//              named texture (level #miplevel) of type/face target, with
//              clCreateFromGLTexture3D
// Author:      Uri Shomroni
// Date:        4/3/2010
// ---------------------------------------------------------------------------
void csImagesAndBuffersMonitor::onTextureCreationFromGL3DTexture(cl_mem textureHandle, cl_mem_flags flags, apTextureType textureType, GLenum target, GLint miplevel, GLuint texture)
{
    // Get the image dimensions and format from OpenCL:
    gtSize_t width = 0;
    gtSize_t height = 0;
    gtSize_t depth = 0;
    cl_image_format imageFormat;
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetImageInfo);
    cl_int rcWid = cs_stat_realFunctionPointers.clGetImageInfo(textureHandle, CL_IMAGE_WIDTH, sizeof(gtSize_t), (void*)(&width), NULL);
    cl_int rcHei = cs_stat_realFunctionPointers.clGetImageInfo(textureHandle, CL_IMAGE_HEIGHT, sizeof(gtSize_t), (void*)(&height), NULL);
    cl_int rcDep = cs_stat_realFunctionPointers.clGetImageInfo(textureHandle, CL_IMAGE_DEPTH, sizeof(gtSize_t), (void*)(&depth), NULL);
    cl_int rcFmt = cs_stat_realFunctionPointers.clGetImageInfo(textureHandle, CL_IMAGE_FORMAT, sizeof(cl_image_format), (void*)(&imageFormat), NULL);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetImageInfo);

    GT_ASSERT((rcWid == CL_SUCCESS) && (rcHei == CL_SUCCESS) && (rcDep == CL_SUCCESS));

    // Create the texture object:
    int textureName = _nextFreeImageName;
    apCLImage* pImageObject = new apCLImage(textureName, textureType, width, height, depth);

    // Increase the next texture free index:
    _nextFreeImageName ++;

    // Register this object in the allocated objects monitor:
    su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pImageObject);

    // If we got the format:
    GT_IF_WITH_ASSERT(rcFmt == CL_SUCCESS)
    {
        // Set the texture data format and type:
        pImageObject->setFormatAndType(imageFormat.image_channel_order, imageFormat.image_channel_data_type);
    }

    // Set texture properties:
    pImageObject->setMemoryFlags(flags);
    pImageObject->setMemObjectHandle((oaCLMemHandle)textureHandle);

    // Add the texture object:
    _imagesMonitors.push_back(pImageObject);

    // Add the command queue handle:
    csOpenCLHandleMonitor& handlesMonitor = cs_stat_openCLMonitorInstance.openCLHandleMonitor();
    int imageIndex = (int)_imagesMonitors.size() - 1;
    handlesMonitor.registerOpenCLHandle((oaCLHandle)textureHandle, _spyContextId, imageIndex, OS_TOBJ_ID_CL_IMAGE, -1, textureName);

    // Register a callback to handle this memory object deletion:
    if (_isMemObjectDestructorCallbackSupported && (cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback != NULL))
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
        cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback(textureHandle, &csImagesAndBuffersMonitor::onMemoryObjectDeletion, this);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
    }

    // Handle OpenGL interoperability:
    bool rcGL = handleImageShareWithGLTexture(pImageObject, imageIndex, target, miplevel, texture);
    GT_ASSERT(rcGL);

}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::onTextureCreationFromGLRenderbuffer
// Description: Called when an OpenCL image is created the OpenGL 3D texture
//              named texture (level #miplevel) of type/face target, with
//              clCreateFromGLTexture3D
// Author:      Uri Shomroni
// Date:        4/3/2010
// ---------------------------------------------------------------------------
void csImagesAndBuffersMonitor::onTextureCreationFromGLRenderbuffer(cl_mem imageHandle, cl_mem_flags flags, apTextureType imageType, GLuint renderbuffer)
{
    // Get the image dimensions and format from OpenCL:
    gtSize_t width = 0;
    gtSize_t height = 0;
    cl_image_format imageFormat;
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetImageInfo);
    cl_int rcWid = cs_stat_realFunctionPointers.clGetImageInfo(imageHandle, CL_IMAGE_WIDTH, sizeof(gtSize_t), (void*)(&width), NULL);
    cl_int rcHei = cs_stat_realFunctionPointers.clGetImageInfo(imageHandle, CL_IMAGE_HEIGHT, sizeof(gtSize_t), (void*)(&height), NULL);
    cl_int rcFmt = cs_stat_realFunctionPointers.clGetImageInfo(imageHandle, CL_IMAGE_FORMAT, sizeof(cl_image_format), (void*)(&imageFormat), NULL);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetImageInfo);

    GT_ASSERT((rcWid == CL_SUCCESS) && (rcHei == CL_SUCCESS));

    // Create the image object:
    int imageName = _nextFreeImageName;
    apCLImage* pImageObject = new apCLImage(imageName, imageType, width, height, 1);

    // Increase the next free image index:
    _nextFreeImageName ++;

    // Register this object in the allocated objects monitor:
    su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pImageObject);

    // If we got the format:
    GT_IF_WITH_ASSERT(rcFmt == CL_SUCCESS)
    {
        // Set the image data format and type:
        pImageObject->setFormatAndType(imageFormat.image_channel_order, imageFormat.image_channel_data_type);
    }

    // Set image properties:
    pImageObject->setMemoryFlags(flags);
    pImageObject->setMemObjectHandle((oaCLMemHandle)imageHandle);

    // Add the image object:
    _imagesMonitors.push_back(pImageObject);

    // Add the command queue handle:
    csOpenCLHandleMonitor& handlesMonitor = cs_stat_openCLMonitorInstance.openCLHandleMonitor();
    int imageIndex = (int)_imagesMonitors.size() - 1;
    handlesMonitor.registerOpenCLHandle((oaCLHandle)imageHandle, _spyContextId, imageIndex, OS_TOBJ_ID_CL_IMAGE, -1, imageName);

    // Register a callback to handle this memory object deletion:
    if (_isMemObjectDestructorCallbackSupported && (cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback != NULL))
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
        cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback(imageHandle, &csImagesAndBuffersMonitor::onMemoryObjectDeletion, this);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
    }

    // Handle OpenGL interoperability:
    bool rcGL = handleImageShareWithGLRenderBuffer(pImageObject, imageIndex, renderbuffer);
    GT_ASSERT(rcGL);

}


// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::onImageCreationFromDirectX
// Description: Called when an OpenCL image is created a DirectX texture or surface
//              via clCreateFromD3D*Texture*DKHR or clCreateFromDX9MediaSurfaceKHR
// Author:      Uri Shomroni
// Date:        2/10/2014
// ---------------------------------------------------------------------------
void csImagesAndBuffersMonitor::onImageCreationFromDirectX(cl_mem textureHandle, cl_mem_flags flags, apTextureType textureType)
{
    // Get the image dimensions and format from OpenCL:
    gtSize_t width = 0;
    gtSize_t height = 0;
    gtSize_t depth = 0;
    cl_image_format imageFormat;
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetImageInfo);
    cl_int rcWid = cs_stat_realFunctionPointers.clGetImageInfo(textureHandle, CL_IMAGE_WIDTH, sizeof(gtSize_t), (void*)(&width), NULL);
    cl_int rcHei = cs_stat_realFunctionPointers.clGetImageInfo(textureHandle, CL_IMAGE_HEIGHT, sizeof(gtSize_t), (void*)(&height), NULL);
    cl_int rcDep = CL_SUCCESS;

    if (AP_3D_TEXTURE == textureType)
    {
        rcDep = cs_stat_realFunctionPointers.clGetImageInfo(textureHandle, CL_IMAGE_DEPTH, sizeof(gtSize_t), (void*)(&depth), NULL);
    }

    cl_int rcFmt = cs_stat_realFunctionPointers.clGetImageInfo(textureHandle, CL_IMAGE_FORMAT, sizeof(cl_image_format), (void*)(&imageFormat), NULL);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetImageInfo);

    GT_ASSERT((rcWid == CL_SUCCESS) && (rcHei == CL_SUCCESS) && (rcDep == CL_SUCCESS));

    // Create the texture object:
    int textureName = _nextFreeImageName;
    apCLImage* pImageObject = new apCLImage(textureName, textureType, width, height, (AP_3D_TEXTURE == textureType) ? depth : 1);

    // Increase the next texture free index:
    _nextFreeImageName++;

    // Register this object in the allocated objects monitor:
    su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(*pImageObject);

    // If we got the format:
    GT_IF_WITH_ASSERT(rcFmt == CL_SUCCESS)
    {
        // Set the texture data format and type:
        pImageObject->setFormatAndType(imageFormat.image_channel_order, imageFormat.image_channel_data_type);
    }

    // Set texture properties:
    pImageObject->setMemoryFlags(flags);
    pImageObject->setMemObjectHandle((oaCLMemHandle)textureHandle);

    // Add the texture object:
    _imagesMonitors.push_back(pImageObject);

    // Add the command queue handle:
    csOpenCLHandleMonitor& handlesMonitor = cs_stat_openCLMonitorInstance.openCLHandleMonitor();
    int imageIndex = (int)_imagesMonitors.size() - 1;
    handlesMonitor.registerOpenCLHandle((oaCLHandle)textureHandle, _spyContextId, imageIndex, OS_TOBJ_ID_CL_IMAGE, -1, textureName);

    // Register a callback to handle this memory object deletion:
    if (_isMemObjectDestructorCallbackSupported && (cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback != NULL))
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
        cs_stat_realFunctionPointers.clSetMemObjectDestructorCallback(textureHandle, &csImagesAndBuffersMonitor::onMemoryObjectDeletion, this);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clSetMemObjectDestructorCallback);
    }
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::onMemObjectMarkedForDeletion
// Description: Called when the mem object is marked for deletion
//              (calling clReleaseMemObject with the reference count at 1)
// Author:      Uri Shomroni
// Date:        31/3/2010
// ---------------------------------------------------------------------------
void csImagesAndBuffersMonitor::onMemObjectMarkedForDeletion(cl_mem memobj)
{
    // Prevent deletion on multiple threads (e.g. when the object is removed then
    // the destruction callback might be called at the same time:
    _memObjectDeletionCS.enter();

    // Get the mem object monitor:
    int deletedObjectIndex = -1;
    apCLMemObject* pMemObj = getMemObjectDetails((oaCLMemHandle)memobj, deletedObjectIndex);

    // NOTICE: We do not fail the function if the memory object was not found, since we have double memory handling
    // mechanism (in OpenCL1.1 we use memory object destruction callback function) so the object might have been destructed already:
    if (pMemObj != NULL)
    {
        // Mark it as deleted:
        pMemObj->onMemObjectMarkedForDeletion();

        osTransferableObjectType memObjType = pMemObj->type();

        // If the object is a buffer:
        if (memObjType == OS_TOBJ_ID_CL_BUFFER)
        {
            GT_IF_WITH_ASSERT((deletedObjectIndex >= 0) && (deletedObjectIndex < (int)_bufferMonitors.size()))
            {
                // Remove the item from the vector:
                _bufferMonitors.removeItem(deletedObjectIndex);
            }
        }

        // If the object is a sub-buffer:
        if (memObjType == OS_TOBJ_ID_CL_SUB_BUFFER)
        {
            GT_IF_WITH_ASSERT((deletedObjectIndex >= 0) && (deletedObjectIndex < (int)_subBufferMonitors.size()))
            {
                // Remove the item from the vector:
                _subBufferMonitors.removeItem(deletedObjectIndex);
            }
        }
        // If the object is a image:
        else if (memObjType == OS_TOBJ_ID_CL_IMAGE)
        {
            GT_IF_WITH_ASSERT((deletedObjectIndex >= 0) && (deletedObjectIndex < (int)_imagesMonitors.size()))
            {
                // Remove the item from the vector:
                _imagesMonitors.removeItem(deletedObjectIndex);
            }
        }
        // If the object is a pipe:
        else if (memObjType == OS_TOBJ_ID_CL_PIPE)
        {
            GT_IF_WITH_ASSERT((deletedObjectIndex >= 0) && (deletedObjectIndex < (int)m_pipeMonitors.size()))
            {
                // Remove the item from the vector:
                m_pipeMonitors.removeItem(deletedObjectIndex);
            }
        }
    }

    // Leave the critical section:
    _memObjectDeletionCS.leave();
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::checkForReleasedObjects
// Description: Checks if any of the objects monitored by this class have been released
// Author:      Uri Shomroni
// Date:        26/8/2013
// ---------------------------------------------------------------------------
void csImagesAndBuffersMonitor::checkForReleasedObjects()
{
    // Collect the object handles:
    gtVector<oaCLMemHandle> objectHandles;
    int numberOfSubBuffers = (int)_subBufferMonitors.size();

    for (int i = 0; i < numberOfSubBuffers; i++)
    {
        const apCLSubBuffer* pSubBuffer = _subBufferMonitors[i];
        GT_IF_WITH_ASSERT(NULL != pSubBuffer)
        {
            objectHandles.push_back(pSubBuffer->memObjectHandle());
        }
    }

    int numberOfBuffers = (int)_bufferMonitors.size();

    for (int i = 0; i < numberOfBuffers; i++)
    {
        const apCLBuffer* pBuffer = _bufferMonitors[i];
        GT_IF_WITH_ASSERT(NULL != pBuffer)
        {
            objectHandles.push_back(pBuffer->memObjectHandle());
        }
    }

    int numberOfImages = (int)_imagesMonitors.size();

    for (int i = 0; i < numberOfImages; i++)
    {
        const apCLImage* pImage = _imagesMonitors[i];
        GT_IF_WITH_ASSERT(NULL != pImage)
        {
            objectHandles.push_back(pImage->memObjectHandle());
        }
    }

    int numberOfPipes = (int)m_pipeMonitors.size();

    for (int i = 0; i < numberOfPipes; i++)
    {
        const apCLPipe* pPipe = m_pipeMonitors[i];
        GT_IF_WITH_ASSERT(NULL != pPipe)
        {
            objectHandles.push_back(pPipe->memObjectHandle());
        }
    }

    // Check each one. This is done separately, since finding an object
    // that was marked for deletion will release it:
    csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
    int objectsFound = (int)objectHandles.size();

    for (int i = 0; i < objectsFound; i++)
    {
        theOpenCLMonitor.checkIfMemObjectWasDeleted((cl_mem)objectHandles[i], false);
    }
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::amountOfImages
// Description: Returns the amount of logged buffers.
// Author:      Sigal Algranaty
// Date:        1/12/2009
// ---------------------------------------------------------------------------
int csImagesAndBuffersMonitor::amountOfImages() const
{
    int retVal = (int)_imagesMonitors.size();
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::imageDetails
// Description: Inputs a image index and return it's monitor.
// Author:      Sigal Algranaty
// Date:        1/12/2009
// ---------------------------------------------------------------------------
const apCLImage* csImagesAndBuffersMonitor::imageDetails(int imageId) const
{
    const apCLImage* retVal = NULL;

    // Index range check:
    int imagesAmount = (int)_imagesMonitors.size();
    GT_IF_WITH_ASSERT((0 <= imagesAmount) && (imageId < imagesAmount))
    {
        retVal = _imagesMonitors[imageId];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::imageDetails
// Description: Inputs a image index and return it's monitor.
// Author:      Sigal Algranaty
// Date:        1/12/2009
// ---------------------------------------------------------------------------
apCLImage* csImagesAndBuffersMonitor::imageDetails(int imageId)
{
    apCLImage* retVal = NULL;

    // Index range check:
    int imagesAmount = (int)_imagesMonitors.size();
    GT_IF_WITH_ASSERT((0 <= imagesAmount) && (imageId < imagesAmount))
    {
        retVal = _imagesMonitors[imageId];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::imageObjectMonitorIndex
// Description: Inputs a image name and outputs its monitor's location (index)
//              in the _images array, or -1 if it does not exist.
// Author:      Sigal Algranaty
// Date:        28/4/2010
// ---------------------------------------------------------------------------
int csImagesAndBuffersMonitor::imageObjectMonitorIndex(int imageName) const
{
    int retVal = -1;

    gtMap<int, int>::const_iterator endIter = _imageNameToIndexMap.end();
    gtMap<int, int>::const_iterator iter = _imageNameToIndexMap.find(imageName);

    if (iter != endIter)
    {
        retVal = (*iter).second;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::bufferObjectMonitorIndex
// Description: Inputs a buffer name and outputs its monitor's location (index)
//              in the buffers array, or -1 if it does not exist.
// Author:      Sigal Algranaty
// Date:        28/4/2010
// ---------------------------------------------------------------------------
int csImagesAndBuffersMonitor::bufferObjectMonitorIndex(int bufferName) const
{
    int retVal = -1;

    gtMap<int, int>::const_iterator endIter = _bufferNameToIndexMap.end();
    gtMap<int, int>::const_iterator iter = _bufferNameToIndexMap.find(bufferName);

    if (iter != endIter)
    {
        retVal = (*iter).second;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::amountOfPipes
// Description: Returns the amount of logged pipes.
// Author:      Uri Shomroni
// Date:        1/10/2014
// ---------------------------------------------------------------------------
int csImagesAndBuffersMonitor::amountOfPipes() const
{
    int retVal = (int)m_pipeMonitors.size();
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::pipeDetails
// Description: Inputs a pipe index and return itss monitor.
// Author:      Uri Shomroni
// Date:        1/10/2014
// ---------------------------------------------------------------------------
const apCLPipe* csImagesAndBuffersMonitor::pipeDetails(int pipeId) const
{
    const apCLPipe* retVal = NULL;

    // Index range check:
    int pipesAmount = (int)m_pipeMonitors.size();
    GT_IF_WITH_ASSERT((0 <= pipesAmount) && (pipeId < pipesAmount))
    {
        retVal = m_pipeMonitors[pipeId];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::pipeDetails
// Description: Inputs a buffer index and return it's monitor.
// Author:      Uri Shomroni
// Date:        1/10/2014
// ---------------------------------------------------------------------------
apCLPipe* csImagesAndBuffersMonitor::pipeDetails(int pipeId)
{
    apCLPipe* retVal = NULL;

    // Index range check:
    int pipesAmount = (int)m_pipeMonitors.size();
    GT_IF_WITH_ASSERT((0 <= pipeId) && (pipeId < pipesAmount))
    {
        retVal = m_pipeMonitors[pipeId];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::pipeObjectMonitorIndex
// Description: Inputs a pipe name and outputs its monitor's location (index)
//              in the pipes array, or -1 if it does not exist.
// Author:      Uri Shomroni
// Date:        1/10/2014
// ---------------------------------------------------------------------------
int csImagesAndBuffersMonitor::pipeObjectMonitorIndex(int pipeName) const
{
    int retVal = -1;

    gtMap<int, int>::const_iterator endIter = m_pipeNameToIndexMap.end();
    gtMap<int, int>::const_iterator iter = m_pipeNameToIndexMap.find(pipeName);

    if (iter != endIter)
    {
        retVal = (*iter).second;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::calculateBuffersMemorySize
// Description: Calculate the buffers memory size
// Arguments:   gtUInt64& buffersMemorySize
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/7/2010
// ---------------------------------------------------------------------------
bool csImagesAndBuffersMonitor::calculateBuffersMemorySize(gtUInt64& buffersMemorySize) const
{
    bool retVal = true;

    // Iterate the buffers monitor:
    for (int i = 0; i < (int)_bufferMonitors.size(); i++)
    {
        // Get the current buffer:
        apCLBuffer* pBufferObject = _bufferMonitors[i];

        if (pBufferObject != NULL)
        {
            gtUInt64 currentBufferMemorySize;
            // Get the calculated buffer object size (all miplevels sizes):
            currentBufferMemorySize = pBufferObject->bufferSize();

            // Set the object size in KB (ceil the number for 0.1KB to turn to be 1KB):
            if (!pBufferObject->wasMarkedForDeletion() && (currentBufferMemorySize > 0))
            {
                currentBufferMemorySize = (((currentBufferMemorySize + 1023) / 1024) + 7) / 8;

                if (currentBufferMemorySize == 0)
                {
                    currentBufferMemorySize = 1;
                }

                // Add the current buffer to the memory size:
                buffersMemorySize += currentBufferMemorySize;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::calculateImagesMemorySize
// Description: Calculate the textures memory size
// Arguments:   gtUInt64& imagesMemorySize
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/7/2010
// ---------------------------------------------------------------------------
bool csImagesAndBuffersMonitor::calculateImagesMemorySize(gtUInt64& imagesMemorySize) const
{
    bool retVal = true;

    // Iterate the textures monitor:
    for (int i = 0; i < (int)_imagesMonitors.size(); i++)
    {
        // Get the current texture:
        apCLImage* pTextureObject = _imagesMonitors[i];

        if (pTextureObject != NULL)
        {
            gtSize_t currentImageMemorySize = 0;

            // Get the calculated texture object size (all miplevels sizes):
            bool rc = pTextureObject->getMemorySize(currentImageMemorySize);
            GT_IF_WITH_ASSERT(rc)
            {
                // Set the object size in KB (ceil the number for 0.1KB to turn to be 1KB):
                if (!pTextureObject->wasMarkedForDeletion() && (currentImageMemorySize > 0))
                {
                    currentImageMemorySize = (((currentImageMemorySize + 1023) / 1024) + 7) / 8;

                    if (currentImageMemorySize == 0)
                    {
                        currentImageMemorySize = 1;
                    }
                }

                // Add the current texture to the memory size:
                imagesMemorySize += currentImageMemorySize;
            }

            retVal = retVal && rc;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::calculatePipesMemorySize
// Description: Calculate the pipes memory size
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        1/10/2014
// ---------------------------------------------------------------------------
bool csImagesAndBuffersMonitor::calculatePipesMemorySize(gtUInt64& pipesMemorySize) const
{
    bool retVal = true;

    // Iterate the pipes monitor:
    for (int i = 0; i < (int)m_pipeMonitors.size(); i++)
    {
        // Get the current pipe:
        apCLPipe* pPipeObject = m_pipeMonitors[i];

        if (NULL != pPipeObject)
        {
            // Get the calculated pipe object size:
            gtUInt64 currentPipeMemorySize = pPipeObject->pipePacketSize() * pPipeObject->pipeMaxPackets();

            // Set the object size in KB (ceil the number for 0.1KB to turn to be 1KB):
            if (!pPipeObject->wasMarkedForDeletion() && (0 < currentPipeMemorySize))
            {
                currentPipeMemorySize = (currentPipeMemorySize + 1023) / 1024;

                // Add the current pipe to the memory size:
                pipesMemorySize += currentPipeMemorySize;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::handleImageShareWithGLTexture
// Description: Handle the share of an OpenGL texture
// Arguments:   apCLImage * pImageObject
//              GLenum target
//              GLint miplevel
//              GLuint texture
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/7/2010
// ---------------------------------------------------------------------------
bool csImagesAndBuffersMonitor::handleImageShareWithGLTexture(apCLImage* pImageObject, int clImageIndex, GLenum target, GLint miplevel, GLuint texture)
{
    bool retVal = false;

    // Get the context monitor:
    int glSpyContextID = -1;
    csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
    const csContextMonitor* pContextMonitor = theOpenCLMonitor.clContextMonitor(_spyContextId);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        // Get the context OpenGL shared ID:
        glSpyContextID = pContextMonitor->openGLSharedContextSpyId();

        if (glSpyContextID > 0)
        {
            // Set the OpenGL spy context ID:
            pImageObject->setOpenGLSpyID(glSpyContextID);
        }
        else
        {
            // TO_DO: gl-cl interp: detected errorL Trying to create an OpenGL - CL interoperability buffer,
            // in a not shared context..
        }
    }

    // OpenCL - OpenGL integration
    pImageObject->setGLTextureDetails(texture, miplevel, target);

    // Get the OpenGL spy, and search for the spy ID for this context:
    osProcedureAddress pProcAddress = NULL;
    suAPIConnector::instance().osGetSpyProcAddress(AP_OPENGL_API_CONNECTION, "gsShareGLTextureWithCLImage", pProcAddress);
    GT_IF_WITH_ASSERT(pProcAddress != NULL)
    {
        GSSHAREGLTEXTUREWITHCLIMAGEPROC pShareGLTextureWithCLImageProcAddress = (GSSHAREGLTEXTUREWITHCLIMAGEPROC)pProcAddress;
        GT_IF_WITH_ASSERT(pShareGLTextureWithCLImageProcAddress != NULL)
        {
            // Inform the OpenGL spy with the texture share:
            gtString detectedErrorStr;
            retVal = pShareGLTextureWithCLImageProcAddress(clImageIndex, pImageObject->imageName(), _spyContextId, glSpyContextID, texture, miplevel, target, detectedErrorStr);

            // TO_DO: gl-cl interp: handle detected error:
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::handleImageShareWithGLRenderBuffer
// Description: Handel sharing of an OpenCL image with an OpenGL render buffer
// Arguments:   apCLImage * pImageObject
//              GLuint renderbuffer
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/7/2010
// ---------------------------------------------------------------------------
bool csImagesAndBuffersMonitor::handleImageShareWithGLRenderBuffer(apCLImage* pImageObject, int clTextureIndex, GLuint renderbuffer)
{
    bool retVal = false;

    // Get the context monitor:
    int glSpyContextID = -1;
    csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
    const csContextMonitor* pContextMonitor = theOpenCLMonitor.clContextMonitor(_spyContextId);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        // Get the context OpenGL shared ID:
        glSpyContextID = pContextMonitor->openGLSharedContextSpyId();

        if (glSpyContextID > 0)
        {
            // Set the OpenGL spy context ID:
            pImageObject->setOpenGLSpyID(glSpyContextID);
        }
        else
        {
            // TO_DO: gl-cl interp: detected errorL Trying to create an OpenGL - CL interoperability buffer,
            // in a not shared context..
        }
    }

    // OpenCL - OpenGL integration
    pImageObject->setGLRenderBufferDetails(renderbuffer);

    // Get the OpenGL spy, and search for the spy ID for this context:
    osProcedureAddress pProcAddress = NULL;
    suAPIConnector::instance().osGetSpyProcAddress(AP_OPENGL_API_CONNECTION, "gsShareGLRenderBufferWithCLImage", pProcAddress);
    GT_IF_WITH_ASSERT(pProcAddress != NULL)
    {
        GSSHAREGLRENDERBUFFERWITHCLIMAGEPROC pShareGLRenderBufferWithCLImageProcAddress = (GSSHAREGLRENDERBUFFERWITHCLIMAGEPROC)pProcAddress;
        GT_IF_WITH_ASSERT(pShareGLRenderBufferWithCLImageProcAddress != NULL)
        {
            // Inform the OpenGL spy with the texture share:
            gtString detectedErrorStr;
            retVal = pShareGLRenderBufferWithCLImageProcAddress(clTextureIndex, pImageObject->imageName(), _spyContextId, glSpyContextID, renderbuffer, detectedErrorStr);

            // TO_DO: gl-cl interp: handle detected error:
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::handleBufferShareWithGLVBO
// Description: Handle an OpenCL buffer share with an OpenGL VBO
// Arguments:   apCLBuffer* pBufferObject
//              GLuint glVBOName
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/7/2010
// ---------------------------------------------------------------------------
bool csImagesAndBuffersMonitor::handleBufferShareWithGLVBO(apCLBuffer* pBufferObject, int clBufferIndex, GLuint glVBOName)
{
    bool retVal = false;

    // Get the context monitor:
    int glSpyContextID = -1;
    csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
    const csContextMonitor* pContextMonitor = theOpenCLMonitor.clContextMonitor(_spyContextId);
    GT_IF_WITH_ASSERT(pContextMonitor != NULL)
    {
        // Get the context OpenGL shared ID:
        glSpyContextID = pContextMonitor->openGLSharedContextSpyId();

        if (glSpyContextID > 0)
        {
            // Set the OpenGL spy context ID:
            pBufferObject->setOpenGLSpyID(glSpyContextID);
        }
        else
        {
            // TO_DO: gl-cl interp: detected errorL Trying to create an OpenGL - CL interoperability buffer,
            // in a not shared context..
        }
    }

    // OpenCL - OpenGL integration:
    pBufferObject->setGLBufferName(glVBOName);

    // Get the OpenGL spy, and search for the spy ID for this context:
    osProcedureAddress pProcAddress = NULL;
    suAPIConnector::instance().osGetSpyProcAddress(AP_OPENGL_API_CONNECTION, "gsShareGLVBOWithCLBuffer", pProcAddress);
    GT_IF_WITH_ASSERT(pProcAddress != NULL)
    {
        GSSHAREGLVBOWITHCLBUFFERPROC pShareGLVBOWithCLBufferProcAddress = (GSSHAREGLVBOWITHCLBUFFERPROC)pProcAddress;
        GT_IF_WITH_ASSERT(pShareGLVBOWithCLBufferProcAddress != NULL)
        {
            // Inform the OpenGL spy with the texture share:
            gtString detectedErrorStr;
            retVal = pShareGLVBOWithCLBufferProcAddress(clBufferIndex, pBufferObject->bufferName(), _spyContextId, glSpyContextID, glVBOName, detectedErrorStr);

            // TO_DO: gl-cl interp: handle detected error:
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::onMemoryObjectDeletion
// Description: Static function which is used for memory object release callback.
//              The function is called after a memory object is released, and is
//              getting the textures and buffers monitor pointer in the user data
//              argument of the function
// Arguments:   cl_mem memobj - the released memory object
//              void *user_data - the csImagesAndBuffersMonitor object
// Author:      Sigal Algranaty
// Date:        28/10/2010
// ---------------------------------------------------------------------------
void CL_CALLBACK csImagesAndBuffersMonitor::onMemoryObjectDeletion(cl_mem memobj, void* user_data)
{
    (void)(memobj); // unused
    // Sanity check:
    GT_IF_WITH_ASSERT(user_data != NULL)
    {
        // Convert the user data to a monitor:
        csImagesAndBuffersMonitor* pTexturesAndBuffersMonitor = (csImagesAndBuffersMonitor*)user_data;
        GT_IF_WITH_ASSERT(pTexturesAndBuffersMonitor != NULL)
        {
            // Uri, 27/10/11 - this clashes with our memory handling system as this does not account
            // for the AMD implementation's reference count handling. Namely, memory objects increase
            // their context's ref count and thus releasing them may cause the context's destruction.
            //
            // If OpenCL will have a global destructor API or if this behavior changes, we could restore this code.
            /*
            bool rc = pTexturesAndBuffersMonitor->onMemoryObjectDeletion(memobj);
            GT_ASSERT(rc);
            */
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::onMemoryObjectDeletion
// Description: Handling a memory object final deletion. Is removing the memory
//              object monitor from our database
// Arguments:   cl_mem memobj - the released memory object
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/10/2010
// ---------------------------------------------------------------------------
bool csImagesAndBuffersMonitor::onMemoryObjectDeletion(cl_mem memobj)
{
    bool retVal = true;

    // Prevent deletion on multiple threads (e.g. when the object is removed then
    // the destruction callback might be called at the same time:
    _memObjectDeletionCS.enter();

    // Search for the memory object:
    int deletedObjectIndex = -1;
    apCLMemObject* pMemoryObject = getMemObjectDetails((oaCLMemHandle)memobj, deletedObjectIndex);

    // NOTICE: We do not fail the function if the memory object was not found, since we have double memory handling
    // mechanism (in OpenCL1.0 we simply count the object reference count) so the object might have been destructed already:
    if (pMemoryObject != NULL)
    {
        // Get the memory object type:
        osTransferableObjectType memObjType = pMemoryObject->type();

        switch (memObjType)
        {
            case OS_TOBJ_ID_CL_BUFFER:
            {
                // If the object is a buffer:
                GT_IF_WITH_ASSERT((deletedObjectIndex >= 0) && (deletedObjectIndex < (int)_bufferMonitors.size()))
                {
                    // Remove the item from the vector:
                    _bufferMonitors.removeItem(deletedObjectIndex);
                }
            }
            break;

            case OS_TOBJ_ID_CL_SUB_BUFFER:
            {
                // If the object is a sub-buffer:
                GT_IF_WITH_ASSERT((deletedObjectIndex >= 0) && (deletedObjectIndex < (int)_bufferMonitors.size()))
                {
                    // Remove the item from the vector:
                    _subBufferMonitors.removeItem(deletedObjectIndex);
                }
            }
            break;

            case OS_TOBJ_ID_CL_IMAGE:
            {
                // If the object is a texture:
                GT_IF_WITH_ASSERT((deletedObjectIndex >= 0) && (deletedObjectIndex < (int)_imagesMonitors.size()))
                {
                    // Remove the item from the vector:
                    _imagesMonitors.removeItem(deletedObjectIndex);
                }
            }
            break;

            case OS_TOBJ_ID_CL_PIPE:
            {
                // If the object is a pipe:
                GT_IF_WITH_ASSERT((deletedObjectIndex >= 0) && (deletedObjectIndex < (int)m_pipeMonitors.size()))
                {
                    // Remove the item from the vector:
                    m_pipeMonitors.removeItem(deletedObjectIndex);
                }
            }
            break;

            default:
            {
                GT_ASSERT(false);
                retVal = false;
            }
            break;
        }
    }

    // Leave the critical section:
    _memObjectDeletionCS.leave();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csImagesAndBuffersMonitor::onMemObjectDestructorCallbackSet
// Description: Called when a mem object has its destructor callback set
// Author:      Uri Shomroni
// Date:        16/10/2013
// ---------------------------------------------------------------------------
void csImagesAndBuffersMonitor::onMemObjectDestructorCallbackSet(oaCLMemHandle hMem, osProcedureAddress64 pfnNotify, osProcedureAddress64 userData)
{
    apCLMemObject* pMem = getMemObjectDetails(hMem);
    GT_IF_WITH_ASSERT(NULL != pMem)
    {
        pMem->setDestructorCallback(pfnNotify, userData);
    }
}

bool csImagesAndBuffersMonitor::updateContextDataSnapshot()
{
    bool retVal = false;

    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetMemObjectInfo);

    // Go through the buffers and update its reference count:
    unsigned int buffersCount = (unsigned int)_bufferMonitors.size();

    for (unsigned int i = 0; i < buffersCount; i++)
    {
        apCLBuffer* pBuffer = _bufferMonitors[i];

        if (pBuffer != NULL)
        {
            // Get the context's amount of creation properties:
            cl_uint referenceCount = 0;
            cl_int rcGetRefCnt = cs_stat_realFunctionPointers.clGetMemObjectInfo(cl_mem(pBuffer->memObjectHandle()), CL_MEM_REFERENCE_COUNT, sizeof(cl_uint), &referenceCount, NULL);
            GT_IF_WITH_ASSERT(rcGetRefCnt == CL_SUCCESS)
            {
                // Subtract 1 for the reference that the debugger adds:
                pBuffer->setReferenceCount((gtUInt32)((referenceCount > 0) ? (referenceCount - 1) : referenceCount));
                retVal = true;
            }
        }
    }

    // Go through the subbuffers and update its reference count:
    unsigned int subBuffersCount = (unsigned int)_subBufferMonitors.size();

    for (unsigned int i = 0; i < subBuffersCount; i++)
    {
        apCLSubBuffer* pSubBuffer = _subBufferMonitors[i];

        if (pSubBuffer != NULL)
        {
            // Get the context's amount of creation properties:
            cl_uint referenceCount = 0;
            cl_int rcGetRefCnt = cs_stat_realFunctionPointers.clGetMemObjectInfo(cl_mem(pSubBuffer->memObjectHandle()), CL_MEM_REFERENCE_COUNT, sizeof(cl_uint), &referenceCount, NULL);
            GT_IF_WITH_ASSERT(rcGetRefCnt == CL_SUCCESS)
            {
                // Subtract 1 for the reference that the debugger adds:
                pSubBuffer->setReferenceCount((gtUInt32)((referenceCount > 0) ? (referenceCount - 1) : referenceCount));
                retVal = true;
            }
        }
    }

    // Go through the images and update its reference count:
    unsigned int imagesCount = (unsigned int)_imagesMonitors.size();

    for (unsigned int i = 0; i < imagesCount; i++)
    {
        apCLImage* pImage = _imagesMonitors[i];

        if (pImage != NULL)
        {
            // Get the context's amount of creation properties:
            cl_uint referenceCount = 0;
            cl_int rcGetRefCnt = cs_stat_realFunctionPointers.clGetMemObjectInfo(cl_mem(pImage->memObjectHandle()), CL_MEM_REFERENCE_COUNT, sizeof(cl_uint), &referenceCount, NULL);
            GT_IF_WITH_ASSERT(rcGetRefCnt == CL_SUCCESS)
            {
                // Subtract 1 for the reference that the debugger adds:
                pImage->setReferenceCount((gtUInt32)((referenceCount > 0) ? (referenceCount - 1) : referenceCount));
                retVal = true;
            }
        }
    }

    // Go through the pipes and update its reference count:
    unsigned int pipesCount = (unsigned int)m_pipeMonitors.size();

    for (unsigned int i = 0; i < pipesCount; i++)
    {
        apCLPipe* pPipe = m_pipeMonitors[i];

        if (pPipe != NULL)
        {
            // Get the context's amount of creation properties:
            cl_uint referenceCount = 0;
            cl_int rcGetRefCnt = cs_stat_realFunctionPointers.clGetMemObjectInfo((cl_mem)pPipe->memObjectHandle(), CL_MEM_REFERENCE_COUNT, sizeof(cl_uint), &referenceCount, NULL);
            GT_IF_WITH_ASSERT(rcGetRefCnt == CL_SUCCESS)
            {
                // Subtract 1 for the reference that the debugger adds:
                pPipe->setReferenceCount((gtUInt32)((referenceCount > 0) ? (referenceCount - 1) : referenceCount));
                retVal = true;
            }
        }
    }

    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetMemObjectInfo);

    return retVal;
}

