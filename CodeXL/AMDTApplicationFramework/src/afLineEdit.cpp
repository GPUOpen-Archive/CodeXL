//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afLineEdit.cpp
///
//==================================================================================

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationFramework/Include/afLineEdit.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>

afLineEdit::afLineEdit(const QString& objectName, QWidget* pParent) : QLineEdit(pParent)
{
    // Set the object name:
    setObjectName(objectName);

    // Initialize the completer based on the string list stored in the global variables manager:
    InitilaizeCompleter();
}

afLineEdit::afLineEdit(const QString& objectName, const QString& contents, QWidget* pParent) : QLineEdit(contents, pParent)
{
    // Set the object name:
    setObjectName(objectName);

    // Initialize the completer based on the string list stored in the global variables manager:
    InitilaizeCompleter();

}

afLineEdit::~afLineEdit()
{
    if (!objectName().isEmpty())
    {
        QCompleter* pCompleter = completer();

        if (pCompleter != nullptr)
        {
            QStringListModel* pCompleterModel = qobject_cast<QStringListModel*>(pCompleter->model());
            GT_IF_WITH_ASSERT(pCompleterModel != nullptr)
            {
                // Update the object completion list:
                afGlobalVariablesManager::instance().SetHistoryList(objectName(), pCompleterModel->stringList());
            }
        }
    }
}

void afLineEdit::InitilaizeCompleter()
{
    // Get the completion list from manager:
    QStringList list;
    afGlobalVariablesManager::instance().GetHistoryList(objectName(), list);

    QCompleter* pCompleter = new QCompleter(this);
    setCompleter(pCompleter);
    QStringListModel* pModel = new QStringListModel;
    pModel->setStringList(list);
    pCompleter->setModel(pModel);

    // Connect a slot to the editing finished signal (we should add the currently edit string to the completer's model list):
    bool rc = connect(this, SIGNAL(editingFinished()), this, SLOT(OnEditFinished()));
    GT_ASSERT(rc);
}

void afLineEdit::OnEditFinished()
{
    // Get the current text:
    QString str = text();

    if (!str.isEmpty() && (completer() != nullptr))
    {
        // Get the list model:
        QStringListModel* pStringListModel = qobject_cast<QStringListModel*>(completer()->model());

        if (pStringListModel != nullptr)
        {
            if (!pStringListModel->stringList().contains(str))
            {
                pStringListModel->insertRows(pStringListModel->rowCount(), 1);
                pStringListModel->setData(pStringListModel->index(pStringListModel->rowCount() - 1), str);
            }

            afGlobalVariablesManager::instance().AddStringToHistoryList(objectName(), str);
        }
    }
}

