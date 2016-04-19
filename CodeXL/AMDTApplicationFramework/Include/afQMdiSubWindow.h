//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afQMdiSubWindow.h
///
//==================================================================================

#ifndef __AFQMDISUBWINDOW_H
#define __AFQMDISUBWINDOW_H

// Qt:
#include <QMdiSubWindow>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

// Forward declaration:
class afViewCreatorAbstract;


// ----------------------------------------------------------------------------------
// Class Name:           AF_API afQMdiSubWindow : public QMdiSubWindow
// General Description: When adding an MDI window to the QT main frame use this class
//                      as sub window, since it holds the information we need for managing
//                      the MDI windowing
// Author:               Sigal Algranaty
// Creation Date:        2/8/2011
// ----------------------------------------------------------------------------------
class AF_API afQMdiSubWindow : public QMdiSubWindow
{
    Q_OBJECT

public:

    afQMdiSubWindow(afViewCreatorAbstract* pViewCreator);
    virtual ~afQMdiSubWindow();

    /// Toolbar:
    void setAssociatedToolbarName(const gtString& toolbarName) {_associatedToolbarName = toolbarName;}
    gtString associatedToolbarName() const {return _associatedToolbarName;}

    /// View creator:
    afViewCreatorAbstract* viewCreator() const {return _pViewCreator;};

    /// File path associated with this view:
    osFilePath filePath() const { return _displayedFilePath; }

    /// Sets the file path of the mdi view
    void setFilePath(const osFilePath& filePath);

    /// Sets if the windows is allowed to be closed:
    void SetAllowedToBeClosed(bool allowedToBeClosed);

    /// Returns id the window is allowed to be closed:
    bool IsAllowedToBeClosed() { return m_isAllowedToBeClosed; }

    /// Set if the window get a close event while it was blocked from closing:
    void SetCloseWhileBlockedFromClosing() { m_closeMdiWhileBlocked = true; }

protected slots:
    /// Handle the copyPath action:
    void OnCopyPathAction();

    /// Open the containing folder of the file:
    void OnOpenContainingFolder();

protected:

    /// Override:
    virtual void closeEvent(QCloseEvent* pCloseEvent);

    /// Displayed file path:
    osFilePath _displayedFilePath;

    /// Associated toolbar name:
    gtString _associatedToolbarName;

    /// The view creator:
    afViewCreatorAbstract* _pViewCreator;

    /// True iff the sub window is active:
    bool _isActive;

    /// Is the mdi window allowed to be closed at all by an event
    bool m_isAllowedToBeClosed;

    /// A flag that shows that a close event was reached while closed disabled was set.
    bool m_closeMdiWhileBlocked;
};

#endif // __AFQMDISUBWINDOW_H
