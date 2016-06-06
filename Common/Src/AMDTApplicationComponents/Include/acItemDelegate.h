//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acItemDelegate.h
///
//==================================================================================

#ifndef __ACITEMDELEGATE_H
#define __ACITEMDELEGATE_H

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QItemDelegate>
#include <QStyledItemDelegate>

// Local:
#include <AMDTApplicationComponents/Include/acFrozenColumnTreeView.h>

class acTreeCtrl;
class acListCtrl;


/// -----------------------------------------------------------------------------------------------
/// \class Name: acItemDelegate : public QItemDelegate
/// \brief Description:  Delegate item that can be used for only set the selection format of the
///                      tree / table (a solid color selection)
/// -----------------------------------------------------------------------------------------------
class AC_API acItemDelegate : public QItemDelegate
{
public:
    acItemDelegate(QObject* pParent = NULL);
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

protected:
    int m_lineHeight;
};


class AC_API acPercentItemDelegate : public acItemDelegate
{
public:
    acPercentItemDelegate(QObject* pParent = NULL);
    void paint(QPainter* pPainter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    void PaintNonConst(QPainter* pPainter, const QStyleOptionViewItem& option, const QModelIndex& index);

    void SetSimulateFocus(bool shouldSimulateFocus) {m_shouldSimulateFocus = shouldSimulateFocus;}

    void SetOwnerTree(acTreeCtrl* pTree) {m_pOwnerTree = pTree;}
    void SetOwnerFrozenTree(acFrozenColumnTreeView* pTreeView) {m_pOwnerFrozenTreeView = pTreeView;}
    void SetOwnerTable(acListCtrl* pOwnerTable) {m_pOwnerTable = pOwnerTable;}
    void CheckIfItemIsSelected(const QStyleOptionViewItem& option, const QModelIndex& index) ;
    void SetPercent(bool shouldPaintPercent) {m_shouldPaintPercent = shouldPaintPercent;}
    void SetShouldDrawContent(bool shouldDrawContent) {m_shouldDrawContent = shouldDrawContent;}

protected:
    void ComputeBarRectangleTextInside(const QModelIndex& index, const QStyleOptionViewItem& option, QPainter* pPainter, QRect& coloredBarRect) const;
    void PaintPercentItem(const QModelIndex& index, QPainter* pPainter, const QStyleOptionViewItem& option) const;

protected:
    bool m_shouldSimulateFocus;
    bool m_shouldDrawContent;
    bool m_isItemSelected;
    bool m_hasFocus;
    bool m_shouldPaintPercent;

    acTreeCtrl* m_pOwnerTree;
    acListCtrl* m_pOwnerTable;
    acFrozenColumnTreeView* m_pOwnerFrozenTreeView;
};

class AC_API acTablePercentItemDelegate: public acPercentItemDelegate
{
public:
    acTablePercentItemDelegate(QObject* pParent = 0);
    void paint(QPainter* pPainter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

class AC_API acTreeItemDeletate : public acPercentItemDelegate
{
public:
    acTreeItemDeletate(QObject* pParent = NULL);
    void paint(QPainter* pPainter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    void SetPercentColumnsList(const QList<int>& percentDelegateColumnsList);
    void SetPercentForgroundColor(int columnIndex, const QColor& c);

private:
    /// Contain the list of columns that should be painted as percent:
    QList<int> m_percentDelegateColumnsList;
    QMap<int, QColor> m_percentDelegateIndexToColorsMap;

    static bool m_sDefaultTextColorInitialized;
    static QColor m_sDefaultTextColor;
};

class AC_API acNumberDelegateItem : public QStyledItemDelegate
{
    friend class acTreeItemDeletate;

public:
    // Singleton:
    static acNumberDelegateItem& Instance();

    virtual QString displayText(const QVariant& value, const QLocale& locale) const;

    /// Sets the static member responsible for the floating point percision:
    static void SetFloatingPointPercision(int percision);
    static int FloatingPointPercision() { return m_sFloatingPointPercision; };

protected:
    // My single instance:
    static acNumberDelegateItem* m_psMySingleInstance;
    static int m_sFloatingPointPercision;
};


#endif //__ACITEMDELEGATE_H

