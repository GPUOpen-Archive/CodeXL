//------------------------------ kaProjectSettingsShaderExtension.cpp ------------------------------

// TinyXml:
#include <tinyxml.h>

// QT:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCSSSettings.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaBuildToolbar.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaProjectSettingsShaderExtension.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>

#include <AMDTBackEnd/Include/beStringConstants.h>

#define KA_TEXT_EDIT_BOX_COLUMN_SPAN 2

// ---------------------------------------------------------------------------
kaProjectSettingsShaderExtension::kaProjectSettingsShaderExtension():
    m_pCommandLineLabel(NULL),
    m_pCompileKindRadioGroup(NULL),
    m_pD3dRadio(NULL),
    m_pFxcRadio(NULL),
    m_pD3dPathCombox(NULL),
    m_pFxcPathCombox(NULL),
    m_pShaderIncludesLineEdit(NULL),
    m_pShaderMacrosLineEdit(NULL),
    m_pShaderIntrinsicsCheckBox(NULL)
{
    m_isBasicBuildOption = false;

    m_pCompileKindRadioGroup = NULL;
    m_pD3dRadio = NULL;

    m_pD3dPathCombox = NULL;
    m_d3dComboLastIndex = 0;
    m_pFxcPathCombox = NULL;
    m_fxcComboLastIndex = 0;

    m_checkboxFlagsNum = 2;

    osFilePath d3dDirPath;
    bool isOk = d3dDirPath.SetInstallRelatedPath(osFilePath::OS_CODEXL_BINARIES_PATH);
    GT_IF_WITH_ASSERT(isOk)
    {
        m_sD3dDllBrowsingPath = d3dDirPath.asString();
        m_sFxcExeBrowsingPath = d3dDirPath.asString();
    }
}

// ---------------------------------------------------------------------------
kaProjectSettingsShaderExtension::~kaProjectSettingsShaderExtension()
{
}

// ---------------------------------------------------------------------------
void kaProjectSettingsShaderExtension::Initialize()
{
    m_pGridLayout = new QGridLayout;

    // The right margin if for the margin between the scroll and the table content:
    m_pGridLayout->setContentsMargins(0, 0, 5, 0);

    GenerateOptionsTable();

    // Create the command text control:
    m_pCommandTextEdit = new QTextEdit;

    bool rc = kaProjectSettingsExtensionBase::connect(m_pCommandTextEdit, SIGNAL(textChanged()), this, SLOT(OnCommandTextChanged()));
    GT_ASSERT(rc);
    rc = kaProjectSettingsExtensionBase::connect(&KA_PROJECT_DATA_MGR_INSTANCE, SIGNAL(ShaderBuildOptionsChanged()), this, SLOT(OnCommandTextChanged()));
    GT_ASSERT(rc);

    QString dummyText = "first \n second \n third\n forth";
    QRect commandLineTextRect = QFontMetrics(m_pCommandTextEdit->font()).boundingRect(dummyText);
    m_pCommandTextEdit->resize(-1, commandLineTextRect.height());

    m_pCommandLineLabel = new QLabel(KA_STR_HLSL_optionsDialogBuildCommand);

    m_pCommandLineLabel->setStyleSheet(AF_STR_captionLabelStyleSheet);

    // Create the main layout and add the controls to it:
    QVBoxLayout* pMainLayout = new QVBoxLayout;


    pMainLayout->addWidget(m_pGridWidget, 1, 0);
    pMainLayout->addStretch();

    QVBoxLayout* pBottomLayout = new QVBoxLayout;

    pBottomLayout->addWidget(m_pCommandLineLabel, 0, Qt::AlignBottom);
    pBottomLayout->addWidget(m_pCommandTextEdit, 1, Qt::AlignBottom);
    pBottomLayout->setContentsMargins(0, 0, 0, 0);
    pMainLayout->addLayout(pBottomLayout);

    m_pCommandTextEdit->setStyleSheet(AF_STR_grayBorderWhiteBackgroundTE);

    setLayout(pMainLayout);
}

// ---------------------------------------------------------------------------
gtString kaProjectSettingsShaderExtension::ExtensionXMLString()
{
    gtString retVal = KA_STR_projectSettingShaderExtensionName;
    return retVal;
}

// ---------------------------------------------------------------------------
gtString kaProjectSettingsShaderExtension::ExtensionTreePathAsString()
{
    gtString retVal = KA_STR_projectSettingShaderExtensionDisplayName;
    return retVal;
}

// ---------------------------------------------------------------------------
bool kaProjectSettingsShaderExtension::GetXMLSettingsString(gtString& projectAsXMLString)
{
    bool retVal = true;

    projectAsXMLString.appendFormattedString(L"<%ls>", ExtensionXMLString().asCharArray());
    projectAsXMLString.appendFormattedString(L"</%ls>", ExtensionXMLString().asCharArray());

    return retVal;
}

// ---------------------------------------------------------------------------
bool kaProjectSettingsShaderExtension::SetSettingsFromXMLString(const gtString& projectAsXMLString)
{
    GT_UNREFERENCED_PARAMETER(projectAsXMLString);

    bool retVal = true;

    // get shader compile type from manager (xml) and the relevant radio button
    m_originalBuildOptions = KA_PROJECT_DATA_MGR_INSTANCE.ShaderBuildOptions();

    // get compile type
    QString compileType = KA_PROJECT_DATA_MGR_INSTANCE.ShaderCompileType();

    // set path Combos
    GT_IF_WITH_ASSERT(NULL != m_pD3dPathCombox && NULL != m_pFxcPathCombox && NULL != m_pD3dRadio)
    {
        SetPathCombobox(m_pD3dPathCombox, KA_PROJECT_DATA_MGR_INSTANCE.ShaderD3dBuilderPath(), D3D);
        SetPathCombobox(m_pFxcPathCombox, KA_PROJECT_DATA_MGR_INSTANCE.ShaderFxcBuilderPath(), FXC);

        CompilerType selectedType = D3D;

        if (compileType == KA_STR_HLSL_optionsDialogD3DCompileType)
        {
            m_pD3dRadio->setChecked(true);
            m_pD3dPathCombox->setEnabled(true);
            m_pFxcPathCombox->setEnabled(false);
            selectedType = D3D;
        }
        else // if (compileType == KA_STR_HLSL_optionsDialogFXCCompileType)
        {
            m_pFxcRadio->setChecked(true);
            m_pD3dPathCombox->setEnabled(false);
            m_pFxcPathCombox->setEnabled(true);
            selectedType = FXC;
        }

        DisableCheckBoxWithEmptyOption(selectedType);
    }

    GT_IF_WITH_ASSERT(NULL != m_pCommandTextEdit)
    {
        m_pCommandTextEdit->clear();
        m_pCommandTextEdit->setText(m_originalBuildOptions);
    }

    QString macrosStr = KA_PROJECT_DATA_MGR_INSTANCE.ShaderMacros();
    GT_IF_WITH_ASSERT(NULL != m_pShaderMacrosLineEdit)
    {
        m_pShaderMacrosLineEdit->setText(macrosStr);
    }

    QString includesStr = KA_PROJECT_DATA_MGR_INSTANCE.ShaderIncludes();
    GT_IF_WITH_ASSERT(NULL != m_pShaderIncludesLineEdit)
    {
        m_pShaderIncludesLineEdit->setText(includesStr);
    }

    // Shader intrinsics extension
    bool isShaderIntrinsicsEnabled = KA_PROJECT_DATA_MGR_INSTANCE.IsD3D11ShaderIntrinsicsExtensionEnabled();
    m_pShaderIntrinsicsCheckBox->setChecked(isShaderIntrinsicsEnabled);


    return retVal;
}

// ---------------------------------------------------------------------------
bool kaProjectSettingsShaderExtension::SetPathCombobox(QComboBox* pCombo, const QString& path, CompilerType compilerType)
{
    osFilePath filePath(acQStringToGTString(path));

    return SetPathCombobox(pCombo, filePath, compilerType);
}

// ---------------------------------------------------------------------------
bool kaProjectSettingsShaderExtension::SetPathCombobox(QComboBox* pCombo, const osFilePath& path, CompilerType compilerType)
{
    bool isValid = true;

    GT_IF_WITH_ASSERT(NULL != pCombo)
    {
        if (!path.isEmpty())
        {
            pCombo->blockSignals(true);

            // validate path
            if (compilerType == D3D)
            {
                isValid = ValidateD3DPath(path);
            }
            else
            {
                isValid = ValidateFXCPath(path);
            }

            if (isValid)
            {
                // check if path is already existing in the combo (for relevant compile type)
                QString qPathStr = acGTStringToQString(path.asString());
                int itemIndex = pCombo->findText(qPathStr, Qt::MatchFixedString);

                if (itemIndex != -1)
                {
                    pCombo->setCurrentIndex(itemIndex);
                }
                else
                {
                    // if not found , add it and select the new item
                    pCombo->insertItem(0, qPathStr);
                    pCombo->setCurrentIndex(0);
                }

                // set tool-tip
                pCombo->setToolTip(qPathStr);

                SetComboSelectedStringBold(pCombo);
            }

            pCombo->blockSignals(false);
        }
    }

    return isValid;
}

// ---------------------------------------------------------------------------
bool kaProjectSettingsShaderExtension::SaveCurrentSettings()
{
    bool retVal = true;

    // Save the command text value to the toolbar in the none VS mode
    // and to the data manager:
    GT_IF_WITH_ASSERT(NULL != m_pCommandTextEdit)
    {
        // Save the text for next time we'll need the original text:
        QString commandText = m_pCommandTextEdit->toPlainText();
        m_originalBuildOptions = commandText;

        // update the project data with build options
        KA_PROJECT_DATA_MGR_INSTANCE.SetShaderBuildOptions(commandText);

        auto pActiveProgram = KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgram();

        if (pActiveProgram != nullptr && kaProgramDX == pActiveProgram->GetBuildType())
        {
            kaApplicationCommands::instance().SetToolbarBuildOptions(commandText);
        }

        // set selected compile type to data manager

        if (IsD3dCompileKind())
        {
            KA_PROJECT_DATA_MGR_INSTANCE.SetShaderCompileType(KA_STR_HLSL_optionsDialogD3DCompileType);
        }
        else
        {
            KA_PROJECT_DATA_MGR_INSTANCE.SetShaderCompileType(KA_STR_HLSL_optionsDialogFXCCompileType);
        }

        // set builder path to project data
        GT_IF_WITH_ASSERT(NULL != m_pD3dPathCombox)
        {
            KA_PROJECT_DATA_MGR_INSTANCE.SetShaderD3dBuilderPath(m_pD3dPathCombox->currentText());
        }

        GT_IF_WITH_ASSERT(NULL != m_pFxcPathCombox)
        {
            KA_PROJECT_DATA_MGR_INSTANCE.SetShaderFxcBuilderPath(m_pFxcPathCombox->currentText());
        }

        // set macros to project data
        GT_IF_WITH_ASSERT(NULL != m_pShaderMacrosLineEdit)
        {
            KA_PROJECT_DATA_MGR_INSTANCE.SetShaderMacros(m_pShaderMacrosLineEdit->text());
        }

        // set includes to project data
        GT_IF_WITH_ASSERT(NULL != m_pShaderIncludesLineEdit)
        {
            KA_PROJECT_DATA_MGR_INSTANCE.SetShaderIncludes(m_pShaderIncludesLineEdit->text());
        }

        // set shader intrinsics enablement
        GT_IF_WITH_ASSERT(NULL != m_pShaderIntrinsicsCheckBox)
        {
            KA_PROJECT_DATA_MGR_INSTANCE.SetD3D11ShaderIntrinsicsExtensionEnabled(m_pShaderIntrinsicsCheckBox->isChecked());
        }

        unsigned int commandMask = GetD3dBuildptionsMask();
        KA_PROJECT_DATA_MGR_INSTANCE.SetShaderD3dBuildOptionsMask(commandMask);
    }
    return retVal;
}

// ---------------------------------------------------------------------------
void kaProjectSettingsShaderExtension::RestoreDefaultProjectSettings()
{
    // clear all check-boxes, text lines, and set combo-boxes with default value
    if (NULL != m_pCommandTextEdit)
    {
        m_pCommandTextEdit->setPlainText("");
    }

    // set the D3D compile as selected
    GT_IF_WITH_ASSERT(NULL != m_pD3dRadio)
    {
        m_pD3dRadio->setChecked(true);
    }

    GT_IF_WITH_ASSERT(NULL != m_pD3dPathCombox)
    {
        m_pD3dPathCombox->setEnabled(true);
    }

    GT_IF_WITH_ASSERT(NULL != m_pFxcPathCombox)
    {
        m_pFxcPathCombox->setEnabled(false);
    }

    // By default, D3D11 Shader Intrinsics extension is disabled.
    GT_IF_WITH_ASSERT(NULL != m_pShaderIntrinsicsCheckBox)
    {
        m_pShaderIntrinsicsCheckBox->setChecked(false);
    }

    //update data manager with default compile type, d3d and fxc path
    KA_PROJECT_DATA_MGR_INSTANCE.SetShaderCompileType(KA_STR_HLSL_optionsDialogD3DCompileType);
    KA_PROJECT_DATA_MGR_INSTANCE.SetShaderD3dBuilderPath(m_pD3dPathCombox->currentText());
    KA_PROJECT_DATA_MGR_INSTANCE.SetShaderFxcBuilderPath(m_pFxcPathCombox->currentText());

    InitPathComboxes();

    DisableCheckBoxWithEmptyOption(D3D);
}

// ---------------------------------------------------------------------------
bool kaProjectSettingsShaderExtension::AreSettingsValid(gtString& invalidMessageStr)
{
    GT_UNREFERENCED_PARAMETER(invalidMessageStr);

    // No project setting page at this stage for this extension
    bool retVal = true;

    if (m_pFxcPathCombox != nullptr && m_pD3dPathCombox != nullptr)
    {
        if (m_pFxcRadio != nullptr && m_pFxcRadio->isChecked() &&
            m_pFxcPathCombox->currentText().compare(KA_STR_HLSL_optionsDialogBrowse) == 0)
        {
            invalidMessageStr = KA_STR_HLSL_optionsDialogFxcLocationErrMessage;
            retVal = false;
        }
        else if (m_pD3dRadio != nullptr && m_pD3dRadio->isChecked())
        {
            if (m_pD3dPathCombox->currentText().compare(KA_STR_HLSL_optionsDialogBrowse) == 0)
            {
                invalidMessageStr = KA_STR_HLSL_optionsDialogD3dLocationErrMessage;
                retVal = false;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
bool kaProjectSettingsShaderExtension::RestoreCurrentSettings()
{
    bool retVal = true;

    // Restore the data manager information, it will restore the edit box data:
    KA_PROJECT_DATA_MGR_INSTANCE.SetShaderBuildOptions(m_originalBuildOptions);
    const kaProgram* pActiveProgram = KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgram();

    // restore the toolbar information:
    if (pActiveProgram != nullptr && pActiveProgram->GetBuildType() == kaProgramDX)
    {
        kaApplicationCommands::instance().SetToolbarBuildOptions(m_originalBuildOptions);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void kaProjectSettingsShaderExtension::GenerateOptionsTable()
{
    GT_IF_WITH_ASSERT(NULL != m_pGridLayout)
    {
        // general section
        QLabel* pGeneralLabel = new QLabel(KA_STR_optionsDialogGeneralSection);
        pGeneralLabel->setStyleSheet(AF_STR_captionLabelStyleSheetMain);
        m_pGridLayout->addWidget(pGeneralLabel, 0, 0, 1, 3);

        int numRows = m_pGridLayout->rowCount();

        // compile kind radio buttons
        m_pCompileKindRadioGroup = new QButtonGroup();
        m_pD3dRadio = new QRadioButton(KA_STR_HLSL_optionsDialogD3dCompiler);
        m_pCompileKindRadioGroup->addButton(m_pD3dRadio);
        m_pGridLayout->addWidget(m_pD3dRadio, numRows, 0, 1, 1);

        m_pD3dPathCombox = new QComboBox();
        m_pGridLayout->addWidget(m_pD3dPathCombox, numRows, 1, 1, 2);
        bool rc = connect(m_pD3dPathCombox, SIGNAL(activated(int)), this, SLOT(OnD3dComboxSelectionChanged(int)));
        GT_ASSERT(rc);

        m_pFxcRadio = new QRadioButton(KA_STR_HLSL_optionsDialogFXCCompiler);
        m_pCompileKindRadioGroup->addButton(m_pFxcRadio);
        numRows++;
        m_pGridLayout->addWidget(m_pFxcRadio, numRows, 0, 1, 1);

        // path combo-boxes
        m_pFxcPathCombox = new QComboBox();
        m_pGridLayout->addWidget(m_pFxcPathCombox, numRows, 1, 1, 2);
        rc = connect(m_pFxcPathCombox, SIGNAL(activated(int)), this, SLOT(OnFxcComboxSelectionChanged(int)));
        GT_ASSERT(rc);

        InitPathComboxes();

        // connect the change on radio buttons selection
        rc = connect(m_pCompileKindRadioGroup, SIGNAL(buttonClicked(int)), this, SLOT(OnCompileTypeSelectionChanged(int)));
        GT_ASSERT(rc);
        // set the D3D compile as selected
        m_pD3dRadio->setChecked(true);

        // add text edit for macros and includes:
        kaProjectSettingFlagData* pDataFlag = AddTextEditOption(m_pGridLayout, KA_STR_HLSL_optionsDialogPredefMacros, KA_STR_HLSL_optionsDialogPredefMacrosTooltip);
        m_pShaderMacrosLineEdit = qobject_cast<QLineEdit*>(pDataFlag->m_pWidget);

        pDataFlag = AddTextEditOption(m_pGridLayout, KA_STR_HLSL_optionsDialogIncludeDirs, KA_STR_HLSL_optionsDialogIncludeDirsTooltip);
        m_pShaderIncludesLineEdit = qobject_cast<QLineEdit*>(pDataFlag->m_pWidget);

        // AMD extensions section.
        numRows = m_pGridLayout->rowCount();
        QLabel* pAmdExtensionsLabel = new QLabel(KA_STR_optionsDialogAmdExtensions);
        pAmdExtensionsLabel->setStyleSheet(AF_STR_captionLabelStyleSheetMain);
        m_pGridLayout->addWidget(pAmdExtensionsLabel, numRows, 0, 1, 3);
        
        // Generate the tooltip string.
        QString tooltipStr;
        acWrapAndBuildFormattedTooltip(KA_STR_HLSL_optionsDialogEnableShaderIntrinsics, KA_STR_HLSL_optionsDialogAvoidShaderIntrinsicsToolTip, tooltipStr);

        // Create the check box.
        m_pShaderIntrinsicsCheckBox = new QCheckBox(KA_STR_HLSL_optionsDialogEnableShaderIntrinsics);
        m_pShaderIntrinsicsCheckBox->setToolTip(tooltipStr);

        // Add the check box to the grid.
        numRows = m_pGridLayout->rowCount();
        m_pGridLayout->addWidget(m_pShaderIntrinsicsCheckBox, numRows, 0, 1, 1);

        // Shader Build Options label
        numRows = m_pGridLayout->rowCount();
        pGeneralLabel = new QLabel(KA_STR_HLSL_optionsDialogBuildOptionsSection);
        pGeneralLabel->setStyleSheet(AF_STR_captionLabelStyleSheetMain);
        m_pGridLayout->addWidget(pGeneralLabel, numRows, 0, 1, 1, 0);

        pGeneralLabel = new QLabel(KA_STR_HLSL_optionsDialogD3dCompiler);
        pGeneralLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        pGeneralLabel->setStyleSheet(AF_STR_captionLabelStyleSheetMain);
        m_pGridLayout->addWidget(pGeneralLabel, numRows, 1, 1, 1, 0);

        pGeneralLabel = new QLabel(KA_STR_HLSL_optionsDialogFXCCompiler);
        pGeneralLabel->setStyleSheet(AF_STR_captionLabelStyleSheetMain);
        m_pGridLayout->addWidget(pGeneralLabel, numRows, 2, 1, 1);
        m_pGridLayout->setHorizontalSpacing(0);

        // create the scrolling area
        QScrollArea* pScrollArea = new QScrollArea;
        QWidget* pScrollAreaWidget = new QWidget;
        pScrollArea->setWidget(pScrollAreaWidget);
        pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        pScrollArea->setWidgetResizable(true);
        pScrollArea->setContentsMargins(0, 0, 0, 0);
        pScrollArea->setBackgroundRole(QPalette::Base);
        pScrollArea->setFrameStyle(0);
        m_pGridWidget = pScrollArea;
        m_pGridWidget->setContentsMargins(10, 10, 10, 10);

        // Make sure that the grid widget is transparent:
        QPalette p(m_pGridWidget->palette());
        p.setBrush(QPalette::Window, Qt::NoBrush);
        m_pGridWidget->setPalette(p);

        pScrollAreaWidget->setLayout(m_pGridLayout);

        // Shader Build Options
        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogAvoidFlowControl, KA_STR_HLSL_optionsDialogAvoidFlowControlToolTip, COMMANDBITFLAG_ALLOWFLOWCONTROL);
        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogDebug, KA_STR_HLSL_optionsDialogDebugToolTip, COMMANDBITFLAG_DEBUG);
        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogEnableBackwordsCompatibility, KA_STR_HLSL_optionsDialogEnableBackwordsCompatibilityToolTip, COMMANDBITFLAG_ENABLEBACKWORDSCOMPATIBILITY);
        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogEnableStrictness, KA_STR_HLSL_optionsDialogEnableStrictnessToolTip, COMMANDBITFLAG_ENABLESTRICTNESS);
        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogEPixelOptOff, KA_STR_HLSL_optionsDialogEPixelOptOffToolTip, COMMANDBITFLAG_EPIXELOPTOFF);
        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogVertexOptOff, KA_STR_HLSL_optionsDialogVertexOptOffToolTip, COMMANDBITFLAG_VERTEXOPTOFF);
        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogIeeeStrictness, KA_STR_HLSL_optionsDialogIeeeStrictnessToolTip, COMMANDBITFLAG_IEEESTRICTNESS);
        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogNoPres, KA_STR_HLSL_optionsDialogNoPresToolTip, COMMANDBITFLAG_NOPRES);

        QList<int> comboFlagsList;
        comboFlagsList.append(COMMANDBITFLAG_OPTLEVELSKIP); // skip optimization
        comboFlagsList.append(COMMANDBITFLAG_OPTLEVEL0);    // optimization level 0
        comboFlagsList.append(COMMANDBITFLAG_OPTLEVEL1);    // optimization level 1
        comboFlagsList.append(COMMANDBITFLAG_OPTLEVEL2);    // optimization level 2
        comboFlagsList.append(COMMANDBITFLAG_OPTLEVEL3);    // optimization level 3
        AddComboBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogOptLevel, KA_STR_HLSL_optionsDialogOptLevelToolTip, kaProjectSettingFlagData::KA_FLAG_TYPE_COMBO_BOX, 2, comboFlagsList);

        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogPackMatrixColMajor, KA_STR_HLSL_optionsDialogPackMatrixColMajorToolTip, COMMANDBITFLAG_PACKMATRIXCOLMAJOR);
        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogPackMatrixRowMajor, KA_STR_HLSL_optionsDialogPackMatrixRowMajorToolTip, COMMANDBITFLAG_PACKMATRIXROWMAJOR);
        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogPartialPrecision, KA_STR_HLSL_optionsDialogPartialPrecisionToolTip, COMMANDBITFLAG_PARTIALPRECISION);
        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogPreferFlowControl, KA_STR_HLSL_optionsDialogPreferFlowControlToolTip, COMMANDBITFLAG_REFERFLOWCONTROL);
        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogResourcesMatAlias, KA_STR_HLSL_optionsDialogResourcesMatAliasToolTip, COMMANDBITFLAG_RESOURCESMATALIAS);
        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogSkipValidation, KA_STR_HLSL_optionsDialogSkipValidationToolTip, COMMANDBITFLAG_SKIPVALIDATION);
        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogWarningsAreErrors, KA_STR_HLSL_optionsDialogWarningsAreErrorsToolTip, COMMANDBITFLAG_WARNINGSAREERRORS);

        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogOutputHexLiterals, KA_STR_HLSL_optionsDialogOutputHexLiteralsToolTip, 0);
        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogNumOfInst, KA_STR_HLSL_optionsDialogNumOfInstToolTip, 0);
        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogOutputInstInAsm, KA_STR_HLSL_optionsDialogOutputInstInAsmToolTip, 0);
        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogStripDebug, KA_STR_HLSL_optionsDialogStripDebugToolTip, 0);
        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogStripPrivate, KA_STR_HLSL_optionsDialogStripPrivateToolTip, 0);
        AddCheckBoxOption(m_pGridLayout, KA_STR_HLSL_optionsDialogStripReflection, KA_STR_HLSL_optionsDialogStripReflectionToolTip, 0);

        DisableCheckBoxWithEmptyOption(D3D);
    }
}

// ---------------------------------------------------------------------------
void kaProjectSettingsShaderExtension::InitPathComboxes()
{
    gtString foundFilePath;
    // ---D3D---
    GT_IF_WITH_ASSERT(NULL != m_pD3dPathCombox)
    {
        QFont* font = new QFont(m_pD3dPathCombox->font());
        font->setBold(true);

        m_pD3dPathCombox->blockSignals(true);
        m_pD3dPathCombox->clear();

        // find default path for D3D compiler file. if found add it's path to the combo
        m_pD3dPathCombox->addItem(KA_STR_HLSL_optionsDialogDefaultCompiler);

        QString d3dDllPath;
        kaApplicationCommands::instance().GetD3DCompilerDefaultDllPath(d3dDllPath);

        // Set the tooltip for the D3D compiler item.
        // if the bundled file is selected - get its path
        m_pD3dPathCombox->setToolTip(d3dDllPath);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        // get the first d3d file path in search order - if the file exist in combo get the next one
        gtVector<osFilePath> foundPaths;
        bool isPathsFound = osSearchForModuleInLoaderOrder(SA_BE_STR_HLSL_optionsDefaultCompilerFileName, foundPaths);

        if (isPathsFound)
        {
            // find the first path in ordered paths search that is not in the combo and add it to the combo
            for (gtVector<osFilePath>::iterator it = foundPaths.begin(); it != foundPaths.end(); it++)
            {
                osFilePath filePath = (*it);
                QString pathStr = acGTStringToQString(filePath.asString());

                // if the path is not in the combo - add it and stop
                if (QString::compare(pathStr, d3dDllPath, Qt::CaseInsensitive) != 0)
                {
                    m_pD3dPathCombox->addItem(pathStr);
                    break;
                }
            }
        }

#endif

        // add browse item
        m_pD3dPathCombox->addItem(KA_STR_HLSL_optionsDialogBrowse);

        // set the browse item bold
        m_pD3dPathCombox->setItemData(0, QVariant(*font), Qt::FontRole);

        m_pD3dPathCombox->setCurrentIndex(0);

        m_d3dComboLastIndex = 0;

        m_pD3dPathCombox->blockSignals(false);

        delete font;
    }

    // ---FXC---
    GT_IF_WITH_ASSERT(NULL != m_pFxcPathCombox)
    {
        QFont* font = new QFont(m_pFxcPathCombox->font());
        font->setBold(true);

        m_pFxcPathCombox->blockSignals(true);
        m_pFxcPathCombox->clear();

        // find default path for FXC compiler file. if found add it's path to the combo
        osFilePath fxcDirPath(KA_STR_HLSL_optionsDialogfxcBuilderFileDirPath);
        osDirectory fxcDir(fxcDirPath);
        foundFilePath = fxcDir.FindFile(KA_STR_HLSL_optionsDialogFXCFileName_GT);

        if (!foundFilePath.isEmpty())
        {
            m_pFxcPathCombox->addItem(acGTStringToQString(foundFilePath));
        }

        // add browse item
        m_pFxcPathCombox->addItem(KA_STR_HLSL_optionsDialogBrowse);

        // set the browse item bold
        m_pFxcPathCombox->setItemData(0, QVariant(*font), Qt::FontRole);

        m_fxcComboLastIndex = 0;

        m_pFxcPathCombox->blockSignals(false);

        delete font;
    }
}

// ---------------------------------------------------------------------------
kaProjectSettingFlagData* kaProjectSettingsShaderExtension::AddCheckBoxOption(QGridLayout* pLayout,
        const QString& checkBoxInfo,
        const QString& checkBoxInfoTooltip,
        int commandFlag)
{
    kaProjectSettingFlagData* retData = NULL;
    QStringList stringList = checkBoxInfo.split("#");

    GT_IF_WITH_ASSERT(stringList.length() == 3)
    {
        // Build formatted tooltip:
        QString tooltipStr;
        acWrapAndBuildFormattedTooltip(stringList[0], checkBoxInfoTooltip, tooltipStr);

        // Create all the components:
        QCheckBox* pCheckBox = new QCheckBox(stringList[0]);

        pCheckBox->setToolTip(tooltipStr);

        // Add both D3d and Fxc labels:

        // Since we set the grid layout horizontal spacing as 0, we add 3 spaces for an artificial spacing
        // (the horizontal 0 spacing is needed to make sure that the table header with the gray color is continous:
        QString str1 = stringList[1];
        str1.append(AF_STR_SpaceA AF_STR_SpaceA AF_STR_SpaceA);

        QString str2 = stringList[2];
        str2.append(AF_STR_SpaceA AF_STR_SpaceA AF_STR_SpaceA);

        QLabel* pD3dLabel = new QLabel(str1);
        QLabel* pFxcLabel = new QLabel(str2);

        pD3dLabel->setDisabled(true);
        pD3dLabel->setToolTip(tooltipStr);
        pFxcLabel->setDisabled(true);
        pFxcLabel->setToolTip(tooltipStr);

        kaProjectSettingFlagData* pFlagData = new kaProjectSettingFlagData;

        // add both D3d and Fxc strings to flag strings list
        pFlagData->m_flagStringList.push_back(stringList[1]);
        pFlagData->m_flagStringList.push_back(stringList[2]);
        pFlagData->m_flagType = kaProjectSettingFlagData::KA_FLAG_TYPE_CHECK_BOX;
        pFlagData->m_pLabels << pD3dLabel;
        pFlagData->m_pLabels << pFxcLabel;
        pFlagData->m_pWidget = pCheckBox;
        pFlagData->m_pChildData = NULL;
        pFlagData->m_pParentData = NULL;
        pFlagData->m_bitFlagsList.append(commandFlag);

        // update the current command string according to the selected compile type
        GT_IF_WITH_ASSERT(NULL != m_pD3dRadio)
        {
            if (IsD3dCompileKind())
            {
                pFlagData->m_previousFlagString = stringList[1];
            }
            else
            {
                pFlagData->m_previousFlagString = stringList[2];
            }
        }

        m_widgetFlagDataMap[pCheckBox] = pFlagData;
        retData = pFlagData;

        // Add the items to the grid:
        int numRows = pLayout->rowCount();
        pLayout->addWidget(pCheckBox, numRows, 0, 1, 1);
        pLayout->addWidget(pD3dLabel, numRows, 1, 1, 1);
        pLayout->addWidget(pFxcLabel, numRows, 2, 1, 1, Qt::AlignLeft);

        connect(pCheckBox, SIGNAL(clicked()), this, SLOT(OnCheckBoxClicked()));
    }

    return retData;
}

// ---------------------------------------------------------------------------
void kaProjectSettingsShaderExtension::DisableCheckBoxWithEmptyOption(CompilerType selectedBuildType)
{
    gtMap<QWidget*, kaProjectSettingFlagData*>::iterator flagDataIterator = m_widgetFlagDataMap.begin();

    for (; flagDataIterator != m_widgetFlagDataMap.end(); flagDataIterator++)
    {
        kaProjectSettingFlagData* pFlagData = (*flagDataIterator).second;

        GT_IF_WITH_ASSERT(NULL != pFlagData)
        {
            if (kaProjectSettingFlagData::KA_FLAG_TYPE_CHECK_BOX == pFlagData->m_flagType)
            {
                GT_IF_WITH_ASSERT(pFlagData->m_pLabels.count() > selectedBuildType)
                {
                    GT_IF_WITH_ASSERT(NULL != pFlagData->m_pWidget)
                    {
                        // if build option flag label is empty for the selected build type - the checkbox should be disabled
                        bool isLabelEmpty = (pFlagData->m_pLabels[selectedBuildType])->text().isEmpty();
                        pFlagData->m_pWidget->setEnabled(!isLabelEmpty);
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
kaProjectSettingFlagData* kaProjectSettingsShaderExtension::AddComboBoxOption(QGridLayout* pLayout,
        const QString& comboBoxInfo,
        const QString& comboBoxInfoTooltip,
        kaProjectSettingFlagData::kaProjectFlagType comboType,
        int defaultIndex,
        QList<int> commandFlagsList)
{
    kaProjectSettingFlagData* retData = NULL;

    GT_IF_WITH_ASSERT(NULL != pLayout)
    {
        QStringList stringList = comboBoxInfo.split("#");

        // the combo label is separated from the rest by '#'
        GT_IF_WITH_ASSERT(stringList.length() == 2)
        {
            kaProjectSettingFlagData* pFlagData = new kaProjectSettingFlagData;

            // Build formatted tooltip:
            QString tooltipStr;
            acWrapAndBuildFormattedTooltip(stringList[0], comboBoxInfoTooltip, tooltipStr);

            // Create all the components:
            QLabel* pLabel = new QLabel(stringList[0]);
            pLabel->setToolTip(tooltipStr);

            // the combo items are separated by ','
            QStringList comboOptionsList = stringList[1].split(",");
            QComboBox* pComboBox = new QComboBox();
            pComboBox->setToolTip(tooltipStr);

            int optionsNum = comboOptionsList.count();
            QStringList optionStringsList;

            // for each option get the text for the combo + flag for d3d option + flag for fxc option
            for (int i = 0; i < optionsNum; i++)
            {
                optionStringsList.clear();
                // the options in the item are seperated by ';' - item label in combo ; d3d command ; fxc command
                optionStringsList = comboOptionsList[i].split(";");
                GT_IF_WITH_ASSERT(optionStringsList.count() == 3)
                {
                    pComboBox->addItem(optionStringsList[0]);
                    pFlagData->m_flagStringList << optionStringsList[1];
                    pFlagData->m_flagStringList << optionStringsList[2];
                }
            }

            // set default index to combobox
            if (defaultIndex < pComboBox->count())
            {
                pFlagData->m_defaultComboIndex = defaultIndex;
            }
            else
            {
                pFlagData->m_defaultComboIndex = 0;
            }

            pComboBox->setCurrentIndex(pFlagData->m_defaultComboIndex);

            pFlagData->m_flagType = comboType;
            pFlagData->m_pWidget = pComboBox;
            pFlagData->m_pChildData = NULL;
            pFlagData->m_pParentData = NULL;
            pFlagData->m_bitFlagsList = commandFlagsList;

            // Add the items to the vectors:
            m_comboBoxVector.push_back(pComboBox);

            m_widgetFlagDataMap[pComboBox] = pFlagData;
            retData = pFlagData;

            // Add the items to the grid:
            int numRows = pLayout->rowCount();
            pLayout->addWidget(pLabel, numRows, 0, 1, 1);
            pLayout->addWidget(pComboBox, numRows, 1, 1, 2);

            connect(pComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboBoxChanged(int)));
        }
    }

    return retData;
}

// ---------------------------------------------------------------------------
void kaProjectSettingsShaderExtension::OnD3dComboxSelectionChanged(int selectedIndex)
{
    GT_IF_WITH_ASSERT(NULL != m_pD3dPathCombox && m_pD3dPathCombox->count() > 0)
    {
        // Item indexes.
        //const int DEFAULT_COMPILER_INDEX = 0;
        //const int PATH_ENV_VAR_INDEX = 1;
        const int BROWSE_INDEX = m_pD3dPathCombox->count() - 1;

        // if browse was clicked
        if (selectedIndex == BROWSE_INDEX)
        {
            OnBrowseButtonClicked(m_pD3dPathCombox, KA_STR_HLSL_optionsDialogD3DFileType, m_d3dComboLastIndex, D3D);
        }
        else
        {
            // else - update the last selected index
            m_d3dComboLastIndex = selectedIndex;

            // set tool tip
            QString selectedItemText = m_pD3dPathCombox->itemText(selectedIndex);

            // if the bundled file is selected - get its path
            if (selectedItemText == KA_STR_HLSL_optionsDialogDefaultCompiler)
            {
                kaApplicationCommands::instance().GetD3DCompilerDefaultDllPath(selectedItemText);
            }

            m_pD3dPathCombox->setToolTip(selectedItemText);
        }

        SetComboSelectedStringBold(m_pD3dPathCombox);
    }
}

// ---------------------------------------------------------------------------
void kaProjectSettingsShaderExtension::OnFxcComboxSelectionChanged(int selectedIndex)
{
    GT_IF_WITH_ASSERT(NULL != m_pFxcPathCombox && m_pFxcPathCombox->count() > 0)
    {
        // browse item in the combo is the last index
        int browseIndex = m_pFxcPathCombox->count() - 1;

        // if browse was clicked
        if (selectedIndex == browseIndex)
        {
            OnBrowseButtonClicked(m_pFxcPathCombox, KA_STR_HLSL_optionsDialogFXCFileType, m_fxcComboLastIndex, FXC);
        }
        else
        {
            // else - update the last selected index
            m_fxcComboLastIndex = selectedIndex;

            // set tool tip
            QString selectedItemText = m_pFxcPathCombox->itemText(selectedIndex);
            m_pFxcPathCombox->setToolTip(selectedItemText);
        }

        SetComboSelectedStringBold(m_pFxcPathCombox);
    }
}

// ---------------------------------------------------------------------------
void kaProjectSettingsShaderExtension::OnBrowseButtonClicked(QComboBox* pCombox, const QString& allowedBrowsedFile, int lastSelectedIndex, CompilerType compilerType)
{
    // open browse dialog box
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
    {
        gtString& browsingPath = GetBrowsingPathByComboType(compilerType);
        QString defaultPath = acGTStringToQString(browsingPath);

        QString dialogCaption = (pCombox == m_pD3dPathCombox) ? KA_STR_HLSL_optionsDialogD3DCompilerSelectionCaption : KA_STR_HLSL_optionsDialogFXCExeSelectionCaption;
        QString selectedFile = pApplicationCommands->ShowFileSelectionDialog(dialogCaption, defaultPath, allowedBrowsedFile, NULL);

        // if the file the user selected is the same kind as browsed
        if (!selectedFile.isEmpty())
        {
            browsingPath = acQStringToGTString(selectedFile);

            bool isValid = SetPathCombobox(pCombox, selectedFile, compilerType);

            if (!isValid)
            {
                // if some error - return to last selected combobox item
                pCombox->setCurrentIndex(lastSelectedIndex);

                acMessageBox::instance().warning(AF_STR_ErrorA, KA_STR_HLSL_optionsDialogFileError, QMessageBox::Ok);
            }
            else
            {
                // set tool tip
                pCombox->setToolTip(acGTStringToQString(browsingPath));
            }
        }
        else
        {
            // if some error - return to last selected combobox item
            pCombox->setCurrentIndex(lastSelectedIndex);
        }
    }
}

// ---------------------------------------------------------------------------
gtString& kaProjectSettingsShaderExtension::GetBrowsingPathByComboType(CompilerType compilerType)
{
    gtString& ret = m_sD3dDllBrowsingPath;

    if (compilerType == FXC)
    {
        ret = m_sFxcExeBrowsingPath;
    }

    return ret;
}

// ---------------------------------------------------------------------------
bool kaProjectSettingsShaderExtension::ValidateD3DPath(const osFilePath& path) const
{
    // check file exist
    osFile file(path);
    bool isValid = file.exists();

    if (isValid)
    {
        // compare to valid file name and extension
        QRegExp rx(KA_STR_HLSL_optionsDialogD3DFileName_QT, Qt::CaseInsensitive);
        rx.setPatternSyntax(QRegExp::Wildcard);

        gtString gtStr;
        path.getFileName(gtStr);
        QString qFullName(acGTStringToQString(gtStr));

        qFullName.append(".");

        path.getFileExtension(gtStr);
        qFullName.append(acGTStringToQString(gtStr));

        isValid = rx.exactMatch(qFullName);
    }

    return isValid;
}

// ---------------------------------------------------------------------------
bool kaProjectSettingsShaderExtension::ValidateFXCPath(const osFilePath& path) const
{
    // check file exist
    osFile file(path);
    bool isValid = file.exists();

    if (isValid)
    {
        // compare to valid file name and extension
        gtString fileName;
        path.getFileName(fileName);
        fileName.append('.');

        gtString fileExt;
        path.getFileExtension(fileExt);
        fileName.append(fileExt);

        isValid = (fileName.compare(KA_STR_HLSL_optionsDialogFXCFileName_GT) == 0);
    }

    return isValid;
}

// ---------------------------------------------------------------------------
void kaProjectSettingsShaderExtension::OnCompileTypeSelectionChanged(int compileKind)
{
    GT_UNREFERENCED_PARAMETER(compileKind);

    GT_IF_WITH_ASSERT(NULL != m_pD3dRadio)
    {
        bool isD3dCompileKind = IsD3dCompileKind();

        GT_IF_WITH_ASSERT(NULL != m_pD3dPathCombox && NULL != m_pFxcPathCombox)
        {
            m_pD3dPathCombox->setEnabled(isD3dCompileKind);
            m_pFxcPathCombox->setEnabled(!isD3dCompileKind);
        }

        // update the command line
        // Pass on all the flags and look in the options list if it is there
        gtMap<QWidget*, kaProjectSettingFlagData*>::iterator flagDataIterator;
        QString command;

        // get the compile kind
        int usedFlagsListIndex;

        if (isD3dCompileKind)
        {
            usedFlagsListIndex = 0;
        }
        else
        {
            usedFlagsListIndex = 1;
        }

        for (flagDataIterator = m_widgetFlagDataMap.begin(); flagDataIterator != m_widgetFlagDataMap.end(); flagDataIterator++)
        {
            kaProjectSettingFlagData* pFlagData = (*flagDataIterator).second;

            GT_IF_WITH_ASSERT(NULL != pFlagData)
            {
                if (kaProjectSettingFlagData::KA_FLAG_TYPE_CHECK_BOX == pFlagData->m_flagType)
                {
                    // get the string related to the selected compile kind using the usedFlagsListIndex
                    pFlagData->m_previousFlagString = pFlagData->m_flagStringList[usedFlagsListIndex];
                    QCheckBox* pCheckBox = qobject_cast<QCheckBox*>(pFlagData->m_pWidget);

                    GT_IF_WITH_ASSERT(NULL != pCheckBox)
                    {
                        if (pCheckBox->checkState() == Qt::Checked)
                        {
                            if (!command.isEmpty() && !pFlagData->m_previousFlagString.isEmpty())
                            {
                                command.append(" ");
                            }

                            command.append(pFlagData->m_previousFlagString);
                        }
                    }
                }
                // not handles the case of joined combo
                else if (kaProjectSettingFlagData::KA_FLAG_TYPE_COMBO_BOX == pFlagData->m_flagType)
                {
                    // get the index in strings list for default - indexes of m_flagStringList: odd - for D3D items, even - for FXC items
                    int comboDefaultInList = pFlagData->m_defaultComboIndex * 2 + usedFlagsListIndex;
                    GT_IF_WITH_ASSERT(comboDefaultInList < pFlagData->m_flagStringList.count())
                    {
                        pFlagData->m_previousFlagString = pFlagData->m_flagStringList[comboDefaultInList];
                    }

                    QComboBox* pCombox = qobject_cast<QComboBox*>(pFlagData->m_pWidget);

                    GT_IF_WITH_ASSERT(NULL != pCombox)
                    {
                        int index = pCombox->currentIndex();

                        if (index < 0 || index > pCombox->count() - 1)
                        {
                            // set default index
                            index = pFlagData->m_defaultComboIndex;
                        }

                        // get the string related to the selected compile kind using the usedFlagsListIndex
                        pFlagData->m_previousFlagString = pFlagData->m_flagStringList[index * 2 + usedFlagsListIndex];

                        if (!pFlagData->m_previousFlagString.isEmpty())
                        {
                            if (!command.isEmpty())
                            {
                                command.append(" ");
                            }

                            command.append(pFlagData->m_previousFlagString);
                        }
                    }
                }
                else if (kaProjectSettingFlagData::KA_FLAG_TYPE_LINE_EDIT == pFlagData->m_flagType)
                {
                    QLineEdit* pLineEdit = qobject_cast<QLineEdit*>(pFlagData->m_pWidget);

                    GT_IF_WITH_ASSERT(NULL != pLineEdit)
                    {
                        pFlagData->m_previousFlagString = pLineEdit->text();

                        if (!IsD3dCompileKind())
                        {
                            if (!command.isEmpty())
                            {
                                command.append(" ");
                            }

                            command.append(pFlagData->m_previousFlagString);
                        }
                    }
                }
            }
        }

        // update the command edit line
        if (NULL != m_pCommandTextEdit)
        {
            m_pCommandTextEdit->blockSignals(true);

            m_pCommandTextEdit->clear();
            m_pCommandTextEdit->setText(command);

            m_pCommandTextEdit->blockSignals(false);
        }

        UpdateCommandLabelWithMask();

        CompilerType selectedType = isD3dCompileKind ? D3D : FXC;
        DisableCheckBoxWithEmptyOption(selectedType);
    }
}

// ---------------------------------------------------------------------------
void kaProjectSettingsShaderExtension::OnCompileTypeChangedFromXML()
{
    GT_IF_WITH_ASSERT(NULL != m_pD3dRadio)
    {
        // get shader compile type from manager (xml) and set the relevant radio button
        QString compileType = KA_PROJECT_DATA_MGR_INSTANCE.ShaderCompileType();

        GT_IF_WITH_ASSERT(NULL != m_pCompileKindRadioGroup && NULL != m_pD3dRadio && NULL != m_pFxcRadio)
        {
            m_pCompileKindRadioGroup->blockSignals(true);

            if (compileType == KA_STR_HLSL_optionsDialogD3DCompileType)
            {
                m_pD3dRadio->setChecked(true);
            }
            else
            {
                m_pFxcRadio->setChecked(true);
            }

            m_pCompileKindRadioGroup->blockSignals(false);

            OnCompileTypeSelectionChanged(0);
        }
    }
}

// ---------------------------------------------------------------------------
int kaProjectSettingsShaderExtension::EditBoxColumnSpan()
{
    return KA_TEXT_EDIT_BOX_COLUMN_SPAN;
}

// ---------------------------------------------------------------------------
int kaProjectSettingsShaderExtension::GetFlagIndex()
{
    int flagIndex;

    if (IsD3dCompileKind())
    {
        flagIndex = 0;
    }
    else
    {
        flagIndex = 1;
    }

    return flagIndex;
}

// ---------------------------------------------------------------------------
void kaProjectSettingsShaderExtension::HandleComboBoxInText(kaProjectSettingFlagData* pFlagData, QStringList& optionsList)
{
    // update the current command string according to the selected compile type

    QComboBox* pComboBox = qobject_cast<QComboBox*>(pFlagData->m_pWidget);

    GT_IF_WITH_ASSERT(NULL != pComboBox)
    {
        int foundString = -1;

        int numStrings = pFlagData->m_flagStringList.count();
        int firstIndexByCompileType = 0;

        // get the selected compile kind
        if (IsD3dCompileKind())
        {
            firstIndexByCompileType = 0;
        }
        else
        {
            firstIndexByCompileType = 1;
        }

        int foundIndex = pFlagData->m_defaultComboIndex * 2 + firstIndexByCompileType;

        // Start from 1 and not from 0 since the 0 contains the "default" empty string
        for (int nString = firstIndexByCompileType; nString < numStrings; nString++)
        {
            foundString = optionsList.indexOf(pFlagData->m_flagStringList[nString]);

            if (foundString != -1)
            {
                foundIndex = nString;
                optionsList.removeAt(foundString);
                break;
            }

            nString++;
        }

        // set combo item as selected. indexes of m_flagStringList: odd - for D3D items, even - for FXC items
        int comboSelectedIndex = (foundIndex - firstIndexByCompileType) / 2;
        GT_IF_WITH_ASSERT(comboSelectedIndex < pComboBox->count())
        {
            pComboBox->blockSignals(true);
            pComboBox->setCurrentIndex(comboSelectedIndex);
            pComboBox->blockSignals(false);
        }

        pFlagData->m_previousFlagString = pFlagData->m_flagStringList[foundIndex];
    }
}

// ---------------------------------------------------------------------------
void kaProjectSettingsShaderExtension::OnComboBoxChanged(int index)
{
    // Get the control that initiated the event:
    QComboBox* pComboBox = qobject_cast<QComboBox*>(sender());

    GT_IF_WITH_ASSERT(NULL != pComboBox)
    {
        kaProjectSettingFlagData* pFlagData = m_widgetFlagDataMap[pComboBox];
        GT_IF_WITH_ASSERT(NULL != pFlagData && NULL != m_pD3dRadio)
        {
            // get the selected compile kind
            int nString;

            if (IsD3dCompileKind())
            {
                nString = 0;
            }
            else
            {
                nString = 1;
            }

            if (index < 0 || index > pComboBox->count() - 1)
            {
                // set default index
                index = pFlagData->m_defaultComboIndex;
            }

            QString textToShow = pFlagData->m_flagStringList[index * 2 + nString];

            ReplaceText(pFlagData, textToShow);

            pFlagData->m_previousFlagString = textToShow;
        }
    }
}

// ---------------------------------------------------------------------------
void kaProjectSettingsShaderExtension::HandleTextEditInText(kaProjectSettingFlagData* pFlagData, QStringList& optionsList)
{
    if (!IsD3dCompileKind())
    {
        kaProjectSettingsExtensionBase::HandleTextEditInText(pFlagData, optionsList);
    }
}

// ---------------------------------------------------------------------------
void kaProjectSettingsShaderExtension::OnTextChange(const QString& lineText)
{
    if (!IsD3dCompileKind())
    {
        kaProjectSettingsExtensionBase::OnTextChange(lineText);
    }
}

// ---------------------------------------------------------------------------
bool kaProjectSettingsShaderExtension::IsD3dCompileKind()
{
    bool retval = false;
    GT_IF_WITH_ASSERT(NULL != m_pD3dRadio)
    {
        retval = m_pD3dRadio->isChecked();
    }
    return retval;
}

// ---------------------------------------------------------------------------
void kaProjectSettingsShaderExtension::OnCommandTextChanged()
{
    // call base function
    kaProjectSettingsExtensionBase::OnCommandTextChanged();

    // update command box label with mask
    UpdateCommandLabelWithMask();
}

// ---------------------------------------------------------------------------
unsigned int kaProjectSettingsShaderExtension::GetD3dBuildptionsMask()
{
    unsigned int commandMask = 0;
    gtMap<QWidget*, kaProjectSettingFlagData*>::iterator flagDataIterator;

    if (IsD3dCompileKind())
    {
        //set mask
        for (flagDataIterator = m_widgetFlagDataMap.begin(); flagDataIterator != m_widgetFlagDataMap.end(); flagDataIterator++)
        {
            kaProjectSettingFlagData* pFlagData = (*flagDataIterator).second;

            GT_IF_WITH_ASSERT(NULL != pFlagData)
            {
                if (kaProjectSettingFlagData::KA_FLAG_TYPE_CHECK_BOX == pFlagData->m_flagType)
                {
                    QCheckBox* checkbox = qobject_cast<QCheckBox*>(pFlagData->m_pWidget);
                    GT_IF_WITH_ASSERT(NULL != checkbox)
                    {
                        // get flag mask for checked check-boxes
                        if (checkbox->isChecked())
                        {
                            if (pFlagData->m_bitFlagsList.count() > 0 && pFlagData->m_bitFlagsList[0] != 0)
                            {
                                commandMask |= pFlagData->m_bitFlagsList[0];
                            }
                        }
                    }
                }
                else if (kaProjectSettingFlagData::KA_FLAG_TYPE_COMBO_BOX == pFlagData->m_flagType)
                {
                    QComboBox* combobox = qobject_cast<QComboBox*>(pFlagData->m_pWidget);
                    GT_IF_WITH_ASSERT(NULL != combobox)
                    {
                        // get flag mask for selected combo item
                        int selectedItemIndex = combobox->currentIndex();
                        {
                            if (pFlagData->m_bitFlagsList.count() > selectedItemIndex && pFlagData->m_bitFlagsList[selectedItemIndex] != 0)
                            {
                                commandMask |= pFlagData->m_bitFlagsList[selectedItemIndex];
                            }
                        }
                    }
                }
            }
        }
    }

    return commandMask;
}

// ---------------------------------------------------------------------------
void kaProjectSettingsShaderExtension::UpdateCommandLabelWithMask()
{
    QString header = KA_STR_HLSL_optionsDialogBuildCommand;

    if (IsD3dCompileKind())
    {
        // update command box label - only for D3D compile type
        unsigned int commandMask = GetD3dBuildptionsMask();

        // set mask to label
        header.append(" - 0X");
        QString hex = QString::number(commandMask, 16);
        header.append(hex);
    }

    GT_IF_WITH_ASSERT(NULL != m_pCommandLineLabel)
    {
        m_pCommandLineLabel->setText(header);
    }
}

// ---------------------------------------------------------------------------
QString kaProjectSettingsShaderExtension::GetBuildOptionString()
{
    QString stringToParse;

    kaProjectDataManager* pManager = qobject_cast<kaProjectDataManager*>(sender());

    if (NULL != pManager)
    {
        stringToParse = pManager->ShaderBuildOptions();
    }

    return stringToParse;
}

// ---------------------------------------------------------------------------
void kaProjectSettingsShaderExtension::SetComboSelectedStringBold(QComboBox* pCombo)
{
    GT_IF_WITH_ASSERT(pCombo != NULL)
    {
        unsigned int count = pCombo->count();

        // get items regular font
        QFont font = m_pD3dPathCombox->font();
        font.setBold(false);

        // set all items with regular font
        for (unsigned int i = 0; i < count; i++)
        {
            pCombo->setItemData(i, QVariant(font), Qt::FontRole);
        }

        // set selected item with bold font
        font.setBold(true);
        pCombo->setItemData(pCombo->currentIndex(), QVariant(font), Qt::FontRole);
    }
}

void kaProjectSettingsShaderExtension::UpdateProjectDataManagerWithTBOptions(const QString& value, QStringList& optionsList)
{

    KA_PROJECT_DATA_MGR_INSTANCE.SetShaderBuildOptions(value);

    // if there is strings that not found a matching flag, add them to the avoidable list
    // only relevant for shaders build
    m_avoidableStringsList.clear();

    for (int i = 0; i < optionsList.count(); i++)
    {
        m_avoidableStringsList << optionsList[i];
    }

    KA_PROJECT_DATA_MGR_INSTANCE.SetShaderAvoidableBuildOptions(m_avoidableStringsList);

}

void kaProjectSettingsShaderExtension::UpdateProjectDataManagerWithTBOptions(const QString& value)
{
    QString oldCommands = KA_PROJECT_DATA_MGR_INSTANCE.ShaderBuildOptions();

    if (oldCommands != value)
    {
        KA_PROJECT_DATA_MGR_INSTANCE.SetShaderBuildOptions(value);
    }

}
