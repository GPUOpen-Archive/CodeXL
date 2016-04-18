//------------------------------ kaAddFileDialog.h ------------------------------

#ifndef __KAADDFILEDIALOG_H
#define __KAADDFILEDIALOG_H

// Qt
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>
#include <AMDTKernelAnalyzer/src/kaDataTypes.h>
#include <AMDTKernelAnalyzer/src/kaProgram.h>
Q_DECLARE_METATYPE(kaProgram*)

// ----------------------------------------------------------------------------------
// Class Name:              kaAddFileDialog : public QDialog
// General Description:     allows the user to select the type of the file added to the project
//                          and the name of the file (default name supplied by the system)
// Author:               Gilad Yarnitzky
// Creation Date:        14/1/2015
// ----------------------------------------------------------------------------------
class KA_API kaAddFileDialog : public QDialog
{
    Q_OBJECT

public:
    enum
    {
        kaFileOpenCL = 0,
        kaFileDxVS,
        kaFileDxHS,
        kaFileDxDS,
        kaFileDxGS,
        kaFileDxPS,
        kaFileDxCS,
        kaFileDxGenericHLSL,
        kaFileGlslFrag,
        kaFileGlslVert,
        kaFileGlslComp,
        kaFileGlslGeom,
        kaFileGlslTese,
        kaFileGlslTesc,
        kaFileGenericGLSL,
    } kaFileAddTypesEnum;

    kaAddFileDialog(QWidget* parent, kaProgram* pAssociatedProgram, kaPipelinedProgram::PipelinedStage stage);
    kaAddFileDialog(kaProgram* pAssociatedProgram, const kaProgramTypes programType, kaPipelinedProgram::PipelinedStage stage);
    ~kaAddFileDialog();

    /// get the created file name - for openCL is the word "kernel", for others is the first word in the radio name.
    /// \returns the selected name
    void GetFileNameAndExtension(QString& fileName, QString& fileExtension);

    /// checks if Create New Program combo box option is selected
    bool ShouldForceProgramCreation(kaProgramTypes& typeOfProgramToBeCreated);

    /// file type setter
    void SetFileType(kaFileTypes fileType) { m_fileType = fileType; }

    /// file type getter
    kaFileTypes FileType() { return m_fileType; }

    /// returns associated program
    kaProgram* GetAssociatedProgram() { return m_pAssociatedProgram; }

    /// return the type of associated program
    kaProgramTypes GetAssociatedProgramType() { return m_associatedProgramType; }

    /// populates container widget with radio buttons
    void CreateFileTypesRadioButtons();

    /// returns selected platform
    QString GetSelectedPlatform() const;

protected:

    /// Create the dialog layout:
    void CreateLayout(kaPipelinedProgram::PipelinedStage stage);

protected slots:

    /// called on one of the radio buttons in the group selection changed
    /// changing the description according to the selected radio button
    /// \param pButtonis the selected radio button
    void OnFileTypeSelectionChanged(QAbstractButton* pButton);

    /// called on associated program selection change
    /// \param selection index
    void OnAssociatedProgramSelected(int index);

    /// returns first file type that suits passed program type
    /// \param kaAssociatedProgramType
    kaFileTypes SelectFirstAppropriateFileType(kaProgramTypes kaAssociatedProgramType);

    /// returns program type by passed file type
    /// \param fileType
    /// \retval program type
    kaProgramTypes GetProgramTypeByFileType(kaFileTypes fileType);
    void OnPlatformChanged(int);
    void InitRadioButtonsMap();
    void SetupAssociatedProgram();

    void FillProgramsCombo();

    /// Is handling the ok button click
    void OnOk();
    void SelectDefaultFileType(kaPipelinedProgram::PipelinedStage stage);
private:

    /// Type Combobox
    QComboBox* m_pTypeCombo;

    /// The name horizontal layout
    QHBoxLayout* m_pNameLayout;
    /// The File name
    QLineEdit* m_pFileName;

    /// The main layout
    QVBoxLayout* m_pMainLayout;

    typedef QMap<kaFileTypes, QString> FileTypesList;
    QMap <kaProgramTypes, FileTypesList> m_fileTypesDataMap;
    QMap<kaFileTypes, QString> m_fileTypesToDescriptionMap;

    /// File types radio buttons
    QButtonGroup* m_pFileTypesRadioButtonsGroup;

    /// vector of all radio buttons
    QVector<QRadioButton*> m_fileTypesRadioButtonsVector;

    /// Selected radio index in the radio group
    int m_selectedRadioButtonIndex;

    /// the label that will show the description text
    QLabel* m_pDescriptionLabel;

    /// type of the chosen file
    kaFileTypes m_fileType;

    /// type of the selected program
    kaProgramTypes m_selectedProgramType;

    /// type of the chosen file
    QComboBox* m_pAssociatedProgramCombo;

    /// Associated program
    kaProgram* m_pAssociatedProgram;

    /// Indicates the type of a program that should be created
    kaProgramTypes m_associatedProgramType;

    /// platform selection combo box
    QComboBox* m_pPlatformSelectionCombo;

    /// Shader type title
    QLabel m_fileTypeTitle;

    /// Title for the file types sections
    QLabel* m_pFileTypesHeader;

    QFormLayout* m_pRadioButtonsLayout;
};


#endif  // __KAADDFILEDIALOG_H
