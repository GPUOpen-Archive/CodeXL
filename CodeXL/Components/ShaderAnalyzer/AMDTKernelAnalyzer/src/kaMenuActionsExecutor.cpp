//------------------------------ kaMenuActionsExecutor.h ------------------------------

// QT:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apExecutionModeChangedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaApplicationTreeHandler.h>
#include <AMDTKernelAnalyzer/src/kaBackendManager.h>
#include <AMDTKernelAnalyzer/src/kaMenuActionsExecutor.h>
#include <AMDTKernelAnalyzer/Include/kaCommandIDs.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>
#include <AMDTKernelAnalyzer/src/kaTreeDataExtension.h>

// ---------------------------------------------------------------------------
// Name:        kaMenuActionsExecutor::kaMenuActionsExecutor
// Description: Creator
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
kaMenuActionsExecutor::kaMenuActionsExecutor(void) : m_pApplicationTree(NULL)
{
}

// ---------------------------------------------------------------------------
// Name:        kaMenuActionsExecutor::~kaMenuActionsExecutor
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
kaMenuActionsExecutor::~kaMenuActionsExecutor(void)
{
}

// ---------------------------------------------------------------------------
// Name:        kaMenuActionsExecutor::populateSupportedCommandIds
// Description: Create a vector of command Ids that are supported by this actions creator object
// Author:      Doron Ofek
// Date:        Mar-3, 2015
// ---------------------------------------------------------------------------
void kaMenuActionsExecutor::populateSupportedCommandIds()
{
    // fill the vector of supported command ids:
    m_supportedCommandIds.push_back(ID_KA_MODE);
    m_supportedCommandIds.push_back(ID_KA_OPENCL_BUILD);
    m_supportedCommandIds.push_back(ID_KA_CANCEL_BUILD);
    m_supportedCommandIds.push_back(ID_KA_NEW_FILE);
    m_supportedCommandIds.push_back(ID_KA_NEW_FILE_ANALYZE_MENU);
    m_supportedCommandIds.push_back(ID_KA_ADD_FILE);
    m_supportedCommandIds.push_back(ID_KA_ADD_FILE_ANALYZE_MENU);
    m_supportedCommandIds.push_back(ID_KA_ANALYZE_SETTING);
}

// ---------------------------------------------------------------------------
// Name:        kaMenuActionsExecutor::initActionIcons
// Description: Create the icons for the application commands
// Author:      Sigal Algranaty
// Date:        26/7/2011
// ---------------------------------------------------------------------------
void kaMenuActionsExecutor::initActionIcons()
{
    // Initialize commands icons
    initSingleActionIcon(ID_KA_OPENCL_BUILD, AC_ICON_EXECUTION_BUILD);
    initSingleActionIcon(ID_KA_CANCEL_BUILD, AC_ICON_EXECUTION_CANCEL_BUILD);
}


// ---------------------------------------------------------------------------
// Name:        kaMenuActionsExecutor::caption
// Description: Get the caption of the action item
// Arguments:   int actionIndex
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
bool kaMenuActionsExecutor::actionText(int actionIndex, gtString& caption, gtString& tooltip, gtString& keyboardShortcut)
{
    (void)keyboardShortcut;
    bool retVal = true;

    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {

        case ID_KA_MODE:
            caption = KA_STR_executionModeMenu;
            tooltip = KA_STR_executionModeStatusbarString;
            break;

        case ID_KA_OPENCL_BUILD:
            caption = KA_STR_openclBuild;
            tooltip = KA_STR_openclBuildStatusbarString;
            //Shortcut taken care of by afExectionModeManager
            break;

        case ID_KA_CANCEL_BUILD:
            caption = KA_STR_CancelBuild;
            tooltip = KA_STR_CancelBuildStatusbarString;
            break;


        case ID_KA_ADD_FILE:
        case ID_KA_ADD_FILE_ANALYZE_MENU:
            caption = KA_STR_addFile;
            tooltip = KA_STR_addFileStatusbarString;
            break;

        case ID_KA_NEW_FILE:
        case ID_KA_NEW_FILE_ANALYZE_MENU:
            caption = KA_STR_newFile;
            tooltip = KA_STR_newFileStatusbarString;
            break;

        case ID_KA_ANALYZE_SETTING:
            caption = KA_STR_analyzeSettingsMenu;
            tooltip = KA_STR_analyzeSettingsMenuStatusbarString;
            break;

        default:
            GT_ASSERT(false);
            retVal = false;
            break;

    };

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaMenuActionsExecutor::menuPosition
// Description: Menu position
//              Each hierarchy on the ,emu include name/priority.
//              If separator is needed after the item then 's' after the priority is needed
//              in case of a sub menu if one item is marked with an 's' it is enough to mark a separator after it:
// Arguments:   int actionIndex
//              positionData - defines the item position within its parent menu
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
gtString kaMenuActionsExecutor::menuPosition(int actionIndex, afActionPositionData& positionData)
{
    gtString retVal;

    positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_NONE;
    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {

        case ID_KA_OPENCL_BUILD:
        case ID_KA_CANCEL_BUILD:
        case ID_KA_ADD_FILE_ANALYZE_MENU:
        {
            retVal = AF_STR_AnalyzeMenuString;
        }
        break;

        case ID_KA_ANALYZE_SETTING:
        case ID_KA_MODE:
        case ID_KA_NEW_FILE_ANALYZE_MENU:
        {
            retVal = AF_STR_AnalyzeMenuString;
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
        }
        break;

        case ID_KA_NEW_FILE:
        {
            retVal = AF_STR_FileMenuString;
            positionData.m_beforeActionMenuPosition = AF_STR_FileMenuString;
            positionData.m_beforeActionText = AF_STR_openFile;
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
        }
        break;

        case ID_KA_ADD_FILE:
        {
            retVal = AF_STR_FileMenuString;
            positionData.m_beforeActionMenuPosition = AF_STR_FileMenuString;
            positionData.m_beforeActionText = AF_STR_openFile;
        }
        break;

        default:
            GT_ASSERT(false);
            break;
    };

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaMenuActionsExecutor::toolbarPosition
// Description: Toolbar position
// Arguments:   int actionIndex
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
gtString kaMenuActionsExecutor::toolbarPosition(int actionIndex)
{
    (void)actionIndex;
    gtString retVal;

    /*    switch (actionIndex)
        {
        default:
            break;
        };
    */
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaMenuActionsExecutor::handleTrigger
// Description: Handle the action when it is triggered
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void kaMenuActionsExecutor::handleTrigger(int actionIndex)
{
    if (NULL == m_pApplicationTree)
    {
        setApplicationTree();
    }

    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {
        case ID_KA_OPENCL_BUILD:
        {
            gtVector<osFilePath> filesForBuild;
            GT_IF_WITH_ASSERT(kaApplicationCommands::instance().activeCLFiles(filesForBuild) > 0)
            {
                kaApplicationCommands::instance().buildCommand(filesForBuild);
            }
        }
        break;

        case ID_KA_CANCEL_BUILD:
        {
            if (kaBackendManager::instance().isInBuild())
            {
                kaApplicationCommands::instance().cancelBuildCommand();
            }
        }
        break;

        case ID_KA_MODE:
        {
            apExecutionModeChangedEvent executionModeEvent(KA_STR_executionMode, 0);
            apEventsHandler::instance().registerPendingDebugEvent(executionModeEvent);
        }
        break;

        case ID_KA_ADD_FILE:
        case ID_KA_ADD_FILE_ANALYZE_MENU:
        {
            kaApplicationTreeHandler*  pTreeHandler = kaApplicationTreeHandler::instance();
            GT_IF_WITH_ASSERT(pTreeHandler != nullptr)
            {
                pTreeHandler->OnAddFile(AF_TREE_ITEM_ITEM_NONE);
            }
        }
        break;

        case ID_KA_NEW_FILE:
        case ID_KA_NEW_FILE_ANALYZE_MENU:
        {
            // Create new file (do not force project creation):
            kaApplicationTreeHandler*  pTreeHandler =  kaApplicationTreeHandler::instance();
            GT_IF_WITH_ASSERT(pTreeHandler != nullptr)
            {
                pTreeHandler->OnNewFile(AF_TREE_ITEM_ITEM_NONE);
            }
        }
        break;

        case ID_KA_ANALYZE_SETTING:
        {
            afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
            GT_IF_WITH_ASSERT(NULL != pApplicationCommands)
            {
                pApplicationCommands->OnProjectSettings(KA_STR_projectSettingExtensionDisplayName);
            }
        }
        break;

        default:
            GT_ASSERT_EX(false, L"Unknown event id");
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        kaMenuActionsExecutor::handleUiUpdate
// Description: Handle UI update
// Arguments:   int actionIndex
// Author:      Gilad Yarnitzky
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void kaMenuActionsExecutor::handleUiUpdate(int actionIndex)
{
    bool isActionEnabled = false, isActionChecked = false, isActionCheckable = false;
    QString updatedActionText;

    if (NULL == m_pApplicationTree)
    {
        setApplicationTree();
    }

    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {
        case ID_KA_MODE:
        {
            kaApplicationCommands::instance().onUpdateKAModeCommand(isActionEnabled, isActionChecked);
            isActionCheckable = true;

            // Check if we are in analyze mode:
            bool isInKAMode = (afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode));

            updatedActionText = isInKAMode ? acGTStringToQString(KA_STR_executionModeMenu) : acGTStringToQString(KS_STR_SwitchToAnalyzeMode);
        }
        break;

        case ID_KA_OPENCL_BUILD:
        {
            kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();

            if (pTreeHandler != nullptr)
            {
                updatedActionText = pTreeHandler->GetBuildCommandString();
                updatedActionText.append("\t");
                updatedActionText.append(AF_STR_BuildShortcut);
                kaApplicationCommands::instance().onUpdateBuildCommand(isActionEnabled);
            }
        }
        break;

        case ID_KA_CANCEL_BUILD:
        {
            updatedActionText = KA_STR_CancelBuildASCII;
            updatedActionText.append("\t");
            updatedActionText.append(AF_STR_CancelBuildShortcut);
            kaApplicationCommands::instance().onUpdateCancelBuildCommand(isActionEnabled);
        }
        break;

        case ID_KA_ADD_FILE:
        case ID_KA_ADD_FILE_ANALYZE_MENU:
        {
            kaApplicationCommands::instance().onUpdateAddFileCommand(isActionEnabled);
        }
        break;

        case ID_KA_NEW_FILE:
        case ID_KA_NEW_FILE_ANALYZE_MENU:
        {
            // Same rules apply to create new file as to add file command:
            kaApplicationCommands::instance().onUpdateAddFileCommand(isActionEnabled);
        }
        break;

        case ID_KA_ANALYZE_SETTING:
        {
            isActionEnabled = (afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode));
        }
        break;

        default:
            GT_ASSERT_EX(false, L"Unknown event id");
            break;
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

        // Update the text if needed:
        if (!updatedActionText.isEmpty())
        {
            pAction->setText(updatedActionText);
        }
    }

}

// ---------------------------------------------------------------------------
// Name:        kaMenuActionsExecutor::groupAction
// Description: If the requested action should be a part of a group, create
//              the action group and add the relevant actions to the group
// Arguments:   int actionIndex
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        24/7/2011
// ---------------------------------------------------------------------------
void kaMenuActionsExecutor::groupAction(int actionIndex)
{
    (void)actionIndex;
}

void kaMenuActionsExecutor::setApplicationTree()
{
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();

    if (NULL != pApplicationCommands)
    {
        m_pApplicationTree = pApplicationCommands->applicationTree();
    }
}
