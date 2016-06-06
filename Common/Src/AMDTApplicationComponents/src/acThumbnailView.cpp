//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acThumbnailView.cpp
///
//==================================================================================

//------------------------------ acDataView.cpp ------------------------------

#include <qtIgnoreCompilerWarnings.h>

#include <QLabel>

// Local:
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acThumbnailView.h>

#define AC_IMAGE_THUMBNAIL_MAX_X_Y_DIMENSION 192
#define AC_IMAGE_THUMBNAIL_MARGIN 15
#define AC_IMAGE_THUMBNAIL_INNER_MARGIN 5
#define AC_IMAGE_THUMBNAIL_OUTER_MARGIN 4

#define AC_SELECTED_ITEM_BG_COLOR QColor(238, 238, 238)
#define AC_SELECTED_ITEM_FRAME_COLOR QColor(125, 125, 125, 255)
#define AC_ITEM_TEXT_COLOR QColor(0, 0, 0, 255)

#define AC_Str_14DigitsString "88888888888888"

acThumbnailView::acThumbnailView() : QGraphicsView(),
    m_pGraphicScene(nullptr), m_pGraphicWidget(nullptr), m_pGraphicLayout(nullptr), m_currentColCount(0)
{
    m_pGraphicScene = new QGraphicsScene;

    m_pGraphicWidget = new QGraphicsWidget(nullptr);

    // Initialize the graphic layout
    m_pGraphicLayout = new QGraphicsGridLayout;
    m_pGraphicLayout->setColumnAlignment(0, Qt::AlignLeft);
    m_pGraphicLayout->setColumnAlignment(1, Qt::AlignLeft);
    m_pGraphicLayout->setColumnMinimumWidth(0, AC_IMAGE_THUMBNAIL_MAX_X_Y_DIMENSION + AC_IMAGE_THUMBNAIL_MARGIN);
    m_pGraphicLayout->setColumnMinimumWidth(1, AC_IMAGE_THUMBNAIL_MAX_X_Y_DIMENSION + AC_IMAGE_THUMBNAIL_MARGIN);
    m_pGraphicLayout->setContentsMargins(0, 0, 0, 0);
    m_pGraphicLayout->setSpacing(0);
    m_pGraphicLayout->setVerticalSpacing(0);
    m_pGraphicLayout->setHorizontalSpacing(0);

    m_pGraphicWidget->setLayout(m_pGraphicLayout);
    m_pGraphicScene->addItem(m_pGraphicWidget);

    setScene(m_pGraphicScene);

    setMinimumWidth(AC_IMAGE_THUMBNAIL_MAX_X_Y_DIMENSION * 2 + 4 * AC_IMAGE_THUMBNAIL_MARGIN);

    m_currentColCount = 2;
    m_nextAvailbleCell.setX(0);
    m_nextAvailbleCell.setY(0);

    setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_calculatedThumbnailWidth = 0;
    QFont font = m_pGraphicWidget->font();
    font.setPointSize(10);
    font.setWeight(QFont::DemiBold);

    m_calculatedThumbnailWidth = QFontMetrics(font).boundingRect(AC_Str_14DigitsString).width();
    m_calculatedThumbnailWidth += acScalePixelSizeToDisplayDPI(AC_IMAGE_THUMBNAIL_MAX_X_Y_DIMENSION + 4 * AC_IMAGE_THUMBNAIL_INNER_MARGIN);

    m_pGraphicScene->setBackgroundBrush(QColor::fromRgb(251, 251, 251));
    setStyleSheet("border:0px solid black;");

}

acThumbnailView::~acThumbnailView()
{
    foreach (Thumb t, m_thumbnailsVector)
    {
        delete t.m_pPixmap;
    }

    m_thumbnailsVector.clear();
}

void acThumbnailView::Update()
{
    // Calculate the column count according to the minimum count of item getting into the width
    int singleCellWidth = acScalePixelSizeToDisplayDPI(AC_IMAGE_THUMBNAIL_MAX_X_Y_DIMENSION * 2 + AC_IMAGE_THUMBNAIL_MARGIN * 4);
    m_currentColCount = width() / singleCellWidth;
    m_currentColCount = 2;
    BuildLayout();
}

void acThumbnailView::resizeEvent(QResizeEvent* pEvent)
{
    /*if (pEvent != nullptr)
    {
        // Calculate the column count according to the minimum count of item getting into the width
        int singleCellWidth = AC_IMAGE_THUMBNAIL_WIDTH * 2 + AC_IMAGE_THUMBNAIL_MARGIN * 4;

        int currentColCount = pEvent->size().width() / singleCellWidth;
        if (currentColCount > 2)
        {
            currentColCount = 2;
        }

        if (currentColCount != m_currentColCount)
        {
            // Build the layout according to the new column count
            BuildLayout();
        }
    }
    */

    QGraphicsView::resizeEvent(pEvent);
}

void acThumbnailView::AddThumbnail(const QString& imageFilePath, const QStringList& imageTextLines, const QVariant& itemUserId, bool shouldUpdate)
{
    // Create the pixmap
    Thumb t;
    QPixmap pixOrigSize(imageFilePath);

    if (pixOrigSize.width() > pixOrigSize.height())
    {
        t.m_pPixmap = new QPixmap(pixOrigSize.scaledToWidth(acScalePixelSizeToDisplayDPI(AC_IMAGE_THUMBNAIL_MAX_X_Y_DIMENSION)));
    }
    else
    {
        t.m_pPixmap = new QPixmap(pixOrigSize.scaledToHeight(acScalePixelSizeToDisplayDPI(AC_IMAGE_THUMBNAIL_MAX_X_Y_DIMENSION)));
    }

    t.m_textLines = imageTextLines;
    t.m_itemUserId = itemUserId;
    m_thumbnailsVector << t;

    if (shouldUpdate)
    {
        // Create a new item
        acThumbLayoutItem* pNewItem = new acThumbLayoutItem(t.m_pPixmap, t.m_textLines, itemUserId, this);
        pNewItem->setToolTip(m_itemTooltip);

        pNewItem->setAcceptHoverEvents(true);

        // Connect to the item's press and double click signals
        bool rc = connect(pNewItem, SIGNAL(ItemPressed(const QVariant&)), this, SLOT(OnItemPressed(const QVariant&)));
        GT_ASSERT(rc);
        rc = connect(pNewItem, SIGNAL(ItemDoubleClicked(const QVariant&)), this, SLOT(OnItemDoubleClicked(const QVariant&)));
        GT_ASSERT(rc);

        // Add the item to the layout and advance the next available cell coordinates
        m_pGraphicLayout->addItem(pNewItem, m_nextAvailbleCell.x(), m_nextAvailbleCell.y(), Qt::AlignLeft);
        AdvanceNextAvailableCell();
    }
}

void acThumbnailView::AddThumbnail(const unsigned char* pImageBuffer, unsigned long imageSize, const QStringList& textLines, const QVariant& itemUserId, bool shouldUpdate)
{
    GT_IF_WITH_ASSERT(pImageBuffer != nullptr)
    {
        // Create the pixmap
        Thumb t;
        QPixmap pixOrigSize;
        pixOrigSize.loadFromData((const uchar*)pImageBuffer, imageSize, "png");
        t.m_pPixmap = new QPixmap(pixOrigSize.scaledToWidth(acScalePixelSizeToDisplayDPI(AC_IMAGE_THUMBNAIL_MAX_X_Y_DIMENSION)));
        t.m_textLines = textLines;
        t.m_itemUserId = itemUserId;
        m_thumbnailsVector << t;

        if (shouldUpdate)
        {
            // Create a new item
            acThumbLayoutItem* pNewItem = new acThumbLayoutItem(t.m_pPixmap, t.m_textLines, itemUserId, this);
            pNewItem->setAcceptHoverEvents(true);
            pNewItem->setToolTip(m_itemTooltip);

            // Connect to the item's press and double click signals
            bool rc = connect(pNewItem, SIGNAL(ItemPressed(const QVariant&)), this, SLOT(OnItemPressed(const QVariant&)));
            GT_ASSERT(rc);
            rc = connect(pNewItem, SIGNAL(ItemDoubleClicked(const QVariant&)), this, SLOT(OnItemDoubleClicked(const QVariant&)));
            GT_ASSERT(rc);

            // Add the item to the layout and advance the next available cell coordinates
            m_pGraphicLayout->addItem(pNewItem, m_nextAvailbleCell.x(), m_nextAvailbleCell.y(), Qt::AlignLeft);
            AdvanceNextAvailableCell();
        }
    }
}

void acThumbnailView::BuildLayout()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pGraphicLayout != nullptr)
    {
        // Remove all items
        for (int i = 0; i < m_pGraphicLayout->count(); i++)
        {
            m_pGraphicLayout->removeItem(m_pGraphicLayout->itemAt(0));
        }

        m_nextAvailbleCell.setX(0);
        m_nextAvailbleCell.setY(0);

        int singleCellWidth = acScalePixelSizeToDisplayDPI(AC_IMAGE_THUMBNAIL_MAX_X_Y_DIMENSION * 2 + AC_IMAGE_THUMBNAIL_MARGIN * 4);

        for (int i = 0; i < m_currentColCount; i++)
        {
            m_pGraphicLayout->setColumnFixedWidth(i, singleCellWidth);
        }

        foreach (Thumb t, m_thumbnailsVector)
        {
            acThumbLayoutItem* pNewItem1 = new acThumbLayoutItem(t.m_pPixmap, t.m_textLines, t.m_itemUserId, this);
            pNewItem1->setToolTip(m_itemTooltip);

            m_pGraphicLayout->addItem(pNewItem1, m_nextAvailbleCell.x(), m_nextAvailbleCell.y(), Qt::AlignLeft);

            AdvanceNextAvailableCell();

        }

        m_pGraphicLayout->updateGeometry();
    }
}

void acThumbnailView::AdvanceNextAvailableCell()
{
    if (m_nextAvailbleCell.y() + 1 >= m_currentColCount)
    {
        m_pGraphicLayout->setRowAlignment(m_nextAvailbleCell.x(), Qt::AlignTop | Qt::AlignLeft);
        m_nextAvailbleCell.setX(m_nextAvailbleCell.x() + 1);
        m_nextAvailbleCell.setY(0);
    }
    else
    {
        m_nextAvailbleCell.setY(m_nextAvailbleCell.y() + 1);
    }
}

void acThumbnailView::SetItemTooltip(const QString& itemTooltip)
{
    m_itemTooltip = itemTooltip;
}

int acThumbnailView::ItemsCount()
{
    int retVal = 0;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pGraphicLayout != nullptr)
    {
        retVal = m_pGraphicLayout->count();
    }
    return retVal;
}

void acThumbnailView::OnItemPressed(QVariant itemId)
{
    GT_IF_WITH_ASSERT(m_pGraphicLayout != nullptr)
    {
        for (int i = 0; i < m_pGraphicLayout->count(); i++)
        {
            acThumbLayoutItem* pItem = (acThumbLayoutItem*)(m_pGraphicLayout->itemAt(i));

            if (pItem != nullptr)
            {
                bool shouldSelect = (pItem->UserID() == itemId);

                // Select only the pressed item
                pItem->SetSelected(shouldSelect);
            }
        }
    }

    m_pGraphicLayout->updateGeometry();
    repaint();

    // Emit the signal
    emit ItemPressed(itemId);
}

void acThumbnailView::SetSelected(int thumbIndex, bool shouldSelect)
{
    GT_IF_WITH_ASSERT(m_pGraphicLayout != nullptr && m_pGraphicLayout->count() > thumbIndex)
    {
        acThumbLayoutItem* pItem = (acThumbLayoutItem*)(m_pGraphicLayout->itemAt(thumbIndex));

        if (pItem != nullptr)
        {
            pItem->SetSelected(shouldSelect);

            emit ItemPressed(pItem->UserID());
        }
    }
}

bool acThumbnailView::GetSelected(QVariant& selectedItemUserID)
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(m_pGraphicLayout != nullptr)
    {
        for (int i = 0; i < m_pGraphicLayout->count(); i++)
        {
            acThumbLayoutItem* pItem = (acThumbLayoutItem*)(m_pGraphicLayout->itemAt(i));

            if (pItem != nullptr)
            {
                if (pItem->IsSelected())
                {
                    retVal = true;
                    selectedItemUserID = pItem->UserID();
                    break;
                }
            }
        }
    }
    return retVal;
}

void acThumbnailView::keyPressEvent(QKeyEvent* pEvent)
{
    GT_IF_WITH_ASSERT(m_pGraphicLayout != nullptr && pEvent != nullptr)
    {
        int selectedIndex = GetSelectedItemIndex();
        int nextSelectionIndex = -1;

        if (selectedIndex > -1)
        {
            switch (pEvent->key())
            {
                case Qt::RightArrow:
                case Qt::Key_Right:
                    nextSelectionIndex = selectedIndex + 1;
                    break;

                case Qt::LeftArrow:
                case Qt::Key_Left:
                    nextSelectionIndex = selectedIndex - 1;
                    break;

                case Qt::DownArrow:
                case Qt::Key_Down:
                    nextSelectionIndex = selectedIndex + m_currentColCount;
                    break;

                case Qt::UpArrow:
                case Qt::Key_Up:
                    nextSelectionIndex = selectedIndex - m_currentColCount;
                    break;

                case Qt::Key_Enter:
                    acThumbLayoutItem* pItem = (acThumbLayoutItem*)(m_pGraphicLayout->itemAt(selectedIndex));

                    if (pItem != nullptr)
                    {
                        QVariant varItem = pItem->UserID();
                        emit ItemDoubleClicked(varItem);
                    }

                    break;
            }

            if (nextSelectionIndex > -1 && nextSelectionIndex <= m_pGraphicLayout->count())
            {
                acThumbLayoutItem* pCurrItem = (acThumbLayoutItem*)(m_pGraphicLayout->itemAt(selectedIndex));

                if (pCurrItem)
                {
                    pCurrItem->SetSelected(false);

                }

                acThumbLayoutItem* pNextItem = (acThumbLayoutItem*)(m_pGraphicLayout->itemAt(nextSelectionIndex));

                if (pNextItem)
                {
                    pNextItem->SetSelected(true);
                }
            }
        }
    }
    repaint();
}

int acThumbnailView::GetSelectedItemIndex()const
{
    GT_IF_WITH_ASSERT(m_pGraphicLayout != nullptr)
    {
        int itemCount = m_pGraphicLayout->count();

        for (int i = 0; i < itemCount; i++)
        {
            acThumbLayoutItem* pItem = (acThumbLayoutItem*)(m_pGraphicLayout->itemAt(i));

            if (pItem != nullptr && pItem->IsSelected())
            {
                return i;
            }
        }
    }
    return - 1;
}

//////////////////////////////////////////////////////////////////////////////////////////// acBaseLayoutItem ////////////////////////////////////////////////////////////////////////////////////////////
acBaseLayoutItem::acBaseLayoutItem(QVariant userId, acThumbnailView* pParentView) : QGraphicsLayoutItem(), QGraphicsItem(nullptr),
    m_userId(userId), m_isSelected(false), m_pParentView(pParentView)
{
    setGraphicsItem(this);
}

acBaseLayoutItem::~acBaseLayoutItem()
{

}

void acBaseLayoutItem::paint(QPainter* pPainter, const QStyleOptionGraphicsItem* pOption, QWidget* pWidget)
{
    Q_UNUSED(pWidget);
    Q_UNUSED(pOption);
    Q_UNUSED(pPainter);

    GT_ASSERT(false);
}

void acBaseLayoutItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* pEvent)
{
    QGraphicsItem::mouseDoubleClickEvent(pEvent);

    emit ItemDoubleClicked(m_userId);
}

void acBaseLayoutItem::mouseMoveEvent(QGraphicsSceneMouseEvent* pEvent)
{
    QGraphicsItem::mouseMoveEvent(pEvent);
}

void acBaseLayoutItem::mousePressEvent(QGraphicsSceneMouseEvent* pEvent)
{
    QGraphicsItem::mousePressEvent(pEvent);
    emit ItemPressed(m_userId);
}

QRectF acBaseLayoutItem::boundingRect() const
{
    return QRectF(QPointF(0, 0), geometry().size());
}

void acBaseLayoutItem::setGeometry(const QRectF& geom)
{
    prepareGeometryChange();
    QGraphicsLayoutItem::setGeometry(geom);
    setPos(geom.topLeft());
}

QSizeF acBaseLayoutItem::sizeHint(Qt::SizeHint which, const QSizeF& constraint) const
{
    Q_UNUSED(which);
    QSizeF retVal = constraint;
    return retVal;
}

acThumbLayoutItem::acThumbLayoutItem(QPixmap* pPixmap, const QStringList& textLines, const QVariant& itemUserId, acThumbnailView* pParentView)
    : acBaseLayoutItem(itemUserId, pParentView)
{
    m_pPixmap = pPixmap;
    m_textLines = textLines;
}

acThumbLayoutItem::~acThumbLayoutItem()
{
}

void acThumbLayoutItem::paint(QPainter* pPainter, const QStyleOptionGraphicsItem* pOption, QWidget* pWidget)
{
    Q_UNUSED(pWidget);
    Q_UNUSED(pOption);

    PaintBG(pPainter);

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pPixmap != nullptr)
    {
        QRectF frame(QPointF(AC_IMAGE_THUMBNAIL_OUTER_MARGIN, 0), geometry().size());

        // Draw the pixmap
        QPointF pixpos(frame.left() + AC_IMAGE_THUMBNAIL_INNER_MARGIN, frame.top() + AC_IMAGE_THUMBNAIL_INNER_MARGIN);
        pPainter->drawPixmap(pixpos, *m_pPixmap);

        QPointF frameRectLeft = pixpos;
        frameRectLeft.setX(frameRectLeft.x() - 1);
        frameRectLeft.setY(frameRectLeft.y() - 1);
        QRectF frameRect = QRectF(frameRectLeft, QSizeF(m_pPixmap->width() + 1, m_pPixmap->height() + 1));
        pPainter->setPen(QPen(AC_ITEM_TEXT_COLOR));
        pPainter->drawRect(frameRect);

        QFont font = pPainter->font();
        font.setPointSize(10);
        font.setWeight(QFont::DemiBold);
        pPainter->setFont(font);

        QPointF textpos = pixpos;
        textpos.setX(pixpos.x() + m_pPixmap->width() + acScalePixelSizeToDisplayDPI(AC_IMAGE_THUMBNAIL_MARGIN));
        textpos.setY(pixpos.y() + acScalePixelSizeToDisplayDPI(AC_IMAGE_THUMBNAIL_MARGIN));

        // Draw the lines. The first line will be bold and will have bigger font size
        foreach (QString line, m_textLines)
        {
            pPainter->drawText(textpos, line);
            font.setPointSize(8);
            font.setWeight(QFont::Normal);
            pPainter->setFont(font);
            textpos.setY(textpos.y() + acScalePixelSizeToDisplayDPI(15));
        }
    }
}

void acThumbLayoutItem::PaintBG(QPainter* pPainter)
{
    GT_IF_WITH_ASSERT(pPainter != nullptr)
    {
        // Paint only selected items
        if (m_isSelected)
        {
            QRectF frame(QPointF(0, 0), geometry().size());
            pPainter->setPen(QPen(AC_SELECTED_ITEM_FRAME_COLOR));
            pPainter->fillRect(frame, QBrush(AC_SELECTED_ITEM_BG_COLOR));
        }
    }
}

QSizeF acThumbLayoutItem::sizeHint(Qt::SizeHint which, const QSizeF& constraint) const
{

    QSizeF retVal;

    switch (which)
    {
        case Qt::MinimumSize:
        case Qt::PreferredSize:
        case Qt::MaximumSize:
        {
            int height = acScalePixelSizeToDisplayDPI(AC_IMAGE_THUMBNAIL_MAX_X_Y_DIMENSION + 2 * AC_IMAGE_THUMBNAIL_MARGIN + AC_IMAGE_THUMBNAIL_OUTER_MARGIN);
            int width = acScalePixelSizeToDisplayDPI(AC_IMAGE_THUMBNAIL_MAX_X_Y_DIMENSION * 2 + 4 * AC_IMAGE_THUMBNAIL_MARGIN + AC_IMAGE_THUMBNAIL_OUTER_MARGIN);

            // Get the calculated width from the parent view
            GT_IF_WITH_ASSERT(m_pParentView != nullptr)
            {
                width = m_pParentView->ThumbnailWidth() + acScalePixelSizeToDisplayDPI(AC_IMAGE_THUMBNAIL_OUTER_MARGIN);
            }

            GT_IF_WITH_ASSERT(m_pPixmap != nullptr)
            {
                height = m_pPixmap->height() + acScalePixelSizeToDisplayDPI(2 * AC_IMAGE_THUMBNAIL_MARGIN + AC_IMAGE_THUMBNAIL_OUTER_MARGIN);
            }

            width += acScalePixelSizeToDisplayDPI(2 * AC_IMAGE_THUMBNAIL_MARGIN);
            retVal.setWidth(width);
            retVal.setHeight(height);
        }
        break;

        default:
            retVal = constraint;
            break;
    }

    return retVal;
}
