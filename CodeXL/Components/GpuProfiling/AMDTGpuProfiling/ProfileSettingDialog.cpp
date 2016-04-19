//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProfileSettingDialog.cpp $
/// \version $Revision: #14 $
/// \brief :  This file contains ProfileSettingDialog class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProfileSettingDialog.cpp#14 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================
#ifdef _WIN32
    #pragma warning(push, 1)
#endif
#include <QtCore>
#include <QtWidgets>
#include <QtXml>
#include <AMDTApplicationComponents/Include/acFunctions.h>

#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>

#ifdef _WIN32
    #pragma warning(pop)
#endif

#include <AMDTOSWrappers/Include/osFilePath.h>

#include "ProfileSettingDialog.h"
#include "EditNameValueDialog.h"


const QString ProfileSettingDialog::ProfileSettingDataFileExtn(".profsetdata");
const int ProfileSettingDialog::CacheSize(5);
const QString ProfileSettingDialog::NAMESTR("name");
const QString ProfileSettingDialog::VARSTR("Variable");
const QString ProfileSettingDialog::ENVVARSSTR("EnvironmentVariables");
const QString ProfileSettingDialog::USECUSTOMSETSTR("UseCustomSetting");
const QString ProfileSettingDialog::MERGEENVSTR("MergeEnvironment");
const QString ProfileSettingDialog::CMDLINEARGSTR("CmdLineArg");
const QString ProfileSettingDialog::ACTPROJDIRSTR("ActiveProjectDirectory");
const QString ProfileSettingDialog::PLATFORMCONGISTR("PlatformConfig");
const QString ProfileSettingDialog::APPPATHSTR("ApplicationPath");
const QString ProfileSettingDialog::PROJFULLNAMESTR("ProjectFullName");
const QString ProfileSettingDialog::ORIGINALSETTINGFULLSTR("Original settings from Visual Studio environment");
const QString ProfileSettingDialog::SAMEASORIGINALSTR(" (same as settings from Visual Studio)");
const QString ProfileSettingDialog::CUSTOMSETTINGFULLSTR("Custom settings");
const QString ProfileSettingDialog::CUSTOMSTR("Custom");
const QString ProfileSettingDialog::ORIGINALSTR("Original");

const QString ProfileSettingDialog::CACHE_APP_PATH_STR("CodeXLGPUProfilerSessionParameters::ApplicationPath");
const QString ProfileSettingDialog::CACHE_WORK_DIR_STR("ProfileSettingDialog::ActiveProjectDirectory");
const QString ProfileSettingDialog::CACHE_CMD_OPT_STR("ProfileSettingDialog::CommandLineArguments");

ProfileSettingDialog::ProfileSettingDialog(QWidget* parent):
    m_customDataExist(false)
{
    osFilePath cacheFileName;
    Util::GetProfilerAppDataDir(cacheFileName);
    cacheFileName.setFileName(L"profilesetting.cache");
    gtString cacheFileNameAsStr = cacheFileName.asString();
    m_cacheFileName = acGTStringToQString(cacheFileNameAsStr);

    setupUi(this);
    setParent(parent);
    InitializeFieldsAndControls();
    setFixedSize(563, 464);
    setWindowTitle(QString("%1 Session Parameters").arg(afGlobalVariablesManager::ProductNameA()));
}

void ProfileSettingDialog::InitializeFieldsAndControls()
{
    applicationPathComboBox->setEditable(true);
    workingDirectoryComboBox->setEditable(true);
    cmdLineArgComboBox->setEditable(true);

    m_modelEnvironmentVariables = new QStandardItemModel(0, 2, this);
    QStringList headers;
    headers.append("Variable");
    headers.append("Value");
    m_modelEnvironmentVariables->setHorizontalHeaderLabels(headers);

    connect(applicationPathButton, SIGNAL(clicked()), this, SLOT(onApplicationPathButton_Click()));
    connect(workingDirectoryButton, SIGNAL(clicked()), this, SLOT(onWorkingDirectoryButton_Click()));

    connect(okeyButton, SIGNAL(clicked()), this, SLOT(onSaveButton_Click()));
    connect(restoreButton, SIGNAL(clicked()), this, SLOT(onRestoreButton_Click()));
    connect(newButton, SIGNAL(clicked()), this, SLOT(onNewButton_Click()));
    connect(editButton, SIGNAL(clicked()), this, SLOT(onEditButton_Click()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(onDeleteButton_Click()));
    connect(environmentVariableListView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onEnvironmentVariableListView_DoubleClick()));



    environmentVariableListView->setRootIsDecorated(false);
    environmentVariableListView->setModel(m_modelEnvironmentVariables);
    environmentVariableListView->sortByColumn(0, Qt::AscendingOrder);

    m_pSelectionModel = new QItemSelectionModel(environmentVariableListView->model());
    environmentVariableListView->setSelectionModel(m_pSelectionModel);

    m_pSelectionModel = environmentVariableListView->selectionModel();
    connect(m_pSelectionModel,
            SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this,
            SLOT(onUpdateSelectionChange()));
    editButton->setEnabled(false);
    deleteButton->setEnabled(false);

    connect(applicationPathComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(onApplicationPathComboBox_TextChanged()));
    connect(applicationPathComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onApplicationPathComboBox_TextChanged()));
    connect(workingDirectoryComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(onWorkingDirectoryComboBox_TextChanged()));
    connect(workingDirectoryComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onWorkingDirectoryComboBox_TextChanged()));
    connect(cmdLineArgComboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(onCmdLineArgComboBox_TextChanged()));
    connect(cmdLineArgComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onCmdLineArgComboBox_TextChanged()));
    connect(mergeEnvironmentCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onMergeEnvironmentCheckBox_CheckedChanged()));

    LoadCache();
    restoreButton->setVisible(false);
    ChangeCaptions(true);
}

void ProfileSettingDialog::ChangeCaptions(bool currentIsOriginal)
{
    restoreButton->setText
    (restoreButton->text().replace
     (
         (currentIsOriginal ? ORIGINALSTR : CUSTOMSTR),
         (currentIsOriginal ? CUSTOMSTR : ORIGINALSTR)
     )
    );
    QString customSettingString = CUSTOMSETTINGFULLSTR;

    if (ModifiedIsOriginal())
    {
        customSettingString += SAMEASORIGINALSTR;
    }

    labelProfilesettingStatus->setText(currentIsOriginal ? ORIGINALSETTINGFULLSTR : customSettingString);
}

void ProfileSettingDialog::LoadDialogData(const ProfileSettingData& profileSettingData)
{
    m_projFullName = profileSettingData.ProjectInfo().m_path;
    m_platformConfig = profileSettingData.PlatformConfig();

    UpdateProfSettingDataFile();
    m_originalData.Assign(profileSettingData);

    m_SavedSettingExist = LoadProfilerSettingData();

    if (m_SavedSettingExist && (!m_OriginalSelected))
    {
        m_currentIsOriginal = false;
    }
    else
    {
        m_currentIsOriginal = true;
    }

    LoadFields(m_currentIsOriginal);
}

void ProfileSettingDialog::GetProfileSetting(ProfileSettingData& profileSettingData)
{
    profileSettingData.SetProjectInfoPath(m_projFullName);
    profileSettingData.SetPlatformConfig(m_platformConfig);

    profileSettingData.SetApplicationPath(applicationPathComboBox->currentText());
    profileSettingData.SetWorkingDirectory(workingDirectoryComboBox->currentText());
    profileSettingData.SetCommandlineArguments(cmdLineArgComboBox->currentText());
    profileSettingData.SetMergeEnvironment(mergeEnvironmentCheckBox->isChecked());

    QAbstractItemModel* model = environmentVariableListView->model();

    for (int i = 0 ; i < model->rowCount() ; ++i)
    {
        QString s;
        s = model->index(i, 0).data(Qt::DisplayRole).toString();
        s.append("=");
        s.append(model->index(i, 1).data(Qt::DisplayRole).toString());
        profileSettingData.AddEnvVariable(s);
    }
}


void ProfileSettingDialog::LoadCache()
{
    try
    {
        QFileInfo cacheFile(m_cacheFileName);

        if (!cacheFile.exists())
        {
            return;
        }

        {
            QFile inFile(m_cacheFileName);

            if (!inFile.open(QIODevice::ReadOnly))
            {
                return;
            }

            QTextStream sr(&inFile);

            while (!sr.atEnd())
            {
                QString str = sr.readLine();

                if (str.trimmed().isEmpty())
                {
                    continue;
                }

                int i = str.indexOf('=');

                if (-1 == i)
                {
                    continue;
                }

                QStringList setting;
                setting.append(str.mid(0, i));
                setting.append(str.mid(i + 1));

                if (setting.at(1).trimmed().length() == 0)
                {
                    continue;
                }

                if (setting.at(0) == CACHE_APP_PATH_STR)
                {
                    applicationPathComboBox->addItem(setting.at(1));
                }
                else if (setting.at(0) == CACHE_WORK_DIR_STR)
                {
                    workingDirectoryComboBox->addItem(setting.at(1));
                }
                else if (setting.at(0) == CACHE_CMD_OPT_STR)
                {
                    cmdLineArgComboBox->addItem(setting.at(1));
                }
            }
        }
    }
    catch (...)
    {
    }
}


void ProfileSettingDialog::SaveCache()
{
    try
    {
        {
            QFile outFile(m_cacheFileName);
            outFile.open(QIODevice::WriteOnly | QIODevice::Text);
            QTextStream sw(&outFile);

            QStringList sl;

            // applicationPath
            sl.clear();
            QString s = applicationPathComboBox->currentText();

            for (int i = 0; i <= applicationPathComboBox->count() - 1; ++i)
            {
                sl.append(applicationPathComboBox->itemText(i));
            }

            if (sl.contains(s.trimmed()))
            {
                sl.removeOne(s);
            }
            else if (sl.count() == CacheSize)
            {
                sl.removeAt(sl.count() - 1);
            }

            sl.insert(0, s);

            for (int i = 0; i <= sl.count() - 1; ++i)
            {
                sw << QString("%1=%2").arg(CACHE_APP_PATH_STR).arg(sl[i]) << QString("\n");
            }

            // workingDirectory
            sl.clear();
            s = workingDirectoryComboBox->currentText();

            for (int i = 0; i <= workingDirectoryComboBox->count() - 1; ++i)
            {
                sl.append(workingDirectoryComboBox->itemText(i));
            }

            if (sl.contains(s))
            {
                sl.removeOne(s);
            }
            else if (sl.count() == CacheSize)
            {
                sl.removeAt(sl.count() - 1);
            }

            sl.insert(0, s);

            for (int i = 0; i <= sl.count() - 1; ++i)
            {
                sw << QString("%1=%2").arg(CACHE_WORK_DIR_STR).arg(sl[i]) << QString("\n");
            }

            // cmdLineArgComboBox
            sl.clear();
            s = cmdLineArgComboBox->currentText();

            for (int i = 0; i <= cmdLineArgComboBox->count() - 1; ++i)
            {
                sl.append(cmdLineArgComboBox->itemText(i));
            }

            if (sl.contains(s))
            {
                sl.removeOne(s);
            }
            else if (sl.count() == CacheSize)
            {
                sl.removeAt(sl.count() - 1);
            }

            sl.insert(0, s);

            for (int i = 0; i <= sl.count() - 1; ++i)
            {
                sw << QString("%1=%2").arg(CACHE_CMD_OPT_STR).arg(sl[i]) << QString("\n");
            }
        }
    }
    catch (...)
    {
    }
}

void ProfileSettingDialog::LoadFields(bool loadOriginal)
{
    ProfileSettingData pd = loadOriginal ? m_originalData : m_modifiedData;

    applicationPathComboBox->setEditText(pd.ApplicationPath());
    workingDirectoryComboBox->setEditText(pd.WorkingDirectory());
    cmdLineArgComboBox->setEditText(pd.CommandlineArguments());
    mergeEnvironmentCheckBox->setChecked(pd.MergeEnvironment());

    QStringList envList = pd.EnvVariableList();

    for (int i = 0; i < envList.count(); ++i)
    {
        QString s = envList.at(i);
        QStringList v = s.split('=');

        if (v.count() != 2)
        {
            continue;
        }

        QStandardItem* itemVariable = new QStandardItem(v.at(0));
        itemVariable->setEditable(false);

        QStandardItem* itemValue = new QStandardItem(v.at(1));
        itemValue->setEditable(false);

        m_modelEnvironmentVariables->setItem(i, 0, itemVariable);
        m_modelEnvironmentVariables->setItem(i, 1, itemValue);
    }

    restoreButton->setVisible(m_customDataExist);
    ChangeCaptions(loadOriginal);
}

void ProfileSettingDialog::UpdateDataChange()
{
    restoreButton->setVisible(true);
    ChangeCaptions(false);
}

void ProfileSettingDialog::onEnvironmentVariableListView_DoubleClick()
{
    ShowEditNameValue(false);
}

void ProfileSettingDialog::SelectFile()
{
    try
    {
        QString defaultPath = acGTStringToQString(afProjectManager::instance().currentProjectFilePath().fileDirectoryAsString());
        QString appFilePath = afApplicationCommands::instance()->ShowFileSelectionDialog("Select Application File", defaultPath, "Executable Files (*.exe);;All files (*.*)", NULL, false);

        if (!appFilePath.isEmpty())
        {
            applicationPathComboBox->setEditText(appFilePath.trimmed());
        }
    }
    catch (...)
    {
    }
}

void ProfileSettingDialog::SelectDirectory()
{
    try
    {
        QString defaultDir = acGTStringToQString(afProjectManager::instance().currentProjectFilePath().fileDirectoryAsString());
        QString dirPath = afApplicationCommands::instance()->ShowFolderSelectionDialog("Select Working Directory", defaultDir);

        if (!dirPath.isEmpty())
        {
            workingDirectoryComboBox->lineEdit()->setText(dirPath);
        }
    }
    catch (...)
    {
    }
}

void ProfileSettingDialog::onApplicationPathButton_Click()
{
    SelectFile();
    UpdateDataChange();
}

void ProfileSettingDialog::onWorkingDirectoryButton_Click()
{
    SelectDirectory();
    UpdateDataChange();
}

void ProfileSettingDialog::onApplicationPathComboBox_TextChanged()
{
    UpdateDataChange();
}

void ProfileSettingDialog::onWorkingDirectoryComboBox_TextChanged()
{
    UpdateDataChange();
}

void ProfileSettingDialog::onCmdLineArgComboBox_TextChanged()
{
    UpdateDataChange();
}

void ProfileSettingDialog::onMergeEnvironmentCheckBox_CheckedChanged()
{
    UpdateDataChange();
}

void ProfileSettingDialog::onRestoreButton_Click()
{
    bool currentIsOriginal = !restoreButton->text().contains(ORIGINALSTR);
    ProfileSettingData loadData = (currentIsOriginal && m_customDataExist) ? m_modifiedData : m_originalData;
    applicationPathComboBox->setEditText(loadData.ApplicationPath());
    workingDirectoryComboBox->setEditText(loadData.WorkingDirectory());
    cmdLineArgComboBox->setEditText(loadData.CommandlineArguments());
    mergeEnvironmentCheckBox->setChecked(loadData.MergeEnvironment());

    int rowCount = m_modelEnvironmentVariables->rowCount();
    m_modelEnvironmentVariables->removeRows(0, rowCount);

    QStringList localEnvList = loadData.EnvVariableList();

    for (int i = 0; i < localEnvList.count(); ++i)
    {
        QString str = localEnvList.at(i);
        QStringList v = str.split('=');

        if (v.length() != 2)
        {
            continue;
        }

        QStandardItem* itemVariable = new QStandardItem(v.at(0));
        itemVariable->setEditable(false);

        QStandardItem* itemValue = new QStandardItem(v.at(1));
        itemValue->setEditable(false);

        m_modelEnvironmentVariables->setItem(i, 0, itemVariable);
        m_modelEnvironmentVariables->setItem(i, 1, itemValue);
    }

    ChangeCaptions(!(currentIsOriginal && m_customDataExist));
    restoreButton->setVisible(m_customDataExist); // if custom data !exist, hide button
}

void ProfileSettingDialog::onSaveButton_Click()
{
    if (InputsAreValid())
    {
        SaveCache();
        SaveProfilerSettingData();
        accept();
    }
    else
    {
        return;
    }
}

void ProfileSettingDialog::UpdateProfSettingDataFile()
{
    m_profileSettingDataFileXML = m_projFullName.trimmed();

    if (!m_profileSettingDataFileXML.trimmed().isEmpty())
    {
        m_profileSettingDataFileXML = m_profileSettingDataFileXML +
                                      ProfileSettingDataFileExtn +
                                      ".xml";
    }
    else
    {
        m_profileSettingDataFileXML.clear();
    }
}

void ProfileSettingDialog::ProfileSetting_Shown()
{
    restoreButton->setVisible(m_customDataExist);
    ChangeCaptions(m_currentIsOriginal);
}

void ProfileSettingDialog::onNewButton_Click()
{
    ShowEditNameValue(true);
}

void ProfileSettingDialog::onEditButton_Click()
{
    ShowEditNameValue(false);
}

void ProfileSettingDialog::ShowEditNameValue(bool openInAddMode)
{
    QString variable;
    QString value;

    if (!openInAddMode)
    {
        int row = environmentVariableListView->currentIndex().row();

        if (-1 == row)
        {
            Util::ShowWarningBox("Select a row to edit");
            return;
        }

        variable = m_modelEnvironmentVariables->item(row, 0)->text();
        value = m_modelEnvironmentVariables->item(row, 1)->text();
    }

    EditNameValueDialog* ed = new EditNameValueDialog(variable, value, openInAddMode);
    int result = ed->exec();

    if (QDialog::Accepted == result)
    {
        int newIndex = -1;
        int existingNameIndex = -1;
        QString name;
        QString value;
        ed->GetNameValue(name, value);

        for (int j = 0; j < m_modelEnvironmentVariables->rowCount(); ++j)
        {
            if (m_modelEnvironmentVariables->item(j, 0)->text() == name)
            {
                existingNameIndex = j;
                break;
            }
        }

        if (-1 != existingNameIndex)
        {
            newIndex = existingNameIndex;

            if (openInAddMode)
            {
                if (m_modelEnvironmentVariables->item(existingNameIndex, 1)->text() == value)
                {
                    QString strMsg = QString("Environment Variable \"%1\" already exists with same value \"%2\".").arg(name).arg(value);
                    Util::ShowWarningBox(strMsg);
                }
                else
                {
                    QString str1 = QString("Environment Variable \"%1\" already exists.").arg(name);
                    QString str2 = QString("Replace the existing value \"%1\" with new value \"%2\"?").arg(m_modelEnvironmentVariables->item(existingNameIndex, 1)->text()).arg(value);
                    QString both = str1 + str2;

                    int retVal = acMessageBox::instance().question(afGlobalVariablesManager::ProductNameA(),
                                                                   both.toLatin1().data(),
                                                                   QMessageBox::Yes | QMessageBox::No,
                                                                   QMessageBox::Yes);

                    if (QMessageBox::Yes == retVal)
                    {
                        m_modelEnvironmentVariables->item(existingNameIndex, 1)->setText(value);
                    }
                }
            }
            else
            {
                m_modelEnvironmentVariables->item(existingNameIndex, 1)->setText(value);
            }
        }
        else if (openInAddMode)
        {
            QStandardItem* itemVariable = new QStandardItem(name);
            itemVariable->setEditable(false);

            QStandardItem* itemValue = new QStandardItem(value);
            itemValue->setEditable(false);
            int newRowIndex = m_modelEnvironmentVariables->rowCount();
            m_modelEnvironmentVariables->setItem(newRowIndex, 0, itemVariable);
            m_modelEnvironmentVariables->setItem(newRowIndex, 1, itemValue);
            newIndex = newRowIndex;
        }

        if (-1 != newIndex)
        {
            UpdateDataChange();
        }
    }
}

void ProfileSettingDialog::onDeleteButton_Click()
{
    int row = environmentVariableListView->currentIndex().row();
    m_modelEnvironmentVariables->removeRow(row);
}

bool ProfileSettingDialog::InputsAreValid()
{
    if (applicationPathComboBox->currentText().trimmed().isEmpty())
    {
        Util::ShowWarningBox("Select the application to profile");
        applicationPathComboBox->setFocus();
        return false;
    }

    QFileInfo f(applicationPathComboBox->currentText().trimmed());

    if (!f.exists())
    {
        Util::ShowWarningBox("Application does not exist. Please enter a valid application name");
        applicationPathComboBox->setFocus();
        return false;
    }

    if (workingDirectoryComboBox->currentText().trimmed().isEmpty())
    {
        QString message = "Active project directory is not set.";
        message += "Do you want to set it to the application's directory?";

        int retVal = acMessageBox::instance().question(afGlobalVariablesManager::ProductNameA(),
                                                       message.toLatin1().data(),
                                                       QMessageBox::Yes | QMessageBox::No,
                                                       QMessageBox::Yes);

        if (QMessageBox::Yes == retVal)
        {
            QFileInfo fi(applicationPathComboBox->currentText().trimmed());
            workingDirectoryComboBox->setEditText(fi.path());
        }

        workingDirectoryComboBox->setFocus();
        return false;
    }

    QFileInfo f1(workingDirectoryComboBox->currentText().trimmed());

    if ((!f1.isDir()) || f1.isSymLink())
    {
        Util::ShowWarningBox("Active project directory does not exist. Please enter a valid directory");
        workingDirectoryComboBox->setFocus();
        return false;
    }

    return true;
}

void ProfileSettingDialog::onUpdateSelectionChange()
{
    int row = environmentVariableListView->currentIndex().row();
    bool buttonState = (-1 != row);
    editButton->setEnabled(buttonState);
    deleteButton->setEnabled(buttonState);
}

bool ProfileSettingDialog::LoadProfilerSettingData()
{
    bool result = false;

    try
    {
        QFileInfo f(m_profileSettingDataFileXML);

        if (!f.exists())
        {
            // Start with original setting
            m_UseCustomSetting = false;
            m_customDataExist = false;
            return false;
        }

        QDomDocument doc;
        QDomElement  root;
        QFile inFile(m_profileSettingDataFileXML);
        doc.setContent(&inFile);
        inFile.close();
        root = doc.documentElement();
        int recordCount = root.elementsByTagName("ProfileSetting").count();

        if (recordCount > 0)
        {
            QDomElement currentProfileSetting;

            for (int i = 0; i < recordCount; ++i)
            {
                QDomNode cNode = root.childNodes().at(i);
                currentProfileSetting = cNode.toElement();
                QString projFullName = currentProfileSetting.attribute(PROJFULLNAMESTR);
                QString stringPlatformConfig = currentProfileSetting.attribute(PLATFORMCONGISTR);

                if ((stringPlatformConfig == m_platformConfig) && (projFullName == m_projFullName))
                {
                    // read all the data here
                    m_modifiedData.SetApplicationPath(currentProfileSetting.elementsByTagName(APPPATHSTR).at(0).toElement().text());
                    m_modifiedData.SetWorkingDirectory(currentProfileSetting.elementsByTagName(ACTPROJDIRSTR).at(0).toElement().text());
                    m_modifiedData.SetCommandlineArguments(currentProfileSetting.elementsByTagName(CMDLINEARGSTR).at(0).toElement().text());
                    m_modifiedData.SetMergeEnvironment(currentProfileSetting.elementsByTagName(MERGEENVSTR).at(0).toElement().text().trimmed() == Util::ms_TRUESTR);
                    m_UseCustomSetting = currentProfileSetting.elementsByTagName(USECUSTOMSETSTR).at(0).toElement().text().trimmed() == Util::ms_TRUESTR;
                    QDomElement env = currentProfileSetting.elementsByTagName(ENVVARSSTR).at(0).toElement();

                    if (env.hasChildNodes())
                    {
                        int envCount = env.elementsByTagName(VARSTR).count();

                        for (int j = 0; j < envCount; ++j)
                        {
                            QDomNode envNode = env.childNodes().at(j);
                            QString v = envNode.toElement().attribute(NAMESTR);
                            v.append("=");
                            v.append(envNode.toElement().text());
                            m_modifiedData.AddEnvVariable(v);
                        }
                    }

                    m_customDataExist = true;
                    result = true;
                    break;
                }
            }
        }
        else
        {
            m_UseCustomSetting = false;
            m_customDataExist = false;
            return false;
        }
    }
    catch (...)
    {
        result = false;
        m_customDataExist = false;
        m_UseCustomSetting = false;
    }

    return result;
}

bool ProfileSettingDialog::SaveProfilerSettingData()
{
    if (!restoreButton->isVisible())
    {
        m_UseCustomSetting = false;
    }
    else if (restoreButton->text().contains(ORIGINALSTR))
    {
        m_UseCustomSetting = true;
    }
    else
    {
        m_UseCustomSetting = false;
    }

    m_OriginalSelected = !m_UseCustomSetting;

    if (ModifiedIsCurrent())
    {
        return false;
    }

    QString strMsg;

    if (!m_UseCustomSetting)
    {
        // Original setting is selected
        strMsg = "This setting is unmodified (same as current Visual Studio setting).";
        strMsg += QString("\n") + "Do you want to save it as a custom setting";
    }
    else if (m_SavedSettingExist)
    {
        strMsg = "Do you want to save this setting as a custom setting";
    }

    if (!strMsg.trimmed().isEmpty())
    {
        if (m_SavedSettingExist)
        {
            strMsg += " and replace the existing saved setting";
        }

        strMsg += "?";

        int retVal = acMessageBox::instance().question(afGlobalVariablesManager::ProductNameA(),
                                                       strMsg.toLatin1().data(),
                                                       QMessageBox::Yes | QMessageBox::No,
                                                       QMessageBox::Yes);

        if (QMessageBox::No == retVal)
        {
            return false;
        }
    }

    ProfileSettingData storeData = restoreButton->text().contains(ORIGINALSTR) ? m_modifiedData : m_originalData;

    QString projName = m_projFullName.trimmed();

    bool replaced = false;

    try
    {
        QDomText txtValue;
        QDomDocument doc;
        QDomElement  itemProjSetting = doc.createElement("ProfileSetting");
        itemProjSetting.setAttribute(PLATFORMCONGISTR, m_platformConfig);
        itemProjSetting.setAttribute(PROJFULLNAMESTR, m_projFullName);

        QDomElement  subitemCommon = doc.createElement(APPPATHSTR);

        txtValue = doc.createTextNode(applicationPathComboBox->currentText().trimmed());
        subitemCommon.appendChild(txtValue);
        itemProjSetting.appendChild(subitemCommon);

        subitemCommon = doc.createElement(ACTPROJDIRSTR);
        txtValue = doc.createTextNode(workingDirectoryComboBox->currentText().trimmed());
        subitemCommon.appendChild(txtValue);
        itemProjSetting.appendChild(subitemCommon);

        subitemCommon = doc.createElement(CMDLINEARGSTR);
        txtValue = doc.createTextNode(cmdLineArgComboBox->currentText().trimmed());
        subitemCommon.appendChild(txtValue);
        itemProjSetting.appendChild(subitemCommon);

        subitemCommon = doc.createElement(MERGEENVSTR);
        txtValue = doc.createTextNode(Util::BoolToQString(mergeEnvironmentCheckBox->isChecked()));
        subitemCommon.appendChild(txtValue);
        itemProjSetting.appendChild(subitemCommon);

        subitemCommon = doc.createElement(USECUSTOMSETSTR);
        txtValue = doc.createTextNode(Util::BoolToQString(m_UseCustomSetting));
        subitemCommon.appendChild(txtValue);
        itemProjSetting.appendChild(subitemCommon);

        subitemCommon = doc.createElement(ENVVARSSTR);

        for (int j = 0; j < m_modelEnvironmentVariables->rowCount(); ++j)
        {
            QDomElement envItem = doc.createElement(VARSTR);
            envItem.setAttribute(NAMESTR, m_modelEnvironmentVariables->item(j, 0)->text());
            txtValue = doc.createTextNode(m_modelEnvironmentVariables->item(j, 1)->text());
            envItem.appendChild(txtValue);
            subitemCommon.appendChild(envItem);
        }

        itemProjSetting.appendChild(subitemCommon);

        QFileInfo f(m_profileSettingDataFileXML);

        if (f.exists())
        {
            QDomElement  root;

            QFile inFile(m_profileSettingDataFileXML);

            if (!doc.setContent(&inFile))
            {
                Util::ShowWarningBox("Error in saving profile data");
                return false;
            }

            inFile.close();
            root = doc.documentElement();
            int recordCount = root.elementsByTagName("ProfileSetting").count();

            if (recordCount > 0)
            {
                bool found = false;
                QDomElement currentProfileSetting;

                for (int i = 0; i < recordCount; ++i)
                {
                    QDomNode oldNode = root.childNodes().at(i);
                    currentProfileSetting = oldNode.toElement();
                    QString projFullName = currentProfileSetting.attribute(PROJFULLNAMESTR);
                    QString stringPlatformConfig = currentProfileSetting.attribute(PLATFORMCONGISTR);

                    if ((stringPlatformConfig == m_platformConfig) && (projFullName == m_projFullName))
                    {
                        root.replaceChild(itemProjSetting, oldNode);
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    root.appendChild(itemProjSetting);
                }
            }

            QFile file(m_profileSettingDataFileXML);

            if (!file.open(QIODevice::WriteOnly))
            {
                Util::ShowWarningBox("Error in saving profile data");
                return false;
            }

            QTextStream ts(&file);
            ts << doc.toString();
        }
        else
        {
            QDomElement  root =  doc.createElement("ProfileSettings");

            root.appendChild(itemProjSetting);
            doc.appendChild(root);

            QFile outFile(m_profileSettingDataFileXML);

            if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                Util::ShowWarningBox("Error in saving profile data");
                return false;
            }

            QTextStream t(&outFile);
            t << doc.toString();
        }
    }
    catch (...)
    {
        Util::ShowWarningBox("Error in saving profile data");
        replaced = false;
    }

    return replaced;
}

bool ProfileSettingDialog::ModifiedIsOriginal()
{
    try
    {
        if (!m_SavedSettingExist)
        {
            return false;
        }

        bool modifiedIsOriginalResult = true;

        if (m_originalData.ApplicationPath().trimmed().isEmpty() != m_modifiedData.ApplicationPath().trimmed().isEmpty())
        {
            modifiedIsOriginalResult = false;
        }
        else if ((!m_originalData.ApplicationPath().trimmed().isEmpty()) &&
                 (m_originalData.ApplicationPath().trimmed() != m_modifiedData.ApplicationPath().trimmed()))
        {
            modifiedIsOriginalResult = false;
        }
        else if (m_originalData.WorkingDirectory().trimmed().isEmpty() != m_modifiedData.WorkingDirectory().trimmed().isEmpty())
        {
            modifiedIsOriginalResult = false;
        }
        else if ((!m_originalData.WorkingDirectory().trimmed().isEmpty()) &&
                 (m_originalData.WorkingDirectory().trimmed() != m_modifiedData.WorkingDirectory().trimmed()))
        {
            modifiedIsOriginalResult = false;
        }
        else if (m_originalData.CommandlineArguments().trimmed().isEmpty() != m_modifiedData.CommandlineArguments().trimmed().isEmpty())
        {
            modifiedIsOriginalResult = false;
        }
        else if ((!m_originalData.CommandlineArguments().trimmed().isEmpty()) &&
                 (m_originalData.CommandlineArguments().trimmed() != m_modifiedData.CommandlineArguments().trimmed()))
        {
            modifiedIsOriginalResult = false;
        }
        else if (m_modifiedData.EnvVariableList().count() != m_originalData.EnvVariableList().count())
        {
            modifiedIsOriginalResult = false;
        }
        else
        {
            // Counts of 2 lists EnvironmentVariableListView and OriginalData.EnvVariableList are already same
            // so if one is contained within other, it implies they are equal
            QStringList originalList = m_originalData.EnvVariableList();

            foreach (QString s, m_modifiedData.EnvVariableList())
            {
                if (-1 == originalList.indexOf(s))
                {
                    modifiedIsOriginalResult = false;
                    break;
                }
            }
        }

        return modifiedIsOriginalResult;
    }
    catch (...)
    {
        return false;
    }
}

bool ProfileSettingDialog::ModifiedIsCurrent()
{
    try
    {
        if (!m_SavedSettingExist)
        {
            return false;
        }

        bool modifiedIsCurrentResult = true;

        if (applicationPathComboBox->currentText().trimmed().isEmpty() != m_modifiedData.ApplicationPath().trimmed().isEmpty())
        {
            modifiedIsCurrentResult = false;
        }
        else if ((!m_modifiedData.ApplicationPath().trimmed().isEmpty()) &&
                 (applicationPathComboBox->currentText().trimmed() != m_modifiedData.ApplicationPath().trimmed()))
        {
            modifiedIsCurrentResult = false;
        }
        else if (workingDirectoryComboBox->currentText().trimmed().isEmpty() != m_modifiedData.WorkingDirectory().trimmed().isEmpty())
        {
            modifiedIsCurrentResult = false;
        }
        else if ((!workingDirectoryComboBox->currentText().trimmed().isEmpty()) &&
                 (workingDirectoryComboBox->currentText().trimmed() != m_modifiedData.WorkingDirectory().trimmed()))
        {
            modifiedIsCurrentResult = false;
        }
        else if (cmdLineArgComboBox->currentText().trimmed().isEmpty() != m_modifiedData.CommandlineArguments().trimmed().isEmpty())
        {
            modifiedIsCurrentResult = false;
        }
        else if ((!cmdLineArgComboBox->currentText().trimmed().isEmpty()) &&
                 (cmdLineArgComboBox->currentText().trimmed() != m_modifiedData.CommandlineArguments().trimmed()))
        {
            modifiedIsCurrentResult = false;
        }

        if (m_modifiedData.EnvVariableList().count() != m_modelEnvironmentVariables->rowCount())
        {
            modifiedIsCurrentResult = false;
        }
        else
        {
            // Counts of 2 lists EnvironmentVariableListView and OriginalData.EnvVariableList are already same
            // so if one is contained within other, it implies they are equal
            QStringList modifiedList = m_modifiedData.EnvVariableList();

            for (int j = 0; j < m_modelEnvironmentVariables->rowCount(); ++j)
            {
                QString s = m_modelEnvironmentVariables->item(j, 0)->text();
                s.append("=");
                s.append(m_modelEnvironmentVariables->item(j, 1)->text());

                if (!modifiedList.contains(s))
                {
                    modifiedIsCurrentResult =  false;
                    break;
                }
            }
        }

        return modifiedIsCurrentResult;
    }
    catch (...)
    {
        return false;
    }
}



