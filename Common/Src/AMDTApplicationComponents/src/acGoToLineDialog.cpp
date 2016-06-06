//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acGoToLineDialog.cpp
///
//==================================================================================
#include <AMDTApplicationComponents/Include/acGoToLineDialog.h>

#include <AMDTApplicationComponents/Include/acQtIncludes.h>

#include <AMDTBaseTools/Include/gtAssert.h>
#include <QIntValidator>


//------------------------------ acGoToLineDialog.cpp ------------------------------

acGoToLineDialog::acGoToLineDialog(QWidget* pParent, unsigned int numLines) : acDialog(pParent)
{
    // remove context help button
    Qt::WindowFlags flags = windowFlags();
    Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
    flags = flags & (~helpFlag);
    setWindowFlags(flags);

    m_pPushButtonOK = new QPushButton("OK");
    m_pPushButtonCancel = new QPushButton("Cancel");

    m_pMainLayout = new QVBoxLayout;

    QString str = "Line number: (1 - %1)";
    m_pGotoLineInstructionsLabel = new QLabel();
    m_pGotoLineInstructionsLabel->setText(QString(str).arg(numLines - 1));
    m_pLineEdit = new QLineEdit(QString::number(numLines - 1));
    m_pMainLayout->addWidget(m_pGotoLineInstructionsLabel);
    m_pMainLayout->addWidget(m_pLineEdit);

    QValidator *inputRange = new QIntValidator(0, numLines - 1, this);
    m_pLineEdit->setValidator(inputRange);
    m_pLineEdit->setFocus();
    m_pLineEdit->selectAll();

    m_pButtonBox = new QHBoxLayout;

    m_pButtonBox->addStretch();
    m_pButtonBox->addWidget(m_pPushButtonOK);
    m_pButtonBox->addWidget(m_pPushButtonCancel);
    m_pMainLayout->addLayout(m_pButtonBox);

    setLayout(m_pMainLayout);

    bool rc = connect(m_pPushButtonOK, SIGNAL(clicked()), this, SLOT(OnOk()));
    GT_ASSERT(rc);
    rc = connect(m_pPushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    GT_ASSERT(rc);
}
acGoToLineDialog::~acGoToLineDialog()
{

}
unsigned int acGoToLineDialog::GetLineNumber()const 
{
    return m_numSelectedLine; 
}

void acGoToLineDialog::OnOk()
{
    m_numSelectedLine = m_pLineEdit->text().toInt();
    accept();
}
