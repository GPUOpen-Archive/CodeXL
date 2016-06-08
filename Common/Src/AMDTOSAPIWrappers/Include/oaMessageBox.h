//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================

//------------------------------ oaMessageBox.h ------------------------------

#ifndef __OAMESSAGEBOX
#define __OAMESSAGEBOX

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osMessageBox.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaOSAPIWrappersDLLBuild.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>

void oaMessageBoxDisplayCB(const gtString& title, const gtString& message, osMessageBox::osMessageBoxIcon icon, /*oaWindowHandle*/ void* hParentWindow);


#endif  // __OSMESSAGEBOX
