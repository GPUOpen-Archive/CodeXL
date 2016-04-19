//------------------------------ kaProjectSettingsExtensionBase.h ------------------------------

#ifndef __KAPROJECTSETTINGSEXTENSIONBASE_H
#define __KAPROJECTSETTINGSEXTENSIONBASE_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTApplicationFramework/Include/afProjectSettingsExtension.h>
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>

struct kaProjectSettingFlagData
{
public:
    enum kaProjectFlagType
    {
        KA_FLAG_TYPE_CHECK_BOX = 0,
        KA_FLAG_TYPE_COMBO_BOX,
        KA_FLAG_TYPE_COMBO_BOX_JOINED,
        KA_FLAG_TYPE_LINE_EDIT,
        KA_FLAG_TYPE_LINE_EDIT_SINGLE,
        KA_FLAG_TYPE_LINE_EDIT_CHILD
    };



    /// what type of flag:
    kaProjectFlagType           m_flagType;

    /// Associated string (multiple options if it is combobox)
    QStringList                 m_flagStringList;

    /// pointer to the control (either checkbox, combobox or textedit):
    QWidget*                    m_pWidget;

    /// Pointer to the label if it is a checkbox, NULL in all other cases:
    QList<QLabel*>              m_pLabels;

    /// Previous text that was used by the flag data
    QString                     m_previousFlagString;

    /// Child data relation
    kaProjectSettingFlagData*   m_pChildData;

    /// Parent data relation
    kaProjectSettingFlagData*   m_pParentData;

    /// the default item index for combo widget
    int                         m_defaultComboIndex;

    /// list of flags for the specific command (checkbox - only 1 flag [0], combobox - list of flags)
    QList<int>                  m_bitFlagsList;
};

// ----------------------------------------------------------------------------------
// Class Name:           kaDebugActionsCreator : public afActionCreatorAbstract
// General Description:  This class is used for handling the creation and execution of
//                       all actions related to the source code editing
// Author:               Gilad Yarnitzky
// Date:                 5/8/2013
// ----------------------------------------------------------------------------------
class KA_API kaProjectSettingsExtensionBase : public afProjectSettingsExtension
{
    Q_OBJECT

public:
    kaProjectSettingsExtensionBase();
    virtual ~kaProjectSettingsExtensionBase();

protected slots:
    /// Handle text change in the Command TextEdit:
    virtual void OnCommandTextChanged();

    /// Handle the clicking on the different check boxes:
    virtual void OnCheckBoxClicked();
    /// Handle Combobox selection changes:
    virtual void OnComboBoxChanged(int index);

    /// Handle the text changes:
    virtual void OnTextChange(const QString& lineText);

protected:
    /// Handle text modifications:
    void AddString(const QString& stringToAdd);
    void RemoveString(const QString& stringToRemove);
    void ReplaceString(const QString& stringToFind, const QString& stringToReplace);

    /// Enable Child row:
    void EnableChild(kaProjectSettingFlagData* pData, bool mode);

    virtual void UpdateProjectDataManagerWithTBOptions(const QString& value, QStringList& optionsList) = 0;
    virtual void UpdateProjectDataManagerWithTBOptions(const QString& value);
    //-----------------------------------------------------------------------------
    /// Adds a combobox option to the dialog.
    /// \param[in] checkBoxInfo  String with the information Caption#flag
    //-----------------------------------------------------------------------------
    virtual kaProjectSettingFlagData* AddCheckBoxOption(QGridLayout* pLayout, const QString& checkBoxInfo, const QString& checkBoxInfoTooltip);

    //-----------------------------------------------------------------------------
    /// Adds a combobox option to the dialog.
    /// \param[in] comboBoxInfo  String with the information Caption#option1,option2,....
    ///            in case of JOINED then the format is Caption#lead,option1,option2,....only if there is () or - section it gets the lead
    //-----------------------------------------------------------------------------
    virtual kaProjectSettingFlagData* AddComboBoxOption(QGridLayout* pLayout, const QString& comboBoxInfo, const QString& comboBoxInfoTooltip, kaProjectSettingFlagData::kaProjectFlagType comboType = kaProjectSettingFlagData::KA_FLAG_TYPE_COMBO_BOX);

    //-----------------------------------------------------------------------------
    /// Adds a combobox option to the dialog.
    /// \param[in] textEditInfo  String with the information Caption#flag
    //-----------------------------------------------------------------------------
    kaProjectSettingFlagData* AddTextEditOption(QGridLayout* pLayout, const QString& textEditInfo, const QString& textEditInfoTooltip, kaProjectSettingFlagData::kaProjectFlagType editType = kaProjectSettingFlagData::KA_FLAG_TYPE_LINE_EDIT, kaProjectSettingFlagData* pParent = NULL);

    //-----------------------------------------------------------------------------
    /// the function returns the column span fr the edit box
    /// \returns the column span - int his case = 1
    //-----------------------------------------------------------------------------
    virtual int EditBoxColumnSpan();

    //-----------------------------------------------------------------------------
    /// Replace text in the command text
    /// \param[in] pFlagData    flag data that hold the new and old text
    /// \param[in] newText      the text to be used
    //-----------------------------------------------------------------------------
    void ReplaceText(kaProjectSettingFlagData* pFlagData, const QString& newText);

    /// Replace text when the text can be splitup alone the command text and with spaces between the section
    void ReplaceSplitText(kaProjectSettingFlagData* pFlagData, const QString& newText);

    /// Join joined ComboBoxes two parameters into one:
    void JoinJoinedComboBoxes(QStringList& optionsList);

    /// Join words with starting and brackets
    void JoinBrackets(QStringList& optionsList);

    /// Add wrapping brackets around string if there aren;t already there
    void AddBrackets(QString& stringToAddBrackets);

    /// returns 0 - the first flag in the list
    virtual int GetFlagIndex();

    /// Handle all the cases in the Command text box
    void HandleCheckBoxInText(kaProjectSettingFlagData* pFlagData, QStringList& optionsList);
    virtual void HandleComboBoxInText(kaProjectSettingFlagData* pFlagData, QStringList& optionsList);
    virtual void HandleTextEditInText(kaProjectSettingFlagData* pFlagData, QStringList& optionsList);
    void HandleSingleLineTextInText(kaProjectSettingFlagData* pFlagData, QStringList& optionsList);

    /// get build option string from project manager
    /// \returns build option string to parse
    virtual QString GetBuildOptionString();

    /// Holds the generated Combobox items:
    gtVector<QComboBox*> m_comboBoxVector;

    /// strings of joined options such as "-x clc"
    gtVector<QString> m_joinedVectorStrings;

    /// map that connects between the control and the flag data:
    gtMap<QWidget*, kaProjectSettingFlagData*> m_widgetFlagDataMap;

    /// main text control that hold out command line:
    QTextEdit* m_pCommandTextEdit;

    /// Grid layout widget
    QWidget* m_pGridWidget;

    /// Holds the grid layout of the dialog
    QGridLayout* m_pGridLayout;

    /// updating the text so no need to parse it:
    bool m_updatingCommandText;

    /// is basic Build Option
    bool m_isBasicBuildOption;

    /// check box num of flags
    int m_checkboxFlagsNum;

    /// unnecessary string in the build options text to which no flag was found
    QStringList m_avoidableStringsList;
};

#endif //__KAPROJECTSETTINGSEXTENSIONBASE_H

