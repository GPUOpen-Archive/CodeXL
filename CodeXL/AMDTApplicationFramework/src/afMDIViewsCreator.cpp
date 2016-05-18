//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afMDIViewsCreator.cpp
///
//==================================================================================

//Qt:
#include <QtWidgets>

// Infra:
#include <AMDTApplicationComponents/Include/acImageView.h>

// qscintilla related stuff need to be first:
#include <AMDTApplicationFramework/Include/views/afSourceCodeView.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afActionCreatorAbstract.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afMDIViewsCreator.h>
#include <AMDTApplicationFramework/Include/afSourceCodeActionsCreator.h>
#include <AMDTApplicationFramework/Include/afSourceCodeViewsManager.h>
#include <AMDTApplicationFramework/Include/afQMdiSubWindow.h>
#include <AMDTApplicationFramework/Include/afHTMLView.h>


// ---------------------------------------------------------------------------
// Name:        afMDIViewsCreator::afMDIViewsCreator
// Description: Creator
// Author:      Sigal Algranaty
// Date:        27/7/2011
// ---------------------------------------------------------------------------
afMDIViewsCreator::afMDIViewsCreator()
{
    // Create the view actions creator:
    _pViewActionCreator = new afSourceCodeActionsCreator;

    _pViewActionCreator->initializeCreator();

    m_pApplicationCommands = afApplicationCommands::instance();
    GT_ASSERT(m_pApplicationCommands != nullptr);

}

// ---------------------------------------------------------------------------
// Name:        afMDIViewsCreator::~afMDIViewsCreator
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
afMDIViewsCreator::~afMDIViewsCreator()
{
}

// ---------------------------------------------------------------------------
// Name:        afMDIViewsCreator::titleString
// Description: Get the title of the created view
// Arguments:   int viewIndex
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
void afMDIViewsCreator::titleString(int viewIndex, gtString& viewTitle, gtString& viewMenuCommand)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    GT_IF_WITH_ASSERT((_pCreationEvent != nullptr) && (_pCreationEvent->eventType() == apEvent::AP_MDI_CREATED_EVENT))
    {
        apMDIViewCreateEvent* pMDIEvent = (apMDIViewCreateEvent*)_pCreationEvent;
        GT_IF_WITH_ASSERT(pMDIEvent != nullptr)
        {
            // Source code creation event:
            if (pMDIEvent->CreatedMDIType() == AF_STR_GenericMDIViewsCreatorID)
            {
                // Check if the title is set on the event:
                viewTitle = pMDIEvent->viewTitle();

                if (viewTitle.isEmpty())
                {
                    // Extract the title from the file path:
                    pMDIEvent->filePath().getFileNameAndExtension(viewTitle);

                    if (viewTitle.compare(L"NoSource.html") == 0)
                    {
                        viewTitle = AF_STR_HtmlFindSourceWebpageTitle;
                    }
                }

                viewMenuCommand = viewTitle;
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        afMDIViewsCreator::associatedToolbar
// Description: Get the name of the toolbar associated with the requested view
// Arguments:   int viewIndex
// Return Val:  gtString
// Author:      Sigal Algranaty
// Date:        28/7/2011
// ---------------------------------------------------------------------------
gtString afMDIViewsCreator::associatedToolbar(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    gtString retVal;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afMDIViewsCreator::type
// Description: Get view type
// Arguments:   int viewIndex
// Return Val:  afViewType
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
afMDIViewsCreator::afViewType afMDIViewsCreator::type(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    afViewCreatorAbstract::afViewType retDockArea = AF_VIEW_mdi;

    return retDockArea;
}

// ---------------------------------------------------------------------------
// Name:        afMDIViewsCreator::dockArea
// Description: Get the docking area
// Arguments:   int viewIndex
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
int afMDIViewsCreator::dockArea(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    int retVal = AF_VIEW_DOCK_LeftDockWidgetArea;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afMDIViewsCreator::dockWidgetFeatures
// Description: Irrelevant for MDI views
// Return Val:  DockWidgetFeatures
// Author:      Sigal Algranaty
// Date:        1/8/2011
// ---------------------------------------------------------------------------
QDockWidget::DockWidgetFeatures afMDIViewsCreator::dockWidgetFeatures(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    return QDockWidget::NoDockWidgetFeatures;
}

// ---------------------------------------------------------------------------
// Name:        afMDIViewsCreator::initialSize
// Description: Get the initial size
// Arguments:   int viewIndex
// Return Val:  QSize - size of the view
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
QSize afMDIViewsCreator::initialSize(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    QSize retSize(0, 0);

    return retSize;
}

// ---------------------------------------------------------------------------
// Name:        afMDIViewsCreator::visibility
// Description: Get the initial visibility of the view
// Arguments:   int viewIndex
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
bool afMDIViewsCreator::visibility(int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    bool retVal = true;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afMDIViewsCreator::amountOfViewTypes
// Description: Get number of views that can be created by this creator
// Return Val:  int
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
int afMDIViewsCreator::amountOfViewTypes()
{
    return 1;
}

// ---------------------------------------------------------------------------
// Name:        afMDIViewsCreator::createViewContent
// Description: Create the content for the view
// Arguments:   int viewIndex
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/8/2011
// ---------------------------------------------------------------------------
bool afMDIViewsCreator::createViewContent(int viewIndex, QWidget*& pContentQWidget, QWidget* pQParent)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    bool retVal = false;

    pContentQWidget = nullptr;

    // Set the default minimum view height and width
    int minViewWidth = 100;
    int minViewHeight = 30;
    QSize minViewSize(minViewWidth, minViewHeight);

    GT_IF_WITH_ASSERT((_pCreationEvent != nullptr) && (_pCreationEvent->eventType() == apEvent::AP_MDI_CREATED_EVENT))
    {
        apMDIViewCreateEvent* pMDIEvent = (apMDIViewCreateEvent*)_pCreationEvent;
        GT_IF_WITH_ASSERT(pMDIEvent != nullptr)
        {
            gtString fileExtension;
            pMDIEvent->filePath().getFileExtension(fileExtension);

            if (AF_STR_pngFileExtension == fileExtension)
            {
                pContentQWidget = new acImageView(pQParent, pMDIEvent->filePath());
                retVal = true;
            }
            else if (AF_STR_htmlFileExtension == fileExtension)
            {
                retVal = CreateHtmlView(pQParent, pContentQWidget);
            }
            else
            {
                retVal = CreateSourceCodeView(pQParent, pContentQWidget);
            }
        }

        // Set the created window:
        m_viewsCreated.push_back(pContentQWidget);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afMDIViewsCreator::getCurrentlyDisplayedFilePath
// Description: Get the currently displayed file path
// Arguments:   osFilePath& filePath
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/8/2011
// ---------------------------------------------------------------------------
bool afMDIViewsCreator::getCurrentlyDisplayedFilePath(osFilePath& filePath)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pCreationEvent != nullptr)
    {
        // Source code creation event:
        if (_pCreationEvent->eventType() == apEvent::AP_MDI_CREATED_EVENT)
        {
            // Down cast the event:
            apMDIViewCreateEvent* pSourceCodeEvent = (apMDIViewCreateEvent*)_pCreationEvent;
            GT_IF_WITH_ASSERT(pSourceCodeEvent != nullptr)
            {
                // Get the file path from the event:
                filePath = pSourceCodeEvent->filePath();
                retVal = true;
            }
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afMDIViewsCreator::displayExistingView
// Description:
// Arguments:   const apMDIViewCreateEvent& mdiViewEvent
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/8/2011
// ---------------------------------------------------------------------------
bool afMDIViewsCreator::displayExistingView(const apMDIViewCreateEvent& mdiViewEvent)
{
    bool retVal = false;

    // Get the file extension
    gtString ext;
    mdiViewEvent.filePath().getFileExtension(ext);

    // Handle image and source views separately
    if (ext == AF_STR_pngFileExtension)
    {
        acImageView* pExistingView = nullptr;

        if (afMainAppWindow::instance() != nullptr)
        {
            afQMdiSubWindow* pImageMdiWindow = afMainAppWindow::instance()->findMDISubWindow(mdiViewEvent.filePath());

            if (pImageMdiWindow != nullptr)
            {
                // Get the image view
                pExistingView = pImageMdiWindow->findChild<acImageView*>();

                if (pExistingView != nullptr)
                {
                    pExistingView->show();
                    retVal = true;
                }
            }
        }
    }
    else if (ext == AF_STR_htmlFileExtension)
    {
        afHTMLView* pExistingView = nullptr;

        if (afMainAppWindow::instance() != nullptr)
        {
            afQMdiSubWindow* pImageMdiWindow = afMainAppWindow::instance()->findMDISubWindow(mdiViewEvent.filePath());

            if (pImageMdiWindow != nullptr)
            {
                // Get the image view
                pExistingView = pImageMdiWindow->findChild<afHTMLView*>();

                if (pExistingView != nullptr)
                {
                    pExistingView->ReloadView();
                    pExistingView->show();
                    retVal = true;
                }
            }
        }
    }
    else
    {
        // Source code view
        int viewIndex = 0;
        afSourceCodeViewsManager& theSourceCodeViewsManager = afSourceCodeViewsManager::instance();
        afSourceCodeView* pSourceCodeView = theSourceCodeViewsManager.getExistingView(mdiViewEvent.filePath(), viewIndex);

        if (pSourceCodeView != nullptr)
        {
            GT_IF_WITH_ASSERT(mdiViewEvent.eventType() == apEvent::AP_MDI_CREATED_EVENT)
            {
                // Display the file:
                pSourceCodeView->displayFile(mdiViewEvent.filePath(), mdiViewEvent.lineNumber(), mdiViewEvent.programCounterIndex());
                pSourceCodeView->SetCursorPositionToMiddle(mdiViewEvent.lineNumber(), mdiViewEvent.programCounterIndex());

                // Set the program counter index and the line number for the requested source code file:
                theSourceCodeViewsManager.setLineNumberAndProgramCounter(mdiViewEvent.filePath(), mdiViewEvent.lineNumber(), mdiViewEvent.programCounterIndex());

                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afMDIViewsCreator::onMDISubWindowClose
// Description: Handle sub window close
// Arguments:   afQMdiSubWindow* pMDISubWindow
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/8/2011
// ---------------------------------------------------------------------------
bool afMDIViewsCreator::onMDISubWindowClose(afQMdiSubWindow* pMDISubWindow)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pMDISubWindow != nullptr)
    {
        // Get the sub window widget:
        QWidget* pWidget = pMDISubWindow->widget();
        GT_IF_WITH_ASSERT(pWidget != nullptr)
        {
            // Check if the widget is a source code view:
            gtASCIIString className(pWidget->metaObject()->className());

            if (className == "afSourceCodeView" || className == "afHTMLView")
            {
                // Down cast the widget to a source code view:
                // Remove the source code view from the vector of my widgets:
                int existingViewIndex = -1;

                for (int i = 0; i < (int)m_viewsCreated.size(); i++)
                {
                    if (pWidget == m_viewsCreated[i])
                    {
                        existingViewIndex = i;
                        break;
                    }
                }

                // Remove the view:
                GT_IF_WITH_ASSERT(existingViewIndex >= 0)
                {
                    m_viewsCreated.removeItem(existingViewIndex);
                }

                afSourceCodeView* pSourceCodeView = qobject_cast<afSourceCodeView*>(pWidget);

                if (pSourceCodeView != nullptr)
                {
                    // Remove the view from the manager:
                    afSourceCodeViewsManager::instance().removeSourceCodeView(pSourceCodeView);
                }
            }
        }
    }
    return retVal;
}


bool afMDIViewsCreator::CreateSourceCodeView(QWidget* pQParent, QWidget*& pContentQWidget)
{
    bool retVal = false;

    apMDIViewCreateEvent* pMDIEvent = (apMDIViewCreateEvent*)_pCreationEvent;
    GT_IF_WITH_ASSERT(pMDIEvent != nullptr)
    {
        // Get the requested source code view from the manager:
        afSourceCodeView* pSourceCodeView = afSourceCodeViewsManager::instance().getSourceCodeWindow(pMDIEvent->filePath(), pMDIEvent->lineNumber(), pMDIEvent->programCounterIndex(), pQParent);
        pSourceCodeView->SetCursorPositionToMiddle(pMDIEvent->lineNumber(), pMDIEvent->programCounterIndex());

        GT_IF_WITH_ASSERT(pSourceCodeView != nullptr)
        {
            // Set the file icon:
            const osFilePath& sourceCodeViewPath = pSourceCodeView->filePath();

            if (sourceCodeViewPath.isWritable())
            {
                gtString sourceFileExtension;
                sourceCodeViewPath.getFileExtension(sourceFileExtension);

                if ((sourceFileExtension == L"cpp") || (sourceFileExtension == L"cxx"))
                {
                    // C++ source file:
                    initSingleViewIcon(0, AC_ICON_SOURCE_CPP);
                }
                else if (sourceFileExtension == L"c")
                {
                    // C source file:
                    initSingleViewIcon(0, AC_ICON_SOURCE_C);
                }
                else if ((sourceFileExtension == L"h") || (sourceFileExtension == L"hpp") || (sourceFileExtension == L"hxx"))
                {
                    // Header file:
                    initSingleViewIcon(0, AC_ICON_SOURCE_H);
                }
                else if (sourceFileExtension == L"cl")
                {
                    // CL source file:
                    initSingleViewIcon(0, AC_ICON_SOURCE_CL);
                }
                else if (sourceFileExtension == L"glsl")
                {
                    // GLSL shader source file:
                    initSingleViewIcon(0, AC_ICON_SOURCE_GLSL);
                }
                else
                {
                    // Unknown file type:
                    initSingleViewIcon(0, AC_ICON_SOURCE_GENERIC);
                }

            }
            else // !pSourceCodeView->filePath().isWritable()
            {
                initSingleViewIcon(0, AC_ICON_SOURCE_READONLY);
            }

            pContentQWidget = pSourceCodeView;
            retVal = true;
        }
    }
    return retVal;
}

bool afMDIViewsCreator::CreateHtmlView(QWidget* pQParent, QWidget*& pContentQWidget)
{
    GT_UNREFERENCED_PARAMETER(pQParent);

    bool retVal = false;

    apMDIViewCreateEvent* pMDIEvent = (apMDIViewCreateEvent*)_pCreationEvent;
    GT_IF_WITH_ASSERT(pMDIEvent != nullptr)
    {
        // check if the view already exists and if it does return the previously created view
        int numCreatedView = m_viewsCreated.size();

        for (int nView = 0; nView < numCreatedView; nView++)
        {
            afHTMLView* pCurrentView = dynamic_cast<afHTMLView*>(m_viewsCreated[nView]);

            if (pCurrentView != nullptr)
            {
                if (pCurrentView->FilePath() == pMDIEvent->filePath())
                {
                    pContentQWidget = m_viewsCreated[nView];
                    pCurrentView->ReloadView();
                    retVal = true;
                    break;
                }
            }
        }

        if (nullptr == pContentQWidget)
        {
            pContentQWidget = new afHTMLView(pMDIEvent->filePath());
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afMDIViewsCreator::handleTrigger
// Description: Handle the action when it is triggered
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        1/9/2011
// ---------------------------------------------------------------------------
void afMDIViewsCreator::handleTrigger(int viewIndex, int actionIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    // Get the current active sub window:
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pMainWindow != nullptr)
    {
        // Get the current sub window:
        afQMdiSubWindow* pSubWindow = pMainWindow->activeMDISubWindow();

        // Get the current source code view:
        afSourceCodeView* pSourceCodeView = qobject_cast<afSourceCodeView*>(pSubWindow->widget());

        // We should not get here if the active view is not a afSourceCodeView:
        GT_IF_WITH_ASSERT(pSourceCodeView != nullptr)
        {
            // Handle the action by its id:
            int commandId = actionIndexToCommandId(actionIndex);

            switch (commandId)
            {
                case ID_CUT:
                    pSourceCodeView->cut();
                    break;

                case ID_COPY:
                    pSourceCodeView->copy();
                    break;

                case ID_PASTE:
                    pSourceCodeView->paste();
                    break;

                case ID_FIND:
                {
                    // Open the find widget (and respond to single characters click)
                    pMainWindow->OnFind(true);
                    break;
                }

                case ID_FIND_NEXT:
                case ID_FIND_PREV:
                    // When opening the find dialog, the find next & prev actions is connected, so we're connected to the
                    // relevant slot:
                    break;

                case ID_SELECT_ALL:
                    pSourceCodeView->selectAll();
                    break;

                case ID_SHOW_LINE_NUMBERS:
                {
                    // Check if white spaces are currently visible:
                    afSourceCodeViewsManager& theSourceCodeViewsManager = afSourceCodeViewsManager::instance();
                    bool isWSVisible = theSourceCodeViewsManager.showLineNumbers();

                    // Set all the open source code windows white spaces view flag:
                    theSourceCodeViewsManager.setViewLineNumbers(!isWSVisible);
                }
                break;

                case AF_ID_SAVE_FILE:
                    m_pApplicationCommands->onFileSaveFile();
                    break;

                case AF_ID_SAVE_FILE_AS:
                    m_pApplicationCommands->onFileSaveFileAs();
                    break;

                case ID_GO_TO:
                {
                    pSourceCodeView->OnGoToLine();
                }
                break;

                default:
                {
                    GT_ASSERT_EX(false, L"Unsupported application command");
                    break;
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afMDIViewsCreator::handleUiUpdate
// Description: Handle UI update
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        1/9/2011
// ---------------------------------------------------------------------------
void afMDIViewsCreator::handleUiUpdate(int viewIndex, int actionIndex)
{
    GT_UNREFERENCED_PARAMETER(viewIndex);

    bool isActionEnabled = false, isActionChecked = false, isActionCheckable = false;

    // Get the current active sub window:
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pMainWindow != nullptr)
    {
        // Get the current sub window:
        afQMdiSubWindow* pSubWindow = pMainWindow->activeMDISubWindow();

        // Get the current source code view:
        afSourceCodeView* pSourceCodeView = qobject_cast<afSourceCodeView*>(pSubWindow->widget());

        // We should not get here if the active view is not a afSourceCodeView:
        if (pSourceCodeView != nullptr)
        {
            int commandId = actionIndexToCommandId(actionIndex);

            switch (commandId)
            {

                case ID_COPY:
                case ID_CUT:
                    isActionEnabled = !pSourceCodeView->selectedText().isEmpty();
                    break;

                case ID_PASTE:
                case ID_FIND_NEXT:
                case ID_FIND_PREV:
                case ID_FIND:
                case ID_SELECT_ALL:
                case ID_GO_TO:
                    isActionEnabled = true;
                    break;

                case ID_SHOW_LINE_NUMBERS:
                    isActionEnabled = true;
                    isActionCheckable = true;
                    isActionChecked = afSourceCodeViewsManager::instance().showLineNumbers();
                    break;

                case AF_ID_SAVE_FILE:
                case AF_ID_SAVE_FILE_AS:
                    m_pApplicationCommands->onUpdateFileSave(isActionEnabled);
                    break;

                default:
                    GT_ASSERT_EX(false, L"Unknown event id");
                    break;
            }
        }
    }

    // Sanity check:
    GT_IF_WITH_ASSERT(_pViewActionCreator)
    {
        // Get the QT action:
        QAction* pAction = _pViewActionCreator->action(actionIndex);
        GT_IF_WITH_ASSERT(pAction != nullptr)
        {
            // Set the action enable / disable:
            pAction->setEnabled(isActionEnabled);

            // Set the action checkable state:
            pAction->setCheckable(isActionCheckable);

            // Set the action check state:
            pAction->setChecked(isActionChecked);
        }
    }
}
