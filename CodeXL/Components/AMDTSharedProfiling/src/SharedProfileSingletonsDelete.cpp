//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SharedProfileSingletonsDelete.cpp
///
//==================================================================================

//------------------------------ SharedProfileSingletonsDelete.cpp ------------------------------

#include <QtWidgets>
#include <QWidget>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <inc/ProfileApplicationTreeHandler.h>
#include <inc/SharedProfileSingletonsDelete.h>

// The static instance of this class. When the process that contains this file exits,
// the destructor of this single instance will be called. This destructor releases all
// the existing singleton objects.
static SharedProfileSingletonsDelete static_instance;


// ---------------------------------------------------------------------------
// Name:        SharedProfileSingletonsDelete::~SharedProfileSingletonsDelete
// Description: Destructor - release all the existing singleton objects.
// ---------------------------------------------------------------------------
SharedProfileSingletonsDelete::~SharedProfileSingletonsDelete()
{

    // Delete the kaGlobalVariableManager instance:
    OS_OUTPUT_DEBUG_LOG(L"Before deleting ProfileApplicationTreeHandler", OS_DEBUG_LOG_DEBUG);
    delete ProfileApplicationTreeHandler::m_pMySingleInstance;
    ProfileApplicationTreeHandler::m_pMySingleInstance = NULL;
    OS_OUTPUT_DEBUG_LOG(L"After deleting ProfileApplicationTreeHandler", OS_DEBUG_LOG_DEBUG);

}
