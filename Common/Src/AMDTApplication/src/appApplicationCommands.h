//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file appApplicationCommands.h
///
//==================================================================================

#ifndef __APPAPPLICATIONCOMMANDS_H
#define __APPAPPLICATIONCOMMANDS_H

// Infra:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>

// ----------------------------------------------------------------------------------
// Class Name:          appApplicationCommands : public afApplicationCommands
// General Description: The standalone CodeXL application's implementation of afApplicationCommands
// Author:              Uri Shomroni
// Creation Date:       3/5/2012
// ----------------------------------------------------------------------------------
class appApplicationCommands : public afApplicationCommands
{
public:
    appApplicationCommands();
    virtual ~appApplicationCommands();

    // Overrides afApplicationCommands:
    virtual void setViewLineNumbers(bool show);
    virtual bool closeFile(const osFilePath& filePath);

    /// \param filePath the full path of the file
    /// \param lineNumber line number, or -1 if not applicable
    /// \param programCounterIndex counter index for source files, or -1 if not applicable
    /// \param viewIndex used for internal implementation. Used for indexing of inner views
    virtual bool OpenFileAtLine(const osFilePath& filePath, int lineNumber, int programCounterIndex = -1, int viewIndex = -1);

    /// save all mdi windows that are related to the supplied filepath
    virtual void SaveAllMDISubWindowsForFilePath(const osFilePath& filePath);

    // Qt:
    virtual QWidget* applicationMainWindow();

    // Get the drag and drop action for the drop event:
    virtual afApplicationTree::DragAction DragActionForDropEvent(QDropEvent* pEvent);
};

#endif //__APPAPPLICATIONCOMMANDS_H

