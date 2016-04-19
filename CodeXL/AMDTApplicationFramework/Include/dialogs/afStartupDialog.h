//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afStartupDialog.h
///
//==================================================================================

#ifndef __AFSTARTUPDIALOG_H
#define __AFSTARTUPDIALOG_H

// Qt:
#include <QtCore>
#include <QtWidgets>
#include <qdialog.h>

// Infra:
#include <AMDTApplicationComponents/Include/acDialog.h>

// Local:
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>

// Helper class, defined here for the sake of being moc'ed
class afRichTextButton : public QPushButton
{
    Q_OBJECT

public:
    afRichTextButton(const QImage& iconImage, const QString& text);
    virtual ~afRichTextButton();
    void Draw(const QSizeF& buttonSize);
    QSizeF CalculatedSize() const { return m_calculatedSize; };

private:
    QSizeF m_calculatedSize;
    QString m_buttonText;
    QImage* m_pImage;
};

class afStartupDialog : public acDialog
{
    Q_OBJECT

public:
    afStartupDialog(QWidget* pParent = 0);
    ~afStartupDialog();

    /// Directories list as string accessors:
    afStartupAction GetUserSelection() const {return m_userSelection;};

private slots:
    void OnButtonClicked();

private:
    /// Current user selection in "No project set" dialog:
    afStartupAction m_userSelection;

    afRichTextButton* m_pNewProjectButton;
    afRichTextButton* m_pNewProjectButtonForDebug;
    afRichTextButton* m_pNewProjectButtonForProfile;
    afRichTextButton* m_pNewProjectButtonForFrameAnalyze;
    afRichTextButton* m_pAttachToProcessButton;
    afRichTextButton* m_pSystemWideButton;
    afRichTextButton* m_pAnalyzeNewCLFile;
    afRichTextButton* m_pAnalyzeAddCLFile;
};

#endif //__AFSTARTUPDIALOG_H

