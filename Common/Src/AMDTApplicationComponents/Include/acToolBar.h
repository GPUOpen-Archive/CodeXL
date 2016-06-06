//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acToolBar.h
///
//==================================================================================

//------------------------------ acToolBar.h ------------------------------

#ifndef __ACTOOLBAR_H
#define __ACTOOLBAR_H

// Qt:
#include <QToolBar>
#include <QtGui>
#include <QWidgetAction>
#include <QtWidgets/QComboBox>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>


class AC_API acToolbarActionData
{

public:
    enum ToolbarActionType
    {
        AC_WIDGET_ACTION,
        AC_LABEL_ACTION,
        AC_COMBOBOX_ACTION,
        AC_TOOLBUTTON_ACTION
    };

    acToolbarActionData();

    acToolbarActionData(const char* signal, const QObject* pReceiver, const char* member);
    acToolbarActionData(const acToolbarActionData& other);

    bool m_isEnabled;
    bool m_isVisible;
    bool m_isMenuActionsEnabled;
    QString m_text;
    QString m_tooltip;
    QStringList m_menuStringsList;
    QStringList m_comboToolTipList;
    ToolbarActionType m_actionType;
    QPixmap* m_pPixmap;
    const char* m_pSignal;
    const QObject* m_pReceiver;
    const char* m_pMember;
    const char* m_pMemberForMenuActions;
    const char* m_pMemberForMenuAboutToShow;
    int m_margin;
    QString m_objectName;
    int m_minWidth;
    QComboBox::SizeAdjustPolicy m_adjustPolicy;
    int m_currentIndex;
    bool m_isBold;
    bool m_isUnderline;
    Qt::Alignment m_textAlignment;
    QKeySequence m_keyboardShortcut;
};

class acToolBar;
/// -----------------------------------------------------------------------------------------------
/// \class Name: acWidgetAction : public QWidgetAction
/// \brief Description:  In Qt Toolbars, once the toolbar is created for a non main window, the toolbar
///                      looses it's ability to display the expand button. In this case, the documentation
///                      of QToolbar guides to implement inheritance of QWidgetAction, and use it in these
///                      toolbars
/// -----------------------------------------------------------------------------------------------
class AC_API acWidgetAction : public QWidgetAction
{
    Q_OBJECT
public:

    acWidgetAction(QObject* pParent, const acToolbarActionData& data);

    /// Update the enable state of the widget:
    /// \param enable should the widget be enabled?
    void UpdateEnabled(bool enabled);

    /// Update the visibility of the widget:
    /// \param isVisible should the widget be visible?
    void UpdateVisible(bool isVisible);

    /// Update the menu item enable state of the widget menu items:
    /// \param isEnabled should the widget menu items be enabled?
    void UpdateMenuItemsEnabled(bool isEnabled);

    /// Update the text for label:
    /// \param text the updated text
    void UpdateText(const QString& text);

    /// Update the icon:
    /// \param icon the updated icon
    void UpdateIcon(const QIcon& icon);

    /// Update the bold value for the label font:
    /// \param isBold should the label font be bold?
    void UpdateIsBold(bool isBold);

    /// Update the tooltip for the widget:
    /// \param tooltip the tooltip requested
    void UpdateTooltip(const QString& tooltip);

    /// Update the string list for combo box:
    /// \param menuStringsList the updated strings list
    void UpdateStringList(const QStringList& menuStringsList);

    /// Update the string list with a new string for combo box:
    /// \param position the position of the new added string
    /// \param text the new string to add
    void InsertItemToStringList(int position, const QString& text);

    /// Add a string to the string list for combo box:
    /// \param text the new string to add
    void AddItemToStringList(const QString& text);

    /// Update the current index for combo box:
    /// \param currentIndex the updated combo index
    void UpdateCurrentIndex(int currentIndex);

    /// Update the keyboard shortcut for the action (used for tool button actions):
    /// \param currentIndex the updated combo index
    void UpdateShortcut(QKeySequence shortcut);

protected:

    // Overrides QWidgetAction:
    virtual QWidget* createWidget(QWidget* pParent);

private:

    /// Contain the data for this action:
    acToolbarActionData m_actionData;

    /// The parent toolbar:
    acToolBar* m_pToolbar;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API acToolBar : public QToolBar
// General Description: A base class for all Qt toolbars used in the application, controls
//                      common style and functionality.
// Author:              Uri Shomroni
// Creation Date:       19/2/2012
// ----------------------------------------------------------------------------------
class AC_API acToolBar : public QToolBar
{
    Q_OBJECT
public:
    acToolBar(QWidget* pParent, QString title = "");
    virtual ~acToolBar();

    acWidgetAction* AddWidget(const acToolbarActionData& toolbarData);
    acWidgetAction* AddWidget(const char* signal, const QObject* pReceiver, const char* member);
    acWidgetAction* AddLabel(const QString& text, const char* signal, const QObject* pReceiver, const char* member);
    acWidgetAction* AddLabel(const QString& text, bool isBold = false, bool isUnderline = false, int margin = 5);
    acWidgetAction* AddComboBox(const QStringList& comboTextList, const char* signal, const QObject* pReceiver, const char* member);
    acWidgetAction* AddComboBox(const QStringList& comboTextList, const QStringList& comboToolTipListList, const char* signal, const QObject* pReceiver, const char* member);
    acWidgetAction* AddToolbutton(QPixmap* pIconPixmap, const QString& text, const QStringList& toolbuttonMenu, const char* signal, const QObject* pReceiver, const char* member, const char* menuActionsMember, const char* menuAboutToShow);
};

#endif //__ACTOOLBAR_H

