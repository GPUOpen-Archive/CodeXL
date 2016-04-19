//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afSystemInformationDialog.cpp
///
//==================================================================================


// Qt:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>
#include <QTabWidget>

// infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTApplicationComponents/Include/acDefinitions.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <VersionInfo/VersionInfo.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afOpenCLDeviceInformationCollector.h>
#include <AMDTApplicationFramework/Include/commands/afSystemInformationCommand.h>
#include <AMDTApplicationFramework/Include/dialogs/afSystemInformationDialog.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// The size of the dialog's list:
#define AF_SYSTEM_INFORMATION_LISTS_WIDTH 720
#define GD_SYSTEM_INFORMATION_LISTS_HEIGHT 360
// Open cl device information collection thread run times
#define AF_THREAD_TOTAL_WAIT_TIME     20000
#define AF_THREAD_WAIT_INTERVAL       500


// ---------------------------------------------------------------------------
// Name:        afSystemInformationDialog::afSystemInformationDialog
// Description: Definition of the Help About Dialog.
// Author:      Avi Shapira
// Date:        5/11/2003
// ---------------------------------------------------------------------------
afSystemInformationDialog::afSystemInformationDialog(QWidget* parent, InformationTabs selectedTab)
    : QDialog(afMainAppWindow::instance()),
      _pNotebookInfo(nullptr), _pCurrentShownListCtrl(nullptr), _pNotebookDescription(nullptr), m_pSaveButton(nullptr),
      _pListCtrlSystem(nullptr), _systemInfoFirstTime(true), _systemInfoFirstTimeToFile(true),
      _pListCtrlDisplay(nullptr), _displayInfoFirstTime(true), _displayInfoFirstTimeToFile(true),
      _pListCtrlOpenGLRenderer(nullptr), _openGLRendererInfoFirstTime(true),

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
      // This pane is excluded from CodeXL Mac since CGL pixel formats are objects created and released by need and cannot, by consequence, be enumerated or recreated.
#else
      _pListCtrlPixelFormats(nullptr), _pixelFormatInfoFirstTime(true),
#endif

      _pListCtrlOpenGLExtensions(nullptr), _openGLExtInfoFirstTime(true),
      _pListCtrlOpenCLPlatforms(nullptr), _openCLPlatformsInfoFirstTime(true), _openCLPlatformsInfoFirstTimeToFile(true),
      _pListCtrlOpenCLDevices(nullptr), _openCLDevicesInfoFirstTime(true), _openCLDevicesInfoFirstTimeToFile(true),
      _selectedInformationTab(selectedTab), m_openCLDeviceInfoCollectorTimerId(0),
      m_OpenCLDeviceInfoCollectorThreadRunTime(0), m_ListCtrlOpenCLDevicesOnMsgMode(false),
      m_pOpenCLDeviceInfoCollectorThread(nullptr)
{
    GT_UNREFERENCED_PARAMETER(parent);

    // Get the product title:
    QString title = afGlobalVariablesManager::ProductNameA();

    // Add the specific dialog caption
    title.append(AF_STR_SpaceA);
    title.append(AF_STR_SystemInformationTitle);

    // Set the title:
    this->setWindowTitle(title);

    // Set window flags (minimize / maximize / close buttons):
    Qt::WindowFlags flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);

    // Set the dialog icon:
    afLoadTitleBarIcon(this);

    // Create and layout the dialog items:
    setDialogLayout();

    // Perform an on notebook change event:
    onNotebookChangePage(_selectedInformationTab);
}

// ---------------------------------------------------------------------------
afSystemInformationDialog::~afSystemInformationDialog()
{
    // If timer is set, stop it
    if (m_openCLDeviceInfoCollectorTimerId > 0)
    {
        killTimer(m_openCLDeviceInfoCollectorTimerId);
    }

    // if thread is running stop & delete it
    if (nullptr != m_pOpenCLDeviceInfoCollectorThread)
    {
        m_pOpenCLDeviceInfoCollectorThread->StopCollectingInfo();
        delete m_pOpenCLDeviceInfoCollectorThread;
        m_pOpenCLDeviceInfoCollectorThread = nullptr;
    }
}

// ---------------------------------------------------------------------------
// Name:        afSystemInformationDialog::onNotebookChangePage
// Description: The main function that handle the page changes
// Author:      Yaki Tebeka
// Date:        15/3/2010
// ---------------------------------------------------------------------------
void afSystemInformationDialog::onNotebookChangePage(int pageIndex)
{
    gtList <gtString> getheringDataLine;
    getheringDataLine.push_back(acQStringToGTString(AF_STR_CollectingOpenCLDevicesInfo));
    getheringDataLine.push_back(L"");

    switch (pageIndex)
    {
        case SYS_INFO_SYSTEM:
        {
            // If needed, fill the system information into the list control:
            if (_systemInfoFirstTime)
            {
                afSystemInformationCommand sysInfoCmd;
                bool rcSysInfo = sysInfoCmd.collectSystemInformation(_systemInfoData);
                GT_IF_WITH_ASSERT(rcSysInfo)
                {
                    fillSysInfoDataIntoListCtrl(_systemInfoData, _pListCtrlSystem);
                    _systemInfoFirstTime = false;
                }
            }

            // Set the system information list control as the displayed list control:
            _pNotebookDescription->setText(AF_STR_DescriptionSystem);
            _pCurrentShownListCtrl = _pListCtrlSystem;
        }
        break;

        case SYS_INFO_DISPLAY:
        {
            // If needed, fill the display information into the list control:
            if (_displayInfoFirstTime)
            {
                afSystemInformationCommand sysInfoCmd;
                bool rcDisplayInfo = sysInfoCmd.collectDisplayInformation(_displayInfoData);
                GT_IF_WITH_ASSERT(rcDisplayInfo)
                {
                    fillSysInfoDataIntoListCtrl(_displayInfoData, _pListCtrlDisplay);
                    _displayInfoFirstTime = false;
                }
            }

            // Set the display information list control as the displayed list control:
            _pNotebookDescription->setText(AF_STR_DescriptionDisplay);
            _pCurrentShownListCtrl = _pListCtrlDisplay;
        }
        break;


        case SYS_INFO_GRAPHIC_CARD:
        {
            // If needed, fill the graphic card information into the list control:
            if (_openGLRendererInfoFirstTime)
            {
                afSystemInformationCommand sysInfoCmd;
                _openGLRendererInfoData = sysInfoCmd.GetGraphicCardData();
                _openGLRendererInfoFirstTime = false;

                if (_openGLRendererInfoData.size() == 0)
                {
                    _openGLRendererInfoFirstTime = true;
                    _openGLRendererInfoData.push_back(getheringDataLine);
                }

                fillSysInfoDataIntoListCtrl(_openGLRendererInfoData, _pListCtrlOpenGLRenderer);
            }

            // Set the graphic card list control as the displayed list control:
            _pNotebookDescription->setText(AF_STR_DescriptionGraphicCard);
            _pCurrentShownListCtrl = _pListCtrlOpenGLRenderer;
        }
        break;

            // All platforms except Mac:
            // Note: this pane is excluded from CodeXL Mac since CGL pixel formats are objects
            // created and released by need and cannot, by consequence, be enumerated or recreated.
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

        case SYS_INFO_PIXEL_FORMAT:
        {
            // If needed, fill the pixel formats information into the list control:
            if (_pixelFormatInfoFirstTime)
            {
                afSystemInformationCommand sysInfoCmd;
                _pixelFormatInfoData = sysInfoCmd.GetPixelData();
                _pixelFormatInfoFirstTime = false;

                if (_pixelFormatInfoData.size() == 0)
                {
                    _pixelFormatInfoFirstTime = true;
                    _pixelFormatInfoData.push_back(getheringDataLine);
                }

                fillSysInfoDataIntoListCtrl(_pixelFormatInfoData, _pListCtrlPixelFormats);
            }

            // Set the pixel format information list control as the displayed list control:
            _pNotebookDescription->setText(AF_STR_DescriptionPixelFormats);
            _pCurrentShownListCtrl = _pListCtrlPixelFormats;
        }
        break;

#endif // All platforms except Mac


        case SYS_INFO_OPENGL_EXTENSION:
        {
            // If needed, fill the OpenGL extensions information into the list control:
            if (_openGLExtInfoFirstTime)
            {
                afSystemInformationCommand sysInfoCmd;
                _openGLExtInfoData = sysInfoCmd.GetOpenGLExtData();
                _openGLExtInfoFirstTime = false;

                if (_openGLExtInfoData.size() == 0)
                {
                    _openGLExtInfoFirstTime = true;
                    _openGLExtInfoData.push_back(getheringDataLine);
                }

                fillSysInfoDataIntoListCtrl(_openGLExtInfoData, _pListCtrlOpenGLExtensions);
            }

            // Set the OpenGL extensions information list control as the displayed list control:
            _pNotebookDescription->setText(AF_STR_DescriptionOpenGLExtensions);
            _pCurrentShownListCtrl = _pListCtrlOpenGLExtensions;
        }
        break;

        case SYS_INFO_OPENCL_PLATFORMS:
        {
            // If needed, fill the OpenCL platforms information into the list control:
            if (_openCLPlatformsInfoFirstTime)
            {
                afSystemInformationCommand sysInfoCmd;
                bool rcOCLPlatform = sysInfoCmd.collectOpenCLPlatformsInformation(_openCLPlatformsInfoData, true);

                // Do not assert for machine with no OpenCL installed:
                if (rcOCLPlatform)
                {
                    fillSysInfoDataIntoListCtrl(_openCLPlatformsInfoData, _pListCtrlOpenCLPlatforms);
                    _openCLPlatformsInfoFirstTime = false;
                    _pNotebookDescription->setText(AF_STR_DescriptionOpenCLPlatforms);
                }
                else
                {
                    displayErrorMessage(AF_STR_NoOpenCLPlatforms, _pListCtrlOpenCLPlatforms);
                }
            }

            // Set the OpenCL Platforms list control as the displayed list control:
            _pNotebookDescription->setText(AF_STR_DescriptionOpenCLPlatforms);
            _pCurrentShownListCtrl = _pListCtrlOpenCLPlatforms;

        }
        break;

        case SYS_INFO_OPENCL_DEVICES:
        {
            // If needed, fill the OpenCL devices information into the list control:
            if (_openCLDevicesInfoFirstTime)
            {
                // m_pOpenCLDeviceInfoCollectorThread should be nullptr, this is just for safety
                if (nullptr != m_pOpenCLDeviceInfoCollectorThread)
                {
                    delete m_pOpenCLDeviceInfoCollectorThread;
                }

                // Create a thread to collect data
                m_pOpenCLDeviceInfoCollectorThread = new afOpenCLDeviceInformationCollector();

                GT_IF_WITH_ASSERT(nullptr != _pListCtrlOpenCLDevices)
                {
                    GT_IF_WITH_ASSERT(m_pOpenCLDeviceInfoCollectorThread->StartCollectingInfo())
                    {
                        // Modify display to notify user
                        _pListCtrlOpenCLDevices->clearList();
                        _pListCtrlOpenCLDevices->setColumnCount(1);
                        _pListCtrlOpenCLDevices->horizontalHeader()->hide();
                        _pListCtrlOpenCLDevices->verticalHeader()->hide();
                        _pListCtrlOpenCLDevices->addRow(AF_STR_CollectingOpenCLDevicesInfo);
                        m_ListCtrlOpenCLDevicesOnMsgMode = true;
                        // Start a timer, to allow checking if the thread is done
                        m_openCLDeviceInfoCollectorTimerId = startTimer(AF_THREAD_WAIT_INTERVAL);
                    }
                }
            }

            // Set the OpenCL devices list control as the displayed list control:
            _pNotebookDescription->setText(AF_STR_DescriptionOpenCLDevices);
            _pCurrentShownListCtrl = _pListCtrlOpenCLDevices;
        }
        break;

        default:
        {
            // Unknown pane page:
            GT_ASSERT(false);
        }
        break;
    }
}

// ---------------------------------------------------------------------------
void afSystemInformationDialog::timerEvent(QTimerEvent* event)
{
    bool stopThread = false;
    m_OpenCLDeviceInfoCollectorThreadRunTime += AF_THREAD_WAIT_INTERVAL;
    GT_UNREFERENCED_PARAMETER(event);

    // If past the max waiting time, stop timer to prevent it from going off in the mean while
    if (m_OpenCLDeviceInfoCollectorThreadRunTime > AF_THREAD_TOTAL_WAIT_TIME)
    {
        StopOpenCLDeviceInformationTimer();
        stopThread = true;
    }

    // timer should be set only when there's a thread running
    GT_IF_WITH_ASSERT(nullptr !=  m_pOpenCLDeviceInfoCollectorThread)
    {
        // if thread is running past the time allowed, stop it
        if (stopThread)
        {
            m_pOpenCLDeviceInfoCollectorThread->StopCollectingInfo();
        }

        // If thread is not active, might have finish before the timer, handle the thread
        if (!m_pOpenCLDeviceInfoCollectorThread->IsActive())
        {
            bool rcGetInfo = m_pOpenCLDeviceInfoCollectorThread->GetOpenCLDeviceInformation(_openCLDevicesInfoData);
            delete m_pOpenCLDeviceInfoCollectorThread;
            m_pOpenCLDeviceInfoCollectorThread = nullptr;
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_INFO, L"rcGetInfo=%d, _openCLDevicesInfoData.size = %d", rcGetInfo, _openCLDevicesInfoData.size());

            GT_IF_WITH_ASSERT(rcGetInfo)
            {
                // Collecting data had no errors, if list is empty its because there is nothing to display
                LoadOpenCLDeviceInformation(AF_STR_NoOpenCLDevices);
            }
            else
            {
                // Error accrued while collecting data
                LoadOpenCLDeviceInformation(AF_STR_FailToCollectOpenCLDevicesInfo);
            }

            // In case thread finished by itself and wasn't stopped, still need to stop timer
            StopOpenCLDeviceInformationTimer();
        }
    }
}

// ---------------------------------------------------------------------------
void afSystemInformationDialog::StopOpenCLDeviceInformationTimer()
{
    if (m_openCLDeviceInfoCollectorTimerId > 0)
    {
        killTimer(m_openCLDeviceInfoCollectorTimerId);
        m_openCLDeviceInfoCollectorTimerId = 0;
        m_OpenCLDeviceInfoCollectorThreadRunTime = 0;
    }
}

// ---------------------------------------------------------------------------
void afSystemInformationDialog::LoadOpenCLDeviceInformation(QString errMsg)
{
    GT_IF_WITH_ASSERT(nullptr != _pListCtrlOpenCLDevices)
    {
        if (m_ListCtrlOpenCLDevicesOnMsgMode)
        {
            _pListCtrlOpenCLDevices->clearList();
            _pListCtrlOpenCLDevices->horizontalHeader()->show();
            _pListCtrlOpenCLDevices->verticalHeader()->show();
            _pListCtrlOpenCLDevices->setColumnCount(0);
            m_ListCtrlOpenCLDevicesOnMsgMode = false;
        }

        //Do not assert for machine with no OpenCL installed:
        if (_openCLDevicesInfoData.size() > 0)
        {
            fillSysInfoDataIntoListCtrl(_openCLDevicesInfoData, _pListCtrlOpenCLDevices, false, AF_STR_OpenCLDevicesTableMinColumnWidth);
            _openCLDevicesInfoFirstTime = false;
            _pNotebookDescription->setText(AF_STR_DescriptionOpenCLDevices);
        }
        else
        {
            displayErrorMessage(errMsg, _pListCtrlOpenCLDevices);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afSystemInformationDialog::getItemAsText
// Description:
// Arguments:   int columnIndex
//              int lineIndex
// Author:      Avi Shapira
// Date:        28/12/2003
// ---------------------------------------------------------------------------
gtString afSystemInformationDialog::getItemAsText(int lineIndex, int columnIndex)
{
    gtString retVal;
    GT_IF_WITH_ASSERT(_pCurrentShownListCtrl != nullptr)
    {
        bool rc = _pCurrentShownListCtrl->getItemText(lineIndex, columnIndex, retVal);
        GT_ASSERT(rc);
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationDialog::getItemAsLong
// Description:
// Arguments:   int columnIndex
//              int lineIndex
// Author:      Avi Shapira
// Date:        28/12/2003
// ---------------------------------------------------------------------------
long afSystemInformationDialog::getItemAsLong(int lineIndex, int columnIndex)
{
    long retVal = 0;
    GT_IF_WITH_ASSERT(_pCurrentShownListCtrl != nullptr)
    {
        gtString text;
        bool rc = _pCurrentShownListCtrl->getItemText(lineIndex, columnIndex, text);
        GT_ASSERT(rc);

        rc = text.toLongNumber(retVal);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationDialog::replaceListItems
// Description:
// Arguments:   int line1Index
//              int line2Index
// Author:      Avi Shapira
// Date:        28/12/2003
// ---------------------------------------------------------------------------
void afSystemInformationDialog::replaceListItems(int line1Index, int line2Index)
{
    // Sanity check
    GT_IF_WITH_ASSERT(_pCurrentShownListCtrl != nullptr)
    {
        int amountOfColumns = _pCurrentShownListCtrl->columnCount();

        for (int i = 0; i < amountOfColumns; i++)
        {
            QString str1, str2;
            bool rc1 = _pCurrentShownListCtrl->getItemText(line1Index, i, str1);
            bool rc2 = _pCurrentShownListCtrl->getItemText(line2Index, i, str1);
            GT_IF_WITH_ASSERT(rc1 && rc2)
            {
                _pCurrentShownListCtrl->setItemText(line2Index, i, str1);
                _pCurrentShownListCtrl->setItemText(line1Index, i, str2);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationDialog::fillSysInfoDataIntoListCtrl
// Description: Fills system information data into a given notebook's list control.
// Author:      Yaki Tebeka
// Date:        14/3/2010
// ---------------------------------------------------------------------------
void afSystemInformationDialog::fillSysInfoDataIntoListCtrl(const gtList< gtList<gtString> >& sysInfoData, acListCtrl* pListCtrl, bool columnAutoResize, int minColumnWidth)
{
    GT_UNREFERENCED_PARAMETER(columnAutoResize);
    bool enableHorizontalScrollbar = false;

    // Sanity check
    GT_IF_WITH_ASSERT(pListCtrl != nullptr)
    {
        // Clear the list control:
        pListCtrl->clear();

        // Initialize the headers (get the first line, and add it's values to the columns titles):
        QStringList headers;
        gtList< gtList <gtString> >::const_iterator linesIterator = sysInfoData.begin();

        if (linesIterator != sysInfoData.end())
        {
            // Get the current line:
            const gtList <gtString>& currentLine = *linesIterator++;

            // Iterate the current line columns:
            int columnIndex = 0;
            gtList <gtString>::const_iterator columnsIterator = currentLine.begin();

            while (columnsIterator != currentLine.end())
            {
                const gtString& currentString = *columnsIterator;

                // Add the columnIndex data to the pListCtrl
                headers << acGTStringToQString(currentString);

                // Increment iterators:
                columnsIterator++;
                columnIndex++;
            }
        }

        if (pListCtrl->columnCount() < headers.size())
        {
            // Initialize the table headers:
            pListCtrl->initHeaders(headers, false);
        }

        // Remove horizontal scroll bar:
        pListCtrl->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        // Iterate the content lines:
        while (linesIterator != sysInfoData.end())
        {
            // Get the current line:
            const gtList <gtString>& currentLine = *linesIterator++;

            // Iterate the current line columns:
            int columnIndex = 0;
            QStringList currentRow;
            gtList <gtString>::const_iterator columnsIterator = currentLine.begin();

            while (columnsIterator != currentLine.end())
            {
                gtString currentString = *columnsIterator;
                currentRow << acGTStringToQString(currentString);

                // Increment iterators:
                columnsIterator++;
                columnIndex++;
            }

            // Add this row to the list control:
            pListCtrl->addRow(currentRow, nullptr);
        }

        // Handle the column widths:
        if (pListCtrl->columnCount() > 1)
        {
            // We set the list to invisible to make the width adjustment global and not just to visible elements:
            pListCtrl->setVisible(false);
            int cols = pListCtrl->columnCount();
            // Set the first column to be full size:
            pListCtrl->resizeColumnToContents(0);

            // Calculate the total width for the other columns (all except the first)
            int width = pListCtrl->width() - pListCtrl->columnWidth(0) - pListCtrl->verticalScrollBar()->width() - 5;
            // Distribute the remaining space equally:
            int colWidth = (int)(width / (cols - 1));

            if (colWidth < minColumnWidth)
            {
                colWidth = minColumnWidth;

                if (colWidth * (cols - 1) > width)
                {
                    // Return horizontal scroll bar:
                    enableHorizontalScrollbar = true;
                }
            }

            GT_IF_WITH_ASSERT(width > 0)
            {
                // Set the width of all columns equally:
                for (int i = 1; i < cols - 1; ++i)
                {
                    GT_IF_WITH_ASSERT(colWidth > 0)
                    {
                        pListCtrl->setColumnWidth(i, colWidth);
                        width -= colWidth;
                    }
                }

                // Set the remaining width to the last column:
                pListCtrl->setColumnWidth(cols - 1, max(width, colWidth));
            }

            if (enableHorizontalScrollbar)
            {
                pListCtrl->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            }

            // Restore control:
            pListCtrl->setVisible(true);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationDialog::displayErrorMessage
// Description: Displays an input error message in a given notebook's list control.
// Author:      Yaki Tebeka
// Date:        4/8/2010
// ---------------------------------------------------------------------------
void afSystemInformationDialog::displayErrorMessage(const QString& errorMsg, acListCtrl* pListCtrl)
{
    GT_IF_WITH_ASSERT((pListCtrl != nullptr) && (_pNotebookDescription != nullptr))
    {
        pListCtrl->clearContents();
        _pNotebookDescription->setText(errorMsg);
    }
}


// ---------------------------------------------------------------------------
// Name:        sortByInteger
// Description:
// Arguments:   Item a[]
//              int l
//              int r
// Author:      Avi Shapira
// Date:        28/12/2003
// ---------------------------------------------------------------------------
void afSystemInformationDialog::sortByInteger(int l, int r, int columnIndex)
{
    int i = l - 1, j = r;
    long v = getItemAsLong(r, columnIndex);


    int rItemVal = getItemAsLong(r, columnIndex);
    int lItemVal = getItemAsLong(l, columnIndex);

    if (rItemVal <= lItemVal) { return; }

    for (;;)
    {
        while (getItemAsLong(++i, columnIndex) < v);

        while (v < getItemAsLong(--j, columnIndex))
        {
            if (j == l) { break; }
        }

        int iItemVal = getItemAsLong(i, columnIndex);
        int jItemVal = getItemAsLong(j, columnIndex);

        if (iItemVal <= jItemVal)
        {
            break;
        }

        replaceListItems(i, j);
    }

    replaceListItems(i, r);

    sortByInteger(l, i - 1, columnIndex);
    sortByInteger(i + 1, r, columnIndex);
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationDialog::setDialogLayout
// Description: Create the dialog items and lay them out into the frame
// Author:      Avi Shapira
// Date:        28/12/2003
// ---------------------------------------------------------------------------
void afSystemInformationDialog::setDialogLayout()
{
    // Main dialog notebook and sizer
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    unsigned int listWidth = acScalePixelSizeToDisplayDPI(AF_SYSTEM_INFORMATION_LISTS_WIDTH);
    unsigned int listHeight = acScalePixelSizeToDisplayDPI(GD_SYSTEM_INFORMATION_LISTS_HEIGHT);
    QSize listsSize(listWidth, listHeight);
    _pNotebookInfo = new QTabWidget;

    bool rc = connect(_pNotebookInfo, SIGNAL(currentChanged(int)), this, SLOT(onNotebookChangePage(int)));

    _pNotebookDescription = new QLabel;
    _pNotebookDescription->setWordWrap(true);
    // System pane
    _pListCtrlSystem = new acListCtrl(this);
    _pListCtrlSystem->resize(listsSize);
    _pNotebookInfo->addTab(_pListCtrlSystem, AF_STR_InformationSystem);

    // Display pane
    _pListCtrlDisplay = new acListCtrl(this);
    _pNotebookInfo->addTab(_pListCtrlDisplay, AF_STR_InformationDisplay);

    // Graphics Card pane
    _pListCtrlOpenGLRenderer = new acListCtrl(this);
    _pNotebookInfo->addTab(_pListCtrlOpenGLRenderer, AF_STR_InformationOpenGLRenderer);

    // Pixel Formats pane
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    // Note: this pane is excluded from CodeXL Mac since CGL pixel formats are objects
    // created and released by need and cannot, by consequence, be enumerated or recreated.
#else
    _pListCtrlPixelFormats = new acListCtrl(this);
    _pNotebookInfo->addTab(_pListCtrlPixelFormats, AF_STR_InformationPixelFormats);
#endif

    // OpenGL Extensions pane
    _pListCtrlOpenGLExtensions = new acListCtrl(this);
    _pNotebookInfo->addTab(_pListCtrlOpenGLExtensions, AF_STR_InformationOpenGLExtensions);

    // OpenCL Platforms pane
    _pListCtrlOpenCLPlatforms = new acListCtrl(this);
    _pNotebookInfo->addTab(_pListCtrlOpenCLPlatforms, AF_STR_InformationOpenCLPlatforms);

    // OpenCL Devices pane
    _pListCtrlOpenCLDevices = new acListCtrl(this);
    _pNotebookInfo->addTab(_pListCtrlOpenCLDevices, AF_STR_InformationOpenCLDevices);


    // Other dialog items and finalizing the layout:
    _pCurrentShownListCtrl = _pListCtrlSystem;
    pMainLayout->addWidget(_pNotebookInfo, 1);
    pMainLayout->addWidget(_pNotebookDescription, 0);

    QPushButton* pOkButton = new QPushButton(tr("OK"));
    m_pSaveButton = new QPushButton(tr("&Save"));

    // Add image banner widget:
    QDialogButtonBox* pButtonBox = new QDialogButtonBox();

    pButtonBox->addButton(pOkButton, QDialogButtonBox::AcceptRole);
    pButtonBox->addButton(m_pSaveButton, QDialogButtonBox::ActionRole);

    // Select the requested tab:
    _pNotebookInfo->setCurrentIndex(_selectedInformationTab);

    rc = connect(pButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
    GT_ASSERT(rc);
    rc = connect(pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButtonClick(QAbstractButton*)));
    GT_ASSERT(rc);

    pMainLayout->addWidget(pButtonBox, 0, Qt::AlignRight | Qt::AlignBottom);
    pMainLayout->setMargin(10);

    // dialog is re-sizable with minimum size limits
    setMinimumHeight(listHeight);
    setMinimumWidth(listWidth);

    setLayout(pMainLayout);
}


void afSystemInformationDialog::onButtonClick(QAbstractButton* pButton)
{
    if (pButton == m_pSaveButton)
    {
        onSysInfoSaveButton();
    }
}

// ---------------------------------------------------------------------------
// Name:        afSystemInformationDialog::onSysInfoSaveButton
// Description: Save the info into a file
// Author:      Yoni Rabin
// Date:        20/5/2012
// ---------------------------------------------------------------------------
void afSystemInformationDialog::onSysInfoSaveButton(/*QAbstractButton* pButton*/)
{
    _sysInfoSaveOutputString.makeEmpty();

    blockSignals(true);

    // Get the application commands instance:
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
    {
        // get a system information file path:
        osFilePath defaultFilePath(osFilePath::OS_USER_DOCUMENTS);
        defaultFilePath.setFileName(AF_STR_systemInformationFileSample);
        QString defaultPathStr = acGTStringToQString(defaultFilePath.asString());

        // prevent the user from closing this dialog before finishing with the file dialog:
        setEnabled(false);
        QString selectedFilePath = pApplicationCommands->ShowFileSelectionDialog(AF_STR_systemInformationFileTitle, defaultPathStr, AF_STR_systemInformationFileDetails, nullptr, true);
        setEnabled(true);

        // Return the focus to me:
        this->setFocus();
        this->raise();

        if (!selectedFilePath.isEmpty())
        {
            afSystemInformationCommand sysInfoCmd;

            _sysInfoSaveOutputString.append(L"////////////////////////////////////////////////////////////\n");
            _sysInfoSaveOutputString.append(L"// System\n");
            _sysInfoSaveOutputString.append(L"////////////////////////////////////////////////////////////\n");

            if (_systemInfoFirstTimeToFile)
            {
                sysInfoCmd.collectSystemInformation(_systemInfoDataFile);
                _systemInfoFirstTimeToFile = false;
            }

            addSystemInfoIntoOutputString(_systemInfoDataFile, 29);

            _sysInfoSaveOutputString.append(L"\n\n\n\n");
            _sysInfoSaveOutputString.append(L"////////////////////////////////////////////////////////////\n");
            _sysInfoSaveOutputString.append(L"// Display\n");
            _sysInfoSaveOutputString.append(L"////////////////////////////////////////////////////////////\n");

            if (_displayInfoFirstTimeToFile)
            {
                sysInfoCmd.collectDisplayInformation(_displayInfoDataFile);
                _displayInfoFirstTimeToFile = false;
            }

            addSystemInfoIntoOutputString(_displayInfoDataFile, 21);

            _sysInfoSaveOutputString.append(L"\n\n\n\n");
            _sysInfoSaveOutputString.append(L"////////////////////////////////////////////////////////////\n");
            _sysInfoSaveOutputString.append(L"// Graphic Card\n");
            _sysInfoSaveOutputString.append(L"////////////////////////////////////////////////////////////\n");

            if (_openGLRendererInfoFirstTime)
            {
                _openGLRendererInfoDataFile = sysInfoCmd.GetGraphicCardData();
                _openGLRendererInfoFirstTime = false;
            }

            addSystemInfoIntoOutputString(_openGLRendererInfoDataFile, 21);

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            {
                // Note: this pane is excluded from CodeXL Mac since CGL pixel formats are objects
                // created and released by need and cannot, by consequence, be enumerated or recreated.
            }
#else
            {
                _sysInfoSaveOutputString.append(L"\n\n\n\n");
                _sysInfoSaveOutputString.append(L"////////////////////////////////////////////////////////////\n");
                _sysInfoSaveOutputString.append(L"// Pixel Formats\n");
                _sysInfoSaveOutputString.append(L"////////////////////////////////////////////////////////////\n");

                if (_pixelFormatInfoFirstTime)
                {
                    _pixelFormatInfoDataFile = sysInfoCmd.GetPixelData();
                    _pixelFormatInfoFirstTime = false;
                }

                addSystemInfoIntoOutputString(_pixelFormatInfoDataFile, 10);
            }
#endif

            _sysInfoSaveOutputString.append(L"\n\n\n\n");
            _sysInfoSaveOutputString.append(L"////////////////////////////////////////////////////////////\n");
            _sysInfoSaveOutputString.append(L"// OpenGL Extensions\n");
            _sysInfoSaveOutputString.append(L"////////////////////////////////////////////////////////////\n");

            if (_openGLExtInfoFirstTime)
            {
                _openGLExtInfoDataFile = sysInfoCmd.GetOpenGLExtData();
                _openGLExtInfoFirstTime = false;
            }

            addSystemInfoIntoOutputString(_openGLExtInfoDataFile, 10);

            _sysInfoSaveOutputString.append(L"\n\n\n\n");
            _sysInfoSaveOutputString.append(L"////////////////////////////////////////////////////////////\n");
            _sysInfoSaveOutputString.append(L"// OpenCL Platforms\n");
            _sysInfoSaveOutputString.append(L"////////////////////////////////////////////////////////////\n");

            if (_openCLPlatformsInfoFirstTimeToFile)
            {
                sysInfoCmd.collectOpenCLPlatformsInformation(_openCLPlatformsInfoData, true);
                _openCLPlatformsInfoFirstTimeToFile = false;
            }

            addSystemInfoIntoOutputString(_openCLPlatformsInfoData, 10);

            _sysInfoSaveOutputString.append(L"\n\n\n\n");
            _sysInfoSaveOutputString.append(L"////////////////////////////////////////////////////////////\n");
            _sysInfoSaveOutputString.append(L"// OpenCL Devices\n");
            _sysInfoSaveOutputString.append(L"////////////////////////////////////////////////////////////\n");

            if (_openCLDevicesInfoFirstTimeToFile)
            {
                // Create a thread to collect data
                m_pOpenCLDeviceInfoCollectorThread = new afOpenCLDeviceInformationCollector();
                GT_IF_WITH_ASSERT(m_pOpenCLDeviceInfoCollectorThread != nullptr && m_pOpenCLDeviceInfoCollectorThread->StartCollectingInfo())
                {
                    // set a timeout on the thread
                    osWaitForFlagToTurnOff(m_pOpenCLDeviceInfoCollectorThread->IsActive(), AF_THREAD_TOTAL_WAIT_TIME);

                    m_pOpenCLDeviceInfoCollectorThread->GetOpenCLDeviceInformation(_openCLDevicesInfoData);

                    delete m_pOpenCLDeviceInfoCollectorThread;
                    m_pOpenCLDeviceInfoCollectorThread = nullptr;
                }
                _openCLDevicesInfoFirstTimeToFile = false;
            }

            addSystemInfoIntoOutputString(_openCLDevicesInfoData, 33, true);

            // Save the output string into the output file:
            bool rc = saveSystemInfoFile(acQStringToGTString(selectedFilePath));

            if (!rc)
            {
                acMessageBox::instance().critical(AF_STR_ErrorA, AF_STR_ErrorMessageSaveSystemInformationFailed);
            }
        }
    }

    blockSignals(false);
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationDialog::onListCtrlKeyDown
// Description: Reads key input in the list ctrls. Note that this function
//              expects to be called from a (afSystemInformationDialog *) cast
//              of a pointer which is actually an (acListCtrl *)
// Arguments: eve - the key event which generated this call.
// Author:      Uri Shomroni
// Date:        27/5/2008
// ---------------------------------------------------------------------------
void afSystemInformationDialog::onListCtrlKeyDown(QKeyEvent* pKeyEvent)
{
    GT_UNREFERENCED_PARAMETER(pKeyEvent);

    GT_ASSERT_EX(false, L"implement key events");
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationDialog::addSystemInfoIntoOutputString
// Description: Inputs a list containing system information and adds it's information
//              into the output string (_sysInfoSaveOutputString)
// Author:      Avi Shapira
// Date:        7/4/2005
// ---------------------------------------------------------------------------
void afSystemInformationDialog::addSystemInfoIntoOutputString(gtList< gtList <gtString> > infoList, int fieldWidth, bool trimLines)
{
    // Iterate the lines:
    gtList< gtList <gtString> >::iterator linesIterator = infoList.begin();

    while (linesIterator != infoList.end())
    {
        // Get the current line:
        gtList <gtString>& currentLine = *linesIterator++;

        // Create the segmented strings from the list of strings
        gtVector < gtVector < gtString >> segmentedStringsVector;
        int largestNumberOfSegments = 0;

        gtList <gtString>::iterator columnsIterator = currentLine.begin();

        while (columnsIterator != currentLine.end())
        {
            gtVector<gtString> segmentedString;

            gtString& currentString = *columnsIterator;

            if (trimLines)
            {
                while (!currentString.isEmpty())
                {
                    gtString currentSegment;
                    currentString.getSubString(0, fieldWidth - 1, currentSegment);
                    segmentedString.push_back(currentSegment);
                    currentString.extruct(0, fieldWidth);
                }
            }
            else
            {
                segmentedString.push_back(currentString);
            }

            segmentedStringsVector.push_back(segmentedString);

            if ((int)segmentedString.size() > largestNumberOfSegments)
            {
                largestNumberOfSegments = segmentedString.size();
            }

            columnsIterator++;
        }

        // Iterate columns and in each column print the segment for the current line
        for (int nLine = 0; nLine < largestNumberOfSegments;  nLine++)
        {
            int numColumns = segmentedStringsVector.size();

            for (int nColumn = 0; nColumn < numColumns; nColumn++)
            {
                // Get the current string segment of the current column, this can be empty if the current string had less sections
                // so verify this
                gtString currentString;

                if ((int)segmentedStringsVector[nColumn].size() > nLine)
                {
                    currentString = segmentedStringsVector[nColumn][nLine];
                }

                // print the line using the field width
                // Set the string width format
                gtString stringFormat = L"%-";
                stringFormat.appendFormattedString(L"%d", fieldWidth);
                stringFormat.appendFormattedString(L"s");

                _sysInfoSaveOutputString.appendFormattedString(stringFormat.asCharArray(), currentString.asCharArray());
                _sysInfoSaveOutputString += AF_STR_Space;
            }

            _sysInfoSaveOutputString += AF_STR_NewLine;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        afSystemInformationDialog::saveSystemInfoFile
// Description: Save the system information file into the disk
// Author:      Avi Shapira
// Date:        7/4/2005
// ---------------------------------------------------------------------------
bool afSystemInformationDialog::saveSystemInfoFile(const osFilePath& filePath)
{
    bool retVal = false;
    osTime fileSavedDateAndTime;
    gtString fileSavedDate;
    gtString fileSavedTime;
    gtString fileHeader;

    osFile sysInfoFile;
    retVal = sysInfoFile.open(filePath, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

    if (retVal)
    {
        fileSavedDateAndTime.setFromCurrentTime();

        fileSavedDateAndTime.dateAsString(fileSavedDate, osTime::WINDOWS_STYLE, osTime::LOCAL);
        fileSavedDateAndTime.timeAsString(fileSavedTime, osTime::WINDOWS_STYLE, osTime::LOCAL);

        // Get the product version:
        osProductVersion appVersion;
        osGetApplicationVersion(appVersion);
        gtString productName(STRPRODUCTNAME);

        fileHeader.append(L"////////////////////////////////////////////////////////////");
        fileHeader.append(AF_STR_NewLine);
        fileHeader.append(L"// " AF_STR_System_Snapshot_Header AF_STR_NewLine);
        fileHeader.append(L"// Generation date: ") += fileSavedDate += AF_STR_NewLine;
        fileHeader.append(L"// Generation time: ") += fileSavedTime += AF_STR_NewLine;
        fileHeader.append(L"//" AF_STR_NewLine);
        fileHeader.append(L"// Generated by   : ") += productName += AF_STR_NewLine;
        fileHeader.append(L"// Version        : ") += appVersion.toString() += AF_STR_NewLine;
        fileHeader.append(L"// " AF_STR_AMD_DEVELOPER_WEBSITE_URL AF_STR_NewLine);
        fileHeader.append(L"////////////////////////////////////////////////////////////\n\n\n\n");

        sysInfoFile << fileHeader;
        sysInfoFile << _sysInfoSaveOutputString;
        sysInfoFile.close();
    }

    return retVal;
}
