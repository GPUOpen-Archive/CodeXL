//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afHelpAboutDialog.h
///
//==================================================================================

#ifndef __AFHELPABOUTDIALOG
#define __AFHELPABOUTDIALOG

// QT:
#include <QDialog>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osApplication.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationComponents/Include/acHelpAboutDialog.h>

// ----------------------------------------------------------------------------------
// Class Name:          GD_API afHelpAboutDialog : public acHelpAboutDialog
// General Description: help dialog screen
//
// Author:              Yoni Rabin
// Creation Date:       9/4/2012
// ----------------------------------------------------------------------------------
class AF_API afHelpAboutDialog : public acHelpAboutDialog
{
    Q_OBJECT
public:

    afHelpAboutDialog(osExecutedApplicationType executionApplicationType, QWidget* pParent);

};

#endif  // __AFHELPABOUTDIALOG
