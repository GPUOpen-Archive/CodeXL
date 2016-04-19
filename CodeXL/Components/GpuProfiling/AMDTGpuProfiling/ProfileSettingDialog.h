//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProfileSettingDialog.h $
/// \version $Revision: #5 $
/// \brief :  This file contains Profile setting dialog
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProfileSettingDialog.h#5 $
// Last checkin:   $DateTime: 2015/08/31 01:56:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538740 $
//=====================================================================
#ifndef _PROFILE_SETTING_DIALOG_H_
#define _PROFILE_SETTING_DIALOG_H_


#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#include <QtWidgets/qdialog.h>
#include <qstandarditemmodel.h>
#include "ui_ProfileSetting.h"

#include <AMDTGpuProfiling/Util.h>
#include "ProfileSettingData.h"

/// UI for profile setting dialog (currently unused in CodeXL)
class ProfileSettingDialog : public QDialog, private Ui::ProfileSetting
{
    Q_OBJECT
public:

    /// Initializes a new instance of the ProfileSetting class.
    /// \param parent parent widget
    ProfileSettingDialog(QWidget* parent = 0);

    /// Load the data for dialog
    /// \param profileSettingData source data
    void LoadDialogData(const ProfileSettingData& profileSettingData);

    /// Gets final data
    /// \param profileSettingData output data
    void GetProfileSetting(ProfileSettingData& profileSettingData);

    /// Load cache for fields applicationPath, Working Directory, cmdLineArg
    void LoadCache();

    /// Save cache for fields applicationPath, Working Directory, cmdLineArg
    void SaveCache();

    /// Gets the value whether original setting is selected.
    /// \return flag value to indicate if original data is selected.
    bool GetOriginalSelected() const { return m_OriginalSelected; }

    /// Sets the flag that original data is selected.
    /// \param OriginalSelected flag value to indicate if original data is selected.
    void SetOriginalSelected(bool OriginalSelected) { m_OriginalSelected = OriginalSelected; }

    /// Gets the value whether to use custom setting or not.
    /// \return flag value to indicate whether to use custom setting or not.
    bool GetUseCustomSetting() const { return m_UseCustomSetting;}
private:
    /// Loads the environment variables
    void InitializeFieldsAndControls();

    /// Load the UI with the input data
    /// \param loadOriginal to load original or modified data
    void LoadFields(bool loadOriginal);

    /// If dirty let restore
    void UpdateDataChange();

    /// Show Open File Dialog and select application path
    void SelectFile();


    /// Show Open File Dialog and select working directory
    void SelectDirectory();

    /// Change caption of 3 things button and 2 group boxes
    /// \param currentIsOriginal  if current data is original
    void ChangeCaptions(const bool currentIsOriginal);

    /// Set the data file name
    void UpdateProfSettingDataFile();

    /// Make restore button and captions in proper state
    void ProfileSetting_Shown();

    /// Common function for add and edit
    /// \param openInAddMode if on add mode
    void ShowEditNameValue(const bool openInAddMode);

    /// Validates the fields
    /// \return true if inputs are valid else false
    bool InputsAreValid();

    /// Find and load a record from xml file
    /// \return true if found else false
    bool LoadProfilerSettingData();

    /// Create or replace updated record
    /// \return if record is already there then true else false
    bool SaveProfilerSettingData();

    /// Check whether modified data is same as original data
    /// \return if modified is same as original then true else false
    bool ModifiedIsOriginal();

    /// Check whether modified data is same as current data
    /// \return if modified is same as current values then true else false
    bool ModifiedIsCurrent();

    /// Dialog fields' state indicator (original VS values)
    static const QString ORIGINALSTR;

    /// Dialog fields' state indicator (modified by user)
    static const QString CUSTOMSTR;

    /// Full string for group box caption for "Custom settings"
    static const QString CUSTOMSETTINGFULLSTR;

    /// Full string for message indicating Custom is Original
    static const QString SAMEASORIGINALSTR;

    /// Full string for group box caption for "Original settings"
    static const QString ORIGINALSETTINGFULLSTR;

    /// Tag name ApplicationPath
    static const QString PROJFULLNAMESTR;

    /// Tag name ApplicationPath
    static const QString APPPATHSTR;

    /// Tag name PlatformConfig
    static const QString PLATFORMCONGISTR;

    /// Tag name ActiveProjectDirectory
    static const QString ACTPROJDIRSTR;

    /// Tag name CmdLineArg
    static const QString CMDLINEARGSTR;

    /// Tag name MergeEnvironment
    static const QString MERGEENVSTR;

    /// Tag name UseCustomSetting
    static const QString USECUSTOMSETSTR;

    /// Tag name EnvironmentVariables
    static const QString ENVVARSSTR;

    /// Tag name one Environment Variable
    static const QString VARSTR;

    /// Attribute name of environment variable
    static const QString NAMESTR;

    /// Extension of record file
    static const QString ProfileSettingDataFileExtn;

    /// Size of cache of 3 fields
    static const int CacheSize;

    /// Cache file key for Application path combo box
    static const QString CACHE_APP_PATH_STR;

    /// Cache file key for Working Directory combo box
    static const QString CACHE_WORK_DIR_STR;

    /// Cache file key for Command in Option combo box
    static const QString CACHE_CMD_OPT_STR;

    /// Gets or sets a value indicating whether
    /// original or custom setting is selected
    bool m_OriginalSelected;

    /// Gets or sets full name of the project
    QString m_projFullName;

    /// Gets or sets a value indicating whether to use custom setting or not
    bool m_UseCustomSetting;

    /// Gets or sets a value indicating whether there exist custom setting or not
    bool m_SavedSettingExist;

    /// Cache for fields applicationPath, Working Directory, cmdLineArg
    QString m_cacheFileName;

    /// True if displayed data is original
    bool m_currentIsOriginal;

    /// Platform and config combined QString
    QString m_platformConfig;

    /// Data file name where profiler setting is saved
    QString m_profileSettingDataFileXML;

    /// Indicator of custom data exist or not
    bool m_customDataExist;

    /// Fields' original from VS
    ProfileSettingData m_originalData;

    /// Fields' modified by user
    ProfileSettingData m_modifiedData;

    /// Model for environment variable list
    QStandardItemModel* m_modelEnvironmentVariables;

    /// Model for environment variable tree view item selection
    QItemSelectionModel* m_pSelectionModel;
private slots:
    /// Open OpenFile dialog
    void onApplicationPathButton_Click();

    /// Open open directory dialog
    void onWorkingDirectoryButton_Click();

    /// Update data file path and allow restore dirty fields
    void onApplicationPathComboBox_TextChanged();

    /// Allow restore dirty fields
    void onWorkingDirectoryComboBox_TextChanged();

    /// Allow restore dirty fields
    void onCmdLineArgComboBox_TextChanged();

    /// Allow restore dirty fields
    void onMergeEnvironmentCheckBox_CheckedChanged();

    /// Restore other state of data
    void onRestoreButton_Click();

    /// Save data and signal Ok to caller
    void onSaveButton_Click();

    /// Add new environment variable
    void onNewButton_Click();

    /// Edit environment variable
    void onEditButton_Click();

    /// Delete environment variable
    void onDeleteButton_Click();

    /// To invoke the edit dialog on double click
    void onEnvironmentVariableListView_DoubleClick();

    /// Enable and disable edit, delete button
    void onUpdateSelectionChange();
};

#endif // _PROFILE_SETTING_DIALOG_H_


