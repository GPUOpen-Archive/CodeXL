//------------------------------ kaMultiSourceActionCreator.h ------------------------------

// QT:
#include <QAction>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTKernelAnalyzer/src//kaMultiSourceActionCreator.h>
#include <AMDTKernelAnalyzer/Include/kaCommandIDs.h>

// ---------------------------------------------------------------------------
// Name:        kaMultiSourceActionCreator::kaMultiSourceActionCreator
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        10/9/2013
// ---------------------------------------------------------------------------
kaMultiSourceActionCreator::kaMultiSourceActionCreator()
{

}

// ---------------------------------------------------------------------------
// Name:        kaMultiSourceActionCreator::~kaMultiSourceActionCreator
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        10/9/2013
// ---------------------------------------------------------------------------
kaMultiSourceActionCreator::~kaMultiSourceActionCreator()
{
}

// ---------------------------------------------------------------------------
// Name:        kaMultiSourceActionCreator::populateSupportedCommandIds
// Description: Create a vector of command Ids that are supported by this actions creator object
// Author:      Doron Ofek
// Date:        Mar-3, 2015
// ---------------------------------------------------------------------------
void kaMultiSourceActionCreator::populateSupportedCommandIds()
{
    // fill the vector of supported command ids:
    m_supportedCommandIds.push_back(ID_CUT);
    m_supportedCommandIds.push_back(ID_COPY);
    m_supportedCommandIds.push_back(ID_PASTE);
    m_supportedCommandIds.push_back(ID_FIND);
    m_supportedCommandIds.push_back(ID_FIND_NEXT);
    m_supportedCommandIds.push_back(ID_FIND_PREV);
    m_supportedCommandIds.push_back(ID_SELECT_ALL);
    m_supportedCommandIds.push_back(ID_SHOW_LINE_NUMBERS);
    m_supportedCommandIds.push_back(AF_ID_SAVE_FILE);
    m_supportedCommandIds.push_back(AF_ID_SAVE_FILE_AS);
}

// ---------------------------------------------------------------------------
// Name:        kaMultiSourceActionCreator::menuPosition
// Description: Menu position
//              Each hierarchy on the ,emu include name/priority.
//              If separator is needed after the item then 's' after the priority is needed
//              in case of a sub menu if one item is marked with an 's' it is enough to mark a separator after it:
// Arguments:   int actionIndex
//              positionData - defines the item position within its parent menu
// Author:      Gilad Yarnitzky
// Date:        10/9/2013
// ---------------------------------------------------------------------------
gtString kaMultiSourceActionCreator::menuPosition(int actionIndex, afActionPositionData& positionData)
{
    gtString retVal;

    positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_NONE;

    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {

        case ID_CUT:
        case ID_COPY:
        case ID_PASTE:
        case ID_FIND:
        case ID_FIND_NEXT:
        case ID_FIND_PREV:
        case ID_SELECT_ALL:
            retVal = AF_STR_EditMenuString;
            break;

        case ID_SHOW_LINE_NUMBERS:
        {
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
            retVal = AF_STR_ViewMenuString;

            positionData.m_beforeActionMenuPosition = AF_STR_ViewMenuString;
            positionData.m_beforeActionText = AF_STR_ResetGUILayouts;
        }
        break;

        case AF_ID_SAVE_FILE:
        case AF_ID_SAVE_FILE_AS:
        {
            retVal = AF_STR_FileMenuString;
            positionData.m_beforeActionMenuPosition = AF_STR_FileMenuString;
            positionData.m_beforeActionText = AF_STR_ProjectSettings;
        }
        break;

        default:
            GT_ASSERT(false);
            break;
    };

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaMultiSourceActionCreator::toolbarPosition
// Description: Toolbar position
// Arguments:   int actionIndex
// Author:      Gilad Yarnitzky
// Date:        10/9/2013
// ---------------------------------------------------------------------------
gtString kaMultiSourceActionCreator::toolbarPosition(int actionIndex)
{
    (void)(actionIndex); // unused
    gtString retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaMultiSourceActionCreator::groupAction
// Description: Group actions if needed
// Arguments:   int actionIndex
// Author:      Gilad Yarnitzky
// Date:        10/9/2013
// ---------------------------------------------------------------------------
void kaMultiSourceActionCreator::groupAction(int actionIndex)
{
    (void)(actionIndex); // unused
}
