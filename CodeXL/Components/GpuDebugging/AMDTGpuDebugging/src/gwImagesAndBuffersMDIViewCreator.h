//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwImagesAndBuffersMDIViewCreator.h
///
//==================================================================================

//------------------------------ gwImagesAndBuffersMDIViewCreator.h ------------------------------

#ifndef __GWIMAGESANDBUFFERSMDIVIEWCREATOR_H
#define __GWIMAGESANDBUFFERSMDIVIEWCREATOR_H

// Infra:
#include <AMDTApplicationFramework/Include/afQtViewCreatorAbstract.h>

// Forward declaration:
class gdDebugApplicationTreeData;

class gwImagesAndBuffersMDIViewCreator : public afQtViewCreatorAbstract
{
public:

    gwImagesAndBuffersMDIViewCreator();
    ~gwImagesAndBuffersMDIViewCreator();

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
    virtual const gtString CreatedMDIType() const { return AF_STR_ImageBuffersViewsCreatorID; };

    // Docable widget features:
    virtual QDockWidget::DockWidgetFeatures dockWidgetFeatures(int viewIndex);

    // Return the displayed file path:
    virtual bool getCurrentlyDisplayedFilePath(osFilePath& filePath);

    // Get the initial size:
    virtual QSize initialSize(int viewIndex);

    // Get the initial visibility of the view:
    virtual bool visibility(int viewIndex);

    // Get number of views that can be created by this creator:
    virtual int amountOfViewTypes();

    // Create the inner view:
    virtual bool createViewContent(int viewIndex, QWidget*& pContentQWidget, QWidget* pQParent);

    // Display the view containing the content specified in the event:
    virtual bool displayExistingView(const apMDIViewCreateEvent& mdiViewEvent);

    // Handle sub window close:
    virtual bool onMDISubWindowClose(afQMdiSubWindow* pMDISubWindow);

    // Handle the action when it is triggered:
    virtual void handleTrigger(int viewIndex, int actionIndex);

    // Handle UI update:
    virtual void handleUiUpdate(int viewIndex, int actionIndex);

protected:

    afApplicationTreeItemData* getMatchingTreeItemData(gdDebugApplicationTreeHandler* pMonitoredObjectTree);


protected:

    // The application main command handler:
    gdApplicationCommands* _pApplicationCommandsHandler;
};


#endif //__GWIMAGESANDBUFFERSMDIVIEWCREATOR_H

