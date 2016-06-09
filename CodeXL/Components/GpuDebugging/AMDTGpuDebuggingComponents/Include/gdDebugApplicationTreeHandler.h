//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdDebugApplicationTreeHandler.h
///
//==================================================================================
#ifndef __GDMONITOREDOBJECTSTREE
#define __GDMONITOREDOBJECTSTREE

// Qt:
#include <QtWidgets>
#include <QWidget>

class apCLBuffer;
class apCLCommandQueue;
class apCLEvent;
class apCLImage;
class apCLPipe;
class apCLProgram;
class apCLSampler;
class apCLSubBuffer;
class apGLDisplayList;
class apGLRenderBuffer;
class apGLShaderObject;
class apGLTexture;
class apGLTextureMemoryData;
class apGLVBO;
class apMemoryLeakEvent;
class afProgressBarWrapper;
class apStaticBuffer;
class acToolBar;
class acTreeCtrl;
class afApplicationTree;
class gdApplicationCommands;
class gdDebugApplicationTreeData;

// Infra:
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTAPIClasses/Include/apContextID.h>
#include <AMDTAPIClasses/Include/apDisplayBuffer.h>
#include <AMDTAPIClasses/Include/apGLTexture.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>
#include <AMDTApplicationFramework/Include/views/afBaseView.h>
#include <AMDTApplicationFramework/Include/afTreeItemType.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeData.h>

// ----------------------------------------------------------------------------------
// Class Name:          gdDebugApplicationTreeHandler: public QWidget , public apIEventsObserver, public afBaseView
// General Description: A basic monitored objects navigation tree. The tree builds a monitored
//                      objects with the current objects existing in the API.
//                      The tree can be inherited and used for various navigation purposes.
// Author:              Sigal Algranaty
// Creation Date:       26/9/2010
// ----------------------------------------------------------------------------------
class GD_API gdDebugApplicationTreeHandler : public afApplicationTreeHandler, public apIEventsObserver
{

    Q_OBJECT

public:

    gdDebugApplicationTreeHandler();
    virtual ~gdDebugApplicationTreeHandler();

    static gdDebugApplicationTreeHandler* instance();

    bool doesItemExist(const osFilePath& objectFilePath, afApplicationTreeItemData*& pOriginalItemData) const;

    // afApplicationTreeHandler overrides:
    virtual bool BuildContextMenuForItems(const gtList<const afApplicationTreeItemData*> contextMenuItemsList, QMenu& menu);
    virtual afApplicationTreeItemData* FindMatchingTreeItem(const afApplicationTreeItemData& displayedItemId);
    virtual void SetItemsVisibility() {}; // Do nothing
    virtual bool BuildItemHTMLPropeties(const afApplicationTreeItemData& displayedItemId, afHTMLContent& htmlContent);

    // Updates the view with the current existing monitored objects:
    virtual bool updateMonitoredObjectsTree();
    void setInfoUpdated(bool isUpdated) {m_isInfoUpdated = isUpdated;}
    bool shouldUpdateMemoryData() const {return m_isTextureMemoryDataUpdateRequired;}
    void setShouldUpdateMemoryData(bool shouldUpdate);

    // Tree functionality:
    afApplicationTreeItemData* getTreeItemData(QTreeWidgetItem* pTreeItem) const;
    void getTreeItemDatas(QTreeWidgetItem* pTreeItem, afApplicationTreeItemData*& pItemData, gdDebugApplicationTreeData*& pGDItemData) const;
    gdDebugApplicationTreeData* FindMatchingTreeItemGDData(const afApplicationTreeItemData& displayedItemId);
    QString GetTreeItemText(const QTreeWidgetItem* pItem) const;
    bool treeItemHasChildren(QTreeWidgetItem* pTreeItem) const;
    int getTreeChildrenCount(QTreeWidgetItem* pTreeItem, bool recursively) const;
    QTreeWidgetItem* getTreeItemParent(const QTreeWidgetItem* pTreeItem) const;
    void createObjectItemData(afApplicationTreeItemData*& pNewItemData, gdDebugApplicationTreeData*& pNewGDItemData);

    void setItemIcon(QTreeWidgetItem* pItem, int index, bool recursive = false);

    // Clear all tree items:
    virtual void clearTreeItems(bool addNonAvailableMessage);
    void clearTreeSelection();

    // Debugged process events callback function:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);

    // Event observer name:
    virtual const wchar_t* eventObserverName() const { return L"DebuggerObjectsNavigationTree"; };

    virtual bool updateContextSpecificData(bool isDeleted, gdDebugApplicationTreeData* pRenderContextItemData) { (void)(isDeleted); (void)(pRenderContextItemData); return false;};
    void handleKernelDebuggingSelection();

    void countShadersByType(gdDebugApplicationTreeData* pShadersNodeItemData);
    void countTexturesByType(gdDebugApplicationTreeData* pTexturesNodeItemData);

    int amountOfDisplayedObjectForType(const apContextID& contextID, afTreeItemType objectType) const ;
    int amountOfDisplayedBuffers(const apContextID& contextID) const ;
    int amountOfFBOAttachments(const apContextID& contextID, int fboIndex) const;

    bool getPBufferId(int index, int& PBufferId) const ;
    bool getFBONameFromIndex(const apContextID& contextID, int index, GLuint& fboName) const ;
    int getPBufferIndex(int pbufferId) const ;
    int getFBOIndex(const apContextID& contextID, GLuint fboName) const ;
    int getAmountOfBuffersAttachedToPBuffer(int index) const ;
    afApplicationTreeItemData* getPBufferAttachmentItemData(int pbufferIndex, int attahmentIndex) const;
    afApplicationTreeItemData* getFBOAttachmentItemData(const apContextID& contextID, int fboIndex, int attahmentIndex) const;

    // Items tree data accessors:
    afApplicationTreeItemData* getOpenGLObjectDataByName(const afApplicationTreeItemData& ecobjtID);
    afApplicationTreeItemData* getItemDataByType(const apContextID& contextID, afTreeItemType itemType, int itemIndex = -1) const;
    afApplicationTreeItemData* getOpenCLObjectDataByIndex(const apContextID& contextID, afTreeItemType objectType, int objectIndex);
    afApplicationTreeItemData* getOpenCLObjectDataByName(const apContextID& contextID, afTreeItemType objectType, int objectName, int ownerObjectIndex);
    afApplicationTreeItemData* getOpenGLContextData(const apContextID& contextID);
    afApplicationTreeItemData* getOpenCLContextData(const apContextID& contextID);
    afApplicationTreeItemData* getOpenCLKernelData(const apContextID& contextID, int programIndex, int kernelIndex);
    afApplicationTreeItemData* getOpenCLKernelData(const apContextID& contextID, int programIndex, oaCLKernelHandle kernelHandle);
    afApplicationTreeItemData* getFBODataByFBOAttachment(const afApplicationTreeItemData& ecobjtID);
    afApplicationTreeItemData* getStaticBufferDataByType(const apContextID& contextID, apDisplayBuffer bufferType);
    afApplicationTreeItemData* getPBufferStaticBufferDataByIdAndType(const apContextID& contextID, int pbufferId, apDisplayBuffer bufferType);
    afApplicationTreeItemData* getPBufferDataById(const apContextID& contextID, int pbufferID);

    void selectedContext(apContextID& contextID) const;

    // Images and buffers:
    bool openImageBufferObject(afApplicationTreeItemData* pItemData);
    bool openShortcutItem(afApplicationTreeItemData* pItemData);

    // Get the last activated item data:
    afApplicationTreeItemData* activatedItemData() const {return m_pActivatedItemData;}

    bool onMemoryLeakBreakpoint(const apMemoryLeakEvent& eve);

    // Edit menu commands:
    virtual void onUpdateEdit_Copy(bool& isEnabled);
    virtual void onUpdateEdit_SelectAll(bool& isEnabled);
    virtual void onEdit_Copy();
    virtual void onEdit_SelectAll();

protected slots:
    void onOpenItem();
    void onItemViewMemory();
    void onContextStatistics();

protected:
    virtual bool activateItem(QTreeWidgetItem* pItemToActivate);

    // OpenGL & OpenCL:
    bool updateOpenGLMonitoredObjects(gtVector<apContextID>& vListSharingContexts);
    bool updateOpenCLMonitoredObjects(gtVector<apContextID>& vListSharingContexts);
    bool addMoreItemsMessage(QTreeWidgetItem* pParent, int hiddenItemsCount);

    // Utilities:
    void initializeGLObjectsUpdateProgress();

    // Tree Display:
    void setCLNodeTextFromStrAndData(gtString& originalString, afApplicationTreeItemData* treeNodeData);

    // OpenGL memory items:
    bool updateApplicationGLRenderContexts(unsigned int& objectCount, gtVector<apContextID>& vListSharingContexts);
    bool updateApplicationPBuffers(unsigned int& objectCount);
    bool updatePBufferAttachments(const afApplicationTreeItemData& pbufferItemData);
    bool updateApplicationSyncObjects(unsigned int& objectCount);
    bool updateGLTexturesList(int contextId, gtUInt32& texturesMemorySize, unsigned int& textureObjectCount);
    bool updateRenderBuffersList(int contextId, gtUInt32& renderBuffersMemorySize, unsigned int& renderBufferObjectCount);
    bool updateStaticBuffersList(int contextId, gtUInt32& staticBuffersMemorySize, unsigned int& staticBufferObjectCount);
    bool updateVBOList(int contextId, gtUInt32& vbosMemorySize, unsigned int& vboCount);
    bool updateProgramsList(int contextId, gtUInt32& programsMemorySize, unsigned int& programObjectCount);
    bool updateShadersList(int contextId, gtUInt32& shadersMemorySize, unsigned int& shaderObjectCount);
    bool updateDisplayListsList(int contextId, gtUInt32& dispListsMemorySize, unsigned int& displayListObjectCount);
    bool updateFBOsList(int contextId, gtUInt32& fbosMemorySize, unsigned int& fboCount);
    bool updateFBOAttachments(int contextId, int fboIndex, int& fboAttachmentsCount, QTreeWidgetItem* pFBOTreeItemId);
    bool updateOpenGLRenderContext(int renderContextId, unsigned int& objectCount, gtVector<apContextID>& vListSharingContexts);
    bool updateProgramPipelinesList(int contextId, int& pipelineObjectsCount);
    bool updateOpenGlSamplersList(int contextId, int& samplerObjectsCount);
    void markSharingContexts(const gtVector<apContextID>& vListSharingContexts);

    // OpenCL memory items:
    bool updateApplicationOpenCLContexts(unsigned int& objectCount, gtVector<apContextID>& vListSharingContexts);
    bool updateCLImagesList(apContextID contextID, gtUInt32& texturesMemorySize, unsigned int& textureObjectCount);
    bool updateCLBuffersList(apContextID contextID, gtUInt32& buffersMemorySize, unsigned int& buffersObjectsCount);
    bool updateCLSubBuffersList(apContextID contextID, int ownerBufferIndex, QTreeWidgetItem* pBufferTreeId, const apCLBuffer& bufferDetails, int& subBuffersObjectsCount);
    bool updateCLPipesList(apContextID contextID, gtUInt32& pipesMemorySize, unsigned int& pipesObjectsCount);
    bool updateCLCommandQueuesList(apContextID contextID, gtUInt32& commandQueuesMemorySize, unsigned int& commandQueuesObjectsCount);
    bool updateCLProgramsList(apContextID contextID, gtUInt32& programsMemorySize, unsigned int& programObjectsCount);
    bool updateCLKernelsList(apContextID contextID, QTreeWidgetItem* pProgramTreeId, int programIndex, const apCLProgram& programDetails, gtUInt32& kernelsMemorySize, unsigned int& kernelsObjectsCount);
    bool updateCLSamplersList(apContextID contextID, gtUInt32& samplersMemorySize, unsigned int& samplersObjectsCount);
    bool updateCLEventsList(apContextID contextID, gtUInt32& eventsMemorySize, unsigned int& eventsObjectsCount);

    // Objects memory data help functions:
    QTreeWidgetItem* getContextTreeItemId(const apContextID& contextId);

    // Virtual functions that build the objects item data:
    bool buildTextureObjectData(int contextId, const apGLTextureMemoryData& textureMemoryData, afApplicationTreeItemData*& pTextureItemData, int& imageIndex);
    bool buildRenderBufferObjectData(int contextId, GLuint renderBufferName, afApplicationTreeItemData*& pRenderBufferItemData, int& imageIndex);
    bool buildPipelineObjectData(int contextId, GLuint pipelineName, afApplicationTreeItemData*& pItemData, int& imageIndex);
    bool buildSamplerObjectData(int contextId, GLuint samplerName, afApplicationTreeItemData*& pItemData, int& imageIndex);
    bool buildStaticBufferObjectData(int contextId, apDisplayBuffer bufferType, afApplicationTreeItemData*& pStaticBufferItemData, int& imageIndex);
    bool buildVBOObjectData(int contextId, int vboIndex, afApplicationTreeItemData*& pVBOItemData, int& imageIndex, apGLVBO& vboDetails);
    bool buildProgramObjectData(int contextId, int programIndex, afApplicationTreeItemData*& pProgramItemData, int& imageIndex);
    bool buildShaderObjectData(int contextId, int shaderIndex, afApplicationTreeItemData*& pShaderItemData, int& imageIndex, gtString& shaderNameString);
    bool buildDisplayListObjectData(int contextId, int listIndex, afApplicationTreeItemData*& pListItemData, int& imageIndex);
    bool buildFBOObjectData(int contextId, int fboIndex, afApplicationTreeItemData*& pFBOItemData, int& imageIndex);
    bool buildCLImageObjectData(apContextID contextID, int textureIndex, const apCLImage& pTextureDetails, afApplicationTreeItemData*& pTextureItemData, int& imageIndex);
    bool buildCLBufferObjectData(apContextID contextID, int bufferIndex, const apCLBuffer& pBufferDetails, afApplicationTreeItemData*& pBufferItemData, int& imageIndex);
    bool buildCLPipeObjectData(apContextID contextID, int pipeIndex, const apCLPipe& pPipeDetails, afApplicationTreeItemData*& pPipeItemData, int& imageIndex);
    bool buildCLSamplerObjectData(apContextID contextID, int samplerIndex, const apCLSampler& pSamplerDetails, afApplicationTreeItemData*& pSamplerItemData, int& imageIndex);
    bool buildCLEventObjectData(apContextID contextID, int eventIndex, const apCLEvent& pEventDetails, afApplicationTreeItemData*& pEventItemData, int& imageIndex);
    bool buildCLCommandQueueObjectData(apContextID contextID, int commandQueueId, const apCLCommandQueue& pCommandQueueDetails, afApplicationTreeItemData*& pCommandQueueItemData, int& imageIndex);
    bool buildCLProgramObjectData(apContextID contextID, int programId, const apCLProgram& pProgramDetails, afApplicationTreeItemData*& pProgramItemData, int& imageIndex);

    // Function handling memory data for each of the object types:
    void fillTextureMemoryData(const apGLTextureMemoryData& textureMemoryData, afApplicationTreeItemData*& pTextureItemData);
    void fillRenderBufferMemoryData(apGLRenderBuffer& renderBufferDetails, afApplicationTreeItemData*& pRenderBufferItemData);
    void fillStaticBufferMemoryData(apStaticBuffer& staticBuffersDetails, afApplicationTreeItemData*& pStaticBufferItemData);
    void fillVBOMemoryData(const apGLVBO& vboDetails, afApplicationTreeItemData*& pVBOItemData);
    void fillShaderMemoryData(afApplicationTreeItemData*& pShaderItemData, gtAutoPtr<apGLShaderObject>& aptrShaderDetails);
    void fillDisplayListMemoryData(const apGLDisplayList& displayListDetails, afApplicationTreeItemData*& pListItemData);
    void fillCLImageMemoryData(const apCLImage& pTextureDetails, afApplicationTreeItemData*& pTextureItemData);
    void fillCLBufferMemoryData(const apCLBuffer& pBufferDetails, afApplicationTreeItemData*& pBufferItemData);
    void fillCLSubBufferMemoryData(const apCLSubBuffer& pSubBufferDetails, afApplicationTreeItemData*& pSubBufferItemData);
    void fillCLPipeMemoryData(const apCLPipe& pPipeDetails, afApplicationTreeItemData*& pPipeItemData);
    void fillCLSamplerMemoryData(const apCLSampler& samplerDetails, afApplicationTreeItemData*& pSamplerItemData);
    void fillCLEventMemoryData(const apCLEvent& eventDetails, afApplicationTreeItemData*& pEventItemData);
    void fillCLCommandQueueMemoryData(const apCLCommandQueue& pCommandQueueDetails, afApplicationTreeItemData*& pCommandQueueItemData);
    void fillCLProgramMemoryData(const apCLProgram& pProgramDetails, afApplicationTreeItemData*& pProgramItemData);

    // Update memory leaks in the viewer and get their size:
    bool onIndependentGLAllocatedObjectLeak(const apMemoryLeakEvent& eve);
    bool onIndependentCLAllocatedObjectLeak(const apMemoryLeakEvent& eve);
    bool onRenderContextDependentAllocatedObjectLeak(const apMemoryLeakEvent& eve);
    bool onComputeContextDependentAllocatedObjectLeak(const apMemoryLeakEvent& eve);
    bool onComputationProgramDependentAllocatedObjectLeak(const apMemoryLeakEvent& eve);

    bool updateTreeItemMemoryData(QTreeWidgetItem* pTreeItem);
    bool updateContextMemoryData(bool isDeleted, afApplicationTreeItemData* pRenderContextItemData);
    bool updateMemoryDataByType(afApplicationTreeItemData* pItemData);

    // Implement this function in inherited classes if textures update is necessary:
    bool updateTexturesMemoryParameters(int contextId, const gtVector<apGLTextureMipLevelID>& textureNamesForUpdate);

    void markItemAndAllDescendantsAsMemoryLeaks(QTreeWidgetItem* pTreeItemID);

    bool getCLContextLivingImageObjects(const apContextID& contextID, gtPtrVector<apCLImage*>& contextImages);
    bool getCLContextLivingBufferObjects(const apContextID& contextID, gtPtrVector<apCLBuffer*>& contextBuffers);
    bool getCLContextLivingPipeObjects(const apContextID& contextID, gtPtrVector<apCLPipe*>& contextPipes);
    bool getCLContextLivingSamplerObjects(const apContextID& contextID, gtPtrVector<apCLSampler*>& contextSamplers);
    bool getCLContextLivingEventObjects(const apContextID& contextID, gtPtrVector<apCLEvent*>& contextEvents);
    bool getCLContextLivingQueueObjects(const apContextID& contextID, gtPtrVector<apCLCommandQueue*>& contextQueues, gtVector<int>& queueIndices);
    bool getCLContextLivingProgramObjects(const apContextID& contextID, gtPtrVector<apCLProgram*>& contextPrograms, gtVector<int>& programIndices);

    bool createAndLoadImageList();
    int textureTypeToIconIndex(const apContextID& contextId, apTextureType textureType);
    void expandCurrentContext();

    void selectPreviouslySelectedItem();

    class gdTreeItemId
    {
    public:
        gdTreeItemId(const apContextID& contextID, afTreeItemType itemType, int itemIndex)
            : m_itemType(itemType), m_itemIndex(itemIndex), m_contextID(contextID) {};
        gdTreeItemId()
            : m_itemType(AF_TREE_ITEM_ITEM_NONE), m_itemIndex(-1) {};
        gdTreeItemId(const gdTreeItemId& other)
            : m_itemType(other.m_itemType), m_itemIndex(other.m_itemIndex), m_contextID(other.m_contextID) {};

        afTreeItemType m_itemType;
        int m_itemIndex;
        apContextID m_contextID;

        bool operator<(const gdTreeItemId& other) const
        {
            bool retVal = false;

            if ((int)m_itemType < (int)other.m_itemType)
            {
                retVal = true;
            }
            else if ((int)m_itemType == (int)other.m_itemType)
            {
                if ((int)m_contextID._contextType < (int)other.m_contextID._contextType)
                {
                    retVal = true;
                }
                else if ((int)m_contextID._contextType == (int)other.m_contextID._contextType)
                {
                    if ((int)m_contextID._contextId < (int)other.m_contextID._contextId)
                    {
                        retVal = true;
                    }
                    else if ((int)m_contextID._contextId == (int)other.m_contextID._contextId)
                    {
                        retVal = (m_itemIndex < other.m_itemIndex);
                    }
                }
            }

            return retVal;
        }
    };

    QTreeWidgetItem* appendItemToTree(QTreeWidgetItem* parentTree, const gtString& itemLabel, int itemImageIndex, afApplicationTreeItemData* pItemTreeData);

    // Maps from item index to an item data:
    gtMap<gdTreeItemId, afApplicationTreeItemData*> m_itemIndexToItemDataMap;

    // Optimization - save the amount of children for each buffers root:
    // gtMap<wxTreeItemIdValue, int> _itemRootToAmountOfChildrenMap;

private:
    // My single instance:
    static gdDebugApplicationTreeHandler* m_pMySingleInstance;

protected:
    // Icons:
    gtPtrVector<QPixmap*> m_treeItemsVector;

    // Tree control:
    afApplicationTree* m_pApplicationTree;
    QTreeWidgetItem* m_pHeaderItem;

    // This should be true iff we need to update GL Texture Memory.
    bool m_isTextureMemoryDataUpdateRequired;

    // Contain true iff the information for the tree is updated for the last process suspension:
    bool m_isInfoUpdated;

    // Tree nodes for the application tree:
    QTreeWidgetItem* m_pPBuffersTreeId;
    QTreeWidgetItem* m_pSyncObjectsTreeId;
    gtVector<QTreeWidgetItem*> m_openGLContextsTreeIds;
    gtVector<QTreeWidgetItem*> m_openCLContextsTreeIds;

    // Holds the last memory leak event handled:
    apMemoryLeakEvent* m_pLastMemoryLeakEvent;

    // Icons indices:
    int m_texture1DIconIndex;
    int m_texture2DIconIndex;
    int m_texture3DIconIndex;
    int m_texture1DArrayIconIndex;
    int m_texture2DArrayIconIndex;
    int m_texture2DMultisampleIconIndex;
    int m_texture2DMultisampleArrayIconIndex;
    int m_allTexturesIconIndex;
    int m_texturesShortcutIconIndex;
    int m_textureCubeMapIconIndex;
    int m_textureCubeMapArrayIconIndex;
    int m_textureRectangleIconIndex;
    int m_textureBufferIconIndex;
    int m_textureUnknownIconIndex;
    int m_openGLBufferGenericIconIndex;
    int m_openGLBufferArrayIconIndex;
    int m_openGLBufferDrawIndirIconIndex;
    int m_openGLBufferDispatchIndirIconIndex;
    int m_openGLBufferElementArrayIconIndex;
    int m_openGLBufferPixelPackIconIndex;
    int m_openGLBufferPixelUnpackIconIndex;
    int m_openGLBufferCopyReadIconIndex;
    int m_openGLBufferCopyWriteIconIndex;
    int m_openGLBufferTransformFeedbackIconIndex;
    int m_openGLBufferUniformIconIndex;
    int m_openGLBufferAtomicIconIndex;
    int m_openGLBufferShaderStorageIconIndex;
    int m_openGLBufferQueryIconIndex;
    int m_openGLBufferTextureIconIndex;
    int m_openGLBufferUnknownIconIndex;
    int m_renderBufferIconIndex;
    int m_renderBufferShortcutIconIndex;
    int m_staticBufferIconIndex;
    int m_staticBufferShortcutIconIndex;
    int m_pbufferIconIndex;
    int m_syncObjectsIconIndex;
    int m_commandQueueIconIndex;
    int m_displayListIconIndex;
    int m_renderContextIconIndex;
    int m_renderContextDeletedIconIndex;
    int m_renderContextSharedIconIndex;
    int m_renderContextDeletedSharedIconIndex;
    int m_computeContextIconIndex;
    int m_computeContextDeletedIconIndex;
    int m_openGLProgramIconIndex;
    int m_openGLProgramDeletedIconIndex;
    int m_openGLShaderIconIndex;
    int m_openGLShaderDeletedIconIndex;
    int m_fboIconIndex;
    int m_glSamplerIconIndex;
    int m_openGLProgramPipelineIconIndex;
    int m_allCLImagesIconIndex;
    int m_clImage2DIconIndex;
    int m_clImage3DIconIndex;
    int m_clBufferIconIndex;
    int m_clPipeIconIndex;
    int m_clSamplerIconIndex;
    int m_clEventIconIndex;
    int m_openCLProgramIconIndex;
    int m_openCLKernelIconIndex;
    int m_memoryLeakIconIndex;
    int m_informationIconIndex;

    // Contain true iff the debugged process is suspended:
    bool m_isDebuggedProcessSuspended;

    // Application commands instance:
    gdApplicationCommands* m_pApplicationCommands;

    // Will hold the last activated item data:
    afApplicationTreeItemData* m_pActivatedItemData;
};


#endif  // __GDMONITOREDOBJECTSTREE
