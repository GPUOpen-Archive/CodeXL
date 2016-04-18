//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwKernelWorkItemToolbar.h
///
//==================================================================================

//------------------------------ gwKernelWorkItemToolbar.h ------------------------------

#ifndef __GWKERNELWORKITEMTOOLBAR_H
#define __GWKERNELWORKITEMTOOLBAR_H

// Qt:
#include <QComboBox>
#include <QLabel>

// Forward declaration:
class apBeforeKernelDebuggingEvent;
class apAfterKernelDebuggingEvent;

// Infra:
#include <AMDTApplicationComponents/Include/acToolBar.h>

// Local:
#include <AMDTGpuDebugging/Include/gwgDEBuggerAppWrapperDLLBuild.h>

class GW_API gwWorkItemCombo : public QComboBox
{
    Q_OBJECT

public:
    gwWorkItemCombo(QWidget* pParent);

protected:
    virtual void keyPressEvent(QKeyEvent* pEvent);

};

// ----------------------------------------------------------------------------------
// Class Name:          gwKernelWorkItemToolbar : public acToolBar
// General Description: This class implements a toolbar with combo boxes for kernel work
//                      items
// Author:              Sigal Algranaty
// Creation Date:       2/8/2011
// ----------------------------------------------------------------------------------
class GW_API gwKernelWorkItemToolbar : public acToolBar
{
    Q_OBJECT

public:
    gwKernelWorkItemToolbar(QWidget* pParent);

    // Event handling:
    void updateToolbarValues();
    void resetTooblarValues(bool rebuildThreadValues, bool rebuildWIValues);

    void onAfterKernelDebuggingEvent(const apAfterKernelDebuggingEvent& event);

protected slots:
    void comboSelectionChange(int selectedItemIndex);
    void comboLineEditEditingFinishedHandler();

private:
    void populateComboBox(QComboBox* pComboBox, QIntValidator* pValidator, bool& isEnabledBuffer, int currBoxCount, int currBoxOffset);
    void updateToolbarThreadValues();
    void updateToolbarWorkItemValues();
    void rebuildWITooblarValues();

private:
    // Combo boxes labels:
    QLabel* _pWILabels[3];
    QString _wiLabelStrings[3];

    // Contain true iff coordinate i should be enabled:
    bool m_areWIComboBoxesEnabled[3];

    bool m_shouldRebuildWICombos;
    bool m_shouldRebuildThreadsCombo;

    // Toolbar widgets:
    QComboBox* m_pThreadsCombobox;
    int m_threadsComboSeparatorIndex;
    QComboBox* _pCoordComboBoxes[3];

    // Combo boxes validators:
    QIntValidator* _pWIComboValidators[3];

private:
    // Updates the selected item cache.
    void updateComboSelectionCache(int comboIndex, int selectedItem);

    // Gets the former selected item for the combo.
    int fetchComboSelectionFromCache(int comboIndex);

    // Cache the selected index for each combo box.
    int m_selectedIndexComboA;
    int m_selectedIndexComboB;
    int m_selectedIndexComboC;
};


#endif //__GWKERNELWORKITEMTOOLBAR_H

