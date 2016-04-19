//------------------------------ kaBuildToolbar.h ------------------------------

#ifndef __KABUILDTOOLBAR_H
#define __KABUILDTOOLBAR_H

// Qt:
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

// Infra:
#include <AMDTApplicationComponents/Include/acToolBar.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaDataTypes.h>

class QTreeWidgetItem;
class kaSourceFile;

class kaBuildToolbar : public acToolBar
{
    Q_OBJECT

public:

    kaBuildToolbar(QWidget* pParent);
    ~kaBuildToolbar();

    /// update ui based on the file path
    void updateUIonMDI(const osFilePath& filePath, bool forceBuild = false);

    /// returns the compile options:
    QString compileOptions() { return m_pCompileOptions->text(); }

    /// Sets the text of the build options:
    void SetBuildOptions(const QString& buildOptions);

    /// Gets the currently selected entry point or kernel:
    QString getEntryPointName() const;

    /// sets the function combo-box to the function parameter
    /// \param function is the string to be set
    void ChangeFunctionSelection(const gtString& function);

    // clear tool bar widgets
    void ClearToolBar();

    /// rebuild the entry combobox for a specific file
    /// \param pFile the file that should rebuild the list for
    /// \param should we restore the current combo selection
    void RebuildKernelsAndEntryList(kaSourceFile* pFile, bool shouldRememberCurrentFunc);

protected slots:
    void comboSelectionChange(int selectedItemIndex);
    void onBuild();
    void onOptions();
    void onBuildOptionsTextChanged(const QString& optionsText);
    void OnSettings();

    /// Handle Build Architecture selection
    void BuildArchitectureSelectionChange(int selectedItemIndex);

    /// Handle Platform selection
    void ShaderModelSelectionChange(int selectedItemIndex);

    /// Handle DirectX Shader Type selection
    /// \param selectedItemIndex is Type combo box selected item index
    void DirectXShaderTypeSelectionChange(int selectedItemIndex);

    /// Handle application main tree item activation (only if they are related to KA)
    void ItemSelectionChanged();

    // Update UI when build started:
    void OnBuildStarted(const gtString& sourceName);

    // Update UI when build ended:
    void OnBuildEnded(const gtString& sourceName);

private:
    void UpdateToolbarElementsBySelectedNodeType();
    void UpdateToolbarElementsByFileType(kaFileTypes fileType);
    void UpdateToolbarElementsByProgramType(kaProgramTypes programType);

    /// Show kernels/entry points combo and hide other ui elements for a file from source pool
    void ShowSourceFileNavigationDropList();
private:

    /// Label for the compile options:
    QLabel* m_pLineLabel;
    /// Field for the compile options:
    QLineEdit* m_pCompileOptions;
    /// Button to open device options:
    QPushButton* m_pOptionsButton;
    /// Button to open project settings:
    QPushButton* m_pSettingsButton;
    QComboBox* m_pBuildArchitectureCombo;
    /// targets label
    acWidgetAction* m_pShaderModelsLabelAction;
    /// List of targets
    acWidgetAction* m_pShaderModelsComboAction;
    /// types label
    acWidgetAction* m_pShaderTypesLabelAction;

    /// Label to the current roll of the m_pKernelsAndEntriesCombo combo
    QLabel* m_pKernelsAndEntriesLabel;

    /// action for hiding kernels and entries combo
    QAction* m_pKernelsAndEntriesComboActionToHide;
    /// action for hiding kernels and entries combo label
    QAction* m_pKernelsAndEntriesComboLabelActionToHide;

    /// List of kernels

    QComboBox* m_pKernelsAndEntriesCombo;
    /// File path to the file which the kernel list refers to:
    osFilePath m_filePath;

    bool m_inUpdateUIonMDI;

    /// DirectX shaders
    acWidgetAction* m_pDXShaderTypesComboAction;
    /// DX types label
    // acWidgetAction* m_pDXTypesLabelAction;
};
#endif // __KABUILDTOOLBAR_H
