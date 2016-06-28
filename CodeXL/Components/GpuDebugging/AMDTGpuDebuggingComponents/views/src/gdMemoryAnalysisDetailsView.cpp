//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdMemoryAnalysisDetailsView.cpp
///
//==================================================================================

//------------------------------ gdMemoryAnalysisDetailsView.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTAPIClasses/Include/apTextureType.h>
#include <AMDTAPIClasses/Include/apCLSampler.h>
#include <AMDTAPIClasses/Include/apCLImage.h>
#include <AMDTAPIClasses/Include/apDefaultTextureNames.h>
#include <AMDTAPIClasses/Include/apGLRenderBuffer.h>
#include <AMDTAPIClasses/Include/apGLVBO.h>
#include <AMDTAPIClasses/Include/apInternalFormat.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdMemoryAnalysisDetailsView.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeData.h>


#define GD_MEMORY_DETAILS_VIEW_MIN_COLUMN_WIDTH 20

// Uri, 23/06/15 - Event queue logging is currently disabled since it was more trouble than it's worth.
// This needs to be re-enabled and all its problems should be fixed (csCommandQueueMonitor.m_logQueueEvents)
// before this information is to be displayed in the GUI.
// #define GD_MEMORY_ANALYSIS_DETAILS_DISPLAY_CL_QUEUE_EVENTS

// ---------------------------------------------------------------------------
// Name:        sortIntValues / sortStringValues
// Description: Return sort value for 2 integer / strings values
// Arguments:   int value1
//              int value2
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        26/9/2010
// ---------------------------------------------------------------------------
bool sortIntValues(bool shouldSortUp, int value1, int value2);
bool sortStringValues(bool shouldSortUp, const gtString& value1, const gtString& value2);

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::gdMemoryAnalysisDetailsView
// Description: Constructor.
// Arguments:   parent - My parent window.
// Author:      Avi Shapira
// Date:        29/1/2006
// ---------------------------------------------------------------------------
gdMemoryAnalysisDetailsView::gdMemoryAnalysisDetailsView(QWidget* pParent, afProgressBarWrapper* pProgressBar, gdDebugApplicationTreeHandler* pMemoryTree)
    : acListCtrl(pParent),
      afBaseView(pProgressBar), _pMemoryTree(pMemoryTree), _chosenContextId(0), _shouldIgnoreSizeEvent(false), _pExportAction(NULL)
{
    // Create the columns and default message:
    clearAndDisplayMessage();

    // Initialize object type to sort type mapping:
    initSortTypeMapping();

    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);

    bool rcConnect = connect(horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(onColumnHeaderClick(int)));
    GT_ASSERT(rcConnect);

    gdDebugApplicationTreeData* pGDData = new gdDebugApplicationTreeData;

    m_currentDisplayedItem.setExtendedData(pGDData);
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::~gdMemoryAnalysisDetailsView
// Description: Destructor
// Author:      Avi Shapira
// Date:        29/1/2006
// ---------------------------------------------------------------------------
gdMemoryAnalysisDetailsView::~gdMemoryAnalysisDetailsView()
{
    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);

    // Delete all mapping items:
    gtMap<afTreeItemType, gtVector<gdMemoryAnalysisDetailsSortDirection>*>::const_iterator iter = _memoryObjectTypeClickedColumnToSortType.begin();
    gtMap<afTreeItemType, gtVector<gdMemoryAnalysisDetailsSortDirection>*>::const_iterator iterEnd = _memoryObjectTypeClickedColumnToSortType.end();

    while (iter != iterEnd)
    {
        gtVector<gdMemoryAnalysisDetailsSortDirection>* mapping = (*iter).second;

        if (mapping != NULL)
        {
            delete mapping;
        }

        iter++;
    }

    _memoryObjectTypeClickedColumnToSortType.clear();
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::onEvent
// Description: Responds to tree item selection and activation event
// Arguments:   const apEvent& eve
// Author:      Sigal Algranaty
// Date:        21/7/2008
// ---------------------------------------------------------------------------
void gdMemoryAnalysisDetailsView::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(eve);  // unused
    (void)(vetoEvent);  // unused
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::setListCtrlColumns
// Description: Create the list control columns according to the currently displayed
//              list type
// Author:      Sigal Algranaty
// Date:        29/9/2008
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::setListCtrlColumns()
{
    bool retVal = true;

    // Vector that contain the columns strings according to the object type:
    QStringList columnsStringByObjectType;

    switch (m_currentDisplayedItem.m_itemType)
    {
        case AF_TREE_ITEM_GL_TEXTURES_NODE:
        case AF_TREE_ITEM_GL_TEXTURE:
        {
            // Create the "Memory Details View" columns for textures:
            columnsStringByObjectType << AF_STR_NameA;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemorySize;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListTextureType;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListDimensions;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListTextureMipmap;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListTextureInternalFormat;
            break;
        }

        case AF_TREE_ITEM_GL_RENDER_BUFFER:
        case AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE:
        {
            // Create the "Memory Details View" columns for rebder buffers:
            columnsStringByObjectType << AF_STR_NameA;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemorySize;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListAttachment;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListDimensions;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListPixelFormatFormat;
            columnsStringByObjectType << AF_STR_TypeA;
            break;
        }

        case AF_TREE_ITEM_GL_VBO:
        case AF_TREE_ITEM_GL_VBO_NODE:
        {
            // Create the "Memory Details View" columns for VBOs:
            columnsStringByObjectType << AF_STR_NameA;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemorySize;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListAttachment;
            break;
        }

        case AF_TREE_ITEM_GL_SYNC_OBJECT:
        case AF_TREE_ITEM_GL_SYNC_OBJECTS_NODE:
        {
            // Create the "Memory Details View" columns for sync objects:
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListSyncHandle;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemorySize;
            break;
        }

        case AF_TREE_ITEM_GL_STATIC_BUFFER:
        case AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE:
        {
            // Create the "Memory Details View" columns for static buffers:
            columnsStringByObjectType << AF_STR_NameA;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemorySize;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListDimensions;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListPixelFormatFormat;
            columnsStringByObjectType << AF_STR_TypeA;
            break;
        }

        case AF_TREE_ITEM_GL_SAMPLER:
        case AF_TREE_ITEM_GL_SAMPLERS_NODE:
        case AF_TREE_ITEM_GL_PROGRAM_PIPELINE:
        case AF_TREE_ITEM_GL_PROGRAM_PIPELINES_NODE:
        case AF_TREE_ITEM_GL_PROGRAM:
        case AF_TREE_ITEM_GL_PROGRAMS_NODE:
        {
            // Create the "Memory Details View" columns for programs:
            columnsStringByObjectType << AF_STR_NameA;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemorySize;
            break;
        }

        case AF_TREE_ITEM_GL_VERTEX_SHADER:
        case AF_TREE_ITEM_GL_TESSELLATION_CONTROL_SHADER:
        case AF_TREE_ITEM_GL_TESSELLATION_EVALUATION_SHADER:
        case AF_TREE_ITEM_GL_GEOMETRY_SHADER:
        case AF_TREE_ITEM_GL_FRAGMENT_SHADER:
        case AF_TREE_ITEM_GL_COMPUTE_SHADER:
        case AF_TREE_ITEM_GL_UNSUPPORTED_SHADER:
        case AF_TREE_ITEM_GL_SHADERS_NODE:
        {
            // Create the "Memory Details View" columns for shaders:
            columnsStringByObjectType << AF_STR_NameA;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListSourceCodeSize;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListShaderType;
            break;
        }

        case AF_TREE_ITEM_GL_DISPLAY_LIST:
        case AF_TREE_ITEM_GL_DISPLAY_LISTS_NODE:
        {
            // Create the "Memory Details View" columns for display lists:
            columnsStringByObjectType << AF_STR_NameA;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemorySize;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListVerticesAmount;
            break;
        }

        case AF_TREE_ITEM_GL_FBO_NODE:
        {
            // Create the "Memory Details View" columns for fbos:
            columnsStringByObjectType << AF_STR_NameA;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemorySize;
            break;
        }

        case AF_TREE_ITEM_GL_FBO:
        case AF_TREE_ITEM_GL_FBO_ATTACHMENT:
        {
            // Create the "Memory Details View" columns for FBOs attachments:
            columnsStringByObjectType << AF_STR_NameA;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemorySize;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListAttachmentTarget;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListAttachmentPoint;
            break;
        }

        case AF_TREE_ITEM_GL_RENDER_CONTEXT:
        case AF_TREE_ITEM_CL_CONTEXT:
        {
            // Create the "Memory Details View" columns for render context:
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListObjectType;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemorySize;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListNumberOfObjects;
            break;
        }

        case AF_TREE_ITEM_GL_PBUFFERS_NODE:
        {
            // Create the "Memory Details View" columns for PBuffers:
            columnsStringByObjectType << AF_STR_NameA;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemorySize;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListDimensions;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListRenderContext;
            break;
        }

        case AF_TREE_ITEM_GL_PBUFFER_NODE:
        case AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER:
        {
            // Create the "Memory Details View" columns for PBuffers:
            columnsStringByObjectType << AF_STR_NameA;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemorySize;
            columnsStringByObjectType << AF_STR_TypeA;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListDimensions;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListRenderContext;
            break;
        }

        case AF_TREE_ITEM_APP_ROOT:
        {
            // Create the "Memory Details View" columns for application root:
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListRenderContextName;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemorySize;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListNumberOfObjects;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListReferenceCount;
            break;
        }

        case AF_TREE_ITEM_ITEM_NONE:
        {
            // This item "type" is used to symbolize no items (just a single column with the view title)
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListCaption;
            break;
        }

        case AF_TREE_ITEM_CL_IMAGES_NODE:
        case AF_TREE_ITEM_CL_IMAGE:
        {
            // Create the "Memory Details View" columns for OpenCL textures:
            columnsStringByObjectType << AF_STR_NameA;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemorySize;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListTextureType;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListDimensions;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListTexturePixelFormat;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListTextureDataType;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemoryFlags;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListReferenceCount;
            break;
        }

        case AF_TREE_ITEM_CL_BUFFERS_NODE:
        case AF_TREE_ITEM_CL_BUFFER:
        case AF_TREE_ITEM_CL_SUB_BUFFER:
        {
            // Create the "Memory Details View" columns for OpenCL buffers:
            columnsStringByObjectType << AF_STR_NameA;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemorySize;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemoryFlags;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListReferenceCount;
            break;
        }

        case AF_TREE_ITEM_CL_PIPES_NODE:
        case AF_TREE_ITEM_CL_PIPE:
        {
            // Create the "Memory Details View" columns for OpenCL pipes:
            columnsStringByObjectType << AF_STR_NameA;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemorySize;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListPipePacketSize;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListPipeMaxPackets;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemoryFlags;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListReferenceCount;
            break;
        }

        case AF_TREE_ITEM_CL_COMMAND_QUEUE:
        case AF_TREE_ITEM_CL_COMMAND_QUEUES_NODE:
        {
            // Create the "Memory Details View" columns for OpenCL command queues:
            columnsStringByObjectType << AF_STR_NameA;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemorySize;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListReferenceCount;
#ifdef GD_MEMORY_ANALYSIS_DETAILS_DISPLAY_CL_QUEUE_EVENTS
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerAmountOfEvents;
#endif
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListOutOfOrderExecutionMode;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListProfilingMode;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListQueueOnDevice;
            break;
        }

        case AF_TREE_ITEM_CL_PROGRAM:
        case AF_TREE_ITEM_CL_KERNEL:
        case AF_TREE_ITEM_CL_PROGRAMS_NODE:
        {
            // Create the "Memory Details View" columns for OpenCL programs:
            columnsStringByObjectType << AF_STR_NameA;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemorySize;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListReferenceCount;
            break;
        }

        case AF_TREE_ITEM_CL_SAMPLER:
        case AF_TREE_ITEM_CL_SAMPLERS_NODE:
        {
            // Create the "Memory Details View" columns for OpenCL samplers:
            columnsStringByObjectType << AF_STR_NameA;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemorySize;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListFilterMode;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListAddressingMode;
            break;
        }

        case AF_TREE_ITEM_CL_EVENT:
        case AF_TREE_ITEM_CL_EVENTS_NODE:
        {
            // Create the "Memory Details View" columns for OpenCL events:
            columnsStringByObjectType << AF_STR_NameA;
            columnsStringByObjectType << GD_STR_MemoryAnalysisViewerListMemorySize;
            break;
        }

        default:
        {
            GT_ASSERT_EX(false, L"Unknown memory object type");
            retVal = false;
            break;
        }
    }



    GT_IF_WITH_ASSERT(retVal)
    {
        gtVector<float> vColumnWidths;
        getColumnsWidths(vColumnWidths);

        // Build the columns according to the selected item type:
        initHeaders(columnsStringByObjectType, vColumnWidths, false);
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::initSortTypeMapping
// Description: Initializes a mapping between and object type + clicked column to the
//              necessary sort type
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        13/10/2008
// ---------------------------------------------------------------------------
void gdMemoryAnalysisDetailsView::initSortTypeMapping()
{
    // Insert application root sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings1 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings1->push_back(GD_SORT_BY_NAME);
    pMappings1->push_back(GD_SORT_BY_SIZE);
    pMappings1->push_back(GD_SORT_BY_OBJECTS_COUNT);
    pMappings1->push_back(GD_SORT_BY_REF_COUNT);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_APP_ROOT] = pMappings1;

    // Insert render context root sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings2 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings2->push_back(GD_SORT_BY_NAME);
    pMappings2->push_back(GD_SORT_BY_SIZE);
    pMappings2->push_back(GD_SORT_BY_OBJECTS_COUNT);
    pMappings2->push_back(GD_SORT_BY_REF_COUNT);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_GL_RENDER_CONTEXT] = pMappings2;

    // Insert texture sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings3 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings3->push_back(GD_SORT_BY_NAME);
    pMappings3->push_back(GD_SORT_BY_SIZE);
    pMappings3->push_back(GD_SORT_BY_TYPE);
    pMappings3->push_back(GD_SORT_BY_DIMENSIONS);
    pMappings3->push_back(GD_SORT_BY_MIPMAP);
    pMappings3->push_back(GD_SORT_BY_FORMAT);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_GL_TEXTURES_NODE] = pMappings3;

    // Insert texture sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings4 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings4->push_back(GD_SORT_BY_NAME);
    pMappings4->push_back(GD_SORT_BY_SIZE);
    pMappings4->push_back(GD_SORT_BY_ATTACHMENT_TARGET);
    pMappings4->push_back(GD_SORT_BY_DIMENSIONS);
    pMappings4->push_back(GD_SORT_BY_DATA_TYPE);
    pMappings4->push_back(GD_SORT_BY_PIXEL_TYPE);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE] = pMappings4;

    // Insert static buffers sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings5 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings5->push_back(GD_SORT_BY_NAME);
    pMappings5->push_back(GD_SORT_BY_SIZE);
    pMappings5->push_back(GD_SORT_BY_DIMENSIONS);
    pMappings5->push_back(GD_SORT_BY_DATA_TYPE);
    pMappings5->push_back(GD_SORT_BY_PIXEL_TYPE);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE] = pMappings5;

    // Insert application root sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings6 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings6->push_back(GD_SORT_BY_NAME);
    pMappings6->push_back(GD_SORT_BY_SIZE);
    pMappings6->push_back(GD_SORT_BY_DIMENSIONS);
    pMappings6->push_back(GD_SORT_BY_RENDER_CONTEXT);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_GL_PBUFFERS_NODE] = pMappings6;

    // Insert static buffers sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings7 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings7->push_back(GD_SORT_BY_NAME);
    pMappings7->push_back(GD_SORT_BY_SIZE);
    pMappings7->push_back(GD_SORT_BY_DIMENSIONS);
    pMappings7->push_back(GD_SORT_BY_DATA_TYPE);
    pMappings7->push_back(GD_SORT_BY_PIXEL_TYPE);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_GL_PBUFFER_NODE] = pMappings7;


    // Insert vbo sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings8 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings8->push_back(GD_SORT_BY_NAME);
    pMappings8->push_back(GD_SORT_BY_SIZE);
    pMappings8->push_back(GD_SORT_BY_VBO_ATTACHMENT);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_GL_VBO_NODE] = pMappings8;

    // Insert sync sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings9 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings9->push_back(GD_SORT_BY_SYNC_HANDLE);
    pMappings9->push_back(GD_SORT_BY_SIZE);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_GL_SYNC_OBJECT] = pMappings9;

    // Insert program sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings10 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings10->push_back(GD_SORT_BY_NAME);
    pMappings10->push_back(GD_SORT_BY_SIZE);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_GL_PROGRAMS_NODE] = pMappings10;

    // Insert shader sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings11 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings11->push_back(GD_SORT_BY_NAME);
    pMappings11->push_back(GD_SORT_BY_SIZE);
    pMappings11->push_back(GD_SORT_BY_SHADER_TYPE);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_GL_SHADERS_NODE] = pMappings11;

    // Insert display list sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings12 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings12->push_back(GD_SORT_BY_NAME);
    pMappings12->push_back(GD_SORT_BY_SIZE);
    pMappings12->push_back(GD_SORT_BY_AMOUNT_OF_VERTICES);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_GL_DISPLAY_LISTS_NODE] = pMappings12;

    // Insert FBO sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings13 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings13->push_back(GD_SORT_BY_NAME);
    pMappings13->push_back(GD_SORT_BY_SIZE);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_GL_FBO_NODE] = pMappings13;

    // Insert FBO attachment sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings14 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings14->push_back(GD_SORT_BY_NAME);
    pMappings14->push_back(GD_SORT_BY_SIZE);
    pMappings14->push_back(GD_SORT_BY_ATTACHMENT_TARGET);
    pMappings14->push_back(GD_SORT_BY_ATTACHMENT_POINT);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_GL_FBO_ATTACHMENT] = pMappings14;

    // Insert render context root sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings20 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings20->push_back(GD_SORT_BY_NAME);
    pMappings20->push_back(GD_SORT_BY_SIZE);
    pMappings20->push_back(GD_SORT_BY_OBJECTS_COUNT);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_CL_CONTEXT] = pMappings20;

    // Insert texture sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings15 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings15->push_back(GD_SORT_BY_NAME);
    pMappings15->push_back(GD_SORT_BY_SIZE);
    pMappings15->push_back(GD_SORT_BY_TYPE);
    pMappings15->push_back(GD_SORT_BY_DIMENSIONS);
    pMappings15->push_back(GD_SORT_BY_PIXEL_TYPE);
    pMappings15->push_back(GD_SORT_BY_FORMAT);
    pMappings15->push_back(GD_SORT_BY_MEMORY_FLAGS);
    pMappings15->push_back(GD_SORT_BY_REF_COUNT);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_CL_IMAGES_NODE] = pMappings15;

    // Insert CL buffers sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings16 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings16->push_back(GD_SORT_BY_NAME);
    pMappings16->push_back(GD_SORT_BY_SIZE);
    pMappings16->push_back(GD_SORT_BY_MEMORY_FLAGS);
    pMappings16->push_back(GD_SORT_BY_REF_COUNT);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_CL_BUFFERS_NODE] = pMappings16;

    // Insert CL command queues sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings17 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings17->push_back(GD_SORT_BY_NAME);
    pMappings17->push_back(GD_SORT_BY_SIZE);
    pMappings17->push_back(GD_SORT_BY_REF_COUNT);
#ifdef GD_MEMORY_ANALYSIS_DETAILS_DISPLAY_CL_QUEUE_EVENTS
    pMappings17->push_back(GD_SORT_BY_AMOUNT_OF_EVENTS);
#endif
    pMappings17->push_back(GD_SORT_BY_OUT_OF_ORDER_EXE_MODE);
    pMappings17->push_back(GD_SORT_BY_PROFILING_MODE);
    pMappings17->push_back(GD_SORT_BY_ON_DEVICE);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_CL_COMMAND_QUEUES_NODE] = pMappings17;

    // Insert CL program sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings18 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings18->push_back(GD_SORT_BY_NAME);
    pMappings18->push_back(GD_SORT_BY_SIZE);
    pMappings18->push_back(GD_SORT_BY_REF_COUNT);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_CL_PROGRAMS_NODE] = pMappings18;

    // Insert CL program sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings19 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings19->push_back(GD_SORT_BY_NAME);
    pMappings19->push_back(GD_SORT_BY_SIZE);
    pMappings19->push_back(GD_SORT_BY_FILTER_MODE);
    pMappings19->push_back(GD_SORT_BY_ADDRESSING_MODE);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_CL_SAMPLERS_NODE] = pMappings19;

    // Insert CL event sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings21 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings21->push_back(GD_SORT_BY_NAME);
    pMappings21->push_back(GD_SORT_BY_SIZE);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_CL_EVENTS_NODE] = pMappings21;

    // Insert GL program pipeline mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings22 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings22->push_back(GD_SORT_BY_NAME);
    pMappings22->push_back(GD_SORT_BY_SIZE);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_GL_PROGRAM_PIPELINES_NODE] = pMappings22;

    // Insert GL sampler mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings23 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings23->push_back(GD_SORT_BY_NAME);
    pMappings23->push_back(GD_SORT_BY_SIZE);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_GL_SAMPLERS_NODE] = pMappings23;

    // Insert CL pipes sort mappings:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pMappings24 = new gtVector<gdMemoryAnalysisDetailsSortDirection>;
    pMappings24->push_back(GD_SORT_BY_NAME);
    pMappings24->push_back(GD_SORT_BY_SIZE);
    pMappings24->push_back(GD_SORT_BY_PACKET_SIZE);
    pMappings24->push_back(GD_SORT_BY_MAX_PACKETS);
    pMappings24->push_back(GD_SORT_BY_MEMORY_FLAGS);
    pMappings24->push_back(GD_SORT_BY_REF_COUNT);
    _memoryObjectTypeClickedColumnToSortType[AF_TREE_ITEM_CL_PIPES_NODE] = pMappings24;

    _sortInfo._sortType = GD_SORT_BY_NAME;
    _sortInfo._sortOrder = Qt::DescendingOrder;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::onColumnHeaderClick
// Description: Is called when a column header is clicked
// Arguments:   int columnIndex
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        22/1/2012
// ---------------------------------------------------------------------------
void gdMemoryAnalysisDetailsView::onColumnHeaderClick(int columnIndex)
{
    // Get the current sort type:
    gtVector<gdMemoryAnalysisDetailsSortDirection>* pCurrentMapping = _memoryObjectTypeClickedColumnToSortType[m_currentDisplayedItem.m_itemType];

    if (pCurrentMapping != NULL)
    {
        GT_IF_WITH_ASSERT(columnIndex < (int)pCurrentMapping->size())
        {
            int previousSortType = _sortInfo._sortType;

            _sortInfo._sortType = (*pCurrentMapping)[columnIndex];

            // Toggle if necessary:
            if (_sortInfo._sortType == previousSortType)
            {
                // Reverse the order if we clicked the same column twice
                if (_sortInfo._sortOrder == Qt::AscendingOrder)
                {
                    _sortInfo._sortOrder = Qt::DescendingOrder;
                }
                else
                {
                    _sortInfo._sortOrder = Qt::AscendingOrder;
                }
            }

            // Sort the table items:
            sortItems(columnIndex, _sortInfo._sortOrder);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::extendContextMenu
// Description: Extend the default context menu with the specific actions of
//              this view
// Author:      Sigal Algranaty
// Date:        22/1/2012
// ---------------------------------------------------------------------------
void gdMemoryAnalysisDetailsView::extendContextMenu()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pContextMenu != NULL)
    {
        // Add a separator:
        m_pContextMenu->addSeparator();

        // Create delete breakpoint action:
        _pExportAction = new QAction(GD_STR_MemoryAnalysisViewerExportMemoryData, this);

        // Connect the action to delete slot:
        bool rcConnect = connect(_pExportAction, SIGNAL(triggered()), this, SLOT(onSaveMemoryData()));
        GT_ASSERT(rcConnect);

        // Add the actions to the menu:
        m_pContextMenu->addAction(_pExportAction);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::sizeStringFromNumber
// Description: inputs size into sizeString by adding thousands separators and
//              a unit, eg 2543 -> "2,543 KB".
// Return Val: void
// Author:      Uri Shomroni
// Date:        27/10/2008
// ---------------------------------------------------------------------------
void gdMemoryAnalysisDetailsView::sizeStringFromNumber(gtString& sizeString, gtUInt32 size)
{
    sizeString.makeEmpty();

    // Build the object size's string:
    if (size > 0)
    {
        sizeString.appendFormattedString(L"%d", size);
        sizeString.addThousandSeperators();
        sizeString.append(AF_STR_Space AF_STR_KilobytesShort);
    }
    else
    {
        sizeString.append(L"0 " AF_STR_KilobytesShort);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::onSaveStatisticsData
// Description: Export the total statistics data to a file
// Arguments: wxCommandEvent& event
// Return Val: void
// Author:      Sigal Algranaty
// Date:        4/8/2008
// ---------------------------------------------------------------------------
void gdMemoryAnalysisDetailsView::onSaveMemoryData()
{
    // Open Save Dialog:
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
    {
        // Show the save file as dialog:
        QString csvFilePathStr;
        bool rc = pApplicationCommands->ShowQTSaveCSVFileDialog(csvFilePathStr, GD_STR_saveMemoryFileName, this);

        if (rc)
        {
            // Get the current project name:
            gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();

            // Write the string to a file:
            writeListDataToFile(acQStringToGTString(csvFilePathStr), projectName, GD_STR_MemoryAnalysisFileDescription);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::onAboutToShowContextMenu
// Description: Is called before the context menu is shown
// Author:      Sigal Algranaty
// Date:        22/1/2012
// ---------------------------------------------------------------------------
void gdMemoryAnalysisDetailsView::onAboutToShowContextMenu()
{
    // Call the base class implementation:
    acListCtrl::onAboutToShowContextMenu();

    GT_IF_WITH_ASSERT(_pExportAction != NULL)
    {
        bool isEnabled = (rowCount() > 1);
        _pExportAction->setEnabled(isEnabled);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::allocateNewWidgetItem
// Description: Override the standard widget item
// Arguments:   const QString& text
// Return Val:  QTableWidgetItem*
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
QTableWidgetItem* gdMemoryAnalysisDetailsView::allocateNewWidgetItem(const QString& text)
{
    // Allocate my own widget item:
    return new gdMemoryAnalysisDetailsView::gdMemoryWidgetItem(text, this);
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsTableWidgetItem::gdStatisticsTableWidgetItem
// Description: Constructor
// Arguments:   const QString& text
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
gdMemoryAnalysisDetailsView::gdMemoryWidgetItem::gdMemoryWidgetItem(const QString& text, gdMemoryAnalysisDetailsView* pParent)
    : QTableWidgetItem(text), _pParent(pParent)
{

}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsTableWidgetItem::~gdStatisticsTableWidgetItem
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
gdMemoryAnalysisDetailsView::gdMemoryWidgetItem::~gdMemoryWidgetItem()
{

}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::operator<
// Description: Override for sorting
// Arguments:   const QTreeWidgetItem & other
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::gdMemoryWidgetItem::operator<(const QTableWidgetItem& other) const
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pParent != NULL)
    {
        // Get both item data:
        afApplicationTreeItemData* pMyItemData = (afApplicationTreeItemData*)_pParent->getItemData(row());
        afApplicationTreeItemData* pOtherItemData = (afApplicationTreeItemData*)_pParent->getItemData(other.row());

        // Compare both items:
        retVal = _pParent->isItemSmallerThen(pMyItemData, pOtherItemData);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::getColumnsWidths
// Description: Calculate the column widths according to the current object shown
// Arguments:   gtVector<float>& vColumnWidths
// Author:      Sigal Algranaty
// Date:        22/1/2012
// ---------------------------------------------------------------------------
void gdMemoryAnalysisDetailsView::getColumnsWidths(gtVector<float>& vColumnWidths)
{
    // Vector that contain the ratio of the different columns, with -1 in the last column visible
    // Any columns after the -1 will go out the view (creating a scrollbar). To avoid flickering
    // when changing lists because of rounding, every set must have at least one -1.
    switch (m_currentDisplayedItem.m_itemType)
    {

        case AF_TREE_ITEM_GL_TEXTURE:
        case AF_TREE_ITEM_GL_TEXTURES_NODE:
        {
            // Set the "Memory Details View" columns widths for textures:
            vColumnWidths.push_back(0.15F);     // Name
            vColumnWidths.push_back(0.175F);    // Mem Size
            vColumnWidths.push_back(0.125F);    // Tex Type
            vColumnWidths.push_back(0.175F);    // Dimensions
            vColumnWidths.push_back(0.175F);        // Mipmap
            vColumnWidths.push_back(-1.0F);     // Int Format (0.2)
            break;
        }

        case AF_TREE_ITEM_GL_RENDER_BUFFER:
        case AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE:
        {
            // Set the "Memory Details View" columns widths for render buffers:
            vColumnWidths.push_back(0.175F);    // Name
            vColumnWidths.push_back(0.15F);     // Mem Size
            vColumnWidths.push_back(0.15F);     // Attachment
            vColumnWidths.push_back(0.175F);    // Dimensions
            vColumnWidths.push_back(0.225F);    // Format
            vColumnWidths.push_back(-1.0F);     // Type (0.125)
            break;
        }

        case AF_TREE_ITEM_GL_VBO:
        case AF_TREE_ITEM_GL_VBO_NODE:
        {
            // Set the "Memory Details View" columns widths for VBOs:
            vColumnWidths.push_back(0.175F);    // Name
            vColumnWidths.push_back(0.15F);     // Mem Size
            vColumnWidths.push_back(-1.0F);     // Attachment
            break;
        }

        case AF_TREE_ITEM_GL_STATIC_BUFFER:
        case AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE:
        {
            // Set the "Memory Details View" columns widths for static buffers:
            vColumnWidths.push_back(0.225F);    // Name
            vColumnWidths.push_back(0.175F);    // Mem Size
            vColumnWidths.push_back(0.175F);    // Dimensions
            vColumnWidths.push_back(0.275F);    // Format
            vColumnWidths.push_back(-1.0F);     // Type (0.15)
            break;
        }

        case AF_TREE_ITEM_GL_PROGRAM:
        case AF_TREE_ITEM_GL_PROGRAMS_NODE:
        {
            // Set the "Memory Details View" columns widths for programs:
            vColumnWidths.push_back(0.2F);      // Name
            vColumnWidths.push_back(-1.0F);     // Mem Size
            break;
        }

        case AF_TREE_ITEM_GL_PROGRAM_PIPELINE:
        case AF_TREE_ITEM_GL_PROGRAM_PIPELINES_NODE:
        {
            // Set the "Memory Details View" columns widths for program pipelines:
            vColumnWidths.push_back(0.3F);      // Name
            vColumnWidths.push_back(-1.0F);     // Memory Size
            break;
        }

        case AF_TREE_ITEM_GL_SAMPLER:
        case AF_TREE_ITEM_GL_SAMPLERS_NODE:
        {
            // Set the "Memory Details View" columns widths for GL samplers:
            vColumnWidths.push_back(0.25F);      // Name
            vColumnWidths.push_back(-1.0F);     // Memory Size
            break;
        }

        case AF_TREE_ITEM_GL_VERTEX_SHADER:
        case AF_TREE_ITEM_GL_TESSELLATION_CONTROL_SHADER:
        case AF_TREE_ITEM_GL_TESSELLATION_EVALUATION_SHADER:
        case AF_TREE_ITEM_GL_GEOMETRY_SHADER:
        case AF_TREE_ITEM_GL_FRAGMENT_SHADER:
        case AF_TREE_ITEM_GL_COMPUTE_SHADER:
        case AF_TREE_ITEM_GL_UNSUPPORTED_SHADER:
        case AF_TREE_ITEM_GL_SHADERS_NODE:
        {
            // Set the "Memory Details View" columns widths for shaders:
            vColumnWidths.push_back(0.3F);      // Name
            vColumnWidths.push_back(0.2F);      // Source Code Size
            vColumnWidths.push_back(-1.0F);     // Shader Type
            break;
        }

        case AF_TREE_ITEM_GL_DISPLAY_LIST:
        case AF_TREE_ITEM_GL_DISPLAY_LISTS_NODE:
        {
            // Set the "Memory Details View" columns widths for display lists:
            vColumnWidths.push_back(0.2F);      // Name
            vColumnWidths.push_back(0.2F);      // Mem size
            vColumnWidths.push_back(-1.0F);     // # of vertices
            break;
        }

        case AF_TREE_ITEM_GL_SYNC_OBJECT:
        case AF_TREE_ITEM_GL_SYNC_OBJECTS_NODE:
        {
            // Set the "Memory Details View" columns widths for sync objects:
            vColumnWidths.push_back(0.2F);      // Handle
            vColumnWidths.push_back(0.2F);      // Mem size
            break;
        }

        case AF_TREE_ITEM_GL_FBO_NODE:
        {
            // Set the "Memory Details View" columns widths for sync objects:
            vColumnWidths.push_back(0.2F);      // Name
            vColumnWidths.push_back(-1.0F);     // Mem Size
            break;
        }

        case AF_TREE_ITEM_GL_FBO:
        case AF_TREE_ITEM_GL_FBO_ATTACHMENT:
        {
            // Set the "Memory Details View" columns widths for FBO attachments:
            vColumnWidths.push_back(0.2F);      // Name
            vColumnWidths.push_back(0.2F);      // Attachment target
            vColumnWidths.push_back(0.2F);      // Attachment point
            vColumnWidths.push_back(-1.0F);     // Mem Size
            break;
        }

        case AF_TREE_ITEM_GL_RENDER_CONTEXT:
        case AF_TREE_ITEM_CL_CONTEXT:
        case AF_TREE_ITEM_APP_ROOT:
        {
            // Set the "Memory Details View" columns widths for context objects:
            vColumnWidths.push_back(0.2F);      // Type
            vColumnWidths.push_back(0.3F);      // Mem Size
            vColumnWidths.push_back(-1.0F);     // Number of Objects
            break;
        }

        case AF_TREE_ITEM_GL_PBUFFERS_NODE:
        {
            // Set the "Memory Details View" columns widths for PBuffers:
            vColumnWidths.push_back(0.2F);      // Name
            vColumnWidths.push_back(0.15F);     // Mem Size
            vColumnWidths.push_back(0.15F);     // Dimensions
            vColumnWidths.push_back(-1.0F);     // Render Context
        }
        break;

        case AF_TREE_ITEM_GL_PBUFFER_NODE:
        case AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER:
        {
            // Set the "Memory Details View" columns widths for PBuffers:
            vColumnWidths.push_back(0.2F);      // Name
            vColumnWidths.push_back(0.15F);     // Mem Size
            vColumnWidths.push_back(0.15F);     // Dimensions
            vColumnWidths.push_back(0.15F);     // Type
            vColumnWidths.push_back(-1.0F);     // Render Context
            break;
        }

        case AF_TREE_ITEM_ITEM_NONE:
        {
            // This item "type" is used to symbolize no items (just a single column with the view title)
            vColumnWidths.push_back(-1.0F);     // Viewer name (One column)
            break;
        }

        case AF_TREE_ITEM_CL_IMAGES_NODE:
        case AF_TREE_ITEM_CL_IMAGE:
        {
            // Set the "Memory Details View" columns widths for textures:
            vColumnWidths.push_back(0.12F);     // Name
            vColumnWidths.push_back(0.145F);    // Mem Size
            vColumnWidths.push_back(0.125F);    // Tex Type
            vColumnWidths.push_back(0.175F);    // Dimensions
            vColumnWidths.push_back(0.2F);      // Pixel Format (0.2)
            vColumnWidths.push_back(0.1F);      // Data type
            vColumnWidths.push_back(0.1F);      // Flags
            vColumnWidths.push_back(-1.0F);     // Reference count
            break;
        }

        case AF_TREE_ITEM_CL_BUFFER:
        case AF_TREE_ITEM_CL_SUB_BUFFER:
        case AF_TREE_ITEM_CL_BUFFERS_NODE:
        {
            vColumnWidths.push_back(0.2F);      // Name
            vColumnWidths.push_back(0.2F);      // Mem size
            vColumnWidths.push_back(0.2F);      // Flags
            vColumnWidths.push_back(0.1F);      // Reference count
            break;
        }

        case AF_TREE_ITEM_CL_PIPE:
        case AF_TREE_ITEM_CL_PIPES_NODE:
        {
            vColumnWidths.push_back(0.2F);      // Name
            vColumnWidths.push_back(0.2F);      // Mem size
            vColumnWidths.push_back(0.2F);      // Packet size
            vColumnWidths.push_back(0.2F);      // Max packets
            vColumnWidths.push_back(0.2F);      // Flags
            vColumnWidths.push_back(0.1F);      // Reference count
            break;
        }

        case AF_TREE_ITEM_CL_COMMAND_QUEUE:
        case AF_TREE_ITEM_CL_COMMAND_QUEUES_NODE:
        {
            vColumnWidths.push_back(0.2F);      // Name
            vColumnWidths.push_back(0.2F);      // Mem size
            vColumnWidths.push_back(0.1F);      // Reference count
#ifdef GD_MEMORY_ANALYSIS_DETAILS_DISPLAY_CL_QUEUE_EVENTS
            vColumnWidths.push_back(0.1F);      // Num of events
#endif
            vColumnWidths.push_back(0.1F);      // Out of order execution mode
            vColumnWidths.push_back(0.1F);      // Profiling mode
            vColumnWidths.push_back(0.1F);      // On-device
            break;
        }

        case AF_TREE_ITEM_CL_PROGRAM:
        case AF_TREE_ITEM_CL_KERNEL:
        case AF_TREE_ITEM_CL_PROGRAMS_NODE:
        {
            // Set the "Memory Details View" columns widths for CL programs:
            vColumnWidths.push_back(0.2F);      // Name
            vColumnWidths.push_back(0.1F);      // Mem Size
            vColumnWidths.push_back(-1.0F);     // Reference count
            break;
        }

        case AF_TREE_ITEM_CL_SAMPLER:
        case AF_TREE_ITEM_CL_SAMPLERS_NODE:
        {
            // Set the "Memory Details View" columns widths for CL samplers:
            vColumnWidths.push_back(0.2F);      // Name
            vColumnWidths.push_back(0.15F);     // Mem Size
            vColumnWidths.push_back(0.125F);    // Filter Mode
            vColumnWidths.push_back(0.175F);    // Addressing Mode
            break;
        }

        case AF_TREE_ITEM_CL_EVENT:
        case AF_TREE_ITEM_CL_EVENTS_NODE:
        {
            // Set the "Memory Details View" columns widths for CL events:
            vColumnWidths.push_back(0.2F);      // Name
            vColumnWidths.push_back(-1.0F);     // Mem Size
            break;
        }

        default:
        {
            GT_ASSERT_EX(false, L"Unknown memory object type");
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::clearAndDisplayMessage
// Description: Clears the view and displays a "not available message:
// Author:      Uri Shomroni
// Date:        10/11/2008
// ---------------------------------------------------------------------------
void gdMemoryAnalysisDetailsView::clearAndDisplayMessage()
{
    // Clear the view:
    clearList();

    // Reset the _currentDisplayedListID member:
    m_currentDisplayedItem.m_itemType = AF_TREE_ITEM_ITEM_NONE;

    gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(m_currentDisplayedItem.extendedItemData());

    if (pGDData != NULL)
    {
        pGDData->_contextId._contextId = 0;
        pGDData->_contextId._contextType = AP_OPENGL_CONTEXT;
        pGDData->_objectOpenGLName = 0;
    }

    // Display only one column, with the view's title:
    setListCtrlColumns();

    // Display the "memory information is only available ..." message:
    if (afExecutionModeManager::instance().isActiveMode(GD_STR_executionMode))
    {
        addRow(GD_STR_MemoryAnalysisViewerListUnavailableSuspended);
    }
    else
    {
        addRow(GD_STR_MemoryAnalysisViewerListUnavailableDebugMode);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::updateListItems
// Description: Update the memory items list according to the currently selected tree item
// Arguments: QTreeWidgetItem* pselectedTreeItemId - the selected tree item id
//            bool rebuildData - should the items be rebuilt, even if the last selected item
//            is equal to the current one.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/9/2008
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::updateListItems(QTreeWidgetItem* pselectedTreeItemId, bool rebuildData)
{
    bool retVal = false;

    // Don't cause selection events:
    blockSignals(true);

    GT_IF_WITH_ASSERT(pselectedTreeItemId != NULL)
    {
        GT_IF_WITH_ASSERT(_pMemoryTree != NULL)
        {
            // Get the memory object item data:
            gdDebugApplicationTreeData* pGDItemData = NULL;
            afApplicationTreeItemData* pCurrentlySelectedItemData = NULL;
            _pMemoryTree->getTreeItemDatas(pselectedTreeItemId, pCurrentlySelectedItemData, pGDItemData);

            bool isValid = (pCurrentlySelectedItemData != NULL) && (pGDItemData != NULL);

            // If the item data exists, update the currently items:
            if (isValid)
            {
                bool isAlreadyDisplayed = false;

                if (rebuildData)
                {
                    // Update the list of objects:
                    retVal = displayObjectsList(pCurrentlySelectedItemData, isAlreadyDisplayed);
                }

                // If the item is displayed already, only select the requested list item:
                if (isAlreadyDisplayed)
                {
                    // Unselect all items in list view:
                    clearSelection();

                    gdDebugApplicationTreeData* pDebugItemData = qobject_cast<gdDebugApplicationTreeData*>(pCurrentlySelectedItemData->extendedItemData());
                    GT_IF_WITH_ASSERT(pDebugItemData != NULL)
                    {
                        // If we do not rebuild the list, select the right item in the list:
                        int listViewIndex = pDebugItemData->_listViewIndex;

                        // Calling SetItemState with the index = -1 causes the effect to be applied to all items in the list
                        // so, if we don't have a selection (we chose the root), we should not do this:
                        afTreeItemType parentType;
                        bool rcGetParentType = itemDataToParentType(pCurrentlySelectedItemData, parentType);
                        GT_ASSERT(rcGetParentType);

                        if ((listViewIndex >= 0) && (pCurrentlySelectedItemData->m_itemType != parentType))
                        {
                            ensureRowVisible(listViewIndex, true);
                        }
                    }

                    retVal = true;
                }
            }
        }
    }

    blockSignals(false);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::findDisplayedListIDByItemID
// Description: Find the id of the item that should be displayed in the list,
//              according to the requested item id
// Arguments:   const gdDebugApplicationTreeData* pObjectItemID
//              gdDebugApplicationTreeData& itemListId
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/2/2011
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::findDisplayedListIDByItemID(const afApplicationTreeItemData* pObjectItemID, afApplicationTreeItemData& itemListId)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((pObjectItemID != NULL) && (_pMemoryTree != NULL))
    {
        // Get the item type:
        afTreeItemType itemType = pObjectItemID->m_itemType;

        // Check if the item is a root item:
        bool isRoot = afApplicationTreeItemData::isItemTypeRoot(itemType);

        // Check if the item is one of the items that are both parents and children:
        isRoot = isRoot || (itemType == AF_TREE_ITEM_CL_PROGRAM);
        isRoot = isRoot || (itemType == AF_TREE_ITEM_GL_FBO);
        isRoot = isRoot || (itemType == AF_TREE_ITEM_GL_RENDER_CONTEXT) || (itemType == AF_TREE_ITEM_CL_CONTEXT);

        if (itemType == AF_TREE_ITEM_CL_BUFFER)
        {
            // CL Buffer could have children - sub buffers:
            int amountOfChildren = _pMemoryTree->getTreeChildrenCount(pObjectItemID->m_pTreeWidgetItem, false);
            isRoot = (amountOfChildren > 0);
        }

        if (isRoot)
        {
            // Just display the same object:
            itemListId = *pObjectItemID;
            retVal = true;
        }
        else
        {
            // Get the parent:
            QTreeWidgetItem* pparentItemID = _pMemoryTree->getTreeItemParent(pObjectItemID->m_pTreeWidgetItem);

            if (pparentItemID != NULL)
            {
                // Get the item data for the parent:
                afApplicationTreeItemData* pParentItemData = _pMemoryTree->getTreeItemData(pparentItemID);
                GT_IF_WITH_ASSERT(pParentItemData != NULL)
                {
                    // Copy the id:
                    pParentItemData->copyID(itemListId);
                    retVal = true;
                }
            }
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::displayObjectsList
// Description: Display a list for the requested object
// Arguments:   const gdDebugApplicationTreeData* pObjectItemData
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/2/2011
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::displayObjectsList(afApplicationTreeItemData* pObjectItemData, bool& isAlreadyDisplayed)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pObjectItemData != NULL)
    {
        // Get the item tree id:
        QTreeWidgetItem* pselectedTreeItemId = pObjectItemData->m_pTreeWidgetItem;

        // Given the requested item, find the item that should be displayed in the detailed list:
        afApplicationTreeItemData itemListID(false);
        bool rcGetItemListID = findDisplayedListIDByItemID(pObjectItemData, itemListID);
        GT_IF_WITH_ASSERT(rcGetItemListID)
        {
            // Check if the currently displayed list has the same item id:
            isAlreadyDisplayed = (pObjectItemData->m_itemType == m_currentDisplayedItem.m_itemType);

            if (isAlreadyDisplayed)
            {
                isAlreadyDisplayed = m_currentDisplayedItem.isSameObject(pObjectItemData);

                if (isAlreadyDisplayed)
                {
                    retVal = true;
                }
            }

            if (!retVal)
            {
                // Clear the list:
                clearList();

                // Copy the new displayed list id to the displayed one:
                m_currentDisplayedItem = *pObjectItemData;

                // Rebuild the list control columns according to the new displayed item:
                setListCtrlColumns();

                // Check if the item is a leaf:
                bool isLeaf = !_pMemoryTree->treeItemHasChildren(pselectedTreeItemId);

                // Get the item data for the requested list in the tree:
                afApplicationTreeItemData* pDisplayedListItemData = _pMemoryTree->FindMatchingTreeItem(itemListID);
                GT_IF_WITH_ASSERT(pDisplayedListItemData != NULL)
                {
                    // Get the list item tree id:
                    QTreeWidgetItem* plistTreeItemId = pDisplayedListItemData->m_pTreeWidgetItem;
                    GT_IF_WITH_ASSERT(plistTreeItemId != NULL)
                    {
                        retVal = true;

                        // Display a progress dialog for showing the list items:
                        int numberOfChildren = _pMemoryTree->getTreeChildrenCount(plistTreeItemId, false);

                        // Show the progress bar:
                        if (_pOwnerProgressBar != NULL)
                        {
                            _pOwnerProgressBar->setProgressText(GD_STR_MemoryAnalysisViewerPopulatingList);
                            _pOwnerProgressBar->setProgressRange(numberOfChildren);
                        }

                        gtUInt32 totalSize = 0;
                        int loadedItems = 0;

                        // Iterate the children, and add each of them to the list:
                        for (int i = 0 ; i < plistTreeItemId->childCount(); i++)
                        {
                            // Get the current child:
                            QTreeWidgetItem* pcurrentChildItemId = plistTreeItemId->child(i);

                            if (pcurrentChildItemId != NULL)
                            {
                                // Check if the item should be selected:
                                bool shouldSelectItem = (isLeaf && pcurrentChildItemId == pObjectItemData->m_pTreeWidgetItem);

                                // Add the current item to the list:
                                bool rcAddItem = addItemToList(pcurrentChildItemId, shouldSelectItem, totalSize);
                                GT_ASSERT(rcAddItem);

                                retVal = retVal && rcAddItem;

                                if (_pOwnerProgressBar != NULL)
                                {
                                    _pOwnerProgressBar->updateProgressBar(loadedItems++);
                                }
                            }
                        }

                        // Sort the list items:
                        _sortInfo._sortOrder = Qt::AscendingOrder;
                        _sortInfo._sortType = GD_SORT_BY_NAME;

                        sortItems(0, Qt::AscendingOrder);

                        // Only add the total item for objects we know the size of:
                        if (doesItemTypeHaveTotalLine(itemListID.m_itemType))
                        {
                            bool isInBeginEndBlock = false;
                            gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pObjectItemData->extendedItemData());
                            GT_IF_WITH_ASSERT(pGDItemData != NULL)
                            {
                                if (pGDItemData->_contextId._contextType == AP_OPENGL_CONTEXT)
                                {
                                    // Check if the requested item context is in glBegin - glEnd block:
                                    isInBeginEndBlock = gaIsInOpenGLBeginEndBlock(pGDItemData->_contextId._contextId);
                                }
                            }

                            // Add total item:
                            addTotalItem(totalSize, isInBeginEndBlock, pDisplayedListItemData->m_objectCount);
                        }
                    }

                    // Hide the progress bar:
                    if (_pOwnerProgressBar != NULL)
                    {
                        _pOwnerProgressBar->hideProgressBar();
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addItemToList
// Description: Add the requested item to the list, select it if requested, and
//              set its object size
// Arguments:   QTreeWidgetItem* pitemToAdd
//              bool shouldSelectItem
//              int& objectSize
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/2/2011
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addItemToList(QTreeWidgetItem* pitemToAdd, bool shouldSelectItem, gtUInt32& totalSize)
{
    bool retVal = false;

    // Add the current child to the list:
    gtUInt32 objectSize = 0;

    // Get the item data for the requested item:
    gdDebugApplicationTreeData* pGDItemData = NULL;
    afApplicationTreeItemData* pItemData = NULL;
    _pMemoryTree->getTreeItemDatas(pitemToAdd, pItemData, pGDItemData);
    GT_IF_WITH_ASSERT((pItemData != NULL) && (pGDItemData != NULL))
    {
        switch (pItemData->m_itemType)
        {
            case AF_TREE_ITEM_GL_RENDER_CONTEXT:
            case AF_TREE_ITEM_CL_CONTEXT:
            {
                // Add render context:
                retVal = addContextItem(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_GL_TEXTURES_NODE:
            case AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE:
            case AF_TREE_ITEM_GL_VBO_NODE:
            case AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE:
            case AF_TREE_ITEM_GL_SHADERS_NODE:
            case AF_TREE_ITEM_GL_PROGRAMS_NODE:
            case AF_TREE_ITEM_GL_DISPLAY_LISTS_NODE:
            case AF_TREE_ITEM_GL_SAMPLERS_NODE:
            case AF_TREE_ITEM_GL_PROGRAM_PIPELINES_NODE:
            case AF_TREE_ITEM_GL_FBO_NODE:
            {
                retVal = addGLContextChildObjectToList(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_CL_IMAGES_NODE:
            case AF_TREE_ITEM_CL_BUFFERS_NODE:
            case AF_TREE_ITEM_CL_PIPES_NODE:
            case AF_TREE_ITEM_CL_COMMAND_QUEUES_NODE:
            case AF_TREE_ITEM_CL_PROGRAMS_NODE:
            case AF_TREE_ITEM_CL_SAMPLERS_NODE:
            case AF_TREE_ITEM_CL_EVENTS_NODE:
            {
                retVal = addCLContextChildObjectToList(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_GL_TEXTURE:
            {
                bool isInBeginEndBlock = false;

                if (pGDItemData->_contextId._contextType == AP_OPENGL_CONTEXT)
                {
                    // Check if the requested item context is in glBegin - glEnd block:
                    isInBeginEndBlock = gaIsInOpenGLBeginEndBlock(pGDItemData->_contextId._contextId);
                }

                retVal = addGLTextureItem(pitemToAdd, shouldSelectItem, objectSize, isInBeginEndBlock);
                break;
            }

            case AF_TREE_ITEM_GL_RENDER_BUFFER:
            {
                retVal = addRenderBufferItem(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_GL_VBO:
            {
                retVal = addVBOItem(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_GL_FBO:
            {
                retVal = addFBOItem(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_GL_FBO_ATTACHMENT:
            {
                retVal = addFBOAttchmentItem(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_GL_STATIC_BUFFER:
            {
                retVal = addStaticBufferItem(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_GL_PROGRAM:
            {
                retVal = addProgramItem(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_GL_PROGRAM_PIPELINE:
            {
                retVal = addProgramPipelineItem(pitemToAdd, shouldSelectItem);
                break;
            }

            case AF_TREE_ITEM_GL_SAMPLER:
            {
                retVal = addGlSamplerItem(pitemToAdd, shouldSelectItem);
                break;
            }

            case AF_TREE_ITEM_GL_VERTEX_SHADER:
            case AF_TREE_ITEM_GL_TESSELLATION_CONTROL_SHADER:
            case AF_TREE_ITEM_GL_TESSELLATION_EVALUATION_SHADER:
            case AF_TREE_ITEM_GL_GEOMETRY_SHADER:
            case AF_TREE_ITEM_GL_FRAGMENT_SHADER:
            case AF_TREE_ITEM_GL_COMPUTE_SHADER:
            case AF_TREE_ITEM_GL_UNSUPPORTED_SHADER:
            {
                retVal = addShaderItem(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_GL_DISPLAY_LIST:
            {
                retVal = addDisplayListItem(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_GL_PBUFFERS_NODE:
            case AF_TREE_ITEM_GL_PBUFFER_NODE:
            {
                retVal = addPBufferItem(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER:
            {
                retVal = addPBufferStaticBufferItem(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_GL_SYNC_OBJECT:
            case AF_TREE_ITEM_GL_SYNC_OBJECTS_NODE:
            {
                retVal = addSyncObjectItem(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_CL_IMAGE:
            {
                retVal = addCLImageItem(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_CL_BUFFER:
            case AF_TREE_ITEM_CL_SUB_BUFFER:
            {
                afTreeItemType parentType;
                bool rc = itemDataToParentType(pItemData, parentType);
                GT_ASSERT(rc);

                if (parentType == AF_TREE_ITEM_CL_BUFFERS_NODE)
                {
                    retVal = addCLBufferItem(pitemToAdd, shouldSelectItem, objectSize);
                }
                else
                {
                    retVal = addCLSubBufferItem(pitemToAdd, shouldSelectItem, objectSize);
                }

                break;
            }

            case AF_TREE_ITEM_CL_PIPE:
            {
                retVal = addCLPipeItem(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_CL_COMMAND_QUEUE:
            {
                retVal = addCLCommandQueueItem(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_CL_PROGRAM:
            {
                retVal = addCLProgramItem(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_CL_KERNEL:
            {
                retVal = addCLKernelItem(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_CL_SAMPLER:
            {
                retVal = addCLSamplerItem(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_CL_EVENT:
            {
                retVal = addCLEventItem(pitemToAdd, shouldSelectItem, objectSize);
                break;
            }

            case AF_TREE_ITEM_ITEM_NONE:
            {
                // This is not a real object type, so something is wrong
                GT_ASSERT(false);
                break;
            }

            default:
            {
                GT_ASSERT(false);
                break;
            }
        }
    }

    // Add the current item size to the total size:
    totalSize += objectSize;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::setSelectionFromPreviousItem
// Description: Restore the selection of a previously selected item. Note that
//              this function should only be called when the memory viewer is
//              ignoring selections, otherwise it will choose a tree item and
//              cause the viewer to refresh and change the selected item.
//              This is only for the case when the selected item is not a leaf
//              in the tree.
// Author:      Uri Shomroni
// Date:        18/11/2008
// ---------------------------------------------------------------------------
void gdMemoryAnalysisDetailsView::setSelectionFromPreviousItem(const afApplicationTreeItemData& previouslySelectedItem)
{
    afTreeItemType listItemsType = previouslySelectedItem.m_itemType;
    gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(previouslySelectedItem.extendedItemData());
    GT_IF_WITH_ASSERT(pGDItemData != NULL)
    {
        // Make sure we have the right list:
        if (listItemsType == m_currentDisplayedItem.m_itemType)
        {
            int numberOfItems = rowCount();
            int itemToSelect = -1;

            if (listItemsType == AF_TREE_ITEM_APP_ROOT)
            {
                // Find the requested Context:
                apContextID renderContextID = pGDItemData->_contextId;

                // Iterate the list items looking for it:
                for (long i = 0; i < numberOfItems; i++)
                {
                    afApplicationTreeItemData* pCurrentItemData = (afApplicationTreeItemData*)getItemData(i);

                    if (pCurrentItemData != NULL)
                    {
                        gdDebugApplicationTreeData* pDebugItemData = qobject_cast<gdDebugApplicationTreeData*>(previouslySelectedItem.extendedItemData());
                        GT_IF_WITH_ASSERT(pDebugItemData != NULL)
                        {
                            if (pDebugItemData->_contextId == renderContextID)
                            {
                                itemToSelect = i;
                                break;
                            }
                        }
                    }
                }
            }
            else if (listItemsType == AF_TREE_ITEM_GL_PBUFFERS_NODE)
            {
                // Find the requested PBuffer:
                GLuint pbufferName = pGDItemData->_objectOpenGLName;

                // Iterate the list items looking for it:
                for (long i = 0; i < numberOfItems; i++)
                {
                    afApplicationTreeItemData* pCurrentItemData = (afApplicationTreeItemData*)getItemData(i);

                    if (pCurrentItemData != NULL)
                    {
                        gdDebugApplicationTreeData* pDebugItemData = qobject_cast<gdDebugApplicationTreeData*>(previouslySelectedItem.extendedItemData());
                        GT_IF_WITH_ASSERT(pDebugItemData != NULL)
                        {
                            if (pDebugItemData->_objectOpenGLName == pbufferName)
                            {
                                itemToSelect = i;
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                // Do nothing...
            }

            if ((itemToSelect >= 0) && (itemToSelect < numberOfItems))
            {
                ensureRowVisible(itemToSelect, true);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::doesItemTypeHaveTotalLine
// Description: Checks if a given item type is one that displays a total item or not
// Return Val: true = this list item type has a total line
//             false = this list item type has no total line
// Author:      Uri Shomroni
// Date:        19/11/2008
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::doesItemTypeHaveTotalLine(const afTreeItemType& listItemsType)
{
    bool retVal = ((listItemsType != AF_TREE_ITEM_GL_FBO_NODE) && (listItemsType != AF_TREE_ITEM_GL_PROGRAMS_NODE) &&
                   (listItemsType != AF_TREE_ITEM_GL_PROGRAM_PIPELINES_NODE) && (listItemsType != AF_TREE_ITEM_GL_SAMPLERS_NODE) &&
                   (listItemsType != AF_TREE_ITEM_GL_PBUFFERS_NODE));

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addContextItem
// Description: Add a render context item to the list
// Arguments: QTreeWidgetItem* pselectedTreeItemId
//            bool shouldSelectItem
//            gtUInt32& renderContextSize
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/10/2008
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addContextItem(QTreeWidgetItem* pcontextTreeItemId, bool shouldSelectItem, gtUInt32& contextSize)
{
    bool retVal = false;

    // Get the context item data:
    // Get the memory object item data:
    gdDebugApplicationTreeData* pGDItemData = NULL;
    afApplicationTreeItemData* pItemData = NULL;
    _pMemoryTree->getTreeItemDatas(pcontextTreeItemId, pItemData, pGDItemData);
    GT_IF_WITH_ASSERT((pItemData != NULL) && (pGDItemData != NULL))
    {
        gdDebugApplicationTreeData* pDebugItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pDebugItemData != NULL)
        {
            // Add the current context to the list:
            gtString renderContextName, objectSizeString, objectCountString, refCountString;
            gdGetContextNameString(pDebugItemData->_contextId, renderContextName);

            calculateTreeItemSize(pcontextTreeItemId, contextSize);

            // Build the object size's string:
            sizeStringFromNumber(objectSizeString, contextSize);

            // Build the object count string:
            objectCountString.appendFormattedString(L"%u", pItemData->m_objectCount);
            objectCountString.addThousandSeperators();

            // Build the reference count string:
            refCountString.appendFormattedString(L"%d", pDebugItemData->_referenceCount);

            // Build string list for this item:
            QStringList itemStrings;
            itemStrings << acGTStringToQString(renderContextName);
            itemStrings << acGTStringToQString(objectSizeString);
            itemStrings << acGTStringToQString(objectCountString);

            // Add row for this item:
            retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

            // Set the item list view index:
            pDebugItemData->_listViewIndex = rowCount() - 1;

            if (shouldSelectItem)
            {
                ensureRowVisible(pDebugItemData->_listViewIndex, true);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addGLTextureItem
// Description: Add a texture item to the list (the texture item is the one associated with
//              the input tree item id)
// Arguments: QTreeWidgetItem* ptextureItemTreeItem
//            bool shouldSelectItem - should the item be selected
//            gtUInt32& textureObjectSize - output - the texture object's size
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/9/2008
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addGLTextureItem(QTreeWidgetItem* ptextureItemTreeItem, bool shouldSelectItem, gtUInt32& textureObjectSize, bool isInBeginEndBlock)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pMemoryTree != NULL)
    {
        // Get the memory object item data:
        gdDebugApplicationTreeData* pGDItemData = NULL;
        afApplicationTreeItemData* pItemData = NULL;
        _pMemoryTree->getTreeItemDatas(ptextureItemTreeItem, pItemData, pGDItemData);
        GT_IF_WITH_ASSERT((pItemData != NULL) && (pGDItemData != NULL))
        {
            gtString textureName, textureType, textureSize, textureDimensions, textureMipmap, textureFormat;

            // Get the
            // Build the texture name:
            gdHTMLProperties htmlBuilder;
            apGLTextureMipLevelID textureID;
            textureID._textureName = pGDItemData->_objectOpenGLName;
            textureID._textureMipLevel = 0;
            htmlBuilder.getGLTextureName(textureID, pGDItemData->_objectOpenCLName, pGDItemData->_objectCLSpyID, textureName);

            // Set the texture's memory size:
            textureObjectSize = pItemData->m_objectMemorySize;

            // Size if not available in glBegin-glEnd block:
            if (isInBeginEndBlock)
            {
                textureSize = GD_STR_MemoryAnalysisViewerNAGLBeginEndMessage;
                textureDimensions = GD_STR_MemoryAnalysisViewerNAGLBeginEndMessage;
            }
            else
            {
                // Build the texture size's string (for compressed texture's format the size is not available):
                sizeStringFromNumber(textureSize, textureObjectSize);

                // Generate getDimensions string:
                GLsizei width = pGDItemData->_objectWidth;
                GLsizei height = pGDItemData->_objectHeight;
                GLsizei depth = pGDItemData->_objectDepth;
                textureDimensions = AF_STR_NotAvailable;

                if ((pGDItemData->_textureType == AP_1D_TEXTURE) || (pGDItemData->_textureType == AP_BUFFER_TEXTURE))
                {
                    if (width >= 0)
                    {
                        textureDimensions.makeEmpty();
                        textureDimensions.appendFormattedString(L"%d", width);
                    }
                }
                else if ((pGDItemData->_textureType == AP_2D_TEXTURE) || (pGDItemData->_textureType == AP_1D_ARRAY_TEXTURE) || (pGDItemData->_textureType == AP_CUBE_MAP_TEXTURE) || (pGDItemData->_textureType == AP_TEXTURE_RECTANGLE))
                {
                    if ((width >= 0) && (height >= 0))
                    {
                        textureDimensions.makeEmpty();
                        textureDimensions.appendFormattedString(L"%dx%d", width, height);
                    }
                }
                else if ((pGDItemData->_textureType == AP_3D_TEXTURE) || (pGDItemData->_textureType == AP_2D_ARRAY_TEXTURE) || (pGDItemData->_textureType == AP_CUBE_MAP_ARRAY_TEXTURE))
                {
                    if ((width >= 0) && (height >= 0) && (depth >= 0))
                    {
                        textureDimensions.makeEmpty();
                        textureDimensions.appendFormattedString(L"%dx%dx%d", width, height, depth);
                    }
                }
            }

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            // Get the new project file name:
            gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();

            // Get the CodeXL project type:
            apProjectExecutionTarget CodeXLProjectExecutionTarget = globalVarsManager.CodeXLProjectExecutionTarget();
            bool isiPhoneProject = (CodeXLProjectExecutionTarget == AP_IPHONE_SIMULATOR_EXECUTION_TARGET) || (CodeXLProjectExecutionTarget == AP_IPHONE_DEVICE_EXECUTION_TARGET);

            // In iPhone we do not have parameters, so we do not write the 'requested' string:
            if (!isiPhoneProject)
            {
                // If the memory size is estimated,
                if (pItemData->_isMemorySizeEstimated)
                {
                    textureSize += GD_STR_MemoryAnalysisViewerSizeEstimated;
                }
            }

#endif

            // Get the texture type as string:
            apTextureTypeAsString(pGDItemData->_textureType, textureType);

            // Get the texture mipmap type:
            textureMipmap = pGDItemData->_mipmapStr;

            // Get texture internal format as string:
            GLenum internalFormat = pGDItemData->_internalFormat;
            bool isHardwareFormat = true;

            // If we can't get the used format, display the reuqested format:
            if (internalFormat < 1)
            {
                internalFormat = pGDItemData->_requestedInternalFormat;
                isHardwareFormat = false;
            }

            textureFormat.append(apInternalFormatValueAsString(internalFormat));

            // dummy if for gcc warning for default linux variant and windows
            (void)(isHardwareFormat);

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

            // In iPhone we do not have parameters, so we do not write the 'requested' string:
            if (!isiPhoneProject)
            {
                // If this is not the format from the hardware:
                if (!isHardwareFormat)
                {
                    textureFormat.append(GD_STR_MemoryAnalysisViewerInternalFormatRequested);
                }
            }

#endif

            // Build string list for this item:
            QStringList itemStrings;
            itemStrings << acGTStringToQString(textureName);
            itemStrings << acGTStringToQString(textureSize);
            itemStrings << acGTStringToQString(textureType);
            itemStrings << acGTStringToQString(textureDimensions);
            itemStrings << acGTStringToQString(textureMipmap);
            itemStrings << acGTStringToQString(textureFormat);

            // Add row for this item:
            retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

            // Set the item list view index:
            pGDItemData->_listViewIndex = rowCount() - 1;

            if (shouldSelectItem)
            {
                ensureRowVisible(pGDItemData->_listViewIndex, true);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addStaticBufferItem
// Description: Add a static buffer item to the list (the buffer item is the one associated with
//              the input tree item id)
// Arguments: QTreeWidgetItem* pbufferItemTreeItem
//            bool shouldSelectItem - should the item be selected
//            gtUInt32& staticBufferSize - output - the render buffer's size
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/9/2008
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addStaticBufferItem(QTreeWidgetItem* pbufferItemTreeItem, bool shouldSelectItem, gtUInt32& staticBufferSize)
{
    bool retVal = false;

    // Get the memory object item data:
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)_pMemoryTree->getTreeItemData(pbufferItemTreeItem);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gtString bufferName, bufferSize, bufferDimensions, bufferPixelFormatType, bufferPixelFormatFormat;

        gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDItemData != NULL)
        {
            // Get the buffer name from the buffer display type:
            apDisplayBuffer bufferType = pGDItemData->_bufferType;
            apGetBufferName(bufferType, bufferName);

            // Set the size parameter:
            staticBufferSize = pItemData->m_objectMemorySize;

            // Build the buffer's string (for compressed buffer's format the size is not available):
            sizeStringFromNumber(bufferSize, staticBufferSize);

            GLint width = pGDItemData->_objectWidth;
            GLint height = pGDItemData->_objectHeight;

            if ((pGDItemData->_objectWidth >= 0) && (pGDItemData->_objectHeight >= 0))
            {
                bufferDimensions.appendFormattedString(L"%dx%d", width, height);
            }
            else
            {
                bufferDimensions = AF_STR_NotAvailable;
            }


            // Get render buffer pixel format and data type:
            oaDataType dataType = pGDItemData->_dataType;
            oaTexelDataFormat dataFormat = pGDItemData->_dataFormat;

            // Get Pixel format as string:
            GLenum bufferDataFormatEnum = oaTexelDataFormatToGLEnum(dataFormat);
            apGLenumParameter bufferDataFormatParameter(bufferDataFormatEnum);
            bufferDataFormatParameter.valueAsString(bufferPixelFormatFormat);

            // Get data type as string:
            GLenum bufferDataTypeEnum = oaDataTypeToGLEnum(dataType);
            apGLenumParameter bufferDataTypeParameter(bufferDataTypeEnum);
            bufferDataTypeParameter.valueAsString(bufferPixelFormatType);

            // Build string list for this item:
            QStringList itemStrings;
            itemStrings << acGTStringToQString(bufferName);
            itemStrings << acGTStringToQString(bufferSize);
            itemStrings << acGTStringToQString(bufferDimensions);
            itemStrings << acGTStringToQString(bufferPixelFormatFormat);
            itemStrings << acGTStringToQString(bufferPixelFormatType);

            // Add row for this item:
            retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

            // Set the item list view index:
            pGDItemData->_listViewIndex = rowCount() - 1;

            if (shouldSelectItem)
            {
                ensureRowVisible(pGDItemData->_listViewIndex, true);
            }

            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addProgramItem
// Description: Adds a program item into the list
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        16/11/2008
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addProgramItem(QTreeWidgetItem* pprogramItemTreeItem, bool shouldSelectItem, gtUInt32& programSize)
{
    bool retVal = false;

    // Get the memory object item data:
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)_pMemoryTree->getTreeItemData(pprogramItemTreeItem);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDItemData != NULL)
        {
            gtString programName;
            programName.appendFormattedString(GD_STR_PropertiesProgramNameFormat, pGDItemData->_objectOpenGLName);

            gtString programSizeString;

            // Set the size parameter:
            programSize = pItemData->m_objectMemorySize;

            // We should use this instead, if we want to display the program size:
            // sizeStringFromNumber(programSizeString, programSize);

            // Build string list for this item:
            QStringList itemStrings;
            itemStrings << acGTStringToQString(programName);
            itemStrings << GD_STR_MemoryAnalysisViewerObjectSizeInsignificantA;

            // Add row for this item:
            retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

            // Set the item list view index:
            pGDItemData->_listViewIndex = rowCount() - 1;

            if (shouldSelectItem)
            {
                ensureRowVisible(pGDItemData->_listViewIndex, true);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addProgramPipelineItem
// Description: Adds an OpenGL program pipeline item into the list
// Return Val:  bool  - Success / failure.
// Author:      Amit Ben-Moshe
// Date:        31/7/2014
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addProgramPipelineItem(QTreeWidgetItem* pprogramItemTreeItem, bool shouldSelectItem)
{
    bool retVal = false;

    // Get the memory object item data:
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)_pMemoryTree->getTreeItemData(pprogramItemTreeItem);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDItemData != NULL)
        {
            gtString programPipelineName;
            programPipelineName.appendFormattedString(GD_STR_PropertiesProgramPipelineNameFormat, pGDItemData->_objectOpenGLName);

            gtString programSizeString;

            // Build string list for this item:
            QStringList itemStrings;
            itemStrings << acGTStringToQString(programPipelineName);
            itemStrings << GD_STR_MemoryAnalysisViewerObjectSizeInsignificantA;

            // Add row for this item:
            retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

            // Set the item list view index:
            pGDItemData->_listViewIndex = rowCount() - 1;

            if (shouldSelectItem)
            {
                ensureRowVisible(pGDItemData->_listViewIndex, true);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addShaderItem
// Description: Adds a shader item into the list
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        16/11/2008
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addShaderItem(QTreeWidgetItem* pshaderItemTreeItem, bool shouldSelectItem, gtUInt32& shaderSize)
{
    bool retVal = false;

    // Get the memory object item data:
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)_pMemoryTree->getTreeItemData(pshaderItemTreeItem);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDItemData != NULL)
        {
            gtString shaderName;
            gdShaderType shaderType = pGDItemData->_shaderType;
            gdShaderNameStringFromNameAndType(pGDItemData->_objectOpenGLName, shaderType, shaderName);

            gtString shaderSizeString;

            // Set the size parameter:
            shaderSize = pItemData->m_objectMemorySize;

            // Build the shader's [source code] size string:
            sizeStringFromNumber(shaderSizeString, shaderSize);

            // Get the shader type:
            gtString shaderTypeString;
            int firstSpace = shaderName.find(' ');
            firstSpace = max(0, firstSpace);
            shaderName.getSubString(0, firstSpace - 1, shaderTypeString);

            // Build string list for this item:
            QStringList itemStrings;
            itemStrings << acGTStringToQString(shaderName);
            itemStrings << acGTStringToQString(shaderSizeString);
            itemStrings << acGTStringToQString(shaderTypeString);

            // Add row for this item:
            retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

            // Set the item list view index:
            pGDItemData->_listViewIndex = rowCount() - 1;

            if (shouldSelectItem)
            {
                ensureRowVisible(pGDItemData->_listViewIndex, true);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addDisplayListItem
// Description: Adds a display list item into the list
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        17/11/2008
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addDisplayListItem(QTreeWidgetItem* pdpyListItemTreeItem, bool shouldSelectItem, gtUInt32& dpyListSize)
{
    bool retVal = false;

    // Get the memory object item data:
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)_pMemoryTree->getTreeItemData(pdpyListItemTreeItem);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDItemData != NULL)
        {
            gtString dpyListName;
            dpyListName.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeDisplayListName, pGDItemData->_objectOpenGLName);

            gtString dpyListSizeString = GD_STR_MemoryAnalysisViewerObjectSizeNoEstimateAvilable;
            gtString amountOfVerticesString;

            // Get the size parameter:
            dpyListSize = pItemData->m_objectMemorySize;

            // Get amount of vertices:
            gtUInt32 amountOfVertices = pGDItemData->_amountOfRenderedVertices;

            // Build the display lists's size string:
            sizeStringFromNumber(dpyListSizeString, dpyListSize);
            amountOfVerticesString.appendFormattedString(L"%d", amountOfVertices);

            // Build string list for this item:
            QStringList itemStrings;
            itemStrings << acGTStringToQString(dpyListName);
            itemStrings << acGTStringToQString(dpyListSizeString);
            itemStrings << acGTStringToQString(amountOfVerticesString);

            // Add row for this item:
            retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

            // Set the item list view index:
            pGDItemData->_listViewIndex = rowCount() - 1;

            if (shouldSelectItem)
            {
                ensureRowVisible(pGDItemData->_listViewIndex, true);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addFBOItem
// Description: Adds an FBO item into the list
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        17/11/2008
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addFBOItem(QTreeWidgetItem* pfboItemTreeItem, bool shouldSelectItem, gtUInt32& fboSize)
{
    bool retVal = false;

    // Get the memory object item data:
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)_pMemoryTree->getTreeItemData(pfboItemTreeItem);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDItemData != NULL)
        {
            gtString fboName;
            fboName.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeFBOName, pGDItemData->_objectOpenGLName);


            // Set the size parameter:
            fboSize = pItemData->m_objectMemorySize;

            // Build string list for this item:
            QStringList itemStrings;
            itemStrings << acGTStringToQString(fboName);
            itemStrings << GD_STR_MemoryAnalysisViewerObjectSizeInsignificantA;

            // Add row for this item:
            retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

            // Set the item list view index:
            pGDItemData->_listViewIndex = rowCount() - 1;

            if (shouldSelectItem)
            {
                ensureRowVisible(pGDItemData->_listViewIndex, true);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addFBOAttchmentItem
// Description: Adds an FBO attachment item into the list
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        14/2/2011
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addFBOAttchmentItem(QTreeWidgetItem* pfboAttachmentItemTreeItem, bool shouldSelectItem, gtUInt32& fboSize)
{
    (void)(fboSize);  // unused
    bool retVal = false;

    // Get the memory object item data:
    gdDebugApplicationTreeData* pGDItemData = NULL;
    afApplicationTreeItemData* pItemData = NULL;
    _pMemoryTree->getTreeItemDatas(pfboAttachmentItemTreeItem, pItemData, pGDItemData);
    GT_IF_WITH_ASSERT((pItemData != NULL) && (pGDItemData != NULL))
    {
        // Get the attachment node name from tree:
        QString fboAttachmentName = _pMemoryTree->GetTreeItemText(pfboAttachmentItemTreeItem);

        // Build the strings for the FBO attachment target and points:
        gtString fboAttachTarget, fboAttachPoint;

        // Get the buffer type:
        apDisplayBuffer bufferType = apGLEnumToColorIndexBufferType(pGDItemData->_bufferAttachmentPoint);

        // Get the buffer short name:
        bool rc1 = apGetBufferShortName(bufferType, fboAttachPoint);
        GT_ASSERT(rc1);

        // Get the buffer attachment string:
        bool rc2 = apGLFBO::fboAttachmentTargetToString(pGDItemData->_bufferAttachmentTarget, fboAttachTarget);
        GT_ASSERT(rc2);

        // Build string list for this item:
        QStringList itemStrings;
        itemStrings << fboAttachmentName;
        itemStrings << GD_STR_MemoryAnalysisViewerObjectSizeInsignificantA;
        itemStrings << acGTStringToQString(fboAttachTarget);
        itemStrings << acGTStringToQString(fboAttachPoint);

        // Add row for this item:
        retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

        // Set the item list view index:
        pGDItemData->_listViewIndex = rowCount() - 1;

        if (shouldSelectItem)
        {
            ensureRowVisible(pGDItemData->_listViewIndex, true);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addPBufferItem
// Description: Add a pbuffer item to the list (the buffer item is the one associated with
//              the input tree item id)
// Arguments: QTreeWidgetItem* pbufferItemTreeItem
//            bool shouldSelectItem - should the item be selected
//            gtUInt32& pbufferSize - output - the render buffer's size
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/9/2008
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addPBufferItem(QTreeWidgetItem* pbufferItemTreeItem, bool shouldSelectItem, gtUInt32& pbufferSize)
{
    bool retVal = false;

    // Get the memory object item data:
    gdDebugApplicationTreeData* pGDItemData = NULL;
    afApplicationTreeItemData* pItemData = NULL;
    _pMemoryTree->getTreeItemDatas(pbufferItemTreeItem, pItemData, pGDItemData);
    GT_IF_WITH_ASSERT((pItemData != NULL) && (pGDItemData != NULL))
    {
        gtString bufferName, bufferDimensions, bufferRenderContext;

        bufferName.appendFormattedString(GD_STR_ImagesAndBuffersViewerPBufferName, pGDItemData->_objectOpenGLName);

        // Set the PBuffers size:
        pbufferSize = pItemData->m_objectMemorySize;

        GLint width = pGDItemData->_objectWidth;
        GLint height = pGDItemData->_objectHeight;
        bufferDimensions.appendFormattedString(L"%dx%d", width, height);

        apContextID relatedRenderContextId = pGDItemData->_contextId;

        if (relatedRenderContextId._contextId > 0)
        {
            bufferRenderContext.appendFormattedString(L"%d", relatedRenderContextId._contextId);
        }
        else
        {
            bufferRenderContext.append(AF_STR_NotAvailable);
        }

        // Build string list for this item:
        QStringList itemStrings;
        itemStrings << acGTStringToQString(bufferName);
        itemStrings << GD_STR_MemoryAnalysisViewerObjectSizeInsignificantA;
        itemStrings << acGTStringToQString(bufferDimensions);
        itemStrings << acGTStringToQString(bufferRenderContext);

        // Add row for this item:
        retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

        // Set the item list view index:
        pGDItemData->_listViewIndex = rowCount() - 1;

        if (shouldSelectItem)
        {
            ensureRowVisible(pGDItemData->_listViewIndex, true);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addPBufferItem
// Description: Add a pbuffer item to the list (the buffer item is the one associated with
//              the input tree item id)
// Arguments: QTreeWidgetItem* pbufferItemTreeItem
//            bool shouldSelectItem - should the item be selected
//            gtUInt32& pbufferSize - output - the render buffer's size
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/9/2008
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addPBufferStaticBufferItem(QTreeWidgetItem* ppbufferStaticBufferItemTreeItem, bool shouldSelectItem, gtUInt32& pbufferSize)
{
    (void)(pbufferSize);  // unused
    bool retVal = false;

    // Get the memory object item data:
    gdDebugApplicationTreeData* pGDItemData = NULL;
    afApplicationTreeItemData* pItemData = NULL;
    _pMemoryTree->getTreeItemDatas(ppbufferStaticBufferItemTreeItem, pItemData, pGDItemData);
    GT_IF_WITH_ASSERT((pItemData != NULL) && (pGDItemData != NULL))
    {
        // Define the strings for the static buffer column values:
        gtString bufferTypeStr, dimensionsStr = AF_STR_NotAvailable, contextStr;

        // Get the pbuffer static buffer name from the tree:
        QString staticBufferName = _pMemoryTree->GetTreeItemText(ppbufferStaticBufferItemTreeItem);

        // Get the buffer short name:
        bool rc1 = apGetBufferShortName(pGDItemData->_bufferType, bufferTypeStr);
        GT_ASSERT(rc1);

        // Build the context string:
        contextStr.appendFormattedString(L"%d", pGDItemData->_contextId._contextId);

        // Build the buffer dimensions string:
        GT_IF_WITH_ASSERT(_pMemoryTree != NULL)
        {
            // Get the original buffer dimensions:
            gdDebugApplicationTreeData* pStaticBufferData = NULL;
            afApplicationTreeItemData* pStaticBufferAFData = NULL;
            _pMemoryTree->getTreeItemDatas(pItemData->m_pOriginalItemTreeItem, pStaticBufferAFData, pStaticBufferData);
            GT_IF_WITH_ASSERT((pStaticBufferAFData != NULL) && (pStaticBufferData != NULL))
            {
                if ((pStaticBufferData->_objectWidth >= 0) && (pStaticBufferData->_objectHeight >= 0))
                {
                    dimensionsStr.makeEmpty();
                    dimensionsStr.appendFormattedString(L"%dx%d", pStaticBufferData->_objectWidth, pStaticBufferData->_objectHeight);
                }
            }
        }

        // Build string list for this item:
        QStringList itemStrings;
        itemStrings << staticBufferName;
        itemStrings << GD_STR_MemoryAnalysisViewerObjectSizeInsignificantA;
        itemStrings << acGTStringToQString(bufferTypeStr);
        itemStrings << acGTStringToQString(dimensionsStr);
        itemStrings << acGTStringToQString(contextStr);

        // Add row for this item:
        retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

        // Set the item list view index:
        pGDItemData->_listViewIndex = rowCount() - 1;

        if (shouldSelectItem)
        {
            ensureRowVisible(pGDItemData->_listViewIndex, true);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addSyncObjectItem
// Description: Add a sync object item to the list (the sync object item is the one associated with
//              the input tree item id)
// Arguments: QTreeWidgetItem* psyncObjectItemTreeItem
//            bool shouldSelectItem - should the item be selected
//            gtUInt32& pbufferSize - output - the render buffer's size
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/9/2008
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addSyncObjectItem(QTreeWidgetItem* psyncObjectItemTreeItem, bool shouldSelectItem, gtUInt32& syncObjectSize)
{
    bool retVal = false;

    // Get the memory object item data:
    gdDebugApplicationTreeData* pGDItemData = NULL;
    afApplicationTreeItemData* pItemData = NULL;
    _pMemoryTree->getTreeItemDatas(psyncObjectItemTreeItem, pItemData, pGDItemData);
    GT_IF_WITH_ASSERT((pItemData != NULL) && (pGDItemData != NULL))
    {
        gtString syncObjectNameStr;
        syncObjectNameStr.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeSyncName, pGDItemData->_objectOpenGLName);

        // Set the sync object size:
        syncObjectSize = pItemData->m_objectMemorySize;

        // We should use this instead, if we want to display the sync object size:
        // sizeStringFromNumber(syncObjectSizeStr, syncObjectSize);

        // Build string list for this item:
        QStringList itemStrings;
        itemStrings << acGTStringToQString(syncObjectNameStr);
        itemStrings << GD_STR_MemoryAnalysisViewerObjectSizeInsignificantA;

        // Add row for this item:
        retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

        // Set the item list view index:
        pGDItemData->_listViewIndex = rowCount() - 1;

        if (shouldSelectItem)
        {
            ensureRowVisible(pGDItemData->_listViewIndex, true);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addRenderBufferItem
// Description: Add a render buffer item to the list (the buffer item is the one associated with
//              the input tree item id)
// Arguments: QTreeWidgetItem* pbufferItemTreeItem
//            bool shouldSelectItem - should the item be selected
//            gtUInt32& renderBufferSize - output - the render buffer's size
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/9/2008
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addRenderBufferItem(QTreeWidgetItem* pbufferItemTreeItem, bool shouldSelectItem, gtUInt32& renderBufferSize)
{
    bool retVal = false;

    // Get the memory object item data:
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)_pMemoryTree->getTreeItemData(pbufferItemTreeItem);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDItemData != NULL)
        {
            gtString bufferName, bufferAttachment, bufferSize, bufferDimensions, bufferPixelFormatType, bufferPixelFormatFormat;
            bufferName.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeRenderBufferName, pGDItemData->_objectOpenGLName);
            renderBufferSize = pItemData->m_objectMemorySize;

            // Build the buffer's string (for compressed buffer's format the size is not available):
            sizeStringFromNumber(bufferSize, renderBufferSize);

            // Generate dimension string:
            GLint width = pGDItemData->_objectWidth;
            GLint height = pGDItemData->_objectHeight;

            if ((pGDItemData->_objectHeight >= 0) && (pGDItemData->_objectWidth >= 0))
            {
                bufferDimensions.appendFormattedString(L"%dx%d", width, height);
            }
            else
            {
                bufferDimensions.append(AF_STR_NotAvailable);
            }

            // Get the buffer attachment string:
            bool rc1 = apGetBufferShortName(pGDItemData->_bufferType, bufferAttachment);

            if (!rc1)
            {
                bufferAttachment = GD_STR_PropertiesRenderBufferUnAttached;
            }


            // Get render buffer pixel format and data type:
            oaDataType dataType = pGDItemData->_dataType;
            oaTexelDataFormat dataFormat = pGDItemData->_dataFormat;

            // Get Pixel format as string:
            GLenum bufferPixelFormatEnum = oaTexelDataFormatToGLEnum(dataFormat);
            apGLenumParameter bufferPixelFormatParameter(bufferPixelFormatEnum);
            bufferPixelFormatParameter.valueAsString(bufferPixelFormatFormat);

            // Get data type as string:
            GLenum bufferDataTypeEnum = oaDataTypeToGLEnum(dataType);
            apGLenumParameter bufferDataTypeParameter(bufferDataTypeEnum);
            bufferDataTypeParameter.valueAsString(bufferPixelFormatType);

            // Get the buffer attachment name:
            rc1 = apGetBufferShortName(pGDItemData->_bufferType, bufferAttachment);

            if (!rc1)
            {
                bufferAttachment = GD_STR_PropertiesRenderBufferUnAttached;
            }

            // Build string list for this item:
            QStringList itemStrings;
            itemStrings << acGTStringToQString(bufferName);
            itemStrings << acGTStringToQString(bufferSize);
            itemStrings << acGTStringToQString(bufferAttachment);
            itemStrings << acGTStringToQString(bufferDimensions);
            itemStrings << acGTStringToQString(bufferPixelFormatFormat);
            itemStrings << acGTStringToQString(bufferPixelFormatType);

            // Add row for this item:
            retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

            // Set the item list view index:
            pGDItemData->_listViewIndex = rowCount() - 1;

            if (shouldSelectItem)
            {
                ensureRowVisible(pGDItemData->_listViewIndex, true);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addRenderBufferItem
// Description: Add a render buffer item to the list (the buffer item is the one associated with
//              the input tree item id)
// Arguments: vboItemTreeItem vbo item tree id
//            bool shouldSelectItem - should the item be selected
//            gtUInt32& vboSize - output - the VBO size
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/10/2008
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addVBOItem(QTreeWidgetItem* pvboItemTreeItem, bool shouldSelectItem, gtUInt32& vboObjectSize)
{
    bool retVal = false;

    vboObjectSize = 0;

    // Get the memory object item data:
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)_pMemoryTree->getTreeItemData(pvboItemTreeItem);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDItemData != NULL)
        {
            gtString vboName, vboSize, vboAttachment;

            // Get the VBO Details;
            apGLVBO vboDetails;
            bool rc = gaGetVBODetails(pGDItemData->_contextId._contextId, pGDItemData->_objectOpenGLName, vboDetails);
            GT_ASSERT(rc);
            gdGetVBODisplayName(vboDetails, vboName);

            // Set the object size:
            vboObjectSize = pItemData->m_objectMemorySize;

            // Build the vbo's size string:
            sizeStringFromNumber(vboSize, vboObjectSize);

            // Get Pixel format as string:
            GLenum bufferAttachment = pGDItemData->_bufferAttachmentTarget;

            if (bufferAttachment == 0)
            {
                vboAttachment = GD_STR_PropertiesRenderBufferUnAttached;
            }
            else
            {
                apGLenumParameter bufferDataFormatParameter(bufferAttachment);
                bufferDataFormatParameter.valueAsString(vboAttachment);
            }

            // Build string list for this item:
            QStringList itemStrings;
            itemStrings << acGTStringToQString(vboName);
            itemStrings << acGTStringToQString(vboSize);
            itemStrings << acGTStringToQString(vboAttachment);

            // Add row for this item:
            retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

            // Set the item list view index:
            pGDItemData->_listViewIndex = rowCount() - 1;

            if (shouldSelectItem)
            {
                ensureRowVisible(pGDItemData->_listViewIndex, true);
            }
        }
    }

    return retVal;
}

//------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addTotalItem
// Description: Add an item that display the total memory size of the object in the
//              current context
// Arguments:   gtUInt32 - the total size
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        12/10/2008
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addTotalItem(gtUInt32 totalSize, bool isInBeginEndBlock, unsigned int objectsCount)
{
    bool retVal = true;

    // Get the amount of list items:
    int listSize = rowCount();

    // Check if the list is empty:
    bool isEmptyList = (listSize <= 0);

    // Build displayed strings:
    gtString totalItemString = isEmptyList ? AF_STR_NoItems : AF_STR_Total;

    // Get number of columns:
    int numOfColumns = columnCount();

    // Set the string for the column string:
    gtString column1Str;

    if (!isEmptyList)
    {
        if (!isInBeginEndBlock)
        {
            sizeStringFromNumber(column1Str, totalSize);
        }
        else
        {
            column1Str = GD_STR_MemoryAnalysisViewerNAGLBeginEndMessage;
        }
    }

    // Build string list for this item:
    QStringList itemStrings;
    itemStrings << acGTStringToQString(totalItemString);
    itemStrings << acGTStringToQString(column1Str);

    int firstColumnToEmpty = 2;

    // If the list displays the number of items in the child object, display the total:
    if ((m_currentDisplayedItem.m_itemType == AF_TREE_ITEM_APP_ROOT) || (m_currentDisplayedItem.m_itemType == AF_TREE_ITEM_GL_RENDER_CONTEXT))
    {
        // Only add the value if it is greater than zero
        if (objectsCount > 0)
        {
            gtString objectCountString;
            objectCountString.appendFormattedString(L"%u", objectsCount).addThousandSeperators();
            itemStrings << acGTStringToQString(objectCountString);
            firstColumnToEmpty++;
        }
    }

    for (int i = firstColumnToEmpty; i < numOfColumns; i++)
    {
        itemStrings << "";
    }

    // Add row for this item:
    retVal = addRow(itemStrings, NULL, false, Qt::Unchecked);

    // Set the item bold:
    setItemBold(rowCount() - 1);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addGLContextChildObjectToList
// Description: Add an object to a render context objects list.
// Arguments: QTreeWidgetItem* pobjectItemTreeItem
//            bool shouldSelectItem
//            gtUInt32& renderContextObjectSize
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/10/2008
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addGLContextChildObjectToList(QTreeWidgetItem* pobjectItemTreeItem, bool shouldSelectItem, gtUInt32& renderContextObjectSize)
{
    bool retVal = false;

    // Get the memory object item data:
    gdDebugApplicationTreeData* pGDItemData = NULL;
    afApplicationTreeItemData* pItemData = NULL;
    _pMemoryTree->getTreeItemDatas(pobjectItemTreeItem, pItemData, pGDItemData);
    GT_IF_WITH_ASSERT((pItemData != NULL) && (pGDItemData != NULL))
    {

        gtASCIIString itemTypeString;
        gtString objectSizeString;
        gtString objectCountString;

        // Calculate the object size (summary of the item's children sizes):
        calculateTreeItemSize(pobjectItemTreeItem, renderContextObjectSize);

        // Build the object size's string:
        sizeStringFromNumber(objectSizeString, renderContextObjectSize);

        objectCountString.appendFormattedString(L"%u", pItemData->m_objectCount);
        objectCountString.addThousandSeperators();

        // Get the item name according to the tree item type:
        switch (pItemData->m_itemType)
        {
            case AF_TREE_ITEM_GL_TEXTURES_NODE:
            {
                itemTypeString = GD_STR_MemoryAnalysisViewerListTexturesNodeName;
            }
            break;

            case AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE:
            {
                itemTypeString = GD_STR_MemoryAnalysisViewerListRenderBuffersNodeName;
            }
            break;

            case AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE:
            {
                itemTypeString = GD_STR_MemoryAnalysisViewerListStaticBuffersNodeName;
            }
            break;

            case AF_TREE_ITEM_GL_VBO_NODE:
            {
                itemTypeString = GD_STR_MemoryAnalysisViewerListVBONodeName;
            }
            break;

            case AF_TREE_ITEM_GL_PROGRAMS_NODE:
            {
                itemTypeString = GD_STR_MemoryAnalysisViewerListGLProgramsNodeName;

                // We do not know the size of programs:
                objectSizeString = GD_STR_MemoryAnalysisViewerObjectSizeInsignificant;
            }
            break;

            case AF_TREE_ITEM_GL_SAMPLERS_NODE:
            {
                itemTypeString = GD_STR_MemoryAnalysisViewerListOpenGLSamplersNodeName;

                // The size of OpenGL samplers is considered insignificant (a few bytes):
                objectSizeString = GD_STR_MemoryAnalysisViewerObjectSizeInsignificant;
            }
            break;

            case AF_TREE_ITEM_GL_PROGRAM_PIPELINES_NODE:
            {
                itemTypeString = GD_STR_MemoryAnalysisViewerListOpenGLProgramPipelinesNodeName;

                // The size of OpenGL program pipelines is considered insignificant (a few bytes):
                objectSizeString = GD_STR_MemoryAnalysisViewerObjectSizeInsignificant;
            }
            break;

            case AF_TREE_ITEM_GL_SHADERS_NODE:
            {
                itemTypeString = GD_STR_MemoryAnalysisViewerListShadersNodeName;
            }
            break;

            case AF_TREE_ITEM_GL_DISPLAY_LISTS_NODE:
            {
                itemTypeString = GD_STR_MemoryAnalysisViewerListDisplayListsNodeName;
            }
            break;

            case AF_TREE_ITEM_GL_FBO_NODE:
            {
                itemTypeString = GD_STR_MemoryAnalysisViewerListFBOsNodeName;

                // We do not know the size of FBOs:
                objectSizeString = GD_STR_MemoryAnalysisViewerObjectSizeInsignificant;
            }
            break;

            default:
            {
                retVal = false;
                GT_ASSERT_EX(retVal, L"Unknown object type");
            }
            break;
        }

        // Build string list for this item:
        QStringList itemStrings;
        itemStrings << itemTypeString.asCharArray();
        itemStrings << acGTStringToQString(objectSizeString);
        itemStrings << acGTStringToQString(objectCountString);

        // Add row for this item:
        retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

        // Set the item list view index:
        pGDItemData->_listViewIndex = rowCount() - 1;

        if (shouldSelectItem)
        {
            ensureRowVisible(pGDItemData->_listViewIndex, true);
        }
    }

    return retVal;
}

// TO_DO: VS MEMORY:  what is this functions?
// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addCLContextChildObjectToList
// Description: Add an object to an OpenCL context objects list.
// Arguments: QTreeWidgetItem* pobjectItemTreeItem
//            bool shouldSelectItem
//            gtUInt32& clContextObjectSize
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/12/2009
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addCLContextChildObjectToList(QTreeWidgetItem* pobjectItemTreeItem, bool shouldSelectItem, gtUInt32& clContextObjectSize)
{
    bool retVal = false;

    // Get the memory object item data:
    gdDebugApplicationTreeData* pGDItemData = NULL;
    afApplicationTreeItemData* pItemData = NULL;
    _pMemoryTree->getTreeItemDatas(pobjectItemTreeItem, pItemData, pGDItemData);
    GT_IF_WITH_ASSERT((pItemData != NULL) && (pGDItemData != NULL))
    {
        gtASCIIString itemTypeString;
        gtString objectSizeString;
        gtString objectCountString;
        gtString objectRefCountString;

        // Calculate the object size (summary of the item's children sizes):
        calculateTreeItemSize(pobjectItemTreeItem, clContextObjectSize);

        // Build the object size's string:
        sizeStringFromNumber(objectSizeString, clContextObjectSize);

        objectCountString.appendFormattedString(L"%u", pItemData->m_objectCount);
        objectCountString.addThousandSeperators();

        objectRefCountString.appendFormattedString(L"%d", pGDItemData->_referenceCount);

        // Get the item name according to the tree item type:
        switch (pItemData->m_itemType)
        {

            case AF_TREE_ITEM_CL_IMAGES_NODE:
            {
                itemTypeString = GD_STR_MemoryAnalysisViewerListImagesNodeName;
            }
            break;

            case AF_TREE_ITEM_CL_BUFFERS_NODE:
            {
                itemTypeString = GD_STR_MemoryAnalysisViewerListCLBuffersNodeName;
            }
            break;

            case AF_TREE_ITEM_CL_PIPES_NODE:
            {
                itemTypeString = GD_STR_MemoryAnalysisViewerListCLPipesNodeName;
            }
            break;

            case AF_TREE_ITEM_CL_COMMAND_QUEUES_NODE:
            {
                itemTypeString = GD_STR_MemoryAnalysisViewerListCommandQueuesNodeName;
            }
            break;

            case AF_TREE_ITEM_CL_PROGRAMS_NODE:
            {
                itemTypeString = GD_STR_MemoryAnalysisViewerListCLProgramsNodeName;
            }
            break;

            case AF_TREE_ITEM_CL_SAMPLERS_NODE:
            {
                itemTypeString = GD_STR_MemoryAnalysisViewerListCLSamplersNodeName;
            }
            break;

            case AF_TREE_ITEM_CL_EVENTS_NODE:
            {
                itemTypeString = GD_STR_MemoryAnalysisViewerListCLEventsNodeName;
            }
            break;

            default:
            {
                retVal = false;
                GT_ASSERT_EX(retVal, L"Unknown object type");
            }
            break;
        }

        // Build string list for this item:
        QStringList itemStrings;
        itemStrings << itemTypeString.asCharArray();
        itemStrings << acGTStringToQString(objectSizeString);
        itemStrings << acGTStringToQString(objectCountString);

        // Add row for this item:
        retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

        // Set the item list view index:
        pGDItemData->_listViewIndex = rowCount() - 1;

        if (shouldSelectItem)
        {
            ensureRowVisible(pGDItemData->_listViewIndex, true);
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::calculateTreeItemSize
// Description: Recursive function that calculate the item size
// Arguments: QTreeWidgetItem* ptreeItemId
//            gtUInt32& itemSize
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        12/10/2008
// ---------------------------------------------------------------------------
void gdMemoryAnalysisDetailsView::calculateTreeItemSize(QTreeWidgetItem* ptreeItemId, gtUInt32& itemSize)
{
    bool sizeFound = false;

    // Get the item's data:
    // Get the memory object item data:
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)_pMemoryTree->getTreeItemData(ptreeItemId);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        if (pItemData->m_objectMemorySize > 0)
        {
            // Add the item's size:
            itemSize += pItemData->m_objectMemorySize;
            sizeFound = true;
        }

        // If size was not set on the tree item data, calculate it by it's children's size:
        if (!sizeFound)
        {
            for (int i = 0 ; i < ptreeItemId->childCount(); i++)
            {
                QTreeWidgetItem* pChild = ptreeItemId->child(i);

                if (pChild != NULL)
                {
                    // Call recursively to the child's size calculation:
                    calculateTreeItemSize(pChild, itemSize);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addCLImageItem
// Description: Add a texture item to the list (the texture item is the one associated with
//              the input tree item id)
// Arguments: QTreeWidgetItem* ptextureItemTreeItem
//            bool shouldSelectItem - should the item be selected
//            gtUInt32& textureObjectSize - output - the texture object's size
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/12/2009
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addCLImageItem(QTreeWidgetItem* ptextureItemTreeItem, bool shouldSelectItem, gtUInt32& textureObjectSize)
{
    bool retVal = false;

    // Get the memory object item data:
    // Get the memory object item data:
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)_pMemoryTree->getTreeItemData(ptextureItemTreeItem);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDItemData != NULL)
        {
            gtString textureName, textureType, textureSize, textureRefCount, textureDimensions, textureDataType, texturePixelFormat, textureMemoryFlags;

            // Get the texture details:
            apCLImage textureDetails;
            bool rc = gaGetOpenCLImageObjectDetails(pGDItemData->_contextId._contextId, pGDItemData->_objectOpenCLIndex, textureDetails);
            GT_IF_WITH_ASSERT(rc)
            {
                // Build the texture name:
                gdHTMLProperties htmlBuilder;
                htmlBuilder.getCLImageName(textureDetails, textureName);
            }

            // Set the texture's memory size:
            textureObjectSize = pItemData->m_objectMemorySize;

            // Build the texture size's string (for compressed texture's format the size is not available):
            sizeStringFromNumber(textureSize, textureObjectSize);

            textureRefCount.appendFormattedString(L"%d", pGDItemData->_referenceCount);

            // Generate getDimensions string:
            gtSize_t width = pGDItemData->_objectWidth;
            gtSize_t height = pGDItemData->_objectHeight;
            gtSize_t depth = pGDItemData->_objectDepth;

            textureDimensions = AF_STR_NotAvailable;

            if (pGDItemData->_textureType == AP_2D_TEXTURE)
            {
                textureDimensions.makeEmpty();
                textureDimensions.appendFormattedString(L"%dx%d", width, height);
            }
            else if (pGDItemData->_textureType == AP_3D_TEXTURE)
            {
                textureDimensions.makeEmpty();
                textureDimensions.appendFormattedString(L"%dx%dx%d", width, height, depth);
            }

            // Get the texture type as string:
            apTextureTypeAsString(pGDItemData->_textureType, textureType);

            // Translate the pixel format and data types to string:
            oaGetTexelDataFormatName(pGDItemData->_dataFormat, texturePixelFormat);
            oaDataTypeAsString(pGDItemData->_dataType, textureDataType);

            // Translate the image memory flags to a string:
            pGDItemData->_memoryFlags.valueAsString(textureMemoryFlags);

            // Build string list for this item:
            QStringList itemStrings;
            itemStrings << acGTStringToQString(textureName);
            itemStrings << acGTStringToQString(textureSize);
            itemStrings << acGTStringToQString(textureType);
            itemStrings << acGTStringToQString(textureDimensions);
            itemStrings << acGTStringToQString(texturePixelFormat);
            itemStrings << acGTStringToQString(textureDataType);
            itemStrings << acGTStringToQString(textureMemoryFlags);
            itemStrings << acGTStringToQString(textureRefCount);

            // Add row for this item:
            retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

            // Set the item list view index:
            pGDItemData->_listViewIndex = rowCount() - 1;

            if (shouldSelectItem)
            {
                ensureRowVisible(pGDItemData->_listViewIndex, true);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addCLBufferItem
// Description: Add an OpenCL item to the list (the OpenCL buffer item is the one associated with
//              the input tree item id)
// Arguments: QTreeWidgetItem* pbufferItemTreeItem
//            bool shouldSelectItem - should the item be selected
//            gtUInt32& bufferObjectSize - output - the buffer object's size
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/12/2009
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addCLBufferItem(QTreeWidgetItem* pbufferItemTreeItem, bool shouldSelectItem, gtUInt32& bufferObjectSize)
{
    bool retVal = false;

    // Get the memory object item data:
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)_pMemoryTree->getTreeItemData(pbufferItemTreeItem);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDItemData != NULL)
        {
            gtString bufferName, bufferMemoryFlags, bufferSize, bufferReferenceCount;

            // Build the buffer's string:
            sizeStringFromNumber(bufferSize, pItemData->m_objectMemorySize);

            // Build the buffer name:
            bufferName.appendFormattedString(L"Buffer %d", pGDItemData->_objectOpenCLName);

            // Set the buffer's memory size:
            bufferObjectSize = pItemData->m_objectMemorySize;

            // Translate the buffer memory flags to a string:
            pGDItemData->_memoryFlags.valueAsString(bufferMemoryFlags);

            bufferReferenceCount.appendFormattedString(L"%d", pGDItemData->_referenceCount);

            // Build string list for this item:
            QStringList itemStrings;
            itemStrings << acGTStringToQString(bufferName);
            itemStrings << acGTStringToQString(bufferSize);
            itemStrings << acGTStringToQString(bufferMemoryFlags);
            itemStrings << acGTStringToQString(bufferReferenceCount);

            // Add row for this item:
            retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

            // Set the item list view index:
            pGDItemData->_listViewIndex = rowCount() - 1;

            if (shouldSelectItem)
            {
                ensureRowVisible(pGDItemData->_listViewIndex, true);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addCLSubBufferItem
// Description: Add an OpenCL item to the list (the OpenCL sub buffer item is the one associated with
//              the input tree item id)
// Arguments:   QTreeWidgetItem* pbufferItemTreeItem
//              bool shouldSelectItem - should the item be selected
//              gtUInt32& bufferObjectSize - output - the buffer object's size
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/1/2011
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addCLSubBufferItem(QTreeWidgetItem* psubBufferItemTreeItem, bool shouldSelectItem, gtUInt32& bufferObjectSize)
{
    bool retVal = false;

    // Get the memory object item data:
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)_pMemoryTree->getTreeItemData(psubBufferItemTreeItem);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDItemData != NULL)
        {
            gtString bufferName, bufferMemoryFlags, bufferSize, bufferReferenceCount;

            // Build the buffer's string:
            sizeStringFromNumber(bufferSize, pItemData->m_objectMemorySize);

            // Build the buffer name:
            bufferName.appendFormattedString(L"Sub-Buffer %d", pGDItemData->_objectOpenCLName);

            // Set the buffer's memory size:
            bufferObjectSize = pItemData->m_objectMemorySize;

            // Translate the buffer memory flags to a string:
            pGDItemData->_memoryFlags.valueAsString(bufferMemoryFlags);

            bufferReferenceCount.appendFormattedString(L"%d", pGDItemData->_referenceCount);

            // Build string list for this item:
            QStringList itemStrings;
            itemStrings << acGTStringToQString(bufferName);
            itemStrings << acGTStringToQString(bufferSize);
            itemStrings << acGTStringToQString(bufferMemoryFlags);
            itemStrings << acGTStringToQString(bufferReferenceCount);

            // Add row for this item:
            retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

            // Set the item list view index:
            pGDItemData->_listViewIndex = rowCount() - 1;

            if (shouldSelectItem)
            {
                ensureRowVisible(pGDItemData->_listViewIndex, true);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addCLPipeItem
// Description: Add a pipe item to the list (the pipe item is the one associated with
//              the input tree item id)
// Arguments: QTreeWidgetItem* pPipeItemTreeItem
//            bool shouldSelectItem - should the item be selected
//            gtUInt32& pipeObjectSize - output - the pipe object's size
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        1/10/2014
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addCLPipeItem(QTreeWidgetItem* pPipeItemTreeItem, bool shouldSelectItem, gtUInt32& pipeObjectSize)
{
    bool retVal = false;

    // Get the memory object item data:
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)_pMemoryTree->getTreeItemData(pPipeItemTreeItem);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDItemData != NULL)
        {
            gtString pipeName;
            gtString pipeSize;
            gtString pipePacketSize;
            gtString pipeMaxPackets;
            gtString pipeMemoryFlags;
            gtString pipeRefCount;

            pipeName.appendFormattedString(L"Pipe %d", pGDItemData->_objectOpenCLName);

            // Set the Pipe's memory size:
            pipeObjectSize = pItemData->m_objectMemorySize;

            // Build the pipe size's string (for compressed pipe's format the size is not available):
            sizeStringFromNumber(pipeSize, pipeObjectSize);

            pipePacketSize.appendFormattedString(L"%d", pGDItemData->m_packetSize);
            pipeMaxPackets.appendFormattedString(L"%d", pGDItemData->m_maxPackets);

            // Translate the pipe memory flags to a string:
            pGDItemData->_memoryFlags.valueAsString(pipeMemoryFlags);

            pipeRefCount.appendFormattedString(L"%d", pGDItemData->_referenceCount);

            // Build string list for this item:
            QStringList itemStrings;
            itemStrings << acGTStringToQString(pipeName);
            itemStrings << acGTStringToQString(pipeSize);
            itemStrings << acGTStringToQString(pipePacketSize);
            itemStrings << acGTStringToQString(pipeMaxPackets);
            itemStrings << acGTStringToQString(pipeMemoryFlags);
            itemStrings << acGTStringToQString(pipeRefCount);

            // Add row for this item:
            retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

            // Set the item list view index:
            pGDItemData->_listViewIndex = rowCount() - 1;

            if (shouldSelectItem)
            {
                ensureRowVisible(pGDItemData->_listViewIndex, true);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addCLCommandQueueItem
// Description: Add an OpenCL item to the list (the OpenCL command queue item is
//              the one associated with the input tree item id)
// Arguments: QTreeWidgetItem* pcommandQueueItemTreeItem
//            bool shouldSelectItem - should the item be selected
//            gtUInt32& bufferObjectSize - output - the buffer object's size
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        16/2/2010
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addCLCommandQueueItem(QTreeWidgetItem* pcommandQueueItemTreeItem, bool shouldSelectItem, gtUInt32& commandQueueObjectSize)
{
    bool retVal = false;

    // Get the memory object item data:
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)_pMemoryTree->getTreeItemData(pcommandQueueItemTreeItem);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDItemData != NULL)
        {
            // Define strings for command queue attributes:
            gtString commandQueueNameStr, commandQueueSizeStr, refCountStr, amountOfEventsStr, outOfOrderExecutionModeStr, profilingModeStr, onDeviceStr;

            // Build the command queue name:
            commandQueueNameStr.appendFormattedString(GD_STR_PropertiesCLCommandQueueName, pGDItemData->_objectOpenCLName);

            // Build the command queue's size string:
            if (0 < pItemData->m_objectMemorySize)
            {
                sizeStringFromNumber(commandQueueSizeStr, pItemData->m_objectMemorySize);
            }
            else
            {
                commandQueueSizeStr = GD_STR_MemoryAnalysisViewerObjectSizeInsignificant;
            }

            // Set the command queue's memory size:
            commandQueueObjectSize = pItemData->m_objectMemorySize;

            // Build the reference count string:
            refCountStr.appendFormattedString(L"%d", pGDItemData->_referenceCount);

            // Build the amount of events string:
            amountOfEventsStr.appendFormattedString(L"%d", pGDItemData->_amountOfEvents);

            // Build the out of order execution mode string:
            outOfOrderExecutionModeStr = pGDItemData->m_queueOutOfOrderExecutionMode ? AF_STR_Enabled : AF_STR_Disabled;

            // Build the profiling mode string:
            profilingModeStr = pGDItemData->m_queueProfilingMode ? AF_STR_Enabled : AF_STR_Disabled;

            // Build the on-device string:
            onDeviceStr = pGDItemData->m_queueOnDevice ? GD_STR_MemoryAnalysisViewerListQueueOnDeviceOnDevice : GD_STR_MemoryAnalysisViewerListQueueOnDeviceHost;

            if (pGDItemData->m_queueOnDeviceDefault)
            {
                // Only device queues can be the on-device default:
                GT_IF_WITH_ASSERT(pGDItemData->m_queueOnDevice)
                {
                    onDeviceStr.append(GD_STR_MemoryAnalysisViewerListQueueOnDeviceDefault);
                }
            }

            // Build string list for this item:
            QStringList itemStrings;
            itemStrings << acGTStringToQString(commandQueueNameStr);
            itemStrings << acGTStringToQString(commandQueueSizeStr);
            itemStrings << acGTStringToQString(refCountStr);
#ifdef GD_MEMORY_ANALYSIS_DETAILS_DISPLAY_CL_QUEUE_EVENTS
            itemStrings << acGTStringToQString(amountOfEventsStr);
#endif
            itemStrings << acGTStringToQString(outOfOrderExecutionModeStr);
            itemStrings << acGTStringToQString(profilingModeStr);
            itemStrings << acGTStringToQString(onDeviceStr);

            // Add row for this item:
            retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

            // Set the item list view index:
            pGDItemData->_listViewIndex = rowCount() - 1;

            if (shouldSelectItem)
            {
                ensureRowVisible(pGDItemData->_listViewIndex, true);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addCLProgramItem
// Description: Add an OpenCL item to the list (the OpenCL program item is
//              the one associated with the input tree item id)
// Arguments:   QTreeWidgetItem* pprogramItemTreeItem
//              bool shouldSelectItem - should the item be selected
//              gtUInt32& programObjectSize - output - the program object's size
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        16/2/2010
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addCLProgramItem(QTreeWidgetItem* pprogramItemTreeItem, bool shouldSelectItem, gtUInt32& programObjectSize)
{
    (void)(programObjectSize);  // unused
    bool retVal = false;

    // Get the memory object item data:
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)_pMemoryTree->getTreeItemData(pprogramItemTreeItem);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDItemData != NULL)
        {
            // Build the program name string:
            gtString programName;
            programName.appendFormattedString(GD_STR_PropertiesProgramNameFormat, pGDItemData->_objectOpenCLName);

            // Build the program's size string:
            gtString programSizeString = GD_STR_MemoryAnalysisViewerObjectSizeInsignificant;
            // We should use this instead, if we want to display the program size:
            // sizeStringFromNumber(programSizeString, pCLProgramData->m_objectMemorySize);

            gtString programRefCountString;
            programRefCountString.appendFormattedString(L"%d", pGDItemData->_referenceCount);

            // Build string list for this item:
            QStringList itemStrings;
            itemStrings << acGTStringToQString(programName);
            itemStrings << acGTStringToQString(programSizeString);
            itemStrings << acGTStringToQString(programRefCountString);

            // Add row for this item:
            retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

            // Set the item list view index:
            pGDItemData->_listViewIndex = rowCount() - 1;

            if (shouldSelectItem)
            {
                ensureRowVisible(pGDItemData->_listViewIndex, true);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addCLKernelItem
// Description: Add an OpenCL item to the list (the OpenCL kernel item is
//              the one associated with the input tree item id)
// Arguments:   QTreeWidgetItem* pkernelItemTreeItem
//              bool shouldSelectItem - should the item be selected
//              gtUInt32& kernelObjectSize - output - the kernel object's size
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/2/2010
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addCLKernelItem(QTreeWidgetItem* pkernelItemTreeItem, bool shouldSelectItem, gtUInt32& kernelObjectSize)
{
    (void)(kernelObjectSize);  // unused
    bool retVal = false;

    // Get the memory object item data:
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)_pMemoryTree->getTreeItemData(pkernelItemTreeItem);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDItemData != NULL)
        {
            // Build the kernel name string:
            QString kernelName = _pMemoryTree->GetTreeItemText(pkernelItemTreeItem);

            QString kernelReferenceCount;
            kernelReferenceCount.sprintf("%d", pGDItemData->_referenceCount);

            // Build string list for this item:
            QStringList itemStrings;
            itemStrings << kernelName;
            itemStrings << GD_STR_MemoryAnalysisViewerObjectSizeInsignificantA;
            itemStrings << kernelReferenceCount;

            // Add row for this item:
            retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

            // Set the item list view index:
            pGDItemData->_listViewIndex = rowCount() - 1;

            if (shouldSelectItem)
            {
                ensureRowVisible(pGDItemData->_listViewIndex, true);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addCLSamplerItem
// Description: Add an OpenCL item to the list (the OpenCL sampler item is
//              the one associated with the input tree item id)
// Arguments:   QTreeWidgetItem* psamplerItemTreeItem
//              bool shouldSelectItem
//              gtUInt32& programObjectSize
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/5/2010
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addCLSamplerItem(QTreeWidgetItem* psamplerItemTreeItem, bool shouldSelectItem, gtUInt32& programObjectSize)
{
    (void)(programObjectSize);  // unused
    bool retVal = false;

    // Get the memory object item data:
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)_pMemoryTree->getTreeItemData(psamplerItemTreeItem);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDItemData != NULL)
        {
            gtString samplerName, filterModeString, addressingModeString;

            // Build the sampler name string:
            samplerName.appendFormattedString(GD_STR_PropertiesSamplerNameFormat, pGDItemData->_objectOpenCLName);

            // Build string list for this item:
            QStringList itemStrings;
            itemStrings << acGTStringToQString(samplerName);
            itemStrings << GD_STR_MemoryAnalysisViewerObjectSizeInsignificantA;
            itemStrings << acGTStringToQString(filterModeString);
            itemStrings << acGTStringToQString(addressingModeString);

            // Add row for this item:
            retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

            // Set the item list view index:
            pGDItemData->_listViewIndex = rowCount() - 1;

            if (shouldSelectItem)
            {
                ensureRowVisible(pGDItemData->_listViewIndex, true);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addCLEventItem
// Description: Add an OpenCL item to the list (the OpenCL event item is
//              the one associated with the input tree item id)
// Arguments:   QTreeWidgetItem* pEventItemTreeItem
//              bool shouldSelectItem
//              gtUInt32& programObjectSize
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addCLEventItem(QTreeWidgetItem* pEventItemTreeItem, bool shouldSelectItem, gtUInt32& programObjectSize)
{
    (void)(programObjectSize);  // unused
    bool retVal = false;

    // Get the memory object item data:
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)_pMemoryTree->getTreeItemData(pEventItemTreeItem);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDItemData != NULL)
        {
            gtString eventName, filterModeString, addressingModeString;

            // Build the event name string:
            eventName.appendFormattedString(GD_STR_PropertiesCLEventName, pGDItemData->_objectOpenCLName);

            // Build string list for this item:
            QStringList itemStrings;
            itemStrings << acGTStringToQString(eventName);
            itemStrings << GD_STR_MemoryAnalysisViewerObjectSizeInsignificantA;

            // Add row for this item:
            retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

            // Set the item list view index:
            pGDItemData->_listViewIndex = rowCount() - 1;

            if (shouldSelectItem)
            {
                ensureRowVisible(pGDItemData->_listViewIndex, true);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        sortIntValues / sortStringValues
// Description: Return sort value for 2 integer / strings values
// Arguments:   int value1
//              int value2
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        26/9/2010
// ---------------------------------------------------------------------------
bool sortIntValues(Qt::SortOrder sortOrder, int value1, int value2)
{
    bool shouldSortUp = (sortOrder == Qt::AscendingOrder);
    bool retVal = ((!shouldSortUp && (value1 > value2)) || (shouldSortUp && (value1 < value2)));
    return retVal;
}

bool sortStringValues(Qt::SortOrder sortOrder, const gtString& value1, const gtString& value2)
{
    bool shouldSortUp = (sortOrder == Qt::AscendingOrder);
    bool retVal = ((!shouldSortUp && (value1 > value2)) || (shouldSortUp && (value1 < value2)));
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::isItemSmallerThen
// Description: Compare 2 memory items by their item datas
// Arguments:   afApplicationTreeItemData* pItemData1
//              afApplicationTreeItemData* pItemData2
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        22/1/2012
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::isItemSmallerThen(afApplicationTreeItemData* pItemData1, afApplicationTreeItemData* pItemData2)
{
    bool retVal = false;

    // If one of our items has a NULL data, it means that it is one of the total items.
    // Notice: since we want the total item to be last both in ascending and descending sort order,
    // we 'cheat' and return false / true according to the current sort order:
    if ((pItemData1 == 0) && (pItemData2 == 0))
    {
        retVal = false;
    }
    else if ((pItemData1 == 0) && (pItemData2 != 0))
    {
        if (_sortInfo._sortOrder == Qt::AscendingOrder)
        {
            retVal = false;
        }
        else
        {
            retVal = true;
        }

    }
    else if ((pItemData1 != 0) && (pItemData2 == 0))
    {
        if (_sortInfo._sortOrder == Qt::AscendingOrder)
        {
            retVal = true;
        }
        else
        {
            retVal = false;
        }
    }

    else
    {
        // Get the GD item data:
        gdDebugApplicationTreeData* pGDItemData1 = qobject_cast<gdDebugApplicationTreeData*>(pItemData1->extendedItemData());
        gdDebugApplicationTreeData* pGDItemData2 = qobject_cast<gdDebugApplicationTreeData*>(pItemData2->extendedItemData());
        GT_IF_WITH_ASSERT((pGDItemData1 != NULL) && (pGDItemData2 != NULL))
        {
            switch (_sortInfo._sortType)
            {
                case GD_SORT_BY_NAME:
                {
                    int itemName1 = pGDItemData1->_objectOpenGLName;
                    int itemName2 = pGDItemData2->_objectOpenGLName;

                    if (pGDItemData1->_contextId.isOpenCLContext())
                    {
                        itemName1 = pGDItemData1->_objectOpenCLIndex;
                        itemName2 = pGDItemData2->_objectOpenCLIndex;
                    }

                    retVal = sortIntValues(_sortInfo._sortOrder, itemName1, itemName2);
                }
                break;

                case GD_SORT_BY_SIZE:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, pItemData1->m_objectMemorySize, pItemData2->m_objectMemorySize);
                }
                break;

                case GD_SORT_BY_AMOUNT_OF_VERTICES:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, pGDItemData1->_amountOfRenderedVertices, pGDItemData2->_amountOfRenderedVertices);
                }
                break;

                case GD_SORT_BY_MEMORY_FLAGS:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, (int)pGDItemData1->_memoryFlags.getFlags(), (int)pGDItemData2->_memoryFlags.getFlags());
                }
                break;

                case GD_SORT_BY_PACKET_SIZE:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, (int)pGDItemData1->m_packetSize, (int)pGDItemData2->m_packetSize);
                }
                break;

                case GD_SORT_BY_MAX_PACKETS:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, (int)pGDItemData1->m_maxPackets, (int)pGDItemData2->m_maxPackets);
                }
                break;

                case GD_SORT_BY_REF_COUNT:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, pGDItemData1->_referenceCount, pGDItemData2->_referenceCount);
                }
                break;

                case GD_SORT_BY_AMOUNT_OF_EVENTS:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, pGDItemData1->_amountOfEvents, pGDItemData2->_amountOfEvents);
                }
                break;

                case GD_SORT_BY_OUT_OF_ORDER_EXE_MODE:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, pGDItemData1->m_queueOutOfOrderExecutionMode ? 1 : 0, pGDItemData2->m_queueOutOfOrderExecutionMode ? 1 : 0);
                }
                break;

                case GD_SORT_BY_PROFILING_MODE:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, pGDItemData1->m_queueProfilingMode ? 1 : 0, pGDItemData2->m_queueProfilingMode ? 1 : 0);
                }
                break;

                case GD_SORT_BY_ON_DEVICE:
                {
                    int val1 = (pGDItemData1->m_queueOnDevice ? 2 : 0) + (pGDItemData1->m_queueOnDeviceDefault ? 1 : 0);
                    int val2 = (pGDItemData2->m_queueOnDevice ? 2 : 0) + (pGDItemData2->m_queueOnDeviceDefault ? 1 : 0);
                    retVal = sortIntValues(_sortInfo._sortOrder, val1, val2);
                }
                break;

                case GD_SORT_BY_TYPE:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, (int)pGDItemData1->_textureType, (int)pGDItemData2->_textureType);
                }
                break;

                case GD_SORT_BY_DIMENSIONS:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, (int)pGDItemData1->_objectWidth, (int)pGDItemData2->_objectWidth);

                    if (pGDItemData1->_objectWidth == pGDItemData2->_objectWidth)
                    {
                        retVal = sortIntValues(_sortInfo._sortOrder, (int)pGDItemData1->_objectHeight, (int)pGDItemData2->_objectWidth);
                    }
                }
                break;

                case GD_SORT_BY_MIPMAP:
                {
                    retVal = sortStringValues(_sortInfo._sortOrder, pGDItemData1->_mipmapStr, pGDItemData2->_mipmapStr);
                }
                break;

                case GD_SORT_BY_FORMAT:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, (int)pGDItemData1->_internalFormat, (int)pGDItemData2->_internalFormat);
                }
                break;

                case GD_SORT_BY_ATTACHMENT_TARGET:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, (int)pGDItemData1->_bufferType, (int)pGDItemData2->_bufferType);
                }
                break;

                case GD_SORT_BY_ATTACHMENT_POINT:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, (int)pGDItemData1->_bufferAttachmentPoint, (int)pGDItemData2->_bufferAttachmentPoint);
                }
                break;

                case GD_SORT_BY_PIXEL_TYPE:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, (int)pGDItemData1->_dataType, (int)pGDItemData2->_dataType);
                }
                break;

                case GD_SORT_BY_DATA_TYPE:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, (int)pGDItemData1->_dataFormat, (int)pGDItemData2->_dataFormat);
                }
                break;

                case GD_SORT_BY_VBO_ATTACHMENT:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, (int)pGDItemData1->_bufferAttachmentTarget, (int)pGDItemData2->_bufferAttachmentTarget);
                }
                break;

                case GD_SORT_BY_SYNC_HANDLE:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, (int)pGDItemData1->_syncHandle, (int)pGDItemData2->_syncHandle);
                }
                break;

                case GD_SORT_BY_SHADER_TYPE:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, (int)pGDItemData1->_shaderType, (int)pGDItemData2->_shaderType);
                }
                break;

                case GD_SORT_BY_OBJECTS_COUNT:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, (int)pItemData1->m_objectCount, (int)pItemData2->m_objectCount);
                }
                break;

                case GD_SORT_BY_RENDER_CONTEXT:
                {
                    // Treat all non positive values the same - in case we accidentally set both 0 / -1 here
                    int itemRCId1 = pGDItemData1->_contextId._contextId;
                    itemRCId1 = max(itemRCId1, 0);
                    int itemRCId2 = pGDItemData2->_contextId._contextId;
                    itemRCId2 = max(itemRCId2, 0);

                    retVal = sortIntValues(_sortInfo._sortOrder, itemRCId1, itemRCId2);
                }
                break;

                case GD_SORT_BY_FILTER_MODE:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, (int)pGDItemData1->_filterMode, (int)pGDItemData2->_filterMode);
                }
                break;

                case GD_SORT_BY_ADDRESSING_MODE:
                {
                    retVal = sortIntValues(_sortInfo._sortOrder, (int)pGDItemData1->_addressingMode, (int)pGDItemData2->_addressingMode);
                }
                break;

                default:
                {
                    // Unknown sort option:
                    GT_ASSERT(false);
                }
                break;
            }
        }
    }

    // Uri, 31/12/08: For some reason, Mac list controls are sorted in reverse (even though this function
    // returns correct values). To overcome this problem, we return inverse values intentionally.
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    retVal *= -1;
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdCanSortByColumnInTL
// Description: Checks for a specific column's data if we can sort by it under a free license
// Author:      Uri Shomroni
// Date:        22/6/2009
// ---------------------------------------------------------------------------
bool gdCanSortByColumnInTL(const afTreeItemType& listItemsType, const size_t& columnIndex)
{
    bool retVal = false;

    switch (listItemsType)
    {
        case AF_TREE_ITEM_APP_ROOT:
        case AF_TREE_ITEM_GL_RENDER_CONTEXT:
            // These items have no detailed children:
            retVal = true;
            break;

        case AF_TREE_ITEM_GL_TEXTURES_NODE:
        case AF_TREE_ITEM_GL_TEXTURE:
            retVal = ((columnIndex != 1) && (columnIndex != 4) && (columnIndex != 5));
            break;

        case AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE:
        case AF_TREE_ITEM_GL_RENDER_BUFFER:
            retVal = ((columnIndex != 1) && (columnIndex != 4) && (columnIndex != 5));
            break;

        case AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE:
        case AF_TREE_ITEM_GL_STATIC_BUFFER:
            retVal = ((columnIndex != 1) && (columnIndex != 3) && (columnIndex != 4));
            break;

        case AF_TREE_ITEM_GL_PBUFFERS_NODE:
        case AF_TREE_ITEM_GL_PBUFFER_NODE:
            retVal = (columnIndex != 1);
            break;

        case AF_TREE_ITEM_GL_VBO_NODE:
        case AF_TREE_ITEM_GL_VBO:
            retVal = (columnIndex != 1);
            break;

        case AF_TREE_ITEM_GL_PROGRAMS_NODE:
        case AF_TREE_ITEM_GL_PROGRAM:
            retVal = (columnIndex != 1);
            break;

        case AF_TREE_ITEM_GL_SHADERS_NODE:
        case AF_TREE_ITEM_GL_VERTEX_SHADER:
        case AF_TREE_ITEM_GL_TESSELLATION_CONTROL_SHADER:
        case AF_TREE_ITEM_GL_TESSELLATION_EVALUATION_SHADER:
        case AF_TREE_ITEM_GL_GEOMETRY_SHADER:
        case AF_TREE_ITEM_GL_FRAGMENT_SHADER:
        case AF_TREE_ITEM_GL_COMPUTE_SHADER:
        case AF_TREE_ITEM_GL_UNSUPPORTED_SHADER:
            retVal = (columnIndex != 1);
            break;

        case AF_TREE_ITEM_GL_DISPLAY_LISTS_NODE:
        case AF_TREE_ITEM_GL_DISPLAY_LIST:
            retVal = (columnIndex != 1) && (columnIndex != 2);
            break;

        case AF_TREE_ITEM_GL_FBO_NODE:
        case AF_TREE_ITEM_GL_FBO:
            retVal = (columnIndex != 1);
            break;

        default:
            // Something's wrong:
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::itemDataToParentType
// Description: Translate an item data to parent type
// Arguments:   const gdDebugApplicationTreeData* pItemData
//              afTreeItemType& parentType
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/1/2011
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::itemDataToParentType(const afApplicationTreeItemData* pItemData, afTreeItemType& parentType) const
{
    bool retVal = true;

    parentType = AF_TREE_ITEM_APP_ROOT;

    switch (pItemData->m_itemType)
    {
        case AF_TREE_ITEM_APP_ROOT:
            parentType = AF_TREE_ITEM_APP_ROOT;
            break;

        case AF_TREE_ITEM_GL_RENDER_CONTEXT:
        {
            if (pItemData->m_objectCount > 0)
            {
                // This is a non-empty render context:
                parentType = AF_TREE_ITEM_GL_RENDER_CONTEXT;
            }
            else
            {
                // This is an empty render context, so display the list for the app:
                parentType = AF_TREE_ITEM_APP_ROOT;
            }
        }
        break;

        case AF_TREE_ITEM_GL_TEXTURES_NODE:
        case AF_TREE_ITEM_GL_TEXTURE:
            parentType = AF_TREE_ITEM_GL_TEXTURES_NODE;
            break;

        case AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE:
        case AF_TREE_ITEM_GL_RENDER_BUFFER:
            parentType = AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE;
            break;

        case AF_TREE_ITEM_GL_STATIC_BUFFER:
        case AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE:
            parentType = AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE;
            break;

        case AF_TREE_ITEM_GL_PBUFFERS_NODE:
            parentType = AF_TREE_ITEM_GL_PBUFFERS_NODE;
            break;

        case AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER:
            parentType = AF_TREE_ITEM_GL_PBUFFER_NODE;
            break;

        case AF_TREE_ITEM_GL_PBUFFER_NODE:
        {
            if (pItemData->m_objectCount > 0)
            {
                // This is a non-empty pbuffer:
                parentType = AF_TREE_ITEM_GL_PBUFFER_NODE;
            }
            else
            {
                // This is an empty pbuffer, so display the list for the app:
                parentType = AF_TREE_ITEM_GL_PBUFFERS_NODE;
            }
        }
        break;

        case AF_TREE_ITEM_GL_VBO:
        case AF_TREE_ITEM_GL_VBO_NODE:
            parentType = AF_TREE_ITEM_GL_VBO_NODE;
            break;

        case AF_TREE_ITEM_GL_PROGRAM:
        case AF_TREE_ITEM_GL_PROGRAMS_NODE:
            parentType = AF_TREE_ITEM_GL_PROGRAMS_NODE;
            break;

        case AF_TREE_ITEM_GL_PROGRAM_PIPELINE:
        case AF_TREE_ITEM_GL_PROGRAM_PIPELINES_NODE:
            parentType = AF_TREE_ITEM_GL_PROGRAM_PIPELINES_NODE;
            break;

        case AF_TREE_ITEM_GL_SAMPLER:
        case AF_TREE_ITEM_GL_SAMPLERS_NODE:
            parentType = AF_TREE_ITEM_GL_SAMPLERS_NODE;
            break;

        case AF_TREE_ITEM_GL_VERTEX_SHADER:
        case AF_TREE_ITEM_GL_TESSELLATION_CONTROL_SHADER:
        case AF_TREE_ITEM_GL_TESSELLATION_EVALUATION_SHADER:
        case AF_TREE_ITEM_GL_GEOMETRY_SHADER:
        case AF_TREE_ITEM_GL_FRAGMENT_SHADER:
        case AF_TREE_ITEM_GL_COMPUTE_SHADER:
        case AF_TREE_ITEM_GL_UNSUPPORTED_SHADER:
        case AF_TREE_ITEM_GL_SHADERS_NODE:
            parentType = AF_TREE_ITEM_GL_SHADERS_NODE;
            break;

        case AF_TREE_ITEM_GL_DISPLAY_LIST:
        case AF_TREE_ITEM_GL_DISPLAY_LISTS_NODE:
            parentType = AF_TREE_ITEM_GL_DISPLAY_LISTS_NODE;
            break;

        case AF_TREE_ITEM_GL_FBO:
        case AF_TREE_ITEM_GL_FBO_NODE:
            parentType = AF_TREE_ITEM_GL_FBO_NODE;
            break;

        case AF_TREE_ITEM_GL_FBO_ATTACHMENT:
            parentType = AF_TREE_ITEM_GL_FBO;
            break;

        case AF_TREE_ITEM_GL_SYNC_OBJECT:
        case AF_TREE_ITEM_GL_SYNC_OBJECTS_NODE:
            parentType = AF_TREE_ITEM_GL_SYNC_OBJECTS_NODE;
            break;

        case AF_TREE_ITEM_CL_IMAGES_NODE:
        case AF_TREE_ITEM_CL_IMAGE:
            parentType = AF_TREE_ITEM_CL_IMAGES_NODE;
            break;

        case AF_TREE_ITEM_CL_BUFFERS_NODE:
        case AF_TREE_ITEM_CL_BUFFER:
        {
            if (pItemData->m_objectCount > 0)
            {
                // This is a buffer with sub-buffers:
                parentType = AF_TREE_ITEM_CL_BUFFER;
            }
            else
            {
                // This is a buffer with no sub buffers:
                parentType = AF_TREE_ITEM_CL_BUFFERS_NODE;
            }

            break;
        }

        case AF_TREE_ITEM_CL_SUB_BUFFER:
            parentType = AF_TREE_ITEM_CL_BUFFER;
            break;

        case AF_TREE_ITEM_CL_PIPES_NODE:
        case AF_TREE_ITEM_CL_PIPE:
            parentType = AF_TREE_ITEM_CL_PIPES_NODE;
            break;

        case AF_TREE_ITEM_CL_COMMAND_QUEUE:
        case AF_TREE_ITEM_CL_COMMAND_QUEUES_NODE:
            parentType = AF_TREE_ITEM_CL_COMMAND_QUEUES_NODE;
            break;

        case AF_TREE_ITEM_CL_PROGRAM:
        case AF_TREE_ITEM_CL_PROGRAMS_NODE:
            parentType = AF_TREE_ITEM_CL_PROGRAMS_NODE;
            break;

        case AF_TREE_ITEM_CL_KERNEL:
            parentType = AF_TREE_ITEM_CL_PROGRAM;
            break;

        case AF_TREE_ITEM_CL_SAMPLER:
        case AF_TREE_ITEM_CL_SAMPLERS_NODE:
            parentType = AF_TREE_ITEM_CL_SAMPLERS_NODE;
            break;

        case AF_TREE_ITEM_CL_EVENT:
        case AF_TREE_ITEM_CL_EVENTS_NODE:
            parentType = AF_TREE_ITEM_CL_EVENTS_NODE;
            break;

        case AF_TREE_ITEM_CL_CONTEXT:
        {
            if (pItemData->m_objectCount > 0)
            {
                // This is a non-empty context:
                parentType = AF_TREE_ITEM_CL_CONTEXT;
            }
            else
            {
                // This is an empty render context, so display the list for the app:
                parentType = AF_TREE_ITEM_APP_ROOT;
            }
        }
        break;

        default:
            retVal = false;
            GT_ASSERT_EX(retVal, L"Unsupported memory object type");
            break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::onChartItemClicked
// Description:
// Arguments:   int clickedItemIndex
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        23/1/2012
// ---------------------------------------------------------------------------
void gdMemoryAnalysisDetailsView::onChartItemClicked(int clickedItemIndex)
{
    // Select the item:
    GT_IF_WITH_ASSERT((clickedItemIndex >= 0) && (clickedItemIndex < rowCount()))
    {
        ensureRowVisible(clickedItemIndex, true);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryAnalysisDetailsView::addGlSamplerItem
// Description: Adds an OpenGL sampler item into the list
// Return Val:  bool  - Success / failure.
// Author:      Amit Ben-Moshe
// Date:        3/8/2014
// ---------------------------------------------------------------------------
bool gdMemoryAnalysisDetailsView::addGlSamplerItem(QTreeWidgetItem* pitemToAdd, bool shouldSelectItem)
{
    bool retVal = false;

    // Get the memory object item data:
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)_pMemoryTree->getTreeItemData(pitemToAdd);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDItemData != NULL)
        {
            gtString samplerName;
            samplerName.appendFormattedString(GD_STR_PropertiesGlSamplerNameFormat, pGDItemData->_objectOpenGLName);

            gtString programSizeString;

            // Build string list for this item:
            QStringList itemStrings;
            itemStrings << acGTStringToQString(samplerName);
            itemStrings << GD_STR_MemoryAnalysisViewerObjectSizeInsignificantA;

            // Add row for this item:
            retVal = addRow(itemStrings, pItemData, false, Qt::Unchecked);

            // Set the item list view index:
            pGDItemData->_listViewIndex = rowCount() - 1;

            if (shouldSelectItem)
            {
                ensureRowVisible(pGDItemData->_listViewIndex, true);
            }
        }
    }

    return retVal;
}

