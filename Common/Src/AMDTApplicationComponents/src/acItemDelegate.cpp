//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acItemDelegate.cpp
///
//==================================================================================

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <QPainter>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/Include/acTreeCtrl.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osProcess.h>

/// Local:
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>
#include <AMDTApplicationComponents/Include/acUserRoles.h>

#define AC_MAX_BAR_WIDTH 150

acItemDelegate::acItemDelegate(QObject* pParent) : QItemDelegate(pParent), m_lineHeight((int)acScalePixelSizeToDisplayDPI(AC_DEFAULT_LINE_HEIGHT))
{
};

QSize acItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    // Get the base class size hint:
    QSize retVal = QItemDelegate::sizeHint(option, index);

    retVal.setHeight(m_lineHeight);

    return retVal;
}

acPercentItemDelegate::acPercentItemDelegate(QObject* pParent) : acItemDelegate(pParent),
    m_shouldSimulateFocus(false), m_shouldDrawContent(true), m_isItemSelected(false), m_hasFocus(false), m_shouldPaintPercent(true),
    m_pOwnerTree(NULL), m_pOwnerTable(NULL), m_pOwnerFrozenTreeView(NULL)
{
}

void acPercentItemDelegate::paint(QPainter* pPainter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    acPercentItemDelegate* pNonConstDelegate = (acPercentItemDelegate*)this;
    pNonConstDelegate->PaintNonConst(pPainter, option, index);
}

void acPercentItemDelegate::CheckIfItemIsSelected(const QStyleOptionViewItem& option, const QModelIndex& index)
{
    Q_UNUSED(option);

    m_isItemSelected = false;

    if (m_pOwnerTable != NULL)
    {
        m_hasFocus = m_pOwnerTable->hasFocus();

        // If this is a table:
        QTableWidgetItem* pItem = m_pOwnerTable->item(index.row(), index.column());

        if (pItem != NULL)
        {
            m_isItemSelected = pItem->isSelected();
        }
    }
    else if (m_pOwnerTree != NULL)
    {
        if (m_pOwnerTree != NULL)
        {
            m_isItemSelected = m_pOwnerTree->isItemSelected(index, m_hasFocus);
        }
    }
    else if (m_pOwnerFrozenTreeView != NULL)
    {
        if (m_pOwnerFrozenTreeView != NULL)
        {
            m_isItemSelected = m_pOwnerFrozenTreeView->isItemSelected(index, m_hasFocus);
        }
    }

    if (m_shouldSimulateFocus)
    {
        m_hasFocus = true;
    }
}

void acPercentItemDelegate::PaintNonConst(QPainter* pPainter, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    // Check if this item is selected:
    CheckIfItemIsSelected(option, index);

    // Override the selected flag in the options:
    QStyleOptionViewItem newOptions = option;

    if (m_isItemSelected && m_hasFocus)
    {
        newOptions.state = QStyle::State_Selected;
    }

    bool shouldDrawContent = !m_shouldPaintPercent && m_shouldDrawContent;

    if (shouldDrawContent)
    {
        // Paint the item:
        QItemDelegate::paint(pPainter, newOptions, index);
    }
    else
    {
        drawBackground(pPainter, newOptions, index);
    }
}


acTablePercentItemDelegate::acTablePercentItemDelegate(QObject* pParent) : acPercentItemDelegate(pParent)
{

}

void acTablePercentItemDelegate::paint(QPainter* pPainter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // Call the base class implementation:
    acPercentItemDelegate::paint(pPainter, option, index);

    // Paint the item as percent:
    PaintPercentItem(index, pPainter, option);
}

void acPercentItemDelegate::ComputeBarRectangleTextInside(const QModelIndex& index, const QStyleOptionViewItem& option, QPainter* pPainter, QRect& coloredBarRect) const
{
    // Get the data as double (we expect to find a data in the range of 00. to 1.0:
    double pct = index.data().toDouble() / (double) 100.0;

    // Calculate the cell coordinates and text width:
    int xp1 = 0, yp1 = 0, xp2 = 0, yp2 = 0, textWidth = 0;

    // The colored bar rectangle:
    coloredBarRect = option.rect;

    // Text bounding box rectangle:
    QRect textBoundingBox;

    // Find out the size of the bounding box of the text
    textBoundingBox = pPainter->boundingRect(QRect(0, 0, 100, 100), Qt::AlignLeft | Qt::AlignTop, QString(" 100%"));
    textWidth = 0;

    // Get coordinates of this cell:
    coloredBarRect.getCoords(&xp1, &yp1, &xp2, &yp2);
    int w = xp2 - xp1 + 1;

    // Calculate the extent of the outline (leave 1 pixel from edge of control):
    xp1++;
    yp1++;
    xp2 -= textWidth;
    yp2--;


    // The width of the bar should not exceed the specified constant:
    int fullPercendWidth = qMin((w - textWidth), AC_MAX_BAR_WIDTH);

    // Compute the width of the colored bar:
    w = fullPercendWidth * pct;

    xp2 = xp1 + w - 1;

    // xp1 += textWidth + 10;
    //xp2 += textWidth + 10;

    // Set the rectangle for the colored bar:
    coloredBarRect.setCoords(xp1, yp1, xp2, yp2);
}

void acPercentItemDelegate::PaintPercentItem(const QModelIndex& index, QPainter* pPainter, const QStyleOptionViewItem& option) const
{
    QColor color = acGetSystemDefaultBackgroundColor();

    // We expect only double values to have this delegate:
    if ((-1 != index.data().toDouble()) && (pPainter != NULL) && m_shouldPaintPercent)
    {
        if (index.data().toDouble() > 0)
        {
            // Compute the size and coordinates of the colored bar rectangle:
            QRect coloredBarRect(0, 0, 0, 0);
            ComputeBarRectangleTextInside(index, option, pPainter, coloredBarRect);

            QFont font;
            font.setStyleStrategy(QFont::ForceOutline);

            if (m_shouldPaintPercent)
            {
                if (coloredBarRect.width() > 2)
                {
                    if (m_isItemSelected && m_hasFocus)
                    {
                        color = QColor::fromRgb(color.red() * 6 / 10, color.green() * 6 / 10, color.blue() * 6 / 10);
                        acReadableNegativeColor(color);
                    }

                    pPainter->fillRect(coloredBarRect, color);
                    pPainter->setPen(QColor::fromRgb(color.red() * 8 / 10, color.green() * 8 / 10, color.blue() * 8 / 10));
                    pPainter->drawRect(coloredBarRect);
                }
            }

            // Draw the text, the filled bar, and its outline:
            QColor textColor;
            QVariant qVar = index.data(Qt::TextColorRole);

            // if color data role exist - take the original color
            if (qVar.canConvert<QColor>() && !m_isItemSelected)
            {
                textColor = qvariant_cast<QColor>(qVar);
            }
            else
            {
                textColor = option.palette.text().color();
            }

            if (m_isItemSelected && m_hasFocus)
            {
                acReadableNegativeColor(textColor);
            }

            pPainter->setPen(textColor);
            QString percentText;
            QRect textRect = coloredBarRect;
            textRect.setLeft(coloredBarRect.left() + 3);
            textRect.setTop(coloredBarRect.top() + 1);
            textRect.setWidth(option.rect.width());

            // Always draw percentage with 2 percision:
            percentText = QString::number(index.data().toFloat(), 'f', 2);
            percentText.append("%");
            pPainter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, percentText);
        }
    }
    else
    {
        QString text = index.data().toString();
        pPainter->drawText(option.rect, Qt::AlignLeft | Qt::AlignVCenter, text);
    }
}

QColor acTreeItemDeletate::m_sDefaultTextColor = Qt::black;
bool acTreeItemDeletate::m_sDefaultTextColorInitialized = false;


acTreeItemDeletate::acTreeItemDeletate(QObject* pParent) : acPercentItemDelegate(pParent)
{

}

void acTreeItemDeletate::paint(QPainter* pPainter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    bool shouldPaintPercent = false;

    // Call the base class implementation:
    acPercentItemDelegate::paint(pPainter, option, index);

    // Check if the item is selection:
    acPercentItemDelegate* pNonConstDelegate = (acPercentItemDelegate*)this;
    pNonConstDelegate->CheckIfItemIsSelected(option, index);

    if (index.isValid())
    {
        shouldPaintPercent = (m_percentDelegateColumnsList.indexOf(index.column()) >= 0);
    }

    if (shouldPaintPercent)
    {
        QStyleOptionViewItem newOption = option;

        // Save the painter settings:
        pPainter->save();

        if (!m_sDefaultTextColorInitialized)
        {
            m_sDefaultTextColor = option.palette.brush(QPalette::Text).color();
            m_sDefaultTextColorInitialized = true;
        }

        // Set the color for the percent item:
        if (m_percentDelegateIndexToColorsMap.contains(index.column()))
        {
            QColor c = m_percentDelegateIndexToColorsMap[index.column()];
            newOption.palette.setBrush(QPalette::Text, QBrush(c));
        }
        else
        {
            newOption.palette.setBrush(QPalette::Text, QBrush(m_sDefaultTextColor));
        }

        // Paint the item as percent:
        PaintPercentItem(index, pPainter, newOption);

        // Save the painter settings:
        pPainter->restore();
    }
    else
    {
        bool isFloat = false;
        float floatVal = index.data().toFloat(&isFloat);

        if (isFloat)
        {
            int precision = acNumberDelegateItem::m_sFloatingPointPercision;

            if (fmod(floatVal, (float)1.0) == 0.0)
            {
                precision = 0;
            }

            QString text = QLocale(QLocale::English).toString(floatVal, 'f', precision);
            pPainter->drawText(option.rect, Qt::AlignLeft | Qt::AlignVCenter, text);
        }
        else
        {
            // Call the default implementation of paint:
            QItemDelegate::paint(pPainter, option, index);
        }
    }
}

void acTreeItemDeletate::SetPercentColumnsList(const QList<int>& percentDelegateColumnsList)
{
    m_percentDelegateColumnsList.clear();
    m_percentDelegateColumnsList = percentDelegateColumnsList;

    m_percentDelegateIndexToColorsMap.clear();
}

void acTreeItemDeletate::SetPercentForgroundColor(int columnIndex, const QColor& c)
{
    m_percentDelegateIndexToColorsMap[columnIndex] = c;
}

// Static member initializations:
acNumberDelegateItem* acNumberDelegateItem::m_psMySingleInstance = NULL;
int acNumberDelegateItem::m_sFloatingPointPercision = 2;

acNumberDelegateItem& acNumberDelegateItem::Instance()
{
    if (m_psMySingleInstance == NULL)
    {
        m_psMySingleInstance = new acNumberDelegateItem;
        GT_ASSERT(m_psMySingleInstance);
    }

    return *m_psMySingleInstance;
}

QString acNumberDelegateItem::displayText(const QVariant& value, const QLocale& locale) const
{
    GT_UNREFERENCED_PARAMETER(locale);

    QString retVal;
    bool isFloat = false;
    float floatVal = value.toFloat(&isFloat);

    if (isFloat)
    {
        int precision = acNumberDelegateItem::m_sFloatingPointPercision;

        if (fmod(floatVal, (float)1.0) == 0.0)
        {
            precision = 0;
        }

        retVal = QLocale(QLocale::English).toString(floatVal, 'f', precision);
    }

    else
    {
        retVal = value.toString();
    }

    return retVal;
}

void acNumberDelegateItem::SetFloatingPointPercision(int percision)
{
    m_sFloatingPointPercision = percision;
}
