//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdDebugApplicationTreeHandler.cpp
///
//==================================================================================

//------------------------------ gdDebugApplicationTreeHandler.cpp ------------------------------

// Include compiler warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

#include <AMDTApplicationComponents/Include/acChartWindow.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStatisticsView.h>

// Qt pre compiled header:
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acQtIncludes.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Infra:
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apMemoryLeakEvent.h>
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunSuspendedEvent.h>
#include <AMDTAPIClasses/Include/apAPIConnectionType.h>
#include <AMDTAPIClasses/Include/apCLBuffer.h>
#include <AMDTAPIClasses/Include/apCLCommandQueue.h>
#include <AMDTAPIClasses/Include/apCLContext.h>
#include <AMDTAPIClasses/Include/apCLEvent.h>
#include <AMDTAPIClasses/Include/apCLImage.h>
#include <AMDTAPIClasses/Include/apCLKernel.h>
#include <AMDTAPIClasses/Include/apCLObjectID.h>
#include <AMDTAPIClasses/Include/apCLPipe.h>
#include <AMDTAPIClasses/Include/apCLProgram.h>
#include <AMDTAPIClasses/Include/apCLSampler.h>
#include <AMDTAPIClasses/Include/apCLSubBuffer.h>
#include <AMDTAPIClasses/Include/apGLRenderBuffer.h>
#include <AMDTAPIClasses/Include/apGLRenderContextInfo.h>
#include <AMDTAPIClasses/Include/apGLShaderObject.h>
#include <AMDTAPIClasses/Include/apGLTexture.h>
#include <AMDTAPIClasses/Include/apGLSync.h>
#include <AMDTAPIClasses/Include/apGLVBO.h>
#include <AMDTAPIClasses/Include/apGLPipeline.h>
#include <AMDTAPIClasses/Include/apGLSampler.h>
#include <AMDTAPIClasses/Include/apPBuffer.h>
#include <AMDTAPIClasses/Include/apStaticBuffer.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acToolBar.h>
#include <AMDTApplicationComponents/Include/acTreeCtrl.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdImagesAndBuffersManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTGpuDebuggingComponents/Include/gdStatisticsPanel.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdPropertiesEventObserver.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdMemoryView.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeHandler.h>

#define GD_ESTIMATED_AMOUNT_OF_STATIC_BUFFERS_PER_HOLDER 8

// Helper macros:
// Calculate a rounded up KB count from bits / bytes. This is equivalent to ceil((float) x / factor)), and does not require floating point operations, so it's faster.
#define GD_MEM_SIZE_BYTES_TO_KB(x) ((x + 1023) / 1024)
#define GD_MEM_SIZE_BITS_TO_KB(x) ((x + 8191) / 8192)

// Static member initializations
gdDebugApplicationTreeHandler* gdDebugApplicationTreeHandler::m_pMySingleInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::gdDebugApplicationTreeHandler
// Description: Constructor.
// Arguments:   parent - My parent window
//              windowID - This window's ID
// Author:      Sigal Algranaty
// Date:        26/10/2010
// ---------------------------------------------------------------------------
gdDebugApplicationTreeHandler::gdDebugApplicationTreeHandler():
    m_pApplicationTree(NULL), m_pHeaderItem(NULL), m_isTextureMemoryDataUpdateRequired(false), m_isInfoUpdated(false),
    m_pPBuffersTreeId(NULL), m_pSyncObjectsTreeId(NULL), m_pLastMemoryLeakEvent(NULL),
    m_texture1DIconIndex(-1), m_texture2DIconIndex(-1), m_texture3DIconIndex(-1),
    m_texture1DArrayIconIndex(-1), m_texture2DArrayIconIndex(-1), m_texture2DMultisampleIconIndex(-1), m_texture2DMultisampleArrayIconIndex(-1), m_allTexturesIconIndex(-1), m_texturesShortcutIconIndex(-1),
    m_textureCubeMapIconIndex(-1), m_textureCubeMapArrayIconIndex(-1), m_textureRectangleIconIndex(-1), m_textureBufferIconIndex(-1), m_textureUnknownIconIndex(-1), m_openGLBufferGenericIconIndex(-1),
    m_openGLBufferArrayIconIndex(-1), m_openGLBufferDrawIndirIconIndex(-1), m_openGLBufferDispatchIndirIconIndex(-1), m_openGLBufferElementArrayIconIndex(-1), m_openGLBufferPixelPackIconIndex(-1),
    m_openGLBufferPixelUnpackIconIndex(-1), m_openGLBufferCopyReadIconIndex(-1), m_openGLBufferCopyWriteIconIndex(-1), m_openGLBufferTransformFeedbackIconIndex(-1), m_openGLBufferUniformIconIndex(-1),
    m_openGLBufferAtomicIconIndex(-1), m_openGLBufferShaderStorageIconIndex(-1), m_openGLBufferQueryIconIndex(-1), m_openGLBufferTextureIconIndex(-1), m_openGLBufferUnknownIconIndex(-1),
    m_renderBufferIconIndex(-1), m_renderBufferShortcutIconIndex(-1), m_staticBufferIconIndex(-1), m_staticBufferShortcutIconIndex(-1), m_pbufferIconIndex(-1),
    m_syncObjectsIconIndex(-1), m_commandQueueIconIndex(-1), m_displayListIconIndex(-1), m_renderContextIconIndex(-1), m_renderContextDeletedIconIndex(-1),
    m_renderContextSharedIconIndex(-1), m_renderContextDeletedSharedIconIndex(-1), m_computeContextIconIndex(-1), m_computeContextDeletedIconIndex(-1), m_openGLProgramIconIndex(-1), m_openGLProgramDeletedIconIndex(-1),
    m_openGLShaderIconIndex(-1), m_openGLShaderDeletedIconIndex(-1),
    m_fboIconIndex(-1), m_glSamplerIconIndex(-1), m_openGLProgramPipelineIconIndex(-1), m_allCLImagesIconIndex(-1), m_clImage2DIconIndex(-1), m_clImage3DIconIndex(-1), m_clBufferIconIndex(-1),
    m_clPipeIconIndex(-1), m_clSamplerIconIndex(-1), m_clEventIconIndex(-1), m_openCLProgramIconIndex(-1), m_openCLKernelIconIndex(-1), m_memoryLeakIconIndex(-1), m_informationIconIndex(-1),
    m_isDebuggedProcessSuspended(false), m_pApplicationCommands(NULL), m_pActivatedItemData(NULL)
{
    // Get the tree control from framework:
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
    {
        m_pApplicationTree = pApplicationCommands->applicationTree();
        GT_IF_WITH_ASSERT(m_pApplicationTree != NULL)
        {
            m_pHeaderItem = m_pApplicationTree->headerItem();
            m_pApplicationTree->registerApplicationTreeHandler(this);
        }

        // Create the icons image list:
        createAndLoadImageList();

        // Register myself to listen to debugged process events:
        apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);

        // Reset the last selected item:
        m_pApplicationTree->resetLastSelectedItem();

        // Do not update memory data by default:
        m_isTextureMemoryDataUpdateRequired = false;

        // Get the application commands instance:
        m_pApplicationCommands = gdApplicationCommands::gdInstance();
        GT_ASSERT(m_pApplicationCommands != NULL);
    }
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        instance
/// \brief Description: My single instance
/// \return gdDebugApplicationTreeHandler*
/// -----------------------------------------------------------------------------------------------
gdDebugApplicationTreeHandler* gdDebugApplicationTreeHandler::instance()
{
    if (m_pMySingleInstance == NULL)
    {
        m_pMySingleInstance = new gdDebugApplicationTreeHandler;
    }

    return m_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getTreeItemData
// Description:
// Arguments:   QTreeWidgetItem* pTreeItem
// Return Val:  gdDebugApplicationTreeData*
// Author:      Sigal Algranaty
// Date:        1/11/2010
// ---------------------------------------------------------------------------
afApplicationTreeItemData* gdDebugApplicationTreeHandler::getTreeItemData(QTreeWidgetItem* pTreeItem) const
{
    afApplicationTreeItemData* pRetVal = NULL;
    GT_IF_WITH_ASSERT(pTreeItem != NULL)
    {
        QVariant itemData = pTreeItem->data(0, Qt::UserRole);
        pRetVal = (afApplicationTreeItemData*)itemData.value<void*>();
    }
    return pRetVal;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        getTreeItemDatas
/// \brief Description: Get the generic and GD item datas for a tree item
/// \param[in]          pTreeItem
/// \param[in]          pItemData
/// \param[in]          pGDItemData
/// \return void
/// -----------------------------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::getTreeItemDatas(QTreeWidgetItem* pTreeItem, afApplicationTreeItemData*& pItemData, gdDebugApplicationTreeData*& pGDItemData) const
{
    pItemData = NULL;
    pGDItemData = NULL;
    pItemData = getTreeItemData(pTreeItem);

    if (pItemData != NULL)
    {
        pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::ItemHasChildren
// Description:
// Arguments:   QTreeWidgetItem* pTreeItem
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/11/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::treeItemHasChildren(QTreeWidgetItem* pTreeItem) const
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(pTreeItem != NULL)
    {
        retVal = (pTreeItem->childCount() > 0);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getTreeChildrenCount
// Description:
// Arguments:   QTreeWidgetItem* pTreeItem
//              bool recursively
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        1/11/2010
// ---------------------------------------------------------------------------
int gdDebugApplicationTreeHandler::getTreeChildrenCount(QTreeWidgetItem* pTreeItem, bool recursively) const
{
    (void)(recursively);  // unused
    int retVal = 0;
    GT_IF_WITH_ASSERT(pTreeItem != NULL)
    {
        retVal = pTreeItem->childCount();
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getTreeItemParent
// Description:
// Arguments:   const QTreeWidgetItem* item
// Return Val:  QTreeWidgetItem
// Author:      Sigal Algranaty
// Date:        1/11/2010
// ---------------------------------------------------------------------------
QTreeWidgetItem* gdDebugApplicationTreeHandler::getTreeItemParent(const QTreeWidgetItem* pItem) const
{
    QTreeWidgetItem* pRetVal = NULL;
    GT_IF_WITH_ASSERT(pItem != NULL)
    {
        pRetVal = pItem->parent();
    }
    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::setItemIcon
// Description: Set the icon for a tree item
// Arguments:   QTreeWidgetItem* pItem
//              int index
// Author:      Sigal Algranaty
// Date:        8/8/2012
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::setItemIcon(QTreeWidgetItem* pItem, int iconIndex, bool recursive)
{
    GT_IF_WITH_ASSERT(pItem != NULL)
    {
        GT_IF_WITH_ASSERT((iconIndex >= 0) && (iconIndex < (int)m_treeItemsVector.size()))
        {
            QPixmap* pPixmap = m_treeItemsVector[iconIndex];
            GT_IF_WITH_ASSERT(pPixmap != NULL)
            {
                pItem->setIcon(0, *pPixmap);
            }

            if (recursive)
            {
                for (int i = 0; i < pItem->childCount(); i++)
                {
                    setItemIcon(pItem, iconIndex, recursive);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::~gdDebugApplicationTreeHandler
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        23/9/2008
// ---------------------------------------------------------------------------
gdDebugApplicationTreeHandler::~gdDebugApplicationTreeHandler()
{
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::clearTreeItems
// Description: Clear all tree items
// Arguments:   bool addNonAvailableMessage - should add a message to the tree that
//              states that memory information is not available at the moment.
// Return Val: void
// Author:      Sigal Algranaty
// Date:        23/9/2008
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::clearTreeItems(bool addNonAvailableMessage)
{
    (void)(addNonAvailableMessage);  // unused
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pApplicationTree != NULL) && (m_pHeaderItem != NULL))
    {
        // Clear all the debugger nodes from the application tree:
        int count = m_pHeaderItem->childCount();

        for (int i = count - 1  ; i >= 0; i--)
        {
            // Get the next child:
            QTreeWidgetItem* pChild = m_pHeaderItem->child(i);

            if (pChild != NULL)
            {
                afApplicationTreeItemData* pItemData = m_pApplicationTree->getTreeItemData(pChild);

                if (pItemData != NULL)
                {
                    gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());

                    if (pGDData != NULL)
                    {
                        m_pHeaderItem->removeChild(pChild);
                    }
                }
            }
        }

        // Clear the OpenGL render context vector:
        m_openGLContextsTreeIds.clear();

        // Clear the OpenCL context vector:
        m_openCLContextsTreeIds.clear();

        // Clear the maps:
        m_itemIndexToItemDataMap.clear();
    }
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateMemoryAnalysisApplicationTree
// Description: Updates a debugged process memory analysis tree
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/9/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateMonitoredObjectsTree()
{
    bool retVal = true;

    if (!m_isInfoUpdated && gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_SPIES_UTILITIES_API_CONNECTION))
    {
        // Clear all tree items:
        clearTreeItems(false);

        GT_IF_WITH_ASSERT(m_pApplicationTree != NULL)
        {
            afApplicationTreeItemData* pRootData = m_pApplicationTree->rootItemData();
            GT_IF_WITH_ASSERT(pRootData != NULL)
            {
                // Reset the application object size & count:
                pRootData->m_objectCount = 0;
                pRootData->m_objectMemorySize = 0;

                gtVector<apContextID> vListSharingContexts;

                // Update the OpenGL memory items:
                bool rc1 = updateOpenGLMonitoredObjects(vListSharingContexts);

                // Update the OpenCL memory items:
                bool rc2 = updateOpenCLMonitoredObjects(vListSharingContexts);

                retVal = rc1 && rc2;

                // Mark the sharing contexts:
                markSharingContexts(vListSharingContexts);

                // Hide the progress bar dialog:
                afProgressBarWrapper::instance().hideProgressBar();

                // Try to select the previous selected item (might be impossible if the item doesn't exist in the application memory)
                selectPreviouslySelectedItem();

                // Refresh the window:
                m_pApplicationTree->repaint();
            }
        }

        m_isInfoUpdated = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::selectPreviouslySelectedItem
// Description: Select the same item that was previously selected by the user if this item exists
//              If the item doesn't exist, we select the closest existing parent
// Arguments:   gdMonitoredObjectInfo previouslySelectedItemData
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        26/10/2008
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::selectPreviouslySelectedItem()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pApplicationTree != NULL)
    {
        // Get the current tree item to select:
        // The tree item id for selection:
        QTreeWidgetItem* pTreeItemIdToSelect = m_pApplicationTree->headerItem();

        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_SPIES_UTILITIES_API_CONNECTION))
        {
            // Get the triggering context id:
            apContextID triggeringContextId;
            bool rc = gaGetBreakpointTriggeringContextId(triggeringContextId);
            GT_ASSERT(rc);

            // Get the triggering context matching item data:
            QTreeWidgetItem* pContextTreeItemId = getContextTreeItemId(triggeringContextId);
            GT_IF_WITH_ASSERT(pContextTreeItemId != NULL)
            {
                // Select the triggering context id by default:
                pTreeItemIdToSelect = pContextTreeItemId;
            }

            // Get the last item selected:
            const afApplicationTreeItemData* pLastItemSelected = m_pApplicationTree->getCurrentlySelectedItemData();

            if (pLastItemSelected != NULL)
            {
                // "More items" messages do not have a GD data:
                if (AF_TREE_ITEM_MESSAGE != pLastItemSelected->m_itemType)
                {
                    gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pLastItemSelected->extendedItemData());

                    // last item selected can have a non debug information (like root item)
                    if (pGDData != NULL)
                    {
                        // If the selected
                        if (pGDData->_contextId == triggeringContextId)
                        {
                            // Try to find this item in the current tree:
                            afApplicationTreeItemData* pMatchingItemData = FindMatchingTreeItem(*pLastItemSelected);

                            if (pMatchingItemData != NULL)
                            {
                                // Get the matching item tree id:
                                pTreeItemIdToSelect = pMatchingItemData->m_pTreeWidgetItem;
                            }
                        }
                    }
                }
            }
        }

        // If there is a selected item, select it:
        if ((pTreeItemIdToSelect != NULL) && (pTreeItemIdToSelect != m_pApplicationTree->headerItem()))
        {
            pTreeItemIdToSelect->setSelected(true);
            pTreeItemIdToSelect->setExpanded(true);
            m_pApplicationTree->scrollToItem(pTreeItemIdToSelect, QAbstractItemView::EnsureVisible);

            // Send a tree selection event:
            afApplicationTreeItemData* pTreeItemData = getTreeItemData(pTreeItemIdToSelect);

            if (pTreeItemData != NULL)
            {
                // Create an event handling the tree selection:
                apMonitoredObjectsTreeSelectedEvent treeItemEvent(pTreeItemData);

                // Register the event:
                apEventsHandler::instance().registerPendingDebugEvent(treeItemEvent);
            }
        }
        else
        {
            // Expand the current render context items:
            expandCurrentContext();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateOpenGLMonitoredObjects
// Description: Update the OpenGL existing memory items.
// Arguments:   updateOpenGLMonitoredObjects* pApplicationRootData - the application root item
// Return Val: void
// Author:      Sigal Algranaty
// Date:        6/12/2009
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateOpenGLMonitoredObjects(gtVector<apContextID>& vListSharingContexts)
{
    bool retVal = true;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pApplicationTree != NULL)
    {
        afApplicationTreeItemData* pRootData = m_pApplicationTree->rootItemData();
        GT_IF_WITH_ASSERT(pRootData != NULL)
        {
            if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
            {
                // Estimate the amount of items, and initialize progress:
                initializeGLObjectsUpdateProgress();

                // Update the application render contexts:
                unsigned int objectCount = 0;
                bool rcRC = updateApplicationGLRenderContexts(objectCount, vListSharingContexts);
                pRootData->m_objectCount += objectCount;

                // Update the application's pixel buffers:
                bool rcPBO = true, rcSyncs = true;
                rcPBO = updateApplicationPBuffers(objectCount);
                pRootData->m_objectCount += objectCount;

                // Update the application's sync objects:
                rcSyncs = updateApplicationSyncObjects(objectCount);
                pRootData->m_objectCount += objectCount;

                retVal = rcRC && rcPBO && rcSyncs;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::initializeGLObjectsUpdateProgress
// Description: Estimate the amount of existing objects, and initialize progress
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/9/2010
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::initializeGLObjectsUpdateProgress()
{
    // Only if parent frame exist, we update progress:
    if (afProgressBarWrapper::instance().shouldUpdateProgress())
    {
        // Estimate the number of allocated objects that will appear in the tree:
        // Since static buffers are not counted in the spy, we add the number of RCs and PBOs
        // Times the number of static buffers for each to the estimate
        unsigned int treeNodesEstimate = 0;
        bool rcAllocObj = gaGetAmountOfRegisteredAllocatedObjects(treeNodesEstimate);
        GT_IF_WITH_ASSERT(rcAllocObj)
        {
            int numberOfStaticBufferHolders = 0;
            rcAllocObj = gaGetAmountOfRenderContexts(numberOfStaticBufferHolders);
            GT_IF_WITH_ASSERT(rcAllocObj)
            {
                treeNodesEstimate += (numberOfStaticBufferHolders * GD_ESTIMATED_AMOUNT_OF_STATIC_BUFFERS_PER_HOLDER);
                rcAllocObj = gaGetAmountOfPBuffersObjects(numberOfStaticBufferHolders);
                GT_IF_WITH_ASSERT(rcAllocObj)
                {
                    int numberOfSyncObjects;
                    rcAllocObj = gaGetAmountOfSyncObjects(numberOfSyncObjects);
                    GT_IF_WITH_ASSERT(rcAllocObj)
                    {
                        afProgressBarWrapper::instance().setProgressDetails(AF_STR_EmptyStr, treeNodesEstimate);
                    }
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        setCLNodeTextFromStrAndData
// Description: Sets the Strings of a CL node based on the original string and the data of the node
// Author:      Gilad Yarnitzky
// Date:        16/3/2011
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::setCLNodeTextFromStrAndData(gtString& originalString, afApplicationTreeItemData* pTreeNodeData)
{
    GT_IF_WITH_ASSERT(pTreeNodeData != NULL)
    {
        gdDebugApplicationTreeData* pItemData = qobject_cast<gdDebugApplicationTreeData*>(pTreeNodeData->extendedItemData());
        GT_IF_WITH_ASSERT(pItemData != NULL)
        {
            // If sample name is not empty add it to the display:
            if (!pItemData->_objectOpenCLNameStr.isEmpty())
            {
                originalString.append(L" (");
                originalString.append(pItemData->_objectOpenCLNameStr);
                originalString.append(L")");
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateOpenCLMonitoredObjects
// Description: Update the OpenCL monitored item in the tree
// Arguments:   rootMemorySize - add the memory size to this variable
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/12/2009
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateOpenCLMonitoredObjects(gtVector<apContextID>& vListSharingContexts)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(m_pApplicationTree != NULL)
    {
        retVal = true;

        // Update the application render contexts:
        unsigned int objectCount = 0;
        bool rcRC = updateApplicationOpenCLContexts(objectCount, vListSharingContexts);

        // Append the root object count:
        afApplicationTreeItemData* pRootItemData = m_pApplicationTree->rootItemData();
        GT_IF_WITH_ASSERT(pRootItemData != NULL)
        {
            pRootItemData->m_objectCount += objectCount;
        }

        retVal = rcRC;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::addMoreItemsMessage
// Description: When the item count for a specific category exceeds the project setting,
//              display a message which shows this and explains how to change the setting.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        10/11/2014
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::addMoreItemsMessage(QTreeWidgetItem* pParent, int hiddenItemsCount)
{
    bool retVal = false;

    gtString treeText;
    treeText.appendFormattedString(GD_STR_MonitoredObjectsTreeItemsHiddenText, hiddenItemsCount);
    gtString detailText;
    detailText.appendFormattedString(GD_STR_MonitoredObjectsTreeItemsHiddenDetails, gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType());
    afApplicationTreeItemData* pMessageData = afCreateMessageTreeItemData(GD_STR_MonitoredObjectsTreeItemsHiddenTitle, detailText);
    QTreeWidgetItem* pTreeItem = m_pApplicationTree->addTreeItem(treeText, pMessageData, pParent);
    setItemIcon(pTreeItem, m_informationIconIndex);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::expandCurrentContext
// Description: Expand the current context, and select the render context root
// Return Val: void
// Author:      Sigal Algranaty
// Date:        18/11/2008
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::expandCurrentContext()
{
    if (((m_openGLContextsTreeIds.size() > 0) || (m_openCLContextsTreeIds.size() > 0)) && (m_pApplicationTree != NULL))
    {
        // First collapse all items:
        m_pApplicationTree->collapseAll();

        // Get the application chosen context id:
        gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();
        apContextID chosenContextId = globalVarsManager.chosenContext();

        // Get the relevant tree item id:
        QTreeWidgetItem* pRenderTreeItemId = getContextTreeItemId(chosenContextId);

        // Do not check for the root, if we have no contexts (or the null context is selected), expand and select the root
        GT_IF_WITH_ASSERT(pRenderTreeItemId != NULL)
        {
            m_pApplicationTree->expandItem(pRenderTreeItemId);
            m_pApplicationTree->clearSelection();
            pRenderTreeItemId->setSelected(true);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateApplicationGLRenderContexts
// Description: Updates the currently existing render contexts in the executed application.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/9/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateApplicationGLRenderContexts(unsigned int& objectCount, gtVector<apContextID>& vListSharingContexts)
{
    bool retVal = true;

    // Get the current amount of contexts:
    int currentContextsAmount = 0;
    objectCount = 0;
    bool rc = gaGetAmountOfRenderContexts(currentContextsAmount);
    GT_IF_WITH_ASSERT(rc)
    {
        // Iterate the render contexts, and add for each of them a node to the tree:
        for (int renderContextId = 1; renderContextId < currentContextsAmount; renderContextId++)
        {
            // Display "Updating RC ###..."
            gtString progressMessage;
            progressMessage.makeEmpty();
            progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingContext GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, renderContextId);
            afProgressBarWrapper::instance().setProgressDetails(progressMessage, currentContextsAmount);

            // Add the current render context to the application tree:
            rc = updateOpenGLRenderContext(renderContextId, objectCount, vListSharingContexts);
            GT_ASSERT(rc);
            retVal = retVal && rc;
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateOpenGLRenderContext
// Description: Updates an OpenGL single render context
// Arguments:   int renderContextId
//              objectCount - is updated with the render context
//              gtVector<int> vListSharingContexts - a vector of sharing context ids.
//              The context is added if its sharing.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/7/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateOpenGLRenderContext(int renderContextId, unsigned int& objectCount, gtVector<apContextID>& vListSharingContexts)
{
    bool retVal = true;
    unsigned int currentCategoryObjectCount = 0;

    // Build the context name string;
    gtString currentRenderContextName;
    apContextID contextId(AP_OPENGL_CONTEXT, renderContextId);
    gdGetContextNameString(contextId, currentRenderContextName);

    // Create a new memory item data, and fill the relevant fields:
    gdDebugApplicationTreeData* pRenderContextItemData = NULL;
    afApplicationTreeItemData* pItemData = NULL;
    createObjectItemData(pItemData, pRenderContextItemData);

    // Set the render context details:
    pItemData->m_itemType = AF_TREE_ITEM_GL_RENDER_CONTEXT;
    pItemData->m_objectCount = 0;
    pRenderContextItemData->_contextId._contextId = renderContextId;
    pRenderContextItemData->_contextId._contextType = AP_OPENGL_CONTEXT;

    // Check if the render context was deleted:
    bool isDeleted = gaWasContextDeleted(pRenderContextItemData->_contextId);
    pRenderContextItemData->_isMarkedForDeletion = isDeleted;

    // Find the icon index according to the deletion status:
    int iconIndex = (isDeleted) ? m_renderContextDeletedIconIndex : m_renderContextIconIndex;

    // Check if the context is shared, and include it in its name:
    apGLRenderContextInfo RCInfo;
    bool isSharingGL = false;
    bool rcRCInfo = gaGetRenderContextDetails(renderContextId, RCInfo);
    GT_IF_WITH_ASSERT(rcRCInfo)
    {
        if ((RCInfo.sharingContextID() > 0) || (RCInfo.openCLSpyID() > 0))
        {
            apContextID sharedContext(AP_OPENGL_CONTEXT, RCInfo.sharingContextID());

            if (RCInfo.openCLSpyID() > 0)
            {
                sharedContext._contextType = AP_OPENCL_CONTEXT;
                sharedContext._contextId = RCInfo.openCLSpyID();
            }
            else
            {
                isSharingGL = true;
            }

            vListSharingContexts.push_back(sharedContext);
        }
    }

    // Add the context node to the tree:
    QTreeWidgetItem* renderContextTreeItemId = appendItemToTree(m_pHeaderItem, currentRenderContextName, iconIndex, pItemData);

    // Mark the render context as true if the context is deleted:
    pRenderContextItemData->_isMarkedForDeletion = isDeleted;

    // Add the render context tree item id to the vector of render contexts:
    m_openGLContextsTreeIds.push_back(renderContextTreeItemId);

    // Increment the progress bar:
    afProgressBarWrapper::instance().incrementProgressBar();

    // Initialize objects size:
    gtUInt32 objectsSize = 0;

    // Don't update static buffers for deleted contexts, as they are deleted with the context:
    bool staticBuffersExist = !isDeleted;

    // In OpenGL ES there are no static buffers:
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)

    if (apDoesProjectTypeSupportOpenGLES(gdGDebuggerGlobalVariablesManager::instance().CodeXLProjectType()))
    {
        staticBuffersExist = false;
    }

#endif

    if (staticBuffersExist)
    {
        // Add static buffers to the list:
        bool rcStaticBuff = updateStaticBuffersList(renderContextId, objectsSize, currentCategoryObjectCount);
        GT_IF_WITH_ASSERT(rcStaticBuff)
        {
            pItemData->m_objectCount += currentCategoryObjectCount;
        }

        retVal = rcStaticBuff;
    }

    // If the context is not shared, update its memory objects:
    if (!isSharingGL)
    {
        // Update textures list, if textures are not masked:
        bool rcTex = true, rcRenderBuff = true, rcPipelines = true, rcSamplers = true, rcFBO = true, rcVBO = true, rcProg = true, rcShad = true, rcDisplayList = true;
        rcTex = updateGLTexturesList(renderContextId, objectsSize, currentCategoryObjectCount);
        GT_IF_WITH_ASSERT(rcTex)
        {
            pItemData->m_objectCount += currentCategoryObjectCount;
            pItemData->m_objectMemorySize += objectsSize;
        }

        // Update textures list, if textures are not masked:
        // Update render buffers list:
        rcRenderBuff = updateRenderBuffersList(renderContextId, objectsSize, currentCategoryObjectCount);
        GT_IF_WITH_ASSERT(rcRenderBuff)
        {
            pItemData->m_objectCount += currentCategoryObjectCount;
            pItemData->m_objectMemorySize += objectsSize;
        }

        // Update the program pipeline objects:
        int pipelineObjectsCount = 0;
        rcPipelines = updateProgramPipelinesList(renderContextId, pipelineObjectsCount);
        GT_IF_WITH_ASSERT(rcPipelines)
        {
            pItemData->m_objectCount += pipelineObjectsCount;
        }

        // Update the OpenGL sampler objects:
        int samplerObjectsCount = 0;
        rcSamplers = updateOpenGlSamplersList(renderContextId, samplerObjectsCount);
        GT_IF_WITH_ASSERT(rcSamplers)
        {
            pItemData->m_objectCount += samplerObjectsCount;
        }

        // Update FBOs list:
        rcFBO = updateFBOsList(renderContextId, objectsSize, currentCategoryObjectCount);
        GT_IF_WITH_ASSERT(rcFBO)
        {
            pItemData->m_objectCount += currentCategoryObjectCount;
            pItemData->m_objectMemorySize += objectsSize;
        }

        // Update vbos list:
        rcVBO = updateVBOList(renderContextId, objectsSize, currentCategoryObjectCount);
        GT_IF_WITH_ASSERT(rcVBO)
        {
            pItemData->m_objectCount += currentCategoryObjectCount;
            pItemData->m_objectMemorySize += objectsSize;
        }

        // Update Programs List:
        rcProg = updateProgramsList(renderContextId, objectsSize, currentCategoryObjectCount);
        GT_IF_WITH_ASSERT(rcProg)
        {
            pItemData->m_objectCount += currentCategoryObjectCount;
            pItemData->m_objectMemorySize += objectsSize;
        }

        // Update Shaders list:
        rcShad = updateShadersList(renderContextId, objectsSize, currentCategoryObjectCount);
        GT_IF_WITH_ASSERT(rcShad)
        {
            pItemData->m_objectCount += currentCategoryObjectCount;
            pItemData->m_objectMemorySize += objectsSize;
        }

        // Update display lists list:
        rcDisplayList = updateDisplayListsList(renderContextId, objectsSize, currentCategoryObjectCount);
        GT_IF_WITH_ASSERT(rcDisplayList)
        {
            pItemData->m_objectCount += currentCategoryObjectCount;
            pItemData->m_objectMemorySize += objectsSize;
        }

        retVal = retVal && rcTex && rcRenderBuff && rcVBO && rcProg && rcShad && rcDisplayList;
    }

    objectCount += pItemData->m_objectCount;

    // Call the virtual implementation to update data for the specific viewers:
    updateContextSpecificData(isDeleted, pRenderContextItemData);

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateApplicationOpenCLContexts
// Description: Updates the currently existing OpenCL contexts in the executed application.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/12/2009
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateApplicationOpenCLContexts(unsigned int& objectCount, gtVector<apContextID>& vListSharingContexts)
{
    bool retVal = true;

    // Get the current amount of contexts:
    int currentContextsAmount = 0;
    objectCount = 0;
    bool rc = gaGetAmountOfOpenCLContexts(currentContextsAmount);

    // Do not throw an exception, when there's no OpenCL spy, this function fails:
    if (rc)
    {
        gtString progressMessage;

        // Iterate the contexts, and add for each of them a node to the tree:
        for (int i = 1; i < currentContextsAmount; i++)
        {
            unsigned int currentCategoryObjectCount = 0;

            // Build the context name string;
            gtString currentContextName;
            apContextID contextId(AP_OPENCL_CONTEXT, i);
            gdGetContextNameString(contextId, currentContextName);

            apCLContext contextDetails;
            gaGetOpenCLContextDetails(i,  contextDetails);

            if (contextDetails.openGLSpyID() > 0)
            {
                apContextID sharedContext(AP_OPENGL_CONTEXT, contextDetails.openGLSpyID());
                vListSharingContexts.push_back(sharedContext);
            }

            // Display "Updating RC ###..."
            progressMessage.makeEmpty();
            progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingContext GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, i + 1);
            afProgressBarWrapper::instance().setProgressDetails(progressMessage, 4);

            // Get the context details:
            bool wasContextDeleted = gaWasContextDeleted(apContextID(AP_OPENCL_CONTEXT, i));

            // Create a new memory item data, and fill the relevant fields:
            gdDebugApplicationTreeData* pContextItemData = NULL;
            afApplicationTreeItemData* pItemData = NULL;
            createObjectItemData(pItemData, pContextItemData);

            pItemData->m_itemType = AF_TREE_ITEM_CL_CONTEXT;
            pItemData->m_objectCount = 0;
            pItemData->m_objectMemorySize = 0;

            pContextItemData->_contextId = contextId;
            pContextItemData->_isMarkedForDeletion = wasContextDeleted;
            pContextItemData->_objectOpenCLNameStr = contextDetails.contextName();
            pContextItemData->_referenceCount = contextDetails.referenceCount();

            // Select the icon index:
            int iconIndex = m_computeContextIconIndex;

            if (wasContextDeleted)
            {
                iconIndex = m_computeContextDeletedIconIndex;
            }

            // Add the context node to the tree:
            QTreeWidgetItem* contextTreeItemId = appendItemToTree(m_pHeaderItem, currentContextName, iconIndex, pItemData);

            // Add the render context tree item id to the vector of render contexts:
            m_openCLContextsTreeIds.push_back(contextTreeItemId);

            // Increment the progress bar:
            afProgressBarWrapper::instance().incrementProgressBar();

            gtUInt32 objectsSize = 0;

            // Update the context memory objects:
            bool rcTex = true;
            bool rcBuff = true;
            bool rcPipe = true;
            bool rcCommandQueues = true;
            bool rcPrograms = true;
            bool rcSampler = true;
            bool rcEvents = true;

            // Update textures list:
            rcTex = updateCLImagesList(contextId, objectsSize, currentCategoryObjectCount);
            GT_IF_WITH_ASSERT(rcTex)
            {
                pItemData->m_objectCount += currentCategoryObjectCount;
                pItemData->m_objectMemorySize += objectsSize;
            }

            // Update OpenCL samplers list:
            rcSampler = updateCLSamplersList(contextId, objectsSize, currentCategoryObjectCount);
            GT_IF_WITH_ASSERT(rcSampler)
            {
                pItemData->m_objectCount += currentCategoryObjectCount;
                pItemData->m_objectMemorySize += objectsSize;
            }

            // Update OpenCL events list:
            rcEvents = updateCLEventsList(contextId, objectsSize, currentCategoryObjectCount);
            GT_IF_WITH_ASSERT(rcEvents)
            {
                pItemData->m_objectCount += currentCategoryObjectCount;
                pItemData->m_objectMemorySize += objectsSize;
            }

            // Update OpenCL buffers list:
            rcBuff = updateCLBuffersList(contextId, objectsSize, currentCategoryObjectCount);
            GT_IF_WITH_ASSERT(rcBuff)
            {
                pItemData->m_objectCount += currentCategoryObjectCount;
                pItemData->m_objectMemorySize += objectsSize;
            }

            // Update OpenCL pipes list:
            rcPipe = updateCLPipesList(contextId, objectsSize, currentCategoryObjectCount);
            GT_IF_WITH_ASSERT(rcPipe)
            {
                pItemData->m_objectCount += currentCategoryObjectCount;
                pItemData->m_objectMemorySize += objectsSize;
            }

            // Update OpenCL command queues list:
            rcCommandQueues = updateCLCommandQueuesList(contextId, objectsSize, currentCategoryObjectCount);
            GT_IF_WITH_ASSERT(rcCommandQueues)
            {
                pItemData->m_objectCount += currentCategoryObjectCount;
                pItemData->m_objectMemorySize += objectsSize;
            }

            // Update OpenCL programs list:
            rcPrograms = updateCLProgramsList(contextId, objectsSize, currentCategoryObjectCount);
            GT_IF_WITH_ASSERT(rcPrograms)
            {
                pItemData->m_objectCount += currentCategoryObjectCount;
                pItemData->m_objectMemorySize += objectsSize;
            }

            retVal = retVal && rcTex && rcBuff && rcPipe && rcCommandQueues && rcPrograms && rcEvents && rcSampler;
            objectCount += pItemData->m_objectCount;

            // Update my memory data if requested:
            bool rcUpdate = updateContextMemoryData(wasContextDeleted, pItemData);
            retVal = retVal && rcUpdate;
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateGLTexturesList
// Description: Add the current textures to the memory tree
// Arguments: int contextId - the render context id
//            gtUInt32& texturesMemorySize - the textures memory size (is calculated by this function)
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/9/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateGLTexturesList(int contextId, gtUInt32& texturesMemorySize, unsigned int& textureObjectCount)
{
    bool retVal = true;

    texturesMemorySize = 0;
    textureObjectCount = 0;

    // Get the context tree item id:
    apContextID contextIdentifier(AP_OPENGL_CONTEXT, contextId);
    QTreeWidgetItem* pContextTreeId = getContextTreeItemId(contextIdentifier);

    // Make sure that the tree node is ok:
    GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
    {
        // Get amount of textures for this context:
        int amountOfTextures = 0;
        retVal = gaGetAmountOfTextureObjects(contextId, amountOfTextures);
        GT_IF_WITH_ASSERT(retVal)
        {
            if (amountOfTextures > 0)
            {
                // Add the render context texture node:
                // Create a new memory item data, and fill the relevant fields:
                gdDebugApplicationTreeData* pTexturesNodeData = NULL;
                afApplicationTreeItemData* pItemData = NULL;
                createObjectItemData(pItemData, pTexturesNodeData);

                pItemData->m_itemType = AF_TREE_ITEM_GL_TEXTURES_NODE;
                pItemData->m_objectCount = 0;
                pItemData->m_objectMemorySize = 0;

                pTexturesNodeData->_contextId = contextIdentifier;
                pTexturesNodeData->_objectOpenGLName = contextId;

                // Initialize the map (so that the types are sorted when used):
                for (int textureType = (int)AP_1D_TEXTURE; textureType <= (int)AP_AMOUNT_OF_TEXTURE_BIND_TARGETS; textureType++)
                {
                    pTexturesNodeData->_textureTypesAmount[(apTextureType)textureType] = 0;
                }

                // Display Progress Bar Message:
                gtString progressMessage1;
                progressMessage1.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingContextTextures GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, contextId);
                afProgressBarWrapper::instance().setProgressDetails(progressMessage1, amountOfTextures);

                gtVector<apGLTextureMipLevelID> renderContextTextureNames;
                gtVector<apGLTextureMipLevelID> textureNamesForUpdate;

                // Before adding the textures to the tree, update the texture parameters:
                for (int i = 0; i < amountOfTextures; i++)
                {
                    // Get the current texture name:
                    GLuint textureName = 0;
                    retVal = retVal && gaGetTextureObjectName(contextId, i, textureName);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        // Get the texture type (for texture buffer we do not update parameters):
                        apTextureType textureType = AP_UNKNOWN_TEXTURE_TYPE;
                        bool rc = gaGetTextureObjectType(contextId, i, textureType);
                        GT_IF_WITH_ASSERT(rc)
                        {
                            if (textureType != AP_BUFFER_TEXTURE)
                            {
                                // Add the texture to the textures for update names:
                                apGLTextureMipLevelID textureId;
                                textureId._textureName = textureName;
                                textureId._textureMipLevel = 0;
                                textureNamesForUpdate.push_back(textureId);
                            }

                            apGLTextureMipLevelID textureId;
                            textureId._textureName = textureName;
                            textureId._textureMipLevel = 0;
                            renderContextTextureNames.push_back(textureId);

                            // Add the texture type to the map:
                            pTexturesNodeData->_textureTypesAmount[textureType] ++;
                        }
                    }

                    // Increment the progress bar
                    afProgressBarWrapper::instance().incrementProgressBar();
                }

                // Update texture parameters:
                bool rc = updateTexturesMemoryParameters(contextId, textureNamesForUpdate);
                GT_ASSERT(rc || gaIsInKernelDebugging());

                // Count the textures by type:
                // countTexturesByType(pTexturesNodeData);

                // Add the item to the tree:
                QTreeWidgetItem* pTexturesBaseTreeItemId = appendItemToTree(pContextTreeId, GD_STR_MemoryAnalysisViewerTreeTexturesNode, m_allTexturesIconIndex, pItemData);
                GT_IF_WITH_ASSERT(pTexturesBaseTreeItemId != NULL)
                {
                    // Display Progress Bar Message:
                    gtString progressMessage;
                    progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingContextTextures GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, contextId);
                    afProgressBarWrapper::instance().setProgressDetails(progressMessage, amountOfTextures);

                    // Iterate the textures and add each of them to the tree:
                    int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
                    int itemsToShow = min(amountOfTextures, maxItemsShown);

                    for (int i = 0; i < itemsToShow; i++)
                    {
                        // Get the current texture name:
                        apGLTextureMipLevelID textureID = renderContextTextureNames[i];

                        // Build a texture item data, and fill the texture details:
                        afApplicationTreeItemData* pNewTextureItemData = NULL;
                        int imageIndex = m_textureUnknownIconIndex;

                        // Get the texture object details:
                        apGLTextureMemoryData textureDetails;
                        retVal = gaGetTextureMemoryDataObjectDetails(contextId, textureID._textureName, textureDetails);
                        GT_IF_WITH_ASSERT(retVal)
                        {
                            retVal = buildTextureObjectData(contextId, textureDetails, pNewTextureItemData, imageIndex);
                            GT_IF_WITH_ASSERT(retVal && (pNewTextureItemData != NULL))
                            {
                                // Add the texture to the tree:
                                gtString textureNameStr;

                                // Build the texture name:
                                gdHTMLProperties htmlBuilder;
                                int openCLImageIndex = -1;
                                int openCLImageName = -1;
                                int openCLSpyID = -1;
                                textureDetails.getCLDetails(openCLImageIndex, openCLImageName, openCLSpyID);
                                htmlBuilder.getGLTextureName(textureID, openCLImageName, openCLSpyID, textureNameStr);

                                QTreeWidgetItem* pTextureTreeItemId = appendItemToTree(pTexturesBaseTreeItemId, textureNameStr, imageIndex, pNewTextureItemData);
                                pItemData->m_objectMemorySize += pNewTextureItemData->m_objectMemorySize;

                                // Increment the progress bar
                                afProgressBarWrapper::instance().incrementProgressBar();

                                retVal = retVal && (pTextureTreeItemId != NULL);
                            }
                        }
                    }

                    // If some items were hidden:
                    if (maxItemsShown < amountOfTextures)
                    {
                        // Show a message:
                        addMoreItemsMessage(pTexturesBaseTreeItemId, (amountOfTextures - maxItemsShown));
                    }

                    // Set the amount of texture objects:
                    pItemData->m_objectCount += amountOfTextures;
                }

                // Set the textures memory size:
                texturesMemorySize = pItemData->m_objectMemorySize;
                textureObjectCount = pItemData->m_objectCount;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getContextTreeItemId
// Description: Returns the tree node for contextId, or the root if one such
//              does not exist
// Author:      Uri Shomroni
// Date:        21/3/2010
// ---------------------------------------------------------------------------
QTreeWidgetItem* gdDebugApplicationTreeHandler::getContextTreeItemId(const apContextID& contextId)
{
    QTreeWidgetItem* pRetVal = m_pHeaderItem;

    // Get the context details:
    const int& contextNumber = contextId._contextId;

    // The null context would never appear in our tree, but it is used as a default
    // for some values (e.g. when the tree is empty)
    if (contextId.isValid() && (!contextId.isDefault()))
    {
        if (contextId.isOpenGLContext())
        {
            // See if we have an item for this context:
            int highestRenderContextIndex = (int)m_openGLContextsTreeIds.size();
            bool bValidIndex = (contextNumber > 0) && (contextNumber <= highestRenderContextIndex);
            GT_IF_WITH_ASSERT(bValidIndex)
            {
                // Get the context tree item id:
                pRetVal = m_openGLContextsTreeIds[contextNumber - 1];
            }
        }
        else if (contextId.isOpenCLContext())
        {
            // See if we have an item for this context:
            int highestComputeContextIndex = (int)m_openCLContextsTreeIds.size();
            bool bValidIndex = (contextNumber > 0) && (contextNumber <= highestComputeContextIndex);
            GT_IF_WITH_ASSERT(bValidIndex)
            {
                // Get the context tree item id:
                pRetVal = m_openCLContextsTreeIds[contextNumber - 1];
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::buildTextureObjectData
// Description: Collects a texture object details, and build a memory data object to attach to the tree node
// Arguments: int contextId - the context id
//            GLuint textureName - the texute OpenGL name
//            gdDebugApplicationTreeData*& pTextureItemData - the texture item created data
//            int& imageIndex - the added item index (is calculated according to texture type)
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/9/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::buildTextureObjectData(int contextId, const apGLTextureMemoryData& textureMemoryData, afApplicationTreeItemData*& pItemData, int& imageIndex)
{
    bool retVal = true;

    // Get the texture mip levels count:
    GLuint minLevel = textureMemoryData.minLevel();
    GLuint maxLevel = textureMemoryData.maxLevel();
    int numOfMiplevels = maxLevel - minLevel + 1;

    pItemData = NULL;
    gdDebugApplicationTreeData* pTextureItemData = NULL;
    createObjectItemData(pItemData, pTextureItemData);

    pItemData->m_itemType = AF_TREE_ITEM_GL_TEXTURE;

    // Get the texture type:
    apContextID textureContextId(AP_OPENGL_CONTEXT, contextId);
    imageIndex = textureTypeToIconIndex(textureContextId, textureMemoryData.textureType());

    // Set the OpenGL texture name:
    pTextureItemData->_objectOpenGLName = textureMemoryData.textureName();

    // Set the texture id:
    pTextureItemData->_textureMiplevelID._textureMipLevel = 0;
    pTextureItemData->_textureMiplevelID._textureName = textureMemoryData.textureName();

    // Set the OpenCL image name:
    textureMemoryData.getCLDetails(pTextureItemData->_objectOpenCLIndex, pTextureItemData->_objectOpenCLName, pTextureItemData->_objectCLSpyID);

    // Set the context ID:
    pTextureItemData->_contextId._contextType = AP_OPENGL_CONTEXT;
    pTextureItemData->_contextId._contextId = contextId;

    // Get the texture type:
    pTextureItemData->_textureType = textureMemoryData.textureType();

    // Set the texture mipmap levels:
    pTextureItemData->_minLevel = textureMemoryData.minLevel();
    pTextureItemData->_maxLevel = textureMemoryData.maxLevel();
    pTextureItemData->_isDataCached = false;

    // Get the texture internal format:
    pTextureItemData->_internalFormat = (textureMemoryData.textureType() == AP_BUFFER_TEXTURE) ? textureMemoryData.bufferInternalFormat() : textureMemoryData.usedInternalPixelFormat();
    pTextureItemData->_requestedInternalFormat = textureMemoryData.requestedInternalPixelFormat();

    // Set the OpenCL image index:
    pTextureItemData->_objectOpenCLIndex = textureMemoryData.openCLImageIndex();
    pTextureItemData->_objectOpenCLName = textureMemoryData.openCLImageName();

    apTextureMipMapType mipmapType = textureMemoryData.getTextureMipmapType();

    // Convert the mipmap type to string:
    if (mipmapType == AP_MIPMAP_AUTO_GENERATE)
    {
        pTextureItemData->_mipmapStr = GD_STR_MemoryAnalysisViewerMipMapAuto;
    }
    else if (mipmapType == AP_MIPMAP_NONE_MANUAL)
    {
        pTextureItemData->_mipmapStr = GD_STR_MemoryAnalysisViewerMipMapManual;
    }
    else
    {
        pTextureItemData->_mipmapStr = AF_STR_None;
    }

    // Add the mipmap number of levels:
    if (mipmapType != AP_MIPMAP_NONE && numOfMiplevels > 1)
    {
        // Build the mipmap levels string:
        pTextureItemData->_mipmapStr.appendFormattedString(GD_STR_MemoryAnalysisViewerMipMapLevels, numOfMiplevels);
    }

    // Fill memory size details:
    fillTextureMemoryData(textureMemoryData, pItemData);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::buildStaticBufferObjectData
// Description: Collects a static buffer object details, and build a memory data object to attach to the tree node
// Arguments: int contextId
//            apDisplayBuffer bufferType
//            gdStaticBufferMemoryItemData*& pStaticBufferItemData
//            int& imageIndex
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/10/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::buildStaticBufferObjectData(int contextId, apDisplayBuffer bufferType, afApplicationTreeItemData*& pStaticBufferItemData, int& imageIndex)
{
    bool retVal = false;
    // Get the texture object details:
    apStaticBuffer staticBufferDetails;
    retVal = gaGetStaticBufferObjectDetails(contextId, bufferType, staticBufferDetails);
    GT_IF_WITH_ASSERT(retVal)
    {
        gdDebugApplicationTreeData* pGDStaticBufferItemData = NULL;
        pStaticBufferItemData = NULL;
        createObjectItemData(pStaticBufferItemData, pGDStaticBufferItemData);

        pStaticBufferItemData->m_itemType = AF_TREE_ITEM_GL_STATIC_BUFFER;

        // Set the buffer type:
        pGDStaticBufferItemData->_objectOpenGLName = (GLuint)bufferType;
        pGDStaticBufferItemData->_bufferType = bufferType;

        // Set the image index:
        imageIndex = m_staticBufferIconIndex;

        // Set the render context id;
        pGDStaticBufferItemData->_contextId._contextId = contextId;
        pGDStaticBufferItemData->_contextId._contextType = AP_OPENGL_CONTEXT;

        // Get the buffer's data type:
        oaTexelDataFormat bufferDataFormat = OA_TEXEL_FORMAT_UNKNOWN;
        oaDataType dataType = OA_UNKNOWN_DATA_TYPE;
        staticBufferDetails.getBufferFormat(bufferDataFormat, dataType);

        // Set the buffer data format and data type:
        pGDStaticBufferItemData->_dataFormat = bufferDataFormat;
        pGDStaticBufferItemData->_dataType = dataType;

        // Fill memory size details:
        fillStaticBufferMemoryData(staticBufferDetails, pStaticBufferItemData);

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::buildVBOObjectData
// Description: Collects a vertex buffer object details, and build a memory data object to attach to the tree node
// Arguments: int contextId - render context id
//            int vboIndex - the vbo index
//            gdVBOMemoryItemData*& pVBOItemData - the new vbo allocated data
//            int& imageIndex - vbo image index
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/10/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::buildVBOObjectData(int contextId, int vboIndex, afApplicationTreeItemData*& pVBOItemData, int& imageIndex, apGLVBO& vboDetails)
{
    bool retVal = false;

    // Get the vbo name:
    GLuint vboName = 0;
    retVal = gaGetVBOName(contextId, vboIndex, vboName);
    GT_IF_WITH_ASSERT(retVal)
    {
        // Get the VBO details:
        retVal = gaGetVBODetails(contextId, vboName, vboDetails);
        GT_IF_WITH_ASSERT(retVal)
        {
            gdDebugApplicationTreeData* pGDVBOItemData = NULL;
            pVBOItemData = NULL;
            createObjectItemData(pVBOItemData, pGDVBOItemData);

            // Set the render context id:
            pGDVBOItemData->_contextId._contextType = AP_OPENGL_CONTEXT;
            pGDVBOItemData->_contextId._contextId = contextId;

            // Set the CL buffer index:
            pGDVBOItemData->_objectOpenCLIndex = vboDetails.openCLBufferIndex();
            pGDVBOItemData->_objectOpenCLName = vboDetails.openCLBufferName();

            // Set the vbo name:
            pGDVBOItemData->_objectOpenGLName = vboName;

            // Set item type:
            pVBOItemData->m_itemType = AF_TREE_ITEM_GL_VBO;

            // Set the image index:
            GLenum lastBindTarget = vboDetails.lastBufferTarget();

            switch (lastBindTarget)
            {
                case GL_ARRAY_BUFFER:
                    imageIndex = m_openGLBufferArrayIconIndex;
                    break;

                case GL_DRAW_INDIRECT_BUFFER:
                    imageIndex = m_openGLBufferDrawIndirIconIndex;
                    break;

                case GL_DISPATCH_INDIRECT_BUFFER:
                    imageIndex = m_openGLBufferDispatchIndirIconIndex;
                    break;

                case GL_ELEMENT_ARRAY_BUFFER:
                    imageIndex = m_openGLBufferElementArrayIconIndex;
                    break;

                case GL_PIXEL_PACK_BUFFER:
                    imageIndex = m_openGLBufferPixelPackIconIndex;
                    break;

                case GL_PIXEL_UNPACK_BUFFER:
                    imageIndex = m_openGLBufferPixelUnpackIconIndex;
                    break;

                case GL_COPY_READ_BUFFER:
                    imageIndex = m_openGLBufferCopyReadIconIndex;
                    break;

                case GL_COPY_WRITE_BUFFER:
                    imageIndex = m_openGLBufferCopyWriteIconIndex;
                    break;

                case GL_TRANSFORM_FEEDBACK_BUFFER:
                    imageIndex = m_openGLBufferTransformFeedbackIconIndex;
                    break;

                case GL_UNIFORM_BUFFER:
                    imageIndex = m_openGLBufferUniformIconIndex;
                    break;

                case GL_ATOMIC_COUNTER_BUFFER:
                    imageIndex = m_openGLBufferAtomicIconIndex;
                    break;

                case GL_SHADER_STORAGE_BUFFER:
                    imageIndex = m_openGLBufferShaderStorageIconIndex;
                    break;

                case GL_QUERY_BUFFER:
                    imageIndex = m_openGLBufferQueryIconIndex;
                    break;

                case GL_TEXTURE_BUFFER:
                    imageIndex = m_openGLBufferTextureIconIndex;
                    break;

                case GL_NONE:
                    imageIndex = m_openGLBufferGenericIconIndex;
                    break;

                default:
#if AMDT_BUILD_TARGET == AMDT_DEBUG_BUILD
                    // Unsupported buffer target:
                    GT_ASSERT(false);
#endif
                    imageIndex = m_openGLBufferUnknownIconIndex;
                    break;
            }

            // Fill memory size details:
            fillVBOMemoryData(vboDetails, pVBOItemData);

            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::buildProgramObjectData
// Description:
// Arguments: contextId - what is the current render context
//            programIndex - what is the internal index of the program
//            pProgramItemData - will be filled with the program's data
//            int& imageIndex - what image index to use for the program
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        16/11/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::buildProgramObjectData(int contextId, int programIndex, afApplicationTreeItemData*& pProgramItemData, int& imageIndex)
{
    bool retVal = false;

    // Get the program name:
    GLuint programName = 0;
    retVal = gaGetProgramObjectName(contextId, programIndex, programName);
    GT_IF_WITH_ASSERT(retVal)
    {
        apGLProgram programDetails;
        retVal = gaGetProgramObjectDetails(contextId, programName, programDetails);
        GT_IF_WITH_ASSERT(retVal)
        {
            gdDebugApplicationTreeData* pGDProgramItemData = NULL;
            pProgramItemData = NULL;
            createObjectItemData(pProgramItemData, pGDProgramItemData);

            // Set the render context id:
            pGDProgramItemData->_contextId._contextType = AP_OPENGL_CONTEXT;
            pGDProgramItemData->_contextId._contextId = contextId;

            // Set the vbo name:
            pGDProgramItemData->_objectOpenGLName = programName;

            // Set item type:
            pProgramItemData->m_itemType = AF_TREE_ITEM_GL_PROGRAM;

            // Set the marked for deletion status:
            bool markedForDeletion = programDetails.isMarkedForDeletion();
            pGDProgramItemData->_isMarkedForDeletion = markedForDeletion;

            // Set the image index:
            imageIndex = markedForDeletion ? m_openGLProgramDeletedIconIndex : m_openGLProgramIconIndex;

            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::buildShaderObjectData
// Description:
// Arguments: int contextId
//            int shaderIndex
//            gdShaderMemoryItemData*& pShaderItemData
//            int& imageIndex
//            gtString& shaderNameString
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        16/11/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::buildShaderObjectData(int contextId, int shaderIndex, afApplicationTreeItemData*& pShaderItemData, int& imageIndex, gtString& shaderNameString)
{
    bool retVal = false;

    // Get the shader name:
    GLuint shaderName = 0;
    retVal = gaGetShaderObjectName(contextId, shaderIndex, shaderName);
    GT_IF_WITH_ASSERT(retVal)
    {
        gtAutoPtr<apGLShaderObject> aptrShaderDetails = NULL;
        retVal = gaGetShaderObjectDetails(contextId, shaderName, aptrShaderDetails);
        GT_IF_WITH_ASSERT(retVal && (aptrShaderDetails.pointedObject() != NULL))
        {
            gdDebugApplicationTreeData* pGDShaderItemData = NULL;
            pShaderItemData = NULL;
            createObjectItemData(pShaderItemData, pGDShaderItemData);

            // Set the render context id:
            pGDShaderItemData->_contextId._contextType = AP_OPENGL_CONTEXT;
            pGDShaderItemData->_contextId._contextId = contextId;

            // Set the shader name:
            pGDShaderItemData->_objectOpenGLName = shaderName;

            // Set the "marked for deletion" status:
            bool markedForDeletion = aptrShaderDetails->isMarkedForDeletion();
            pGDShaderItemData->_isMarkedForDeletion = markedForDeletion;

            // Set the image index:
            imageIndex = markedForDeletion ? m_openGLShaderDeletedIconIndex : m_openGLShaderIconIndex;

            // TO_DO: VS GLSL: set the shader's owner program:
            int programName = -1;
            pGDShaderItemData->_objectOwnerName = programName;

            // Set the shader name and type:
            shaderNameString.makeEmpty();
            osTransferableObjectType shaderTObjType = aptrShaderDetails->type();
            afTreeItemType shaderObjectType = AF_TREE_ITEM_ITEM_NONE;
            gdShaderType shadType = gdShaderTypeFromTransferableObjectType(shaderTObjType, shaderObjectType);
            pGDShaderItemData->_shaderType = shadType;
            gdShaderNameStringFromNameAndType(shaderName, shadType, shaderNameString);

            pShaderItemData->m_itemType = shaderObjectType;

            // Fill memory size details:
            fillShaderMemoryData(pShaderItemData, aptrShaderDetails);

            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::buildDisplayListObjectData
// Description: Builds the Itemdata for a display list
// Author:      Uri Shomroni
// Date:        17/11/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::buildDisplayListObjectData(int contextId, int listIndex, afApplicationTreeItemData*& pListItemData, int& imageIndex)
{
    bool retVal = false;

    GLuint listName = 0;
    bool rcName = gaGetDisplayListObjectName(contextId, listIndex, listName);
    GT_IF_WITH_ASSERT(rcName)
    {
        apGLDisplayList displayListDetails;
        bool rcDispList = gaGetDisplayListObjectDetails(contextId, listName, displayListDetails);
        GT_IF_WITH_ASSERT(rcDispList)
        {
            retVal = true;

            // Create the item data:
            gdDebugApplicationTreeData* pGDListItemData = NULL;
            pListItemData = NULL;
            createObjectItemData(pListItemData, pGDListItemData);
            pListItemData->m_itemType = AF_TREE_ITEM_GL_DISPLAY_LIST;

            // Update the display list rendered vertices:
            pGDListItemData->_amountOfRenderedVertices = displayListDetails.amountOfRenderedVertices();

            pGDListItemData->_objectOpenGLName = listName;
            pGDListItemData->_contextId._contextType = AP_OPENGL_CONTEXT;
            pGDListItemData->_contextId._contextId = contextId;

            imageIndex = m_displayListIconIndex;

            // Fill memory size details:
            fillDisplayListMemoryData(displayListDetails, pListItemData);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::buildFBOObjectData
// Description: Builds the item data for an FBO
// Author:      Uri Shomroni
// Date:        17/11/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::buildFBOObjectData(int contextId, int fboIndex, afApplicationTreeItemData*& pFBOItemData, int& imageIndex)
{
    bool retVal = false;

    GLuint fboName = 0;
    bool rcName = gaGetFBOName(contextId, fboIndex, fboName);
    GT_IF_WITH_ASSERT(rcName)
    {
        apGLFBO fboDetails;
        bool rcFBO = gaGetFBODetails(contextId, fboName, fboDetails);
        GT_IF_WITH_ASSERT(rcFBO)
        {
            retVal = true;

            // Create the item data:
            gdDebugApplicationTreeData* pGDFBOItemData = NULL;
            pFBOItemData = NULL;
            createObjectItemData(pFBOItemData, pGDFBOItemData);

            // AF item data:
            pFBOItemData->m_itemType = AF_TREE_ITEM_GL_FBO;
            pFBOItemData->m_objectCount = fboDetails.amountOfBindedObjects();

            // We currently do not calculate FBO sizes:
            pFBOItemData->m_objectMemorySize = 0;

            // GD Item data:
            pGDFBOItemData->_objectOpenGLName = fboName;

            // Set context id:
            pGDFBOItemData->_contextId._contextType = AP_OPENGL_CONTEXT;
            pGDFBOItemData->_contextId._contextId = contextId;

            imageIndex = m_fboIconIndex;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateFBOAttachments
// Description: Add an FBO's attachment to the memory tree
// Arguments:   int contextId
//              int fboIndex
//              int& fboAttachmentsCount
//              QTreeWidgetItem fboTreeItemId
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        14/2/2011
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateFBOAttachments(int contextId, int fboIndex, int& fboAttachmentsCount, QTreeWidgetItem* pFBOTreeItemId)
{
    bool retVal = false;

    // Get the FBO name by its index:
    GLuint fboName = 0;
    bool rcName = gaGetFBOName(contextId, fboIndex, fboName);
    GT_IF_WITH_ASSERT(rcName)
    {
        // Get the FBO details:
        apGLFBO fboDetails;
        bool rcFBO = gaGetFBODetails(contextId, fboName, fboDetails);
        GT_IF_WITH_ASSERT(rcFBO)
        {
            // Get the FBO binded objects:
            gtList<apFBOBindObject*> bindedObjects = fboDetails.getBindedObjects();

            fboAttachmentsCount = (int)bindedObjects.length();

            if (0 < fboAttachmentsCount)
            {
                // Iterate the attachments and add them to the tree:
                gtList<apFBOBindObject*>::const_iterator iter = bindedObjects.begin();
                gtList<apFBOBindObject*>::const_iterator iterEnd = bindedObjects.end();

                for (; iter != iterEnd ; iter++)
                {
                    // Get the current binded object:
                    apFBOBindObject* pBindedObject = (*iter);

                    if (pBindedObject != NULL)
                    {
                        // Allocate an item data for the FBO attachment:
                        gdDebugApplicationTreeData* pBindedObjectData = NULL;
                        afApplicationTreeItemData* pItemData = NULL;
                        createObjectItemData(pItemData, pBindedObjectData);

                        // Fill the binded object data:
                        pItemData->m_itemType = AF_TREE_ITEM_GL_FBO_ATTACHMENT;
                        pBindedObjectData->_bufferAttachmentTarget = pBindedObject->_attachmentTarget;
                        pBindedObjectData->_bufferAttachmentPoint = pBindedObject->_attachmentPoint;
                        pBindedObjectData->_objectOpenGLName = fboName;
                        pBindedObjectData->_contextId._contextId = contextId;
                        pBindedObjectData->_contextId._contextType = AP_OPENGL_CONTEXT;

                        bool isTextureAttachment = false;
                        bool rc = apGLFBO::isTextureAttachmentTarget(pBindedObject->_attachmentTarget, isTextureAttachment);
                        GT_IF_WITH_ASSERT(rc)
                        {
                            // Find the item that contain the original item for this attachment:
                            afApplicationTreeItemData originalItemID;
                            gdDebugApplicationTreeData* pOriginalItemID = new gdDebugApplicationTreeData;
                            pOriginalItemID->_contextId._contextId = contextId;
                            pOriginalItemID->_contextId._contextType = AP_OPENGL_CONTEXT;
                            pOriginalItemID->_objectOpenGLName = pBindedObject->_name;
                            originalItemID.setExtendedData(pOriginalItemID);

                            // Set the shortcut item type:
                            originalItemID.m_itemType = isTextureAttachment ? AF_TREE_ITEM_GL_TEXTURE : AF_TREE_ITEM_GL_RENDER_BUFFER;

                            // Find the original item on the tree:
                            afApplicationTreeItemData* pOriginalItemData = getOpenGLObjectDataByName(originalItemID);
                            GT_IF_WITH_ASSERT(pOriginalItemData != NULL)
                            {
                                pItemData->m_pOriginalItemTreeItem = pOriginalItemData->m_pTreeWidgetItem;
                            }

                            // Build the buffer attachment string:
                            gtString attachmentTargetStr;
                            gtString bindedObjectStr;
                            bool rc2 = apGLFBO::fboAttachmentTargetToString(pBindedObject->_attachmentTarget, attachmentTargetStr);
                            GT_ASSERT(rc2);

                            // Build the binded object string:
                            bindedObjectStr.appendFormattedString(L"%ls %u", attachmentTargetStr.asCharArray(), pBindedObject->_name);

                            // Get the icon according to the attachment type:
                            int iconIndex = isTextureAttachment ? m_texturesShortcutIconIndex : m_renderBufferShortcutIconIndex;

                            // Add the item to the tree:
                            appendItemToTree(pFBOTreeItemId, bindedObjectStr, iconIndex, pItemData);

                            retVal = true;
                        }
                    }
                }
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::buildRenderBufferObjectData
// Description: Collects a render buffer object details, and build a memory data object to attach to the tree node
// Arguments: int contextId - the context id
//            GLuint renderBufferName - the render buffers OpenGL name
//            gdDebugApplicationTreeData*& pRenderBufferItemData - the render buffer item created data
//            int& imageIndex - the added item index (is calculated according to texture type)
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/9/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::buildRenderBufferObjectData(int contextId, GLuint renderBufferName, afApplicationTreeItemData*& pItemData, int& imageIndex)
{
    bool retVal = false;

    // Get the render buffer object details:
    apGLRenderBuffer renderBufferDetails(0);
    retVal = gaGetRenderBufferObjectDetails(contextId, renderBufferName, renderBufferDetails);

    // Create the item data:
    gdDebugApplicationTreeData* pRenderBufferItemData = NULL;
    pItemData = NULL;
    createObjectItemData(pItemData, pRenderBufferItemData);

    pItemData->m_itemType = AF_TREE_ITEM_GL_RENDER_BUFFER;

    imageIndex = m_renderBufferIconIndex;

    // Set the render buffer's OpenGL name:
    pRenderBufferItemData->_objectOpenGLName = renderBufferName;

    // Set the render context id;
    pRenderBufferItemData->_contextId._contextType = AP_OPENGL_CONTEXT;
    pRenderBufferItemData->_contextId._contextId = contextId;

    // Set the OpenCL object index:
    pRenderBufferItemData->_objectOpenCLIndex = renderBufferDetails.openCLImageIndex();
    pRenderBufferItemData->_objectOpenCLName = renderBufferDetails.openCLImageName();

    // Get the buffer's data type:
    oaDataType pixelFormat = OA_UNKNOWN_DATA_TYPE;
    pixelFormat = renderBufferDetails.bufferDataType();

    oaTexelDataFormat bufferDataFormat = renderBufferDetails.bufferFormat();
    pRenderBufferItemData->_dataFormat = bufferDataFormat;
    pRenderBufferItemData->_dataType = pixelFormat;

    // Get the buffer attachment:
    // Get the FBO object in order to search for the render buffer within the FBO attachments:
    apGLFBO fboDetails;
    gaGetFBODetails(contextId, renderBufferDetails.getFBOName(), fboDetails);

    // Search within the binded objects, and find this render buffer:
    gtList <apFBOBindObject*> bindedObjects = fboDetails.getBindedObjects();
    gtList <apFBOBindObject*>::const_iterator iter;
    gtList <apFBOBindObject*>::const_iterator iterEnd = bindedObjects.end();
    apDisplayBuffer bufferType = AP_DISPLAY_BUFFER_UNKNOWN;

    for (iter = bindedObjects.begin(); iter != iterEnd; ++iter)
    {
        apFBOBindObject* pCurrent = *iter;
        GT_IF_WITH_ASSERT(pCurrent != NULL)
        {
            if ((pCurrent->_name == renderBufferName) && (pCurrent->_attachmentTarget == GL_RENDERBUFFER_EXT))
            {
                bufferType = apGLEnumToColorIndexBufferType(pCurrent->_attachmentPoint);
                break;
            }
        }
    }

    // Set the buffer attachment:
    pRenderBufferItemData->_bufferType = bufferType;

    // Fill memory size details:
    fillRenderBufferMemoryData(renderBufferDetails, pItemData);

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateRenderBuffersList
// Description: Add the current render buffers to the memory tree
// Arguments:  int contextId - the render context id
//             gtUInt32& renderBuffersMemorySize - the render buffers size
//             (calculated by this function)
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/9/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateRenderBuffersList(int contextId, gtUInt32& renderBuffersMemorySize, unsigned int& renderBufferObjectCount)
{
    bool retVal = true;

    renderBuffersMemorySize = 0;
    renderBufferObjectCount = 0;

    // Get the context tree item id:
    apContextID contextIdentifier(AP_OPENGL_CONTEXT, contextId);
    QTreeWidgetItem* pContextTreeId = getContextTreeItemId(contextIdentifier);

    // Make sure that the tree node is ok:
    GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
    {
        // Get amount of render buffers for this context:
        int amountOfBuffers = 0;
        retVal = gaGetAmountOfRenderBufferObjects(contextId, amountOfBuffers);
        GT_IF_WITH_ASSERT(retVal)
        {
            if (amountOfBuffers > 0)
            {
                // Display Progress Bar Message:
                gtString progressMessage;
                progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingContextRenderBuffers GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, contextId);
                afProgressBarWrapper::instance().setProgressDetails(progressMessage, amountOfBuffers);

                // Create a new memory item data, and fill the relevant fields:
                gdDebugApplicationTreeData* pRenderBufferNodeData = NULL;
                afApplicationTreeItemData* pItemData = NULL;
                createObjectItemData(pItemData, pRenderBufferNodeData);
                pItemData->m_itemType = AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE;
                pItemData->m_objectCount = 0;
                pItemData->m_objectMemorySize = 0;

                pRenderBufferNodeData->_contextId = contextIdentifier;
                pRenderBufferNodeData->_objectOpenGLName = contextId;

                QTreeWidgetItem* pRenderBuffersBaseTreeItemId = appendItemToTree(pContextTreeId, GD_STR_MemoryAnalysisViewerTreeRenderBuffersNode, m_renderBufferIconIndex, pItemData);
                GT_IF_WITH_ASSERT(pRenderBuffersBaseTreeItemId != NULL)
                {
                    // Iterate the buffers and add each of them to the tree:
                    int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
                    int itemsToShow = min(amountOfBuffers, maxItemsShown);

                    for (int i = 0; i < itemsToShow; i++)
                    {
                        // Get the current render buffer name:
                        GLuint renderBufferName = 0;
                        retVal  = retVal && gaGetRenderBufferObjectName(contextId, i, renderBufferName);
                        GT_IF_WITH_ASSERT(retVal)
                        {
                            afApplicationTreeItemData* pNewRenderBufferItemData = NULL;
                            int imageIndex = m_renderBufferIconIndex;
                            retVal = retVal && buildRenderBufferObjectData(contextId, renderBufferName, pNewRenderBufferItemData, imageIndex);
                            GT_IF_WITH_ASSERT(retVal && (pNewRenderBufferItemData != NULL))
                            {
                                // Add the render buffer to the tree:
                                gtString renderBufferNameStr;
                                renderBufferNameStr.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeRenderBufferName, renderBufferName);
                                QTreeWidgetItem* pRenderBufferTreeItemId = appendItemToTree(pRenderBuffersBaseTreeItemId, renderBufferNameStr.asCharArray(), imageIndex, pNewRenderBufferItemData);
                                pItemData->m_objectMemorySize += pNewRenderBufferItemData->m_objectMemorySize;

                                // Increment the progress bar
                                afProgressBarWrapper::instance().incrementProgressBar();

                                retVal = retVal && (pRenderBufferTreeItemId != NULL);
                            }
                        }
                    }

                    // If some items were hidden:
                    if (maxItemsShown < amountOfBuffers)
                    {
                        // Show a message:
                        addMoreItemsMessage(pRenderBuffersBaseTreeItemId, (amountOfBuffers - maxItemsShown));
                    }

                    pItemData->m_objectCount += amountOfBuffers;
                    renderBuffersMemorySize = pItemData->m_objectMemorySize;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateDisplayListsList
// Description: Add the current display lists to the memory tree
// Arguments: int contextId - the render context id
//            gtUInt32& displayListSize - the display list size (calculated by this function)
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/9/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateDisplayListsList(int contextId, gtUInt32& displayListSize, unsigned int& displayListObjectCount)
{
    bool retVal = true;

    displayListSize = 0;
    displayListObjectCount = 0;

    // Get the context tree item id:
    apContextID contextIdentifier(AP_OPENGL_CONTEXT, contextId);
    QTreeWidgetItem* pContextTreeId = getContextTreeItemId(contextIdentifier);

    // Make sure that the tree node is ok:
    GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
    {
        int amountOfDisplayLists = 0;
        bool rcAmount = gaGetAmountOfDisplayLists(contextId, amountOfDisplayLists);
        GT_IF_WITH_ASSERT(rcAmount)
        {
            if (amountOfDisplayLists > 0)
            {
                // Display Progress Bar Message:
                gtString progressMessage;
                progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingContextDisplayLists GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, contextId);
                afProgressBarWrapper::instance().setProgressDetails(progressMessage, amountOfDisplayLists);

                // Create a new memory item data, and fill the relevant fields:
                gdDebugApplicationTreeData* pDisplayListsNodeData = NULL;
                afApplicationTreeItemData* pDisplayListsBaseItemData = NULL;
                createObjectItemData(pDisplayListsBaseItemData, pDisplayListsNodeData);
                pDisplayListsBaseItemData->m_itemType = AF_TREE_ITEM_GL_DISPLAY_LISTS_NODE;
                pDisplayListsBaseItemData->m_objectMemorySize = 0;

                pDisplayListsNodeData->_contextId = contextIdentifier;
                pDisplayListsBaseItemData->m_objectCount = 0;

                QTreeWidgetItem* pDisplayListsBaseTreeItemId = appendItemToTree(pContextTreeId, GD_STR_MemoryAnalysisViewerTreeDisplaysListsNode, m_displayListIconIndex, pDisplayListsBaseItemData);
                GT_IF_WITH_ASSERT(pDisplayListsBaseTreeItemId != NULL)
                {
                    // Iterate the lists and add them to the tree:
                    int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
                    int itemsToShow = min(amountOfDisplayLists, maxItemsShown);

                    for (int i = 0; i < itemsToShow; i++)
                    {
                        afApplicationTreeItemData* pNewDisplayListItemData = NULL;
                        int imageIndex = m_displayListIconIndex;
                        bool rcItemData = buildDisplayListObjectData(contextId, i, pNewDisplayListItemData, imageIndex);
                        GT_IF_WITH_ASSERT(rcItemData && (pNewDisplayListItemData != NULL))
                        {
                            gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pNewDisplayListItemData->extendedItemData());
                            GT_IF_WITH_ASSERT(pGDData != NULL)
                            {
                                gtString displayListNameString;
                                displayListNameString.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeDisplayListName, pGDData->_objectOpenGLName);

                                QTreeWidgetItem* pDisplayListTreeItemId = appendItemToTree(pDisplayListsBaseTreeItemId, displayListNameString.asCharArray(), imageIndex, pNewDisplayListItemData);
                                retVal = retVal && (pDisplayListTreeItemId != NULL);

                                // Add the new display list to the display lists total memory size:
                                pDisplayListsBaseItemData->m_objectMemorySize += pNewDisplayListItemData->m_objectMemorySize;

                                // Increase the amount of objects:
                                pDisplayListsBaseItemData->m_objectCount++;

                                // Increment the progress bar
                                afProgressBarWrapper::instance().incrementProgressBar();
                            }
                        }
                    }

                    // If some items were hidden:
                    if (maxItemsShown < amountOfDisplayLists)
                    {
                        // Show a message and count them in the parent (don't calculate their memory size to avoid slowdown and save communication channel transactions):
                        addMoreItemsMessage(pDisplayListsBaseTreeItemId, (amountOfDisplayLists - maxItemsShown));
                        pDisplayListsBaseItemData->m_objectCount = amountOfDisplayLists;
                    }

                    displayListSize = pDisplayListsBaseItemData->m_objectMemorySize;
                    displayListObjectCount = pDisplayListsBaseItemData->m_objectCount;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateFBOsList
// Description: Add the current context FBOs to the tree
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        17/11/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateFBOsList(int contextId, gtUInt32& fbosMemorySize, unsigned int& fboCount)
{
    bool retVal = true;
    fbosMemorySize = 0;
    fboCount = 0;

    fbosMemorySize = 0;

    // Get the context tree item id:
    apContextID contextIdentifier(AP_OPENGL_CONTEXT, contextId);
    QTreeWidgetItem* pContextTreeId = getContextTreeItemId(contextIdentifier);

    // Make sure that the tree node is ok:
    GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
    {
        int amountOfFBOs = 0;
        bool rcAmount = gaGetAmountOfFBOs(contextId, amountOfFBOs);
        GT_IF_WITH_ASSERT(rcAmount)
        {
            if (amountOfFBOs > 0)
            {
                // Display Progress Bar Message:
                gtString progressMessage;
                progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingContextFBOs GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, contextId);
                afProgressBarWrapper::instance().setProgressDetails(progressMessage, amountOfFBOs);

                // Create a new memory item data, and fill the relevant fields:
                gdDebugApplicationTreeData* pFBOsNodeData = NULL;
                afApplicationTreeItemData* pItemData = NULL;
                createObjectItemData(pItemData, pFBOsNodeData);

                pItemData->m_itemType = AF_TREE_ITEM_GL_FBO_NODE;
                pItemData->m_objectMemorySize = 0;
                pItemData->m_objectCount = 0;

                pFBOsNodeData->_contextId = contextIdentifier;
                pFBOsNodeData->_objectOpenGLName = contextId;

                QTreeWidgetItem* pFbosBaseTreeItemId = appendItemToTree(pContextTreeId, GD_STR_MemoryAnalysisViewerTreeFBOsNode, m_fboIconIndex, pItemData);
                GT_IF_WITH_ASSERT(pFbosBaseTreeItemId != NULL)
                {
                    // Iterate the FBOs and add them to the tree:
                    int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
                    int itemsToShow = min(amountOfFBOs, maxItemsShown);

                    for (int i = 0; i < itemsToShow; i++)
                    {
                        afApplicationTreeItemData* pFBOItemData = NULL;
                        int imageIndex = m_fboIconIndex;
                        bool rcItemData = buildFBOObjectData(contextId, i, pFBOItemData, imageIndex);
                        GT_IF_WITH_ASSERT(rcItemData && (pFBOItemData != NULL))
                        {
                            gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pFBOItemData->extendedItemData());
                            GT_IF_WITH_ASSERT(pGDData != NULL)
                            {
                                gtString fboNameString;
                                fboNameString.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeFBOName, pGDData->_objectOpenGLName);

                                QTreeWidgetItem* pFboTreeItemId = appendItemToTree(pFbosBaseTreeItemId, fboNameString.asCharArray(), imageIndex, pFBOItemData);
                                retVal = retVal && (pFboTreeItemId != NULL);

                                pItemData->m_objectMemorySize += pFBOItemData->m_objectMemorySize;

                                // Increase the amount of objects:
                                pItemData->m_objectCount ++;

                                // Add the FBO's attachments to the tree:
                                bool rcAddAttach = updateFBOAttachments(contextId, i, pFBOItemData->m_objectCount, pFboTreeItemId);
                                retVal = retVal && rcAddAttach;

                                // Increment the progress bar
                                afProgressBarWrapper::instance().incrementProgressBar();
                            }
                        }
                    }

                    // If some items were hidden:
                    if (maxItemsShown < amountOfFBOs)
                    {
                        // Show a message and count them in the parent (don't calculate their memory size to avoid slowdown and save communication channel transactions):
                        addMoreItemsMessage(pFbosBaseTreeItemId, (amountOfFBOs - maxItemsShown));
                        pItemData->m_objectCount = amountOfFBOs;
                    }
                }
                fbosMemorySize = pItemData->m_objectMemorySize;
                fboCount = pItemData->m_objectCount;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateStaticBuffersList
// Description: Add the current static buffers to the memory tree
// Return Val: bool  - Success / failure.
// Arguments: int contextId
//            gtUInt32& staticBuffersMemorySize (calculated by this function)
// Author:      Sigal Algranaty
// Date:        23/9/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateStaticBuffersList(int contextId, gtUInt32& staticBuffersMemorySize, unsigned int& staticBufferObjectCount)
{
    bool retVal = true;

    staticBuffersMemorySize = 0;
    staticBufferObjectCount = 0;

    // Get the context tree item id:
    apContextID contextIdentifier(AP_OPENGL_CONTEXT, contextId);
    QTreeWidgetItem* pContextTreeId = getContextTreeItemId(contextIdentifier);

    // Make sure that the tree node is ok:
    GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
    {
        // Check the debugged process status:
        bool isDebuggedProcessBeingTerminated = gaIsDuringDebuggedProcessTermination();
        bool isInKernelDebugging = gaIsInKernelDebugging();
        bool isHostDebugging = gaIsHostBreakPoint();

        if (!isDebuggedProcessBeingTerminated && !isInKernelDebugging && !isHostDebugging)
        {
            // Update all static buffers dimension according to current HDC dimensions:
            bool rcUpdateDimensions = gaUpdateStaticBuffersDimension(contextId);
            GT_ASSERT(rcUpdateDimensions);
        }

        // Get amount of static buffers for this context:
        int amountOfStaticBuffers;
        retVal = gaGetAmountOfStaticBuffersObjects(contextId, amountOfStaticBuffers);
        GT_IF_WITH_ASSERT(retVal)
        {
            if (amountOfStaticBuffers > 0)
            {
                // Display Progress Bar Message:
                gtString progressMessage;
                progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingContextStaticBuffers GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, contextId);
                afProgressBarWrapper::instance().setProgressDetails(progressMessage, amountOfStaticBuffers);

                // Create a new memory item data, and fill the relevant fields:
                gdDebugApplicationTreeData* pStaticBufferNodeData = NULL;
                afApplicationTreeItemData* pItemData = NULL;
                createObjectItemData(pItemData, pStaticBufferNodeData);
                pItemData->m_itemType = AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE;
                pItemData->m_objectCount = 0;
                pItemData->m_objectMemorySize = 0;

                pStaticBufferNodeData->_contextId = contextIdentifier;
                pStaticBufferNodeData->_objectOpenGLName = contextId;

                QTreeWidgetItem* pStaticBuffersBaseTreeItemId = appendItemToTree(pContextTreeId, GD_STR_MemoryAnalysisViewerTreeStaticBuffersNode, m_staticBufferIconIndex, pItemData);
                GT_IF_WITH_ASSERT(pStaticBuffersBaseTreeItemId != NULL)
                {
                    // Iterate the static buffers and add each of them to the tree:
                    int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
                    int itemsToShow = min(amountOfStaticBuffers, maxItemsShown);

                    for (int i = 0; i < itemsToShow; i++)
                    {
                        // Get the current render buffer name:
                        apDisplayBuffer bufferType = AP_DISPLAY_BUFFER_UNKNOWN;
                        retVal = retVal && gaGetStaticBufferType(contextId, i, bufferType);
                        GT_IF_WITH_ASSERT(retVal)
                        {
                            gtString bufferName;
                            retVal = retVal && apGetBufferName(bufferType, bufferName);
                            GT_IF_WITH_ASSERT(retVal)
                            {
                                afApplicationTreeItemData* pNewStaticBufferItemData = NULL;
                                int imageIndex = m_staticBufferIconIndex;
                                retVal = buildStaticBufferObjectData(contextId, bufferType, pNewStaticBufferItemData, imageIndex) && retVal;
                                GT_IF_WITH_ASSERT(retVal && (pNewStaticBufferItemData != NULL))
                                {
                                    // Add the static buffer to the tree:
                                    QTreeWidgetItem* pStaticBufferTreeItemId = appendItemToTree(pStaticBuffersBaseTreeItemId, bufferName.asCharArray(), imageIndex, pNewStaticBufferItemData);
                                    pItemData->m_objectMemorySize += pNewStaticBufferItemData->m_objectMemorySize;

                                    // Increment the progress bar
                                    afProgressBarWrapper::instance().incrementProgressBar();

                                    retVal = retVal && (pStaticBufferTreeItemId != NULL);
                                }
                            }
                        }
                    }

                    // If some items were hidden:
                    if (maxItemsShown < amountOfStaticBuffers)
                    {
                        // Show a message:
                        addMoreItemsMessage(pStaticBuffersBaseTreeItemId, (amountOfStaticBuffers - maxItemsShown));
                    }

                    pItemData->m_objectCount = amountOfStaticBuffers;
                }

                // Set the static buffers size:
                staticBuffersMemorySize = pItemData->m_objectMemorySize;
                staticBufferObjectCount = pItemData->m_objectCount;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateVBOList
// Description: Add the current vertex buffer object to the memory tree
// Return Val: bool  - Success / failure.
// Arguments: int contextId
//            gtUInt32& vboMemorySize (calculated by this function)
// Author:      Sigal Algranaty
// Date:        23/9/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateVBOList(int contextId, gtUInt32& vboMemorySize, unsigned int& vboCount)
{
    bool retVal = true;

    vboMemorySize = 0;
    vboCount = 0;

    // Get the context tree item id:
    apContextID contextIdentifier(AP_OPENGL_CONTEXT, contextId);
    QTreeWidgetItem* pContextTreeId = getContextTreeItemId(contextIdentifier);

    // Make sure that the tree node is ok:
    GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
    {
        // Get amount of static buffers for this context:
        int amountOfVBO = 0;
        retVal = gaGetAmountOfVBOs(contextId, amountOfVBO);
        GT_IF_WITH_ASSERT(retVal)
        {
            // If there are VBOs, add the render context VBOs node:
            if (amountOfVBO > 0)
            {
                // Display Progress Bar Message:
                gtString progressMessage;
                progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingContextVBOs GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, contextId);
                afProgressBarWrapper::instance().setProgressDetails(progressMessage, amountOfVBO);

                // Create a new memory item data, and fill the relevant fields:
                gdDebugApplicationTreeData* pVBONodeData = NULL;
                afApplicationTreeItemData* pItemData = NULL;
                createObjectItemData(pItemData, pVBONodeData);
                pItemData->m_itemType = AF_TREE_ITEM_GL_VBO_NODE;
                pItemData->m_objectCount = 0;
                pItemData->m_objectMemorySize = 0;

                pVBONodeData->_contextId = contextIdentifier;
                pVBONodeData->_objectOpenGLName = contextId;

                QTreeWidgetItem* pVBOBaseTreeItemId = appendItemToTree(pContextTreeId, GD_STR_MemoryAnalysisViewerTreeVBOsNode, m_openGLBufferGenericIconIndex, pItemData);
                GT_IF_WITH_ASSERT(pVBOBaseTreeItemId != NULL)
                {
                    // Iterate the VBOs and add each of them to the tree:
                    int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
                    int itemsToShow = min(amountOfVBO, maxItemsShown);

                    for (int i = 0; i < itemsToShow; i++)
                    {
                        apGLVBO vboDetails;
                        afApplicationTreeItemData* pNewVBOItemData = NULL;
                        int imageIndex = m_openGLBufferUnknownIconIndex;
                        bool rcBuildVBOData = buildVBOObjectData(contextId, i, pNewVBOItemData, imageIndex, vboDetails);
                        GT_IF_WITH_ASSERT(rcBuildVBOData && (pNewVBOItemData != NULL))
                        {
                            retVal = true;

                            // Build the VBO name string:
                            gtString vboName;
                            gdGetVBODisplayName(vboDetails, vboName);

                            // Add the VBO to the tree:
                            QTreeWidgetItem* pVBOTreeItemId = appendItemToTree(pVBOBaseTreeItemId, vboName.asCharArray(), imageIndex, pNewVBOItemData);

                            // Add the new VBO buffer to the buffers total memory size:
                            pItemData->m_objectMemorySize += pNewVBOItemData->m_objectMemorySize;

                            // Increase the amount of objects:
                            pItemData->m_objectCount ++;

                            // Increment the progress bar
                            afProgressBarWrapper::instance().incrementProgressBar();

                            retVal = retVal && (pVBOTreeItemId != NULL);

                        }
                    }

                    // If some items were hidden:
                    if (maxItemsShown < amountOfVBO)
                    {
                        // Show a message and count them in the parent (don't calculate their memory size to avoid slowdown and save communication channel transactions):
                        addMoreItemsMessage(pVBOBaseTreeItemId, (amountOfVBO - maxItemsShown));
                        pItemData->m_objectCount = amountOfVBO;
                    }
                }

                // Set the vbos size:
                vboMemorySize = pItemData->m_objectMemorySize;
                vboCount = pItemData->m_objectCount;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateProgramsList
// Description: Add the current context's shading programs node to the memory
//              tree and outputs the total size
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        16/11/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateProgramsList(int contextId, gtUInt32& programsMemorySize, unsigned int& programObjectCount)
{
    bool retVal = true;

    programsMemorySize = 0;
    programObjectCount = 0;

    // Get the context tree item id:
    apContextID contextIdentifier(AP_OPENGL_CONTEXT, contextId);
    QTreeWidgetItem* pContextTreeId = getContextTreeItemId(contextIdentifier);

    // Make sure that the tree node is ok:
    GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
    {
        // Get amount of shading programs for this context:
        int amountOfPrograms = 0;
        retVal = gaGetAmountOfProgramObjects(contextId, amountOfPrograms);
        GT_IF_WITH_ASSERT(retVal)
        {
            if (amountOfPrograms > 0)
            {
                // Display Progress Bar Message:
                gtString progressMessage;
                progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingContextGLPrograms GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, contextId);
                afProgressBarWrapper::instance().setProgressDetails(progressMessage, amountOfPrograms);

                // Add the render context programs node:
                // Create a new memory item data, and fill the relevant fields:
                gdDebugApplicationTreeData* pProgramNodeData = NULL;
                afApplicationTreeItemData* pItemData = NULL;
                createObjectItemData(pItemData, pProgramNodeData);

                pItemData->m_itemType = AF_TREE_ITEM_GL_PROGRAMS_NODE;
                pItemData->m_objectCount = 0;
                pItemData->m_objectMemorySize = 0;

                pProgramNodeData->_contextId = contextIdentifier;

                QTreeWidgetItem* pProgramBaseTreeItemId = appendItemToTree(pContextTreeId, GD_STR_MemoryAnalysisViewerTreeGLProgramsNode, m_openGLProgramIconIndex, pItemData);
                GT_IF_WITH_ASSERT(pProgramBaseTreeItemId != NULL)
                {
                    // Iterate the programs and add each of them to the tree:
                    int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
                    int itemsToShow = min(amountOfPrograms, maxItemsShown);

                    for (int i = 0; i < itemsToShow; i++)
                    {
                        afApplicationTreeItemData* pNewProgramItemData = NULL;
                        int imageIndex = m_openGLProgramIconIndex;
                        retVal = retVal && buildProgramObjectData(contextId, i, pNewProgramItemData, imageIndex);
                        GT_IF_WITH_ASSERT(retVal && (pNewProgramItemData != NULL))
                        {
                            gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pNewProgramItemData->extendedItemData());
                            GT_IF_WITH_ASSERT(pGDData != NULL)
                            {
                                // Build the program name string:
                                gtString programName;
                                programName.appendFormattedString(GD_STR_PropertiesProgramNameFormat, pGDData->_objectOpenGLName);

                                // Add the program to the tree:
                                QTreeWidgetItem* pProgramTreeItemId = appendItemToTree(pProgramBaseTreeItemId, programName.asCharArray(), imageIndex, pNewProgramItemData);
                                retVal = retVal && (pProgramTreeItemId != NULL);

                                // Add the new progrma to the programs total memory size:
                                pItemData->m_objectMemorySize += pNewProgramItemData->m_objectMemorySize;

                                // Increase the amount of objects:
                                pItemData->m_objectCount ++;

                                // Increment the progress bar
                                afProgressBarWrapper::instance().incrementProgressBar();
                            }
                        }
                    }

                    // If some items were hidden:
                    if (maxItemsShown < amountOfPrograms)
                    {
                        // Show a message and count them in the parent (don't calculate their memory size to avoid slowdown and save communication channel transactions):
                        addMoreItemsMessage(pProgramBaseTreeItemId, (amountOfPrograms - maxItemsShown));
                        pItemData->m_objectCount = amountOfPrograms;
                    }
                }

                // Set the programs size:
                programsMemorySize = pItemData->m_objectMemorySize;
                programObjectCount = pItemData->m_objectCount;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateShadersList
// Description: Add the current context's shaders node to the memory tree and
//              outputs the total size
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        16/11/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateShadersList(int contextId, gtUInt32& shadersMemorySize, unsigned int& shaderObjectCount)
{
    bool retVal = true;

    shadersMemorySize = 0;
    shaderObjectCount = 0;

    // Get the context tree item id:
    apContextID contextIdentifier(AP_OPENGL_CONTEXT, contextId);
    QTreeWidgetItem* pContextTreeId = getContextTreeItemId(contextIdentifier);

    // Make sure that the tree node is ok:
    GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
    {
        // Get amount of shaders for this context:
        int amountOfShaders = 0;
        retVal = gaGetAmountOfShaderObjects(contextId, amountOfShaders);
        GT_IF_WITH_ASSERT(retVal)
        {
            if (amountOfShaders > 0)
            {
                // Display Progress Bar Message:
                gtString progressMessage;
                progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingContextShaders GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, contextId);
                afProgressBarWrapper::instance().setProgressDetails(progressMessage, amountOfShaders);

                // Add the render context shaders node:
                // Create a new memory item data, and fill the relevant fields:
                gdDebugApplicationTreeData* pShaderNodeData = NULL;
                afApplicationTreeItemData* pItemData = NULL;
                createObjectItemData(pItemData, pShaderNodeData);
                pItemData->m_itemType = AF_TREE_ITEM_GL_SHADERS_NODE;
                pItemData->m_objectCount = 0;
                pItemData->m_objectMemorySize = 0;

                pShaderNodeData->_contextId = contextIdentifier;

                // Count the shaders by type:
                countShadersByType(pShaderNodeData);

                QTreeWidgetItem* pShaderBaseTreeItemId = appendItemToTree(pContextTreeId, GD_STR_MemoryAnalysisViewerTreeShadersNode, m_openGLShaderIconIndex, pItemData);
                GT_IF_WITH_ASSERT(pShaderBaseTreeItemId != NULL)
                {
                    // Iterate the shaders and add each of them to the tree:
                    int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
                    int itemsToShow = min(amountOfShaders, maxItemsShown);

                    for (int i = 0; i < itemsToShow; i++)
                    {
                        gtString shaderName;
                        afApplicationTreeItemData* pNewShaderItemData = NULL;
                        int imageIndex = m_openGLShaderIconIndex;
                        retVal = retVal && buildShaderObjectData(contextId, i, pNewShaderItemData, imageIndex, shaderName);
                        GT_IF_WITH_ASSERT(retVal && (pNewShaderItemData != NULL))
                        {
                            // Add the shader to the tree:
                            QTreeWidgetItem* pShaderTreeItemId = appendItemToTree(pShaderBaseTreeItemId, shaderName.asCharArray(), imageIndex, pNewShaderItemData);
                            retVal = retVal && (pShaderTreeItemId != NULL);

                            // Add the new shader to the shaders total memory size:
                            pItemData->m_objectMemorySize += pNewShaderItemData->m_objectMemorySize;

                            // Increase the amount of objects:
                            pItemData->m_objectCount ++;

                            // Increment the progress bar
                            afProgressBarWrapper::instance().incrementProgressBar();
                        }
                    }

                    // If some items were hidden:
                    if (maxItemsShown < amountOfShaders)
                    {
                        // Show a message and count them in the parent (don't calculate their memory size to avoid slowdown and save communication channel transactions):
                        addMoreItemsMessage(pShaderBaseTreeItemId, (amountOfShaders - maxItemsShown));
                        pItemData->m_objectCount = amountOfShaders;
                    }
                }

                // Set the shaders size:
                shadersMemorySize = pItemData->m_objectMemorySize;
                shaderObjectCount = pItemData->m_objectCount;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateApplicationPBuffers
// Description: Add the current PBuffers to the memory tree
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/9/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateApplicationPBuffers(unsigned int& objectCount)
{
    bool retVal = false;

    objectCount = 0;

    // Get amount of PBuffers
    int amountOfPBufferObjects = 0;
    bool rcAmount = gaGetAmountOfPBuffersObjects(amountOfPBufferObjects);
    GT_IF_WITH_ASSERT(rcAmount)
    {
        retVal = true;

        if (amountOfPBufferObjects > 0)
        {
            gdDebugApplicationTreeData* pPBuffersNodeData = NULL;
            afApplicationTreeItemData* pItemData = NULL;
            createObjectItemData(pItemData, pPBuffersNodeData);

            pItemData->m_itemType = AF_TREE_ITEM_GL_PBUFFERS_NODE;
            pItemData->m_objectMemorySize = 0;

            m_pPBuffersTreeId = appendItemToTree(m_pHeaderItem, GD_STR_MemoryAnalysisViewerTreePBuffersNode, m_pbufferIconIndex, pItemData);
            GT_IF_WITH_ASSERT(m_pPBuffersTreeId != NULL)
            {
                gtString progressMessage;

                // Iterate the PBuffers units:
                int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
                int itemsToShow = min(amountOfPBufferObjects, maxItemsShown);

                for (int pbufferID = 0; pbufferID < itemsToShow; pbufferID++)
                {
                    bool currentBufferOk = false;
                    progressMessage.makeEmpty();
                    progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingPBuffer GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, pbufferID);
                    afProgressBarWrapper::instance().setProgressDetails(progressMessage, amountOfPBufferObjects);

                    // Get the PBuffer details:
                    apPBuffer pbufferDetails;
                    bool rcPBO = gaGetPBufferObjectDetails(pbufferID, pbufferDetails);
                    GT_IF_WITH_ASSERT(rcPBO)
                    {
                        int contextId = pbufferDetails.pbufferContextId();

                        bool updateSize = (contextId != -1);

                        if (pbufferDetails.isDeleted())
                        {
                            // If the PBuffer is deleted, don't try and refresh its size.
                            updateSize = false;
                        }

                        // Update all static buffers dimension according to current HDC dimensions:
                        bool rcSize = false;

                        if (updateSize)
                        {
                            // This PBuffer belongs to a render context, update its size
                            rcSize = gaUpdatePBuffersDimension(contextId);
                        }
                        else
                        {
                            // This PBuffer doesn't belong to a render context yet, don't try to update its size:
                            rcSize = true;
                        }

                        // We allow the viewer to display the information even if we failed the update:
                        GT_ASSERT(rcSize);

                        // Get the (updated) information about the PBuffer (if this fails, we will use the older info):
                        pbufferDetails.setAllocatedObjectId(-1, true);
                        rcPBO = gaGetPBufferObjectDetails(pbufferID, pbufferDetails);
                        GT_ASSERT(rcPBO);

                        // Add the PBuffer to the tree:
                        gdDebugApplicationTreeData* pGDPBufferData = NULL;
                        afApplicationTreeItemData* pPBufferData = NULL;
                        createObjectItemData(pPBufferData, pGDPBufferData);

                        pPBufferData->m_itemType = AF_TREE_ITEM_GL_PBUFFER_NODE;

                        pGDPBufferData->_objectOpenGLName = pbufferID;
                        pGDPBufferData->_contextId._contextType = AP_OPENGL_CONTEXT;
                        pGDPBufferData->_contextId._contextId = contextId;
                        pGDPBufferData->_objectWidth = pbufferDetails.width();
                        pGDPBufferData->_objectHeight = pbufferDetails.height();

                        gtString pbufferName;

                        // If PBuffer exists (not deleted):
                        bool isDeleted = pbufferDetails.isDeleted();

                        if (!isDeleted)
                        {
                            // Generate a PBuffer name:
                            pbufferName.appendFormattedString(GD_STR_ImagesAndBuffersViewerPBufferName, pbufferID);
                        }
                        else
                        {
                            // PBuffer was deleted - show PBuffer deleted name:
                            pbufferName.appendFormattedString(GD_STR_ImagesAndBuffersViewerPBufferNameDeleted, pbufferID);
                        }

                        // If we know the render context, add it as well:
                        if (contextId > 0)
                        {
                            pbufferName.appendFormattedString(GD_STR_ImagesAndBuffersViewerPBufferNameRenderContext, contextId);
                        }

                        // Add the pbuffer to the tree:
                        QTreeWidgetItem* pPbufferTreeItemId = appendItemToTree(m_pPBuffersTreeId, pbufferName.asCharArray(), m_pbufferIconIndex, pPBufferData);
                        GT_IF_WITH_ASSERT(pPbufferTreeItemId != NULL)
                        {
                            // Increment the progress bar:
                            afProgressBarWrapper::instance().incrementProgressBar();

                            // Add the PBuffers content to the tree:
                            gtSize_t pbufferSize = 0;
                            currentBufferOk = true;
                            pPBufferData->m_objectMemorySize = pbufferSize;
                            pItemData->m_objectMemorySize += pbufferSize;

                            // Increase the amount of objects:
                            pItemData->m_objectCount ++;

                            // Add the buffers attached to the pbuffer to the objects tree:
                            bool rcAddPBufffersAttachments = updatePBufferAttachments(*pPBufferData);
                            GT_ASSERT(rcAddPBufffersAttachments);
                        }
                    }
                    retVal = retVal && currentBufferOk;
                }

                // If some items were hidden:
                if (maxItemsShown < amountOfPBufferObjects)
                {
                    // Show a message and count them in the parent (don't calculate their memory size to avoid slowdown and save communication channel transactions):
                    addMoreItemsMessage(m_pPBuffersTreeId, (amountOfPBufferObjects - maxItemsShown));
                }
            }

            // Set the amount of pbuffers to the pbuffers root:
            pItemData->m_objectCount = amountOfPBufferObjects;
            objectCount = pItemData->m_objectCount;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updatePBufferAttachments
// Description: Update a PBuffer static buffers list
// Arguments:   const gdDebugApplicationTreeData& pbufferItemData
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        16/2/2011
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updatePBufferAttachments(const afApplicationTreeItemData& pbufferItemData)
{
    bool retVal = false;

    gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pbufferItemData.extendedItemData());
    GT_IF_WITH_ASSERT(pGDData != NULL)
    {
        // Get amount of buffers in the PBuffer
        int amountOfBuffersObjects = 0;
        bool rc1 = gaGetAmountOfPBufferContentObjects(pGDData->_objectOpenGLName, amountOfBuffersObjects);
        GT_IF_WITH_ASSERT(rc1)
        {
            // Iterate the static buffers units:
            for (int bufferIter = 0; bufferIter < amountOfBuffersObjects; bufferIter++)
            {
                // Get the static buffer type:
                apDisplayBuffer bufferType;
                bool rc2 = gaGetPBufferStaticBufferType(pGDData->_objectOpenGLName, bufferIter, bufferType);
                GT_IF_WITH_ASSERT(rc2)
                {
                    // Get the PBuffer static buffer details:
                    apStaticBuffer staticBufferDetails;
                    bool rc3 = gaGetPBufferStaticBufferObjectDetails(pGDData->_objectOpenGLName, bufferType, staticBufferDetails);
                    GT_IF_WITH_ASSERT(rc3)
                    {
                        // Generate the name for the static buffer:
                        gtString bufferName;
                        bool rc4 = apGetBufferName(bufferType, bufferName);
                        GT_IF_WITH_ASSERT(rc4)
                        {
                            // Create the pbuffer attachment item data:
                            gdDebugApplicationTreeData* pAttachmentItemData = NULL;
                            afApplicationTreeItemData* pItemData = NULL;
                            createObjectItemData(pItemData, pAttachmentItemData);

                            // Set the viewer item details:
                            pItemData->m_itemType = AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER;
                            pAttachmentItemData->_objectOpenGLName = pGDData->_objectOpenGLName;
                            pAttachmentItemData->_bufferType = bufferType;
                            pAttachmentItemData->_contextId = pGDData->_contextId;
                            pAttachmentItemData->_objectOwnerName = pGDData->_objectOpenGLName;

                            // Add the static buffer to the buffers tree:
                            QTreeWidgetItem* pPbufferAttachmentItemId = appendItemToTree(pbufferItemData.m_pTreeWidgetItem, bufferName, m_staticBufferShortcutIconIndex, pItemData);
                            GT_IF_WITH_ASSERT(pPbufferAttachmentItemId != NULL)
                            {
                                // Get the original object item id:
                                afApplicationTreeItemData* pOriginalItemData = getPBufferDataById(pGDData->_contextId, pGDData->_objectOpenGLName);
                                GT_IF_WITH_ASSERT(pOriginalItemData != NULL)
                                {
                                    // Set the item id:
                                    pItemData->m_pOriginalItemTreeItem = pOriginalItemData->m_pTreeWidgetItem;
                                    retVal = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateApplicationSyncObjects
// Description: Add the current Sync objects to the memory tree
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/10/2009
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateApplicationSyncObjects(unsigned int& objectCount)
{
    bool retVal = false;

    objectCount = 0;

    // Get amount of sync objects
    int amountOfSyncObjects = 0;
    bool rcAmount = gaGetAmountOfSyncObjects(amountOfSyncObjects);
    GT_IF_WITH_ASSERT(rcAmount)
    {
        retVal = true;

        if (amountOfSyncObjects > 0)
        {
            gdDebugApplicationTreeData* pSyncObjectNodeData = NULL;
            afApplicationTreeItemData* pItemData = NULL;
            createObjectItemData(pItemData, pSyncObjectNodeData);

            // Set the allocated item data type:
            pItemData->m_itemType = AF_TREE_ITEM_GL_SYNC_OBJECTS_NODE;
            pItemData->m_objectMemorySize = 0;

            // Add the sync objects root item to the tree:
            m_pSyncObjectsTreeId = appendItemToTree(m_pHeaderItem, GD_STR_MemoryAnalysisViewerTreeSyncObjectsNode, m_syncObjectsIconIndex, pItemData);
            GT_IF_WITH_ASSERT(m_pSyncObjectsTreeId != NULL)
            {
                gtString progressMessage;

                // Iterate the sync objects:
                int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
                int itemsToShow = min(amountOfSyncObjects, maxItemsShown);

                for (int syncObjectIndex = 0; syncObjectIndex < itemsToShow; syncObjectIndex++)
                {
                    bool currentSyncObjectOK = false;
                    progressMessage.makeEmpty();
                    progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingSync GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, syncObjectIndex);
                    afProgressBarWrapper::instance().setProgressText(progressMessage);

                    // Get the sync object details:
                    apGLSync syncDetails;
                    bool rcSync = gaGetSyncObjectDetails(syncObjectIndex, syncDetails);
                    GT_IF_WITH_ASSERT(rcSync)
                    {
                        // Add the Sync object to the tree:
                        gdDebugApplicationTreeData* pGDSyncObjectData = NULL;
                        afApplicationTreeItemData* pSyncObjectData = NULL;
                        createObjectItemData(pSyncObjectData, pGDSyncObjectData);

                        // Set the sync object attributes:
                        int syncObjectId = syncDetails.syncID();
                        pGDSyncObjectData->_objectOpenGLName = syncObjectId;
                        pGDSyncObjectData->_syncHandle = syncDetails.syncHandle();
                        pGDSyncObjectData->_syncIndex = syncObjectIndex;
                        pItemData->m_itemType = AF_TREE_ITEM_GL_SYNC_OBJECT;

                        // Generate a sync object name:
                        gtString syncIdStr;
                        syncIdStr.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeSyncName, syncObjectId);

                        // Sync icon:
                        int imageIndex = m_syncObjectsIconIndex;

                        // Add the sync object to the tree:
                        QTreeWidgetItem* pSyncObjectTreeItemId = appendItemToTree(m_pSyncObjectsTreeId, syncIdStr, imageIndex, pItemData);

                        GT_IF_WITH_ASSERT(pSyncObjectTreeItemId != NULL)
                        {
                            currentSyncObjectOK = true;

                            // Increment the progress bar:
                            afProgressBarWrapper::instance().incrementProgressBar();

                            // A Sync objects memory is insignificant:
                            pSyncObjectData->m_objectMemorySize = 0;
                            pItemData->m_objectMemorySize += pSyncObjectData->m_objectMemorySize;

                            // Increase the amount of objects:
                            pItemData->m_objectCount ++;
                        }
                    }
                    retVal = retVal && currentSyncObjectOK;
                }

                // If some items were hidden:
                if (maxItemsShown < amountOfSyncObjects)
                {
                    // Show a message and count them in the parent (don't calculate their memory size to avoid slowdown and save communication channel transactions):
                    addMoreItemsMessage(m_pSyncObjectsTreeId, (amountOfSyncObjects - maxItemsShown));
                    pItemData->m_objectCount = amountOfSyncObjects;
                }
            }
            pItemData->m_objectCount = amountOfSyncObjects;
            objectCount = pItemData->m_objectCount;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::createAndLoadImageList
// Description: Creates this view image list, and loads its images
//              from disk.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/9/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::createAndLoadImageList()
{
    bool rc = true;

    // Add the Bitmaps into the recourse file
#define GD_TREE_HANDLER_ICONS_COUNT 61
    QPixmap* pPixmaps = new QPixmap[GD_TREE_HANDLER_ICONS_COUNT];

    int currentIdx = 0;

    QPixmap* renderContextIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*renderContextIcon, AC_ICON_DEBUG_APPTREE_GL_CONTEXT);

    QPixmap* renderContextDeletedIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*renderContextDeletedIcon, AC_ICON_DEBUG_APPTREE_GL_CONTEXTDELETED);

    QPixmap* renderContextSharedIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*renderContextSharedIcon, AC_ICON_DEBUG_APPTREE_GL_CONTEXTSHARED);

    QPixmap* renderContextDeletedSharedIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*renderContextDeletedSharedIcon, AC_ICON_DEBUG_APPTREE_GL_CONTEXTDELETEDSHARED);

    QPixmap* allTexturesIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*allTexturesIcon, AC_ICON_DEBUG_APPTREE_GL_TEXGENERIC);

    QPixmap* textures1DIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*textures1DIcon, AC_ICON_DEBUG_APPTREE_GL_TEX1D);

    QPixmap* textures2DIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*textures2DIcon, AC_ICON_DEBUG_APPTREE_GL_TEX2D);

    QPixmap* textures3DIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*textures3DIcon, AC_ICON_DEBUG_APPTREE_GL_TEX3D);

    QPixmap* textures1DArrayIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*textures1DArrayIcon, AC_ICON_DEBUG_APPTREE_GL_TEX1DA);

    QPixmap* textures2DArrayIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*textures2DArrayIcon, AC_ICON_DEBUG_APPTREE_GL_TEX2DA);

    QPixmap* textures2DMultisampleIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*textures2DMultisampleIcon, AC_ICON_DEBUG_APPTREE_GL_TEX2DMS);

    QPixmap* textures2DMultisampleArrayIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*textures2DMultisampleArrayIcon, AC_ICON_DEBUG_APPTREE_GL_TEX2DMSA);

    QPixmap* texturesListIconRectangle = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*texturesListIconRectangle, AC_ICON_DEBUG_APPTREE_GL_TEXRECT);

    QPixmap* texturesListIconCubeMap = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*texturesListIconCubeMap, AC_ICON_DEBUG_APPTREE_GL_TEXCUBE);

    QPixmap* texturesListIconCubeMapArray = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*texturesListIconCubeMapArray, AC_ICON_DEBUG_APPTREE_GL_TEXCUBEA);

    QPixmap* texturesListIconBuffer = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*texturesListIconBuffer, AC_ICON_DEBUG_APPTREE_GL_TEXBUFFER);

    QPixmap* texturesUnknown = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*texturesUnknown, AC_ICON_DEBUG_APPTREE_GL_TEXUNKNOWN);

    QPixmap* openGLBufferGenericIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLBufferGenericIcon, AC_ICON_DEBUG_APPTREE_GL_BUFFER_GENERIC);

    QPixmap* openGLBufferArrayIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLBufferArrayIcon, AC_ICON_DEBUG_APPTREE_GL_BUFFER_ARRAY);

    QPixmap* openGLBufferDrawIndirIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLBufferDrawIndirIcon, AC_ICON_DEBUG_APPTREE_GL_BUFFER_DRAW_INDIRECT);

    QPixmap* openGLBufferDispatchIndirIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLBufferDispatchIndirIcon, AC_ICON_DEBUG_APPTREE_GL_BUFFER_DISPATCH_INDIRECT);

    QPixmap* openGLBufferElementArrayIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLBufferElementArrayIcon, AC_ICON_DEBUG_APPTREE_GL_BUFFER_ELEMENT_ARRAY);

    QPixmap* openGLBufferPixelPackIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLBufferPixelPackIcon, AC_ICON_DEBUG_APPTREE_GL_BUFFER_PIXEL_PACK);

    QPixmap* openGLBufferPixelUnpackIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLBufferPixelUnpackIcon, AC_ICON_DEBUG_APPTREE_GL_BUFFER_PIXEL_UNPACK);

    QPixmap* openGLBufferCopyReadIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLBufferCopyReadIcon, AC_ICON_DEBUG_APPTREE_GL_BUFFER_COPY_READ);

    QPixmap* openGLBufferCopyWriteIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLBufferCopyWriteIcon, AC_ICON_DEBUG_APPTREE_GL_BUFFER_COPY_WRITE);

    QPixmap* openGLBufferTransformFeedbackIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLBufferTransformFeedbackIcon, AC_ICON_DEBUG_APPTREE_GL_BUFFER_TRANSFORM_FEEDBACK);

    QPixmap* openGLBufferUniformIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLBufferUniformIcon, AC_ICON_DEBUG_APPTREE_GL_BUFFER_UNIFORM);

    QPixmap* openGLBufferAtomicIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLBufferAtomicIcon, AC_ICON_DEBUG_APPTREE_GL_BUFFER_ATOMIC_COUNTER);

    QPixmap* openGLBufferShaderStorageIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLBufferShaderStorageIcon, AC_ICON_DEBUG_APPTREE_GL_BUFFER_SHADER_STORAGE);

    QPixmap* openGLBufferQueryIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLBufferQueryIcon, AC_ICON_DEBUG_APPTREE_GL_BUFFER_QUERY);

    QPixmap* openGLBufferTextureIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLBufferTextureIcon, AC_ICON_DEBUG_APPTREE_GL_BUFFER_TEXTURE);

    QPixmap* openGLBufferUnknownIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLBufferUnknownIcon, AC_ICON_DEBUG_APPTREE_GL_BUFFER_UNKNOWN);

    QPixmap* glSamplerIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*glSamplerIcon, AC_ICON_DEBUG_APPTREE_GL_SAMPLER);

    QPixmap* staticBuffersIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*staticBuffersIcon, AC_ICON_DEBUG_APPTREE_GL_STATICBUFFER);

    QPixmap* staticBuffersShortcutIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*staticBuffersShortcutIcon, AC_ICON_DEBUG_APPTREE_GL_STATICBUFFERLINK);

    QPixmap* renderBuffersIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*renderBuffersIcon, AC_ICON_DEBUG_APPTREE_GL_RENDERBUFFER);

    QPixmap* renderBufferShortcutIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*renderBufferShortcutIcon, AC_ICON_DEBUG_APPTREE_GL_RENDERBUFFERLINK);

    QPixmap* fboIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*fboIcon, AC_ICON_DEBUG_APPTREE_GL_FBO);

    QPixmap* openGLProgramIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLProgramIcon, AC_ICON_DEBUG_APPTREE_GL_PROGRAM);

    QPixmap* openGLProgramDeletedIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLProgramDeletedIcon, AC_ICON_DEBUG_APPTREE_GL_PROGRAMDELETED);

    QPixmap* openGLShaderIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLShaderIcon, AC_ICON_DEBUG_APPTREE_GL_SHADER);

    QPixmap* openGLShaderDeletedIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLShaderDeletedIcon, AC_ICON_DEBUG_APPTREE_GL_SHADERDELETED);

    QPixmap* openGLProgramPipelineIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openGLProgramPipelineIcon, AC_ICON_DEBUG_APPTREE_GL_PIPELINE);

    QPixmap* displayListIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*displayListIcon, AC_ICON_DEBUG_APPTREE_GL_DISPLAYLIST);

    QPixmap* pbuffersIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*pbuffersIcon, AC_ICON_DEBUG_APPTREE_GL_PBUFFER);

    QPixmap* syncIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*syncIcon, AC_ICON_DEBUG_APPTREE_GL_SYNC);

    QPixmap* computeContextIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*computeContextIcon, AC_ICON_DEBUG_APPTREE_CL_CONTEXT);

    QPixmap* computeContextDeletedIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*computeContextDeletedIcon, AC_ICON_DEBUG_APPTREE_CL_CONTEXTDELETED);

    QPixmap* openCLBufferIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openCLBufferIcon, AC_ICON_DEBUG_APPTREE_CL_BUFFER);

    QPixmap* openCLPipeIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openCLPipeIcon, AC_ICON_DEBUG_APPTREE_CL_PIPE);

    QPixmap* allCLTexturesIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*allCLTexturesIcon, AC_ICON_DEBUG_APPTREE_CL_IMAGEGENERIC);

    QPixmap* textures2DIconCL = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*textures2DIconCL, AC_ICON_DEBUG_APPTREE_CL_IMAGE2D);

    QPixmap* textures3DIconCL = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*textures3DIconCL, AC_ICON_DEBUG_APPTREE_CL_IMAGE3D);

    QPixmap* openCLSamplerIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openCLSamplerIcon, AC_ICON_DEBUG_APPTREE_CL_SAMPLER);

    QPixmap* commandQueueIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*commandQueueIcon, AC_ICON_DEBUG_APPTREE_CL_QUEUE);

    QPixmap* openCLEventIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openCLEventIcon, AC_ICON_DEBUG_APPTREE_CL_EVENT);

    QPixmap* openCLProgramIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openCLProgramIcon, AC_ICON_DEBUG_APPTREE_CL_PROGRAM);

    QPixmap* openCLKernelIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*openCLKernelIcon, AC_ICON_DEBUG_APPTREE_CL_KERNEL);

    QPixmap* yellowWarningIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*yellowWarningIcon, AC_ICON_WARNING_YELLOW);

    QPixmap* informationIcon = &(pPixmaps[currentIdx++]);
    acSetIconInPixmap(*informationIcon, AC_ICON_WARNING_INFO);

    GT_ASSERT(GD_TREE_HANDLER_ICONS_COUNT == currentIdx);

    m_treeItemsVector.reserve(GD_TREE_HANDLER_ICONS_COUNT);

    m_texture1DIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(textures1DIcon);

    m_texture2DIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(textures2DIcon);

    m_texture3DIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(textures3DIcon);

    m_texture1DArrayIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(textures1DArrayIcon);

    m_texture2DArrayIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(textures2DArrayIcon);

    m_texture2DMultisampleIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(textures2DMultisampleIcon);

    m_texture2DMultisampleArrayIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(textures2DMultisampleArrayIcon);

    m_textureUnknownIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(texturesUnknown);

    m_allTexturesIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(allTexturesIcon);

    m_texturesShortcutIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(allTexturesIcon);

    m_textureCubeMapIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(texturesListIconCubeMap);

    m_textureCubeMapArrayIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(texturesListIconCubeMapArray);

    m_textureRectangleIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(texturesListIconRectangle);

    m_textureBufferIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(texturesListIconBuffer);

    m_openGLBufferGenericIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLBufferGenericIcon);

    m_openGLBufferArrayIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLBufferArrayIcon);

    m_openGLBufferDrawIndirIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLBufferDrawIndirIcon);

    m_openGLBufferDispatchIndirIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLBufferDispatchIndirIcon);

    m_openGLBufferElementArrayIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLBufferElementArrayIcon);

    m_openGLBufferPixelPackIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLBufferPixelPackIcon);

    m_openGLBufferPixelUnpackIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLBufferPixelUnpackIcon);

    m_openGLBufferCopyReadIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLBufferCopyReadIcon);

    m_openGLBufferCopyWriteIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLBufferCopyWriteIcon);

    m_openGLBufferTransformFeedbackIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLBufferTransformFeedbackIcon);

    m_openGLBufferUniformIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLBufferUniformIcon);

    m_openGLBufferAtomicIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLBufferAtomicIcon);

    m_openGLBufferShaderStorageIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLBufferShaderStorageIcon);

    m_openGLBufferQueryIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLBufferQueryIcon);

    m_openGLBufferTextureIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLBufferTextureIcon);

    m_openGLBufferUnknownIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLBufferUnknownIcon);

    m_renderBufferIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(renderBuffersIcon);

    m_renderBufferShortcutIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(renderBufferShortcutIcon);

    m_staticBufferIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(staticBuffersIcon);

    m_staticBufferShortcutIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(staticBuffersShortcutIcon);

    m_pbufferIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(pbuffersIcon);

    m_syncObjectsIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(syncIcon);

    m_commandQueueIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(commandQueueIcon);

    m_displayListIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(displayListIcon);

    m_renderContextIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(renderContextIcon);

    m_renderContextDeletedIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(renderContextDeletedIcon);

    m_renderContextSharedIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(renderContextSharedIcon);

    m_renderContextDeletedSharedIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(renderContextDeletedSharedIcon);

    m_computeContextIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(computeContextIcon);

    m_computeContextDeletedIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(computeContextDeletedIcon);

    m_openGLProgramIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLProgramIcon);

    m_openGLProgramDeletedIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLProgramDeletedIcon);

    m_openGLShaderIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLShaderIcon);

    m_openGLShaderDeletedIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLShaderDeletedIcon);

    m_fboIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(fboIcon);

    m_glSamplerIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(glSamplerIcon);

    m_openGLProgramPipelineIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openGLProgramPipelineIcon);

    m_allCLImagesIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(allCLTexturesIcon);

    m_clImage2DIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(textures2DIconCL);

    m_clImage3DIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(textures3DIconCL);

    m_clBufferIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openCLBufferIcon);

    m_clPipeIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openCLPipeIcon);

    m_clSamplerIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openCLSamplerIcon);

    m_clEventIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openCLEventIcon);

    m_openCLProgramIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openCLProgramIcon);

    m_openCLKernelIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(openCLKernelIcon);

    m_memoryLeakIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(yellowWarningIcon);

    m_informationIconIndex = (int)m_treeItemsVector.size();
    m_treeItemsVector.push_back(informationIcon);

    return rc;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::textureTypeToIconIndex
// Description: Maps a texture type to tree item index
// Arguments: apTextureType textureType
// Return Val: int
// Author:      Sigal Algranaty
// Date:        24/9/2008
// ---------------------------------------------------------------------------
int gdDebugApplicationTreeHandler::textureTypeToIconIndex(const apContextID& contextId, apTextureType textureType)
{
    int retVal = 0;

    bool isOpenGLTexture = (contextId.isOpenGLContext());
    GT_ASSERT(contextId.isValid() && (!contextId.isDefault()));

    switch (textureType)
    {
        case AP_1D_TEXTURE:
            retVal = m_texture1DIconIndex;
            break;

        case AP_2D_TEXTURE:
            retVal = isOpenGLTexture ? m_texture2DIconIndex : m_clImage2DIconIndex;
            break;

        case AP_3D_TEXTURE:
            retVal = isOpenGLTexture ? m_texture3DIconIndex : m_clImage3DIconIndex;
            break;

        case AP_1D_ARRAY_TEXTURE:
            retVal = m_texture1DArrayIconIndex;
            break;

        case AP_2D_ARRAY_TEXTURE:
            retVal = m_texture2DArrayIconIndex;
            break;

        case AP_2D_TEXTURE_MULTISAMPLE:
            retVal = m_texture2DMultisampleIconIndex;
            break;

        case AP_2D_TEXTURE_MULTISAMPLE_ARRAY:
            retVal = m_texture2DMultisampleArrayIconIndex;
            break;

        case AP_CUBE_MAP_TEXTURE:
            retVal = m_textureCubeMapIconIndex;
            break;

        case AP_CUBE_MAP_ARRAY_TEXTURE:
            retVal = m_textureCubeMapArrayIconIndex;
            break;

        case AP_TEXTURE_RECTANGLE:
            retVal = m_textureRectangleIconIndex;
            break;

        case AP_BUFFER_TEXTURE:
            retVal = m_textureBufferIconIndex;
            break;

        case AP_UNKNOWN_TEXTURE_TYPE:
            retVal = m_textureUnknownIconIndex;
            break;

        default:
            // Invalid input:
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateCLImagesList
// Description: Update OpenCL textures list
// Arguments: int contextIndex - the context index (zero based)
//            gtUInt32& texturesMemorySize
//            unsigned int& textureObjectCount
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/12/2009
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateCLImagesList(apContextID contextID, gtUInt32& texturesMemorySize, unsigned int& textureObjectCount)
{
    bool retVal = true;

    // Sanity check:
    texturesMemorySize = 0;
    textureObjectCount = 0;

    // Get the context tree item id:
    QTreeWidgetItem* pContextTreeId = getContextTreeItemId(contextID);

    // Make sure that the tree node is ok:
    GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
    {
        // Get the non-deleted textures for this context:
        gtPtrVector<apCLImage*> currentContextImages;
        retVal = getCLContextLivingImageObjects(contextID, currentContextImages);
        int amountOfImages = (int)currentContextImages.size();

        if (amountOfImages > 0)
        {
            // Display Progress Bar Message:
            gtString progressMessage;
            progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingContextImages GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, contextID._contextId);
            afProgressBarWrapper::instance().setProgressDetails(progressMessage, amountOfImages);

            // Add the render context texture node:
            // Create a new memory item data, and fill the relevant fields:
            gdDebugApplicationTreeData* pTexturesNodeData = NULL;
            afApplicationTreeItemData* pItemData = NULL;
            createObjectItemData(pItemData, pTexturesNodeData);

            pItemData->m_itemType = AF_TREE_ITEM_CL_IMAGES_NODE;
            pItemData->m_objectCount = 0;
            pItemData->m_objectMemorySize = 0;

            pTexturesNodeData->_contextId = contextID;
            pTexturesNodeData->_objectOpenCLName = contextID._contextId;

            QTreeWidgetItem* pTexturesBaseTreeItemId = appendItemToTree(pContextTreeId, GD_STR_MemoryAnalysisViewerTreeImagesNode, m_allCLImagesIconIndex, pItemData);
            GT_IF_WITH_ASSERT(pTexturesBaseTreeItemId != NULL)
            {
                // Set the tree item id on the memory object data:
                pItemData->m_objectCount += amountOfImages;

                // Add the images to the tree:
                int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
                int itemsToShow = min(amountOfImages, maxItemsShown);

                for (int i = 0; i < itemsToShow; i++)
                {
                    // Build a texture item data, and fill the texture details:
                    afApplicationTreeItemData* pNewTextureItemData = NULL;
                    GT_IF_WITH_ASSERT(currentContextImages[i] != NULL)
                    {
                        int imageIndex = 0;
                        bool rc = buildCLImageObjectData(contextID, i, *currentContextImages[i], pNewTextureItemData, imageIndex);
                        retVal = retVal && rc;
                        GT_IF_WITH_ASSERT(retVal && (pNewTextureItemData != NULL))
                        {
                            // Add the texture to the tree:
                            gtString textureNameStr;

                            // Build the texture name:
                            gdHTMLProperties htmlBuilder;
                            htmlBuilder.getCLImageName(*currentContextImages[i], textureNameStr);

                            // Add this texture item to the tree:
                            QTreeWidgetItem* pTextureTreeItemId = appendItemToTree(pTexturesBaseTreeItemId, textureNameStr.asCharArray(), imageIndex, pNewTextureItemData);

                            // Add the new texture to the textures total memory size:
                            pItemData->m_objectMemorySize += pNewTextureItemData->m_objectMemorySize;

                            // Increment the progress bar
                            afProgressBarWrapper::instance().incrementProgressBar();

                            retVal = retVal && (pTextureTreeItemId != NULL);

                        }
                    }
                }

                // If some items were hidden:
                if (maxItemsShown < amountOfImages)
                {
                    // Show a message:
                    addMoreItemsMessage(pTexturesBaseTreeItemId, (amountOfImages - maxItemsShown));
                }

                // Return the textures node's data:
                texturesMemorySize = pItemData->m_objectMemorySize;
                textureObjectCount = pItemData->m_objectCount;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateCLBuffersList
// Description: Update OpenCL buffers list
// Arguments: int contextId
//            gtUInt32& buffersMemorySize
//            unsigned int& buffersObjectCount
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/12/2009
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateCLBuffersList(apContextID contextID, gtUInt32& buffersMemorySize, unsigned int& buffersObjectCount)
{
    bool retVal = true;

    buffersMemorySize = 0;
    buffersObjectCount = 0;

    // Get the context tree item id:
    QTreeWidgetItem* pContextTreeId = getContextTreeItemId(contextID);

    // Make sure that the tree node is ok:
    GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
    {
        // Get the non-deleted buffers for this context:
        gtPtrVector<apCLBuffer*> currentContextBuffers;
        retVal = getCLContextLivingBufferObjects(contextID, currentContextBuffers);
        int amountOfBuffers = (int)currentContextBuffers.size();

        if (amountOfBuffers > 0)
        {
            // Display Progress Bar Message:
            gtString progressMessage;
            progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingContextCLBuffers GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, contextID._contextId);
            afProgressBarWrapper::instance().setProgressDetails(progressMessage, amountOfBuffers);

            // Add the context buffers node:
            // Create a new memory item data, and fill the relevant fields:
            gdDebugApplicationTreeData* pBuffersItemData = NULL;
            afApplicationTreeItemData* pItemData = NULL;
            createObjectItemData(pItemData, pBuffersItemData);

            pItemData->m_itemType = AF_TREE_ITEM_CL_BUFFERS_NODE;
            pItemData->m_objectMemorySize = 0;
            pItemData->m_objectCount = 0;

            pBuffersItemData->_contextId = contextID;
            pBuffersItemData->_objectOpenCLName = contextID._contextId;

            QTreeWidgetItem* pBuffersBaseTreeItemId = appendItemToTree(pContextTreeId, GD_STR_MemoryAnalysisViewerTreeBuffersNode, m_clBufferIconIndex, pItemData);
            GT_IF_WITH_ASSERT(pBuffersBaseTreeItemId != NULL)
            {
                // Set the tree item id on the memory object data:
                pItemData->m_objectCount += amountOfBuffers;

                // Add the buffers to the tree:
                int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
                int itemsToShow = min(amountOfBuffers, maxItemsShown);

                for (int i = 0; i < itemsToShow; i++)
                {
                    // Build a buffer item data, and fill the buffer details:
                    afApplicationTreeItemData* pNewBufferItemData = NULL;
                    GT_IF_WITH_ASSERT(currentContextBuffers[i] != NULL)
                    {
                        int imageIndex = m_clBufferIconIndex;
                        retVal = retVal && buildCLBufferObjectData(contextID, i, *currentContextBuffers[i], pNewBufferItemData, imageIndex);
                        GT_IF_WITH_ASSERT(retVal && (pNewBufferItemData != NULL))
                        {
                            gtString bufferNameStr;
                            gdGetBufferDisplayName(*(currentContextBuffers[i]), bufferNameStr);

                            // Add this buffer item to the tree:
                            QTreeWidgetItem* pBufferTreeItemId = appendItemToTree(pBuffersBaseTreeItemId, bufferNameStr.asCharArray(), imageIndex, pNewBufferItemData);
                            retVal = retVal && (pBufferTreeItemId != NULL);

                            pItemData->m_objectMemorySize += pNewBufferItemData->m_objectMemorySize;

                            // Add the buffer's sub buffers:
                            int subBuffersObjectsCount = 0;
                            bool rcAddSubBuffers = updateCLSubBuffersList(contextID, i, pBufferTreeItemId, *currentContextBuffers[i], subBuffersObjectsCount);
                            GT_ASSERT(rcAddSubBuffers);

                            // Fill the amount of sub buffers:
                            pNewBufferItemData->m_objectCount += subBuffersObjectsCount;

                            // Increment the progress bar
                            afProgressBarWrapper::instance().incrementProgressBar();
                        }
                    }
                }

                // If some items were hidden:
                if (maxItemsShown < amountOfBuffers)
                {
                    // Show a message and count them in the parent (don't calculate their memory size to avoid slowdown and save communication channel transactions):
                    addMoreItemsMessage(pBuffersBaseTreeItemId, (amountOfBuffers - maxItemsShown));
                    pItemData->m_objectCount = amountOfBuffers;
                }
            }

            // Return the buffers node's data:
            buffersMemorySize = pItemData->m_objectMemorySize;
            buffersObjectCount = pItemData->m_objectCount;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateCLPipesList
// Description: Update OpenCL pipes list
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        1/10/2014
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateCLPipesList(apContextID contextID, gtUInt32& pipesMemorySize, unsigned int& pipesObjectCount)
{
    bool retVal = true;

    pipesMemorySize = 0;
    pipesObjectCount = 0;

    // Get the context tree item id:
    QTreeWidgetItem* pContextTreeId = getContextTreeItemId(contextID);

    // Make sure that the tree node is ok:
    GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
    {
        // Get the non-deleted pipes for this context:
        gtPtrVector<apCLPipe*> currentContextPipes;
        retVal = getCLContextLivingPipeObjects(contextID, currentContextPipes);
        int amountOfPipes = (int)currentContextPipes.size();

        if (amountOfPipes > 0)
        {
            // Display Progress Bar Message:
            gtString progressMessage;
            progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingContextCLPipes GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, contextID._contextId);
            afProgressBarWrapper::instance().setProgressDetails(progressMessage, amountOfPipes);

            // Add the context pipes node:
            // Create a new memory item data, and fill the relevant fields:
            gdDebugApplicationTreeData* pPipesItemData = NULL;
            afApplicationTreeItemData* pItemData = NULL;
            createObjectItemData(pItemData, pPipesItemData);

            pItemData->m_itemType = AF_TREE_ITEM_CL_PIPES_NODE;
            pItemData->m_objectMemorySize = 0;
            pItemData->m_objectCount = 0;

            pPipesItemData->_contextId = contextID;
            pPipesItemData->_objectOpenCLName = contextID._contextId;

            QTreeWidgetItem* pPipesBaseTreeItemId = appendItemToTree(pContextTreeId, GD_STR_MemoryAnalysisViewerTreePipesNode, m_clPipeIconIndex, pItemData);
            GT_IF_WITH_ASSERT(pPipesBaseTreeItemId != NULL)
            {
                // Set the tree item id on the memory object data:
                pItemData->m_objectCount += amountOfPipes;

                // Add the pipes to the tree:
                int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
                int itemsToShow = min(amountOfPipes, maxItemsShown);

                for (int i = 0; i < itemsToShow; i++)
                {
                    // Build a pipe item data, and fill the pipe details:
                    afApplicationTreeItemData* pNewPipeItemData = NULL;
                    GT_IF_WITH_ASSERT(currentContextPipes[i] != NULL)
                    {
                        int imageIndex = m_clPipeIconIndex;
                        retVal = retVal && buildCLPipeObjectData(contextID, i, *currentContextPipes[i], pNewPipeItemData, imageIndex);
                        GT_IF_WITH_ASSERT(retVal && (pNewPipeItemData != NULL))
                        {
                            gtString pipeNameStr;
                            gdGetPipeDisplayName(*(currentContextPipes[i]), pipeNameStr);

                            // Add this pipe item to the tree:
                            QTreeWidgetItem* pPipeTreeItemId = appendItemToTree(pPipesBaseTreeItemId, pipeNameStr.asCharArray(), imageIndex, pNewPipeItemData);
                            retVal = retVal && (pPipeTreeItemId != NULL);

                            pItemData->m_objectMemorySize += pNewPipeItemData->m_objectMemorySize;

                            // Increment the progress bar
                            afProgressBarWrapper::instance().incrementProgressBar();
                        }
                    }
                }

                // If some items were hidden:
                if (maxItemsShown < amountOfPipes)
                {
                    // Show a message and count them in the parent (don't calculate their memory size to avoid slowdown and save communication channel transactions):
                    addMoreItemsMessage(pPipesBaseTreeItemId, (amountOfPipes - maxItemsShown));
                }
            }

            // Return the pipes node's data:
            pipesMemorySize = pItemData->m_objectMemorySize;
            pipesObjectCount = pItemData->m_objectCount;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateCLSamplersList
// Description: Update OpenCL buffers list
// Arguments:   int contextId
//              gtUInt32& samplersMemorySize
//              unsigned int& samplersObjectsCount
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        3/5/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateCLSamplersList(apContextID contextID, gtUInt32& samplersMemorySize, unsigned int& samplersObjectsCount)
{
    bool retVal = true;

    samplersMemorySize = 0;
    samplersObjectsCount = 0;

    // Get the context tree item id:
    QTreeWidgetItem* pContextTreeId = getContextTreeItemId(contextID);

    // Make sure that the tree node is ok:
    GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
    {
        // Get the current living samplers:
        gtPtrVector<apCLSampler*> contextSamplers;
        bool rcGetSamplers = getCLContextLivingSamplerObjects(contextID, contextSamplers);
        GT_IF_WITH_ASSERT(rcGetSamplers)
        {
            // Get the amount of living samplers:
            int amountOfSamplers = (int)contextSamplers.size();

            if (amountOfSamplers > 0)
            {
                // Display Progress Bar Message:
                gtString progressMessage;
                progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingContextCLSamplers GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, contextID._contextId);
                afProgressBarWrapper::instance().setProgressDetails(progressMessage, amountOfSamplers);

                // Add the context samplers node:
                // Create a new memory item data, and fill the relevant fields:
                gdDebugApplicationTreeData* pSamplersItemData = NULL;
                afApplicationTreeItemData* pItemData = NULL;
                createObjectItemData(pItemData, pSamplersItemData);

                pItemData->m_itemType = AF_TREE_ITEM_CL_SAMPLERS_NODE;
                pItemData->m_objectCount = 0;
                pItemData->m_objectMemorySize = 0;

                pSamplersItemData->_contextId = contextID;

                // Add the samplers root to the tree:
                QTreeWidgetItem* pSamplersBaseTreeItemId = appendItemToTree(pContextTreeId, GD_STR_MemoryAnalysisViewerTreeSamplersNode, m_clSamplerIconIndex, pItemData);
                GT_IF_WITH_ASSERT(pSamplersBaseTreeItemId != NULL)
                {
                    // Set the tree item id on the memory object data:
                    pItemData->m_objectCount += amountOfSamplers;

                    // Add the samplers to the tree:
                    int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
                    int itemsToShow = min(amountOfSamplers, maxItemsShown);

                    for (int i = 0; i < itemsToShow; i++)
                    {
                        // Build a sampler item data, and fill the buffer details:
                        afApplicationTreeItemData* pNewSamplerItemData = NULL;
                        GT_IF_WITH_ASSERT(contextSamplers[i] != NULL)
                        {
                            int imageIndex = m_clSamplerIconIndex;
                            retVal = retVal && buildCLSamplerObjectData(contextID, i, *contextSamplers[i], pNewSamplerItemData, imageIndex);
                            GT_IF_WITH_ASSERT(retVal && (pNewSamplerItemData != NULL))
                            {
                                // Build the sampler's name string:
                                gtString samplerNameStr;
                                samplerNameStr.appendFormattedString(GD_STR_TexturesViewerNameCLSampler, contextSamplers[i]->samplerId());

                                // Add AMD ext name:
                                setCLNodeTextFromStrAndData(samplerNameStr, pNewSamplerItemData);

                                // Add this sampler item to the tree:
                                QTreeWidgetItem* pSamplerTreeItemId = appendItemToTree(pSamplersBaseTreeItemId, samplerNameStr.asCharArray(), imageIndex, pNewSamplerItemData);
                                retVal = retVal && (pSamplerTreeItemId != NULL);

                                // Add the new sampler to the buffers total memory size:
                                pItemData->m_objectMemorySize += pItemData->m_objectMemorySize;

                                // Increment the progress bar
                                afProgressBarWrapper::instance().incrementProgressBar();
                            }
                        }
                    }

                    // If some items were hidden:
                    if (maxItemsShown < amountOfSamplers)
                    {
                        // Show a message:
                        addMoreItemsMessage(pSamplersBaseTreeItemId, (amountOfSamplers - maxItemsShown));
                    }
                }

                // Return the samplers node's data:
                samplersMemorySize = pItemData->m_objectMemorySize;
                samplersObjectsCount = pItemData->m_objectCount;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateCLEventsList
// Description: Update OpenCL buffers list
// Return Val:  bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateCLEventsList(apContextID contextID, gtUInt32& eventsMemorySize, unsigned int& eventsObjectsCount)
{
    bool retVal = true;

    eventsMemorySize = 0;
    eventsObjectsCount = 0;

    // Get the context tree item id:
    QTreeWidgetItem* pContextTreeId = getContextTreeItemId(contextID);

    // Make sure that the tree node is ok:
    GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
    {
        // Get the current living events:
        gtPtrVector<apCLEvent*> contextEvents;
        bool rcGetEvents = getCLContextLivingEventObjects(contextID, contextEvents);
        GT_IF_WITH_ASSERT(rcGetEvents)
        {
            // Get the amount of living events:
            int amountOfEvents = (int)contextEvents.size();

            if (amountOfEvents > 0)
            {
                // Display Progress Bar Message:
                gtString progressMessage;
                progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingContextCLEvents GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, contextID._contextId);
                afProgressBarWrapper::instance().setProgressDetails(progressMessage, amountOfEvents);

                // Add the context events node:
                // Create a new memory item data, and fill the relevant fields:
                gdDebugApplicationTreeData* pEventsItemData = NULL;
                afApplicationTreeItemData* pItemData = NULL;
                createObjectItemData(pItemData, pEventsItemData);

                pItemData->m_itemType = AF_TREE_ITEM_CL_EVENTS_NODE;
                pItemData->m_objectCount = 0;
                pItemData->m_objectMemorySize = 0;

                pEventsItemData->_contextId = contextID;

                // Add the events root to the tree:
                QTreeWidgetItem* pEventsBaseTreeItemId = appendItemToTree(pContextTreeId, GD_STR_MemoryAnalysisViewerTreeEventsNode, m_clEventIconIndex, pItemData);
                GT_IF_WITH_ASSERT(pEventsBaseTreeItemId != NULL)
                {
                    // Set the tree item id on the memory object data:
                    pItemData->m_objectCount += amountOfEvents;

                    // Add the events to the tree:
                    int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
                    int itemsToShow = min(amountOfEvents, maxItemsShown);

                    for (int i = 0; i < itemsToShow; i++)
                    {
                        // Build a event item data, and fill the buffer details:
                        afApplicationTreeItemData* pNewEventItemData = NULL;
                        GT_IF_WITH_ASSERT(contextEvents[i] != NULL)
                        {
                            int imageIndex = m_clEventIconIndex;
                            retVal = retVal && buildCLEventObjectData(contextID, i, *contextEvents[i], pNewEventItemData, imageIndex);
                            GT_IF_WITH_ASSERT(retVal && (pNewEventItemData != NULL))
                            {
                                // Build the event's name string:
                                gtString eventNameStr;
                                eventNameStr.appendFormattedString(GD_STR_PropertiesCLEventName, i + 1);

                                // Add AMD ext name:
                                setCLNodeTextFromStrAndData(eventNameStr, pNewEventItemData);

                                // Add this event item to the tree:
                                QTreeWidgetItem* pEventTreeItemId = appendItemToTree(pEventsBaseTreeItemId, eventNameStr.asCharArray(), imageIndex, pNewEventItemData);
                                retVal = retVal && (pEventTreeItemId != NULL);

                                // Add the new event to the buffers total memory size:
                                pItemData->m_objectMemorySize += pItemData->m_objectMemorySize;

                                // Increment the progress bar
                                afProgressBarWrapper::instance().incrementProgressBar();
                            }
                        }
                    }

                    // If some items were hidden:
                    if (maxItemsShown < amountOfEvents)
                    {
                        // Show a message and count them in the parent (don't calculate their memory size to avoid slowdown and save communication channel transactions):
                        addMoreItemsMessage(pEventsBaseTreeItemId, (amountOfEvents - maxItemsShown));
                    }
                }

                // Return the events node's data:
                eventsMemorySize = pItemData->m_objectMemorySize;
                eventsObjectsCount = pItemData->m_objectCount;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateCLCommandQueuesList
// Description: Update OpenCL command queues list
// Arguments: int contextId
//            gtUInt32& commandQueuesMemorySize
//            unsigned int& commandQueuesObjectCount
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        16/2/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateCLCommandQueuesList(apContextID contextID, gtUInt32& commandQueuesMemorySize, unsigned int& commandQueuesObjectCount)
{
    bool retVal = true;

    commandQueuesMemorySize = 0;
    commandQueuesObjectCount = 0;

    // Get the context tree item id:
    QTreeWidgetItem* pContextTreeId = getContextTreeItemId(contextID);

    // Make sure that the tree node is ok:
    GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
    {
        // Get the non-deleted command queues for this context:
        gtPtrVector<apCLCommandQueue*> currentContextQueues;
        gtVector<int> currentContextQueueIndices;
        retVal = getCLContextLivingQueueObjects(contextID, currentContextQueues, currentContextQueueIndices);
        int amountOfCommandQueues = (int)currentContextQueues.size();
        int amountOfIndices = (int)currentContextQueueIndices.size();
        GT_IF_WITH_ASSERT(retVal && (amountOfCommandQueues == amountOfIndices))
        {
            if (amountOfCommandQueues > 0)
            {
                // Display Progress Bar Message:
                gtString progressMessage;
                progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingContextCommandQueues GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, contextID._contextId);
                afProgressBarWrapper::instance().setProgressDetails(progressMessage, amountOfCommandQueues);

                // Add the context command queues node:
                // Create a new memory item data, and fill the relevant fields:
                gdDebugApplicationTreeData* pCommandQueuesItemData = NULL;
                afApplicationTreeItemData* pItemData = NULL;
                createObjectItemData(pItemData, pCommandQueuesItemData);

                pItemData->m_itemType = AF_TREE_ITEM_CL_COMMAND_QUEUES_NODE;
                pItemData->m_objectMemorySize = 0;
                pItemData->m_objectCount = 0;
                pCommandQueuesItemData->_contextId = contextID;

                // Add the command queue item to the tree:
                QTreeWidgetItem* pCommandQueuesBaseTreeItemId = appendItemToTree(pContextTreeId, GD_STR_MemoryAnalysisViewerTreeCommandQueuesNode, m_commandQueueIconIndex, pItemData);
                GT_IF_WITH_ASSERT(pCommandQueuesBaseTreeItemId != NULL)
                {
                    // Set the tree item id on the memory object data:
                    pItemData->m_objectCount += amountOfCommandQueues;

                    // Add each of the command queues:
                    int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
                    int itemsToShow = min(amountOfCommandQueues, maxItemsShown);

                    for (int i = 0; i < itemsToShow; i++)
                    {
                        // Build a command queue item data, and fill the command queue details:
                        afApplicationTreeItemData* pNewCommandQueueItemData = NULL;
                        GT_IF_WITH_ASSERT(currentContextQueues[i] != NULL)
                        {
                            int imageIndex = m_commandQueueIconIndex;
                            retVal = retVal && buildCLCommandQueueObjectData(contextID, currentContextQueueIndices[i], *currentContextQueues[i], pNewCommandQueueItemData, imageIndex);
                            GT_IF_WITH_ASSERT(retVal && (pNewCommandQueueItemData != NULL))
                            {
                                gtString queueNameStr;
                                queueNameStr.appendFormattedString(GD_STR_PropertiesCLCommandQueueName, currentContextQueueIndices[i] + 1);

                                // Add AMD ext name:
                                setCLNodeTextFromStrAndData(queueNameStr, pNewCommandQueueItemData);

                                // Add this command queue item to the tree:
                                QTreeWidgetItem* pCommandQueueTreeItemId = appendItemToTree(pCommandQueuesBaseTreeItemId, queueNameStr.asCharArray(), imageIndex, pNewCommandQueueItemData);
                                retVal = retVal && (pCommandQueueTreeItemId != NULL);

                                // Add the new command queue to the command queue total memory size:
                                pItemData->m_objectMemorySize += pNewCommandQueueItemData->m_objectMemorySize;

                                // Increment the progress bar
                                afProgressBarWrapper::instance().incrementProgressBar();
                            }
                        }
                    }

                    // If some items were hidden:
                    if (maxItemsShown < amountOfCommandQueues)
                    {
                        // Show a message:
                        addMoreItemsMessage(pCommandQueuesBaseTreeItemId, (amountOfCommandQueues - maxItemsShown));
                    }
                }

                // Return the queues node's data:
                commandQueuesMemorySize = pItemData->m_objectMemorySize;
                commandQueuesObjectCount = pItemData->m_objectCount;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateCLProgramsList
// Description: Update OpenCL programs list
// Arguments: int contextId
//            gtUInt32& programsMemorySize
//            unsigned int& programsObjectCount
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/3/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateCLProgramsList(apContextID contextID, gtUInt32& programsMemorySize, unsigned int& programsObjectCount)
{
    bool retVal = true;

    programsMemorySize = 0;
    programsObjectCount = 0;

    // Get the context tree item id:
    QTreeWidgetItem* pContextTreeId = getContextTreeItemId(contextID);

    // Make sure that the tree node is ok:
    GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL) && (m_pApplicationTree != NULL))
    {
        // Get the non-deleted programs for this context:
        gtPtrVector<apCLProgram*> currentContextPrograms;
        gtVector<int> currentContextProgramIndices;
        retVal = getCLContextLivingProgramObjects(contextID, currentContextPrograms, currentContextProgramIndices);
        int amountOfPrograms = (int)currentContextPrograms.size();
        int amountOfIndices = (int)currentContextProgramIndices.size();
        GT_IF_WITH_ASSERT(retVal && (amountOfPrograms == amountOfIndices))
        {
            if (amountOfPrograms > 0)
            {
                // Display Progress Bar Message:
                gtString progressMessage;
                progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingContextCLPrograms GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, contextID._contextId);
                afProgressBarWrapper::instance().setProgressDetails(progressMessage, amountOfPrograms);

                // Add the context program node:
                // Create a new memory item data, and fill the relevant fields:
                gdDebugApplicationTreeData* pProgramItemData = NULL;
                afApplicationTreeItemData* pItemData = NULL;
                createObjectItemData(pItemData, pProgramItemData);

                pItemData->m_itemType = AF_TREE_ITEM_CL_PROGRAMS_NODE;
                pItemData->m_objectMemorySize = 0;
                pItemData->m_objectCount = 0;

                pProgramItemData->_contextId = contextID;

                // Add the programs item to the tree:
                QTreeWidgetItem* pProgramsBaseTreeItemId = appendItemToTree(pContextTreeId, GD_STR_MemoryAnalysisViewerTreeCLProgramsNode, m_openCLProgramIconIndex, pItemData);
                GT_IF_WITH_ASSERT(pProgramsBaseTreeItemId != NULL)
                {
                    // Set the tree item id on the memory object data:
                    pItemData->m_objectCount += amountOfPrograms;

                    // Add each of the programs:
                    int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
                    int itemsToShow = min(amountOfPrograms, maxItemsShown);

                    for (int i = 0; i < itemsToShow; i++)
                    {
                        GT_IF_WITH_ASSERT(currentContextPrograms[i] != NULL)
                        {
                            // Build a program item data, and fill the program details:
                            afApplicationTreeItemData* pNewProgramItemData = NULL;
                            int imageIndex = m_openCLProgramIconIndex;
                            bool rc = buildCLProgramObjectData(contextID, currentContextProgramIndices[i], *currentContextPrograms[i], pNewProgramItemData, imageIndex);
                            retVal = retVal && rc;
                            GT_IF_WITH_ASSERT(rc && (pNewProgramItemData != NULL))
                            {
                                gtString programNameStr;
                                programNameStr.appendFormattedString(GD_STR_PropertiesCLProgramName, currentContextProgramIndices[i] + 1);

                                // Add AMD ext name:
                                setCLNodeTextFromStrAndData(programNameStr, pNewProgramItemData);

                                // Add this program item to the tree:
                                QTreeWidgetItem* pProgramTreeItemId = appendItemToTree(pProgramsBaseTreeItemId, programNameStr.asCharArray(), imageIndex, pNewProgramItemData);
                                retVal = retVal && (pProgramTreeItemId != NULL);

                                // Add the current program kernelHandles to the object tree:
                                gtUInt32 kernelsMemorySize = 0, kernelsAmount = 0;
                                bool rcKernels = updateCLKernelsList(contextID, pProgramTreeItemId, currentContextProgramIndices[i], *currentContextPrograms[i], kernelsMemorySize, kernelsAmount);
                                GT_ASSERT(rcKernels);

                                // Add the kernelHandles memory size:
                                pNewProgramItemData->m_objectMemorySize += kernelsMemorySize;

                                // Add the new program to the program total memory size:
                                pItemData->m_objectMemorySize += pNewProgramItemData->m_objectMemorySize;

                                // Increment the progress bar
                                afProgressBarWrapper::instance().incrementProgressBar();
                            }
                        }
                    }

                    // If some items were hidden:
                    if (maxItemsShown < amountOfPrograms)
                    {
                        // Show a message and count them in the parent (don't calculate their memory size to avoid slowdown and save communication channel transactions):
                        addMoreItemsMessage(pProgramsBaseTreeItemId, (amountOfPrograms - maxItemsShown));
                    }
                }

                // Return the programs node's data:
                programsMemorySize = pItemData->m_objectMemorySize;
                programsObjectCount = pItemData->m_objectCount;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateCLSubBuffersList
// Description: Update an OpenCL buffer sub buffers list
// Arguments:   int contextId
//              int ownerBufferIndex - the owner buffer index
//              bufferTreeId - the buffers' tree id
//              bufferDetails - the buffer details
//              subBuffersObjectsCount - the amount of sub buffers for this buffer
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/3/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateCLSubBuffersList(apContextID contextID, int ownerBufferIndex, QTreeWidgetItem* pBufferTreeId, const apCLBuffer& bufferDetails, int& subBuffersObjectsCount)
{
    bool retVal = true;

    // Get the sub buffers indices:
    const gtVector<int>& subBuffers = bufferDetails.subBuffersIndices();
    subBuffersObjectsCount = (int)subBuffers.size();

    // Iterate the sub buffers:
    int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
    int itemsToShow = min(subBuffersObjectsCount, maxItemsShown);

    for (int i = 0; i < itemsToShow; i++)
    {
        bool currentBufferUpdated = false;

        // Get the current sub buffer index:
        int subBufferIndex = subBuffers[i];

        // Get the sub buffer details
        apCLSubBuffer subBufferDetails;
        bool rcSubBuffer = gaGetOpenCLSubBufferObjectDetails(contextID._contextId, subBufferIndex, subBufferDetails);
        GT_IF_WITH_ASSERT(rcSubBuffer)
        {
            // Create a new memory item data, and fill the relevant fields:
            // TO_DO: Sub buffers - think how we want to handle memory issues:
            gdDebugApplicationTreeData* pSubBufferItemData = NULL;
            afApplicationTreeItemData* pItemData = NULL;
            createObjectItemData(pItemData, pSubBufferItemData);

            pItemData->m_itemType = AF_TREE_ITEM_CL_SUB_BUFFER;
            pItemData->m_objectCount = 0;
            pItemData->m_objectMemorySize = 0;

            pSubBufferItemData->_contextId = contextID;
            pSubBufferItemData->_objectOpenCLIndex = subBufferIndex;
            pSubBufferItemData->_objectOpenCLName = subBufferDetails.subBufferName();
            pSubBufferItemData->_objectOwnerName = bufferDetails.bufferName();
            pSubBufferItemData->_objectOpenCLOwnerIndex = ownerBufferIndex;

            // Fill memory data:
            fillCLSubBufferMemoryData(subBufferDetails, pItemData);

            // Build the sub buffer name:
            gtString subBufferDisplayName;
            subBufferDisplayName.appendFormattedString(GD_STR_TexturesViewerNameCLSubBuffer, subBufferIndex);

            // Add the current sub buffer to the tree:
            QTreeWidgetItem* pSubBufferTreeItemId = appendItemToTree(pBufferTreeId, subBufferDisplayName.asCharArray(), m_clBufferIconIndex, pItemData);
            GT_IF_WITH_ASSERT(pSubBufferTreeItemId != NULL)
            {
                // Set the tree item id:
                currentBufferUpdated = true;
            }
        }

        // Update the return value:
        retVal = retVal && currentBufferUpdated;
    }

    // If some items were hidden:
    if (maxItemsShown < subBuffersObjectsCount)
    {
        // Show a message:
        addMoreItemsMessage(pBufferTreeId, (subBuffersObjectsCount - maxItemsShown));
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateCLKernelsList
// Description: Updates a programs kernel objects
// Arguments:   apContextID contextID - the program context
//              int programIndex - the program index
//              gtUInt32& kernelsMemorySize - the program kernelHandles memory size
//              unsigned int& kernelsObjectsCount - the amount of kernelHandles attached to this program
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/10/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateCLKernelsList(apContextID contextID, QTreeWidgetItem* pProgramTreeId, int programIndex, const apCLProgram& programDetails, gtUInt32& kernelsMemorySize, unsigned int& kernelsObjectsCount)
{
    (void)(kernelsMemorySize);  // unused
    (void)(kernelsObjectsCount);  // unused
    bool retVal = true;

    // Get the object handles:
    const gtVector<oaCLKernelHandle>& progKernelHandles = programDetails.kernelHandles();

    // Iterate the kernelHandles:
    int numberOfKernels = (int)progKernelHandles.size();
    int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
    int itemsToShow = min(numberOfKernels, maxItemsShown);

    for (int i = 0; i < itemsToShow; i++)
    {
        // Get the kernel details
        apCLKernel kernelDetails;
        bool rcKernel = gaGetOpenCLKernelObjectDetails(contextID._contextId, progKernelHandles[i], kernelDetails);
        GT_IF_WITH_ASSERT(rcKernel)
        {
            // Create a new memory item data, and fill the relevant fields:
            gdDebugApplicationTreeData* pKernelItemData = NULL;
            afApplicationTreeItemData* pItemData = NULL;
            createObjectItemData(pItemData, pKernelItemData);

            pItemData->m_itemType = AF_TREE_ITEM_CL_KERNEL;
            pItemData->m_objectCount = 0;
            pItemData->m_objectMemorySize = 0;

            pKernelItemData->_contextId = contextID;
            pKernelItemData->_objectOpenCLIndex = i;
            pKernelItemData->_objectOpenCLName = i + 1;
            pKernelItemData->_objectOpenCLNameStr = kernelDetails.kernelName();
            pKernelItemData->_clProgramHandle = programDetails.programHandle();
            pKernelItemData->_clKernelHandle = kernelDetails.kernelHandle();
            pKernelItemData->_objectOwnerName = programIndex;
            pKernelItemData->_referenceCount = kernelDetails.referenceCount();

            // Build the kernel name:
            gtString kernelDisplayName = GD_STR_ShadersSourceCodeViewerListCtrlShaderNameIndentation;
            kernelDisplayName.appendFormattedString(GD_STR_ShadersSourceCodeViewerListCtrlOpenCLKernelName, i + 1);
            kernelDisplayName += L" - ";
            kernelDisplayName += kernelDetails.kernelFunctionName();

            // Add AMD ext Name:
            setCLNodeTextFromStrAndData(kernelDisplayName, pItemData);

            // Add the current kernel to the tree:
            rcKernel = false;
            QTreeWidgetItem* pKernelTreeItemId = appendItemToTree(pProgramTreeId, kernelDisplayName.asCharArray(), m_openCLKernelIconIndex, pItemData);
            GT_IF_WITH_ASSERT(pKernelTreeItemId != NULL)
            {
                // Set the kernel item id:
                rcKernel = true;
            }

            retVal = retVal && rcKernel;
        }
    }

    // If some items were hidden:
    if (maxItemsShown < numberOfKernels)
    {
        // Show a message:
        addMoreItemsMessage(pProgramTreeId, (numberOfKernels - maxItemsShown));
    }

    return retVal;

}
// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getCLContextLivingImageObjects
// Description: Fills the vector with the images in contextID which were not yet
//              marked for deletion
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        1/4/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::getCLContextLivingImageObjects(const apContextID& contextID, gtPtrVector<apCLImage*>& contextImages)
{
    bool retVal = false;

    // Clear the vector:
    contextImages.deleteElementsAndClear();

    // Get the (total) amount of images:
    int contextIndex = contextID._contextId;
    int numberOfImages = 0;
    bool rcNum = gaGetAmountOfOpenCLImageObjects(contextIndex, numberOfImages);
    GT_IF_WITH_ASSERT(rcNum)
    {
        retVal = true;

        // Get each image's data:
        for (int i = 0; i < numberOfImages; i++)
        {
            bool keepCurrentImage = false;
            apCLImage* pCurrentImage = new apCLImage;

            // Get the image details
            bool rcImg = gaGetOpenCLImageObjectDetails(contextIndex, i, (*pCurrentImage));
            GT_IF_WITH_ASSERT(rcImg)
            {
                // Only keep images that were not deleted:
                keepCurrentImage = !(pCurrentImage->wasMarkedForDeletion());
            }

            // Fail if we could not get an image's data:
            retVal = retVal && rcImg;

            if (keepCurrentImage)
            {
                // Add the image to the vector:
                contextImages.push_back(pCurrentImage);
            }
            else // !keepCurrentImage
            {
                // Dispose of the temporary object:
                delete pCurrentImage;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::buildCLImageObjectData
// Description: Builds an OpenCL texture object data
// Arguments: int contextId
//            GLuint imageName
//            gdTextureMemoryItemData*& pTextureItemData
//            int& imageIndex
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/12/2009
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::buildCLImageObjectData(apContextID contextID, int textureIndex, const apCLImage& textureDetails, afApplicationTreeItemData*& pItemData, int& imageIndex)
{
    bool retVal = false;

    retVal = true;

    // Increment the progress bar by the amount of mip levels:
    afProgressBarWrapper::instance().incrementProgressBar(1);

    pItemData = NULL;
    gdDebugApplicationTreeData* pTextureItemData = NULL;
    createObjectItemData(pItemData, pTextureItemData);

    pItemData->m_itemType = AF_TREE_ITEM_CL_IMAGE;
    pTextureItemData->_contextId = contextID;

    // Get the texture type:
    apTextureType texType = textureDetails.imageType();
    pTextureItemData->_textureType = texType;
    imageIndex = textureTypeToIconIndex(contextID, texType);

    // Set the texture id:
    pTextureItemData->_objectOpenCLName = textureDetails.imageName();
    pTextureItemData->_objectOpenCLNameStr = textureDetails.memObjectName();
    pTextureItemData->_objectOpenCLIndex = textureIndex;
    pTextureItemData->_referenceCount = textureDetails.referenceCount();

    // Try to get the OpenGL texture / render buffer details:
    GLuint textureName = textureDetails.openGLTextureName();
    GLuint renderBufferName = textureDetails.openGLRenderBufferName();

    // Set the OpenGL interop name:
    pTextureItemData->_objectOpenGLName = (textureName > 0) ? textureName : renderBufferName;

    // Get the texture data format as oaTexelDataFormat:
    oaTexelDataFormat dataFormat = OA_TEXEL_FORMAT_UNKNOWN;
    oaDataType dataType = OA_UNKNOWN_DATA_TYPE;

    // Get the data format as oaTexelDataFormat:
    bool rcTexelFormat = oaCLImageFormatToTexelFormat(textureDetails.dataFormat(), dataFormat);
    GT_ASSERT(rcTexelFormat);

    // Get the data type as oaDataType:
    bool rcDataType = oaCLImageDataTypeToOSDataType(textureDetails.dataType(), dataType);
    GT_ASSERT(rcDataType)

    pTextureItemData->_dataFormat = dataFormat;
    pTextureItemData->_dataType = dataType;

    // Get the memory flags:
    pTextureItemData->_memoryFlags = textureDetails.memoryFlags();

    // Fill the memory size:
    fillCLImageMemoryData(textureDetails, pItemData);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getCLContextLivingBufferObjects
// Description: Fills the vector with the buffers in contextID which were not yet
//              marked for deletion
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        1/4/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::getCLContextLivingBufferObjects(const apContextID& contextID, gtPtrVector<apCLBuffer*>& contextBuffers)
{
    bool retVal = false;

    // Clear the vector:
    contextBuffers.deleteElementsAndClear();

    // Get the (total) amount of buffers:
    int contextIndex = contextID._contextId;
    int numberOfBuffers = 0;
    bool rcNum = gaGetAmountOfOpenCLBufferObjects(contextIndex, numberOfBuffers);
    GT_IF_WITH_ASSERT(rcNum)
    {
        retVal = true;

        // Get each buffer's data:
        for (int i = 0; i < numberOfBuffers; i++)
        {
            bool keepCurrentBuffer = false;
            apCLBuffer* pCurrentBuffer = new apCLBuffer;

            // Get the buffer details
            bool rcBuf = gaGetOpenCLBufferObjectDetails(contextIndex, i, (*pCurrentBuffer));
            GT_IF_WITH_ASSERT(rcBuf)
            {
                // Only keep Buffers that were not deleted:
                keepCurrentBuffer = !(pCurrentBuffer->wasMarkedForDeletion());
            }

            // Fail if we could not get a buffer's data:
            retVal = retVal && rcBuf;

            if (keepCurrentBuffer)
            {
                // Add the buffer to the vector:
                contextBuffers.push_back(pCurrentBuffer);
            }
            else // !keepCurrentBuffer
            {
                // Dispose of the temporary object:
                delete pCurrentBuffer;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getCLContextLivingPipeObjects
// Description: Fills the vector with the pipes in contextID which were not yet
//              marked for deletion
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        1/10/2014
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::getCLContextLivingPipeObjects(const apContextID& contextID, gtPtrVector<apCLPipe*>& contextPipes)
{
    bool retVal = false;

    // Clear the vector:
    contextPipes.deleteElementsAndClear();

    // Get the (total) amount of pipes:
    int contextIndex = contextID._contextId;
    int numberOfPipes = 0;
    bool rcNum = gaGetAmountOfOpenCLPipeObjects(contextIndex, numberOfPipes);
    GT_IF_WITH_ASSERT(rcNum)
    {
        retVal = true;

        // Get each pipe's data:
        for (int i = 0; i < numberOfPipes; i++)
        {
            bool keepCurrentPipe = false;
            apCLPipe* pCurrentPipe = new apCLPipe;

            // Get the pipe details
            bool rcBuf = gaGetOpenCLPipeObjectDetails(contextIndex, i, (*pCurrentPipe));
            GT_IF_WITH_ASSERT(rcBuf)
            {
                // Only keep Pipes that were not deleted:
                keepCurrentPipe = !(pCurrentPipe->wasMarkedForDeletion());
            }

            // Fail if we could not get a pipe's data:
            retVal = retVal && rcBuf;

            if (keepCurrentPipe)
            {
                // Add the pipe to the vector:
                contextPipes.push_back(pCurrentPipe);
            }
            else // !keepCurrentPipe
            {
                // Dispose of the temporary object:
                delete pCurrentPipe;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getCLContextLivingSamplerObjects
// Description: Fills the vector with the samplers in contextID which were not yet
//              marked for deletion
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/5/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::getCLContextLivingSamplerObjects(const apContextID& contextID, gtPtrVector<apCLSampler*>& contextSamplers)
{
    bool retVal = false;

    // Clear the vector:
    contextSamplers.deleteElementsAndClear();

    // Get the (total) amount of samplers:
    int contextIndex = contextID._contextId;
    int numberOfSamplers = 0;
    bool rcNum = gaGetAmountOfOpenCLSamplers(contextIndex, numberOfSamplers);
    GT_IF_WITH_ASSERT(rcNum)
    {
        retVal = true;

        // Get each sampler's data:
        for (int i = 0; i < numberOfSamplers; i++)
        {
            bool keepCurrentSampler = false;
            apCLSampler* pCurrentSampler = new apCLSampler;

            // Get the sampler details
            bool rcSampler = gaGetOpenCLSamplerObjectDetails(contextIndex, i, (*pCurrentSampler));
            GT_IF_WITH_ASSERT(rcSampler)
            {
                // Only keep samplers that were not deleted:
                keepCurrentSampler = !(pCurrentSampler->wasMarkedForDeletion());
            }

            // Fail if we could not get a sampler's data:
            retVal = retVal && rcSampler;

            if (keepCurrentSampler)
            {
                // Add the buffer to the vector:
                contextSamplers.push_back(pCurrentSampler);
            }
            else // !keepCurrentSampler
            {
                // Dispose of the temporary object:
                delete pCurrentSampler;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getCLContextLivingEventObjects
// Description: Fills the vector with the events in contextID which were not yet
//              marked for deletion
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::getCLContextLivingEventObjects(const apContextID& contextID, gtPtrVector<apCLEvent*>& contextEvents)
{
    bool retVal = false;

    // Clear the vector:
    contextEvents.deleteElementsAndClear();

    // Get the (total) amount of events:
    int contextIndex = contextID._contextId;
    int numberOfEvents = 0;
    bool rcNum = gaGetAmountOfOpenCLEvents(contextIndex, numberOfEvents);
    GT_IF_WITH_ASSERT(rcNum)
    {
        retVal = true;

        // Get each Event's data:
        for (int i = 0; i < numberOfEvents; i++)
        {
            bool keepCurrentEvent = true;
            apCLEvent* pCurrentEvent = new apCLEvent(OA_CL_NULL_HANDLE, OA_CL_NULL_HANDLE, false);

            // Get the event details
            bool rcEvent = gaGetOpenCLEventObjectDetails(contextIndex, i, (*pCurrentEvent));
            GT_ASSERT(rcEvent);

            // Fail if we could not get an event's data:
            retVal = retVal && rcEvent;

            if (keepCurrentEvent)
            {
                // Add the event to the vector:
                contextEvents.push_back(pCurrentEvent);
            }
            else // !keepCurrentEvent
            {
                // Dispose of the temporary object:
                delete pCurrentEvent;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::buildCLBufferObjectData
// Description: Builds an OpenCL buffer object data
// Arguments: int contextId
//            gdCLBufferMemoryItemData*& pBufferItemData
//            int& imageIndex
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/12/2009
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::buildCLBufferObjectData(apContextID contextID, int bufferIndex, const apCLBuffer& bufferDetails, afApplicationTreeItemData*& pItemData, int& imageIndex)
{
    bool retVal = true;

    // Increment the progress bar by the amount of mip levels:
    afProgressBarWrapper::instance().incrementProgressBar(1);

    // Allocate a new buffer item data:
    pItemData = NULL;
    gdDebugApplicationTreeData* pBufferItemData = NULL;
    createObjectItemData(pItemData, pBufferItemData);

    pItemData->m_itemType = AF_TREE_ITEM_CL_BUFFER;
    pBufferItemData->_objectOpenCLIndex = bufferIndex;
    pBufferItemData->_objectOpenCLName = bufferDetails.bufferName();
    pBufferItemData->_referenceCount = bufferDetails.referenceCount();

    pBufferItemData->_contextId = contextID;
    pBufferItemData->_memoryFlags = bufferDetails.memoryFlags();

    // Set the buffer GL details:
    pBufferItemData->_objectOpenGLName = bufferDetails.openGLBufferName();

    imageIndex = m_clBufferIconIndex;

    // Fill memory data:
    fillCLBufferMemoryData(bufferDetails, pItemData);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::buildCLPipeObjectData
// Description: Builds an OpenCL pipe object data
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        1/10/2014
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::buildCLPipeObjectData(apContextID contextID, int pipeIndex, const apCLPipe& pipeDetails, afApplicationTreeItemData*& pItemData, int& imageIndex)
{
    bool retVal = true;

    // Increment the progress bar by 1, for the pipe object:
    afProgressBarWrapper::instance().incrementProgressBar(1);

    // Allocate a new pipe item data:
    pItemData = NULL;
    gdDebugApplicationTreeData* pPipeItemData = NULL;
    createObjectItemData(pItemData, pPipeItemData);

    pItemData->m_itemType = AF_TREE_ITEM_CL_PIPE;
    pPipeItemData->_objectOpenCLIndex = pipeIndex;
    pPipeItemData->_objectOpenCLName = pipeDetails.pipeName();
    pPipeItemData->_referenceCount = pipeDetails.referenceCount();

    pPipeItemData->_contextId = contextID;
    pPipeItemData->_memoryFlags = pipeDetails.memoryFlags();

    pPipeItemData->m_packetSize = pipeDetails.pipePacketSize();
    pPipeItemData->m_maxPackets = pipeDetails.pipeMaxPackets();

    imageIndex = m_clPipeIconIndex;

    // Fill memory data:
    fillCLPipeMemoryData(pipeDetails, pItemData);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::buildCLSamplerObjectData
// Description: Builds an OpenCL sampler object data
// Arguments:   int contextId - the context id
//              gdCLSamplerMemoryItemData*& pSamplerItemData - the item data to fill
//              const apCLSampler& samplerDetails - the sampler item details
//              int& imageIndex - the sampler icon index
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/5/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::buildCLSamplerObjectData(apContextID contextID, int samplerIndex, const apCLSampler& samplerDetails, afApplicationTreeItemData*& pItemData, int& imageIndex)
{
    bool retVal = true;

    // Increment the progress bar by the amount of mip levels:
    afProgressBarWrapper::instance().incrementProgressBar(1);

    pItemData = NULL;
    gdDebugApplicationTreeData* pSamplerItemData = NULL;
    createObjectItemData(pItemData, pSamplerItemData);

    pItemData->m_itemType = AF_TREE_ITEM_CL_SAMPLER;
    pSamplerItemData->_objectOpenCLIndex = samplerIndex;
    pSamplerItemData->_objectOpenCLName = samplerDetails.samplerId();

    pSamplerItemData->_contextId = contextID;
    pSamplerItemData->_addressingMode = samplerDetails.addressingMode();
    pSamplerItemData->_filterMode = samplerDetails.filterMode();

    // Set the object size in KB (we currently treat the sampler memory size as insignificant):
    pItemData->m_objectMemorySize = 0;

    imageIndex = m_clSamplerIconIndex;

    // Fill memory data:
    fillCLSamplerMemoryData(samplerDetails, pItemData);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::buildCLEventObjectData
// Description: Builds an OpenCL event object data
// Arguments:   int contextId - the context id
//              gdCLEventMemoryItemData*& pEventItemData - the item data to fill
//              const apCLEvent& eventDetails - the event item details
//              int& imageIndex - the event icon index
// Return Val:  bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::buildCLEventObjectData(apContextID contextID, int eventIndex, const apCLEvent& eventDetails, afApplicationTreeItemData*& pItemData, int& imageIndex)
{
    bool retVal = true;

    // Increment the progress bar by the amount of mip levels:
    afProgressBarWrapper::instance().incrementProgressBar(1);

    pItemData = NULL;
    gdDebugApplicationTreeData* pEventItemData = NULL;
    createObjectItemData(pItemData, pEventItemData);

    pItemData->m_itemType = AF_TREE_ITEM_CL_EVENT;
    pEventItemData->_objectOpenCLIndex = eventIndex;
    pEventItemData->_objectOpenCLName = eventIndex + 1;

    pEventItemData->_contextId = contextID;

    // Set the object size in KB (we currently treat the event memory size as insignificant):
    pItemData->m_objectMemorySize = 0;

    imageIndex = m_clEventIconIndex;

    // Fill memory data:
    fillCLEventMemoryData(eventDetails, pItemData);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getCLContextLivingQueueObjects
// Description: Fills the vector with the command queues in contextID which were not yet
//              marked for deletion
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        1/4/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::getCLContextLivingQueueObjects(const apContextID& contextID, gtPtrVector<apCLCommandQueue*>& contextQueues, gtVector<int>& queueIndices)
{
    bool retVal = false;

    // Clear the vector:
    contextQueues.deleteElementsAndClear();
    queueIndices.clear();

    // Get the (total) amount of queues:
    int contextIndex = contextID._contextId;
    int numberOfQueues = 0;
    bool rcNum = gaGetAmountOfCommandQueues(contextIndex, numberOfQueues);
    GT_IF_WITH_ASSERT(rcNum)
    {
        retVal = true;

        // Get each queue's data:
        for (int i = 0; i < numberOfQueues; i++)
        {
            bool keepCurrentQueue = false;
            apCLCommandQueue* pCurrentQueue = new apCLCommandQueue;

            // Get the queue details
            bool rcImg = gaGetCommandQueueDetails(contextIndex, i, (*pCurrentQueue));
            GT_IF_WITH_ASSERT(rcImg)
            {
                // Only keep queues that were not deleted:
                keepCurrentQueue = !(pCurrentQueue->wasMarkedForDeletion());
            }

            // Fail if we could not get a queue's data:
            retVal = retVal && rcImg;

            if (keepCurrentQueue)
            {
                // Add the queue to the vector:
                contextQueues.push_back(pCurrentQueue);
                queueIndices.push_back(i);
            }
            else // !keepCurrentQueue
            {
                // Dispose of the temporary object:
                delete pCurrentQueue;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::buildCLCommandQueueObjectData
// Description: Build an OpenCL command queue object data
// Arguments: apContextID contextID
//            int commandQueueId
//            gdCLCommandQueueMemoryItemData*& pCommandQueueItemData
//            int& imageIndex
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        16/2/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::buildCLCommandQueueObjectData(apContextID contextID, int commandQueueId, const apCLCommandQueue& commandQueueDetails, afApplicationTreeItemData*& pItemData, int& imageIndex)
{
    bool retVal = true;

    // Increment the progress bar by the amount of mip levels:
    afProgressBarWrapper::instance().incrementProgressBar(1);

    // Image index:
    imageIndex = m_commandQueueIconIndex;

    // Allocate an item data for the command queue:
    pItemData = NULL;
    gdDebugApplicationTreeData* pCommandQueueItemData = NULL;
    createObjectItemData(pItemData, pCommandQueueItemData);

    pItemData->m_itemType = AF_TREE_ITEM_CL_COMMAND_QUEUE;
    pCommandQueueItemData->_objectOpenCLIndex = commandQueueId;
    pCommandQueueItemData->_objectOpenCLName = commandQueueId + 1;
    pCommandQueueItemData->_objectOpenCLNameStr = commandQueueDetails.queueName();
    pCommandQueueItemData->_referenceCount = commandQueueDetails.referenceCount();

    // Update the command queue item data with the right details:
    pCommandQueueItemData->_contextId = contextID;
    pCommandQueueItemData->_referenceCount = commandQueueDetails.referenceCount();
    pCommandQueueItemData->m_queueOutOfOrderExecutionMode = commandQueueDetails.outOfOrderExecutionModeEnable();
    pCommandQueueItemData->m_queueProfilingMode = commandQueueDetails.profilingModeEnable();
    pCommandQueueItemData->m_queueOnDevice = commandQueueDetails.queueOnDevice();
    pCommandQueueItemData->m_queueOnDeviceDefault = commandQueueDetails.isDefaultOnDeviceQueue();

    bool rc = gaGetAmountOfEventsInQueue(commandQueueDetails.commandQueueHandle(), pCommandQueueItemData->_amountOfEvents);
    GT_ASSERT(rc);

    // Set the object size in KB (we currently treat the command queue memory size as insignificant):
    pItemData->m_objectMemorySize = 0;

    // Fill memory data:
    fillCLCommandQueueMemoryData(commandQueueDetails, pItemData);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getCLContextLivingProgramObjects
// Description: Fills the vector with the programs in contextID which were not yet
//              marked for deletion
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        1/4/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::getCLContextLivingProgramObjects(const apContextID& contextID, gtPtrVector<apCLProgram*>& contextPrograms, gtVector<int>& programIndices)
{
    bool retVal = false;

    // Clear the vector:
    contextPrograms.deleteElementsAndClear();
    programIndices.clear();

    // Get the (total) amount of programs:
    int contextIndex = contextID._contextId;
    int numberOfPrograms = 0;
    bool rcNum = gaGetAmountOfOpenCLProgramObjects(contextIndex, numberOfPrograms);
    GT_IF_WITH_ASSERT(rcNum)
    {
        retVal = true;

        // Get each program's data:
        for (int i = 0; i < numberOfPrograms; i++)
        {
            bool keepCurrentProgram = false;
            apCLProgram* pCurrentProgram = new apCLProgram(0);

            // Get the program details
            bool rcBuf = gaGetOpenCLProgramObjectDetails(contextIndex, i, (*pCurrentProgram));
            GT_IF_WITH_ASSERT(rcBuf)
            {
                // Only keep Programs that were not deleted:
                keepCurrentProgram = !(pCurrentProgram->wasMarkedForDeletion());
            }

            // Fail if we could not get a program's data:
            retVal = retVal && rcBuf;

            if (keepCurrentProgram)
            {
                // Add the Program to the vector:
                contextPrograms.push_back(pCurrentProgram);
                programIndices.push_back(i);
            }
            else // !keepCurrentProgram
            {
                // Dispose of the temporary object:
                delete pCurrentProgram;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::buildCLProgramObjectData
// Description: Build an OpenCL program object data
// Arguments:   apContextID contextID
//            int programId
//            gdCLProgramMemoryItemData*& pProgramItemData
//            int& imageIndex
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        9/3/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::buildCLProgramObjectData(apContextID contextID, int programId, const apCLProgram& programDetails, afApplicationTreeItemData*& pItemData, int& imageIndex)
{
    bool retVal = true;

    // Increment the progress bar by the amount of mip levels:
    afProgressBarWrapper::instance().incrementProgressBar(1);

    // Image index:
    imageIndex = m_openCLProgramIconIndex;

    // Allocate an item data for the command queue:
    pItemData = NULL;
    gdDebugApplicationTreeData* pProgramItemData = NULL;
    createObjectItemData(pItemData, pProgramItemData);

    pItemData->m_itemType = AF_TREE_ITEM_CL_PROGRAM;
    pProgramItemData->_objectOpenCLIndex = programId;
    pProgramItemData->_objectOpenCLName = programId + 1;
    pProgramItemData->_objectOpenCLNameStr = programDetails.programName();
    pProgramItemData->_clProgramHandle = programDetails.programHandle();
    pProgramItemData->_referenceCount = programDetails.referenceCount();

    // Update the command queue item data with the right details:
    pProgramItemData->_contextId = contextID;

    // Set the object size in KB (we currently treat the program memory size as insignificant):
    pItemData->m_objectMemorySize = 0;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::markSharingContexts
// Description: Mark each of the sharing contexts with a
// Arguments:   const gtVector<int>& vListSharingContexts
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        13/7/2010
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::markSharingContexts(const gtVector<apContextID>& vListSharingContexts)
{
    // Mark the shared contexts (ones that other contexts are sharing) as such:
    int n = vListSharingContexts.size();

    for (int j = 0; j < n; j++)
    {
        // Get the shared context tree item id:
        apContextID currentContext = vListSharingContexts[j];
        QTreeWidgetItem* pSharedContextTreeId = getContextTreeItemId(currentContext);
        GT_IF_WITH_ASSERT((pSharedContextTreeId != m_pHeaderItem) && (pSharedContextTreeId != NULL))
        {
            gdDebugApplicationTreeData* pContextData = NULL;
            afApplicationTreeItemData* pItemData = NULL;
            getTreeItemDatas(pSharedContextTreeId, pItemData, pContextData);
            GT_IF_WITH_ASSERT((pContextData != NULL) && (pItemData != NULL))
            {
                bool isDeleted = pContextData->_isMarkedForDeletion;

                // Get the current image index:
                int imageIndex = isDeleted ? m_renderContextDeletedSharedIconIndex : m_renderContextSharedIconIndex;

                // Set the item icon:
                setItemIcon(pSharedContextTreeId, imageIndex);

                if (pSharedContextTreeId->childCount() > 0)
                {
                    pSharedContextTreeId->setExpanded(true);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::onEvent
// Description: Is called when a debugged process event occurs.
// Arguments:   const apEvent& eve - A class representing the event.
// Author:      Sigal Algranaty
// Date:        27/9/2010
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent);  // unused
    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    GT_IF_WITH_ASSERT(m_pApplicationTree != NULL)
    {
        // Update toolbar buttons on every event:
        m_pApplicationTree->updateToolbarButtons();

        switch (eventType)
        {
            case apEvent::AP_DEBUGGED_PROCESS_CREATED:
            case apEvent::AP_DEBUGGED_PROCESS_RUN_STARTED:
            case apEvent::AP_API_CONNECTION_ENDED:
            {
                // Do not respond to selection event while not suspended:
                m_pApplicationTree->setIgnoreSelection(true);

                // Update the tree:
                clearTreeItems(false);

                m_isInfoUpdated = false;
                m_isDebuggedProcessSuspended =  false;

                // When the process is done, clear the selected items list:
                m_pApplicationTree->resetLastSelectedItem();

                if (m_pLastMemoryLeakEvent != NULL)
                {
                    delete m_pLastMemoryLeakEvent;
                    m_pLastMemoryLeakEvent = NULL;
                }

                // Update opened views:
                gdImagesAndBuffersManager::instance().updateOpenedViewsOnEvent(gaIsDebuggedProcessSuspended());

                // Update the tree root text:
                m_pApplicationTree->updateTreeRootText();
            }
            break;

            case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
            {
                // Do not respond to selection event while not suspended:
                m_pApplicationTree->setIgnoreSelection(true);

                clearTreeItems(true);
                m_isInfoUpdated = false;
                m_isDebuggedProcessSuspended = false;

                if (m_pLastMemoryLeakEvent != NULL)
                {
                    delete m_pLastMemoryLeakEvent;
                    m_pLastMemoryLeakEvent = NULL;
                }

                // Update opened views:
                gdImagesAndBuffersManager::instance().updateOpenedViewsOnEvent(gaIsDebuggedProcessSuspended());

                // Update the tree root text:
                m_pApplicationTree->updateTreeRootText();
            }
            break;

            case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
            {
                // Do not respond to selection event while not suspended:
                m_pApplicationTree->setIgnoreSelection(true);

                // Update the tree:
                clearTreeItems(false);
                m_isInfoUpdated = false;
                m_isDebuggedProcessSuspended =  false;

                if (m_pLastMemoryLeakEvent != NULL)
                {
                    delete m_pLastMemoryLeakEvent;
                    m_pLastMemoryLeakEvent = NULL;
                }

                // Update opened views:
                gdImagesAndBuffersManager::instance().updateOpenedViewsOnEvent(gaIsDebuggedProcessSuspended());

                // Update the tree root text:
                m_pApplicationTree->updateTreeRootText();
            }
            break;

            case apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED:
            {
                // Do not ignore selections while process is suspended:
                m_pApplicationTree->setIgnoreSelection(false);

                // Set the debugged process suspension flag:
                m_isDebuggedProcessSuspended = true;

                // Update the tree:
                const apDebuggedProcessRunSuspendedEvent& suspendedEvent = (const apDebuggedProcessRunSuspendedEvent&)eve;

                if (!suspendedEvent.IsHostBreakPointReason())
                {
                    updateMonitoredObjectsTree();

                    // Select the debugged kernel in the monitored tree:
                    handleKernelDebuggingSelection();

                    // Update opened views:
                    gdImagesAndBuffersManager::instance().updateOpenedViewsOnEvent(gaIsDebuggedProcessSuspended());
                }

                // Update the tree root text:
                m_pApplicationTree->updateTreeRootText();
            }
            break;

            case apEvent::APP_GLOBAL_VARIABLE_CHANGED:
            {
                // If there is a new project, set the root name:
                const afGlobalVariableChangedEvent& variableChangedEvent = (const afGlobalVariableChangedEvent&)eve;

                if (variableChangedEvent.changedVariableId() == afGlobalVariableChangedEvent::CURRENT_PROJECT)
                {
                    clearTreeItems(true);
                    m_isInfoUpdated = false;

                    // Update the tree to create the root with the new application name:
                    updateMonitoredObjectsTree();
                }
            }
            break;

            case apEvent::AP_MEMORY_LEAK:
            {
                // Save the last memory leak event:
                const apMemoryLeakEvent& memoryLeakEvent = (const apMemoryLeakEvent&)eve;
                m_pLastMemoryLeakEvent = (apMemoryLeakEvent*)memoryLeakEvent.clone();
            }
            break;

            case apEvent::AP_BREAKPOINT_HIT:
            {
                // Update the tree:
                const apBreakpointHitEvent& breakpointEvent = ((const apBreakpointHitEvent&)eve);

                bool rcUpdate = true;

                if (AP_BREAK_COMMAND_HIT != breakpointEvent.breakReason())
                {
                    rcUpdate = updateMonitoredObjectsTree();
                }

                GT_IF_WITH_ASSERT(rcUpdate)
                {
                    // Down cast the event to a breakpoint hit event:
                    if (breakpointEvent.breakReason() == AP_MEMORY_LEAK_BREAKPOINT_HIT)
                    {
                        // Perform memory leak breakpoint operations:
                        GT_IF_WITH_ASSERT(m_pLastMemoryLeakEvent != NULL)
                        {
                            onMemoryLeakBreakpoint(*m_pLastMemoryLeakEvent);
                        }
                    }
                }
            }
            break;

            case apEvent::GD_MONITORED_OBJECT_ACTIVATED_EVENT:
            {
                // Get the activation event:
                const apMonitoredObjectsTreeActivatedEvent& activationEvent = (const apMonitoredObjectsTreeActivatedEvent&)eve;

                // Get the item data;
                afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)activationEvent.selectedItemData();

                if (pItemData != NULL)
                {
                    gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());

                    if (pGDData != NULL)
                    {
                        // Display the item (is this is a debugger tree node):
                        bool rc = activateItem(pItemData->m_pTreeWidgetItem);
                        GT_ASSERT(rc);
                    }
                }
            }
            break;

            default:
                // Do nothing...
                break;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::activateItem
// Description: Activates the requested item
// Arguments:   QTreeWidgetItem* pItemToActivate
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/7/2011
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::activateItem(QTreeWidgetItem* pItemToActivate)
{
    bool retVal = false;

    // Get the tree item data:
    m_pActivatedItemData = getTreeItemData(pItemToActivate);

    if (m_pActivatedItemData != NULL)
    {
        // Get the activated object type:
        afTreeItemType itemType = m_pActivatedItemData->m_itemType;
        retVal = true;

        switch (itemType)
        {
            case AF_TREE_ITEM_CL_KERNEL:
            case AF_TREE_ITEM_CL_PROGRAM:
            {
                GT_IF_WITH_ASSERT(m_pApplicationCommands != NULL)
                {
                    // Display the selected kernel / program source code:
                    m_pApplicationCommands->displayOpenCLProgramSourceCode(m_pActivatedItemData);
                    retVal = true;
                }
            }
            break;

            case AF_TREE_ITEM_GL_VERTEX_SHADER:
            case AF_TREE_ITEM_GL_TESSELLATION_CONTROL_SHADER:
            case AF_TREE_ITEM_GL_TESSELLATION_EVALUATION_SHADER:
            case AF_TREE_ITEM_GL_GEOMETRY_SHADER:
            case AF_TREE_ITEM_GL_FRAGMENT_SHADER:
            case AF_TREE_ITEM_GL_COMPUTE_SHADER:
            case AF_TREE_ITEM_GL_UNSUPPORTED_SHADER:
            {
                GT_IF_WITH_ASSERT(m_pApplicationCommands != NULL)
                {
                    // Display the selected shader source code:
                    m_pApplicationCommands->displayOpenGLSLShaderCode(m_pActivatedItemData);
                    retVal = true;
                }
            }
            break;

            case AF_TREE_ITEM_GL_FBO_ATTACHMENT:
            {
                // Open the FBO attachment item:
                bool rcOpenFBOAttach = openShortcutItem(m_pActivatedItemData);
                GT_ASSERT(rcOpenFBOAttach);
            }
            break;

            case AF_TREE_ITEM_GL_TEXTURE:
            case AF_TREE_ITEM_GL_VBO:
            case AF_TREE_ITEM_CL_IMAGE:
            case AF_TREE_ITEM_CL_BUFFER:
            case AF_TREE_ITEM_CL_SUB_BUFFER:
            case AF_TREE_ITEM_GL_STATIC_BUFFER:
            case AF_TREE_ITEM_GL_RENDER_BUFFER:
            case AF_TREE_ITEM_GL_TEXTURES_NODE:
            case AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE:
            case AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE:
            case AF_TREE_ITEM_GL_PBUFFERS_NODE:
            case AF_TREE_ITEM_GL_PBUFFER_NODE:
            case AF_TREE_ITEM_GL_VBO_NODE:
            case AF_TREE_ITEM_GL_FBO_NODE:
            case AF_TREE_ITEM_CL_IMAGES_NODE:
            case AF_TREE_ITEM_CL_BUFFERS_NODE:
            case AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER:
            {
                // Open this buffer / image object:
                bool rcOpenImageBufferObject = openImageBufferObject(m_pActivatedItemData);
                GT_ASSERT(rcOpenImageBufferObject);
            }
            break;

            default:
                retVal = false;
                break;
        }

        // Add the item to the list of selected items:
        m_pApplicationTree->addSelectedItem(m_pActivatedItemData, true);

        // Update the back / forward buttons:
        m_pApplicationTree->updateToolbarButtons();
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::countTexturesByType
// Description: Count the textures by its types, and set this data on the textures node item data
// Arguments:   gdDebugApplicationTreeData* pTexturesNodeItemData
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        14/10/2010
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::countTexturesByType(gdDebugApplicationTreeData* pTexturesNodeItemData)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pTexturesNodeItemData != NULL)
    {
        // Initialize the map (so that the types are sorted when used):
        for (int textureType = (int)AP_1D_TEXTURE; textureType <= (int)AP_AMOUNT_OF_TEXTURE_BIND_TARGETS; textureType++)
        {
            pTexturesNodeItemData->_textureTypesAmount[(apTextureType)textureType] = 0;
        }

        // Get the amount of textures for this context:
        int amountOfTextureObjects = 0;
        bool rcTexAmount = gaGetAmountOfTextureObjects(pTexturesNodeItemData->_contextId._contextId, amountOfTextureObjects);
        GT_ASSERT(rcTexAmount);

        // Iterate the textures:
        for (int i = 0; i < amountOfTextureObjects; i++)
        {
            // Get the current textures type:
            apTextureType textureType = AP_UNKNOWN_TEXTURE_TYPE;
            bool rcGetType = gaGetTextureObjectType(pTexturesNodeItemData->_contextId._contextId, i, textureType);
            GT_IF_WITH_ASSERT(rcGetType)
            {
                // Add the texture type to the map:
                pTexturesNodeItemData->_textureTypesAmount[textureType] ++;
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::countShadersByType
// Description: Count the shaders by its types, and set this data on the shaders node item data
// Return Val:  bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        19/11/2008
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::countShadersByType(gdDebugApplicationTreeData* pShadersNodeItemData)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pShadersNodeItemData != NULL)
    {
        // Initialize the map (so that the types are sorted when used):
        for (int shaderType = GD_VERTEX_SHADER; shaderType < (int)GD_UNKNOWN_SHADER; shaderType++)
        {
            pShadersNodeItemData->_shaderTypesAmount[(gdShaderType)shaderType] = 0;
        }

        // Set the total amount of shaders to be 0:
        int amountOfShaders = 0;

        // Get the amount of shaders for this context:
        bool rcGetAmount = gaGetAmountOfShaderObjects(pShadersNodeItemData->_contextId._contextId, amountOfShaders);
        GT_IF_WITH_ASSERT(rcGetAmount)
        {
            for (int i = 0; i < amountOfShaders; i++)
            {
                // Get the shader name:
                GLuint shaderName = 0;
                bool rc = gaGetShaderObjectName(pShadersNodeItemData->_contextId._contextId, i, shaderName);
                GT_IF_WITH_ASSERT(rc)
                {
                    gtAutoPtr<apGLShaderObject> aptrShaderDetails = NULL;
                    rc = gaGetShaderObjectDetails(pShadersNodeItemData->_contextId._contextId, shaderName, aptrShaderDetails);
                    GT_IF_WITH_ASSERT(rc && (aptrShaderDetails.pointedObject() != NULL))
                    {
                        osTransferableObjectType shaderObjectType = aptrShaderDetails->type();

                        // Get the shader type:
                        afTreeItemType objType;
                        gdShaderType shaderType = gdShaderTypeFromTransferableObjectType(shaderObjectType, objType);

                        // Add the shader to the mapping:
                        pShadersNodeItemData->_shaderTypesAmount[(gdShaderType)shaderType] ++ ;
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdTreeViewerBase::getItemDataByType
// Description: Return the item data related to the item located in index itemIndex,
//              with the requested item type. This function is used for optimization,
//              in order to avoid the navigation of the tree
// Arguments:   gdTexturesAndBuffersItemType itemType
//              int itemIndex
// Return Val:  afApplicationTreeItemData*
// Author:      Sigal Algranaty
// Date:        11/5/2009
// ---------------------------------------------------------------------------
afApplicationTreeItemData* gdDebugApplicationTreeHandler::getItemDataByType(const apContextID& contextID, afTreeItemType itemType, int itemIndex) const
{
    afApplicationTreeItemData* pRetVal = NULL;

    // Define a tree item id object with the details:
    gdTreeItemId itemID(contextID, itemType, itemIndex);

    // Find the item index in the map:
    gtMap<gdTreeItemId, afApplicationTreeItemData*>::const_iterator iterFind = m_itemIndexToItemDataMap.find(itemID);

    if (iterFind != m_itemIndexToItemDataMap.end())
    {
        pRetVal = (*iterFind).second;
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::appendItemToTree
// Description: Append an item to the tree, and add the item id and data to the mapping
//              NOTICE: Do not append items to the tree using AppendItem directly, but
//              use this function, cause it add the item to the mapping
// Arguments:   parentTree - the parent in the tree
//              itemLabel - the item lacel
//              itemImageIndex - image index
//              gdDebugApplicationTreeData* pItemTreeData - the item data
// Return Val:  QTreeWidgetItem
// Author:      Sigal Algranaty
// Date:        31/10/2010
// ---------------------------------------------------------------------------
QTreeWidgetItem* gdDebugApplicationTreeHandler::appendItemToTree(QTreeWidgetItem* pParentTree, const gtString& itemLabel, int itemImageIndex, afApplicationTreeItemData* pItemTreeData)
{
    QTreeWidgetItem* pRetVal = NULL;

    // Sanity check:
    GT_IF_WITH_ASSERT((pParentTree != NULL) && (pItemTreeData != NULL) && (m_pApplicationTree != NULL))
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Add the node to the tree:
            QStringList itemStrings;
            pRetVal = m_pApplicationTree->addTreeItem(itemLabel, pItemTreeData, pParentTree);
            GT_IF_WITH_ASSERT(pRetVal != NULL)
            {
                // Set the icon:
                setItemIcon(pRetVal, itemImageIndex);

                // Define the item index:
                pItemTreeData->m_itemTreeIndex = -1;

                // If this item is supposed to be an item with similar 'brothers', find its real index:
                if (!afApplicationTreeItemData::isItemTypeRoot(pItemTreeData->m_itemType))
                {
                    // Get the 'real' item index:
                    pItemTreeData->m_itemTreeIndex = pParentTree->childCount() - 1;
                }

                // Define tree item id for the mapping:
                gdTreeItemId itemId(pGDData->_contextId, pItemTreeData->m_itemType, pItemTreeData->m_itemTreeIndex) ;

                // Set the item index on the item id structure;
                itemId.m_itemIndex = pItemTreeData->m_itemTreeIndex;

                // Add the item data to the map:
                m_itemIndexToItemDataMap.insert(pair<gdTreeItemId, afApplicationTreeItemData*>(itemId, pItemTreeData));
                pItemTreeData->m_pTreeWidgetItem = pRetVal;
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getOpenCLObjectDataByIndex
// Description: Return an OpenCL item data by its index and parent type
// Arguments:   const apContextID& contextID
//              afTreeItemType objectType
//              int objectIndex
// Return Val:  gdDebugApplicationTreeData
// Author:      Sigal Algranaty
// Date:        31/10/2010
// ---------------------------------------------------------------------------
afApplicationTreeItemData* gdDebugApplicationTreeHandler::getOpenCLObjectDataByIndex(const apContextID& contextID, afTreeItemType objectType, int objectIndex)
{
    afApplicationTreeItemData* retVal = NULL;

    // Get the item type for the parent of this object:
    afTreeItemType parentObjectType = afApplicationTreeItemData::itemTypeToParent(objectType);
    GT_IF_WITH_ASSERT(objectType != AF_TREE_ITEM_ITEM_NONE)
    {
        // Get the cl objects tree root id:
        afApplicationTreeItemData* pCLRootData = getItemDataByType(contextID, parentObjectType);

        if (pCLRootData != NULL)
        {
            GT_IF_WITH_ASSERT(pCLRootData->m_pTreeWidgetItem != NULL)
            {
                for (int i = 0 ; i < pCLRootData->m_pTreeWidgetItem->childCount(); i++)
                {
                    // Get the current child:
                    QTreeWidgetItem* pTreeChild = pCLRootData->m_pTreeWidgetItem->child(i);
                    GT_IF_WITH_ASSERT(pTreeChild != NULL)
                    {
                        // Get the item data from the tree item id:
                        afApplicationTreeItemData* pViewerItem = NULL;
                        gdDebugApplicationTreeData* pGDData = NULL;
                        getTreeItemDatas(pTreeChild, pViewerItem, pGDData);

                        if (pViewerItem != NULL)
                        {
                            // Is this the item type we are looking for?
                            if (pViewerItem->m_itemType == objectType)
                            {
                                // Is this the buffer we were looking for?
                                if (pGDData->_objectOpenCLIndex == objectIndex)
                                {
                                    // Item was found, select it:
                                    retVal = pViewerItem;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getOpenCLObjectDataByName
// Description: Return an OpenCL item data by its index and parent type
// Arguments:   const apContextID& contextID
//              afTreeItemType objectType
//              int objectIndex
// Return Val:  afApplicationTreeItemData
// Author:      Sigal Algranaty
// Date:        31/10/2010
// ---------------------------------------------------------------------------
afApplicationTreeItemData* gdDebugApplicationTreeHandler::getOpenCLObjectDataByName(const apContextID& contextID, afTreeItemType objectType, int objectName, int ownerObjectIndex)
{
    afApplicationTreeItemData* pRetVal = NULL;

    // Get the item type for the parent of this object:
    afTreeItemType parentObjectType = afApplicationTreeItemData::itemTypeToParent(objectType);
    GT_IF_WITH_ASSERT(objectType != AF_TREE_ITEM_ITEM_NONE)
    {
        afApplicationTreeItemData* pCLRootData = getItemDataByType(contextID, parentObjectType, ownerObjectIndex);

        if (pCLRootData != NULL)
        {
            GT_IF_WITH_ASSERT(pCLRootData->m_pTreeWidgetItem != NULL)
            {
                for (int i = 0 ; i < pCLRootData->m_pTreeWidgetItem->childCount(); i++)
                {
                    // Get the current child:
                    QTreeWidgetItem* pChild = pCLRootData->m_pTreeWidgetItem->child(i);
                    GT_IF_WITH_ASSERT(pChild != NULL)
                    {
                        // Get the item data from the tree item id:
                        afApplicationTreeItemData* pViewerItem = NULL;
                        gdDebugApplicationTreeData* pGDData = NULL;
                        getTreeItemDatas(pChild, pViewerItem, pGDData);

                        if (pGDData != NULL)
                        {
                            // Is this the item type we are looking for?
                            if (pViewerItem->m_itemType == objectType)
                            {
                                // Is this the buffer we were looking for?
                                if (pGDData->_objectOpenCLName == objectName)
                                {
                                    // Item was found, select it:
                                    pRetVal = pViewerItem;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::selectOpenGLObjectByName
// Description: Select an OpenGL object by its OpenGL name
// Arguments:   const apContextID& contextID
//              afTreeItemType rootObjectType
//              GLuint objectOpenGLName
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        31/10/2010
// ---------------------------------------------------------------------------
afApplicationTreeItemData* gdDebugApplicationTreeHandler::getOpenGLObjectDataByName(const afApplicationTreeItemData& objectID)
{
    afApplicationTreeItemData* pRetVal = NULL;

    gdDebugApplicationTreeData* pOriginalGDData = qobject_cast<gdDebugApplicationTreeData*>(objectID.extendedItemData());
    GT_IF_WITH_ASSERT(pOriginalGDData != NULL)
    {
        // Get the item type for the parent of this object:
        afTreeItemType parentObjectType = afApplicationTreeItemData::itemTypeToParent(objectID.m_itemType);
        GT_IF_WITH_ASSERT(objectID.m_itemType != AF_TREE_ITEM_ITEM_NONE)
        {
            // Get the cl objects tree root id:
            afApplicationTreeItemData* pGLRootData = getItemDataByType(pOriginalGDData->_contextId, parentObjectType);

            if (pGLRootData != NULL)
            {
                GT_IF_WITH_ASSERT(pGLRootData->m_pTreeWidgetItem != NULL)
                {
                    for (int i = 0 ; i < pGLRootData->m_pTreeWidgetItem->childCount(); i++)
                    {
                        // Get the current child:
                        QTreeWidgetItem* pChild = pGLRootData->m_pTreeWidgetItem->child(i);
                        GT_IF_WITH_ASSERT(pChild != NULL)
                        {
                            // Get the item data from the tree item id:
                            afApplicationTreeItemData* pItemData = NULL;
                            gdDebugApplicationTreeData* pGDData = NULL;
                            getTreeItemDatas(pChild, pItemData, pGDData);

                            if ((pItemData != NULL) && (pGDData != NULL))
                            {
                                // Is this the item type we are looking for?
                                if (pItemData->m_itemType == objectID.m_itemType)
                                {
                                    bool sameObject = false;

                                    if (pItemData->m_itemType == AF_TREE_ITEM_GL_STATIC_BUFFER)
                                    {
                                        sameObject = (pGDData->_bufferType == pOriginalGDData->_bufferType);
                                    }
                                    else
                                    {
                                        sameObject = (pGDData->_objectOpenGLName == pOriginalGDData->_objectOpenGLName);
                                    }

                                    if (sameObject)
                                    {
                                        // Item was found, select it:
                                        pRetVal = pItemData;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::selectOpenGLObjectByName
// Description: Return an OpenGL context data by the context id
// Arguments:   const apContextID& contextID
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/2/2011
// ---------------------------------------------------------------------------
afApplicationTreeItemData* gdDebugApplicationTreeHandler::getOpenGLContextData(const apContextID& contextID)
{
    afApplicationTreeItemData* pRetVal = NULL;

    // Iterate the context tree item ids:
    gtVector<QTreeWidgetItem*>::iterator iter = m_openGLContextsTreeIds.begin();
    gtVector<QTreeWidgetItem*>::iterator iterEnd = m_openGLContextsTreeIds.end();

    for (; iter != iterEnd; iter++)
    {
        // Get the tree item id:
        QTreeWidgetItem* pContextTreeItemId = *iter;
        GT_IF_WITH_ASSERT(pContextTreeItemId != NULL)
        {
            // Get the context tree item data:
            gdDebugApplicationTreeData* pContextData = NULL;
            afApplicationTreeItemData* pItemData = NULL;
            getTreeItemDatas(pContextTreeItemId, pItemData, pContextData);
            GT_IF_WITH_ASSERT(pContextData != NULL)
            {
                // Check if this is the requested context:
                if ((contextID == pContextData->_contextId) && (pItemData->m_itemType == AF_TREE_ITEM_GL_RENDER_CONTEXT))
                {
                    pRetVal = pItemData;
                    break;
                }
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getOpenCLContextData
// Description: Return an OpenCL context data by the context id
// Arguments:   const apContextID& contextID
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/2/2011
// ---------------------------------------------------------------------------
afApplicationTreeItemData* gdDebugApplicationTreeHandler::getOpenCLContextData(const apContextID& contextID)
{
    afApplicationTreeItemData* pRetVal = NULL;

    // Iterate the context tree item ids:
    gtVector<QTreeWidgetItem*>::iterator iter = m_openCLContextsTreeIds.begin();
    gtVector<QTreeWidgetItem*>::iterator iterEnd = m_openCLContextsTreeIds.end();

    for (; iter != iterEnd; iter++)
    {
        // Get the tree item id:
        QTreeWidgetItem* pContextTreeItemId = *iter;
        GT_IF_WITH_ASSERT(pContextTreeItemId != NULL)
        {
            // Get the context tree item data:
            afApplicationTreeItemData* pContextData = getTreeItemData(pContextTreeItemId);
            GT_IF_WITH_ASSERT(pContextData != NULL)
            {
                gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pContextData->extendedItemData());
                GT_IF_WITH_ASSERT(pGDData != NULL)
                {
                    // Check if this is the requested context:
                    if ((contextID == pGDData->_contextId) && (pContextData->m_itemType == AF_TREE_ITEM_CL_CONTEXT))
                    {
                        pRetVal = pContextData;
                        break;
                    }
                }
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getStaticBufferDataByType
// Description: Return an item data for the tree item with the input buffer type for a context
// Arguments:   apDisplayBuffer bufferType
// Return Val:  afApplicationTreeItemData*
// Author:      Sigal Algranaty
// Date:        31/10/2010
// ---------------------------------------------------------------------------
afApplicationTreeItemData* gdDebugApplicationTreeHandler::getStaticBufferDataByType(const apContextID& contextID, apDisplayBuffer bufferType)
{
    afApplicationTreeItemData* retVal = NULL;

    // Get the cl objects tree root id:
    afApplicationTreeItemData* pStaticBuffersRoot = getItemDataByType(contextID, AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE);

    if (pStaticBuffersRoot != NULL)
    {
        GT_IF_WITH_ASSERT(pStaticBuffersRoot->m_pTreeWidgetItem != NULL)
        {
            for (int i = 0 ; i < pStaticBuffersRoot->m_pTreeWidgetItem->childCount(); i++)
            {
                // Get the current child:
                QTreeWidgetItem* pTreeItem = pStaticBuffersRoot->m_pTreeWidgetItem->child(i);

                // Get the item data from the tree item id:
                afApplicationTreeItemData* pViewerItem = NULL;
                gdDebugApplicationTreeData* pGDData = NULL;
                getTreeItemDatas(pTreeItem, pViewerItem, pGDData);

                if ((pViewerItem != NULL) && (pGDData != NULL))
                {
                    // Is this the item type we are looking for?
                    if (pGDData->_bufferType == bufferType)
                    {
                        // Item was found, select it:
                        retVal = pViewerItem;
                        break;
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getFBODataByFBOAttachment
// Description: Return an item data for an FBO attachment
// Arguments:   const apContextID& contextID
//              GLuint fboName
//              GLenum fboAttachment
// Return Val:  afApplicationTreeItemData*
// Author:      Sigal Algranaty
// Date:        31/10/2010
// ---------------------------------------------------------------------------
afApplicationTreeItemData* gdDebugApplicationTreeHandler::getFBODataByFBOAttachment(const afApplicationTreeItemData& displayedItemId)
{
    afApplicationTreeItemData* pRetVal = NULL;

    gdDebugApplicationTreeData* pOrigGDData = qobject_cast<gdDebugApplicationTreeData*>(displayedItemId.extendedItemData());
    GT_IF_WITH_ASSERT(pOrigGDData != NULL)
    {
        // Build an item data with the details of the owner FBO:
        afApplicationTreeItemData fboItemId;
        gdDebugApplicationTreeData* pFBOData = new gdDebugApplicationTreeData;
        fboItemId.setExtendedData(pFBOData);
        pFBOData->_objectOpenGLName = pOrigGDData->_objectOpenGLName;
        pFBOData->_contextId = pOrigGDData->_contextId;
        fboItemId.m_itemType = AF_TREE_ITEM_GL_FBO;

        // Get the owner FBO item data:
        afApplicationTreeItemData* pFBOItemData = getOpenGLObjectDataByName(fboItemId);

        if (pFBOItemData != NULL)
        {
            GT_IF_WITH_ASSERT(pFBOItemData->m_pTreeWidgetItem != NULL)
            {
                for (int i = 0; i < pFBOItemData->m_pTreeWidgetItem->childCount(); i++)
                {
                    // Get the current child:
                    QTreeWidgetItem* pChild = pFBOItemData->m_pTreeWidgetItem->child(i);

                    if (pChild != NULL)
                    {
                        // Get the item data from the tree item id:
                        afApplicationTreeItemData* pItemData = NULL;
                        gdDebugApplicationTreeData* pGDData = NULL;
                        getTreeItemDatas(pChild, pItemData, pGDData);

                        if ((pItemData != NULL) && (pGDData != NULL))
                        {
                            // Is this the item type we are looking for?
                            if (pGDData->_fboAttachmentFBOName == pOrigGDData->_fboAttachmentFBOName)
                            {
                                // Item was found, select it:
                                pRetVal = pItemData;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getPBufferStaticBufferDataByIdAndType
// Description: Return the static buffer item data for a pbuffer with the requested buffer type
// Arguments:   int pbufferId
//              apDisplayBuffer bufferType
// Return Val:  gdDebugApplicationTreeData*
// Author:      Sigal Algranaty
// Date:        31/10/2010
// ---------------------------------------------------------------------------
afApplicationTreeItemData* gdDebugApplicationTreeHandler::getPBufferStaticBufferDataByIdAndType(const apContextID& contextID, int pbufferId, apDisplayBuffer bufferType)
{
    afApplicationTreeItemData* pRetVal = NULL;

    // Get the cl objects tree root id:
    afApplicationTreeItemData* pPBufferItemData = getPBufferDataById(contextID, pbufferId);

    if (pPBufferItemData != NULL)
    {
        GT_IF_WITH_ASSERT(pPBufferItemData->m_pTreeWidgetItem != NULL)
        {
            int count = pPBufferItemData->m_pTreeWidgetItem->childCount();

            for (int i = 0 ; i < count; i++)
            {
                // Get the next child:
                QTreeWidgetItem* pChild = pPBufferItemData->m_pTreeWidgetItem->child(i);

                if (pChild != NULL)
                {
                    afApplicationTreeItemData* pTreeItemData = NULL;
                    gdDebugApplicationTreeData* pGDData = NULL;
                    getTreeItemDatas(pPBufferItemData->m_pTreeWidgetItem, pTreeItemData, pGDData);

                    if ((pTreeItemData != NULL) && (pGDData != NULL))
                    {
                        // Is this the item type we are looking for?
                        if (pGDData->_bufferType == bufferType)
                        {
                            // Item was found, select it:
                            pRetVal = pTreeItemData;
                            break;
                        }
                    }
                }
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getOpenCLKernelData
// Description: Get an OpenCL kernel object data
// Arguments:   int contextId
//              int programIndex
//              int kernelIndex
// Return Val:  afApplicationTreeItemData*
// Author:      Sigal Algranaty
// Date:        13/1/2011
// ---------------------------------------------------------------------------
afApplicationTreeItemData* gdDebugApplicationTreeHandler::getOpenCLKernelData(const apContextID& contextID, int programIndex, int kernelName)
{
    afApplicationTreeItemData* pRetVal = NULL;

    // Get the cl objects tree root id:
    afApplicationTreeItemData* pProgramItemData = getOpenCLObjectDataByIndex(contextID, AF_TREE_ITEM_CL_PROGRAM, programIndex);

    if (pProgramItemData != NULL)
    {
        GT_IF_WITH_ASSERT(pProgramItemData->m_pTreeWidgetItem != NULL)
        {
            int count = pProgramItemData->m_pTreeWidgetItem->childCount();

            for (int i = 0 ; i < count; i++)
            {
                // Get the next child:
                QTreeWidgetItem* pChild = pProgramItemData->m_pTreeWidgetItem->child(i);

                if (pChild != NULL)
                {
                    gdDebugApplicationTreeData* pGDData = NULL;
                    afApplicationTreeItemData* pItemData = NULL;
                    getTreeItemDatas(pChild, pItemData, pGDData);

                    if ((pItemData != NULL) && (pGDData != NULL))
                    {
                        // Is this the item type we are looking for?
                        // Is this the item we are looking for?
                        if (pGDData->_objectOpenCLName == kernelName)
                        {
                            // Item was found, select it:
                            pRetVal = pItemData;
                            break;
                        }
                    }
                }
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getOpenCLKernelData
// Description: Get an OpenCL kernel item data
// Arguments:   const apContextID& contextID - the kernel context id
//              int programIndex - the kernel's program id
//              oaCLKernelHandle kernelHandle - the kernel handle
// Return Val:  afApplicationTreeItemData*
// Author:      Sigal Algranaty
// Date:        30/6/2011
// ---------------------------------------------------------------------------
afApplicationTreeItemData* gdDebugApplicationTreeHandler::getOpenCLKernelData(const apContextID& contextID, int programIndex, oaCLKernelHandle kernelHandle)
{
    afApplicationTreeItemData* pRetVal = NULL;

    // Get the cl objects tree root id:
    afApplicationTreeItemData* pProgramItemData = getOpenCLObjectDataByIndex(contextID, AF_TREE_ITEM_CL_PROGRAM, programIndex);

    if (pProgramItemData != NULL)
    {
        GT_IF_WITH_ASSERT(pProgramItemData->m_pTreeWidgetItem != NULL)
        {
            int count = pProgramItemData->m_pTreeWidgetItem->childCount();

            for (int i = 0 ; i < count; i++)
            {
                // Get the next child:
                QTreeWidgetItem* pChild = pProgramItemData->m_pTreeWidgetItem->child(i);

                if (pChild != NULL)
                {
                    afApplicationTreeItemData* pTreeItemData = NULL;
                    gdDebugApplicationTreeData* pGDData = NULL;
                    getTreeItemDatas(pChild, pTreeItemData, pGDData);

                    if ((pTreeItemData != NULL) && (pGDData != NULL))
                    {
                        // Is this the item type we are looking for?
                        if (pGDData->_clKernelHandle == kernelHandle)
                        {
                            // Item was found, select it:
                            pRetVal = pTreeItemData;
                            break;
                        }
                    }
                }
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getPBufferDataById
// Description: Return the static buffer item data for a pbuffer with the requested buffer type
// Arguments:   int pbufferId - the pbuffer id
// Return Val:  afApplicationTreeItemData*
// Author:      Sigal Algranaty
// Date:        31/10/2010
// ---------------------------------------------------------------------------
afApplicationTreeItemData* gdDebugApplicationTreeHandler::getPBufferDataById(const apContextID& contextID, int pbufferID)
{
    (void)(contextID);  // unused
    afApplicationTreeItemData* pRetVal = NULL;

    // Get the cl objects tree root id:
    afApplicationTreeItemData* pPBuffersItemData = getItemDataByType(contextID, AF_TREE_ITEM_GL_PBUFFER_NODE);

    if (pPBuffersItemData != NULL)
    {
        int count = pPBuffersItemData->m_pTreeWidgetItem->childCount();

        for (int i = 0 ; i < count; i++)
        {
            // Get the next child:
            QTreeWidgetItem* pChild = pPBuffersItemData->m_pTreeWidgetItem->child(i);

            if (pChild != NULL)
            {
                afApplicationTreeItemData* pTreeItemData = NULL;
                gdDebugApplicationTreeData* pGDData = NULL;
                getTreeItemDatas(pPBuffersItemData->m_pTreeWidgetItem, pTreeItemData, pGDData);

                if ((pTreeItemData != NULL) && (pGDData != NULL))
                {
                    // Is this the item type we are looking for?
                    if (pGDData->_objectOpenGLName == (GLuint)pbufferID)
                    {
                        // Item was found, select it:
                        pRetVal = pTreeItemData;
                        break;
                    }
                }
            }
        }
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::amountOfDisplayedObjectForType
// Description: Return the amount of objects for the requested object type, for
//              the specific context
// Arguments:   const apContextID& contextID
//              afTreeItemType objectType
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        31/10/2010
// ---------------------------------------------------------------------------
int gdDebugApplicationTreeHandler::amountOfDisplayedObjectForType(const apContextID& contextID, afTreeItemType objectType) const
{
    int retVal = 0;

    // Get the parent item type:
    afTreeItemType parentType = afApplicationTreeItemData::itemTypeToParent(objectType);
    GT_IF_WITH_ASSERT(parentType != AF_TREE_ITEM_ITEM_NONE)
    {
        // Get the item data for the parent of the object:
        afApplicationTreeItemData* pParentItemData = getItemDataByType(contextID, parentType);

        if (pParentItemData != NULL)
        {
            retVal = pParentItemData->m_objectCount;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getAmountOfBuffersAttachedToPBuffer
// Description: Return the amount of buffers attached to a requested pbuffer
// Arguments:   int index
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        1/11/2010
// ---------------------------------------------------------------------------
int gdDebugApplicationTreeHandler::getAmountOfBuffersAttachedToPBuffer(int index) const
{
    int retVal = 0;

    // Get the parent item type:
    afApplicationTreeItemData* pPBufferItemData = getItemDataByType(apContextID(AP_NULL_CONTEXT, 0), AF_TREE_ITEM_GL_PBUFFER_NODE, index);

    if (pPBufferItemData != NULL)
    {
        retVal = pPBufferItemData->m_objectCount;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::amountOfDisplayedBuffers
// Description: Return the amount of displayed buffers for the requested context
// Arguments:   const apContextID& contextID

// Return Val:  int
// Author:      Sigal Algranaty
// Date:        31/10/2010
// ---------------------------------------------------------------------------
int gdDebugApplicationTreeHandler::amountOfDisplayedBuffers(const apContextID& contextID) const
{
    int retVal = 0;

    if (contextID._contextType == AP_OPENGL_CONTEXT)
    {
        // Get the amount of FBOs:
        int amountOfFBOs = amountOfDisplayedObjectForType(contextID, AF_TREE_ITEM_GL_FBO);

        // Get the amount of VBOs:
        int amountOfVBOs = amountOfDisplayedObjectForType(contextID, AF_TREE_ITEM_GL_VBO);

        // Get the amount of static buffers:
        int amountOfStaticBuffers = amountOfDisplayedObjectForType(contextID, AF_TREE_ITEM_GL_STATIC_BUFFER);

        // Get the amount of render buffers:
        int amountOfRenderBuffers = amountOfDisplayedObjectForType(contextID, AF_TREE_ITEM_GL_RENDER_BUFFER);

        retVal = (amountOfVBOs + amountOfFBOs + amountOfRenderBuffers + amountOfStaticBuffers);
    }
    else if (contextID._contextType == AP_OPENCL_CONTEXT)
    {
        // Get the amounf of OpenCL buffers:
        retVal = amountOfDisplayedObjectForType(contextID, AF_TREE_ITEM_CL_BUFFER);
    }
    else
    {
        // Null context:
        retVal = 0;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::doesItemExist
// Description: Given an object internal file path, the function returns its
//              matching item data
// Arguments:   const osFilePath& objectFilePath
//              gdDebugApplicationTreeData*& pOriginalItemData
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/8/2011
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::doesItemExist(const osFilePath& objectFilePath, afApplicationTreeItemData*& pOriginalItemData) const
{
    bool retVal = false;

    // Get the file name:
    gtString fileName;
    objectFilePath.getFileName(fileName);

    // Get the item type from the item name:
    afApplicationTreeItemData objectID;
    objectID.setExtendedData(new gdDebugApplicationTreeData);
    int additionalParameter = -1;
    bool rcGetObjectDetails = gdHTMLProperties::htmlLinkToObjectDetails(fileName, objectID, additionalParameter);
    GT_IF_WITH_ASSERT(rcGetObjectDetails)
    {
        GT_IF_WITH_ASSERT(m_pApplicationTree != NULL)
        {
            // Check if an item exist in the tree:
            retVal = m_pApplicationTree->doesItemExist(&objectID, pOriginalItemData);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getPBufferId
// Description: Return the PBuffer id of the index'th PBuffer
// Arguments:   int index
//              int& PBufferId
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/5/2009
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::getPBufferId(int index, int& pbufferId) const
{
    bool retVal = false;

    // Get the pbuffer item data:
    afApplicationTreeItemData* pPBufferItemData = getItemDataByType(apContextID(AP_NULL_CONTEXT, 0), AF_TREE_ITEM_GL_PBUFFER_NODE, index);

    if (pPBufferItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pPBufferItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            pbufferId = pGDData->_objectOpenGLName;
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdBuffersTreeViewer::getPBufferAttachmentItemData
// Description: Return the item data related to pbuffer attahment
// Arguments: int pbufferIndex
//            int attahmentIndex
// Return Val: gdTexturesAndBuffersItemData*
// Author:      Sigal Algranaty
// Date:        11/5/2009
// ---------------------------------------------------------------------------
afApplicationTreeItemData* gdDebugApplicationTreeHandler::getPBufferAttachmentItemData(int pbufferIndex, int attachmentIndex) const
{
    afApplicationTreeItemData* pRetVal = NULL;

    // Get the pbuffer item data:
    afApplicationTreeItemData* pPBufferItemData = getItemDataByType(apContextID(AP_NULL_CONTEXT, 0), AF_TREE_ITEM_GL_PBUFFER_NODE, pbufferIndex);

    if (pPBufferItemData != NULL)
    {
        GT_IF_WITH_ASSERT(pPBufferItemData->m_pTreeWidgetItem != NULL)
        {
            GT_IF_WITH_ASSERT((attachmentIndex >= 0) && (attachmentIndex < pPBufferItemData->m_pTreeWidgetItem->childCount()))
            {
                QTreeWidgetItem* pChild = pPBufferItemData->m_pTreeWidgetItem->child(attachmentIndex);
                GT_IF_WITH_ASSERT(pChild != NULL)
                {
                    pRetVal = getTreeItemData(pChild);
                }
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::amountOfFBOAttachments
// Description: Return the amount of displayed attachments for the requested FBO
// Arguments:   int fboIndex
//              int attahmentIndex
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        11/5/2009
// ---------------------------------------------------------------------------
int gdDebugApplicationTreeHandler::amountOfFBOAttachments(const apContextID& contextId, int fboIndex) const
{
    int retVal = 0;

    // Get the FBO item data:
    afApplicationTreeItemData* pFBOItemData = getItemDataByType(contextId, AF_TREE_ITEM_GL_FBO, fboIndex);

    if (pFBOItemData != NULL)
    {
        // Get the FBO tree item id:
        QTreeWidgetItem* pFBOTreeItemId = pFBOItemData->m_pTreeWidgetItem;

        // Found the FBO tree item id:
        GT_IF_WITH_ASSERT(pFBOTreeItemId != NULL)
        {
            retVal = pFBOTreeItemId->childCount();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getFBOIndex
// Description: Return the index of an FBO object by its OpenGL name
// Arguments:   const apContextID& contextID
//              GLuint fboName
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        1/11/2010
// ---------------------------------------------------------------------------
int gdDebugApplicationTreeHandler::getFBOIndex(const apContextID& contextID, GLuint fboName) const
{
    int retVal = -1;

    // Search the vector for the item we want:
    int numberOfFBOItems = amountOfDisplayedObjectForType(contextID, AF_TREE_ITEM_GL_FBO);

    for (int i = 0; i < numberOfFBOItems; i++)
    {
        // Get the current FBO item data:
        afApplicationTreeItemData* pViewerItem = getItemDataByType(contextID, AF_TREE_ITEM_GL_FBO, i);
        GT_IF_WITH_ASSERT(pViewerItem != NULL)
        {
            gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pViewerItem->extendedItemData());
            GT_IF_WITH_ASSERT(pGDData != NULL)
            {
                if (pGDData->_objectOpenGLName == fboName)
                {
                    retVal = i;
                    break;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::getFBOAttachmentItemData
// Description: Return the item data for attahmentIndex'th attachment for the FBO located in fboIndex
// Arguments:   int fboIndex
//              int attahmentIndex
// Return Val:  gdTexturesAndBuffersItemData*
// Author:      Sigal Algranaty
// Date:        11/5/2009
// ---------------------------------------------------------------------------
afApplicationTreeItemData* gdDebugApplicationTreeHandler::getFBOAttachmentItemData(const apContextID& contextID, int fboIndex, int attachmentIndex) const
{
    afApplicationTreeItemData* pRetVal = NULL;

    // Get the current FBO item data:
    afApplicationTreeItemData* pViewerItem = getItemDataByType(contextID, AF_TREE_ITEM_GL_FBO, fboIndex);
    GT_IF_WITH_ASSERT(pViewerItem != NULL)
    {
        // Get the FBO tree item id:
        QTreeWidgetItem* pFBOTreeItemId = pViewerItem->m_pTreeWidgetItem;
        GT_IF_WITH_ASSERT(pFBOTreeItemId != NULL)
        {
            GT_IF_WITH_ASSERT((attachmentIndex >= 0) && (attachmentIndex < pFBOTreeItemId->childCount()))
            {
                QTreeWidgetItem* pItem = pFBOTreeItemId->child(attachmentIndex);
                pRetVal = getTreeItemData(pItem);
            }
        }
    }
    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::findMatchingTreeItem
// Description: Look for an object in the objects tree by its id
// Arguments:   const gdDebugApplicationTreeData& displayedItemId -
//              an item data with only the identification information for the object
// Return Val:  gdDebugApplicationTreeData*
// Author:      Sigal Algranaty
// Date:        29/12/2010
// ---------------------------------------------------------------------------
afApplicationTreeItemData* gdDebugApplicationTreeHandler::FindMatchingTreeItem(const afApplicationTreeItemData& displayedItemId)
{
    afApplicationTreeItemData* pRetVal = NULL;

    // Do not assert for this cast. Items that are not debug items can also be tested with this function:
    gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(displayedItemId.extendedItemData());

    if (pGDData != NULL)
    {
        if (afApplicationTreeItemData::isItemTypeRoot(displayedItemId.m_itemType))
        {
            // Get the data item by its type:
            pRetVal = getItemDataByType(pGDData->_contextId, displayedItemId.m_itemType);
        }
        else
        {
            // Select the item by its type:
            switch (displayedItemId.m_itemType)
            {
                // If the item is a header, select the header from the objects explorer:
                case AF_TREE_ITEM_GL_PBUFFERS_NODE:
                case AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE:
                case AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE:
                case AF_TREE_ITEM_GL_VBO_NODE:
                case AF_TREE_ITEM_GL_TEXTURES_NODE:
                case AF_TREE_ITEM_GL_DISPLAY_LISTS_NODE:
                case AF_TREE_ITEM_GL_FBO_NODE:
                case AF_TREE_ITEM_APP_ROOT:
                {
                    // Select the all textures header
                    pRetVal = getItemDataByType(pGDData->_contextId, displayedItemId.m_itemType);
                }
                break;

                case AF_TREE_ITEM_GL_TEXTURE:
                case AF_TREE_ITEM_GL_FBO:
                case AF_TREE_ITEM_GL_RENDER_BUFFER:
                case AF_TREE_ITEM_GL_VBO:
                case AF_TREE_ITEM_GL_VERTEX_SHADER:
                case AF_TREE_ITEM_GL_TESSELLATION_CONTROL_SHADER:
                case AF_TREE_ITEM_GL_TESSELLATION_EVALUATION_SHADER:
                case AF_TREE_ITEM_GL_FRAGMENT_SHADER:
                case AF_TREE_ITEM_GL_GEOMETRY_SHADER:
                case AF_TREE_ITEM_GL_COMPUTE_SHADER:
                case AF_TREE_ITEM_GL_UNSUPPORTED_SHADER:
                case AF_TREE_ITEM_GL_DISPLAY_LIST:
                case AF_TREE_ITEM_GL_PROGRAM:
                case AF_TREE_ITEM_GL_PROGRAM_PIPELINE:
                case AF_TREE_ITEM_GL_SAMPLER:
                {
                    // Get the render buffer data by its GL name:
                    pRetVal = getOpenGLObjectDataByName(displayedItemId);
                }
                break;

                case AF_TREE_ITEM_GL_FBO_ATTACHMENT:
                {
                    // Get the FBO attachment data:
                    pRetVal = getFBODataByFBOAttachment(displayedItemId);
                }
                break;

                case AF_TREE_ITEM_GL_STATIC_BUFFER:
                {
                    // Get static buffer item data:
                    pRetVal = getStaticBufferDataByType(pGDData->_contextId, pGDData->_bufferType);
                }
                break;

                case AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER:
                {
                    // Get PBuffer details:
                    int pbufferId = pGDData->_objectOpenGLName;
                    apDisplayBuffer bufferType = pGDData->_bufferType;
                    pRetVal = getPBufferStaticBufferDataByIdAndType(pGDData->_contextId, pbufferId, bufferType);
                }
                break;

                case AF_TREE_ITEM_GL_PBUFFER_NODE:
                {
                    // Get pbuffer item data:
                    pRetVal = getPBufferDataById(pGDData->_contextId, pGDData->_objectOpenGLName);
                }
                break;

                case AF_TREE_ITEM_CL_BUFFER:
                case AF_TREE_ITEM_CL_IMAGE:
                case AF_TREE_ITEM_CL_PIPE:
                case AF_TREE_ITEM_CL_PROGRAM:
                case AF_TREE_ITEM_CL_COMMAND_QUEUE:
                {
                    // Get the CL buffer item data:
                    pRetVal = getOpenCLObjectDataByName(pGDData->_contextId, displayedItemId.m_itemType, pGDData->_objectOpenCLName, pGDData->_objectOwnerName);
                }
                break;

                case AF_TREE_ITEM_CL_KERNEL:
                {
                    // Get the CL kernel item data:
                    pRetVal = getOpenCLKernelData(pGDData->_contextId, pGDData->_objectOwnerName, pGDData->_objectOpenCLName);
                }
                break;

                case AF_TREE_ITEM_GL_RENDER_CONTEXT:
                {
                    pRetVal = getOpenGLContextData(pGDData->_contextId);
                }
                break;

                case AF_TREE_ITEM_CL_CONTEXT:
                {
                    pRetVal = getOpenCLContextData(pGDData->_contextId);
                }
                break;

                default:
                {
                    pRetVal = NULL;
                    GT_ASSERT_EX(false, L"Unknown object type.");
                }
                break;
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::fillTextureMemoryData
// Description: fills a texture memory data on the texture item data
// Arguments:   const apGLTextureMemoryData& textureMemoryData
//              afApplicationTreeItemData*& pTextureItemData
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/9/2010
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::fillTextureMemoryData(const apGLTextureMemoryData& textureMemoryData, afApplicationTreeItemData*& pTextureItemData)
{
    if (m_isTextureMemoryDataUpdateRequired)
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(pTextureItemData != NULL)
        {
            // Get the calculated texture object size (all miplevels sizes):
            bool isEstimate = false;
            bool rc = textureMemoryData.getMemorySize(pTextureItemData->m_objectMemorySize, isEstimate);

            if (!rc)
            {
                OS_OUTPUT_DEBUG_LOG(GD_STR_LogMsg_couldNotDetermineTextureMemorySize, OS_DEBUG_LOG_DEBUG);
            }

            // Set the object size in KB (Round up the number e.g. from 0.1KB to 1KB):
            if (pTextureItemData->m_objectMemorySize > 0)
            {
                pTextureItemData->m_objectMemorySize = GD_MEM_SIZE_BITS_TO_KB(pTextureItemData->m_objectMemorySize);

                if (pTextureItemData->m_objectMemorySize == 0)
                {
                    pTextureItemData->m_objectMemorySize = 1;
                }
            }

            gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pTextureItemData->extendedItemData());
            GT_IF_WITH_ASSERT(pGDData != NULL)
            {
                // Get the texture dimensions:
                GLsizei width = 0;
                GLsizei height = 0;
                GLsizei depth = 0;
                textureMemoryData.getDimensions(width, height, depth);
                pGDData->_objectWidth = width;
                pGDData->_objectHeight = height;
                pGDData->_objectDepth = depth;
                pGDData->_isMemorySizeEstimated = isEstimate;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::fillStaticBufferMemoryData
// Description: Fills a static buffer memory size details
// Arguments:   const apStaticBuffer& staticBufferDetails
//              gdDebugApplicationTreeData*& pStaticBufferItemData
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/9/2010
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::fillStaticBufferMemoryData(apStaticBuffer& staticBufferDetails, afApplicationTreeItemData*& pStaticBufferItemData)
{
    //if (m_shouldUpdateTextureMemoryData)
    //{
    // Sanity check:
    GT_IF_WITH_ASSERT(pStaticBufferItemData != NULL)
    {
        // Calculate the texture object size:
        GLsizei width = 0;
        GLsizei height = 0;
        staticBufferDetails.getBufferDimensions(width, height);

        // Get the calculated render buffer size:
        bool rc = staticBufferDetails.calculateMemorySize(pStaticBufferItemData->m_objectMemorySize);
        GT_ASSERT(rc);

        // Set the object size in KB (Round up the number e.g. from 0.1KB to 1KB):
        pStaticBufferItemData->m_objectMemorySize = GD_MEM_SIZE_BITS_TO_KB(pStaticBufferItemData->m_objectMemorySize);

        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pStaticBufferItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Set the buffer's dimensions:
            pGDData->_objectWidth = width;
            pGDData->_objectHeight = height;
        }
    }
    //}
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::fillVBOMemoryData
// Description: Fills the VBO memory data
// Arguments:   afApplicationTreeItemData*& pVBOItemData
//              const apGLVBO& vboDetails
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/9/2010
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::fillVBOMemoryData(const apGLVBO& vboDetails, afApplicationTreeItemData*& pVBOItemData)
{
    //if (m_shouldUpdateTextureMemoryData)
    //{
    // Sanity check:
    GT_IF_WITH_ASSERT(pVBOItemData != NULL)
    {
        // Set the object size in KB (Round up the number e.g. from 0.1KB to 1KB):
        pVBOItemData->m_objectMemorySize = vboDetails.size();

        if (pVBOItemData->m_objectMemorySize > 0)
        {
            pVBOItemData->m_objectMemorySize = GD_MEM_SIZE_BYTES_TO_KB(pVBOItemData->m_objectMemorySize);

            if (pVBOItemData->m_objectMemorySize == 0)
            {
                pVBOItemData->m_objectMemorySize = 1;
            }
        }
    }
    //}
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::fillShaderMemoryData
// Description: Calculates the shader memory size
// Arguments:   afApplicationTreeItemData*& pShaderItemData
//              gtAutoPtr<apGLShaderObject>& aptrShaderDetails
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/9/2010
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::fillShaderMemoryData(afApplicationTreeItemData*& pShaderItemData, gtAutoPtr<apGLShaderObject>& aptrShaderDetails)
{
    //if (m_shouldUpdateTextureMemoryData)
    //{
    // Sanity check:
    GT_IF_WITH_ASSERT(pShaderItemData != NULL)
    {
        // Set the shader source code length:
        unsigned long sourceCodeLength = aptrShaderDetails->sourceCodeLength();

        // Set the object size in KB (The size is given in chars = bytes):
        pShaderItemData->m_objectMemorySize = GD_MEM_SIZE_BYTES_TO_KB(sourceCodeLength);
    }
    //}
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::fillDisplayListMemoryData
// Description: Fills the display list memory size
// Arguments:   afApplicationTreeItemData*& pListItemData
//              apGLDisplayList& displayListDetails
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/9/2010
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::fillDisplayListMemoryData(const apGLDisplayList& displayListDetails, afApplicationTreeItemData*& pListItemData)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pListItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pListItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Update the display list geometry size:
            pListItemData->m_objectMemorySize = displayListDetails.geometrySize();

            // Update the display list rendered vertices:
            pGDData->_amountOfRenderedVertices = displayListDetails.amountOfRenderedVertices();

            // Set the object size in KB (Round up the number e.g. from 0.1KB to 1KB):
            pListItemData->m_objectMemorySize = GD_MEM_SIZE_BYTES_TO_KB(pListItemData->m_objectMemorySize);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::fillRenderBufferMemoryData
// Description: Fills the render buffer memory size
// Arguments:   afApplicationTreeItemData*& pRenderBufferItemData
//              apGLRenderBuffer& renderBufferDetails
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/9/2010
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::fillRenderBufferMemoryData(apGLRenderBuffer& renderBufferDetails, afApplicationTreeItemData*& pRenderBufferItemData)
{
    //if (m_shouldUpdateTextureMemoryData)
    //{
    // Sanity check:
    GT_IF_WITH_ASSERT(pRenderBufferItemData != NULL)
    {
        // Calculate the texture object size:
        GLsizei width = 0;
        GLsizei height = 0;
        renderBufferDetails.getBufferDimensions(width, height);

        // Get the calculated render buffer size:
        bool rc = renderBufferDetails.calculateMemorySize(pRenderBufferItemData->m_objectMemorySize);
        GT_ASSERT(rc);

        // Set the object size in KB (Round up the number e.g. from 0.1KB to 1KB):
        if (pRenderBufferItemData->m_objectMemorySize > 0)
        {
            pRenderBufferItemData->m_objectMemorySize = GD_MEM_SIZE_BITS_TO_KB(pRenderBufferItemData->m_objectMemorySize);

            if (pRenderBufferItemData->m_objectMemorySize == 0)
            {
                pRenderBufferItemData->m_objectMemorySize = 1;
            }
        }

        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pRenderBufferItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            pGDData->_objectWidth = width;
            pGDData->_objectHeight = height;
        }
    }
    //}
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::fillCLTextureMemoryData
// Description: Fills the OpenCL texture memory data
// Arguments:   const apCLImage& textureDetails
//              afApplicationTreeItemData*& pTextureItemData
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/9/2010
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::fillCLImageMemoryData(const apCLImage& imageDetails, afApplicationTreeItemData*& pImageItemData)
{
    //if (m_shouldUpdateTextureMemoryData)
    //{
    // Sanity check:
    GT_IF_WITH_ASSERT(pImageItemData != NULL)
    {
        // Calculate the image object size:
        gtSize_t width = 0;
        gtSize_t height = 0;
        gtSize_t depth = 0;
        imageDetails.getDimensions(width, height, depth);

        // Get the calculated image object size (all miplevels sizes):
        bool rc = imageDetails.getMemorySize(pImageItemData->m_objectMemorySize);

        if (!rc)
        {
            OS_OUTPUT_DEBUG_LOG(GD_STR_LogMsg_couldNotDetermineTextureMemorySize, OS_DEBUG_LOG_DEBUG);
        }

        // Set the object size in KB (Round up the number e.g. from 0.1KB to 1KB):
        if (pImageItemData->m_objectMemorySize > 0)
        {
            pImageItemData->m_objectMemorySize = GD_MEM_SIZE_BITS_TO_KB(pImageItemData->m_objectMemorySize);

            if (pImageItemData->m_objectMemorySize == 0)
            {
                pImageItemData->m_objectMemorySize = 1;
            }
        }

        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pImageItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Set the image memory flags:
            pGDData->_memoryFlags = imageDetails.memoryFlags();

            pGDData->_objectWidth = width;
            pGDData->_objectHeight = height;
            pGDData->_objectDepth = depth;
        }
    }
    //}
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::fillCLBufferMemoryData
// Description: Fills the OpenCL buffer memory size
// Arguments:   const apCLBuffer& bufferDetails
//              afApplicationTreeItemData*& pBufferItemData
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/9/2010
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::fillCLBufferMemoryData(const apCLBuffer& bufferDetails, afApplicationTreeItemData*& pBufferItemData)
{
    //if (m_shouldUpdateTextureMemoryData)
    //{
    // Sanity check:
    GT_IF_WITH_ASSERT(pBufferItemData != NULL)
    {
        // Set the object size in KB (Round up the number e.g. from 0.1KB to 1KB):
        pBufferItemData->m_objectMemorySize = bufferDetails.bufferSize();

        if (pBufferItemData->m_objectMemorySize > 0)
        {
            pBufferItemData->m_objectMemorySize = GD_MEM_SIZE_BYTES_TO_KB(pBufferItemData->m_objectMemorySize);

            if (pBufferItemData->m_objectMemorySize == 0)
            {
                pBufferItemData->m_objectMemorySize = 1;
            }
        }

        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pBufferItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Set the buffer memory flags:
            pGDData->_memoryFlags = bufferDetails.memoryFlags();
        }
    }
    //}
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::fillCLSubBufferMemoryData
// Description: Fills the OpenCL sub-buffer memory size
// Arguments:   const apCLSubBuffer& subBufferDetails
//              afApplicationTreeItemData*& pBufferItemData
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/9/2010
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::fillCLSubBufferMemoryData(const apCLSubBuffer& subBufferDetails, afApplicationTreeItemData*& pSubBufferItemData)
{
    //if (m_shouldUpdateTextureMemoryData)
    //{
    // Sanity check:
    GT_IF_WITH_ASSERT(pSubBufferItemData != NULL)
    {
        // Set the object size in KB (Round up the number e.g. from 0.1KB to 1KB):
        pSubBufferItemData->m_objectMemorySize = subBufferDetails.bufferRegion().size;

        if (pSubBufferItemData->m_objectMemorySize > 0)
        {
            pSubBufferItemData->m_objectMemorySize = GD_MEM_SIZE_BYTES_TO_KB(pSubBufferItemData->m_objectMemorySize);

            if (pSubBufferItemData->m_objectMemorySize == 0)
            {
                pSubBufferItemData->m_objectMemorySize = 1;
            }
        }

        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pSubBufferItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Set the sub-buffer memory flags:
            pGDData->_memoryFlags = subBufferDetails.memoryFlags();
        }
    }
    //}
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::fillCLPipeMemoryData
// Description: Fills the OpenCL pipe memory size
// Author:      Uri Shomroni
// Date:        1/10/2014
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::fillCLPipeMemoryData(const apCLPipe& pipeDetails, afApplicationTreeItemData*& pPipeItemData)
{
    //if (m_shouldUpdateTextureMemoryData)
    //{
    // Sanity check:
    GT_IF_WITH_ASSERT(pPipeItemData != NULL)
    {
        // Set the object size in KB (Round up the number e.g. from 0.1KB to 1KB):
        pPipeItemData->m_objectMemorySize = pipeDetails.pipeMaxPackets() * pipeDetails.pipePacketSize();

        if (pPipeItemData->m_objectMemorySize > 0)
        {
            pPipeItemData->m_objectMemorySize = GD_MEM_SIZE_BYTES_TO_KB(pPipeItemData->m_objectMemorySize);

            if (pPipeItemData->m_objectMemorySize == 0)
            {
                pPipeItemData->m_objectMemorySize = 1;
            }
        }

        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pPipeItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Set the pipe memory flags:
            pGDData->_memoryFlags = pipeDetails.memoryFlags();
        }
    }
    //}
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::fillCLSamplerMemoryData
// Description:
// Arguments:   const apCLSampler& samplerDetails
//              afApplicationTreeItemData*& pSamplerItemData
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/9/2010
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::fillCLSamplerMemoryData(const apCLSampler& samplerDetails, afApplicationTreeItemData*& pSamplerItemData)
{
    (void)(samplerDetails);  // unused
    (void)(pSamplerItemData);  // unused
    // Do nothing:
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::fillCLEventMemoryData
// Description:
// Arguments:   const apCLEvent& eventDetails
//              afApplicationTreeItemData*& pEventItemData
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::fillCLEventMemoryData(const apCLEvent& eventDetails, afApplicationTreeItemData*& pEventItemData)
{
    (void)(eventDetails);  // unused
    (void)(pEventItemData);  // unused
    // Do nothing:
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::fillCLCommandQueueMemoryData
// Description: Fills the command queue memory size (0 currently)
// Arguments:   const apCLCommandQueue& commandQueueDetails
//              afApplicationTreeItemData*& pCommandQueueItemData
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/9/2010
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::fillCLCommandQueueMemoryData(const apCLCommandQueue& commandQueueDetails, afApplicationTreeItemData*& pCommandQueueItemData)
{
    // Set the size:
    gtUInt32 queueSz = commandQueueDetails.queueSize();
    pCommandQueueItemData->m_objectMemorySize = commandQueueDetails.queueOnDevice() ? GD_MEM_SIZE_BYTES_TO_KB(queueSz) : 0;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::fillCLProgramMemoryData
// Description:
// Arguments:   const apCLProgram& programDetails
//              afApplicationTreeItemData*& pProgramItemData
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/9/2010
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::fillCLProgramMemoryData(const apCLProgram& programDetails, afApplicationTreeItemData*& pProgramItemData)
{
    (void)(programDetails);  // unused
    (void)(pProgramItemData);  // unused
    // Do nothing:
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateTexturesMemoryParameters
// Description: Update the textures parameters. Do this only for memory data extraction
// Arguments:   contextId
//            textureNamesForUpdate
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        30/5/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateTexturesMemoryParameters(int contextId, const gtVector<apGLTextureMipLevelID>& textureNamesForUpdate)
{
    bool retVal = true;

    if (m_isTextureMemoryDataUpdateRequired)
    {
        if (!gaIsDuringDebuggedProcessTermination())
        {
            // Get the current progress range:
            // This is needed since we want to restore the range after we finish
            // using the progress bar dialog for the textures update:
            int currentProgressRange = afProgressBarWrapper::instance().progressRange();

            // Update the progress with the amount of textures for update:
            afProgressBarWrapper::instance().setProgressDetails(GD_STR_MemoryAnalysisViewerUpdatingTexturesParameters, textureNamesForUpdate.size());

            if (textureNamesForUpdate.size() > 0)
            {
                int currentTextureIndex = 0;

                while (currentTextureIndex < (int)textureNamesForUpdate.size())
                {
                    int amountOfAddedTextures = 0;
                    gtVector<apGLTextureMipLevelID> currentTexturesChunk;

                    for (int i = 0; i < 100; i++)
                    {
                        // Add the current texture id:
                        if (currentTextureIndex < (int)textureNamesForUpdate.size())
                        {
                            currentTexturesChunk.push_back(textureNamesForUpdate[currentTextureIndex]);
                            currentTextureIndex++;
                            amountOfAddedTextures++;
                        }
                    }

                    // Update the current textures chunk:
                    bool rc = gaUpdateTextureParameters(contextId, currentTexturesChunk, true);
                    retVal = retVal && rc;

                    // Increment the progress bar:
                    afProgressBarWrapper::instance().incrementProgressBar(amountOfAddedTextures);
                }
            }

            // Restore the previous progress bar range:
            afProgressBarWrapper::instance().setProgressRange(currentProgressRange);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateContextMemoryData
// Description: Updates a specific data for memory
// Arguments:   bool isDeleted
//              gdDebugApplicationTreeData* pRenderContextItemData
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/9/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateContextMemoryData(bool isDeleted, afApplicationTreeItemData* pRenderContextItemData)
{
    bool retVal = true;

    //if (m_shouldUpdateTextureMemoryData)
    //{
    retVal = false;

    GT_IF_WITH_ASSERT((pRenderContextItemData != NULL) && (m_pHeaderItem != NULL))
    {
        // Go through all the context children, and summarize their memory size:
        int renderContextMemorySize = 0;

        for (int i = 0; i < m_pHeaderItem->childCount(); i++)
        {
            QTreeWidgetItem* pChild = m_pHeaderItem->child(i);

            // Get the current child item data:
            afApplicationTreeItemData* pChildMemoryData = NULL;
            gdDebugApplicationTreeData* pChildGDMemoryData = NULL;
            getTreeItemDatas(pChild, pChildMemoryData, pChildGDMemoryData);
            GT_IF_WITH_ASSERT(pChildMemoryData != NULL)
            {
                // This item type is present whenever the analyzer plugin is loaded or profile:
                afTreeItemType childItemType = pChildMemoryData->m_itemType;

                if ((AF_TREE_FIRST_DEBUGGER_ITEM_TYPE <= childItemType) && (AF_TREE_LAST_DEBUGGER_ITEM_TYPE >= childItemType))
                {
                    GT_IF_WITH_ASSERT(pChildGDMemoryData != NULL)
                    {
                        // Add the current child memory:
                        renderContextMemorySize += pChildMemoryData->m_objectMemorySize;
                    }
                }
            }
        }

        // Set the object size:
        pRenderContextItemData->m_objectMemorySize = renderContextMemorySize;

        // If the context is deleted, mark is children as memory leaked:
        if (isDeleted)
        {
            // Mark the context and all its children (incl. static buffers) as leaks:
            markItemAndAllDescendantsAsMemoryLeaks(pRenderContextItemData->m_pTreeWidgetItem);
        }

        retVal = true;
    }
    //}

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::markItemAndAllDescendantsAsMemoryLeaks
// Description: Sets the item and all its descendants as memory leaks, except those that have
//              the _isMarkedForDeletion flag on.
// Author:      Uri Shomroni
// Date:        4/11/2008
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::markItemAndAllDescendantsAsMemoryLeaks(QTreeWidgetItem* pTreeItem)
{
    GT_IF_WITH_ASSERT(pTreeItem != NULL)
    {
        int childrenCount = pTreeItem->childCount();

        if (childrenCount > 0)
        {
            for (int i = 0 ; i < childrenCount; i++)
            {
                markItemAndAllDescendantsAsMemoryLeaks(pTreeItem->child(i));
            }
        }

        // Get the item data:
        bool shouldChangeIcon = false;
        afApplicationTreeItemData* pItemData = NULL;
        gdDebugApplicationTreeData* pGDItemData = NULL;
        getTreeItemDatas(pTreeItem, pItemData, pGDItemData);

        if ((pItemData != NULL) && (pGDItemData != NULL))
        {
            // Check if this object's icon cannot be changed:
            shouldChangeIcon = !pGDItemData->_isMarkedForDeletion;
        }

        if (shouldChangeIcon)
        {
            setItemIcon(pTreeItem, m_memoryLeakIconIndex);

            if (childrenCount > 0)
            {
                pTreeItem->setExpanded(true);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::setShouldUpdateMemoryData
// Description: Change the memory update status.
//              If the update status was false, and became true, and the process is
//              suspended, go through each of the objects, and update its memory data
// Arguments:   bool shouldUpdate
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        18/1/2011
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::setShouldUpdateMemoryData(bool shouldUpdate)
{
    if (m_isTextureMemoryDataUpdateRequired != shouldUpdate)
    {
        // Set my update memory status:
        m_isTextureMemoryDataUpdateRequired = shouldUpdate;

        if (m_isTextureMemoryDataUpdateRequired)
        {
            if (gaIsDebuggedProcessSuspended())
            {
                // Call the recursive function handling memory data update for the root:
                bool rcUpdateMemory = updateTreeItemMemoryData(m_pHeaderItem);
                GT_ASSERT(rcUpdateMemory);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateTreeItemMemoryData
// Description: Updates a tree item memory data. The function is recursive, and
//              goes through each of the item children and updates them as well
// Arguments:   QTreeWidgetItem* pTreeItem
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        18/1/2011
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateTreeItemMemoryData(QTreeWidgetItem* pTreeItem)
{
    bool retVal = true;

    if (pTreeItem != NULL)
    {
        // Get the tree item data:
        afApplicationTreeItemData* pItemData = getTreeItemData(pTreeItem);
        GT_IF_WITH_ASSERT(pItemData != NULL)
        {
            // Update the item data according to its type:
            // Starting with true and not asking the root node that does not have item data:
            for (int i = 0 ; i < pTreeItem->childCount(); i++)
            {
                QTreeWidgetItem* pChild = pTreeItem->child(i);

                // Update the child memory data:
                bool rcUpdateChild = updateTreeItemMemoryData(pChild);
                retVal = retVal && rcUpdateChild;

            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateMemoryDataByType
// Description: Updates the requested item data with the memory details for the
//              owner object
// Arguments:   gdDebugApplicationTreeData* pItemData
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        18/1/2011
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateMemoryDataByType(afApplicationTreeItemData* pItemData)
{
    bool retVal = false;
    // Sanity check:
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());

        if (pGDData != NULL)
        {
            switch (pItemData->m_itemType)
            {
                case AF_TREE_ITEM_GL_TEXTURE:
                {
                    // Get the texture object details:
                    apGLTextureMemoryData textureMemoryData;
                    retVal = gaGetTextureMemoryDataObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenGLName, textureMemoryData);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        // Fill the texture memory data:
                        fillTextureMemoryData(textureMemoryData, pItemData);
                    }
                    break;
                }

                case AF_TREE_ITEM_GL_RENDER_BUFFER:
                {
                    // Get the render buffer object details:
                    apGLRenderBuffer renderBufferDetails(0);
                    retVal = gaGetRenderBufferObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenGLName, renderBufferDetails);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        // Fill the render buffer memory data:
                        fillRenderBufferMemoryData(renderBufferDetails, pItemData);
                    }
                    break;
                }

                case AF_TREE_ITEM_GL_STATIC_BUFFER:
                {
                    // Get the static buffer object details:
                    apStaticBuffer staticBufferDetails;
                    retVal = gaGetStaticBufferObjectDetails(pGDData->_contextId._contextId, pGDData->_bufferType, staticBufferDetails);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        // Fill the static buffer memory data:
                        fillStaticBufferMemoryData(staticBufferDetails, pItemData);
                    }
                    break;
                }

                case AF_TREE_ITEM_GL_VBO:
                {
                    // Get the VBO object details:
                    apGLVBO vboDetails;
                    retVal = gaGetVBODetails(pGDData->_contextId._contextId, pGDData->_objectOpenGLName, vboDetails);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        // Fill the VBO memory data:
                        fillVBOMemoryData(vboDetails, pItemData);
                    }
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
                    // Get the shader object details:
                    gtAutoPtr<apGLShaderObject> aptrShaderDetails = NULL;
                    retVal = gaGetShaderObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenGLName, aptrShaderDetails);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        // Fill the shader memory data:
                        fillShaderMemoryData(pItemData, aptrShaderDetails);
                    }
                    break;
                }

                case AF_TREE_ITEM_GL_DISPLAY_LIST:
                {
                    // Get the display list object details:
                    apGLDisplayList displayListDetails;
                    retVal = gaGetDisplayListObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenGLName, displayListDetails);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        // Fill the display list memory data:
                        fillDisplayListMemoryData(displayListDetails, pItemData);
                    }
                    break;
                }

                case AF_TREE_ITEM_CL_IMAGE:
                {
                    // Get the image object details:
                    apCLImage imageDetails;
                    retVal = gaGetOpenCLImageObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, imageDetails);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        // Fill the image memory data:
                        fillCLImageMemoryData(imageDetails, pItemData);
                    }
                    break;
                }

                case AF_TREE_ITEM_CL_BUFFER:
                {
                    // Get the buffer object details:
                    apCLBuffer bufferDetails;
                    retVal = gaGetOpenCLBufferObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, bufferDetails);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        // Fill the buffer memory data:
                        fillCLBufferMemoryData(bufferDetails, pItemData);
                    }
                    break;
                }

                case AF_TREE_ITEM_CL_SUB_BUFFER:
                {
                    // Get the sub-buffer object details:
                    apCLSubBuffer subBufferDetails;
                    retVal = gaGetOpenCLSubBufferObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, subBufferDetails);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        // Fill the sub-buffer memory data:
                        fillCLSubBufferMemoryData(subBufferDetails, pItemData);
                    }
                    break;
                }

                case AF_TREE_ITEM_CL_PIPE:
                {
                    // Get the pipe object details:
                    apCLPipe pipeDetails;
                    retVal = gaGetOpenCLPipeObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, pipeDetails);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        // Fill the pipe memory data:
                        fillCLPipeMemoryData(pipeDetails, pItemData);
                    }
                    break;
                }

                case AF_TREE_ITEM_CL_SAMPLER:
                {
                    // Get the sampler object details:
                    apCLSampler samplerDetails;
                    retVal = gaGetOpenCLSamplerObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, samplerDetails);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        // Fill the sampler memory data:
                        fillCLSamplerMemoryData(samplerDetails, pItemData);
                    }
                    break;
                }

                case AF_TREE_ITEM_CL_EVENT:
                {
                    // Get the event object details:
                    apCLEvent eventDetails(OA_CL_NULL_HANDLE, OA_CL_NULL_HANDLE, false);
                    retVal = gaGetOpenCLEventObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, eventDetails);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        // Fill the event memory data:
                        fillCLEventMemoryData(eventDetails, pItemData);
                    }
                    break;
                }

                case AF_TREE_ITEM_CL_COMMAND_QUEUE:
                {
                    // Get the command queue object details:
                    apCLCommandQueue commandQueueDetails;
                    retVal = gaGetCommandQueueDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, commandQueueDetails);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        // Fill the command queue memory data:
                        fillCLCommandQueueMemoryData(commandQueueDetails, pItemData);
                    }
                    break;
                }

                case AF_TREE_ITEM_CL_PROGRAM:
                {
                    // Get the program object details:
                    apCLProgram programDetails(OA_CL_NULL_HANDLE);
                    retVal = gaGetOpenCLProgramObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, programDetails);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        // Fill the program memory data:
                        fillCLProgramMemoryData(programDetails, pItemData);
                    }
                    break;
                }

                default:
                    // Object memory data should not be updated:
                    retVal = true;
                    break;
            }
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::onMemoryLeakBreakpoint
// Description: Marks memory leaks in the view and outputs the size of the leak
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        4/11/2008
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::onMemoryLeakBreakpoint(const apMemoryLeakEvent& eve)
{
    bool retVal = false;

    // Get the memory leak event type:
    apMemoryLeakEvent::apMemoryLeakType leakType = eve.leakType();

    switch (leakType)
    {
        case apMemoryLeakEvent::AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK:
        {
            // Handle independent allocated object leak:
            retVal = onIndependentGLAllocatedObjectLeak(eve);
        }
        break;

        case apMemoryLeakEvent::AP_GL_CONTEXT_ALLOCATED_OBJECT_LEAK:
        {
            // Handle render context dependent allocated object leak:
            retVal = onRenderContextDependentAllocatedObjectLeak(eve);
        }
        break;

        case apMemoryLeakEvent::AP_CL_CONTEXT_ALLOCATED_OBJECT_LEAK:
        {
            // Handle compute context dependent allocated object leak:
            retVal = onComputeContextDependentAllocatedObjectLeak(eve);
        }
        break;

        case apMemoryLeakEvent::AP_CL_PROGRAM_ALLOCATED_OBJECT_LEAK:
        {
            // Handle compute context dependent allocated object leak:
            retVal = onComputationProgramDependentAllocatedObjectLeak(eve);
        }
        break;

        default:
        {
            // We added a new leak type, but didn't implement it here
            GT_ASSERT(false);
        }
        break;
    }

    return retVal;
}


// Edit menu commands

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::onUpdateEdit_Copy
// Description: enables the copy command in the VS edit menu
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        7/2/2011
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::onUpdateEdit_Copy(bool& isEnabled)
{
    isEnabled = false;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::onUpdateEdit_SelectAll
// Description: enables the select all command in the VS edit menu
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        7/2/2011
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::onUpdateEdit_SelectAll(bool& isEnabled)
{
    isEnabled = false;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::onEdit_Copy
// Description: execute the copy command in the VS edit menu
// Arguments:
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        7/2/2011
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::onEdit_Copy()
{
    // does not support copy
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::onEdit_SelectAll
// Description: execute the select command in the VS edit menu
// Arguments:
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        7/2/2011
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::onEdit_SelectAll()
{
    // does not support select all
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::onIndependentGLAllocatedObjectLeak
// Description: Handling memory leak for independent allocated objects (render context / pbo / sync object)
// Arguments: const apMemoryLeakEvent& eve
//            int& totalLeakKBs
//            bool changeSelection
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/10/2009
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::onIndependentGLAllocatedObjectLeak(const apMemoryLeakEvent& eve)
{
    bool retVal = true;

    // Get the list of leaking render contexts:
    gtVector <unsigned int> leakingRCList;
    bool rcRCs = eve.leakingRenderContexts(leakingRCList);
    GT_IF_WITH_ASSERT(rcRCs)
    {
        // Iterate the leaking contexts:
        int amountOfLeakingContexts = (int)leakingRCList.size();

        for (int i = 0 ; i < amountOfLeakingContexts; i++)
        {
            // Get the current context id:
            apContextID currentContextID(AP_OPENGL_CONTEXT, leakingRCList[i]);
            QTreeWidgetItem* pContextTreeId = getContextTreeItemId(currentContextID);

            // Make sure that the tree node is ok:
            GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
            {
                // Mark the context and all its children (incl. static buffers) as leaks:
                markItemAndAllDescendantsAsMemoryLeaks(pContextTreeId);
            }
        }
    }

    // Get the list of leaking pixel buffers:
    gtVector<unsigned int> leakingPBOList;
    bool rcPBOs = eve.leakingPBuffers(leakingPBOList);
    GT_IF_WITH_ASSERT(rcPBOs)
    {
        // Iterate the leaking pbuffers:
        int amountOfLeakingPBOs = (int)leakingPBOList.size();

        for (int i = 0; i < amountOfLeakingPBOs; i++)
        {
            // Get the current pbuffer id:
            unsigned int currentPBufferID = leakingPBOList[i];
            apContextID contextID;
            afApplicationTreeItemData* pItemData = NULL;
            gdDebugApplicationTreeData* pGDData = NULL;
            getTreeItemDatas(m_pPBuffersTreeId, pItemData, pGDData);
            GT_IF_WITH_ASSERT((pItemData != NULL) && (pGDData != NULL))
            {
                afApplicationTreeItemData* pLeakedBufferTreeIdData = getPBufferDataById(pGDData->_contextId, currentPBufferID);
                GT_IF_WITH_ASSERT(pLeakedBufferTreeIdData != NULL)
                {
                    markItemAndAllDescendantsAsMemoryLeaks(pLeakedBufferTreeIdData->m_pTreeWidgetItem);
                }
            }
        }
    }

    // Get the list of leaking pixel buffers:
    gtVector<apMemoryLeakEvent::apLeakedSyncObjectID> leakingSyncObjects;
    bool rcSyncObjects = eve.leakingSyncObjects(leakingSyncObjects);
    GT_IF_WITH_ASSERT(rcSyncObjects)
    {
        // Iterate the leaking sync objects:
        int amountOfLeakingSyncs = (int)leakingSyncObjects.size();

        for (int i = 0; i < amountOfLeakingSyncs; i++)
        {
            // Get the current sync handle:
            int currentSyncID = leakingSyncObjects[i]._syncID;

            // Check if the sync handle is legal:
            if (currentSyncID > -1)
            {
                QTreeWidgetItem* pSyncItem = NULL;

                for (int j = 0 ; j < m_pSyncObjectsTreeId->childCount(); ++j)
                {
                    QTreeWidgetItem* pItem = m_pSyncObjectsTreeId->child(j);

                    if (pItem != NULL)
                    {
                        const afApplicationTreeItemData* pItemData = getTreeItemData(pItem);
                        GT_IF_WITH_ASSERT(pItemData != NULL)
                        {
                            gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
                            GT_IF_WITH_ASSERT(pGDData != NULL)
                            {
                                // The PBuffers node should only have PBuffer children:
                                GT_IF_WITH_ASSERT(pItemData->m_itemType == AF_TREE_ITEM_GL_SYNC_OBJECT)
                                {
                                    if ((int) pGDData->_objectOpenGLName == currentSyncID)
                                    {
                                        pSyncItem = pItem;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                // Make sure that the tree node is ok:
                GT_IF_WITH_ASSERT(pSyncItem != NULL)
                {
                    // Mark the sync object as leaks:
                    markItemAndAllDescendantsAsMemoryLeaks(pSyncItem);
                }
            }
        }
    }

    // Select and expand items relevant for this leak:
    if (eve.numberOfLeakingRenderContexts() > 0)
    {
        // If we have leaking render contexts, show the app node:
        m_pHeaderItem->setSelected(true);
        m_pHeaderItem->setExpanded(true);
    }
    else if ((eve.numberOfLeakingPBuffers() > 0) && (m_pPBuffersTreeId != NULL))
    {
        // If we have leaking render contexts, show the app node:
        m_pPBuffersTreeId->setSelected(true);
        m_pPBuffersTreeId->setExpanded(true);
    }
    else if ((eve.numberOfLeakingSyncObjects() > 0) && (m_pSyncObjectsTreeId != NULL))
    {
        // If there are no leaking RCs, and no leaking pbuffers, but there are sync objects, show their node:
        m_pSyncObjectsTreeId->setSelected(true);
        m_pSyncObjectsTreeId->setExpanded(true);
    }
    else
    {
        // Otherwise, select the root:
        m_pHeaderItem->setSelected(true);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::onIndependentCLAllocatedObjectLeak
// Description: Handling memory leak for independent allocated objects (compute context)
// Arguments: const apMemoryLeakEvent& eve
//            int& totalLeakKBs
//            bool changeSelection
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        23/07/2015
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::onIndependentCLAllocatedObjectLeak(const apMemoryLeakEvent& eve)
{
    bool retVal = true;

    // Get the list of leaking compute contexts:
    gtVector <unsigned int> leakingCtxList;
    bool rcRCs = eve.leakingComputeContexts(leakingCtxList);
    GT_IF_WITH_ASSERT(rcRCs)
    {
        // Iterate the leaking contexts:
        int amountOfLeakingContexts = (int)leakingCtxList.size();

        for (int i = 0 ; i < amountOfLeakingContexts; i++)
        {
            // Get the current context id:
            apContextID currentContextID(AP_OPENCL_CONTEXT, leakingCtxList[i]);
            QTreeWidgetItem* pContextTreeId = getContextTreeItemId(currentContextID);

            // Make sure that the tree node is ok:
            GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
            {
                // Mark the context and all its children (incl. static buffers) as leaks:
                markItemAndAllDescendantsAsMemoryLeaks(pContextTreeId);
            }
        }
    }

    // Select and expand items relevant for this leak:
    if (eve.numberOfLeakingComputeContexts() > 0)
    {
        // If we have leaking compute contexts, show the app node:
        m_pHeaderItem->setSelected(true);
        m_pHeaderItem->setExpanded(true);
    }

    {
        // Otherwise, select the root:
        m_pHeaderItem->setSelected(true);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::onRenderContextDependentAllocatedObjectLeak
// Description: Handling memory leak for GL context dependent allocated objects (textures/ buffer/ vbos / etc.)
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/10/2009
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::onRenderContextDependentAllocatedObjectLeak(const apMemoryLeakEvent& eve)
{
    bool retVal = false;
    apContextID leakingContextID(AP_OPENGL_CONTEXT, eve.leakingObjectsRenderContextID());

    // Get the context tree item id:
    QTreeWidgetItem* pContextTreeId = getContextTreeItemId(leakingContextID);

    QTreeWidgetItem* pSharedContextItemID = NULL;
    QTreeWidgetItem* pSharedContextChild = NULL;

    // Make sure that the tree node is ok:
    GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
    {
        // Get the tree ID:
        QTreeWidgetItem* pChild = NULL;

        if (pContextTreeId->childCount() > 0)
        {
            pChild = pContextTreeId->child(0);
        }

        bool needToMarkSharedContext = false;
        apGLRenderContextInfo contextInfo;
        bool rcRC = gaGetRenderContextDetails(leakingContextID._contextId, contextInfo);
        GT_IF_WITH_ASSERT(rcRC)
        {
            int sharedContextID = contextInfo.sharingContextID();

            if (sharedContextID != -1)
            {
                // If we got the root (i.e the id is invalid), don't copy it:
                apContextID sharedContextIdentifier(AP_OPENGL_CONTEXT, sharedContextID);
                QTreeWidgetItem* pSharedContextTreeId = getContextTreeItemId(sharedContextIdentifier);
                GT_IF_WITH_ASSERT((pSharedContextTreeId != m_pHeaderItem) && (pSharedContextTreeId != NULL))
                {
                    pSharedContextItemID = pSharedContextTreeId;
                    needToMarkSharedContext = true;
                }
            }
        }

        // If we the original item has no children, check to see if we need to go over the shared context as well.
        if (pChild == NULL)
        {
            if (needToMarkSharedContext)
            {
                GT_IF_WITH_ASSERT(pSharedContextItemID != NULL)
                {
                    pSharedContextChild = pSharedContextItemID->child(0);
                    needToMarkSharedContext = false;
                }
            }
        }

        int i = 0;

        while ((pChild != NULL) && (i < pContextTreeId->childCount()))
        {
            // See if this is one of the types we mark as leaked:
            const afApplicationTreeItemData* pItemData = getTreeItemData(pSharedContextChild);
            GT_IF_WITH_ASSERT(pItemData != NULL)
            {
                afTreeItemType currentItemType = pItemData->m_itemType;

                switch (currentItemType)
                {
                    case AF_TREE_ITEM_GL_TEXTURES_NODE:
                    case AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE:
                    case AF_TREE_ITEM_GL_VBO_NODE:
                    case AF_TREE_ITEM_GL_PROGRAMS_NODE:
                    case AF_TREE_ITEM_GL_PROGRAM_PIPELINES_NODE:
                    case AF_TREE_ITEM_GL_SAMPLERS_NODE:
                    case AF_TREE_ITEM_GL_SHADERS_NODE:
                    case AF_TREE_ITEM_GL_DISPLAY_LISTS_NODE:
                    case AF_TREE_ITEM_GL_FBO_NODE:
                    {
                        // Mark the leak:
                        markItemAndAllDescendantsAsMemoryLeaks(pSharedContextChild);
                    }
                    break;

                    case AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE:
                    {
                        // Do nothing, these items are deleted with the render context and so do not count as leaks
                    }
                    break;

                    case AF_TREE_ITEM_APP_ROOT:
                    case AF_TREE_ITEM_MESSAGE:
                    case AF_TREE_ITEM_GL_RENDER_CONTEXT:
                    case AF_TREE_ITEM_GL_TEXTURE:
                    case AF_TREE_ITEM_GL_RENDER_BUFFER:
                    case AF_TREE_ITEM_GL_STATIC_BUFFER:
                    case AF_TREE_ITEM_GL_PBUFFERS_NODE:
                    case AF_TREE_ITEM_GL_PBUFFER_NODE:
                    case AF_TREE_ITEM_GL_SYNC_OBJECTS_NODE:
                    case AF_TREE_ITEM_GL_SYNC_OBJECT:
                    case AF_TREE_ITEM_GL_VBO:
                    case AF_TREE_ITEM_GL_PROGRAM:
                    case AF_TREE_ITEM_GL_PROGRAM_PIPELINE:
                    case AF_TREE_ITEM_GL_SAMPLER:
                    case AF_TREE_ITEM_GL_VERTEX_SHADER:
                    case AF_TREE_ITEM_GL_TESSELLATION_CONTROL_SHADER:
                    case AF_TREE_ITEM_GL_TESSELLATION_EVALUATION_SHADER:
                    case AF_TREE_ITEM_GL_GEOMETRY_SHADER:
                    case AF_TREE_ITEM_GL_FRAGMENT_SHADER:
                    case AF_TREE_ITEM_GL_COMPUTE_SHADER:
                    case AF_TREE_ITEM_GL_UNSUPPORTED_SHADER:
                    case AF_TREE_ITEM_GL_DISPLAY_LIST:
                    case AF_TREE_ITEM_GL_FBO:
                    case AF_TREE_ITEM_CL_CONTEXT:
                    case AF_TREE_ITEM_CL_IMAGES_NODE:
                    case AF_TREE_ITEM_CL_IMAGE:
                    case AF_TREE_ITEM_CL_BUFFERS_NODE:
                    case AF_TREE_ITEM_CL_BUFFER:
                    case AF_TREE_ITEM_CL_PIPES_NODE:
                    case AF_TREE_ITEM_CL_PIPE:
                    case AF_TREE_ITEM_CL_SAMPLERS_NODE:
                    case AF_TREE_ITEM_CL_SAMPLER:
                    case AF_TREE_ITEM_CL_EVENTS_NODE:
                    case AF_TREE_ITEM_CL_EVENT:
                    case AF_TREE_ITEM_CL_COMMAND_QUEUES_NODE:
                    case AF_TREE_ITEM_CL_COMMAND_QUEUE:
                    case AF_TREE_ITEM_CL_PROGRAMS_NODE:
                    case AF_TREE_ITEM_CL_PROGRAM:
                    {
                        // These should never be the [direct] child of a render context!
                        GT_ASSERT(false);
                    }
                    break;

                    default:
                    {
                        // We added a new item type, but didn't implement it here
                        GT_ASSERT(false);
                    }
                    break;
                }
            }

            // Get the next item:
            pChild = pContextTreeId->child(++i);

            // If we finished the original item's children, check to see if we need to go over the shared context as well.
            if (pSharedContextChild != NULL)
            {
                if (needToMarkSharedContext)
                {
                    GT_IF_WITH_ASSERT(pSharedContextItemID != NULL)
                    {
                        pSharedContextChild = pSharedContextItemID->child(0);
                        needToMarkSharedContext = false;
                    }
                }
            }
        }
    }

    // Select the render context node and expand it:
    if (pContextTreeId->childCount() > 0)
    {
        pContextTreeId->setExpanded(true);
    }

    // Also expand the shared context if there is one:
    if (pSharedContextItemID != NULL)
    {
        if (pSharedContextItemID->childCount() > 0)
        {
            pSharedContextItemID->setExpanded(true);
        }
    }

    pContextTreeId->setSelected(true);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::onComputeContextDependentAllocatedObjectLeak
// Description: Handling memory leak for CL context dependent allocated objects (queues / buffers/ images / etc)
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        23/6/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::onComputeContextDependentAllocatedObjectLeak(const apMemoryLeakEvent& eve)
{
    bool retVal = false;
    apContextID leakingContextID(AP_OPENCL_CONTEXT, eve.leakingObjectsComputeContextID());

    // Get the context tree item id:
    QTreeWidgetItem* pContextTreeId = getContextTreeItemId(leakingContextID);

    // Make sure that the tree node is ok:
    GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
    {
        for (int i = 0 ; i < pContextTreeId->childCount(); i++)
        {
            QTreeWidgetItem* pChildItemId = pContextTreeId->child(i);

            if (pChildItemId != NULL)
            {
                // See if this is one of the types we mark as leaked:
                const afApplicationTreeItemData* pItemData = getTreeItemData(pChildItemId);
                GT_IF_WITH_ASSERT(pItemData != NULL)
                {
                    afTreeItemType currentItemType = pItemData->m_itemType;

                    switch (currentItemType)
                    {
                        case AF_TREE_ITEM_CL_IMAGES_NODE:
                        case AF_TREE_ITEM_CL_BUFFERS_NODE:
                        case AF_TREE_ITEM_CL_SAMPLERS_NODE:
                        case AF_TREE_ITEM_CL_EVENTS_NODE:
                        case AF_TREE_ITEM_CL_COMMAND_QUEUES_NODE:
                        case AF_TREE_ITEM_CL_PROGRAMS_NODE:
                        {
                            // Mark the leak:
                            markItemAndAllDescendantsAsMemoryLeaks(pChildItemId);
                        }
                        break;

                        case AF_TREE_ITEM_APP_ROOT:
                        case AF_TREE_ITEM_MESSAGE:
                        case AF_TREE_ITEM_GL_RENDER_CONTEXT:
                        case AF_TREE_ITEM_GL_TEXTURES_NODE:
                        case AF_TREE_ITEM_GL_TEXTURE:
                        case AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE:
                        case AF_TREE_ITEM_GL_RENDER_BUFFER:
                        case AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE:
                        case AF_TREE_ITEM_GL_STATIC_BUFFER:
                        case AF_TREE_ITEM_GL_PBUFFERS_NODE:
                        case AF_TREE_ITEM_GL_PBUFFER_NODE:
                        case AF_TREE_ITEM_GL_SYNC_OBJECTS_NODE:
                        case AF_TREE_ITEM_GL_SYNC_OBJECT:
                        case AF_TREE_ITEM_GL_VBO_NODE:
                        case AF_TREE_ITEM_GL_VBO:
                        case AF_TREE_ITEM_GL_PROGRAMS_NODE:
                        case AF_TREE_ITEM_GL_PROGRAM:
                        case AF_TREE_ITEM_GL_SHADERS_NODE:
                        case AF_TREE_ITEM_GL_VERTEX_SHADER:
                        case AF_TREE_ITEM_GL_TESSELLATION_CONTROL_SHADER:
                        case AF_TREE_ITEM_GL_TESSELLATION_EVALUATION_SHADER:
                        case AF_TREE_ITEM_GL_GEOMETRY_SHADER:
                        case AF_TREE_ITEM_GL_FRAGMENT_SHADER:
                        case AF_TREE_ITEM_GL_COMPUTE_SHADER:
                        case AF_TREE_ITEM_GL_UNSUPPORTED_SHADER:
                        case AF_TREE_ITEM_GL_DISPLAY_LISTS_NODE:
                        case AF_TREE_ITEM_GL_DISPLAY_LIST:
                        case AF_TREE_ITEM_GL_FBO_NODE:
                        case AF_TREE_ITEM_GL_FBO:
                        case AF_TREE_ITEM_CL_CONTEXT:
                        case AF_TREE_ITEM_CL_IMAGE:
                        case AF_TREE_ITEM_CL_BUFFER:
                        case AF_TREE_ITEM_CL_PIPE:
                        case AF_TREE_ITEM_CL_SAMPLER:
                        case AF_TREE_ITEM_CL_EVENT:
                        case AF_TREE_ITEM_CL_COMMAND_QUEUE:
                        case AF_TREE_ITEM_CL_PROGRAM:
                        {
                            // These should never be the [direct] child of a compute context!
                            GT_ASSERT(false);
                        }
                        break;

                        default:
                        {
                            // We added a new item type, but didn't implement it here
                            GT_ASSERT(false);
                        }
                        break;
                    }
                }
            }

            // Select the render context node and expand it:
            if (pContextTreeId->childCount() > 0)
            {
                pContextTreeId->setExpanded(true);
            }

            pContextTreeId->setSelected(true);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::onComputationProgramDependentAllocatedObjectLeak
// Description: Handling memory leak for CL program-dependent allocated objects (kernelHandles)
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        29/7/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::onComputationProgramDependentAllocatedObjectLeak(const apMemoryLeakEvent& eve)
{
    bool retVal = false;

    unsigned int containingContextIndex = 0;
    unsigned int programIndex = 0;
    eve.leakingObjectsComputationProgramID(containingContextIndex, programIndex);
    apContextID containingContextId(AP_OPENCL_CONTEXT, containingContextIndex);

    // Get the context tree item id:
    QTreeWidgetItem* pContextTreeId = getContextTreeItemId(containingContextId);
    GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
    {
        // Get the tree ID of the context's programs:
        QTreeWidgetItem* pContextProgramsNode = NULL;

        for (int i = 0; i < pContextTreeId->childCount(); i++)
        {
            QTreeWidgetItem* pContextChildItemId = pContextProgramsNode->child(i);

            if (pContextChildItemId != NULL)
            {
                // Get the item data:
                const afApplicationTreeItemData* pContextChildItemData = getTreeItemData(pContextChildItemId);
                GT_IF_WITH_ASSERT(pContextChildItemData != NULL)
                {
                    // If this is the programs node:
                    if (pContextChildItemData->m_itemType == AF_TREE_ITEM_CL_PROGRAMS_NODE)
                    {
                        // Return it and stop looking:
                        pContextProgramsNode = pContextChildItemId;
                        break;
                    }
                }
            }
        }

        // Since the about-to-be deleted program is not yet deleted, the context should have a programs node:
        GT_IF_WITH_ASSERT(pContextProgramsNode != NULL)
        {
            // Find the correct program:
            QTreeWidgetItem* pProgramTreeId = NULL;

            for (int i = 0 ; i < pContextProgramsNode->childCount(); i++)
            {
                QTreeWidgetItem* pCurrentProgramItemId = pContextProgramsNode->child(i);

                if (pCurrentProgramItemId != NULL)
                {
                    // Get the item data:
                    gdDebugApplicationTreeData* pCurrentProgramItemData = NULL;
                    afApplicationTreeItemData* pItemData = NULL;
                    getTreeItemDatas(pCurrentProgramItemId, pItemData, pCurrentProgramItemData);
                    GT_IF_WITH_ASSERT(pCurrentProgramItemData != NULL)
                    {
                        // Verify it is a program (this node should have no other types of children
                        GT_IF_WITH_ASSERT(pItemData->m_itemType == AF_TREE_ITEM_CL_PROGRAM)
                        {
                            // Check the program's index:
                            if (pCurrentProgramItemData->_objectOpenCLIndex == (int)programIndex)
                            {
                                // Return the tree Id and stop looking:
                                pProgramTreeId = pCurrentProgramItemId;
                                break;
                            }
                        }
                    }
                }
            }

            // If we found the program:
            GT_IF_WITH_ASSERT(pProgramTreeId != NULL)
            {
                retVal = true;

                // Mark all the program's child items as leaks:
                for (int i = 0; i < pProgramTreeId->childCount(); i++)
                {
                    QTreeWidgetItem* pProgramChildItemId = pProgramTreeId->child(i);

                    if (pProgramChildItemId != NULL)
                    {
                        // Get the item data:
                        const afApplicationTreeItemData* pProgramChildItemData = getTreeItemData(pProgramChildItemId);
                        GT_IF_WITH_ASSERT(pProgramChildItemData != NULL)
                        {
                            // Programs should only have kernelHandles as their children:
                            GT_IF_WITH_ASSERT(pProgramChildItemData->m_itemType == AF_TREE_ITEM_CL_KERNEL)
                            {
                                // Mark the kernel as a leak:
                                markItemAndAllDescendantsAsMemoryLeaks(pProgramChildItemId);
                            }
                        }

                        // Select the program node and expand it:
                        if (pProgramChildItemId->childCount() > 0)
                        {
                            pProgramChildItemId->setExpanded(true);
                        }

                        pProgramChildItemId->setSelected(true);
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::selectedContext
// Description: Return the context of the currently selected item
// Return Val:  const apContextID&
// Author:      Sigal Algranaty
// Date:        30/1/2011
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::selectedContext(apContextID& selectedContext) const
{
    selectedContext._contextType = AP_NULL_CONTEXT;
    selectedContext._contextId = 0;

    GT_IF_WITH_ASSERT(m_pApplicationTree != NULL)
    {
        // Get the currently selected item:
        QTreeWidgetItem* pSelectedItem = m_pApplicationTree->getTreeSelection();

        if (pSelectedItem != NULL)
        {
            // Get the item data for the selected item:
            afApplicationTreeItemData* pItemData = NULL;
            gdDebugApplicationTreeData* pGDItemData = NULL;
            getTreeItemDatas(pSelectedItem, pItemData, pGDItemData);

            // We do not want to fail if the tree root is selected:
            if (AF_TREE_ITEM_APP_ROOT != pItemData->m_itemType)
            {
                GT_IF_WITH_ASSERT(pGDItemData != NULL)
                {
                    selectedContext = pGDItemData->_contextId;
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::openImageOrBufferObject
// Description: Open the requested image / buffer object
// Arguments:   pItemData - the item data representing the buffer / image in the navigation tree
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/11/2010
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::openImageBufferObject(afApplicationTreeItemData* pItemData)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        GT_IF_WITH_ASSERT(m_pApplicationCommands != NULL)
        {
            // Get the object name from tree:
            QString itemStr = GetTreeItemText(pItemData->m_pTreeWidgetItem);

            // Display the selected kernel / program source code:
            retVal = m_pApplicationCommands->displayImageBufferObject(pItemData, acQStringToGTString(itemStr));
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::openShortcutItem
// Description: Open the requested FBO attachment
// Arguments:   pItemData - the item data representing the FBO attachment
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        14/2/2011
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::openShortcutItem(afApplicationTreeItemData* pItemData)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        // Make sure that the FBO attachment original item is set correctly:
        GT_IF_WITH_ASSERT(pItemData->m_pOriginalItemTreeItem != NULL)
        {
            // Get the original item data:
            afApplicationTreeItemData* pOriginalItemData = getTreeItemData(pItemData->m_pOriginalItemTreeItem);
            GT_IF_WITH_ASSERT(pOriginalItemData != NULL)
            {
                bool isShortcutItem = ((pOriginalItemData->m_itemType == AF_TREE_ITEM_GL_TEXTURE)
                                       || (pOriginalItemData->m_itemType == AF_TREE_ITEM_GL_RENDER_BUFFER)
                                       || (pOriginalItemData->m_itemType == AF_TREE_ITEM_GL_STATIC_BUFFER)
                                       || (pOriginalItemData->m_itemType == AF_TREE_ITEM_GL_PBUFFER_NODE));

                // If this is a shortcut item:
                GT_IF_WITH_ASSERT(isShortcutItem)
                {
                    // Open the original item:
                    retVal = openImageBufferObject(pOriginalItemData);
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::handleKernelDebuggingSelection
// Description: If we entered into a kernel debugging session, select the
//              debugged kernel in the tree
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        29/6/2011
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::handleKernelDebuggingSelection()
{
    // If we are in kernel debugging:
    if (gaIsInKernelDebugging())
    {
        // Get the currently debugged kernel function name:
        apCLKernel currentlyDebuggedKernel(OA_CL_NULL_HANDLE, 0, OA_CL_NULL_HANDLE, L"");
        bool rcKer = gaGetCurrentlyDebuggedKernelDetails(currentlyDebuggedKernel);
        GT_IF_WITH_ASSERT(rcKer)
        {
            // Get the context for the kernel:
            apContextID kernelContextID;
            rcKer = gaGetBreakpointTriggeringContextId(kernelContextID);
            GT_IF_WITH_ASSERT(rcKer)
            {
                // Get the OpenCL kernel item data:
                afApplicationTreeItemData* pKernelItemData = getOpenCLKernelData(kernelContextID, currentlyDebuggedKernel.programIndex(), currentlyDebuggedKernel.kernelHandle());
                GT_IF_WITH_ASSERT((pKernelItemData != NULL) && (m_pApplicationTree != NULL))
                {
                    // Select the kernel item in the tree:
                    m_pApplicationTree->selectItem(pKernelItemData, false);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::buildContextMenuForItem
// Description: Builds a context menu for a requested item
// Arguments:   const gtList<const afApplicationTreeItemData*> contextMenuItemsList
//              QMenu& menu
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/2/2011
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::BuildContextMenuForItems(const gtList<const afApplicationTreeItemData*> contextMenuItemsList, QMenu& menu)
{
    bool retVal = false;

    if (m_isDebuggedProcessSuspended)
    {
        if (contextMenuItemsList.size() == 1)
        {
            const afApplicationTreeItemData* pItemData = contextMenuItemsList.front();

            // Sanity check:
            GT_IF_WITH_ASSERT((pItemData != NULL) && (m_pApplicationTree != NULL))
            {
                QTreeWidgetItem* pContextMenuItem = pItemData->m_pTreeWidgetItem;
                GT_IF_WITH_ASSERT(pContextMenuItem != NULL)
                {
                    gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());

                    if (pGDData != NULL)
                    {
                        retVal = true;

                        // Get the selected item text:
                        QString menuItemText;
                        QString itemText = GetTreeItemText(pContextMenuItem);

                        // Get the tree item type:
                        afTreeItemType itemType = pItemData->m_itemType;

                        // Check if the item can be activated:
                        bool canBeActivated = afApplicationTreeItemData::isItemImageOrBuffer(itemType);
                        canBeActivated |= afApplicationTreeItemData::isItemThumbnail(itemType);
                        canBeActivated |= afApplicationTreeItemData::isItemSource(itemType);

                        if (canBeActivated)
                        {
                            // Build the string for the open item menu item:
                            menuItemText = QString(GD_STR_MonitoredObjectsTreeOpenItem).arg(itemText);

                            QAction* pAction = menu.addAction(menuItemText, this, SLOT(onOpenItem()));
                            QFont font = pAction->font();
                            font.setBold(true);
                            pAction->setFont(font);

                            menu.addSeparator();
                        }

                        // Add the memory view item to the context menu:
                        menuItemText = QString(GD_STR_MonitoredObjectsTreeViewItemMemory).arg(itemText);
                        menu.addAction(menuItemText, this, SLOT(onItemViewMemory()));

                        if (!pGDData->_contextId.isDefault())
                        {
                            // Build the context string:
                            gtString itemContextText;
                            pGDData->_contextId.toString(itemContextText);

                            // Build the menu item text:
                            menuItemText = QString(GD_STR_MonitoredObjectsTreeViewContextStatistics).arg(acGTStringToQString(itemContextText));

                            menu.addAction(menuItemText, this, SLOT(onContextStatistics()));
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::onItemViewMemory
// Description: Is triggered when the menu item "View item memory" is clicked
// Arguments:
// Author:      Sigal Algranaty
// Date:        19/1/2011
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::onItemViewMemory()
{
    // Sanity check: (m_pContextMenuTreeItemId should be set on context menu popup):
    GT_IF_WITH_ASSERT(m_pApplicationTree != NULL)
    {
        QList<QTreeWidgetItem*> treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();
        QTreeWidgetItem* pContextMenuItem = treeSelectedItems.at(0);
        GT_IF_WITH_ASSERT(pContextMenuItem != NULL)
        {
            // Get the selected item data:
            const afApplicationTreeItemData* pTreeItemData = getTreeItemData(pContextMenuItem);
            GT_IF_WITH_ASSERT(pTreeItemData != NULL)
            {
                GT_IF_WITH_ASSERT(m_pApplicationCommands != NULL)
                {
                    // Call the command handling the memory view display (to make sure that the memory view is displayed):
                    m_pApplicationCommands->raiseMemoryView();

                    // Get the memory view:
                    gdMemoryView* pMemoryView = m_pApplicationCommands->memoryView();
                    GT_IF_WITH_ASSERT(pMemoryView != NULL)
                    {
                        bool rcDisplay = pMemoryView->displayItemMemory(pTreeItemData);
                        GT_ASSERT(rcDisplay);
                    }
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::onContextStatistics
// Description: Is triggered when the menu item "View context statistics" is clicked
// Arguments:
// Author:      Sigal Algranaty
// Date:        30/1/2011
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::onContextStatistics()
{
    // Sanity check: (m_pContextMenuTreeItemId should be set on context menu popup):
    GT_IF_WITH_ASSERT(m_pApplicationTree != NULL)
    {
        QList<QTreeWidgetItem*> treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();
        QTreeWidgetItem* pContextMenuItem = treeSelectedItems.at(0);
        GT_IF_WITH_ASSERT(pContextMenuItem != NULL)
        {
            // Get the selected item data:
            afApplicationTreeItemData* pTreeItemData = NULL;
            gdDebugApplicationTreeData* pGDItemData = NULL;
            getTreeItemDatas(pContextMenuItem, pTreeItemData, pGDItemData);
            GT_IF_WITH_ASSERT((pTreeItemData != NULL) && (pGDItemData != NULL))
            {
                GT_IF_WITH_ASSERT(m_pApplicationCommands != NULL)
                {
                    // Call the command handling the statistics view display (to make sure that the memory view is displayed):
                    m_pApplicationCommands->raiseStatisticsView();

                    // Get the statistics view:
                    gdStatisticsPanel* pStatisticsPanel = m_pApplicationCommands->statisticsPanel();
                    GT_IF_WITH_ASSERT(pStatisticsPanel != NULL)
                    {
                        // Get the statistics view from the panel:
                        gdStatisticsView* pStatisticsView = pStatisticsPanel->statisticsView();
                        GT_IF_WITH_ASSERT(pStatisticsView != NULL)
                        {
                            bool rcDisplay = pStatisticsView->displayContext(pGDItemData->_contextId);
                            GT_ASSERT(rcDisplay);
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::onOpenItem
// Description: Is triggered when the menu item "Open Item" is clicked
// Arguments:
// Author:      Sigal Algranaty
// Date:        2/2/2011
// ---------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::onOpenItem()
{
    // Sanity check: (m_pContextMenuTreeItemId should be set on context menu popup):
    GT_IF_WITH_ASSERT(m_pApplicationTree != NULL)
    {
        QList<QTreeWidgetItem*> treeSelectedItems = m_pApplicationTree->treeControl()->selectedItems();
        QTreeWidgetItem* pContextMenuItem = treeSelectedItems.at(0);
        GT_IF_WITH_ASSERT(pContextMenuItem != NULL)
        {
            // Activate the item for whom the context menu was open:
            bool rcActivateItem = activateItem(pContextMenuItem);
            GT_ASSERT(rcActivateItem);
        }
    }
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        createObjectItemData
/// \brief Description: Create an item data, with the extended GD item data, and link the two
/// \param[in]          pNewItemData
/// \param[in]          pNewGDItemData
/// \return void
/// -----------------------------------------------------------------------------------------------
void gdDebugApplicationTreeHandler::createObjectItemData(afApplicationTreeItemData*& pNewItemData, gdDebugApplicationTreeData*& pNewGDItemData)
{
    pNewItemData = new afApplicationTreeItemData;

    pNewGDItemData = new gdDebugApplicationTreeData;

    // Set the new item data extension pointer:
    pNewItemData->setExtendedData(pNewGDItemData);
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        GetTreeItemText
/// \brief Description: Get a text for a tree item
/// \param[in]          pItem
/// \return gtString
/// -----------------------------------------------------------------------------------------------
QString gdDebugApplicationTreeHandler::GetTreeItemText(const QTreeWidgetItem* pItem) const
{
    QString retVal;
    GT_IF_WITH_ASSERT(m_pApplicationTree != NULL)
    {
        retVal = m_pApplicationTree->TreeItemText(pItem);
    }
    return retVal;
}

bool gdDebugApplicationTreeHandler::BuildItemHTMLPropeties(const afApplicationTreeItemData& displayedItemId, afHTMLContent& htmlContent)
{
    bool retVal = gdPropertiesEventObserver::instance().BuildItemHTMLProperties(&displayedItemId, false, true, true, htmlContent);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateProgramPipelinesList
// Description: Adds the current program pipeline objects to the tree
// Arguments:  int contextId - the OpenGL context id of the program pipelines
//             int& pipelineObjectsCount - the number of program pipeline objects (output parameter)
// Return Val: bool  - Success / failure.
// Author:     Amit Ben-Moshe
// Date:       24/6/2014
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateProgramPipelinesList(int contextId, int& pipelineObjectsCount)
{
    bool retVal = true;
    pipelineObjectsCount = 0;

    // Get the context tree item id:
    apContextID contextIdentifier(AP_OPENGL_CONTEXT, contextId);
    QTreeWidgetItem* pContextTreeId = getContextTreeItemId(contextIdentifier);

    // Make sure that the tree node is ok:
    GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
    {
        // Get amount of program pipelines for this context:
        retVal = gaGetAmountOfPipelineObjects(contextId, pipelineObjectsCount);
        GT_IF_WITH_ASSERT(retVal)
        {
            if (pipelineObjectsCount > 0)
            {
                // Display Progress Bar Message:
                gtString progressMessage;
                progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingProgramPipelines GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, contextId);
                afProgressBarWrapper::instance().setProgressDetails(progressMessage, pipelineObjectsCount);

                // Create a new memory item data, and fill the relevant fields:
                gdDebugApplicationTreeData* pPipelineTreeDataItem = NULL;
                afApplicationTreeItemData* pItemData = NULL;
                createObjectItemData(pItemData, pPipelineTreeDataItem);
                pItemData->m_itemType = AF_TREE_ITEM_GL_PROGRAM_PIPELINES_NODE;
                pItemData->m_objectCount = 0;
                pItemData->m_objectMemorySize = 0;

                pPipelineTreeDataItem->_contextId = contextIdentifier;
                pPipelineTreeDataItem->_objectOpenGLName = contextId;

                QTreeWidgetItem* pPipelinesBaseTreeItemId = appendItemToTree(pContextTreeId, GD_STR_MemoryAnalysisViewerTreeProgramPipelinesNode, m_openGLProgramPipelineIconIndex, pItemData);
                GT_IF_WITH_ASSERT(pPipelinesBaseTreeItemId != NULL)
                {
                    // Iterate the buffers and add each of them to the tree:
                    int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
                    int itemsToShow = min(pipelineObjectsCount, maxItemsShown);

                    for (int i = 0; i < itemsToShow; i++)
                    {
                        // Get the current program pipeline name:
                        GLuint pipelineName = 0;
                        retVal = retVal && gaGetPipelineObjectName(contextId, i, pipelineName);
                        GT_IF_WITH_ASSERT(retVal)
                        {
                            afApplicationTreeItemData* pNewPipelineItemData = NULL;
                            int imageIndex = m_openGLProgramPipelineIconIndex;

                            retVal = retVal && buildPipelineObjectData(contextId, pipelineName, pNewPipelineItemData, imageIndex);
                            GT_IF_WITH_ASSERT(retVal && (pNewPipelineItemData != NULL))
                            {
                                // Add the program pipeline to the tree:
                                gtString pipelineNameStr;
                                pipelineNameStr.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeProgramPipelineName, pipelineName);
                                QTreeWidgetItem* pPipelineTreeItemId = appendItemToTree(pPipelinesBaseTreeItemId, pipelineNameStr.asCharArray(), imageIndex, pNewPipelineItemData);
                                pItemData->m_objectMemorySize += pNewPipelineItemData->m_objectMemorySize;

                                // Increment the progress bar
                                afProgressBarWrapper::instance().incrementProgressBar();

                                retVal = retVal && (pPipelineTreeItemId != NULL);
                            }
                        }
                    }

                    // If some items were hidden:
                    if (maxItemsShown < pipelineObjectsCount)
                    {
                        // Show a message:
                        addMoreItemsMessage(pPipelinesBaseTreeItemId, (pipelineObjectsCount - maxItemsShown));
                    }

                    pItemData->m_objectCount += pipelineObjectsCount;
                }
            }
        }
    }
    return retVal;
}


// --------------------------------------------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::buildPipelineObjectData
// Description: Collects a program pipeline's object details, and builds a tree data item with that data
// Arguments: int contextId - the relevant OpenGL context id
//            GLuint pipelineName - the program pipeline's OpenGL name
//            afApplicationTreeItemData*& pItemData - an output parameter to hold the created data item
//            int& imageIndex - an output parameter to hold the item's index;
//            note: as for now, imageIndex uses the same index which is used for render buffer.
//                  this should be change/eliminated after creating a dedicated icon for program pipeline objects
// Return Val: bool  - Success / failure.
// Author:    Amit Ben-Moshe
// Date:      24/6/2014
// ---------------------------------------------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::buildPipelineObjectData(int contextId, GLuint pipelineName, afApplicationTreeItemData*& pItemData, int& imageIndex)
{
    bool retVal = false;

    // Get the render buffer object details:
    apGLPipeline pipelineDataBuffer;
    retVal = gaGetPipelineObjectDetails(contextId, pipelineName, pipelineDataBuffer);

    // Create the item data:
    gdDebugApplicationTreeData* pPipelineItemData = NULL;
    pItemData = NULL;
    createObjectItemData(pItemData, pPipelineItemData);

    GT_IF_WITH_ASSERT(pItemData != NULL && pPipelineItemData != NULL)
    {
        pItemData->m_itemType = AF_TREE_ITEM_GL_PROGRAM_PIPELINE;

        imageIndex = m_openGLProgramPipelineIconIndex;

        // Set the program pipeline's OpenGL name:
        pPipelineItemData->_objectOpenGLName = pipelineName;

        // Set the render context id;
        pPipelineItemData->_contextId._contextType = AP_OPENGL_CONTEXT;
        pPipelineItemData->_contextId._contextId = contextId;

    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::updateOpenGlSamplersList
// Description: Adds the current OpenGL sampler objects to the tree
// Arguments:  int contextId - the OpenGL context id of the OpenGL samplers
//             int& samplerObjectsCount - the number of sampler objects (output parameter)
// Return Val: bool  - Success / failure.
// Author:     Amit Ben-Moshe
// Date:       24/6/2014
// ---------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::updateOpenGlSamplersList(int contextId, int& samplerObjectsCount)
{
    bool retVal = true;
    samplerObjectsCount = 0;

    // Get the context tree item id:
    apContextID contextIdentifier(AP_OPENGL_CONTEXT, contextId);
    QTreeWidgetItem* pContextTreeId = getContextTreeItemId(contextIdentifier);

    // Make sure that the tree node is ok:
    GT_IF_WITH_ASSERT((pContextTreeId != m_pHeaderItem) && (pContextTreeId != NULL))
    {
        // Get amount of samplers for this context:
        retVal = gaGetAmountOfSamplerObjects(contextId, samplerObjectsCount);
        GT_IF_WITH_ASSERT(retVal)
        {
            if (samplerObjectsCount > 0)
            {
                // Display Progress Bar Message:
                gtString progressMessage;
                progressMessage.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeUpdatingOpenGlSamplers GD_STR_MemoryAnalysisViewerTreeUpdatingEllipsis, contextId);
                afProgressBarWrapper::instance().setProgressDetails(progressMessage, samplerObjectsCount);

                // Create a new memory item data, and fill the relevant fields:
                gdDebugApplicationTreeData* pSamplerTreeDataItem = NULL;
                afApplicationTreeItemData* pItemData = NULL;
                createObjectItemData(pItemData, pSamplerTreeDataItem);
                pItemData->m_itemType = AF_TREE_ITEM_GL_SAMPLERS_NODE;
                pItemData->m_objectCount = 0;
                pItemData->m_objectMemorySize = 0;

                pSamplerTreeDataItem->_contextId = contextIdentifier;
                pSamplerTreeDataItem->_objectOpenGLName = contextId;

                QTreeWidgetItem* pSamplersBaseTreeItemId = appendItemToTree(pContextTreeId, GD_STR_MemoryAnalysisViewerTreeOpenGlSamplersNode, m_glSamplerIconIndex, pItemData);
                GT_IF_WITH_ASSERT(pSamplersBaseTreeItemId != NULL)
                {
                    // Iterate the buffers and add each of them to the tree:
                    int maxItemsShown = gdGDebuggerGlobalVariablesManager::instance().maxTreeItemsPerType();
                    int itemsToShow = min(samplerObjectsCount, maxItemsShown);

                    for (int i = 0; i < itemsToShow; i++)
                    {
                        // Get the current sampler name:
                        GLuint samplerName = 0;
                        retVal = retVal && gaGetSamplerObjectName(contextId, i, samplerName);
                        GT_IF_WITH_ASSERT(retVal)
                        {
                            afApplicationTreeItemData* pNewSamplerItemData = NULL;

                            int imageIndex = m_glSamplerIconIndex;

                            retVal = retVal && buildSamplerObjectData(contextId, samplerName, pNewSamplerItemData, imageIndex);
                            GT_IF_WITH_ASSERT(retVal && (pNewSamplerItemData != NULL))
                            {
                                // Add the sampler item to the tree:
                                gtString samplerNameStr;
                                samplerNameStr.appendFormattedString(GD_STR_MemoryAnalysisViewerTreeOpenGlSamplerName, samplerName);
                                QTreeWidgetItem* pSamplerTreeItemId = appendItemToTree(pSamplersBaseTreeItemId, samplerNameStr.asCharArray(), imageIndex, pNewSamplerItemData);
                                pItemData->m_objectMemorySize += pNewSamplerItemData->m_objectMemorySize;

                                // Increment the progress bar
                                afProgressBarWrapper::instance().incrementProgressBar();

                                retVal = retVal && (pSamplerTreeItemId != NULL);
                            }
                        }

                        // If some items were hidden:
                        if (maxItemsShown < samplerObjectsCount)
                        {
                            // Show a message:
                            addMoreItemsMessage(pSamplersBaseTreeItemId, (samplerObjectsCount - maxItemsShown));
                        }
                    }

                    pItemData->m_objectCount += samplerObjectsCount;
                }
            }
        }
    }
    return retVal;
}

// --------------------------------------------------------------------------------------------------------------
// Name:        gdDebugApplicationTreeHandler::buildSamplerObjectData
// Description: Collects an OpenGL sampler object's details, and builds a tree data item with that data
// Arguments: int contextId - the relevant OpenGL context id
//            GLuint samplerName - the sampler's OpenGL name
//            afApplicationTreeItemData*& pItemData - an output parameter to hold the created data item
//            int& imageIndex - an output parameter to hold the item's index;
//            note: as for now, imageIndex uses the same index which is used for render buffer.
//                  this should be change/eliminated after creating a dedicated icon for OpenGL sampler objects
// Return Val: bool  - Success / failure.
// Author:    Amit Ben-Moshe
// Date:      24/6/2014
// ---------------------------------------------------------------------------------------------------------------
bool gdDebugApplicationTreeHandler::buildSamplerObjectData(int contextId, GLuint samplerName, afApplicationTreeItemData*& pItemData, int& imageIndex)
{
    // Create the item data:
    gdDebugApplicationTreeData* pSamplerItemData = NULL;
    pItemData = NULL;
    createObjectItemData(pItemData, pSamplerItemData);

    GT_IF_WITH_ASSERT(pItemData != NULL && pSamplerItemData != NULL)
    {
        pItemData->m_itemType = AF_TREE_ITEM_GL_SAMPLER;

        // TODO: CHANGE THE ICON!!
        imageIndex = m_glSamplerIconIndex;

        // Set the sampler's OpenGL name:
        pSamplerItemData->_objectOpenGLName = samplerName;

        // Set the render context id;
        pSamplerItemData->_contextId._contextType = AP_OPENGL_CONTEXT;
        pSamplerItemData->_contextId._contextId = contextId;
    }
    return true;
}

