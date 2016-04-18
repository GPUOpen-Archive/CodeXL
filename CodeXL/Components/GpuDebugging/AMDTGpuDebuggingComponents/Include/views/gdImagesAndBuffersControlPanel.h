//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdImagesAndBuffersControlPanel.h
///
//==================================================================================

//------------------------------ gdImagesAndBuffersControlPanel.h ------------------------------

#ifndef __GDIMAGESANDBUFFERSCONTROLPANEL
#define __GDIMAGESANDBUFFERSCONTROLPANEL

// Qt:
#include <QtWidgets>

// Forward decelerations:
QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTAPIClasses/Include/apTextureType.h>
#include <AMDTApplicationComponents/Include/acColorSampleBox.h>
#include <AMDTApplicationComponents/Include/acDoubleSlider.h>
#include <AMDTApplicationComponents/Include/acRawFileHandler.h>
#include <AMDTApplicationComponents/Include/acImageItem.h>
#include <AMDTApplicationComponents/Include/acImageManager.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>


class gdImageDataView;
class afApplicationTreeItemData;

/**
 ***************************************************************************************************
 * \class Name:         GD_API gdImagesAndBuffersControlPanel: public QFrame
 * \brief Description:  This class represents a textures, buffers and multi-watch control panel
 * \author              salgrana
 * \date                11/1/2009
 ***************************************************************************************************
 */
class GD_API gdImagesAndBuffersControlPanel: public QFrame
{
    Q_OBJECT

public:
    friend class gdMultiWatchView;

    gdImagesAndBuffersControlPanel(gdImageDataView* pImageDataView);
    virtual ~gdImagesAndBuffersControlPanel();

    bool isDisplayedItemSuitableForDoubleSlider();
    void showControlPanels(bool showDoubleSlider, bool show3DSlider, bool showMipLevelsSlider, bool showMultiwatch);

    // Get the current combo box variable name:
    const QString selectedMultiWatchVariableName() const ;
    int selectedMultiWatchVariableIndex() const ;
    bool selectMultiWatchVariableName(const gtString& variableName, bool& isVariableExist) const ;

    // Clear functions:
    void clearPixelInformation(bool clearGlobalInfo, bool clearSelectedPixelInfo = false);
    void clearControlPanel(bool bClearSelectedPixelInfo = true);

    void updateControlPanel(acImageItemID canvasId, acImageItem* pImageItem, QPoint posOnImage, bool leftButtonDown, bool isHoveringImage);

    // Set the displayed raw file handler:
    void setupPanel(const afApplicationTreeItemData* pDisplayedItemData);

    void displayStaleObjectWarningMessage();

    void setDisplayedFileHandler(acRawFileHandler* pRawFileHandler) {m_pDisplayedRawFileHandler = pRawFileHandler;};

    // Additional controls display:
    void updateDoubleSliderDisplay();
    void updateBufferFormatControlsDisplay();

    void updateTextureSlidersDisplay(apTextureType textureType, int currentMiplevel, int minLevel, int maxLevel);
    void updateMultiVariablesControlsDisplay(const gtString& variableName);
    bool updateCurrentWorkItem();

    bool setDoubleSliderTitle();
    void initializeVariableNamesCombo(bool isInKernelDebugging);
    void enableMultiwatchControls(bool isEnabled);
    bool change3DActivePage(int newActivePage, bool normalize);
    bool setZCoordinate(int zCoord);
    void setComboBoxFocus(int selectedIndex);
    void setIsExpanded(bool isExpanded);
    void setPanelCaption(const gtASCIIString& caption);

    // Is used to debug the panel layout:
    void setWidgetBGForDebug(QWidget* pWidget, const QColor& c);

protected slots:

    void onShowHidePanelClick();
    void onBufferFormatChange(int currentIndex);
    void onBufferFormatGroupChange(int currentIndex);
    void onOffsetSpinValueChanged(int newValue);
    void onStrideSpinValueChanged(int newValue);
    void on3DSliderChange(int newValue);
    void onZCoordSliderChange(int newValue);
    void onMiplevelsSliderChange(int newValue);
    void onDoubleSliderPositionChanged(double leftValue, double rightValue);

protected:

    enum gdTextureAndBuffersInfoPanelBufferFormatGroup
    {
        GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_COLOR = 0,
        GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_FIRST = GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_COLOR,
        GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_VERTEX = 1,
        GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_NORMAL = 2,
        GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_TEXTURE = 3,
        GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_INDEX = 4,
        GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_INTERLEAVED = 5,
        GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_OPENCL_C = 6,
        GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_ALL = 7,
        GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_LAST = GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_ALL
    };


    gdTextureAndBuffersInfoPanelBufferFormatGroup getMatchingFormatGroup(oaTexelDataFormat bufferFormat);

    void setFrameLayout();

    // Information panel text generation functions:
    void infoPanelGenerateMousePosition(acImageItem* pImageItem, QPoint posOnImage, QString& strMousePos, bool isHoveringImage);
    void infoPanelGeneratePixelColour(acImageItem* pImageItem, QPoint posOnImage, QRgb& pixelColour, QString& strPixelColour, bool isHoveringImage);
    void infoPanelGenerateRawDataValue(acImageItem* pImageItem, QPoint posOnImage, int canvasID, QString& strRawDataValue, bool isHoveringImage);

    // Layout functions:
    void createTopPanelLayout();
    void createBottomPanelLayout();
    void createMultiWatchControlsLayout();
    void createPixelInformationLayout();

    void setWorkSizeControlsVisibility(bool isVisible);

    QLabel* createBlueForegroundLabel();

    void createBufferFormatLayout();
    void create3DSliderLayout();
    void createMiplevelLayout();
    void createDoubleSliderLayout();
    void initBufferFormatCombos();
    bool initializeBufferFormatSelection();
    gtASCIIString bufferFormatGroupToString(gdTextureAndBuffersInfoPanelBufferFormatGroup formatGroup);
    void initBufferFormatsComboBox(gdTextureAndBuffersInfoPanelBufferFormatGroup formatsGroup);
    void updateBufferFormat(int selectedBufferFormatIndex);
    bool updateBufferDisplayProperties(oaTexelDataFormat displayFormat, int displayOffset, gtSize_t displayStride);
    void updateWorkSizeControls();

    void setupControlsAfterExpansionChange();

private:
    /// Updates pixel position related UI on control panel
    /// \param pImageItem the image item hovered or clicked
    /// \param posOnImage - mouse coordinates on image
    /// \param leftButtonDown - true if clicked, false if hovered
    /// \param isHoveringImage - true if imageView, false if dataView
    void updatePositionUI(acImageItem* pImageItem, QPoint posOnImage, bool isLeftButtonDown, bool isHoveringImage);

    /// Updates Raw Value (hovered and selected) related UI on control panel
    /// \param pImageItem the image item hovered or clicked
    /// \param posOnImage - mouse coordinates on image
    /// \param leftButtonDown - true if clicked, false if hovered
    /// \param isHoveringImage - true if imageView, false if dataView
    void updateRawValueUI(acImageItemID canvasId, acImageItem* pImageItem, QPoint posOnImage, bool isLeftButtonDown, bool isHoveringImage);

    /// Updates Color (hovered and selected) related UI on control panel
    /// \param pImageItem the image item hovered or clicked
    /// \param posOnImage - mouse coordinates on image
    /// \param leftButtonDown - true if clicked, false if hovered
    /// \param isHoveringImage - true if imageView, false if dataView
    void updateColorUI(acImageItem* pImageItem, QPoint posOnImage, bool isLeftButtonDown, bool isHoveringImage);


    /// Sets output parameters to texture dimensions
    /// \param textureWidth receives texture width
    /// \param textureHeight receives texture height
    /// \param newValue texture mipmap value (default 0)
    /// \returns true on success false otherwise
    bool GetTextureDimensions(GLsizei& textureWidth, GLsizei& textureHeight, int newValue = 0);

protected:

    // Contain the owner image / data view:
    gdImageDataView* m_pImageDataView;

    // Contain the current item displayed data:
    const afApplicationTreeItemData* m_pDisplayedItemData;

    // Contain the displayed item raw file handler:
    acRawFileHandler* m_pDisplayedRawFileHandler;

    // Widget layout objects:
    QGridLayout* m_pMainLayout;
    QBoxLayout* m_pTopLayout;
    QWidget* m_pTopGroupBox;
    QBoxLayout* m_pBottomLayout;
    QWidget* m_pBottomGroupBox;

    // Show / Hide button:
    QPushButton* m_pShowHideButton;

    // Panel title lable:
    QLabel* m_pPanelCaption;

    // Current pixel controls:
    QGridLayout* m_pPixelInformationGridLayout;
    QLabel* m_pHoveredPixelPositionText;
    QLabel* m_pHoveredPixelValueText;
    QLabel* m_pHoveredPixelColorText;
    QLabel* m_pGlobalWorkOffsetText;
    QLabel* m_pGlobalWorkSizeValueText;
    QLabel* m_pLocalWorkSizeValueText;
    QLabel* m_pGlobalWorkOffsetLabel;
    QLabel* m_pGlobalWorkSizeLabel;
    QLabel* m_pLocalWorkSizeLabel;
    QLabel* m_pHoveredPositionLabel;
    QLabel* m_pSelectedPositionLabel;
    acColorSampleBox* m_pHoveredColorPreview;

    // Previous pixel controls:
    QLabel* m_pSelectedPixelPositionText;
    QLabel* m_pSelectedPixelValueText;
    QLabel* m_pSelectedPixelColorText;
    acColorSampleBox* m_pSelectedColorPreview;

    // Pixel information layout:
    QBoxLayout* m_pPixelInfoLayout;
    QWidget* m_pPixelInfoGroupBox;

    // 3D Texture controls:
    QSlider* m_p3DTextureSlider;
    QLabel* m_p3DSliderCaption;
    QGridLayout* m_p3DSliderLayout;
    QWidget* m_p3DSliderGroupBox;
    QLabel* m_p3DSliderMinLabel;
    QLabel* m_p3DSliderMaxLabel;

    // Double slider controls:
    acDoubleSlider* m_pDoubleSlider;
    QBoxLayout* m_pDoubleSliderLayout;
    QWidget* m_pDoubleSliderGroupBox;
    QLabel* m_pDoubleSliderTitle;

    // Miplevel controls:
    QGridLayout* m_pMipmapSliderLayout;
    QWidget* m_pMipmapSliderGroupBox;
    QSlider* m_pTextureMipLevelsSlider;
    QLabel* m_pMiplevelsSliderCaption;
    QLabel* m_pMipmapMinLabel;
    QLabel* m_pMipmapMaxLabel;

    // Buffer format controls:
    QWidget* m_pBufferFormatGroupBox;
    QGridLayout* m_pBufferFormatLayout;
    QComboBox* m_pBufferFormatsGroupsComboBox;
    QComboBox* m_pBufferFormatsComboBox;
    QSpinBox* m_pOffsetSpinControl;
    QSpinBox* m_pStrideSpinControl;
    int m_currentlyLoadedMiplevel;

    // Multi watch controls:
    QGridLayout* m_pMultiWatchLayout;
    QWidget* m_pMultiWatchGroupBox;
    QComboBox* m_pMultiWatchVariableNameCombo;
    gtMap<QString, int> m_multiWatchComboVarNameToIndexMap;
    QSlider* m_pMultiWatchZCoordinateSlider;
    QLabel* m_pMultiWatchSliderMinLabel;
    QLabel* m_pMultiWatchSliderMaxLabel;
    QLabel* m_pMultiWatchSliderCaption;
    QLabel* m_pMultiWatchVariableNameCaption;
    QLabel* m_pMultiWatchVariableTypeCaption;
    QLabel* m_pMultiWatchVariableTypeText;
    int m_currentDisplayedZCoord;

    // Contain true iff the control panel is expanded:
    bool m_isExpanded;
    QPixmap* m_pShowBitmap;
    QPixmap* m_pHideBitmap;

    // Text box for notifications:
    QLabel* m_pNotificationTextLine;

    // Contain the current texel formats displayed in the buffer format combo box:
    gtVector<oaTexelDataFormat> m_currentDisplayedTexelFormats;

    // Multiwatch data:
    int m_currentKernelGlobalWorkOffset[3];
    int m_currentKernelGlobalWorkSize[3];
    int m_currentKernelLocalWorkSize[3];
    int m_currentAmountOfWorkDimensions;

    // True iff we should force the operation of the Z coordinate slider:
    bool m_forceZCoordSlider;

    // True iff we are currently updating the combo boxes for the buffer formats:
    bool m_isUpdatingFormatCombos;

    // Contain the panel minimum width:
    int m_minWidth;
};

#endif  // __GDTEXTURESANDBUFFERSINFOPANEL

