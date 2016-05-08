//------------------------------ gpConnectDialog.cpp ------------------------------

#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <qtIgnoreCompilerWarnings.h>

// Qt:
#include <QDomDocument>

// Infra:
#include <AMDTGraphicsServerInterface/Include/AMDTGraphicsServerInterface.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acProgressAnimationWidget.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>

// Local:
#include <AMDTGpuProfiling/gpConnectDialog.h>
#include <AMDTGpuProfiling/ProfileManager.h>
#include <AMDTGpuProfiling/gpExecutionMode.h>

#define MONITORING_SIZE 48
#define DIALOG_WIDTH 600
#define COLUMN_INDEX_WIDTH 20
#define COLUMN_APP_WIDTH 100
#define TIMER_INTERVAL_MS 500
#define FORCE_CONNECT_TIMER_CHECK 100
#define FORCE_CONNECT_NUM_CHECKS 100

gpConnectDialog::gpConnectDialog(GraphicsServerCommunication* pGraphicsServerCommunication, QWidget* pParent, bool forceAutoConnect) :
    acDialog(pParent),
    m_pConnectionsTable(nullptr),
    m_pAutomaticCheckBox(nullptr),
    m_pOptionsComboBox(nullptr),
    m_pOptionsEdit(nullptr),
    m_pAnimationWidget(nullptr),
    m_pGraphicsServerCommunication(pGraphicsServerCommunication),
    m_timePassed(0),
    m_pidToConnectTo(""),
    m_apiToConnectTo(GPU_STR_TraceViewDX12),
    m_processName(""),
    m_processNumber("1"),
    m_validator(1, 100, this),
    m_forceAutoConnect(forceAutoConnect),
    m_oneProcessAlive(false)
{
    // Get the CodeXL project type title as string:
    QString title(GPU_STR_connectionDialogTitle);

    // Set the title:
    setWindowTitle(title);

    // init the view layout
    InitLayout();

    bool rc = connect(&m_sampleTimer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));
    GT_ASSERT(rc);

    Qt::WindowFlags flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);
}

gpConnectDialog::~gpConnectDialog()
{

}

void gpConnectDialog::InitLayout()
{
    setMinimumWidth(DIALOG_WIDTH);

    QVBoxLayout* pMainLayout = new QVBoxLayout(this);

    QLabel* pHeaderLabel = new QLabel(GPU_STR_connectionDialogHeader);
    pMainLayout->addWidget(pHeaderLabel);

    m_pConnectionsTable = new acListCtrl(this);

    QString headerString(GPU_STR_connectionDialogTableHeader);
    QStringList headerList = headerString.split(',');
    m_pConnectionsTable->initHeaders(headerList, false);
    // the # column set to a small width
    m_pConnectionsTable->setColumnWidth(eIndexColumn, COLUMN_INDEX_WIDTH);
    // the application name column resize to content
    m_pConnectionsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    pMainLayout->addWidget(m_pConnectionsTable);

    bool rc = connect(m_pConnectionsTable, SIGNAL(itemSelectionChanged()), this, SLOT(OnTableSelectionChanged()));
    GT_ASSERT(rc);

    // layout to hold the monitoring widget and text;
    QHBoxLayout* pMonitoringHLayout = new QHBoxLayout;
    m_pAnimationWidget = new acProgressAnimationWidget(this);
    m_pAnimationWidget->setMinimumSize(MONITORING_SIZE, MONITORING_SIZE);
    m_pAnimationWidget->setMaximumSize(MONITORING_SIZE, MONITORING_SIZE);
    m_pAnimationWidget->StartAnimation();
    QLabel* pMonitoringLabel = new QLabel(GPU_STR_connectionDialogMonitoring);
    pMonitoringHLayout->addWidget(m_pAnimationWidget);
    pMonitoringHLayout->addWidget(pMonitoringLabel);
    pMainLayout->addLayout(pMonitoringHLayout);

    QHBoxLayout* pAutoHLayout = new QHBoxLayout;
    m_pAutomaticCheckBox = new QCheckBox(GPU_STR_connectionDialogAutoCheckBox);
    m_pOptionsComboBox = new QComboBox(this);
    m_pOptionsEdit = new QLineEdit(this);
    QString defaultOptions(GPU_STR_connectionDialogComboOptionsDefault);
    QStringList defaultList = defaultOptions.split(',');
    m_pOptionsComboBox->addItems(defaultList);
    pAutoHLayout->addWidget(m_pAutomaticCheckBox);
    pAutoHLayout->addWidget(m_pOptionsComboBox);
    pAutoHLayout->addWidget(m_pOptionsEdit);
    pAutoHLayout->addStretch(1);

    pMainLayout->addLayout(pAutoHLayout);

    gtString filePathString(L"");
    gtString OKButtonCaption(GPU_STR_connectionDialogOKCaption);
    QHBoxLayout* pDefaultButtonsLayout = getBottomButtonLayout(false, filePathString, OKButtonCaption);

    pMainLayout->addLayout(pDefaultButtonsLayout);

    if (m_pOKButton != nullptr)
    {
        m_pOKButton->setEnabled(false);
    }

    // connect the accept button
    rc = connect(this, SIGNAL(accepted()), this, SLOT(OnAccept()));
    GT_ASSERT(rc);

    rc = connect(m_pOptionsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnConnectionSelected(int)));
    GT_ASSERT(rc);

    rc = connect(m_pOptionsEdit, SIGNAL(textEdited(const QString&)), this, SLOT(OnTextEdited(const QString&)));
    GT_ASSERT(rc);

    // set the data from the settings:
    gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
    GT_IF_WITH_ASSERT(pFrameAnalysisManager != nullptr)
    {
        gpProjectSettings& settings = pFrameAnalysisManager->ProjectSettings();

        if (settings.m_shouldConnectAutomatically == 1)
        {
            m_pAutomaticCheckBox->setChecked(true);
        }

        m_processName = settings.m_processName;
        m_processNumber = settings.m_processNumber;
        m_pOptionsComboBox->setCurrentIndex(settings.m_connection);

        switch (settings.m_connection)
        {
            case gpProjectSettings::egpProcessConnection:
                m_pOptionsEdit->setText(settings.m_processName);
                break;

            case gpProjectSettings::egpFirstDXInProcessConnection:
                m_pOptionsEdit->setText(settings.m_processNumber);
                break;

            case gpProjectSettings::egpFirstDX12Connection:
                m_pOptionsEdit->setHidden(true);
                break;

            default:
                break;
        }
    }

    setLayout(pMainLayout);
}

// connect will display the dialog or show progress dialog depending on the setting
bool gpConnectDialog::Connect()
{
    bool retVal = false;

    afApplicationCommands* pCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pCommands != nullptr)
    {
        m_sampleTimer.setInterval(TIMER_INTERVAL_MS);
        m_sampleTimer.start();

        if (!m_forceAutoConnect)
        {
            if (pCommands->showModal(this) == QDialog::Accepted)
            {
                retVal = true;
            }
        }
        else
        {
            // try and connect to the dx12 application without showing the dialog but since we do it by filling
            // the dialog controls we need to give the application the ability to process the events
            // start counting 10 seconds
            int counter = 0;

            do
            {
                qApp->processEvents(); // keep UI responsive
                osSleep(FORCE_CONNECT_TIMER_CHECK);
                counter++;
            }
            while (m_pidToConnectTo.isEmpty() && counter < FORCE_CONNECT_NUM_CHECKS);

            // if we found a PID then it is the same as connect accept by the user if the force connect mode
            if (!m_pidToConnectTo.isEmpty())
            {
                retVal = true;
            }
        }
    }

    return retVal;
}

void gpConnectDialog::onTimerTimeout()
{
    m_timePassed += TIMER_INTERVAL_MS;

    // Collect the data from the server and compare it to what we already have and see if things changed
    GT_IF_WITH_ASSERT(m_pGraphicsServerCommunication != nullptr && m_pConnectionsTable != nullptr)
    {
        gtASCIIString httpResponse;
        bool rc = m_pGraphicsServerCommunication->GetProcesses(httpResponse);

        if (rc)
        {
            QVector<gpConnectionData> connectDataVector;

            // parse the data and make a list of the currently received pid and API
            QDomDocument processesDoc;
            processesDoc.setContent(acGTASCIIStringToQString(httpResponse));
            QDomNodeList processes = processesDoc.elementsByTagName(GPU_STR_connectionDialogProcessNode);
            int numProcesses = processes.size();

            if (numProcesses > 0)
            {
                m_oneProcessAlive = true;
            }

            for (int nProcess = 0; nProcess < numProcesses; nProcess++)
            {
                QDomNode currentProcess = processes.item(nProcess);
                QDomElement pidElement = currentProcess.firstChildElement(GPU_STR_connectionDialogPIDNode);
                QDomElement nameElement = currentProcess.firstChildElement(GPU_STR_connectionDialogNameNode);
                QDomElement apiElement = currentProcess.firstChildElement(GPU_STR_connectionDialogAPINode);

                while (!apiElement.isNull())
                {
                    QString attachedStr = apiElement.attribute(GPU_STR_connectionDialogAttachedAttribute);

                    GT_IF_WITH_ASSERT(!pidElement.isNull() && !nameElement.isNull() && !apiElement.isNull() && !attachedStr.isEmpty())
                    {
                        gpConnectionData newConnection;
                        newConnection.mName = nameElement.text();
                        newConnection.mPID = pidElement.text();
                        newConnection.mAPI = apiElement.text();
                        newConnection.mTime = m_timePassed;
                        newConnection.mAttached = attachedStr;
                        connectDataVector.push_back(newConnection);
                    }

                    apiElement = currentProcess.nextSiblingElement(GPU_STR_connectionDialogAPINode);
                }
            }

            // pass through all the items in the vector and find it in the list ctrl (based on PID and API)
            // if it is not there add it in the end of the list
            // if it is there check if the attached status needs to be changed
            int numConnections = connectDataVector.size();

            for (int nConnection = 0; nConnection < numConnections; nConnection++)
            {
                int foundRow = -1;
                int numRows = m_pConnectionsTable->rowCount();

                for (int nRow = 0; nRow < numRows; nRow++)
                {
                    gtString pidStr;
                    gtString apiStr;
                    m_pConnectionsTable->getItemText(nRow, eProcIDColumn, pidStr);
                    m_pConnectionsTable->getItemText(nRow, eAPIColumn, apiStr);

                    if (connectDataVector[nConnection].mPID == acGTStringToQString(pidStr) && connectDataVector[nConnection].mAPI == acGTStringToQString(apiStr))
                    {
                        foundRow = nRow;
                        break;
                    }
                }

                if (foundRow != -1)
                {
                    // check if the attached status was changed
                    gtString itemText;
                    m_pConnectionsTable->getItemText(foundRow, eAttachedColumn, itemText);
                    QString stringToCheck(GPU_STR_connectionDialogStarted);

                    if (connectDataVector[nConnection].mAttached.compare("true", Qt::CaseInsensitive) == 0)
                    {
                        stringToCheck = GPU_STR_connectionDialogAttached;
                    }

                    if (stringToCheck != acGTStringToQString(itemText))
                    {
                        m_pConnectionsTable->setItemText(foundRow, eAttachedColumn, stringToCheck);
                    }
                }
                else
                {
                    // create a new item and add it in the end
                    QStringList newConnectionData;
                    newConnectionData += QString::number(numRows + 1);
                    newConnectionData += connectDataVector[nConnection].mName;
                    newConnectionData += connectDataVector[nConnection].mPID;
                    newConnectionData += connectDataVector[nConnection].mAPI;

                    if (connectDataVector[nConnection].mAttached.compare("true", Qt::CaseInsensitive) == 0)
                    {
                        newConnectionData += GPU_STR_connectionDialogAttached;
                    }
                    else
                    {
                        newConnectionData += GPU_STR_connectionDialogStarted;
                    }

                    newConnectionData += QString::number(connectDataVector[nConnection].mTime / 1000, 'f', 2);
                    m_pConnectionsTable->addRow(newConnectionData, nullptr);
                }
            }

            // remove rows of pid that now longer exists
            int numRows = m_pConnectionsTable->rowCount();

            for (int nRow = 0; nRow < numRows; nRow++)
            {
                int foundRow = -1;
                int numConnections = connectDataVector.size();

                for (int nConnection = 0; nConnection < numConnections; nConnection++)
                {
                    gtString pidStr;
                    gtString apiStr;
                    m_pConnectionsTable->getItemText(nRow, eProcIDColumn, pidStr);
                    m_pConnectionsTable->getItemText(nRow, eAPIColumn, apiStr);

                    if (connectDataVector[nConnection].mPID == acGTStringToQString(pidStr) && connectDataVector[nConnection].mAPI == acGTStringToQString(apiStr))
                    {
                        foundRow = nRow;
                        break;
                    }
                }

                if (-1 == foundRow)
                {
                    m_pConnectionsTable->removeRow(nRow);
                    numRows--;
                    nRow--;
                }
            }

            if (CheckAutoConnected(connectDataVector))
            {
                accept();
            }

            // if there was at least one process alive monitored and now there are none, it means the parent process was killed and all of it
            // child process and we should close the dialog with no connections
            if (0 == numProcesses && m_oneProcessAlive)
            {
                reject();
            }
        }
        else
        {
            // The message was not handled successfully by the server. This means the server is hanging or crashed
            reject();
        }
    }

    GT_IF_WITH_ASSERT(m_pAnimationWidget != nullptr)
    {
        // increment the animation widget:
        m_pAnimationWidget->Increment(20);
        m_pAnimationWidget->repaint();
    }
}

bool gpConnectDialog::CheckAutoConnected(QVector<gpConnectionData>& connections)
{
    bool retVal = false;

    gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
    GT_IF_WITH_ASSERT(pFrameAnalysisManager != nullptr)
    {
        gpProjectSettings& settings = pFrameAnalysisManager->ProjectSettings();
        int processNumberToConnect = settings.m_processNumber.toInt();

        if (settings.m_shouldConnectAutomatically == 1 || m_forceAutoConnect)
        {
            int numConnections = connections.size();
            int numConnectionsForProcess = 0;

            for (int nConnection = 0; nConnection < numConnections; nConnection++)
            {
                bool foundConnection = false;
                bool isDX12Connection = (connections[nConnection].mAPI.compare("DX12", Qt::CaseInsensitive) == 0 && connections[nConnection].mAttached.compare("true", Qt::CaseInsensitive) == 0);

                // first DX12 that is found
                if (settings.m_connection == gpProjectSettings::egpFirstDX12Connection || m_forceAutoConnect)
                {
                    if (isDX12Connection)
                    {
                        foundConnection = true;
                    }
                }
                else if (settings.m_connection == gpProjectSettings::egpFirstDXInProcessConnection)
                {
                    if ((connections[nConnection].mName == settings.m_processName) && (isDX12Connection))
                    {
                        foundConnection = true;
                    }
                }
                else if (settings.m_connection == gpProjectSettings::egpProcessConnection)
                {
                    numConnectionsForProcess++;

                    if (numConnectionsForProcess == processNumberToConnect)
                    {
                        if (isDX12Connection)
                        {
                            foundConnection = true;
                        }
                        else
                        {
                            QString warningMessage = QString(GPU_STR_connectionDialogWrongAPI).arg(processNumberToConnect).arg(connections[nConnection].mName);
                            acMessageBox::instance().warning(AF_STR_WarningA, warningMessage);
                            reject();
                        }
                    }
                }
                else
                {
                    GT_ASSERT(false);
                }

                if (foundConnection)
                {
                    m_pidToConnectTo = connections[nConnection].mPID;
                    m_apiToConnectTo = connections[nConnection].mAPI;
                    retVal = true;
                    accept();
                    break;
                }
            }
        }
    }

    return retVal;
}

void gpConnectDialog::OnAccept()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pConnectionsTable != nullptr)
    {
        // if no pid was selected try and get it from the list
        if (m_pidToConnectTo.isEmpty())
        {
            QList<QTableWidgetItem*> selectedItems = m_pConnectionsTable->selectedItems();

            if (selectedItems.size() == m_pConnectionsTable->columnCount())
            {
                int currentRow = m_pConnectionsTable->currentRow();
                gtString itemText;
                m_pConnectionsTable->getItemText(currentRow, eProcIDColumn, itemText);
                m_pidToConnectTo = acGTStringToQString(itemText);
                m_pConnectionsTable->getItemText(currentRow, eAPIColumn, itemText);
                m_apiToConnectTo = acGTStringToQString(itemText);
            }
        }

        // copy the controls to the settings:
        gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
        GT_IF_WITH_ASSERT(pFrameAnalysisManager != nullptr)
        {
            gpProjectSettings& settings = pFrameAnalysisManager->ProjectSettings();
            settings.m_shouldConnectAutomatically = m_pAutomaticCheckBox->isChecked() ? 1 : 0;
            settings.m_connection = (gpProjectSettings::eConnectionType)m_pOptionsComboBox->currentIndex();
            settings.m_processName = m_processName;
            settings.m_processNumber = m_processNumber;
        }
    }
}

void gpConnectDialog::OnConnectionSelected(int index)
{
    GT_IF_WITH_ASSERT(m_pOptionsEdit != nullptr)
    {
        m_pOptionsEdit->setHidden((gpProjectSettings::eConnectionType)index == gpProjectSettings::egpFirstDX12Connection);

        if ((gpProjectSettings::eConnectionType)index == gpProjectSettings::egpProcessConnection)
        {
            m_pOptionsEdit->setValidator(&m_validator);
            m_pOptionsEdit->setText(m_processNumber);
        }
        else if ((gpProjectSettings::eConnectionType)index == gpProjectSettings::egpFirstDXInProcessConnection)
        {
            m_pOptionsEdit->setValidator(nullptr);
            m_pOptionsEdit->setText(m_processName);
        }
    }
}

void gpConnectDialog::OnTextEdited(const QString& text)
{
    GT_IF_WITH_ASSERT(m_pAutomaticCheckBox != nullptr && m_pOptionsComboBox != nullptr && m_pOptionsEdit != nullptr)
    {
        if (m_pOptionsComboBox->currentIndex() == gpProjectSettings::egpProcessConnection)
        {
            m_processNumber = text;;
        }
        else if (m_pOptionsComboBox->currentIndex() == gpProjectSettings::egpFirstDXInProcessConnection)
        {
            m_processName = text;
        }
    }
}

void gpConnectDialog::OnTableSelectionChanged()
{
    GT_IF_WITH_ASSERT(m_pConnectionsTable != nullptr && m_pOKButton != nullptr)
    {
        QList<QTableWidgetItem*> selectedItems = m_pConnectionsTable->selectedItems();
        bool isSelected = (selectedItems.size() == m_pConnectionsTable->columnCount());
        bool isDX12 = false;

        if (isSelected && selectedItems.size() >= eAPIColumn)
        {
            QTableWidgetItem* pAPIItem = selectedItems[eAPIColumn];

            if (pAPIItem != nullptr)
            {
                if (pAPIItem->text().compare("DX12", Qt::CaseInsensitive) == 0)
                {
                    isDX12 = true;
                }
            }
        }

        m_pOKButton->setEnabled(isSelected && isDX12);
    }
}