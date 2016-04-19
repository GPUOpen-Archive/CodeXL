//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/EditNameValueDialog.cpp $
/// \version $Revision: #4 $
/// \brief :  This file contains EditNameValueDialog class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/EditNameValueDialog.cpp#4 $
// Last checkin:   $DateTime: 2015/08/31 01:56:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538740 $
//=====================================================================
#include <qtIgnoreCompilerWarnings.h>

#include <QtWidgets/qmessagebox.h>


#include "EditNameValueDialog.h"


EditNameValueDialog::EditNameValueDialog(QString& name, QString& value, bool openInAddMode)
{
    setupUi(this);

    nameTextBox->setText(name);
    valueTextBox->setText(value);
    nameTextBox->setReadOnly(!openInAddMode);


    connect(nameTextBox, SIGNAL(textChanged(QString)), this, SLOT(onNameTextBox_TextChange()));
    connect(valueTextBox, SIGNAL(textChanged(QString)), this, SLOT(onValueTextBox_TextChange()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(onOkButton_Click()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(onCancelButton_Click()));

    setFixedSize(324, 106);
    setWindowTitle(QString("%1 Environment Variable").arg((openInAddMode ? "Add" : "Edit")));

    if (!openInAddMode)
    {
        valueTextBox->setFocus();
    }
}


void EditNameValueDialog::onNameTextBox_TextChange()
{
    NameIsValid(true);
    okButton->setEnabled(InputsAreValid(false));
}

void EditNameValueDialog::onValueTextBox_TextChange()
{
    ValueIsValid(true);
    okButton->setEnabled(InputsAreValid(false));
}

void EditNameValueDialog::onOkButton_Click()
{
    if (InputsAreValid(true))
    {
        accept();
    }
}

void EditNameValueDialog::onCancelButton_Click()
{
    reject();
}

bool EditNameValueDialog::ValueIsValid(bool showMsg)
{
    bool result = true;
    int index = -1;

    if (-1 != (index = valueTextBox->text().indexOf("=")))
    {
        result = false;

        if (showMsg)
        {
            Util::ShowWarningBox("Value cannot contain \"=\"");
            valueTextBox->setFocus();
        }
    }

    return result;
}

bool EditNameValueDialog::NameIsValid(bool showMsg)
{
    bool result = true;
    int index = -1;

    if (nameTextBox->text().trimmed().isEmpty())
    {
        result = false;

        if (showMsg)
        {
            Util::ShowWarningBox("Enter variable name");
            nameTextBox->setFocus();
        }
    }
    else if (-1 != (index = nameTextBox->text().indexOf("=")))
    {
        result = false;

        if (showMsg)
        {
            Util::ShowWarningBox("Name cannot contain \"=\"");
            nameTextBox->setFocus();
        }
    }

    return result;
}


bool EditNameValueDialog::InputsAreValid(bool showMsg)
{
    return NameIsValid(showMsg) && ValueIsValid(showMsg);
}

void EditNameValueDialog::GetNameValue(QString& name, QString& value)
{
    name = nameTextBox->text().trimmed();
    value = valueTextBox->text().trimmed();
}


