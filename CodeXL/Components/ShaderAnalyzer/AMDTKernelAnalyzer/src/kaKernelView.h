//------------------------------ kaKernelView.h ------------------------------

#ifndef __KAKERNELVIEW_H
#define __KAKERNELVIEW_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acTabWidget.h>

// Framework:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>

// Local:
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>

class kaOverviewView;
class kaMultiSourceView;

class KA_API kaKernelView : public QWidget, public afBaseView
{
    Q_OBJECT

public:
    kaKernelView(QWidget* pParent);
    virtual ~kaKernelView();

    // Display a specific file in a tab:
    void displayFile(const osFilePath& detailedFilePath, const osFilePath& kernelFilePath, int nodeType, QString nodeStr = QString());

    /// set dataFile:
    /// reads the data file info if this is the first time data file is set:
    void setDataFile(const osFilePath& dataFile);
    const osFilePath& dataFile() const { return m_dataFile; }

    /// Update the overview control, only used in the VS where overview is inserted as a special case:
    void updateOverview();

    /// Show line number in the internal tabs:
    void showLineNumbers(bool show);

    /// Update the view by updating all the views in the tab control:
    void updateView(bool selectedView = false);

    /// Edit menu commands
    virtual void onUpdateEdit_Cut(bool& isEnabled);
    virtual void onUpdateEdit_Copy(bool& isEnabled);
    virtual void onUpdateEdit_Paste(bool& isEnabled);
    virtual void onUpdateEdit_SelectAll(bool& isEnabled);
    virtual void onUpdateEdit_Find(bool& isEnabled);
    virtual void onUpdateEdit_FindNext(bool& isEnabled);

    virtual void onEdit_Cut();
    virtual void onEdit_Copy();
    virtual void onEdit_Paste();
    virtual void onEdit_SelectAll();
    virtual void onEdit_Find();
    virtual void onEdit_FindNext();

    /// Enable the "file save" and ""file save as" commands
    void OnUpdateFileSave(bool& isEnabled);

    /// Pass the file Save to the Multi Source for handling the command
    void FileSave();

    /// Pass the file Save to the Multi Source for handling the command
    void FileSaveAs();

    /// Store the source code view from which the find dialog was called:
    void storeFindClickedView();

    /// write data file with the names of the opened tabs:
    void writeDataFile();

    /// Marks the kernel line:
    void MarkKernelLine(kaMultiSourceView* pMultiSourceView);

    /// Get the kernel source file that is used
    osFilePath& sourceFile() { return m_sourcePath; };

    /// close all tabs
    void CloseAllTabs();

    /// Get the active Multi Source View:
    kaMultiSourceView* GetActiveMultiSourceView();

private slots:
    /// handle close view action:
    void tabCloseRequestedHandler(int index);

    /// Slots implementing the find command. Notice: This slot names cannot be changed, since it is connected in the construction of the main window
    /// Is called when the main window find is clicked:
    void onFindClick();

    /// Is called when the main window find next is clicked:
    void onFindNext();

    /// Is called when the main window find previous is clicked:
    void onFindPrev();



private:
    /// Create a multi source view with the correct internal source file paths:
    QWidget* createMultiSourceView(const osFilePath& filePath, const osFilePath& kernelFilePath, bool isGCN = true);

    /// Handle the close event: clear the data file:
    virtual void closeEvent(QCloseEvent* event);

private:

    /// Main view layout:
    QLayout* m_pMainLayout;

    /// Holds all the internal views:
    acTabWidget* m_pTabWidget;

    /// map of filePaths and created views:
    gtMap<gtString, int> m_filePathToViewMap;

    /// data file to be used with vs:
    osFilePath m_dataFile;

    /// source file path in data file:
    osFilePath m_sourcePath;

    /// overview for VS
    kaOverviewView* m_pOverView;
};
#endif // __KAKERNELVIEW_H