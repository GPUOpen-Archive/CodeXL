//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ahAssertDialog.h
///
//==================================================================================

#ifndef __AHASSERTDIALOG_H
#define __AHASSERTDIALOG_H

// This solves a problem in Qt 4.7, which includes an unused typedefs in one of its
// header files. This problem is claimed to be fixed in Qt 4.8. Changing the Qt sources
// directly is off the table due to licensing issues.
#ifdef __GNUC__
    #include <features.h>
    #if __GNUC_PREREQ(4,8)
        #pragma GCC diagnostic ignored "-Wunused-local-typedefs"
    #endif
#endif


// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>

// ----------------------------------------------------------------------------------
// Class Name:           ahAssertDialog : public acDialog
// General Description:
//   A Message dialog that displays the details of an assertion failure.
//
// Author:               Avi Shapira
// Creation Date:        27/11/2005
// ----------------------------------------------------------------------------------
class ahAssertDialog : QMessageBox
{

public:

    ahAssertDialog(const osFilePath& filePath, const int lineNumber, const gtASCIIString& assertReason, QWidget* pParent);
    virtual ~ahAssertDialog();

    int exec();

protected:

    bool loadIcon();
    void fillAssertionMessageBody();
    void setDialogTitle();

protected:

    // The displayed assertion parameters:
    osFilePath _filePath;
    int _lineNumber;
    gtASCIIString _assertReason;
    QWidget* _pParentWidget;

};


#endif //__AHASSERTDIALOG_H

