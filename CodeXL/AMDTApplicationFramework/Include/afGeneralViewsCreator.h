//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afGeneralViewsCreator.h
///
//==================================================================================

#ifndef __AFGENERALVIEWSCREATOR_H
#define __AFGENERALVIEWSCREATOR_H

// Forward declaration:
class afPropertiesView;
class afApplicationTree;
class afInformationView;

// Infra:
#include <AMDTApplicationFramework/Include/afQtViewCreatorAbstract.h>
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:           afGeneralViewsCreator : public afQtViewCreatorAbstract
// General Description:  A class handling the creation of the framework general views
// Author:               Sigal Algranaty
// Creation Date:        6/9/2011
// ----------------------------------------------------------------------------------
class AF_API afGeneralViewsCreator : public afQtViewCreatorAbstract
{
public:

    static afGeneralViewsCreator& Instance();

    ~afGeneralViewsCreator();

    // Virtual functions that needs to be implemented:

    // Get the title of the created view:
    virtual void titleString(int viewIndex, gtString& viewTitle, gtString& viewMenuCommand);

    // Get the associated toolbar string:
    virtual gtString associatedToolbar(int viewIndex);

    // Get view type:
    virtual afViewCreatorAbstract::afViewType type(int viewIndex);

    // Get the docking area:
    virtual int dockArea(int viewIndex);

    // Get the dock view we want to dock with. "-" prefix means below that view
    virtual gtString dockWith(int viewIndex);

    // True iff the view is creating views in run time:
    virtual bool isDynamic() { return false;};

    // Docable widget features:
    virtual QDockWidget::DockWidgetFeatures dockWidgetFeatures(int viewIndex);

    // Initialize the creator:
    virtual void initialize();

    // Get the initial size:
    virtual QSize initialSize(int viewIndex);

    // Get the initial visibility of the view:
    virtual bool visibility(int viewIndex);

    // Get the initial activity of the view:
    virtual bool initiallyActive(int viewIndex);

    // Should add a separator after view command?  - false by default, override when a separator is needed:
    virtual bool addSeparator(int viewIndex);

    // Get number of views that can be created by this creator:
    virtual int amountOfViewTypes();

    // Create the inner view:
    virtual bool createViewContent(int viewIndex, QWidget*& pContentQWidget, QWidget* pQParent);

    // Handle the action when it is triggered
    virtual void handleTrigger(int viewIndex, int actionIndex);


    // handle UI update
    virtual void handleUiUpdate(int viewIndex, int actionIndex);

    // The created views:
    static afPropertiesView* propertiesView() {return m_spPropertiesView;};
    static afApplicationTree* applicationTree() {return m_spApplicationTree;};
    static afInformationView* informationView() {return m_spInformationView;};

    enum afGeneralViewsIndex
    {
        afApplicationTreeViewIndex,
        afPropertiesViewIndex,
        afInformationViewIndex,
        afAmountOfGenericViews = afInformationViewIndex + 1
    };

private:
    // Only afSingletonsDelete can delete my instance:
    friend class afSingletonsDelete;

    // Singleton instance:
    static afGeneralViewsCreator* m_spMySingleInstance;

    afGeneralViewsCreator();

    // Views:
    static afPropertiesView* m_spPropertiesView;
    static afApplicationTree* m_spApplicationTree;
    static afInformationView* m_spInformationView;

};



#endif //__AFGENERALVIEWSCREATOR_H

