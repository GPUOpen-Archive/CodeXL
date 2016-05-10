//------------------------------ cpProjectSettingsExtension.h ------------------------------

#include <qtIgnoreCompilerWarnings.h>

// TinyXml:
#include <tinyxml.h>

// QT:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afCSSSettings.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// Local:
#include <AMDTGpuProfiling/gpProjectSettingsExtension.h>
#include <AMDTGpuProfiling/ProfileManager.h>
#include <AMDTGpuProfiling/gpExecutionMode.h>

#define GP_LINE_EDIT_MARGIN 15

gpProjectSettingsExtension::gpProjectSettingsExtension() :
    m_pAutomaticCheckBox(nullptr),
    m_pOptionsComboBox(nullptr),
    m_pOptionsEdit(nullptr),
    m_pPortLineEdit(nullptr),
    m_bUpdateSettings(false),
    m_processName(""),
    m_processNumber("1"),
    m_validator(1, 100, this)
{
}

gpProjectSettingsExtension::~gpProjectSettingsExtension()
{
}

void gpProjectSettingsExtension::Initialize()
{
    QVBoxLayout* pMainLayout = new QVBoxLayout;

    QLabel* pCaptionAPI = new QLabel(GPU_STR_projectSettingsAPISelection);
    pCaptionAPI->setStyleSheet(AF_STR_captionLabelStyleSheetMain);


    // Add H layout for the sampling interval widgets:
    QHBoxLayout* pHLayout1 = new QHBoxLayout;
    QHBoxLayout* pHLayout2 = new QHBoxLayout;

    m_pAutomaticCheckBox = new QCheckBox(GPU_STR_projectSettingsAutomaticConnect);

    m_pOptionsComboBox = new QComboBox();
    m_pOptionsComboBox->addItem(GPU_STR_projectSettingsComboProcessNumberOption);
    m_pOptionsComboBox->addItem(GPU_STR_projectSettingsComboAPIInProcessOption);
    m_pOptionsComboBox->addItem(GPU_STR_projectSettingsComboAPIOption);

    m_pOptionsEdit = new QLineEdit();

    pHLayout1->addWidget(m_pAutomaticCheckBox);
    pHLayout1->addWidget(m_pOptionsComboBox);
    pHLayout1->addWidget(m_pOptionsEdit);

    pHLayout1->addStretch();

    m_pPortLineEdit = new QLineEdit();
    QIntValidator* pValidator = new QIntValidator;
    pValidator->setRange(0, GP_MAX_PORT_NUMBER);
    m_pPortLineEdit->setValidator(pValidator);
    int maxWidth = QFontMetrics(m_pPortLineEdit->font()).boundingRect(QString::number(GP_MAX_PORT_NUMBER)).width() + GP_LINE_EDIT_MARGIN;
    m_pPortLineEdit->setMaximumWidth(maxWidth);
    pHLayout2->addWidget(new QLabel(GPU_STR_projectSettingsServerConnectionPort), 0);
    pHLayout2->addWidget(m_pPortLineEdit, 0);
    pHLayout2->addStretch();

    pMainLayout->addWidget(pCaptionAPI);
    pMainLayout->addLayout(pHLayout1);
    pMainLayout->addLayout(pHLayout2);

    pMainLayout->addStretch();

    setLayout(pMainLayout);

    // connecting to the controls events
    bool rc = connect(m_pAutomaticCheckBox, SIGNAL(clicked(bool)), this, SLOT(OnAutomaticClicked(bool)));
    GT_ASSERT(rc);

    rc = connect(m_pOptionsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnConnectionSelected(int)));
    GT_ASSERT(rc);

    rc = connect(m_pOptionsEdit, SIGNAL(textEdited(const QString&)), this, SLOT(OnTextEdited(const QString&)));
    GT_ASSERT(rc);
}

gtString gpProjectSettingsExtension::ExtensionXMLString()
{
    gtString retVal = GPU_STR_projectSettingExtensionName;
    return retVal;
}

gtString gpProjectSettingsExtension::ExtensionTreePathAsString()
{
    gtString retVal = GPU_STR_projectSettingExtensionDisplayName;
    return retVal;
}

bool gpProjectSettingsExtension::GetXMLSettingsString(gtString& projectAsXMLString)
{
    bool retVal = false;

    projectAsXMLString.appendFormattedString(L"<%ls>", ExtensionXMLString().asCharArray());

    gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
    GT_IF_WITH_ASSERT(pFrameAnalysisManager != nullptr)
    {
        gpProjectSettings& settings = pFrameAnalysisManager->ProjectSettings();
        // Add the flag as a bool flag, the combo selection and the string text to the xml
        afUtils::addFieldToXML(projectAsXMLString, GPU_STR_projectSettingsAutomaticXMLField, settings.m_shouldConnectAutomatically);
        afUtils::addFieldToXML(projectAsXMLString, GPU_STR_projectSettingsConnectionXMLField, settings.m_connection);
        afUtils::addFieldToXML(projectAsXMLString, GPU_STR_projectSettingsPortNumberXMLField, settings.m_serverConnectionPort);
        afUtils::addFieldToXML(projectAsXMLString, GPU_STR_projectSettingsProcessNumberXMLField, acQStringToGTString(settings.m_processNumber));
        afUtils::addFieldToXML(projectAsXMLString, GPU_STR_projectSettingsProcessNameXMLField, acQStringToGTString(settings.m_processName));
    }
    projectAsXMLString.appendFormattedString(L"</%ls>", ExtensionXMLString().asCharArray());

    retVal = true;

    return retVal;
}

bool gpProjectSettingsExtension::SetSettingsFromXMLString(const gtString& projectAsXMLString)
{
    bool retVal = false;

    gtString enabledCountersStr, samplingIntervalStr;

    TiXmlNode* pTPNode = new TiXmlElement(ExtensionXMLString().asASCIICharArray());
    QString projectAsQtXML = acGTStringToQString(projectAsXMLString);
    QByteArray projectAsQtXMLAsUTF8 = projectAsQtXML.toUtf8();

    pTPNode->Parse(projectAsQtXMLAsUTF8.data(), 0, TIXML_DEFAULT_ENCODING);
    gtString tpNodeTitle;
    tpNodeTitle.fromASCIIString(pTPNode->Value());

    if (ExtensionXMLString() == tpNodeTitle.asCharArray())
    {
        gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
        GT_IF_WITH_ASSERT(pFrameAnalysisManager != nullptr)
        {
            gpProjectSettings& settings = pFrameAnalysisManager->ProjectSettings();
            int connectionType = 0, portNumber = GP_DEFAULT_PORT;
            gtString connectionNumber;
            gtString connectionName;

            afUtils::getFieldFromXML(*pTPNode, GPU_STR_projectSettingsAutomaticXMLField, settings.m_shouldConnectAutomatically);
            afUtils::getFieldFromXML(*pTPNode, GPU_STR_projectSettingsConnectionXMLField, connectionType);
            afUtils::getFieldFromXML(*pTPNode, GPU_STR_projectSettingsPortNumberXMLField, portNumber);
            settings.m_connection = (gpProjectSettings::eConnectionType)connectionType;
            afUtils::getFieldFromXML(*pTPNode, GPU_STR_projectSettingsProcessNumberXMLField, connectionNumber);
            afUtils::getFieldFromXML(*pTPNode, GPU_STR_projectSettingsProcessNameXMLField, connectionName);

            settings.m_processNumber = acGTStringToQString(connectionNumber);
            settings.m_processName = acGTStringToQString(connectionName);

            settings.m_serverConnectionPort = portNumber;

            retVal = true;
        }
    }

    // Load settings to the controls:
    retVal = RestoreCurrentSettings() && retVal;

    return retVal;
}

bool gpProjectSettingsExtension::SaveCurrentSettings()
{
    bool retVal = true;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pAutomaticCheckBox != nullptr && m_pOptionsComboBox != nullptr && m_pOptionsEdit != nullptr && m_pPortLineEdit != nullptr)
    {
        gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
        GT_IF_WITH_ASSERT(pFrameAnalysisManager != nullptr)
        {
            if (m_bUpdateSettings)
            {
                gpProjectSettings& settings = pFrameAnalysisManager->ProjectSettings();
                settings.m_shouldConnectAutomatically = m_pAutomaticCheckBox->isChecked() ? 1 : 0;
                settings.m_connection = (gpProjectSettings::eConnectionType)m_pOptionsComboBox->currentIndex();
                bool rc = false;
                settings.m_serverConnectionPort = m_pPortLineEdit->text().toInt(&rc);
                GT_ASSERT(rc);
                settings.m_processName = m_processName;
                settings.m_processNumber = m_processNumber;

                m_bUpdateSettings = false;
            }

            retVal = true;
        }
    }

    return retVal;
}

void gpProjectSettingsExtension::RestoreDefaultProjectSettings()
{
    GT_IF_WITH_ASSERT(m_pAutomaticCheckBox != nullptr && m_pOptionsComboBox != nullptr && m_pOptionsEdit != nullptr)
    {
        m_pAutomaticCheckBox->setChecked(true);
        m_pOptionsComboBox->setCurrentIndex(2);
        m_pOptionsEdit->setVisible(false);
        m_pPortLineEdit->setText(QString::number(GP_DEFAULT_PORT));
        m_processName = "";
        m_processNumber = "1";
        // Set the value in the line edit:
        m_bUpdateSettings = true;
    }
}

bool gpProjectSettingsExtension::AreSettingsValid(gtString& invalidMessageStr)
{
    GT_UNREFERENCED_PARAMETER(invalidMessageStr);

    // No project setting page at this stage for this extension
    bool retVal = true;

    return retVal;
}

bool gpProjectSettingsExtension::RestoreCurrentSettings()
{
    bool retVal = true;

    GT_IF_WITH_ASSERT(m_pAutomaticCheckBox != nullptr && m_pOptionsComboBox != nullptr && m_pOptionsEdit != nullptr)
    {
        gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
        GT_IF_WITH_ASSERT(pFrameAnalysisManager != nullptr)
        {
            gpProjectSettings& settings = pFrameAnalysisManager->ProjectSettings();
            m_pAutomaticCheckBox->setChecked(settings.m_shouldConnectAutomatically != 0);
            m_pOptionsComboBox->setCurrentIndex(settings.m_connection);
            m_pOptionsEdit->setVisible(true);
            m_processName = settings.m_processName;
            m_processNumber = settings.m_processNumber;

            switch (settings.m_connection)
            {
                case gpProjectSettings::egpProcessConnection:
                    m_pOptionsEdit->setText(settings.m_processNumber);
                    break;

                case gpProjectSettings::egpFirstApiInProcessConnection:
                    m_pOptionsEdit->setText(settings.m_processName);
                    break;

                case gpProjectSettings::egpFirstApiConnection:
                    m_pOptionsEdit->setVisible(false);
                    m_pOptionsEdit->setText("");
                    break;

                default:
                    GT_ASSERT(false);
                    break;
            }

            m_pPortLineEdit->setText(QString::number(settings.m_serverConnectionPort));
        }
    }
    return retVal;
}

void gpProjectSettingsExtension::OnAutomaticClicked(bool)
{
    m_bUpdateSettings = true;
}

void gpProjectSettingsExtension::OnConnectionSelected(int index)
{
    m_bUpdateSettings = true;
    m_pOptionsEdit->setHidden((gpProjectSettings::eConnectionType)index == gpProjectSettings::egpFirstApiConnection);

    if ((gpProjectSettings::eConnectionType)index == gpProjectSettings::egpProcessConnection)
    {
        m_pOptionsEdit->setValidator(&m_validator);
        m_pOptionsEdit->setText(m_processNumber);
    }
    else if ((gpProjectSettings::eConnectionType)index == gpProjectSettings::egpFirstApiInProcessConnection)
    {
        m_pOptionsEdit->setValidator(nullptr);
        m_pOptionsEdit->setText(m_processName);
    }
}

void gpProjectSettingsExtension::OnTextEdited(const QString& text)
{
    m_bUpdateSettings = true;
    GT_IF_WITH_ASSERT(m_pAutomaticCheckBox != nullptr && m_pOptionsComboBox != nullptr && m_pOptionsEdit != nullptr)
    {
        if (m_pOptionsComboBox->currentIndex() == gpProjectSettings::egpProcessConnection)
        {
            m_processNumber = text;
        }
        else if (m_pOptionsComboBox->currentIndex() == gpProjectSettings::egpFirstApiInProcessConnection)
        {
            m_processName = text;
        }
    }
}
