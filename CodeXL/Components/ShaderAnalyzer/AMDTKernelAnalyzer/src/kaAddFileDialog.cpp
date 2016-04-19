//------------------------------ kaAddFileDialog.cpp ------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <QPixmap>
#include <QVariant>
#include <QFormLayout>

// Infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTApplicationFramework/src/afUtils.h>
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afCSSSettings.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acIcons.h>

// Local:
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>
#include <AMDTKernelAnalyzer/src/kaAddFileDialog.h>
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaDataTypes.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaUtils.h>

// The maximum lines count for the file descriptions. Used for calculating the minimum height for the description label (to avoid scrolling):
#define KA_FILE_DIALOG_DESCRIPTION_LINES_COUNT 10
const QSize DLG_FIX_SIZE(440, 470);
// ----------------------------------------------------------------------------------
// Class Name:           kaAddFileDialog::kaAddFileDialog(QWidget *parent)
// General Description:  Constructor.
// Author:               Gilad Yarnitzky
// Creation Date:        14/1/2015
// ----------------------------------------------------------------------------------
kaAddFileDialog::kaAddFileDialog(QWidget* parent, kaProgram* pAssociatedProgram, kaPipelinedProgram::PipelinedStage stage) : QDialog(parent),
    m_pTypeCombo(nullptr), m_pNameLayout(nullptr), m_pFileName(nullptr), m_pMainLayout(nullptr), m_pFileTypesRadioButtonsGroup(nullptr),
    m_selectedRadioButtonIndex(0), m_pDescriptionLabel(nullptr), m_fileType(kaFileTypeUnknown), m_selectedProgramType(kaProgramUnknown),
    m_pAssociatedProgramCombo(nullptr), m_pAssociatedProgram(pAssociatedProgram), m_associatedProgramType(kaProgramUnknown),
    m_pPlatformSelectionCombo(nullptr), m_pFileTypesHeader(nullptr), m_pRadioButtonsLayout(nullptr)
{
    // Initialize the map containing the data for the radio buttons
    InitRadioButtonsMap();

    setWindowTitle(KA_STR_addFileTitle);

    CreateLayout(stage);

    setWindowFlags(Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    // Set the dialog icon:
    afLoadTitleBarIcon(this);
}

kaAddFileDialog::kaAddFileDialog(kaProgram* pAssociatedProgram, const kaProgramTypes programType, kaPipelinedProgram::PipelinedStage stage):
    QDialog(nullptr),
    m_pTypeCombo(nullptr), m_pNameLayout(nullptr), m_pFileName(nullptr), m_pMainLayout(nullptr),
    m_pFileTypesRadioButtonsGroup(nullptr),
    m_selectedRadioButtonIndex(0), m_pDescriptionLabel(nullptr), m_fileType(kaFileTypeUnknown),
    m_selectedProgramType(programType),
    m_pAssociatedProgramCombo(nullptr), m_pAssociatedProgram(pAssociatedProgram), m_associatedProgramType(programType),
    m_pPlatformSelectionCombo(nullptr), m_pFileTypesHeader(nullptr), m_pRadioButtonsLayout(nullptr)
{
    //Get file type based on program type and pipeline stage
    m_fileType = kaUtils::PipelineStageToFileType(programType, stage);

    // Initialize the map containing the data for the radio buttons
    InitRadioButtonsMap();
}

// ---------------------------------------------------------------------------
// Name:        ~kaAddFileDialog
// ---------------------------------------------------------------------------
kaAddFileDialog::~kaAddFileDialog()
{
}

void kaAddFileDialog::GetFileNameAndExtension(QString& fileName, QString& fileExtension)
{
    // Get the file name description as string
    QString fileDescriptionAsString;
    GT_IF_WITH_ASSERT(m_fileTypesDataMap.contains(m_selectedProgramType))
    {
        FileTypesList list = m_fileTypesDataMap[m_selectedProgramType];
        GT_IF_WITH_ASSERT(list.contains(m_fileType))
        {
            fileDescriptionAsString = list[m_fileType];
        }
    }

    GT_IF_WITH_ASSERT(!fileDescriptionAsString.isEmpty())
    {
        QStringList list = fileDescriptionAsString.split(" ");
        GT_IF_WITH_ASSERT(!list.isEmpty())
        {
            fileName = list[0];
        }

        int indexOfDot = fileDescriptionAsString.indexOf(".");
        int indexOfRightBrace = fileDescriptionAsString.indexOf(")");

        if ((indexOfDot != -1) && (indexOfRightBrace != -1))
        {
            fileExtension = fileDescriptionAsString.mid(indexOfDot + 1, indexOfRightBrace - indexOfDot - 1);
        }
    }
}

void kaAddFileDialog::CreateLayout(kaPipelinedProgram::PipelinedStage stage)
{
    // Create the layouts:
    m_pMainLayout = new QVBoxLayout;

    m_pMainLayout->addWidget(new QLabel(KA_STR_addFileTypeSelectionTitle));

    // set scrolling area widgets
    QVBoxLayout* pTypeSelectionLayout = new QVBoxLayout;
    QPalette p = palette();
    p.setColor(backgroundRole(), Qt::white);
    p.setColor(QPalette::Base, Qt::white);
    setPalette(p);

    m_pMainLayout->addLayout(pTypeSelectionLayout);

    QHBoxLayout* pPlatformSelectionLayout = new QHBoxLayout;
    pPlatformSelectionLayout->addWidget(new QLabel(KA_STR_addFileSelectPlatformTitle));
    m_pPlatformSelectionCombo = new QComboBox;
    pPlatformSelectionLayout->addWidget(m_pPlatformSelectionCombo);

    m_pPlatformSelectionCombo->addItem(KA_STR_platformOpenCL);
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    m_pPlatformSelectionCombo->addItem(KA_STR_platformDirectX);
#endif
    m_pPlatformSelectionCombo->addItem(KA_STR_platformOpenGL);
    m_pPlatformSelectionCombo->addItem(KA_STR_platformVulkan);

    bool rc = connect(m_pPlatformSelectionCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnPlatformChanged(int)));
    GT_ASSERT(rc);

    pTypeSelectionLayout->addLayout(pPlatformSelectionLayout);

    CreateFileTypesRadioButtons();
    rc = connect(m_pFileTypesRadioButtonsGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(OnFileTypeSelectionChanged(QAbstractButton*)));
    GT_ASSERT(rc);
    pTypeSelectionLayout->addLayout(m_pRadioButtonsLayout);

    QHBoxLayout* pProgramComboLayout = new QHBoxLayout;

    pProgramComboLayout->addWidget(new QLabel(KA_STR_addFileAssociateProgram));
    m_pAssociatedProgramCombo = new QComboBox;

    rc = connect(m_pAssociatedProgramCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnAssociatedProgramSelected(int)));
    GT_ASSERT(rc);

    pProgramComboLayout->addWidget(m_pAssociatedProgramCombo);
    pTypeSelectionLayout->addLayout(pProgramComboLayout);

    pTypeSelectionLayout->addWidget(new QLabel(KA_STR_addFileDescriptionTitle));
    m_pDescriptionLabel = new QLabel("");
    m_pDescriptionLabel->setWordWrap(true);
    m_pDescriptionLabel->setAlignment(Qt::AlignTop);
    pTypeSelectionLayout->addWidget(m_pDescriptionLabel, 1, Qt::AlignTop);

    // Calculate the height takes for the description to avoid scroll:
    int descHeight = QFontMetrics(m_pDescriptionLabel->font()).boundingRect(KA_STR_addFileHLSLDescription).height() * KA_FILE_DIALOG_DESCRIPTION_LINES_COUNT;
    m_pDescriptionLabel->setMinimumHeight(descHeight);

    // Add the OK + Cancel buttons:
    QDialogButtonBox* pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    pButtonBox->setCenterButtons(true);
    // Connect the buttons to slots:
    rc = connect(pButtonBox, SIGNAL(accepted()), this, SLOT(OnOk()));
    GT_ASSERT(rc);
    rc = connect(pButtonBox, SIGNAL(rejected()), this, SLOT(reject()));
    GT_ASSERT(rc);
    m_pMainLayout->addWidget(pButtonBox, 0, Qt::AlignRight);
    setLayout(m_pMainLayout);

    // Set the associated program
    SetupAssociatedProgram();

    // Select the default file type according to the reqeusted stage
    SelectDefaultFileType(stage);

    setFixedSize(DLG_FIX_SIZE);

    // Set the default selected file
    updateGeometry();
}

void kaAddFileDialog::OnOk()
{
    // If the user wanted to create a program, create it
    if (m_pAssociatedProgramCombo->currentText() == KA_STR_addFileProgramComboItemNewProgram)
    {
        // fixing 2011 - changing default rendering program to compute if needed
        if (m_pFileTypesRadioButtonsGroup != nullptr)
        {
            QString radioText = m_pFileTypesRadioButtonsGroup->checkedButton()->text();

            if (radioText.contains(KA_STR_addFileGLSLCompute))
            {
                if (m_selectedProgramType == kaProgramVK_Rendering)
                {
                    m_selectedProgramType = kaProgramVK_Compute;
                }
                else if (m_selectedProgramType == kaProgramGL_Rendering)
                {
                    m_selectedProgramType = kaProgramGL_Compute;
                }
            }
        }

        m_pAssociatedProgram = kaApplicationCommands::instance().CreateDefaultProgram(m_selectedProgramType);
    }

    // Accept
    accept();
}

void kaAddFileDialog::SelectDefaultFileType(kaPipelinedProgram::PipelinedStage stage)
{
    // Get the file type from the rendering stage
    kaFileTypes fileType = kaRenderingProgram::RenderStageToFileType(stage);

    if (fileType != kaFileTypeUnknown)
    {
        GT_IF_WITH_ASSERT((m_selectedProgramType == kaProgramVK_Rendering) || (m_selectedProgramType == kaProgramVK_Compute) ||
                          (m_selectedProgramType == kaProgramGL_Rendering) || (m_selectedProgramType == kaProgramGL_Compute))
        {
            GT_IF_WITH_ASSERT(m_pFileTypesRadioButtonsGroup != nullptr)
            {
                m_fileType = fileType;

                // Look for the radio button with this file type and select it
                GT_IF_WITH_ASSERT(m_fileTypesDataMap.contains(m_selectedProgramType))
                {
                    if (m_fileTypesDataMap[m_selectedProgramType].contains(m_fileType))
                    {
                        // Get the radio text from the map and look for the radio button
                        QString radioText = m_fileTypesDataMap[m_selectedProgramType][fileType];

                        const QList<QAbstractButton*>& radioButtons = m_pFileTypesRadioButtonsGroup->buttons();

                        for (int i = 0; i < (int)radioButtons.size(); i++)
                        {
                            QRadioButton* pRadio = qobject_cast<QRadioButton*>(radioButtons[i]);
                            GT_IF_WITH_ASSERT(pRadio != nullptr)
                            {
                                if (pRadio->text() == radioText)
                                {
                                    pRadio->setChecked(true);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        if ((m_selectedProgramType == kaProgramVK_Rendering) || (m_selectedProgramType == kaProgramGL_Rendering))
        {
            const QList<QAbstractButton*>& radioButtons = m_pFileTypesRadioButtonsGroup->buttons();

            for (int i = 0; i < (int)radioButtons.size(); i++)
            {
                QRadioButton* pRadio = qobject_cast<QRadioButton*>(radioButtons[i]);
                GT_IF_WITH_ASSERT(pRadio != nullptr)
                {
                    QString radioText = pRadio->text();

                    if (radioText.contains(KA_STR_addFileGLSLCompute) || radioText.contains(KA_STR_addFileGLSLGeneric))
                    {
                        pRadio->setDisabled(true);
                    }
                }
            }

        }
    }
}

// ---------------------------------------------------------------------------
void kaAddFileDialog::OnFileTypeSelectionChanged(QAbstractButton* pButton)
{
    QRadioButton* pRadioButton = static_cast<QRadioButton*>(pButton);
    GT_IF_WITH_ASSERT(nullptr != pRadioButton)
    {
        QString fileTypeText = pRadioButton->text();
        GT_IF_WITH_ASSERT(m_fileTypesDataMap.contains(m_selectedProgramType))
        {
            // Get the list of file types for the current program type
            const FileTypesList& currentList = m_fileTypesDataMap[m_selectedProgramType];

            // Go over the list and look for the file type with the selected text
            auto iter = currentList.begin();
            auto iterEnd = currentList.end();

            for (; iter != iterEnd; iter++)
            {
                if ((*iter) == fileTypeText)
                {
                    m_fileType = iter.key();
                }
            }

            // Set the description label
            GT_IF_WITH_ASSERT((m_fileTypesToDescriptionMap.contains(m_fileType)) && (m_pDescriptionLabel != nullptr))
            {
                m_pDescriptionLabel->setText(m_fileTypesToDescriptionMap[m_fileType]);
            }
        }
    }
}

void kaAddFileDialog::OnAssociatedProgramSelected(int index)
{
    GT_IF_WITH_ASSERT(m_pAssociatedProgramCombo != nullptr)
    {
        QVariant dataProgram = m_pAssociatedProgramCombo->itemData(index, Qt::UserRole);
        m_pAssociatedProgram = qvariant_cast<kaProgram*>(dataProgram);
    }
}

kaFileTypes kaAddFileDialog::SelectFirstAppropriateFileType(kaProgramTypes associatedProgramType)
{
    m_fileType = kaFileTypeUnknown;

    // Find the first file type for this program type
    GT_IF_WITH_ASSERT(m_fileTypesDataMap.contains(associatedProgramType))
    {
        m_fileType = m_fileTypesDataMap[associatedProgramType].begin().key();
    }
    return m_fileType;
}


kaProgramTypes kaAddFileDialog::GetProgramTypeByFileType(kaFileTypes fileType)
{
    kaProgramTypes retVal = kaProgramUnknown;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pPlatformSelectionCombo != nullptr)
    {
        QString chosenPlatform = m_pPlatformSelectionCombo->currentText();

        switch (fileType)
        {
            case kaFileTypeGLSLFrag:
            case kaFileTypeGLSLGeom:
            case kaFileTypeGLSLTese:
            case kaFileTypeGLSLTesc:
            case kaFileTypeGLSLVert:
            case kaFileTypeGLSLGenericShader:
                if (chosenPlatform == KA_STR_platformOpenGL)
                {
                    retVal = kaProgramGL_Rendering;
                }
                else
                {
                    retVal = kaProgramVK_Rendering;
                }

                break;

            case kaFileTypeGLSLComp:
                if (chosenPlatform == KA_STR_platformOpenGL)
                {
                    retVal = kaProgramGL_Compute;
                }
                else
                {
                    retVal = kaProgramVK_Compute;
                }

                break;

            case kaFileTypeDXCompute:
            case kaFileTypeDXDomain:
            case kaFileTypeDXGeometry:
            case kaFileTypeDXHull:
            case kaFileTypeDXPixel:
            case kaFileTypeDXVertex:
            case kaFileTypeDXGenericShader:
                retVal = kaProgramDX;
                break;

            case kaFileTypeOpenCL:
                retVal = kaProgramCL;
                break;

            default:
                break;
        }
    }

    return retVal;
}

void kaAddFileDialog::OnPlatformChanged(int currentIndex)
{
    kaProgramTypes prevSelection = m_selectedProgramType;

    if (m_pPlatformSelectionCombo != nullptr)
    {
        // Hide show relevant radio buttons
        m_selectedProgramType = kaProgramUnknown;
        QString chosenPlatform = m_pPlatformSelectionCombo->itemText(currentIndex);

        if (chosenPlatform == KA_STR_platformOpenCL)
        {
            m_selectedProgramType = kaProgramCL;
            m_pFileTypesHeader->setText(KA_STR_addFileOpenCLTypesTitle);
        }
        else if (chosenPlatform == KA_STR_platformOpenGL)
        {
            m_pFileTypesHeader->setText(KA_STR_addFileOpenGLTypesTitle);
            m_selectedProgramType = kaProgramGL_Rendering;
        }

        else if (chosenPlatform == KA_STR_platformDirectX)
        {
            m_pFileTypesHeader->setText(KA_STR_addFileDirectXTypesTitle);
            m_selectedProgramType = kaProgramDX;
        }

        else if (chosenPlatform == KA_STR_platformVulkan)
        {
            m_pFileTypesHeader->setText(KA_STR_addFileVulkanTypesTitle);
            m_selectedProgramType = kaProgramVK_Rendering;
        }


        GT_IF_WITH_ASSERT((m_selectedProgramType != kaProgramUnknown) && (m_pFileTypesRadioButtonsGroup != nullptr))
        {
            GT_IF_WITH_ASSERT(m_fileTypesDataMap.contains(m_selectedProgramType))
            {
                int checkedRadioIndex = -1;
                int radioIndex = 0;
                const QList<QAbstractButton*>& radioButtons = m_pFileTypesRadioButtonsGroup->buttons();
                const FileTypesList& list = m_fileTypesDataMap[m_selectedProgramType];
                auto iter = list.begin();
                auto iterEnd = list.end();

                for (; iter != iterEnd; iter++)
                {
                    GT_IF_WITH_ASSERT(radioIndex < radioButtons.size())
                    {
                        QRadioButton* pRadio = qobject_cast<QRadioButton*>(radioButtons[radioIndex]);
                        GT_IF_WITH_ASSERT(pRadio != nullptr)
                        {
                            if (pRadio->isChecked())
                            {
                                checkedRadioIndex = radioIndex;
                            }

                            pRadio->show();
                            pRadio->setText(*iter);
                            radioIndex++;
                        }
                    }
                }

                if ((checkedRadioIndex < 0) || (checkedRadioIndex >= list.size()))
                {
                    checkedRadioIndex = 0;
                }

                radioButtons[checkedRadioIndex]->setChecked(true);
                OnFileTypeSelectionChanged(radioButtons[checkedRadioIndex]);

                for (int i = radioIndex; i < radioButtons.size(); i++)
                {
                    radioButtons[i]->hide();
                }
            }
        }
    }

    // If the program type had changed, fill the programs combo
    if (prevSelection != m_selectedProgramType)
    {
        FillProgramsCombo();
    }
}

void kaAddFileDialog::InitRadioButtonsMap()
{
    QStringList typesStringList = QString(KA_STR_addFileCLTypes).split(AF_STR_CommaA);
    GT_IF_WITH_ASSERT(typesStringList.count() == 1)
    {
        FileTypesList clList;
        clList[kaFileTypeOpenCL] = typesStringList[0];
        m_fileTypesDataMap[kaProgramCL] = clList;
    }

    typesStringList = QString(KA_STR_addFileDXTypes).split(AF_STR_CommaA);
    GT_IF_WITH_ASSERT(typesStringList.count() == 7)
    {
        FileTypesList dxList;
        dxList[kaFileTypeDXVertex] = typesStringList[0];
        dxList[kaFileTypeDXHull] = typesStringList[1];
        dxList[kaFileTypeDXDomain] = typesStringList[2];
        dxList[kaFileTypeDXGeometry] = typesStringList[3];
        dxList[kaFileTypeDXPixel] = typesStringList[4];
        dxList[kaFileTypeDXCompute] = typesStringList[5];
        dxList[kaFileTypeDXGenericShader] = typesStringList[6];

        m_fileTypesDataMap[kaProgramDX] = dxList;
    }

    typesStringList = QString(KA_STR_addFileGLTypes).split(AF_STR_CommaA);
    GT_IF_WITH_ASSERT(typesStringList.count() == 7)
    {
        FileTypesList glList;
        glList[kaFileTypeGLSLFrag] = typesStringList[0];
        glList[kaFileTypeGLSLVert] = typesStringList[1];
        glList[kaFileTypeGLSLComp] = typesStringList[2];
        glList[kaFileTypeGLSLGeom] = typesStringList[3];
        glList[kaFileTypeGLSLTesc] = typesStringList[4];
        glList[kaFileTypeGLSLTese] = typesStringList[5];
        glList[kaFileTypeGLSLGenericShader] = typesStringList[6];

        m_fileTypesDataMap[kaProgramGL_Rendering] = glList;
        m_fileTypesDataMap[kaProgramGL_Compute] = glList;

        m_fileTypesDataMap[kaProgramVK_Rendering] = glList;
        m_fileTypesDataMap[kaProgramVK_Compute] = glList;
    }

    // Build the descriptions map
    m_fileTypesToDescriptionMap[kaFileTypeOpenCL] = KA_STR_addFileCLDescription;
    m_fileTypesToDescriptionMap[kaFileTypeDXVertex] = KA_STR_addFileVSDescription;
    m_fileTypesToDescriptionMap[kaFileTypeDXHull] = KA_STR_addFileHSDescription;
    m_fileTypesToDescriptionMap[kaFileTypeDXDomain] = KA_STR_addFileDSDescription;
    m_fileTypesToDescriptionMap[kaFileTypeDXGeometry] = KA_STR_addFileGSDescription;
    m_fileTypesToDescriptionMap[kaFileTypeDXPixel] = KA_STR_addFilePSDescription;
    m_fileTypesToDescriptionMap[kaFileTypeDXCompute] = KA_STR_addFileCSDescription;
    m_fileTypesToDescriptionMap[kaFileTypeDXGenericShader] = KA_STR_addFileHLSLDescription;
    m_fileTypesToDescriptionMap[kaFileTypeGLSLVert] = KA_STR_addFileVertDescription;
    m_fileTypesToDescriptionMap[kaFileTypeGLSLTesc] = KA_STR_addFileTESCDescription;
    m_fileTypesToDescriptionMap[kaFileTypeGLSLTese] = KA_STR_addFileTESEDescription;
    m_fileTypesToDescriptionMap[kaFileTypeGLSLGeom] = KA_STR_addFileGSDescription;
    m_fileTypesToDescriptionMap[kaFileTypeGLSLFrag] = KA_STR_addFileFSDescription;
    m_fileTypesToDescriptionMap[kaFileTypeGLSLComp] = KA_STR_addFileCSDescription;
    m_fileTypesToDescriptionMap[kaFileTypeGLSLGenericShader] = KA_STR_addFileGLSLDescription;
}

void kaAddFileDialog::SetupAssociatedProgram()
{
    int index = -1;

    if (m_pAssociatedProgram != nullptr)
    {
        m_fileType = SelectFirstAppropriateFileType(m_pAssociatedProgram->GetBuildType());

        switch (m_pAssociatedProgram->GetBuildType())
        {
            case kaProgramCL:
                index = 0;
                break;
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

            case kaProgramDX:
                index = 1;
                break;

            case kaProgramGL_Compute:
            case kaProgramGL_Rendering:
                index = 2;
                break;

            case kaProgramVK_Compute:
            case kaProgramVK_Rendering:
                index = 3;
                break;
#else

            case kaProgramGL_Compute:
            case kaProgramGL_Rendering:
                index = 1;
                break;

            case kaProgramVK_Compute:
            case kaProgramVK_Rendering:
                index = 2;
                break;
#endif

            default: // should not get here
                index = 0;
                break;
        }

        m_pPlatformSelectionCombo->setCurrentIndex(index);
        OnPlatformChanged(index);
        m_pPlatformSelectionCombo->setDisabled(true);
    }
    else
    {
        m_fileType = kaFileTypeOpenCL;
        OnPlatformChanged(0);
    }

}

void kaAddFileDialog::FillProgramsCombo()
{
    GT_IF_WITH_ASSERT(m_pAssociatedProgramCombo != nullptr)
    {
        if (m_pAssociatedProgram != nullptr)
        {
            QVariant programAsVariant = qVariantFromValue(m_pAssociatedProgram);
            m_pAssociatedProgramCombo->addItem(acGTStringToQString(m_pAssociatedProgram->GetProgramName()), programAsVariant);
            m_pAssociatedProgramCombo->setEnabled(false);
        }
        else
        {
            m_pAssociatedProgramCombo->clear();
            m_pAssociatedProgramCombo->addItem(KA_STR_addFileProgramComboItemNewProgram);
            gtVector<kaProgram*> programList = KA_PROJECT_DATA_MGR_INSTANCE.GetPrograms();

            for (kaProgram* pProgram : programList)
            {
                if (pProgram != nullptr)
                {
                    if (m_selectedProgramType == pProgram->GetBuildType())
                    {
                        QVariant programAsVairant = qVariantFromValue(pProgram);
                        m_pAssociatedProgramCombo->addItem(acGTStringToQString(pProgram->GetProgramName()), programAsVairant);
                    }
                }
            }

            m_pAssociatedProgramCombo->addItem(KA_STR_addFileProgramComboItemNone);
        }
    }
}

bool kaAddFileDialog::ShouldForceProgramCreation(kaProgramTypes& typeOfProgramToBeCreated)
{
    bool retVal = false;

    if (m_pAssociatedProgramCombo != nullptr)
    {
        if (m_pAssociatedProgramCombo->currentText() == KA_STR_addFileProgramComboItemNewProgram)
        {
            retVal = true;
            typeOfProgramToBeCreated = GetProgramTypeByFileType(m_fileType);
        }
    }

    return retVal;
}

void kaAddFileDialog::CreateFileTypesRadioButtons()
{
    // Find the amount of the radio buttons that should be created
    int radioButtonsCount = 1;

    for (auto iter = m_fileTypesDataMap.begin(); iter != m_fileTypesDataMap.end(); iter++)
    {
        radioButtonsCount = qMax(radioButtonsCount, (*iter).count());
    }

    m_pRadioButtonsLayout = new QFormLayout;
    QRadioButton* pRadioButton;
    QString option;
    QStringList typeStringList;

    // Create the file types header
    m_pFileTypesHeader = new QLabel(KA_STR_addFileOpenCLTypesTitle);
    m_pRadioButtonsLayout->addRow(m_pFileTypesHeader);

    m_pFileTypesRadioButtonsGroup = new QButtonGroup;

    for (int i = 0; i < radioButtonsCount; i++)
    {

        pRadioButton = new QRadioButton;
        m_pRadioButtonsLayout->addRow(pRadioButton);
        m_pFileTypesRadioButtonsGroup->addButton(pRadioButton);
    }
}

QString kaAddFileDialog::GetSelectedPlatform()const
{
    QString ret;

    if (m_pAssociatedProgram != nullptr)
    {
        kaProgramTypes programType =  m_pAssociatedProgram->GetBuildType();
        ret = kaUtils::ProgramTypeToPlatformString(programType);
    }
    else if (m_pPlatformSelectionCombo != nullptr)
    {
        return m_pPlatformSelectionCombo->currentText();
    }

    return ret;
}

