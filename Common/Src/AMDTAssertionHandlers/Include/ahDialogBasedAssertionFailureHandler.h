//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ahDialogBasedAssertionFailureHandler.h
///
//==================================================================================

#ifndef __AHDIALOGBASEDASSERTIONFAILUREHANDLER_H
#define __AHDIALOGBASEDASSERTIONFAILUREHANDLER_H

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtIAssertionFailureHandler.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTAssertionHandlers/Include/ahDialogBasedAssertionFailureHandler.h>


// ----------------------------------------------------------------------------------
// Struct Name:         ahSourceCodeLocation
// General Description:
//  Represents a location within the source code (file path + line number).
//
// Author:               Yaki Tebeka
// Creation Date:        12/4/2005
// ----------------------------------------------------------------------------------
struct ahSourceCodeLocation
{
    osFilePath _filePath;
    int _lineNumber;
};



// ----------------------------------------------------------------------------------
// Class Name:           ahDialogBasedAssertionFailureHandler : public gtIAssertionFailureHandler
// General Description:
//  An assertion failure handler that pops up the assertion failure dialog.
//
// Author:               Yaki Tebeka
// Creation Date:        12/4/2005
// ----------------------------------------------------------------------------------
class ahDialogBasedAssertionFailureHandler : public gtIAssertionFailureHandler
{
public:
    ahDialogBasedAssertionFailureHandler(osThreadId mainGUIThreadId);
    virtual ~ahDialogBasedAssertionFailureHandler();

    // Overrides gtIAssertionFailureHandler:
    virtual void onAssertionFailure(const wchar_t* functionName, const wchar_t* fileName,
                                    int lineNumber, const wchar_t* message);

private:
    bool shouldDisplayDialog() const;
    bool isInIgnoredSourceLocations(const osFilePath& filePath, const int lineNumber);
    int displayAssertionDialog(const wchar_t* functionName, const osFilePath& filePath,
                               int lineNumber, const wchar_t* message);
    void handleUsersDecision(int decision, const osFilePath& filePath, int lineNumber);
    void outputAssertionMessageToStderr(const wchar_t* functionName, const wchar_t* fileName,
                                        int lineNumber, const wchar_t* message);

    // Do not allow the use of my default constructor:
    ahDialogBasedAssertionFailureHandler();

private:
    // The main GUI thread id:
    osThreadId _mainGUIThreadId;

    // Ignored source code locations:
    gtPtrVector<ahSourceCodeLocation*> _ignoredSourceLocations;
};


#endif //__AHDIALOGBASEDASSERTIONFAILUREHANDLER_H

