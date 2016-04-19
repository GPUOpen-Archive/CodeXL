//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/EditNameValueDialog.h $
/// \version $Revision: #3 $
/// \brief :  This file contains EditNameValue dialog
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/EditNameValueDialog.h#3 $
// Last checkin:   $DateTime: 2015/08/31 01:56:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538740 $
//=====================================================================
#ifndef _EDIT_NAME_VALUE_DIALOG_H_
#define _EDIT_NAME_VALUE_DIALOG_H_
#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include "ui_EditNameValue.h"


#include <AMDTGpuProfiling/Util.h>


/// dialog to edit a name value pair
class EditNameValueDialog : public QDialog, private Ui::EditNameValue
{
    Q_OBJECT

public:

    /// Initializes a new instance of the EditNameValueDialog class.
    /// \param name name to set
    /// \param value value to set
    /// \param openInAddMode open mode
    EditNameValueDialog(QString& name, QString& value, bool openInAddMode);

    /// Get selected fields
    /// \param name name of variable
    /// \param value value of variable
    void  GetNameValue(QString& name, QString& value);
private:

    /// Form load event handler to change the caption of form
    void EditNameValue_Load();

    /// validates the field "value"
    /// \param showMsg to display message or not
    /// \return true if value is valid else false
    bool ValueIsValid(bool showMsg);

    /// validates the field "name"
    /// \param showMsg to display message or not
    /// \return true if name is valid else false
    bool NameIsValid(bool showMsg);


    /// validates the fields
    /// \param showMsg to display message or not
    /// \return true if inputs are valid else false
    bool InputsAreValid(bool showMsg);

private slots:
    /// Name text box change handler
    void onNameTextBox_TextChange();

    /// Value text box change handler
    void onValueTextBox_TextChange();

    /// Ok Button Click handler
    void onOkButton_Click();

    /// Cancel Button Click handler
    void onCancelButton_Click();
};

#endif // _EDIT_NAME_VALUE_DIALOG_H_

