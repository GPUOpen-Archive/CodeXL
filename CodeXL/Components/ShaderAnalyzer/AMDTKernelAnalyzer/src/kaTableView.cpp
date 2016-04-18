//------------------------------ kaTableView.cpp ------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/Include/acQHTMLWindow.h>

// Framework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaTableView.h>
#include <AMDTKernelAnalyzer/src/kaDataAnalyzerFunctions.h>
#include <AMDTKernelAnalyzer/src/kaGlobalVariableManager.h>
#include <AMDTKernelAnalyzer/src/kaTreeModel.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>

#define KA_ANALYSIS_INSTRUCTION_CHACHE_SIZE 32768

// ---------------------------------------------------------------------------
// Name:        kaTableView::kaTableView
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        24/8/2013
// ---------------------------------------------------------------------------
kaTableView::kaTableView(QWidget* pParent, const osFilePath& kernelFilePath,
                         int amountOfHeaderRows, int amountOfColsPerDevice)
    : QWidget(pParent), m_amountOfHeaderRows(amountOfHeaderRows), m_amountOfColsPerDevice(amountOfColsPerDevice), m_filePath(kernelFilePath), m_pMainLayout(NULL), m_pTableInformationCaption(NULL), m_pAnalysisTable(NULL), m_cyclesRowIndex(-1), m_CodeLenIndex(-1), m_ScratchRegsIndex(-1)
{
    // Get the system default background color:
    QColor bgColor = acGetSystemDefaultBackgroundColor();
    m_b2Color = QColor(bgColor.red() * 0.9, bgColor.green() * 0.9, bgColor.blue() * 0.9);
    m_b1Color = QColor(bgColor.red() * 0.85, bgColor.green() * 0.85, bgColor.blue() * 0.85);
    m_a2Color = bgColor;
    m_a1Color = QColor(bgColor.red() * 0.95, bgColor.green() * 0.95, bgColor.blue() * 0.95);
    m_RedColor = QColor(bgColor.red() * 0.85, bgColor.green() * 0.0, bgColor.blue() * 0.0);

    m_b2Color = m_a1Color;
    m_b1Color = m_a2Color;

    m_pMainLayout = new QVBoxLayout(this);
}

// ---------------------------------------------------------------------------
void kaTableView::initializeMainTable(const QString& tableCaption, const QString& tableRows, const QString& tableRowsTooltip)
{
    m_pAnalysisTable = new acListCtrl(NULL, KA_ANALYSIS_TABLE_HEIGHT);

    m_pAnalysisTable->setShowGrid(true);

    // Add the basic lines of the table:
    m_pAnalysisTable->verticalHeader()->hide();
    m_pAnalysisTable->horizontalHeader()->hide();

    m_pAnalysisTable->setColumnCount(1);
    m_pAnalysisTable->insertRow(0);
    addItem(m_pAnalysisTable, 0, 0, KA_STR_statisticsTableMainColumn);

    QStringList tableRowsList = QString(tableRows).split(",");
    // "," is used in the strings so a different splitter is needed in the tooltip:
    QStringList tableRowsTooltipList = QString(tableRowsTooltip).split("#");
    int numRows = tableRowsList.count();

    for (int nRow = 0; nRow < numRows; nRow++)
    {
        m_pAnalysisTable->insertRow(nRow + 1);
        addItem(m_pAnalysisTable, nRow + 1, 0, tableRowsList[nRow], tableRowsTooltipList[nRow]);

        if (tableRowsList[nRow] == KA_STR_analysisTotalCyclesRowName)
        {
            m_cyclesRowIndex = nRow;
        }
        else if (tableRowsList[nRow] == KA_STR_analysisCodeLen)
        {
            m_CodeLenIndex = nRow + 1;
        }
        else if ((tableCaption == KA_STR_statisticsTableInfo) && (tableRowsList[nRow] == KA_STR_statisticsScratchReg))
        {
            m_ScratchRegsIndex = nRow + 1;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        kaTableView::~kaTableView
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        24/8/2013
// ---------------------------------------------------------------------------
kaTableView::~kaTableView()
{
}

// ---------------------------------------------------------------------------
// Name:        kaTableView::readDataFile
// Description: read the data file and add it to the table
// Author:      Gilad Yarnitzky
// Date:        24/8/2013
// ---------------------------------------------------------------------------
void kaTableView::readDataFile()
{
    osFile fileToRead;
    bool rc = fileToRead.open(m_filePath, osChannel::OS_ASCII_TEXT_CHANNEL);
    m_familyNameColumnMap.clear();

    QString lastDeviceInserted;

    int numDevices = 0;
    GT_IF_WITH_ASSERT(rc)
    {
        gtASCIIString lineRead;

        bool isFirstLineRead = fileToRead.readLine(lineRead);

        if (isFirstLineRead)
        {
            while (fileToRead.readLine(lineRead))
            {
                gtString lineConverted;
                lineConverted.fromASCIIString(lineRead.asCharArray());

                // Extract the device name.
                size_t deviceStrEnd = lineRead.find(",");
                gtASCIIString deviceNameAscii = lineRead.substr(0, deviceStrEnd);
                gtASCIIString deviceInfoAscii = lineRead.substr(deviceStrEnd + 1);

                gtString deviceName;
                gtString deviceInfo;
                deviceName.fromASCIIString(deviceNameAscii.asCharArray());
                deviceInfo.fromASCIIString(deviceInfoAscii.asCharArray());

                // Split the line to elements.
                QStringList lineAsList = acGTStringToQString(lineConverted).split(",");

                // Check if there are none zero items in the line at all, if not do not insert it to the table (probably a CPU or a buggy device)
                bool emptyList = true;
                int numItemsInList = lineAsList.count();

                for (int nItem = 1; nItem < numItemsInList; nItem++)
                {
                    if (lineAsList[nItem] != "0")
                    {
                        emptyList = false;
                        break;
                    }
                }

                if (emptyList)
                {
                    continue;
                }

                // Find the family from the device name
                QString familyName;
                QString deviceInfoAsQstr = acGTStringToQString(deviceInfo);
                QString deviceNameAsQstr = acGTStringToQString(deviceName);
                rc = kaFindFamilyName(deviceNameAsQstr, familyName);
                GT_IF_WITH_ASSERT(rc)
                {
                    // If there is no special handling for this device (and family) add it to the main table:
                    if (!handleDeviceData(deviceNameAsQstr, familyName, deviceInfoAsQstr))
                    {
                        int columnToEnterDevice = -1;

                        // If the family exists add the device at the end of the family, if not create a new family:
                        if (m_familyNameColumnMap.find(familyName) == m_familyNameColumnMap.end())
                        {
                            // Create a new column for the device family:
                            m_pAnalysisTable->insertColumn(numDevices + 1);
                            columnToEnterDevice = numDevices + 1;
                            m_familyNameColumnMap[familyName] = columnToEnterDevice;
                            addItem(m_pAnalysisTable, 0, columnToEnterDevice, familyName);
                        }
                        else
                        {
                            // Add the device at the end of the family:
                            columnToEnterDevice = m_familyNameColumnMap[familyName] + 1;
                            m_pAnalysisTable->insertColumn(columnToEnterDevice);

                            // advance all family start columns after the current family:
                            gtMap<QString, int>::iterator mapIterator;

                            for (mapIterator = m_familyNameColumnMap.begin(); mapIterator != m_familyNameColumnMap.end(); mapIterator++)
                            {
                                if ((*mapIterator).second >= columnToEnterDevice)
                                {
                                    (*mapIterator).second = (*mapIterator).second + 1;
                                }
                            }

                            m_familyNameColumnMap[familyName] = columnToEnterDevice;
                            addItem(m_pAnalysisTable, 0, columnToEnterDevice, "");
                        }

                        numDevices++;
                        // insert device name:
                        // Row 0 is for the header line
                        QString deviceNameStr = deviceNameAsQstr;

                        // Do not enter the device name in two consecutive columns:
                        if (deviceNameStr == lastDeviceInserted)
                        {
                            deviceNameStr = "";
                        }
                        else
                        {
                            lastDeviceInserted = deviceNameStr;
                        }

                        addItem(m_pAnalysisTable, 1, columnToEnterDevice, deviceNameStr);

                        // Insert the list to the table
                        int numReadings = lineAsList.count();

                        for (int nRead = 1; nRead < numReadings; nRead++)
                        {
                            QString text = lineAsList[nRead];
                            addItem(m_pAnalysisTable, nRead + 1, columnToEnterDevice, text);
                        }
                    }
                }
            }
        }

        fileToRead.close();
    }
}

// ---------------------------------------------------------------------------
// Name:        kaTableView::addItem
// Description: Add an item to the table with the right color
// Author:      Gilad Yarnitzky
// Date:        24/8/2013
// ---------------------------------------------------------------------------
void kaTableView::addItem(acListCtrl* pTable, int row, int column, QString text, QString toolTip)
{
    // if the text is of the value -1 that change it to N/A
    if (text == KA_STR_CAL_NA_Value_64)
    {
        text = "N/A";
    }

    QTableWidgetItem* pItemInserted = new QTableWidgetItem(text);


    if (!toolTip.isEmpty())
    {
        pItemInserted->setToolTip(toolTip);
    }

    // Set the item flags:
    Qt::ItemFlags flags = pItemInserted->flags() & (~Qt::ItemIsEditable);
    pItemInserted->setFlags(flags);

    GT_IF_WITH_ASSERT(NULL != pTable)
    {
        pTable->setItem(row, column, pItemInserted);
    }
}

// ---------------------------------------------------------------------------
bool kaTableView::updateView()
{
    bool retVal = false;

    // Delete old columns (need to keep column 0 and headers)
    int numColumns = m_pAnalysisTable->columnCount();

    for (int nColumn = 1; nColumn < numColumns; nColumn++)
    {
        m_pAnalysisTable->removeColumn(1);
    }

    if (m_filePath.exists())
    {
        readDataFile();
        retVal = true;
    }

    // Update the table background colors:
    updateBGColors();

    return retVal;
}

// ---------------------------------------------------------------------------
QColor kaTableView::findColorForCell(int row, int column)
{
    QColor retVal = m_a1Color;
    /// The colors of the table should be as follows:
    /// A1 (Family)  | B1 (Family1)                                                                  | A1 (Family2)                                                                                      | B1 (Family3)
    /// A1 (Device)  | B1 (Bonaire 3 cells) | B2 (Casper 3 cells) | (...all the devices in Family1)  | A1 (Bonaire 3 cells) | A2 (Casper 3 cells) | A1 (Hawaii 3 cells) | .(...all the devices in Family2)


    QList<int> sortedFamilyCellsIndices;
    gtMap<QString, int>::iterator mapIterator = m_familyNameColumnMap.begin();

    for (; mapIterator != m_familyNameColumnMap.end(); mapIterator++)
    {
        sortedFamilyCellsIndices << (*mapIterator).second;
    }

    qSort(sortedFamilyCellsIndices);

    // The first column is always a1:
    if (column == 0)
    {
        /* if (row <= m_amountOfHeaderRows)
         {
         retVal = m_a1Color;
         }
         else
         {*/
        retVal = Qt::white;
        //}
    }

    else
    {
        int familyIndex = 0;
        int familyColumnIndex = 0;

        foreach (int currentFamilyColumnIndex, sortedFamilyCellsIndices)
        {
            familyIndex++;

            if (currentFamilyColumnIndex >= column)
            {
                familyColumnIndex = currentFamilyColumnIndex;
                break;
            }
        }

        if (row == 0)
        {
            retVal = (familyIndex % 2 == 0) ? m_a1Color : m_b1Color;
        }

        // else  if (row <= m_amountOfHeaderRows)
        else
        {
            bool evenFamily = (familyIndex % 2 == 0);
            int deviceId = column - familyColumnIndex;

            if (m_amountOfColsPerDevice > 1)
            {
                if ((deviceId / m_amountOfColsPerDevice % 2 == 0) || (deviceId / m_amountOfColsPerDevice % 2 == 2))
                {
                    retVal = evenFamily ? m_a1Color : m_b2Color;
                }
                else
                {
                    retVal = evenFamily ? m_a2Color : m_b1Color;
                }
            }
            else
            {
                retVal = evenFamily ? m_a1Color : m_b1Color;
            }
        }

        // ENH436741 Add the size of kernel binary to the statistics table, red font if oversize
        if (row == m_CodeLenIndex)
        {
            QString text;
            bool rc = m_pAnalysisTable->getItemText(row, column, text);
            GT_ASSERT(rc);
            int iCodeLen = text.toInt();

            if (iCodeLen > KA_ANALYSIS_INSTRUCTION_CHACHE_SIZE)
            {
                retVal = m_RedColor;
            }
        } // want to see if scratch registers > 0 than color it red
        else if ((row == m_ScratchRegsIndex) && (m_pTableInformationCaption->text() == KA_STR_statisticsTableInfo))
        {
            QString text;
            bool rc = m_pAnalysisTable->getItemText(row, column, text);
            GT_ASSERT(rc);
            int scratchRegs = text.toInt();

            if (scratchRegs > 0)
            {
                retVal = m_RedColor;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void kaTableView::updateBGColors()
{

    QList<int> sortedFamilyCellsIndices;
    gtMap<QString, int>::iterator mapIterator = m_familyNameColumnMap.begin();

    for (; mapIterator != m_familyNameColumnMap.end(); mapIterator++)
    {
        sortedFamilyCellsIndices << (*mapIterator).second;
    }

    qSort(sortedFamilyCellsIndices);

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pAnalysisTable != NULL)
    {
        for (int row = 0; row < m_pAnalysisTable->rowCount(); row++)
        {
            for (int col = 0; col < m_pAnalysisTable->columnCount(); col++)
            {
                // Find the color for this item:
                QColor bgColor = findColorForCell(row, col);

                // Get the table item:
                QTableWidgetItem* pItem = m_pAnalysisTable->item(row, col);

                if (pItem != NULL)
                {
                    // Set the background color:
                    pItem->setData(Qt::BackgroundColorRole, bgColor);

                    if (col > 0)
                    {
                        pItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
                    }
                }
            }
        }

        // Go through the cells and set their span:
        int nextColForSpan = 1;

        foreach (int currentFamilyColumnIndex, sortedFamilyCellsIndices)
        {
            int colForSpan = nextColForSpan;
            int colcountForFamily = currentFamilyColumnIndex - nextColForSpan + 1;

            if (colcountForFamily > 1)
            {
                m_pAnalysisTable->setSpan(0, colForSpan, 1, colcountForFamily);

                if (m_amountOfColsPerDevice > 1)
                {
                    for (int col = colForSpan; col < colForSpan + colcountForFamily; col = col + m_amountOfColsPerDevice)
                    {
                        m_pAnalysisTable->setSpan(1, col, 1, m_amountOfColsPerDevice);
                    }
                }
            }

            nextColForSpan = colForSpan + colcountForFamily;
        }
    }
}

// ---------------------------------------------------------------------------
void kaTableView::resizeToContent(acListCtrl* pTable)
{
    GT_IF_WITH_ASSERT(NULL != pTable)
    {
        pTable->setFont(font());
        pTable->setIgnoreResize(true);

        for (int col = 0; col < pTable->columnCount(); col++)
        {
            int currentColWidth = 0;

            for (int row = 0; row < pTable->rowCount(); row++)
            {
                QString text;
                QTableWidgetItem* pItem = pTable->item(row, col);

                if (NULL != pItem)
                {
                    text = pItem->text();
                    QRect rect = pTable->fontMetrics().boundingRect(text);
                    // add the width of the icon if it exists assume the icon will be square and the same height of the rect + some space:
                    bool hasIcon = !pItem->icon().isNull();
                    currentColWidth = qMax(rect.width() + (hasIcon ? rect.height() + 4 : 0) + 10, currentColWidth);
                }
            }

            // Make sure that the minimum width is 50px:
            currentColWidth = qMax(currentColWidth, 50);

            pTable->setColumnWidth(col, currentColWidth);
        }
    }
}

// ---------------------------------------------------------------------------
void kaTableView::onUpdateEdit_Copy(bool& isEnabled)
{
    isEnabled = false;

    if (m_pAnalysisTable != NULL)
    {
        m_pAnalysisTable->onUpdateEditCopy(isEnabled);
    }
}

// ---------------------------------------------------------------------------
void kaTableView::onUpdateEdit_SelectAll(bool& isEnabled)
{
    if (m_pAnalysisTable != NULL)
    {
        m_pAnalysisTable->onUpdateEditSelectAll(isEnabled);
    }
}

// ---------------------------------------------------------------------------
void kaTableView::onUpdateEdit_Find(bool& isEnabled)
{
    isEnabled = false;

    if (m_pAnalysisTable != NULL)
    {
        m_pAnalysisTable->onUpdateEditFind(isEnabled);
    }
}

// ---------------------------------------------------------------------------
void kaTableView::onUpdateEdit_FindNext(bool& isEnabled)
{
    isEnabled = false;

    if (m_pAnalysisTable != NULL)
    {
        m_pAnalysisTable->onUpdateEditFindNext(isEnabled);
    }
}

// ---------------------------------------------------------------------------
void kaTableView::onEdit_Copy()
{
    if (m_pAnalysisTable != NULL)
    {
        m_pAnalysisTable->onEditCopy();
    }
}

// ---------------------------------------------------------------------------
void kaTableView::onEdit_SelectAll()
{
    if (m_pAnalysisTable != NULL)
    {
        m_pAnalysisTable->onEditSelectAll();
    }
}

// ---------------------------------------------------------------------------
void kaTableView::onEdit_Find()
{
    if (m_pAnalysisTable != NULL)
    {
        m_pAnalysisTable->onFindClick();
    }
}

// ---------------------------------------------------------------------------
void kaTableView::onEdit_FindNext()
{
    GT_IF_WITH_ASSERT(m_pAnalysisTable != NULL)
    {
        m_pAnalysisTable->onFindNext();
    }
}
