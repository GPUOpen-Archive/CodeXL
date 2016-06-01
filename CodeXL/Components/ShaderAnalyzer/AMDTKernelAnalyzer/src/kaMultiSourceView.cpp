//------------------------------ kaMultiSourceView.cpp ------------------------------
// Qt
#include <QtWidgets>

#include <AMDTKernelAnalyzer/src/kaSourceCodeView.h>
#include <AMDTKernelAnalyzer/src/kaSourceCodeTableView.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTApplicationComponents/Include/acSourceCodeDefinitions.h>
#include <AMDTApplicationComponents/Include/acTabWidget.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Framework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afDocUpdateManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afSourceCodeViewsManager.h>
#include <AMDTApplicationFramework/Include/afBrowseAction.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaKernelView.h>
#include <AMDTKernelAnalyzer/src/kaMultiSourceView.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaUtils.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>


// ---------------------------------------------------------------------------
// Name:        kaMultiSourceView::kaMultiSourceView
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        20/8/2013
// ---------------------------------------------------------------------------
kaMultiSourceView::kaMultiSourceView(QWidget* pParent, const osFilePath& sourceFilePath, const osFilePath& mdiFilePath, int leftWidgetSize, int rightWidgetSize) : QWidget(pParent),
    m_pMainLayout(nullptr), m_pSourceView(nullptr), m_pSplitter(nullptr), m_pTabWidget(nullptr), m_pFindSourceCodeView(nullptr), m_platformIndicator(kaPlatformUnknown),
    m_mdiFilePath(mdiFilePath), m_isILViewHidden(false), m_isISAViewHidden(false),
    m_isISAViewEmpty(false), m_isILViewEmpty(false), m_pExportToCSVAction(nullptr)
{
    // initially the view is up to date:
    m_isNotUpToDateShown = false;

    //get program name from path
    boost::filesystem::path path(mdiFilePath.asString().asCharArray());
    gtString programName = kaUtils::ToGtString(path.parent_path().parent_path().filename());

    m_platformIndicator = KA_PROJECT_DATA_MGR_INSTANCE.GetBuildPlatform(programName);

    m_pParentKernelView = qobject_cast<kaKernelView*>(pParent);
    m_displayedViews[0] = m_displayedViews[1] = m_displayedViews[2] = true;

    // initialize the menu actions:
    m_pActions[0] = m_pActions[1] = m_pActions[2] = nullptr;

    bool showLineNumber = afSourceCodeViewsManager::instance().showLineNumbers();
    int iContextMneuMaskSource = 0x1E;

    // Create the main layout and splitter:
    m_pMainLayout = new QHBoxLayout(this);

    m_pMainLayout->setContentsMargins(0, 0, 0, 0);

    m_pSplitter = new QSplitter(this);
    bool rc = QObject::connect(m_pSplitter, SIGNAL(splitterMoved(int, int)), this, SLOT(OnSplitterMoved(int, int)));
    GT_ASSERT(rc);

    // Add the source view to the splitter:
    m_pSourceView = new kaSourceCodeView(this, false, iContextMneuMaskSource);

    // register to the document save signals:
    rc = QObject::connect(m_pSourceView, SIGNAL(DocumentSaved(QString)), &KA_PROJECT_DATA_MGR_INSTANCE, SLOT(OnDocumentSaved(QString)));
    GT_ASSERT(rc);

    m_pSourceView->displayFile(sourceFilePath, 0 , -1);
    m_pSourceView->setProgramCounter(0, 0);
    // disable editing - only high level source view should be editable:
    m_pSourceView->setReadOnly(afGlobalVariablesManager::instance().isRunningInsideVisualStudio());
    m_pSourceView->showLineNumbers(showLineNumber);

    m_pSourceView->SetMDIFilePath(mdiFilePath);

    // register the document to the update mechanism:
    // If not running from VS notify the user that the doc needs to update:
    afDocUpdateManager::instance().RegisterDocument(m_pSourceView, sourceFilePath, this, !afGlobalVariablesManager::instance().isRunningInsideVisualStudio());

    QString kernelName;
    QString sourceSectionName;
    QString ilSectionName;

    switch (m_platformIndicator)
    {
        case kaPlatformOpenCL:
            sourceSectionName = KA_STR_sourceSectionOpenCL;
            ilSectionName = KA_STR_ILSectionOpenCL;
            break;

        case kaPlatformDirectX:
            sourceSectionName = KA_STR_sourceSectionHLSL;
            ilSectionName = KA_STR_ILSectionHLSL;
            break;

        case kaPlatformOpenGL:
        case kaPlatformVulkan:
            sourceSectionName = KA_STR_sourceSectionGLSL;
            ilSectionName = KA_STR_ILSectionGLSL;
            break;

        default:
            sourceSectionName = KA_STR_sourceSectionOpenCL;
            ilSectionName = KA_STR_ILSectionOpenCL;
            break;
    }

    AddViewToSplitter(m_pSplitter, m_pSourceView, sourceSectionName, kernelName);

    // Add the tab view:
    m_pTabWidget = new acTabWidget(this);

    m_pTabWidget->setTabPosition(QTabWidget::South);
    m_pTabWidget->setTabsClosable(true);

    m_pSplitter->addWidget(m_pTabWidget);
    // store the initial splitter ratio for hide show:
    m_splitterRatio = 1.0;

    m_pMainLayout->addWidget(m_pSplitter);
    setLayout(m_pMainLayout);

    if (0 != leftWidgetSize || 0 != rightWidgetSize)
    {
        m_widgetSizesRatio.push_back(leftWidgetSize);
        m_widgetSizesRatio.push_back(rightWidgetSize);
        m_splitterRatio = rightWidgetSize / (leftWidgetSize * 1.0);
    }

    rc = connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(OnApplicationFocusChanged(QWidget*, QWidget*)));
    GT_ASSERT(rc);

    rc = connect(m_pTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(TabCloseRequestedHandler(int)));
    GT_ASSERT(rc);

    // Create menu actions
    QString menuItem = KA_STR_menuShow;

    // ISA menu item
    menuItem += KA_STR_ISASection;
    menuItem.remove(AF_STR_HtmlBoldTagStartA);
    m_pActions[ID_ISA_VIEW_SECTION] = new QAction(menuItem, this);

    rc = connect(m_pActions[ID_ISA_VIEW_SECTION], SIGNAL(triggered()), this, SLOT(onISA()));
    GT_ASSERT(rc);

    // IL menu item
    menuItem = KA_STR_menuShow;
    menuItem += ilSectionName;
    menuItem.remove(AF_STR_HtmlBoldTagStartA);
    m_pActions[ID_IL_VIEW_SECTION] = new QAction(menuItem, this);

    rc = connect(m_pActions[ID_IL_VIEW_SECTION], SIGNAL(triggered()), this, SLOT(onIL()));
    GT_ASSERT(rc);

    // Source menu item
    menuItem = KA_STR_menuShow;
    menuItem += sourceSectionName;
    menuItem.remove(AF_STR_HtmlBoldTagStartA);
    m_pActions[ID_SOURCE_VIEW_SECTION] = new QAction(menuItem, this);

    rc = connect(m_pActions[ID_SOURCE_VIEW_SECTION], SIGNAL(triggered()), this, SLOT(onSource()));
    GT_ASSERT(rc);

    // set checkable functionality:
    for (int nAction = 0  ; nAction < ID_VIEW_SECTION_NUMBER ; nAction++)
    {
        m_pActions[nAction]->setCheckable(true);
        m_pActions[nAction]->setChecked(true);
    }

    // when the text change we need to mark the IL/ISA as modified
    rc = connect(m_pSourceView, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
    GT_ASSERT(rc);
    afBrowseAction* pExportToCSVAction = new afBrowseAction(KA_STR_exportToCSV);
    pExportToCSVAction->setEnabled(false);

    if (m_pExportToCSVAction != nullptr)
    {
        delete m_pExportToCSVAction;
        m_pExportToCSVAction = nullptr;
    }

    m_pExportToCSVAction = pExportToCSVAction;
    m_pExportToCSVAction->setText(KA_STR_exportToCSV);
    rc = connect(m_pExportToCSVAction, SIGNAL(triggered()), SLOT(OnExportToCSV()));
    GT_ASSERT(rc);
    rc = connect(m_pTabWidget, SIGNAL(currentChanged(int)), this, SLOT(UpdateDirtyViewsOnTabChange(int)));
    AddMenuItemsToSourceView(m_pSourceView);
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::resizeEvent(QResizeEvent* event)
{
    // Check if the widgets are already initialized in the main splitter for setting their initial ratio:
    QList<int> currentSizes = m_pSplitter->sizes();

    if (currentSizes[0] != 0 && !m_widgetSizesRatio.isEmpty())
    {
        SetSplitterSizeBasedOnRatios(m_pSplitter, m_widgetSizesRatio);
        // Clear the list to mark it as used (no need for first time flag)
        m_widgetSizesRatio.clear();
    }

    if (!m_isILViewHidden && !m_isISAViewHidden)
    {
        if (m_isISAViewEmpty && !m_isILViewEmpty)
        {
            //hide ISA view when it is empty and IL view is not
            onISA();
            m_isISAViewHidden = true;
            GT_IF_WITH_ASSERT(nullptr != m_pActions[ID_ISA_VIEW_SECTION])
            {
                m_pActions[ID_ISA_VIEW_SECTION]->setChecked(false);
            }
        }
        else
        {
            // in all other cases hide IL view
            onIL();
            m_isILViewHidden = true;
            GT_IF_WITH_ASSERT(nullptr != m_pActions[ID_IL_VIEW_SECTION])
            {
                m_pActions[ID_IL_VIEW_SECTION]->setChecked(false);
            }
        }
    }

    QWidget::resizeEvent(event);
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::UpdateDirtyViewsOnSubWindowChange()
{
    GT_IF_WITH_ASSERT(m_pSourceView != nullptr && !m_pSourceView->filePath().asString().isEmpty() && m_pSourceView->filePath().exists())
    {
        m_pSourceView->updateView();
        afDocUpdateManager::instance().UpdateDocument(m_pSourceView);
    }

    if (m_pTabWidget != nullptr)
    {
        int numTabs = m_pTabWidget->count();

        for (int nTab = 0; nTab < numTabs; nTab++)
        {
            kaMultiSourceTabData* pTabData = m_tabDataMap[nTab];
            GT_IF_WITH_ASSERT(nullptr != pTabData)
            {
                if (!pTabData->m_isaFilePath.asString().isEmpty() && pTabData->m_isaFilePath.exists())
                {
                    kaSourceCodeTableView* pView = pTabData->m_pISASourceTableView;

                    if (nullptr != pView)
                    {
                        if (pView->IsDirty() && m_pTabWidget->currentIndex() == nTab)
                        {
                            pView->UpdateView();
                            ShowUpdateNotUpdateCaption(false);
                        }
                    }
                }

                if (!pTabData->m_ilFilePath.asString().isEmpty() && pTabData->m_ilFilePath.exists())
                {
                    if (nullptr != pTabData->m_pILSourceView)
                    {
                        pTabData->m_pILSourceView->updateView();
                        ShowUpdateNotUpdateCaption(false);
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        kaMultiSourceView::~kaMultiSourceView
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        20/8/2013
// ---------------------------------------------------------------------------
kaMultiSourceView::~kaMultiSourceView()
{
    m_isILViewHidden = false;
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::AddView(const osFilePath& identifyFilePath, const osFilePath& isaFilePath, const osFilePath& ilFilePath, bool isGCN,  int leftWidgetSize, int rightWidgetSize)
{
    if (m_identifyPathToViewMap.find(identifyFilePath.asString()) != m_identifyPathToViewMap.end())
    {
        // Find the tab with the same name as the file name and activate it:
        int indexOfView = m_identifyPathToViewMap[identifyFilePath.asString()];
        m_pTabWidget->setCurrentIndex(indexOfView);
    }
    else
    {
        QString kernelNameQt = kaUtils::GetKernelNameFromPath(identifyFilePath);

        kaMultiSourceTabData* pTabData = new kaMultiSourceTabData;

        // Initiate information values:
        int iContextMenuReadOnlyMask = 0x1E;
        bool showLineNumber = afSourceCodeViewsManager::instance().showLineNumbers();

        kaSourceCodeView* pILSourceView = nullptr;
        kaSourceCodeTableView* pISASourceTableView = nullptr;
        kaSourceCodeView* pISASourceView = nullptr;

        // Create the views:
        QColor backgroundColor = QApplication::palette().color(QPalette::Window);

        // Check the GPU family and use kaSourceCodeTableView for GCN families
        // and kaSourceView for others
        pTabData->m_isaFilePath = isaFilePath;

        if (isGCN)
        {
            pISASourceTableView = new kaSourceCodeTableView(this);
            pISASourceTableView->setContentsMargins(0, 0, 0, 0);
            pTabData->m_pISASourceTableView = pISASourceTableView;
            pISASourceTableView->SetViewPlatform(m_platformIndicator);
            // Display the ISA text in the ISA source table view:
            pISASourceTableView->SetISAText(isaFilePath);
        }
        else
        {
            pISASourceView = new kaSourceCodeView(this, false, iContextMenuReadOnlyMask);
            pISASourceView->setReadOnly(true);
            pISASourceView->showLineNumbers(showLineNumber);
            pISASourceView->setPaper(backgroundColor);
            pISASourceView->SetMonoFont(AC_SOURCE_CODE_EDITOR_DEFAULT_FONT_SIZE);
            pTabData->m_pISASourceView = pISASourceView;
            // Display the ISA text in the ISA source view:
            pISASourceView->SetISAText(isaFilePath);

        }

        pILSourceView = new kaSourceCodeView(this, false, iContextMenuReadOnlyMask);
        pILSourceView->setReadOnly(true);
        pILSourceView->showLineNumbers(showLineNumber);
        pILSourceView->setPaper(backgroundColor);
        pILSourceView->SetMonoFont(AC_SOURCE_CODE_EDITOR_DEFAULT_FONT_SIZE);

        if (isaFilePath.isEmpty())
        {
            m_isISAViewEmpty = true;
        }

        pTabData->m_pILSourceView = pILSourceView;

        if (!ilFilePath.isEmpty())
        {
            pILSourceView->displayFile(ilFilePath, 0 , -1);
            pTabData->m_ilFilePath = ilFilePath;
        }
        else
        {
            pILSourceView->SetInternalText(KA_STR_ILNotAvailable);
            pILSourceView->showLineNumbers(false);
            m_isILViewEmpty = true;
        }

        pTabData->m_identifyPath = identifyFilePath;

        // Create the splitter and add the different views to the splitter and the splitter to the tab view:
        QSplitter* pSplitter = new QSplitter();
        bool rc = QObject::connect(pSplitter, SIGNAL(splitterMoved(int, int)), this, SLOT(OnSplitterMoved(int, int)));
        GT_ASSERT(rc);

        GT_IF_WITH_ASSERT(nullptr != pILSourceView || nullptr != pISASourceTableView || nullptr != pISASourceView)
        {
            if (nullptr != pILSourceView)
            {
                QString ilSectionName;

                switch (m_platformIndicator)
                {
                    case kaPlatformOpenCL:
                        ilSectionName = KA_STR_ILSectionOpenCL;
                        break;

                    case kaPlatformOpenGL:
                        ilSectionName = KA_STR_ILSectionGLSL;
                        break;

                    case kaPlatformDirectX:
                        ilSectionName = KA_STR_ILSectionHLSL;
                        break;

                    default:
                        ilSectionName = KA_STR_ILSectionOpenCL;
                        break;
                }

                AddViewToSplitter(pSplitter, pILSourceView, ilSectionName, kernelNameQt);
                AddMenuItemsToSourceView(pILSourceView);
            }

            if (pISASourceView != nullptr)
            {
                AddViewToSplitter(pSplitter, pISASourceView, KA_STR_ISASection, kernelNameQt);
                AddMenuItemsToSourceView(pISASourceView);
            }
            else if (pISASourceTableView != nullptr)
            {
                AddViewToSplitter(pSplitter, pISASourceTableView, KA_STR_ISASection, kernelNameQt);
                AddMenuItemsToSourceTableView(pISASourceTableView);
            }

            if (0 != leftWidgetSize || 0 != rightWidgetSize)
            {
                QList<int> widgetSizes;
                widgetSizes.push_back(leftWidgetSize);
                widgetSizes.push_back(rightWidgetSize);
                SetSplitterSizeBasedOnRatios(pSplitter, widgetSizes);
            }

            gtString fileName;
            identifyFilePath.getFileName(fileName);

            // update the initial hide/show of the views:
            // When doing the IL section it is also for the case where both sections are hidden. This is why it is enough to hide it even if
            // IL section is visible. This is to Sync with the display mechanism that works correctly if two sections are hidden then actually one of them
            // is hidden and the entire splitter is hidden:
            if (m_displayedViews[ID_IL_VIEW_SECTION] && !m_displayedViews[ID_ISA_VIEW_SECTION])
            {
                SplitterViewsToShow(pSplitter, &pTabData->m_splitterRatio, 1, false);
            }
            else if (!m_displayedViews[ID_IL_VIEW_SECTION])
            {
                SplitterViewsToShow(pSplitter, &pTabData->m_splitterRatio, 0, false);
            }

            // Set the initial ratio to (since the initial ratio might be overwritten by the initial hide/show)
            pTabData->m_splitterRatio = 1.0 * rightWidgetSize / leftWidgetSize;

            int newIndex = m_pTabWidget->addTab(pSplitter, acGTStringToQString(fileName));
            m_identifyPathToViewMap[identifyFilePath.asString()] = newIndex;
            m_pTabWidget->setCurrentIndex(newIndex);
            m_tabDataMap[newIndex] = pTabData;
        }
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::SaveAs()
{
    kaSourceCodeView* pView = nullptr;
    QAction* pAction = static_cast<QAction*>(sender());

    // check if the action is for source code view
    if (nullptr != m_pSourceView &&
        m_pSourceView->IsActionOfThisView(pAction))
    {
        pView = m_pSourceView;
    }
    else
    {
        kaMultiSourceTabData* pTabData = m_tabDataMap[m_pTabWidget->currentIndex()];
        GT_IF_WITH_ASSERT(nullptr != pTabData)
        {
            // check if the action from m_ILSourceView
            if ((nullptr != pTabData->m_pILSourceView) && !pTabData->m_ilFilePath.asString().isEmpty())
            {
                if (pTabData->m_pILSourceView->IsActionOfThisView(pAction))
                {
                    pView = pTabData->m_pILSourceView;
                }
            }

            // check if the action from m_ILSourceView
            if ((nullptr != pTabData->m_pISASourceView) && !pTabData->m_ilFilePath.asString().isEmpty())
            {
                if (pTabData->m_pISASourceView->IsActionOfThisView(pAction))
                {
                    pView = pTabData->m_pISASourceView;
                }
            }
        }
    }

    GT_IF_WITH_ASSERT(pView != nullptr)
    {
        FileSaveAs(pView);
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::OnExportToCSV()
{
    kaSourceCodeTableView* pISATableView = nullptr;
    kaMultiSourceTabData* pTabData = m_tabDataMap[m_pTabWidget->currentIndex()];
    GT_IF_WITH_ASSERT(nullptr != pTabData)
    {
        if ((nullptr != pTabData->m_pISASourceTableView) && !pTabData->m_isaFilePath.asString().isEmpty())
        {
            pISATableView = pTabData->m_pISASourceTableView;
        }
    }

    if (pISATableView != nullptr)
    {
        QString defaultFileFullPath(acGTStringToQString(pISATableView->ISAFilePath().asString()));
        // Build the CSV default file name:
        gtString gtNewFileName;
        osDirectory fileDir;
        pISATableView->ISAFilePath().getFileName(gtNewFileName);
        //get file directory name, i.e. shader/kernel name
        pISATableView->ISAFilePath().getFileDirectory(fileDir);
        osFilePath containingPath;
        containingPath = fileDir.directoryPath();
        gtString strFileDir = containingPath.asString();
        QFileInfo dirInfo(acGTStringToQString(strFileDir));
        QString dirName = dirInfo.baseName();
        dirName.prepend(AF_STR_Hyphen);
        dirName.append(AF_STR_Hyphen);
        gtNewFileName.prepend(acQStringToGTString(dirName));
        QDateTime dateTime = dateTime.currentDateTime();
        QString dateTimeString = dateTime.toString("yyyyMMdd-hhmm");
        dateTimeString.prepend(AF_STR_Hyphen);
        gtNewFileName << acQStringToGTString(dateTimeString);
        QString csvFileName = acGTStringToQString(afProjectManager::instance().currentProjectSettings().projectName());
        csvFileName.append(AF_STR_HyphenA);
        csvFileName.append(afGlobalVariablesManager::ProductNameA());
        csvFileName.append(acGTStringToQString(gtNewFileName));
        csvFileName.append(AF_STR_saveCSVFilePostfix);
        QString csvFilters = acGTStringToQString(AF_STR_csvFileDetails);
        afApplicationCommands* pAppCommands = afApplicationCommands::instance();
        GT_IF_WITH_ASSERT(pAppCommands != nullptr)
        {
            QString loadFileName = pAppCommands->ShowFileSelectionDialog(KA_STR_exportToCSV, csvFileName, csvFilters, m_pExportToCSVAction, true);

            if (!loadFileName.isEmpty())
            {
                gtString file = acQStringToGTString(loadFileName);
                pISATableView->ExportToCSV(file);
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::ShowLineNumbers(bool show)
{
    QAction* pAction = static_cast<QAction*>(sender());

    if (nullptr != m_pSourceView)
    {
        // show/hide line numbers only if the action belong to this view
        if (m_pSourceView->IsActionOfThisView(pAction))
        {
            m_pSourceView->showLineNumbers(show);
        }
    }

    int numTabs = m_pTabWidget->count();

    for (int nTab = 0 ; nTab < numTabs ; nTab++)
    {
        kaMultiSourceTabData* pTabData = m_tabDataMap[nTab];
        GT_IF_WITH_ASSERT(nullptr != pTabData)
        {
            if ((nullptr != pTabData->m_pILSourceView) && !pTabData->m_ilFilePath.asString().isEmpty())
            {
                // show/hide line numbers only if the action belong to this view
                if (pTabData->m_pILSourceView->IsActionOfThisView(pAction))
                {
                    pTabData->m_pILSourceView->showLineNumbers(show);
                }
            }

            if ((nullptr != pTabData->m_pISASourceView) && !pTabData->m_isaFilePath.asString().isEmpty())
            {
                if (pTabData->m_pISASourceView->IsActionOfThisView(pAction))
                {
                    pTabData->m_pISASourceView->showLineNumbers(show);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::AddViewToSplitter(QSplitter* pSplitter, QWidget* pView, QString caption, const QString& kernelName)
{
    if (nullptr != pSplitter)
    {
        // Create the objects:
        QWidget* pWidget = new QWidget;

        QVBoxLayout* pVLayOut = new QVBoxLayout();

        pVLayOut->setContentsMargins(0, 0, 0, 0);

        if (!kernelName.isEmpty())
        {
            caption += " - " + kernelName;
        }

        if (m_isNotUpToDateShown)
        {
            caption += KA_STR_viewNotUpdated;
        }

        QLabel* pText = new QLabel(caption, this);
        pText->setObjectName(KA_STR_updatedLabel);

        // Organize everything:
        pVLayOut->addWidget(pText);

        // second parameter stretches the widget
        pVLayOut->addWidget(pView, 1, 0);

        pWidget->setLayout(pVLayOut);
        pSplitter->addWidget(pWidget);
    }
}


// ---------------------------------------------------------------------------
void kaMultiSourceView::onSource()
{
    m_displayedViews[ID_SOURCE_VIEW_SECTION] = !m_displayedViews[ID_SOURCE_VIEW_SECTION];

    if (nullptr != m_pSourceView)
    {
        SplitterViewsToShow(m_pSplitter, &m_splitterRatio, 0, m_displayedViews[ID_SOURCE_VIEW_SECTION]);
    }

    enableActions();
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::onIL()
{
    m_displayedViews[ID_IL_VIEW_SECTION] = !m_displayedViews[ID_IL_VIEW_SECTION];

    // If it is time to show the IL section and the tab control is hidden show it
    if (m_displayedViews[ID_IL_VIEW_SECTION] && !m_displayedViews[ID_ISA_VIEW_SECTION])
    {
        SplitterViewsToShow(m_pSplitter, &m_splitterRatio, 1, true);

        // Ensure the IL section is visible (flipping visibility if needed:
        TabSplitterEnsureSideVisiblity(0);

        // Ensure only the ISA is hidden on the right side:
        TabSplitterViewsToShow(1, m_displayedViews[ID_ISA_VIEW_SECTION]);
    }
    else
    {
        // If both IL and ISA are now hidden also hide the source control
        if (!m_displayedViews[ID_IL_VIEW_SECTION] && !m_displayedViews[ID_ISA_VIEW_SECTION])
        {
            SplitterViewsToShow(m_pSplitter, &m_splitterRatio, 1, false);
        }
        else
        {
            // Go through all the tab views and hide the left side of the splitters view there
            TabSplitterViewsToShow(0, m_displayedViews[ID_IL_VIEW_SECTION]);
        }
    }

    enableActions();
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::onISA()
{
    m_displayedViews[ID_ISA_VIEW_SECTION] = !m_displayedViews[ID_ISA_VIEW_SECTION];

    // If it is time to show the ISA section and the tab control is hidden show it
    if (m_displayedViews[ID_ISA_VIEW_SECTION] && !m_displayedViews[ID_IL_VIEW_SECTION])
    {
        SplitterViewsToShow(m_pSplitter, &m_splitterRatio, 1, true);

        // Ensure the IL section is visible (flipping visibility if needed:
        TabSplitterEnsureSideVisiblity(1);

        // Ensure only the ISA is hidden on the right side:
        TabSplitterViewsToShow(0, m_displayedViews[ID_IL_VIEW_SECTION]);
    }
    else
    {
        // If both IL and ISA are now hidden also hide the source control
        if (!m_displayedViews[ID_IL_VIEW_SECTION] && !m_displayedViews[ID_ISA_VIEW_SECTION])
        {
            SplitterViewsToShow(m_pSplitter, &m_splitterRatio, 1, false);
        }
        else
        {
            // Go through all the tab views and hide the left side of the splitters view there
            TabSplitterViewsToShow(1, m_displayedViews[ID_ISA_VIEW_SECTION]);
        }
    }

    enableActions();
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::enableActions()
{
    int numViews = 0;
    int visibleView = -1;

    // if there is only one visible view disable its action:
    for (int nView = 0 ; nView < ID_VIEW_SECTION_NUMBER ; nView++)
    {
        if (m_displayedViews[nView])
        {
            numViews++;
            visibleView = nView;
        }
    }

    if (numViews == 1)
    {
        if (m_pActions[visibleView] != nullptr)
        {
            m_pActions[visibleView]->setEnabled(false);
        }
    }
    else
    {
        // if more then one is visible make sure all actions are enabled:
        for (int nView = 0 ; nView < ID_VIEW_SECTION_NUMBER ; nView++)
        {
            if (m_pActions[nView] != nullptr)
            {
                m_pActions[nView]->setEnabled(true);
            }
        }
    }

    if (m_platformIndicator == kaPlatformOpenGL)
    {
        if (m_pActions[ID_IL_VIEW_SECTION] != nullptr)
        {
            m_pActions[ID_IL_VIEW_SECTION]->setChecked(false);
        }

        if (m_pActions[ID_IL_VIEW_SECTION] != nullptr)
        {
            m_pActions[ID_IL_VIEW_SECTION]->setEnabled(false);
        }
    }
}

// ---------------------------------------------------------------------------
bool kaMultiSourceView::updateView(bool selectedView)
{
    bool retVal = true;

    GT_IF_WITH_ASSERT(m_pSourceView != nullptr && !m_pSourceView->filePath().asString().isEmpty() && m_pSourceView->filePath().exists())
    {
        m_pSourceView->updateView();
        afDocUpdateManager::instance().UpdateDocument(m_pSourceView);
    }
    else
    {
        retVal = false;
    }

    if (m_pTabWidget != nullptr)
    {
        int numTabs = m_pTabWidget->count();

        for (int nTab = 0; nTab < numTabs; nTab++)
        {
            // even if there is only ISA or IL the tab is left open (for example CPU)
            bool shouldCloseTab = true;

            kaMultiSourceTabData* pTabData = m_tabDataMap[nTab];
            GT_IF_WITH_ASSERT(nullptr != pTabData)
            {
                if (!pTabData->m_isaFilePath.asString().isEmpty() && pTabData->m_isaFilePath.exists())
                {
                    kaSourceCodeTableView* pView = pTabData->m_pISASourceTableView;

                    if (nullptr != pView)
                    {
                        pView->SetDirty(true);

                        if (selectedView && m_pTabWidget->currentIndex() == nTab)
                        {
                            pView->UpdateView();
                        }

                        shouldCloseTab = false;
                    }
                }

                if (!pTabData->m_ilFilePath.asString().isEmpty() && pTabData->m_ilFilePath.exists())
                {
                    if (nullptr != pTabData->m_pILSourceView)
                    {
                        pTabData->m_pILSourceView->updateView();
                        shouldCloseTab = false;
                    }
                }
            }

            if (shouldCloseTab)
            {
                // Clear the stored find view if it was removed:
                TabCloseRequestedHandler(nTab);
                nTab--;
                numTabs--;
            }
        }

        ShowUpdateNotUpdateCaption(false);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::onUpdateEdit_Copy(bool& isEnabled)
{
    isEnabled = false;
    QWidget* pFocusedView = focusedView();
    kaSourceCodeView* pSourceView = qobject_cast<kaSourceCodeView*>(pFocusedView);

    if (pSourceView != nullptr)
    {
        isEnabled = !pSourceView->selectedText().isEmpty();
    }

    kaSourceCodeTableView* pSourceTableView = qobject_cast<kaSourceCodeTableView*>(pFocusedView);

    if (pSourceTableView != nullptr)
    {
        isEnabled = pSourceTableView->HasSelectedItems();
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::onUpdateEdit_SelectAll(bool& isEnabled)
{
    isEnabled = false;
    QWidget* pFocusedView = focusedView();
    kaSourceCodeView* pSourceView = qobject_cast<kaSourceCodeView*>(pFocusedView);

    if (pSourceView != nullptr)
    {
        isEnabled = !pSourceView->selectedText().isEmpty();
    }

    kaSourceCodeTableView* pSourceTableView = qobject_cast<kaSourceCodeTableView*>(pFocusedView);

    if (pSourceTableView != nullptr)
    {
        isEnabled = pSourceTableView->ContainsData();
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::onUpdateEdit_Find(bool& isEnabled)
{
    isEnabled = false;
    QWidget* pFocusedView = focusedView();
    kaSourceCodeView* pSourceView = qobject_cast<kaSourceCodeView*>(pFocusedView);

    if (pSourceView != nullptr)
    {
        isEnabled = !pSourceView->text().isEmpty();
    }
    else
    {
        isEnabled = true;
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::onUpdateEdit_FindNext(bool& isEnabled)
{
    onUpdateEdit_Find(isEnabled);
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::onEdit_Copy()
{
    QWidget* pFocusedView = focusedView();
    kaSourceCodeView* pSourceView = qobject_cast<kaSourceCodeView*>(pFocusedView);

    if (pSourceView != nullptr)
    {
        pSourceView->onCopy();
    }

    kaSourceCodeTableView* pSourceTableView = qobject_cast<kaSourceCodeTableView*>(pFocusedView);

    if (pSourceTableView != nullptr)
    {
        pSourceTableView->OnCopy();
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::onEdit_SelectAll()
{
    QWidget* pFocusedView = focusedView();
    kaSourceCodeView* pSourceView = qobject_cast<kaSourceCodeView*>(pFocusedView);

    if (pSourceView != nullptr)
    {
        pSourceView->onSelectAll();
    }

    kaSourceCodeTableView* pSourceTableView = qobject_cast<kaSourceCodeTableView*>(pFocusedView);

    if (pSourceTableView != nullptr)
    {
        pSourceTableView->OnSelectAll();
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::onEdit_Find()
{
    GT_IF_WITH_ASSERT(m_pFindSourceCodeView != nullptr)
    {
        kaSourceCodeView* pSourceView = qobject_cast<kaSourceCodeView*>(m_pFindSourceCodeView);

        if (pSourceView != nullptr)
        {
            pSourceView->onFindClick();
        }

        kaSourceCodeTableView* pSourceTableView = qobject_cast<kaSourceCodeTableView*>(m_pFindSourceCodeView);

        if (pSourceTableView != nullptr)
        {
            pSourceTableView->OnFindClick();
        }
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::onEdit_FindNext()
{
    GT_IF_WITH_ASSERT(m_pFindSourceCodeView != nullptr)
    {
        kaSourceCodeView* pSourceView = qobject_cast<kaSourceCodeView*>(m_pFindSourceCodeView);

        if (pSourceView != nullptr)
        {
            pSourceView->onFindNext();
        }

        kaSourceCodeTableView* pSourceTableView = qobject_cast<kaSourceCodeTableView*>(m_pFindSourceCodeView);

        if (pSourceTableView != nullptr)
        {
            pSourceTableView->OnFindNext();
        }
    }
}

// ---------------------------------------------------------------------------
QWidget* kaMultiSourceView::focusedView()
{
    QWidget* pRetVal = nullptr;

    if (m_pSourceView != nullptr)
    {
        if (m_pSourceView->hasFocus())
        {
            pRetVal = m_pSourceView;
        }
    }

    if (nullptr == pRetVal)
    {
        int numTabs = m_pTabWidget->count();

        for (int nTab = 0 ; nTab < numTabs ; nTab++)
        {
            kaMultiSourceTabData* pTabData = m_tabDataMap[nTab];
            GT_IF_WITH_ASSERT(nullptr != pTabData)
            {
                if (nullptr != pTabData->m_pILSourceView)
                {
                    if (pTabData->m_pILSourceView->hasFocus())
                    {
                        pRetVal = pTabData->m_pILSourceView;
                    }
                }

                if (nullptr != pTabData->m_pISASourceTableView)
                {
                    if (pTabData->m_pISASourceTableView->IsTableInFocus())
                    {
                        pRetVal = pTabData->m_pISASourceTableView;
                    }
                }
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::storeFindClickedView()
{
    // Store the focused view:
    m_pFindSourceCodeView  = focusedView();
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::WriteDataFileString(gtString& fileString)
{
    QList<int> widgetSizes = m_pSplitter->sizes();
    fileString.appendFormattedString(L"%d,%d\n", widgetSizes[0], widgetSizes[1]);

    int numTabs = m_pTabWidget->count();

    for (int nTab = 0 ; nTab < numTabs ; nTab++)
    {
        kaMultiSourceTabData* pTabData = m_tabDataMap[nTab];
        GT_IF_WITH_ASSERT(nullptr != pTabData)
        {
            fileString.appendFormattedString(L"%ls,", m_pTabWidget->tabText(nTab).toStdWString().c_str());
            fileString.appendFormattedString(L"%ls,", pTabData->m_identifyPath.asString().asCharArray());
            fileString.appendFormattedString(L"%ls,", pTabData->m_isaFilePath.asString().asCharArray());
            fileString.appendFormattedString(L"%ls,", pTabData->m_ilFilePath.asString().asCharArray());

            QSplitter* pSplitter = qobject_cast<QSplitter*>(m_pTabWidget->widget(nTab));
            GT_IF_WITH_ASSERT(nullptr != pSplitter)
            {
                widgetSizes = pSplitter->sizes();
                fileString.appendFormattedString(L"%d,%d\n", widgetSizes[0], widgetSizes[1]);
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::OnApplicationFocusChanged(QWidget* pOld, QWidget* pNew)
{
    (void)(pOld); // unused

    // When the keyboard focus changes to one of the source code views, we want to set this view as
    // the find dialog handler:
    if (pNew != nullptr)
    {
        // Check if the user switched focus to one of the source code views:
        if (pNew == focusedView())
        {
            // If we switch to focus on one of the source code views,
            // we should move the find parameters from the previous find source code view
            // to the new one:
            if (m_pFindSourceCodeView != pNew)
            {
                m_pFindSourceCodeView = pNew;
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::onTextChanged()
{
    ShowUpdateNotUpdateCaption(true);
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::TabCloseRequestedHandler(int index)
{
    bool indexFound = false;
    // find the view in the map and remove it:
    gtMap<gtString, int>::iterator mapIterator;

    for (mapIterator = m_identifyPathToViewMap.begin() ; mapIterator != m_identifyPathToViewMap.end() ; mapIterator++)
    {
        if ((*mapIterator).second == index)
        {
            QWidget* pWidgetToBeRemoved = m_pTabWidget->widget(index);
            m_pTabWidget->removeTab(index);
            m_identifyPathToViewMap.erase(mapIterator);

            delete pWidgetToBeRemoved;

            // remove the tabData associated with the tab:
            gtMap<int, kaMultiSourceTabData*>::iterator tabDataIterator = m_tabDataMap.find(index);
            GT_IF_WITH_ASSERT(tabDataIterator != m_tabDataMap.end())
            {
                kaMultiSourceTabData* pTabData = (*tabDataIterator).second;

                if (nullptr != pTabData && pTabData->m_pILSourceView == m_pFindSourceCodeView /*|| pTabData->m_ISASourceView == m_pFindSourceCodeView*/)
                {
                    m_pFindSourceCodeView = nullptr;
                }

                m_tabDataMap.erase(tabDataIterator);
                GT_IF_WITH_ASSERT(nullptr != pTabData)
                {
                    delete pTabData;
                }
            }

            indexFound = true;
            break;

        }
    }

    if (indexFound)
    {
        // need to pass through the tabs indexes from low to high:
        gtMap<int, gtString> reversePathToViewMap;
        gtMap<int, gtString>::iterator reverseMapIt;

        for (mapIterator = m_identifyPathToViewMap.begin() ; mapIterator != m_identifyPathToViewMap.end() ; mapIterator++)
        {
            if ((*mapIterator).second > index)
            {
                reversePathToViewMap[(*mapIterator).second] = (*mapIterator).first;
            }
        }

        // Now in the reversePathToViewMap we have only the indexes that are higher and in ascending order
        // go through all new map iterators and update all indexes higher then current index
        for (reverseMapIt = reversePathToViewMap.begin() ; reverseMapIt != reversePathToViewMap.end() ; reverseMapIt++)
        {
            gtString& path = (*reverseMapIt).second;

            // after removing a tab reduce the index of all the following tabs:
            int oldIndex = m_identifyPathToViewMap[path];
            m_identifyPathToViewMap[path] = oldIndex - 1;

            // and need to remove the tab data object and put it under the new index:
            gtMap<int, kaMultiSourceTabData*>::iterator tabDataIterator = m_tabDataMap.find(oldIndex);
            GT_IF_WITH_ASSERT(tabDataIterator != m_tabDataMap.end())
            {
                // Remove the old tab data with the old index:
                kaMultiSourceTabData* pTabData = (*tabDataIterator).second;
                m_tabDataMap.erase(tabDataIterator);
                GT_IF_WITH_ASSERT(nullptr != pTabData)
                {
                    // Add it with the new index:
                    m_tabDataMap[oldIndex - 1] = pTabData;
                }
            }
        }

        GT_IF_WITH_ASSERT(nullptr != m_pParentKernelView)
        {
            m_pParentKernelView->writeDataFile();
        }
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::AddMenuItemsToSourceView(kaSourceCodeView* pView)
{
    GT_IF_WITH_ASSERT(nullptr != pView)
    {
        QAction* pShowLineNumbersAction = new QAction(AF_STR_sourceCodeShowLineNumbers, this);
        bool rc = connect(pShowLineNumbersAction, SIGNAL(toggled(bool)), this, SLOT(ShowLineNumbers(bool)));
        GT_ASSERT(rc);
        pShowLineNumbersAction->setCheckable(true);
        pShowLineNumbersAction->setChecked(true);
        pView->addSeparator();
        pView->addMenuAction(pShowLineNumbersAction);
        pView->addMenuAction(m_pActions[ID_ISA_VIEW_SECTION]);
        pView->addMenuAction(m_pActions[ID_IL_VIEW_SECTION]);
        pView->addMenuAction(m_pActions[ID_SOURCE_VIEW_SECTION]);

        if (m_platformIndicator == kaPlatformOpenGL)
        {
            if (m_pActions[ID_IL_VIEW_SECTION] != nullptr)
            {
                m_pActions[ID_IL_VIEW_SECTION]->setChecked(false);
            }

            if (m_pActions[ID_IL_VIEW_SECTION] != nullptr)
            {
                m_pActions[ID_IL_VIEW_SECTION]->setEnabled(false);
            }
        }

        // add action last in menu
        QAction* pSaveAsAction = new QAction(AF_STR_sourceCodeSaveAs, this);
        pView->addMenuAction(pSaveAsAction, false);

        rc = connect(pSaveAsAction, SIGNAL(triggered()), this, SLOT(SaveAs()));
        GT_ASSERT(rc);

        // add separator last in menu
        pView->addSeparator(false);

        // add action last in menu
        pView->addMenuAction(m_pExportToCSVAction, false);
    }
}

void kaMultiSourceView::AddMenuItemsToSourceTableView(kaSourceCodeTableView* pView)
{
    GT_IF_WITH_ASSERT(nullptr != pView)
    {
        pView->AddSeparator();
        pView->AddMenuAction(m_pActions[ID_ISA_VIEW_SECTION]);
        pView->AddMenuAction(m_pActions[ID_IL_VIEW_SECTION]);
        pView->AddMenuAction(m_pActions[ID_SOURCE_VIEW_SECTION]);

        if (m_platformIndicator == kaPlatformOpenGL)
        {
            if (m_pActions[ID_IL_VIEW_SECTION] != nullptr)
            {
                m_pActions[ID_IL_VIEW_SECTION]->setChecked(false);
            }

            if (m_pActions[ID_IL_VIEW_SECTION] != nullptr)
            {
                m_pActions[ID_IL_VIEW_SECTION]->setEnabled(false);
            }
        }

        pView->AddSeparator(false);
        // add action last in menu
        pView->AddMenuAction(m_pExportToCSVAction, false);
        m_pExportToCSVAction->setEnabled(true);
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::TabSplitterViewsToShow(int sideToDisplay, bool show)
{
    // Go through all the tabs and get the splitter:
    int numTabs = m_pTabWidget->count();

    for (int nTab = 0 ; nTab < numTabs ; nTab++)
    {
        QSplitter* pSplitter = qobject_cast<QSplitter*>(m_pTabWidget->widget(nTab));
        kaMultiSourceTabData* pTabData =  m_tabDataMap[nTab];
        GT_IF_WITH_ASSERT(nullptr != pSplitter && nullptr != pTabData)
        {
            QList<int> widgetSizes = pSplitter->sizes();

            // The splitter can have only one view (in case there is only ISA or IL
            if (widgetSizes.count() == 2)
            {
                SplitterViewsToShow(pSplitter, &pTabData->m_splitterRatio, sideToDisplay, show);
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::TabSplitterEnsureSideVisiblity(int sideToDisplay)
{
    // Go through all the tabs and get the splitter:
    int numTabs = m_pTabWidget->count();

    for (int nTab = 0 ; nTab < numTabs ; nTab++)
    {
        QSplitter* pSplitter = qobject_cast<QSplitter*>(m_pTabWidget->widget(nTab));
        kaMultiSourceTabData* pTabData =  m_tabDataMap[nTab];
        GT_IF_WITH_ASSERT(nullptr != pSplitter && nullptr != pTabData)
        {
            QList<int> widgetSizes = pSplitter->sizes();

            // The splitter can have only one view (in case there is only ISA or IL
            if (widgetSizes.count() == 2)
            {
                if (0 == widgetSizes[sideToDisplay])
                {
                    widgetSizes[sideToDisplay] = widgetSizes[1 - sideToDisplay];
                    widgetSizes[1 - sideToDisplay] = 0;
                    pSplitter->setSizes(widgetSizes);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::SplitterViewsToShow(QSplitter* pSplitter, float* pRatio, int sideToDisplay, bool show)
{
    GT_IF_WITH_ASSERT(nullptr != pSplitter && nullptr != pRatio && (0 == sideToDisplay || 1 == sideToDisplay))
    {
        QList<int> widgetSizes = pSplitter->sizes();

        if (widgetSizes.count() == 2)
        {
            if (show)
            {
                // if both sides are hidden
                if (0 == *pRatio)
                {
                    widgetSizes[sideToDisplay] = pSplitter->width();
                    widgetSizes[1 - sideToDisplay] = 0;
                }
                else
                {
                    // calculate the size of the widget based on the old ratio that was passed
                    float total = widgetSizes[0] + widgetSizes[1];
                    widgetSizes[0] = (int)(total / (1.0 + *pRatio));
                    widgetSizes[1] = (int)(total - widgetSizes[0]);
                }

                // Set the sizes
                pSplitter->setSizes(widgetSizes);
            }
            else
            {
                // save the ratio ignore when ratio is 0, do not store it
                if ((0 != widgetSizes[0]) && (0 != widgetSizes[1]))
                {
                    *pRatio =  widgetSizes[1] / (1.0 * widgetSizes[0]);
                }

                // hide the needed side
                widgetSizes[1 - sideToDisplay] = pSplitter->width();
                widgetSizes[sideToDisplay] = 0;
                // set the new sizes
                pSplitter->setSizes(widgetSizes);
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::UpdateDocument(const osFilePath& docToUpdate)
{
    GT_UNREFERENCED_PARAMETER(docToUpdate);

    if (nullptr != m_pSourceView)
    {
        m_pSourceView->UpdateFile();
    }

    ShowUpdateNotUpdateCaption(true);

}

// ---------------------------------------------------------------------------
void kaMultiSourceView::MarkKernelLine(int sourceLine)
{
    if (nullptr != m_pSourceView)
    {
        m_pSourceView->setCursorPosition(sourceLine, 0);
        m_pSourceView->markerAdd(sourceLine, QsciScintilla::RightTriangle);
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::UnregisterDocument()
{
    afDocUpdateManager::instance().UnregisterDocumentOfWidget(m_pSourceView);
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::FileSave()
{
    if (nullptr != m_pSourceView)
    {
        // Save the file:
        m_pSourceView->saveFile();
        afApplicationCommands::instance()->MarkMDIWindowAsChanged(m_mdiFilePath, false);

        // update the document to the update mechanism:
        afDocUpdateManager::instance().UpdateDocument(m_pSourceView);
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::FileSaveAs(kaSourceCodeView* pView)
{
    if (nullptr != pView)
    {
        // open dialog window
        osFilePath origFilePath = pView->filePath();
        afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();

        QString dialogCaption(AF_STR_saveDataDialogHeaderA);
        QString defaultFileFullPath(acGTStringToQString(origFilePath.asString()));
        QString fileFilters(AF_STR_allFileDetails);

        QString newFileName = pApplicationCommands->ShowFileSelectionDialog(dialogCaption, defaultFileFullPath, fileFilters, nullptr, true);

        // save to file
        if (!newFileName.isEmpty())
        {
            gtString gtNewFileName = acQStringToGTString(newFileName);
            pView->saveFileAs(gtNewFileName);
        }
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::FileSaveAs()
{
    FileSaveAs(m_pSourceView);
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::ShowUpdateNotUpdateCaption(bool showCaption)
{
    // update all views only if the mode is different then the one already set
    if (showCaption != m_isNotUpToDateShown)
    {
        int numTabs = m_pTabWidget->count();

        for (int nTab = 0; nTab < numTabs; nTab++)
        {
            kaMultiSourceTabData* pTabData = m_tabDataMap[nTab];
            GT_IF_WITH_ASSERT(nullptr != pTabData)
            {
                if (nullptr != pTabData->m_pILSourceView)
                {
                    ShowUpdateNotUpdateCaptionInSplitter(pTabData->m_pILSourceView, showCaption);
                }

                if (nullptr != pTabData->m_pISASourceTableView)
                {
                    ShowUpdateNotUpdateCaptionInSplitter(pTabData->m_pISASourceTableView, showCaption);
                }
            }
        }

        m_isNotUpToDateShown = showCaption;
    }
}

// ---------------------------------------------------------------------------
void kaMultiSourceView::ShowUpdateNotUpdateCaptionInSplitter(QWidget* pWidget, bool showCaption)
{
    // The widget is places in a layout inside a widget with the label above it
    // the best way without storing all labels is to get the parent widget (no need to get the layout)
    // and then ask the parent widget for the label which we tagged with the name KA_STR_updatedLabel
    GT_IF_WITH_ASSERT(nullptr != pWidget)
    {
        // Get the parent widget
        QWidget* pParent = pWidget->parentWidget();

        if (nullptr != pParent)
        {
            // Get the label widget inside:
            QLabel* pLabel = pParent->findChild<QLabel*>(KA_STR_updatedLabel);

            if (nullptr != pLabel)
            {
                QString labelCaption = pLabel->text();
                bool captionExists = (labelCaption.indexOf(KA_STR_viewNotUpdated) != -1);

                if (showCaption)
                {
                    // Add the caption
                    if (!captionExists)
                    {
                        pLabel->setText(labelCaption + KA_STR_viewNotUpdated);
                    }
                }
                else
                {
                    // remove the caption
                    if (captionExists)
                    {
                        labelCaption.remove(KA_STR_viewNotUpdated);
                        pLabel->setText(labelCaption);
                    }
                }
            }
        }
    }
}

void kaMultiSourceView::SetSplitterSizeBasedOnRatios(QSplitter* pSplitter, const QList<int>& ratioList)
{
    GT_IF_WITH_ASSERT(nullptr != pSplitter)
    {
        int numWidgets = pSplitter->count();

        // ratio list must include the same number of sections as number of widgets. otherwise we can't set the ratio correctly of each widget
        GT_IF_WITH_ASSERT(ratioList.size() == numWidgets)
        {
            // Get the total size of the widgets and total ratios
            QList<int> currentSize = pSplitter->sizes();
            int totalSize = pSplitter->width();
            int totalRatios = 0;

            for (int nWidget = 0; nWidget < numWidgets; nWidget++)
            {
                totalRatios += ratioList[nWidget];
            }

            GT_IF_WITH_ASSERT(totalRatios != 0)
            {
                // create the new size list based on the ratios
                QList<int> newSize;
                newSize.reserve(numWidgets);

                for (int nWidget = 0; nWidget < numWidgets; nWidget++)
                {
                    newSize.push_back((int)(totalSize * ratioList[nWidget] * 1.0 / totalRatios));
                }

                pSplitter->setSizes(newSize);
            }
        }
    }
}

void kaMultiSourceView::OnSplitterMoved(int pos, int index)
{
    GT_UNREFERENCED_PARAMETER(pos);
    GT_UNREFERENCED_PARAMETER(index);

    QSplitter* pSplitter = qobject_cast<QSplitter*>(sender());

    GT_IF_WITH_ASSERT(pSplitter != nullptr)
    {
        QList<int> currentSize = pSplitter->sizes();

        // Handle the main splitter action
        if (pSplitter == m_pSplitter)
        {
            // store the ratio
            if (currentSize[0] != 0 && currentSize[1] != 0)
            {
                m_splitterRatio = 1.0 * currentSize[1] / currentSize[0];
                // update the visibility flags when we are setting the tab visible based on the tab status
                QSplitter* pTabSplitter = qobject_cast<QSplitter*>(m_pTabWidget->widget(0));

                if (pTabSplitter != nullptr)
                {
                    QList<int> tabSplitterSize = pTabSplitter->sizes();

                    if (tabSplitterSize[0] != 0)
                    {
                        m_displayedViews[ID_IL_VIEW_SECTION] = true;
                    }

                    if (tabSplitterSize[1] != 0)
                    {
                        m_displayedViews[ID_ISA_VIEW_SECTION] = true;
                    }
                }

                m_displayedViews[ID_SOURCE_VIEW_SECTION] = true;
            }
            else
            {
                if (currentSize[1] == 0)
                {
                    // update the visibility flags when we are setting the tab invisible
                    m_displayedViews[ID_IL_VIEW_SECTION] = false;
                    m_displayedViews[ID_ISA_VIEW_SECTION] = false;
                }
                else
                {
                    m_displayedViews[ID_SOURCE_VIEW_SECTION] = false;
                }
            }
        }

        // check if it is one of the tab splitter
        int numTabs = m_pTabWidget->count();

        for (int nTab = 0; nTab < numTabs; nTab++)
        {
            QSplitter* pTabSplitter = qobject_cast<QSplitter*>(m_pTabWidget->widget(nTab));
            kaMultiSourceTabData* pTabData = m_tabDataMap[nTab];

            if (pTabSplitter == pSplitter && pTabData != nullptr)
            {
                QList<int> currentTabSize = pTabSplitter->sizes();

                // store the ratio
                if (currentTabSize[0] != 0 && currentTabSize[1] != 0)
                {
                    pTabData->m_splitterRatio = 1.0 * currentTabSize[1] / currentTabSize[0];
                    int tabToRestore = !m_displayedViews[ID_IL_VIEW_SECTION] ? 0 : -1;

                    if (!m_displayedViews[ID_ISA_VIEW_SECTION])
                    {
                        tabToRestore = 1;
                    }

                    m_displayedViews[ID_IL_VIEW_SECTION] = true;
                    m_displayedViews[ID_ISA_VIEW_SECTION] = true;

                    if (tabToRestore != -1)
                    {
                        TabSplitterViewsToShow(tabToRestore, m_displayedViews[tabToRestore == 0 ? ID_IL_VIEW_SECTION : ID_ISA_VIEW_SECTION]);
                    }
                }
                else
                {
                    m_displayedViews[ID_IL_VIEW_SECTION] = !(currentTabSize[0] == 0);
                    m_displayedViews[ID_ISA_VIEW_SECTION] = !(currentTabSize[1] == 0);

                    if (currentTabSize[0] == 0)
                    {
                        TabSplitterViewsToShow(0, m_displayedViews[ID_IL_VIEW_SECTION]);
                    }
                    else
                    {
                        TabSplitterViewsToShow(1, m_displayedViews[ID_ISA_VIEW_SECTION]);
                    }
                }

                break;
            }
        }

        // enable and check the commands based on the displayed views (need to be done since the actions are not executed:
        for (int nAction = 0; nAction < ID_VIEW_SECTION_NUMBER; nAction++)
        {
            m_pActions[nAction]->setChecked(m_displayedViews[nAction]);
        }

        enableActions();
    }
}

void kaMultiSourceView::UpdateDirtyViewsOnTabChange(int nCurrentIndex)
{
    if (m_tabDataMap.find(nCurrentIndex) != m_tabDataMap.end())
    {
        kaMultiSourceTabData* pTabData = m_tabDataMap[nCurrentIndex];

        if (nullptr != pTabData)
        {
            if (!pTabData->m_isaFilePath.asString().isEmpty() && pTabData->m_isaFilePath.exists())
            {
                if (nullptr != pTabData->m_pISASourceTableView)
                {
                    pTabData->m_pISASourceTableView->UpdateView();
                }
            }

            if (!pTabData->m_ilFilePath.asString().isEmpty() && pTabData->m_ilFilePath.exists())
            {
                if (nullptr != pTabData->m_pILSourceView)
                {
                    pTabData->m_pILSourceView->updateView();
                }
            }
        }
    }
}
