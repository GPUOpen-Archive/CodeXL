//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppMDIViewCreator.h
///
//==================================================================================

//------------------------------ ppMDIViewCreator.h ------------------------------

#ifndef __PPMDIVIEWCREATOR_H
#define __PPMDIVIEWCREATOR_H

// Qt:
#include <QWidget>

#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osChannel.h>

// Framework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afQtViewCreatorAbstract.h>

// Local:
#include <AMDTPowerProfiling/src/ppSessionController.h>


class PP_API ppMDIViewCreator : public afQtViewCreatorAbstract
{
public:

    ppMDIViewCreator();
    ~ppMDIViewCreator();

    // Virtual functions that needs to be implemented:

    // Get the title of the created view:
    virtual void titleString(int viewIndex, gtString& viewTitle, gtString& viewMenuCommand);

    // Get the associated toolbar string:
    virtual gtString associatedToolbar(int viewIndex);

    // Get view type:
    virtual afViewCreatorAbstract::afViewType type(int viewIndex);

    // Get the docking area:
    virtual int dockArea(int viewIndex);

    // True iff the view is creating views in run time:
    virtual bool isDynamic() { return true;};

    // For dynamic views: the event type which is used for these views creation:
    virtual const gtString CreatedMDIType() const { return AF_STR_PowerProfileViewsCreatorID; }

    // Dockable widget features:
    virtual QDockWidget::DockWidgetFeatures dockWidgetFeatures(int viewIndex);

    // Get the initial size:
    virtual QSize initialSize(int viewIndex);

    // Get the initial visibility of the view:
    virtual bool visibility(int viewIndex);

    // Get number of views that can be created by this creator:
    virtual int amountOfViewTypes();

    // Handle the action when it is triggered:
    virtual void handleTrigger(int viewIndex, int actionIndex);

    // Handle UI update:
    virtual void handleUiUpdate(int viewIndex, int actionIndex);

    // Create the context for the view:
    virtual bool createViewContent(int viewIndex, QWidget*& pContentQWidget, QWidget* pQParent);

    // Display the view containing the content specified in the event:
    virtual bool displayExistingView(const apMDIViewCreateEvent& mdiViewEvent);

    // Get the file that is currently displayed:
    virtual bool getCurrentlyDisplayedFilePath(osFilePath& filePath);

    // Events:
    virtual bool onMDISubWindowClose(afQMdiSubWindow* pMDISubWindow);

    /// Is called after a session was renamed:
    /// \param pRenamedSessionData the item data for the renamed session
    /// \param oldSessionFilePath the file path before the rename
    /// \param oldSessionDir  the old session folder
    void OnSessionRename(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath, const osDirectory& oldSessionDir);

    /// Is called before a session is renamed:
    /// \param pAboutToRenameSessionData the item data for the session which is about to be renamed
    /// \param isRenameEnabled is the rename enabled?
    /// \param renameDisableMessage message for the user is the rename is disabled
    void OnBeforeSessionRename(SessionTreeNodeData* pAboutToRenameSessionData, bool& isRenameEnabled, QString& renameDisableMessage);


    /// Is called when a session window should be activated (start listening to events):
    /// \param pSessionData the item data representing the session that should be activated
    /// \return true for success false for failure (window not found)
    bool ActivateSession(SessionTreeNodeData* pSessionData);

    /// Is called when a session is about to be deleted:
    /// \param deletedSessionFilePath the file path for the deleted session
    void OnSessionDelete(const gtString& deletedSessionFilePath);

private:


    // map that holds the views that were created so far in order to prevent creating same views again:
    gtMap<gtString, QWidget*> m_createdViewsMap;
};


#endif //__PPMDIVIEWCREATOR_H

