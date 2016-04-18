//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspSaveListDialog.h
///
//==================================================================================

//------------------------------ vspSaveListDialog.h ------------------------------

#ifndef __VSPSAVELISTDIALOG
#define __VSPSAVELISTDIALOG

// Qt:
#include <QtWidgets>
#include <QDialog>

// Local (core):
#include <Include/Public/CoreInterfaces/IDteTreeEventHandler.h>

class acTreeCtrl;
QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

// ----------------------------------------------------------------------------------
// Class Name: vspSaveListDialog
// General Description: Display the out of date projects that needs build before running
// Author:              gilad yarnitzky
// Creation Date:       24/3/2011
// ----------------------------------------------------------------------------------
class vspSaveListDialog : public QDialog, IDteTreeEventHandler
{
    Q_OBJECT
public:
    vspSaveListDialog(QWidget* pParent);
    ~vspSaveListDialog();

    bool hasItems();

    bool userDecision() const {return m_userDecision;};

private slots:

    void onAccept();
private:

    void buildLayout();

    // IDteTreeEventHandler region - start.

    virtual void vscAddSolutionAsItemToTree(const wchar_t* pItemName);

    virtual void vscAddIOpenDocumentAsItemToTree(const wchar_t* documentName, const wchar_t* documentProjectName, const wchar_t* solutionName);

    virtual void vscAddOpenProjectAsItemToTree(const wchar_t* pItemName, const wchar_t* pSolutionName);

    virtual void vscExpandWholeTree();


    // IDteTreeEventHandler region - end.

    acTreeCtrl* m_pFilesTree;
    QTreeWidgetItem* m_pTreeRoot;

    QLabel* m_pUpperText;
    QPushButton* m_pYesButton;

    bool m_userDecision;

};

#endif  // __VSPSAVELISTDIALOG
