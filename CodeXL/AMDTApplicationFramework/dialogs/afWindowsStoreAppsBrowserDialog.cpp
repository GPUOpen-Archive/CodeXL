//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afWindowsStoreAppsBrowserDialog.cpp
///
//==================================================================================

//Qt
#include <QtWidgets>

// Infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTApplicationFramework/Include/dialogs/afWindowsStoreAppsBrowserDialog.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>


afWindowsStoreAppsBrowserDialog::afWindowsStoreAppsBrowserDialog(const QString& userModelId) :
    QDialog(afMainAppWindow::instance())
{
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    setWindowTitle(AF_STR_winStoreAppWindowTitle);

    createDialogLayout();

    fillStoreAppsList(userModelId);
}

afWindowsStoreAppsBrowserDialog::~afWindowsStoreAppsBrowserDialog()
{
}

void afWindowsStoreAppsBrowserDialog::createDialogLayout()
{
    m_pAppsList = new QListWidget;
    connect(m_pAppsList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onAppSelected(QListWidgetItem*)));

    QLabel* pUserModelIdLabel = new QLabel;
    pUserModelIdLabel->setText(AF_STR_winStoreAppUserModelIDLabel);
    m_pUserModelIdTextEdit = new QLineEdit;
    m_pUserModelIdTextEdit->setReadOnly(true);

    QHBoxLayout* pUserModelIdLayout = new QHBoxLayout;
    pUserModelIdLayout->addWidget(pUserModelIdLabel);
    pUserModelIdLayout->addWidget(m_pUserModelIdTextEdit);


    // Create the dialog buttons:
    QDialogButtonBox* pButtonBox = new QDialogButtonBox();


    GT_IF_WITH_ASSERT(nullptr != pButtonBox)
    {
        QPushButton* pOpenButton = new QPushButton(AF_STR_OK_Button);


        QPushButton* pCancelButton = new QPushButton(AF_STR_Cancel_Button);


        pButtonBox->addButton(pOpenButton, QDialogButtonBox::AcceptRole);
        pButtonBox->addButton(pCancelButton, QDialogButtonBox::RejectRole);
    }

    // Connect the buttons:
    bool rc = connect(pButtonBox, SIGNAL(accepted()), this, SLOT(onOpenButton()));
    GT_ASSERT(rc);
    rc = connect(pButtonBox, SIGNAL(rejected()), this, SLOT(onCancelButton()));
    GT_ASSERT(rc);


    // Create the main layout:
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addWidget(m_pAppsList);
    pMainLayout->addLayout(pUserModelIdLayout);
    pMainLayout->addWidget(pButtonBox);

    setLayout(pMainLayout);
}

void afWindowsStoreAppsBrowserDialog::onOpenButton()
{
    done(Accepted);
}

void afWindowsStoreAppsBrowserDialog::onCancelButton()
{
    done(Rejected);
}

void afWindowsStoreAppsBrowserDialog::onAppSelected(QListWidgetItem* pItem)
{
    GT_IF_WITH_ASSERT(nullptr != pItem)
    {
        m_selectedAppName = pItem->text();

        QStringList dataList = pItem->data(Qt::UserRole).toStringList();
        GT_ASSERT(dataList.size() == 2);

        m_pUserModelIdTextEdit->setText(dataList.at(0));
        m_packageDirectory = dataList.at(1);
    }
}

void afWindowsStoreAppsBrowserDialog::fillStoreAppsList(const QString& currentUserModelId)
{
    gtList<WindowsStoreAppInfo> storeApps;

    if (osEnumerateInstalledWindowsStoreApps(storeApps))
    {
        QListWidgetItem* pSelectedItem = nullptr;

        for (gtList<WindowsStoreAppInfo>::const_iterator it = storeApps.begin(), itEnd = storeApps.end(); it != itEnd; ++it)
        {
            const WindowsStoreAppInfo& storeAppInfo = *it;

            if (0 == storeAppInfo.m_userModelId.compareNoCase(L"Microsoft.Windows.Desktop"))
            {
                continue;
            }

            QImage logoImage(acGTStringToQString(storeAppInfo.m_logoPath.asString()));
            QIcon icon = createIcon(logoImage, storeAppInfo.m_backgroundColor);

            QListWidgetItem* pItem = new QListWidgetItem(icon, acGTStringToQString(storeAppInfo.m_name));
            QStringList dataList;

            QString userModelId = acGTStringToQString(storeAppInfo.m_userModelId);
            dataList.push_back(userModelId);

            QString packageDirectory = acGTStringToQString(storeAppInfo.m_packageDirectory);
            dataList.push_back(packageDirectory);

            pItem->setData(Qt::UserRole, QVariant(dataList));
            pItem->setSizeHint(QSize(0, 26));
            m_pAppsList->addItem(pItem);

            if (nullptr == pSelectedItem && !currentUserModelId.isEmpty() &&
                0 == currentUserModelId.compare(userModelId, Qt::CaseInsensitive))
            {
                pSelectedItem = pItem;
            }
        }

        m_pAppsList->sortItems();

        if (nullptr == pSelectedItem)
        {
            pSelectedItem = m_pAppsList->item(0);
        }

        if (nullptr != pSelectedItem)
        {
            m_pAppsList->setCurrentItem(pSelectedItem);
            onAppSelected(pSelectedItem);
        }
    }
}

QIcon afWindowsStoreAppsBrowserDialog::createIcon(const QImage& logoImage, QRgb backgroundColor)
{
    QImage logoWithBackground = QImage(logoImage.size(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&logoWithBackground);

    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(logoWithBackground.rect(),
                     QColor(qBlue(backgroundColor),
                            qGreen(backgroundColor),
                            qRed(backgroundColor),
                            qAlpha(backgroundColor)));

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(0, 0, logoImage);

    painter.end();

    return QIcon(QPixmap::fromImage(logoWithBackground));
}
