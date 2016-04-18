//------------------------------ kaStatisticsView.h ------------------------------

#ifndef __KASTATISTICSVIEW_H
#define __KASTATISTICSVIEW_H

// Qt.
#include <QtWidgets>

// Infra.
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local.
#include <AMDTKernelAnalyzer/src/kaDataTypes.h>
#include <AMDTKernelAnalyzer/src/kaTableView.h>
#include <AMDTKernelAnalyzer/src/kaBuildToolbar.h>

class kaStatisticsViewSIMDDelegate;
class kaStatisticsViewTipDelegate;
class kaProjectDataManagerAnaylzeData;

class kaStatisticsView : public kaTableView
{
    Q_OBJECT

public:
    kaStatisticsView(QWidget* pParent, const osFilePath& kernelFilePath, const osFilePath& detailedFilePath, kaPlatform platform, const gtString& buildProfile);

    virtual ~kaStatisticsView();

    /// Update the view by reloading the file and rebuilding the table:
    virtual bool updateView();

protected:
    /// Handle data for device in a special way
    virtual bool handleDeviceData(QString& deviceName, QString& familyName, QString& deviceInfo);

    virtual void resizeEvent(QResizeEvent* event);

protected slots:
    /// Handle changes in the device combobox:
    void OnDeviceComboBoxChanged(int deviceIndex);

    /// Handle editing of workgroup data
    void OnWorkGroupEdit();

    /// some editing was done on the workgroup items, enable validation
    void OnWorkGroupTextEdit(const QString& text);

    /// Handle editing of dynamic LDS
    void OnDynamicLDSEdit();

private:
    /// Get the kernel info
    void GetKernelDataInfo(QWidget* pParent);

    /// Create the upper panel with the workgroup and dynamic LDS info:
    void CreateUpperPanel(QVBoxLayout* pVBoxLayout);

    /// Create the upper Table
    void CreateUpperTable(QVBoxLayout* pVBoxLayout);

    /// Create the lower Table
    void CreateLowerTable();

    /// Create information tables
    void CreateInformationTables();

    /// Create the SC source shader tab and CS data tab:
    void CreateISATabs();

    /// Recreate information tables for SGPRs
    void RebuildSGPRsInformationTable(const QString& deviceDetails, unsigned actualSGPRs, int& SGPRsWavesLimit);

    /// initialize pair of tables: label and info
    void InitTableAndLabels(acListCtrl*& labelSide, acListCtrl*& infoSide, QBoxLayout* pLayoutToInsert);

    /// Resize to largest content between three tables
    void ResizeToLargestOfThree(acListCtrl* pTable0, acListCtrl* pTable1, acListCtrl* pTable2);

    /// Sets the bold item and 3d for the info table
    void HighlighInfoItem(acListCtrl* pInfoTable, int itemIndex);

    /// update all items related to workgroup info
    /// returns LDS waves limit
    int UpdateWorkgroupBasedInfo();

    /// Update recommanded action based on waves limit of SGPRs VGPRs & LDS
    void UpdateRecommendation(int SGPRsLimit, int VGPRsLimit, int LDSLimit);

    /// int to KB string
    QString BytesToKBString(int iValue);

    /// Disable selection on a table for number of rows
    void DisableTableSelection(acListCtrl* pTable, int startRow, int numRows);

    /// Calc the maximum constrain based on all three criteria
    void CalcMaximumConstrain();

    /// Update the ISA sections in the information table:
    void UpdateISASections(const QString& deviceText);

    /// return true if the file is a shader file
    /// \returns true if shader file
    bool IsShaderFile() const;

    /// checks if the shader build target is compute shader
    /// \returns true if the shader build target is cs
    bool IsComputeShader() const;

    /// should LDS be enabled
    /// \return true if LDS should be enabled
    bool IsLdsEnabled() const;

    /// Get kernel name
    /// \returns the kernel name from the detailed path
    QString GetKernelName(const osFilePath& detailedPath);

    /// Disables context menu in info tables
    void SetTablesContextMenuPolicy(Qt::ContextMenuPolicy policy = Qt::NoContextMenu);

    /// Update local workgroup dimensions
    /// by seeking function names in each DX compute shader
    /// that have been built and taking 3 numbers from preceding line with keyword "numthreads"
    /// \param [in] pFile - source file info used by statistics view
    /// \param [in] entryPoint - DX shader entry point name
    /// \param [in] shaderProfile - DX shader type and model combination
    void UpdateLocalWorkgroupDimensions(kaSourceFile* pFile, const gtString& entryPoint, const gtString& shaderProfile);

    /// Stacked layout to show only one layer based on selected combo box
    QStackedLayout* m_pStackedLayout;

    /// two widgets that holds the layers to be displayed
    QWidget* m_pNILayerTableWidget;
    QWidget* m_pSILayerTableWidget;

    /// Combobox string description
    QLabel* m_pComboDescription;

    /// Combo box to select the different devices
    QComboBox* m_pDeviceComboBox;

    /// two tables of information for SI and later devices
    acListCtrl* m_pUpperTable;
    acListCtrl* m_pLowerTable;

    /// map between the device full name (family name:device name) and its data
    gtMap<QString, QString> m_mapDeviceNameToData;

    /// Kernel Data
    kaProjectDataManagerAnaylzeData* m_pKernelInfo;

    /// Information:
    QGroupBox* m_pInfoGroupBoxTab1;

    /// Group box for the SC Shader dump text:
    QLabel* m_pSCSourceShaderTab2;

    /// Group box for the CS data:
    QLabel* m_pCSDataTab3;

    /// The tab widget containing Performance Reference Data, SC Source Shader Data and CS data:
    QTabWidget* m_pInfoTabWidget;

    /// SGPRs Information tables labels
    acListCtrl* m_pSGRPsInfoTableLabels;
    /// SGPRs Information tables
    acListCtrl* m_pSGRPsInfoTable;

    /// VGPRs Information tables labels
    acListCtrl* m_pVGRPsInfoTableLabels;
    /// VGPRs Information tables
    acListCtrl* m_pVGRPsInfoTable;

    /// SGPR Information tables labels
    acListCtrl* m_pLDSInfoTableLabels;
    /// SGPR Information tables
    acListCtrl* m_pLDSInfoTable;

    /// delegates used for drawing the information tables:
    kaStatisticsViewTipDelegate* m_pInfoDelegates[3];

    /// LDS information line
    QLabel* m_pLDSInfoLine;

    /// Max wave label:
    acListCtrl* m_pMaxWavesTable;

    /// Advice layout
    QHBoxLayout* m_pAdviceBox;

    /// recommendation label for the user
    QLabel* m_pRecommendLabel;

    /// local workgroup information
    QLineEdit* m_pWorkgroup[3];

    /// dynamic LDS usage value inserted by user
    QLineEdit* m_pDynamicLDS;

    /// saving the lsat value of the dynamic LDS - for checking if update needed when OnDynamicLDSEdit called
    int m_lastDynamicLDS;

    /// LDS used cached after selecting device
    int m_LDSUsed;

    /// scroll area
    QScrollArea* m_pScrollArea;

    /// scroll area widget
    QWidget* m_pScrollAreaWidget;

    /// tables resized after initial creation
    bool m_tablesResized;

    /// was the dynamic value changed. if it was do not update dynamic value when changing device
    bool m_wasDynamicChangedByUser;

    /// Contain the kernel file path:
    osFilePath m_kernelFilePath;

    /// directX / openCL platform
    kaPlatform m_platform;

    gtString m_buildProfile;

    /// Should check the validity of workgroup values. The values should be checked after the first editingFinished and set again
    /// after the first gain focus. this is because the way the VS sends the editingFinished if we show the warning dialog: it resends the
    /// editingFinished event even if already lost the foucs
    bool m_shouldCheckWorkGroupValues;

    /// kernel name
    QString m_kernelName;
};

class kaStatisticsViewSIMDDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    kaStatisticsViewSIMDDelegate(acListCtrl* ownerTable, int useColor, bool showColumns, QObject* parent = 0) : QStyledItemDelegate(parent), m_pOwningTable(ownerTable), m_useColor(useColor), m_showColumns(showColumns) {};
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
    /// owning Table
    acListCtrl* m_pOwningTable;

    /// Should use color then define which column number
    int m_useColor;

    /// Should show Columns
    bool m_showColumns;
};

class kaStatisticsViewTipDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    kaStatisticsViewTipDelegate(QObject* parent = 0) : QStyledItemDelegate(parent), m_column(-1) {};
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void setColumn(int column) { m_column = column; };
private:
    /// highlighted area:
    int m_column;

};

#endif // __KASTATISTICSVIEW_H

