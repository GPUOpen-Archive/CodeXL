//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SessionExplorerDefs.h
///
//==================================================================================

#ifndef _SESSION_EXPLORER_DEFS_H_
#define _SESSION_EXPLORER_DEFS_H_

/// type used to identify sessions
typedef unsigned int ExplorerSessionId;

/// static const returned from AddSession when adding fails
static const ExplorerSessionId SESSION_ID_ERROR = (ExplorerSessionId) - 1;

/// enum used to describe whether or not a session should be removed from the UI only or from disk as well
enum SessionExplorerDeleteType
{
    /// indicates that a session should be removed from the Session Explorer Tree view only
    SESSION_EXPLORER_REMOVE_FROM_TREE_ONLY,

    /// indicates that the seesion should be removed from the Session Explorer Tree View and from disk
    SESSION_EXPLORER_REMOVE_FILES
};



#endif // _SESSION_EXPLORER_DEFS_H_
