//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acImageItemDelegate.h
///
//==================================================================================

//------------------------------ acImageItemDelegate.h ------------------------------

#ifndef __ACIMAGEITEMDELEGATE_H
#define __ACIMAGEITEMDELEGATE_H

// Qt:
#include <QStyledItemDelegate>

// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include//gtVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>


class acImageManager;
class acImageItem;
// ----------------------------------------------------------------------------------
// Class Name:          acImageItemDelegate : public QStyledItemDelegate
// General Description: This class is used for painting a single item in the images
//                      manager. The delegate class is used both for painting a single
//                      image item, and multiple image items in thumbnail view
// Author:              Sigal Algranaty
// Creation Date:       20/6/2012
// ----------------------------------------------------------------------------------
class acImageItemDelegate : public QStyledItemDelegate
{
public:

    // Constructor:
    acImageItemDelegate(acImageManager* pImageManager);

    // Destructor:
    ~acImageItemDelegate();

    // QItemDelegate overrides:
    virtual void paint(QPainter* pPainter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    void drawTopLabel(const QModelIndex& index, QPainter* pPainter, const gtString& itemTopLabel, int& textBottom) const;

    /// Resets the m_bFirstTimeBestFit flag to open the new image in best fit mode.
    void resetFirstTimeBestFit() { m_bFirstTimeBestFit = true; }

private:

    void drawItemImage(QPainter* pPainter, const QModelIndex& index, int textBottom, const QSize& itemSize, const QRect& itemBoundingRect, QRect& imageRect) const;
    void drawItemBottomLabel(QPainter* pPainter, const gtASCIIString& itemBottomLabel, const QRect& imageRect, const QRect& itemRect) const;

    void displayItemAsText(QPainter* pPainter, const QRect& imageRect, const acImageItem* pImageItem) const;
    void displayItemAsImage(QPainter* pPainter, QRect& imageRect, acImageItem* pImageItem, const QSize& itemSize) const;

private:

    // Parent images manager:
    acImageManager* m_pImageManager;

    // Background color:
    static QColor m_sBGColor;
    static bool m_sInitialized;
    static QBrush* m_spBGBrush;
    mutable bool m_bFirstTimeBestFit;
};


#endif //__ACIMAGEITEMDELEGATE_H

