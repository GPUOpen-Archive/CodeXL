//------------------------------ kaInformationViewsCreator.cpp ------------------------------

// Qt
#include <QtGui>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaInformationViewsCreator.h>
#include <AMDTKernelAnalyzer/Include/kaInformationView.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>
#include <AMDTKernelAnalyzer/Include/icons/ka_information_view_icon.xpm>

// ---------------------------------------------------------------------------
// Name:        kaInformationViewsCreator::kaInformationViewsCreator
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
kaInformationViewsCreator::kaInformationViewsCreator()
{

}

// ---------------------------------------------------------------------------
// Name:        kaInformationViewsCreator::~kaInformationViewsCreator
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
kaInformationViewsCreator::~kaInformationViewsCreator()
{
}

// Initialize the creator:

// ---------------------------------------------------------------------------
// Name:        kaInformationViewsCreator::initialize
// Description: Initialize the creator
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
void kaInformationViewsCreator::initialize()
{
    initSingleViewIcon(0, ka_information_view_icon_xpm);
}

// ---------------------------------------------------------------------------
// Name:        kaInformationViewsCreator::titleString
// Description: Get the title of the created view
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
void kaInformationViewsCreator::titleString(int viewIndex, gtString& viewTitle, gtString& viewMenuCommand)
{
    switch (viewIndex)
    {
        case 0: viewTitle = KA_STR_outputPaneCaption; break;

        default:
            GT_ASSERT(false);
            break;
    }

    viewMenuCommand = viewTitle;
}


// ---------------------------------------------------------------------------
// Name:        kaInformationViewsCreator::associatedToolbar
// Description: Get the associated toolbar string:
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
gtString kaInformationViewsCreator::associatedToolbar(int viewIndex)
{
    gtString retVal = L"";

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        kaInformationViewsCreator::type
// Description: Get view type:
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
afViewCreatorAbstract::afViewType kaInformationViewsCreator::type(int viewIndex)
{
    (void)(viewIndex); // unused
    afViewCreatorAbstract::afViewType retDockType = AF_VIEW_dock;

    return retDockType;
}


// ---------------------------------------------------------------------------
// Name:        kaInformationViewsCreator::dockArea
// Description: Get the docking area:
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
int kaInformationViewsCreator::dockArea(int viewIndex)
{
    (void)(viewIndex); // unused
    int retVal = AF_VIEW_DOCK_BottomDockWidgetArea;

    return retVal;

}


// ---------------------------------------------------------------------------
// Name:        kaInformationViewsCreator::dockWith
// Description: Get the dock view we want to dock with. "-" prefix means below that view
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
gtString kaInformationViewsCreator::dockWith(int viewIndex)
{
    (void)(viewIndex); // unused
    // taken from but getting the #include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
    // needs doing some ugly adding of include path we usually don't do so added a copy of the string:
    gtString retVal = KA_STR_outputPaneDockWithDebugProcessView;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        kaInformationViewsCreator::dockWidgetFeatures
// Description: Dockable widget features:
// Arguments:   int viewIndex
// Return Val:  QDockWidget::DockWidgetFeatures
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
QDockWidget::DockWidgetFeatures kaInformationViewsCreator::dockWidgetFeatures(int viewIndex)
{
    (void)(viewIndex); // unused
    QDockWidget::DockWidgetFeatures retVal = QDockWidget::AllDockWidgetFeatures;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaInformationViewsCreator::initialSize
// Description: Get the initial size:
// Arguments:   int viewIndex
// Return Val:  QSize
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
QSize kaInformationViewsCreator::initialSize(int viewIndex)
{
    (void)(viewIndex); // unused
    QSize retSize;
    QSize desktopSize = QApplication::desktop()->rect().size();
    int minViewWidth = 100;
    int minViewHeight = 30;
    QSize minViewSize(minViewWidth, minViewHeight);

    retSize.setWidth(desktopSize.width() * 2 / 5);
    retSize.setHeight(desktopSize.height() / 4);

    return retSize;
}


// ---------------------------------------------------------------------------
// Name:        kaInformationViewsCreator::visibility
// Description: Get the initial visibility of the view:
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
bool kaInformationViewsCreator::visibility(int viewIndex)
{
    (void)(viewIndex); // unused
    bool retVal = false;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        kaInformationViewsCreator::amountOfViewTypes
// Description: Get number of views that can be created by this creator:
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
int kaInformationViewsCreator::amountOfViewTypes()
{
    return 1;
}


// ---------------------------------------------------------------------------
// Name:        kaInformationViewsCreator::createViewContent
// Description: Create the inner view:
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
bool kaInformationViewsCreator::createViewContent(int viewIndex, QWidget*& pContentQWidget, QWidget* pQParent)
{
    (void)(viewIndex); // unused
    bool retVal = false;

    pContentQWidget = new kaInformationView(pQParent);
    GT_ASSERT_ALLOCATION(pContentQWidget);

    if (NULL != pContentQWidget)
    {
        retVal = true;
    }

    // Set the created window:
    m_viewsCreated.push_back(pContentQWidget);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        kaInformationViewsCreator::handleTrigger
// Description: Handle the action when it is triggered
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
void kaInformationViewsCreator::handleTrigger(int viewIndex, int actionIndex)
{
    (void)viewIndex; // unused;
    (void)actionIndex; // unused;
}


// ---------------------------------------------------------------------------
// Name:        kaInformationViewsCreator::handleUiUpdate
// Description: handle UI update
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
void kaInformationViewsCreator::handleUiUpdate(int viewIndex, int actionIndex)
{
    (void)viewIndex; // unused;
    (void)actionIndex; // unused;
}


// ---------------------------------------------------------------------------
// Name:        kaInformationViewsCreator::createQtWidgetWrapping
// Description: Create the QT widget for the view:
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
QWidget* kaInformationViewsCreator::createQtWidgetWrapping(int viewIndex)
{
    (void)(viewIndex); // unused
    return NULL;
}

