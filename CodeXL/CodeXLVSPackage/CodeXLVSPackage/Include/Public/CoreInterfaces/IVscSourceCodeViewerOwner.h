#ifndef IVscSourceCodeViewerOwner_h__
#define IVscSourceCodeViewerOwner_h__

class IVscSourceCodeViewerOwner
{
public:
    virtual ~IVscSourceCodeViewerOwner() {}
    virtual bool scvOwnerOpenFileAtPosition(const wchar_t* pFilePath, int lineNumber = 0, int columnNumber = 0, bool selectLine = true) const = 0;
    virtual bool scvOwnerGetWindowFromFilePath(const wchar_t* pFilePath, void*& pWindowBuffer) const = 0;
    virtual bool scvOwnerCloseAndReleaseWindow(void*& pWindow, bool closeWindow = true) const = 0;
};

#endif // IVscSourceCodeViewerOwner_h__
