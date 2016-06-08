//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acThumbnailView.h
///
//==================================================================================

//------------------------------ acThumbnailView.h ------------------------------

#ifndef __ACTHUMBNAILVIEW
#define __ACTHUMBNAILVIEW

// Qt:
#include <QGraphicsView>
#include <QGraphicsLayoutItem>
#include <QGraphicsItem>
#include <QGraphicsGridLayout>
#include <QGraphicsWidget>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

/// This class is used for painting a Qt thumbnail view.
/// thumbnails are painted with an adjacent text column (on the right)
class AC_API acThumbnailView : public QGraphicsView
{
    Q_OBJECT

public:
    /// Constructor
    acThumbnailView();

    /// Destructor:
    ~acThumbnailView();

    void Update();

protected:

    /// Overriding QGraphicsView to re-layout if column count should change
    void resizeEvent(QResizeEvent* pEvent);

public:

    /// Add a thumbnail to the view
    /// \param imageFilePath the file path with the thumbnail location (currently supporting only bmp)
    /// \param imageTextLines the text describing the image item, separated into lines
    void AddThumbnail(const QString& imageFilePath, const QStringList& imageTextLines, const QVariant& itemUserId, bool shouldUpdate = true);

    /// Add a thumbnail to the view
    /// \param pImageBuffer a buffer with the image data (currently expecting a bmp format)
    /// \param imageSize the buffer size
    /// \param imageTextLines the text describing the image item, separated into lines
    /// \param itemUserId is used for identifying the item. The id will be returned in the signals
    /// \param shouldUpdate should the thumbnail be displayed when added, or only later
    void AddThumbnail(const unsigned char* pImageBuffer, unsigned long imageSize, const QStringList& imageTextLines, const QVariant& itemUserId, bool shouldUpdate = true);

    /// Build the thumbnails layout
    void BuildLayout();

    /// Select / unselect a thumbnail item
    /// \param thumbIndex the thumbnail index
    /// \param shouldSelect should the item be selected?
    void SetSelected(int thumbIndex, bool shouldSelect);

    /// Get the currently selected item
    /// \param selectedItemUserID the user id for the selected item
    /// \return true if there is a selected item
    bool GetSelected(QVariant& selectedItemUserID);

    /// Return the current thumbnail width
    int ThumbnailWidth() const { return m_calculatedThumbnailWidth; };

    /// Set the tooltip displayed for the items in the view
    void SetItemTooltip(const QString& itemTooltip);

    /// Return the amount of thumbnails
    int ItemsCount();

protected:
    virtual void keyPressEvent(QKeyEvent* e);

signals:

    /// Is used to signal the users that an item was pressed
    void ItemPressed(const QVariant& itemId);

    /// Is used to signal the users that an item was double clicked
    void ItemDoubleClicked(const QVariant& itemId);

protected slots:

    /// Is handling the ItemPressed signal
    void OnItemPressed(QVariant itemId);

    /// Is handling the ItemDoubleClicked signal
    void OnItemDoubleClicked(QVariant itemId) { emit ItemDoubleClicked(itemId); };

private:

    /// Advance the next available cell coordinates
    void AdvanceNextAvailableCell();

    int GetSelectedItemIndex()const;
private:

    /// The graphic scene
    QGraphicsScene* m_pGraphicScene;

    /// The graphic widget (used for layouting the graphic object)
    QGraphicsWidget* m_pGraphicWidget;

    /// The graphic layout
    QGraphicsGridLayout* m_pGraphicLayout;

    struct Thumb
    {
        QPixmap* m_pPixmap;
        QStringList m_textLines;
        QVariant m_itemUserId;
    };
    QVector<Thumb> m_thumbnailsVector;

    /// Contain the column count (is calculated according to the view width)
    int m_currentColCount;

    /// Contain the next available cell coordinates in the grid layout
    QPoint m_nextAvailbleCell;

    /// A number containing the calculated thumbnail width
    int m_calculatedThumbnailWidth;

    /// Text for the item tooltip
    QString m_itemTooltip;
};


class acBaseLayoutItem : public QObject, public QGraphicsLayoutItem, public QGraphicsItem
{
    Q_OBJECT
public:

    /// Constructor
    acBaseLayoutItem(QVariant userId, acThumbnailView* pParentView);

    /// Destructor
    ~acBaseLayoutItem();

    // Inherited from QGraphicsLayoutItem
    void setGeometry(const QRectF& geom);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF& constraint = QSizeF()) const;

    // Inherited from QGraphicsItem
    QRectF boundingRect() const;

    /// Overriding QGraphicsItem
    void paint(QPainter* pPainter, const QStyleOptionGraphicsItem* pOption, QWidget* pWidget);

    /// Accessor
    const QVariant& UserID() const { return m_userId; }

    /// Set the item selected flag
    void SetSelected(bool isSelected) { m_isSelected = isSelected; }

    /// Get the item selected flag
    bool IsSelected() const { return m_isSelected; }
signals:

    /// Item with itemId was double clicked
    void ItemDoubleClicked(const QVariant& itemId);

    /// Item with itemId was pressed
    void ItemPressed(const QVariant& itemId);


protected:

    /// Overriding QGraphicsItem event
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* pEvent);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* pEvent);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* pEvent);


    /// Input user id (will be used to identify items in signals)
    QVariant m_userId;

    /// Is the item selected
    bool m_isSelected;

    /// The parent view
    acThumbnailView* m_pParentView;
};

class acThumbLayoutItem : public acBaseLayoutItem
{
    Q_OBJECT
public:

    /// Constructor
    acThumbLayoutItem(QPixmap* pPixmap, const QStringList& textLines, const QVariant& itemUserId, acThumbnailView* pParentView);

    /// Destructor
    ~acThumbLayoutItem();

    // Inherited from acPixLayoutItem
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF& constraint = QSizeF()) const;
    void paint(QPainter* pPainter, const QStyleOptionGraphicsItem* pOption, QWidget* pWidget);

private:

    /// Paint the item background
    void PaintBG(QPainter* pPainter);

private:

    /// Thumbnail image as QPixmap
    QPixmap* m_pPixmap;

    /// The thumbnail text lines
    QStringList m_textLines;

};

#endif  // __ACTHUMBNAILVIEW
