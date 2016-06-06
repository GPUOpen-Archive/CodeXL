//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acSingeltonsDelete.cpp
///
//==================================================================================

//------------------------------ acSingeltonsDelete.cpp ------------------------------

// Local:
#include <inc/acSingeltonsDelete.h>
#include <AMDTApplicationComponents/Include/acSourceCodeLanguageHighlighter.h>
#include <AMDTApplicationComponents/Include/acSoftwareUpdaterProxySetting.h>

// A static instance of the singleton deleter class. Its destructor will delete all
// this GRApplicationComponents library singletons.
static acSingeltonsDelete singeltonDeleter;

// ---------------------------------------------------------------------------
// Name:        acSingeltonsDelete::~acSingeltonsDelete
// Description: Destructor - deletes all the singleton instances.
// Author:      Sigal Algranaty
// Date:        9/8/2011
// ---------------------------------------------------------------------------
acSingeltonsDelete::~acSingeltonsDelete()
{
    // Delete the acSourceCodeLanguageHighlighter single instance:
    delete acSourceCodeLanguageHighlighter::_pMySingleInstance;
    acSourceCodeLanguageHighlighter::_pMySingleInstance = NULL;

    delete acSoftwareUpdaterProxySetting::m_proxyCheckingThread;
    acSoftwareUpdaterProxySetting::m_proxyCheckingThread = NULL;
}


