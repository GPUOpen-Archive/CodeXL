//------------------------------ kaProjectSettingsShaderExtension.h ------------------------------

#ifndef __KAPROJECTSETTINGSSHADEREXTENSION_H
#define __KAPROJECTSETTINGSSHADEREXTENSION_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTApplicationFramework/Include/afProjectSettingsExtension.h>

// Local:
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>
#include <AMDTKernelAnalyzer/src/kaProjectSettingsExtensionBase.h>

#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

// ----------------------------------------------------------------------------------
class KA_API kaProjectSettingsShaderExtension : public kaProjectSettingsExtensionBase
{
    Q_OBJECT

    enum CompilerType
    {
        D3D = 0,
        FXC
    };

    enum CommandBitFlags
    {
        COMMANDBITFLAG_ALLOWFLOWCONTROL = 1 << 9,
        COMMANDBITFLAG_DEBUG = 1 << 0,
        COMMANDBITFLAG_ENABLEBACKWORDSCOMPATIBILITY = 1 << 12,
        COMMANDBITFLAG_ENABLESTRICTNESS = 1 << 11,
        COMMANDBITFLAG_EPIXELOPTOFF = 1 << 7,
        COMMANDBITFLAG_VERTEXOPTOFF = 1 << 6,
        COMMANDBITFLAG_IEEESTRICTNESS = 1 << 13,
        COMMANDBITFLAG_NOPRES = 1 << 8,
        COMMANDBITFLAG_OPTLEVELSKIP = 1 << 2,
        COMMANDBITFLAG_OPTLEVEL0 = 1 << 14,
        COMMANDBITFLAG_OPTLEVEL1 = 0,
        COMMANDBITFLAG_OPTLEVEL2 = (1 << 14) | (1 << 15),
        COMMANDBITFLAG_OPTLEVEL3 = 1 << 15,
        COMMANDBITFLAG_PACKMATRIXCOLMAJOR = 1 << 4,
        COMMANDBITFLAG_PACKMATRIXROWMAJOR = 1 << 3,
        COMMANDBITFLAG_PARTIALPRECISION = 1 << 5,
        COMMANDBITFLAG_REFERFLOWCONTROL = 1 << 10,
        COMMANDBITFLAG_RESOURCESMATALIAS = 1 << 19,
        COMMANDBITFLAG_SKIPVALIDATION = 1 << 1,
        COMMANDBITFLAG_WARNINGSAREERRORS = 1 << 18,
    };

public:
    kaProjectSettingsShaderExtension();
    virtual ~kaProjectSettingsShaderExtension();

    /// Initialize the widget:
    virtual void Initialize();

    /// Return the extension name:
    /// \returns the extension xml name
    virtual gtString ExtensionXMLString();

    /// Return the extension page title:
    /// \returns tree path name
    virtual gtString ExtensionTreePathAsString();

    /// Load the project settings into a string
    /// \param projectAsXMLString is the xml string
    virtual bool GetXMLSettingsString(gtString& projectAsXMLString);

    /// sets the project settings from the xml string
    /// \param projectAsXMLString is the xml string
    virtual bool SetSettingsFromXMLString(const gtString& projectAsXMLString);

    /// restores default settings of the project settings
    virtual void RestoreDefaultProjectSettings();

    /// returns the setting is valid (true)
    /// \param invalidMessageStr is message for invlaid case
    virtual bool AreSettingsValid(gtString& invalidMessageStr);

    /// restored current settings
    virtual bool RestoreCurrentSettings();

    /// Get the data from the widget:
    virtual bool SaveCurrentSettings();

protected:

    virtual void UpdateProjectDataManagerWithTBOptions(const QString& value, QStringList& optionsList) override;
    virtual void UpdateProjectDataManagerWithTBOptions(const QString& value) override;
    /// Adds a check-box option to the dialog.
    /// \param Layout - the check-box layout to be set on
    /// \param checkBoxInfo is the check-box properties
    /// \param checkBoxInfoTooltip is the tool tip string of the check-box
    /// \param commandFlag bit flag for the command
    /// \returns pointer to flags data struct representing the added widget info
    virtual kaProjectSettingFlagData* AddCheckBoxOption(QGridLayout* pLayout,
                                                        const QString& checkBoxInfo,
                                                        const QString& checkBoxInfoTooltip,
                                                        int commandFlag);

    /// Adds a combobox option to the dialog.
    /// \param Layout - the combo layout to be set on
    /// \param comboBoxInfo is the combo properties
    /// \param comboBoxInfoTooltip is the tool tip string of the combo
    /// \param comboType is the combo type (for shader is KA_FLAG_TYPE_COMBO_BOX)
    /// \param defaultIndex is the index of the default item in the combobox
    /// \param list of bit flags, one per combo item, for the command
    /// \returns pointer to flags data struct representing the added widget info
    virtual kaProjectSettingFlagData* AddComboBoxOption(QGridLayout* pLayout,
                                                        const QString& comboBoxInfo,
                                                        const QString& comboBoxInfoTooltip,
                                                        kaProjectSettingFlagData::kaProjectFlagType comboType,
                                                        int defaultIndex,
                                                        QList<int> commandFlagsList);

    /// handles the click on browse button from the combo items
    /// \param pCombox is the combo holding the browse button
    /// \param allowedBrowsedFile - is the kind of files to be displayed in the browse dialog window
    /// \param lastSelectedIndex = is the last valid selected index in the combo
    void OnBrowseButtonClicked(QComboBox* pCombox, const QString& allowedBrowsedFile, int lastSelectedIndex, CompilerType compilerType);

    /// The edit box widget column span
    /// \returns the actual column span
    virtual int EditBoxColumnSpan();

    /// returns the relevant flag index in the list by compile kind
    virtual int GetFlagIndex();

    /// Handle ComboBox case in the Command text box
    /// \param pFlagData is the data to be set from
    /// \param optionsList is the command line strings list
    virtual void HandleComboBoxInText(kaProjectSettingFlagData* pFlagData, QStringList& optionsList);

    /// Handle TextEdit case in the Command text box
    /// \param pFlagData is the data to be set from
    /// \param optionsList is the command line strings list
    virtual void HandleTextEditInText(kaProjectSettingFlagData* pFlagData, QStringList& optionsList);

    /// get build option string from project manager
    /// \returns build option string to parse
    virtual QString GetBuildOptionString();

protected slots:

    /// Handle text change in the Command TextEdit:
    virtual void OnCommandTextChanged();

    // Handles the change in radio buttons selection
    // \param compileKind - unused
    void OnCompileTypeSelectionChanged(int compileKind);

    /// Handles compile type changed from xml
    void OnCompileTypeChangedFromXML();

    /// Handles the change in D3D path combo selection
    /// \param selectedIndex is the selected index in the combo
    void OnD3dComboxSelectionChanged(int selectedIndex);

    /// Handles the change in FXC path combo selection
    /// \param selectedIndex is the selected index in the combo
    void OnFxcComboxSelectionChanged(int selectedIndex);

    /// Handles the change in combo selection
    /// \param index is the selected index in the combo
    virtual void OnComboBoxChanged(int index);

    /// Handle the text changes
    /// \param lineText is the text string
    virtual void OnTextChange(const QString& lineText);

private:
    /// Generate the table of controls
    void GenerateOptionsTable();

    /// \returns true if D3d compile type
    bool IsD3dCompileKind();

    /// adds mask to command box label (only for D3d compile type)
    void UpdateCommandLabelWithMask();

    /// returns the msak for D3d compile. will return 0 if the selected compile type is FXC
    /// \returns the build options mask
    unsigned int GetD3dBuildptionsMask();

    /// sets the combo with the path. if it's new - adds it first.
    /// \param pCombo is the combo to be set
    /// \param path is the path string item to be selected or added and selected
    /// \param compilerType
    /// \return true if the path was valid
    bool SetPathCombobox(QComboBox* pCombo, const QString& path, CompilerType compilerType);

    /// sets the combo with the path. if it's new - adds it first.
    /// \param pCombo is the combo to be set
    /// \param path is the path osFilePath item to be selected or added and selected
    /// \param compilerType
    /// \return true if the path was valid
    bool SetPathCombobox(QComboBox* pCombo, const osFilePath& path, CompilerType compilerType);

    /// sets the D3D path combo and FXC path combo with default paths + browse item
    void InitPathComboxes();

    /// returns true if the path is valid as D3D builder file
    /// \param path is the checked path
    bool ValidateD3DPath(const osFilePath& path) const;

    /// returns true if the path is valid as FXC builder file
    /// \param path is the checked path
    bool ValidateFXCPath(const osFilePath& path) const;

    /// sets the check-box widget disable if the relevant (D3d/FXC selected build type) build option is empty
    /// the selected build type
    /// \param selectedBuildType is the selected build type D3D/FXC
    void DisableCheckBoxWithEmptyOption(CompilerType selectedBuildType);

    /// gets the browsing string& by the combo compile type
    /// \param compilerType is the combo type
    /// \returns a reference to the relevant gtString
    gtString& GetBrowsingPathByComboType(CompilerType compilerType);

    /// sets the paths combo selected item as bold
    /// \param pCombo is the combo to be updated
    void SetComboSelectedStringBold(QComboBox* pCombo);

    QLabel* m_pCommandLineLabel;

    QButtonGroup* m_pCompileKindRadioGroup;     // radio buttons group (D3d radio button + Fxc radio button
    QRadioButton* m_pD3dRadio;                  // D3d radio button - saved for checking the selected radio button
    QRadioButton* m_pFxcRadio;                  // FXC radio button

    QComboBox* m_pD3dPathCombox;        // D3D path combo
    int        m_d3dComboLastIndex;     // last selected index in D3D path combo
    QComboBox* m_pFxcPathCombox;        // FXC path combo
    int        m_fxcComboLastIndex;     // last selected index in FXC path combo

    QString m_originalBuildOptions;     // build options string

    QLineEdit* m_pShaderIncludesLineEdit;   // shader includes inserted by user
    QLineEdit* m_pShaderMacrosLineEdit;     // shader macros inserted by user

    QCheckBox* m_pShaderIntrinsicsCheckBox; // checked to enable D3D Shader Intrinsics extension

    gtString m_sD3dDllBrowsingPath;               // browsing directory path for d3d dll's
    gtString m_sFxcExeBrowsingPath;               // browsing directory path for Fxc exe files

};

#endif //__KAPROJECTSETTINGSSHADEREXTENSION_H

