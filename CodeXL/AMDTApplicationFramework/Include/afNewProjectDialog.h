//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afNewProjectDialog.h
///
//==================================================================================

#ifndef __AFNEWPROJECTDIALOG
#define __AFNEWPROJECTDIALOG

//Qt
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE


// Forward decelerations:
class acTextCtrl;
class acTreeCtrl;
class afApplicationCommands;
class afLineEdit;
class afBrowseAction;


// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// need to undef Bool after all includes so the moc will compile in Linux
#undef Bool

// ----------------------------------------------------------------------------------
// Class Name:          afNewProjectDialog : public QDialog
// General Description: Defines the dialog for the CodeXL project settings and new project
// Author:              Sigal Algranaty
// Creation Date:       09/4/2012
// ----------------------------------------------------------------------------------
class AF_API afNewProjectDialog : public QDialog
{
    Q_OBJECT

public:

    enum afDialogMode
    {
        AF_DIALOG_NEW_PROJECT = 0,
        AF_DIALOG_EDIT_PROJECT = 1
    };

    /// afFocusArea an enumeration for the area in the dialog on which the user can ask to set the focus.
    /// AF_FOCUS_REMOTE_HOST - focus on remote host:
    enum afFocusArea
    {
        AF_FOCUS_DEFAULT = 0,
        AF_FOCUS_REMOTE_HOST
    };

    virtual ~afNewProjectDialog();
    static afNewProjectDialog& instance();

    /// Show the project settings dialog:
    /// \param mode the dialog mode (edit / new)
    /// \param selectedTreeFilePath string representing the tree file path to select. If empty, select the previous one
    /// \param executablePath the requested executable path
    /// \param mode focusArea what should be focused when the dialog opens?
    void ShowDialog(afDialogMode mode, const gtString& selectedTreeFilePath, const gtString& executablePath = L"", afFocusArea focusArea = AF_FOCUS_DEFAULT);

    void setStoredProjectSessionType(const gtString& sessionType) {m_projectSettings.setLastActiveSessionType(sessionType);}

signals:

    void SettingsTreeSelectionAboutToChange();
    void OkButtonClicked();

public slots:

    void onExeChanged(const QString& strExe);
protected slots:

    void OnClickBrowseForExe();
    void OnApplicationPathBrowse();

    void OnClickBrowseForWindowsStoreApp();
    void OnExeEditingFinished();
    void OnRemoteHostTextChanged(const QString& text);
    void OnClickBrowseForPath();
    void OnEditEnvironmentVariables();
    void OnOkButton();
    void OnCancelButton();
    void OnRestoreDefaultSettings(bool keepProjectUnchanged = true);
    void OnSettingsTreeItemChanged();
    void OnSettingsTreeItemClicked();

    void OnProjectNameEdit(int oldCursor, int newCursor);
    void OnAppTypeRadioButtonSelect();
    void OnApplicationTypeRadioButtonSelect();
    void OnHostRadioButtonSelection(bool isSelected);

    void OnClickBrowseForSourceFilesDirectory();
    void OnClickBrowseForSourceRootDirectory();

    /// Is handling the test connection click signal:
    void OnTestConnection();

protected:
    /// Overrides QWidget:
    virtual void resizeEvent(QResizeEvent* pResizeEvent);

    void keyPressEvent(QKeyEvent* e);
    bool storeProjectSettings();

    void setRequestedExePath(const gtString& executablePath);
    void createDialogLayout();

    /// Adds the current extension settings page to the tree:
    /// \param extensionName a string containing the extension tree path, a list of strings separated by ",":
    /// \param pExtensionWidget the widget that will be used to configure the extension settings
    void AddExtensionSettingsPage(const gtString& extensionTreePath, QWidget* pExtensionWidget);

    void createGeneralPage();
    void fillGeneralPageData();
    void initDialogCurrentProjectSettings();
    bool isValidRemoteSettings(gtString& invalidMessageStr) const;
    void adjustGuiToHostChange(bool isRemote);

    void SelectTreeItemByTreePath(const QString& selectedTreeFilePath);

    /// Set the focus on the requested area. See afFocusArea enumeration to see the focus options:
    /// \param focusArea the area in the dialog on which the user wants to set the focus
    void SetFocusArea(afFocusArea focusArea);

    /// Check if the current settings are valid:
    /// \param invalidMessageStr the message describing the invalidity
    /// \param invalidExtensionTreePath the tree path of the extension with the problem
    /// \return true if the settings are valid
    bool AreSettingsValid(gtString& invalidMessageStr, gtString& invalidExtensionTreePath) const;

    /// Checks if project application paths(both remote and local) are valid, both executable path and working directory path
    /// \param isAppValid output parameter, true if application has valid path and application is an executable
    /// \param isWorkingFolderValid output parameter, true if working directory path exists
    /// \return None
    void IsApplicationPathsValid(bool& isAppValid, bool& isWorkingFolderValid) const;


    /// extracts port and host address from project settings
    /// \param dmnAddress output parameter, holds project port and host address
    /// \return true if extracted port address ok
    bool  GetRemotePortAddress(osPortAddress& dmnAddress) const;

    /// Fit the GUI controls for VS extension:
    void FitToVisualStudio();

    // Overrides QWidget:
    virtual void closeEvent(QCloseEvent* pEvent);

    int CalculateTreeWidth();

private:
    // Only afSingletonsDelete can delete my instance:
    friend class afMainAppWindow;

protected:

    // Static members:
    static afNewProjectDialog* m_spMySingleInstance;

    // Do not allow the use of my constructor:
    afNewProjectDialog();

    // Application commands handler:
    afApplicationCommands* m_pApplicationCommands;

    // The tab widget that wraps the dialog:
    acTreeCtrl* m_pSettingsTree;

    /// Displays the currently selected settings page:
    QFrame* m_pCurrentSettingsPageContainer;
    QStackedLayout* m_pCurrentSettingsFrameLayout;

    // General page widget:
    QGroupBox* m_pGeneralPage;

    QLabel* m_pProjectNameTitle;
    QLineEdit* m_pProjectNameTextEdit;

    QRadioButton* m_pProgramExeRadioButton;
    QLabel* m_pProgramExeLabel;
    QLineEdit* m_pProgramExeTextEdit;
    QToolButton* m_pBrowseForExeButton;
    afBrowseAction* m_pBrowseForExeButtonAction;

    QRadioButton* m_pWinStoreAppRadioButton;

    // Remote profiling/debugging.
    QRadioButton* m_pRemoteHostRadioButton;
    QRadioButton* m_pLocalHostRadioButton;
    QWidget* m_pDummyRemoteWidgetA;
    QWidget* m_pDummyRemoteWidgetB;
    QHBoxLayout* m_pRemoteHostLayoutA;
    QHBoxLayout* m_pRemoteHostLayoutB;
    afLineEdit* m_pRemoteHostIpLineEdit;
    QLabel* m_pRemoteHostPortLabel;
    QLineEdit* m_pRemoteHostPortLineEdit;
    QLabel* m_pRemoteHostAddressLabel;
    QPushButton* m_pTestConnectionButton;

    QLabel* m_pWorkingFolderTitle;
    QLineEdit* m_pWorkingFolderTextEdit;
    QToolButton* m_pWorkingFolderPathButton;
    afBrowseAction* m_pBrowseForWorkingFolderAction;

    QLabel* m_pProgramArgsTitle;
    QLineEdit* m_pProgramArgs;

    QLabel* m_pEnvironmentVariablesTitle;
    QLineEdit* m_pEnvironmentVariablesTextEdit;
    QToolButton* m_pEditEnvironmentVariables;

    // The created / edited project settings:
    apProjectSettings m_projectSettings;

    // Source code directory:
    QLabel* m_pSourceFilesDirectoryTitle;
    QLineEdit* m_pSourceFilesDirectoryTextEdit;
    QToolButton* m_pSourceFilesDirectoryButton;

    // Source code resolution root directory:
    QLabel* m_pSourceCodeRootDirectoryTitle;
    QLineEdit* m_pSourceCodeRootDirectoryTextEdit;
    QToolButton* m_pSourceCodeRootDirectoryButton;
    afBrowseAction* m_pSourceCodeRootDirectoryButtonAction;

    // Are we currently setting the controls data:
    bool m_initializingData;

    /// Is mapping from string list (describing tree items path) to a tree widget:
    QMap<QString, QTreeWidgetItem*> m_treePathToTreeItemMap;

    /// Saves the dialog mode (new / edit):
    afDialogMode m_dialogMode;

    /// Caching global flag (for readability):
    bool m_isRunningFromVS;

    /// A string representing the current selected file path of the left handed tree:
    QString m_selectedTreeFilePath;

};

#endif  // __AFNEWPROJECTDIALOG
