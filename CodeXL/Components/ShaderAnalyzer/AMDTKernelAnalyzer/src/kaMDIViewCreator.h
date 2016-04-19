//------------------------------ kaMDIViewCreator.h ------------------------------

#ifndef __KAMDIVIEWCREATOR_H
#define __KAMDIVIEWCREATOR_H

// Qt:
#include <QWidget>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>

// Framework:
#include <AMDTApplicationFramework/Include/afQtViewCreatorAbstract.h>
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

// Forward declaration:
class gdDebugApplicationTreeData;

class kaMDIViewCreator : public afQtViewCreatorAbstract
{
public:

    kaMDIViewCreator();
    ~kaMDIViewCreator();

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
    virtual const gtString CreatedMDIType() const { return AF_STR_KernelAnalyzerViewsCreatorID; };

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

private:
    bool getInformationPaths(apMDIViewCreateEvent* pTreeItem, osFilePath& kernelPath, osFilePath& identifyPath, osFilePath& detailedPath);

    void toggleAllMultiSourceViewsLineNumbers();

    /// Validate that the overview file exists. If not create it again.
    void ValidateOverviewFileExists(const osFilePath& filePath);

    // map that holds the views that were created so far in order to prevent creating same views again:
    gtMap<gtString, QWidget*> m_createdViewsMap;
};


#endif //__KAMDIVIEWCREATOR_H

