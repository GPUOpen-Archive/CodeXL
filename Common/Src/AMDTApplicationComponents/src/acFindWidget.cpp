//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acFindWidget.cpp
///
//==================================================================================

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationComponents/Include/acFindWidget.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>

#define AC_FIND_PUSH_BUTTON_WIDTH 24

#define AC_STR_FindWidgetPushButtonStyle "QPushButton:hover {border: 1px solid gray; } QPushButton{border: 1px solid #B4B4B4; }"
#define AC_STR_FindWidgetPrevNextButtonsStyle "QPushButton:hover { "\
    "border: 1px solid gray;"\
    "border-style: inset;" \
    "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 white, stop: 1 rgba(255,255,207,255));margin-top: 2px;" \
    "}"\
    "QPushButton { "\
    "border: 1px solid gray;"\
    "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 white, stop: 1 rgba(255,255,207,255));" \
    "background-color:rgba(255,255,207,255);margin-top: 2px;"\
    "}"

#define AC_STR_FindWidgetPushButtonRadiusRight "QPushButton:hover { border-bottom-right-radius:2px; border-top-right-radius:2px; }" \
    "QPushButton{ border-bottom-right-radius:2px; border-top-right-radius:2px; }"

#define AC_STR_FindWidgetPushButtonStyleNoBorder "QPushButton:hover {border: 1px solid gray; border-radius:2px;margin-top: 2px;} QPushButton{border: none;margin-top: 2px;}"
#define AC_STR_FindWidgetToolBarStyle "QToolBar { border-top: 2px solid #B9B9B9; border-bottom: 1px solid #B9B9B9; border-style: outset;}"
#define AC_STR_FindWidgetTextWidgetStyle "QWidget { background-color:rgba(255,255,207,255); border: 1px solid gray; border-right:none; border-top-left-radius:2px;border-bottom-left-radius:2px; border-style: inset;  margin-top: 2px;}"
#define AC_STR_FindWidgetTextWidgetStyleNoResults "QWidget { background-color:rgba(255,255,207,255); border: 1px solid red; border-right:none; border-top-left-radius:2px;border-bottom-left-radius:2px; border-style: inset;  margin-top: 2px;}"
#define AC_STR_FindWidgetLineEditStyle "QLineEdit {border-style: none;}"
#define AC_STR_FindWidgetMatchCaseStyle "QPushButton { padding-right: 8px; padding-left: 8px;} QPushButton:checked { border: 1px solid gray; border-radius:2px; padding-right: 10px; padding-left: 10px; padding: 2px;margin-top: 2px;border-radius:4px;  border-style:inset; } QPushButton:hover { border: 1px solid gray; border-radius:2px;  border-style:outset; padding-right: 10px; padding-left: 10px; padding: 2px;margin-top: 2px;}"

acFindWidget* acFindWidget::m_spMySingleInstance = nullptr;

// ---------------------------------------------------------------------------
acFindWidget::acFindWidget()
    : QWidget(nullptr),
      m_pDockWidget(nullptr), m_pEditToolbar(nullptr), m_pFindTextWidget(nullptr),
      m_pFindTextLayout(nullptr), m_pFindLineEdit(nullptr), m_pFindCloseLineButton(nullptr),
      m_pNextButton(nullptr), m_pPrevButton(nullptr), m_pCaseSensitiveButton(nullptr),
      m_pCloseButton(nullptr), m_pLastWidgetWithFocus(nullptr),
      m_pFindNextAction(nullptr), m_pFindPrevAction(nullptr),
      m_pCloseAction(nullptr)
{
    setContentsMargins(0, 0, 0, 0);

    // init the dock widget
    m_pDockWidget = new QDockWidget(nullptr);
    m_pDockWidget->setObjectName("FindDockWidget");
    m_pDockWidget->setAllowedAreas(Qt::TopDockWidgetArea);
    m_pDockWidget->setFloating(false);
    m_pDockWidget->setTitleBarWidget(new QWidget());
    m_pDockWidget->setContentsMargins(0, 0, 0, 0);
    // init the toolbar, sits on the dock widget
    m_pEditToolbar = new acToolBar(m_pDockWidget);
    m_pEditToolbar->setContentsMargins(0, 2, 0, 2);
    m_pEditToolbar->setStyleSheet(AC_STR_FindWidgetToolBarStyle);
    m_pEditToolbar->setObjectName("FindToolBar");

    // widget that contains the text ctrl combined with an icon,
    // m_pFindTextWidget is used as a wrapper to make the text box and icon look like one unit
    m_pFindTextWidget = new QWidget();
    m_pFindTextWidget->setContentsMargins(0, 2, 0, 2);
    m_pFindTextWidget->setFixedWidth(196);
    m_pFindTextWidget->setStyleSheet(AC_STR_FindWidgetTextWidgetStyle);

    // m_pFindTextWidget 's layout
    m_pFindTextLayout = new QHBoxLayout();
    m_pFindTextLayout->setContentsMargins(1, 0, 1, 0);
    m_pFindTextWidget->setLayout(m_pFindTextLayout);

    // m_pFindLineEdit - holds the searched text
    m_pFindLineEdit = new acLineEdit(nullptr);
    m_pFindLineEdit->setStyleSheet(AC_STR_FindWidgetLineEditStyle);


    // button holds 2 icons, allows to clear the searched text
    // the image displayed is modified according to the icon enable state
    InitPushButton(m_pFindCloseLineButton, AC_STR_FindWidgetPushButtonStyleNoBorder);
    m_pFindCloseLineButton->setEnabled(false);
    m_pFindCloseLineButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    QPixmap searchPixmap;
    acSetIconInPixmap(searchPixmap, AC_ICON_FIND_FIND);
    QPixmap closePixmap;
    acSetIconInPixmap(closePixmap, AC_ICON_FIND_CLOSE_CLEAR);
    QIcon searchIcon;
    searchIcon.addPixmap(searchPixmap, QIcon::Disabled);
    searchIcon.addPixmap(closePixmap, QIcon::Normal);
    m_pFindCloseLineButton->setIcon(searchIcon);

    // add to the custom text widget layout
    m_pFindTextLayout->addWidget(m_pFindLineEdit, 1, 0);
    m_pFindTextLayout->addWidget(m_pFindCloseLineButton, 0, 0);

    // init search up and search down buttons
    InitPushButton(m_pNextButton, AC_STR_FindWidgetPrevNextButtonsStyle, AC_ICON_FIND_DOWN);
    QString style2 = AC_STR_FindWidgetPrevNextButtonsStyle;
    style2.append(AC_STR_FindWidgetPushButtonRadiusRight);
    InitPushButton(m_pPrevButton, style2, AC_ICON_FIND_UP);

    m_pPrevButton->setEnabled(false);
    m_pNextButton->setEnabled(false);

    m_pPrevButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_pNextButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    // init case sensitive check box
    m_pCaseSensitiveButton = new QPushButton(tr(AC_STR_matchCase));
    m_pCaseSensitiveButton->setFlat(true);
    m_pCaseSensitiveButton->setCheckable(true);
    m_pCaseSensitiveButton->setStyleSheet(AC_STR_FindWidgetMatchCaseStyle);
    m_pCaseSensitiveButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);


    // init toolbar close button
    InitPushButton(m_pCloseButton, AC_STR_FindWidgetPushButtonStyleNoBorder, AC_ICON_FIND_CLOSE_CLEAR);
    m_pCloseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    // add an empty expanding widget(to make the close button align to right) :
    QWidget* pSpacer = new QWidget;
    pSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QWidget* pSpacer2 = new QWidget;
    pSpacer2->setFixedWidth(5);

    // adding all widgets to toolbar
    m_pEditToolbar->addWidget(pSpacer);
    m_pEditToolbar->addWidget(m_pFindTextWidget);
    m_pFindNextAction = m_pEditToolbar->addWidget(m_pNextButton);
    m_pFindPrevAction = m_pEditToolbar->addWidget(m_pPrevButton);
    m_pEditToolbar->addWidget(pSpacer2);
    m_pEditToolbar->addWidget(m_pCaseSensitiveButton);
    m_pCloseAction = m_pEditToolbar->addWidget(m_pCloseButton);

    // Set the shortcuts for the items which their shortcuts do not change:
    m_pFindPrevAction->setShortcut(QKeySequence::FindPrevious);
    m_pCloseAction->setShortcut(QKeySequence(Qt::Key_Escape));

    m_pDockWidget->setWidget(m_pEditToolbar);

    // connect signals
    bool rc = connect(m_pCloseButton, SIGNAL(clicked()), this, SLOT(OnClose()));
    GT_ASSERT(rc == true);
    rc = connect(m_pNextButton, SIGNAL(clicked()), this, SLOT(OnFindNext()));
    GT_ASSERT(rc == true);
    rc = connect(m_pPrevButton, SIGNAL(clicked()), this, SLOT(OnFindPrevious()));
    GT_ASSERT(rc == true);
    rc = connect(m_pCloseAction, SIGNAL(triggered()), this, SLOT(OnClose()));
    GT_ASSERT(rc);
    rc = connect(m_pFindNextAction, SIGNAL(triggered()), this, SLOT(OnFindNext()));
    GT_ASSERT(rc);
    rc = connect(m_pFindPrevAction, SIGNAL(triggered()), this, SLOT(OnFindPrevious()));
    GT_ASSERT(rc);
    rc = connect(m_pFindCloseLineButton, SIGNAL(clicked()), this, SLOT(OnClearFindText()));
    GT_ASSERT(rc);
    rc = connect(m_pFindLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(OnFindTextChange(const QString&)));
    GT_ASSERT(rc);
    rc = connect(m_pCaseSensitiveButton, SIGNAL(clicked()), this, SLOT(OnFindParamChanged()));
    GT_ASSERT(rc);

    m_pDockWidget->setHidden(true);
}

// ---------------------------------------------------------------------------
acFindWidget::~acFindWidget()
{
}


acFindWidget& acFindWidget::Instance()
{
    if (nullptr == m_spMySingleInstance)
    {
        m_spMySingleInstance = new acFindWidget;
    }

    return *m_spMySingleInstance;
}
// ---------------------------------------------------------------------------
void acFindWidget::OnFindPrevious()
{
    acFindParameters::Instance().m_isSearchUp = true;
    emit OnFind();
}

// ---------------------------------------------------------------------------
void acFindWidget::OnFindNext()
{
    acFindParameters::Instance().m_isSearchUp = false;
    emit OnFind();
}

// ---------------------------------------------------------------------------
void acFindWidget::SetFindFocusedWidget(QWidget* pFocusedWidget)
{
    bool rc;

    if (nullptr != m_pLastWidgetWithFocus)
    {
        rc = m_pLastWidgetWithFocus->disconnect(SIGNAL(destroyed()));
        GT_ASSERT(rc);
    }

    m_pLastWidgetWithFocus = pFocusedWidget;
    rc = connect(pFocusedWidget, SIGNAL(destroyed()), this, SLOT(OnDeleteLastWidgetWithFocus()));
    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
void acFindWidget::OnClose()
{
    GT_IF_WITH_ASSERT(nullptr != m_pDockWidget)
    {
        m_pDockWidget->setHidden(true);
    }

    // setting focus to last widget who had it, only if still visible to user
    if (nullptr != m_pLastWidgetWithFocus && m_pLastWidgetWithFocus->isTopLevel())
    {
        m_pLastWidgetWithFocus->setFocus();
    }

    m_pLastWidgetWithFocus = nullptr;
}

// ---------------------------------------------------------------------------
void acFindWidget::OnDeleteLastWidgetWithFocus()
{
    m_pLastWidgetWithFocus = nullptr;
}

void acFindWidget::SetFocusOnFindText()
{
    // Sanity check
    GT_IF_WITH_ASSERT(m_pFindLineEdit != nullptr)
    {
        m_pFindLineEdit->setFocus();
        m_pFindLineEdit->selectAll();
    }
}

void acFindWidget::SetSearchUpParam(bool searchUp)
{
    m_searchUp = searchUp;
}

void acFindWidget::SetTextToFind(QString text)
{
    // Sanity check
    GT_IF_WITH_ASSERT(m_pFindLineEdit != nullptr)
    {
        m_pFindLineEdit->setText(text);
        m_pNextButton->setEnabled(!text.isEmpty());
        m_pPrevButton->setEnabled(!text.isEmpty());
        AdjustWidgetsToCurrentText(text);
    }
}

// ---------------------------------------------------------------------------
void acFindWidget::OnClearFindText()
{
    GT_IF_WITH_ASSERT(m_pFindLineEdit != nullptr)
    {
        m_pFindLineEdit->clear();
        AdjustWidgetsToCurrentText("");

        UpdateUI();
        m_pFindLineEdit->setFocus();

    }
}

// ---------------------------------------------------------------------------
void acFindWidget::OnFindTextChange(const QString& text)
{
    // Adjust the widgets look to the current text:
    AdjustWidgetsToCurrentText(text);

    acFindParameters::Instance().m_findExpr = m_pFindLineEdit->text();

    if (acFindParameters::Instance().m_shouldRespondToTextChange)
    {
        // Emit a find signal:
        emit OnFind();
    }
}

bool acFindWidget::IsFindWidgetHidden()
{
    bool isHidden = false;

    GT_IF_WITH_ASSERT(m_pDockWidget != nullptr)
    {
        isHidden = m_pDockWidget->isHidden();
    }
    return isHidden;
}

// ---------------------------------------------------------------------------
void acFindWidget::ShowFindWidget(bool show)
{
    GT_IF_WITH_ASSERT(m_pDockWidget != nullptr)
    {
        m_pDockWidget->setHidden(!show);

        // Notice: find next action is supposed to use F3 shortcut. The shortcut should both be available
        // when the toolbar is opened, and when it is closed (through the main menu).
        // To avoid shortcuts ambiguity, we set the shortcut on show, and un-set on hide:
        QList<QKeySequence> findNextShortcuts;

        if (show)
        {
            findNextShortcuts.append(QKeySequence::FindNext);
            findNextShortcuts.append(QKeySequence(Qt::Key_Return));
        }

        // On hide the list will be empty:

        m_pFindNextAction->setShortcuts(findNextShortcuts);
    }
}

// ---------------------------------------------------------------------------
void acFindWidget::InitPushButton(QPushButton*& pButton, QString styleSheet, acIconId iconId)
{
    pButton = new QPushButton();
    pButton->setStyleSheet(styleSheet);
    pButton->setFlat(true);
    pButton->setFixedWidth(AC_FIND_PUSH_BUTTON_WIDTH);

    if (iconId != AC_ICON_EMPTY)
    {
        QPixmap pixMap;
        acSetIconInPixmap(pixMap, iconId);
        pButton->setIcon(QIcon(pixMap));
    }
}

void acFindWidget::UpdateUI()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pFindTextWidget != nullptr) && (m_pFindLineEdit != nullptr))
    {
        bool wasFound = acFindParameters::Instance().m_lastResult || acFindParameters::Instance().m_findExpr.isEmpty();
        // Set the style according to the results:
        QString style = wasFound ? AC_STR_FindWidgetTextWidgetStyle : AC_STR_FindWidgetTextWidgetStyleNoResults;
        m_pFindTextWidget->setStyleSheet(style);

        // Give the focus back to the text:
        m_pFindLineEdit->setFocus();
    }
}

void acFindWidget::AdjustWidgetsToCurrentText(const QString& text)
{
    // m_pFindLineButton change displayed image according to text
    // text is not empty - image is delete and pressing button will clear the text
    GT_IF_WITH_ASSERT((m_pFindCloseLineButton != nullptr) && (m_pNextButton != nullptr) && (m_pPrevButton != nullptr))
    {
        bool hasText = !text.isEmpty();
        m_pFindCloseLineButton->setEnabled(hasText);
        m_pNextButton->setEnabled(hasText);
        m_pPrevButton->setEnabled(hasText);
    }
}

void acFindWidget::OnFindParamChanged()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pFindLineEdit != nullptr) && (m_pCaseSensitiveButton != nullptr))
    {
        acFindParameters::Instance().m_findExpr = m_pFindLineEdit->text();
        acFindParameters::Instance().m_isCaseSensitive = m_pCaseSensitiveButton->isChecked();
        acFindParameters::Instance().m_isSearchUp = m_searchUp;
    }
}
