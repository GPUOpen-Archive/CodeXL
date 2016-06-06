//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acDialog.cpp
///
//==================================================================================

//------------------------------ acDialog.cpp ------------------------------

#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Local:
#include <AMDTApplicationComponents/inc/acStringConstants.h>
#include <AMDTApplicationComponents/Include/acDialog.h>
#include <AMDTApplicationComponents/Include/acQtIncludes.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// ----------------------------------------------------------------------------------
// Class Name:          log::acDialog(QWidget* parent, bool hasOkButton, bool hasCancelButton, QDialogButtonBox::StandardButton defaultButton,  QList<QPushButton*>* pCustomButtons, Qt::WindowFlags flags)
// General Description: A wrapper class for QDialog
//
// Author:              Yoni Rabin
// Creation Date:       27/6/2012
// ----------------------------------------------------------------------------------
acDialog::acDialog(QWidget* parent, bool hasOkButton, bool hasCancelButton, QDialogButtonBox::StandardButton defaultButton,  QList<QPushButton*>* pCustomButtons, Qt::WindowFlags flags)
    : QDialog(parent, flags)
    , m_pCustomButtons(pCustomButtons)
    , m_pOKButton(nullptr)
    , m_defaultButton(defaultButton)
    , m_pLogo(NULL)
    , m_hasOkButton(hasOkButton)
    , m_hasCancelButton(hasCancelButton)
{
    setWindowFlags(flags);
}

// ---------------------------------------------------------------------------
// Name:        ~acDialog::acDialog
// Description: Destructor
// Return Val:
// Author:      Sigal Algranaty
// Date:        15/9/2010
// ---------------------------------------------------------------------------
acDialog::~acDialog()
{
}

QPixmap* acDialog::setLogoPixmap(const gtString& filePath)
{
    return new QPixmap(filePath.asASCIICharArray());
}

QHBoxLayout* acDialog::getBottomButtonLayout(bool hasLogo, const gtString& filePathwithLogo, const gtString& OKButtonNewText)
{
    QHBoxLayout* pLayout = new QHBoxLayout();

    // Logo:
    if (hasLogo)
    {
        m_pLogo = setLogoPixmap(filePathwithLogo);

        if (m_pLogo != NULL)
        {
            QLabel* pDisplayImage = new QLabel();
            pDisplayImage->setPixmap(*m_pLogo);
            pLayout->addWidget(pDisplayImage, 0, Qt::AlignLeft);
        }
    }

    pLayout->addStretch(1);

    // Buttons:
    {
        if (m_hasOkButton)
        {
            QString OkButtonDefaultText = AC_STR_DefaultOKButton;

            if (!OKButtonNewText.isEmpty())
            {
                OkButtonDefaultText = acGTStringToQString(OKButtonNewText);
            }

            m_pOKButton = new QPushButton(OkButtonDefaultText);
            pLayout->addWidget(m_pOKButton, 0, Qt::AlignRight);

            if (m_defaultButton == QDialogButtonBox::Ok)
            {
                m_pOKButton->setAutoDefault(true);
                m_pOKButton->setDefault(true);
            }
            else
            {
                m_pOKButton->setAutoDefault(false);
                m_pOKButton->setDefault(false);
            }

            connect(m_pOKButton, SIGNAL(clicked()), this, SLOT(accept()));
        }

        if (m_hasCancelButton)
        {
            QPushButton* pBtn = new QPushButton(tr("Cancel"));
            pLayout->addWidget(pBtn, 0, Qt::AlignRight);

            if (m_defaultButton == QDialogButtonBox::Cancel)
            {
                pBtn->setAutoDefault(true);
                pBtn->setDefault(true);
            }
            else
            {
                pBtn->setAutoDefault(false);
                pBtn->setDefault(false);
            }

            connect(pBtn, SIGNAL(clicked()), this, SLOT(reject()));
        }

        if (m_pCustomButtons)
        {
            QList<QPushButton*>::iterator it = m_pCustomButtons->begin();

            while (it != m_pCustomButtons->end())
            {
                pLayout->addWidget(*it, 0, Qt::AlignRight);
                it++;
            }
        }
    }
    return pLayout;
}
