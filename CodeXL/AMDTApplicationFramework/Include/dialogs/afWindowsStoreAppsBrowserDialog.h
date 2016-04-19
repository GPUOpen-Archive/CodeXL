//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afWindowsStoreAppsBrowserDialog.h
///
//==================================================================================

#ifndef __AFWINDOWSSTOREAPPSBROWSERDIALOG_H
#define __AFWINDOWSSTOREAPPSBROWSERDIALOG_H

// Qt:
#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QImage>
#include <QRgb>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>


class AF_API afWindowsStoreAppsBrowserDialog : public QDialog
{
    Q_OBJECT

public:
    afWindowsStoreAppsBrowserDialog(const QString& userModelId);
    virtual ~afWindowsStoreAppsBrowserDialog();

    QString GetUserModelID() const { return m_pUserModelIdTextEdit->text(); }
    const QString& GetAppName() const { return m_selectedAppName; }
    const QString& GetPackageDirectory() const { return m_packageDirectory; }

protected slots:
    void onOpenButton();
    void onCancelButton();
    void onAppSelected(QListWidgetItem* pItem);

private:
    void createDialogLayout();
    void fillStoreAppsList(const QString& userModelId);
    QIcon createIcon(const QImage& logoImage, QRgb backgroundColor);

    QListWidget* m_pAppsList;
    QLineEdit* m_pUserModelIdTextEdit;
    QString m_selectedAppName;
    QString m_packageDirectory;
};

#endif //__AFWINDOWSSTOREAPPSBROWSERDIALOG_H

