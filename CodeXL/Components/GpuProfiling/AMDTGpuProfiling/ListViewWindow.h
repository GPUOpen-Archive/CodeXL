//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ListViewWindow.h $
/// \version $Revision: #5 $
/// \brief :  This file contains ListViewWindow class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ListViewWindow.h#5 $
// Last checkin:   $DateTime: 2015/08/31 01:56:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538740 $
//=====================================================================

#ifndef _LIST_VIEW_WINDOW_H_
#define _LIST_VIEW_WINDOW_H_

#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#include <QtCore/qstring.h>
#include <QtWidgets/qlistwidget.h>
#include <QtWidgets/qdialog.h>
#include "ui_ListViewDialog.h"

#include "GlobalSettings.h"

/// dialog used to show list data (currently unused in CodeXL -- designed to show deleted files)
class ListViewWindow : public QDialog, private Ui::ListViewDialog
{
    Q_OBJECT
public:
    /// Constructor
    ListViewWindow();

    /// Destructor
    ~ListViewWindow();

    /// To configure different labels and set data to be displayed
    /// \param dlgHeading Heading of the Dialog
    /// \param itemDescription Description of the Items listed here
    /// \param items items in the list
    /// \param showDetailDeletion to show details deletion or not
    void SetDataAndConfig(
        const QString& dlgHeading,
        const QString& itemDescription,
        const QStringList& items,
        bool showDetailDeletion);

private slots:
    /// Ok BUtton click handler
    void OkButton_Click();


};


#endif //_LIST_VIEW_WINDOW_H_



