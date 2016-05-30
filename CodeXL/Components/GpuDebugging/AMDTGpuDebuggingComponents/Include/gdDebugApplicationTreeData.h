//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdDebugApplicationTreeData.h
///
//==================================================================================

//------------------------------ gdMonitoredObjectTreeData.h ----------------------------

#ifndef __GDMONITOREDOBJECTTREEDATA
#define __GDMONITOREDOBJECTTREEDATA

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>
#include <AMDTAPIClasses/Include/apOpenCLParameters.h>
#include <AMDTAPIClasses/Include/apTextureType.h>
#include <AMDTAPIClasses/Include/apGLTexture.h>
#include <AMDTAPIClasses/Include/apDisplayBuffer.h>
#include <AMDTAPIClasses/Include/apContextID.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afTreeItemType.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdShaderType.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>



// ----------------------------------------------------------------------------------
// Class Name:          gdDebugApplicationTreeData
// General Description: Item data for monitored objects tree.
//                      The class contain basic information for the identification of
//                      the object, and pointer to a more specific item data classes.
//                      When object navigation tree is inherited, and a more expanded use
//                      of the object data is needed, another type of object data should be
//                      created, and be added as member to this class.
// Author:              Sigal Algranaty
// Creation Date:       26/9/2010
// ----------------------------------------------------------------------------------
class GD_API gdDebugApplicationTreeData : public afTreeDataExtension
{

    Q_OBJECT

public:
    gdDebugApplicationTreeData();
    virtual ~gdDebugApplicationTreeData();

    virtual bool isSameObject(afTreeDataExtension* pOtherItemData) const;
    virtual void copyID(afTreeDataExtension*& pOther) const ;

    void copyID(gdDebugApplicationTreeData& other) const ;

    bool isSameObject(const gdDebugApplicationTreeData* pOtherItemData, bool compareMiplevels = false) const;
    static bool compareTreeItemData(const afApplicationTreeItemData* pItemData1, const afApplicationTreeItemData* pItemData2, bool compareMiplevels = false);

    // Context ID:
    apContextID _contextId;

    // GL ID:
    GLuint _objectOpenGLName;

    int _objectOwnerName;

    // CL ID:
    int _objectCLSpyID;
    int _objectOpenCLIndex;
    int _objectOpenCLName;
    int _objectOpenCLOwnerIndex;
    gtString _objectOpenCLNameStr;

    // Memory leaks:
    bool _isMarkedForDeletion;

    // Generic memory item data:
    GLint _objectWidth;
    GLint _objectHeight;
    GLint _objectDepth;
    gtUInt32 _amountOfRenderedVertices;
    int _pieChartIndex;
    int _listViewIndex;

    // Textures:
    apGLTextureMipLevelID _textureMiplevelID;
    apTextureType _textureType;
    gtString _mipmapStr;
    GLenum _internalFormat;
    GLenum _requestedInternalFormat;
    bool _isMemorySizeEstimated;
    oaTexelDataFormat _dataFormat;
    oaDataType _dataType;
    int _minLevel;
    int _maxLevel;
    int _textureLayer;

    // Item load data:
    bool _isDataCached;

    // Map texture type -> amount:
    gtMap<apTextureType, int> _textureTypesAmount;
    gtMap<gdShaderType, int> _shaderTypesAmount;

    // Render / Static buffers:
    apDisplayBuffer _bufferType;

    // VBOs & FBOs::
    GLenum _bufferAttachmentTarget;
    GLenum _bufferAttachmentPoint;
    GLuint _fboAttachmentFBOName;

    // Shaders:
    gdShaderType _shaderType;

    // OpenCL Kernel & Programs:
    oaCLProgramHandle _clProgramHandle;
    oaCLKernelHandle _clKernelHandle;

    // Sync objects:
    int _syncIndex;
    oaGLSyncHandle _syncHandle;
    GLenum _syncCondition;

    // CL Samplers:
    cl_addressing_mode _addressingMode;
    cl_filter_mode _filterMode;

    // CL Command queue:
    int _referenceCount;
    int _amountOfEvents;
    bool m_queueOutOfOrderExecutionMode;
    bool m_queueProfilingMode;
    bool m_queueOnDevice;
    bool m_queueOnDeviceDefault;

    // CL Mem Objects:
    apCLMemFlags _memoryFlags;

    // CL pipes:
    gtUInt32 m_packetSize;
    gtUInt32 m_maxPackets;

    // CL-GL Interop:
    bool m_isAcquired;

    // Multi variables:
    gtString _multiVariableName;
};


#endif  // __GDMONITOREDOBJECTTREEDATA
