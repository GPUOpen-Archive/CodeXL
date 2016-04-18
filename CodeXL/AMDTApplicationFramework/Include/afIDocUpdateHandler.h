//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afIDocUpdateHandler.h
///
//==================================================================================

#ifndef __AFIDOCUPDATEHANDLER_H
#define __AFIDOCUPDATEHANDLER_H

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

class AF_API afIDocUpdateHandler
{
public:
    /// Handler the document update event:
    virtual void UpdateDocument(const osFilePath& docToUpdate) = 0;

};


#endif  // __AFIDOCUPDATEHANDLER_H
