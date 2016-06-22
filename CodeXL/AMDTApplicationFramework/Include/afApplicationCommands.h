//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afApplicationCommands.h
///
//==================================================================================


#ifndef __AFGAPPLICATIONCOMMANDS_H
#define __AFGAPPLICATIONCOMMANDS_H

// Qt:
#include <QtWidgets>

class afPropertiesView;
class afBrowseAction;


// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/dialogs/afSystemInformationDialog.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
// A private class used to handle icons, to avoid crashes on Linux:
class afFilteredFileIconProvider : public QFileIconProvider
{
public:
    afFilteredFileIconProvider() {};
    virtual ~afFilteredFileIconProvider() {};
    virtual QIcon icon(IconType type) const
    {
        gtString iconPath;
        bool retVal = afGetApplicationImagesPath(iconPath);
        GT_ASSERT(retVal)

        iconPath.append(osFilePath::osPathSeparator);

        switch (type)
        {
            case QFileIconProvider::Folder: iconPath.append(AC_STR_image_folder); break;

            case QFileIconProvider::File:   iconPath.append(AC_STR_image_file); break;

            default:
                /*case QFileIconProvider::Computer:
                case QFileIconProvider::Desktop:
                case QFileIconProvider::Trashcan:
                case QFileIconProvider::Network:
                case QFileIconProvider::Drive:*/
                iconPath.append(AC_STR_image_file);
                break;
        }

        QString iconPathQStr(QString::fromWCharArray(iconPath.asCharArray()));

        QIcon returnIcon(iconPathQStr);

        return returnIcon;
    };

    virtual QIcon icon(const QFileInfo& info) const
    {
        // Avoid showing the icons for image files:
        if (info.isFile())
        {
            return icon(QFileIconProvider::File);
        }
        else if (info.isDir())
        {
            return icon(QFileIconProvider::Folder);
        }
        else // info
        {
            // Get the normal icon:
            return icon(QFileIconProvider::File);
        }
    };
    virtual QString type(const QFileInfo& info) const
    {
        return m_defaultFileIconProvider.type(info);
    };

private:
    QFileIconProvider m_defaultFileIconProvider;
};
#endif

/// afCodeXLSample en enumeration for CodeXL samples
enum afCodeXLSampleID
{
    AF_SAMPLE_NONE = -1,
    AF_TEAPOT_SAMPLE = 0,
    AF_MATMUL_SAMPLE,
    AF_D3D12MULTITHREADING_SAMPLE
};

// ----------------------------------------------------------------------------------
// Class Name:              afApplicationCommands
// General Description:     This class is handling application commands that are implemented
//                          by the framework
// Author:                  Sigal Algranaty
// Creation Date:           4/4/2012
// ----------------------------------------------------------------------------------
class AF_API afApplicationCommands
{
public:

    virtual ~afApplicationCommands();

    // File menu commands:
    virtual void OnFileSaveProject();
    virtual void OnFileSaveProjectAs();
    virtual void onFileSaveFile();
    virtual void onFileSaveFileAs();
    virtual void OnFileNewProject(const gtString& executablePath = L"");
    virtual void OnFileOpenProject(const gtString& projectFilePath);

    /// Close the current project:
    virtual void OnFileCloseProject(bool shouldOpenWelcomePage);
    virtual void onFileOpenFile();
    virtual void onFileExit();
    virtual bool promptForExit();
    virtual void onUpdateProjectSave(bool& isEnabled);
    virtual void onUpdateProjectClose(bool& isEnabled);
    virtual void onUpdateProjectSaveAs(bool& isEnabled);
    virtual void onUpdateFileSave(bool& isEnabled);

    /// CSV file selection dialog:
    /// QString& selectedFilePath, const QString& fileNamePostFix, QWidget* pParent
    bool ShowQTSaveCSVFileDialog(QString& selectedFilePath, const QString& fileNamePostFix, QWidget* pParent, const QString& title = QString());


    bool ShowQTSaveFileDialog(QString& selectedFilePath, const QString& fileNamePostFix, const QString& defaultPath, QWidget* pParent, const QString& extension, const QString& title);

    // Properties view:
    virtual afPropertiesView* propertiesView();

    // Application tree:
    virtual afApplicationTree* applicationTree();
    virtual gtString applicationRootString();

    // information view:
    virtual void ClearInformationView();
    virtual void AddStringToInformationView(const QString& messageToDisplay);

    /// Add debug performance printouts to information view. Use this function to look for performance bottlenecks:
    void StartPerformancePrintout(const QString& tagName);
    void EndPerformancePrintout(const QString& tagName);

    // Source code view:
    virtual void setViewLineNumbers(bool show) = 0;
    virtual bool closeFile(const osFilePath& filePath) = 0;
    // save and mdi view of a specific path:
    virtual bool saveMDIFile(const osFilePath& filePath);

    /// \param filePath the full path of the file
    /// \param lineNumber line number, or -1 if not applicable
    /// \param programCounterIndex counter index for source files, or -1 if not applicable
    /// \param viewIndex used for internal implementation. Used for indexing of inner views
    virtual bool OpenFileAtLine(const osFilePath& filePath, int lineNumber, int programCounterIndex = -1, int viewIndex = -1) = 0;

    /// Open the folder in which the file is contained
    /// \param filePath the file for which the folder should be opened
    virtual void OpenContainingFolder(const osFilePath& filePath);

    /// Find the list of opened windows for a file represented in pKAData
    /// \param containingDirectory the directory in which files should be looked for
    /// \param listOfOpenedWindows[out] a list of file paths which has windows opened, related to this file
    virtual void GetListOfOpenedWindowsForFile(const gtString& containingDirectory, gtVector<osFilePath>& listOfOpenedWindows);

    /// Attach an item data to MDI source code view:
    virtual bool AttachItemDataToSourceView(const osFilePath& filePath, const afApplicationTreeItemData* pTreeItemData);

    virtual bool FillRecentlyUsedProjectsNames(gtVector<gtString>& projectNames, gtString& currentProjectPath, bool showAll = false);
    virtual bool OnFileRecentProject(gtVector<gtString>& projectsNames, int projectIndex);
    virtual bool UpdateRecentlyUsedProjects();
    virtual void OnFileOpenWelcomePage();

    // Tools menu commands:
    virtual void onToolsSystemInfo(afSystemInformationDialog::InformationTabs selectedTab = afSystemInformationDialog::SYS_INFO_SYSTEM);
    virtual void onToolsOptions(const gtString& openingTab = L"");

    // Main frame caption:
    virtual void setApplicationCaption(const gtString& caption);
    virtual void setActiveWindowCaption(const gtString& caption);
    virtual bool setWindowCaption(QWidget* pWidget, const gtString& windowCaption);
    virtual const gtString captionPrefix() {return L"";}

    /// Mark a MDI window as changed (add "*") to the title, or remove it:
    /// \param mdiFilePath the MDI window file path
    /// \param isChanged is the MDI window changed, or not
    virtual void MarkMDIWindowAsChanged(const osFilePath& mdiFilePath, bool isChanged);


    // Error report dialog:
    virtual bool shouldReportClientApplicationCrash(const osCallStack& clientAppCrashStack);

    /// Display Qt multiple files selection dialog:
    /// \param selectedFilesVector[out] the list of files selected
    /// \param dialogCaption the title for the dialog
    /// \param defaultFileFullPath the initial value selected in the dialog
    /// \param fileFilters a list of file filters separated by ";"
    /// \return true if the user selected at least one path
    virtual bool ShowMultipleFilesSelectionDialog(gtVector<osFilePath>& selectedFilesVector, const QString& dialogCaption, const QString& defaultFileFullPath, const QString& fileFilters);

    /// Display Qt file selection dialog:
    /// \param dialogCaption the title for the dialog
    /// \param defaultFileFullPath the initial value selected in the dialog
    /// \param fileFilters a list of file filters separated by ";"
    /// \param pBrowseAction the browse action is used (browse action is used to store and load the last path to the global settings)
    /// \param saveFile the browse action is used
    /// \return the folder path or an empty string if the user clicked cancel
    virtual QString ShowFileSelectionDialog(const QString& dialogCaption, QString& defaultFileFullPath, const QString& fileFilters, afBrowseAction* pBrowseAction, bool saveFile = false);

    /// Display Qt folder selection dialog:
    /// \param dialogCaption the title for the dialog
    /// \param defaultFolder the initial value selected in the dialog
    /// \param fileBrowseLocationFieldID the field ID on which we should store the last browse folder (the last path will be loaded to the global settings)
    /// \return the folder path or an empty string if the user clicked cancel
    virtual QString ShowFileSelectionDialogWithBrowseHistory(const QString& dialogCaption, const QString& fileFilters, bool saveFile, const QString& fileBrowseLocationFieldID);

    /// Display Qt folder selection dialog:
    /// \param dialogCaption the title for the dialog
    /// \param defaultFolder the initial value selected in the dialog
    /// \param pBrowseAction the browse action is used (browse action is used to store and load the last path to the global settings)
    /// \return the folder path or an empty string if the user clicked cancel
    virtual QString ShowFolderSelectionDialog(const QString& dialogCaption, QString& defaultFolder, afBrowseAction* pBrowseAction = nullptr);

    /// The function identifies CodeXL samples source files. If the source file path cannot be found on the local machine, 
    /// we assume that this is an installed binary pdb, with our machines paths. 
    /// In this case we convert it to the samples location folder
    /// \param srcFilePath the source file path
    /// \param localSrcFilePath[out] the same source file path on the local machine
    /// \return true iff this is a sample file path
    bool ConvertSamplesFilePath(const osFilePath& srcFilePath, osFilePath& localSrcFilePath);

    // Enable / disable commands:
    virtual void enableWhenNoProcess(bool& isEnabled) const;

    // Qt:
    virtual int showModal(QDialog* pDialog);
    virtual QWidget* applicationMainWindow() {return nullptr;}

    // Update UI:
    virtual void updateToolbarCommands();

    /// Edit project remote host:
    virtual void OnProjectSettingsEditRemoteHost();

    /// Open the project settings dialog for edit:
    virtual void OnProjectSettings(const gtString& projectSettingsPath = L"");

    virtual void onUpdateDebugSettings(bool& isEnabled);

    // Help menu commands:
    virtual void onHelpUserGuide();
    virtual void onHelpQuickStart();
    virtual void onHelpUpdates();
    virtual void onHelpOpenURL(const gtString& urlStr);
    virtual void onHelpAbout();
    virtual void openCodeXLWebsite();

    // View menu commands:
    virtual void onViewResetGUILayout();
    virtual void bringDockToFront(const gtString& dockName);


    // Projects:
    virtual void getProjectsFilePath(const gtString& projectName, osFilePath& projectFilePath);
    virtual void updateProjectSettingsFromImplementation();

    // Return my single instance:
    static afApplicationCommands* instance();

    /// Load CodeXL sample:
    /// \param sampleId sample id (afCodeXLSample enumeration)
    void LoadSample(afCodeXLSampleID sampleId);

    /// Return the single instance static find dialog:
    static acFindWidget* GetFindWidget() { return m_spFindWidget; };

    /// A wrapper function to opens a message box
    /// @param[in] QMessageBox::Icon type  Message bos type: critical, information, question or warning
    /// @param[in] title Message box title
    /// @param[in] buttons  Message box buttons
    /// @param[in] defaultButton button with initial focus
    /// @return    button pressed
    virtual QMessageBox::StandardButton ShowMessageBox(QMessageBox::Icon type, const QString& title, const QString& text, QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);


#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    // Special file dialog image services:
    QIcon getToolButtonIcon(QToolButton* pToolButton);
    QIcon getPushButtonIcon(QPushButton* pPushButton);
    void prepareDialog(QFileDialog& dialog);
#endif

    // Service function:
    /// Close all open MDI views that were connected to the files in the directory
    /// go through all MDI and check that the file related to the MDI still exists:
    virtual void closeDocumentsOfDeletedFiles();


    /// Create default empty project and set it as current:
    virtual void CreateDefaultProject(const gtString& exeMode, const gtString& sessionType = L"");

    /// Finds the next available default project name:
    virtual QString FindDefaultProjectName();

    /// Clears the properties view: set the current selected item in tree
    virtual void RestorePropertiesViewToCurrentlySelectedItem();

    /// Create the cxl file for a sample:
    /// \param sampleId sample id (afCodeXLSample enumeration)
    bool WriteSampleCXL(afCodeXLSampleID sampleId);

    /// Get the sample properties for the requested sample
    /// \param sampleId the sample id
    /// \param sampleName the sample name
    /// \param sampleMode the sample default exe mode
    /// \param sampleSessionType the sample default session type
    /// \param sampleDirName the sample directory name
    /// \param sampleBinaryName the sample binary name
    /// \param sampleProjectName the sample project name
    /// \param buildOptions the build options should be set for the sample
    void GetSampleProperties(afCodeXLSampleID sampleId, gtString& sampleName, gtString& sampleMode, gtString& sampleSessionType,
                             gtString& sampleDirName, gtString& sampleBinaryName, gtString& sampleProjectName, gtString& buildOptions);

    /// Get the sample source code directories
    /// \param sampleId the sample id
    /// \param samplePath the sample path
    /// \param sourceFileDirectories the list of source directories for this sample, separated by a ";"
    void GetSampleSourceFileDirectories(afCodeXLSampleID sampleId, const osFilePath& samplePath, gtString& sourceFileDirectories);

    // Register my instance (this function is private, to make sure that only approved
    // classes access this function):
    static bool registerInstance(afApplicationCommands* pApplicationCommandsInstance);

    /// Check which drag action is needed for the current drop event:
    /// \param pEvent the event describing the dragged files list
    /// \return the action needed for the dragged files (no action in case of empty list / mixed files list)
    virtual afApplicationTree::DragAction DragActionForDropEvent(QDropEvent* pEvent);

    // Handle a drop event
    void HandleDropEvent(QWidget* receiver, QDropEvent* pEvent);

    /// save all mdi windows that are related to the supplied filepath
    virtual void SaveAllMDISubWindowsForFilePath(const osFilePath& filePath) = 0;

protected:

    // Do not allow the use of my constructor:
    afApplicationCommands();

    /// Build the CL files section in the teapot CXL text:
    /// \param resDirectory the location of the res folder (in which all the cl files should be located)
    /// \param cxlContent the CXL file text for editing
    void BuildSampleCLFilesSection(const afCodeXLSampleID sampleId, const osDirectory& resDirectory, gtString& cxlContent);

    /// Open a new project dialog for the dragged file:
    /// \param pEvent This function should only be called when pEvent has a list of 1 exe file
    void OpenNewProjectForDraggedFile(QDropEvent* pEvent);

public:
    // This should only be used at module destruction parallel to where the instance was created:
    static void cleanupInstance();
protected:
    /// My single instance:
    static afApplicationCommands* m_spMySingleInstance;

    /// True iff we are during a load process:
    static bool m_sIsInLoadProcess;


    /// A static instance of the find dialog:
    static acFindWidget* m_spFindWidget;

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    afFilteredFileIconProvider globalFileIconProvider;
#endif

    /// Is used for performance printouts in debug:
    QTime m_performanceTimer;

    /// True iff performance timestamps should be printed to the application output window.
    /// This flag is true when an environment variable named AF_CODEXL_PRINT_TIMESTAMPS is defined and has value=TRUE
    bool m_shouldPrintPerformanceTimestamps;
};


#endif  // __AFGAPPLICATIONCOMMANDS_H
