//------------------------------ kaExportBinariesDialog.cpp ------------------------------

// Qt
#include <QtWidgets>

// infra:
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// framework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/src/afUtils.h>


// Local:
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>
#include <AMDTKernelAnalyzer/src/kaExportBinariesDialog.h>


//------------------- ctor ----------------------------------------
kaExportBinariesDialog::kaExportBinariesDialog(QWidget* parent, const QString& qstrBaseFileName,
                                               const QString& qstrBaseNameSuffix,
                                               const QStringList& qslDevicesToExport,
                                               const bool is32bitCheckboxSet,
                                               const bool is64bitCheckboxSet) :
    QFileDialog(parent),
    m_pVBoxLayout(0),
    m_pCheckBoxIncludeSource(0),
    m_pCheckBoxIncludeIL(0),
    m_pCheckBoxIncludeISA(0),
    m_pCheckBoxBittness32(0),
    m_pCheckBoxBittness64(0),
    m_pCheckBoxIncludeLLVM_IR(0),
    m_pCheckBoxIncludeDebugInfo(0),
    m_pButtonExport(0),
    m_pOrigLayout(0),
    buttonBox(0),
    lookInLabel(0),
    lookInCombo(0),
    backButton(0),
    forwardButton(0),
    detailModeButton(0),
    toParentButton(0),
    newFolderButton(0),
    listModeButton(0),
    fileTypeCombo(0),
    fileNameLabel(0),
    fileTypeLabel(0),
    bittnessTypeLabel(0),
    baseFileName(0),
    fileNameEdit(0),
    lookInComboLineEdit(0),
    qstrDefaultBaseName(qstrBaseFileName),
    baseFileNameDescription(0),
    m_bExportButtonEnabled(true),
    m_qslistDevicesToExport(qslDevicesToExport),
    qstrConstBaseNameSuffix(qstrBaseNameSuffix),
    baseFileNameLabel(0)
{
    SetDialogOptions();
    bool bInitSucceeded = InitDialogWidgets(is32bitCheckboxSet, is64bitCheckboxSet);
    SetDialogLayout(bInitSucceeded);

}

//-------------------- dtor ---------------------------------------
kaExportBinariesDialog::~kaExportBinariesDialog()
{
}

//-----------------------------------------------------------
void kaExportBinariesDialog::SetDialogOptions()
{
    setOption(QFileDialog::DontUseNativeDialog);
    setOption(QFileDialog::HideNameFilterDetails);
    setFileMode(QFileDialog::Directory);
}


//---------------------------------------------------------
bool kaExportBinariesDialog::InitDialogWidgets(const bool is32bitCheckboxSet, const bool is64bitCheckboxSet)
{
    bool bRes = true;

    m_pCheckBoxIncludeDebugInfo = new QCheckBox(KA_STR_ExportBinariesDialogDebugInfoCheckboxText);
    m_pCheckBoxIncludeIL = new QCheckBox(KA_STR_ExportBinariesDialogILCheckboxText);
    m_pCheckBoxIncludeISA = new QCheckBox(KA_STR_ExportBinariesDialogISACheckboxText);
    m_pCheckBoxBittness32 = new QCheckBox(KA_STR_ExportBinariesDialogBittness32CheckboxText);
    m_pCheckBoxBittness64 = new QCheckBox(KA_STR_ExportBinariesDialogBittness64CheckboxText);
    m_pCheckBoxIncludeLLVM_IR = new QCheckBox(KA_STR_ExportBinariesDialogLLVMIRCheckboxText);
    m_pCheckBoxIncludeSource = new QCheckBox(KA_STR_ExportBinariesDialogSourceCheckboxText);

    m_pCheckBoxIncludeDebugInfo->setToolTip(KA_STR_ExportBinariesDialogDebugInfoCheckboxTooltip);
    m_pCheckBoxIncludeDebugInfo->setChecked(true);
    m_pCheckBoxIncludeIL->setToolTip(KA_STR_ExportBinariesDialogILCheckboxTooltip);
    m_pCheckBoxIncludeIL->setChecked(true);
    m_pCheckBoxIncludeISA->setToolTip(KA_STR_ExportBinariesDialogISACheckboxTooltip);
    m_pCheckBoxIncludeISA->setChecked(true);

    //TODO check if needs to be set to true
    m_pCheckBoxBittness32->setToolTip(KA_STR_ExportBinariesDialogBittness32CheckboxTooltip);
    m_pCheckBoxBittness32->setChecked(is32bitCheckboxSet);
    m_pCheckBoxBittness64->setToolTip(KA_STR_ExportBinariesDialogBittness64CheckboxTooltip);
    m_pCheckBoxBittness64->setChecked(is64bitCheckboxSet);
    m_pCheckBoxIncludeLLVM_IR->setToolTip(KA_STR_ExportBinariesDialogLLVMIRCheckboxTooltip);
    m_pCheckBoxIncludeLLVM_IR->setChecked(true);
    m_pCheckBoxIncludeSource->setToolTip(KA_STR_ExportBinariesDialogSourceCheckboxTooltip);


    m_pOrigLayout = dynamic_cast<QGridLayout*>(layout());

    if (nullptr == m_pOrigLayout)
    {
        bRes = false;
    }

    buttonBox = findChild<QDialogButtonBox*>();

    if (nullptr == buttonBox)
    {
        bRes = false;
    }

    // AcceptMode::AcceptOpen is used when choosing directory
    // assigning m_pButtonExport to buttonBox Accept button
    QFileDialog::AcceptMode am = acceptMode();

    if (am == QFileDialog::AcceptOpen)
    {
        m_pButtonExport = (QPushButton*)buttonBox->button(QDialogButtonBox::Open);
    }
    else
    {
        m_pButtonExport = (QPushButton*)buttonBox->button(QDialogButtonBox::Save);
    }

    if (nullptr == m_pButtonExport)
    {
        bRes = false;
    }
    else
    {
        m_pButtonExport->installEventFilter(this);
    }

    lookInLabel = findChild<QLabel*>(KA_STR_QFileDialogLookInLabel);

    if (nullptr == lookInLabel)
    {
        bRes = false;
    }

    lookInCombo = findChild<QComboBox*>(KA_STR_QFileDialogLookInCombo);

    if (nullptr == lookInCombo)
    {
        bRes = false;
    }
    else
    {
        lookInCombo->setEditable(true);
        lookInComboLineEdit = lookInCombo->lineEdit();

        if (lookInComboLineEdit == nullptr)
        {
            bRes = false;
        }
    }

    backButton = findChild<QToolButton*>(KA_STR_QFileDialogToolButtonBack);

    if (nullptr == backButton)
    {
        bRes = false;
    }

    forwardButton = findChild<QToolButton*>(KA_STR_QFileDialogToolButtonForward);

    if (nullptr == forwardButton)
    {
        bRes = false;
    }

    detailModeButton = findChild<QToolButton*>(KA_STR_QFileDialogToolButtonDetailMode);

    if (nullptr == detailModeButton)
    {
        bRes = false;
    }

    toParentButton = findChild<QToolButton*>(KA_STR_QFileDialogToolButtonParent);

    if (nullptr == toParentButton)
    {
        bRes = false;
    }

    newFolderButton = findChild<QToolButton*>(KA_STR_QFileDialogToolButtonNewFolder);

    if (nullptr == newFolderButton)
    {
        bRes = false;
    }

    listModeButton = findChild<QToolButton*>(KA_STR_QFileDialogToolButtonListMode);

    if (nullptr == listModeButton)
    {
        bRes = false;
    }

    fileNameLabel = findChild<QLabel*>(KA_STR_QFileDialogLabelFileName);

    if (nullptr == fileNameLabel)
    {
        bRes = false;
    }
    else
    {
        fileNameLabel->setText(KA_STR_ExportBinariesDialogFileNameLabel);
    }

    // fileTypeCombo will be hidden, but fileTypeLabel renamed to "Include:"
    fileTypeCombo = findChild<QComboBox*>(KA_STR_QFileDialogComboBoxFileType);

    if (nullptr == fileTypeCombo)
    {
        bRes = false;
    }

    fileTypeLabel = findChild<QLabel*>(KA_STR_QFileDialogLabelFileType);

    if (nullptr == fileTypeLabel)
    {
        bRes = false;
    }


    bittnessTypeLabel = new QLabel(KA_STR_ExportBinariesDialogBittnessLabel);

    baseFileName = new QLineEdit;

    if (nullptr == baseFileName)
    {
        bRes = false;
    }

    fileNameEdit = findChild<QLineEdit*>(KA_STR_QFileDialogLineEditFileName);

    if (nullptr == fileNameEdit)
    {
        bRes = false;
    }
    else
    {
        fileNameEdit->hide();
    }

    baseFileNameDescription = new QLabel;

    if (nullptr == baseFileNameDescription)
    {
        bRes = false;
    }

    baseFileNameLabel = new QLabel;

    if (nullptr == baseFileNameLabel)
    {
        bRes = false;
    }

    QRegExp re(OS_INVALID_CHARS_FOR_FILE_QREGEXP);
    QRegExpValidator* pRegExpValidator = new QRegExpValidator(re);

    baseFileName->setValidator(pRegExpValidator);

    return bRes;
}

//-----------------------------------------------------------
void kaExportBinariesDialog::SetDialogLayout(bool bInitSucceeded)
{
    if (bInitSucceeded)
    {
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS))
        QList<QUrl> urls;
        urls = sidebarUrls();
        urls.prepend(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));
        urls.prepend(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)));
        setSidebarUrls(urls);
#endif
        setWindowTitle(KA_STR_ExportBinariesDialogTitle);
        setLabelText(QFileDialog::Accept, KA_STR_ExportBinariesDialogAcceptButtonText);
        m_pOrigLayout->removeWidget(buttonBox);
        buttonBox->setOrientation(Qt::Horizontal);
        m_pOrigLayout->removeWidget(lookInLabel);
        m_pOrigLayout->removeWidget(fileTypeCombo);
        fileTypeCombo->hide();
        m_pOrigLayout->removeWidget(fileTypeLabel);
        m_pOrigLayout->removeWidget(fileNameLabel);
        lookInLabel->setText(KA_STR_ExportBinariesDialogExportToLabelText);
        fileTypeLabel->setText(KA_STR_ExportBinariesDialogIncludeLable);
        m_pOrigLayout->addWidget(fileTypeLabel, 3, 0);

        QHBoxLayout* pHBoxLayout = new QHBoxLayout; // nullptr checks deleted since CodeXL uses a handler for failed allocations
        pHBoxLayout->addWidget(lookInLabel);
        pHBoxLayout->addWidget(lookInCombo);
        QSizePolicy policy = lookInCombo->sizePolicy();
        policy.setHorizontalStretch(1);
        policy.setVerticalStretch(0);
        policy.setVerticalPolicy(QSizePolicy::Fixed);
        policy.setHorizontalPolicy(QSizePolicy::Ignored);
        lookInCombo->setSizePolicy(policy);
        lookInCombo->setMinimumSize(50, 0);
        pHBoxLayout->addWidget(backButton);
        pHBoxLayout->addWidget(forwardButton);
        pHBoxLayout->addWidget(toParentButton);
        pHBoxLayout->addWidget(newFolderButton);
        pHBoxLayout->addWidget(listModeButton);
        pHBoxLayout->addWidget(detailModeButton);
        m_pOrigLayout->addLayout(pHBoxLayout, 0, 0, 1, 3);

        //Base name and actual name rows (QGBoxLayout) are embedded into QVBoxLayout
        QVBoxLayout* pFileNamesRows = new QVBoxLayout;

        QHBoxLayout* pBaseFileNameLayout = new QHBoxLayout;
        pBaseFileNameLayout->addWidget(baseFileNameLabel);
        baseFileNameLabel->setMinimumWidth(100);
        baseFileNameLabel->setText(KA_STR_ExportBinariesDialogBaseFaleNameLabel);
        baseFileNameLabel->setBuddy(baseFileName);
        pBaseFileNameLayout->addWidget(baseFileName);
        QSizePolicy baseNamePolicy = baseFileName->sizePolicy();
        baseNamePolicy.setHorizontalStretch(1);
        baseNamePolicy.setVerticalStretch(0);
        baseNamePolicy.setVerticalPolicy(QSizePolicy::Fixed);
        baseNamePolicy.setHorizontalPolicy(QSizePolicy::Ignored);
        baseFileName->setSizePolicy(baseNamePolicy);
        baseFileName->setMinimumSize(50, 0);
        baseFileName->setText(qstrDefaultBaseName);
        pFileNamesRows->addLayout(pBaseFileNameLayout);


        QHBoxLayout* pFileNameLayout = new QHBoxLayout;
        QSizePolicy spFileName(QSizePolicy::Fixed, QSizePolicy::Fixed);
        spFileName.setHorizontalStretch(1);
        fileNameLabel->setSizePolicy(spFileName);
        fileNameLabel->setMinimumSize(100, 0);
        pFileNameLayout->addWidget(fileNameLabel);
        fileNameLabel->setText(KA_STR_ExportBinariesDialogFileNameLabel);
        QSizePolicy spbaseFileNameDescription(QSizePolicy::Preferred, QSizePolicy::Preferred);
        spbaseFileNameDescription.setHorizontalStretch(1);
        baseFileNameDescription->setSizePolicy(spbaseFileNameDescription);
        baseFileNameDescription->setMinimumWidth(200);
        baseFileNameDescription->setText(qstrDefaultBaseName + qstrConstBaseNameSuffix);
        pFileNameLayout->addWidget(baseFileNameDescription);
        baseFileNameDescription->setText(qstrDefaultBaseName + qstrConstBaseNameSuffix);
        baseFileNameDescription->setAutoFillBackground(true);
        QPalette p;
        p.setColor(QPalette::WindowText, QColor(0, 0, 0, 150));
        baseFileNameDescription->setAutoFillBackground(true);
        baseFileNameDescription->setPalette(p);
        pFileNamesRows->addLayout(pFileNameLayout);

        // these rows are placed into [4,0] position of the main grid layout and
        // span 1 row and 3 columns
        m_pOrigLayout->addLayout(pFileNamesRows, 3, 0, 1, 3);


        // check boxes with "Sections included:" label
        // are put into a row with a grid that includes line of check boxes
        QGridLayout* pCheckboxGrid = new QGridLayout;

        pCheckboxGrid->addWidget(fileTypeLabel, 0, 0);
        fileTypeLabel->setMinimumWidth(100);
        pCheckboxGrid->addWidget(m_pCheckBoxIncludeSource, 0, 1);
        pCheckboxGrid->addWidget(m_pCheckBoxIncludeLLVM_IR, 0, 2);
        m_pCheckBoxIncludeSource->setMinimumWidth(20);
        m_pCheckBoxIncludeLLVM_IR->setMinimumWidth(20);

        pCheckboxGrid->addWidget(m_pCheckBoxIncludeIL, 0 , 3);
        m_pCheckBoxIncludeIL->setMinimumWidth(20);
        pCheckboxGrid->addWidget(m_pCheckBoxIncludeDebugInfo, 0, 4);
        m_pCheckBoxIncludeDebugInfo->setMinimumWidth(20);
        m_pCheckBoxIncludeISA->setMaximumWidth(50);
        pCheckboxGrid->addWidget(m_pCheckBoxIncludeISA, 0 , 5);

        bittnessTypeLabel->setMinimumWidth(100);
        pCheckboxGrid->addWidget(bittnessTypeLabel, 1, 0);


        m_pCheckBoxBittness32->setMinimumWidth(20);
        m_pCheckBoxBittness64->setMinimumWidth(20);
        pCheckboxGrid->addWidget(m_pCheckBoxBittness32, 1, 1);
        pCheckboxGrid->addWidget(m_pCheckBoxBittness64, 1, 2);

        m_pOrigLayout->addLayout(pCheckboxGrid, 4, 0);

        m_pOrigLayout->addWidget(buttonBox, 5, 1, 1, 2);

        // removes Help button
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
        bool rc = false;
        // when editing finished and focus is passed to other widget - check entered directory
        rc = connect(lookInComboLineEdit, SIGNAL(editingFinished()), this, SLOT(CheckEnteredDirectory()));
        GT_ASSERT(rc);

        rc = connect(baseFileName, SIGNAL(textEdited(const QString&)), this, SLOT(BuildFileNameStringOnTextEdited()));
        GT_ASSERT(rc);

        rc = connect(m_pButtonExport, SIGNAL(clicked()), SLOT(ValidateBaseFileNameFinal()));
        GT_ASSERT(rc);

        rc = connect(this, SIGNAL(currentChanged(const QString&)), SLOT(testSelection(const QString&)));
        GT_ASSERT(rc);

        rc = connect(m_pCheckBoxIncludeDebugInfo, SIGNAL(toggled(bool)), this, SLOT(CheckBoxStateChanged(bool)));
        GT_ASSERT(rc);

        rc = connect(m_pCheckBoxIncludeIL, SIGNAL(toggled(bool)), this, SLOT(CheckBoxStateChanged(bool)));
        GT_ASSERT(rc);

        rc = connect(m_pCheckBoxIncludeISA, SIGNAL(toggled(bool)), this, SLOT(CheckBoxStateChanged(bool)));
        GT_ASSERT(rc);

        rc = connect(m_pCheckBoxBittness32, SIGNAL(toggled(bool)), this, SLOT(CheckBoxStateChanged(bool)));
        GT_ASSERT(rc);
        rc = connect(m_pCheckBoxBittness64, SIGNAL(toggled(bool)), this, SLOT(CheckBoxStateChanged(bool)));
        GT_ASSERT(rc);

        rc = connect(m_pCheckBoxIncludeLLVM_IR, SIGNAL(toggled(bool)), this, SLOT(CheckBoxStateChanged(bool)));
        GT_ASSERT(rc);

        rc = connect(m_pCheckBoxIncludeSource, SIGNAL(toggled(bool)), this, SLOT(CheckBoxStateChanged(bool)));
        GT_ASSERT(rc);

        rc = disconnect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
        GT_ASSERT(rc);

        // set focus on look in line combo box line edit
        QTimer::singleShot(0, lookInComboLineEdit, SLOT(setFocus()));
    }
}

void kaExportBinariesDialog::CheckBoxStateChanged(bool bState)
{
    GT_UNREFERENCED_PARAMETER(bState);
    bool bIsSourceIncludeChecked = (nullptr != m_pCheckBoxIncludeDebugInfo && m_pCheckBoxIncludeDebugInfo->isChecked()) ||
                                   (nullptr != m_pCheckBoxIncludeIL && m_pCheckBoxIncludeIL->isChecked()) ||
                                   (nullptr != m_pCheckBoxIncludeISA && m_pCheckBoxIncludeISA->isChecked()) ||
                                   (nullptr != m_pCheckBoxIncludeLLVM_IR && m_pCheckBoxIncludeLLVM_IR->isChecked()) ||
                                   (nullptr != m_pCheckBoxIncludeSource && m_pCheckBoxIncludeSource->isChecked());
    bool isBittnessChecked = (nullptr != m_pCheckBoxBittness32 && m_pCheckBoxBittness32->isChecked()) ||
                             (nullptr != m_pCheckBoxBittness64 && m_pCheckBoxBittness64->isChecked());

    bool bRes = bIsSourceIncludeChecked && isBittnessChecked;

    if (bRes != m_bExportButtonEnabled)
    {
        m_bExportButtonEnabled = bRes;

        if (nullptr != m_pButtonExport)
        {
            m_pButtonExport->setFocus();
            m_pButtonExport->setEnabled(m_bExportButtonEnabled);
            m_pButtonExport->update();
        }
    }
}

void kaExportBinariesDialog::testSelection(const QString& dirName)
{
    QFileInfo fileInfo(dirName);
    QString filePath = fileInfo.filePath();

    if (fileInfo.exists())
    {
        if (fileInfo.isFile())
        {
            QString strName = fileInfo.completeBaseName();
            QString strAbsPath = adjustQtStringToCurrentOS(fileInfo.absolutePath());
            int nLastIndex = strName.lastIndexOf(KA_STR_ExportBinariesDialogHyphenPart);

            if (-1 != nLastIndex)
            {
                qstrCustomBaseName = strName.mid(0, nLastIndex);
            }
            else
            {
                qstrCustomBaseName = strName;
            }

            if (nullptr != baseFileName)
            {
                baseFileName->setText(qstrCustomBaseName);
            }

            if (nullptr != baseFileNameDescription)
            {
                baseFileNameDescription->setText(qstrCustomBaseName + qstrConstBaseNameSuffix);
            }
        }
    }
}
// while entering base name display actual file name combining two parts
void kaExportBinariesDialog::BuildFileNameStringOnTextEdited()
{
    if (nullptr != baseFileName)
    {
        qstrCustomBaseName = baseFileName->text();

        if (nullptr != baseFileNameDescription)
        {
            baseFileNameDescription->setText(qstrCustomBaseName + qstrConstBaseNameSuffix);
        }
    }
}

QStringList kaExportBinariesDialog::selectedFiles() const
{
    QStringList files;

    if (nullptr != lookInComboLineEdit)
    {
        QString file = lookInComboLineEdit->text();
        files.append(file);
    }

    return files;
}

// Export button will behave as OK button only if baseFileName field is not empty
void kaExportBinariesDialog::ValidateBaseFileNameFinal()
{
    if (nullptr != baseFileName)
    {
        qstrCustomBaseName = baseFileName->text();

        if (qstrCustomBaseName.isEmpty())
        {
            acMessageBox::instance().warning(AF_STR_WarningA, KA_STR_ExportBinariesDialogEmptyBaseNameWarning);
            qstrCustomBaseName = qstrDefaultBaseName;
            baseFileName->setText(qstrCustomBaseName);
            baseFileName->setFocus();
            baseFileName->selectAll();

            if (nullptr != baseFileNameDescription)
            {
                baseFileNameDescription->setText(qstrCustomBaseName + qstrConstBaseNameSuffix);
            }
        }
        else
        {
            accept();
        }
    }
}

/// Prepares current exported binary full name to check if already exists
void kaExportBinariesDialog::PrepareCurrentBinaryFullName(QString& qstrResult, const QString& qstrCurrentPath, const QString& qstrBaseFileName, const QString& qstrDeviceName)const
{
    gtString  fileName;
    osFilePath(acQStringToGTString(qstrDeviceName)).getFileName(fileName);


    qstrResult = qstrCurrentPath;
    qstrResult += osFilePath::osPathSeparator;
    qstrResult += qstrBaseFileName;
    qstrResult += KA_STR_ExportBinariesDialogHyphenPart;
    qstrResult += acGTStringToQString(fileName);
    qstrResult += KA_STR_ExportBinariesDialogBinPart;
}

/// Checks if current binary already exists
bool kaExportBinariesDialog::ExportedBinaryAlreadyExists(const QString& qstrCurrentExportedBinary)const
{
    bool bBinaryWithSameNameExists = false;
    QFileInfo fileInfo(qstrCurrentExportedBinary);
    bBinaryWithSameNameExists |= fileInfo.exists();
    return bBinaryWithSameNameExists;
}


void kaExportBinariesDialog::accept()
{
    bool bRes = CheckEnteredDirectory();

    if (bRes)
    {
        QStringList files = selectedFiles();

        if (!files.isEmpty())
        {
            int numDevicesToExport = m_qslistDevicesToExport.size();
            //check if at least one binary already exists in chosen directory
            bool bBinaryWithSameNameExists = false;
            QString qstrCurrentBinaryName;

            for (int nExport = 0; nExport < numDevicesToExport; nExport++)
            {
                PrepareCurrentBinaryFullName(qstrCurrentBinaryName, selectedFiles().first(), GetBaseName(), m_qslistDevicesToExport[nExport]);
                bBinaryWithSameNameExists |= ExportedBinaryAlreadyExists(qstrCurrentBinaryName);
            }

            int userSelection = 0;

            if (bBinaryWithSameNameExists)
            {
                userSelection = acMessageBox::instance().question(AF_STR_QuestionA, KA_STR_BinariesExist, QMessageBox::Yes | QMessageBox::No);
            }

            if (!bBinaryWithSameNameExists || (userSelection == QMessageBox::Yes))
            {
                emit fileSelected(files.first());
                QDialog::accept();
            }
        }
    }
}

// checks if entered text is existing directory before calling setDirectory
void kaExportBinariesDialog::SetChosenDirectory()
{
    const QString& text = lookInComboLineEdit->text();

    if (!text.isNull())
    {
        if (QDir(text).exists())
        {
            setDirectory(text);
        }
        else
        {
            lookInComboLineEdit->selectAll();
            lookInCombo->setFocus();
        }
    }
}

bool kaExportBinariesDialog::CheckEnteredDirectory()
{
    bool bRes = true;
    QDir currDir = directory();
    const QString& text = lookInComboLineEdit->text();

    if (!text.isNull())
    {
        if (QDir(text).exists())
        {

            setDirectory(text);
        }
        else
        {
            bRes = false;
            lookInComboLineEdit->setText(this->adjustQtStringToCurrentOS(currDir.path()));
            lookInComboLineEdit->selectAll();
        }
    }
    else
    {
        bRes = false;
    }

    return bRes;
}

// this method prevents closing the dialog by pressing Enter
void kaExportBinariesDialog::keyPressEvent(QKeyEvent* e)
{
    if (e->key() != Qt::Key_Enter && e->key() != Qt::Key_Return)
    {
        QFileDialog::keyPressEvent(e);
    }
}

bool kaExportBinariesDialog::eventFilter(QObject* obj, QEvent* e)
{
    bool bRes = false;

    if (obj == m_pButtonExport)
    {
        if (e->type() == QEvent::EnabledChange)
        {
            m_pButtonExport->setEnabled(m_bExportButtonEnabled);
            m_pButtonExport->update();
        }
    }

    return bRes;

}

// the following method are used to get user choices after the dialog is closed
bool kaExportBinariesDialog::IsILIncluded()const
{
    bool retVal = false;

    // Sanity check:
    if (m_pCheckBoxIncludeIL != nullptr)
    {
        retVal = (m_pCheckBoxIncludeIL->checkState() == Qt::Checked);

    }

    return retVal;
}

bool kaExportBinariesDialog::IsSourceIncluded()const
{
    bool retVal = false;

    // Sanity check:
    if (m_pCheckBoxIncludeSource != nullptr)
    {
        retVal = (m_pCheckBoxIncludeSource->checkState() == Qt::Checked);
    }

    return retVal;
}

bool kaExportBinariesDialog::IsISAIncluded()const
{
    bool retVal = false;

    // Sanity check:
    if (m_pCheckBoxIncludeISA != nullptr)
    {
        retVal = (m_pCheckBoxIncludeISA->checkState() == Qt::Checked);
    }

    return retVal;
}

bool kaExportBinariesDialog::Is32BitIncluded() const
{
    bool retVal = false;

    // Sanity check:
    if (m_pCheckBoxBittness32 != nullptr)
    {
        retVal = (m_pCheckBoxBittness32->checkState() == Qt::Checked);
    }

    return retVal;
}


bool kaExportBinariesDialog::Is64BitIncluded() const
{
    bool retVal = false;

    // Sanity check:
    if (m_pCheckBoxBittness64 != nullptr)
    {
        retVal = (m_pCheckBoxBittness64->checkState() == Qt::Checked);
    }

    return retVal;
}

bool kaExportBinariesDialog::IsLLVM_IRIncluded()const
{
    bool retVal = false;

    // Sanity check:
    if (m_pCheckBoxIncludeLLVM_IR != nullptr)
    {
        retVal = (m_pCheckBoxIncludeLLVM_IR->checkState() == Qt::Checked);
    }

    return retVal;
}

bool kaExportBinariesDialog::IsDebugInfoIncluded()const
{
    bool retVal = false;

    // Sanity check:
    if (m_pCheckBoxIncludeDebugInfo != nullptr)
    {
        retVal = (m_pCheckBoxIncludeDebugInfo->checkState() == Qt::Checked);
    }

    return retVal;
}

const QString kaExportBinariesDialog::GetBaseName()const
{
    QString qstrBaseFileName;

    if (baseFileName)
    {
        qstrBaseFileName = baseFileName->text();
    }

    return qstrBaseFileName;
}

QString kaExportBinariesDialog::adjustQtStringToCurrentOS(const QString& dirName)const
{
    gtString gtPath = acQStringToGTString(dirName);
    osFilePath::adjustStringToCurrentOS(gtPath);
    return acGTStringToQString(gtPath);
}
