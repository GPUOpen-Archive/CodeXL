//------------------------------ kaProjectSettingsExtensionOther.h ------------------------------

// TinyXml:
#include <tinyxml.h>

// QT:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationFramework/Include/afCSSSettings.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaBuildToolbar.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaProjectSettingsExtensionOther.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtensionOther::kaProjectSettingsExtensionOther
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
kaProjectSettingsExtensionOther::kaProjectSettingsExtensionOther()
{

}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtensionOther::~gdProjectSettingsExtension
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
kaProjectSettingsExtensionOther::~kaProjectSettingsExtensionOther()
{
}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtensionOther::initialize
// Description: Create the widget that is reading the debug setting for the debugger
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
void kaProjectSettingsExtensionOther::Initialize()
{
    m_pGridLayout = new QGridLayout(this);

    m_pGridLayout->setContentsMargins(0, 0, 0, 0);

    GenerateOptionsTable();

    m_pGridWidget = new QWidget;

    m_pGridWidget->setLayout(m_pGridLayout);

    // Create the command text control:
    m_pCommandTextEdit = new QTextEdit;

    kaProjectSettingsExtensionBase::connect(m_pCommandTextEdit, SIGNAL(textChanged()), (kaProjectSettingsExtensionBase*)this, SLOT(OnCommandTextChanged()));
    kaProjectSettingsExtensionBase::connect(&KA_PROJECT_DATA_MGR_INSTANCE, SIGNAL(BuildOptionsChanged()), this, SLOT(OnCommandTextChanged()));

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

    m_pCommandTextEdit->setStyleSheet("QTextEdit { border: 1px solid Gray; background-color: white;}");

    setLayout(pMainLayout);
}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtensionOther::ExtensionXMLString
// Description: Return the extension string
// Return Val:  gtString&
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
gtString kaProjectSettingsExtensionOther::ExtensionXMLString()
{
    gtString retVal = KA_STR_projectSettingExtensionNameOther;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtensionOther::extensionDisplayName
// Description: Return the display name for the extension
// Return Val:  gtString
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
gtString kaProjectSettingsExtensionOther::ExtensionTreePathAsString()
{
    gtString retVal = KA_STR_projectSettingExtensionOtherDisplayName;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtensionOther::getXMLSettingsString
// Description: Get the current debugger project settings as XML string
// Arguments:   gtASCIIString& projectAsXMLString
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
bool kaProjectSettingsExtensionOther::GetXMLSettingsString(gtString& projectAsXMLString)
{
    bool retVal = true;

    projectAsXMLString.appendFormattedString(L"<%ls>", ExtensionXMLString().asCharArray());
    projectAsXMLString.appendFormattedString(L"</%ls>", ExtensionXMLString().asCharArray());

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtensionOther::setSettingsFromXMLString
// Description: Get the project settings from the XML string
// Arguments:   const gtASCIIString& projectAsXMLString
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
bool kaProjectSettingsExtensionOther::SetSettingsFromXMLString(const gtString& projectAsXMLString)
{
    GT_UNREFERENCED_PARAMETER(projectAsXMLString);

    bool retVal = true;

    // The other page is a dummy page from the XML point of view. It does not write/read anything

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtensionOther::SaveCurrentSettings
// Description: Get the current project settings from the controls, and store into
//              the current project properties
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
bool kaProjectSettingsExtensionOther::SaveCurrentSettings()
{
    bool retVal = true;

    // The other page is a dummy page from the XML point of view. It does not write/read anything

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtensionOther::RestoreDefaultProjectSettings
// Description: Restore default project settings
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
void kaProjectSettingsExtensionOther::RestoreDefaultProjectSettings()
{
    // The other page is a dummy page from the XML point of view. It does not write/read anything
}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtensionOther::AreSettingsValid
// Description: Check if the current settings are valid
// Arguments:   gtString& invalidMessageStr
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
bool kaProjectSettingsExtensionOther::AreSettingsValid(gtString& invalidMessageStr)
{
    GT_UNREFERENCED_PARAMETER(invalidMessageStr);

    // No project setting page at this stage for this extension
    bool retVal = true;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtensionOther::RestoreCurrentSettings
// Description: Load the current settings to the displayed widgets
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
bool kaProjectSettingsExtensionOther::RestoreCurrentSettings()
{
    bool retVal = true;

    // The other page is a dummy page from the XML point of view. It does not write/read anything

    return retVal;
}

// ---------------------------------------------------------------------------
void kaProjectSettingsExtensionOther::GenerateOptionsTable()
{
    // Add the "other" section:
    QLabel* pOtherLabel = new QLabel(KA_STR_optionsDialogOtherSection);

    pOtherLabel->setStyleSheet(AF_STR_captionLabelStyleSheetMain);

    int numRows = m_pGridLayout->rowCount();
    m_pGridLayout->addWidget(pOtherLabel, numRows, 0, 1, 2);

    AddTextEditOption(m_pGridLayout, KA_STR_optionsDialogClVersionText, "", kaProjectSettingFlagData::KA_FLAG_TYPE_LINE_EDIT_SINGLE);
    AddCheckBoxOption(m_pGridLayout, KA_STR_optionsDialogKernelArgumentCheck, KA_STR_optionsDialogKernelArgumentCheckTooltip);
    AddCheckBoxOption(m_pGridLayout, KA_STR_optionsDialogCreateLibraryCheck, KA_STR_optionsDialogCreateLibraryCheckTooltip);
    AddCheckBoxOption(m_pGridLayout, KA_STR_optionsDialogEnableLinkCheck, KA_STR_optionsDialogEnableLinkCheckTooltip);
    AddCheckBoxOption(m_pGridLayout, KA_STR_optionsDialogProduceDebuggingCheck, KA_STR_optionsDialogProduceDebuggingCheckTooltip);
    AddComboBoxOption(m_pGridLayout, KA_STR_optionsDialogSpecifyUAVCombo, KA_STR_optionsDialogSpecifyUAVComboTooltip);
    AddComboBoxOption(m_pGridLayout, KA_STR_optionsDialogOpenCLBif3Combo, KA_STR_optionsDialogOpenCLBif3ComboTooltip);
    AddComboBoxOption(m_pGridLayout, KA_STR_optionsDialogEncryptedOpenCLCombo, KA_STR_optionsDialogEncryptedOpenCLComboTooltip);
    AddComboBoxOption(m_pGridLayout, KA_STR_optionsDialogUseJITCombo, KA_STR_optionsDialogUseJITComboTooltip);
    AddComboBoxOption(m_pGridLayout, KA_STR_optionsDialogForUseJITCombo, KA_STR_optionsDialogForUseJITComboTooltip);
    AddComboBoxOption(m_pGridLayout, KA_STR_optionsDialogDisableAVXCombo, KA_STR_optionsDialogDisableAVXComboTooltip);
    AddComboBoxOption(m_pGridLayout, KA_STR_optionsDialogEnablefmaCombo, KA_STR_optionsDialogEnablefmaComboTooltip);
    kaProjectSettingFlagData* pParent = AddCheckBoxOption(m_pGridLayout, KA_STR_optionsDialogStoreTempCheck, KA_STR_optionsDialogStoreTempCheckTooltip);
    AddTextEditOption(m_pGridLayout, KA_STR_optionsDialogTempPrefixText, KA_STR_optionsDialogTempPrefixTextTooltip, kaProjectSettingFlagData::KA_FLAG_TYPE_LINE_EDIT, pParent);
    pParent = AddCheckBoxOption(m_pGridLayout, KA_STR_optionsDialogReplaceMathFunctionCheck, KA_STR_optionsDialogReplaceMathFunctionCheckTooltip);
    AddTextEditOption(m_pGridLayout, KA_STR_optionsDialogNativeVersionText, KA_STR_optionsDialogNativeVersionTextTooltip, kaProjectSettingFlagData::KA_FLAG_TYPE_LINE_EDIT, pParent);
}

void kaProjectSettingsExtensionOther::UpdateProjectDataManagerWithTBOptions(const QString& value, QStringList& optionsList)
{
    GT_UNREFERENCED_PARAMETER(optionsList);
    GT_UNREFERENCED_PARAMETER(value);

}
