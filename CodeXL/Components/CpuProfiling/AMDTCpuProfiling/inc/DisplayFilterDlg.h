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

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>

// Standard:
#include <memory>
#include <vector>


class CpuSessionWindow;
class DisplayFilter;
class cxlProfileDataReader;


class DisplayFilterDlg : public QDialog
{
    Q_OBJECT

public:
    /// Returns my single instance:
    static DisplayFilterDlg& instance();

    /// Display the dialog for the requested session path:
    QDialog::DialogCode displayDialog(const QString& sessionPath, bool enableOnlySystemModule);

    bool isDisplaySystemModules() const { return m_displaySystemModules; }
    bool isShowPercentage() const { return m_displayPercentageInColumn; }

    virtual ~DisplayFilterDlg();

private:
    DisplayFilterDlg(QWidget* pParent);

    bool initializeConfiguration();
    bool populateCoreList(gtUInt32 noOfCores);
    bool populateColumnList();
    bool initializeLayout();
    void disableAllControlsExceptSystemModule(bool disable);
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
    osFilePath          m_sessionFile;

    bool m_displaySystemModules = false;
    bool m_displayPercentageInColumn = false;
    afApplicationTreeItemData* m_pSessionTreeItemData = nullptr;
    CpuSessionWindow* m_pCurrentSessionWindow = nullptr;
    std::vector<QCheckBox*> m_pCheckBoxCore;
    QCheckBox* m_pCheckBoxAllCore = nullptr;
    QGridLayout*  m_pLayoutForCoreList = nullptr;
    QWidget* m_pWidgetCoreList = nullptr;
    gtUInt32 m_noOfCores = 0;
    gtUInt32 m_noOfColumn = 0;
    QWidget* m_pWidgetColumnList = nullptr;
    QVBoxLayout* m_pVBLayoutForColumn = nullptr;
    std::vector<QCheckBox*> m_pCheckBoxColumns;
    QLabel* m_plabelColumns = nullptr;
    QLabel* m_plabelGeneral = nullptr;
    QLabel* m_plabelCPUCores = nullptr;
    QComboBox* m_pComboBoxViewes = nullptr;
    QCheckBox* m_pCheckBoxDisplaySystemModules = nullptr;
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
    QHBoxLayout* m_pCheckBoxDisplaySystemModulesLayout = nullptr;
    QHBoxLayout* m_pCheckBoxShowPercentageBarsLayout = nullptr;
    QHBoxLayout* m_pScrollAreaCPUCoreLayout = nullptr;
    QFrame* m_pLine = nullptr;
    QHBoxLayout* m_pCheckBoxSeparateColumnsByLayout = nullptr;
    QHBoxLayout* m_pCoreLayout = nullptr;
    QHBoxLayout* m_pNUMALayout = nullptr;
    QHBoxLayout* m_pButtonBox = nullptr;
    bool m_enableOnlySystemModule = false;
    std::shared_ptr<DisplayFilter> m_displayFilter;
    std::shared_ptr<cxlProfileDataReader>   m_pProfDataReader;
    QString                                 m_cofigName;
    std::map<int, int>                      m_colIdxCounterIdMap;
    std::vector<QString>                    m_unSelectedCounters;
};


#endif//__DISPLAYFILTERDLG_H



