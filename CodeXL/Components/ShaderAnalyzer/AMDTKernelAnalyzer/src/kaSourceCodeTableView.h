//------------------------------ kaSourceCodeView.h ------------------------------

#ifndef __KASOURCECODETABLEVIEW__H
#define __KASOURCECODETABLEVIEW__H



#include <string>
#include <vector>
// Qt
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>

// Backend:
#include <AMDTBackEnd/Emulator/Parser/Instruction.h>

// Local:
#include <kaDataTypes.h>

class Instruction;
class QWebView;

enum KA_ISA_COLUMNS
{
    KA_ISA_ADDRESS = 0,
    KA_ISA_OPCODE,
    KA_ISA_OPERANDS,
    KA_ISA_CYCLES,
    KA_ISA_INSTRUCTION_TYPE,
    KA_ISA_HEX,
    KA_ISA_COLUMN_COUNT
};

class kaSourceCodeTableView : public QWidget
{
    Q_OBJECT
public:
    /// Constructor
    kaSourceCodeTableView(QWidget* pParent);

    /// Destructor
    virtual ~kaSourceCodeTableView();

    /// Update the view by reloading the file
    void UpdateView();

    /// Set text without updating caption
    void SetISAText(const osFilePath& isaFilePath);

    /// Sets the platform indicator
    void SetViewPlatform(kaPlatform platformIndicator) { m_platformIndicator = platformIndicator; }

    bool DisplayISAFile(const osFilePath& filePath);

    /// Returns true if there are selected items
    bool HasSelectedItems() const;

    /// Returns true if the table has items
    bool ContainsData() const;

    /// Calls acListCtrl::onFindClick
    void OnFindClick();

    /// Calls acListCtrl::onFindNext
    void OnFindNext();

    /// Checks if m_pSourceTableView is in focus
    bool IsTableInFocus() const;

    /// Returns file path
    const osFilePath& ISAFilePath()const { return m_filePath; }

    /// Sets dirty flag
    void SetDirty(bool isDirty) { m_isDirty = isDirty; }

    /// Gets dirty flag
    bool IsDirty()const { return m_isDirty; }

private:
    void FillSourceTableView(const std::vector<Instruction*>& isaInstructions);

    void AddInstruction(const Instruction* pInstruction);
    void AddInstructionRow(const Instruction* pInstruction);
    void AddLabelRow(const Instruction* pInstruction);

    const QString& GetColorStringByInstructionCategory(Instruction::InstructionCategory instructionType);
    void BuildOpcodeString(const std::string& opcode, const QString& colorAsHex, bool isBranch, QString& opcodeString) const;

    ///returns color depending on cycles value
    const QColor& GetColorStringDependingOnCycles(int cycleCount);

    /// returns color formatted operands string
    QString GetSyntaxHighligtedOperandsString(const std::string& operands);


private:
    struct kaSourceCodeTableViewRowItemData
    {
        kaSourceCodeTableViewRowItemData() : m_pColor(nullptr) {};
        ~kaSourceCodeTableViewRowItemData() {};
        QString m_originalText;
        QString m_coloredText;
        const QColor* m_pColor;
    };

private:
    /// Indicates if this source code view is used for OpenCL. (needed to know what type of info to display about in the captions and menu)
    kaPlatform m_platformIndicator;

    /// list of added context menu actions
    gtVector<QAction*> m_addedMenuActions;

    /// HTML view for displaying the ISA code as HTML table:
    QWebView* m_pSourceTableViewHtml;

    /// The HTML text representing the ISA code:
    QString m_htmlText;

    /// Instructions as CSV values string:
    QString m_instructionsAsCSV;

    // Displayed file path:
    osFilePath m_filePath;
    QString m_deviceName;
    std::vector<Instruction*> m_isaInstructions;
    std::map<std::string, size_t> m_labelsMap;

    /// Source code table context menu:
    QMenu* m_pContextMenu;

    QString m_colorStrings[Instruction::InstructionsCategoriesCount];
    QString m_defaultColorString;

    /// dirty flag indicates if the table view should be updated
    bool m_isDirty;
public slots:

    void AddSeparator(bool first = true);
    void AddMenuAction(QAction* pAction, bool first = true);
    void UseColorFormatting(bool show);

    /// Save the contents of html table as comma separated file
    /// \param [in] newFileName
    void ExportToCSV(const gtString& newFileName);

    void OnContextMenuEvent(const QPoint& pos);


    /// Implements copy in the HTML table:
    void OnCopy();

    /// Implements select all in the HTML table:
    void OnSelectAll();

    /// Used to update menu items
    void OnAboutToShowMenu();
protected:
    void keyPressEvent(QKeyEvent* pEvent);
};

#endif  // __KASOURCECODETABLEVIEW__H
