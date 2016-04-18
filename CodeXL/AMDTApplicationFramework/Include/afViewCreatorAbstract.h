//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afViewCreatorAbstract.h
///
//==================================================================================

#ifndef __AFVIEWCREATORABSTRACT_H
#define __AFVIEWCREATORABSTRACT_H

// Qt:
#include <QDockWidget>

// Forward declaration:
class afActionCreatorAbstract;

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewCreateEvent.h>
#include <AMDTApplicationComponents/Include/acIcons.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationFramework/Include/afQMdiSubWindow.h>
#include <AMDTApplicationFramework/Include/afActionPositionData.h>

// ----------------------------------------------------------------------------------
// Class Name:          afViewCreatorAbstract
// General Description: abstract class for the view creator
//                      A view creator can create several views. using the API to see
//                      how many views can be create.
//
// Author:              Gilad Yarnitzky
// Creation Date:       14/7/2011
// ----------------------------------------------------------------------------------
class AF_API afViewCreatorAbstract
{
public:

    typedef enum
    {
        AF_VIEW_dock = 0,
        AF_VIEW_mdi = 1,
        AF_AMOUNT_OF_VIEW_TYPES
    } afViewType;

    typedef enum
    {
        AF_VIEW_DOCK_LeftDockWidgetArea = 0x1,
        AF_VIEW_DOCK_RightDockWidgetArea = 0x2,
        AF_VIEW_DOCK_TopDockWidgetArea = 0x4,
        AF_VIEW_DOCK_BottomDockWidgetArea = 0x8
    } afDockingAreaFlag;


    afViewCreatorAbstract();
    virtual ~afViewCreatorAbstract();

    /// Initialize the creator:
    virtual void initialize();

    /// Get the title of the created view, and the view menu command (that contain accelerator):
    virtual void titleString(int viewIndex, gtString& viewTitle, gtString& viewMenuCommand) = 0;

    /// Get the execution mode of the view:
    virtual gtString executionMode(int viewIndex) { (void)(viewIndex); return L""; }

    /// Get the menu position in the menu view:
    virtual gtString modeMenuPosition(int viewIndex, afActionPositionData& positionData) { (void)(viewIndex); (void)(positionData); return L""; }

    /// If one of the views contain toolbar, this function updates it's commands:
    virtual void updateViewToolbarCommands() {};

    /// Get the associated toolbar string:
    virtual gtString associatedToolbar(int viewIndex) = 0;

    /// Get view type:
    virtual afViewType type(int viewIndex) = 0;

    /// Get the docking area:
    virtual int dockArea(int viewIndex) = 0;

    /// Should add a separator after view command?  - false by default, override when a separator is needed:
    virtual bool addSeparator(int viewIndex) { (void)(viewIndex); return false;};

    /// Get the dock view we want to dock with. "-" prefix means below that view
    virtual gtString dockWith(int viewIndex) { (void)(viewIndex); return L""; } ;

    /// True iff the view is creating views in run time:
    virtual bool isDynamic() = 0;

    /// Get the docking features:
    virtual QDockWidget::DockWidgetFeatures dockWidgetFeatures(int viewIndex) = 0;

    /// Create the inner view:
    virtual bool createViewContent(int viewIndex, QWidget*& pContentQWidget, QWidget* pQParent) = 0;

    /// Get the initial size:
    virtual QSize initialSize(int viewIndex) = 0;

    /// Get the initial visibility of the view:
    virtual bool visibility(int viewIndex) = 0;

    /// Get the initial activity of the view:
    virtual bool initiallyActive(int viewIndex) { (void)(viewIndex); return false;};

    /// Get number of views that was created by this creator:
    virtual int amountOfCreatedViews() = 0;

    /// Get number of types of views that are supported by this creator:
    virtual int amountOfViewTypes() { return 0; }

    /// Handle the action when it is triggered
    virtual void handleTrigger(int viewIndex, int actionIndex) = 0;

    /// handle UI update
    virtual void handleUiUpdate(int viewIndex, int actionIndex) = 0;

    /// Get the icon file:
    virtual QPixmap* iconAsPixmap(int viewIndex);

    /// For dynamic views: re-implement this function to match apMDIViewCreateEvent::CreatedMDIType
    virtual const gtString CreatedMDIType() const;

    /// Set the creation event:
    virtual void setCreationEvent(apEvent* pCreationEvent) {_pCreationEvent = pCreationEvent;};

    /// Restore my created view size:
    virtual void restoreMinimalSize() {};

    /// Enable the creator to block the process of a file open.
    /// \param filePath the file needs to be opened
    /// \param cannotOpenFileMessage the message displayed to the user in case that the file cannot be opened
    /// \return true iff the file can currently be opened
    virtual bool CanFileBeOpened(const osFilePath& filePath, gtString& cannotOpenFileMessage) { GT_UNREFERENCED_PARAMETER(filePath); GT_UNREFERENCED_PARAMETER(cannotOpenFileMessage); return true; };

    /// Display the view containing the content specified in the event:
    virtual bool displayExistingView(const apMDIViewCreateEvent& mdiViewEvent) { (void)(mdiViewEvent); return false;};

    /// Get the docking widget:
    virtual QDockWidget* containingDockWidget(int viewIndex) { (void)(viewIndex); return nullptr; };

    /// Get the file that is currently displayed:
    virtual bool getCurrentlyDisplayedFilePath(osFilePath& filePath) { (void)(filePath); return false;}

    /// Get the specific QWidget:
    virtual QWidget* widget(int viewIndex) = 0;

    /// Utilities (override this function is the views that has icons):
    virtual void initViewsIcons();
    void initSingleViewIcon(int actionIndex, acIconId iconId);

    class afViewIconData
    {
    public:

        QPixmap* _pCommandPixmap;
        bool _isPixmapInitialized;

        afViewIconData();
    };

    /// Events:
    virtual bool onMDISubWindowClose(afQMdiSubWindow* pMDISubWindow);

    /// Action creator accessor:
    afActionCreatorAbstract* actionCreator() const {return _pViewActionCreator;}

    /// Accessor for the supported edit commands, stored in the actions creator
    const gtVector<int>& supportedCommandIds() const;

    /// Accessor for the creation event
    const apEvent* CreationEvent() const { return _pCreationEvent; }

    /// Convert the action index to the corresponding command id.
    /// The actual conversion is performed by the actions creator
    int actionIndexToCommandId(const int actionIndex) const;

protected:

    /// Contain the icon data for the views:
    gtVector<afViewIconData> _iconsDataVector;

    /// Contain the event that is used for current view creation:
    apEvent* _pCreationEvent;

    /// The creator for the actions related to me:
    afActionCreatorAbstract* _pViewActionCreator;


};

#endif // __AFVIEWCREATORABSTRACT_H
