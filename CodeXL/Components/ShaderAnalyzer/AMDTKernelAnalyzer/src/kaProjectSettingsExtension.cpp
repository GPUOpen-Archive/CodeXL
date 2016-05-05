//------------------------------ kaProjectSettingsExtension.h ------------------------------

// TinyXml:
#include <tinyxml.h>

// QT:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCSSSettings.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaBuildToolbar.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaProjectSettingsExtension.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtension::kaProjectSettingsExtension
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
kaProjectSettingsExtension::kaProjectSettingsExtension() : m_pMacrosLineEdit(nullptr)
{

}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtension::~gdProjectSettingsExtension
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
kaProjectSettingsExtension::~kaProjectSettingsExtension()
{
}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtension::initialize
// Description: Create the widget that is reading the debug setting for the debugger
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
void kaProjectSettingsExtension::Initialize()
{
    m_pGridLayout = new QGridLayout(this);

    m_pGridLayout->setContentsMargins(0, 0, 0, 0);

    GenerateOptionsTable();

    m_pGridWidget = new QWidget;

    m_pGridWidget->setLayout(m_pGridLayout);

    // Create the command text control:
    m_pCommandTextEdit = new QTextEdit;

    bool rc = kaProjectSettingsExtensionBase::connect(m_pCommandTextEdit, SIGNAL(textChanged()), this, SLOT(OnCommandTextChanged()));
    GT_ASSERT(rc);
    rc = kaProjectSettingsExtensionBase::connect(&KA_PROJECT_DATA_MGR_INSTANCE, SIGNAL(BuildOptionsChanged()), this, SLOT(OnCommandTextChanged()));
    GT_ASSERT(rc);
    kaApplicationCommands::instance().SetCommandTextEdit(m_pCommandTextEdit);

    QString dummyText = "first \n second \n third\n forth";
    QRect commandLineTextRect = QFontMetrics(m_pCommandTextEdit->font()).boundingRect(dummyText);
    m_pCommandTextEdit->resize(-1, commandLineTextRect.height());

    QLabel* pInfoLabel = new QLabel(KA_STR_optionsDialogEditCaption);

    pInfoLabel->setStyleSheet(AF_STR_captionLabelStyleSheet);

    // Create the main layout and add the controls to it:
    QVBoxLayout* pMainLayout = new QVBoxLayout;


    pMainLayout->addWidget(m_pGridWidget);
    pMainLayout->addStretch();

    QVBoxLayout* pBottomLayout = new QVBoxLayout;

    pBottomLayout->addWidget(pInfoLabel, 0, Qt::AlignBottom);
    pBottomLayout->addWidget(m_pCommandTextEdit, 1, Qt::AlignBottom);
    pBottomLayout->setContentsMargins(0, 0, 0, 0);
    pMainLayout->addLayout(pBottomLayout);

    m_pCommandTextEdit->setStyleSheet(AF_STR_grayBorderWhiteBackgroundTE);

    setLayout(pMainLayout);
}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtension::ExtensionXMLString
// Description: Return the extension string
// Return Val:  gtString&
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
gtString kaProjectSettingsExtension::ExtensionXMLString()
{
    gtString retVal = KA_STR_projectSettingExtensionName;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtension::extensionDisplayName
// Description: Return the display name for the extension
// Return Val:  gtString
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
gtString kaProjectSettingsExtension::ExtensionTreePathAsString()
{
    gtString retVal = KA_STR_projectSettingExtensionDisplayName;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtension::getXMLSettingsString
// Description: Get the current debugger project settings as XML string
// Arguments:   gtASCIIString& projectAsXMLString
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
bool kaProjectSettingsExtension::GetXMLSettingsString(gtString& projectAsXMLString)
{
    bool retVal = false;

    projectAsXMLString.appendFormattedString(L"<%ls>", ExtensionXMLString().asCharArray());

    retVal = KA_PROJECT_DATA_MGR_INSTANCE.getXMLSettingsString(projectAsXMLString);

    projectAsXMLString.appendFormattedString(L"</%ls>", ExtensionXMLString().asCharArray());

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtension::setSettingsFromXMLString
// Description: Get the project settings from the XML string
// Arguments:   const gtASCIIString& projectAsXMLString
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
bool kaProjectSettingsExtension::SetSettingsFromXMLString(const gtString& projectAsXMLString)
{
    bool retVal = false;

    TiXmlNode* pKANode = new TiXmlElement(ExtensionXMLString().asASCIICharArray());


    QString projectAsQtXML = acGTStringToQString(projectAsXMLString);
    QByteArray projectAsQtXMLAsUTF8 = projectAsQtXML.toUtf8();

    pKANode->Parse(projectAsQtXMLAsUTF8.data(), 0, TIXML_DEFAULT_ENCODING);
    gtString kaNodeTitle;
    kaNodeTitle.fromASCIIString(pKANode->Value());

    if (ExtensionXMLString() == kaNodeTitle.asCharArray())
    {
        retVal = KA_PROJECT_DATA_MGR_INSTANCE.setSettingsFromXMLString(projectAsXMLString, pKANode);

        if (retVal)
        {
            m_originalBuildOptions = KA_PROJECT_DATA_MGR_INSTANCE.BuildOptions();
        }
    }

    QString macrosStr = KA_PROJECT_DATA_MGR_INSTANCE.KernelMacros();
    GT_IF_WITH_ASSERT(nullptr != m_pMacrosLineEdit)
    {
        m_pMacrosLineEdit->setText(macrosStr);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtension::SaveCurrentSettings
// Description: Get the current project settings from the controls, and store into
//              the current project properties
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
bool kaProjectSettingsExtension::SaveCurrentSettings()
{
    bool retVal = true;

    // Save the command text value to the toolbar in the none VS mode
    // and to the data manager:
    auto pActiveProgram = KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgram();

    if (pActiveProgram != nullptr && pActiveProgram->GetBuildType() != kaProgramDX)
    {
        // Save the text for next time we'll need the original text:
        m_originalBuildOptions = m_pCommandTextEdit->toPlainText();
        KA_PROJECT_DATA_MGR_INSTANCE.SetKernelBuildOptions(m_originalBuildOptions);
        kaApplicationCommands::instance().SetToolbarBuildOptions(m_originalBuildOptions);

    }

    // set macros to project data
    GT_IF_WITH_ASSERT(nullptr != m_pMacrosLineEdit)
    {
        KA_PROJECT_DATA_MGR_INSTANCE.SetKernelMacros(m_pMacrosLineEdit->text());
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtension::RestoreDefaultProjectSettings
// Description: Restore default project settings
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
void kaProjectSettingsExtension::RestoreDefaultProjectSettings()
{
    if (nullptr != m_pCommandTextEdit)
    {
        m_pCommandTextEdit->setPlainText("");
    }

    m_originalBuildOptions.clear();

    // Restore the data manager information, it will restore the edit box data:
    KA_PROJECT_DATA_MGR_INSTANCE.setBuildOptions("");
    KA_PROJECT_DATA_MGR_INSTANCE.SetShaderBuildOptions("");
}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtension::AreSettingsValid
// Description: Check if the current settings are valid
// Arguments:   gtString& invalidMessageStr
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
bool kaProjectSettingsExtension::AreSettingsValid(gtString& invalidMessageStr)
{
    GT_UNREFERENCED_PARAMETER(invalidMessageStr);

    // No project setting page at this stage for this extension
    bool retVal = true;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtension::RestoreCurrentSettings
// Description: Load the current settings to the displayed widgets
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
bool kaProjectSettingsExtension::RestoreCurrentSettings()
{
    bool retVal = true;

    // Restore the data manager information, it will restore the edit box data:

    kaProgram* pActiveProgram = KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgram();

    // restore the toolbar information:
    if (pActiveProgram != nullptr && pActiveProgram->GetBuildType() == kaProgramCL)
    {
        KA_PROJECT_DATA_MGR_INSTANCE.setBuildOptions(m_originalBuildOptions);
        kaApplicationCommands::instance().SetToolbarBuildOptions(m_originalBuildOptions);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void kaProjectSettingsExtension::GenerateOptionsTable()
{
    // Create the first section:
    QLabel* pGeneralLabel = new QLabel(KA_STR_optionsDialogGeneralSection);

    pGeneralLabel->setStyleSheet(AF_STR_captionLabelStyleSheetMain);

    m_pGridLayout->addWidget(pGeneralLabel, 1, 0, 1, 2);
    kaProjectSettingFlagData* pDataFlag  = AddTextEditOption(m_pGridLayout, KA_STR_optionsDialogPredefinedText, KA_STR_optionsDialogPredefinedTextTooltip);
    m_pMacrosLineEdit = qobject_cast<QLineEdit*>(pDataFlag->m_pWidget);

    AddTextEditOption(m_pGridLayout, KA_STR_optionsDialogAdditionalText, KA_STR_optionsDialogAdditionalTextTooltip);
    AddComboBoxOption(m_pGridLayout, KA_STR_optionsDialogOpenCLFormatCombo, KA_STR_optionsDialogOpenCLFormatComboTooltip, kaProjectSettingFlagData::KA_FLAG_TYPE_COMBO_BOX_JOINED);
    AddCheckBoxOption(m_pGridLayout, KA_STR_optionsDialogDisableWarningCheck, KA_STR_optionsDialogDisableWarningCheckTooltip);
    AddCheckBoxOption(m_pGridLayout, KA_STR_optionsDialogTreatAsErrorCheck, KA_STR_optionsDialogTreatAsErrorCheckTooltip);

    // Create the second section:
    QLabel* pOptimizeLabel = new QLabel(KA_STR_optionsDialogOptimizationSection);

    pOptimizeLabel->setStyleSheet(AF_STR_captionLabelStyleSheet);
    int numRows = m_pGridLayout->rowCount();
    m_pGridLayout->addWidget(pOptimizeLabel, numRows, 0, 1, 2);

    AddComboBoxOption(m_pGridLayout, KA_STR_optionsDialogOptimizationCombo, "");
    AddCheckBoxOption(m_pGridLayout, KA_STR_optionsDialogTreatDoubleCheck, KA_STR_optionsDialogTreatDoubleCheckTooltip);
    AddCheckBoxOption(m_pGridLayout, KA_STR_optionsDialogFlushCheck, KA_STR_optionsDialogFlushCheckTooltip);
    AddCheckBoxOption(m_pGridLayout, KA_STR_optionsDialogCompilerAssumesCheck, KA_STR_optionsDialogCompilerAssumesCheckTooltip);
    AddCheckBoxOption(m_pGridLayout, KA_STR_optionsDialogEnableMADCheck, KA_STR_optionsDialogEnableMADCheckTooltip);
    AddCheckBoxOption(m_pGridLayout, KA_STR_optionsDialogIgnoreSignednessCheck, KA_STR_optionsDialogIgnoreSignednessCheckTooltip);
    AddCheckBoxOption(m_pGridLayout, KA_STR_optionsDialogAllowUnsafeCheck, KA_STR_optionsDialogAllowUnsafeCheckTooltip);
    AddCheckBoxOption(m_pGridLayout, KA_STR_optionsDialogAssumeNaNCheck, KA_STR_optionsDialogAssumeNaNCheckTooltip);
    AddCheckBoxOption(m_pGridLayout, KA_STR_optionsDialogAggressiveMathCheck, KA_STR_optionsDialogAggressiveMathCheckTooltip);
    AddCheckBoxOption(m_pGridLayout, KA_STR_optionsDialogCorrectlyRoundCheck, KA_STR_optionsDialogCorrectlyRoundCheckTooltip);
}

void kaProjectSettingsExtension::UpdateProjectDataManagerWithTBOptions(const QString& value, QStringList& optionsList)
{
    GT_UNREFERENCED_PARAMETER(optionsList);
    auto pActiveProgram = KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgram();

    if (m_isBasicBuildOption && pActiveProgram != nullptr && kaProgramDX != pActiveProgram->GetBuildType())
    {
        KA_PROJECT_DATA_MGR_INSTANCE.setBuildOptions(value);
    }
}

void kaProjectSettingsExtension::UpdateProjectDataManagerWithTBOptions(const QString& value)
{
    // If just updating the text make sure that the project manager text is also updated and it will also update the
    // second text box in the second setting page
    QString oldCommands = KA_PROJECT_DATA_MGR_INSTANCE.BuildOptions();

    if (oldCommands != value)
    {
        KA_PROJECT_DATA_MGR_INSTANCE.setBuildOptions(value);
    }

}
