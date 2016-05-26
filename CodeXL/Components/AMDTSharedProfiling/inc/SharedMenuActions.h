//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SharedMenuActions.h
///
//==================================================================================

#ifndef _SHAREDMENUACTIONS_H
#define _SHAREDMENUACTIONS_H
//QT
#include <QtCore>
#include <QtWidgets>

#include <qsettings.h>

// Infra:
#include <AMDTApplicationFramework/Include/afActionExecutorAbstract.h>

class SharedProfileManager;


class SharedMenuActions : public QObject, public afActionExecutorAbstract
{
    Q_OBJECT
public:
    /// Get the singleton instance
    static SharedMenuActions& instance();

    /// Destructor
    ~SharedMenuActions();

    //inherited from afActionExecutorAbstract
    /// Handle the action when it is triggered
    virtual void handleTrigger(int actionIndex);

    /// Handle UI update of an action
    virtual void handleUiUpdate(int actionIndex);

    //inherited from afActionCreatorAbstract
    /// Get the caption of the action item
    virtual bool actionText(int actionIndex, gtString& actionText, gtString& tooltip, gtString& keyboardShortcut);

    void addAccelerator(gtString& caption);

    /// Get the menu position of the action item
    virtual gtString menuPosition(int actionIndex, afActionPositionData& positionData);

    /// Get the toolbar position of the action item
    virtual gtString toolbarPosition(int actionIndex);

    /// Get number of actions created for the component
    virtual int numberActions();

    /// Get the group command ids for the action
    virtual void groupAction(int actionIndex);

    /// Utilities override this function if one of the actions has an icon
    virtual void initActionIcons();

    /// Handle all gui actions needed when the selection changes
    void updateSelected(const gtString& selected);

public slots:
    void onUpdateSelection(const gtString& selected);

protected:
    /// Constructor
    SharedMenuActions();

    // Each derived class must populate the vector of supported command Ids
    virtual void populateSupportedCommandIds();

    /// The count of 'static' menu items
    int m_menuCmdCount;

    /// The singleton instance
    static SharedMenuActions* m_pMySingleInstance;

    ///
    SharedProfileManager* m_pProfiles;
};

#endif //_SHAREDMENUACTIONS_H
