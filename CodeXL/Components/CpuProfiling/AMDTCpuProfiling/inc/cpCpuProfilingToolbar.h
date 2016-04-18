//=============================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
//=============================================================

#ifndef __CPUPROFILINGTOOLBAR_H
#define __CPUPROFILINGTOOLBAR_H

// Qt:
#include <QCheckbox>
#include <QComboBox>
#include <QToolButton>
#include <QPushButton>

// Infra:
#include <AMDTApplicationComponents/Include/acToolBar.h>


/// Implements the toolbar handling the CPU profile actions
class cpCpuProfilingToolbar : public acToolBar
{
    Q_OBJECT

public:

    /// Get my single instance:
    static cpCpuProfilingToolbar& instance();


    QComboBox* viewsComboBox() const {return m_pViewsComboBox;}
    QComboBox* separateComboBox() const {return m_pSeparateComboBox;};
    QToolButton* expandCollapseButton() const {return m_pExpandCollapseButton;};
    QToolButton* bit64Button() const {return m_p64bitButton;};
    QToolButton* percentButton() const {return m_pPercentButton;};
    QComboBox* aggregateByMPComboBox() const {return m_pAggregateByMPComboBox;};
    QToolButton* cssButton() const {return m_pCssButton;};
    QPushButton* cpuFilterButton() const {return m_pCpuFilterButton;}
    QComboBox* pidComboBox() const {return m_pPidComboBox;}
    QComboBox* tidComboBox() const {return m_pTidComboBox;}
    QToolButton* pidTidButton() const {return m_pPidTidButton;}
    QCheckBox* sysLibsCheckBox() const {return m_pSysLibs;}
    QComboBox*  functionsComboBox() const {return m_pFunctionsComboBox;}
    QPushButton* whiteSpaceButton() const {return m_pWhiteSpaceButton;}
    QComboBox* srcDasmSel() const {return m_pSrcDasmSel;}
    QToolButton* codeBytesButton() const {return m_pCodeBytesButton;}


    void setAggregation(int aggregation) {m_MPAggregation = aggregation;}
    int aggregation() const {return m_MPAggregation;}

private:

    /// Initialize the toolbar widgets:
    void initialize();

    /// Initialize the toolbar system tab widgets:
    void initializeSystemTabWidgets();
private:

    /// Private constructor (single instance class)
    cpCpuProfilingToolbar(QWidget* pParent);

    /// Single instance static member:
    static cpCpuProfilingToolbar* m_psMySingleInstance;


    /// Views widgets:
    QComboBox*       m_pViewsComboBox;
    QToolButton*     m_pPercentButton;
    QComboBox*       m_pSeparateComboBox;
    QPushButton*     m_pCpuFilterButton;

    /// Options widgets:
    QToolButton*     m_pCssButton;
    QToolButton*     m_pExpandCollapseButton;
    QToolButton*     m_p64bitButton;

    // Aggregation by Modules/Processes widgets:
    QComboBox*       m_pAggregateByMPComboBox;
    int             m_MPAggregation;
    QComboBox*       m_pPidComboBox;
    QComboBox*       m_pTidComboBox;
    QToolButton*     m_pPidTidButton;
    QCheckBox*       m_pSysLibs;

    // Source code widgets:
    QComboBox*   m_pFunctionsComboBox;
    QPushButton* m_pWhiteSpaceButton;
    QComboBox*   m_pSrcDasmSel;
    QToolButton* m_pCodeBytesButton;

};


#endif //__CPUPROFILINGTOOLBAR_H

