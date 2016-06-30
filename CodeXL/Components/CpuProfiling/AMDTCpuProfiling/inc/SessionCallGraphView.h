//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SessionCallGraphView.h
/// \brief  Interface for the CallGraphTab class.
///
//==================================================================================
// $Id: //devtools/branch/CPUProfileGUI/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/CallGraphTab.h#2 $
// Last checkin:   $DateTime: 2013/03/24 03:43:56 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 467122 $
//=============================================================

#pragma once

// Qt:
#include <QtCore>
#include <QtWidgets>


// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acTreeCtrl.h>

#include <AMDTExecutableFormat/inc/ExecutableFile.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>
#include <SessionTreeNodeData.h>


//#include <QDockWidget>
#include <QFileInfo>

#include <AMDTCpuPerfEventUtils/inc/ViewConfig.h>
#include <AMDTCpuProfilingRawData/inc/CpuProfileReader.h>
#include <AMDTCpuCallstackSampling/inc/FunctionGraph.h>
#include "inc/DataTab.h"
#include <inc/StringConstants.h>

// need to undef Bool after all includes so the moc will compile in Linux
#undef Bool

class SessionCallGraphView;       // Forward reference
class FunctionGraph;
class CpuProfileCss;
class CallGraphFuncListItem;

// The columns shown in the instruction view
enum CallGraphButterflyColumns
{
    CALLGRAPH_NAME = 0,
    CALLGRAPH_ADDRESS,
    CALLGRAPH_SELF,
    CALLGRAPH_CHILDREN,
    CALLGRAPH_TOTAL,
    CALLGRAPH_OFFSET_INDEX,
    CALLGRAPH_INVALID,
    CALLGRAPH_BFY_NAME = 0,
    CALLGRAPH_BFY_ADDRESS,
    CALLGRAPH_BFY_SAMPLES,
    CALLGRAPH_BFY_INVALID
};

enum CallGraphFunctionsColumns
{
	CALLGRAPH_FUNCTION_NAME = 0,
    CALLGRAPH_FUNCTION_SELF_SAMPLES,
    CALLGRAPH_FUNCTION_DEEP_SAMPLES,
    CALLGRAPH_FUNCTION_DEEP_SAMPLE_PERCENTAGE,
    CALLGRAPH_FUNCTION_PATH_CNT,
#ifdef CSS_FUNC_PATH_INFO
    CALLGRAPH_FUNCTION_PATH_SAMPLES,
    CALLGRAPH_FUNCTION_PATH_SAMPLES_PER_PATH,
#endif
    CALLGRAPH_FUNCTION_SOURCE_FILE,
    CALLGRAPH_FUNCTION_MODULE,
    CALLGRAPH_FUNCTION_OFFSET, // used for column count

    CALLGRAPH_PATH_TREE = 0,
    CALLGRAPH_PATH_SELF,
    CALLGRAPH_PATH_DOWNSTREAM_SAMPLES,
    CALLGRAPH_PATH_DOWNSTREAM_PERCENTAGE,
    CALLGRAPH_PATH_MODULE,
    CALLGRAPH_PATH_OFFSET, // used for column count

    CALLGRAPH_BUTTERFLY_FUNCTION = 0,
    CALLGRAPH_BUTTERFLY_SAMPLES,
    CALLGRAPH_BUTTERFLY_PERCENTAGE,
    CALLGRAPH_BUTTERFLY_MODULE,
    CALLGRAPH_BUTTERFLY_OFFSET // used for column count
};

enum CallGraphDepth
{
    CALLGRAPH_MODULE = 0,
    CALLGRAPH_FUNCTIONS,
};



class FunctionsTreeCtrl : public acTreeCtrl
{
    Q_OBJECT

public:
    FunctionsTreeCtrl(SessionCallGraphView* pCallGraphTab, QWidget* pParent, bool addExpandCollapeAllActions, int numberOfColumns = 1);
    ~FunctionsTreeCtrl();

public slots:
    virtual void onAboutToShowContextMenu();

protected slots:
    void GoToSource();
    void GoToFunctionsView();
    void PathIndicatorToggled(int state);
    virtual void onContextMenuEvent(const QPoint& position);

protected:
    void AddCallGraphContextMenuActions();

signals:
    void editSourceFile(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32>);
    void functionSelected(AMDTFunctionId funcId);

private:
    SessionCallGraphView* m_pCallGraphTab;

    /// Context menu source code action:
    QAction* m_pSourceCodeAction;

    /// Context menu functions view action:
    QAction* m_pDisplayInFunctionsAction;
};

class CallGraphPathFuncList : public QWidget
{
    friend class acTablePercentItemDelegate;
    Q_OBJECT
public:
    CallGraphPathFuncList(QWidget* pParent);
    virtual ~CallGraphPathFuncList();
    bool initialize();
    void clear();

    FunctionsTreeCtrl* functionsTreeControl() const {return m_pFuncTable;}

    bool setFunction(gtUInt64 functionIndex);
	void SetFunctionPath(shared_ptr<cxlProfileDataReader> pProfDataRdr, AMDTFunctionId funcId, AMDTUInt32 processId, bool displaySysMod);
	//void SetFunctionPath(CpuProfileCss& css, const FunctionGraph::Node& funcNode);

    void setWindowTitle(const QString& title)  // Overload this
    {
        if (nullptr == m_pWindowLabel)
        {
            m_pWindowLabel = new QLabel;
        }

        m_pWindowLabel->setText(title);
    }

private slots:
    void onFunctionClicked(QTreeWidgetItem* pItem);
    void onCurrentListItemChanged(QTreeWidgetItem* pItem);
    void onSelectionChanged();
    void OnDblClicked(QTreeWidgetItem* pItem);

signals:
    void siteSelected(gtUInt64 funIdex);
    void passStatusMessage(QString message);
    void showPathData(gtUInt64 funcIndex);
    void functionSelected(AMDTFunctionId funcId);

private:
    //CallGraphFuncListItem* AcquireTopLevelItem(const FunctionGraph::Node* pFuncNode);
	CallGraphFuncListItem* AcquireTopLevelItem(AMDTFunctionId funcId, gtString funcName, gtString modulePath);
    //CallGraphFuncListItem* AcquireChildItem(CallGraphFuncListItem* pParentItem, const FunctionGraph::Node* pFuncNode);
	CallGraphFuncListItem* AcquireChildItem(CallGraphFuncListItem* pParentItem, AMDTFunctionId funcId, gtString funcName, gtString modulePath);
    void InitializeItem(CallGraphFuncListItem* pFuncListItem);
	void InitializeItem(CallGraphFuncListItem* pFuncListItem, gtString funcName, gtString modulePath);
    /// resize function name column
    void ResizeFunctionNameColumn();

    FunctionsTreeCtrl* m_pFuncTable;
    SessionCallGraphView* m_pCallGraphTab;
    QLabel* m_pWindowLabel;
};

class CallGraphFuncListItem : public QTreeWidgetItem
{
public:
	explicit CallGraphFuncListItem(const FunctionGraph::Node* pFuncNode = nullptr);
	explicit CallGraphFuncListItem(AMDTFunctionId funcId) { m_functionId = funcId;}
    CallGraphFuncListItem(acTreeCtrl* pParent, QTreeWidgetItem* pAfter);
    CallGraphFuncListItem(QTreeWidgetItem* pParent, QTreeWidgetItem* pAfter);
    virtual ~CallGraphFuncListItem();

    qulonglong GetCountValue(int column) const;
    qulonglong AddCountValue(int column, qulonglong value);
    void SetPercentageValue(int column, double numerator, double denominator);

    const FunctionGraph::Node* m_pFuncNode;
	AMDTUInt32	m_functionId;
	gtUInt64	m_selfSample;
	gtUInt64	m_moduleId;
	gtString	m_functionName;
	gtString	m_moduleName;
};

class CallGraphFuncList : public QWidget
{
    friend class acTablePercentItemDelegate;
    Q_OBJECT
public:
    CallGraphFuncList(QWidget* pParent);
    virtual ~CallGraphFuncList();
    void Initialize();
    void clear();
    void sortByColumn(int column, Qt::SortOrder order);
    CallGraphFuncListItem* DisplayFunctions(CpuProfileCss& css, int precision);
    FunctionsTreeCtrl* functionsTreeControl() const { return m_pFuncTable; }

    void setWindowTitle(const QString& title);
    CallGraphFuncListItem* getSelectedFunction();
    bool isFuncHidden(const FunctionGraph::Node& funcNode);
    CallGraphFuncListItem* HideSystemModules(CpuProfileCss& css);
    CallGraphFuncListItem* UnhideAllItems();

    void SetFunctionNameHeader(int totalFuncCount, int shownFuncCount);

    CallGraphFuncListItem* FindTopLevelItem(const FunctionGraph::Node& funcNode);

    // insert new row to list - indication for empty table
    void InitEmptyTableRow();
    // hides/unhides empty table item row in the table list
    void HandleDisplayEmptyTableItem(int count);
    bool FillDisplayFuncList(shared_ptr<cxlProfileDataReader> pProfDataRdr,
                             bool isSystemDLLIgnored,
                             AMDTUInt32 counterId,
                             AMDTUInt32 processId, 
		AMDTFunctionId& funcIdMaxSamples);

    CallGraphFuncListItem* AddFuncListItem(AMDTFunctionId functionId,
										   gtString functionName,
                                           gtUInt64 totalSelfSamples,
                                           gtUInt64 totalDeepSamples,
                                           double   deepSamplesPerc,
                                           gtUInt32 pathCount,
                                           gtString srcFile,
                                           gtString moduleName,
                                           gtUInt32 srcFileLine,
										AMDTUInt32 moduleId);

	std::map<gtString, std::pair<int, AMDTFunctionId>> m_FuncNameIdxMap;

public slots:
    void selectAFunction(AMDTFunctionId funcId);

private slots:
    void onFunctionClicked(QTreeWidgetItem* pItem);
    void onCurrentListItemChanged(QTreeWidgetItem* pItem);
    void onSelectionChanged();
    void OnDblClicked(QTreeWidgetItem* pItem);
    void sortIndicatorChanged(int col, Qt::SortOrder order);

signals:
    void siteSelected(gtUInt64 funIdex);
    void passStatusMessage(QString message);
    void showPathData(gtUInt64 funcIndex);
    void functionSelected(AMDTFunctionId funcId);

protected:
    bool operator<(const QTreeWidgetItem& other)const;

    // saves one item of empty row message per table
    CallGraphFuncListItem* m_pEmptyTableMsgItem;

private:
    CallGraphFuncListItem* AddTopLevelItem(const FunctionGraph::Node& funcNode,
                                           unsigned pathsNumber, gtUInt64 selfCount, gtUInt64 deepCount);
    /// resize function name column
    void ResizeFunctionNameColumn();
	QString GetFileNameEntry(const gtString& srcFile, int srcLIne);
	QString GetModuleNameEntry(const gtString& moduleName);

    FunctionsTreeCtrl* m_pFuncTable;
    gtUInt64 m_DeepSampleTotal;
    SessionCallGraphView* m_pCallGraphTab;
    QLabel* m_pWindowLabel;

    // Last sort information:
    Qt::SortOrder m_lastSortOrder;
    int m_lastSortColumn;
};


class CallGraphButterfly : public QWidget
{
    Q_OBJECT
public:
    CallGraphButterfly(QWidget* pParent);
    virtual ~CallGraphButterfly();

    FunctionsTreeCtrl* parentsTreeControl() {return m_pParentsTreeControl;}
    FunctionsTreeCtrl* childrenTreeControl() {return m_pChildrenTreeControl;}

    bool Initialize();
    void clear();

#if 0
    void SetParentsFunction(CpuProfileCss& css, const FunctionGraph::Node& funcNode);
    void SetChildrenFunction(CpuProfileCss& css, const FunctionGraph::Node& funcNode);
#endif

    void SetParentsFunction(shared_ptr<cxlProfileDataReader> pProfDataRdr,
                            bool displaySystemModule,
                            AMDTUInt32 processId,
                            AMDTUInt32 counterId, 
		AMDTFunctionId	funcId);

    void SetChildrenFunction(shared_ptr<cxlProfileDataReader> pProfDataRdr,
                             bool displaySystemModule,
                             AMDTUInt32 processId,
                             AMDTUInt32 counterId, 
		AMDTFunctionId	funcId);

    void setWindowTitle(const QString& title)  // Overload this
    {
        m_pWindowLabel->setText(title);
    }

private slots:
    void onFunctionClicked(QTreeWidgetItem* pItem);
    void onCurrentListItemChanged(QTreeWidgetItem* pItem);
    void onSelectionChanged();

signals:
    void siteSelected(QString funName, gtUInt64 address, bool isParent);
    void siteSelectedFunction(gtUInt64 address, bool isParent);
    void passStatusMessage(QString message);
    void functionSelected(AMDTFunctionId funcId);

private:
    CallGraphFuncListItem* AddTopLevelItem(FunctionsTreeCtrl& treeCtrl,
                                           const CpuProfileModule* pModule,
                                           const FunctionGraph::Node* pFuncNode,
                                           const QString& funcName,
                                           qulonglong samplesCount,
                                           double totalDeepCount,
                                           bool noSamples);


	CallGraphFuncListItem* AddTopLevelItem(FunctionsTreeCtrl& treeCtrl,
											const gtString modulePath,
											const gtString& funcName,
											double samplesCount,
											double totalDeepCount,
											double samplePercent,
											bool noSamples);
    QTreeWidget* m_pLastTree;
    SessionCallGraphView* m_pCallGraphTab;

    FunctionsTreeCtrl* m_pParentsTreeControl;
    FunctionsTreeCtrl* m_pChildrenTreeControl;
    QLabel* m_pWindowLabel;
};

class SessionCallGraphView : public DataTab
{
    friend class CallGraphFuncList;
    friend class CallGraphPathFuncList;
    friend class CallGraphButterfly;

    Q_OBJECT

public:
    SessionCallGraphView(QWidget* pParent, CpuSessionWindow* pSessionWindow, afApplicationTreeItemData* pDisplayedSessionItemData);
    ~SessionCallGraphView();

public:
    bool Display(const QString& caption, unsigned int pid = SHOW_ALL_PIDS);
    bool fillCounterIndicatorCombo();

    void showPid(unsigned int pid);
    //void ShowParentChild(CpuProfileCss& css, const FunctionGraph::Node& funcNode);
	bool ShowParentChild(AMDTFunctionId functionId);
    void selectFunction(AMDTFunctionId functionId);
	void selectFunction(const QString& functionName, ProcessIdType pid);
	void ShowPaths(AMDTFunctionId functionId);

    ColumnSpec m_cuurentColSpec;

    /// Updates the current tables display according to the needed update type:
    /// \param updateType the type of update needs to be performed (see SettingsDifference for details):
    virtual void UpdateTableDisplay(unsigned int updateType);

    EventMaskType getFilterEvent() const;

    void displayInFunctionView(const QString& functionName);

protected:

    acToolBar* CreateToolbar();
    void SetViewLayout();

private:

    QFileInfo m_sessionFile;
    CallGraphButterfly* m_pButterfly = nullptr;
    unsigned int m_pid;
    acWidgetAction* m_pPidComboAction = nullptr;
    CallGraphFuncList* m_pFuncTable = nullptr;
    CallGraphPathFuncList* m_pPathFuncTable = nullptr;

    QSplitter* m_pSplitter = nullptr;

    acWidgetAction* m_pHotSpotIndicatorComboBoxAction = nullptr;

    TableDisplaySettings m_displaySettings;

    /// \param pid the process ID to select when the Combo-Box is ready
    void fillPIDComb(unsigned int pid);

    int getPidFromProcessString(QString processString);

    //selected pid and counterId
    AMDTUInt32				m_selectedPID;
	AMDTUInt32				m_selectedCounter;
	AMDTFunctionId			m_selectedFuncId;
	bool					m_isSystemDLLDisplayed	= false;

    std::vector<AMDTUInt32> m_hotSpotCounterIdMap;
	std::map<gtString, AMDTFunctionId> m_FunctionNameIdMap;

public slots:
    void OnDblClicked(QTreeWidgetItem* pItem);
    void OnSelectionChange();
    //void OnIncludeSystemModules();
    //void OnExcludeSystemModules();
    void onOpenDisplayFilterDialog();
    // When a list item is expanded
    void OnExpandItem(QTreeWidgetItem* pItem);
    void OnSelectPid(int index);
    void FunctionListSelectionDone(AMDTFunctionId funcId);
    void OnButterflyClicked(QString funName, gtUInt64 address, bool parent);
    void editSource(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32>);
    void OnSelectHotSpotIndicator(int index);
    void filterMonitoredEvent(QStringList& eventNameList);
    //overwriting
    virtual void onViewChanged();
    virtual void onEditCopy();
    virtual void onEditSelectAll();
    virtual void onFindClick();
    virtual void onFindNext();

signals:

	void functionSelected(AMDTFunctionId funcId);
    void showParentChildFunction(gtUInt64 selFuncIndex);
    void showSelectedPaths(gtUInt64 selFuncIndex);
    //void functionActivated(gtVAddr functionAddress, ProcessIdType pid, ThreadIdType tid, const CpuProfileModule* pModule);
	void opensourceCodeViewSig(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32> funcModInfo);

};