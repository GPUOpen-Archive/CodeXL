//------------------------------ kaCreateProgramDialog.h ------------------------------

#ifndef __KACREATEPROGRAMDIALOG_H
#define __KACREATEPROGRAMDIALOG_H

// Qt
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>
#include <AMDTKernelAnalyzer/src/kaDataTypes.h>

//Base Tools
#include <AMDTBaseTools/Include/gtVector.h>


// Forward decelerations:
class acTextCtrl;
class acTreeCtrl;
class afApplicationCommands;
class afLineEdit;
// ----------------------------------------------------------------------------------
// Class Name:              kaCreateProgramDialog : public QDialog
// General Description:     allows the user to select the type of the Program added to the project
//
// Author:               Yuri Rshtunique
// Creation Date:        3/12/2015
// ----------------------------------------------------------------------------------


class KA_API kaCreateProgramDialog : public QDialog
{
    Q_OBJECT

public:

    kaCreateProgramDialog(QWidget* parent);
    ~kaCreateProgramDialog();

    /// Get the program type based on user selection
    kaProgramTypes GetSelectedProgramType()const { return m_programType; }


protected:
    /// Create the dialog layout:
    void CreateLayout();

protected slots:
    /// called on one of the radio buttons in the group toggled.
    /// changing the description according to the selected radio button
    /// \param pButtonis the selected radio button
    void OnTypeSelectionChanged(QAbstractButton* pButton);

protected:

    /// OK/ Cancel buttons
    QDialogButtonBox* m_pButtonBox;

    /// The main layout
    QVBoxLayout* m_pMainLayout;

    /// type options radio buttons group
    QButtonGroup* m_pTypeRadioGroup;

    /// vector of all radio buttons
    QVector<QRadioButton*> m_typeRadioButtonsVector;

    /// descriptions vector for all radio buttons
    QVector<QString> m_descriptionsVector;

    /// the label that will show the description text
    QLabel* m_pDescriptionLabel;

    /// type of the chosen program/folder
    kaProgramTypes m_programType;

    /// selected radio index in the radio group
    unsigned int m_togledRadioButtonIndex;

};

#endif  // __KACREATEPROGRAMDIALOG_H
