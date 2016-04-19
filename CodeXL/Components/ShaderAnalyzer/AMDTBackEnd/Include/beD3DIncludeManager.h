#ifndef __D3DIncludeManager_h
#define __D3DIncludeManager_h

// D3D.
#include <d3dcommon.h>

// Infra.
#include <vector>

// C++.
#include <string>

class D3DIncludeManager :
    public ID3DInclude
{
public:
    D3DIncludeManager(const std::string& shaderDir, const std::vector<std::string>& includeSearchDirs);
    virtual ~D3DIncludeManager();

    virtual STDMETHODIMP Open(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes);

    virtual STDMETHODIMP Close(THIS_ LPCVOID pData);

private:
    std::string m_shaderDir;
    std::vector<std::string> m_includeSearchDirs;

};

#endif // __D3DIncludeManager_h
