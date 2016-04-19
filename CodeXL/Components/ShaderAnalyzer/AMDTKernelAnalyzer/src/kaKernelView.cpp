//------------------------------ kaKernelView.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Framework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afTreeItemType.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaAnalysisView.h>
#include <AMDTKernelAnalyzer/src/kaKernelView.h>
#include <AMDTKernelAnalyzer/src/kaMultiSourceView.h>
#include <AMDTKernelAnalyzer/src/kaOverviewView.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaStatisticsView.h>
#include <AMDTKernelAnalyzer/src/kaDataTypes.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>
#include <AMDTKernelAnalyzer/src/kaUtils.h>


// ---------------------------------------------------------------------------
// Name:        kaKernelView
// Description: constructor
// Author:      Gilad Yarnitzky
// Date:        18/8/2013
// ---------------------------------------------------------------------------
kaKernelView::kaKernelView(QWidget* pParent): QWidget(pParent), afBaseView(&afProgressBarWrapper::instance()), m_pOverView(NULL)
{
    // Create the tab view in a layout and add it.
    m_pMainLayout = new QVBoxLayout(this);


    m_pTabWidget = new acTabWidget;

    m_pTabWidget->setTabsClosable(true);

    m_pMainLayout->addWidget(m_pTabWidget);
    m_pMainLayout->setContentsMargins(0, 0 , 0, 0);

    connect(m_pTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloseRequestedHandler(int)));

    setLayout(m_pMainLayout);
}


// ---------------------------------------------------------------------------
// Name:        kaKernelView::~kaKernelView
// Description: destructor
// Author:      Gilad Yarnitzky
// Date:        18/8/2013
// ---------------------------------------------------------------------------
kaKernelView::~kaKernelView()
{

}


// ---------------------------------------------------------------------------
// Name:        kaKernelView::displayFile
// Description: Display a specific file. Opens it if it is not opened yet, or set the tab as active
//              if it is opened
// Author:      Gilad Yarnitzky
// Date:        18/8/2013
// ---------------------------------------------------------------------------
void kaKernelView::displayFile(const osFilePath& detailedFilePath, const osFilePath& kernelFilePath, int nodeType, QString nodeStr)
{
    bool tabFound = false;

    // check if the filePath is already displayed:
    if (m_filePathToViewMap.find(detailedFilePath.asString()) != m_filePathToViewMap.end())
    {
        // Find the tab with the same name as the file name and activate it:
        int indexOfView = m_filePathToViewMap[detailedFilePath.asString()];
        m_pTabWidget->setCurrentIndex(indexOfView);

        tabFound = true;
    }

    if (!tabFound || AF_TREE_ITEM_KA_DEVICE == nodeType)
    {
        QWidget* pCreatedView = NULL;

        // Create the appropriate view:
        switch (nodeType)
        {
            case AF_TREE_ITEM_KA_ANALYSIS:
            {
                pCreatedView = new kaAnalysisView(this, detailedFilePath);

            }
            break;

            case AF_TREE_ITEM_KA_STATISTICS:
            {
                //get program name from path
                boost::filesystem::path path(detailedFilePath.asString().asCharArray());
                gtString programName = kaUtils::ToGtString(path.parent_path().parent_path().filename());

                kaPlatform platform = KA_PROJECT_DATA_MGR_INSTANCE.GetBuildPlatform(programName);
                kaSourceFile* pFileData = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(kernelFilePath);
                gtString buildProfile;
                GT_IF_WITH_ASSERT(pFileData != nullptr)
                {
                    buildProfile = pFileData->BuildProfile();
                }
                pCreatedView = new kaStatisticsView(this, kernelFilePath, detailedFilePath, platform, buildProfile);
            }
            break;

            case AF_TREE_ITEM_KA_DEVICE:
            {
                bool isGCN = true;

                if (!nodeStr.isEmpty())
                {
                    QStringList nonGCNVersions = QString(KA_STR_NonGCNVersions).split(" ");

                    for (auto it : nonGCNVersions)
                    {
                        if (nodeStr.contains(it, Qt::CaseInsensitive))
                        {
                            isGCN = false;
                            break;
                        }
                    }
                }

                pCreatedView = createMultiSourceView(detailedFilePath, kernelFilePath, isGCN);
            }
            break;

            case AF_TREE_ITEM_KA_OVERVIEW:
            {
                m_pOverView = new kaOverviewView(this, detailedFilePath);
                m_pOverView->setObjectName(KA_STR_overviewNameASCII);
                m_pMainLayout->removeWidget(m_pTabWidget);
                m_pTabWidget->setVisible(false);
                m_pMainLayout->addWidget(m_pOverView);
            }
            break;

            default:
            {
                GT_ASSERT(false);
            }
            break;
        }

        if (nodeType != AF_TREE_ITEM_KA_OVERVIEW && (NULL != pCreatedView))
        {
            // Add it to the tab widget and the the map of tabs:
            QString fileNameQt;

            // for the Device type the node name is fixed and in the view we add tabs with the device name
            if (AF_TREE_ITEM_KA_DEVICE == nodeType)
            {
                fileNameQt = KA_STR_sourceTab;
            }
            else if (AF_TREE_ITEM_KA_STATISTICS == nodeType)
            {
                fileNameQt = fileNameQt.fromWCharArray(KA_STR_buildMainStatisticsFileName);
            }
            else
            {
                gtString fileName;
                detailedFilePath.getFileName(fileName);
                fileNameQt = acGTStringToQString(fileName);
            }

            int newViewIndex = m_pTabWidget->addTab(pCreatedView, fileNameQt);
            GT_IF_WITH_ASSERT(newViewIndex != -1)
            {
                m_filePathToViewMap[detailedFilePath.asString()] = newViewIndex;
                m_pTabWidget->setCurrentIndex(newViewIndex);

                writeDataFile();
            }
        }
        else if (NULL == pCreatedView)
        {
            // find the index of the source tab:
            int numTabs = m_pTabWidget->count();
            int sourceIndex = -1;

            for (int nTab = 0 ; nTab < numTabs ; nTab++)
            {
                if (m_pTabWidget->tabText(nTab) == KA_STR_sourceTab)
                {
                    sourceIndex = nTab;
                }
            }

            // The source index can be -1 if the view created is kaOverviewView in VS:
            if (-1 != sourceIndex)
            {
                m_filePathToViewMap[detailedFilePath.asString()] = sourceIndex;
                m_pTabWidget->setCurrentIndex(sourceIndex);

                writeDataFile();
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        kaKernelView::TabCloseRequestedHandler
// Description: closeView and remove it from available views and from tab view
// Author:      Gilad Yarnitzky
// Date:        18/8/2013
// ---------------------------------------------------------------------------
void kaKernelView::tabCloseRequestedHandler(int index)
{
    bool indexFound = false;
    // find the view in the map and remove it:
    gtMap<gtString, int>::iterator mapIterator;

    for (mapIterator = m_filePathToViewMap.begin() ; mapIterator != m_filePathToViewMap.end() ; mapIterator++)
    {
        if ((*mapIterator).second == index)
        {
            QWidget* pWidgetToBeRemoved = m_pTabWidget->widget(index);
            m_pTabWidget->removeTab(index);
            m_filePathToViewMap.erase(mapIterator);

            // before delete the view, if it is a multi source view unregister it from the document observed list:
            kaMultiSourceView* pMultiSourceView = qobject_cast<kaMultiSourceView*>(pWidgetToBeRemoved);

            if (NULL != pMultiSourceView)
            {
                pMultiSourceView->UnregisterDocument();
            }

            delete pWidgetToBeRemoved;

            writeDataFile();
            indexFound = true;
            break;

        }
    }

    if (indexFound)
    {
        // go through all new map iterators and update all indexes higher then current index
        for (mapIterator = m_filePathToViewMap.begin() ; mapIterator != m_filePathToViewMap.end() ; mapIterator++)
        {
            if ((*mapIterator).second > index)
            {
                // after removing a tab reduce the index of all the following tabs:
                (*mapIterator).second--;
            }
        }
    }

    // If there are no tab left close the main kernel view tab
    if (m_pTabWidget->count() == 0)
    {
        afApplicationCommands::instance()->closeFile(m_dataFile);
    }
}


// ---------------------------------------------------------------------------
// Name:        kaKernelView::createMultiSourceView
// Description: Create a multi source view with the correct internal source file paths
// Author:      Gilad Yarnitzky
// Date:        20/8/2013
// ---------------------------------------------------------------------------
QWidget* kaKernelView::createMultiSourceView(const osFilePath& filePath, const osFilePath& kernelFilePath, bool isGCN)
{
    QWidget* sourceView = NULL;
    gtString fileName;
    filePath.getFileName(fileName);

    if (!filePath.isEmpty())
    {
        osFilePath ilFilePath(filePath);
        osFilePath isaFilePath(filePath);
        ilFilePath.setFileExtension(KA_STR_kernelViewILExtension);
        isaFilePath.setFileExtension(KA_STR_kernelViewISAExtension);

        if (!ilFilePath.exists())
        {
            ilFilePath.clear();
        }

        if (!isaFilePath.exists())
        {
            isaFilePath.clear();
        }

        // Check that the kaMultiSourcecode does not already exists in the tabs
        kaMultiSourceView* pMultiWatchView = NULL;
        int numTabs = m_pTabWidget->count();

        for (int nTab = 0 ; nTab < numTabs && (NULL == pMultiWatchView) ; nTab++)
        {
            pMultiWatchView = qobject_cast<kaMultiSourceView*>(m_pTabWidget->widget(nTab));

            if (NULL != pMultiWatchView)
            {
                break;
            }
        }

        if (NULL == pMultiWatchView)
        {
            // mdi file path needed for handling editing of the source file
            osFilePath mdiFilePath(filePath);
            gtString identifyFileName;
            m_dataFile.getFileName(identifyFileName);
            mdiFilePath.setFileName(identifyFileName);
            mdiFilePath.setFileExtension(KA_STR_kernelViewExtension);

            pMultiWatchView = new kaMultiSourceView(this, kernelFilePath, mdiFilePath);
            sourceView = pMultiWatchView;

        }
        else
        {
            sourceView = NULL;
        }

        GT_IF_WITH_ASSERT(NULL != pMultiWatchView)
        {
            pMultiWatchView->AddView(filePath, isaFilePath, ilFilePath, isGCN);

            QFileInfo kernelFileInfo(acGTStringToQString(kernelFilePath.asString()));
            QFileInfo isaFileInfo(acGTStringToQString(isaFilePath.asString()));

            // if the source file modification date is later then the isa/il files date - mark the files as not updated
            if (kernelFileInfo.lastModified() > isaFileInfo.lastModified())
            {
                pMultiWatchView->ShowUpdateNotUpdateCaption(true);
            }

            MarkKernelLine(pMultiWatchView);
        }
    }

    return sourceView;
}

// ---------------------------------------------------------------------------
void kaKernelView::setDataFile(const osFilePath& dataFile)
{
    gtString dataFileExt;
    dataFile.getFileExtension(dataFileExt);

    if (m_dataFile.asString().isEmpty())
    {
        m_dataFile = dataFile;

        if (KA_STR_kernelViewExtension == dataFileExt)
        {
            // open the data file and read the info and open the files tabs:
            osFile deviceFile;
            deviceFile.open(dataFile, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_READ);
            gtString dataFileAsString;
            deviceFile.readIntoString(dataFileAsString);
            gtStringTokenizer fileAsLines(dataFileAsString, L"\n");

            // Read first line with the source line info:
            gtString sourceFileInfo;
            fileAsLines.getNextToken(sourceFileInfo);
            m_sourcePath.setFullPathFromString(sourceFileInfo);

            // Read second line with main splitter ratio info:
            gtString splitterRatio;
            fileAsLines.getNextToken(splitterRatio);
            QString splitterRatioQt = acGTStringToQString(splitterRatio);
            QList<QString> ratioList = splitterRatioQt.split(",");
            int mainLeftSizeAsInt = 0, mainRightSizeAsInt = 0;

            if (ratioList.count() == 2)
            {
                mainLeftSizeAsInt = ratioList[0].toInt();
                mainRightSizeAsInt = ratioList[1].toInt();
            }

            gtString fileToOpen;

            kaMultiSourceView* pMultiSourceView = NULL;

            while (fileAsLines.getNextToken(fileToOpen))
            {
                gtStringTokenizer filePath(fileToOpen, L",");
                gtString tabName, identifyPath, isaFile, ilFile;
                gtString leftSize, rightSize;
                int leftSizeAsInt, rightSizeAsInt;
                filePath.getNextToken(tabName);
                filePath.getNextToken(identifyPath);
                filePath.getNextToken(isaFile);
                filePath.getNextToken(ilFile);
                filePath.getNextToken(leftSize);
                filePath.getNextToken(rightSize);
                bool rc = leftSize.toIntNumber(leftSizeAsInt);
                GT_ASSERT(rc);
                rc = rightSize.toIntNumber(rightSizeAsInt);
                GT_ASSERT(rc);

                if (NULL == pMultiSourceView)
                {
                    osFilePath mdiFilePath(dataFile);

                    pMultiSourceView = new kaMultiSourceView(this, m_sourcePath, mdiFilePath, mainLeftSizeAsInt, mainRightSizeAsInt);
                    // Add it to the tab widget and the the map of tabs:
                    int newViewIndex = m_pTabWidget->addTab(pMultiSourceView, KA_STR_sourceTab);
                    GT_IF_WITH_ASSERT(newViewIndex != -1)
                    {
                        m_filePathToViewMap[identifyPath] = newViewIndex;
                        m_pTabWidget->setCurrentIndex(newViewIndex);
                    }
                }

                pMultiSourceView->AddView(identifyPath, isaFile, ilFile, leftSizeAsInt, rightSizeAsInt);
            }

            deviceFile.close();
        }
    }
}

void kaKernelView::MarkKernelLine(kaMultiSourceView* pMultiSourceView)
{
    // Mark the source at the kernel:
    kaSourceFile* pCurrentFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(m_sourcePath.asString());

    // The lowest directory is the kernel name:
    osFilePath tempPath = m_dataFile;
    tempPath.setFileName(L"");
    tempPath.setFileExtension(L"");
    gtString dataFileAsPath = tempPath.asString();
    int kernalNameTerminator = dataFileAsPath.reverseFind(osFilePath::osPathSeparator);

    GT_IF_WITH_ASSERT(kernalNameTerminator != -1)
    {
        gtString kernelName = dataFileAsPath.truncate(kernalNameTerminator + 1, dataFileAsPath.length());
        QString kernelNameAsQt = acGTStringToQString(kernelName);

        GT_IF_WITH_ASSERT(NULL != pCurrentFile)
        {
            int lineNumber = -1;
            int numKernels = pCurrentFile->analyzeVector().size();

            for (int nKernel = 0 ; nKernel < numKernels ; nKernel++)
            {
                if (pCurrentFile->analyzeVector()[nKernel].m_kernelName == kernelNameAsQt)
                {
                    lineNumber = pCurrentFile->analyzeVector()[nKernel].m_lineInSourceFile;
                    break;
                }
            }

            GT_IF_WITH_ASSERT(NULL != pMultiSourceView)
            {
                pMultiSourceView->MarkKernelLine(lineNumber);
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaKernelView::writeDataFile()
{
    gtString dataFileExt;
    m_dataFile.getFileExtension(dataFileExt);

    if (dataFileExt == KA_STR_kernelViewExtension)
    {
        osFile deviceFile;
        deviceFile.open(m_dataFile, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
        gtString sourcePath = m_sourcePath.asString();
        sourcePath.append(L"\n");
        deviceFile.writeString(sourcePath);
        int numTabs = m_pTabWidget->count();

        for (int nTab = 0 ; nTab < numTabs ; nTab++)
        {
            kaMultiSourceView* pCurrentView = qobject_cast<kaMultiSourceView*>(m_pTabWidget->widget(nTab));

            if (NULL != pCurrentView)
            {
                gtString fileString;
                pCurrentView->WriteDataFileString(fileString);
                deviceFile.writeString(fileString);
            }
        }

        deviceFile.close();
    }
}

// ---------------------------------------------------------------------------
void kaKernelView::updateOverview()
{
    kaOverviewView* childView = this->findChild<kaOverviewView*>(KA_STR_overviewNameASCII);

    if (NULL != childView)
    {
        childView->updateView();
    }
}

// ---------------------------------------------------------------------------
void kaKernelView::showLineNumbers(bool show)
{
    int numTabs = m_pTabWidget->count();

    for (int nTab = 0 ; nTab < numTabs ; nTab++)
    {
        kaMultiSourceView* pCurrentView = qobject_cast<kaMultiSourceView*>(m_pTabWidget->widget(nTab));

        if (NULL != pCurrentView)
        {
            pCurrentView->ShowLineNumbers(show);
        }
    }
}

// ---------------------------------------------------------------------------
void kaKernelView::updateView(bool selectedView)
{
    int numTabs = m_pTabWidget->count();

    for (int nTab = 0 ; nTab < numTabs ; nTab++)
    {
        kaMultiSourceView* pMultiView = qobject_cast<kaMultiSourceView*>(m_pTabWidget->widget(nTab));

        if (NULL != pMultiView)
        {
            if (!pMultiView->updateView(selectedView))
            {
                tabCloseRequestedHandler(nTab);
                nTab--;
                numTabs--;
            }
        }
        else
        {
            kaTableView* pTableView = qobject_cast<kaTableView*>(m_pTabWidget->widget(nTab));

            if (NULL != pTableView)
            {
                if (!pTableView->updateView())
                {
                    tabCloseRequestedHandler(nTab);
                    nTab--;
                    numTabs--;
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaKernelView::CloseAllTabs()
{
    gtMap<gtString, int>::iterator mapIterator;

    for (mapIterator = m_filePathToViewMap.begin(); mapIterator != m_filePathToViewMap.end(); mapIterator++)
    {
        int index = (*mapIterator).second;
        QWidget* pWidgetToBeRemoved = m_pTabWidget->widget(index);
        m_pTabWidget->removeTab(index);
        // before delete the view, if it is a multi source view unregister it from the document observed list:
        kaMultiSourceView* pMultiSourceView = qobject_cast<kaMultiSourceView*>(pWidgetToBeRemoved);

        if (NULL != pMultiSourceView)
        {
            pMultiSourceView->UnregisterDocument();
        }

        delete pWidgetToBeRemoved;
    }

    m_filePathToViewMap.clear();
}

// ---------------------------------------------------------------------------
void kaKernelView::closeEvent(QCloseEvent* event)
{
    GT_UNREFERENCED_PARAMETER(event);

    // close all tabs:
    CloseAllTabs();

    // Write the data file with no widgets in it to be displayed next time we open the view:
    writeDataFile();
}

// ---------------------------------------------------------------------------
void kaKernelView::onUpdateEdit_Cut(bool& isEnabled)
{
    isEnabled = false;
}

// ---------------------------------------------------------------------------
void kaKernelView::onUpdateEdit_Copy(bool& isEnabled)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTabWidget != NULL)
    {
        isEnabled = false;
        kaMultiSourceView* pMultiSourceView = qobject_cast<kaMultiSourceView*>(m_pTabWidget->currentWidget());

        if (pMultiSourceView != NULL)
        {
            pMultiSourceView->onUpdateEdit_Copy(isEnabled);
        }
        else
        {
            kaTableView* pTableView = qobject_cast<kaTableView*>(m_pTabWidget->currentWidget());

            if (pTableView != NULL)
            {
                pTableView->onUpdateEdit_Copy(isEnabled);
            }
            else
            {
                // Special case when in VS
                kaOverviewView* pOverView = qobject_cast<kaOverviewView*>(m_pTabWidget->currentWidget());

                if (pOverView != NULL)
                {
                    pOverView->onUpdateEdit_Copy(isEnabled);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaKernelView::onUpdateEdit_Paste(bool& isEnabled)
{
    isEnabled = false;
}

// ---------------------------------------------------------------------------
void kaKernelView::onUpdateEdit_SelectAll(bool& isEnabled)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTabWidget != NULL)
    {
        isEnabled = false;
        kaMultiSourceView* pMultiSourceView = qobject_cast<kaMultiSourceView*>(m_pTabWidget->currentWidget());

        if (pMultiSourceView != NULL)
        {
            pMultiSourceView->onUpdateEdit_SelectAll(isEnabled);
        }
        else
        {
            kaTableView* pTableView = qobject_cast<kaTableView*>(m_pTabWidget->currentWidget());

            if (pTableView != NULL)
            {
                pTableView->onUpdateEdit_SelectAll(isEnabled);
            }
            else
            {
                // Special case when in VS
                kaOverviewView* pOverView = qobject_cast<kaOverviewView*>(m_pTabWidget->currentWidget());

                if (pOverView != NULL)
                {
                    pOverView->onUpdateEdit_SelectAll(isEnabled);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaKernelView::onUpdateEdit_Find(bool& isEnabled)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTabWidget != NULL)
    {
        isEnabled = false;
        kaMultiSourceView* pMultiSourceView = qobject_cast<kaMultiSourceView*>(m_pTabWidget->currentWidget());

        if (pMultiSourceView != NULL)
        {
            pMultiSourceView->onUpdateEdit_Find(isEnabled);
        }
        else
        {
            kaTableView* pTableView = qobject_cast<kaTableView*>(m_pTabWidget->currentWidget());

            if (pTableView != NULL)
            {
                pTableView->onUpdateEdit_Find(isEnabled);
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaKernelView::onUpdateEdit_FindNext(bool& isEnabled)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTabWidget != NULL)
    {
        isEnabled = false;
        kaMultiSourceView* pMultiSourceView = qobject_cast<kaMultiSourceView*>(m_pTabWidget->currentWidget());

        if (pMultiSourceView != NULL)
        {
            pMultiSourceView->onUpdateEdit_FindNext(isEnabled);
        }
        else
        {
            kaTableView* pTableView = qobject_cast<kaTableView*>(m_pTabWidget->currentWidget());

            if (pTableView != NULL)
            {
                pTableView->onUpdateEdit_FindNext(isEnabled);
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaKernelView::onEdit_Cut()
{

}

// ---------------------------------------------------------------------------
void kaKernelView::onEdit_Copy()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTabWidget != NULL)
    {
        kaMultiSourceView* pMultiSourceView = qobject_cast<kaMultiSourceView*>(m_pTabWidget->currentWidget());

        if (pMultiSourceView != NULL)
        {
            pMultiSourceView->onEdit_Copy();
        }
        else
        {
            kaTableView* pTableView = qobject_cast<kaTableView*>(m_pTabWidget->currentWidget());

            if (pTableView != NULL)
            {
                pTableView->onEdit_Copy();
            }
            else
            {
                // Special case when in VS
                kaOverviewView* pOverView = qobject_cast<kaOverviewView*>(m_pTabWidget->currentWidget());

                if (pOverView != NULL)
                {
                    pOverView->onEdit_Copy();
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaKernelView::onEdit_Paste()
{

}

// ---------------------------------------------------------------------------
void kaKernelView::onEdit_SelectAll()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTabWidget != NULL)
    {
        kaMultiSourceView* pMultiSourceView = qobject_cast<kaMultiSourceView*>(m_pTabWidget->currentWidget());

        if (pMultiSourceView != NULL)
        {
            pMultiSourceView->onEdit_SelectAll();
        }
        else
        {
            kaTableView* pTableView = qobject_cast<kaTableView*>(m_pTabWidget->currentWidget());

            if (pTableView != NULL)
            {
                pTableView->onEdit_SelectAll();
            }
            else
            {
                // Special case when in VS
                kaOverviewView* pOverView = qobject_cast<kaOverviewView*>(m_pTabWidget->currentWidget());

                if (pOverView != NULL)
                {
                    pOverView->onEdit_SelectAll();
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaKernelView::onEdit_Find()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTabWidget != NULL)
    {
        kaMultiSourceView* pMultiSourceView = qobject_cast<kaMultiSourceView*>(m_pTabWidget->currentWidget());

        if (pMultiSourceView != NULL)
        {
            pMultiSourceView->onEdit_Find();
        }
        else
        {
            kaTableView* pTableView = qobject_cast<kaTableView*>(m_pTabWidget->currentWidget());

            if (pTableView != NULL)
            {
                pTableView->onEdit_SelectAll();
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaKernelView::onEdit_FindNext()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTabWidget != NULL)
    {
        kaMultiSourceView* pMultiSourceView = qobject_cast<kaMultiSourceView*>(m_pTabWidget->currentWidget());

        if (pMultiSourceView != NULL)
        {
            pMultiSourceView->onEdit_FindNext();
        }
        else
        {
            kaTableView* pTableView = qobject_cast<kaTableView*>(m_pTabWidget->currentWidget());

            if (pTableView != NULL)
            {
                pTableView->onEdit_SelectAll();
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaKernelView::onFindClick()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTabWidget != NULL)
    {
        kaMultiSourceView* pMultiSourceView = qobject_cast<kaMultiSourceView*>(m_pTabWidget->currentWidget());

        if (pMultiSourceView != NULL)
        {
            pMultiSourceView->onEdit_Find();
        }
        else
        {
            kaTableView* pTableView = qobject_cast<kaTableView*>(m_pTabWidget->currentWidget());

            if (pTableView != NULL)
            {
                pTableView->onEdit_Find();
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaKernelView::onFindPrev()
{
    // m_isSearchUp flag is up only when find prev button is pressed
    acFindParameters::Instance().m_isSearchUp = true;
    onFindNext();
    acFindParameters::Instance().m_isSearchUp = false;;
}

// ---------------------------------------------------------------------------
void kaKernelView::onFindNext()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTabWidget != NULL)
    {
        kaMultiSourceView* pMultiSourceView = qobject_cast<kaMultiSourceView*>(m_pTabWidget->currentWidget());

        if (pMultiSourceView != NULL)
        {
            pMultiSourceView->onEdit_FindNext();
        }
        else
        {
            kaTableView* pTableView = qobject_cast<kaTableView*>(m_pTabWidget->currentWidget());

            if (pTableView != NULL)
            {
                pTableView->onEdit_FindNext();
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaKernelView::storeFindClickedView()
{
    kaMultiSourceView* pMultiSourceView = qobject_cast<kaMultiSourceView*>(m_pTabWidget->currentWidget());

    if (pMultiSourceView != NULL)
    {
        pMultiSourceView->storeFindClickedView();
    }
}

// ---------------------------------------------------------------------------
void kaKernelView::OnUpdateFileSave(bool& isEnabled)
{
    isEnabled = false;
    GT_IF_WITH_ASSERT(m_pTabWidget->count() > 0)
    {
        if (NULL != GetActiveMultiSourceView())
        {
            isEnabled = true;
        }
    }
}

// ---------------------------------------------------------------------------
void kaKernelView::FileSave()
{
    kaMultiSourceView* pActiveSourceView = GetActiveMultiSourceView();

    if (NULL != pActiveSourceView)
    {
        pActiveSourceView->FileSave();
    }
}

// ---------------------------------------------------------------------------
void kaKernelView::FileSaveAs()
{
    kaMultiSourceView* pActiveSourceView = GetActiveMultiSourceView();

    if (NULL != pActiveSourceView)
    {
        pActiveSourceView->FileSaveAs();
    }
}

// ---------------------------------------------------------------------------
kaMultiSourceView* kaKernelView::GetActiveMultiSourceView()
{
    kaMultiSourceView* retVal = NULL;

    GT_IF_WITH_ASSERT(m_pTabWidget->count() > 0)
    {
        QWidget* pWidgetToCheck = m_pTabWidget->widget(m_pTabWidget->currentIndex());
        kaMultiSourceView* pMultiSourceView = qobject_cast<kaMultiSourceView*>(pWidgetToCheck);

        if (NULL != pMultiSourceView)
        {
            retVal = pMultiSourceView;
        }
    }

    return retVal;
}
