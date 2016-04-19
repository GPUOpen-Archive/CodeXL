//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspPackageWrapper.h
///
//==================================================================================

//------------------------------ vspPackageWrapper.h ------------------------------

#ifndef __VSPPACKAGEWRAPPER_H
#define __VSPPACKAGEWRAPPER_H

// Pre declerations:
class CodeXLVSPackage;

// ----------------------------------------------------------------------------------
// Class Name:          vspPackageWrapper
// General Description: Holds a pointer to the package
// Author:              Sigal Algranaty
// Creation Date:       24/10/2010
// ----------------------------------------------------------------------------------
class vspPackageWrapper
{
public:
    ~vspPackageWrapper();

    static vspPackageWrapper& instance();

    void setPackage(CodeXLVSPackage* pPackage);
    void clearPackage();
    CodeXLVSPackage* getPackage() {return _pPackage;};

    bool isVSUIContextActive(REFGUID rguidCmdUI);

    IVsOutputWindowPane* getDebugPane();
    void outputMessage(const std::wstring& messageString, bool outputOnlyToLog);
    void clearMessagePane();
    IVsOutputWindowPane* getBuildPane();
    /// \param messageString message to display
    /// \param outputOnlyToLog
    /// \param filePathAndName when build error message is connected to error this string represents the file were the error is located, empty if not
    /// \param line when build error message is connected to error, this int represents the error line
    void outputBuildMessage(std::wstring& messageString, bool outputOnlyToLog, std::wstring filePathAndName, int line);
    void clearBuildPane();

    // Inform the package of a change in debug engines:
    void informPackageOfNewDebugEngine(void* pNewDebugEngine);
    void uninformPackageOfDebugEngine();

protected:
    static vspPackageWrapper* _pMySingleInstance;
    CodeXLVSPackage* _pPackage;
    IVsOutputWindowPane* _pOutputWindowPane;

private:
    // Do not allow the use of my constructor:
    vspPackageWrapper();

private:
    friend class vspSingletonsDelete;
};

#endif //__VSPPACKAGEWRAPPER_H

