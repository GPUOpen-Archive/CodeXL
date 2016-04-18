#ifndef IDteTreeEventHandler_h__
#define IDteTreeEventHandler_h__
class IDteTreeEventHandler
{
public:
    virtual ~IDteTreeEventHandler() { }
    virtual void vscAddSolutionAsItemToTree(const wchar_t* pItemName) = 0;
    virtual void vscAddIOpenDocumentAsItemToTree(const wchar_t* documentName, const wchar_t* documentProjectName, const wchar_t* solutoinName) = 0;
    virtual void vscAddOpenProjectAsItemToTree(const wchar_t* pProjectName, const wchar_t* pSolutionName) = 0;
    virtual void vscExpandWholeTree() = 0;

};


#endif // IDteTreeEventHandler_h__
