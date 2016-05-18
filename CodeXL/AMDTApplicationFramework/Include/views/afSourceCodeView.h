//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afSourceCodeView.h
///
//==================================================================================

#ifndef __AFSOURCECODEVIEW__H
#define __AFSOURCECODEVIEW__H

// Forward declaration:
class afApplicationCommands;
class apBreakPoint;
class acFindWidget;

// Infra:
#include <AMDTApplicationComponents/Include/acSourceCodeView.h>
#include <AMDTApplicationComponents/Include/acFindParameters.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationFramework/Include/afIDocUpdateHandler.h>
#include <AMDTApplicationFramework/Include/views/afBaseView.h>

// need to undef Bool after all includes so the moc will compile in Linux
#undef Bool

class afApplicationTreeItemData;

// ----------------------------------------------------------------------------------
// Class Name:          AF_API afSourceCodeView: public acSourceCodeView
// General Description: Inherit acSourceCodeView and implement a code editor functionality
//                      in CodeXL application
// Author:              Sigal Algranaty
// Creation Date:       3/8/2011
// ----------------------------------------------------------------------------------
class AF_API afSourceCodeView: public acSourceCodeView, public afBaseView, public afIDocUpdateHandler
{
    Q_OBJECT
public:
    afSourceCodeView(QWidget* pParent, bool shouldShowLineNumbers, unsigned int contextMenuMask = (unsigned int)AC_ContextMenu_Debug);
    virtual ~afSourceCodeView();

    // Overriding acSourceCodeView:
    virtual bool displayFile(const osFilePath& fileName, int lineNumber, int pcIndex);

    void updateBreakpointsFromTheAPI();

    /// Update document interface implementation:
    virtual void UpdateDocument(const osFilePath& docToUpdate);

    /// \param[out] selectedText returns the text currently selected
    virtual void GetSelectedText(gtString& selectedText);

    /// Set the matching tree item data:
    void SetMatchingTreeItemData(const afApplicationTreeItemData* pMatchingTreeItemData) { m_pMatchingTreeItemData = pMatchingTreeItemData; };

signals:
    /// Signal for getting a double clicked text line
    /// \param[in] textLine   A line of text
    void textLineDoubleClicked(const QStringList& fullTextAsLines, const int clickedLinePosition);

    /// Signal clicked after the find was calling:
    void FindResultsChanged(bool wasFound);

public slots:


    /// Slots implementing the find command. Notice: This slot names cannot be changed, since it is connected in the construction of the main window
    /// Is called when the main window find is clicked:
    virtual void onFindClick();

    /// Is called when the main window find next is clicked:
    virtual void onFindNext();

    /// Is called when the main window find previous is clicked:
    virtual void onFindPrev();

    void OnGoToLine();

protected slots:

    void marginClicked(int margin, int line, Qt::KeyboardModifiers state);

    void onTextChanged();

    // Override redo action:
    virtual void undo();

    // Context menu slots:
    void onDisableBreakpoint();
    void onEnableBreakpoint();
    void onDeleteBreakpoint();
    void onAboutToBreakpointsContextMenu();

    /// Update menu actions before the menu is displayed
    void onMenuAboutToShow();


protected:

    //! Re-implemented to handle mouse moves.
    // Do not commit!
    virtual void mouseMoveEvent(QMouseEvent* e);

    // Utilities:
    void enableExistingBreakpoint(int line, bool isEnabled);
    void initializeMarginContextMenu(int clickedLine, int lineMarkers);

    // acSourceCodeView overrides:
    virtual void displayMarginContextMenu(QPoint position);
    virtual void keyPressEvent(QKeyEvent* pKeyEvent);
    virtual void mouseDoubleClickEvent(QMouseEvent* pEvent);

    apBreakPoint* findBreakpointInLine(int line);

    // Utilities:
    void addCommandsToContextMenu(unsigned int contextMenuMask);

protected slots:

    void onAddWatch();
    void onAddMultiWatch();
    void onAboutToShowTextContextMenu();
    void onShowLineNumbers(bool show);
protected:

    // Application commands instance:
    afApplicationCommands* m_pApplicationCommands;

    // The current clicked line:
    int _clickedBreakpointLine;

    // Actions for context menu:
    QAction* _pAddWatchAction;
    QAction* _pAddMultiWatchAction;

    QPoint _currentMousePosition;

    QAction* _pShowLineNumbersAction;

    /// Contain true iff breakpoints should be enabled:
    bool m_enableBreakpoints;

    /// Contain the matching tree item data. If not nullptr, will be used for selecting the file matching item on edit:
    const afApplicationTreeItemData* m_pMatchingTreeItemData;
};


#endif  // __AFSOURCECODEVIEW__H
