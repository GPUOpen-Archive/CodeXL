//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acQHTMLWindow.h
///
//==================================================================================

//------------------------------ acQHTMLWindow.h ------------------------------

#ifndef __ACQHTMLWINDOW_H
#define __ACQHTMLWINDOW_H

// Qt:
#include <QDataStream>
#include <QTextBrowser>


// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:          AC_API acQHTMLWindow : public QTextBrowser
// General Description: Is used for HTML browsing
// Author:              Sigal Algranaty
// Creation Date:       29/11/2011
// ----------------------------------------------------------------------------------
class AC_API acQHTMLWindow : public QTextBrowser
{
    Q_OBJECT

public:

    // Self functions:
    acQHTMLWindow(QWidget* pParent);

public:

    // Overriding QTextBrowser to avoid Qt warning when trying to open a file:
    virtual QVariant loadResource(int type, const QUrl& name) { (void)(type); (void)(name); return true;};

public slots:

    // Edit commands implementations:
    void onEditCopy();
    void onEditSelectAll();
    void onContextMenuEvent(const QPoint& point);
    void onAboutToShowContextMenu();
    virtual void onLinkClicked(const QUrl& link);
    virtual void setSource(const QUrl& name);

protected:

    void initContextMenu();

    // QTextBrowser overrides:
    void keyPressEvent(QKeyEvent* pKeyEvent);
    void onUpdateEditCopy(bool& isEnabled);
    void onUpdateEditSelectAll(bool& isEnabled);

protected:

    // Context menu:
    QMenu* _pContextMenu;
    QAction* _pCopyAction;
    QAction* _pSelectAllAction;

};


#endif //__ACQHTMLWINDOW_H

