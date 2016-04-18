#include <qtIgnoreCompilerWarnings.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>

// Local:
#include <AMDTGpuProfiling/gpBaseSessionView.h>
#include <AMDTGpuProfiling/ProfileManager.h>
#include <AMDTGpuProfiling/CodeViewerWindow.h>
#include <AMDTGpuProfiling/KernelOccupancyWindow.h>
#include <AMDTGpuProfiling/SessionViewTabWidget.h>


gpBaseSessionView::gpBaseSessionView(QWidget* pParent) : SharedSessionWindow(pParent), m_pSessionDataModel(nullptr),
    m_pSessionTabWidget(nullptr), m_pKernelOccupancyWindow(nullptr), m_kernelOccupancyTabIndex(-1),
    m_pCodeViewerWindow(nullptr), m_codeViewerTabIndex(-1), m_firstActivation(true)
{
    m_pSessionTabWidget = new SessionViewTabWidget;

    bool rc = connect(m_pSessionTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(OnTabClose(int)));
    GT_ASSERT(rc);
}

gpBaseSessionView::~gpBaseSessionView()
{

}

void gpBaseSessionView::OnOccupancyFileGenerationFinish(bool success, const QString& strError, const QString& strOccupancyHTMLFileName)
{
    // Disconnect from handling the generation completion:
    bool rc = disconnect(ProfileManager::Instance(), SIGNAL(OccupancyFileGenerationFinished(bool, const QString&, const QString&)), this, SLOT(OnOccupancyFileGenerationFinish(bool, const QString&, const QString&)));
    GT_ASSERT(rc);

    if (success)
    {
        QFileInfo fileInfo;
        fileInfo.setFile(strOccupancyHTMLFileName);

        GPUSessionTreeItemData* pSessionData = qobject_cast<GPUSessionTreeItemData*>(m_pSessionData);

        if (fileInfo.exists() && pSessionData != nullptr)
        {
            pSessionData->AddAdditionalFile(strOccupancyHTMLFileName);

            QString dirPath = QString::fromWCharArray(pSessionData->SessionDir().directoryPath().asString().asCharArray());

            QDir localDir(dirPath);
            QStringList filterList;
            filterList << "*.js";
            filterList << "*.css";

            QFileInfoList files = localDir.entryInfoList(filterList, QDir::Files);

            foreach (QFileInfo file, files)
            {
                pSessionData->AddAdditionalFile(file.filePath());
            }
        }

        if (m_pKernelOccupancyWindow == nullptr)
        {
            m_pKernelOccupancyWindow = new KernelOccupancyWindow(this);

            if (!m_pKernelOccupancyWindow)
            {
                QString errMsg = QString(GP_Str_ErrorUnableToLoad).arg(GP_Str_OccupancyViewName);
                errMsg += "\n";
                errMsg += GP_Str_ErrorInsufficientMemory;
                Util::ShowErrorBox(errMsg);
                return;
            }

            // Add the kernel occupancy window
            m_kernelOccupancyTabIndex = m_pSessionTabWidget->addTab(m_pKernelOccupancyWindow, "");
        }

        // Load the file
        m_pKernelOccupancyWindow->LoadOccupancyHTMLFile(strOccupancyHTMLFileName);
        GT_ASSERT(m_kernelOccupancyTabIndex != -1);

        // Set the text for the window
        m_pSessionTabWidget->setTabText(m_kernelOccupancyTabIndex, QString(GP_Str_OccupancyWindowCaption).arg(m_currentDisplayedOccupancyKernel));
        m_pSessionTabWidget->setCurrentIndex(m_kernelOccupancyTabIndex);
    }
    else
    {
        Util::ShowErrorBox(strError);
    }
}

void gpBaseSessionView::OnTabClose(int index)
{
    if (index != 0)
    {
        if (index == m_codeViewerTabIndex)
        {
            m_codeViewerTabIndex = -1;
            SAFE_DELETE(m_pCodeViewerWindow);

            if (m_kernelOccupancyTabIndex > index)
            {
                m_kernelOccupancyTabIndex--;
            }
        }
        else if (index == m_kernelOccupancyTabIndex)
        {
            m_kernelOccupancyTabIndex = -1;
            m_currentDisplayedOccupancyKernel.clear();
            SAFE_DELETE(m_pKernelOccupancyWindow);

            if (m_codeViewerTabIndex > index)
            {
                m_codeViewerTabIndex--;
            }
        }
    }
}


void gpBaseSessionView::OnAboutToActivate()
{
    if (!m_firstActivation)
    {
        afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
        GT_IF_WITH_ASSERT((pApplicationCommands != nullptr) && (m_pSessionData != nullptr) && (m_pSessionData->m_pParentData != nullptr))
        {
            afApplicationTree* pTree = pApplicationCommands->applicationTree();
            GT_IF_WITH_ASSERT(pTree != nullptr)
            {
                const afApplicationTreeItemData* pCurrentItemData = pTree->getCurrentlySelectedItemData();
                bool isSameSession = false;

                if (pCurrentItemData != nullptr)
                {
                    isSameSession = (pCurrentItemData->m_filePath == m_pSessionData->m_pParentData->m_filePath);
                }

                if (!isSameSession)
                {
                    if ((m_pSessionData->m_pParentData != nullptr) && (m_pSessionData->m_pParentData->m_pTreeWidgetItem != nullptr))
                    {
                        pTree->selectItem(m_pSessionData->m_pParentData, true);
                    }
                }
            }
        }
    }

    m_firstActivation = false;
}
