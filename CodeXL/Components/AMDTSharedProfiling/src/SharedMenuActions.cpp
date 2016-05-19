//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SharedMenuActions.cpp
///
//==================================================================================

#ifdef _WIN32
    #include <Windows.h>
#endif

// QT:
#include <QtCore>
#include <QtWidgets>

#include <QtWidgets/QAction>
#include <QtWidgets/QMessageBox>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afQtCreatorsManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>

// Local:
#include <inc/SharedProfileManager.h>
#include <inc/SharedMenuActions.h>
#include <inc/StringConstants.h>

const QString SHARED_PROFILE_MRU_KEY = "UserMRUProfiles";

/// Static variable instance
SharedMenuActions* SharedMenuActions::m_pMySingleInstance = NULL;

SharedMenuActions::SharedMenuActions() : afActionExecutorAbstract(),
    m_user(QSettings::NativeFormat, QSettings::UserScope, "AMD", "CodeXL")
{
    m_pProfiles = &SharedProfileManager::instance();

    connect(m_pProfiles, SIGNAL(profileSelectionChanged(const gtString&)),
            this, SLOT(onUpdateSelection(const gtString&)));

    //Read the user-persistent MRU profiles
    QVariant value = m_user.value(SHARED_PROFILE_MRU_KEY, QString());

    if (value.isValid())
    {
        QStringList mruList = value.toString().split("\n");

        for (QStringList::Iterator it = mruList.begin(); it != mruList.end(); ++it)
        {
            if ((*it).isEmpty())
            {
                continue;
            }

            gtString currentProfile = acQStringToGTString(*it);
            m_mruProfiles.push_back(currentProfile);
        }
    }
}

SharedMenuActions::~SharedMenuActions(void)
{
    m_user.sync();
}

// ---------------------------------------------------------------------------
// Name:        SharedMenuActions::populateSupportedCommandIds
// Description: Create a vector of command Ids that are supported by this actions creator object
// Author:      Doron Ofek
// Date:        Mar-3, 2015
// ---------------------------------------------------------------------------
void SharedMenuActions::populateSupportedCommandIds()
{
    // fill the vector of supported command ids:
    m_supportedCommandIds.push_back(ID_PM_START_PROFILE);
    m_supportedCommandIds.push_back(ID_PM_PAUSE_PROFILE);
    m_supportedCommandIds.push_back(ID_PM_STOP_PROFILE);
    m_supportedCommandIds.push_back(ID_PM_ATTACH_PROFILE);
    m_supportedCommandIds.push_back(ID_PM_SELECTED_PROFILE);
    m_supportedCommandIds.push_back(ID_PM_PROFILE_SETTINGS_DIALOG);
}

SharedMenuActions& SharedMenuActions::instance()
{
    // If this class single instance was not already created:
    if (NULL == m_pMySingleInstance)
    {
        // Create it:
        m_pMySingleInstance = new SharedMenuActions;
        GT_ASSERT(m_pMySingleInstance);
        afQtCreatorsManager::instance().registerActionExecutor(m_pMySingleInstance);
    }

    return *m_pMySingleInstance;
}

void SharedMenuActions::initActionIcons()
{
    // Start command:
    initSingleActionIcon(ID_PM_START_PROFILE, AC_ICON_EXECUTION_PLAY);

    // Pause command:
    initSingleActionIcon(ID_PM_PAUSE_PROFILE, AC_ICON_EXECUTION_PAUSE);

    // Pause command:
    initSingleActionIcon(ID_PM_STOP_PROFILE, AC_ICON_EXECUTION_STOP);

    // Stop command:
    initSingleActionIcon(ID_PM_STOP_PROFILE, AC_ICON_EXECUTION_STOP);
}

/// Each hierarchy on the menu include name/priority.
/// If separator is needed after the item then 's' after the priority is needed
/// in case of a sub menu if one item is marked with an 's' it is enough to mark a separator after it:
bool SharedMenuActions::actionText(int actionIndex, gtString& caption, gtString& tooltip, gtString& keyboardShortcut)
{
    (void)(keyboardShortcut); // unused
    bool retVal = true;
    //Three cases:
    // actionIndex < COUNT_OF_STATIC_PM_MENUS: static menu items
    // COUNT_OF_STATIC_PM_MENUS <= actionIndex  < COUNT_OF_STATIC_PM_MENUS + # of mru profiles
    int middleRange = MAX_MRU_PROFILES + COUNT_OF_STATIC_PM_MENUS;
    // COUNT_OF_STATIC_PM_MENUS + # of mru profiles <= actionIndex
    int maxRange = m_pProfiles->profiles().size() + MAX_MRU_PROFILES + COUNT_OF_STATIC_PM_MENUS;

    if (actionIndex <= COUNT_OF_STATIC_PM_MENUS)
    {
        // Get the command id:
        int commandId = actionIndexToCommandId(actionIndex);

        switch (commandId)
        {
            case ID_PM_START_PROFILE:
                caption = PM_STR_MENU_START;
                tooltip = PM_STR_STATUS_START;
                break;

            case ID_PM_ATTACH_PROFILE:
                caption = PM_STR_MENU_ATTACH;
                tooltip = PM_STR_STATUS_ATTACH;
                break;

            case ID_PM_SELECTED_PROFILE:
                caption = PM_STR_PROFILE_MODE_MENU_COMMAND_PREFIX;
                caption.append(m_pProfiles->currentSelection());
                tooltip = PM_STR_STATUS_SELECT;
                break;

            case ID_PM_PROFILE_SETTINGS_DIALOG:
                caption = PM_STR_MENU_SETTINGS;
                tooltip = PM_STR_STATUS_SETTINGS;
                break;

            case ID_PM_PAUSE_PROFILE:
                caption = PM_STR_MENU_PAUSE;
                tooltip = PM_STR_STATUS_PAUSE;
                break;

            case ID_PM_STOP_PROFILE:
                caption = PM_STR_MENU_STOP;
                tooltip = PM_STR_STATUS_STOP;
                break;

            default:
            {
                GT_ASSERT_EX(false, L"Unsupported application command");
                retVal = false;
                break;
            }
        }
    }
    else if (actionIndex < middleRange)
    {
        //Initial caption of mru
        caption = PM_STR_SELECT_EMPTY;
        tooltip = L"Select a profile";
    }
    else if (actionIndex < maxRange)
    {
        int profileIndex = actionIndex - middleRange;
        caption = m_pProfiles->profiles().at(profileIndex);
        addAccelerator(caption);

        tooltip = L"Select a profile";
    }
    else
    {
        GT_ASSERT_EX(false, L"Unsupported application command");
        retVal = false;
    }

    return retVal;
}

gtString SharedMenuActions::menuPosition(int actionIndex, afActionPositionData& positionData)
{
    gtString retVal;

    positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_NONE;

    //Three cases:
    // actionIndex < COUNT_OF_STATIC_PM_MENUS: static menu items
    // COUNT_OF_STATIC_PM_MENUS <= actionIndex  < COUNT_OF_STATIC_PM_MENUS + # of mru profiles
    int middleRange = MAX_MRU_PROFILES + COUNT_OF_STATIC_PM_MENUS;
    // COUNT_OF_STATIC_PM_MENUS + # of mru profiles <= actionIndex
    int maxRange = m_pProfiles->profiles().size() + MAX_MRU_PROFILES + COUNT_OF_STATIC_PM_MENUS;


    if (actionIndex <= COUNT_OF_STATIC_PM_MENUS)
    {
        // Get the command id:
        int commandId = actionIndexToCommandId(actionIndex);

        switch (commandId)
        {
            // Profile menu
        case ID_PM_START_PROFILE:
        case ID_PM_PAUSE_PROFILE:
        case ID_PM_STOP_PROFILE:
                retVal = AF_STR_ProfileMenuString;
                positionData.m_beforeActionMenuPosition = AF_STR_ProfileMenuString;
                positionData.m_beforeActionMenuPosition.append(PM_STR_MENU_SEPARATOR);
                positionData.m_beforeActionText = PM_STR_MENU_SETTINGS;
                break;

            case ID_PM_ATTACH_PROFILE:
                retVal = AF_STR_ProfileMenuString;
                positionData.m_beforeActionMenuPosition = AF_STR_ProfileMenuString;
                positionData.m_beforeActionMenuPosition.append(PM_STR_MENU_SEPARATOR);
                positionData.m_beforeActionText = PM_STR_MENU_SETTINGS;
                positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
                break;

            case ID_PM_SELECTED_PROFILE:
                positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
                retVal = AF_STR_ProfileMenuString;
                positionData.m_beforeActionMenuPosition = AF_STR_ProfileMenuString;
                positionData.m_beforeActionMenuPosition.append(PM_STR_MENU_SEPARATOR);
                positionData.m_beforeActionText = PM_STR_MENU_SETTINGS;
                break;

            case ID_PM_PROFILE_SETTINGS_DIALOG:
                positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
                retVal = AF_STR_ProfileMenuString;
                break;

            default:
            {
                GT_ASSERT_EX(false, L"Unsupported application command");
                break;
            }
        }
    }
    else if (actionIndex < middleRange)
    {
        int profileIndex = actionIndex - COUNT_OF_STATIC_PM_MENUS;

        //Add a separator for the 1st
        if (0 == profileIndex)
        {
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
        }

        positionData.m_beforeActionMenuPosition = AF_STR_ProfileMenuString;
        positionData.m_beforeActionMenuPosition.append(PM_STR_MENU_SEPARATOR);
        positionData.m_beforeActionText = ID_PM_SELECTED_PROFILE;
        retVal = AF_STR_ProfileMenuString;
    }
    else if (actionIndex < maxRange)
    {
        unsigned int profileIndex = actionIndex - middleRange;

        //Add a separator for the 1st
        if (0 == profileIndex)
        {
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
        }

        positionData.m_beforeActionMenuPosition = AF_STR_ProfileMenuString;
        positionData.m_beforeActionMenuPosition.append(PM_STR_MENU_SEPARATOR);
        positionData.m_beforeActionText = PM_STR_MENU_SETTINGS;
        retVal = AF_STR_ProfileMenuString;
    }
    else
    {
        GT_ASSERT_EX(false, L"Unsupported application command");
    }

    return retVal;
}

gtString SharedMenuActions::toolbarPosition(int actionIndex)
{
    (void)(actionIndex); // unused
    gtString retVal;

    return retVal;
}

int SharedMenuActions::numberActions()
{
    //The number of actions created.
    return m_pProfiles->profiles().size() + MAX_MRU_PROFILES + COUNT_OF_STATIC_PM_MENUS;
}

void SharedMenuActions::handleTrigger(int actionIndex)
{
    // Get the QT action object:
    QAction* pAction = action(actionIndex);

    if (NULL == pAction)
    {
        return;
    }

    //Three cases:
    // actionIndex < COUNT_OF_STATIC_PM_MENUS: static menu items
    // COUNT_OF_STATIC_PM_MENUS <= actionIndex  < COUNT_OF_STATIC_PM_MENUS + # of mru profiles
    // COUNT_OF_STATIC_PM_MENUS + # of mru profiles <= actionIndex
    int maxRange = m_pProfiles->profiles().size() + MAX_MRU_PROFILES + COUNT_OF_STATIC_PM_MENUS;

    if (actionIndex <= COUNT_OF_STATIC_PM_MENUS)
    {
        // Get the command id:
        int commandId = actionIndexToCommandId(actionIndex);

        switch (commandId)
        {
            case ID_PM_START_PROFILE:
                updateMru(m_pProfiles->currentSelection());
                m_pProfiles->onStartAction();
                break;

            case ID_PM_ATTACH_PROFILE:
                updateMru(m_pProfiles->currentSelection());
                m_pProfiles->onInvokeAttachToProcess();
                break;

            case ID_PM_SELECTED_PROFILE:
                m_pProfiles->onSelectProfileMode(false);
                break;

            case ID_PM_PROFILE_SETTINGS_DIALOG:
                m_pProfiles->onInvokeProjectSettings();
                break;

            case ID_PM_PAUSE_PROFILE:
                m_pProfiles->onPauseToggle();
                break;

            case ID_PM_STOP_PROFILE:
                m_pProfiles->stopCurrentRun();
                break;

            default:
            {
                GT_ASSERT_EX(false, L"Unsupported application command");
                break;
            }
        }
    }
    else if (actionIndex < maxRange)
    {
        std::wstring    wpAction(pAction->text().toStdWString());
        gtString text(wpAction.c_str());

        updateMru(text);
        int acceleratorPos = text.find(L"&");

        if (acceleratorPos >= 0)
        {
            text = text.removeChar(text[acceleratorPos]);
        }

        updateSelected(text);
        m_pProfiles->updateSelected(text);
    }
    else
    {
        GT_ASSERT_EX(false, L"Unsupported application command");
    }
}

void SharedMenuActions::handleUiUpdate(int actionIndex)
{
    bool isActionEnabled(false), isActionChecked(false), isActionCheckable(false), isShown(true);
    QString actionText;

    // Get the QT action object:
    QAction* pAction = action(actionIndex);

    if (NULL == pAction)
    {
        return;
    }

    //Three cases:
    // actionIndex < COUNT_OF_STATIC_PM_MENUS: static menu items
    // COUNT_OF_STATIC_PM_MENUS <= actionIndex  < COUNT_OF_STATIC_PM_MENUS + # of mru profiles
    int middleRange = MAX_MRU_PROFILES + COUNT_OF_STATIC_PM_MENUS;
    // COUNT_OF_STATIC_PM_MENUS + # of mru profiles <= actionIndex
    int maxRange = m_pProfiles->profiles().size() + MAX_MRU_PROFILES + COUNT_OF_STATIC_PM_MENUS;

    if (actionIndex <= COUNT_OF_STATIC_PM_MENUS)
    {
        // Get the command id:
        int commandId = actionIndexToCommandId(actionIndex);

        switch (commandId)
        {
            //Profile menu:
            case ID_PM_START_PROFILE:
                isActionEnabled = m_pProfiles->isStartEnabled(isActionCheckable, isActionChecked);
                actionText = SharedProfileManager::instance().FindStartProfileActionText();
                break;

            case ID_PM_ATTACH_PROFILE:
                isActionEnabled = m_pProfiles->isAttachEnabled(isActionCheckable, isActionChecked);
                break;

            case ID_PM_SELECTED_PROFILE:
            {
                isActionEnabled = m_pProfiles->isProfileModeEnabled(isActionCheckable, isActionChecked);
                bool isInProfileMode = (afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE));
                gtString text = isInProfileMode ? PM_STR_PROFILE_MODE_MENU_COMMAND_PREFIX : PM_STR_SWITCH_TO_PROFILE_MODE_MENU_COMMAND_PREFIX;
                text.append(SharedProfileManager::instance().currentSelection());

                // Get the command as Qt string:
                actionText = acGTStringToQString(text);
            }
            break;

            case ID_PM_PROFILE_SETTINGS_DIALOG:
                isActionEnabled = m_pProfiles->isProjectSettingsEnabled(isActionCheckable, isActionChecked);
                break;

            case ID_PM_PAUSE_PROFILE:
            {
                isActionEnabled = m_pProfiles->isPausedEnabled(isActionCheckable, isActionChecked, isShown);
                pAction->setText(acGTStringToQString(PM_STR_MENU_PAUSE));
                pAction->setToolTip(acGTStringToQString(PM_STR_STATUS_PAUSE));

                if (m_pProfiles->selectedSessionTypeName().startsWith(L"CPU:"))
                {
                    pAction->setText(acGTStringToQString(PM_STR_MENU_PAUSE_DATA));
                    pAction->setToolTip(PM_STR_STATUS_PAUSE_DATA);
                }
            }
            break;

            case ID_PM_STOP_PROFILE:
                isActionEnabled = m_pProfiles->isStopEnabled(isActionCheckable, isActionChecked);
                break;

            default:
                GT_ASSERT_EX(false, L"Unknown event id");
                break;
        }
    }
    else if (actionIndex < middleRange)
    {
        std::wstring    wpAction(pAction->text().toStdWString());
        gtString profileType(wpAction.c_str());

        //At this point, the MRU profile list is never shown
        isShown = false;
        isActionEnabled = m_pProfiles->isProfileEnabled(m_pProfiles->indexForSessionType(profileType),
                                                        isActionCheckable, isActionChecked);
    }
    else if (actionIndex < maxRange)
    {
        isActionEnabled = m_pProfiles->isProfileEnabled((actionIndex - middleRange),
                                                        isActionCheckable, isActionChecked);
    }
    else
    {
        GT_ASSERT_EX(false, L"Unsupported application command");
    }

    GT_IF_WITH_ASSERT(pAction != NULL)
    {
        // Set the action enable / disable:
        pAction->setEnabled(isActionEnabled);

        // Set the action checkable state:
        pAction->setCheckable(isActionCheckable);

        // Set the action check state:
        pAction->setChecked(isActionChecked);

        pAction->setVisible(isShown);

        if (!actionText.isEmpty())
        {
            pAction->setText(actionText);
        }
    }
}

void SharedMenuActions::groupAction(int actionIndex)
{
    (void)(actionIndex); // unused
}

void SharedMenuActions::updateMru(const gtString& newMru)
{
    //search the mru list for the selected profile
    bool bFound(false);
    unsigned int i;

    for (i = 0; i < m_mruProfiles.size(); i++)
    {
        if (0 == newMru.compareNoCase(m_mruProfiles.at(i)))
        {
            bFound = true;
            break;
        }
    }

    //If it's in the list, but not the top item
    if ((bFound) && (i != 0))
    {
        m_mruProfiles.removeItem(i);
        bFound = false;
    }

    // if it's not found
    if (!bFound)
    {
        m_mruProfiles.insert(m_mruProfiles.begin(), newMru);
    }

    //If the mru list has too many items
    while (m_mruProfiles.size() > MAX_MRU_PROFILES)
    {
        m_mruProfiles.pop_back();
    }

    //Update the text of the mru menu items
    QString value;

    for (i = 0; i < m_mruProfiles.size(); i++)
    {
        QAction* pUpdate = action(i + COUNT_OF_STATIC_PM_MENUS + 1);

        if (NULL == pUpdate)
        {
            break;
        }

        pUpdate->setText(m_mruProfiles.at(i).asASCIICharArray());
        value += QString::fromWCharArray(m_mruProfiles.at(i).asCharArray()) + '\n';
    }

    //Update the user-persistent mru list
    m_user.setValue(SHARED_PROFILE_MRU_KEY, value.trimmed());
    m_user.sync();
}

void SharedMenuActions::updateSelected(const gtString& selected)
{
    m_pProfiles->updateSelected(selected);
}

void SharedMenuActions::onUpdateSelection(const gtString& selected)
{
    int actionIndex;
    QAction* pCurrentAction = NULL;

    GT_IF_WITH_ASSERT(commandIdToActionIndex(ID_PM_SELECTED_PROFILE, actionIndex))
    {
        pCurrentAction = action(actionIndex);
    }

    if (NULL != pCurrentAction)
    {
        // Check if we are in profile mode:
        bool isInProfileMode = (afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE));

        gtString caption = isInProfileMode ? PM_STR_PROFILE_MODE_MENU_COMMAND_PREFIX : PM_STR_SWITCH_TO_PROFILE_MODE_MENU_COMMAND_PREFIX;
        caption.append(selected);
        pCurrentAction->setText(caption.asASCIICharArray());
    }
}

void SharedMenuActions::addAccelerator(gtString& caption)
{
    static QMap<QString, QString> sCaptionToCaptionsWithAccelerators;
    static bool sInitialized = false;

    if (!sInitialized)
    {
        sCaptionToCaptionsWithAccelerators[PM_profileTypeTimeBasedPrefix] = PM_profileTypeTimeBasedPrefixWithAccelerator;
        sCaptionToCaptionsWithAccelerators[PM_profileTypeCustomProfilePrefix] = PM_profileTypeCustomProfilePrefixWithAccelerator;
        sCaptionToCaptionsWithAccelerators[PM_profileTypeCLUPrefix] = PM_profileTypeCLUPrefixWithAccelerator;
        sCaptionToCaptionsWithAccelerators[PM_profileTypeAssesPerformancePrefix] = PM_profileTypeAssesPerformancePrefixWithAccelerator;
        sCaptionToCaptionsWithAccelerators[PM_profileTypeInstructionBasedSamplingPrefix] = PM_profileTypeInstructionBasedSamplingPrefixWithAccelerator;
        sCaptionToCaptionsWithAccelerators[PM_profileTypeInvestigateBranchingPrefix] = PM_profileTypeInvestigateBranchingPrefixWithAccelerator;
        sCaptionToCaptionsWithAccelerators[PM_profileTypeInvestigateDataAccessPrefix] = PM_profileTypeInvestigateDataAccessPrefixWithAccelerator;
        sCaptionToCaptionsWithAccelerators[PM_profileTypeInvestigateInstructionAccessPrefix] = PM_profileTypeInvestigateInstructionAccessPrefixWithAccelerator;
        sCaptionToCaptionsWithAccelerators[PM_profileTypeInvestigateInstructionL2CacheAccessPrefix] = PM_profileTypeInvestigateInstructionL2CacheAccessPrefixWithAccelerator;
        sCaptionToCaptionsWithAccelerators[PM_profileTypeThreadProfilePrefix] = PM_profileTypeThreadProfilePrefixWithAccelerator;
        sCaptionToCaptionsWithAccelerators[PM_profileTypePerformanceCountersPrefix] = PM_profileTypePerformanceCountersPrefixWithAccelerator;
        sCaptionToCaptionsWithAccelerators[PM_profileTypeApplicationTracePrefix] = PM_profileTypeApplicationTracePrefixWithAccelerator;
        sCaptionToCaptionsWithAccelerators[PM_profileTypePowerProfilePrefix] = PM_profileTypePowerProfilePrefixWithAccelerator;
    }

    QString qstrCaption = acGTStringToQString(caption);
    GT_IF_WITH_ASSERT(sCaptionToCaptionsWithAccelerators.contains(qstrCaption))
    {
        QString commandWithAccelerator = sCaptionToCaptionsWithAccelerators[qstrCaption];
        caption = acQStringToGTString(commandWithAccelerator);
    }
}


//TODO On project open, read mru list
//TODO On project close, write mru list
