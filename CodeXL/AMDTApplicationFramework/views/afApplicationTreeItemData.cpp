//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afApplicationTreeItemData.cpp
///
//==================================================================================

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Local:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>

// ---------------------------------------------------------------------------
// Name:        afApplicationTreeItemData::afApplicationTreeItemData
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        31/10/2010
// ---------------------------------------------------------------------------
afApplicationTreeItemData::afApplicationTreeItemData(bool handleExtendedDataMemory):
    m_itemType(AF_TREE_ITEM_APP_ROOT), m_isItemRemovable(true), m_pTreeWidgetItem(nullptr), m_pOriginalItemTreeItem(nullptr),
    m_itemTreeIndex(-1), m_isRoot(false), m_objectCount(0), m_objectMemorySize(0),
    m_handleExtendedDataMemory(handleExtendedDataMemory), m_filePath(L""), m_filePathLineNumber(-1), m_pExtendedItemData(nullptr)
{
    _itemLoadStatus._itemLoadStatusType = AF_ITEM_NOT_LOADED;
    _itemLoadStatus._loadStatusDescription = AF_ITEM_ERROR_UNKNOWN;
};


// ---------------------------------------------------------------------------
// Name:        afApplicationTreeItemData::~afApplicationTreeItemData
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        21/3/2011
// ---------------------------------------------------------------------------
afApplicationTreeItemData::~afApplicationTreeItemData()
{
    if (m_handleExtendedDataMemory && (m_pExtendedItemData != nullptr))
    {
        delete m_pExtendedItemData;
    }
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTreeItemData::copyID
// Description: Copy only data relevant for object identification
// Arguments:   afApplicationTreeItemData& other
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        29/12/2010
// ---------------------------------------------------------------------------
void afApplicationTreeItemData::copyID(afApplicationTreeItemData& other) const
{
    other.m_itemType = m_itemType;
    other.m_filePath = m_filePath;
    other.m_filePathLineNumber = m_filePathLineNumber;

    if (m_pExtendedItemData != nullptr)
    {
        m_pExtendedItemData->copyID(other.m_pExtendedItemData);
    }
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTreeItemData::isSameObject
// Description: Return true iff the data represents the same object as the input one
//              This functions should be implemented by child classes
// Arguments:   const afApplicationTreeItemData* pOtherItemData
//              bool compareMiplevels
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        31/10/2010
// ---------------------------------------------------------------------------
bool afApplicationTreeItemData::isSameObject(const afApplicationTreeItemData* pOtherItemData) const
{
    bool retVal = false;

    if (pOtherItemData != nullptr)
    {
        retVal = (pOtherItemData->m_itemType == m_itemType);
        retVal = retVal && (pOtherItemData->m_filePath == m_filePath);

        if (retVal)
        {
            if (m_pExtendedItemData != nullptr)
            {
                retVal = m_pExtendedItemData->isSameObject(pOtherItemData->extendedItemData());
            }
            else
            {
                if (pOtherItemData->extendedItemData() != nullptr)
                {
                    retVal = false;
                }
            }
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        afApplicationTreeItemData::itemTypeToParent
// Description: Return item type parent
// Arguments:   afTreeItemType itemType
// Return Val:  afTreeItemType
// Author:      Sigal Algranaty
// Date:        31/10/2010
// ---------------------------------------------------------------------------
afTreeItemType afApplicationTreeItemData::itemTypeToParent(afTreeItemType itemType)
{
    afTreeItemType retVal = AF_TREE_ITEM_ITEM_NONE;

    switch (itemType)
    {
        case AF_TREE_ITEM_CL_CONTEXT:
        case AF_TREE_ITEM_GL_RENDER_CONTEXT:
        case AF_TREE_ITEM_GL_PBUFFERS_NODE:
        case AF_TREE_ITEM_GL_SYNC_OBJECTS_NODE:
        case AF_TREE_ITEM_APP_ROOT:
        case AF_TREE_ITEM_MESSAGE:
            retVal = AF_TREE_ITEM_APP_ROOT;
            break;

        case AF_TREE_ITEM_GL_PROGRAMS_NODE:
        case AF_TREE_ITEM_GL_SHADERS_NODE:
        case AF_TREE_ITEM_GL_DISPLAY_LISTS_NODE:
            retVal = AF_TREE_ITEM_GL_RENDER_CONTEXT;
            break;

        case AF_TREE_ITEM_GL_SYNC_OBJECT:
            retVal = AF_TREE_ITEM_GL_SYNC_OBJECTS_NODE;
            break;

        case AF_TREE_ITEM_GL_TEXTURE:
            retVal = AF_TREE_ITEM_GL_TEXTURES_NODE;
            break;

        case AF_TREE_ITEM_GL_STATIC_BUFFER:
            retVal = AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE;
            break;

        case AF_TREE_ITEM_GL_TEXTURES_NODE:
            retVal = AF_TREE_ITEM_GL_RENDER_CONTEXT;
            break;

        case AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE:
        case AF_TREE_ITEM_GL_PROGRAM_PIPELINES_NODE:
        case AF_TREE_ITEM_GL_SAMPLERS_NODE:
        case AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE:
        case AF_TREE_ITEM_GL_FBO_NODE:
        case AF_TREE_ITEM_GL_VBO_NODE:
            retVal = AF_TREE_ITEM_GL_RENDER_CONTEXT;
            break;

        case AF_TREE_ITEM_GL_RENDER_BUFFER:
            retVal = AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE;
            break;

        case AF_TREE_ITEM_GL_PROGRAM_PIPELINE:
            retVal = AF_TREE_ITEM_GL_PROGRAM_PIPELINES_NODE;
            break;

        case AF_TREE_ITEM_GL_SAMPLER:
            retVal = AF_TREE_ITEM_GL_SAMPLERS_NODE;
            break;

        case AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER:
            retVal = AF_TREE_ITEM_GL_PBUFFER_NODE;
            break;

        case AF_TREE_ITEM_GL_VBO:
            retVal = AF_TREE_ITEM_GL_VBO_NODE;
            break;

        case AF_TREE_ITEM_GL_PROGRAM:
            retVal = AF_TREE_ITEM_GL_PROGRAMS_NODE;
            break;

        case AF_TREE_ITEM_GL_VERTEX_SHADER:
        case AF_TREE_ITEM_GL_TESSELLATION_CONTROL_SHADER:
        case AF_TREE_ITEM_GL_TESSELLATION_EVALUATION_SHADER:
        case AF_TREE_ITEM_GL_GEOMETRY_SHADER:
        case AF_TREE_ITEM_GL_FRAGMENT_SHADER:
        case AF_TREE_ITEM_GL_COMPUTE_SHADER:
        case AF_TREE_ITEM_GL_UNSUPPORTED_SHADER:
            retVal = AF_TREE_ITEM_GL_SHADERS_NODE;
            break;

        case AF_TREE_ITEM_GL_DISPLAY_LIST:
            retVal = AF_TREE_ITEM_GL_DISPLAY_LISTS_NODE;
            break;

        case AF_TREE_ITEM_GL_FBO:
            retVal = AF_TREE_ITEM_GL_FBO_NODE;
            break;

        case AF_TREE_ITEM_GL_FBO_ATTACHMENT:
            retVal = AF_TREE_ITEM_GL_FBO;
            break;

        case AF_TREE_ITEM_CL_IMAGE:
            retVal = AF_TREE_ITEM_CL_IMAGES_NODE;
            break;

        case AF_TREE_ITEM_CL_BUFFER:
            retVal = AF_TREE_ITEM_CL_BUFFERS_NODE;
            break;

        case AF_TREE_ITEM_CL_SUB_BUFFER:
            retVal = AF_TREE_ITEM_CL_BUFFER;
            break;

        case AF_TREE_ITEM_CL_PIPE:
            retVal = AF_TREE_ITEM_CL_PIPES_NODE;
            break;

        case AF_TREE_ITEM_CL_SAMPLER:
            retVal = AF_TREE_ITEM_CL_SAMPLERS_NODE;
            break;

        case AF_TREE_ITEM_CL_EVENT:
            retVal = AF_TREE_ITEM_CL_EVENTS_NODE;
            break;

        case AF_TREE_ITEM_CL_COMMAND_QUEUE:
            retVal = AF_TREE_ITEM_CL_COMMAND_QUEUES_NODE;
            break;

        case AF_TREE_ITEM_CL_PROGRAM:
            retVal = AF_TREE_ITEM_CL_PROGRAMS_NODE;
            break;

        case AF_TREE_ITEM_CL_KERNEL:
            retVal = AF_TREE_ITEM_CL_KERNEL;
            break;

        case AF_TREE_ITEM_CL_COMMAND_QUEUES_NODE:
        case AF_TREE_ITEM_CL_SAMPLERS_NODE:
        case AF_TREE_ITEM_CL_EVENTS_NODE:
        case AF_TREE_ITEM_CL_PROGRAMS_NODE:
        case AF_TREE_ITEM_CL_IMAGES_NODE:
        case AF_TREE_ITEM_CL_BUFFERS_NODE:
        case AF_TREE_ITEM_CL_PIPES_NODE:
            retVal = AF_TREE_ITEM_CL_CONTEXT;
            break;

        default:
        {
            GT_ASSERT_EX(false, L"Unknown object type");
            break;
        }

    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTreeItemData::isItemTypeRoot
// Description: Return true iff this item should appear as single unique child for its parent
// Arguments:   afTreeItemType itemType
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/11/2010
// ---------------------------------------------------------------------------
bool afApplicationTreeItemData::isItemTypeRoot(afTreeItemType itemType)
{
    bool retVal = false;

    switch (itemType)
    {
        case AF_TREE_ITEM_APP_ROOT:
        case AF_TREE_ITEM_GL_TEXTURES_NODE:
        case AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE:
        case AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE:
        case AF_TREE_ITEM_GL_PBUFFERS_NODE:
        case AF_TREE_ITEM_GL_PBUFFER_NODE:
        case AF_TREE_ITEM_GL_SYNC_OBJECTS_NODE:
        case AF_TREE_ITEM_GL_VBO_NODE:
        case AF_TREE_ITEM_GL_PROGRAMS_NODE:
        case AF_TREE_ITEM_GL_PROGRAM_PIPELINES_NODE:
        case AF_TREE_ITEM_GL_SAMPLERS_NODE:
        case AF_TREE_ITEM_GL_SHADERS_NODE:
        case AF_TREE_ITEM_GL_DISPLAY_LISTS_NODE:
        case AF_TREE_ITEM_GL_FBO_NODE:
        case AF_TREE_ITEM_CL_IMAGES_NODE:
        case AF_TREE_ITEM_CL_BUFFERS_NODE:
        case AF_TREE_ITEM_CL_PIPES_NODE:
        case AF_TREE_ITEM_CL_SAMPLERS_NODE:
        case AF_TREE_ITEM_CL_EVENTS_NODE:
        case AF_TREE_ITEM_CL_COMMAND_QUEUES_NODE:
        case AF_TREE_ITEM_CL_PROGRAMS_NODE:
        {
            retVal = true;
        }
        break;

        default:
        {
            retVal = false;
            break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTreeItemData::isItemImageOrBuffer
// Description: Return true if the item type is a buffer or an image type
// Arguments:   afTreeItemType itemType
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        14/11/2010
// ---------------------------------------------------------------------------
bool afApplicationTreeItemData::isItemImageOrBuffer(afTreeItemType itemType)
{
    bool retVal = false;

    // Check if item is a viewable object (Texture or Buffer)
    retVal = ((itemType == AF_TREE_ITEM_GL_TEXTURE) ||
              (itemType == AF_TREE_ITEM_GL_VBO) ||
              (itemType == AF_TREE_ITEM_CL_IMAGE) ||
              (itemType == AF_TREE_ITEM_CL_BUFFER) ||
              (itemType == AF_TREE_ITEM_CL_SUB_BUFFER) ||
              (itemType == AF_TREE_ITEM_GL_STATIC_BUFFER) ||
              (itemType == AF_TREE_ITEM_GL_RENDER_BUFFER) ||
              (itemType == AF_TREE_ITEM_GL_FBO_ATTACHMENT) ||
              (itemType == AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER));

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afApplicationTreeItemData::isItemImage
// Description: Return true if the item type is an image
// Arguments:   afTreeItemType itemType
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/9/2011
// ---------------------------------------------------------------------------
bool afApplicationTreeItemData::isItemImage(afTreeItemType itemType)
{
    bool retVal = false;

    // Check if the item can be viewed as an image
    retVal = ((itemType == AF_TREE_ITEM_GL_TEXTURE)         ||
              (itemType == AF_TREE_ITEM_CL_IMAGE)           ||
              (itemType == AF_TREE_ITEM_GL_STATIC_BUFFER)   ||
              (itemType == AF_TREE_ITEM_GL_RENDER_BUFFER)   ||
              (itemType == AF_TREE_ITEM_GL_FBO_ATTACHMENT)  ||
              (itemType == AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER));

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afApplicationTreeItemData::isItemBuffer
// Description: Return true if the item type is a buffer
// Arguments:   afTreeItemType itemType
// Return Val:  bool - Success / failure.
// Author:      Yuri Rshtunique
// Date:        17/11/2014
// ---------------------------------------------------------------------------
bool afApplicationTreeItemData::isItemBuffer(afTreeItemType itemType)
{
    bool retVal = false;

    // Check if an item can be represented with data only ( not as an image)
    retVal = ((itemType == AF_TREE_ITEM_GL_VBO) ||
              (itemType == AF_TREE_ITEM_CL_BUFFER) ||
              (itemType == AF_TREE_ITEM_CL_SUB_BUFFER));
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afApplicationTreeItemData::isItemSource
// Description: Return true iff the item type can be displayed as source code
// Arguments:   afTreeItemType itemType
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/8/2011
// ---------------------------------------------------------------------------
bool afApplicationTreeItemData::isItemSource(afTreeItemType itemType)
{
    bool retVal = false;

    // Check if item is a viewable object (Texture or Buffer)
    retVal = ((itemType == AF_TREE_ITEM_CL_PROGRAM) ||
              (itemType == AF_TREE_ITEM_CL_KERNEL) ||
              (itemType == AF_TREE_ITEM_GL_VERTEX_SHADER) ||
              (itemType == AF_TREE_ITEM_GL_TESSELLATION_CONTROL_SHADER) ||
              (itemType == AF_TREE_ITEM_GL_TESSELLATION_EVALUATION_SHADER) ||
              (itemType == AF_TREE_ITEM_GL_GEOMETRY_SHADER) ||
              (itemType == AF_TREE_ITEM_GL_FRAGMENT_SHADER) ||
              (itemType == AF_TREE_ITEM_GL_COMPUTE_SHADER) ||
              (itemType == AF_TREE_ITEM_GL_UNSUPPORTED_SHADER));

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTreeItemData::isItemThumbnail
// Description: Return true iff this item is an images / buffers thumbnail item
// Arguments:   afTreeItemType itemType
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/12/2010
// ---------------------------------------------------------------------------
bool afApplicationTreeItemData::isItemThumbnail(afTreeItemType itemType)
{
    bool retVal = false;

    switch (itemType)
    {
        case AF_TREE_ITEM_GL_TEXTURES_NODE:
        case AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE:
        case AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE:
        case AF_TREE_ITEM_GL_PBUFFERS_NODE:
        case AF_TREE_ITEM_GL_VBO_NODE:
        case AF_TREE_ITEM_GL_FBO_NODE:
        case AF_TREE_ITEM_CL_IMAGES_NODE:
        case AF_TREE_ITEM_CL_BUFFERS_NODE:
        {
            retVal = true;
        }
        break;

        default:
        {
            retVal = false;
            break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTreeItemData::itemTypeAsString
// Description: Convert a tree item type to a string
// Arguments:   afTreeItemType itemType
//              gtString& itemTypeStr
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        3/1/2011
// ---------------------------------------------------------------------------
bool afApplicationTreeItemData::itemTypeAsString(afTreeItemType itemType, gtString& itemTypeStr)
{
    bool retVal = true;

    switch (itemType)
    {
        case AF_TREE_ITEM_APP_ROOT:
            itemTypeStr = AF_STR_AppRoot;
            break;

        case AF_TREE_ITEM_MESSAGE:
            itemTypeStr = AF_STR_Message;
            break;

        case AF_TREE_ITEM_GL_RENDER_CONTEXT:
            itemTypeStr = AF_STR_GLContext;
            break;

        case AF_TREE_ITEM_GL_TEXTURES_NODE:
            itemTypeStr = AF_STR_GLTextures;
            break;

        case AF_TREE_ITEM_GL_TEXTURE:
            itemTypeStr = AF_STR_GLTexture;
            break;

        case AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE:
            itemTypeStr = AF_STR_GLRenderBuffers;
            break;

        case AF_TREE_ITEM_GL_RENDER_BUFFER:
            itemTypeStr = AF_STR_GLRenderBuffer;
            break;

        case AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE:
            itemTypeStr = AF_STR_GLStaticBuffers;
            break;

        case AF_TREE_ITEM_GL_STATIC_BUFFER:
            itemTypeStr = AF_STR_GLStaticBuffer;
            break;

        case AF_TREE_ITEM_GL_PBUFFERS_NODE:
            itemTypeStr = AF_STR_GLAllPBuffers;
            break;

        case AF_TREE_ITEM_GL_PBUFFER_NODE:
            itemTypeStr = AF_STR_GLPBuffers;
            break;

        case AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER:
            itemTypeStr = AF_STR_GLAllPBuffers;
            break;

        case AF_TREE_ITEM_GL_SYNC_OBJECTS_NODE:
            itemTypeStr = AF_STR_GLSyncs;
            break;

        case AF_TREE_ITEM_GL_SYNC_OBJECT:
            itemTypeStr = AF_STR_GLSync;
            break;

        case AF_TREE_ITEM_GL_VBO_NODE:
            itemTypeStr = AF_STR_GLVBOs;
            break;

        case AF_TREE_ITEM_GL_VBO:
            itemTypeStr = AF_STR_GLVBO;
            break;

        case AF_TREE_ITEM_GL_PROGRAMS_NODE:
            itemTypeStr = AF_STR_Programs;
            break;

        case AF_TREE_ITEM_GL_PROGRAM:
            itemTypeStr = AF_STR_Program;
            break;

        case AF_TREE_ITEM_GL_SHADERS_NODE:
            itemTypeStr = AF_STR_GLShaders;
            break;

        case AF_TREE_ITEM_GL_VERTEX_SHADER:
            itemTypeStr = AF_STR_GLVertexShader;
            break;

        case AF_TREE_ITEM_GL_TESSELLATION_CONTROL_SHADER:
            itemTypeStr = AF_STR_GLTessellationControlShader;
            break;

        case AF_TREE_ITEM_GL_TESSELLATION_EVALUATION_SHADER:
            itemTypeStr = AF_STR_GLTessellationEvaluationShader;
            break;

        case AF_TREE_ITEM_GL_GEOMETRY_SHADER:
            itemTypeStr = AF_STR_GLGeometryShader;
            break;

        case AF_TREE_ITEM_GL_FRAGMENT_SHADER:
            itemTypeStr = AF_STR_GLFragmentShader;
            break;

        case AF_TREE_ITEM_GL_COMPUTE_SHADER:
            itemTypeStr = AF_STR_GLComputeShader;
            break;

        case AF_TREE_ITEM_GL_UNSUPPORTED_SHADER:
            itemTypeStr = AF_STR_GLUnknownShader;
            break;

        case AF_TREE_ITEM_GL_DISPLAY_LISTS_NODE:
            itemTypeStr = AF_STR_GLDisplayLists;
            break;

        case AF_TREE_ITEM_GL_DISPLAY_LIST:
            itemTypeStr = AF_STR_GLDisplayList;
            break;

        case AF_TREE_ITEM_GL_FBO_NODE:
            itemTypeStr = AF_STR_GLFBOs;
            break;

        case AF_TREE_ITEM_GL_FBO:
            itemTypeStr = AF_STR_GLFBO;
            break;

        case AF_TREE_ITEM_GL_FBO_ATTACHMENT:
            itemTypeStr = AF_STR_GLFBOAttachment;
            break;

        case AF_TREE_ITEM_CL_CONTEXT:
            itemTypeStr = AF_STR_CLContext;
            break;

        case AF_TREE_ITEM_CL_IMAGES_NODE:
            itemTypeStr = AF_STR_CLImages;
            break;

        case AF_TREE_ITEM_CL_IMAGE:
            itemTypeStr = AF_STR_CLImage;
            break;

        case AF_TREE_ITEM_CL_BUFFERS_NODE:
            itemTypeStr = AF_STR_CLBuffers;
            break;

        case AF_TREE_ITEM_CL_BUFFER:
            itemTypeStr = AF_STR_CLBuffer;
            break;

        case AF_TREE_ITEM_CL_SUB_BUFFER:
            itemTypeStr = AF_STR_CLSubBuffer;
            break;

        case AF_TREE_ITEM_CL_PIPES_NODE:
            itemTypeStr = AF_STR_CLPipes;
            break;

        case AF_TREE_ITEM_CL_PIPE:
            itemTypeStr = AF_STR_CLPipe;
            break;

        case AF_TREE_ITEM_CL_SAMPLERS_NODE:
            itemTypeStr = AF_STR_CLSamplers;
            break;

        case AF_TREE_ITEM_CL_SAMPLER:
            itemTypeStr = AF_STR_CLSampler;
            break;

        case AF_TREE_ITEM_CL_EVENTS_NODE:
            itemTypeStr = AF_STR_CLEvents;
            break;

        case AF_TREE_ITEM_CL_EVENT:
            itemTypeStr = AF_STR_CLEvent;
            break;

        case AF_TREE_ITEM_CL_COMMAND_QUEUES_NODE:
            itemTypeStr = AF_STR_CLCommandQueues;
            break;

        case AF_TREE_ITEM_CL_COMMAND_QUEUE:
            itemTypeStr = AF_STR_CLCommandQueue;
            break;

        case AF_TREE_ITEM_CL_DEVICE:
            itemTypeStr = AF_STR_CLDevice;
            break;

        case AF_TREE_ITEM_CL_PROGRAMS_NODE:
            itemTypeStr = AF_STR_Programs;
            break;

        case AF_TREE_ITEM_CL_PROGRAM:
            itemTypeStr = AF_STR_Program;
            break;

        case AF_TREE_ITEM_CL_KERNEL:
            itemTypeStr = AF_STR_CLKernel;
            break;

        case AF_TREE_ITEM_CL_KERNEL_VARIABLE:
            itemTypeStr = AF_STR_CLKernelVariable;
            break;

        case AF_TREE_ITEM_PROFILE_SESSION:
            itemTypeStr = AF_STR_ProfileSession;
            break;

        case AF_TREE_ITEM_PROFILE_SESSION_TYPE:
            itemTypeStr = AF_STR_ProfileSessionType;
            break;

        case AF_TREE_ITEM_PROFILE_CPU_OVERVIEW:
            itemTypeStr = AF_STR_ProfileSessionCPUOverview;
            break;

        case AF_TREE_ITEM_PROFILE_CPU_MODULES:
            itemTypeStr = AF_STR_ProfileSessionCPUModules;
            break;

        case AF_TREE_ITEM_PROFILE_CPU_CALL_GRAPH:
            itemTypeStr = AF_STR_ProfileSessionCPUCallGraph;
            break;

        case AF_TREE_ITEM_PROFILE_CPU_FUNCTIONS:
            itemTypeStr = AF_STR_ProfileSessionCPUFunctions;
            break;

        case AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODES:
            itemTypeStr = AF_STR_ProfileSessionCPUSourceCodes;
            break;

        case AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODE:
            itemTypeStr = AF_STR_ProfileSessionCPUSourceCode;
            break;

        case AF_TREE_ITEM_PROFILE_GPU_SUMMARY:
            itemTypeStr = AF_STR_ProfileSessionGPUSummary;
            break;

        case AF_TREE_ITEM_PROFILE_GPU_API_SUMMARY:
            itemTypeStr = AF_STR_ProfileSessionGPUAPISummary;
            break;

        case AF_TREE_ITEM_PROFILE_GPU_CONTEXT_SUMMARY:
            itemTypeStr = AF_STR_ProfileSessionGPUContextSummary;
            break;

        case AF_TREE_ITEM_PROFILE_KERNEL_SUMMARY:
            itemTypeStr = AF_STR_ProfileSessionGPUKernelSummary;
            break;

        case AF_TREE_ITEM_PROFILE_GPU_TOP10_TRANSFER_SUMMARY:
            itemTypeStr = AF_STR_ProfileSessionGPUTop10TransferSummary;
            break;

        case AF_TREE_ITEM_PROFILE_GPU_TOP10_KERNEL_SUMMARY:
            itemTypeStr = AF_STR_ProfileSessionGPUTop10KernelsSummary;
            break;

        case AF_TREE_ITEM_PROFILE_GPU_BEST_PRACTICE_SUMMARY:
            itemTypeStr = AF_STR_ProfileSessionGPUBestPracticeSummary;
            break;

        case AF_TREE_ITEM_PROFILE_GPU_KERNELS:
            itemTypeStr = AF_STR_ProfileSessionGPUKernels;
            break;

        case AF_TREE_ITEM_PROFILE_GPU_KERNEL:
            itemTypeStr = AF_STR_ProfileSessionGPUKernel;
            break;

        case AF_TREE_ITEM_PP_SUMMARY:
            itemTypeStr = AF_STR_ProfileSessionPPSummary;
            break;

        case AF_TREE_ITEM_PP_TIMELINE:
            itemTypeStr = AF_STR_ProfileSessionPPTimeline;
            break;

        case AF_TREE_ITEM_KA_FILE:
            itemTypeStr = AF_STR_AnalyzerFile;
            break;

        case AF_TREE_ITEM_KA_EXE_FILE:
            itemTypeStr = AF_STR_AnalyzerExeFile;
            break;

        case AF_TREE_ITEM_KA_SOURCE:
            itemTypeStr = AF_STR_AnalyzerOpenCLSource;
            break;

        case AF_TREE_ITEM_KA_KERNEL:
            itemTypeStr = AF_STR_AnalyzerKernel;
            break;

        case AF_TREE_ITEM_KA_STATISTICS:
            itemTypeStr = AF_STR_AnalyzerStatistics;
            break;

        case AF_TREE_ITEM_KA_ANALYSIS:
            itemTypeStr = AF_STR_AnalyzerAnalysis;
            break;

        case AF_TREE_ITEM_KA_DEVICE:
            itemTypeStr = AF_STR_AnalyzerDevice;
            break;

        case AF_TREE_ITEM_KA_HISTORY:
            itemTypeStr = AF_STR_AnalyzerHistory;
            break;

        case AF_TREE_ITEM_KA_ADD_FILE:
            itemTypeStr = AF_STR_AnalyzerAddFile;
            break;

        case AF_TREE_ITEM_KA_NEW_FILE:
            itemTypeStr = AF_STR_AnalyzerNewFile;
            break;

        default:
            retVal = false;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTreeItemData::getItemStatusAsString
// Description: Translate the item status to a string:
// Arguments:   gtString& statusStr
//              gtString& statusTitle
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/4/2011
// ---------------------------------------------------------------------------
bool afApplicationTreeItemData::getItemStatusAsString(afTreeItemType itemType, gtString& statusTitle, gtString& statusStr)
{
    bool retVal = true;

    if (!isItemDisplayed())
    {
        // If the item was failed to load, add a failure header:
        gtString itemTypeStr;
        afApplicationTreeItemData::itemTypeAsString(itemType, itemTypeStr);
        statusTitle.appendFormattedString(AF_STR_ImagesAndBuffersViewObjectUnavailableMessageHeader, itemTypeStr.asCharArray());
    }

    switch (_itemLoadStatus._loadStatusDescription)
    {

        case AF_ITEM_LOAD_EMPTY_THUMBS:
        {
            // Get the item type as string:
            gtString itemTypeStr;
            afApplicationTreeItemData::itemTypeAsString(itemType, itemTypeStr);
            itemTypeStr.append('s');
            statusStr.appendFormattedString(AF_STR_ImagesAndBuffersViewerNoObjectsAvailableMessage, itemTypeStr.asCharArray());
        }
        break;

        case AF_ITEM_LOAD_CL_GL_INTEROP:
        {
            statusStr = AF_STR_ImagesAndBuffersViewGLCLInterop;
            statusTitle = AF_STR_ImagesAndBuffersViewKernelInteroperabilityTitle;
        }
        break;

        case AF_ITEM_LOAD_GLBEGIN_END_BLOCK:
        {
            statusStr = AF_STR_ImagesAndBuffersViewGLBeginEndMessage;
            statusTitle = AF_STR_ImagesAndBuffersViewGLBeginEndTitle;
        }

        break;

        case AF_ITEM_LOAD_PROCESS_IS_RUNNING:
        {
            statusTitle = AF_STR_ImagesAndBuffersViewImagesBuffersProcessIsRunningTitle;
            statusStr = (itemType == AF_TREE_ITEM_CL_KERNEL_VARIABLE) ? AF_STR_ImagesAndBuffersViewVariablesProcessIsRunning : AF_STR_ImagesAndBuffersViewImagesBuffersProcessIsRunning;
        }
        break;

        case AF_ITEM_LOAD_PROCESS_IS_TERMINATED:
        {
            statusTitle = AF_STR_ImagesAndBuffersViewImagesBuffersProcessIsNotRunningTitle;
            statusStr = (itemType == AF_TREE_ITEM_CL_KERNEL_VARIABLE) ? AF_STR_ImagesAndBuffersViewVariablesProcessIsRunning : AF_STR_ImagesAndBuffersViewImagesBuffersProcessIsRunning;
        }
        break;

        case AF_ITEM_LOAD_NOT_IN_KERNEL_DEBUGGING:
        {
            statusTitle = AF_STR_ImagesAndBuffersViewKernelIsNotDebuggedTitle;
            statusStr = AF_STR_ImagesAndBuffersViewKernelIsNotDebugged;
        }
        break;

        case AF_ITEM_LOAD_EMPTY_VARIABLE_NAME:
        {
            statusStr = AF_STR_MultiWatchViewVariableNameMessage;
            statusTitle = AF_STR_MultiWatchViewVariableNameTitle;
        }
        break;

        case AF_ITEM_LOAD_TEXTURE_TYPE_UNKNOWN:
        {
            statusTitle = AF_STR_ImagesAndBuffersViewObjectUnavailableMessageTitle;
            statusStr = AF_STR_ImagesAndBuffersViewTextureTypeUnknown;
        }
        break;

        case AF_ITEM_LOAD_IMAGE_TYPE_UNKNOWN:
        {
            statusStr = AF_STR_ImagesAndBuffersViewImageTypeUnknown;
            statusTitle = AF_STR_ImagesAndBuffersViewObjectUnavailableMessageTitle;
        }
        break;

        case AF_ITEM_LOAD_STATIC_BUFFER_BOUND_TO_FBO:
        {
            statusStr = AF_STR_ImagesAndBuffersViewBoundToFBO;
            statusTitle = AF_STR_ImagesAndBuffersViewObjectUnavailableMessageTitle;
        }
        break;

        case AF_ITEM_LOAD_VARIABLE_LOAD_FAILURE:
        {
            statusStr = AF_STR_Empty;
            statusTitle = AF_STR_ImagesAndBuffersViewObjectUnavailableMessageTitle;

        }
        break;

        case AF_ITEM_LOAD_VARIABLE_UNSUPPORTED_TYPE:
        {
            statusTitle = AF_STR_ImagesAndBuffersViewObjectUnavailableMessageTitle;
            statusStr = AF_STR_MultiWatchViewKernelVariableUnsupportedType;
        }
        break;

        case AF_ITEM_LOAD_VARIABLE_NOT_EXIST:
        {
            statusTitle = AF_STR_ImagesAndBuffersViewVariableDoNotExistMessageTitle;
            statusStr = AF_STR_MultiWatchViewVariableNameMessage;
        }
        break;

        default:
        {
            GT_ASSERT_EX(false, L"unsupporoted load failure");
            statusStr = AF_STR_ImagesAndBuffersViewLoadFailure;
            retVal = false;
        }
        break;
    }


    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTreeItemData::isItemDisplayed
// Description: Checks if the item is displayed according to the item load status
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/4/2011
// ---------------------------------------------------------------------------
bool afApplicationTreeItemData::isItemDisplayed()
{
    bool retVal = false;
    retVal = ((_itemLoadStatus._itemLoadStatusType == AF_ITEM_LOAD_SUCCESS) ||
              (_itemLoadStatus._itemLoadStatusType == AF_ITEM_PAGE_NOT_LOADED) ||
              (_itemLoadStatus._itemLoadStatusType == AF_ITEM_PAGE_LOAD_ERROR) ||
              (_itemLoadStatus._loadStatusDescription == AF_ITEM_LOAD_EMPTY_THUMBS));

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afApplicationTreeItemData::setItemLoadStatus
// Description: Sets the item load status and description
// Arguments:   afItemLoadStatusType itemLoadStatus
//              afItemLoadFailureDescription loadDescription /*= GD_ITEM_ERROR_NONE*/
// Author:      Sigal Algranaty
// Date:        6/4/2011
// ---------------------------------------------------------------------------
void afApplicationTreeItemData::setItemLoadStatus(afItemLoadStatusType itemLoadStatus, afItemLoadFailureDescription loadDescription)
{
    _itemLoadStatus._itemLoadStatusType = itemLoadStatus;
    _itemLoadStatus._loadStatusDescription = loadDescription;
}

void afApplicationTreeItemData::setExtendedData(afTreeDataExtension* pExtensionData)
{
    m_pExtendedItemData = pExtensionData;

    if ((pExtensionData != nullptr) && (pExtensionData->m_pParentData == nullptr))
    {
        pExtensionData->m_pParentData = this;
    }
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTreeMessageItemData::isSameObject
// Description: Comparator
// Author:      Uri Shomroni
// Date:        10/11/2014
// ---------------------------------------------------------------------------
bool afApplicationTreeMessageItemData::isSameObject(afTreeDataExtension* pOtherItemData) const
{
    bool retVal = false;

    afApplicationTreeMessageItemData* pOtherAsMessage = qobject_cast<afApplicationTreeMessageItemData*>(pOtherItemData);

    if (nullptr != pOtherAsMessage)
    {
        retVal = (m_messageTitle == pOtherAsMessage->m_messageTitle) && (m_messageText == pOtherAsMessage->m_messageText);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationTreeMessageItemData::copyID
// Description: Copies identifying details
// Author:      Uri Shomroni
// Date:        10/11/2014
// ---------------------------------------------------------------------------
void afApplicationTreeMessageItemData::copyID(afTreeDataExtension*& pOtherItemData) const
{
    if (nullptr == pOtherItemData)
    {
        afApplicationTreeMessageItemData* pExtendedData = new afApplicationTreeMessageItemData;

        pExtendedData->m_messageTitle = m_messageTitle;
        pExtendedData->m_messageText = m_messageText;

        pOtherItemData = pExtendedData;
    }
    else
    {
        afApplicationTreeMessageItemData* pOtherAsMessage = qobject_cast<afApplicationTreeMessageItemData*>(pOtherItemData);
        GT_IF_WITH_ASSERT(nullptr != pOtherAsMessage)
        {
            pOtherAsMessage->m_messageTitle = m_messageTitle;
            pOtherAsMessage->m_messageText = m_messageText;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afCreateMessageTreeItemData
// Description: Creates an item data for a message tree item
// Author:      Uri Shomroni
// Date:        10/11/2014
// ---------------------------------------------------------------------------
AF_API afApplicationTreeItemData* afCreateMessageTreeItemData(const gtString& messageTitle, const gtString& messageText)
{
    afApplicationTreeItemData* retVal = new afApplicationTreeItemData(true);
    afApplicationTreeMessageItemData* pExtendedData = new afApplicationTreeMessageItemData;

    pExtendedData->m_messageTitle = messageTitle;
    pExtendedData->m_messageText = messageText;
    pExtendedData->m_pParentData = retVal;

    retVal->setExtendedData(pExtendedData);
    retVal->m_itemType = AF_TREE_ITEM_MESSAGE;

    return retVal;
}

