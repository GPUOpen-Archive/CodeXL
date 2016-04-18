//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdCallsStackListCtrl.h
///
//==================================================================================

//------------------------------ gdCallsStackListCtrl.h ------------------------------

#ifndef __GDCALLSSTACKLISTCTRL_H
#define __GDCALLSSTACKLISTCTRL_H

// Qt:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:           GD_API gdCallsStackListCtrl: public acWXListCtrl
// General Description: A class that holds and displays a calls stack
// Author:               Uri Shomroni
// Creation Date:        19/10/2008
// ----------------------------------------------------------------------------------
class GD_API gdCallsStackListCtrl: public acListCtrl
{
    Q_OBJECT

public:
    gdCallsStackListCtrl(QWidget* pParent, bool isMainCallStack);
    virtual ~gdCallsStackListCtrl();

    void updateCallsStack(const osCallStack& callsStack);

    void deleteListItems();

    void setEmptyCallStackString(const QString& str) {_emptyCallStackString = str;};

    // A struct which we attach to each list item:
    struct gdCallsStackListItemData
    {
        gdCallsStackListItemData() : _functionStartAddress(NULL), _sourceCodeFileLineNumber(0), _instructionCounterAddress(NULL) {};

        // The function name
        gtString _functionName;

        // The function Start address
        osInstructionPointer _functionStartAddress;

        // The source code file path
        osFilePath _sourceCodeFilePath;

        // The function line number in the source code
        int _sourceCodeFileLineNumber;

        // The source code module path
        osFilePath _sourceCodeModulePath;

        // the instruction counter address
        osInstructionPointer _instructionCounterAddress;
    };

    int callStackSize() const { return _callStackSize; };
    void setCallStackSize(int callStackSize);

protected slots:

    // Can be override in inherited classes:
    virtual void onCallStackListSelected(QTableWidgetItem* pSelectedItem);
    virtual void onCallStackListActivated(QTableWidgetItem* pActivatedItem);
    virtual void onCallStackListCurrentItemChanged(QTableWidgetItem* pCurrentItem, QTableWidgetItem* pPreviousItem);

protected:

    void createAndLoadImageList();
    void onSetFocus();


protected:

    // The size (depth) of the calls Stack:
    int _callStackSize;

    // Icons:
    gtPtrVector<QPixmap*> _listIconsVec;

    // Is this call stack is the main one:
    bool _isMainCallStack;

    // Contain the string for empty call stack:
    QString _emptyCallStackString;

    // send activation events
    bool m_sendActivationEvents;
};

#endif //__GDCALLSSTACKLISTCTRL_H
