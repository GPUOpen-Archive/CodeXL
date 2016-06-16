//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdImagesAndBuffersControlPanel.cpp
///
//==================================================================================

//------------------------------ gdImagesAndBuffersControlPanel.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>
#include <QColor>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTAPIClasses/Include/apDefaultTextureNames.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acDefinitions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImageDataView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImagesAndBuffersControlPanel.h>
#include <AMDTGpuDebuggingComponents/Include/res/icons/show_controlPanel.xpm>
#include <AMDTGpuDebuggingComponents/Include/res/icons/hide_controlPanel.xpm>

// Defines the color sample size (in pixels)
#define GD_IMAGES_AND_BUFFERS_COLOR_SAMPLE_SIZE 24
#define GD_IMAGES_AND_BUFFERS_SLIDER_WIDTH 300
#define GD_IMAGES_AND_BUFFERS_ITEMS_TOP_MARGIN 3
#define GD_IMAGES_AND_BUFFERS_ITEMS_GRID_VERTICAL_SPACING 8
#define GD_IMAGES_AND_BUFFERS_ITEMS_RIGHT_MARGIN 5

#define GD_STR_NotAvailableBlue "<p color=blue>N/A</p>"
#define GD_STR_textBlue "<p color=blue>%s</p>"


const int SLIDER_PAGE_STEP = 1;

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::gdImagesAndBuffersControlPanel
// Description: Constructor
// Arguments:   gdImageDataView* pImageDataView
// Author:      Sigal Algranaty
// Date:        24/6/2012
// ---------------------------------------------------------------------------
gdImagesAndBuffersControlPanel::gdImagesAndBuffersControlPanel(gdImageDataView* pImageDataView)
    : QFrame(pImageDataView),
      m_pImageDataView(pImageDataView), m_pDisplayedItemData(nullptr), m_pDisplayedRawFileHandler(nullptr), m_pMainLayout(nullptr),
      m_pTopLayout(nullptr), m_pTopGroupBox(nullptr), m_pBottomLayout(nullptr), m_pBottomGroupBox(nullptr), m_pShowHideButton(nullptr), m_pPanelCaption(nullptr),
      m_pPixelInformationGridLayout(nullptr), m_pHoveredPixelPositionText(nullptr), m_pHoveredPixelValueText(nullptr), m_pHoveredPixelColorText(nullptr), m_pGlobalWorkOffsetText(nullptr),
      m_pGlobalWorkSizeValueText(nullptr), m_pLocalWorkSizeValueText(nullptr), m_pGlobalWorkOffsetLabel(nullptr), m_pGlobalWorkSizeLabel(nullptr),
      m_pLocalWorkSizeLabel(nullptr), m_pHoveredPositionLabel(nullptr), m_pSelectedPositionLabel(nullptr), m_pHoveredColorPreview(nullptr),
      m_pSelectedPixelPositionText(nullptr), m_pSelectedPixelValueText(nullptr), m_pSelectedPixelColorText(nullptr), m_pSelectedColorPreview(nullptr),
      m_pPixelInfoLayout(nullptr), m_pPixelInfoGroupBox(nullptr), m_p3DTextureSlider(nullptr), m_p3DSliderCaption(nullptr), m_p3DSliderLayout(nullptr), m_p3DSliderGroupBox(nullptr), m_p3DSliderMinLabel(nullptr), m_p3DSliderMaxLabel(nullptr),
      m_pDoubleSlider(nullptr), m_pDoubleSliderLayout(nullptr), m_pDoubleSliderGroupBox(nullptr), m_pDoubleSliderTitle(nullptr), m_pMipmapSliderLayout(nullptr), m_pMipmapSliderGroupBox(nullptr), m_pTextureMipLevelsSlider(nullptr), m_pMiplevelsSliderCaption(nullptr),
      m_pBufferFormatGroupBox(nullptr), m_pBufferFormatLayout(nullptr), m_pBufferFormatsGroupsComboBox(nullptr), m_pBufferFormatsComboBox(nullptr), m_pOffsetSpinControl(nullptr),
      m_pStrideSpinControl(nullptr), m_currentlyLoadedMiplevel(-1), m_pMultiWatchLayout(nullptr), m_pMultiWatchGroupBox(nullptr), m_pMultiWatchVariableNameCombo(nullptr),
      m_pMultiWatchZCoordinateSlider(nullptr), m_pMultiWatchSliderMinLabel(nullptr), m_pMultiWatchSliderMaxLabel(nullptr),
      m_pMultiWatchSliderCaption(nullptr), m_pMultiWatchVariableNameCaption(nullptr), m_pMultiWatchVariableTypeCaption(nullptr),
      m_pMultiWatchVariableTypeText(nullptr),
      m_currentDisplayedZCoord(0), m_isExpanded(true), m_pShowBitmap(nullptr), m_pHideBitmap(nullptr), m_pNotificationTextLine(nullptr),
      m_currentAmountOfWorkDimensions(0), m_forceZCoordSlider(false), m_isUpdatingFormatCombos(false), m_minWidth(0)
{
    // Reset kernel debugging data:
    m_currentKernelGlobalWorkOffset[0] = 0;
    m_currentKernelGlobalWorkOffset[1] = 0;
    m_currentKernelGlobalWorkOffset[2] = 0;
    m_currentKernelGlobalWorkSize[0] = 0;
    m_currentKernelGlobalWorkSize[1] = 0;
    m_currentKernelGlobalWorkSize[2] = 0;
    m_currentKernelLocalWorkSize[0] = 0;
    m_currentKernelLocalWorkSize[1] = 0;
    m_currentKernelLocalWorkSize[2] = 0;

    // Create the information panel GUI elements:
    setFrameLayout();
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::~gdImagesAndBuffersControlPanel
// Description: Destructor
// Return Val:
// Author:      Sigal Algranaty
// Date:        11/1/2009
// ---------------------------------------------------------------------------
gdImagesAndBuffersControlPanel::~gdImagesAndBuffersControlPanel()
{
    if (m_pHoveredColorPreview != nullptr)
    {
        delete m_pHoveredColorPreview;
    }

    if (m_pSelectedColorPreview != nullptr)
    {
        delete m_pSelectedColorPreview;
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::setFrameLayout
// Description:
// Return Val: void
// Author:      Sigal Algranaty
// Date:        11/1/2009
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::setFrameLayout()
{
    // Initialize the horizontal main layout:
    m_pMainLayout = new QGridLayout;


    // Create both bitmaps for the show / hide button:
    m_pShowBitmap = new QPixmap(show_controlPanel_xpm);


    m_pHideBitmap = new QPixmap(hide_controlPanel_xpm);


    // Create the show / hide button:
    QSize buttonSize(8, 8);
    m_pShowHideButton = new QPushButton;

    m_pShowHideButton->setFlat(true);

    // Set initial size:
    m_pShowHideButton->setIcon(QIcon(*m_pHideBitmap));
    m_pShowHideButton->resize(buttonSize);

    // Connect to slot:
    bool rc = connect(m_pShowHideButton, SIGNAL(clicked()), this, SLOT(onShowHidePanelClick()));
    GT_ASSERT(rc);

    // Create the top panel layout:
    createTopPanelLayout();

    // Create the bottom panel layout:
    createBottomPanelLayout();

    // Add both the panels to the right layout:
    m_pMainLayout->addWidget(m_pShowHideButton, 0, 0, 2, 1, Qt::AlignTop);
    m_pMainLayout->addWidget(m_pTopGroupBox, 0, 1, 1, 1, Qt::AlignTop);
    m_pMainLayout->addWidget(m_pBottomGroupBox, 1, 1, 1, 1, Qt::AlignTop);
    m_pMainLayout->setContentsMargins(0, GD_IMAGES_AND_BUFFERS_ITEMS_TOP_MARGIN, GD_IMAGES_AND_BUFFERS_ITEMS_RIGHT_MARGIN, 0);
    m_pMainLayout->setHorizontalSpacing(0);
    m_pMainLayout->setVerticalSpacing(0);

    m_pMainLayout->setColumnStretch(0, 1);
    m_pMainLayout->setColumnStretch(1, 99);

    m_pMainLayout->setSizeConstraint(QLayout::SetFixedSize);

    GT_IF_WITH_ASSERT((m_pTopGroupBox != nullptr) && (m_pBottomGroupBox != nullptr) && (m_pMainLayout != nullptr))
    {
        // Calculate the minimum width for the control panel top and bottom layouts:

        // Set the size of the selected and hovered value labels:
        QString testValuesStr;
        testValuesStr.sprintf("R: %u, G: %u, B: %u, A: %u", 255, 255, 255, 255);
        int textWidth = QFontMetrics(m_pHoveredPixelValueText->font()).boundingRect(testValuesStr).width() + 3;
        int labelWidth = QFontMetrics(m_pHoveredPixelValueText->font()).boundingRect(GD_STR_ImagesAndBuffersViewerControlPanelHoveredPositionCaption).width() + 3;
        m_minWidth = textWidth + labelWidth + GD_IMAGES_AND_BUFFERS_COLOR_SAMPLE_SIZE;
        m_pMainLayout->setColumnMinimumWidth(1, m_minWidth);
    }

    // Set the main layout:
    setLayout(m_pMainLayout);
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::setupPanel
// Description: Sets up the panel for the requested displayed file handler
// Author:      Sigal Algranaty
// Date:        27/2/2011
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::setupPanel(const afApplicationTreeItemData* pDisplayedItemData)
{
    // Sanity check;
    GT_IF_WITH_ASSERT(pDisplayedItemData != nullptr)
    {
        // Set the displayed data:
        m_pDisplayedItemData = pDisplayedItemData;
        gdDebugApplicationTreeData* pGDDisplayedItemData = qobject_cast<gdDebugApplicationTreeData*>(pDisplayedItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDDisplayedItemData != nullptr)
        {
            bool showMipLevelSlider = false, show3DSlider = false, showDoubleSlider = false, showMultiwatch = false;

            // Hide by default the buffer format group box:
            m_pBufferFormatGroupBox->hide();

            if (m_pDisplayedRawFileHandler != nullptr)
            {
                // Check if raw data is suitable for double slider
                showDoubleSlider = isDisplayedItemSuitableForDoubleSlider();

                // Show the double slider when valid:
                updateDoubleSliderDisplay();
            }

            showMultiwatch = (pDisplayedItemData->m_itemType == AF_TREE_ITEM_CL_KERNEL_VARIABLE);
            bool isTexture = ((pDisplayedItemData->m_itemType == AF_TREE_ITEM_GL_TEXTURE) || (pDisplayedItemData->m_itemType == AF_TREE_ITEM_CL_IMAGE));

            if (isTexture)
            {
                // Show mipmap levels slider only for textures with mipmap levels:
                showMipLevelSlider = ((pGDDisplayedItemData->_maxLevel - pGDDisplayedItemData->_minLevel) > 0);

                if (m_pDisplayedRawFileHandler != nullptr)
                {
                    // We should 3d slider only for textures with several pages (more than one page):
                    show3DSlider = m_pDisplayedRawFileHandler->amountOfExternalPages() > 1;
                }
            }

            // Show the control panel sliders:
            showControlPanels(showDoubleSlider, show3DSlider, showMipLevelSlider, showMultiwatch);

            if (isTexture)
            {
                // Update the textures sliders:
                updateTextureSlidersDisplay(pGDDisplayedItemData->_textureType, pGDDisplayedItemData->_textureMiplevelID._textureMipLevel, pGDDisplayedItemData->_minLevel, pGDDisplayedItemData->_maxLevel);
            }
            else if (showMultiwatch)
            {
                // Update the multi variable controls:
                updateMultiVariablesControlsDisplay(pGDDisplayedItemData->_multiVariableName);
            }
            else
            {
                // Update the buffer format controls display:
                updateBufferFormatControlsDisplay();
            }


            if (m_pDisplayedRawFileHandler != nullptr)
            {
                GT_IF_WITH_ASSERT(m_pImageDataView != nullptr)
                {
                    acDataView* pDataViewManager = m_pImageDataView->dataView();
                    GT_IF_WITH_ASSERT(pDataViewManager != nullptr)
                    {
                        // Check if the data format can be displayed as hexadecimal:
                        bool isHexEnabled = oaCanTypeBeDisplayedAsHex(m_pDisplayedRawFileHandler->dataFormat());

                        // Enable / disable the hexadecimal format:
                        pDataViewManager->enableHexCheckbox(isHexEnabled);
                    }
                }
            }

            // Show / Hide the object stale warning:
            displayStaleObjectWarningMessage();

            // Update the window layout:
            updateGeometry();
        }
    }
};

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::clearCurrentPixelInformation
// Description: Clears the current clicked pixel information
// Arguments:   clearGlobalInfo - true iff global information should also be cleared
// Author:      Sigal Algranaty
// Date:        22/3/2011
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::clearPixelInformation(bool clearGlobalInfo, bool clearSelectedInfo)
{
    GT_IF_WITH_ASSERT((m_pHoveredPixelPositionText != nullptr) && (m_pHoveredPixelColorText != nullptr) &&
                      (m_pHoveredPixelValueText != nullptr) && (m_pSelectedPixelColorText != nullptr) &&
                      (m_pSelectedPixelPositionText != nullptr) && (m_pSelectedPixelValueText != nullptr) &&
                      (m_pGlobalWorkSizeValueText != nullptr) && (m_pLocalWorkSizeValueText != nullptr) &&
                      (m_pGlobalWorkOffsetText != nullptr) && (m_pMultiWatchVariableTypeText != nullptr))
    {
        if (clearGlobalInfo)
        {
            // Clear the text boxes:
            m_pGlobalWorkOffsetText->setText(GD_STR_NotAvailableBlue);
            m_pGlobalWorkSizeValueText->setText(GD_STR_NotAvailableBlue);
            m_pLocalWorkSizeValueText->setText(GD_STR_NotAvailableBlue);
            m_pMultiWatchVariableTypeText->setText(GD_STR_NotAvailableBlue);
        }

        if (clearSelectedInfo)
        {
            m_pSelectedPixelPositionText->setText(GD_STR_NotAvailableBlue);
            m_pSelectedPixelColorText->setText(GD_STR_NotAvailableBlue);
            m_pSelectedPixelValueText->setText(GD_STR_NotAvailableBlue);
        }

        m_pHoveredPixelPositionText->setText(GD_STR_NotAvailableBlue);
        m_pHoveredPixelColorText->setText(GD_STR_NotAvailableBlue);
        m_pHoveredPixelValueText->setText(GD_STR_NotAvailableBlue);
    }

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSelectedColorPreview != nullptr) && (m_pHoveredColorPreview != nullptr))
    {
        if (clearSelectedInfo)
        {
            // Clear the color samples:
            m_pSelectedColorPreview->clearColourSample();
            m_pSelectedColorPreview->repaint(m_pSelectedColorPreview->rect());
        }

        // Clear the current color
        m_pHoveredColorPreview->clearColourSample();
        m_pHoveredColorPreview->repaint(m_pHoveredColorPreview->rect());
    }

    GT_IF_WITH_ASSERT(m_pHoveredColorPreview != nullptr)
    {
        // Clear the current color
        m_pHoveredColorPreview->clearColourSample();
        m_pHoveredColorPreview->repaint(m_pHoveredColorPreview->rect());
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::updatePixelPosition
// Description: Updates the pixel position on control panel
// Arguments:   pImageItem - The image item which have the mouse focus
//              posOnImage - Mouse position on image
//              leftButtonDown - is mouse left button down or not
//              isHoveringImage - true if hovering/clicking image view, false if on data view
// Author:      Yuri Rshtunique
// Date:        December 15, 2014
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::updatePositionUI(acImageItem* pImageItem, QPoint posOnImage, bool isLeftButtonDown, bool isHoveringImage)
{
    QLabel* pPosTextCtrlToUpdate = nullptr;

    QString strMousePosition;

    if (pImageItem != nullptr)
    {
        infoPanelGenerateMousePosition(pImageItem, posOnImage, strMousePosition, isHoveringImage);
    }
    else
    {
        strMousePosition = AF_STR_NotAvailableA;
    }

    if (!isLeftButtonDown)
    {
        pPosTextCtrlToUpdate = m_pHoveredPixelPositionText;
    }
    else
    {
        pPosTextCtrlToUpdate = m_pSelectedPixelPositionText;
    }

    // Update the text control
    GT_IF_WITH_ASSERT(pPosTextCtrlToUpdate != nullptr)
    {
        // Update the position of the pixel:
        QString text;
        text.sprintf(GD_STR_textBlue, strMousePosition.toLatin1().data());
        pPosTextCtrlToUpdate->setText(text);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::updateRawValueUI
// Description: Updates the pixel position on control panel
// Arguments:   pImageItem - The image item which have the mouse focus
//              posOnImage - Mouse position on image
//              leftButtonDown - is mouse left button down or not
//              isHoveringImage - true if hovering/clicking image view, false if on data view
// Author:      Yuri Rshtunique
// Date:        December 15, 2014
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::updateRawValueUI(acImageItemID canvasId, acImageItem* pImageItem, QPoint posOnImage, bool isLeftButtonDown, bool isHoveringImage)
{
    QString strRawDataValue;
    QLabel* pValueTextCtrlToUpdate = nullptr;

    // Generate raw data value text
    if (pImageItem != nullptr)
    {
        infoPanelGenerateRawDataValue(pImageItem, posOnImage, canvasId, strRawDataValue, isHoveringImage);
    }
    else
    {
        strRawDataValue = AF_STR_NotAvailableA;
    }

    // If mouse clicked
    if (isLeftButtonDown)
    {
        pValueTextCtrlToUpdate = m_pSelectedPixelValueText;
    }
    else
    {
        pValueTextCtrlToUpdate = m_pHoveredPixelValueText;
    }

    QString text;
    text.sprintf(GD_STR_textBlue, strRawDataValue.toLatin1().data());
    pValueTextCtrlToUpdate->setText(text);
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::updateColorUI
// Description: Updates the pixel position on control panel
// Arguments:   pImageItem - The image item which have the mouse focus
//              posOnImage - Mouse position on image
//              leftButtonDown - is mouse left button down or not
//              isHoveringImage - true if hovering/clicking image view, false if on data view
// Author:      Yuri Rshtunique
// Date:        December 15, 2014
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::updateColorUI(acImageItem* pImageItem, QPoint posOnImage, bool isLeftButtonDown, bool isHoveringImage)
{
    QString strPixelcolor;
    QRgb pixelcolor = qRgba(0, 0, 0, 0);

    if (pImageItem != nullptr)
    {
        infoPanelGeneratePixelColour(pImageItem, posOnImage, pixelcolor, strPixelcolor, isHoveringImage);
    }
    else
    {
        strPixelcolor = AF_STR_NotAvailableA;
    }

    QLabel* pColorTextCtrlToUpdate = nullptr;

    // If this event occurred because of mouse movement and not a left click, update the current pixel color
    if (isLeftButtonDown)
    {
        // Update selected pixel color
        m_pSelectedColorPreview->setColourSample(pixelcolor);
        pColorTextCtrlToUpdate = m_pSelectedPixelColorText;
    }
    else
    {
        // Update hovered pixel color
        pColorTextCtrlToUpdate = m_pHoveredPixelColorText;
        m_pHoveredColorPreview->setColourSample(pixelcolor);
    }

    QString text;
    text.sprintf(GD_STR_textBlue, strPixelcolor.toLatin1().data());
    pColorTextCtrlToUpdate->setText(text);
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::updateControlPanel
// Description: Updates the image control panel
// Arguments:   canvasId - The canvas ID of the item
//              pImageItem - The image item which have the mouse focus
//              posOnImage - Mouse position on image
//              leftButtonDown - is mouse left button down or not?
// Author:      Eran Zinman
// Date:        20/6/2007
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::updateControlPanel(acImageItemID canvasId, acImageItem* pImageItem, QPoint posOnImage, bool leftButtonDown, bool isHoveringImage)
{
    updatePositionUI(pImageItem, posOnImage, leftButtonDown, isHoveringImage);
    updateRawValueUI(canvasId, pImageItem, posOnImage, leftButtonDown, isHoveringImage);
    updateColorUI(pImageItem, posOnImage, leftButtonDown, isHoveringImage);
    layout()->activate();
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::setDoubleSliderTitle
// Description: Sets the double slider title
// Author:      Eran Zinman
// Date:        29/12/2007
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersControlPanel::setDoubleSliderTitle()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pDoubleSliderTitle != nullptr) && (m_pDisplayedRawFileHandler != nullptr))
    {
        QString strSliderTitle;

        // Get texel data format and data type:
        oaTexelDataFormat texelFormat = m_pDisplayedRawFileHandler->dataFormat();

        if (texelFormat == OA_TEXEL_FORMAT_VARIABLE_VALUE)
        {
            strSliderTitle = GD_STR_MultiWatchDoubleSliderLabel;
        }
        else
        {
            // Get primary channel name:
            oaTexelDataFormat primaryChannelFormat = oaGetTexelFormatComponentType(texelFormat, 0);
            GT_IF_WITH_ASSERT(primaryChannelFormat != OA_TEXEL_FORMAT_UNKNOWN)
            {

                gtString strChannelName;
                bool rc3 = oaGetTexelDataFormatName(primaryChannelFormat, strChannelName);
                GT_IF_WITH_ASSERT(rc3)
                {
                    strSliderTitle = QString(GD_STR_ImagesAndBuffersViewerDoubleSliderLabel).arg(acGTStringToQString(strChannelName));
                }
            }
        }

        // Set the new text value:
        m_pDoubleSliderTitle->setText(strSliderTitle);

        retVal = true;

    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::onMiplevelsSliderChange
// Description: Called when the texture mip levels slider is changed
// Arguments:   event - The scroll event details
// Author:      Eran Zinman
// Date:        11/1/2009
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::onMiplevelsSliderChange(int newValue)
{
    // Get the current slider value, which is the required new mip level:
    int currentlyLoadedMiplevel = newValue;

    // Check if not processing the current mip level load:
    if (currentlyLoadedMiplevel != m_currentlyLoadedMiplevel)
    {
        // Save the last viewed item properties (are used later when adjusting the 3d texture level):
        int currentZoom = 0;
        m_pImageDataView->saveLastViewedItemProperties(currentZoom);

        // Adjust the shown 3d texture level to the new miplevels scale:
        m_pImageDataView->adjust3dTextureLevel(currentlyLoadedMiplevel, m_currentlyLoadedMiplevel);//parameters order is wrong - should be (old, new)

        // Set the currently loaded mip level:
        m_currentlyLoadedMiplevel = currentlyLoadedMiplevel;

        // Clear the image buffer viewer:
        m_pImageDataView->clearView();

        // Get the mip level raw data from texture viewer (force the reload of the texture image):
        m_pImageDataView->displayCurrentTextureMiplevel(m_currentlyLoadedMiplevel, true);

        // Update the min / max and current mipmap levels of the texture:
        GT_IF_WITH_ASSERT((m_pMipmapMinLabel != nullptr) && (m_pMipmapMaxLabel != nullptr) && (m_pTextureMipLevelsSlider != nullptr) && (m_pMiplevelsSliderCaption != nullptr))
        {
            gtASCIIString minLabel, maxLabel, currentLabel;
            minLabel.appendFormattedString("%d", m_pTextureMipLevelsSlider->minimum());
            maxLabel.appendFormattedString("%d", m_pTextureMipLevelsSlider->maximum());
            currentLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewerCurrentMiplevelSliderCaption, newValue);
            m_pMipmapMinLabel->setText(minLabel.asCharArray());
            m_pMipmapMaxLabel->setText(maxLabel.asCharArray());
            GLsizei textureWidth = -1, textureHeight = -1;

            if (GetTextureDimensions(textureWidth, textureHeight, newValue))
            {
                currentLabel.appendFormattedString(" (%d x %d)", textureWidth, textureHeight);
            }

            m_pMiplevelsSliderCaption->setText(currentLabel.asCharArray());
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::onZCoordSliderChange
// Description: Called when the Z coordinate slider is changed.
// Arguments:   int newValue
// Author:      Sigal Algranaty
// Date:        1/3/2011
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::onZCoordSliderChange(int newValue)
{
    (void)(newValue);  // unused
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pMultiWatchSliderMinLabel != nullptr) && (m_pMultiWatchSliderMaxLabel != nullptr) && (m_pMultiWatchZCoordinateSlider != nullptr) && (m_pMultiWatchSliderCaption != nullptr))
    {
        // If there is a display item:
        if (m_pDisplayedRawFileHandler != nullptr)
        {
            // Get the currently selected 3D page:
            int newActivePage = m_pMultiWatchZCoordinateSlider->value();

            // Reduce the Z coordinate from the active page:
            newActivePage -= m_currentKernelGlobalWorkOffset[2];

            // Perform the Z coordinate update actions, only when the page had changed since last update,
            // unless we should currently force the actions:
            if ((newActivePage != m_pDisplayedRawFileHandler->activePage()) || m_forceZCoordSlider)
            {
                // Switch to the new active page, and normalize the page values:
                bool rcChangePage = change3DActivePage(newActivePage, true);
                GT_ASSERT(rcChangePage);

                // Get the maximal and minimal values of the raw file:
                double minValue = 0;
                double maxValue = 0;
                m_pDisplayedRawFileHandler->getMinMaxValues(minValue, maxValue);

                // For multi watch variable - set the max and min values for the variable as the max / min possible values:
                m_pDoubleSlider->setSliderMinMaxPossibleValues(minValue, maxValue);

                // Try to set the new coordinate value:
                int newZCoordValue = newActivePage + m_currentKernelGlobalWorkOffset[2];
                bool rcSetWorkItem = gaSetKernelDebuggingCurrentWorkItemCoordinate(2, newZCoordValue);
                GT_ASSERT(rcSetWorkItem);

                // Update the min / max and current z coordinate:
                gtASCIIString minLabel, maxLabel, currentLabel;
                minLabel.appendFormattedString("%d", m_pMultiWatchZCoordinateSlider->minimum());
                maxLabel.appendFormattedString("%d", m_pMultiWatchZCoordinateSlider->maximum());

                currentLabel.appendFormattedString(GD_STR_MultiWatchSliderCaption, newZCoordValue);

                m_pMultiWatchSliderMinLabel->setText(minLabel.asCharArray());
                m_pMultiWatchSliderMaxLabel->setText(maxLabel.asCharArray());
                m_pMultiWatchSliderCaption->setText(currentLabel.asCharArray());

                // Update the work size control after the Z coordinate change:
                updateWorkSizeControls();

                // Get image view:
                GT_IF_WITH_ASSERT(nullptr != m_pImageDataView)
                {
                    acImageManager* pImageViewManager = m_pImageDataView->imageView();
                    GT_IF_WITH_ASSERT(nullptr != pImageViewManager)
                    {
                        // Refresh the image view:
                        pImageViewManager->forceImagesRepaint();
                    }
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::on3DSliderChange
// Description: Called when the 3D Image slider is changed
// Arguments:   event - The scroll event details
// Author:      Eran Zinman
// Date:        12/7/2007
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::on3DSliderChange(int newValue)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_p3DTextureSlider != nullptr)
    {
        // Get the currently selected 3D page:
        int newActivePage = m_p3DTextureSlider->value();

        // Switch to the new active page:
        bool rcChangePage = change3DActivePage(newActivePage, false);
        GT_ASSERT(rcChangePage);

        // Update the min / max and current mipmap levels of the texture:
        GT_IF_WITH_ASSERT((m_p3DSliderMinLabel != nullptr) && (m_p3DSliderMaxLabel != nullptr) && (m_p3DTextureSlider != nullptr) && (m_p3DSliderCaption != nullptr) && (m_pDisplayedItemData != nullptr))
        {
            gdDebugApplicationTreeData* pGDDisplayedItemData = qobject_cast<gdDebugApplicationTreeData*>(m_pDisplayedItemData->extendedItemData());
            GT_IF_WITH_ASSERT(pGDDisplayedItemData != nullptr)
            {
                gtASCIIString minLabel, maxLabel, currentLabel;
                minLabel.appendFormattedString("%d", m_p3DTextureSlider->minimum());
                maxLabel.appendFormattedString("%d", m_p3DTextureSlider->maximum());

                if ((pGDDisplayedItemData->_textureType == AP_2D_ARRAY_TEXTURE) || (pGDDisplayedItemData->_textureType == AP_1D_ARRAY_TEXTURE))
                {
                    currentLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewerArraySliderCaption, newValue);
                }
                else if (pGDDisplayedItemData->_contextId.isOpenGLContext())
                {
                    currentLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewer3DSliderCaptionGL, newValue);
                }
                else if (pGDDisplayedItemData->_contextId.isOpenCLContext())
                {
                    currentLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewer3DSliderCaptionCL, newValue);
                }

                m_p3DSliderMinLabel->setText(minLabel.asCharArray());
                m_p3DSliderMaxLabel->setText(maxLabel.asCharArray());
                m_p3DSliderCaption->setText(currentLabel.asCharArray());

                // Set the texture layer / index
                pGDDisplayedItemData->_textureLayer = newActivePage;

                // Update the displayed texture heading
                int miplevel = -1;

                if (m_pTextureMipLevelsSlider != nullptr && m_pTextureMipLevelsSlider->isVisible())
                {
                    miplevel = m_pTextureMipLevelsSlider->value();
                }

                m_pImageDataView->UpdateTextureHeading(pGDDisplayedItemData, miplevel);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::change3DActivePage
// Description: Changes the active page for a 3D data display
// Arguments:   int newActivePage - the active page index
//              bool normalize - should we normalize the values for the raw file
//              handler
// Author:      Sigal Algranaty
// Date:        1/3/2011
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersControlPanel::change3DActivePage(int newActivePage, bool normalize)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pImageDataView != nullptr)
    {
        // Sanity check:
        acDataView* pDataViewManager = m_pImageDataView->dataView();
        acImageManager* pImageViewManager = m_pImageDataView->imageView();

        GT_IF_WITH_ASSERT((pDataViewManager != nullptr) && (pImageViewManager != nullptr))
        {
            // Get amount of items in textures and buffers manager
            int amountOfItems = pImageViewManager->amountOfImagesInManager();

            // Loop through all the items and apply the 3d slider change
            for (int i = 0; i < amountOfItems; i++)
            {
                // Get manager object
                acImageItem* pManagerObject = pImageViewManager->getItem(i);

                if (pManagerObject)
                {
                    // Get the raw data from the texture data viewer
                    acDataViewItem* pTextureRawData = pDataViewManager->getDataItemByCanvasID(i);
                    GT_IF_WITH_ASSERT(pTextureRawData != nullptr)
                    {
                        // Get raw data handler
                        acRawFileHandler* pRawDataHandler = pTextureRawData->getRawDataHandler();
                        GT_IF_WITH_ASSERT(pRawDataHandler != nullptr)
                        {
                            // Is it a multi page object?
                            if (pRawDataHandler->amountOfExternalPages() > 1)
                            {
                                retVal = true;

                                // Was the page changed?
                                int prevActivePage = pRawDataHandler->activePage();

                                if (prevActivePage != newActivePage)
                                {
                                    // Set active page
                                    bool rc = pRawDataHandler->setActivePage(newActivePage);
                                    GT_IF_WITH_ASSERT(rc)
                                    {
                                        if (normalize)
                                        {
                                            // Get the file data format:
                                            oaTexelDataFormat dataFormat = m_pDisplayedRawFileHandler->dataFormat();
                                            pRawDataHandler->normalizeValues(dataFormat, true);
                                        }

                                        // Convert the raw data to a free image object:
                                        QImage* pImage = pRawDataHandler->convertToQImage();
                                        GT_IF_WITH_ASSERT(pImage != nullptr)
                                        {
                                            // Replace the image in the image manager:
                                            retVal = pImageViewManager->replaceImageBitmap(i, pImage);

                                            int zoomLevel = m_pImageDataView->currentZoomLevel();
                                            m_pImageDataView->setTextureManagerZoomLevel(zoomLevel);
                                        }

                                        // Update the image and data views
                                        pImageViewManager->update();
                                        pDataViewManager->update();

                                        pImageViewManager->forceImagesRepaint();
                                    }
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
// Name:        gdImagesAndBuffersControlPanel::onDoubleSliderPositionChanged
// Description: Is called when the double slider position was changed
// Arguments:   event - A class representing the slider position changed event.
// Author:      Eran Zinman
// Date:        8/11/2007
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::onDoubleSliderPositionChanged(double minValue, double maxValue)
{
    // Get the data and image views:
    acDataView* pDataViewManager = m_pImageDataView->dataView();
    acImageManager* pImageViewManager = m_pImageDataView->imageView();

    GT_IF_WITH_ASSERT(pDataViewManager && pImageViewManager)
    {
        // Get amount of items in textures and buffers manager
        int amountOfItems = pImageViewManager->amountOfImagesInManager();

        // Loop through all the items and apply the double slider change
        for (int i = 0; i < amountOfItems; i++)
        {
            acImageItem* pManagerObject = pImageViewManager->getItem(i);

            if (pManagerObject)
            {
                // Get the raw data from the texture data viewer
                acDataViewItem* pTextureRawData = pDataViewManager->getDataItemByCanvasID(i);
                GT_IF_WITH_ASSERT(pTextureRawData != nullptr)
                {
                    // Get raw data handler
                    acRawFileHandler* pRawDataHandler = pTextureRawData->getRawDataHandler();
                    GT_IF_WITH_ASSERT(pRawDataHandler != nullptr)
                    {
                        // Set new max and min values
                        bool rc = pRawDataHandler->setMinMaxValues(minValue, maxValue);
                        GT_IF_WITH_ASSERT(rc)
                        {
                            // Convert new image to QImage format
                            QImage* pImage = pRawDataHandler->convertToQImage();
                            GT_IF_WITH_ASSERT(pImage != nullptr)
                            {
                                // Replace the bitmap with the new bitmap
                                rc = pImageViewManager->replaceImageBitmap(i, pImage);
                                GT_ASSERT(rc);

                                int zoomLevel = m_pImageDataView->currentZoomLevel();
                                m_pImageDataView->setTextureManagerZoomLevel(zoomLevel);
                            }
                        }
                    }
                }
            }
        }

        // Refresh the image and data view:
        pImageViewManager->forceImagesRepaint();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::isDisplayedItemSuitableForDoubleSlider
// Description: Check if the current displayed raw data is suitable to be used with double slider
// Author:      Eran Zinman
// Date:        30/11/2007
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersControlPanel::isDisplayedItemSuitableForDoubleSlider()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pDisplayedRawFileHandler != nullptr)
    {
        // Get the raw file handler data format:
        oaTexelDataFormat dataFormat = m_pDisplayedRawFileHandler->dataFormat();

        // We only support the following types:
        if ((dataFormat == OA_TEXEL_FORMAT_STENCIL) ||
            (dataFormat == OA_TEXEL_FORMAT_STENCIL_INDEX1_EXT) ||
            (dataFormat == OA_TEXEL_FORMAT_STENCIL_INDEX4_EXT) ||
            (dataFormat == OA_TEXEL_FORMAT_STENCIL_INDEX8_EXT) ||
            (dataFormat == OA_TEXEL_FORMAT_STENCIL_INDEX16_EXT) ||
            (dataFormat == OA_TEXEL_FORMAT_DEPTH) ||
            (dataFormat == OA_TEXEL_FORMAT_DEPTH_COMPONENT16) ||
            (dataFormat == OA_TEXEL_FORMAT_DEPTH_COMPONENT24) ||
            (dataFormat == OA_TEXEL_FORMAT_DEPTH_COMPONENT32) ||
            (dataFormat == OA_TEXEL_FORMAT_DEPTH_EXT) ||
            (dataFormat == OA_TEXEL_FORMAT_LUMINANCE) ||
            (dataFormat == OA_TEXEL_FORMAT_LUMINANCE_COMPRESSED) ||
            (dataFormat == OA_TEXEL_FORMAT_LUMINANCEALPHA) ||
            (dataFormat == OA_TEXEL_FORMAT_LUMINANCEALPHA_COMPRESSED) ||
            (dataFormat == OA_TEXEL_FORMAT_VARIABLE_VALUE))
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::showControlPanels
// Description: Show/Hide the information panel sliders
// Arguments: bool showDoubleSlider
//            bool show3DSlider
//            bool showMipLevelsSlider
// Return Val: void
// Author:      Sigal Algranaty
// Date:        11/1/2009
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::showControlPanels(bool showDoubleSlider, bool show3DSlider, bool showMipLevelsSlider, bool showMultiVariable)
{
    // Show hide the controls
    if ((m_pDoubleSliderGroupBox != nullptr) && (m_p3DSliderLayout != nullptr) && (m_pMipmapSliderLayout != nullptr) && (m_pBottomGroupBox != nullptr))
    {
        // Show / Hide the multi variable controls:
        m_pMultiWatchGroupBox->setVisible(showMultiVariable);

        // If one of the sliders are shown, show the bottom panel:
        bool showBottomPanel = false;

        if (m_isExpanded)
        {
            showBottomPanel = showMipLevelsSlider || showDoubleSlider || show3DSlider || showMultiVariable;

            m_pBottomGroupBox->setVisible(showBottomPanel);

            // Show / Hide the 3D Texture slider:
            m_p3DSliderGroupBox->setVisible(show3DSlider);

            // Show / Hide the double slider:
            m_pDoubleSliderGroupBox->setVisible(showDoubleSlider);

            // Show / Hide the mip levels slider:
            m_pMipmapSliderGroupBox->setVisible(showMipLevelsSlider);
        }

        // Update the window layout:
        updateGeometry();
    }
}


void gdImagesAndBuffersControlPanel::infoPanelGenerateMousePosition(acImageItem* pImageItem, QPoint posOnImage, QString& strMousePos, bool isHoveringImage)
{
    GT_IF_WITH_ASSERT((pImageItem != nullptr) && (m_pDisplayedRawFileHandler != nullptr) && (m_pImageDataView != nullptr))
    {
        // The position is an image position, we should translate to absolute position (consider zoom on image):
        QPoint absolutePosOnImage = posOnImage;
        bool bRes = false;

        if (nullptr != m_pImageDataView)
        {
            bRes = m_pImageDataView->zoomedImagePositionToImagePosition(pImageItem, posOnImage, absolutePosOnImage, isHoveringImage);
        }

        GT_IF_WITH_ASSERT(bRes)
        {
            QSize imageSize = pImageItem->originalImageSize();

            int mouseXPos = absolutePosOnImage.x();
            int mouseYPos = absolutePosOnImage.y();
            int mouseZPos = m_pDisplayedRawFileHandler->activePage();
            // Make sure that the mouse position is within image size range:
            bool rcValidPos = ((mouseXPos < imageSize.width()) && (mouseYPos < imageSize.height()));

            if (m_pDisplayedRawFileHandler->dataFormat() == OA_TEXEL_FORMAT_VARIABLE_VALUE)
            {
                // Update kernel variable coordinate with the global work offset:
                mouseXPos += m_currentKernelGlobalWorkOffset[0];
                mouseYPos += m_currentKernelGlobalWorkOffset[1];
                mouseZPos += m_currentKernelGlobalWorkOffset[2];
            }

            // Check that the position is positive:
            rcValidPos = rcValidPos && (mouseYPos >= 0) && (mouseXPos >= 0);

            // Check that we have valid (x, y) values:
            GT_IF_WITH_ASSERT(rcValidPos)
            {
                // Create output string:
                strMousePos.sprintf("X: %d, Y: %d", mouseXPos, mouseYPos);
            }

            if (m_pDisplayedRawFileHandler != nullptr)
            {
                if ((m_pDisplayedRawFileHandler->amountOfExternalPages() > 1) && (m_pDisplayedRawFileHandler->dataFormat() == OA_TEXEL_FORMAT_VARIABLE_VALUE))
                {
                    // Add Z coordinate:
                    QString zpos;
                    zpos.sprintf(", Z:%d", mouseZPos);
                    strMousePos.append(zpos);
                }
            }
        }
    }
}


void gdImagesAndBuffersControlPanel::infoPanelGeneratePixelColour(acImageItem* pImageItem, QPoint posOnImage, QRgb& pixelcolor, QString& strPixelcolor, bool isHoveringImage)
{
    GT_UNREFERENCED_PARAMETER(isHoveringImage);
    // Clear the string
    strPixelcolor.clear();

    GT_IF_WITH_ASSERT(pImageItem != nullptr)
    {
        // On "GrayScale" and "Invert" modes, show full RGBA:
        bool isGreyscaleChecked = pImageItem->isActionChecked(AC_IMAGE_GRAYSCALE_FILTER);
        bool isInvertChecked = pImageItem->isActionChecked(AC_IMAGE_INVERT_FILTER);
        bool fullRGBA = (isInvertChecked || isGreyscaleChecked);

        // Check color channel status
        bool redEnabled = pImageItem->isActionChecked(AC_IMAGE_CHANNEL_RED) || fullRGBA;
        bool greenEnabled = pImageItem->isActionChecked(AC_IMAGE_CHANNEL_GREEN) || fullRGBA;
        bool blueEnabled = pImageItem->isActionChecked(AC_IMAGE_CHANNEL_BLUE) || fullRGBA;
        bool alphaEnabled = pImageItem->isActionChecked(AC_IMAGE_CHANNEL_ALPHA) || fullRGBA;

        // Get pixel color and process any active channel filters
        QPoint p(posOnImage.x(), posOnImage.y());
        QPoint notZoomedPoint = p;
        float imageXPos = (float)posOnImage.x();
        float imageYPos = (float)posOnImage.y();

        if ((pImageItem->originalImageSize().width() > 0) && (pImageItem->originalImageSize().height() > 0))
        {
            float zoomedWidth = (float)(pImageItem->zoomedImageSize().width());
            float zoomedHight = (float)(pImageItem->zoomedImageSize().height());
            float origWidth = (float)(pImageItem->originalImageSize().width());
            float origHeight = (float)(pImageItem->originalImageSize().height());
            float zoomLevelX = zoomedWidth / origWidth;
            float zoomLevelY = zoomedHight / origHeight;
            float fX = -1.0;
            float fY = -1.0;
            fX = floor((imageXPos * zoomLevelX) + 0.5);
            fY = floor((imageYPos * zoomLevelY) + 0.5);

            if (fX >= zoomedWidth)
            {
                fX = zoomedWidth - 1;
            }

            if (fY >= zoomedHight)
            {
                fY = zoomedHight - 1;
            }

            notZoomedPoint.setX(fX);
            notZoomedPoint.setY(fY);
        }

        bool rc = pImageItem->getPixelColour(notZoomedPoint, pixelcolor);
        GT_IF_WITH_ASSERT(rc)
        {
            pImageItem->applyChannelsFilter(pixelcolor);

            // If any value was written before the current value, add a ","
            bool prevValueWritten = false;

            // Add red channel value if enabled
            if (redEnabled)
            {
                strPixelcolor.sprintf("R: %u", qRed(pixelcolor));
                prevValueWritten = true;
            }

            // Add green channel value if enabled
            if (greenEnabled)
            {
                // Should we add a "," before this value
                if (prevValueWritten)
                {
                    strPixelcolor.append(", ");
                }

                strPixelcolor.append("G: ");
                strPixelcolor.append(QString("%1").arg(qGreen(pixelcolor)));
                prevValueWritten = true;
            }

            // Add blue channel value if enabled
            if (blueEnabled)
            {
                // Should we add a "," before this value
                if (prevValueWritten)
                {
                    strPixelcolor.append(", ");
                }

                strPixelcolor.append("B: ");
                strPixelcolor.append(QString("%1").arg(qBlue(pixelcolor)));
                prevValueWritten = true;
            }

            // Add alpha channel value if enabled
            if (alphaEnabled)
            {
                // Should we add a "," before this value
                if (prevValueWritten)
                {
                    strPixelcolor.append(", ");
                }

                strPixelcolor.append("A: ");
                strPixelcolor.append(QString("%1").arg(qAlpha(pixelcolor)));
                prevValueWritten = true;
            }

            // Was any value written?
            if (!prevValueWritten)
            {
                // No value was written, add "N/A" text
                strPixelcolor.append(AF_STR_NotAvailableA);
            }
        }
    }
}

void gdImagesAndBuffersControlPanel::infoPanelGenerateRawDataValue(acImageItem* pImageItem, QPoint posOnImage, int canvasID, QString& strRawDataValue, bool isHoveringImage)
{
    // Empty output string
    strRawDataValue.clear();

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pImageDataView != nullptr)
    {
        // Get the data view item:
        acDataView* pDataViewManager = m_pImageDataView->dataView();

        // Sanity check:
        GT_IF_WITH_ASSERT((pImageItem != nullptr) && (pDataViewManager != nullptr) && (m_pDisplayedRawFileHandler != nullptr))
        {
            // The position is an image position, we should translate to absolute position (consider zoom on image):
            QPoint absolutePosOnImage = posOnImage;
            bool rc2 = false;

            if (nullptr != m_pImageDataView)
            {
                rc2 = m_pImageDataView->zoomedImagePositionToImagePosition(pImageItem, posOnImage, absolutePosOnImage, isHoveringImage);
            }

            GT_IF_WITH_ASSERT(rc2)
            {
                // Get the raw data from the texture data viewer
                acDataViewItem* pTextureRawData = pDataViewManager->getDataItemByCanvasID(canvasID);
                GT_IF_WITH_ASSERT(pTextureRawData != nullptr)
                {
                    // Get the raw data handler object
                    acRawFileHandler* pRawDataHandler = pTextureRawData->getRawDataHandler();
                    GT_IF_WITH_ASSERT(pRawDataHandler != nullptr)
                    {
                        // Get the raw data format
                        oaTexelDataFormat dataFormat = pTextureRawData->dataFormat();
                        GT_IF_WITH_ASSERT(dataFormat != OA_TEXEL_FORMAT_UNKNOWN)
                        {
                            // Get amount of data format components
                            int amountOfComponents = oaAmountOfTexelFormatComponents(dataFormat);
                            GT_IF_WITH_ASSERT(amountOfComponents >= 1)
                            {
                                // Loop through the raw data components, separate them by a comma ",":
                                for (int i = 0; i < amountOfComponents; i++)
                                {
                                    // Get channel type
                                    oaTexelDataFormat channelDataFormat = oaGetTexelFormatComponentType(dataFormat, i);
                                    GT_IF_WITH_ASSERT(channelDataFormat != OA_TEXEL_FORMAT_UNKNOWN)
                                    {
                                        // Get component value
                                        apPixelValueParameter* pValueParameter = nullptr;
                                        bool isValueAvailable = false;
                                        rc2 = pRawDataHandler->getRawDataComponentValue(absolutePosOnImage.x(), absolutePosOnImage.y(), i, pValueParameter, isValueAvailable);
                                        GT_IF_WITH_ASSERT(rc2)
                                        {
                                            if ((pValueParameter != nullptr) && isValueAvailable)
                                            {
                                                if (pRawDataHandler->dataFormat() != OA_TEXEL_FORMAT_VARIABLE_VALUE)
                                                {
                                                    // Get channel name (in short version)
                                                    gtString channelName;
                                                    bool rc3 = oaGetTexelDataFormatName(channelDataFormat, channelName, true /* Get short version */);
                                                    GT_IF_WITH_ASSERT(rc3)
                                                    {
                                                        // Add a comma before every value (except from the first value)
                                                        if (i != 0)
                                                        {
                                                            strRawDataValue.append(", ");
                                                        }

                                                        // Add channel name and value (as string) to main string
                                                        strRawDataValue.append(acGTStringToQString(channelName));
                                                        strRawDataValue.append(": ");
                                                    }
                                                }

                                                // Convert the value to string:
                                                gtString strComponentValue;

                                                if (gaIsHexDisplayMode())
                                                {
                                                    pValueParameter->valueAsHexString(strComponentValue);
                                                }
                                                else
                                                {
                                                    pValueParameter->valueAsString(strComponentValue);
                                                }

                                                // Append the component value:
                                                strRawDataValue.append(acGTStringToQString(strComponentValue));
                                            }
                                            else
                                            {
                                                strRawDataValue = AF_STR_OutOfScope;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::updateTextureSlidersDisplay
// Description: Updates the 3D Texture slider - Show it if we are in 3D texture
//              mode, and hide it otherwise.
//              Also, update it's settings (current value and maximum value)
// Arguments:   pRawDataHandler - The raw data that we need to adjust the 3d
//              apTextureType textureType - the displayed texture type
//              int currentMiplevel - the currently displayed mip level
//              int minLevel - the current texture min level
//              int maxLevel - the current texture max level
//              texture slider to (active page, maximum amount of pages, etc..)
// Author:      Eran Zinman
// Date:        12/7/2007
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::updateTextureSlidersDisplay(apTextureType textureType, int currentMiplevel, int minLevel, int maxLevel)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pDisplayedRawFileHandler != nullptr) && m_p3DTextureSlider && m_pTextureMipLevelsSlider)
    {
        GT_IF_WITH_ASSERT(m_pDisplayedRawFileHandler->isOk())
        {
            // Get number of pages:
            int numOfPages = m_pDisplayedRawFileHandler->amountOfExternalPages();

            // Do we actually have several pages?
            if (numOfPages > 0)
            {
                // Get current page
                int currentPage = m_pDisplayedRawFileHandler->activePage();

                // Change the slider tick frequency to every 10% of the image. If image is < 10, put tick in every image:
                int ticksFreq = (numOfPages / 10) + 1;
                m_p3DTextureSlider->setTickInterval(ticksFreq);


                // Set the position label value:
                m_pHoveredPositionLabel->setText(GD_STR_ImagesAndBuffersViewerControlPanelHoveredPositionCaption);
                m_pSelectedPositionLabel->setText(GD_STR_ImagesAndBuffersViewerControlPanelSelectedPositionCaption);

                // Hide the local / global work size controls:
                setWorkSizeControlsVisibility(false);

                // Update the 3D slider
                m_p3DTextureSlider->setMaximum(numOfPages - 1);
                m_p3DTextureSlider->setValue(currentPage);

                // Get the currently displayed item data:
                GT_IF_WITH_ASSERT(m_pDisplayedItemData != nullptr)
                {
                    gdDebugApplicationTreeData* pGDDisplayedItemData = qobject_cast<gdDebugApplicationTreeData*>(m_pDisplayedItemData->extendedItemData());
                    GT_IF_WITH_ASSERT(pGDDisplayedItemData != nullptr)
                    {
                        gtASCIIString currentLabel;

                        // Set the slider tooltip and caption according to texture type:
                        if (textureType == AP_3D_TEXTURE)
                        {
                            m_p3DTextureSlider->setToolTip(GD_STR_ImagesAndBuffersViewerControlPanel3DTextureSliderTooltip);
                            m_p3DSliderCaption->setText(GD_STR_ImagesAndBuffersViewerArraySliderCaption);

                            gtASCIIString minLabel, maxLabel;

                            if (pGDDisplayedItemData->_contextId.isOpenGLContext())
                            {
                                currentLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewer3DSliderCaptionGL, currentMiplevel);
                            }
                            else if (pGDDisplayedItemData->_contextId.isOpenCLContext())
                            {
                                currentLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewer3DSliderCaptionCL, currentMiplevel);
                            }

                            minLabel.appendFormattedString("%d", minLevel);
                            maxLabel.appendFormattedString("%d", maxLevel);

                            m_p3DSliderMinLabel->setText(minLabel.asCharArray());
                            m_p3DSliderMaxLabel->setText(maxLabel.asCharArray());
                        }
                        else if ((textureType == AP_2D_ARRAY_TEXTURE) || (textureType == AP_1D_ARRAY_TEXTURE))
                        {
                            currentLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewerArraySliderCaption, pGDDisplayedItemData->_textureLayer);
                            m_p3DTextureSlider->setToolTip(GD_STR_ImagesAndBuffersViewerControlPanelTextureArraySliderTooltip);
                        }

                        m_p3DSliderCaption->setText(currentLabel.asCharArray());

                    }
                }
            }

            // Update the mip levels slider:
            if (minLevel != maxLevel)
            {
                // Set max value only if needed:
                int max = m_pTextureMipLevelsSlider->maximum();

                if (max != maxLevel)
                {
                    m_pTextureMipLevelsSlider->setMaximum(maxLevel);
                    gtASCIIString maxLabel;
                    maxLabel.appendFormattedString("%d", m_pTextureMipLevelsSlider->maximum());
                    m_pMipmapMaxLabel->setText(maxLabel.asCharArray());
                }

                // Set min value only if needed:
                int min = m_pTextureMipLevelsSlider->minimum();

                if (min != minLevel)
                {
                    m_pTextureMipLevelsSlider->setMinimum(minLevel);
                }

                // Set value only if needed:
                int val = m_pTextureMipLevelsSlider->value();

                if (val != currentMiplevel)
                {
                    m_pTextureMipLevelsSlider->setValue(currentMiplevel);
                }

                GLsizei textureWidth = -1, textureHeight = -1;

                if (GetTextureDimensions(textureWidth, textureHeight))
                {
                    gtASCIIString currentLabel;
                    currentLabel.appendFormattedString(GD_STR_ImagesAndBuffersViewerCurrentMiplevelSliderCaption, currentMiplevel);
                    currentLabel.appendFormattedString(" (%d x %d)", textureWidth, textureHeight);
                    m_pMiplevelsSliderCaption->setText(currentLabel.asCharArray());
                }

                m_pMipmapSliderGroupBox->show();
            }
            else
            {
                m_pMipmapSliderGroupBox->hide();
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::updateWorkSizeControls
// Description:
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        13/3/2011
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::updateWorkSizeControls()
{
    // Hide the local / global work size controls:
    setWorkSizeControlsVisibility(true);

    // Update the global / local work size:
    QString globalWorkOffset;
    QString globalWorkSize;
    QString localWorkSize;

    // Build the string for the global work size:
    if (m_currentKernelGlobalWorkSize[0] > 0)
    {
        globalWorkSize.append("X: ");
        globalWorkSize.append(QString("%1").arg(m_currentKernelGlobalWorkSize[0]));
    }

    if (m_currentKernelGlobalWorkSize[1] > 0)
    {
        globalWorkSize.append(", Y: ");
        globalWorkSize.append(QString("%1").arg(m_currentKernelGlobalWorkSize[1]));
    }

    if (m_currentKernelGlobalWorkSize[2] > 0)
    {
        globalWorkSize.append(", Z: ");
        globalWorkSize.append(QString("%1").arg(m_currentKernelGlobalWorkSize[2]));
    }

    // Build the string for the local work size:
    if (m_currentKernelLocalWorkSize[0] > 0)
    {
        localWorkSize.append("X: ");
        localWorkSize.append(QString("%1").arg(m_currentKernelLocalWorkSize[0]));
    }

    if (m_currentKernelLocalWorkSize[1] > 0)
    {
        localWorkSize.append(", Y: ");
        localWorkSize.append(QString("%1").arg(m_currentKernelLocalWorkSize[1]));
    }

    if (m_currentKernelLocalWorkSize[2] > 0)
    {
        localWorkSize.append(", Z: ");
        localWorkSize.append(QString("%1").arg(m_currentKernelLocalWorkSize[2]));
    }

    // Build the string for the global work offset:
    globalWorkOffset.append("X: ");
    globalWorkOffset.append(QString("%1").arg(m_currentKernelGlobalWorkOffset[0]));

    if (m_currentAmountOfWorkDimensions > 1)
    {
        globalWorkOffset.append(", Y: ");
        globalWorkOffset.append(QString("%1").arg(m_currentKernelGlobalWorkOffset[1]));
    }

    if (m_currentAmountOfWorkDimensions > 2)
    {
        globalWorkOffset.append(", Z: ");
        globalWorkOffset.append(QString("%1").arg(m_currentKernelGlobalWorkOffset[2]));
    }

    // Set the text control default style:
    QString text;
    text.sprintf(GD_STR_textBlue, globalWorkOffset.toLatin1().data());
    m_pGlobalWorkOffsetText->setText(text);
    text = "";
    text.sprintf(GD_STR_textBlue, globalWorkSize.toLatin1().data());
    m_pGlobalWorkSizeValueText->setText(text);
    text = "";
    text.sprintf(GD_STR_textBlue, localWorkSize.toLatin1().data());
    m_pLocalWorkSizeValueText->setText(text);

}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::updateMultiVariablesControlsDisplay
// Description: Update the display of the multi variables controls
// Arguments:   gtString& variableName - the displayed variable name
// Author:      Sigal Algranaty
// Date:        27/2/2011
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::updateMultiVariablesControlsDisplay(const gtString& variableName)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pMultiWatchVariableNameCombo != nullptr) && (m_pMultiWatchZCoordinateSlider != nullptr)
                      && (m_pMultiWatchLayout != nullptr) && (m_pMultiWatchVariableTypeText != nullptr) && (m_pTopGroupBox != nullptr))
    {
        m_pMultiWatchVariableTypeText->setText("");
        // Show the multi watch panel:
        m_pMultiWatchGroupBox->show();

        // Set the panel caption:
        setPanelCaption(GD_STR_ImagesAndBuffersViewerControlPanelKernelMultiWatchCaption);

        // Set the work item label value:
        m_pHoveredPositionLabel->setText(GD_STR_ImagesAndBuffersViewerControlPanelHoveredWorkitemCaption);
        m_pSelectedPositionLabel->setText(GD_STR_ImagesAndBuffersViewerControlPanelSelectedWorkitemCaption);

        if (gaIsDebuggedProcessSuspended() && !variableName.isEmpty())
        {
            // Read the current global work size:
            gaGetKernelDebuggingGlobalWorkSize(m_currentKernelGlobalWorkSize[0], m_currentKernelGlobalWorkSize[1], m_currentKernelGlobalWorkSize[2]);

            // Read the current global work offset:
            gaGetKernelDebuggingGlobalWorkOffset(m_currentKernelGlobalWorkOffset[0], m_currentKernelGlobalWorkOffset[1], m_currentKernelGlobalWorkOffset[2]);

            // Read the current local work size:
            gaGetKernelDebuggingLocalWorkSize(m_currentKernelLocalWorkSize[0], m_currentKernelLocalWorkSize[1], m_currentKernelLocalWorkSize[2], m_currentAmountOfWorkDimensions);

            // Enable the multiwatch controls:
            enableMultiwatchControls(true);

            // Update the work size controls:
            updateWorkSizeControls();

            // Set the multi watch variable type text:
            int workItem[3] = {0, 0, 0};
            apExpression variableValue;
            bool rc = gaGetKernelDebuggingExpressionValue(variableName, workItem, 0, variableValue);

            if (rc)
            {
                // Set the variable type text:
                QString text;
                QString varTypeQt = acGTStringToQString(variableValue.m_type);
                text.sprintf(GD_STR_textBlue, varTypeQt.toLatin1().data());
                m_pMultiWatchVariableTypeText->setText(text);
                //disable HexCheckbox on Data View in case of non-integer variable type ( BUG452383 fix)
                GT_IF_WITH_ASSERT(m_pImageDataView != nullptr)
                {
                    acDataView* pDataView = m_pImageDataView->dataView();
                    GT_IF_WITH_ASSERT(pDataView != nullptr)
                    {
                        bool enableHexCheckBox = (0 != variableValue.m_type.compareNoCase(GD_STR_variableTypeFloat)) && (0 != variableValue.m_type.compareNoCase(GD_STR_variableTypeDouble));
                        pDataView->enableHexCheckbox(enableHexCheckBox);
                    }
                }
            }

            // Check if the z coordinate slider should be shown:
            int zCoordMaxValue = 0;

            if (m_pDisplayedRawFileHandler != nullptr)
            {
                zCoordMaxValue = m_pDisplayedRawFileHandler->amountOfExternalPages();
            }

            // Show / Hide the Z-Coordinate slider:
            bool showSlider = (zCoordMaxValue > 1);
            m_pMultiWatchZCoordinateSlider->setVisible(showSlider);
            m_pMultiWatchSliderCaption->setVisible(showSlider);
            m_pMultiWatchSliderMinLabel->setVisible(showSlider);
            m_pMultiWatchSliderMaxLabel->setVisible(showSlider);

            // Update the window layout:
            updateGeometry();

            if (zCoordMaxValue > 1)
            {
                // Get current page:
                int currentPage = m_pDisplayedRawFileHandler->activePage() + m_currentKernelGlobalWorkOffset[2];

                // Change the slider tick frequency to every 10% of the image. If image is < 10, put tick in every image:
                int ticksFreq = (zCoordMaxValue / 10) + 1;
                m_pMultiWatchZCoordinateSlider->setTickInterval(ticksFreq);

                // Update the Z-Coordinate slider:
                // Set the min level of the slider:
                m_pMultiWatchZCoordinateSlider->setMinimum(m_currentKernelGlobalWorkOffset[2]);
                m_pMultiWatchZCoordinateSlider->setMaximum(m_currentKernelGlobalWorkOffset[2] + zCoordMaxValue - 1);
                m_pMultiWatchZCoordinateSlider->setValue(currentPage);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::updateCurrentWorkItem
// Description: Update the view with the current work item
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/3/2011
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersControlPanel::updateCurrentWorkItem()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pImageDataView != nullptr) && (m_pDisplayedItemData != nullptr))
    {
        acDataView* pDataViewManager = m_pImageDataView->dataView();
        acImageManager* pImageViewManager = m_pImageDataView->imageView();

        GT_IF_WITH_ASSERT((pDataViewManager != nullptr) && (pImageViewManager != nullptr))
        {
            gdDebugApplicationTreeData* pGDDisplayedItemData = qobject_cast<gdDebugApplicationTreeData*>(m_pDisplayedItemData->extendedItemData());
            GT_IF_WITH_ASSERT(pGDDisplayedItemData != nullptr)
            {
                // Update the multi variable controls:
                updateMultiVariablesControlsDisplay(pGDDisplayedItemData->_multiVariableName);

                // Check if there are currently items in the manager object:
                int amountOfItems = pImageViewManager->amountOfImagesInManager();

                if (amountOfItems > 0)
                {
                    // Get manager object:
                    acImageItem* pManagerObject = pImageViewManager->getItem(0);

                    if (pManagerObject)
                    {
                        // Get the current work item:
                        int workItem[3];
                        retVal = gaGetKernelDebuggingCurrentWorkItem(workItem[0], workItem[1], workItem[2]);

                        if (retVal)
                        {
                            // Change the z coordinate if needed:
                            if (workItem[2] >= 0)
                            {
                                // Get the current raw file handler:
                                acRawFileHandler* pRawFileHandler = pDataViewManager->getRawDataHandler();

                                if (pRawFileHandler != nullptr)
                                {
                                    if (workItem[2] != pRawFileHandler->activePage())
                                    {
                                        // Change the 3D active page:
                                        setZCoordinate(workItem[2]);
                                    }
                                }
                            }

                            // Highlight the pixel position with the current work item:
                            int yCoord = workItem[1] - m_currentKernelGlobalWorkOffset[1];

                            if (yCoord < 0)
                            {
                                // 1D kernel:
                                yCoord = 0;
                            }

                            QPoint pixelPoint(workItem[0] - m_currentKernelGlobalWorkOffset[0], yCoord);
                            pDataViewManager->highlightPixelPosition(0, pixelPoint);
                        }
                    }
                }
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::disableMultiwatchControls
// Description: Disable the controls handling multiwatch
// Author:      Sigal Algranaty
// Date:        13/3/2011
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::enableMultiwatchControls(bool isEnabled)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pMultiWatchVariableNameCombo != nullptr) && (m_pMultiWatchZCoordinateSlider != nullptr))
    {
        // Enable / Disable the variable names combo box and Z coordinate slider:
        m_pMultiWatchVariableNameCombo->setEnabled(isEnabled);
        m_pMultiWatchZCoordinateSlider->setEnabled(isEnabled);
    }
}
// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::updateBufferFormatControlsDisplay
// Description: Updates the buffer format controls
// Arguments:   pRawFileHandler - The raw data that represent the buffer data file
// Author:      Sigal Algranaty
// Date:        16/4/2009
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::updateBufferFormatControlsDisplay()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pBufferFormatGroupBox != nullptr) && (m_pMainLayout != nullptr) && (m_pTopGroupBox != nullptr))
    {
        if (m_pDisplayedRawFileHandler != nullptr)
        {
            GT_IF_WITH_ASSERT(m_pDisplayedRawFileHandler->isOk())
            {
                // Set the position label value:
                m_pHoveredPositionLabel->setText(GD_STR_ImagesAndBuffersViewerControlPanelHoveredPositionCaption);
                m_pSelectedPositionLabel->setText(GD_STR_ImagesAndBuffersViewerControlPanelSelectedPositionCaption);

                // Hide the work size rows in grid layout:
                setWorkSizeControlsVisibility(false);

                // Update the buffer format values:
                oaTexelDataFormat dataFormat = m_pDisplayedRawFileHandler->dataFormat();

                if (oaIsBufferTexelFormat(dataFormat))
                {
                    // Set the panel caption:
                    setPanelCaption(GD_STR_ImagesAndBuffersViewerControlPanelBufferDisplayOptionsCaption);

                    // Hide the bottom panel:
                    m_pBottomGroupBox->hide();

                    // Hide the pixel information panel:
                    m_pPixelInfoGroupBox->hide();

                    // Set the spin control's maximum value to be our buffer's size:
                    gtSizeType bufferRawDataSize = m_pDisplayedRawFileHandler->getDataSize();
                    m_pOffsetSpinControl->setRange(0, bufferRawDataSize);

                    // Initialize the buffer format selection according to the raw file handler:
                    initializeBufferFormatSelection();

                    m_pBufferFormatGroupBox->setVisible(true);

                    // Update the window layout:
                    updateGeometry();

                    // Get the buffer display properties:
                    oaTexelDataFormat bufferDisplayFormat;
                    int offset = 0;
                    GLsizei stride = 0;
                    m_pDisplayedRawFileHandler->getDisplayProperties(bufferDisplayFormat, offset, stride);

                    // Set the buffer display properties:
                    m_pOffsetSpinControl->setValue(offset);
                    m_pStrideSpinControl->setValue(stride);
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------.
// Name:        gdImagesAndBuffersControlPanel::clearControlPanel
// Description: Clear the control panel from the current displayed item
// Author:      Sigal Algranaty
// Date:        24/5/2010
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::clearControlPanel(bool bClearSelectedPixelInfo)
{
    // Sanity check
    GT_IF_WITH_ASSERT(m_pTopGroupBox != nullptr)
    {
        if (m_pDisplayedItemData != nullptr)
        {
            // Change the main caption to image information:
            if (m_pDisplayedItemData->isItemBuffer(m_pDisplayedItemData->m_itemType))
            {
                setPanelCaption(GD_STR_ImagesAndBuffersViewerControlPanelBufferDisplayOptionsCaption);
            }
            else if (m_pDisplayedItemData->isItemImage(m_pDisplayedItemData->m_itemType))
            {
                setPanelCaption(GD_STR_ImagesAndBuffersViewerControlPanelImageInformationCaption);
            }
        }

        // Clear the current pixel information:
        clearPixelInformation(true, bClearSelectedPixelInfo);

        // Hide the buffer format panel:
        m_pBufferFormatGroupBox->hide();

        // Show the pixel information panel:
        m_pPixelInfoGroupBox->show();

        // Update the window layout:
        updateGeometry();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::updateDoubleSliderDisplay
// Description: Links the double slider to the displayed buffer handler object
// Author:      Eran Zinman
// Date:        25/11/2007
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::updateDoubleSliderDisplay()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pDisplayedRawFileHandler != nullptr) && (m_pDisplayedItemData != nullptr))
    {
        gdDebugApplicationTreeData* pGDDisplayedItemData = qobject_cast<gdDebugApplicationTreeData*>(m_pDisplayedItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDDisplayedItemData != nullptr)
        {
            // Let's verify that our raw data is suitable for be used with double slider
            if (isDisplayedItemSuitableForDoubleSlider())
            {
                // Get texel data format and data type
                oaTexelDataFormat texelFormat;
                oaDataType dataType;
                m_pDisplayedRawFileHandler->getDataTypeAndFormat(texelFormat, dataType);

                // Check if the displayed item is an OpenGL item:
                bool isOpenGLDisplayedItem = false;
                GT_IF_WITH_ASSERT(m_pDisplayedItemData != nullptr)
                {
                    isOpenGLDisplayedItem = (pGDDisplayedItemData->_contextId._contextType == AP_OPENGL_CONTEXT);
                }

                // Set the slider data type according to the raw data type
                bool rc1 = m_pDoubleSlider->setSliderDataType(dataType, isOpenGLDisplayedItem);
                GT_IF_WITH_ASSERT(rc1)
                {
                    // If the data is not normalized, set the slider left and right values
                    if (!m_pDisplayedRawFileHandler->isNormalized())
                    {
                        // Else - use the double slider default minimum and maximum values
                        double minValue = m_pDoubleSlider->sliderLeftValue();
                        double maxValue = m_pDoubleSlider->sliderRightValue();

                        // Set the min max values:
                        bool rc2 = m_pDisplayedRawFileHandler->setMinMaxValues(minValue, maxValue);
                        GT_ASSERT(rc2);

                        // Sets the double slider minimum and maximum possible values:
                        m_pDoubleSlider->setSliderMinMaxPossibleValues(minValue, maxValue);
                    }

                    // Set the double slider title:
                    bool rc3 = setDoubleSliderTitle();
                    GT_IF_WITH_ASSERT(rc3)
                    {
                        // Get the buffer handler left and right values
                        double minValue = 0;
                        double maxValue = 0;
                        m_pDisplayedRawFileHandler->getMinMaxValues(minValue, maxValue);

                        if (m_pDisplayedRawFileHandler->dataFormat() == OA_TEXEL_FORMAT_VARIABLE_VALUE)
                        {
                            // For multiwatch variable - set the max and min values for the variable as the max / min possible values:
                            m_pDoubleSlider->setSliderMinMaxPossibleValues(minValue, maxValue);
                        }

                        // Get the last item properties:
                        acDisplayedItemProperties lastViewItemProperties = m_pImageDataView->lastViewedItemProperties();

                        // Should we use the last viewed item properties for min and max values?
                        if (lastViewItemProperties._arePropertiesValid && lastViewItemProperties._isNormalized)
                        {
                            // Set the raw data handler min and max values
                            m_pDisplayedRawFileHandler->setMinMaxValues(lastViewItemProperties._minValue, lastViewItemProperties._maxValue);

                            // Update the double slider left and right positions
                            m_pDoubleSlider->setSliderPositions(lastViewItemProperties._minValue, lastViewItemProperties._maxValue);
                        }
                        else
                        {
                            // Update the double slider left and right positions
                            m_pDoubleSlider->setSliderPositions(minValue, maxValue);
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::createTopPanelLayout
// Description: Create the top panel layout.
//              1. (positioned on the left) Pixel information (current and previous)
//              2. (positioned on the right) buffer format text + button
// Return Val: void
// Author:      Sigal Algranaty
// Date:        7/4/2009
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::createTopPanelLayout()
{
    m_pTopGroupBox = new QWidget;


    // Create the top layout:
    m_pTopLayout = new QVBoxLayout;

    m_pTopLayout->setContentsMargins(0, 0, 0, 0);

    // Create the MultiWatch controls layout:
    createMultiWatchControlsLayout();

    // Create the pixel information panel layout:
    createPixelInformationLayout();

    // Create the buffer format panel layout:
    createBufferFormatLayout();

    m_pPanelCaption = new QLabel(GD_STR_ImagesAndBuffersViewerControlPanelImageInformationCaption);


    m_pPanelCaption->setTextFormat(Qt::RichText);

    // Add the buffer format container and the pixel information panel to the top layout:
    m_pTopLayout->addSpacing(10);
    m_pTopLayout->addWidget(m_pPanelCaption, 0, Qt::AlignTop);
    m_pTopLayout->addWidget(m_pBufferFormatGroupBox, 0, Qt::AlignTop);
    m_pTopLayout->addWidget(m_pMultiWatchGroupBox, 0, Qt::AlignTop);
    m_pTopLayout->addWidget(m_pPixelInfoGroupBox, 0, Qt::AlignTop);
    m_pTopLayout->addStretch();
    m_pTopLayout->addSpacing(10);

    // Hide the buffer format panel by default:
    m_pBufferFormatGroupBox->hide();
    m_pMultiWatchGroupBox->hide();

    // Apply the change to the window (and resize it if needed):
    m_pTopGroupBox->setLayout(m_pTopLayout);
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::createBufferFormatLayout
// Description: Create the layout for the buffer format controls
// Author:      Sigal Algranaty
// Date:        17/2/2011
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::createBufferFormatLayout()
{
    // Create the buffer format controls, and hide them:
    // These controls are only going to be shown in buffer display mode:

    // Create a layout for the buffer format controls:
    m_pBufferFormatLayout = new QGridLayout;

    m_pBufferFormatLayout->setContentsMargins(0, GD_IMAGES_AND_BUFFERS_ITEMS_TOP_MARGIN, 0, 0);
    m_pBufferFormatLayout->setHorizontalSpacing(0);
    m_pBufferFormatLayout->setVerticalSpacing(GD_IMAGES_AND_BUFFERS_ITEMS_GRID_VERTICAL_SPACING);

    // Create the buffer format group box:
    m_pBufferFormatGroupBox = new QWidget;


    // Create the "Format" headline:
    QLabel* pBufferFormatText = new QLabel(GD_STR_ImagesAndBuffersViewerControlPanelFormatCaption);


    // Create the "Type" headline:
    QLabel* pBufferTypeText = new QLabel(GD_STR_ImagesAndBuffersViewerControlPanelTypeCaption);


    // Initialize buffer Format combo box:
    initBufferFormatCombos();

    // Add the offset text:
    QLabel* pOffsetSpinText = new QLabel(GD_STR_ImagesAndBuffersViewerControlPanelBufferOffsetCaption);
    GT_ASSERT(pOffsetSpinText != nullptr);

    // Create the buffer Offset Spin Ctrl:
    QSize offsetSpinCtrlSize(18 * AC_DEFAULT_TEXT_AVERAGE_CHAR_WIDTH, -1);
    m_pOffsetSpinControl = new QSpinBox;

    m_pOffsetSpinControl->resize(offsetSpinCtrlSize);

    // Connect to the offset value changed event:
    bool rc = connect(m_pOffsetSpinControl, SIGNAL(valueChanged(int)), this, SLOT(onOffsetSpinValueChanged(int)));
    GT_ASSERT(rc);

    // Add the stride text:
    QLabel* pStrideSpinText = new QLabel(GD_STR_ImagesAndBuffersViewerControlPanelBufferStrideCaption);


    // Create the buffer stride Spin Ctrl:
    m_pStrideSpinControl = new QSpinBox;

    QSize strideSpinCtrlSize(10 * AC_DEFAULT_TEXT_AVERAGE_CHAR_WIDTH, -1);
    m_pStrideSpinControl->resize(strideSpinCtrlSize);
    m_pStrideSpinControl->setValue(0);

    // Connect to the offset value changed event:
    rc = connect(m_pStrideSpinControl, SIGNAL(valueChanged(int)), this, SLOT(onStrideSpinValueChanged(int)));
    GT_ASSERT(rc);

    // Add the buffer format controls to the layouts:
    m_pBufferFormatLayout->addWidget(pBufferTypeText, 0, 0, 1 , 1, Qt::AlignTop);
    m_pBufferFormatLayout->addWidget(pBufferFormatText, 1, 0, 1 , 1, Qt::AlignTop);
    m_pBufferFormatLayout->addWidget(pOffsetSpinText, 2, 0, 1 , 1, Qt::AlignTop);
    m_pBufferFormatLayout->addWidget(pStrideSpinText, 3, 0, 1 , 1, Qt::AlignTop);

    // Add the combos and spin controls to the second column:
    m_pBufferFormatLayout->addWidget(m_pBufferFormatsGroupsComboBox, 0, 1, 1, 1, Qt::AlignTop);
    m_pBufferFormatLayout->addWidget(m_pBufferFormatsComboBox, 1, 1, 1, 1, Qt::AlignTop);
    m_pBufferFormatLayout->addWidget(m_pOffsetSpinControl, 2, 1, 1, 1, Qt::AlignTop);
    m_pBufferFormatLayout->addWidget(m_pStrideSpinControl, 3, 1, 1, 1, Qt::AlignTop);

    // Set the buffer format layout:
    m_pBufferFormatGroupBox->setLayout(m_pBufferFormatLayout);
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::createPixelInformationLayout
// Description: Create the pixel information layout
// Author:      Sigal Algranaty
// Date:        17/2/2011
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::createPixelInformationLayout()
{
    m_pPixelInfoGroupBox = new QWidget;


    // Create the bottom panel text:
    QLabel* pPanelBottomText = new QLabel(GD_STR_ImagesAndBuffersViewerImageMouseHoverMessage);


    // Create the layout for the table:
    m_pPixelInformationGridLayout = new QGridLayout;

    m_pPixelInformationGridLayout->setContentsMargins(0, GD_IMAGES_AND_BUFFERS_ITEMS_TOP_MARGIN, 0, 0);

    // Create the current and selected color values text:
    // Notice: The dummy string doesn't have enough characters, since we want to display the current pixel information,
    // that contain something like 120 characters (take GL_FLOAT type in consideration):
    int amountOfChars = 30;
    QSize currrentRGBValuesSize(amountOfChars * AC_DEFAULT_TEXT_AVERAGE_CHAR_WIDTH + GD_IMAGES_AND_DATA_CONTROL_VALUE_CAPTION_X_MARGIN, -1);

    // Create the current and selected color bitmaps:
    QSize colorSampleSize(GD_IMAGES_AND_BUFFERS_COLOR_SAMPLE_SIZE, GD_IMAGES_AND_BUFFERS_COLOR_SAMPLE_SIZE);

    // Create the current pixel color, value and position text boxes:

    // Define the size for the RGB values text control:
    m_pHoveredColorPreview = new acColorSampleBox(this, colorSampleSize);


    m_pSelectedColorPreview = new acColorSampleBox(this, colorSampleSize);


    // Create the labels for the values:
    m_pGlobalWorkOffsetText = createBlueForegroundLabel();
    m_pGlobalWorkSizeValueText = createBlueForegroundLabel();
    m_pLocalWorkSizeValueText = createBlueForegroundLabel();
    m_pHoveredPixelPositionText = createBlueForegroundLabel();
    m_pHoveredPixelColorText = createBlueForegroundLabel();
    m_pHoveredPixelValueText = createBlueForegroundLabel();
    m_pSelectedPixelPositionText = createBlueForegroundLabel();
    m_pSelectedPixelColorText = createBlueForegroundLabel();
    m_pSelectedPixelValueText = createBlueForegroundLabel();

    // Set the size of the selected and hovered value labels:
    QString testValuesStr;
    testValuesStr.sprintf("R: %u, G: %u, B: %u, A: %u", 255, 255, 255, 255);
    QRect testValuesStrBoundingRect = QFontMetrics(m_pHoveredPixelValueText->font()).boundingRect(testValuesStr);

    // Create the titles for the left column:
    m_pGlobalWorkOffsetLabel = new QLabel(GD_STR_ImagesAndBuffersViewerControlPanelGlobalWorkOffsetCaption);


    m_pGlobalWorkSizeLabel = new QLabel(GD_STR_ImagesAndBuffersViewerControlPanelGlobalWorkSizeCaption);


    m_pLocalWorkSizeLabel = new QLabel(GD_STR_ImagesAndBuffersViewerControlPanelLocalWorkSizeCaption);


    m_pHoveredPositionLabel = new QLabel(GD_STR_ImagesAndBuffersViewerControlPanelHoveredPositionCaption);


    m_pSelectedPositionLabel = new QLabel(GD_STR_ImagesAndBuffersViewerControlPanelSelectedPositionCaption);

    QLabel* pHoveredValueLabel = new QLabel(GD_STR_ImagesAndBuffersViewerControlPanelHoveredValueCaption);


    QLabel* pSelectedValueLabel = new QLabel(GD_STR_ImagesAndBuffersViewerControlPanelSelectedValueCaption);


    QLabel* pHoveredColorPixelLabel = new QLabel(GD_STR_ImagesAndBuffersViewerControlPanelHoveredColorCaption);


    QLabel* pSelectedColorPixelLabel = new QLabel(GD_STR_ImagesAndBuffersViewerControlPanelSelectedColorCaption);


    // Setup the first column layout (the lines captions):
    m_pPixelInformationGridLayout->addWidget(m_pGlobalWorkOffsetLabel, 0, 0, 1, 1);
    m_pPixelInformationGridLayout->addWidget(m_pGlobalWorkSizeLabel, 1, 0, 1, 1);
    m_pPixelInformationGridLayout->addWidget(m_pLocalWorkSizeLabel, 2, 0, 1, 1);
    m_pPixelInformationGridLayout->addWidget(m_pHoveredPositionLabel, 3, 0, 1, 1);
    m_pPixelInformationGridLayout->addWidget(m_pSelectedPositionLabel, 4, 0, 1, 1);
    m_pPixelInformationGridLayout->addWidget(pHoveredValueLabel, 5, 0, 1, 1);
    m_pPixelInformationGridLayout->addWidget(pSelectedValueLabel, 6, 0, 1, 1);
    m_pPixelInformationGridLayout->addWidget(pHoveredColorPixelLabel, 7, 0, 1, 1);
    m_pPixelInformationGridLayout->addWidget(pSelectedColorPixelLabel, 8, 0, 1, 1);

    // Setup the second column layout (the values of the position, color and values of the pixel):
    m_pPixelInformationGridLayout->addWidget(m_pGlobalWorkOffsetText, 0, 1, 1, 2);
    m_pPixelInformationGridLayout->addWidget(m_pGlobalWorkSizeValueText, 1, 1, 1, 2);
    m_pPixelInformationGridLayout->addWidget(m_pLocalWorkSizeValueText, 2, 1, 1, 2);
    m_pPixelInformationGridLayout->addWidget(m_pHoveredPixelPositionText, 3, 1, 1, 2);
    m_pPixelInformationGridLayout->addWidget(m_pSelectedPixelPositionText, 4, 1, 1, 2);
    m_pPixelInformationGridLayout->addWidget(m_pHoveredPixelValueText, 5, 1, 1, 2);
    m_pPixelInformationGridLayout->addWidget(m_pSelectedPixelValueText, 6, 1, 1, 2);
    m_pPixelInformationGridLayout->addWidget(m_pHoveredPixelColorText, 7, 1, 1, 1);
    m_pPixelInformationGridLayout->addWidget(m_pSelectedPixelColorText, 8, 1, 1, 1);


    m_pPixelInformationGridLayout->setColumnMinimumWidth(1, testValuesStrBoundingRect.width());

    // The last column - contain only lines 8, 9 - the color sample boxes:
    m_pPixelInformationGridLayout->addWidget(m_pHoveredColorPreview, 7, 2, 1, 1);
    m_pPixelInformationGridLayout->addWidget(m_pSelectedColorPreview, 8, 2, 1, 1);
    m_pPixelInformationGridLayout->addWidget(pPanelBottomText, 9, 0, 1, 3);

    m_pPixelInformationGridLayout->setColumnStretch(0, 10);
    m_pPixelInformationGridLayout->setColumnStretch(1, 10);
    m_pPixelInformationGridLayout->setColumnStretch(2, 0);

    m_pPixelInformationGridLayout->setColumnMinimumWidth(1, testValuesStrBoundingRect.width() + 3);

    // Set a fixed row height for each pixel information line:
    for (int i = 0 ; i < 10 ; i++)
    {
        m_pPixelInformationGridLayout->setRowMinimumHeight(i, GD_IMAGES_AND_BUFFERS_COLOR_SAMPLE_SIZE - 4);
    }

    // Set the pixel format group layout:
    m_pPixelInfoGroupBox->setLayout(m_pPixelInformationGridLayout);
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::createBottomPanelLayout
// Description: Create the bottom panel layout
//              1. Double slider
// Return Val: void
// Author:      Sigal Algranaty
// Date:        7/4/2009
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::createBottomPanelLayout()
{
    // Create the bottom group box:
    m_pBottomGroupBox = new QWidget;


    // Create the bottom layout:
    m_pBottomLayout = new QVBoxLayout;

    m_pBottomLayout->setContentsMargins(0, 0, 0, 0);

    // Create bottom part, include:
    // 1. Double slider for adjusting min and max values
    // 2. 3D Texture slider
    // 3. Mipmaps slider

    m_pNotificationTextLine = new QLabel(GD_STR_ImagesAndBuffersViewerControlPanelStaleObjectWarning);

    m_pNotificationTextLine->setVisible(false);

    m_pNotificationTextLine->setTextFormat(Qt::RichText);

    QPalette p = m_pNotificationTextLine->palette();
    p.setColor(QPalette::Foreground, Qt::red);
    m_pNotificationTextLine->setPalette(p);


    // Create the 3D slider controls:
    create3DSliderLayout();

    // Create mip level controls:
    createMiplevelLayout();

    // Create double slider layout:
    createDoubleSliderLayout();

    m_pBottomLayout->addWidget(m_pDoubleSliderGroupBox, 0);
    m_pBottomLayout->addWidget(m_pMipmapSliderGroupBox, 0);
    m_pBottomLayout->addWidget(m_p3DSliderGroupBox, 0);
    m_pBottomLayout->addWidget(m_pNotificationTextLine, 0);

    m_pBottomGroupBox->setLayout(m_pBottomLayout);
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::create3DSliderLayout
// Description: Create the 3D slider controls
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        17/3/2011
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::create3DSliderLayout()
{
    // Create 3D slider layout:
    m_p3DSliderLayout = new QGridLayout;

    m_p3DSliderLayout->setContentsMargins(0, 0, 0, 0);

    // Create the 3D Texture slider container:
    m_p3DSliderGroupBox = new QWidget;


    // Hide this box by default:
    m_p3DSliderGroupBox->setVisible(false);

    m_p3DSliderMinLabel = new QLabel("0");

    m_p3DSliderMaxLabel = new QLabel("0");


    // Create the 3D Texture slider:
    m_p3DTextureSlider = new QSlider(Qt::Horizontal);

    m_p3DTextureSlider->setTickPosition(QSlider::TicksBelow);
    m_p3DTextureSlider->setPageStep(SLIDER_PAGE_STEP);

    // Set the 3D Texture slider tooltip and tick frequency:
    m_p3DTextureSlider->setMinimum(0);
    m_p3DTextureSlider->setMaximum(120);
    m_p3DTextureSlider->setTickInterval(10);
    m_p3DTextureSlider->setToolTip(GD_STR_ImagesAndBuffersViewerControlPanel3DTextureSliderTooltip);

    // Connect to 3D textures slider change slot:
    bool rc = connect(m_p3DTextureSlider, SIGNAL(valueChanged(int)), this, SLOT(on3DSliderChange(int)));
    GT_ASSERT(rc);

    // Create the 3d slider caption:
    gtASCIIString captionStr;
    captionStr.appendFormattedString(GD_STR_ImagesAndBuffersViewer3DSliderCaptionGL, 0);
    m_p3DSliderCaption = new QLabel(captionStr.asCharArray());


    m_p3DSliderLayout->addWidget(m_p3DSliderCaption, 0, 0, 1, 3);
    m_p3DSliderLayout->addWidget(m_p3DSliderMinLabel, 1, 0, 1, 1);
    m_p3DSliderLayout->addWidget(m_p3DTextureSlider, 1, 1, 1, 1);
    m_p3DSliderLayout->addWidget(m_p3DSliderMaxLabel, 1, 2, 1, 1);
    m_p3DSliderLayout->setColumnStretch(0, 5);
    m_p3DSliderLayout->setColumnStretch(1, 90);
    m_p3DSliderLayout->setColumnStretch(2, 5);

    m_p3DSliderGroupBox->setLayout(m_p3DSliderLayout);
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::createMultiWatchControlsLayout
// Description: Create multi watch controls layout
// Author:      Sigal Algranaty
// Date:        27/2/2011
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::createMultiWatchControlsLayout()
{
    // Create multi watch panel sizers:
    m_pMultiWatchLayout = new QGridLayout;

    m_pMultiWatchLayout->setContentsMargins(0, 5, 0, 0);
    m_pMultiWatchLayout->setHorizontalSpacing(0);
    m_pMultiWatchLayout->setVerticalSpacing(5);

    m_pMultiWatchGroupBox = new QWidget;


    // Create the multi watch slider caption:
    gtASCIIString multiWatchVariableString(GD_STR_MultiWatchVariableNameCaption);
    m_pMultiWatchVariableNameCaption = new QLabel(multiWatchVariableString.asCharArray());

    // Create the name combo box:
    m_pMultiWatchVariableNameCombo = new QComboBox;


    // Set the size of the selected and hovered value labels:
    QRect testValuesStrBoundingRect = QFontMetrics(m_pMultiWatchVariableNameCombo->font()).boundingRect(GD_STR_MultiWatchSelectVariable);
    int comboWidth = testValuesStrBoundingRect.width() + 30;
    m_pMultiWatchVariableNameCombo->setMinimumWidth(comboWidth);

    // Make the combo editable:
    m_pMultiWatchVariableNameCombo->setEditable(true);
    m_pMultiWatchVariableNameCombo->resize(QSize(comboWidth, -1));
    m_pMultiWatchVariableNameCombo->addItem(GD_STR_MultiWatchSelectVariable);
    m_pMultiWatchVariableNameCombo->setEnabled(false);

    m_pMultiWatchLayout->addWidget(m_pMultiWatchVariableNameCaption, 0, 0, 1, 1, Qt::AlignLeft);
    m_pMultiWatchLayout->addWidget(m_pMultiWatchVariableNameCombo, 0, 1, 1, 3, Qt::AlignCenter);

    // Create the multi watch slider caption:
    gtASCIIString zcoordCaption;
    zcoordCaption.appendFormattedString(GD_STR_MultiWatchSliderCaption, 0);
    m_pMultiWatchSliderCaption = new QLabel(zcoordCaption.asCharArray());
    m_pMultiWatchSliderMinLabel = new QLabel("0");
    m_pMultiWatchSliderMaxLabel = new QLabel("0");


    // Create the slider for the multi watch Z coordinate:
    m_pMultiWatchZCoordinateSlider = new QSlider(Qt::Horizontal);

    m_pMultiWatchZCoordinateSlider->setMinimum(0);
    m_pMultiWatchZCoordinateSlider->setMaximum(120);
    m_pMultiWatchZCoordinateSlider->setTickPosition(QSlider::TicksBelow);
    m_pMultiWatchZCoordinateSlider->setMinimumWidth(comboWidth - 50);

    // Hide the slider by default:
    m_pMultiWatchZCoordinateSlider->setVisible(false);
    m_pMultiWatchSliderCaption->setVisible(false);
    m_pMultiWatchSliderMinLabel->setVisible(false);
    m_pMultiWatchSliderMaxLabel->setVisible(false);
    // Set the slider tooltip and tick frequency:
    m_pMultiWatchZCoordinateSlider->setTickInterval(1);
    m_pMultiWatchZCoordinateSlider->setPageStep(SLIDER_PAGE_STEP);
    m_pMultiWatchZCoordinateSlider->setToolTip(GD_STR_MultiWatchSliderTooltip);

    // Connect to Z-coordinate slider change slot:
    bool rc = connect(m_pMultiWatchZCoordinateSlider, SIGNAL(valueChanged(int)), this, SLOT(onZCoordSliderChange(int)));
    GT_ASSERT(rc);

    m_pMultiWatchLayout->addWidget(m_pMultiWatchSliderCaption, 1, 0, 1, 1, Qt::AlignLeft);
    m_pMultiWatchLayout->addWidget(m_pMultiWatchSliderMinLabel, 1, 1, 1, 1, Qt::AlignRight);
    m_pMultiWatchLayout->addWidget(m_pMultiWatchZCoordinateSlider, 1, 2, 1, 1, Qt::AlignCenter);
    m_pMultiWatchLayout->addWidget(m_pMultiWatchSliderMaxLabel, 1, 3, 1, 1, Qt::AlignLeft);

    // Create the multi watch slider caption:
    m_pMultiWatchVariableTypeCaption = new QLabel(GD_STR_MultiWatchVariableTypeCaption);


    // Create a variable type text control:
    m_pMultiWatchVariableTypeText = createBlueForegroundLabel();

    m_pMultiWatchVariableTypeText->setTextFormat(Qt::RichText);

    // Add the variable type and text control to the sizer:
    m_pMultiWatchLayout->addWidget(m_pMultiWatchVariableTypeCaption, 2, 0, 1, 1);
    m_pMultiWatchLayout->addWidget(m_pMultiWatchVariableTypeText, 2, 1, 1, 3, Qt::AlignLeft);

    // We want that the combo box would be wide as the pixel information cell, so we give it the
    // calculated width for the pixel information label as minimum, and we decrease the digits from it.

    QString test = GD_STR_PropertiesQueueCommandGlobalWorkOffsetA;
    int firstColWidth = 0;
    testValuesStrBoundingRect = QFontMetrics(m_pMultiWatchSliderMaxLabel->font()).boundingRect(test);
    firstColWidth = testValuesStrBoundingRect.width() + 10;

    m_pMultiWatchLayout->setColumnMinimumWidth(0, firstColWidth);
    m_pMultiWatchLayout->setColumnMinimumWidth(2, comboWidth);

    // Set the multi watch layout:
    m_pMultiWatchGroupBox->setLayout(m_pMultiWatchLayout);

}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::createMiplevelLayout
// Description: Create the layout for the mip level controls
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        17/3/2011
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::createMiplevelLayout()
{
    // Create mip levels slider layout & group box:
    m_pMipmapSliderLayout = new QGridLayout;

    m_pMipmapSliderLayout->setContentsMargins(0, 0, 0, 0);

    m_pMipmapSliderGroupBox = new QWidget;

    // Hide this group box by default:
    m_pMipmapSliderGroupBox->setVisible(false);

    gtASCIIString mipLevel;
    mipLevel.appendFormattedString(GD_STR_ImagesAndBuffersViewerCurrentMiplevelSliderCaption, 0);

    // Create the mip level slider caption:
    m_pMiplevelsSliderCaption = new QLabel(mipLevel.asCharArray());

    m_pMipmapMinLabel = new QLabel("0");

    m_pMipmapMaxLabel = new QLabel("0");

    m_pTextureMipLevelsSlider = new QSlider(Qt::Horizontal);

    m_pTextureMipLevelsSlider->setMinimum(0);
    m_pTextureMipLevelsSlider->setMaximum(0);
    m_pTextureMipLevelsSlider->setToolTip(GD_STR_ImagesAndBuffersViewerControlPanelTextureMiplevelsSliderTooltip);
    m_pTextureMipLevelsSlider->setTickPosition(QSlider::TicksBelow);
    m_pTextureMipLevelsSlider->setPageStep(SLIDER_PAGE_STEP);
    m_pTextureMipLevelsSlider->setSingleStep(SLIDER_PAGE_STEP);

    // Connect to mip level slider change slot:
    bool rc = connect(m_pTextureMipLevelsSlider, SIGNAL(valueChanged(int)), this, SLOT(onMiplevelsSliderChange(int)));
    GT_ASSERT(rc);

    m_pMipmapSliderLayout->addWidget(m_pMiplevelsSliderCaption, 0, 0, 1, 3);
    m_pMipmapSliderLayout->addWidget(m_pMipmapMinLabel, 1, 0, 1, 1);
    m_pMipmapSliderLayout->addWidget(m_pTextureMipLevelsSlider, 1, 1, 1, 1);
    m_pMipmapSliderLayout->addWidget(m_pMipmapMaxLabel, 1, 2, 1, 1);
    m_pMipmapSliderLayout->setColumnStretch(0, 5);
    m_pMipmapSliderLayout->setColumnStretch(1, 90);
    m_pMipmapSliderLayout->setColumnStretch(2, 5);

    m_pMipmapSliderGroupBox->setLayout(m_pMipmapSliderLayout);
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::createDoubleSliderLayout
// Description: Create double slider controls layout
// Author:      Sigal Algranaty
// Date:        17/3/2011
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::createDoubleSliderLayout()
{
    // Create mip levels slider layout & group box:
    m_pDoubleSliderLayout = new QVBoxLayout;

    m_pDoubleSliderLayout->setContentsMargins(0, 0, 0, 0);

    m_pDoubleSliderGroupBox = new QWidget;


    // Hide this group box by default:
    m_pDoubleSliderGroupBox->setVisible(false);

    // Add the double slider headline:
    m_pDoubleSliderTitle = new QLabel;


    // Create the double slider item:
    m_pDoubleSlider = new acDoubleSlider;


    m_pDoubleSliderLayout->addWidget(m_pDoubleSliderTitle);
    m_pDoubleSliderLayout->addWidget(m_pDoubleSlider);

    // Set the buffer format layout:
    m_pDoubleSliderGroupBox->setLayout(m_pDoubleSliderLayout);

    bool rc = connect(m_pDoubleSlider, SIGNAL(doubleSliderPositionChanged(double, double)), this, SLOT(onDoubleSliderPositionChanged(double, double)));
    GT_ASSERT(rc);

}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::onBufferFormatChange
// Description: The buffer format was changed
// Arguments:   int currentItemIndex
// Author:      Sigal Algranaty
// Date:        22/4/2009
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::onBufferFormatChange(int currentItemIndex)
{
    // Update the buffer format with the selected format index:
    updateBufferFormat(currentItemIndex);
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::onBufferFormatGroupChange
// Description: The buffer format group combo value has changed
// Arguments:   int currentItemIndex
// Author:      Sigal Algranaty
// Date:        22/4/2009
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::onBufferFormatGroupChange(int currentIndex)
{
    gdTextureAndBuffersInfoPanelBufferFormatGroup selectedFormatsGroup = (gdTextureAndBuffersInfoPanelBufferFormatGroup)currentIndex;
    initBufferFormatsComboBox(selectedFormatsGroup);

    GT_IF_WITH_ASSERT(m_pBufferFormatsComboBox != nullptr)
    {
        // Get the currently selected format index:
        int selectedIndex = m_pBufferFormatsComboBox->currentIndex();

        // Update the buffer format with the selected format index:
        updateBufferFormat(selectedIndex);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::initBufferFormatCombos
// Description: Initializes the buffer format combo box
// Return Val: void
// Author:      Sigal Algranaty
// Date:        22/4/2009
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::initBufferFormatCombos()
{
    // Create the buffer format combo boxes:
    m_pBufferFormatsComboBox = new QComboBox;


    m_pBufferFormatsGroupsComboBox = new QComboBox;


    // Connect to buffer format change slot:
    bool rc = connect(m_pBufferFormatsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onBufferFormatChange(int)));
    GT_ASSERT(rc);

    // Connect to buffer format change slot:
    rc = connect(m_pBufferFormatsGroupsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onBufferFormatGroupChange(int)));
    GT_ASSERT(rc);

    // Add the relevant buffer format groups to the combo box:
    for (int i = GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_FIRST; i <= GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_LAST; i++)
    {
        // Translate the current buffer format group to a string:
        gtASCIIString currentBufferFormatGroupAsString = bufferFormatGroupToString((gdTextureAndBuffersInfoPanelBufferFormatGroup)i);

        // Add the Buffer format groups string to the combo box:
        m_pBufferFormatsGroupsComboBox->addItem(currentBufferFormatGroupAsString.asCharArray());
    }

    // Select the current format:
    m_pBufferFormatsGroupsComboBox->setCurrentIndex(GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_ALL);

    // Initialize the specific buffer formats:
    initBufferFormatsComboBox(GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_ALL);
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::initBufferFormatsComboBox
// Description: Init buffer format combo box according to selected group of formats
// Arguments: gdTextureAndBuffersInfoPanelBufferFormatGroup formatsGroup
// Return Val: void
// Author:      Sigal Algranaty
// Date:        22/4/2009
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::initBufferFormatsComboBox(gdTextureAndBuffersInfoPanelBufferFormatGroup formatsGroup)
{
    m_isUpdatingFormatCombos = true;

    // Check what is the currently selected format, so that if the new format group is all,
    // we can reselect this format:
    oaTexelDataFormat currentlySelectFormat = OA_TEXEL_FORMAT_FIRST_GL_BUFFER_FORMAT;
    int selectedFormatIndex = m_pBufferFormatsComboBox->currentIndex();

    if ((selectedFormatIndex > 0) && (selectedFormatIndex < (int)m_currentDisplayedTexelFormats.size()))
    {
        currentlySelectFormat = m_currentDisplayedTexelFormats[selectedFormatIndex];
    }

    // Check what should be the first and last buffer formats to display:
    oaTexelDataFormat firstBufferFormat = OA_TEXEL_FORMAT_FIRST_GL_BUFFER_FORMAT;
    oaTexelDataFormat lastBufferFormat = OA_TEXEL_FORMAT_LAST_GL_BUFFER_FORMAT;

    // By default select the first FBO format for this group:
    selectedFormatIndex = 0;

    // Reset the formats currently displayed:
    m_currentDisplayedTexelFormats.clear();

    // Clear the combo box:
    m_pBufferFormatsComboBox->clear();

    switch (formatsGroup)
    {
        case GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_COLOR:
            firstBufferFormat = OA_TEXEL_FIRST_GL_COLOR_BUFFER_FORMAT;
            lastBufferFormat = OA_TEXEL_LAST_GL_COLOR_BUFFER_FORMAT;
            break;

        case GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_VERTEX:
            firstBufferFormat = OA_TEXEL_FIRST_GL_VERTEX_BUFFER_FORMAT;
            lastBufferFormat = OA_TEXEL_LAST_GL_VERTEX_BUFFER_FORMAT;
            break;

        case GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_INDEX:
            firstBufferFormat = OA_TEXEL_FIRST_GL_COLORINDEX_BUFFER_FORMAT;
            lastBufferFormat = OA_TEXEL_LAST_GL_COLORINDEX_BUFFER_FORMAT;
            break;

        case GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_NORMAL:
            firstBufferFormat = OA_TEXEL_FIRST_GL_NORMAL_BUFFER_FORMAT;
            lastBufferFormat = OA_TEXEL_LAST_GL_NORMAL_BUFFER_FORMAT;
            break;

        case GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_TEXTURE:
            firstBufferFormat = OA_TEXEL_FIRST_GL_TEXCOORD_BUFFER_FORMAT;
            lastBufferFormat = OA_TEXEL_LAST_GL_TEXCOORD_BUFFER_FORMAT;
            break;

        case GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_INTERLEAVED:
            firstBufferFormat = OA_TEXEL_FIRST_GL_INTERLEAVED_BUFFER_FORMAT;
            lastBufferFormat = OA_TEXEL_LAST_GL_INTERLEAVED_BUFFER_FORMAT;
            break;

        case GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_ALL:
            firstBufferFormat = OA_TEXEL_FORMAT_FIRST_GL_BUFFER_FORMAT;
            lastBufferFormat = OA_TEXEL_FORMAT_LAST_GL_BUFFER_FORMAT;
            selectedFormatIndex = currentlySelectFormat - OA_TEXEL_FORMAT_FIRST_GL_BUFFER_FORMAT;
            break;

        case GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_OPENCL_C:
            firstBufferFormat = OA_TEXEL_FORMAT_FIRST_CL_BUFFER_FORMAT;
            lastBufferFormat = OA_TEXEL_FORMAT_LAST_CL_BUFFER_FORMAT;
            break;

        default:
            GT_ASSERT_EX(false, GD_STR_ImagesAndBuffersViewerControlPanelUnknownBufferFormatGroup);
            break;
    }

    GT_IF_WITH_ASSERT(lastBufferFormat > firstBufferFormat)
    {
        for (int i = firstBufferFormat; i <= lastBufferFormat; i++)
        {
            // Translate the current buffer format to a string:
            oaTexelDataFormat currentTexelFormat = (oaTexelDataFormat)i;
            gtString temp;
            oaTexelDataFormatAsString(currentTexelFormat, temp);
            QString currentBufferFormatAsString = acGTStringToQString(temp);

            // Cut the "OA_TEXEL_FORMAT_" from the string:
            currentBufferFormatAsString.replace(GD_STR_ImagesAndBuffersViewerControlPanelTexelFormatPrefix, AF_STR_EmptyA);

            if (formatsGroup == GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_OPENCL_C)
            {
                // Display the OpenCL C types as lower case strings:
                currentBufferFormatAsString = currentBufferFormatAsString.toLower();
            }

            // Add the buffer format string to the combo box:
            m_pBufferFormatsComboBox->addItem(currentBufferFormatAsString);

            // Add this texel format to the displayed ones:
            m_currentDisplayedTexelFormats.push_back(currentTexelFormat);
        }
    }

    m_isUpdatingFormatCombos = false;

    m_pBufferFormatsComboBox->setCurrentIndex(selectedFormatIndex);


}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::initializeBufferFormatSelection
// Description: Is called when a buffer is displayed, and initializes the buffer format
//              combo box selection with the current buffer raw file format
// Arguments:   acRawFileHandler* pRawFileHandler - the raw file handler
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/4/2009
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersControlPanel::initializeBufferFormatSelection()
{
    bool retVal = false;
    // Select the first format:
    int selectedFormatIndex = 0;

    if (m_pDisplayedRawFileHandler != nullptr)
    {
        // Get the current buffer format:
        oaTexelDataFormat currentBufferFormat = m_pDisplayedRawFileHandler->dataFormat();

        // Get the matching formats group:
        gdTextureAndBuffersInfoPanelBufferFormatGroup matchingFormatsGroup = getMatchingFormatGroup(currentBufferFormat);

        // Set the currently selected formats group:
        m_pBufferFormatsGroupsComboBox->setCurrentIndex((int)matchingFormatsGroup);

        // Initialize the formats according to selected formats group:
        initBufferFormatsComboBox(matchingFormatsGroup);

        // Go through the current formats, and select the originally selected format:
        for (int i = 0; i < (int)m_currentDisplayedTexelFormats.size(); i++)
        {
            if (m_currentDisplayedTexelFormats[i] == currentBufferFormat)
            {
                selectedFormatIndex = i;
                retVal = true;
                break;
            }
        }

        // Check if the data format can be displayed as hexadecimal:
        bool isHexEnabled = oaCanTypeBeDisplayedAsHex(currentBufferFormat);

        // Enable / disable the hexadecimal format:
        GT_IF_WITH_ASSERT(m_pImageDataView != nullptr)
        {
            acDataView* pDataView = m_pImageDataView->dataView();
            GT_IF_WITH_ASSERT(pDataView != nullptr)
            {
                pDataView->enableHexCheckbox(isHexEnabled);
            }
        }
    }

    // Select the current format:
    m_pBufferFormatsComboBox->setCurrentIndex(selectedFormatIndex);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::bufferFormatGroupToString
// Description: Translate a buffer formats group to a string
// Arguments: gdTextureAndBuffersInfoPanelBufferFormatGroup formatGroup
// Return Val: gtString
// Author:      Sigal Algranaty
// Date:        22/4/2009
// ---------------------------------------------------------------------------
gtASCIIString gdImagesAndBuffersControlPanel::bufferFormatGroupToString(gdTextureAndBuffersInfoPanelBufferFormatGroup formatGroup)
{
    gtASCIIString retVal = AF_STR_EmptyA;

    switch (formatGroup)
    {
        case GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_OPENCL_C:
            retVal = GD_STR_ImagesAndBuffersViewerControlPanelBufferOpenCLC;
            break;

        case GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_COLOR :
            retVal = GD_STR_ImagesAndBuffersViewerControlPanelBufferGroupColor;
            break;

        case GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_VERTEX:
            retVal = GD_STR_ImagesAndBuffersViewerControlPanelBufferGroupVertex;
            break;

        case GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_INDEX:
            retVal = GD_STR_ImagesAndBuffersViewerControlPanelBufferGroupIndex;
            break;

        case GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_NORMAL:
            retVal = GD_STR_ImagesAndBuffersViewerControlPanelBufferGroupNormal;
            break;

        case GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_TEXTURE:
            retVal = GD_STR_ImagesAndBuffersViewerControlPanelBufferGroupTexture;
            break;

        case GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_INTERLEAVED:
            retVal = GD_STR_ImagesAndBuffersViewerControlPanelBufferGroupInterleaved;
            break;

        case GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_ALL:
            retVal = GD_STR_ImagesAndBuffersViewerControlPanelBufferGroupAll;
            break;


        default:
            GT_ASSERT(false);
            break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::updateBufferFormat
// Description: Updates the data view with the selected buffer format index
// Arguments: int selectedBufferFormatIndex
// Return Val: void
// Author:      Sigal Algranaty
// Date:        23/4/2009
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::updateBufferFormat(int selectedBufferFormatIndex)
{
    if ((m_pImageDataView != nullptr) && (!m_isUpdatingFormatCombos))
    {
        GT_IF_WITH_ASSERT((selectedBufferFormatIndex >= 0) && (selectedBufferFormatIndex < (int)m_currentDisplayedTexelFormats.size()))
        {
            // Translate the selected value to a texel format:
            oaTexelDataFormat newDataFormat = m_currentDisplayedTexelFormats[selectedBufferFormatIndex];

            // Get the data view manager:
            acDataView* pDataViewManager = m_pImageDataView->dataView();
            GT_IF_WITH_ASSERT(pDataViewManager != nullptr)
            {
                // Get the current raw file handler:
                acRawFileHandler* pRawFileHandler = pDataViewManager->getRawDataHandler();

                if (pRawFileHandler != nullptr)
                {
                    // Get the current texel format (to see if it is changed):
                    oaTexelDataFormat currentDataFormat = pRawFileHandler->dataFormat();

                    // If buffer format was updated by the user, update the data view manager:
                    if (currentDataFormat != newDataFormat)
                    {
                        // Load the data into the grid view with the new buffer format:
                        pDataViewManager->updateDataItem();

                        // Check if the data format can be displayed as hexadecimal:
                        bool isHexEnabled = oaCanTypeBeDisplayedAsHex(newDataFormat);

                        // Enable / disable the hexadecimal format:
                        pDataViewManager->enableHexCheckbox(isHexEnabled);

                        // Set the new raw file format:
                        pRawFileHandler->setBufferDataFormat(newDataFormat);

                        // Update the data viewer:
                        pDataViewManager->updateDataItem();

                        // Get the current display properties set on the file handler:
                        int currentDisplayedOffset = 0;
                        GLsizei currentDisplayedStride = 0;
                        oaTexelDataFormat currentDisplayedFormat = OA_TEXEL_FORMAT_UNKNOWN;
                        pRawFileHandler->getDisplayProperties(currentDisplayedFormat, currentDisplayedOffset, currentDisplayedStride);

                        // Update the relevant buffer with the current display properties:
                        bool rc = updateBufferDisplayProperties(currentDisplayedFormat, currentDisplayedOffset, currentDisplayedStride);
                        GT_ASSERT(rc);
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdOptionsDialog::onOffsetSpinCtrlTextChanged
// Description: Is called when the user change the value of the offset spin ctrl.
// Author:      Sigal Algranaty
// Date:        23/4/2009
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::onOffsetSpinValueChanged(int newValue)
{
    // Get the data view manager:
    acDataView* pDataViewManager = m_pImageDataView->dataView();
    GT_IF_WITH_ASSERT(pDataViewManager != nullptr)
    {
        // Get the current raw file handler:
        acRawFileHandler* pRawFileHandler = pDataViewManager->getRawDataHandler();

        if (pRawFileHandler != nullptr)
        {
            // Get the previous offset value:
            int previousOffset = pRawFileHandler->offset();

            // Get the offset out of the event:
            int currentOffset = newValue;

            // Check if this is not a "false" event:
            if ((currentOffset != previousOffset) && (currentOffset >= 0))
            {
                // Set the new offset:
                pRawFileHandler->setOffset(currentOffset);

                // Update the data viewer:
                pDataViewManager->updateDataItem();

                // Get the current display properties set on the file handler:
                int currentDisplayedOffset = 0;
                GLsizei currentDisplayedStride = 0;
                oaTexelDataFormat currentDisplayedFormat = OA_TEXEL_FORMAT_UNKNOWN;
                pRawFileHandler->getDisplayProperties(currentDisplayedFormat, currentDisplayedOffset, currentDisplayedStride);

                // Update the relevant buffer with the current display properties:
                bool rc = updateBufferDisplayProperties(currentDisplayedFormat, currentDisplayedOffset, currentDisplayedStride);
                GT_ASSERT(rc);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdOptionsDialog::onStrideSpinCtrlTextChanged
// Description: Is called when the user change the value of the stride spin ctrl.
// Author:      Sigal Algranaty
// Date:        23/4/2009
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::onStrideSpinValueChanged(int newValue)
{
    // Get the data view manager:
    acDataView* pDataViewManager = m_pImageDataView->dataView();
    GT_IF_WITH_ASSERT(pDataViewManager != nullptr)
    {
        // Get the current raw file handler:
        acRawFileHandler* pRawFileHandler = pDataViewManager->getRawDataHandler();

        if (pRawFileHandler != nullptr)
        {
            // Get the previous stride value:
            int previousStride = pRawFileHandler->stride();

            // Get the stride out of the event:
            int currentStride = newValue;

            // Check if this is not a "false" event:
            if (currentStride != previousStride)
            {
                // Set the new offset:
                pRawFileHandler->setStride(currentStride);

                // Update the data viewer:
                pDataViewManager->updateDataItem();

                // Get the current display properties set on the file handler:
                int currentDisplayedOffset = 0;
                GLsizei currentDisplayedStride = 0;
                oaTexelDataFormat currentDisplayedFormat = OA_TEXEL_FORMAT_UNKNOWN;
                pRawFileHandler->getDisplayProperties(currentDisplayedFormat, currentDisplayedOffset, currentDisplayedStride);

                // Update the relevant buffer with the current display properties:
                bool rc = updateBufferDisplayProperties(currentDisplayedFormat, currentDisplayedOffset, currentDisplayedStride);
                GT_ASSERT(rc);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::getMatchingFormatGroup
// Description: Return a VBO format group by VBO format
// Arguments: oaTexelDataFormat vboFormat
// Return Val: gdTextureAndBuffersInfoPanelBufferFormatGroup
// Author:      Sigal Algranaty
// Date:        5/5/2009
// ---------------------------------------------------------------------------
gdImagesAndBuffersControlPanel::gdTextureAndBuffersInfoPanelBufferFormatGroup gdImagesAndBuffersControlPanel::getMatchingFormatGroup(oaTexelDataFormat vboFormat)
{
    gdTextureAndBuffersInfoPanelBufferFormatGroup retVal = GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_ALL;

    if ((vboFormat >= OA_TEXEL_FIRST_GL_COLOR_BUFFER_FORMAT) && (vboFormat <= OA_TEXEL_LAST_GL_COLOR_BUFFER_FORMAT))
    {
        retVal = GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_COLOR;
    }
    else if ((vboFormat >= OA_TEXEL_FIRST_GL_VERTEX_BUFFER_FORMAT) && (vboFormat <= OA_TEXEL_LAST_GL_VERTEX_BUFFER_FORMAT))
    {
        retVal = GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_VERTEX;
    }
    else if ((vboFormat >= OA_TEXEL_FIRST_GL_NORMAL_BUFFER_FORMAT) && (vboFormat <= OA_TEXEL_LAST_GL_NORMAL_BUFFER_FORMAT))
    {
        retVal = GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_NORMAL;
    }
    else if ((vboFormat >= OA_TEXEL_FIRST_GL_TEXCOORD_BUFFER_FORMAT) && (vboFormat <= OA_TEXEL_LAST_GL_TEXCOORD_BUFFER_FORMAT))
    {
        retVal = GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_TEXTURE;
    }
    else if ((vboFormat >= OA_TEXEL_FIRST_GL_INTERLEAVED_BUFFER_FORMAT) && (vboFormat <= OA_TEXEL_LAST_GL_INTERLEAVED_BUFFER_FORMAT))
    {
        retVal = GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_INTERLEAVED;
    }
    else if ((vboFormat >= OA_TEXEL_FIRST_GL_COLORINDEX_BUFFER_FORMAT) && (vboFormat <= OA_TEXEL_LAST_GL_COLORINDEX_BUFFER_FORMAT))
    {
        retVal = GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_INDEX;
    }
    else if ((vboFormat >= OA_TEXEL_FORMAT_FIRST_CL_BUFFER_FORMAT) && (vboFormat <= OA_TEXEL_FORMAT_LAST_CL_BUFFER_FORMAT))
    {
        retVal = GD_IMAGES_AND_BUFFERS_BUFFER_FORMATS_OPENCL_C;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::updateBufferDisplayProperties
// Description: Update the currently display buffer (VBO or a CL buffer) with the
//              user selected display properties - format, offset & stride
// Arguments: oaTexelDataFormat displayFormat
//            int displayOffset
//            gtSize_t displayStride
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/12/2009
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersControlPanel::updateBufferDisplayProperties(oaTexelDataFormat displayFormat, int displayOffset, gtSize_t displayStride)
{
    bool retVal = false;

    // Sanity Check:
    GT_IF_WITH_ASSERT(m_pImageDataView != nullptr)
    {
        // Get the currently displayed item data:
        GT_IF_WITH_ASSERT(m_pDisplayedItemData != nullptr)
        {
            gdDebugApplicationTreeData* pGDDisplayedItemData = qobject_cast<gdDebugApplicationTreeData*>(m_pDisplayedItemData->extendedItemData());
            GT_IF_WITH_ASSERT(pGDDisplayedItemData != nullptr)
            {
                // If the last selected context is an OpenGL context - VBO:
                if (pGDDisplayedItemData->_contextId.isOpenGLContext())
                {
                    // Get the currently displayed VBO name:
                    GLuint vboName = 0;

                    vboName = pGDDisplayedItemData->_objectOpenGLName;

                    // Make sure the VBO name is valid:
                    if (vboName != 0)
                    {
                        // Set the VBO display details:
                        retVal = gaSetVBODisplayProperties(pGDDisplayedItemData->_contextId._contextId, vboName, displayFormat, displayOffset, displayStride);
                    }
                }
                // If the last selected context is an OpenGL context - CL buffer:
                else if (pGDDisplayedItemData->_contextId.isOpenCLContext())
                {
                    // Get the selected buffer id:
                    int bufferIndex = pGDDisplayedItemData->_objectOpenCLIndex;

                    if (bufferIndex >= 0)
                    {
                        // Set the CL buffer display details:
                        retVal = gaSetCLBufferDisplayProperties(pGDDisplayedItemData->_contextId._contextId, bufferIndex, displayFormat, displayOffset, displayStride);
                    }
                }
                else // contextId.isDefault()
                {
                    // nullptr Context should not contain texture objects:
                    GT_ASSERT_EX(false, L"should not get here");
                }
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::initializeVariableNamesCombo
// Description: Set the selected variable name
// Arguments:   const gtString& variableName
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        28/2/2011
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::initializeVariableNamesCombo(bool isInKernelDebugging)
{
    // Sanity check
    GT_IF_WITH_ASSERT(m_pMultiWatchVariableNameCombo != nullptr)
    {
        // Block the combo box signals:
        m_pMultiWatchVariableNameCombo->blockSignals(true);

        // Get the currently selected name:
        QString currentlySelectedName = selectedMultiWatchVariableName();

        // Clear the combo names:
        m_pMultiWatchVariableNameCombo->clear();
        m_multiWatchComboVarNameToIndexMap.clear();
        int itemIndex = -1;

        if (gaIsDebuggedProcessSuspended() && isInKernelDebugging)
        {
            // Enable the variables combo box:
            m_pMultiWatchVariableNameCombo->setEnabled(true);

            // Get all the current variable names:
            gtVector<apExpression> multiWatchVariableNamesVector;
            bool rcGetNames = gaGetKernelDebuggingAvailableVariables(0, multiWatchVariableNamesVector, true, -1, true);
            GT_ASSERT(rcGetNames);

            // Add the available kernel names:
            for (int i = 0 ; i < (int)multiWatchVariableNamesVector.size(); i++)
            {
                // Currently, filter out derefernced values, since the multiwatch view only supports direct values:
                QString currentVarName = acGTStringToQString(multiWatchVariableNamesVector[i].m_name);

                if ((currentVarName.indexOf('*') < 0) && (currentVarName.indexOf('[') < 0) && (currentVarName.indexOf(']') < 0))
                {
                    // Add the current string to the combo:
                    m_pMultiWatchVariableNameCombo->addItem(currentVarName);
                    int currentItemIndex  = m_pMultiWatchVariableNameCombo->count() - 1;
                    m_multiWatchComboVarNameToIndexMap[currentVarName] = currentItemIndex;

                    if (currentVarName == currentlySelectedName)
                    {
                        itemIndex = currentItemIndex;
                    }
                }
            }
        }

        // If the previous variable does not exist, add it, and select it:
        if ((itemIndex < 0) && (!currentlySelectedName.isEmpty()))
        {
            // Add the variable name to the combo:
            m_pMultiWatchVariableNameCombo->addItem(currentlySelectedName);
            itemIndex = m_pMultiWatchVariableNameCombo->count() - 1;
            m_multiWatchComboVarNameToIndexMap[currentlySelectedName] = itemIndex;
        }

        if (itemIndex < 0)
        {
            if (isInKernelDebugging)
            {
                // Add a message to the top combo box:
                m_pMultiWatchVariableNameCombo->addItem(GD_STR_MultiWatchSelectVariable);
                itemIndex = m_pMultiWatchVariableNameCombo->count() - 1;
            }
            else
            {
                // In this case disable the combo:
                m_pMultiWatchVariableNameCombo->setEnabled(false);
            }
        }

        // Select the item:
        m_pMultiWatchVariableNameCombo->setCurrentIndex(itemIndex);

        // Unblock the combo box signals:
        m_pMultiWatchVariableNameCombo->blockSignals(false);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::selectedMultiWatchVariableName
// Description: Get the current variable name from the variable names combo box
// Return Val:  const gtString
// Author:      Sigal Algranaty
// Date:        28/2/2011
// ---------------------------------------------------------------------------
const QString gdImagesAndBuffersControlPanel::selectedMultiWatchVariableName() const
{
    QString retVal;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pMultiWatchVariableNameCombo != nullptr)
    {
        retVal.append(m_pMultiWatchVariableNameCombo->currentText());
    }

    // Check if the current variable name is the fixed strings we add to the combo box:
    bool isVariableNameConstant = (retVal == GD_STR_MultiWatchSelectVariable);

    if (isVariableNameConstant)
    {
        retVal.clear();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::selectedMultiWatchVariableIndex
// Description: Get the current selected index
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        4/4/2011
// ---------------------------------------------------------------------------
int gdImagesAndBuffersControlPanel::selectedMultiWatchVariableIndex() const
{
    int retVal = -1;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pMultiWatchVariableNameCombo != nullptr)
    {
        retVal = m_pMultiWatchVariableNameCombo->currentIndex();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::selectMultiWatchVariableName
// Description:
// Arguments:    const gtString& variableName
//              bool& isVariableExist
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        3/3/2011
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersControlPanel::selectMultiWatchVariableName(const gtString& variableName, bool& isVariableExist) const
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pMultiWatchVariableNameCombo != nullptr)
    {
        retVal = true;

        // Get the index for the requested variable name:
        int comboItemIndex = -1;
        QString asciiVarName = acGTStringToQString(variableName);
        gtMap<QString, int>::const_iterator findIter = m_multiWatchComboVarNameToIndexMap.find(asciiVarName);

        if (findIter != m_multiWatchComboVarNameToIndexMap.end())
        {
            comboItemIndex = (*findIter).second;
        }

        // If the variable exist within the combo box, select it:
        if (comboItemIndex >= 0)
        {
            m_pMultiWatchVariableNameCombo->setCurrentIndex(comboItemIndex);
            isVariableExist = true;

            // If the combo contain a "Please type of select..." string, remove it:
            int itemToRemove = m_pMultiWatchVariableNameCombo->findText(GD_STR_MultiWatchSelectVariable, Qt::MatchExactly);

            if (itemToRemove >= 0)
            {
                m_pMultiWatchVariableNameCombo->removeItem(itemToRemove);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::setZCoord
// Description: Sets the Z coordinate value
// Arguments:   int coordinate
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/3/2011
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersControlPanel::setZCoordinate(int newZCoordValue)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pMultiWatchZCoordinateSlider != nullptr)
    {
        GT_IF_WITH_ASSERT((newZCoordValue <= m_pMultiWatchZCoordinateSlider->maximum()) && (newZCoordValue >= m_pMultiWatchZCoordinateSlider->minimum()))
        {
            // Set the z coordinate value:
            m_pMultiWatchZCoordinateSlider->setValue(newZCoordValue);

            // At this stage we should force the slider change event, since we need to set the Z coordinate properties:
            m_forceZCoordSlider = true;

            // Perform a Z coord slider change event:
            onZCoordSliderChange(newZCoordValue);

            // Update the min / max and current mipmap levels of the texture:
            GT_IF_WITH_ASSERT((m_pMultiWatchSliderMinLabel != nullptr) && (m_pMultiWatchSliderMaxLabel != nullptr) && (m_pMultiWatchZCoordinateSlider != nullptr) && (m_pMultiWatchSliderCaption != nullptr))
            {
                gtASCIIString minLabel, maxLabel, currentLabel;
                minLabel.appendFormattedString("%d", m_pMultiWatchZCoordinateSlider->minimum());
                maxLabel.appendFormattedString("%d", m_pMultiWatchZCoordinateSlider->maximum());

                currentLabel.appendFormattedString(GD_STR_MultiWatchSliderCaption, newZCoordValue);

                m_pMultiWatchSliderMinLabel->setText(minLabel.asCharArray());
                m_pMultiWatchSliderMaxLabel->setText(maxLabel.asCharArray());
                m_pMultiWatchSliderCaption->setText(currentLabel.asCharArray());

            }

            m_forceZCoordSlider = false;

            retVal = true;
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::setComboBoxFocus
// Description:
// Arguments:   selectedIndex - the variable index to select
// Author:      Sigal Algranaty
// Date:        4/4/2011
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::setComboBoxFocus(int selectedIndex)
{
    GT_IF_WITH_ASSERT(m_pMultiWatchVariableNameCombo != nullptr)
    {
        m_pMultiWatchVariableNameCombo->setFocus();

        if (selectedIndex >= 0)
        {
            m_pMultiWatchVariableNameCombo->setCurrentIndex(selectedIndex);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::onShowHidePanelClick
// Description: Toggle the show / hide state of the control panel
// Author:      Sigal Algranaty
// Date:        9/5/2011
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::onShowHidePanelClick()
{
    // Toggle the expanded flag:
    m_isExpanded = !m_isExpanded;

    // Setup controls after is expanded state had changed:
    setupControlsAfterExpansionChange();
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::setupControlsAfterExpansionChange
// Description: Setup the controls after expanded / not expanded state had changed
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        11/5/2011
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::setupControlsAfterExpansionChange()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pTopGroupBox != nullptr) && (m_pBottomGroupBox != nullptr) && (m_pShowHideButton != nullptr) &&
                      (m_pMainLayout != nullptr) && (m_pShowBitmap != nullptr) && (m_pHideBitmap != nullptr))
    {
        // Toggle the show / hide of the right sizer:
        m_pTopGroupBox->setVisible(m_isExpanded);
        m_pBottomGroupBox->setVisible(m_isExpanded);

        if (m_isExpanded)
        {
            // Set the hide bitmap:
            m_pShowHideButton->setIcon(QIcon(*m_pHideBitmap));

            // Get the displayed data:
            if (m_pDisplayedItemData != nullptr)
            {
                // Show / Hide each of the sizers:
                setupPanel(m_pDisplayedItemData);
            }

            m_pMainLayout->setColumnMinimumWidth(1, m_minWidth);
        }
        else
        {
            // Set the show bitmap:
            m_pShowHideButton->setIcon(QIcon(*m_pShowBitmap));
            m_pMainLayout->setColumnMinimumWidth(1, 0);

        }

        //         if (m_pImageDataView != nullptr)
        //         {
        //             m_pImageDataView->updateGeometry();
        //             int zoomLevel = 0;
        //             m_pImageDataView->applyBestFit(zoomLevel);
        //         }

        // Update the window layout:
        layout()->activate();
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::setIsExpanded
// Description:
// Arguments:   bool shouldControlPanelBeExpanded
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        11/5/2011
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::setIsExpanded(bool isExpanded)
{
    m_isExpanded = isExpanded;

    // Layout the panel controls after expansion:
    setupControlsAfterExpansionChange();
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::createBlueForegroundLabel
// Description: Create a label with a blue foreground
// Return Val:  QLabel*
// Author:      Sigal Algranaty
// Date:        25/6/2012
// ---------------------------------------------------------------------------
QLabel* gdImagesAndBuffersControlPanel::createBlueForegroundLabel()
{
    QLabel* pRetVal = new QLabel;

    pRetVal->setTextFormat(Qt::RichText);

    QPalette p = pRetVal->palette();
    p.setColor(QPalette::Foreground, Qt::blue);
    pRetVal->setPalette(p);

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::setPanelCaption
// Description: Sets the panel caption
// Arguments:   const gtASCIIString& caption
// Author:      Sigal Algranaty
// Date:        4/7/2012
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::setPanelCaption(const gtASCIIString& caption)
{
    GT_IF_WITH_ASSERT(m_pPanelCaption != nullptr)
    {
        gtASCIIString title;
        title.appendFormattedString("<p><b>%s</b></p>", caption.asCharArray());
        m_pPanelCaption->setText(title.asCharArray());
    }
}


// ---------------------------------------------------------------------------
// Name:        setWidgetBGForDebug
// Description: Used for debugging
// Arguments:   QWidget* pWidget
//              const QColor& c
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        5/7/2012
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::setWidgetBGForDebug(QWidget* pWidget, const QColor& c)

{
    GT_IF_WITH_ASSERT(pWidget != nullptr)
    {
        // Set the background color:
        QPalette p = pWidget->palette();
        p.setColor(pWidget->backgroundRole(), c);
        p.setColor(QPalette::Base, c);
        pWidget->setAutoFillBackground(true);
        pWidget->setPalette(p);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersControlPanel::setWorkSizeControlsVisibility
// Description: Show / hide the work size controls visibility
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        15/7/2012
// ---------------------------------------------------------------------------
void gdImagesAndBuffersControlPanel::setWorkSizeControlsVisibility(bool isVisible)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pPixelInformationGridLayout != nullptr) && (m_pGlobalWorkOffsetLabel != nullptr) && (m_pGlobalWorkSizeLabel != nullptr) &&
                      (m_pLocalWorkSizeLabel != nullptr) && (m_pGlobalWorkOffsetText != nullptr) && (m_pGlobalWorkSizeValueText != nullptr) && (m_pLocalWorkSizeValueText != nullptr))
    {
        // Hide the local / global work size controls:
        m_pGlobalWorkOffsetLabel->setVisible(isVisible);
        m_pGlobalWorkSizeLabel->setVisible(isVisible);
        m_pLocalWorkSizeLabel->setVisible(isVisible);
        m_pGlobalWorkOffsetText->setVisible(isVisible);
        m_pGlobalWorkSizeValueText->setVisible(isVisible);
        m_pLocalWorkSizeValueText->setVisible(isVisible);

        int rowMinHeight = isVisible ? GD_IMAGES_AND_BUFFERS_COLOR_SAMPLE_SIZE - 4 : 0;

        // Set a fixed row height for each pixel information line:
        for (int i = 0 ; i < 3 ; i++)
        {
            m_pPixelInformationGridLayout->setRowMinimumHeight(i, rowMinHeight);
        }
    }
}


/**
***************************************************************************************************
* \brief Name:        displayStaleObjectWarningMessage
* \brief Description:
* \return             void
* \author             salgrana
* \date               21/8/2012
***************************************************************************************************
*/
void gdImagesAndBuffersControlPanel::displayStaleObjectWarningMessage()
{
    if ((m_pDisplayedItemData != nullptr) && (m_pNotificationTextLine != nullptr))
    {
        if (gaIsInKernelDebugging())
        {
            bool isItemCLMemObj = (afApplicationTreeItemData::isItemImageOrBuffer(m_pDisplayedItemData->m_itemType)
                                   || (m_pDisplayedItemData->m_itemType == AF_TREE_ITEM_GL_TEXTURES_NODE) || (m_pDisplayedItemData->m_itemType == AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE)
                                   || (m_pDisplayedItemData->m_itemType == AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE) || (m_pDisplayedItemData->m_itemType == AF_TREE_ITEM_GL_PBUFFERS_NODE)
                                   || (m_pDisplayedItemData->m_itemType == AF_TREE_ITEM_GL_VBO_NODE) || (m_pDisplayedItemData->m_itemType == AF_TREE_ITEM_GL_FBO_NODE)
                                   || (m_pDisplayedItemData->m_itemType == AF_TREE_ITEM_CL_IMAGES_NODE) || (m_pDisplayedItemData->m_itemType == AF_TREE_ITEM_CL_BUFFERS_NODE)
                                   || (m_pDisplayedItemData->m_itemType == AF_TREE_ITEM_CL_BUFFERS_NODE) || (m_pDisplayedItemData->m_itemType == AF_TREE_ITEM_CL_BUFFER)
                                   || (m_pDisplayedItemData->m_itemType == AF_TREE_ITEM_CL_SUB_BUFFER)
                                   || (m_pDisplayedItemData->m_itemType == AF_TREE_ITEM_CL_PIPES_NODE) || (m_pDisplayedItemData->m_itemType == AF_TREE_ITEM_CL_PIPE));


            // Show / Hide the notification:
            m_pNotificationTextLine->setVisible(isItemCLMemObj);

            if (isItemCLMemObj)
            {
                m_pBottomGroupBox->show();
            }
        }
    }
}

bool gdImagesAndBuffersControlPanel::GetTextureDimensions(GLsizei& textureWidth, GLsizei& textureHeight, /*[in]*/int newValue)
{
    bool res = true;

    if (m_pDisplayedItemData != nullptr)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(m_pDisplayedItemData->extendedItemData());

        if (pGDData != nullptr)
        {
            afTreeItemType propertiesItemType = m_pDisplayedItemData->m_itemType;

            if (propertiesItemType == AF_TREE_ITEM_GL_TEXTURE)
            {
                apGLTextureMipLevelID textureId;
                textureId._textureName = pGDData->_objectOpenGLName;
                textureId._textureMipLevel = 0;
                apGLTextureMiplevelData textureMiplevelData;
                res = gaGetTextureObjectThumbnailData(pGDData->_contextId._contextId, textureId._textureName, textureMiplevelData);

                if (res)
                {
                    if (textureMiplevelData.textureType() != AP_BUFFER_TEXTURE)
                    {
                        apGLTexture textureDetails;
                        res = gaGetTextureObjectDetails(pGDData->_contextId._contextId, textureId._textureName, textureDetails);

                        // Get the texture dimensions:
                        if (res)
                        {
                            GLsizei textureDepth, textureBorder;
                            textureDetails.getDimensions(textureWidth, textureHeight, textureDepth, textureBorder, newValue);
                        }
                    }
                }
            }
        }
    }

    return res;
}
