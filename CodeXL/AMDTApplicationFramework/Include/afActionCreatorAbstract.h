//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afActionCreatorAbstract.h
///
//==================================================================================

#ifndef __AFACTIONCREATORABSTRACT_H
#define __AFACTIONCREATORABSTRACT_H

class QAction;
class QPixmap;

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTApplicationComponents/Include/acIcons.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationFramework/Include/afActionPositionData.h>


// ----------------------------------------------------------------------------------
// Class Name:          afActionCreatorAbstract
// General Description: abstract implementation for the action creator
// Author:              Gilad Yarnitzky
// Creation Date:       14/7/2011
// ----------------------------------------------------------------------------------
class AF_API afActionCreatorAbstract
{
public:
    afActionCreatorAbstract();
    virtual ~afActionCreatorAbstract();

    // Initialize the creator:
    void initializeCreator();

    // Get the caption of the action item.  The common edit actions that
    // are shared by most action creators are covered by this function.
    // A derived class should over-ride this function if it implements
    // actions that do not appear in this list.
    virtual bool actionText(int actionIndex, gtString& actionText, gtString& tooltip, gtString& keyboardShortcut);

    // Menu position.
    // Each hierarchy on the ,emu include name/priority.
    // If separator is needed after the item then 's' after the priority is needed
    // in case of a sub menu if one item is marked with an 's' it is enough to mark a separator after it:
    virtual gtString menuPosition(int actionIndex, afActionPositionData& positionData) = 0;

    // Toolbar position. separators are defined by "/"
    // Position is defined as in menus:
    virtual gtString toolbarPosition(int actionIndex) = 0;

    // Get number of actions created:
    virtual int numberActions() { return m_supportedCommandIds.size();} ;

    // Get the group command ids for the action:
    virtual void groupAction(int actionIndex) = 0;

    // Get the specific action:
    QAction* action(int actionIndex);

    // Set action:
    void setAction(QAction* pAction, int actionIndex, int globalIndex);

    // Get the icon file:
    virtual QPixmap* iconAsPixmap(int actionIndex, bool& shouldUseInMenu);

    // Get the local index from a global index:
    int localFromGlobalActionIndex(int actionGlobalIndex) const;

    // Utilities (override this function is one of the actions has an icon):
    virtual void initActionIcons();
    void initSingleActionIcon(int commandId, acIconId iconId, bool displayInMenu = false);

    //  Get separator position
    //  -1 if separator should be added before toolbar item with nActionID, 0 - no separator(by default), 1 - after
    virtual int separatorPosition(int nActionID) { (void)(nActionID); return 0; }

    const gtVector<int>& supportedCommandIds() { return m_supportedCommandIds; }

    // Convert a local action index to the corresponding command id
    int actionIndexToCommandId(const int actionIndex) const;

    // Convert a command id to the corresponding local action index
    bool commandIdToActionIndex(const int commandId, int& actionIndex) const;

    class afCommandIconData
    {
    public:

        QPixmap* _pCommandPixmap;
        bool _isPixmapInitialized;
        bool _shouldDisplayInMenu;

        afCommandIconData();
    };

protected:

    // Each derived class must populate the vector of supported command Ids
    virtual void populateSupportedCommandIds() = 0;

    // The actions windows handled by the creator:
    gtVector<QAction*> m_actionsCreated;

    // Contain the icon data for the commands:
    gtVector<afCommandIconData> m_iconsDataVector;

    // A map from global action index to local:
    gtMap<int, int> m_globalIndexToLocalIndex;

    // Vector of supported command ids:
    gtVector<int> m_supportedCommandIds;

};


#endif //__AFACTIONCREATORABSTRACT_H

