//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acFindWidget.h
///
//==================================================================================

//------------------------------ acFindWidget.h ------------------------------

#ifndef __ACFINDWIDGET_H
#define __ACFINDWIDGET_H

// System:
#include <QtWidgets>

// Infra
#include <AMDTApplicationComponents/Include/acToolBar.h>
#include <AMDTApplicationComponents/Include/acLineEdit.h>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acFindParameters.h>


class AC_API acFindWidget : public QWidget
{
    Q_OBJECT

public:
    virtual ~acFindWidget();

    static acFindWidget& Instance();


    void SetSearchUpParam(bool searchUp);
    void SetTextToFind(QString text);
    void SetFocusOnFindText();
    bool IsFindWidgetHidden();
    void ShowFindWidget(bool show);

    /// \Returns pointer to FindWidget DockWidget, so it can be added to main layout
    QDockWidget* GetDockWidget() { return m_pDockWidget; };

    /// \Returns the widget for find button (so that the client can connect)
    QWidget* GetFindButton() { return m_pNextButton; };

    /// \brief Saves the currently active widget for the find feature:
    void SetFindFocusedWidget(QWidget* pFocusedWidget);

    /// Is called when the find result parameters are changed
    void UpdateUI();

protected slots:
    void OnFindNext();
    void OnFindPrevious();
    void OnClearFindText();
    void OnFindTextChange(const QString& text);


    void OnClose();
    void OnDeleteLastWidgetWithFocus();

    void OnFindParamChanged();


signals:
    void OnFind();

private:

    /// Adjusts the widgets look to the current fins text:
    void AdjustWidgetsToCurrentText(const QString& text);

    acFindWidget();
    static acFindWidget* m_spMySingleInstance;

    QDockWidget* m_pDockWidget;
    acToolBar* m_pEditToolbar;
    QWidget* m_pFindTextWidget;
    QHBoxLayout* m_pFindTextLayout;
    acLineEdit* m_pFindLineEdit;
    QPushButton* m_pFindCloseLineButton;
    QPushButton* m_pNextButton;
    QPushButton* m_pPrevButton;
    QPushButton* m_pCaseSensitiveButton;
    QPushButton* m_pCloseButton;

    QWidget* m_pLastWidgetWithFocus;

    QAction* m_pFindNextAction;
    QAction* m_pFindPrevAction;
    QAction* m_pCloseAction;

    bool m_searchUp;

    void InitPushButton(QPushButton*& pButton, QString styleSheet, acIconId iconId = AC_ICON_EMPTY);
};

#endif // ____ACFINDWIDGET_H_H
