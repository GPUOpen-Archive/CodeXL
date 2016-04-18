//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afMDIViewsCreator.h
///
//==================================================================================

#ifndef __AFMDIVIEWSCREATOR_H
#define __AFMDIVIEWSCREATOR_H

// Qt:
#include <QWidget>

// Infra:
#include <AMDTApplicationFramework/Include/afQtViewCreatorAbstract.h>
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

// Forward declaration:
class gdDebugApplicationTreeData;
class afApplicationCommands;

/// This views creator is used to create basic and generic file types views
/// The view supports source code views and image views
class AF_API afMDIViewsCreator : public afQtViewCreatorAbstract
{
public:

    afMDIViewsCreator();
    ~afMDIViewsCreator();

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

    // Docable widget features:
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

    virtual const gtString CreatedMDIType() const { return AF_STR_GenericMDIViewsCreatorID; }

    // Create the context for the view:
    virtual bool createViewContent(int viewIndex, QWidget*& pContentQWidget, QWidget* pQParent);

    // Display the view containing the content specified in the event:
    virtual bool displayExistingView(const apMDIViewCreateEvent& mdiViewEvent);


    // Get the file that is currently displayed:
    virtual bool getCurrentlyDisplayedFilePath(osFilePath& filePath);

    // Events:
    virtual bool onMDISubWindowClose(afQMdiSubWindow* pMDISubWindow);
protected:

    /// Create a source code view
    /// \param pQParent the Qt widget parent
    /// \param pContentQWidget[out] the created source code view
    /// \return true for success
    bool CreateSourceCodeView(QWidget* pQParent, QWidget*& pContentQWidget);

    /// Create an html view
    /// \param pQParent the Qt widget parent
    /// \param pContentQWidget[out] the created source code view
    /// \return true for success
    bool CreateHtmlView(QWidget* pQParent, QWidget*& pContentQWidget);

protected:
    afApplicationCommands* m_pApplicationCommands;

};


#endif //__AFMDIVIEWSCREATOR_H

