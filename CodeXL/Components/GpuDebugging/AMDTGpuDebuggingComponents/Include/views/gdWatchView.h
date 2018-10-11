//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdWatchView.h
///
//==================================================================================

//------------------------------ gdWatchView.h ------------------------------

#ifndef __GDWATCHVIEW_H
#define __GDWATCHVIEW_H

// Forward declaration:
class gdApplicationCommands;

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:          GD_API gdWatchView : public acListCtrl, public apIEventsObserver
// General Description: Displays the names, values, and types of watched expressions
//                      entered by the user.
// Author:              Uri Shomroni
// Creation Date:       8/9/2011
// ----------------------------------------------------------------------------------
class GD_API gdWatchView : public acListCtrl, public apIEventsObserver
{
    Q_OBJECT

public:
    gdWatchView(QWidget* pParent);
    virtual ~gdWatchView();

    // Overrides apIEventsObserver:
    virtual const wchar_t* eventObserverName() const { return L"WatchView"; };

    // Add watch:
    bool addWatch(const gtString& variableName);

protected:

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);

    // Overrides acListCtrl:
    virtual void onBeforeRemoveRow(int row);

    void dropEvent(QDropEvent* pEvent);
    void dragEnterEvent(QDragEnterEvent* pEvent);
    void dragMoveEvent(QDragMoveEvent* pEvent);
    void dragLeaveEvent(QDragLeaveEvent* pEvent);

protected slots:

    void onItemChanged(QTableWidgetItem* pItem);
    void onCellClicked(int row, int column);
    void displaySelectedWatchProperties();


    void onEditPaste();

private:
    void clearListValues();
    void updateWatchValues();
    void updateWatchLineValue(int lineNum);
    void addEditableWatchRow();
    void updateValueColumnHeader();

private:

    gdApplicationCommands* _pApplicationCommands;

    int m_stackDepth;

private:
    bool m_currentlyAddingLine;
};

#endif //__GDWATCHVIEW_H

