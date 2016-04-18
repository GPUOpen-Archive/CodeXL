//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/GeneralSettingWindow.cpp $
/// \version $Revision: #10 $
/// \brief  This file contains general setting options
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/GeneralSettingWindow.cpp#10 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

#ifdef _WIN32
    #pragma warning(push, 1)
#endif
#include <QtCore>
#include <QtWidgets>
#ifdef _WIN32
    #pragma warning(pop)
#endif

#include "GeneralSettingWindow.h"
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include "GlobalSettings.h"

//infra
#include <AMDTBaseTools/Include/gtAssert.h>


GeneralSettingComponents::GeneralSettingComponents() :
    m_alwaysDeleteSessionFile(false),
    m_neverDeleteSessionFile(true),
    m_askUserBeforeDelete(false),
    m_showDetailsOfDeletion(false)
{

}

GeneralSettingWindow::GeneralSettingWindow(QWidget* /*parent*/): afGlobalSettingsPage(),
    m_constStrAlwaysDeleteSessionFile("AlwaysDeleteSessionFiles"),
    m_constStrNeverDeleteSessionFile("NeverDeleteSessionFiles"),
    m_constStrAskUserBeforeDelete("AskUserBeforeDelete"),
    m_constStrShowDetailsOfDeletion("ShowDetailsOfDeletion"),
    m_pAlwaysDeleteSessionFileRB(NULL),
    m_pNeverDeleteSessionFileRB(NULL),
    m_pAskUserBeforeDeleteRB(NULL),
    m_pShowDetailsOfDeletionCB(NULL)
{

}

GeneralSettingWindow::~GeneralSettingWindow()
{
    SAFE_DELETE(m_pAlwaysDeleteSessionFileRB);
    SAFE_DELETE(m_pNeverDeleteSessionFileRB);
    SAFE_DELETE(m_pAskUserBeforeDeleteRB);
    SAFE_DELETE(m_pShowDetailsOfDeletionCB);
}

void GeneralSettingWindow::initialize()
{
    m_pAlwaysDeleteSessionFileRB = new QRadioButton("Always delete session files");


    m_pNeverDeleteSessionFileRB = new QRadioButton("Never delete session files");


    m_pAskUserBeforeDeleteRB = new QRadioButton("Ask user every time");


    m_pShowDetailsOfDeletionCB = new QCheckBox("Show details of deletion");

    m_pShowDetailsOfDeletionCB->setDisabled(true);

    QVBoxLayout* deleteSessionFileLayout = new QVBoxLayout();

    deleteSessionFileLayout->addWidget(m_pAlwaysDeleteSessionFileRB);
    deleteSessionFileLayout->addWidget(m_pNeverDeleteSessionFileRB);
    deleteSessionFileLayout->addWidget(m_pAskUserBeforeDeleteRB);
    deleteSessionFileLayout->addWidget(m_pShowDetailsOfDeletionCB);

    QGroupBox* deleteSessionFileGB = new QGroupBox(tr("Delete session files when a project is closed"));

    deleteSessionFileGB->setLayout(deleteSessionFileLayout);

    QVBoxLayout* mainLayout = new QVBoxLayout();

    mainLayout->addWidget(deleteSessionFileGB);
    mainLayout->addStretch();

    setLayout(mainLayout);

    connect(m_pAlwaysDeleteSessionFileRB, SIGNAL(toggled(bool)), this, SLOT(RadioButtonToggled(bool)));
    connect(m_pNeverDeleteSessionFileRB, SIGNAL(toggled(bool)), this, SLOT(RadioButtonToggled(bool)));
    connect(m_pAskUserBeforeDeleteRB, SIGNAL(toggled(bool)), this, SLOT(RadioButtonToggled(bool)));
}

gtString GeneralSettingWindow::pageTitle()
{
    return GPU_STR_GENERAL_SETTINGS_DISPLAY;
}

gtString GeneralSettingWindow::xmlSectionTitle()
{
    return GPU_STR_GENERAL_SETTINGS;
}

bool GeneralSettingWindow::getXMLSettingsString(gtASCIIString& projectAsXMLString)
{
    bool retVal = false;
    gtString pageName = GPU_STR_GENERAL_SETTINGS;

    retVal = getProjectSettingsXML(projectAsXMLString, pageName);
    return retVal;
}

bool GeneralSettingWindow::setSettingsFromXMLString(const gtASCIIString& projectAsXMLString)
{
    return setProjectSettingsXML(projectAsXMLString);
}

void GeneralSettingWindow::restoreDefaultSettings()
{
    GeneralSettingComponents restore;
    m_currentSettings = restore;
    restoreCurrentSettings();
}

void GeneralSettingWindow::loadCurrentSettings()
{
    restoreCurrentSettings();
}

bool GeneralSettingWindow::saveCurrentSettings()
{
    m_currentSettings.m_alwaysDeleteSessionFile = m_pAlwaysDeleteSessionFileRB->isChecked();
    m_currentSettings.m_neverDeleteSessionFile = m_pNeverDeleteSessionFileRB->isChecked();
    m_currentSettings.m_askUserBeforeDelete = m_pAskUserBeforeDeleteRB->isChecked();
    m_currentSettings.m_showDetailsOfDeletion = m_pShowDetailsOfDeletionCB->isChecked();

    UpdateGlobalSettings();

    return true;
}

void GeneralSettingWindow::UpdateGlobalSettings()
{
    QMap<QString, bool> generalOptionSetting;
    generalOptionSetting.insert("Always", m_currentSettings.m_alwaysDeleteSessionFile);
    generalOptionSetting.insert("Never", m_currentSettings.m_neverDeleteSessionFile);
    generalOptionSetting.insert("ASK", m_currentSettings.m_askUserBeforeDelete);
    generalOptionSetting.insert("ShowDetailDeletion", m_currentSettings.m_showDetailsOfDeletion);

    GlobalSettings::Instance()->m_generalOpt.Load(generalOptionSetting);
}

bool GeneralSettingWindow::restoreCurrentSettings()
{
    m_pAlwaysDeleteSessionFileRB->setChecked(m_currentSettings.m_alwaysDeleteSessionFile);
    m_pNeverDeleteSessionFileRB->setChecked(m_currentSettings.m_neverDeleteSessionFile);
    m_pAskUserBeforeDeleteRB->setChecked(m_currentSettings.m_askUserBeforeDelete);
    m_pShowDetailsOfDeletionCB->setChecked(m_currentSettings.m_showDetailsOfDeletion);

    m_pShowDetailsOfDeletionCB->setEnabled(m_pAskUserBeforeDeleteRB->isChecked());

    UpdateGlobalSettings();

    return true;
}

bool GeneralSettingWindow::setProjectSettingsXML(const gtASCIIString& projectAsXMLString)
{
    QString qtStr(projectAsXMLString.asCharArray());

    QDomDocument doc;
    doc.setContent(qtStr.toLatin1());
    QString nodeVal;
    bool val;
    QDomNodeList list;

    list = doc.elementsByTagName(m_constStrAlwaysDeleteSessionFile.asCharArray());
    nodeVal = list.at(0).toElement().text();
    val = (nodeVal == "T") ? true : false;
    m_currentSettings.m_alwaysDeleteSessionFile = val;

    list = doc.elementsByTagName(m_constStrNeverDeleteSessionFile.asCharArray());
    nodeVal = list.at(0).toElement().text();
    val = (nodeVal == "T") ? true : false;
    m_currentSettings.m_neverDeleteSessionFile = val;

    list = doc.elementsByTagName(m_constStrAskUserBeforeDelete.asCharArray());
    nodeVal = list.at(0).toElement().text();
    val = (nodeVal == "T") ? true : false;
    m_currentSettings.m_askUserBeforeDelete = val;

    list = doc.elementsByTagName(m_constStrShowDetailsOfDeletion.asCharArray());
    nodeVal = list.at(0).toElement().text();
    val = (nodeVal == "T") ? true : false;
    m_currentSettings.m_showDetailsOfDeletion = val;

    restoreCurrentSettings();
    return true;
}

bool GeneralSettingWindow::getProjectSettingsXML(gtASCIIString& projectAsXMLString, gtString& projectPage)
{
    gtASCIIString numVal;
    projectAsXMLString.append("<");
    projectAsXMLString.append(projectPage.asASCIICharArray());
    projectAsXMLString.append(">");

    writeSession(projectAsXMLString, "Current");

    projectAsXMLString.append("</");
    projectAsXMLString.append(projectPage.asASCIICharArray());
    projectAsXMLString.append(">");

    return true;
}

void GeneralSettingWindow::writeSession(gtASCIIString& projectAsXMLString, const gtASCIIString type)
{
    gtASCIIString numVal;

    projectAsXMLString.append("<Session type=\"");
    projectAsXMLString.append(type);
    projectAsXMLString.append("\">");

    writeBool(projectAsXMLString, m_constStrAlwaysDeleteSessionFile, m_currentSettings.m_alwaysDeleteSessionFile);
    writeBool(projectAsXMLString, m_constStrNeverDeleteSessionFile, m_currentSettings.m_neverDeleteSessionFile);
    writeBool(projectAsXMLString, m_constStrAskUserBeforeDelete, m_currentSettings.m_askUserBeforeDelete);
    writeBool(projectAsXMLString, m_constStrShowDetailsOfDeletion, m_currentSettings.m_showDetailsOfDeletion);

    projectAsXMLString.append("</Session>");
}

void GeneralSettingWindow::writeValue(gtASCIIString& projectAsXMLString,
                                      const gtASCIIString& key, const gtASCIIString& value)
{
    projectAsXMLString.append("<");
    projectAsXMLString.append(key);
    projectAsXMLString.append(">");
    projectAsXMLString.append(value);
    projectAsXMLString.append("</");
    projectAsXMLString.append(key);
    projectAsXMLString.append(">");
}

void GeneralSettingWindow::writeBool(gtASCIIString& projectAsXMLString,
                                     const gtASCIIString& key, const bool value)
{
    gtASCIIString val;
    val = value ? "T" : "F";
    writeValue(projectAsXMLString, key, val);
}

void GeneralSettingWindow::RadioButtonToggled(bool /*isChecked*/)
{
    m_pShowDetailsOfDeletionCB->setChecked(m_pAskUserBeforeDeleteRB->isChecked());
    m_pShowDetailsOfDeletionCB->setEnabled(m_pAskUserBeforeDeleteRB->isChecked());
}

