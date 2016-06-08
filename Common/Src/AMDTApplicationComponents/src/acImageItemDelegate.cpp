//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acImageItemDelegate.cpp
///
//==================================================================================


// Qt:
#include <QComboBox>
#include <QPaintEngine>
#include <QPainter>
#include <QTextDocument>
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acImageItemDelegate.h>
#include <AMDTApplicationComponents/Include/acImageManager.h>
#include <AMDTApplicationComponents/Include/res/icons/image_background_pattern.xpm>

QColor acImageItemDelegate::m_sBGColor = Qt::black;
bool acImageItemDelegate::m_sInitialized = false;
QBrush* acImageItemDelegate::m_spBGBrush = NULL;
// ---------------------------------------------------------------------------
// Name:        acImageItemDelegate::acImageItemDelegate
// Description: Constructor
// Arguments:   acImageManager* pImageManager
//              const QColor& bgColor
// Author:      Sigal Algranaty
// Date:        20/6/2012
// ---------------------------------------------------------------------------
acImageItemDelegate::acImageItemDelegate(acImageManager* pImageManager)
    : QStyledItemDelegate(pImageManager), m_pImageManager(pImageManager), m_bFirstTimeBestFit(true)
{
    if (!m_sInitialized)
    {
        // Get the system default background color:
        m_sBGColor = acGetSystemDefaultBackgroundColor();
        QPixmap bgPixmap(image_background_pattern);
        m_spBGBrush = new QBrush(Qt::white, bgPixmap);

    }
}

// ---------------------------------------------------------------------------
// Name:        acImageItemDelegate::~acImageItemDelegate
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        20/6/2012
// ---------------------------------------------------------------------------
acImageItemDelegate::~acImageItemDelegate()
{

}

// ---------------------------------------------------------------------------
// Name:        acImageItemDelegate::paint
// Description: Implementing a single item paint
// Arguments:   QPainter* pPainter
//              const QStyleOptionViewItem & option
//              const QModelIndex & index
// Author:      Sigal Algranaty
// Date:        20/6/2012
// ---------------------------------------------------------------------------
void acImageItemDelegate::paint(QPainter* pPainter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    GT_UNREFERENCED_PARAMETER(option);

    gtString outputStr;

    // NOTICE: There is a strange bug. For some reason, when we raise the assert dialog while we paint, the painter crashes
    // (one of the paint engines parameters is NULL). Thererfore, we should wrap each assert call with parinter::save & restore calls:
    pPainter->save();

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pImageManager != NULL) && (pPainter != NULL))
    {
        pPainter->restore();

        gtString itemTopLabel, itemBottomLabel;

        // Get the image item:
        acImageItem* pItem = m_pImageManager->getItem(index);

        if (pItem != NULL)
        {
            itemTopLabel = pItem->topLabel();
            itemBottomLabel = pItem->bottomLabel();

            if (itemTopLabel.isEmpty())
            {
                itemTopLabel = m_pImageManager->m_itemHeading;
            }

            // Get the bounding rect for the requested item:
            QRect itemRect = m_pImageManager->itemVisibleRect(index);
            QSize itemSize = itemRect.size();

            // Fill the item background color for this single item:
            pPainter->fillRect(itemRect, m_sBGColor);

            if (!pItem->isEmpty())
            {
                int textBottom = 0;

                if (!pItem->isDisplayingText() && !itemTopLabel.isEmpty())
                {
                    // Draw the item top label:
                    drawTopLabel(index, pPainter, itemTopLabel, textBottom);
                }

                // Draw the item image:
                QRect imageRect;
                drawItemImage(pPainter, index, textBottom, itemSize, itemRect, imageRect);

                if (!pItem->isDisplayingText() && !itemBottomLabel.isEmpty())
                {
                    // Draw the item bottom label:
                    drawItemBottomLabel(pPainter, itemBottomLabel.asASCIICharArray(), imageRect, itemRect);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acImageItemDelegate::drawItemImage
// Description: Calculate the position and size of the image to draw.
// Arguments:   const QImage& srcImage
//              const QRect& textBoundingRect
//              QPoint& imageOriginOnCanvas
//              QPoint& imageOrigin
//              QSize& imageDimensions
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        14/6/2012
// ---------------------------------------------------------------------------
void acImageItemDelegate::drawItemImage(QPainter* pPainter, const QModelIndex& index, int textBottom, const QSize& itemSize, const QRect& itemBoundingRect, QRect& imageRect) const
{
    // Sanity check
    GT_IF_WITH_ASSERT((pPainter != NULL) && (m_pImageManager != NULL))
    {
        // The function is calculating the parameter for the following Qt draw image function:
        // void QPainter::drawImage ( int x, int y, const QImage & image, int sx = 0, int sy = 0, int sw = -1, int sh = -1)
        // The parameters needed for this function are as follows:
        // x, y: specifies the top-left point in the paint device that is to be drawn onto
        // sx, sy: specifies the top-left point in image that is to be drawn. The default is (0, 0)
        // sw, sh: specifies the size of the image that is to be drawn. The default, (0, 0) (and negative) means all the way to the bottom-right of the image.

        // Get the image X and Y positions:
        int imageYPos = textBottom + AC_IMAGE_MANAGER_IMAGE_TO_LABEL_MARGIN;
        int imageXPos = itemBoundingRect.left();
        imageRect.setTop(imageYPos);
        imageRect.setLeft(imageXPos);

        // Find the maximum possible image dimensions:
        bool isSingleItem = ((m_pImageManager->m_managerMode == AC_MANAGER_MODE_STANDARD_ITEM) || (m_pImageManager->m_managerMode == AC_MANAGER_MODE_TEXT_ITEM));
        int imageMaxW = isSingleItem ? itemSize.width() : AC_IMAGES_MANAGER_THUMBNAIL_SIZE;
        int imageMaxH = isSingleItem ? itemSize.height() : AC_IMAGES_MANAGER_THUMBNAIL_SIZE;

        imageRect.setWidth(imageMaxW);
        imageRect.setHeight(imageMaxH);

        // Get the item image index:
        int imageIndex = m_pImageManager->modelIndexToImageIndex(index);

        if ((imageIndex >= 0) && (imageIndex < (int)m_pImageManager->m_imageItems.size()))
        {
            // Get the draw item:
            acImageItem* pImageItem = m_pImageManager->m_imageItems[imageIndex];

            if (pImageItem != NULL)
            {
                if (pImageItem->isDisplayingText())
                {
                    displayItemAsText(pPainter, imageRect, pImageItem);
                }
                else
                {
                    displayItemAsImage(pPainter, imageRect, pImageItem, itemSize);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acImageItemDelegate::drawItemBottomLabel
// Description: Draw the item bottom label
// Arguments:   QPainter* pPainter
//              const gtString &itemBottomLabel
//              const QRect &imageRect
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        20/6/2012
// ---------------------------------------------------------------------------
void acImageItemDelegate::drawItemBottomLabel(QPainter* pPainter, const gtASCIIString& itemBottomLabel, const QRect& imageRect, const QRect& itemRect) const
{
    // Sanity check
    GT_IF_WITH_ASSERT(!itemBottomLabel.isEmpty() && pPainter != NULL)
    {
        // Break the label into 2 lines:
        gtASCIIString bottomLabelLine1 = itemBottomLabel, bottomLabelLine2;
        int newLinePos = itemBottomLabel.find('\n');

        if (newLinePos > 0)
        {
            itemBottomLabel.getSubString(0, newLinePos - 1, bottomLabelLine1);
            itemBottomLabel.getSubString(newLinePos + 1, itemBottomLabel.length() - 1, bottomLabelLine2);
        }

        // Get the first line text bounding rect:
        QFont font = pPainter->font();
        QRect bottomTextBoundingRect1 = QFontMetrics(font).boundingRect(bottomLabelLine1.asCharArray());

        // Calculate the position for the 2 lines of the bottom label:
        int bottomLabelY1 = imageRect.top() + AC_IMAGES_MANAGER_THUMBNAIL_SIZE + AC_IMAGE_MANAGER_IMAGE_TO_LABEL_MARGIN;

        QTextOption textOptions;
        textOptions.setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        textOptions.setWrapMode(QTextOption::WordWrap);
        QRect textRect(QPoint(itemRect.x(), bottomLabelY1), QSize(itemRect.width(), bottomTextBoundingRect1.height()));
        pPainter->drawText(textRect, bottomLabelLine1.asCharArray(), textOptions);
        textRect.setTop(textRect.top() + AC_IMAGE_MANAGER_IMAGE_TO_LABEL_MARGIN + bottomTextBoundingRect1.height());
        textRect.setBottom(textRect.bottom() + AC_IMAGE_MANAGER_IMAGE_TO_LABEL_MARGIN + bottomTextBoundingRect1.height());
        pPainter->drawText(textRect, bottomLabelLine2.asCharArray(), textOptions);
    }
}


// ---------------------------------------------------------------------------
// Name:        acImageItemDelegate::drawTopLabel
// Description: Draw the item top label
// Arguments:   const QModelIndex &index
//              QPainter* pPainter
// Author:      Sigal Algranaty
// Date:        20/6/2012
// ---------------------------------------------------------------------------
void acImageItemDelegate::drawTopLabel(const QModelIndex& index, QPainter* pPainter, const gtString& itemTopLabel, int& textBottom) const
{
    // Sanity check

    if (pPainter != NULL)
    {
        pPainter->save();
    }

    GT_IF_WITH_ASSERT((pPainter != NULL) && (m_pImageManager != NULL))
    {
        pPainter->restore();

        // Get the item rectangle:
        QRect itemRect = m_pImageManager->itemVisibleRect(index);
        QSize itemSize = itemRect.size();

        QPainterPath text;
        QFont font = pPainter->font();

        if (font.pointSize() > 0)
        {
            QFont originalFont = font;

            font.setPointSize(font.pointSize() + 1);
            font.setBold(true);
            pPainter->setFont(font);

            QRect topTextBoundingRect = QFontMetrics(font).boundingRect(itemTopLabel.asASCIICharArray());
            int textLeft = ((itemSize.width() - topTextBoundingRect.width()) / 2) + itemRect.topLeft().x();
            QPoint textPosition(textLeft, AC_IMAGE_MANAGER_TOP_MARGIN + itemRect.topLeft().y());

            // Draw the top label:
            QTextOption textOptions;
            textOptions.setAlignment(Qt::AlignTop | Qt::AlignHCenter);
            textOptions.setWrapMode(QTextOption::WordWrap);
            QRect textRect(QPoint(itemRect.x(), AC_IMAGE_MANAGER_TOP_MARGIN + itemRect.topLeft().y()), QSize(itemRect.width(), topTextBoundingRect.height()));
            pPainter->drawText(textRect, itemTopLabel.asASCIICharArray(), textOptions);

            // Output the text bottom coordinate:
            textBottom = textRect.bottom();

            // Restore the font:
            pPainter->setFont(originalFont);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        acImageItemDelegate::displayItemAsText
// Description: Display the requested item as text
// Arguments:   QPainter* pPainter
//              const QRect& imageRect
//              const acImageItem* pImageItem
// Author:      Sigal Algranaty
// Date:        21/6/2012
// ---------------------------------------------------------------------------
void acImageItemDelegate::displayItemAsText(QPainter* pPainter, const QRect& imageRect, const acImageItem* pImageItem) const
{
    // Sanity check
    GT_IF_WITH_ASSERT((pPainter != NULL) && (pImageItem != NULL) && (m_pImageManager != NULL))
    {
        // Get the view width:
        int viewWidth = m_pImageManager->size().width();

        // Get the strings:
        gtString headline = pImageItem->displayedTextHeadline();
        gtString message = pImageItem->displayedTextMessage();

        // Draw the title:
        QFont currentFont = pPainter->font();
        currentFont.setPointSize(12);
        currentFont.setBold(true);
        QRect titleBoundingRect = QFontMetrics(currentFont).boundingRect(headline.asASCIICharArray());
        pPainter->setFont(currentFont);

        QPen pen = pPainter->pen();
        pen.setColor(pImageItem->displayedTextColor());
        pPainter->setPen(pen);

        // Draw the title:
        QTextOption textOptions;
        textOptions.setAlignment(Qt::AlignTop | Qt::AlignLeft);
        textOptions.setWrapMode(QTextOption::WordWrap);
        QRect textRect(QPoint(imageRect.topLeft().x() + AC_IMAGE_MANAGER_TOP_MARGIN, imageRect.topLeft().y() + AC_IMAGE_MANAGER_TOP_MARGIN), QSize(viewWidth, titleBoundingRect.height()));
        pPainter->drawText(textRect, headline.asASCIICharArray(), textOptions);

        currentFont.setPointSize(10);
        currentFont.setBold(false);
        pPainter->setFont(currentFont);

        pen.setColor(Qt::black);
        pPainter->setPen(pen);

        // Draw the title:
        textRect.setTop(textRect.top() + titleBoundingRect.height() + AC_IMAGE_MANAGER_TOP_MARGIN);
        textRect.setBottom(textRect.bottom() + titleBoundingRect.height() + AC_IMAGE_MANAGER_TOP_MARGIN);
        pPainter->drawText(textRect, message.asASCIICharArray(), textOptions);
    }
}

// ---------------------------------------------------------------------------
// Name:        acImageItemDelegate::displayItemAsImage
// Description: Display an item image
// Arguments:   QPainter* pPainter
//              const QRect& imageRect
//              const acImageItem* pImageItem
// Author:      Sigal Algranaty
// Date:        25/6/2012
// ---------------------------------------------------------------------------
void acImageItemDelegate::displayItemAsImage(QPainter* pPainter, QRect& imageRect, acImageItem* pImageItem, const QSize& itemSize) const
{
    // Sanity check:
    GT_IF_WITH_ASSERT((pPainter != NULL) && (pImageItem != NULL) && (m_spBGBrush != NULL))
    {
        // Get the QImage for the item:
        QImage* pImage = pImageItem->asQImage();

        if (pImage != NULL)
        {
            // Define the available space for the image item:
            QRect imageAvailableBoundingRect = imageRect;

            // Check if image should be centered:
            int imageX = imageRect.left();
            int imageRectW = imageRect.width();
            int imageRealW = pImage->width();
            int imageXDiff = imageRectW - imageRealW;

            if (imageXDiff > 0)
            {
                imageX += imageXDiff / 2;
            }

            // That is the base position for the image Y coordinate:
            int imageY = imageRect.top();

            bool isSingleItem = ((m_pImageManager->m_managerMode == AC_MANAGER_MODE_STANDARD_ITEM) || (m_pImageManager->m_managerMode == AC_MANAGER_MODE_TEXT_ITEM));
            int imageMaxH = isSingleItem ? itemSize.height() : AC_IMAGES_MANAGER_THUMBNAIL_SIZE;

            // If the image is shorter, position it in the middle:
            int imageRealH = pImage->height();
            int imageHeightDiff = imageMaxH - imageRealH;

            if (imageHeightDiff > 0)
            {
                imageY += imageHeightDiff / 2;
            }

            // Get the painted image dimensions:
            int imageW = -1;
            int imageH = -1;

            if (m_bFirstTimeBestFit && (nullptr != m_pImageManager) && (m_pImageManager->managerMode() != AC_MANAGER_MODE_THUMBNAIL_VIEW))
            {
                int nZoomLevel = -1;
                int nOriginalWidth = pImage->width();
                int nOriginalHeight = pImage->height();
                int nAvailableWidth = imageRect.width();
                int nAvailableHeight = imageRect.height();
                GT_IF_WITH_ASSERT((0 != nOriginalWidth) && (0 != nOriginalHeight))
                {
                    //calculate necessary zoom level if first time best fit
                    int bestFitPercentageX = int((double(nAvailableWidth - AC_IMAGE_MANAGER_SCROLLBAR_MARGIN) * double(100) / double(nOriginalWidth)));
                    int bestFitPercentageY = int((double(nAvailableHeight - AC_IMAGE_MANAGER_SCROLLBAR_MARGIN) * double(100) / double(nOriginalHeight)));
                    nZoomLevel = min(bestFitPercentageX, bestFitPercentageY);
                    nZoomLevel -= AC_IMAGES_MANAGER_THUMBNAIL_MARGIN;
                    nZoomLevel = max(nZoomLevel, 1);
                    m_pImageManager->setFirstTimeBestFitZoom(nZoomLevel);
                    m_bFirstTimeBestFit = false;
                }
            }
            else
            {
                imageW = pImage->width();
                imageH = pImage->height();
                imageRect.setWidth(imageW);
                imageRect.setHeight(imageH);

                QSize imageDimensions(imageW, imageH);
                QPoint imageOrigin(0, 0);

                // Check if the image should be panned:
                if (m_pImageManager->activeTool() == AC_IMAGE_MANAGER_TOOL_PAN)
                {
                    // Set the image origin:
                    // imageOrigin = m_pImageManager->m_panPosition;

                    // Check if the image origin is in the scope of the image:
                }

                //
                QPoint imageOriginOnCanvas(imageX, imageY);

                // Draw the image border:
                QRect imageBorderRect(imageOriginOnCanvas.x() - 1, imageOriginOnCanvas.y() - 1, imageDimensions.width() + 2, imageDimensions.height() + 2);
                pPainter->fillRect(imageBorderRect, AC_IMAGE_MANAGER_IMAGE_BORDER_COLOR);

                QRect imageBGRect(imageOriginOnCanvas, imageDimensions);
                pPainter->fillRect(imageBGRect, AC_IMAGE_MANAGER_THUMBNAIL_BG_COLOR);
                pPainter->fillRect(imageBGRect, Qt::white);
                pPainter->fillRect(imageBGRect, *m_spBGBrush);

                // Draw the image:
                pPainter->drawImage(imageOriginOnCanvas.x(), imageOriginOnCanvas.y(), *pImage, imageOrigin.x(), imageOrigin.y(), imageDimensions.width(), imageDimensions.height());

                // Update the image item with the actual and available drawing rectangle of the image:
                pImageItem->setImageAvailableBoundingRect(imageAvailableBoundingRect);
                pImageItem->setImageBoundingRect(imageBGRect);
                imageRect = imageBGRect;
            }

        }
        else
        {
            // Draw the image border:
            QRect imageBorderRect(imageRect.left() - 1, imageRect.top() - 1, imageRect.width() + 2, imageRect.height() + 2);
            pPainter->fillRect(imageBorderRect, AC_IMAGE_MANAGER_IMAGE_BORDER_COLOR);
            pPainter->fillRect(imageRect, Qt::white);
            pPainter->drawText(imageRect, "Image is not available");
        }
    }
}

