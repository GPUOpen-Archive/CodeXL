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
#include <inc/SessionWindow.h>

#include <memory>
#include <vector>

// Local:
#include <inc/DisplayFilter.h>
#include <AMDTCpuProfilingDataAccess/inc/AMDTCpuProfilingDataAccess.h>

class CpuSessionWindow;
class CpuProfileInfo;

class DisplayFilterDlg : public QDialog
{
    Q_OBJECT
public:

    /// Returns my single instance:
    static DisplayFilterDlg& instance();

    /// Display the dialog for the requested session path:
    QDialog::DialogCode displayDialog(const QString& sessionPath, bool enableOnlySystemDll);

    bool DisplaySystemDlls() const { return m_displaySystemDLLs; }
    bool ShowPercentage() const { return m_displayPercentageInColumn; }

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
    bool createConfigCounterMap();

    private slots:
    void onClickAllCoreItem(int state);
    void onClickCoreItem(int state);
    void onChangeView(const QString& newlySelectedView);
    void onClickOk();
    void onClickCancel();
    void onClickCheckBoxSeparateColumnsBy(int state);

private:

    static DisplayFilterDlg* m_psMySingleInstance;
    osFilePath          m_sessionFile;

    bool m_displaySystemDLLs = false;
    bool m_displayPercentageInColumn = false;
    afApplicationTreeItemData* m_pSessionTreeItemData = nullptr;
    CpuSessionWindow* m_pCurrentSessionWindow = nullptr;
    std::vector<QCheckBox*> m_pCheckBoxCore;
    QCheckBox* m_pCheckBoxAllCore = nullptr;
    QGridLayout*  m_pLayoutForCoreList = nullptr;
    QWidget* m_pWidgetCoreList = nullptr;
    int m_noOfCores = 0;
    int m_noOfColumn = 0;
    QWidget* m_pWidgetColumnList = nullptr;
    QVBoxLayout* m_pVBLayoutForColumn = nullptr;
    std::vector<QCheckBox*> m_pCheckBoxColumns;
    QLabel* m_plabelColumns = nullptr;
    QLabel* m_plabelGeneral = nullptr;
    QLabel* m_plabelCPUCores = nullptr;
    QComboBox* m_pComboBoxViewes = nullptr;
    QCheckBox* m_pCheckBoxDisplaySystemDLLs = nullptr;
    QCheckBox* m_pCheckBoxShowPercentageBars = nullptr;
    QCheckBox* m_pCheckBoxSeparateColumnsBy = nullptr;
    QRadioButton* m_pRadioButtonSeparateByCore = nullptr;
    QRadioButton* m_pRadioButtonSeparateByNUMA = nullptr;
    QPushButton* m_pPushButtonOK = nullptr;
    QPushButton* m_pPushButtonCancel = nullptr;
    QScrollArea* m_pScrollAreaColumns = nullptr;
    QScrollArea* m_pScrollAreaCPUCore = nullptr;
    QVBoxLayout* m_pMainLayout = nullptr;
    QHBoxLayout* m_pComboBoxViewesLayout = nullptr;
    QHBoxLayout* m_pScrollAreaColumnsLayout = nullptr;
    QHBoxLayout* m_pCheckBoxDisplaySystemDLLsLayout = nullptr;
    QHBoxLayout* m_pCheckBoxShowPercentageBarsLayout = nullptr;
    QHBoxLayout* m_pScrollAreaCPUCoreLayout = nullptr;
    QFrame* m_pLine = nullptr;
    QHBoxLayout* m_pCheckBoxSeparateColumnsByLayout = nullptr;
    QHBoxLayout* m_pCoreLayout = nullptr;
    QHBoxLayout* m_pNUMALayout = nullptr;
    QHBoxLayout* m_pButtonBox = nullptr;
    bool m_enableOnlySystemDll = false;
    shared_ptr<DisplayFilter> m_displayFilter = nullptr;

    std::shared_ptr<cxlProfileDataReader>   m_pProfDataReader;
    shared_ptr<cofigNameCounterMap>         m_CongigrationMap;
    QString                                 m_cofigName;
    gtVector<AMDTUInt32>                    m_selectedCounters;
    std::map<int, int>                      m_colIdxCounterIdMap;
    std::vector<gtString>					m_notChecked;


};


#endif//__DISPLAYFILTERDLG_H



