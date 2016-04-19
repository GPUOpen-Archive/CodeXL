//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ListViewWindow.cpp $
/// \version $Revision: #3 $
/// \brief :  This file contains ListViewWindow
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ListViewWindow.cpp#3 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================
#include "ListViewWindow.h"


ListViewWindow::ListViewWindow()
{
    setupUi(this);

    connect(okButton,
            SIGNAL(clicked()),
            this,
            SLOT(OkButton_Click())
           );
    itemListBox->setAlternatingRowColors(true);
}

ListViewWindow::~ListViewWindow()
{
}

void ListViewWindow::OkButton_Click()
{
    GlobalSettings::Instance()->m_generalOpt.m_showDetailDeletion = showDeleteSummaryCheckBox->isChecked();
    GlobalSettings::Instance()->Save();
    QDialog::accept();
}

void ListViewWindow::SetDataAndConfig(
    const QString& dlgHeading,
    const QString& itemDescription,
    const QStringList& items,
    bool showDetailDeletion)
{
    setWindowTitle(dlgHeading);

    showDeleteSummaryCheckBox->setChecked(showDetailDeletion);

    foreach (QString s, items)
    {
        if (s.trimmed().isEmpty())
        {
            continue;
        }
        else
        {
            itemListBox->addItem(s);
        }
    }

    groupBoxItemList->setTitle(itemDescription);
}


