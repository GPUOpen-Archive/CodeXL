//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdImageAndBufferView.h
///
//==================================================================================

//------------------------------ gdImageAndBufferView.h ----------------------------

#ifndef __GDIMAGEANDBUFFERVIEW
#define __GDIMAGEANDBUFFERVIEW

// Qt:
#include <QtWidgets>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImageDataView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImagesAndBuffersExporter.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeData.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeHandler.h>



// ----------------------------------------------------------------------------------
// Class Name:           gdThumbnailView: public acFrame
// General Description:  View displaying thumbnail objects.
// ----------------------------------------------------------------------------------
class GD_API gdImageAndBufferView : public gdImageDataView
{
    Q_OBJECT

    friend class gdImagesAndBuffersControlPanel;
public:
    // Constructor
    gdImageAndBufferView(QWidget* pParent, afProgressBarWrapper* pProgressBar,  gdDebugApplicationTreeHandler* pObjectsTree);

    // Destructor
    ~gdImageAndBufferView();

    // Overrides apIEventsObserver:
    virtual const wchar_t* eventObserverName() const { return L"ImageAndBuffersView"; };

    // Base class override:
    virtual void displayCurrentTextureMiplevel(int miplevel, bool forceReload = false);

    // display item:
    bool displayItem(afApplicationTreeItemData* pItemData);

    // Set & Get the currently displayed item:
    void setDisplayedItemID(const afApplicationTreeItemData& displayedItemId) {_displayedItemId = displayedItemId;}
    const afApplicationTreeItemData& displayedItemID() {return _displayedItemId;}

    // Set my is in begin-end block flag:
    void setIsInGLBeginEndBlock(bool isInGLBeginEndBlock) {_isInGLBeginEndBlock = isInGLBeginEndBlock;};

    // Process run / suspension events:
    void updateObjectDisplay(bool& doesObjectExist);


    // Textures:
    bool loadTextureFile(const osFilePath& filePath, const gtString& objectName, QPoint matrixInsertPos, int index, int indexStride);

    /// Load the currently displayed texture to the image manager
    /// \param newMipLevel for textures with mip levels
    /// \param textureLayer for 3d textures and texture arrays. the value will be treated as array index for texture array and 3d layer for 3d textures
    /// \return true for success
    bool LoadTexture(int newMipLevel);

    /// Update the heading for the currently displayed texture
    /// \param pGDData the displayed texture GD data
    /// \param newMipLevel the texture mip level
    virtual void UpdateTextureHeading(gdDebugApplicationTreeData* pGDData, int newMipLevel);

    bool loadTextureObject();
    bool loadTextureElements();
    bool loadTexBuffer();

    // GL Buffers:
    bool loadStaticBuffer();
    bool loadPBufferStaticBuffer();
    void alertStaticBuffersAvailable();
    bool loadRenderBuffer();
    bool loadFBOAttachmentBuffer();

    // VBOs:
    bool loadVBO();

    // CL buffers:
    bool loadCLBuffer();
    bool loadCLSubBuffer();

    // Generic buffer loading function:
    bool loadBufferFile(const osFilePath& bufferFile);
    bool loadBufferFile(const osFilePath& bufferFile, oaTexelDataFormat bufferDisplayFormat, int offset, GLsizei stride);

    // Calculate the matrix position of a texture element
    bool calcTextureElementMatrixPosAndLabel(apTextureType textureType, int elementIndex, QPoint& matrixPos, gtString& topLabel);

    // Set the object not loaded status:
    void setObjectNotLoadedStatus(bool isFBOBound = false);
    void initializeObjectNotLoadedStatus(afItemLoadStatusType itemStatus, afItemLoadFailureDescription itemStatusDescription);

protected slots:

    void onNoteBookPageChange(int currentPage) {gdImageDataView::onNoteBookPageChange(currentPage);};

protected:

    // Contain true iff we are in a glBegin - glEnd block:
    bool _isInGLBeginEndBlock;

    // Contain true iff we are in a kernel debugging session:
    bool _isInKernelDebugging;

    bool _isLastMipLevelFailed;


};

#endif  // __GDTHUMBNAILVIEW
