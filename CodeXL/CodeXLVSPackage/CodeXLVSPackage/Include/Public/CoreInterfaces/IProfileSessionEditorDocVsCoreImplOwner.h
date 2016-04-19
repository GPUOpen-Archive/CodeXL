//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file IProfileSessionEditorDocVsCoreImplOwner.h
///
//==================================================================================

#ifndef IProfileSessionEditorDocVsCoreImplOwner_h__
#define IProfileSessionEditorDocVsCoreImplOwner_h__

// This is an interface that allows the VS package components
// (which have no access to AMDT/Qt logic) to handle core events (such as Qt events).
class IProfileSessionEditorDocVsCoreImplOwner
{
public:
    virtual ~IProfileSessionEditorDocVsCoreImplOwner() {}

    // Core event: after session is renamed.
    // Note that the caller has to free oldSessionFilePath.
    // Using the core's ReleaseMemory function.
    virtual void ceOnAfterSessionRenamed(wchar_t* oldSessionFilePath) = 0;

    // Core event: after explorer ready for sessions.
    virtual void ceOnExplorerReadyForSessions() = 0;

};

#endif // IProfileSessionEditorDocVsCoreImplOwner_h__

