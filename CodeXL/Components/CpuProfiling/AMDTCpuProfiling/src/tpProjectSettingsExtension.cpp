//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpProjectSettingsExtension.cpp
///
//==================================================================================

//------------------------------ tpProjectSettingsExtension.h ------------------------------

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
#include <inc/StringConstants.h>
#include <inc/tpProjectSettingsExtension.h>


tpProjectSettingsExtension::tpProjectSettingsExtension() : m_pDummyLineEdit(nullptr)
{
}

tpProjectSettingsExtension::~tpProjectSettingsExtension()
{
}

void tpProjectSettingsExtension::Initialize()
{
    QVBoxLayout* pMainLayout = new QVBoxLayout;

    QLabel* pCaption1 = new QLabel(CP_STR_projectSettingsDummyValue);
    pCaption1->setStyleSheet(AF_STR_captionLabelStyleSheetMain);


    // Add H layout for the sampling interval widgets:
    QHBoxLayout* pHLayout = new QHBoxLayout;
    QLabel* pLabel = new QLabel(CP_STR_projectSettingsDummyValue);
    m_pDummyLineEdit = new QLineEdit;
    m_pDummyLineEdit->setText(CP_STR_projectSettingsDummyValueDefault);

    pHLayout->addWidget(pLabel);
    pHLayout->addWidget(m_pDummyLineEdit);
    pHLayout->addStretch();

    pMainLayout->addWidget(pCaption1);
    pMainLayout->addLayout(pHLayout);
    pMainLayout->addStretch();

    setLayout(pMainLayout);
}

gtString tpProjectSettingsExtension::ExtensionXMLString()
{
    gtString retVal = CP_STR_projectSettingExtensionName;
    return retVal;
}

gtString tpProjectSettingsExtension::ExtensionTreePathAsString()
{
    gtString retVal = CP_STR_projectSettingExtensionDisplayName;
    return retVal;
}

bool tpProjectSettingsExtension::GetXMLSettingsString(gtString& projectAsXMLString)
{
    bool retVal = false;

    projectAsXMLString.appendFormattedString(L"<%ls>", ExtensionXMLString().asCharArray());

    afUtils::addFieldToXML(projectAsXMLString, CP_STR_projectSettingsDummyValueXML, acQStringToGTString(m_settings.m_dummyString));

    projectAsXMLString.appendFormattedString(L"</%ls>", ExtensionXMLString().asCharArray());

    retVal = true;

    return retVal;
}

bool tpProjectSettingsExtension::SetSettingsFromXMLString(const gtString& projectAsXMLString)
{
    bool retVal = true;

    gtString enabledCountersStr, samplingIntervalStr;

    TiXmlNode* pTPNode = new TiXmlElement(ExtensionXMLString().asASCIICharArray());
    QString projectAsQtXML = acGTStringToQString(projectAsXMLString);
    QByteArray projectAsQtXMLAsUTF8 = projectAsQtXML.toUtf8();

    pTPNode->Parse(projectAsQtXMLAsUTF8.data(), 0, TIXML_DEFAULT_ENCODING);
    gtString tpNodeTitle;
    tpNodeTitle.fromASCIIString(pTPNode->Value());

    if (ExtensionXMLString() == tpNodeTitle.asCharArray())
    {
        gtString valueGT;
        afUtils::getFieldFromXML(*pTPNode, CP_STR_projectSettingsDummyValueXML, valueGT);
        m_settings.m_dummyString = acGTStringToQString(valueGT);
    }

    // Load settings to the controls:
    retVal = RestoreCurrentSettings() && retVal;

    return retVal;
}

bool tpProjectSettingsExtension::SaveCurrentSettings()
{
    bool retVal = true;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pDummyLineEdit != nullptr)
    {
        // Set the user dummy string:
        m_settings.m_dummyString = m_pDummyLineEdit->text();
    }

    return retVal;
}

void tpProjectSettingsExtension::RestoreDefaultProjectSettings()
{
    GT_IF_WITH_ASSERT(m_pDummyLineEdit != nullptr)
    {
        // Set the value in the line edit:
        m_pDummyLineEdit->setText(CP_STR_projectSettingsDummyValueDefault);
    }
}

bool tpProjectSettingsExtension::AreSettingsValid(gtString& invalidMessageStr)
{
    GT_UNREFERENCED_PARAMETER(invalidMessageStr);

    // No project setting page at this stage for this extension
    bool retVal = true;

    return retVal;
}

bool tpProjectSettingsExtension::RestoreCurrentSettings()
{
    bool retVal = true;

    GT_IF_WITH_ASSERT(m_pDummyLineEdit != nullptr)
    {
        // Set the value in the line edit:
        m_pDummyLineEdit->setText(m_settings.m_dummyString);
    }
    return retVal;
}