//------------------------------ kaBuildToolbar.cpp ------------------------------

// Qt
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acDefinitions.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afQMdiSubWindow.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afDocUpdateManager.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaApplicationTreeHandler.h>
#include <AMDTKernelAnalyzer/src/kaBuildToolbar.h>
#include <AMDTKernelAnalyzer/src/kaBackendManager.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaTreeDataExtension.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>

// ---------------------------------------------------------------------------
// Name:        kaBuildToolbar::kaBuildToolbar
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        25/8/2013
// ---------------------------------------------------------------------------
kaBuildToolbar::kaBuildToolbar(QWidget* pParent) : acToolBar(pParent, KA_STR_toolbarName),
    m_pLineLabel(nullptr), m_pCompileOptions(nullptr),
    m_pOptionsButton(nullptr), m_pSettingsButton(nullptr),
    m_pBuildArchitectureCombo(nullptr), m_pShaderModelsLabelAction(nullptr),
    m_pShaderModelsComboAction(nullptr), m_pShaderTypesLabelAction(nullptr),
    m_pKernelsAndEntriesLabel(nullptr),
    m_pKernelsAndEntriesComboActionToHide(nullptr), m_pKernelsAndEntriesComboLabelActionToHide(nullptr),
    m_pKernelsAndEntriesCombo(nullptr), m_inUpdateUIonMDI(false),
    m_pDXShaderTypesComboAction(nullptr)
{
    // Create the toolbar object name:
    QString toolBarQtName(KA_STR_toolbarName);
    setObjectName(toolBarQtName);
    setStyleSheet(QString("QToolBar{spacing:5px;}"));
    m_pLineLabel = new QLabel(KA_STR_toolbarInfoLabel, this);

    addWidget(m_pLineLabel);

    m_pCompileOptions = new QLineEdit(this);

    m_pCompileOptions->setMinimumWidth(AC_DEFAULT_TEXT_AVERAGE_CHAR_WIDTH * 20);
    m_pCompileOptions->setMaximumWidth(AC_DEFAULT_TEXT_AVERAGE_CHAR_WIDTH * 50);
    m_pCompileOptions->setToolTip(KA_STR_toolbarInfoLabelToolTip);
    addWidget(m_pCompileOptions);

    m_pSettingsButton = new QPushButton(KA_STR_toolbarSettingsButton, this);

    m_pSettingsButton->setToolTip(KA_STR_toolbarSettingsButtonTooltip);
    QFontMetrics fontMetric = m_pSettingsButton->fontMetrics();
    m_pSettingsButton->setMaximumWidth(fontMetric.boundingRect(KA_STR_toolbarSettingsButton).width() + 15);
    addWidget(m_pSettingsButton);
    addSeparator();

    // Add the build architecture combo box.
    m_pBuildArchitectureCombo = new QComboBox(this);
    QStringList architecturesList;
    gtString arch32Bit(AF_STR_loadProjectArchitecture32Bit);
    gtString arch64Bit(AF_STR_loadProjectArchitecture64Bit);
    architecturesList << acGTStringToQString(arch32Bit);
    architecturesList << acGTStringToQString(arch64Bit);
    m_pBuildArchitectureCombo->setToolTip(KA_STR_toolbarBuildArchitectureComboTooltip);
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    m_pBuildArchitectureCombo->insertItems(0, architecturesList);
#else
    m_pBuildArchitectureCombo->addItem(architecturesList[0]);
    m_pBuildArchitectureCombo->setVisible(false);
#endif
    addWidget(m_pBuildArchitectureCombo);

    m_pShaderModelsLabelAction = AddLabel(KA_STR_toolbarDXShaderModelLabel);

    QStringList toolTipList;
    toolTipList << QString(KA_STR_toolbarDXShaderTypesTooltip);
    QStringList shaderModels = QString(KA_STR_toolbarDXShaderModel).split(" ");
    m_pShaderModelsComboAction = AddComboBox(shaderModels, toolTipList, SIGNAL(currentIndexChanged(int)), this, SLOT(ShaderModelSelectionChange(int)));

    m_pShaderTypesLabelAction = AddLabel(KA_STR_toolbarTypeLabel);
    QStringList typeComboTooltip;
    typeComboTooltip << QString(KA_STR_toolbarTypesTooltip);
    QStringList typesList = QString(KA_STR_toolbarTypeData).split(" ");

    typesList = QString(KA_STR_toolbarDXShaderTypes).split(" ");
    m_pDXShaderTypesComboAction = AddComboBox(typesList, typeComboTooltip, SIGNAL(currentIndexChanged(int)), this, SLOT(DirectXShaderTypeSelectionChange(int)));

    // The m_pKernelsAndEntriesLabel & m_pKernelsAndEntriesCombo change roll based on the platform selected:
    m_pKernelsAndEntriesCombo = new QComboBox(this);

    int defaultCharW = (int)acScalePixelSizeToDisplayDPI(AC_DEFAULT_TEXT_AVERAGE_CHAR_WIDTH);
    m_pKernelsAndEntriesCombo->setMinimumWidth(defaultCharW * 30);
    m_pKernelsAndEntriesCombo->setToolTip(KA_STR_toolbarKernelNamesTooltip);
    m_pKernelsAndEntriesCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    QSize size = m_pKernelsAndEntriesCombo->size();

    m_pKernelsAndEntriesLabel = new QLabel(KA_STR_toolbarKernelLabel);
    m_pKernelsAndEntriesComboLabelActionToHide = addWidget(m_pKernelsAndEntriesLabel);
    m_pKernelsAndEntriesComboActionToHide = addWidget(m_pKernelsAndEntriesCombo);

    m_pOptionsButton = new QPushButton(KA_STR_toolbarOptionsButton, this);
    m_pOptionsButton->setFixedHeight(size.height());

    addSeparator();
    m_pOptionsButton->setToolTip(KA_STR_toolbarOptionsButtonTooltip);
    addWidget(m_pOptionsButton);

    // connect actions:
    bool rcConnect = connect(m_pKernelsAndEntriesCombo, SIGNAL(activated(int)), this, SLOT(comboSelectionChange(int)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(m_pBuildArchitectureCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(BuildArchitectureSelectionChange(int)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(m_pSettingsButton, SIGNAL(clicked()), this, SLOT(OnSettings()));
    GT_ASSERT(rcConnect);

    rcConnect = connect(m_pOptionsButton, SIGNAL(clicked()), this, SLOT(onOptions()));
    GT_ASSERT(rcConnect);

    rcConnect = connect(m_pCompileOptions, SIGNAL(textChanged(const QString&)), this, SLOT(onBuildOptionsTextChanged(const QString&)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(&kaBackendManager::instance(), SIGNAL(buildStart(const gtString&)), this, SLOT(OnBuildStarted(const gtString&)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(&kaBackendManager::instance(), SIGNAL(buildComplete(const gtString&)), this, SLOT(OnBuildEnded(const gtString&)));
    GT_ASSERT(rcConnect);

    // connect the tree control item activation:
    afApplicationTree* pTree = afApplicationCommands::instance()->applicationTree();
    GT_IF_WITH_ASSERT(nullptr != pTree)
    {
        rcConnect = connect(kaApplicationTreeHandler::instance(), SIGNAL(KADocumentSelectionChanged()), this, SLOT(ItemSelectionChanged()));
        GT_ASSERT(rcConnect);
    }
}


// ---------------------------------------------------------------------------
// Name:        kaBuildToolbar::~kaBuildToolbar
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        25/8/2013
// ---------------------------------------------------------------------------
kaBuildToolbar::~kaBuildToolbar()
{

}


// ---------------------------------------------------------------------------
// Name:        kaBuildToolbar::updateUI
// Description: update the ui based on the file path
// Arguments:   osFilePath& filePath
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        25/8/2013
// ---------------------------------------------------------------------------
void kaBuildToolbar::updateUIonMDI(const osFilePath& filePath, bool forceBuild)
{
    m_inUpdateUIonMDI = true;

    // force build clears the current file path so what ever path was stored before is not used
    if (forceBuild)
    {
        m_filePath.clear();
    }

    kaApplicationCommands::instance().ActivateMDITreeItem(filePath);

    m_inUpdateUIonMDI = false;
}

// ---------------------------------------------------------------------------
// Name:        kaBuildToolbar::comboSelectionChange
// Description: handle the selection change in the combo
// Author:      Gilad Yarnitzky
// Date:        25/8/2013
// ---------------------------------------------------------------------------
void kaBuildToolbar::comboSelectionChange(int selectedItemIndex)
{
    if (!m_inUpdateUIonMDI)
    {
        if (m_pKernelsAndEntriesCombo != nullptr)
        {
            gtString functionName = acQStringToGTString(m_pKernelsAndEntriesCombo->itemText(selectedItemIndex));
            gtString typeName;
            int lineNumber, fileId;

            KA_PROJECT_DATA_MGR_INSTANCE.SetEntryPointOrKernel(functionName, fileId, lineNumber, typeName);
            // Open the source file:
            KA_PROJECT_DATA_MGR_INSTANCE.OpenSourceFileOnGivenLine(lineNumber);

            // KA_PROJECT_DATA_MGR_INSTANCE.GetSelectedDXShaderType(gtString& fileType);
            if (m_pDXShaderTypesComboAction != nullptr)
            {
                QComboBox* pComobBox = qobject_cast<QComboBox*>(widgetForAction(m_pDXShaderTypesComboAction));

                if (nullptr != pComobBox)
                {
                    pComobBox->setCurrentText(acGTStringToQString(typeName));
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        kaBuildToolbar::onBuild
// Description: start the build
// Author:      Gilad Yarnitzky
// Date:        25/8/2013
// ---------------------------------------------------------------------------
void kaBuildToolbar::onBuild()
{
    osFilePath fileToOpen;
    GT_IF_WITH_ASSERT(kaApplicationCommands::instance().isMDIWindowIsCLFile(fileToOpen))
    {
        // Insert the opencl file to a vector:
        gtVector<osFilePath> filePathsVector;
        filePathsVector.push_back(fileToOpen);

        // Call the build command:
        kaApplicationCommands::instance().buildCommand(filePathsVector);
    }
}


// ---------------------------------------------------------------------------
// Name:        kaBuildToolbar::onOptions
// Description: open the options dialog
// Author:      Gilad Yarnitzky
// Date:        25/8/2013
// ---------------------------------------------------------------------------
void kaBuildToolbar::onOptions()
{
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(nullptr != pApplicationCommands)
    {
        pApplicationCommands->onToolsOptions(KA_STR_analyzeSettingsPageTitle);
    }
}


// ---------------------------------------------------------------------------
// Name:        kaBuildToolbar::onBuildOptionsTextChanged
// Description: Handle text changed event
// Author:      Gilad Yarnitzky
// Date:        1/9/2013
// ---------------------------------------------------------------------------
void kaBuildToolbar::onBuildOptionsTextChanged(const QString& optionsText)
{
    kaApplicationCommands::instance().setBuildOptions(optionsText);
}

// ---------------------------------------------------------------------------
void kaBuildToolbar::SetBuildOptions(const QString& buildOptions)
{
    GT_IF_WITH_ASSERT(m_pCompileOptions != nullptr)
    {
        m_pCompileOptions->blockSignals(true);
        m_pCompileOptions->setText(buildOptions);
        m_pCompileOptions->blockSignals(false);
    }
}


// ---------------------------------------------------------------------------
void kaBuildToolbar::OnSettings()
{
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(nullptr != pApplicationCommands)
    {
        pApplicationCommands->OnProjectSettings(KA_STR_projectSettingExtensionDisplayName);
    }
}


// ---------------------------------------------------------------------------
void kaBuildToolbar::OnBuildStarted(const gtString& sourceName)
{
    GT_UNREFERENCED_PARAMETER(sourceName);

    m_pOptionsButton->setEnabled(false);
}

// Update UI when build started:
// ---------------------------------------------------------------------------
void kaBuildToolbar::OnBuildEnded(const gtString& sourceName)
{
    GT_UNREFERENCED_PARAMETER(sourceName);

    m_pOptionsButton->setEnabled(true);
}

void kaBuildToolbar::UpdateToolbarElementsBySelectedNodeType()
{
    kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();

    if (pTreeHandler != nullptr)
    {
        kaProgram* pProgram = KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgram();
        const afApplicationTreeItemData* pItemData = pTreeHandler->GetSelectedItemData();
        GT_ASSERT(m_pCompileOptions != nullptr);

        if (pItemData != nullptr)
        {
            const bool isBuildOptionsEnabled = KA_PROJECT_DATA_MGR_INSTANCE.IsProgramPipeLine(pProgram) == false && pItemData->m_itemType != AF_TREE_ITEM_KA_FILE;
            m_pCompileOptions->setEnabled(isBuildOptionsEnabled);
            m_pSettingsButton->setEnabled(isBuildOptionsEnabled);

            if (isBuildOptionsEnabled && pProgram != nullptr)
            {
                QString buildOptions = pProgram->GetBuildType() == kaProgramCL ? KA_PROJECT_DATA_MGR_INSTANCE.BuildOptions() : KA_PROJECT_DATA_MGR_INSTANCE.ShaderBuildOptions();
                m_pCompileOptions->setText(buildOptions);
            }

            switch (pItemData->m_itemType)
            {
                case AF_TREE_ITEM_KA_PROGRAM:
                case AF_TREE_ITEM_KA_PROGRAM_GL_VERT:
                case AF_TREE_ITEM_KA_PROGRAM_GL_TESC:
                case AF_TREE_ITEM_KA_PROGRAM_GL_TESE:
                case AF_TREE_ITEM_KA_PROGRAM_GL_GEOM:
                case AF_TREE_ITEM_KA_PROGRAM_GL_FRAG:
                case AF_TREE_ITEM_KA_PROGRAM_GL_COMP:
                case AF_TREE_ITEM_GL_VERTEX_SHADER:
                case AF_TREE_ITEM_GL_TESSELLATION_CONTROL_SHADER:
                case AF_TREE_ITEM_GL_TESSELLATION_EVALUATION_SHADER:
                case AF_TREE_ITEM_GL_GEOMETRY_SHADER:
                case AF_TREE_ITEM_GL_FRAGMENT_SHADER:
                case AF_TREE_ITEM_GL_COMPUTE_SHADER:
                {
                    if (pProgram != nullptr)
                    {
                        UpdateToolbarElementsByProgramType(pProgram->GetBuildType());
                    }
                }
                break;

                case AF_TREE_ITEM_KA_PROGRAM_SHADER:
                {
                    //can be OpenCL kernel or DX shader
                    kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

                    if (pKAData != nullptr)
                    {
                        int fileId = KA_PROJECT_DATA_MGR_INSTANCE.GetFileID(pKAData->filePath());

                        if (pProgram != nullptr)
                        {
                            if (pProgram->GetBuildType() == kaProgramDX)
                            {

                                // DX shader detected
                                UpdateToolbarElementsByFileType(kaFileTypeDXGenericShader);
                                kaDxFolder* pDXFolder = dynamic_cast<kaDxFolder*>(pProgram);

                                if (pDXFolder != nullptr)
                                {
                                    gtString programModel, fileType, fileProfile;
                                    KA_PROJECT_DATA_MGR_INSTANCE.GetSelectedDXShaderType(fileType);
                                    programModel = pDXFolder->GetProgramLevelShaderModel();

                                    if (programModel.isEmpty())
                                    {
                                        GT_ASSERT(pDXFolder->GetFileShaderModel(fileId, programModel));
                                        pDXFolder->SetProgramLevelShaderModel(programModel);
                                    }
                                    else
                                    {
                                        pDXFolder->SetFileModel(fileId, programModel);
                                        GT_ASSERT(pDXFolder->GetFileShaderModel(fileId, programModel));
                                    }

                                    QComboBox* pComboModels = qobject_cast<QComboBox*>(widgetForAction(m_pShaderModelsComboAction));
                                    QComboBox* pComboTypes = qobject_cast<QComboBox*>(widgetForAction(m_pDXShaderTypesComboAction));

                                    if (pComboModels != nullptr && pComboTypes != nullptr && m_pKernelsAndEntriesCombo != nullptr)
                                    {
                                        pComboModels->setCurrentText(acGTStringToQString(programModel));
                                        pComboTypes->setCurrentText(acGTStringToQString(fileType));
                                        kaSourceFile* pFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(pKAData->filePath());
                                        RebuildKernelsAndEntryList(pFile, false);
                                        gtString selectedEntryPoint;
                                        pDXFolder->GetFileSelectedEntryPoint(fileId, selectedEntryPoint);

                                        if (!selectedEntryPoint.isEmpty())
                                        {
                                            m_pKernelsAndEntriesCombo->setCurrentText(acGTStringToQString(selectedEntryPoint));
                                        }
                                    }
                                }
                            }
                            else if (pProgram->GetBuildType() == kaProgramCL)
                            {
                                // CL kernel detected
                                UpdateToolbarElementsByFileType(kaFileTypeOpenCL);
                                kaSourceFile* pFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(pKAData->filePath());
                                RebuildKernelsAndEntryList(pFile, false);
                            }
                        }
                    }
                }
                break;

                case AF_TREE_ITEM_KA_FILE:
                {
                    ShowSourceFileNavigationDropList();
                    kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

                    if (pKAData != nullptr)
                    {
                        kaSourceFile* pFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(pKAData->filePath());
                        RebuildKernelsAndEntryList(pFile, false);
                    }
                }
                break;

                default:
                {
                    UpdateToolbarElementsByProgramType(kaProgramCL);
                }
                break;
            }
        }
    }
}

void kaBuildToolbar::UpdateToolbarElementsByFileType(kaFileTypes fileType)
{
    GT_IF_WITH_ASSERT(m_pShaderModelsComboAction != nullptr &&
                      m_pShaderModelsLabelAction != nullptr &&
                      m_pKernelsAndEntriesComboActionToHide != nullptr &&
                      m_pKernelsAndEntriesComboLabelActionToHide != nullptr &&
                      m_pDXShaderTypesComboAction != nullptr &&
                      m_pShaderTypesLabelAction != nullptr &&
                      m_pKernelsAndEntriesLabel != nullptr)
    {
        switch (fileType)
        {
            case kaFileTypeDXCompute:
            case kaFileTypeDXVertex:
            case kaFileTypeDXHull:
            case kaFileTypeDXPixel:
            case kaFileTypeDXGeometry:
            case kaFileTypeDXDomain:
            case kaFileTypeDXGenericShader:
                m_pShaderModelsLabelAction->UpdateVisible(true);
                m_pShaderModelsComboAction->UpdateVisible(true);
                m_pShaderModelsComboAction->UpdateEnabled(false);
                m_pKernelsAndEntriesComboActionToHide->setVisible(true);
                m_pKernelsAndEntriesComboLabelActionToHide->setVisible(true);
                m_pKernelsAndEntriesLabel->setText(KA_STR_toolbarEntryPointLabel);
                m_pShaderTypesLabelAction->UpdateVisible(true);
                m_pDXShaderTypesComboAction->UpdateVisible(true);
                break;

            case kaFileTypeGLSLComp:
            case kaFileTypeGLSLVert:
            case kaFileTypeGLSLTesc:
            case kaFileTypeGLSLTese:
            case kaFileTypeGLSLGeom:
            case kaFileTypeGLSLFrag:
            case kaFileTypeGLSLGenericShader:
            {
                m_pShaderModelsLabelAction->UpdateVisible(false);
                m_pShaderModelsComboAction->UpdateVisible(false);
                m_pKernelsAndEntriesComboActionToHide->setVisible(false);
                m_pKernelsAndEntriesComboLabelActionToHide->setVisible(false);
                m_pShaderTypesLabelAction->UpdateVisible(true);
                m_pDXShaderTypesComboAction->UpdateVisible(false);
            }
            break;

            case kaFileTypeOpenCL:
                m_pShaderModelsLabelAction->UpdateVisible(false);
                m_pShaderModelsComboAction->UpdateVisible(false);
                m_pKernelsAndEntriesComboActionToHide->setVisible(true);
                m_pKernelsAndEntriesComboLabelActionToHide->setVisible(true);
                m_pKernelsAndEntriesLabel->setText(KA_STR_toolbarKernelLabel);
                m_pShaderTypesLabelAction->UpdateVisible(false);
                m_pDXShaderTypesComboAction->UpdateVisible(false);
                break;

            default:
                m_pShaderModelsLabelAction->UpdateVisible(false);
                m_pShaderModelsComboAction->UpdateVisible(false);
                m_pKernelsAndEntriesComboActionToHide->setVisible(false);
                m_pKernelsAndEntriesComboLabelActionToHide->setVisible(false);
                m_pShaderTypesLabelAction->UpdateVisible(false);
                m_pDXShaderTypesComboAction->UpdateVisible(false);
        }
    }
}

void kaBuildToolbar::UpdateToolbarElementsByProgramType(kaProgramTypes programType)
{
    GT_IF_WITH_ASSERT(m_pShaderModelsComboAction != nullptr &&
                      m_pShaderModelsLabelAction != nullptr &&
                      m_pKernelsAndEntriesComboActionToHide != nullptr &&
                      m_pKernelsAndEntriesComboLabelActionToHide != nullptr &&
                      m_pDXShaderTypesComboAction != nullptr &&
                      m_pShaderTypesLabelAction != nullptr &&
                      m_pKernelsAndEntriesLabel != nullptr)
    {
        switch (programType)
        {
            case kaProgramDX:
                m_pShaderModelsLabelAction->UpdateVisible(true);
                m_pShaderModelsComboAction->UpdateVisible(true);
                m_pShaderModelsComboAction->UpdateEnabled(true);
                m_pKernelsAndEntriesComboActionToHide->setVisible(false);
                m_pKernelsAndEntriesComboLabelActionToHide->setVisible(false);
                m_pShaderTypesLabelAction->UpdateVisible(false);
                m_pDXShaderTypesComboAction->UpdateVisible(false);
                break;

            case kaProgramGL_Rendering:
            case kaProgramGL_Compute:
            case kaProgramVK_Rendering:
            case kaProgramVK_Compute:
                m_pShaderModelsLabelAction->UpdateVisible(false);
                m_pShaderModelsComboAction->UpdateVisible(false);
                m_pKernelsAndEntriesComboActionToHide->setVisible(false);
                m_pKernelsAndEntriesComboLabelActionToHide->setVisible(false);
                m_pShaderTypesLabelAction->UpdateVisible(false);
                m_pDXShaderTypesComboAction->UpdateVisible(false);
                break;

            case kaProgramCL:
                m_pShaderModelsLabelAction->UpdateVisible(false);
                m_pShaderModelsComboAction->UpdateVisible(false);
                m_pKernelsAndEntriesComboActionToHide->setVisible(false);
                m_pKernelsAndEntriesComboLabelActionToHide->setVisible(false);
                m_pShaderTypesLabelAction->UpdateVisible(false);
                m_pDXShaderTypesComboAction->UpdateVisible(false);
                break;

            default:
                m_pShaderModelsLabelAction->UpdateVisible(false);
                m_pShaderModelsComboAction->UpdateVisible(false);
                m_pKernelsAndEntriesComboActionToHide->setVisible(false);
                m_pKernelsAndEntriesComboLabelActionToHide->setVisible(false);
                m_pShaderTypesLabelAction->UpdateVisible(false);
                m_pDXShaderTypesComboAction->UpdateVisible(false);
                break;
        }
    }
}

// ---------------------------------------------------------------------------
void kaBuildToolbar::ShaderModelSelectionChange(int selectedItemIndex)
{
    //Combo is visible iff shader file tree node is selected under the DX Folder
    GT_UNREFERENCED_PARAMETER(selectedItemIndex);

    if (!m_inUpdateUIonMDI)
    {
        kaDxFolder* pProgram = dynamic_cast<kaDxFolder*>(KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgram());
        const QComboBox* pComboModels = qobject_cast<QComboBox*>(widgetForAction(m_pShaderModelsComboAction));

        if (pProgram != nullptr && nullptr != pComboModels)
        {
            gtString modelStr = acQStringToGTString(pComboModels->currentText());
            // setting program level shader model and updating all program shaders model
            pProgram->SetProgramLevelShaderModel(modelStr);
        }
    }
}

QString kaBuildToolbar::getEntryPointName() const
{
    QString ret;

    if (m_pKernelsAndEntriesCombo != nullptr)
    {
        ret = m_pKernelsAndEntriesCombo->currentText();
    }

    return ret;
}

// ---------------------------------------------------------------------------
void kaBuildToolbar::ItemSelectionChanged()
{
    if (m_pBuildArchitectureCombo != nullptr)
    {
        gtString projectArchitecture;
        KA_PROJECT_DATA_MGR_INSTANCE.GetProjectArchitectureAsString(projectArchitecture);

        if (acQStringToGTString(m_pBuildArchitectureCombo->currentText()) != projectArchitecture)
        {
            m_pBuildArchitectureCombo->setCurrentText(acGTStringToQString(projectArchitecture));
        }
    }

    m_inUpdateUIonMDI = true;
    UpdateToolbarElementsBySelectedNodeType();
    m_inUpdateUIonMDI = false;
}

void kaBuildToolbar::RebuildKernelsAndEntryList(kaSourceFile* pFile, bool shouldRememberCurrentFunc)
{
    GT_IF_WITH_ASSERT(pFile != nullptr)
    {
        QString currentSelection;

        if (shouldRememberCurrentFunc && m_pKernelsAndEntriesCombo != nullptr)
        {
            currentSelection = m_pKernelsAndEntriesCombo->currentText();
        }

        // rebuild the list
        if (nullptr != m_pKernelsAndEntriesCombo)
        {
            m_pKernelsAndEntriesCombo->clear();
        }

        QStringList kernelNames;

        for (const auto& itr : pFile->analyzeVector())
        {
            kernelNames.append(itr.m_kernelName);
        }

        kernelNames.removeDuplicates();
        m_pKernelsAndEntriesCombo->addItems(kernelNames);
        m_pKernelsAndEntriesCombo->setEnabled(true);
        m_pKernelsAndEntriesCombo->setCurrentText(acGTStringToQString(pFile->EntryPointFunction()));
    }
}

void kaBuildToolbar::ChangeFunctionSelection(const gtString& function)
{
    if (nullptr != m_pKernelsAndEntriesCombo)
    {
        m_pKernelsAndEntriesCombo->setCurrentText(acGTStringToQString(function));
    }
}

void kaBuildToolbar::ClearToolBar()
{
    // clear kernels/entry point combo
    if (m_pKernelsAndEntriesCombo != nullptr)
    {
        m_pKernelsAndEntriesCombo->clear();
    }

    // clear build options edit line
    if (m_pCompileOptions != nullptr)
    {
        m_pCompileOptions->clear();
    }
}

// -------------------------------------------------------------------------- -
void kaBuildToolbar::DirectXShaderTypeSelectionChange(int selectedItemIndex)
{
    //Combo is visible iff shader file tree node is selected under the DX Folder
    GT_UNREFERENCED_PARAMETER(selectedItemIndex);

    if (!m_inUpdateUIonMDI)
    {
        kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();

        if (pTreeHandler != nullptr)
        {
            afApplicationTreeItemData* pItemData = pTreeHandler->GetSelectedItemData();

            if (pItemData != nullptr)
            {
                kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

                if (pKAData != nullptr)
                {
                    int fileId = KA_PROJECT_DATA_MGR_INSTANCE.GetFileID(pKAData->filePath());
                    const QComboBox* pComboTypes = qobject_cast<QComboBox*>(widgetForAction(m_pDXShaderTypesComboAction));
                    const QComboBox* pComboModels = qobject_cast<QComboBox*>(widgetForAction(m_pShaderModelsComboAction));
                    kaDxFolder* pProgram = dynamic_cast<kaDxFolder*>(KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgram());

                    if (pProgram != nullptr && nullptr != pComboTypes &&
                        nullptr != pComboModels && nullptr != m_pKernelsAndEntriesCombo)
                    {
                        gtString typeStr = acQStringToGTString(pComboTypes->itemText(selectedItemIndex));
                        gtString modelStr = acQStringToGTString(pComboModels->currentText());
                        gtString entryPointStr = acQStringToGTString(m_pKernelsAndEntriesCombo->currentText());

                        pProgram->SetFileModel(fileId, modelStr);
                        pProgram->SetFileSelectedType(fileId, typeStr);
                        pProgram->SetFileSelectedEntryPoint(fileId, entryPointStr);
                    }
                }
            }
        }
    }
}


// -------------------------------------------------------------------------- -
void kaBuildToolbar::BuildArchitectureSelectionChange(int selectedItemIndex)
{
    if (!m_inUpdateUIonMDI)
    {
        if (m_pBuildArchitectureCombo != nullptr)
        {
            QString bitnessStr = m_pBuildArchitectureCombo->itemText(selectedItemIndex);
            KA_PROJECT_DATA_MGR_INSTANCE.SetProjectArchitectureFromString(acQStringToGTString(bitnessStr));
        }
    }
}

void kaBuildToolbar::ShowSourceFileNavigationDropList()
{
    bool isCLFileSelected = KA_PROJECT_DATA_MGR_INSTANCE.IsCLFileSelected();
    bool isDXShaderSelected = KA_PROJECT_DATA_MGR_INSTANCE.IsDXShaderSelected();
    bool isEntryPointComboVisible = (isCLFileSelected || isDXShaderSelected);

    m_pShaderModelsLabelAction->UpdateVisible(false);
    m_pShaderModelsComboAction->UpdateVisible(false);
    m_pKernelsAndEntriesComboActionToHide->setVisible(isEntryPointComboVisible);
    m_pKernelsAndEntriesComboLabelActionToHide->setVisible(isEntryPointComboVisible);

    if (isCLFileSelected)
    {
        m_pKernelsAndEntriesLabel->setText(KA_STR_toolbarKernelLabel);
    }
    else if (isDXShaderSelected)
    {
        m_pKernelsAndEntriesLabel->setText(KA_STR_toolbarEntryPointLabel);
    }

    m_pShaderTypesLabelAction->UpdateVisible(false);
    m_pDXShaderTypesComboAction->UpdateVisible(false);
}

