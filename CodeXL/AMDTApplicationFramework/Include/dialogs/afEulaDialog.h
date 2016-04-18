//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afEulaDialog.h
///
//==================================================================================

#ifndef __AFEULADIALOG
#define __AFEULADIALOG


// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// AMDTApplicationComponents
#include <AMDTApplicationComponents/Include/acEulaDialog.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>



// ----------------------------------------------------------------------------------
// Class Name:              afEulaDialog : public QDialog
// General Description:     Pops before the first use of the application. User must
//                          accept the terms of agreement.
// Author:               Guy Ilany
// Creation Date:        19/12/2007
// ----------------------------------------------------------------------------------
class AF_API afEulaDialog
{

public:
    afEulaDialog(QWidget* parent);
    ~afEulaDialog();

    void setHtmlStringIntoDialog();
    int execute();

private:
    acEulaDialog m_dlg;
};


#endif  // __AFEULADIALOG
