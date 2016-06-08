//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acSourceCodeView.cpp
///
//==================================================================================

//------------------------------ acSourceCodeView.cpp ------------------------------

#include <math.h>

// Qt:
#include <QtWidgets>

// QScintilla:
#include <Qsci/qscilexercpp.h>
#include <Qsci/qsciabstractapis.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osFile.h>

// Local:
#include <inc/acStringConstants.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acSourceCodeDefinitions.h>
#include <AMDTApplicationComponents/Include/acSourceCodeView.h>
#include <AMDTApplicationComponents/Include/acSourceCodeLanguageHighlighter.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
const gtString acSourceCodeView::_defaultLanguageExtention(L"cpp");

#define AC_SOURCE_CODE_TOOLTIP_TIMER 300

// ---------------------------------------------------------------------------
// Name:        acSourceCodeView::acSourceCodeView
// Description: Constructor.
// Arguments:   parent - My parent Qt widget
// Author:      Sigal Algranaty
// Date:        2/8/2011
// ---------------------------------------------------------------------------
acSourceCodeView::acSourceCodeView(QWidget* pParent, bool shouldShowLineNumbers, unsigned int iContextMenuMask):
    QsciScintilla(pParent), _topFramePCMarkerIndex(-1), _framePCMarkerIndex(-1), _enabledBreakpointMarkerIndex(-1), _disabledBreakpointMarkerIndex(-1),
    _shouldShowLineNumbers(shouldShowLineNumbers), _pTextContextMenu(NULL), _pMarginContextMenu(NULL), _ignoreTextChanged(false), m_pCutMenuAction(NULL),
    m_pCopyMenuAction(NULL), m_pPasteMenuAction(NULL), m_isModified(false), m_pTooltipResolver(nullptr)
{
    // Initialize scintilla settings:
    setIndentationGuidesBackgroundColor(AC_SOURCE_CODE_EDITOR_GUIDES_BG_COLOR);
    setIndentationGuidesForegroundColor(AC_SOURCE_CODE_EDITOR_GUIDES_FORE_COLOR);
    setAutoIndent(true);
    setIndentationsUseTabs(true);
    setAutoCompletionSource(QsciScintilla::AcsAll);
    setAutoCompletionFillupsEnabled(true);
    setAutoCompletionThreshold(3);
    setWhitespaceVisibility(QsciScintilla::WsInvisible);
    setWhitespaceForegroundColor(AC_SOURCE_CODE_EDITOR_WHITE_SPACE_DEFAULT_COLOR);
    setIndentationsUseTabs(true);
    setMarginsForegroundColor(AC_SOURCE_CODE_EDITOR_MARGIN_FORE_COLOR);
    setMarginsBackgroundColor(AC_SOURCE_CODE_EDITOR_MARGIN_BG_COLOR);
    setFoldMarginColors(AC_SOURCE_CODE_EDITOR_MARGIN_FORE_COLOR, AC_SOURCE_CODE_EDITOR_MARGIN_BG_COLOR);
    setTabWidth(2);
    // In windows there is a bug in qscintilla that shows cr/lf as lf only and when entering newline it enters
    // cr cr/lf as cr/lf when setting the next line it solves the problem correctly for windows, and does not change behavior
    // on Linux
    setEolMode(EolUnix);

    // set default encoding to UTF8
    setUtf8(true);

    // Margin attributes:
    setMarginLineNumbers(1, false);
    setMarginType(1, QsciScintilla::SymbolMargin);
    unsigned int marginWidth = acScalePixelSizeToDisplayDPI(20);
    setMarginWidth(1, marginWidth);
    setMarginSensitivity(1, true);

    // Selection attributes:
    setSelectionBackgroundColor(AC_SOURCE_CODE_EDITOR_SELECTION_BG_COLOR);

    // Initialize the context menu:
    initContextMenus(iContextMenuMask);

    // Define the markers:
    const QImage& enabledBPMarkerImage = acGetIcon(AC_ICON_SOURCE_ENABLED_BREAKPOINT);
    _enabledBreakpointMarkerIndex = markerDefine(enabledBPMarkerImage);

    const QImage& disabledBPMarkerImage = acGetIcon(AC_ICON_SOURCE_DISABLED_BREAKPOINT);
    _disabledBreakpointMarkerIndex = markerDefine(disabledBPMarkerImage);

    const QImage& topFramePCMarkerImage = acGetIcon(AC_ICON_SOURCE_TOP_PROGRAM_COUNTER);
    _topFramePCMarkerIndex = markerDefine(topFramePCMarkerImage);

    const QImage& pcMarkerImage = acGetIcon(AC_ICON_SOURCE_PROGRAM_COUNTER);
    _framePCMarkerIndex = markerDefine(pcMarkerImage);

    // Show / Hide the line numbers:
    showLineNumbers(shouldShowLineNumbers);

    //Set default language
    _languageIndexInHighLighter = acSourceCodeLanguageHighlighter::instance().getLanguageIndexByExtension(_defaultLanguageExtention);

    // Connect the menu event
    bool rc = connect(_pTextContextMenu, SIGNAL(aboutToShow()), this, SLOT(OnAboutToShowMenu()));
    GT_ASSERT(rc);

    // Set the tooltip timer information
    m_tooltipTimer.setSingleShot(true);
    rc = connect(&m_tooltipTimer, SIGNAL(timeout()), this, SLOT(onTooltipTimer()));
    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        acSourceCodeView::~acSourceCodeView
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        2/8/2011
// ---------------------------------------------------------------------------
acSourceCodeView::~acSourceCodeView()
{
    if (_pTextContextMenu != NULL)
    {
        delete _pTextContextMenu;
    }

    if (_pMarginContextMenu != NULL)
    {
        delete _pMarginContextMenu;
    }
}

// ---------------------------------------------------------------------------
// Name:        acSourceCodeView::initContextMenus
// Description: Initialize the view context menus
// Author:      Sigal Algranaty
// Date:        9/8/2011
// ---------------------------------------------------------------------------
void acSourceCodeView::initContextMenus(unsigned int contextMenuMask)
{
    // Create a context menu:
    _pTextContextMenu = new QMenu(this);


    if ((contextMenuMask & AC_ContextMenu_Language) == AC_ContextMenu_Language)
    {
        // Define a highlight language sub menu:
        QMenu* pLanguageMenu = new QMenu(AC_STR_sourceCodeFileLanguage, _pTextContextMenu);


        // Connect the language action to the handler:
        bool rcConnect = connect(pLanguageMenu, SIGNAL(aboutToShow()), this, SLOT(onLanguageMenuClick()));
        GT_ASSERT(rcConnect);

        // Get the supported language vector:
        const gtVector<acSourceLanguage*>& languages = acSourceCodeLanguageHighlighter::instance().languages();

        for (int i = 0; i < (int)languages.size(); i++)
        {
            // Get the current language:
            acSourceLanguage* pLanguage = languages[i];
            GT_IF_WITH_ASSERT(pLanguage != NULL)
            {
                // Allocate a new language action:
                QAction* pNewLanguageAction = new QAction(pLanguageMenu);


                // Set the action text:
                pNewLanguageAction->setText(pLanguage->_languageName.asASCIICharArray());
                pNewLanguageAction->setCheckable(true);


                // Define a data for the action - the language index:
                QVariant actionData(i);
                pNewLanguageAction->setData(actionData);
                pLanguageMenu->addAction(pNewLanguageAction);

                // Connect the language action to the handler:
                rcConnect = pLanguageMenu->connect(pNewLanguageAction, SIGNAL(triggered()), this, SLOT(onLanguageMenu()));
                GT_ASSERT(rcConnect);
            }
        }

        // Add the language menu to the menu:
        _pTextContextMenu->addMenu(pLanguageMenu);
    }



    if ((contextMenuMask & AC_ContextMenu_Cut) == AC_ContextMenu_Cut)
    {
        // TO_DO: CodeXL frame string constants:
        QKeySequence shortcutCut(AC_STR_sourceCodeCutShortcut);
        m_pCutMenuAction = _pTextContextMenu->addAction(AC_STR_sourceCodeCut, this, SLOT(onCut()), shortcutCut);
    }

    if ((contextMenuMask & AC_ContextMenu_Copy) == AC_ContextMenu_Copy)
    {
        QKeySequence shortcutCopy(AC_STR_sourceCodeCopyShortcut);
        m_pCopyMenuAction = _pTextContextMenu->addAction(AC_STR_sourceCodeCopy, this, SLOT(onCopy()), shortcutCopy);
    }

    if ((contextMenuMask & AC_ContectMenu_Paste) == AC_ContectMenu_Paste)
    {
        QKeySequence shortcutPaste(AC_STR_sourceCodePasteShortcut);
        m_pPasteMenuAction = _pTextContextMenu->addAction(AC_STR_sourceCodePaste, this, SLOT(onPaste()), shortcutPaste);
    }

    if ((contextMenuMask & AC_ContextMenu_SelectAll) == AC_ContextMenu_SelectAll)
    {
        QKeySequence shortcutSelectAll(AC_STR_sourceCodeViewSelectAllShortcut);
        _pTextContextMenu->addAction(AC_STR_sourceCodeViewSelectAll, this, SLOT(onSelectAll()), shortcutSelectAll);
    }


}

// ---------------------------------------------------------------------------
// Name:        acSourceCodeView::displayFile
// Description: Display the requested file
// Arguments:   filePath - the file path
//              selectedLineNumber - the selected line number
//              pcIndex - Contain the index of the displayed source code in the call stack frame:
//                        (0 for top, 1 for others -1 for code not in debug mode):
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        3/8/2011
// ---------------------------------------------------------------------------
bool acSourceCodeView::displayFile(const osFilePath& filePath, int selectedLineNumber, int pcIndex)
{
    bool retVal = false;

    _ignoreTextChanged = true;

    if (_filePath != filePath)
    {
        // Clear the text:
        clear();

        // Set the file path:
        _filePath = filePath;

        // Get the file extension:
        gtString fileExtension;
        filePath.getFileExtension(fileExtension);
        fileExtension = fileExtension.toLowerCase();
        QString strExt = acGTStringToQString(fileExtension);
        int posOfValidExt = strExt.indexOf(QRegExp(AC_STR_FILE_EXTENSIONS_QREGEXP), 0);

        // if file extension is not a kernel/shader extension set it to .cl to enable syntax highlight
        if (-1 == posOfValidExt)
        {
            fileExtension = L"cl";
        }

        // Find the lexer according to the file type:
        QsciLexer* pLexer = acSourceCodeLanguageHighlighter::instance().getLexerByExtension(fileExtension);

        if (pLexer != NULL)
        {
            // Set my lexer:
            setLexer(pLexer);

            setMarginsForegroundColor(AC_SOURCE_CODE_EDITOR_MARGIN_FORE_COLOR);
            setMarginsBackgroundColor(AC_SOURCE_CODE_EDITOR_MARGIN_BG_COLOR);

        }

        // Read the file and display it:
        osFile file(filePath);
        unsigned long fileSize = 0;
        retVal = file.getSize(fileSize);

        // Make sure file size is smaller then 1M
        GT_IF_WITH_ASSERT(retVal && (fileSize < 1024 * 1024 * 10))
        {
            retVal = file.open(filePath, osFile::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_READ);

            GT_IF_WITH_ASSERT(retVal)
            {
                gtByte* pBuffer = new gtByte[fileSize + 1];


                // Read the file into the buffer:
                gtSize_t dataRead = 0;
                retVal = file.readAvailableData(pBuffer, fileSize, dataRead);

                GT_IF_WITH_ASSERT(retVal)
                {
                    // Add null at the end of the buffer making sure it is null terminated:
                    pBuffer[dataRead] = 0;

                    // Read the buffer into the source editor using the append API:
                    append(pBuffer);
                }

                // free the buffer:
                delete [] pBuffer;
            }

            file.close();
        }
    }

    // Set the cursor at the requested line number:
    if (selectedLineNumber >= 0)
    {
        if (pcIndex >= 0)
        {
            // Display the program counter:
            setProgramCounter(selectedLineNumber, pcIndex);
        }
        // If no program counter is displayed - select the line:
        else
        {
            // Get the selected line length:
            int selectedLineLength = lineLength(selectedLineNumber - 1);

            // Set the line selection:
            setSelection(selectedLineNumber - 1, 0, selectedLineNumber - 1, selectedLineLength);
        }
    }

    showLineNumbers(_shouldShowLineNumbers);

    _ignoreTextChanged = false;

    return retVal;
}

// ---------------------------------------------------------------------------
void acSourceCodeView::UpdateFile()
{
    // Read the file and display it:
    osFile file(_filePath);
    unsigned long fileSize = 0;
    bool retVal = file.getSize(fileSize);

    // Make sure file size is smaller then 1M
    GT_IF_WITH_ASSERT(retVal && (fileSize < 1024 * 1024 * 10))
    {
        retVal = file.open(_filePath, osFile::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_READ);

        GT_IF_WITH_ASSERT(retVal)
        {
            gtByte* pBuffer = new gtByte[fileSize + 1];


            // Read the file into the buffer:
            gtSize_t dataRead = 0;
            retVal = file.readAvailableData(pBuffer, fileSize, dataRead);

            GT_IF_WITH_ASSERT(retVal)
            {
                // Add null at the end of the buffer making sure it is null terminated:
                pBuffer[dataRead] = 0;

                // replace the text:
                blockSignals(true);
                setText(pBuffer);
                blockSignals(false);
            }

            // free the buffer:
            delete [] pBuffer;

            file.close();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acSourceCodeView::setProgramCounter
// Description: Set the program counter in the requested line
// Arguments:   int lineNumber - the program counter line
//              pcIndex - Contain the index of the displayed source code in the call stack frame:
//                        (0 for top, 1 for others -1 for code not in debug mode):
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/8/2011
// ---------------------------------------------------------------------------
bool acSourceCodeView::setProgramCounter(int lineNumber, int pcIndex)
{
    bool retVal = false;

    // Check what should be the program counter marker:
    int markerIndex = _framePCMarkerIndex;

    if (pcIndex == 0)
    {
        // Top frame:
        markerIndex = _topFramePCMarkerIndex;
    }

    // Delete all program counter markers:
    markerDeleteAll(_framePCMarkerIndex);
    markerDeleteAll(_topFramePCMarkerIndex);

    if ((lineNumber > 0) && (lineNumber <= lines()))
    {
        GT_IF_WITH_ASSERT(pcIndex >= 0)
        {
            // Add a program counter marker in the requested line:
            int markerLine = markerAdd(lineNumber - 1, markerIndex);

            if (markerLine >= 0)
            {
                retVal = true;
            }

            // Set the cursor position:
            setCursorPosition(lineNumber - 1, 0);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acSourceCodeView::contextMenuEvent
// Description: Overriding QScintilla base - popup a context menu
// Arguments:   QContextMenuEvent *pEvent
// Author:      Sigal Algranaty
// Date:        8/8/2011
// ---------------------------------------------------------------------------
void acSourceCodeView::contextMenuEvent(QContextMenuEvent* pEvent)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((_pTextContextMenu != NULL) && (pEvent != NULL))
    {
        // Check if the clicked position is in the margins:
        int marginsWidth = marginWidth(0) + marginWidth(1) + marginWidth(2);

        if (pEvent->pos().x() < marginsWidth)
        {
            displayMarginContextMenu(pEvent->pos());
        }
        else
        {
            _pTextContextMenu->exec(acMapToGlobal(this, pEvent->pos()));
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acSourceCodeView::onCut
// Description: Handling the cut command
// Author:      Sigal Algranaty
// Date:        9/8/2011
// ---------------------------------------------------------------------------
void acSourceCodeView::onCut()
{
    cut();
}

// ---------------------------------------------------------------------------
// Name:        acSourceCodeView::onCopy
// Description: Handling the copy command
// Author:      Sigal Algranaty
// Date:        9/8/2011
// ---------------------------------------------------------------------------
void acSourceCodeView::onCopy()
{
    copy();
}

// ---------------------------------------------------------------------------
// Name:        acSourceCodeView::onPaste
// Description: Handling the paste command
// Author:      Sigal Algranaty
// Date:        9/8/2011
// ---------------------------------------------------------------------------
void acSourceCodeView::onPaste()
{
    paste();
}

// ---------------------------------------------------------------------------
// Name:        acSourceCodeView::onPaste
// Description: Handling the paste command
// Author:      Dana Elifaz
// Date:        9/8/2011
// ---------------------------------------------------------------------------
void acSourceCodeView::onSelectAll()
{
    selectAll();
}

// ---------------------------------------------------------------------------
// Name:        acSourceCodeView::onLanguageMenu
// Description: Handle one of the on language menu commands
// Arguments:   pAction - the handler action
// Author:      Sigal Algranaty
// Date:        9/8/2011
// ---------------------------------------------------------------------------
void acSourceCodeView::onLanguageMenu()
{

    // Get the sender for this slot:
    QObject* pObject = sender();

    // Sanity check:
    GT_IF_WITH_ASSERT(pObject != NULL)
    {
        // Get the action:
        QAction* pAction = qobject_cast<QAction*>(pObject);

        // Sanity check:
        GT_IF_WITH_ASSERT(pAction != NULL)
        {
            QMenu* pLanguageMenu = qobject_cast<QMenu*>(pAction->parent());
            // Sanity check:
            GT_IF_WITH_ASSERT(pLanguageMenu != NULL)
            {
                QObjectList pLanguageList = pLanguageMenu->children();
                int noOfActions = pLanguageList.count();

                for (int i = 0; noOfActions > i; ++i)
                {
                    QAction* pLanguageAction = qobject_cast<QAction*>(pLanguageList.at(i));
                    GT_IF_WITH_ASSERT(pLanguageAction != NULL)
                    {
                        pLanguageAction->setChecked(false);
                    }
                }
            }

            pAction->setChecked(true);

            // Get the action data:
            QVariant dataLoc  = pAction->data();

            // Get the language index from the data:
            bool rc = false;
            int languageIndex = dataLoc.toInt(&rc);
            GT_IF_WITH_ASSERT(rc)
            {
                // Get the language object from the syntax highlighter object:
                _languageIndexInHighLighter = languageIndex;
                const acSourceLanguage* pLanguage = acSourceCodeLanguageHighlighter::instance().language(_languageIndexInHighLighter);
                GT_IF_WITH_ASSERT(pLanguage != NULL)
                {
                    GT_IF_WITH_ASSERT(pLanguage->_pLanguageLexer != NULL)
                    {
                        // Set the language:
                        setLexer(pLanguage->_pLanguageLexer);
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acSourceCodeView::onLanguageMenuClick
// Description: Handle the language menu show
// Author:      Bhattacharyya Koushik
// Date:        10/9/2012
// ---------------------------------------------------------------------------
void acSourceCodeView::onLanguageMenuClick()
{
    // Get the sender for this slot:
    QObject* pObject = sender();
    QMenu* pLanguageMenu = qobject_cast<QMenu*>(pObject);

    GT_IF_WITH_ASSERT(pLanguageMenu != NULL)
    {
        QList<QAction*> actionList = pLanguageMenu->actions();

        for (int i = 0; i < actionList.size() ; ++i)
        {
            // Get the action data:
            QVariant dataInner  = actionList.at(i)->data();
            bool rc = false;
            int languageIndex = dataInner.toInt(&rc);
            GT_IF_WITH_ASSERT(rc)
            {
                actionList.at(i)->setChecked((languageIndex == _languageIndexInHighLighter));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acSourceCodeView::saveFile
// Description: Save the current file
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/8/2011
// ---------------------------------------------------------------------------
bool acSourceCodeView::saveFile()
{
    bool retVal = false;

    // Check if the file is writable:
    QFileInfo fileInfo(QString(_filePath.asString().asASCIICharArray()));

    bool isWritable = fileInfo.isWritable();

    if (!isWritable)
    {
        gtASCIIString msg;
        msg.appendFormattedString(AC_STR_sourceCodeFileReadOnlyQuestion, _filePath.asString().asASCIICharArray());
        // Ask the user if he wants to overwrite write protection:
        int userAnswer = acMessageBox::instance().question(AC_STR_Question, msg.asCharArray(), QMessageBox::Yes | QMessageBox::No);

        if (userAnswer == QMessageBox::Yes)
        {
            // Overwrite the file write protection:
            isWritable = _filePath.makeWritable();
        }
    }

    if (isWritable)
    {
        // Write the text to the file:
        osFile file(_filePath);

        // Open the file for write:
        bool rcOpen = file.open(osFile::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
        GT_IF_WITH_ASSERT(rcOpen)
        {
            // Get the current text:
            QByteArray pData = text().toLatin1();

            // Write the data:
            file.write(pData.data(), pData.length());

            file.close();

            retVal = true;

            m_isModified = false;

            emit DocumentSaved(acGTStringToQString(_filePath.asString()));
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acSourceCodeView::saveFileAs
// Description: Save the current file
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        17/8/2011
// ---------------------------------------------------------------------------
bool acSourceCodeView::saveFileAs(gtString& newFileName)
{
    bool retVal = false;

    QString fileName = QString::fromWCharArray(newFileName.asCharArray());

    if (!fileName.isEmpty())
    {
        // Save the file:
        osFile file(newFileName);

        // Open the file for write:
        bool rcOpen = file.open(osFile::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
        GT_IF_WITH_ASSERT(rcOpen)
        {
            // Get the current text:
            QByteArray pData = text().toLatin1();

            // Write the data:
            file.write(pData.data(), pData.length());

            file.close();

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acSourceCodeView::showLineNumbers
// Description: Show / Hide the line numbers. Also adapt the margin width to
//              the file line number
// Arguments:   bool show
// Author:      Sigal Algranaty
// Date:        4/3/2012
// ---------------------------------------------------------------------------
void acSourceCodeView::showLineNumbers(bool show)
{
    _shouldShowLineNumbers = show;
    setMarginLineNumbers(0, show);

    if (show)
    {
        // Set the margin type:
        setMarginType(0, QsciScintilla::NumberMargin);

        // Check the line numbers for this file:
        int fileLength = lines();

        // Get the amount of digits in line number:
        int amountOfDigits = log10((float)fileLength) + 1;
        gtASCIIString marginWidthString = "88";

        if (amountOfDigits == 2)
        {
            marginWidthString = "888";
        }

        if (amountOfDigits == 3)
        {
            marginWidthString = "8888";
        }

        if (amountOfDigits == 4)
        {
            marginWidthString = "88888";
        }

        if (amountOfDigits == 5)
        {
            marginWidthString = "888888";
        }

        if (amountOfDigits == 6)
        {
            marginWidthString = "8888888";
        }

        if (amountOfDigits == 7)
        {
            marginWidthString = "88888888";
        }

        setMarginWidth(0, marginWidthString.asCharArray());
        setMarginSensitivity(0, true);
    }
    else
    {
        setMarginWidth(0, "");
    }
}

// ---------------------------------------------------------------------------
void acSourceCodeView::OnAboutToShowMenu()
{
    bool isTextEmpty = !selectedText().isEmpty();

    if (m_pCopyMenuAction != NULL)
    {
        m_pCopyMenuAction->setEnabled(isTextEmpty);
    }

    if (m_pCutMenuAction != NULL)
    {
        m_pCutMenuAction->setEnabled(isTextEmpty && !isReadOnly());
    }

    if (m_pPasteMenuAction != NULL)
    {
        m_pPasteMenuAction->setEnabled(!isReadOnly());
    }
}

// ---------------------------------------------------------------------------
void acSourceCodeView::SetMonoFont(int fontSize)
{
    QFont monoSpaceFont(AC_SOURCE_CODE_EDITOR_DEFAULT_FONT_FAMILY);
    monoSpaceFont.setStyleHint(QFont::Monospace);
    monoSpaceFont.setPointSize(fontSize);

    setFont(monoSpaceFont);
}

// ---------------------------------------------------------------------------
void acSourceCodeView::SetCursorPositionToMiddle(int line, int index)
{
    int firstLine = firstVisibleLine();
    int lastLine = -1;

    if (line > 0 && line < lines())
    {
        // calculate last line on display
        // creating a QPoint thats at the end of the display and use it to
        // capture last line on display
        int marginW = marginWidth(0) + marginWidth(1) + marginWidth(2) + 1;
        QPoint pos(marginW, height());

        int maxIterations = 10;
        int iteration = 0;

        while (lastLine < 0 && iteration < maxIterations && pos.y() > 0)
        {
            lastLine = lineAt(pos);
            pos.setY(pos.y() - 5);
        }

        // calculate middle line index, lets say midLine is 3 then the third line visible would mark the middle of the display
        int midLine = (lastLine - firstLine) / 2;

        if (lastLine > 0 && lastLine > line)
        {
            int newFirstLine = (line > midLine) ? (line - midLine) : 0;
            // by setting what should be the first line - control the cursor position
            setFirstVisibleLine(newFirstLine);
        }

        setCursorPosition(line, index);
    }
}

// ---------------------------------------------------------------------------
void acSourceCodeView::SetTooltipResolver(acISourceViewToolTip* pTooltipResolver)
{
    GT_ASSERT(nullptr != pTooltipResolver)
    {
        GT_ASSERT(nullptr == m_pTooltipResolver || m_pTooltipResolver == pTooltipResolver)
        {
            m_pTooltipResolver = pTooltipResolver;
        }
    }
}

// ---------------------------------------------------------------------------
void acSourceCodeView::mouseMoveEvent(QMouseEvent* event)
{
    if (nullptr != event)
    {
        m_lastMousePos = event->pos();

        m_tooltipTimer.start(AC_SOURCE_CODE_TOOLTIP_TIMER);
        // Get the text position under the text
        QsciScintilla::mouseMoveEvent(event);
    }
}

// ---------------------------------------------------------------------------
void acSourceCodeView::leaveEvent(QEvent* leaveEvent)
{
    GT_UNREFERENCED_PARAMETER(leaveEvent);
    m_tooltipTimer.stop();
}

// ---------------------------------------------------------------------------
void acSourceCodeView::onTooltipTimer()
{
    long closePos = SendScintilla(SCI_POSITIONFROMPOINTCLOSE, m_lastMousePos.x(), m_lastMousePos.y());
    int textType = SendScintilla(SCI_GETSTYLEAT, closePos);

    if (textType == QsciLexerCPP::Identifier)
    {
        QString wordToQuery = wordAtPoint(m_lastMousePos);

        if (!wordToQuery.isEmpty())
        {
            if (nullptr != m_pTooltipResolver)
            {
                QString tooltipToShow = m_pTooltipResolver->Tooltip(wordToQuery);

                if (!tooltipToShow.isEmpty())
                {
                    QPoint screenPos = acMapToGlobal(this, m_lastMousePos);
                    QToolTip::showText(screenPos, tooltipToShow);
                }
            }
        }
    }
}
