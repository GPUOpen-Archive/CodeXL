//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppProjectSettingsExtension.cpp
///
//==================================================================================

//------------------------------ ppProjectSettingsExtension.h ------------------------------

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
#include <AMDTApplicationFramework/src/afUtils.h>

// Local:
#include <AMDTPowerProfiling/Include/ppStringConstants.h>
#include <AMDTPowerProfiling/src/ppAppController.h>
#include <AMDTPowerProfiling/src/ppProjectSettingsExtension.h>
#include <AMDTPowerProfiling/src/ppAidFunctions.h>

// ---------------------------------------------------------------------------
// Name:        ppProjectSettingsExtension::ppProjectSettingsExtension
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        25/8/2014
// ---------------------------------------------------------------------------
ppProjectSettingsExtension::ppProjectSettingsExtension() : m_pSamplingIntervalSpinBox(nullptr)
{
    ppAppController::instance().ClearAfterLoadFlag();
}

// ---------------------------------------------------------------------------
// Name:        ppProjectSettingsExtension::~gdProjectSettingsExtension
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        25/8/2014
// ---------------------------------------------------------------------------
ppProjectSettingsExtension::~ppProjectSettingsExtension()
{
}

// ---------------------------------------------------------------------------
// Name:        ppProjectSettingsExtension::initialize
// Description: Create the widget that is reading the debug setting for the debugger
// Author:      Gilad Yarnitzky
// Date:        25/8/2014
// ---------------------------------------------------------------------------
void ppProjectSettingsExtension::Initialize()
{
    QVBoxLayout* pMainLayout = new QVBoxLayout;

    QLabel* pCaption1 = new QLabel(PP_STR_projectSettingsSamplingInterval);
    pCaption1->setStyleSheet(AF_STR_captionLabelStyleSheetMain);


    // Add H layout for the sampling interval widgets:
    QHBoxLayout* pHLayout = new QHBoxLayout;
    QLabel* pLabel = new QLabel(PP_STR_projectSettingsSampleEvery);
    m_pSamplingIntervalSpinBox = new QSpinBox;
    m_pSamplingIntervalSpinBox->setMinimum(PP_MIN_SAMPLING_INTERVAL);
    m_pSamplingIntervalSpinBox->setMaximum(PP_MAX_SAMPLING_INTERVAL);
    m_pSamplingIntervalSpinBox->setSingleStep(PP_DEFAULT_SAMPLING_INTERVAL);

    pHLayout->addWidget(pLabel);
    pHLayout->addWidget(m_pSamplingIntervalSpinBox);
    pHLayout->addStretch();

    pMainLayout->addWidget(pCaption1);
    pMainLayout->addLayout(pHLayout);
    pMainLayout->addStretch();

    setLayout(pMainLayout);
}

// ---------------------------------------------------------------------------
// Name:        ppProjectSettingsExtension::ExtensionXMLString
// Description: Return the extension string
// Return Val:  gtString&
// Author:      Gilad Yarnitzky
// Date:        25/8/2014
// ---------------------------------------------------------------------------
gtString ppProjectSettingsExtension::ExtensionXMLString()
{
    gtString retVal = PP_STR_projectSettingExtensionName;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        ppProjectSettingsExtension::extensionDisplayName
// Description: Return the display name for the extension
// Return Val:  gtString
// Author:      Gilad Yarnitzky
// Date:        25/8/2014
// ---------------------------------------------------------------------------
gtString ppProjectSettingsExtension::ExtensionTreePathAsString()
{
    gtString retVal = PP_STR_projectSettingExtensionDisplayName;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        ppProjectSettingsExtension::getXMLSettingsString
// Description: Get the current debugger project settings as XML string
// Arguments:   gtASCIIString& projectAsXMLString
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        25/8/2014
// ---------------------------------------------------------------------------
bool ppProjectSettingsExtension::GetXMLSettingsString(gtString& projectAsXMLString)
{
    bool retVal = false;
    ppAppController& appController = ppAppController::instance();

    projectAsXMLString.appendFormattedString(L"<%ls>", ExtensionXMLString().asCharArray());

    gtVector<int> enabledCounters;
    appController.GetCurrentProjectEnabledCounters(enabledCounters);
    gtString enabledCountersStr;

    for (int counter : enabledCounters)
    {
        enabledCountersStr.appendUnsignedIntNumber(counter);
        enabledCountersStr.append(L",");
    }

    afUtils::addFieldToXML(projectAsXMLString, PP_STR_projectSettingEnabledCounters, enabledCountersStr);

    unsigned int samplingInterval = appController.GetCurrentProjectSamplingInterval();
    gtString samplingIntervalStr;
    samplingIntervalStr.appendUnsignedIntNumber(samplingInterval);
    afUtils::addFieldToXML(projectAsXMLString, PP_STR_projectSettingSamplingInterval, samplingIntervalStr);

    projectAsXMLString.appendFormattedString(L"</%ls>", ExtensionXMLString().asCharArray());

    retVal = true;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        ppProjectSettingsExtension::setSettingsFromXMLString
// Description: Get the project settings from the XML string
// Arguments:   const gtASCIIString& projectAsXMLString
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        25/8/2014
// ---------------------------------------------------------------------------
bool ppProjectSettingsExtension::SetSettingsFromXMLString(const gtString& projectAsXMLString)
{
    bool retVal = false;

    gtString enabledCountersStr, samplingIntervalStr;

    TiXmlNode* pPPNode = new TiXmlElement(ExtensionXMLString().asASCIICharArray());
    QString projectAsQtXML = acGTStringToQString(projectAsXMLString);
    QByteArray projectAsQtXMLAsUTF8 = projectAsQtXML.toUtf8();

    pPPNode->Parse(projectAsQtXMLAsUTF8.data(), 0, TIXML_DEFAULT_ENCODING);
    gtString ppNodeTitle;
    ppNodeTitle.fromASCIIString(pPPNode->Value());

    if (ExtensionXMLString() == ppNodeTitle.asCharArray())
    {
        afUtils::getFieldFromXML(*pPPNode, PP_STR_projectSettingEnabledCounters, enabledCountersStr);
        afUtils::getFieldFromXML(*pPPNode, PP_STR_projectSettingSamplingInterval, samplingIntervalStr);
    }

    gtVector<int> enabledCounters;

    int startPos = 0;
    int endPos = enabledCountersStr.findFirstOf(L",", startPos);

    // assume that enabledCountersStr is of the following: number followed by delimiter and so on
    while (endPos > startPos)
    {
        gtString subStr;
        enabledCountersStr.getSubString(startPos, endPos - 1, subStr);

        if (subStr.isIntegerNumber())
        {
            int counter;
            subStr.toIntNumber(counter);
            enabledCounters.push_back(counter);
        }

        startPos = endPos + 1;
        endPos = enabledCountersStr.findFirstOf(L",", startPos);
    }

    if (enabledCounters.size())
    {
        // update enabled counters only if we have setting from previous session
        ppAppController::instance().SetCurrentProjectEnabledCounters(enabledCounters);
    }

    retVal = true;
    ppAppController::instance().SetAfterLoadFlag();

    if (!samplingIntervalStr.isEmpty())
    {
        unsigned int samplingInteral = PP_DEFAULT_SAMPLING_INTERVAL;
        retVal = samplingIntervalStr.toUnsignedIntNumber(samplingInteral);
        GT_IF_WITH_ASSERT(retVal)
        {
            ppAppController::instance().SetCurrentProjectSamplingInterval(samplingInteral);
        }
    }

    // Load settings to the controls:
    retVal = RestoreCurrentSettings() && retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        ppProjectSettingsExtension::SaveCurrentSettings
// Description: Get the current project settings from the controls, and store into
//              the current project properties
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        25/8/2014
// ---------------------------------------------------------------------------
bool ppProjectSettingsExtension::SaveCurrentSettings()
{
    bool retVal = true;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSamplingIntervalSpinBox != nullptr)
    {
        // Set the user sampling interval:
        unsigned int userSamplingInterval = m_pSamplingIntervalSpinBox->text().toUInt(&retVal);
        GT_IF_WITH_ASSERT(retVal)
        {
            ppAppController::instance().SetCurrentProjectSamplingInterval(userSamplingInterval);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        ppProjectSettingsExtension::RestoreDefaultProjectSettings
// Description: Restore default project settings
// Author:      Gilad Yarnitzky
// Date:        25/8/2014
// ---------------------------------------------------------------------------
void ppProjectSettingsExtension::RestoreDefaultProjectSettings()
{
    GT_IF_WITH_ASSERT(m_pSamplingIntervalSpinBox != nullptr)
    {
        // Get the current project sampling interval:
        unsigned int samplingInterval = PP_DEFAULT_SAMPLING_INTERVAL;

        // Set the value in the spin box:
        m_pSamplingIntervalSpinBox->setValue(samplingInterval);
    }
}

// ---------------------------------------------------------------------------
// Name:        ppProjectSettingsExtension::AreSettingsValid
// Description: Check if the current settings are valid
// Arguments:   gtString& invalidMessageStr
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        25/8/2014
// ---------------------------------------------------------------------------
bool ppProjectSettingsExtension::AreSettingsValid(gtString& invalidMessageStr)
{
    GT_UNREFERENCED_PARAMETER(invalidMessageStr);

    // No project setting page at this stage for this extension
    bool retVal = true;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        ppProjectSettingsExtension::RestoreCurrentSettings
// Description: Load the current settings to the displayed widgets
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        25/8/2014
// ---------------------------------------------------------------------------
bool ppProjectSettingsExtension::RestoreCurrentSettings()
{
    bool retVal = true;

    GT_IF_WITH_ASSERT(m_pSamplingIntervalSpinBox != nullptr)
    {
        // Get the current project sampling interval:
        unsigned int samplingInterval = ppAppController::instance().GetCurrentProjectSamplingInterval();

        // Set the value in the spin box:
        m_pSamplingIntervalSpinBox->setValue(samplingInterval);
    }
    return retVal;
}