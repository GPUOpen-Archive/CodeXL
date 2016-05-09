//------------------------------ kaApplicationCommands.cpp ------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTApplicationComponents/Include/acSourceCodeView.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFileLauncher.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTAPIClasses/Include/apProjectSettings.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewCreateEvent.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTOSAPIWrappers/Include/oaDriver.h>

// Framework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afMessageBox.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afQMdiSubWindow.h>
#include <AMDTApplicationFramework/Include/afIRunModeManager.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaProgram.h>
#include <AMDTKernelAnalyzer/src/kaCreateProgramDialog.h>
#include <AMDTKernelAnalyzer/src/kaAddFileDialog.h>
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaApplicationTreeHandler.h>
#include <AMDTKernelAnalyzer/src/kaDataAnalyzerFunctions.h>
#include <AMDTKernelAnalyzer/src/kaGlobalVariableManager.h>
#include <AMDTKernelAnalyzer/src/kaKernelView.h>
#include <AMDTKernelAnalyzer/src/kaMultiSourceView.h>
#include <AMDTKernelAnalyzer/src/kaOverviewView.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaSourceCodeView.h>
#include <AMDTKernelAnalyzer/src/kaTreeDataExtension.h>
#include <AMDTKernelAnalyzer/src/kaTreeModel.h>
#include <AMDTKernelAnalyzer/Include/kaAppWrapper.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>
#include <AMDTBackEnd/Include/beStringConstants.h>
#include <AMDTKernelAnalyzer/src/kaBackendManager.h>



// Static member initializations:
kaApplicationCommands* kaApplicationCommands::m_psMySingleInstance = nullptr;

#define KA_TREE_SELECTION_DEPTH 2
// ---------------------------------------------------------------------------
// Name:        kaApplicationCommands::kaApplicationCommands
// Description: constructor
// Author:      Gilad Yarnitzky
// Date:        30/7/2013
// ---------------------------------------------------------------------------
kaApplicationCommands::kaApplicationCommands() : m_pSettingCommandTextEdit(nullptr)
{
    kaApplicationTreeHandler::instance();
    m_extensionsList = QString(AF_STR_AnalyzedSourceFileExtension).split(AF_STR_CommaA);
    m_dxShaderProfilesList = QString(KA_STR_toolbarDXShaderProfileData).split(AF_STR_SpaceA);
}

// ---------------------------------------------------------------------------
// Name:        kaApplicationCommands::~kaApplicationCommands
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        30/7/2013
// ---------------------------------------------------------------------------
kaApplicationCommands::~kaApplicationCommands()
{
}

// ---------------------------------------------------------------------------
// Name:        kaApplicationCommands::instance
// Description: Returns the single instance of this class.
//              (If it does not exist - create it)
// Author:      Gilad Yarnitzky
// Date:        30/7/2013
// ---------------------------------------------------------------------------
kaApplicationCommands& kaApplicationCommands::instance()
{
    if (m_psMySingleInstance == nullptr)
    {
        m_psMySingleInstance = new kaApplicationCommands;
        GT_ASSERT(m_psMySingleInstance);
    }

    return *m_psMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        kaApplicationCommands::AddFileCommand
// Description: handle the add file command
// Author:      Gilad Yarnitzky
// Date:        6/8/2013
// ---------------------------------------------------------------------------
void kaApplicationCommands::AddFileCommand(gtVector<osFilePath>& addedFilePaths)
{

    // open the file browser to look for the cl file
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();

    // Sanity check:
    GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
    {
        // If there is no project, create one:
        if (afProjectManager::instance().currentProjectFilePath().isEmpty())
        {
            pApplicationCommands->CreateDefaultProject(KA_STR_executionMode);
        }

        // Select the executable file:
        gtVector<osFilePath> selectedFilesVector;

        if (m_lastAddFilePath.asString().isEmpty())
        {
            m_lastAddFilePath = afProjectManager::instance().currentProjectSettings().workDirectory().asFilePath();
        }

        osFilePath startingPath;

        // Get the list of selected cl files:
        QString selectionDetailsString = KA_STR_addFileSelectionFileDetails;
        bool rc = pApplicationCommands->ShowMultipleFilesSelectionDialog(selectedFilesVector, KA_STR_addFileSelectionTitle, acGTStringToQString(m_lastAddFilePath.asString()), selectionDetailsString);

        if (rc)
        {
            AddFilesToTree(selectedFilesVector, false);
            if (selectedFilesVector.size() > 0)
            {
                addedFilePaths = selectedFilesVector;
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaApplicationCommands::AddFilesToTree(const gtVector<osFilePath>& clFilesToAddVector, bool displayExistingFileErr)
{
    // open the file browser to look for the cl file
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();

    // Sanity check:
    GT_IF_WITH_ASSERT(nullptr != pApplicationCommands)
    {

        for (int i = 0; i < (int)clFilesToAddVector.size(); i++)
        {
            // Get the current file path:
            osFilePath selectedCLFile = clFilesToAddVector[i];

            bool rc = AddSourceFile(selectedCLFile, displayExistingFileErr);
            GT_ASSERT(rc);
        }

        // Save the project after the file was added:
        pApplicationCommands->OnFileSaveProject();
    }
}

// ---------------------------------------------------------------------------
void kaApplicationCommands::AddCLFilesToProject(const gtString& sourceDirsString)
{
    gtVector<osFilePath> clFilesToAdd;
    gtList<osFilePath> clFilesTmp;

    if (!sourceDirsString.isEmpty())
    {
        gtString searchStr = L"*.";
        searchStr = searchStr.append(AF_STR_clSourceFileExtension);

        gtStringTokenizer strTokenizer(sourceDirsString, AF_STR_Semicolon);
        gtString currentPath;

        // extracting all path specified in sourceDir separated by semicolons
        while (strTokenizer.getNextToken(currentPath))
        {
            // Get all the folders in the directory
            osDirectory sourceDirectory;
            sourceDirectory.setDirectoryFullPathFromString(currentPath);

            // Get all *.cl files from current dir
            sourceDirectory.getContainedFilePaths(searchStr, osDirectory::SORT_BY_NAME_ASCENDING, clFilesTmp);
            clFilesToAdd.insert(clFilesToAdd.end(), clFilesTmp.begin(), clFilesTmp.end());
        }

        AddFilesToTree(clFilesToAdd, false);
    }
}


// ---------------------------------------------------------------------------
// Name:        kaApplicationCommands::onUpdateAddFileCommand
// Description: update the ui of "add file" command
// Arguments:   bool& isEnabled
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        6/8/2013
// ---------------------------------------------------------------------------
void kaApplicationCommands::onUpdateAddFileCommand(bool& isEnabled)
{
    isEnabled = (afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode));

    // Enable add file command only when the build is not in progress:
    isEnabled = isEnabled && !kaBackendManager::instance().isInBuild();
}

// ---------------------------------------------------------------------------
void kaApplicationCommands::buildCommand(const gtVector<osFilePath>& filePathsVector)
{
    GT_IF_WITH_ASSERT(filePathsVector.size() > 0)
    {
        kaProjectDataManager::instance().SetLastBuildFiles(filePathsVector);
        bool rc = true;
        kaProgram* pProgram = KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgram();

        GT_IF_WITH_ASSERT(pProgram != nullptr)
        {
            KA_PROJECT_DATA_MGR_INSTANCE.SetLastBuildProgram(pProgram);

            kaProgramTypes programType = pProgram->GetBuildType();

            if (programType == kaProgramGL_Compute ||
                programType == kaProgramGL_Rendering ||
                programType == kaProgramVK_Compute ||
                programType == kaProgramVK_Rendering)
            {
                // marking to  erase all output dir. this can't be done in the CL/DX buildProgramCommand since it is used to build the wrapper
                // program of specific cl files which create a dummy program for the same path and if we do it there it will erase the entire directory.
                UpdateProgramInputAndOutputFiles(filePathsVector, pProgram, true);

                //F7 or Build button/menu item is used on GL/VK program
                buildProgramCommand(pProgram);
                rc = false;
            }

            if (rc)
            {
                buildNonRendering(filePathsVector, pProgram);

            }

        }
    }
}

void kaApplicationCommands::buildNonRendering(const gtVector<osFilePath>& filePathsVector, kaProgram* pProgram)
{
    afApplicationCommands* pApplicationCommand = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(nullptr != pApplicationCommand)
    {
        // files in vectors are at least 1 or we would have not gotten to this point
        kaSourceFile* pFileData = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(filePathsVector[0]);
        gtString entryPoint;
        gtString glShaderType;

        if (pFileData != nullptr)
        {
            entryPoint = pFileData->EntryPointFunction();
            glShaderType = pFileData->GetGLShaderType();
        }

        // Get the list of devices:
        set<string> selectedDeviceName;
        getSelectedDevices(selectedDeviceName);
        UpdateProgramInputAndOutputFiles(filePathsVector, pProgram);


        // CodeXL does not support OpenCL 2.0 kernels yet. If the build options specify OpenCL 2.0 then we issue an error message and cancel the build.
        // Get the build mode from the toolbar

        if (selectedDeviceName.size() != 0)
        {
            if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
            {
                // Make sure the output view is visible:
                pApplicationCommand->bringDockToFront(KA_STR_outputPaneCaption);
            }

            // OpenGL-specific build options.
            GLAdditionalBuildOptions additionalGLBuildOptions;
            const auto buildType = pProgram->GetBuildType();

            // Build DX or OpenCL
            if (buildType == kaProgramDX || buildType == kaProgramCL)
            {
                if (buildType == kaProgramCL && isOpenCL2Build())
                {
                    acMessageBox::instance().critical(AF_STR_ErrorA, KA_STR_OpenCL2NotSupported);
                }
                else
                {
                    //Create dummy program from active one, filter only files that were selected via UI and build the dummy wrapperProgramm
                    std::unique_ptr<kaProgram> wrapperProgram(pProgram->Clone());
                    gtList<int> ids = KA_PROJECT_DATA_MGR_INSTANCE.GetFileIDs(filePathsVector);
                    wrapperProgram->FilterByIds(ids);

                    // Generate new file name for the wrapper program (based on included source files).
                    gtString fixedProgramName = wrapperProgram->GetProgramName();
                    fixedProgramName << " (";
                    bool isFirst = true;
                    osFilePath filePath;

                    for (int fileId : ids)
                    {
                        filePath.clear();
                        KA_PROJECT_DATA_MGR_INSTANCE.GetFilePathByID(fileId, filePath);
                        gtString fileNameAndExtension;
                        filePath.getFileNameAndExtension(fileNameAndExtension);

                        if (!isFirst)
                        {
                            fixedProgramName << ", ";
                        }

                        if (!fileNameAndExtension.isEmpty())
                        {
                            fixedProgramName << fileNameAndExtension;
                        }

                        isFirst = false;
                    }

                    fixedProgramName << ")";

                    // Set the wrapper program's name;
                    wrapperProgram->SetProgramDisplayName(fixedProgramName);

                    // Launch the build.
                    buildProgramCommand(wrapperProgram.get());
                }
            }
            else
            {
                GT_ASSERT_EX(false, L"Unknown platform!");
            }
        }
        else
        {
            acMessageBox::instance().critical(AF_STR_ErrorA, KA_STR_emptyDeviceListError);
        }
    }
}

void kaApplicationCommands::UpdateProgramInputAndOutputFiles(const gtVector<osFilePath>& filePathsVector, kaProgram* pProgram, const bool eraseAll /*= false*/)
{
    for (const osFilePath& filePath : filePathsVector)
    {
        if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
        {
            // Save the modified source file for this path:
            SaveAllMDISubWindowsForFilePath(filePath);
        }
        else
        {
            afApplicationCommands::instance()->SaveAllMDISubWindowsForFilePath(filePath);
        }

        // CodeXL does not support OpenCL 2.0 kernels yet. If the build options specify OpenCL 2.0 then we issue an error message and cancel the build.
        if (isOpenCL2Build())
        {
            continue;
        }

        // erase from the output directory the files related with this source file
        EraseProgramFilePathOutput(pProgram, filePath, eraseAll);
    }
}

// ---------------------------------------------------------------------------
void kaApplicationCommands::cancelBuildCommand()
{
    kaBackendManager::instance().CancelBuild();
}


// ---------------------------------------------------------------------------
// Name:        kaApplicationCommands::isOpenCL2Build
// Description: Check if the build options specify OpenCL 2.0 support
// Arguments:
// Return Val:  true if "-cl-std=CL2.0" is found in the build options string
// Author:      Doron Ofek
// Date:        Nov-20, 2014
// ---------------------------------------------------------------------------
bool kaApplicationCommands::isOpenCL2Build() const
{
    bool isOpenCL2Build = false;

    const QString& buildOptionsString = KA_PROJECT_DATA_MGR_INSTANCE.BuildOptions();
    QString openCL2BuildOption("-cl-std=CL2.0");

    // Check if the build options specify OpenCL 2.0 support
    int indexOfOpenCL2BuildOption = buildOptionsString.indexOf(openCL2BuildOption, 0, Qt::CaseInsensitive);

    if (indexOfOpenCL2BuildOption != -1)
    {
        isOpenCL2Build = true;
    }

    return isOpenCL2Build;
}

// ---------------------------------------------------------------------------
osFilePath kaApplicationCommands::OutputFilePathForCurrentProject()
{
    // create the directory that will contain the output files and update the project manager data:
    const afProjectManager& projectManager = afProjectManager::instance();
    const apProjectSettings& projectSettings = projectManager.currentProjectSettings();
    gtString mainDirectoryName = projectSettings.projectName();
    mainDirectoryName.append(KA_STR_buildMainDirectoryName);
    osFilePath outputPath = projectManager.currentProjectFilePath();
    outputPath.setFileName(L"");
    outputPath.setFileExtension(L"");
    outputPath.appendSubDirectory(mainDirectoryName);

    osFilePath retPath(outputPath);

    return retPath;
}

// ---------------------------------------------------------------------------
void kaApplicationCommands::AppendFileSubPath(const osFilePath& fileNamePath, osFilePath& outputPath) const
{
    // create the directory of the specific run for the specific fileName:
    gtString fileName;
    gtString fileExt;
    fileNamePath.getFileName(fileName);
    fileNamePath.getFileExtension(fileExt);
    fileName.appendFormattedString(L"_%ls", fileExt.asCharArray());
    outputPath.appendSubDirectory(fileName);
}

// ---------------------------------------------------------------------------
// Name:        kaApplicationCommands::getSelectedDevices
// Description: Get the selected devices from the global settings
// Arguments:   set<string>& selectedDeviceName
// Author:      Gilad Yarnitzky
// Date:        13/8/2013
// ---------------------------------------------------------------------------
void kaApplicationCommands::getSelectedDevices(set<string>& selectedDeviceName) const
{
    QStringList  selectedDevices;
    CheckableTreeItem::getCheckedName(kaGlobalVariableManager::instance().currentTreeList(), KA_TREE_SELECTION_DEPTH, &selectedDevices);

    // Move from the QString format to the format used in the backend manager.
    for (const QString& device : selectedDevices)
    {
        selectedDeviceName.insert(device.trimmed().toLatin1().data());
    }
}

// ---------------------------------------------------------------------------
// Name:        kaApplicationCommands::onUpdateBuildCommand
// Description: check if there is a selected file in the tree
// Arguments:   bool& isEnabled
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        12/8/2013
// ---------------------------------------------------------------------------
void kaApplicationCommands::onUpdateBuildCommand(bool& isEnabled)
{
    bool isInBuild = kaBackendManager::instance().isInBuild();
    isEnabled = ((afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode)) && !isInBuild);
    const kaProgram* pActiveProgram = KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgram();
    const bool isActiveProgramHasFiles = pActiveProgram != nullptr && pActiveProgram->HasFile();
    gtVector<osFilePath> dummFilesPath;

    isEnabled = isEnabled && isActiveProgramHasFiles;
}

// ---------------------------------------------------------------------------
void kaApplicationCommands::onUpdateCancelBuildCommand(bool& isEnabled)
{
    bool isInBuild = kaBackendManager::instance().isInBuild();
    isEnabled = ((afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode)) && isInBuild);
}

// ---------------------------------------------------------------------------
// Name:        kaApplicationCommands::openUpdateContainingFolder
// Description: Enable the command based on the type of the node
// Author:      Gilad Yarnitzky
// Date:        22/8/2013
// ---------------------------------------------------------------------------
void kaApplicationCommands::openUpdateContainingFolder(bool& isEnabled)
{
    isEnabled = (afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode));
    gtVector<osFilePath> dummFilesPath;

    isEnabled = isEnabled && (activeCLFiles(dummFilesPath) > 0);
}

// ---------------------------------------------------------------------------
// Name:        kaApplicationCommands::isMDIWindowIsCLFile
// Description: check if the active mdi window is of a cl file
// Arguments:   osFilePath& filePath
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        22/8/2013
// ---------------------------------------------------------------------------
bool kaApplicationCommands::isMDIWindowIsCLFile(osFilePath& filePath)
{
    bool retVal = false;

    afMainAppWindow* pMainWindow = afMainAppWindow::instance();

    GT_IF_WITH_ASSERT(nullptr != pMainWindow)
    {
        afQMdiSubWindow* pActiveWindow = pMainWindow->activeMDISubWindow();

        if (nullptr != pActiveWindow)
        {
            gtString fileExtension;
            osFilePath windowPath = pActiveWindow->filePath();
            windowPath.getFileExtension(fileExtension);

            if (AF_STR_clSourceFileExtension == fileExtension)
            {
                filePath = windowPath;
                retVal = true;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void kaApplicationCommands::onUpdateKAModeCommand(bool& isActionEnabled, bool& isActionChecked)
{
    // Enable if the active mode is not debug mode:
    bool isKAModeActive = (afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode));
    afRunModes runModes = afPluginConnectionManager::instance().getCurrentRunModeMask();
    isActionEnabled = ((!isKAModeActive) && (runModes == 0)) && kaAppWrapper::s_loadEnabled;
    isActionChecked = isKAModeActive;
}

// ---------------------------------------------------------------------------
unsigned int kaApplicationCommands::activeCLFiles(gtVector<osFilePath>& filesPath)
{
    unsigned int retVal = 0;

    // Sanity check:
    GT_IF_WITH_ASSERT(kaApplicationTreeHandler::instance() != nullptr)
    {
        // Get the active cl file path from the KA tree hander:
        retVal = kaApplicationTreeHandler::instance()->activeBuildFiles(filesPath);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void kaApplicationCommands::updateOpenViews(const osFilePath& filePath)
{
    // Pass through all the MDI files and close those that are do not exist
    GT_IF_WITH_ASSERT(afMainAppWindow::instance())
    {
        QMdiArea* pMdiArea = afMainAppWindow::instance()->mdiArea();
        GT_IF_WITH_ASSERT(pMdiArea)
        {
            QList<QMdiSubWindow*> windowsSubList = pMdiArea->subWindowList();

            foreach (QMdiSubWindow* pCurrentSubWindow, windowsSubList)
            {

                // Get the widget from the window:
                afQMdiSubWindow* pAfQTSubWindow = qobject_cast<afQMdiSubWindow*>(pCurrentSubWindow);

                if (pAfQTSubWindow != nullptr)
                {
                    // *** DO NOT SUBMIT ***
                    QWidget* pWidget = pAfQTSubWindow->widget();
                    kaKernelView* pKernelView = qobject_cast<kaKernelView*>(pWidget);
                    kaOverviewView* pOverviewView = qobject_cast<kaOverviewView*>(pWidget);

                    if (nullptr != pKernelView)
                    {
                        if (nullptr != pKernelView->GetActiveMultiSourceView() &&
                            nullptr != pKernelView->GetActiveMultiSourceView()->SourceView() &&
                            pKernelView->GetActiveMultiSourceView()->SourceView()->filePath() == filePath)
                        {
                            bool selectedView = false;

                            if (pMdiArea->activeSubWindow() == pCurrentSubWindow)
                            {
                                selectedView = true;
                            }

                            pKernelView->updateView(selectedView);
                        }
                        else
                        {
                            pKernelView->updateView();
                        }
                    }
                    else if (nullptr != pOverviewView)
                    {
                        pOverviewView->updateView();
                    }
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
void kaApplicationCommands::setBuildOptions(const QString& buildOptions)
{
    KA_PROJECT_DATA_MGR_INSTANCE.setBuildOptions(buildOptions);
    int amountOfExtensions = afProjectManager::instance().amountOfProjectExtensions();

    for (int i = 0; i < amountOfExtensions; ++i)
    {
        // Save the project settings for each of the extensions(implicitely it sets the command options text box):
        bool rc = afProjectManager::instance().saveCurrentProjectData(i);
        GT_ASSERT(rc);
    }
}

// ---------------------------------------------------------------------------
void kaApplicationCommands::SetToolbarBuildOptions(const QString& buildOptions)
{
    if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        kaBuildToolbar* pToolbar = afMainAppWindow::instance()->findChild<kaBuildToolbar*>(KA_STR_toolbarName);

        if (nullptr != pToolbar)
        {
            pToolbar->SetBuildOptions(buildOptions);
        }
    }
}

// ---------------------------------------------------------------------------
bool kaApplicationCommands::AddSourceFile(const osFilePath& clFilePath, bool displayExistingFileErr)
{
    bool retVal = false;
    kaSourceFile* pCurrentFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(clFilePath);

    if (pCurrentFile != nullptr)
    {

        if (displayExistingFileErr)
        {
            // Output a message to the user that the file exists:
            acMessageBox::instance().critical(AF_STR_ErrorA, KA_STR_treeProjectFileExists);
        }
        retVal = true;
    }
    else
    {
        // store the path for next time:
        m_lastAddFilePath = clFilePath;

        // Add the data to the project data manager, the data manager update the tree
        KA_PROJECT_DATA_MGR_INSTANCE.AddFileOnProjectLoad(clFilePath);
        pCurrentFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(clFilePath);
        KA_PROJECT_DATA_MGR_INSTANCE.BuildFunctionsList(clFilePath, pCurrentFile);

        GT_IF_WITH_ASSERT(nullptr != pCurrentFile)
        {
            QString newFilePlatform = acGTStringToQString(pCurrentFile->BuildPlatform());

            if (newFilePlatform.compare(KA_STR_platformDirectX) == 0)
            {
                const gtVector<kaProjectDataManagerAnaylzeData>& analyzedVector = pCurrentFile->analyzeVector();

                if (!analyzedVector.empty())
                {
                    SetNewFileProfile(clFilePath, analyzedVector[0].m_kernelName);
                }
            }
            else if (newFilePlatform.compare(KA_STR_platformOpenGL) == 0)
            {
                // GL file have a default entry point and do not have their file parsed
                // In GL type file, the user can not change the entry point and it will remain KA_STR_buildDXEntryPoint
                pCurrentFile->SetEntryPointFunction(acQStringToGTString(KA_STR_buildDXEntryPoint));
            }

            osFilePath overViewPath = pCurrentFile->buildDirectory().directoryPath();
            overViewPath.setFileName(KA_STR_overviewName);
            overViewPath.setFileExtension(KA_STR_overviewExtension);
            osFile overViewFile;
            overViewFile.open(overViewPath, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
            overViewFile.writeString(clFilePath.asString());
            overViewFile.close();

            retVal = true;
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
void kaApplicationCommands::NewProgramCommand(bool shouldForceProjectCreation)
{
    // If there is no project, create one:
    bool isProjectSet = !afProjectManager::instance().currentProjectFilePath().isEmpty();

    if (!isProjectSet || shouldForceProjectCreation)
    {
        afApplicationCommands::instance()->CreateDefaultProject(KA_STR_executionMode);
    }

    // Check if this is a KA Mode:
    bool isInKAMode = afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode);

    if (!isInKAMode)
    {
        // Switch to KA mode. Notice, handleDebugEvent is called, to make sure that the mode is switched immediately:
        apExecutionModeChangedEvent executionModeEvent(KA_STR_executionMode, -1);
        apEventsHandler::instance().handleDebugEvent(executionModeEvent);
    }

    // Check if this is a KA Mode and if project is set (the mode has changed, so the test should be performed again):
    isInKAMode = afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode);
    isProjectSet = !afProjectManager::instance().currentProjectFilePath().isEmpty();

    // Open an add file dialog to see what kind of file is going to be added:
    kaCreateProgramDialog programDialog(nullptr);
    int rc = afApplicationCommands::instance()->showModal(&programDialog);

    if (rc)
    {
        if (isProjectSet && isInKAMode)
        {
            CreateDefaultProgram(programDialog.GetSelectedProgramType());
        }
    }
}

// ---------------------------------------------------------------------------
void kaApplicationCommands::NewFileCommand(bool shouldForceProjectCreation, osFilePath& newFilePath, kaProgram*& pAssociatedProgram, kaPipelinedProgram::PipelinedStage stage)
{
    // If there is no project, create one:
    bool isProjectSet = !afProjectManager::instance().currentProjectFilePath().isEmpty();

    if (!isProjectSet || shouldForceProjectCreation)
    {
        afApplicationCommands::instance()->CreateDefaultProject(KA_STR_executionMode);
    }

    // Check if this is a KA Mode:
    bool isInKAMode = afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode);

    if (!isInKAMode)
    {
        // Switch to KA mode. Notice, handleDebugEvent is called, to make sure that the mode is switched immediately:
        apExecutionModeChangedEvent executionModeEvent(KA_STR_executionMode, -1);
        apEventsHandler::instance().handleDebugEvent(executionModeEvent);
    }

    // Check if this is a KA Mode and if project is set (the mode has changed, so the test should be performed again):
    isInKAMode = afExecutionModeManager::instance().isActiveMode(KA_STR_executionMode);
    isProjectSet = !afProjectManager::instance().currentProjectFilePath().isEmpty();

    // Open an add file dialog to see what kind of file is going to be added:
    unique_ptr<kaAddFileDialog> ptrAddFileDialog(nullptr);
    int rc = true;

    // Show the dialog
    const kaProgramTypes  programType = pAssociatedProgram == nullptr ? kaProgramTypes::kaProgramUnknown : pAssociatedProgram->GetBuildType();

    if (pAssociatedProgram == nullptr || (KA_PROJECT_DATA_MGR_INSTANCE.IsRender(pAssociatedProgram) &&
                                          stage == kaPipelinedProgram::PipelinedStage::KA_PIPELINE_STAGE_NONE) || programType == kaProgramTypes::kaProgramDX)
    {
        ptrAddFileDialog.reset(new kaAddFileDialog(nullptr, pAssociatedProgram, stage));
        rc = afApplicationCommands::instance()->showModal(ptrAddFileDialog.get());
    }
    else
    {
        ptrAddFileDialog.reset(new kaAddFileDialog(pAssociatedProgram, programType, stage));
    }

    if (rc)
    {
        if (isProjectSet && isInKAMode)
        {
            QString fileName, fileExt, filePlatform;
            ptrAddFileDialog->GetFileNameAndExtension(fileName, fileExt);
            filePlatform = ptrAddFileDialog->GetSelectedPlatform();

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS

            fileExt.remove(".");
#endif
            newFilePath = CreateDefaultFile(fileName, fileExt, filePlatform);
        }
        else
        {
            // Create the default kernel after the project is created and
            kaApplicationTreeHandler::instance()->SetShouldCreateDefaultKernel(true);
        }
    }

    if (newFilePath.exists())
    {
        kaSourceFile* pFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(newFilePath);

        if (pFile != nullptr)
        {
            pFile->SetFileType(ptrAddFileDialog->FileType());
        }
    }

    pAssociatedProgram = ptrAddFileDialog->GetAssociatedProgram();
}

// ---------------------------------------------------------------------------
osFilePath kaApplicationCommands::CreateDefaultFile(QString& fileNameFromUser, QString& fileExt, QString filePlatform /*= QString()*/)
{
    if (m_lastAddFilePath.asString().isEmpty())
    {
        m_lastAddFilePath = afProjectManager::instance().currentProjectSettings().workDirectory().asFilePath();
    }

    // Get the current file path:
    osFilePath selectedFile = OutputFilePathForCurrentProject();
    selectedFile.setFileExtension(acQStringToGTString(fileExt));

    // Look for the next free "Kerneli.cl" file name available:
    osDirectory selectedFileDir;
    bool rc = selectedFile.getFileDirectory(selectedFileDir);
    GT_IF_WITH_ASSERT(rc)
    {
        // Create the kernel output directory if it doesn't exist:
        gtString fileName = acQStringToGTString(fileNameFromUser);

        for (int i = 1; i < 500; i++)
        {
            gtString tempFileName = fileName;
            tempFileName.appendFormattedString(L"%d", i);
            osFilePath tempFilePath = selectedFile;
            tempFilePath.setFileName(tempFileName);

            if (!tempFilePath.exists())
            {
                selectedFile = tempFilePath;
                break;
            }
        }
    }

    // Create the default kernel file:
    CreateDefaultKernelFile(selectedFile, fileExt, filePlatform);

    // Add the file to the current project:
    rc = AddSourceFile(selectedFile);
    GT_ASSERT(rc);

    // Save the project after the file was added:
    afApplicationCommands::instance()->OnFileSaveProject();

    // Handle the new created kernel in the tree handler:

    kaApplicationTreeHandler::instance()->SetShouldCreateDefaultKernel(false);
    return selectedFile;
}

// ---------------------------------------------------------------------------
kaProgram* kaApplicationCommands::CreateDefaultProgram(kaProgramTypes programType)
{
    kaProgram* pProgram = nullptr;

    if (programType != -1)
    {
        // Get the current file path:
        osFilePath programPath = OutputFilePathForCurrentProject();
        // Look for the next free name
        QString programTypesTitles = acGTStringToQString(KA_STR_programTypesTitles);
        QStringList typesTitlesList = programTypesTitles.split(",");
        gtString programName = acQStringToGTString(FindDefaultProgramName(typesTitlesList[programType]));
        programPath.appendSubDirectory(programName);
        osDirectory programDir(programPath);

        // if directory not exists - create it
        if (!programDir.exists())
        {
            programDir.create();
        }

        pProgram = kaProgramFactory::Create(programType, programName);

        GT_IF_WITH_ASSERT(pProgram != nullptr)
        {
            if (kaApplicationTreeHandler::instance() != nullptr)
            {
                kaApplicationTreeHandler::instance()->AddProgram(true, pProgram);
            }

            KA_PROJECT_DATA_MGR_INSTANCE.AddProgram(pProgram);
        }
    }

    return pProgram;
}


// ---------------------------------------------------------------------------
void kaApplicationCommands::CreateDefaultKernelFile(const osFilePath& selectedNewFile, QString& fileExt, QString filePlatform)
{
    // Get the kernel name from the file path:
    gtString createdName;
    selectedNewFile.getFileName(createdName);
    // Add a dummy kernel to the file:
    osFile fileToWrite;
    osDirectory newFileDirectory;

    if (selectedNewFile.getFileDirectory(newFileDirectory))
    {
        newFileDirectory.create();
    }

    bool rc = fileToWrite.open(selectedNewFile, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
    GT_IF_WITH_ASSERT(rc)
    {
        gtASCIIString kernelText;
        kernelText.append(KA_STR_createdFileHeader);

        // Add the specific section of the file based on the extension
        int indexOfExtension = m_extensionsList.indexOf(fileExt);

        if (filePlatform == KA_STR_platformVulkan)
        {
            switch (indexOfExtension)
            {
                case kaAddFileDialog::kaFileGlslVert:
                    kernelText.appendFormattedString(KA_STR_createVK_VERT, createdName.asASCIICharArray());
                    break;

                case kaAddFileDialog::kaFileGlslFrag:
                    kernelText.appendFormattedString(KA_STR_createVK_FRAG, createdName.asASCIICharArray());
                    break;

                case kaAddFileDialog::kaFileGlslGeom:
                    kernelText.appendFormattedString(KA_STR_createVK_GEOM, createdName.asASCIICharArray());
                    break;

                case kaAddFileDialog::kaFileGlslTese:
                    kernelText.appendFormattedString(KA_STR_createVK_TESE, createdName.asASCIICharArray());
                    break;

                case kaAddFileDialog::kaFileGlslTesc:
                    kernelText.appendFormattedString(KA_STR_createVK_TESC, createdName.asASCIICharArray());
                    break;

                case kaAddFileDialog::kaFileGlslComp:
                    kernelText.appendFormattedString(KA_STR_createVK_COMP, createdName.asASCIICharArray());
                    break;

                default:
                    break;
            }
        }
        else
        {
            //not Vulkan
            switch (indexOfExtension)
            {
                case kaAddFileDialog::kaFileOpenCL:
                    kernelText.appendFormattedString(KA_STR_createdOpenCLFile, createdName.asASCIICharArray());
                    break;

                // Fixing CODEXL-1232 - Pixel shader contents used
                case kaAddFileDialog::kaFileDxGenericHLSL:
                    kernelText.appendFormattedString(DEFAULT_DX_PS, createdName.asASCIICharArray());
                    break;

                case kaAddFileDialog::kaFileDxVS:
                    kernelText.appendFormattedString(KA_STR_DEFAULT_DX_VS, createdName.asASCIICharArray());
                    break;

                case kaAddFileDialog::kaFileDxCS:
                    kernelText.appendFormattedString(DEFAULT_DX_CS, createdName.asASCIICharArray());
                    break;

                case kaAddFileDialog::kaFileDxGS:
                    kernelText.appendFormattedString(DEFAULT_DX_GS, createdName.asASCIICharArray());
                    break;

                case kaAddFileDialog::kaFileDxHS:
                    kernelText.appendFormattedString(DEFAULT_DX_HS, createdName.asASCIICharArray());
                    break;

                case kaAddFileDialog::kaFileDxDS:
                    kernelText.appendFormattedString(DEFAULT_DX_DS, createdName.asASCIICharArray());
                    break;

                case kaAddFileDialog::kaFileDxPS:
                    kernelText.appendFormattedString(DEFAULT_DX_PS, createdName.asASCIICharArray());
                    break;

                case kaAddFileDialog::kaFileGlslGeom:
                    kernelText.appendFormattedString(DEFAULT_GL_GEOM, createdName.asASCIICharArray());
                    break;

                case kaAddFileDialog::kaFileGlslFrag:
                case kaAddFileDialog::kaFileGlslVert:
                case kaAddFileDialog::kaFileGenericGLSL:
                    kernelText.appendFormattedString(KA_STR_createdGLSFile, createdName.asASCIICharArray());
                    break;

                case kaAddFileDialog::kaFileGlslComp:
                    kernelText.appendFormattedString(DEFAULT_GL_COMP, createdName.asASCIICharArray());
                    break;

                case kaAddFileDialog::kaFileGlslTese:
                    kernelText.appendFormattedString(DEFAULT_GL_TESE, createdName.asASCIICharArray());
                    break;

                case kaAddFileDialog::kaFileGlslTesc:
                    kernelText.appendFormattedString(DEFAULT_GL_TESC, createdName.asASCIICharArray());
                    break;
            }
        }

        fileToWrite.write(kernelText.asCharArray(), kernelText.length());

        // close it:
        fileToWrite.close();
    }
}

// ---------------------------------------------------------------------------
void kaApplicationCommands::ActivateMDITreeItem(const osFilePath& filePath)
{
    if (m_MDITreeItemfilePath != filePath)
    {
        // need to check what type of mdi is activated, the file path is not enough: all multi watch views do not have the real
        // source file name as the path so we need to get it from the path going two levels up getting the name from:
        // %appdata%/AMD/CodeXL/ProjectName_kernelOutput/FileName_Ext/Function/Devices.clxtxt -> %appdata%/AMD/CodeXL/ProjectName_kernelOutput/FileName.ext
        osFilePath filePathToActivate(filePath);
        gtString fileInternalName;
        filePath.getFileName(fileInternalName);

        if (KA_STR_kernelViewFile == fileInternalName)
        {
            osDirectory fileDirectory(filePath);
            // go to the file name level (two levels up since one the osDirectory includes the Devices.clxtxt as part of the path and it counts as a level
            fileDirectory.upOneLevel().upOneLevel();
            gtString fileNamePathAsString = fileDirectory.directoryPath().asString();
            osFilePath newFilePath = fileDirectory.upOneLevel().directoryPath();
            gtString  newFilePathStr = newFilePath.asString();
            gtString fileFullName = fileNamePathAsString.truncate(newFilePathStr.length() + 1, -1);

            // from the file name find the "_"
            int splitPos = fileFullName.find(L"_");

            if (splitPos != -1)
            {
                gtString fileName = fileFullName;
                fileName.truncate(0, splitPos - 1);
                gtString fileExt = fileFullName;
                fileExt.truncate(splitPos + 1, -1);
                newFilePath.setFileName(fileName);
                newFilePath.setFileExtension(fileExt);
                filePathToActivate.setFromOtherPath(newFilePath);
            }
        }

        // activate the tree node item:
        kaApplicationTreeHandler* pKATreeHandler = kaApplicationTreeHandler::instance();
        GT_IF_WITH_ASSERT(nullptr != pKATreeHandler)
        {
            if (pKATreeHandler->WasTreeCreated())
            {
                const gtVector<kaProgram*> projectPrograms = KA_PROJECT_DATA_MGR_INSTANCE.GetPrograms();
                for(kaProgram* pProg: projectPrograms)
                {
                    if (nullptr != pProg)
                    {
                        // check if current program contains this file
                        if (pProg->HasFile(filePathToActivate, AF_TREE_ITEM_ITEM_NONE))
                        {
                            afApplicationTree* pTree = afApplicationCommands::instance()->applicationTree();
                            if (nullptr != pTree)
                            {
                                QTreeWidgetItem* pProgramItem = pKATreeHandler->GetProgramTreeItem(pProg);
                                QTreeWidgetItem* pProgramChildItem = nullptr;
                                if (pProgramItem != nullptr)
                                {
                                    for (int i = 0; i < pProgramItem->childCount(); ++i)
                                    {
                                        pProgramChildItem = pProgramItem->child(i);
                                        if (nullptr != pProgramChildItem)
                                        {
                                            afApplicationTreeItemData* pChildData = pTree->getTreeItemData(pProgramChildItem);

                                            if (pChildData != nullptr)
                                            {
                                                kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pChildData->extendedItemData());

                                                if (pKAData != nullptr)
                                                {
                                                    if (pKAData->filePath() == filePathToActivate)
                                                    {
                                                        pTree->selectItem(pChildData, false);
                                                        m_MDITreeItemfilePath = filePath;
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
bool kaApplicationCommands::ParseISAFile(const osFilePath& isaFilePath, QString& dsamText, QString& scShaderText, QString& csDataText)
{
    bool retVal = false;

    // Clear the 3 strings:
    dsamText = KA_STR_ISANotAvailable;
    scShaderText.clear();
    csDataText.clear();


    // Get the text from the ISA file path:
    if (!isaFilePath.isEmpty())
    {

        // Convert the ISA file path to Qt string:
        QString isaPathStr = acGTStringToQString(isaFilePath.asString());

        // Open the ISA file:
        QFile file(isaPathStr);
        bool rc = file.open(QIODevice::ReadOnly | QIODevice::Text);
        GT_IF_WITH_ASSERT(rc)
        {
            bool isInSCShader = false;
            bool isInDASM = false;
            bool isInCSData = false;

            QTextStream stream(&file);
            QString line;

            do
            {
                // Read the next line:
                line = stream.readLine();

                // If we found the ISA start line, flag to start appending the string:
                if ((line.indexOf(KA_STR_ISA_DASMStart) >= 0) || (line.indexOf(KA_STR_ISA_DASMStart2) >= 0))
                {
                    isInDASM = true;
                    dsamText.clear();
                    isInCSData = false;
                    isInSCShader = false;

                    // Do not append the ISA text prefix to the ISA text string:
                    continue;
                }
                else if (line.indexOf(KA_STR_ISA_CSDataStart) >= 0)
                {
                    isInCSData = true;
                    isInDASM = false;
                    isInSCShader = false;

                    // Do not append the CS Data prefix to the ISA text string:
                    continue;
                }
                else if (line.indexOf(KA_STR_ISA_SCShaderStart) >= 0)
                {
                    isInSCShader = true;
                    isInDASM = false;
                    isInCSData = false;

                    // Do not append the SRC Shader source prefix to the ISA text string:
                    continue;
                }

                // Append this line to the relevant section:
                if (isInSCShader)
                {
                    // Add the current line to the ISA text:
                    scShaderText.append(line);
                    scShaderText.append(AF_STR_NewLineA);
                }
                // If we're in the dasm block, append this line:
                else if (isInDASM)
                {
                    // Add the current line to the ISA text:
                    dsamText.append(line);
                    dsamText.append(AF_STR_NewLineA);
                }
                else if (isInCSData)
                {
                    csDataText.append(line);
                    csDataText.append(AF_STR_NewLineA);
                }

            }
            while (!line.isNull());
        }
    }

    retVal = !scShaderText.isEmpty() && !dsamText.isEmpty() && !csDataText.isEmpty();

    // Remove new lines from start of strings:
    while (scShaderText.indexOf(AF_STR_NewLineA) == 0)
    {
        scShaderText = scShaderText.mid(1);
    }

    while (dsamText.indexOf(AF_STR_NewLineA) == 0)
    {
        dsamText = dsamText.mid(1);
    }

    while (csDataText.indexOf(AF_STR_NewLineA) == 0)
    {
        csDataText = csDataText.mid(1);
    }

    // Chop the new lines at the end of the strings:
    while (scShaderText.endsWith('\n'))
    {
        scShaderText.chop(1);
    }

    while (dsamText.endsWith('\n'))
    {
        dsamText.chop(1);
    }

    while (csDataText.endsWith('\n'))
    {
        csDataText.chop(1);
    }

    return retVal;
}

void kaApplicationCommands::OpenSourceFile(kaTreeDataExtension* pExtendedData, int lineNumber, bool forceopen)
{
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();

    // Sanity check:
    GT_IF_WITH_ASSERT((nullptr != pApplicationCommands) && (pExtendedData != nullptr))
    {
        // Get the current active MDI and see if it is the current file path. If it is just set the current line number in it
        // Check if it is afSourceCode or kaMultiSourceView
        bool foundSourceView = false;
        afMainAppWindow* pMainWindow = afMainAppWindow::instance();
        GT_IF_WITH_ASSERT(pMainWindow != nullptr)
        {
            afSourceCodeView* pActiveSourceView = nullptr;

            // Get the MDI sub window:
            afQMdiSubWindow* pSubWindow = pMainWindow->activeMDISubWindow();

            if (pSubWindow != nullptr)
            {
                osFilePath filePath;
                // two cases are handled here: afSourceCodeView and kaMultiSource
                afSourceCodeView* pSourceView = qobject_cast<afSourceCodeView*>(pSubWindow->widget());

                if (pSourceView != nullptr)
                {
                    if (pSourceView->filePath() == pExtendedData->filePath())
                    {
                        pActiveSourceView = pSourceView;
                    }
                }
                else
                {
                    kaKernelView* pMultiView = qobject_cast<kaKernelView*>(pSubWindow->widget());

                    if (pMultiView != nullptr)
                    {
                        kaMultiSourceView* pMultiSourceView = pMultiView->GetActiveMultiSourceView();

                        if (pMultiSourceView != nullptr && pMultiSourceView->SourceView()->filePath() == pExtendedData->filePath())
                        {
                            pActiveSourceView = pMultiSourceView->SourceView();
                        }
                    }
                }

                if (pActiveSourceView != nullptr && !forceopen)
                {
                    pActiveSourceView->setProgramCounter(lineNumber, 0);
                    foundSourceView = true;
                }
            }
        }

        // if the active mdi is not the source window then open the source window
        if (!foundSourceView)
        {
            if (!pExtendedData->filePath().isEmpty())
            {
                // open the file as indicated in the extended info
                gtString fileNameToDisplay;
                pExtendedData->filePath().getFileNameAndExtension(fileNameToDisplay);
                apMDIViewCreateEvent sourceCodeViewEvent(AF_STR_GenericMDIViewsCreatorID, pExtendedData->filePath(), fileNameToDisplay, 0, lineNumber, 0);

                // Notice: We call handleDebugEvent and not registerPendingDebugEvent, since we want the view to be created immediately.
                // We need its instance:
                apEventsHandler::instance().handleDebugEvent(sourceCodeViewEvent);
            }

        }

        // Get the source code view handling this source file:
        if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
        {
            // Attach the item data to the source code view:
            pApplicationCommands->AttachItemDataToSourceView(pExtendedData->filePath(), pExtendedData->m_pParentData);
        }
    }
}

// ---------------------------------------------------------------------------
void kaApplicationCommands::GetD3DCompilerDefaultDllPath(QString& dirPathStr)
{
    dirPathStr = "";
    osFilePath dirPath;

    // get d3d compiler dll file path + name + extension
    if (dirPath.SetInstallRelatedPath(osFilePath::OS_CODEXL_BINARIES_PATH))
    {
        dirPath.appendSubDirectory(SA_BE_STR_HLSL_optionsDefaultCompilerSubfolder);
        dirPath.setFileName(SA_BE_STR_HLSL_optionsDefaultCompilerFileName);
        dirPath.setFileExtension(SA_BE_STR_HLSL_optionsDefaultCompilerFileExtension);
        dirPathStr = acGTStringToQString(dirPath.asString());
    }
}

// Find the mdi sub window for the specific file path.
// For each mdi check the default file path like any source file. if it is not a source file
// check it if it is a kaKernelView with a multi source in it.
// This function will return only ONE file, so if the file was opened and edited in several
// views the first one will be saved and the rest will be ignored. There is no sense in saving all
// instances of modifications in multiple open views.
// ---------------------------------------------------------------------------
void kaApplicationCommands::SaveAllMDISubWindowsForFilePath(const osFilePath& filePath)
{
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();

    // Sanity check:
    GT_IF_WITH_ASSERT((pMainWindow != nullptr) && (pMainWindow->mdiArea() != nullptr) && (pApplicationCommands != nullptr))
    {
        // Get the MDI sub windows list:
        QList<QMdiSubWindow*> windowsSubList = pMainWindow->mdiArea()->subWindowList();

        foreach (QMdiSubWindow* pCurrentSubWindow, windowsSubList)
        {
            GT_IF_WITH_ASSERT(pCurrentSubWindow != nullptr)
            {
                // Get the widget from the window:
                afQMdiSubWindow* pAfQTSubWindow = qobject_cast<afQMdiSubWindow*>(pCurrentSubWindow);

                if (pAfQTSubWindow != nullptr)
                {
                    // Compare file names:
                    if (pAfQTSubWindow->filePath() == filePath)
                    {
                        acSourceCodeView* pSourceView = qobject_cast<acSourceCodeView*>(pAfQTSubWindow->widget());

                        if (pSourceView != nullptr && pSourceView->IsModified())
                        {
                            pApplicationCommands->saveMDIFile(filePath);
                            break;
                        }
                    }
                    else
                    {
                        kaKernelView* pKernelView = qobject_cast<kaKernelView*>(pCurrentSubWindow->widget());

                        if (nullptr != pKernelView &&
                            nullptr != pKernelView->GetActiveMultiSourceView() &&
                            nullptr != pKernelView->GetActiveMultiSourceView()->SourceView() &&
                            pKernelView->GetActiveMultiSourceView()->SourceView()->filePath() == filePath)
                        {
                            if (pKernelView->GetActiveMultiSourceView()->SourceView()->IsModified())
                            {
                                pKernelView->FileSave();
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

osFilePath kaApplicationCommands::GetOutputDirectoryFilePath(const osFilePath& filePath, bool shouldCreate)
{
    osFilePath retVal = OutputFilePathForCurrentProject();

    // check if the directory exists, if not create it:
    osDirectory outputDirectory(retVal);

    // Create the folder if it doesn't exist:
    if (!outputDirectory.exists() && shouldCreate)
    {
        outputDirectory.create();
    }

    // create the directory of the specific run for the specific kernel:
    AppendFileSubPath(filePath, retVal);

    return retVal;
}

void kaApplicationCommands::DetectDXShaderProfileFromEntryPointName(const QString& entryPoint, gtString& profileCandidate)
{

    for (const QString& currProfile : m_dxShaderProfilesList)
    {
        if (entryPoint.startsWith(currProfile.mid(0, 2), Qt::CaseInsensitive) || entryPoint.endsWith(currProfile.mid(0, 2), Qt::CaseInsensitive))
        {
            profileCandidate = acQStringToGTString(currProfile.mid(0, 2));
            profileCandidate.append(L"_5_0");
            break;
        }
    }
}

void kaApplicationCommands::SetNewFileProfile(const osFilePath& filePath, const QString& entryPoint /*= ""*/)
{
    GT_IF_WITH_ASSERT(!filePath.isEmpty())
    {
        // get data manager pointer for the new file
        kaSourceFile* pCurrentFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(filePath);
        GT_IF_WITH_ASSERT(pCurrentFile != nullptr)
        {
            // get file extension
            gtString gtFileExt;
            filePath.getFileExtension(gtFileExt);
            QString fileExt = acGTStringToQString(gtFileExt).toLower();
            gtString dxShaderProfile;
            GT_IF_WITH_ASSERT(!fileExt.isEmpty())
            {
                // in case of hlsl extension trying to detect shader profile by shader name
                if (fileExt.compare(KA_STR_CommonDXShaderExtension) == 0)
                {
                    // Failed to detect shader profile by shader name - using entry point name
                    DetectDXShaderProfileFromEntryPointName(entryPoint, dxShaderProfile);

                    // if profileCandidate is empty set vs_5_0 i.e. last element
                    if (dxShaderProfile.isEmpty())
                    {
                        pCurrentFile->setBuildProfile(acQStringToGTString(m_dxShaderProfilesList[m_dxShaderProfilesList.size() - 1]));
                    }
                    else
                    {
                        pCurrentFile->setBuildProfile(dxShaderProfile);
                    }
                }
                else
                {
                    // set default profile to the first in list
                    dxShaderProfile = acQStringToGTString(m_dxShaderProfilesList[0]);

                    unsigned int count = m_dxShaderProfilesList.count();

                    for (int i = count - 1; i >= 0; i--)
                    {
                        // find a profile that starts with the same characters as the file extension
                        if (m_dxShaderProfilesList[i].startsWith(fileExt))
                        {
                            dxShaderProfile = acQStringToGTString(m_dxShaderProfilesList[i]);
                            break;
                        }
                    }

                    // update the file profile
                    pCurrentFile->setBuildProfile(dxShaderProfile);
                }
            }
        }
    }
}

QString kaApplicationCommands::FindDefaultProgramName(const QString& currentName)
{
    osFilePath programPath = OutputFilePathForCurrentProject();
    QString tmpName = currentName + "%1";
    QString programName;
    int i = 1;

    do
    {
        programName = tmpName.arg(i, 2, 10, QChar('0'));
        programPath.setFileName(acQStringToGTString(programName));
        ++i;
    }
    while (programPath.exists());

    return programName;
}

void kaApplicationCommands::buildProgramCommand(kaProgram* pProgram)
{
    GT_IF_WITH_ASSERT(pProgram != nullptr)
    {
        set<string> selectedDeviceNames;
        getSelectedDevices(selectedDeviceNames);
        kaBackendManager::instance().PrepareProgramBuild(pProgram, selectedDeviceNames,
                                                         KA_PROJECT_DATA_MGR_INSTANCE.GetProjectArchitecture());
    }
}


void kaApplicationCommands::EraseProgramFilePathOutput(kaProgram* pProgram, const osFilePath& filePath, const bool eraseAll /*= false*/)
{
    if (pProgram != nullptr)
    {
        // Clean the output directory:
        osDirectory dir32Bit, dir64Bit;
        bool isProgram32Bit = KA_PROJECT_DATA_MGR_INSTANCE.GetBuildArchitecture() == kaBuildArch32_bit;
        pProgram->GetAndCreateOutputDirectories(dir32Bit, dir64Bit, isProgram32Bit, !isProgram32Bit);
        osDirectory eraseDir = KA_PROJECT_DATA_MGR_INSTANCE.GetProjectArchitecture() == kaBuildArch32_bit ? dir32Bit : dir64Bit;
        osFilePath erasePath(eraseDir.directoryPath());

        if (filePath.isEmpty() || eraseAll)
        {
            eraseDir.deleteRecursively();
        }
        else
        {
            gtString fileName;
            gtString fileNameAndExt;
            filePath.getFileName(fileName);
            gtString fileNameToSearch(L"*");
            fileNameToSearch.append(fileName);
            fileNameToSearch.append(L".*");
            gtList<osFilePath> filePaths;
            eraseDir.getContainedFilePaths(fileNameToSearch, osDirectory::SORT_BY_NAME_ASCENDING, filePaths);
            gtList<osFilePath>::iterator filePathIt = filePaths.begin();

            while (filePathIt != filePaths.end())
            {
                filePathIt->getFileNameAndExtension(fileNameAndExt);
                eraseDir.deleteFile(fileNameAndExt);
                filePathIt++;
            }
        }
    }
}
