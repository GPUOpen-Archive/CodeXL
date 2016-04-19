//------------------------------ kaMultiSourceView.h ------------------------------

#ifndef __KAMULTISOURCEVIEW_H
#define __KAMULTISOURCEVIEW_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtMap.h>

// Framework:
#include <AMDTApplicationFramework/Include/afIDocUpdateHandler.h>

// Local
#include <AMDTKernelAnalyzer/src/kaDataTypes.h>

class acTabWidget;
class kaSourceCodeView;
class kaKernelView;
class kaSourceCodeTableView;
class afBrowseAction;

struct kaMultiSourceTabData
{
public:
    kaMultiSourceTabData() : m_pISASourceView(nullptr), m_pISASourceTableView(nullptr), m_pILSourceView(nullptr) {}
    /// ISA code viewer for for NI and EG families:
    kaSourceCodeView* m_pISASourceView;

    /// ISA code table viewer for new families:
    kaSourceCodeTableView* m_pISASourceTableView;

    /// IL code viewer:
    kaSourceCodeView* m_pILSourceView;

    /// isa file path:
    osFilePath m_isaFilePath;

    /// il file path:
    osFilePath m_ilFilePath;

    /// identify file Path:
    osFilePath m_identifyPath;

    /// ratio of tab splitter ratio for hide/show toggle:
    float m_splitterRatio;

    /// Export ISA table to csv file action for this tab
    QAction* m_pExportToCSVAction;
};

// ----------------------------------------------------------------------------------
// Class Name:          kaMultiSourceView : public QWidget
// General Description: multiple view with ability to source, IL, ISA at the same time
//
// Author:              Gilad Yarnitzky
// Creation Date:       29/9/2013
// ----------------------------------------------------------------------------------
class kaMultiSourceView : public QWidget, public afIDocUpdateHandler
{
    Q_OBJECT

    enum
    {
        ID_SOURCE_VIEW_SECTION = 0,
        ID_IL_VIEW_SECTION,
        ID_ISA_VIEW_SECTION,
        ID_VIEW_SECTION_NUMBER
    };
public:
    kaMultiSourceView(QWidget* pParent, const osFilePath& sourceFilePath, const osFilePath& mdiFilePath, int leftWidgetSize = 1, int rightWidgetSize = 5);
    virtual ~kaMultiSourceView();

    void AddView(const osFilePath& identifyFilePath, const osFilePath& isaFilePath, const osFilePath& ilFilePath, bool isGCN = true, int leftWidgetSize = 1, int rightWidgetSize = 1);

    //    const osFilePath& identifyFilePath() { return m_identifyPath; }

    /// Update the view by reloading the files in all the source views:
    bool updateView(bool selectedView = false);

    /// Edit menu commands
    virtual void onUpdateEdit_Copy(bool& isEnabled);
    virtual void onUpdateEdit_SelectAll(bool& isEnabled);
    virtual void onUpdateEdit_Find(bool& isEnabled);
    virtual void onUpdateEdit_FindNext(bool& isEnabled);

    virtual void onEdit_Copy();
    virtual void onEdit_SelectAll();
    virtual void onEdit_Find();
    virtual void onEdit_FindNext();

    /// Do the file save command (based on the application command file save):
    void FileSave();

    /// Do the file save as Command (based on the application command file save as):
    void FileSaveAs(kaSourceCodeView* pView);

    void FileSaveAs();

    /// Return the currently focused source code view:
    QWidget* focusedView();

    /// Store the current focused view (the view from which the find dialog was called):
    void storeFindClickedView();

    /// write the tabs info to a string to be restored when reopening the view:
    void WriteDataFileString(gtString& fileString);

    /// Update document interface implementation:
    virtual void UpdateDocument(const osFilePath& docToUpdate);

    /// Mark kernel: bring the caret and mark it with a yellow arrow
    void MarkKernelLine(int sourceLine);

    /// Unregsiter the document to be observed:
    void UnregisterDocument();

    /// set the update/not updated text for the IL/ISA source files
    void ShowUpdateNotUpdateCaption(bool showCaption);

    /// Access to the source view
    kaSourceCodeView* SourceView() { return m_pSourceView; }

    void resizeEvent(QResizeEvent* event);

    /// Updates active view when MDI sub window changes
    void UpdateDirtyViewsOnSubWindowChange();

public slots:
    /// save file as
    void SaveAs();

    /// save ISA instructions parts as csv file on request
    void OnExportToCSV();

    /// Show line numbers in internal source code views:
    void ShowLineNumbers(bool show);

protected slots:
    /// handle close view action:
    void TabCloseRequestedHandler(int index);

    void onSource();
    void onISA();
    void onIL();

    void OnApplicationFocusChanged(QWidget* pOld, QWidget* pNew);
    void onTextChanged();

    // handle the movement of the splitter:
    void OnSplitterMoved(int pos, int index);

    /// updates ISA table view
    void UpdateDirtyViewsOnTabChange(int nTab);

private:
    /// Add a view to a splitter, creating the layout with the caption in the right place for it:
    void AddViewToSplitter(QSplitter* pSplitter, QWidget* pView, QString caption, const QString& kernelName);

    /// Add all the source hide/show actions for the view menu with a separator:
    void AddMenuItemsToSourceView(kaSourceCodeView* pView);

    /// Add all the source hide/show actions for the view menu with a separator:
    void AddMenuItemsToSourceTableView(kaSourceCodeTableView* pView);

    /// Check enabling of the actions based on the amount of visible views:
    void enableActions();

    /// Hide tab splitter view widget left/right side
    void TabSplitterViewsToShow(int sideToDisplay, bool show);

    /// In the tab splitter make sure that a specific side is visible out of the two:
    void TabSplitterEnsureSideVisiblity(int sideToDisplay);

    /// show/hide widget in a specific splitter
    void SplitterViewsToShow(QSplitter* pSplitter, float* pRatio, int sideToDisplay, bool show);

    /// update view caption in a splitter
    void ShowUpdateNotUpdateCaptionInSplitter(QWidget* pWidget, bool showCaption);

private:
    /// Set the sizes of widgets in splitter based on ratios
    void SetSplitterSizeBasedOnRatios(QSplitter* pSplitter, const QList<int>& ratioList);

    /// Main view layout:
    QBoxLayout* m_pMainLayout;

    /// Source code viewer:
    kaSourceCodeView* m_pSourceView;

    // Main splitter that holds the source view and the tab control:
    QSplitter* m_pSplitter;

    /// Holds all the internal splitters that holds the IL/ISA:
    acTabWidget* m_pTabWidget;

    /// displayed views (the flag applies to all tabs):
    bool m_displayedViews[3];

    /// Menu actions for the show/hide source views):
    QAction* m_pActions[ID_VIEW_SECTION_NUMBER];

    /// Contain current focused view (the view from which the find dialog was called):
    QWidget* m_pFindSourceCodeView;

    /// map of filePaths and created views:
    gtMap<gtString, int> m_identifyPathToViewMap;

    /// map between the tab index and the data of the tab:
    gtMap<int, kaMultiSourceTabData*> m_tabDataMap;

    /// Parent kernel view needed for accessing writeData function:
    kaKernelView* m_pParentKernelView;

    /// ratio of source splitter for hide/show toggle:
    float m_splitterRatio;

    /// stores the state of the up to date flag
    bool m_isNotUpToDateShown;

    /// Is an opencl file (needed to know what type of info to display about in the captions and menu)
    kaPlatform m_platformIndicator;

    /// Initial ratio list to be used in first resize event that has the widget initialized
    QList<int> m_widgetSizesRatio;

    /// MDI file path that holds the multi source view. Used to identify the source view actions
    osFilePath m_mdiFilePath;

    /// Indicates that IL view is hidden
    bool m_isILViewHidden;

    /// Indicates that ISA view is hidden
    bool m_isISAViewHidden;

    /// Indicates that ISA view is empty
    bool m_isISAViewEmpty;

    /// Indicates that IL view is empty
    bool m_isILViewEmpty;

    /// Active tab export ISA table view to csv
    afBrowseAction* m_pExportToCSVAction;
};

#endif // __KAMULTISOURCEVIEW_H
