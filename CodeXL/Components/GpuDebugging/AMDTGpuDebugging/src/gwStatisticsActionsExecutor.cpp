//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwStatisticsActionsExecutor.cpp
///
//==================================================================================

//------------------------------ gwStatisticsActionsExecutor.cpp ------------------------------

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <QAction>

#include <AMDTApplicationComponents/Include/acChartWindow.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/src/afUtils.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdStatisticsPanel.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStatisticsView.h>

// Local:
#include <src/gwStatisticsActionsExecutor.h>

// ---------------------------------------------------------------------------
// Name:        gwStatisticsActionsExecutor::gwStatisticsActionsExecutor
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        20/9/2011
// ---------------------------------------------------------------------------
gwStatisticsActionsExecutor::gwStatisticsActionsExecutor(): _pStatisticsView(NULL), _pStatisticsPanel(NULL)
{
}

// ---------------------------------------------------------------------------
// Name:        gwStatisticsActionsExecutor::~gwStatisticsActionsExecutor
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        20/9/2011
// ---------------------------------------------------------------------------
gwStatisticsActionsExecutor::~gwStatisticsActionsExecutor()
{
}

// ---------------------------------------------------------------------------
// Name:        gwStatisticsActionsExecutor::populateSupportedCommandIds
// Description: Create a vector of command Ids that are supported by this actions creator object
// Author:      Doron Ofek
// Date:        Mar-3, 2015
// ---------------------------------------------------------------------------
void gwStatisticsActionsExecutor::populateSupportedCommandIds()
{
    // fill the vector of supported command ids:
    m_supportedCommandIds.push_back(ID_STATISTICS_VIEWER_EXPORT_TOTAL_STATISTICS);
    m_supportedCommandIds.push_back(ID_STATISTICS_VIEWER_EXPORT_FUNCTION_CALLS_STATISTICS);
    m_supportedCommandIds.push_back(ID_STATISTICS_VIEWER_EXPORT_DEPRECATION_STATISTICS);
    m_supportedCommandIds.push_back(ID_STATISTICS_VIEWER_EXPORT_BATCH_STATISTICS);
    m_supportedCommandIds.push_back(ID_STATISTICS_VIEWER_CLEAR_STATISTICS);
}

// ---------------------------------------------------------------------------
// Name:        gwStatisticsActionsExecutor::caption
// Description: Get the caption of the action item
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        14/8/2011
// ---------------------------------------------------------------------------
bool gwStatisticsActionsExecutor::actionText(int actionIndex, gtString& caption, gtString& tooltip, gtString& keyboardShortcut)
{
    (void)(tooltip); // unused
    (void)(keyboardShortcut); // unused
    bool retVal = true;

    // Get the command id:
    // Get the command id:
    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {

        case ID_STATISTICS_VIEWER_EXPORT_TOTAL_STATISTICS:
            caption = GD_STR_StatisticsViewerFileExportTotalStatistics;
            break;

        case ID_STATISTICS_VIEWER_EXPORT_FUNCTION_CALLS_STATISTICS:
            caption = GD_STR_StatisticsViewerFileExportFunctionCallStatistics;
            break;

        case ID_STATISTICS_VIEWER_EXPORT_DEPRECATION_STATISTICS:
            caption = GD_STR_StatisticsViewerFileExportDeprecationStatistics;
            break;

        case ID_STATISTICS_VIEWER_EXPORT_BATCH_STATISTICS:
            caption = GD_STR_StatisticsViewerFileExportBatchStatistics;
            break;

        case ID_STATISTICS_VIEWER_CLEAR_STATISTICS:
            caption = GD_STR_StatisticsViewerFileClearStatistics;
            break;

        default:
            GT_ASSERT(false);
            retVal = false;
            break;

    };

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwStatisticsActionsExecutor::menuPosition
// Description: Menu position
//              Each hierarchy on the ,emu include name/priority.
//              If separator is needed after the item then 's' after the priority is needed
//              in case of a sub menu if one item is marked with an 's' it is enough to mark a separator after it:
// Arguments:   int actionIndex
//              positionData - defines the item position within its parent menu
// Author:      Sigal Algranaty
// Date:        20/9/2011
// ---------------------------------------------------------------------------
gtString gwStatisticsActionsExecutor::menuPosition(int actionIndex, afActionPositionData& positionData)
{
    gtString retVal;

    positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_NONE;

    // Get the command id:
    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {
        case ID_STATISTICS_VIEWER_EXPORT_FUNCTION_CALLS_STATISTICS:
        case ID_STATISTICS_VIEWER_EXPORT_DEPRECATION_STATISTICS:
        case ID_STATISTICS_VIEWER_EXPORT_BATCH_STATISTICS:
        case ID_STATISTICS_VIEWER_CLEAR_STATISTICS:
        {
            retVal = AF_STR_FileMenuString;
            retVal.append(AF_Str_MenuSeparator);
            retVal.append(GD_STR_exportFileMenu);
        }
        break;

        case ID_STATISTICS_VIEWER_EXPORT_TOTAL_STATISTICS:
        {
            retVal = AF_STR_FileMenuString;
            retVal.append(AF_Str_MenuSeparator);
            retVal.append(GD_STR_exportFileMenu);
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
        }
        break;

        default:
            GT_ASSERT(false);
            break;
    };

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwStatisticsActionsExecutor::toolbarPosition
// Description: Toolbar position
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        20/9/2011
// ---------------------------------------------------------------------------
gtString gwStatisticsActionsExecutor::toolbarPosition(int actionIndex)
{
    (void)(actionIndex); // unused
    gtString retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwStatisticsActionsExecutor::handleTrigger
// Description: Handle the action when it is triggered
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        20/9/2011
// ---------------------------------------------------------------------------
void gwStatisticsActionsExecutor::handleTrigger(int actionIndex)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pStatisticsView != NULL)
    {
        // Get the command id:
        int commandId = actionIndexToCommandId(actionIndex);

        switch (commandId)
        {
            case ID_STATISTICS_VIEWER_EXPORT_TOTAL_STATISTICS:
            {
                _pStatisticsView->exportStatistics(GD_STATISTICS_VIEW_TOTAL_INDEX);
            }
            break;

            case ID_STATISTICS_VIEWER_EXPORT_FUNCTION_CALLS_STATISTICS:
            {
                _pStatisticsView->exportStatistics(GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX);
            }
            break;

            case ID_STATISTICS_VIEWER_EXPORT_DEPRECATION_STATISTICS:
            {
                _pStatisticsView->exportStatistics(GD_STATISTICS_VIEW_DEPRECATION_INDEX);
            }
            break;

            case ID_STATISTICS_VIEWER_EXPORT_BATCH_STATISTICS:
            {
                _pStatisticsView->exportStatistics(GD_STATISTICS_VIEW_BATCH_INDEX);
            }
            break;

            case ID_STATISTICS_VIEWER_CLEAR_STATISTICS:
            {
                _pStatisticsPanel->onClearButton();
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

// ---------------------------------------------------------------------------
// Name:        gwStatisticsActionsExecutor::handleUiUpdate
// Description: Handle UI update
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        20/9/2011
// ---------------------------------------------------------------------------
void gwStatisticsActionsExecutor::handleUiUpdate(int actionIndex)
{
    bool isActionEnabled = false, isActionChecked = false, isActionCheckable = false;

    if (_pStatisticsView == NULL)
    {
        // Get the application commands handler:
        gdApplicationCommands* pApplicationCommandsHandler = gdApplicationCommands::gdInstance();
        GT_IF_WITH_ASSERT(pApplicationCommandsHandler != NULL)
        {
            _pStatisticsPanel = pApplicationCommandsHandler->statisticsPanel();
            GT_IF_WITH_ASSERT(_pStatisticsPanel != NULL)
            {
                _pStatisticsView = _pStatisticsPanel->statisticsView();
                GT_ASSERT(_pStatisticsView != NULL);
            }
        }
    }

    // Sanity check:
    GT_IF_WITH_ASSERT((_pStatisticsView != NULL) && (_pStatisticsPanel != NULL))
    {
        // Get the command id:
        int commandId = actionIndexToCommandId(actionIndex);

        bool isStatisticsCommand = ((commandId == ID_STATISTICS_VIEWER_EXPORT_TOTAL_STATISTICS) ||
                                    (commandId == ID_STATISTICS_VIEWER_EXPORT_FUNCTION_CALLS_STATISTICS) ||
                                    (commandId == ID_STATISTICS_VIEWER_EXPORT_DEPRECATION_STATISTICS) ||
                                    (commandId == ID_STATISTICS_VIEWER_EXPORT_BATCH_STATISTICS) ||
                                    (commandId == ID_STATISTICS_VIEWER_CLEAR_STATISTICS));

        if (isStatisticsCommand)
        {
            switch (commandId)
            {

                case ID_STATISTICS_VIEWER_EXPORT_TOTAL_STATISTICS:
                    isActionEnabled = (_pStatisticsView->pageItemCount(GD_STATISTICS_VIEW_TOTAL_INDEX) > 1);
                    break;

                case ID_STATISTICS_VIEWER_EXPORT_FUNCTION_CALLS_STATISTICS:
                    isActionEnabled = (_pStatisticsView->pageItemCount(GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX) > 1);
                    break;

                case ID_STATISTICS_VIEWER_EXPORT_DEPRECATION_STATISTICS:
                    isActionEnabled = (_pStatisticsView->pageItemCount(GD_STATISTICS_VIEW_DEPRECATION_INDEX) > 1);
                    break;

                case ID_STATISTICS_VIEWER_EXPORT_BATCH_STATISTICS:
                    isActionEnabled = (_pStatisticsView->pageItemCount(GD_STATISTICS_VIEW_BATCH_INDEX) > 1);
                    break;

                case ID_STATISTICS_VIEWER_CLEAR_STATISTICS:
                    _pStatisticsPanel->onUpdateClearButton(isActionEnabled);
                    break;

                default:
                    break;

            }
        }
    }

    // Get the QT action:
    QAction* pAction = action(actionIndex);
    GT_IF_WITH_ASSERT(pAction != NULL)
    {
        // Set the action enable / disable:
        pAction->setEnabled(isActionEnabled);

        // Set the action checkable state:
        pAction->setCheckable(isActionCheckable);

        // Set the action check state:
        pAction->setChecked(isActionChecked);

        gtString caption, tooltip, keyboardShortcut;
        bool rc = actionText(actionIndex, caption, tooltip, keyboardShortcut);

        if (rc)
        {
            // Set the action text (recently used project names are changed):
            pAction->setText(acGTStringToQString(caption));
            pAction->setToolTip(acGTStringToQString(tooltip));

            QList<QKeySequence> shortcuts;
            QString qShortcut = QString::fromWCharArray(keyboardShortcut.asCharArray());

            // Create a key shortcut from the string:
            QKeySequence keySeauence(acGTStringToQString(keyboardShortcut));
            shortcuts.append(keySeauence);

            // Set the shortcuts:
            pAction->setShortcuts(shortcuts);

            // Set context to application context:
            pAction->setShortcutContext(Qt::ApplicationShortcut);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gwStatisticsActionsExecutor::groupAction
// Description: If the requested action should be a part of a group, create
//              the action group and add the relevant actions to the group
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        20/9/2011
// ---------------------------------------------------------------------------
void gwStatisticsActionsExecutor::groupAction(int actionIndex)
{
    (void)(actionIndex); // unused

}
