
#ifndef __AFACTIONPOSITIONDATA_H
#define __AFACTIONPOSITIONDATA_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:          AF_API afActionPositionData
// General Description: This class is used to hold an action position within a menu
// Author:              Sigal Algranaty
// Creation Date:       15/4/2012
// ----------------------------------------------------------------------------------
class AF_API afActionPositionData
{
public:
    afActionPositionData()
        :
        m_actionPosition(AF_POSITION_MENU_CENTER_BLOCK),
        m_beforeActionMenuPosition(L""),
        m_beforeActionText(L""),
        m_actionSeparatorType(AF_SEPARATOR_NONE)
    {

    };
    enum afCommandPosition
    {
        AF_POSITION_MENU_START_BLOCK,
        AF_POSITION_MENU_CENTER_BLOCK,
        AF_POSITION_MENU_END_BLOCK
    };

    enum afCommandSeparatorType
    {
        AF_SEPARATOR_NONE,
        AF_SEPARATOR_BEFORE_COMMAND,
        AF_SEPARATOR_BEFORE_COMMAND_GROUP
    };

    // Should the action be in start / middle / end block of the menu:
    afCommandPosition m_actionPosition;

    // If the action should come before another action within it's block, these strings contain this action description:
    gtString m_beforeActionMenuPosition;
    gtString m_beforeActionText;

    // Separator type: none / separator before me / separator before my group:
    afCommandSeparatorType m_actionSeparatorType;

};

#endif //__AFACTIONPOSITIONDATA_H

