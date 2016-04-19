//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdMemoryView.h
///
//==================================================================================

//------------------------------ gdMemoryView..h ------------------------------

#ifndef __GDMEMORYVIEW_H
#define __GDMEMORYVIEW_H

// Qt:
#include <QtWidgets>

// Forward declaration:
class afApplicationCommands;
class gdApplicationCommands;

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// CodeXLAppCode:
#include <AMDTGpuDebuggingComponents/Include/views/gdMemoryAnalysisDetailsView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdMemoryViewBase.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:          gdMemoryView
// General Description: GUI object handling memory consumption view in VS integration
// Author:              Sigal Algranaty
// Creation Date:       17/1/2011
// ----------------------------------------------------------------------------------
class GD_API gdMemoryView : public QSplitter, public gdMemoryViewBase
{
    Q_OBJECT

public:

    gdMemoryView(afProgressBarWrapper* pProgressBar, QWidget* pParent);
    virtual ~gdMemoryView();

    bool updateView(bool isViewShown);
    virtual bool isShown() {return _isShown;};

    bool displayItemMemory(const afApplicationTreeItemData* pTreeItemData);

protected:

    // Virtual function:
    // Raise the view and show it as top window:
    virtual void raiseView();

    void setFrameLayout();
    virtual void onTreeItemSelection(const apMonitoredObjectsTreeSelectedEvent& eve);

protected slots:

    void onListItemSelected(QTableWidgetItem* pItem);
    void onListItemDeselected(QTableWidgetItem* pItem);
    void onListItemActivated(QTableWidgetItem* pItem);
    void onListItemSelectionChanged();

protected:

    // Contain true iff the view is currently shown in VS:
    bool _isShown;

    // Widget containing the control in the bottom and top of the view:
    QWidget* _pBottomWidget;

    // Contain the proportion for the top / bottom panels:
    float _bottomPanelProp;

    // Contain true iff this is the first time that the window layout is set:
    bool _isFirstTimeLayout;

    // Application commands handler:
    afApplicationCommands* _pApplicationCommands;
    gdApplicationCommands* _pGDApplicationCommands;
};


#endif //  __GDMEMORYVIEW_H

