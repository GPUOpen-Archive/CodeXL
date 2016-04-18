//------------------------------ kaCreateProgramDialog.cpp ------------------------------
// Warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>


// Infra:
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acTreeCtrl.h>
#include <AMDTApplicationComponents/Include/acValidators.h>
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>
#include <AMDTApplicationFramework/Include/afAidFunctions.h>

// Local:
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>
#include <AMDTKernelAnalyzer/src/kaCreateProgramDialog.h>

const QString SPACER("   ");

const int MIN_DESCRIPTION_LABEL_WIDTH = 340;
const int MIN_DESCRIPTION_LABEL_HEIGHT = 70;
const int DLG_FIX_WIDTH = 400;
const int DLG_FIX_HEIGHT = 400;

// ----------------------------------------------------------------------------------
// Class Name:           kaCreateProgramDialog::kaCreateProgramDialog(QWidget *parent)
// General Description:  Constructor.
// Author:               Gilad Yarnitzky
// Creation Date:        14/1/2015
// ----------------------------------------------------------------------------------
kaCreateProgramDialog::kaCreateProgramDialog(QWidget* parent) : QDialog(parent),
    m_pButtonBox(nullptr)
{

    CreateLayout();

    setWindowFlags(Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setWindowTitle(KA_STR_CreateNewProgramDlgTitle);
    // Set the dialog icon:
    afLoadTitleBarIcon(this);

}

// ---------------------------------------------------------------------------
// Name:        ~kaCreateProgramDialog
// ---------------------------------------------------------------------------
kaCreateProgramDialog::~kaCreateProgramDialog()
{
}
void kaCreateProgramDialog::CreateLayout()
{
    // Create the layouts:
    m_pMainLayout = new QVBoxLayout;

    QFormLayout* pTypeSelectionLayout = new QFormLayout;
    QPalette p = palette();
    p.setColor(backgroundRole(), Qt::white);
    p.setColor(QPalette::Base, Qt::white);
    setPalette(p);

    m_pMainLayout->addLayout(pTypeSelectionLayout);

    pTypeSelectionLayout->addRow(new QLabel(KA_STR_SelectProgramType));

    m_pTypeRadioGroup = new QButtonGroup();

    // add all radio buttons
    QRadioButton* pRadioButton = nullptr;
    pTypeSelectionLayout->addRow(new QLabel(KA_STR_ProgramTypeVulkan));

    pRadioButton = new QRadioButton(KA_STR_ProgramTypeRendering);
    m_typeRadioButtonsVector << pRadioButton;
    m_pTypeRadioGroup->addButton(pRadioButton);
    pTypeSelectionLayout->addRow(new QLabel(SPACER), pRadioButton);

    pRadioButton = new QRadioButton(KA_STR_ProgramTypeCompute);
    m_typeRadioButtonsVector << pRadioButton;
    m_pTypeRadioGroup->addButton(pRadioButton);
    pTypeSelectionLayout->addRow(new QLabel(SPACER), pRadioButton);

    pTypeSelectionLayout->addRow(new QLabel(KA_STR_ProgramTypeOpenGL));

    pRadioButton = new QRadioButton(KA_STR_ProgramTypeRendering);
    m_typeRadioButtonsVector << pRadioButton;
    m_pTypeRadioGroup->addButton(pRadioButton);
    pTypeSelectionLayout->addRow(new QLabel(SPACER), pRadioButton);

    pRadioButton = new QRadioButton(KA_STR_ProgramTypeCompute);
    m_typeRadioButtonsVector << pRadioButton;
    m_pTypeRadioGroup->addButton(pRadioButton);
    pTypeSelectionLayout->addRow(new QLabel(SPACER), pRadioButton);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    pTypeSelectionLayout->addRow(new QLabel(KA_STR_ProgramTypeDirectX));

    pRadioButton = new QRadioButton(KA_STR_ProgramTypeDirectXFolder);
    m_typeRadioButtonsVector << pRadioButton;
    m_pTypeRadioGroup->addButton(pRadioButton);
    pTypeSelectionLayout->addRow(new QLabel(SPACER), pRadioButton);

#endif

    pTypeSelectionLayout->addRow(new QLabel(KA_STR_ProgramTypeOpenCL));

    pRadioButton = new QRadioButton(KA_STR_ProgramTypeOpenCL1_2);
    m_typeRadioButtonsVector << pRadioButton;
    m_pTypeRadioGroup->addButton(pRadioButton);
    pTypeSelectionLayout->addRow(new QLabel(SPACER), pRadioButton);

    bool rc = connect(m_pTypeRadioGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(OnTypeSelectionChanged(QAbstractButton*)));
    GT_ASSERT(rc);

    pTypeSelectionLayout->addRow(new QLabel(KA_STR_DescriptionTitle));

    // create vector with all descriptions according to radio buttons order
    m_descriptionsVector << KA_STR_VulkanRenderingProgramDescription;
    m_descriptionsVector << KA_STR_VulkanComputeProgramDescription;
    m_descriptionsVector << KA_STR_OpenGLRenderingProgramDescription;
    m_descriptionsVector << KA_STR_OpenGLComputeProgramDescription;
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    m_descriptionsVector << KA_STR_DirectXFolderDescription;
#endif
    m_descriptionsVector << KA_STR_OpenCLFolderDescription;
    m_pDescriptionLabel = new QLabel(KA_STR_VulkanRenderingProgramDescription);
    m_pDescriptionLabel->setTextFormat(Qt::RichText);
    m_pDescriptionLabel->setStyleSheet(QString("QLabel { border: 2px solid gray;  border-radius: 2px; padding: 2px; qproperty-alignment: AlignLeft;}"));


    m_pDescriptionLabel->setWordWrap(true);
    m_pDescriptionLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_pDescriptionLabel->setScaledContents(true);
    QSize minDescSize(acScalePixelSizeToDisplayDPI(MIN_DESCRIPTION_LABEL_WIDTH), acScalePixelSizeToDisplayDPI(MIN_DESCRIPTION_LABEL_HEIGHT));
    m_pDescriptionLabel->setMinimumSize(minDescSize);

    pTypeSelectionLayout->addRow(m_pDescriptionLabel);
    // Add the OK + Cancel buttons:
    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    m_pButtonBox->setCenterButtons(true);

    // Connect the buttons to slots:
    rc = connect(m_pButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
    GT_ASSERT(rc);
    rc = connect(m_pButtonBox, SIGNAL(rejected()), this, SLOT(reject()));
    GT_ASSERT(rc);
    m_pMainLayout->addWidget(m_pButtonBox, 0, Qt::AlignRight);

    setLayout(m_pMainLayout);

    // set default type selected
    (m_typeRadioButtonsVector[0])->toggle();
    OnTypeSelectionChanged(m_typeRadioButtonsVector[0]);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QSize fixedSize(acScalePixelSizeToDisplayDPI(DLG_FIX_WIDTH), acScalePixelSizeToDisplayDPI(DLG_FIX_HEIGHT));
    setFixedSize(fixedSize);
}

// ---------------------------------------------------------------------------
void kaCreateProgramDialog::OnTypeSelectionChanged(QAbstractButton* pButton)
{
    QRadioButton* pRadioButton = static_cast<QRadioButton*>(pButton);
    GT_IF_WITH_ASSERT(NULL != pRadioButton)
    {
        // get toggeled radio index
        int index = m_typeRadioButtonsVector.indexOf(pRadioButton);

        GT_IF_WITH_ASSERT(index >= 0 && m_descriptionsVector.count() > index)
        {
            // save the index of the toggeled radio
            m_togledRadioButtonIndex = index;

            // set description with same index
            m_pDescriptionLabel->setText(m_descriptionsVector[index]);
            m_pDescriptionLabel->updateGeometry();

            switch (index)
            {
                case 0:
                    m_programType = kaProgramVK_Rendering;
                    break;

                case 1:
                    m_programType = kaProgramVK_Compute;
                    break;

                case 2:
                    m_programType = kaProgramGL_Rendering;
                    break;

                case 3:
                    m_programType = kaProgramGL_Compute;
                    break;
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

                case 4:
                    m_programType = kaProgramDX;
                    break;

                case 5:
                    m_programType = kaProgramCL;
                    break;
#else

                case 4:
                    m_programType = kaProgramCL;
                    break;
#endif

                default:
                    break;
            }
        }
    }
}
