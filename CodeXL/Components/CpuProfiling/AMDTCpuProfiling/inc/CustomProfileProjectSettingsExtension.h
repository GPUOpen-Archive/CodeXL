//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CustomProfileProjectSettingsExtension.h
///
//==================================================================================

//------------------------------ CustomProfileProjectSettingsExtension.h ------------------------------

#ifndef __CUSTOMPROFILEPROJECTSETTINGSEXTENSION_H
#define __CUSTOMPROFILEPROJECTSETTINGSEXTENSION_H

// Qt:
#include <QtWidgets>

// AMDTCpuPerfEventUtils:
#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>

// Infra:
#include <AMDTApplicationFramework/Include/afProjectSettingsExtension.h>

const int MAX_UNITMASK = 8;

enum
{
    EVENT_NAME_COLUMN = 0,
    EVENT_INTERVAL_COLUMN,
    EVENT_UNITMASK_COLUMN,
    EVENT_USR_COLUMN,
    EVENT_OS_COLUMN
};



class CustomEventItemDelegate: public QItemDelegate
{
    Q_OBJECT

public:
    CustomEventItemDelegate(QObject* pParent);
    QWidget* createEditor(QWidget* pParent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

class CustomProfileProjectSettingsExtension : public afProjectSettingsExtension
{
    Q_OBJECT

public:
    CustomProfileProjectSettingsExtension();
    virtual ~CustomProfileProjectSettingsExtension();

    // Initialize the widget:
    virtual void Initialize();

    // Return the extension name:
    virtual gtString ExtensionXMLString();

    // Return the extension page title:
    virtual gtString ExtensionTreePathAsString();

    // Load / Save the project settings into a string:
    virtual bool GetXMLSettingsString(gtString& projectAsXMLString);
    virtual bool SetSettingsFromXMLString(const gtString& projectAsXMLString);
    virtual void RestoreDefaultProjectSettings();
    virtual bool AreSettingsValid(gtString& invalidMessageStr);

    virtual bool RestoreCurrentSettings();

    // Get the data from the widget:
    virtual bool SaveCurrentSettings();

protected slots:

    void OnUpdateRemoveAll();
    void OnAvailableEventDoubleClick(QTreeWidgetItem* pItem);
    void OnMonitoredEventDoubleClick(QTreeWidgetItem* pItem, int column);
    void OnAddEvent();
    void OnRemoveEvent();
    void OnRemoveAll();

    void OnEventEdited(QTreeWidgetItem* pItem, int column);
    void OnUnitMaskChanged();
    void OnOpsCountChanged();
    void OnMonitoredItemChanged(QTreeWidgetItem* pCurrent);
    void OnAvailableItemChanged(QTreeWidgetItem* pCurrent);
    void OnSettingsTreeSelectionAboutToChange();

protected:

    bool InitializeFromProfileType(const QString& profileType);
    void monitorEvent(QTreeWidgetItem* pAvailableItem);
    void hideSettingsBox();
    void highlightMonitored();
    void recurseHighlightChildren(QTreeWidgetItem* pItem, const QStringList& monitoredItems);
    bool validateUnique(QString& errString, unsigned int& ebpCount);
    bool validateIntervals(QString& errString);
    bool validateUsrOs(QString& errString);

    bool addConfigurationToTree(const gtString& name, QTreeWidget* pTree);
    bool addAllEventsToAvailable(bool isIbsAvailable, bool isAmdSystem);

    QString extendSourceName(const QString& source);
    QString buildEventName(gtUInt16 event, const QString& eventName);
    QString opToolTip(bool cycleCount);
    QString eventUnitMaskTip(gtUInt16 event, gtUByte unitMask);
    void SettingsWereChanged();

    QTreeWidgetItem* FindMonitoredEventItem(const QTreeWidgetItem& availableItem) const;
    QTreeWidgetItem* FindAvailableEventItem(QTreeWidgetItem& availableParent, const QTreeWidgetItem& monitoredItem) const;
    bool IsDuplicatableEvent(const QTreeWidgetItem& item) const;
    bool IsValidEvent(const QTreeWidgetItem& item) const;

private:

    EventEngine m_eventEngine;
    EventsFile* m_pEventFile;

    // Qt Widgets:
    QTreeWidget* m_pAvailableTree;
    QTreeWidget* m_pMonitoredEventsTreeWidget;
    QPushButton* m_pAddEvent;
    QPushButton* m_pRemoveEvent;
    QPushButton* m_pRemoveAll;
    QPlainTextEdit* m_pDescription;
    QLabel* m_pSettingsLabel;
    QScrollArea* m_pSettings;
    QLabel* m_pDescriptionTitle;
    QLabel* m_pErrLabel;
    QRadioButton* m_pDipatchRadio;
    QRadioButton* m_pCycleRadio;
    QCheckBox* m_pUnitMasks[MAX_UNITMASK];
    typedef QMap<QString, QTreeWidgetItem*> SourceMap;
    unsigned int m_maxEbpEvents;

    /// Contain true iff the custom profile settings were changed before moving to other tree node in settings tree:
    bool m_wereSettingsChanged;
    static bool m_sWasConnectedToTree;

};


#endif //__CUSTOMPROFILEPROJECTSETTINGSEXTENSION_H

