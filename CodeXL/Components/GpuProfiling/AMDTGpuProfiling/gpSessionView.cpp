//------------------------------ gpSessionView.cpp ------------------------------

#include <qtIgnoreCompilerWarnings.h>

// Qt:
#include <QDomDocument>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtASCIIStringTokenizer.h>
#include <AMDTAPIClasses/Include/apProjectSettings.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acThumbnailView.h>

// Framework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afCSSSettings.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afTreeItemType.h>

#include <AMDTGraphicsServerInterface/Include/AMDTGraphicsServerInterface.h>

// Local:
#include <AMDTGpuProfiling/gpCountersSelectionDialog.h>
#include <AMDTGpuProfiling/gpSessionView.h>
#include <AMDTGpuProfiling/gpExecutionMode.h>
#include <AMDTGpuProfiling/gpUIManager.h>
#include <AMDTGpuProfiling/gpTreeHandler.h>
#include <AMDTGpuProfiling/gpViewsCreator.h>
#include <AMDTGpuProfiling/ProfileManager.h>

#define GP_IMAGE_SECTION_WIDTH 512
#define GP_SESSION_VIEW_BUTTON_SIZE 72

// ---------------------------------------------------------------------------
// Name:        gpSessionView
// Description: constructor
// Author:      Gilad Yarnitzky
// Date:        18/8/2013
// ---------------------------------------------------------------------------
gpSessionView::gpSessionView(QWidget* pParent, const osFilePath& sessionPath) : gpBaseSessionView(pParent),
    m_pSessionData(nullptr), m_pImageLabel(nullptr), m_pSimulateUserCaptureButton(nullptr) , m_pCaptureButton(nullptr), m_pCaptureButtonCPU(nullptr), m_pCaptureButtonGPU(nullptr),
    m_pStopButton(nullptr), m_pOpenTimelineButton(nullptr), m_pCurrentFrameDetailsLabel(nullptr), m_pCurrentFrameCaptionLabel(nullptr),
    m_pPropsHTMLLabel(nullptr), m_pSnapshotsThumbView(nullptr),
    m_isSessionRunning(false), m_isDisplayingCurrentFrame(false), m_lastCapturedFrameIndex(-1), m_sessionPath(sessionPath)
{
    // init the view layout
    InitLayout();

    QPalette p = palette();
    p.setColor(backgroundRole(), QColor::fromRgb(251, 251, 251));
    p.setColor(QPalette::Base, QColor::fromRgb(251, 251, 251));
    setAutoFillBackground(true);
    setPalette(p);
}

// ---------------------------------------------------------------------------
// Name:        gpSessionView::~gpSessionView
// Description: destructor
// Author:      Gilad Yarnitzky
// Date:        18/8/2013
// ---------------------------------------------------------------------------
gpSessionView::~gpSessionView()
{
    // Remove me from the list of session windows in the session view creator:
    gpViewsCreator::Instance()->OnWindowClose(this);

    m_pSimulateUserCaptureButton = m_pCaptureButton = m_pCaptureButtonCPU = m_pCaptureButtonGPU = m_pStopButton = m_pOpenTimelineButton = nullptr;
    m_pImageLabel = m_pCurrentFrameDetailsLabel = m_pCurrentFrameCaptionLabel = m_pPropsHTMLLabel = m_pCapturedFramesCaptionLabel = nullptr;
    m_pSnapshotsThumbView = nullptr;

}

void gpSessionView::InitLayout()
{
    gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
    gpProjectSettings& settings = pFrameAnalysisManager->ProjectSettings();

    // layout to hold the two main vboxlayouts;
    QHBoxLayout* pMainLayout = new QHBoxLayout;

    QVBoxLayout* pThumbLayout = new QVBoxLayout;
    QVBoxLayout* pImageLayout = new QVBoxLayout;

    // Create the image section
    m_pImageLabel = new QLabel;

    // This pixmap is set so that the label will demand the max size possible
    QPixmap placeHolderPixmap(GP_IMAGE_SECTION_WIDTH, GP_IMAGE_SECTION_WIDTH);
    m_pImageLabel->setPixmap(placeHolderPixmap);

    m_pImageLabel->setMaximumWidth(GP_IMAGE_SECTION_WIDTH);
    m_pImageLabel->setMaximumHeight(GP_IMAGE_SECTION_WIDTH);

    // Create the frame analysis section:
    m_pCurrentFrameCaptionLabel = new QLabel(GPU_STR_dashboard_MainImageCaptionRunning);
    m_pCurrentFrameCaptionLabel->setStyleSheet(AF_STR_captionLabelStyleSheetMain);

    m_pCurrentFrameDetailsLabel = new QLabel;
    QLabel* pPropsHTMLLabelCaption = new QLabel(GPU_STR_dashboard_ExecutionCaption);
    m_pCurrentFrameDetailsLabel->setStyleSheet("font-weight: bold; color: gray; font-size: 15px;");

    pPropsHTMLLabelCaption->setStyleSheet(AF_STR_captionLabelStyleSheetMain);
    m_pPropsHTMLLabel = new QLabel;
    m_pPropsHTMLLabel->setTextFormat(Qt::RichText);

    QFrame* pImageFrameWidget = new QFrame;
    QPalette p = pImageFrameWidget->palette();
    p.setColor(backgroundRole(), Qt::white);
    p.setColor(QPalette::Base, Qt::white);
    p.setColor(QPalette::Shadow, Qt::red);
    pImageFrameWidget->setAutoFillBackground(true);
    pImageFrameWidget->setPalette(p);
    pImageFrameWidget->setLineWidth(1);
    pImageFrameWidget->setMidLineWidth(1);
    pImageFrameWidget->setFrameStyle(QFrame::Panel | QFrame::Raised);


    QVBoxLayout* pImageFrameLayout = new QVBoxLayout;
    m_pCurrentFrameDetailsLabel->setContentsMargins(0, 0, 0, 0);

    pImageFrameLayout->addWidget(m_pCurrentFrameDetailsLabel, 0, Qt::AlignCenter);
    pImageFrameLayout->addWidget(m_pImageLabel, 0, Qt::AlignCenter);
    pImageFrameWidget->setLayout(pImageFrameLayout);

    pImageLayout->addWidget(m_pCurrentFrameCaptionLabel);
    pImageLayout->addSpacing(4);
    pImageLayout->addWidget(pImageFrameWidget);
    pImageLayout->addStretch();
    pImageLayout->addWidget(pPropsHTMLLabelCaption);
    pImageLayout->addWidget(m_pPropsHTMLLabel);

    FillExecutionDetails();

    // Create the frame analysis section:
    m_pCapturedFramesCaptionLabel = new QLabel(GPU_STR_dashboard_CapturedFramesCaption);
    m_pCapturedFramesCaptionLabel->setStyleSheet(AF_STR_captionLabelStyleSheetMain);

#define GP_SessionViewButtonStyle "QToolButton { border: 1px solid transparent; background-color: transparent; font-weight: bold; color: gray; }" \
    "QToolButton:hover { border: 1px solid gray; background-color: #CDE6F7; color: black;}"

    QHBoxLayout* pButtonsLayout = new QHBoxLayout;
    pButtonsLayout->addStretch();

    m_pCaptureButton = new QToolButton;
    m_pCaptureButton->setText(GPU_STR_dashboard_CaptureButton);
    m_pCaptureButton->setStyleSheet(GP_SessionViewButtonStyle);

    QString tooltipStr = QString(GPU_STR_dashboard_CaptureTooltip).arg(settings.m_numFramesToCapture);
    m_pCaptureButton->setToolTip(tooltipStr);
/////////////////// capture buttons
    m_pCaptureButtonCPU = new QToolButton;
    m_pCaptureButtonCPU->setText(GPU_STR_dashboard_CaptureCPUButton);
    m_pCaptureButtonCPU->setStyleSheet(GP_SessionViewButtonStyle);

    QString tooltipStrCPU = QString(GPU_STR_dashboard_CaptureTooltip).arg(settings.m_numFramesToCapture);
    m_pCaptureButtonCPU->setToolTip(tooltipStrCPU);

    m_pCaptureButtonGPU = new QToolButton;
    m_pCaptureButtonGPU->setText(GPU_STR_dashboard_CaptureGPUButton);
    m_pCaptureButtonGPU->setStyleSheet(GP_SessionViewButtonStyle);

    QString tooltipStrGPU = QString(GPU_STR_dashboard_CaptureTooltip).arg(settings.m_numFramesToCapture);
    m_pCaptureButtonGPU->setToolTip(tooltipStrGPU);
///////////////////

    m_pStopButton = new QToolButton;
    m_pStopButton->setStyleSheet(GP_SessionViewButtonStyle);
    m_pStopButton->setText(GPU_STR_dashboard_StopButton);
    m_pStopButton->setToolTip(GPU_STR_dashboard_StopTooltip);

    m_pOpenTimelineButton = new QToolButton;
    m_pOpenTimelineButton->setStyleSheet(GP_SessionViewButtonStyle);
    m_pOpenTimelineButton->setText(GPU_STR_dashboard_OpenTimelineButton);
    m_pOpenTimelineButton->setToolTip(GPU_STR_dashboard_OpenTimelineTooltip);

    bool rc = connect(m_pCaptureButton, SIGNAL(clicked()), this, SLOT(OnCaptureButtonClick()));
    GT_ASSERT(rc);
    rc = connect(m_pCaptureButtonCPU, SIGNAL(clicked()), this, SLOT(OnCaptureCPUButtonClick()));
    GT_ASSERT(rc);
    rc = connect(m_pCaptureButtonGPU, SIGNAL(clicked()), this, SLOT(OnCaptureGPUButtonClick()));
    GT_ASSERT(rc);

    rc = connect(m_pStopButton, SIGNAL(clicked()), this, SLOT(OnStopButtonClick()));
    GT_ASSERT(rc);

    rc = connect(m_pOpenTimelineButton, SIGNAL(clicked()), this, SLOT(OnOpenTimelineButtonClick()));
    GT_ASSERT(rc);

    acIconSize largerButtonIcon = acGetScaledIconSize(AC_32x32_ICON);
    int iconDim = acIconSizeToPixelSize(largerButtonIcon);
    QSize iconSize(iconDim, iconDim);
    QPixmap captureButtonIcon;
    acSetIconInPixmap(captureButtonIcon, AC_ICON_EXECUTION_CAPTURE, largerButtonIcon);
    QPixmap captureButtonIconCPU;
    acSetIconInPixmap(captureButtonIconCPU, AC_ICON_EXECUTION_CAPTURE_ONLY_CPU, largerButtonIcon);
    QPixmap captureButtonIconGPU;
    acSetIconInPixmap(captureButtonIconGPU, AC_ICON_EXECUTION_CAPTURE_ONLY_GPU, largerButtonIcon);
    m_pCaptureButton->setIcon(captureButtonIcon);
    m_pCaptureButton->setIconSize(iconSize);
    m_pCaptureButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pCaptureButton->setFixedWidth(acScalePixelSizeToDisplayDPI(GP_SESSION_VIEW_BUTTON_SIZE));
    m_pCaptureButtonCPU->setIcon(captureButtonIconCPU);
    m_pCaptureButtonCPU->setIconSize(iconSize);
    m_pCaptureButtonCPU->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pCaptureButtonCPU->setFixedWidth(acScalePixelSizeToDisplayDPI(GP_SESSION_VIEW_BUTTON_SIZE));
    m_pCaptureButtonGPU->setIcon(captureButtonIconGPU);
    m_pCaptureButtonGPU->setIconSize(iconSize);
    m_pCaptureButtonGPU->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pCaptureButtonGPU->setFixedWidth(acScalePixelSizeToDisplayDPI(GP_SESSION_VIEW_BUTTON_SIZE));

    QPixmap stopButtonIcon;
    acSetIconInPixmap(stopButtonIcon, AC_ICON_EXECUTION_STOP, largerButtonIcon);
    m_pStopButton->setIcon(stopButtonIcon);
    m_pStopButton->setIconSize(iconSize);
    m_pStopButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pStopButton->setFixedWidth(acScalePixelSizeToDisplayDPI(GP_SESSION_VIEW_BUTTON_SIZE));

    QPixmap openTimelineButtonIcon;
    acSetIconInPixmap(openTimelineButtonIcon, AC_ICON_FRAME_ANALYSIS_APP_TREE_TIMELINE, largerButtonIcon);
    m_pOpenTimelineButton->setIcon(openTimelineButtonIcon);
    m_pOpenTimelineButton->setIconSize(iconSize);
    m_pOpenTimelineButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pOpenTimelineButton->setFixedWidth(acScalePixelSizeToDisplayDPI(GP_SESSION_VIEW_BUTTON_SIZE));

#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    m_pSimulateUserCaptureButton = new QToolButton;
    m_pSimulateUserCaptureButton->setText("Capture Simulate");
    QImage captureInv(captureButtonIcon.toImage());
    captureInv.invertPixels(QImage::InvertRgb);
    m_pSimulateUserCaptureButton->setIcon(QPixmap::fromImage(captureInv));
    m_pSimulateUserCaptureButton->setIconSize(iconSize);
    m_pSimulateUserCaptureButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_pSimulateUserCaptureButton->setStyleSheet(GP_SessionViewButtonStyle);
    rc = connect(m_pSimulateUserCaptureButton, SIGNAL(clicked()), this, SLOT(OnSimulateCaptureButtonClick()));
    GT_ASSERT(rc);
    // pButtonsLayout->addWidget(m_pSimulateUserCaptureButton);
#endif

    pButtonsLayout->addWidget(m_pCaptureButton);
    pButtonsLayout->addWidget(m_pCaptureButtonCPU);
    pButtonsLayout->addWidget(m_pCaptureButtonGPU);
    pButtonsLayout->addWidget(m_pStopButton);
    pButtonsLayout->addWidget(m_pOpenTimelineButton);
    pButtonsLayout->addStretch();
    pImageFrameLayout->addLayout(pButtonsLayout, Qt::AlignCenter | Qt::AlignVCenter);
    pImageFrameLayout->addStretch();

    // Initialize and fill the thumbnail view
    m_pSnapshotsThumbView = new acThumbnailView;
    m_pSnapshotsThumbView->SetItemTooltip(GPU_STR_dashboard_ItemTooltip);

    rc = connect(m_pSnapshotsThumbView, SIGNAL(ItemDoubleClicked(const QVariant&)), this, SLOT(OnItemDoubleClicked(const QVariant&)));
    GT_ASSERT(rc);
    rc = connect(m_pSnapshotsThumbView, SIGNAL(ItemPressed(const QVariant&)), this, SLOT(OnItemPressed(const QVariant&)));
    GT_ASSERT(rc);

    pThumbLayout->addWidget(m_pCapturedFramesCaptionLabel);
    pThumbLayout->addWidget(m_pSnapshotsThumbView, Qt::AlignLeft);

    QWidget* pLeftWidget = new QWidget;
    QWidget* pRightWidget = new QWidget;
    pLeftWidget->setLayout(pImageLayout);
    pRightWidget->setLayout(pThumbLayout);
    pLeftWidget->setMinimumWidth(200);
    pRightWidget->setMinimumWidth(200);

    pMainLayout->addWidget(pLeftWidget, 0);
    pMainLayout->addWidget(pRightWidget, 1);

    setLayout(pMainLayout);
}

void gpSessionView::FillOfflineSessionThumbnails()
{
    OS_DEBUG_LOG_TRACER;
    // Get the list of frame thumbnails for the current session
    QList<FrameIndex> frameIndicesList;
    gpUIManager::Instance()->GetListOfFrameFolders(m_sessionFilePath, frameIndicesList);

    afProgressBarWrapper::instance().ShowProgressDialog(GPU_STR_TraceViewLoadingFASession, frameIndicesList.size());

    foreach (FrameIndex frameIndex, frameIndicesList)
    {
        // Get the directory, overview and thumbnail paths for this frame
        QDir frameDir;
        QString overviewFilePath, thumbnailFilePath;
        FrameInfo currentFrameInfo;
        bool rc = gpUIManager::Instance()->GetPathsForFrame(m_sessionFilePath, frameIndex, frameDir, overviewFilePath, thumbnailFilePath);
        GT_IF_WITH_ASSERT(rc)
        {
            // Get the frame info and thumb data
            rc = gpUIManager::Instance()->GetFrameImageAndInfo(overviewFilePath, currentFrameInfo);

            if (rc)
            {
                // Add the captured frame
                AddCapturedFrame(currentFrameInfo);
            }

            afProgressBarWrapper::instance().incrementProgressBar();
        }
    }

    afProgressBarWrapper::instance().hideProgressBar();
    GT_IF_WITH_ASSERT(m_pSnapshotsThumbView != nullptr)
    {
        if (!frameIndicesList.isEmpty())
        {
            // Select the first thumbnail (we want the main image to be filled with one of the frame)
            m_pSnapshotsThumbView->SetSelected(0, true);
        }
    }
    UpdateCaption();

}

bool gpSessionView::DisplaySession(const osFilePath& sessionFilePath, afTreeItemType sessionInnerPage, QString& errorMessage)
{
    GT_UNREFERENCED_PARAMETER(sessionInnerPage);
    GT_UNREFERENCED_PARAMETER(errorMessage);
    bool retVal = true;
    OS_DEBUG_LOG_TRACER_WITH_RETVAL(retVal);

    m_sessionFilePath = sessionFilePath;

    // Get the running session data
    SessionTreeNodeData* pSessionData = ProfileApplicationTreeHandler::instance()->FindSessionDataByProfileFilePath(m_sessionFilePath);
    gpExecutionMode* pModeManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
    GT_IF_WITH_ASSERT((pModeManager != nullptr) && (pSessionData != nullptr) && (m_pStopButton != nullptr)
                      && (m_pCurrentFrameCaptionLabel != nullptr) && (m_pCaptureButton != nullptr)&& (m_pCaptureButtonCPU != nullptr)&& (m_pCaptureButtonGPU != nullptr))
    {
        m_pSessionData = qobject_cast<gpSessionTreeNodeData*>(pSessionData);
        GT_IF_WITH_ASSERT(m_pSessionData != nullptr)
        {
            // Check if this session is running
            m_isSessionRunning = (m_pSessionData == gpUIManager::Instance()->CurrentlyRunningSessionData());

            if (m_isSessionRunning)
            {
                // Connect to the session stop signal
                bool rc = connect(gpUIManager::Instance(), SIGNAL(ApplicationRunEnded(gpSessionTreeNodeData*)), this, SLOT(OnApplicationRunEnded(gpSessionTreeNodeData*)));
                GT_ASSERT(rc);

                // Connect to the session stop signal
                rc = connect(gpUIManager::Instance(), SIGNAL(UIUpdated()), this, SLOT(OnUpdateUI()));
                GT_ASSERT(rc);


                gpSessionUpdaterThread& updaterThread = gpUIManager::Instance()->UIUpdaterThread();
                updaterThread.m_sessionName = m_pSessionData->m_displayName.toStdString().c_str();
                const apProjectSettings& projectSettings = afProjectManager::instance().currentProjectSettings();
                updaterThread.m_projectName = projectSettings.projectName().asASCIICharArray();

                updaterThread.start();

                // Make sure that the slots are not disconnected
                // Connect to the thread's image update signal
                disconnect(&gpUIManager::Instance()->UIUpdaterThread(), SIGNAL(CurrentFrameUpdateReady(QPixmap*, const FrameInfo&)), this, SLOT(OnCurrentFrameUpdate(QPixmap*, const FrameInfo&)));
                disconnect(gpUIManager::Instance(), SIGNAL(CapturedFrameUpdateReady(int, int)), this, SLOT(OnCaptureFrameData(int, int)));

                // Connect to the thread's image update signal
                rc = connect(&gpUIManager::Instance()->UIUpdaterThread(), SIGNAL(CurrentFrameUpdateReady(QPixmap*, const FrameInfo&)), this, SLOT(OnCurrentFrameUpdate(QPixmap*, const FrameInfo&)), Qt::DirectConnection);
                GT_ASSERT(rc);

                rc = connect(gpUIManager::Instance(), SIGNAL(CapturedFrameUpdateReady(int, int)), this, SLOT(OnCaptureFrameData(int, int)));
                GT_ASSERT(rc);

                // connect to the session termination signal
                rc = connect(&gpUIManager::Instance()->UIUpdaterThread(), SIGNAL(finished()), this, SLOT(OnUpdateThreadFinished()));
                GT_ASSERT(rc);
            }

            // Show / hide the stop and capture buttons
            m_pStopButton->setVisible(m_isSessionRunning);
            m_pCaptureButton->setVisible(m_isSessionRunning);
            m_pCaptureButtonCPU->setVisible(m_isSessionRunning);
            m_pCaptureButtonGPU->setVisible(m_isSessionRunning);

            // If the session is not running, thumbnails should be uploaded from the file system
            if (!m_isSessionRunning)
            {
                FillOfflineSessionThumbnails();
            }

            int itemsCount = m_pSnapshotsThumbView->ItemsCount();
            m_pOpenTimelineButton->setVisible(!m_isSessionRunning && (itemsCount > 0));

            QString caption = m_isSessionRunning ? GPU_STR_dashboard_MainImageCaptionRunning : GPU_STR_dashboard_MainImageCaptionStopped;
            m_pCurrentFrameCaptionLabel->setText(caption);
        }
    }

    return retVal;
}

void gpSessionView::OnCaptureButtonClick()
{
    OS_DEBUG_LOG_TRACER;
    afApplicationCommands::instance()->StartPerformancePrintout("Capture");
    // Sanity check
    GT_IF_WITH_ASSERT(ProfileManager::Instance()->GetFrameAnalysisModeManager() != nullptr && m_pCaptureButton != nullptr&& m_pCaptureButtonCPU != nullptr&& m_pCaptureButtonGPU != nullptr)
    {
        // Add the captured frame to the tree
        bool rc = ProfileManager::Instance()->GetFrameAnalysisModeManager()->CaptureFrame(gpExecutionMode::FrameAnalysisCaptureType_LinkedTrace);
        GT_ASSERT(rc);

        // Disable the capture button until the captured frame is received
        m_pCaptureButton->setEnabled(false);
        m_pCaptureButtonCPU->setEnabled(false);
        m_pCaptureButtonGPU->setEnabled(false);
    }
}
void gpSessionView::OnCaptureCPUButtonClick()
{
    OS_DEBUG_LOG_TRACER;
    afApplicationCommands::instance()->StartPerformancePrintout("CaptureCPU");
    // Sanity check
    GT_IF_WITH_ASSERT(ProfileManager::Instance()->GetFrameAnalysisModeManager() != nullptr && m_pCaptureButton != nullptr&& m_pCaptureButtonCPU != nullptr&& m_pCaptureButtonGPU != nullptr)
    {
        // Add the captured frame to the tree
        bool rc = ProfileManager::Instance()->GetFrameAnalysisModeManager()->CaptureFrame(gpExecutionMode::FrameAnalysisCaptureType_APITrace);
        GT_ASSERT(rc);

        // Disable the capture button until the captured frame is received
        m_pCaptureButton->setEnabled(false);
        m_pCaptureButtonCPU->setEnabled(false);
        m_pCaptureButtonGPU->setEnabled(false);
    }
}
void gpSessionView::OnCaptureGPUButtonClick()
{
    OS_DEBUG_LOG_TRACER;
    afApplicationCommands::instance()->StartPerformancePrintout("CaptureGPU");
    // Sanity check
    GT_IF_WITH_ASSERT(ProfileManager::Instance()->GetFrameAnalysisModeManager() != nullptr && m_pCaptureButton != nullptr&& m_pCaptureButtonCPU != nullptr&& m_pCaptureButtonGPU != nullptr)
    {
        // Add the captured frame to the tree
        bool rc = ProfileManager::Instance()->GetFrameAnalysisModeManager()->CaptureFrame(gpExecutionMode::FrameAnalysisCaptureType_GPUTrace);
        GT_ASSERT(rc);

        // Disable the capture button until the captured frame is received
        m_pCaptureButton->setEnabled(false);
        m_pCaptureButtonCPU->setEnabled(false);
        m_pCaptureButtonGPU->setEnabled(false);
    }
}
void gpSessionView::OnSimulateCaptureButtonClick()
{
    OS_DEBUG_LOG_TRACER;
    // Set a dummy frame index (only add 5, to make sure that the next "real" captured frame will be bigger then m_lastCapturedFrameIndex
    m_lastCapturedFrameIndex += 5;

    GT_IF_WITH_ASSERT(m_pSessionData != nullptr)
    {
        // Calculate the server path for the current session
        gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();
        gtString sessionName = acQStringToGTString(m_pSessionData->m_displayName);
        gtString frameFolderName, frameFileName, thumbFileName, xmlFileName;
        frameFolderName.appendFormattedString(L"Frame_%010d", m_lastCapturedFrameIndex);
        thumbFileName.appendFormattedString(L"%ls_FrameBuffer%d", projectName.asCharArray(), m_lastCapturedFrameIndex);
        osFilePath serverFramesFilePath(osFilePath::OS_TEMP_DIRECTORY);
        serverFramesFilePath.appendSubDirectory(L"CodeXL");
        serverFramesFilePath.appendSubDirectory(projectName);

        osDirectory sessionFolder;
        serverFramesFilePath.getFileDirectory(sessionFolder);

        if (!sessionFolder.exists())
        {
            sessionFolder.create();
        }

        serverFramesFilePath.appendSubDirectory(sessionName);
        serverFramesFilePath.appendSubDirectory(frameFolderName);

        osDirectory frameFolder;
        serverFramesFilePath.getFileDirectory(frameFolder);

        if (frameFolder.exists())
        {
            m_lastCapturedFrameIndex += 1;

            // Calculate the server path for the current session
            gtString sessionName = acQStringToGTString(m_pSessionData->m_displayName);
            gtString frameFolderName;
            frameFolderName.appendFormattedString(L"Frame_%010d", m_lastCapturedFrameIndex);
            osFilePath serverFramesFilePath(osFilePath::OS_TEMP_DIRECTORY);
            serverFramesFilePath.appendSubDirectory(L"CodeXL");
            gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();
            serverFramesFilePath.appendSubDirectory(projectName);
            serverFramesFilePath.appendSubDirectory(sessionName);
            serverFramesFilePath.appendSubDirectory(frameFolderName);
            serverFramesFilePath.getFileDirectory(frameFolder);
        }

        // Create the frame folder
        frameFolder.create();

        osFilePath thumbFilePath = serverFramesFilePath;
        osFilePath traceFilePath = serverFramesFilePath;
        osFilePath xmlFilePath = serverFramesFilePath;
        thumbFilePath.setFileName(L"TempForSimulation");
        traceFilePath.setFileName(L"TempForSimulation");
        xmlFilePath.setFileName(L"TempForSimulation");

        thumbFilePath.setFileExtension(AF_STR_FrameAnalysisTraceFileImageExtension);
        traceFilePath.setFileExtension(AF_STR_FrameAnalysisTraceFileExtension);
        xmlFilePath.setFileExtension(L"xml");

        bool rc = QFile::copy("C:\\temp\\TempForSimulation.ltr", acGTStringToQString(traceFilePath.asString()));
        GT_IF_WITH_ASSERT(rc)
        {
            rc = QFile::copy("C:\\temp\\TempForSimulation.png", acGTStringToQString(thumbFilePath.asString()));
            GT_IF_WITH_ASSERT(rc)
            {
                FrameInfo dummyFrameInfo;
                osDirectory serverFrameFolder;
                xmlFilePath.getFileDirectory(serverFrameFolder);
                dummyFrameInfo.m_frameIndex = m_lastCapturedFrameIndex;
                dummyFrameInfo.m_descriptionFileRemotePath = xmlFilePath.asString();
                dummyFrameInfo.m_serverFolderPath = serverFrameFolder.directoryPath().asString();
                dummyFrameInfo.m_traceFileRemotePath = traceFilePath.asString();
                QString xmlString;
                gpUIManager::Instance()->FrameInfoToXML(dummyFrameInfo, xmlString);
                QFile fileHandle(acGTStringToQString(xmlFilePath.asString()));

                // Open the file for write:
                if (fileHandle.open(QFile::WriteOnly | QFile::Truncate))
                {
                    QTextStream streamData(&fileHandle);
                    streamData << xmlString;
                    fileHandle.close();
                }
            }
        }

    }
}


void gpSessionView::OnStopButtonClick()
{
    // Stop the execution
    bool rc = ProfileManager::Instance()->GetFrameAnalysisModeManager()->stopCurrentRun();
    GT_ASSERT(rc);

    // Make sure that the toolbar is in synchronized
    afApplicationCommands::instance()->updateToolbarCommands();
}

void gpSessionView::OnOpenTimelineButtonClick()
{
    OS_DEBUG_LOG_TRACER;
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSnapshotsThumbView != nullptr)
    {
        QVariant selectedItem;
        bool isItemSelected = m_pSnapshotsThumbView->GetSelected(selectedItem);

        // If there is no item selected we should not get here
        GT_IF_WITH_ASSERT(isItemSelected)
        {
            // Get the requested frame index
            QString capturedFrameIndexStr = selectedItem.toString();
            FrameIndex captureFrameIndex = FrameIndexFromString(capturedFrameIndexStr);

            // Build the trace file path for this frame
            osFilePath traceFilePath = gpTreeHandler::BuildFrameChildFilePath(m_pSessionData, AF_TREE_ITEM_GP_FRAME_TIMELINE, captureFrameIndex);
            afApplicationCommands::instance()->OpenFileAtLine(traceFilePath, AF_TREE_ITEM_GP_FRAME_TIMELINE);
        }
    }
}

void gpSessionView::AddCapturedFrame(const FrameInfo& frameInfo)
{
    OS_DEBUG_LOG_TRACER;
    // Sanity check
    GT_IF_WITH_ASSERT(m_pSnapshotsThumbView != nullptr)
    {
        // Add the frame thumbnail to the thumbnail view
        QStringList textLines;
        FrameIndex frameIndex(frameInfo.m_frameIndex, frameInfo.m_frameIndex + frameInfo.m_framesCount - 1);
        QString elapsedTimeString = MsecToTimeString(frameInfo.m_elapsedTimeMS, true, false);
        QString durationTimeString = MsecToTimeString(frameInfo.m_frameDuration, true, false);
        QLocale locale(QLocale::English);
        QString frameIndexStr = FrameIndexToString(frameIndex);
        QString apiCallsStr = locale.toString((qlonglong)frameInfo.m_apiCalls);
        QString drawCallsStr = locale.toString((qlonglong)frameInfo.m_drawCalls);

        textLines << QString(GPU_STR_dashboard_FrameNumber).arg(frameIndexStr);
        textLines << QString(elapsedTimeString);
        textLines << QString(GPU_STR_dashboard_Duration).arg(durationTimeString);
        textLines << QString(GPU_STR_dashboard_FPS).arg(frameInfo.m_fps);
        textLines << QString(GPU_STR_dashboard_APICalls).arg(apiCallsStr);
        textLines << QString(GPU_STR_dashboard_DrawCalls).arg(drawCallsStr);

        m_pSnapshotsThumbView->AddThumbnail(frameInfo.m_pImageBuffer, frameInfo.m_imageSize, textLines, QVariant(frameIndexStr));

        // Set the last captured frame index
        m_lastCapturedFrameIndex = frameInfo.m_frameIndex;

        afApplicationCommands::instance()->EndPerformancePrintout("Capture");
    }
    UpdateCaption();
}


void gpSessionView::OnCaptureFrameData(int firstFrameIndex, int lastFrameIndex)
{
    OS_DEBUG_LOG_TRACER;
    // Sanity check
    gpExecutionMode* pExecutionMode = ProfileManager::Instance()->GetFrameAnalysisModeManager();
    GT_IF_WITH_ASSERT((m_pSessionData != nullptr) && (m_pSnapshotsThumbView != nullptr) && (pExecutionMode != nullptr))
    {
        if (firstFrameIndex == gpSessionUpdaterThread::INVALID_FRAME_INDEX_INDICATING_NO_RENDER)
        {
            // Anything to do in this case?
        }
        else
        {
            // Get the directory, overview and thumbnail paths for this frame
            QDir frameDir;
            QString overviewFilePath, thumbnailFilePath;
            FrameIndex frameIndex(firstFrameIndex, lastFrameIndex);
            bool rc = gpUIManager::Instance()->GetPathsForFrame(m_sessionFilePath, frameIndex, frameDir, overviewFilePath, thumbnailFilePath);
            GT_IF_WITH_ASSERT(rc)
            {
                // Get the current frame data from the remote agent
                FrameInfo capturedFrameInfo;
                capturedFrameInfo.m_frameIndex = frameIndex.first;
                capturedFrameInfo.m_framesCount = frameIndex.second - frameIndex.first + 1;
                bool rc = pExecutionMode->GetCapturedFrameData(capturedFrameInfo);
                GT_IF_WITH_ASSERT(rc && capturedFrameInfo.m_pImageBuffer != nullptr)
                {
                    // Write the thumbnail to the disk for off line session load
                    FILE* pFile;
                    pFile = fopen(thumbnailFilePath.toStdString().c_str(), "wb");
                    const size_t writtenCount = fwrite(capturedFrameInfo.m_pImageBuffer, sizeof(char), capturedFrameInfo.m_imageSize, pFile);
                    fclose(pFile);
                    GT_ASSERT(writtenCount == capturedFrameInfo.m_imageSize);
                }

                // Add the captured frame to the UI
                AddCapturedFrame(capturedFrameInfo);
            }
        }

        m_pCaptureButton->setEnabled(true);
        m_pCaptureButtonCPU->setEnabled(true);
        m_pCaptureButtonGPU->setEnabled(true);

        // Notify the UI that the capture process had ended
        pExecutionMode->SetIsCapturing(false);
        gpUIManager::Instance()->UIUpdaterThread().SetCaptureIsComplete();
    }
}

void gpSessionView::OnApplicationRunEnded(gpSessionTreeNodeData* pRunningSessionData)
{
    GT_UNREFERENCED_PARAMETER(pRunningSessionData);
    OS_DEBUG_LOG_TRACER;
    m_isSessionRunning = false;

    // disconnect termination signal so the user won't get a message that the application ended
    disconnect(&gpUIManager::Instance()->UIUpdaterThread(), SIGNAL(finished()), this, SLOT(OnUpdateThreadFinished()));

    // Request that the thread updating the image stop running
    gpUIManager::Instance()->UIUpdaterThread().requestInterruption();

    // Wait for the thread to stop
    const unsigned long waitTimeoutInMilliseconds = 1000;
    bool isGracefulExit = gpUIManager::Instance()->UIUpdaterThread().wait(waitTimeoutInMilliseconds);

    if (false == isGracefulExit)
    {
        // The thread did not exit in time so we terminate it
        gpUIManager::Instance()->UIUpdaterThread().terminate();
    }

    // Disconnect from the thread's image update signal
    disconnect(&gpUIManager::Instance()->UIUpdaterThread(), SIGNAL(CurrentFrameUpdateReady(QPixmap*, const FrameInfo&)), this, SLOT(OnCurrentFrameUpdate(QPixmap*, const FrameInfo&)));

    // Disconnect from the capture frame ready signal
    disconnect(gpUIManager::Instance(), SIGNAL(CapturedFrameUpdateReady(int, int)), this, SLOT(OnCaptureFrameData(int, int)));

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pStopButton != nullptr) && (m_pCurrentFrameCaptionLabel != nullptr) && (m_pOpenTimelineButton != nullptr) && (m_pSnapshotsThumbView != nullptr))
    {
        // Show / hide the buttons
        QVariant var;
        bool isItemSelected = m_pSnapshotsThumbView->GetSelected(var);
        int itemsCount = m_pSnapshotsThumbView->ItemsCount();
        m_pStopButton->setVisible(m_isSessionRunning);
        m_pCaptureButton->setVisible(m_isSessionRunning);
        m_pCaptureButtonCPU->setVisible(m_isSessionRunning);
        m_pCaptureButtonGPU->setVisible(m_isSessionRunning);
        m_pOpenTimelineButton->setVisible(!m_isSessionRunning && (itemsCount > 0));
        m_pOpenTimelineButton->setEnabled(isItemSelected && (itemsCount > 0));
        m_pCurrentFrameCaptionLabel->setText(GPU_STR_dashboard_MainImageCaptionStopped);

        // Select the first capture frame, to make sure that the "Open Timeline" button is enabled
        if (m_pSnapshotsThumbView->ItemsCount() > 0)
        {
            m_pSnapshotsThumbView->SetSelected(0, true);
        }
    }
}


void gpSessionView::OnUpdateUI()
{
    // Update the session for live view
    if (m_isSessionRunning)
    {
        // Sanity check:
        gpExecutionMode* pModeManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
        GT_IF_WITH_ASSERT(m_pStopButton != nullptr && m_pCaptureButton != nullptr && m_pCaptureButtonCPU != nullptr && m_pCaptureButtonGPU != nullptr && pModeManager != nullptr)
        {
            m_pCaptureButton->setEnabled(pModeManager->IsCaptureEnabled());
            m_pCaptureButtonCPU->setEnabled(pModeManager->IsCaptureEnabled());
            m_pCaptureButtonGPU->setEnabled(pModeManager->IsCaptureEnabled());
            m_pStopButton->setEnabled(pModeManager->IsStopEnabled());

            gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
            gpProjectSettings& settings = pFrameAnalysisManager->ProjectSettings();
            QString tooltipStr = QString(GPU_STR_dashboard_CaptureTooltip).arg(settings.m_numFramesToCapture);
            m_pCaptureButton->setToolTip(tooltipStr);
            m_pCaptureButtonCPU->setToolTip(tooltipStr);
            m_pCaptureButtonGPU->setToolTip(tooltipStr);

        }
    }
}

void gpSessionView::OnCurrentFrameUpdate(QPixmap* pCurrentFramePixmap, const FrameInfo& frameInfo)
{
    OS_DEBUG_LOG_TRACER;

    if (m_isSessionRunning && !m_isDisplayingCurrentFrame)
    {
        // Sanity check:
        GT_IF_WITH_ASSERT((m_pImageLabel != nullptr) && (m_pCurrentFrameDetailsLabel != nullptr))
        {
            if (pCurrentFramePixmap != nullptr)
            {
                QSize optimizedSize = m_pImageLabel->size();

                if (pCurrentFramePixmap->width() > pCurrentFramePixmap->height())
                {
                    float ratio = (float)m_pImageLabel->width() / (float)pCurrentFramePixmap->width();
                    optimizedSize.setWidth(m_pImageLabel->width());
                    optimizedSize.setHeight(pCurrentFramePixmap->height() * ratio);
                }
                else
                {
                    float ratio = (float)m_pImageLabel->height() / (float)pCurrentFramePixmap->height();
                    optimizedSize.setHeight(m_pImageLabel->height());
                    optimizedSize.setWidth(pCurrentFramePixmap->width() * ratio);
                }

                QPixmap resizedImage = pCurrentFramePixmap->scaled(optimizedSize);
                m_pImageLabel->setPixmap(resizedImage);

                delete pCurrentFramePixmap;

                // Build the description string
                QString timeString = MsecToTimeString(frameInfo.m_elapsedTimeMS, false, false);
                QLocale locale(QLocale::English);
                QString frameIndexStr = locale.toString((qlonglong)frameInfo.m_frameIndex);
                QString currentFrameDetails = QString(GPU_STR_dashboard_FrameDetailsLabel).arg(frameIndexStr).arg(timeString).arg(frameInfo.m_fps);
                m_pCurrentFrameDetailsLabel->setText(currentFrameDetails);
            }
            else
            {
                // Null pixmap indicates the profiled application is dead or communication failure, so stop the session
                OnStopButtonClick();
            }
        }
    }
}

void gpSessionView::OnItemDoubleClicked(const QVariant& capturedFrameId)
{
    bool shouldOpen = true;

    if (m_isSessionRunning)
    {
        shouldOpen = false;

        // Trace files are only opened for offline sessions
        int userSelection = acMessageBox::instance().information(AF_STR_InformationA, GPU_STR_dashboard_RunTimeDoubleClickMessage, QMessageBox::Yes | QMessageBox::No);

        if (userSelection == QMessageBox::Yes)
        {
            OnStopButtonClick();
            shouldOpen = true;
        }
    }


    if (shouldOpen)
    {
        // Get the requested frame index
        QString capturedFrameIndexStr = capturedFrameId.toString();
        FrameIndex captureFrameIndex = FrameIndexFromString(capturedFrameIndexStr);

        // Build the overview file path for this frame
#ifdef INCLUDE_FRAME_ANALYSIS_PERFORMANCE_COUNTERS
        osFilePath overviewFilePath = gpTreeHandler::BuildFrameChildFilePath(m_pSessionData, AF_TREE_ITEM_GP_FRAME_OVERVIEW, captureFrameIndex);
        afApplicationCommands::instance()->OpenFileAtLine(overviewFilePath, AF_TREE_ITEM_GP_FRAME_OVERVIEW);
#else
        // Build the trace file path for this frame
        osFilePath traceFilePath = gpTreeHandler::BuildFrameChildFilePath(m_pSessionData, AF_TREE_ITEM_GP_FRAME_TIMELINE, captureFrameIndex);
        afApplicationCommands::instance()->OpenFileAtLine(traceFilePath, AF_TREE_ITEM_GP_FRAME_TIMELINE);
#endif
    }
}

void gpSessionView::OnItemPressed(const QVariant& capturedFrameId)
{
    if (!m_isSessionRunning)
    {
        // For offline sessions, display the selected frame in the main image area
        QString capturedFrameIndexStr = capturedFrameId.toString();
        FrameIndex captureFrameIndex = FrameIndexFromString(capturedFrameIndexStr);

        GT_IF_WITH_ASSERT(captureFrameIndex.first >= 0)
        {
            // Set the current frame image
            DisplayCurrentFrame(captureFrameIndex);

            // Sanity check:
            GT_IF_WITH_ASSERT(m_pOpenTimelineButton != nullptr)
            {
                // Enable the open timeline button
                m_pOpenTimelineButton->setEnabled(true);
            }
        }
    }
}

void gpSessionView::DisplayCurrentFrame(FrameIndex frameIndex)
{
    OS_DEBUG_LOG_TRACER;
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pImageLabel != nullptr) && (m_pCurrentFrameDetailsLabel != nullptr))
    {
        bool rc = false;
        FrameInfo frameInfo;
        frameInfo.m_frameIndex = frameIndex.first;
        frameInfo.m_framesCount = frameIndex.second - frameIndex.first + 1;
        gtASCIIString infoXML;

        // Get the frame info from the files saved on disk
        rc = gpUIManager::Instance()->GetFrameInfo(m_sessionFilePath, frameInfo);

        GT_IF_WITH_ASSERT(rc)
        {
            GT_IF_WITH_ASSERT(frameInfo.m_pImageBuffer != nullptr)
            {
                QPixmap thumbnailPixmap;
                bool rcLoad = thumbnailPixmap.loadFromData(frameInfo.m_pImageBuffer, frameInfo.m_imageSize, GP_thumbnailImageExtension);
                GT_IF_WITH_ASSERT(rcLoad)
                {
                    QSize optimizedSize = m_pImageLabel->size();

                    if (thumbnailPixmap.width() > thumbnailPixmap.height())
                    {
                        float ratio = (float)m_pImageLabel->width() / (float)thumbnailPixmap.width();
                        optimizedSize.setWidth(m_pImageLabel->width());
                        optimizedSize.setHeight(thumbnailPixmap.height() * ratio);
                    }
                    else
                    {
                        float ratio = (float)m_pImageLabel->height() / (float)thumbnailPixmap.height();
                        optimizedSize.setHeight(m_pImageLabel->height());
                        optimizedSize.setWidth(thumbnailPixmap.width() * ratio);
                    }

                    QPixmap resizedImage = thumbnailPixmap.scaled(optimizedSize);
                    m_pImageLabel->setPixmap(resizedImage);
                }
                delete[] frameInfo.m_pImageBuffer;
                frameInfo.m_pImageBuffer = nullptr;
                frameInfo.m_imageSize = 0;
            }

            // Build the description string
            QString timeString = MsecToTimeString(frameInfo.m_elapsedTimeMS, false, false);
            QLocale locale(QLocale::English);
            QString frameIndexStr = locale.toString((qlonglong)frameInfo.m_frameIndex);
            QString currentFrameDetails = QString(GPU_STR_dashboard_FrameDetailsLabel).arg(frameIndexStr).arg(timeString).arg(frameInfo.m_fps);
            m_pCurrentFrameDetailsLabel->setText(currentFrameDetails);
        }
    }
}

void gpSessionView::FillExecutionDetails()
{
    GT_IF_WITH_ASSERT(m_pPropsHTMLLabel != nullptr)
    {
        gtString strContent;
        afHTMLContent htmlContent;

        gpFASessionData sessionData;

        bool rc = gpUIManager::Instance()->ReadDashboardFile(sessionData, m_sessionPath);
        GT_ASSERT(rc);

        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_BOLD_LINE, GPU_STR_dashboard_HTMLHost, sessionData.m_hostIP);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_BOLD_LINE, GPU_STR_dashboard_HTMLTargetPath, sessionData.m_exePath);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_BOLD_LINE, GPU_STR_dashboard_HTMLWorkingDirectory, sessionData.m_workingFolder);

        // If there are on command line arguments, add an empty string to avoid HTML empty cell
        gtString cmdArgs = sessionData.m_cmdArgs;
        if (cmdArgs.isEmpty())
        {
            cmdArgs = acQStringToGTString(AF_STR_HtmlNBSP);
        }
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_BOLD_LINE, GPU_STR_dashboard_HTMLCommandLineArguments, cmdArgs);

        htmlContent.toString(strContent);

        m_pPropsHTMLLabel->setText(acGTStringToQString(strContent));
    }
}

void gpSessionView::OnUpdateThreadFinished()
{
    OS_DEBUG_LOG_TRACER;
    disconnect(&gpUIManager::Instance()->UIUpdaterThread(), SIGNAL(finished()), this, SLOT(OnUpdateThreadFinished()));

    // take action only if termination was because of an error
    if (gpUIManager::Instance()->UIUpdaterThread().EndedWithError() && m_isSessionRunning)
    {
        acMessageBox::instance().information(AF_STR_InformationA, GPU_STR_dashboard_serverdisconnectedError);
        OnStopButtonClick();
    }
}

void gpSessionView::UpdateCaption()
{
    GT_IF_WITH_ASSERT(m_pCapturedFramesCaptionLabel != nullptr)
    {
        int numFrames = gpUIManager::Instance()->GetNumberOfFrameFolders(m_sessionFilePath);
        m_pCapturedFramesCaptionLabel->setText(QString(GPU_STR_dashboard_CapturedFramesCaption).append(GPU_STR_dashboard_CapturedFramesCaptionNumFrames).arg(numFrames));
    }
}