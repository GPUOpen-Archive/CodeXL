//------------------------------ kaProjectSettingsExtensionBase.h ------------------------------

// TinyXml:
#include <tinyxml.h>

// QT:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaBuildToolbar.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaProjectSettingsExtensionBase.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtensionBase::kaProjectSettingsExtensionBase
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------

#define KA_TEXT_EDIT_BOX_COLUMN_SPAN 1

kaProjectSettingsExtensionBase::kaProjectSettingsExtensionBase() : afProjectSettingsExtension(), m_pCommandTextEdit(NULL), m_pGridWidget(NULL), m_pGridLayout(NULL)
{
    m_updatingCommandText = false;
    m_isBasicBuildOption = true;
    m_checkboxFlagsNum = 1;
}

// ---------------------------------------------------------------------------
// Name:        kaProjectSettingsExtensionBase::~gdProjectSettingsExtension
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        5/8/2013
// ---------------------------------------------------------------------------
kaProjectSettingsExtensionBase::~kaProjectSettingsExtensionBase()
{
}


// ---------------------------------------------------------------------------
kaProjectSettingFlagData* kaProjectSettingsExtensionBase::AddCheckBoxOption(QGridLayout* pLayout, const QString& checkBoxInfo, const QString& checkBoxInfoTooltip)
{
    kaProjectSettingFlagData* retData = NULL;
    QStringList stringList = checkBoxInfo.split("#");

    GT_IF_WITH_ASSERT(stringList.length() == 2)
    {
        // Build formatted tooltip:
        QString tooltipStr;
        acWrapAndBuildFormattedTooltip(stringList[1], checkBoxInfoTooltip, tooltipStr);

        // Create all the components:
        QCheckBox* pCheckBox = new QCheckBox(stringList[0]);

        pCheckBox->setToolTip(tooltipStr);

        QLabel* pLabel = new QLabel(stringList[1]);

        pLabel->setDisabled(true);
        pLabel->setToolTip(tooltipStr);

        kaProjectSettingFlagData* pFlagData = new kaProjectSettingFlagData;


        pFlagData->m_flagStringList.push_back(stringList[1]);
        pFlagData->m_flagType = kaProjectSettingFlagData::KA_FLAG_TYPE_CHECK_BOX;
        pFlagData->m_pLabels << pLabel;
        pFlagData->m_pWidget = pCheckBox;
        pFlagData->m_pChildData = NULL;
        pFlagData->m_pParentData = NULL;
        pFlagData->m_previousFlagString = stringList[1];

        m_widgetFlagDataMap[pCheckBox] = pFlagData;
        retData = pFlagData;

        // Add the items to the grid:
        int numRows = pLayout->rowCount();
        pLayout->addWidget(pCheckBox, numRows, 0, 1, 1);
        pLayout->addWidget(pLabel, numRows, 1, 1, 1);

        connect(pCheckBox, SIGNAL(clicked()), this, SLOT(OnCheckBoxClicked()));
    }

    return retData;
}

// ---------------------------------------------------------------------------
kaProjectSettingFlagData* kaProjectSettingsExtensionBase::AddComboBoxOption(QGridLayout* pLayout, const QString& comboBoxInfo, const QString& comboBoxInfoTooltip, kaProjectSettingFlagData::kaProjectFlagType comboType)
{
    kaProjectSettingFlagData* retData = NULL;
    QStringList stringList = comboBoxInfo.split("#");

    GT_IF_WITH_ASSERT(stringList.length() == 2)
    {
        // Build formatted tooltip:
        QString tooltipStr;
        acWrapAndBuildFormattedTooltip("", comboBoxInfoTooltip, tooltipStr);

        // Create all the components:
        QLabel* pLabel = new QLabel(stringList[0]);

        pLabel->setToolTip(tooltipStr);

        QStringList comboOptionsList = stringList[1].split(",");
        QComboBox* pComboBox = new QComboBox();

        pComboBox->addItems(comboOptionsList);
        pComboBox->setToolTip(tooltipStr);

        kaProjectSettingFlagData* pFlagData = new kaProjectSettingFlagData;


        // find only the text in the ( ) for each string to be passed to he flag string data
        int numStrings = comboOptionsList.length();

        for (int nString = 0; nString < numStrings; nString++)
        {
            QString currentString = comboOptionsList[nString];
            int startOfSub = currentString.indexOf('-');

            if (startOfSub != -1)
            {
                QString subString;
                int endOfSub = currentString.indexOf(')');

                if (-1 == endOfSub)
                {
                    subString = currentString.mid(startOfSub);
                }
                else
                {
                    subString = currentString.mid(startOfSub, endOfSub - startOfSub);
                }

                pFlagData->m_flagStringList.push_back(subString);
            }
            else
            {
                // for the "Default" string enter and empty string
                pFlagData->m_flagStringList.push_back("");
            }
        }

        pFlagData->m_flagType = comboType;
        pFlagData->m_pWidget = pComboBox;
        pFlagData->m_pChildData = NULL;
        pFlagData->m_pParentData = NULL;

        if (kaProjectSettingFlagData::KA_FLAG_TYPE_COMBO_BOX_JOINED == comboType)
        {
            // get the leading from the second item in the string by splitting space
            QStringList leadingString = pFlagData->m_flagStringList[1].split(" ");
            GT_IF_WITH_ASSERT(leadingString.count() == 2)
            {
                m_joinedVectorStrings.push_back(leadingString[0]);
            }
        }

        // Add the items to the vectors:
        m_comboBoxVector.push_back(pComboBox);

        m_widgetFlagDataMap[pComboBox] = pFlagData;
        retData = pFlagData;

        // Add the items to the grid:
        int numRows = pLayout->rowCount();
        pLayout->addWidget(pLabel, numRows, 0, 1, 1);
        pLayout->addWidget(pComboBox, numRows, 1, 1, 1);

        connect(pComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboBoxChanged(int)));
    }

    return retData;
}

// ---------------------------------------------------------------------------
kaProjectSettingFlagData* kaProjectSettingsExtensionBase::AddTextEditOption(QGridLayout* pLayout, const QString& textEditInfo, const QString& textEditInfoTooltip, kaProjectSettingFlagData::kaProjectFlagType editType, kaProjectSettingFlagData* pParent)
{
    kaProjectSettingFlagData* retData = NULL;
    QStringList stringList = textEditInfo.split("#");

    GT_IF_WITH_ASSERT(stringList.length() == 2)
    {
        // Build formatted tooltip:
        QString tooltipStr;
        acWrapAndBuildFormattedTooltip("", textEditInfoTooltip, tooltipStr);

        // Create all the components:
        QLabel* pLabel = new QLabel(stringList[0]);

        pLabel->setToolTip(tooltipStr);

        QLineEdit* pLineEdit = new QLineEdit();

        pLineEdit->setToolTip(tooltipStr);

        kaProjectSettingFlagData* pFlagData = new kaProjectSettingFlagData;


        pFlagData->m_flagStringList.push_back(stringList[1]);
        pFlagData->m_flagType = editType;
        pFlagData->m_pLabels << pLabel;
        pFlagData->m_pWidget = pLineEdit;
        pFlagData->m_pChildData = NULL;
        pFlagData->m_pParentData = NULL;

        // Handle the parent/child case
        if (NULL != pParent)
        {
            pFlagData->m_flagType = kaProjectSettingFlagData::KA_FLAG_TYPE_LINE_EDIT_CHILD;
            pFlagData->m_pParentData = pParent;
            pParent->m_pChildData = pFlagData;
            EnableChild(pFlagData, false);
        }


        m_widgetFlagDataMap[pLineEdit] = pFlagData;
        retData = pFlagData;

        // Add the items to the grid:
        int numRows = pLayout->rowCount();
        pLayout->addWidget(pLabel, numRows, 0, 1, 1);
        pLayout->addWidget(pLineEdit, numRows, 1, 1, EditBoxColumnSpan());


        connect(pLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(OnTextChange(const QString&)));
    }

    return retData;
}

// ---------------------------------------------------------------------------
int kaProjectSettingsExtensionBase::EditBoxColumnSpan()
{
    return KA_TEXT_EDIT_BOX_COLUMN_SPAN;
}

// ---------------------------------------------------------------------------
QString kaProjectSettingsExtensionBase::GetBuildOptionString()
{
    QString stringToParse;

    kaProjectDataManager* pManager = qobject_cast<kaProjectDataManager*>(sender());

    if (NULL != pManager)
    {
        stringToParse = pManager->BuildOptions();
    }

    return stringToParse;
}

// ---------------------------------------------------------------------------
void kaProjectSettingsExtensionBase::OnCommandTextChanged()
{
    QTextEdit* pTextEdit = qobject_cast<QTextEdit*>(sender());
    QString stringToParse;

    bool shouldUpdate = false;

    // Check if the event was because of the clicking on the edit:
    if (NULL != pTextEdit)
    {
        stringToParse = pTextEdit->toPlainText();
        shouldUpdate = true;
    }
    else
    {
        // check if it is because of sync
        stringToParse = GetBuildOptionString();

        if (stringToParse != m_pCommandTextEdit->toPlainText())
        {
            m_pCommandTextEdit->setPlainText(stringToParse);
        }
    }

    // if not in external update of text then handle the update
    if (!m_updatingCommandText && shouldUpdate)
    {
        // take the text and parse it to the different components
        QStringList optionsList = stringToParse.split(" ", QString::SkipEmptyParts);

        JoinJoinedComboBoxes(optionsList);

        JoinBrackets(optionsList);

        // Pass on all the flags and look in the options list if it is there
        gtMap<QWidget*, kaProjectSettingFlagData*>::iterator flagDataIterator;

        for (flagDataIterator = m_widgetFlagDataMap.begin(); flagDataIterator != m_widgetFlagDataMap.end(); flagDataIterator++)
        {
            kaProjectSettingFlagData* pFlagData = (*flagDataIterator).second;

            GT_IF_WITH_ASSERT(NULL != pFlagData)
            {
                // if the flag is set  mark it
                // if not then disable it (if checkbox) or set to default (if combobox)
                // edit box are special case that handled in the end
                if (kaProjectSettingFlagData::KA_FLAG_TYPE_CHECK_BOX == pFlagData->m_flagType)
                {
                    HandleCheckBoxInText(pFlagData, optionsList);
                }
                else if (kaProjectSettingFlagData::KA_FLAG_TYPE_COMBO_BOX == pFlagData->m_flagType || kaProjectSettingFlagData::KA_FLAG_TYPE_COMBO_BOX_JOINED == pFlagData->m_flagType)
                {
                    HandleComboBoxInText(pFlagData, optionsList);
                }
                else if (kaProjectSettingFlagData::KA_FLAG_TYPE_LINE_EDIT == pFlagData->m_flagType)
                {
                    HandleTextEditInText(pFlagData, optionsList);
                }
                else if (kaProjectSettingFlagData::KA_FLAG_TYPE_LINE_EDIT_CHILD == pFlagData->m_flagType)
                {
                    // in the function there is a check for the child case:
                    HandleSingleLineTextInText(pFlagData, optionsList);
                }
                else if (kaProjectSettingFlagData::KA_FLAG_TYPE_LINE_EDIT_SINGLE == pFlagData->m_flagType)
                {
                    HandleSingleLineTextInText(pFlagData, optionsList);
                }
            }
        }

        UpdateProjectDataManagerWithTBOptions(stringToParse, optionsList);

    }
    else if (m_updatingCommandText)
    {
        UpdateProjectDataManagerWithTBOptions(stringToParse);

    }
}

void kaProjectSettingsExtensionBase::UpdateProjectDataManagerWithTBOptions(const QString& value)
{
    if (m_isBasicBuildOption)
    {
        // If just updating the text make sure that the project manager text is also updated and it will also update the
        // second text box in the second setting page
        QString oldCommands = KA_PROJECT_DATA_MGR_INSTANCE.BuildOptions();

        if (oldCommands != value)
        {
            KA_PROJECT_DATA_MGR_INSTANCE.setBuildOptions(value);
        }
    }
    else
    {
        QString oldCommands = KA_PROJECT_DATA_MGR_INSTANCE.ShaderBuildOptions();

        if (oldCommands != value)
        {
            KA_PROJECT_DATA_MGR_INSTANCE.SetShaderBuildOptions(value);
        }
    }
}

// ---------------------------------------------------------------------------
void kaProjectSettingsExtensionBase::JoinJoinedComboBoxes(QStringList& optionsList)
{
    int numJoinedString = m_joinedVectorStrings.size();
    {
        for (int nString = 0; nString < numJoinedString; nString++)
        {
            QString joinedString = m_joinedVectorStrings.at(nString);
            int joinedLocation = optionsList.indexOf(joinedString);

            if (joinedLocation != -1 && joinedLocation != optionsList.count() - 1)
            {
                // Look at the next string and join it if it is not another option (starts with '-')
                if (!optionsList.at(joinedLocation + 1).startsWith('-'))
                {
                    // join the two words into one option:
                    joinedString = joinedString + " " + optionsList.at(joinedLocation + 1);
                    optionsList[joinedLocation] = joinedString;
                    optionsList.removeAt(joinedLocation + 1);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaProjectSettingsExtensionBase::JoinBrackets(QStringList& optionsList)
{
    bool foundBrackets = false;

    for (int nString = 0; nString < optionsList.size(); nString++)
    {
        QString currentString = optionsList[nString];

        if (!foundBrackets)
        {
            // look for a starting brackets
            if (currentString.startsWith('\"') && !currentString.endsWith('\"'))
            {
                foundBrackets = true;
            }
        }
        else
        {
            // join words until an ending brackets
            if (nString > 0)
            {
                QString newString = optionsList[nString - 1] + " " + optionsList[nString];
                optionsList[nString - 1] = newString;

                if (optionsList[nString].endsWith('\"'))
                {
                    foundBrackets = false;
                }

                optionsList.removeAt(nString);
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaProjectSettingsExtensionBase::AddBrackets(QString& stringToAddBrackets)
{
    // check if there is a space or starts with '-' then add brackets:
    if (stringToAddBrackets.indexOf(" ") != -1 || stringToAddBrackets.startsWith('-'))
    {
        if (!stringToAddBrackets.startsWith('\"'))
        {
            stringToAddBrackets = '\"' + stringToAddBrackets;
        }

        if (!stringToAddBrackets.endsWith('\"'))
        {
            stringToAddBrackets = stringToAddBrackets + '\"';
        }
    }
}

// ---------------------------------------------------------------------------
int kaProjectSettingsExtensionBase::GetFlagIndex()
{
    return 0;
}

// ---------------------------------------------------------------------------
void kaProjectSettingsExtensionBase::HandleCheckBoxInText(kaProjectSettingFlagData* pFlagData, QStringList& optionsList)
{
    // If there is a child then it handles enabling of the parent checkbox correctly
    if (NULL == pFlagData->m_pChildData)
    {
        // check if the string is found:
        bool foundFlag = false;
        int flagIndex = GetFlagIndex();

        int foundString = -1;

        if (flagIndex < pFlagData->m_flagStringList.count())
        {
            foundString = optionsList.indexOf(pFlagData->m_flagStringList[flagIndex]);
        }

        if (foundString != -1)
        {
            foundFlag = true;
            // Remove it found the list:
            optionsList.removeAt(foundString);
        }

        // enable/disable the label connected to the checkbox:
        foreach (QLabel* pLabel, pFlagData->m_pLabels)
        {
            pLabel->setEnabled(foundFlag);
        }

        QCheckBox* pCheckBox = qobject_cast<QCheckBox*>(pFlagData->m_pWidget);
        GT_IF_WITH_ASSERT(NULL != pCheckBox)
        {
            pCheckBox->setChecked(foundFlag);
        }
    }
}

// ---------------------------------------------------------------------------
void kaProjectSettingsExtensionBase::HandleComboBoxInText(kaProjectSettingFlagData* pFlagData, QStringList& optionsList)
{
    int foundString = -1;
    int foundIndex = 0;
    int numStrings = pFlagData->m_flagStringList.count();

    // Start from 1 and not from 0 since the 0 contains the "default" empty string
    for (int nString = 1; nString < numStrings; nString++)
    {
        foundString = optionsList.indexOf(pFlagData->m_flagStringList[nString]);

        if (foundString != -1)
        {
            foundIndex = nString;
            optionsList.removeAt(foundString);
            break;
        }
    }

    QComboBox* pComboBox = qobject_cast<QComboBox*>(pFlagData->m_pWidget);
    GT_IF_WITH_ASSERT(NULL != pComboBox)
    {
        pComboBox->setCurrentIndex(foundIndex);
        pFlagData->m_previousFlagString = pFlagData->m_flagStringList[foundIndex];
    }
}

// ---------------------------------------------------------------------------
void kaProjectSettingsExtensionBase::HandleTextEditInText(kaProjectSettingFlagData* pFlagData, QStringList& optionsList)
{
    QString finalText;
    QString storedText;

    int foundString;

    do
    {
        foundString = optionsList.indexOf(pFlagData->m_flagStringList[0]);

        if (foundString != -1)
        {
            // Remove it found the list:
            optionsList.removeAt(foundString);

            // Check that there are strings left (-D was not the only string) and that -D was not the last string
            // and that the next parameter is not and option
            if ((foundString < optionsList.count()) && (!optionsList.at(foundString).startsWith('-')))
            {
                // add it as an option, if it is not the first one separate it with ';' and remove it from the list of strings
                if (!finalText.isEmpty())
                {
                    finalText.append(";");
                }

                // prepare the string to be stored as the previous string
                if (!storedText.isEmpty())
                {
                    storedText += " ";
                }

                QString foundItem = optionsList.at(foundString);

                // remove the brackets from the strings:
                if (foundItem.startsWith('\"'))
                {
                    // This will remove all brackets even the ones in the middle of a words that are not legal to begin with.
                    foundItem.remove('\"');
                }

                storedText += pFlagData->m_flagStringList[0] + " " + foundItem;

                finalText += foundItem;
                optionsList.removeAt(foundString);
            }
        }
    }
    while (foundString != -1);

    // set the final text
    QLineEdit* pLineEdit = qobject_cast<QLineEdit*>(pFlagData->m_pWidget);
    GT_IF_WITH_ASSERT(NULL != pLineEdit)
    {
        pLineEdit->blockSignals(true);
        pLineEdit->setText(finalText);
        pLineEdit->blockSignals(false);
    }

    pFlagData->m_previousFlagString = storedText;
}

// ---------------------------------------------------------------------------
void kaProjectSettingsExtensionBase::HandleSingleLineTextInText(kaProjectSettingFlagData* pFlagData, QStringList& optionsList)
{
    QString finalText;
    QString storedText;

    bool foundString = false;
    int numStrings = optionsList.count();

    // Start from 1 and not from 0 since the 0 contains the "default" empty string
    for (int nString = 0; nString < numStrings; nString++)
    {
        QString equalOption = pFlagData->m_flagStringList[0] + "=";

        if (optionsList[nString].startsWith(equalOption) || (optionsList[nString] == pFlagData->m_flagStringList[0]))
        {
            storedText = optionsList[nString];
            optionsList.removeAt(nString);
            break;
        }
    }

    // if we found a text check it has "=" to make it a valid flag and then use it:
    if (!storedText.isEmpty())
    {
        foundString = true;
        int equalPos = storedText.indexOf("=");

        if (equalPos != -1 && equalPos != storedText.length() - 1)
        {
            finalText = storedText.mid(equalPos + 1);
        }
        else
        {
            finalText = "";
        }

        QLineEdit* pLineEdit = qobject_cast<QLineEdit*>(pFlagData->m_pWidget);
        GT_IF_WITH_ASSERT(NULL != pLineEdit)
        {
            pLineEdit->blockSignals(true);
            pLineEdit->setText(finalText);
            pLineEdit->blockSignals(false);
        }
        pFlagData->m_previousFlagString = storedText;
    }

    // If the flag is found and it is the KA_FLAG_TYPE_LINE_EDIT_CHILD enable/disable child/parent
    if (kaProjectSettingFlagData::KA_FLAG_TYPE_LINE_EDIT_CHILD == pFlagData->m_flagType)
    {
        pFlagData->m_pWidget->setEnabled(foundString);

        foreach (QLabel* pLabel, pFlagData->m_pLabels)
        {
            pLabel->setEnabled(foundString);
        }

        // Since the text field is enabled we must enable the parent: -xxx=yyy
        // the -xxx will not be identified by the parent correctly
        GT_IF_WITH_ASSERT(NULL != pFlagData->m_pParentData)
        {
            foreach (QLabel* pLabel, pFlagData->m_pParentData->m_pLabels)
            {
                pLabel->setEnabled(foundString);
            }

            QCheckBox* pCheckBox = qobject_cast<QCheckBox*>(pFlagData->m_pParentData->m_pWidget);
            GT_IF_WITH_ASSERT(NULL != pCheckBox)
            {
                pCheckBox->setChecked(foundString);
            }
        }
    }

}

// ---------------------------------------------------------------------------
void kaProjectSettingsExtensionBase::OnCheckBoxClicked()
{
    // Get the control that initiated the event:
    QCheckBox* pCheckBox = qobject_cast<QCheckBox*>(sender());
    GT_IF_WITH_ASSERT(NULL != pCheckBox)
    {
        kaProjectSettingFlagData* pFlagData = m_widgetFlagDataMap[pCheckBox];
        GT_IF_WITH_ASSERT(NULL != pFlagData)
        {
            if (pCheckBox->isChecked())
            {
                // Highlight the label
                foreach (QLabel* pLabel, pFlagData->m_pLabels)
                {
                    pLabel->setDisabled(false);
                }

                // In case of connected text enable it
                if (NULL != pFlagData->m_pChildData)
                {
                    EnableChild(pFlagData->m_pChildData, true);
                }

                // Add the label
                GT_IF_WITH_ASSERT(pFlagData->m_flagStringList.count() == m_checkboxFlagsNum)
                {
                    QString textToAdd = pFlagData->m_previousFlagString;
                    AddString(textToAdd);
                }
            }
            else
            {
                // Dim the label
                foreach (QLabel* pLabel, pFlagData->m_pLabels)
                {
                    pLabel->setDisabled(true);
                }

                // In case of connected text disable it
                if (NULL != pFlagData->m_pChildData)
                {
                    EnableChild(pFlagData->m_pChildData, false);
                }

                // Remove the label
                GT_IF_WITH_ASSERT(pFlagData->m_flagStringList.count() == m_checkboxFlagsNum)
                {
                    QString textToRemove = pFlagData->m_previousFlagString;
                    RemoveString(textToRemove);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaProjectSettingsExtensionBase::OnComboBoxChanged(int index)
{
    // Get the control that initiated the event:
    QComboBox* pComboBox = qobject_cast<QComboBox*>(sender());
    GT_IF_WITH_ASSERT(NULL != pComboBox)
    {
        kaProjectSettingFlagData* pFlagData = m_widgetFlagDataMap[pComboBox];
        GT_IF_WITH_ASSERT(NULL != pFlagData)
        {
            QString textToShow = pFlagData->m_flagStringList[index];

            ReplaceText(pFlagData, textToShow);
        }
    }
}

// ---------------------------------------------------------------------------
void kaProjectSettingsExtensionBase::OnTextChange(const QString& lineText)
{
    GT_UNREFERENCED_PARAMETER(lineText);

    // Get the control that initiated the event:
    QLineEdit* pLineEdit = qobject_cast<QLineEdit*>(sender());
    GT_IF_WITH_ASSERT(NULL != pLineEdit)
    {
        kaProjectSettingFlagData* pFlagData = m_widgetFlagDataMap[pLineEdit];
        GT_IF_WITH_ASSERT(NULL != pFlagData)
        {
            QString newText;

            if (kaProjectSettingFlagData::KA_FLAG_TYPE_LINE_EDIT == pFlagData->m_flagType)
            {
                GT_IF_WITH_ASSERT(pFlagData->m_flagStringList.count() == 1)
                {
                    // If this is a regular text edit then parse sections separated by ';'
                    QStringList itemsList = pLineEdit->text().split(";");
                    int numItems = itemsList.count();

                    for (int nString = 0; nString < numItems; nString++)
                    {
                        if (!itemsList[nString].isEmpty())
                        {
                            AddBrackets(itemsList[nString]);
                            newText = newText + pFlagData->m_flagStringList[0] + " " + itemsList[nString];

                            if (nString != numItems - 1)
                            {
                                newText.append(" ");
                            }
                        }
                    }

                    ReplaceSplitText(pFlagData, newText);
                }
            }
            else if (kaProjectSettingFlagData::KA_FLAG_TYPE_LINE_EDIT_SINGLE == pFlagData->m_flagType)
            {
                newText = "";

                if (!pLineEdit->text().isEmpty())
                {
                    newText = pFlagData->m_flagStringList[0] + "=" + pLineEdit->text();
                }

                ReplaceText(pFlagData, newText);
            }
            else if (kaProjectSettingFlagData::KA_FLAG_TYPE_LINE_EDIT_CHILD == pFlagData->m_flagType)
            {
                // else add the value to the flag: flag="value"
                GT_IF_WITH_ASSERT(pFlagData->m_flagStringList.count() == 1)
                {
                    newText = pFlagData->m_flagStringList[0];
                    QString lineText1 = pLineEdit->text();

                    if (!lineText1.isEmpty())
                    {
                        AddBrackets(lineText1);
                        newText = newText + "=" + lineText1;
                    }
                }
                // We use the parent flag data old string from this point in order to have
                // just one string changing:
                kaProjectSettingFlagData* pUsedFlagData = pFlagData->m_pParentData;;
                ReplaceText(pUsedFlagData, newText);
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaProjectSettingsExtensionBase::ReplaceText(kaProjectSettingFlagData* pFlagData, const QString& newText)
{
    GT_IF_WITH_ASSERT(NULL != pFlagData)
    {
        // add/replace the new text:
        if (pFlagData->m_previousFlagString.isEmpty())
        {
            AddString(newText);
        }
        else if (!newText.isEmpty())
        {
            ReplaceString(pFlagData->m_previousFlagString, newText);
        }
        else
        {
            RemoveString(pFlagData->m_previousFlagString);
        }

        pFlagData->m_previousFlagString = newText;
    }
}

// ---------------------------------------------------------------------------
void kaProjectSettingsExtensionBase::ReplaceSplitText(kaProjectSettingFlagData* pFlagData, const QString& newText)
{
    GT_IF_WITH_ASSERT(NULL != pFlagData && pFlagData->m_flagStringList.count() > 0)
    {
        QString currentText = m_pCommandTextEdit->toPlainText();
        int flagPosition = -1;
        QString searchText = pFlagData->m_flagStringList[0];
        int searchLength = pFlagData->m_flagStringList[0].length();

        do
        {
            flagPosition = currentText.indexOf(searchText);

            if (flagPosition != -1)
            {
                // check that there is a space after:
                if ((currentText.length() > flagPosition + searchLength) && (currentText.at(flagPosition + searchLength) == ' '))
                {
                    // continue until non space is reached
                    int currentPos = flagPosition + searchLength;

                    while (currentPos < currentText.length() && currentText.at(currentPos) == ' ')
                    {
                        currentPos++;
                    }

                    // check the first character found and see if it is a flag marker, if not treat it as part of the flag and look for the end of it
                    if (currentPos < currentText.length() && currentText.at(currentPos) != '-' && currentText.at(currentPos) != '\"')
                    {
                        while (currentPos < currentText.length() && currentText.at(currentPos) != ' ')
                        {
                            currentPos++;
                        }
                    }
                    // check if it is a part of a parameter in bracket
                    else if (currentPos < currentText.length() && currentText.at(currentPos++) == '\"')
                    {
                        // look for the ending bracket
                        while (currentPos < currentText.length() && currentText.at(currentPos) != '\"')
                        {
                            currentPos++;
                        }

                        // if it is possible include the enclosing bracket
                        if (currentPos < currentText.length())
                        {
                            currentPos++;
                        }
                    }

                    // Remove the text
                    currentText.remove(flagPosition, currentPos - flagPosition);
                }
                else
                {
                    if (!(currentText.length() > flagPosition + searchLength))
                    {
                        currentText.remove(flagPosition, searchLength);
                    }
                }
            }
        }
        while (flagPosition != -1);

        while (currentText.indexOf("  ") != -1)
        {
            currentText.replace("  ", " ");
        }

        m_pCommandTextEdit->blockSignals(true);
        m_pCommandTextEdit->setText(currentText);
        m_pCommandTextEdit->blockSignals(false);
        AddString(newText);
    }
}

// ---------------------------------------------------------------------------
void kaProjectSettingsExtensionBase::AddString(const QString& stringToAdd)
{
    QString currentText = m_pCommandTextEdit->toPlainText();

    if (!stringToAdd.isEmpty() && currentText.indexOf(stringToAdd) == -1)
    {
        if (!currentText.isEmpty())
        {
            currentText.append(" ");
        }

        currentText.append(stringToAdd);
        m_updatingCommandText = true;
        m_pCommandTextEdit->setPlainText(currentText);
        m_updatingCommandText = false;
    }
}

// ---------------------------------------------------------------------------
void kaProjectSettingsExtensionBase::RemoveString(const QString& stringToRemove)
{
    QString currentText = m_pCommandTextEdit->toPlainText();

    if (!stringToRemove.isEmpty() && currentText.indexOf(stringToRemove) != -1)
    {
        currentText.remove(stringToRemove);
        // This might cause duplicate spaces so remove them
        currentText.replace("  ", " ");

        // also check if there is a space as first character and remove it
        if ((currentText.count() > 0) && (' ' == currentText[0]))
        {
            currentText.remove(0, 1);
        }

        m_updatingCommandText = true;
        m_pCommandTextEdit->setPlainText(currentText);
        m_updatingCommandText = false;
    }
}

// ---------------------------------------------------------------------------
void kaProjectSettingsExtensionBase::ReplaceString(const QString& stringToFind, const QString& stringToReplace)
{
    QString currentText = m_pCommandTextEdit->toPlainText();

    if (!stringToFind.isEmpty())
    {
        if (currentText.indexOf(stringToFind) != -1)
        {
            currentText.replace(stringToFind, stringToReplace);
            m_updatingCommandText = true;
            m_pCommandTextEdit->setPlainText(currentText);
            m_updatingCommandText = false;
        }
        else
        {
            // if not found add it
            AddString(stringToReplace);
        }
    }
}

// ---------------------------------------------------------------------------
void kaProjectSettingsExtensionBase::EnableChild(kaProjectSettingFlagData* pData, bool mode)
{
    GT_IF_WITH_ASSERT(NULL != pData)
    {
        // Label can be NULL
        foreach (QLabel* pLabel, pData->m_pLabels)
        {
            pLabel->setEnabled(mode);
        }

        GT_IF_WITH_ASSERT(NULL != pData->m_pWidget)
        {
            pData->m_pWidget->setEnabled(mode);
        }
    }
}
