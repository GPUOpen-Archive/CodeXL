//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpDisplaySettingsDialog.h
///
//==================================================================================

//------------------------------ tpDisplaySettingsDialog.h ------------------------------

#ifndef __TPDISPLAYSETTINGSDIALOG_H
#define __TPDISPLAYSETTINGSDIALOG_H


// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <QtCore>
#include <QDialog>
#include <QGridLayout>
#include <QCheckBox>

// Backend:
#include <AMDTThreadProfileDataTypes.h>

// Local
#include <inc/tpSessionData.h>

class tpDisplaySettingsDialog : public QDialog
{
    Q_OBJECT
public:
    /// constructor
    tpDisplaySettingsDialog(QWidget* pParent = nullptr, tpSessionData* pSessionData = nullptr);

    /// gets the data selected int he dialog window
    /// \param (out) selectedCoresStr - string of selected cores. if all cores selected returns "all Cores" string. if non selected returns "non cores selected"
    /// \param (out) selectedProcsNum - the number of selected processes
    /// \param (out) selectedThreadsNum - the number of selected threads
    void GetTpSettingsData(QString& selectedCoresStr, int& selectedProcsNum, int& selectedThreadsNum);

    /// gets the selected processes and thread map
    //// \param procsThreadsMap is a reference to the map to be set
    void SelectedProcessesAndThreadsMap(QMap<AMDTProcessId, QVector<AMDTThreadId> >& procsThreadsMap);

    /// gets the selected cores List
    /// \param coresList a refernce to the cores list to be set
    void SelectedCoresList(QVector<QString>& coresList);

private slots:
    /// function called on Ok button clicked
    void OnClickOk();

    /// function called on cancel button clicked
    void OnClickCancel();

    /// function called on one of the processes tree items check state changed
    /// \params not used
    void ProcessesTreeItemChanged(QTreeWidgetItem*, int);

    /// function called on one of the core check-boxes check state changed
    /// \param checked is true when check box checked
    void CoreCheckStateChanged(bool checked);

    /// function called on o "all" check-box check state changed
    /// \param checked is true when check box checked
    void AllCBCheckStateChanged(bool checked);

private:
    /// initialize dialog window layouts
    void initializeLayout();

    /// Creates cores check-boxes by getting system cores from backend
    void SetCoresColumn();

    /// creates the process and threads tree by getting the current processes and thread map from backend
    void SetProcessesTree();

    /// updates tree items when one of the tree items check state was changed
    /// if the changed check state is of a top level item - updates all it's child items
    /// if the changed item is a child item - update it's parent item by going over the parents children list and deciding the new check state of the parent
    /// \param treeWidgetItem - is the changed tree item
    void UpdateTreeWidgetItemCheckState(QTreeWidgetItem* treeWidgetItem);

    /// gets the number of selected cores - se;ected check-boxes in cores list
    /// \returns the number of selected cores
    int SelectedCoresCount();

    /// gets a string of names of selected cores - selected check-boxes in cores list
    /// \returns the selected cores names list. if no cores selected returns "no cores selected". if all cores selected returns "all cores"
    void SelectedCoresString(QString& str);

    /// gets the number of selected processes - selected top level items in processes tree
    /// \returns the number of selected processes
    int SelectedProcessesCount();

    /// gets the number of selected threads - selected child level items in processes tree
    /// \returns the number of selected threads
    int SelectedThreadsCount();

    /// creates copy of the m_processes tree into m_copiedProcessTree
    void CreateCopiedTree();

    /// copies check state of all tree items from source tree to target tree
    /// \param srcTree - source tree
    /// \param dstTree - target tree
    void CopyCheckStatesFromTree(QTreeWidget* srcTree, QTreeWidget* dstTree);

    /// update the last saved cores state list by current check-boxes list state
    void UpdateLastCoresSelection();

    /// set cores check-boxes selection state by the last saved
    void UpdateFromLastCoresSelection();

    /// get process id by name
    /// \param name is the process or thread name from item tree
    /// \returns process/thread id
    AMDTProcessId GetIdFromTreeItem(const QString& name);


    /// vertical cores layout
    QVBoxLayout* m_pCoresLayout;
    /// vertical processes layout
    QVBoxLayout* m_pProcessesLayout;

    /// processes and threads tree
    QTreeWidget* m_pProcessTree;
    /// backup copied tree
    QTreeWidget* m_lastProcessesTree;
    /// cores check-boxes list
    QVector<QCheckBox*> m_coresCBsList;
    /// backup copied cores check-boxes list
    QVector<Qt::CheckState> m_lastCoresStatesList;

    /// session data
    tpSessionData* m_pSessionData;
};

#endif //__TPDISPLAYSETTINGSDIALOG_H
