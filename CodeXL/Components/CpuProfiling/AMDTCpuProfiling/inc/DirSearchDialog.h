//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DirSearchDialog.h
/// \brief  derivative of the iDirSearch.ui
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/DirSearchDialog.h#7 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================
#ifndef _DIRSEARCHDIALOG_H
#define _DIRSEARCHDIALOG_H

// Qt:
#include <QtCore>
#include <QtWidgets>
#include <QDialog>

// Generated:
#include <tmp/ui/ui_iDirSearch.h>

class DirSearchDialog : public QDialog, public Ui_iDirSearch
{
    Q_OBJECT

public:
    DirSearchDialog(QWidget* parent = 0,  Qt::WindowFlags fl = Qt::Dialog | Qt::WindowTitleHint);
    ~DirSearchDialog();

    void setDirList(QString dirList);
    QString getDirList();

private slots:
    void onAdd();
    void onRemove();
    void onUp();
    void onDown();
    void onDoubleClick(int row);
    void onCurrentChanged(int row);
private:
    void setCurrentRow(int row);
};
#endif //_DIRSEARCHDIALOG_H
