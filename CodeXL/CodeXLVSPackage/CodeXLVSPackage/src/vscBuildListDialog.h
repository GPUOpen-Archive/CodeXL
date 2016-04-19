//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscBuildListDialog.h
///
//==================================================================================

//------------------------------ vscBuildListDialog.h ------------------------------

#ifndef __VSPBUILDLISTDIALOG
#define __VSPBUILDLISTDIALOG

// Qt:
#include <QtWidgets>
#include <QDialog>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtString.h>

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

// Forward declaration:
class acListCtrl;

// ----------------------------------------------------------------------------------
// Class Name: vscBuildListDialog
// General Description: Display the out of date projects that needs build before running
// Author:              gilad yarnitzky
// Creation Date:       24/3/2011
// ----------------------------------------------------------------------------------
class vscBuildListDialog : public QDialog
{
    Q_OBJECT
public:
    vscBuildListDialog(QWidget* pParent, gtVector<gtString>& projectNames);
    ~vscBuildListDialog();

    bool shouldBuild() const {return m_shouldBuild;}
private:
    void buildLayout();

private slots:
    void onAccept();

private:

    acListCtrl* m_pProjectsList;

    QLabel* m_pUpperText;
    QLabel* m_pLowerText;

    QPushButton* m_pYesButton;

    bool m_shouldBuild;
};

#endif  // __VSPBUILDLISTDIALOG
