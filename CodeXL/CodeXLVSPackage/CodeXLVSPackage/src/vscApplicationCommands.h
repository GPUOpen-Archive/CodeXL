//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscApplicationCommands.h
///
//==================================================================================

//------------------------------ vspApplicationCommands.h ---------------------------

#ifndef __VSPAPPLICATIONCOMMANDS_H
#define __VSPAPPLICATIONCOMMANDS_H


// Infra:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>

// Qt:
#include <QtWidgets>

// Local:
#include <src/vspWindowsManager.h>
#include <Include/Public/CoreInterfaces/IVscApplicationCommandsOwner.h>


// ----------------------------------------------------------------------------------
// Class Name:              vspApplicationCommands : public gdApplicationCommands
// General Description:     This class is handling application commands for CodeXL
//                          Visual Studio package
//                          The class contain only commands with different implementations
//                          in standalone and VS package, and that are used from somewhere
//                          else then the application menu
// Author:                  Sigal Algranaty
// Creation Date:           7/2/2011
// ----------------------------------------------------------------------------------
class vscApplicationCommands : public QObject, public gdApplicationCommands, public afApplicationCommands
{
    Q_OBJECT

public:
    vscApplicationCommands();
    virtual ~vscApplicationCommands();

    // Sets the class's owner in terms of VS interaction.
    static void setOwner(IVscApplicationCommandsOwner* pOwner);

    // The following functions should be implemented for each inherited class of this class:

    // Breakpoints:
    virtual bool openBreakpointsDialog();
    virtual bool isBreakpointsDialogCommandEnabled();

    /// \param filePath the full path of the file
    /// \param lineNumber line number, or -1 if not applicable
    /// \param programCounterIndex counter index for source files, or -1 if not applicable
    /// \param viewIndex used for internal implementation. Used for indexing of inner views
    virtual bool OpenFileAtLine(const osFilePath& filePath, int lineNumber, int programCounterIndex = -1, int viewIndex = -1);

    virtual bool closeFile(const osFilePath& filePath);
    virtual bool saveMDIFile(const osFilePath& filePath);

    /// Find the list of opened windows for a file represented in pKAData
    /// \param containingDirectory the directory in which files should be looked for
    /// \param listOfOpenedWindows[out] a list of file paths which has windows opened, related to this file
    virtual void GetListOfOpenedWindowsForFile(const gtString& containingDirectory, gtVector<osFilePath>& listOfOpenedWindows);

    virtual void displayOpenCLProgramSourceCode(afApplicationTreeItemData* pProgramItemData);
    virtual void displayOpenGLSLShaderCode(afApplicationTreeItemData* pShaderItemData);
    virtual bool displayImageBufferObject(afApplicationTreeItemData* pItemData, const gtString& itemText);


    // Show a message box:
    virtual void showMessageBox(const gtString& caption, const gtString& message, osMessageBox::osMessageBoxIcon icon = osMessageBox::OS_DISPLAYED_INFO_ICON);

    /// A wrapper function, Display a Message box according to type: question, warning, critical or information
    virtual QMessageBox::StandardButton ShowMessageBox(QMessageBox::Icon type, const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton);

    // Start debugging command:
    virtual void buildProcessStopString(gtString& propertiesInfo);

    // Error report dialog:
    virtual bool shouldReportClientApplicationCrash(const osCallStack& clientAppCrashStack);

    // Accessors for the single instance view objects:
    virtual afPropertiesView* propertiesView() {return vspWindowsManager::instance().propertiesView(NULL, QSize(-1, -1));}
    virtual afApplicationTree* applicationTree() {return vspWindowsManager::instance().monitoredObjectsTree(NULL, QSize(-1, -1));}
    virtual void setViewLineNumbers(bool show);
    virtual gdAPICallsHistoryPanel* callsHistoryPanel() {return vspWindowsManager::instance().callsHistoryPanel(NULL, QSize(-1, -1));}
    virtual gdMemoryView* memoryView() {return vspWindowsManager::instance().memoryView(NULL, QSize(-1, -1));}
    virtual gdStatisticsPanel* statisticsPanel() {return (gdStatisticsPanel*)vspWindowsManager::instance().statisticsPanel(NULL, QSize(-1, -1));}
    virtual gdCallStackView* callStackView() {return NULL;}
    virtual gdDebuggedProcessEventsView* debuggedProcessEventsView() {return NULL;}
    virtual gdStateVariablesView* stateVariablesView() {return vspWindowsManager::instance().stateVariablesView(NULL, QSize(-1, -1));}
    virtual gdPerformanceGraphView* performanceGraphView() {return NULL;}
    virtual gdPerformanceDashboardView* performanceDashboardView() {return NULL;}
    virtual gdCommandQueuesView* commandQueuesView() {return NULL;}
    virtual gdMultiWatchView* multiWatchView(int viewIndex) { return (gdMultiWatchView*)vspWindowsManager::instance().multiwatchView(NULL, QSize(-1, -1), viewIndex + ID_MULTIWATCH_VIEW_FIRST); }

    // Set frame caption:
    virtual void setApplicationCaption(const gtString& caption);
    virtual bool setWindowCaption(QWidget* pWidget, const gtString& windowCaption);
    virtual const gtString captionPrefix();

    // Raise view commands:
    virtual bool raiseStatisticsView();
    virtual bool raiseMemoryView();

    virtual void onFileExit() {}

    // QT:
    virtual int showModal(QDialog* pDialog);

    // Update UI:
    virtual void updateToolbarCommands();

    // Project
    virtual bool UpdateRecentlyUsedProjects() { return false; }
    virtual void onFileOpenStartupDialog();
    virtual void OnFileSaveProject();
    virtual void OnFileSaveProjectAs();
    virtual void OnFileOpenWelcomePage() {};
    virtual void OnFileCloseProject(bool shouldOpenWelcomePage);

    // Projects:
    virtual void updateProjectSettingsFromImplementation();

    // information view:
    virtual void ClearInformationView();
    /// function process message, and extract short error messages that contains file name
    /// and line number so it can post them separately in a way that enable connecting message to file and line listed in it
    virtual void AddStringToInformationView(const QString& messageToDisplay);

    // Service function:
    /// Close all open MDI views that were connected to the files in the directory
    /// go through all MDI and check that the file related to the MDI still exists:
    virtual void closeDocumentsOfDeletedFiles();

    /// save all mdi windows that are related to the supplied filepath
    virtual void SaveAllMDISubWindowsForFilePath(const osFilePath& filePath);

public slots:

    void PrintToDebugLog(const QString& msg);

private:

    static IVscApplicationCommandsOwner* m_pOwner;

    void updateAPIBreakpointsForBreakpointsDialog();
    bool updateAPISingleBreakpoint(const gtString& breakpointName, bool isEnabled);

    /// This map is cleared at every build and populated with [stage;file path] pairs at glsl program build
    QMap<QString, QString> m_stagePathMap;
};



#endif  // __VSPAPPLICATIONCOMMANDS_H

