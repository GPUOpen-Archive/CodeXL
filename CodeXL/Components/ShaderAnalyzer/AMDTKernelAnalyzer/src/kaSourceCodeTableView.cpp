//------------------------------ kaSourceCodeView.cpp ------------------------------

// Qt:
#include <QtWidgets>
#include <QWebView>

#include <iomanip>

// Infra:
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acFindWidget.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acQHTMLWindow.h>
#include <AMDTApplicationComponents/Include/acSourceCodeDefinitions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

// Backend:
#include <AMDTBackEnd/Emulator/Parser/ISAParser.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaSourceCodeTableView.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>


const QStringList s_isaTableHeaders { "&nbsp;Address", "Opcode", "Operands", "Cycles", "&nbsp;Functional Unit", "Hex" };


#define KA_CYCLE_SCALE_VALUE_STEPS 7 // 1, 2, 4, 8, 16, 32, 64


// --------------------------------------------------------------------------
kaSourceCodeTableView::kaSourceCodeTableView(QWidget* pParent) : m_platformIndicator(kaPlatformUnknown), m_pSourceTableViewHtml(nullptr), m_pContextMenu(nullptr), m_isDirty(false)
{
    setParent(pParent);
    QHBoxLayout* pLayout = new QHBoxLayout;
    pLayout->setContentsMargins(0, 0, 0, 0);
    QFrame* pISATableFrame = new QFrame;
    pISATableFrame->setStyleSheet(QString(KA_STR_ISATABLE_FRAME_STYLE));
    QHBoxLayout* pFrameLayout = new QHBoxLayout;
    m_pSourceTableViewHtml = new QWebView(nullptr);
    m_pSourceTableViewHtml->page()->setLinkDelegationPolicy(QWebPage::DontDelegateLinks);
    pFrameLayout->addWidget(m_pSourceTableViewHtml);
    m_pSourceTableViewHtml->setCursor(QCursor(Qt::ArrowCursor));
    pISATableFrame->setLayout(pFrameLayout);
    pLayout->addWidget(pISATableFrame);
    setLayout(pLayout);

    // Set the zoom factor to fit the resolution. QWebView assumes 96 DPI.
    QWidget* pScreen = QApplication::desktop()->screen();

    if (pScreen != nullptr)
    {
        const int horizontalDpi = pScreen->logicalDpiX();
        m_pSourceTableViewHtml->setZoomFactor(horizontalDpi / 96.0);
    }

    // Context menu:
    m_pSourceTableViewHtml->setContextMenuPolicy(Qt::CustomContextMenu);
    m_pContextMenu = new QMenu;

    m_addedMenuActions.push_back(m_pContextMenu->addAction(AF_STR_CopyA, this, SLOT(OnCopy())));
    m_addedMenuActions.push_back(m_pContextMenu->addAction(AF_STR_SelectAllA, this, SLOT(OnSelectAll())));

    // [sALU], [sMEM]
    m_colorStrings[Instruction::ScalarMemoryRead]   = acGetAMDColorScaleColor(AC_AMD_GREEN, 0).name(QColor::HexRgb);
    m_colorStrings[Instruction::ScalarMemoryWrite]  = acGetAMDColorScaleColor(AC_AMD_GREEN, 0).name(QColor::HexRgb);
    m_colorStrings[Instruction::ScalarALU]          = acGetAMDColorScaleColor(AC_AMD_GREEN, 0).name(QColor::HexRgb);

    // [vMEM]
    m_colorStrings[Instruction::VectorMemoryRead]   = acGetAMDColorScaleColor(AC_AMD_RED, 0).name(QColor::HexRgb);
    m_colorStrings[Instruction::VectorMemoryWrite]  = acGetAMDColorScaleColor(AC_AMD_RED, 0).name(QColor::HexRgb);

    // [vALU]
    m_colorStrings[Instruction::VectorALU]          = acGetAMDColorScaleColor(AC_AMD_CYAN, 2).name(QColor::HexRgb);

    // [LDS]
    m_colorStrings[Instruction::LDS]                = acGetAMDColorScaleColor(AC_AMD_PURPLE, 0).name(QColor::HexRgb);

    // [GDS], [Export]
    m_colorStrings[Instruction::GDS]                = acGetCodeXLColorScaleColor(AC_CODEXL_MAGENTA, 0).name(QColor::HexRgb);
    m_colorStrings[Instruction::Export]             = acGetCodeXLColorScaleColor(AC_CODEXL_MAGENTA, 0).name(QColor::HexRgb);

    // [Internal] (regarded as "Flow Control")
    m_colorStrings[Instruction::Internal]           = acGetAMDColorScaleColor(AC_AMD_ORANGE, 0).name(QColor::HexRgb);

    // [Branch]
    m_colorStrings[Instruction::Branch]             = acGetAMDColorScaleColor(AC_AMD_ORANGE, 2).name(QColor::HexRgb);

    // Currently unclear.
    m_colorStrings[Instruction::Atomics] = acGetCodeXLColorScaleColor(AC_CODEXL_MAGENTA, 2).name(QColor::HexRgb);
    m_defaultColorString = acQAMD_GRAY_LIGHT_COLOUR.name(QColor::HexRgb);


    bool rc = connect(m_pSourceTableViewHtml, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnContextMenuEvent(const QPoint&)));
    GT_ASSERT(rc);

    // Connect the menu event
    rc = connect(m_pContextMenu, SIGNAL(aboutToShow()), this, SLOT(OnAboutToShowMenu()));
    GT_ASSERT(rc);
}

// --------------------------------------------------------------------------
kaSourceCodeTableView::~kaSourceCodeTableView()
{
}

// --------------------------------------------------------------------------
inline const QString& kaSourceCodeTableView::GetColorStringByInstructionCategory(Instruction::InstructionCategory instructionType)
{
    if ((int)instructionType < Instruction::InstructionsCategoriesCount)
    {
        return m_colorStrings[instructionType];
    }
    else
    {
        return m_defaultColorString;
    }
}

// --------------------------------------------------------------------------
void kaSourceCodeTableView::BuildOpcodeString(const std::string& opcode, const QString& colorAsHex, bool isBranch, QString& opcodeString) const
{
    if (isBranch)
    {
        opcodeString = QString("<b>" KA_Str_HTML_ColoredOpcode "</b>").arg(colorAsHex).arg(opcode.c_str());
    }
    else
    {
        opcodeString = QString(KA_Str_HTML_ColoredOpcode).arg(colorAsHex).arg(opcode.c_str());
    }
}

// --------------------------------------------------------------------------
inline const QColor& kaSourceCodeTableView::GetColorStringDependingOnCycles(int cycleCount)
{
    const QColor* retColor = &acQAMD_GRAY3_COLOUR;

    int valueSteps[KA_CYCLE_SCALE_VALUE_STEPS] = { 1, 2, 4, 8, 16, 32, 64 };
    int colorSteps[KA_CYCLE_SCALE_VALUE_STEPS] = { 0, 11, 22, 33, 77, 89, 100 };

    GT_ASSERT(0 <= cycleCount);

    if (0 < cycleCount)
    {
        int index = 0;

        for (int i = KA_CYCLE_SCALE_VALUE_STEPS - 2; 0 <= i; --i)
        {
            if (cycleCount > valueSteps[i])
            {
                index = i + 1;
                break;
            }
        }

        retColor = &acGetWarningScaleColor(colorSteps[index]);
    }

    return *retColor;
}

// --------------------------------------------------------------------------
QString kaSourceCodeTableView::GetSyntaxHighligtedOperandsString(const std::string& operands)
{
    QString opers = operands.c_str();

    static bool s_firstTime = true;
    static QString s_operandFontTag;
    static QString s_labelFontTag;
    static const QString s_closeFontTag = "</font>";

    if (s_firstTime)
    {
        s_operandFontTag = "<font color='";
        s_operandFontTag += acQAMD_PURPLE_PRIMARY_COLOUR.name(QColor::HexRgb);
        s_operandFontTag += "'>";

        s_labelFontTag = "<font style='background-color:";
        s_labelFontTag += acQYELLOW_WARNING_COLOUR.name(QColor::HexRgb);
        s_labelFontTag += ";' color='";
        s_labelFontTag += acGetCodeXLColorScaleColor(AC_CODEXL_BLUE, 1).name(QColor::HexRgb);
        s_labelFontTag += "'>";

        s_firstTime = false;
    }

    QStringList operandsList = opers.split(",");
    QString operandPart;
    QString labelText;

    //int count = 0;
    for (auto it : operandsList)
    {
        int hexPos = it.indexOf(QRegularExpression(KA_ISA_TABLE_HEXPOS_REGEX), 0);
        int twoTwoDigitsPos = it.indexOf(QRegularExpression(KA_ISA_TABLE_22DIGITSPOS_REGEX), 0);
        int oneOneDigitsPos = it.indexOf(QRegularExpression(KA_ISA_TABLE_11DIGITSPOS_REGEX), 0);
        int oneTwoDigitsPos = it.indexOf(QRegularExpression(KA_ISA_TABLE_12DIGITSPOS_REGEX), 0);

        bool isLabel = it.contains("label");

        if (isLabel)
        {
            operandPart.append(s_labelFontTag);
            operandPart.append(it);
            operandPart.append(s_closeFontTag);
        }
        else if (-1 != hexPos)
        {
            operandPart.append(s_operandFontTag);
            operandPart.append(it);
            operandPart.append(s_closeFontTag);
        }
        else if (-1 != twoTwoDigitsPos)
        {
            operandPart.append(it.mid(0, twoTwoDigitsPos));
            operandPart.append(s_operandFontTag);
            operandPart.append(it.mid(twoTwoDigitsPos, 2));
            operandPart.append(s_closeFontTag);
            operandPart.append(it.mid(twoTwoDigitsPos + 2, 1));
            operandPart.append(s_operandFontTag);
            operandPart.append(it.mid(twoTwoDigitsPos + 3, 2));
            operandPart.append(s_closeFontTag);
            operandPart.append(it.mid(twoTwoDigitsPos + 5));
        }
        else if (-1 != oneOneDigitsPos && -1 == oneTwoDigitsPos)
        {
            operandPart.append(it.mid(0, oneOneDigitsPos));
            operandPart.append(s_operandFontTag);
            operandPart.append(it.mid(oneOneDigitsPos, 1));
            operandPart.append(s_closeFontTag);
            operandPart.append(it.mid(oneOneDigitsPos + 1, 1));
            operandPart.append(s_operandFontTag);
            operandPart.append(it.mid(oneOneDigitsPos + 2, 1));
            operandPart.append(s_closeFontTag);
            operandPart.append(it.mid(oneOneDigitsPos + 3));
        }
        else if (-1 != oneTwoDigitsPos)
        {
            operandPart.append(it.mid(0, oneTwoDigitsPos));
            operandPart.append(s_operandFontTag);
            operandPart.append(it.mid(oneTwoDigitsPos, 1));
            operandPart.append(s_closeFontTag);
            operandPart.append(it.mid(oneTwoDigitsPos + 1, 1));
            operandPart.append(s_operandFontTag);
            operandPart.append(it.mid(oneTwoDigitsPos + 2, 2));
            operandPart.append(s_closeFontTag);
            operandPart.append(it.mid(oneTwoDigitsPos + 4));
        }
        else
        {
            operandPart.append(it);
        }
    }

    labelText += operandPart;
    return labelText;
}

// --------------------------------------------------------------------------
void kaSourceCodeTableView::UpdateView()
{
    if (m_isDirty)
    {
        gtString fileExtenstion;
        m_filePath.getFileExtension(fileExtenstion);

        if (fileExtenstion == KA_STR_kernelViewISAExtension)
        {
            SetISAText(m_filePath);
            m_isDirty = false;
        }
    }
}

// --------------------------------------------------------------------------
void kaSourceCodeTableView::SetISAText(const osFilePath& isaFilePath)
{
    if ((!isaFilePath.isEmpty()) && (isaFilePath.exists()))
    {
        gtString fileExtenstion;
        isaFilePath.getFileExtension(fileExtenstion);

        if (fileExtenstion == KA_STR_kernelViewISAExtension)
        {
            m_filePath = isaFilePath;
            DisplayISAFile(isaFilePath);
        }
    }
}

bool  kaSourceCodeTableView::DisplayISAFile(const osFilePath& filePath)
{
    bool retVal = false;

    // Extract the device name.
    gtString fileName;
    retVal = filePath.getFileName(fileName);

    if (retVal)
    {
        const gtString FN_DEVICE_DELIMITER = L"_";
        size_t delimPos = fileName.find(FN_DEVICE_DELIMITER);
        gtString deviceName;

        if (delimPos > 1)
        {
            fileName.getSubString(0, delimPos - 1, deviceName);
            m_deviceName = deviceName.asASCIICharArray();
        }

        // Read the file.
        std::string strFilePath(filePath.asString().asASCIICharArray());
        std::string strISAFileContent;
        std::ifstream file(strFilePath);
        strISAFileContent = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        ParserISA parserISA;
        retVal = parserISA.Parse(strISAFileContent);

        if (retVal)
        {
            const std::vector<Instruction*> isaInstructions = parserISA.GetInstructions();

            // Read the buffer into the source table:
            if (!isaInstructions.empty())
            {
                m_isaInstructions = isaInstructions;
                FillSourceTableView(isaInstructions);
            }
        }
    }

    return retVal;
}

void kaSourceCodeTableView::FillSourceTableView(const std::vector<Instruction*>& isaInstructions)
{
    // parse pBuffer
    if (!isaInstructions.empty())
    {
        GT_IF_WITH_ASSERT(m_pSourceTableViewHtml)
        {
            // Reserve 5 * file size for the HTML text size. We expect the HTML text to be 5 times bigger then the text size
            QFile isaFile(acGTStringToQString(m_filePath.asString()));
            quint64 isaSize = isaFile.size();
            m_htmlText.reserve(isaSize * 5);

            // This string is used as the general style sheet for the generated HTML
            static QString style = QString(KA_STR_ISATABLE_STYLE).arg(AC_SOURCE_CODE_EDITOR_DEFAULT_FONT_FAMILY);

            // Initialize the HTML text. Open an html, body and table tags. These tags are closed at the end of this function
            m_htmlText = QString(KA_STR_ISATABLE_OPENTAGS).arg(style);

            // Get the tooltip strings
            QStringList columnTooltipsList = QString(KA_STR_SourceTableViewColumnsTooltips).split("|");

            // Add table headers
            for (int i = 0; i < s_isaTableHeaders.size(); i++)
            {
                QString tooltip;
                GT_IF_WITH_ASSERT(columnTooltipsList.size() == s_isaTableHeaders.size())
                {
                    tooltip = columnTooltipsList[i];
                }

                if (s_isaTableHeaders[i] == "Cycles")
                {
                    m_htmlText.append(QString("<th title='%1' style='text-align:center;'>%2</th>").arg(tooltip).arg(s_isaTableHeaders[i]));
                }
                else
                {
                    m_htmlText.append(QString("<th nowrap title='%1'>%2</th>").arg(tooltip).arg(s_isaTableHeaders[i]));
                }
            }

            m_htmlText.append("</tr>");

            // Set the progress bar
            afProgressBarWrapper::instance().ShowProgressDialog(L"Initializing the ISA table", isaInstructions.size(), 100);

            m_instructionsAsCSV.append(CSV_HEADER);
            std::string emptyString;

            for (Instruction* pInstruction : isaInstructions)
            {
                if (pInstruction != nullptr)
                {
                    pInstruction->GetCSVString(m_deviceName.toStdString(), emptyString);
                    m_instructionsAsCSV.append(emptyString.c_str());
                    // Add the current instruction
                    AddInstruction(pInstruction);
                    afProgressBarWrapper::instance().incrementProgressBar();
                }
            }

            m_htmlText.append(KA_STR_ISATABLE_CLOSETAGS);

            // Build the file path
            // We load an HTML file to the web view. When the text was loaded directly, for some reason, the
            // inner link did not work. When a file was loaded it worked.
            osFilePath htmlFilePath = m_filePath;
            htmlFilePath.setFileExtension(KA_STR_htmlExtension);

            gtString htmlFileName;
            htmlFilePath.getFileName(htmlFileName);
            htmlFileName.prepend(L".");
            htmlFilePath.setFileName(htmlFileName);
            osFile htmlOsFile;
            bool retVal = htmlOsFile.open(htmlFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

            if (retVal)
            {
                htmlOsFile << acQStringToGTString(m_htmlText);
                htmlOsFile.close();
            }

            QString qfilePathStr = acGTStringToQString(htmlFilePath.asString());

            // Display the ISA html file in the web view:
            QUrl url = QUrl::fromLocalFile(qfilePathStr);
            m_pSourceTableViewHtml->setUrl(url);
            osFilePath cssFilePath = m_filePath;
            cssFilePath.setFileExtension(KA_STR_cssExtension);

            gtString csslFileName;
            cssFilePath.getFileName(csslFileName);
            csslFileName.prepend(L".");
            cssFilePath.setFileName(csslFileName);

            osFile cssOsFile;
            retVal = cssOsFile.open(cssFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

            if (retVal)
            {
                cssOsFile << KA_STR_ISAVIEW_CURSOR_cssText;
                cssOsFile.close();
            }

            QString cssPathStr = acGTStringToQString(cssFilePath.asString());

            m_pSourceTableViewHtml->page()->settings()->setUserStyleSheetUrl(QUrl::fromLocalFile(cssPathStr));

            // set hidden attribute on Windows - these files begin with dot which makes them hidden on Linux
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            GT_ASSERT(htmlOsFile.hide());
            GT_ASSERT(cssOsFile.hide());
#endif

            afProgressBarWrapper::instance().hideProgressBar();

            m_pSourceTableViewHtml->updateGeometry();
        }
    }
}

void kaSourceCodeTableView::AddSeparator(bool first)
{
    GT_IF_WITH_ASSERT(nullptr != m_pContextMenu)
    {
        QAction* pFirstAction = m_pContextMenu->actions().at(0);

        if (first)
        {
            // add separator first in menu
            GT_IF_WITH_ASSERT(nullptr != pFirstAction)
            {
                m_pContextMenu->insertSeparator(pFirstAction);
            }
        }
        else
        {
            // add separator last in menu
            m_pContextMenu->addSeparator();
        }
    }
}

void kaSourceCodeTableView::AddMenuAction(QAction* pAction, bool first)
{
    if (m_pContextMenu != nullptr)
    {
        if (first)
        {
            QAction* pFirst = m_pContextMenu->actions().isEmpty() ? nullptr : m_pContextMenu->actions().first();
            m_pContextMenu->insertAction(pFirst, pAction);
        }
        else
        {
            m_pContextMenu->addAction(pAction);
        }

        m_addedMenuActions.push_back(pAction);
    }
}

bool kaSourceCodeTableView::HasSelectedItems() const
{
    bool retVal = false;

    if (m_pSourceTableViewHtml != nullptr)
    {
        retVal = !m_pSourceTableViewHtml->page()->selectedText().isEmpty();
    }

    return retVal;
}

bool kaSourceCodeTableView::ContainsData() const
{
    bool retVal = !m_htmlText.isEmpty();
    return retVal;
}

void kaSourceCodeTableView::OnCopy()
{
    if (m_pSourceTableViewHtml != nullptr)
    {
        m_pSourceTableViewHtml->page()->triggerAction(QWebPage::Copy);
    }
}

void kaSourceCodeTableView::OnSelectAll()
{
    if (m_pSourceTableViewHtml != nullptr)
    {
        m_pSourceTableViewHtml->page()->triggerAction(QWebPage::SelectAll);
    }
}

void kaSourceCodeTableView::OnFindClick()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSourceTableViewHtml != nullptr)
    {

        if (!acFindParameters::Instance().m_findExpr.isEmpty())
        {
            // Define the Qt find flags matching the user selection:
            QWebPage::FindFlags findFlags = 0;

            if (acFindParameters::Instance().m_isCaseSensitive)
            {
                findFlags = findFlags | QWebPage::FindCaseSensitively;
            }

            // Check if there is another appearance of the text:
            acFindParameters::Instance().m_lastResult = m_pSourceTableViewHtml->page()->findText(acFindParameters::Instance().m_findExpr, findFlags);
        }

        // After results are updated, ask the find widget to update the UI:
        acFindWidget::Instance().UpdateUI();
    }
}

void kaSourceCodeTableView::OnFindNext()
{
    OnFindClick();
}

bool kaSourceCodeTableView::IsTableInFocus() const
{
    bool retVal = false;

    if (m_pSourceTableViewHtml != nullptr)
    {
        retVal = m_pSourceTableViewHtml->hasFocus();
    }

    return retVal;
}

void kaSourceCodeTableView::AddInstruction(const Instruction* pInstruction)
{
    // Parse line by line filling insert data
    if (pInstruction != nullptr)
    {

        if (pInstruction->GetLabel() == NO_LABEL)
        {
            AddInstructionRow(pInstruction);
        }
        else
        {
            AddLabelRow(pInstruction);
        }
    }
}

void kaSourceCodeTableView::AddLabelRow(const Instruction* pInstruction)
{
    GT_IF_WITH_ASSERT(pInstruction != nullptr)
    {
        // An instruction with label:
        std::string labelString = pInstruction->GetPointingLabelString();
        QString coloredLabel = GetSyntaxHighligtedOperandsString(labelString);
        QString qlabelString = labelString.c_str();
        qlabelString.replace(":", "");
        m_htmlText.append(QString("<tr><td><a name=\"%1\">%2</a></td></tr>").arg(qlabelString).arg(coloredLabel));
    }
}

void kaSourceCodeTableView::AddInstructionRow(const Instruction* pInstruction)
{
    GT_IF_WITH_ASSERT(pInstruction != nullptr)
    {
        m_htmlText.append("<tr>\n");

        // An instruction with no label:
        Instruction::InstructionCategory category = pInstruction->GetInstructionCategory();
        std::string opcode = pInstruction->GetInstructionOpCode();
        bool isBranch = (pInstruction->GetInstructionCategory() == Instruction::InstructionCategory::Branch);
        QString fullOffset = pInstruction->GetInstructionOffset().c_str();

        int offsetSize = fullOffset.size();
        int stringOffset = offsetSize >= 6 ? offsetSize - 6 : 0;

        QString relevantDigits = fullOffset.mid(stringOffset, 6);
        QString address = QString("0x%1").arg(relevantDigits);

        // Add the address to the table with 2 spaces, so that the labels are left aligned:
        m_htmlText.append(QString("<td>&nbsp;%1</td>\n").arg(address));

        const QString& instColorString = GetColorStringByInstructionCategory(category);
        QString opcodeLabelText;
        BuildOpcodeString(opcode, instColorString, isBranch, opcodeLabelText);

        // Set the instruction type text with coloring:
        QString currentHTMLTDText;

        if (isBranch)
        {
            currentHTMLTDText = QString(KA_Str_HTML_CellFGColorBold).arg(instColorString).arg(opcodeLabelText);
        }
        else
        {
            currentHTMLTDText = QString(KA_Str_HTML_CellFGColor).arg(instColorString).arg(opcodeLabelText);
        }

        m_htmlText.append(currentHTMLTDText);

        std::string operands = pInstruction->GetInstructionParameters();
        QString operandsLabelText = GetSyntaxHighligtedOperandsString(operands);

        // NOTICE: This line break is added because we want to shorten this column:
        operandsLabelText.replace("0 offen", "<br> 0 offen");

        if (operandsLabelText.contains(KA_STR_LabelIndicator))
        {
            QString linkLabel = QString("<a href='#%1'>%1</a>").arg(operands.c_str());
            QString linkTooltip = QString("Jump to %1").arg(operands.c_str());
            m_htmlText.append(QString(KA_Str_HTML_CellWithTooltip).arg(linkTooltip).arg(linkLabel));
        }
        else
        {
            m_htmlText.append(QString(KA_Str_HTML_Cell).arg(operandsLabelText));
        }

        // Get the number of cycle that this instruction costs.
        int cycleCount = pInstruction->GetInstructionClockCount(m_deviceName.toStdString());
        QString cyclesStr = (cycleCount > 0) ? QString::number(cycleCount) : (isBranch ? "4/16" : KA_STR_NA_VALUE);
        int usedCycleCount = (isBranch && (0 == cycleCount)) ? 8 : cycleCount;
        const QColor& cyclesColor = GetColorStringDependingOnCycles(usedCycleCount);
        QString tooltip = isBranch ? KA_STR_BranchInstructionTooltip : "";
        m_htmlText.append(QString("<td title ='%1' style='background-color:%2;' align=center>&nbsp;%3&nbsp;</td>\n").arg(tooltip).arg(cyclesColor.name(QColor::HexRgb)).arg(cyclesStr));

        // Add the instruction type column:
        QString strInstructionCategory = Instruction::GetFunctionalUnitAsString(category).c_str();
        m_htmlText.append(QString(KA_Str_HTML_CellFGColorNoWrap).arg(instColorString).arg(strInstructionCategory));

        // Get instruction binary representation, and add it to the table:
        QString instructionHex = pInstruction->GetInstructionBinaryRep().c_str();
        m_htmlText.append(QString(KA_Str_HTML_Cell).arg(instructionHex));

        m_htmlText.append("</tr>\n");

    }
}

void kaSourceCodeTableView::OnContextMenuEvent(const QPoint& pos)
{
    if (m_pContextMenu != nullptr)
    {
        m_pContextMenu->exec(acMapToGlobal(this, pos));
    }
}

void kaSourceCodeTableView::UseColorFormatting(bool show)
{
    GT_UNREFERENCED_PARAMETER(show);
}


// ---------------------------------------------------------------------------
// Name:        kaSourceCodeTableView::SaveFileAsCSV
// Description: Save comma separated instructions elements into the given file
// Author:      Yuri Rshtunique
// Date:        16/8/2015
// ---------------------------------------------------------------------------
void kaSourceCodeTableView::ExportToCSV(const gtString& newFileName)
{
    QString fileName = QString::fromWCharArray(newFileName.asCharArray());

    if (!fileName.isEmpty())
    {
        // Save the file:
        osFile file(newFileName);
        // Open the file for write:
        bool rcOpen = file.open(osFile::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
        GT_IF_WITH_ASSERT(rcOpen)
        {
            // Get html text:
            if (!m_instructionsAsCSV.isEmpty())
            {
                // Write the data:
                file.writeString(acQStringToGTString(m_instructionsAsCSV));
                file.close();
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaSourceCodeTableView::OnAboutToShowMenu()
{
    for (QAction* pAction : m_addedMenuActions)
    {
        QString actionText = pAction->text();
        GT_IF_WITH_ASSERT(actionText.length() > 0)
        {
            if (actionText.contains(QString("Copy"), Qt::CaseInsensitive))
            {
                QString selectedText = m_pSourceTableViewHtml->selectedText();

                if (selectedText.length())
                {
                    pAction->setEnabled(true);
                }
                else
                {
                    pAction->setEnabled(false);
                }

                break;
            }
        }
    }
}

void kaSourceCodeTableView::keyPressEvent(QKeyEvent* pE)
{
    if (pE != nullptr)
    {
        if ((pE->key() == Qt::Key_A) && (pE->modifiers().testFlag(Qt::ControlModifier)))
        {
            if (m_pSourceTableViewHtml != nullptr && m_pSourceTableViewHtml->page() != nullptr)
            {
                m_pSourceTableViewHtml->page()->triggerAction(QWebPage::SelectAll);
            }
        }
    }
}


