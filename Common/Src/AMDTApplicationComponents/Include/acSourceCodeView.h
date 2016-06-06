//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acSourceCodeView.h
///
//==================================================================================

//------------------------------ acSourceCodeView.h ------------------------------

#ifndef __ACSOURCECODEVIEW__H
#define __ACSOURCECODEVIEW__H

// QScintilla:
#define QSCINTILLA_DLL
#include <Qsci/qsciscintilla.h>

// Qt:
#include <QMenu>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:                 AC_API acSourceCodeView: public QsciScintilla
// General Description: Generic source code view implementation. The class derive from
//                      QScintilla class, and provide syntax highlighting and more of
//                      QScintilla functionality
// Author:              Sigal Algranaty
// Creation Date:       3/8/2011
// ----------------------------------------------------------------------------------

class AC_API acISourceViewToolTip
{
public:
    /// Get the tooltip string based on the highlighted string in the source code
    virtual QString Tooltip(QString& highlightedString) = 0;
};

class AC_API acSourceCodeView: public QsciScintilla
{
    Q_OBJECT

public:

    enum ContextMenuItems
    {
        AC_ContextMenu_Language = 0x001,
        AC_ContextMenu_Cut = 0x002,
        AC_ContextMenu_Copy = 0x004,
        AC_ContectMenu_Paste = 0x008,
        AC_ContextMenu_SelectAll = 0x010,
        AC_ContextMenu_Edit = AC_ContextMenu_Copy | AC_ContectMenu_Paste | AC_ContextMenu_SelectAll,
        AC_ContextMenu_LineNumbers = 0x020,
        AC_ContextMenu_Watch = 0x040,
        AC_ContextMenu_Default = AC_ContextMenu_Language | AC_ContextMenu_Cut | AC_ContextMenu_Edit | AC_ContextMenu_LineNumbers,
        AC_ContextMenu_Debug = AC_ContextMenu_Default | AC_ContextMenu_Watch
    };


    enum acSourceMarkerType
    {
        AC_MARKER_ENABLED_BREAKPOINT,
        AC_MARKER_DISABLED_BREAKPOINT,
        AC_MARKER_PROGRAM_COUNTER
    };

    acSourceCodeView(QWidget* pParent, bool shouldShowLineNumbers, unsigned int contextMenuMask = (unsigned int)AC_ContextMenu_Default);
    virtual ~acSourceCodeView();

    virtual bool displayFile(const osFilePath& fileName, int lineNumber, int pcIndex);
    bool setProgramCounter(int lineNumber, int pcIndexd);
    virtual bool saveFile();
    virtual bool saveFileAs(gtString& newFileName);
    const osFilePath& filePath() const {return _filePath;}

    void showLineNumbers(bool show);

    /// Set the MDI details for this view:
    /// \param mdiFilePath the path related to this MDI
    void SetMDIFilePath(const osFilePath& mdiFilePath) { m_mdiFilePath = mdiFilePath; }

    /// Update the file from disk:
    void UpdateFile();

    /// Set the editor to be mono font
    void SetMonoFont(int fontSize);

    /// Is the file modified from last save:
    bool IsModified() const {return m_isModified;}

    /// Expands QsciScintilla::setCursorPosition() behaviour, sets cursor to line and index
    /// and verify cursor is positioned in the middle when possible
    void SetCursorPositionToMiddle(int line, int index);

    /// Set tooltip resolver
    void SetTooltipResolver(acISourceViewToolTip* pTooltipResolver);

signals:
    /// Document was saved signal
    void DocumentSaved(QString filePathAsStr);

public slots:

    void onSelectAll();
    void onCopy();
    void OnAboutToShowMenu();
    void onTooltipTimer();
protected:

    // Utilities:
    virtual void initContextMenus(unsigned int contextMenuMask = (unsigned int)AC_ContextMenu_Default);

    // QsciScintillaBase overrides:
    //! Re-implemented to handle the context menu.
    virtual void contextMenuEvent(QContextMenuEvent* e);
    virtual void displayMarginContextMenu(QPoint position) { (void)(position); };

    /// track the mouse move event
    void mouseMoveEvent(QMouseEvent* event);
    void leaveEvent(QEvent* leaveEvent);

protected slots:

    // Context menu slots:
    void onLanguageMenu();
    void onLanguageMenuClick();
    void onCut();
    void onPaste();

protected:

    // Define the index of supported markers:
    int _topFramePCMarkerIndex;
    int _framePCMarkerIndex;
    int _enabledBreakpointMarkerIndex;
    int _disabledBreakpointMarkerIndex;

    // Line numbers:
    bool _shouldShowLineNumbers;

    // Text context menu:
    QMenu* _pTextContextMenu;

    // Margin context menu:
    QMenu* _pMarginContextMenu;

    // Displayed file path:
    osFilePath _filePath;

    // True iff the text change event should be ignored:
    bool _ignoreTextChanged;

    // Index of current language
    int _languageIndexInHighLighter;

    // Default language extension
    const static gtString _defaultLanguageExtention;

    // Actions:
    QAction* m_pCutMenuAction;
    QAction* m_pCopyMenuAction;
    QAction* m_pPasteMenuAction;

    // The file path for this view if this is an MDI:
    osFilePath m_mdiFilePath;

    // Was text changed after last save?
    bool m_isModified;

    /// Timer for the tooltip
    QTimer m_tooltipTimer;

    /// last mouse position for the tooltip
    QPoint m_lastMousePos;

    /// Tooltip resolver
    acISourceViewToolTip* m_pTooltipResolver;
};

#endif  // __ACSOURCECODEVIEW__H
