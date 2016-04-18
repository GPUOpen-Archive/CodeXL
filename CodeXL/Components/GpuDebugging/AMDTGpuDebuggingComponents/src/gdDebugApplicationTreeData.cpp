//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdDebugApplicationTreeData.cpp
///
//==================================================================================

//------------------------------ gdDebugApplicationTreeData.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeData.h>



// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeData::gdDebugApplicationTreeData
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        31/10/2010
// ---------------------------------------------------------------------------
gdDebugApplicationTreeData::gdDebugApplicationTreeData():
    _contextId(), _objectOpenGLName(0), _objectOwnerName(-1),
    _objectCLSpyID(-1), _objectOpenCLIndex(-1), _objectOpenCLName(-1), _objectOpenCLOwnerIndex(-1),
    _isMarkedForDeletion(false),
    _objectWidth(0), _objectHeight(0), _objectDepth(0), _amountOfRenderedVertices(0),
    _pieChartIndex(-1), _listViewIndex(-1), _textureType(AP_UNKNOWN_TEXTURE_TYPE), _mipmapStr(AF_STR_None),
    _internalFormat(0), _isMemorySizeEstimated(false), _dataFormat(OA_TEXEL_FORMAT_UNKNOWN), _dataType(OA_UNKNOWN_DATA_TYPE),
    _minLevel(0), _maxLevel(0), _textureLayer(0), _isDataCached(false), _bufferType(AP_DISPLAY_BUFFER_UNKNOWN), _bufferAttachmentTarget(0), _bufferAttachmentPoint(0),
    _fboAttachmentFBOName(0), _shaderType(GD_UNKNOWN_SHADER), _clProgramHandle(OA_CL_NULL_HANDLE), _clKernelHandle(OA_CL_NULL_HANDLE), _syncIndex(-1), _syncHandle(OA_GL_NULL_HANDLE), _syncCondition(GL_NONE),
    _addressingMode(0), _filterMode(0), _referenceCount(0), _amountOfEvents(0), m_queueOutOfOrderExecutionMode(false), m_queueProfilingMode(false), m_queueOnDevice(false), m_queueOnDeviceDefault(false), _memoryFlags(0), m_packetSize(0), m_maxPackets(0)

{
    _textureMiplevelID._textureMipLevel = 0;
    _textureMiplevelID._textureName = 0;
};


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeData::~gdDebugApplicationTreeData
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        21/3/2011
// ---------------------------------------------------------------------------
gdDebugApplicationTreeData::~gdDebugApplicationTreeData()
{

}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeData::copyID
// Description: Copy only data relevant for object identification
// Arguments:   gdDebugApplicationTreeData& other
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        29/12/2010
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeData::copyID(gdDebugApplicationTreeData& other) const
{
    other._contextId = _contextId;
    other._objectOpenGLName = _objectOpenGLName;
    other._objectOpenCLIndex = _objectOpenCLIndex;
    other._objectOpenCLName = _objectOpenCLName;
    other._objectOpenCLNameStr = _objectOpenCLNameStr;
    other._objectOwnerName = _objectOwnerName;
    other._objectOpenCLOwnerIndex = _objectOpenCLOwnerIndex;
    other._bufferType = _bufferType;
    other._bufferAttachmentTarget = _bufferAttachmentTarget;
    other._bufferAttachmentPoint = _bufferAttachmentPoint;
    other._fboAttachmentFBOName = _fboAttachmentFBOName;
    other._syncIndex = _syncIndex;
    other._textureMiplevelID._textureMipLevel = _textureMiplevelID._textureMipLevel;
    other._textureMiplevelID._textureName = _textureMiplevelID._textureName;
    other._multiVariableName = _multiVariableName;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        copyID
/// \brief Description: Copy my details for pOther
/// \param[in]          pOther
/// \return void
/// -----------------------------------------------------------------------------------------------
void gdDebugApplicationTreeData::copyID(afTreeDataExtension*& pOther) const
{
    if (pOther == NULL)
    {
        pOther = new gdDebugApplicationTreeData;

    }

    GT_IF_WITH_ASSERT(pOther != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pOther);
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            copyID(*pGDData);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeData::isSameObject
// Description: Return true iff the data represents the same object as the input one
// Arguments:   const gdDebugApplicationTreeData* pOtherItemData
//              bool compareMiplevels
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        31/10/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeData::isSameObject(const gdDebugApplicationTreeData* pOtherItemData, bool compareMiplevels) const
{
    bool retVal = false;

    // NOTICE:
    // Not all of the data parameters are compared, we only want to compare the objects
    // with unique ids (minimal information necessary) since we do not always have an item data
    // with all data filled in.
    if (pOtherItemData != NULL)
    {
        if (_contextId == pOtherItemData->_contextId)
        {
            if (_contextId._contextType == AP_OPENGL_CONTEXT)
            {
                // Compare only GL data for GL objects:
                retVal = ((_bufferType == pOtherItemData->_bufferType) &&
                          (_textureMiplevelID._textureName == pOtherItemData->_textureMiplevelID._textureName) &&
                          (_objectOpenGLName == pOtherItemData->_objectOpenGLName) &&
                          (_multiVariableName == pOtherItemData->_multiVariableName));
            }
            else if (_contextId._contextType == AP_OPENCL_CONTEXT)
            {
                retVal = ((_objectOpenCLName == pOtherItemData->_objectOpenCLName) &&
                          (_objectOpenCLNameStr == pOtherItemData->_objectOpenCLNameStr) &&
                          (_objectOwnerName == pOtherItemData->_objectOwnerName) &&
                          (_objectOpenCLOwnerIndex == pOtherItemData->_objectOpenCLOwnerIndex) &&
                          (_bufferType == pOtherItemData->_bufferType) &&
                          (_contextId == pOtherItemData->_contextId));
            }
            else
            {
                // Compare all data for null context:
                retVal = ((_objectOpenCLName == pOtherItemData->_objectOpenCLName) &&
                          (_objectOpenCLNameStr == pOtherItemData->_objectOpenCLNameStr) &&
                          (_objectOwnerName == pOtherItemData->_objectOwnerName) &&
                          (_objectOpenCLOwnerIndex == pOtherItemData->_objectOpenCLOwnerIndex) &&
                          (_bufferType == pOtherItemData->_bufferType) &&
                          (_contextId == pOtherItemData->_contextId) &&
                          (_textureMiplevelID._textureName == pOtherItemData->_textureMiplevelID._textureName) &&
                          (_multiVariableName == pOtherItemData->_multiVariableName));

            }
        }

        // Compare miplevels if needed:
        if (compareMiplevels)
        {
            retVal = retVal && (_textureMiplevelID._textureMipLevel == pOtherItemData->_textureMiplevelID._textureMipLevel);
        }
    }

    return retVal;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        isSameObject
/// \brief Description: Check if the input object represents the same item
/// \param[in]          pOtherItemData
/// \return True :
/// \return False:
/// -----------------------------------------------------------------------------------------------
bool gdDebugApplicationTreeData::isSameObject(afTreeDataExtension* pOtherItemData) const
{
    bool retVal = false;
    gdDebugApplicationTreeData* pOtherGDData = qobject_cast<gdDebugApplicationTreeData*>(pOtherItemData);

    if (pOtherGDData != NULL)
    {
        retVal = isSameObject(pOtherGDData, false);
    }

    return retVal;
}
