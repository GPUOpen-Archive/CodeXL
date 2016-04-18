#ifndef IVscApplicationCommandsOwner_h__
#define IVscApplicationCommandsOwner_h__

#include "CodeXLVSPackageCoreDefs.h"

class IVscApplicationCommandsOwner
{
public:
    virtual ~IVscApplicationCommandsOwner() {}

    virtual void CloseDocumentsOfDeletedFiles() const = 0;

    virtual bool SaveFileWithPath(const wchar_t* pFilePathStr) const = 0;

    virtual bool GetFunctionBreakpoints(wchar_t**& pEnabledFunctionBreakpointsBuffer,  int& enabledFuncBreakpointsCount,
                                        wchar_t**& pDisabledFunctionBreakpointsBuffer, int& disabledFuncBreakpointsCount) const = 0;

    virtual bool CloseFile(const wchar_t* pFilePath) const = 0;

    virtual bool OpenFileAtPosition(const wchar_t* pFilePath, int lineNumber = 0, int columnNumber = 0, bool selectLine = true) const = 0;

    virtual void DeleteWcharStrBuffers(wchar_t**& pBuffersArray, int buffersArraySize) const = 0;

    virtual bool RaiseStatisticsView() = 0;

    virtual bool RaiseMemoryView() = 0;

    virtual void SetToolWindowCaption(int windowId, const wchar_t* windowCaption) = 0;

    virtual void ClearBuildPane() = 0;

    virtual void OutputBuildMessage(const wchar_t* pMsgToPrint, bool outputOnlyToLog, const wchar_t* pFile, int line) = 0;

    virtual void UpdateProjectSettingsFromStartupProject() = 0;

    // This function is named with a prefix to prevent name clashes with other derived virtual functions which are expected to be found
    // in the deriving classes.
    virtual void ivacoOwnerDeleteWCharStr(wchar_t*& pStr) = 0;
};


#endif // IVscApplicationCommandsOwner_h__
