//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afStartupDialog.cpp
///
//==================================================================================

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acIcons.h>

// Qt:
#include <QtCore>
#include <QtWidgets>

/// Local:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afCSSSettings.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/dialogs/afStartupDialog.h>
#include <AMDTApplicationFramework/src/afUtils.h>

#define AF_STARTUP_DLG_DEFAULT_LAYOUT_SPACING 15

afRichTextButton::afRichTextButton(const QImage& iconImage, const QString& text): QPushButton()
{
    m_pImage = new QImage(iconImage);
    m_buttonText = text;

    QString buttonText  = QString(AF_STR_StartupButtonTableHTML).arg(text);
    QTextDocument htmlTextDocument;
    htmlTextDocument.addResource(QTextDocument::ImageResource, QUrl("mydata://image.jpg"), QVariant(*m_pImage));
    htmlTextDocument.setDefaultStyleSheet(AF_STR_noBorderAlignLeft);
    htmlTextDocument.setHtml(buttonText);

    // Get the size of the document:
    m_calculatedSize = htmlTextDocument.size();
    QPixmap pixmap(htmlTextDocument.size().width(), htmlTextDocument.size().height());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    htmlTextDocument.drawContents(&painter, pixmap.rect());

    QIcon icon(pixmap);
    setIcon(icon);
    setIconSize(pixmap.rect().size());
}
afRichTextButton::~afRichTextButton()
{
    delete m_pImage;
    m_pImage = nullptr;
}

void afRichTextButton::Draw(const QSizeF& buttonSize)
{
    GT_IF_WITH_ASSERT(m_pImage != nullptr)
    {
        QString buttonText  = QString(AF_STR_StartupButtonTableHTML).arg(m_buttonText);
        QTextDocument htmlTextDocument;
        htmlTextDocument.addResource(QTextDocument::ImageResource, QUrl("mydata://image.jpg"), QVariant(*m_pImage));
        htmlTextDocument.setDefaultStyleSheet(AF_STR_noBorderAlignLeft);
        htmlTextDocument.setHtml(buttonText);

        // Get the size of the document:
        QPixmap pixmap(buttonSize.width(), buttonSize.height());
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        htmlTextDocument.drawContents(&painter, pixmap.rect());

        QIcon icon(pixmap);
        setIcon(icon);
        setIconSize(pixmap.rect().size());
        setStyleSheet(AF_STR_StartupDialogPushButtonCSS);
    }
}


afStartupDialog::afStartupDialog(QWidget* pParent)
    : acDialog(pParent), m_pNewProjectButton(nullptr)
{
    // Window modality:
    setWindowModality(Qt::WindowModal);

    Qt::WindowFlags flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);

    setWindowTitle(AF_STR_StartupDialogCaption);

    float maxWidth = 0;
    float maxHeight = 0;

    // Get the system icon size ("16"):
    acIconSize iconSize = acGetScaledIconSize(AC_32x32_ICON);

    // Create the buttons for the startup actions:
    QString buttonText = QString("<b>%1</b>%2").arg(AF_STR_StartupDialogNewProjectTitle).arg(AF_STR_StartupDialogNewProjectDescription);
    m_pNewProjectButton = new afRichTextButton(acGetIcon(AC_ICON_STARTUP_NEW, iconSize), buttonText);


    buttonText = QString("<b>%1</b>%2").arg(AF_STR_StartupDialogNewProjectDebuggingTitle).arg(AF_STR_StartupDialogNewProjectDebuggingDescription);
    m_pNewProjectButtonForDebug = new afRichTextButton(acGetIcon(AC_ICON_STARTUP_NEW_DEBUG, iconSize), buttonText);


    buttonText = QString("<b>%1</b>%2").arg(AF_STR_StartupDialogNewProjectProfilingTitle).arg(AF_STR_StartupDialogNewProjectProfilingDescription);
    m_pNewProjectButtonForProfile = new afRichTextButton(acGetIcon(AC_ICON_STARTUP_NEW_PROFILE, iconSize), buttonText);


    buttonText = QString("<b>%1</b>%2").arg(AF_STR_StartupDialogNewProjectAnalyzeTitle).arg(AF_STR_StartupDialogNewProjectAnalyzeDescription);
    m_pAnalyzeNewCLFile = new afRichTextButton(acGetIcon(AC_ICON_STARTUP_NEW_ANALYZE, iconSize), buttonText);


    buttonText = QString("<b>%1</b>%2").arg(AF_STR_StartupDialogAddCLFileAnalyzeTitle).arg(AF_STR_StartupDialogAddCLFileAnalyzeDescription);
    m_pAnalyzeAddCLFile = new afRichTextButton(acGetIcon(AC_ICON_STARTUP_NEW_ANALYZE_FILE, iconSize), buttonText);


    maxWidth = qMax((float)m_pNewProjectButton->CalculatedSize().width(), maxWidth);
    maxHeight = qMax((float)m_pNewProjectButton->CalculatedSize().height(), maxHeight);

    maxWidth = qMax((float)m_pNewProjectButtonForDebug->CalculatedSize().width(), maxWidth);
    maxHeight = qMax((float)m_pNewProjectButtonForDebug->CalculatedSize().height(), maxHeight);

    maxWidth = qMax((float)m_pNewProjectButtonForProfile->CalculatedSize().width(), maxWidth);
    maxHeight = qMax((float)m_pNewProjectButtonForProfile->CalculatedSize().height(), maxHeight);

    maxWidth = qMax((float)m_pAnalyzeAddCLFile->CalculatedSize().width(), maxWidth);
    maxHeight = qMax((float)m_pAnalyzeAddCLFile->CalculatedSize().height(), maxHeight);

    maxWidth = qMax((float)m_pAnalyzeNewCLFile->CalculatedSize().width(), maxWidth);
    maxHeight = qMax((float)m_pAnalyzeNewCLFile->CalculatedSize().height(), maxHeight);
    maxWidth += 10;

    // Re-draw the buttons with fixed width and height:
    m_pNewProjectButton->Draw(QSizeF(maxWidth, m_pNewProjectButton->CalculatedSize().height()));
    m_pNewProjectButtonForDebug->Draw(QSizeF(maxWidth, m_pNewProjectButtonForDebug->CalculatedSize().height()));
    m_pNewProjectButtonForProfile->Draw(QSizeF(maxWidth, m_pNewProjectButtonForProfile->CalculatedSize().height()));
    m_pAnalyzeNewCLFile->Draw(QSizeF(maxWidth, m_pAnalyzeNewCLFile->CalculatedSize().height()));
    m_pAnalyzeAddCLFile->Draw(QSizeF(maxWidth, m_pAnalyzeAddCLFile->CalculatedSize().height()));

    // Connect widgets to the slots:
    bool rc = connect(m_pNewProjectButton, SIGNAL(clicked()), this, SLOT(OnButtonClicked()));
    GT_ASSERT(rc);

    rc = connect(m_pNewProjectButtonForDebug, SIGNAL(clicked()), this, SLOT(OnButtonClicked()));
    GT_ASSERT(rc);

    rc = connect(m_pNewProjectButtonForProfile, SIGNAL(clicked()), this, SLOT(OnButtonClicked()));
    GT_ASSERT(rc);

    rc = connect(m_pAnalyzeNewCLFile, SIGNAL(clicked()), this, SLOT(OnButtonClicked()));
    GT_ASSERT(rc);

    rc = connect(m_pAnalyzeAddCLFile, SIGNAL(clicked()), this, SLOT(OnButtonClicked()));
    GT_ASSERT(rc);

    QLabel* pDescription = new QLabel(AF_STR_StartupDialogDescription);
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->setSpacing(2);

    pLayout->addWidget(pDescription, Qt::AlignLeft);
    pLayout->addSpacing(AF_STARTUP_DLG_DEFAULT_LAYOUT_SPACING);
    pLayout->addWidget(m_pNewProjectButton, 0, Qt::AlignLeft);

    pLayout->addSpacing(AF_STARTUP_DLG_DEFAULT_LAYOUT_SPACING);
    QLabel* pDebugLabel = new QLabel(AF_STR_StartupDialogDebugCaption);

    pDebugLabel->setStyleSheet(AF_STR_CSSBold);
    pLayout->addWidget(pDebugLabel);
    pLayout->addWidget(m_pNewProjectButtonForDebug, 0, Qt::AlignLeft);

    pLayout->addSpacing(AF_STARTUP_DLG_DEFAULT_LAYOUT_SPACING);
    QLabel* pProfileLabel = new QLabel(AF_STR_StartupDialogProfileCaption);

    pProfileLabel->setStyleSheet(AF_STR_CSSBold);

    pLayout->addWidget(pProfileLabel, 1, Qt::AlignLeft);
    pLayout->addWidget(m_pNewProjectButtonForProfile, 0, Qt::AlignLeft);

    pLayout->addSpacing(AF_STARTUP_DLG_DEFAULT_LAYOUT_SPACING);
    QLabel* pAnalyzeLabel = new QLabel(AF_STR_StartupDialogAnalyzeCaption);

    pAnalyzeLabel->setStyleSheet(AF_STR_CSSBold);
    pLayout->addWidget(pAnalyzeLabel);
    pLayout->addWidget(m_pAnalyzeNewCLFile, 0, Qt::AlignLeft);
    pLayout->addWidget(m_pAnalyzeAddCLFile, 0, Qt::AlignLeft);

    pLayout->addStretch();

    // Create the dialog buttons:
    QDialogButtonBox* pBox = new QDialogButtonBox(QDialogButtonBox::Cancel, Qt::Horizontal);


    pLayout->addStretch();
    pLayout->addSpacing(10);
    pLayout->addWidget(pBox, Qt::AlignRight);


    rc = connect(pBox, SIGNAL(rejected()), this, SLOT(reject()));
    GT_ASSERT(rc);

    setLayout(pLayout);

    QString bgStr = QString("QDialog{background-color:#%1;}").arg(acGetSystemDefaultBackgroundColorAsHexQString());
    setStyleSheet(bgStr);

    pAnalyzeLabel->setEnabled(true);
    m_pAnalyzeNewCLFile->setEnabled(true);
    m_pAnalyzeAddCLFile->setEnabled(true);
}

afStartupDialog::~afStartupDialog()
{
}

void afStartupDialog::OnButtonClicked()
{
    QWidget* pButton = qobject_cast<QAbstractButton*>(sender());

    if (pButton != nullptr)
    {
        if (pButton == m_pNewProjectButton)
        {
            m_userSelection = AF_NO_PROJECT_USER_ACTION_CREATE_NEW_PROJECT;
        }
        else if (pButton == m_pNewProjectButtonForDebug)
        {
            m_userSelection = AF_NO_PROJECT_USER_ACTION_CREATE_NEW_PROJECT_DEBUG;
        }
        else if (pButton == m_pNewProjectButtonForProfile)
        {
            m_userSelection = AF_NO_PROJECT_USER_ACTION_CREATE_NEW_PROJECT_PROFILE;
        }
        else if (pButton == m_pAnalyzeNewCLFile)
        {
            m_userSelection = AF_NO_PROJECT_USER_ACTION_NEW_FILE_FOR_ANALYZE;
        }
        else if (pButton == m_pAnalyzeAddCLFile)
        {
            m_userSelection = AF_NO_PROJECT_USER_ACTION_ADD_FILE_FOR_ANALYZE;
        }

        accept();
    }
}
