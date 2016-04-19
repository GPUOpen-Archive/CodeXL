//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DisplayFilterDlg.h
///
//==================================================================================

#ifndef __DISPLAYFILTERDLG_H
#define __DISPLAYFILTERDLG_H

// Qt:
#include <QDialog>
#include <QStandardItemModel>
#include <QListWidgetItem>
#include <QRadioButton>
#include <QGridLayout>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>

// Local:
#include <inc/DisplayFilter.h>

class CpuSessionWindow;
class CpuProfileInfo;
class CpuProfileReader;

class DisplayFilterDlg : public QDialog
{
    Q_OBJECT
public:

    /// Returns my single instance:
    static DisplayFilterDlg& instance();

    /// Display the dialog for the requested session path:
    QDialog::DialogCode displayDialog(const QString& sessionPath, bool enableOnlySystemDll);

    /// Return the edited session settings:
    const SessionDisplaySettings& SessionSettings() const {return m_displaySettings;}

    bool DisplaySystemDlls() const {return m_displaySystemDLLs;}
    bool ShowPercentage() const {return m_displayPercentageInColumn;}

    virtual ~DisplayFilterDlg();

private:

    DisplayFilterDlg(QWidget* pParent);

    bool initializeConfiguration();
    bool populateCoreList(int noOfCores);
    bool populateColumnList();
    bool initializeLayout();
    void disableAllControlsExceptSystemDll(bool disable);
    void addFinalLayout();
    void updateHiddenColumnList();

private slots:
    void onClickAllCoreItem(int state);
    void onClickCoreItem(int state);
    void onChangeView(const QString& newlySelectedView);
    void onClickOk();
    void onClickCancel();
    void onClickCheckBoxSeparateColumnsBy(int state);

private:

    static DisplayFilterDlg* m_psMySingleInstance;

    /// The edited settings:
    SessionDisplaySettings m_displaySettings;
    bool m_displaySystemDLLs;
    bool m_displayPercentageInColumn;

    afApplicationTreeItemData* m_pSessionTreeItemData;
    CpuSessionWindow*   m_pCurrentSessionWindow;
    CpuProfileReader*    m_pProfileReader;
    CpuProfileInfo*      m_pProfileInfo;

    // Cores:
    QCheckBox* m_pCheckBoxCore;
    QCheckBox* m_pCheckBoxAllCore;
    QGridLayout*  m_pLayoutForCoreList;
    QWidget* m_pWidgetCoreList;
    int m_noOfCores;
    int m_noOfColumn;

    QWidget* m_pWidgetColumnList;
    QVBoxLayout* m_pVBLayoutForColumn;
    QCheckBox* m_pCheckBoxColumns;//


    osFilePath          m_sessionFile;

    QLabel* m_plabelColumns;
    QLabel* m_plabelGeneral;
    QLabel* m_plabelCPUCores;

    QComboBox* m_pComboBoxViewes;

    QCheckBox* m_pCheckBoxDisplaySystemDLLs;
    QCheckBox* m_pCheckBoxShowPercentageBars;
    QCheckBox* m_pCheckBoxSeparateColumnsBy;

    QRadioButton* m_pRadioButtonSeparateByCore;
    QRadioButton* m_pRadioButtonSeparateByNUMA;

    QPushButton* m_pPushButtonOK;
    QPushButton* m_pPushButtonCancel;

    QScrollArea* m_pScrollAreaColumns;
    QScrollArea* m_pScrollAreaCPUCore;
    QVBoxLayout* m_pMainLayout;

    QHBoxLayout* m_pComboBoxViewesLayout;
    QHBoxLayout* m_pScrollAreaColumnsLayout;
    QHBoxLayout* m_pCheckBoxDisplaySystemDLLsLayout;
    QHBoxLayout* m_pCheckBoxShowPercentageBarsLayout;
    QHBoxLayout* m_pScrollAreaCPUCoreLayout;
    QFrame* m_pLine;
    QHBoxLayout* m_pCheckBoxSeparateColumnsByLayout;
    QHBoxLayout* m_pCoreLayout;
    QHBoxLayout* m_pNUMALayout;
    QHBoxLayout* m_pButtonBox;
    bool m_enableOnlySystemDll;

};
#endif//__DISPLAYFILTERDLG_H



