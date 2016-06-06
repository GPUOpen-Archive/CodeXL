//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acQTextFilterCtrl.h
///
//==================================================================================

//------------------------------ acQTextFilterCtrl.h ------------------------------

#ifndef __ACQTEXTFILTERCTRL
#define __ACQTEXTFILTERCTRL

// Qt:
#include <QWidget>

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTApplicationComponents/Include/acListCtrl.h>

// Forward declarations:
class acListCtrl;
class QSortFilterProxyModel;
class QTableWidgetItem;

// ----------------------------------------------------------------------------------
// Class Name:          acQTextFilterCtrl : public QLineEdit
// General Description: A filter text control that filters list control items.
//
// Author:              Yoni Rabin
// Creation Date:       24/4/2012
// ----------------------------------------------------------------------------------
class AC_API acQTextFilterCtrl : public QLineEdit
{
    Q_OBJECT
public:

    acQTextFilterCtrl(QWidget* pParent = NULL);
    ~acQTextFilterCtrl();
    void initialize(acListCtrl* pListCtrl);
    void terminate();
    bool isDefaultString();
    const QString& GetFilterString() {return m_filterString;}
    void clear();
protected:
    // Event processing:
    void onEmptyFilterResult();
    void onNoneEmptyFilterResult();
    virtual void setInitializeFilterStyle();
    virtual void focusInEvent(QFocusEvent* pEvent);
    virtual void focusOutEvent(QFocusEvent* pEvent);

protected slots:
    void onFilterTextChanged(QString filterText);
    void onCursorPositionChanged(int, int);

Q_SIGNALS:
    void focused(bool hasFocus);

private:
    // The controlled list:
    acListCtrl* m_pList;

    // Hold the list control full content:
    gtPtrVector<QTableWidgetItem*> m_listFullContent;

    // The string to put in the text box when we don't have a filter:
    QString m_defaultNoFilterString;

    // Ugly hack to detect backspace
    QString m_defaultNoFilterTruncString;

    // Internal representation of the filter being applied:
    QString m_filterString;

    // Whether we are currently in the middle of an operation:
    bool m_filtering;

    // Whether any results passed the filter:
    bool m_isFilterResultOutputEmpty;

    // Color schemes:
    QPalette m_paletteRedWhite;
    QPalette m_paletteWhiteBlack;
    QPalette m_paletteWhiteGray;

};


#endif  // __ACQTEXTFILTERCTRL
