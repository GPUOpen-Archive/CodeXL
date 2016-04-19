#ifndef IVscTimerOwner_h__
#define IVscTimerOwner_h__
#include <WinDef.h>

class IVscTimerOwner
{
public:

    virtual ~IVscTimerOwner() {}

    virtual bool vscTimerOwner_GetStartupProjectDebugInfo(wchar_t*& pExecutableFilePathBuffer, wchar_t*& pWorkingDirectoryPathBuffer,
                                                          wchar_t*& pCommandLineArgumentsBuffer, wchar_t*& pEnvironmentBuffer, bool& isProjectOpenBuffer, bool& isProjectTypeValidBuffer, bool& isNonNativeProjectBuffer) = 0;

    virtual void vscTimerOwner_UpdateProjectSettingsFromStartupInfo() const = 0;

    virtual void vscTimerOwner_UpdateCommandShellUi() const = 0;

    virtual void vscTimerOwner_GetActiveDocumentFileFullPath(wchar_t*& pStrBuffer, bool& hasActiveDocBuffer) const = 0;

    virtual void vscTimerOwner_GetParentWindowHandle(HWND& winHandle) const = 0;

    // This function should allow the vscTimer to request the allocating object
    // to delete memory buffers which are no longer used by it.
    virtual void vscTimerOwner_DeleteWCharStr(wchar_t*& pStr) const = 0;

    virtual bool vscTimerOwner_GetActiveWindowHandle(HWND& winHandle) const = 0;

};

#endif // IVscTimerOwner_h__
