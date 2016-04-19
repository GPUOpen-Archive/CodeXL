//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdThumbnailView.h
///
//==================================================================================

//------------------------------ gdThumbnailView.h ------------------------------

// Qt:
#include <QtWidgets>

#ifndef __GDTHUMBNAILVIEW
#define __GDTHUMBNAILVIEW


// Infra:
#include <AMDTApplicationComponents/Include/acImageManager.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeData.h>


// Pre-declarations:
class afApplicationTreeItemData;
class apCLImage;
class apGLRenderBuffer;
class apGLVBO;
class apCLBuffer;
class acImageManager;
class gdDebugApplicationTreeHandler;
class afProgressBarWrapper;
// ----------------------------------------------------------------------------------
// Class Name:           gdThumbnailView: public acFrame
// General Description:  View displaying thumbnail objects.
// ----------------------------------------------------------------------------------
class GD_API gdThumbnailView : public acImageManager, public afBaseView
{
    Q_OBJECT

public:
    // Constructor
    gdThumbnailView(QWidget* pParent, afProgressBarWrapper* pProgressBar, gdDebugApplicationTreeHandler* pObjectsTree);

    // Destructor
    ~gdThumbnailView();

    void clearView();

    // Display item:
    bool displayThumbnailItem(afApplicationTreeItemData* pItemData);

    // Set the currently displayed item:
    void setDisplayedItemID(const afApplicationTreeItemData& displayedItemId) {_displayedItemId = displayedItemId;}
    const afApplicationTreeItemData& displayedItemID() {return _displayedItemId;}

    // Layout:
    bool setFrameLayout(const QSize& viewSize);

    // Set my is in begin-end block flag:
    void setIsInGLBeginEndBlock(bool isInGLBeginEndBlock) {_isInGLBeginEndBlock = isInGLBeginEndBlock;};

    // Apply the last viewed item properties:
    void applyLastViewedItemProperties(const acDisplayedItemProperties& lastViewedItemProperties);

    // Currently displayed item data:
    afApplicationTreeItemData* displayedItemData() const {return _pDisplayedItemData;}

    // Display the item as text if that's what needed:
    void displayTextMessageIfNeeded();

    // Process run / suspension events:
    void onProcessNotSuspended();
    void updateObjectDisplay();

protected slots:

    void onImageItemEvent(acImageItemID imageItemID, const QPoint& posOnImage, bool mouseLeftDown, bool mouseDoubleClick);

protected:

    // Add objects to thumbnail view functions:
    bool addTexturesToThumbnailView(const afApplicationTreeItemData* pThumbnailItemData);
    bool addStaticBuffersToThumbnailView(const afApplicationTreeItemData* pThumbnailItemData);
    bool addPBufferContentToThumbnailView(const afApplicationTreeItemData* pThumbnailItemData);
    bool addPBuffersToThumbnailView(const afApplicationTreeItemData* pThumbnailItemData);
    bool addRenderBufferToThumbnailView(const afApplicationTreeItemData* pThumbnailItemData);
    bool addFBOAttachmentsToThumbnailView(const afApplicationTreeItemData* pThumbnailItemData);
    bool addAllFBOsAttachmentToThumbnailView(const afApplicationTreeItemData* pThumbnailItemData);
    bool addVBOsToThumbnailView(const afApplicationTreeItemData* pThumbnailItemData);
    bool addCLBuffersToThumbnailView(const afApplicationTreeItemData* pThumbnailItemData);

    bool addSingleOpenGLTextureThumbnail(afApplicationTreeItemData* pTextureItemData);
    bool addSingleOpenCLImageThumbnail(afApplicationTreeItemData* pTextureItemData);

    // Thumbnail labels:
    void generateTextureThumbnailLabel(const apGLTextureMiplevelData& textureThumbnailDetails, gtString& botttomLabel, const apContextID& contextId);
    void generateCLImageThumbnailLabel(const apCLImage& textureDetails, gtString& botttomLabel);
    void generateGLTextureTopLabel(GLuint textureName, gtString& topLabel);
    void generateCLImageTopLabel(const apCLImage& textureDetails, gtString& topLabel);
    void generateRenderBufferThumbnailLabel(const apGLRenderBuffer& renderBufferDetails, gtString& bottomLabel);
    void generateCLBufferThumbnailLabel(const apCLBuffer& bufferDetails, gtString& bottomLabel);
    void generateVBOThumbnailLabel(const apGLVBO& vboDetails, gtString& bottomLabel, const apContextID& contextId);


protected:

    // The currently displayed item data:
    afApplicationTreeItemData* _pDisplayedItemData;

    // Contain only data identifying the object:
    afApplicationTreeItemData _displayedItemId;

    // Flags that we are in a glBegin - glEnd block
    bool _isInGLBeginEndBlock;

};

#endif  // __GDTHUMBNAILVIEW
