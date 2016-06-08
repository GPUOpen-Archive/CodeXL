//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acToolBar.cpp
///
//==================================================================================

//------------------------------ acToolBar.cpp ------------------------------

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QComboBox>
#include <QLabel>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acToolBar.h>


acToolbarActionData::acToolbarActionData() :
    m_isEnabled(true),
    m_isVisible(true),
    m_isMenuActionsEnabled(true),
    m_actionType(AC_LABEL_ACTION),
    m_pPixmap(NULL),
    m_pSignal(NULL),
    m_pReceiver(NULL),
    m_pMember(NULL),
    m_pMemberForMenuActions(NULL),
    m_pMemberForMenuAboutToShow(NULL),
    m_margin(-1),
    m_minWidth(-1),
    m_adjustPolicy(QComboBox::AdjustToContents),
    m_currentIndex(-1),
    m_isBold(false),
    m_isUnderline(false),
    m_textAlignment(Qt::AlignLeft | Qt::AlignVCenter)
{
    m_isEnabled = true;
    m_isVisible = true;
    m_isMenuActionsEnabled = true;
    m_text = "";
    m_tooltip = "";
    m_actionType = AC_LABEL_ACTION;
    m_pPixmap = NULL;
    m_pSignal = NULL;
    m_pReceiver = NULL;
    m_pMember = NULL;
    m_pMemberForMenuActions = NULL;
    m_pMemberForMenuAboutToShow = NULL;
    m_margin = -1;
    m_objectName = "";
    m_minWidth = -1;
    m_adjustPolicy = QComboBox::AdjustToContents;
    m_currentIndex = -1;
    m_isBold = false;
    m_isUnderline = false;
    m_textAlignment = Qt::AlignLeft | Qt::AlignVCenter;
}

acToolbarActionData::acToolbarActionData(const char* signal, const QObject* pReceiver, const char* member) :
    m_isEnabled(true),
    m_isVisible(true),
    m_isMenuActionsEnabled(true),
    m_actionType(AC_LABEL_ACTION),
    m_pPixmap(NULL),
    m_pSignal(signal),
    m_pReceiver(pReceiver),
    m_pMember(member),
    m_pMemberForMenuActions(NULL),
    m_pMemberForMenuAboutToShow(NULL),
    m_margin(-1),
    m_minWidth(-1),
    m_adjustPolicy(QComboBox::AdjustToContents),
    m_currentIndex(-1),
    m_isBold(false),
    m_isUnderline(false),
    m_textAlignment(Qt::AlignLeft | Qt::AlignVCenter)
{
}

acToolbarActionData::acToolbarActionData(const acToolbarActionData& other)
{
    m_isEnabled = other.m_isEnabled;
    m_isVisible = other.m_isVisible;
    m_isMenuActionsEnabled = other.m_isMenuActionsEnabled;
    m_text = other.m_text;
    m_tooltip = other.m_tooltip;
    m_menuStringsList = other.m_menuStringsList;
    m_comboToolTipList = other.m_comboToolTipList;
    m_actionType = other.m_actionType;
    m_pSignal = other.m_pSignal;
    m_pPixmap = other.m_pPixmap;
    m_pReceiver = other.m_pReceiver;
    m_pMember = other.m_pMember;
    m_pMemberForMenuActions = other.m_pMemberForMenuActions;
    m_pMemberForMenuAboutToShow = other.m_pMemberForMenuAboutToShow;
    m_margin = other.m_margin;
    m_objectName = other.m_objectName;
    m_minWidth = other.m_minWidth;
    m_adjustPolicy = other.m_adjustPolicy;
    m_currentIndex = other.m_currentIndex;
    m_isBold = other.m_isBold;
    m_isUnderline = other.m_isUnderline;
    m_textAlignment = other.m_textAlignment;
    m_keyboardShortcut = other.m_keyboardShortcut;
}


// ---------------------------------------------------------------------------
// Name:        acToolBar::acToolBar
// Description: Constructor
// Author:      Uri Shomroni
// Date:        20/2/2012
// ---------------------------------------------------------------------------
acToolBar::acToolBar(QWidget* pParent, QString title)
    : QToolBar(title, pParent)
{
    // The button style:
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    unsigned int recommendedIconPixelSize = acIconSizeToPixelSize(acGetRecommendedIconSize());
    setIconSize(QSize(recommendedIconPixelSize, recommendedIconPixelSize));
}

// ---------------------------------------------------------------------------
// Name:        acToolBar::~acToolBar
// Description: Destructor
// Author:      Uri Shomroni
// Date:        20/2/2012
// ---------------------------------------------------------------------------
acToolBar::~acToolBar()
{

}

acWidgetAction* acToolBar::AddLabel(const QString& text, const char* signal, const QObject* pReceiver, const char* member)
{
    acWidgetAction* pRetVal = NULL;

    acToolbarActionData dataLoc;
    dataLoc.m_text = text;
    dataLoc.m_pSignal = signal;
    dataLoc.m_pReceiver = pReceiver;
    dataLoc.m_pMember = member;
    dataLoc.m_actionType = acToolbarActionData::AC_LABEL_ACTION;
    dataLoc.m_margin = 5;

    pRetVal = new acWidgetAction(this, dataLoc);

    addAction(pRetVal);

    return pRetVal;
}

acWidgetAction* acToolBar::AddLabel(const QString& text, bool isBold, bool isUnderline, int margin)
{
    acWidgetAction* pRetVal = NULL;

    acToolbarActionData dataLoc;
    dataLoc.m_text = text;
    dataLoc.m_isBold = isBold;
    dataLoc.m_isUnderline = isUnderline;
    dataLoc.m_actionType = acToolbarActionData::AC_LABEL_ACTION;
    dataLoc.m_margin = margin;

    pRetVal = new acWidgetAction(this, dataLoc);

    addAction(pRetVal);

    return pRetVal;
}

acWidgetAction* acToolBar::AddComboBox(const QStringList& comboTextList, const char* signal, const QObject* pReceiver, const char* member)
{
    acWidgetAction* pRetVal = NULL;

    acToolbarActionData dataLoc;
    dataLoc.m_menuStringsList = comboTextList;
    dataLoc.m_pSignal = signal;
    dataLoc.m_pReceiver = pReceiver;
    dataLoc.m_pMember = member;
    dataLoc.m_actionType = acToolbarActionData::AC_COMBOBOX_ACTION;

    pRetVal = new acWidgetAction(this, dataLoc);

    addAction(pRetVal);

    return pRetVal;
}

acWidgetAction* acToolBar::AddComboBox(const QStringList& comboTextList, const QStringList& comboToolTipList, const char* signal, const QObject* pReceiver, const char* member)
{
    acWidgetAction* pRetVal = NULL;

    acToolbarActionData dataLoc;
    dataLoc.m_menuStringsList = comboTextList;
    dataLoc.m_pSignal = signal;
    dataLoc.m_pReceiver = pReceiver;
    dataLoc.m_pMember = member;
    dataLoc.m_actionType = acToolbarActionData::AC_COMBOBOX_ACTION;
    dataLoc.m_comboToolTipList = comboToolTipList;

    pRetVal = new acWidgetAction(this, dataLoc);

    addAction(pRetVal);

    return pRetVal;
}

acWidgetAction* acToolBar::AddWidget(const char* signal, const QObject* pReceiver, const char* member)
{
    acWidgetAction* pRetVal = NULL;

    acToolbarActionData dataLoc;
    dataLoc.m_pSignal = signal;
    dataLoc.m_pReceiver = pReceiver;
    dataLoc.m_pMember = member;
    dataLoc.m_actionType = acToolbarActionData::AC_WIDGET_ACTION;

    pRetVal = new acWidgetAction(this, dataLoc);

    addAction(pRetVal);

    return pRetVal;
}

acWidgetAction* acToolBar::AddWidget(const acToolbarActionData& toolbarData)
{
    acWidgetAction* pRetVal = NULL;

    pRetVal = new acWidgetAction(this, toolbarData);

    addAction(pRetVal);

    return pRetVal;
}

acWidgetAction* acToolBar::AddToolbutton(QPixmap* pIconPixmap, const QString& text, const QStringList& toolbuttonMenu,
                                         const char* signal, const QObject* pReceiver, const char* member, const char* menuActionsMember, const char* menuAboutToShow)
{
    acWidgetAction* pRetVal = NULL;

    acToolbarActionData dataLoc;
    dataLoc.m_menuStringsList = toolbuttonMenu;
    dataLoc.m_text = text;
    dataLoc.m_pSignal = signal;
    dataLoc.m_pReceiver = pReceiver;
    dataLoc.m_pMember = member;
    dataLoc.m_pMemberForMenuActions = menuActionsMember;
    dataLoc.m_pMemberForMenuAboutToShow = menuAboutToShow;
    dataLoc.m_actionType = acToolbarActionData::AC_TOOLBUTTON_ACTION;
    dataLoc.m_pPixmap = pIconPixmap;
    pRetVal = new acWidgetAction(this, dataLoc);

    addAction(pRetVal);

    return pRetVal;
}

QWidget* acWidgetAction::createWidget(QWidget* pParent)
{
    QWidget* pRetVal = NULL;

    switch (m_actionData.m_actionType)
    {

        case acToolbarActionData::AC_WIDGET_ACTION:
        {
            pRetVal = new QWidget(pParent);

        }
        break;

        case acToolbarActionData::AC_COMBOBOX_ACTION:
        {
            QComboBox* pCombo = new QComboBox(pParent);
            pRetVal = pCombo;

            bool hasToolTip = ((m_actionData.m_comboToolTipList.count() > 0) &&
                               (m_actionData.m_comboToolTipList.count() == m_actionData.m_menuStringsList.count()));

            if (hasToolTip)
            {
                // in case there is tooltip.. add the tool tip to the combo item
                int listSize = m_actionData.m_menuStringsList.count();

                for (int i = 0; i < listSize; i++)
                {
                    pCombo->addItem(m_actionData.m_menuStringsList[i]);
                    pCombo->setItemData(i, m_actionData.m_comboToolTipList[i], Qt::ToolTipRole);
                }
            }
            else
            {
                pCombo->addItems(m_actionData.m_menuStringsList);
            }

            pCombo->setSizeAdjustPolicy(m_actionData.m_adjustPolicy);

            if (m_actionData.m_currentIndex >= 0)
            {
                pCombo->setCurrentIndex(m_actionData.m_currentIndex);
            }
        }
        break;

        case acToolbarActionData::AC_TOOLBUTTON_ACTION:
        {
            QToolButton* pButton = new QToolButton(pParent);
            pButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

            if (m_actionData.m_pPixmap != NULL)
            {
                pButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
                pButton->setIcon(QIcon(*m_actionData.m_pPixmap));
            }

            pButton->setText(m_actionData.m_text);
            pButton->setShortcut(m_actionData.m_keyboardShortcut);

            pRetVal = pButton;
            QMenu* pActionsMenu = new QMenu;

            foreach (QString str, m_actionData.m_menuStringsList)
            {
                // Add the current action for the tool button menu. The slot for the menu items is stored in m_pMemberForMenuActions:
                pActionsMenu->addAction(str, m_actionData.m_pReceiver, m_actionData.m_pMemberForMenuActions);
            }

            // Set the button menu:
            pButton->setMenu(pActionsMenu);

            // Connect the about to show signal to the requested slot:
            bool rc = connect(pActionsMenu, SIGNAL(aboutToShow()), m_actionData.m_pReceiver, m_actionData.m_pMemberForMenuAboutToShow);
            GT_ASSERT(rc);

            pButton->setPopupMode(QToolButton::MenuButtonPopup);

        }
        break;

        case acToolbarActionData::AC_LABEL_ACTION:
        default:
        {
            QLabel* pLabel = new QLabel(m_actionData.m_text, pParent);
            pRetVal = pLabel;

            if (m_actionData.m_margin > 0)
            {
                pLabel->setMargin(m_actionData.m_margin);
            }

            // If there is a label with link and an object name in the toolbar we need to remove all spacing in the left/top/bottom area so it can be
            // clicked by the QA auto testing system. the top/bottom change does not affect the appearance, only the left part will look a bit closer to the
            // object on the left of it if there a margin set.
            // 1: set the margin and padding left/right correctly
            // 2: Set the control maximum height to the text size
            if ((m_actionData.m_pMember != NULL) && (m_actionData.m_pReceiver != NULL) && (m_actionData.m_pSignal != NULL) && (!m_actionData.m_objectName.isEmpty()))
            {
                pLabel->setMargin(0);
                QString styleSheetStr = QString("QLabel { padding-right: %1px; padding-left: -3px;}").arg(m_actionData.m_margin);
                pLabel->setStyleSheet(styleSheetStr);
                int fontHeight = pLabel->fontMetrics().height();
                pLabel->setMaximumHeight(fontHeight);
            }

            pLabel->setAlignment(m_actionData.m_textAlignment);
            pLabel->setContextMenuPolicy(Qt::NoContextMenu);
            QFont font = pLabel->font();
            font.setBold(m_actionData.m_isBold);
            font.setUnderline(m_actionData.m_isUnderline);
            pLabel->setFont(font);
        }
        break;
    }

    // Connect to the requested signal if not null:
    if ((m_actionData.m_pMember != NULL) && (m_actionData.m_pReceiver != NULL) && (m_actionData.m_pSignal != NULL))
    {
        bool rc = connect(pRetVal, m_actionData.m_pSignal, m_actionData.m_pReceiver, m_actionData.m_pMember);
        GT_ASSERT(rc);
    }

    if (m_actionData.m_minWidth > 0)
    {
        pRetVal->setMinimumWidth(m_actionData.m_minWidth);
    }

    if (!m_actionData.m_objectName.isEmpty())
    {
        pRetVal->setObjectName(m_actionData.m_objectName);
    }

    pRetVal->setEnabled(m_actionData.m_isEnabled);
    pRetVal->setVisible(m_actionData.m_isVisible);

    return pRetVal;
}

acWidgetAction::acWidgetAction(QObject* pParent, const acToolbarActionData& data) : QWidgetAction(pParent), m_actionData(data)
{
    m_pToolbar = qobject_cast<acToolBar*>(pParent);
}

void acWidgetAction::UpdateText(const QString& text)
{
    // Update the action data:
    m_actionData.m_text = text;

    QLabel* pLabel = qobject_cast<QLabel*>(m_pToolbar->widgetForAction(this));

    if (pLabel != NULL)
    {
        pLabel->setText(text);
    }
    else
    {
        // m_text is also used for QToolButton:
        QToolButton* pToolbutton = qobject_cast<QToolButton*>(m_pToolbar->widgetForAction(this));

        if (pToolbutton != NULL)
        {
            pToolbutton->setText(text);
        }
    }
}

void acWidgetAction::UpdateIcon(const QIcon& icon)
{
    // m_text is also used for QToolButton:
    QToolButton* pToolbutton = qobject_cast<QToolButton*>(m_pToolbar->widgetForAction(this));

    if (pToolbutton != NULL)
    {
        pToolbutton->setIcon(icon);
    }
}

void acWidgetAction::UpdateTooltip(const QString& tooltip)
{
    // Update the action data:
    m_actionData.m_text = tooltip;
    QWidget* pWidget = qobject_cast<QWidget*>(m_pToolbar->widgetForAction(this));

    if (pWidget != NULL)
    {
        pWidget->setToolTip(tooltip);
    }
}

void acWidgetAction::UpdateStringList(const QStringList& menuStringsList)
{
    // Update the action data:
    m_actionData.m_menuStringsList = menuStringsList;

    // m_menuStringsList is used both for QComboBox list, and for the QToolbutton menu:

    QComboBox* pComboBox = qobject_cast<QComboBox*>(m_pToolbar->widgetForAction(this));

    if (pComboBox != NULL)
    {
        pComboBox->blockSignals(true);
        pComboBox->clear();
        pComboBox->addItems(menuStringsList);
        pComboBox->blockSignals(false);
    }

    else
    {
        QToolButton* pToolbutton = qobject_cast<QToolButton*>(m_pToolbar->widgetForAction(this));

        if (pToolbutton != NULL)
        {
            QMenu* pActionsMenu = pToolbutton->menu();

            if (pActionsMenu != NULL)
            {
                pActionsMenu->clear();

                foreach (QString str, m_actionData.m_menuStringsList)
                {
                    // Add the current action for the tool button menu. The slot for the menu items is stored in m_pMemberForMenuActions:
                    pActionsMenu->addAction(str, m_actionData.m_pReceiver, m_actionData.m_pMemberForMenuActions);
                }
            }
        }
    }
}

void acWidgetAction::UpdateCurrentIndex(int currentIndex)
{
    // Update the action data:
    m_actionData.m_currentIndex = currentIndex;
    GT_IF_WITH_ASSERT(m_pToolbar != NULL)
    {
        QComboBox* pComboBox = qobject_cast<QComboBox*>(m_pToolbar->widgetForAction(this));

        if (pComboBox != NULL)
        {
            pComboBox->setCurrentIndex(currentIndex);
        }
    }
}


void acWidgetAction::UpdateShortcut(QKeySequence shortcut)
{
    // Update the action data:
    m_actionData.m_keyboardShortcut = shortcut;
    GT_IF_WITH_ASSERT(m_pToolbar != NULL)
    {
        QToolButton* pToolbutton = qobject_cast<QToolButton*>(m_pToolbar->widgetForAction(this));

        if (pToolbutton != NULL)
        {
            pToolbutton->setShortcut(shortcut);
        }
    }
}


void acWidgetAction::UpdateEnabled(bool enabled)
{
    // Update the action data:
    m_actionData.m_isEnabled = enabled;

    QWidget* pWidget = qobject_cast<QWidget*>(m_pToolbar->widgetForAction(this));

    if (pWidget != NULL)
    {
        pWidget->setEnabled(enabled);
    }
}

void acWidgetAction::UpdateVisible(bool isVisible)
{
    // Show / hide the action:
    setVisible(isVisible);

    // Update the action data:
    m_actionData.m_isVisible = isVisible;

    QWidget* pWidget = qobject_cast<QWidget*>(m_pToolbar->widgetForAction(this));

    if (pWidget != NULL)
    {
        pWidget->setVisible(isVisible);
    }
}


void acWidgetAction::UpdateMenuItemsEnabled(bool isEnabled)
{
    m_actionData.m_isMenuActionsEnabled = isEnabled;

    QToolButton* pToolbutton = qobject_cast<QToolButton*>(m_pToolbar->widgetForAction(this));

    if (pToolbutton != NULL)
    {
        QMenu* pActionsMenu = pToolbutton->menu();

        if (pActionsMenu != NULL)
        {
            foreach (QAction* pAction, pActionsMenu->actions())
            {
                // Update each of the actions enable state:
                pAction->setEnabled(isEnabled);
            }
        }
    }
}


void acWidgetAction::InsertItemToStringList(int position, const QString& text)
{
    // Update the action data:
    m_actionData.m_menuStringsList.insert(position, text);
    QComboBox* pComboBox = qobject_cast<QComboBox*>(m_pToolbar->widgetForAction(this));

    if (pComboBox != NULL)
    {
        pComboBox->insertItem(position, text);
    }
}

void acWidgetAction::AddItemToStringList(const QString& text)
{
    // Update the action data:
    m_actionData.m_menuStringsList.append(text);

    QComboBox* pComboBox = qobject_cast<QComboBox*>(m_pToolbar->widgetForAction(this));

    if (pComboBox != NULL)
    {
        pComboBox->addItem(text);
    }
}

void acWidgetAction::UpdateIsBold(bool isBold)
{
    // Update the action data:
    m_actionData.m_isBold = isBold;

    QLabel* pLabel = qobject_cast<QLabel*>(m_pToolbar->widgetForAction(this));

    if (pLabel != NULL)
    {
        QFont font = pLabel->font();
        font.setBold(m_actionData.m_isBold);
        pLabel->setFont(font);
    }
}
