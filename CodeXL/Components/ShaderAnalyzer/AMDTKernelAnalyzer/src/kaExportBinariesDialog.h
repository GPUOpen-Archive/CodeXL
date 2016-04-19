//------------------------------ kaExportBinariesDialog.h ------------------------------
#ifndef __KAEXPORTBINARIESDIALOG
#define __KAEXPORTBINARIESDIALOG

#include <QtWidgets>

#include <qfiledialog.h>
#include <qobject.h>

// ----------------------------------------------------------------------------------
// Class Name:              kaExportBinariesDialog : public QFileDialog
// General Description:     Prompts the user to choose export directory along
//                          with sections to be included (other sections will be suppressed)
//                          Source - includes .source
//                          IL - includes .amdil
//                          ISA - includes .text
//                          LLVM IR - includes .llvmir
//                          Debug info - includes .debugil, .debug_info, .debug_addrev, .debug_line
//                          .debug_log, .debug_ranges, .debug_str
//
// Author:               Yuri Rshtunique
// Creation Date:        17/06/2014
// ----------------------------------------------------------------------------------
class kaExportBinariesDialog : public QFileDialog
{
    Q_OBJECT

public:
    kaExportBinariesDialog(QWidget* parent, const QString& qstrBaseFileName,
                           const QString& qstrBaseNameSuffix,
                           const QStringList& qslistDevicesToExport,
                           const bool is32bitCheckboxSet,
                           const bool is64bitCheckboxSet);
    ~kaExportBinariesDialog();

    bool IsILIncluded()const;
    bool IsSourceIncluded()const;
    bool IsISAIncluded()const;
    bool Is32BitIncluded()const;
    bool Is64BitIncluded()const;
    bool IsLLVM_IRIncluded()const;
    bool IsDebugInfoIncluded()const;


    void ILEnabled(const bool val)        {m_pCheckBoxIncludeIL->setEnabled(val);}
    void SourceEnabled(const bool val)    {m_pCheckBoxIncludeSource->setEnabled(val);}
    void ISAEnabled(const bool val)       {m_pCheckBoxIncludeISA->setEnabled(val);}
    void LLVM_IREnabled(const bool val)   {m_pCheckBoxIncludeLLVM_IR->setEnabled(val);}
    void DebugInfoEnabled(const bool val) {m_pCheckBoxIncludeDebugInfo->setEnabled(val);}
    void Bitness32Enabled(const bool val) { m_pCheckBoxBittness32->setEnabled(val); }
    void Bitness64Enabled(const bool val) { m_pCheckBoxBittness64->setEnabled(val); }


    void ILCehcked(const bool val)        { m_pCheckBoxIncludeIL->setChecked(val);};
    void SourceCehcked(const bool val)    {m_pCheckBoxIncludeSource->setChecked(val);}
    void ISACehcked(const bool val)       {m_pCheckBoxIncludeISA->setChecked(val);}
    void LLVM_IRCehcked(const bool val)   {m_pCheckBoxIncludeLLVM_IR->setChecked(val);}
    void DebugInfoCehcked(const bool val) {m_pCheckBoxIncludeDebugInfo->setChecked(val);}

    const QString GetBaseName()const;
    QStringList selectedFiles() const;

protected:
    void keyPressEvent(QKeyEvent* e);
    bool eventFilter(QObject* obj, QEvent* e);
    void accept();

private:
    QVBoxLayout*            m_pVBoxLayout;
    QCheckBox*              m_pCheckBoxIncludeSource;
    QCheckBox*              m_pCheckBoxIncludeIL;
    QCheckBox*              m_pCheckBoxIncludeISA;
    QCheckBox*              m_pCheckBoxBittness32;
    QCheckBox*              m_pCheckBoxBittness64;
    QCheckBox*              m_pCheckBoxIncludeLLVM_IR;
    QCheckBox*              m_pCheckBoxIncludeDebugInfo;
    QPushButton*            m_pButtonExport;
    QGridLayout*            m_pOrigLayout;

    // following members names are QFileDialog children object names
    QDialogButtonBox*       buttonBox;
    QLabel*                 lookInLabel;
    QComboBox*              lookInCombo;
    QToolButton*            backButton;
    QToolButton*            forwardButton;
    QToolButton*            detailModeButton;
    QToolButton*            toParentButton;
    QToolButton*            newFolderButton;
    QToolButton*            listModeButton;
    QComboBox*              fileTypeCombo;
    QLabel*                 fileNameLabel;
    QLabel*                 fileTypeLabel;
    QLabel*                 bittnessTypeLabel;
    QLineEdit*              baseFileName;
    QLineEdit*              fileNameEdit;
    QLineEdit*              lookInComboLineEdit;
    QString                 qstrDefaultBaseName;
    QLabel*                 baseFileNameDescription;
    bool                    m_bExportButtonEnabled;
    const QStringList&      m_qslistDevicesToExport;

    QString                 qstrCustomBaseName;
    QString                 qstrConstBaseNameSuffix;
    QLabel*                 baseFileNameLabel;


    void SetDialogOptions();
    bool InitDialogWidgets(const bool is32bitCheckboxSet, const bool is64bitCheckboxSet);
    void SetDialogLayout(bool bInitSucceeded);
    QString adjustQtStringToCurrentOS(const QString& dirName)const;

    /// Prepares current exported binary full name to check if already exists
    void PrepareCurrentBinaryFullName(QString& qstrResult, const QString& qstrCurrentPath, const QString& qstrBaseFileName, const QString& qstrDeviceName)const;

    /// Checks if current binary already exists
    bool ExportedBinaryAlreadyExists(const QString&)const;

protected slots:
    void SetChosenDirectory();
    void BuildFileNameStringOnTextEdited();
    void ValidateBaseFileNameFinal();
    void testSelection(const QString& dirName);
    void CheckBoxStateChanged(bool bState);
    bool CheckEnteredDirectory();
};

#endif // __KAEXPORTBINARIESDIALOG
