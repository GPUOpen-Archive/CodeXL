//------------------------------ kaTableView.h ------------------------------

#ifndef __KATABLEVIEW_H
#define __KATABLEVIEW_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtMap.h>

class acQHTMLWindow;
class acListCtrl;

#define KA_ANALYSIS_TABLE_HEIGHT 17

class kaTableView : public QWidget
{
    Q_OBJECT

public:
    kaTableView(QWidget* pParent, const osFilePath& kernelFilePath, int amountOfHeaderRows = 2, int amountOfColsPerDevice = 1);

    void resizeToContent(acListCtrl* pTable);

    virtual ~kaTableView();

    /// Update the view by reloading the file and rebuilding the table:
    virtual bool updateView();

    /// Edit menu commands
    virtual void onUpdateEdit_Copy(bool& isEnabled);
    virtual void onUpdateEdit_SelectAll(bool& isEnabled);
    virtual void onUpdateEdit_Find(bool& isEnabled);
    virtual void onUpdateEdit_FindNext(bool& isEnabled);

    virtual void onEdit_Copy();
    virtual void onEdit_SelectAll();
    virtual void onEdit_Find();
    virtual void onEdit_FindNext();

protected:
    /// Initialize the main table (m_pAnalysisTable)
    void initializeMainTable(const QString& tableCaption, const QString& tableRows, const QString& tableRowsTooltip);

    /// Holds the amount of items in the vertical header:
    int m_amountOfHeaderRows;

    /// The amount of cells that should be merged for each device:
    int m_amountOfColsPerDevice;

    // Read the data:
    void readDataFile();

    // Add an item with color:
    void addItem(acListCtrl* pTable, int row, int column, QString text, QString toolTip = "");

    // Find the color for the requested cell:
    QColor findColorForCell(int row, int column);

    // Update the table background color:
    void updateBGColors();

    /// Handle data for device in a special way
    virtual bool handleDeviceData(QString& deviceName, QString& familyName, QString& deviceInfo)
    {
        GT_UNREFERENCED_PARAMETER(deviceName);
        GT_UNREFERENCED_PARAMETER(familyName);
        GT_UNREFERENCED_PARAMETER(deviceInfo);

        return false;
    }

    // File path of the file that used to open the view:
    osFilePath m_filePath;

    // Main view layout:
    QVBoxLayout* m_pMainLayout;

    /// Table information caption
    QLabel* m_pTableInformationCaption;

    /// Holds all the internal views:
    acListCtrl* m_pAnalysisTable;

    /// Holds the background colors for the non-white cells:
    /// The background colors usage is detailed in findColorForCell function implementation:
    QColor m_a1Color;
    QColor m_a2Color;
    QColor m_b1Color;
    QColor m_b2Color;
    QColor m_RedColor;

private:
    /// Map containing the family name with the column index:
    gtMap<QString, int> m_familyNameColumnMap;

    /// Contain the row index for the cycles data:
    int m_cyclesRowIndex;
    /// The row index for CodeLenInByte data:
    int m_CodeLenIndex;
    /// The row of ScratchRegs in statistics view- need to know if to mark it or not
    int m_ScratchRegsIndex;
};

#endif // __KATABLEVIEW_H

