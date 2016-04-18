//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afMultipleDirectoriesBrowseDialog.h
///
//==================================================================================

#ifndef __AFMULTIPLEDIRECTORIESBROWSEDIALOG
#define __AFMULTIPLEDIRECTORIESBROWSEDIALOG

// Qt:
#include <QtCore>
#include <QtWidgets>
#include <qdialog.h>

// Infra:
#include <AMDTApplicationComponents/Include/acDialog.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>

class afMultipleDirectoriesBrowseDialog : public acDialog
{
    Q_OBJECT

public:
    afMultipleDirectoriesBrowseDialog(QWidget* pParent = 0);
    ~afMultipleDirectoriesBrowseDialog();

    /// Directories list as string accessors:
    void SetDirectoriesList(const QString& dirList);
    QString GetDirectoriesList();

private slots:

    void OnAddDirectory();
    void OnRemoveDirectory();
    void OnUp();
    void OnDown();
    void OnDoubleClick(int row);
    void OnCurrentChanged();

private:

    void SetCurrentRow(int row);
    void SetDialogLayout();

private:

    QPushButton* m_pUpButton;
    QPushButton* m_pDownButton;
    QPushButton* m_pNewDirectoryButton;
    QPushButton* m_pRemoveDirectoryButton;

    acListCtrl* m_pDirTable;
    QTableWidgetItem* m_pLastClickedItem;
};

#endif // __AFMULTIPLEDIRECTORIESBROWSEDIALOG