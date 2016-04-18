//------------------------------ kaSourceCodeView.cpp ------------------------------

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// QScintilla:
#include <Qsci/qscilexercpp.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acSourceCodeLanguageHighlighter.h>
#include <AMDTApplicationComponents/Include/acSourceCodeDefinitions.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaSourceCodeView.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>

// --------------------------------------------------------------------------
kaSourceCodeView::kaSourceCodeView(QWidget* pParent, bool shouldShowLineNumbers, unsigned int contextMenuMask) : afSourceCodeView(pParent, shouldShowLineNumbers, contextMenuMask), m_platformIndicator(kaPlatformUnknown)
{
    m_enableBreakpoints = false;
}

// --------------------------------------------------------------------------
kaSourceCodeView::~kaSourceCodeView()
{

}

// --------------------------------------------------------------------------
void kaSourceCodeView::addMenuAction(QAction* pAction, bool inBeginning)
{
    GT_IF_WITH_ASSERT(NULL != _pTextContextMenu && NULL != pAction)
    {
        // Add the action. The connection the slot should be done by who ever called to add this action:
        if (inBeginning)
        {
            // add action first in menu
            QAction* pFirstAction = _pTextContextMenu->actions().at(0);
            GT_IF_WITH_ASSERT(NULL != pFirstAction)
            {
                _pTextContextMenu->insertAction(pFirstAction, pAction);
            }
        }
        else
        {
            // add action last last in menu
            _pTextContextMenu->addAction(pAction);
        }

        m_addedMenuActions << pAction;
    }
}

// --------------------------------------------------------------------------
void kaSourceCodeView::addSeparator(bool inBeginning)
{
    GT_IF_WITH_ASSERT(NULL != _pTextContextMenu)
    {
        QAction* pFirstAction = _pTextContextMenu->actions().at(0);

        if (inBeginning)
        {
            // add separator first in menu
            GT_IF_WITH_ASSERT(NULL != pFirstAction)
            {
                _pTextContextMenu->insertSeparator(pFirstAction);
            }
        }
        else
        {
            // add separator last in menu
            _pTextContextMenu->addSeparator();
        }
    }
}

// --------------------------------------------------------------------------
void kaSourceCodeView::updateView()
{
    gtString fileExtenstion;
    _filePath.getFileExtension(fileExtenstion);

    if (fileExtenstion == KA_STR_kernelViewISAExtension)
    {
        SetISAText(_filePath);
    }
    else
    {
        UpdateFile();
    }
}

// --------------------------------------------------------------------------
void kaSourceCodeView::SetInternalText(const QString& text)
{
    _ignoreTextChanged = true;

    setText(text);

    _ignoreTextChanged = false;
}

// --------------------------------------------------------------------------
void kaSourceCodeView::SetISAText(const osFilePath& isaFilePath)
{
    if ((!isaFilePath.isEmpty()) && (isaFilePath.exists()))
    {
        if (m_platformIndicator == kaPlatformOpenCL)
        {
            QString dsamText, scShaderText, csDataText;
            bool rc = kaApplicationCommands::instance().ParseISAFile(isaFilePath, dsamText, scShaderText, csDataText);
            GT_ASSERT(rc);

            // Notice, do not check the return value. If the function extracted the isa text this is enough. The return value is asserted, in case it fails:
            if (!dsamText.isEmpty())
            {
                // Set the cursor in the opened file:
                setCursorPosition(0, 0);

                setText(dsamText);

                // Find the lexer for the ISA:
                QsciLexer* pLexer = acSourceCodeLanguageHighlighter::instance().getLexerByExtension(KA_STR_kernelViewISAExtension);

                if (pLexer != NULL)
                {
                    // Set my lexer:
                    setLexer(pLexer);

                    setMarginsForegroundColor(AC_SOURCE_CODE_EDITOR_MARGIN_FORE_COLOR);
                    setMarginsBackgroundColor(AC_SOURCE_CODE_EDITOR_MARGIN_BG_COLOR);

                }
            }
        }
        else
        {
            displayFile(isaFilePath, 0, -1);
        }
    }
    else
    {
        // Set the ISA text:
        SetInternalText(KA_STR_ISANotAvailable);

        // Hide line numbers for non available ISA:
        showLineNumbers(false);
    }
}

// --------------------------------------------------------------------------
bool kaSourceCodeView::IsActionOfThisView(QAction* pAction) const
{
    bool ret = false;
    unsigned int count = m_addedMenuActions.count();

    // search for the action
    for (unsigned int i = 0; i < count; i++)
    {
        if (m_addedMenuActions[i] == pAction)
        {
            ret = true;
            break;
        }
    }

    return ret;
}

