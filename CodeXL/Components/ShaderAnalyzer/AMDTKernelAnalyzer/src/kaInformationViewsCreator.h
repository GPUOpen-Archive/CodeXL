//------------------------------ kaInformationViewsCreator.h ------------------------------

#ifndef __KAINFORMATIONVIEWSCREATOR_H
#define __KAINFORMATIONVIEWSCREATOR_H

// Forward declaration:

// Infra:
#include <AMDTApplicationFramework/Include/afQtViewCreatorAbstract.h>

// ----------------------------------------------------------------------------------
// Class Name:          kaInformationViewsCreator : public afQtViewCreatorAbstract
// General Description: creates the dockables views for the kernel analyzer
//
// Author:              Gilad Yarnitzky
// Creation Date:       21/8/2013
// ----------------------------------------------------------------------------------
class kaInformationViewsCreator : public afQtViewCreatorAbstract
{
public:
    kaInformationViewsCreator();
    ~kaInformationViewsCreator();

    // Virtual functions that needs to be implemented:

    // Initialize the creator:
    virtual void initialize();

    // Get the title of the created view:
    virtual void titleString(int viewIndex, gtString& viewTitle, gtString& viewMenuCommand);

    // Get the associated toolbar string:
    virtual gtString associatedToolbar(int viewIndex);

    // Get view type:
    virtual afViewCreatorAbstract::afViewType type(int viewIndex);

    // Get the docking area:
    virtual int dockArea(int viewIndex);

    // True iff the view is creating views in run time:
    virtual bool isDynamic() { return false;};

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

    // Create the QT widget for the view:
    virtual QWidget* createQtWidgetWrapping(int viewIndex);

    // Create the context for the view:
    virtual bool createViewContent(int viewIndex, QWidget*& pContentQWidget, QWidget* pQParent);

    // Perform the QT wrapping:
    virtual bool createQTWrapping(int viewIndex, QMainWindow* pMainWindow) { (void)(viewIndex); (void)(pMainWindow); return true;};

    // Get the dock view we want to dock with. "-" prefix means below that view
    virtual gtString dockWith(int viewIndex);

protected:
};

#endif //__KAINFORMATIONVIEWSCREATOR_H

